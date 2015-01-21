#include "NES.h"
#include "Utils.h"

#include"Mapper\Mapper0.h"
#include"Mapper\Mapper1.h"
#include"Mapper\Mapper2.h"
#include"Mapper\Mapper3.h"
#include"Mapper\Mapper4.h"
#include"Mapper\Mapper7.h"
#include"Mapper\Mapper65.h"
#include"Mapper\Mapper71.h"

NES::NES()
{
}


NES::~NES()
{
}

// Loads rom with path as argument
bool NES::loadGame(const char* path)
{
	iNES rom;
	if (rom.Load((const char*)path))
	{
		Log->Error("Cannot load %s", getFilename(path).c_str());
		return false;
	}
	Log->Success("%s opened", getFilename(path).c_str());

	cpu = new CPU_interpreter();

	cpu->memory.ppu = &cpu->ppu;
	cpu->memory.apu = &cpu->apu;
	cpu->ppu.Mirroring = rom.getMirroring();

	if (rom.getMapper() == 0)  cpu->memory.mapper = new Mapper0(cpu->ppu);
	else if (rom.getMapper() == 1)  cpu->memory.mapper = new Mapper1(cpu->ppu);
	else if (rom.getMapper() == 2)  cpu->memory.mapper = new Mapper2(cpu->ppu);
	else if (rom.getMapper() == 3)  cpu->memory.mapper = new Mapper3(cpu->ppu);
	else if (rom.getMapper() == 4)  cpu->memory.mapper = new Mapper4(cpu->ppu);
	else if (rom.getMapper() == 7)  cpu->memory.mapper = new Mapper7(cpu->ppu);
	else if (rom.getMapper() == 65) cpu->memory.mapper = new Mapper65(cpu->ppu);
	else if (rom.getMapper() == 71) cpu->memory.mapper = new Mapper71(cpu->ppu);
	else {
		Log->Error("Unsupported mapper");
		return false;
	}

	if (rom.PRG_ROM_pages == 0) {
		Log->Error("Rom has no PRG ROM, nothing to execute!");
		return false;
	}

	cpu->memory.mapper->setPrg(rom.PRG_ROM);
	Log->Success("%dB PRG_ROM copied", rom.PRG_ROM_pages * 16384);

	if (rom.CHR_ROM_pages > 0) {
		cpu->memory.mapper->setChr(rom.CHR_ROM);
		memcpy(cpu->ppu.memory, &rom.CHR_ROM[0], 8192);

		Log->Success("%dB CHR_ROM copied", rom.CHR_ROM_pages * 8192);
	}

	cpu->Power();

	SDL_PauseAudio(0);
	return 0;
}


void NES::reset()
{
	cpu->Reset();
}

void NES::setInput(uint8_t buttons)
{
	cpu->memory.setInput(buttons);
}

bool NES::emulateFrame()
{
	int cycles = 0, apu_cycles = 0;
	bool framerefresh = false;
	while (!framerefresh)
	{
		for (int i = cycles * 3; i>0; i--)
		{
			if (i % 3 == 0) cycles--;
			uint8_t ppuresult = cpu->ppu.Step();
			if (ppuresult) // NMI requested
			{
				framerefresh = true;
				if (ppuresult == 2)
				{
					cpu->NMI();
					cycles += 7;
					i += 7 * 3;
				}
			}
			if (cpu->ppu.cycles == 260) {
				Mapper4 *MMC3 = dynamic_cast<Mapper4*>(cpu->memory.mapper);
				if (MMC3 && MMC3->MMC3_irqEnabled && MMC3->MMC3_irqCounter == cpu->ppu.scanline+1)
				{
					cpu->IRQ();
				}
			}
		}
		cycles += cpu->Step();
		apu_cycles += cycles;
		if (cpu->isJammed())
		{
			Log->Error("CPU Jammed.");
			framerefresh = true;
			return false;
		}
		if (apu_cycles >= 7457)
		{
			cpu->apu.frameStep();
			apu_cycles = 0;
		}
		Mapper65 *mapper65 = dynamic_cast<Mapper65*>(cpu->memory.mapper);
		if (mapper65 && mapper65->irqEnabled) {
			if (mapper65->irqCounter <= cycles) cpu->IRQ();
			mapper65->irqCounter -= cycles;
		}
		cpu->apu.activeTimer++;
	}
	return true;
}


bool NES::singleStep()
{
	int cycles = 1, apu_cycles = 0;
	for (int i = cycles * 3; i>0; i--)
	{
		if (i % 3 == 0) cycles--;
		uint8_t ppuresult = cpu->ppu.Step();
		if (ppuresult) // NMI requested
		{
			if (ppuresult == 2)
			{
				cpu->NMI();
				cycles += 7;
				i += 7 * 3;
			}
		}
		if (cpu->ppu.cycles == 260) {
			Mapper4 *MMC3 = dynamic_cast<Mapper4*>(cpu->memory.mapper);
			if (MMC3 && MMC3->MMC3_irqEnabled && MMC3->MMC3_irqCounter == cpu->ppu.scanline + 1)
			{
				cpu->IRQ();
			}
		}
	}
	cycles += cpu->Step();
	apu_cycles += cycles;
	if (cpu->isJammed())
	{
		Log->Error("CPU Jammed.");
		return false;
	}
	if (apu_cycles >= 7457)
	{
		cpu->apu.frameStep();
		apu_cycles = 0;
	}
	Mapper65 *mapper65 = dynamic_cast<Mapper65*>(cpu->memory.mapper);
	if (mapper65 && mapper65->irqEnabled) {
		if (mapper65->irqCounter <= cycles) cpu->IRQ();
		mapper65->irqCounter -= cycles;
	}
	cpu->apu.activeTimer++;
	return true;
}

void NES::render(SDL_Texture *canvas)
{
	cpu->ppu.Render(canvas);
}