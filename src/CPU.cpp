#include "CPU.h"

CPU::CPU(void)
{
	OPCODE op = {0x00, CPU::UNK, "UNK"};
	for (int i = 0; i< 256; i++)
	{
		op.number = i;
		OpcodeTableOptimized[i] = op;
	}
	log->Debug("Opcode table cleaned");
	int i;
	for (i = 0; i< 256; i++)
	{
		if (OpcodeTable[i].number == 0x02)
		{
			break;
		}
		OpcodeTableOptimized[ OpcodeTable[i].number ] = OpcodeTable[i];
	}
	log->Debug("Opcode table filled");
	log->Debug("CPU created");
}

CPU::~CPU(void)
{
	log->Debug("CPU destroyed");
}

void CPU::RESET() 
{
	log->Debug("CPU RESET");
	this->PC = this->memory[0xFFFD]<<8 | this->memory[0xFFFC];
}

void CPU::NMI() 
{
	//log->Debug("CPU NMI");
	this->Push16( this->PC );
	this->Push( this->P );

	this->PC = this->memory[0xFFFB]<<8 | this->memory[0xFFFA];
}

void CPU::IRQ() 
{
	//log->Debug("CPU IRQ");
	this->Push16( this->PC );
	this->Push( this->P );

	this->PC = this->memory[0xFFFF]<<8 | this->memory[0xFFFE];
}


void CPU::Reset()
{
	log->Debug("CPU reseted");
	this->A = 0;
	this->X = 0;
	this->Y = 0;
	this->P = (1<<5) | INTERRUPT_FLAG;
	this->PC = this->memory[0xFFFD]<<8 | this->memory[0xFFFC];
	this->SP = 0;
}

void CPU::Push( uint8_t v )
{
	this->memory.Write( 0x100 + this->SP--, v );
}
void CPU::Push16( uint16_t v )
{
	this->Push( v&0xff );
	this->Push( (v>>8)&0xff );
}
uint8_t CPU::Pop( )
{
	return this->memory[ 0x100 + ++this->SP ];
}
uint16_t CPU::Pop16( )
{
	return this->Pop()<<8 | this->Pop();
}

void CPU::Load( uint8_t* rom, uint16_t size )
{
	
}

#ifdef _DEBUG
#define DISASM(x,y) (sprintf(buffer,x,y))
#endif
#ifndef _DEBUG
#define DISASM(x,y) 
#endif

#undef _DEBUG
void CPU::Step()
{
	char buffer[512] = {0};
	int opsize = 1;

	OPCODE op = OpcodeTableOptimized[ this->memory[this->PC] ];

	uint8_t* arg1 = &this->memory[ this->PC+1 ];
	uint8_t* arg2 = &this->memory[ this->PC+2 ];
	switch (op.address)
	{
	case Implicit: // No args
		this->virtaddr = 0;
		this->virtaddr = NULL;
		opsize = 1;
		DISASM("%c", 0);
		break;
	case Accumulator: // Accumulato
		opsize = 1;
		this->virtaddr = 0xffff+1;
		DISASM("%c", 'A');
		break;
	case Immediate: // imm8
		this->virtaddr = this->PC+1;
		opsize = 2;
		DISASM("#%.2x", Readv());
		break;
	case Zero_page: // $0000+arg1
		this->virtaddr = *arg1;
		opsize = 2;
		DISASM("$%.2x", virtaddr);
		break;
	case Zero_page_x: // $0000+arg1+X
		this->virtaddr = *arg1+ this->X;
		opsize = 2;
		DISASM("$%.2x,X", this->memory[*arg1]);
		break;
	case Zero_page_y: // $0000+arg1+Y
		this->virtaddr = *arg1+ this->Y;
		opsize = 2;
		DISASM("$%.2x,Y", this->memory[*arg1]);
		break;
	case Relative: // -128 to +127, not sure, HAXED
		opsize = 2;
		this->virtaddr = (this->PC+opsize) + (signed char)(*arg1);
		DISASM("$%.4x", virtaddr);
		break;
	case Absolute: // 16bit address
		this->virtaddr = ((*arg2)<<8) | (*arg1);
		opsize = 3;
		DISASM("$%.4x", virtaddr );
		break;
	case Absolute_x: // 16bit address + X
		this->virtaddr = ((*arg2<<8) | (*arg1)) + this->X;
		opsize = 3;
		DISASM("$%.4x,X", virtaddr-this->X );
		break;
	case Absolute_y: // 16bit address + Y
		this->virtaddr = ((*arg2<<8) | (*arg1)) + this->Y;
		opsize = 3;
		DISASM("$%.4x,Y", virtaddr- this->Y );
		break;
	case Indirect:
		this->virtaddr = this->memory[ (*arg2<<8) | (*arg1) ];
		opsize = 3;
		DISASM("($%.4x)", virtaddr );
		break;
	case Indexed_indirect: ///?????
		this->virtaddr = this->memory[ *arg1 + this->X ];
		opsize = 2;
		DISASM("($%.2x,X)", this->memory[ (*arg2<<8) | (*arg1) ]);
		break;
	case Indirect_indexed: ///?????
		this->virtaddr = this->memory[ *arg1 ] + this->Y ;
		opsize = 2;
		DISASM("($%.2x),Y", this->memory[ (*arg2<<8) | (*arg1) ]);
		break;
	default:
		log->Error("CPU::Step(): Unknown addressing mode!");
		break;
	}
	// Page crossed + 1 to cycles
#ifdef _DEBUG

	char hexvals[32];
	for (int i = 0; i<opsize; i++)
	{
		sprintf(hexvals+(i*3), "%.2x ", this->memory[this->PC+i] );
	}

	log->Debug("0x%x: %s\t\t%s %s", this->PC, hexvals, op.mnemnic, buffer );
#endif
	uint16_t oldPC = this->PC;
	op.inst(this);

	if (oldPC == this->PC) 
	{
		this->PC += opsize;
	}
}

