// Pakoon1View.cpp : implementation of the CPakoon1View class
//
// (c) Copyright 2002, Mikko Oksalahti (see end of file for details)
//

#define OEMRESOURCE
//#include "stdafx.h"
//#include "Pakoon1.h"

#include <cstdlib>
#include <iostream>
#include <sstream>

#include <SDL2/SDL.h>

using namespace std;

#include "Pakoon1Doc.h"
#include "Pakoon1View.h"
//#include "Dialogs.h"
//#include "DlgControls.h"
//#include "DlgSounds.h"

#include "OpenGLHelpers.h"
#include "OpenGLExtFunctions.h"
#include "BTextures.h"
#include "GL/gl.h"
#include "GL/glu.h"

#include "SoundModule.h"
#include "Settings.h"
#include "BNavSatWnd.h"
#include "BServiceWnd.h"
#include "BMessages.h"
#include "HeightMap.h"
#include "BTextRenderer.h"
#include "BUI.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern bool g_cbBlackAndWhite;
extern double Random(double dRange);

double g_dPhysicsStepsInSecond = 200.0; // Was 200.0
bool g_bControl = false;
bool g_bShift = false;


/////////////////////////////////////////////////////////////////////////////
// CPakoon1View

//*************************************************************************************************
//IMPLEMENT_DYNCREATE(CPakoon1View, CView)

//BEGIN_MESSAGE_MAP(CPakoon1View, CView)
//{{AFX_MSG_MAP(CPakoon1View)
/*ON_WM_CREATE()
ON_WM_ERASEBKGND()
ON_WM_DESTROY()
ON_WM_CHAR()
ON_WM_KEYDOWN()
ON_WM_KEYUP()
ON_WM_MOUSEMOVE()
ON_WM_LBUTTONDOWN()
ON_WM_LBUTTONUP()*/
//}}AFX_MSG_MAP
//END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPakoon1View construction/destruction

//*************************************************************************************************
CPakoon1View::CPakoon1View() {
  m_nMenuTime = 0;
  m_pDrawFunction = &CPakoon1View::OnDrawCurrentMenu;
  m_pKeyDownFunction = &CPakoon1View::OnKeyDownCurrentMenu;
  m_bDrawOnlyMenu = false;
  m_bFullRedraw = true;
  //m_hCursor = 0;
  SoundModule::Initialize();
  m_bInitClock = true;
  m_bCreateDLs = false;
  m_bWireframe = false;
  m_bNormals = false;
  m_bIgnoreNextChar = false;
  m_clockMenuScroll = 0;
  //m_pThreadLoading = 0;

  m_bShowVideo = false;
  
  //omat
  exit = false;
  mouse_x = 0;
  mouse_y = 0;
}

CPakoon1View::~CPakoon1View() {
}


//*************************************************************************************************
//BOOL CPakoon1View::PreCreateWindow(CREATESTRUCT& cs) {
bool CPakoon1View::PreCreateWindow() {
  // TODO: Modify the Window class or styles here by modifying
  //  the CREATESTRUCT cs
  //return CView::PreCreateWindow(cs);
  return false;
}

//DEVMODE g_devmodeOrig;


//*************************************************************************************************
//int CPakoon1View::OnCreate(LPCREATESTRUCT lpCreateStruct) {
int CPakoon1View::OnCreate() {

  // Save current screen resolution
  //FIXME
  /*EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &g_devmodeOrig);

  BOOL retval = CView::OnCreate(lpCreateStruct);

  HWND hWnd = GetSafeHwnd();
  HDC hDC = ::GetDC(hWnd);
  PIXELFORMATDESCRIPTOR pfd = {
  sizeof(PIXELFORMATDESCRIPTOR), // size of this pfd
  1, // version number
  PFD_DRAW_TO_WINDOW |    // support window
    PFD_SUPPORT_OPENGL |  // support OpenGL
    // PFD_SUPPORT_GDI |     // support GDI
    PFD_DOUBLEBUFFER |    // double buffered
    PFD_NEED_SYSTEM_PALETTE, 
  PFD_TYPE_RGBA, // RGBA type
  32, // 32-bit color depth
  0, 0, 0, 0, 0, 0, // color bits ignored
  0, // no alpha buffer
  0, // shift bit ignored
  0, // no accumulation buffer
  0, 0, 0, 0, // accum bits ignored
  32, // 32-bit z-buffer 
  8, // 8-bit stencil buffer for reflections
  0, // no auxiliary buffer
  PFD_MAIN_PLANE, // main layer
  0, // reserved
  0, 0, 0 // layer masks ignored
  };
  int pixelformat = ChoosePixelFormat(hDC, &pfd);
  TRACE("Pixelformat %d\n", pixelformat);
  if(SetPixelFormat(hDC, pixelformat, &pfd) == FALSE) {
    BGame::MyAfxMessageBox("SetPixelFormat failed");
  }  
  m_hGLRC = wglCreateContext(hDC);
  wglMakeCurrent(hDC, m_hGLRC);*/

  // Check Player.State file integrity
  BGame::GetPlayer()->LoadStateFile();
  if((BGame::GetPlayer()->m_dCash <= 0.01) && (BGame::GetPlayer()->m_dFuel <= 0.01)) {
    // Assist player
    BGame::MyAfxMessageBox("Assisting player with 50 units of cash, 1/4 tank of fuel.");
    BGame::GetPlayer()->m_dCash = 50.0;
    BGame::GetPlayer()->m_dFuel = 25.0;
    BGame::GetPlayer()->SaveStateFile();
  }
  if((BGame::GetPlayer()->m_dCash > 0.0) && (BGame::GetPlayer()->m_dFuel <= 0.01)) {
    // Assist player
    BGame::MyAfxMessageBox("Assisting player with 1/4 tank of fuel.");
    BGame::GetPlayer()->m_dCash -= 25.0;
    BGame::GetPlayer()->m_dFuel = 25.0;
    BGame::GetPlayer()->SaveStateFile();
  }

  // Set view for BGame
  BGame::SetView(this);

  // Read settings for graphics etc.
  Settings::ReadSettings(m_game.GetSimulation());
  switch(BGame::m_nTerrainResolution) {
    case 0: m_game.GetSimulation()->GetTerrain()->SetRenderResolution(BTerrain::RENDER_MINIMUM); break;
    case 1: m_game.GetSimulation()->GetTerrain()->SetRenderResolution(BTerrain::RENDER_LOW); break;
    case 2: m_game.GetSimulation()->GetTerrain()->SetRenderResolution(BTerrain::RENDER_MEDIUM); break;
    case 3: m_game.GetSimulation()->GetTerrain()->SetRenderResolution(BTerrain::RENDER_HIGH); break;
    case 4: m_game.GetSimulation()->GetTerrain()->SetRenderResolution(BTerrain::RENDER_MAXIMUM); break;
    case 5: m_game.GetSimulation()->GetTerrain()->SetRenderResolution(BTerrain::RENDER_SLOW_MACHINE); break;
  }
  BGame::m_bShowDust = (BGame::m_nDustAndClouds == 0);

  // Initialize common OpenGL features
  InitializeOpenGL();

  // Initialize multitexturing support
  OpenGLHelpers::Init();
  BTextures::Init();

  m_game.GetSimulation()->GetGround()->LoadTextures();
  m_game.GetSimulation()->GetGround()->PreProcessVisualization();
  m_game.GetSimulation()->GetTerrain()->PreProcessVisualization();
  m_game.GetSimulation()->PreProcessVisualization();
  BTerrain::GetGasStationObject()->PreProcessVisualizationGasStation();
  

  // Check for multiprosessor support
  //FIXME
  /*SYSTEM_INFO si;
  GetSystemInfo(&si);
  BGame::m_bMultiProcessor = si.dwNumberOfProcessors > 1;*/

  // Change cursor to Pakoon! cursor
  //FIXME
  /*m_hCursor = AfxGetApp()->LoadCursor(IDC_POINTER);
  if(m_hCursor) {
    ::SetClassLong(GetSafeHwnd(), GCL_HCURSOR, 0L);
    ::SetCursor(m_hCursor);
  }*/

  //::ReleaseDC(GetSafeHwnd(), hDC);

  BGame::MyAfxMessageBox("----------------------");
  BGame::MyAfxMessageBox("- OpenGL Info        -");
  BGame::MyAfxMessageBox("----------------------");
  string sInfo, sInfo2;
  sInfo2 = (char *) glGetString(GL_VENDOR);
  sInfo = "Vendor: " + sInfo2;
  BGame::MyAfxMessageBox(sInfo);
  sInfo2 = (char *) glGetString(GL_RENDERER);
  sInfo = "Renderer: " + sInfo2;
  BGame::MyAfxMessageBox(sInfo);
  sInfo2 = (char *) glGetString(GL_VERSION);
  sInfo = "Version: " + sInfo2;
  BGame::MyAfxMessageBox(sInfo);
  sInfo2 = (char *) glGetString(GL_EXTENSIONS);
  sInfo = "Extensions: " + sInfo2;
  BGame::MyAfxMessageBox(sInfo);
  BGame::MyAfxMessageBox("----------------------");

  // Play with AVI videos 
  // InitVideo();
  
  //omat
  BGame::m_nDispWidth = window_width;
  BGame::m_nDispHeight = window_height;

  //return retval;
  return true;
}

// BEGIN_VIDEO
//#include <vfw.h>												// Header File For Video For Windows
//#pragma comment( lib, "vfw32.lib" )								// Search For VFW32.lib While Linking

//FIXME
/*AVISTREAMINFO		psi;										// Pointer To A Structure Containing Stream Info
PAVISTREAM			pavi;										// Handle To An Open Stream
PGETFRAME			pgf;										// Pointer To A GetFrame Object
BITMAPINFOHEADER	bmih;										// Header Information For DrawDibDraw Decoding
long				lastframe;									// Last Frame Of The Stream
int					width;										// Video Width
int					height;										// Video Height
char				*pdata;										// Pointer To Texture Data
int					mpf;										// Will Hold Rough Milliseconds Per Frame

HDRAWDIB hdd;													// Handle For Our Dib
HBITMAP hBitmap;												// Handle To A Device Dependant Bitmap
HDC hdc = CreateCompatibleDC(0);								// Creates A Compatible Device Context
unsigned char* data = 0;										// Pointer To Our Resized Image


void flipIt(void* buffer)										// Flips The Red And Blue Bytes (256x256)
{
	void* b = buffer;											// Pointer To The Buffer
	__asm														// Assembler Code To Follow
	{
		mov ecx, 256*256										// Counter Set To Dimensions Of Our Memory Block
		mov ebx, b												// Points ebx To Our Data (b)
		label:													// Label Used For Looping
			mov al,[ebx+0]										// Loads Value At ebx Into al
			mov ah,[ebx+2]										// Loads Value At ebx+2 Into ah
			mov [ebx+2],al										// Stores Value In al At ebx+2
			mov [ebx+0],ah										// Stores Value In ah At ebx
			
			add ebx,3											// Moves Through The Data By 3 Bytes
			dec ecx												// Decreases Our Loop Counter
			jnz label											// If Not Zero Jump Back To Label
	}
}*/

//static int g_nFramesInVideo = 0;

/*void OpenAVI(LPCSTR szFile)										// Opens An AVI File (szFile)
{
	TCHAR	title[100];											// Will Hold The Modified Window Title

	AVIFileInit();												// Opens The AVIFile Library

	// Opens The AVI Stream
	if (AVIStreamOpenFromFile(&pavi, szFile, streamtypeVIDEO, 0, OF_READ, NULL) !=0)
	{
		// An Error Occurred Opening The Stream
		MessageBox (HWND_DESKTOP, "Failed To Open The AVI Stream", "Error", MB_OK | MB_ICONEXCLAMATION);
    return;
	}

	AVIStreamInfo(pavi, &psi, sizeof(psi));						// Reads Information About The Stream Into psi
	width=psi.rcFrame.right-psi.rcFrame.left;					// Width Is Right Side Of Frame Minus Left
	height=psi.rcFrame.bottom-psi.rcFrame.top;					// Height Is Bottom Of Frame Minus Top

	lastframe=AVIStreamLength(pavi);							// The Last Frame Of The Stream
  g_nFramesInVideo = lastframe;

	mpf=AVIStreamSampleToTime(pavi,lastframe)/lastframe;		// Calculate Rough Milliseconds Per Frame

	bmih.biSize = sizeof (BITMAPINFOHEADER);					// Size Of The BitmapInfoHeader
	bmih.biPlanes = 1;											// Bitplanes	
	bmih.biBitCount = 24;										// Bits Format We Want (24 Bit, 3 Bytes)
	bmih.biWidth = 256;											// Width We Want (256 Pixels)
	bmih.biHeight = 256;										// Height We Want (256 Pixels)
	bmih.biCompression = BI_RGB;								// Requested Mode = RGB

	hBitmap = CreateDIBSection (hdc, (BITMAPINFO*)(&bmih), DIB_RGB_COLORS, (void**)(&data), NULL, NULL);
	SelectObject (hdc, hBitmap);								// Select hBitmap Into Our Device Context (hdc)

	pgf=AVIStreamGetFrameOpen(pavi, NULL);						// Create The PGETFRAME	Using Our Request Mode
	if (pgf==NULL)
	{
		// An Error Occurred Opening The Frame
		MessageBox (HWND_DESKTOP, "Failed To Open The AVI Frame", "Error", MB_OK | MB_ICONEXCLAMATION);
	}
}

void GrabAVIFrame(int frame)									// Grabs A Frame From The Stream
{
	LPBITMAPINFOHEADER lpbi;									// Holds The Bitmap Header Information
	lpbi = (LPBITMAPINFOHEADER)AVIStreamGetFrame(pgf, frame);	// Grab Data From The AVI Stream
	pdata=(char *)lpbi+lpbi->biSize+lpbi->biClrUsed * sizeof(RGBQUAD);	// Pointer To Data Returned By AVIStreamGetFrame

	// Convert Data To Requested Bitmap Format
	DrawDibDraw (hdd, hdc, 0, 0, 256, 256, lpbi, pdata, 0, 0, width, height, 0);

	flipIt(data);												// Swap The Red And Blue Bytes (GL Compatability)

	// Update The Texture
	glTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0, 256, 256, GL_RGB, GL_UNSIGNED_BYTE, data);
}

void CloseAVI(void)												// Properly Closes The Avi File
{
	DeleteObject(hBitmap);										// Delete The Device Dependant Bitmap Object
	DrawDibClose(hdd);											// Closes The DrawDib Device Context
	AVIStreamGetFrameClose(pgf);								// Deallocates The GetFrame Resources
	AVIStreamRelease(pavi);										// Release The Stream
	AVIFileExit();												// Release The File
}*/

static int g_nVideoTexture = 0;

//*************************************************************************************************
//FIXME
/*void CPakoon1View::InitVideo() {
  hdd = DrawDibOpen();										// Grab A Device Context For Our Dib
  OpenAVI(".\\Test3.AVI");
  g_nVideoTexture = BTextures::LoadTexture(".\\Textures\\Test256x256.tga", false);
}*/
// END_VIDEO

//*************************************************************************************************
//BOOL CPakoon1View::OnEraseBkgnd(CDC* pDC) {
bool CPakoon1View::OnEraseBkgnd() {
  //return TRUE;
  return true;
}

//*************************************************************************************************
void CPakoon1View::OnDestroy() {
  // Return to original display settings

  // CloseAVI();

	//FIXME
  /*DEVMODE devmode;
  EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devmode);
  if((devmode.dmPelsWidth  != g_devmodeOrig.dmPelsWidth) || 
     (devmode.dmPelsHeight != g_devmodeOrig.dmPelsHeight) || 
     (devmode.dmBitsPerPel != g_devmodeOrig.dmBitsPerPel) ||
     (devmode.dmDisplayFrequency != g_devmodeOrig.dmDisplayFrequency)) {
    devmode.dmPelsWidth = g_devmodeOrig.dmPelsWidth;
    devmode.dmPelsHeight = g_devmodeOrig.dmPelsHeight;
    devmode.dmBitsPerPel = g_devmodeOrig.dmBitsPerPel;
    devmode.dmDisplayFrequency = g_devmodeOrig.dmDisplayFrequency;
    ChangeDisplaySettings(&devmode, 0);
  }

  HWND hWnd = GetSafeHwnd();
  HDC hDCWnd = ::GetDC(hWnd);
  wglMakeCurrent(hDCWnd, m_hGLRC);
  BTextures::Exit();
  HDC   hDC = wglGetCurrentDC();
  wglMakeCurrent(NULL, NULL);
  if (m_hGLRC)
    wglDeleteContext(m_hGLRC);
  if (hDC)
    ::ReleaseDC(GetSafeHwnd(), hDC);
  CView::OnDestroy();*/
}


//*************************************************************************************************
void CPakoon1View::InitializeOpenGL() {
  // glClearColor(225.0f/255.0f, 234.0f/255.0f, 1, 0); /* For RGB-mode */
  glClearColor(0, 0, 0, 0); /* For RGB-mode */

  glClearDepth(1);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);

  glShadeModel(GL_SMOOTH);
  glEnable(GL_LIGHTING);
  glEnable(GL_NORMALIZE);

  glEnable( GL_LIGHT0);
  OpenGLHelpers::SetDefaultLighting();

  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

  CPakoon1Doc* pDoc = GetDocument();
  //ASSERT_VALID(pDoc);
  switch(m_game.m_nTextureSmoothness) {
    case 0: // Boxy
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      break;
    case 1: // Gritty
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      break;
    case 2: // Silky
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      break;
  }

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  // glClearColor(225.0f/255.0f, 234.0f/255.0f, 1, 0); /* For RGB-mode */
  glClearColor(0, 0, 0, 0);
  if(g_cbBlackAndWhite) {
    // glClearColor(0.7f, 0.7f, 0.7f, 0.0f);
    glClearColor(0, 0, 0, 0);
  }
  // glClearColor(225.0f/255.0f, 234.0f/255.0f, 1, 0); /* For RGB-mode */
  glClearColor(0, 0, 0, 0);

  GLfloat vFogColor[4];
  vFogColor[0] = 225.0f/255.0f;
  vFogColor[1] = 234.0f/255.0f;
  vFogColor[2] = 1.0;
  vFogColor[3] = 0.0;
  glEnable( GL_FOG);
  glFogi( GL_FOG_MODE, GL_LINEAR);
  glFogf( GL_FOG_START, 1000.0f);
  glFogf( GL_FOG_END, float(cdWorldHemisphereRadius));
  glFogfv( GL_FOG_COLOR, vFogColor);

  // Prepare for environment mapping
  // glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
	// glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);	
  glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
  glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);

  return;
}






static double g_cdPI = 3.1415926;



//*************************************************************************************************
//void CPakoon1View::OnDrawIntro(CDC* pDC) {
void CPakoon1View::OnDrawIntro() {
	//FIXME
  /*HDC hDC = pDC->GetSafeHdc();
  wglMakeCurrent(hDC, m_hGLRC); 
  ::ReleaseDC(GetSafeHwnd(), hDC);*/
}




//*************************************************************************************************
//int TerrainLoadThread(LPVOID pParam) {
int TerrainLoadThread(void *) {
  BGame::SetProgressPos(0);

  // Setup Terrain module for this scene
  BGame::GetSimulation()->GetTerrain()->StartUsingScene(BGame::GetSimulation()->GetScene()->m_sName, 
                                                        BGame::GetSimulation()->GetScene()->m_vOrigin, 
                                                        BGame::GetSimulation()->GetScene()->m_dGroundTextureScaler1, 
                                                        BGame::GetSimulation()->GetScene()->m_dGroundTextureScaler2);

  //EnterCriticalSection(&BGame::m_csMutex);
  SDL_LockMutex(BGame::m_csMutex);
  BGame::m_bGameReadyToStart = true;
  //LeaveCriticalSection(&BGame::m_csMutex);
  SDL_UnlockMutex(BGame::m_csMutex);
  //AfxEndThread(0);
  return 0;
}


//*************************************************************************************************
//void CPakoon1View::OnDrawCurrentMenu(CDC* pDC) {
void CPakoon1View::OnDrawCurrentMenu() {

  if(BGame::m_bQuitPending) {
    //AfxGetMainWnd()->PostMessage(WM_COMMAND, ID_FILE_CLOSE, 0);
    setExit();
  }

  if(!BGame::m_bMenusCreated) {
    BGame::SetupMenus();

    // Setup for first menu
    BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), 
                                 &CPakoon1View::OnKeyDownCurrentMenu);
    //Invalidate();

    // Start menu music
    SoundModule::StartMenuMusic();
    SoundModule::SetMenuMusicVolume(int(double(BGame::m_nMusicVolume) / 100.0 * 255.0));

    return;
  }

  //EnterCriticalSection(&(BGame::m_csMutex));
  SDL_LockMutex(BGame::m_csMutex);

  // Check if we need to change display settings
  //static bResolutionChanged = false;
  static bool bResolutionChanged = false;
  //FIXME
  /*if(!bResolutionChanged) {
    bResolutionChanged = true;
    DEVMODE devmode;
    EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devmode);
    if((devmode.dmPelsWidth  != (DWORD)BGame::m_nDispWidth) || 
       (devmode.dmPelsHeight != (DWORD)BGame::m_nDispHeight) || 
       (devmode.dmBitsPerPel != (DWORD)BGame::m_nDispBits) ||
       (devmode.dmDisplayFrequency != (DWORD)BGame::m_nDispHz)) {
      devmode.dmPelsWidth = (DWORD)BGame::m_nDispWidth;
      devmode.dmPelsHeight = (DWORD)BGame::m_nDispHeight;
      devmode.dmBitsPerPel = (DWORD)BGame::m_nDispBits;
      devmode.dmDisplayFrequency = (DWORD)BGame::m_nDispHz;
      ChangeDisplaySettings(&devmode, CDS_FULLSCREEN);
      AfxGetMainWnd()->SetWindowPos(NULL, -2, -2, BGame::m_nDispWidth + 4, BGame::m_nDispHeight + 4, 0);
    }
  }*/

  // Cater for Precaching Terrain menu special needs
  //if(BGame::m_pMenuCurrent->m_type == BMenu::TType::PRECACHING_TERRAIN) {
  if(BGame::m_pMenuCurrent->m_type == BMenu::PRECACHING_TERRAIN) {
    if(!BGame::m_bGameLoading) {
      // Initiate game load (start separate thread)
      BGame::m_bGameReadyToStart = false;
      BGame::m_bGameLoading = true;

      BGame::GetSimulation()->GetScene()->LoadSceneFromFile(BGame::m_sScene);
      BGame::SetupScene();

      // m_pThreadLoading = AfxBeginThread(TerrainLoadThread, 0, THREAD_PRIORITY_LOWEST);
      //m_pThreadLoading = AfxBeginThread(TerrainLoadThread, 0); // XP fix?
#if defined(__EMSCRIPTEN__) && !defined(__EMSCRIPTEN_PTHREADS__)
      TerrainLoadThread((void *)this);
#else
      m_pThreadLoading = SDL_CreateThread(TerrainLoadThread, "TerrainLoad", (void *)this);
#endif
    } else {
      if(BGame::m_bGameReadyToStart) {

        // Prepare to enter game
        BGame::GetPlayer()->LoadStateFile();
        BGame::GetSimulation()->GetScene()->PlaceTerrainObjects();
        BGame::GetSimulation()->GetTerrain()->CreateTerrainDisplayLists();
        BGame::GetSimulation()->GetVehicle()->LoadVehicleFromFile(BGame::m_sVehicle);
        BGame::GetSimulation()->GetVehicle()->m_dFuel = BGame::GetPlayer()->m_dFuel;
        BGame::GetSimulation()->GetVehicle()->m_dKerosine = BGame::GetPlayer()->m_dKerosine;
        BGame::GetSimulation()->GetVehicle()->PreProcessVisualization();
        // (Dunno why, but this needs to be done again here)
        for(int o = 0; o < BGame::GetSimulation()->GetScene()->m_nObjects; ++o) {
          BGame::GetSimulation()->GetScene()->m_pObjects[o].RecreateShadow();
        }

        BMessages::RemoveAll();

        // Init delivery
        //BGame::m_state = BGame::TState::PICKUP;
        BGame::m_state = BGame::PICKUP;
        BGame::m_dPizzaTemp = 0.0;

        // Move vehicle to start location
        BVector vStartLocation = BGame::GetSimulation()->GetScene()->m_vStartLocation;
        BVector vSceneLoc; 
        if(BGame::GetPlayer()->LoadCurrentSceneInfo(vSceneLoc)) {
          vStartLocation = vSceneLoc;
        }
        BGame::GetSimulation()->UpdateCarLocation();
        BVector vLoc = BGame::GetSimulation()->GetVehicle()->m_vLocation;
        BVector val_bvector = vStartLocation - vLoc;
        //BGame::GetSimulation()->GetVehicle()->Move(vStartLocation - vLoc);
        BGame::GetSimulation()->GetVehicle()->Move(val_bvector);
        BGame::GetSimulation()->UpdateCar();

        // Make terrain valid so that the first simulation will work
        BGame::GetSimulation()->GetTerrain()->MakeTerrainValid(BGame::GetSimulation()->GetVehicle()->m_vLocation,
                                                               BGame::GetSimulation()->GetCamera()->m_vLocation,
                                                               BGame::GetSimulation()->GetCamera()->m_orientation.m_vForward,
                                                               true, 
                                                               false, 
                                                               false);
        BVector vOnGround, vNormal;
        BGame::GetSimulation()->EnsureVehicleIsOverGround();
        //BGame::GetSimulation()->GetCamera()->m_locMode = BCamera::TCameraLoc::FOLLOW;
        BGame::GetSimulation()->GetCamera()->m_locMode = BCamera::FOLLOW;
        BGame::GetSimulation()->GetCamera()->m_vLocation = BGame::GetSimulation()->GetVehicle()->m_vLocation + BVector(-20, -10, 0);
        BGame::GetSimulation()->GetCamera()->m_orientation.m_vForward = BGame::GetSimulation()->GetVehicle()->m_vLocation - BGame::GetSimulation()->GetCamera()->m_vLocation;
        BGame::GetSimulation()->GetCamera()->m_orientation.m_vUp = BVector(0, 0, -1);
        BGame::GetSimulation()->GetCamera()->m_orientation.m_vRight = BGame::GetSimulation()->GetCamera()->m_orientation.m_vUp.CrossProduct(BGame::GetSimulation()->GetCamera()->m_orientation.m_vForward);
        // Make terrain valid so that the first simulation will work
        BGame::GetSimulation()->GetTerrain()->MakeTerrainValid(BGame::GetSimulation()->GetVehicle()->m_vLocation,
                                                               BGame::GetSimulation()->GetCamera()->m_vLocation,
                                                               BGame::GetSimulation()->GetCamera()->m_orientation.m_vForward,
                                                               true, 
                                                               false, 
                                                               false);

        // Init earthquake
        BGame::m_bEarthquakeFactor = 0.0;
        BGame::m_bEarthquakeActive = false;
        BGame::m_bEarthquakeNextStart = (double)SDL_GetTicks() + 1000 * (60.0 + Random(60.0));

        // Init wind particles
        if(BGame::m_bWindActive) {
          BGame::UpdateWindParticles(true);
        }

        // Switch to game mode
        SoundModule::StopMenuMusic();
        SoundModule::StartEngineSound();
        SoundModule::SetVehicleSoundsVolume(int(double(BGame::m_nVehicleVolume) / 100.0 * 255.0));
        SoundModule::SetEngineSoundVolume(255.0);
        ReturnPressedOnCurrentMenu();
        m_bInitClock = true;
      }
    }
  }

  //*************************************************
  // Preparations stuff
  //*************************************************

  glClearColor(0, 0, 0, 0);

  /*if(m_hCursor) {
    ::SetCursor(m_hCursor);
  }
  ShowCursor(FALSE);

  HDC hDC = pDC->GetSafeHdc();  
  wglMakeCurrent(hDC, m_hGLRC); // Setup as opengl dc to get the black and white stuff work

  CRect rectWnd;
  GetClientRect(&rectWnd);*/
  SDL_ShowCursor(0);

  // Init OpenGL
  glDrawBuffer(GL_BACK);

  // Reset OpenGL
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  //glViewport(0, 0, (GLint) rectWnd.Width(), (GLint) rectWnd.Height());
  glViewport(0, 0, (GLint) window_width, (GLint) window_height);
  gluLookAt(0, -5, 0, 0, 0, 0, 0, 0, -1);

  GLfloat fLight1PositionG[ 4];
  fLight1PositionG[0] = (GLfloat) 0.25;
  fLight1PositionG[1] = (GLfloat) 0;
  fLight1PositionG[2] = (GLfloat) -1;
  fLight1PositionG[3] = (GLfloat) 0; /* w=0 -> directional light (not positional) */
  glLightfv( GL_LIGHT0, GL_POSITION, fLight1PositionG);                     

  bool bScrolling = false;
  double dScrollPhase = 0;
  unsigned clockNow = SDL_GetTicks();
  if((clockNow - m_clockMenuScroll) < 500.0) {
    bScrolling = true;
    dScrollPhase = double(clockNow - m_clockMenuScroll) / 500.0;
    dScrollPhase = (1.0 + sin(((dScrollPhase * 2.0) - 1.0) * 3.1415926 * 0.5)) * 0.5;
  }

  //*************************************************
  // Draw menu
  //*************************************************

  // Setup 2D rendering
  Setup2DRendering();

  // Draw background
  OpenGLHelpers::SetColorFull(0, 0, 0.5, 1);
  glEnable(GL_COLOR_MATERIAL);
  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  glDisable(GL_TEXTURE_2D);

  //double dPhase1 = sin(2.0 * 3.141592654 * double(clock() % 11031) / 11030.0);
  double dPhase1 = sin(2.0 * 3.141592654 * double(SDL_GetTicks() % 11031) / 11030.0);
  //double dPhase2 = sin(2.0 * 3.141592654 * double(clock() % 17131) / 17130.0);
  double dPhase2 = sin(2.0 * 3.141592654 * double(SDL_GetTicks() % 17131) / 17130.0);
  //double dPhase3 = sin(2.0 * 3.141592654 * double(clock() % 15131) / 15130.0);
  double dPhase3 = sin(2.0 * 3.141592654 * double(SDL_GetTicks() % 15131) / 15130.0);
  //double dPhase4 = sin(2.0 * 3.141592654 * double(clock() % 12131) / 12130.0);
  double dPhase4 = sin(2.0 * 3.141592654 * double(SDL_GetTicks() % 12131) / 12130.0);

  glBegin(GL_TRIANGLE_STRIP);
  glColor4d(dPhase1 * 0.3, 0, 0, 1);
  glVertex3f(0, 0, 0);
  glColor4d(0, dPhase2 * 0.2, 0, 1);
  //glVertex3f(0, rectWnd.Height(), 0);
  glVertex3f(0, window_height, 0);
  glColor4d(0, 0, dPhase3 * 0.2, 1);
  //glVertex3f(rectWnd.Width(), 0, 0);
  glVertex3f(window_width, 0, 0);
  glColor4d(dPhase4 * 0.35, dPhase4 * 0.2, 0, 1);
  //glVertex3f(rectWnd.Width(), rectWnd.Height(), 0);
  glVertex3f(window_width, window_height, 0);
  glEnd();

  glColor4d(1, 1, 1, 1);
  glDisable(GL_COLOR_MATERIAL);

  if(BGame::m_pMenuCurrent != &(BGame::m_menuCredits)) {
    { // ****************** Draw Alpha/Beta text (REMOVE WHEN GAME READY!) ****************

      BUI::TextRenderer()->StartRenderingText();
      string sText = "BETA VERSION! (build 6)";
      //BUI::TextRenderer()->DrawSmallTextAt(rectWnd.Width() / 2, rectWnd.Height() - 40, sText, sText.GetLength(), BTextRenderer::TTextAlign::ALIGN_CENTER, 0.75, 0, 0, 1);
      BUI::TextRenderer()->DrawSmallTextAt(window_width / 2, window_height - 40, sText, sText.length(), BTextRenderer::ALIGN_CENTER, 0.75, 0, 0, 1);
      BUI::TextRenderer()->StopRenderingText();
    }

    // Always draw the copyright on bottom
    OpenGLHelpers::SetColorFull(0.65, 0.7, 0.75, 1);
    OpenGLHelpers::SwitchToTexture(0);
    //BTextures::Use(BTextures::Texture::MENU_TITLES);
    BTextures::Use(BTextures::MENU_TITLES);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // not mipmapped

    glPushMatrix();
    //glTranslated((rectWnd.Width() - 512) / 2, 0, 0);
    glTranslated((window_width - 512) / 2, 0, 0);
    glBegin(GL_TRIANGLE_STRIP);
    OpenGLHelpers::SetTexCoord(0, 0);
    glVertex3f(0, 0, 0);
    OpenGLHelpers::SetTexCoord(0, 30.0 / 512.0);
    glVertex3f(0, 30, 0);
    OpenGLHelpers::SetTexCoord(1, 0);
    glVertex3f(512, 0, 0);
    OpenGLHelpers::SetTexCoord(1, 30.0 / 512.0);
    glVertex3f(512, 30, 0);
    glEnd();
    glPopMatrix();
  }

  if(bScrolling) {
    // Draw previous and new menu scrolling
    glPushMatrix();
    if(m_scrollDir == SCROLL_RIGHT) {
      //glTranslated(-int(double(rectWnd.Width() * dScrollPhase)), 0, 0);
      glTranslated(-int(double(window_width * dScrollPhase)), 0, 0);
    } else if(m_scrollDir == SCROLL_LEFT) {
      //glTranslated(+int(double(rectWnd.Width() * dScrollPhase)), 0, 0);
      glTranslated(+int(double(window_width * dScrollPhase)), 0, 0);
    } else if(m_scrollDir == SCROLL_UP) {
      //glTranslated(0, -int(double(rectWnd.Height() * dScrollPhase)), 0);
      glTranslated(0, -int(double(window_height * dScrollPhase)), 0);
    } else if(m_scrollDir == SCROLL_DOWN) {
      //glTranslated(0, +int(double(rectWnd.Height() * dScrollPhase)), 0);
      glTranslated(0, +int(double(window_height * dScrollPhase)), 0);
    }
    DrawMenu(BGame::m_pMenuPrevious);
    glPopMatrix();

    glPushMatrix();
    if(m_scrollDir == SCROLL_RIGHT) {
      //glTranslated(int(double(rectWnd.Width() * (1.0 - dScrollPhase))), 0, 0);
      glTranslated(int(double(window_width * (1.0 - dScrollPhase))), 0, 0);
    } else if(m_scrollDir == SCROLL_LEFT) {
      //glTranslated(-int(double(rectWnd.Width() * (1.0 - dScrollPhase))), 0, 0);
      glTranslated(-int(double(window_width * (1.0 - dScrollPhase))), 0, 0);
    } else if(m_scrollDir == SCROLL_UP) {
      //glTranslated(0, +int(double(rectWnd.Height() * (1.0 - dScrollPhase))), 0);
      glTranslated(0, +int(double(window_height * (1.0 - dScrollPhase))), 0);
    } else if(m_scrollDir == SCROLL_DOWN) {
      //glTranslated(0, -int(double(rectWnd.Height() * (1.0 - dScrollPhase))), 0);
      glTranslated(0, -int(double(window_height * (1.0 - dScrollPhase))), 0);
    }
    DrawMenu(BGame::m_pMenuCurrent);
    glPopMatrix();
  } else {
    DrawMenu(BGame::m_pMenuCurrent);
  }

  //*************************************************
  // Finish draw
  //*************************************************

  glFinish();

  //SwapBuffers(hDC);
  SDL_GL_SwapWindow(window);

  //LeaveCriticalSection(&(BGame::m_csMutex));
  SDL_UnlockMutex(BGame::m_csMutex);

  // And again and again and...
  if((BGame::m_pMenuCurrent == &(BGame::m_menuPrecachingTerrain)) && !BGame::m_bMultiProcessor) {
    //Sleep(1);
    SDL_Delay(1);
  }
  //Invalidate();
}


