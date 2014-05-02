
#include <Skynetclient.h>

#define TOKEN_STRING(js, t, s) \
	(strncmp(js+(t).start, s, (t).end - (t).start) == 0 \
	 && strlen(s) == (t).end - (t).start)
	
ringbuffer txbuf(SKYNET_TX_BUFFER_SIZE);
ringbuffer rxbuf(SKYNET_RX_BUFFER_SIZE);

SkynetClient::SkynetClient(Client &_client){
	this->client = &_client; 
}

int SkynetClient::connect(IPAddress ip, uint16_t port){
	status = 0;
	bind = 0;

	DBGCSN("Connecting TCP");

	//connect tcp or fail
	if (!client->connect(ip, port))
	{
		client->stop();
		DBGCSN("TCP Failed");
		return false;
	}

	xmit(FPOST1);
	xmit(ip);
	xmit(FPOST2);

	//receive data or return
	if(!waitSocketData())
	{
		client->stop();
		DBGCSN("Post Failed");
		return false;
	}

	//check for OK or return
	if(readLine(databuffer, SOCKET_RX_BUFFER_SIZE) == 0 || strstr (databuffer,"200") == NULL){
		client->stop();
		DBGCSN("No Initial OK response");
		return false;
	}

	//dump the response until the socketid line
	for(int i = 0; i<7; i++){
		if(readLine(databuffer, SOCKET_RX_BUFFER_SIZE)==0)
		{
			client->stop();
			DBGCSN("Malformed POST response");
			return false;
		}
	}
		
	//get the sid out of buffer
	char sid[SID_MAXLEN+1];

	int i = 0;

	while((char)databuffer[i] != ':'){
	   sid[i]=databuffer[i];
	   i++;
	}
 
	sid[i++]=0;

	DBGCS("SID: ");
	DBGCN(sid);

	//dump the remaining response
	while(client->available())
		client->read();

	xmit(FGET1);
	xmit(sid);
	xmit(FGET2);
	xmit(ip);
	xmit(FGET3);

	//receive data or return
	if(!waitSocketData())
	{
		client->stop();
		DBGCSN("GET Failed");
		return false;
	}
	
	//check for OK or return
	if(readLine(databuffer, SOCKET_RX_BUFFER_SIZE) == 0 || strstr (databuffer,"101") == NULL)
	{
		DBGCSN("No Final OK response");
		client->stop();
		return false;
	}

	DBGCSN("Websocket Connected");

	//dump the rest of the response
	for(int i = 0; i<5; i++){
		readLine(databuffer, SOCKET_RX_BUFFER_SIZE);
	}

	//havent gotten a heartbeat yet so lets set current time
	lastBeat = millis();

	//monitor to initiate communications with skynet
	while(!monitor() && (unsigned long)(millis() - lastBeat) <= SOCKETTIMEOUT);

	if(!status){
		DBGCSN("Skynet handshake timeout");
		client->stop();
	}

	return status;
}

