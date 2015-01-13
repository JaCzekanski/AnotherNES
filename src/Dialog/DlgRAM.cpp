#include "DlgRAM.h"
#include <algorithm>

DlgRAM::DlgRAM(CPU* cpu)
{

	FILE* font = fopen("D:\\Kuba\\dev\\AnotherNES\\font.bin", "rb");

	if (!font)
	{
		Log->Fatal("Cannot read font file.");
		return;
	}
	for (int n = 0; n<256; n++)
	{
		for (int i = 0; i<8; i++)
		{
			unsigned char c = fgetc(font);
			Font[n][i] = c;
		}
	}

	fclose(font);

	this->cpu = cpu;
	// Palette window
	int wX = 0;
	int wY = 0;
	//SDL_GetWindowPosition( MainWindow, &wX, &wY );
	// 2048 / 32 = 64
	ToolboxRAM = SDL_CreateWindow( "AnotherNES - RAM", 5, 5, 32*(8*2+1), 64*(8+1), SDL_WINDOW_SHOWN );
	if (ToolboxRAM == NULL)
	{
		Log->Fatal("Cannot create toolbox ram");
		return;
	}
	//if (SurfaceIcon) SDL_SetWindowIcon( ToolboxPalette, SurfaceIcon );
	this->Clear();
	this->WindowID = SDL_GetWindowID(ToolboxRAM);
	Log->Success("Toolbox ram created");


	ramState.resize(2048);
	prevRamState.resize(2048);
}

DlgRAM::~DlgRAM(void)
{
	SDL_DestroyWindow(ToolboxRAM);
	Log->Success("Toolbox ram destroyed.");
}

struct byte_pair
{
	int byte;
	int count;
};

bool byte_pair_comp(byte_pair i, byte_pair j) { return (i.count > j.count); }

void DlgRAM::Update()
{
	SDL_Rect r;
	// Update toolbox palette
	SDL_Surface *tps = SDL_GetWindowSurface(ToolboxRAM);
	SDL_FillRect(tps, NULL, 0xff222222);

	for (int i = 0; i < 2048; i++)
	{
		ramState[i].value = cpu->memory[i];
		ramState[i].blinkTime = prevRamState[i].blinkTime;
		int baseBrighness = (ramState[i].value == 0 ? 0x10 : 0x40);

		if (ramState[i].value != prevRamState[i].value)
		{
			ramState[i].blinkTime = 10;
		}

		if (ramState[i].blinkTime > 0) ramState[i].blinkTime--;
		ramState[i].brightness = baseBrighness + (ramState[i].blinkTime) * 16;
	}

	// Zero page
	r = { 0, 0, 32*17, 8*17 };
	SDL_FillRect(tps, &r, 0xff005050);

	// Stack
	r = { 0, 8 * 17, 32 * 17, 8 * 17 };
	SDL_FillRect(tps, &r, 0xff500050);

	// Mouse selected
	r = { selectedX * 17 -1, selectedY * 17 -1, 18, 18 };
	SDL_FillRect(tps, &r, 0xffff0000);

	SDL_Surface *font_char = SDL_CreateRGBSurface(0, 16, 8, 32, 0, 0, 0, 0xff000000);
	SDL_SetSurfaceBlendMode(font_char, SDL_BLENDMODE_ADD);

	for ( int y = 0; y<64; y++ )
	{
		for (int x = 0; x<32; x++ )
		{
			SDL_Rect r = { x*17, y*17, 16, 16 };
			uint8_t byte = cpu->memory[(y * 32) + x];
			int grayscale = ramState[(y * 32) + x].brightness;
			Palette_entry col;
			if (cpu->memory.memoryLock[(y * 32) + x])
			{
				grayscale = 0xff;
				col.r = grayscale;
				col.g = grayscale;
				col.b = 0;
			}
			else
			{
				col.r = 0;
				col.g = grayscale;
				col.b = 0;
			}
			
			SDL_FillRect( tps, &r, col.r<<16 | col.g<<8 | col.b );

			// Make char
			SDL_LockSurface(font_char);
			for (int yy = 0; yy < 8; yy++)
			{
				//if (byte == 0x00) grayscale = 0x22;
				for (int xx = 0; xx < 8; xx++)
				{
					int h = (byte & 0xf0) >> 4;
					if (h <= 9) h = '0' + h;
					else h = 1 + (h-10);

					int bit = (Font[h][yy] & (0x80 >> xx)) ? grayscale : 0x00;
					*((unsigned char*)font_char->pixels + (yy * 16 + xx) * 4 + 0) = bit;
					*((unsigned char*)font_char->pixels + (yy * 16 + xx) * 4 + 1) = bit;
					*((unsigned char*)font_char->pixels + (yy * 16 + xx) * 4 + 2) = bit;
					*((unsigned char*)font_char->pixels + (yy * 16 + xx) * 4 + 3) = (bit?0xff:0x00);
				}
				for (int xx = 8; xx < 16; xx++)
				{
					int h = (byte & 0xf);
					if (h <= 9) h = '0' + h;
					else h = 1 + (h - 10);

					int bit = (Font[h][yy] & (0x80 >> (xx - 8))) ? grayscale : 0x00;
					*((unsigned char*)font_char->pixels + (yy * 16 + xx) * 4 + 0) = bit;
					*((unsigned char*)font_char->pixels + (yy * 16 + xx) * 4 + 1) = bit;
					*((unsigned char*)font_char->pixels + (yy * 16 + xx) * 4 + 2) = bit;
					*((unsigned char*)font_char->pixels + (yy * 16 + xx) * 4 + 3) = (bit ? 0xff : 0x00);
				}
			}

			SDL_UnlockSurface(font_char);

			SDL_Rect r2 = { x * 17 , y * 17 + 4, 16, 8 };
			SDL_BlitSurface(font_char, NULL, tps, &r2);
		}
	}
	SDL_FreeSurface(font_char);
	SDL_UpdateWindowSurface(ToolboxRAM);

	prevRamState = ramState;
}

