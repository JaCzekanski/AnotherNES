#pragma once
#include "../headers.h"
#include "../cpu.h"
#include <sdl.h>

class DlgPatterntable
{
private:
	CPU* cpu;

	SDL_Window *ToolboxPatterntable;
public:
	uint32_t WindowID;

	DlgPatterntable(CPU* cpu);
	~DlgPatterntable(void);
	void Update();
	void Clear();
};
