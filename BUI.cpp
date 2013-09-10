//
// BUI: Misc UI components
//
// (c) Copyright 2002, Mikko Oksalahti (see end of file for details)
//

#include <SDL2/SDL.h>

//#include "stdafx.h"
#include "BUI.h"
#include "BSimulation.h"
#include "BTextRenderer.h"
#include "BGame.h"
#include "Pakoon1Doc.h"
#include "Pakoon1View.h"

BTextRenderer BUI::m_textRenderer;
BUISelectionList *BUI::m_pSelList = 0;
void (CPakoon1View::* (BUI::m_pPrevKeyDownFunction))(int, int, int) = 0;
int BUI::m_nPrevSliderValue = 0;
int *BUI::m_pnSliderValue = 0;

//*************************************************************************************************
void BUI::StartUsingSelectionList(BUISelectionList *pList, void (CPakoon1View::*pPrevKeyDownFunction)(int, int, int)) {
  m_pSelList = pList;
  pList->SaveSelection();
  m_pPrevKeyDownFunction = pPrevKeyDownFunction;
  CPakoon1View *pView = BGame::GetView();
  pView->m_pKeyDownFunction = &CPakoon1View::OnKeyDownSelectionList;

}

//*************************************************************************************************
void BUI::StartUsingSlider(int *pnSliderValue, void (CPakoon1View::*pPrevKeyDownFunction)(int, int, int)) {
  m_pnSliderValue = pnSliderValue;
  m_nPrevSliderValue = *pnSliderValue;
  m_pPrevKeyDownFunction = pPrevKeyDownFunction;
  CPakoon1View *pView = BGame::GetView();
  pView->m_pKeyDownFunction = &CPakoon1View::OnKeyDownSlider;

}


//*************************************************************************************************
BUISelectionList::BUISelectionList() {
  m_nSelected = -1;
  m_psItems = 0;
  m_nItems = 0;
  m_sPrompt = "";
}


//*************************************************************************************************
void BUISelectionList::SaveSelection() {
  m_nSavedSelection = m_nSelected;
}

//*************************************************************************************************
void BUISelectionList::Cancel() {
  m_nSelected = m_nSavedSelection;
}


//*************************************************************************************************
//void BUISelectionList::SetItems(CString *psItems, int nItems, CString sPrompt) {
void BUISelectionList::SetItems(string *psItems, int nItems, string sPrompt) {
  m_psItems = psItems;
  m_nItems = nItems;
  m_sPrompt = sPrompt;
}


//*************************************************************************************************
//int BUISelectionList::SelectItem(CString sItem) {
int BUISelectionList::SelectItem(string sItem) {
  // Search for the string and select that item.
  // Return selected item's index
  for(int i = 0; i < m_nItems; ++i) {
    //if(sItem.CompareNoCase(m_psItems[i]) == 0) {
    if(sItem.compare(m_psItems[i]) == 0) {
      m_nSelected = i;
      return i;
    }
  }
  m_nSelected = -1;
  return -1;
}


//*************************************************************************************************
void BUISelectionList::AdvanceSelection(int nAmount) {
  m_nSelected += nAmount;
  if(m_nSelected < -100) {
    m_nSelected = 0;
  }
  if(m_nSelected < 0) {
    m_nSelected = (m_nItems - 1);
  }
  if(m_nSelected > (m_nItems + 100)) {
    m_nSelected = (m_nItems - 1);
  }
  if(m_nSelected > (m_nItems - 1)) {
    m_nSelected = 0;
  }
}


//*************************************************************************************************
//int BUISelectionList::GetSelectedItem(CString &sItemText) {
int BUISelectionList::GetSelectedItem(string &sItemText) {
  if(m_nSelected != -1) {
    sItemText = m_psItems[m_nSelected];
  }
  return m_nSelected;
}

