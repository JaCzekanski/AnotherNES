#include "Canvas.h"


Canvas::Canvas( SDL_Renderer *renderer)
{
	this->renderer = renderer;

	windowSelected = false;
}


Canvas::~Canvas()
{
}

int Canvas::addWindow(Window *window)
{
	windows.push_back(std::shared_ptr<Window>(window));
	return window->getId();
}

std::shared_ptr<Window> Canvas::findWindow(int id)
{
	for (auto w : windows)
	{
		if (w->getId() == id) return w;
	}
	return NULL;
}

void Canvas::Draw()
{
	SDL_Texture *previousTexture = SDL_GetRenderTarget(renderer);

	for (auto w : windows)
	{
		SDL_Rect oldViewport, newViewport = { w->x, w->y, w->width, w->height };
		SDL_RenderGetViewport(renderer, &oldViewport);
		
		SDL_RenderSetViewport(renderer, &newViewport);
		w->Draw(renderer);
		
		SDL_RenderSetViewport(renderer, &oldViewport);
	}
}

bool Canvas::Click(int x, int y, int button, bool released = false)
{
	for (auto w : windows)
		w->active = false;

	windowSelected = false;

	int pos = 0;
	for (auto w : windows)
	{
		if (w->moving || w->resizing ||
			(x >= w->x && x < w->x + w->width &&
			 y >= w->y && y < w->y + w->height))
		{
			windows.erase(windows.begin()+pos);
			windows.insert(windows.begin(), w);

			w->active = true;
			w->Click(x - w->x, y - w->y, button, released);

			windowSelected = true;
			return true;
		}
		pos++;
	}
	return false; //No window was handled
}

void Canvas::Move(int x, int y)
{
	if (!windowSelected) return;

	auto w = windows.front();
	w->Move(x - w->x, y - w->y);
}

bool Canvas::Key(SDL_Keycode k)
{
	for (auto w : windows)
	{
		if (w->active)
		{
			w->Key(k);
			return true;
		}
	}
	return false;
}