void CPU::LDA( CPU* c ) // Load Accumulator
{
	c->A = c->Readv();
	c->ZERO( c->A );
	c->NEGATIVE( c->A&0x80 );
}

void CPU::LDX( CPU* c ) // Load X
{
	c->X = c->Readv();
	c->ZERO( c->X );
	c->NEGATIVE( c->X&0x80 );
}

void CPU::LDY( CPU* c ) // Load Y
{
	c->Y = c->Readv();
	c->ZERO( c->Y );
	c->NEGATIVE( c->Y&0x80 );
}

void CPU::STA( CPU* c ) // Store Accumulator
{
	c->Writev( c->A );
}

void CPU::STX( CPU* c ) // Store X
{
	c->Writev( c->X );
}

void CPU::STY( CPU* c ) // Store Y
{
	c->Writev( c->Y );
}

// Register Transfers
void CPU::TAX( CPU* c ) // Transfer accumulator to X
{
	c->X = c->A;
	c->ZERO( c->X );
	c->NEGATIVE( c->X&0x80 );
}

void CPU::TAY( CPU* c ) // Transfer accumulator to Y
{
	c->Y = c->A;
	c->ZERO( c->Y );
	c->NEGATIVE( c->Y&0x80 );
}

void CPU::TXA( CPU* c ) // Transfer X to accumulator
{
	c->A = c->X;
	c->ZERO( c->A );
	c->NEGATIVE( c->A&0x80 );
}

void CPU::TYA( CPU* c ) // Transfer y to accumulator
{
	c->A = c->Y;
	c->ZERO( c->A );
	c->NEGATIVE( c->A&0x80 );
}


// Stack Operations
void CPU::TSX( CPU* c ) // Transfer stack pointer to X
{
	c->X = c->SP;
	c->ZERO( c->X );
	c->NEGATIVE( c->X&0x80 );
}

void CPU::TXS( CPU* c ) // Transfer X to stack pointer
{
	c->SP = c->X;
}

void CPU::PHA( CPU* c ) // Push accumulator on stack
{
	c->Push( c->A );
}

void CPU::PHP( CPU* c ) // Push processor status on stack
{
	c->Push( c->P );
}

void CPU::PLA( CPU* c ) // Pull accumulator from stack
{
	c->A = c->Pop();
	c->ZERO( c->A );
	c->NEGATIVE( c->A&0x80 );
}

void CPU::PLP( CPU* c ) // Pull processor status from stack
{
	c->P = c->Pop();
}


