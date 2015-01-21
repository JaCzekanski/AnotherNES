#include <windows.h>
#include <xinput.h>
#include <iostream>
#include <SDL.h>
#include <SDL_syswm.h>
#undef main
#include "headers.h"
#include "resource.h"
#include "Utils.h"
#include "iNES.h"
#include "NSF.h"
#include "CPU.h"
#include "CPU_interpreter.h"
#include "APU.h"
#include "NES.h"

#include "dialog/DlgOAM.h"
#include "dialog/DlgPalette.h"
#include "dialog/DlgNametable.h"
#include "Dialog/DlgPatterntable.h"
#include "dialog/DlgAbout.h"
#include "dialog/DlgRAM.h"

#include "version.h"

#include"Mapper\Mapper0.h"
#include"Mapper\Mapper1.h"
#include"Mapper\Mapper2.h"
#include"Mapper\Mapper3.h"
#include"Mapper\Mapper4.h"
#include"Mapper\Mapper7.h"
#include"Mapper\Mapper65.h"
#include"Mapper\Mapper71.h"

#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

DlgOAM *ToolboxOAM;
DlgPalette *ToolboxPalette;
DlgNametable *ToolboxNametable;
DlgPatterntable *ToolboxPatterntable;
DlgRAM *ToolboxRAM;

SDL_Window *mainWindow;
SDL_Renderer* renderer;
HWND mainWindowHwnd;
HMENU Menu;

namespace EmuState
{
	enum State  {
		Idle,
		Running,
		Paused,
		Quited
	};
};

EmuState::State EmulatorState = EmuState::Idle;

void ClearMainWindow()
{
	SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);
	SDL_RenderClear(renderer);
	SDL_RenderPresent(renderer);
}

// Loads rom with path as argument
//bool LoadGame( const char* path )
//{
//	rom = new iNES();
//	if (rom->Load( (const char*)path ))
//	{
//		Log->Error("Cannot load %s", getFilename(path).c_str());
//		return 1;
//	}
//	Log->Success("%s opened", getFilename(path).c_str());
//
//	cpu = new CPU_interpreter();
//
//	Log->Info("Mapper: %d", rom->getMapper());
//	cpu->memory.ppu = &cpu->ppu;
//	cpu->memory.apu = &cpu->apu;
//	cpu->ppu.Mirroring = rom->getMirroring();
//
//	if (rom->getMapper() == 0)  cpu->memory.mapper = new Mapper0(cpu->ppu);
//	else if (rom->getMapper() == 1)  cpu->memory.mapper = new Mapper1(cpu->ppu);
//	else if (rom->getMapper() == 2)  cpu->memory.mapper = new Mapper2(cpu->ppu);
//	else if (rom->getMapper() == 3)  cpu->memory.mapper = new Mapper3(cpu->ppu);
//	else if (rom->getMapper() == 4)  cpu->memory.mapper = new Mapper4(cpu->ppu);
//	else if (rom->getMapper() == 7)  cpu->memory.mapper = new Mapper7(cpu->ppu);
//	else if (rom->getMapper() == 65) cpu->memory.mapper = new Mapper65(cpu->ppu);
//	else if (rom->getMapper() == 71) cpu->memory.mapper = new Mapper71(cpu->ppu);
//	else {
//		Log->Error("Unsupported mapper");
//		return 1;
//	}
//
//	if (rom->PRG_ROM_pages > 0) {
//		cpu->memory.mapper->setPrg(rom->PRG_ROM);
//
//		Log->Success("%dB PRG_ROM copied", rom->PRG_ROM_pages * 16384);
//	}
//
//	if (rom->CHR_ROM_pages > 0) {
//		cpu->memory.mapper->setChr(rom->CHR_ROM);
//		memcpy(cpu->ppu.memory, &rom->CHR_ROM[0], 8192);
//
//		Log->Success("%dB CHR_ROM copied", rom->CHR_ROM_pages * 8192);
//	}
//
//	cpu->Power();
//
//	SDL_PauseAudio(0);
//	return 0;
//}