int SkynetClient::connect(const char* host, uint16_t port) 
{
	status = 0;
	bind = 0;

	DBGCSN("Connecting TCP");

	//connect tcp or fail
	if (!client->connect(host, port))
	{
		client->stop();
		DBGCSN("TCP Failed");
		return false;
	}

	xmit(FPOST1);
	xmit(host);
	xmit(FPOST2);

	//receive data or return
	if(!waitSocketData())
	{
		client->stop();
		DBGCSN("Post Failed");
		return false;
	}

	//check for OK or return
	if(readLine(databuffer, SOCKET_RX_BUFFER_SIZE) == 0 || strstr (databuffer,"200") == NULL){
		client->stop();
		DBGCSN("No Initial OK response");
		return false;
	}

	//dump the response until the socketid line
	for(int i = 0; i<7; i++){
		if(readLine(databuffer, SOCKET_RX_BUFFER_SIZE)==0)
		{
			client->stop();
			DBGCSN("Malformed POST response");
			return false;
		}
	}
		
	//get the sid out of buffer
	char sid[SID_MAXLEN+1];

	int i = 0;

	while((char)databuffer[i] != ':'){
	   sid[i]=databuffer[i];
	   i++;
	}
 
	sid[i++]=0;

	DBGCS("SID: ");
	DBGCN(sid);

	//dump the remaining response
	while(client->available())
		client->read();

	xmit(FGET1);
	xmit(sid);
	xmit(FGET2);
	xmit(host);
	xmit(FGET3);

	//receive data or return
	if(!waitSocketData())
	{
		client->stop();
		DBGCSN("GET Failed");
		return false;
	}
	
	//check for OK or return
	if(readLine(databuffer, SOCKET_RX_BUFFER_SIZE) == 0 || strstr (databuffer,"101") == NULL)
	{
		DBGCSN("No Final OK response");
		client->stop();
		return false;
	}

	DBGCSN("Websocket Connected");

	//dump the rest of the response
	for(int i = 0; i<5; i++){
		readLine(databuffer, SOCKET_RX_BUFFER_SIZE);
	}

	//havent gotten a heartbeat yet so lets set current time
	lastBeat = millis();

	//monitor to initiate communications with skynet
	while(!monitor() && (unsigned long)(millis() - lastBeat) <= SOCKETTIMEOUT);

	if(!status){
		DBGCSN("Skynet handshake timeout");
		client->stop();
	}

	return status;
}

uint8_t SkynetClient::waitSocketData()
{
	lastBeat = millis();
	while (!client->available() && ((unsigned long)(millis() - lastBeat) <= SOCKETTIMEOUT))
	{
		;
	}
	return client->available();
}

uint8_t SkynetClient::readLine(char *buf, uint8_t max)
{
	int count = 0;

	//end on newline, -1 from client, or -1 from client not available
	char c = client->read();
	while(c!=-1 && c!=10 && c!=255)
	{
		switch (c)
		{
			//dont store but get more chars
			case 0:
			case 13:
				break;
			//if it fits, store it	
			default:
				if(count < max-1)
				{
					buf[count++]=c;
				}else
				{
					c = toupper(c);
				}
				DBGC(c);
		}
		c = client->read();	
	}
	
	buf[count++]=0;
	DBGCN();
	return count;
}

uint8_t SkynetClient::connected() {
  return bind;
}

void SkynetClient::stop() {
	status = 0;
	bind = 0;
	client->stop();
}

int SkynetClient::monitor()
{
	//if we've expired, reconnect to skynet at least
	if(status == 1 && (unsigned long)(millis() - lastBeat) >= HEARTBEATTIMEOUT){
		DBGCS("Timeout: ");
		DBGCN(millis());

		DBGCS("lastbeat: ");
		DBGCN(lastBeat);

		stop();
		return status;
	}

	flush();

    if (client->available()) 
    {
		int size = readLine(databuffer, SOCKET_RX_BUFFER_SIZE);

		char *first  = strchr(databuffer, ':'); 
		char *second  = strchr(first+1, ':');
		char *third  = strchr(second+1, ':');

		//-2 for the colons
		int ackSize = second - first - 2;
		char ack[MAXACK+1];

		//if first and second colon aren't next to eachother, and acksize is sane, grab the ack character(s)
		if (ackSize>0 && ackSize<MAXACK)
		{
			DBGCN(ackSize);
			DBGCN(first+1);

			strncpy(ack, first+1, ackSize);
			ack[ackSize] = '\0';
			DBGCS("ack: ");
			DBGCN(ack);
		}

		//where json parser should start
		char *dataptr = third+1;

		char socketType = databuffer[0];
		switch (socketType) {
	
			//disconnect
			case '0':
				DBGCSN("Disconnect");
				stop();
				break;
			
			//messages
			case '1':
				DBGCSN("Socket Connect");
				break;
				
			case '3':
				DBGCSN("Data");
				b64::decodestore(dataptr, rxbuf);
				break;

			case '5':	
				DBGCSN("Message");
				processSkynet(dataptr, ack);
				break;
				
			//hearbeat
			case '2':
				DBGCS("Heartbeat at: ");
				lastBeat = millis();
				DBGCN(lastBeat);
				xmit((char)0);
				xmit(HEARTBEAT);
				xmit((char)255);
				break;

		    //huh?
			default:
				DBGCS("Drop: ");
				DBGCN(socketType);
				break;
		}
	}
	return status;
}

