/* 
 *                SSSSS  kk                            tt    
 *               SS      kk  kk yy   yy nn nnn    eee  tt    
 *                SSSSS  kkkkk  yy   yy nnn  nn ee   e tttt  
 *                    SS kk kk   yyyyyy nn   nn eeeee  tt    
 *                SSSSS  kk  kk      yy nn   nn  eeeee  tttt 
 *                               yyyyy                         
 * 
 * SkynetClient for http://skynet.im, OPEN COMMUNICATIONS NETWORK & API FOR 
 * THE INTERNET OF THINGS.
 * 
 * This sketch is VERY low on ram. Currently it only works on an Arduino Mega
 * (4x the ram of an Uno)
 *
 * Works with ethernet shields compatible with EthernetClient library from
 * Arduino. If you don't know, grab the original 
 * http://arduino.cc/en/Main/ArduinoEthernetShield
 * 
 * Also requires the Arduino-zed JSMNSpark json parsing library 
 * https://github.com/jacobrosenthal/JSMNSpark 
 * 
 * You will notice we're using F() in Serial.print which might be new to you
 * Its covered briefly on the arduino print page but it means we can store
 * our strings in flash, instead of in ram. 
 * 
 * You can turn on debugging within SkynetClient.h by uncommenting 
 * #define SKYNETCLIENT_DEBUG
 */

#include <EEPROM.h>
#include <WiFi.h>
#include "SPI.h"
#include "SkynetClient.h"
#include "jsmnSpark.h"

//avoid pins 10 11 12 13 plus if you're using sd card 4, and additionally pin 7 if wifi
#define REDLED 6
#define BLUELED 5
#define GREENLED 4

SkynetClient skynetclient;

char ssid[] = "yournetworkname";     //  your network SSID (name)
char pass[] = "yourpassword";  // your WPA network password
// char key[] = "D0D0DEADF00DABBADEAFBEADED";       // your WEP network key
// int keyIndex = 0;                                // your WEP network key Index number

int status = WL_IDLE_STATUS;     // the Wifi radio's status

char hostname[] = "skynet.im";
int port = 80;

void setup()
{
  pinMode(REDLED, OUTPUT);
  pinMode(GREENLED, OUTPUT);
  pinMode(BLUELED, OUTPUT);
	
  //delay to give you time to open a console so we don't hammer server
  delay(5000);
  Serial.begin(9600);
  
  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println(F("WiFi shield not present"));
    // don't continue:
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if ( fv != "1.1.0" )
    Serial.println(F("Please upgrade the firmware"));

  // attempt to connect to Wifi network:
  while ( status != WL_CONNECTED) {
    Serial.print(F("Attempting to connect to WPA SSID: "));
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass); //begin WPA
    // status = WiFi.begin(ssid, keyIndex, key); //begin WEP

    // wait 10 seconds for connection:
    delay(10000);
  }

  // you're connected now, so print out the data:
  Serial.print(F("You're connected to the network"));
  printCurrentNet();
  printWifiData();
  
  skynetclient.setMessageDelegate(onMessage);

  bool status;
  do {
    status=skynetclient.connect(hostname, port);
  }while(!status);
  
  Serial.println(F("Connected!"));
  Serial.print(F("uuid: "));
  Serial.println(skynetclient.uuid);
  Serial.print(F("token: "));
  Serial.println(skynetclient.token);
}

void onMessage(char *data){
  //print your payload from skynet buffer
  while(skynetclient.available())
    Serial.print((char)skynetclient.read());
  Serial.println();
  
  //or parse for something inth the data structure
  jsmn_parser p;
  jsmntok_t token[64];
  jsmn_init(&p);
  
  int r = jsmn_parse(&p, data, token, 64);
  if (r != 0)
  {
    Serial.print(F("Parse Failed :("));
    Serial.println(r);
  }else
  {
  	int sizeoftoken = token[10].end - token[10].start;
   	char color[sizeoftoken + 1]; //space for null char
    strncpy(color, data + token[10].start, token[10].end - token[10].start);
  	color[sizeoftoken] = '\0'; //place the null char
    Serial.print("color: ");
    Serial.println(color);
    
  	sizeoftoken = token[11].end - token[11].start;
    char value[sizeoftoken + 1]; //space for null char
    strncpy(value, data + token[11].start, token[11].end - token[11].start);
  	value[sizeoftoken] = '\0'; //place the null char
    Serial.print("status: ");
    Serial.println(value);
	
	if (strcmp("red", color) == 0 && strcmp("on", value) == 0)
	{
        Serial.println(F("turning red on"));
	    digitalWrite(REDLED, HIGH);
	}else if (strcmp("red", color) == 0 && strcmp("off", value) == 0)
	{
        Serial.println(F("turning red off"));
	    digitalWrite(REDLED, LOW);
	}else if (strcmp("blue", color) == 0 && strcmp("on", value) == 0)
	{
        Serial.println(F("turning blue on"));
	    digitalWrite(BLUELED, HIGH);
	}else if (strcmp("blue", color) == 0 && strcmp("off", value) == 0)
	{
        Serial.println(F("turning blue off"));
	    digitalWrite(BLUELED, LOW);
	}else if (strcmp("green", color) == 0 && strcmp("on", value) == 0)
	{
        Serial.println(F("turning green on"));
	    digitalWrite(GREENLED, HIGH);
	}else if (strcmp("green", color) == 0 && strcmp("off", value) == 0)
	{
        Serial.println(F("turning green on"));
	    digitalWrite(GREENLED, LOW);
	}
  }
}

void loop(){
  //need to call monitor to check for new data on ethernet
  skynetclient.monitor(); 
}

void printWifiData() {
  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print(F("IP Address: "));
  Serial.println(ip);
  Serial.println(ip);

  // print your MAC address:
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print(F("MAC address: "));
  Serial.print(mac[5], HEX);
  Serial.print(":");
  Serial.print(mac[4], HEX);
  Serial.print(":");
  Serial.print(mac[3], HEX);
  Serial.print(":");
  Serial.print(mac[2], HEX);
  Serial.print(":");
  Serial.print(mac[1], HEX);
  Serial.print(":");
  Serial.println(mac[0], HEX);

}

void printCurrentNet() {
  // print the SSID of the network you're attached to:
  Serial.print(F("SSID: "));
  Serial.println(WiFi.SSID());

  // print the MAC address of the router you're attached to:
  byte bssid[6];
  WiFi.BSSID(bssid);
  Serial.print(F("BSSID: "));
  Serial.print(bssid[5], HEX);
  Serial.print(":");
  Serial.print(bssid[4], HEX);
  Serial.print(":");
  Serial.print(bssid[3], HEX);
  Serial.print(":");
  Serial.print(bssid[2], HEX);
  Serial.print(":");
  Serial.print(bssid[1], HEX);
  Serial.print(":");
  Serial.println(bssid[0], HEX);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print(F("signal strength (RSSI):"));
  Serial.println(rssi);

  // print the encryption type:
  byte encryption = WiFi.encryptionType();
  Serial.print(F("Encryption Type:"));
  Serial.println(encryption, HEX);
  Serial.println();
}