// Logical
void CPU::AND( CPU* c ) // Logical AND
{
	c->A = c->A & c->Readv();
	c->ZERO( c->A );
	c->NEGATIVE( c->A&0x80 );
}

void CPU::EOR( CPU* c ) // Exclusive OR
{
	c->A = c->A ^ c->Readv();
	c->ZERO( c->A );
	c->NEGATIVE( c->A&0x80 );
}

void CPU::ORA( CPU* c ) // Logical Inclusive OR
{
	c->A = c->A | c->Readv();
	c->ZERO( c->A );
	c->NEGATIVE( c->A&0x80 );
}

void CPU::BIT( CPU* c ) // Bit Test
{
	uint8_t tmp =  c->Readv();
	c->ZERO( c->A & tmp );
	c->OVERFLOW( tmp&0x40 );
	c->NEGATIVE( tmp&0x80 );
}

// Arithmetic
// TODO: NOT SURE IF WORKING
void CPU::ADC( CPU* c ) // Add with Carry
{
	uint16_t ret = c->A + c->Readv() + (c->P&CARRY_FLAG);
	c->A = ret&0xff;


	c->CARRY( ret&0x100 );
	c->ZERO( c->A );
	c->NEGATIVE( c->A&0x80 );
}
void CPU::SBC( CPU* c ) // Subtract with Carry
{
	uint16_t ret = c->A -  c->Readv() - (1-(c->P&CARRY_FLAG));
	c->A = ret&0xff;

	c->CARRY( !(ret&0x100) );
	c->ZERO( c->A );
	c->NEGATIVE( c->A&0x80 );
}
void CPU::CMP( CPU* c ) // Compare accumulator
{
	uint16_t ret = c->A - c->Readv();

	c->CARRY( (ret<256?1:0) ); // Set if A >= M
	c->ZERO( (ret?0:1) );
	c->NEGATIVE( ret&0x80 );
}
void CPU::CPX( CPU* c ) // Compare X register
{
	uint16_t ret = c->X - c->Readv();

	c->CARRY( (ret<256?1:0) ); // Set if X >= M
	c->ZERO( (ret?0:1) );
	c->NEGATIVE( ret&0x80 );
}
void CPU::CPY( CPU* c ) // Compare Y register
{
	uint16_t ret = c->Y - c->Readv();

	c->CARRY( (ret<256?1:0) ); // Set if X >= M
	c->ZERO( (ret?0:1) );
	c->NEGATIVE( ret&0x80 );
}
	
// Increments & Decrements
void CPU::INC( CPU* c ) // Increment a memory location
{
	uint8_t ret = c->Readv()+1;
	
	c->Writev( ret );

	c->ZERO( ret );
	c->NEGATIVE( ret&0x80 );
}
void CPU::INX( CPU* c ) // Increment the X register
{
	c->X = c->X+1;

	c->ZERO( c->X );
	c->NEGATIVE( c->X&0x80 );
}
void CPU::INY( CPU* c ) // Increment the Y register
{
	c->Y = c->Y+1;

	c->ZERO( c->Y );
	c->NEGATIVE( c->Y&0x80 );
}
void CPU::DEC( CPU* c ) // Decrement a memory location
{
	uint8_t ret = c->Readv()-1;
	c->Writev( ret );

	c->ZERO( ret );
	c->NEGATIVE( ret&0x80 );
}
void CPU::DEX( CPU* c ) // Decrement the X register
{
	c->X = c->X-1;

	c->ZERO( c->X );
	c->NEGATIVE( c->X&0x80 );
}
void CPU::DEY( CPU* c ) // Decrement the Y register
{
	c->Y = c->Y-1;

	c->ZERO( c->Y );
	c->NEGATIVE( c->Y&0x80 );
}


