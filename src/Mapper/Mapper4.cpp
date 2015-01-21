#include "Mapper4.h"

Mapper4::Mapper4(PPU& ppu) : Mapper(ppu)
{
	prgMode = false;
	chrMode = false;
	for (int i = 0; i < 8; i++) MMC3_reg[i] = 0;
};

uint8_t Mapper4::Read(uint16_t n)
{
	if (prgPages == 0) {
		Log->Error("Mapper4: No PRG rom loaded");
		return 0;
	}

	int page = 0;
	switch (n / 0x2000)
	{
	case 0: page = (!prgMode) ? MMC3_reg[6] : (prgPages*2 - 2); break;
	case 1: page = MMC3_reg[7]; break;
	case 2: page = (!prgMode) ? (prgPages * 2 - 2) : MMC3_reg[6]; break;
	case 3: page = prgPages * 2 - 1; break;
	}
	return prg[ page*prgSize + (n % 0x2000)];
}

void Mapper4::Write(uint16_t n, uint8_t data)
{
	MMC3_write(n, data);
}

void Mapper4::MMC3_chrCopy()
{
	/* $0000  $0400 $0800  $0C00 $1000  $1400  $1800  $1C00
	+------------+------------+------+------+------+------+
	|     R0     |     R1     |  R2  |  R3  |  R4  |  R5  |  CHR_mode = 0
	+------------+------------+------+------+------+------+
	|  R2  |  R3 |  R4  | R5  |     R0      |     R1      |  CHR_mode = 1
	+------+-----+------+-----+-------------+-------------+
	*/
	if (!chrMode) {
		memcpy(ppu.memory + 0x0000, &chr[(MMC3_reg[0] & 0xfe) * 0x400], 0x800);
		memcpy(ppu.memory + 0x0800, &chr[(MMC3_reg[1] & 0xfe) * 0x400], 0x800);

		memcpy(ppu.memory + 0x1000, &chr[MMC3_reg[2] * 0x400], 0x400);
		memcpy(ppu.memory + 0x1400, &chr[MMC3_reg[3] * 0x400], 0x400);
		memcpy(ppu.memory + 0x1800, &chr[MMC3_reg[4] * 0x400], 0x400);
		memcpy(ppu.memory + 0x1c00, &chr[MMC3_reg[5] * 0x400], 0x400);
	}
	else {
		memcpy(ppu.memory + 0x0000, &chr[MMC3_reg[2] * 0x400], 0x400);
		memcpy(ppu.memory + 0x0400, &chr[MMC3_reg[3] * 0x400], 0x400);
		memcpy(ppu.memory + 0x0800, &chr[MMC3_reg[4] * 0x400], 0x400);
		memcpy(ppu.memory + 0x0c00, &chr[MMC3_reg[5] * 0x400], 0x400);

		memcpy(ppu.memory + 0x1000, &chr[(MMC3_reg[0] & 0xfe) * 0x400], 0x800);
		memcpy(ppu.memory + 0x1800, &chr[(MMC3_reg[1] & 0xfe) * 0x400], 0x800);
	}
}
void Mapper4::MMC3_write(uint16_t n, uint8_t data)
{
	static int reg = 0;
	n += 0x8000;
	if (n == 0x8000) {
		prgMode = (data & 0x40) ? true : false;
		chrMode = (data & 0x80) ? true : false;

		reg = data & 0x7;
	}
	else if (n == 0x8001) {
		if (reg > 5) {
			MMC3_reg[reg] = data % (prgPages * 2); // PRG ROM
			return;
		}

		if (chrPages == 0) return;
		MMC3_reg[reg] = data % (chrPages * 8);
		MMC3_chrCopy();
	}
	else if (n == 0xA000 ) {
		if (ppu.Mirroring != Mirroring::FourScreen) {
			if ((data & 1) == 0) ppu.Mirroring = Mirroring::Vertical;
			else ppu.Mirroring = Mirroring::Horizontal;
		}
	}
	else if (n == 0xA001) return; // Enable WRAM / WP
	else if (n == 0xC000) MMC3_irqCounter = data; // IRQ Reload value
	else if (n == 0xC001) return; // IRQ Clear
	else if (n == 0xE000) MMC3_irqEnabled = false; // IRQ Acknowledge / Disable
	else if (n == 0xE001) MMC3_irqEnabled = true; // IRQ Enable
	//else Log->Debug("MMC3 other reg write: %x", n);
}
