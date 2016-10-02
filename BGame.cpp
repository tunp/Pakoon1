//
// BGame: Center location for controlling the game
//
// (c) Copyright 2002, Mikko Oksalahti (see end of file for details)
//

#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>

using namespace std;

//#include "stdafx.h"
#include "BGame.h"
#include "FileIOHelpers.h"
#include "BTextures.h" 
#include "SoundModule.h"
#include "BMessages.h"
#include "HeightMap.h"
#include "Pakoon1View.h"

BSimulation   BGame::m_simulation;
BPlayer       BGame::m_player;
BCmdModule    BGame::m_cmdModule;
BNavSatWnd    BGame::m_navsatWnd;
BServiceWnd   BGame::m_serviceWnd;
BSceneEditor  BGame::m_sceneEditor;
CPakoon1View *BGame::m_pView = 0;

int     BGame::m_nDispWidth;
int     BGame::m_nDispHeight;
int     BGame::m_nDispBits;
int     BGame::m_nDispHz;
bool    BGame::m_bDisplayInfo;
int     BGame::m_nSkyDetail;
int     BGame::m_nDistantDetail;
int     BGame::m_nTerrainResolution;
int     BGame::m_nDustAndClouds;
int     BGame::m_nWaterSurface = 0;
int     BGame::m_nMusicVolume;
int     BGame::m_nVehicleVolume;
int     BGame::m_nSoundscape;
int     BGame::m_nWaterDetail;
int     BGame::m_nColorMode;
int     BGame::m_nScreenFormat;
int     BGame::m_nTextureSmoothness; 
bool    BGame::m_bDashboard;
bool    BGame::m_bNavSat;
bool    BGame::m_bService;
bool    BGame::m_bSlowMotion;
bool    BGame::m_bFrozen;
bool    BGame::m_bShowDust;
bool    BGame::m_bShowQuickHelp;
bool    BGame::m_bDrawOnScreenTracking;
double  BGame::m_dNavSatHandleAngle;
double  BGame::m_dServiceHandleAngle;
unsigned BGame::m_clockFrozenStart = 0;
unsigned BGame::m_clockLastLift = 0;
int     BGame::m_nFreezeRefCount = 0;
int     BGame::m_nPhysicsSteps; 
bool    BGame::m_bShowHint;
unsigned BGame::m_clockHintStart;
int     BGame::m_nGameMenuSelection = 5; // Return to game by default
int     BGame::m_nYesNoSelection = 1; // Yes by default
bool    BGame::m_bShowGameMenu = false;
bool    BGame::m_bShowCancelQuestion = false;
bool    BGame::m_bSceneEditorMode = true;
bool    BGame::m_bFadingIn = false;
unsigned BGame::m_clockFadeStart = 0;

string BGame::m_sScene = "";
string BGame::m_sVehicle = "";

bool    BGame::m_bMenuMode = true;
BMenu  *BGame::m_pMenuCurrent = 0;
BMenu  *BGame::m_pMenuPrevious = 0;
BMenu   BGame::m_menuMain;
BMenu   BGame::m_menuChooseScene;
BMenu   BGame::m_menuChooseVehicle;
BMenu   BGame::m_menuSettings;
BMenu   BGame::m_menuSaga;
BMenu   BGame::m_menuCredits;
BMenu   BGame::m_menuPrecachingTerrain;
bool    BGame::m_bMenusCreated = false;
bool    BGame::m_bSettingsFromGame = false;

double  BGame::m_dPizzaTemp = 0.0;
unsigned BGame::m_clockLastPizzaTempCheck = 0;
unsigned BGame::m_clockPickupStart = 0;
unsigned BGame::m_clockDeliveryStart = 0;
bool    BGame::m_bDeliveryStartInProgress = false;
bool    BGame::m_bPickupStartInProgress = false;
int     BGame::m_nPizzaDamaged = 0;

double  BGame::m_dLastProfit = 0.0;
double  BGame::m_dLastDeliveryDistance = 1.0;

int     BGame::m_nPizza = 0;
int     BGame::m_nPizzas = 11;
string BGame::m_sPizzas[11] = {"Opera", 
                              "Quattro Stagioni", 
                              "Capricciosa", 
                              "Frutti di Mare", 
                              "Cacciatora", 
                              "Tropicana", 
                              "Vegetariana",
                              "Margherita",
                              "Pepperoni",
                              "Mexicana",
                              "Americana"};


bool    BGame::m_bGameLoading = false;
bool    BGame::m_bGameReadyToStart = false;
bool    BGame::m_bQuitPending = false;

//BGame::TState  BGame::m_state = BGame::TState::PICKUP;
BGame::TState  BGame::m_state = BGame::PICKUP;

BVector BGame::m_vGasStationClosest = BVector(-9999.0, -9999.0, -9999.0);
unsigned BGame::m_clockLastFuelExit = 0;
unsigned BGame::m_clockFuelingStarted = 0;
bool    BGame::m_bFueling = false;
int     BGame::m_nFuelSelect = 0;
bool    BGame::m_bFuelingInProgress = false;

BObject *BGame::m_pClient = 0;
BObject *BGame::m_pBase = 0;


//CRITICAL_SECTION BGame::m_csMutex;
SDL_mutex *BGame::m_csMutex;
double  BGame::m_dProgressMax = 1.0;
double  BGame::m_dProgressPos = 0.0;

bool   BGame::m_bJumpToHome = false;

bool   BGame::m_bBuyingVehicle = false;
bool   BGame::m_bCannotBuyVehicle = false;
double BGame::m_dPurchasePrice = 100.0;

bool    BGame::m_bHasEarthquakes = false;
bool    BGame::m_bEarthquakeActive = false;
double  BGame::m_bEarthquakeFactor = 0.0;
unsigned BGame::m_bEarthquakeStarted = 0;
unsigned BGame::m_bEarthquakeWillEnd = 0;
unsigned BGame::m_bEarthquakeNextStart = 0;

bool    BGame::m_bWindActive = true;
BVector BGame::m_vWindDirection = BVector(1, 0, 0);
BVector BGame::m_vWindParticles[g_cnWindParticles];
double  BGame::m_dWindStrength = 0.1;
double  BGame::m_dBaseWindStrength = 0.07; // was 0.1

bool    BGame::m_bMultiProcessor = false;

bool    BGame::m_bAnalyzerMode = false;
unsigned BGame::m_clockAnalyzerStarted = 0;
int     BGame::m_nVisualize = 255; // all on


BUISelectionList BGame::m_listYesNo;
BUISelectionList BGame::m_listOK;


int              BGame::m_nController;
//BControllerState BGame::m_controllerstate; // Access to a controller, such as a joystick or a wheel //FIXME

