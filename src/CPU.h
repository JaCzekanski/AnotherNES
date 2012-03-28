#pragma once
#include "headers.h"
#include "CPU_ram.h"
#include "PPU.h"

#define CARRY_FLAG     (1<<0)
#define ZERO_FLAG      (1<<1)
#define INTERRUPT_FLAG (1<<2) // 1 - disabled
#define DECIMAL_FLAG   (1<<3)
#define BREAK_FLAG     (1<<4)

#define OVERFLOW_FLAG  (1<<6)
#define NEGATIVE_FLAG  (1<<7)


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

	uint8_t instuction[3];

	CPU(void);
	~CPU(void);


	void RESET(); // as somebody pressed reset
	void NMI();
	void IRQ();

	void Reset(); // Resets all registers and prepares for execution
	void Load( uint8_t* rom, uint16_t size ); // Load ROM into program space
	void Step(); // One step of execution


	static void BRK(CPU* c);
	static void RTS(CPU* c);
	static void SEI(CPU* c);
	static void CLD(CPU* c);

	static void SEC(CPU* c);
	
	
	// Load
	static void LDA(CPU* c);
	static void LDX(CPU* c);
	static void LDY(CPU* c);

	// Store
	static void STA(CPU* c);
	static void STX(CPU* c);
	static void STY(CPU* c);

	// Transfer
	static void TAX(CPU* c);
	static void TAY(CPU* c);
	static void TXA(CPU* c);
	static void TYA(CPU* c);
	static void TSX(CPU* c);
	static void TXS(CPU* c);


	// Branch
	static void BCC(CPU* c); // v not sure
	static void BCS(CPU* c);
	static void BEQ(CPU* c);
	static void BMI(CPU* c);
	static void BNE(CPU* c);
	static void BPL(CPU* c);

	// Arithmetics
	static void DEX(CPU* c);
	static void DEY(CPU* c);
	static void INX(CPU* c);
	static void INY(CPU* c);

	static void INC(CPU* c);

	// Logic
	static void ORA(CPU* c);
	static void AND(CPU* c);

	static void ASL(CPU* c);
	static void LSR(CPU* c);

	// Stack
	static void PLA(CPU* c);
	static void PHA(CPU* c);


	// Subroutine
	static void JSR(CPU* c);
	static void JMP(CPU* c);

	//Compare
	static void CMP(CPU* c); // Not sure
	static void CPX(CPU* c); // Not sure
	static void CPY(CPU* c); // Not sure

	static void BIT(CPU* c); 

	static void UNK(CPU* c); //DBG PURPOSES
};

typedef void (*_Instruction) (CPU*);
struct OPCODE
{
	int number;
	_Instruction inst;
	char* mnemnic;
};

