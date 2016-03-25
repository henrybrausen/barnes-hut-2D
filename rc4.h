#ifndef __RC4_H__
#define __RC4_H__

#include <stdint.h>
#include <string>

using namespace std;

// Implementation of an RC4 stream cypher.
// Useful as a pseudo-random number generator
class rc4 {
public:
	static void apply_key(const string& key);
	static const uint8_t next_byte();
protected:
	static void swap(unsigned int i, unsigned int j);
	static uint8_t s[256];
	static unsigned int i,j;
};

unsigned int rand32();

#endif
