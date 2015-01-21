#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include <string>
#include <vector>
#include <functional>


class Widget
{
protected:
	std::string name;


	std::function<void(int x, int y, int button, int released)> onClick;
	std::function<void(SDL_Keycode k)> onKey;
public:
	int x, y;
	int width, height;
	bool active;

	Widget(std::string name, int x, int y, int width = 0, int height = 0);
	virtual ~Widget();

	void setOnClick(std::function<void(int x, int y, int button, int released)> f) { onClick = f; }
	void setOnKey  (std::function<void(SDL_Keycode k)> f) { onKey = f; }
	
	void setName(std::string name);
	std::string getName() const;
	
	virtual void Draw( SDL_Renderer *renderer );
	virtual void Click(int x, int y, int button, bool released); 
	//void Move(int x, int y);
	virtual void Key(SDL_Keycode k);
	//virtual void Char(int k);
};

