#pragma once
/* Mapper 2
   UxROM
 
   PRG ROM remapping only
   CHR RAM used
*/
#include "Mapper.h"


class Mapper2 : public Mapper
{
	const int prgSize = 16 * 1024;
	uint8_t prgReg;
public:
	Mapper2(PPU& ppu);
	~Mapper2();
	uint8_t Read(uint16_t n);
	void Write(uint16_t n, uint8_t data);
};