//*************************************************************************************************
BGame::BGame() {

  time_t ltime;
  time(&ltime);

  FILE *fp = fopen("Pakoon1.log", "w");
  fprintf(fp, "Pakoon1 started (build 4) *** %s--------------------------------------------\n", ctime(&ltime));
  fclose(fp);

  m_cmdModule.SetSim(&m_simulation);
  m_nController = 0;        // 0 = keyboard, 1 = joystick
  m_bDisplayInfo = true;    // Show fps, speed, altitude etc.
  m_nSkyDetail = 2;         // High
  m_nDistantDetail = 2;     // High
  m_nColorMode = 1;         // Color
  m_nWaterDetail = 1;       // Draw water surface
  m_nScreenFormat = 0;      // Full screen
  m_nTextureSmoothness = 2; // Silky
  m_bDashboard = true;
  m_bNavSat = false;
  m_bService = false;
  m_bSlowMotion = false;
  m_bFrozen = false;
  m_dNavSatHandleAngle = 0;
  m_dServiceHandleAngle = -20;
  m_bShowDust = true;
  m_bShowQuickHelp = false;
  m_bDrawOnScreenTracking = true;
  m_nPhysicsSteps = 10;
  m_bShowHint = false;
  m_clockHintStart = SDL_GetTicks();

  static string sYesNo[2] = {"Yes", "No"};
  static string sOK[1] = {"OK"};
  m_listYesNo.SetItems(sYesNo, 2);
  m_listYesNo.SelectItem("Yes");
  m_listOK.SetItems(sOK, 1);
  m_listOK.SelectItem("OK");

  // Setup common tracking targets
  m_simulation.AddTrackingTarget("FUEL", BVector(0, 0, 0), 1, 0, 0);         // Red Fuel

  //InitializeCriticalSection(&m_csMutex);
  m_csMutex = SDL_CreateMutex();

  m_pMenuCurrent = &m_menuMain;
}

//*************************************************************************************************
BGame::~BGame() {
  //DeleteCriticalSection(&m_csMutex);
  SDL_DestroyMutex(m_csMutex);
}


//*************************************************************************************************
void BGame::SetProgressRange(double dMax) {
  //EnterCriticalSection(&m_csMutex);
  SDL_LockMutex(m_csMutex);
  m_dProgressMax = dMax;
  //LeaveCriticalSection(&m_csMutex);
  SDL_UnlockMutex(m_csMutex);
}


//*************************************************************************************************
void BGame::SetProgressPos(double dPos) {
  //EnterCriticalSection(&m_csMutex);
  SDL_LockMutex(m_csMutex);
  m_dProgressPos = dPos;
  //LeaveCriticalSection(&m_csMutex);
  SDL_UnlockMutex(m_csMutex);
  if(!m_bMultiProcessor) {
    //Sleep(1);
    SDL_Delay(1);
  }
}


//*************************************************************************************************
double BGame::GetRelativeProgress() {
  double dRet;
  //EnterCriticalSection(&m_csMutex);
  SDL_LockMutex(m_csMutex);
  dRet = m_dProgressPos / m_dProgressMax;
  //LeaveCriticalSection(&m_csMutex);
  SDL_UnlockMutex(m_csMutex);
  return dRet;
}


