#pragma once
#include "headers.h"
#include "PPU.h"
#include "APU.h"
#include <vector>

#include "Mapper\Mapper.h"

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
	uint8_t bit;

	uint8_t PRG_mode, CHR_mode;

	uint8_t MMC3_reg[8];
	void MMC3_write(uint16_t n, uint8_t data);
	void MMC3_chrCopy();

public:
	Mapper *mapper_;
	uint8_t MMC3_irqCounter = 0;
	bool MMC3_irqEnabled = false;

	uint8_t mapper;

	uint8_t prg_rom[2024*1024]; // 2MB
	uint8_t prg_lowpage, prg_highpage;
	uint8_t prg_pages;
	uint8_t chr_pages;
	uint8_t memory[0xffff]; // Not sure if this is good idea 
	bool memoryLock[2048];
	std::vector<uint8_t> SRAM;

	PPU* ppu;
	APU* apu;

	CPU_ram(void);
	~CPU_ram(void);
	void Load( uint16_t dst, const void* source, size_t num );

	void Write(uint16_t n, uint8_t data);
	uint8_t operator[](size_t n);
};