//*************************************************************************************************
void CPakoon1View::DrawMenu(BMenu *pMenu) {
  //CRect rectWnd;
  //GetClientRect(&rectWnd);

  if(pMenu) {

    double dCharHeight = BUI::TextRenderer()->GetCharHeight();
    double dCharWidth  = BUI::TextRenderer()->GetCharWidth();

    // Draw menu title

    glPushMatrix();
    //int nThird = rectWnd.Height() / 6;
    int nThird = window_height / 6;
    //glTranslated((rectWnd.Width() - pMenu->m_nTitleWidth) / 2, 
    glTranslated((window_width - pMenu->m_nTitleWidth) / 2, 
                 //rectWnd.Height() - nThird - pMenu->m_nTitleHeight / 2,
                 window_height - nThird - pMenu->m_nTitleHeight / 2,
                 0);

    if(pMenu->m_nTitleWidth) {
      OpenGLHelpers::SwitchToTexture(0);
      //BTextures::Use(BTextures::Texture::MENU_TITLES);
      BTextures::Use(BTextures::MENU_TITLES);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // not mipmapped

      OpenGLHelpers::SetColorFull(1, 1, 1, 1);
      glBegin(GL_TRIANGLE_STRIP);
      OpenGLHelpers::SetTexCoord(pMenu->m_dTitleX, pMenu->m_dTitleY);
      glVertex3f(0, 0, 0);
      OpenGLHelpers::SetTexCoord(pMenu->m_dTitleX, pMenu->m_dTitleY + (double(pMenu->m_nTitleHeight) / 512.0));
      glVertex3f(0, pMenu->m_nTitleHeight, 0);
      OpenGLHelpers::SetTexCoord(pMenu->m_dTitleX + (double(pMenu->m_nTitleWidth) / 512.0), pMenu->m_dTitleY);
      glVertex3f(pMenu->m_nTitleWidth, 0, 0);
      OpenGLHelpers::SetTexCoord(pMenu->m_dTitleX + (double(pMenu->m_nTitleWidth) / 512.0), pMenu->m_dTitleY + (double(pMenu->m_nTitleHeight) / 512.0));
      glVertex3f(pMenu->m_nTitleWidth, pMenu->m_nTitleHeight, 0);
      glEnd();
    }

    // Draw separator
    if(pMenu->m_bDrawLine) {
      glEnable(GL_COLOR_MATERIAL);
      glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
      glDisable(GL_TEXTURE_2D);
      glBegin(GL_LINE_STRIP);
      glColor3d(0, 0, 0);
      glVertex3f(-pMenu->m_nTitleWidth / 2 - 10, 0, 0);
      glColor3d(1, 1, 1);
      glVertex3f(-pMenu->m_nTitleWidth / 2, 0, 0);
      glColor3d(0.25, 0.35, 0.4);
      glVertex3f(pMenu->m_nTitleWidth + pMenu->m_nTitleWidth / 2, 0, 0);
      glColor3d(0, 0, 0);
      glVertex3f(pMenu->m_nTitleWidth + pMenu->m_nTitleWidth / 2 + 10, 0, 0);
      glEnd();
      glTranslated(0, 4, 0);
      glBegin(GL_LINE_STRIP);
      glColor3d(0, 0, 0);
      glVertex3f(-pMenu->m_nTitleWidth / 2 - 10, 0, 0);
      glColor3d(1, 1, 1);
      glVertex3f(-pMenu->m_nTitleWidth / 2, 0, 0);
      glColor3d(0.25, 0.35, 0.4);
      glVertex3f(pMenu->m_nTitleWidth + pMenu->m_nTitleWidth / 2, 0, 0);
      glColor3d(0, 0, 0);
      glVertex3f(pMenu->m_nTitleWidth + pMenu->m_nTitleWidth / 2 + 10, 0, 0);
      glEnd();
      glColor3d(1, 1, 1);
      glDisable(GL_COLOR_MATERIAL);
    }

    glPopMatrix();

    // Draw menu contents
    if(pMenu->m_listMenu.GetNofItems()) {
      int i;

      //if(pMenu->m_type == BMenu::TType::CHOOSE_VEHICLE) {
      if(pMenu->m_type == BMenu::CHOOSE_VEHICLE) {
        for(i = 0; i < pMenu->m_nItems; ++i) {
          // Check for owned vehicles
          string sVehicle;
          //sVehicle.Format(">%s<", pMenu->m_items[i].m_sText);
			sVehicle = ">" + pMenu->m_items[i].m_sText + "<";
          //if(BGame::GetPlayer()->m_sValidVehicles.Find(sVehicle) == -1) {
          if(BGame::GetPlayer()->m_sValidVehicles.find(sVehicle) == -1) {
            pMenu->m_items[i].m_bDisabled = true;
          } else {
            pMenu->m_items[i].m_bDisabled = false;
          }
        }
      }

      //pMenu->m_listMenu.DrawAt(rectWnd.Width() / 2, rectWnd.Height() / 2, pMenu->m_align, 1, 1, 1, false);
      pMenu->m_listMenu.DrawAt(window_width / 2, window_height / 2, pMenu->m_align, 1, 1, 1, false);

      // If there are menu items that have an associated list value,
      // draw them also
      for(i = 0; i < pMenu->m_nItems; ++i) {
        //if(pMenu->m_items[i].m_type == BMenuItem::TType::STRING_FROM_LIST) {
        if(pMenu->m_items[i].m_type == BMenuItem::STRING_FROM_LIST) {
          // Draw list string
          //DrawMenuItemTextAtRelPos(rectWnd.Width() / 2, 
          DrawMenuItemTextAtRelPos(window_width / 2, 
                                   //rectWnd.Height() / 2, 
                                   window_height / 2, 
                                   pMenu->m_nItems,
                                   i, 
                                   &(pMenu->m_items[i]));
        //} else if(pMenu->m_items[i].m_type == BMenuItem::TType::SLIDER) {
        } else if(pMenu->m_items[i].m_type == BMenuItem::SLIDER) {
          // Draw slider
          //DrawMenuItemSliderAtRelPos(rectWnd.Width() / 2, 
          DrawMenuItemSliderAtRelPos(window_width / 2, 
                                     //rectWnd.Height() / 2, 
                                     window_height / 2, 
                                     pMenu->m_nItems,
                                     i, 
                                     &(pMenu->m_items[i]));
        }
      }

      // Draw open subitem
      for(i = 0; i < pMenu->m_nItems; ++i) {
        if(pMenu->m_items[i].m_bOpen) {
          //if(pMenu->m_items[i].m_type == BMenuItem::TType::STRING_FROM_LIST) {
          if(pMenu->m_items[i].m_type == BMenuItem::STRING_FROM_LIST) {
            // Draw submenu
            //pMenu->m_items[i].m_listMenu.DrawAt(rectWnd.Width() / 2 + 35, 
            pMenu->m_items[i].m_listMenu.DrawAt(window_width / 2 + 35, 
                                                //rectWnd.Height() / 2 + double(i) * -dCharHeight + (dCharHeight * double(pMenu->m_nItems)) / 2.0, 
                                                window_height / 2 + double(i) * -dCharHeight + (dCharHeight * double(pMenu->m_nItems)) / 2.0, 
                                                //BTextRenderer::TTextAlign::ALIGN_LEFT,
                                                BTextRenderer::ALIGN_LEFT,
                                                1, 
                                                1, 
                                                1,
                                                true,
                                                true);
          //} else if(pMenu->m_items[i].m_type == BMenuItem::TType::SLIDER) {
          } else if(pMenu->m_items[i].m_type == BMenuItem::SLIDER) {
            // Draw open slider
            //DrawMenuItemSliderAtRelPos(rectWnd.Width() / 2, 
            DrawMenuItemSliderAtRelPos(window_width / 2, 
                                       //rectWnd.Height() / 2, 
                                       window_height / 2, 
                                       pMenu->m_nItems,
                                       i, 
                                       &(pMenu->m_items[i]));
          }
        }
      }

      // Draw associated image, if one is available
      string sTmp;
      int nSelected = pMenu->m_listMenu.GetSelectedItem(sTmp);
      if((nSelected != -1) && (pMenu->m_items[nSelected].m_nAssocImage != -1)) {
        OpenGLHelpers::SetColorFull(1, 1, 1, 1);
        OpenGLHelpers::SwitchToTexture(0);
        BTextures::Use(pMenu->m_items[nSelected].m_nAssocImage);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // not mipmapped
        glPushMatrix();
        //glTranslated(rectWnd.Width() / 2 + rectWnd.Width() / 40,
        glTranslated(window_width / 2 + window_width / 40,
                     //rectWnd.Height() / 2, 
                     window_height / 2, 
                     0);
        glBegin(GL_TRIANGLE_STRIP);
        OpenGLHelpers::SetTexCoord(0, 0);
        glVertex3f(0, -128, 0);
        OpenGLHelpers::SetTexCoord(0, 1);
        glVertex3f(0, 128, 0);
        OpenGLHelpers::SetTexCoord(1, 0);
        glVertex3f(256, -128, 0);
        OpenGLHelpers::SetTexCoord(1, 1);
        glVertex3f(256, 128, 0); 
        glEnd();

        // If disabled, draw veil over image
        if(pMenu->m_items[nSelected].m_bDisabled) {
          OpenGLHelpers::SetColorFull(0, 0, 0, 0.5);
          glDisable(GL_TEXTURE_2D);
          glBegin(GL_LINES);
          for(i = 1; i < 256; i += 2) {
            glVertex3f(0, -128 + i, 0);
            glVertex3f(256, -128 + i, 0);
          }
          OpenGLHelpers::SetColorFull(0, 0, 0, 0.25);
          for(i = 0; i < 256; i += 2) {
            glVertex3f(0, -128 + i, 0);
            glVertex3f(256, -128 + i, 0);
          }
          glEnd();
          OpenGLHelpers::SetColorFull(1, 1, 1, 1);
        }

        glPopMatrix();
      }
    }

    // Draw credits images with fade, if showing credits
    //if(pMenu->m_type == BMenu::TType::CREDITS) {
    if(pMenu->m_type == BMenu::CREDITS) {
      // Find the active item(s)
      int    i;
      int    nItems = 0;
      int    nItemsToDraw[2];
      double dItemsAlpha[2];
      nItemsToDraw[0] = -1;
      nItemsToDraw[1] = -1;
      unsigned clockNow = SDL_GetTicks();
      double dCurSecond = double(clockNow - pMenu->m_clockStarted) / 1000.0;

      if((BGame::m_pMenuCurrent == &(BGame::m_menuCredits)) && 
         (dCurSecond > (1 + pMenu->m_items[pMenu->m_nItems - 1].m_nValue + pMenu->m_items[pMenu->m_nItems - 1].m_nValue2))) {
        // Return to main menu
        BGame::m_pMenuPrevious = BGame::m_pMenuCurrent;
        // BGame::m_pMenuPrevious = &(BGame::m_menuCredits);
        BGame::m_pMenuCurrent = &(BGame::m_menuMain);
        BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), 
                                     &CPakoon1View::OnKeyDownCurrentMenu);
        StartMenuScroll(SCROLL_UP);
        //Invalidate();
        return;
      } else {
        for(i = 0; i < pMenu->m_nItems; ++i) {
          double dFrom = double(pMenu->m_items[i].m_nValue);
          double dTo = dFrom + double(pMenu->m_items[i].m_nValue2);
          if((dCurSecond >= dFrom) && 
             (dCurSecond <= (dTo - 2.0))) {
            nItemsToDraw[nItems] = i;
            dItemsAlpha[nItems] = 1.0;
            ++nItems;
          } else if((dCurSecond >= (dFrom - 2.0)) && 
                    (dCurSecond <= (dFrom))) {
            // Fading in
            nItemsToDraw[nItems] = i;
            dItemsAlpha[nItems] = 0.5 * (dCurSecond - (dFrom - 2.0));
            ++nItems;
          } else if((dCurSecond >= (dTo - 2.0)) && 
                    (dCurSecond <= (dTo))) {
            // Fading out
            nItemsToDraw[nItems] = i;
            dItemsAlpha[nItems] = 1.0 - (0.5 * (dCurSecond - (dTo - 2.0)));
            ++nItems;
          }
          if(nItems == 2) {
            break;
          }
        }

        // Draw the two menu items (their images)
        glPushMatrix();
        //glTranslated(rectWnd.Width() / 2 - 256.0,
        glTranslated(window_width / 2 - 256.0,
                     //rectWnd.Height() / 2, 
                     window_height / 2, 
                     0);
        for(i = 0; i < nItems; ++i) {
          if(pMenu->m_items[nItemsToDraw[i]].m_nAssocImage >= 0) {
            OpenGLHelpers::SetColorFull(1, 1, 1, dItemsAlpha[i]);
            OpenGLHelpers::SwitchToTexture(0);
            BTextures::Use(pMenu->m_items[nItemsToDraw[i]].m_nAssocImage);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // not mipmapped
            glBegin(GL_TRIANGLE_STRIP);
            OpenGLHelpers::SetTexCoord(0, 0);
            glVertex3f(0, -128, 0);
            OpenGLHelpers::SetTexCoord(0, 1);
            glVertex3f(0, 128, 0);
            OpenGLHelpers::SetTexCoord(1, 0);
            glVertex3f(512, -128, 0);
            OpenGLHelpers::SetTexCoord(1, 1);
            glVertex3f(512, 128, 0); 
            glEnd();
          }
        }

        // Draw frames
        float dAlpha = 0.5;
        if((dCurSecond < (pMenu->m_items[1].m_nValue - 2.0)) || 
           (dCurSecond > (pMenu->m_items[pMenu->m_nItems - 1].m_nValue))) {
          dAlpha = 0;
        }
        if((dCurSecond >= (pMenu->m_items[1].m_nValue - 2.0)) && 
           (dCurSecond <= (pMenu->m_items[1].m_nValue))) {
          dAlpha = 0.5 * 0.5 * (dCurSecond - (pMenu->m_items[1].m_nValue - 2.0));
        }
        if((dCurSecond >= (pMenu->m_items[pMenu->m_nItems - 2].m_nValue + pMenu->m_items[pMenu->m_nItems - 2].m_nValue2 - 2.0)) && 
           (dCurSecond <= (pMenu->m_items[pMenu->m_nItems - 2].m_nValue + pMenu->m_items[pMenu->m_nItems - 2].m_nValue2))) {
          dAlpha = 0.5 * (1.0 - 0.5 * (dCurSecond - (pMenu->m_items[pMenu->m_nItems - 2].m_nValue + pMenu->m_items[pMenu->m_nItems - 2].m_nValue2 - 2.0)));
        }

        glEnable(GL_COLOR_MATERIAL);
        glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
        glDisable(GL_TEXTURE_2D);

        glBegin(GL_LINES);
        glColor4f(1, 1, 1, dAlpha);
        glVertex3f(0, -128, 0);
        glVertex3f(0,  127, 0);
        glVertex3f(511, -128, 0);
        glVertex3f(511,  127, 0);
        glVertex3f(0, -128, 0);
        glVertex3f(511, -128, 0);
        glVertex3f(0, 127, 0);
        glVertex3f(511, 127, 0);

        glColor4f(0, 0, 0, 0);
        glVertex3f(0, -128 - 40, 0);
        glColor4f(1, 1, 1, dAlpha);
        glVertex3f(0, -128, 0);

        glColor4f(0, 0, 0, 0);
        glVertex3f(511, -128 - 40, 0);
        glColor4f(1, 1, 1, dAlpha);
        glVertex3f(511, -128, 0);

        glColor4f(0, 0, 0, 0);
        glVertex3f(0, 127 + 40, 0);
        glColor4f(1, 1, 1, dAlpha);
        glVertex3f(0, 127, 0);

        glColor4f(0, 0, 0, 0);
        glVertex3f(511, 127 + 40, 0);
        glColor4f(1, 1, 1, dAlpha);
        glVertex3f(511, 126, 0);

        glColor4f(0, 0, 0, 0);
        glVertex3f(-40, -128, 0);
        glColor4f(1, 1, 1, dAlpha);
        glVertex3f(0, -128, 0);

        glColor4f(0, 0, 0, 0);
        glVertex3f(-40, 127, 0);
        glColor4f(1, 1, 1, dAlpha);
        glVertex3f(0, 127, 0);

        glColor4f(0, 0, 0, 0);
        glVertex3f(511 + 40, -128, 0);
        glColor4f(1, 1, 1, dAlpha);
        glVertex3f(511, -128, 0);

        glColor4f(0, 0, 0, 0);
        glVertex3f(511 + 40, 127, 0);
        glColor4f(1, 1, 1, dAlpha);
        glVertex3f(511, 127, 0);

        glEnd();

        glDisable(GL_COLOR_MATERIAL);

        glPopMatrix();
      }
    }

    // Draw progress indicator, if loading terrain
    //if(BGame::m_pMenuCurrent->m_type == BMenu::TType::PRECACHING_TERRAIN) {
    if(BGame::m_pMenuCurrent->m_type == BMenu::PRECACHING_TERRAIN) {
      OpenGLHelpers::SetColorFull(1, 1, 1, 1);
      glEnable(GL_COLOR_MATERIAL);
      glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
      glDisable(GL_TEXTURE_2D);

      //double dSmall = double(rectWnd.Height()) / 8.0;
      double dSmall = double(window_height) / 8.0;
      //double dBig = double(rectWnd.Height()) / 5.3;
      double dBig = double(window_height) / 5.3;

      glPushMatrix();
      //glTranslated(rectWnd.Width() / 2, rectWnd.Height() / 2, 0);
      glTranslated(window_width / 2, window_height / 2, 0);
      glBegin(GL_TRIANGLE_STRIP);
      int nLastAngle = int(BGame::GetRelativeProgress() * 360.0);
      if(nLastAngle > 360) {
        nLastAngle = 360;
      }
      int i;
      for(i = 0; i <= nLastAngle; ++i) {
        double dAngle = (double(i) / 360.0) * (2.0 * 3.141592654);
        glColor4d(0.8, 0.9, 1.0, 1);
        glVertex3f(sin(dAngle) * dSmall, cos(dAngle) * dSmall, 0);
        glColor4d(0, 0, 0, 0);
        glVertex3f(sin(dAngle) * dBig, cos(dAngle) * dBig, 0);
      }
      glEnd();
      glBegin(GL_TRIANGLE_STRIP);
      for(i = 0; i <= nLastAngle; ++i) {
        double dAngle = (double(i) / 360.0) * (2.0 * 3.141592654);
        glColor4d(0, 0, 0, 0);
        glVertex3f(sin(dAngle) * (dSmall - 2), cos(dAngle) * (dSmall - 2), 0);
        glColor4d(0.8, 0.9, 1.0, 1);
        glVertex3f(sin(dAngle) * dSmall, cos(dAngle) * dSmall, 0);
      }
      glEnd();
      glPopMatrix();
      glColor4d(1, 1, 1, 1);
      glDisable(GL_COLOR_MATERIAL);
    }

    // Finally, draw message box over menu, if needed
    //if(pMenu->m_type == BMenu::TType::CHOOSE_VEHICLE) {
    if(pMenu->m_type == BMenu::CHOOSE_VEHICLE) {
      glPushMatrix();
      //glTranslated(rectWnd.Width() / 2, rectWnd.Height() / 2, 0);
      glTranslated(window_width / 2, window_height / 2, 0);
      if(BGame::m_bBuyingVehicle) {
        DrawPanel(dCharWidth * 32, dCharHeight * 8);
        BUI::TextRenderer()->StartRenderingText();
        //BUI::TextRenderer()->DrawTextAt(0, dCharHeight * 2, "Do you want to buy", BTextRenderer::TTextAlign::ALIGN_CENTER);
        BUI::TextRenderer()->DrawTextAt(0, dCharHeight * 2, "Do you want to buy", BTextRenderer::ALIGN_CENTER);
        string sTmp;
        string sVehicle;
        pMenu->m_listMenu.GetSelectedItem(sVehicle);
        //sTmp.Format("%s?", sVehicle);
		sTmp = sVehicle + "?";
        //BUI::TextRenderer()->DrawTextAt(0, dCharHeight * 1, sTmp, BTextRenderer::TTextAlign::ALIGN_CENTER);
        BUI::TextRenderer()->DrawTextAt(0, dCharHeight * 1, sTmp, BTextRenderer::ALIGN_CENTER);
        //sTmp.Format("(It costs %.0lf. You have %.0lf)", BGame::m_dPurchasePrice, BGame::GetPlayer()->m_dCash);
		stringstream format;
		format << "(It costs " << (int) BGame::m_dPurchasePrice << ". You have " << (int) BGame::GetPlayer()->m_dCash;
		sTmp = format.str();
        //BUI::TextRenderer()->DrawTextAt(0, 0, sTmp, BTextRenderer::TTextAlign::ALIGN_CENTER);
        BUI::TextRenderer()->DrawTextAt(0, 0, sTmp, BTextRenderer::ALIGN_CENTER);
        BUI::TextRenderer()->StopRenderingText();
        //BGame::m_listYesNo.DrawAt(0, -dCharHeight * 2.5, BTextRenderer::TTextAlign::ALIGN_CENTER, 1, 1, 1, false);
        BGame::m_listYesNo.DrawAt(0, -dCharHeight * 2.5, BTextRenderer::ALIGN_CENTER, 1, 1, 1, false);
      } else if(BGame::m_bCannotBuyVehicle) {
        DrawPanel(dCharWidth * 32, dCharHeight * 8);
        BUI::TextRenderer()->StartRenderingText();
        //BUI::TextRenderer()->DrawTextAt(0, dCharHeight * 2, "You can not buy", BTextRenderer::TTextAlign::ALIGN_CENTER);
        BUI::TextRenderer()->DrawTextAt(0, dCharHeight * 2, "You can not buy", BTextRenderer::ALIGN_CENTER);
        string sTmp;
        string sVehicle;
        pMenu->m_listMenu.GetSelectedItem(sVehicle);
        //sTmp.Format("%s", sVehicle);
		sTmp = sVehicle;
        //BUI::TextRenderer()->DrawTextAt(0, dCharHeight * 1, sTmp, BTextRenderer::TTextAlign::ALIGN_CENTER);
        BUI::TextRenderer()->DrawTextAt(0, dCharHeight * 1, sTmp, BTextRenderer::ALIGN_CENTER);
        //sTmp.Format("(It costs %.0lf. You have %.0lf)", BGame::m_dPurchasePrice, BGame::GetPlayer()->m_dCash);
		stringstream format;
		format << "(It costs " << (int) BGame::m_dPurchasePrice << ". You have " << (int) BGame::GetPlayer()->m_dCash;
		sTmp = format.str();
        //BUI::TextRenderer()->DrawTextAt(0, 0, sTmp, BTextRenderer::TTextAlign::ALIGN_CENTER);
        BUI::TextRenderer()->DrawTextAt(0, 0, sTmp, BTextRenderer::ALIGN_CENTER);
        BUI::TextRenderer()->StopRenderingText();
        //BGame::m_listOK.DrawAt(0, -dCharHeight * 2.5, BTextRenderer::TTextAlign::ALIGN_CENTER, 1, 1, 1, false);
        BGame::m_listOK.DrawAt(0, -dCharHeight * 2.5, BTextRenderer::ALIGN_CENTER, 1, 1, 1, false);
      }
      glPopMatrix();
    }
  }
}



//*************************************************************************************************
void CPakoon1View::DrawMenuItemTextAtRelPos(int nX, int nY, int nItems, int nIndex, BMenuItem *pMenuItem) {
  double dCharHeight = BUI::TextRenderer()->GetCharHeight();
  double dCharWidth  = BUI::TextRenderer()->GetCharWidth();

  if((pMenuItem->m_nValue >= 0) && (pMenuItem->m_nValue < pMenuItem->m_nAssocListItems)) {

    glPushMatrix();
    glTranslated(10, (dCharHeight * double(nItems)) / 2.0, 0);

    BUI::TextRenderer()->StartRenderingText();
    BUI::TextRenderer()->DrawTextAt(nX,
                                    nY + double(nIndex) * -dCharHeight,
                                    pMenuItem->m_sAssocListItems[pMenuItem->m_nValue],
                                    //BTextRenderer::TTextAlign::ALIGN_LEFT,
                                    BTextRenderer::ALIGN_LEFT,
                                    0.7,
                                    0.7,
                                    0.7);
    BUI::TextRenderer()->StopRenderingText();
    glPopMatrix();
  }  
}


//*************************************************************************************************
void CPakoon1View::DrawMenuItemSliderAtRelPos(int nX, int nY, int nItems, int nIndex, BMenuItem *pMenuItem) {
  double dCharHeight = BUI::TextRenderer()->GetCharHeight();
  double dCharWidth  = BUI::TextRenderer()->GetCharWidth();

  OpenGLHelpers::SwitchToTexture(1, true);
  OpenGLHelpers::SwitchToTexture(0, true);
  glEnable(GL_COLOR_MATERIAL);
  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  glDisable(GL_TEXTURE_2D);

  if((pMenuItem->m_nValue >= 0) && (pMenuItem->m_nValue <= 100)) {

    glPushMatrix();
    glTranslated(nX + 10, -1 + nY + double(nIndex) * -dCharHeight + (dCharHeight * double(nItems)) / 2.0, 0);

    double dDimmer = 0.75;
    if(pMenuItem->m_bOpen) {
      double dAlpha = fabs(double(SDL_GetTicks() % 1000) - 500.0) / 500.0;
      dDimmer = 0.6 + 0.4 * dAlpha;
    }

    glBegin(GL_TRIANGLE_STRIP);

    glColor4d(dDimmer * 1, dDimmer * 1, dDimmer * 1, 1);
    glVertex3f(0, -dCharHeight / 6, 0);
    glColor4d(0, 0, 0, 0);
    glVertex3f(0, -dCharHeight / 6 - dCharHeight / 3, 0);
    glColor4d(dDimmer * 1, dDimmer * 1, dDimmer * 1, 1);
    glVertex3f(100, -dCharHeight / 6, 0);
    glColor4d(0, 0, 0, 0);
    glVertex3f(100, -dCharHeight / 6 - dCharHeight / 3, 0);

    glColor4d(dDimmer * 1, dDimmer * 1, dDimmer * 1, 1);
    glVertex3f(100, -dCharHeight / 6, 0);
    glColor4d(0, 0, 0, 0);
    glVertex3f(100 + dCharHeight / 3, -dCharHeight / 6, 0);
    glColor4d(dDimmer * 1, dDimmer * 1, dDimmer * 1, 1);
    glVertex3f(100, -dCharHeight / 6 + dCharHeight / 3, 0);
    glColor4d(0, 0, 0, 0);
    glVertex3f(100 + dCharHeight / 3, -dCharHeight / 6 + dCharHeight / 3, 0);

    glColor4d(dDimmer * 1, dDimmer * 1, dDimmer * 1, 1);
    glVertex3f(100, dCharHeight / 6, 0);
    glColor4d(0, 0, 0, 0);
    glVertex3f(100, dCharHeight / 6 + dCharHeight / 3, 0);
    glColor4d(dDimmer * 1, dDimmer * 1, dDimmer * 1, 1);
    glVertex3f(0, dCharHeight / 6, 0);
    glColor4d(0, 0, 0, 0);
    glVertex3f(0, dCharHeight / 6 + dCharHeight / 3, 0);

    glColor4d(dDimmer * 1, dDimmer * 1, dDimmer * 1, 1);
    glVertex3f(0, -dCharHeight / 6 + dCharHeight / 3, 0);
    glColor4d(0, 0, 0, 0);
    glVertex3f(0 - dCharHeight / 3, -dCharHeight / 6 + dCharHeight / 3, 0);
    glColor4d(dDimmer * 1, dDimmer * 1, dDimmer * 1, 1);
    glVertex3f(0, -dCharHeight / 6, 0);
    glColor4d(0, 0, 0, 0);
    glVertex3f(0 - dCharHeight / 3, -dCharHeight / 6, 0);

    glColor4d(dDimmer * 1, dDimmer * 1, dDimmer * 1, 1);
    glVertex3f(0, -dCharHeight / 6, 0);
    glColor4d(0, 0, 0, 0);
    glVertex3f(0, -dCharHeight / 6 - dCharHeight / 3, 0);

    glEnd();

    glBegin(GL_QUADS);
    glColor4d(dDimmer * 0.5, dDimmer * 0.65, dDimmer * 0.9, 1);
    glVertex3f(0, -dCharHeight / 6, 0);
    glColor4d(dDimmer * 0.3, dDimmer * 0.45, dDimmer * 0.7, 1);
    glVertex3f(pMenuItem->m_nValue, -dCharHeight / 6, 0);
    glColor4d(dDimmer * 0.4, dDimmer * 0.5, dDimmer * 0.8, 1);
    glVertex3f(pMenuItem->m_nValue, dCharHeight / 6, 0);
    glColor4d(dDimmer * 0.8, dDimmer * 0.9, dDimmer * 1, 1);
    glVertex3f(0, dCharHeight / 6, 0);
    glEnd();

    /*
    glBegin(GL_QUADS);
    glColor4d(dDimmer * 0.8, dDimmer * 0.9, dDimmer * 1, 1);
    glVertex3f(0, -dCharHeight / 6, 0);
    glColor4d(dDimmer * 0.4, dDimmer * 0.5, dDimmer * 0.8, 1);
    glVertex3f(pMenuItem->m_nValue, -dCharHeight / 6, 0);
    glColor4d(dDimmer * 0.3, dDimmer * 0.45, dDimmer * 0.7, 1);
    glVertex3f(pMenuItem->m_nValue, dCharHeight / 6, 0);
    glColor4d(dDimmer * 0.5, dDimmer * 0.65, dDimmer * 0.9, 1);
    glVertex3f(0, dCharHeight / 6, 0);
    glEnd();
    */
    // glColor4d(dDimmer * 1, dDimmer * 1, dDimmer * 1, 1);
    glColor4d(1, 1, 1, 1);
    glBegin(GL_LINE_STRIP);
    glVertex3f(0, -dCharHeight / 6, 0);
    glVertex3f(100, -dCharHeight / 6, 0);
    glVertex3f(100, dCharHeight / 6, 0);
    glVertex3f(0, dCharHeight / 6, 0);
    glVertex3f(0, -dCharHeight / 6, 0);
    glEnd();

    glPopMatrix();
  }  

  glColor4d(1, 1, 1, 1);
  glDisable(GL_COLOR_MATERIAL);
}




