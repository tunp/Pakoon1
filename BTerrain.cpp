//
// BTerrain.cpp
// 
// Classes for the terrain model of Pakoon! v1.0
//
// (c) Copyright 2002, Mikko Oksalahti (see end of file for details)
//


//#include "stdafx.h"
#include "BTerrain.h"
#include "HeightMap.h"
#include "OpenGLHelpers.h"
#include "OpenGLExtFunctions.h"
#include "BTextures.h"
//#include "DlgPreCache.h"
#include "BObject.h"
#include "BGame.h"
#include "BScene.h"


static void OpenGL_SetColor(double dRed, double dGreen, double dBlue, double dAlpha) {
  static GLfloat fCurColor[4];
  fCurColor[0] = (GLfloat) dRed;
  fCurColor[1] = (GLfloat) dGreen;
  fCurColor[2] = (GLfloat) dBlue;
  fCurColor[3] = (GLfloat) dAlpha;
  static GLfloat fNoColor[4];
  fNoColor[0] = (GLfloat) dRed * 0.3f;
  fNoColor[1] = (GLfloat) dGreen * 0.3f;
  fNoColor[2] = (GLfloat) dBlue * 0.3f;
  fNoColor[3] = (GLfloat) dAlpha;
  static GLfloat fSomeColor[4];
  fSomeColor[0] = (GLfloat) dRed / 1.5f;
  fSomeColor[1] = (GLfloat) dGreen / 1.5f;
  fSomeColor[2] = (GLfloat) dBlue / 1.5f;
  fSomeColor[3] = (GLfloat) dAlpha;
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, fCurColor);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, fCurColor);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, fCurColor);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 10.0f);
}


//*****************************************************************************
//* CLASS BTerrainBlock *******************************************************
//*****************************************************************************
BTerrainBlock::BTerrainBlock() {
  // Initialize with zero data
  m_vCenter.Set(0, 0, 0);
  m_vCorner.Set(0, 0, 0);
  m_dSize = double(cnBlockTiles) * cdBlockTileSize;
  m_nSize = 0;
  m_nMaxRes = -1;
  m_nDisplayListRes = -1;
  m_bDisplayListValid = false;
  m_bVisible = false;
  m_nDisplayLists = 0;
  m_nDisplayListBase = -1;
  m_pdHeights = 0;
  m_pTiles = 0;
  m_nHeights = 0;
  m_pNext = m_pPrev = 0;
  m_nTriangles = 0;
}


//*****************************************************************************
BTerrainBlock::~BTerrainBlock() {
  Destroy();
}


//*****************************************************************************
void BTerrainBlock::Destroy() {
  // clean up 
  DestroyDisplayLists();
  if(m_pdHeights) {
    delete [] m_pdHeights;
    m_pdHeights = 0;
  }
  if(m_pTiles) {
    delete [] m_pTiles;
    m_pTiles = 0;
  }
}


//*****************************************************************************
void BTerrainBlock::DestroyDisplayLists() {
  // clean up 
  if(m_bDisplayListValid) {
    m_bDisplayListValid = false;
    glDeleteLists(m_nDisplayListBase, m_nDisplayLists);
  }
}


//*****************************************************************************
BVector BTerrainBlock::GetCenter() { 
// Center of the block in world coordinates
  return m_vCenter;
}


//*****************************************************************************
void BTerrainBlock::SetCenter(BVector v) {
// Sets the center of the block in world coordinates
  m_vCenter = v;
  m_vCenter.m_dZ = 0;
  m_vCorner = v - BVector(m_dSize / 2.0, m_dSize / 2.0, 0.0);
}


//*****************************************************************************
int BTerrainBlock::GetDisplayRes() {
  return m_nDisplayListRes;
}


//*****************************************************************************
bool BTerrainBlock::PointIsInsideBlock(BVector vLoc, double dMargin) {
// True if the given point is inside the block
  return (vLoc.m_dX >= (m_vCorner.m_dX - dMargin)) && (vLoc.m_dX <= (m_vCorner.m_dX + m_dSize + dMargin)) &&
         (vLoc.m_dY >= (m_vCorner.m_dY - dMargin)) && (vLoc.m_dY <= (m_vCorner.m_dY + m_dSize + dMargin));
}


//*****************************************************************************
bool BTerrainBlock::IsVisible(BVector vOffset, BVector vFrom, BVector vLookingTowards, double dAngleOfView) {
// True if part of the block is visible from the given location
  //m_bVisible = true;
  //return true;

  double  dAlpha, dBeta, dTmp;
  BVector vCenterWithHeight = m_vCenter;
  vCenterWithHeight.m_dZ = -HeightMap::CalcHeightAt(vOffset.m_dX + m_vCenter.m_dX, 
                                                    vOffset.m_dY + m_vCenter.m_dY,
                                                    dTmp);

  BVector vToCenter    = vCenterWithHeight - vFrom;
  double  dLenToCenter = vToCenter.Length();

  if(dLenToCenter <= 1453.0) { // 453.0 ~ sqrt(320 * 320 + 320 * 320) = block's bounding sphere's radius
    m_bVisible = true;
    return true;
  }

  vToCenter.ToUnitLength();
  vLookingTowards.ToUnitLength();
  dAlpha = acos(vLookingTowards.ScalarProduct(vToCenter));
  dBeta = asin(453.0 / dLenToCenter);
  m_bVisible = (dAlpha - dBeta) < dAngleOfView;
  return m_bVisible;
}



//*****************************************************************************
bool BTerrainBlock::IsAtLeastOfRes(int nRes) {
// True if block has data for at least of given resolution
  return m_nMaxRes >= nRes;
}


//*****************************************************************************
void BTerrainBlock::CalculateWithRes(int nRes, BVector vOffset) {
// Calculates data for the given resolution (invalidates display list)

  int nX, nY;

  // Create new data
  double *pdNewHeights = 0;
  int nNewSize = (2 << nRes) + 1;

  if(m_pdHeights && (m_nMaxRes > 0)) {
    // There are old height values, utilize them
    if(m_nMaxRes >= nRes) {
      // We have the necessary data already
      return;
    }

    // Initialize new array with invalid data
    pdNewHeights = new double[nNewSize * nNewSize];
    m_nHeights = nNewSize * nNewSize;

    for(nY = 0; nY < nNewSize; ++nY) {
      for(nX = 0; nX < nNewSize; ++nX) {
        pdNewHeights[nY * nNewSize + nX] = 12345678.9;
      }
    }

    // Prefill the new height values with the ones we have
    int nSizeRatio = (1 << (nRes - m_nMaxRes));
    for(nY = 0; nY < m_nSize; ++nY) {
      for(nX = 0; nX < m_nSize; ++nX) {
        pdNewHeights[(nY * nSizeRatio) * nNewSize + (nX * nSizeRatio)] = 
          m_pdHeights[nY * m_nSize + nX];
      }
    }
  } else {
    // Initialize new array with invalid data
    pdNewHeights = new double[nNewSize * nNewSize];
    m_nHeights = nNewSize * nNewSize;
    for(nY = 0; nY < nNewSize; ++nY) {
      for(nX = 0; nX < nNewSize; ++nX) {
        pdNewHeights[nY * nNewSize + nX] = 12345678.9;
      }
    }
  }

  // Get rid of old data
  if(m_pdHeights) {
    delete [] m_pdHeights;
    m_pdHeights = 0;
  }

  // Calculate the missing terrain data
  double dTmp;
  for(nY = 0; nY < nNewSize; ++nY) {
    for(nX = 0; nX < nNewSize; ++nX) {
      if(fabs(pdNewHeights[nY * nNewSize + nX] - 12345678.9) < 0.1) {
        pdNewHeights[nY * nNewSize + nX] = 
          HeightMap::CalcHeightAt(vOffset.m_dX + m_vCorner.m_dX + double(nX) / double(nNewSize - 1) * m_dSize, 
                                  vOffset.m_dY + m_vCorner.m_dY + double(nY) / double(nNewSize - 1) * m_dSize, 
                                  dTmp);
      }
    }
  }

  // Record new size data
  m_pdHeights = pdNewHeights;
  m_nMaxRes = nRes;
  m_nSize = nNewSize;
}


