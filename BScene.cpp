//
// Scene
//
// (c) Copyright 2002, Mikko Oksalahti (see end of file for details)
//

#include <sstream>

using namespace std;

//#include "stdafx.h"
#include "BScene.h"
#include "BTextures.h"
#include "BGame.h"
#include "FileIOHelpers.h"

#include "StringTools.h"

//*************************************************************************************************
BScene::BScene() {
  m_dFriction = 0.5;
  m_vOrigin.Set(0, 0, 0);
  m_vStartLocation.Set(0, 0, 0);
  m_nDeliveryOrderEntries = 0;
  m_bRandomDeliveryInUse = true;
  m_nCurrentDeliveryEntry = 0;
  m_bSceneInUse = false;
  m_dGroundTextureScaler1 = 0.1;
  m_dGroundTextureScaler2 = 0.01;
  //m_sSkyTexture = _T("");
  m_sSkyTexture = "";
  //m_sGround1Texture = _T("");
  m_sGround1Texture = "";
  //m_sGround2Texture = _T("");
  m_sGround2Texture = "";
  //m_sEnvMapTexture = _T("");
  m_sEnvMapTexture = "";
  m_nObjects = 0;
  m_OBJData.Init();
  m_sName = "";
  m_sFilename = "";
}


//*************************************************************************************************
BScene::~BScene() {
  CleanUp();
}


//*************************************************************************************************
void BScene::CleanUp() {
  m_nObjects = 0;

  BGame::GetSimulation()->GetTerrain()->StopUsingScene();
}

double g_dSceneFriction = 0.5;

