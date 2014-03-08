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

//Blue LED built in on spark on d7
#define REDLED 6
#define BLUELED 7
#define GREENLED 5

SkynetClient skynetclient;

//you can't have 2 of the same mac on your network!
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
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
  Serial.println();
  
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
        Serial.println("turning red on");
	    digitalWrite(REDLED, HIGH);
	}else if (strcmp("red", color) == 0 && strcmp("off", value) == 0)
	{
        Serial.println("turning red off");
	    digitalWrite(REDLED, LOW);
	}else if (strcmp("blue", color) == 0 && strcmp("on", value) == 0)
	{
        Serial.println("turning blue on");
	    digitalWrite(BLUELED, HIGH);
	}else if (strcmp("blue", color) == 0 && strcmp("off", value) == 0)
	{
        Serial.println("turning blue off");
	    digitalWrite(BLUELED, LOW);
	}else if (strcmp("green", color) == 0 && strcmp("on", value) == 0)
	{
        Serial.println("turning green on");
	    digitalWrite(GREENLED, HIGH);
	}else if (strcmp("green", color) == 0 && strcmp("off", value) == 0)
	{
        Serial.println("turning green on");
	    digitalWrite(GREENLED, LOW);
	}
  }
}

void loop(){
  //need to call monitor to check for new data on ethernet
  skynetclient.monitor(); 
}