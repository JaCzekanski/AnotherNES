#include "DlgCPU.h"
#include <algorithm>
#include "../CPU_interpreter.h"
#include "../GUI/Font.h"
#include "../GUI/Button.h"
#include "../Utils.h"
#include "../App.h"

DlgCPU::DlgCPU(App & _app) : app(_app), cpu(_app.nes.getCPU())
{
	ToolboxCPU = SDL_CreateWindow("AnotherNES - CPU", 5, 5, WIDTH*(8 + 1), HEIGHT*(8 + 1), SDL_WINDOW_SHOWN);
	if (ToolboxCPU == NULL)
	{
		Log->Fatal("Cannot create toolbox cpu");
		return;
	}
	renderer = SDL_CreateRenderer(ToolboxCPU, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == NULL)
	{
		Log->Fatal("Cannot create renderer");
		return;
	}
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	this->WindowID = SDL_GetWindowID(ToolboxCPU);

	FONT.setRenderer(renderer);
	canvas = shared_ptr<Canvas>( new Canvas(renderer));
	Window *w = new Window("Registers", 0, 0, 150, 150);
	registersId = w->getId();
	canvas->addWindow(w);

	w = new Window("Stack", 0, 150, 150, 200);
	stackId = w->getId();
	canvas->addWindow(w);

	w = new Window("Control", 0, 350, 150, 150);

	Button *b = new Button("Run", 5, 5 + 20 * 0);
	b->setOnPress(std::bind(&DlgCPU::onRun, this));
	w->addWidget(b);

	b = new Button("Pause", 5, 5 + 20 * 1);
	b->setOnPress(std::bind(&DlgCPU::onPause, this));
	w->addWidget(b);

	b = new Button("Single step", 5, 5 + 20 * 2);
	b->setOnPress(std::bind(&DlgCPU::onSingleStep, this));
	w->addWidget(b);

	canvas->addWindow(w);

	w = new Window("Disassembly", 150, 0, 400, 400);
	disassemblyId = w->getId();
	canvas->addWindow(w);


	Log->Success("Toolbox CPU created");
}

DlgCPU::~DlgCPU(void)
{
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(ToolboxCPU);
	Log->Success("Toolbox CPU destroyed.");
}

#define DISASM(x,y) (sprintf(buffer,x,y))

void DlgCPU::disassembler()
{
	char buffer[512];
	uint16_t address = cpu->PC;

	shared_ptr<Window> disassembly = canvas->findWindow(disassemblyId);
	disassembly->clearWidgets();

	for (int i = 0; i < 30; i++)
	{
		memset(buffer, 0, 512);
		OPCODE op = OpcodeTableOptimized[cpu->memory[address]];

		uint8_t low, high; // Temporary variables for address calculations
		uint8_t arg1 = cpu->memory[address + 1];
		uint8_t arg2 = cpu->memory[address + 2];
		uint16_t addrlo, addrhi;
		int opsize = 0;
		switch (op.address)
		{
		case Implicit: // No args
			opsize = 1;
			DISASM("%c", 0);
			break;
		case Accumulator:
			opsize = 1;
			DISASM("%c", 'A');
			break;
		case Immediate: // imm8
			opsize = 2;
			DISASM("#$%.2X", address + 1);
			break;
		case Zero_page: // $0000+arg1
			opsize = 2;
			DISASM("$%.2X", arg1);
			break;
		case Zero_page_x: // $0000+arg1+X
			opsize = 2;
			DISASM("$%.2X,X", (arg1 + cpu->X) & 0xff);
			break;
		case Zero_page_y: // $0000+arg1+Y
			opsize = 2;
			DISASM("$%.2X,Y", (arg1 + cpu->Y) & 0xff);
			break;
		case Relative: // -128 to +127
			opsize = 2;
			DISASM("$%.4X", (address + opsize) + (signed char)(arg1));
			break;
		case Absolute: // 16bit address
			opsize = 3;
			DISASM("$%.4X", (arg2 << 8) | arg1);
			break;
		case Absolute_x: // 16bit address + X
		{
			uint16_t virtaddr = (((arg2 << 8) | arg1) + cpu->X) & 0xffff;
			opsize = 3;
			DISASM("$%.4X,X", virtaddr - cpu->X);
			break;
		}
		case Absolute_y: // 16bit address + Y, NOT Working 037h - LDA failure to wrap properly from ffffh to 0000h
		{
			uint16_t virtaddr = (((arg2 << 8) | arg1) + cpu->Y) & 0xffff;
			opsize = 3;
			DISASM("$%.4X,Y", virtaddr - cpu->Y);
			break;
		}
		case Indirect:
			opsize = 3;
			DISASM("($%.4X)", (arg2 << 8) | arg1);
			break;
		case Indexed_indirect:
			opsize = 2;
			DISASM("($%.2X,X)", arg1);
			break;
		case Indirect_indexed:
			opsize = 2;
			addrlo = arg1;
			addrhi = (arg1 + 1) & 0xff;

			low = cpu->memory[addrlo];
			high = cpu->memory[addrhi];
			
			DISASM("($%.2X),Y", arg1);
			break;
		default:
			Log->Error("CPU_interpreter::Step(): Unknown addressing mode!");
			break;
		}

		char hexvals[32];
		for (int j = 0; j<opsize; j++)
		{
			sprintf(hexvals+(j*3), "%.2X ", cpu->memory[address+j] );
		}
		for (int j = 0; j<(3-opsize); j++)
		{
			sprintf(hexvals+((j+opsize)*3), "   "  );
		}
		int j;
		int len = strlen(buffer);
		for ( j = len; j < 10; j++)
		{
			buffer[j] = ' ';
		}
		buffer[++j] =  0;
	
		address += opsize;
		disassembly->addWidget(new Widget(string_format("$%04x %s %s %s", address, hexvals, op.mnemnic, buffer), 15, 5 + i * 12));
	}

}

