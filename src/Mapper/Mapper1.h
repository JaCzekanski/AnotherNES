#pragma once
/* Mapper 1
   MMC1
 
   PRG ROM remapping only
   CHR RAM used
*/
#include "Mapper.h"


class Mapper1 : public Mapper
{
	int prgSize = 16 * 1024;
	int chrSize = 4 * 1024;

	uint8_t counter, reg;
	bool slotSelect, prgMode, chrMode;
	uint8_t chrReg0, chrReg1, prgReg;

	void MMC1_write(uint16_t n, uint8_t data);
public:
	Mapper1(PPU& ppu);
	~Mapper1();
	uint8_t Read(uint16_t n);
	void Write(uint16_t n, uint8_t data);
};

