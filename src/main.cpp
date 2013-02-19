#ifdef _DEBUG
#include <vld.h>
#endif
bool debug = false;
int buttonState = 0;
#include <windows.h>
#include <xinput.h>
#include <iostream>
#include <SDL.h>
#include <SDL_syswm.h>
#undef main
#include "headers.h"
#include "resource.h"
#include "iNES.h"
#include "CPU.h"
#include "CPU_interpreter.h"
#include "APU.h"

#include "dialog/DlgOAM.h"
#include "dialog/DlgPalette.h"
#include "dialog/DlgNametable.h"
#include "dialog/DlgAbout.h"

#include "version.h"

#define ROM_NAME "rom/contra.nes"
Logger* log;
CPU* cpu;
iNES* rom;

unsigned char FileName[2048];

DlgOAM *ToolboxOAM;
DlgPalette *ToolboxPalette;
DlgNametable *ToolboxNametable;

void audiocallback(void *userdata, Uint8 *stream, int len)
{
	if (cpu)
	{
		cpu->apu.audiocallback( userdata, stream, len );
	}
}
int64_t tick = 0;
int main()
{
	HINSTANCE hInstance = GetModuleHandle(NULL);
	log = new Logger("log.txt");
	log->Info("AnotherNES version %d.%d", MAJOR_VERSION, MINOR_VERSION);
	
	if ( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO ) < 0 )
	{
		log->Fatal("SDL_Init failed");
		return 1;
	}
	log->Success("SDL_Init successful");

	SDL_Window* MainWindow = SDL_CreateWindow( "AnotherNES", 
		542, 20,
		256*2, 240*2+20, SDL_WINDOW_SHOWN );

	if ( MainWindow == NULL )
	{
		log->Fatal("Cannot create main window");
		return 1;
	}
	log->Success("Main window created");
	
	SDL_SysWMinfo WindowInfo;
	SDL_VERSION(&WindowInfo.version);
	SDL_GetWindowWMInfo( MainWindow, &WindowInfo);

	HWND MainWindowHwnd = WindowInfo.info.win.window;

	// Icon
	SDL_Surface *SurfaceIcon = SDL_LoadBMP("icon.bmp");
	if (!SurfaceIcon)
	{
		log->Error("Cannot load icon.bmp");
		SurfaceIcon = NULL;
	}
	else
		SDL_SetWindowIcon( MainWindow, SurfaceIcon );

	// Menu
	HMENU Menu = LoadMenu( hInstance, MAKEINTRESOURCE( RES_MENU ) );
	if (!SetMenu( MainWindowHwnd, Menu ))
	{
		log->Fatal("Problem loading resource (menu)");
		return 1;
	}
	SDL_EventState( SDL_SYSWMEVENT, SDL_ENABLE );

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

	SDL_AudioSpec requested, obtained;
	requested.channels = 1;
	requested.format = AUDIO_U8;
	requested.freq = 44100 ;
	requested.samples = 2048;
	requested.callback = audiocallback;
	if ( SDL_OpenAudio( &requested, &obtained ) == -1 )
	{
		log->Error("SDL_OpenAudio error.");
	}
	SDL_PauseAudio(0);

	log->Success("Audio initialized.");
	//SDL_WM_IconifyWindow();

	XINPUT_STATE xstate;
	bool XboxPresent = false;
	if (XInputGetState(0, &xstate) == ERROR_SUCCESS)
	{
		log->Success("XInput: Xbox360 controller connected.");
		XboxPresent = true;
	}
	else
	{
		log->Info("XInput: No Xbox360 controller found.");
	}
	

	log->Info("Creating CPU interpreter");
	cpu = new CPU_interpreter();


	strcpy( (char*)FileName, ROM_NAME );