//*************************************************************************************************
void BUISelectionList::DrawAt(double dX, 
                              double dY, 
                              BTextRenderer::TTextAlign textAlign,
                              double dRed, 
                              double dGreen, 
                              double dBlue,
                              bool bScrolling,
                              bool bWithBackground) {
  double dCharHeight = BUI::TextRenderer()->GetCharHeight();
  double dCharWidth  = BUI::TextRenderer()->GetCharWidth();

  glPushMatrix();

  glTranslated(dX, dY, 0);
  if(bScrolling && (m_nSelected != -1)) {
    glTranslated(0, dCharHeight * double(m_nSelected), 0);
  } else {
    glTranslated(0, (dCharHeight * double(m_nItems)) / 2.0, 0);
  }

  // Draw background, if requested
  int i;
  if(bWithBackground) {
    double dMaxLen = 0.0;
    for(i = 0; i < m_nItems; ++i) {
      //if((double(m_psItems[i].GetLength()) * dCharWidth) > dMaxLen) {
      if((double(m_psItems[i].length()) * dCharWidth) > dMaxLen) {
        //dMaxLen = double(m_psItems[i].GetLength()) * dCharWidth;
        dMaxLen = double(m_psItems[i].length()) * dCharWidth;
      }
    }
    double dXOffset = 0;
    //if(textAlign == BTextRenderer::TTextAlign::ALIGN_CENTER) {
    if(textAlign == BTextRenderer::ALIGN_CENTER) {
      dXOffset = -dMaxLen / 2.0;
    //} else if(textAlign == BTextRenderer::TTextAlign::ALIGN_RIGHT) {
    } else if(textAlign == BTextRenderer::ALIGN_RIGHT) {
      dXOffset = -dMaxLen;
    }

    double dYBase = double(m_nItems - 1) * -dCharHeight;

    // Shadow
    glTranslated(6, -6, 0);
    OpenGLHelpers::SetColorFull(0, 0, 0, 0.5);
    glBegin(GL_QUADS);
    glVertex3f(-5 + dXOffset, dYBase - dCharHeight / 2.0, 0);
    glVertex3f(-5 + dXOffset, dYBase + double(m_nItems - 1) * dCharHeight + dCharHeight / 2.0, 0);
    glVertex3f(dXOffset + dMaxLen + 6, dYBase  + double(m_nItems - 1) * dCharHeight + dCharHeight / 2.0, 0);
    glVertex3f(dXOffset + dMaxLen + 6, dYBase - dCharHeight / 2.0, 0);
    glEnd();

    // Actual background
    glTranslated(-6, 6, 0);

    OpenGLHelpers::SetColorFull(0.15, 0.15, 0.15, 1);
    glBegin(GL_QUADS);
    glVertex3f(-5 + dXOffset, dYBase - dCharHeight / 2.0, 0);
    glVertex3f(-5 + dXOffset, dYBase + double(m_nItems - 1) * dCharHeight + dCharHeight / 2.0, 0);
    glVertex3f(dXOffset + dMaxLen + 6, dYBase  + double(m_nItems - 1) * dCharHeight + dCharHeight / 2.0, 0);
    glVertex3f(dXOffset + dMaxLen + 6, dYBase - dCharHeight / 2.0, 0);
    glEnd();

    OpenGLHelpers::SetColorFull(1, 1, 1, 1);
    glBegin(GL_LINE_STRIP);
    glVertex3f(-5 + dXOffset, dYBase - dCharHeight / 2.0, 0);
    glVertex3f(-5 + dXOffset, dYBase + double(m_nItems - 1) * dCharHeight + dCharHeight / 2.0, 0);
    glVertex3f(dXOffset + dMaxLen + 6, dYBase  + double(m_nItems - 1) * dCharHeight + dCharHeight / 2.0, 0);
    glVertex3f(dXOffset + dMaxLen + 6, dYBase - dCharHeight / 2.0, 0);
    glVertex3f(-5 + dXOffset, dYBase - dCharHeight / 2.0, 0);
    glEnd();
  }

  // First draw selection rectangle behind the selected item
  if(m_nSelected != -1) {
    glDisable(GL_TEXTURE_2D);
    //double dLen = double(m_psItems[m_nSelected].GetLength()) * dCharWidth;
    double dLen = double(m_psItems[m_nSelected].length()) * dCharWidth;
    double dYBase = double(m_nSelected) * -dCharHeight;

    double dXOffset = 0;
    //if(textAlign == BTextRenderer::TTextAlign::ALIGN_CENTER) {
    if(textAlign == BTextRenderer::ALIGN_CENTER) {
      dXOffset = -dLen / 2.0;
    //} else if(textAlign == BTextRenderer::TTextAlign::ALIGN_RIGHT) {
    } else if(textAlign == BTextRenderer::ALIGN_RIGHT) {
      dXOffset = -dLen;
    }

    double dAlpha = fabs(double(clock() % CLOCKS_PER_SEC) - (double(CLOCKS_PER_SEC) / 2.0)) / (double(CLOCKS_PER_SEC) / 2.0);

    glTranslated(0, -1, 0);
    glBegin(GL_QUADS);
    OpenGLHelpers::SetColorFull(0, 0, 0, 0.95 * dAlpha);
    glVertex3f(-5 + dXOffset, dYBase + -dCharHeight / 2.0, 0);
    OpenGLHelpers::SetColorFull(0.5, 0.5, 1, 0.5 * dAlpha);
    glVertex3f(-5 + dXOffset, dYBase + dCharHeight / 2.0, 0);
    OpenGLHelpers::SetColorFull(0.25, 0.25, 1, 0.5 * dAlpha);
    glVertex3f(dXOffset + dLen + 5, dYBase + dCharHeight / 2.0, 0);
    OpenGLHelpers::SetColorFull(0, 0, 0, 0.95 * dAlpha);
    glVertex3f(dXOffset + dLen + 5, dYBase + -dCharHeight / 2.0, 0);
    glEnd();

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    OpenGLHelpers::SetColorFull(1, 1, 1, 0.4);
    glBegin(GL_QUADS);
    glVertex3f(-5 + dXOffset, dYBase + -dCharHeight / 2.0, 0);
    glVertex3f(-5 + dXOffset, dYBase + dCharHeight / 2.0, 0);
    glVertex3f(dXOffset + dLen + 5, dYBase + dCharHeight / 2.0, 0);
    glVertex3f(dXOffset + dLen + 5, dYBase + -dCharHeight / 2.0, 0);
    glEnd();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glTranslated(0, 1, 0);
  }

  // Then draw items
  BUI::TextRenderer()->StartRenderingText();
  for(i = 0; i < m_nItems; ++i) {

    //double dLen = double(m_psItems[i].GetLength()) * dCharWidth;
    double dLen = double(m_psItems[i].length()) * dCharWidth;
    double dXOffset = 0;
    //if(textAlign == BTextRenderer::TTextAlign::ALIGN_CENTER) {
    if(textAlign == BTextRenderer::ALIGN_CENTER) {
      dXOffset = -dLen / 2.0;
    //} else if(textAlign == BTextRenderer::TTextAlign::ALIGN_RIGHT) {
    } else if(textAlign == BTextRenderer::ALIGN_RIGHT) {
      dXOffset = -dLen;
    }

    BUI::TextRenderer()->DrawTextAt(0, // dXOffset, 
                                    double(i) * -dCharHeight, 
                                    m_psItems[i],
                                    textAlign,
                                    dRed,
                                    dGreen,
                                    dBlue);
  }
  BUI::TextRenderer()->StopRenderingText();
  glPopMatrix();
}




