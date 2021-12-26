/*
 * ピンポンフラッシュのデバイス子機
 */
#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
#include "apInfo.h"
#include "networkInfo.h"

// 状態定義
int state;
#define LIGHTING_REQUEST_WAIT 0   // 点灯リクエスト待ち
#define HIGH_POWER_LED_ON     10  // ハイパワーLED点灯
#define HIGH_POWER_LED_OFF    20  // ハイパワーLED消灯

// LEDのI/Oポート設定
#define LED_HP  5
#define LED_Y   16
// HIGH：LED ON / LOW：LED OFF
int ledY_sta;
int ledHP_sta;

// SPのI/Oポート設定
#define SP      14
// HIGH：SP_ON / LOW：SP OFF
int sp_sta;

// SWのI/Oポート設定
#define SW  2

// ハイパワーLEDの点滅間隔
int HLED_lighting_interval;
#define HLED_LIGHTING_INTERVAL  300   // 間隔：300[ms]

// SPリセット時限
int sp_reset_cnt;
int sp_flag;
#define SP_RESET_CNT  2000  // 2000[ms]

// 書き込むデバイス子機に対応する定数をコメントアウトしてコンパイルする
//#define FOLLOWER1
#define FOLLOWER2
//#define FOLLOWER3

// この書き方あんまりよくない気がする
// デバイスの数が増えたら管理が大変になる
#ifdef FOLLOWER1
// 自機のID情報
static const char id = FOLLOWER_ID_1;
// このデバイスのipアドレス
static const IPAddress ip( FOLLOWER_IP_ADDRESS_1 );
#endif
#ifdef FOLLOWER2
static const char id = FOLLOWER_ID_2;
static const IPAddress ip( FOLLOWER_IP_ADDRESS_2 );
#endif
#ifdef FOLLOWER3
static const char id = FOLLOWER_ID_3;
static const IPAddress ip( FOLLOWER_IP_ADDRESS_3 );
#endif

// このデバイスのネットワーク情報
static const IPAddress subnet( MY_SUBNETMASK );     // サブネットマスク
static const int port_Rx = FOLLOWER_PORT_RX; //受信用のポート

// AP情報
const char ssid[] = MY_SSID;          // SSID
const char pass[] = MY_SSID_PASSWORD; // password

// UDP用のポート設定
WiFiUDP udp_Rx;

// ネットワーク初期設定
void WiFi_setup(){

  // 子機モードでAPにアクセスする
  WiFi.mode( WIFI_STA );
  WiFi.begin( ssid, pass);
  delay( 100 ); // 設定反映待ち
  
  WiFi.config( ip, ip, subnet);
  while( WiFi.status() != WL_CONNECTED ){
    
    ledY_sta = !ledY_sta;
    digitalWrite( LED_Y, ledY_sta);
    delay( 500 );
  }
  ledY_sta = HIGH;
  digitalWrite( LED_Y, ledY_sta);

  udp_Rx.begin( port_Rx );  // UDP通信の開始(引数はポート番号)
}

// IOポートの初期設定
void IO_setup(){
  
  pinMode(  LED_Y, OUTPUT);
  pinMode( LED_HP, OUTPUT);
  pinMode(     SP, OUTPUT);
  pinMode(     SW,  INPUT);
}

// 状態のリセット
void state_reset(){

  ledY_sta  = LOW;
  ledHP_sta = LOW;
  sp_sta    = LOW;
  digitalWrite(  LED_Y,  ledY_sta);
  digitalWrite( LED_HP, ledHP_sta);
  digitalWrite(     SP,    sp_sta);
  state = LIGHTING_REQUEST_WAIT;
  HLED_lighting_interval  = 0;
  sp_reset_cnt = 0;
  sp_flag = false;
}

void setup() {
  
  IO_setup();     // IOポートの初期設定
  state_reset();  // 状態のリセット
  WiFi_setup();   // ネットワーク初期設定
}

void loop() 
{

  switch( state ){

    // ############################
    // ##### 点灯リクエスト待ち #####
    case LIGHTING_REQUEST_WAIT:

      // UDPでデータを受信したときの処理
      // 点灯リクエスト(自機のID)の場合、
      if( udp_Rx.parsePacket() ){
        
        char data = udp_Rx.read();
        // 受信データバッファクリア(ってリファレンスに書いてあるけど、実際はクリアされてない気がする)
        udp_Rx.flush();

        // 点灯リクエスト受信処理
        // ハイパワーLED点灯に状態遷移
        if( data == id ){

          // ハイパワーLED点灯処理
          ledHP_sta = HIGH;
          digitalWrite( LED_HP, ledHP_sta);

          // タイマリセット
          HLED_lighting_interval = 0;
          sp_reset_cnt = 0;

          // スピーカ鳴動処理
          if( sp_flag ){
            sp_flag = false;
            sp_sta = HIGH;
            digitalWrite( SP, sp_sta);
          }
          state = HIGH_POWER_LED_ON;
        }
      }

      // スピーカが連続で何度も鳴動するとうるさいため
      // インターホンが鳴りやんで次の来客待ちになったら鳴動可能状態にする
      // デバイス親機からの来客情報が2000msec来なければ、インターホン親機のA接点が閉じたと判断する
      if( sp_reset_cnt >= SP_RESET_CNT ){
        sp_flag = true;
      }
      else{
        sp_reset_cnt++;
      }
      break;

    // ############################
    // ##### ハイパワーLED点灯 #####
    case HIGH_POWER_LED_ON:

      // 指定時間後に、ハイパワーLED消灯へ遷移
      if( ++HLED_lighting_interval >= HLED_LIGHTING_INTERVAL ){

        // ハイパワーLED消灯処理
        // ここに書く
        ledHP_sta = LOW;
        digitalWrite( LED_HP, ledHP_sta);
        HLED_lighting_interval = 0; // タイマリセット
        sp_sta = LOW;
        digitalWrite( SP, sp_sta);
        state = HIGH_POWER_LED_OFF;
      }
      break;

    // ############################
    // ##### ハイパワーLED消灯 #####
    case HIGH_POWER_LED_OFF:

      // 指定時間後に、点灯リクエスト待ちへ遷移
      if( ++HLED_lighting_interval >= HLED_LIGHTING_INTERVAL ){

        // ハイパワーLED消灯処理
        // ここに書く
        ledHP_sta = LOW;
        digitalWrite( LED_HP, ledHP_sta);
        HLED_lighting_interval = 0; // タイマリセット
        state = LIGHTING_REQUEST_WAIT;
      }
      break;

    // ###################################
    // ## 例外処理 点灯リクエスト待ちへ遷移 ##
    default:
      // どの状態でこの処理を行うのか分からないため、
      // 状態を初期化する
      state_reset();
      break;
  } 

  // WiFiの接続チェック
  // 毎周期確認し、切断時には再度繋ぎなおす
  WiFi_connection_chk();
  delay(1);
}

// WiFiの接続チェック
// 切断時には再度繋ぎなおす
void WiFi_connection_chk(){
  
  if( WiFi.status() != WL_CONNECTED ){
    
    WiFi.disconnect();
    // ハイパワーLEDが点灯した状態でWiFiチェックが入ると、ずっと点灯してしまうためリセット
    state_reset();
    WiFi_setup();
  }
}
