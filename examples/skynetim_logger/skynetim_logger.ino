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
 * This sketch send data to the Skynet data endpoint for logging and graphing
 * by skynet itself!
 *
 * Works with ethernet shields compatible with EthernetClient library from
 * Arduino. If you don't know, grab the original 
 * http://arduino.cc/en/Main/ArduinoEthernetShield
 * 
 * You will notice we're using F() in Serial.print which might be new to you
 * Its covered briefly on the arduino print page but it means we can store
 * our strings in flash, instead of in ram. 
 * 
 * You can turn on debugging within SkynetClient.h by uncommenting 
 * #define SKYNETCLIENT_DEBUG
 */

#include <EEPROM.h>
#include "Ethernet.h"
#include "SPI.h"
#include "SkynetClient.h"
#include <JsonParser.h>

EthernetClient client;

SkynetClient skynetclient(client);

//you can't have 2 of the same mac on your network!
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
char hostname[] = "skynet.im";
int port = 80;

void setup()
{
  Serial.begin(9600);
  
  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println(F("Failed to configure Ethernet using DHCP"));
    // no point in carrying on, so do nothing forevermore:
    for(;;)
      ;
  }
}

void loop() {
  while(!skynetclient.monitor()){
    
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
  skynetclient.logMessage("\"humidity\":123,\"temperature\":\"21C\"");
  delay(1000);
}