//*************************************************************************************************
void BGame::SetupMenus() {
	
  //******************************************
  // Choose Vehicle
  //******************************************

  //m_menuChooseVehicle.m_type = BMenu::TType::CHOOSE_VEHICLE;
  m_menuChooseVehicle.m_type = BMenu::CHOOSE_VEHICLE;
  m_menuChooseVehicle.m_nTitleWidth = 284;
  m_menuChooseVehicle.m_nTitleHeight = 60;
  m_menuChooseVehicle.m_dTitleX = 0;
  m_menuChooseVehicle.m_dTitleY = (512.0 - 177.0) / 512.0;

  // Fetch vehicles

  //CFileFind finder;
  //string strWildcard = _T(".\\*.vehicle");
  //string strWildcard = ".\\*.vehicle";

  // count .vehicle files
  int nVehicles = 0;
  /*BOOL bWorking = finder.FindFile(strWildcard);
  while(bWorking) {
    bWorking = finder.FindNextFile();
    ++nVehicles;
  }
  finder.Close();*/
  
	ifstream vehicles_file("vehicles");
	string vehicle;
	vector<string> vehicles;
	while (!vehicles_file.eof()) {
		getline(vehicles_file, vehicle);
		if (!vehicle.empty() && vehicle[0] != '\r' && vehicle[0] != '\n') {
			vehicles.push_back(vehicle);
			nVehicles++;
		}
	}
	vehicles_file.close();

  m_menuChooseVehicle.m_nItems = nVehicles;
  m_menuChooseVehicle.m_sItems = new string[nVehicles];
  m_menuChooseVehicle.m_items = new BMenuItem[nVehicles];

  // Load .vehicle files
  int i = 0;
  //bWorking = finder.FindFile(strWildcard);
  //while(bWorking) {
  for (int x = 0; x < nVehicles; x++) {
    //bWorking = finder.FindNextFile();
    // Add filename to object list
    //m_menuChooseVehicle.m_items[i].m_sAssocFile = finder.GetFilePath();
    m_menuChooseVehicle.m_items[i].m_sAssocFile = vehicles[x];
    FileHelpers::GetKeyStringFromINIFile("Properties",
                                         "Name",
                                         //finder.GetFileTitle(),
                                         "",
                                         m_menuChooseVehicle.m_items[i].m_sText,
                                         //finder.GetFilePath());
                                         vehicles[x]);
    string sImageFile;
    FileHelpers::GetKeyStringFromINIFile("Properties",
                                         "Image",
                                         //finder.GetFileTitle(),
                                         "",
                                         sImageFile,
                                         //finder.GetFilePath());
                                         vehicles[x]);
    m_menuChooseVehicle.m_items[i].m_nAssocImage = BTextures::LoadTexture(sImageFile, false);
    ++i;
  }
  //finder.Close();
  for(i = 0; i < m_menuChooseVehicle.m_nItems; ++i) {
    m_menuChooseVehicle.m_sItems[i] = m_menuChooseVehicle.m_items[i].m_sText;
  }

  m_menuChooseVehicle.m_listMenu.SetItems(m_menuChooseVehicle.m_sItems, m_menuChooseVehicle.m_nItems);
  m_menuChooseVehicle.m_listMenu.SelectItem(m_menuChooseVehicle.m_sItems[0]);
  //m_menuChooseVehicle.m_align = BTextRenderer::TTextAlign::ALIGN_RIGHT;
  m_menuChooseVehicle.m_align = BTextRenderer::ALIGN_RIGHT;

  //******************************************
  // Choose Scene
  //******************************************

  //m_menuChooseScene.m_type = BMenu::TType::CHOOSE_SCENE;
  m_menuChooseScene.m_type = BMenu::CHOOSE_SCENE;
  m_menuChooseScene.m_nTitleWidth = 284;
  m_menuChooseScene.m_nTitleHeight = 60;
  m_menuChooseScene.m_dTitleX = 0;
  m_menuChooseScene.m_dTitleY = (512.0 - 238.0) / 512.0;

  //strWildcard = _T(".\\*.scene");
  //strWildcard = ".\\*.scene";

  // count .scene files
  int nScenes = 0;
  /*bWorking = finder.FindFile(strWildcard);
  while(bWorking) {
    bWorking = finder.FindNextFile();
    ++nScenes;
  }
  finder.Close();*/
  
	ifstream scenes_file("scenes");
	string scene;
	vector<string> scenes;
	while (!scenes_file.eof()) {
		getline(scenes_file, scene);
		if (!scene.empty() && scene[0] != '\r' && scene[0] != '\n') {
			scenes.push_back(scene);
			nScenes++;
		}
	}
	scenes_file.close();

  m_menuChooseScene.m_nItems = nScenes;
  m_menuChooseScene.m_sItems = new string[nScenes];
  m_menuChooseScene.m_items = new BMenuItem[nScenes];

  // Load .Scene files
    i = 0;
  //bWorking = finder.FindFile(strWildcard);
  //while(bWorking) {
  for (int x = 0; x < nScenes; x++) {
    //bWorking = finder.FindNextFile();
    // Add filename to object list
    //m_menuChooseScene.m_items[i].m_sAssocFile = finder.GetFilePath();
    m_menuChooseScene.m_items[i].m_sAssocFile = scenes[x];
    FileHelpers::GetKeyStringFromINIFile("Properties",
                                         "Name",
                                         //finder.GetFileTitle(),
                                         "",
                                         m_menuChooseScene.m_items[i].m_sText,
                                         //finder.GetFilePath());
                                         scenes[x]);
    string sImageFile;
    FileHelpers::GetKeyStringFromINIFile("Properties",
                                         "Image",
                                         //finder.GetFileTitle(),
                                         "",
                                         sImageFile,
                                         //finder.GetFilePath());
                                         scenes[x]);
    m_menuChooseScene.m_items[i].m_nAssocImage = BTextures::LoadTexture(sImageFile, false);
    ++i;
  }
  //finder.Close();
  for(i = 0; i < m_menuChooseScene.m_nItems; ++i) {
    m_menuChooseScene.m_sItems[i] = m_menuChooseScene.m_items[i].m_sText;
  }

  m_menuChooseScene.m_listMenu.SetItems(m_menuChooseScene.m_sItems, m_menuChooseScene.m_nItems);
  m_menuChooseScene.m_listMenu.SelectItem(m_menuChooseScene.m_sItems[0]);
  //m_menuChooseScene.m_align = BTextRenderer::TTextAlign::ALIGN_RIGHT;
  m_menuChooseScene.m_align = BTextRenderer::ALIGN_RIGHT;

  //******************************************
  // PrecachingTerrain
  //******************************************

  //m_menuPrecachingTerrain.m_type = BMenu::TType::PRECACHING_TERRAIN;
  m_menuPrecachingTerrain.m_type = BMenu::PRECACHING_TERRAIN;
  m_menuPrecachingTerrain.m_nTitleWidth = 284;
  m_menuPrecachingTerrain.m_nTitleHeight = 60;
  m_menuPrecachingTerrain.m_dTitleX = 0;
  m_menuPrecachingTerrain.m_dTitleY = (512.0 - 123.0) / 512.0;

  m_bMenusCreated = true;

  //******************************************
  // MAIN
  //******************************************

  //m_menuMain.m_type = BMenu::TType::MAIN;
  m_menuMain.m_type = BMenu::MAIN;
  m_menuMain.m_nTitleWidth = 284;
  m_menuMain.m_nTitleHeight = 80;
  m_menuMain.m_dTitleX = 0;
  m_menuMain.m_dTitleY = (512.0 - 314.0) / 512.0;

  m_menuMain.m_nItems = 5;
  m_menuMain.m_sItems = new string[m_menuMain.m_nItems];
  m_menuMain.m_items = new BMenuItem[m_menuMain.m_nItems];

  m_menuMain.m_items[0].m_sText = "START GAME";
  m_menuMain.m_items[1].m_sText = "SETTINGS";
  m_menuMain.m_items[2].m_sText = "THE SAGA OF PAKOON";
  m_menuMain.m_items[3].m_sText = "CREDITS";
  m_menuMain.m_items[4].m_sText = "EXIT";

  for(i = 0; i < m_menuMain.m_nItems; ++i) {
    m_menuMain.m_sItems[i] = m_menuMain.m_items[i].m_sText;
  }

  m_menuMain.m_listMenu.SetItems(m_menuMain.m_sItems, m_menuMain.m_nItems);
  m_menuMain.m_listMenu.SelectItem(m_menuMain.m_sItems[0]);
  //m_menuMain.m_align = BTextRenderer::TTextAlign::ALIGN_CENTER;
  m_menuMain.m_align = BTextRenderer::ALIGN_CENTER;

  //******************************************
  // Settings
  //******************************************

  //m_menuSettings.m_type = BMenu::TType::SETTINGS;
  m_menuSettings.m_type = BMenu::SETTINGS;
  m_menuSettings.m_nTitleWidth = 284;
  m_menuSettings.m_nTitleHeight = 60;
  m_menuSettings.m_dTitleX = 0;
  m_menuSettings.m_dTitleY = (512.0 - 60.0) / 512.0;

  m_menuSettings.m_nItems = 9;
  m_menuSettings.m_sItems = new string[m_menuSettings.m_nItems];
  m_menuSettings.m_items = new BMenuItem[m_menuSettings.m_nItems];

  m_menuSettings.m_items[0].m_sText = "Resolution:";
  //m_menuSettings.m_items[0].m_type = BMenuItem::TType::STRING_FROM_LIST;
  m_menuSettings.m_items[0].m_type = BMenuItem::STRING_FROM_LIST;
  m_menuSettings.m_items[1].m_sText = "Colors:";
  //m_menuSettings.m_items[1].m_type = BMenuItem::TType::STRING_FROM_LIST;
  m_menuSettings.m_items[1].m_type = BMenuItem::STRING_FROM_LIST;
  m_menuSettings.m_items[2].m_sText = "Refresh:";
  //m_menuSettings.m_items[2].m_type = BMenuItem::TType::STRING_FROM_LIST;
  m_menuSettings.m_items[2].m_type = BMenuItem::STRING_FROM_LIST;

  EnumerateScreenResolutions();

  m_menuSettings.m_items[3].m_sText = "Terrain Details:";
  //m_menuSettings.m_items[3].m_type = BMenuItem::TType::STRING_FROM_LIST;
  m_menuSettings.m_items[3].m_type = BMenuItem::STRING_FROM_LIST;
    m_menuSettings.m_items[3].m_nAssocListItems = 6;
    m_menuSettings.m_items[3].m_sAssocListItems = new string[m_menuSettings.m_items[3].m_nAssocListItems];
    m_menuSettings.m_items[3].m_sAssocListItems[0] = "Minimum";
    m_menuSettings.m_items[3].m_sAssocListItems[1] = "Low";
    m_menuSettings.m_items[3].m_sAssocListItems[2] = "Medium";
    m_menuSettings.m_items[3].m_sAssocListItems[3] = "High";
    m_menuSettings.m_items[3].m_sAssocListItems[4] = "Maximum";
    m_menuSettings.m_items[3].m_sAssocListItems[5] = "(slow machine)";
    m_menuSettings.m_items[3].m_nValue = m_nTerrainResolution;
  m_menuSettings.m_items[4].m_sText = "Dust & Clouds:";
  //m_menuSettings.m_items[4].m_type = BMenuItem::TType::STRING_FROM_LIST;
  m_menuSettings.m_items[4].m_type = BMenuItem::STRING_FROM_LIST;
    m_menuSettings.m_items[4].m_nAssocListItems = 2;
    m_menuSettings.m_items[4].m_sAssocListItems = new string[m_menuSettings.m_items[4].m_nAssocListItems];
    m_menuSettings.m_items[4].m_sAssocListItems[0] = "On";
    m_menuSettings.m_items[4].m_sAssocListItems[1] = "Off";
    m_menuSettings.m_items[4].m_nValue = m_nDustAndClouds;
  m_menuSettings.m_items[5].m_sText = "Water surface:";
  //m_menuSettings.m_items[5].m_type = BMenuItem::TType::STRING_FROM_LIST;
  m_menuSettings.m_items[5].m_type = BMenuItem::STRING_FROM_LIST;
    m_menuSettings.m_items[5].m_nAssocListItems = 3;
    m_menuSettings.m_items[5].m_sAssocListItems = new string[m_menuSettings.m_items[5].m_nAssocListItems];
    m_menuSettings.m_items[5].m_sAssocListItems[0] = "Textured";
    m_menuSettings.m_items[5].m_sAssocListItems[1] = "Transparent";
    m_menuSettings.m_items[5].m_sAssocListItems[2] = "Reflective";
    m_menuSettings.m_items[5].m_nValue = m_nWaterSurface;
  m_menuSettings.m_items[6].m_sText = "Music Volume:";
  //m_menuSettings.m_items[6].m_type = BMenuItem::TType::SLIDER;
  m_menuSettings.m_items[6].m_type = BMenuItem::SLIDER;
    m_menuSettings.m_items[6].m_nValue = m_nMusicVolume;
  m_menuSettings.m_items[7].m_sText = "Vehicle Volume:";
  //m_menuSettings.m_items[7].m_type = BMenuItem::TType::SLIDER;
  m_menuSettings.m_items[7].m_type = BMenuItem::SLIDER;
    m_menuSettings.m_items[7].m_nValue = m_nVehicleVolume;
  m_menuSettings.m_items[8].m_sText = "Soundscape:";
  //m_menuSettings.m_items[8].m_type = BMenuItem::TType::STRING_FROM_LIST;
  m_menuSettings.m_items[8].m_type = BMenuItem::STRING_FROM_LIST;
    m_menuSettings.m_items[8].m_nAssocListItems = 2;
    m_menuSettings.m_items[8].m_sAssocListItems = new string[m_menuSettings.m_items[8].m_nAssocListItems];
    m_menuSettings.m_items[8].m_sAssocListItems[0] = "3D";
    m_menuSettings.m_items[8].m_sAssocListItems[1] = "Lame";
    m_menuSettings.m_items[8].m_nValue = m_nSoundscape;

  for(i = 0; i < m_menuSettings.m_nItems; ++i) {
    m_menuSettings.m_sItems[i] = m_menuSettings.m_items[i].m_sText;
  }

  m_menuSettings.m_listMenu.SetItems(m_menuSettings.m_sItems, m_menuSettings.m_nItems);
  m_menuSettings.m_listMenu.SelectItem(m_menuSettings.m_sItems[0]);
  //m_menuSettings.m_align = BTextRenderer::TTextAlign::ALIGN_RIGHT;
  m_menuSettings.m_align = BTextRenderer::ALIGN_RIGHT;

  //******************************************
  // The Saga
  //******************************************

  //m_menuSaga.m_type = BMenu::TType::SAGA;
  m_menuSaga.m_type = BMenu::SAGA;
  m_menuSaga.m_nTitleWidth = 165;
  m_menuSaga.m_nTitleHeight = 60;
  m_menuSaga.m_dTitleX = 347.0 / 512.0;
  m_menuSaga.m_dTitleY = (512.0 - 177.0) / 512.0;

  m_menuSaga.m_nItems = 5;
  m_menuSaga.m_sItems = new string[m_menuSaga.m_nItems];
  m_menuSaga.m_items = new BMenuItem[m_menuSaga.m_nItems];

  m_menuSaga.m_items[0].m_sText = "I:    Disaster";
  m_menuSaga.m_sItems[0] = m_menuSaga.m_items[0].m_sText;
  m_menuSaga.m_items[0].m_nAssocImage = BTextures::LoadTexture("./Textures/Saga1.tga", false);

  m_menuSaga.m_items[1].m_sText = "II: Rebuilding"; 
  m_menuSaga.m_sItems[1] = m_menuSaga.m_items[1].m_sText;
  m_menuSaga.m_items[1].m_nAssocImage = BTextures::LoadTexture("./Textures/Saga2.tga", false);

  m_menuSaga.m_items[2].m_sText = "III:  Movement";
  m_menuSaga.m_sItems[2] = m_menuSaga.m_items[2].m_sText;
  m_menuSaga.m_items[2].m_nAssocImage = BTextures::LoadTexture("./Textures/Saga3.tga", false);

  m_menuSaga.m_items[3].m_sText = "IV:        Job";
  m_menuSaga.m_sItems[3] = m_menuSaga.m_items[3].m_sText;
  m_menuSaga.m_items[3].m_nAssocImage = BTextures::LoadTexture("./Textures/Saga4.tga", false);

  m_menuSaga.m_items[4].m_sText = "V:       Dream";
  m_menuSaga.m_sItems[4] = m_menuSaga.m_items[4].m_sText;
  m_menuSaga.m_items[4].m_nAssocImage = BTextures::LoadTexture("./Textures/Saga5.tga", false);

  m_menuSaga.m_listMenu.SetItems(m_menuSaga.m_sItems, m_menuSaga.m_nItems);
  m_menuSaga.m_listMenu.SelectItem(m_menuSaga.m_sItems[0]);
  //m_menuSaga.m_align = BTextRenderer::TTextAlign::ALIGN_RIGHT;
  m_menuSaga.m_align = BTextRenderer::ALIGN_RIGHT;

  //******************************************
  // Credits
  //******************************************

  //m_menuCredits.m_type = BMenu::TType::CREDITS;
  m_menuCredits.m_type = BMenu::CREDITS;
  m_menuCredits.m_nTitleWidth = 0;
  m_menuCredits.m_nTitleHeight = 0;
  m_menuCredits.m_dTitleX = 0;
  m_menuCredits.m_dTitleY = 0;
  m_menuCredits.m_bDrawLine = false;

  m_menuCredits.m_nItems = 7;
  m_menuCredits.m_sItems = new string[m_menuCredits.m_nItems];
  m_menuCredits.m_items = new BMenuItem[m_menuCredits.m_nItems];

  m_menuCredits.m_items[0].m_nValue = 0; // Second at which to start showing this item
  m_menuCredits.m_items[0].m_nValue2 = 4; // Seconds to show this item
  m_menuCredits.m_items[0].m_nAssocImage = -1; // Just a blank delay

  m_menuCredits.m_items[1].m_nValue = 4;
  m_menuCredits.m_items[1].m_nValue2 = 10; // Seconds to show this item
  m_menuCredits.m_items[1].m_nAssocImage = BTextures::LoadTexture("./Textures/CreditsProgramming.tga", false);

  m_menuCredits.m_items[2].m_nValue = 14;
  m_menuCredits.m_items[2].m_nValue2 = 10; // Seconds to show this item
  m_menuCredits.m_items[2].m_nAssocImage = BTextures::LoadTexture("./Textures/CreditsModeling.tga", false);

  m_menuCredits.m_items[3].m_nValue = 24;
  m_menuCredits.m_items[3].m_nValue2 = 15; // Seconds to show this item
  m_menuCredits.m_items[3].m_nAssocImage = BTextures::LoadTexture("./Textures/CreditsBetaTesting.tga", false);

  m_menuCredits.m_items[4].m_nValue = 39;
  m_menuCredits.m_items[4].m_nValue2 = 10; // Seconds to show this item
  m_menuCredits.m_items[4].m_nAssocImage = BTextures::LoadTexture("./Textures/CreditsSpecialThanks1.tga", false);

  m_menuCredits.m_items[5].m_nValue = 49;
  m_menuCredits.m_items[5].m_nValue2 = 7; // Seconds to show this item
  m_menuCredits.m_items[5].m_nAssocImage = BTextures::LoadTexture("./Textures/CreditsEnd.tga", false);

  m_menuCredits.m_items[6].m_nValue = 56;
  m_menuCredits.m_items[6].m_nValue2 = 1; // Seconds to show this item
  m_menuCredits.m_items[6].m_nAssocImage = -1;

  m_menuCredits.m_listMenu.SetItems(m_menuCredits.m_sItems, 0);
  //m_menuCredits.m_align = BTextRenderer::TTextAlign::ALIGN_RIGHT;
  m_menuCredits.m_align = BTextRenderer::ALIGN_RIGHT;


  m_bMenusCreated = true;
}


