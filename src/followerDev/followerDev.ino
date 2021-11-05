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

WiFiUDP wifiUdp; 
static const char *kRemoteIpadr = "192.168.4.1";  //送信先のIPアドレス
static const int kRmoteUdpPort = 10000; //送信先のポート

const IPAddress ip(192, 168, 4, 2);       // IPアドレス(ゲートウェイも兼ねる)
const IPAddress defge(192, 168, 4, 1);       // IPアドレス(ゲートウェイも兼ねる)
const IPAddress subnet(255, 255, 255, 0); // サブネットマスク

static void WiFi_setup()
{
  static const int kLocalPort = 5000;  //自身のポート
  WiFi.mode(WIFI_STA);//重要!
  WiFi.begin(ssid, pass);
  delay(100);
  WiFi.config(ip, defge, subnet);
  while( WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println(".");
  }
  Serial.print("AP IP address: ");
  IPAddress myIP = WiFi.softAPIP();
  Serial.println(myIP);

  Serial.println("Starting UDP");
  wifiUdp.begin(kLocalPort);
}

static void Serial_setup()
{
  Serial.begin(115200);
  Serial.println(""); // to separate line  
}

void setup() {
  Serial_setup();
  WiFi_setup();
}

void loop() 
{
  if (wifiUdp.parsePacket()) {
    char i = wifiUdp.read();  //ceramie追記 askiiから文字列へ
    wifiUdp.flush();
    Serial.println(i); // UDP通信で来た値を表示
  }
}
