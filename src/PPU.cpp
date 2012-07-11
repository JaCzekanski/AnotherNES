#include "PPU.h"

PPU::PPU(void)
{
	memset( this->memory, 0, 0x4000 );
	memset( this->memory+0x2000 , 0xFF, 0xc00 );
	memset( this->OAM , 0, 0xff );
	cycles = 0;
	scanline = 0;
	NMI_enabled = false;
	VBLANK = true;
	SpriteSize = false;
	PPUADDRhalf = true; // True - hi, false - lo
	
	ShowBackground = false;
	ShowSprites = false;

	PPUADDRhi = 0;
	PPUADDRlo = 0;
	BackgroundPattenTable = 0;
	SpritePattenTable = 0;
	BaseNametable = 0x2000;
	VRAMaddressIncrement = 1;
	OAMADDR = 0;

	SCROLLhalf = true;
	ScrollX = 0;
	ScrollY = 0;
	log->Debug("CPU_ram: Created");
}

PPU::~PPU(void)
{
	log->Debug("CPU_ram: Destroyed");
}

void PPU::Write( uint8_t reg, uint8_t data )
{
	uint16_t addr;
	switch (reg+0x2000)
	{
		case 0x2000: //PPUCTRL
			// 7 - NMI on VBLANK
			if (data&0x80) NMI_enabled = true;
			else NMI_enabled = false;

			// 6 - ppu slave/master, ignore

			// 5 - Sprite size (0 - 8x8, 1 - 8x16), ignore
			if (data&0x20) 
			{
				if (!SpriteSize) log->Debug("PPU: 8x16");
				SpriteSize = true;
			}
			else
			{
				if (SpriteSize) log->Debug("PPU: 8x8");
				SpriteSize = false;
			}
				

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

			break;
			
		case 0x2001: //PPUMASK
			// 7 - Intensify blues (and darken other colors)
			// 6 - Intensify greens (and darken other colors)
			// 5 - Intensify reds (and darken other colors)
			// 4 - 1: Show sprites
			// 3 - 1: Show background
			// 2 - 1: Show sprites in leftmost 8 pixels of screen; 0: Hide, ignored
			// 1 - 1: Show background in leftmost 8 pixels of screen; 0: Hide, ignored
			// 0 - Grayscale (0: normal color; 1: produce a monochrome display), ignored
			if (data&0x10) ShowSprites = true;
			else ShowSprites = false;

			if (data&0x08) ShowBackground = true;
			else ShowBackground = false;
			break;
			
		case 0x2003: //OAMADDR
			//log->Debug("PPU: 0x2003 <- 0x%.2x", data);
			OAMADDR = data;
			// Object Attribute Memory (sprites), ignored
			break;
			
		case 0x2004: //OAMDATA
			log->Debug("PPU: 0x2004 <- 0x%.2x", data);
			// Object Attribute Memory (sprites), ignored
			break;
			
		case 0x2005: //PPUSCROLL
			if (SCROLLhalf) 
			{
				_ScrollX = ScrollX;
				ScrollX = data;
			}
			else 
			{
				_ScrollY = ScrollY;
				ScrollY = data;
			}
			SCROLLhalf = ! SCROLLhalf;

			if (_ScrollX!=ScrollX || _ScrollY!=ScrollY) 
			{
				log->Debug("PPU: Scroll {%.2x,%.2x}", ScrollX, ScrollY);
			}
			break;

		case 0x2006: //PPUADDR
			// Access to PPU memory from CPU, write x2
			if (PPUADDRhalf) PPUADDRhi = data;
			else PPUADDRlo = data;
			PPUADDRhalf = !PPUADDRhalf;
			break;
			
		case 0x2007: //PPUDATA
			// Access to PPU memory from CPU
			addr = (PPUADDRhi<<8) | PPUADDRlo;

			if (addr>=0x3f00 && addr<=0x3f1f) //Palette
			{
			}
			memory[ addr%0x4000 ] = data;

			addr+=VRAMaddressIncrement;

			PPUADDRhi = (addr>>8)&0xff;
			PPUADDRlo = addr&0xff;
			break;

		default:
			log->Error("CPU write to wrong PPU address");
			int a = 0;
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
			ret = (VBLANK<<7);
			// 7 - Vertical blank has started (0: not in VBLANK; 1: in VBLANK)

			// 6 - Sprite 0 Hit.  Set when a nonzero pixel of sprite 0 'hits', ignored
			//     a nonzero background pixel.  Used for raster timing.

			// 5 - Sprite overflow. The PPU can handle only eight sprites on one
			//     scanline and sets this bit if it starts dropping sprites.
			//     Normally, this triggers when there are 9 sprites on a scanline,
			//     but the actual behavior is significantly more complicated.

			// 4:0 - Least significant bits previously written into a PPU register
			//       (due to register not being updated for this address)

			// Reset PPUADDR latch to high
			PPUADDRhalf  = true;
			SCROLLhalf = true;
			break;
						
		case 0x2004: //OAMDATA
			// Object Attribute Memory (sprites), ignored
			break;
			
		case 0x2007: //PPUDATA
			// Access to PPU memory from CPU,
			addr = (PPUADDRhi<<8) | PPUADDRlo;
			ret = memory[ addr ];

			addr+=VRAMaddressIncrement;

			PPUADDRhi = (addr>>8)&0xff;
			PPUADDRlo = addr&0xff;
			break;

		default:
			//log->Error("CPU read from wrong PPU address");
			int a = 0;
			break;
	}
	return ret;
}

