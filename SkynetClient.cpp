
#include <SkynetClient.h>

// EthernetClient client;
WiFiClient client;
aJsonClientStream serial_stream(&client);

// struct ring_buffer
// {
//   unsigned char buffer[SERIAL_BUFFER_SIZE];
//   volatile unsigned int head;
//   volatile unsigned int tail;
// };
// 
// ring_buffer socket_rx_buffer =    { { 0 }, 0, 0};
// 
// inline void store_char(unsigned char c, ring_buffer *buffer)
// {
//   int i = (unsigned int)(buffer->head + 1) % SERIAL_BUFFER_SIZE;
// 
//   // if we should be storing the received character into the location
//   // just before the tail (meaning that the head would advance to the
//   // current location of the tail), we're about to overflow the buffer
//   // and so we don't write the character or advance the head.
//   if (i != buffer->tail) {
//     buffer->buffer[buffer->head] = c;
//     buffer->head = i;
//   }
// }

SkynetClient::SkynetClient(){
}

int SkynetClient::connect(const char* host, uint16_t port) {
	IPAddress remote_addr;
	if (WiFi.hostByName(host, remote_addr))
	{
		return connect(remote_addr, port);
	}
}

int SkynetClient::connect(IPAddress ip, uint16_t port) {

	// _rx_buffer = &s	ocket_rx_buffer;
	theip = ip;
	theport = port;

	//connect tcp or fail
	if (!client.connect(theip, theport)) 
		return false;

	//establish socket or fail
	sendHandshake();
	if(!readHandshake()){
		stop();
		return false;
	}
	
	//monitor to initiate communications with skynet TODO some fail condition
	while(!status)
		monitor();
	
	return status;
}

uint8_t SkynetClient::connected() {
  return status;
}

void SkynetClient::stop() {
	status = 0;
	client.stop();
}

void SkynetClient::dump(int x){

	while ( client.available() && x>0 ){
		client.read();
		x--;
	}
	
}
void SkynetClient::monitor() {
	char which = 0;
	
	//we need characters, and the first should be a null char
	if(!client.available() || client.read()!=0)
		return;
	
	if(client.available())
		which = client.read();
	
	switch (which) {
		//disconnect	
		case '0':
			stop();
		break;
		
		//messages
		case '1':
			//kill 5 chars
			dump(5); 

		case '3':	
		case '5':	

			//kill 3 chars
			dump(3); 
			
			process();
			break;
			
		//hearbeat
		case '2':
			client.print((char)0);
			client.print(F("2::"));
			client.print((char)255);
			DBGCN(F("Heartbeat"));
			break;

		default: 
			DBGC(F("Drop: "));
			DBGCN(which);

			break;
	}
	
	client.flush();
}

