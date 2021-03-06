#include "CPU_interpreter.h"

OPCODE OpcodeTableOptimized[256];
CPU_interpreter::CPU_interpreter()
{
	OPCODE op = {0x00, CPU_interpreter::UNK, "UNK"};
	for (int i = 0; i< 256; i++)
	{
		op.number = i;
		OpcodeTableOptimized[i] = op;
	}

	for (int i = 0; i< 256; i++)
	{
		if (OpcodeTable[i].number == 0x02)	break;
		OpcodeTableOptimized[ OpcodeTable[i].number ] = OpcodeTable[i];
	}
}

CPU_interpreter::~CPU_interpreter(void)
{
}

int CPU_interpreter::Step()
{
	int opsize = 0;

	OPCODE op = OpcodeTableOptimized[ this->memory[this->PC] ];

	uint8_t low,high; // Temporary variables for address calculations
 	uint8_t arg1 = this->memory[ this->PC+1 ];
	uint8_t arg2 = this->memory[ this->PC+2 ];
	uint16_t addrlo, addrhi;
	int CYCLE = op.cycles;
	bool page_crossed = false;
	page_cross = true;
	switch (op.address)
	{
	case Implicit: // No args
		this->virtaddr = 0;
		opsize = 1;
		break;
	case Accumulator:
		opsize = 1;
		this->virtaddr = 0x10000;
		break;
	case Immediate: // imm8
		this->virtaddr = this->PC+1;
		opsize = 2;
		break;
	case Zero_page: // $0000+arg1
		this->virtaddr = arg1;
		opsize = 2;
		break;
	case Zero_page_x: // $0000+arg1+X
		this->virtaddr = (arg1 + this->X) & 0xff;
		opsize = 2;
		break;
	case Zero_page_y: // $0000+arg1+Y
		this->virtaddr = (arg1 + this->Y) & 0xff;
		opsize = 2;
		break;
	case Relative: // -128 to +127
		opsize = 2;
		this->virtaddr = (this->PC+opsize) + (signed char)(arg1);
		break;
	case Absolute: // 16bit address
		this->virtaddr = (arg2<<8) | arg1;
		opsize = 3;
		break;
	case Absolute_x: // 16bit address + X
		this->virtaddr = ( ((arg2 << 8) | arg1) + this->X  ) & 0xffff;
		if (this->virtaddr>>8 != arg2) page_crossed = true;
		opsize = 3;
		break;
	case Absolute_y: // 16bit address + Y
		this->virtaddr = ( ((arg2 << 8) | arg1) + this->Y  ) & 0xffff;
		if (this->virtaddr >> 8 != arg2) page_crossed = true;
		opsize = 3;
		break;
	case Indirect:
		addrlo = (arg2<<8) | arg1;
		addrhi = (arg2<<8) | (uint8_t)(arg1 +1);
		this->virtaddr = this->memory[ addrlo ] | 
						 this->memory[ addrhi ]<<8;
		opsize = 3;
		break;
	case Indexed_indirect: 
		opsize = 2;
		addrlo = (arg1 + this->X    ) & 0xff;
		addrhi = (arg1 + this->X + 1) & 0xff;

		low  = this->memory[ addrlo ];
		high = this->memory[ addrhi ];
		this->virtaddr = low | (high<<8);
		break;
	case Indirect_indexed:
		opsize = 2;
		addrlo = arg1; 
		addrhi = (arg1+1) & 0xff;

		low  = this->memory[ addrlo ];
		high = this->memory[ addrhi ];

		this->virtaddr = (uint16_t)((low | (high << 8)) + this->Y);
		if (this->virtaddr >> 8 != arg2) page_crossed = true;

		break;
	default:
		Log->Error("CPU_interpreter::Step(): Unknown addressing mode!");
		jammed = true;
		return 0;
		break;
	}

	uint16_t oldPC = this->PC;
	PCchanged = false;

	op.inst(this);

	if (page_crossed && page_cross) CYCLE++;
	if (op.address == Relative && (oldPC != this->PC || PCchanged))
	{
		CYCLE++; // Branch succeed
		if (((oldPC + 2) & 0xff00) != (this->PC & 0xff00)) CYCLE++; // Branch to new page
	}
	if (!(PCchanged || oldPC != this->PC)) this->PC += opsize;
	
	return CYCLE;
 }

