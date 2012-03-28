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
	this->memory[ 0x100 + this->SP-- ] = (this->PC+2)&0xff;
	this->memory[ 0x100 + this->SP-- ] = ((this->PC+2)>>8)&0xff;
	this->memory[ 0x100 + this->SP-- ] = this->P;

	this->PC = this->memory[0xFFFB]<<8 | this->memory[0xFFFA];
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

void CPU::Load( uint8_t* rom, uint16_t size )
{
	
}

void CPU::Step()
{
	this->instuction[0] = this->memory[ this->PC ] ;
	this->instuction[1] = this->memory[ this->PC+1 ] ;
	this->instuction[2] = this->memory[ this->PC+2 ] ;

	OPCODE op = OpcodeTableOptimized[this->instuction[0]];
	//log->Debug("0x%x: %x %s", this->PC, this->instuction[0], op.mnemnic );
	op.inst(this);

	this->PC++;
}

void CPU::BRK(CPU* c)
{
	c->P |= BREAK_FLAG;
	c->PC = c->memory[0xFFFF]<<8 | c->memory[0xFFFE];
}

void CPU::RTS(CPU* c)
{
	uint8_t high = c->memory[ 0x100 + (++c->SP) ];
	uint8_t low = c->memory[ 0x100 + (++c->SP) ];
	c->PC = (high<<8 | low);
}

void CPU::SEI(CPU* c)
{
	c->P |= INTERRUPT_FLAG;
}

void CPU::CLD(CPU* c)
{
	c->P &= ~DECIMAL_FLAG;
}

void CPU::SEC(CPU* c)
{
	c->P |= CARRY_FLAG;
}

void CPU::LDA(CPU* c)
{
	uint8_t addr = ( c->instuction[0] &0x1c )>>2;

	switch (addr)
	{
	case 0x00: // (zero page, X)
		c->A = c->memory[ c->memory[c->instuction[1] + c->X] ];
		c->PC++;
			break;

	case 0x01: // zero page
		c->A = c->memory[ c->instuction[1] ];
		c->PC++;
			break;

	case 0x02: // #immediate
		c->A = c->instuction[1];
		c->PC++;
			break;

	case 0x03: // absolute
		c->A = c->memory[ (c->instuction[2]<<8) | c->instuction[1] ];
		c->PC+=2;
			break;

	case 0x04: // (zero page),Y
		c->A = c->memory[ c->memory[c->instuction[1]] + c->Y ];
		c->PC++;
			break;

	case 0x05: // zero page, X
		c->A = c->memory[ c->instuction[1] + c->X ];
			break;

	case 0x06: // absolute, Y
		c->A = c->memory[ ((c->instuction[2]<<8) | c->instuction[1]) + c->Y ];
		c->PC+=2;
			break;

	case 0x07: // absolute, X
		c->A = c->memory[ ((c->instuction[2]<<8) | c->instuction[1]) + c->X ];
		c->PC+=2;
			break;
	}
	if (!c->A) c->P |= ZERO_FLAG;
	else c->P &= ~ZERO_FLAG;

	if ( ((signed char)c->A) <0) c->P |= NEGATIVE_FLAG;
	else c->P &= ~NEGATIVE_FLAG;
}

void CPU::LDX(CPU* c)
{
	uint8_t addr = ( c->instuction[0] &0x1c )>>2;

	switch (addr)
	{
	case 0x00: // #immediate
		c->X = c->instuction[1];
		c->PC++;
			break;

	case 0x01: // zero page
		c->X = c->memory[ c->instuction[1] ];
		c->PC++;
			break;

	case 0x05: // zero page,Y
		c->X = c->memory[ c->instuction[1] + c->Y ];
		c->PC++;
			break;

	case 0x03: // absolute
		c->X = c->memory[ (c->instuction[2]<<8) | c->instuction[1] ];
		c->PC+=2;
			break;

	case 0x07: // absolute, Y
		c->X = c->memory[ ((c->instuction[2]<<8) | c->instuction[1]) + c->Y ];
		c->PC+=2;
			break;

	}
	if (!c->X) c->P |= ZERO_FLAG;
	else c->P &= ~ZERO_FLAG;

	if ( ((signed char)c->X) <0) c->P |= NEGATIVE_FLAG;
	else c->P &= ~NEGATIVE_FLAG;
}

