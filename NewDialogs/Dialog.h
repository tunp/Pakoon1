#ifndef DIALOG_H
#define DIALOG_H

#include <vector>
#include <iostream>
#include <string>

using namespace std;

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "Item.h"

class Dialog {
protected:
	SDL_Surface *surface;
	SDL_Surface *background;
	SDL_Rect pos;
	vector<Item *> items;
private:
	bool default_pos;
	bool exit;
public:
	bool loadBackground(string name);
	void addItem(Item *item);
	void deleteItems();
  virtual bool needsRedraw();
	void draw();
	SDL_Surface *getSurface();
	void deleteSurfaceAndBackground();
	virtual bool onMousePress(int x, int y);
	virtual bool onMouseRelease(int x, int y);
	virtual bool onMouseMove(int x, int y);
	virtual bool onFingerDown(int x, int y, int finger_id);
	virtual bool onFingerUp(int x, int y, int finger_id);
	void setPos(SDL_Rect pos);
	SDL_Rect *getPos();
	void setExit(bool exit);
	bool isExit();
	bool isDefaultPos();
	void setDefaultPos(bool default_pos);
};

#endif