void CPU_interpreter::LDA( CPU_interpreter* c ) // Load Accumulator
{
	c->A = c->Readv();
	c->ZERO( c->A?0:1 );
	c->NEGATIVE( c->A&0x80 );
}

void CPU_interpreter::LDX( CPU_interpreter* c ) // Load X
{
	c->X = c->Readv();
	c->ZERO( c->X?0:1 );
	c->NEGATIVE( c->X&0x80 );
}

void CPU_interpreter::LDY( CPU_interpreter* c ) // Load Y
{
	c->Y = c->Readv();
	c->ZERO( c->Y?0:1 );
	c->NEGATIVE( c->Y&0x80 );
}

void CPU_interpreter::STA( CPU_interpreter* c ) // Store Accumulator
{
	c->Writev( c->A );
	c->page_cross = false;
}

void CPU_interpreter::STX( CPU_interpreter* c ) // Store X
{
	c->Writev( c->X );
}

void CPU_interpreter::STY( CPU_interpreter* c ) // Store Y
{
	c->Writev( c->Y );
}

// Register Transfers
void CPU_interpreter::TAX( CPU_interpreter* c ) // Transfer accumulator to X
{
	c->X = c->A;
	c->ZERO( c->X?0:1 );
	c->NEGATIVE( c->X&0x80 );
}

void CPU_interpreter::TAY( CPU_interpreter* c ) // Transfer accumulator to Y
{
	c->Y = c->A;
	c->ZERO( c->Y?0:1 );
	c->NEGATIVE( c->Y&0x80 );
}

void CPU_interpreter::TXA( CPU_interpreter* c ) // Transfer X to accumulator
{
	c->A = c->X;
	c->ZERO( c->A?0:1 );
	c->NEGATIVE( c->A&0x80 );
}

void CPU_interpreter::TYA( CPU_interpreter* c ) // Transfer y to accumulator
{
	c->A = c->Y;
	c->ZERO( c->A?0:1 );
	c->NEGATIVE( c->A&0x80 );
}


// Stack Operations
void CPU_interpreter::TSX( CPU_interpreter* c ) // Transfer stack pointer to X
{
	c->X = c->SP;
	c->ZERO( c->X?0:1 );
	c->NEGATIVE( c->X&0x80 );
}

void CPU_interpreter::TXS( CPU_interpreter* c ) // Transfer X to stack pointer
{
	c->SP = c->X;
}

void CPU_interpreter::PHA( CPU_interpreter* c ) // Push accumulator on stack
{
	c->Push( c->A );
}

void CPU_interpreter::PHP( CPU_interpreter* c ) // Push processor status on stack
{
	c->Push( c->P | BREAK_FLAG | UNKNOWN_FLAG );
}

void CPU_interpreter::PLA( CPU_interpreter* c ) // Pull accumulator from stack
{
	c->A = c->Pop();
	c->ZERO( c->A?0:1 );
	c->NEGATIVE( c->A&0x80 );
}

void CPU_interpreter::PLP( CPU_interpreter* c ) // Pull processor status from stack
{
	c->P = c->Pop() | UNKNOWN_FLAG;
	c->P &= ~BREAK_FLAG;
}


// Logical
void CPU_interpreter::AND( CPU_interpreter* c ) // Logical AND
{
	c->A = c->A & c->Readv();
	c->ZERO( c->A?0:1 );
	c->NEGATIVE( c->A&0x80 );
}

void CPU_interpreter::EOR( CPU_interpreter* c ) // Exclusive OR
{
	c->A = c->A ^ c->Readv();
	c->ZERO( c->A?0:1 );
	c->NEGATIVE( c->A&0x80 );
}

void CPU_interpreter::ORA( CPU_interpreter* c ) // Logical Inclusive OR
{
	c->A = c->A | c->Readv();
	c->ZERO( c->A?0:1 );
	c->NEGATIVE( c->A&0x80 );
}

