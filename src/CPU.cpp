#include "CPU.h"

CPU::CPU(void)
{
	this->Power();
	Log->Debug("CPU created");
}

CPU::~CPU(void)
{
	Log->Debug("CPU destroyed");
}

void CPU::Push( uint8_t v )
{
	this->memory.Write( 0x100 + this->SP--, v );
}
void CPU::Push16( uint16_t v )
{
	this->Push( (v>>8)&0xff );
	this->Push( v&0xff );
}
uint8_t CPU::Pop( )
{
	return this->memory[ 0x100 + ++this->SP ];
}
uint16_t CPU::Pop16( )
{
	uint8_t low = this->Pop();
	uint8_t high = this->Pop();
	return low | (high<<8);
}

void CPU::Power()
{
	Log->Debug("CPU power up state");
	this->A = 0;
	this->X = 0;
	this->Y = 0;
	this->P = INTERRUPT_FLAG | UNKNOWN_FLAG;
	this->PC = this->memory[0xFFFD]<<8 | this->memory[0xFFFC];
	//this->PC = 0xc000;
	this->SP = 0xFD;
	jammed = false;
}

void CPU::Reset() 
{
	Log->Debug("CPU Reset");
	// A,X,Y not affected
	// S -= 3
	// I set to true
	// Internal memory unchanged
	// APU silenced $4015 = 0
	this->SP-=3;
	this->P |= INTERRUPT_FLAG;
	this->PC = this->memory[0xFFFD] << 8 | this->memory[0xFFFC];
	jammed = false;
}

void CPU::NMI() 
{
	//Log->Debug("CPU NMI");
	this->Push16( this->PC );
	this->Push( this->P );

	this->PC = this->memory[0xFFFB]<<8 | this->memory[0xFFFA];
}

void CPU::IRQ() 
{
	//Log->Debug("CPU IRQ");
	this->Push16( this->PC );
	this->Push( this->P );

	this->PC = this->memory[0xFFFF]<<8 | this->memory[0xFFFE];
}