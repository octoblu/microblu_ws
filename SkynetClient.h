#ifndef _SKYNETCLIENT_H
#define _SKYNETCLIENT_H

#include "Arduino.h"
#include "SPI.h"
#include "Client.h"
#include <EEPROM.h>
#include "utility/ringbuffer.h"
#include "utility/b64.h"
#include <JsonParser.h>

#define SKYNETCLIENT_DEBUG
#ifdef SKYNETCLIENT_DEBUG
	#ifdef PSTR
		#define DBGCSN( ... ) Serial.println( F(__VA_ARGS__) )
		#define DBGCS( ... ) Serial.print( F(__VA_ARGS__) )
		#define DBGCN( ... ) Serial.println( __VA_ARGS__ )
		#define DBGC( ... ) Serial.print( __VA_ARGS__ )
	#else
		#define DBGCSN( ... ) Serial.println( __VA_ARGS__ )
		#define DBGCS( ... ) Serial.print( __VA_ARGS__ )
		#define DBGCN( ... ) Serial.println( __VA_ARGS__ )
		#define DBGC( ... ) Serial.print( __VA_ARGS__ )
	#endif
#else
	#define DBGCN( ... )
	#define DBGC( ... )
	#define DBGCSN( ... )
	#define DBGCS( ... )
#endif

#define SID_MAXLEN 24
#define UUIDSIZE 37
#define TOKENSIZE 33
#define MAXACK 5

#define HEARTBEATTIMEOUT 60000
#define SOCKETTIMEOUT 30000UL

#define EEPROMBLOCKADDRESS 0
#define EEPROMBLOCK 'S'
#define TOKENADDRESS EEPROMBLOCKADDRESS+1
#define UUIDADDRESS TOKENADDRESS+TOKENSIZE

#define MAX_PARSE_OBJECTS 16 //16 needed for Ready from Skynet
#define MAX_FLASH_STRING 50 //for PROGMEM strings

// Length of static data buffers
#define SOCKET_RX_BUFFER_SIZE 250 //186 needed for biggest skynet message, READY
#define SKYNET_TX_BUFFER_SIZE 150 //~150 is needed for firmata's capability query on an uno
#define SKYNET_RX_BUFFER_SIZE 32

class SkynetClient : public Stream {
	public:
		SkynetClient(Client &_client);
		
		typedef void (*MessageDelegate)(const char *data);

		void setMessageDelegate(MessageDelegate messageDelegate);
		void sendMessage(const char* device, char const *object);
		void logMessage(char const *object);

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

		void printByByteF(PGM_P data);
		void printByByte(const char *data);
		void printByByte(const char *data, size_t size);
		void printToken(const char *js, jsmntok_t t);

		uint8_t waitSocketData();
		uint8_t readLine(char *buf, uint8_t max);

		void eeprom_write_bytes(int, char*, int);
		void eeprom_read_bytes(int, char*, int);

		void processSkynet(char *data, char *ack);
		void processIdentify(char *data, jsmntok_t *tok);
		void processReady(char *data, jsmntok_t *tok);
		void processNotReady(char *data, jsmntok_t *tok);
		void processMessage(char *data, jsmntok_t *tok);
		void processBind(char *data, jsmntok_t *tok, char *ack);
};

#endif // _SKYNETCLIENT_H