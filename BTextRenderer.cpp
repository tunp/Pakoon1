//
// BTextRenderer: Misc text rendering routines
//
// (c) Copyright 2002, Mikko Oksalahti (see end of file for details)
//

//#include "stdafx.h"
#include "BTextRenderer.h"
#include "BTextures.h"
#include "BSimulation.h"

#include <algorithm>

using namespace std;


//*************************************************************************************************
BTextRenderer::BTextRenderer() {
  m_dCharWidth = 14.5;
  m_dCharHeight = 28.75;
}

//*************************************************************************************************
void BTextRenderer::StartRenderingText() {
  OpenGLHelpers::SwitchToTexture(0);
  //BTextures::Use(BTextures::Texture::PANEL);
  BTextures::Use(BTextures::PANEL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

//*************************************************************************************************
void BTextRenderer::StopRenderingText() {
  glDisable(GL_TEXTURE_2D);
}


//*************************************************************************************************
void BTextRenderer::DrawTextAt(double dX, 
                               double dY, 
                               //CString sText, 
                               string sText, 
                               TTextAlign textalign,
                               double dRed, 
                               double dGreen, 
                               double dBlue, 
                               double dAlpha) {

  OpenGLHelpers::SetColorFull(dRed, dGreen, dBlue, dAlpha);

  glPushMatrix();
  glTranslated(dX, dY - m_dCharHeight / 2.0, 0);
  if(textalign == ALIGN_RIGHT) {
    //glTranslated(-m_dCharWidth * sText.GetLength(), 0, 0);
    glTranslated(-m_dCharWidth * sText.length(), 0, 0);
  } else if(textalign == ALIGN_CENTER) {
    //glTranslated(-(m_dCharWidth * double(sText.GetLength())) / 2.0, 0, 0);
    glTranslated(-(m_dCharWidth * double(sText.length())) / 2.0, 0, 0);
  }

  // Draw text char by char
  //for(int i = 0; i < sText.GetLength(); ++i) {
  for(int i = 0; i < sText.length(); ++i) {
    char c = sText[i];
    c = toupper(c);
    c -= ' ';

    glBegin(GL_TRIANGLE_STRIP);
    int xOff = c % 5;
    int yOff = c / 5;
    OpenGLHelpers::SetTexCoord((1.0 + double(xOff) * m_dCharWidth)/512.0, 
                               (511.0 - (0.0 + double(yOff) * m_dCharHeight)) / 512.0);
    glVertex3f(0, m_dCharHeight - 4.0, 0);
    OpenGLHelpers::SetTexCoord((1.0 + double(xOff) * m_dCharWidth)/512.0, 
                               (511.0 - (0.0 + double(yOff) * m_dCharHeight + m_dCharHeight - 4.0)) / 512.0);
    glVertex3f(0, 0, 0);
    OpenGLHelpers::SetTexCoord((1.0 + double(xOff) * m_dCharWidth + m_dCharWidth)/512.0, 
                               (511.0 - (0.0 + double(yOff) * m_dCharHeight)) / 512.0);
    glVertex3f(m_dCharWidth, m_dCharHeight - 4.0, 0);
    OpenGLHelpers::SetTexCoord((1.0 + double(xOff) * m_dCharWidth + m_dCharWidth)/512.0, 
                               (511.0 - (0.0 + double(yOff) * m_dCharHeight + m_dCharHeight - 4.0)) / 512.0);
    glVertex3f(m_dCharWidth, 0, 0);
    glEnd();
    glTranslated(m_dCharWidth, 0, 0);
  }

  glPopMatrix();
}


//*************************************************************************************************
void BTextRenderer::DrawSmallTextAt(double dX, 
                                    double dY, 
                                    //CString sText, 
                                    string sText, 
                                    int     nChars,
                                    TTextAlign textalign,
                                    double dRed, 
                                    double dGreen, 
                                    double dBlue, 
                                    double dAlpha) {

  double xSize = 8.25;
  double ySize = 13.9;

  OpenGLHelpers::SetColorFull(dRed, dGreen, dBlue, dAlpha);

  glPushMatrix();
  glTranslated(dX, dY, 0);
  if(textalign == ALIGN_RIGHT) {
    //glTranslated(-xSize * sText.GetLength(), 0, 0);
    glTranslated(-xSize * sText.length(), 0, 0);
  } else if(textalign == ALIGN_CENTER) {
    //glTranslated(-(xSize * double(sText.GetLength())) / 2.0, 0, 0);
    glTranslated(-(xSize * double(sText.length())) / 2.0, 0, 0);
  }

  // Draw text char by char
  //for(int i = 0; i < min(nChars, sText.GetLength()); ++i) {
  for(int i = 0; i < min(nChars, (int) sText.length()); ++i) {
    char c = sText[i];
    c = toupper(c);
    c -= ' ';

    glBegin(GL_TRIANGLE_STRIP);
    int xOff = c % 10;
    int yOff = c / 10;
    OpenGLHelpers::SetTexCoord((401.0 + double(xOff) * xSize)/512.0, (511.0 - (284.0 + double(yOff) * ySize)) / 512.0);
    glVertex3f(0, ySize, 0);
    OpenGLHelpers::SetTexCoord((401.0 + double(xOff) * xSize)/512.0, (511.0 - (284.0 + double(yOff) * ySize + ySize - 0.8)) / 512.0);
    glVertex3f(0, 0, 0);
    OpenGLHelpers::SetTexCoord((401.0 + double(xOff) * xSize + xSize)/512.0, (511.0 - (284.0 + double(yOff) * ySize)) / 512.0);
    glVertex3f(xSize, ySize, 0);
    OpenGLHelpers::SetTexCoord((401.0 + double(xOff) * xSize + xSize)/512.0, (511.0 - (284.0 + double(yOff) * ySize + ySize - 0.8)) / 512.0);
    glVertex3f(xSize, 0, 0);
    glEnd();
    glTranslated(xSize, 0, 0);
  }

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