void CPU::LDY(CPU* c)
{
	uint8_t addr = ( c->instuction[0] &0x1c )>>2;

	switch (addr)
	{
	case 0x00: // #immediate
		c->Y = c->instuction[1];
		c->PC++;
			break;

	case 0x01: // zero page
		c->Y = c->memory[ c->instuction[1] ];
		c->PC++;
			break;

	case 0x05: // zero page,X
		c->Y = c->memory[ c->instuction[1] + c->X ];
		c->PC++;
			break;

	case 0x03: // absolute
		c->Y = c->memory[ (c->instuction[2]<<8) | c->instuction[1] ];
		c->PC+=2;
			break;

	case 0x07: // absolute, X
		c->Y = c->memory[ ((c->instuction[2]<<8) | c->instuction[1]) + c->X ];
		c->PC+=2;
			break;

	}
	if (!c->Y) c->P |= ZERO_FLAG;
	else c->P &= ~ZERO_FLAG;

	if ( ((signed char)c->Y) <0) c->P |= NEGATIVE_FLAG;
	else c->P &= ~NEGATIVE_FLAG;
}

void CPU::STA(CPU* c)
{
	uint8_t addr = ( c->instuction[0] &0x1c )>>2;

	switch (addr)
	{

	case 0x01: // zero page
		c->memory.Write( c->instuction[1], c->A );
		c->PC++;
			break;

	case 0x05: // zero page, X
		c->memory.Write( c->instuction[1] + c->X, c->A );
			break;

	case 0x03: // absolute
		c->memory.Write( (c->instuction[2]<<8) | c->instuction[1], c->A );
		c->PC+=2;
			break;

	case 0x07: // absolute, X
		c->memory.Write( ((c->instuction[2]<<8) | c->instuction[1]) + c->X, c->A );
		c->PC+=2;
			break;

	case 0x06: // absolute, Y
		c->memory.Write( ((c->instuction[2]<<8) | c->instuction[1]) + c->Y , c->A );
		c->PC+=2;
			break;

	case 0x00: // (zero page, X)
		c->memory.Write( c->memory[c->instuction[1] + c->X], c->A );
		c->PC++;
			break;

	case 0x04: // (zero page),Y
		c->memory.Write( c->memory[c->instuction[1]] + c->Y , c->A );
		c->PC++;
			break;
	}
}

void CPU::STX(CPU* c)
{
	uint8_t addr = ( c->instuction[0] &0x1c )>>2;

	switch (addr)
	{

	case 0x01: // zero page
		c->memory.Write( c->instuction[1], c->X );
		c->PC++;
			break;

	case 0x05: // zero page, X
		c->memory.Write( c->instuction[1] + c->Y, c->X );
			break;

	case 0x03: // absolute
		c->memory.Write( (c->instuction[2]<<8) | c->instuction[1], c->X );
		c->PC+=2;
			break;
	}
}

void CPU::STY(CPU* c)
{
	uint8_t addr = ( c->instuction[0] &0x1c )>>2;

	switch (addr)
	{

	case 0x01: // zero page
		c->memory.Write( c->instuction[1], c->Y );
		c->PC++;
			break;

	case 0x05: // zero page, X
		c->memory.Write( c->instuction[1] + c->X, c->Y );
			break;

	case 0x03: // absolute
		c->memory.Write( (c->instuction[2]<<8) | c->instuction[1], c->Y );
		c->PC+=2;
			break;
	}
}