//*************************************************************************************************
void BGame::UpdateSettings() {
  m_menuSettings.m_items[3].m_nValue = m_nTerrainResolution;
  m_menuSettings.m_items[4].m_nValue = m_nDustAndClouds;
}




//*************************************************************************************************
void BGame::EnumerateScreenResolutions() {

  // First enumerate all to see how many there are

  int i;
  int nModes = SDL_GetNumDisplayModes(0); //FIXME always 0 display
  //BOOL bRet;
  int bRet;
  //DEVMODE devmode;
  int nSetting = 0;
  /*do {
    bRet = EnumDisplaySettings(NULL, nSetting, &devmode);
    ++nModes;
    ++nSetting;
  } while(bRet);*/

  int      nTmp;
  int      nAllRes = 0;
  string *psAllRes = new string[nModes];
  int      nAllColors = 0;
  string *psAllColors = new string[nModes];
  int      nAllRefresh = 0;
  string *psAllRefresh = new string[nModes];

  string sTmp;
  nSetting = 0;
  //do {
  for (int x = 0; x < nModes; x++) {
    //bRet = EnumDisplaySettings(NULL, nSetting, &devmode);
    SDL_DisplayMode mode;
    if (SDL_GetDisplayMode(0, x, &mode) >= 0) {
		//sTmp.Format("%ld*%ld PIXELS", devmode.dmPelsWidth, devmode.dmPelsHeight);
		stringstream format;
		format << mode.w << "*" << mode.h << " PIXELS";
		sTmp = format.str();
		if(!FindStringFromArray(sTmp, psAllRes, nAllRes, nTmp)) {
		  psAllRes[nAllRes] = sTmp;
		  ++nAllRes;
		}
		//sTmp.Format("%ld BITS", devmode.dmBitsPerPel);
		format.str("");
		format << SDL_BITSPERPIXEL(mode.format) << " BITS";
		sTmp = format.str();
		if(!FindStringFromArray(sTmp, psAllColors, nAllColors, nTmp)) {
		  psAllColors[nAllColors] = sTmp;
		  ++nAllColors;
		}
		//sTmp.Format("%ld HERTZ", devmode.dmDisplayFrequency);
		format.str("");
		format << mode.refresh_rate << " HERTZ";
		sTmp = format.str();
		if(!FindStringFromArray(sTmp, psAllRefresh, nAllRefresh, nTmp)) {
		  psAllRefresh[nAllRefresh] = sTmp;
		  ++nAllRefresh;
		}
		++nSetting;
	}
  //} while(bRet);
	}

  m_menuSettings.m_items[0].m_nAssocListItems = nAllRes;
  m_menuSettings.m_items[0].m_sAssocListItems = new string[nAllRes];
  for(i = 0; i < nAllRes; ++i) {
    m_menuSettings.m_items[0].m_sAssocListItems[i] = psAllRes[i];
  }

  m_menuSettings.m_items[1].m_nAssocListItems = nAllColors;
  m_menuSettings.m_items[1].m_sAssocListItems = new string[nAllColors];
  for(i = 0; i < nAllColors; ++i) {
    m_menuSettings.m_items[1].m_sAssocListItems[i] = psAllColors[i];
  }

  m_menuSettings.m_items[2].m_nAssocListItems = nAllRefresh;
  m_menuSettings.m_items[2].m_sAssocListItems = new string[nAllRefresh];
  for(i = 0; i < nAllRefresh; ++i) {
    m_menuSettings.m_items[2].m_sAssocListItems[i] = psAllRefresh[i];
  }

  // Preselect current mode
  //sTmp.Format("%d*%d PIXELS", m_nDispWidth, m_nDispHeight);
	stringstream format;
	format << m_nDispWidth << "*" << m_nDispHeight << " PIXELS";
	sTmp = format.str();
  //(void) FindStringFromArray(sTmp, psAllRes, nAllRes, m_menuSettings.m_items[0].m_nValue)
  if (!FindStringFromArray(sTmp, psAllRes, nAllRes, m_menuSettings.m_items[0].m_nValue)) {
	  m_menuSettings.m_items[0].m_nValue = 0;
  }
  //sTmp.Format("%d BITS", m_nDispBits);
	format.str("");
	format << m_nDispBits << " BITS";
	sTmp = format.str();
  //(void) FindStringFromArray(sTmp, psAllColors, nAllColors, m_menuSettings.m_items[1].m_nValue);
  if (!FindStringFromArray(sTmp, psAllColors, nAllColors, m_menuSettings.m_items[1].m_nValue)) {
	  m_menuSettings.m_items[1].m_nValue = 0;
  }
  m_menuSettings.m_items[1].m_nValue = 0;
  //sTmp.Format("%d HERTZ", m_nDispHz);
	format.str("");
	format << m_nDispHz << " HERTZ";
	sTmp = format.str();
  //(void) FindStringFromArray(sTmp, psAllRefresh, nAllRefresh, m_menuSettings.m_items[2].m_nValue);
  if (!FindStringFromArray(sTmp, psAllRefresh, nAllRefresh, m_menuSettings.m_items[2].m_nValue)) {
	  m_menuSettings.m_items[2].m_nValue = 0;
  }

  delete [] psAllRes;
  delete [] psAllColors;
  delete [] psAllRefresh;
}