//*****************************************************************************
bool BTerrainBlock::HasValidDisplayList(int nRes) {
// True if block has display list for the given resolution
  return m_bDisplayListValid;
}



//*****************************************************************************
double BTerrainBlock::HeightAt(int nX, int nY, bool bIgnore) {
// Return height at the given location
  //if(!bIgnore) {
//    if(fabs(m_pdHeights[nY * m_nSize + nX]) > 123456788.0 || 
//       m_pdHeights[nY * m_nSize + nX] == 0.0) {
//      CString sMsg;
//      sMsg.Format("Uninitialized, X = %d, Y = %d, Height = %lf", nX, nY, m_pdHeights[nY * m_nSize + nX]);
      //AfxMessageBox(sMsg);
//    }
  //}
  return m_pdHeights[nY * m_nSize + nX];
}




//*****************************************************************************
BVector BTerrainBlock::GetNormalAtTile(int nX, int nY, int nRes) {
// Returns the normal at the given resolution-specific point
// Normal is the average of the neighboring normals:
//
//   Y
//   
//   ^
//   | A  B  C
//   | D  xy E
//   | F  G  H
//   +---------> X
//

  BVector vToA, vToB, vToC, vToD, vToE, vToF, vToG, vToH;
  BVector vNormal(0, 0, 0);
  int     nTileSize = 1 << (m_nMaxRes - nRes);
  double  dTileSize = m_dSize / double(2 << nRes);
  double  dHeightAtXY = HeightAt(nX, nY);

  // First calculate vectors to neighboring points
  if(nY + nTileSize < m_nSize) {
    if(nX - nTileSize >= 0) {
      vToA.Set(-dTileSize, dTileSize, dHeightAtXY - HeightAt(nX - nTileSize, nY + nTileSize, true));
      vToA.ToUnitLength();
    }
    vToB.Set(0, dTileSize, dHeightAtXY - HeightAt(nX, nY + nTileSize, true));
    vToB.ToUnitLength();
    if(nX + nTileSize < m_nSize) {
      vToC.Set(dTileSize, dTileSize, dHeightAtXY - HeightAt(nX + nTileSize, nY + nTileSize, true));
      vToC.ToUnitLength();
    }
  }
  if(nY - nTileSize >= 0) {
    if(nX - nTileSize >= 0) {
      vToF.Set(-dTileSize, -dTileSize, dHeightAtXY - HeightAt(nX - nTileSize, nY - nTileSize, true));
      vToF.ToUnitLength();
    }
    vToG.Set(0, -dTileSize, dHeightAtXY - HeightAt(nX, nY - nTileSize, true));
    vToG.ToUnitLength();
    if(nX + nTileSize < m_nSize) {
      vToH.Set(dTileSize, -dTileSize, dHeightAtXY - HeightAt(nX + nTileSize, nY - nTileSize, true));
      vToH.ToUnitLength();
    }
  }
  if(nX - nTileSize >= 0) {
    vToD.Set(-dTileSize, 0, dHeightAtXY - HeightAt(nX - nTileSize, nY, true));
    vToD.ToUnitLength();
  }
  if(nX + nTileSize < m_nSize) {
    vToE.Set(dTileSize, 0, dHeightAtXY - HeightAt(nX + nTileSize, nY, true));
    vToE.ToUnitLength();
  }

  // Then create and sum normals together
  if(nY + nTileSize < m_nSize) {
    if(nX - nTileSize >= 0) {
      vNormal += vToA.CrossProduct(vToB);
    }
    if(nX + nTileSize < m_nSize) {
      vNormal += vToB.CrossProduct(vToC);
    }
  }
  if(nY - nTileSize >= 0) {
    if(nX - nTileSize >= 0) {
      vNormal += vToG.CrossProduct(vToF);
    }
    if(nX + nTileSize < m_nSize) {
      vNormal += vToH.CrossProduct(vToG);
    }
  }
  if(nX + nTileSize < m_nSize) {
    if(nY - nTileSize >= 0) {
      vNormal += vToE.CrossProduct(vToH);
    }
    if(nY + nTileSize < m_nSize) {
      vNormal += vToC.CrossProduct(vToE);
    }
  }
  if(nX - nTileSize >= 0) {
    if(nY - nTileSize >= 0) {
      vNormal += vToF.CrossProduct(vToD);
    }
    if(nY + nTileSize < m_nSize) {
      vNormal += vToD.CrossProduct(vToA);
    }
  }

  vNormal.ToUnitLength();
  return vNormal;
}





//*****************************************************************************
void BTerrainBlock::SetTextureCoordinate(BVector vPoint) {
  if(OpenGLHelpers::m_bMultiTexturing) {
    // Use first texture unit
    /*
    if(vPoint.m_dZ <= -126.0) {
      glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.5f, 62.0f/64.0f);
    } else if(vPoint.m_dZ > 0.0) {
      glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.5f, 0.0f);
    } else {
      glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.5f, float(-vPoint.m_dZ) / 128.0f);
    }
    */
    //glMultiTexCoord2fARB(GL_TEXTURE0_ARB, float(vPoint.m_dX) / 1000.0f, float(vPoint.m_dY) / 1000.0f);
#ifndef __EMSCRIPTEN__
    glMultiTexCoord2fARB(GL_TEXTURE0_ARB, float(vPoint.m_dX) * BTerrain::m_fGroundTextureScaler1, float(vPoint.m_dY) * BTerrain::m_fGroundTextureScaler1);
#endif
    // Use second texture unit
    //glMultiTexCoord2fARB(GL_TEXTURE1_ARB, float(vPoint.m_dX) / 100.0f, float(vPoint.m_dY) / 100.0f);
#ifndef __EMSCRIPTEN__
    glMultiTexCoord2fARB(GL_TEXTURE1_ARB, float(vPoint.m_dX) * BTerrain::m_fGroundTextureScaler2, float(vPoint.m_dY) * BTerrain::m_fGroundTextureScaler2);
#endif
  } else {
    // No multitexturing, use second texture unit coordinates
    glTexCoord2f(float(vPoint.m_dX) / 100.0f, float(vPoint.m_dY) / 100.0f);
  }
}



