#pragma once
#include "headers.h"
#include <SDL.h>

class PPU
{
private:
	uint32_t cycles;
	uint16_t scanline;
	bool VBLANK;
	bool NMI_enabled;
	uint16_t BackgroundPattenTable;
	uint16_t SpritePattenTable;
	uint8_t VRAMaddressIncrement;
	uint16_t BaseNametable;

	bool PPUADDRhalf;
	uint8_t PPUADDRhi;
	uint8_t PPUADDRlo;
public:
	uint8_t memory[0x4000]; // 16KB
	PPU(void);
	~PPU(void);

	// CPU interface
	void Write( uint8_t reg, uint8_t data );
	uint8_t Read( uint8_t reg);
	uint8_t Step();
	
	void Render(SDL_Surface* s);
};