void CPU::TAX(CPU* c)
{
	c->X = c->A;
	if (!c->X) c->P |= ZERO_FLAG;
	else c->P &= ~ZERO_FLAG;

	if ( ((signed char)c->X) <0) c->P |= NEGATIVE_FLAG;
	else c->P &= ~NEGATIVE_FLAG;
}

void CPU::TAY(CPU* c)
{
	c->Y = c->A;
	if (!c->Y) c->P |= ZERO_FLAG;
	else c->P &= ~ZERO_FLAG;

	if ( ((signed char)c->Y) <0) c->P |= NEGATIVE_FLAG;
	else c->P &= ~NEGATIVE_FLAG;
}

void CPU::TXA(CPU* c)
{
	c->A = c->X;
	if (!c->A) c->P |= ZERO_FLAG;
	else c->P &= ~ZERO_FLAG;

	if ( ((signed char)c->A) <0) c->P |= NEGATIVE_FLAG;
	else c->P &= ~NEGATIVE_FLAG;
}

void CPU::TYA(CPU* c)
{
	c->A = c->Y;
	if (!c->Y) c->P |= ZERO_FLAG;
	else c->P &= ~ZERO_FLAG;

	if ( ((signed char)c->Y) <0) c->P |= NEGATIVE_FLAG;
	else c->P &= ~NEGATIVE_FLAG;
}
void CPU::TSX(CPU* c)
{
	c->X = c->SP;
	if (!c->X) c->P |= ZERO_FLAG;
	else c->P &= ~ZERO_FLAG;

	if ( ((signed char)c->X) <0) c->P |= NEGATIVE_FLAG;
	else c->P &= ~NEGATIVE_FLAG;
}
void CPU::TXS(CPU* c)
{
	c->SP = c->X;
	if (!c->SP) c->P |= ZERO_FLAG;
	else c->P &= ~ZERO_FLAG;

	if ( ((signed char)c->SP) <0) c->P |= NEGATIVE_FLAG;
	else c->P &= ~NEGATIVE_FLAG;
}




// Branch
void CPU::BCC(CPU* c) // carry clear
{
	if (!(c->P & CARRY_FLAG)) c->PC += (signed char)c->instuction[1];
	c->PC++;
}
void CPU::BCS(CPU* c) // carry set
{
	if (c->P & CARRY_FLAG) c->PC += (signed char)c->instuction[1];
	c->PC++;
}
void CPU::BEQ(CPU* c) // zero set
{
	if (c->P & ZERO_FLAG) c->PC += (signed char)c->instuction[1];
	c->PC++;
}
void CPU::BMI(CPU* c) // negative set
{
	if (c->P & NEGATIVE_FLAG) c->PC += (signed char)c->instuction[1];
	c->PC++;
}
void CPU::BNE(CPU* c) // zero clear
{
	if (!(c->P & ZERO_FLAG)) c->PC += (signed char)c->instuction[1];
	c->PC++;
}
void CPU::BPL(CPU* c) // negative clear
{
	if (!(c->P & NEGATIVE_FLAG)) c->PC += (signed char)c->instuction[1];
	c->PC++;
}

// Jump to subroutine
void CPU::JSR(CPU* c) 
{
	c->memory[ 0x100 + c->SP-- ] = (c->PC+2)&0xff;
	c->memory[ 0x100 + c->SP-- ] = ((c->PC+2)>>8)&0xff;

	c->PC = ((c->instuction[2]<<8) | c->instuction[1]) -1;
}


void CPU::JMP(CPU* c) 
{
	if (c->instuction[0] == 0x4c) //absolute
	{
		c->PC = ((c->instuction[2]<<8) | c->instuction[1]) -1;
	}
	else if (c->instuction[0] == 0x6c) //indirect
	{
		log->Info("JMP indirect, fucking don't know if works");
		c->PC = c->memory[ ((c->instuction[2]<<8) | c->instuction[1]) ] -1 ;
	}
}

