#include "rc4.h"

#include <stdint.h>
#include <string>

using namespace std;

uint8_t rc4::s[256];
unsigned int rc4::i;
unsigned int rc4::j;

unsigned int rand32()
{
	unsigned int ret = 0;
	for (int i = 0; i < 4; ++i)
		ret |= (unsigned int)rc4::next_byte() << i*8;
	return ret;
}

void rc4::swap(unsigned int i, unsigned int j)
{
	uint8_t t = s[i];
	s[i]=s[j];
	s[j]=t;
}

void rc4::apply_key(const string& key)
{
	for(i=0; i < 256; ++i) {
		s[i]=i;
	}
	for(i=j=0; i < 256; ++i) {
		j = (j + s[i] + key[i%key.length()])&255;
		swap(i,j);
	}
	i=j=0;
}

const uint8_t rc4::next_byte()
{
	i = (i+1)&255;
	j = (j+s[i])&255;
	swap(i,j);
	return s[(s[i]+s[j])&255];
}