//lookup uuid and token if we have them and send in for validation
void SkynetClient::processIdentify(char *data, jsmntok_t *tok)
{
	char temp[UUIDSIZE];

    DBGCS("Sending: ");

	xmit((char)0);
	xmit(EMIT);
	xmit(FIDENTIFY1);
	xmitToken(data, tok[7]);
	
	if( EEPROM.read( (uint8_t)EEPROMBLOCKADDRESS) == EEPROMBLOCK )
	{
		getUuid(temp);

		xmit(FIDENTIFY2);
		xmit(temp);

		getToken(temp);

		xmit(FIDENTIFY3);
		xmit(temp);
	}

	xmit(FCLOSE);
	xmit((char)255);
}

//Got credentials back, store if necessary
void SkynetClient::processReady(char *data, jsmntok_t *tok)
{
	DBGCSN("Skynet Connect");

	char temp[UUIDSIZE];

	status = 1;
	
	getUuid(temp);

    //if uuid has been refreshed, save it
    if (!TOKEN_STRING(data, tok[13], temp ))
    {
      	DBGCSN("uuid refresh");
		strncpy(temp, data + tok[13].start, tok[13].end - tok[13].start);
		
      	DBGCS("new: ");
      	DBGCN(temp);
		
      	setUuid(temp);
    }else
    {
    	DBGCSN("no uuid refresh necessary");
    }

    getToken(temp);
	
    //if token has been refreshed, save it
    if (!TOKEN_STRING(data, tok[15], temp ))
    {
		DBGCSN("token refresh");
	  	strncpy(temp, data + tok[15].start, tok[15].end - tok[15].start);

        DBGCS("new: ");
      	DBGCN(temp);
      
		setToken(temp);
    }else
    {
		DBGCSN("no token refresh necessary");
    }

}

//Credentials have been invalidted, send blank identify for new ones
void SkynetClient::processNotReady(char *data, jsmntok_t *tok)
{
    DBGCS("Sending: ");

	xmit((char)0);
	xmit(EMIT);
	xmit(FIDENTIFY1);
	xmitToken(data, tok[11]);
	xmit(FCLOSE);
	xmit((char)255);
}

void SkynetClient::processBind(char *data, jsmntok_t *tok, char *ack)
{
	bind = 1;

    DBGCS("Sending Bind: ");

	xmit((char)0);
	xmit(BND);
	xmit(ack);
	xmit(FBIND1);
	xmit((char)255);
}

void SkynetClient::processMessage(char *data, jsmntok_t *tok)
{
	//just give them the args
	int index = tok[5].end;
	data[index]=0;

	DBGCN(data + tok[5].start);

	if (messageDelegate != NULL) {
		messageDelegate(data + tok[5].start);
	}
}

void SkynetClient::processSkynet(char *data, char *ack)
{
	jsmn_parser p;
	jsmntok_t tok[MAX_PARSE_OBJECTS];

	jsmn_init(&p);

	int r = jsmn_parse(&p, data, tok, MAX_PARSE_OBJECTS);
	if (r != 0){
	    DBGCS("parse failed: ");
		DBGCN(r);
		return;
	}

    if (TOKEN_STRING(data, tok[2], IDENTIFY )) 
    {
		DBGCSN(IDENTIFY);
		processIdentify(data, tok);
    } 
    else if (TOKEN_STRING(data, tok[2], READY )) 
    {
		DBGCSN(READY);
		processReady(data, tok);
    }
    else if (TOKEN_STRING(data, tok[2], NOTREADY )) 
    {
		DBGCSN(NOTREADY);
		processNotReady(data, tok);
    }
    else if (TOKEN_STRING(data, tok[2], BIND )) 
    {
		DBGCSN(BIND);
		processBind(data, tok, ack);
    }
    else if (TOKEN_STRING(data, tok[2], MESSAGE )) 
    {
		DBGCSN(MESSAGE);
		processMessage(data, tok);
    }
    else
    {
		DBGCS("Unknown:");
    }
}