//*************************************************************************************************
void CPakoon1View::ReturnPressedOnCurrentMenu() {
  //if(BGame::m_pMenuCurrent->m_type == BMenu::TType::SETTINGS) {
  if(BGame::m_pMenuCurrent->m_type == BMenu::SETTINGS) {
    // See if we need to open/close sublist/slider
    string sTmp;
    int nSelected = BGame::m_pMenuCurrent->m_listMenu.GetSelectedItem(sTmp);
    if(nSelected != -1) {
      BMenuItem *pMenuItem = &(BGame::m_pMenuCurrent->m_items[nSelected]);
      if(!pMenuItem->m_bOpen) {
        //if(pMenuItem->m_type == BMenuItem::TType::STRING_FROM_LIST) {
        if(pMenuItem->m_type == BMenuItem::STRING_FROM_LIST) {
          // Open menu
          pMenuItem->m_bOpen = true;
          pMenuItem->m_listMenu.SetItems(pMenuItem->m_sAssocListItems, pMenuItem->m_nAssocListItems);
          pMenuItem->m_listMenu.SelectItem(pMenuItem->m_sAssocListItems[pMenuItem->m_nValue]);
          BUI::StartUsingSelectionList(&(pMenuItem->m_listMenu), 
                                       &CPakoon1View::OnKeyDownCurrentMenu);
        //} else if(pMenuItem->m_type == BMenuItem::TType::SLIDER) {
        } else if(pMenuItem->m_type == BMenuItem::SLIDER) {
          // Open slider
          pMenuItem->m_bOpen = true;
          BUI::StartUsingSlider(&(pMenuItem->m_nValue), 
                                &CPakoon1View::OnKeyDownCurrentMenu);
        }
        //Invalidate();
      } else {
        //if(pMenuItem->m_type == BMenuItem::TType::STRING_FROM_LIST) {
        if(pMenuItem->m_type == BMenuItem::STRING_FROM_LIST) {
          // Update value
          pMenuItem->m_nValue = pMenuItem->m_listMenu.GetSelectedItem(sTmp);
        //} else if(pMenuItem->m_type == BMenuItem::TType::SLIDER) {
        } else if(pMenuItem->m_type == BMenuItem::SLIDER) {
          // Set the selected volume
          //if(pMenuItem->m_sText.CompareNoCase("Music Volume:") == 0) {
          if(pMenuItem->m_sText.compare("Music Volume:") == 0) {
            SoundModule::SetMenuMusicVolume(int(double(pMenuItem->m_nValue) / 100.0 * 255.0));
          //} else if(pMenuItem->m_sText.CompareNoCase("Vehicle Volume:") == 0) {
          } else if(pMenuItem->m_sText.compare("Vehicle Volume:") == 0) {
            SoundModule::SetVehicleSoundsVolume(int(double(pMenuItem->m_nValue) / 100.0 * 255.0));
          }
        }
        // Close menu
        pMenuItem->m_bOpen = false;
        BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), 
                                     &CPakoon1View::OnKeyDownCurrentMenu);
        //Invalidate();
      }
    }
  //} else if(BGame::m_pMenuCurrent->m_type == BMenu::TType::MAIN) {
  } else if(BGame::m_pMenuCurrent->m_type == BMenu::MAIN) {

    string sTmp;
    int nSelected = BGame::m_pMenuCurrent->m_listMenu.GetSelectedItem(sTmp);

    switch(nSelected) {
      case 0: // START GAME
        BGame::m_pMenuPrevious = BGame::m_pMenuCurrent;
        BGame::m_pMenuCurrent = &(BGame::m_menuChooseScene);
        BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), 
                                     &CPakoon1View::OnKeyDownCurrentMenu);
        StartMenuScroll(SCROLL_RIGHT);
        //Invalidate();
        break;
      case 1: // SETTINGS
        BGame::m_bSettingsFromGame = false;
        BGame::m_pMenuPrevious = BGame::m_pMenuCurrent;
        BGame::m_pMenuCurrent = &(BGame::m_menuSettings);
        BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), 
                                     &CPakoon1View::OnKeyDownCurrentMenu);
        StartMenuScroll(SCROLL_LEFT);
        //Invalidate();
        break;
      case 2: // THE SAGA OF PAKOON
        BGame::m_pMenuPrevious = BGame::m_pMenuCurrent;
        BGame::m_pMenuCurrent = &(BGame::m_menuSaga);
        BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), 
                                     &CPakoon1View::OnKeyDownCurrentMenu);
        StartMenuScroll(SCROLL_UP);
        //Invalidate();
        break;
      case 3: // CREDITS
        BGame::m_pMenuPrevious = BGame::m_pMenuCurrent;
        BGame::m_pMenuCurrent = &(BGame::m_menuCredits);
        BGame::m_menuCredits.m_clockStarted = SDL_GetTicks();
        BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), 
                                     &CPakoon1View::OnKeyDownCurrentMenu);
        StartMenuScroll(SCROLL_DOWN);
        //Invalidate();
        break;
      case 4: // EXIT
        // Exit
        SoundModule::StopMenuMusic();

        // Write settings for graphics etc.
        //BGame::GetPlayer()->SaveStateFile();
        //Settings::WriteSettings(m_game.GetSimulation());

        //AfxGetMainWnd()->PostMessage(WM_COMMAND, ID_FILE_CLOSE, 0);
        setExit();
        break;
    }
  //} else if(BGame::m_pMenuCurrent->m_type == BMenu::TType::CHOOSE_SCENE) {
  } else if(BGame::m_pMenuCurrent->m_type == BMenu::CHOOSE_SCENE) {
    // Load Scene
    string sTmp;
    int nSelected = BGame::m_pMenuCurrent->m_listMenu.GetSelectedItem(sTmp);
    BGame::m_sScene = BGame::m_pMenuCurrent->m_items[nSelected].m_sAssocFile;

    BGame::m_pMenuPrevious = BGame::m_pMenuCurrent;
    BGame::m_pMenuCurrent = &(BGame::m_menuChooseVehicle);
    BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), 
                                 &CPakoon1View::OnKeyDownCurrentMenu);
    StartMenuScroll(SCROLL_RIGHT);
    //Invalidate();
  //} else if(BGame::m_pMenuCurrent->m_type == BMenu::TType::CHOOSE_VEHICLE) {
  } else if(BGame::m_pMenuCurrent->m_type == BMenu::CHOOSE_VEHICLE) {

    string sTmp;
    int nSelected = BGame::m_pMenuCurrent->m_listMenu.GetSelectedItem(sTmp);
    BGame::m_sVehicle = BGame::m_pMenuCurrent->m_items[nSelected].m_sAssocFile;

    if(BGame::m_bBuyingVehicle) {
      // Check whether user wants to buy the selected vehicle
      int nYesNo = BGame::m_listYesNo.GetSelectedItem(sTmp);
      if(nYesNo == 0) {
        // Buy        
        BGame::GetPlayer()->m_sValidVehicles += (">" + BGame::m_pMenuCurrent->m_items[nSelected].m_sText + "<");
        BGame::GetPlayer()->m_dCash -= BGame::m_dPurchasePrice;
        BGame::GetPlayer()->SaveStateFile();
      }
      BGame::m_bBuyingVehicle = false;
      BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), 
                                   &CPakoon1View::OnKeyDownCurrentMenu);
    } else if(BGame::m_bCannotBuyVehicle) {
      // exit cannot buy message
      BGame::m_bCannotBuyVehicle = false;
      BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), 
                                   &CPakoon1View::OnKeyDownCurrentMenu);
    } else if(BGame::m_pMenuCurrent->m_items[nSelected].m_bDisabled) {
      double dPrice = 100.0;
      //if(BGame::m_pMenuCurrent->m_items[nSelected].m_sText.CompareNoCase("Spirit") == 0) {
      if(BGame::m_pMenuCurrent->m_items[nSelected].m_sText.compare("Spirit") == 0) {
        dPrice = 500.0;
      //} else if(BGame::m_pMenuCurrent->m_items[nSelected].m_sText.CompareNoCase("Veyronette") == 0) {
      } else if(BGame::m_pMenuCurrent->m_items[nSelected].m_sText.compare("Veyronette") == 0) {
        dPrice = 300.0;
      }
      BGame::m_dPurchasePrice = dPrice;
      if(BGame::GetPlayer()->m_dCash >= dPrice) {
        BUI::StartUsingSelectionList(&(BGame::m_listYesNo), &CPakoon1View::OnKeyDownCurrentMenu);
        BGame::m_bBuyingVehicle = true;
      } else {
        BUI::StartUsingSelectionList(&(BGame::m_listOK), &CPakoon1View::OnKeyDownCurrentMenu);
        BGame::m_bCannotBuyVehicle = true;
      }
    } else {
      // Load Vehicle
      // Proceed to PreCaching Terrain menu
      BGame::SetProgressPos(0);
      BGame::m_pMenuPrevious = BGame::m_pMenuCurrent;
      BGame::m_pMenuCurrent = &(BGame::m_menuPrecachingTerrain);
      StartMenuScroll(SCROLL_RIGHT);
    }
    //Invalidate();
  //} else if((BGame::m_pMenuCurrent->m_type == BMenu::TType::SAGA) || 
  } else if((BGame::m_pMenuCurrent->m_type == BMenu::SAGA) || 
            //(BGame::m_pMenuCurrent->m_type == BMenu::TType::CREDITS)) {
            (BGame::m_pMenuCurrent->m_type == BMenu::CREDITS)) {
    // "Do nothing"
    BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), 
                                 &CPakoon1View::OnKeyDownCurrentMenu);
    //Invalidate();
  //} else if(BGame::m_pMenuCurrent->m_type == BMenu::TType::PRECACHING_TERRAIN) {
  } else if(BGame::m_pMenuCurrent->m_type == BMenu::PRECACHING_TERRAIN) {
    // START GAME

    BGame::m_bMenuMode = false;

    m_pDrawFunction = &CPakoon1View::OnDrawGame;
    m_pKeyDownFunction = &CPakoon1View::OnKeyDownGame;
    m_game.m_bFadingIn = true;
    m_game.m_clockFadeStart = SDL_GetTicks();
    //ShowCursor(FALSE);
    SDL_ShowCursor(0);
    //Invalidate();
  }
}


//*************************************************************************************************
void CPakoon1View::CancelPressedOnCurrentMenu() {
  //if(BGame::m_pMenuCurrent->m_type == BMenu::TType::MAIN) {
  if(BGame::m_pMenuCurrent->m_type == BMenu::MAIN) {
    // NOT YET DONE, JUST RETURN TO MAIN MENU!!!
    // Setup for first menu
    BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), 
                                 &CPakoon1View::OnKeyDownCurrentMenu);
    // Exit
    //SoundModule::StopMenuMusic();

    // Write settings for graphics etc.
    //Settings::WriteSettings(m_game.GetSimulation());

    //AfxGetMainWnd()->PostMessage(WM_COMMAND, ID_FILE_CLOSE, 0);
  //} else if(BGame::m_pMenuCurrent->m_type == BMenu::TType::SETTINGS) {
  } else if(BGame::m_pMenuCurrent->m_type == BMenu::SETTINGS) {
    // See if we need to open/close sublist/slider
    BMenu *pMenu = BGame::m_pMenuCurrent;
    string sTmp;
    int nSelected = pMenu->m_listMenu.GetSelectedItem(sTmp);
    if(nSelected != -1) {
      BMenuItem *pMenuItem = &(pMenu->m_items[nSelected]);
      if(pMenuItem->m_bOpen) {
        // Close menu
        pMenuItem->m_bOpen = false;
        BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), 
                                     &CPakoon1View::OnKeyDownCurrentMenu);
        //Invalidate();
      } else {

        // Save settings
        string sTmp;
        //sscanf(LPCTSTR(pMenu->m_items[0].m_sAssocListItems[pMenu->m_items[0].m_nValue]), "%d*%d", &(BGame::m_nDispWidth), &(BGame::m_nDispHeight));
        //sscanf(pMenu->m_items[0].m_sAssocListItems[pMenu->m_items[0].m_nValue].c_str(), "%d*%d", &(BGame::m_nDispWidth), &(BGame::m_nDispHeight)); //we use only desktop resolution now
        //sscanf(LPCTSTR(pMenu->m_items[1].m_sAssocListItems[pMenu->m_items[1].m_nValue]), "%d", &(BGame::m_nDispBits));
        sscanf(pMenu->m_items[1].m_sAssocListItems[pMenu->m_items[1].m_nValue].c_str(), "%d", &(BGame::m_nDispBits));
        //sscanf(LPCTSTR(pMenu->m_items[2].m_sAssocListItems[pMenu->m_items[2].m_nValue]), "%d", &(BGame::m_nDispHz));
        sscanf(pMenu->m_items[2].m_sAssocListItems[pMenu->m_items[2].m_nValue].c_str(), "%d", &(BGame::m_nDispHz));
        BGame::m_nTerrainResolution = pMenu->m_items[3].m_nValue;
        BGame::m_nDustAndClouds = pMenu->m_items[4].m_nValue;
        BGame::m_nWaterSurface = pMenu->m_items[5].m_nValue;
        BGame::m_nMusicVolume = pMenu->m_items[6].m_nValue;
        BGame::m_nVehicleVolume = pMenu->m_items[7].m_nValue;
        BGame::m_nSoundscape = pMenu->m_items[8].m_nValue;

        Settings::WriteSettings(BGame::GetSimulation());

        switch(BGame::m_nTerrainResolution) {
          case 0: m_game.GetSimulation()->GetTerrain()->SetRenderResolution(BTerrain::RENDER_MINIMUM); break;
          case 1: m_game.GetSimulation()->GetTerrain()->SetRenderResolution(BTerrain::RENDER_LOW); break;
          case 2: m_game.GetSimulation()->GetTerrain()->SetRenderResolution(BTerrain::RENDER_MEDIUM); break;
          case 3: m_game.GetSimulation()->GetTerrain()->SetRenderResolution(BTerrain::RENDER_HIGH); break;
          case 4: m_game.GetSimulation()->GetTerrain()->SetRenderResolution(BTerrain::RENDER_MAXIMUM); break;
          case 5: m_game.GetSimulation()->GetTerrain()->SetRenderResolution(BTerrain::RENDER_SLOW_MACHINE); break;
        }

        BGame::m_bShowDust = (BGame::m_nDustAndClouds == 0);

        // See if resolution needs to be changed
        //FIXME
        /*DEVMODE devmode;
        EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devmode);
        if((devmode.dmPelsWidth  != (DWORD)BGame::m_nDispWidth) || 
           (devmode.dmPelsHeight != (DWORD)BGame::m_nDispHeight) || 
           (devmode.dmBitsPerPel != (DWORD)BGame::m_nDispBits) ||
           (devmode.dmDisplayFrequency != (DWORD)BGame::m_nDispHz)) {
          devmode.dmPelsWidth = (DWORD)BGame::m_nDispWidth;
          devmode.dmPelsHeight = (DWORD)BGame::m_nDispHeight;
          devmode.dmBitsPerPel = (DWORD)BGame::m_nDispBits;
          devmode.dmDisplayFrequency = (DWORD)BGame::m_nDispHz;
          ChangeDisplaySettings(&devmode, CDS_FULLSCREEN);
          AfxGetMainWnd()->SetWindowPos(NULL, -2, -2, BGame::m_nDispWidth + 4, BGame::m_nDispHeight + 4, 0);
        }*/

        // Go back to main menu or game menu

        if(BGame::m_bSettingsFromGame) {
          BGame::m_bMenuMode = false;
          m_pKeyDownFunction = &CPakoon1View::OnKeyDownGameMenu;
          BGame::m_bShowGameMenu = true;
          m_pDrawFunction = &CPakoon1View::OnDrawGame;
          m_game.m_bFadingIn = true;
          m_game.m_clockFadeStart = SDL_GetTicks();
          //ShowCursor(FALSE);
          SDL_ShowCursor(0);
        } else {
          StartMenuScroll(SCROLL_RIGHT);
          BGame::m_pMenuPrevious = BGame::m_pMenuCurrent;
          BGame::m_pMenuCurrent = &(BGame::m_menuMain);
          BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), 
                                       &CPakoon1View::OnKeyDownCurrentMenu);
        }
        //Invalidate();
      }
    }
  //} else if(BGame::m_pMenuCurrent->m_type == BMenu::TType::CHOOSE_SCENE) {
  } else if(BGame::m_pMenuCurrent->m_type == BMenu::CHOOSE_SCENE) {
    StartMenuScroll(SCROLL_LEFT);
    BGame::m_pMenuPrevious = BGame::m_pMenuCurrent;
    BGame::m_pMenuCurrent = &(BGame::m_menuMain);
    BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), 
                                 &CPakoon1View::OnKeyDownCurrentMenu);
    //Invalidate();
  //} else if(BGame::m_pMenuCurrent->m_type == BMenu::TType::CHOOSE_VEHICLE) {
  } else if(BGame::m_pMenuCurrent->m_type == BMenu::CHOOSE_VEHICLE) {
    if(BGame::m_bBuyingVehicle || BGame::m_bCannotBuyVehicle) {
      BGame::m_bBuyingVehicle = false;
      BGame::m_bCannotBuyVehicle = false;
      BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), 
                                   &CPakoon1View::OnKeyDownCurrentMenu);
    } else {
      StartMenuScroll(SCROLL_LEFT);
      BGame::m_pMenuPrevious = BGame::m_pMenuCurrent;
      BGame::m_pMenuCurrent = &(BGame::m_menuChooseScene);
      BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), 
                                   &CPakoon1View::OnKeyDownCurrentMenu);
    }
    //Invalidate();
  //} else if(BGame::m_pMenuCurrent->m_type == BMenu::TType::CREDITS) {
  } else if(BGame::m_pMenuCurrent->m_type == BMenu::CREDITS) {
    StartMenuScroll(SCROLL_UP);
    BGame::m_pMenuPrevious = BGame::m_pMenuCurrent;
    BGame::m_pMenuCurrent = &(BGame::m_menuMain);
    BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), 
                                 &CPakoon1View::OnKeyDownCurrentMenu);
    //Invalidate();
  //} else if(BGame::m_pMenuCurrent->m_type == BMenu::TType::SAGA) {
  } else if(BGame::m_pMenuCurrent->m_type == BMenu::SAGA) {
    StartMenuScroll(SCROLL_DOWN);
    BGame::m_pMenuPrevious = BGame::m_pMenuCurrent;
    BGame::m_pMenuCurrent = &(BGame::m_menuMain);
    BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), 
                                 &CPakoon1View::OnKeyDownCurrentMenu);
    //Invalidate();
  }
}


//*************************************************************************************************
void CPakoon1View::StartMenuScroll(TMenuScroll scroll) {
  m_clockMenuScroll = SDL_GetTicks();
  m_scrollDir = scroll;
}











double g_dRate = 30.0;
double g_d10LastFPS[10];
double g_dAveRate = 30.0;
extern double g_dExtraAlpha;

//*************************************************************************************************
//void CPakoon1View::OnDrawGame(CDC* pDC) {
void CPakoon1View::OnDrawGame() {
  static int nFrameNo = 0;
  static unsigned clockLastCheckPoint = SDL_GetTicks();
  static unsigned clockLastCheckPoint2 = SDL_GetTicks();
  //static string sRate = _T("");
  static string sRate = "";

  BCamera *pCamera = m_game.GetSimulation()->GetCamera();
  pCamera->m_vSpeed = pCamera->m_vLocation - pCamera->m_vPrevLocation;
  pCamera->m_vPrevLocation = pCamera->m_vLocation;

  BGame::UpdateEarthquake();
  BGame::UpdateWindParticles();
  BGame::UpdateAnalyzer();

  if(m_bInitClock) {
    clockLastCheckPoint = SDL_GetTicks();
    m_bInitClock = false;
  }

  /*HDC hDC = pDC->GetSafeHdc();
  wglMakeCurrent(hDC, m_hGLRC); 
  
  CRect rect;
  GetClientRect(&rect);*/

  // glPushAttrib(GL_ENABLE_BIT | GL_LIGHTING_BIT | GL_FOG_BIT | GL_DEPTH_BUFFER_BIT | GL_CURRENT_BIT | GL_COLOR_BUFFER_BIT | GL_POLYGON_BIT | GL_STENCIL_BUFFER_BIT | GL_TEXTURE_BIT);

  glDrawBuffer(GL_BACK);

  // Reset OpenGL
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glScaled(-1.0, 1.0, 1.0);

  double dScreenFormat = 1.0;
  if(m_game.m_nScreenFormat == 1) {
    dScreenFormat = 2.0 / 3.0;
  }
  double aspect = 1.0;
  //aspect = (double) rect.Width() / (double) (rect.Height() * dScreenFormat);
  aspect = (double) window_width / (double) (window_height * dScreenFormat);

  if(BGame::m_bJumpToHome) {
    double dJumpStep = 200.0;
    // Go towards home location. If close enough, return to normal simulation
    BVector vFrom = BGame::GetSimulation()->GetVehicle()->m_vLocation;
    BVector vTo = BGame::GetSimulation()->GetScene()->m_vStartLocation;
    if((vFrom - vTo).Length() < dJumpStep) {
      // Return to normal simulation
      BGame::m_bJumpToHome = false;
      //BGame::GetSimulation()->GetVehicle()->Move(vTo - vFrom);
      BVector val_bvector(vTo - vFrom);
      BGame::GetSimulation()->GetVehicle()->Move(val_bvector);
      BGame::GetSimulation()->UpdateCar();
      BGame::GetSimulation()->EnsureVehicleIsOverGround();
      //BGame::GetSimulation()->GetCamera()->m_locMode = BCamera::TCameraLoc::FOLLOW;
      BGame::GetSimulation()->GetCamera()->m_locMode = BCamera::FOLLOW;
      m_nMenuTime += BGame::ContinueSimulation();
    } else {
      // Jump towards home
      BVector vJump = vTo - vFrom;
      vJump.ToUnitLength();
      vJump = vJump * dJumpStep;
      // BGame::GetSimulation()->GetVehicle()->m_vLocation += vJump;
      BGame::GetSimulation()->GetVehicle()->Move(vJump);
      BGame::GetSimulation()->UpdateCar();
      vJump.ToUnitLength();
      BCamera *pCam = BGame::GetSimulation()->GetCamera();
      pCam->m_orientation.m_vForward = vJump;
      pCam->m_orientation.m_vUp = BVector(0, 0, -1);
      pCam->m_orientation.m_vRight = pCam->m_orientation.m_vUp.CrossProduct(pCam->m_orientation.m_vForward);
    }
  }

  if(!m_game.m_bFrozen) {
    //m_game.GetSimulation()->PrePaint(pDC);
    m_game.GetSimulation()->PrePaint();
  }

  static unsigned clockLastCrash = SDL_GetTicks();
  if(BGame::GetSimulation()->m_dMaxGForce > 1.0) {
    //if(BGame::GetState() == BGame::TState::DELIVERY) {
    if(BGame::GetState() == BGame::DELIVERY) {
      if((double(SDL_GetTicks() - clockLastCrash) / 1000.0) > 1.0) {
        clockLastCrash = SDL_GetTicks();
        BMessages::Show(50, "pizzadamage", "Pizza damaged! profit penalty -10%", 3, false, 1, 0, 0);
        ++(BGame::m_nPizzaDamaged);
      }
    } else {
      BMessages::Show(40, "damage", "Relax, man!", 1, false, 1, 0, 0);
    }
  }

  // double dScaler = 10.0; // To get better z-buffer performance on a Geforce MX2 card

  if(pCamera->m_locMode == BCamera::INCAR) {
    gluPerspective(pCamera->m_dAngleOfView, aspect, 1.0f, float(cdWorldHemisphereRadius * 1.5));
  } else if(pCamera->m_locMode == BCamera::OVERVIEW) {
    gluPerspective(pCamera->m_dAngleOfView, aspect, 10.0f, float(cdWorldHemisphereRadius * 1.5));
  } else if(pCamera->m_locMode == BCamera::ONSIDE) {
    gluPerspective(pCamera->m_dAngleOfView, aspect, 0.2f, float(cdWorldHemisphereRadius * 1.5));
  } else {
    gluPerspective(pCamera->m_dAngleOfView, aspect, 1.0f, float(cdWorldHemisphereRadius * 1.5));
  }

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // Set look at -point for camera 
  glViewport(0, 
             //(GLint) ((rect.Height() - rect.Height() * dScreenFormat) / 2), 
             (GLint) ((window_height - window_height * dScreenFormat) / 2), 
             //(GLint) rect.Width(), 
             (GLint) window_width, 
             //(GLint) (double(rect.Height()) * dScreenFormat));
             (GLint) (double(window_height) * dScreenFormat));

  static double dSortaClock = 0.0;
  //static clock_t clockPrev = clock();
  static unsigned clockPrev = SDL_GetTicks();
  //clock_t clockNow = clock();
  unsigned clockNow = SDL_GetTicks();
  dSortaClock += 0.001 * double(clockNow - clockPrev);
  clockPrev = clockNow;
  double dVert;
  double dHoriz;
  if(pCamera->m_locMode != BCamera::INCAR) {
    dVert = 0.125 * sin(dSortaClock) - 0.125 / 2.0;
    dHoriz = 0.125 * sin(dSortaClock / 3.0) - 0.125 / 2.0;
  } else {
    dVert = 0.0;
    dHoriz = 0.0;
  }

  if(!BGame::GetSceneEditor()->IsActive()) {
    //m_game.GetSimulation()->SetUpCamera(&rect);
    m_game.GetSimulation()->SetUpCamera();
  } else {
    BVector vVehicleTo = pCamera->m_vLocation + pCamera->m_orientation.m_vForward * 5.0;
    BVector vVehicleLoc = m_game.GetSimulation()->GetVehicle()->m_vLocation;
    //m_game.GetSimulation()->GetVehicle()->Move(vVehicleTo - vVehicleLoc);
    BVector val_bvector(vVehicleTo - vVehicleLoc);
    m_game.GetSimulation()->GetVehicle()->Move(val_bvector);
    m_game.GetSimulation()->UpdateCar();
    m_game.GetSimulation()->GetVehicle()->m_orientation = pCamera->m_orientation;

    // Ensure camera is above ground
    double dTmp;
    BVector vInternalOffset = BGame::GetSimulation()->GetTerrain()->m_vOffset;
    double dZ = -HeightMap::CalcHeightAt(vInternalOffset.m_dX + pCamera->m_vLocation.m_dX, 
                                         vInternalOffset.m_dY + pCamera->m_vLocation.m_dY, 
                                         dTmp, 
                                         HeightMap::OVERALL);
    if(pCamera->m_vLocation.m_dZ > (dZ - 1.0)) {
      pCamera->m_vLocation.m_dZ = (dZ - 1.0);
    }
  }

  gluLookAt(pCamera->m_vLocation.m_dX + dHoriz, 
            pCamera->m_vLocation.m_dY, 
            pCamera->m_vLocation.m_dZ + dVert,
            pCamera->m_vLocation.m_dX + pCamera->m_orientation.m_vForward.m_dX + dHoriz,
            pCamera->m_vLocation.m_dY + pCamera->m_orientation.m_vForward.m_dY, 
            pCamera->m_vLocation.m_dZ + pCamera->m_orientation.m_vForward.m_dZ + dVert, 
            pCamera->m_orientation.m_vUp.m_dX, 
            pCamera->m_orientation.m_vUp.m_dY, 
            pCamera->m_orientation.m_vUp.m_dZ);

  GLfloat fLight1PositionG[ 4];
  fLight1PositionG[0] = (GLfloat) 0.25;
  fLight1PositionG[1] = (GLfloat) 0;
  fLight1PositionG[2] = (GLfloat) -1;
  fLight1PositionG[3] = (GLfloat) 0; /* w=0 -> directional light (not positional) */
  glLightfv( GL_LIGHT0, GL_POSITION, fLight1PositionG);                     

  // Draw the world

  glEnable(GL_DEPTH_TEST);
  //m_nMenuTime += m_game.GetSimulation()->Paint(pDC, m_bCreateDLs, m_bWireframe, m_bNormals, rect);
  m_nMenuTime += m_game.GetSimulation()->Paint(m_bCreateDLs, m_bWireframe, m_bNormals);
  m_bCreateDLs = false;

  // Draw video on top of car
  if(m_bShowVideo) {
    DrawVideo();
  }

  // Scene editor stuff (active object highlight)
  if(m_game.GetSceneEditor()->IsActive()) {
    m_game.GetSceneEditor()->HighlightActiveObject();
  }

  // Find out the screen position for each tracking target
  if(BGame::m_bDrawOnScreenTracking) {
    BTrackingTarget *pTarget = m_game.GetSimulation()->m_targets;
    while(pTarget) {
      //UpdateTrackingTargetScreenPos(pTarget, m_rectWnd);
      UpdateTrackingTargetScreenPos(pTarget);
      pTarget = pTarget->m_pNext;
    }
  }

  // Update tracking of closest gas station
  BGame::GetSimulation()->UpdateGasStationTracking();

  // Check if we need to start fueling
  CheckForFueling();

  // Check if we need to start pickup or delivery
  //if(BGame::GetState() == BGame::TState::PICKUP) {
  if(BGame::GetState() == BGame::PICKUP) {
    CheckForPickup();
  //} else if(BGame::GetState() == BGame::TState::DELIVERY) {
  } else if(BGame::GetState() == BGame::DELIVERY) {
    CheckForDelivery();
    // Update pizza temp
    unsigned clockNow = SDL_GetTicks();
    double dElapsed = double(clockNow - BGame::m_clockLastPizzaTempCheck) / 1000.0 * 0.21;
    double dScaler = (BGame::m_dPizzaTemp - 26.2) / (60.0 - 26.2);
    BGame::m_clockLastPizzaTempCheck = clockNow;
    BGame::m_dPizzaTemp -= (dElapsed * dScaler);
  }

  // Draw 2D graphics
  //if(BGame::m_nVisualize & BGame::TAnalyzerVis::GASSTATIONS) {
  if(BGame::m_nVisualize & BGame::GASSTATIONS) {
    Setup2DRendering();

    // If under water, draw faint water veil
    if(m_game.GetSimulation()->GetCamera()->m_vLocation.m_dZ > 0.0) {
      glDisable(GL_TEXTURE_2D);
      OpenGLHelpers::SetColorFull(0.25, 0.35, 0.5, 0.8);
      glBegin(GL_TRIANGLE_STRIP);
      glVertex3f(0, 0, 0);
      //glVertex3f(0, m_rectWnd.Height(), 0);
      glVertex3f(0, window_height, 0);
      //glVertex3f(m_rectWnd.Width(), 0, 0);
      glVertex3f(window_width, 0, 0);
      //glVertex3f(m_rectWnd.Width(), m_rectWnd.Height(), 0);
      glVertex3f(window_width, window_height, 0);
      glEnd();
    }

    // Draw tracking targets on screen.
    if(BGame::m_bDrawOnScreenTracking) {
      BTrackingTarget *pTarget = m_game.GetSimulation()->m_targets;
      while(pTarget) {
        //DrawTrackingTargetOnScreen(pTarget, m_rectWnd);
        DrawTrackingTargetOnScreen(pTarget);
        pTarget = pTarget->m_pNext;
      }
    }

    if((BGame::m_bShowDust) && (g_dExtraAlpha > 0.0)) {
      // If inside cloud, draw veil
      OpenGLHelpers::SetColorFull(1, 1, 1, 0.7 * g_dExtraAlpha);
      glDisable(GL_TEXTURE_2D);

      glBegin(GL_TRIANGLE_STRIP);
      glVertex3f(0, 0, 0);
      //glVertex3f(0, rect.Height(), 0);
      glVertex3f(0, window_height, 0);
      //glVertex3f(rect.Width(), 0, 0);
      glVertex3f(window_width, 0, 0);
      //glVertex3f(rect.Width(), rect.Height(), 0);
      glVertex3f(window_width, window_height, 0);
      glEnd();
    }

    // Draw panels if they're on
    if(BGame::m_bDashboard) {
      //DrawDashboard(pDC);
      DrawDashboard();
    }
    if(BGame::m_bService) {
      //DrawServiceWnd(pDC);
      DrawServiceWnd();
    }
    if(BGame::m_bNavSat) {
      //DrawNavSat(pDC);
      DrawNavSat();
      if(!BGame::m_bMultiProcessor) {
        //Sleep(1);
        SDL_Delay(1);
      }
    }

    // Scene editor stuff
    if(m_game.GetSceneEditor()->IsActive()) {
      //m_game.GetSceneEditor()->Draw(m_rectWnd);
      SDL_Rect rectWnd;
      rectWnd.w = window_width;
      rectWnd.h = window_height;
      m_game.GetSceneEditor()->Draw(rectWnd);
    }

    // If something requires attention, draw a faint veil
    if(m_game.m_bDeliveryStartInProgress || 
       m_game.m_bPickupStartInProgress || 
       m_game.m_bShowGameMenu || 
       m_game.m_bShowQuickHelp || 
       m_game.m_bShowCancelQuestion || 
       m_game.m_bFueling) {
      //OpenGLHelpers::DrawVeil(0.6, 0.6, 1, 0.5, m_rectWnd);
      SDL_Rect rectWnd;
      rectWnd.w = window_width;
      rectWnd.h = window_height;
      OpenGLHelpers::DrawVeil(0.6, 0.6, 1, 0.5, rectWnd);
    }

    if(BGame::m_bPickupStartInProgress) {
      //DrawPickupInfo(pDC, rect);
      DrawPickupInfo();
      if(((SDL_GetTicks() - BGame::m_clockPickupStart) / 1000.0) > 5) {
        m_nMenuTime += BGame::ContinueSimulation();
        BGame::m_bPickupStartInProgress = false;
        BMessages::Show(50, "deliver", "DELIVER PIZZA TO CLIENT", 8, true);
        BGame::m_nPizzaDamaged = 0;
      }
    }

    if(BGame::m_bDeliveryStartInProgress) {
      //DrawDeliveryInfo(pDC, rect);
      DrawDeliveryInfo();
      if(((SDL_GetTicks() - BGame::m_clockDeliveryStart) / 1000.0) > 5) {
        m_nMenuTime += BGame::ContinueSimulation();
        BGame::m_bDeliveryStartInProgress = false;
        BMessages::Show(50, "pickup", "RETURN TO BASE", 8, true);
      }
    }

    // If fueling, draw gas station buttons
    if(BGame::m_bFueling) {
      //DrawFuelingButtons(pDC, rect);
      DrawFuelingButtons();
    }

    if(BGame::m_bShowQuickHelp) {
      //DrawQuickHelp(pDC);
      DrawQuickHelp();
    }

    if(m_game.m_bShowCancelQuestion) {
      // Cancel order question 
      glEnable(GL_TEXTURE_2D);
      OpenGLHelpers::SwitchToTexture(0);
      //BTextures::Use(BTextures::Texture::PANEL);
      BTextures::Use(BTextures::PANEL);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

      glPushMatrix();
      //glTranslated(m_rectWnd.Width() / 2.0 - 235.0 / 2.0, m_rectWnd.Height() / 2.0 - 50, 0);
      glTranslated(window_width / 2.0 - 235.0 / 2.0, window_height / 2.0 - 50, 0);

      OpenGLHelpers::SetColorFull(1, 0.25, 0.25, 1);

      glBegin(GL_TRIANGLE_STRIP);
      OpenGLHelpers::SetTexCoord(138.0 / 512.0, (512.0 - 259.0) / 512.0);    glVertex3f(0, 0, 0);
      OpenGLHelpers::SetTexCoord(138.0 / 512.0, (512.0 - 149.0) / 512.0);    glVertex3f(0, 110.0, 0);
      OpenGLHelpers::SetTexCoord(373.0 / 512.0, (512.0 - 259.0) / 512.0);    glVertex3f(235.0, 0, 0);
      OpenGLHelpers::SetTexCoord(373.0 / 512.0, (512.0 - 149.0) / 512.0);    glVertex3f(235.0, 110.0, 0);
      glEnd();

      glTranslated(70.0, -50.0, 0);

      // Draw Yeah and Nope (highlight selected)
      if(m_game.m_nYesNoSelection == 1) {
        OpenGLHelpers::SetColorFull(0, 0.5, 0, 1);
      } else {
        OpenGLHelpers::SetColorFull(0, 0.5, 0, 0.25);
      }

      glBegin(GL_TRIANGLE_STRIP);
      OpenGLHelpers::SetTexCoord(166.0 / 512.0, (512.0 - 308.0) / 512.0);    glVertex3f(0 - 40.0, 0, 0);
      OpenGLHelpers::SetTexCoord(166.0 / 512.0, (512.0 - 271.0) / 512.0);    glVertex3f(0 - 40.0, 37.0, 0);
      OpenGLHelpers::SetTexCoord(246.0 / 512.0, (512.0 - 308.0) / 512.0);    glVertex3f(80.0 - 40.0, 0, 0);
      OpenGLHelpers::SetTexCoord(246.0 / 512.0, (512.0 - 271.0) / 512.0);    glVertex3f(80.0 - 40.0, 37.0, 0);
      glEnd();

      if(m_game.m_nYesNoSelection == 2) {
        OpenGLHelpers::SetColorFull(1, 0.25, 0.25, 1);
      } else {
        OpenGLHelpers::SetColorFull(1, 0.25, 0.25, 0.25);
      }

      glBegin(GL_TRIANGLE_STRIP);
      OpenGLHelpers::SetTexCoord(252.0 / 512.0, (512.0 - 315.0) / 512.0);    glVertex3f(0 + 40.0, -7, 0);
      OpenGLHelpers::SetTexCoord(252.0 / 512.0, (512.0 - 271.0) / 512.0);    glVertex3f(0 + 40.0, 37.0, 0);
      OpenGLHelpers::SetTexCoord(338.0 / 512.0, (512.0 - 315.0) / 512.0);    glVertex3f(86.0 + 40.0, -7, 0);
      OpenGLHelpers::SetTexCoord(338.0 / 512.0, (512.0 - 271.0) / 512.0);    glVertex3f(86.0 + 40.0, 37.0, 0);
      glEnd();

      glPopMatrix();
    }

    if(m_game.m_bShowGameMenu) {
      // Main menu
      glEnable(GL_TEXTURE_2D);
      OpenGLHelpers::SwitchToTexture(0);
      //BTextures::Use(BTextures::Texture::MAIN_GAME_MENU);
      BTextures::Use(BTextures::MAIN_GAME_MENU);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

      glPushMatrix();
      //glTranslated(m_rectWnd.Width() / 2.0 - 128, m_rectWnd.Height() / 2.0 - 128, 0);
      glTranslated(window_width / 2.0 - 128, window_height / 2.0 - 128, 0);

      double dHeight = 256.0 / 5.0;

      if(BGame::m_nGameMenuSelection == 1) {
        OpenGLHelpers::SetColorFull(1, 1, 1, 1);
      } else {
        OpenGLHelpers::SetColorFull(1, 1, 1, 0.25);
      }

      glBegin(GL_TRIANGLE_STRIP);
      OpenGLHelpers::SetTexCoord(0, 0);    glVertex3f(0, 0, 0);
      OpenGLHelpers::SetTexCoord(0, 0.2);  glVertex3f(0, dHeight, 0);
      OpenGLHelpers::SetTexCoord(1, 0);    glVertex3f(256, 0, 0);
      OpenGLHelpers::SetTexCoord(1, 0.2);  glVertex3f(256, dHeight, 0);
      glEnd();

      if(BGame::m_nGameMenuSelection == 2) {
        OpenGLHelpers::SetColorFull(1, 1, 1, 1);
      } else {
        OpenGLHelpers::SetColorFull(1, 1, 1, 0.25);
      }

      glBegin(GL_TRIANGLE_STRIP);
      OpenGLHelpers::SetTexCoord(0, 0.2);   glVertex3f(0, dHeight, 0);
      OpenGLHelpers::SetTexCoord(0, 0.4);   glVertex3f(0, dHeight * 2.0, 0);
      OpenGLHelpers::SetTexCoord(1, 0.2);   glVertex3f(256, dHeight, 0);
      OpenGLHelpers::SetTexCoord(1, 0.4);   glVertex3f(256, dHeight * 2.0, 0);
      glEnd();

      if(BGame::m_nGameMenuSelection == 3) {
        OpenGLHelpers::SetColorFull(1, 1, 1, 1);
      } else {
        OpenGLHelpers::SetColorFull(1, 1, 1, 0.25);
      }

      glBegin(GL_TRIANGLE_STRIP);
      OpenGLHelpers::SetTexCoord(0, 0.4);   glVertex3f(0, dHeight * 2.0, 0);
      OpenGLHelpers::SetTexCoord(0, 0.6);   glVertex3f(0, dHeight * 3.0, 0);
      OpenGLHelpers::SetTexCoord(1, 0.4);   glVertex3f(256, dHeight * 2.0, 0);
      OpenGLHelpers::SetTexCoord(1, 0.6);   glVertex3f(256, dHeight * 3.0, 0);
      glEnd();

      if(BGame::m_nGameMenuSelection == 4) {
        OpenGLHelpers::SetColorFull(1, 1, 1, 1);
      } else {
        OpenGLHelpers::SetColorFull(1, 1, 1, 0.25);
      }

      glBegin(GL_TRIANGLE_STRIP);
      OpenGLHelpers::SetTexCoord(0, 0.6);    glVertex3f(0, dHeight * 3.0, 0);
      OpenGLHelpers::SetTexCoord(0, 0.8);    glVertex3f(0, dHeight * 4.0, 0);
      OpenGLHelpers::SetTexCoord(1, 0.6);    glVertex3f(256, dHeight * 3.0, 0);
      OpenGLHelpers::SetTexCoord(1, 0.8);    glVertex3f(256, dHeight * 4.0, 0);
      glEnd();

      if(BGame::m_nGameMenuSelection == 5) {
        OpenGLHelpers::SetColorFull(1, 1, 1, 1);
      } else {
        OpenGLHelpers::SetColorFull(1, 1, 1, 0.25);
      }

      glBegin(GL_TRIANGLE_STRIP);
      OpenGLHelpers::SetTexCoord(0, 0.8);    glVertex3f(0, dHeight * 4.0, 0);
      OpenGLHelpers::SetTexCoord(0, 1.0);    glVertex3f(0, dHeight * 5.0, 0);
      OpenGLHelpers::SetTexCoord(1, 0.8);    glVertex3f(256, dHeight * 4.0, 0);
      OpenGLHelpers::SetTexCoord(1, 1.0);    glVertex3f(256, dHeight * 5.0, 0);
      glEnd();

      glPopMatrix();
    }

    // Draw hint text if it's on
    if(m_game.m_bShowHint) {
      unsigned clockNow = SDL_GetTicks();
      if((clockNow - m_game.m_clockHintStart) > 2000) {
        m_game.m_bShowHint = false;
      } else {
        double dAlpha = 1.0 - double(clockNow - m_game.m_clockHintStart) / 2000.0;

        // OpenGLHelpers::SetColorFull(0.3, 0.6, 0.8, dAlpha);
        OpenGLHelpers::SetColorFull(0.9, 0.2, 0.1, dAlpha);
        OpenGLHelpers::SwitchToTexture(0);
        BTextures::Use(BTextures::PANEL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glPushMatrix();
        //glTranslated(m_rectWnd.Width() / 2.0, m_rectWnd.Height() / 2.0, 0);
        glTranslated(window_width / 2.0, window_height / 2.0, 0);

        glBegin(GL_TRIANGLE_STRIP);
        OpenGLHelpers::SetTexCoord(0, 0);
        glVertex3f(-321.0 / 2.0, -57.0 / 2.0, 0);
        OpenGLHelpers::SetTexCoord(0, (512.0 - 455.0) / 512.0);
        glVertex3f(-321.0 / 2.0, 57.0 / 2.0, 0);
        OpenGLHelpers::SetTexCoord(321.0 / 512.0, 0);
        glVertex3f(321.0 / 2.0, -57.0 / 2.0, 0);
        OpenGLHelpers::SetTexCoord(321.0 / 512.0, (512.0 - 455.0) / 512.0);
        glVertex3f(321.0 / 2.0, 57.0 / 2.0, 0);
        glEnd();

        glPopMatrix();
      }
    }

    if(!m_game.m_bShowGameMenu && !m_game.m_bShowQuickHelp && !m_game.m_bShowCancelQuestion) {
      glPushMatrix();
      //glTranslated(m_rectWnd.Width() / 2.0, m_rectWnd.Height() * 0.35, 0);
      glTranslated(window_width / 2.0, window_height * 0.35, 0);
      m_messages.Render();
      glPopMatrix();
    }

    // Draw mouse cursor, if needed
    if(((BGame::m_bService) || (BGame::m_bNavSat)) && 
       !(m_game.m_bShowGameMenu || m_game.m_bShowQuickHelp || m_game.m_bShowCancelQuestion)) { 
      //DrawMouseCursor(m_rectWnd);
      DrawMouseCursor();
    }

    // On top of everything, draw fade in if we are in the first second
    if(m_game.m_bFadingIn) {
      double dFade = double(SDL_GetTicks() - m_game.m_clockFadeStart) / 1000.0;
      if(dFade > 1.0) {
        m_game.m_bFadingIn = false;
        BMessages::Show(50, "start game", "GO TO BASE FOR FIRST PICKUP", 5, true);
      } else {
        glDisable(GL_TEXTURE_2D);
        OpenGLHelpers::SetColorFull(0, 0, 0, 1.0 - dFade);
        glBegin(GL_TRIANGLE_STRIP);
        glVertex3f(0, 0, 0);
        //glVertex3f(0, m_rectWnd.Height(), 0);
        glVertex3f(0, window_height, 0);
        //glVertex3f(m_rectWnd.Width(), 0, 0);
        glVertex3f(window_width, 0, 0);
        //glVertex3f(m_rectWnd.Width(), m_rectWnd.Height(), 0);
        glVertex3f(window_width, window_height, 0);
        glEnd();
      }
    }

    End2DRendering();

    // Calculate framerates
    clockNow = SDL_GetTicks();
    g_d10LastFPS[nFrameNo] = double(clockNow - clockLastCheckPoint2) / 1000.0;
    if(g_d10LastFPS[nFrameNo] < 0.00001) {
      g_d10LastFPS[nFrameNo] = 999.9;
    } else {
      g_d10LastFPS[nFrameNo] = 1.0 / g_d10LastFPS[nFrameNo];
    }
    clockLastCheckPoint2 = clockNow;

    if(++nFrameNo == 10) {
      g_dRate = 10.0 / (double(clockNow - (clockLastCheckPoint + m_nMenuTime)) / 1000.0);
      g_dAveRate += g_dRate;
      if(m_nMenuTime) {
        m_nMenuTime = 0;
      }

      m_game.GetSimulation()->GetVehicle()->m_dSpeedKmh = m_game.GetSimulation()->GetVehicle()->m_dSpeed * 
                                                      g_dRate * 
                                                      m_game.GetSimulation()->m_nPhysicsStepsBetweenRender * 
                                                      3.6;

      /*sRate.Format("FPS:%.1lf, Speed:%.0lf km/h (%.2lf), SimSteps:%d, CamLoc:(%.1lf, %.1lf, %.1lf)", 
                   g_dRate, 
                   m_game.GetSimulation()->GetVehicle()->m_dSpeedKmh,
                   m_game.GetSimulation()->GetVehicle()->m_dSpeed,
                   m_game.GetSimulation()->m_nPhysicsStepsBetweenRender,
                   m_game.GetSimulation()->GetCamera()->m_vLocation.m_dX,
                   m_game.GetSimulation()->GetCamera()->m_vLocation.m_dY,
                   m_game.GetSimulation()->GetCamera()->m_vLocation.m_dZ);*/
        stringstream format;
		format << "FPS:" << g_dRate <<
			", Speed:" << m_game.GetSimulation()->GetVehicle()->m_dSpeedKmh <<
			" km/h (" << m_game.GetSimulation()->GetVehicle()->m_dSpeed <<
			"), SimSteps:" << m_game.GetSimulation()->m_nPhysicsStepsBetweenRender <<
			", CamLoc:(" << m_game.GetSimulation()->GetCamera()->m_vLocation.m_dX <<
			", " << m_game.GetSimulation()->GetCamera()->m_vLocation.m_dY <<
			", " << m_game.GetSimulation()->GetCamera()->m_vLocation.m_dZ << ")";
		sRate = format.str();

      // Calculate the absolute time sync ratio (ATSR)
      if(!m_game.m_bFrozen) {
        m_game.GetSimulation()->m_dMaxGForce = 0.0;
        static int nSkipAmount = 1;
        static int nSkipper = 0;
        if(++nSkipper >= nSkipAmount) {
          if(nSkipAmount < 1) {
            ++nSkipAmount;
          }
          nSkipper = 0;
          g_dAveRate /= double(nSkipAmount);
          m_game.GetSimulation()->m_nPhysicsStepsBetweenRender = int(g_dPhysicsStepsInSecond / g_dAveRate);
          double dFraction = (g_dPhysicsStepsInSecond / g_dRate) - double(int(g_dPhysicsStepsInSecond / g_dRate));
          m_game.GetSimulation()->m_dPhysicsFraction = dFraction; 
          m_game.GetSimulation()->m_bCalibrateSimulationSpeed  = false;
          g_dAveRate = 0;
        }
      }
  
      nFrameNo = 0;
      clockLastCheckPoint = clockNow;
    }

    if(BGame::m_bFrozen) {
      GLint nDepth;
      glGetIntegerv(GL_MODELVIEW_STACK_DEPTH , &nDepth); 

      /*sRate.Format("FPS:%.1lf, CamLoc:(%.1lf, %.1lf, %.1lf), STACK: %d",
                   g_dRate, 
                   m_game.GetSimulation()->GetCamera()->m_vLocation.m_dX,
                   m_game.GetSimulation()->GetCamera()->m_vLocation.m_dY,
                   m_game.GetSimulation()->GetCamera()->m_vLocation.m_dZ,
                   nDepth);*/
		stringstream format;
		format << "FPS:" << g_dRate <<
			", CamLoc:(" << m_game.GetSimulation()->GetCamera()->m_vLocation.m_dX <<
			", " << m_game.GetSimulation()->GetCamera()->m_vLocation.m_dY <<
			", " << m_game.GetSimulation()->GetCamera()->m_vLocation.m_dZ <<
			"), STACK: " << nDepth;
		sRate = format.str();
    }

    // Override realistic physics time when in slow motion
    if(BGame::m_bSlowMotion) {
      m_game.GetSimulation()->m_nPhysicsStepsBetweenRender = 1;
    }

    if(m_game.m_bDisplayInfo) {
      BUI::TextRenderer()->StartRenderingText();
      //BUI::TextRenderer()->DrawSmallTextAt(58, rect.Height() - 20, sRate, sRate.GetLength(), BTextRenderer::TTextAlign::ALIGN_LEFT, 0, 0, 0, 1);
      BUI::TextRenderer()->DrawSmallTextAt(58, window_height - 20, sRate, sRate.length(), BTextRenderer::ALIGN_LEFT, 0, 0, 0, 1);
      string sText = "BETA VERSION! (build 6)";
      //BUI::TextRenderer()->DrawSmallTextAt(rect.Width() / 2, rect.Height() - 40, sText, sText.GetLength(), BTextRenderer::TTextAlign::ALIGN_CENTER, 0.75, 0, 0, 1);
      BUI::TextRenderer()->DrawSmallTextAt(window_width / 2, window_height - 40, sText, sText.length(), BTextRenderer::ALIGN_CENTER, 0.75, 0, 0, 1);
      BUI::TextRenderer()->StopRenderingText();
    }
  }

  //SwapBuffers(pDC->GetSafeHdc());
  SDL_GL_SwapWindow(window);

  //::ReleaseDC(GetSafeHwnd(), hDC);

  // glPopAttrib();


  if(!m_game.GetSimulation()->m_bPaused) {
    //Invalidate();
  }
}


extern BVector g_vCenter;

//*************************************************************************************************
void CPakoon1View::DrawVideo() {
  static int nFrame = 0;
  // Setup 256x256 texture
  OpenGLHelpers::SetColorFull(0, 1, 0.5, 1);
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE);

#ifndef __EMSCRIPTEN__
  glActiveTextureARB(GL_TEXTURE0_ARB);
#endif
  glEnable(GL_TEXTURE_2D);

  // OpenGLHelpers::SwitchToTexture(0);
  BTextures::Use(g_nVideoTexture);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  // GrabAVIFrame(nFrame);
  //nFrame = (nFrame + 1) % g_nFramesInVideo; //FIXME

  // Draw quad on top of car
  BVector vCar = g_vCenter + BGame::GetSimulation()->GetVehicle()->m_orientation.m_vUp;

  BVector v1 = BGame::GetSimulation()->GetVehicle()->m_orientation.m_vRight * -1.5;
  BVector v2 = BGame::GetSimulation()->GetVehicle()->m_orientation.m_vRight * -1.5 + BGame::GetSimulation()->GetVehicle()->m_orientation.m_vUp * 2.0;
  BVector v3 = BGame::GetSimulation()->GetVehicle()->m_orientation.m_vRight *  1.5 + BGame::GetSimulation()->GetVehicle()->m_orientation.m_vUp * 2.0;
  BVector v4 = BGame::GetSimulation()->GetVehicle()->m_orientation.m_vRight *  1.5;

  glPushMatrix();
  glTranslatef(vCar.m_dX, vCar.m_dY, vCar.m_dZ);

  glBegin(GL_QUAD_STRIP);
  for(int i = 0; i < 100; ++i) {
    if(i % 2) {
      OpenGLHelpers::SetColorFull(1, 1, 1, 1);
    } else {
      OpenGLHelpers::SetColorFull(0.5, 0.5, 0.5, 1);
    }
    OpenGLHelpers::SetTexCoord(0, double(i) * 0.01);
    BVector vNew1 = v1 + BGame::GetSimulation()->GetVehicle()->m_orientation.m_vUp * 2.0 * (double(i) * 0.01);
    glVertex3f(vNew1.m_dX, vNew1.m_dY, vNew1.m_dZ);
    OpenGLHelpers::SetTexCoord(1, double(i) * 0.01);
    vNew1 = v4 + BGame::GetSimulation()->GetVehicle()->m_orientation.m_vUp * 2.0 * (double(i) * 0.01);
    glVertex3f(vNew1.m_dX, vNew1.m_dY, vNew1.m_dZ);
  }
  glEnd();
  glPopMatrix();

  /*
  OpenGLHelpers::SetTexCoord(0, 0);
  glVertex3f(v1.m_dX, v1.m_dY, v1.m_dZ);
  OpenGLHelpers::SetTexCoord(0, 1);
  glVertex3f(v2.m_dX, v2.m_dY, v2.m_dZ);
  OpenGLHelpers::SetTexCoord(1, 1);
  glVertex3f(v3.m_dX, v3.m_dY, v3.m_dZ);
  OpenGLHelpers::SetTexCoord(1, 0);
  glVertex3f(v4.m_dX, v4.m_dY, v4.m_dZ);
  */

  // cleanup
  glDisable(GL_BLEND);
  glDisable(GL_TEXTURE_2D);
}



