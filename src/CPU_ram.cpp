#include "CPU_ram.h"

CPU_ram::CPU_ram(void)
{
	mapper = 0;
	prg_highpage = 0;
	prg_lowpage = 0;
	prg_pages = 0;
	memset( this->memory, 0, 0xffff );
	this->memory[0x0008] = 0xf7;
	this->memory[0x0009] = 0xef;
	this->memory[0x000a] = 0xdf;
	this->memory[0x000f] = 0xbf;

	memset(this->memoryLock, 0, 2048);

	SRAM.resize(0x2000);

	for (int i = 0; i < 8; i++) MMC3_reg[i] = 0;
	
	ppu = NULL;
	Log->Debug("CPU_ram: created");
}

CPU_ram::~CPU_ram(void)
{
	Log->Debug("CPU_ram: destroyed");
}
void CPU_ram::MMC3_chrCopy()
{
	/* $0000  $0400 $0800  $0C00 $1000  $1400  $1800  $1C00
	+------------+------------+------+------+------+------+
	|     R0     |     R1     |  R2  |  R3  |  R4  |  R5  |  CHR_mode = 0
	+------------+------------+------+------+------+------+
	|  R2  |  R3 |  R4  | R5  |     R0      |     R1      |  CHR_mode = 1
	+------+-----+------+-----+-------------+-------------+
	*/
	if (!CHR_mode) {
		memcpy(ppu->memory + 0x0000, &ppu->CHR_ROM[(MMC3_reg[0] & 0xfe) * 0x400], 0x800);
		memcpy(ppu->memory + 0x0800, &ppu->CHR_ROM[(MMC3_reg[1] & 0xfe) * 0x400], 0x800);

		memcpy(ppu->memory + 0x1000, &ppu->CHR_ROM[MMC3_reg[2] * 0x400], 0x400);
		memcpy(ppu->memory + 0x1400, &ppu->CHR_ROM[MMC3_reg[3] * 0x400], 0x400);
		memcpy(ppu->memory + 0x1800, &ppu->CHR_ROM[MMC3_reg[4] * 0x400], 0x400);
		memcpy(ppu->memory + 0x1c00, &ppu->CHR_ROM[MMC3_reg[5] * 0x400], 0x400);
	}
	else {
		memcpy(ppu->memory + 0x0000, &ppu->CHR_ROM[MMC3_reg[2] * 0x400], 0x400);
		memcpy(ppu->memory + 0x0400, &ppu->CHR_ROM[MMC3_reg[3] * 0x400], 0x400);
		memcpy(ppu->memory + 0x0800, &ppu->CHR_ROM[MMC3_reg[4] * 0x400], 0x400);
		memcpy(ppu->memory + 0x0c00, &ppu->CHR_ROM[MMC3_reg[5] * 0x400], 0x400);

		memcpy(ppu->memory + 0x1000, &ppu->CHR_ROM[(MMC3_reg[0] & 0xfe) * 0x400], 0x800);
		memcpy(ppu->memory + 0x1800, &ppu->CHR_ROM[(MMC3_reg[1] & 0xfe) * 0x400], 0x800);
	}
}
void CPU_ram::MMC3_write(uint16_t n, uint8_t data)
{
	static int reg = 0;
	if (n == 0x8000) {
		PRG_mode = (data & 0x40) ? true : false;
		CHR_mode = (data & 0x80) ? true : false;

		reg = data & 0x7;
	}
	else if (n == 0x8001) {
		if (reg > 5) {
			MMC3_reg[reg] = data % (prg_pages * 2); // PRG ROM
			return;
		}

		if (chr_pages == 0) return;
		MMC3_reg[reg] = data % (chr_pages * 8);
		MMC3_chrCopy();
	}
	else if (n == 0xA000) {
		if (data & 1 == 0) ppu->Mirroring = VERTICAL;
		else ppu->Mirroring = HORIZONTAL;
	}
	else if (n == 0xA001) return; // Enable WRAM / WP
	else if (n == 0xC000) MMC3_irqCounter = data; // IRQ Reload value
	else if (n == 0xC001) return; // IRQ Clear
	else if (n == 0xE000) MMC3_irqEnabled = false; // IRQ Acknowledge / Disable
	else if (n == 0xE001) MMC3_irqEnabled = true; // IRQ Enable
	else Log->Debug("MMC3 other reg write: %x", n);
}

