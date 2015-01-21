#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include <string>
#include <vector>
#include <functional>

#include "widget.h"

class Button : public Widget
{
	bool clicked;
	bool hover;

	std::function<void()>  onPress;
public:
	Button(std::string name, int x, int y, int width = 0, int height = 0);

	void setOnPress(std::function<void()> f) { onPress = f; }

	void Draw( SDL_Renderer *renderer );
	void Click(int x, int y, int button, bool released);
	void Key(SDL_Keycode k);
};

