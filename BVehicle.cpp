//
// Vehicle
//
// (c) Copyright 2002, Mikko Oksalahti (see end of file for details)
//

#include <sstream>

using namespace std;

//#include "stdafx.h"
#include "BVehicle.h"

#include "GL/gl.h"
#include "GL/glu.h"
#include "OpenGLHelpers.h"
#include "OpenGLExtFunctions.h"
#include "BTextures.h"
//#include "resource.h"
#include "BGame.h"
#include "FileIOHelpers.h"
#include "Pakoon1Doc.h"
#include "Pakoon1View.h"

const double g_cdPI = 3.141592654;
extern bool g_cbBlackAndWhite;



//**********************************************************************

BVehicle::BVehicle() {
  m_nBodyPoints     = 0;
  m_pBodyPoint      = 0;
  m_nWheels         = 0;
  m_pPart           = 0;
  m_dRPM            = 0;
  m_dFuel           = 50.0; // half tank
  m_dKerosine       = 50.0; // half tank
  m_dFuelFactor     = 1.0;
  m_dKerosineFactor = 1.0;
  m_dHorsePowers    = 1.0;
  m_nLiftPoint1     = 0;
  m_nLiftPoint2     = 0;

  m_nDLVehicleBody = -1; // force creation of display list for the vehicle
  m_nDLElevator = -1; // force creation of display list for the vehicle
  m_nDLRudder = -1; // force creation of display list for the vehicle
  m_nDLPropeller = -1; // force creation of display list for the vehicle

  InitAll();

  m_vHomeLocation.Set(0, 0, 0);
  // Move(m_vHomeLocation);
}

//**********************************************************************

void BVehicle::ConsumeFuel(double dAmount) {
  m_dFuel -= dAmount;
  if(m_dFuel < 0.0) {
    BMessages::Remove("outoffuel");
    BMessages::Show(40, "outoffuel", "Out of fuel!", 2, false, 1, 0, 0);
    m_dFuel = 0.0;
    m_dFuelFactor = 0.0; // Out of fuel. Engine no longer runs
    m_dAccelerationFactor = 0.0;
    m_dReversingFactor = 0.0;
  }
}

//**********************************************************************

void BVehicle::ConsumeKerosine(double dAmount) {
  static bool bKerosineOut = false;
  m_dKerosine -= dAmount;
  if(m_dKerosine < 0.0) {
    BMessages::Remove("outofkero");
    BMessages::Show(40, "outofkero", "Out of kerosine!", 2, false, 1, 0, 0);
    m_dKerosine = 0.0;
    m_dKerosineFactor = 0.0; // Out of kerosine. jet/copter no longer run
    if(m_rotor.m_dHeliMode > 100.0 && !bKerosineOut) {
      BGame::GetSimulation()->ToggleHeli();
    }
    if(m_jet.m_dJetMode > 100.0 && !bKerosineOut) {
      BGame::Command()->Run("toggle jet");
    }
    bKerosineOut = true;
  }
}

//**********************************************************************

void BVehicle::InitAll() {
  m_dSpeed     = 0.0;
  m_bBreaking     = false;
  m_bHandBreaking = false;
  m_bAccelerating = false;
  m_bReversing    = false;
  m_bTurningLeft  = false;
  m_bTurningRight = false;
  m_dTurn         = 0.0;
  m_dSpeedKmh = 0.0;
  m_bWireframe = false;

  m_rotor.m_nHeliMode = 0;
  m_rotor.m_bHeliModeActivating = false;
  m_rotor.m_bHeliLifting = false;
  m_rotor.m_bHeliDescending = false;
  m_rotor.m_bHeliForwarding = false;
  m_rotor.m_bHeliRighting = false;
  m_rotor.m_bHeliBacking = false;
  m_rotor.m_bHeliLefting = false;
  m_rotor.m_dHeliLift = 0.0;
  m_rotor.m_dHeliForward = 0.0;
  m_rotor.m_dHeliRight = 0.0;
  m_rotor.m_dHeliMode = 0.0;
  m_rotor.m_bHeliBladeOK[0] = m_rotor.m_bHeliBladeOK[1] = m_rotor.m_bHeliBladeOK[2] = true;
  m_rotor.m_bHeliCoverOK = true;
  m_rotor.m_bHeliHatchesOK = true;
  m_rotor.m_dHeliLeftHatchOffsetAngle = 0.0;
  m_rotor.m_dHeliRightHatchOffsetAngle = 0.0;
  m_rotor.m_dHeliBladePower = 1.0;
  m_rotor.m_bHeliOK = true;

  m_jet.m_nJetMode = 0;
  m_jet.m_dJetMode = 0.0;
  m_jet.m_bJetModeActivating = false;

  m_dAccelerationFactor = 1.0;
  m_dReversingFactor = 1.0;

  m_dSteeringAid = 1.0;

  m_airplane.m_dRudder = 0;
  m_airplane.m_dElevator = 0;
  m_airplane.m_dAilerons = 0;
  m_dPropellerFactor = 0;

  m_nTrailpoints = 0;
  m_nTrailpointEntries = 0;
  m_nTrailpointHead = 0;

  m_OBJData.Init();

  m_vLocation = BVector(0, 0, 0);
}


//**********************************************************************

void BVehicle::DeleteAll() {
  if(m_pBodyPoint) {
    delete [] m_pBodyPoint;
    m_pBodyPoint = 0;
  }
  for(int i = 0; i < 10; ++i) {
    if(m_pWheel[i]) {
      delete m_pWheel[i];
    }
    m_nWheels = 0;
  }
  if(m_pPart) {
    delete [] m_pPart;
    m_pPart = 0;
  }

  m_OBJData.Free();
}

//**********************************************************************

BVehicle::~BVehicle() {
  DeleteAll();
}

//**********************************************************************

void BVehicle::LoadTextures() {
}


//**********************************************************************

