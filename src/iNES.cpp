#include "iNES.h"

iNES::iNES(void)
{
	PRG_ROM = NULL;
	CHR_ROM = NULL;
	log->Debug("iNES created");
}

iNES::~iNES(void)
{
	if (PRG_ROM != NULL) delete PRG_ROM;
	if (CHR_ROM != NULL) delete CHR_ROM;
	log->Debug("iNES destroyed");
}
int iNES::Load( const char* name )
{
	FILE* rom = fopen(name, "rb");

	if (!rom)
	{
		log->Error("iNES.cpp: Cannot open %s", rom);
		return 1;
	}
	
	char magic[4];
	fread( magic, 1, 4, rom );
	if (memcmp( magic, "NES\x1A", 4))
	{
		log->Error("iNES.cpp: Wrong magic: expected NES\\x1a");
		return 2; // Wrong MAGIC
	}

	PRG_ROM_pages = fgetc( rom ); // Size of PRG ROM in 16KB units
	CHR_ROM_pages = fgetc( rom ); // Size of PRG ROM in 8KB units (0 - uses CHR RAM)
	uint8_t flags6 = fgetc( rom );
	uint8_t flags7 = fgetc( rom );
	PRG_RAM_pages = fgetc( rom ); // Size of PRG RAM in 8KB units
	uint8_t flags9 = fgetc( rom );
	fseek( rom, 6, SEEK_CUR );

	/* 
	Flags 6
	76543210
	||||||||
	||||+||+- 0xx0: vertical arrangement/horizontal mirroring
	|||| ||   0xx1: horizontal arrangement/vertical mirroring
	|||| ||   1xxx: four-screen mirroring
	|||| |+-- 1: SRAM in CPU $6000-$7FFF, if present, is battery backed
	|||| +--- 1: 512-byte trainer at $7000-$71FF (stored before PRG data)
	++++----- Lower nybble of mapper number


	Flags 7
	76543210
	||||||||
	|||||||+- VS Unisystem
	||||||+-- PlayChoice-10 (8KB of Hint Screen data stored after CHR data)
	||||++--- If equal to 2, flags 8-15 are in NES 2.0 format
	++++----- Upper nybble of mapper number

	Flags 9
	76543210
	||||||||
	|||||||+- TV system (0: NTSC; 1: PAL)
	+++++++-- Reserved, set to zero

	  Source: http://wiki.nesdev.com/w/index.php/INES
	*/

	uint8_t mapper = ((flags6&0xf0)>>8) |
					 (flags7&0xf0);

	if (mapper != 0)
	{
		log->Error("iNES.cpp: Only NROM mapper supported for now");
		return 3;
	}

	if (flags6&0x04) // Trainer
	{
		fseek(rom, 512, SEEK_CUR);
	}

	PRG_ROM = new uint8_t[ PRG_ROM_pages*16*1024 ];
	CHR_ROM = new uint8_t[ CHR_ROM_pages*8*1024 ];

	fread( PRG_ROM, 1, PRG_ROM_pages*16*1024, rom );
	fread( CHR_ROM, 1, CHR_ROM_pages*8*1024, rom );

	log->Debug("iNES loaded");
	return 0;
}
