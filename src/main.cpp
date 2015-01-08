int buttonState = 0;
#include <windows.h>
#include <xinput.h>
#include <SDL.h>
#include <SDL_syswm.h>
#undef main
#include "headers.h"
#include "resource.h"

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

DlgOAM *ToolboxOAM;
DlgPalette *ToolboxPalette;
DlgNametable *ToolboxNametable;
DlgPatterntable *ToolboxPatterntable;
DlgRAM *ToolboxRAM;
SDL_Window* mainWindow;

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

std::string getFilename(std::string name)
{
	std::size_t found = name.find_last_of('.');
	if (found == std::string::npos) return name;
	else return name.substr(0, found);
}

std::string getExtension(std::string name)
{
	std::size_t found = name.find_last_of('.');
	if (found == std::string::npos) return "";
	else return name.substr(found + 1);
}


EmuState::State EmulatorState = EmuState::Idle;

SDL_Event event;


void audiocallback(void *userdata, Uint8 *stream, int len)
{
	memset(stream, 0x7f, len);
	//if (cpu)
	//{
	//	cpu->apu.audiocallback( userdata, stream, len );
	//}
}

// Loads nsf with path as argument
bool LoadNSF( const char* path )
{
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
//
//	Log->Info("Creating CPU interpreter");
//	cpu = new CPU_interpreter();
//
//	cpu->memory.mapper = 0;
//
//	memcpy( cpu->memory.prg_rom+(nsf->load_address-0x8000), nsf->data, nsf->size );
//	cpu->memory.prg_pages = 2;
//	cpu->memory.prg_lowpage = 0;
//	cpu->memory.prg_highpage = 1;
//
//	Log->Success("%dB bytes of NSF data copid", nsf->size);
//
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
//		while ( !returned )
//		{
//			cpu->Step();
//			if (cpu->PC == 0xfff0+1) returned = true;
//		}
//
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
//		newticks = SDL_GetTicks();
//		delta = newticks - ticks;
//		Sleep( (delta>0)?0:16-delta);
//		ticks = newticks;
//	}
//PlayerExit:
//	Log->Info("Player exited.");
	return 0;
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
	requested.freq = 44100;
	requested.samples = 2048;
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

void clearWindow( SDL_Window* window)
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

	clearWindow(mainWindow);
	return mainWindow;
}

void CloseGame( )
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
	clearWindow(mainWindow);

	Log->Info("Emulation closed");
}