void BVehicle::LoadVehicleFromFile(string sFilename) {
  DeleteAll();
  InitAll();

  int i;
  string sKey, sValue;

  //* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
  // First load in straight-forward properties

  FileHelpers::GetKeyStringFromINIFile("Properties", "Name", "default", m_sName, sFilename);
  FileHelpers::GetKeyStringFromINIFile("Properties", "Image", "default", m_sImageFilename, sFilename);
  FileHelpers::GetKeyStringFromINIFile("Properties", "SteeringAid", "On", sValue, sFilename);
  //if(sValue.CompareNoCase("Off") == 0) {
  if(sValue.compare("Off") == 0) {
    BGame::GetSimulation()->m_bSteeringAidOn = false;
  } else {
    BGame::GetSimulation()->m_bSteeringAidOn = true;
  }
  FileHelpers::GetKeyVectorFromINIFile("Properties", "FuelLocation", BVector(0, 0, 0), m_vFuelLocation, sFilename);
  FileHelpers::GetKeyDoubleFromINIFile("Properties", "FuelDistance", 2.0, m_dFuelDistance, sFilename);

  FileHelpers::GetKeyStringFromINIFile("Geometry", "RightDir", "+X", m_OBJData.m_sRightDir, sFilename);
  FileHelpers::GetKeyStringFromINIFile("Geometry", "ForwardDir", "+Y", m_OBJData.m_sForwardDir, sFilename);
  FileHelpers::GetKeyStringFromINIFile("Geometry", "DownDir", "+Z", m_OBJData.m_sDownDir, sFilename);
  FileHelpers::GetKeyDoubleFromINIFile("Geometry", "Scale", 1.0, m_OBJData.m_dScale, sFilename);

  FileHelpers::GetKeyDoubleFromINIFile("Body", "TotalMass", 1000.0, m_dTotalMass, sFilename);
  FileHelpers::GetKeyDoubleFromINIFile("Engine", "HorsePowers", 100.0, m_dHorsePowers, sFilename);
  m_dHorsePowers /= 200.0;

  //* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
  // Check existence of optional sections
  string sHasSection;
  FileHelpers::GetKeyStringFromINIFile("Rotor", "", "default", sHasSection, sFilename);
  //m_bHasRotor = sHasSection.CompareNoCase("default") != 0;
  m_bHasRotor = sHasSection.compare("default") != 0;
  FileHelpers::GetKeyStringFromINIFile("Jet", "", "default", sHasSection, sFilename);
  //m_bHasJet = sHasSection.CompareNoCase("default") != 0;
  m_bHasJet = sHasSection.compare("default") != 0;
  FileHelpers::GetKeyStringFromINIFile("Airplane", "", "default", sHasSection, sFilename);
  //m_bHasAirplaneControls = sHasSection.CompareNoCase("default") != 0;
  m_bHasAirplaneControls = sHasSection.compare("default") != 0;

  //* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
  if(m_bHasRotor) {
    // Load Rotor data
    FileHelpers::GetKeyVectorFromINIFile("Rotor", "ExitPoint", BVector(0, 0, 0), m_rotor.m_vExitPoint, sFilename);
    FileHelpers::GetKeyVectorFromINIFile("Rotor", "ExitDir", BVector(0, -1, 0), m_rotor.m_vExitDir, sFilename);
    m_rotor.m_vExitDir.ToUnitLength();
    FileHelpers::GetKeyDoubleFromINIFile("Rotor", "BladeLength", 4.0, m_rotor.m_dBladeLength, sFilename);
    FileHelpers::GetKeyDoubleFromINIFile("Rotor", "BladeTipWidth", 4.0, m_rotor.m_dBladeTipWidth, sFilename);

    string sBodyPoint;
    FileHelpers::GetKeyStringFromINIFile("Rotor", "BodyPoint1", "1, 0", sBodyPoint, sFilename);
    //sscanf(LPCTSTR(sBodyPoint), "%d, %lf", &(m_rotor.m_nBodyPoints[0]), &(m_rotor.m_dBodyEffects[0]));
    sscanf(sBodyPoint.c_str(), "%d, %lf", &(m_rotor.m_nBodyPoints[0]), &(m_rotor.m_dBodyEffects[0]));
    FileHelpers::GetKeyStringFromINIFile("Rotor", "BodyPoint2", "1, 0", sBodyPoint, sFilename);
    //sscanf(LPCTSTR(sBodyPoint), "%d, %lf", &(m_rotor.m_nBodyPoints[1]), &(m_rotor.m_dBodyEffects[1]));
    sscanf(sBodyPoint.c_str(), "%d, %lf", &(m_rotor.m_nBodyPoints[1]), &(m_rotor.m_dBodyEffects[1]));
    FileHelpers::GetKeyStringFromINIFile("Rotor", "BodyPoint3", "1, 0", sBodyPoint, sFilename);
    //sscanf(LPCTSTR(sBodyPoint), "%d, %lf", &(m_rotor.m_nBodyPoints[2]), &(m_rotor.m_dBodyEffects[2]));
    sscanf(sBodyPoint.c_str(), "%d, %lf", &(m_rotor.m_nBodyPoints[2]), &(m_rotor.m_dBodyEffects[2]));
    FileHelpers::GetKeyStringFromINIFile("Rotor", "BodyPoint4", "1, 0", sBodyPoint, sFilename);
    //sscanf(LPCTSTR(sBodyPoint), "%d, %lf", &(m_rotor.m_nBodyPoints[3]), &(m_rotor.m_dBodyEffects[3]));
    sscanf(sBodyPoint.c_str(), "%d, %lf", &(m_rotor.m_nBodyPoints[3]), &(m_rotor.m_dBodyEffects[3]));
    --(m_rotor.m_nBodyPoints[0]);
    --(m_rotor.m_nBodyPoints[1]);
    --(m_rotor.m_nBodyPoints[2]);
    --(m_rotor.m_nBodyPoints[3]);

    FileHelpers::GetKeyDoubleFromINIFile("Rotor", "Effect", 1.0, m_rotor.m_dEffect, sFilename);
  }

  //* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
  if(m_bHasJet) {
    // Load Jet
    FileHelpers::GetKeyVectorFromINIFile("Jet", "ExitPoint", BVector(0, 0, 0), m_jet.m_vExitPoint, sFilename);
    FileHelpers::GetKeyVectorFromINIFile("Jet", "ExitDir", BVector(0, -1, 0), m_jet.m_vExitDir, sFilename);
    m_jet.m_vExitDir.ToUnitLength();
    FileHelpers::GetKeyDoubleFromINIFile("Jet", "ExitDiameter", 0.5, m_jet.m_dExitDiameter, sFilename);

    string sBodyPoint;
    FileHelpers::GetKeyStringFromINIFile("Jet", "BodyPoint1", "1, 0", sBodyPoint, sFilename);
    //sscanf(LPCTSTR(sBodyPoint), "%d, %lf", &(m_jet.m_nBodyPoints[0]), &(m_jet.m_dBodyEffects[0]));
    sscanf(sBodyPoint.c_str(), "%d, %lf", &(m_jet.m_nBodyPoints[0]), &(m_jet.m_dBodyEffects[0]));
    FileHelpers::GetKeyStringFromINIFile("Jet", "BodyPoint2", "1, 0", sBodyPoint, sFilename);
    //sscanf(LPCTSTR(sBodyPoint), "%d, %lf", &(m_jet.m_nBodyPoints[1]), &(m_jet.m_dBodyEffects[1]));
    sscanf(sBodyPoint.c_str(), "%d, %lf", &(m_jet.m_nBodyPoints[1]), &(m_jet.m_dBodyEffects[1]));
    FileHelpers::GetKeyStringFromINIFile("Jet", "BodyPoint3", "1, 0", sBodyPoint, sFilename);
    //sscanf(LPCTSTR(sBodyPoint), "%d, %lf", &(m_jet.m_nBodyPoints[2]), &(m_jet.m_dBodyEffects[2]));
    sscanf(sBodyPoint.c_str(), "%d, %lf", &(m_jet.m_nBodyPoints[2]), &(m_jet.m_dBodyEffects[2]));
    FileHelpers::GetKeyStringFromINIFile("Jet", "BodyPoint4", "1, 0", sBodyPoint, sFilename);
    //sscanf(LPCTSTR(sBodyPoint), "%d, %lf", &(m_jet.m_nBodyPoints[3]), &(m_jet.m_dBodyEffects[3]));
    sscanf(sBodyPoint.c_str(), "%d, %lf", &(m_jet.m_nBodyPoints[3]), &(m_jet.m_dBodyEffects[3]));
    --(m_jet.m_nBodyPoints[0]);
    --(m_jet.m_nBodyPoints[1]);
    --(m_jet.m_nBodyPoints[2]);
    --(m_jet.m_nBodyPoints[3]);

    FileHelpers::GetKeyDoubleFromINIFile("Rotor", "Effect", 1.0, m_jet.m_dEffect, sFilename);
  }

  //* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
  if(m_bHasAirplaneControls) {
    // Load Airplane
    //  [Airplane]

    //  Rudder = <baseIndex>, <maxEffectDir>, <effectDir>, <pivotPoint>, <rotationAxis>, <throw, degrees>, <effect>   // See NOTE 7)
    //  Elevator = <baseIndex>, <maxEffectDir>, <effectDir>, <pivotPoint>, <rotationAxis>, <throw, degrees>, <effect> // See NOTE 7)
    //  Ailerons = <pnt1>, <pnt2>, <maxEffectDir>, <effectDir>, <effect>                                              // See NOTE 7)
    //  Propeller = <basePoint>, <forwardDir>, <position>, <effect>                                                   // See NOTE 7)

    string sAirplane;

    FileHelpers::GetKeyStringFromINIFile("Airplane", "Rudder", "1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, -1, 40, 1.0", sAirplane, sFilename);
    //sscanf(LPCTSTR(sAirplane), "%d, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf", 
    sscanf(sAirplane.c_str(), "%d, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf", 
           &(m_airplane.m_nRudderBodyPoint),
           &(m_airplane.m_vRudderMaxEffect.m_dX),
           &(m_airplane.m_vRudderMaxEffect.m_dY),
           &(m_airplane.m_vRudderMaxEffect.m_dZ),
           &(m_airplane.m_vRudderEffect.m_dX),
           &(m_airplane.m_vRudderEffect.m_dY),
           &(m_airplane.m_vRudderEffect.m_dZ),
           &(m_airplane.m_vRudderPivot.m_dX),
           &(m_airplane.m_vRudderPivot.m_dY),
           &(m_airplane.m_vRudderPivot.m_dZ),
           &(m_airplane.m_vRudderRotAxis.m_dX),
           &(m_airplane.m_vRudderRotAxis.m_dY),
           &(m_airplane.m_vRudderRotAxis.m_dZ),
           &(m_airplane.m_dRudderThrow),
           &(m_airplane.m_dRudderEffect));
    m_airplane.m_vRudderMaxEffect.ToUnitLength();
    m_airplane.m_vRudderEffect.ToUnitLength();
    m_airplane.m_vRudderRotAxis.ToUnitLength();
    --(m_airplane.m_nRudderBodyPoint);

    FileHelpers::GetKeyStringFromINIFile("Airplane", "Elevator", "1, 0, 1, 0, 0, 0, -1, 0, 0, 0, 0, 0, -1, 40, 1", sAirplane, sFilename);
    //sscanf(LPCTSTR(sAirplane), "%d, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf", 
    sscanf(sAirplane.c_str(), "%d, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf", 
           &(m_airplane.m_nElevBodyPoint),
           &(m_airplane.m_vElevMaxEffect.m_dX),
           &(m_airplane.m_vElevMaxEffect.m_dY),
           &(m_airplane.m_vElevMaxEffect.m_dZ),
           &(m_airplane.m_vElevEffect.m_dX),
           &(m_airplane.m_vElevEffect.m_dY),
           &(m_airplane.m_vElevEffect.m_dZ),
           &(m_airplane.m_vElevPivot.m_dX),
           &(m_airplane.m_vElevPivot.m_dY),
           &(m_airplane.m_vElevPivot.m_dZ),
           &(m_airplane.m_vElevRotAxis.m_dX),
           &(m_airplane.m_vElevRotAxis.m_dY),
           &(m_airplane.m_vElevRotAxis.m_dZ),
           &(m_airplane.m_dElevThrow),
           &(m_airplane.m_dElevEffect),
           &(m_airplane.m_dElevTrim));
    m_airplane.m_vElevMaxEffect.ToUnitLength();
    m_airplane.m_vElevEffect.ToUnitLength();
    m_airplane.m_vElevRotAxis.ToUnitLength();
    --(m_airplane.m_nElevBodyPoint);

    FileHelpers::GetKeyStringFromINIFile("Airplane", "Ailerons", "1, 2, 0, 1, 0, 0, 0, -1, 1", sAirplane, sFilename);
    //sscanf(LPCTSTR(sAirplane), "%d, %d, %lf, %lf, %lf, %lf, %lf, %lf, %lf", 
    sscanf(sAirplane.c_str(), "%d, %d, %lf, %lf, %lf, %lf, %lf, %lf, %lf", 
           &(m_airplane.m_nAilrnsBodyPoint1),
           &(m_airplane.m_nAilrnsBodyPoint2),
           &(m_airplane.m_vAilrnsMaxEffect.m_dX),
           &(m_airplane.m_vAilrnsMaxEffect.m_dY),
           &(m_airplane.m_vAilrnsMaxEffect.m_dZ),
           &(m_airplane.m_vAilrnsEffect.m_dX),
           &(m_airplane.m_vAilrnsEffect.m_dY),
           &(m_airplane.m_vAilrnsEffect.m_dZ),
           &(m_airplane.m_dAilrnsEffect));
    m_airplane.m_vAilrnsMaxEffect.ToUnitLength();
    m_airplane.m_vAilrnsEffect.ToUnitLength();
    --(m_airplane.m_nAilrnsBodyPoint1);
    --(m_airplane.m_nAilrnsBodyPoint2);

    FileHelpers::GetKeyStringFromINIFile("Airplane", "Propeller", "1, 0, 1, 0, 0, 0, 0, 1", sAirplane, sFilename);
    //sscanf(LPCTSTR(sAirplane), "%d, %lf, %lf, %lf, %lf, %lf, %lf, %lf", 
    sscanf(sAirplane.c_str(), "%d, %lf, %lf, %lf, %lf, %lf, %lf, %lf", 
           &(m_airplane.m_nPropBodyPoint),
           &(m_airplane.m_vPropDir.m_dX),
           &(m_airplane.m_vPropDir.m_dY),
           &(m_airplane.m_vPropDir.m_dZ),
           &(m_airplane.m_vPropPos.m_dX),
           &(m_airplane.m_vPropPos.m_dY),
           &(m_airplane.m_vPropPos.m_dZ),
           &(m_airplane.m_dPropEffect));
    m_airplane.m_vPropDir.ToUnitLength();
    --(m_airplane.m_nPropBodyPoint);
    FileHelpers::GetKeyStringFromINIFile("Airplane", "UseRudderOnTurns", "default", sAirplane, sFilename);
    //if(sAirplane.CompareNoCase("default") == 0) {
    if(sAirplane.compare("default") == 0) {
      m_airplane.m_dUseRudderForTurn = 0.0;
    } else {
      //sscanf(LPCTSTR(sAirplane), "%lf", &(m_airplane.m_dUseRudderForTurn));
      sscanf(sAirplane.c_str(), "%lf", &(m_airplane.m_dUseRudderForTurn));
    }
    FileHelpers::GetKeyStringFromINIFile("Airplane", "Trailpoints", "-1", sAirplane, sFilename);
    //sscanf(LPCTSTR(sAirplane), "%d, %d, %d, %d, %d", 
    sscanf(sAirplane.c_str(), "%d, %d, %d, %d, %d", 
           &(m_nTrailpoint[0]),
           &(m_nTrailpoint[1]),
           &(m_nTrailpoint[2]),
           &(m_nTrailpoint[3]),
           &(m_nTrailpoint[4]));
    --(m_nTrailpoint[0]);
    --(m_nTrailpoint[1]);
    --(m_nTrailpoint[2]);
    --(m_nTrailpoint[3]);
    --(m_nTrailpoint[4]);
    for(i = 0; m_nTrailpoint[i] >= 0; ++i)
      ;
    m_nTrailpoints = i;
  }


  //* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
  // Load physics body points
  if(m_pBodyPoint) {
    delete [] m_pBodyPoint;
  }
  m_pBodyPoint = new BBodyPoint[100];
  m_nBodyPoints = 0;
  for(i = 0; i < 100; ++i) {
    // Try to read in the body point
    //sKey.Format("BodyPoint%d", i + 1);
    stringstream format;
    format << "BodyPoint" << i + 1;
    sKey = format.str();
    FileHelpers::GetKeyStringFromINIFile("Body", sKey, "default", sValue, sFilename);
    //if(sValue.CompareNoCase("default") == 0) {
    if(sValue.compare("default") == 0) {
      // Break out, there are no body points
      break;
    }
    //sscanf(LPCTSTR(sValue), 
    sscanf(sValue.c_str(), 
           "%lf, %lf, %lf, %lf, %lf",
           &(m_pBodyPoint[i].m_vLocation.m_dX),
           &(m_pBodyPoint[i].m_vLocation.m_dY),
           &(m_pBodyPoint[i].m_vLocation.m_dZ),
           &(m_pBodyPoint[i].m_dMass),
           &(m_pBodyPoint[i].m_dFriction));
  }
  m_nBodyPoints = i;

  // Found out new origin (=average of body points)
  BVector vMassCenter = BVector(0, 0, 0);
  for(i = 0; i < m_nBodyPoints; ++i) {
    vMassCenter = vMassCenter + m_pBodyPoint[i].m_vLocation;
  }
  vMassCenter = vMassCenter * (1.0 / double(m_nBodyPoints));

  for(i = 0; i < m_nBodyPoints; ++i) {
    m_pBodyPoint[i].m_vLocation += vMassCenter * -1.0;
  }
  m_OBJData.m_vMassCenter = vMassCenter;


  //* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
  // Load direction points
  FileHelpers::GetKeyStringFromINIFile("Body", "ForwardPoints", "default", sValue, sFilename);
  //if(sValue.CompareNoCase("default") != 0) {
  if(sValue.compare("default") != 0) {
    //if(sscanf(LPCTSTR(sValue), 
    if(sscanf(sValue.c_str(), 
              "%d, %d, %d, %d", 
              &(m_nForwardPoints[0]),
              &(m_nForwardPoints[1]),
              &(m_nForwardPoints[2]),
              &(m_nForwardPoints[3])) != 4) {
      m_nForwardPoints[2] = m_nForwardPoints[0];
      m_nForwardPoints[3] = m_nForwardPoints[1];
    }
    --(m_nForwardPoints[0]);
    --(m_nForwardPoints[1]);
    --(m_nForwardPoints[2]);
    --(m_nForwardPoints[3]);
  }

  FileHelpers::GetKeyStringFromINIFile("Body", "RightPoints", "default", sValue, sFilename);
  //if(sValue.CompareNoCase("default") != 0) {
  if(sValue.compare("default") != 0) {
    //if(sscanf(LPCTSTR(sValue), 
    if(sscanf(sValue.c_str(), 
              "%d, %d, %d, %d", 
              &(m_nRightPoints[0]),
              &(m_nRightPoints[1]),
              &(m_nRightPoints[2]),
              &(m_nRightPoints[3])) != 4) {
      m_nRightPoints[2] = m_nRightPoints[0];
      m_nRightPoints[3] = m_nRightPoints[1];
    }
    --(m_nRightPoints[0]);
    --(m_nRightPoints[1]);
    --(m_nRightPoints[2]);
    --(m_nRightPoints[3]);
  }

  //* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
  // Load lift points
  FileHelpers::GetKeyIntFromINIFile("Body", "LiftPoint1", 1, m_nLiftPoint1, sFilename);
  --m_nLiftPoint1;
  FileHelpers::GetKeyIntFromINIFile("Body", "LiftPoint2", 1, m_nLiftPoint2, sFilename);
  --m_nLiftPoint2;


  //* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
  // Load in the Parts
  // First count how many there are
  m_nParts = 0;
  do {
    string sHasSection;
    string sSection;
    //sSection.Format("Part%d", m_nParts + 1);
    stringstream format;
    format << "Part" << m_nParts + 1;
    sSection = format.str();
    FileHelpers::GetKeyStringFromINIFile(sSection, "", "default", sHasSection, sFilename);
    //if(sHasSection.CompareNoCase("default") != 0) {
    if(sHasSection.compare("default") != 0) {
      ++m_nParts;
    } else {
      break;
    }
  } while(m_nParts < 10000); // just a sanity check to break the loop eventually

  if(m_pPart) {
    delete [] m_pPart;
  }
  m_pPart = new BPart[m_nParts];

  // Read parts
  for(int nPart = 0; nPart < m_nParts; ++nPart) {
    string sSection;
    //sSection.Format("Part%d", nPart + 1);
    stringstream format;
    format << "Part" << nPart + 1;
    sSection = format.str();
    m_pPart[nPart].SetOBJData(&m_OBJData);
    m_pPart[nPart].LoadPartFromFile(sFilename, sSection, false, true);
  }

  m_partShadow.SetOBJData(&m_OBJData);
  m_partShadow.LoadPartFromFile(sFilename, "PartShadow", true, true);
  if(m_bHasAirplaneControls) {
    BVector vCenterSafe = m_OBJData.m_vMassCenter;
    m_OBJData.m_vMassCenter.Set(0, 0, 0);
    m_partRudder.SetOBJData(&m_OBJData);
    m_partElevator.SetOBJData(&m_OBJData);
    m_partPropeller.SetOBJData(&m_OBJData);
    m_partRudder.LoadPartFromFile(sFilename, "PartRudder", false, true);
    m_partElevator.LoadPartFromFile(sFilename, "PartElevator", false, true);
    m_partPropeller.LoadPartFromFile(sFilename, "PartPropeller", false, true);
    m_OBJData.m_vMassCenter = vCenterSafe;
  }

  //* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
  // Load aerodynamics
  if(m_pAeroPoint) {
    delete [] m_pAeroPoint;
  }
  m_pAeroPoint = new BAeroPoint[m_nBodyPoints];

  m_bAerodynamics = false;

  for(i = 0; i < m_nBodyPoints; ++i) {
    //sKey.Format("BodyPoint%d", i + 1);
    stringstream format;
    format << "BodyPoint" << i + 1;
    sKey = format.str();
    FileHelpers::GetKeyStringFromINIFile("Aerodynamics", sKey, "default", sValue, sFilename);
    //if(sValue.CompareNoCase("default") == 0) {
    if(sValue.compare("default") == 0) {
      // Not all aerodynamic points were defined, ignore aerodynamics
      m_bAerodynamics = false;
      delete [] m_pAeroPoint;
      m_pAeroPoint = 0;
      break;
    }
    m_bAerodynamics = true;
    //sscanf(LPCTSTR(sValue), 
    sscanf(sValue.c_str(), 
           "%lf, %lf, %lf, %lf, %lf, %lf", 
           &(m_pAeroPoint[i].m_vMaxDragDir.m_dX),
           &(m_pAeroPoint[i].m_vMaxDragDir.m_dY),
           &(m_pAeroPoint[i].m_vMaxDragDir.m_dZ),
           &(m_pAeroPoint[i].m_dMaxDrag),
           &(m_pAeroPoint[i].m_dRelMinDrag),
           &(m_pAeroPoint[i].m_dLift));
    m_pAeroPoint[i].m_vMaxDragDir.ToUnitLength();
  }

  m_nWheels = 0;
  for(i = 0; i < 10; ++i) {
    m_pWheel[i] = new BWheel;
  }
  for(i = 0; i < 10; ++i) {
    if(!LoadWheelFromFile(sFilename, i + 1, dynamic_cast<BWheel*>(m_pWheel[i]))) {
      break;
    }
  }
  m_nWheels = i;

  PrepareVehicle();
}