void CPU_interpreter::BIT( CPU_interpreter* c ) // Bit Test
{
	uint8_t tmp =  c->Readv();
	c->ZERO( (c->A & tmp)?0:1 );
	c->OVERFLOW_( tmp&0x40 );
	c->NEGATIVE( tmp&0x80 );
}

// Arithmetic
void CPU_interpreter::ADC( CPU_interpreter* c ) // Add with Carry
{
	uint16_t ret =  (uint16_t)c->Readv() + c->A + (c->P&CARRY_FLAG);

	c->ZERO( (ret & 0xff)? 0 : 1 );
	c->CARRY( ret > 0xff );
	c->NEGATIVE( ret&0x80 );

	c->OVERFLOW_( ! ( ( c->A ^ c->Readv() ) & 0x80 )
		      &&  ( ( c->A ^ ret ) & 0x80) );

	
	c->A = ret&0xff;
}
void CPU_interpreter::SBC( CPU_interpreter* c ) // Subtract with Carry, WRONG
{
	uint16_t ret = (uint16_t)c->A - c->Readv() - ((c->P&CARRY_FLAG) ? 0 : 1);

	c->ZERO( (ret & 0xff)? 0 : 1 );
	c->CARRY( ret < 0x100 );
	c->NEGATIVE( ret&0x80 );

	c->OVERFLOW_(  ( ( c->A ^ c->Readv() ) & 0x80 )
		      &&  ( ( c->A ^ ret ) & 0x80)  );
	
	c->A = ret&0xff;
}
void CPU_interpreter::CMP( CPU_interpreter* c ) // Compare accumulator
{
	uint16_t ret = c->A - c->Readv();

	c->CARRY( (ret<256?1:0) ); // Set if A >= M
	c->ZERO( (ret?0:1) );
	c->NEGATIVE( ret&0x80 );
}
void CPU_interpreter::CPX( CPU_interpreter* c ) // Compare X register
{
	uint16_t ret = c->X - c->Readv();

	c->CARRY( (ret<256?1:0) ); // Set if X >= M
	c->ZERO( (ret?0:1) );
	c->NEGATIVE( ret&0x80 );
}
void CPU_interpreter::CPY( CPU_interpreter* c ) // Compare Y register
{
	uint16_t ret = c->Y - c->Readv();

	c->CARRY( (ret<256?1:0) ); // Set if X >= M
	c->ZERO( (ret?0:1) );
	c->NEGATIVE( ret&0x80 );
}
	
// Increments & Decrements
void CPU_interpreter::INC( CPU_interpreter* c ) // Increment a memory location
{
	uint8_t ret = c->Readv()+1;
	
	c->Writev( ret );

	c->ZERO( ret?0:1 );
	c->NEGATIVE( ret&0x80 );
}
void CPU_interpreter::INX( CPU_interpreter* c ) // Increment the X register
{
	c->X = c->X+1;

	c->ZERO( c->X?0:1 );
	c->NEGATIVE( c->X&0x80 );
}
void CPU_interpreter::INY( CPU_interpreter* c ) // Increment the Y register
{
	c->Y = c->Y+1;

	c->ZERO( c->Y?0:1 );
	c->NEGATIVE( c->Y&0x80 );
}
void CPU_interpreter::DEC( CPU_interpreter* c ) // Decrement a memory location
{
	uint8_t ret = c->Readv()-1;
	c->Writev( ret );

	c->ZERO( ret?0:1 );
	c->NEGATIVE( ret&0x80 );
}
void CPU_interpreter::DEX( CPU_interpreter* c ) // Decrement the X register
{
	c->X = c->X-1;

	c->ZERO( c->X?0:1 );
	c->NEGATIVE( c->X&0x80 );
}
void CPU_interpreter::DEY( CPU_interpreter* c ) // Decrement the Y register
{
	c->Y = c->Y-1;

	c->ZERO( c->Y?0:1 );
	c->NEGATIVE( c->Y&0x80 );
}