void CPU::DEX(CPU* c)
{
	c->X--;
	if (!c->X) c->P |= ZERO_FLAG;
	else c->P &= ~ZERO_FLAG;

	if ( ((signed char)c->X) <0) c->P |= NEGATIVE_FLAG;
	else c->P &= ~NEGATIVE_FLAG;
}

void CPU::DEY(CPU* c)
{
	c->Y--;
	if (!c->Y) c->P |= ZERO_FLAG;
	else c->P &= ~ZERO_FLAG;

	if ( ((signed char)c->Y) <0) c->P |= NEGATIVE_FLAG;
	else c->P &= ~NEGATIVE_FLAG;
}

void CPU::INX(CPU* c)
{
	c->X++;
	if (!c->X) c->P |= ZERO_FLAG;
	else c->P &= ~ZERO_FLAG;

	if ( ((signed char)c->X) <0) c->P |= NEGATIVE_FLAG;
	else c->P &= ~NEGATIVE_FLAG;
}

void CPU::INY(CPU* c)
{
	c->Y++;
	if (!c->Y) c->P |= ZERO_FLAG;
	else c->P &= ~ZERO_FLAG;

	if ( ((signed char)c->Y) <0) c->P |= NEGATIVE_FLAG;
	else c->P &= ~NEGATIVE_FLAG;
}

void CPU::INC(CPU* c)
{
	uint8_t addr = ( c->instuction[0] &0x1c )>>2;
	uint8_t tmp = 0;

	switch (addr)
	{

	case 0x01: // zero page
		tmp = c->memory[ c->instuction[1] ]+1;
		c->PC++;
			break;

	case 0x05: // zero page, X
		tmp = c->memory[ c->instuction[1] + c->X ]+1;
			break;

	case 0x03: // absolute
		tmp = c->memory[ (c->instuction[2]<<8) | c->instuction[1] ]+1;
		c->PC+=2;
			break;

	case 0x07: // absolute, X
		tmp = c->memory[ (c->instuction[2]<<8) | c->instuction[1] + c->X ]+1;
		c->PC+=2;
			break;
	}
	
	c->memory.Write( c->instuction[1], tmp );

	if (!tmp) c->P |= ZERO_FLAG;
	else c->P &= ~ZERO_FLAG;

	if ( ((signed char)tmp) <0) c->P |= NEGATIVE_FLAG;
	else c->P &= ~NEGATIVE_FLAG;
}


// Logic

void CPU::ORA(CPU* c)
{
	uint8_t addr = ( c->instuction[0] &0x1c )>>2;
	uint8_t tmp = 0;
	switch (addr)
	{
	case 0x02: // immediate
		tmp = c->instuction[1];
		c->PC++;
			break;

	case 0x01: // zero page
		tmp = c->memory[c->instuction[1]];
		c->PC++;
			break;

	case 0x05: // zero page, X
		tmp = c->memory[c->instuction[1] + c->X];
			break;

	case 0x03: // absolute
		tmp = c->memory[(c->instuction[2]<<8) | c->instuction[1]];
		c->PC+=2;
			break;

	case 0x07: // absolute, X
		tmp = c->memory[(c->instuction[2]<<8) | c->instuction[1] + c->X ];
		c->PC+=2;
			break;

	case 0x06: // absolute, Y
		tmp = c->memory[(c->instuction[2]<<8) | c->instuction[1] + c->Y ];
		c->PC+=2;
			break;

	case 0x00: // (zero page, X)
		tmp = c->memory[ c->memory[(c->instuction[2]<<8) | c->instuction[1] + c->Y ]  ];
		c->PC++;
			break;

	case 0x04: // (zero page),Y
		tmp = c->memory[ c->memory[(c->instuction[2]<<8) | c->instuction[1]  ] + c->Y ];
		c->PC++;
			break;
	}

	c->A = c->A | tmp;

	if (!c->A) c->P |= ZERO_FLAG;
	else c->P &= ~ZERO_FLAG;

	if ( ((signed char)c->A) <0) c->P |= NEGATIVE_FLAG;
	else c->P &= ~NEGATIVE_FLAG;
}