//**********************************************************************

void BVehicle::PrepareVehicle() {
  PPEC_Vehicle::Init();

  // Calculate normals for the parts
  m_OBJData.PrepareWaveFrontModel();

  // Calculate visual width and length for default shadowing
  double dMinX = 99999.9, dMaxX = -99999.9, dMinY = 99999.9, dMaxY = -99999.9;
  for(int i = 0; i < m_nParts; ++i) {
    for(int j = 0; j < m_pPart[i].m_nVertices; ++j) {
      BVector vTmp = m_OBJData.m_pvOBJPoints[m_pPart[i].m_nVertexStart + j];
      if(vTmp.m_dX < dMinX) {
        dMinX = vTmp.m_dX;
      }
      if(vTmp.m_dX > dMaxX) {
        dMaxX = vTmp.m_dX;
      }
      if(vTmp.m_dY < dMinY) {
        dMinY= vTmp.m_dY;
      }
      if(vTmp.m_dY> dMaxY) {
        dMaxY = vTmp.m_dY;
      }
    }
  }
  m_dVisualWidth = dMaxX - dMinX;
  m_dVisualLength = dMaxY - dMinY;
}


//**********************************************************************

bool BVehicle::LoadWheelFromFile(string sFilename, 
                                 int nWheel, 
                                 BWheel *pWheel) {
  double dTmp;
  string sKey, sValue;

  //sKey.Format("Wheel%dTurn", nWheel);
    stringstream format;
    format << "Wheel" << nWheel << "Turn";
    sKey = format.str();
  FileHelpers::GetKeyStringFromINIFile("Wheels", sKey, "default", sValue, sFilename);
  //if(sValue.CompareNoCase("default") == 0) {
  if(sValue.compare("default") == 0) {
    return false;
  }

  // Turns
  //sKey.Format("Wheel%dTurn", nWheel);
    format.str("");
    format << "Wheel" << nWheel << "Turn";
    sKey = format.str();
  FileHelpers::GetKeyStringFromINIFile("Wheels", sKey, "Fixed", sValue, sFilename);
  if(sValue[0] == 'F') {
    pWheel->m_bTurns = false;
  } else {
    pWheel->m_bTurns = true;
    int nAfterComma = 0;
    while(sValue[nAfterComma] && (sValue[nAfterComma] != ',')) {
      ++nAfterComma;
    }
    if(sValue[nAfterComma] == ',') {
      ++nAfterComma;
      //sscanf(LPCTSTR(sValue) + nAfterComma, 
      sscanf(sValue.c_str() + nAfterComma, 
             "%lf, %lf", 
             &(pWheel->m_dThrow),
             &dTmp);
      if(dTmp < 0.0) {
        pWheel->m_dThrow *= -1.0;
      }
    }
  }

  //sKey.Format("Wheel%dSide", nWheel);
    format.str("");
    format << "Wheel" << nWheel << "Side";
    sKey = format.str();
  FileHelpers::GetKeyStringFromINIFile("Wheels", sKey, "Right", sValue, sFilename);
  pWheel->m_bLeft = (sValue[0] == 'l') || (sValue[0] == 'L');

  //sKey.Format("Wheel%dDrive", nWheel);
    format.str("");
    format << "Wheel" << nWheel << "Drive";
    sKey = format.str();
  FileHelpers::GetKeyDoubleFromINIFile("Wheels", sKey, 1.0, pWheel->m_dDriveFactor, sFilename);

  //sKey.Format("Wheel%dBrake", nWheel);
    format.str("");
    format << "Wheel" << nWheel << "Brake";
    sKey = format.str();
  FileHelpers::GetKeyDoubleFromINIFile("Wheels", sKey, 1.0, pWheel->m_dBrakeFactor, sFilename);

  //sKey.Format("Wheel%dFriction", nWheel);
    format.str("");
    format << "Wheel" << nWheel << "Friction";
    sKey = format.str();
  FileHelpers::GetKeyDoubleFromINIFile("Wheels", sKey, 0.9, pWheel->m_dFriction, sFilename);

  //sKey.Format("Wheel%dRadius", nWheel);
    format.str("");
    format << "Wheel" << nWheel << "Radius";
    sKey = format.str();
  FileHelpers::GetKeyDoubleFromINIFile("Wheels", sKey, 0.4, pWheel->m_dRadius, sFilename);

  //sKey.Format("Wheel%dProfileHeight", nWheel);
    format.str("");
    format << "Wheel" << nWheel << "ProfileHeight";
    sKey = format.str();
  FileHelpers::GetKeyDoubleFromINIFile("Wheels", sKey, 0.1, pWheel->m_dProfileHeight, sFilename);

  //sKey.Format("Wheel%dWidth", nWheel);
    format.str("");
    format << "Wheel" << nWheel << "Width";
    sKey = format.str();
  FileHelpers::GetKeyDoubleFromINIFile("Wheels", sKey, 0.1, pWheel->m_dWidth, sFilename);

  //sKey.Format("Wheel%dStyle", nWheel);
    format.str("");
    format << "Wheel" << nWheel << "Style";
    sKey = format.str();
  FileHelpers::GetKeyStringFromINIFile("Wheels", sKey, "ChromeDome", sValue, sFilename);
  //if(sValue.CompareNoCase("ChromeDome") == 0) {
  if(sValue.compare("ChromeDome") == 0) {
    //pWheel->m_style = BWheel::TWheelStyle::CHROMEDOME;
    pWheel->m_style = BWheel::CHROMEDOME;
  //} else if(sValue.CompareNoCase("OffRoad") == 0) {
  } else if(sValue.compare("OffRoad") == 0) {
    //pWheel->m_style = BWheel::TWheelStyle::OFFROAD;
    pWheel->m_style = BWheel::OFFROAD;
  //} else if(sValue.CompareNoCase("ClassicAirplane") == 0) {
  } else if(sValue.compare("ClassicAirplane") == 0) {
    //pWheel->m_style = BWheel::TWheelStyle::CLASSICAIRPLANE;
    pWheel->m_style = BWheel::CLASSICAIRPLANE;
  } else {
    //pWheel->m_style = BWheel::TWheelStyle::CHROMEDOME;
    pWheel->m_style = BWheel::CHROMEDOME;
  }

  //sKey.Format("Wheel%dSuspBodyPoint", nWheel);
    format.str("");
    format << "Wheel" << nWheel << "SuspBodyPoint";
    sKey = format.str();
  FileHelpers::GetKeyIntFromINIFile("Wheels", sKey, 1, pWheel->m_nBodyPoint, sFilename);
  --(pWheel->m_nBodyPoint);

  //sKey.Format("Wheel%dSuspBasePoint", nWheel);
    format.str("");
    format << "Wheel" << nWheel << "SuspBasePoint";
    sKey = format.str();
  FileHelpers::GetKeyVectorFromINIFile("Wheels", sKey, BVector(0, 0, 0), pWheel->m_vSuspBasePoint, sFilename);
  pWheel->m_vSuspBasePoint -= m_OBJData.m_vMassCenter;
  
  //sKey.Format("Wheel%dSuspDir", nWheel);
    format.str("");
    format << "Wheel" << nWheel << "SuspDir";
    sKey = format.str();
  FileHelpers::GetKeyVectorFromINIFile("Wheels", sKey, BVector(0, 0, 0), pWheel->m_vSuspDir, sFilename);
  pWheel->m_vSuspDir.ToUnitLength();

  //sKey.Format("Wheel%dSuspRelaxedDistance", nWheel);
    format.str("");
    format << "Wheel" << nWheel << "SuspRelaxedDistance";
    sKey = format.str();
  FileHelpers::GetKeyDoubleFromINIFile("Wheels", sKey, 0.3, pWheel->m_dSuspRelaxedDistance, sFilename);

  //sKey.Format("Wheel%dSuspThrow", nWheel);
    format.str("");
    format << "Wheel" << nWheel << "SuspThrow";
    sKey = format.str();
  FileHelpers::GetKeyDoubleFromINIFile("Wheels", sKey, 0.3, pWheel->m_dMaxSuspThrow, sFilename);

  pWheel->m_dMaxSusp = pWheel->m_dMaxSuspThrow;
  pWheel->m_dMinSusp = 0.0;
  //sKey.Format("Wheel%dMinSusp", nWheel);
    format.str("");
    format << "Wheel" << nWheel << "MinSusp";
    sKey = format.str();
  FileHelpers::GetKeyDoubleFromINIFile("Wheels", sKey, 0.0, pWheel->m_dMinSusp, sFilename);

  //sKey.Format("Wheel%dMaxSusp", nWheel);
    format.str("");
    format << "Wheel" << nWheel << "MaxSusp";
    sKey = format.str();
  FileHelpers::GetKeyDoubleFromINIFile("Wheels", sKey, pWheel->m_dMaxSuspThrow, pWheel->m_dMaxSusp, sFilename);

  //sKey.Format("Wheel%dSuspStiffness", nWheel);
    format.str("");
    format << "Wheel" << nWheel << "SuspStiffness";
    sKey = format.str();
  FileHelpers::GetKeyDoubleFromINIFile("Wheels", sKey, 0.35, pWheel->m_dSuspStiffness, sFilename);
  if(pWheel->m_dSuspStiffness <= 0.0001) {
    pWheel->m_dSuspStiffness = 0.0001;
  }

  pWheel->m_bBroken = false;
  pWheel->m_dSuspension = 0.0;
  pWheel->SetPoints(m_pBodyPoint);
  pWheel->m_vLocationOrig = pWheel->m_vSuspBasePoint + pWheel->m_vSuspDir * pWheel->m_dSuspRelaxedDistance;

  return true;
}








