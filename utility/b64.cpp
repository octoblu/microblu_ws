#include "b64.h"

const char base64[ 64 ] PROGMEM  = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
    'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};
  

void b64::send(ringbuffer &buffer, Client &out)
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
			DBGCS("c");
			DBGC((int)i%3);
			DBGCS(": ");
			DBGCN((int)(buffer.peek()));
			c[i%3]=buffer.pop();
      		i++;
		}while(buffer.available() && (i%3!=0));

		if (i%3==1)
		{
			d0 = (c[0] >> 2) & 63;
			d1 = ((c[0] << 4) & 48) | ((c[1] >> 4) & 15);

			DBGCS("d0: ");
			DBGC((int)d0);
			DBGCS(" : ");
			DBGC(lookup(d0));
			out.print( lookup(d0) );
			DBGCS("d1: ");
			DBGC((int)d1);
			DBGCS(" : ");
			DBGC(lookup(d1));
			out.print( lookup(d1) );
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

			DBGCS("d0: ");
			DBGC((int)d0);
			DBGCS(" : ");
			DBGC(lookup(d0));
			out.print(lookup(d0));

			DBGCS("d1: ");
			DBGC((int)d1);
			DBGCS(" : ");
			DBGC(lookup(d1));
			out.print(lookup(d1));

			DBGCS("d2: ");
			DBGC((int)d2);
			DBGCS(" : ");
			DBGC(lookup(d2));
			out.print(lookup(d2));
			DBGC('=');
		    out.print( '=' );
		}
		else
		{
			d0 = (c[0] >> 2) & 63;
			d1 = ((c[0] << 4) & 48) | ((c[1] >> 4) & 15);
			d2 = ((c[1] << 2) & 60) | ((c[2] >> 6) & 3);
			d3 = c[2] & 63;

			DBGCS("d0: ");
			DBGC((int)d0);
			DBGCS(" : ");
			DBGC(lookup(d0));
			out.print(lookup(d0));

			DBGCS("d1: ");
			DBGC((int)d1);
			DBGCS(" : ");
			DBGC(lookup(d1));
			out.print(lookup(d1));

			DBGCS("d2: ");
			DBGC((int)d2);
			DBGC(" : ");
			DBGC(lookup(d2));
			out.print(lookup(d2));

			DBGCS("d3: ");
			DBGC((int)d3);
			DBGC(" : ");
			DBGC(lookup(d3));
			out.print(lookup(d3));
		}
	}
}

void b64::send(const uint8_t *buf, size_t size, Client &out)
{
	int i = 0;
	while(i<size)
	{
	
	}
	DBGCN();
}

char b64::lookup(const char c)
{
	return pgm_read_byte( &base64[ c ]);
}

void b64::decodestore(char *src, ringbuffer &buffer)
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
			DBGCS("c");
			DBGC(i%4);
			DBGCS(": ");
			DBGC(src[i]);
			DBGCS(": ");
			DBGCN((int)reverselookup(src[i]));
			c[i%4]=reverselookup(src[i++]);
		}while( src[i] != '\0' && i%4 != 0 );

		d0 = ((c[0] << 2) & 252) | (c[1] >>4 & 3);
		DBGCS("d0: ");
		DBGCN(d0, HEX);
    	buffer.push(d0);
    	
    	//if c3 is equal sign (negative one in our lookup), we ignore the second to last character
		if(c[3] != -1)
		{
			d1 = ((c[1] << 4) & 240) | (c[2] >>2 & 15);
			DBGCS("d1: ");
			DBGCN(d1, HEX);
    		buffer.push(d1);
		}

		//if c2 is equal sign (negative one in our lookup), we ignore the second to last character
		if(c[2] != -1)
		{
			d2 = ((c[2] << 6) & 192) | (c[3] & 63);
			DBGCS("d2: ");
			DBGCN(d2, HEX);
	    	buffer.push(d2);
	    }
	}
	DBGCN();
}

char b64::reverselookup(const char c)
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
