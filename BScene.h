//
// Scene
//
// (c) Copyright 2002, Mikko Oksalahti (see end of file for details)
//

#pragma once

#include "BaseClasses.h"
#include "BObject.h"
#include "BTerrain.h" 
#include "BUI.h"
#include "GL/gl.h"
#include "GL/glu.h"
#include <string>

using namespace std;


class BScene {

public:

  // General properties

  string       m_sFilename;
  string       m_sName;
  string       m_sImageFilename;
  double        m_dFriction;
  double        m_dGroundTextureScaler1;
  double        m_dGroundTextureScaler2;
  string       m_sSkyTexture;
  string       m_sGround1Texture;
  string       m_sGround2Texture;
  string       m_sEnvMapTexture;
  double        m_dWindFactor;
  bool          m_bHasEarthquakes;
  bool          m_bSceneInUse;

  // Shape/Geometry related properties

  BVector  m_vOrigin;
  BVector  m_vStartLocation;
  BOBJData m_OBJData;

  // Object related properties

  int     m_nCurrentDeliveryEntry;
  int     m_nDeliveryOrderEntries;
  int     m_nDeliveryOrder[10];
  bool    m_bRandomDeliveryInUse;

  // Objects

  int     m_nObjects;
  BObject m_pObjects[100];

  string          m_sSceneObjectNames[100];
  BUISelectionList m_sellistSceneObjects;

  BScene();
  ~BScene();
  void CleanUp();
  void Save();
  void LoadSceneFromFile(string sFilename);
  void PlaceTerrainObjects(BTerrainBlock *pBlockSingle = 0);
  void PlaceObjectsOnTerrain(BTerrainBlock *pBlockSingle = 0);
  void UpdateObjectList();
};



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