//**********************************************************************
void BVehicle::Move(BVector& vRelMove) {
  for(int i = 0; i < m_nBodyPoints; ++i) {
    m_pBodyPoint[i].m_vLocation = m_pBodyPoint[i].m_vLocation + vRelMove;
  }
}


//**********************************************************************
void BVehicle::BodyTouchesMatterAt(BVector vCollLoc) {
  BGame::GetSimulation()->CreateDustCloudAt(vCollLoc, 1);
}

//**********************************************************************
void BVehicle::WheelTouchesMatterAt(BVector vLoc, double dRadius) {
  BGame::GetSimulation()->CreateDustCloudAt(vLoc, 2, -1.0, BVector(0, 0, 0), dRadius);
}

//**********************************************************************
double BVehicle::GroundHardnessAt(BVector vLoc) {
  return 0.5; // NOT READY Use correct ground hardness!
}


//**********************************************************************

void BVehicle::PreProcessVisualization() {
  BVector vTmp[44];
  // Create a display list of the Vehicle

  if(m_nDLVehicleBody == -1) {
    m_nDLVehicleBody = glGenLists(1);
  }

  glNewList(m_nDLVehicleBody, GL_COMPILE);
  // Visualize OBJ model
  if(m_nParts) {
    int nPart;
    m_pPart[0].RenderPart(1);
    for(nPart = 0; nPart < m_nParts; ++nPart) {
      m_pPart[nPart].SetPartColor();
      m_pPart[nPart].RenderPart(0);
    }
    m_pPart[0].RenderPart(2);
  }
  glEndList();

  if(m_bHasAirplaneControls) {
    if(m_nDLElevator == -1) {
      m_nDLElevator = glGenLists(1);
    }
    glNewList(m_nDLElevator, GL_COMPILE);
    m_partElevator.SetPartColor();

    m_partElevator.RenderPart(0);

    glEndList();

    if(m_nDLRudder == -1) {
      m_nDLRudder = glGenLists(1);
    }
    glNewList(m_nDLRudder, GL_COMPILE);
    m_partRudder.SetPartColor();

    m_partRudder.RenderPart(0);

    glEndList();

    if(m_nDLPropeller == -1) {
      m_nDLPropeller = glGenLists(1);
    }

    glNewList(m_nDLPropeller, GL_COMPILE);
    m_partPropeller.RenderPart(1);
    m_partPropeller.RenderPart(0);
    m_partPropeller.RenderPart(2);
    glEndList();

  }
  
  PreProcessWheels();
}

