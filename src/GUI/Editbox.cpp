#include "Editbox.h"
#include "Font.h"


Editbox::Editbox(std::string name, int x, int y, int width, int height) : Widget(name, x, y, width, height), blinkerTimer(0), textPosition(name.length())
{
	if (width == 0)
		this->width = FONT.getWidth(name+"|.");
	if (height == 0)
		this->height = FONT.getHeight(name)+2;
}

void Editbox::Draw(SDL_Renderer *renderer)
{
	SDL_Rect r = { 0, 0, this->width, this->height };
	Uint8 brightness = (active) ? 0x60 : 0x40;
	Uint8 alpha = (active) ? 0xe0 : 0xd0;

	SDL_SetRenderDrawColor(renderer, brightness, brightness, brightness, alpha);
	SDL_RenderFillRect(renderer, &r);

	SDL_SetRenderDrawColor(renderer, brightness * 2, brightness * 2, brightness * 2, alpha);
	SDL_RenderDrawRect(renderer, &r);

	std::string text = name;
	if ( active && blinkerTimer < 30 ) text.insert(text.begin() + textPosition, '|');

	FONT.setColor(active ? 0xff : 0xa0);
	FONT.render(text, 1, 1, width - 2, height - 2); // set clipping?

	if (blinkerTimer++ >= 60) blinkerTimer = 0;
}

void Editbox::Click(int x, int y, int button, bool released = false)
{
	Widget::Click(x, y, button, released);

	blinkerTimer = 0;
}

void Editbox::Key(SDL_Keycode k)
{
	Widget::Key(k);
	if (k == SDLK_LEFT && textPosition > 0) 
	{
		textPosition--;
		blinkerTimer = 0;
	}
	if (k == SDLK_RIGHT && textPosition < name.length())
	{
		textPosition++;
		blinkerTimer = 0;
	}
	if (k & 0x40000000) return; // Special keys

	if (k == SDLK_ESCAPE) active = false;
	else if (k == SDLK_RETURN)
	{
		active = false;
		if (onEdit) onEdit(name);
	}
	else if (k == SDLK_BACKSPACE)
	{
		if (name.length() > 0 && textPosition - 1 >= 0) name.erase(name.begin() + textPosition - 1);
		textPosition--;
	}
	else if (k == SDLK_DELETE)
	{
		if (name.length() > 0) name.erase(name.begin() + textPosition);
		//if (textPosition >= name.length()) textPosition = name.length();
	}
	else
	{
		name.insert(name.begin() + textPosition, k);
		textPosition++;
	}
	if (textPosition < 0) textPosition = 0;
	if (textPosition > name.length()) textPosition = name.length();
}