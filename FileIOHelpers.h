//
// File IO helpers
//
// (c) Copyright 2002, Mikko Oksalahti (see end of file for details)
//

#pragma once

#include <string>
#include <map>

using namespace std;

#include "BaseClasses.h"


namespace FileHelpers {
	map<string, map<string, string> > readMapFromFile(string filename);
	bool WriteMapToFile(map<string, map<string, string> > filemap, string filename);
  int WriteKeyStringToINIFile(string  sSection,        // section name
                              string  sKeyName,        // key name
                              string  sValue,          // value to write
                              string  sFilename);      // initialization file name
  int GetKeyStringFromINIFile(string  sSection,        // section name
                              string  sKeyName,        // key name
                              string  sDefault,        // default string
                              string &sReturnedString, // destination buffer
                              string  sFilename);      // initialization file name
  int GetKeyDoubleFromINIFile(string  sSection,        // section name
                              string  sKeyName,        // key name
                              double   dDefault,        // default
                              double  &dReturnedDouble, // destination buffer
                              string  sFilename);      // initialization file name
  int GetKeyIntFromINIFile(   string  sSection,        // section name
                              string  sKeyName,        // key name
                              int      nDefault,        // default
                              int &nReturnedInt,        // destination buffer
                              string  sFilename);      // initialization file name
  int GetKeyVectorFromINIFile(string  sSection,        // section name
                              string  sKeyName,        // key name
                              BVector  vDefault,        // default
                              BVector &vReturnedVector, // destination buffer
                              string  sFilename);      // initialization file name
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
