/*
 * ピンポンフラッシュのデバイス子機
 */
#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
#include "wifiConfig.h"

// LEDのI/Oポート設定
#define LED_R 2
#define LED_Y 4

// ハイパワーLEDの点滅間隔 / 点滅期間
#define HLED_LIGHTING_INTERVAL  500   // 間隔：500[ms]
#define HLED_LIGHTING_COUNTER   60    // 期間：0.5[s]*60 = 30[s]

// 書き込むデバイス子機に対応する定数をコメントアウトしてコンパイルする
//#define FOLLOWER1
//#define FOLLOWER2
#define FOLLOWER3

const char ssid[] = MY_SSID;          // SSID
const char pass[] = MY_SSID_PASSWORD; // password

WiFiUDP udp_Tx;
WiFiUDP udp_Rx;

// この書き方あんまりよくない気がする
// デバイスの数が増えたら管理が大変になる
#ifdef FOLLOWER1
// 自機の情報
static const char id = '1';
// このデバイスのネットワーク情報
static const IPAddress ip( FOLLOWER_IP_ADDRESS_1 ); // IPアドレス(ゲートウェイも兼ねる)
// 親機のネットワーク情報
static const int reader_Port_Rx = LEADER_PORT_RX1;    //送信先のポート
#endif
#ifdef FOLLOWER2
static const char id = '2';
static const IPAddress ip( FOLLOWER_IP_ADDRESS_2 ); // IPアドレス(ゲートウェイも兼ねる)
static const int reader_Port_Rx = LEADER_PORT_RX2;    //送信先のポート
#endif
#ifdef FOLLOWER3
static const char id = '3';
static const IPAddress ip( FOLLOWER_IP_ADDRESS_3 ); // IPアドレス(ゲートウェイも兼ねる)
static const int reader_Port_Rx = LEADER_PORT_RX3;    //送信先のポート
#endif

// このデバイスのネットワーク情報
static const IPAddress subnet( MY_SUBNETMASK );     // サブネットマスク
static const int port_Tx = FOLLOWER_PORT_TX; //送信用のポート
static const int port_Rx = FOLLOWER_PORT_RX; //受信用のポート

// 親機のネットワーク情報
static const IPAddress leader_IP(LEADER_IP_ADDRESS);  //送信先のIPアドレス

// HIGH：LED ON / LOW：LED OFF
int ledY_sta;
int ledR_sta;

// ハイパワーLEDの点滅間隔 / 点滅回数
int HLED_lighting_interval;
int HLED_lighting_counter;

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
  udp_Tx.begin( port_Tx );  // UDP通信の開始(引数はポート番号)
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
  HLED_lighting_counter   = 0;
  // ハイパワーLED消灯処理
}

void setup() {

  // I/Oポートの初期設定
  IO_setup();   // I/Oポートの初期設定
  // ハイパワーLEDの初期設定
  HLED_setup();
//  Serial_setup();
  WiFi_setup(); // ネットワークの初期設定
}

void loop() 
{

  // 点灯リクエストを受信したときの処理
  if( udp_Rx.parsePacket() ){
    
//    ledR_sta = !ledR_sta;
//    digitalWrite( 2, ledR_sta);
    char data = udp_Rx.read();
    // 受信データバッファクリア(ってリファレンスに書いてあるけど、実際はクリアされてない気がする)
    udp_Rx.flush();

    // 自機に向けられたデータを受信したら
    // ハイパワーLEDを光らせる処理に入る
    if( data == id ){

      // ACKの送信
      udp_Tx.beginPacket(leader_IP, reader_Port_Rx);
      udp_Tx.write( id );
      udp_Tx.endPacket();

      // ハイパワーLED点灯準備
      HLED_lighting_interval  = HLED_LIGHTING_INTERVAL;
      HLED_lighting_counter   = HLED_LIGHTING_COUNTER;
      
//      Serial.println(i); // UDP通信で来た値を表示
    }
  }

  // 指定回数点滅させる
  if( HLED_lighting_counter > 0 ){

    HLED_lighting_interval--;

    // HLED_LIGHTING_INTERVAL秒間経過した場合、指定回数を更新
    if( HLED_lighting_interval <= 0 ){

      // カウンタを減らして、インターバルを初期値に戻す
      HLED_lighting_counter--;
      HLED_lighting_interval = HLED_LIGHTING_INTERVAL;
      
      // ハイパワーLEDの出力を反転させる
      // ここに記載
      ledR_sta = !ledR_sta;
      digitalWrite( LED_R, ledR_sta);
    }
  }
  else{
    ledR_sta = LOW;
    digitalWrite( LED_R, ledR_sta);
    HLED_setup();
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
