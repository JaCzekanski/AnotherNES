#pragma once
#include "headers.h"
#include "CPU.h"
#include "CPU_ram.h"
#include "PPU.h"

#include <vector>
using namespace std;

class CPU_interpreter : public CPU
{
public:
	bool PCchanged; // pc+opcode?
	uint32_t virtaddr; // Seen by 6502
	uint8_t* realaddr; // Seen by computer

	CPU_interpreter(void);
	~CPU_interpreter(void);

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
	void OVERFLOW_( uint8_t s )
	{
		if (s) this->P |= OVERFLOW_FLAG;
		else this->P &= ~OVERFLOW_FLAG;
	}
	void NEGATIVE( uint8_t s )
	{
		if (s) this->P |= NEGATIVE_FLAG;
		else this->P &= ~NEGATIVE_FLAG;
	}

	int Step();


	// Load/Store Operations
	static void LDA( CPU_interpreter* c ); // Load Accumulator
	static void LDX( CPU_interpreter* c ); // Load X
	static void LDY( CPU_interpreter* c ); // Load Y
	static void STA( CPU_interpreter* c ); // Store Accumulator
	static void STX( CPU_interpreter* c ); // Store X
	static void STY( CPU_interpreter* c ); // Store Y

	// Register Transfers
	static void TAX( CPU_interpreter* c ); // Transfer accumulator to X
	static void TAY( CPU_interpreter* c ); // Transfer accumulator to Y
	static void TXA( CPU_interpreter* c ); // Transfer X to accumulator
	static void TYA( CPU_interpreter* c ); // Transfer y to accumulator

	// Stack Operations
	static void TSX( CPU_interpreter* c ); // Transfer stack pointer to X
	static void TXS( CPU_interpreter* c ); // Transfer X to stack pointer
	static void PHA( CPU_interpreter* c ); // Push accumulator on stack
	static void PHP( CPU_interpreter* c ); // Push processor status on stack
	static void PLA( CPU_interpreter* c ); // Pull accumulator from stack
	static void PLP( CPU_interpreter* c ); // Pull processor status from stack

	// Logical
	static void AND( CPU_interpreter* c ); // Add with Carry
	static void EOR( CPU_interpreter* c ); // Exclusive OR
	static void ORA( CPU_interpreter* c ); // Logical Inclusive OR
	static void BIT( CPU_interpreter* c ); // Bit Test

	// Arithmetic
	static void ADC( CPU_interpreter* c ); // Add with Carry
	static void SBC( CPU_interpreter* c ); // Subtract with Carry
	static void CMP( CPU_interpreter* c ); // Compare accumulator
	static void CPX( CPU_interpreter* c ); // Compare X register
	static void CPY( CPU_interpreter* c ); // Compare Y register
	
	// Increments & Decrements
	static void INC( CPU_interpreter* c ); // Increment a memory location
	static void INX( CPU_interpreter* c ); // Increment the X register
	static void INY( CPU_interpreter* c ); // Increment the Y register
	static void DEC( CPU_interpreter* c ); // Decrement a memory location
	static void DEX( CPU_interpreter* c ); // Decrement the X register
	static void DEY( CPU_interpreter* c ); // Decrement the Y register

	// Shifts
	static void ASL( CPU_interpreter* c ); // Arithmetic Shift Left
	static void LSR( CPU_interpreter* c ); // Logical Shift Right
	static void ROL( CPU_interpreter* c ); // Rotate Left
	static void ROR( CPU_interpreter* c ); // Rotate Right

	// Jumps & Calls
	static void JMP( CPU_interpreter* c ); // Jump to another location
	static void JSR( CPU_interpreter* c ); // Jump to a subroutine
	static void RTS( CPU_interpreter* c ); // Return form subroutine

