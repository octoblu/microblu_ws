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
 * This sketch sets your uuid and token into eeprom. SkynetClient will
 * get its own uuid and token, and will replace these if they're invalid, 
 * but sometimes you want to set known values.
 */

#include "SkynetClient.h"
#include <EEPROM.h>
#include "Ethernet.h"

#define UUID  "XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX"
#define TOKEN "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"

#include <JsonParser.h>

EthernetClient client; //just need a client to get a Skynet object
SkynetClient skynetclient(client);

void setup() {
  Serial.begin(9600);
  
  char uuid[UUIDSIZE];
  
  Serial.println("Reading uuid");
  skynetclient.getUuid(uuid);
  Serial.print("uuid: ");
  Serial.println(uuid);
  
  skynetclient.getToken(uuid);
  Serial.print("token: ");
  Serial.println(uuid); 
  
  Serial.println("Setting uuid:");
  skynetclient.setUuid(UUID);
  Serial.println("Setting token:");
  skynetclient.setToken(TOKEN);
  Serial.println("Set!");
  
  Serial.println("Reading back");
  skynetclient.getUuid(uuid);
  Serial.print("uuid: ");
  Serial.println(uuid);
  
  skynetclient.getToken(uuid);
  Serial.print("token: ");
  Serial.println(uuid); 
}

void loop() {

}