void DlgCPU::Update()
{
	if (cpu == NULL) return;
	SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);
	SDL_RenderClear(renderer);

	if (app.emulatorState == EmulatorState::Paused) {
		shared_ptr<Window> registers = canvas->findWindow(registersId);
		registers->clearWidgets();

		int y = 0;
		registers->addWidget(new Widget(string_format("A:  $%02x", cpu->A), 5, 5 + y++ * 15));
		registers->addWidget(new Widget(string_format("X:  $%02x", cpu->X), 5, 5 + y++ * 15));
		registers->addWidget(new Widget(string_format("Y:  $%02x", cpu->Y), 5, 5 + y++ * 15));
		registers->addWidget(new Widget(string_format("SP: $%02x", cpu->SP), 5, 5 + y++ * 15));
		registers->addWidget(new Widget(string_format("PC: $%04x", cpu->PC), 5, 5 + y++ * 15));
		registers->addWidget(new Widget(string_format("P:  $%02x", cpu->P), 5, 5 + y++ * 15));
		registers->addWidget(new Widget(string_format("%c%c %c%c%c%c%c",
			(cpu->P&NEGATIVE_FLAG ? 'N' : ' '),
			(cpu->P&OVERFLOW_FLAG ? 'V' : ' '),
			(cpu->P&BREAK_FLAG ? 'B' : ' '),
			(cpu->P&DECIMAL_FLAG ? 'D' : ' '),
			(cpu->P&INTERRUPT_FLAG ? 'I' : ' '),
			(cpu->P&ZERO_FLAG ? 'Z' : ' '),
			(cpu->P&CARRY_FLAG ? 'C' : ' ')), 5, 5 + y++ * 15));


		// Stack
		shared_ptr<Window> stack = canvas->findWindow(stackId);
		stack->clearWidgets();

		for (int i = 0; i < 10; i++)
		{
			stack->addWidget(new Widget(string_format("$%04x: $%02x", 0x100 + cpu->SP + i, cpu->memory[0x100+cpu->SP + i]), 5, 5 + i * 15));
		}

		disassembler();
	}

	canvas->Draw();

	SDL_RenderPresent(renderer);
}

void DlgCPU::Clear()
{
	SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);
	SDL_RenderClear(renderer);
	SDL_RenderPresent(renderer);
}


void DlgCPU::Click(int x, int y, int button, bool released)
{
	canvas->Click(x, y, button, released);
}


void DlgCPU::Move(int x, int y)
{
	canvas->Move(x, y);
}

void DlgCPU::Key(SDL_Keycode k)
{
	canvas->Key(k);
}

void DlgCPU::onRun()
{
	SDL_PauseAudio(0);
	app.emulatorState = EmulatorState::Running;
}

void DlgCPU::onPause()
{
	app.emulatorState = EmulatorState::Paused;
}

void DlgCPU::onSingleStep()
{
	app.emulatorState = EmulatorState::Paused;
	SDL_PauseAudio(1);
	app.nes.singleStep();
}