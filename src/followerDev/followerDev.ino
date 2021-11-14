/*
 * ピンポンフラッシュのデバイス子機
 */
#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
#include "wifiConfig.h"

// 状態定義
int state;
#define LIGHTING_REQUEST_WAIT 0
#define HIGH_POWER_LED_ON     10
#define HIGH_POWER_LED_OFF    20

// LEDのI/Oポート設定
#define LED_R 2
#define LED_Y 4

// ハイパワーLEDの点滅間隔
int HLED_lighting_interval;
#define HLED_LIGHTING_INTERVAL  500   // 間隔：300[ms]

// 書き込むデバイス子機に対応する定数をコメントアウトしてコンパイルする
//#define FOLLOWER1
//#define FOLLOWER2
#define FOLLOWER3

// この書き方あんまりよくない気がする
// デバイスの数が増えたら管理が大変になる
#ifdef FOLLOWER1
// 自機のID情報
static const char id = '1';
// このデバイスのipアドレス
static const IPAddress ip( FOLLOWER_IP_ADDRESS_1 );
#endif
#ifdef FOLLOWER2
static const char id = '2';
static const IPAddress ip( FOLLOWER_IP_ADDRESS_2 );
#endif
#ifdef FOLLOWER3
static const char id = '3';
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

// HIGH：LED ON / LOW：LED OFF
int ledY_sta;
int ledR_sta;

void WiFi_setup(){

  // 子機モードでAPにアクセスする
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  delay(100); // 設定反映待ち
  
  WiFi.config(ip, ip, subnet);
  while( WiFi.status() != WL_CONNECTED ){
    
    ledY_sta = !ledY_sta;
    digitalWrite( LED_Y, ledY_sta);
    delay(500);
//    Serial.println(".");
  }
  ledY_sta = HIGH;
  digitalWrite( LED_Y, ledY_sta);
//  Serial.print("AP IP address: ");
//  IPAddress myIP = WiFi.softAPIP();
//  Serial.println(myIP);

//  Serial.println("Starting UDP");
  udp_Rx.begin( port_Rx );  // UDP通信の開始(引数はポート番号)
}

//static void Serial_setup()
//{
//  Serial.begin(115200);
//  Serial.println(""); // to separate line  
//}

// IOポートの初期設定
void IO_setup(){
  
  ledY_sta = HIGH;
  ledR_sta = LOW;
  pinMode( LED_Y, OUTPUT);
  pinMode( LED_R, OUTPUT);
  digitalWrite( LED_Y, ledY_sta);
  digitalWrite( LED_R, ledR_sta);
}

// ハイパワーLEDの初期設定
void HLED_setup(){

  HLED_lighting_interval  = 0;
  // ハイパワーLED消灯処理
}

void setup() {

  // 状態の初期化
  state = 0;
  
  // I/Oポートの初期設定
  IO_setup();   // I/Oポートの初期設定
  // ハイパワーLEDの初期設定
  HLED_setup();
//  Serial_setup();
  WiFi_setup(); // ネットワークの初期設定
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
          // ここに書く
          ledR_sta = HIGH;
          digitalWrite( LED_R, ledR_sta);
          HLED_lighting_interval = 0; // タイマリセット
          state = HIGH_POWER_LED_ON;
        }
        
      }
      break;

    // ############################
    // ##### ハイパワーLED点灯 #####
    case HIGH_POWER_LED_ON:

      // 指定時間後に、ハイパワーLED消灯へ遷移
      if( ++HLED_lighting_interval >= HLED_LIGHTING_INTERVAL ){

        // ハイパワーLED消灯処理
        // ここに書く
        ledR_sta = LOW;
        digitalWrite( LED_R, ledR_sta);
        HLED_lighting_interval = 0; // タイマリセット
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
        ledR_sta = LOW;
        digitalWrite( LED_R, ledR_sta);
        HLED_lighting_interval = 0; // タイマリセット
        state = LIGHTING_REQUEST_WAIT;
      }
      break;

    // ###################################
    // ## 例外処理 点灯リクエスト待ちへ遷移 ##
    default:
      // どの状態でこの処理を行うのか分からないため、ハイパワーLED消灯処理
      // ここに書く
      // ハイパワーLED消灯処理
      // ここに書く
      ledR_sta = LOW;
      digitalWrite( LED_R, ledR_sta);
      state = LIGHTING_REQUEST_WAIT;
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
//    Serial.println("disconnect!");
    // ハイパワーLEDが点灯した状態でWiFiチェックが入ると、ずっと点灯してしまうためリセット
    HLED_setup();
    WiFi_setup();
  }
}
