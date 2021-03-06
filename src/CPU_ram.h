#pragma once
#include "headers.h"
#include <vector>
#include "PPU.h"
#include "APU.h"
#include <vector>

#include "Mapper\Mapper.h"

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
class DlgRAM;

class CPU_ram
{
	friend DlgRAM;

	uint8_t buttonState, buttonBit;
	uint8_t memory[2048];
	bool memoryLock[2048];
	std::vector<uint8_t> SRAM;

public:
	Mapper *mapper;
	PPU* ppu;
	APU* apu;

	CPU_ram(void);
	~CPU_ram(void);

	void Write(uint16_t n, uint8_t data);
	uint8_t operator[](size_t n);
	void setInput(int keys) { buttonState = keys; }
};
