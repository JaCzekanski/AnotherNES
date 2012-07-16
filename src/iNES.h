#pragma once
#include "headers.h"

class iNES
{
public:
	uint8_t PRG_ROM_pages;
	uint8_t CHR_ROM_pages;
	uint8_t PRG_RAM_pages;

	uint8_t* PRG_ROM;
	uint8_t* CHR_ROM;

	uint8_t Mirroring;


	iNES(void);
	~iNES(void);

	int Load( const char* name );
};
