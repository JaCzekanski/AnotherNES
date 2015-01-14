#pragma once
/* Mapper 3
   CNROM
 
   CHR ROM remapping only
*/
#include "Mapper.h"


class Mapper3 : public Mapper
{
	const int prgSize = 16 * 1024;
	const int chrSize = 8 * 1024;
public:
	Mapper3(PPU& ppu);
	~Mapper3();
	uint8_t Read(uint16_t n);
	void Write(uint16_t n, uint8_t data);
};

