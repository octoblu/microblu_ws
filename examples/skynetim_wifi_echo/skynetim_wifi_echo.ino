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
 * Works with ethernet shields compatible with EthernetClient library from
 * Arduino. If you don't know, grab the original
 * http://arduino.cc/en/Main/ArduinoEthernetShield
 *
 * Also requires the ArduinoJsonParser 
 * https://github.com/bblanchon/ArduinoJsonParser 
 *
 * This sketch is VERY big both in program space and ram.
 *
 * You will notice we're using F() in Serial.print. It is covered briefly on
 * the arduino print page but it means we can store our strings in program
 * memory instead of in ram.
 *
 * You can turn on debugging within SkynetClient.h by uncommenting
 * #define SKYNETCLIENT_DEBUG but note this takes up a ton of program space
 * which means you'll probably have to debug on a Mega
 */

#include <EEPROM.h>
#include <WiFi.h>
#include "SPI.h"
#include "SkynetClient.h"
#include <JsonParser.h>

WiFiClient client;

SkynetClient skynetclient(client);

char ssid[] = "yournetworkname";     //  your network SSID (name)
char pass[] = "yourpassword";  // your WPA network password
// char key[] = "D0D0DEADF00DABBADEAFBEADED";       // your WEP network key
// int keyIndex = 0;                                // your WEP network key Index number

char hostname[] = "skynet.im";
int port = 80;

void setup()
{
  Serial.begin(9600);

  int wifiStatus = WL_IDLE_STATUS;
  do {
    Serial.print(F("Attempting to connect to WPA SSID: "));
    Serial.println(ssid);

    wifiStatus = WiFi.begin(ssid, pass); //begin WPA
    // status = WiFi.begin(ssid, keyIndex, key); //begin WEP
  } while ( wifiStatus != WL_CONNECTED);

  skynetclient.setMessageDelegate(onMessage);

  bool skynetStatus = false;
  do {
    skynetStatus = skynetclient.connect(hostname, port);
  } while (!skynetStatus);

  Serial.println(F("Connected!"));
  Serial.print(F("uuid: "));
  Serial.println(skynetclient.uuid);
}

void onMessage(const char * const data) {
  
  JsonParser<16> parser;

  Serial.print("Parse ");
  Serial.println(data);

  JsonHashTable hashTable = parser.parseHashTable((char*)data);

  if (!hashTable.success())
  {
      Serial.println("JsonParser.parseHashTable() failed");
      return;
  }
    
  char* payload = hashTable.getString("payload");
  Serial.print("payload=");
  Serial.println(payload);
    
  char* fromUuid = hashTable.getString("fromUuid");
  Serial.print("fromUuid=");
  Serial.println(fromUuid);

  skynetclient.sendMessage(fromUuid, payload);
}

void loop() {
  //need to call monitor to check for new data
  skynetclient.monitor();
}