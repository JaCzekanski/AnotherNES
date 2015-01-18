#pragma once
#include "headers.h"
#include <sdl.h>

static uint16_t noiseLookup[] = {
	0x004,
	0x008,
	0x010,
	0x020,
	0x040,
	0x060,
	0x080,
	0x0A0,
	0x0CA,
	0x0FE,
	0x17C,
	0x1FC,
	0x2FA,
	0x3F8,
	0x7F2,
	0xFE4
};

static uint8_t lenghtsLookup[] = {
	0x0A, 0xFE,
	0x14, 0x02,
	0x28, 0x04,
	0x50, 0x06,
	0xA0, 0x08,
	0x3C, 0x0A,
	0x0E, 0x0C,
	0x1A, 0x0E,
	0x0C, 0x10,
	0x18, 0x12,
	0x30, 0x14,
	0x60, 0x16,
	0xC0, 0x18,
	0x48, 0x1A,
	0x10, 0x1C,
	0x20, 0x1E
};

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

	uint16_t length;
	uint16_t lengthTimer;
	bool constantVolume;

	// Pulse
	bool sweepWrite = false;
	uint16_t sweepTimer;
	bool sweepEnable;
	bool sweepNegative;
	uint8_t sweepPeriod;
	uint8_t sweepShift;
	
	// Envelope
	bool envelopeLoop;
	uint16_t envelopeCounter = 0;
	uint16_t envelopeDivider = 0;
	uint16_t envelopeDividerCounter = 0;
	bool envelopeCounterReset = false;

	// Noise
	bool noiseLoop;
	oscillator()
	{
		duty = 0;
		envelopeCounter = 0;
		envelopeDivider = 0;
		envelopeLoop = false;

		constantVolume = true;
		enabled = false;
		volume = 0;
	}
};

class APU
{
private:
	bool frameCounterMode = false; // 0 - 4 step, 1 - 5 step
	bool frameCounterInterruptEnabled = false;
	int frameCounterStep = 0;
	oscillator osc[4];
public:
	int activeTimer; // Timer incremented by emulator, if value is not changing then emu is inactive
	APU(void);
	~APU(void);

	uint8_t Step(); 
	bool frameStep();

	void Write( uint8_t reg, uint8_t data );

	void audiocallback(void *userdata, Uint8 *stream, int len);
};

