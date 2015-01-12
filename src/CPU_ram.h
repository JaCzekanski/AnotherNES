#pragma once
#include "headers.h"
#include "PPU.h"
#include "APU.h"

extern int buttonState;
/* Memory map

$FFFA - $FFFB - NMI vector
$FFFC - $FFFD - RESET vector
$FFFE - $FFFF - IRQ vector


$FFFF - $C000 - PRG-ROM upper bank
$BFFF - $8000 - PRG-ROM lower bank
-------------
$7FFF - $6000 - SRAM
-------------
$5FFF - $4020 - Expansiom ROM
-------------
$401F - $2000 - IO
-------------
$1FFF - $0800 - Mirrors $0000-$07FF
$07FF - $0200 - RAM
$01FF - $0100 - Stack
$00FF - $0000 - Zero Page

*/
class CPU_ram
{
public:
	uint8_t mapper;

	uint8_t prg_rom[2024*1024]; // 2MB
	uint8_t prg_lowpage;
	uint8_t prg_highpage;
	uint8_t prg_pages;
	uint8_t memory[0xffff]; // Not sure if this is good idea 
	bool memoryLock[2048];

	uint8_t ZERO;
	uint8_t RET;

	uint8_t bit;

	PPU* ppu;
	APU* apu;

	CPU_ram(void);
	~CPU_ram(void);
	void Load( uint16_t dst, const void* source, size_t num );

	void Write( uint16_t n, uint8_t data )
	{
		switch ((n & 0xE000) >> 13)
		{
		case 0: // 0x0000 - 0x1FFF: Zero page, stack, ram, Mirrors
			if (!memoryLock[n % 2048])
				memory[n % 2048] = data; // Mirror
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
			else if (n == 0x4016) // JOY1
			{
				bit = 7; // Strobe
				return;
			}
			else if (n == 0x4017) // JOY2 and Frame counter control
			{
				bit = 7; // Strobe
				return;
			}
			memory[n] = data;
			return;

		case 3: // 0x6000 - 0x7FFF: SRAM (SRAM not implemented, return memory)
			return;

		case 4: // 0x8000 - 0x9FFF: Low rom
		case 5: // 0xA000 - 0xBFFF
		case 6: // 0xC000 - 0xDFFF: High rom
		case 7: // 0xE000 - 0xFFFF
			// Mapper 2
			// $8000-$FFFF [PPPP PPPP]
			if (mapper == 2 || mapper == 71 || mapper == 104)
				prg_lowpage = data;
			else if (mapper == 0)
			{

			}
			else
			{
				Log->Fatal("Unsupported mapper.");
				return;
			}
			return;
		}
	}

	uint8_t & operator[](size_t n)
	{
		switch ((n & 0xE000) >> 13)
		{
		case 0: // 0x0000 - 0x1FFF: Zero page, stack, ram, Mirrors
			return memory[n & 0x7FF];
		case 1: // 0x2000 - 0x3FFF: PPU ports
			RET = ppu->Read((n - 0x2000) & 0x07);
			return RET;
		case 2: // 0x4000 - 0x5FFF: APU and IO registers
			if (n == 0x4016) // JOY1 
			{
				//A, B, Select, Start, Up, Down, Left, Right.
				if (buttonState & (1 << bit)) RET = 1; //Strobe
				else RET = 0;
				bit--;
				return RET;
			}
			return ZERO;

		case 3: // 0x6000 - 0x7FFF: SRAM (SRAM not implemented, return memory)
			return ZERO;

		case 4: // 0x8000 - 0x9FFF: Low rom
		case 5: // 0xA000 - 0xBFFF
			if (prg_pages == 1) return prg_rom[(n - 0x8000) % 0x4000]; // Return PRG-ROM
			if (mapper == 2 || mapper == 71 || mapper == 104)
			{
				n -= 0x8000;
				return prg_rom[(prg_lowpage%prg_pages) * 0x4000 + n % 0x4000];
			}
			return prg_rom[(prg_lowpage * 0x4000) + n - 0x8000]; // Return PRG-ROM

		case 6: // 0xC000 - 0xDFFF: High rom
		case 7: // 0xE000 - 0xFFFF
			if (prg_pages == 1) return prg_rom[(n - 0x8000) % 0x4000]; // Return PRG-ROM
			if (mapper == 2 || mapper == 71 || mapper == 104)
			{
				n -= 0x8000;
				return prg_rom[(prg_highpage%prg_pages) * 0x4000 + (n - 0x4000)];
			}
			return prg_rom[(prg_lowpage * 0x4000) + n - 0x8000]; // Return PRG-ROM
		}
	}

};