//*****************************************************************************
void BTerrainBlock::CreateDisplayListsOfRes(int nRes, bool bOverride, bool bWireframe, bool bNormals) {
// Creates display lists for the given resolution

  m_nDisplayLists = 1; // For test use only one pass
  if(m_nDisplayListBase == -1) { // create new list only if not already created. Otherwise use the existing list.
    m_nDisplayListBase = glGenLists(1);
  }

  // Render the triangles in the block into the display list
  BVector vPoint, vNormal;
  if(bOverride) {
    glNewList(m_nDisplayListBase, GL_COMPILE_AND_EXECUTE);
  } else {
    glNewList(m_nDisplayListBase, GL_COMPILE);
  }
  int nRenderSize = (2 << nRes) + 1;
  int nSizeRatio  = (1 << (m_nMaxRes - nRes));
  double dScaler  = 1.0 / double(nRenderSize - 1) * m_dSize;
  int nX, nY, nXStart = 0, nYStart = 0, nXEnd = nRenderSize, nYEnd = nRenderSize - 1;

  m_dTileSize = m_dSize / (2 << nRes);
  m_nStep     = nSizeRatio;

  m_nTriangles = -2;

  if(bWireframe) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    OpenGL_SetColor(0, 0, 0, 1);
    for(nY = nYStart; nY < nYEnd; ++nY) {
      glBegin(GL_TRIANGLE_STRIP);
      for(nX = nXStart; nX < nXEnd; ++nX) {
        vPoint.Set(m_vCorner.m_dX + double(nX)     * dScaler,
                   m_vCorner.m_dY + double(nY + 1) * dScaler,
                   -HeightAt(nX * nSizeRatio, (nY + 1) * nSizeRatio));

        // Set vertex
        glVertex3f(vPoint.m_dX, vPoint.m_dY, vPoint.m_dZ);

        vPoint.Set(m_vCorner.m_dX + double(nX) * dScaler,
                   m_vCorner.m_dY + double(nY) * dScaler,
                   -HeightAt(nX * nSizeRatio, nY * nSizeRatio));

        // Set vertex
        glVertex3f(vPoint.m_dX, vPoint.m_dY, vPoint.m_dZ);

        m_nTriangles += 2;

      }
      glEnd();
    }
    OpenGL_SetColor(1, 1, 1, 1);
  }

  double dR = -1.0, dG = -1.0, dB = -1.0;
  BVector dNormalOld(0, 0, 0);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glPolygonOffset(-1, -1);
  for(nY = nYStart; nY < nYEnd; ++nY) {
    glBegin(GL_TRIANGLE_STRIP);
    for(nX = nXStart; nX < nXEnd; ++nX) {
      vPoint.Set(m_vCorner.m_dX + double(nX)     * dScaler,
                 m_vCorner.m_dY + double(nY + 1) * dScaler,
                 -HeightAt(nX * nSizeRatio, (nY + 1) * nSizeRatio));
      vNormal = GetNormalAtTile(nX * nSizeRatio, (nY + 1) * nSizeRatio, nRes);

      // Set normal, texture coordinate and vertex
      double dNewR, dNewG, dNewB;
      BTerrain::GetColorForHeight(-vPoint.m_dZ, dNewR, dNewG, dNewB);
      if((dNewR != dR) || (dNewG != dG) || (dNewB != dB)) {
        // glColor3f(dNewR, dNewG, dNewB);
        glColor3ub(GLubyte(dNewR * 255.0), GLubyte(dNewG * 255.0), GLubyte(dNewB * 255.0));

        dR = dNewR;
        dG = dNewG;
        dB = dNewB;
      }
      if((dNormalOld - vNormal).Length() > 0.001) {
        glNormal3f(vNormal.m_dX, vNormal.m_dY, vNormal.m_dZ);
        dNormalOld = vNormal;
      }
      SetTextureCoordinate(vPoint);
      glVertex3f(vPoint.m_dX, vPoint.m_dY, vPoint.m_dZ);

      vPoint.Set(m_vCorner.m_dX + double(nX) * dScaler,
                 m_vCorner.m_dY + double(nY) * dScaler,
                 -HeightAt(nX * nSizeRatio, nY * nSizeRatio));
      vNormal = GetNormalAtTile(nX * nSizeRatio, nY * nSizeRatio, nRes);

      // Set normal, texture coordinate and vertex
      BTerrain::GetColorForHeight(-vPoint.m_dZ, dNewR, dNewG, dNewB);
      if((dNewR != dR) || (dNewG != dG) || (dNewB != dB)) {
        // glColor3f(dNewR, dNewG, dNewB);
        glColor3ub(GLubyte(dNewR * 255.0), GLubyte(dNewG * 255.0), GLubyte(dNewB * 255.0));
        dR = dNewR;
        dG = dNewG;
        dB = dNewB;
      }
      if((dNormalOld - vNormal).Length() > 0.001) {
        glNormal3f(vNormal.m_dX, vNormal.m_dY, vNormal.m_dZ);
        dNormalOld = vNormal;
      }
      SetTextureCoordinate(vPoint);
      glVertex3f(vPoint.m_dX, vPoint.m_dY, vPoint.m_dZ);

      m_nTriangles += 2;
    }
    glEnd();
  }

  if(bNormals) {
    OpenGL_SetColor(0.5, 0, 0, 1);
    for(nY = nYStart; nY < nYEnd + 1; ++nY) {
      glBegin(GL_LINES);
      for(nX = nXStart; nX < nXEnd; ++nX) {
        vPoint.Set(m_vCorner.m_dX + double(nX) * dScaler,
                   m_vCorner.m_dY + double(nY) * dScaler,
                   -HeightAt(nX * nSizeRatio, nY * nSizeRatio));
        vNormal = GetNormalAtTile(nX * nSizeRatio, nY * nSizeRatio, nRes) * 10.0;
        glVertex3f(vPoint.m_dX, vPoint.m_dY, vPoint.m_dZ);
        vPoint += vNormal;
        glVertex3f(vPoint.m_dX, vPoint.m_dY, vPoint.m_dZ);
      }
      glEnd();
    }
    OpenGL_SetColor(1, 1, 1, 1);
  }

  glEndList();
  m_nDisplayListRes = nRes;
  m_bDisplayListValid = true;
  if(m_nTriangles == -2) {
    m_nTriangles = 0;
  }

  // Create tile data (such as normals and friction information etc.)
  CreateTileData(nRes);
}


extern double g_dSceneFriction;


//*****************************************************************************
void BTerrainBlock::CreateTileData(int nRes) {
  // calculate normals for collision detection
  int    nRenderSize = (2 << nRes);

  // Reallocate memory for tile data
  if(m_pTiles) {
    delete [] m_pTiles;
  }
  m_pTiles = new BTerrainTile[nRenderSize * nRenderSize];

  // create normals (and friction)
  for(int nY = 0; nY < nRenderSize; ++nY) {
    for(int nX = 0; nX < nRenderSize; ++nX) {
      BVector v1, v2, v3, v4;
      v1.Set(0,           0,           -HeightAt(nX       * m_nStep, nY * m_nStep));
      v2.Set(m_dTileSize, 0,           -HeightAt((nX + 1) * m_nStep, nY * m_nStep));
      v3.Set(0,           m_dTileSize, -HeightAt(nX       * m_nStep, (nY + 1) * m_nStep));
      v4.Set(m_dTileSize, m_dTileSize, -HeightAt((nX + 1) * m_nStep, (nY + 1) * m_nStep));
      m_pTiles[nY * nRenderSize + nX].m_vNormal1 = (v1 - v2).CrossProduct(v4 - v2);
      m_pTiles[nY * nRenderSize + nX].m_vNormal1.ToUnitLength();
      m_pTiles[nY * nRenderSize + nX].m_vNormal2 = (v4 - v3).CrossProduct(v1 - v3);
      m_pTiles[nY * nRenderSize + nX].m_vNormal2.ToUnitLength();
      m_pTiles[nY * nRenderSize + nX].m_dFriction = g_dSceneFriction;
    }
  }
}




//*****************************************************************************
int BTerrainBlock::Render(int nPass){
// Renders the given pass display list (different textures are rendered in different passes)
  int nRet = 0;
  if(m_bVisible && m_bDisplayListValid) {
    if((nPass >= 0) && (nPass < m_nDisplayLists)) {
      glCallList(m_nDisplayListBase + nPass);
      nRet = m_nTriangles;
    }
  }

  return nRet;
}


