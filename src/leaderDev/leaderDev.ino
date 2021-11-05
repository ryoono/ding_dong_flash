/*
#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
//#include <WiFiClient.h> 
#include "wifiConfig.h"

#define SW_PIN 13
#define LED_PIN 4
#define LED2_PIN 2

char WiFibuff[16];
int flag = 0;
int led_state = LOW;

// UDP setting
WiFiUDP UDP_Tx;
WiFiUDP UDP_Rx;

// WiFi setting
IPAddress myIP(LEADER_IP_ADDRESS);
IPAddress mySubnet(MY_SUBNETMASK);
IPAddress followerIpAddress1(FOLLOWER_IP_ADDRESS_1);
const char *ssid = MY_SSID;
const char *password = MY_SSID_PASSWORD;

void connectWiFi() {

  Serial.println("3");
//  WiFi.begin( ssid, password);
  Serial.println("4");
  WiFi.config( myIP, WiFi.gatewayIP(), mySubnet);
  Serial.println("5");

//  while( WiFi.status() != WL_CONNECTED ){
//    Serial.println("6");
//    delay(500);
//    led_state = !led_state;
//    digitalWrite( LED_PIN, led_state);
//  }
  Serial.println("7");
//  digitalWrite( LED_PIN, LOW);

  Serial.println("8");
  UDP_Tx.begin( LEADER_PORT_TX1 );
  Serial.println("9");
  UDP_Rx.begin( LEADER_PORT_RX1 );
  Serial.println("10");
}

void setup() {

  Serial.begin(115200);
  delay(1000);
  Serial.println("start");
  WiFi.mode( WIFI_AP ); // APを立てつつ、STAモード
  Serial.println("1");
  WiFi.softAP( ssid, password);
  Serial.println("2");
  connectWiFi();

  Serial.println("11");
//  pinMode( SW_PIN, INPUT);
//  pinMode( LED_PIN, OUTPUT);
//  digitalWrite( LED_PIN, HIGH);
//  pinMode( LED2_PIN, OUTPUT);
//  digitalWrite( LED2_PIN, HIGH);

  delay(3000);

//  digitalWrite( LED_PIN, LOW);
//  digitalWrite( LED2_PIN, LOW);
}

void rcvWiFi() {
  UDP_Rx.read(WiFibuff, 1);
  UDP_Rx.flush();
}

void loop() {

//  if( digitalRead( SW_PIN ) == HIGH && flag == 0 ){
//    digitalWrite( LED_PIN, HIGH);
//    if( UDP_Tx.beginPacket( followerIpAddress1, FOLLOWER_PORT_RX) ){
//      UDP_Tx.write("H");
//      UDP_Tx.endPacket();
//      digitalWrite( LED2_PIN, HIGH);
//      flag = 1;
//    }
//  }

//  if(Serial.available() > 0) { 
//    char data[2];
//    data[0] = Serial.read();
//    data[1] = '\0';
//    if( UDP_Tx.beginPacket( followerIpAddress1, FOLLOWER_PORT_RX) ){
//      UDP_Tx.write(data);
//      UDP_Tx.endPacket();
//      digitalWrite( LED2_PIN, HIGH);
//      flag = 1;
//    }
//  } 
  
  if(UDP_Rx.parsePacket() > 0){
    
    rcvWiFi();
    Serial.println("11");
    if(WiFibuff[0] == '1'){
      Serial.println("12");
      //digitalWrite( LED2_PIN, HIGH);
    }
    else if( WiFibuff[0] == '2' ){
      Serial.println("13");
    }
  }
  //Serial.println(".");
//  end_loop();
}

void end_loop(){
  
  if (WiFi.status() != WL_CONNECTED) {
    
    WiFi.disconnect();
    connectWiFi();
  }
}
*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <WiFiUDP.h>
#include "wifiConfig.h"

const char ssid[] = MY_SSID; // SSID
const char pass[] = MY_SSID_PASSWORD;  // password
const int localPort = 10000;      // 受信ポート番号

const IPAddress ip(192, 168, 4, 1);       // IPアドレス(ゲートウェイも兼ねる)
const IPAddress subnet(255, 255, 255, 0); // サブネットマスク

WiFiUDP udp;
static const char *kRemoteIpadr = "192.168.4.2";  //送信先のIPアドレス
static const int kRmoteUdpPort = 5000; //送信先のポート

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, pass);           // SSIDとパスの設定
  delay(100);                        // 追記：このdelayを入れないと失敗する場合がある
  WiFi.softAPConfig(ip, ip, subnet); // IPアドレス、ゲートウェイ、サブネットマスクの設定

  Serial.print("AP IP address: ");
  IPAddress myIP = WiFi.softAPIP();
  Serial.println(myIP);

  Serial.println("Starting UDP");
  udp.begin(localPort);  // UDP通信の開始(引数はポート番号)

  Serial.print("Local port: ");
  Serial.println(localPort);
}

void loop() {
  udp.beginPacket(kRemoteIpadr, kRmoteUdpPort);
  udp.write('a');  //10進数のaskiiで送信される
  udp.endPacket();

  delay(3000);
}
