//
// Pakoon1View.h : interface of the CPakoon1View class
//
// (c) Copyright 2002, Mikko Oksalahti (see end of file for details)
//

#if !defined(AFX_Pakoon1VIEW_H__73E6FC07_BC82_11D4_B532_0060B0F1F5DD__INCLUDED_)
#define AFX_Pakoon1VIEW_H__73E6FC07_BC82_11D4_B532_0060B0F1F5DD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>

using namespace std;

#include <SDL2/SDL.h>

//#include "STRMIF.H"
//#include "CONTROL.H"
//#include "UUIDS.H"
#include "BaseClasses.h"
#include "BGame.h"
#include "BMessages.h"
#include "Pakoon1Doc.h"

class BMenuItemOBSOLETE {
public:
  //CRect   m_rect;
  bool    m_bActive;
  int     m_nLight;
  //BMenuItemOBSOLETE() {m_rect.SetRectEmpty(); m_bActive = false; m_nLight = 0;}
};


//class CPakoon1View : public CView {
class CPakoon1View {
	CPakoon1Doc m_pDocument;

  void InitVideo();
  void DrawVideo();
  bool m_bShowVideo;

  //CRect     m_rectWnd;
  BGame     m_game;
  BMessages m_messages;

  bool    m_bInitClock;

  bool    m_bWireframe;
  bool    m_bNormals;
  bool    m_bCreateDLs;

  //HCURSOR m_hCursor;

  BMenuItemOBSOLETE m_miMenu[7];

  //HGLRC   m_hGLRC;
public:
  clock_t m_nMenuTime; // Time spent viewing menu
  
  int window_width;
  int window_height;
  
  int mouse_x;
  int mouse_y;
  
  enum MouseFlags {MOUSE_NONE, MOUSE_DOWN, MOUSE_UP};
    
  bool exit;
private:

  int     m_chaMenuMusic; // Channel for the menu background music

  bool    m_bIgnoreNextChar;

  //CWinThread* m_pThreadLoading;
  SDL_Thread *m_pThreadLoading;

  enum TMenuScroll {SCROLL_LEFT, SCROLL_RIGHT, SCROLL_UP, SCROLL_DOWN};

  TMenuScroll m_scrollDir;
  clock_t     m_clockMenuScroll;

  //void ProcessMouseInput(int nFlags, CPoint point);
  void ProcessMouseInput(MouseFlags nFlags, SDL_Point point);

  void StartMenuScroll(TMenuScroll scroll);
  void DrawMenu(BMenu *pMenu);
  void DrawMenuItemTextAtRelPos(int nX, int nY, int nItems, int nIndex, BMenuItem *pMenuItem);
  void DrawMenuItemSliderAtRelPos(int nX, int nY, int nItems, int nIndex, BMenuItem *pMenuItem);

  //void DrawMenuItem(CDC* pDC, CFont *pFont, int m, int nY, string sText, COLORREF color, CRect rectWnd);
public:

  bool m_bDrawOnlyMenu;
  bool m_bFullRedraw;

  //void OnDrawIntro(CDC* pDC);     // OBSOLETE
  void OnDrawIntro();     // OBSOLETE
  //void OnDrawCurrentMenu(CDC* pDC);
  void OnDrawCurrentMenu();
  //void OnDrawGame(CDC* pDC);
  void OnDrawGame();

  void OnKeyDownGame          (int nChar, int nRepCnt, int nFlags);
  void OnKeyDownGameMenu      (int nChar, int nRepCnt, int nFlags);
  void OnKeyDownFueling       (int nChar, int nRepCnt, int nFlags);
  void OnKeyDownCancelQuestion(int nChar, int nRepCnt, int nFlags);
  void OnKeyDownSceneEditor   (int nChar, int nRepCnt, int nFlags);
  void OnKeyDownSelectionList (int nChar, int nRepCnt, int nFlags);
  void OnKeyDownSlider        (int nChar, int nRepCnt, int nFlags);
  void OnKeyDownCurrentMenu   (int nChar, int nRepCnt, int nFlags);

  void ReturnPressedOnCurrentMenu();
  void CancelPressedOnCurrentMenu();

  void CheckForFueling();
  void CheckForPickup();
  void CheckForDelivery();