//*****************************************************************************
void BTerrainBlock::RenderGasStation(bool bShadows) {
  // If block has a gas station, draw that as well
  glPushMatrix();
  glTranslated(m_vGasoCenter.m_dX, m_vGasoCenter.m_dY, m_vGasoCenter.m_dZ);
  BTerrain::GetGasStationObject()->DrawObject(bShadows);
  glPopMatrix();
}

//*****************************************************************************
void BTerrainBlock::LoadTerrainBlockCache(FILE *fp) {
// Loads the cached terrain data from file to speed up rendering.
  fread(&m_dSize, sizeof(double), 1, fp);
  fread(&m_nSize, sizeof(int), 1, fp);
  fread(&m_nMaxRes, sizeof(int), 1, fp);
  fread(&m_nHeights, sizeof(int), 1, fp);
  m_pdHeights = new double[m_nHeights];
  for(int i = 0; i < m_nHeights; ++i) {
    fread(&m_pdHeights[i], sizeof(double), 1, fp);
  }
}


//*****************************************************************************
void BTerrainBlock::SaveTerrainBlockCache(FILE *fp) {
// Saves the current terrain data to a cache file to speed up rendering.
  fwrite(&m_vCenter.m_dX, sizeof(double), 1, fp);
  fwrite(&m_vCenter.m_dY, sizeof(double), 1, fp);
  fwrite(&m_vCenter.m_dZ, sizeof(double), 1, fp);

  fwrite(&m_dSize, sizeof(double), 1, fp);
  fwrite(&m_nSize, sizeof(int), 1, fp);
  fwrite(&m_nMaxRes, sizeof(int), 1, fp);
  fwrite(&m_nHeights, sizeof(int), 1, fp);
  for(int i = 0; i < m_nHeights; ++i) {
    fwrite(&m_pdHeights[i], sizeof(double), 1, fp);
  }
}













//*****************************************************************************
//* CLASS BBlockRing **********************************************************
//*****************************************************************************
BBlockRing::BBlockRing() {
  m_pFirstBlock = 0;
  m_nBlocks = 0;
}


//*****************************************************************************
BBlockRing::~BBlockRing() {
  RemoveAll();
}


//*****************************************************************************
void BBlockRing::RemoveAll() {
  BTerrainBlock *pBlock;
  while(pBlock = GetHead()) {
    Remove(pBlock);
    delete pBlock;
  }
}



//*****************************************************************************
BTerrainBlock *BBlockRing::GetHead() {
  return m_pFirstBlock;
}


//*****************************************************************************
BTerrainBlock *BBlockRing::GetTail() {
  if(m_pFirstBlock) {
    return m_pFirstBlock->m_pPrev;
  } else {
    return 0;
  }
}


//*****************************************************************************
BTerrainBlock *BBlockRing::SetHead(BTerrainBlock *pNewHead) {
  m_pFirstBlock = pNewHead;
  return m_pFirstBlock;
}


//*****************************************************************************
BTerrainBlock *BBlockRing::Remove(BTerrainBlock *pBlock) {
// Removes the block from the ring and deletes it. Returns next block
  BTerrainBlock *pRet = pBlock->m_pNext;
  if(m_pFirstBlock) {
    if(m_pFirstBlock->m_pNext == m_pFirstBlock) {
      // Remove last one
      m_pFirstBlock = 0;
      m_nBlocks = 0;
      return 0;
    }

    // Delete from ring
    if(pBlock == m_pFirstBlock) {
      // If first is deleted, update first pointer to next in ring
      m_pFirstBlock = pBlock->m_pNext;
    }

    // Update ring links
    pBlock->m_pNext->m_pPrev = pBlock->m_pPrev;
    pBlock->m_pPrev->m_pNext = pBlock->m_pNext;

    --m_nBlocks;
  }

  return pRet;
}


//*****************************************************************************
BTerrainBlock *BBlockRing::AddTail(BTerrainBlock *pNewBlock) {
  // Add new block to the ring
  if(!m_pFirstBlock) {
    // Add as first
    pNewBlock->m_pNext = pNewBlock;
    pNewBlock->m_pPrev = pNewBlock;
    m_pFirstBlock = pNewBlock;
  } else {
    // Add to the end of the ring
    pNewBlock->m_pNext = m_pFirstBlock;
    pNewBlock->m_pPrev = m_pFirstBlock->m_pPrev;

    m_pFirstBlock->m_pPrev->m_pNext = pNewBlock;
    m_pFirstBlock->m_pPrev = pNewBlock;
  }
  ++m_nBlocks;
  return pNewBlock;
}


//*****************************************************************************
BTerrainBlock *BBlockRing::AddFront(BTerrainBlock *pNewBlock) {
  // Add new block to the ring
  if(!m_pFirstBlock) {
    // Add as first
    pNewBlock->m_pNext = pNewBlock;
    pNewBlock->m_pPrev = pNewBlock;
    m_pFirstBlock = pNewBlock;
  } else {
    // Add to the beginning of the ring
    pNewBlock->m_pNext = m_pFirstBlock->m_pNext;
    pNewBlock->m_pPrev = m_pFirstBlock;

    m_pFirstBlock->m_pNext->m_pPrev = pNewBlock;
    m_pFirstBlock->m_pNext = pNewBlock;
  }
  ++m_nBlocks;
  return pNewBlock;
}













//*****************************************************************************
//* CLASS BTerrain ************************************************************
//*****************************************************************************

bool BTerrain::m_bOverride = false;
int  BTerrain::m_nRes1 = 3;
int  BTerrain::m_nRes2 = 3;
int  BTerrain::m_nRes3 = 3;
int  BTerrain::m_nRes4 = 3;
int  BTerrain::m_nRes5 = 3;
int  BTerrain::m_nRes6 = 3;
BObject BTerrain::m_soGasStation;
GLfloat BTerrain::m_fGroundTextureScaler1;
GLfloat BTerrain::m_fGroundTextureScaler2;


//*****************************************************************************
BTerrain::BTerrain() {
// Initialize with zero data

  m_pPrevCenter = 0;
  m_dBlockSize  = double(cnBlockTiles) * cdBlockTileSize;
  m_vOffset.Set(1862400.0 - 1040.0, 3280800.0 + 900.0, 0.0);
  // m_vOffset.Set(2091048064.0, 2090996864.0, 0.0);
  // m_vOffset.Set(44853120.0, 45429888.0, 0.0);
  // m_vOffset.Set(888960.0, 128.0, 0.0);
  // m_vOffset.Set(3680798.0, 3166398.0, 0.0);
  // m_vOffset.Set(-389888.0, -2816.0, 0.0);
  // m_vOffset.Set(381056.0, 281728.0, 0.0);
  m_vCameraLoc.Set(0, 0, 0);
  m_vCameraLookDir.Set(0, 1, 0);
  //m_sSceneName = _T("");
  m_sSceneName = "";

#ifdef __EMSCRIPTEN__
  SetRenderResolution(RENDER_SLOW_MACHINE);
#else
  SetRenderResolution(RENDER_LOW);
#endif
}