	// Branches
	static void BCC( CPU_interpreter* c ); // Branch if carry flag clear	 
	static void BCS( CPU_interpreter* c ); // Branch if carry flag set	 
	static void BEQ( CPU_interpreter* c ); // Branch if zero flag set	 
	static void BMI( CPU_interpreter* c ); // Branch if negative flag set	 
	static void BNE( CPU_interpreter* c ); // Branch if zero flag clear	 
	static void BPL( CPU_interpreter* c ); // Branch if negative flag clear	 
	static void BVC( CPU_interpreter* c ); // Branch if overflow flag clear	 
	static void BVS( CPU_interpreter* c ); // Branch if overflow flag set

	// Status Flag Changes
	static void CLC( CPU_interpreter* c ); // Clear carry flag
	static void CLD( CPU_interpreter* c ); // Clear decimal mode flag
	static void CLI( CPU_interpreter* c ); // Clear interrupt disable flag
	static void CLV( CPU_interpreter* c ); // Clear overflow flag
	static void SEC( CPU_interpreter* c ); // Set carry flag
	static void SED( CPU_interpreter* c ); // Set decimal mode flag
	static void SEI( CPU_interpreter* c ); // Set interrupt disable flag

	// System Function
	static void BRK( CPU_interpreter* c ); // Force an interrupt
	static void NOP( CPU_interpreter* c ); // No Operation
	static void RTI( CPU_interpreter* c ); // Return from Interrupt

	// Undocumented opcodes
	static void LAX( CPU_interpreter* c ); // Load A and X
	static void SAX( CPU_interpreter* c ); // AND X register with accumulator and store result in memory
	static void DCP( CPU_interpreter* c ); // Substract 1 from memory (without borrow)
	static void ISB( CPU_interpreter* c ); // Increase memory by one, then subtract memory from A (with borrow)
	static void SLO( CPU_interpreter* c ); // Shift left one bit in memory, then OR accumulator with memory
	static void RLA( CPU_interpreter* c ); // Rotate one bit left in memory, then AND accumulator with memory
	static void SRE( CPU_interpreter* c ); // Shift right one bit in memory, then EOR accumulator with memory
	static void RRA( CPU_interpreter* c ); // Rotate one bit right in memory, then add memory to accumulator

	static void ANC( CPU_interpreter* c ); // AND byte with accumulator. If result is negative then carry is set.
	static void ALR( CPU_interpreter* c ); // AND byte with accumulator, then shift right one bit in accumulator.

	static void ARR( CPU_interpreter* c ); // AND byte with accumulator, then rotate one bit right in accumulator and set some flags
	static void ATX( CPU_interpreter* c ); // AND byte with accumulator, then transfer accumulator to X register.
	static void AXS( CPU_interpreter* c ); // AND X register with accumulator and store result in X register, then subtract byte from X register (without borrow).
	static void SYA( CPU_interpreter* c ); // AND Y register with the high byte of the target address of the argument + 1. Store the result in memory.
	static void SXA( CPU_interpreter* c ); // AND X register with the high byte of the target address of the argument + 1. Store the result in memory.

	static void KIL(CPU_interpreter* c); // CPU JAM

	static void UNK(CPU_interpreter* c); //DBG PURPOSES
};

