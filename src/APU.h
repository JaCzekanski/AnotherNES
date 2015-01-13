#pragma once
#include "headers.h"
#include <sdl.h>

static bool DutyCycle[4][8] = {
	{ 0, 1, 0, 0, 0, 0, 0, 0 }, // 0, 12.5%
	{ 0, 1, 1, 0, 0, 0, 0, 0 }, // 1, 25%
	{ 0, 1, 1, 1, 1, 0, 0, 0 }, // 2, 50%
	{ 1, 0, 0, 1, 1, 1, 1, 1 }  // 3. 25% negated
};

struct oscillator
{
	bool enabled;
	uint16_t frequency;
	uint16_t phase;
	uint16_t duty;
	uint8_t waveform;
	uint8_t volume;
	uint8_t volume_orig;

	uint16_t length;
	uint16_t lengthTimer;
	bool constantVolume;

	// Pulse
	uint16_t sweepTimer;
	bool sweepEnable;
	bool sweepNegative;
	uint8_t sweepPeriod;
	uint8_t sweepShift;


	// Noise
	bool mode;
	oscillator()
	{
		constantVolume = true;
		enabled = false;
		volume = 0;
	}
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

	oscillator osc[4];
public:
	int activeTimer; // Timer incremented by emulator, if value is not changing then emu is inactive
	APU(void);
	~APU(void);

	uint8_t Step();

	void Write( uint8_t reg, uint8_t data );

	void audiocallback(void *userdata, Uint8 *stream, int len);
};

