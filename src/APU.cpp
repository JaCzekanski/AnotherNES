#include "APU.h"

APU::APU(void)
{
	AUDIOACCESS = true;
	audcount = 0;
	audcount2 = 0;
	gvolume = 0x0f;
	dir = false;
	dir2 = false;
	Log->Debug("APU created");
}

APU::~APU(void)
{
	Log->Debug("APU destroyed");
}

void APU::Write( uint8_t reg, uint8_t data )
{
	AUDIO[reg] = data;
	AUDIOACCESS = true;
}

void APU::audiocallback(void *userdata, Uint8 *stream, int len)
{
	if (AUDIOACCESS)
	{
		AUDIOACCESS = false;
		gvolume = 0xf;
	}
	for (int i = 0; i<len; i++)
	{
		stream[i] = 0x7f;
	}
	if ( AUDIO[0x15] | 0x01 ) // PULSE 1 enabled
	{
		// SQ1_ENV ($4000);
		uint8_t volume = AUDIO[0] & 0xf;
		bool LengthCounterDisable = (AUDIO[0]&0x20)>>5;
		uint8_t dutycycle = (AUDIO[0] & 0xC0) >> 6;

		if (AUDIO[0]&0x10) // if 1 - Envelope
		{
			volume = gvolume;
		}
		else // else - volume
		{
			volume = gvolume;
		}

		if (LengthCounterDisable) // Length controlled by us
		{		
		}
		else
		{
			
		}
	
		if (volume > 0)
		{

				// SQ1_LO ($4002), SQ1_HI ($4003)
				uint16_t period = (AUDIO[3]&0x7)<<8 | AUDIO[2];
				uint8_t length = (AUDIO[3]&0xf8)>>3;

				int freq = 1789772.67f / (float)(16*(period+1));

				for (int i = 0; i<len; i++)
				{
					if (dutycycle==1 || dutycycle==3)
					{
						if (audcount>(44100.f/freq) ) 
						{
							dir = !dir;
							audcount = 0;
						}
					}
					else if (dutycycle==2)
					{
						if (audcount>(44100.f/freq) ) 
						{
							dir = !dir;
							audcount = 0;
						}
					}

					if (dir) stream[i] =(volume*8)/2;
					audcount++;
				}
		}
	}
	if ( AUDIO[0x15] | 0x02 ) // PULSE 2 enabled
	{
		// SQ1_ENV ($4000);
		uint8_t volume = AUDIO[0+4] & 0xf;
		bool LengthCounterDisable = (AUDIO[0+4]&0x20)>>5;
		uint8_t dutycycle = (AUDIO[0+4] & 0xC0) >> 6;

		if (AUDIO[0+4]&0x10) // if 1 - Envelope
		{
			volume = gvolume;
		}
		else // else - volume
		{
			volume = gvolume;
		}

		if (LengthCounterDisable) // Length controlled by us
		{
			//Log->Debug("LengthCounterDisable: true");			
		}
		else
		{
			
		}
	
		if (volume > 0)
		{
				// SQ1_LO ($4002), SQ1_HI ($4003)
				uint16_t period = (AUDIO[3+4]&0x7)<<8 | AUDIO[2+4];
				uint8_t length = (AUDIO[3+4]&0xf8)>>3;

				int freq = 1789772.67f / (float)(16*(period+1));

				for (int i = 0; i<len; i++)
				{
					if (dutycycle==1 || dutycycle==3)
					{
						if (audcount2>(44100.f/freq) ) 
						{
							dir2 = !dir2;
							audcount2 = 0;
						}
					}
					else if (dutycycle==2)
					{
						if (audcount2>(44100.f/freq) ) 
						{
							dir2 = !dir2;
							audcount2 = 0;
						}
					}

					if (dir2) stream[i] += (volume*8)/2;
					//else stream[i] = 0x7f;
					audcount2++;
				}
		}
	}
	if (gvolume>0) gvolume--;

}