//**********************************************************************
void BVehicle::PreProcessWheels() {
  BVector vTmp[44];

  //**********************
  //** Wheel
  //**********************

  for(int nWheel = 0; nWheel < m_nWheels; ++nWheel) {
    BWheel *pWheel = dynamic_cast<BWheel *>(m_pWheel[nWheel]);

    // Render wheel (first try to reuse a display list from the previous wheels)
    bool bReused = false;
    for(int i = 0; i < nWheel; ++i) {
      BWheel *pWheel2 = dynamic_cast<BWheel *>(m_pWheel[i]);
      if((pWheel->m_style == pWheel2->m_style) &&
         (pWheel->m_dRadius == pWheel2->m_dRadius) &&
         (pWheel->m_dProfileHeight == pWheel2->m_dProfileHeight) &&
         (pWheel->m_dWidth == pWheel2->m_dWidth)) {
        pWheel->m_nDLWheel = pWheel2->m_nDLWheel;
        bReused = true;
        break;
      }
    }
    if(bReused) {
      continue;
    }

    //if(pWheel->m_style == BWheel::TWheelStyle::OFFROAD) {
    if(pWheel->m_style == BWheel::OFFROAD) {
      // Wheel blade
      pWheel->m_nDLWheelBlade = glGenLists(1);
      glNewList(pWheel->m_nDLWheelBlade, GL_COMPILE);
      RenderWheelBlade(pWheel->m_dRadius * 0.6, 
                       pWheel->m_dRadius * 0.5, 
                       pWheel->m_dRadius * 0.15, 
                       0.6, 
                       0.4, 
                       0.08, 
                       0.0,
                       nWheel);
      glEndList();
    }

    // One (right) wheel resting on the X-axis
    pWheel->m_nDLWheel = glGenLists(1);
    glNewList(pWheel->m_nDLWheel, GL_COMPILE);
    glShadeModel(GL_SMOOTH);
    // sides
    OpenGLHelpers::SwitchToTexture(0);
    OpenGLHelpers::SetColorFull(1, 1, 1, 1);
    //if(pWheel->m_style == BWheel::TWheelStyle::OFFROAD) {
    if(pWheel->m_style == BWheel::OFFROAD) {
      BTextures::Use(BTextures::LoadTexture("Textures/wheeldetailed2.tga"));
    } else {
      BTextures::Use(BTextures::WHEEL);
    }
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 128.0f);
    BVector vNormals[22];
    int fan;
    //vTmp[0] = BVector(0.1, 0, 0);
    //vNormals[0] = BVector(1, 0, 0);
    glBegin(GL_TRIANGLE_STRIP);
    //glNormal3f(vNormals[0].m_dX, vNormals[0].m_dY, vNormals[0].m_dZ); 
    //glTexCoord2f(0.5, 0.5);
    //glVertex3f(vTmp[0].m_dX, vTmp[0].m_dY, vTmp[0].m_dZ);

    double dWidth05 = pWheel->m_dWidth / 2.0;
    double dWidth05Bevel = dWidth05 * 0.8;
    double dRadius = pWheel->m_dRadius;

    for(fan = 0; fan < 21; ++fan) {
      vTmp[fan] = BVector(dWidth05, 0, 0) +
                  (BVector(0, 1, 0) * dRadius * 0.9 *
                   cos((double(fan) / 20.0) * 2.0 * g_cdPI)) + 
                  (BVector(0, 0, -1) * dRadius * 0.9 *
                   sin((double(fan) / 20.0) * 2.0 * g_cdPI));
      vNormals[fan] = BVector(1, 0, 0) + vTmp[fan];
      vNormals[fan].ToUnitLength();

      glNormal3f(vNormals[fan].m_dX, vNormals[fan].m_dY, vNormals[fan].m_dZ);
      glTexCoord2f( vTmp[fan].m_dY / (dRadius * 1.8) + 0.5, 
                   -vTmp[fan].m_dZ / (dRadius * 1.8) + 0.5);
      glVertex3f(vTmp[fan].m_dX, vTmp[fan].m_dY, vTmp[fan].m_dZ);
      vTmp[fan] = BVector(dWidth05, 0, 0) +
                  (BVector(0, 1, 0) * dRadius * 0.65 *
                   cos((double(fan) / 20.0) * 2.0 * g_cdPI)) + 
                  (BVector(0, 0, -1) * dRadius * 0.65 *
                   sin((double(fan) / 20.0) * 2.0 * g_cdPI));
      glTexCoord2f( vTmp[fan].m_dY / (dRadius * 1.8) + 0.5, 
                   -vTmp[fan].m_dZ / (dRadius * 1.8) + 0.5);
      glVertex3f(vTmp[fan].m_dX, vTmp[fan].m_dY, vTmp[fan].m_dZ);
    }
    // OpenGLHelpers::TriangleFanWithNormals(vTmp, vNormals, 22);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    OpenGLHelpers::SetColorFull(0.2, 0.2, 0.2, 1);
    glBegin(GL_TRIANGLE_STRIP);
    for(fan = 0; fan < 21; ++fan) {
      vTmp[fan] = BVector(-dWidth05, 0, 0) +
                  (BVector(0, 1, 0) * dRadius * 0.9 *
                   cos((double(fan) / 20.0) * 2.0 * g_cdPI)) + 
                  (BVector(0, 0, -1) * dRadius * 0.9 *
                   sin((double(fan) / 20.0) * 2.0 * g_cdPI));
      vNormals[fan] = BVector(-1, 0, 0) + vTmp[fan];
      vNormals[fan].ToUnitLength();

      glNormal3f(vNormals[fan].m_dX, vNormals[fan].m_dY, vNormals[fan].m_dZ);
      glVertex3f(vTmp[fan].m_dX, vTmp[fan].m_dY, vTmp[fan].m_dZ);
      vTmp[fan] = BVector(-dWidth05, 0, 0) +
                  (BVector(0, 1, 0) * dRadius * 0.65 *
                   cos((double(fan) / 20.0) * 2.0 * g_cdPI)) + 
                  (BVector(0, 0, -1) * dRadius * 0.65 *
                   sin((double(fan) / 20.0) * 2.0 * g_cdPI));
      glVertex3f(vTmp[fan].m_dX, vTmp[fan].m_dY, vTmp[fan].m_dZ);
    }
    glEnd();

    // Blades (hub cap)

    //if(pWheel->m_style != BWheel::TWheelStyle::OFFROAD) {
    if(pWheel->m_style != BWheel::OFFROAD) {
      //if(pWheel->m_style == BWheel::TWheelStyle::CHROMEDOME) {
      if(pWheel->m_style == BWheel::CHROMEDOME) {
        OpenGLHelpers::SetColorFull(1, 1, 1, 1);
        OpenGLHelpers::SwitchToTexture(0);
        BTextures::Use(BTextures::ENVMAP);
        glEnable(GL_TEXTURE_GEN_S);
	      glEnable(GL_TEXTURE_GEN_T);
        glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
	      glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);	
      } else {
        OpenGLHelpers::SetColorFull(0.8, 0.8, 0.8, 1);
        OpenGLHelpers::SwitchToTexture(0);
        glDisable(GL_TEXTURE_2D);
      }
      BVector vHubCenter(dWidth05Bevel, 0, 0);
      double  dHubRadius = dRadius * 0.6;
      BVector v, vNormal;
      //double dAngleStep = (pWheel->m_style == BWheel::TWheelStyle::CHROMEDOME) ? 15.0 : 30.0;
      double dAngleStep = (pWheel->m_style == BWheel::CHROMEDOME) ? 15.0 : 30.0;

      for(double dBigAngle = 0.0; dBigAngle < 76.0; dBigAngle += dAngleStep) {
        glBegin(GL_TRIANGLE_STRIP);
        for(double dSmallAngle = 0.0; dSmallAngle < 360.1; dSmallAngle += dAngleStep) {
          double dBigAngleRad     = dBigAngle / 180.0 * g_cdPI;
          double dBigAngleNextRad = (dBigAngle + dAngleStep) / 180.0 * g_cdPI;
          double dSmallAngleRad = dSmallAngle / 180.0 * g_cdPI;
          v = vHubCenter;
          v = v + BVector(cos(dBigAngleRad) * dHubRadius * 0.3,
                          sin(dBigAngleRad) * dHubRadius * cos(dSmallAngleRad),
                          sin(dBigAngleRad) * dHubRadius * sin(dSmallAngleRad));
          vNormal = v - vHubCenter;
          vNormal = vNormal + BVector(0.25, 0, 0);
          vNormal.ToUnitLength();
          glNormal3f(vNormal.m_dX, vNormal.m_dY, vNormal.m_dZ);
          glVertex3f(v.m_dX, v.m_dY, v.m_dZ);

          v = vHubCenter;
          v = v + BVector(cos(dBigAngleNextRad) * dHubRadius * 0.3,
                          sin(dBigAngleNextRad) * dHubRadius * cos(dSmallAngleRad),
                          sin(dBigAngleNextRad) * dHubRadius * sin(dSmallAngleRad));
          vNormal = v - vHubCenter;
          vNormal = vNormal + BVector(0.25, 0, 0);
          vNormal.ToUnitLength();
          glNormal3f(vNormal.m_dX, vNormal.m_dY, vNormal.m_dZ);
          glVertex3f(v.m_dX, v.m_dY, v.m_dZ);
        }
        glEnd();
      }

      //if(pWheel->m_style == BWheel::TWheelStyle::CLASSICAIRPLANE) {
      if(pWheel->m_style == BWheel::CLASSICAIRPLANE) {
        vHubCenter.Set(-dWidth05Bevel, 0, 0);
        for(double dBigAngle = 0.0; dBigAngle < 76.0; dBigAngle += dAngleStep) {
          glBegin(GL_TRIANGLE_STRIP);
          for(double dSmallAngle = 0.0; dSmallAngle < 360.1; dSmallAngle += dAngleStep) {
            double dBigAngleRad     = dBigAngle / 180.0 * g_cdPI;
            double dBigAngleNextRad = (dBigAngle + dAngleStep) / 180.0 * g_cdPI;
            double dSmallAngleRad = dSmallAngle / 180.0 * g_cdPI;
            v = vHubCenter;
            v = v + BVector(-cos(dBigAngleRad) * dHubRadius * 0.3,
                            sin(dBigAngleRad) * dHubRadius * cos(dSmallAngleRad),
                            sin(dBigAngleRad) * dHubRadius * sin(dSmallAngleRad));
            vNormal = v - vHubCenter;
            vNormal = vNormal + BVector(-0.25, 0, 0);
            vNormal.ToUnitLength();
            glNormal3f(vNormal.m_dX, vNormal.m_dY, vNormal.m_dZ);
            glVertex3f(v.m_dX, v.m_dY, v.m_dZ);

            v = vHubCenter;
            v = v + BVector(-cos(dBigAngleNextRad) * dHubRadius * 0.3,
                            sin(dBigAngleNextRad) * dHubRadius * cos(dSmallAngleRad),
                            sin(dBigAngleNextRad) * dHubRadius * sin(dSmallAngleRad));
            vNormal = v - vHubCenter;
            vNormal = vNormal + BVector(-0.25, 0, 0);
            vNormal.ToUnitLength();
            glNormal3f(vNormal.m_dX, vNormal.m_dY, vNormal.m_dZ);
            glVertex3f(v.m_dX, v.m_dY, v.m_dZ);
          }
          glEnd();
        }
      }

      //if(pWheel->m_style == BWheel::TWheelStyle::CHROMEDOME) {
      if(pWheel->m_style == BWheel::CHROMEDOME) {
        glDisable(GL_TEXTURE_GEN_S);
	      glDisable(GL_TEXTURE_GEN_T);
      }
    } else {
      // OpenGLHelpers::SetColorFull(1, 1, 1, 1);
      OpenGLHelpers::SwitchToTexture(0);
      glDisable(GL_TEXTURE_2D);

      glPushMatrix();
      glCallList(pWheel->m_nDLWheelBlade);
      glRotated(60.0, 1, 0, 0);
      glCallList(pWheel->m_nDLWheelBlade);
      glRotated(60.0, 1, 0, 0);
      glCallList(pWheel->m_nDLWheelBlade);
      glRotated(60.0, 1, 0, 0);
      glCallList(pWheel->m_nDLWheelBlade);
      glRotated(60.0, 1, 0, 0);
      glCallList(pWheel->m_nDLWheelBlade);
      glRotated(60.0, 1, 0, 0);
      glCallList(pWheel->m_nDLWheelBlade);
      glPopMatrix();
    }

    // Traction surface TOP
    OpenGLHelpers::SetColorFull(0.2, 0.2, 0.2, 1);
    for(fan = 0; fan < 21; ++fan) {
      vTmp[fan * 2] = BVector(-dWidth05Bevel, 0, 0) +
                      (BVector(0, 1, 0) * dRadius *
                       cos((double(fan) / 20.0) * 2.0 * g_cdPI)) + 
                      (BVector(0, 0, -1) * dRadius *
                       sin((double(fan) / 20.0) * 2.0 * g_cdPI));
      vTmp[fan * 2 + 1] = BVector(dWidth05Bevel, 0, 0) +
                          (BVector(0, 1, 0) * dRadius *
                           cos((double(fan) / 20.0) * 2.0 * g_cdPI)) + 
                          (BVector(0, 0, -1) * dRadius *
                           sin((double(fan) / 20.0) * 2.0 * g_cdPI));
      vNormals[fan] = (BVector(0, 1, 0) * cos((double(fan) / 20.0) * 2.0 * g_cdPI)) + 
                      (BVector(0, 0, -1) * sin((double(fan) / 20.0) * 2.0 * g_cdPI));
      vNormals[fan].ToUnitLength();
    }
    OpenGLHelpers::TriangleStripWithNormals(vTmp, vNormals, 42);

    for(fan = 0; fan < 21; ++fan) {
      vTmp[fan * 2] = BVector(-dWidth05, 0, 0) +
                      (BVector(0, 1, 0) * dRadius * 0.9 *
                       cos((double(fan) / 20.0) * 2.0 * g_cdPI)) + 
                      (BVector(0, 0, -1) * dRadius * 0.9 *
                       sin((double(fan) / 20.0) * 2.0 * g_cdPI));
      vTmp[fan * 2 + 1] = BVector(-dWidth05Bevel, 0, 0) +
                          (BVector(0, 1, 0) * dRadius *
                           cos((double(fan) / 20.0) * 2.0 * g_cdPI)) + 
                          (BVector(0, 0, -1) * dRadius *
                           sin((double(fan) / 20.0) * 2.0 * g_cdPI));
      vNormals[fan] = vTmp[fan * 2];
      vNormals[fan].ToUnitLength();
    }
    OpenGLHelpers::TriangleStripWithNormals(vTmp, vNormals, 42);

    for(fan = 0; fan < 21; ++fan) {
      vTmp[fan * 2] = BVector(dWidth05Bevel, 0, 0) +
                      (BVector(0, 1, 0) * dRadius *
                       cos((double(fan) / 20.0) * 2.0 * g_cdPI)) + 
                      (BVector(0, 0, -1) * dRadius *
                       sin((double(fan) / 20.0) * 2.0 * g_cdPI));
      vTmp[fan * 2 + 1] = BVector(dWidth05, 0, 0) +
                          (BVector(0, 1, 0) * dRadius * 0.9 *
                           cos((double(fan) / 20.0) * 2.0 * g_cdPI)) + 
                          (BVector(0, 0, -1) * dRadius * 0.9 *
                           sin((double(fan) / 20.0) * 2.0 * g_cdPI));
      vNormals[fan] = vTmp[fan * 2];
      vNormals[fan].ToUnitLength();
    }
    OpenGLHelpers::TriangleStripWithNormals(vTmp, vNormals, 42);

    // Traction surface INSIDE
    OpenGLHelpers::SetColorFull(0.2, 0.2, 0.2, 1);
    for(fan = 0; fan < 21; ++fan) {
      vTmp[fan * 2] = BVector(-dWidth05Bevel, 0, 0) +
                      (BVector(0, 1, 0) * dRadius * 0.6 *
                       cos((double(fan) / 20.0) * 2.0 * g_cdPI)) + 
                      (BVector(0, 0, -1) * dRadius * 0.6 *
                       sin((double(fan) / 20.0) * 2.0 * g_cdPI));
      vTmp[fan * 2 + 1] = BVector(dWidth05Bevel, 0, 0) +
                          (BVector(0, 1, 0) * dRadius * 0.6 *
                           cos((double(fan) / 20.0) * 2.0 * g_cdPI)) + 
                          (BVector(0, 0, -1) * dRadius * 0.6 *
                           sin((double(fan) / 20.0) * 2.0 * g_cdPI));
      vNormals[fan] = ((BVector(0, 1, 0) * cos((double(fan) / 20.0) * 2.0 * g_cdPI)) + 
                       (BVector(0, 0, -1) * sin((double(fan) / 20.0) * 2.0 * g_cdPI))) * 
                      -1.0;
      vNormals[fan].ToUnitLength();
    }
    OpenGLHelpers::TriangleStripWithNormals(vTmp, vNormals, 42);

    for(fan = 0; fan < 21; ++fan) {
      BVector vFoo = (BVector(0, 1, 0) * dRadius * 0.8 *
                      cos((double(fan) / 20.0) * 2.0 * g_cdPI)) + 
                     (BVector(0, 0, -1) * dRadius * 0.8 *
                      sin((double(fan) / 20.0) * 2.0 * g_cdPI));
      vTmp[fan * 2] = BVector(-dWidth05, 0, 0) +
                      (BVector(0, 1, 0) * dRadius * 0.65 *
                       cos((double(fan) / 20.0) * 2.0 * g_cdPI)) + 
                      (BVector(0, 0, -1) * dRadius * 0.65 *
                       sin((double(fan) / 20.0) * 2.0 * g_cdPI));
      vTmp[fan * 2 + 1] = BVector(-dWidth05Bevel, 0, 0) +
                          (BVector(0, 1, 0) * dRadius * 0.6 *
                           cos((double(fan) / 20.0) * 2.0 * g_cdPI)) + 
                          (BVector(0, 0, -1) * dRadius * 0.6 *
                           sin((double(fan) / 20.0) * 2.0 * g_cdPI));
      vNormals[fan] = vTmp[fan * 2] - vFoo;
      vNormals[fan].ToUnitLength();
    }
    OpenGLHelpers::TriangleStripWithNormals(vTmp, vNormals, 42);

    for(fan = 0; fan < 21; ++fan) {
      BVector vFoo = (BVector(0, 1, 0) * dRadius * 0.8 *
                      cos((double(fan) / 20.0) * 2.0 * g_cdPI)) + 
                     (BVector(0, 0, -1) * dRadius * 0.8 *
                      sin((double(fan) / 20.0) * 2.0 * g_cdPI));
      vTmp[fan * 2] = BVector(dWidth05Bevel, 0, 0) +
                      (BVector(0, 1, 0) * dRadius * 0.6 *
                       cos((double(fan) / 20.0) * 2.0 * g_cdPI)) + 
                      (BVector(0, 0, -1) * dRadius * 0.6 *
                       sin((double(fan) / 20.0) * 2.0 * g_cdPI));
      vTmp[fan * 2 + 1] = BVector(dWidth05, 0, 0) +
                          (BVector(0, 1, 0) * dRadius * 0.65 *
                           cos((double(fan) / 20.0) * 2.0 * g_cdPI)) + 
                          (BVector(0, 0, -1) * dRadius * 0.65 *
                           sin((double(fan) / 20.0) * 2.0 * g_cdPI));
      vNormals[fan] = vTmp[fan * 2] - vFoo;
      vNormals[fan].ToUnitLength();
    }
    OpenGLHelpers::TriangleStripWithNormals(vTmp, vNormals, 42);

    glEndList();
  }
}

