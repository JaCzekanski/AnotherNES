#pragma once
#include "../headers.h"
#include "../cpu.h"
#include <sdl.h>

struct byte_info
{
	int value;

	int brightness = 0xaa;
	int blinkTime = 0;
};

class DlgRAM
{
private:
	CPU* cpu;

	unsigned char Font[256][8];

	int selectedX = -1, selectedY = -1;
	bool editStarted = false;
	int editValue = 0;

	SDL_Window *ToolboxRAM;
	SDL_Surface *tos;


	vector <byte_info> ramState;
	vector <byte_info> prevRamState;
public:
	uint32_t WindowID;

	DlgRAM(CPU* cpu);
	~DlgRAM(void);
	void Update();
	void Clear();
	void Click(int x, int y, int button);
	void Move(int x, int y);
	void Key(SDL_Keycode k);
};
