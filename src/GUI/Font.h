#pragma once
#include <SDL_ttf.h>
#include <SDL.h>
#include <string>

#define FONT Font::getInstance()

class Font
{
	TTF_Font *font = NULL;
	SDL_Renderer *renderer;
	SDL_Color color;
	Font();
	~Font();

	void _render(std::string text, int x, int y, int width, int height, int adjust);
public:
	
	static Font& getInstance()
	{
		static Font instance;
		return instance;
	}

	int getWidth(std::string text);
	int getHeight(std::string text);

	void render(std::string text, int x, int y, int width, int height);
	void renderCenter(std::string text, int x, int y, int width, int height);
	void renderRight(std::string text, int x, int y, int width, int height);
	void setColor(SDL_Color color);
	void setColor(Uint8 color);
	void setRenderer(SDL_Renderer *renderer);

};

