//g++ main.cpp Pakoon1View.cpp Pakoon1Doc.cpp BaseClasses.cpp BGame.cpp BMessages.cpp BPlayer.cpp BTerrain.cpp BSimulation.cpp BGround.cpp BScene.cpp BSceneEditor.cpp BTextures.cpp BObject.cpp BVehicle.cpp BCamera.cpp BTextRenderer.cpp BCmdModule.cpp BServiceWnd.cpp BMenu.cpp BUI.cpp BNavSatWnd.cpp OpenGLHelpers.cpp FileIOHelpers.cpp Settings.cpp ControllerModule.cpp SoundModule.cpp HeightMap.cpp PerlinNoise.cpp PakoonPhysicsEngine/PakoonPhysicsEngine.cpp StringTools.cpp -o Pakoon -lSDL2 -lGL -lGLU -lvorbisfile
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "Pakoon1View.h"

#include <iostream>

#ifdef __EMSCRIPTEN__
//#include <GL/Regal.h>
//#include <GL/RegalGLU.h>
#include <GL/gl.h>
#include <emscripten.h>
extern "C" void initialize_gl4es();
#endif


using namespace std;

void mainLoop(CPakoon1View *pakoon1) {
  pakoon1->OnDraw();

  SDL_Event event;
  while(SDL_PollEvent(&event))
  {
    SDL_Point point;
    if(event.type == SDL_KEYDOWN) {
      pakoon1->OnKeyDown(event.key.keysym.sym, 0, 0);
      if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_BACKSPACE) {
        pakoon1->OnChar(event.key.keysym.sym, 0, 0);
      }
    } else if (event.type == SDL_KEYUP) {
      pakoon1->OnKeyUp(event.key.keysym.sym, 0, 0);
    } else if (event.type == SDL_MOUSEBUTTONDOWN) {
      if (event.button.button == 1) {
        point.x = event.motion.x;
        point.y = event.motion.y;
        pakoon1->OnLButtonDown(point);
      }
    } else if (event.type == SDL_MOUSEBUTTONUP) {
      if (event.button.button == 1) {
        point.x = event.motion.x;
        point.y = event.motion.y;
        pakoon1->OnLButtonUp(point);
      }
    } else if (event.type == SDL_MOUSEMOTION) {
      point.x = event.motion.x;
      point.y = event.motion.y;
      pakoon1->OnMouseMove(point);
    } else if (event.type == SDL_FINGERDOWN) {
      pakoon1->OnFingerDown(event.tfinger.x, event.tfinger.y, event.tfinger.fingerId);
    } else if (event.type == SDL_FINGERUP) {
      pakoon1->OnFingerUp(event.tfinger.x, event.tfinger.y, event.tfinger.fingerId);
    } else if (event.type == SDL_QUIT) {
      pakoon1->setExit();
    } else if (event.type == SDL_TEXTINPUT) {
      //FIXME handle full text
      pakoon1->OnChar((int) event.text.text[0], 0, 0);
    }
  }
}

#ifdef __EMSCRIPTEN__
void emscripten_loop(void *pakoon1) {
  if (((CPakoon1View *)pakoon1)->isExit()) {
    emscripten_run_script("history.back()");
    emscripten_cancel_main_loop();
  } else {
    mainLoop((CPakoon1View *)pakoon1);
  }
}
#endif

int main(int argc, char **argv) {
#ifdef __EMSCRIPTEN__
  initialize_gl4es();
#endif
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		cout << "SDL init failed" << endl;
	}
  
  if(TTF_Init() == -1) {
    cout << "TTF_Init: " << TTF_GetError() << endl;
    return 1;
  }

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1); 
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16); 
	
#ifdef __EMSCRIPTEN__
	SDL_Window *window = SDL_CreateWindow("Pakoon1", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_OPENGL);
#else
	SDL_Window *window = SDL_CreateWindow("Pakoon1", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 0, 0, SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN_DESKTOP);
#endif
	if (window == NULL) {
		cout << "error creating window" << endl;
	}
	SDL_GLContext context = SDL_GL_CreateContext(window);
#ifdef __EMSCRIPTEN__
  //RegalMakeCurrent((RegalSystemContext)1);
#endif
	
	CPakoon1View pakoon1;
	pakoon1.setWindow(window);
	SDL_GetWindowSize(window, &pakoon1.window_width, &pakoon1.window_height);
	pakoon1.OnCreate();

#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop_arg((em_arg_callback_func)emscripten_loop, &pakoon1, 0, 1);
	SDL_GL_SetSwapInterval(0);
#else
	SDL_GL_SetSwapInterval(0);
	while (!pakoon1.isExit()) {
    mainLoop(&pakoon1);
  }
  TTF_Quit();
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();
#endif
	
	return 0;
}
