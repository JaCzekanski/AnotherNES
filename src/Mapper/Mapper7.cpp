#include "Mapper7.h"

Mapper7::Mapper7(PPU& ppu) : Mapper(ppu)
{
	prgReg = 0;
};

uint8_t Mapper7::Read(uint16_t n)
{
	if (prgPages == 0) {
		Log->Error("Mapper7: No PRG rom loaded");
		return 0;
	}
	return prg[prgReg * prgSize + n];
}

void Mapper7::Write(uint16_t n, uint8_t data)
{
	prgReg = (data&0x7) % (prgPages/2);
	ppu.Mirroring = (data & 0x10) ? Mirroring::ScreenB : Mirroring::ScreenA;
}