//*****************************************************************************
void BTerrain::SetRenderResolution(RendRes renderResolution) {
  switch(renderResolution) {
    case RENDER_SLOW_MACHINE:
      m_nRes1 = 2;
      m_nRes2 = 2;
      m_nRes3 = 2;
      m_nRes4 = 2;
      m_nRes5 = 2;
      m_nRes6 = 2;
      BGame::m_nTerrainResolution = 5;
      break;
    case RENDER_MINIMUM:
      m_nRes1 = 3;
      m_nRes2 = 3;
      m_nRes3 = 3;
      m_nRes4 = 3;
      m_nRes5 = 3;
      m_nRes6 = 3;
      BGame::m_nTerrainResolution = 0;
      break;
    case RENDER_LOW:
      m_nRes1 = 4;
      m_nRes2 = 4;
      m_nRes3 = 4;
      m_nRes4 = 3;
      m_nRes5 = 3;
      m_nRes6 = 3;
      BGame::m_nTerrainResolution = 1;
      break;
    case RENDER_MEDIUM:
      m_nRes1 = 5;
      m_nRes2 = 5;
      m_nRes3 = 4;
      m_nRes4 = 4;
      m_nRes5 = 3;
      m_nRes6 = 3;
      BGame::m_nTerrainResolution = 2;
      break;
    case RENDER_HIGH:
      m_nRes1 = 6;
      m_nRes2 = 5;
      m_nRes3 = 5;
      m_nRes4 = 4;
      m_nRes5 = 4;
      m_nRes6 = 4;
      BGame::m_nTerrainResolution = 3;
      break;
    case RENDER_MAXIMUM:
      m_nRes1 = 6;
      m_nRes2 = 6;
      m_nRes3 = 5;
      m_nRes4 = 5;
      m_nRes5 = 4;
      m_nRes6 = 4;
      BGame::m_nTerrainResolution = 4;
      break;
  }
}





//*****************************************************************************
void BTerrain::PreProcessVisualization() {

  // Create GasStation object
  // m_soGasStation.m_dScale = 1.0;
  m_soGasStation.m_vLocation = BVector(0, 0, 0);
  m_soGasStation.m_dRed = 1.0;
  m_soGasStation.m_dGreen = 1.0;
  m_soGasStation.m_dBlue = 1.0;
  m_soGasStation.m_dFriction = 0.5;

  m_soGasStation.m_nParts = 0;
  m_soGasStation.m_nCollDetParts = 1;
  m_soGasStation.m_pCollDetPart = new BObjectPart[1];

  m_soGasStation.m_pCollDetPart[0].m_nFaces = 5;
  m_soGasStation.m_pCollDetPart[0].m_pFace = new BFace[5];
  m_soGasStation.m_pCollDetPart[0].m_pFace[0].m_nPoints = 4;
  m_soGasStation.m_pCollDetPart[0].m_pFace[0].m_vPoint[0] = BVector(-5, -5, 5);
  m_soGasStation.m_pCollDetPart[0].m_pFace[0].m_vPoint[1] = BVector(-5, -5, -5);
  m_soGasStation.m_pCollDetPart[0].m_pFace[0].m_vPoint[2] = BVector(5, -5, -5);
  m_soGasStation.m_pCollDetPart[0].m_pFace[0].m_vPoint[3] = BVector(5, -5, 5);

  m_soGasStation.m_pCollDetPart[0].m_pFace[1].m_nPoints = 4;
  m_soGasStation.m_pCollDetPart[0].m_pFace[1].m_vPoint[0] = BVector(5, -5, 5);
  m_soGasStation.m_pCollDetPart[0].m_pFace[1].m_vPoint[1] = BVector(5, -5, -5);
  m_soGasStation.m_pCollDetPart[0].m_pFace[1].m_vPoint[2] = BVector(5, 5, -5);
  m_soGasStation.m_pCollDetPart[0].m_pFace[1].m_vPoint[3] = BVector(5, 5, 5);

  m_soGasStation.m_pCollDetPart[0].m_pFace[2].m_nPoints = 4;
  m_soGasStation.m_pCollDetPart[0].m_pFace[2].m_vPoint[0] = BVector(5, 5, 5);
  m_soGasStation.m_pCollDetPart[0].m_pFace[2].m_vPoint[1] = BVector(5, 5, -5);
  m_soGasStation.m_pCollDetPart[0].m_pFace[2].m_vPoint[2] = BVector(-5, 5, -5);
  m_soGasStation.m_pCollDetPart[0].m_pFace[2].m_vPoint[3] = BVector(-5, 5, 5);

  m_soGasStation.m_pCollDetPart[0].m_pFace[3].m_nPoints = 4;
  m_soGasStation.m_pCollDetPart[0].m_pFace[3].m_vPoint[0] = BVector(-5, 5, 5);
  m_soGasStation.m_pCollDetPart[0].m_pFace[3].m_vPoint[1] = BVector(-5, 5, -5);
  m_soGasStation.m_pCollDetPart[0].m_pFace[3].m_vPoint[2] = BVector(-5, -5, -5);
  m_soGasStation.m_pCollDetPart[0].m_pFace[3].m_vPoint[3] = BVector(-5, -5, 5);

  m_soGasStation.m_pCollDetPart[0].m_pFace[4].m_nPoints = 4;
  m_soGasStation.m_pCollDetPart[0].m_pFace[4].m_vPoint[0] = BVector(-5, -5, -5);
  m_soGasStation.m_pCollDetPart[0].m_pFace[4].m_vPoint[1] = BVector(-5, 5, -5);
  m_soGasStation.m_pCollDetPart[0].m_pFace[4].m_vPoint[2] = BVector(5, 5, -5);
  m_soGasStation.m_pCollDetPart[0].m_pFace[4].m_vPoint[3] = BVector(5, -5, -5);

  m_soGasStation.Setup();
  m_soGasStation.m_bShadowPolygonOK = true;
}



//*****************************************************************************
BTerrain::~BTerrain() {
  // Clean up
  SaveTerrainCache(m_sSceneName);
}


//*****************************************************************************
void BTerrain::GetColorForHeight(double dHeight, double &rdR, double &rdG, double &rdB) {
// Returns the terrain color for the given height
  // Map color as follows:
  //   Near water sand color            (< 12 meters)
  //   Next (up) green                  (< 60 meters)
  //   Next (up) tundra (brown/reddish) (< 350 meters)
  //   Next (up) ice (white)            (> 350 meters)

  double dRatio = 0.0;
  // dHeight = -dHeight;
  if(dHeight < 12.0) {
    rdR = 1.0;   // SAND
    rdG = 0.85; 
    rdB = 0.4;
  } else if(dHeight < 18.0) {
    dRatio = (dHeight - 12.0) / (6.0);
    rdR = (1    * (1.0 - dRatio) + 0.1 * dRatio); // SAND TO GREEN
    rdG = (0.85 * (1.0 - dRatio) + 0.6 * dRatio); 
    rdB = (0.4  * (1.0 - dRatio) + 0.4 * dRatio);
  } else if(dHeight < 25.0) {
    rdR = 0.3;  // GREEN
    rdG = 0.6; 
    rdB = 0.4;
  } else if(dHeight < 60.0) {
    dRatio = (dHeight - 25.0) / (35.0); 
    rdR = (0.3 * (1.0 - dRatio) + 0.8 * dRatio);  // GREEN TO TUNDRA
    rdG = (0.6 * (1.0 - dRatio) + 0.6 * dRatio); 
    rdB = (0.4 * (1.0 - dRatio) + 0.5 * dRatio);
  } else if(dHeight < 350.0) {
    rdR = 0.8; // TUNDRA
    rdG = 0.6; 
    rdB = 0.5;
  } else {
    dRatio = (dHeight - 350.0) / (100.0);
    if(dRatio > 1.0) {
      rdR = 1.0;
      rdG = 1.0;
      rdB = 1.0;
    } else {
      rdR = (0.8 * (1.0 - dRatio) + dRatio); 
      rdG = (0.6 * (1.0 - dRatio) + dRatio);  
      rdB = (0.5 * (1.0 - dRatio) + dRatio);
    }
  }
}


