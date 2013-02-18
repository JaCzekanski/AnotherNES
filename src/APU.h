#pragma once
#include "headers.h"
#include <sdl.h>

static bool DutyCycle[4][8] = {
	{ 0, 1, 0, 0, 0, 0, 0, 0 }, // 0, 12.5%
	{ 0, 1, 1, 0, 0, 0, 0, 0 }, // 1, 25%
	{ 0, 1, 1, 1, 1, 0, 0, 0 }, // 2, 50%
	{ 1, 0, 0, 1, 1, 1, 1, 1 }  // 3. 25% negated
};

class APU
{
private :
	int AUDIO[0x20];
	bool AUDIOACCESS;
	int audcount;
	int audcount2;
	int gvolume;
	bool dir;
	bool dir2;
public:
	APU(void);
	~APU(void);

	void Write( uint8_t reg, uint8_t data );

	void audiocallback(void *userdata, Uint8 *stream, int len);
};

