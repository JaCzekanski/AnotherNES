#include "Mapper71.h"

Mapper71::Mapper71(PPU& ppu) : Mapper(ppu)
{
	prgReg = 0;
};

uint8_t Mapper71::Read(uint16_t n)
{
	if (prgPages == 0) {
		Log->Error("Mapper71: No PRG rom loaded");
		return 0;
	}
	if (n >= 0x4000) return prg[((prgPages - 1) * prgSize) + (n-0x4000)];
	return  prg[prgReg * prgSize + n];
}

void Mapper71::Write(uint16_t n, uint8_t data)
{
	//Log->Debug("n: %x", n+0x8000);
	// Only in Firehawk
	/*if (n < 0x4000) ppu.Mirroring = (data & 0x10) ? Mirroring::ScreenB : Mirroring::ScreenA;
	else */if (n >= 0x4000) prgReg = data % prgPages;
}