//*************************************************************************************************
//void BScene::LoadSceneFromFile(CString sFilename) {
void BScene::LoadSceneFromFile(string sFilename) {

  if(m_bSceneInUse) {
    CleanUp();
  }
  // Load scene
  m_bSceneInUse = true;

  // General properties

  FileHelpers::GetKeyStringFromINIFile("Properties", "Name", "default", m_sName, sFilename);
  FileHelpers::GetKeyStringFromINIFile("Properties", "Image", "default", m_sImageFilename, sFilename);
  FileHelpers::GetKeyDoubleFromINIFile("Properties", "GroundFriction", 0.5, m_dFriction, sFilename);
  g_dSceneFriction = m_dFriction;
  FileHelpers::GetKeyDoubleFromINIFile("Properties", "Ground1Scaler", 0.1, m_dGroundTextureScaler1, sFilename);
  FileHelpers::GetKeyDoubleFromINIFile("Properties", "Ground2Scaler", 0.01, m_dGroundTextureScaler2, sFilename);
  FileHelpers::GetKeyStringFromINIFile("Properties", "SkyTexture", "", m_sSkyTexture, sFilename);
  FileHelpers::GetKeyStringFromINIFile("Properties", "Ground1Texture", "", m_sGround1Texture, sFilename);
  FileHelpers::GetKeyStringFromINIFile("Properties", "Ground2Texture", "", m_sGround2Texture, sFilename);
  FileHelpers::GetKeyStringFromINIFile("Properties", "EnvMapTexture", "", m_sEnvMapTexture, sFilename);
  FileHelpers::GetKeyDoubleFromINIFile("Properties", "Wind", 0.0, m_dWindFactor, sFilename);
  if(m_dWindFactor > 0.0001) {
    BGame::m_dBaseWindStrength = 0.1 * m_dWindFactor;
    BGame::m_bWindActive = true;
  } else {
    BGame::m_dBaseWindStrength = 0.0;
    BGame::m_dWindStrength = 0.0;
    BGame::m_bWindActive = false;
  }
  //CString sTmp;
  string sTmp;
  FileHelpers::GetKeyStringFromINIFile("Properties", "Earthquakes", "false", sTmp, sFilename);
  //m_bHasEarthquakes = sTmp.CompareNoCase("true") == 0;
  m_bHasEarthquakes = StringTools::compareNoCase(sTmp, "true") == 0;
  BGame::m_bHasEarthquakes = m_bHasEarthquakes;

  //if(!m_sSkyTexture.IsEmpty()) {
  if(!m_sSkyTexture.empty()) {
    //BTextures::ReloadTexture(BTextures::Texture::SKY, m_sSkyTexture);
    BTextures::ReloadTexture(BTextures::SKY, m_sSkyTexture);
  }
  //if(!m_sGround1Texture.IsEmpty()) {
  if(!m_sGround1Texture.empty()) {
    //BTextures::ReloadTexture(BTextures::Texture::GROUND_BASE, m_sGround1Texture);
    BTextures::ReloadTexture(BTextures::GROUND_BASE, m_sGround1Texture);
  }
  //if(!m_sGround2Texture.IsEmpty()) {
  if(!m_sGround2Texture.empty()) {
    //BTextures::ReloadTexture(BTextures::Texture::GROUND_COLOR_MAP, m_sGround2Texture);
    BTextures::ReloadTexture(BTextures::GROUND_COLOR_MAP, m_sGround2Texture);
  }
  //if(!m_sEnvMapTexture.IsEmpty()) {
  if(!m_sEnvMapTexture.empty()) {
    //BTextures::ReloadTexture(BTextures::Texture::ENVMAP, m_sEnvMapTexture);
    BTextures::ReloadTexture(BTextures::ENVMAP, m_sEnvMapTexture);
  }

  // Shape/Geometry related properties

  FileHelpers::GetKeyVectorFromINIFile("Geometry", "Origin", BVector(0, 0, 0), m_vOrigin, sFilename);
  FileHelpers::GetKeyVectorFromINIFile("Geometry", "StartLocation", BVector(0, 0, 0), m_vStartLocation, sFilename);

  // Object related properties
  //CString sValue;
  string sValue;
  FileHelpers::GetKeyStringFromINIFile("Delivery", "Order", "", sValue, sFilename);
  //m_nDeliveryOrderEntries = sscanf(LPCTSTR(sValue), 
  m_nDeliveryOrderEntries = sscanf(sValue.c_str(), 
                                   "%d, %d, %d, %d, %d, %d, %d, %d, %d, %d)",
                                   &(m_nDeliveryOrder[0]),   
                                   &(m_nDeliveryOrder[1]),   
                                   &(m_nDeliveryOrder[2]),   
                                   &(m_nDeliveryOrder[3]),   
                                   &(m_nDeliveryOrder[4]),   
                                   &(m_nDeliveryOrder[5]),   
                                   &(m_nDeliveryOrder[6]),   
                                   &(m_nDeliveryOrder[7]),   
                                   &(m_nDeliveryOrder[8]),   
                                   &(m_nDeliveryOrder[9]));  
  m_nCurrentDeliveryEntry = 0;
  FileHelpers::GetKeyStringFromINIFile("Delivery", "RandomOrderInUse", "false", sValue, sFilename);
  //m_bRandomDeliveryInUse = (sValue.CompareNoCase("true") == 0);
  m_bRandomDeliveryInUse = (sValue.compare("true") == 0);

  // Load objects
  // First count them

  m_nObjects = 0;
  do {
    //CString sHasSection;
    string sHasSection;
    //CString sSection;
    string sSection;
    //sSection.Format("Object%d", m_nObjects + 1);
    stringstream format;
    format << "Object" << m_nObjects + 1;
    sSection = format.str();
    FileHelpers::GetKeyStringFromINIFile(sSection, "", "default", sHasSection, sFilename);
    //if(sHasSection.CompareNoCase("default") != 0) {
    if(sHasSection.compare("default") != 0) {
      ++m_nObjects;
    } else {
      break;
    }
  } while(m_nObjects < 100); // just a sanity check to break the loop eventually

  // Read objects
  int nObject = 0;
  for(nObject = 0; nObject < m_nObjects; ++nObject) {
    //CString sSection;
    string sSection;
    //sSection.Format("Object%d", nObject + 1);
    stringstream format;
    format << "Object" << nObject + 1;
    sSection = format.str();
    m_pObjects[nObject].SetOBJData(&m_OBJData);
    m_pObjects[nObject].LoadObjectFromFile(sFilename, sSection);
  }

  // Prepare objects
  m_OBJData.PrepareWaveFrontModel();
  for(nObject = 0; nObject < m_nObjects; ++nObject) {
    m_pObjects[nObject].Setup();
    m_pObjects[nObject].PreProcessVisualization();
  }
}