void SkynetClient::process()
{
  if (serial_stream.available()) {
	  
    msg = aJson.parse(&serial_stream);
	
    temp = aJson.getObjectItem(msg, NAME);
	
    parsedArgs = aJson.getObjectItem(msg, ARGS);

    DBGC(F("Parsed: "));
    if (strcmp(temp->valuestring, IDENTIFY) == 0) 
    {
      DBGCN(IDENTIFY);

      parsedArgsZero = aJson.getArrayItem(parsedArgs, 0);
      temp = aJson.getObjectItem(parsedArgsZero, SOCKETID);
      
      reply=aJson.createObject();
      args=aJson.createObject();
      argsArray=aJson.createArray();
  
      DBGC(F("socketid: "));
      DBGCN(temp->valuestring);

      eeprom_read_bytes(token, TOKENADDRESS, TOKENSIZE);
      token[TOKENSIZE-1]='\0'; //in case courrupted or not defined
      DBGC(F("token loaded from eeprom: "));
      DBGCN(token);
      
      eeprom_read_bytes(uuid, UUIDADDRESS, UUIDSIZE);
      uuid[UUIDSIZE-1]='\0'; //in case courrupted or not defined
      DBGC(F("uuid loaded from eeprom: "));
      DBGCN(uuid);

      aJson.addStringToObject(args, SOCKETID, temp->valuestring);
      aJson.addStringToObject(args, UUID, uuid);
      aJson.addStringToObject(args, TOKEN, token);
      
      aJson.addItemToArray(argsArray,args);
      aJson.addStringToObject(reply, NAME, IDENTITY);
      aJson.addItemToObject(reply, ARGS, argsArray);

      DBGC(F("Sending: "));
      DBGCN(aJson.print(reply));
      send(EMIT, aJson.print(reply));

////      aJson.deleteItem(reply);
////      aJson.deleteItem(args);
////      aJson.deleteItem(argsArray);

    } 
    else if (strcmp(temp->valuestring, READY) == 0)
    {
      DBGCN(READY);
	  status = 1;

      parsedArgsZero = aJson.getArrayItem(parsedArgs, 0);

      temp = aJson.getObjectItem(parsedArgsZero, TOKEN);
      
      eeprom_read_bytes(token, TOKENADDRESS, TOKENSIZE);
      token[TOKENSIZE-1]='\0'; //in case courrupted or not defined

      //if token has been refreshed, save it
      if (strcmp(temp->valuestring, token) != 0)
      {
        DBGCN(F("token refresh"));
        DBGC(F("old: "));
        DBGCN(token);
        DBGC(F("new: "));
        DBGCN(temp->valuestring);
        eeprom_write_bytes(temp->valuestring, TOKENADDRESS, TOKENSIZE);
      }else
      {
        DBGCN(F("no token refresh necessary"));
      }

      temp = aJson.getObjectItem(parsedArgsZero, UUID);
  
      eeprom_read_bytes(uuid, UUIDADDRESS, UUIDSIZE);
      uuid[UUIDSIZE-1]='\0'; //in case courrupted or not defined

      //if uuid has been refreshed, save it
      if (strcmp(temp->valuestring, uuid) != 0)
      {
        DBGCN(F("uuid refresh"));
        DBGC(F("old: "));
        DBGCN(uuid);
        DBGC(F("new: "));
        DBGCN(temp->valuestring);
        eeprom_write_bytes(temp->valuestring, UUIDADDRESS, UUIDSIZE);
      }else
      {
        DBGCN(F("no uuid refresh necessary"));
      }


    }
    else if (strcmp(temp->valuestring, NOTREADY) == 0)
    {
      //send another identify
      DBGCN(NOTREADY);
      
      parsedArgsZero = aJson.getArrayItem(parsedArgs, 0);
      temp = aJson.getObjectItem(parsedArgsZero, SOCKETID);
     
      DBGCN(temp->valuestring);
      
      reply=aJson.createObject();
      args=aJson.createObject();
      argsArray=aJson.createArray();
  
      DBGC("socketid: ");
      DBGCN(temp->valuestring);

      aJson.addStringToObject(args, SOCKETID, temp->valuestring);
      
      aJson.addItemToArray(argsArray,args);
      aJson.addStringToObject(reply, NAME, IDENTITY);
      aJson.addItemToObject(reply, ARGS, argsArray);

      DBGC(aJson.print(reply));
      send(EMIT, aJson.print(reply));
      
    }
    else if (strcmp(temp->valuestring, MESSAGE) == 0)
    {
      //arg0 is recipient (us), could check against our uuid, or find * and know it was broadcast
      //grab arg1 and read as base64
      //arg2 if available is who sent
      DBGCN(MESSAGE);
      
      parsedArgsZero = aJson.getArrayItem(parsedArgs, 1);
      DBGC(F("message: "));
      DBGCN(aJson.print(parsedArgsZero));
	  
		if (messageDelegate != NULL) {
			messageDelegate(parsedArgsZero);
		}
    }
    else
    {
      DBGC(F("Unknown:"));
      DBGCN(temp->valuestring);

    }
//    aJson.deleteItem(msg); //supposedly just delete the root and takes care of everything
  }
  
}

void SkynetClient::sendHandshake() {
	client.println(F("GET /socket.io/1/ HTTP/1.1"));
	client.print(F("Host: "));
	client.println(theip);
	client.println(F("Origin: Arduino\r\n"));
}

bool SkynetClient::waitForInput(void) {
unsigned long now = millis();
	while (!client.available() && ((millis() - now) < 30000UL)) {;}
	return client.available();
}

void SkynetClient::eatHeader(void) {
	while (client.available()) {	// consume the header
		readLine();
		if (strlen(databuffer) == 0) break;
	}
}

int SkynetClient::readHandshake() {

	if (!waitForInput()) return false;

	// check for happy "HTTP/1.1 200" response
	readLine();
	if (atoi(&databuffer[9]) != 200) {
		while (client.available()) readLine();
		client.stop();
		return 0;
	}
	eatHeader();
	readLine();	// read first line of response
	readLine();	// read sid : transport : timeout

	char *iptr = databuffer;
	char *optr = sid;
	while (*iptr && (*iptr != ':') && (optr < &sid[SID_LEN-2])) *optr++ = *iptr++;
	*optr = 0;

	DBGC(F("Connected. SID="));
	DBGCN(sid);	// sid:transport:timeout 

	while (client.available()) readLine();
	// client.stop();
	// delay(1000);
	// 
	// // reconnect on websocket connection
	// DBGCN(F("WS Connect..."));
	// if (!client.connect(theip, theport)) {
	// 	DBGCN(F("Reconnect failed."));
	// 	return 0;
	// }
	// DBGCN(F("Reconnected."));

	client.print(F("GET /socket.io/1/websocket/"));
	client.print(sid);
	client.println(F(" HTTP/1.1"));
	client.print(F("Host: "));
	client.println(theip);
	client.println(F("Origin: ArduinoSkynetClient"));
	client.println(F("Upgrade: WebSocket"));	// must be camelcase ?!
	client.println(F("Connection: Upgrade\r\n"));

	if (!waitForInput()) return 0;

	readLine();
	if (atoi(&databuffer[9]) != 101) {
		while (client.available()) readLine();
		client.stop();
		return false;
	}
	eatHeader();
	monitor();		// treat the response as input
	return 1;
}

