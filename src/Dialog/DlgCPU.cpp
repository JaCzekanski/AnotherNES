#include "DlgCPU.h"
#include <algorithm>
#include "../CPU_interpreter.h"

DlgCPU::DlgCPU(CPU* cpu)
{
	FILE* font = fopen("D:\\Kuba\\dev\\AnotherNES\\font.bin", "rb");
	if (!font)
	{
		Log->Fatal("Cannot read font file.");
		return;
	}
	fseek(font, 256 * 8, SEEK_SET); // Second font type (big and small letters)
	Font.resize(256);
	for (int n = 0; n<128; n++)
	{
		SDL_Surface *font_char = SDL_CreateRGBSurface(0, 8, 8, 32, 0, 0, 0, 0xff000000);
		SDL_SetSurfaceBlendMode(font_char, SDL_BLENDMODE_ADD);
		SDL_LockSurface(font_char);

		for (int i = 0; i<8; i++)
		{
			unsigned char c = fgetc(font);

			for (int xx = 0; xx < 8; xx++)
			{
				int bit = (c & (0x80 >> xx)) ? 0xff : 0x00;
				*((unsigned char*)font_char->pixels + (i * 8 + xx) * 4 + 0) = bit;
				*((unsigned char*)font_char->pixels + (i * 8 + xx) * 4 + 1) = bit;
				*((unsigned char*)font_char->pixels + (i * 8 + xx) * 4 + 2) = bit;
				*((unsigned char*)font_char->pixels + (i * 8 + xx) * 4 + 3) = 0xff;
			}

		}
		SDL_UnlockSurface(font_char);
		int pos = n;

		Font[n + 128] = font_char;  // Original set

		if (n == 0) pos = '@';
		else if (n >= 1 && n <= 26)	pos = n - 1 + 'a';
		else if (n >= 32 && n <= 63) pos = n - 32 + ' '; // Digits
		else if (n >= 65 && pos <= 90) pos = n - 65 + 'A';
		else if (n == 27) pos = '[';
		else if (n == 29) pos = ']';
		else if (n == 93) { pos = '|'; Font[2] = font_char; }
		else if (n == 100) pos = '_';
		else if (n == 107) { pos = '}'; Font[7] = font_char; }
		else if (n == 115) { pos = '{'; Font[10] = font_char; }

		// Frames
		else if (n == 64) pos = 1;   // -
		//else if (n == 93) pos = 2;   // |
		else if (n == 112) pos = 3;  // |`
		else if (n == 110) pos = 4;  // `|
		else if (n == 109) pos = 5;  // |_
		else if (n == 125) pos = 6;  // _|
		//else if (n == 107) pos = 7;  // |-
		else if (n == 113) pos = 8;  // _|_
		else if (n == 114) pos = 9;  // `|`
		//else if (n == 115) pos = 10;  // -|
		else if (n == 91) pos = 11;  // +
		else if (n == 47)
		{
			// / and \ 

		}
		else if (n == 39)
		{
			// `
		}
		else continue;
		Font[pos] = font_char;
	}

	fclose(font);


	this->cpu = cpu;
	ToolboxCPU = SDL_CreateWindow("AnotherNES - CPU", 5, 5, WIDTH*(8 + 1), HEIGHT*(8 + 1), SDL_WINDOW_SHOWN);
	if (ToolboxCPU == NULL)
	{
		Log->Fatal("Cannot create toolbox cpu");
		return;
	}
	this->Clear();
	DrawRect(0, 0, WIDTH, HEIGHT, true);
	this->WindowID = SDL_GetWindowID(ToolboxCPU);
	Log->Success("Toolbox CPU created");
}

DlgCPU::~DlgCPU(void)
{
	for (int i = 0; i < Font.size(); i++)
	{
		if (Font[i] != NULL)
		{
			// Get rid of copies
			for (int j = i + 1; j < Font.size(); j++)
			{
				if (Font[j] == Font[i]) Font[j] = NULL;
			}
			SDL_FreeSurface(Font[i]);
		}
	}
	SDL_DestroyWindow(ToolboxCPU);
	Log->Success("Toolbox CPU destroyed.");
}

#define DISASM(x,y) (sprintf(buffer,x,y))

void DlgCPU::disassembly()
{
	char buffer[512];
	uint16_t address = cpu->PC;

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
		mprintf(15, 1 + i, 30, 5, "$%04x %s %s %s", address, hexvals, op.mnemnic, buffer);
	}

}

