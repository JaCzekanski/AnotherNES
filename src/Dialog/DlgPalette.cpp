#include "DlgPalette.h"

DlgPalette::DlgPalette( CPU* cpu )
{
	this->cpu = cpu;
	// Palette window
	int wX = 0;
	int wY = 0;
	//SDL_GetWindowPosition( MainWindow, &wX, &wY );
	ToolboxPalette = SDL_CreateWindow( "AnotherNES - Palette", 5, 240*2+24+20, 16*16, 2*16, SDL_WINDOW_SHOWN );
	if ( ToolboxPalette == NULL )
	{
		log->Fatal("Cannot create toolbox palette");
		return;
	}
	//if (SurfaceIcon) SDL_SetWindowIcon( ToolboxPalette, SurfaceIcon );
	log->Success("Toolbox palette created");
}

DlgPalette::~DlgPalette(void)
{
	SDL_DestroyWindow(ToolboxPalette);
	log->Success("Toolbox Palette destroyed.");
}

void DlgPalette::Update()
{
	// Update toolbox palette
	SDL_Surface *tps = SDL_GetWindowSurface( ToolboxPalette );
	for ( int y = 0; y<2; y++ )
	{
		for (int x = 0; x<16; x++ )
		{
			SDL_Rect r = { x*16, y*16, 16, 16 };
			uint8_t pal = cpu->ppu.memory[0x3f00 + (y*16) + x];
			Palette_entry col = nes_palette[ pal ];

			SDL_FillRect( tps, &r, col.r<<16 | col.g<<8 | col.b );
		}
	}
	SDL_UpdateWindowSurface( ToolboxPalette );
}
