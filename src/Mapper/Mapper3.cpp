#include "Mapper3.h"

Mapper3::Mapper3(PPU& ppu) : Mapper(ppu)
{

};

uint8_t Mapper3::Read(uint16_t n)
{
	if (prgPages == 0) {
		Log->Error("Mapper3: No PRG rom loaded");
		return 0;
	}
	return prg[n % (prgPages*prgSize)];
}

void Mapper3::Write(uint16_t n, uint8_t data)
{
	uint8_t chrReg = data % chrPages;
	memcpy(ppu.memory, &chr[chrReg * chrSize], chrSize);
}