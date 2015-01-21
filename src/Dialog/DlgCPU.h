#pragma once
#include "../headers.h"
#include "../cpu.h"
#include "../GUI/Canvas.h"
#include <sdl.h>

class App;
class DlgCPU
{
private:
	const int WIDTH = 64;
	const int HEIGHT = 64;
	//unsigned char screen[HEIGHT][WIDTH];
	unsigned char screen[64][64];
	std::vector<SDL_Surface*> Font;

	int selectedX = -1, selectedY = -1;

	shared_ptr<Canvas> canvas;

	SDL_Window *ToolboxCPU;
	SDL_Renderer *renderer;
	SDL_Surface *tos;

	int registersId, disassemblyId, controlId, stackId;

	void disassembler();
public:
	App &app;
	CPU* cpu;
	uint32_t WindowID;

	DlgCPU(App & _app);
	~DlgCPU(void);
	void Update();
	void Clear();
	void Click(int x, int y, int button, bool released);
	void Move(int x, int y);
	void Key(SDL_Keycode k);

	void onRun();
	void onPause();
	void onSingleStep();
};
