#ifndef _SKYNETCLIENT_H
#define _SKYNETCLIENT_H

#ifdef SPARK
#include "application.h"
#else
#include "Arduino.h"
#include "Client.h"
#include <EEPROM.h>
#endif

#include "jsmn.h"
#include "ringbuffer.h"
#include "b64.h"

#define SKYNETCLIENT_DEBUG
#ifdef SKYNETCLIENT_DEBUG
	#ifdef PSTR
		#define DBGCSN( ... ) Serial.println( F(__VA_ARGS__) )
		#define DBGCS( ... ) Serial.print( F(__VA_ARGS__) )
	#else
		#define DBGCSN( ... ) Serial.println( __VA_ARGS__ )
		#define DBGCS( ... ) Serial.print( __VA_ARGS__ )
	#endif
	#define DBGCN( ... ) Serial.println( __VA_ARGS__ )
	#define DBGC( ... ) Serial.print( __VA_ARGS__ )
#else
	#define DBGCN( ... )
	#define DBGC( ... )
	#define DBGCSN( ... )
	#define DBGCS( ... )
#endif

const uint8_t SID_MAXLEN = 24;
const uint8_t UUIDSIZE = 37;
const uint8_t TOKENSIZE = 33;
const uint8_t MAXACK = 5;

const uint16_t HEARTBEATTIMEOUT = 60000;
const uint16_t SOCKETTIMEOUT = 10000;

const uint8_t EEPROMBLOCK = 'S';
const uint16_t EEPROMBLOCKADDRESS = 0;
const uint16_t TOKENADDRESS = EEPROMBLOCKADDRESS+1;
const uint16_t UUIDADDRESS = TOKENADDRESS+TOKENSIZE;

const uint8_t MAX_PARSE_OBJECTS = 16; //16 needed for Ready from Skynet
const uint8_t MAX_FLASH_STRING = 50; //for longst PROGMEM string, FGET3

// Length of static data buffers
const uint8_t SOCKET_RX_BUFFER_SIZE = 186; //186 needed for biggest skynet message, READY
const uint8_t SKYNET_TX_BUFFER_SIZE = 150; //~150 is needed for firmata's capability query on an uno
const uint8_t SKYNET_RX_BUFFER_SIZE = 64; //~? needed for firmata to fill with response

const prog_uchar FLOG1[] PROGMEM = {"{\"name\":\"data\",\"args\":[{"};
const prog_uchar FLOG2[] PROGMEM = {", \"uuid\":\""};

const prog_uchar FUUID[] PROGMEM = {"\"uuid\":\""};


const prog_uchar FIDENTIFY1[] PROGMEM = {"{\"name\":\"identity\",\"args\":[{"};
const prog_uchar FIDENTIFY2[] PROGMEM = {"\", \"uuid\":\""};
const prog_uchar FIDENTIFY3[] PROGMEM = {"\", \"token\":\""};

const prog_uchar FCLOSE1[] PROGMEM = {"\"}]}"};
const prog_uchar FCLOSE2[] PROGMEM = {"}]}"};

const prog_uchar FBIND1[] PROGMEM = {"+[{\"result\":\"ok\"}]"};

const prog_uchar FMESSAGE1[] PROGMEM = {"{\"name\":\"message\",\"args\":[{\"devices\":\""};
const prog_uchar FMESSAGE2[] PROGMEM = {"\",\"payload\":\""};

const prog_uchar FGET1[] PROGMEM = {"GET /socket.io/1/websocket/"};
const prog_uchar FGET2[] PROGMEM = {" HTTP/1.1\r\nHost: "};
const prog_uchar FGET3[] PROGMEM = {"\r\nUpgrade: WebSocket\r\nConnection: Upgrade\r\n\r\n"};

const prog_uchar FPOST1[] PROGMEM = {"POST /socket.io/1/ HTTP/1.1\r\nHost: "};
const prog_uchar FPOST2[] PROGMEM = {"\r\n\r\n"};

const char IDENTIFY[] = "identify";
const char READY[] = "ready";
const char NOTREADY[] = "notReady";
const char BIND[] = "bindSocket";
const char MESSAGE[] = "message";

const char EMIT[] = "5:::";
const char MSG[] = "3:::";
const char HEARTBEAT[] = "2::";
const char BND[] = "6:::";

class SkynetClient : public Stream {
	public:
		SkynetClient(Client &_client);
		
		typedef void (*MessageDelegate)(const char *data);

		void setMessageDelegate(MessageDelegate messageDelegate);
		void sendMessage(const char* device, char const *object);
		void logMessage(char const *object);

		int connect(IPAddress ip, uint16_t port);
	    int connect(const char *host, uint16_t port);
	    size_t write(uint8_t c);
	    size_t write(const uint8_t *buf, size_t size);
	    size_t writeRaw(const uint8_t *buf, size_t size);

	    int available();
	    int read();
	    // int read(uint8_t *buf, size_t size);
	    int peek();
	    void flush();
	    void stop();
	    uint8_t connected();
	    operator bool();
		int monitor();
		void getUuid(char *uuid);
		void getToken(char *token);
		void setUuid(char *uuid);
		void setToken(char *token);
		
	private:
		Client* client;
		char databuffer[SOCKET_RX_BUFFER_SIZE];
		uint8_t status;
		uint8_t bind;
		unsigned long lastBeat;
        MessageDelegate messageDelegate;

		void xmit(const __FlashStringHelper* data);
		void xmit(const char *data);
		void xmit(char data);
		void xmit(const char *js, jsmntok_t t);
		void xmit(IPAddress data);
		void xmit(const prog_uchar *data);

		uint8_t waitSocketData();
		uint8_t readLine(char *buf, uint8_t max);

		void eeprom_write_bytes(int, char*, int);
		void eeprom_write_byte(int, const char);
		void eeprom_read_bytes(int, char*, int);

		void processSkynet(char *data, char *ack);
		void processIdentify(char *data, jsmntok_t *tok);
		void sendIdentify(bool credentials);
		void processReady(char *data, jsmntok_t *tok);
		void processNotReady(char *data, jsmntok_t *tok);
		void processMessage(char *data, jsmntok_t *tok);
		void processBind(char *data, jsmntok_t *tok, char *ack);
};

#endif // _SKYNETCLIENT_H