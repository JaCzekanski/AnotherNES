#pragma once
#include "Mapper.h"

class Mapper0 : public Mapper
{
	const int prgSize = 16 * 1024;
public:
	Mapper0(PPU& ppu);
	~Mapper0();
	uint8_t Read(uint16_t n);
	void Write(uint16_t n, uint8_t data);
};

