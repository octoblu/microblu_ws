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
 * Go into bitlash/src/bitlash.h and change #define STARTDB 0 to #define STARTDB 71
 * or your first couple bitlash function will look an awful lot like token and uuids.. 
 * which might be fine with you...
 *
 * You can turn on debugging within SkynetClient.h by uncommenting 
 * #define SKYNETCLIENT_DEBUG
 */
 

#include <SPI.h>
#include <Wire.h>
#include <Scout.h>
#include <GS.h>
#include <bitlash.h>
#include <lwm.h>
#include <js0n.h>

#include "version.h"

#include <EEPROM.h>
#include "Ethernet.h"
#include "SPI.h"
#include "SkynetClient.h"
#include <JsonParser.h>

GSTcpClient client(Scout.wifi.gs);

SkynetClient skynetclient(client);

void setup() {
  
  addBitlashFunction("skynet.uuid", (bitlash_function)skynetuuid);
  addBitlashFunction("skynet.token", (bitlash_function)skynettoken);
  addBitlashFunction("skynet.setuuid", (bitlash_function)skynetsetuuid);
  addBitlashFunction("skynet.settoken", (bitlash_function)skynetsettoken);
  addBitlashFunction("skynet.connected", (bitlash_function)skynetconnected);
  addBitlashFunction("skynet.sendmessage", (bitlash_function)skynetsendmessage);
  addBitlashFunction("skynet.logmessage", (bitlash_function)skynetlogmessage);
  addBitlashFunction("skynet.connect", (bitlash_function)skynetconnect);

  Scout.setup(SKETCH_NAME, SKETCH_REVISION, SKETCH_BUILD);
  // Add custom setup code here
}

void loop() {
  Scout.loop();

  skynetclient.monitor();

  // Add custom loop code here
}

void skynetconnected (){
  speol(skynetclient.connected());
}

void skynetuuid (){
    char uuid[UUIDSIZE];
    skynetclient.getUuid(uuid);
    speol(uuid);
}

numvar skynettoken (){
    char token[TOKENSIZE];
    skynetclient.getToken(token);
    speol(token);
}

numvar skynetsetuuid(){
    skynetclient.setUuid((char*)getstringarg(1));
}

numvar skynetsettoken(){
  skynetclient.setToken((char*)getstringarg(1));
}

numvar skynetsendmessage(){
  skynetclient.sendMessage((char*)getstringarg(1), (char*)getstringarg(2));
}

numvar skynetlogmessage(){
  skynetclient.logMessage((char*)getstringarg(1));
}

//ackward, but for now connect vi ip address and ports as ints ex:
//skynet.connect(54,186,40,188,80);
numvar skynetconnect(){
  IPAddress ip(getarg(1),getarg(2),getarg(3),getarg(4));
  speol(skynetclient.connect(ip, getarg(5)));
}
