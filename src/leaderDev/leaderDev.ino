/*
 * ピンポンフラッシュのデバイス親機
 */

#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
#include "wifiConfig.h"

// ハイパワーLEDの点滅間隔 / 点滅期間
#define HLED_LIGHTING_INTERVAL  500   // 間隔：500[ms]
#define HLED_LIGHTING_COUNTER   60    // 期間：0.5[s]*60 = 30[s]

// LEDのI/Oポート設定
#define LED_R 2
#define LED_Y 4
#define SA 13

const char ssid[] = MY_SSID; // SSID
const char pass[] = MY_SSID_PASSWORD;  // password

const IPAddress ip(LEADER_IP_ADDRESS);       // IPアドレス(ゲートウェイも兼ねる)
const IPAddress subnet(MY_SUBNETMASK); // サブネットマスク

WiFiUDP udp_Tx_1;
WiFiUDP udp_Rx_1;
WiFiUDP udp_Tx_2;
WiFiUDP udp_Rx_2;
WiFiUDP udp_Tx_3;
WiFiUDP udp_Rx_3;

const int port_1_Rx = LEADER_PORT_RX1;      // 受信ポート番号
const int port_2_Rx = LEADER_PORT_RX2;      // 受信ポート番号
const int port_3_Rx = LEADER_PORT_RX3;      // 受信ポート番号

static const IPAddress follower_1_IP(FOLLOWER_IP_ADDRESS_1);  //送信先のIPアドレス
static const IPAddress follower_2_IP(FOLLOWER_IP_ADDRESS_2);  //送信先のIPアドレス
static const IPAddress follower_3_IP(FOLLOWER_IP_ADDRESS_3);  //送信先のIPアドレス
static const int port_1_Tx = LEADER_PORT_TX1; //送信用のポート
static const int port_2_Tx = LEADER_PORT_TX2; //送信用のポート
static const int port_3_Tx = LEADER_PORT_TX3; //送信用のポート
static const int follower_1_Port_Rx = FOLLOWER_PORT_RX; //送信先のポート
static const int follower_2_Port_Rx = FOLLOWER_PORT_RX; //送信先のポート
static const int follower_3_Port_Rx = FOLLOWER_PORT_RX; //送信先のポート

// ハイパワーLEDの点滅間隔 / 点滅回数
int HLED_lighting_interval;
int HLED_lighting_counter;

// HIGH：LED ON / LOW：LED OFF
int ledY_sta;
int ledR_sta;

// ハイパワーLEDの初期設定
void HLED_setup(){

  HLED_lighting_interval  = 0;
  HLED_lighting_counter   = 0;
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
  
  Serial.begin(115200);
  delay(100);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, pass);           // SSIDとパスの設定
  delay(100);                        // 追記：このdelayを入れないと失敗する場合がある
  WiFi.softAPConfig(ip, ip, subnet); // IPアドレス、ゲートウェイ、サブネットマスクの設定

  Serial.print("AP IP address: ");
  IPAddress myIP = WiFi.softAPIP();
  Serial.println(myIP);

  Serial.println("Starting UDP");
  udp_Tx_1.begin(port_1_Tx);  // UDP通信の開始(引数はポート番号)
  udp_Rx_1.begin(port_1_Rx);  // UDP通信の開始(引数はポート番号)
  udp_Tx_2.begin(port_2_Tx);  // UDP通信の開始(引数はポート番号)
  udp_Rx_2.begin(port_2_Rx);  // UDP通信の開始(引数はポート番号)
  udp_Tx_3.begin(port_3_Tx);  // UDP通信の開始(引数はポート番号)
  udp_Rx_3.begin(port_3_Rx);  // UDP通信の開始(引数はポート番号)

  Serial.print("Local port: ");
  //Serial.println(localPort);

  HLED_setup();
}

void loop() {
  
  if (udp_Rx_1.parsePacket()) {
    char i = udp_Rx_1.read();  //ceramie追記 askiiから文字列へ
    udp_Rx_1.flush();
    Serial.print("back : ");
    Serial.println(i); // UDP通信で来た値を表示
  }
  if (udp_Rx_2.parsePacket()) {
    char i = udp_Rx_2.read();  //ceramie追記 askiiから文字列へ
    udp_Rx_2.flush();
    Serial.print("back : ");
    Serial.println(i); // UDP通信で来た値を表示
  }
  if (udp_Rx_3.parsePacket()) {
    char i = udp_Rx_3.read();  //ceramie追記 askiiから文字列へ
    udp_Rx_3.flush();
    Serial.print("back : ");
    Serial.println(i); // UDP通信で来た値を表示
  }

  //if( Serial.available() > 0 ){ 
  if( digitalRead( SA ) == HIGH ){
    char data = Serial.read();
    Serial.println(data);
    udp_Tx_1.beginPacket(follower_1_IP, follower_1_Port_Rx);
    udp_Tx_1.write('1');  //10進数のaskiiで送信される
    udp_Tx_1.endPacket();

    udp_Tx_2.beginPacket(follower_2_IP, follower_2_Port_Rx);
    udp_Tx_2.write('2');  //10進数のaskiiで送信される
    udp_Tx_2.endPacket();

    udp_Tx_3.beginPacket(follower_3_IP, follower_3_Port_Rx);
    udp_Tx_3.write('3');  //10進数のaskiiで送信される
    udp_Tx_3.endPacket();

    // ハイパワーLED点灯準備
      HLED_lighting_interval  = HLED_LIGHTING_INTERVAL;
      HLED_lighting_counter   = HLED_LIGHTING_COUNTER;
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
  
  delay(1);
}
