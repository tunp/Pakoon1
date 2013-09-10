//g++ main.cpp Pakoon1View.cpp Pakoon1Doc.cpp BaseClasses.cpp BGame.cpp BMessages.cpp BPlayer.cpp BTerrain.cpp BSimulation.cpp BGround.cpp BScene.cpp BSceneEditor.cpp BTextures.cpp BObject.cpp BVehicle.cpp BCamera.cpp BTextRenderer.cpp BCmdModule.cpp BServiceWnd.cpp BMenu.cpp BUI.cpp BNavSatWnd.cpp OpenGLHelpers.cpp FileIOHelpers.cpp Settings.cpp ControllerModule.cpp SoundModule.cpp HeightMap.cpp PerlinNoise.cpp PakoonPhysicsEngine/PakoonPhysicsEngine.cpp StringTools.cpp -o Pakoon -lSDL2 -lGL -lGLU -lvorbisfile
#include "SDL2/SDL.h"

#include "Pakoon1View.h"

int main(int argc, char **argv) {
	SDL_Init(SDL_INIT_VIDEO);
	//SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1); 
	//SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16); 
	
	SDL_Window *window = SDL_CreateWindow("Pakoon1", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 960, SDL_WINDOW_OPENGL);
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
	
	CPakoon1View pakoon1;
	pakoon1.setWindow(window);
	pakoon1.window_width = 1280;
	pakoon1.window_height = 960;
	pakoon1.OnCreate();
	
	while (!pakoon1.isExit()) {
		pakoon1.OnDraw();
		
		SDL_Event event;
		while(SDL_PollEvent(&event))
		{
			SDL_Point point;
			if(event.type == SDL_KEYDOWN) {
				pakoon1.OnKeyDown(event.key.keysym.sym, 0, 0);
			} else if (event.type == SDL_KEYUP) {
				pakoon1.OnKeyUp(event.key.keysym.sym, 0, 0);
			} else if (event.type == SDL_MOUSEBUTTONDOWN) {
				if (event.button.button == 1) {
					point.x = event.motion.x;
					point.y = event.motion.y;
					pakoon1.OnLButtonDown(point);
				}
			} else if (event.type == SDL_MOUSEBUTTONUP) {
				if (event.button.button == 1) {
					point.x = event.motion.x;
					point.y = event.motion.y;
					pakoon1.OnLButtonUp(point);
				}
			} else if (event.type == SDL_MOUSEMOTION) {
				point.x = event.motion.x;
				point.y = event.motion.y;
				pakoon1.OnMouseMove(point);
			} else if (event.type == SDL_QUIT) {
				pakoon1.setExit();
			}
		}
	}
	
	SDL_Quit();
	
	return true;
}
