#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include <string>
#include <vector>
#include <functional>

#include "widget.h"

class Editbox : public Widget
{
	int blinkerTimer;
	int textPosition;

	std::function<void(std::string)>  onEdit;
public:
	Editbox(std::string name, int x, int y, int width = 0, int height = 0);
	
	void setOnEdit(std::function<void(std::string)> f) { onEdit = f; }


	void Draw( SDL_Renderer *renderer );
	void Click(int x, int y, int button, bool released);
	void Key(SDL_Keycode k);
};

