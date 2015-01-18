#include "PPU.h"


PPU::PPU(void)
{
	memset( this->memory, 0, 0x4000 );
	memset( this->memory+0x2000 , 0xFF, 0xc00 );
	memset( this->OAM , 0, 0xff );

	// Startup palette according to blargg tests
	
    memcpy( this->memory+0x3f00, "\x09,\x01,\x00,\x01,\x00,\x02,\x02,\x0D,\x08,\x10,\x08,\x24,\x00,\x00,\x04,\x2C,\x09,\x01,\x34,\x03,\x00,\x04,\x00,\x14,\x08,\x3A,\x00,\x02,\x00,\x20,\x2C,\x08", 32 );

	writeIgnored = true;
	frameCounter = 0;

	cycles = 0;
	scanline = -1;
	NMI_enabled = false;
	VBLANK = false;
	SpriteSize = false;
	
	ShowBackground = false;
	ShowSprites = false;
	spriteOverflow = false;
	showLeftSprites = false;
	showLeftBackground = false;

	PPUDATAbuffer = 0;
	BackgroundPattenTable = 0;
	SpritePattenTable = 0;
	BaseNametable = 0x2000;
	VRAMaddressIncrement = 1;
	OAMADDR = 0;

	loopy_v = 0;
	loopy_t = 0;
	loopy_x = 0;
	loopy_w = 0;
	Log->Debug("PPU: created");
}

PPU::~PPU(void)
{
	Log->Debug("PPU: destroyed");
}

