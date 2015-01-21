#include "Button.h"
#include "Font.h"


Button::Button(std::string name, int x, int y, int width, int height) : Widget(name, x, y, width, height)
{
	clicked = false;
	hover = true;
	if (width == 0)
		this->width = FONT.getWidth(name)+2;
	if (height == 0)
		this->height = FONT.getHeight(name)+2;
}

void Button::Draw(SDL_Renderer *renderer)
{
	SDL_Rect r = { 0, 0, this->width, this->height };
	Uint8 brightness = (active) ? 0x45 : 0x40;
	Uint8 alpha = (active) ? 0xe0 : 0xd0;

	if (hover) brightness += 0x10;
	if (clicked) brightness += 0x20;

	SDL_SetRenderDrawColor(renderer, brightness, brightness, brightness, alpha);
	SDL_RenderFillRect(renderer, &r);

	SDL_SetRenderDrawColor(renderer, brightness * 2, brightness * 2, brightness * 2, alpha);
	SDL_RenderDrawRect(renderer, &r);

	FONT.setColor(active ? 0xff : 0xa0);
	FONT.renderCenter(name, 1, 1, width - 2, height - 2);
}

void Button::Click(int x, int y, int button, bool released = false)
{
	Widget::Click(x, y, button, released);

	if (button == SDL_BUTTON_LEFT)
	{
		active = true;
		if (released) clicked = false;
		else {
			onPress();
			clicked = true;
		}
	}
}

void Button::Key(SDL_Keycode k)
{
	Widget::Key(k);
	if (k == SDLK_ESCAPE) active = false;
	else if (k == SDLK_RETURN || k == SDLK_SPACE)
	{
		if (onClick) onClick(x, y, SDL_BUTTON_LEFT, false);
	}
}