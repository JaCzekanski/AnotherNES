#include "Window.h"
#include "Font.h"

Window::Window(std::string name, int x, int y, int width, int height)
{
	this->x = x;
	this->y = y;
	this->width = width;
	this->height = height;
	this->name = name;

	moving = false;
	resizing = false;

	pressedX = pressedY = 0;

	active = false;

	id = windowIdCounter++;
}


Window::~Window()
{
}


void Window::addWidget(Widget *widget)
{
	//Widget *w = new Widget(widget);

	widgets.push_back(std::shared_ptr<Widget>(widget));
//	widgets.push_back(std::unique_ptr<Widget*>(new Widget(widget)));
}

void Window::Draw(SDL_Renderer *renderer)
{
	SDL_Rect r = { 0, 0, this->width, this->height };

	Uint8 brightness = (active) ? 0x40 : 0x20;
	Uint8 alpha = (active) ? 0xe0 : 0xd0;

	// Frame
	SDL_SetRenderDrawColor(renderer, brightness * 2, brightness * 2, brightness * 2, alpha);
	SDL_RenderDrawRect(renderer, &r);

	// Window
	r.x += 1; r.y += 1; r.w -= 2; r.h -= 2;
	SDL_SetRenderDrawColor(renderer, brightness, brightness, brightness, alpha);
	SDL_RenderFillRect(renderer, &r);

	// Titlebar
	r.x = 1;
	r.w = this->width - 2;
	r.y = 1;
	r.h = titleBarHeight;
	SDL_SetRenderDrawColor(renderer, brightness * 2, brightness * 2, brightness * 2, alpha);
	SDL_RenderFillRect(renderer, &r);
	
	FONT.setColor(active ? 0xff : 0xa0);
	FONT.renderCenter(name.c_str(), 1, 1, this->width - 2, titleBarHeight);

	SDL_Texture *previousTexture = SDL_GetRenderTarget(renderer);

	SDL_Rect oldViewport;
	SDL_RenderGetViewport(renderer, &oldViewport);

	for (auto w : widgets)
	{
		SDL_Rect newViewport = { 
			oldViewport.x + w->x + 1,
			oldViewport.y + w->y + 1 + titleBarHeight,
			(w->width  < (width - w->x)) ? w->width : width - 2 - w->x,
			(w->height < (height - w->y - titleBarHeight)) ? w->height : height - 2 - titleBarHeight - w->y };
	
		SDL_RenderSetViewport(renderer, &newViewport);
		//SDL_Rect rect = { 0,0,it->width, it->height };
		//SDL_SetRenderDrawColor(renderer, brightness * 2, 0, 0, alpha);
		//SDL_RenderFillRect(renderer, &rect);
		w->Draw(renderer);
	}
	SDL_RenderSetViewport(renderer, &oldViewport);
}

void Window::Click(int x, int y, int button, bool released = false)
{
	resizing = false;
	moving = false;
	if (!active) return;

	pressedX = x;
	pressedY = y;

	if (!released && y <= titleBarHeight && !resizing)
	{
		moving = true;
		return;
	}
	if (!released && x > width - 10 && y > height - 10 && !moving)
	{
		resizing = true;
		return;
	}

	int rx = x - 1;
	int ry = y - 1 - titleBarHeight;

	for (auto w : widgets) w->active = false;
	for (auto w : widgets)
	{
		if ((rx >= w->x && rx < w->x + w->width &&
			ry >= w->y && ry < w->y + w->height))
		{
			w->active = true;
			w->Click(rx - w->x, ry - w->y, button, released);
			return;
		}
	}
}


void Window::Move(int x, int y)
{
	if (!active) {
		moving = false;
		resizing = false;
		return;
	}

	if (!moving && ! resizing) return;
	
	if (moving)
	{
		this->x -= pressedX - x;
		this->y -= pressedY - y;
		if (this->x < 0) this->x = 0;
		if (this->y < 0) this->y = 0;
	}

	if (resizing)
	{
		width += x-pressedX;
		height += y-pressedY;

		pressedX = x;
		pressedY = y;

		int minWidth = FONT.getWidth(name) + 10;
		if (width < minWidth) width = minWidth;
		if (height < 40) height = 40;
	}
}


void Window::Key(SDL_Keycode k)
{
	for (auto w : widgets)
	{
		if (w->active)
		{
			w->Key(k);
			return;
		}
	}
}