//*****************************************************************************
void BTerrain::MakeTerrainValid(BVector vLocCar, 
                                BVector vLocCamera, 
                                BVector vLookingTowards, 
                                bool bCreateDLs, 
                                bool bWireframe, 
                                bool bNormals) {
// Calculates terrain blocks and validates their display lists (if necessary)

  vLocCar.m_dZ = 0;
  m_vCameraLoc = vLocCamera;

  // Update block set
  BTerrainBlock *pBlock;
  BTerrainBlock *pBlockCenter = FindCenterBlock(vLocCar);
  BScene *pScene = BGame::GetSimulation()->GetScene();
  
  if(pBlockCenter && (pBlockCenter != m_pPrevCenter)) { // Entered new area
    // Delete obsolete blocks
    if(m_ringVisible.GetNBlocks() > 1) {
      pBlock = m_ringVisible.GetHead()->m_pNext; // Skip the first block. We don't want to delete it.
      do {
        if((pBlock->GetCenter() - vLocCar).Length() > (cdWorldHemisphereRadius + 453.0)) {
          pBlock = DeleteVisibleBlock(pBlock); // Delete this and proceed to next
        } else {
          pBlock = pBlock->m_pNext; // proceed to next
        }
      } while(pBlock != m_ringVisible.GetHead());
    }

    // Add newly born blocks
    for(int nY = -5; nY <= 5; ++nY) {
      for(int nX = -5; nX <= 5; ++nX) {
        if((nX != 0) || (nY != 0)) { // Exclude center block, since that is guaranteed to be available
          if(!ExistsBlockForLocation(nX, nY, &m_ringVisible) && 
             ((GetCenterForRelLoc(pBlockCenter->GetCenter(), nX, nY) - vLocCar).Length() <= (cdWorldHemisphereRadius + 453.0))) {
            // Try to find block from cache
            if(pBlock = ExistsBlockForLocation(nX, nY, &m_ringCache)) {
              m_ringCache.Remove(pBlock);
              m_ringVisible.AddTail(pBlock);
            } else {
              pBlock = CreateBlockAt(pBlockCenter->GetCenter(), nX, nY);
            }
            if(pScene) {
              pScene->PlaceTerrainObjects(pBlock);
            }
          }
        }
      }
    }
  }

  // Make sure all display lists are ok
  pBlock = m_ringVisible.GetHead();
  if(pBlock) {
    do {
      // if(pBlock->IsVisible(m_vOffset, vLocCamera, vLookingTowards, 55.0 / 180.0 * 3.141592654)) { // 888 CHECK! Angle Of View!!!
      int nRes = GetResBasedOnDistance((pBlock->GetCenter() - vLocCar).Length());
      if(!pBlock->IsAtLeastOfRes(nRes)) {
        pBlock->CalculateWithRes(nRes, m_vOffset);
      }
      if(!pBlock->HasValidDisplayList(nRes) || 
         (pBlock->GetDisplayRes() != nRes) || 
         bCreateDLs) {
        pBlock->CreateDisplayListsOfRes(nRes, m_bOverride, bWireframe, bNormals);
      }
      // }
      pBlock = pBlock->m_pNext;
    } while(pBlock != m_ringVisible.GetHead());
  }
}


//*****************************************************************************
int BTerrain::Render(int nSkyDetail, BVector vCamLoc, BVector vLookingTo) {
// Draws the terrain 

  if(OpenGLHelpers::m_bMultiTexturing) {
    OpenGLHelpers::SwitchToTexture(0);
    //BTextures::Use(BTextures::Texture::GROUND_COLOR_MAP);
    BTextures::Use(BTextures::GROUND_COLOR_MAP);

    OpenGLHelpers::SwitchToTexture(1);
    //BTextures::Use(BTextures::Texture::GROUND_BASE);
    BTextures::Use(BTextures::GROUND_BASE);
  } else {
    OpenGLHelpers::SwitchToTexture(0);
    //BTextures::Use(BTextures::Texture::GROUND_BASE);
    BTextures::Use(BTextures::GROUND_BASE);
  }

  int nTriangles = 0;
  int nPass = 0;

  if(!m_ringVisible.GetHead()) {
    return nTriangles;
  }

  glShadeModel(GL_SMOOTH);
  glEnable(GL_CULL_FACE);
  glEnable(GL_COLOR_MATERIAL);
  glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
  glCullFace(GL_BACK);
  OpenGLHelpers::SetColorFull(1, 1, 1, 1);
  // Render all blocks within world hemisphere
  int nAll = 0;
  int nVis = 0;
  for(nPass = 0; nPass < cnRequiredRenderPasses; ++nPass) {
    BTerrainBlock *pBlock = m_ringVisible.GetHead();
    do {
      ++nAll;
      if(pBlock->IsVisible(m_vOffset, vCamLoc, vLookingTo, 55.0 / 180.0 * 3.141592654)) {
        ++nVis;
        nTriangles += pBlock->Render(nPass);
      }
      pBlock = pBlock->m_pNext;
    } while(pBlock != m_ringVisible.GetHead());
  }

  glDisable(GL_TEXTURE_2D);
  glDisable(GL_COLOR_MATERIAL);

  OpenGLHelpers::SwitchToTexture(1, true);

  // Draw Gas Stations  

  GLfloat fLight1AmbientG[ 4];
  fLight1AmbientG[0] = 0.5f;
  fLight1AmbientG[1] = 0.5f;
  fLight1AmbientG[2] = 0.5f;
  fLight1AmbientG[3] = 0.0f;
  glLightfv( GL_LIGHT0, GL_AMBIENT,  fLight1AmbientG);

  OpenGLHelpers::SwitchToTexture(0);
  BTextures::Use(BTextures::SHADOW);
  OpenGLHelpers::SetColorFull(1, 1, 1, 1);
  glEnable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Render all gas stations within world hemisphere
  //if(BGame::m_nVisualize & BGame::TAnalyzerVis::GASSTATIONS) {
  if(BGame::m_nVisualize & BGame::GASSTATIONS) {
    glCullFace(GL_BACK);
    for(nPass = 0; nPass < cnRequiredRenderPasses; ++nPass) {
      BTerrainBlock *pBlock = m_ringVisible.GetHead();
      do {
        if(pBlock->m_bHasGaso && pBlock->m_bVisible) {
          pBlock->RenderGasStation(true);
        }
        pBlock = pBlock->m_pNext;
      } while(pBlock != m_ringVisible.GetHead());
    }
    glCullFace(GL_FRONT);

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    OpenGLHelpers::SetColorFull(1, 1, 1, 1);
    BTextures::Use(BTextures::GASSTATION);

    glShadeModel(GL_FLAT);

    // Render all blocks within world hemisphere
    for(nPass = 0; nPass < cnRequiredRenderPasses; ++nPass) {
      BTerrainBlock *pBlock = m_ringVisible.GetHead();
      do {
        if(pBlock->m_bHasGaso && pBlock->m_bVisible) {
          pBlock->RenderGasStation(false);
        }
        pBlock = pBlock->m_pNext;
      } while(pBlock != m_ringVisible.GetHead());
    }
    glDisable(GL_CULL_FACE);
    glShadeModel(GL_SMOOTH);
  }

  fLight1AmbientG[0] = 0.3f;
  fLight1AmbientG[1] = 0.3f;
  fLight1AmbientG[2] = 0.3f;
  fLight1AmbientG[3] = 0.0f;
  glLightfv( GL_LIGHT0, GL_AMBIENT,  fLight1AmbientG);

  return nTriangles;
} 