// Loads nsf with path as argument
//bool LoadNSF( const char* path )
//{
//	Log->Info("Opening %s", path);
//	NSF* nsf = new NSF();
//	if (nsf->Load( (const char*)path ))
//	{
//		Log->Error("Cannot load %s", path);
//		return 0;
//	}
//	Log->Success("%s opened", path);
//	Log->Info("Press escape to exit player");
//
//
//	Log->Info("Creating CPU interpreter");
//	cpu = new CPU_interpreter();
//
//	std::vector<uint8_t> prg;
//	int pages = ((nsf->load_address - 0x8000) + nsf->size + 16384 + 1) / 16384;
//	prg.resize(16384 * pages);
//	memcpy(&prg[nsf->load_address - 0x8000], nsf->data, nsf->size);
//
//	cpu->memory.mapper = new Mapper0(cpu->ppu);
//	cpu->memory.mapper->setPrg(prg);
//		
//	cpu->memory.ppu = &cpu->ppu;
//	cpu->memory.apu = &cpu->apu;
//	cpu->Reset();
//
//	int song = 0;
//	bool key_released = true;
//new_song:
//	Log->Info("Song %d", song);
//
//	cpu->PC = nsf->init_address;
//	cpu->A = song;
//	cpu->X = 0; // ntsc
//
//	for (int i = 0; i<=0x07ff; i++) cpu->memory.Write(i,0);
//	for (int i = 0x6000; i<=0x7fff; i++) cpu->memory.Write(i,0);
//	for (int i = 0x4000; i<=0x400f; i++) cpu->memory.Write(i,0);
//	cpu->memory.Write(0x4010,0x10);
//	for (int i = 0x4011; i<=0x4013; i++) cpu->memory.Write(i,0);
//
//	cpu->memory.Write(0x4017,0x40);
//	cpu->memory.Write(0x4015,0x0f);
//
//	// c->Push16(c->PC+2); // Corrected
//
//	cpu->Push16(0xfff0);
//	bool returned = false;
//	while ( !returned )
//	{
//		cpu->Step();
//			if (cpu->PC == 0xfff0+1) returned = true;
//	}
//	SDL_PauseAudio(0);
//
//	while (1)
//	{
//		cpu->PC = nsf->play_address;
//		//cpu->SP -= 2;
//		cpu->Push16(0xfff0);
//		returned = false;
//		Uint32 ticks = SDL_GetTicks();
//		Uint32 newticks =0;
//		Uint32 delta = 0;
//		int apu_cycles = 0;
//		while ( !returned )
//		{
//			cpu->Step();
//			if (cpu->PC == 0xfff0 + 1) returned = true;
//		}
//		cpu->apu.activeTimer++;
//
//		SDL_Event event;
//		while (1)
//		{
//			int PendingEvents = SDL_PollEvent(&event);
//			if (event.type == SDL_KEYDOWN && key_released)
//			{
//				key_released = false;
//				if (event.key.keysym.sym == SDLK_LEFT)
//				{
//					if (song > 0) song--;
//					goto new_song;
//				}
//				if (event.key.keysym.sym == SDLK_RIGHT)
//				{
//					/*if (song > 0) */song++;
//					goto new_song;
//				}
//				if (event.key.keysym.sym == SDLK_ESCAPE) goto PlayerExit;
//			}
//			if (event.type == SDL_KEYUP) key_released = true;
//			if (!PendingEvents) break;
//		}
//
//		cpu->apu.frameStep();
//		cpu->apu.frameStep();
//		cpu->apu.frameStep();
//		cpu->apu.frameStep();
//		int ticksPerFrame = 16; // NTSC, 1/60 == 16.6666ms
//		//if (nsf->pal_ntsc_bits) ticksPerFrame = 20; // Pal, 1/50 == 20ms
//
//		newticks = SDL_GetTicks();
//		while (newticks - ticks < ticksPerFrame)
//		{
//			SDL_Delay(1);
//			newticks = SDL_GetTicks();
//		}
//		ticks = newticks;
//	}
//PlayerExit:
//	Log->Info("Player exited.");
//	return 0;
//}