void CPU::AND(CPU* c)
{
	uint8_t addr = ( c->instuction[0] &0x1c )>>2;
	uint8_t tmp = 0;
	switch (addr)
	{
	case 0x02: // immediate
		tmp = c->instuction[1];
		c->PC++;
			break;

	case 0x01: // zero page
		tmp = c->memory[c->instuction[1]];
		c->PC++;
			break;

	case 0x05: // zero page, X
		tmp = c->memory[c->instuction[1] + c->X];
			break;

	case 0x03: // absolute
		tmp = c->memory[(c->instuction[2]<<8) | c->instuction[1]];
		c->PC+=2;
			break;

	case 0x07: // absolute, X
		tmp = c->memory[(c->instuction[2]<<8) | c->instuction[1] + c->X ];
		c->PC+=2;
			break;

	case 0x06: // absolute, Y
		tmp = c->memory[(c->instuction[2]<<8) | c->instuction[1] + c->Y ];
		c->PC+=2;
			break;

	case 0x00: // (zero page, X)
		tmp = c->memory[ c->memory[(c->instuction[2]<<8) | c->instuction[1] + c->Y ]  ];
		c->PC++;
			break;

	case 0x04: // (zero page),Y
		tmp = c->memory[ c->memory[(c->instuction[2]<<8) | c->instuction[1]  ] + c->Y ];
		c->PC++;
			break;
	}

	c->A = c->A | tmp;

	if (!c->A) c->P |= ZERO_FLAG;
	else c->P &= ~ZERO_FLAG;

	if ( ((signed char)c->A) <0) c->P |= NEGATIVE_FLAG;
	else c->P &= ~NEGATIVE_FLAG;
}

void CPU::ASL(CPU* c)
{
	uint8_t addr = ( c->instuction[0] &0x1c )>>2;
	uint8_t old_ = 0;
	uint8_t new_ = 0;

	switch (addr)
	{
	case 0x02: // Accumulator
		old_ = c->A;
		c->A = c->A<<1;
		new_ = c->A;
			break;

	case 0x01: // zero page
		old_ = c->memory[ c->instuction[1] ];
		c->memory[ c->instuction[1] ] = c->memory[ c->instuction[1] ]<<1;
		new_ = c->memory[ c->instuction[1] ];
		c->PC++;
			break;

	case 0x05: // zero page, X
		old_ = c->memory[ c->instuction[1] + c->X ];
		c->memory[ c->instuction[1] + c->X ] = c->memory[ c->instuction[1] + c->X ]<<1;
		new_ = c->memory[ c->instuction[1] + c->X ];
			break;

	case 0x03: // absolute
		old_ = c->memory[ (c->instuction[2]<<8) | c->instuction[1]] ;
		c->memory[ (c->instuction[2]<<8) | c->instuction[1] ] = c->memory[ (c->instuction[2]<<8) | c->instuction[1] ]<<1;
		new_ = c->memory[ (c->instuction[2]<<8) | c->instuction[1]] ;
		c->PC+=2;
			break;

	case 0x07: // absolute, X
		old_ = c->memory[ (c->instuction[2]<<8) | c->instuction[1] + c->X ];
		c->memory[ (c->instuction[2]<<8) | c->instuction[1] + c->X ] = c->memory[ (c->instuction[2]<<8) | c->instuction[1] + c->X ]<<1;
		new_ = c->memory[ (c->instuction[2]<<8) | c->instuction[1] + c->X ];
		c->PC+=2;
			break;
	}
	
	if (old_&0x80) c->P |= CARRY_FLAG;
	else c->P &= ~CARRY_FLAG;

	if (old_&0x80) c->P |= NEGATIVE_FLAG;
	else c->P &= ~NEGATIVE_FLAG;

	if (new_) c->P |= ZERO_FLAG;
	else c->P &= ~ZERO_FLAG;

}

