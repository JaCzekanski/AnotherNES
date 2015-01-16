#pragma once
/* Mapper 7
   AxROM
 
   PRG ROM remapping only
   CHR RAM used
*/
#include "Mapper.h"


class Mapper7 : public Mapper
{
	const int prgSize = 32 * 1024;
	uint8_t prgReg;
public:
	Mapper7(PPU& ppu);
	~Mapper7();
	uint8_t Read(uint16_t n);
	void Write(uint16_t n, uint8_t data);
};