int main( int argc, char *argv[] )
{
#ifdef _DEBUG
	Log->Verbose();
#endif

	Log->Info("AnotherNES version %d.%d", MAJOR_VERSION, MINOR_VERSION);
	
	if (!initializeSDL()) return 1;
	initializeAudio();
	initializeXInput();

	mainWindow = createMainWindow();
	if (!mainWindow) return 1;
	
	SDL_SysWMinfo WindowInfo;
	SDL_VERSION(&WindowInfo.version);
	SDL_GetWindowWMInfo( mainWindow, &WindowInfo);

	HWND MainWindowHwnd = WindowInfo.info.win.window;

	// Menu
	Menu = LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCE(RES_MENU));
	if (!SetMenu( MainWindowHwnd, Menu ))
	{
		Log->Fatal("Problem loading resource (menu)");
		return 1;
	}
	SDL_EventState( SDL_SYSWMEVENT, SDL_ENABLE );

	//SDL_DisplayMode mode;
	//mode.format = SDL_PIXELFORMAT_RGBA8888;
	//mode.w = 256*2;
	//mode.h = 240*2;
	//mode.refresh_rate = 0;
	//mode.driverdata = 0;


	//if ( SDL_SetWindowDisplayMode( MainWindow, &mode ) < 0 )
	//{
	//	Log->Fatal("SDL_SetWindowDisplayMode error");
	//	return 1;
	//}


	SDL_Surface* screen = SDL_GetWindowSurface( mainWindow );

	SDL_Surface* canvas = SDL_CreateRGBSurface( 0, 256, 240, 32, 0, 0, 0, 0xff000000 );
	if (!canvas) Log->Fatal("Cannot create canvas surface!");

	
	EmulatorState = EmuState::Idle;
	NES nes;

	bool FrameLimit = true;
	Uint32 oldticks = 0, ticks = 0;
	bool mouseUp = true;

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
		if (event.type == SDL_DROPFILE)
		{
			static Uint32 prevtimestamp = 0;
			if (event.drop.timestamp != prevtimestamp) 
			{
				Log->Info("File dropped on window.");
				prevtimestamp = event.drop.timestamp;

				std::string filename = event.drop.file;
				SDL_free(event.drop.file);

				if (getExtension(filename).compare("nsf") == 0)
				{
					if (LoadNSF(filename.c_str())) Log->Error("File %s opening problem.", filename.c_str());
				}
				else
				{
					if (EmulatorState != EmuState::Idle) CloseGame();
					if (nes.loadGame(filename.c_str()))	Log->Error("File %s opening problem.", filename.c_str());
					else
					{
						EnableMenuItem(Menu, FILE_CLOSE, MF_BYCOMMAND | MF_ENABLED);

						CheckMenuItem(Menu, EMULATION_PAUSE, MF_BYCOMMAND | MF_UNCHECKED);
						EnableMenuItem(Menu, EMULATION_PAUSE, MF_BYCOMMAND | MF_ENABLED);

						EnableMenuItem(Menu, EMULATION_RESET_SOFT, MF_BYCOMMAND | MF_ENABLED);
						EnableMenuItem(Menu, EMULATION_RESET_HARD, MF_BYCOMMAND | MF_ENABLED);

						clearWindow(mainWindow);
						EmulatorState = EmuState::Running;
					}
				}
			}
		}
		if (event.type == SDL_WINDOWEVENT)
		{
			if ( event.window.event == SDL_WINDOWEVENT_CLOSE )
			{
				int ID = event.window.windowID;
				if ( ID == SDL_GetWindowID(mainWindow) )
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
		}
		if (event.type == SDL_KEYDOWN)
		{
			if (ToolboxRAM && event.key.windowID == ToolboxRAM->WindowID)
				ToolboxRAM->Key(event.key.keysym.sym);
		}
		if (event.type == SDL_MOUSEMOTION)
		{
			if (ToolboxRAM && event.motion.windowID == ToolboxRAM->WindowID)
				ToolboxRAM->Move(event.motion.x , event.motion.y);
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
						ofn.hwndOwner = MainWindowHwnd;
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
							if ( LoadNSF( (const char*)FileName ) )
							{
								Log->Error("File %s opening problem.", FileName);
								break;
							}
						}
						else
						{
						if (EmulatorState != EmuState::Idle)
							CloseGame();
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

							clearWindow(mainWindow);
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
						CheckMenuItem( Menu, DEBUG_WINDOWS_PALETTE, MF_BYCOMMAND | MF_CHECKED );
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
						CheckMenuItem( Menu, DEBUG_WINDOWS_NAMETABLE, MF_BYCOMMAND | MF_CHECKED );
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
					DlgAbout DialogAbout(MainWindowHwnd);
					break;
				}
			}
		}
		if (PendingEvents) { PendingEvents = false; continue; }

		ticks = SDL_GetTicks();
		if ( EmulatorState == EmuState::Running )
		{
			buttonState = 0;
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

			if (ToolboxRAM) ToolboxRAM->Update();

			if (nes.emulateFrame())
			{
				SDL_LockSurface(canvas);
				nes.render(canvas);
				SDL_UnlockSurface(canvas);

				SDL_SoftStretch(canvas, NULL, screen, NULL);
				SDL_UpdateWindowSurface(mainWindow);
			}
			else EmulatorState = EmuState::Paused;

			ticks = SDL_GetTicks();
			if (FrameLimit)
			{
				int ticksPerFrame = 16; // NTSC, 1/60 == 16.6666ms
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

	delete ToolboxOAM; ToolboxOAM = NULL;
	delete ToolboxPalette; ToolboxPalette = NULL;
	delete ToolboxNametable; ToolboxNametable = NULL;
	delete ToolboxPatterntable; ToolboxPatterntable = NULL;
	
	SDL_FreeSurface( canvas ); canvas = NULL;
	SDL_Quit();
	return 0;
}