//*************************************************************************************************
void CPakoon1View::CheckForFueling() {
  // Check if we need to start fueling
  BVehicle *pVehicle = BGame::GetSimulation()->GetVehicle();
  BVector vFuel = pVehicle->ToWorldCoord(pVehicle->m_vFuelLocation);
  if(((vFuel - BGame::m_vGasStationClosest).Length() < pVehicle->m_dFuelDistance) && 
     !m_game.m_bFueling && 
     !BGame::m_bShowCancelQuestion && 
     !BGame::m_bShowGameMenu && 
     !BGame::m_bShowQuickHelp) {
    if((SDL_GetTicks() - BGame::m_clockLastFuelExit) > 3000) {
      // Start fueling
      BGame::FreezeSimulation();
      m_pKeyDownFunction = &CPakoon1View::OnKeyDownFueling;
      m_game.m_bFueling = true;
    }
  }
}


//*************************************************************************************************
void CPakoon1View::CheckForPickup() {
  // Check if we need to pickup
  BVehicle *pVehicle = BGame::GetSimulation()->GetVehicle();
  BVector v = pVehicle->ToWorldCoord(pVehicle->m_vFuelLocation);

  BObject *pBase = BGame::GetBase();
  if(pBase && (((v - pBase->m_vCenter).Length() < pBase->m_dActiveRadius) && 
                 !BGame::m_bShowCancelQuestion && 
                 !BGame::m_bShowGameMenu && 
                 !BGame::m_bShowQuickHelp)) {

    // When in base, always fix rotors (for 10$ a part)
    BVehicle *pVehicle = BGame::GetSimulation()->GetVehicle();
    if(pVehicle->m_bHasRotor) {
      if(!(pVehicle->m_rotor.m_bHeliOK)) {
        pVehicle->m_rotor.m_bHeliOK = true;
        BVehicle *pVehicle = BGame::GetSimulation()->GetVehicle();
        pVehicle->m_rotor.m_dHeliBladePower = 1.0;
        double dCost = 0.0;
        if(!(pVehicle->m_rotor.m_bHeliCoverOK)) {
          pVehicle->m_rotor.m_bHeliCoverOK = true;
          dCost += 10.0;
        }
        for(int i = 0; i < 3; ++i) {
          if(!(pVehicle->m_rotor.m_bHeliBladeOK[i])) {
            pVehicle->m_rotor.m_bHeliBladeOK[i] = true;
            dCost += 10.0;
          }
        }
        BGame::GetPlayer()->m_dCash -= dCost;
        string sTmp;
        //sTmp.Format("Rotor fixed. Cost %.0lf$", dCost);
		stringstream format;
		format << "Rotor fixed. Cost " << (int) dCost << "$";
		sTmp = format.str();
        BMessages::Show(40, "rotorfixed", sTmp, 3, false, 0.2, 0.5, 0.2);
      }
    }

    // Start pickup
    BGame::m_nPizzaDamaged = 0.0;
    BMessages::Remove("start game");
    BGame::FreezeSimulation();
    BGame::m_bPickupStartInProgress = true;
    BGame::m_clockPickupStart = SDL_GetTicks();
    //BGame::SetState(BGame::TState::DELIVERY);
    BGame::SetState(BGame::DELIVERY);
    BGame::m_dPizzaTemp = 60.0 + Random(5.0);
    BGame::m_nPizza = rand() % BGame::m_nPizzas;

    // Select client
    bool bFound = false;
    BScene *pScene = BGame::GetSimulation()->GetScene();
    if(!pScene->m_bRandomDeliveryInUse) {
      if(pScene->m_nCurrentDeliveryEntry < pScene->m_nDeliveryOrderEntries) {
        if((pScene->m_nDeliveryOrder[pScene->m_nCurrentDeliveryEntry] < pScene->m_nObjects) &&
          //pScene->m_pObjects[pScene->m_nDeliveryOrder[pScene->m_nCurrentDeliveryEntry]].m_type == BObject::TType::CLIENT) {
          pScene->m_pObjects[pScene->m_nDeliveryOrder[pScene->m_nCurrentDeliveryEntry]].m_type == BObject::CLIENT) {
          bFound = true;
          BGame::SetCurrentClient(&pScene->m_pObjects[pScene->m_nDeliveryOrder[pScene->m_nCurrentDeliveryEntry]]);
          ++(pScene->m_nCurrentDeliveryEntry);
        }
      }
    }
    if(!bFound) {      
      pScene->m_bRandomDeliveryInUse = true;
      // Select random client

      // First count clients
      int nClients = 0;
      int nCandidate = 0;
      while(nCandidate < pScene->m_nObjects) {
        //if(pScene->m_pObjects[nCandidate].m_type == BObject::TType::CLIENT) {
        if(pScene->m_pObjects[nCandidate].m_type == BObject::CLIENT) {
          ++nClients;
        }
        ++nCandidate;
      }

      if(nClients) {
        int nClient = rand() % nClients;
        int nCandidate = 0;
        int nClientCandidate = 0;
        while(nCandidate < pScene->m_nObjects) {
          //if(pScene->m_pObjects[nCandidate].m_type == BObject::TType::CLIENT) {
          if(pScene->m_pObjects[nCandidate].m_type == BObject::CLIENT) {
            if(nClientCandidate == nClient) {
              bFound = true;
              BGame::SetCurrentClient(&pScene->m_pObjects[nCandidate]);
              break;
            }
            ++nClientCandidate;
          }
          ++nCandidate;
        }
      }
    }

    if(BGame::GetCurrentClient()) {
      BGame::m_dLastDeliveryDistance = (BGame::GetCurrentClient()->m_vCenterOnGround - BGame::GetSimulation()->GetVehicle()->m_vLocation).Length();
    } else {
      BGame::m_dLastDeliveryDistance = 800.0;
    }

    BGame::m_clockLastPizzaTempCheck = SDL_GetTicks();
  }
}


//*************************************************************************************************
void CPakoon1View::CheckForDelivery() {
  // Check if we need to deliver
  BVehicle *pVehicle = BGame::GetSimulation()->GetVehicle();
  BVector v = pVehicle->ToWorldCoord(pVehicle->m_vFuelLocation);

  BObject *pClient = BGame::GetCurrentClient();
  if(pClient && (((v - pClient->m_vCenter).Length() < pClient->m_dActiveRadius) && 
                 !BGame::m_bShowCancelQuestion && 
                 !BGame::m_bShowGameMenu && 
                 !BGame::m_bShowQuickHelp)) {
    // Start delivery
    BGame::FreezeSimulation();
    BGame::m_bDeliveryStartInProgress = true;
    BGame::m_clockDeliveryStart = SDL_GetTicks();
    //BGame::SetState(BGame::TState::PICKUP);
    BGame::SetState(BGame::PICKUP);
    BGame::GetSimulation()->RemoveTrackingTarget("CLIENT");

    // Calculate profit and add that to player's pocket
    BGame::m_dLastProfit = BGame::GetMaxProfitFor(BGame::m_dLastDeliveryDistance, BGame::m_dPizzaTemp);
    BGame::GetPlayer()->m_dCash += BGame::m_dLastProfit;

    BGame::m_dPizzaTemp = 0.0;
  }
}





GLint g_nMatrixMode;

//*************************************************************************************************
void CPakoon1View::Setup2DRendering() {
  // Setup 2D projection and draw dashboard.

  glGetIntegerv(GL_MATRIX_MODE, &g_nMatrixMode);

  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

  //GetClientRect(&m_rectWnd);

  // Set up projection geometry so that we can use screen coordinates
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  //gluOrtho2D(0, (GLfloat) m_rectWnd.Width(), 0, (GLfloat) m_rectWnd.Height());
  gluOrtho2D(0, (GLfloat) window_width, 0, (GLfloat) window_height);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  OpenGLHelpers::SetColorFull(1, 1, 1, 1);
  
  GLfloat fLight1AmbientG[ 4];
  fLight1AmbientG[0] = 1;
  fLight1AmbientG[1] = 1;
  fLight1AmbientG[2] = 1;
  fLight1AmbientG[3] = 1;
  glLightfv( GL_LIGHT0, GL_AMBIENT,  fLight1AmbientG);
}


//*************************************************************************************************
void CPakoon1View::End2DRendering() {
  glEnable(GL_CULL_FACE);

  GLfloat fLight1AmbientG[ 4];
  fLight1AmbientG[0] = 0.4f;
  fLight1AmbientG[1] = 0.4f;
  fLight1AmbientG[2] = 0.4f;
  fLight1AmbientG[3] = 0.0f;
  glLightfv( GL_LIGHT0, GL_AMBIENT,  fLight1AmbientG);

  glMatrixMode(g_nMatrixMode);
}





//*************************************************************************************************
//void CPakoon1View::DrawFuelingButtons(CDC* pDC, CRect &rectWnd) {
void CPakoon1View::DrawFuelingButtons() {

  BVehicle *pVehicle = BGame::GetSimulation()->GetVehicle();

  // Check for fuel intake amount
  if(BGame::m_bFuelingInProgress) {
    unsigned clockNow = SDL_GetTicks();
    double dLitresInserted = double(clockNow - BGame::m_clockFuelingStarted) / 1000.0 * 5.0;
    BGame::m_clockFuelingStarted = clockNow;
    double dCostOfFuel = dLitresInserted;

    if((BGame::GetPlayer()->m_dCash - dCostOfFuel) < 0.0) {
      if(BGame::GetPlayer()->m_dCash > 0.0) {
        dCostOfFuel = BGame::GetPlayer()->m_dCash;
        dLitresInserted = dCostOfFuel;
      } else {
        dCostOfFuel = 0.0;
        dLitresInserted = 0.0;
      }
      BGame::m_bFuelingInProgress = false;
    }

    if(BGame::m_nFuelSelect == 0) {
      // Insert gasoline
      if(pVehicle->m_dFuel + dLitresInserted > 100.0) {
        dLitresInserted = 100.0 - pVehicle->m_dFuel;
        BGame::m_bFuelingInProgress = false;
      }
      pVehicle->m_dFuel += dLitresInserted;
    }

    if(BGame::m_nFuelSelect == 1) {
      // Insert kerosine
      if(pVehicle->m_dKerosine + dLitresInserted > 100.0) {
        dLitresInserted = 100.0 - pVehicle->m_dKerosine;
        BGame::m_bFuelingInProgress = false;
      }
      pVehicle->m_dKerosine += dLitresInserted;
    }

    // Pay the meter
    BGame::GetPlayer()->m_dCash -= dCostOfFuel;
  }

  static string sFueling[3];
  //sFueling[0].Format("GASOLINE: %d %%", int(BGame::GetSimulation()->GetVehicle()->m_dFuel));
    stringstream format;
    format << "GASOLINE: " << (int) BGame::GetSimulation()->GetVehicle()->m_dFuel << " %";
    sFueling[0] = format.str();
  //sFueling[1].Format("KEROSINE: %d %%", int(BGame::GetSimulation()->GetVehicle()->m_dKerosine));
    format.str("");
    format << "KEROSINE: " << (int) BGame::GetSimulation()->GetVehicle()->m_dKerosine << " %";
    sFueling[1] = format.str();
  //sFueling[2].Format("    CASH: %d $", int(BGame::GetPlayer()->m_dCash));
    format.str("");
    format << "    CASH: " << (int) BGame::GetPlayer()->m_dCash << "  $";
    sFueling[2] = format.str();

  double dCharHeight = BUI::TextRenderer()->GetCharHeight();
  double dCharWidth  = BUI::TextRenderer()->GetCharWidth();

  // Draw panel around
  glPushMatrix();
  //glTranslated(rectWnd.Width() / 2.0, rectWnd.Height() / 2.0 + 40, 0);
  glTranslated(window_width / 2.0, window_height / 2.0 + 40, 0);
  DrawPanel(dCharWidth * 30, dCharHeight * 5 + 256);
  glPopMatrix();

  // Draw buttons
  OpenGLHelpers::SwitchToTexture(0);
  //BTextures::Use(BTextures::Texture::FUELING);
  BTextures::Use(BTextures::FUELING);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // not mipmapped

  double dBlink = fabs(double(SDL_GetTicks() % 500 - 250.0)) / 250.0;
  double dBlink2 = fabs(double(SDL_GetTicks() % 250 - 125.0)) / 125.0;

  // Draw Exit button
  double dColor = 1.0;
  if((BGame::m_nFuelSelect == 2) && (!BGame::m_bFuelingInProgress)) {
    dColor = 0.5 + dBlink / 2.0;
  }

  OpenGLHelpers::SetColorFull(dColor, dColor, dColor, dColor);
  glPushMatrix();
  //glTranslated(rectWnd.Width() / 2.0, rectWnd.Height() / 2.0 - dCharHeight * 1.5 - 54.0 / 2.0, 0);
  glTranslated(window_width / 2.0, window_height / 2.0 - dCharHeight * 1.5 - 54.0 / 2.0, 0);
  glBegin(GL_TRIANGLE_STRIP);
  OpenGLHelpers::SetTexCoord(0, 0);
  glVertex3f(-120.0 / 2.0, -54.0 / 2.0, 0);
  OpenGLHelpers::SetTexCoord(0, 54.0 / 256);
  glVertex3f(-120.0 / 2.0, 54.0 / 2.0, 0);
  OpenGLHelpers::SetTexCoord(120.0 / 256.0, 0);
  glVertex3f(120.0 / 2.0, -54.0 / 2.0, 0);
  OpenGLHelpers::SetTexCoord(120.0 / 256.0, 54.0 / 256);
  glVertex3f(120.0 / 2.0, 54.0 / 2.0, 0);
  glEnd();
  glPopMatrix();

  // Draw Gaso button
  dColor = 1.0;
  if((BGame::m_nFuelSelect == 0) && (!BGame::m_bFuelingInProgress)) {
    dColor = 0.5 + dBlink / 2.0;
  }

  OpenGLHelpers::SetColorFull(dColor, dColor, dColor, dColor);
  glPushMatrix();
  //glTranslated(rectWnd.Width() / 2.0 - 70.0, rectWnd.Height() / 2.0 + dCharHeight * 2.5 + 128.0 / 2.0, 0);
  glTranslated(window_width / 2.0 - 70.0, window_height / 2.0 + dCharHeight * 2.5 + 128.0 / 2.0, 0);
  if((BGame::m_nFuelSelect == 0) && 
     (BGame::m_bFuelingInProgress)) {
    glScaled(0.9 + dBlink2 * 0.2, 0.9 + dBlink2 * 0.2, 0.9 + dBlink2 * 0.2);
  }
  glBegin(GL_TRIANGLE_STRIP);
  OpenGLHelpers::SetTexCoord(0, 0.5);
  glVertex3f(-128.0 / 2.0, -128.0 / 2.0, 0);
  OpenGLHelpers::SetTexCoord(0, 1.0);
  glVertex3f(-128.0 / 2.0, 128.0 / 2.0, 0);
  OpenGLHelpers::SetTexCoord(0.5, 0.5);
  glVertex3f(128.0 / 2.0, -128.0 / 2.0, 0);
  OpenGLHelpers::SetTexCoord(0.5, 1.0);
  glVertex3f(128.0 / 2.0, 128.0 / 2.0, 0);
  glEnd();

  glPopMatrix();

  // Draw Kero button
  dColor = 1.0;
  if((BGame::m_nFuelSelect == 1) && (!BGame::m_bFuelingInProgress)) {
    dColor = 0.5 + dBlink / 2.0;
  }

  OpenGLHelpers::SetColorFull(dColor, dColor, dColor, dColor);
  glPushMatrix();
  //glTranslated(rectWnd.Width() / 2.0 + 70.0, rectWnd.Height() / 2.0 + dCharHeight * 2.5 + 128.0 / 2.0, 0);
  glTranslated(window_width / 2.0 + 70.0, window_height / 2.0 + dCharHeight * 2.5 + 128.0 / 2.0, 0);
  if((BGame::m_nFuelSelect == 1) && 
     (BGame::m_bFuelingInProgress)) {
    glScaled(0.9 + dBlink2 * 0.2, 0.9 + dBlink2 * 0.2, 0.9 + dBlink2 * 0.2);
  }
  glBegin(GL_TRIANGLE_STRIP);
  OpenGLHelpers::SetTexCoord(0.5, 0.5);
  glVertex3f(-128.0 / 2.0, -128.0 / 2.0, 0);
  OpenGLHelpers::SetTexCoord(0.5, 1.0);
  glVertex3f(-128.0 / 2.0, 128.0 / 2.0, 0);
  OpenGLHelpers::SetTexCoord(1.0, 0.5);
  glVertex3f(128.0 / 2.0, -128.0 / 2.0, 0);
  OpenGLHelpers::SetTexCoord(1.0, 1.0);
  glVertex3f(128.0 / 2.0, 128.0 / 2.0, 0);
  glEnd();
  glPopMatrix();

  // Draw texts
  glPushMatrix();
  //glTranslated(rectWnd.Width() / 2.0, rectWnd.Height() / 2.0, 0);
  glTranslated(window_width / 2.0, window_height / 2.0, 0);

  BUISelectionList selList;
  selList.SetItems(sFueling, 3);
  //selList.DrawAt(-dCharWidth * 8.0, 0, BTextRenderer::TTextAlign::ALIGN_LEFT);
  selList.DrawAt(-dCharWidth * 8.0, 0, BTextRenderer::ALIGN_LEFT);
  glPopMatrix();
}