void CPU_ram::Load( uint16_t dst, const void* source, size_t num )
{
	uint8_t* _source = (uint8_t*)source;
	if (dst+num>0xffff)
	{
		Log->Fatal("CPU_ram: dst+num>0xffff!");
		return;
	}
	for (uint16_t i = dst; i<dst+num; i++)
	{
		memory[i] = *_source++;
	}
	Log->Debug("CPU_ram: loaded");
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
		mapper_->Write(n-0x8000, data);
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

	case 4:
	case 5:
	case 6:
	case 7:
		return mapper_->Read(n-0x8000);
	//case 4: // 0x8000 - 0x9FFF: Low rom
	//	if (mapper == 4) // MMC3
	//	{
	//		if (!PRG_mode) return prg_rom[8192 * MMC3_reg[6] + (n - 0x8000)];
	//		else return prg_rom[8192 * ((2 * prg_pages) - 2) + (n - 0x8000)];
	//	}
	//case 5: // 0xA000 - 0xBFFF
	//	if (prg_pages == 1) return prg_rom[(n - 0x8000) % 0x4000]; // Return PRG-ROM
	//	if (mapper == 2 || mapper == 71 || mapper == 104)
	//	{
	//		n -= 0x8000;
	//		return prg_rom[(prg_lowpage%prg_pages) * 0x4000 + n % 0x4000];
	//	}
	//	else if (mapper == 1) // MMC1
	//	{
	//		if (PRG_mode == 1)
	//		{
	//			if (slotSelect) return prg_rom[16 * 1024 * (PRG_reg%prg_pages) + (n - 0x8000)];
	//			else return prg_rom[16 * 1024 * 0x00 + (n - 0x8000)];
	//		}
	//		else return prg_rom[32 * 1024 * (PRG_reg%prg_pages) + (n - 0x8000)];
	//	}
	//	else if (mapper == 4) return prg_rom[8192 * MMC3_reg[7] + (n - 0xA000)]; // MMC3

	//	return prg_rom[(prg_lowpage * 0x4000) + n - 0x8000]; // Return PRG-ROM

	//case 6: // 0xC000 - 0xDFFF: High rom
	//	if (mapper == 4) // MMC3
	//	{
	//		if (!PRG_mode) return prg_rom[8192 * ((2 * prg_pages) - 2) + (n - 0xC000)];
	//		else return prg_rom[8192 * MMC3_reg[6] + (n - 0xC000)];
	//	}
	//case 7: // 0xE000 - 0xFFFF
	//	if (prg_pages == 1) return prg_rom[(n - 0x8000) % 0x4000]; // Return PRG-ROM
	//	if (mapper == 2 || mapper == 71 || mapper == 104)
	//	{
	//		n -= 0x8000;
	//		return prg_rom[(prg_highpage%prg_pages) * 0x4000 + (n - 0x4000)];
	//	}
	//	else if (mapper == 1) // MMC1
	//	{
	//		if (PRG_mode == 1)
	//		{
	//			if (slotSelect) return prg_rom[16 * 1024 * (0x0f % prg_pages) + (n - 0xc000)];
	//			else return prg_rom[16 * 1024 * (PRG_reg%prg_pages) + (n - 0xC000)];
	//		}
	//		else return prg_rom[32 * 1024 * (PRG_reg%prg_pages) + (n - 0xC000)];
	//	}
	//	else if (mapper == 4) return prg_rom[8192 * ((2 * prg_pages) - 1) + (n - 0xE000)]; // MMC3

	//	return prg_rom[(prg_lowpage * 0x4000) + n - 0x8000]; // Return PRG-ROM
	}
	return 0;
}