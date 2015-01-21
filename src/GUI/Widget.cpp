#include "Widget.h"
#include "Font.h"

Widget::Widget(std::string name, int x, int y, int width, int height)
{
	this->x = x;
	this->y = y;
	this->width = width;
	this->height = height;
	this->name = name;
	
	if (width == 0)
		this->width = FONT.getWidth(name);
	if (height == 0)
		this->height = FONT.getHeight(name);
	active = false;
}


Widget::~Widget()
{
}

void Widget::setName(std::string name)
{
	this->name = name;
}
std::string Widget::getName() const
{
	return this->name;
}

void Widget::Draw(SDL_Renderer *renderer)
{
	if (name.empty()) return;
	FONT.setColor(active ? 0xff : 0xa0);
	FONT.render(name.c_str(), 0, 0, width, height);
}

void Widget::Click(int x, int y, int button, bool released = false)
{
	if (onClick) onClick(x, y, button, released);
}

void Widget::Key(SDL_Keycode k)
{
	if (onKey) onKey(k);
}