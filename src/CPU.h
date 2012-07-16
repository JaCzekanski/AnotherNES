#pragma once
#include "headers.h"
#include "CPU_ram.h"
#include "PPU.h"

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

	CPU_ram memory;
	PPU ppu;
	uint32_t cycles;

	bool PCchanged; // pc+opcode?
	uint32_t virtaddr; // Seen by 6502
	uint8_t* realaddr; // Seen by computer

	CPU(void);
	~CPU(void);

	uint8_t Read( uint16_t addr )
	{
		return this->memory[addr];
	}
	void Write( uint16_t addr, uint8_t data )
	{
		this->memory.Write( addr, data );
	}

	uint16_t Getv( )
	{
		PCchanged = true;
		return this->virtaddr;
	}

	uint8_t Readv(  )
	{
		if (virtaddr > 0xffff) return this->A;
		else return this->memory[virtaddr];
	}
	void Writev( uint8_t data )
	{
		if (virtaddr >0xffff) this->A = data;
		else this->memory.Write( virtaddr, data );
	}


	void RESET(); // as somebody pressed reset
	void NMI();
	void IRQ();

	void Push( uint8_t v );
	void Push16( uint16_t v );
	uint8_t Pop( );
	uint16_t Pop16( );

	void CARRY( uint8_t s )
	{
		if (s) this->P |= CARRY_FLAG;
		else this->P &= ~CARRY_FLAG;
	}
	void ZERO( uint8_t s )
	{
		if (s) this->P |= ZERO_FLAG;
		else this->P &= ~ZERO_FLAG;
	}
	void INTERRUPT( uint8_t s )
	{
		if (s) this->P |= INTERRUPT_FLAG;
		else this->P &= ~INTERRUPT_FLAG;
	}
	void DECIMAL( uint8_t s )
	{
		if (s) this->P |= DECIMAL_FLAG;
		else this->P &= ~DECIMAL_FLAG;
	}
	void BREAK( uint8_t s )
	{
		if (s) this->P |= BREAK_FLAG;
		else this->P &= ~BREAK_FLAG;
	}
	void OVERFLOW( uint8_t s )
	{
		if (s) this->P |= OVERFLOW_FLAG;
		else this->P &= ~OVERFLOW_FLAG;
	}
	void NEGATIVE( uint8_t s )
	{
		if (s) this->P |= NEGATIVE_FLAG;
		else this->P &= ~NEGATIVE_FLAG;
	}

	void Reset(); // Resets all registers and prepares for execution
	void Load( uint8_t* rom, uint16_t size ); // Load ROM into program space
	void Step(); // One step of execution

	// Load/Store Operations
	static void LDA( CPU* c ); // Load Accumulator
	static void LDX( CPU* c ); // Load X
	static void LDY( CPU* c ); // Load Y
	static void STA( CPU* c ); // Store Accumulator
	static void STX( CPU* c ); // Store X
	static void STY( CPU* c ); // Store Y

	// Register Transfers
	static void TAX( CPU* c ); // Transfer accumulator to X
	static void TAY( CPU* c ); // Transfer accumulator to Y
	static void TXA( CPU* c ); // Transfer X to accumulator
	static void TYA( CPU* c ); // Transfer y to accumulator

	// Stack Operations
	static void TSX( CPU* c ); // Transfer stack pointer to X
	static void TXS( CPU* c ); // Transfer X to stack pointer
	static void PHA( CPU* c ); // Push accumulator on stack
	static void PHP( CPU* c ); // Push processor status on stack
	static void PLA( CPU* c ); // Pull accumulator from stack
	static void PLP( CPU* c ); // Pull processor status from stack

	// Logical
	static void AND( CPU* c ); // Add with Carry
	static void EOR( CPU* c ); // Exclusive OR
	static void ORA( CPU* c ); // Logical Inclusive OR
	static void BIT( CPU* c ); // Bit Test

	// Arithmetic
	static void ADC( CPU* c ); // Add with Carry
	static void SBC( CPU* c ); // Subtract with Carry
	static void CMP( CPU* c ); // Compare accumulator
	static void CPX( CPU* c ); // Compare X register
	static void CPY( CPU* c ); // Compare Y register
	
	// Increments & Decrements
	static void INC( CPU* c ); // Increment a memory location
	static void INX( CPU* c ); // Increment the X register
	static void INY( CPU* c ); // Increment the Y register
	static void DEC( CPU* c ); // Decrement a memory location
	static void DEX( CPU* c ); // Decrement the X register
	static void DEY( CPU* c ); // Decrement the Y register

	// Shifts
	static void ASL( CPU* c ); // Arithmetic Shift Left
	static void LSR( CPU* c ); // Logical Shift Right
	static void ROL( CPU* c ); // Rotate Left
	static void ROR( CPU* c ); // Rotate Right

	// Jumps & Calls
	static void JMP( CPU* c ); // Jump to another location
	static void JSR( CPU* c ); // Jump to a subroutine
	static void RTS( CPU* c ); // Return form subroutine

	// Branches
	static void BCC( CPU* c ); // Branch if carry flag clear	 
	static void BCS( CPU* c ); // Branch if carry flag set	 
	static void BEQ( CPU* c ); // Branch if zero flag set	 
	static void BMI( CPU* c ); // Branch if negative flag set	 
	static void BNE( CPU* c ); // Branch if zero flag clear	 
	static void BPL( CPU* c ); // Branch if negative flag clear	 
	static void BVC( CPU* c ); // Branch if overflow flag clear	 
	static void BVS( CPU* c ); // Branch if overflow flag set

	// Status Flag Changes
	static void CLC( CPU* c ); // Clear carry flag
	static void CLD( CPU* c ); // Clear decimal mode flag
	static void CLI( CPU* c ); // Clear interrupt disable flag
	static void CLV( CPU* c ); // Clear overflow flag
	static void SEC( CPU* c ); // Set carry flag
	static void SED( CPU* c ); // Set decimal mode flag
	static void SEI( CPU* c ); // Set interrupt disable flag

	// System Function
	static void BRK( CPU* c ); // Force an interrupt
	static void NOP( CPU* c ); // No Operation
	static void RTI( CPU* c ); // Return from Interrupt

	// Undocumented opcodes
	static void LAX( CPU* c ); // Load A and X
	static void SAX( CPU* c ); // AND X register with accumulator and store result in memory
	static void DCP( CPU* c ); // Substract 1 from memory (without borrow)
	static void ISB( CPU* c ); // Increase memory by one, then subtract memory from A (with borrow)
	static void SLO( CPU* c ); // Shift left one bit in memory, then OR accumulator with memory
	static void RLA( CPU* c ); // Rotate one bit left in memory, then AND accumulator with memory
	static void SRE( CPU* c ); // Shift right one bit in memory, then EOR accumulator with memory
	static void RRA( CPU* c ); // Rotate one bit right in memory, then add memory to accumulator


	static void UNK(CPU* c); //DBG PURPOSES
};