//*************************************************************************************************
//void CPakoon1View::DrawDashboard(CDC* pDC) {
void CPakoon1View::DrawDashboard() {
  // if short on space, draw tight
  glPushMatrix();
  if((BGame::m_nDispWidth < 1024) && (BGame::m_bNavSat)) {
    glTranslated(128, 0, 0);
  }

  // Draw dashboard as a texture
  OpenGLHelpers::SetColorFull(1, 1, 1, 1);

  // Dashboard
  OpenGLHelpers::SwitchToTexture(0);
  BTextures::Use(BTextures::DASHBOARD);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  glBegin(GL_TRIANGLE_STRIP);
  OpenGLHelpers::SetTexCoord(0, 1);
  //glVertex3f((m_rectWnd.Width() - 512) / 2, 128, 0);
  glVertex3f((window_width - 512) / 2, 128, 0);
  OpenGLHelpers::SetTexCoord(0, 0);
  //glVertex3f((m_rectWnd.Width() - 512) / 2, 0, 0);
  glVertex3f((window_width - 512) / 2, 0, 0);
  OpenGLHelpers::SetTexCoord(1, 1);
  //glVertex3f((m_rectWnd.Width() - 512) / 2 + 512, 128, 0);
  glVertex3f((window_width - 512) / 2 + 512, 128, 0);
  OpenGLHelpers::SetTexCoord(1, 0);
  //glVertex3f((m_rectWnd.Width() - 512) / 2 + 512, 0, 0);
  glVertex3f((window_width - 512) / 2 + 512, 0, 0);
  glEnd();

  // Draw compass in correct orientation
  double dAngle = 0.0;

  BVector vNorth(0, 1, 0);
  BVector vEast(1, 0, 0);
  BVector vForward = m_game.GetSimulation()->GetVehicle()->m_orientation.m_vForward;
  vForward.m_dZ = 0.0;
  vForward.ToUnitLength();

  double dCos = vNorth.ScalarProduct(vForward);
  double dTmp = vEast.ScalarProduct(vForward);
  if(dTmp > 0.0) {
    dAngle = acos(dCos);
  } else {
    dAngle = -acos(dCos);
  }
  dAngle = dAngle / 3.141592654 * 180.0;

  glPushMatrix();
  //glTranslated((m_rectWnd.Width() - 512) / 2 + 256 - 40, 66, 0);
  glTranslated((window_width - 512) / 2 + 256 - 40, 66, 0);
  glRotated(dAngle, 0, 0, 1);

  BTextures::Use(BTextures::COMPASS);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  glBegin(GL_TRIANGLE_STRIP);
  OpenGLHelpers::SetTexCoord(0, 1);
  glVertex3f(-44, -44, 0);
  OpenGLHelpers::SetTexCoord(0, 0);
  glVertex3f(-44, 44, 0);
  OpenGLHelpers::SetTexCoord(1, 1);
  glVertex3f(44, -44, 0);
  OpenGLHelpers::SetTexCoord(1, 0);
  glVertex3f(44, 44, 0);
  glEnd();
  glPopMatrix();

  // Draw fuel indicators (30° full, 155° empty)
  // Draw car fuel indicator
  dAngle = 155.0 - 125.0 * (m_game.GetSimulation()->GetVehicle()->m_dFuel / 100.0);
  glPushMatrix();
  //glTranslated((m_rectWnd.Width() - 512) / 2 + 117.5, 55.0, 0);
  glTranslated((window_width - 512) / 2 + 117.5, 55.0, 0);
  glRotated(dAngle, 0, 0, 1);
  glTranslated(0, 7, 0);
  glDisable(GL_TEXTURE_2D);
  OpenGLHelpers::SetColorFull(0.1, 0.1, 0.1, 1);
  glBegin(GL_POLYGON);
  glVertex3f(-2.0, 0,    0);
  glVertex3f(-1.0, 20.0, 0);
  glVertex3f(1.0,  20.0, 0);
  glVertex3f(2.0,  0,    0);
  glEnd();
  glPopMatrix();

  // Draw kerosine indicator
  dAngle = -155.0 + 125.0 * (m_game.GetSimulation()->GetVehicle()->m_dKerosine / 100.0);
  glPushMatrix();
  //glTranslated((m_rectWnd.Width() - 512) / 2 + 117.5, 55.0, 0);
  glTranslated((window_width - 512) / 2 + 117.5, 55.0, 0);
  glRotated(dAngle, 0, 0, 1);
  glTranslated(0, 7, 0);
  OpenGLHelpers::SetColorFull(0.1, 0.1, 0.1, 1);
  glBegin(GL_POLYGON);
  glVertex3f(-2.0, 0,    0);
  glVertex3f(-1.0, 20.0, 0);
  glVertex3f(1.0,  20.0, 0);
  glVertex3f(2.0,  0,    0);
  glEnd();
  glPopMatrix();

  // Draw beacon indicators
  BTrackingTarget *pTarget = m_game.GetSimulation()->m_targets;
  while(pTarget) {
    //DrawTrackingTarget(pTarget, m_rectWnd);
    DrawTrackingTarget(pTarget);
    pTarget = pTarget->m_pNext;
  }

  // Draw Temp and Cash on the auxiliary panel
  if(!BGame::m_bSceneEditorMode) {
    glPushMatrix();
    //glTranslated((m_rectWnd.Width() - 512) / 2 + 363.0, -8.0, 0);
    glTranslated((window_width - 512) / 2 + 363.0, -8.0, 0);
    BUI::TextRenderer()->StartRenderingText();
    string sTmp;
    //sTmp.Format("%.0lf$", BGame::GetPlayer()->m_dCash);
    stringstream format;
    format << (int) BGame::GetPlayer()->m_dCash << "$";
    sTmp = format.str();
    //BUI::TextRenderer()->DrawSmallTextAt(1.5, 128.0 - 66.0 - 1.5, sTmp, sTmp.GetLength(), BTextRenderer::TTextAlign::ALIGN_LEFT, 0, 0, 0, 0.3);
    BUI::TextRenderer()->DrawSmallTextAt(1.5, 128.0 - 66.0 - 1.5, sTmp, sTmp.length(), BTextRenderer::ALIGN_LEFT, 0, 0, 0, 0.3);
    //BUI::TextRenderer()->DrawSmallTextAt(0, 128.0 - 66.0, sTmp, sTmp.GetLength(), BTextRenderer::TTextAlign::ALIGN_LEFT, 0, 0, 0, 1);
    BUI::TextRenderer()->DrawSmallTextAt(0, 128.0 - 66.0, sTmp, sTmp.length(), BTextRenderer::ALIGN_LEFT, 0, 0, 0, 1);
    if(BGame::m_dPizzaTemp > 10.0) {
      //sTmp.Format("%.1lf", BGame::m_dPizzaTemp);
		format.str("");
		format.precision(3);
		format << BGame::m_dPizzaTemp;
		sTmp = format.str();
      //BUI::TextRenderer()->DrawSmallTextAt(1.5, 128.0 - 81.0 - 1.5, sTmp, sTmp.GetLength(), BTextRenderer::TTextAlign::ALIGN_LEFT, 0, 0, 0, 0.3);
      BUI::TextRenderer()->DrawSmallTextAt(1.5, 128.0 - 81.0 - 1.5, sTmp, sTmp.length(), BTextRenderer::ALIGN_LEFT, 0, 0, 0, 0.3);
      //BUI::TextRenderer()->DrawSmallTextAt(0, 128.0 - 81.0, sTmp, sTmp.GetLength(), BTextRenderer::TTextAlign::ALIGN_LEFT, 0, 0, 0, 1);
      BUI::TextRenderer()->DrawSmallTextAt(0, 128.0 - 81.0, sTmp, sTmp.length(), BTextRenderer::ALIGN_LEFT, 0, 0, 0, 1);
    }
    BUI::TextRenderer()->StopRenderingText();
    glPopMatrix();    
    OpenGLHelpers::SetColorFull(1, 1, 1, 1);
  }

  glPopMatrix();


  // G-Force meter

  // First level
  double dGForce = BGame::GetSimulation()->m_dMaxGForce;
  if(dGForce > 1.3) {
    dGForce = 1.3;
  }
  BVector vColor = ColorForGForce(dGForce);
  glEnable(GL_COLOR_MATERIAL);
  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  glDisable(GL_TEXTURE_2D);
  glBegin(GL_TRIANGLE_STRIP);
  glColor4f(vColor.m_dX, vColor.m_dY, vColor.m_dZ, 1);
  //glVertex3f(10, m_rectWnd.Height() - 220.0 + 140.0 * dGForce, 0); 
  glVertex3f(10, window_height - 220.0 + 140.0 * dGForce, 0); 
  glColor4f(0, 0.5, 0, 1);
  //glVertex3f(10, m_rectWnd.Height() - 220.0, 0); 
  glVertex3f(10, window_height - 220.0, 0); 
  glColor4f(vColor.m_dX, vColor.m_dY, vColor.m_dZ, 1);
  //glVertex3f(5 + 40, m_rectWnd.Height() - 220.0 + 140.0 * dGForce, 0); 
  glVertex3f(5 + 40, window_height - 220.0 + 140.0 * dGForce, 0); 
  glColor4f(0, 0.5, 0, 1);
  //glVertex3f(5 + 40, m_rectWnd.Height() - 220.0, 0); 
  glVertex3f(5 + 40, window_height - 220.0, 0); 
  glEnd();  
  glDisable(GL_COLOR_MATERIAL);

  // Then frames
  OpenGLHelpers::SetColorFull(1, 1, 1, 1);
  OpenGLHelpers::SwitchToTexture(0);
  BTextures::Use(BTextures::PANEL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glBegin(GL_TRIANGLE_STRIP);
  OpenGLHelpers::SetTexCoord(454.0 / 512.0, (512.0 - 266.0) / 512.0);
  //glVertex3f(5, m_rectWnd.Height() - 220.0 - 5, 0);
  glVertex3f(5, window_height - 220.0 - 5, 0);
  OpenGLHelpers::SetTexCoord(454.0 / 512.0, (512.0 - 46.0) / 512.0);
  //glVertex3f(5, m_rectWnd.Height() - 5, 0);
  glVertex3f(5, window_height - 5, 0);
  OpenGLHelpers::SetTexCoord(502.0 / 512.0, (512.0 - 266.0) / 512.0);
  //glVertex3f(5 + 48, m_rectWnd.Height() - 220.0 - 5, 0);
  glVertex3f(5 + 48, window_height - 220.0 - 5, 0);
  OpenGLHelpers::SetTexCoord(502.0 / 512.0, (512.0 - 46.0) / 512.0);
  //glVertex3f(5 + 48, m_rectWnd.Height() - 5, 0);
  glVertex3f(5 + 48, window_height - 5, 0);
  glEnd();
}


//*************************************************************************************************
BVector CPakoon1View::ColorForGForce(double dGForce) {
  if(dGForce > 1.0) {
    dGForce = 1.0;
  }
  if(dGForce < 0.0) {
    dGForce = 0.0;
  }
  return BVector(dGForce, (1.0 - dGForce) * 0.5, 0.0);
}





//*************************************************************************************************
//void CPakoon1View::DrawTrackingTarget(BTrackingTarget *pTarget, CRect &rectWnd) {
void CPakoon1View::DrawTrackingTarget(BTrackingTarget *pTarget) {
  double dAngle;

  BVector vToTarget(pTarget->m_vLoc.m_dX - m_game.GetSimulation()->GetVehicle()->m_vLocation.m_dX,
                    pTarget->m_vLoc.m_dY - m_game.GetSimulation()->GetVehicle()->m_vLocation.m_dY,
                    0);
  double dDist = vToTarget.Length();
  vToTarget.ToUnitLength();
  BVector vUp(0, 0, -1);
  BVector vRight = vUp.CrossProduct(vToTarget);
  BVector vForward = m_game.GetSimulation()->GetVehicle()->m_orientation.m_vForward;
  vForward.m_dZ = 0.0;
  vForward.ToUnitLength();
  double dCos = vToTarget.ScalarProduct(vForward);
  double dTmp = vRight.ScalarProduct(vForward);
  if(dTmp > 0.0) {
    dAngle = acos(dCos);
  } else {
    dAngle = -acos(dCos);
  }
  dAngle = dAngle / 3.141592654 * 180.0;

  glPushMatrix();
  //glTranslated((rectWnd.Width() - 512) / 2 + 256 - 41, 66.5, 0);
  glTranslated((window_width - 512) / 2 + 256 - 41, 66.5, 0);
  glRotated(dAngle, 0, 0, 1);
  glTranslated(0, 21, 0);
  glDisable(GL_TEXTURE_2D);
  OpenGLHelpers::SetColorFull(pTarget->m_dRed, pTarget->m_dGreen, pTarget->m_dBlue, 0.5);
  glBegin(GL_TRIANGLES);
  glVertex3f(-5, 0, 0);
  glVertex3f(0, 10, 0);
  glVertex3f(5, 0, 0);
  glEnd();
  glPopMatrix();

  // Draw distance text
  glEnable(GL_TEXTURE_2D);
  BTextures::Use(BTextures::LED_NUMBERS);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	// OpenGLHelpers::SetColorFull(1, 1, 1, 1);
  //if(pTarget->m_sId.CompareNoCase("BASE") == 0) {
  if(pTarget->m_sId.compare("BASE") == 0) {
    OpenGLHelpers::SetColorFull((pTarget->m_dRed + 1) / 2, 
                                (pTarget->m_dGreen + 1) / 2, 
                                (pTarget->m_dBlue + 1) / 2, 
                                1);
  } else {
    OpenGLHelpers::SetColorFull(pTarget->m_dRed, 
                                pTarget->m_dGreen, 
                                pTarget->m_dBlue, 
                                1);
  }
  string sDist;
  //sDist.Format("%.0lf", dDist);
    stringstream format;
    format << (int) dDist;
    sDist = format.str();
  //double dX = (rectWnd.Width() - 512) / 2 + 224;
  double dX = (window_width - 512) / 2 + 224;
  double dY = 63;
  //if(pTarget->m_sId.CompareNoCase("BASE") == 0) {
  if(pTarget->m_sId.compare("BASE") == 0) {
    // Distance to base next to blue dot
    dY -= 10;
  //} else if(pTarget->m_sId.CompareNoCase("CLIENT") == 0) {
  } else if(pTarget->m_sId.compare("CLIENT") == 0) {
    // Distance to client next to green dot
    dY += 10;
  //} else if(pTarget->m_sId.CompareNoCase("FUEL") == 0) {
  } else if(pTarget->m_sId.compare("FUEL") == 0) {
    // Distance to fuel next to red dot
  }

  // Draw text number by number
  //int nDigit = sDist.GetLength() - 1;
  int nDigit = sDist.length() - 1;
  while(nDigit >= 0) {
    //TCHAR c = sDist.GetAt(nDigit);
    char c = sDist.at(nDigit);
    c -= '0';
    glBegin(GL_POLYGON);
    OpenGLHelpers::SetTexCoord((c * 5) / 64.0, 8.0 / 16.0);
    glVertex3f(dX, dY, 0);
    OpenGLHelpers::SetTexCoord((c * 5) / 64.0, 16.0 / 16.0);
    glVertex3f(dX, dY + 8, 0);
    OpenGLHelpers::SetTexCoord((c * 5 + 4) / 64.0, 16.0 / 16.0);
    glVertex3f(dX + 4, dY + 8, 0);
    OpenGLHelpers::SetTexCoord((c * 5 + 4) / 64.0, 8.0 / 16.0);
    glVertex3f(dX + 4, dY, 0);
    glEnd();
    dX -= 5;
    --nDigit;
  }
}



//*************************************************************************************************
//void CPakoon1View::UpdateTrackingTargetScreenPos(BTrackingTarget *pTarget, CRect &rectWnd) {
void CPakoon1View::UpdateTrackingTargetScreenPos(BTrackingTarget *pTarget) {
  GLdouble modelMatrix[16];
  GLdouble projMatrix[16];
  GLint    viewport[4];

  glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);
  glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);
  glGetIntegerv(GL_VIEWPORT, viewport);

  if(gluProject(pTarget->m_vLoc.m_dX, 
                pTarget->m_vLoc.m_dY, 
                pTarget->m_vLoc.m_dZ,
                modelMatrix,
                projMatrix,
                viewport,
                &(pTarget->m_vScreenPos.m_dX),
                &(pTarget->m_vScreenPos.m_dY),
                &(pTarget->m_vScreenPos.m_dZ))) {

    // Check for behindness and offscreen
    
    BVector vToTarget = pTarget->m_vLoc - BGame::GetSimulation()->GetCamera()->m_vLocation;
    double dBehind = vToTarget.ScalarProduct(BGame::GetSimulation()->GetCamera()->m_orientation.m_vForward);

    if(dBehind < 0.0) {
      //if(pTarget->m_vScreenPos.m_dX < rectWnd.Width() / 2) {
      if(pTarget->m_vScreenPos.m_dX < window_width / 2) {
        //pTarget->m_vScreenPos.m_dX = rectWnd.Width() - 30.0;
        pTarget->m_vScreenPos.m_dX = window_width - 30.0;
      } else {
        pTarget->m_vScreenPos.m_dX = 30.0;
      }
      //if(pTarget->m_vScreenPos.m_dY < rectWnd.Height() / 2) {
      if(pTarget->m_vScreenPos.m_dY < window_height / 2) {
        //pTarget->m_vScreenPos.m_dY = rectWnd.Height() - 30.0;
        pTarget->m_vScreenPos.m_dY = window_height - 30.0;
      } else {
        pTarget->m_vScreenPos.m_dY = 30.0;
      }
    } else {
      if(pTarget->m_vScreenPos.m_dX < 30.0) {
        pTarget->m_vScreenPos.m_dX = 30.0;
      }
      if(pTarget->m_vScreenPos.m_dY < 30.0) {
        pTarget->m_vScreenPos.m_dY = 30.0;
      }
      //if(pTarget->m_vScreenPos.m_dX > rectWnd.Width() - 30.0) {
      if(pTarget->m_vScreenPos.m_dX > window_width - 30.0) {
        //pTarget->m_vScreenPos.m_dX = rectWnd.Width() - 30.0;
        pTarget->m_vScreenPos.m_dX = window_width - 30.0;
      }
      //if(pTarget->m_vScreenPos.m_dY > rectWnd.Height() - 30.0) {
      if(pTarget->m_vScreenPos.m_dY > window_height - 30.0) {
        //pTarget->m_vScreenPos.m_dY = rectWnd.Height() - 30.0;
        pTarget->m_vScreenPos.m_dY = window_height - 30.0;
      }
    }

  } else {
    pTarget->m_vScreenPos.Set(0, 0, 0);
  }
}


