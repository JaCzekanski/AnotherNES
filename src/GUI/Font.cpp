#include "Font.h"


Font::Font()
{
	font = TTF_OpenFont("D:/Kuba/dev/AnotherNES/consolab.ttf", 12);
	color.r = 0xff;
	color.g = 0xff;
	color.b = 0xff;
	color.a = 0xff;
}


Font::~Font()
{
	if (font) 
	{
		TTF_CloseFont(font);
		font = NULL;
	}
}

int Font::getWidth(std::string text)
{
	if (!font) return 0;
	if (!renderer) return 0;
	
	SDL_Surface *textSurface = TTF_RenderText_Blended(font, text.c_str(), color);// TTF_RenderText_Solid(font, text.c_str(), color);
	int width = textSurface->w;
	SDL_FreeSurface(textSurface);

	return width;
}

int Font::getHeight(std::string text)
{
	if (!font) return 0;
	if (!renderer) return 0;

	SDL_Surface *textSurface = TTF_RenderText_Blended(font, text.c_str(), color);
	int height = TTF_FontHeight(font);;// textSurface->h;
	SDL_FreeSurface(textSurface);

	return height;
}

void Font::_render(std::string text, int x, int y, int width, int height, int adjust)
{
	if (!font) return;
	if (!renderer) return;

	SDL_Surface *textSurface = TTF_RenderText_Blended(font, text.c_str(), color);
	SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
	SDL_FreeSurface(textSurface);

	int textw, texth;
	SDL_QueryTexture(textTexture, NULL, NULL, &textw, &texth);

	SDL_Rect r;
	r.x = x;
	r.y = y;
	r.w = (width == 0) ? textw : width;
	r.h = (height == 0) ? texth : height;

	if (adjust == 1) //	 Center
	{
		r.x += (r.w / 2) - (textw / 2);
		r.y += (r.h / 2) - (texth / 2);
	}
	else if (adjust == 2) // Right adjusted
	{
		r.x += r.w - (textw / 2);
		r.y += r.h - (texth / 2);
	}

	r.w = textw;
	r.h = texth;
	
	SDL_RenderCopy(renderer, textTexture, NULL, &r);
	SDL_DestroyTexture(textTexture);
}

void Font::render(std::string text, int x, int y, int width = 0, int height = 0)
{
	_render(text, x, y, width, height, 0);
}
void Font::renderCenter(std::string text, int x, int y, int width, int height)
{
	_render(text, x, y, width, height, 1);
}
void Font::renderRight(std::string text, int x, int y, int width, int height)
{
	_render(text, x, y, width, height, 2);
}


void Font::setColor(SDL_Color color)
{
	this->color = color;
}
void Font::setColor(Uint8 color)
{
	this->color.r = color;
	this->color.g = color;
	this->color.b = color;
	this->color.a = 0xff;
}

void Font::setRenderer(SDL_Renderer *renderer)
{
	this->renderer = renderer;
}