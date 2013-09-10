//
// BSceneModule
//

#pragma once

#include "BaseClasses.h"
#include "BObject.h"
#include "GL/gl.h"
#include "GL/glu.h"
#include <string>

using namespace std;

class BSceneModule {
  void DestroyCurrentScene();
  bool LoadCommonSceneParams(string sSceneName);
  bool LoadObjects(string sSceneName);
  char *ReadVectors(FILE *fp, BVector *pVectors, int nMaxToRead);
public:
  string  m_sSceneName;
  int      m_nObjects;
  BObject *m_pObject;
  BVector  m_vOrigin;

  BSceneModule() {m_sSceneName = ""; m_nObjects = 0; m_pObject = 0;}
  void LoadScene(string sSceneName = "Startup");
};