//*************************************************************************************************
//void CPakoon1View::DrawTrackingTargetOnScreen(BTrackingTarget *pTarget, CRect &rectWnd) {
void CPakoon1View::DrawTrackingTargetOnScreen(BTrackingTarget *pTarget) {
  unsigned clockNow = SDL_GetTicks();
  double dAlpha = fabs(double(clockNow % 1000) - 500.0) / 500.0;
  double dAlphaBase, dAlphaFluct;
  //double dDist = sqrt((rectWnd.Width() / 2.0 - pTarget->m_vScreenPos.m_dX) * (rectWnd.Width() / 2.0 - pTarget->m_vScreenPos.m_dX) +
  double dDist = sqrt((window_width / 2.0 - pTarget->m_vScreenPos.m_dX) * (window_width / 2.0 - pTarget->m_vScreenPos.m_dX) +
                      //(rectWnd.Height() / 2.0 - pTarget->m_vScreenPos.m_dY) * (rectWnd.Height() / 2.0 - pTarget->m_vScreenPos.m_dY)) / (rectWnd.Width() / 2.0);
                      (window_height / 2.0 - pTarget->m_vScreenPos.m_dY) * (window_height / 2.0 - pTarget->m_vScreenPos.m_dY)) / (window_width / 2.0);
  if(dDist > 1.0) {
    dDist = 1.0;
  }
  dAlphaBase = 0.025 + dDist * 0.475;
  dAlphaFluct = dAlphaBase;
  OpenGLHelpers::SetColorFull(pTarget->m_dRed, pTarget->m_dGreen, pTarget->m_dBlue, dAlphaBase + dAlpha * dAlphaFluct);

  OpenGLHelpers::SwitchToTexture(0);
  BTextures::Use(BTextures::PANEL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  glPushMatrix();
  glTranslated(pTarget->m_vScreenPos.m_dX, pTarget->m_vScreenPos.m_dY, 0.0);

  glBegin(GL_TRIANGLE_STRIP);
  OpenGLHelpers::SetTexCoord(128.0/512.0, (512.0 - 68.0)/512.0);
  glVertex3f(-70.0 / 2.0, -37.0 / 2.0, 0.0);
  OpenGLHelpers::SetTexCoord(128.0/512.0, (512.0 - 32.0)/512.0);
  glVertex3f(-70.0 / 2.0, 37.0 / 2.0, 0.0);
  OpenGLHelpers::SetTexCoord(197.0/512.0, (512.0 - 68.0)/512.0);
  glVertex3f(70.0 / 2.0, -37.0 / 2.0, 0.0);
  OpenGLHelpers::SetTexCoord(197.0/512.0, (512.0 - 32.0)/512.0);
  glVertex3f(70.0 / 2.0, 37.0 / 2.0, 0.0);
  glEnd();

  // Draw text

  OpenGLHelpers::SetColorFull(pTarget->m_dRed * 0.5, pTarget->m_dGreen * 0.5, pTarget->m_dBlue * 0.5, dAlphaBase + 0.25);

  double dTexY = 16.0;
  //if(pTarget->m_sId.CompareNoCase("BASE") == 0) {
  if(pTarget->m_sId.compare("BASE") == 0) {
    dTexY = 16.0;
  //} else if(pTarget->m_sId.CompareNoCase("CLIENT") == 0) {
  } else if(pTarget->m_sId.compare("CLIENT") == 0) {
    dTexY = 16.0 + 22.0;
  //} else if(pTarget->m_sId.CompareNoCase("FUEL") == 0) {
  } else if(pTarget->m_sId.compare("FUEL") == 0) {
    dTexY = 16.0 + 44.0;
  }

  glBegin(GL_TRIANGLE_STRIP);
  OpenGLHelpers::SetTexCoord(196.0/512.0, (512.0 - (dTexY + 22.0))/512.0);
  glVertex3f(-53.0 / 2.0, -22.0 / 2.0 - 17.0, 0.0);
  OpenGLHelpers::SetTexCoord(196.0/512.0, (512.0 - dTexY)/512.0);
  glVertex3f(-53.0 / 2.0, 22.0 / 2.0 - 17.0, 0.0);
  OpenGLHelpers::SetTexCoord(249.0/512.0, (512.0 - (dTexY + 22.0))/512.0);
  glVertex3f(53.0 / 2.0, -22.0 / 2.0 - 17.0, 0.0);
  OpenGLHelpers::SetTexCoord(249.0/512.0, (512.0 - dTexY)/512.0);
  glVertex3f(53.0 / 2.0, 22.0 / 2.0 - 17.0, 0.0);
  glEnd();


  glPopMatrix();
}





//*************************************************************************************************
//void CPakoon1View::DrawQuickHelp(CDC* pDC) {
void CPakoon1View::DrawQuickHelp() {
  // Draw quick help as a texture
  OpenGLHelpers::SetColorFull(1, 1, 1, 1);

  OpenGLHelpers::SwitchToTexture(0);
  if(BGame::GetSceneEditor()->IsActive()) {
    BTextures::Use(BTextures::QUICK_HELP_SCENE_EDITOR);
  } else {
    BTextures::Use(BTextures::QUICK_HELP);
  }
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  glBegin(GL_TRIANGLE_STRIP);
  OpenGLHelpers::SetTexCoord(0, 1);
  //glVertex3f((m_rectWnd.Width() - 512) / 2, m_rectWnd.Height() - (m_rectWnd.Height() - 512) / 2, 0);
  glVertex3f((window_width - 512) / 2, window_height - (window_height - 512) / 2, 0);
  OpenGLHelpers::SetTexCoord(0, 0);
  //glVertex3f((m_rectWnd.Width() - 512) / 2, (m_rectWnd.Height() - 512) / 2, 0);
  glVertex3f((window_width - 512) / 2, (window_height - 512) / 2, 0);
  OpenGLHelpers::SetTexCoord(1, 1);
  //glVertex3f((m_rectWnd.Width() - 512) / 2 + 512, m_rectWnd.Height() - (m_rectWnd.Height() - 512) / 2, 0);
  glVertex3f((window_width - 512) / 2 + 512, window_height - (window_height - 512) / 2, 0);
  OpenGLHelpers::SetTexCoord(1, 0);
  //glVertex3f((m_rectWnd.Width() - 512) / 2 + 512, (m_rectWnd.Height() - 512) / 2, 0);
  glVertex3f((window_width - 512) / 2 + 512, (window_height - 512) / 2, 0);
  glEnd();

}



//*************************************************************************************************
//void CPakoon1View::DrawNavSat(CDC* pDC) {
void CPakoon1View::DrawNavSat() {
  // Draw navsat as a texture
  OpenGLHelpers::SetColorFull(1, 1, 1, 1);
  
  // NavSat window
  glPushMatrix();
  glTranslated(7, 7, 0);
  OpenGLHelpers::SwitchToTexture(0);
  int nRes = BGame::GetNavSat()->ActivateCurrentMapTexture();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  double dMaxTex = (double(nRes) - 1.0) / double(nRes);
  if(nRes == 256) {
    dMaxTex = 1.0;
  }
  glBegin(GL_TRIANGLE_STRIP);
  OpenGLHelpers::SetTexCoord(0, dMaxTex);
  glVertex3f(0, 256, 0);
  OpenGLHelpers::SetTexCoord(0, 0);
  glVertex3f(0, 0, 0);
  OpenGLHelpers::SetTexCoord(dMaxTex, dMaxTex);
  glVertex3f(256, 256, 0);
  OpenGLHelpers::SetTexCoord(dMaxTex, 0);
  glVertex3f(256, 0, 0);
  glEnd();
  glDisable(GL_TEXTURE_2D);

  // Draw crosses on "ground glass"
  OpenGLHelpers::SetColorFull(0, 0, 0, 0.3);
  for(double y = 0; y < 128.0; y += 53.0) {
    for(double x = 0; x < 128.0; x += 53.0) {
      glBegin(GL_LINES);
      glVertex3f(128 + x - 10.0, 128 + y, 0);
      glVertex3f(128 + x + 10.0, 128 + y, 0);
      glVertex3f(128 + x, 128 + y - 10.0, 0);
      glVertex3f(128 + x, 128 + y + 10.0, 0);

      if(x != 0) {
        glVertex3f(128 - x - 10.0, 128 + y, 0);
        glVertex3f(128 - x + 10.0, 128 + y, 0);
        glVertex3f(128 - x, 128 + y - 10.0, 0);
        glVertex3f(128 - x, 128 + y + 10.0, 0);
      }

      if(y != 0) {
        glVertex3f(128 + x - 10.0, 128 - y, 0);
        glVertex3f(128 + x + 10.0, 128 - y, 0);
        glVertex3f(128 + x, 128 - y - 10.0, 0);
        glVertex3f(128 + x, 128 - y + 10.0, 0);
      }

      if((x != 0) && (y != 0)) {
        glVertex3f(128 - x - 10.0, 128 - y, 0);
        glVertex3f(128 - x + 10.0, 128 - y, 0);
        glVertex3f(128 - x, 128 - y - 10.0, 0);
        glVertex3f(128 - x, 128 - y + 10.0, 0);
      }

      glEnd();
    }
  }

  // Draw panel over image and indicators
  OpenGLHelpers::SetColorFull(1, 1, 1, 1);
  glEnable(GL_TEXTURE_2D);
  OpenGLHelpers::SwitchToTexture(0);
  BTextures::Use(BTextures::PANEL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  // Draw tracking beacons
  unsigned clockNow = SDL_GetTicks();
  BTrackingTarget *pTarget = m_game.GetSimulation()->m_targets;
  while(pTarget) {
    BVector vTmp(pTarget->m_vLoc.m_dX, pTarget->m_vLoc.m_dY, 0.0);
    /*DrawNavSatIndicator(false,
                        0,
                        pTarget->m_dRed, 
                        pTarget->m_dGreen, 
                        pTarget->m_dBlue, 
                        fabs(double(clockNow % 1000) - 500.0) / 500.0,
                        vTmp, 
                        m_rectWnd);*/
    DrawNavSatIndicator(false,
                        0,
                        pTarget->m_dRed, 
                        pTarget->m_dGreen, 
                        pTarget->m_dBlue, 
                        fabs(double(clockNow % 1000) - 500.0) / 500.0,
                        vTmp);
    pTarget = pTarget->m_pNext;
  }

  // Draw car indicator
  double dAngle;
  BVector vNorth(0, 1, 0);
  BVector vEast(1, 0, 0);
  double dCos = vNorth.ScalarProduct(m_game.GetSimulation()->GetVehicle()->m_orientation.m_vForward);
  double dTmp = vEast.ScalarProduct(m_game.GetSimulation()->GetVehicle()->m_orientation.m_vForward);
  if(dTmp > 0.0) {
    dAngle = -acos(dCos);
  } else {
    dAngle = acos(dCos);
  }
  dAngle = dAngle / 3.141592654 * 180.0;

  (void) BGame::GetNavSat()->Track(BGame::GetSimulation()->GetVehicle()->m_vLocation);
  //DrawNavSatIndicator(true, dAngle, 1, 1, 1, 1, BGame::GetSimulation()->GetVehicle()->m_vLocation, m_rectWnd);
  DrawNavSatIndicator(true, dAngle, 1, 1, 1, 1, BGame::GetSimulation()->GetVehicle()->m_vLocation);
  glPopMatrix();

  // Panel (in 4 parts to exclude the center text)
  glBegin(GL_TRIANGLE_STRIP);
  OpenGLHelpers::SetTexCoord(120.0/512.0, (511.0-94.0)/512.0);
  glVertex3f(0, 298, 0);
  OpenGLHelpers::SetTexCoord(120.0/512.0, (511.0-145.0)/512.0);
  glVertex3f(0, 298 - 51, 0);
  OpenGLHelpers::SetTexCoord(392.0/512.0, (511.0-94.0)/512.0);
  glVertex3f(272, 298, 0);
  OpenGLHelpers::SetTexCoord(392.0/512.0, (511.0-145.0)/512.0);
  glVertex3f(272, 298 - 51, 0);
  glEnd();

  glBegin(GL_TRIANGLE_STRIP);
  OpenGLHelpers::SetTexCoord(120.0/512.0, (511.0-145.0)/512.0);
  glVertex3f(0, 298 - 51, 0);
  OpenGLHelpers::SetTexCoord(120.0/512.0, (511.0-352.0)/512.0);
  glVertex3f(0, 298 - 278, 0);
  OpenGLHelpers::SetTexCoord(138.0/512.0, (511.0-145.0)/512.0);
  glVertex3f(18, 298 - 51, 0);
  OpenGLHelpers::SetTexCoord(138.0/512.0, (511.0-352.0)/512.0);
  glVertex3f(18, 298 - 278, 0);
  glEnd();

  glBegin(GL_TRIANGLE_STRIP);
  OpenGLHelpers::SetTexCoord(375.0/512.0, (511.0-145.0)/512.0);
  glVertex3f(255, 298 - 51, 0);
  OpenGLHelpers::SetTexCoord(375.0/512.0, (511.0-372.0)/512.0);
  glVertex3f(255, 298 - 278, 0);
  OpenGLHelpers::SetTexCoord(393.0/512.0, (511.0-145.0)/512.0);
  glVertex3f(255+18, 298 - 51, 0);
  OpenGLHelpers::SetTexCoord(393.0/512.0, (511.0-372.0)/512.0);
  glVertex3f(255+18, 298 - 278, 0);
  glEnd();

  glBegin(GL_TRIANGLE_STRIP);
  OpenGLHelpers::SetTexCoord(120.0/512.0, (511.0-372.0)/512.0);
  glVertex3f(0, 20, 0);
  OpenGLHelpers::SetTexCoord(120.0/512.0, (511.0-392.0)/512.0);
  glVertex3f(0, 0, 0);
  OpenGLHelpers::SetTexCoord(392.0/512.0, (511.0-372.0)/512.0);
  glVertex3f(272, 20, 0);
  OpenGLHelpers::SetTexCoord(392.0/512.0, (511.0-392.0)/512.0);
  glVertex3f(272, 0, 0);
  glEnd();

  // Draw handle
  glPushMatrix();
  glTranslated(135, 187, 0);
  glRotated(BGame::m_dNavSatHandleAngle, 0, 0, 1);
  glTranslated(0, 90.5, 0);
  glBegin(GL_TRIANGLE_STRIP);
  OpenGLHelpers::SetTexCoord(423.0/512.0, (511.0-176.0)/512.0);
  glVertex3f(-12, 64, 0);
  OpenGLHelpers::SetTexCoord(423.0/512.0, (511.0-240.0)/512.0);
  glVertex3f(-12, 0, 0);
  OpenGLHelpers::SetTexCoord(447.0/512.0, (511.0-176.0)/512.0);
  glVertex3f(12, 64, 0);
  OpenGLHelpers::SetTexCoord(447.0/512.0, (511.0-240.0)/512.0);
  glVertex3f(12, 0, 0);
  glEnd();
  glPopMatrix();

  glDisable(GL_TEXTURE_2D);
}


//*************************************************************************************************
void CPakoon1View::DrawNavSatIndicator(bool bCar, 
                                       double dAngle, 
                                       double dR, 
                                       double dG, 
                                       double dB, 
                                       double dAlpha, 
                                       //BVector vLoc, 
                                       BVector vLoc) {
                                       //CRect &rectWnd) {
  BVector vLocOnWnd = BGame::GetNavSat()->GetRelLoc(vLoc);
  if(vLocOnWnd.m_dX > 0.0 && vLocOnWnd.m_dX < 255 && 
     vLocOnWnd.m_dY > 0.0 && vLocOnWnd.m_dY < 255) {
    OpenGLHelpers::SetColorFull(dR, dG, dB, dAlpha);
    glPushMatrix();
    glTranslated(vLocOnWnd.m_dX, vLocOnWnd.m_dY, 0);
    glRotated(dAngle, 0, 0, 1);
    glBegin(GL_TRIANGLE_STRIP);
    if(bCar) {
      OpenGLHelpers::SetTexCoord(95.0/512.0, (511.0-247.0)/512.0);
      glVertex3f(-7.5, 7.5, 0);
      OpenGLHelpers::SetTexCoord(95.0/512.0, (511.0-262.0)/512.0);
      glVertex3f(-7.5, -7.5, 0);
      OpenGLHelpers::SetTexCoord(110.0/512.0, (511.0-247.0)/512.0);
      glVertex3f(7.5, 7.5, 0);
      OpenGLHelpers::SetTexCoord(110.0/512.0, (511.0-262.0)/512.0);
      glVertex3f(7.5, -7.5, 0);
    } else {
      OpenGLHelpers::SetTexCoord(95.0/512.0, (511.0-195.0)/512.0);
      glVertex3f(-7.5, 7.5, 0);
      OpenGLHelpers::SetTexCoord(95.0/512.0, (511.0-212.0)/512.0);
      glVertex3f(-7.5, -7.5, 0);
      OpenGLHelpers::SetTexCoord(111.0/512.0, (511.0-195.0)/512.0);
      glVertex3f(7.5, 7.5, 0);
      OpenGLHelpers::SetTexCoord(111.0/512.0, (511.0-212.0)/512.0);
      glVertex3f(7.5, -7.5, 0);
    }
    glEnd();
    glPopMatrix();
  }
}




//*************************************************************************************************
//void CPakoon1View::DrawServiceWnd(CDC* pDC) {
void CPakoon1View::DrawServiceWnd() {
  // Draw service window
  // Draw background and texts
  glDisable(GL_TEXTURE_2D);
  double dAlpha = (-BGame::m_dServiceHandleAngle + 20.0) / 40.0;
  OpenGLHelpers::SetColorFull(0, 0, 0, dAlpha);
  glBegin(GL_TRIANGLE_STRIP);
  //glVertex3f(m_rectWnd.Width() - 7 - 256 - 100, m_rectWnd.Height() - 256 - 7, 0);
  glVertex3f(window_width - 7 - 256 - 100, window_height - 256 - 7, 0);
  //glVertex3f(m_rectWnd.Width() - 7, m_rectWnd.Height() - 256 - 7, 0);
  glVertex3f(window_width - 7, window_height - 256 - 7, 0);
  //glVertex3f(m_rectWnd.Width() - 7 - 256 - 100, m_rectWnd.Height() - 7, 0);
  glVertex3f(window_width - 7 - 256 - 100, window_height - 7, 0);
  //glVertex3f(m_rectWnd.Width() - 7, m_rectWnd.Height() - 7, 0);
  glVertex3f(window_width - 7, window_height - 7, 0);
  glEnd();

  OpenGLHelpers::SwitchToTexture(0);
  BTextures::Use(BTextures::PANEL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  //DrawServiceWndTexts(m_rectWnd);
  DrawServiceWndTexts();

  // Panel (draw in four parts to make it wider and to exclude the "It'll cost ya..." text)
  OpenGLHelpers::SetColorFull(1, 1, 1, 1);
  OpenGLHelpers::SwitchToTexture(0);
  BTextures::Use(BTextures::PANEL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  glBegin(GL_TRIANGLE_STRIP);
  OpenGLHelpers::SetTexCoord(392.0/512.0, (511.0-(391.0-27.0))/512.0);
  //glVertex3f(m_rectWnd.Width() - 27, m_rectWnd.Height() - 272, 0);
  glVertex3f(window_width - 27, window_height - 272, 0);
  OpenGLHelpers::SetTexCoord(392.0/512.0, (511.0-391.0)/512.0);
  //glVertex3f(m_rectWnd.Width(), m_rectWnd.Height() - 272, 0);
  glVertex3f(window_width, window_height - 272, 0);
  OpenGLHelpers::SetTexCoord(120.0/512.0, (511.0-(391.0-27.0))/512.0);
  //glVertex3f(m_rectWnd.Width() - 27, m_rectWnd.Height(), 0);
  glVertex3f(window_width - 27, window_height, 0);
  OpenGLHelpers::SetTexCoord(120.0/512.0, (511.0-391.0)/512.0);
  //glVertex3f(m_rectWnd.Width(), m_rectWnd.Height(), 0);
  glVertex3f(window_width, window_height, 0);
  glEnd();

  glBegin(GL_TRIANGLE_STRIP);
  OpenGLHelpers::SetTexCoord(392.0/512.0, (511.0-144.0)/512.0);
  //glVertex3f(m_rectWnd.Width() - 347, m_rectWnd.Height() - 272, 0);
  glVertex3f(window_width - 347, window_height - 272, 0);
  OpenGLHelpers::SetTexCoord(392.0/512.0, (511.0-(391.0-27.0))/512.0);
  //glVertex3f(m_rectWnd.Width() - 27, m_rectWnd.Height() - 272, 0);
  glVertex3f(window_width - 27, window_height - 272, 0);
  OpenGLHelpers::SetTexCoord((392.0 - 20.0)/512.0, (511.0-144.0)/512.0);
  //glVertex3f(m_rectWnd.Width() - 347, m_rectWnd.Height() - 272 + 20.0, 0);
  glVertex3f(window_width - 347, window_height - 272 + 20.0, 0);
  OpenGLHelpers::SetTexCoord((392.0 - 20.0)/512.0, (511.0-(391.0-27.0))/512.0);
  //glVertex3f(m_rectWnd.Width() - 27, m_rectWnd.Height() - 272 + 20.0, 0);
  glVertex3f(window_width - 27, window_height - 272 + 20.0, 0);
  glEnd();

  glBegin(GL_TRIANGLE_STRIP);
  OpenGLHelpers::SetTexCoord(120.0/512.0, (511.0-144.0)/512.0);
  //glVertex3f(m_rectWnd.Width() - 347, m_rectWnd.Height(), 0);
  glVertex3f(window_width - 347, window_height, 0);
  OpenGLHelpers::SetTexCoord(120.0/512.0, (511.0-(391.0-27.0))/512.0);
  //glVertex3f(m_rectWnd.Width() - 27, m_rectWnd.Height(), 0);
  glVertex3f(window_width - 27, window_height, 0);
  OpenGLHelpers::SetTexCoord((120.0 + 20.0)/512.0, (511.0-144.0)/512.0);
  //glVertex3f(m_rectWnd.Width() - 347, m_rectWnd.Height() - 20.0, 0);
  glVertex3f(window_width - 347, window_height - 20.0, 0);
  OpenGLHelpers::SetTexCoord((120.0 + 20.0)/512.0, (511.0-(391.0-27.0))/512.0);
  //glVertex3f(m_rectWnd.Width() - 27, m_rectWnd.Height() - 20.0, 0);
  glVertex3f(window_width - 27, window_height - 20.0, 0);
  glEnd();

  glBegin(GL_TRIANGLE_STRIP);
  OpenGLHelpers::SetTexCoord(392.0/512.0, (511.0-94.0)/512.0);
  //glVertex3f(m_rectWnd.Width() - 397, m_rectWnd.Height() - 272, 0);
  glVertex3f(window_width - 397, window_height - 272, 0);
  OpenGLHelpers::SetTexCoord(392.0/512.0, (511.0-144.0)/512.0);
  //glVertex3f(m_rectWnd.Width() - 347, m_rectWnd.Height() - 272, 0);
  glVertex3f(window_width - 347, window_height - 272, 0);
  OpenGLHelpers::SetTexCoord(120.0/512.0, (511.0-94.0)/512.0);
  //glVertex3f(m_rectWnd.Width() - 397, m_rectWnd.Height(), 0);
  glVertex3f(window_width - 397, window_height, 0);
  OpenGLHelpers::SetTexCoord(120.0/512.0, (511.0-144.0)/512.0);
  //glVertex3f(m_rectWnd.Width() - 347, m_rectWnd.Height(), 0);
  glVertex3f(window_width - 347, window_height, 0);
  glEnd();

  // Draw overlay to overwrite Near/Far texts with Clear/Black
  glPushMatrix();
  //glTranslated(m_rectWnd.Width() - 394, m_rectWnd.Height() - 246, 0);
  glTranslated(window_width - 394, window_height - 246, 0);
  glBegin(GL_TRIANGLE_STRIP);
  OpenGLHelpers::SetTexCoord(366.0/512.0, (511.0-404.0)/512.0);
  glVertex3f(0, 0, 0);
  OpenGLHelpers::SetTexCoord(366.0/512.0, (511.0-433.0)/512.0);
  glVertex3f(29, 0, 0);
  OpenGLHelpers::SetTexCoord(149.0/512.0, (511.0-404.0)/512.0);
  glVertex3f(0, 217, 0);
  OpenGLHelpers::SetTexCoord(149.0/512.0, (511.0-433.0)/512.0);
  glVertex3f(29, 217, 0);
  glEnd();
  glPopMatrix();

  // Draw handle
  glPushMatrix();
  //glTranslated(m_rectWnd.Width() - 287, m_rectWnd.Height() - 137, 0);
  glTranslated(window_width - 287, window_height - 137, 0);
  glRotated(90 + BGame::m_dServiceHandleAngle, 0, 0, 1);
  glTranslated(0, 90.5, 0);
  glBegin(GL_TRIANGLE_STRIP);
  OpenGLHelpers::SetTexCoord(423.0/512.0, (511.0-176.0)/512.0);
  glVertex3f(-12, 64, 0);
  OpenGLHelpers::SetTexCoord(423.0/512.0, (511.0-240.0)/512.0);
  glVertex3f(-12, 0, 0);
  OpenGLHelpers::SetTexCoord(447.0/512.0, (511.0-176.0)/512.0);
  glVertex3f(12, 64, 0);
  OpenGLHelpers::SetTexCoord(447.0/512.0, (511.0-240.0)/512.0);
  glVertex3f(12, 0, 0);
  glEnd();
  glPopMatrix();

  glDisable(GL_TEXTURE_2D);
}




//*************************************************************************************************
//void CPakoon1View::DrawServiceWndTexts(CRect &rectWnd) {
void CPakoon1View::DrawServiceWndTexts() {
  glPushMatrix();
  //glTranslated(rectWnd.Width() - 256 - 100, rectWnd.Height() - 14 - 18, 0);
  glTranslated(window_width - 256 - 100, window_height - 14 - 18, 0);
  BGame::GetServiceWnd()->DrawTexts();
  glPopMatrix();
}


//*************************************************************************************************
//void CPakoon1View::DrawPickupInfo(CDC* pDC, CRect &rectWnd) {
void CPakoon1View::DrawPickupInfo() {
  double dCharWidth = BUI::TextRenderer()->GetCharWidth();
  double dCharHeight = BUI::TextRenderer()->GetCharHeight();
  glPushMatrix();
  //glTranslated(rectWnd.Width() / 2, rectWnd.Height() / 2, 0);
  glTranslated(window_width / 2, window_height / 2, 0);

  glPushMatrix();
  glTranslated(0, dCharHeight / 2, 0);
  DrawPanel(dCharWidth * 34, dCharHeight * 9);
  glPopMatrix();

  BUI::TextRenderer()->StartRenderingText();
  //BUI::TextRenderer()->DrawTextAt(0, dCharHeight * 3.0,  "* * * DELIVERY INFO * * *", BTextRenderer::TTextAlign::ALIGN_CENTER, 0.55, 0.6, 0.85, 1);
  BUI::TextRenderer()->DrawTextAt(0, dCharHeight * 3.0,  "* * * DELIVERY INFO * * *", BTextRenderer::ALIGN_CENTER, 0.55, 0.6, 0.85, 1);
  //BUI::TextRenderer()->DrawTextAt(0, dCharHeight * 1.0,  "CLIENT:", BTextRenderer::TTextAlign::ALIGN_RIGHT, 1, 0.6, 0.6, 1);
  BUI::TextRenderer()->DrawTextAt(0, dCharHeight * 1.0,  "CLIENT:", BTextRenderer::ALIGN_RIGHT, 1, 0.6, 0.6, 1);
  //BUI::TextRenderer()->DrawTextAt(0, dCharHeight * 0,    "DISTANCE:", BTextRenderer::TTextAlign::ALIGN_RIGHT, 1, 0.6, 0.6, 1);
  BUI::TextRenderer()->DrawTextAt(0, dCharHeight * 0,    "DISTANCE:", BTextRenderer::ALIGN_RIGHT, 1, 0.6, 0.6, 1);
  //BUI::TextRenderer()->DrawTextAt(0, dCharHeight * -1.0, "MAX PROFIT:", BTextRenderer::TTextAlign::ALIGN_RIGHT, 1, 0.6, 0.6, 1);
  BUI::TextRenderer()->DrawTextAt(0, dCharHeight * -1.0, "MAX PROFIT:", BTextRenderer::ALIGN_RIGHT, 1, 0.6, 0.6, 1);
  //BUI::TextRenderer()->DrawTextAt(0, dCharHeight * -2.0, "PIZZA:", BTextRenderer::TTextAlign::ALIGN_RIGHT, 1, 0.6, 0.6, 1);
  BUI::TextRenderer()->DrawTextAt(0, dCharHeight * -2.0, "PIZZA:", BTextRenderer::ALIGN_RIGHT, 1, 0.6, 0.6, 1);

  string sTmp;
  //sTmp.Format("%s", BGame::GetCurrentClient()->m_sName);
  sTmp.assign(BGame::GetCurrentClient()->m_sName);
  //BUI::TextRenderer()->DrawTextAt(0, dCharHeight * 1.0,  sTmp, BTextRenderer::TTextAlign::ALIGN_LEFT, 0.6, 1, 0.6, 1);
  BUI::TextRenderer()->DrawTextAt(0, dCharHeight * 1.0,  sTmp, BTextRenderer::ALIGN_LEFT, 0.6, 1, 0.6, 1);
  //sTmp.Format("%.1lf NU", BGame::m_dLastDeliveryDistance);
    stringstream format;
    format.precision(5);
    format.unsetf(ios::floatfield);
    format << BGame::m_dLastDeliveryDistance << " NU";
    sTmp = format.str();
  //BUI::TextRenderer()->DrawTextAt(0, dCharHeight * 0,    sTmp, BTextRenderer::TTextAlign::ALIGN_LEFT, 0.6, 1, 0.6, 1);
  BUI::TextRenderer()->DrawTextAt(0, dCharHeight * 0,    sTmp, BTextRenderer::ALIGN_LEFT, 0.6, 1, 0.6, 1);
  //sTmp.Format("%.0lf $", BGame::GetMaxProfitFor(BGame::m_dLastDeliveryDistance, BGame::m_dPizzaTemp));
    format.str("");
    format << (int) BGame::GetMaxProfitFor(BGame::m_dLastDeliveryDistance, BGame::m_dPizzaTemp) << " $";
    sTmp = format.str();
  //BUI::TextRenderer()->DrawTextAt(0, dCharHeight * -1.0, sTmp, BTextRenderer::TTextAlign::ALIGN_LEFT, 0.6, 1, 0.6, 1);  
  BUI::TextRenderer()->DrawTextAt(0, dCharHeight * -1.0, sTmp, BTextRenderer::ALIGN_LEFT, 0.6, 1, 0.6, 1);  
  //BUI::TextRenderer()->DrawTextAt(0, dCharHeight * -2.0, BGame::m_sPizzas[BGame::m_nPizza], BTextRenderer::TTextAlign::ALIGN_LEFT, 0.6, 1, 0.6, 1);
  BUI::TextRenderer()->DrawTextAt(0, dCharHeight * -2.0, BGame::m_sPizzas[BGame::m_nPizza], BTextRenderer::ALIGN_LEFT, 0.6, 1, 0.6, 1);
  BUI::TextRenderer()->StopRenderingText();
  glPopMatrix();
}


//*************************************************************************************************
//void CPakoon1View::DrawDeliveryInfo(CDC* pDC, CRect &rectWnd) {
void CPakoon1View::DrawDeliveryInfo() {
  double dCharWidth = BUI::TextRenderer()->GetCharWidth();
  double dCharHeight = BUI::TextRenderer()->GetCharHeight();
  glPushMatrix();
  //glTranslated(rectWnd.Width() / 2, rectWnd.Height() / 2, 0);
  glTranslated(window_width / 2, window_height / 2, 0);

  glPushMatrix();
  glTranslated(0, dCharHeight / 2, 0);
  DrawPanel(dCharWidth * 34, dCharHeight * 9);
  glPopMatrix();

  BUI::TextRenderer()->StartRenderingText();
  //BUI::TextRenderer()->DrawTextAt(0, dCharHeight * 3.0,  "> > > DELIVERY SUMMARY < < <", BTextRenderer::TTextAlign::ALIGN_CENTER, 0.55, 0.6, 0.85, 1);
  BUI::TextRenderer()->DrawTextAt(0, dCharHeight * 3.0,  "> > > DELIVERY SUMMARY < < <", BTextRenderer::ALIGN_CENTER, 0.55, 0.6, 0.85, 1);
  //BUI::TextRenderer()->DrawTextAt(0, dCharHeight * 1.0,  "CLIENT:", BTextRenderer::TTextAlign::ALIGN_RIGHT, 1 * 0.5, 0.6 * 0.5, 0.6 * 0.5, 1);
  BUI::TextRenderer()->DrawTextAt(0, dCharHeight * 1.0,  "CLIENT:", BTextRenderer::ALIGN_RIGHT, 1 * 0.5, 0.6 * 0.5, 0.6 * 0.5, 1);
  //BUI::TextRenderer()->DrawTextAt(0, dCharHeight * 0,    "DISTANCE:", BTextRenderer::TTextAlign::ALIGN_RIGHT, 1 * 0.5, 0.6 * 0.5, 0.6 * 0.5, 1);
  BUI::TextRenderer()->DrawTextAt(0, dCharHeight * 0,    "DISTANCE:", BTextRenderer::ALIGN_RIGHT, 1 * 0.5, 0.6 * 0.5, 0.6 * 0.5, 1);
  //BUI::TextRenderer()->DrawTextAt(0, dCharHeight * -1.0, "PROFIT:", BTextRenderer::TTextAlign::ALIGN_RIGHT, 1, 0.6, 0.6, 1);
  BUI::TextRenderer()->DrawTextAt(0, dCharHeight * -1.0, "PROFIT:", BTextRenderer::ALIGN_RIGHT, 1, 0.6, 0.6, 1);
  //BUI::TextRenderer()->DrawTextAt(0, dCharHeight * -2.0, "PIZZA:", BTextRenderer::TTextAlign::ALIGN_RIGHT, 1 * 0.5, 0.6 * 0.5, 0.6 * 0.5, 1);
  BUI::TextRenderer()->DrawTextAt(0, dCharHeight * -2.0, "PIZZA:", BTextRenderer::ALIGN_RIGHT, 1 * 0.5, 0.6 * 0.5, 0.6 * 0.5, 1);

  string sTmp;
  //sTmp.Format("%s", BGame::GetCurrentClient()->m_sName);
  sTmp.assign(BGame::GetCurrentClient()->m_sName);
  //BUI::TextRenderer()->DrawTextAt(0, dCharHeight * 1.0,  sTmp, BTextRenderer::TTextAlign::ALIGN_LEFT, 0.6 * 0.5, 1 * 0.5, 0.6 * 0.5, 1);
  BUI::TextRenderer()->DrawTextAt(0, dCharHeight * 1.0,  sTmp, BTextRenderer::ALIGN_LEFT, 0.6 * 0.5, 1 * 0.5, 0.6 * 0.5, 1);
  //sTmp.Format("%.1lf NU", BGame::m_dLastDeliveryDistance);
    stringstream format;
    format.precision(5);
    format.unsetf(ios::floatfield);
    format << BGame::m_dLastDeliveryDistance << " NU";
    sTmp = format.str();
  //BUI::TextRenderer()->DrawTextAt(0, dCharHeight * 0,    sTmp, BTextRenderer::TTextAlign::ALIGN_LEFT, 0.6 * 0.5, 1 * 0.5, 0.6 * 0.5, 1);
  BUI::TextRenderer()->DrawTextAt(0, dCharHeight * 0,    sTmp, BTextRenderer::ALIGN_LEFT, 0.6 * 0.5, 1 * 0.5, 0.6 * 0.5, 1);
  //sTmp.Format("%.0lf $", BGame::m_dLastProfit);
    format.str("");
    format << (int) BGame::m_dLastProfit << " $";
    sTmp = format.str();
  //BUI::TextRenderer()->DrawTextAt(0, dCharHeight * -1.0, sTmp, BTextRenderer::TTextAlign::ALIGN_LEFT, 0.6, 1, 0.6, 1);  
  BUI::TextRenderer()->DrawTextAt(0, dCharHeight * -1.0, sTmp, BTextRenderer::ALIGN_LEFT, 0.6, 1, 0.6, 1);  
  //BUI::TextRenderer()->DrawTextAt(0, dCharHeight * -2.0, BGame::m_sPizzas[BGame::m_nPizza], BTextRenderer::TTextAlign::ALIGN_LEFT, 0.6 * 0.5, 1 * 0.5, 0.6 * 0.5, 1);
  BUI::TextRenderer()->DrawTextAt(0, dCharHeight * -2.0, BGame::m_sPizzas[BGame::m_nPizza], BTextRenderer::ALIGN_LEFT, 0.6 * 0.5, 1 * 0.5, 0.6 * 0.5, 1);
  BUI::TextRenderer()->StopRenderingText();
  glPopMatrix();
}




//*************************************************************************************************
void CPakoon1View::DrawPanel(double dWidth, 
                             double dHeight, 
                             double dRed, 
                             double dGreen, 
                             double dBlue, 
                             double dAlpha) {
  // Draw an empty panel centered about the current origin

  // Draw background
  glDisable(GL_TEXTURE_2D);
  OpenGLHelpers::SetColorFull(dRed, dGreen, dBlue, dAlpha);
  glBegin(GL_TRIANGLE_STRIP);
  glVertex3f(-dWidth / 2, -dHeight / 2, 0);
  glVertex3f(-dWidth / 2, dHeight / 2, 0);
  glVertex3f(dWidth / 2, -dHeight / 2, 0);
  glVertex3f(dWidth / 2, dHeight / 2, 0);
  glEnd();

  // Draw borders
  OpenGLHelpers::SetColorFull(1, 1, 1, 1);
  OpenGLHelpers::SwitchToTexture(0);
  //BTextures::Use(BTextures::Texture::PANEL);
  BTextures::Use(BTextures::PANEL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // not mipmapped

  glBegin(GL_QUADS);

  // Corners
  OpenGLHelpers::SetTexCoord(395.0 / 512.0, (512.0 - 393.0) / 512.0);
  glVertex3f(-dWidth / 2 - 14, dHeight / 2 + 14, 0);
  OpenGLHelpers::SetTexCoord(368.0 / 512.0, (512.0 - 393.0) / 512.0);
  glVertex3f(-dWidth / 2 - 14, dHeight / 2 - 14, 0);
  OpenGLHelpers::SetTexCoord(368.0 / 512.0, (512.0 - 367.0) / 512.0);
  glVertex3f(-dWidth / 2 + 14, dHeight / 2 - 14, 0);
  OpenGLHelpers::SetTexCoord(395.0 / 512.0, (512.0 - 367.0) / 512.0);
  glVertex3f(-dWidth / 2 + 14, dHeight / 2 + 14, 0);

  OpenGLHelpers::SetTexCoord(142.0 / 512.0, (512.0 - 393.0) / 512.0);
  glVertex3f(-dWidth / 2 - 14, -dHeight / 2 + 14, 0);
  OpenGLHelpers::SetTexCoord(115.0 / 512.0, (512.0 - 393.0) / 512.0);
  glVertex3f(-dWidth / 2 - 14, -dHeight / 2 - 14, 0);
  OpenGLHelpers::SetTexCoord(115.0 / 512.0, (512.0 - 367.0) / 512.0);
  glVertex3f(-dWidth / 2 + 14, -dHeight / 2 - 14, 0);
  OpenGLHelpers::SetTexCoord(142.0 / 512.0, (512.0 - 367.0) / 512.0);
  glVertex3f(-dWidth / 2 + 14, -dHeight / 2 + 14, 0);

  OpenGLHelpers::SetTexCoord(395.0 / 512.0, (512.0 - 367.0) / 512.0);
  glVertex3f(dWidth / 2 - 14, dHeight / 2 + 14, 0);
  OpenGLHelpers::SetTexCoord(368.0 / 512.0, (512.0 - 367.0) / 512.0);
  glVertex3f(dWidth / 2 - 14, dHeight / 2 - 14, 0);
  OpenGLHelpers::SetTexCoord(368.0 / 512.0, (512.0 - 393.0) / 512.0);
  glVertex3f(dWidth / 2 + 14, dHeight / 2 - 14, 0);
  OpenGLHelpers::SetTexCoord(395.0 / 512.0, (512.0 - 393.0) / 512.0);
  glVertex3f(dWidth / 2 + 14, dHeight / 2 + 14, 0);

  OpenGLHelpers::SetTexCoord(142.0 / 512.0, (512.0 - 367.0) / 512.0);
  glVertex3f(dWidth / 2 - 14, -dHeight / 2 + 14, 0);
  OpenGLHelpers::SetTexCoord(115.0 / 512.0, (512.0 - 367.0) / 512.0);
  glVertex3f(dWidth / 2 - 14, -dHeight / 2 - 14, 0);
  OpenGLHelpers::SetTexCoord(115.0 / 512.0, (512.0 - 393.0) / 512.0);
  glVertex3f(dWidth / 2 + 14, -dHeight / 2 - 14, 0);
  OpenGLHelpers::SetTexCoord(142.0 / 512.0, (512.0 - 393.0) / 512.0);
  glVertex3f(dWidth / 2 + 14, -dHeight / 2 + 14, 0);

  // Left and right border

  OpenGLHelpers::SetTexCoord(368.0 / 512.0, (512.0 - 393.0) / 512.0);
  glVertex3f(-dWidth / 2 - 14, dHeight / 2 - 14, 0);
  OpenGLHelpers::SetTexCoord(142.0 / 512.0, (512.0 - 393.0) / 512.0);
  glVertex3f(-dWidth / 2 - 14, -dHeight / 2 + 14, 0);
  OpenGLHelpers::SetTexCoord(142.0 / 512.0, (512.0 - 367.0) / 512.0);
  glVertex3f(-dWidth / 2 + 14, -dHeight / 2 + 14, 0);
  OpenGLHelpers::SetTexCoord(368.0 / 512.0, (512.0 - 367.0) / 512.0);
  glVertex3f(-dWidth / 2 + 14, dHeight / 2 - 14, 0);

  OpenGLHelpers::SetTexCoord(368.0 / 512.0, (512.0 - 367.0) / 512.0);
  glVertex3f(dWidth / 2 - 14, dHeight / 2 - 14, 0);
  OpenGLHelpers::SetTexCoord(142.0 / 512.0, (512.0 - 367.0) / 512.0);
  glVertex3f(dWidth / 2 - 14, -dHeight / 2 + 14, 0);
  OpenGLHelpers::SetTexCoord(142.0 / 512.0, (512.0 - 393.0) / 512.0);
  glVertex3f(dWidth / 2 + 14, -dHeight / 2 + 14, 0);
  OpenGLHelpers::SetTexCoord(368.0 / 512.0, (512.0 - 393.0) / 512.0);
  glVertex3f(dWidth / 2 + 14, dHeight / 2 - 14, 0);

  // Top and bottom borders
  OpenGLHelpers::SetTexCoord(395.0 / 512.0, (512.0 - 367.0) / 512.0);
  glVertex3f(-dWidth / 2 + 14, dHeight / 2 + 14, 0);
  OpenGLHelpers::SetTexCoord(368.0 / 512.0, (512.0 - 367.0) / 512.0);
  glVertex3f(-dWidth / 2 + 14, dHeight / 2 - 14, 0);
  OpenGLHelpers::SetTexCoord(368.0 / 512.0, (512.0 - 254.0) / 512.0);
  glVertex3f(0, dHeight / 2 - 14, 0);
  OpenGLHelpers::SetTexCoord(395.0 / 512.0, (512.0 - 254.0) / 512.0);
  glVertex3f(0, dHeight / 2 + 14, 0);

  OpenGLHelpers::SetTexCoord(395.0 / 512.0, (512.0 - 254.0) / 512.0);
  glVertex3f(0, dHeight / 2 + 14, 0);
  OpenGLHelpers::SetTexCoord(368.0 / 512.0, (512.0 - 254.0) / 512.0);
  glVertex3f(0, dHeight / 2 - 14, 0);
  OpenGLHelpers::SetTexCoord(368.0 / 512.0, (512.0 - 367.0) / 512.0);
  glVertex3f(dWidth / 2 - 14, dHeight / 2 - 14, 0);
  OpenGLHelpers::SetTexCoord(395.0 / 512.0, (512.0 - 367.0) / 512.0);
  glVertex3f(dWidth / 2 - 14, dHeight / 2 + 14, 0);

  OpenGLHelpers::SetTexCoord(142.0 / 512.0, (512.0 - 367.0) / 512.0);
  glVertex3f(-dWidth / 2 + 14, -dHeight / 2 + 14, 0);
  OpenGLHelpers::SetTexCoord(115.0 / 512.0, (512.0 - 367.0) / 512.0);
  glVertex3f(-dWidth / 2 + 14, -dHeight / 2 - 14, 0);
  OpenGLHelpers::SetTexCoord(115.0 / 512.0, (512.0 - 254.0) / 512.0);
  glVertex3f(0, -dHeight / 2 - 14, 0);
  OpenGLHelpers::SetTexCoord(142.0 / 512.0, (512.0 - 254.0) / 512.0);
  glVertex3f(0, -dHeight / 2 + 14, 0);

  OpenGLHelpers::SetTexCoord(142.0 / 512.0, (512.0 - 254.0) / 512.0);
  glVertex3f(0, -dHeight / 2 + 14, 0);
  OpenGLHelpers::SetTexCoord(115.0 / 512.0, (512.0 - 254.0) / 512.0);
  glVertex3f(0, -dHeight / 2 - 14, 0);
  OpenGLHelpers::SetTexCoord(115.0 / 512.0, (512.0 - 367.0) / 512.0);
  glVertex3f(dWidth / 2 - 14, -dHeight / 2 - 14, 0);
  OpenGLHelpers::SetTexCoord(142.0 / 512.0, (512.0 - 367.0) / 512.0);
  glVertex3f(dWidth / 2 - 14, -dHeight / 2 + 14, 0);

  glEnd();
}





/////////////////////////////////////////////////////////////////////////////
// CPakoon1View drawing

//*************************************************************************************************
//void CPakoon1View::OnDraw(CDC* pDC) {
void CPakoon1View::OnDraw() {
  if(m_pDrawFunction) {
    // Call active draw function
    //(this->*m_pDrawFunction)(pDC);
    (this->*m_pDrawFunction)();
  }
}

/////////////////////////////////////////////////////////////////////////////
// CPakoon1View diagnostics

//*************************************************************************************************
//#ifdef _DEBUG
/*void CPakoon1View::AssertValid() const
{
CView::AssertValid();
}

void CPakoon1View::Dump(CDumpContext& dc) const
{
CView::Dump(dc);
}

CPakoon1Doc* CPakoon1View::GetDocument() // non-debug version is inline
{
ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CPakoon1Doc)));
return (CPakoon1Doc*)m_pDocument;
}
//#endif //_DEBUG*/

/////////////////////////////////////////////////////////////////////////////
// CPakoon1View message handlers

//*************************************************************************************************
void CPakoon1View::OnChar(int nChar, int nRepCnt, int nFlags) {
  if(m_pDrawFunction != &CPakoon1View::OnDrawGame) {
    return;
  }

  if(m_bIgnoreNextChar) {
    m_bIgnoreNextChar = false;
    return;
  }

  // Check if Scene Editor processes key input
  switch(m_game.GetSceneEditor()->m_phase) {
    //case BSceneEditor::TPhase::ASKING_OBJECT_NAME:
    case BSceneEditor::ASKING_OBJECT_NAME:
    //case BSceneEditor::TPhase::ASKING_SCENE_DISPLAY_NAME:
    case BSceneEditor::ASKING_SCENE_DISPLAY_NAME:
    //case BSceneEditor::TPhase::ASKING_SCENE_FILENAME:
    case BSceneEditor::ASKING_SCENE_FILENAME:
      {
        //Invalidate();
        m_game.GetSceneEditor()->m_edit.ProcessChar(nChar);
        BUIEdit::TStatus statusEdit;
        string sObjectName = m_game.GetSceneEditor()->m_edit.GetValue(statusEdit);
        //if(statusEdit == BUIEdit::TStatus::READY) {
        if(statusEdit == BUIEdit::READY) {
          m_game.GetSceneEditor()->AdvancePhase();
          //if(m_game.GetSceneEditor()->m_phase == BSceneEditor::TPhase::SELECTING_OBJECT_TYPE) {
          if(m_game.GetSceneEditor()->m_phase == BSceneEditor::SELECTING_OBJECT_TYPE) {
            BUI::StartUsingSelectionList(&(m_game.GetSceneEditor()->m_sellistObjectType), &CPakoon1View::OnKeyDownSceneEditor);
          }
        //} else if(statusEdit == BUIEdit::TStatus::CANCELED) {
        } else if(statusEdit == BUIEdit::CANCELED) {
          m_game.GetSceneEditor()->CancelPhase();
        }
        return;
      }
      break;
  }

  // Check if service window processes key input
  if((BGame::m_bService) && 
     ((toupper(nChar) >= 32) && 
      (toupper(nChar) <= 96) || 
      //nChar == VK_BACK ||
      nChar == SDLK_BACKSPACE ||
      //nChar == VK_RETURN)) {
      nChar == SDLK_RETURN)) {
    BGame::GetServiceWnd()->AddChar(toupper(nChar));
    return;
  }
}


void CPakoon1View::OnKeyDownGame(int nChar, int nRepCnt, int nFlags) {
  if(m_pDrawFunction != &CPakoon1View::OnDrawGame) {
    return;
  }

  static bool bOpen = false;
  // Check for user input

  if(m_game.GetSimulation()->GetCamera()->m_bNeedsToBeInitialized) {
    m_game.GetSimulation()->GetCamera()->m_locMode = BCamera::FOLLOW;
    m_game.GetSimulation()->GetCamera()->m_dAngleOfView = 75.0;
    m_game.GetSimulation()->GetCamera()->m_bNeedsToBeInitialized = false;
  }

  bool bProcessed = false;
  BVector vToHome;

  // Process non-writing key commands always
  switch(nChar) {
    //case VK_CONTROL:
    case SDLK_LCTRL:
      g_bControl = true;
      break;
    //case VK_SHIFT:
    case SDLK_LSHIFT:
      g_bShift = true;
      break;
    //case VK_ESCAPE:
    case SDLK_ESCAPE:
      bProcessed = true;

      if(m_game.m_bShowQuickHelp) {
        m_game.m_bShowQuickHelp = false;        
        m_nMenuTime += BGame::ContinueSimulation();
      } else if (BGame::m_bDeliveryStartInProgress) {
        m_nMenuTime += BGame::ContinueSimulation();
        BGame::m_bDeliveryStartInProgress = false;
        BMessages::Show(50, "pickup", "RETURN TO BASE", 8, true);
      } else if (BGame::m_bPickupStartInProgress) {
        m_nMenuTime += BGame::ContinueSimulation();
        BGame::m_bPickupStartInProgress = false;
        BMessages::Show(50, "deliver", "DELIVER PIZZA TO CLIENT", 8, true);
        BGame::m_nPizzaDamaged = 0;
      } else {
        BGame::FreezeSimulation();
        m_pKeyDownFunction = &CPakoon1View::OnKeyDownGameMenu;
        m_game.m_bShowGameMenu = !m_game.m_bShowGameMenu;
      }
      break;
    //case VK_F1:
    case SDLK_F1:
      bProcessed = true;
      if(!m_game.m_bShowQuickHelp) {
        BGame::FreezeSimulation(false);
        // show quick help
        m_game.m_bShowQuickHelp = true;
      }
      break;
    //case VK_HOME:
    //  bProcessed = true;
    //  m_game.GetSimulation()->GetCamera()->m_dFollowHeight = -3.0;
    //  vToHome = m_game.GetSimulation()->GetVehicle()->GetHomeLocation() - m_game.GetSimulation()->GetVehicle()->m_vLocation + BVector(35, 0, -20);
    //  m_game.GetSimulation()->GetVehicle()->Move(vToHome);
    //  m_game.GetSimulation()->UpdateCar();
    //  break;
    //case VK_DELETE:
    //  bProcessed = true;
    //  m_game.GetSimulation()->GetVehicle()->Move(BVector(0, 0, -1000.0));
    //  m_game.GetSimulation()->UpdateCar();
    //  break;
    //case VK_F2:
    case SDLK_F2:
      bProcessed = true;
      m_game.GetSimulation()->GetCamera()->m_locMode = BCamera::FOLLOW;
      m_game.GetSimulation()->GetCamera()->m_dAngleOfView = 75.0;
      m_messages.Remove("camera");
      m_messages.Show(60, "camera", "camera: chase", 1);
      break;
    //case VK_F3:
    case SDLK_F3:
      bProcessed = true;
      m_game.GetSimulation()->GetCamera()->m_locMode = BCamera::OVERVIEW;
      m_game.GetSimulation()->GetCamera()->m_bInitLoc = true;
      m_game.GetSimulation()->GetCamera()->m_dAngleOfView = 80.0;
      m_messages.Remove("camera");
      m_messages.Show(60, "camera", "camera: overview", 1);
      break;
    //case VK_F4:
    case SDLK_F4:
      bProcessed = true;
      m_game.GetSimulation()->GetCamera()->m_locMode = BCamera::INCAR;
      m_game.GetSimulation()->GetCamera()->m_dAngleOfView = 75.0;
      m_messages.Remove("camera");
      m_messages.Show(60, "camera", "camera: 1st person", 1);
      break;
    //case VK_F5:
    case SDLK_F5:
      bProcessed = true;
      m_game.GetSimulation()->GetCamera()->m_locMode = BCamera::FIXED;
      m_game.GetSimulation()->GetCamera()->m_vFixLocation = m_game.GetSimulation()->GetCamera()->m_vLocation;
      m_game.GetSimulation()->GetCamera()->m_dAngleOfView = 70.0;
      m_messages.Remove("camera");
      m_messages.Show(60, "camera", "camera: stationary", 1);
      break;
    //case VK_F7:
    case SDLK_F7:
      bProcessed = true;
      m_game.Command()->Run("toggle dashboard");
      break;
    //case VK_F8:
    case SDLK_F8:
      bProcessed = true;
      m_game.Command()->Run("toggle navsat");
      break;
    //case VK_F9:
    case SDLK_F9:
      bProcessed = true;
      m_game.Command()->Run("toggle service");
      break;
    //case VK_F11:
    case SDLK_F11:
      bProcessed = true;
      m_game.GetSimulation()->m_bSteeringAidOn = !m_game.GetSimulation()->m_bSteeringAidOn;
      if(m_game.GetSimulation()->m_bSteeringAidOn) {
        m_messages.Remove("steeringaid");
        m_messages.Show(60, "steeringaid", "Steering aid on", 1);
      } else {
        m_messages.Remove("steeringaid");
        m_messages.Show(60, "steeringaid", "Steering aid off", 1);
      }
      break;
    //case VK_ADD:
    //  bProcessed = true;
    //  m_game.GetSimulation()->m_dAccelerationFactor *= 2.0;
    //  break;
    //case VK_SUBTRACT:
    //  bProcessed = true;
    //  m_game.GetSimulation()->m_dAccelerationFactor /= 2.0;
    //  break;
    //case VK_PRIOR:
    case SDLK_PAGEUP:
      bProcessed = true;
      m_game.GetSimulation()->GetCamera()->m_dFollowHeight -= 1.0;
      break;
    //case VK_NEXT:
    case SDLK_PAGEDOWN:
      bProcessed = true;
      m_game.GetSimulation()->GetCamera()->m_dFollowHeight += 1.0;
      if(m_game.GetSimulation()->GetCamera()->m_dFollowHeight > 0.0) {
        m_game.GetSimulation()->GetCamera()->m_dFollowHeight = 0.0;
      }
      break;
  }


  if(!bProcessed) {
    // Check if service window processes key input
    if(!BGame::m_bService) {
      if(nChar == ControllerModule::m_keymap.m_unAccelerate) {
        bProcessed = true;
        m_game.GetSimulation()->GetVehicle()->m_bAccelerating = true;
        m_game.GetSimulation()->GetVehicle()->m_dAccelerationFactor = 1.0 * m_game.GetSimulation()->GetVehicle()->m_dFuelFactor;
        if(m_game.GetSimulation()->GetVehicle()->m_rotor.m_nHeliMode > 399) {
          m_game.GetSimulation()->GetVehicle()->m_rotor.m_bHeliForwarding = true;
          m_game.GetSimulation()->GetVehicle()->m_dAccelerationFactor = 1.0 * m_game.GetSimulation()->GetVehicle()->m_dKerosineFactor;
        }
        if(m_game.GetSimulation()->GetVehicle()->m_jet.m_nJetMode > 100) {
          m_game.GetSimulation()->GetVehicle()->m_dAccelerationFactor = 1.0 * m_game.GetSimulation()->GetVehicle()->m_dKerosineFactor;
        }
      } else if(nChar == ControllerModule::m_keymap.m_unReverse) {
        bProcessed = true;
        m_game.GetSimulation()->GetVehicle()->m_bReversing = true;
        m_game.GetSimulation()->GetVehicle()->m_dReversingFactor = 1.0 * m_game.GetSimulation()->GetVehicle()->m_dFuelFactor;
        if(m_game.GetSimulation()->GetVehicle()->m_rotor.m_nHeliMode > 399) {
          m_game.GetSimulation()->GetVehicle()->m_rotor.m_bHeliBacking = true;
        }
      } else if(nChar == ControllerModule::m_keymap.m_unPropeller) {
        bProcessed = true;
        if(m_game.GetSimulation()->GetVehicle()->m_rotor.m_nHeliMode > 0) {
          m_game.GetSimulation()->GetVehicle()->m_rotor.m_bHeliLifting = true;
        } else {
          m_game.GetSimulation()->GetVehicle()->m_bPropeller = true;
          // m_game.GetSimulation()->GetVehicle()->m_dPropellerFactor = 1.0 * m_game.GetSimulation()->GetVehicle()->m_dKerosineFactor;
        }
      } else if(nChar == ControllerModule::m_keymap.m_unPropellerReverse) {
        bProcessed = true;
        if(m_game.GetSimulation()->GetVehicle()->m_rotor.m_nHeliMode > 0) {
          m_game.GetSimulation()->GetVehicle()->m_rotor.m_bHeliDescending = true;
        } else {
          m_game.GetSimulation()->GetVehicle()->m_bPropReverse = true;
          // m_game.GetSimulation()->GetVehicle()->m_dPropellerFactor = -0.5 * m_game.GetSimulation()->GetVehicle()->m_dKerosineFactor;
        }
      } else if(nChar == ControllerModule::m_keymap.m_unBreak) {
        bProcessed = true;
        m_game.GetSimulation()->GetVehicle()->m_bBreaking = true;
      } else if(nChar == ControllerModule::m_keymap.m_unLeft) {
        bProcessed = true;
        m_game.GetSimulation()->GetVehicle()->m_bTurningLeft = true;
        if((m_game.GetSimulation()->GetVehicle()->m_rotor.m_nHeliMode > 399) && 
           (m_game.GetSimulation()->GetVehicle()->m_dKerosineFactor > 0.0)) {
          m_game.GetSimulation()->GetVehicle()->m_rotor.m_bHeliLefting = true;
        }
      } else if(nChar == ControllerModule::m_keymap.m_unRight) {
        bProcessed = true;
        m_game.GetSimulation()->GetVehicle()->m_bTurningRight = true;
        if((m_game.GetSimulation()->GetVehicle()->m_rotor.m_nHeliMode > 399) && 
           (m_game.GetSimulation()->GetVehicle()->m_dKerosineFactor > 0.0)) {
          m_game.GetSimulation()->GetVehicle()->m_rotor.m_bHeliRighting = true;
        }
      } else if(nChar == ControllerModule::m_keymap.m_unCamera) {
        bProcessed = true;
        m_game.GetSimulation()->SwitchCameraMode();
      } else if(nChar == ControllerModule::m_keymap.m_unLift) {
        bProcessed = true;
        if(!m_game.GetSimulation()->m_bLiftingUp) {
          if((m_game.GetSimulation()->GetVehicle()->m_orientation.m_vUp.m_dZ > 0.0) || 
            (((SDL_GetTicks() - m_game.m_clockLastLift) / 1000.0) < 6)) {
            m_game.m_clockLastLift = SDL_GetTicks();
            m_game.GetSimulation()->m_bLiftingUp = true;
            m_game.GetSimulation()->m_dLiftZ     = m_game.GetSimulation()->GetVehicle()->m_pBodyPoint[0].m_vLocation.m_dZ;
          }
        }
      } else if((nChar == ControllerModule::m_keymap.m_unHeli) && 
                (m_game.GetSimulation()->GetVehicle()->m_dKerosineFactor > 0.0)) {
        bProcessed = true;
        m_game.GetSimulation()->ToggleHeli();
      } else if((nChar == ControllerModule::m_keymap.m_unJet) && 
                (m_game.GetSimulation()->GetVehicle()->m_dKerosineFactor > 0.0)) {
        bProcessed = true;
        m_game.Command()->Run("toggle jet");    
      }

      switch(nChar) {
        case '0':
          bProcessed = true;
          m_game.FreezeSimulation();
          m_game.GetSimulation()->GetTerrain()->SetRenderResolution(BTerrain::RENDER_SLOW_MACHINE);
          BGame::UpdateSettings();
          BGame::GetSimulation()->GetTerrain()->MakeTerrainValid(BGame::GetSimulation()->GetVehicle()->m_vLocation,
                                                                 BGame::GetSimulation()->GetCamera()->m_vLocation,
                                                                 BGame::GetSimulation()->GetCamera()->m_orientation.m_vForward,
                                                                 true, 
                                                                 false, 
                                                                 false);
          BGame::GetSimulation()->EnsureVehicleIsOverGround();
          m_nMenuTime += BGame::ContinueSimulation();
          break;
        case '1':
          bProcessed = true;
          if(BGame::m_bNavSat && g_bControl) {
            BGame::Command()->Run("set navsat resolution 10400");
            BGame::m_dNavSatHandleAngle = 20.0;
          } else {
            m_game.FreezeSimulation();
            m_game.GetSimulation()->GetTerrain()->SetRenderResolution(BTerrain::RENDER_MINIMUM);
            BGame::UpdateSettings();
            BGame::GetSimulation()->GetTerrain()->MakeTerrainValid(BGame::GetSimulation()->GetVehicle()->m_vLocation,
                                                                   BGame::GetSimulation()->GetCamera()->m_vLocation,
                                                                   BGame::GetSimulation()->GetCamera()->m_orientation.m_vForward,
                                                                   true, 
                                                                   false, 
                                                                   false);
            BGame::GetSimulation()->EnsureVehicleIsOverGround();
            m_nMenuTime += BGame::ContinueSimulation();
          }
          break;
        case '2':
          bProcessed = true;
          if(BGame::m_bNavSat && g_bControl) {
            BGame::Command()->Run("set navsat resolution 5200");
            BGame::m_dNavSatHandleAngle = 10.0;
          } else {
            m_game.FreezeSimulation();
            m_game.GetSimulation()->GetTerrain()->SetRenderResolution(BTerrain::RENDER_LOW);
            BGame::UpdateSettings();
            BGame::GetSimulation()->GetTerrain()->MakeTerrainValid(BGame::GetSimulation()->GetVehicle()->m_vLocation,
                                                                   BGame::GetSimulation()->GetCamera()->m_vLocation,
                                                                   BGame::GetSimulation()->GetCamera()->m_orientation.m_vForward,
                                                                   true, 
                                                                   false, 
                                                                   false);
            BGame::GetSimulation()->EnsureVehicleIsOverGround();
            m_nMenuTime += BGame::ContinueSimulation();
          }
          break;
        case '3':
          bProcessed = true;
          if(BGame::m_bNavSat && g_bControl) {
            BGame::Command()->Run("set navsat resolution 2600");
            BGame::m_dNavSatHandleAngle = 0.0;
          } else {
            m_game.FreezeSimulation();
            m_game.GetSimulation()->GetTerrain()->SetRenderResolution(BTerrain::RENDER_MEDIUM);
            BGame::UpdateSettings();
            BGame::GetSimulation()->GetTerrain()->MakeTerrainValid(BGame::GetSimulation()->GetVehicle()->m_vLocation,
                                                                   BGame::GetSimulation()->GetCamera()->m_vLocation,
                                                                   BGame::GetSimulation()->GetCamera()->m_orientation.m_vForward,
                                                                   true, 
                                                                   false, 
                                                                   false);
            BGame::GetSimulation()->EnsureVehicleIsOverGround();
            m_nMenuTime += BGame::ContinueSimulation();
          }
          break;
        case '4':
          bProcessed = true;
          if(BGame::m_bNavSat && g_bControl) {
            BGame::Command()->Run("set navsat resolution 1300");
            BGame::m_dNavSatHandleAngle = -10.0;
          } else {
            m_game.FreezeSimulation();
            m_game.GetSimulation()->GetTerrain()->SetRenderResolution(BTerrain::RENDER_HIGH);
            BGame::UpdateSettings();
            BGame::GetSimulation()->GetTerrain()->MakeTerrainValid(BGame::GetSimulation()->GetVehicle()->m_vLocation,
                                                                   BGame::GetSimulation()->GetCamera()->m_vLocation,
                                                                   BGame::GetSimulation()->GetCamera()->m_orientation.m_vForward,
                                                                   true, 
                                                                   false, 
                                                                   false);
            BGame::GetSimulation()->EnsureVehicleIsOverGround();
            m_nMenuTime += BGame::ContinueSimulation();
          }
          break;
        case '5':
          bProcessed = true;
          if(BGame::m_bNavSat && g_bControl) {
            BGame::Command()->Run("set navsat resolution 650");
            BGame::m_dNavSatHandleAngle = -20.0;
          } else {
            m_game.FreezeSimulation();
            m_game.GetSimulation()->GetTerrain()->SetRenderResolution(BTerrain::RENDER_MAXIMUM);
            BGame::UpdateSettings();
            BGame::GetSimulation()->GetTerrain()->MakeTerrainValid(BGame::GetSimulation()->GetVehicle()->m_vLocation,
                                                                   BGame::GetSimulation()->GetCamera()->m_vLocation,
                                                                   BGame::GetSimulation()->GetCamera()->m_orientation.m_vForward,
                                                                   true, 
                                                                   false, 
                                                                   false);
            BGame::GetSimulation()->EnsureVehicleIsOverGround();
            m_nMenuTime += BGame::ContinueSimulation();
          }
          break;
        case 'w':
        case 'W':
          {
            bProcessed = true;
            // Check whether this is the second click
            static unsigned clockPrev = 0;
            unsigned clockNow = SDL_GetTicks();
            if((clockNow - clockPrev) < (1000.0 / 3)) {
              m_game.GetSimulation()->GetVehicle()->m_bWireframe = !m_game.GetSimulation()->GetVehicle()->m_bWireframe;
              // m_bWireframe = !m_bWireframe;
              // m_bCreateDLs = true;
            }
            clockPrev = clockNow;
          }
        break;
        //case 'n':
        //case 'N':
        //  bProcessed = true;
        //  m_bNormals = !m_bNormals;
        //  m_bCreateDLs = true;
        //  break;
        case 'v':
        case 'V':
          bProcessed = true;
          m_bShowVideo = !m_bShowVideo;
          break;
        case 'd':
        case 'D':
          bProcessed = true;
          m_game.m_bShowDust = !m_game.m_bShowDust;          
          if(m_game.m_bShowDust) {
            m_game.m_nDustAndClouds = 0;
            m_messages.Remove("dust");
            m_messages.Show(60, "dust", "dust on", 1);
          } else {
            m_game.m_nDustAndClouds = 1;
            m_messages.Remove("dust");
            m_messages.Show(60, "dust", "dust off", 1);
          }
          BGame::UpdateSettings();
          break;
        case 'a':
        case 'A':
          {
            bProcessed = true;

            // Check whether this is the second click
            static unsigned clockPrev = 0;
            unsigned clockNow = SDL_GetTicks();
            if((clockNow - clockPrev) < (1000.0 / 3)) {
              BGame::FreezeSimulation(false);
              // Start Analyzer
              BGame::m_bAnalyzerMode = true;
              BGame::m_clockAnalyzerStarted = clockNow;

              {
				  //FIXME
                /*BGame::MyAfxMessageBox("--------------------------");
                BGame::MyAfxMessageBox("ANALYZER STARTED!");
                BGame::MyAfxMessageBox("--------------------------");
                string sLogInfo;
                sLogInfo.Format("Vehicle: %s", BGame::GetSimulation()->GetVehicle()->m_sName);
                BGame::MyAfxMessageBox(sLogInfo);
                sLogInfo.Format("Scene: %s", BGame::GetSimulation()->GetScene()->m_sName);
                BGame::MyAfxMessageBox(sLogInfo);
                sLogInfo.Format("Screen: %d*%d*%d @ %dHz", BGame::m_nDispWidth, BGame::m_nDispHeight, BGame::m_nDispBits, BGame::m_nDispHz);
                BGame::MyAfxMessageBox(sLogInfo);
                sLogInfo.Format("Terrain: %d", BGame::m_nTerrainResolution);
                BGame::MyAfxMessageBox(sLogInfo);
                sLogInfo.Format("Effects: dust=%d water=%d", BGame::m_nDustAndClouds, BGame::m_nWaterSurface);
                BGame::MyAfxMessageBox(sLogInfo);
                sLogInfo.Format("FPS: AVE=%.2lf, Last10=%.1lf %.1lf %.1lf %.1lf %.1lf %.1lf %.1lf %.1lf %.1lf %.1lf ", 
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
                BGame::MyAfxMessageBox(sLogInfo);*/
              }
            }
            clockPrev = clockNow;
          }
          break;
        case 'e':
        case 'E':
          {
            bProcessed = true;

            // Check whether this is the second click
            static unsigned clockPrev = 0;
            unsigned clockNow = SDL_GetTicks();
            if((clockNow - clockPrev) < (1000.0 / 3)) {
            BGame::FreezeSimulation(false);
              m_game.GetSceneEditor()->Activate();
              m_messages.Remove("sceneeditor");
              m_messages.Show(50, "sceneeditor", "scene editor mode", 1);
              m_pKeyDownFunction = &CPakoon1View::OnKeyDownSceneEditor;
            }
            clockPrev = clockNow;
          }
          break;
        case 'i':
        case 'I':
          bProcessed = true;
          m_game.m_bDisplayInfo = !m_game.m_bDisplayInfo;
          break;
        case 'b':
        case 'B':
          bProcessed = true;
          m_game.m_bSlowMotion = !m_game.m_bSlowMotion;
          if(m_game.m_bSlowMotion) {
            m_messages.Remove("slow motion");
            m_messages.Show(40, "slow motion", "slow motion", 1);
          } else {
            m_messages.Remove("slow motion");
            m_messages.Show(40, "slow motion", "slow motion off", 1);
          }
          break;
        case 'f':
        case 'F':
          bProcessed = true;
          {
            if(!m_game.m_bFrozen) {
              m_messages.Show(40, "frozen", "Frozen (f)", 999);
              BGame::FreezeSimulation();
            } else {
              m_messages.Remove("frozen");
              m_nMenuTime += BGame::ContinueSimulation();
            }
          }
          break;
      }

      switch(nChar) {
        //case VK_SPACE:
        case SDLK_SPACE:
          bProcessed = true;
          m_game.GetSimulation()->GetVehicle()->m_bHandBreaking = true;
          break;
      }

    }
  }

  if(!bProcessed) {
    m_game.m_bShowHint = true;
    m_game.m_clockHintStart = SDL_GetTicks();
  }
  //CView::OnKeyDown(nChar, nRepCnt, nFlags);
}


//*************************************************************************************************
void CPakoon1View::OnKeyDownFueling(int nChar, int nRepCnt, int nFlags) {
  switch(nChar) {
    //case VK_ESCAPE:
    case SDLK_ESCAPE:
      m_nMenuTime += BGame::ContinueSimulation();
      m_pKeyDownFunction = &CPakoon1View::OnKeyDownGame;
      m_game.m_bFueling = false;
      m_game.m_clockLastFuelExit = SDL_GetTicks();
      BMessages::Show(50, "fuelexit", "Exit gas station (within 3 seconds)", 3);
      break;
    //case VK_RETURN:
    case SDLK_RETURN:
      if(m_game.m_nFuelSelect == 2) {
        m_nMenuTime += BGame::ContinueSimulation();
        m_pKeyDownFunction = &CPakoon1View::OnKeyDownGame;
        m_game.m_bFueling = false;
        m_game.m_clockLastFuelExit = SDL_GetTicks();
        BMessages::Show(50, "fuelexit", "Exit gas station (within 3 seconds)", 3);
        
        BVehicle *pVehicle = BGame::GetSimulation()->GetVehicle();

        if(pVehicle->m_dFuel > 0.001) {
          pVehicle->m_dFuelFactor = 1.0;
          pVehicle->m_dAccelerationFactor = 1.0;
          pVehicle->m_dReversingFactor = 1.0;
        }
        if(pVehicle->m_dKerosine > 0.001) {
          pVehicle->m_dKerosineFactor = 1.0;
        }

      } else {
        m_game.m_bFuelingInProgress = !m_game.m_bFuelingInProgress;
        if(m_game.m_bFuelingInProgress) {
          m_game.m_clockFuelingStarted = SDL_GetTicks();
        }
      }
      break;
    //case VK_DOWN: 
    case SDLK_DOWN: 
      if(!BGame::m_bFuelingInProgress) {
        m_game.m_nFuelSelect = 2;
      }
      break;
    //case VK_RIGHT:
    case SDLK_RIGHT:
      if(!BGame::m_bFuelingInProgress) {
        m_game.m_nFuelSelect = 1;
      }
      break;
    //case VK_UP: 
    case SDLK_UP: 
    //case VK_LEFT: 
    case SDLK_LEFT: 
      if(!BGame::m_bFuelingInProgress) {
        m_game.m_nFuelSelect = 0;
      }
      break;
  }
  //Invalidate();
  //CView::OnKeyDown(nChar, nRepCnt, nFlags);
}


//*************************************************************************************************
void CPakoon1View::OnKeyDownGameMenu(int nChar, int nRepCnt, int nFlags) {
  switch(nChar) {
    //case VK_ESCAPE:
    case SDLK_ESCAPE:
      if(m_game.m_bShowQuickHelp) {
        m_game.m_bShowQuickHelp = false;
        m_nMenuTime += BGame::ContinueSimulation();
      } else {        
        m_nMenuTime += BGame::ContinueSimulation();
        m_pKeyDownFunction = &CPakoon1View::OnKeyDownGame;
        m_game.m_bShowGameMenu = !m_game.m_bShowGameMenu;
      }
      break;
    //case VK_RETURN:
    case SDLK_RETURN:
      switch(m_game.m_nGameMenuSelection) {

        // NOTE! main menu selection is numbered from bottom up (i.e. 1 is bottommost menu item)

        case 1:
          // Quit (Return to main menu)
          BGame::GetPlayer()->m_dFuel = BGame::GetSimulation()->GetVehicle()->m_dFuel;
          BGame::GetPlayer()->m_dKerosine = BGame::GetSimulation()->GetVehicle()->m_dKerosine;
          BGame::GetPlayer()->SaveStateFile();
          BGame::GetPlayer()->SaveCurrentSceneInfo();
          //ShowCursor(FALSE);
          SDL_ShowCursor(0);
          m_nMenuTime += BGame::ContinueSimulation();
          m_game.m_bShowGameMenu = false;
          BGame::m_bMenuMode = true;
          BGame::m_pMenuCurrent = &(BGame::m_menuMain);
          m_pKeyDownFunction = &CPakoon1View::OnKeyDownCurrentMenu;
          BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), 
                                       &CPakoon1View::OnKeyDownCurrentMenu);
          m_pDrawFunction = &CPakoon1View::OnDrawCurrentMenu;
          BGame::m_bGameLoading = false;
          m_bDrawOnlyMenu = false;
          m_bFullRedraw = true;
          //InvalidateRect(NULL); 

          // Save log info
          {
			stringstream outputStream;
            BGame::MyAfxMessageBox("------------------------------------");
            BGame::MyAfxMessageBox("EXITING SCENE");
            string sLogInfo;
            //sLogInfo.Format("Vehicle: %s", BGame::GetSimulation()->GetVehicle()->m_sName);
            sLogInfo = "Vehicle: " + BGame::GetSimulation()->GetVehicle()->m_sName;
            BGame::MyAfxMessageBox(sLogInfo);
            //sLogInfo.Format("Scene: %s", BGame::GetSimulation()->GetScene()->m_sName);
            sLogInfo = "Scene: " + BGame::GetSimulation()->GetScene()->m_sName;
            BGame::MyAfxMessageBox(sLogInfo);
            //sLogInfo.Format("Screen: %d*%d*%d @ %dHz", BGame::m_nDispWidth, BGame::m_nDispHeight, BGame::m_nDispBits, BGame::m_nDispHz);
            outputStream << "Screen: " << BGame::m_nDispWidth << "*" << BGame::m_nDispHeight << "*" << BGame::m_nDispBits << " @ " << BGame::m_nDispHz << "Hz";
            sLogInfo = outputStream.str();
            BGame::MyAfxMessageBox(sLogInfo);
            //sLogInfo.Format("Terrain: %d", BGame::m_nTerrainResolution);
            outputStream.str("");
            outputStream << "Terrain: " << BGame::m_nTerrainResolution;
            sLogInfo = outputStream.str();
            BGame::MyAfxMessageBox(sLogInfo);
            //sLogInfo.Format("Effects: dust=%d water=%d", BGame::m_nDustAndClouds, BGame::m_nWaterSurface);
            outputStream.str("");
            outputStream << "Effects: dust=" << BGame::m_nDustAndClouds << " water=" << BGame::m_nWaterSurface;
            sLogInfo = outputStream.str();
            BGame::MyAfxMessageBox(sLogInfo);
            /*sLogInfo.Format("FPS: AVE=%.2lf, Last10=%.1lf %.1lf %.1lf %.1lf %.1lf %.1lf %.1lf %.1lf %.1lf %.1lf ", 
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
                            g_d10LastFPS[9]);*/
            outputStream.str("");
            outputStream << "FPS: AVE=" << g_dRate << ", Last10=" <<
                            g_d10LastFPS[0] << " " <<
                            g_d10LastFPS[1] << " " <<
                            g_d10LastFPS[2] << " " <<
                            g_d10LastFPS[3] << " " <<
                            g_d10LastFPS[4] << " " <<
                            g_d10LastFPS[5] << " " <<
                            g_d10LastFPS[6] << " " <<
                            g_d10LastFPS[7] << " " <<
                            g_d10LastFPS[8] << " " <<
                            g_d10LastFPS[9];
            sLogInfo = outputStream.str();
            BGame::MyAfxMessageBox(sLogInfo);
          }

          // Start menu music
          SoundModule::StopEngineSound();
          SoundModule::StartMenuMusic();
          SoundModule::SetMenuMusicVolume(int(double(BGame::m_nMusicVolume) / 100.0 * 255.0));

          break;
        case 2:
          // Show Help
          m_nMenuTime += BGame::ContinueSimulation();
          m_pKeyDownFunction = &CPakoon1View::OnKeyDownGame;
          m_game.m_bShowGameMenu = false;
          m_game.FreezeSimulation(false);
          // show quick help
          m_game.m_bShowQuickHelp = true;
          break;
        case 3:
          // Settings
          BGame::m_pMenuPrevious = 0;
          BGame::m_bMenuMode = true;
          BGame::m_bSettingsFromGame = true;
          BGame::m_pMenuCurrent = &(BGame::m_menuSettings);
          m_pDrawFunction = &CPakoon1View::OnDrawCurrentMenu;
          BUI::StartUsingSelectionList(&(BGame::m_pMenuCurrent->m_listMenu), 
                                       &CPakoon1View::OnKeyDownCurrentMenu);
          StartMenuScroll(SCROLL_LEFT);
          //Invalidate();
          break;
        case 4:
          // Cancel Order
          m_game.m_nYesNoSelection = 1; // preselect "Yeah"
          m_pKeyDownFunction = &CPakoon1View::OnKeyDownCancelQuestion;
          m_game.m_bShowGameMenu = !m_game.m_bShowGameMenu;
          m_game.m_bShowCancelQuestion = true;
          //Invalidate();
          break;
        case 5:
          // Return to game          
          m_nMenuTime += BGame::ContinueSimulation();
          m_pKeyDownFunction = &CPakoon1View::OnKeyDownGame;
          m_game.m_bShowGameMenu = false;
          break;
      }
      break;
    //case VK_DOWN: 
    case SDLK_DOWN:
      m_game.m_nGameMenuSelection -= 1;
      if(m_game.m_nGameMenuSelection == 0) {
        m_game.m_nGameMenuSelection = 5;
      }
      //Invalidate();
      break;
    //case VK_UP: 
    case SDLK_UP:
      m_game.m_nGameMenuSelection += 1;
      if(m_game.m_nGameMenuSelection == 6) {
        m_game.m_nGameMenuSelection = 1;
      }
      //Invalidate();
      break;
  }
  //CView::OnKeyDown(nChar, nRepCnt, nFlags);
}



//*************************************************************************************************
void CPakoon1View::OnKeyDownCurrentMenu(int nChar, int nRepCnt, int nFlags) {
  switch(nChar) {
    //case VK_ESCAPE:
    case SDLK_ESCAPE:
      break;
    //case VK_RETURN:
    case SDLK_RETURN:
      break;
  }
  //CView::OnKeyDown(nChar, nRepCnt, nFlags);
}





//*************************************************************************************************
void CPakoon1View::OnKeyDownCancelQuestion(int nChar, int nRepCnt, int nFlags) {
  switch(nChar) {
    //case VK_ESCAPE:
    case SDLK_ESCAPE:
      // act as Nope was selected
      m_game.m_bShowCancelQuestion = false;
      m_nMenuTime += BGame::ContinueSimulation();
      m_pKeyDownFunction = &CPakoon1View::OnKeyDownGame;
      //Invalidate();
      break;
    //case VK_RETURN:
    case SDLK_RETURN:
      switch(m_game.m_nYesNoSelection) {
        case 1:
          // Cancel Order (i.e. jump to home)
          if(!BGame::m_bJumpToHome) {
            m_game.m_bShowCancelQuestion = false;
            BGame::GetPlayer()->m_dCash -= 50;
            BGame::m_bJumpToHome = true;
            //BGame::GetSimulation()->GetCamera()->m_locMode = BCamera::TCameraLoc::OVERVIEW;
            BGame::GetSimulation()->GetCamera()->m_locMode = BCamera::OVERVIEW;
            BGame::FreezeSimulation(); // freeze so that the vehicle doesn't jump up if it happens to go underground
            
            // Stop vehicle 
            BVehicle *pVehicle = BGame::GetSimulation()->GetVehicle();
            for(int i = 0; i < pVehicle->m_nBodyPoints; ++i) {
              pVehicle->m_pBodyPoint[i].m_vector = BVector(0, 0, 0);
            }
  
            // Return to game          
            m_nMenuTime += BGame::ContinueSimulation();
            m_pKeyDownFunction = &CPakoon1View::OnKeyDownGame;
          }
          //Invalidate();
          break;
        case 2:
          // Don't cancel order
          m_game.m_bShowCancelQuestion = false;
          // Return to game          
          m_nMenuTime += BGame::ContinueSimulation();
          m_pKeyDownFunction = &CPakoon1View::OnKeyDownGame;
          //Invalidate();
          break;
      }
      break;
    //case VK_DOWN: 
    case SDLK_DOWN: 
    //case VK_RIGHT: 
    case SDLK_RIGHT: 
      m_game.m_nYesNoSelection -= 1;
      if(m_game.m_nYesNoSelection == 0) {
        m_game.m_nYesNoSelection = 2;
      }
      //Invalidate();
      break;
    //case VK_UP: 
    case SDLK_UP: 
    //case VK_LEFT: 
    case SDLK_LEFT: 
      m_game.m_nYesNoSelection += 1;
      if(m_game.m_nYesNoSelection == 3) {
        m_game.m_nYesNoSelection = 1;
      }
      //Invalidate();
      break;
  }
  //CView::OnKeyDown(nChar, nRepCnt, nFlags);
}



//*************************************************************************************************
void CPakoon1View::OnKeyDownSceneEditor(int nChar, int nRepCnt, int nFlags) {
  BCamera *pCamera = m_game.GetSimulation()->GetCamera();
  BScene  *pScene  = m_game.GetSimulation()->GetScene();

  switch(m_game.GetSceneEditor()->m_phase) {
    //case BSceneEditor::TPhase::ASKING_OBJECT_NAME:
    case BSceneEditor::ASKING_OBJECT_NAME:
    //case BSceneEditor::TPhase::ASKING_SCENE_DISPLAY_NAME:
    case BSceneEditor::ASKING_SCENE_DISPLAY_NAME:
    //case BSceneEditor::TPhase::ASKING_SCENE_FILENAME:
    case BSceneEditor::ASKING_SCENE_FILENAME:
      // OnChar() processes these
      //CView::OnKeyDown(nChar, nRepCnt, nFlags);
      return;
      break;
  }

  double dModeScaler = 20.0;
  if(g_bShift) {
    dModeScaler = 1.0;
  }
  //if((m_game.GetSceneEditor()->m_phase == BSceneEditor::TPhase::BASIC) &&
  if((m_game.GetSceneEditor()->m_phase == BSceneEditor::BASIC) &&
      g_bControl) {
    dModeScaler = 150.0;
  }

  switch(nChar) {
    //case VK_CONTROL:
    case SDLK_LCTRL:
      g_bControl = true;
      break;
    //case VK_SHIFT:
    case SDLK_LSHIFT:
      g_bShift = true;
      break;
    //case VK_RETURN:
    case SDLK_RETURN:
      // If moving object, return to camera mode
      //if(BGame::GetSceneEditor()->m_phase == BSceneEditor::TPhase::MOVING_OBJECT) {
      if(BGame::GetSceneEditor()->m_phase == BSceneEditor::MOVING_OBJECT) {
        BGame::GetSceneEditor()->AdvancePhase();
      }
      break;
    case 'e':
    case 'E':
      // Return to game mode (NOTE: SHOULD WE WARN ABOUT AN UNSAVED SCENE?)      
      m_game.GetSceneEditor()->Deactivate();
      m_pKeyDownFunction = &CPakoon1View::OnKeyDownGame;
      m_messages.Remove("sceneeditor");
      m_messages.Show(50, "sceneeditor", "game mode", 1);
      m_nMenuTime += BGame::ContinueSimulation();
      break;
    //case VK_RIGHT: 
    case SDLK_RIGHT: 
      //if((m_game.GetSceneEditor()->m_phase == BSceneEditor::TPhase::MOVING_OBJECT) &&
      if((m_game.GetSceneEditor()->m_phase == BSceneEditor::MOVING_OBJECT) &&
         (!g_bControl)) {
        // Move object to right
        BObject *pObject = m_game.GetSceneEditor()->GetActiveObject();
        if(pObject) {
          pObject->m_vLocation += pCamera->m_orientation.m_vRight * 0.1 * dModeScaler;
          pObject->m_vCenter += pCamera->m_orientation.m_vRight * 0.1 * dModeScaler;
          pObject->RecreateShadow();
        }
      } else {
        if(!g_bShift) {
          pCamera->m_orientation.m_vForward = pCamera->m_orientation.m_vForward + pCamera->m_orientation.m_vRight * 0.02 * dModeScaler;
        } else {
          pCamera->m_orientation.m_vForward = pCamera->m_orientation.m_vForward + pCamera->m_orientation.m_vRight * 0.05 * dModeScaler;
        }
        pCamera->m_orientation.m_vForward.ToUnitLength();
        pCamera->m_orientation.m_vRight = pCamera->m_orientation.m_vUp.CrossProduct(pCamera->m_orientation.m_vForward);
      }
      break;
    //case VK_LEFT:
    case SDLK_LEFT:
      //if((m_game.GetSceneEditor()->m_phase == BSceneEditor::TPhase::MOVING_OBJECT) &&
      if((m_game.GetSceneEditor()->m_phase == BSceneEditor::MOVING_OBJECT) &&
         (!g_bControl)) {
        // Move object to west
        BObject *pObject = m_game.GetSceneEditor()->GetActiveObject();
        if(pObject) {
          pObject->m_vLocation -= pCamera->m_orientation.m_vRight * 0.1 * dModeScaler;
          pObject->m_vCenter -= pCamera->m_orientation.m_vRight * 0.1 * dModeScaler;
          pObject->RecreateShadow();
        }
      } else {
        if(!g_bShift) {
          pCamera->m_orientation.m_vForward = pCamera->m_orientation.m_vForward + pCamera->m_orientation.m_vRight * -0.02 * dModeScaler;
        } else {
          pCamera->m_orientation.m_vForward = pCamera->m_orientation.m_vForward + pCamera->m_orientation.m_vRight * -0.05 * dModeScaler;
        }
        pCamera->m_orientation.m_vForward.ToUnitLength();
        pCamera->m_orientation.m_vRight = pCamera->m_orientation.m_vUp.CrossProduct(pCamera->m_orientation.m_vForward);
      }
      break;
    //case VK_DOWN: 
    case SDLK_DOWN: 
      //if((m_game.GetSceneEditor()->m_phase == BSceneEditor::TPhase::MOVING_OBJECT) &&
      if((m_game.GetSceneEditor()->m_phase == BSceneEditor::MOVING_OBJECT) &&
         (!g_bControl)) {
        // Move object to south
        BObject *pObject = m_game.GetSceneEditor()->GetActiveObject();
        if(pObject) {
          pObject->m_vLocation -= pCamera->m_orientation.m_vForward * 0.1 * dModeScaler;
          pObject->m_vCenter -= pCamera->m_orientation.m_vForward * 0.1 * dModeScaler;
          pObject->RecreateShadow();
        }
      } else {
        pCamera->m_vLocation = pCamera->m_vLocation + pCamera->m_orientation.m_vForward * -dModeScaler;
      }
      break;
    //case VK_UP: 
    case SDLK_UP: 
      //if((m_game.GetSceneEditor()->m_phase == BSceneEditor::TPhase::MOVING_OBJECT) &&
      if((m_game.GetSceneEditor()->m_phase == BSceneEditor::MOVING_OBJECT) &&
         (!g_bControl)) {
        // Move object to north
        BObject *pObject = m_game.GetSceneEditor()->GetActiveObject();
        if(pObject) {
          pObject->m_vLocation += pCamera->m_orientation.m_vForward * 0.1 * dModeScaler;
          pObject->m_vCenter += pCamera->m_orientation.m_vForward * 0.1 * dModeScaler;
          pObject->RecreateShadow();
        }
      } else {
        pCamera->m_vLocation = pCamera->m_vLocation + pCamera->m_orientation.m_vForward * dModeScaler;
      }
      break;
    //case VK_NEXT:
    case SDLK_PAGEDOWN:
      //if((m_game.GetSceneEditor()->m_phase == BSceneEditor::TPhase::MOVING_OBJECT) &&
      if((m_game.GetSceneEditor()->m_phase == BSceneEditor::MOVING_OBJECT) &&
         (!g_bControl)) {
        // Move object down
        BObject *pObject = m_game.GetSceneEditor()->GetActiveObject();
        if(pObject) {
          pObject->m_vLocation.m_dZ += 0.1 * dModeScaler;
          pObject->m_vCenter.m_dZ += 0.1 * dModeScaler;
          pObject->RecreateShadow();
        }
      } else {
        pCamera->m_vLocation = pCamera->m_vLocation + pCamera->m_orientation.m_vUp * -dModeScaler;
      }
      break;
    //case VK_PRIOR:
    case SDLK_PAGEUP:
      //if((m_game.GetSceneEditor()->m_phase == BSceneEditor::TPhase::MOVING_OBJECT) &&
      if((m_game.GetSceneEditor()->m_phase == BSceneEditor::MOVING_OBJECT) &&
         (!g_bControl)) {
        // Move object up
        BObject *pObject = m_game.GetSceneEditor()->GetActiveObject();
        if(pObject) {
          pObject->m_vLocation.m_dZ -= 0.1 * dModeScaler;
          pObject->m_vCenter.m_dZ -= 0.1 * dModeScaler;
          pObject->RecreateShadow();
        }
      } else {
        pCamera->m_vLocation = pCamera->m_vLocation + pCamera->m_orientation.m_vUp * dModeScaler;
      }
      break;
    //case VK_HOME:
    case SDLK_HOME:
      //if((m_game.GetSceneEditor()->m_phase == BSceneEditor::TPhase::MOVING_OBJECT) &&
      if((m_game.GetSceneEditor()->m_phase == BSceneEditor::MOVING_OBJECT) &&
         (!g_bControl)) {
        // Rotate clockwise
        BObject *pObject = m_game.GetSceneEditor()->GetActiveObject();
        if(pObject) {
          pObject->m_dZRotation += 1 * dModeScaler;
          pObject->RecreateShadow();
        }
      }
      break;
    //case VK_END:
    case SDLK_END:
      //if((m_game.GetSceneEditor()->m_phase == BSceneEditor::TPhase::MOVING_OBJECT) &&
      if((m_game.GetSceneEditor()->m_phase == BSceneEditor::MOVING_OBJECT) &&
         (!g_bControl)) {
        // Rotate counter clockwise
        BObject *pObject = m_game.GetSceneEditor()->GetActiveObject();
        if(pObject) {
          pObject->m_dZRotation -= 1 * dModeScaler;
          pObject->RecreateShadow();
        }
      } 
      break;
    case 'o':
    case 'O':
      //if((m_game.GetSceneEditor()->m_phase == BSceneEditor::TPhase::MOVING_OBJECT) &&
      if((m_game.GetSceneEditor()->m_phase == BSceneEditor::MOVING_OBJECT) &&
         (!g_bControl)) {
        // Scale object's active radius
        BObject *pObject = m_game.GetSceneEditor()->GetActiveObject();
        if(pObject) {
          double dScaler = 1.01;
          if(!g_bShift) {
            dScaler = 2.0;
          }
          pObject->m_dActiveRadius *= dScaler;
        }
      } 
      break;
    case 'i':
    case 'I':
      //if((m_game.GetSceneEditor()->m_phase == BSceneEditor::TPhase::MOVING_OBJECT) &&
      if((m_game.GetSceneEditor()->m_phase == BSceneEditor::MOVING_OBJECT) &&
         (!g_bControl)) {
        // Shrink object's active radius
        BObject *pObject = m_game.GetSceneEditor()->GetActiveObject();
        if(pObject) {
          double dScaler = (1.0 / 1.01);
          if(!g_bShift) {
            dScaler = 0.5;
          }
          pObject->m_dActiveRadius *= dScaler;
        }
      } 
      break;
    case 'l':
    case 'L':
      //if((m_game.GetSceneEditor()->m_phase == BSceneEditor::TPhase::MOVING_OBJECT) &&
      if((m_game.GetSceneEditor()->m_phase == BSceneEditor::MOVING_OBJECT) &&
         (!g_bControl)) {
        // Scale object bigger
        BObject *pObject = m_game.GetSceneEditor()->GetActiveObject();
        if(pObject) {
          double dScaler = 1.01;
          if(!g_bShift) {
            dScaler = 2.0;
          }
          pObject->m_dScale2 *= dScaler;
          pObject->m_dRadius *= dScaler;
          pObject->RecreateShadow();
        }
      } 
      break;
    case 'k':
    case 'K':
      //if((m_game.GetSceneEditor()->m_phase == BSceneEditor::TPhase::MOVING_OBJECT) &&
      if((m_game.GetSceneEditor()->m_phase == BSceneEditor::MOVING_OBJECT) &&
         (!g_bControl)) {
        // Scale object bigger
        BObject *pObject = m_game.GetSceneEditor()->GetActiveObject();
        if(pObject) {
          double dScaler = (1.0 / 1.01);
          if(!g_bShift) {
            dScaler = 0.5;
          }
          pObject->m_dScale2 *= dScaler;
          pObject->m_dRadius *= dScaler;
          pObject->RecreateShadow();
        }
      } 
      break;
    case 'm':
    case 'M':
      BUI::StartUsingSelectionList(&(m_game.GetSimulation()->GetScene()->m_sellistSceneObjects), 
                                   &CPakoon1View::OnKeyDownSceneEditor);
      m_game.GetSimulation()->GetScene()->m_sellistSceneObjects.SelectItem(BGame::GetSceneEditor()->m_sActiveObject);
      //m_game.GetSceneEditor()->m_phase = BSceneEditor::TPhase::SELECTING_SCENE_OBJECT;
      m_game.GetSceneEditor()->m_phase = BSceneEditor::SELECTING_SCENE_OBJECT;
      break;
    case 'a':
    case 'A':
      m_game.GetSceneEditor()->m_edit.Setup("Object name:", "", 32);
      //m_game.GetSceneEditor()->m_phase = BSceneEditor::TPhase::ASKING_OBJECT_NAME;
      m_game.GetSceneEditor()->m_phase = BSceneEditor::ASKING_OBJECT_NAME;
      m_bIgnoreNextChar = true;
      break;
    case 'd':
    case 'D':
      BUI::StartUsingSelectionList(&(m_game.GetSimulation()->GetScene()->m_sellistSceneObjects), 
                                   &CPakoon1View::OnKeyDownSceneEditor);
      m_game.GetSimulation()->GetScene()->m_sellistSceneObjects.SelectItem(BGame::GetSceneEditor()->m_sActiveObject);
      //m_game.GetSceneEditor()->m_phase = BSceneEditor::TPhase::SELECTING_SCENE_OBJECT_TO_DELETE;
      m_game.GetSceneEditor()->m_phase = BSceneEditor::SELECTING_SCENE_OBJECT_TO_DELETE;
      break;
    case 's':
    case 'S':
      m_game.GetSceneEditor()->m_edit.Setup("Scene display name:", pScene->m_sName, 32);
      //m_game.GetSceneEditor()->m_phase = BSceneEditor::TPhase::ASKING_SCENE_DISPLAY_NAME;
      m_game.GetSceneEditor()->m_phase = BSceneEditor::ASKING_SCENE_DISPLAY_NAME;
      m_bIgnoreNextChar = true;
      break;
    //case VK_F1:
    case SDLK_F1:
      // show quick help
      m_game.m_bShowQuickHelp = true;
      break;
    //case VK_ESCAPE:
    case SDLK_ESCAPE:
      if(m_game.m_bShowQuickHelp) {
        m_game.m_bShowQuickHelp = false;        
      }
      break;
    case '0':
      m_game.GetSimulation()->GetTerrain()->SetRenderResolution(BTerrain::RENDER_SLOW_MACHINE);
      BGame::UpdateSettings();
      break;
    case '1':
      if(BGame::m_bNavSat && g_bControl) {
        BGame::Command()->Run("set navsat resolution 10400");
        BGame::m_dNavSatHandleAngle = 20.0;
      } else {
        m_game.GetSimulation()->GetTerrain()->SetRenderResolution(BTerrain::RENDER_MINIMUM);
        BGame::UpdateSettings();
      }
      break;
    case '2':
      if(BGame::m_bNavSat && g_bControl) {
        BGame::Command()->Run("set navsat resolution 5200");
        BGame::m_dNavSatHandleAngle = 10.0;
      } else {
        m_game.GetSimulation()->GetTerrain()->SetRenderResolution(BTerrain::RENDER_LOW);
        BGame::UpdateSettings();
      }
      break;
    case '3':
      if(BGame::m_bNavSat && g_bControl) {
        BGame::Command()->Run("set navsat resolution 2600");
        BGame::m_dNavSatHandleAngle = 0.0;
      } else {
        m_game.GetSimulation()->GetTerrain()->SetRenderResolution(BTerrain::RENDER_MEDIUM);
        BGame::UpdateSettings();
      }
      break;
    case '4':
      if(BGame::m_bNavSat && g_bControl) {
        BGame::Command()->Run("set navsat resolution 1300");
        BGame::m_dNavSatHandleAngle = -10.0;
      } else {
        m_game.GetSimulation()->GetTerrain()->SetRenderResolution(BTerrain::RENDER_HIGH);
        BGame::UpdateSettings();
      }
      break;
    case '5':
      if(BGame::m_bNavSat && g_bControl) {
        BGame::Command()->Run("set navsat resolution 650");
        BGame::m_dNavSatHandleAngle = -20.0;
      } else {
        m_game.GetSimulation()->GetTerrain()->SetRenderResolution(BTerrain::RENDER_MAXIMUM);
        BGame::UpdateSettings();
      }
      break;
    //case VK_F7:
    case SDLK_F7:
      m_game.Command()->Run("toggle dashboard");
      break;
    //case VK_F8:
    case SDLK_F8:
      m_game.Command()->Run("toggle navsat");
      break;
    //case VK_F9:
    case SDLK_F9:
      m_game.Command()->Run("toggle service");
      break;
  }
  //Invalidate();
  //CView::OnKeyDown(nChar, nRepCnt, nFlags);
}







//*************************************************************************************************
// This is a general purpose keyboard handler for active selection list
void CPakoon1View::OnKeyDownSelectionList(int nChar, int nRepCnt, int nFlags) {
  BUISelectionList *pList = BUI::GetActiveSelectionList();
  if(!pList) {
    m_pKeyDownFunction = BUI::StopUsingSelectionList();
    //Invalidate();
    //CView::OnKeyDown(nChar, nRepCnt, nFlags);
  }

  bool bAdvancePhase = false;

  switch(nChar) {
    //case VK_DOWN: 
    case SDLK_DOWN:
      pList->AdvanceSelection(1);
      break;
    //case VK_UP: 
    case SDLK_UP:
      pList->AdvanceSelection(-1);
      break;
    //case VK_HOME:
    case SDLK_HOME:
      pList->AdvanceSelection(-1000); // to get to the beginning
      break;
    //case VK_END:
    case SDLK_END:
      pList->AdvanceSelection(1000); // to get to the end
      break;
    //case VK_RETURN:
    case SDLK_RETURN:
      m_pKeyDownFunction = BUI::StopUsingSelectionList();
      bAdvancePhase = true;

      if(BGame::m_bMenuMode) {
        ReturnPressedOnCurrentMenu();
      }

      break;
    //case VK_ESCAPE:
    case SDLK_ESCAPE:
      pList->Cancel();
      m_pKeyDownFunction = BUI::StopUsingSelectionList();      
      if(BGame::m_bSceneEditorMode) {
        m_game.GetSceneEditor()->CancelPhase();
      }
      bAdvancePhase = false;

      if(BGame::m_bMenuMode) {
        CancelPressedOnCurrentMenu();
      }

      break;
  }

  if(bAdvancePhase && BGame::m_bSceneEditorMode) {
    BGame::GetSceneEditor()->AdvancePhase();
  }

  //Invalidate();
  //CView::OnKeyDown(nChar, nRepCnt, nFlags);
}


//*************************************************************************************************
// This is a general purpose keyboard handler for active slider
void CPakoon1View::OnKeyDownSlider(int nChar, int nRepCnt, int nFlags) {
  int *pnSliderValue = BUI::m_pnSliderValue;
  if(!pnSliderValue) {
    m_pKeyDownFunction = BUI::StopUsingSlider();
    //Invalidate();
    //CView::OnKeyDown(nChar, nRepCnt, nFlags);
  }

  switch(nChar) {
    //case VK_DOWN: 
    case SDLK_DOWN: 
      *pnSliderValue -= 10;
      break;
    //case VK_UP: 
    case SDLK_UP: 
      *pnSliderValue += 10;
      break;
    //case VK_LEFT: 
    case SDLK_LEFT: 
      *pnSliderValue -= 1;
      break;
    //case VK_RIGHT: 
    case SDLK_RIGHT: 
      *pnSliderValue += 1;
      break;
    //case VK_HOME:
    case SDLK_HOME:
      *pnSliderValue = 0;
      break;
    //case VK_END:
    case SDLK_END:
      *pnSliderValue = 100;
      break;
    //case VK_RETURN:
    case SDLK_RETURN:
      m_pKeyDownFunction = BUI::StopUsingSlider();

      if(BGame::m_bMenuMode) {
        ReturnPressedOnCurrentMenu();
      }

      break;
    //case VK_ESCAPE:
    case SDLK_ESCAPE:
      *pnSliderValue = BUI::m_nPrevSliderValue;
      m_pKeyDownFunction = BUI::StopUsingSlider();      

      if(BGame::m_bMenuMode) {
        CancelPressedOnCurrentMenu();
      }

      break;
  }

  if(*pnSliderValue < 0) {
    *pnSliderValue = 0;
  }
  if(*pnSliderValue > 100) {
    *pnSliderValue = 100;
  }

  //Invalidate();
  //CView::OnKeyDown(nChar, nRepCnt, nFlags);
}





//*************************************************************************************************
void CPakoon1View::OnKeyDown(int nChar, int nRepCnt, int nFlags) {
  if(m_pKeyDownFunction) {
    // Call active draw function
    (this->*m_pKeyDownFunction)(nChar, nRepCnt, nFlags);
  }

}

//*************************************************************************************************
void CPakoon1View::OnKeyUp(int nChar, int nRepCnt, int nFlags) {

  // don't even ask about these...
  //if(nChar == VK_CONTROL) {
  if(nChar == SDLK_LCTRL) {
    g_bControl = false;
  }
  //if(nChar == VK_SHIFT) {
  if(nChar == SDLK_LSHIFT) {
    g_bShift = false;
  }

  if(nChar == ControllerModule::m_keymap.m_unAccelerate) {
    m_game.GetSimulation()->GetVehicle()->m_bAccelerating = false;
    m_game.GetSimulation()->GetVehicle()->m_dAccelerationFactor = 0.0;
    if(m_game.GetSimulation()->GetVehicle()->m_rotor.m_nHeliMode > 399) {
      m_game.GetSimulation()->GetVehicle()->m_rotor.m_bHeliForwarding = false;
    }
  } else if(nChar == ControllerModule::m_keymap.m_unReverse) {
    m_game.GetSimulation()->GetVehicle()->m_bReversing = false;
    if(m_game.GetSimulation()->GetVehicle()->m_rotor.m_nHeliMode > 399) {
      m_game.GetSimulation()->GetVehicle()->m_rotor.m_bHeliBacking = false;
    }
  } else if(nChar == ControllerModule::m_keymap.m_unPropeller) {
    m_game.GetSimulation()->GetVehicle()->m_bPropeller = false;
    m_game.GetSimulation()->GetVehicle()->m_rotor.m_bHeliLifting = false;
  } else if(nChar == ControllerModule::m_keymap.m_unPropellerReverse) {
    m_game.GetSimulation()->GetVehicle()->m_bPropReverse = false;
    m_game.GetSimulation()->GetVehicle()->m_rotor.m_bHeliDescending = false;
  } else if(nChar == ControllerModule::m_keymap.m_unBreak) {
    m_game.GetSimulation()->GetVehicle()->m_bBreaking = false;
  } else if(nChar == ControllerModule::m_keymap.m_unLeft) {
    m_game.GetSimulation()->GetVehicle()->m_bTurningLeft = false;
    if(m_game.GetSimulation()->GetVehicle()->m_rotor.m_nHeliMode > 399) {
      m_game.GetSimulation()->GetVehicle()->m_rotor.m_bHeliLefting = false;
    }
  } else if(nChar == ControllerModule::m_keymap.m_unRight) {
    m_game.GetSimulation()->GetVehicle()->m_bTurningRight = false;
    if(m_game.GetSimulation()->GetVehicle()->m_rotor.m_nHeliMode > 399) {
      m_game.GetSimulation()->GetVehicle()->m_rotor.m_bHeliRighting = false;
    }
  } else if(nChar == ControllerModule::m_keymap.m_unLift) {
    m_game.GetSimulation()->m_bLiftingUp = false;
  } else {
    switch(nChar) {
      //case VK_SPACE:
      case SDLK_SPACE:
        m_game.GetSimulation()->GetVehicle()->m_bHandBreaking = false;
        break;
    }
  }
  //CView::OnKeyUp(nChar, nRepCnt, nFlags);
}





//*************************************************************************************************
//void CPakoon1View::ProcessMouseInput(int nFlags, CPoint point) {
void CPakoon1View::ProcessMouseInput(MouseFlags nFlags, SDL_Point point) {
  //if(nFlags & MK_LBUTTON) {
  if(nFlags == MOUSE_DOWN) {
    //CRect rectWnd;
    //GetClientRect(&rectWnd);
    point.y = window_height - point.y;
    if(BGame::m_bNavSat) {
      // See if navsat resolution is to be changed
      if((point.x > (135 - 60)) && 
         (point.x < (135 + 60)) && 
         (point.y > 260) && 
         (point.y < (260 + 100))) {
        double dCurAngle = BGame::m_dNavSatHandleAngle;
        double dNewAngle;
        // possibly change resolution
        BVector vUp(0, 1, 0);
        BVector vRight(1, 0, 0);
        BVector vToMouse(point.x - 135.0, point.y - 187, 0);
        vToMouse.ToUnitLength();
        double dCos = vUp.ScalarProduct(vToMouse);
        double dTmp = vRight.ScalarProduct(vToMouse);
        if(dTmp < 0.0) {
          dNewAngle = acos(dCos);
        } else {
          dNewAngle = -acos(dCos);
        }
        dNewAngle = dNewAngle / 3.141592654 * 180.0;
        if(dNewAngle > 0.0) {
          dNewAngle = double(int((dNewAngle + 5.0) / 10.0)) * 10.0;
        } else {
          dNewAngle = double(int((dNewAngle - 5.0) / 10.0)) * 10.0;
        }
        if(fabs(dNewAngle) > 20.0) {
          dNewAngle = fabs(dNewAngle) / dNewAngle * 20.0;
        }
        if(dNewAngle != dCurAngle) {
          // Change to new resolution
          BGame::m_dNavSatHandleAngle = dNewAngle;
          if(dNewAngle == -20.0) {
            BGame::Command()->Run("set navsat resolution 650");    
          } else if(dNewAngle == -10.0) {
            BGame::Command()->Run("set navsat resolution 1300");    
          } else if(dNewAngle == 0.0) {
            BGame::Command()->Run("set navsat resolution 2600");    
          } else if(dNewAngle == 10.0) {
            BGame::Command()->Run("set navsat resolution 5200");    
          } else if(dNewAngle == 20.0) {
            BGame::Command()->Run("set navsat resolution 10400");
          }
        }
      }
    }
    if(BGame::m_bService) {
      // See if navsat resolution is to be changed
      //if((point.y > rectWnd.Height() - (135 + 60)) && 
      if((point.y > window_height - (135 + 60)) && 
         //(point.y < rectWnd.Height() - (135 - 60)) && 
         (point.y < window_height - (135 - 60)) && 
         //(point.x > rectWnd.Width() - (360 + 100)) && 
         (point.x > window_width - (360 + 100)) && 
         //(point.x < rectWnd.Width() - (360))) {
         (point.x < window_width - (360))) {
        double dCurAngle = BGame::m_dServiceHandleAngle;
        double dNewAngle;
        // possibly change resolution
        BVector vUp(-1, 0, 0);
        BVector vRight(0, 1, 0);
        //BVector vToMouse(point.x - (rectWnd.Width() - 287), point.y - (rectWnd.Height() - 135.0), 0);
        BVector vToMouse(point.x - (window_width - 287), point.y - (window_height - 135.0), 0);
        vToMouse.ToUnitLength();
        double dCos = vUp.ScalarProduct(vToMouse);
        double dTmp = vRight.ScalarProduct(vToMouse);
        if(dTmp < 0.0) {
          dNewAngle = acos(dCos);
        } else {
          dNewAngle = -acos(dCos);
        }
        dNewAngle = dNewAngle / 3.141592654 * 180.0;
        if(fabs(dNewAngle) > 20.0) {
          dNewAngle = fabs(dNewAngle) / dNewAngle * 20.0;
        }
        BGame::m_dServiceHandleAngle = dNewAngle;
      }
    }
  }
}



//*************************************************************************************************
//void CPakoon1View::OnMouseMove(int nFlags, CPoint point) {
void CPakoon1View::OnMouseMove(SDL_Point point) {
	mouse_x = point.x;
	mouse_y = point.y;
  if((m_pDrawFunction == &CPakoon1View::OnDrawGame) && 
     (BGame::m_bNavSat || BGame::m_bService)) {
    //ProcessMouseInput(nFlags, point);
    ProcessMouseInput(MOUSE_NONE, point);
    return;
  }
}


bool g_bMouseButtonDown = false;


//*************************************************************************************************
//void CPakoon1View::OnLButtonDown(int nFlags, CPoint point) {
void CPakoon1View::OnLButtonDown(SDL_Point point) {
  g_bMouseButtonDown = true;
  if((m_pDrawFunction == &CPakoon1View::OnDrawGame) && 
     (BGame::m_bNavSat || BGame::m_bService)) {
    //ProcessMouseInput(nFlags, point);
    ProcessMouseInput(MOUSE_DOWN, point);
    return;
  }
}

//*************************************************************************************************
//void CPakoon1View::OnLButtonUp(int nFlags, CPoint point) {
void CPakoon1View::OnLButtonUp(SDL_Point point) {
  g_bMouseButtonDown = false;
}

void CPakoon1View::OnFingerDown(float x, float y, int finger_id) {
}

void CPakoon1View::OnFingerUp(float x, float y, int finger_id) {
  SDL_Point point;
  point.x = x * window_width;
  point.y = y * window_height;
  HandleBUITouch(point);
}

void CPakoon1View::HandleBUITouch(SDL_Point point) {
  if (BGame::m_bMenuMode) {
    BUISelectionList *pList = BUI::GetActiveSelectionList();
    if (!pList) {
      pList = &BGame::m_pMenuCurrent->m_listMenu;
    }
    double dX = window_width / 2;
    double dY = window_height / 2;

    // Check if we have open submenu and need position of it
    bool bScrolling = false;
    string sTmp;
    int nSelected = BGame::m_pMenuCurrent->m_listMenu.GetSelectedItem(sTmp);
    if (nSelected != -1) {
      BMenuItem *pMenuItem = &(BGame::m_pMenuCurrent->m_items[nSelected]);
      if (pMenuItem->m_bOpen && pMenuItem->m_type == BMenuItem::STRING_FROM_LIST) {
        double dCharHeight = BUI::TextRenderer()->GetCharHeight();
        // Values from CPakoon1View::DrawMenu
        dX += 35;
        dY -= double(nSelected) * -dCharHeight + (dCharHeight * double(BGame::m_pMenuCurrent->m_nItems)) / 2.0;
        bScrolling = true;
      }
    }

    bool menu_item_pressed = pList->OnFingerUp(point.x, point.y, dX, dY, bScrolling);
    if (menu_item_pressed) {
      ReturnPressedOnCurrentMenu();
    } else {
      CancelPressedOnCurrentMenu();
    }

    nSelected = BGame::m_pMenuCurrent->m_listMenu.GetSelectedItem(sTmp);
    if(nSelected != -1) {
      BMenuItem *pMenuItem = &(BGame::m_pMenuCurrent->m_items[nSelected]);
      if (pMenuItem->m_bOpen && pMenuItem->m_type == BMenuItem::SLIDER) {
        *BUI::m_pnSliderValue = point.x - dX - 10;
        if (*BUI::m_pnSliderValue < 0) {
          *BUI::m_pnSliderValue = 0;
        }
        if (*BUI::m_pnSliderValue > 100) {
          *BUI::m_pnSliderValue = 100;
        }

        // closing slider as no need to keep it open
        ReturnPressedOnCurrentMenu();
      }
    }
  } else {
    // Using values from OnDrawGame
    if (m_game.m_bShowGameMenu) {
      double dHeight = 256.0 / 5.0;
      int i = (point.y - (window_height / 2.0 - 128)) / dHeight;
      m_game.m_nGameMenuSelection = 5 - i;
      OnKeyDownGameMenu(SDLK_RETURN, 0, 0);
    } else if (m_game.m_bShowCancelQuestion) {
      int win_w_center = window_width / 2.0 - 235.0 / 2.0 + 70.0;
      int win_h_center = window_height / 2.0 + 50 + 50;
      if (win_h_center - 37.0 < point.y && win_h_center > point.y) {
        if (win_w_center + (0 - 40.0) < point.x && win_w_center + (80.0 - 40.0) > point.x) {
          m_game.m_nYesNoSelection = 1;
          OnKeyDownCancelQuestion(SDLK_RETURN, 0, 0);
        }
        if (win_w_center + (0 + 40.0) < point.x && win_w_center + (86.0 + 40.0) > point.x) {
          m_game.m_nYesNoSelection = 2;
          OnKeyDownCancelQuestion(SDLK_RETURN, 0, 0);
        }
      }
    } else if (m_game.m_bFueling) {
      double dCharHeight = BUI::TextRenderer()->GetCharHeight();
      double dCharWidth  = BUI::TextRenderer()->GetCharWidth();
      int win_w_center = window_width / 2.0;
      int win_h_center = window_height / 2.0 + dCharHeight * 1.5 + 54.0 / 2.0;

      if (
          win_w_center - 120.0 / 2.0 < point.x && win_w_center + 120.0 / 2.0 > point.x
          && win_h_center - 54.0 / 2.0 < point.y && win_h_center + 54.0 / 2.0 > point.y
      ) {
        m_game.m_nFuelSelect = 2;
        OnKeyDownFueling(SDLK_RETURN, 0, 0);
      }

      win_h_center = window_height / 2.0 - dCharHeight * 2.5 - 128.0 / 2.0;
      if (win_h_center - 128.0 / 2.0 < point.y && win_h_center + 128.0 / 2.0 > point.y) {
        win_w_center = window_width / 2.0 - 70.0;
        if (win_w_center - 128.0 / 2.0 < point.x && win_w_center + 128.0 / 2.0 > point.x) {
          m_game.m_nFuelSelect = 0;
          OnKeyDownFueling(SDLK_RETURN, 0, 0);
        }
        win_w_center = window_width / 2.0 + 70.0;
        if (win_w_center - 128.0 / 2.0 < point.x && win_w_center + 128.0 / 2.0 > point.x) {
          m_game.m_nFuelSelect = 1;
          OnKeyDownFueling(SDLK_RETURN, 0, 0);
        }
      }
    } else if (m_game.m_bShowQuickHelp || m_game.m_bPickupStartInProgress || m_game.m_bDeliveryStartInProgress) {
      OnKeyDownGame(SDLK_ESCAPE, 0, 0);
    }
  }
}

//*************************************************************************************************
//void CPakoon1View::DrawMouseCursor(CRect &rectWnd) {
void CPakoon1View::DrawMouseCursor() {
  //POINT pntMouse;
  SDL_Rect pntMouse;
  //GetCursorPos(&pntMouse);
  pntMouse.x = mouse_x;
  pntMouse.y = mouse_y;
  //pntMouse.y = rectWnd.Height() - pntMouse.y;
  pntMouse.y = window_height - pntMouse.y;

  double dX = 458.0 / 512.0;
  if(g_bMouseButtonDown) {
    dX = 407.0 / 512.0;
  }

  glPushMatrix();
  glTranslatef(pntMouse.x -10, pntMouse.y + 10, 0);
  OpenGLHelpers::SetColor(1, 1, 1, 1);
  OpenGLHelpers::SwitchToTexture(0);
  //BTextures::Use(BTextures::Texture::PANEL);
  BTextures::Use(BTextures::PANEL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // not mipmapped
  glBegin(GL_TRIANGLE_STRIP);
  OpenGLHelpers::SetTexCoord(dX, 54.0 / 512);
  glVertex3f(0, 0, 0);
  OpenGLHelpers::SetTexCoord(dX, 0.0 / 512);
  glVertex3f(0, -54, 0);
  OpenGLHelpers::SetTexCoord(dX + (54.0 / 512.0), 54.0 / 512);
  glVertex3f(54, 0, 0);
  OpenGLHelpers::SetTexCoord(dX + (54.0 / 512.0), 0.0 / 512);
  glVertex3f(54, -54, 0);
  glEnd();
  glPopMatrix();
}


void CPakoon1View::setExit() {
	if(m_pDrawFunction == &CPakoon1View::OnDrawGame) {
		BGame::GetPlayer()->m_dFuel = BGame::GetSimulation()->GetVehicle()->m_dFuel;
		BGame::GetPlayer()->m_dKerosine = BGame::GetSimulation()->GetVehicle()->m_dKerosine;
		BGame::GetPlayer()->SaveStateFile();
		BGame::GetPlayer()->SaveCurrentSceneInfo();
	}
	
	// Write settings for graphics etc.
	BGame::GetPlayer()->SaveStateFile();
	Settings::WriteSettings(m_game.GetSimulation());
	exit = true;
}

/*

  
          // Take controller into use, if not done already
          if(!ControllerModule::m_bCurrentInitialized) {
            ControllerModule::SwitchToController(ControllerModule::m_nCurrent);
          }

          // Start game
          SoundModule::StartSkidSound();
          SoundModule::StartEngineSound();
          m_game.GetSimulation()->GetCamera()->m_bNeedsToBeInitialized = true;

          // Help (invocation technique courtesy of Mr. Jacob Cody)
          {
            char sCurDir[1024];
            GetCurrentDirectory(1024, sCurDir);
            string sHelpPath = sCurDir;
            sHelpPath += "\\Help\\help.html";
            HINSTANCE hi;
            hi = ShellExecute(::GetDesktopWindow(), 
                              NULL, 
                              sHelpPath, 
                              NULL, 
                              NULL, 
                              SW_MAXIMIZE);
          }
*/


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