//**********************************************************************

void BVehicle::RenderWheelBlade(double dLen, 
                                double dWide, 
                                double dNarrow, 
                                double dWideRound,
                                double dNarrowRound,
                                double dDepthOut,
                                double dDepthIn,
                                int nWheel) {
//  End profile:
//   _______
//  / /   \ \
//  I I   I I
//  I I   I I
//  I I   I I

  BWheel *pWheel = dynamic_cast<BWheel *>(m_pWheel[nWheel]);

  // OpenGLHelpers::SetColorFull(0.5, 0.5, 0.7, 0);
  OpenGLHelpers::SetColorFull(0.5, 0.5, 0.5, 1);
  glBegin(GL_TRIANGLE_STRIP);

  glNormal3f(0, 0, -1);
  glVertex3f(dDepthIn, 0, -dWide / 2.0);
  glNormal3f(0, 0, -1);
  glVertex3f(dDepthIn, dLen, -dNarrow / 2.0);

  glNormal3f(0.7, 0, -0.7);
  glVertex3f(dDepthOut * dWideRound, 0, -dWide / 2.0);
  glNormal3f(0.7, 0, -0.7);
  glVertex3f(dDepthOut * dNarrowRound, dLen, -dNarrow / 2.0);

  glNormal3f(1, 0, 0);
  glVertex3f(dDepthOut, 0, -dWide * dWideRound / 2.0);
  glNormal3f(1, 0, 0);
  glVertex3f(dDepthOut, dLen, -dNarrow * dNarrowRound / 2.0);

  glNormal3f(1, 0, 0);
  glVertex3f(dDepthOut, 0, dWide * dWideRound / 2.0);
  glNormal3f(1, 0, 0);
  glVertex3f(dDepthOut, dLen, dNarrow * dNarrowRound / 2.0);

  glNormal3f(0.7, 0, 0.7);
  glVertex3f(dDepthOut * dWideRound, 0, dWide / 2.0);
  glNormal3f(0.7, 0, 0.7);
  glVertex3f(dDepthOut * dNarrowRound, dLen, dNarrow / 2.0);

  glNormal3f(0, 0, 1);
  glVertex3f(dDepthIn, 0, dWide / 2.0);
  glNormal3f(0, 0, 1);
  glVertex3f(dDepthIn, dLen, dNarrow / 2.0);

  glEnd();

  // Back surface
  OpenGLHelpers::SetColorFull(0.2, 0.2, 0.2, 1);
  BVector vTmp[44];
  BVector vNormals[44];
  vTmp[0] = BVector(dDepthIn, 0, 0);
  vNormals[0] = BVector(-1, 0, 0);
  for(int fan = 0; fan < 21; ++fan) {
    vTmp[fan + 1] = BVector(dDepthIn, 0, 0) +
                    (BVector(0, 1, 0) * pWheel->m_dRadius * 0.6 *
                     cos((double(fan) / 20.0) * 2.0 * g_cdPI)) + 
                    (BVector(0, 0, -1) * pWheel->m_dRadius * 0.6 *
                     sin((double(fan) / 20.0) * 2.0 * g_cdPI));
    vNormals[fan + 1] = BVector(-2, 0, 0) + vTmp[fan + 1];
    vNormals[fan + 1].ToUnitLength();
  }
  OpenGLHelpers::TriangleFanWithNormals(vTmp, vNormals, 22);

  /*
  // Hub caps
  OpenGLHelpers::SetColorFull(0.5, 0.5, 0.7, 0);
  vTmp[0] = BVector(0.11, 0, 0);
  vNormals[0] = BVector(1, 0, 0);
  for(fan = 0; fan < 21; ++fan) {
    vTmp[fan + 1] = BVector(0.1, 0, 0) +
                    (BVector(0, 1, 0) * m_pWheel[0].m_dRadius * 0.65 *
                     cos((double(fan) / 20.0) * 2.0 * g_cdPI)) + 
                    (BVector(0, 0, -1) * m_pWheel[0].m_dRadius * 0.65 *
                     sin((double(fan) / 20.0) * 2.0 * g_cdPI));
    vNormals[fan + 1] = BVector(1, 0, 0) + vTmp[fan + 1];
    vNormals[fan + 1].ToUnitLength();
  }
  OpenGLHelpers::TriangleFanWithNormals(vTmp, vNormals, 22);
  */
}


