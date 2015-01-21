#pragma once
#include "../headers.h"
#include "../cpu.h"
#include <sdl.h>

class DlgCPU
{
private:
	const int WIDTH = 64;
	const int HEIGHT = 64;
	//unsigned char screen[HEIGHT][WIDTH];
	unsigned char screen[64][64];
	std::vector<SDL_Surface*> Font;

	int selectedX = -1, selectedY = -1;

	SDL_Window *ToolboxCPU;
	SDL_Surface *tos;

	void disassembly();
	void DrawRect(int x, int y, int w, int h, bool join);
	void mprintf(int x, int y, int w, int h, char const * format, ...);
public:
	CPU* cpu;
	uint32_t WindowID;

	DlgCPU(CPU* cpu);
	~DlgCPU(void);
	void Update();
	void Clear();
	void Click(int x, int y, int button, bool pressed); // true - pressed, false - released
	void Move(int x, int y);
	void Key(SDL_Keycode k);
};