typedef void (*_Instruction) (CPU*);
struct OPCODE
{
	int number;
	_Instruction inst;
	char* mnemnic;
	uint8_t cycles;
	Address address;
};

static OPCODE OpcodeTable[] = 
{
	// Load/Store Operations

	// Load Accumulator
	{0xa9, CPU::LDA, "LDA", 2, Immediate},
	{0xa5, CPU::LDA, "LDA", 3, Zero_page},
	{0xb5, CPU::LDA, "LDA", 4, Zero_page_x},
	{0xad, CPU::LDA, "LDA", 4, Absolute},
	{0xbd, CPU::LDA, "LDA", 4, Absolute_x},
	{0xb9, CPU::LDA, "LDA", 4, Absolute_y},
	{0xa1, CPU::LDA, "LDA", 6, Indexed_indirect},
	{0xb1, CPU::LDA, "LDA", 5, Indirect_indexed},

	// Load X
	{0xa2, CPU::LDX, "LDX", 2, Immediate},
	{0xa6, CPU::LDX, "LDX", 3, Zero_page},
	{0xb6, CPU::LDX, "LDX", 4, Zero_page_y},
	{0xae, CPU::LDX, "LDX", 4, Absolute},
	{0xbe, CPU::LDX, "LDX", 4, Absolute_y},

	// Load Y
	{0xa0, CPU::LDY, "LDY", 2, Immediate},
	{0xa4, CPU::LDY, "LDY", 3, Zero_page},
	{0xb4, CPU::LDY, "LDY", 4, Zero_page_x},
	{0xac, CPU::LDY, "LDY", 4, Absolute},
	{0xbc, CPU::LDY, "LDY", 4, Absolute_x},

	// Store Accumulator
	{0x85, CPU::STA, "STA", 3, Zero_page},
	{0x95, CPU::STA, "STA", 4, Zero_page_x},
	{0x8d, CPU::STA, "STA", 4, Absolute},
	{0x9d, CPU::STA, "STA", 5, Absolute_x},
	{0x99, CPU::STA, "STA", 5, Absolute_y},
	{0x81, CPU::STA, "STA", 6, Indexed_indirect},
	{0x91, CPU::STA, "STA", 6, Indirect_indexed},

	// Store X
	{0x86, CPU::STX, "STX", 3, Zero_page},
	{0x96, CPU::STX, "STX", 4, Zero_page_y},
	{0x8e, CPU::STX, "STX", 4, Absolute},

	// Store Y
	{0x84, CPU::STY, "STY", 3, Zero_page},
	{0x94, CPU::STY, "STY", 4, Zero_page_x},
	{0x8c, CPU::STY, "STY", 4, Absolute},

	
	// Register Transfers
	{0xAA, CPU::TAX, "TAX", 2, Implicit}, // Transfer accumulator to X
	{0xA8, CPU::TAY, "TAY", 2, Implicit}, // Transfer accumulator to Y
	{0x8A, CPU::TXA, "TXA", 2, Implicit}, // Transfer X to accumulator
	{0x98, CPU::TYA, "TYA", 2, Implicit}, // Transfer y to accumulator


	// Stack Operations
	{0xBA, CPU::TSX, "TSX", 2, Implicit}, // Transfer stack pointer to X
	{0x9A, CPU::TXS, "TXS", 2, Implicit}, // Transfer X to stack pointer
	{0x48, CPU::PHA, "PHA", 3, Implicit}, // Push accumulator on stack
	{0x08, CPU::PHP, "PHP", 3, Implicit}, // Push processor status on stack
	{0x68, CPU::PLA, "PLA", 4, Implicit}, // Pull accumulator from stack
	{0x28, CPU::PLP, "PLP", 4, Implicit}, // Pull processor status from stack

	// Logical
	// Logical AND
	{0x29, CPU::AND, "AND", 2, Immediate},
	{0x25, CPU::AND, "AND", 3, Zero_page},
	{0x35, CPU::AND, "AND", 4, Zero_page_x},
	{0x2d, CPU::AND, "AND", 4, Absolute},
	{0x3d, CPU::AND, "AND", 4, Absolute_x},
	{0x39, CPU::AND, "AND", 4, Absolute_y},
	{0x21, CPU::AND, "AND", 6, Indexed_indirect},
	{0x31, CPU::AND, "AND", 5, Indirect_indexed},

	// Logical OR
	{0x49, CPU::EOR, "EOR", 2, Immediate},
	{0x45, CPU::EOR, "EOR", 3, Zero_page},
	{0x55, CPU::EOR, "EOR", 4, Zero_page_x},
	{0x4d, CPU::EOR, "EOR", 4, Absolute},
	{0x5d, CPU::EOR, "EOR", 4, Absolute_x},
	{0x59, CPU::EOR, "EOR", 4, Absolute_y},
	{0x41, CPU::EOR, "EOR", 6, Indexed_indirect},
	{0x51, CPU::EOR, "EOR", 5, Indirect_indexed},

	// Logical Inclusive OR
	{0x09, CPU::ORA, "ORA", 2, Immediate},
	{0x05, CPU::ORA, "ORA", 3, Zero_page},
	{0x15, CPU::ORA, "ORA", 4, Zero_page_x},
	{0x0d, CPU::ORA, "ORA", 4, Absolute},
	{0x1d, CPU::ORA, "ORA", 4, Absolute_x},
	{0x19, CPU::ORA, "ORA", 4, Absolute_y},
	{0x01, CPU::ORA, "ORA", 6, Indexed_indirect},
	{0x11, CPU::ORA, "ORA", 5, Indirect_indexed},

	// Bit Test
	{0x24, CPU::BIT, "BIT", 3, Zero_page},
	{0x2c, CPU::BIT, "BIT", 4, Absolute},


	// Arithmetic

	// Add with Carry
	{0x69, CPU::ADC, "ADC", 2, Immediate},
	{0x65, CPU::ADC, "ADC", 3, Zero_page},
	{0x75, CPU::ADC, "ADC", 4, Zero_page_x},
	{0x6d, CPU::ADC, "ADC", 4, Absolute},
	{0x7d, CPU::ADC, "ADC", 4, Absolute_x},
	{0x79, CPU::ADC, "ADC", 4, Absolute_y},
	{0x61, CPU::ADC, "ADC", 6, Indexed_indirect},
	{0x71, CPU::ADC, "ADC", 5, Indirect_indexed},

	// Subtract with Carry
	{0xE9, CPU::SBC, "SBC", 2, Immediate},
	{0xE5, CPU::SBC, "SBC", 3, Zero_page},
	{0xF5, CPU::SBC, "SBC", 4, Zero_page_x},
	{0xEd, CPU::SBC, "SBC", 4, Absolute},
	{0xFd, CPU::SBC, "SBC", 4, Absolute_x},
	{0xF9, CPU::SBC, "SBC", 4, Absolute_y},
	{0xE1, CPU::SBC, "SBC", 6, Indexed_indirect},
	{0xF1, CPU::SBC, "SBC", 5, Indirect_indexed},

	//  Compare accumulator
	{0xC9, CPU::CMP, "CMP", 2, Immediate},
	{0xC5, CPU::CMP, "CMP", 3, Zero_page},
	{0xD5, CPU::CMP, "CMP", 4, Zero_page_x},
	{0xCD, CPU::CMP, "CMP", 4, Absolute},
	{0xDD, CPU::CMP, "CMP", 4, Absolute_x},
	{0xD9, CPU::CMP, "CMP", 4, Absolute_y},
	{0xC1, CPU::CMP, "CMP", 6, Indexed_indirect},
	{0xD1, CPU::CMP, "CMP", 5, Indirect_indexed},

	// Compare X register
	{0xE0, CPU::CPX, "CPX", 2, Immediate},
	{0xE4, CPU::CPX, "CPX", 3, Zero_page},
	{0xEC, CPU::CPX, "CPX", 4, Absolute},
	
	// Compare Y register
	{0xC0, CPU::CPY, "CPY", 2, Immediate},
	{0xC4, CPU::CPY, "CPY", 3, Zero_page},
	{0xCC, CPU::CPY, "CPY", 4, Absolute},

	
	// Increments & Decrements
	// Increment a memory location
	{0xE6, CPU::INC, "INC", 5, Zero_page},
	{0xF6, CPU::INC, "INC", 6, Zero_page_x},
	{0xEE, CPU::INC, "INC", 6, Absolute},
	{0xFE, CPU::INC, "INC", 7, Absolute_x},

	// Increment the X register
	{0xE8, CPU::INX, "INX", 2, Implicit},
	
	// Increment the Y register
	{0xC8, CPU::INY, "INY", 2, Implicit},

	// Decrement a memory location
	{0xC6, CPU::DEC, "DEC", 5, Zero_page},
	{0xD6, CPU::DEC, "DEC", 6, Zero_page_x},
	{0xCE, CPU::DEC, "DEC", 6, Absolute},
	{0xDE, CPU::DEC, "DEC", 7, Absolute_x},

	// Decrement the X register
	{0xCA, CPU::DEX, "DEX", 2, Implicit},

	// Decrement the Y register
	{0x88, CPU::DEY, "DEY", 2, Implicit},


	// Shifts
	// Arithmetic Shift Left
	{0x0A, CPU::ASL, "ASL", 2, Accumulator},
	{0x06, CPU::ASL, "ASL", 5, Zero_page},
	{0x16, CPU::ASL, "ASL", 6, Zero_page_x},
	{0x0E, CPU::ASL, "ASL", 6, Absolute},
	{0x1E, CPU::ASL, "ASL", 7, Absolute_x},

	// Logical Shift Right
	{0x4A, CPU::LSR, "LSR", 2, Accumulator},
	{0x46, CPU::LSR, "LSR", 5, Zero_page},
	{0x56, CPU::LSR, "LSR", 6, Zero_page_x},
	{0x4E, CPU::LSR, "LSR", 6, Absolute},
	{0x5E, CPU::LSR, "LSR", 7, Absolute_x},

	// Rotate Left
	{0x2A, CPU::ROL, "ROL", 2, Accumulator},
	{0x26, CPU::ROL, "ROL", 5, Zero_page},
	{0x36, CPU::ROL, "ROL", 6, Zero_page_x},
	{0x2E, CPU::ROL, "ROL", 6, Absolute},
	{0x3E, CPU::ROL, "ROL", 7, Absolute_x},

	// Rotate Right
	{0x6A, CPU::ROR, "ROR", 2, Accumulator},
	{0x66, CPU::ROR, "ROR", 5, Zero_page},
	{0x76, CPU::ROR, "ROR", 6, Zero_page_x},
	{0x6E, CPU::ROR, "ROR", 6, Absolute},
	{0x7E, CPU::ROR, "ROR", 7, Absolute_x},

	// Jumps & Calls

	// Jump to another location
	{0x4C, CPU::JMP, "JMP", 3, Absolute},
	{0x6C, CPU::JMP, "JMP", 5, Indirect},

	// Jump to a subroutine
	{0x20, CPU::JSR, "JSR", 3, Absolute},

	// Return form subroutine
	{0x60, CPU::RTS, "RTS", 6, Implicit},


	// Branches
	{0x90, CPU::BCC, "BCC", 2, Relative}, // Branch if carry flag clear
	{0xB0, CPU::BCS, "BCS", 2, Relative}, // Branch if carry flag set	 
	{0xF0, CPU::BEQ, "BEQ", 2, Relative}, // Branch if zero flag set	
	{0x30, CPU::BMI, "BMI", 2, Relative}, // Branch if negative flag set	
	{0xD0, CPU::BNE, "BNE", 2, Relative}, // Branch if zero flag clear	 
	{0x10, CPU::BPL, "BPL", 2, Relative}, // Branch if negative flag clear	 
	{0x50, CPU::BVC, "BVC", 2, Relative}, // Branch if overflow flag clear	
	{0x70, CPU::BVS, "BVS", 2, Relative}, // Branch if overflow flag set


	// Status Flag Changes
	{0x18, CPU::CLC, "CLC", 2, Implicit}, // Clear carry flag
	{0xD8, CPU::CLD, "CLD", 2, Implicit}, // Clear decimal mode flag
	{0x58, CPU::CLI, "CLI", 2, Implicit}, // Clear interrupt disable flag
	{0xB8, CPU::CLV, "CLV", 2, Implicit}, // Clear overflow flag
	{0x38, CPU::SEC, "SEC", 2, Implicit}, // Set carry flag
	{0xF8, CPU::SED, "SED", 2, Implicit}, // Set decimal mode flag
	{0x78, CPU::SEI, "SEI", 2, Implicit}, // Set interrupt disable flag

	// System Function
	{0x00, CPU::BRK, "BRK", 7, Implicit}, // Force an interrupt
	{0xEA, CPU::NOP, "NOP", 2, Implicit}, // No Operation
	{0x40, CPU::RTI, "RTI", 6, Implicit}, // Return from Interrupt

	// Undocumented opcodes
	{0x04, CPU::NOP, "NOP", 3, Zero_page},  // Double No Operation
	{0x14, CPU::NOP, "NOP", 4, Zero_page_x},// Double No Operation
	{0x34, CPU::NOP, "NOP", 4, Zero_page_x},// Double No Operation
	{0x44, CPU::NOP, "NOP", 3, Zero_page},  // Double No Operation
	{0x54, CPU::NOP, "NOP", 4, Zero_page_x},// Double No Operation
	{0x64, CPU::NOP, "NOP", 3, Zero_page},  // Double No Operation
	{0x74, CPU::NOP, "NOP", 4, Zero_page_x},// Double No Operation
	{0x80, CPU::NOP, "NOP", 2, Immediate},  // Double No Operation
	{0x82, CPU::NOP, "NOP", 2, Immediate},  // Double No Operation
	{0x89, CPU::NOP, "NOP", 2, Immediate},  // Double No Operation
	{0xC2, CPU::NOP, "NOP", 2, Immediate},  // Double No Operation
	{0xD4, CPU::NOP, "NOP", 4, Zero_page_x},// Double No Operation
	{0xE2, CPU::NOP, "NOP", 2, Immediate},  // Double No Operation
	{0xF4, CPU::NOP, "NOP", 4, Zero_page_x},// Double No Operation

	{0x1A, CPU::NOP, "NOP", 2, Implicit},   // No Operation
	{0x3A, CPU::NOP, "NOP", 2, Implicit},   // No Operation
	{0x5A, CPU::NOP, "NOP", 2, Implicit},   // No Operation
	{0x7A, CPU::NOP, "NOP", 2, Implicit},   // No Operation
	{0xDA, CPU::NOP, "NOP", 2, Implicit},   // No Operation
	{0xFA, CPU::NOP, "NOP", 2, Implicit},   // No Operation


	{0x0C, CPU::NOP, "NOP", 4, Absolute},     // Triple No Operation
	{0x1C, CPU::NOP, "NOP", 4, Absolute_x},   // Triple No Operation
	{0x3C, CPU::NOP, "NOP", 4, Absolute_x},   // Triple No Operation
	{0x5C, CPU::NOP, "NOP", 4, Absolute_x},   // Triple No Operation
	{0x7C, CPU::NOP, "NOP", 4, Absolute_x},   // Triple No Operation
	{0xDC, CPU::NOP, "NOP", 4, Absolute_x},   // Triple No Operation
	{0xFC, CPU::NOP, "NOP", 4, Absolute_x},   // Triple No Operation

	{0xEB, CPU::SBC, "SBC", 2, Immediate},    // Substract

	// Load A and X
	{0xa7, CPU::LAX, "LAX", 3, Zero_page},
	{0xb7, CPU::LAX, "LAX", 4, Zero_page_y},
	{0xaf, CPU::LAX, "LAX", 4, Absolute},
	{0xbf, CPU::LAX, "LAX", 4, Absolute_y},
	{0xa3, CPU::LAX, "LAX", 6, Indexed_indirect},
	{0xb3, CPU::LAX, "LAX", 5, Indirect_indexed},
	
	
	// AND X register with accumulator and store result in memory
	{0x87, CPU::SAX, "SAX", 3, Zero_page},
	{0x97, CPU::SAX, "SAX", 4, Zero_page_y},
	{0x83, CPU::SAX, "SAX", 6, Indexed_indirect},
	{0x8f, CPU::SAX, "SAX", 4, Absolute},

	// Substract 1 from memory (without borrow)
	{0xc7, CPU::DCP, "DCP", 5, Zero_page},
	{0xd7, CPU::DCP, "DCP", 6, Zero_page_x},
	{0xcf, CPU::DCP, "DCP", 6, Absolute},
	{0xdf, CPU::DCP, "DCP", 7, Absolute_x},
	{0xdb, CPU::DCP, "DCP", 7, Absolute_y},
	{0xc3, CPU::DCP, "DCP", 8, Indexed_indirect},
	{0xd3, CPU::DCP, "DCP", 8, Indirect_indexed},

	// Increase memory by one, then subtract memory from A (with borrow)
	{0xe7, CPU::ISB, "ISB", 5, Zero_page},
	{0xf7, CPU::ISB, "ISB", 6, Zero_page_x},
	{0xef, CPU::ISB, "ISB", 6, Absolute},
	{0xff, CPU::ISB, "ISB", 7, Absolute_x},
	{0xfb, CPU::ISB, "ISB", 7, Absolute_y},
	{0xe3, CPU::ISB, "ISB", 8, Indexed_indirect},
	{0xf3, CPU::ISB, "ISB", 8, Indirect_indexed},

	// Increase memory by one, then subtract memory from A (with borrow)
	{0x07, CPU::SLO, "SLO", 5, Zero_page},
	{0x17, CPU::SLO, "SLO", 6, Zero_page_x},
	{0x0f, CPU::SLO, "SLO", 6, Absolute},
	{0x1f, CPU::SLO, "SLO", 7, Absolute_x},
	{0x1b, CPU::SLO, "SLO", 7, Absolute_y},
	{0x03, CPU::SLO, "SLO", 8, Indexed_indirect},
	{0x13, CPU::SLO, "SLO", 8, Indirect_indexed},
	
	// Rotate one bit left in memory, then AND accumulator with memory
	{0x27, CPU::RLA, "RLA", 5, Zero_page},
	{0x37, CPU::RLA, "RLA", 6, Zero_page_x},
	{0x2f, CPU::RLA, "RLA", 6, Absolute},
	{0x3f, CPU::RLA, "RLA", 7, Absolute_x},
	{0x3b, CPU::RLA, "RLA", 7, Absolute_y},
	{0x23, CPU::RLA, "RLA", 8, Indexed_indirect},
	{0x33, CPU::RLA, "RLA", 8, Indirect_indexed},
	
	// Rotate one bit left in memory, then AND accumulator with memory
	{0x47, CPU::SRE, "SRE", 5, Zero_page},
	{0x57, CPU::SRE, "SRE", 6, Zero_page_x},
	{0x4f, CPU::SRE, "SRE", 6, Absolute},
	{0x5f, CPU::SRE, "SRE", 7, Absolute_x},
	{0x5b, CPU::SRE, "SRE", 7, Absolute_y},
	{0x43, CPU::SRE, "SRE", 8, Indexed_indirect},
	{0x53, CPU::SRE, "SRE", 8, Indirect_indexed},

	// Rotate one bit right in memory, then add memory to accumulator
	{0x67, CPU::RRA, "RRA", 5, Zero_page},
	{0x77, CPU::RRA, "RRA", 6, Zero_page_x},
	{0x6f, CPU::RRA, "RRA", 6, Absolute},
	{0x7f, CPU::RRA, "RRA", 7, Absolute_x},
	{0x7b, CPU::RRA, "RRA", 7, Absolute_y},
	{0x63, CPU::RRA, "RRA", 8, Indexed_indirect},
	{0x73, CPU::RRA, "RRA", 8, Indirect_indexed},

	{0x02, CPU::UNK, "GFOT"}

};

static OPCODE OpcodeTableOptimized[256];