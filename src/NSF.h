#pragma once
#include "headers.h"

class NSF
{
private:
public:
	uint8_t song_count;
	uint8_t song_start;

	uint16_t load_address;
	uint16_t init_address;
	uint16_t play_address;

	uint16_t speed_ntsc;
	uint16_t speed_pal;

	uint8_t pal_ntsc_bits;
	uint8_t extra_soundchips;

	uint8_t *data;
	int size;

	NSF(void);
	~NSF(void);

	int Load( const char* name );
};
