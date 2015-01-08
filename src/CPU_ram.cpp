#include "CPU_ram.h"

CPU_ram::CPU_ram(void)
{
	ZERO =  0;
	mapper = 0;
	prg_highpage = 0;
	prg_lowpage = 0;
	prg_pages = 0;
	memset( memory, 0, 2048 );
	memory[0x0008] = 0xf7;
	memory[0x0009] = 0xef;
	memory[0x000a] = 0xdf;
	memory[0x000f] = 0xbf;

	memset(memoryLock, 0, 2048);

	ppu = NULL;
}

CPU_ram::~CPU_ram(void)
{
}

void CPU_ram::Load( uint16_t dst, const void* source, size_t num )
{
	uint8_t* _source = (uint8_t*)source;
	if (dst+num>0xffff)
	{
		Log->Fatal("CPU_ram: dst+num>0xffff!");
		return;
	}
	for (uint16_t i = dst; i<dst+num; i++)
	{
		memory[i] = *_source++;
	}
	Log->Debug("CPU_ram: loaded");
}