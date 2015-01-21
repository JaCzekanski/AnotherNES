#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include <string>
#include <vector>
#include <memory>

#include "Widget.h"

const int titleBarHeight = 16;

static int windowIdCounter = 1;

class Window
{
	int pressedX, pressedY;

	std::vector< std::shared_ptr<Widget> > widgets;

	int id;
public:
	std::string name;
	int x, y;
	int width, height;
	bool moving, resizing;
	bool active;

	Window(std::string name, int x = 0, int y = 0, int width = 200, int height = 200);
	~Window();

	int getId() const { return id; }

	void addWidget(Widget *widget);
	void clearWidgets() { widgets.clear(); }
	
	void Draw( SDL_Renderer *renderer );

	void Click(int x, int y, int button, bool released); // Position is relative to widget
	void Move(int x, int y);
	void Key(SDL_Keycode k);
};

