#include "CPU_ram.h"

CPU_ram::CPU_ram(void)
{
	mapper = NULL;
	ppu = NULL;
	memset( this->memory, 0, 0xffff );
	this->memory[0x0008] = 0xf7;
	this->memory[0x0009] = 0xef;
	this->memory[0x000a] = 0xdf;
	this->memory[0x000f] = 0xbf;

	memset(this->memoryLock, 0, 2048);
	SRAM.resize(0x2000);
		
	Log->Debug("CPU_ram: created");
}

CPU_ram::~CPU_ram(void)
{
	Log->Debug("CPU_ram: destroyed");
}

void CPU_ram::Write(uint16_t n, uint8_t data)
{
	switch ((n & 0xE000) >> 13)
	{
	case 0: // 0x0000 - 0x1FFF: Zero page, stack, ram, Mirrors
		if (!memoryLock[n % 2048])	memory[n % 2048] = data; // Mirror
		return;

	case 1: // 0x2000 - 0x3FFF: PPU ports
		ppu->Write((n - 0x2000) % 8, data);
		return;

	case 2: // 0x4000 - 0x5FFF: APU and IO registers
		// 0x4000-0x4003 - Pulse 1
		// 0x4004-0x4007 - Pulse 2
		// 0x4008-0x400B - Triangle
		// 0x400C-0x400F - Noise
		// 0x4010-0x4013 - DMC
		if (n <= 0x4013 || n == 0x4015 || n == 0x4017)
		{
			apu->Write(n - 0x4000, data);
			return;
		}
		if (n == 0x4014) // OAM_DMA 
		{
			// Copy $XX00-$XXFF to OAM
			// OAMADDR

			int oamptr = ppu->OAMADDR;
			for (int i = 0; i<256; i++)
			{
				*((uint8_t *)ppu->OAM + oamptr) = memory[data << 8 | i];
				if (++oamptr>0xff) oamptr = 0;
			}
		}
		else if (n == 0x4016 || n == 0x4017) // JOY1
		{
			bit = 7; // Strobe
			return;
		}
		return;

	case 3: // 0x6000 - 0x7FFF: SRAM
		SRAM[n-0x6000] = data;
		return;

	case 4: // 0x8000 - 0x9FFF: Low rom
	case 5: // 0xA000 - 0xBFFF
	case 6: // 0xC000 - 0xDFFF: High rom
	case 7: // 0xE000 - 0xFFFF
		if (mapper) mapper->Write(n - 0x8000, data);
		return;
	}
}


uint8_t CPU_ram::operator[](size_t n)
{
	switch ((n & 0xE000) >> 13)
	{
	case 0: // 0x0000 - 0x1FFF: Zero page, stack, ram, Mirrors
		return memory[n & 0x7FF];
	case 1: // 0x2000 - 0x3FFF: PPU ports
		return ppu->Read((n - 0x2000) & 0x07);
	case 2: // 0x4000 - 0x5FFF: APU and IO registers
		if (n == 0x4016) // JOY1 
		{
			//A, B, Select, Start, Up, Down, Left, Right.
			bit--;
			if (buttonState & (1 << (bit + 1))) return 1;
			else return 0;
		}
		return 0;

	case 3: // 0x6000 - 0x7FFF:
		return SRAM[n - 0x6000];

	case 4: // 0x8000 - 0x9FFF: Low rom
	case 5: // 0xA000 - 0xBFFF
	case 6: // 0xC000 - 0xDFFF: High rom
	case 7: // 0xE000 - 0xFFFF
		if (mapper) return mapper->Read(n - 0x8000);
	}
	return 0;
}