int SkynetClient::readLine() {
	int numBytes = 0;
	dataptr = databuffer;
	DBGC(F("Readline: "));
	while (client.available() && (dataptr < &databuffer[DATA_BUFFER_LEN-2])) {
		char c = client.read();
		if (c == 0){
			DBGC(F("NULL"));
		}else if (c == 255){
			DBGCN(F("0x255"));	
		}else if (c == '\r') {
			;
		}else if (c == '\n') 
			break;
		else {
			DBGC(c);
			*dataptr++ = c;
			numBytes++;
		}
	}
	DBGCN();
	*dataptr = 0;
	return numBytes;
}

void SkynetClient::send(char *encoding, char *data) {
	client.print((char)0);
	client.print(encoding);
	client.print(data);
	client.print((char)255);
}

size_t SkynetClient::write(uint8_t b) {
  return client.write(b);
}

size_t SkynetClient::write(const uint8_t *buf, size_t size) {
  return client.write(buf, size);
}

// int SkynetClient::available() {
//   return (unsigned int)(SERIAL_BUFFER_SIZE + _rx_buffer->head - _rx_buffer->tail) % SERIAL_BUFFER_SIZE;
// }

// int SkynetClient::read() {
//     // if the head isn't ahead of the tail, we don't have any characters
//     if (_rx_buffer->head == _rx_buffer->tail) {
//       return -1;
//     } else {
//       unsigned char c = _rx_buffer->buffer[_rx_buffer->tail];
//       _rx_buffer->tail = (unsigned int)(_rx_buffer->tail + 1) % SERIAL_BUFFER_SIZE;
//       return c;
//     }
// }

// //TODO	
// int SkynetClient::read(uint8_t *buf, size_t size) {
//     // // if the head isn't ahead of the tail, we don't have any characters
//     // if (_rx_buffer->head == _rx_buffer->tail) {
//     //   return -1;
//     // } else {
//     //   unsigned char c = _rx_buffer->buffer[_rx_buffer->tail];
//     //   _rx_buffer->tail = (unsigned int)(_rx_buffer->tail + 1) % SERIAL_BUFFER_SIZE;
//     //   return c;
//     // }
// 	return 0;
// }

// int SkynetClient::peek() {
//     if (_rx_buffer->head == _rx_buffer->tail) {
//       return -1;
//     } else {
//       return _rx_buffer->buffer[_rx_buffer->tail];
//     }
// }

void SkynetClient::flush() {
  while (client.available())
    client.read();
}

// the next function allows us to use the client returned by
// SkynetClient::available() as the condition in an if-statement.

SkynetClient::operator bool() {
  return true;
}

void SkynetClient::setMessageDelegate(MessageDelegate newMessageDelegate) {
	  messageDelegate = newMessageDelegate;
}

void SkynetClient::eeprom_write_bytes(char *buf, int address, int bufSize){
  for(int i = 0; i<bufSize; i++){
    EEPROM.write(address+i, buf[i]);
  }
}

void SkynetClient::eeprom_read_bytes(char *buf, int address, int bufSize){
  for(int i = 0; i<bufSize; i++){
    buf[i] = EEPROM.read(address+i);
  }
}

void SkynetClient::sendMessage(char *device, aJsonObject *object){
  aJsonObject *root=aJson.createObject();
  
  aJson.addStringToObject(root, DEVICES, device);
  aJson.addItemToObject(root, MESSAGE, object);

  DBGC(F("Sending: "));
  DBGCN(aJson.print(root));
  send(EMIT, aJson.print(root));
//  aJson.deleteItem(root);
}

void SkynetClient::sendMessage(char *device, char *object){
  aJsonObject *root=aJson.createObject();
  
  aJson.addStringToObject(root, DEVICES, device);
  aJson.addStringToObject(root, MESSAGE, object);

  DBGC(F("Sending: "));
  DBGCN(aJson.print(root));
  send(EMIT, aJson.print(root));
//  aJson.deleteItem(root);
}