typedef void (*_Instruction) (CPU_interpreter*);
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
	{0xa9, CPU_interpreter::LDA, "LDA", 2, Immediate},
	{0xa5, CPU_interpreter::LDA, "LDA", 3, Zero_page},
	{0xb5, CPU_interpreter::LDA, "LDA", 4, Zero_page_x},
	{0xad, CPU_interpreter::LDA, "LDA", 4, Absolute},
	{0xbd, CPU_interpreter::LDA, "LDA", 4, Absolute_x},
	{0xb9, CPU_interpreter::LDA, "LDA", 4, Absolute_y},
	{0xa1, CPU_interpreter::LDA, "LDA", 6, Indexed_indirect},
	{0xb1, CPU_interpreter::LDA, "LDA", 5, Indirect_indexed},

	// Load X
	{0xa2, CPU_interpreter::LDX, "LDX", 2, Immediate},
	{0xa6, CPU_interpreter::LDX, "LDX", 3, Zero_page},
	{0xb6, CPU_interpreter::LDX, "LDX", 4, Zero_page_y},
	{0xae, CPU_interpreter::LDX, "LDX", 4, Absolute},
	{0xbe, CPU_interpreter::LDX, "LDX", 4, Absolute_y},

	// Load Y
	{0xa0, CPU_interpreter::LDY, "LDY", 2, Immediate},
	{0xa4, CPU_interpreter::LDY, "LDY", 3, Zero_page},
	{0xb4, CPU_interpreter::LDY, "LDY", 4, Zero_page_x},
	{0xac, CPU_interpreter::LDY, "LDY", 4, Absolute},
	{0xbc, CPU_interpreter::LDY, "LDY", 4, Absolute_x},

	// Store Accumulator
	{0x85, CPU_interpreter::STA, "STA", 3, Zero_page},
	{0x95, CPU_interpreter::STA, "STA", 4, Zero_page_x},
	{0x8d, CPU_interpreter::STA, "STA", 4, Absolute},
	{0x9d, CPU_interpreter::STA, "STA", 5, Absolute_x},
	{0x99, CPU_interpreter::STA, "STA", 5, Absolute_y},
	{0x81, CPU_interpreter::STA, "STA", 6, Indexed_indirect},
	{0x91, CPU_interpreter::STA, "STA", 6, Indirect_indexed},

	// Store X
	{0x86, CPU_interpreter::STX, "STX", 3, Zero_page},
	{0x96, CPU_interpreter::STX, "STX", 4, Zero_page_y},
	{0x8e, CPU_interpreter::STX, "STX", 4, Absolute},

	// Store Y
	{0x84, CPU_interpreter::STY, "STY", 3, Zero_page},
	{0x94, CPU_interpreter::STY, "STY", 4, Zero_page_x},
	{0x8c, CPU_interpreter::STY, "STY", 4, Absolute},

	
	// Register Transfers
	{0xAA, CPU_interpreter::TAX, "TAX", 2, Implicit}, // Transfer accumulator to X
	{0xA8, CPU_interpreter::TAY, "TAY", 2, Implicit}, // Transfer accumulator to Y
	{0x8A, CPU_interpreter::TXA, "TXA", 2, Implicit}, // Transfer X to accumulator
	{0x98, CPU_interpreter::TYA, "TYA", 2, Implicit}, // Transfer y to accumulator


	// Stack Operations
	{0xBA, CPU_interpreter::TSX, "TSX", 2, Implicit}, // Transfer stack pointer to X
	{0x9A, CPU_interpreter::TXS, "TXS", 2, Implicit}, // Transfer X to stack pointer
	{0x48, CPU_interpreter::PHA, "PHA", 3, Implicit}, // Push accumulator on stack
	{0x08, CPU_interpreter::PHP, "PHP", 3, Implicit}, // Push processor status on stack
	{0x68, CPU_interpreter::PLA, "PLA", 4, Implicit}, // Pull accumulator from stack
	{0x28, CPU_interpreter::PLP, "PLP", 4, Implicit}, // Pull processor status from stack

	// Logical
	// Logical AND
	{0x29, CPU_interpreter::AND, "AND", 2, Immediate},
	{0x25, CPU_interpreter::AND, "AND", 3, Zero_page},
	{0x35, CPU_interpreter::AND, "AND", 4, Zero_page_x},
	{0x2d, CPU_interpreter::AND, "AND", 4, Absolute},
	{0x3d, CPU_interpreter::AND, "AND", 4, Absolute_x},
	{0x39, CPU_interpreter::AND, "AND", 4, Absolute_y},
	{0x21, CPU_interpreter::AND, "AND", 6, Indexed_indirect},
	{0x31, CPU_interpreter::AND, "AND", 5, Indirect_indexed},

	// Logical OR
	{0x49, CPU_interpreter::EOR, "EOR", 2, Immediate},
	{0x45, CPU_interpreter::EOR, "EOR", 3, Zero_page},
	{0x55, CPU_interpreter::EOR, "EOR", 4, Zero_page_x},
	{0x4d, CPU_interpreter::EOR, "EOR", 4, Absolute},
	{0x5d, CPU_interpreter::EOR, "EOR", 4, Absolute_x},
	{0x59, CPU_interpreter::EOR, "EOR", 4, Absolute_y},
	{0x41, CPU_interpreter::EOR, "EOR", 6, Indexed_indirect},
	{0x51, CPU_interpreter::EOR, "EOR", 5, Indirect_indexed},

	// Logical Inclusive OR
	{0x09, CPU_interpreter::ORA, "ORA", 2, Immediate},
	{0x05, CPU_interpreter::ORA, "ORA", 3, Zero_page},
	{0x15, CPU_interpreter::ORA, "ORA", 4, Zero_page_x},
	{0x0d, CPU_interpreter::ORA, "ORA", 4, Absolute},
	{0x1d, CPU_interpreter::ORA, "ORA", 4, Absolute_x},
	{0x19, CPU_interpreter::ORA, "ORA", 4, Absolute_y},
	{0x01, CPU_interpreter::ORA, "ORA", 6, Indexed_indirect},
	{0x11, CPU_interpreter::ORA, "ORA", 5, Indirect_indexed},

	// Bit Test
	{0x24, CPU_interpreter::BIT, "BIT", 3, Zero_page},
	{0x2c, CPU_interpreter::BIT, "BIT", 4, Absolute},


	// Arithmetic

	// Add with Carry
	{0x69, CPU_interpreter::ADC, "ADC", 2, Immediate},
	{0x65, CPU_interpreter::ADC, "ADC", 3, Zero_page},
	{0x75, CPU_interpreter::ADC, "ADC", 4, Zero_page_x},
	{0x6d, CPU_interpreter::ADC, "ADC", 4, Absolute},
	{0x7d, CPU_interpreter::ADC, "ADC", 4, Absolute_x},
	{0x79, CPU_interpreter::ADC, "ADC", 4, Absolute_y},
	{0x61, CPU_interpreter::ADC, "ADC", 6, Indexed_indirect},
	{0x71, CPU_interpreter::ADC, "ADC", 5, Indirect_indexed},

	// Subtract with Carry
	{0xE9, CPU_interpreter::SBC, "SBC", 2, Immediate},
	{0xE5, CPU_interpreter::SBC, "SBC", 3, Zero_page},
	{0xF5, CPU_interpreter::SBC, "SBC", 4, Zero_page_x},
	{0xEd, CPU_interpreter::SBC, "SBC", 4, Absolute},
	{0xFd, CPU_interpreter::SBC, "SBC", 4, Absolute_x},
	{0xF9, CPU_interpreter::SBC, "SBC", 4, Absolute_y},
	{0xE1, CPU_interpreter::SBC, "SBC", 6, Indexed_indirect},
	{0xF1, CPU_interpreter::SBC, "SBC", 5, Indirect_indexed},

	//  Compare accumulator
	{0xC9, CPU_interpreter::CMP, "CMP", 2, Immediate},
	{0xC5, CPU_interpreter::CMP, "CMP", 3, Zero_page},
	{0xD5, CPU_interpreter::CMP, "CMP", 4, Zero_page_x},
	{0xCD, CPU_interpreter::CMP, "CMP", 4, Absolute},
	{0xDD, CPU_interpreter::CMP, "CMP", 4, Absolute_x},
	{0xD9, CPU_interpreter::CMP, "CMP", 4, Absolute_y},
	{0xC1, CPU_interpreter::CMP, "CMP", 6, Indexed_indirect},
	{0xD1, CPU_interpreter::CMP, "CMP", 5, Indirect_indexed},

	// Compare X register
	{0xE0, CPU_interpreter::CPX, "CPX", 2, Immediate},
	{0xE4, CPU_interpreter::CPX, "CPX", 3, Zero_page},
	{0xEC, CPU_interpreter::CPX, "CPX", 4, Absolute},
	
	// Compare Y register
	{0xC0, CPU_interpreter::CPY, "CPY", 2, Immediate},
	{0xC4, CPU_interpreter::CPY, "CPY", 3, Zero_page},
	{0xCC, CPU_interpreter::CPY, "CPY", 4, Absolute},

	
	// Increments & Decrements
	// Increment a memory location
	{0xE6, CPU_interpreter::INC, "INC", 5, Zero_page},
	{0xF6, CPU_interpreter::INC, "INC", 6, Zero_page_x},
	{0xEE, CPU_interpreter::INC, "INC", 6, Absolute},
	{0xFE, CPU_interpreter::INC, "INC", 7, Absolute_x},

	// Increment the X register
	{0xE8, CPU_interpreter::INX, "INX", 2, Implicit},
	
	// Increment the Y register
	{0xC8, CPU_interpreter::INY, "INY", 2, Implicit},

	// Decrement a memory location
	{0xC6, CPU_interpreter::DEC, "DEC", 5, Zero_page},
	{0xD6, CPU_interpreter::DEC, "DEC", 6, Zero_page_x},
	{0xCE, CPU_interpreter::DEC, "DEC", 6, Absolute},
	{0xDE, CPU_interpreter::DEC, "DEC", 7, Absolute_x},

	// Decrement the X register
	{0xCA, CPU_interpreter::DEX, "DEX", 2, Implicit},

	// Decrement the Y register
	{0x88, CPU_interpreter::DEY, "DEY", 2, Implicit},


	// Shifts
	// Arithmetic Shift Left
	{0x0A, CPU_interpreter::ASL, "ASL", 2, Accumulator},
	{0x06, CPU_interpreter::ASL, "ASL", 5, Zero_page},
	{0x16, CPU_interpreter::ASL, "ASL", 6, Zero_page_x},
	{0x0E, CPU_interpreter::ASL, "ASL", 6, Absolute},
	{0x1E, CPU_interpreter::ASL, "ASL", 7, Absolute_x},

	// Logical Shift Right
	{0x4A, CPU_interpreter::LSR, "LSR", 2, Accumulator},
	{0x46, CPU_interpreter::LSR, "LSR", 5, Zero_page},
	{0x56, CPU_interpreter::LSR, "LSR", 6, Zero_page_x},
	{0x4E, CPU_interpreter::LSR, "LSR", 6, Absolute},
	{0x5E, CPU_interpreter::LSR, "LSR", 7, Absolute_x},

	// Rotate Left
	{0x2A, CPU_interpreter::ROL, "ROL", 2, Accumulator},
	{0x26, CPU_interpreter::ROL, "ROL", 5, Zero_page},
	{0x36, CPU_interpreter::ROL, "ROL", 6, Zero_page_x},
	{0x2E, CPU_interpreter::ROL, "ROL", 6, Absolute},
	{0x3E, CPU_interpreter::ROL, "ROL", 7, Absolute_x},

	// Rotate Right
	{0x6A, CPU_interpreter::ROR, "ROR", 2, Accumulator},
	{0x66, CPU_interpreter::ROR, "ROR", 5, Zero_page},
	{0x76, CPU_interpreter::ROR, "ROR", 6, Zero_page_x},
	{0x6E, CPU_interpreter::ROR, "ROR", 6, Absolute},
	{0x7E, CPU_interpreter::ROR, "ROR", 7, Absolute_x},

	// Jumps & Calls

	// Jump to another location
	{0x4C, CPU_interpreter::JMP, "JMP", 3, Absolute},
	{0x6C, CPU_interpreter::JMP, "JMP", 5, Indirect},

	// Jump to a subroutine
	{0x20, CPU_interpreter::JSR, "JSR", 3, Absolute},

	// Return form subroutine
	{0x60, CPU_interpreter::RTS, "RTS", 6, Implicit},


	// Branches
	{0x90, CPU_interpreter::BCC, "BCC", 2, Relative}, // Branch if carry flag clear
	{0xB0, CPU_interpreter::BCS, "BCS", 2, Relative}, // Branch if carry flag set	 
	{0xF0, CPU_interpreter::BEQ, "BEQ", 2, Relative}, // Branch if zero flag set	
	{0x30, CPU_interpreter::BMI, "BMI", 2, Relative}, // Branch if negative flag set	
	{0xD0, CPU_interpreter::BNE, "BNE", 2, Relative}, // Branch if zero flag clear	 
	{0x10, CPU_interpreter::BPL, "BPL", 2, Relative}, // Branch if negative flag clear	 
	{0x50, CPU_interpreter::BVC, "BVC", 2, Relative}, // Branch if overflow flag clear	
	{0x70, CPU_interpreter::BVS, "BVS", 2, Relative}, // Branch if overflow flag set


	// Status Flag Changes
	{0x18, CPU_interpreter::CLC, "CLC", 2, Implicit}, // Clear carry flag
	{0xD8, CPU_interpreter::CLD, "CLD", 2, Implicit}, // Clear decimal mode flag
	{0x58, CPU_interpreter::CLI, "CLI", 2, Implicit}, // Clear interrupt disable flag
	{0xB8, CPU_interpreter::CLV, "CLV", 2, Implicit}, // Clear overflow flag
	{0x38, CPU_interpreter::SEC, "SEC", 2, Implicit}, // Set carry flag
	{0xF8, CPU_interpreter::SED, "SED", 2, Implicit}, // Set decimal mode flag
	{0x78, CPU_interpreter::SEI, "SEI", 2, Implicit}, // Set interrupt disable flag

	// System Function
	{0x00, CPU_interpreter::BRK, "BRK", 7, Implicit}, // Force an interrupt
	{0xEA, CPU_interpreter::NOP, "NOP", 2, Implicit}, // No Operation
	{0x40, CPU_interpreter::RTI, "RTI", 6, Implicit}, // Return from Interrupt

	// Undocumented opcodes
	{0x04, CPU_interpreter::NOP, "NOP", 3, Zero_page},  // Double No Operation
	{0x14, CPU_interpreter::NOP, "NOP", 4, Zero_page_x},// Double No Operation
	{0x34, CPU_interpreter::NOP, "NOP", 4, Zero_page_x},// Double No Operation
	{0x44, CPU_interpreter::NOP, "NOP", 3, Zero_page},  // Double No Operation
	{0x54, CPU_interpreter::NOP, "NOP", 4, Zero_page_x},// Double No Operation
	{0x64, CPU_interpreter::NOP, "NOP", 3, Zero_page},  // Double No Operation
	{0x74, CPU_interpreter::NOP, "NOP", 4, Zero_page_x},// Double No Operation
	{0x80, CPU_interpreter::NOP, "NOP", 2, Immediate},  // Double No Operation
	{0x82, CPU_interpreter::NOP, "NOP", 2, Immediate},  // Double No Operation
	{0x89, CPU_interpreter::NOP, "NOP", 2, Immediate},  // Double No Operation
	{0xC2, CPU_interpreter::NOP, "NOP", 2, Immediate},  // Double No Operation
	{0xD4, CPU_interpreter::NOP, "NOP", 4, Zero_page_x},// Double No Operation
	{0xE2, CPU_interpreter::NOP, "NOP", 2, Immediate},  // Double No Operation
	{0xF4, CPU_interpreter::NOP, "NOP", 4, Zero_page_x},// Double No Operation

	{0x1A, CPU_interpreter::NOP, "NOP", 2, Implicit},   // No Operation
	{0x3A, CPU_interpreter::NOP, "NOP", 2, Implicit},   // No Operation
	{0x5A, CPU_interpreter::NOP, "NOP", 2, Implicit},   // No Operation
	{0x7A, CPU_interpreter::NOP, "NOP", 2, Implicit},   // No Operation
	{0xDA, CPU_interpreter::NOP, "NOP", 2, Implicit},   // No Operation
	{0xFA, CPU_interpreter::NOP, "NOP", 2, Implicit},   // No Operation


	{0x0C, CPU_interpreter::NOP, "NOP", 4, Absolute},     // Triple No Operation
	{0x1C, CPU_interpreter::NOP, "NOP", 4, Absolute_x},   // Triple No Operation
	{0x3C, CPU_interpreter::NOP, "NOP", 4, Absolute_x},   // Triple No Operation
	{0x5C, CPU_interpreter::NOP, "NOP", 4, Absolute_x},   // Triple No Operation
	{0x7C, CPU_interpreter::NOP, "NOP", 4, Absolute_x},   // Triple No Operation
	{0xDC, CPU_interpreter::NOP, "NOP", 4, Absolute_x},   // Triple No Operation
	{0xFC, CPU_interpreter::NOP, "NOP", 4, Absolute_x},   // Triple No Operation

	{0xEB, CPU_interpreter::SBC, "SBC", 2, Immediate},    // Substract

	// Load A and X
	{0xa7, CPU_interpreter::LAX, "LAX", 3, Zero_page},
	{0xb7, CPU_interpreter::LAX, "LAX", 4, Zero_page_y},
	{0xaf, CPU_interpreter::LAX, "LAX", 4, Absolute},
	{0xbf, CPU_interpreter::LAX, "LAX", 4, Absolute_y},
	{0xa3, CPU_interpreter::LAX, "LAX", 6, Indexed_indirect},
	{0xb3, CPU_interpreter::LAX, "LAX", 5, Indirect_indexed},
	
	
	// AND X register with accumulator and store result in memory
	{0x87, CPU_interpreter::SAX, "SAX", 3, Zero_page},
	{0x97, CPU_interpreter::SAX, "SAX", 4, Zero_page_y},
	{0x83, CPU_interpreter::SAX, "SAX", 6, Indexed_indirect},
	{0x8f, CPU_interpreter::SAX, "SAX", 4, Absolute},

	// Substract 1 from memory (without borrow)
	{0xc7, CPU_interpreter::DCP, "DCP", 5, Zero_page},
	{0xd7, CPU_interpreter::DCP, "DCP", 6, Zero_page_x},
	{0xcf, CPU_interpreter::DCP, "DCP", 6, Absolute},
	{0xdf, CPU_interpreter::DCP, "DCP", 7, Absolute_x},
	{0xdb, CPU_interpreter::DCP, "DCP", 7, Absolute_y},
	{0xc3, CPU_interpreter::DCP, "DCP", 8, Indexed_indirect},
	{0xd3, CPU_interpreter::DCP, "DCP", 8, Indirect_indexed},

	// Increase memory by one, then subtract memory from A (with borrow)
	{0xe7, CPU_interpreter::ISB, "ISB", 5, Zero_page},
	{0xf7, CPU_interpreter::ISB, "ISB", 6, Zero_page_x},
	{0xef, CPU_interpreter::ISB, "ISB", 6, Absolute},
	{0xff, CPU_interpreter::ISB, "ISB", 7, Absolute_x},
	{0xfb, CPU_interpreter::ISB, "ISB", 7, Absolute_y},
	{0xe3, CPU_interpreter::ISB, "ISB", 8, Indexed_indirect},
	{0xf3, CPU_interpreter::ISB, "ISB", 8, Indirect_indexed},

	// Increase memory by one, then subtract memory from A (with borrow)
	{0x07, CPU_interpreter::SLO, "SLO", 5, Zero_page},
	{0x17, CPU_interpreter::SLO, "SLO", 6, Zero_page_x},
	{0x0f, CPU_interpreter::SLO, "SLO", 6, Absolute},
	{0x1f, CPU_interpreter::SLO, "SLO", 7, Absolute_x},
	{0x1b, CPU_interpreter::SLO, "SLO", 7, Absolute_y},
	{0x03, CPU_interpreter::SLO, "SLO", 8, Indexed_indirect},
	{0x13, CPU_interpreter::SLO, "SLO", 8, Indirect_indexed},
	
	// Rotate one bit left in memory, then AND accumulator with memory
	{0x27, CPU_interpreter::RLA, "RLA", 5, Zero_page},
	{0x37, CPU_interpreter::RLA, "RLA", 6, Zero_page_x},
	{0x2f, CPU_interpreter::RLA, "RLA", 6, Absolute},
	{0x3f, CPU_interpreter::RLA, "RLA", 7, Absolute_x},
	{0x3b, CPU_interpreter::RLA, "RLA", 7, Absolute_y},
	{0x23, CPU_interpreter::RLA, "RLA", 8, Indexed_indirect},
	{0x33, CPU_interpreter::RLA, "RLA", 8, Indirect_indexed},
	
	// Rotate one bit left in memory, then AND accumulator with memory
	{0x47, CPU_interpreter::SRE, "SRE", 5, Zero_page},
	{0x57, CPU_interpreter::SRE, "SRE", 6, Zero_page_x},
	{0x4f, CPU_interpreter::SRE, "SRE", 6, Absolute},
	{0x5f, CPU_interpreter::SRE, "SRE", 7, Absolute_x},
	{0x5b, CPU_interpreter::SRE, "SRE", 7, Absolute_y},
	{0x43, CPU_interpreter::SRE, "SRE", 8, Indexed_indirect},
	{0x53, CPU_interpreter::SRE, "SRE", 8, Indirect_indexed},

	// Rotate one bit right in memory, then add memory to accumulator
	{0x67, CPU_interpreter::RRA, "RRA", 5, Zero_page},
	{0x77, CPU_interpreter::RRA, "RRA", 6, Zero_page_x},
	{0x6f, CPU_interpreter::RRA, "RRA", 6, Absolute},
	{0x7f, CPU_interpreter::RRA, "RRA", 7, Absolute_x},
	{0x7b, CPU_interpreter::RRA, "RRA", 7, Absolute_y},
	{0x63, CPU_interpreter::RRA, "RRA", 8, Indexed_indirect},
	{0x73, CPU_interpreter::RRA, "RRA", 8, Indirect_indexed},

	// AND byte with accumulator. If result is negative then carry is set.
	{0x0B, CPU_interpreter::ANC, "ANC", 2, Immediate},
	{0x2B, CPU_interpreter::ANC, "ANC", 2, Immediate},

	// AND byte with accumulator, then shift right one bit in accumulator.
	{0x4B, CPU_interpreter::ALR, "ALR", 2, Immediate},

	// AND byte with accumulator, then rotate one bit right in accumulator and set some flags
	{0x6B, CPU_interpreter::ARR, "ARR", 2, Immediate},

	// AND byte with accumulator, then transfer accumulator to X register.
	{0xAB, CPU_interpreter::ATX, "ATX", 2, Immediate},
	
	// AND X register with accumulator and store result in X register, then subtract byte from X register (without borrow).
	{0xCB, CPU_interpreter::AXS, "AXS", 2, Immediate},

	// AND Y register with the high byte of the target address of the argument + 1. Store the result in memory.
	{0x9C, CPU_interpreter::SYA, "SYA", 5, Absolute_x},

	// AND X register with the high byte of the target address of the argument + 1. Store the result in memory.
	{0x9E, CPU_interpreter::SXA, "SXA", 5, Absolute_y},

	// CPU JAM
	//{0x02, CPU_interpreter::KIL, "KIL", 1, Implicit},
	{0x12, CPU_interpreter::KIL, "KIL", 1, Implicit},
	{0x22, CPU_interpreter::KIL, "KIL", 1, Implicit},
	{0x32, CPU_interpreter::KIL, "KIL", 1, Implicit},
	{0x42, CPU_interpreter::KIL, "KIL", 1, Implicit},
	{0x52, CPU_interpreter::KIL, "KIL", 1, Implicit},
	{0x62, CPU_interpreter::KIL, "KIL", 1, Implicit},
	{0x72, CPU_interpreter::KIL, "KIL", 1, Implicit},
	{0x92, CPU_interpreter::KIL, "KIL", 1, Implicit},
	{0xB2, CPU_interpreter::KIL, "KIL", 1, Implicit},
	{0xD2, CPU_interpreter::KIL, "KIL", 1, Implicit},
	{0xF2, CPU_interpreter::KIL, "KIL", 1, Implicit},

	{0x02, CPU_interpreter::UNK, "GFOT"}

};

static OPCODE OpcodeTableOptimized[256];