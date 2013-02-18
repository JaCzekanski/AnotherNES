#pragma once
#include "../headers.h"
#include "../cpu.h"
#include <sdl.h>

class DlgOAM
{
private:
	CPU* cpu;

	bool SpriteSize;
	SDL_Window *ToolboxOAM;
	void __DrawOAM(SDL_Surface* s, int i);
public:
	DlgOAM( CPU* cpu );
	~DlgOAM(void);
	void Update();

};