void DlgRAM::Clear()
{
	SDL_Surface *tns = SDL_GetWindowSurface(ToolboxRAM);
	SDL_FillRect( tns, NULL, 0x000000 );
	SDL_UpdateWindowSurface( ToolboxRAM );
}


void DlgRAM::Click(int x, int y, int button)
{
	int px = x / 17;
	int py = y / 17;

	int ptr = py * 32 + px;
	if (this->cpu)
	{
		int prevval = cpu->memory[ptr];
		if (button == 0x01) // Left
			cpu->memory.Write(ptr, cpu->memory[ptr]+1);
		else if (button == 0x03) // Right
			cpu->memory.Write(ptr, cpu->memory[ptr]-1);
		else if (button == 0x02) // Middle / lock byte
			cpu->memory.memoryLock[ptr] = !cpu->memory.memoryLock[ptr];
		Log->Info("DlgRAM [0x%x] 0x%x -> 0x%x", ptr, prevval, cpu->memory[ptr]);
	}
}


void DlgRAM::Move(int x, int y)
{
	selectedX = x / 17;
	selectedY = y / 17;
}

void DlgRAM::Key(SDL_Keycode k)
{
	int key;
	if (k >= '0' && k <= '9') key = k - '0';
	else if (k >= 'a' && k <= 'f') key = k - 'a' + 10;
	else return;

	if (this->cpu && selectedX>=0 && selectedY>=0)
	{
		if (editStarted == false)
		{
			editStarted = true;
			editValue = key << 4;
		}
		else
		{
			editStarted = false;
			editValue |= key;

			int ptr = selectedY * 32 + selectedX;
			int prevval = cpu->memory[ptr];
			cpu->memory.Write(ptr, editValue);

			cpu->memory.memoryLock[ptr] = true;

			Log->Info("DlgRAM [0x%x] 0x%x -> 0x%x", ptr, prevval, cpu->memory[ptr]);
		}
	}
}