// Shifts
void CPU::ASL( CPU* c ) // Arithmetic Shift Left
{
	c->CARRY( c->Readv()&0x1 );

	uint8_t ret = c->Readv()<<1;
	c->Writev( ret );

	c->ZERO( ret );
	c->NEGATIVE( (ret&0x80) );
}
void CPU::LSR( CPU* c ) // Logical Shift Right
{
	c->CARRY( c->Readv()&0x80 );

	uint8_t ret = c->Readv()>>1;
	c->Writev( ret );

	c->ZERO( ret );
	c->NEGATIVE( (ret&0x80) );
}
void CPU::ROL( CPU* c ) // Rotate Left
{
	c->CARRY( c->Readv()&0x80 );

	uint8_t ret = c->Readv()<<1 | c->P&CARRY_FLAG;
	c->Writev( ret );

	c->ZERO( ret );
	c->NEGATIVE( (ret&0x80) );
}
void CPU::ROR( CPU* c ) // Rotate Right
{
	c->CARRY( c->Readv()&0x1 );

	uint8_t ret = c->Readv()>>1 | ((c->P&CARRY_FLAG)<<7);
	c->Writev( ret );

	c->ZERO( ret );
	c->NEGATIVE( (ret&0x80) );
}


// Jumps & Calls
void CPU::JMP( CPU* c ) // Jump to another location
{
	c->PC = c->Getv();
}

// TODO: Dunno
void CPU::JSR( CPU* c ) // Jump to a subroutine
{
	c->Push16(c->PC+3);
	c->PC = c->Getv();
}
void CPU::RTS( CPU* c ) // Return form subroutine
{
	c->PC = c->Pop16();
}

// Branches
void CPU::BCC( CPU* c ) // Branch if carry flag clear 
{
	if (! (c->P&CARRY_FLAG) ) c->PC = c->Getv();
}
void CPU::BCS( CPU* c ) // Branch if carry flag set	
{
	if ( (c->P&CARRY_FLAG) ) c->PC = c->Getv();
}
void CPU::BEQ( CPU* c ) // Branch if zero flag set	
{
	if ( (c->P&ZERO_FLAG) ) c->PC = c->Getv();
}
void CPU::BMI( CPU* c ) // Branch if negative flag set
{
	if ( (c->P&NEGATIVE_FLAG) ) c->PC = c->Getv();
}
void CPU::BNE( CPU* c ) // Branch if zero flag clear
{
	if (! (c->P&ZERO_FLAG) ) c->PC = c->Getv();
}
void CPU::BPL( CPU* c ) // Branch if negative flag clear
{
	if (! (c->P&NEGATIVE_FLAG) ) c->PC = c->Getv();
}
void CPU::BVC( CPU* c ) // Branch if overflow flag clear
{
	if (! (c->P&OVERFLOW_FLAG) ) c->PC = c->Getv();
}
void CPU::BVS( CPU* c ) // Branch if overflow flag set
{
	if ( (c->P&OVERFLOW_FLAG) ) c->PC = c->Getv();
}



// Status Flag Changes
void CPU::CLC( CPU* c ) // Clear carry flag
{
	c->CARRY(0);
}
void CPU::CLD( CPU* c ) // Clear decimal mode flag
{
	c->DECIMAL(0);
}
void CPU::CLI( CPU* c ) // Clear interrupt disable flag
{
	c->INTERRUPT(0);
}
void CPU::CLV( CPU* c ) // Clear overflow flag
{
	c->OVERFLOW(0);
}
void CPU::SEC( CPU* c ) // Set carry flag
{
	c->CARRY(1);
}
void CPU::SED( CPU* c ) // Set decimal mode flag
{
	c->DECIMAL(1);
}
void CPU::SEI( CPU* c ) // Set interrupt disable flag
{
	c->INTERRUPT(1);
}

// System Function
void CPU::BRK( CPU* c ) // Force an interrupt
{
	log->Debug("0x%x: Break, suspicious... ", c->PC);
	//c->IRQ();
	c->Push16( c->PC+1 );
	c->Push( c->P );

	c->PC = c->memory[0xFFFF]<<8 | c->memory[0xFFFE];
	c->BREAK(1);
}
void CPU::NOP( CPU* c ) // No Operation
{
}
void CPU::RTI( CPU* c ) // Return from Interrupt
{
	c->P = c->Pop();
	c->PC = c->Pop16();
}

void CPU::UNK(CPU* c)
{
	log->Debug("0x%x: Unknown instruction (0x%x), halting!", c->PC, c->memory[c->PC]);
	for(;;)
	{
		Sleep(1);
	}
}