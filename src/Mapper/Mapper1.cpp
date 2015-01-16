#include "Mapper1.h"

Mapper1::Mapper1(PPU& ppu) : Mapper(ppu)
{
	counter = 0, reg = 0;
	slotSelect = true;
	prgMode = true;
	chrMode = false;
	chrReg0 = chrReg1 = prgReg = 0;
};

uint8_t Mapper1::Read(uint16_t n)
{
	if (prgPages == 0) {
		Log->Error("Mapper1: No PRG rom loaded");
		return 0;
	}
	if (!prgMode) return prg[(prgReg * prgSize) + n]; // 32kB mode

	if (!slotSelect) {
		if (n < prgSize) return prg[n];
		else return prg[(prgReg * prgSize) + (n - prgSize)];
	}
	if (n < prgSize) return prg[prgReg * prgSize + n];
	else return prg[(0x0f % prgPages) * prgSize + (n - prgSize)];
}

void Mapper1::Write(uint16_t n, uint8_t data)
{
	MMC1_write(n, data);
}

void Mapper1::MMC1_write(uint16_t n, uint8_t data)
{
	if (data & 0x80)
	{
		slotSelect = true;
		prgMode = true;
		counter = reg = 0;
		return;
	}

	reg |= (data & 1) << 5;
	reg >>= 1;

	if (++counter < 5) return;

	int addr = n / 0x2000;
	if (addr == 0) {
		int mirroring = reg & 3;
		if (mirroring == 0) ppu.Mirroring = Mirroring::ScreenA;
		else if (mirroring == 1) ppu.Mirroring = Mirroring::ScreenB;
		else if (mirroring == 2) ppu.Mirroring = Mirroring::Vertical;
		else if (mirroring == 3) ppu.Mirroring = Mirroring::Horizontal;
		else Log->Error("MMC1: Unsupported mirroring");

		slotSelect = (reg & 0x4) ? true : false;
		prgMode =  (reg & 0x8) ? true : false;
		chrMode = (reg & 0x10) ? true : false;

		prgSize = (prgMode) ? (16 * 1024) : (32 * 1024);
		chrSize = (chrMode) ? (4 * 1024) : (8 * 1024);
	}
	if (addr == 1 && chrPages > 0) {
		chrReg0 = (reg % (chrPages * 2));// &0x1e;
		memcpy(ppu.memory, &chr[chrReg0 * 0x1000], 0x1000);
		if (!chrMode) memcpy(ppu.memory + 0x1000, &chr[(chrReg0 | 1) * 0x1000], 0x1000);
	}
	if (addr == 2 && chrPages > 0) {
		chrReg1 = reg % (chrPages * 2);
		if (chrMode) memcpy(ppu.memory + 0x1000, &chr[chrReg1 * 0x1000], 0x1000);
	}
	if (addr == 3) {
		prgReg = (reg & 0xf) % prgPages;
		//WRAM_disable = (data & 0x10) ? true : false;
	}
	counter = reg = 0;
}