//*************************************************************************************************
void BScene::PlaceTerrainObjects(BTerrainBlock *pBlockSingle) {
  // Place objects on the terrain blocks
  PlaceObjectsOnTerrain(pBlockSingle);

  // Update object list
  UpdateObjectList();
}


//*************************************************************************************************
void BScene::UpdateObjectList() {
  // Update the selection list for scene objects
  for(int nObject = 0; nObject < m_nObjects; ++nObject) {
    m_sSceneObjectNames[nObject] = m_pObjects[nObject].m_sName;
  }
  m_sellistSceneObjects.SetItems(m_sSceneObjectNames, m_nObjects);
}


//*************************************************************************************************
void BScene::PlaceObjectsOnTerrain(BTerrainBlock *pBlockSingle) {
  for(int nObject = 0; nObject < m_nObjects; ++nObject) {
    // if object's bounding sphere "touches" terrain block,
    // add the object's pointer to the block.

    if(pBlockSingle) {
      // Place objects on single block (if not there already)
      BVector vCenter = pBlockSingle->GetCenter();
      if(pBlockSingle->PointIsInsideBlock(m_pObjects[nObject].m_vCenter, m_pObjects[nObject].m_dRadius)) {
        // Check to see if object is there already
        bool object_found = false;
        for(int i = 0; !object_found && i < pBlockSingle->m_nObjects; ++i) {
          if(pBlockSingle->m_objectArray[i] == &m_pObjects[nObject]) {
            // The object is already in the block's object list --> go to next object
            //continue;
            object_found = true;
          }
        }
        
        if (!object_found) {
			pBlockSingle->m_objectArray[pBlockSingle->m_nObjects] = &m_pObjects[nObject];
			++pBlockSingle->m_nObjects;
		}
      }
    } else {
      // Place objects on every visible block
      BTerrainBlock *pBlock = BGame::GetSimulation()->GetTerrain()->m_ringVisible.GetHead();
      do {      
        BVector vCenter = pBlock->GetCenter();
        if(pBlock->PointIsInsideBlock(m_pObjects[nObject].m_vCenter, m_pObjects[nObject].m_dRadius)) {
          pBlock->m_objectArray[pBlock->m_nObjects] = &m_pObjects[nObject];
          ++pBlock->m_nObjects;
        }

        pBlock = pBlock->m_pNext;
      } while (pBlock != BGame::GetSimulation()->GetTerrain()->m_ringVisible.GetHead());
    }
  }
}


