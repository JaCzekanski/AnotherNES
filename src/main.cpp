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

enum class EmulatorState
{
	Idle,
	Running,
	Paused,
	Quited
};

EmulatorState emulatorState = EmulatorState::Idle;

void ClearMainWindow()
{
	SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);
	SDL_RenderClear(renderer);
	SDL_RenderPresent(renderer);
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

bool initializeAudio( NES* nes )
{
	SDL_AudioSpec requested, obtained;
	requested.channels = 1;
	requested.format = AUDIO_U8;
	requested.freq = 22050;
	requested.samples = 512;
	requested.callback = audiocallback;
	requested.userdata = &nes->getCPU()->apu;
	if (SDL_OpenAudio(&requested, &obtained) == -1)
	{
		Log->Error("SDL_OpenAudio error.");
		return false;
	}
	SDL_PauseAudio(0);
	return true;
}

void closeAudio()
{
	SDL_PauseAudio(1);
	SDL_CloseAudio();
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

void CloseGame()
{
	if (ToolboxNametable)	{
		delete ToolboxNametable;
		ToolboxNametable = NULL;
		CheckMenuItem(Menu, DEBUG_WINDOWS_NAMETABLE, MF_BYCOMMAND | MF_UNCHECKED);
	}
	if (ToolboxPatterntable)	{
		delete ToolboxPatterntable;
		ToolboxPatterntable = NULL;
		CheckMenuItem(Menu, DEBUG_WINDOWS_PATTERNTABLE, MF_BYCOMMAND | MF_UNCHECKED);
	}
	if (ToolboxOAM)	{
		delete ToolboxOAM;
		ToolboxOAM = NULL;
		CheckMenuItem(Menu, DEBUG_WINDOWS_OAM, MF_BYCOMMAND | MF_UNCHECKED);
	}
	if (ToolboxPalette)	{
		delete ToolboxPalette;
		ToolboxPalette = NULL;
		CheckMenuItem(Menu, DEBUG_WINDOWS_PALETTE, MF_BYCOMMAND | MF_UNCHECKED);
	}
	if (ToolboxRAM)	{
		delete ToolboxRAM;
		ToolboxRAM = NULL;
		CheckMenuItem(Menu, DEBUG_WINDOWS_RAM, MF_BYCOMMAND | MF_UNCHECKED);
	}
	if (emulatorState == EmulatorState::Idle) return;
	emulatorState = EmulatorState::Idle;
	EnableMenuItem(Menu, FILE_CLOSE, MF_BYCOMMAND | MF_GRAYED);

	CheckMenuItem(Menu, EMULATION_PAUSE, MF_BYCOMMAND | MF_UNCHECKED);
	EnableMenuItem(Menu, EMULATION_PAUSE, MF_BYCOMMAND | MF_GRAYED);

	EnableMenuItem(Menu, EMULATION_RESET_SOFT, MF_BYCOMMAND | MF_GRAYED);
	EnableMenuItem(Menu, EMULATION_RESET_HARD, MF_BYCOMMAND | MF_GRAYED);

	closeAudio();
	ClearMainWindow();

	Log->Info("Emulation closed");
}

int main( int argc, char *argv[] )
{
	Log->Verbose();
	Log->setProgramName("AnotherNES");
	Log->Info("AnotherNES version %d.%d", MAJOR_VERSION, MINOR_VERSION);
	
	if (!initializeSDL()) return 1;
	initializeXInput();
	
	// Texture filtering
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

	mainWindow = createMainWindow();
	if (!mainWindow) return 1;

	renderer = createRenderer(mainWindow);
	if (!renderer) return 1;

	SDL_Texture* canvas = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 256, 240);
	if (!canvas) Log->Fatal("Cannot create canvas texture!");
	
	emulatorState = EmulatorState::Idle;
	NES nes;

	bool FrameLimit = true;
	Uint32 oldticks = 0, ticks = 0;
	Uint32 prevtimestamp = 0;
	bool mouseUp = true;
	int cycles = 0;
	int apu_cycles = 0;
	SDL_Event event;
	while (emulatorState != EmulatorState::Quited)
	{
		int PendingEvents = false;
		if (emulatorState != EmulatorState::Running) SDL_WaitEvent(&event);
		else PendingEvents = SDL_PollEvent(&event);
		if (event.type == SDL_QUIT) 
		{
			emulatorState = EmulatorState::Quited;
			break;
		}
		if (event.type == SDL_DROPFILE && event.drop.timestamp != prevtimestamp)
		{
			Log->Info("File dropped on window.");
			prevtimestamp = event.drop.timestamp;
			
			char FileName[2048];
			strcpy(FileName, event.drop.file);
			SDL_free(event.drop.file);

			if (emulatorState != EmulatorState::Idle) CloseGame();
			if (nes.loadGame((const char*)FileName)) Log->Error("File %s opening problem.", FileName);
			else
			{
				initializeAudio(&nes);
				EnableMenuItem(Menu, FILE_CLOSE, MF_BYCOMMAND | MF_ENABLED);

				CheckMenuItem(Menu, EMULATION_PAUSE, MF_BYCOMMAND | MF_UNCHECKED);
				EnableMenuItem(Menu, EMULATION_PAUSE, MF_BYCOMMAND | MF_ENABLED);

				EnableMenuItem(Menu, EMULATION_RESET_SOFT, MF_BYCOMMAND | MF_ENABLED);
				EnableMenuItem(Menu, EMULATION_RESET_HARD, MF_BYCOMMAND | MF_ENABLED);

				ClearMainWindow();
				emulatorState = EmulatorState::Running;
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
			else if (event.window.event == SDL_WINDOWEVENT_RESIZED)	ClearMainWindow();
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
					ofn.lpstrFilter = "All supported files\0*.nes\0"
										"NES files\0*.nes\0";
					ofn.nFilterIndex = 0;
					ofn.lpstrInitialDir = "./rom/";
					ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
					if (!GetOpenFileName( &ofn ))
					{
						Log->Debug("GetOpenFileName: No file selected.");
						break;
					}
					if (emulatorState != EmulatorState::Idle) CloseGame();
					if ( nes.loadGame( (const char*)FileName ) )
					{
						Log->Error("File %s opening problem.", FileName);
						break;
					}
					initializeAudio(&nes);

					EnableMenuItem( Menu, FILE_CLOSE, MF_BYCOMMAND | MF_ENABLED );
							
					CheckMenuItem( Menu, EMULATION_PAUSE, MF_BYCOMMAND | MF_UNCHECKED );
					EnableMenuItem( Menu, EMULATION_PAUSE, MF_BYCOMMAND | MF_ENABLED );

					EnableMenuItem( Menu, EMULATION_RESET_SOFT, MF_BYCOMMAND | MF_ENABLED );
					EnableMenuItem( Menu, EMULATION_RESET_HARD, MF_BYCOMMAND | MF_ENABLED );

					ClearMainWindow();
					emulatorState = EmulatorState::Running;
					break;

					// -Close
				case FILE_CLOSE:
					if (emulatorState != EmulatorState::Idle) CloseGame();
					break;

					// -Exit
				case FILE_EXIT:
					emulatorState = EmulatorState::Quited;
					break;

					// Emulation
					// -Pause
				case EMULATION_PAUSE:
					if ( CheckMenuItem( Menu, EMULATION_PAUSE, MF_BYCOMMAND ) == MF_UNCHECKED ) 
					{
						if (emulatorState == EmulatorState::Running)
						{
							CheckMenuItem( Menu, EMULATION_PAUSE, MF_BYCOMMAND | MF_CHECKED );
							emulatorState = EmulatorState::Paused;
							SDL_PauseAudio(1);
							Log->Info("Emulation paused");
						}
					}
					else 
					{
						if (emulatorState == EmulatorState::Paused)
						{
							CheckMenuItem( Menu, EMULATION_PAUSE, MF_BYCOMMAND | MF_UNCHECKED );
							emulatorState = EmulatorState::Running;
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
		if (emulatorState == EmulatorState::Running)
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
			if (nes.emulateFrame())
			{
				nes.render(canvas);
				SDL_RenderCopy(renderer, canvas, NULL, NULL);
				SDL_RenderPresent(renderer);
			}
			else {
				CloseGame();
				continue;
			}

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