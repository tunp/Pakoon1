CC = g++
CFLAGS = -c
LDFLAGS = -lSDL2 -lGL -lGLU -lvorbisfile

OBJECTS = main.o Pakoon1View.o Pakoon1Doc.o BaseClasses.o BGame.o BMessages.o BPlayer.o BTerrain.o BSimulation.o BGround.o BScene.o BSceneEditor.o BTextures.o BObject.o BVehicle.o BCamera.o BTextRenderer.o BCmdModule.o BServiceWnd.o BMenu.o BUI.o BNavSatWnd.o OpenGLHelpers.o FileIOHelpers.o Settings.o ControllerModule.o SoundModule.o HeightMap.o PerlinNoise.o PakoonPhysicsEngine/PakoonPhysicsEngine.o StringTools.o
EXECUTABLE = Pakoon

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $^ -o $@

$(OBJECTS): %.o: %.cpp
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm $(EXECUTABLE) $(OBJECTS)