void CloseGame()
{
	if ( ToolboxNametable )	{
		delete ToolboxNametable;
		ToolboxNametable = NULL;
		CheckMenuItem( Menu, DEBUG_WINDOWS_NAMETABLE, MF_BYCOMMAND | MF_UNCHECKED );
	}
	if (ToolboxPatterntable)	{
		delete ToolboxPatterntable;
		ToolboxPatterntable = NULL;
		CheckMenuItem(Menu, DEBUG_WINDOWS_PATTERNTABLE, MF_BYCOMMAND | MF_UNCHECKED);
	}
	if ( ToolboxOAM )	{
		delete ToolboxOAM;
		ToolboxOAM = NULL;
		CheckMenuItem( Menu, DEBUG_WINDOWS_OAM, MF_BYCOMMAND | MF_UNCHECKED );
	}
	if ( ToolboxPalette )	{
		delete ToolboxPalette;
		ToolboxPalette = NULL;
		CheckMenuItem( Menu, DEBUG_WINDOWS_PALETTE, MF_BYCOMMAND | MF_UNCHECKED );
	}
	if (ToolboxRAM)	{
		delete ToolboxRAM;
		ToolboxRAM = NULL;
		CheckMenuItem(Menu, DEBUG_WINDOWS_RAM, MF_BYCOMMAND | MF_UNCHECKED);
	}
	if (EmulatorState == EmuState::Idle) return;
	EmulatorState = EmuState::Idle;
	EnableMenuItem( Menu, FILE_CLOSE, MF_BYCOMMAND | MF_GRAYED );
	
	CheckMenuItem( Menu, EMULATION_PAUSE, MF_BYCOMMAND | MF_UNCHECKED );
	EnableMenuItem( Menu, EMULATION_PAUSE, MF_BYCOMMAND | MF_GRAYED );

	EnableMenuItem( Menu, EMULATION_RESET_SOFT, MF_BYCOMMAND | MF_GRAYED );
	EnableMenuItem( Menu, EMULATION_RESET_HARD, MF_BYCOMMAND | MF_GRAYED );

	EmulatorState = EmuState::Idle;

	SDL_PauseAudio(1);
	//delete rom; rom = NULL;
	//delete cpu; cpu = NULL;

	ClearMainWindow();

	Log->Info("Emulation closed");
}

void audiocallback(void *userdata, Uint8 *stream, int len)
{
	memset(stream, 0x7f, len);
	//if (cpu) cpu->apu.audiocallback(userdata, stream, len);
}

bool initializeSDL()
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
	{
		Log->Fatal("SDL_Init failed");
		return false;
	}
	return true;
}

bool initializeAudio()
{
	SDL_AudioSpec requested, obtained;
	requested.channels = 1;
	requested.format = AUDIO_U8;
	requested.freq = 22050;
	requested.samples = 512;
	requested.callback = audiocallback;
	if (SDL_OpenAudio(&requested, &obtained) == -1)
	{
		Log->Error("SDL_OpenAudio error.");
		return false;
	}
	SDL_PauseAudio(1);
	return true;
}

bool XboxPresent = false;
void initializeXInput()
{
	XINPUT_STATE xstate;
	if (XInputGetState(0, &xstate) == ERROR_SUCCESS)
	{
		Log->Success("XInput: Xbox360 controller connected.");
		XboxPresent = true;
	}
	else
		Log->Info("XInput: No Xbox360 controller found.");
}

void clearWindow(SDL_Window* window)
{
	SDL_Surface *tns = SDL_GetWindowSurface(window);
	SDL_FillRect(tns, NULL, 0x000000);
	SDL_UpdateWindowSurface(window);
}

SDL_Window* createMainWindow()
{
	SDL_Window* mainWindow = SDL_CreateWindow("AnotherNES",
		542, 20,
		256 * 2, 240 * 2 + 20, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

	if (mainWindow == NULL)
	{
		Log->Fatal("Cannot create main window");
		return false;
	}
	Log->Success("Main window created");

	SDL_Surface *icon = SDL_LoadBMP("icon.bmp");
	if (!icon)
		Log->Error("Cannot load icon.bmp");
	else
	{
		SDL_SetWindowIcon(mainWindow, icon);
		SDL_FreeSurface(icon);
	}

	SDL_SysWMinfo WindowInfo;
	SDL_VERSION(&WindowInfo.version);
	SDL_GetWindowWMInfo(mainWindow, &WindowInfo);

	mainWindowHwnd = WindowInfo.info.win.window;

	// Menu
	Menu = LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCE(RES_MENU));
	if (!SetMenu(mainWindowHwnd, Menu))
	{
		Log->Fatal("Problem loading resource (menu)");
		return NULL;
	}
	SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);

	return mainWindow;
}

SDL_Renderer* createRenderer(SDL_Window* window)
{
	SDL_Renderer *renderer = SDL_CreateRenderer(mainWindow, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == NULL)
	{
		Log->Error("Cannot create renderer");
		return NULL;
	}
	return renderer;
}


