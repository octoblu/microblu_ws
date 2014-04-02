 
                            SSSSS  kk                            tt    
                           SS      kk  kk yy   yy nn nnn    eee  tt    
                            SSSSS  kkkkk  yy   yy nnn  nn ee   e tttt  
                                SS kk kk   yyyyyy nn   nn eeeee  tt    
                            SSSSS  kk  kk      yy nn   nn  eeeee  tttt 
                                           yyyyy                         
 
 
* Clone or download and unzip.
* Rename the resulting folder to remove any - characters, and place folder in the libraries directory of your Arduino sketch directory. EX Documents/Arduino/libraries/skynet_firmware_arduino_master
* Also requires the ArduinoJsonParser library https://github.com/bblanchon/ArduinoJsonParser  
* After adding both, Close/reopen arduino and if all is well you'll find File->Examples->SkynetClient

Theres 2 ways to use Skynet. The simple method is to register a function to be called when Skynet receives a message for your device:
```cpp
void setup()
{
  ...
  skynetclient.setMessageDelegate(onMessage);
}
```

Then you can do whatever you want with that message including json parse it or string match it:
Parse the payload:
```cpp
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

  if (strcmp(payload, "on") == 0)
  {
  	digitalWrite(LEDPIN, HIGH)
  }
    
  char* payload = hashTable.getString("payload");
  Serial.print("payload=");
  Serial.println(payload);
}
```

Secondly, we've created the ability to bind 2 devices, like a virtual serial cable. This can be seen in our Firmata example:
```cpp
void loop() {
  skynetclient.monitor();
  while(skynetclient.available())
  	Serial.print(skynetclient.read());
}
```
