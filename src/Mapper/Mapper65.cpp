#include "Mapper65.h"

Mapper65::Mapper65(PPU& ppu) : Mapper(ppu)
{
	irqEnabled = false;
	prgReg[0] = 0x00 % (0x08 * 2);
	prgReg[1] = 0x01 % (0x08 * 2);
	prgReg[2] = 0xFE % (0x08*2);

	for (int i = 0; i < 8; i++) chrReg[i] = 0;
};

uint8_t Mapper65::Read(uint16_t n)
{
	if (prgPages == 0) {
		Log->Error("Mapper65: No PRG rom loaded");
		return 0;
	}
	int page = (n / 0x2000);
	if ( page == 3 ) return prg[(prgPages * 2 - 1)*prgSize + (n % 2000)];

	return prg[(prgReg[page] % (prgPages * 2))*prgSize + (n % 2000)];
}

void Mapper65::Write(uint16_t n, uint8_t data)
{
	static uint8_t reloadHigh = 0, reloadLow = 0;
	
	n += 0x8000;
	if (n == 0x8000) prgReg[0] = data%(prgPages*2);
	else if (n == 0xA000) prgReg[1] = data % (prgPages * 2);
	else if (n == 0xC000) prgReg[2] = data % (prgPages * 2);
	else if (n >= 0xB000 && n <= 0xB007) {
		chrReg[n - 0xB000] = data % (chrPages * 8);
		chrCopy();
	}
	else if (n == 0x9001) ppu.Mirroring = (data & 0x80) ? Mirroring::Horizontal : Mirroring::Vertical;
	else if (n == 0x9003) irqEnabled = (data & 0x80) ? true : false;
	else if (n == 0x9004) irqCounter = (reloadHigh<<8) | reloadLow;
	else if (n == 0x9005) reloadHigh = data; // High 8 bits of IRQ reload value
	else if (n == 0x9006) reloadLow = data;  // Low 8 bits of IRQ reload value
	else Log->Debug("Mapper65 other reg write: %x - %x", n, data);
}

void Mapper65::chrCopy()
{
/*  $0000  $0400  $0800  $0C00  $1000  $1400  $1800  $1C00
	+------+------+------+------+------+------+------+------+
	| chr0 | chr1 | chr2 | chr3 | chr4 | chr5 | chr6 | chr7 | 
	+------+------+------+------+------+------+------+------+
*/
	for (int i = 0; i < 8; i++)
		memcpy(ppu.memory + i * chrSize, &chr[chrReg[i] * chrSize], chrSize);

}