uint8_t PPU::Step( )
{
	// 1 CPU cycles - 3 PPU cycles
	// 1 scanline - 341 PPU cycles
	cycles++;
	if (cycles%341 == 0) 
	{
		scanline++;
	}
	if (scanline>260) 
	{
		scanline = 0;
		VBLANK = false;
	}

	if (scanline<240) // rendering
	{
		return 0;
	} 
	else if (scanline == 240) // idle, no vblank yet
	{
		return 0;
	}
	else if (scanline == 241) // Set vblank
	{
		if (!VBLANK) 
		{
			VBLANK = true;
			if (NMI_enabled)
			return 1;
		}
		/*if (NMI_enabled)*/ 
	}
	return 0;
}

void PPU::Render(SDL_Surface* s)
{
	SDL_FillRect( s, NULL, 0 );
	/*if (ShowBackground) */this->RenderBackground(s);
	/*if (ShowSprites) */this->RenderSprite(s);
}

void PPU::RenderSprite(SDL_Surface* s)
{
	uint8_t *PIXELS = (uint8_t*)s->pixels;
	uint32_t color = 0;

	if (!SpriteSize)
	{
		for (int i = 0; i<64; i++)
		{
			SPRITE spr = this->OAM[i];
			if (spr.y >= 0xEF) continue;

			uint16_t spriteaddr = SpritePattenTable + spr.index*16;

			// 8x8px
			for (int y = 0; y<8; y++)
			{
				int sprite_y = y;
				if ( spr.attr&0x40 ) sprite_y = 7 - sprite_y; //Hor flip

				uint8_t spritedata = memory[ spriteaddr + sprite_y ];
				uint8_t spritedata2 = memory[ spriteaddr + sprite_y + 8 ];
				for (int x = 0; x<8; x++)
				{
					int sprite_x = x;
					if ( spr.attr&0x80 ) sprite_x = 7 - sprite_x; //Ver flip

					bool c1 = ( spritedata  &(1<<(7-sprite_x)) )? true: false;
					bool c2 = ( spritedata2 &(1<<(7-sprite_x)) )? true: false;
					
					color = c1 | c2<<1;

					if (color == 0) continue;

					uint16_t _y = y+spr.y;
					uint16_t _x = x+spr.x;

					uint32_t dt = ( ( _y *256) +  _x  ) * 4;
					uint8_t *PIXEL = PIXELS+dt;

					Palette_entry e = nes_palette[ memory[0x3F10 + ((spr.attr&0x3)*4) + color] ];


					*(PIXEL+0) = e.b;
					*(PIXEL+1) = e.g;
					*(PIXEL+2) = e.r;
				}
			}
		}

	}
	else //8x16
	{
		for (int i = 0; i<64; i++)
		{
			SPRITE spr = this->OAM[i];
			if (spr.y >= 0xEF) continue;


			uint16_t spriteaddr = ( (spr.index&1) * 0x1000)  + ((spr.index>>1)*32);

			// 8x16px
			for (int y = 0; y<16; y++)
			{
				int sprite_y = y;
				if (sprite_y>7) sprite_y+=8;

				uint8_t spritedata = memory[ spriteaddr + sprite_y ];
				uint8_t spritedata2 = memory[ spriteaddr + sprite_y + 8 ];
				for (int x = 0; x<8; x++)
				{
					int sprite_x = x;
					if ( spr.attr&0x80 ) sprite_x = 7 - sprite_x; //Ver flip

					bool c1 = ( spritedata  &(1<<(7-sprite_x)) )? true: false;
					bool c2 = ( spritedata2 &(1<<(7-sprite_x)) )? true: false;
					
					color = c1 | c2<<1;

					if (color == 0) continue;

					uint16_t _y = y+spr.y;
					uint16_t _x = x+spr.x;

					uint32_t dt = ( ( _y *256) +  _x  ) * 4;
					uint8_t *PIXEL = PIXELS+dt;

					Palette_entry e = nes_palette[ memory[0x3F10 + ((spr.attr&0x3)*4) + color] ];


					*(PIXEL+0) = e.b;
					*(PIXEL+1) = e.g;
					*(PIXEL+2) = e.r;
				}
			}
		}

	}

}
void PPU::RenderBackground(SDL_Surface* s)
{
	//Q&D scrolling 
	SDL_Surface* bg = SDL_CreateRGBSurface( SDL_SWSURFACE, 256, 512, 32, 0, 0, 0, 0 );
	if (!bg) log->Fatal("PPU: Cannot create BG surface!");
	SDL_LockSurface( bg );
	int x = 0;
	int y = 0;

	uint8_t *PIXELS = (uint8_t*)bg->pixels;
	uint32_t color = 0;
	uint16_t Attribute = BaseNametable + 0x3c0;
	for (int i = 0; i<960; i++)
	{
		// Assume that Surface is 256x240
		uint16_t tile = this->memory[i+BaseNametable];
		uint32_t dt = ( (y*256*8) + (x*8) ) * 4;
		uint8_t *PIXEL = PIXELS+dt;
		for (uint8_t b = 0; b<8; b++) //Y
		{
			uint8_t tiledata = memory[ BackgroundPattenTable + tile*16 + b];
			uint8_t tiledata2 = memory[ BackgroundPattenTable + tile*16 + b +8];
			for (uint8_t a = 0; a<8; a++) //X
			{
				bool c1 = ( tiledata&(1<<(7-a)) )? true: false;
				bool c2 = ( tiledata2&(1<<(7-a)) )? true: false;

				uint8_t InfoByte = memory[ Attribute + ((y/4)*8)+(x/4) ];
				uint8_t infopal = 0;
				if ( (y%4)<=1 ) //up
				{
					if ( (x%4)<=1 ) infopal = InfoByte; // up-left
					else infopal = InfoByte>>2; // up-right
				}
				else 
				{
					if ( (x%4)<=1 ) infopal = InfoByte>>4; // down-left
					else infopal = InfoByte>>6; // down-right
				}

				infopal = (infopal&0x03);

				color = c1 | c2<<1;

				//BG Palette + Infopal*4
				Palette_entry e = nes_palette[ memory[0x3F00 + (infopal*4) + color] ];

				*(PIXEL+((b*256)+a)*4) = e.b;
				*(PIXEL+((b*256)+a)*4+1) = e.g;
				*(PIXEL+((b*256)+a)*4+2) = e.r;
			}
		}
		if (x++ == 31)
		{
			y++;
			x = 0;
		}

	}
	SDL_UnlockSurface( bg );
	SDL_UnlockSurface( s );

	SDL_Rect pos;
	pos.x = -ScrollX;
	pos.y = ScrollY;

	SDL_BlitSurface( bg, NULL, s, &pos );
	SDL_LockSurface( s );
	SDL_FreeSurface( bg );
}