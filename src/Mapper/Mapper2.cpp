#include "Mapper2.h"

Mapper2::Mapper2(PPU& ppu) : Mapper(ppu)
{
	prgReg = 0;
};

uint8_t Mapper2::Read(uint16_t n)
{
	if (prgPages == 0) {
		Log->Error("Mapper2: No PRG rom loaded");
		return 0;
	}
	if (n >= 0x4000) return prg[((prgPages - 1) * prgSize) + (n-0x4000)];
	return  prg[prgReg * prgSize + n];
}

void Mapper2::Write(uint16_t n, uint8_t data)
{
	prgReg = data % prgPages;
}