BVector g_vCenter;
GLdouble g_mtxVehicle[16];


//**********************************************************************

void BVehicle::Paint(int m_nPhysicsSteps) {
  int i;

  // Find out Vehicle center
  g_vCenter.Set(0, 0, 0);
  for(i = 0; i < m_nBodyPoints; ++i) {
    g_vCenter += m_pBodyPoint[i].m_vLocation;
  }
  g_vCenter = g_vCenter * (1.0 / double(m_nBodyPoints));

  // Draw body
  glPushMatrix();

  // Use faster matrix approach

  g_mtxVehicle[ 0] = m_orientation.m_vRight.m_dX;
  g_mtxVehicle[ 1] = m_orientation.m_vRight.m_dY;
  g_mtxVehicle[ 2] = m_orientation.m_vRight.m_dZ;

  g_mtxVehicle[ 4] = m_orientation.m_vForward.m_dX;
  g_mtxVehicle[ 5] = m_orientation.m_vForward.m_dY;
  g_mtxVehicle[ 6] = m_orientation.m_vForward.m_dZ;

  g_mtxVehicle[ 8] = -m_orientation.m_vUp.m_dX;
  g_mtxVehicle[ 9] = -m_orientation.m_vUp.m_dY;
  g_mtxVehicle[10] = -m_orientation.m_vUp.m_dZ;

  g_mtxVehicle[12] = g_vCenter.m_dX;
  g_mtxVehicle[13] = g_vCenter.m_dY;
  g_mtxVehicle[14] = g_vCenter.m_dZ;

  g_mtxVehicle[ 3] = 0.0;
  g_mtxVehicle[ 7] = 0.0;
  g_mtxVehicle[11] = 0.0;

  g_mtxVehicle[15] = 1.0;

  glMultMatrixd(g_mtxVehicle);

  if(m_bWireframe) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  }

  glCallList(m_nDLVehicleBody);

  // If airplane, draw control surfaces
  if(m_bHasAirplaneControls) {
    // Draw elevator
    glPushMatrix();

    glTranslated(m_airplane.m_vElevPivot.m_dX, 
                 m_airplane.m_vElevPivot.m_dY, 
                 m_airplane.m_vElevPivot.m_dZ);
    glRotated(m_airplane.m_dElevThrow * m_airplane.m_dElevator, 
              m_airplane.m_vElevRotAxis.m_dX,
              m_airplane.m_vElevRotAxis.m_dY,
              m_airplane.m_vElevRotAxis.m_dZ);

    glCallList(m_nDLElevator);
  
    glPopMatrix();

    // Draw Rudder
    glPushMatrix();

    glTranslated(m_airplane.m_vRudderPivot.m_dX, 
                 m_airplane.m_vRudderPivot.m_dY, 
                 m_airplane.m_vRudderPivot.m_dZ);
    glRotated(m_airplane.m_dRudderThrow * m_airplane.m_dRudder, 
              m_airplane.m_vRudderRotAxis.m_dX,
              m_airplane.m_vRudderRotAxis.m_dY,
              m_airplane.m_vRudderRotAxis.m_dZ);

    glCallList(m_nDLRudder);
  
    glPopMatrix();
  }

  // Draw wheels
  glDisable(GL_CULL_FACE);
  for(i = 0; i < m_nWheels; ++i) {
    glPushMatrix();
    BWheel *pWheel = dynamic_cast<BWheel *>(m_pWheel[i]);
    BVector vLoc = pWheel->m_vLocationOrig + pWheel->m_vSuspDir * -pWheel->m_dSuspension;
    glTranslated(vLoc.m_dX, vLoc.m_dY, vLoc.m_dZ);

    // Rotate according to wheel angle (i.e. wheel revolves)
    if(m_bBreaking && !m_bAccelerating && !m_bReversing) {
      pWheel->m_dAngleStep = 0.0;
    } else if(pWheel->m_bInGround) {
      // Update revolving speed
      double dDistance = pWheel->m_orientation.m_vForward.ScalarProduct(m_pBodyPoint[pWheel->m_nBodyPoint].m_vector) * m_nPhysicsSteps;
      pWheel->m_dAngleStep = dDistance / pWheel->m_dRadius;
    } else if(m_bAccelerating) {
      pWheel->m_dAngleStep += 0.4;
    } else if(m_bReversing) {
      pWheel->m_dAngleStep -= 0.4;
    }
    if(!BGame::m_bFrozen) {
      pWheel->m_dAngle += pWheel->m_dAngleStep;
    }

    if(pWheel->m_bTurns) {
      // Rotate according to turn
      glRotated(m_dTurn * (pWheel->m_dThrow / 40.0) * m_dSteeringAid * 45.0, 0, 0, -1);
    }
    if(pWheel->m_bLeft) {
      // Rotate to make left wheel
      glRotated(180.0, 0, 0, -1);
      glRotated(pWheel->m_dAngle / g_cdPI * 180.0, -1, 0, 0);
    } else {
      glRotated(-pWheel->m_dAngle / g_cdPI * 180.0, -1, 0, 0);
    }
    glCallList(pWheel->m_nDLWheel);
    glPopMatrix();
    pWheel->m_bInGround = false;
  }
  glPopMatrix();

  if(m_bWireframe) {

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Draw body points as small red spheres
    OpenGLHelpers::SetColorFull(0.75, 0, 0, 1);
    GLUquadricObj* pQuad = gluNewQuadric();
    for(i = 0; i < m_nBodyPoints; ++i) {
      glPushMatrix();
      glTranslated(m_pBodyPoint[i].m_vLocation.m_dX,
                   m_pBodyPoint[i].m_vLocation.m_dY,
                   m_pBodyPoint[i].m_vLocation.m_dZ);
      gluSphere(pQuad, 0.05, 10, 10);
      glPopMatrix();
    }

    // Draw airdrag vectors 
    if(m_bAerodynamics) {
      for(i = 0; i < m_nBodyPoints; ++i) {
        BVector vDir = ToWorldCoord(m_pAeroPoint[i].m_vMaxDragDir) - m_vLocation;
        // vDir.ToUnitLength();
        BVector val_bvector = m_pBodyPoint[i].m_vLocation + vDir;
        OpenGLHelpers::Line(m_pBodyPoint[i].m_vLocation, 
                            //m_pBodyPoint[i].m_vLocation + vDir);
                            val_bvector);

      }
    }

    // Label body points
		glDisable(GL_LIGHTING);
    OpenGLHelpers::SetColorFull(0, 0, 0, 1);
	  glColor4d(0, 0, 0, 1);
    glDisable(GL_DEPTH_TEST);
    double dSum = 0.0;
    BVector vCenterOfGravity(0, 0, 0);
    BUI::TextRenderer()->StartRenderingText();
    string sLabel;
    for(i = 0; i < m_nBodyPoints; ++i) {
      glRasterPos3d(m_pBodyPoint[i].m_vLocation.m_dX,
                    m_pBodyPoint[i].m_vLocation.m_dY,
                    m_pBodyPoint[i].m_vLocation.m_dZ);

      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
      glMatrixMode(GL_PROJECTION);
      glPushMatrix();
      BGame::GetView()->Setup2DRendering();

      //sLabel.Format("%d", i + 1);
		stringstream format;
		format << i + 1;
		sLabel = format.str();

      GLdouble dPos[4];
      glGetDoublev(GL_CURRENT_RASTER_POSITION, dPos);
      BUI::TextRenderer()->DrawSmallTextAt(dPos[0], 
                                           dPos[1], 
                                           sLabel, 
                                           //sLabel.GetLength(), 
                                           sLabel.length(), 
                                           //BTextRenderer::TTextAlign::ALIGN_CENTER,
                                           BTextRenderer::ALIGN_CENTER,
                                           0, 0, 0, 1);
      BGame::GetView()->End2DRendering();
      glMatrixMode(GL_PROJECTION);
      glPopMatrix();
      glMatrixMode(GL_MODELVIEW);
      glPopMatrix();

      vCenterOfGravity += (m_pBodyPoint[i].m_vLocation * m_pBodyPoint[i].m_dMass);
      dSum += m_pBodyPoint[i].m_dMass;
    }
    BUI::TextRenderer()->StopRenderingText();
    vCenterOfGravity = vCenterOfGravity * (1.0 / dSum);
    glEnable(GL_DEPTH_TEST);
		glEnable(GL_LIGHTING);

    // Draw wheel base points and dir vectors
    BVector vLoc;
    BVector vLoc2;
    OpenGLHelpers::SetColorFull(0, 0.75, 0, 1);
    for(i = 0; i < m_nWheels; ++i) {
      BWheel *pWheel = dynamic_cast<BWheel *>(m_pWheel[i]);
      glPushMatrix();
      vLoc  = ToWorldCoord(pWheel->m_vSuspBasePoint);
      vLoc2 = ToWorldCoord(pWheel->m_vSuspBasePoint + pWheel->m_vSuspDir * pWheel->m_dSuspRelaxedDistance);
      glTranslated(vLoc.m_dX, vLoc.m_dY, vLoc.m_dZ);
      gluSphere(pQuad, 0.07, 10, 10);
      glPopMatrix();
      OpenGLHelpers::Line(vLoc, vLoc2);
    }

    // Draw axises (sp?)
    OpenGLHelpers::SetColorFull(0.75, 0, 0, 1);
    vLoc  = m_vLocation;
    vLoc2 = m_vLocation + m_orientation.m_vRight;
    OpenGLHelpers::Line(vLoc, vLoc2);
    OpenGLHelpers::SetColorFull(0, 0.75, 0, 1);
    vLoc  = m_vLocation;
    vLoc2 = m_vLocation + m_orientation.m_vForward;
    OpenGLHelpers::Line(vLoc, vLoc2);
    OpenGLHelpers::SetColorFull(0, 0, 0.75, 1);
    vLoc  = m_vLocation;
    vLoc2 = m_vLocation + m_orientation.m_vUp;
    OpenGLHelpers::Line(vLoc, vLoc2);

    // Draw center of gravity
    OpenGLHelpers::SetColorFull(0.75, 0.35, 0, 1);
    glPushMatrix();
    glTranslated(vCenterOfGravity.m_dX, vCenterOfGravity.m_dY, vCenterOfGravity.m_dZ);
    gluSphere(pQuad, 0.07, 10, 10);
    glPopMatrix();

    // Draw Fuel location (and distance)
    OpenGLHelpers::SetColorFull(0, 0, 0, 1);
    glPushMatrix();
    BVector vFuel = ToWorldCoord(m_vFuelLocation);
    glTranslated(vFuel.m_dX, vFuel.m_dY, vFuel.m_dZ);
    gluSphere(pQuad, 0.07, 10, 10);
    BVector val_bvector = BVector(0, 0, 0);
    BVector val_bvector2 = m_orientation.m_vForward * m_dFuelDistance;
    //OpenGLHelpers::Line(BVector(0, 0, 0), 
                        //m_orientation.m_vForward * m_dFuelDistance);
	OpenGLHelpers::Line(val_bvector, val_bvector2);
    glPopMatrix();

    // Draw lift vectors
    OpenGLHelpers::SetColorFull(0, 0.75, 0, 1);
    for(i = 0; i < m_nBodyPoints; ++i) {
      vLoc  = m_pBodyPoint[i].m_vLocation;
      vLoc2 = m_pBodyPoint[i].m_vLocation + m_pBodyPoint[i].m_vDragLift * 100.0;
      OpenGLHelpers::Line(vLoc, vLoc2);
    }

    gluDeleteQuadric(pQuad);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }
}


