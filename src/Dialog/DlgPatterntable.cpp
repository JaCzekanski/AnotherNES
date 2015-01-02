#include "DlgPatterntable.h"

DlgPatterntable::DlgPatterntable(CPU* cpu)
{
	this->cpu = cpu;
	// Nametable window
	ToolboxPatterntable = SDL_CreateWindow("AnotherNES - Pattertable", 5, 20, 16*9*2, 32*9*2, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	if (ToolboxPatterntable == NULL)
	{
		Log->Fatal("Cannot create Toolbox Patterntable");
		return;
	}
	//if (SurfaceIcon) SDL_SetWindowIcon( ToolboxNametable, SurfaceIcon );
	this->Clear();
	this->WindowID = SDL_GetWindowID(ToolboxPatterntable);
	Log->Success("Toolbox Patterntable created");
}

DlgPatterntable::~DlgPatterntable(void)
{
	SDL_DestroyWindow(ToolboxPatterntable);
	Log->Success("Toolbox Patterntable destroyed.");
}

void DlgPatterntable::Update()
{
	// Update toolbox Nametable
	SDL_Surface *tns = SDL_GetWindowSurface(ToolboxPatterntable);
	SDL_Rect r = { 0, 0, 256, 240 };

	SDL_Surface* Sspr = SDL_CreateRGBSurface(SDL_SWSURFACE, 16*9, 32*9, 32, 0, 0, 0, 0);
	if (!Sspr)
	{
		Log->Fatal("PPU: Cannot create Sspr surface!");
		return;
	}
	SDL_LockSurface(Sspr);

	uint16_t NametableAddress = (0x2000 + 0x400 * (cpu->ppu.BaseNametable & 0x03));
	uint16_t Attribute = NametableAddress + 0x3c0;

	SDL_Color(*PIXEL)[16*9] = (SDL_Color(*)[16*9]) Sspr->pixels;
	uint32_t color = 0;
	int x = 0, y = 0;
	for (int tile = 0; tile<512; tile++)
	{
		for (uint8_t b = 0; b<8; b++) //Y
		{
			uint8_t tiledata = cpu->ppu.memory[ tile * 16 + b];
			uint8_t tiledata2 = cpu->ppu.memory[ tile * 16 + b + 8];
			for (uint8_t a = 0; a<8; a++) //X
			{
				uint8_t c1 = (tiledata&(1 << (7 - a))) ? 1 : 0;
				uint8_t c2 = (tiledata2&(1 << (7 - a))) ? 1 : 0;

				uint8_t InfoByte = cpu->ppu.memory[Attribute + ((y / 4) * 8) + (x / 4)];
				uint8_t infopal = 0;
				if ((y % 4) <= 1) //up
				{
					if ((x % 4) <= 1) infopal = InfoByte; // up-left
					else infopal = InfoByte >> 2; // up-right
				}
				else
				{
					if ((x % 4) <= 1) infopal = InfoByte >> 4; // down-left
					else infopal = InfoByte >> 6; // down-right
				}

				infopal = (infopal & 0x03);

				color = c1 | c2 << 1;

				if (color == 0)	infopal = 0;

				//BG Palette + Infopal*4
				Palette_entry e = nes_palette[cpu->ppu.memory[0x3F00 + (infopal * 4) + color]];

				SDL_Color c = { e.b, e.g, e.r };
				PIXEL[y * 9 + b][x * 9 + a] = c;
			}
		}
		if (x++ == 15)
		{
			y++;
			x = 0;
		}

	}

	SDL_UnlockSurface(Sspr);
	SDL_SoftStretch(Sspr, NULL, tns, NULL);
	SDL_FreeSurface(Sspr);
	SDL_UpdateWindowSurface(ToolboxPatterntable);
}

void DlgPatterntable::Clear()
{
	SDL_Surface *tns = SDL_GetWindowSurface(ToolboxPatterntable);
	SDL_FillRect(tns, NULL, 0x000000);
	SDL_UpdateWindowSurface(ToolboxPatterntable);
}