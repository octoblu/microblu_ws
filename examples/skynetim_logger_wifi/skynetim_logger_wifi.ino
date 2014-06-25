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
 * This sketch parses any messages it receives and echos them to the Serial.
 *
 * Works with ethernet shields compatible with EthernetClient library from
 * Arduino. If you don't know, grab the original
 * http://arduino.cc/en/Main/ArduinoEthernetShield
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
#include "SkynetClient.h"
#include "jsmn.h"
#include "SPI.h"
#include <WiFi.h>

WiFiClient client;

SkynetClient skynetclient(client);

char ssid[] = "yournetworkname";     //  your network SSID (name)
char pass[] = "yourpassword";  // your WPA network password
// char key[] = "D0D0DEADF00DABBADEAFBEADED";       // your WEP network key
// int keyIndex = 0;                                // your WEP network key Index number

char hostname[] = "skynet.im";
int port = 80;

int wifiStatus = WL_IDLE_STATUS;

void setup()
{
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
  
}

void loop() {
  
  while (wifiStatus != WL_CONNECTED) 
  {
    Serial.print(F("Attempting to connect to WPA SSID: "));
    Serial.println(ssid);

    wifiStatus = WiFi.begin(ssid, pass); //begin WPA
    // status = WiFi.begin(ssid, keyIndex, key); //begin WEP
  }

  while(!skynetclient.monitor())
  {
    bool skynetStatus = false;
    do {
      skynetStatus = skynetclient.connect(hostname, port);
    } while (!skynetStatus);
    
    Serial.println(F("Connected!"));
    
    char uuid[UUIDSIZE];
  
    skynetclient.getUuid(uuid);
    Serial.print(F("uuid: "));
    Serial.println(uuid);
    
    skynetclient.getToken(uuid);
    Serial.print(F("token: "));
    Serial.println(uuid);   
  }
  
  //message to log MUST be comma seperated key value pair(s) and may
  //not be be an object or array
  //looks like "light":"423","temp":"356"
  String messageString = "\"light\":" + String(int(analogRead(A0))) + ",\"temp\":" + String(int(analogRead(A1)));
  
  skynetclient.logMessage(messageString);
  delay(1000);
}
