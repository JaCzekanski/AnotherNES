#include "CPU_ram.h"

CPU_ram::CPU_ram(void)
{
	ZERO =  0;
	mapper = 0;
	prg_highpage = 0;
	prg_lowpage = 0;
	prg_pages = 0;
	memset( this->memory, 0, 0xffff );
	this->memory[0x0008] = 0xf7;
	this->memory[0x0009] = 0xef;
	this->memory[0x000a] = 0xdf;
	this->memory[0x000f] = 0xbf;

	memset(this->memoryLock, 0, 2048);

	ppu = NULL;
	Log->Debug("CPU_ram: created");
}

CPU_ram::~CPU_ram(void)
{
	Log->Debug("CPU_ram: destroyed");
}

void CPU_ram::Load( uint16_t dst, const void* source, size_t num )
{
	uint8_t* _source = (uint8_t*)source;
	if (dst+num>0xffff)
	{
		Log->Fatal("CPU_ram: dst+num>0xffff!");
		return;
	}
	for (int i = dst; i<dst+num; i++)
	{
		memory[i] = *_source++;
	}
	Log->Debug("CPU_ram: loaded");
}