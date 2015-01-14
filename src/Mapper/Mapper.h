#pragma once
#include <vector>
#include "../PPU.h"
class Mapper
{
protected:
	std::vector<uint8_t> prg, chr;
	int prgPages, chrPages;
	PPU& ppu;
public:
	Mapper( PPU& ppu );
	~Mapper();
	virtual uint8_t Read(uint16_t n) = 0;
	virtual void Write(uint16_t n, uint8_t data) = 0;

	void setPrg(const std::vector<uint8_t> &prg);
	void setChr(const std::vector<uint8_t> &chr);
};

