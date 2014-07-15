#include "DlgNametable.h"

DlgNametable::DlgNametable( CPU* cpu )
{
	this->cpu = cpu;
	// Nametable window
	ToolboxNametable = SDL_CreateWindow( "AnotherNES - Nametable", 5, 20, 256*2, 240*2, SDL_WINDOW_SHOWN );
	if ( ToolboxNametable == NULL )
	{
		Log->Fatal("Cannot create Toolbox Nametable");
		return;
	}
	//if (SurfaceIcon) SDL_SetWindowIcon( ToolboxNametable, SurfaceIcon );
	this->Clear();
	this->WindowID = SDL_GetWindowID(ToolboxNametable);
	Log->Success("Toolbox Nametable created");
}

DlgNametable::~DlgNametable(void)
{
	SDL_DestroyWindow(ToolboxNametable);
	Log->Success("Toolbox Nametable destroyed.");
}

void DlgNametable::Update()
{
	// Update toolbox Nametable
	SDL_Surface *tns = SDL_GetWindowSurface( ToolboxNametable );
	for ( int i = 0; i<4; i++ )
	{
		uint8_t currentNametable = 0;//(cpu->ppu.BaseNametable-0x2000)/0x400;
		uint8_t cnx = (currentNametable%2);
		uint8_t cny = (currentNametable/2);
		SDL_Rect r = { (i%2)*256, (i/2)*240, 256, 240 };

		SDL_Surface* Sspr = SDL_CreateRGBSurface( SDL_SWSURFACE, 256, 256, 32, 0, 0, 0, 0 ); //Fuck error check
		if (!Sspr) Log->Fatal("PPU: Cannot create Sspr surface!");
		SDL_LockSurface( Sspr );

		if (cpu->ppu.Mirroring == VERTICAL) // Vertical
		{
			if (i%2 == 0) __DrawNametable(Sspr, cnx);
			else __DrawNametable(Sspr, !cnx);
		}
		else // Horizontal
		{
			if (i/2 == 0) __DrawNametable(Sspr, (cny)*2);
			else __DrawNametable(Sspr, (!cny)*2);
		}

		SDL_UnlockSurface( Sspr );
		SDL_BlitSurface( Sspr, NULL, tns, &r );
		SDL_FreeSurface( Sspr );
	}
	SDL_UpdateWindowSurface( ToolboxNametable );
}

void DlgNametable::__DrawNametable(SDL_Surface* s, uint8_t nametable)
{
	//Q&D scrolling 
	int x = 0;
	int y = 0;

	SDL_Color (*PIXEL)[256] = (SDL_Color(*)[256]) s->pixels;
	uint32_t color = 0;

	uint16_t NametableAddress = (0x2000 + 0x400 * (nametable&0x03));

	uint16_t Attribute = NametableAddress + 0x3c0;
	for (int i = 0; i<960; i++)
	{
		// Assume that Surface is 256x240
		uint16_t tile = cpu->ppu.memory[i+NametableAddress];
		for (uint8_t b = 0; b<8; b++) //Y
		{
			uint8_t tiledata = cpu->ppu.memory[ cpu->ppu.BackgroundPattenTable + tile*16 + b];
			uint8_t tiledata2 = cpu->ppu.memory[ cpu->ppu.BackgroundPattenTable + tile*16 + b +8];
			for (uint8_t a = 0; a<8; a++) //X
			{
				uint8_t c1 = ( tiledata&(1 << (7 - a)) )? 1: 0;
				uint8_t c2 = (tiledata2&(1 << (7 - a))) ? 1 : 0;

				uint8_t InfoByte = cpu->ppu.memory[ Attribute + ((y/4)*8)+(x/4) ];
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

				if (color == 0)	infopal = 0;

				//BG Palette + Infopal*4
				Palette_entry e = nes_palette[ cpu->ppu.memory[0x3F00 + (infopal*4) + color] ];

				SDL_Color c = {e.b, e.g, e.r};
				PIXEL[y*8+b][x*8+a] = c;
			}
		}
		if (x++ == 31)
		{
			y++;
			x = 0;
		}

	}
}

void DlgNametable::Clear()
{
	SDL_Surface *tns = SDL_GetWindowSurface( ToolboxNametable );
	SDL_FillRect( tns, NULL, 0x000000 );
	SDL_UpdateWindowSurface( ToolboxNametable );
}