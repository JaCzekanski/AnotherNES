#pragma once
/* Mapper 65
   Irem's H3001 mapper
 
   PRG ROM and CHR ROM remapping
   IRQ 
*/
#include "Mapper.h"


class Mapper65 : public Mapper
{
	int prgSize = 8 * 1024;
	int chrSize = 1 * 1024;

	uint8_t prgReg[3];
	uint8_t chrReg[8];

	void chrCopy();
public:
	uint16_t irqCounter = 0;
	bool irqEnabled;

	Mapper65(PPU& ppu);
	~Mapper65();
	uint8_t Read(uint16_t n);
	void Write(uint16_t n, uint8_t data);
};

