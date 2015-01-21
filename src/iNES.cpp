#include "iNES.h"

iNES::iNES(void)
{
	PRG_ROM.clear();
	CHR_ROM.clear();
	mapper = 0;
}

iNES::~iNES(void)
{
}

int iNES::Load( const char* name )
{
	FILE* rom = fopen(name, "rb");

	if (!rom)
	{
		Log->Error("iNES: Cannot open %s", rom);
		return 1;
	}
	
	char buffer[16];
	fread(buffer, 1, 16, rom);
	if (memcmp(buffer, "NES\x1A", 4))
	{
		Log->Error("iNES: Wrong magic: expected NES\\x1a");
		return 2; // Wrong MAGIC
	}
	if (!memcmp(buffer + 7, "DiskDude!", 9))
	{
		Log->Error("iNes.cpp: Garbage header, please use another ROM, will try to boot");
		memset(buffer + 7, 0, 9);
	}

	PRG_ROM_pages  = buffer[4]; // Size of PRG ROM in 16KB units
	CHR_ROM_pages  = buffer[5]; // Size of PRG ROM in 8KB units (0 - uses CHR RAM)
	uint8_t flags6 = buffer[6];
	uint8_t flags7 = buffer[7];
	PRG_RAM_pages  = buffer[8]; // Size of PRG RAM in 8KB units
	uint8_t flags9 = buffer[9];
	
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
		region = Region::PAL;
		Log->Info("iNes: PAL");
	}
	else
	{
		region = Region::NTSC;
		Log->Info("iNes: NTSC");
	}

	if (flags6 & 0x08)
	{
		Log->Info("iNes.cpp: 4 screen mirroring");
		mirroring = Mirroring::FourScreen;
	}
	else
	{
		if (flags6 & 0x01)
		{
			Log->Info("iNes.cpp: Vertical mirroring");
			mirroring = Mirroring::Vertical;
		}
		else
		{
			Log->Info("iNes.cpp: Horizontal mirroring");
			mirroring = Mirroring::Horizontal;
		}
	}

	mapper = ((flags6&0xf0)>>4) | (flags7&0xf0);
	Log->Info("iNes: Mapper %d", mapper);

	if (flags6&0x04) // Trainer
		fseek(rom, 512, SEEK_CUR);

	PRG_ROM.resize(PRG_ROM_pages * prgPageSize);
	CHR_ROM.resize(CHR_ROM_pages * chrPageSize);

	if (PRG_ROM_pages>0) fread(&PRG_ROM[0], 1, PRG_ROM_pages * prgPageSize, rom);
	if (CHR_ROM_pages>0) fread(&CHR_ROM[0], 1, CHR_ROM_pages * chrPageSize, rom);

	fclose( rom );
	Log->Debug("iNES loaded");
	return 0;
}
