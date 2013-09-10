// (c) Copyright 2002, Mikko Oksalahti (see end of file for details)
//

//#include "stdafx.h"
#include "HeightMap.h"
#include "PerlinNoise.h"




double g_dMap1[cnMapSize][cnMapSize];

void HeightMap::GenerateMap(double dXOffset, double dYOffset, double dWidth) {
  // Calculate the whole map
  double dTmp;
  int    x, y;
  double dRatio = double(cnMapSize) / dWidth;
  for(y = 0; y < cnMapSize; ++y) {
    for(x = 0; x < cnMapSize; ++x) {
      g_dMap1[x][y] = CalcHeightAt(double(x) / dRatio + dXOffset, 
                                   double(y) / dRatio + dYOffset, 
                                   dTmp);
    }
  }
}


double HeightMap::HeightAt(double dX, double dY) {
  if(int(dX) < 0 || int(dX) >= cnMapSize || int(dY) < 0 || int(dY) >= cnMapSize) {
    return 0.0;
  } else {
    return g_dMap1[int(dX)][int(dY)];
  }
}

inline double HeightMap::CalcHeightAt(double dX, double dY, double &rdArea, TTerrain nTerrain, int nOctaves) {
  double dTmp = 0.0;
  double dMapSize = 100.0;
  switch(nTerrain) {

    case MONUMENT_VALLEY:
      dTmp = CalcHeightAt(dX / 2.0, dY / 2.0, dTmp, LAKE_POWELL) * 1.3;
      if(dTmp < 0.0) {
        return dTmp * 2.0; //  * 0.2;
      } else if(dTmp > 0.2) {
        return (0.2 + (dTmp - 0.2) * 0.05) * 4.5;
      }
      return dTmp * 4.5;
      break;

    case LAKE_POWELL:
      dTmp = 
        (PerlinNoise::PerlinNoise_2D((dX + 7.0) / (dMapSize / 7.0), 
                                             dY / (dMapSize / 7.0), 
                                     6,
                                     cdPercistence / 2.0)) * 1.5;
      dTmp = pow(dTmp, 2.0);
      return
        -(PerlinNoise::PerlinNoise_2D(dX / (dMapSize / 15.1), 
                                      dY / (dMapSize / 15.1), 
                                      nOctaves,
                                      cdPercistence) * 3.0 * dTmp - 0.01); // Correct for water level
      break;

    case SCOTLAND:

      {
        double dFlatterer = CalcHeightAt(dX / 4.3, dY / 4.3, dTmp, LAKE_POWELL);
        dFlatterer = (0.98 + dFlatterer);
        if(dFlatterer < 0.0) {
          dFlatterer = 0.0;
        } else if(dFlatterer > 1.0) {
          dFlatterer = 1.0;
        }
        return pow(dFlatterer, 80.0);
      }

      return 
        PerlinNoise::PerlinNoise_2D(dX / (dMapSize / 13.0), 
                                    dY / (dMapSize / 13.0), 
                                    2,
                                    cdPercistence * 0.25) * 2.0;
      break;

    case NOISE:
      return PerlinNoise::PerlinNoise_2D((dX + 7.0) / (dMapSize / 0.73), 
                                         dY         / (dMapSize / 0.73), 
                                         6,
                                         cdPercistence / 1.1);
      break;
    case PAINTED_DESERT:
      dTmp = 
        (PerlinNoise::PerlinNoise_2D((dX + 7.0) / (dMapSize / 33.7), 
                                     dY         / (dMapSize / 33.7), 
                                     6,
                                     cdPercistence / 2.0)) * 1.5;
      return pow(fabs(dTmp), 0.25); 

    case MIXER:
      dTmp = 
        (PerlinNoise::PerlinNoise_2D((dX + 7.0) / (dMapSize / 0.73), 
                                     dY         / (dMapSize / 0.73), 
                                     6,
                                     cdPercistence / 1.1)) * 1.5;
      return dTmp * dTmp;
      // return pow(dTmp, 3.0); 

    case CONTINENTS:
      return 
        PerlinNoise::PerlinNoise_2D(dX / (dMapSize * 6.151), 
                                    dY / (dMapSize * 6.151), 
                                    nOctaves,
                                    cdPercistence * 0.5) * 2.0;
      break;

    case OVERALL:

      { // Elegant version??? (faster?)

        dX /= 10.0;
        dY /= 10.0;

        rdArea = 0;
        double dLarge      = CalcHeightAt(dX / 21.1,  dY / 21.1,  dTmp, CONTINENTS);
        double dMedium     = CalcHeightAt(dX /  6.1,  dY /  6.1,  dTmp, LAKE_POWELL, cnOctaves + 1) * 500.0;
        double dFlatterer1 = CalcHeightAt(dX / 11.23, dY / 11.23, dTmp, LAKE_POWELL);
        dFlatterer1 = (0.95 + dFlatterer1);
        if(dFlatterer1 < 0.0) {
          dFlatterer1 = 0.0;
        } else if(dFlatterer1 > 1.0) {
          dFlatterer1 = 1.0;
        }
        rdArea = pow(dFlatterer1, 98.0);
        dMedium *= (rdArea);

        if(fabs(dLarge) < 0.05) {
          double dFlatterer2 = CalcHeightAt(dX / 3.07, dY / 3.07, dTmp, MIXER) * 3.0 + 0.1;
          double d1, d2;
          double dMixer = CalcHeightAt(dX, dY, dTmp, MIXER);
          d1 = CalcHeightAt(dX, dY, dTmp, MONUMENT_VALLEY) * 100.0;
          // d2 = CalcHeightAt(dX, dY, dTmp, PAINTED_DESERT) * 30.0;
          d2 = CalcHeightAt(dX / 8.0, dY / 8.0, dTmp, PAINTED_DESERT) * 300.0;
          dMixer -= 0.1; // was 0.2;
          if(dMixer < 0.0) {
            dMixer = 0.0;
          }
          if(dMixer > 1.0) {
            dMixer = 1.0;
          }
          double dMixed = PerlinNoise::Cosine_Interpolate(d1, d2, dMixer);
          dMixed *= dFlatterer2;
          if(fabs(dLarge) > 0.025) {
            dMedium = PerlinNoise::Cosine_Interpolate(dMixed, dMedium, (fabs(dLarge) - 0.025) / 0.025);
          } else {
            dMedium += dMixed;
          }
        }

        double dRet = dLarge * 2000.0 + dMedium;
        if(dRet < 0.0) {
          return -3.0 + dRet;
        }
        return dRet;
      }

      break;
  }
  return 0.0;
}


BVector HeightMap::NormalAt(double dX, double dY) {
  BVector vNormal(0, 0, 0);
  BVector vRef(0, 1, 0);
  BVector vToNext(1, 0, HeightMap::HeightAt(dX + 1, dY) - HeightMap::HeightAt(dX, dY));
  vToNext.ToUnitLength();
  vNormal += vRef.CrossProduct(vToNext);

  vRef.Set(0, -1, 0);
  vToNext.Set(-1, 0, HeightMap::HeightAt(dX - 1, dY) - HeightMap::HeightAt(dX, dY));
  vToNext.ToUnitLength();
  vNormal += vRef.CrossProduct(vToNext);

  vRef.Set(1, 0, 0);
  vToNext.Set(0, 1, HeightMap::HeightAt(dX, dY - 1) - HeightMap::HeightAt(dX, dY));
  vToNext.ToUnitLength();
  vNormal += vRef.CrossProduct(vToNext);

  vRef.Set(-1, 0, 0);
  vToNext.Set(0, -1, HeightMap::HeightAt(dX, dY + 1) - HeightMap::HeightAt(dX, dY));
  vToNext.ToUnitLength();
  vNormal += vRef.CrossProduct(vToNext);

  vNormal.ToUnitLength();

  return vNormal;
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
