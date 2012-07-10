bool debug = false;

#include <iostream>
#include <SDL.h>
#undef main
#include "headers.h"

#include "iNES.h"
#include "CPU.h"

#define MAJOR_VERSION 0
#define MINOR_VERSION 1
#define ROM_NAME "rom/colours.nes"

Logger* log;
CPU* cpu;
iNES* rom;

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
		SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,
		256, 240, SDL_WINDOW_SHOWN );

	if ( MainWindow == NULL )
	{
		log->Fatal("Cannot create main window");
		return 1;
	}
	log->Success("Main window created");
	
	SDL_DisplayMode mode;
	mode.format = SDL_PIXELFORMAT_RGB888;
	mode.w = 256;
	mode.h = 240;
	mode.refresh_rate = 0;
	mode.driverdata = 0;


	if ( SDL_SetWindowDisplayMode( MainWindow, &mode ) < 0 )
	{
		log->Fatal("SDL_SetWindowDisplayMode error");
		return 1;
	}

	SDL_Surface* screen = SDL_GetWindowSurface( MainWindow );


	//SDL_WM_IconifyWindow();

	log->Info("Creating CPU");
	cpu = new CPU();

	log->Info("Opening %s", ROM_NAME);
	rom = new iNES();
	if (rom->Load( ROM_NAME ))
	{
		log->Error("Cannot load %s", ROM_NAME);
		return 1;
	}
	log->Success("%s opened", ROM_NAME);


	uint16_t loadaddress = 0x8000;
	if (rom->PRG_ROM_pages>2)
	{
		log->Error("PRG_ROM pages > 2 will cause crash (no mappers supported). Breaking.");
		return 1;
	}
	else if (rom->PRG_ROM_pages == 1 )
	{
		loadaddress+=0x4000;
		cpu->memory.Load( loadaddress, rom->PRG_ROM, rom->PRG_ROM_pages*16*1024-1  );
		loadaddress+=0x4000;
	}
	// Hardcoded copy of rom into program space
	cpu->memory.Load( loadaddress, rom->PRG_ROM, rom->PRG_ROM_pages*16*1024-1  );
	log->Success("%dB PRG_ROM copied", rom->PRG_ROM_pages*16*1024);


	if (rom->CHR_ROM_pages!=1)
	{
		log->Error("CHR_ROM pages != 1 will cause crash (no mappers supported). Breaking.");
		return 1;
	}
	memcpy( cpu->ppu.memory, rom->CHR_ROM, rom->CHR_ROM_pages*8*1024 );
	log->Success("%dB CHR_ROM copied", rom->CHR_ROM_pages*8*1024 );

	log->Info("CPU Reset");
	cpu->memory.ppu = &cpu->ppu;
	cpu->Reset();


	SDL_Event event;

	bool dostep = false;
	bool prevstate = false;

	int64_t tick = 0;
	while(1)
	{
		SDL_PollEvent(&event);
		if (event.type == SDL_QUIT) break;
		if (event.type == SDL_KEYDOWN) 
		{
			if (event.key.keysym.sym == SDLK_RETURN)
			{
				debug = !debug;
			}

			if (event.key.keysym.sym == SDLK_SPACE)
			{
				if (!prevstate)	dostep = true;
				prevstate = true;
			}
			else prevstate = false;
		}
		if (event.type == SDL_KEYUP) 
		{
			if (event.key.keysym.sym == SDLK_SPACE)
			{
				prevstate = false;
			}
		}

		//if (!dostep) continue;
		//dostep = false;
		for (int i = 0; i<3; i++)
		{
			if (cpu->ppu.Step()) // NMI requested
			{
				
				SDL_LockSurface( screen );
				
				cpu->ppu.Render( screen );
				SDL_UnlockSurface( screen );
				SDL_UpdateWindowSurface( MainWindow );
				cpu->NMI();
			}
		}

		cpu->Step();
		++tick;
		if (tick%500 == 0 ) Sleep(1);
	}

	delete rom;
	delete cpu;
	SDL_DestroyWindow( MainWindow );
	SDL_Quit();
	log->Info("Goodbye");
	return 0;
}