// Shifts
void CPU_interpreter::ASL( CPU_interpreter* c ) // Arithmetic Shift Left
{
	c->CARRY( c->Readv()&0x80 );

	uint8_t ret = c->Readv()<<1;
	c->Writev( ret );

	c->ZERO( ret?0:1 );
	c->NEGATIVE( (ret&0x80) );
}
void CPU_interpreter::LSR( CPU_interpreter* c ) // Logical Shift Right
{
	c->CARRY( c->Readv()&0x1 );

	uint8_t ret = c->Readv()>>1;
	c->Writev( ret );

	c->ZERO( ret?0:1 );
	c->NEGATIVE( (ret&0x80) );
}
void CPU_interpreter::ROL( CPU_interpreter* c ) // Rotate Left
{
	uint8_t ret = c->Readv()<<1 | c->P&CARRY_FLAG;

	c->CARRY( c->Readv()&0x80 );

	c->Writev( ret );

	c->ZERO( ret?0:1 );
	c->NEGATIVE( (ret&0x80) );
}
void CPU_interpreter::ROR( CPU_interpreter* c ) // Rotate Right
{
	uint8_t ret = c->Readv()>>1 | ((c->P&CARRY_FLAG)<<7);

	c->CARRY( c->Readv()&0x1 );

	c->Writev( ret );

	c->ZERO( ret?0:1 );
	c->NEGATIVE( (ret&0x80) );
}


// Jumps & Calls
void CPU_interpreter::JMP( CPU_interpreter* c ) // Jump to another location
{
	c->PC = c->Getv();
}

void CPU_interpreter::JSR( CPU_interpreter* c ) // Jump to a subroutine
{
	c->Push16(c->PC+2); // Corrected
	c->PC = c->Getv();
}
void CPU_interpreter::RTS( CPU_interpreter* c ) // Return form subroutine
{
	c->PC = c->Pop16()+1;
}

// Branches
void CPU_interpreter::BCC( CPU_interpreter* c ) // Branch if carry flag clear 
{
	if (! (c->P&CARRY_FLAG) ) c->PC = c->Getv();
}
void CPU_interpreter::BCS( CPU_interpreter* c ) // Branch if carry flag set	
{
	if ( (c->P&CARRY_FLAG) ) c->PC = c->Getv();
}
void CPU_interpreter::BEQ( CPU_interpreter* c ) // Branch if zero flag set	
{
	if ( (c->P&ZERO_FLAG) ) c->PC = c->Getv();
}
void CPU_interpreter::BMI( CPU_interpreter* c ) // Branch if negative flag set
{
	if ( (c->P&NEGATIVE_FLAG) ) c->PC = c->Getv();
}
void CPU_interpreter::BNE( CPU_interpreter* c ) // Branch if zero flag clear
{
	if (! (c->P&ZERO_FLAG) ) c->PC = c->Getv();
}
void CPU_interpreter::BPL( CPU_interpreter* c ) // Branch if negative flag clear
{
	if (! (c->P&NEGATIVE_FLAG) ) c->PC = c->Getv();
}
void CPU_interpreter::BVC( CPU_interpreter* c ) // Branch if overflow flag clear
{
	if (! (c->P&OVERFLOW_FLAG) ) c->PC = c->Getv();
}
void CPU_interpreter::BVS( CPU_interpreter* c ) // Branch if overflow flag set
{
	if ( (c->P&OVERFLOW_FLAG) ) c->PC = c->Getv();
}



// Status Flag Changes
void CPU_interpreter::CLC( CPU_interpreter* c ) // Clear carry flag
{
	c->CARRY(0);
}
void CPU_interpreter::CLD( CPU_interpreter* c ) // Clear decimal mode flag
{
	c->DECIMAL(0);
}
void CPU_interpreter::CLI( CPU_interpreter* c ) // Clear interrupt disable flag
{
	c->INTERRUPT(0);
}
void CPU_interpreter::CLV( CPU_interpreter* c ) // Clear overflow flag
{
	c->OVERFLOW_(0);
}
void CPU_interpreter::SEC( CPU_interpreter* c ) // Set carry flag
{
	c->CARRY(1);
}
void CPU_interpreter::SED( CPU_interpreter* c ) // Set decimal mode flag
{
	c->DECIMAL(1);
}
void CPU_interpreter::SEI( CPU_interpreter* c ) // Set interrupt disable flag
{
	c->INTERRUPT(1);
}

