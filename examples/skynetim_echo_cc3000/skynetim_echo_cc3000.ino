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
 * This sketch parses any messages it receives and echos them back to the
 * sender.
 *
 * This sketch requires a port of the Adafruit CC3000 firmare library. Until
 * Adafruit accepts it, delete theirs and grab ours at:
 * https://github.com/jacobrosenthal/Adafruit_CC3000_Library/tree/feature/inherit-client
 *
 * Also requires the ArduinoJsonParser 
 * https://github.com/bblanchon/ArduinoJsonParser 
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
#include <Adafruit_CC3000.h>
#include <ccspi.h>
#include <string.h>
#include "utility/debug.h"
#include "SPI.h"
#include "SkynetClient.h"
#include <JsonParser.h>


// These are the interrupt and control pins
#define ADAFRUIT_CC3000_IRQ   3  // MUST be an interrupt pin!
// These can be any two pins
#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10
// Use hardware SPI for the remaining pins
// On an UNO, SCK = 13, MISO = 12, and MOSI = 11
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT,
                                         SPI_CLOCK_DIVIDER); // you can change this clock speed

#define WLAN_SSID       "myNetwork"           // cannot be longer than 32 characters!
#define WLAN_PASS       "myPassword"
// Security can be WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2
#define WLAN_SECURITY   WLAN_SEC_WPA2

#define IDLE_TIMEOUT_MS  3000      // Amount of time to wait (in milliseconds) with no data 
                                   // received before closing the connection.  If you know the server
                                   // you're accessing is quick to respond, you can reduce this value.
                
const char HOSTNAME[] = "skynet.im";
const uint8_t PORT = 80;

Adafruit_CC3000_Client client = Adafruit_CC3000_Client();
  
SkynetClient skynetclient(client);
  
void setup()
{
  Serial.begin(9600);
  
  /* Initialise the module */
  Serial.println(F("\nInitializing..."));
  if (!cc3000.begin())
  {
    Serial.println(F("Couldn't begin()! Check your wiring?"));
    while(1);
  }
  
  if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
    Serial.println(F("Failed!"));
    while(1);
  }
  
  Serial.println(F("Connected!"));
  
  /* Wait for DHCP to complete */
  Serial.println(F("Request DHCP"));
  while (!cc3000.checkDHCP())
  {
    delay(100); // ToDo: Insert a DHCP timeout!
  }  
  
  skynetclient.setMessageDelegate(onMessage);

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
  while(!skynetclient.monitor()){
    bool skynetStatus = false;
    do {
      skynetStatus = skynetclient.connect(HOSTNAME, PORT);
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
}