 
                            SSSSS  kk                            tt    
                           SS      kk  kk yy   yy nn nnn    eee  tt    
                            SSSSS  kkkkk  yy   yy nnn  nn ee   e tttt  
                                SS kk kk   yyyyyy nn   nn eeeee  tt    
                            SSSSS  kk  kk      yy nn   nn  eeeee  tttt 
                                           yyyyy                         
 
Skynet Firmware allows you to connect to Skynet via your Arduino and an Arduino ethernet or wifi shield (or any other device that properly implements the Client interface)
 * http://arduino.cc/en/Main/ArduinoEthernetShield
 * http://arduino.cc/en/Main/ArduinoBoardEthernet
 * http://arduino.cc/en/Main/ArduinoWiFiShield

##Install
* Clone or download and unzip.
* Rename the resulting folder to remove any - characters, and place folder in the libraries directory of your Arduino sketch directory. EX Documents/Arduino/libraries/skynet_firmware_arduino_master
* Also requires the ArduinoJsonParser library https://github.com/bblanchon/ArduinoJsonParser  
* After adding both, Close/reopen arduino and if all is well you'll find File->Examples->SkynetClient

##Examples
Find full examples in the File->Examples->SkynetClient menu but generally, theres 2 ways to use onMessage function or bind.
###Function
The simplest method is to register a function to be called when Skynet receives a message for your device:
```cpp
void setup()
{
  ...
  skynetclient.setMessageDelegate(onMessage);
}
void loop(){
  skynetclient.monitor();
}
```

Then you can do whatever you want with that message including json parse it or string match it:
```cpp
void onMessage(const char * const data) {
  JsonParser<16> parser;

  Serial.print("Parse ");
  Serial.println(data);

  JsonHashTable hashTable = parser.parseHashTable((char*)data);

  if (strcmp(payload, "on") == 0)
  {
      digitalWrite(LEDPIN, HIGH)
  }else{
      digitalWrite(LEDPIN, LOW)
  }
}
```
###Bind
Secondly, we've created the ability to bind 2 devices like a virtual serial cable. Once complete you can simple read and write like a Serial device:
```cpp
void loop() {
  skynetclient.monitor();
  while(skynetclient.available())
  	Serial.print(skynetclient.read());
}
```
This can be seen in our Firmata example using https://www.npmjs.org/package/skynet-serial. 
