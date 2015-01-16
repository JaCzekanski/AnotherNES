#pragma once
#include "headers.h"
#include <SDL.h>
#include <vector>

namespace Mirroring
{
	enum Mirroring
	{
		Horizontal = 0,
		Vertical,
		FourScreen,
		ScreenA,
		ScreenB
	};
};
//#define HORIZONTAL 0
//#define VERTICAL 1

struct Palette_entry
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t dummy;
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
	bool writeIgnored; // Some games writes to PPU $2000 enabling NMI and get stuck waiting for VBLANK, but it's cleared by NMI
					  // PPU must wait 29658 (little more than one frame) ticks before it start accepts writes 
	int frameCounter;

	bool VBLANK;
	bool NMI_enabled;
	uint8_t VRAMaddressIncrement;

	bool ShowBackground;
	bool ShowSprites;
	bool Sprite0Hit;
	bool spriteOverflow;
	
	bool showLeftSprites;
	bool showLeftBackground;

	uint16_t loopy_v; // 15bits, current VRAM address
	uint16_t loopy_t; // 15bits, temporary VRAM address
	uint8_t  loopy_x; // 3bits, fine X scroll
	bool     loopy_w; // First or second write toggle (PPUADDRhalf)

	uint8_t PPUDATAbuffer;

	uint8_t screen[256][256]; // raw screen data, no palette lookup, [y][x]
	void PaletteLookup(Uint8 *PIXEL);
	
	// Loopy
	//__forceinline  bool renderingIsEnabled();
	// Unfortunately define macro is far mor faster than inline funcion (even if called only once)
	#define renderingIsEnabled() (ShowSprites || ShowBackground)
	inline void loopyCopyTtoV();
	inline void loopyCoarseXIncrement();
	inline void loopyYIncrement();
	inline void loopyCopyHorizontal();

public:
	uint32_t cycles;
	int16_t scanline;
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
	
	void Render(SDL_Texture* s);

};