// System Function
void CPU_interpreter::BRK( CPU_interpreter* c ) // Force an interrupt
{
	c->Push16( c->PC+2 );
	c->Push( c->P | 0x30 );
	c->INTERRUPT(1);

	c->PC = c->memory[0xFFFF]<<8 | c->memory[0xFFFE];
}
void CPU_interpreter::NOP( CPU_interpreter* c ) // No Operation
{
}
void CPU_interpreter::RTI( CPU_interpreter* c ) // Return from Interrupt
{
	c->P = c->Pop() | UNKNOWN_FLAG;
	c->BREAK(0); // THIS FLAG DOES NOT EXISTS :)
	c->PC = c->Pop16();
}


// Undocumented opcodes
void CPU_interpreter::LAX( CPU_interpreter* c ) // Load A and X
{
	c->A = c->Readv();
	c->X = c->Readv();
	c->ZERO( c->A?0:1 );
	c->NEGATIVE( c->A&0x80 );
}

void CPU_interpreter::SAX( CPU_interpreter* c ) //  AND X register with accumulator (without changing them) and store result in memory.
{
	uint8_t ret = c->X & c->A;
	
	c->Writev( ret );
}

void CPU_interpreter::DCP( CPU_interpreter* c ) // Substract 1 from memory and CMP acc with this
{
	uint16_t ret = c->Readv() - 1;

	c->CARRY( ret < 0x100 );
	c->ZERO( (ret & 0xff)? 0 : 1 );
	
	c->Writev( (uint8_t)ret );

	ret = c->A - c->Readv();

	c->CARRY( (ret<256?1:0) ); // Set if A >= M
	c->ZERO( (ret?0:1) );
	c->NEGATIVE(ret & 0x80);
	c->page_cross = false;
}

void CPU_interpreter::ISB( CPU_interpreter* c ) // Increase memory by one, then subtract memory from A (with borrow)
{
	// INC
	uint16_t ret = (uint8_t) (c->Readv()+1);
	
	c->Writev( (uint8_t)ret );

	// SBC
	ret = (uint16_t)c->A - c->Readv() - ((c->P&CARRY_FLAG) ? 0 : 1);

	c->ZERO( (ret & 0xff)? 0 : 1 );
	c->CARRY( ret < 0x100 );
	c->NEGATIVE( ret&0x80 );

	c->OVERFLOW_(  ( ( c->A ^ c->Readv() ) & 0x80 )
		      &&  ( ( c->A ^ ret ) & 0x80)  );
	
	c->A = (uint8_t)ret;
	c->page_cross = false;
}

void CPU_interpreter::SLO( CPU_interpreter* c ) // Shift left one bit in memory, then OR accumulator with memory
{
	// ASL
	c->CARRY( c->Readv()&0x80 );

	uint8_t ret = c->Readv()<<1;
	c->Writev( ret );

	// OR
	c->A = c->A | c->Readv();
	c->ZERO( c->A?0:1 );
	c->NEGATIVE(c->A & 0x80);
	c->page_cross = false;
}

void CPU_interpreter::RLA( CPU_interpreter* c ) // Rotate one bit left in memory, then AND accumulator with memory
{
	// ROL
	uint8_t ret = c->Readv()<<1 | c->P&CARRY_FLAG;

	c->CARRY( c->Readv()&0x80 );
	c->Writev( ret );

	// AND
	c->A = c->A & c->Readv();
	c->ZERO( c->A?0:1 );
	c->NEGATIVE(c->A & 0x80);
	c->page_cross = false;
}

void CPU_interpreter::SRE( CPU_interpreter* c ) // Shift right one bit in memory, then EOR accumulator with memory
{
	//LSR
	c->CARRY( c->Readv()&0x1 );

	uint8_t ret = c->Readv()>>1;
	c->Writev( ret );

	// EOR
	c->A = c->A ^ c->Readv();
	c->ZERO( c->A?0:1 );
	c->NEGATIVE(c->A & 0x80);
	c->page_cross = false;
}

