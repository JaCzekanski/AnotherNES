#pragma once
/* Mapper 4
   MMC3
 
   PRG ROM and CHR ROM remapping
   CHR RAM could be used
   Very versatile
   IRQ 
*/
#include "Mapper.h"


class Mapper4 : public Mapper
{
	int prgSize = 8 * 1024;
	int chrSize = 1 * 1024;

	uint8_t prgMode, chrMode;
	uint8_t MMC3_irqCounter = 0;
	uint8_t MMC3_reg[8];

	void MMC3_write(uint16_t n, uint8_t data);
	void MMC3_chrCopy();
	bool MMC3_irqEnabled = false;
public:
	Mapper4(PPU& ppu);
	~Mapper4();
	uint8_t Read(uint16_t n);
	void Write(uint16_t n, uint8_t data);
};

