#pragma once
#include "headers.h"
#include "PPU.h"
class iNES
{
public:
	uint8_t PRG_ROM_pages;
	uint8_t CHR_ROM_pages;
	uint8_t PRG_RAM_pages;

	uint8_t* PRG_ROM;
	uint8_t* CHR_ROM;

	Mirroring::Mirroring mirroring;
	uint8_t Mapper;

	bool Pal; // false - ntsc, true - pal


	iNES(void);
	~iNES(void);

	int Load( const char* name );
};
