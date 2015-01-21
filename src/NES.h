#pragma once
#include "iNES.h"
#include "CPU.h"
#include "CPU_interpreter.h"
#include "APU.h"


class NES
{
	CPU* cpu;
	iNES rom;
	Region region;

public:
	NES();
	~NES();
	Region getRegion() { return region; }

	bool loadGame(const char* path);
	void reset();
	void setInput(uint8_t buttons);
	bool emulateFrame();
	void render(SDL_Texture *canvas);
	CPU* getCPU() const { return cpu; };
};