//*****************************************************************************
BTerrainBlock *BTerrain::FindCenterBlock(BVector vLoc) {
// Returns the (center) block based on the given location (if none exists, one is created)
  if(m_ringVisible.GetHead()) {
    BTerrainBlock *pBlock = m_ringVisible.GetHead();
    do {
      if(pBlock->PointIsInsideBlock(vLoc)) {
        m_ringVisible.SetHead(pBlock);
        return pBlock;
      }
      pBlock = pBlock->m_pNext;
    } while(pBlock != m_ringVisible.GetHead());
  }

  // No center found, create it
  return m_ringVisible.SetHead(CreateBlockAt(vLoc, 0, 0));
}


//*****************************************************************************
BTerrainBlock *BTerrain::ExistsBlockForLocation(int nRelXFromFirst, int nRelYFromFirst, BBlockRing *pRing) {
// True if there's a block at the given relative location
  if(!pRing->GetHead()) {
    return 0;
  }

  BVector vCenter = m_ringVisible.GetHead()->GetCenter();
  BVector vLoc = GetCenterForRelLoc(vCenter, nRelXFromFirst, nRelYFromFirst);

  BTerrainBlock *pBlock = pRing->GetHead();
  do {
    if(pBlock->PointIsInsideBlock(vLoc)) {
      return pBlock;
    }
    pBlock = pBlock->m_pNext;
  } while(pBlock != pRing->GetHead());
  return 0;
}


//*****************************************************************************
BVector BTerrain::GetCenterForRelLoc(BVector vLoc, int nRelXFromLoc, int nRelYFromLoc) {
  return vLoc + BVector(double(nRelXFromLoc) * m_dBlockSize,
                        double(nRelYFromLoc) * m_dBlockSize,
                        0.0);
}


//*****************************************************************************
BTerrainBlock *BTerrain::CreateBlockAt(BVector vLoc, int nRelXFromLoc, int nRelYFromLoc) {
// Creates a non-valid block at the given relative location
  return CreateBlockAtAbs(GetCenterForRelLoc(vLoc, nRelXFromLoc, nRelYFromLoc), &m_ringVisible);
}


//*****************************************************************************
BTerrainBlock *BTerrain::CreateBlockAtAbs(BVector vLoc, BBlockRing *pRing) {
// Creates a non-valid block at the given relative location

  // Create new block
  BTerrainBlock *pNewBlock = new BTerrainBlock;

  pNewBlock->SetCenter(vLoc); // Corner is set here as well
  pRing->AddTail(pNewBlock);

  // Check if block has a GasStation
  double dTmp;
  pNewBlock->m_nObjects = 0;
  pNewBlock->m_bHasGaso = !((int(vLoc.m_dX / pNewBlock->m_dSize) + int(vLoc.m_dY / pNewBlock->m_dSize)) % 2);
  if(pNewBlock->m_bHasGaso) {
    // Check that the surface is even enough
    double d1, d2, d3, d4;
    d1 = -HeightMap::CalcHeightAt(vLoc.m_dX + m_vOffset.m_dX - 5.0, 
                                  vLoc.m_dY + m_vOffset.m_dY - 5.0, 
                                  dTmp);
    d2 = -HeightMap::CalcHeightAt(vLoc.m_dX + m_vOffset.m_dX + 5.0, 
                                  vLoc.m_dY + m_vOffset.m_dY - 5.0, 
                                  dTmp);
    d3 = -HeightMap::CalcHeightAt(vLoc.m_dX + m_vOffset.m_dX + 5.0, 
                                  vLoc.m_dY + m_vOffset.m_dY + 5.0, 
                                  dTmp);
    d4 = -HeightMap::CalcHeightAt(vLoc.m_dX + m_vOffset.m_dX - 5.0, 
                                  vLoc.m_dY + m_vOffset.m_dY + 5.0, 
                                  dTmp);
    double diff = fabs(d2 - d1) + fabs(d3 - d2) + fabs(d4 - d3) + fabs(d1 - d4);
    if((d1 > 0.0) || (diff > 2.0)) {
      pNewBlock->m_bHasGaso = false;
      pNewBlock->m_nObjects = 0;
    } else {
      pNewBlock->m_vGasoCenter = vLoc;
      pNewBlock->m_vGasoCenter.m_dZ = -HeightMap::CalcHeightAt(vLoc.m_dX + m_vOffset.m_dX, 
                                                               vLoc.m_dY + m_vOffset.m_dY, 
                                                               dTmp) + 0.1;

      // Add the created GasStation to block's object list
      pNewBlock->m_nObjects = 1;
      pNewBlock->m_objectArray[0] = BTerrain::GetGasStationObject();
    }
  }

  return pNewBlock;
}



//*****************************************************************************
BTerrainBlock *BTerrain::DeleteVisibleBlock(BTerrainBlock *pBlock) {
// Removes the block from the ring and moves it to cache. Returns next visible block
  BTerrainBlock *pRet = m_ringVisible.Remove(pBlock);
  m_ringCache.AddFront(pBlock);
  while(m_ringCache.GetNBlocks() > 500) {
    BTerrainBlock *pOldest = m_ringCache.GetTail();
    m_ringCache.Remove(pOldest);
    delete pOldest;
  }
  return pRet;
}


//*****************************************************************************
int BTerrain::GetResBasedOnDistance(double dDistance) {
// Returns the configured resolution for the given distance
  if(m_bOverride) {
    return 6;
  }

  if(dDistance < 640.0) {
    return m_nRes1; // 5;
  }
  if(dDistance < 1000.0) {
    return m_nRes2; // 5;
  }
  if(dDistance < 1600.0) {
    return m_nRes3; // 4;
  } 
  if(dDistance < 2300.0) {
    return m_nRes4; // 3;
  } 
  if(dDistance < 2800.0) {
    return m_nRes5; // 3;
  } 
  return m_nRes6; // 3;
}