//#ifndef _DEBUG
// Show file selection dialog
	OPENFILENAME ofn;
	memset( &ofn, 0, sizeof(ofn) );
	ofn.lStructSize = sizeof( ofn );
	ofn.hwndOwner = MainWindowHwnd;
	ofn.hInstance = hInstance;
	memset(FileName, 0, sizeof(FileName) );
	ofn.lpstrFile = (char*)FileName;
	ofn.nMaxFile = sizeof(FileName);
	ofn.lpstrFilter = "NES\0*.nes\0";
	ofn.nFilterIndex = 0;
	ofn.lpstrInitialDir = "./rom/";
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
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
	cpu->memory.prg_lowpage = 0;
	cpu->memory.prg_highpage = rom->PRG_ROM_pages-1;
	if (rom->Mapper == 104) cpu->memory.prg_highpage = 15; // Golden five fix

	log->Success("%dB PRG_ROM copied", rom->PRG_ROM_pages*16*1024);


	if (rom->CHR_ROM_pages>1)
	{
		log->Error("CHR_ROM pages > 1 will cause crash (no mappers supported). Breaking.");
		return 1;
	}
	memcpy( cpu->ppu.memory, rom->CHR_ROM, rom->CHR_ROM_pages*8*1024 );
	log->Success("%dB CHR_ROM copied", rom->CHR_ROM_pages*8*1024 );

	log->Info("CPU Reset");
	cpu->memory.ppu = &cpu->ppu;
	cpu->memory.apu = &cpu->apu;
	cpu->ppu.Mirroring = rom->Mirroring;
	cpu->Reset();


	SDL_Event event;

	bool EmulationPaused = false;

	int64_t cycles = 0;
	bool DoExit = false;
	while( DoExit == false )
	{
		SDL_PollEvent(&event);
		if (event.type == SDL_QUIT) 
		{
			DoExit = true;
			break;
		}
		if (event.type == SDL_SYSWMEVENT)
		{
			if (event.syswm.msg->msg.win.msg == WM_COMMAND)
			{
				switch( LOWORD( event.syswm.msg->msg.win.wParam ) )
				{
					// File
					// -Exit
				case FILE_EXIT:
					DoExit = true;
					break;

					// Emulation
					// -Pause
				case EMULATION_PAUSE:
					if ( CheckMenuItem( Menu, EMULATION_PAUSE, MF_BYCOMMAND ) == MF_UNCHECKED ) 
					{
						CheckMenuItem( Menu, EMULATION_PAUSE, MF_BYCOMMAND | MF_CHECKED );
						EmulationPaused = true;
						log->Info("Emulation paused");
					}
					else 
					{
						CheckMenuItem( Menu, EMULATION_PAUSE, MF_BYCOMMAND | MF_UNCHECKED );
						EmulationPaused = false;
						log->Info("Emulation resumed");
					}
					break;
					// -Reset (soft)
				case EMULATION_RESET_SOFT:
					cpu->Reset(); 
					break;

					// Options
					// -Sound
					// --Enable
				case OPTIONS_SOUND_ENABLED:
					if ( CheckMenuItem( Menu, OPTIONS_SOUND_ENABLED, MF_BYCOMMAND ) == MF_UNCHECKED ) 
					{
						CheckMenuItem( Menu, OPTIONS_SOUND_ENABLED, MF_BYCOMMAND | MF_CHECKED );
						SDL_PauseAudio(0);
						log->Info("Sound: enabled");
					}
					else 
					{
						CheckMenuItem( Menu, OPTIONS_SOUND_ENABLED, MF_BYCOMMAND | MF_UNCHECKED );
						SDL_PauseAudio(1);
						log->Info("Sound: disabled");
					}
					break;


					// Debug
					// -Windows
					// --OAM
				case DEBUG_WINDOWS_OAM:
					if ( CheckMenuItem( Menu, DEBUG_WINDOWS_OAM, MF_BYCOMMAND ) == MF_UNCHECKED ) 
					{
						CheckMenuItem( Menu, DEBUG_WINDOWS_OAM, MF_BYCOMMAND | MF_CHECKED );
						ToolboxOAM = new DlgOAM(cpu);
					}
					else 
					{
						CheckMenuItem( Menu, DEBUG_WINDOWS_OAM, MF_BYCOMMAND | MF_UNCHECKED );
						if (ToolboxOAM)
						{
							delete ToolboxOAM;
							ToolboxOAM = NULL;
						}
					}
					break;
					
					// --Palette
				case DEBUG_WINDOWS_PALETTE:
					if ( CheckMenuItem( Menu, DEBUG_WINDOWS_PALETTE, MF_BYCOMMAND ) == MF_UNCHECKED ) 
					{
						CheckMenuItem( Menu, DEBUG_WINDOWS_PALETTE, MF_BYCOMMAND | MF_CHECKED );
						ToolboxPalette = new DlgPalette(cpu);
					}
					else 
					{
						CheckMenuItem( Menu, DEBUG_WINDOWS_PALETTE, MF_BYCOMMAND | MF_UNCHECKED );
						if (ToolboxPalette)
						{
							delete ToolboxPalette;
							ToolboxPalette = NULL;
						}
					}
					break;
					
					// --Nametable
				case DEBUG_WINDOWS_NAMETABLE:
					if ( CheckMenuItem( Menu, DEBUG_WINDOWS_NAMETABLE, MF_BYCOMMAND ) == MF_UNCHECKED ) 
					{
						CheckMenuItem( Menu, DEBUG_WINDOWS_NAMETABLE, MF_BYCOMMAND | MF_CHECKED );
						ToolboxNametable = new DlgNametable(cpu);
					}
					else 
					{
						CheckMenuItem( Menu, DEBUG_WINDOWS_NAMETABLE, MF_BYCOMMAND | MF_UNCHECKED );
						if (ToolboxNametable)
						{
							delete ToolboxNametable;
							ToolboxNametable = NULL;
						}
					}
					break;

					// Help
					// -About
				case HELP_ABOUT:
					DlgAbout DialogAbout(MainWindowHwnd);
					break;
				}
			}
		}

		if (XboxPresent )
		{
			if ((tick%1000) == 0)
			{
				buttonState = 0;
				XInputGetState(0, &xstate);
				if (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_B) buttonState |= 1<<7;
				if (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_A) buttonState |= 1<<6;
				if (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) buttonState |= 1<<5;
				if (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_START) buttonState |= 1<<4;
				if (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) buttonState |= 1<<3;
				if (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) buttonState |= 1<<2;
				if (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) buttonState |= 1<<1;
				if (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) buttonState |= 1<<0;
			}
		}
		else buttonState = 0;

		Uint8 *keys = SDL_GetKeyboardState(NULL);
		if ( keys[SDL_SCANCODE_ESCAPE] ) break;
		//A, B, Select, Start, Up, Down, Left, Right.
		if ( keys[SDL_SCANCODE_X] ) buttonState |= 1<<7;
		if ( keys[SDL_SCANCODE_Z] ) buttonState |= 1<<6;
		if ( keys[SDL_SCANCODE_A] ) buttonState |= 1<<5;
		if ( keys[SDL_SCANCODE_S] ) buttonState |= 1<<4;
		if ( keys[SDL_SCANCODE_UP] ) buttonState |= 1<<3;
		if ( keys[SDL_SCANCODE_DOWN] ) buttonState |= 1<<2;
		if ( keys[SDL_SCANCODE_LEFT] ) buttonState |= 1<<1;
		if ( keys[SDL_SCANCODE_RIGHT] ) buttonState |= 1<<0;

		if (!EmulationPaused)
		{
			for (int i = (cycles==0)?3:cycles*3; i>0; i--)
			{
				uint8_t ppuresult = cpu->ppu.Step();
				if (ppuresult) // NMI requested
				{
					if (ppuresult == 100) 
					{
						static int ToolboxDelay = 0; // Performacne hit
						if (ToolboxDelay%5 == 0)
						{
							if (ToolboxOAM) ToolboxOAM->Update();
							if (ToolboxPalette) ToolboxPalette->Update();
							if (ToolboxNametable) ToolboxNametable->Update();
						}
						ToolboxDelay++;
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
			cycles = cpu->Step();
			++tick;
		}
		else
		{
			SDL_WaitEvent(NULL);
		}
	}

	SDL_PauseAudio(1);
	SDL_CloseAudio();
	delete rom; rom = NULL;
	delete cpu; cpu = NULL;

	delete ToolboxOAM; ToolboxOAM = NULL;
	delete ToolboxPalette; ToolboxPalette = NULL;
	delete ToolboxNametable; ToolboxNametable = NULL;
	
	SDL_FreeSurface( canvas ); canvas = NULL;
	SDL_DestroyWindow( MainWindow ); MainWindow = NULL;
	SDL_Quit();
	delete log; log = NULL;
	return 0;
}