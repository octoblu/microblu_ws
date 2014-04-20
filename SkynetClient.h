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
#define DBGCN( ... ) Serial.println( __VA_ARGS__ )
#define DBGC( ... ) Serial.print( __VA_ARGS__ )
#else
#define DBGCN( ... )
#define DBGC( ... )
#endif

#define NAME "name"
#define IDENTITY "identity"
#define IDENTIFY "identify"
#define ARGS "args"
#define SOCKETID "socketid"
#define READY "ready"
#define NOTREADY "notReady"
#define API "api"
#define CONNECT "connect"
#define STATUS "status"
#define UUID "uuid"
#define TOKEN "token"
#define MESSAGE "message"
#define BIND "bindSocket"
#define PAYLOAD "payload"
#define DEVICES "devices"
#define EMIT "5:::"
#define MSG "3:::"
#define HEARTBEAT "2::"
#define FROMUUID "fromUuid"
#define DATA "data"

#define SID_LEN 24
#define UUIDSIZE 37
#define TOKENSIZE 33
#define MAXACK 5

#define HEARTBEATTIMEOUT 60000

#define EEPROMBLOCKADDRESS 0
#define EEPROMBLOCK 'S'
#define TOKENADDRESS EEPROMBLOCKADDRESS+1
#define UUIDADDRESS TOKENADDRESS+TOKENSIZE

#define MAX_PARSE_OBJECTS 16 //16 needed for Ready from Skynet

// Length of static data buffers
#define SOCKET_RX_BUFFER_SIZE 186 //186 needed for biggest skynet message, READY
#define SKYNET_TX_BUFFER_SIZE 150 //~150 is needed for firmata's capability query on an uno
#define SKYNET_RX_BUFFER_SIZE 32

class SkynetClient : public Stream {
	public:
		SkynetClient(Client &_client);
		
		typedef void (*MessageDelegate)(const char *data);

		void setMessageDelegate(MessageDelegate messageDelegate);
		void sendMessage(const char* device, char const *object);

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
		
	private:
		Client* client;
		char databuffer[SOCKET_RX_BUFFER_SIZE];
		uint8_t status;
		uint8_t bind;
		unsigned long lastBeat;
		const char *thehost;
        MessageDelegate messageDelegate;

		void printByByte(const char *data);
		void printByByte(const char *data, size_t size);
		void printToken(const char *js, jsmntok_t t);

        void sendHandshake();
        int readHandshake();
		int readLineHTTP();
		void eatHeader(void);
		bool waitForInput(void);

		int readLineSocket();

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