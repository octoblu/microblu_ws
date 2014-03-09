#ifndef _SKYNETCLIENT_H
#define _SKYNETCLIENT_H

#include "Arduino.h"
#include "SPI.h"
#include "Client.h"
#include <EEPROM.h>
#include <avr/eeprom.h>
#include "jsmnSpark.h"

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
#define PAYLOAD "payload"
#define DEVICES "devices"
#define EMIT "5:::"
#define HEARTBEAT "2::"
#define FROMUUID "fromUuid"
#define DATA "data"

#define SID_LEN 24
#define UUIDSIZE 37
#define TOKENSIZE 33

#define EEPROMBLOCKADDRESS 0
#define EEPROMBLOCK 'S'
#define TOKENADDRESS 1
#define UUIDADDRESS TOKENADDRESS+TOKENSIZE

// Length of static data buffers
#define DATA_BUFFER_LEN 255
#define SKYNET_TX_BUFFER_SIZE 25
#define SKYNET_BUFFER_SIZE 255

struct rx_buffer;
struct tx_buffer;

class SkynetClient  {
	public:
		SkynetClient();
		
		typedef void (*MessageDelegate)(char *data);

		void setMessageDelegate(MessageDelegate messageDelegate);
		void sendMessage(const char *device, const char *object);

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
	    char lastReceivedUuid[UUIDSIZE];
		
	private:
	    rx_buffer *_rx_buffer;
	    tx_buffer *_tx_buffer;
		void dump();
		char *dataptr;
		char databuffer[DATA_BUFFER_LEN];

		IPAddress theip;
		void printByByte(const char *data);
		void printByByte(const char *data, size_t size);

		void printToken(const char *js, jsmntok_t t);

        void sendHandshake();
        int readHandshake();
		int readLine();
		bool waitForInput(void);
		void eatHeader(void);
		uint8_t status;
		void process();

        MessageDelegate messageDelegate;
		void eeprom_write_bytes(char *, int, int);
		void eeprom_read_bytes(char*, int, int);
};

#endif // _SKYNETCLIENT_H