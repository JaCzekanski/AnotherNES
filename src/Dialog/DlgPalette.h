#pragma once
#include "../headers.h"
#include "../cpu.h"
#include <sdl.h>

class DlgPalette
{
private:
	CPU* cpu;


	SDL_Window *ToolboxPalette;
	SDL_Surface *tos;
public:
	DlgPalette( CPU* cpu );
	~DlgPalette(void);
	void Update();

};
