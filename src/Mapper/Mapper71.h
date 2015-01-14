#pragma once
/* Mapper 71
   Camerica games, almost indentical to Mapper 2
 
   PRG ROM remapping only
   CHR RAM used
*/
#include "Mapper.h"


class Mapper71 : public Mapper
{
	const int prgSize = 16 * 1024;
	uint8_t prgReg;
public:
	Mapper71(PPU& ppu);
	~Mapper71();
	uint8_t Read(uint16_t n);
	void Write(uint16_t n, uint8_t data);
};

