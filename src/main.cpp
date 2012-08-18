bool debug = false;
int buttonState = 0;
#include <iostream>
#include <SDL.h>
#undef main
#include "headers.h"

#include "iNES.h"
#include "CPU.h"
#include "CPU_interpreter.h"

#define MAJOR_VERSION 0
#define MINOR_VERSION 1
#define ROM_NAME "rom/micromachines.nes"

Logger* log;
CPU* cpu;
iNES* rom;

unsigned char FileName[2048];

bool SpriteSize;
SDL_Window *ToolboxPalette;
SDL_Window *ToolboxOAM;
SDL_Window *ToolboxNametable;


void __DrawNametable(SDL_Surface* s, uint8_t nametable)
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
				bool c1 = ( tiledata&(1<<(7-a)) )? true: false;
				bool c2 = ( tiledata2&(1<<(7-a)) )? true: false;

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

void __DrawOAM(SDL_Surface* s, int i)
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

void RefrestToolbox()
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

	// Update toolbox Nametable
	SDL_Surface *tns = SDL_GetWindowSurface( ToolboxNametable );
	for ( int i = 0; i<4; i++ )
	{
		uint8_t currentNametable = 0;//(cpu->ppu.BaseNametable-0x2000)/0x400;
		uint8_t cnx = (currentNametable%2);
		uint8_t cny = (currentNametable/2);
		SDL_Rect r = { (i%2)*256, (i/2)*240, 256, 240 };

		SDL_Surface* Sspr = SDL_CreateRGBSurface( SDL_SWSURFACE, 256, 256, 32, 0, 0, 0, 0 ); //Fuck error check
		if (!Sspr) log->Fatal("PPU: Cannot create Sspr surface!");
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

int main()
{
	log = new Logger("log.txt");
	log->Info("AnotherNES version %d.%d", MAJOR_VERSION, MINOR_VERSION);
	
	if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		log->Fatal("SDL_Init failed");
		return 1;
	}
	log->Success("SDL_Init successful");

	SDL_Window* MainWindow = SDL_CreateWindow( "AnotherNES", 
		542, 20,
		256*2, 240*2, SDL_WINDOW_SHOWN );

	if ( MainWindow == NULL )
	{
		log->Fatal("Cannot create main window");
		return 1;
	}
	log->Success("Main window created");
	
	SDL_DisplayMode mode;
	mode.format = SDL_PIXELFORMAT_RGB888;
	mode.w = 256*2;
	mode.h = 240*2;
	mode.refresh_rate = 0;
	mode.driverdata = 0;


	if ( SDL_SetWindowDisplayMode( MainWindow, &mode ) < 0 )
	{
		log->Fatal("SDL_SetWindowDisplayMode error");
		return 1;
	}

	SDL_Surface* screen = SDL_GetWindowSurface( MainWindow );

	SDL_Surface* canvas = SDL_CreateRGBSurface( SDL_SWSURFACE, 256, 240, 32, 0, 0, 0, 0 );
	if (!canvas) log->Fatal("Cannot create canvas surface!");


	// Toolbox

	// Palette window
	int wX, wY;
	SDL_GetWindowPosition( MainWindow, &wX, &wY );
	{
		ToolboxPalette = SDL_CreateWindow( "AnotherNES - Palette", wX, wY+240*2+24, 16*16, 2*16, SDL_WINDOW_SHOWN );
	}
	if ( ToolboxPalette == NULL )
	{
		log->Fatal("Cannot create toolbox palette");
		return 1;
	}
	log->Success("Toolbox palette window created");

	// OAM window
	ToolboxOAM = SDL_CreateWindow( "AnotherNES - OAM", wX+16*16, wY+240*2+24, 16*8*2, 4*8*2, SDL_WINDOW_SHOWN );
	if ( ToolboxOAM == NULL )
	{
		log->Fatal("Cannot create Toolbox OAM");
		return 1;
	}
	log->Success("Toolbox OAM window created");
	
	// Nametable window
	ToolboxNametable = SDL_CreateWindow( "AnotherNES - Nametable", wX-256*2-16, wY, 256*2, 240*2, SDL_WINDOW_SHOWN );
	if ( ToolboxNametable == NULL )
	{
		log->Fatal("Cannot create Toolbox Nametable");
		return 1;
	}
	log->Success("Toolbox Nametable window created");


	//SDL_WM_IconifyWindow();

	log->Info("Creating CPU interpreter");
	cpu = new CPU_interpreter();


	strcpy( (char*)FileName, ROM_NAME );
//#ifndef _DEBUG
// Show file selection dialog
	OPENFILENAME ofn = {0};
	ofn.lStructSize = sizeof( ofn );
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = (char*)FileName;
	ofn.lpstrFile[0] = 0;
	ofn.nMaxFile = sizeof(FileName);
	ofn.lpstrFilter = "NES\0*.nes\0";
	ofn.nFilterIndex = 0;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = "./rom/";
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	if (!GetOpenFileName( &ofn ))
	{
		log->Debug("GetOpenFileName problem!: %d", GetLastError());
	strcpy( (char*)FileName, ROM_NAME );
	}
//#endif

	log->Info("Opening %s", FileName);
	rom = new iNES();
	if (rom->Load( (const char*)FileName ))
	{
		log->Error("Cannot load %s", FileName);
		return 1;
	}
	log->Success("%s opened", FileName);

	log->Info("Mapper: %d", rom->Mapper);

	cpu->memory.mapper = rom->Mapper;

	if (rom->PRG_ROM_pages>128)
	{
		log->Error("PRG_ROM pages > 128 (more than 2MB, unsupported)");
		return 1;
	}
	memcpy( cpu->memory.prg_rom, rom->PRG_ROM, rom->PRG_ROM_pages*16*1024 );
	cpu->memory.prg_pages = rom->PRG_ROM_pages;
	if (rom->PRG_ROM_pages == 1) cpu->memory.prg_lowpage = 1; //fix
	cpu->memory.prg_lowpage = 0;

	cpu->memory.prg_highpage = rom->PRG_ROM_pages-1;
	if (rom->Mapper == 104) cpu->memory.prg_highpage = 15; // Golden five fix

	log->Success("%dB PRG_ROM copied", rom->PRG_ROM_pages*16*1024);


	//if (rom->CHR_ROM_pages!=1)
	//{
	//	log->Error("CHR_ROM pages != 1 will cause crash (no mappers supported). Breaking.");
	//	return 1;
	//}
	memcpy( cpu->ppu.memory, rom->CHR_ROM, rom->CHR_ROM_pages*8*1024 );
	log->Success("%dB CHR_ROM copied", rom->CHR_ROM_pages*8*1024 );

	log->Info("CPU Reset");
	cpu->memory.ppu = &cpu->ppu;
	cpu->ppu.Mirroring = rom->Mirroring;
	cpu->Reset();


	SDL_Event event;

	bool dostep = false;
	bool prevstate = false;

	int64_t tick = 0;
	while(1)
	{
		SDL_PollEvent(&event);
		if (event.type == SDL_QUIT) break;

		Uint8 *keys = SDL_GetKeyboardState(NULL);
		//A, B, Select, Start, Up, Down, Left, Right.
		buttonState = 0;
		if ( keys[SDL_SCANCODE_X] ) buttonState |= 1<<7;
		if ( keys[SDL_SCANCODE_Z] ) buttonState |= 1<<6;
		if ( keys[SDL_SCANCODE_A] ) buttonState |= 1<<5;
		if ( keys[SDL_SCANCODE_S] ) buttonState |= 1<<4;
		if ( keys[SDL_SCANCODE_UP] ) buttonState |= 1<<3;
		if ( keys[SDL_SCANCODE_DOWN] ) buttonState |= 1<<2;
		if ( keys[SDL_SCANCODE_LEFT] ) buttonState |= 1<<1;
		if ( keys[SDL_SCANCODE_RIGHT] ) buttonState |= 1<<0;

		if ( keys[SDL_SCANCODE_R] ) {cpu->Reset(); Sleep(1000);}
		if ( keys[SDL_SCANCODE_T] ) RefrestToolbox();


		for (int i = 0; i<3; i++)
		{
			uint8_t ppuresult = cpu->ppu.Step();
			if (ppuresult) // NMI requested
			{
				if (ppuresult == 100) 
				{
					static int UpdateSecond = 0;
					if (UpdateSecond%2 == 0) RefrestToolbox();
					UpdateSecond++;
				}
				else
				{
					SDL_LockSurface( canvas );
					cpu->ppu.Render( canvas );
					SDL_UnlockSurface( canvas );


					SDL_SoftStretch( canvas, NULL, screen, NULL );
					SDL_UpdateWindowSurface( MainWindow );
					

					if (ppuresult==2) cpu->NMI();
				}
			}
		}

		cpu->Step();
		++tick;
	}

	delete rom;
	delete cpu;
	SDL_FreeSurface( canvas );
	SDL_DestroyWindow( MainWindow );
	SDL_Quit();
	log->Info("Goodbye");
	return 0;
}