//*************************************************************************************************
bool BGame::FindStringFromArray(string s, string *psArray, int nItems, int &rnIndex) {
  rnIndex = -1;
  for(int i = 0; i < nItems; ++i) {
    //if(s.CompareNoCase(psArray[i]) == 0) {
    if(s.compare(psArray[i]) == 0) {
      rnIndex = i;
      return true;
    }
  }
  return false;
}



//*************************************************************************************************
void BGame::FreezeSimulation(bool bPause) {
  if(m_nFreezeRefCount == 0) {
    SoundModule::StopEngineSound();
    m_nPhysicsSteps = m_simulation.m_nPhysicsStepsBetweenRender;
    m_clockFrozenStart = SDL_GetTicks();
    m_bFrozen = true;
    if(bPause) {
      GetSimulation()->m_bPaused = true;
    }
  }
  ++m_nFreezeRefCount;
}

//*************************************************************************************************
unsigned BGame::ContinueSimulation() {
  if(m_nFreezeRefCount > 0) {
    --m_nFreezeRefCount;
    if((m_nFreezeRefCount == 0) && GetSimulation()->m_bPaused) {
      SoundModule::StartEngineSound();
      SoundModule::SetEngineSoundVolume(160.0);
      GetSimulation()->m_bPaused = false;
      m_simulation.m_nPhysicsStepsBetweenRender = m_nPhysicsSteps;
      m_bFrozen = false;
      return SDL_GetTicks() - m_clockFrozenStart;
    }
    if(m_nFreezeRefCount == 0) {
      m_simulation.m_nPhysicsStepsBetweenRender = m_nPhysicsSteps;
      SoundModule::StartEngineSound();
      SoundModule::SetEngineSoundVolume(255.0);
      m_bFrozen = false;
      return SDL_GetTicks() - m_clockFrozenStart;
    }
  }
  return 0;
}


