#pragma once
#include <windows.h>
#include <SDL.h>
#include <memory>
#include "NES.h"

#include "dialog/DlgOAM.h"
#include "dialog/DlgPalette.h"
#include "dialog/DlgNametable.h"
#include "dialog/DlgPatterntable.h"
#include "dialog/DlgAbout.h"
#include "dialog/DlgRAM.h"
#include "dialog/DlgCPU.h"

enum class EmulatorState
{
	Idle,
	Running,
	Paused,
	Quited
};


class App
{
	// Main window handles
	EmulatorState emulatorState;
	SDL_Window *mainWindow;
	SDL_Renderer* renderer;
	SDL_Texture* canvas;
	HWND mainWindowHwnd;
	HMENU Menu;

	// Dialog boxes
	shared_ptr<DlgOAM> ToolboxOAM;
	shared_ptr<DlgPalette> ToolboxPalette;
	shared_ptr<DlgNametable> ToolboxNametable;
	shared_ptr<DlgPatterntable> ToolboxPatterntable;
	shared_ptr<DlgRAM> ToolboxRAM;
	shared_ptr<DlgCPU> ToolboxCPU;

	bool mouseUp;
	bool frameLimit;
	NES nes;
	std::string gamePath;

	// Initialization methods
	bool initializeSDL();
	bool initializeAudio(NES* nes);
	void initializeXInput();
	SDL_Window* createMainWindow();
	SDL_Renderer* createRenderer(SDL_Window* window);
	void clearWindow();
	void closeAudio();
	bool loadGame(std::string path);
	void closeGame();

	// Events
	void onQuit(SDL_QuitEvent e);
	void onDropFile(SDL_DropEvent e);
	void onWindow(SDL_WindowEvent e);
	void onKeyDown(SDL_KeyboardEvent e);
	void onKeyUp(SDL_KeyboardEvent e);
	void onMouseMove(SDL_MouseMotionEvent e);
	void onMouseDown(SDL_MouseButtonEvent e);
	void onMouseUp(SDL_MouseButtonEvent e);
	void onSystem(SDL_SysWMEvent e);

	void updateToolboxes();
	uint8_t getInput();

	bool initialize();
	void cleanup();
	void loop();
public:
	App();
	void run();
};