//*************************************************************************************************
void BScene::Save() {

  // Create scene subdirectory, if one doesn't exist already

  // Save Scene file
  FILE *fp;
  //fp = fopen(m_sFilename, "w");
  fp = fopen(m_sFilename.c_str(), "w");
  if(fp) {
    fprintf(fp, "; Scene file saved by Pakoon Scene Editor v1.ONE\n\n");

    fprintf(fp, "[Properties]\n");
    fprintf(fp, "\n");
    //fprintf(fp, " Name = %s\n", m_sName);
    fprintf(fp, " Name = %s\n", m_sName.c_str());
    //fprintf(fp, " Image = %s\n", m_sImageFilename);
    fprintf(fp, " Image = %s\n", m_sImageFilename.c_str());
    fprintf(fp, " GroundFriction = %g\n", m_dFriction);
    fprintf(fp, " Ground1Scaler = %g\n", m_dGroundTextureScaler1);
    fprintf(fp, " Ground2Scaler = %g\n", m_dGroundTextureScaler2);
    //fprintf(fp, " SkyTexture = %s\n", m_sSkyTexture);
    fprintf(fp, " SkyTexture = %s\n", m_sSkyTexture.c_str());
    //fprintf(fp, " Ground1Texture = %s\n", m_sGround1Texture);
    fprintf(fp, " Ground1Texture = %s\n", m_sGround1Texture.c_str());
    //fprintf(fp, " Ground2Texture = %s\n", m_sGround2Texture);
    fprintf(fp, " Ground2Texture = %s\n", m_sGround2Texture.c_str());
    //fprintf(fp, " EnvMapTexture = %s\n", m_sEnvMapTexture);
    fprintf(fp, " EnvMapTexture = %s\n", m_sEnvMapTexture.c_str());
    fprintf(fp, " Wind = %.5lf\n", m_dWindFactor);
    fprintf(fp, " Earthquakes = %s\n", m_bHasEarthquakes ? "True" : "False");

    fprintf(fp, "\n");

    fprintf(fp, "[Geometry]\n");
    fprintf(fp, "\n");
    fprintf(fp, " Origin = %lf, %lf, %lf\n", m_vOrigin.m_dX, m_vOrigin.m_dY, m_vOrigin.m_dZ);
    fprintf(fp, " StartLocation = %lf, %lf, %lf\n", m_vStartLocation.m_dX, m_vStartLocation.m_dY, m_vStartLocation.m_dZ);
    fprintf(fp, "\n");

    fprintf(fp, "[Delivery]\n");
    fprintf(fp, "\n");
    //CString sTmp = "";
    string sTmp = "";
    int i;
    for(i = 0; i < m_nDeliveryOrderEntries; ++i) {
      //CString sTmp2;
      string sTmp2;
      //sTmp2.Format("%d", m_nDeliveryOrder[i]);
		stringstream format;
		format << m_nDeliveryOrder[i];
		sTmp2 = format.str();
      if(i < (m_nDeliveryOrderEntries - 1)) {
        sTmp2 = sTmp2 + ", ";
      }
      sTmp = sTmp + sTmp2;
    }
    //fprintf(fp, " Order = %s\n", sTmp);
    fprintf(fp, " Order = %s\n", sTmp.c_str());
    fprintf(fp, " RandomOrderInUse = %s\n", (m_bRandomDeliveryInUse ? "True" : "False"));
    fprintf(fp, "\n");

    for(i = 0; i < m_nObjects; ++i) {
      //sTmp.Format("[Object%d]\n", i + 1);
		stringstream format;
		format << "[Object" << i + 1 << "]";
		sTmp = format.str();
      //fprintf(fp, sTmp);
      fprintf(fp, sTmp.c_str());
      fprintf(fp, "\n");
      //fprintf(fp, " Name = %s\n", m_pObjects[i].m_sName);
      fprintf(fp, " Name = %s\n", m_pObjects[i].m_sName.c_str());
      //if(m_pObjects[i].m_type == BObject::TType::CLIENT) {
      if(m_pObjects[i].m_type == BObject::CLIENT) {
        fprintf(fp, " Type = Client\n");
      //} else if(m_pObjects[i].m_type == BObject::TType::BASE) {
      } else if(m_pObjects[i].m_type == BObject::BASE) {
        fprintf(fp, " Type = Base\n");
      } else {
        fprintf(fp, " Type = Other\n");
      }
      //fprintf(fp, " ObjectFile = %s\n", m_pObjects[i].m_sObjectFilename);
      fprintf(fp, " ObjectFile = %s\n", m_pObjects[i].m_sObjectFilename.c_str());
      fprintf(fp, " Location = %g, %g, %g\n", m_pObjects[i].m_vLocation.m_dX, m_pObjects[i].m_vLocation.m_dY, m_pObjects[i].m_vLocation.m_dZ);
      fprintf(fp, " ZRotation = %g\n", m_pObjects[i].m_dZRotation);
      fprintf(fp, " Scale = %g\n", m_pObjects[i].m_dScale2);
      fprintf(fp, " Shadow = %s\n", (m_pObjects[i].m_bHasShadow ? "True" : "False"));
      fprintf(fp, " ActiveRadius = %g\n", m_pObjects[i].m_dActiveRadius);
      fprintf(fp, "\n");
    }
    fclose(fp);
  }
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