//*************************************************************************************************
void BGame::SetupScene() {
  // Find base and clear any client tracking targets

  GetSimulation()->RemoveTrackingTarget("CLIENT");
  GetSimulation()->RemoveTrackingTarget("BASE");

  BScene *pScene = GetSimulation()->GetScene();
  for(int i = 0; i < pScene->m_nObjects; ++i) {
    //if(pScene->m_pObjects[i].m_type == BObject::TType::BASE) {
    if(pScene->m_pObjects[i].m_type == BObject::BASE) {
      SetBase(&(pScene->m_pObjects[i]));
      GetSimulation()->AddTrackingTarget("BASE", pScene->m_pObjects[i].m_vCenter, 0, 0, 1); // Blue base
    }
  }
}


//*************************************************************************************************
void BGame::SetCurrentClient(BObject *pClient) {
  m_pClient = pClient;
  GetSimulation()->RemoveTrackingTarget("CLIENT");
  GetSimulation()->AddTrackingTarget("CLIENT", m_pClient->m_vCenter, 0, 0.5, 0);         // Green Client
}



//*************************************************************************************************
double BGame::GetMaxProfitFor(double dDist, double dPizzaTemp) { 
  double dProfit = (dPizzaTemp / 60.0) * (dDist / 800.0) * 30.0;
  for(int i = 0; i < m_nPizzaDamaged; ++i) {
    dProfit *= 0.9;
  }
  return dProfit;
}


extern double Random(double dRange);

//*************************************************************************************************
void BGame::UpdateEarthquake() {
  // Check what needs to be done

  if(m_bHasEarthquakes) {
    unsigned clockNow = SDL_GetTicks();
    if(m_bEarthquakeActive) {
      // See if we need exit earthquake
      if(clockNow > m_bEarthquakeWillEnd) {
        BMessages::Remove("earthquake");
        m_bEarthquakeFactor = 0.0;
        m_bEarthquakeActive = false;
        // schedule next earthquake
        m_bEarthquakeNextStart = clockNow + 1000 * (60.0 + Random(120.0));
      } else {
        // Update factor
        m_bEarthquakeFactor = 0.0;
        if((clockNow - m_bEarthquakeStarted) < 1000) {
          m_bEarthquakeFactor = double(clockNow - m_bEarthquakeStarted) / 1000.0;
        } else if((m_bEarthquakeWillEnd - clockNow) < 1000) {
          m_bEarthquakeFactor = double(m_bEarthquakeWillEnd - clockNow) / 1000.0;
        } else {
          m_bEarthquakeFactor = 1.0;
        }
      }
    } else {
      // See if earthquake should be started
      if((clockNow >= m_bEarthquakeNextStart) || (m_bEarthquakeNextStart == 0)) {
        m_bEarthquakeWillEnd = clockNow + 1000 * (15.0 + Random(60.0));
        m_bEarthquakeActive = true;
        BMessages::Show(50, "earthquake", "(Earthquake warning)", 999, false, 1, 0.3, 0.3, false, true);
        m_bEarthquakeFactor = 0.0;
      }
    }
  } else {
    m_bEarthquakeActive = false;
  }
}