void DlgCPU::DrawRect(int x, int y, int w, int h, bool join)
{
	if (x> WIDTH || y > HEIGHT || x < 0 || y < 0) return;

	for (int yy = y; yy < y + h; yy++)
	{
		for (int xx = x; xx < x + w; xx++)
		{
			if (xx > WIDTH || yy > HEIGHT || xx<0 || yy <0) continue;
			bool left = false, right = false, bottom = false, top = false;

			bool onLeft = xx == x;
			bool onRight = xx == (x + w - 1);
			bool onTop = yy == y;
			bool onBottom = yy == (y + h - 1);

			int c;

			if (yy == y) top = true;
			if (yy == y + h - 1) bottom = true;
			if (xx == x) left = true;
			if (xx == x + w - 1) right = true;

			if (join)
			{
				c = screen[yy][xx];
				if (c == 1) // -
				{
					top = true;
					bottom = true;
				}
				else if (c == 2) // |
				{
					right = true;
					left = true;
				}
				else if (c == 3) // |`
				{
					top = true;
					left = true;
				}
				else if (c == 4) // `|
				{
					top = true;
					right = true;
				}
				else if (c == 5) // |_
				{
					bottom = true;
					left = true;
				}
				else if (c == 6) // _|
				{
					bottom = true;
					right = true;
				}
				else if (c == 11) // +
				{
					screen[yy][xx] = c;
					continue;
				}

			}
			c = ' ';

			if ((left && right) && ((top || bottom) && onLeft))  c = 7;  // |-
			else if ((left && right) && ((top || bottom) && onRight)) c = 10;  // -|
			else if ((top && bottom) && ((left || right) && onTop)) c = 9;  // `|`
			else if ((top && bottom) && ((left || right) && onBottom)) c = 8;  //  _|_
			else if (((top && bottom) && (left || right)) ||
				((top || bottom) && (left && right))) c = 11;  // + 

			else if (top && right) c = 4;
			else if (top && left) c = 3;
			else if (bottom && right) c = 6;
			else if (bottom && left) c = 5;
			else if (top || bottom) c = 1;
			else if (right || left) c = 2;

			screen[yy][xx] = c;
		}
	}
}

void DlgCPU::mprintf(int x, int y, int w, int h, char const * format, ...)
{
	va_list  args;
	char  buf[1024];

	va_start(args, format);
	vsprintf(buf, format, args);

	int xx = x;
	int yy = y;
	for (int i = 0; i < strlen(buf); i++)
	{
		if (buf[i] == '\n')
		{
			xx = x;
			yy++;
		}
		else
		{
			screen[yy][xx] = buf[i];
			xx++;
		}

		if (xx > x + w - 1)
		{
			xx = x;
			yy++;
		}
		if (yy > y + h - 1) return;// No more space for text
	}
	va_end(args);
}

void DlgCPU::Update()
{
	if (cpu == NULL) return;
	uint8_t P = cpu->P;
	DrawRect(1, 3, 11, 12, true);
	mprintf(2, 4, 9, 10, "Registers\n"
		"A  $%02x\n"
		"X  $%02x\n"
		"Y  $%02x\n"
		"SP $%02x\n"
		"PC $%04x\n"
		"P  $%02x\n"
		"%c%c %c%c%c%c%c"
		, cpu->A, cpu->X, cpu->Y, cpu->SP, cpu->PC, cpu->P, 
		(P&NEGATIVE_FLAG ? 'N' : ' '),
		(P&OVERFLOW_FLAG ? 'V' : ' '),
		(P&BREAK_FLAG    ? 'B' : ' '),
		(P&DECIMAL_FLAG  ? 'D' : ' '),
		(P&INTERRUPT_FLAG? 'I' : ' '),
		(P&ZERO_FLAG     ? 'Z' : ' '),
		(P&CARRY_FLAG    ? 'C' : ' ')
		);

	disassembly();

	SDL_Rect r;
	// Update toolbox palette
	SDL_Surface *tps = SDL_GetWindowSurface(ToolboxCPU);
	SDL_FillRect(tps, NULL, 0xff000000);

	// Mouse selected
	r = { selectedX * 9 -1, selectedY * 9 -1, 10, 10 };
	SDL_FillRect(tps, &r, 0xffff0000);

	for (int y = 0; y < 64; y++)
	{
		for (int x = 0; x < 64; x++)
		{
			SDL_Rect r = { x * 9, y * 9, 8, 8 };
			int c = screen[y][x];
			if (c < Font.size()) 
			{
				SDL_Surface *s = Font.at(c);
				if (s == NULL) continue;
				SDL_BlitSurface(s, NULL, tps, &r);
			}
		}
	}

	SDL_UpdateWindowSurface(ToolboxCPU);

}

void DlgCPU::Clear()
{
	memset(screen, 32, 64 * 64);
	SDL_Surface *tns = SDL_GetWindowSurface(ToolboxCPU);
	SDL_FillRect( tns, NULL, 0x000000 );
	SDL_UpdateWindowSurface( ToolboxCPU );
}


void DlgCPU::Click(int x, int y, int button, bool pressed)
{
	int px = x / 9;
	int py = y / 9;

	if (this->cpu)
	{
		if (button == 0x01) // Left
		{
		}
		else if (button == 0x03) // Right
		{
		}
		else if (button == 0x02) // Middle / lock byte
		{
		}
		Log->Info("DlgCPU %s at %d, %d, %x", (pressed?"pressed ":"released"), px, py, button);
	}
}


void DlgCPU::Move(int x, int y)
{
	selectedX = x / 9;
	selectedY = y / 9;
}

void DlgCPU::Key(SDL_Keycode k)
{
	if (this->cpu && selectedX>=0 && selectedY>=0)
	{
		if (k > 128) return;
		screen[selectedY][selectedX] = k;
		if (++selectedX >= 64)
		{
			selectedX = 0;
			if (++selectedY >= 64) selectedY = 0;
		}
		Log->Info("DlgCPU press %c", k);
	}
}