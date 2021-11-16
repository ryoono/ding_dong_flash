/*
 * ピンポンフラッシュのデバイス親機
 */

#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
#include "wifiConfig.h"

// 状態定義
int state;
#define INTERCOM_PRESSED_WAIT 0
#define SEND_LIGHTING_REQUEST 10
#define HIGH_POWER_LED_ON     20
#define HIGH_POWER_LED_OFF    30

// ハイパワーLEDの点滅間隔
#define HLED_LIGHTING_INTERVAL  300   // 間隔：300[ms]

// LEDのI/Oポート設定
#define LED_R 2
#define LED_Y 4
#define SA 13

const char ssid[] = MY_SSID;          // SSID
const char pass[] = MY_SSID_PASSWORD; // password

const IPAddress ip( LEADER_IP_ADDRESS ); // IPアドレス(ゲートウェイも兼ねる)
const IPAddress subnet( MY_SUBNETMASK ); // サブネットマスク

WiFiUDP udp_Tx_1;
WiFiUDP udp_Tx_2;
WiFiUDP udp_Tx_3;

static const IPAddress follower_1_IP( FOLLOWER_IP_ADDRESS_1 );  //送信先のIPアドレス
static const IPAddress follower_2_IP( FOLLOWER_IP_ADDRESS_2 );
static const IPAddress follower_3_IP( FOLLOWER_IP_ADDRESS_3 );
static const int port_1_Tx = LEADER_PORT_TX1; //送信用のポート
static const int port_2_Tx = LEADER_PORT_TX2;
static const int port_3_Tx = LEADER_PORT_TX3;
static const int follower_1_Port_Rx = FOLLOWER_PORT_RX; //送信先のポート
static const int follower_2_Port_Rx = FOLLOWER_PORT_RX;
static const int follower_3_Port_Rx = FOLLOWER_PORT_RX;

// ハイパワーLEDの点滅間隔
int HLED_lighting_interval;

// HIGH：LED ON / LOW：LED OFF
int ledY_sta;
int ledR_sta;

// インターホン押下カウンタ
int input_cnt;

// ハイパワーLEDの初期設定
void HLED_setup(){

  HLED_lighting_interval = 0;
  // ハイパワーLED消灯処理
}

void setup() {

  ledY_sta = HIGH;
  ledR_sta = LOW;
  pinMode( LED_Y, OUTPUT);
  pinMode( LED_R, OUTPUT);
  pinMode( SA, INPUT);
  digitalWrite( LED_Y, ledY_sta);
  digitalWrite( LED_R, ledR_sta);

  WiFi.mode( WIFI_AP );
  WiFi.softAP( ssid, pass);           // SSIDとパスの設定
  delay( 100 );                       // 追記：このdelayを入れないと失敗する場合がある
  WiFi.softAPConfig( ip, ip, subnet); // IPアドレス、ゲートウェイ、サブネットマスクの設定;

  udp_Tx_1.begin( port_1_Tx );  // UDP通信の開始(引数はポート番号)
  udp_Tx_2.begin( port_2_Tx );  // UDP通信の開始(引数はポート番号)
  udp_Tx_3.begin( port_3_Tx );  // UDP通信の開始(引数はポート番号)
  
  HLED_setup();

  state = INTERCOM_PRESSED_WAIT;
  input_cnt = 0;
}

void loop() {

  switch( state ){

    // ########################
    // ## インターホン押下待ち ##
    case INTERCOM_PRESSED_WAIT:

      // インターホンが100ms連続で押下されていた場合、
      // 点灯リクエストへ遷移
      if( digitalRead( SA ) == HIGH ){
        if( ++input_cnt >= 100 ){

          state = SEND_LIGHTING_REQUEST;
        }
      }
      else{
        input_cnt = 0;
      }
      break;

    //#######################
    // ## 点灯リクエスト送信 ##
    case SEND_LIGHTING_REQUEST:

      // 点灯リクエストを送信し、ハイパワーLED点灯状態へ遷移
      udp_Tx_1.beginPacket( follower_1_IP, follower_1_Port_Rx);
      udp_Tx_1.write( FOLLOWER_ID_1 );  //10進数のaskiiで送信される
      udp_Tx_1.endPacket();
  
      udp_Tx_2.beginPacket( follower_2_IP, follower_2_Port_Rx);
      udp_Tx_2.write( FOLLOWER_ID_2 );  //10進数のaskiiで送信される
      udp_Tx_2.endPacket();
  
      udp_Tx_3.beginPacket( follower_3_IP, follower_3_Port_Rx);
      udp_Tx_3.write( FOLLOWER_ID_3 );  //10進数のaskiiで送信される
      udp_Tx_3.endPacket();

      HLED_lighting_interval = 0;
      ledR_sta = HIGH;
      digitalWrite( LED_R, ledR_sta);

      state = HIGH_POWER_LED_ON;
      break;

    // ######################
    // ## ハイパワーLED点灯 ##
    case HIGH_POWER_LED_ON:

      // 300ms経過したら、ハイパワーLED消灯状態へ遷移
      if( ++HLED_lighting_interval >= HLED_LIGHTING_INTERVAL ){
        
        HLED_lighting_interval = 0;
        ledR_sta = LOW;
        digitalWrite( LED_R, ledR_sta);
        state = HIGH_POWER_LED_OFF;
      }
      break;

    // ######################
    // ## ハイパワーLED消灯 ##
    case HIGH_POWER_LED_OFF:

      // 300ms経過したら、インターホン押下待ち状態へ遷移
      if( ++HLED_lighting_interval >= HLED_LIGHTING_INTERVAL ){
        
        input_cnt = 0;
        state = INTERCOM_PRESSED_WAIT;
      }
      break;

    // #####################################
    // ## 例外処理 インターホン押下待ちへ遷移 ##
    default:
      // どの状態でこの処理を行うのか分からないため、ハイパワーLED消灯処理
      // ここに書く
      ledR_sta = LOW;
      digitalWrite( LED_R, ledR_sta);
      input_cnt = 0;
      state = INTERCOM_PRESSED_WAIT;
      break;
  }
  
  delay(1);
}