int main( int argc, char *argv[] )
{
	Log->Verbose();
	Log->setProgramName("AnotherNES");
	Log->Info("AnotherNES version %d.%d", MAJOR_VERSION, MINOR_VERSION);
	
	if (!initializeSDL()) return 1;
	initializeAudio();
	initializeXInput();
	
	// Texture filtering
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

	mainWindow = createMainWindow();
	if (!mainWindow) return 1;

	renderer = createRenderer(mainWindow);
	if (!renderer) return 1;

	SDL_Texture* canvas = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 256, 240);
	if (!canvas) Log->Fatal("Cannot create canvas texture!");
	
	EmulatorState = EmuState::Idle;
	NES nes;

	bool FrameLimit = true;
	Uint32 oldticks = 0, ticks = 0;
	Uint32 prevtimestamp = 0;
	bool mouseUp = true;
	int cycles = 0;
	int apu_cycles = 0;
	SDL_Event event;
	while( EmulatorState != EmuState::Quited )
	{
		int PendingEvents = false;
		if ( EmulatorState != EmuState::Running ) SDL_WaitEvent(&event);
		else PendingEvents = SDL_PollEvent(&event);
		if (event.type == SDL_QUIT) 
		{
			EmulatorState = EmuState::Quited;
			break;
		}
		if (event.type == SDL_DROPFILE && event.drop.timestamp != prevtimestamp)
		{
			Log->Info("File dropped on window.");
			prevtimestamp = event.drop.timestamp;
			
			char FileName[2048];
			strcpy(FileName, event.drop.file);
			SDL_free(event.drop.file);

			if (memcmp(FileName + strlen((const char*)FileName) - 3, "nsf", 3) == 0) // NSF
			{
				// TODO
				//if (LoadNSF((const char*)FileName))
				//{
				//	Log->Error("File %s opening problem.", FileName);
				//	break;
				//}
			}
			else
			{
				if (EmulatorState != EmuState::Idle)
				{
					CloseGame();
				}
				if (nes.loadGame((const char*)FileName))
				{
					Log->Error("File %s opening problem.", FileName);
				}
				else
				{
					EnableMenuItem(Menu, FILE_CLOSE, MF_BYCOMMAND | MF_ENABLED);

					CheckMenuItem(Menu, EMULATION_PAUSE, MF_BYCOMMAND | MF_UNCHECKED);
					EnableMenuItem(Menu, EMULATION_PAUSE, MF_BYCOMMAND | MF_ENABLED);

					EnableMenuItem(Menu, EMULATION_RESET_SOFT, MF_BYCOMMAND | MF_ENABLED);
					EnableMenuItem(Menu, EMULATION_RESET_HARD, MF_BYCOMMAND | MF_ENABLED);

					ClearMainWindow();
					EmulatorState = EmuState::Running;
				}
			}
		}
		if (event.type == SDL_WINDOWEVENT)
		{
			if ( event.window.event == SDL_WINDOWEVENT_CLOSE )
			{
				int ID = event.window.windowID;
				if (ID == SDL_GetWindowID(mainWindow))
				{
					event.type = SDL_QUIT;
					SDL_PushEvent(&event);
				}
				else 
				{
					if ( ToolboxNametable )	{
						if (ID == ToolboxNametable->WindowID) {
							delete ToolboxNametable;
							ToolboxNametable = NULL;
							CheckMenuItem( Menu, DEBUG_WINDOWS_NAMETABLE, MF_BYCOMMAND | MF_UNCHECKED );
						}
					}
					if (ToolboxPatterntable)	{
						if (ID == ToolboxPatterntable->WindowID) {
							delete ToolboxPatterntable;
							ToolboxPatterntable = NULL;
							CheckMenuItem(Menu, DEBUG_WINDOWS_PATTERNTABLE, MF_BYCOMMAND | MF_UNCHECKED);
						}
					}
					if ( ToolboxOAM )	{
						if (ID == ToolboxOAM->WindowID) {
							delete ToolboxOAM;
							ToolboxOAM = NULL;
							CheckMenuItem( Menu, DEBUG_WINDOWS_OAM, MF_BYCOMMAND | MF_UNCHECKED );
						}
					}
					if ( ToolboxPalette )	{
						if (ID == ToolboxPalette->WindowID) {
							delete ToolboxPalette;
							ToolboxPalette = NULL;
							CheckMenuItem( Menu, DEBUG_WINDOWS_PALETTE, MF_BYCOMMAND | MF_UNCHECKED );
						}
					}
					if (ToolboxRAM)	{
						if (ID == ToolboxRAM->WindowID) {
							delete ToolboxRAM;
							ToolboxRAM = NULL;
							CheckMenuItem(Menu, DEBUG_WINDOWS_RAM, MF_BYCOMMAND | MF_UNCHECKED);
						}
					}

				}
			}
			else if (event.window.event == SDL_WINDOWEVENT_RESIZED)
			{
				//screen = SDL_GetWindowSurface(MainWindow);
				ClearMainWindow();
			}
		}
		if (event.type == SDL_KEYDOWN)
		{
			if (ToolboxRAM && event.key.windowID == ToolboxRAM->WindowID)
			{
				ToolboxRAM->Key(event.key.keysym.sym);
			}
		}
		if (event.type == SDL_MOUSEMOTION)
		{
			if (ToolboxRAM && event.motion.windowID == ToolboxRAM->WindowID)
			{
				ToolboxRAM->Move(event.motion.x , event.motion.y);
			}
		}
		if (event.type == SDL_MOUSEBUTTONDOWN)
		{
			if (mouseUp && ToolboxRAM && event.button.windowID == ToolboxRAM->WindowID)
			{
				ToolboxRAM->Click(event.button.x, event.button.y, event.button.button & 0xff); 
				mouseUp = false;
			}
		}
		if (event.type == SDL_MOUSEBUTTONUP)
		{
			mouseUp = true;
		}
		if (event.type == SDL_SYSWMEVENT)
		{
			if (event.syswm.msg->msg.win.msg == WM_COMMAND)
			{
				switch( LOWORD( event.syswm.msg->msg.win.wParam ) )
				{
					// File
					// -Open
				case FILE_OPEN:
					unsigned char FileName[2048];
						OPENFILENAME ofn;
						memset( &ofn, 0, sizeof(ofn) );
						ofn.lStructSize = sizeof( ofn );
						ofn.hwndOwner = mainWindowHwnd;
						ofn.hInstance = GetModuleHandle(NULL);
						memset(FileName, 0, sizeof(FileName) );
						ofn.lpstrFile = (char*)FileName;
						ofn.nMaxFile = sizeof(FileName)-1;
						ofn.lpstrFilter = "All supported files\0*.nes;*.nsf\0"
										  "NES files\0*.nes\0"
										  "NSF files\0*.nsf\0";
						ofn.nFilterIndex = 0;
						ofn.lpstrInitialDir = "./rom/";
						ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
						if (!GetOpenFileName( &ofn ))
						{
							Log->Debug("GetOpenFileName: No file selected.");
							break;
						}
						if ( memcmp( FileName+strlen((const char*)FileName)-3, "nsf", 3 ) == 0) // NSF
						{
							// TODO
							//if ( LoadNSF( (const char*)FileName ) )
							//{
							//	Log->Error("File %s opening problem.", FileName);
							//	break;
							//}
						}
						else
						{
						if (EmulatorState != EmuState::Idle)
						{
							CloseGame();
						}
							if ( nes.loadGame( (const char*)FileName ) )
							{
								Log->Error("File %s opening problem.", FileName);
								break;
							}

							EnableMenuItem( Menu, FILE_CLOSE, MF_BYCOMMAND | MF_ENABLED );
							
							CheckMenuItem( Menu, EMULATION_PAUSE, MF_BYCOMMAND | MF_UNCHECKED );
							EnableMenuItem( Menu, EMULATION_PAUSE, MF_BYCOMMAND | MF_ENABLED );

							EnableMenuItem( Menu, EMULATION_RESET_SOFT, MF_BYCOMMAND | MF_ENABLED );
							EnableMenuItem( Menu, EMULATION_RESET_HARD, MF_BYCOMMAND | MF_ENABLED );

							ClearMainWindow();
							EmulatorState = EmuState::Running;
						}
					break;

					// -Close
				case FILE_CLOSE:
					if (EmulatorState != EmuState::Idle)
					{
						CloseGame();
					}
					break;

					// -Exit
				case FILE_EXIT:
					EmulatorState = EmuState::Quited;
					break;

					// Emulation
					// -Pause
				case EMULATION_PAUSE:
					if ( CheckMenuItem( Menu, EMULATION_PAUSE, MF_BYCOMMAND ) == MF_UNCHECKED ) 
					{
						if (EmulatorState == EmuState::Running) 
						{
							CheckMenuItem( Menu, EMULATION_PAUSE, MF_BYCOMMAND | MF_CHECKED );
							EmulatorState = EmuState::Paused;
							SDL_PauseAudio(1);
							Log->Info("Emulation paused");
						}
					}
					else 
					{
						if (EmulatorState == EmuState::Paused) 
						{
							CheckMenuItem( Menu, EMULATION_PAUSE, MF_BYCOMMAND | MF_UNCHECKED );
							EmulatorState = EmuState::Running;
							SDL_PauseAudio(0);
							Log->Info("Emulation resumed");
						}
					}
					break;
					// -Reset (soft)
				case EMULATION_RESET_SOFT:
					nes.reset(); 
					break;

					// Options
					// -Sound
					// --Enable
				case OPTIONS_SOUND_ENABLED:
					if ( CheckMenuItem( Menu, OPTIONS_SOUND_ENABLED, MF_BYCOMMAND ) == MF_UNCHECKED ) 
					{
						CheckMenuItem( Menu, OPTIONS_SOUND_ENABLED, MF_BYCOMMAND | MF_CHECKED );
						SDL_PauseAudio(0);
						Log->Info("Sound: enabled");
					}
					else 
					{
						CheckMenuItem( Menu, OPTIONS_SOUND_ENABLED, MF_BYCOMMAND | MF_UNCHECKED );
						SDL_PauseAudio(1);
						Log->Info("Sound: disabled");
					}
					break;


					// Debug
					// -Windows
					// --OAM
				case DEBUG_WINDOWS_OAM:
					if ( CheckMenuItem( Menu, DEBUG_WINDOWS_OAM, MF_BYCOMMAND ) == MF_UNCHECKED ) 
					{
						CheckMenuItem( Menu, DEBUG_WINDOWS_OAM, MF_BYCOMMAND | MF_CHECKED );
						// TODO
						//ToolboxOAM = new DlgOAM(cpu);
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
						CheckMenuItem(Menu, DEBUG_WINDOWS_PALETTE, MF_BYCOMMAND | MF_CHECKED);
						// TODO
						//ToolboxPalette = new DlgPalette(cpu);
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
						CheckMenuItem(Menu, DEBUG_WINDOWS_NAMETABLE, MF_BYCOMMAND | MF_CHECKED);
						// TODO
						//ToolboxNametable = new DlgNametable(cpu);
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

					// --Nametable

				case DEBUG_WINDOWS_PATTERNTABLE:
					if (CheckMenuItem(Menu, DEBUG_WINDOWS_PATTERNTABLE, MF_BYCOMMAND) == MF_UNCHECKED)
					{
						CheckMenuItem(Menu, DEBUG_WINDOWS_PATTERNTABLE, MF_BYCOMMAND | MF_CHECKED);
						// TODO
						//ToolboxPatterntable = new DlgPatterntable(cpu);
					}
					else
					{
						CheckMenuItem(Menu, DEBUG_WINDOWS_PATTERNTABLE, MF_BYCOMMAND | MF_UNCHECKED);
						if (ToolboxPatterntable)
						{
							delete ToolboxPatterntable;
							ToolboxPatterntable = NULL;
						}
					}
					break;

					// --Patterntable
				case DEBUG_WINDOWS_RAM:
					if (CheckMenuItem(Menu, DEBUG_WINDOWS_RAM, MF_BYCOMMAND) == MF_UNCHECKED)
					{
						CheckMenuItem(Menu, DEBUG_WINDOWS_RAM, MF_BYCOMMAND | MF_CHECKED);
						// TODO
						//ToolboxRAM = new DlgRAM(cpu);
					}
					else
					{
						CheckMenuItem(Menu, DEBUG_WINDOWS_RAM, MF_BYCOMMAND | MF_UNCHECKED);
						if (ToolboxRAM)
						{
							delete ToolboxRAM;
							ToolboxRAM = NULL;
						}
					}
					break;

					// Help
					// -About
				case HELP_ABOUT:
					DlgAbout DialogAbout(mainWindowHwnd);
					break;
				}
			}
		}

		if (PendingEvents) { PendingEvents = false; continue; }

		ticks = SDL_GetTicks();
		if ( EmulatorState == EmuState::Running )
		{
			int buttonState = 0;
			if ( XboxPresent )
			{
				XINPUT_STATE xstate;
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
			Uint8 *keys = (Uint8*) SDL_GetKeyboardState(NULL);
			if ( keys[SDL_SCANCODE_ESCAPE] ) break;
			//A, B, Select, Start, Up, Down, Left, Right.
			if ( keys[SDL_SCANCODE_X] ) buttonState |= 1<<7;
			if ( keys[SDL_SCANCODE_Z] ) buttonState |= 1<<6;
			if ( keys[SDL_SCANCODE_A] ) buttonState |= 1<<5;
			if ( keys[SDL_SCANCODE_S] ) buttonState |= 1<<4;
			if ( keys[SDL_SCANCODE_UP] ) buttonState |= 1<<3;
			if ( keys[SDL_SCANCODE_DOWN] ) buttonState |= 1<<2;
			if ( keys[SDL_SCANCODE_LEFT] ) buttonState |= 1<<1;
			if ( keys[SDL_SCANCODE_RIGHT] ) buttonState |= 1 << 0;
			if ( keys[SDL_SCANCODE_SPACE] ) FrameLimit = false;
			else FrameLimit = true;

			nes.setInput(buttonState);

			//bool framerefresh = false;
			//while ( !framerefresh )
			//{
			//	for (int i = cycles*3; i>0; i--)
			//	{
			//		if (i%3 == 0) cycles--;
			//		uint8_t ppuresult = cpu->ppu.Step();
			//		if (ppuresult) // NMI requested
			//		{
			//			static int ToolboxDelay = 0; // Performacne hit
			//			if (ToolboxDelay%5 == 0)
			//			{
			//				if (ToolboxOAM) ToolboxOAM->Update();
			//				if (ToolboxPalette) ToolboxPalette->Update();
			//				if (ToolboxNametable) ToolboxNametable->Update();
			//				if (ToolboxPatterntable) ToolboxPatterntable->Update();
			//			}
			//			ToolboxDelay++;

			//			framerefresh = true;

			//			cpu->ppu.Render( canvas );
			//			SDL_RenderCopy(renderer, canvas, NULL, NULL);
			//				
			//			if (ppuresult == 2)
			//			{
			//				cpu->NMI();
			//				cycles += 7;
			//				i += 7 * 3;
			//			}
			//		}
			//		if (cpu->ppu.cycles == 260) {
			//			Mapper4 *MMC3 = dynamic_cast<Mapper4*>(cpu->memory.mapper);
			//			if (MMC3 && MMC3->MMC3_irqEnabled && MMC3->MMC3_irqCounter == cpu->ppu.scanline)
			//			{
			//				cpu->IRQ();
			//			}
			//		}
			//	}
			//	cycles += cpu->Step();
			//	apu_cycles += cycles;
			//	if (cpu->isJammed())
			//	{
			//		Log->Error("CPU Jammed.");
			//		framerefresh = true;
			//		EmulatorState = EmuState::Paused;
			//	}
			//	if (apu_cycles >= 7457)
			//	{
			//		cpu->apu.frameStep();
			//		apu_cycles = 0;
			//	}
			//	Mapper65 *mapper65 = dynamic_cast<Mapper65*>(cpu->memory.mapper);
			//	if (mapper65 && mapper65->irqEnabled) {
			//		if (mapper65->irqCounter <= cycles) cpu->IRQ();
			//		mapper65->irqCounter -= cycles;
			//	}
			//	cpu->apu.activeTimer++;
			//}

			//if (ToolboxRAM) ToolboxRAM->Update();

			if (nes.emulateFrame())
			{
				nes.render(canvas);
				SDL_RenderCopy(renderer, canvas, NULL, NULL);
				SDL_RenderPresent(renderer);
			}
			else EmulatorState = EmuState::Paused;

			ticks = SDL_GetTicks();
			if (FrameLimit)
			{
				Uint32 ticksPerFrame = 16; // NTSC, 1/60 == 16.6666ms
				if (nes.getRegion() == Region::PAL) ticksPerFrame = 20; // Pal, 1/50 == 20ms
					
				while (ticks - oldticks < ticksPerFrame)
				{
					SDL_Delay(1);
					ticks = SDL_GetTicks();
				}	
			}

			oldticks = ticks;
		}
	}

	SDL_PauseAudio(1);
	SDL_CloseAudio();

	delete ToolboxOAM;
	delete ToolboxPalette;
	delete ToolboxNametable;
	delete ToolboxPatterntable;
	
	SDL_DestroyTexture(canvas);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(mainWindow);
	SDL_Quit();
	return 0;
}