//*************************************************************************************************
BUIEdit::BUIEdit() {
  m_nMaxLength = 0;
  m_sValue = "";
  m_sPrompt = "";
}


//*************************************************************************************************
//void BUIEdit::Setup(CString sPrompt, CString sValue, int nMaxLength) {
void BUIEdit::Setup(string sPrompt, string sValue, int nMaxLength) {
  m_sPrompt = sPrompt;
  m_sValue = sValue;
  m_nMaxLength = nMaxLength;
  status = EDITING;
}


//*************************************************************************************************
void BUIEdit::ProcessChar(unsigned char c) {
  if(!((toupper(c) >= 32) && 
       (toupper(c) <= 96) || 
       (c == '.') ||
       (c == '/') ||
       //(c == VK_BACK) ||
       (c == SDLK_BACKSPACE) ||
       //(c == VK_RETURN) || 
       (c == SDLK_RETURN) || 
       //(c == VK_ESCAPE))) {
       (c == SDLK_ESCAPE))) {
    // Bad character, no go
    return;
  }

  //if(c == VK_BACK) {
  if(c == SDLK_BACKSPACE) {
    // Eat one character
    //if(m_sValue.GetLength() > 0) {
    if(m_sValue.length() > 0) {
      //m_sValue.Delete(m_sValue.GetLength() - 1, 1);
      m_sValue.erase(m_sValue.length() - 1, 1);
    }
    return;
  }

  //if(c == VK_RETURN) {
  if(c == SDLK_RETURN) {
    status = READY;
    return;
  }

  //if(c == VK_ESCAPE) {
  if(c == SDLK_ESCAPE) {
    status = CANCELED;
    return;
  }

  // Add writable character
  //if(m_sValue.GetLength() < m_nMaxLength) {
  if(m_sValue.length() < m_nMaxLength) {
    m_sValue += c;
  }
  return;
}


