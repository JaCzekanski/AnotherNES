#pragma once
#include "../headers.h"
#include "../cpu.h"
#include <sdl.h>

class DlgNametable
{
private:
	CPU* cpu;

	SDL_Window *ToolboxNametable;
	void __DrawNametable(SDL_Surface* s, uint8_t nametable);
public:
	uint32_t WindowID;

	DlgNametable( CPU* cpu );
	~DlgNametable(void);
	void Update();
	void Clear();
};
