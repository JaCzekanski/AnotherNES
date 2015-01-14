#include "Mapper.h"


Mapper::Mapper(PPU &ppu) : ppu(ppu), prgPages(0), chrPages(0)
{
}

void Mapper::setPrg(const std::vector<uint8_t> &prg)
{
	this->prg = prg;
	prgPages = this->prg.size() / (16 * 1024);
}

void Mapper::setChr(const std::vector<uint8_t> &chr)
{
	this->chr = chr;
	chrPages = this->chr.size() / (8 * 1024);
}