void CPU::LSR(CPU* c)
{
	uint8_t addr = ( c->instuction[0] &0x1c )>>2;
	uint8_t old_ = 0;
	uint8_t new_ = 0;

	switch (addr)
	{
	case 0x02: // Accumulator
		old_ = c->A;
		c->A = c->A>>1;
		new_ = c->A;
			break;

	case 0x01: // zero page
		old_ = c->memory[ c->instuction[1] ];
		c->memory[ c->instuction[1] ] = c->memory[ c->instuction[1] ]>>1;
		new_ = c->memory[ c->instuction[1] ];
		c->PC++;
			break;

	case 0x05: // zero page, X
		old_ = c->memory[ c->instuction[1] + c->X ];
		c->memory[ c->instuction[1] + c->X ] = c->memory[ c->instuction[1] + c->X ]>>1;
		new_ = c->memory[ c->instuction[1] + c->X ];
			break;

	case 0x03: // absolute
		old_ = c->memory[ (c->instuction[2]<<8) | c->instuction[1]] ;
		c->memory[ (c->instuction[2]<<8) | c->instuction[1] ] = c->memory[ (c->instuction[2]<<8) | c->instuction[1] ]>>1;
		new_ = c->memory[ (c->instuction[2]<<8) | c->instuction[1]] ;
		c->PC+=2;
			break;

	case 0x07: // absolute, X
		old_ = c->memory[ (c->instuction[2]<<8) | c->instuction[1] + c->X ];
		c->memory[ (c->instuction[2]<<8) | c->instuction[1] + c->X ] = c->memory[ (c->instuction[2]<<8) | c->instuction[1] + c->X ]>>1;
		new_ = c->memory[ (c->instuction[2]<<8) | c->instuction[1] + c->X ];
		c->PC+=2;
			break;
	}
	
	if (old_&1) c->P |= CARRY_FLAG;
	else c->P &= ~CARRY_FLAG;

	if (new_&0x80) c->P |= NEGATIVE_FLAG;
	else c->P &= ~NEGATIVE_FLAG;

	if (new_) c->P |= ZERO_FLAG;
	else c->P &= ~ZERO_FLAG;

}

// Stack

void CPU::PLA(CPU* c) 
{
	c->A = c->memory[ 0x100 + ++c->SP ];
}

void CPU::PHA(CPU* c) 
{
	c->memory[ 0x100 + c->SP-- ] = c->A;
}
// Compare

void CPU::CMP(CPU* c)
{
	uint8_t addr = ( c->instuction[0] &0x1c )>>2;
	uint16_t tmp = 0;
	switch (addr)
	{
	case 0x02: // Immediate
		tmp = c->A - c->instuction[1];
		c->PC++;
			break;

	case 0x01: // zero page
		tmp = c->A - c->memory[ c->instuction[1] ];
		c->PC++;
			break;

	case 0x05: // zero page, X
		tmp = c->A - c->memory[ c->instuction[1] + c->X ];
		c->PC++;
			break;

	case 0x03: // absolute
		tmp = c->A - c->memory[ (c->instuction[2]<<8) | c->instuction[1] ];
		c->PC+=2;
			break;

	case 0x07: // absolute, X
		tmp = c->A - c->memory[ (c->instuction[2]<<8) | c->instuction[1] + c->X ];
		c->PC+=2;
			break;

	case 0x06: // absolute, Y
		tmp = c->A - c->memory[ (c->instuction[2]<<8) | c->instuction[1] + c->Y ];
		c->PC+=2;
			break;

	case 0x00: // (zero page, X)
		tmp = c->A - c->memory[  c->memory[ (c->instuction[2]<<8) | c->instuction[1] + c->X ] ];
		c->PC++;
			break;

//??????????????????????????????????????????????
	case 0x04: // (zero page),Y
		tmp = c->A - c->memory[  c->memory[ (c->instuction[2]<<8) | c->instuction[1] ] + c->X ] ;
		c->PC++;
			break;
	}
	if (!tmp) c->P |= ZERO_FLAG;
	else c->P &= ~ZERO_FLAG;

	if ( ((signed char)tmp) <0) c->P |= NEGATIVE_FLAG;
	else c->P &= ~NEGATIVE_FLAG;

	if ( tmp > 255 ) c->P &= ~CARRY_FLAG;
	else c->P |= CARRY_FLAG;
}

