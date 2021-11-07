/*#include <ESP8266WiFi.h>
//#include <WiFiClient.h> 
#include <WiFiUDP.h>

#include "wifiConfig.h"

#define LED_PIN 4
#define LED2_PIN 2

char WiFibuff[16];
int led_state = LOW;

// UDP setting
WiFiUDP UDP_Tx;
WiFiUDP UDP_Rx;

// WiFi settings
IPAddress myIP( FOLLOWER_IP_ADDRESS_1 );
IPAddress mySubnet( MY_SUBNETMASK );
IPAddress leaderIpAddress1( LEADER_IP_ADDRESS );
const char *ssid = MY_SSID;
const char *password = MY_SSID_PASSWORD;

int flag = 0;

void rcvWiFi() {
  UDP_Rx.read(WiFibuff, 1);
  UDP_Rx.flush();
}

void connectWiFi() {
  
  WiFi.begin( ssid, password);
  WiFi.config( myIP, WiFi.gatewayIP(), mySubnet);

  while( WiFi.status() != WL_CONNECTED ){
    delay(500);
    led_state = !led_state;
    digitalWrite( LED_PIN, led_state);
  }
  
  digitalWrite( LED_PIN, LOW);

  UDP_Tx.begin( FOLLOWER_PORT_TX );
  UDP_Rx.begin( FOLLOWER_PORT_RX );
}

void setup() {

  pinMode( LED_PIN, OUTPUT);
  digitalWrite( LED_PIN, led_state);
  pinMode( LED2_PIN, OUTPUT);
  digitalWrite( LED2_PIN, LOW);
  
  WiFi.mode( WIFI_STA );
  connectWiFi();
}

void sendWiFi(char byteData[]) {
  
  if ( UDP_Tx.beginPacket( leaderIpAddress1, LEADER_PORT_RX1) ) {
    UDP_Tx.write(byteData);
    UDP_Tx.endPacket();
  }
}

void loop() {

  if( UDP_Rx.parsePacket() > 0 ){
    digitalWrite( LED2_PIN, HIGH);
    rcvWiFi();
    if(WiFibuff[0] == 'H'){
      digitalWrite( LED_PIN, HIGH);
      sendWiFi("2");
    }
  }

  delay(5000);
  sendWiFi("1");
  digitalWrite( LED2_PIN, LOW);
  digitalWrite( LED_PIN, LOW);
  
  end_loop();
}

void end_loop(){
  
  if (WiFi.status() != WL_CONNECTED) {
    
    WiFi.disconnect();
    connectWiFi();
  }
}
*/

#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
#include "wifiConfig.h"

const char ssid[] = MY_SSID; // SSID
const char pass[] = MY_SSID_PASSWORD;  // password

WiFiUDP udp_Tx;
WiFiUDP udp_Rx;
 
//static const char *kRemoteIpadr = "192.168.4.1";  //送信先のIPアドレス
//static const int kRmoteUdpPort = 10000; //送信先のポート

static const IPAddress leader_IP(LEADER_IP_ADDRESS);  //送信先のIPアドレス
static const int port_Rx = FOLLOWER_PORT_RX; //受信用のポート
static const int reader_1_Port_Rx = LEADER_PORT_RX1; //送信先のポート
static const int port_Tx = FOLLOWER_PORT_TX; //送信用のポート

const IPAddress ip(FOLLOWER_IP_ADDRESS_1);       // IPアドレス(ゲートウェイも兼ねる)
const IPAddress defge(LEADER_IP_ADDRESS);       // IPアドレス(ゲートウェイも兼ねる)
const IPAddress subnet(MY_SUBNETMASK); // サブネットマスク

int led4;
int led2;

static void WiFi_setup()
{
  WiFi.mode(WIFI_STA);//重要!
  WiFi.begin(ssid, pass);
  delay(100);
  WiFi.config(ip, defge, subnet);
  while( WiFi.status() != WL_CONNECTED) {
    led4 = !led4;
    digitalWrite( 4, led4);
    delay(500);
    Serial.println(".");
  }
  digitalWrite( 4, HIGH);
  Serial.print("AP IP address: ");
  IPAddress myIP = WiFi.softAPIP();
  Serial.println(myIP);

  Serial.println("Starting UDP");
  udp_Tx.begin(port_Tx);  // UDP通信の開始(引数はポート番号)
  udp_Rx.begin(port_Rx);  // UDP通信の開始(引数はポート番号)
}

static void Serial_setup()
{
  Serial.begin(115200);
  Serial.println(""); // to separate line  
}

void setup() {
  pinMode( 4, OUTPUT);
  pinMode( 2, OUTPUT);
  led4 = LOW;
  led2 = LOW;
  digitalWrite( 4, led4);
  digitalWrite( 2, led2);
  Serial_setup();
  WiFi_setup();
}

void loop() 
{
  if (udp_Rx.parsePacket()) {
    led2 = !led2;
    digitalWrite( 2, led2);
    char i = udp_Rx.read();  //ceramie追記 askiiから文字列へ
    udp_Rx.flush();
    udp_Tx.beginPacket(leader_IP, reader_1_Port_Rx);
    udp_Tx.write(i);  //10進数のaskiiで送信される
    udp_Tx.endPacket();
    Serial.println(i); // UDP通信で来た値を表示
  }
}