static OPCODE OpcodeTable[] = 
{
	{0x00, CPU::BRK, "BRK"},
	{0x78, CPU::SEI, "SEI"},
	{0xD8, CPU::CLD, "CLD"},
	{0x38, CPU::SEC, "SEC"},
	{0x60, CPU::RTS, "RTS"},

	// Load A
	{0xA1, CPU::LDA, "LDA"},
	{0xA5, CPU::LDA, "LDA"},
	{0xA9, CPU::LDA, "LDA"},
	{0xAD, CPU::LDA, "LDA"},
	{0xB1, CPU::LDA, "LDA"},
	{0xB5, CPU::LDA, "LDA"},
	{0xB9, CPU::LDA, "LDA"},
	{0xBD, CPU::LDA, "LDA"},

	// Load X
	{0xA2, CPU::LDX, "LDX"},
	{0xA6, CPU::LDX, "LDX"},
	{0xAE, CPU::LDX, "LDX"},
	{0xB6, CPU::LDX, "LDX"},
	{0xBE, CPU::LDX, "LDX"},

	// Load Y
	{0xA0, CPU::LDY, "LDY"},
	{0xA4, CPU::LDY, "LDY"},
	{0xAC, CPU::LDY, "LDY"},
	{0xB4, CPU::LDY, "LDY"},
	{0xBC, CPU::LDY, "LDY"},

	// Store A
	{0x81, CPU::STA, "STA"},
	{0x85, CPU::STA, "STA"},
	{0x8D, CPU::STA, "STA"},
	{0x91, CPU::STA, "STA"},
	{0x95, CPU::STA, "STA"},
	{0x99, CPU::STA, "STA"},
	{0x9D, CPU::STA, "STA"},

	// Store X
	{0x86, CPU::STX, "STX"},
	{0x96, CPU::STX, "STX"},
	{0x8E, CPU::STX, "STX"},

	// Store Y
	{0x84, CPU::STY, "STY"},
	{0x94, CPU::STY, "STY"},
	{0x8C, CPU::STY, "STY"},

	// Transfer
	{0xAA, CPU::TAX, "TAX"},
	{0xA8, CPU::TAY, "TAY"},
	{0x8A, CPU::TXA, "TXA"},
	{0x98, CPU::TYA, "TYA"},
	{0xBA, CPU::TSX, "TSX"},
	{0x9A, CPU::TXS, "TXS"},


	// Branch
	{0x90, CPU::BCC, "BCC"},
	{0xB0, CPU::BCS, "BCS"},
	{0xF0, CPU::BEQ, "BEQ"},
	{0x30, CPU::BMI, "BMI"},
	{0xD0, CPU::BNE, "BNE"},
	{0x10, CPU::BPL, "BPL"},

	{0x20, CPU::JSR, "JSR"},

	{0x4C, CPU::JMP, "JMP"},
	{0x6C, CPU::JMP, "JMP"},


	// Arithmetics

	{0xCA, CPU::DEX, "DEX"},
	{0x88, CPU::DEY, "DEY"},
	{0xE8, CPU::INX, "INX"},
	{0xC8, CPU::INY, "INY"},

	{0xE6, CPU::INC, "INC"},
	{0xEE, CPU::INC, "INC"},
	{0xF6, CPU::INC, "INC"},
	{0xFE, CPU::INC, "INC"},

	// Logic
	{0x01, CPU::ORA, "ORA"},
	{0x05, CPU::ORA, "ORA"},
	{0x09, CPU::ORA, "ORA"},
	{0x0D, CPU::ORA, "ORA"},
	{0x11, CPU::ORA, "ORA"},
	{0x15, CPU::ORA, "ORA"},
	{0x19, CPU::ORA, "ORA"},
	{0x1D, CPU::ORA, "ORA"},

	
	{0x21, CPU::AND, "AND"},
	{0x25, CPU::AND, "AND"},
	{0x29, CPU::AND, "AND"},
	{0x2D, CPU::AND, "AND"},
	{0x31, CPU::AND, "AND"},
	{0x35, CPU::AND, "AND"},
	{0x39, CPU::AND, "AND"},
	{0x3D, CPU::AND, "AND"},

	{0x0A, CPU::ASL, "ASL"},
	{0x06, CPU::ASL, "ASL"},
	{0x16, CPU::ASL, "ASL"},
	{0x0E, CPU::ASL, "ASL"},
	{0x1E, CPU::ASL, "ASL"},

	{0x4A, CPU::LSR, "LSR"},
	{0x46, CPU::LSR, "LSR"},
	{0x56, CPU::LSR, "LSR"},
	{0x4E, CPU::LSR, "LSR"},
	{0x5E, CPU::LSR, "LSR"},


	// Stack
	{0x68, CPU::PLA, "PLA"},
	{0x48, CPU::PHA, "PHA"},

	//Compare
	{0xC1, CPU::CMP, "CMP"},
	{0xC5, CPU::CMP, "CMP"},
	{0xC9, CPU::CMP, "CMP"},
	{0xCD, CPU::CMP, "CMP"},
	{0xD1, CPU::CMP, "CMP"},
	{0xD5, CPU::CMP, "CMP"},
	{0xD9, CPU::CMP, "CMP"},
	{0xDD, CPU::CMP, "CMP"},

	{0xE0, CPU::CPX, "CPX"},
	{0xE4, CPU::CPX, "CPX"},
	{0xEC, CPU::CPX, "CPX"},

	{0xC0, CPU::CPY, "CPY"},
	{0xC4, CPU::CPY, "CPY"},
	{0xCC, CPU::CPY, "CPY"},

	{0x24, CPU::BIT, "BIT"},
	{0x2C, CPU::BIT, "BIT"},






	{0x02, CPU::UNK, "GFOT"}

};

static OPCODE OpcodeTableOptimized[256];