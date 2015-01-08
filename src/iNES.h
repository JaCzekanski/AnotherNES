#pragma once
#include "headers.h"
#include <vector>

namespace Mirroring
{
	enum Mirroring
	{
		Horizontal = 0,
		Vertical,
		// .. and other
	};
}
namespace Region
{
	enum Region
	{
		NTSC,
		PAL
	};
}

class iNES
{
	uint8_t mapper;
	Mirroring::Mirroring mirroring;
	Region::Region region;
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
	Region::Region getRegion() { return region; }
	Mirroring::Mirroring getMirroring() { return mirroring; }
	int getMapper() { return mapper; }
};
