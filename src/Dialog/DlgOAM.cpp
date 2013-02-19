#include "DlgOAM.h"

DlgOAM::DlgOAM( CPU* cpu )
{
	this->cpu = cpu;
	// OAM window
	ToolboxOAM = SDL_CreateWindow( "AnotherNES - OAM", 16*16+10, 240*2+24+20, 16*8*2, 4*8*2, SDL_WINDOW_SHOWN );
	if ( ToolboxOAM == NULL )
	{
		log->Fatal("Cannot create Toolbox OAM");
		return ;
	}
	//if (SurfaceIcon) SDL_SetWindowIcon( ToolboxOAM, SurfaceIcon );
	this->Clear();
	this->WindowID = SDL_GetWindowID(ToolboxOAM);
	log->Success("Toolbox OAM created");
}

DlgOAM::~DlgOAM(void)
{
	SDL_DestroyWindow(ToolboxOAM);
	log->Success("Toolbox OAM destroyed.");
}

void DlgOAM::Update()
{
	// Update toolbox OAM
	if ( SpriteSize != cpu->ppu.SpriteSize )
	{
		SpriteSize = cpu->ppu.SpriteSize;
		if (!SpriteSize) // 8x8
			SDL_SetWindowSize( ToolboxOAM, 16*8*2, 4*8*2 );
		else
			SDL_SetWindowSize( ToolboxOAM, 16*8*2, 4*8*2*2 );
	}
	SDL_Surface *tos = SDL_GetWindowSurface( ToolboxOAM );
	for ( int Gy = 0; Gy<4; Gy++ )
	{
		for (int Gx = 0; Gx<16; Gx++ )
		{
			SDL_Rect r = { Gx*16, Gy*16, 16, 16 };
			SDL_Surface* Sspr = NULL;
			if (!SpriteSize) // 8x8
			{
				Sspr = SDL_CreateRGBSurface( SDL_SWSURFACE, 8, 8, 32, 0, 0, 0, 0 ); //Fuck error check
				r.y = Gy*16;
				r.h = 16;
			}
			else
			{
				Sspr = SDL_CreateRGBSurface( SDL_SWSURFACE, 8, 16, 32, 0, 0, 0, 0 ); //Fuck error check
				r.y = Gy*32;
				r.h = 32;
			}
			if (!Sspr) log->Fatal("PPU: Cannot create Sspr surface!");
			SDL_LockSurface( Sspr );
			
			__DrawOAM( Sspr, Gy*16 + Gx );

			SDL_UnlockSurface( Sspr );
			SDL_BlitScaled( Sspr, NULL, tos, &r );
			SDL_FreeSurface( Sspr );
		}
	}
	SDL_UpdateWindowSurface( ToolboxOAM );
}


void DlgOAM::__DrawOAM(SDL_Surface* s, int i)
{
	uint8_t *PIXEL = (uint8_t*)s->pixels;
	uint32_t color = 0;

	if (!cpu->ppu.SpriteSize)
	{
		//for (int i = 0; i<64; i++)
		{
			SPRITE spr = cpu->ppu.OAM[i];
			if (spr.y<0xff) spr.y+=1;
			//if (spr.y >= 0xEF) continue;

			uint16_t spriteaddr = cpu->ppu.SpritePattenTable + spr.index*16;

			// 8x8px
			for (int y = 0; y<8; y++)
			{
				int sprite_y = y;
				if ( spr.attr&0x80 ) sprite_y = 7 - sprite_y; //Vertical flip

				if ( sprite_y+spr.y+8 > 240 ) continue;

				uint8_t spritedata = cpu->ppu.memory[ spriteaddr + sprite_y ];
				uint8_t spritedata2 = cpu->ppu.memory[ spriteaddr + sprite_y + 8 ];
				for (int x = 0; x<8; x++)
				{
					int sprite_x = x;
					if ( spr.attr&0x40 ) sprite_x = 7 - sprite_x; //Horizontal flip

					bool c1 = ( spritedata  &(1<<(7-sprite_x)) )? true: false;
					bool c2 = ( spritedata2 &(1<<(7-sprite_x)) )? true: false;
					
					color = c1 | c2<<1;

					Palette_entry e = nes_palette[ cpu->ppu.memory[0x3F10 + ((spr.attr&0x3)*4) + color] ];

					*(PIXEL++) = e.b;
					*(PIXEL++) = e.g;
					*(PIXEL++) = e.r;
					PIXEL++;
				}
			}
		}

	}
	else //8x16
	{
		//for (int i = 0; i<64; i++)
		{
			SPRITE spr = cpu->ppu.OAM[i];
			if (spr.y<0xff) spr.y+=1;
			//if (spr.y >= 0xEF) continue;


			uint16_t spriteaddr = ( (spr.index&1) * 0x1000)  + ((spr.index>>1)*32);

			// 8x16px
			for (int y = 0; y<16; y++)
			{
				int sprite_y = y;
				if ( spr.attr&0x80 ) 
				{
					sprite_y = 15 - sprite_y; //Vertical flip
				}
				if (sprite_y>7) sprite_y+=8;

				uint8_t spritedata = cpu->ppu.memory[ spriteaddr + sprite_y ];
				uint8_t spritedata2 = cpu->ppu.memory[ spriteaddr + sprite_y + 8 ];
				for (int x = 0; x<8; x++)
				{
					int sprite_x = x;
					if ( spr.attr&0x40 ) sprite_x = 7 - sprite_x; //Ver flip

					bool c1 = ( spritedata  &(1<<(7-sprite_x)) )? true: false;
					bool c2 = ( spritedata2 &(1<<(7-sprite_x)) )? true: false;
					
					color = c1 | c2<<1;

					Palette_entry e = nes_palette[ cpu->ppu.memory[0x3F10 + ((spr.attr&0x3)*4) + color] ];

					*(PIXEL++) = e.b;
					*(PIXEL++) = e.g;
					*(PIXEL++) = e.r;
					PIXEL++;
				}
			}
		}

	}

}


void DlgOAM::Clear()
{
	SDL_Surface *tns = SDL_GetWindowSurface( ToolboxOAM );
	SDL_FillRect( tns, NULL, 0x000000 );
	SDL_UpdateWindowSurface( ToolboxOAM );
}