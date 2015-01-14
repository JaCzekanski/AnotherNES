#include "Mapper0.h"

Mapper0::Mapper0(PPU& ppu) : Mapper(ppu)
{

};

uint8_t Mapper0::Read(uint16_t n)
{
	if (prgPages == 0) {
		Log->Error("Mapper0: No PRG rom loaded");
		return 0;
	}
	return prg[n % (prgPages*prgSize)];
}

void Mapper0::Write(uint16_t n, uint8_t data)
{

}