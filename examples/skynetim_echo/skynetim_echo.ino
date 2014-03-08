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
 * Works with the Spark core https://www.spark.io/
 * 
 * Requires the JSMNSpark json parsing library https://github.com/pkourany/JSMNSpark
 * 
 * You can turn on debugging within SkynetClient.h by uncommenting 
 * #define SKYNETCLIENT_DEBUG
 */

#include "SkynetClient.h"
#include "jsmnSpark.h"

SkynetClient skynetclient;

//you can't have 2 of the same mac on your network!
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
char hostname[] = "skynet.im";
int port = 80;

void setup()
{
  //delay to give you time to open a console so we don't hammer server
  delay(5000);
  Serial.begin(9600);
  
  skynetclient.setMessageDelegate(onMessage);

  bool status;
  do {
    status=skynetclient.connect(hostname, port);
  }while(!status);
  
  Serial.println("Connected!");
  Serial.print("uuid: ");
  Serial.println(skynetclient.uuid);
  Serial.print("token: ");
  Serial.println(skynetclient.token);
}

void onMessage(char *data){
  //print your payload from skynet buffer
  while(skynetclient.available())
    Serial.print((char)skynetclient.read());
  
  //or parse for something inth the data structure
  jsmn_parser p;
  jsmntok_t token[64];
  jsmn_init(&p);
  
  int r = jsmn_parse(&p, data, token, 64);
  if (r != 0)
  {
    Serial.print("Parse Failed :(");
    Serial.println(r);
  }else
  {
    char fromUuid[token[15].size+1];
    strncpy(fromUuid, data + token[15].start, token[15].end - token[15].start);

    Serial.print("return address:");
    Serial.println(fromUuid);
    skynetclient.sendMessage(fromUuid, "Thanks!");
  }
}

void loop(){
  //need to call monitor to check for new data on ethernet
  skynetclient.monitor(); 
}