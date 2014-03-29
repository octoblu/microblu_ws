#ifndef _SKYNETCLIENT_H
#define _SKYNETCLIENT_H

#include "Arduino.h"
#include "SPI.h"
#include "Client.h"
#include <EEPROM.h>
#include <avr/eeprom.h>
#include "jsmnSpark.h"

//#define SKYNETCLIENT_DEBUG
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

#define EEPROMBLOCKADDRESS 0
#define EEPROMBLOCK 'S'
#define TOKENADDRESS EEPROMBLOCKADDRESS+1
#define UUIDADDRESS TOKENADDRESS+TOKENSIZE

#define MAX_PARSE_OBJECTS 20

// Length of static data buffers
#define SOCKET_RX_BUFFER_SIZE 200 //200 currently covers biggest skynet message like READY
#define SKYNET_TX_BUFFER_SIZE 160 //160 is needed for firmata's capability query
#define SKYNET_RX_BUFFER_SIZE 32

struct rx_buffer;
struct tx_buffer;

class SkynetClient : public Stream {
	public:
		SkynetClient(Client &_client);
		
		typedef void (*MessageDelegate)(const char *data);

		void setMessageDelegate(MessageDelegate messageDelegate);
		void sendMessage(const char* device, char const *object);

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
	    void flushTX();
	    void stop();
	    uint8_t connected();
	    operator bool();
		void monitor();

		char token[TOKENSIZE];
		char uuid[UUIDSIZE];
		
	private:
		Client* client;
	    rx_buffer *_rx_buffer;
	    tx_buffer *_tx_buffer;
		char databuffer[SOCKET_RX_BUFFER_SIZE];
		uint8_t status;
		uint8_t bind;
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
		void processData(const char *data);
		void processSkynet(const char *data, const char ack);
		void b64decodestore(char *src, rx_buffer *buffer);
		void b64send(tx_buffer *buffer, Client &out);
		void b64send(const uint8_t *buf, size_t size, Client &out);
		char b64lookup(const char c);
		char b64reverselookup(const char c);

		void eeprom_write_bytes(int, char*, int);
		void eeprom_read_bytes(int, char*, int);
};

#endif // _SKYNETCLIENT_H