//*************************************************************************************************
void BGame::UpdateWindParticles(bool bInit) {
  static BVector vUp(0, 0, -1);

  double dTmp;
  m_dWindStrength = m_dBaseWindStrength + HeightMap::CalcHeightAt(double(SDL_GetTicks() / 10000.0), 137.0, dTmp, HeightMap::NOISE) * m_dBaseWindStrength * 0.1;

  BCamera *pCamera = GetSimulation()->GetCamera();
  BVector vCamSpeed = pCamera->m_vSpeed * (1.0 / GetSimulation()->m_nPhysicsStepsBetweenRender);
  BVector vOptimalStart = vCamSpeed - m_vWindDirection * m_dWindStrength;
  vOptimalStart.ToUnitLength();
  BVector vRight = vOptimalStart.CrossProduct(vUp);
  vOptimalStart = vOptimalStart * 125.0;
  if(bInit) {
    for(int i = 0; i < g_cnWindParticles; ++i) {
      // Create new particle
      m_vWindParticles[i] = pCamera->m_vLocation + 
                            BVector(1.0, 0, 0) * (Random(250.0) - 125.0) + 
                            BVector(0, 1.0, 0) * (Random(250.0) - 125.0) + 
                            BVector(0, 0, 1.0) * (Random(250.0) - 125.0) * 0.1;
    }
  } else {
    if(!m_bFrozen) {
      double dMaxRad = sqrt(3 * 125.0 * 125.0);
      for(int i = 0; i < g_cnWindParticles; ++i) {
        if((m_vWindParticles[i] - pCamera->m_vLocation).Length() > dMaxRad) {
          // Create new particle
          m_vWindParticles[i] = pCamera->m_vLocation + 
                                vOptimalStart + 
                                vRight * (Random(250.0) - 125.0) + 
                                vUp    * (Random(250.0) - 125.0) * 0.4;
        } else {
          // Move particle  
          m_vWindParticles[i] = m_vWindParticles[i] + m_vWindDirection * m_dWindStrength * GetSimulation()->m_nPhysicsStepsBetweenRender;
        }
      }
    }
  }
}

extern double g_dRate;
extern double g_d10LastFPS[];

//*************************************************************************************************
void BGame::UpdateAnalyzer() {
  if(!m_bAnalyzerMode) {
    return;
  }

  // See in which phase we are in
  //static nPhase = -1;
  /*static int nPhase = -1;
  unsigned clockNow = SDL_GetTicks();

  int nNewPhase = (clockNow - m_clockAnalyzerStarted) / 1000.0 / 3;
  if(nPhase != nNewPhase) {
    // Exit old phase
    if(nPhase != -1) {
      string sInfo;
      MyAfxMessageBox("---------------------------------------------------------");
      string sVis;
      switch(nPhase) {
        case 0: sVis = "SKY"; break;
        case 1: sVis = "WATER"; break;
        case 2: sVis = "TERRAIN"; break;
        case 3: sVis = "VEHICLE"; break;
        case 4: sVis = "GASSTATIONS"; break;
        case 5: sVis = "OBJECTS"; break;
        case 6: sVis = "DUSTANDCLOUDS"; break;
        case 7: sVis = "GRAPHICS2D"; break;
        case 8: sVis = "ALL & ~SKY"; break;
        case 9: sVis = "ALL & ~WATER"; break;
        case 10: sVis = "ALL & ~TERRAIN"; break;
        case 11: sVis = "ALL & ~VEHICLE"; break;
        case 12: sVis = "ALL & ~GASSTATIONS"; break;
        case 13: sVis = "ALL & ~OBJECTS"; break;
        case 14: sVis = "ALL & ~DUSTANDCLOUDS"; break;
        case 15: sVis = "ALL & ~GRAPHICS2D"; break;
        case 16: sVis = "WATER (2)"; break;
        case 17: sVis = "WATER (1)"; break;
        case 18: sVis = "WATER (0)"; break;
        case 19: sVis = "ALL (water 0)"; break;
        case 20: sVis = "ALL (water 2)"; break;
      }
      if(nPhase == 7) {
        sVis = "GRAPHICS2D";
      }
      sInfo.Format("ANALYZER: Phase %d Info (Visualize = %s)", nPhase, sVis);
      MyAfxMessageBox(sInfo);
      sInfo.Format("FPS: AVE=%.2lf, Last10=%.1lf %.1lf %.1lf %.1lf %.1lf %.1lf %.1lf %.1lf %.1lf %.1lf ", 
                    g_dRate,
                    g_d10LastFPS[0],
                    g_d10LastFPS[1],
                    g_d10LastFPS[2],
                    g_d10LastFPS[3],
                    g_d10LastFPS[4],
                    g_d10LastFPS[5],
                    g_d10LastFPS[6],
                    g_d10LastFPS[7],
                    g_d10LastFPS[8],
                    g_d10LastFPS[9]);
      BGame::MyAfxMessageBox(sInfo);
      MyAfxMessageBox("---------------------------------------------------------");
    }
    // Enter new phase
    nPhase = nNewPhase;
    switch(nPhase) {
      case 0:
        m_nVisualize = SKY;
        m_nWaterSurface = 0;
        break;
      case 1:
        m_nVisualize = WATER;
        break;
      case 2:
        m_nVisualize = TERRAIN;
        break;
      case 3:
        m_nVisualize = VEHICLE;
        break;
      case 4:
        m_nVisualize = GASSTATIONS;
        break;
      case 5:
        m_nVisualize = OBJECTS;
        break;
      case 6:
        m_nVisualize = DUSTANDCLOUDS;
        break;
      case 7:
        m_nVisualize = GRAPHICS2D;
        break;
      case 8:
        m_nVisualize = ALL & ~SKY;
        break;
      case 9:
        m_nVisualize = ALL & ~WATER;
        break;
      case 10:
        m_nVisualize = ALL & ~TERRAIN;
        break;
      case 11:
        m_nVisualize = ALL & ~VEHICLE;
        break;
      case 12:
        m_nVisualize = ALL & ~GASSTATIONS;
        break;
      case 13:
        m_nVisualize = ALL & ~OBJECTS;
        break;
      case 14:
        m_nVisualize = ALL & ~DUSTANDCLOUDS;
        break;
      case 15:
        m_nVisualize = ALL & ~GRAPHICS2D;
        break;
      case 16:
        m_nVisualize = WATER;
        m_nWaterSurface = 2;
        break;
      case 17:
        m_nVisualize = WATER;
        m_nWaterSurface = 1;
        break;
      case 18:
        m_nVisualize = WATER;
        m_nWaterSurface = 0;
        break;
      case 19:
        m_nVisualize = ALL;
        break;
      case 20:
        m_nVisualize = ALL;
        m_nWaterSurface = 2;
        break;
      case 21:
        // End analyzer
        nPhase = -1;
        m_bAnalyzerMode = false;
        BGame::MyAfxMessageBox("--------------------------");
        BGame::MyAfxMessageBox("ANALYZER ENDED!");
        BGame::MyAfxMessageBox("--------------------------");
        GetView()->m_nMenuTime += ContinueSimulation();
        BMessages::Show(40, "analyzer", "ANALYZING COMPLETE!", 2);
        break;
    }
  }*/
}





