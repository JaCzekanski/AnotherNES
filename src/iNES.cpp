#include "iNES.h"

iNES::iNES(void)
{
	PRG_ROM = NULL;
	CHR_ROM = NULL;
	Mapper = 0;
	Log->Debug("iNES created");
}

iNES::~iNES(void)
{
	if (PRG_ROM != NULL) 
	{
		delete PRG_ROM;
		PRG_ROM = NULL;
	}
	if (CHR_ROM != NULL)
	{
		delete CHR_ROM;
		CHR_ROM = NULL;
	}
	Log->Debug("iNES destroyed");
}
int iNES::Load( const char* name )
{
	FILE* rom = fopen(name, "rb");

	if (!rom)
	{
		Log->Error("iNES.cpp: Cannot open %s", rom);
		return 1;
	}
	
	char magic[4];
	fread( magic, 1, 4, rom );
	if (memcmp( magic, "NES\x1A", 4))
	{
		Log->Error("iNES.cpp: Wrong magic: expected NES\\x1a");
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
	if (flags9 & 0x01)
	{
		Pal = true;
		Log->Info("iNes.cpp: PAL");
	}
	else
	{
		Pal = false;
		Log->Info("iNes.cpp: NTSC");
	}

	if (flags6&0x01) 
	{
		Log->Info("iNes.cpp: Vertical mirroring");
		Mirroring = 1;
	}
	else 
	{
		Log->Info("iNes.cpp: Horizontal mirroring");
		Mirroring = 0;
	}

	Mapper = ((flags6&0xf0)>>4) | (flags7&0xf0);

	if (flags6&0x04) // Trainer
	{
		fseek(rom, 512, SEEK_CUR);
	}

	PRG_ROM = new uint8_t[ PRG_ROM_pages*16*1024 ];
	CHR_ROM = new uint8_t[ CHR_ROM_pages*8*1024 ];

	fread( PRG_ROM, 1, PRG_ROM_pages*16*1024, rom );
	fread( CHR_ROM, 1, CHR_ROM_pages*8*1024, rom );

	fclose( rom );
	Log->Debug("iNES loaded");
	return 0;
}
