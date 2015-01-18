#include "APU.h"

APU::APU(void)
{
	activeTimer = 0;

	osc[0].waveform = 0; // Pulse 1
	osc[0].enabled = false;
	osc[1].waveform = 0; // Pulse 2
	osc[1].enabled = false;
	osc[2].waveform = 1; // Triangle
	osc[2].enabled = false;
	osc[2].volume = 0xff;
	osc[3].waveform = 2; // Noise
	osc[3].enabled = false;

	Log->Debug("APU created");
}

APU::~APU(void)
{
	Log->Debug("APU destroyed");
}

const float CPU_frequency = 1789773 *1.5f;

uint8_t APU::Step()
{
	int16_t acc = 0;
	uint8_t value = 0;
	float pulse[2], triangle = 0, noise = 0;
	for (int i = 0; i <4; i++)
	{
		if (!osc[i].enabled) continue;
		int8_t value;

		switch (osc[i].waveform) {
		case 0: // Pulse
			{
			int step = osc[i].phase / 8192;
			value = (DutyCycle[osc[i].duty][step]) ? 31 : -32;
			//if (osc[i].phase > osc[i].duty) value = -32;
			//else value = 31;
			osc[i].phase += (CPU_frequency / (8 * osc[i].frequency)) - 1;
			break;
			}
		case 1: // Triagnle
			if (osc[i].phase < 0x8000) value = -32 + (osc[i].phase >> 9);
			else value = 31 - ((osc[i].phase - 0x8000) >> 9);
			osc[i].phase += (CPU_frequency / 2 / (8 * osc[i].frequency)) - 1;
			break;
		case 2: // Noise
			uint8_t xor_bit = 0;
			if (osc[i].noiseLoop) xor_bit = (osc[i].phase & 0x01) ^ ((osc[i].phase & 0x40) >> 6);
			else xor_bit = (osc[i].phase & 0x01) ^ ((osc[i].phase & 0x02) >> 1);

			osc[i].phase >>= 1;
			osc[i].phase |= xor_bit << 14;

			value = ((osc[i].phase&0x01) == 1) ? 31 : -32;
			value /= 3;
			break;
		}

		acc += value * ((osc[i].volume&0xf)<<4);
	}
	return 128 + (acc >> 8);
}

bool APU::frameStep()
{
	bool irq = false;

	if (frameCounterStep == 3 && !frameCounterMode && frameCounterInterruptEnabled) irq = true;

	// Envelope and linear counter
	if (frameCounterStep <= 3) {
		for (int i : {0, 1, 3}) {
			if (osc[i].envelopeCounterReset) {
				osc[i].envelopeCounterReset = false;
				osc[i].envelopeCounter = 15;
				osc[i].envelopeDividerCounter = osc[i].envelopeDivider + 1;
			}
			else osc[i].envelopeDividerCounter--;

			if (osc[i].envelopeDividerCounter == 0) {
				if (osc[i].envelopeCounter == 0 && osc[i].envelopeLoop) osc[i].envelopeCounter = 15;
				else if (osc[i].envelopeCounter != 0) osc[i].envelopeCounter--;

				osc[i].envelopeDividerCounter = osc[i].envelopeDivider + 1;
			}
			if (!osc[i].constantVolume) {
				if (!osc[i].envelopeLoop && osc[i].length == 0) continue;
				osc[i].volume = osc[i].envelopeCounter;
			}
		}
	}

	// Length counter and sweep
	if (!frameCounterMode && (frameCounterStep == 1 || frameCounterStep == 3) ||
		 frameCounterMode && (frameCounterStep == 0 || frameCounterStep == 2)) {
		for (int i : {0, 1, 2, 3}) {
			if (osc[i].envelopeLoop) continue;
			if (osc[i].length > 0) {
				if (--osc[i].length == 0) osc[i].volume = 0;
			}
		}

		for (int i : {0, 1}) {
			if (!osc[i].sweepWrite && osc[i].sweepTimer > 0) osc[i].sweepTimer--;
			if (!osc[i].sweepWrite && osc[i].sweepTimer == 0 && osc[i].sweepEnable) {
				osc[i].sweepTimer = osc[i].sweepPeriod + 1;

				if (osc[i].sweepShift > 0)  {
					uint16_t newperiod = osc[i].frequency >> osc[i].sweepShift;

					if (osc[i].sweepNegative) {
						if (i == 1) newperiod += 1;
						osc[i].frequency -= (newperiod + 1);
					}
					else osc[i].frequency += newperiod;
				}
			}
			if (osc[i].sweepWrite) {
				osc[i].sweepWrite = false;
				osc[i].sweepTimer = osc[i].sweepPeriod + 1;
			}
		}

	}

	++frameCounterStep;
	if (!frameCounterMode && frameCounterStep == 4) frameCounterStep = 0;
	if ( frameCounterMode && frameCounterStep == 5) frameCounterStep = 0;
	return irq;
}

void APU::Write( uint8_t reg, uint8_t data )
{
	if (reg == 0x15) // status
	{
		for (int i = 0; i<4; i++)
			osc[i].enabled = (data & (1 << i))?true:false;
		return;
	}

	uint8_t n = reg / 4;
	if (n > 3) return;
	switch (reg%4)
	{
	case 0: 
		if (n == 2) {
			osc[n].envelopeLoop = (data & 0x80) ? true : false;
			if (osc[n].envelopeLoop) osc[n].length = 0;
			break;
		}
		if (n <= 1) osc[n].duty = (data & 0xc0) >> 6;
		osc[n].volume = (data & 0x0f);

		osc[n].envelopeLoop = (data & 0x20) ? true : false;
		osc[n].constantVolume = (data & 0x10) ? true : false;
		if (!osc[n].constantVolume) osc[n].envelopeDivider = osc[n].volume;
		if (osc[n].envelopeLoop) osc[n].length = 0;
		break;
	case 1: // Sweep, pulse only
		if (n > 1) break;
		osc[n].sweepEnable = (data & 0x80) ? true : false;
		osc[n].sweepPeriod = (data & 0x70) >> 4;
		osc[n].sweepNegative = (data & 0x08) ? true : false;
		osc[n].sweepShift = (data & 0x07);
		osc[n].sweepWrite = true;
		break;
	case 2:
		if (n == 3)
		{
			osc[n].phase = noiseLookup[data & 0xf];
			osc[n].noiseLoop = (data & 0x80) ? true : false;
			break;
		}

		osc[n].frequency &= ~0xff;
		osc[n].frequency |= data;
		break;
	case 3:
		osc[n].length = lenghtsLookup[(data & 0xf8) >> 3];
		osc[n].envelopeCounterReset = true;
		if (n == 3) break;

		osc[n].frequency &= ~0xff00;
		osc[n].frequency |= (data & 0x7) << 8;
		break;
	}
}

void APU::audiocallback(void *userdata, Uint8 *stream, int len)
{
	static int _activeTimer = 0;

	if (activeTimer == _activeTimer) return;
	_activeTimer = activeTimer;

	for (int i = 0; i < len; i++) {
		stream[i] = Step();
	}
}