#include "App.h"
#include <XInput.h>
#include <SDL_syswm.h>
#include <SDL_ttf.h>
#include <algorithm>
#include "resource.h"

#include "Utils.h"

App::App() :emulatorState(EmulatorState::Idle),
			mainWindow(nullptr),
			renderer(nullptr),
			canvas(nullptr),
			mainWindowHwnd(0),
			Menu(nullptr),
			ToolboxOAM(nullptr),
			ToolboxPalette(nullptr),
			ToolboxNametable(nullptr),
			ToolboxPatterntable(nullptr),
			ToolboxRAM(nullptr),
			ToolboxCPU(nullptr),
			mouseUp(true),
			frameLimit(true)
{

}

void App::clearWindow()
{
	SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);
	SDL_RenderClear(renderer);
	SDL_RenderPresent(renderer);
}

bool App::initializeSDL()
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
	{
		Log->Fatal("SDL_Init failed");
		return false;
	}
	return true;
}

bool App::initializeAudio(NES* nes)
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

void App::closeAudio()
{
	SDL_PauseAudio(1);
	SDL_CloseAudio();
}

bool XboxPresent = false;
void App::initializeXInput()
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

SDL_Window* App::createMainWindow()
{
	SDL_Window* mainWindow = SDL_CreateWindow("AnotherNES",
		542, 20,
		256 * 2, 240 * 2 + 20, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

	if (mainWindow == NULL)
	{
		Log->Fatal("Cannot create main window");
		return false;
	}

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

SDL_Renderer* App::createRenderer(SDL_Window* window)
{
	SDL_Renderer *renderer = SDL_CreateRenderer(mainWindow, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == NULL) Log->Error("Cannot create renderer");
	return renderer;
}

bool App::loadGame(std::string path)
{
	if (emulatorState != EmulatorState::Idle) closeGame();
	if (nes.loadGame(path.c_str())) {
		Log->Error("File %s opening problem.", getFilename(path).c_str());
		return false;
	}
	initializeAudio(&nes);
	EnableMenuItem(Menu, FILE_CLOSE, MF_BYCOMMAND | MF_ENABLED);

	CheckMenuItem(Menu, EMULATION_PAUSE, MF_BYCOMMAND | MF_UNCHECKED);
	EnableMenuItem(Menu, EMULATION_PAUSE, MF_BYCOMMAND | MF_ENABLED);

	EnableMenuItem(Menu, EMULATION_RESET_SOFT, MF_BYCOMMAND | MF_ENABLED);
	EnableMenuItem(Menu, EMULATION_RESET_HARD, MF_BYCOMMAND | MF_ENABLED);

	if (ToolboxNametable) ToolboxNametable = shared_ptr<DlgNametable>(new DlgNametable(nes.getCPU()));
	if (ToolboxPatterntable) ToolboxPatterntable = shared_ptr<DlgPatterntable>(new DlgPatterntable(nes.getCPU()));
	if (ToolboxOAM) ToolboxOAM = shared_ptr<DlgOAM>(new DlgOAM(nes.getCPU()));
	if (ToolboxPalette) ToolboxPalette = shared_ptr<DlgPalette>(new DlgPalette(nes.getCPU()));
	if (ToolboxRAM) ToolboxRAM = shared_ptr<DlgRAM>(new DlgRAM(nes.getCPU()));
	if (ToolboxCPU) ToolboxCPU = shared_ptr<DlgCPU>(new DlgCPU(*this));

	clearWindow();
	emulatorState = EmulatorState::Running;
	return true;
}

void App::closeGame()
{
	if (emulatorState == EmulatorState::Idle) return;
	emulatorState = EmulatorState::Idle;
	if (ToolboxNametable)	{
		ToolboxNametable.reset();
		CheckMenuItem(Menu, DEBUG_WINDOWS_NAMETABLE, MF_BYCOMMAND | MF_UNCHECKED);
	}
	if (ToolboxPatterntable)	{
		ToolboxPatterntable.reset();
		CheckMenuItem(Menu, DEBUG_WINDOWS_PATTERNTABLE, MF_BYCOMMAND | MF_UNCHECKED);
	}
	if (ToolboxOAM)	{
		ToolboxOAM.reset();
		CheckMenuItem(Menu, DEBUG_WINDOWS_OAM, MF_BYCOMMAND | MF_UNCHECKED);
	}
	if (ToolboxPalette)	{
		ToolboxPalette.reset();
		CheckMenuItem(Menu, DEBUG_WINDOWS_PALETTE, MF_BYCOMMAND | MF_UNCHECKED);
	}
	if (ToolboxRAM)	{
		ToolboxRAM.reset();
		CheckMenuItem(Menu, DEBUG_WINDOWS_RAM, MF_BYCOMMAND | MF_UNCHECKED);
	}
	if (ToolboxCPU)	{
		ToolboxCPU.reset();
		CheckMenuItem(Menu, DEBUG_WINDOWS_CPU, MF_BYCOMMAND | MF_UNCHECKED);
	}
	EnableMenuItem(Menu, FILE_CLOSE, MF_BYCOMMAND | MF_GRAYED);

	CheckMenuItem(Menu, EMULATION_PAUSE, MF_BYCOMMAND | MF_UNCHECKED);
	EnableMenuItem(Menu, EMULATION_PAUSE, MF_BYCOMMAND | MF_GRAYED);

	EnableMenuItem(Menu, EMULATION_RESET_SOFT, MF_BYCOMMAND | MF_GRAYED);
	EnableMenuItem(Menu, EMULATION_RESET_HARD, MF_BYCOMMAND | MF_GRAYED);

	closeAudio();
	clearWindow();

	Log->Info("Emulation closed");
}

void App::onQuit(SDL_QuitEvent e)
{
	emulatorState = EmulatorState::Quited;
}
void App::onDropFile(SDL_DropEvent e)
{
	static int previousTimestamp = 0;
	if (e.timestamp == previousTimestamp) return;

	previousTimestamp = e.timestamp;
	Log->Info("File dropped on window.");

	std::string path = e.file;
	SDL_free(e.file);

	std::string extension = getExtension(path);
	std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
	
	if (extension.compare("nes") != 0) {
		Log->Error("Not a .nes file");
		return;
	}

	gamePath = path;
	loadGame(path);
}
void App::onWindow(SDL_WindowEvent e)
{
	if (e.event == SDL_WINDOWEVENT_RESIZED) {
		clearWindow();
		return;
	}
	if (e.event != SDL_WINDOWEVENT_CLOSE) return;

	int ID = e.windowID;
	if (ID == SDL_GetWindowID(mainWindow)) {
		SDL_Event event;
		event.type = SDL_QUIT;
		SDL_PushEvent(&event);
		return;
	}

	if (ToolboxNametable && ID == ToolboxNametable->WindowID) {
		ToolboxNametable.reset();
		CheckMenuItem(Menu, DEBUG_WINDOWS_NAMETABLE, MF_BYCOMMAND | MF_UNCHECKED);
	}
	if (ToolboxPatterntable && ID == ToolboxPatterntable->WindowID) {
		ToolboxPatterntable = NULL;
		CheckMenuItem(Menu, DEBUG_WINDOWS_PATTERNTABLE, MF_BYCOMMAND | MF_UNCHECKED);
	}
	if (ToolboxOAM && ID == ToolboxOAM->WindowID) {
		ToolboxOAM.reset();
		CheckMenuItem(Menu, DEBUG_WINDOWS_OAM, MF_BYCOMMAND | MF_UNCHECKED);
	}
	if (ToolboxPalette && ID == ToolboxPalette->WindowID) {
		ToolboxPalette.reset();
		CheckMenuItem(Menu, DEBUG_WINDOWS_PALETTE, MF_BYCOMMAND | MF_UNCHECKED);
	}
	if (ToolboxRAM && ID == ToolboxRAM->WindowID) {
		ToolboxRAM.reset();
		CheckMenuItem(Menu, DEBUG_WINDOWS_RAM, MF_BYCOMMAND | MF_UNCHECKED);
	}
	if (ToolboxCPU && ID == ToolboxCPU->WindowID) {
		ToolboxCPU.reset();
		CheckMenuItem(Menu, DEBUG_WINDOWS_CPU, MF_BYCOMMAND | MF_UNCHECKED);
	}
}
void App::onKeyDown(SDL_KeyboardEvent e)
{
	if (ToolboxRAM && e.windowID == ToolboxRAM->WindowID) ToolboxRAM->Key(e.keysym.sym);
	if (ToolboxCPU && e.windowID == ToolboxCPU->WindowID) ToolboxCPU->Key(e.keysym.sym);
	if (e.windowID == SDL_GetWindowID(mainWindow)) {
		if (e.keysym.sym == SDLK_ESCAPE) {
			if (emulatorState == EmulatorState::Running || emulatorState == EmulatorState::Paused) closeGame();
			else if (emulatorState == EmulatorState::Idle) {
				SDL_Event event;
				event.type = SDL_QUIT;
				SDL_PushEvent(&event);
			}
		}
		else if (e.keysym.sym == SDLK_SPACE) frameLimit = false;
	}
	updateToolboxes(true);
}
void App::onKeyUp(SDL_KeyboardEvent e)
{
	if (e.windowID == SDL_GetWindowID(mainWindow)) {
		if (e.keysym.sym == SDLK_SPACE) frameLimit = true;
	}
}
void App::onMouseMove(SDL_MouseMotionEvent e)
{
	if (ToolboxRAM && e.windowID == ToolboxRAM->WindowID) ToolboxRAM->Move(e.x, e.y);
	if (ToolboxCPU && e.windowID == ToolboxCPU->WindowID) ToolboxCPU->Move(e.x, e.y);
	updateToolboxes(true);
}
void App::onMouseDown(SDL_MouseButtonEvent e)
{
	if (!mouseUp) return;
	if (ToolboxRAM && e.windowID == ToolboxRAM->WindowID)
	{
		ToolboxRAM->Click(e.x, e.y, e.button & 0xff);
		mouseUp = false;
	}
	if (ToolboxCPU && e.windowID == ToolboxCPU->WindowID)
	{
		ToolboxCPU->Click(e.x, e.y, e.button & 0xff, false);
		mouseUp = false;
	}
	updateToolboxes(true);
}
void App::onMouseUp(SDL_MouseButtonEvent e)
{
	mouseUp = true;
	if (ToolboxCPU && e.windowID == ToolboxCPU->WindowID)
		ToolboxCPU->Click(e.x, e.y, e.button & 0xff, true);
	updateToolboxes(true);
}
void App::onSystem(SDL_SysWMEvent e)
{
	if (e.msg->msg.win.msg != WM_COMMAND) return;
	switch (LOWORD(e.msg->msg.win.wParam))
	{
	case FILE_OPEN:
		{
		char fileName[2048] = { 0 };
		OPENFILENAME ofn;
		memset(&ofn, 0, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = mainWindowHwnd;
		ofn.hInstance = GetModuleHandle(NULL);
		ofn.lpstrFile = (char*)fileName;
		ofn.nMaxFile = sizeof(fileName)-1;
		ofn.lpstrFilter = "All supported files\0*.nes\0""NES files\0*.nes\0";
		ofn.nFilterIndex = 0;
		ofn.lpstrInitialDir = "./rom/";
		ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
		if (!GetOpenFileName(&ofn))
		{
			Log->Debug("GetOpenFileName: No file selected.");
			break;
		}
		gamePath = fileName;
		loadGame(fileName);
		}
		break;

	case FILE_CLOSE:
		closeGame();
		break;

	case FILE_EXIT:
		closeGame();
		emulatorState = EmulatorState::Quited;
		break;

	case EMULATION_PAUSE:
		if (CheckMenuItem(Menu, EMULATION_PAUSE, MF_BYCOMMAND) == MF_UNCHECKED)
		{
			if (emulatorState == EmulatorState::Running)
			{
				CheckMenuItem(Menu, EMULATION_PAUSE, MF_BYCOMMAND | MF_CHECKED);
				emulatorState = EmulatorState::Paused;
				SDL_PauseAudio(1);
				Log->Info("Emulation paused");
			}
		}
		else
		{
			if (emulatorState == EmulatorState::Paused)
			{
				CheckMenuItem(Menu, EMULATION_PAUSE, MF_BYCOMMAND | MF_UNCHECKED);
				emulatorState = EmulatorState::Running;
				SDL_PauseAudio(0);
				Log->Info("Emulation resumed");
			}
		}
		break;

	case EMULATION_RESET_SOFT:
		nes.reset();
		break;

	case EMULATION_RESET_HARD:
		loadGame(gamePath);
		break;

	case OPTIONS_SOUND_ENABLED:
		if (CheckMenuItem(Menu, OPTIONS_SOUND_ENABLED, MF_BYCOMMAND) == MF_UNCHECKED)
		{
			CheckMenuItem(Menu, OPTIONS_SOUND_ENABLED, MF_BYCOMMAND | MF_CHECKED);
			SDL_PauseAudio(0);
			Log->Info("Sound: enabled");
		}
		else
		{
			CheckMenuItem(Menu, OPTIONS_SOUND_ENABLED, MF_BYCOMMAND | MF_UNCHECKED);
			SDL_PauseAudio(1);
			Log->Info("Sound: disabled");
		}
		break;


	case DEBUG_WINDOWS_OAM:
		if (CheckMenuItem(Menu, DEBUG_WINDOWS_OAM, MF_BYCOMMAND) == MF_UNCHECKED)
		{
			CheckMenuItem(Menu, DEBUG_WINDOWS_OAM, MF_BYCOMMAND | MF_CHECKED);
			ToolboxOAM = shared_ptr<DlgOAM>(new DlgOAM(nes.getCPU()));
		}
		else
		{
			CheckMenuItem(Menu, DEBUG_WINDOWS_OAM, MF_BYCOMMAND | MF_UNCHECKED);
			ToolboxOAM.reset();
		}
		break;

	case DEBUG_WINDOWS_PALETTE:
		if (CheckMenuItem(Menu, DEBUG_WINDOWS_PALETTE, MF_BYCOMMAND) == MF_UNCHECKED)
		{
			CheckMenuItem(Menu, DEBUG_WINDOWS_PALETTE, MF_BYCOMMAND | MF_CHECKED);
			ToolboxPalette = shared_ptr<DlgPalette>(new DlgPalette(nes.getCPU()));
		}
		else
		{
			CheckMenuItem(Menu, DEBUG_WINDOWS_PALETTE, MF_BYCOMMAND | MF_UNCHECKED);
			ToolboxPalette.reset();
		}
		break;

	case DEBUG_WINDOWS_NAMETABLE:
		if (CheckMenuItem(Menu, DEBUG_WINDOWS_NAMETABLE, MF_BYCOMMAND) == MF_UNCHECKED)
		{
			CheckMenuItem(Menu, DEBUG_WINDOWS_NAMETABLE, MF_BYCOMMAND | MF_CHECKED);
			ToolboxNametable = shared_ptr<DlgNametable>(new DlgNametable(nes.getCPU()));
		}
		else
		{
			CheckMenuItem(Menu, DEBUG_WINDOWS_NAMETABLE, MF_BYCOMMAND | MF_UNCHECKED);
			ToolboxNametable.reset();
		}
		break;

	case DEBUG_WINDOWS_PATTERNTABLE:
		if (CheckMenuItem(Menu, DEBUG_WINDOWS_PATTERNTABLE, MF_BYCOMMAND) == MF_UNCHECKED)
		{
			CheckMenuItem(Menu, DEBUG_WINDOWS_PATTERNTABLE, MF_BYCOMMAND | MF_CHECKED);
			ToolboxPatterntable = shared_ptr<DlgPatterntable>(new DlgPatterntable(nes.getCPU()));
		}
		else
		{
			CheckMenuItem(Menu, DEBUG_WINDOWS_PATTERNTABLE, MF_BYCOMMAND | MF_UNCHECKED);
			ToolboxPatterntable.reset();
		}
		break;

	case DEBUG_WINDOWS_RAM:
		if (CheckMenuItem(Menu, DEBUG_WINDOWS_RAM, MF_BYCOMMAND) == MF_UNCHECKED)
		{
			CheckMenuItem(Menu, DEBUG_WINDOWS_RAM, MF_BYCOMMAND | MF_CHECKED);
			ToolboxRAM = shared_ptr<DlgRAM>(new DlgRAM(nes.getCPU()));
		}
		else
		{
			CheckMenuItem(Menu, DEBUG_WINDOWS_RAM, MF_BYCOMMAND | MF_UNCHECKED);
			ToolboxRAM.reset();
		}
		break;

	case DEBUG_WINDOWS_CPU:
		if (CheckMenuItem(Menu, DEBUG_WINDOWS_CPU, MF_BYCOMMAND) == MF_UNCHECKED)
		{
			CheckMenuItem(Menu, DEBUG_WINDOWS_CPU, MF_BYCOMMAND | MF_CHECKED);
			ToolboxCPU = shared_ptr<DlgCPU>(new DlgCPU(*this));
		}
		else
		{
			CheckMenuItem(Menu, DEBUG_WINDOWS_CPU, MF_BYCOMMAND | MF_UNCHECKED);
			ToolboxCPU.reset();
		}
		break;

	case HELP_ABOUT:
		DlgAbout DialogAbout(mainWindowHwnd);
		break;
	}
}

void App::updateToolboxes(bool withCPU)
{
	if (newFrame || emulatorState == EmulatorState::Paused) {
		newFrame = false;
		if (ToolboxNametable) ToolboxNametable->Update();
		if (ToolboxOAM) ToolboxOAM->Update();
		if (ToolboxPalette) ToolboxPalette->Update();
		if (ToolboxPatterntable) ToolboxPatterntable->Update();
		if (ToolboxRAM) ToolboxRAM->Update();
		if (ToolboxCPU && withCPU) ToolboxCPU->Update();
	}
}

uint8_t App::getInput()
{
	uint8_t buttonState = 0;
	if (XboxPresent)
	{
		XINPUT_STATE xstate;
		XInputGetState(0, &xstate);
		if (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_B) buttonState |= 1 << 7;
		if (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_A) buttonState |= 1 << 6;
		if (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) buttonState |= 1 << 5;
		if (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_START) buttonState |= 1 << 4;
		if (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) buttonState |= 1 << 3;
		if (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) buttonState |= 1 << 2;
		if (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) buttonState |= 1 << 1;
		if (xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) buttonState |= 1 << 0;
	}
	Uint8 *keys = (Uint8*)SDL_GetKeyboardState(NULL);

	//A, B, Select, Start, Up, Down, Left, Right.
	if (keys[SDL_SCANCODE_X]) buttonState |= 1 << 7;
	if (keys[SDL_SCANCODE_Z]) buttonState |= 1 << 6;
	if (keys[SDL_SCANCODE_A]) buttonState |= 1 << 5;
	if (keys[SDL_SCANCODE_S]) buttonState |= 1 << 4;
	if (keys[SDL_SCANCODE_UP]) buttonState |= 1 << 3;
	if (keys[SDL_SCANCODE_DOWN]) buttonState |= 1 << 2;
	if (keys[SDL_SCANCODE_LEFT]) buttonState |= 1 << 1;
	if (keys[SDL_SCANCODE_RIGHT]) buttonState |= 1 << 0;
	return buttonState;
}

bool App::initialize()
{
	Log->Verbose();
	Log->setProgramName("AnotherNES");
	Log->Info("AnotherNES version %d.%d", MAJOR_VERSION, MINOR_VERSION);

	if (!initializeSDL()) return false;
	initializeXInput();

	// Texture filtering
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

	mainWindow = createMainWindow();
	if (!mainWindow) return false;

	renderer = createRenderer(mainWindow);
	if (!renderer) return false;

	canvas = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 256, 240);
	if (!canvas) {
		Log->Fatal("Cannot create canvas texture!");
		return false;
	}

	TTF_Init();

	emulatorState = EmulatorState::Idle;
	return true;
}

void App::cleanup()
{
	closeAudio();
	closeGame();
	SDL_DestroyTexture(canvas);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(mainWindow);
	SDL_Quit();
}

void App::loop()
{
	Uint32 oldticks = 0;
	SDL_Event event;
	while (emulatorState != EmulatorState::Quited)
	{
		int pendingEvents = false;
		if (emulatorState != EmulatorState::Running) SDL_WaitEvent(&event);
		else pendingEvents = SDL_PollEvent(&event);

		if (event.type == SDL_QUIT) onQuit(event.quit);
		else if (event.type == SDL_DROPFILE) onDropFile(event.drop);
		else if (event.type == SDL_WINDOWEVENT) onWindow(event.window);
		else if (event.type == SDL_KEYDOWN) onKeyDown(event.key);
		else if (event.type == SDL_KEYUP) onKeyUp(event.key);
		else if (event.type == SDL_MOUSEMOTION) onMouseMove(event.motion);
		else if (event.type == SDL_MOUSEBUTTONDOWN) onMouseDown(event.button);
		else if (event.type == SDL_MOUSEBUTTONUP) onMouseUp(event.button);
		else if (event.type == SDL_SYSWMEVENT) onSystem(event.syswm);

		if (pendingEvents) continue; 

		if (emulatorState == EmulatorState::Running)
		{
			nes.setInput(getInput());
			if (nes.emulateFrame())	
			{
				nes.render(canvas);
				SDL_RenderCopy(renderer, canvas, NULL, NULL);
				SDL_RenderPresent(renderer);

				updateToolboxes(false);
			} 
			else 
			{
				closeGame();
				continue;
			}

			if (!frameLimit)	continue;

			Uint32 ticksPerFrame = 16; // NTSC, 1/60 == 16.6666ms
			if (nes.getRegion() == Region::PAL) ticksPerFrame = 20; // Pal, 1/50 == 20ms

			Uint32 ticks = SDL_GetTicks();
			while (ticks - oldticks < ticksPerFrame) {
				SDL_Delay(1);
				ticks = SDL_GetTicks();
			}	
			oldticks = ticks;
			newFrame = true;
		}
	}
}

void App::run()
{
	if (!initialize()) return;
	loop();
	cleanup();
}