//*************************************************************************************************
string BGame::GetScrambleChecksum() {
  static string sSeed = "Settings, Main, Medium, TerrainResolution";
  string sRet = sSeed;
  string sState;
  /*sState.Format("%.1lf:%.1lf:%.1lf:%s:%s", 
                m_player.m_dCash, 
                m_player.m_dFuel, 
                m_player.m_dKerosine,
                m_player.m_sValidVehicles,
                m_player.m_sSceneInfo);*/
    stringstream format;
    format << m_player.m_dCash << ":" << m_player.m_dFuel << ":" << m_player.m_dKerosine << ":" << m_player.m_sValidVehicles << ":" << m_player.m_sSceneInfo;
    sState = format.str();

  // calculate checksum string
  int i;

  int nStart = 0;
  //int nEnd = sRet.GetLength() - 1;
  int nEnd = sRet.length() - 1;
  int nStep = 1;
  int nCounter = 0;

  //for(i = 0; i < sState.GetLength(); ++i) {
  for(i = 0; i < sState.length(); ++i) {
    nCounter = 0;

    if(nStep > 0) {
      nStart = 0;
      //nEnd = sRet.GetLength() - 1;
      nEnd = sRet.length() - 1;
    } else {
      //nStart = sRet.GetLength() - 1;
      nStart = sRet.length() - 1;
      nEnd = 0;
    }

    unsigned char c = (unsigned char) sState[i];
    for(int j = nStart; j != nEnd; j += nStep) {
      unsigned char cOld = (unsigned char) sRet[j];
      unsigned char cNew = ((cOld ^ ~c) + c * nCounter) % 256;
      //sRet.SetAt(j, cNew);
      sRet[j] = cNew;
      ++nCounter;
    }
    nStep *= -1;
  }

  // map to readable characters
  //for(i = 0; i < sRet.GetLength(); ++i) {
  for(i = 0; i < sRet.length(); ++i) {
    unsigned char c = (unsigned char) sRet[i];
    if(c < 10) {
      //sRet.SetAt(i, TCHAR(((unsigned char) (sRet[i]) % ('9' - '0')) + '0'));
      sRet[i] = ((unsigned char) (sRet[i]) % ('9' - '0')) + '0';
    } else if(c < 128) {
      //sRet.SetAt(i, TCHAR(((unsigned char) (sRet[i]) % ('z' - 'a')) + 'a'));
      sRet[i] = ((unsigned char) (sRet[i]) % ('z' - 'a')) + 'a';
    } else {
      //sRet.SetAt(i, TCHAR(((unsigned char) (sRet[i]) % ('Z' - 'A')) + 'A'));
      sRet[i] = ((unsigned char) (sRet[i]) % ('Z' - 'A')) + 'A';
    }
  }

  return sRet;
}





//*************************************************************************************************
void BGame::MyAfxMessageBox(string sText, int nTmp) {

  time_t ltime;
  time(&ltime);
  struct tm *newtime;
  newtime = localtime(&ltime);

  FILE *fp = fopen("Pakoon1.log", "a");
  //fprintf(fp, "%.24s: %s", asctime(newtime), LPCTSTR(sText));
  fprintf(fp, "%.24s: %s", asctime(newtime), sText.c_str());
  fprintf(fp, "\n");
  fclose(fp);
  
  cout << sText << endl;
}




// PAKOON! Game, Source Code and Developer Package Copyright
// =========================================================
// 
// Restrictions related to PAKOON! Game and it's use
// -------------------------------------------------
// 
// You may download and play the PAKOON! game for free. You may also copy it freely to your friends and relatives as long as you 
// provide the original setup package (downloaded from www.nic.fi/~moxide) and the copyright included in it is also given. You 
// may also use the PAKOON! game for educational purposes, as long as the origin of the PAKOON! game (i.e. www.nic.fi/~moxide) 
// is mentioned and this copyright is also provided and the creator of the game (i.e. Mikko Oksalahti, email: 
// mikko.oksalahti@nic.fi) is notified of the use in advance.
// You may not sell or otherwise accept any payment for giving or offering the game to someone else. You may not offer the 
// PAKOON! game for free on any webpage, CD, DVD or other media without a written permission from the creator of the PAKOON! 
// game (i.e. Mikko Oksalahti, email: mikko.oksalahti@nic.fi).
// You may freely include a link to PAKOON! homepage (i.e. www.nic.fi/~moxide) from your own site.
// 
// 
// Restrictions related to PAKOON! Game
// Source Code and Developer Package and their use
// -----------------------------------------------
// 
// You may download and use the PAKOON! game source code for personal use. You may not use any part of the source code or the 
// developer package on any commercial or free game or other computer program intended for public distribution without a written 
// permission from the creator of the PAKOON! game (i.e. Mikko Oksalahti, email: mikko.oksalahti@nic.fi). You may use the 
// PAKOON! game source code or developer package for educational purposes, as long as the origin of the PAKOON! game (i.e. 
// www.nic.fi/~moxide) is mentioned and this copyright is also provided and the creator of the game (i.e. Mikko Oksalahti, 
// email: mikko.oksalahti@nic.fi) is notified of the use in advance.
// You may not sell or otherwise accept any payment for giving or offering the PAKOON! game source code or developer package to 
// someone else. You may not offer the PAKOON! game source code or developer package for free on any webpage, CD, DVD or other 
// media without a written permission from the creator of the PAKOON! game (i.e. Mikko Oksalahti, email: 
// mikko.oksalahti@nic.fi).
// 
// 
// Mikko Oksalahti
// Helsinki, FINLAND
// 7.10.2002