  void Setup2DRendering();
  void End2DRendering();
  //void DrawDashboard(CDC* pDC);
  void DrawDashboard();
  //void DrawFuelingButtons(CDC* pDC, CRect &rectWnd);
  void DrawFuelingButtons();
  //void DrawNavSat(CDC* pDC);
  void DrawNavSat();
  //void DrawQuickHelp(CDC* pDC);
  void DrawQuickHelp();
  //void DrawServiceWnd(CDC* pDC);
  void DrawServiceWnd();
  //void DrawServiceWndTexts(CRect &rectWnd);
  void DrawServiceWndTexts();
  void DrawSrvText(string sText);
  //void DrawPickupInfo(CDC* pDC, CRect &rectWnd);
  void DrawPickupInfo();
  //void DrawDeliveryInfo(CDC* pDC, CRect &rectWnd);
  void DrawDeliveryInfo();
  //void DrawTrackingTarget(BTrackingTarget *pTarget, CRect &rectWnd);
  void DrawTrackingTarget(BTrackingTarget *pTarget);
  //void UpdateTrackingTargetScreenPos(BTrackingTarget *pTarget, CRect &rectWnd);
  void UpdateTrackingTargetScreenPos(BTrackingTarget *pTarget);
  //void DrawTrackingTargetOnScreen(BTrackingTarget *pTarget, CRect &rectWnd);
  void DrawTrackingTargetOnScreen(BTrackingTarget *pTarget);
  void DrawNavSatIndicator(bool bCar, 
                           double dAngle, 
                           double dR, 
                           double dG, 
                           double dB, 
                           double dAlpha, 
                           //BVector vLoc, 
                           BVector vLoc);
                           //CRect &rectWnd);
  void DrawPanel(double dWidth, 
                 double dHeight, 
                 double dRed = 0, 
                 double dGreen = 0, 
                 double dBlue = 0, 
                 double dAlpha = 1);

  //void DrawMouseCursor(CRect &rectWnd);
  void DrawMouseCursor();
  BVector ColorForGForce(double dGForce);

private:

  //IMediaControl *m_pMediaControl;

//protected: // create from serialization only
public:
  CPakoon1View();
  //DECLARE_DYNCREATE(CPakoon1View)

// Attributes
public:
  //void (CPakoon1View::*m_pDrawFunction)(CDC*);
  void (CPakoon1View::*m_pDrawFunction)();
  void (CPakoon1View::*m_pKeyDownFunction)(int, int, int);

  //CPakoon1Doc* GetDocument();

// Operations
public:

// Overrides
// ClassWizard generated virtual function overrides
//{{AFX_VIRTUAL(CPakoon1View)
public:
//virtual void OnDraw(CDC* pDC);  // overridden to draw this view
virtual void OnDraw();  // overridden to draw this view
//virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
virtual bool PreCreateWindow();
protected:
//}}AFX_VIRTUAL

void InitializeOpenGL();

// Implementation
public:
  virtual ~CPakoon1View();
  #ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext& dc) const;
  #endif

protected:

// Generated message map functions
//protected:
public:
  //{{AFX_MSG(CPakoon1View)
  //FIXME
  //afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
  int  OnCreate();
  //afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  bool OnEraseBkgnd();
  //afx_msg void OnDestroy();
  void OnDestroy();
  //afx_msg void OnChar(int nChar, int nRepCnt, int nFlags);
  void OnChar(int nChar, int nRepCnt, int nFlags);
  //afx_msg void OnKeyDown(int nChar, int nRepCnt, int nFlags);
  void OnKeyDown(int nChar, int nRepCnt, int nFlags);
  //afx_msg void OnKeyUp(int nChar, int nRepCnt, int nFlags);
  void OnKeyUp(int nChar, int nRepCnt, int nFlags);
  //afx_msg void OnMouseMove(int nFlags, CPoint point);
  void OnMouseMove(SDL_Point point);
  //afx_msg void OnLButtonDown(int nFlags, CPoint point);
  void OnLButtonDown(SDL_Point point);
  //afx_msg void OnLButtonUp(int nFlags, CPoint point);
  void OnLButtonUp(SDL_Point point);
  //}}AFX_MSG
  //DECLARE_MESSAGE_MAP()
  
	CPakoon1Doc* GetDocument() {
		return (CPakoon1Doc*) &m_pDocument;
	}
	
	SDL_Window *window;
	
	void setWindow(SDL_Window *window) {
		this->window = window;
	}
	
	void setExit() {
		exit = true;
	}
	
	bool isExit() {
		return exit;
	}
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_Pakoon1VIEW_H__73E6FC07_BC82_11D4_B532_0060B0F1F5DD__INCLUDED_)



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
