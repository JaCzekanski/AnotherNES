#include "APU.h"

APU::APU(void)
{
	AUDIOACCESS = true;
	audcount = 0;
	audcount2 = 0;
	gvolume = 0x0f;
	dir = false;
	dir2 = false;


	osc[0].waveform = 0; // Pulse 1
	osc[0].enabled = true;
	osc[1].waveform = 0; // Pulse 2
	osc[1].enabled = true;
	osc[2].waveform = 1; // Triangle
	osc[2].enabled = true;
	osc[2].volume = 0xff;
	osc[3].waveform = 2; // Noise

	Log->Debug("APU created");
}

APU::~APU(void)
{
	Log->Debug("APU destroyed");
}

const float CPU_frequency = 1789773;

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
			break;
		case 1: // Triagnle
			if (osc[i].phase < 0x8000) value = -32 + (osc[i].phase >> 9);
			else value = 31 - ((osc[i].phase - 0x8000) >> 9);

		}

		if (!osc[i].constantVolume && osc[i].lengthTimer && --osc[i].lengthTimer == 0)
		{
			osc[i].lengthTimer = osc[i].length;
			if (osc[i].volume) osc[i].volume--;
		}

		osc[i].phase += (CPU_frequency / (16 * osc[i].frequency)) - 1;
		acc += value * osc[i].volume;
	}
	return 128 + (acc>>8);
}

void APU::Write( uint8_t reg, uint8_t data )
{
	if (reg == 0x15) // status
	{
		for (int i = 0; i<3; i++)
			osc[i].enabled = (data & (1 << i))?true:false;
		return;
	}

	uint8_t n = reg / 4;
	if (n > 2) return;
	switch (reg%4)
	{
	case 0: // Pulse 1
		if (n == 2) break;
		osc[n].duty = 8192 * ((data & 0xc0) >> 6);
		osc[n].volume = (data & 0x0f) << 4;
		osc[n].volume_orig = osc[n].volume;
		if (n < 2)
		{
			if (data & 0x10) osc[n].constantVolume = true;
			else osc[n].constantVolume = false;
		}
		break;
	case 2:
		if (!osc[n].constantVolume) osc[n].volume = osc[n].volume_orig;

		osc[n].frequency &= ~0xff;
		osc[n].frequency |= data;
		break;
	case 3:
		if (!osc[n].constantVolume) osc[n].volume = osc[n].volume_orig;

		osc[n].frequency &= ~0xff00;
		osc[n].frequency |= (data & 0x7) << 8;

		osc[n].length = (data & 0xf8) << 4;//; >> 3;
		osc[n].lengthTimer = osc[n].length;
		break;
	}
	//AUDIO[reg] = data;
	//AUDIOACCESS = true;
}

void APU::audiocallback(void *userdata, Uint8 *stream, int len)
{
	for (int i = 0; i < len; i++) {
		stream[i] = Step();
	}
	//if (AUDIOACCESS)
	//{
	//	AUDIOACCESS = false;
	//	gvolume = 0xf;
	//}
	//for (int i = 0; i<len; i++)
	//{
	//	stream[i] = 0x7f;
	//}
	//if ( AUDIO[0x15] | 0x01 ) // PULSE 1 enabled
	//{
	//	// SQ1_ENV ($4000);
	//	uint8_t volume = AUDIO[0] & 0xf;
	//	uint8_t LengthCounterDisable = (AUDIO[0] & 0x20) >> 5;
	//	uint8_t dutycycle = (AUDIO[0] & 0xC0) >> 6;

	//	if (AUDIO[0]&0x10) // if 1 - Envelope
	//	{
	//		volume = gvolume;
	//	}
	//	else // else - volume
	//	{
	//		volume = gvolume;
	//	}

	//	if (LengthCounterDisable) // Length controlled by us
	//	{		
	//	}
	//	else
	//	{
	//		
	//	}
	//
	//	if (volume > 0)
	//	{

	//			// SQ1_LO ($4002), SQ1_HI ($4003)
	//			uint16_t period = (AUDIO[3]&0x7)<<8 | AUDIO[2];
	//			uint8_t length = (AUDIO[3]&0xf8)>>3;

	//			int freq = (int)(1789772.67f / (float)(16*(period+1)));

	//			for (int i = 0; i<len; i++)
	//			{
	//				if (dutycycle==1 || dutycycle==3)
	//				{
	//					if (audcount>(44100.f/freq) ) 
	//					{
	//						dir = !dir;
	//						audcount = 0;
	//					}
	//				}
	//				else if (dutycycle==2)
	//				{
	//					if (audcount>(44100.f/freq) ) 
	//					{
	//						dir = !dir;
	//						audcount = 0;
	//					}
	//				}

	//				if (dir) stream[i] =(volume*8)/2;
	//				audcount++;
	//			}
	//	}
	//}
	//if ( AUDIO[0x15] | 0x02 ) // PULSE 2 enabled
	//{
	//	// SQ1_ENV ($4000);
	//	uint8_t volume = AUDIO[0+4] & 0xf;
	//	uint8_t LengthCounterDisable = (AUDIO[0 + 4] & 0x20) >> 5;
	//	uint8_t dutycycle = (AUDIO[0+4] & 0xC0) >> 6;

	//	if (AUDIO[0+4]&0x10) // if 1 - Envelope
	//	{
	//		volume = gvolume;
	//	}
	//	else // else - volume
	//	{
	//		volume = gvolume;
	//	}

	//	if (LengthCounterDisable) // Length controlled by us
	//	{
	//		//Log->Debug("LengthCounterDisable: true");			
	//	}
	//	else
	//	{
	//		
	//	}
	//
	//	if (volume > 0)
	//	{
	//			// SQ1_LO ($4002), SQ1_HI ($4003)
	//			uint16_t period = (AUDIO[3+4]&0x7)<<8 | AUDIO[2+4];
	//			uint8_t length = (AUDIO[3+4]&0xf8)>>3;

	//			int freq = (int)(1789772.67f / (float)(16*(period+1)));

	//			for (int i = 0; i<len; i++)
	//			{
	//				if (dutycycle==1 || dutycycle==3)
	//				{
	//					if (audcount2>(44100.f/freq) ) 
	//					{
	//						dir2 = !dir2;
	//						audcount2 = 0;
	//					}
	//				}
	//				else if (dutycycle==2)
	//				{
	//					if (audcount2>(44100.f/freq) ) 
	//					{
	//						dir2 = !dir2;
	//						audcount2 = 0;
	//					}
	//				}

	//				if (dir2) stream[i] += (volume*8)/2;
	//				//else stream[i] = 0x7f;
	//				audcount2++;
	//			}
	//	}
	//}
	//if (gvolume>0) gvolume--;

}