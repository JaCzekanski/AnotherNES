#include "APU.h"

APU::APU(void)
{
	AUDIOACCESS = true;
	audcount = 0;
	audcount2 = 0;
	gvolume = 0x0f;
	dir = false;
	dir2 = false;
	activeTimer = 0;

	osc[0].waveform = 0; // Pulse 1
	osc[0].enabled = true;
	osc[1].waveform = 0; // Pulse 2
	osc[1].enabled = true;
	osc[2].waveform = 1; // Triangle
	osc[2].enabled = true;
	osc[2].volume = 0xff;
	osc[3].waveform = 2; // Noise
	osc[3].enabled = true;

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
	for (int i = 0; i <4; i++)
	{
		if (!osc[i].enabled) continue;
		int8_t value;

		switch (osc[i].waveform) {
		case 0: // Pulse
			if (osc[i].phase > osc[i].duty) value = -32;
			else value = 31;

			osc[i].phase += (CPU_frequency / (16 * osc[i].frequency)) - 1;

			if (!osc[i].constantVolume && osc[i].lengthTimer && --osc[i].lengthTimer == 0)
			{
				osc[i].lengthTimer = osc[i].length;
				if (osc[i].volume) osc[i].volume--;
			}
			if (osc[i].sweepEnable && ((osc[i].sweepTimer&0x7ff) >> (osc[i].sweepPeriod>>1)) == 0)
			{
				if (osc[i].sweepNegative) osc[i].frequency -= osc[i].sweepShift;
				else osc[i].frequency += osc[i].sweepShift;
			}
			osc[i].sweepTimer++;
			break;
		case 1: // Triagnle
			if (osc[i].phase < 0x8000) value = -32 + (osc[i].phase >> 9);
			else value = 31 - ((osc[i].phase - 0x8000) >> 9);

			// CPU_frequency/2 ?
			osc[i].phase += (CPU_frequency / 2 / (16 * osc[i].frequency)) - 1;
			break;
		case 2: // Noise
			value = 0;

				uint8_t xor_bit = 0;
				if (osc[i].mode) xor_bit = (osc[i].phase & 0x01) ^ ((osc[i].phase & 0x40) >> 6);
				else xor_bit = (osc[i].phase & 0x01) ^ ((osc[i].phase & 0x02) >> 1);

				osc[i].phase >>= 1;
				osc[i].phase |= xor_bit << 14;

			if (osc[i].length == 0) break;
			osc[i].length--;

			value = ((osc[i].phase&0x01) == 1) ? 31 : -32;
			break;
		}

		acc += value * osc[i].volume;
	}
	return 128 + (acc>>8);
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
		if (n == 2) break;
		osc[n].duty = 8192 * (((data & 0xc0) >> 6)+1);
		osc[n].volume = (data & 0x0f) << 4;
		osc[n].volume_orig = osc[n].volume;
		if (data & 0x10) osc[n].constantVolume = true;
		else osc[n].constantVolume = false;
		break;
	case 1: // Sweep, pulse only
		if (n > 1) break;
		osc[n].sweepEnable = (data & 0x80) ? true : false;
		osc[n].sweepPeriod = (data & 0x70) >> 4;
		osc[n].sweepNegative = (data & 0x08) ? true : false;
		osc[n].sweepShift = (data & 0x0f);
		break;
	case 2:
		if (!osc[n].constantVolume) osc[n].volume = osc[n].volume_orig;

		osc[n].frequency &= ~0xff;
		osc[n].frequency |= data;

		if (n == 3)
		{
			osc[n].frequency &= 0x0f; // Noise
			osc[n].mode = (data & 0x80) ? true : false;
		}
		break;
	case 3:
		if (n == 3)
		{
			osc[n].length = (data & 0xf8) << 6;
			osc[n].lengthTimer = osc[n].length;
			break;
		}
		if (!osc[n].constantVolume) osc[n].volume = osc[n].volume_orig;

		osc[n].frequency &= ~0xff00;
		osc[n].frequency |= (data & 0x7) << 8;
		
		osc[n].length = (data & 0xf8) << 4;// >> 3;
		osc[n].lengthTimer = osc[n].length;
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