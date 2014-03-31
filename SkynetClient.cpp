
#include <Skynetclient.h>

#define TOKEN_STRING(js, t, s) \
	(strncmp(js+(t).start, s, (t).end - (t).start) == 0 \
	 && strlen(s) == (t).end - (t).start)
	
ringbuffer txbuf(SKYNET_TX_BUFFER_SIZE);
ringbuffer rxbuf(SKYNET_RX_BUFFER_SIZE);

SkynetClient::SkynetClient(Client &_client){
	this->client = &_client; 
}

int SkynetClient::connect(const char* host, uint16_t port) 
{
	thehost = host;
	status = 0;
	bind = 0;

	//connect tcp or fail
	if (!client->connect(host, port)) 
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

int SkynetClient::connect(IPAddress ip, uint16_t port) {
	return -1;
}

uint8_t SkynetClient::connected() {
  return bind;
}

void SkynetClient::stop() {
	status = 0;
	bind = 0;
	client->stop();
}

void SkynetClient::monitor()
{
	flushTX();

    if (client->available()) 
    {
		int size = readLineSocket();

		char ack = ' ';
		char *first  = strchr(databuffer, ':'); 
		char *second  = strchr(first+1, ':');
		char *third  = strchr(second+1, ':');

		//if first and second colon aren't next to eachother, grab the ack character
		if ((second - first) != 1)
		{
			ack = first[1];
			DBGC(F("ack: "));
			DBGCN(ack);
		}

		//where json parser should start
		char *dataptr = third+1;

		char socketType = databuffer[0];
		switch (socketType) {
	
			//disconnect
			case '0':
				DBGCN(F("Disconnect"));
				stop();
				break;
			
			//messages
			case '1':
				DBGCN(F("Socket Connect"));
				break;
				
			case '3':
				DBGCN(F("Data"));
				b64decodestore(dataptr, rxbuf);
				break;

			case '5':	
				DBGCN(F("Message"));
				processSkynet(dataptr, ack);
				break;
				
			//hearbeat
			case '2':
				DBGCN(F("Heartbeat"));
				client->print((char)0);
				client->print(F("2::"));
				client->print((char)255);
				break;

		    //huh?
			default:
				DBGC(F("Drop: "));
				DBGCN(socketType);
				break;
		}
	}
}

void SkynetClient::processSkynet(const char *data, const char ack)
{
	jsmn_parser p;
	jsmntok_t tok[MAX_PARSE_OBJECTS];

	jsmn_init(&p);

	int r = jsmn_parse(&p, data, tok, MAX_PARSE_OBJECTS);
	if (r != 0){
	    DBGCN(F("parse failed"));
		DBGCN(r);
		return;
	}

    if (TOKEN_STRING(data, tok[2], IDENTIFY )) 
    {
		DBGCN(F(IDENTIFY));
	    DBGC(F("Sending: "));

	    DBGC((char)0);
		client->print((char)0);

	    DBGC(EMIT);	
		client->print(EMIT);
	
		printByByte("{\"name\":\"identity\",\"args\":[{\"socketid\":\"");
		printToken(data, tok[7]);
		
		if( eeprom_read_byte( (uint8_t*)EEPROMBLOCKADDRESS) == EEPROMBLOCK )
		{
			eeprom_read_bytes(TOKENADDRESS, token, TOKENSIZE);
			token[TOKENSIZE-1]='\0'; //in case courrupted or not defined
	
			eeprom_read_bytes(UUIDADDRESS, uuid, UUIDSIZE);
			uuid[UUIDSIZE-1]='\0'; //in case courrupted or not defined

			printByByte("\", \"uuid\":\"");
			printByByte(uuid);

			printByByte("\", \"token\":\"");
			printByByte(token);
		}
		printByByte("\"}]}");
	  
		DBGCN((char)255);
		client->print((char)255);
    } 
    else if (TOKEN_STRING(data, tok[2], READY )) 
    {
		DBGCN(READY);
		status = 1;
		
        eeprom_read_bytes(TOKENADDRESS, token, TOKENSIZE);
        token[TOKENSIZE-1]='\0'; //in case courrupted or not defined
		
        //if token has been refreshed, save it
        if (!TOKEN_STRING(data, tok[15], token ))
        {
			DBGCN(F("token refresh"));
		  	strncpy(token, data + tok[15].start, tok[15].end - tok[15].start);

            DBGC(F("new: "));
          	DBGCN(token);
          
		  	eeprom_write_bytes(TOKENADDRESS, token, TOKENSIZE);

			//write block identifier, arduino should protect us from writing if it doesnt need it?
          	eeprom_write_byte((uint8_t *)EEPROMBLOCKADDRESS, (uint8_t)EEPROMBLOCK); 

        }else
        {
			DBGCN(F("no token refresh necessary"));
        }
		
		eeprom_read_bytes(UUIDADDRESS, uuid, UUIDSIZE);
        uuid[UUIDSIZE-1]='\0'; //in case courrupted or not defined

        //if uuid has been refreshed, save it
        if (!TOKEN_STRING(data, tok[13], uuid ))
        {
          	DBGCN(F("uuid refresh"));
			strncpy(uuid, data + tok[13].start, tok[13].end - tok[13].start);
			
          	DBGC(F("new: "));
          	DBGCN(uuid);
			
          	eeprom_write_bytes(UUIDADDRESS, uuid, UUIDSIZE);
			
			//write block identifier, arduino should protect us from writing if it doesnt need it?
          	eeprom_write_byte((uint8_t *)EEPROMBLOCKADDRESS, (uint8_t)EEPROMBLOCK); 

         }else
         {
           	DBGCN(F("no uuid refresh necessary"));
         }
    }
    else if (TOKEN_STRING(data, tok[2], NOTREADY )) 
    {
		//send blank identify
		DBGCN(NOTREADY);

	    DBGC(F("Sending: "));

	    DBGC((char)0);
		client->print((char)0);

	    DBGC(EMIT);	
		client->print(EMIT);

		printByByte("{\"name\":\"identity\",\"args\":[{\"socketid\":\"");
		printToken(data, tok[11]);
		printByByte("\"}]}");
	  
		DBGCN((char)255);
		client->print((char)255);
    }
    else if (TOKEN_STRING(data, tok[2], BIND )) 
    {
    	bind = 1;

    	DBGCN(BIND);

	    DBGC(F("Sending: "));

	    DBGC((char)0);
		client->print((char)0);

	    DBGC("6:::");
		client->print("6:::");

		DBGC(ack);
		client->print(ack);

		printByByte("+[{\"result\":\"ok\"}]");
	  
		DBGCN((char)255);
		client->print((char)255);
    }
    else if (TOKEN_STRING(data, tok[2], MESSAGE )) 
    {
		DBGCN(MESSAGE);

		if (messageDelegate != NULL) {
			messageDelegate(data);
		}
    }
    else
    {
		DBGC(F("Unknown:"));
    }
}

void SkynetClient::sendHandshake() {
	client->println(F("GET /socket.io/1/ HTTP/1.1"));
	client->print(F("Host: "));
	client->println(thehost);
	client->println(F("Origin: Arduino\r\n"));
}

bool SkynetClient::waitForInput(void) {
	unsigned long now = millis();
	while (!client->available() && ((millis() - now) < 30000UL)) {;}
	return client->available();
}

void SkynetClient::eatHeader(void) {
	while (client->available()) {	// consume the header
		readLineHTTP();
		if (strlen(databuffer) == 0) break;
	}
}

int SkynetClient::readHandshake() {

	if (!waitForInput()) return false;

	// check for happy "HTTP/1.1 200" response
	readLineHTTP();
	if (atoi(&databuffer[8]) != 200) {
		while (client->available()) readLineHTTP();
		client->stop();
		return 0;
	}
	eatHeader();
	readLineHTTP();	// read first line of response
	readLineHTTP();	// read sid : transport : timeout
		
	char sid[SID_LEN];
	char *iptr = databuffer;
	char *optr = sid;
	while (*iptr && (*iptr != ':') && (optr < &sid[SID_LEN-2])) *optr++ = *iptr++;
	*optr = 0;

	DBGC(F("Connected. SID="));
	DBGCN(sid);	// sid:transport:timeout 

	while (client->available()) readLineHTTP();

	client->print(F("GET /socket.io/1/websocket/"));
	client->print(sid);
	client->println(F(" HTTP/1.1"));
	client->print(F("Host: "));
	client->println(thehost);
	client->println(F("Origin: ArduinoSkynetClient"));
	client->println(F("Upgrade: WebSocket"));	// must be camelcase ?!
	client->println(F("Connection: Upgrade\r\n"));

	if (!waitForInput()) return 0;

	readLineHTTP();
	if (atoi(&databuffer[8]) != 101) {
		while (client->available()) readLineHTTP();
		client->stop();
		return false;
	}
	eatHeader();
	monitor();		// treat the response as input
	return 1;
}

int SkynetClient::readLineHTTP() {
	int numBytes = 0;
	char *dataptr = databuffer;
	DBGC(F("ReadlineHTTP: "));
	while (client->available() && (dataptr < &databuffer[SOCKET_RX_BUFFER_SIZE-3])) {
		char c = client->read();
		if (c == 0){
			;
		}else if (c == -1){
			;
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

int SkynetClient::readLineSocket() {
	int numBytes = 0;
	char *dataptr = databuffer;
	DBGC(F("ReadlineSocket: "));
	//-1 for 0 index
	//-1 for space to add a char
	//-1 to fit a null char so
	while (client->available() && (dataptr < &databuffer[SOCKET_RX_BUFFER_SIZE-3])) {
		char c = client->read();
		if (c == 0){
			;
		}
		else if (c == -1){
			break;
		}
		else {
			DBGC(c);
			*dataptr++ = c;
			numBytes++;
		}
	}
	*dataptr = 0;
	DBGCN();
	return numBytes;
}

//wifi client->print has a buffer that so far we've been unable to locate
//under 154 (our identify size) for sure.. so sending char by char for now
void SkynetClient::printByByte(const char *data, size_t size) {
	if(data != NULL && data[0] != '\0')
	{
		int i = 0;
		while ( i < size)
		{
		    DBGC(data[i]);
			client->print(data[i++]);
		}
	}
}

//wifi client->print has a buffer that so far we've been unable to locate
//under 154 (our identify size) for sure.. so sending char by char for now
void SkynetClient::printByByte(const char *data) {
	if(data != NULL)
	{
		int i = 0;
		while ( data[i] != '\0' )
		{
		    DBGC(data[i]);
			client->print(data[i++]);
		}
	}
}

void SkynetClient::printToken(const char *js, jsmntok_t t) 
{
	int i = 0;
	for(i = t.start; i < t.end; i++) {
	    DBGC(js[i]);
		client->print(js[i]);
	 }
}

size_t SkynetClient::write(const uint8_t *buf, size_t size) {
    DBGC(F("Sending2: "));

    DBGC((char)0);
	client->print((char)0);

    DBGC(MSG);	
	client->print(MSG);
	
	//b64send(buf, size, client);

    DBGCN((char)255);
	client->print((char)255);

	return size;
}

//place write data into a buffer to be sent on next flush or monitor
size_t SkynetClient::write(uint8_t c)
{
	if(bind){
		DBGC(F("Storing: "));

	    DBGCN((char)c);

	    txbuf.push(c);

		return 1;
	}
	else{
		DBGC(F("Not bound, NOT Storing: "));
	    DBGCN((char)c);

		return 0;
	}
}

void SkynetClient::flushTX()
{
	if(txbuf.available()){
		DBGC(F("Sending: "));
	
	    DBGC((char)0);
		client->print((char)0);
	
	    DBGC(MSG);	
		client->print(MSG);

		b64send(txbuf, *client);
		
		DBGCN((char)255);
		client->print((char)255);
	}
}

 char b64[ 64 ] PROGMEM  = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
    'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};
  

void SkynetClient::b64send(ringbuffer &buffer, Client &out)
{
	int i = 0;
	while(buffer.available())
	{
		//default to 
		char c[] = {0, 0, 0};
		char d0, d1, d2, d3;

		//grab 3 chars if available
		do
		{	
			// DBGC("c");
			// DBGC((int)i%3);
			// DBGC(": ");
			// DBGCN((int)(buffer->buffer[buffer->tail]));
			c[i%3]=buffer.pop();
      		i++;
		}while(buffer.available() && (i%3!=0));

		if (i%3==1)
		{
			d0 = (c[0] >> 2) & 63;
			d1 = ((c[0] << 4) & 48) | ((c[1] >> 4) & 15);

			// DBGC("d0: ");
			// DBGC((int)d0);
			// DBGC(" : ");
			DBGC(b64lookup(d0));
			out.print( b64lookup(d0) );
			// DBGC("d1: ");
			// DBGC((int)d1);
			// DBGC(" : ");
			DBGC(b64lookup(d1));
			out.print( b64lookup(d1) );
			DBGC('=');
		    out.print( '=' );
			DBGC('=');
		    out.print( '=' );
		}
		else if (i%3==2)
		{
			d0 = (c[0] >> 2) & 63;
			d1 = ((c[0] << 4) & 48) | ((c[1] >> 4) & 15);
			d2 = ((c[1] << 2) & 60) | ((c[2] >> 6) & 3);

			// DBGC("d0: ");
			// DBGC((int)d0);
			// DBGC(" : ");
			DBGC(b64lookup(d0));
			out.print( b64lookup(d0) );

			// DBGC("d1: ");
			// DBGC((int)d1);
			// DBGC(" : ");
			DBGC(b64lookup(d1));
			out.print( b64lookup(d1) );

			// DBGC("d2: ");
			// DBGC((int)d2);
			// DBGC(" : ");
			DBGC(b64lookup(d2));
			out.print( b64lookup(d2) );
			DBGC('=');
		    out.print( '=' );
		}
		else
		{
			d0 = (c[0] >> 2) & 63;
			d1 = ((c[0] << 4) & 48) | ((c[1] >> 4) & 15);
			d2 = ((c[1] << 2) & 60) | ((c[2] >> 6) & 3);
			d3 = c[2] & 63;

			// DBGC("d0: ");
			// DBGC((int)d0);
			// DBGC(" : ");
			DBGC(b64lookup(d0));
			out.print( b64lookup(d0) );

			// DBGC("d1: ");
			// DBGC((int)d1);
			// DBGC(" : ");
			DBGC(b64lookup(d1));
			out.print( b64lookup(d1) );

			// DBGC("d2: ");
			// DBGC((int)d2);
			// DBGC(" : ");
			DBGC(b64lookup(d2));
			out.print( b64lookup(d2) );

			// DBGC("d3: ");
			// DBGC((int)d3);
			// DBGC(" : ");
			DBGC(b64lookup(d3));
			out.print( b64lookup(d3) );
		}
	}
}

void SkynetClient::b64send(const uint8_t *buf, size_t size, Client &out)
{
	int i = 0;
	while(i<size)
	{
	
	}
	DBGCN();
}

char SkynetClient::b64lookup(const char c)
{
	return pgm_read_byte( &b64[ c ]);
}

void SkynetClient::b64decodestore(char *src, ringbuffer &buffer)
{
	int i = 0;
	while(src[i]!='\0')
	{
		//default to 
		char c[] = {-1, -1, -1, -1};
		char d0, d1, d2;

		//grab 4 chars if available
		do
		{
			// DBGC("c");
			// DBGC(i%4);
			// DBGC(": ");
			// DBGC(src[i]);
			// DBGC(": ");
			// DBGCN((int)b64reverselookup(src[i]));
			c[i%4]=b64reverselookup(src[i++]);
		}while( src[i] != '\0' && i%4 != 0 );

		d0 = ((c[0] << 2) & 252) | (c[1] >>4 & 3);
		// DBGC("d0: ");
		DBGCN(d0, HEX);
    	buffer.push(d0);
    	
    	//if c3 is equal sign (negative one in our lookup), we ignore the second to last character
		if(c[3] != -1)
		{
			d1 = ((c[1] << 4) & 240) | (c[2] >>2 & 15);
			// DBGC("d1: ");
			DBGCN(d1, HEX);
    		buffer.push(d1);
		}

		//if c2 is equal sign (negative one in our lookup), we ignore the second to last character
		if(c[2] != -1)
		{
			d2 = ((c[2] << 6) & 192) | (c[3] & 63);
			// DBGC("d2: ");
			DBGCN(d2, HEX);
	    	buffer.push(d2);
	    }
	}
	DBGCN();
}

char SkynetClient::b64reverselookup(const char c)
{
	if (c >= 'A' && c <= 'Z')
	{
		return c-65;
	}
	else if (c >= 'a' && c <= 'z')
	{
		return c-71;
	}
	else if (c >= '0' && c <= '9')
	{
		return c+4;
	}
	else if (c == 43)
	{
		return 62;
	}
	else if (c == 47)
	{
		return 63;
	}
	else
	{
		return -1;
	}
}

int SkynetClient::available() {
  return rxbuf.available();
}

int SkynetClient::read() {
    // if the head isn't ahead of the tail, we don't have any characters
    if (rxbuf.available()) 
    {
    	return rxbuf.pop();
    } else {
    	return -1;
    }
}

// //TODO	
// int SkynetClient::read(uint8_t *buf, size_t size) {
// }

int SkynetClient::peek() 
{
    if (rxbuf.available()) 
    {
    	return rxbuf.peek();
    } else {
    	return -1;
    }
}

void SkynetClient::flush() 
{
	client->flush();
}

// the next function allows us to use the client returned by
// SkynetClient::available() as the condition in an if-statement.

SkynetClient::operator bool() {
  return true;
}

void SkynetClient::setMessageDelegate(MessageDelegate newMessageDelegate) {
	  messageDelegate = newMessageDelegate;
}

void SkynetClient::eeprom_write_bytes(int address, char *buf, int bufSize){
  for(int i = 0; i<bufSize; i++){
    EEPROM.write(address+i, buf[i]);
  }
}

void SkynetClient::eeprom_read_bytes(int address, char *buf, int bufSize){
  for(int i = 0; i<bufSize; i++){
    buf[i] = EEPROM.read(address+i);
  }
}

void SkynetClient::sendMessage(const char *device, char const *object)
{
	DBGC(F("Sending: "));

    DBGC((char)0);
	client->print((char)0);

    DBGC(EMIT);	
	client->print(EMIT);

	printByByte("{\"name\":\"message\",\"args\":[{\"devices\":\"");
	printByByte(device);
	printByByte("\",\"payload\":\"");
	printByByte(object);
	printByByte("\"}]}");

	DBGCN((char)255);
	client->print((char)255);
}