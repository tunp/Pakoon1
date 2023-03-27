CC = g++
CFLAGS = -c
LDFLAGS = -lSDL2 -lSDL2_ttf -lSDL2_image -lGL -lGLU -lvorbisfile

EMXX = em++
#EMXXFLAGS = -Oz -c -s USE_SDL=2 -s USE_VORBIS=1 -I/home/jettis/val/emscripten-ports-regal/include
EMXXFLAGS = -Oz -c -s USE_SDL=2 -s USE_SDL_TTF=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='["png"]' -s USE_VORBIS=1 -I/home/jettis/git/gl4es/include
#EMXXLINK = -s USE_SDL=2 -s USE_VORBIS=1 -s USE_REGAL=1 -s ALLOW_MEMORY_GROWTH=1 -s ERROR_ON_UNDEFINED_SYMBOLS=0
#EMXXLINK = -s USE_SDL=2 -s USE_VORBIS=1 -s LEGACY_GL_EMULATION=1 -s ALLOW_MEMORY_GROWTH=1 -s ERROR_ON_UNDEFINED_SYMBOLS=0
# emmake make -f Makefile.regal SYSTEM=emscripten
# emmake make -f Makefile.glu SYSTEM=emscripten
#EMXXLINK = -s USE_SDL=2 -s USE_VORBIS=1 -s FULL_ES2=1 -L/home/jettis/val/emscripten-ports-regal/lib/emscripten -lRegallib -lRegalGLUlib -s ALLOW_MEMORY_GROWTH=1
EMXXLINK = -s USE_SDL=2 -s USE_SDL_TTF=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='["png"]' -s USE_VORBIS=1 -s FULL_ES2=1 -L/home/jettis/git/gl4es/lib -lGL4es -L/home/jettis/git/GLU/.libs -lGLU4es -s ALLOW_MEMORY_GROWTH=1 -sSTACK_SIZE=5MB

# gl4es and GLU with emscripten:
# gl4es:
# git clone https://github.com/ptitSeb/gl4es
# Need to change library name so change GL to GL4es in src/CMakeLists.txt
# This is needed because emscripten libGL provides emscripten_getProcAddress etc and gl4es others, so both are needed.
# emcmake cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo -DNOX11=ON -DNOEGL=ON -DSTATICLIB=ON; make
# GLU:
# git clone https://github.com/ptitSeb/GLU
# Need to have mangled function names as gl4es haves them. So add these to src/include/GL/gl.h:
# ----
# #define MANGLE(x) gl4es_gl##x
# #include "gl_mangle.h"
# ----
# And add this to include/GL/glu.h:
# ----
# #include "glu_mangle.h"
# ----
# Add symlink ln -s path/to/gl4es/include/GL/gl_mangle.h src/include/GL/gl_mangle.h
# Compile with emconfigure ./configure and emmake make
# Compile gives error with linking step but just run the shown wasm-ld command again without the -retain-symbols-file argument.
# Add symlink after compile: ln -s libGLU.so.1.3.1 .libs/libGLU4es.so

NEW_DIALOG_OBJECTS = Dialog Item Button DlgOnScreenKbd KbdButton
OBJECTS = main Pakoon1View Pakoon1Doc BaseClasses BGame BMessages BPlayer BTerrain BSimulation BGround BScene BSceneEditor BTextures BObject BVehicle BCamera BTextRenderer BCmdModule BServiceWnd BMenu BUI BNavSatWnd OpenGLHelpers FileIOHelpers Settings ControllerModule SoundModule HeightMap PerlinNoise PakoonPhysicsEngine/PakoonPhysicsEngine StringTools $(addprefix NewDialogs/,$(NEW_DIALOG_OBJECTS))
ASSETS = vehicles scenes Textures/MessageLetters.txt Textures/MainGameMenu.tga Textures/wheeldetailed.tga Textures/skydetailed.tga Textures/groundtexture2.tga Textures/dashboard.tga Textures/COMPASS.TGA Textures/LedNumbersWhite.tga Textures/NavSatWndBigLetters.tga Textures/shadowetc.tga Textures/DustCloud.tga Textures/CloudPuff.tga Textures/EnvMap.tga Textures/EnvMapShiny.tga Textures/QuickHelp.tga Textures/QuickHelpSceneEditor.tga Textures/WaterSurface.tga Textures/GasStation.tga Textures/Bricks.tga Textures/MenuTitles.tga Textures/Fueling.tga Sounds/MixDown3.ogg Sounds/EngineJuliet2.ogg Sounds/CRASH.ogg Bogian.vehicle Spirit.vehicle Veyronette.vehicle Coastal.scene Desert.scene IceWorld.scene CraterIsland.scene Haven.scene TheWall.scene Textures/Saga1.tga Textures/Saga2.tga Textures/Saga3.tga Textures/Saga4.tga Textures/Saga5.tga Textures/CreditsProgramming.tga Textures/CreditsModeling.tga Textures/CreditsBetaTesting.tga Textures/CreditsSpecialThanks1.tga Textures/CreditsEnd.tga Textures/Bogian.tga Textures/Spirit.tga Textures/Veyronette.tga Textures/coastal.tga Textures/Desert.tga Textures/IceWorld.tga Textures/CraterIsland.tga Textures/Haven.tga Textures/TheWall.tga Models/Stonehenge.object Models/Base.object Models/EconoHut.object Textures/groundtexture.tga Textures/Granite.tga Models/Stonehenge1rounded.obj Models/Stonehenge2rounded.obj Models/Stonehenge3rounded.obj Models/Stonehenge4rounded.obj Models/Stonehenge5rounded.obj Models/Stonehenge1.obj Models/Stonehenge2.obj Models/Stonehenge3.obj Models/Stonehenge4.obj Models/Stonehenge5.obj Models/Base_vis.obj Models/Base_CollDet.obj Models/EconoHut1.obj Models/EconoHut2.obj Models/Bogian.obj Textures/Snowy1.tga Textures/IceWorld_EnvMap.tga Textures/RustySteel.tga Models/Spirit_Main.obj Models/Spirit_Cowl.obj Models/Spirit_Blacks.obj Models/Spirit_Rudder.obj Models/Spirit_Elevator.obj Models/Spirit_Propeller.obj Models/Veyronette.obj Models/Veyronette_Roof.obj Textures/wheeldetailed2.tga FreeSans.ttf
EXECUTABLE = Pakoon

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS:%=%.o)
	$(CC) $(LDFLAGS) $^ -o $@

%.o: %.cpp
	$(CC) $(CFLAGS) $< -o $@

Pakoon.html: $(OBJECTS:%=%.em.o) $(ASSETS)
	$(EMXX) $(EMXXLINK) $(filter %.o,$^) $(ASSETS:%=--preload-file %) -o $@

%.em.o: %.cpp
	$(EMXX) $(EMXXFLAGS) $< -o $@

.PHONY: clean
clean:
	rm $(EXECUTABLE) $(OBJECTS:%=%.o)
