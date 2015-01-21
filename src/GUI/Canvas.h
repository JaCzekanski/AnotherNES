#pragma once
#include <SDL.h>
#include <vector>
#include "Window.h"

class Canvas
{
	SDL_Renderer *renderer;

	//Window *lastWindow;
	bool windowSelected = false;
	std::vector< std::shared_ptr<Window> > windows;
public:
	Canvas( SDL_Renderer *renderer );
	~Canvas();

	int addWindow(Window *window);
	std::shared_ptr<Window> findWindow(int id);

	void Draw();
	bool Click(int x, int y, int button, bool released);
	void Move(int x, int y);
	bool Key(SDL_Keycode k);
};

