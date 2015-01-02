#pragma once
#include "headers.h"
#include <SDL.h>

#define HORIZONTAL 0
#define VERTICAL 1

struct Palette_entry
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
};
//http://nesdev.parodius.com/nespal.txt
static struct Palette_entry nes_palette[64] =
{
	{0x6D,0x6D,0x6D}, {0x00,0x24,0x92}, {0x00,0x00,0xDB}, {0x6D,0x49,0xDB},
	{0x92,0x00,0x6D}, {0xB6,0x00,0x6D}, {0xB6,0x24,0x00}, {0x92,0x49,0x00},
	{0x6D,0x49,0x00}, {0x24,0x49,0x00}, {0x00,0x6D,0x24}, {0x00,0x92,0x00},
	{0x00,0x49,0x49}, {0x00,0x00,0x00}, {0x00,0x00,0x00}, {0x00,0x00,0x00},
	{0xB6,0xB6,0xB6}, {0x00,0x6D,0xDB}, {0x00,0x49,0xFF}, {0x92,0x00,0xFF},
	{0xB6,0x00,0xFF}, {0xFF,0x00,0x92}, {0xFF,0x00,0x00}, {0xDB,0x6D,0x00},
	{0x92,0x6D,0x00}, {0x24,0x92,0x00}, {0x00,0x92,0x00}, {0x00,0xB6,0x6D},
	{0x00,0x92,0x92}, {0x24,0x24,0x24}, {0x00,0x00,0x00}, {0x00,0x00,0x00},
	{0xFF,0xFF,0xFF}, {0x6D,0xB6,0xFF}, {0x92,0x92,0xFF}, {0xDB,0x6D,0xFF},
	{0xFF,0x00,0xFF}, {0xFF,0x6D,0xFF}, {0xFF,0x92,0x00}, {0xFF,0xB6,0x00},
	{0xDB,0xDB,0x00}, {0x6D,0xDB,0x00}, {0x00,0xFF,0x00}, {0x49,0xFF,0xDB},
	{0x00,0xFF,0xFF}, {0x49,0x49,0x49}, {0x00,0x00,0x00}, {0x00,0x00,0x00},
	{0xFF,0xFF,0xFF}, {0xB6,0xDB,0xFF}, {0xDB,0xB6,0xFF}, {0xFF,0xB6,0xFF},
	{0xFF,0x92,0xFF}, {0xFF,0xB6,0xB6}, {0xFF,0xDB,0x92}, {0xFF,0xFF,0x49},
	{0xFF,0xFF,0x6D}, {0xB6,0xFF,0x49}, {0x92,0xFF,0x6D}, {0x49,0xFF,0xDB},
	{0x92,0xDB,0xFF}, {0x92,0x92,0x92}, {0x00,0x00,0x00}, {0x00,0x00,0x00}
};

struct SPRITE 
{
	uint8_t y;
	uint8_t index;
	uint8_t attr;
	uint8_t x;
};

class PPU
{
private:
	uint32_t cycles;
	int16_t scanline;
	bool VBLANK;
	bool NMI_enabled;
	uint8_t VRAMaddressIncrement;

	bool ShowBackground;
	bool ShowSprites;
	bool Sprite0Hit;

	uint16_t loopy_v; // 15bits, current VRAM address
	uint16_t loopy_t; // 15bits, temporary VRAM address
	uint8_t  loopy_x; // 3bits, fine X scroll
	bool     loopy_w; // First or second write toggle (PPUADDRhalf)

	uint8_t PPUDATAbuffer;

	uint8_t screen[256][256]; // raw screen data, no palette lookup, [y][x]
	void PaletteLookup(SDL_Surface *s);

	void RenderSprite(SDL_Surface* s);
	void RenderBackground(SDL_Surface* s, uint8_t nametable);
public:
	bool SpriteSize; // false - 8x8, true - 8x16
	uint16_t BaseNametable;
	uint16_t BackgroundPattenTable;
	uint16_t SpritePattenTable;
	uint8_t Mirroring;
	uint8_t OAMADDR;
	uint8_t memory[0x4000]; // 16KB
	SPRITE OAM[64]; // 64B*4
	PPU(void);
	~PPU(void);

	// CPU interface
	void Write( uint8_t reg, uint8_t data );
	uint8_t Read( uint8_t reg);
	uint8_t Step();
	
	void Render(SDL_Surface* s);

	// Loopy
	inline bool renderingIsEnabled();
	inline void loopyCopyTtoV();
	inline void loopyCoarseXIncrement();
	inline void loopyYIncrement();
	inline void loopyCopyHorizontal();
};
