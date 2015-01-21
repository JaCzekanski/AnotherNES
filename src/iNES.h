#pragma once
#include "headers.h"
#include <vector>
#include "PPU.h"

enum class Region
{
	NTSC,
	PAL
};

class iNES
{
	uint8_t mapper;
	Mirroring mirroring;
	Region region;
public:
	static const int prgPageSize = 16 * 1024;
	static const int chrPageSize = 8 * 1024;

	uint8_t PRG_ROM_pages;
	uint8_t CHR_ROM_pages;
	uint8_t PRG_RAM_pages;

	std::vector<uint8_t> PRG_ROM;
	std::vector<uint8_t> CHR_ROM;

	iNES(void);
	~iNES(void);

	int Load( const char* name );
	Region getRegion() { return region; }
	Mirroring getMirroring() { return mirroring; }
	uint8_t getMapper() { return mapper; }
};