//**********************************************************************

void BVehicle::DrawPropeller() {
  glPushMatrix();
  glMultMatrixd(g_mtxVehicle);

  // If airplane, draw propeller
  if(m_bHasAirplaneControls) {
    // Draw Propeller
    glPushMatrix();

    glTranslated(m_airplane.m_vPropPos.m_dX, 
                 m_airplane.m_vPropPos.m_dY, 
                 m_airplane.m_vPropPos.m_dZ);
    glRotated(m_airplane.m_dPropAngle, 
              m_airplane.m_vPropDir.m_dX,
              m_airplane.m_vPropDir.m_dY,
              m_airplane.m_vPropDir.m_dZ);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_partPropeller.SetPartColor(1.0f - float(fabs(m_dPropellerFactor)));
    glCallList(m_nDLPropeller);

    glDisable(GL_BLEND);
  
    glPopMatrix();
  }


  glPopMatrix();
}



//**********************************************************************
double BVehicle::PointInsideMatter(BVector vPoint,         // [IN]  Point which is checked for matter collision
                                   BVector& rvNormal,      // [OUT] Normal of the (closest) surface point
                                   double &rdFriction,     // [OUT] Friction of the (closest) surface point
                                   double &rdBaseDepth,    // [OUT] How deep inside the matter the vPoint is
                                   double &rdThermoLoss) { // [OUT] Thermodynamic loss at the collision point
  double depthGround = BGame::GetSimulation()->PointUnderGround(vPoint, rvNormal, rdFriction, rdBaseDepth, rdThermoLoss);

  // Check also against objects
  static BVector vNormal2;
  static double  dFriction2;
  static double  dBaseDepth2;
  double depthObjects = BGame::GetSimulation()->PointInsideObject(vPoint, vNormal2, dFriction2, dBaseDepth2);

  // See which dominates
  if((depthObjects <= 0.0) || (depthGround > depthObjects)) {
    return depthGround;
  } else {
    rvNormal = vNormal2;
    rdFriction = dFriction2;
    rdBaseDepth = dBaseDepth2;
    return depthObjects;
  }
}






//**********************************************************************

static double RadToDeg(double dRad) {
  return dRad / 3.141592654 * 180.0;
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
