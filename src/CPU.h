#pragma once
#include "headers.h"
#include "CPU_ram.h"
#include "PPU.h"
#include "APU.h"

#include <vector>
using namespace std;

// Flags
#define CARRY_FLAG     (1<<0)
#define ZERO_FLAG      (1<<1)
#define INTERRUPT_FLAG (1<<2) // 1 - disabled
#define DECIMAL_FLAG   (1<<3)
#define BREAK_FLAG     (1<<4)
#define UNKNOWN_FLAG   (1<<5)
#define OVERFLOW_FLAG  (1<<6)
#define NEGATIVE_FLAG  (1<<7)


// Addresing modes
enum Address {
	Implicit,
	Accumulator,
	Immediate,
	Zero_page,
	Zero_page_x,
	Zero_page_y,
	Relative,
	Absolute,
	Absolute_x,
	Absolute_y,
	Indirect,
	Indexed_indirect,
	Indirect_indexed
};


//$FFFA - $FFFB - NMI vector
//$FFFC - $FFFD - RESET vector
//$FFFE - $FFFF - IRQ vector

/* CPU is little endian
example:
$FFFC - $FFFD - RESET vector
$FFF0: 19 19 1A 1A 1C 1D 1D 1E 1E 1F 82 80 00 80 F0 FF

vector has two bytes: 00 80
$FFFC: 00 - lower half  = a
$FFFD: 80 - higher half = b
output = (b<<8)|a
*/


class CPU
{
public:
	uint8_t A;  // Accumulator
	uint8_t X;  // Index register X
	uint8_t Y;  // Index register Y
	uint8_t P;  // Processor Status
	uint16_t PC;// Program counter
	uint8_t SP; // Stack pointer

	bool jammed;
public:
	CPU_ram memory;
	PPU ppu;
	APU apu;

	CPU(void);
	~CPU(void);

	bool isJammed() { return jammed; }
	
	void Push( uint8_t v );
	void Push16( uint16_t v );
	uint8_t Pop( );
	uint16_t Pop16( );

	void Power();
	void Reset(); // as somebody pressed reset

	void NMI();
	void IRQ();

	void Load( uint8_t* rom, uint16_t size ); // Load ROM into program space
	virtual int Step()=0; // One step of execution
};