void PPU::Write( uint8_t reg, uint8_t data )
{
	uint16_t addr;
	switch (reg+0x2000)
	{
		case 0x2000: //PPUCTRL
			if (writeIgnored) break;
			// 7 - NMI on VBLANK
			if (data&0x80) NMI_enabled = true;
			else NMI_enabled = false;

			// 6 - ppu slave/master, ignore

			// 5 - Sprite size (0 - 8x8, 1 - 8x16)
			if (data&0x20) SpriteSize = true;
			else SpriteSize = false;	

			// 4 - Background pattern table address (0: $0000; 1: $1000)
			if (data&0x10) BackgroundPattenTable = 0x1000;
			else BackgroundPattenTable = 0;
			
			// 3 - Sprite pattern table address for 8x8 sprites (0: $0000; 1: $1000; ignored in 8x16 mode)
			if (data&0x08) SpritePattenTable = 0x1000;
			else SpritePattenTable = 0;

			// 2 - VRAM address increment per CPU read/write of PPUDATA 
			//     (0: increment by 1, going across; 1: increment by 32, going down)
			if (data&0x04) VRAMaddressIncrement = 32;
			else VRAMaddressIncrement = 1;
			
			// 1:0 - Base nametable address
			//     (0 = $2000; 1 = $2400; 2 = $2800; 3 = $2C00)
			BaseNametable = 0x2000 + 0x400*(data&0x03);

			loopy_t = (loopy_t & ~0xc00) | ((data & 0x03) << 10); // t: ...BA.. ........ = d: ......BA
			break;
			
		case 0x2001: //PPUMASK
			// 7 - Intensify blues (and darken other colors)
			// 6 - Intensify greens (and darken other colors)
			// 5 - Intensify reds (and darken other colors)
			// 4 - 1: Show sprites
			// 3 - 1: Show background
			// 2 - 1: Show sprites in leftmost 8 pixels of screen; 0: Hide
			// 1 - 1: Show background in leftmost 8 pixels of screen; 0: Hide
			// 0 - Grayscale (0: normal color; 1: produce a monochrome display), ignored

			if (data&0x10) ShowSprites = true;
			else ShowSprites = false;

			if (data&0x08) ShowBackground = true;
			else ShowBackground = false;

			if (data & 0x04) showLeftSprites = true;
			else showLeftSprites = false;

			if (data & 0x02) showLeftBackground = true;
			else showLeftBackground = false;

			break;

		case 0x2002: //PPUSTATUS
			break;
			
		case 0x2003: //OAMADDR
			if (writeIgnored) break;
			OAMADDR = data;
			break;
			
		case 0x2004: //OAMDATA
			if (writeIgnored) break;
			*((uint8_t *)OAM+OAMADDR) = data;
			OAMADDR++;
			break;
			
		case 0x2005: //PPUSCROLL
			if (loopy_w == 0) // PPUADDRhalf == 1
			{
				loopy_t = (loopy_t & ~0x1f) | ((data & 0xf8)>>3); // t: ....... ...HGFED = d: HGFED...
				loopy_x = data & 0x7; // x:              CBA = d: .....CBA
				loopy_w = 1;
			}
			else // PPUADDRhalf == 0
			{
				loopy_t = (loopy_t & ~0x73e0) | 
					   ((data & 0x7) << 12) |
					   ((data & 0xf8)<<  2); // t: CBA..HG FED..... = d: HGFEDCBA
				loopy_w = 0;
			}
			break;

		case 0x2006: //PPUADDR
			// Access to PPU memory from CPU, write x2
			if (loopy_w == 0) // PPUADDRhalf == 1
			{
				loopy_t = (loopy_t & ~0x7f00) | ((data & 0x3f) << 8); // t: .FEDCBA ........ = d: ..FEDCBA
				loopy_w = 1;
			}
			else
			{
				loopy_t = (loopy_t & ~0xff) | data;
				loopy_v = loopy_t;
				loopy_w = 0;
			}
			break;
			
		case 0x2007: //PPUDATA
			if (writeIgnored) break;
			// Access to PPU memory from CPU
			addr = loopy_v;

			if (addr >= 0x3000 && addr <= 0x3eff) addr -= 0x1000;
			if (addr >= 0x3f00 && addr <= 0x3fff) //Palette
			{
				uint16_t tmpaddr = (addr - 0x3f00) % 0x20;

				if (tmpaddr == 0x10 || tmpaddr == 0x14 || tmpaddr == 0x18 || tmpaddr == 0x1c) 
					memory[0x3f00 + tmpaddr - 0x10] = data;
				else memory[0x3f00 + tmpaddr] = data;
			}
			else {
				if (addr >= 0x2000)
				{
					// Rad racer fix, Single screen game fix
					if (Mirroring == Mirroring::Vertical) addr &= ~0x0800;
					else if (Mirroring == Mirroring::Horizontal) addr &= ~0x0400;
					else if (Mirroring == Mirroring::ScreenA) addr &= ~0x0C00;
					else if (Mirroring == Mirroring::ScreenB) addr = (addr & ~0x0C00) | 0x0400;
				}
				memory[addr] = data;
			}

			if (scanline < 240 && (ShowBackground || ShowSprites)) Log->Info("Update of loopy_v during rendering");
			loopy_v += VRAMaddressIncrement;
			break;
	}
}

uint8_t PPU::Read( uint8_t reg)
{
	int ret = 0;
	int16_t addr;
	switch (reg+0x2000)
	{
		case 0x2002: //PPUSTATUS
			ret = (VBLANK << 7) | (Sprite0Hit << 6) | (spriteOverflow << 5);
			// 7 - Vertical blank has started (0: not in VBLANK; 1: in VBLANK)

			// 6 - Sprite 0 Hit.  Set when a nonzero pixel of sprite 0 'hits', ignored
			//     a nonzero background pixel.  Used for raster timing.

			// 5 - Sprite overflow. The PPU can handle only eight sprites on one
			//     scanline and sets this bit if it starts dropping sprites.
			//     Normally, this triggers when there are 9 sprites on a scanline,
			//     but the actual behavior is significantly more complicated.

			// 4:0 - Least significant bits previously written into a PPU register
			//       (due to register not being updated for this address)

			VBLANK = false;
			loopy_w = 0;
			break;
						
		case 0x2004: //OAMDATA
			ret = *((uint8_t *)OAM+OAMADDR);
			if (((OAMADDR) % 4) == 2) ret = ret & 0xE3;
			break;
			
		case 0x2007: //PPUDATA
			// Access to PPU memory from CPU,
			addr = loopy_v;// (PPUADDRhi << 8) | PPUADDRlo;
			addr = addr%0x4000;

			/* Note 
			   When reading PPUDATA at 0-0x3EFF PPU returns buffer value - not current
			   value at memory. You just return buffer, then read memory to buffer
			   and increment address
		    */
			if (addr >= 0x3000 && addr <= 0x3eff) addr -= 0x1000;
			if (addr < 0x3EFF)
			{
				ret = PPUDATAbuffer;
				PPUDATAbuffer = memory[ addr ];
			}
			else
			{
				uint16_t tmpaddr = (addr - 0x3f00) % 0x20;

				if (tmpaddr == 0x10 || tmpaddr == 0x14 || tmpaddr == 0x18 || tmpaddr == 0x1c)
					ret = memory[0x3f00 + tmpaddr - 0x10];
				else ret = memory[0x3f00 + tmpaddr];
			}

			loopy_v += VRAMaddressIncrement;
			break;

		default:
			break;
	}
	return ret;
}