void CPU_interpreter::RRA( CPU_interpreter* c ) // Rotate one bit right in memory, then add memory to accumulator
{
	// Rotate (ROR)
	uint16_t ret = (uint8_t)( c->Readv()>>1 | ((c->P&CARRY_FLAG)<<7));

	c->CARRY( c->Readv()&0x1 );
	c->Writev( (uint8_t)ret );


	// ADC
	ret = (uint16_t)c->Readv() + c->A + (c->P&CARRY_FLAG);

	c->ZERO( (ret & 0xff)? 0 : 1 );
	c->CARRY( ret > 0xff );
	c->NEGATIVE( ret&0x80 );

	c->OVERFLOW_( ! ( ( c->A ^ c->Readv() ) & 0x80 )
		      &&  ( ( c->A ^ ret ) & 0x80) );

	
	c->A = (uint8_t)ret;
	c->page_cross = false;
}

// UNTESTED
void CPU_interpreter::ANC( CPU_interpreter* c ) // AND byte with accumulator. If result is negative then carry is set.
{
	c->A = c->A & c->Readv();
	c->ZERO( c->A?0:1 );
	c->NEGATIVE( c->A&0x80 );
	c->CARRY( c->A&0x80 );
}

// UNTESTED
void CPU_interpreter::ALR( CPU_interpreter* c ) // AND byte with accumulator, then shift right one bit in accumulator.
{
	c->A = c->A & c->Readv();

	c->CARRY( c->A&0x1 );

	uint8_t ret = c->A>>1;
	c->A = ret;

	c->ZERO( ret?0:1 );
	c->NEGATIVE( (ret&0x80) );
}

void CPU_interpreter::ARR( CPU_interpreter* c ) // AND byte with accumulator, then shift right one bit in accumulator.
{
	c->A = c->A & c->Readv();


	c->A = c->A>>1 | ((c->P&CARRY_FLAG)<<7);

	if ( c->A & 0x40 && c->A & 0x20 ) // Set C, clear V
	{
		c->CARRY( 1 );
		c->OVERFLOW_( 0 );
	}
	else if ( !(c->A & 0x40) && !(c->A & 0x20) ) // Clear C and V
	{
		c->CARRY( 0 );
		c->OVERFLOW_( 0 );
	}
	else if ( c->A & 0x20 ) // Set V, clear C
	{
		c->CARRY( 0 );
		c->OVERFLOW_( 1 );
	}
	else if ( c->A & 0x40 ) // Set C and V
	{
		c->CARRY( 1 );
		c->OVERFLOW_( 1 );
	}

	c->ZERO( c->A?0:1 );
	c->NEGATIVE( (c->A&0x80) );
}

void CPU_interpreter::ATX( CPU_interpreter* c ) // AND byte with accumulator, then transfer accumulator to X register.
{
	c->A = c->A & c->Readv();
	c->X = c->A;

	c->ZERO( c->A?0:1 );
	c->NEGATIVE( (c->A&0x80) );
}

void CPU_interpreter::AXS( CPU_interpreter* c ) // AND X register with accumulator and store result in X register, then subtract byte from X register (without borrow).
{
	c->X = c->X & c->A;

	uint16_t ret = (uint16_t)c->X - c->Readv();

	c->ZERO( (ret & 0xff)? 0 : 1 );
	c->CARRY( ret < 0x100 );
	c->NEGATIVE( ret&0x80 );

	c->X = (uint8_t)ret;
}

void CPU_interpreter::SYA( CPU_interpreter* c ) // AND Y register with the high byte of the target address of the argument + 1. Store the result in memory.
{
	c->Writev( c->Y & ((c->virtaddr>>8)+1) );
}

void CPU_interpreter::SXA( CPU_interpreter* c ) // AND X register with the high byte of the target address of the argument + 1. Store the result in memory.
{
	c->Writev( c->X & ((c->virtaddr>>8)+1) );
}

void CPU_interpreter::KIL(CPU_interpreter* c)
{
	c->jammed = true;
	Log->Fatal("0x%x: CPU Jam (0x%x), halting!", c->PC, c->memory[c->PC]);
}
	
void CPU_interpreter::UNK(CPU_interpreter* c)
{
	c->jammed = true;
	Log->Debug("0x%x: Unknown instruction (0x%x), halting!", c->PC, c->memory[c->PC]);
}