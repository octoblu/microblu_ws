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

#include <utility/w5100.h>
#include <EEPROM.h>
#include "Ethernet.h"
#include "SPI.h"
#include "SkynetClient.h"
#include "jsmnSpark.h"

EthernetClient client;

SkynetClient skynetclient(client);

//you can't have 2 of the same mac on your network!
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
char hostname[] = "skynet.im";
int port = 80;

void setup()
{
  //delay to give you time to open a console so we don't hammer server
  delay(5000);
  Serial.begin(9600);
  
  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println(F("Failed to configure Ethernet using DHCP"));
    // no point in carrying on, so do nothing forevermore:
    for(;;)
      ;
  }
  
  //decrease tcp timeout, fail quicker so we can get on with things
  W5100.setRetransmissionTime(0x7D);
  
  // print your local IP address:
  Serial.print(F("My IP address: "));
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    // print the value of each byte of the IP address:
    Serial.print(Ethernet.localIP()[thisByte], DEC);
    Serial.print("."); 
  }
  Serial.println();
  
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
  //access your payload from skynet buffer
  while(skynetclient.available())
    Serial.print((char)skynetclient.read());
   
   //or send a reply
   skynetclient.sendMessage("Thanks!");  
}

void loop(){
  //need to call monitor to check for new data on ethernet
  skynetclient.monitor(); 
}