uint8_t PPU::Step( )
{
	uint8_t ret = 0;

	if (scanline == -1) // Pre-render scanline (or 261)
	{
		if (cycles == 1)
		{
			VBLANK = false;
			Sprite0Hit = false;
			spriteOverflow = false;
		}
		else if (cycles >= 280 && cycles <= 304 && renderingIsEnabled())
			loopyCopyTtoV();
	}

	static uint16_t backgroundDatal, backgroundDatah;
	/*
	aaaaaaaa bbbbbbbb
	01234567 89abcdef
	^
	This byte is outputed
	and register is shifted left <<
	*/
	static SPRITE secondOAM[8], oldSecondOAM[8];
	static int secondOAMsprites, oldSecondOAMsprites;
	static uint8_t paletteDatal, paletteDatah;
	static bool paletteDataLatchl, paletteDataLatchh;

	if (renderingIsEnabled() && scanline < 240)
	{
		if (ShowSprites && cycles == 0)
		{
			memcpy(oldSecondOAM, secondOAM, sizeof(oldSecondOAM));
			oldSecondOAMsprites = secondOAMsprites;
		}
		if ((cycles >= 1 && cycles <= 256) || (cycles >= 321 && cycles <= 336)) // Tile fetching
		{
			static uint8_t newBackgroundDatal, newBackgroundDatah;
			static uint8_t newPaletteData;
			static uint8_t NTbyte, ATbyte;
			uint8_t step = ((cycles - 1) % 8) + 1;

			uint16_t v = loopy_v;
			if (Mirroring == Mirroring::Vertical) v &= ~0x0800; // Clear second bit (y)
			else if (Mirroring == Mirroring::Horizontal) v &= ~0x0400; //  Clear first bit (x), Horizontal
			else if (Mirroring == Mirroring::ScreenA) v = (loopy_v & ~0x0C00);// | 0x0400;
			else if (Mirroring == Mirroring::ScreenB) v = (loopy_v & ~0x0C00) | 0x0400;

			if (step == 1) // Cycle 1 and 2, NT byte
			{
				// Copy new data
				backgroundDatal = (backgroundDatal & 0xff00) | newBackgroundDatal;
				backgroundDatah = (backgroundDatah & 0xff00) | newBackgroundDatah;

				paletteDataLatchl = (newPaletteData & 1);
				paletteDataLatchh = ((newPaletteData & 2) >> 1);

				uint16_t tileaddr = 0x2000 | (v & 0x0fff);
				NTbyte = memory[tileaddr];
			}
			else if (step == 3) // Cycle 3 and 4, AT byte
			{
				uint16_t attraddr = 0x23c0 | (v & 0x0c00) | ((v >> 4) & 0x38) | ((v >> 2) & 0x07);
				ATbyte = memory[attraddr];
			}
			else if (step == 5) // Cycle 5 and 6, low BG tile byte
			{
				uint8_t fineY = ((v & 0x7000) >> 12);

				newBackgroundDatal &= ~0xff;
				newBackgroundDatal |= memory[BackgroundPattenTable + NTbyte * 16 + fineY];
			}
			else if (step == 7) // Cycle 7 and 8, high BG tile byte
			{
				uint8_t fineY = ((v & 0x7000) >> 12);

				newBackgroundDatah &= ~0xff;
				newBackgroundDatah |= memory[BackgroundPattenTable + NTbyte * 16 + fineY + 8];
			}
			else if (step == 8) // combine palette data
			{
				uint8_t coarseX = (v & 0x1f);
				uint8_t coarseY = (v >> 5) & 0x1f;

				uint8_t shift = ((coarseY & 0x2) * 2) + (coarseX & 0x2);
				newPaletteData = (ATbyte >> shift);
			}

			if (cycles == 64 && ShowSprites) // <1,64> second OAM clear
				memset(&secondOAM, 0xff, sizeof(secondOAM));
			if (cycles == 256) // <65,256> second OAM evaluation
			{
				secondOAMsprites = 0;
				if (scanline != -1 && ShowSprites)
				{
					int j = 0;
					for (int i = 0; i < 64; i++)
					{
						SPRITE &spr = OAM[i];
						if (!(scanline >= spr.y && scanline < spr.y + (SpriteSize ? 16 : 8))) continue;
						secondOAM[j] = spr;
						if (i == 0) secondOAM[j].attr |= 4;
						j++;
						if (j == 9)
						{
							spriteOverflow = true;
							break;
						}
					}
					secondOAMsprites = j;
				}
			}
		}

		if (scanline >= 0 && cycles >= 1 && cycles <= 256)
		{
			uint8_t renderX = cycles - 1;
			uint8_t renderY = scanline;
			uint8_t BackgroundByte = memory[0x3F00];
			uint8_t backgroundColor = 0;
			uint8_t SpriteByte = 0;
			bool SpriteRendered = false;
			bool SpriteFront = false;

			if (ShowBackground && (showLeftBackground || renderX >= 8))
			{
				uint8_t color = (((backgroundDatal << loopy_x) & 0x8000) >> 15) |
					(((backgroundDatah << loopy_x) & 0x8000) >> 14);

				uint8_t pal = (((paletteDatal << loopy_x) & 0x80) >> 7) |
					(((paletteDatah << loopy_x) & 0x80) >> 6);

				backgroundColor = color;
				if (color == 0) pal = 0;
				BackgroundByte = memory[0x3F00 + (pal * 4) + color];
			}
			
			if (ShowSprites && (showLeftSprites || renderX >= 8))
			{
				for (int i = 0; i < oldSecondOAMsprites; i++) // OAM
				{
					SPRITE &spr = oldSecondOAM[i];
					if (!(renderX >= spr.x && renderX < spr.x + 8)) continue;

					SpriteFront = (spr.attr & 0x20) ? false : true;

					uint16_t spriteaddr = (SpriteSize ? (spr.index & 0x01) * 0x1000 : SpritePattenTable) + 
										  (SpriteSize ? (spr.index & 0xfe) : spr.index) * 16;
					
					uint8_t sprite_x = renderX - (spr.x); // x inside sprite
					uint8_t sprite_y = renderY - (spr.y + 1); // y inside sprite

					if (spr.attr & 0x80) sprite_y = (SpriteSize ? 15 : 7) - sprite_y; //Vertical flip
					if (sprite_y>7) sprite_y += 8;

					uint8_t spritedata = memory[spriteaddr + sprite_y];
					uint8_t spritedata2 = memory[spriteaddr + sprite_y + 8];

					if (spr.attr & 0x40) sprite_x = 7 - sprite_x; //Horizontal flip

					uint8_t c1 = (spritedata  &(1 << (7 - sprite_x))) ? 1 : 0;
					uint8_t c2 = (spritedata2 &(1 << (7 - sprite_x))) ? 1 : 0;

					uint8_t color = c1 | c2 << 1;
					if (color == 0) continue;

					SpriteByte = memory[0x3F10 + ((spr.attr & 0x3) * 4) + color];
					SpriteRendered = true;

					if ((spr.attr & 4) &&
						renderX != 255 &&
						ShowBackground &&
						color != 0 &&
						backgroundColor != 0 && 
						(showLeftBackground || renderX >= 8))
						Sprite0Hit = true;
					if (SpriteFront) break;
					
				}
			}
			if (SpriteRendered)
			{
				if (SpriteFront || !ShowBackground) screen[renderY][renderX] = SpriteByte;
				else
				{
					if (BackgroundByte == memory[0x3f00])
						screen[renderY][renderX] = SpriteByte;
					else
						screen[renderY][renderX] = BackgroundByte;
				}
			}
			else
				screen[renderY][renderX] = BackgroundByte;
		}

		if ((cycles >= 1 && cycles <= 256) || (cycles >= 321 && cycles <= 336)) // shift register shifting
		{
			backgroundDatal <<= 1;
			backgroundDatah <<= 1;
			paletteDatal = (paletteDatal << 1) | paletteDataLatchl;
			paletteDatah = (paletteDatah << 1) | paletteDataLatchh;

			if ((cycles % 8) == 0) loopyCoarseXIncrement();
		}
		if (cycles == 256) // Increment vertical position in v
			loopyYIncrement();
		if (cycles == 257) // Copy horizonal pos from t to v
			loopyCopyHorizontal();
	}
	if (scanline == 241 && cycles == 1) // Vblank
	{
		VBLANK = true;
		if (NMI_enabled) ret = 2;
		else ret = 1;
	}

	// 1 CPU cycles - 3 PPU cycles
	// 1 scanline - 341 PPU cycles
	if (++cycles > 340)
	{
		cycles = 0;
		if (++scanline > 260) {
			scanline = -1;
			frameCounter++;
			if (frameCounter == 1) 
				writeIgnored = false;
		}
	}
	return ret;
}

