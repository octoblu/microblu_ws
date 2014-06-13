#ifndef _RINGBUFFER_H
#define _RINGBUFFER_H

#include <stdio.h>
#include <stdlib.h>

class ringbuffer {
	public:
	    ringbuffer(size_t _size);

	    ~ringbuffer();

	    void push(unsigned char c);
	    
	    unsigned char pop();

	    unsigned char peek();

	    bool available();

	private:
	    unsigned char * buffer;
	    size_t size;
	    size_t head;
	    size_t tail;
};

#endif // _RINGBUFFER_H