void CPU::CPX(CPU* c)
{
	uint8_t addr = ( c->instuction[0] &0x1c )>>2;
	uint16_t tmp = 0;
	switch (addr)
	{
	case 0x00: // Immediate
		tmp = c->X - c->instuction[1];
		c->PC++;
			break;

	case 0x01: // zero page
		tmp = c->X - c->memory[ c->instuction[1] ];
		c->PC++;
			break;

	case 0x03: // absolute
		tmp = c->X - c->memory[ (c->instuction[2]<<8) | c->instuction[1] ];
		c->PC+=2;
			break;
	}
	if (!tmp) c->P |= ZERO_FLAG;
	else c->P &= ~ZERO_FLAG;

	if ( ((signed char)tmp) <0) c->P |= NEGATIVE_FLAG;
	else c->P &= ~NEGATIVE_FLAG;

	if ( tmp > 255 ) c->P &= ~CARRY_FLAG;
	else c->P |= CARRY_FLAG;
}

void CPU::CPY(CPU* c)
{
	uint8_t addr = ( c->instuction[0] &0x1c )>>2;
	uint16_t tmp = 0;
	switch (addr)
	{
	case 0x00: // Immediate
		tmp = c->Y - c->instuction[1];
		c->PC++;
			break;

	case 0x01: // zero page
		tmp = c->Y - c->memory[ c->instuction[1] ];
		c->PC++;
			break;

	case 0x03: // absolute
		tmp = c->Y - c->memory[ (c->instuction[2]<<8) | c->instuction[1] ];
		c->PC+=2;
			break;
	}
	if (!tmp) c->P |= ZERO_FLAG;
	else c->P &= ~ZERO_FLAG;

	if ( ((signed char)tmp) <0) c->P |= NEGATIVE_FLAG;
	else c->P &= ~NEGATIVE_FLAG;

	if ( tmp > 255 ) c->P &= ~CARRY_FLAG;
	else c->P |= CARRY_FLAG;
}

void CPU::BIT(CPU* c)
{
	uint8_t addr = ( c->instuction[0] &0x1c )>>2;
	uint16_t tmp = 0;
	uint8_t val = 0;
	switch (addr)
	{
	case 0x01: // zero page
		val = c->memory[ c->instuction[1] ];
		c->PC++;
		break;
	case 0x03: // absolute
		val = c->memory[ (c->instuction[2]<<8) | c->instuction[1] ];
		c->PC+=2;
		break;
	}

	tmp = c->A & val;

	if (!tmp) c->P |= ZERO_FLAG;
	else c->P &= ~ZERO_FLAG;


	if ( val&0x80 ) c->P |= NEGATIVE_FLAG;
	else c->P &= ~NEGATIVE_FLAG;

	if ( val&0x40 ) c->P &= ~OVERFLOW_FLAG;
	else c->P |= OVERFLOW_FLAG;
}


void CPU::UNK(CPU* c)
{
	log->Debug("0x%x: Unknown instruction (0x%x), halting!", c->PC, c->instuction[0]);
	for(;;)
	{
		Sleep(1);
	}
}