void PPU::Render(SDL_Texture* s)
{
	uint8_t *pixels = nullptr;
	int pitch = 0;
	SDL_LockTexture(s, nullptr, (void**)&pixels, &pitch);
	if (pitch != 0x400 || pixels == nullptr) {
		Log->Error("Cannot lock texture for rendering");
		return;
	}
	PaletteLookup(pixels);
	SDL_UnlockTexture(s);

	memset(screen, 63, 256 * 256); // Not the best solution?
}


void PPU::PaletteLookup(Uint8 *PIXEL)
{
	for (int y = 0; y < 240; y++)
	{
		for (int x = 0; x < 256; x++, PIXEL+=4)
		{
			Palette_entry e = nes_palette[screen[y][x]];

			*(PIXEL + 0) = e.b;
			*(PIXEL + 1) = e.g;
			*(PIXEL + 2) = e.r;
			*(PIXEL + 3) = 0xff;
		}
	}
}


// Loopy
//
//bool PPU::renderingIsEnabled()
//{
//	return (ShowSprites || ShowBackground);
//}

void PPU::loopyCopyTtoV()
{
	loopy_v = (loopy_v & ~0x7be0) | (loopy_t & 0x7BE0);
}

void PPU::loopyCoarseXIncrement()
{
	if ((loopy_v & 0x001f) == 0x1f)
	{
		loopy_v &= ~0x1f;
		loopy_v ^= 0x0400; // Flip page bit
	}
	else
		loopy_v++;
}

void PPU::loopyYIncrement()
{
	if ((loopy_v & 0x7000) == 0x7000) // Fine y bits overflow
	{
		loopy_v &= ~0x7000;
		int y = (loopy_v & 0x3e0) >> 5;  // Coarse y bits
		if (y == 29)
		{
			y = 0;
			loopy_v ^= 0x800;
		}
		else if (y == 31)
			y = 0;
		else
			y++;

		loopy_v = (loopy_v & (~0x03e0)) | (y << 5);
	}
	else loopy_v += 0x1000; // Increment fine ybits
}

void PPU::loopyCopyHorizontal()
{
	loopy_v = (loopy_v & ~0x41f) | (loopy_t & 0x41f);
}