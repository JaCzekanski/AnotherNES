#include "CPU_ram.h"

CPU_ram::CPU_ram(void)
{
	ZERO =  0;
	memset( this->memory, 0, 0xffff );
	ppu = NULL;
	log->Debug("CPU_ram: Created");
}

CPU_ram::~CPU_ram(void)
{
	log->Debug("CPU_ram: destroyed");
}

void CPU_ram::Load( uint16_t dst, const void* source, size_t num )
{
	uint8_t* _source = (uint8_t*)source;
	if (dst+num>0xffff)
	{
		log->Fatal("CPU_ram: dst+num>0xffff!");
		return;
	}
	for (int i = dst; i<dst+num; i++)
	{
		memory[i] = *_source++;
	}
	log->Debug("CPU_ram: loaded");
}