//*****************************************************************************
//void BTerrain::LoadTerrainCache(CString sFilenamePrefix) {
void BTerrain::LoadTerrainCache(string sFilenamePrefix) {
// Loads the cached terrain data from file to speed up rendering.
  FILE *fp;
  //CString sName = _T(".\\") + sFilenamePrefix + _T("_") + _T("TerrainCache.dat");
  string sName = sFilenamePrefix + "_" + "TerrainCache.dat";
  //fp = fopen(LPCTSTR(sName), "rb");
  fp = fopen(sName.c_str(), "rb");
  BVector vCenter;
  if(fp) {
    fread(&m_vOffset.m_dX, sizeof(double), 1, fp);
    fread(&m_vOffset.m_dY, sizeof(double), 1, fp);
    fread(&m_vOffset.m_dZ, sizeof(double), 1, fp);
    fread(&m_vCameraLoc.m_dX, sizeof(double), 1, fp);
    fread(&m_vCameraLoc.m_dY, sizeof(double), 1, fp);
    fread(&m_vCameraLoc.m_dZ, sizeof(double), 1, fp);
    fread(&m_vCameraLookDir.m_dX, sizeof(double), 1, fp);
    fread(&m_vCameraLookDir.m_dY, sizeof(double), 1, fp);
    fread(&m_vCameraLookDir.m_dZ, sizeof(double), 1, fp);
    BGame::SetProgressRange(90.0);
    BGame::SetProgressPos(0.0);
    int i = 1;
    while(fread(&vCenter.m_dX, sizeof(double), 1, fp)) {
      fread(&vCenter.m_dY, sizeof(double), 1, fp);
      fread(&vCenter.m_dZ, sizeof(double), 1, fp);
      BTerrainBlock *pBlock = CreateBlockAtAbs(vCenter, &m_ringVisible);
      pBlock->LoadTerrainBlockCache(fp);
      BGame::SetProgressPos(i);
      ++i;
    }
    BGame::SetProgressPos(100.0);
  } else {
    // DlgPreCache dialog;
    // dialog.m_nSteps = 9 * 9 + 1; // One for the save

    // dialog.Create(IDD_CREATING_TERRAIN);
    // CProgressCtrl *pProgress = static_cast<CProgressCtrl *>(dialog.GetDlgItem(IDC_PROGRESS1));
    // CStatic       *pTimeleft = static_cast<CStatic *>(dialog.GetDlgItem(IDC_TIMELEFT));

    // Create precache

    int nCreated = 1;
    BTerrainBlock *pBlockCenter = FindCenterBlock(m_vCameraLoc);
    // Add newly born blocks
    for(int nY = -5; nY <= 5; ++nY) {
      for(int nX = -5; nX <= 5; ++nX) {
        if((nX != 0) || (nY != 0)) { // Exclude center block, since that is guaranteed to be available
          if(!ExistsBlockForLocation(nX, nY, &m_ringVisible) && 
             ((GetCenterForRelLoc(pBlockCenter->GetCenter(), nX, nY) - m_vCameraLoc).Length() <= (cdWorldHemisphereRadius + 453.0))) {
            CreateBlockAt(pBlockCenter->GetCenter(), nX, nY);
            ++nCreated;
          }
        }
      }
    }

    //CString sTmp;
    string sTmp;
    time_t timeStart, timeNow;
    time(&timeStart);
    int nDone = 0;
    // pProgress->SetRange(0, nCreated);
    BGame::SetProgressRange(nCreated);
    // Make sure all display lists are ok
    BTerrainBlock *pBlock = m_ringVisible.GetHead();
    do {
      if(!pBlock->IsAtLeastOfRes(4)) { // Max res
        pBlock->CalculateWithRes(4, m_vOffset);
      }
      ++nDone;
      time(&timeNow);
      /*sTmp.Format("Time left: %d min %d sec...", 
                  int(double(nCreated - nDone) * double((timeNow - timeStart) / double(nDone)) / 60.0),
                  int(double(nCreated - nDone) * double((timeNow - timeStart) / double(nDone))) - int(double(nCreated - nDone) * double((timeNow - timeStart) / double(nDone)) / 60.0) * 60);*/ //FIXME
      // pTimeleft->SetWindowText(sTmp);
      // pProgress->SetPos(nDone);
      BGame::SetProgressPos(nDone);
      pBlock = pBlock->m_pNext;
    } while(pBlock != m_ringVisible.GetHead());

    // Save it
    SaveTerrainCache(m_sSceneName);
    ++nDone;
    // pProgress->SetPos(nDone);
    BGame::SetProgressPos(nDone);

    // dialog.DestroyWindow();
  }

  // Load extra cache
  //sName = _T(".\\") + sFilenamePrefix + _T("_") + _T("TerrainCache2.dat");
  sName = sFilenamePrefix + "_" + "TerrainCache2.dat";
  //fp = fopen(LPCTSTR(sName), "rb");
  fp = fopen(sName.c_str(), "rb");
  if(fp) {
    while(fread(&vCenter.m_dX, sizeof(double), 1, fp)) {
      fread(&vCenter.m_dY, sizeof(double), 1, fp);
      fread(&vCenter.m_dZ, sizeof(double), 1, fp);
      BTerrainBlock *pBlock = CreateBlockAtAbs(vCenter, &m_ringCache);
      pBlock->LoadTerrainBlockCache(fp);
    }
  }
}

//*****************************************************************************
void BTerrain::CreateTerrainDisplayLists() {
  // Make sure all display lists are ok
  BTerrainBlock *pBlock = m_ringVisible.GetHead();
  do {
    if(!pBlock->IsAtLeastOfRes(4)) { // Max res
      pBlock->CalculateWithRes(4, m_vOffset);
    }
    pBlock->CreateDisplayListsOfRes(3, true, false, false);
    pBlock = pBlock->m_pNext;
  } while(pBlock != m_ringVisible.GetHead());
}


//*****************************************************************************
//void BTerrain::SaveTerrainCache(CString sFilenamePrefix) {
void BTerrain::SaveTerrainCache(string sFilenamePrefix) {
// Saves the current terrain data to a cache file to speed up rendering.
  if(m_ringVisible.GetHead()) {
    FILE *fp;
    //CString sName = _T(".\\") + sFilenamePrefix + _T("_") + _T("TerrainCache.dat");
    string sName = sFilenamePrefix + "_" + "TerrainCache.dat";
    //fp = fopen(LPCTSTR(sName), "wb");
    fp = fopen(sName.c_str(), "wb");
    if(fp) {
      fwrite(&m_vOffset.m_dX, sizeof(double), 1, fp);
      fwrite(&m_vOffset.m_dY, sizeof(double), 1, fp);
      fwrite(&m_vOffset.m_dZ, sizeof(double), 1, fp);
      fwrite(&m_vCameraLoc.m_dX, sizeof(double), 1, fp);
      fwrite(&m_vCameraLoc.m_dY, sizeof(double), 1, fp);
      fwrite(&m_vCameraLoc.m_dZ, sizeof(double), 1, fp);
      fwrite(&m_vCameraLookDir.m_dX, sizeof(double), 1, fp);
      fwrite(&m_vCameraLookDir.m_dY, sizeof(double), 1, fp);
      fwrite(&m_vCameraLookDir.m_dZ, sizeof(double), 1, fp);
      
      BTerrainBlock *pBlock = m_ringVisible.GetHead();
      do {
        pBlock->SaveTerrainBlockCache(fp);
        pBlock = pBlock->m_pNext;
      } while(pBlock != m_ringVisible.GetHead());
      fclose(fp);
    } else {
      BGame::MyAfxMessageBox("Cannot open terrain cache file for writing.");
    }
  }

  // Save extra cache
  if(m_ringCache.GetHead()) {
    FILE *fp;
    //CString sName = sFilenamePrefix + _T("_") + _T("TerrainCache2.dat");
    string sName = sFilenamePrefix + "_" + "TerrainCache2.dat";
    //fp = fopen(LPCTSTR(sName), "wb");
    fp = fopen(sName.c_str(), "wb");
    if(fp) {
      BTerrainBlock *pBlock = m_ringCache.GetHead();
      do {
        pBlock->SaveTerrainBlockCache(fp);
        pBlock = pBlock->m_pNext;
      } while(pBlock != m_ringCache.GetHead());
      fclose(fp);
    } else {
      BGame::MyAfxMessageBox("Cannot open secondary terrain cache file for writing.");
    }
  }
}



//*****************************************************************************
//void BTerrain::StartUsingScene(CString sSceneName, 
void BTerrain::StartUsingScene(string sSceneName, 
                               BVector vOrigin, 
                               double dGroundTextureScaler1, 
                               double dGroundTextureScaler2) {
  m_sSceneName = sSceneName;
  m_vOffset = vOrigin;
  m_fGroundTextureScaler1 = GLfloat(dGroundTextureScaler1);
  m_fGroundTextureScaler2 = GLfloat(dGroundTextureScaler2);

  // Load/Create scene's terrain cache
  LoadTerrainCache(sSceneName);
}


//*****************************************************************************
void BTerrain::StopUsingScene() {
  // Save and free scene's terrain cache
  //if(!m_sSceneName.IsEmpty()) {
  if(!m_sSceneName.empty()) {
    SaveTerrainCache(m_sSceneName);
    //m_sSceneName = _T("");
    m_sSceneName = "";
  }

  m_ringVisible.RemoveAll();
  m_ringCache.RemoveAll();
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