void SkynetClient::xmit(const __FlashStringHelper* data) 
{
	PGM_P p = reinterpret_cast<PGM_P>(data);

	char buffer[MAX_FLASH_STRING];
	strcpy_P(buffer, p);

	DBGC(buffer);
	client->print(buffer);
}

void SkynetClient::xmit(IPAddress data) 
{
	DBGC(data);
	client->print(data);
}

void SkynetClient::xmit(const char *data) 
{
	DBGC(data);
	client->print(data);
}

void SkynetClient::xmit(char data)
{
	DBGC(data);
	client->print(data);
}

void SkynetClient::xmitToken(const char *js, jsmntok_t t) 
{
	int i = 0;
	for(i = t.start; i < t.end; i++) {
	    DBGC(js[i]);
		client->print(js[i]);
	 }
}

size_t SkynetClient::write(const uint8_t *buf, size_t size) {
    DBGCS("Sending2: ");

	xmit((char)0);

	xmit(MSG);
	
	//b64::send(buf, size, client);

	xmit((char)255);

	return size;
}

//place write data into a buffer to be sent on next flush or monitor
size_t SkynetClient::write(uint8_t c)
{
	if(bind){
		DBGCS("Storing: ");

	    DBGCN((char)c);

	    txbuf.push(c);

		return 1;
	}
	else{
		DBGCS("Not bound, NOT Storing: ");
	    DBGCN((char)c);

		return 0;
	}
}

void SkynetClient::flush()
{
	if(txbuf.available()){
		DBGCS("Sending: ");
	
		xmit((char)0);
	
		xmit(MSG);

		DBGCS("--BUFFER--");
		b64::send(txbuf, *client);
		
		xmit((char)255);
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

void SkynetClient::getToken(char *token)
{
	eeprom_read_bytes(TOKENADDRESS, token, TOKENSIZE);
	token[TOKENSIZE-1]='\0'; //in case courrupted or not defined
}

void SkynetClient::setToken(char *token)
{
	eeprom_write_bytes(TOKENADDRESS, token, TOKENSIZE);
	
	//write block identifier, arduino should protect us from writing if it doesnt need it?
  	EEPROM.write((uint8_t )EEPROMBLOCKADDRESS, (uint8_t)EEPROMBLOCK); 
}

void SkynetClient::getUuid(char *uuid)
{
	eeprom_read_bytes(UUIDADDRESS, uuid, UUIDSIZE);
	uuid[UUIDSIZE-1]='\0'; //in case courrupted or not defined
}

void SkynetClient::setUuid(char *uuid)
{
	eeprom_write_bytes(UUIDADDRESS, uuid, UUIDSIZE);

	//write block identifier, arduino should protect us from writing if it doesnt need it?
	EEPROM.write((uint8_t)EEPROMBLOCKADDRESS, (uint8_t)EEPROMBLOCK); 
}

void SkynetClient::sendMessage(const char *device, char const *object)
{
	DBGCS("Sending: ");

	xmit((char)0);
	xmit(EMIT);
	xmit(FMESSAGE1);
	xmit(device);
	xmit(FMESSAGE2);
	xmit(object);
	xmit(FCLOSE);
	xmit((char)255);
}

void SkynetClient::logMessage(char const *object)
{
	char temp[UUIDSIZE];

	DBGCS("Logging: ");

	xmit((char)0);
	xmit(EMIT);
	xmit(FLOG1);
	xmit(object);
	xmit(FLOG2);

	getUuid(temp);

	xmit(temp);
	xmit(FIDENTIFY3);
	
	getToken(temp);
	
	xmit(temp);
	xmit(FCLOSE);
	xmit((char)255);
}