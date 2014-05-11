#ifndef _B64_H
#define _B64_H

#ifdef SPARK
#include "application.h"
#include "pgmspace.h"
#else
#include "Arduino.h"
#include "avr/pgmspace.h"
#include "Client.h"
#endif

#include "ringbuffer.h"

//#define B64_DEBUG
#ifdef B64_DEBUG
#define DBGCN( ... ) Serial.println( __VA_ARGS__ )
#define DBGC( ... ) Serial.print( __VA_ARGS__ )
#define DBGCSN( ... ) Serial.println( F(__VA_ARGS__) )
#define DBGCS( ... ) Serial.print( F(__VA_ARGS__) )
#else
#define DBGCN( ... )
#define DBGC( ... )
#define DBGCSN( ... )
#define DBGCS( ... )
#endif

class b64 {
	public:
		static void decodestore(char *src, ringbuffer &buffer);
		static void send(ringbuffer &buffer, Client &out);
		static void send(const uint8_t *buf, size_t size, Client &out);
		static char lookup(const char c);
		static char reverselookup(const char c);
};
#endif // _B64_H