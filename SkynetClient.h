#ifndef _SKYNETCLIENT_H
#define _SKYNETCLIENT_H

#include "jsmnSpark.h"


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

#define UUIDSIZE 37
#define TOKENSIZE 33

// Length of static data buffers
#define DATA_BUFFER_LEN 255
#define SID_LEN 24

#define SKYNETCLIENT_DEBUG
#ifdef SKYNETCLIENT_DEBUG
#define DBGCN( ... ) Serial.println( __VA_ARGS__ )
#define DBGC( ... ) Serial.print( __VA_ARGS__ )
#else
#define DBGCN( ... )
#define DBGC( ... )
#endif

#if (RAMEND < 1000)
  #define SKYNET_BUFFER_SIZE 200
#else
  #define SKYNET_BUFFER_SIZE 255
#endif
struct ring_buffer;

class SkynetClient  {
	public:
		SkynetClient();
		
		typedef void (*MessageDelegate)(char *data);

		void setMessageDelegate(MessageDelegate messageDelegate);
		void sendMessage(char device[], char object[]);

	    int connect(char *host, uint16_t port);
	    size_t write(const uint8_t *buf, size_t size);
	    int available();
	    int read();
	    // int read(uint8_t *buf, size_t size);
	    int peek();
	    void flush();
	    void stop();
	    uint8_t connected();
	    operator bool();
		void monitor();
		
		char token[TOKENSIZE];
		char uuid[UUIDSIZE];
		
	private:
	    ring_buffer *_rx_buffer;	
		void dump();
		char *dataptr;
		char databuffer[DATA_BUFFER_LEN];
		char sid[SID_LEN];
		char *theip;
		void printByByte(char*);		
		void printToken(char *js, jsmntok_t t);

        void sendHandshake();
        int readHandshake();
		int readLine();
		bool waitForInput(void);
		void eatHeader(void);
		uint8_t status;
		void process();

        MessageDelegate messageDelegate;
};

#endif // _SKYNETCLIENT_H