//*************************************************************************************************
//CString BUIEdit::GetValue(TStatus &rStatus) {
string BUIEdit::GetValue(TStatus &rStatus) {
  rStatus = status;
  return m_sValue;
}


//*************************************************************************************************
void BUIEdit::DrawAt(double dX, double dY, bool bCursor, BTextRenderer::TTextAlign textAlign) {
  double dCharHeight = BUI::TextRenderer()->GetCharHeight();
  double dCharWidth  = BUI::TextRenderer()->GetCharWidth();

  glPushMatrix();

  glTranslated(dX, dY, 0);

  //if(textAlign == BTextRenderer::TTextAlign::ALIGN_LEFT) {
  if(textAlign == BTextRenderer::ALIGN_LEFT) {
    //glTranslated(dCharWidth * double(m_sPrompt.GetLength()), 0, 0);
    glTranslated(dCharWidth * double(m_sPrompt.length()), 0, 0);
  }

  BUI::TextRenderer()->StartRenderingText();

  // Prompt
  BUI::TextRenderer()->DrawTextAt(0, 
                                  0, 
                                  m_sPrompt,
                                  //BTextRenderer::TTextAlign::ALIGN_RIGHT,
                                  BTextRenderer::ALIGN_RIGHT,
                                  1,
                                  0.75,
                                  0.5);


  // Value
  BUI::TextRenderer()->DrawTextAt(0, 
                                  0, 
                                  m_sValue,
                                  //BTextRenderer::TTextAlign::ALIGN_LEFT,
                                  BTextRenderer::ALIGN_LEFT,
                                  1,
                                  1,
                                  1);

  BUI::TextRenderer()->StopRenderingText();

  // Cursor

  glDisable(GL_TEXTURE_2D);
  //double dXOffset = double(m_sValue.GetLength()) * dCharWidth;
  double dXOffset = double(m_sValue.length()) * dCharWidth;
  double dLen = dCharWidth;

  double dAlpha = fabs(double(clock() % CLOCKS_PER_SEC) - (double(CLOCKS_PER_SEC) / 2.0)) / (double(CLOCKS_PER_SEC) / 2.0);

  glTranslated(-1, -1, 0);
  glBegin(GL_QUADS);
  OpenGLHelpers::SetColorFull(0, 0, 0, 0.95 * dAlpha);
  glVertex3f(dXOffset, -dCharHeight / 2.0, 0);
  OpenGLHelpers::SetColorFull(0.5, 0.5, 1, 0.5 * dAlpha);
  glVertex3f(dXOffset, dCharHeight / 2.0, 0);
  OpenGLHelpers::SetColorFull(0.25, 0.25, 1, 0.5 * dAlpha);
  glVertex3f(dXOffset + dLen, dCharHeight / 2.0, 0);
  OpenGLHelpers::SetColorFull(0, 0, 0, 0.95 * dAlpha);
  glVertex3f(dXOffset + dLen, -dCharHeight / 2.0, 0);
  glEnd();

  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  OpenGLHelpers::SetColorFull(1, 1, 1, 0.4);
  glBegin(GL_QUADS);
  glVertex3f(dXOffset, -dCharHeight / 2.0, 0);
  glVertex3f(dXOffset, dCharHeight / 2.0, 0);
  glVertex3f(dXOffset + dLen, dCharHeight / 2.0, 0);
  glVertex3f(dXOffset + dLen, -dCharHeight / 2.0, 0);
  glEnd();
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glTranslated(0, 1, 0);

  glPopMatrix();
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
