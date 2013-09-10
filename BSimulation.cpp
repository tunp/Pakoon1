//
// Simulation
//
// (c) Copyright 2002, Mikko Oksalahti (see end of file for details)
//

#include <cstdlib>

//#include "stdafx.h"
//#include "initguid.h"
#include "BSimulation.h"
#include "BGround.h"
#include "BObject.h"
#include "OpenGLHelpers.h"
#include "OpenGLExtFunctions.h"
#include "BTextures.h"
#include "SoundModule.h"
#include "Settings.h"
//#include "DInput.h"
#include "BGame.h"
#include "BMessages.h"
#include "HeightMap.h"

const double BSimulation::g_cdMaxSpeed = 400.0;
const double BSimulation::g_cdTurnFactor = 0.015;
const double BSimulation::g_cdAirTurnFactor = 0.05;
const double BSimulation::g_cdPropThrustFactor = 0.05;

bool g_cbBlackAndWhite = false;
bool g_cbMipMap        = true;
extern double g_dRate;


double g_dEarthQuake = 0.0;

//*****************************************************************************
double Random(double dRange) {
  return double(rand()) / double(RAND_MAX) * dRange;
}


BVector RandomVector(double dLength) {
  static BVector vRet;
  vRet.m_dX = Random(dLength) - dLength / 2.0;
  vRet.m_dY = Random(dLength) - dLength / 2.0;
  vRet.m_dZ = Random(dLength) - dLength / 2.0;
  return vRet;
}

//*****************************************************************************
// BASE ALGORITHM AND BASE VISUALIZATION
//*****************************************************************************

BSimulation::BSimulation() {
  m_dRotorVolFactor = 1.0;
  m_bPaused = false;
  m_bLiftingUp = false; 
  m_dLiftZ = 0.0;
  // m_dAccelerationFactor = g_cdAccelerationFactor;
  m_dTurnFactor = g_cdTurnFactor;
  m_dAirTurnFactor = g_cdAirTurnFactor;
  m_dPropThrustFactor = g_cdPropThrustFactor;
  m_bRecordTrail = false;
  m_bRaining = false;
  m_fp = 0;
  m_nPhysicsStepsBetweenRender = 10;
  m_dPhysicsFraction = 0.0;
  m_bSteeringAidOn = true;
  m_bCalibrateSimulationSpeed = true;

  m_nDustCloudsHead1 = -1;
  m_nDustClouds1 = 0;
  m_nDustCloudsHead2 = -1;
  m_nDustClouds2 = 0;
  m_nDustCloudsHead3 = -1;
  m_nDustClouds3 = 0;

  m_nClouds = 2;

  // setup game resolution to be initially same as current
  //DEVMODE devmode;
  //SDL_DisplayMode mode; //FIXME
  //EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devmode);
  //SDL_GetCurrentDisplayMode(0, &mode); //FIXME
  //BGame::m_nDispWidth  = devmode.dmPelsWidth;
  //BGame::m_nDispWidth  = Pakoon1View.window_width; //FIXME
  //BGame::m_nDispHeight = devmode.dmPelsHeight;
  //BGame::m_nDispHeight = Pakoon1View.window_height; //FIXME
  //BGame::m_nDispBits   = devmode.dmBitsPerPel;
  //BGame::m_nDispBits   = SDL_BITSPERPIXEL(mode.format); //FIXME
  //BGame::m_nDispHz     = devmode.dmDisplayFrequency;
  //BGame::m_nDispHz     = mode.refresh_rate; //FIXME

  // Setup camera and car location
  BVector vLocation(0, 0, 0);
  m_camera.SetLocation(vLocation);
  vLocation.m_dZ = -33.0 - 3.0;
  m_vehicle.Move(vLocation);

  // Setup targets
  m_targets = 0;

  // Create test cloud
  CreateTestCloud();
}



//*****************************************************************************
BSimulation::~BSimulation() {
}


//*****************************************************************************
void BSimulation::CreateTestCloud() {
  srand(clock());
  for(int nCloud = 0; nCloud < m_nClouds; ++nCloud) {
    m_cloud[nCloud].m_nPuffs = 10;
    int i;
    for(i = 0; i < m_cloud[nCloud].m_nPuffs; ++i) {
      m_cloud[nCloud].m_puff[i].m_vLocation = RandomVector(700.0);
      m_cloud[nCloud].m_puff[i].m_vLocation.m_dZ = -400.0;
      m_cloud[nCloud].m_puff[i].m_dRadius = 200.0 + Random(150.0);
    }    
    m_cloud[nCloud].m_vCenter = RandomVector(2000.0);
    m_cloud[nCloud].m_vCenter.m_dZ = 0.0;
    for(i = 0; i < m_cloud[nCloud].m_nPuffs; ++i) {
      double dSize = 0.65 + (1.0 - ((m_cloud[nCloud].m_puff[i].m_vLocation).Length() / 700.0)) * 0.35;
      m_cloud[nCloud].m_puff[i].m_vLocation += m_cloud[nCloud].m_vCenter;
      m_cloud[nCloud].m_puff[i].m_dRadius *= dSize;
    }
    m_cloud[nCloud].m_vCenter.m_dZ = -400.0;
  }
}






//*****************************************************************************
void BSimulation::PreProcessVisualization() {
}

//*****************************************************************************
//void BSimulation::PrePaint(CDC *pDC) {  
void BSimulation::PrePaint() {  
  if(ControllerModule::m_bInitialized && 
     (BGame::m_nController == 1) && 
     (ControllerModule::m_nCurrent >= 0)) {
	//FIXME
    /*DIJOYSTATE stateRaw;
    if(ControllerModule::GetControllerState(BGame::m_controllerstate, &stateRaw)) {
      m_vehicle.m_dTurn = BGame::m_controllerstate.m_dTurn;

      m_vehicle.m_rotor.m_bHeliLifting = false;
      m_bLiftingUp = false;
      if(BGame::m_controllerstate.m_bLift) {
        if(m_vehicle.m_rotor.m_nHeliMode > 0) {
          m_vehicle.m_rotor.m_bHeliLifting = true;
        } else if(!m_bLiftingUp) {
          m_bLiftingUp = true;
          m_dLiftZ     = m_vehicle.m_pBodyPoint[m_vehicle.m_nLiftPoint1].m_vLocation.m_dZ;
        }
      }

      m_vehicle.m_rotor.m_bHeliRighting = false;
      m_vehicle.m_rotor.m_bHeliLefting = false;
      m_vehicle.m_rotor.m_bHeliForwarding = false;
      m_vehicle.m_rotor.m_bHeliBacking = false;
      if((BGame::m_controllerstate.m_dTurn > 0.1)  && 
         (m_vehicle.m_rotor.m_nHeliMode > 399)) {
        m_vehicle.m_rotor.m_bHeliRighting = true;
      } else if((BGame::m_controllerstate.m_dTurn < -0.1)  && 
                (m_vehicle.m_rotor.m_nHeliMode > 399)) {
        m_vehicle.m_rotor.m_bHeliLefting = true;
      } 

      m_vehicle.m_bAccelerating = false; 
      m_vehicle.m_bReversing = false;
      if(BGame::m_controllerstate.m_dAcceleration > 0) {
        m_vehicle.m_dAccelerationFactor = BGame::m_controllerstate.m_dAcceleration * m_vehicle.m_dFuelFactor;
        m_vehicle.m_bAccelerating = true; 
        if((BGame::m_controllerstate.m_dAcceleration > 0.1) && 
           (m_vehicle.m_rotor.m_nHeliMode > 399)) {
          m_vehicle.m_rotor.m_bHeliForwarding = true;
        }
      }
      if(BGame::m_controllerstate.m_dReverse > 0) {
        m_vehicle.m_dReversingFactor = BGame::m_controllerstate.m_dReverse * m_vehicle.m_dFuelFactor;
        m_vehicle.m_bReversing = true;
        if((BGame::m_controllerstate.m_dReverse > 0.1) && 
           (m_vehicle.m_rotor.m_nHeliMode > 399)) {
          m_vehicle.m_rotor.m_bHeliBacking = true;
        }
      }
      m_vehicle.m_bBreaking = BGame::m_controllerstate.m_dBrake > 0.5;
      if(BGame::m_controllerstate.m_bHeliEvent) {
        ToggleHeli();
      }
      if(BGame::m_controllerstate.m_bJetEvent) {
        BGame::Command()->Run("toggle jet");
      }
      if(BGame::m_controllerstate.m_bCameraEvent) {
        SwitchCameraMode();
      }      
    }*/
  } 


  m_vehicle.m_vWindDirection = BGame::m_vWindDirection;
  m_vehicle.m_dWindStrength = BGame::m_dWindStrength;

  m_nPhysicsSteps = m_nPhysicsStepsBetweenRender;

  m_vehicle.m_bHitDetected = false;
  m_vehicle.m_dHitSpeed = 0;

  UpdateCar();      

  int nExtraStep = 1;
  if(BGame::m_bSlowMotion) {
    nExtraStep = 0;
  }

  double dFraction = m_dPhysicsFraction;
  for(int i = 0; i < m_nPhysicsSteps + nExtraStep; ++i) {
    if(BGame::m_bEarthquakeActive) {
      double dTmp;
      double dNoise = HeightMap::CalcHeightAt(double(clock()), 137.0, dTmp, HeightMap::NOISE);
      // double dNoise = sin(double(clock()) / 3000.0) * Random(0.85);
      g_dEarthQuake = dNoise * BGame::m_bEarthquakeFactor;
    } else {
      g_dEarthQuake = 0.0;
    }
    if(i < m_nPhysicsSteps) {
      m_dPhysicsFraction = 1;
    } else {
      m_dPhysicsFraction = dFraction;
    }
    BVector vTmp(0, 0, 0);
    //FIXME
    /*SoundModule::Update3DSounds(m_vehicle.m_vLocation, 
                                m_vehicle.m_pBodyPoint[0].m_vector * 300.0,
                                m_camera.m_vLocation, 
                                m_camera.m_orientation, 
                                vTmp); // Wrong. Use camera's velocity.*/

		ApplySteering();

    CreateSmokeTrails();

    m_vehicle.SimulateTimeStep();

    MoveCarPoints();
    UpdateCar();   

    if(m_vehicle.m_bHasAirplaneControls) {
      UpdateAirplaneControls();
    }
    if(m_vehicle.m_rotor.m_nHeliMode > 299) {
      UpdateHeliControls();
    }
    if(m_vehicle.m_jet.m_nJetMode > 99) {
      UpdateJetControls();
    }
    if(m_bLiftingUp) {
      if(m_vehicle.m_pBodyPoint[m_vehicle.m_nLiftPoint1].m_vLocation.m_dZ > (m_dLiftZ - 4.5)) {
        m_vehicle.m_pBodyPoint[m_vehicle.m_nLiftPoint1].m_vector = m_vehicle.m_vGravity * -40;
        m_vehicle.m_pBodyPoint[m_vehicle.m_nLiftPoint2].m_vector = m_vehicle.m_vGravity * -40;
      }
    }
  }
  UpdateTrails();
  if(BGame::m_bShowDust) {
    UpdateDustClouds();
  }

  // Consume fuel
  if(m_vehicle.m_bPropeller && m_vehicle.m_bHasAirplaneControls) {
    m_vehicle.ConsumeKerosine(0.001);
    m_vehicle.ConsumeKerosine(0.001 * m_vehicle.m_dPropellerFactor);
  }
  if(m_vehicle.m_jet.m_dJetMode > 100.0) {
    m_vehicle.ConsumeKerosine(0.001);
    m_vehicle.ConsumeKerosine(0.008 * m_vehicle.m_dAccelerationFactor);
  }
  if(m_vehicle.m_rotor.m_dHeliMode > 100.0) {
    m_vehicle.ConsumeKerosine(0.0025);
  }
  if(m_vehicle.m_dFuel > 0.0) {
    m_vehicle.ConsumeFuel(0.0005);
  }
  m_vehicle.ConsumeFuel(0.0025 * m_vehicle.m_dAccelerationFactor);

  // Update Max G-Force value
  m_dMaxGForce = m_vehicle.GetMaxGForce();
}



//*****************************************************************************
void BSimulation::UpdateEngineSound() {
  // Update car engine sound
  static int nEngineBaseVol = 55;
  double dIdealRPM1 = m_vehicle.m_dSpeed * 200000.0;
  double dIdealRPM2 = m_vehicle.m_dSpeed * m_vehicle.m_pWheel[0]->m_dTTT * 200000.0;
  double dIdealRPM = dIdealRPM1 * 0.3 + dIdealRPM2 * 0.7;
  if(m_vehicle.m_bWheelsTouchGround) {
    // Try to reach ideal
    m_vehicle.m_dRPM += (dIdealRPM - m_vehicle.m_dRPM) / 10.0;
  } else {
    if(((m_vehicle.m_bAccelerating && (m_vehicle.m_dAccelerationFactor > 0.1)) || 
        (m_vehicle.m_bReversing && (m_vehicle.m_dReversingFactor > 0.1))) && 
       (m_vehicle.m_dRPM < 60000.0)) {
      m_vehicle.m_dRPM += 200.0;
    } else {
      m_vehicle.m_dRPM *= 0.99;
    }
  }
  SoundModule::SetEngineSoundRPM(int(m_vehicle.m_dRPM));
}


//*****************************************************************************
void BSimulation::UpdateAirplaneControls() {

  // Apply propeller thrust

  if((m_vehicle.m_bPropeller || m_vehicle.m_bPropReverse) && (m_vehicle.m_dKerosineFactor > 1e-6)) {
    BVector vProp = (m_vehicle.m_orientation.m_vRight * m_vehicle.m_airplane.m_vPropDir.m_dX +
                     m_vehicle.m_orientation.m_vForward * m_vehicle.m_airplane.m_vPropDir.m_dY +
                     m_vehicle.m_orientation.m_vUp * -m_vehicle.m_airplane.m_vPropDir.m_dZ);
    vProp.ToUnitLength();
    vProp = vProp * m_vehicle.m_dPropellerFactor * m_vehicle.m_airplane.m_dPropEffect * m_vehicle.m_dHorsePowers * 0.01;

    m_vehicle.m_pBodyPoint[m_vehicle.m_airplane.m_nPropBodyPoint].m_vector += vProp;
  }

  // Rotate Propeller
  m_vehicle.m_airplane.m_dPropAngle += 2.1 * fabs(m_vehicle.m_dPropellerFactor) * 20.0;

  // Control surface forces are applied in PPEC_Vehicle::SimulateTimeStep()
}


//*****************************************************************************
void BSimulation::UpdateHeliControls() {
  double dEffect = ((double(m_vehicle.m_rotor.m_nHeliMode) - 299.0) / 101.0) * m_vehicle.m_rotor.m_dHeliBladePower;
  if(m_vehicle.m_rotor.m_bHeliLifting) {
    // Lift up in heli mode
    if(m_vehicle.m_rotor.m_dHeliLift < (m_vehicle.m_vGravity.Length() * (m_vehicle.m_nBodyPoints + 25))) { 
      m_vehicle.m_rotor.m_dHeliLift += 0.00003;
    }        
  } else if(m_vehicle.m_rotor.m_bHeliDescending) {
    // Descend in heli mode
    if(m_vehicle.m_rotor.m_dHeliLift > (m_vehicle.m_vGravity.Length() * (m_vehicle.m_nBodyPoints - 10))) {
      m_vehicle.m_rotor.m_dHeliLift -= 0.00002;
    }        
  } else {
    // Level in heli mode
    if(m_vehicle.m_rotor.m_dHeliLift < (m_vehicle.m_vGravity.Length() * (m_vehicle.m_nBodyPoints + 10))) { 
      m_vehicle.m_rotor.m_dHeliLift += 0.0001;
    } else if(m_vehicle.m_rotor.m_dHeliLift > (m_vehicle.m_vGravity.Length() * (m_vehicle.m_nBodyPoints - 10))) {
      m_vehicle.m_rotor.m_dHeliLift -= 0.0001;
    }        
  }
  if(m_vehicle.m_rotor.m_bHeliForwarding) {
    if(m_vehicle.m_rotor.m_dHeliForward < 0.01) {
      m_vehicle.m_rotor.m_dHeliForward += 0.0001;
    }        
  } else {
    if(m_vehicle.m_rotor.m_dHeliForward > 0.0) {
      m_vehicle.m_rotor.m_dHeliForward -= 0.0001;
    }        
  }
  if(m_vehicle.m_rotor.m_bHeliBacking) {
    if(m_vehicle.m_rotor.m_dHeliForward > -0.01) {
      m_vehicle.m_rotor.m_dHeliForward -= 0.0001;
    }        
  } else {
    if(m_vehicle.m_rotor.m_dHeliForward < 0.0) {
      m_vehicle.m_rotor.m_dHeliForward += 0.0001;
    }        
  }
  if(m_vehicle.m_rotor.m_bHeliRighting) {
    if(m_vehicle.m_rotor.m_dHeliRight < 0.02) {
      m_vehicle.m_rotor.m_dHeliRight += 0.0002;
    }        
  } else {
    if(m_vehicle.m_rotor.m_dHeliRight > 0.0) {
      m_vehicle.m_rotor.m_dHeliRight = 0;
    }        
  }
  if(m_vehicle.m_rotor.m_bHeliLefting) {
    if(m_vehicle.m_rotor.m_dHeliRight > -0.02) {
      m_vehicle.m_rotor.m_dHeliRight -= 0.0002;
    }        
  } else {
    if(m_vehicle.m_rotor.m_dHeliRight < 0.0) {
      m_vehicle.m_rotor.m_dHeliRight = 0;
    }        
  }
  // Apply lift
  BVector vHeliVector = m_vehicle.m_orientation.m_vUp * (m_vehicle.m_rotor.m_dHeliLift * 0.6) +
                        BVector(0, 0, -m_vehicle.m_rotor.m_dHeliLift * 0.4);
  m_vehicle.m_pBodyPoint[m_vehicle.m_rotor.m_nBodyPoints[0]].m_vector  += vHeliVector * dEffect * m_vehicle.m_rotor.m_dBodyEffects[0];
  m_vehicle.m_pBodyPoint[m_vehicle.m_rotor.m_nBodyPoints[1]].m_vector  += vHeliVector * dEffect * m_vehicle.m_rotor.m_dBodyEffects[1];
  m_vehicle.m_pBodyPoint[m_vehicle.m_rotor.m_nBodyPoints[2]].m_vector  += vHeliVector * dEffect * m_vehicle.m_rotor.m_dBodyEffects[2];
  m_vehicle.m_pBodyPoint[m_vehicle.m_rotor.m_nBodyPoints[3]].m_vector  += vHeliVector * dEffect * m_vehicle.m_rotor.m_dBodyEffects[3];

  // Apply forward
  vHeliVector = m_vehicle.m_orientation.m_vForward * dEffect;
  vHeliVector.m_dZ = 0.0;
  if(m_vehicle.m_rotor.m_dHeliForward > 0.0) {
    m_vehicle.m_pBodyPoint[m_vehicle.m_rotor.m_nBodyPoints[0]].m_vector  += vHeliVector * dEffect * m_vehicle.m_rotor.m_dBodyEffects[0] * m_vehicle.m_rotor.m_dHeliForward * 0.3;
    m_vehicle.m_pBodyPoint[m_vehicle.m_rotor.m_nBodyPoints[1]].m_vector  += vHeliVector * dEffect * m_vehicle.m_rotor.m_dBodyEffects[1] * m_vehicle.m_rotor.m_dHeliForward * 0.3;
    m_vehicle.m_pBodyPoint[m_vehicle.m_rotor.m_nBodyPoints[2]].m_vector  += vHeliVector * dEffect * m_vehicle.m_rotor.m_dBodyEffects[2] * m_vehicle.m_rotor.m_dHeliForward * 0.3;
    m_vehicle.m_pBodyPoint[m_vehicle.m_rotor.m_nBodyPoints[3]].m_vector  += vHeliVector * dEffect * m_vehicle.m_rotor.m_dBodyEffects[3] * m_vehicle.m_rotor.m_dHeliForward * 0.3;
  } else {
    m_vehicle.m_pBodyPoint[m_vehicle.m_rotor.m_nBodyPoints[0]].m_vector  += vHeliVector * dEffect * m_vehicle.m_rotor.m_dBodyEffects[0] * m_vehicle.m_rotor.m_dHeliForward * 0.3;
    m_vehicle.m_pBodyPoint[m_vehicle.m_rotor.m_nBodyPoints[1]].m_vector  += vHeliVector * dEffect * m_vehicle.m_rotor.m_dBodyEffects[1] * m_vehicle.m_rotor.m_dHeliForward * 0.3;
    m_vehicle.m_pBodyPoint[m_vehicle.m_rotor.m_nBodyPoints[2]].m_vector  += vHeliVector * dEffect * m_vehicle.m_rotor.m_dBodyEffects[2] * m_vehicle.m_rotor.m_dHeliForward * 0.3;
    m_vehicle.m_pBodyPoint[m_vehicle.m_rotor.m_nBodyPoints[3]].m_vector  += vHeliVector * dEffect * m_vehicle.m_rotor.m_dBodyEffects[3] * m_vehicle.m_rotor.m_dHeliForward * 0.3;
  }
  // Apply right turn
  m_vehicle.m_pBodyPoint[m_vehicle.m_rotor.m_nBodyPoints[0]].m_vector += m_vehicle.m_orientation.m_vForward * m_vehicle.m_rotor.m_dHeliRight * 0.15 * dEffect;
  m_vehicle.m_pBodyPoint[m_vehicle.m_rotor.m_nBodyPoints[1]].m_vector += m_vehicle.m_orientation.m_vForward * m_vehicle.m_rotor.m_dHeliRight * -0.15 * dEffect;

  // Apply instability shake
  static double dAngle = 0.0;
  BVector vShakeVector = m_vehicle.m_orientation.m_vForward * cos(dAngle) * 0.001 +
                         m_vehicle.m_orientation.m_vRight   * sin(dAngle) * 0.001;
  m_vehicle.m_pBodyPoint[m_vehicle.m_rotor.m_nBodyPoints[0]].m_vector  += vShakeVector * dEffect * m_vehicle.m_rotor.m_dBodyEffects[0];
  m_vehicle.m_pBodyPoint[m_vehicle.m_rotor.m_nBodyPoints[1]].m_vector  += vShakeVector * dEffect * m_vehicle.m_rotor.m_dBodyEffects[1];
  dAngle += 0.1;
}


//*****************************************************************************
void BSimulation::UpdateJetControls() {
  // Apply Jet Boost
  BVector vJet = m_vehicle.m_orientation.m_vForward * 
                 // g_cdAccelerationFactor *       
                 (m_vehicle.m_bAccelerating ? m_vehicle.m_dAccelerationFactor * 1.5 : 0.012);
  m_vehicle.m_pBodyPoint[m_vehicle.m_jet.m_nBodyPoints[0]].m_vector  += vJet * m_vehicle.m_jet.m_dBodyEffects[0];
  m_vehicle.m_pBodyPoint[m_vehicle.m_jet.m_nBodyPoints[1]].m_vector  += vJet * m_vehicle.m_jet.m_dBodyEffects[1];
  m_vehicle.m_pBodyPoint[m_vehicle.m_jet.m_nBodyPoints[2]].m_vector  += vJet * m_vehicle.m_jet.m_dBodyEffects[2];
  m_vehicle.m_pBodyPoint[m_vehicle.m_jet.m_nBodyPoints[3]].m_vector  += vJet * m_vehicle.m_jet.m_dBodyEffects[3];
}

//*****************************************************************************
void BSimulation::ToggleHeli() {
  if(GetVehicle()->m_rotor.m_nHeliMode) {
    GetVehicle()->m_rotor.m_bHeliModeActivating = !GetVehicle()->m_rotor.m_bHeliModeActivating;
  } else {
    // Start heli mode
    if(GetVehicle()->m_bHasRotor) {
      GetVehicle()->m_rotor.m_dHeliLift = m_vehicle.m_vGravity.Length() * m_vehicle.m_nBodyPoints;
      GetVehicle()->m_rotor.m_bHeliLifting = false;
      GetVehicle()->m_rotor.m_bHeliDescending = false;
      GetVehicle()->m_rotor.m_bHeliForwarding = false;
      GetVehicle()->m_rotor.m_bHeliRighting = false;
      GetVehicle()->m_rotor.m_nHeliMode = 99;
      GetVehicle()->m_rotor.m_dHeliMode = 99.0,
      GetVehicle()->m_rotor.m_bHeliModeActivating = true;
    }
  }
}


//*****************************************************************************
void BSimulation::SwitchCameraMode() {
  switch(GetCamera()->m_locMode) {
    case BCamera::FIXED:
      GetCamera()->m_locMode = BCamera::FOLLOW;
      GetCamera()->m_dAngleOfView = 75.0;
      BMessages::Remove("camera");
      BMessages::Show(60, "camera", "camera: chase", 1);
      break;
    case BCamera::FOLLOW:
      GetCamera()->m_locMode = BCamera::OVERVIEW;
      GetCamera()->m_bInitLoc = true;
      GetCamera()->m_dAngleOfView = 80.0;
      BMessages::Remove("camera");
      BMessages::Show(60, "camera", "camera: overview", 1);
      break;
    case BCamera::OVERVIEW:
      GetCamera()->m_locMode = BCamera::INCAR;
      GetCamera()->m_dAngleOfView = 75.0;
      BMessages::Remove("camera");
      BMessages::Show(60, "camera", "camera: 1st person", 1);
      break;
    case BCamera::INCAR:
      GetCamera()->m_locMode = BCamera::ONSIDE;
      GetCamera()->m_dAngleOfView = 75.0;
      BMessages::Remove("camera");
      BMessages::Show(60, "camera", "camera: front wheel", 1);
      break;
    case BCamera::ONSIDE:
      GetCamera()->m_locMode = BCamera::SIDEVIEW;
      GetCamera()->m_dAngleOfView = 75.0;
      BMessages::Remove("camera");
      BMessages::Show(60, "camera", "camera: side", 1);
      break;
    case BCamera::SIDEVIEW:
      GetCamera()->m_locMode = BCamera::FIXED;
      GetCamera()->m_vFixLocation = GetCamera()->m_vLocation;
      GetCamera()->m_dAngleOfView = 75.0;
      BMessages::Remove("camera");
      BMessages::Show(60, "camera", "camera: stationary", 1);
      break;
  }
}


//*****************************************************************************
void BSimulation::UpdateGasStationTracking() {
  // Find closest gas station
  BTrackingTarget *pTarget = m_targets;
  while(pTarget) {
    //if(pTarget->m_sId.CompareNoCase("FUEL") == 0) {
    if(pTarget->m_sId.compare("FUEL") == 0) {
      break;
    }
    pTarget = pTarget->m_pNext;
  }

  BVector vVehicle = GetVehicle()->m_vLocation;
  vVehicle.m_dZ = 0.0;
  BTerrainBlock *pNewTargetBlock = 0;
  double dDist = 99999.9;
  if(pTarget) {
    // Loop through terrain blocks to find closer station

    int nBlock = 0;
    BTerrainBlock *pBlock;
    for(nBlock = 0, pBlock = GetTerrain()->m_ringVisible.GetHead(); 
        nBlock < GetTerrain()->m_ringVisible.GetNBlocks(); 
        pBlock = pBlock->m_pNext, ++nBlock) {
      if(pBlock->m_bHasGaso) {
        double dNewDist = (vVehicle - pBlock->m_vGasoCenter).Length();
        if(dNewDist < dDist) {
          pNewTargetBlock = pBlock;
          dDist = dNewDist;
        }
      }
    }
  }

  if(pNewTargetBlock) {
    pTarget->m_vLoc = pNewTargetBlock->m_vGasoCenter;
    BGame::m_vGasStationClosest = pNewTargetBlock->m_vGasoCenter + BVector(0, -5.0, 0);
  }
}



//*****************************************************************************
void BSimulation::PaintSky(float fBrightness, bool bFog) {
  glPushMatrix();
  glTranslated(m_camera.m_vLocation.m_dX, m_camera.m_vLocation.m_dY, 0.0);
  if(!bFog) {
    glDisable(GL_FOG);
  }
  GLfloat fLight1AmbientG[ 4];
  fLight1AmbientG[0] = fBrightness;
  fLight1AmbientG[1] = fBrightness;
  fLight1AmbientG[2] = fBrightness;
  fLight1AmbientG[3] = fBrightness;
  glLightfv( GL_LIGHT0, GL_AMBIENT,  fLight1AmbientG);

  OpenGLHelpers::SwitchToTexture(0);
  if(BGame::m_nSkyDetail == 1) {
    BTextures::Use(BTextures::SKY);
  } else if(BGame::m_nSkyDetail == 2) {
    BTextures::Use(BTextures::SKY);
  }
  OpenGLHelpers::SetColorFull(1, 1, 1, fBrightness);
  m_ground.PaintSky(BGame::m_nSkyDetail);
  fLight1AmbientG[0] = 0.3f;
  fLight1AmbientG[1] = 0.3f;
  fLight1AmbientG[2] = 0.3f;
  fLight1AmbientG[3] = 0.0f;
  glLightfv( GL_LIGHT0, GL_AMBIENT,  fLight1AmbientG);
  if(!bFog) {
    glEnable(GL_FOG);
  }
  glPopMatrix();
}



//*****************************************************************************
void BSimulation::PaintWaterSurface() {

  // Use stencil buffer to render sky reflection on the water surface
  // First draw the water surface only to the stencil buffer
  glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
  glDepthMask(GL_FALSE);

  glEnable(GL_STENCIL_TEST);
  glStencilFunc(GL_ALWAYS, 1, 1);
  glStencilOp(GL_REPLACE, GL_ZERO, GL_REPLACE);
  
  glPushMatrix();
  glTranslated(m_camera.m_vLocation.m_dX, m_camera.m_vLocation.m_dY, 0.0);
  m_ground.PaintWaterSurface(1, 1);
  glPopMatrix();

  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  glStencilFunc(GL_EQUAL, 1, 1);
  glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
 
  // Draw the reflection of the sky clipped with the stencil buffer
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glPushMatrix();
    glScalef(1, 1, -1);
    glCullFace(GL_FRONT);
    glDisable(GL_DEPTH_TEST);
    PaintSky(0.5f, false);
    glEnable(GL_DEPTH_TEST);
    glCullFace(GL_BACK);
  glPopMatrix();

  glStencilFunc(GL_ALWAYS, 1, 1);
  glDepthMask(GL_TRUE);
  glDisable(GL_BLEND);
  glDisable(GL_STENCIL_TEST);
}


//*****************************************************************************
void BSimulation::PaintFaintWaterSurface() {
  // Paint faint water surface always to simulate the surface effect

  OpenGLHelpers::SwitchToTexture(0);
  BTextures::Use(BTextures::WATER);

  if(BGame::m_nWaterSurface != 0) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }

  glPushMatrix();
  glTranslated(int(m_camera.m_vLocation.m_dX / 100.0) * 100.0, int(m_camera.m_vLocation.m_dY / 100.0) * 100.0, 0.0);
  m_ground.PaintWaterSurface(1.0, 0.7);
  glPopMatrix();

  glDisable(GL_TEXTURE_2D);

  if(BGame::m_nWaterSurface != 0) {
    glDisable(GL_BLEND);
  }
}






//*****************************************************************************
//int BSimulation::Paint(CDC *pDC, bool bCreateDLs, bool bWireframe, bool bNormals, CRect &rectWnd) {
int BSimulation::Paint(bool bCreateDLs, bool bWireframe, bool bNormals) {

  // Render sky
  //if(BGame::m_nVisualize & BGame::TAnalyzerVis::SKY) {
  if(BGame::m_nVisualize & BGame::SKY) {
    PaintSky(1.0f, false);
  }

  // Update terrain database
  int nOffTime = 0;
  glPushMatrix();
  glTranslated(0, 0, g_dEarthQuake);
  //if(BGame::m_nVisualize & BGame::TAnalyzerVis::TERRAIN) {
  if(BGame::m_nVisualize & BGame::TERRAIN) {
    clock_t clockStart = clock();
    m_terrain.MakeTerrainValid(m_vehicle.m_vLocation, 
                               m_camera.m_vLocation, 
                               m_camera.m_orientation.m_vForward, 
                               bCreateDLs, 
                               bWireframe, 
                               bNormals);
    nOffTime = clock() - clockStart;

    // Render terrain
    OpenGLHelpers::SetDefaultLighting();

    m_terrain.Render(BGame::m_nSkyDetail, m_camera.m_vLocation, m_camera.m_orientation.m_vForward);
  }

  // Render Objects
  //if(BGame::m_nVisualize & BGame::TAnalyzerVis::OBJECTS) {
  if(BGame::m_nVisualize & BGame::OBJECTS) {
    PaintSceneObjects();
  }

  // Render water surface
  //if(BGame::m_nVisualize & BGame::TAnalyzerVis::WATER) {
  if(BGame::m_nVisualize & BGame::WATER) {
    if((m_camera.m_vLocation.m_dZ < 0.0) && (BGame::m_nWaterSurface == 2)) {
      PaintWaterSurface();
    }
  }

  glShadeModel(GL_SMOOTH);
  OpenGLHelpers::SetColorFull(1, 1, 1, 1);

  // Render shadows and trail marks and jet and rotor (they all use same texture)
  //if(BGame::m_nVisualize & BGame::TAnalyzerVis::DUSTANDCLOUDS) {
  if(BGame::m_nVisualize & BGame::DUSTANDCLOUDS) {
    if(!BGame::m_bSceneEditorMode) {
      OpenGLHelpers::SwitchToTexture(0);
      BTextures::Use(BTextures::SHADOW);
      glEnable(GL_BLEND);
      glDisable(GL_DEPTH_TEST);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      DrawShadowAndTrails();
      glPopMatrix();

      glDisable(GL_BLEND);
      glEnable(GL_DEPTH_TEST);
    }
  }
  glPopMatrix();

  // Render vehicle
  //if(BGame::m_nVisualize & BGame::TAnalyzerVis::VEHICLE) {
  if(BGame::m_nVisualize & BGame::VEHICLE) {
    if((GetCamera()->m_locMode != BCamera::INCAR) && (!BGame::m_bSceneEditorMode)) {
      glEnable(GL_CULL_FACE);
      glCullFace(GL_FRONT);
      m_vehicle.Paint(m_nPhysicsSteps);
      glCullFace(GL_BACK);
      glDisable(GL_CULL_FACE);
    }
  }

  //if(BGame::m_nVisualize & BGame::TAnalyzerVis::WATER) {
  if(BGame::m_nVisualize & BGame::WATER) {
    PaintFaintWaterSurface();
  }

  //if(BGame::m_nVisualize & BGame::TAnalyzerVis::VEHICLE) {
  if(BGame::m_nVisualize & BGame::VEHICLE) {
    if(m_vehicle.m_jet.m_nJetMode) {
      DrawJet();
    }
    if(m_vehicle.m_rotor.m_nHeliMode || !m_vehicle.m_rotor.m_bHeliHatchesOK) {
      DrawRotor();
    }
  }

  //if(BGame::m_nVisualize & BGame::TAnalyzerVis::DUSTANDCLOUDS) {
  if(BGame::m_nVisualize & BGame::DUSTANDCLOUDS) {
    if(BGame::m_bShowDust) {
      if(!BGame::m_bSceneEditorMode) {
        DrawDustClouds();
      }
      DrawClouds();
    }

    if(m_vehicle.m_bHasAirplaneControls) {
      DrawSmokeTrails();
    }
  }

  // Last, draw propeller
  //if(BGame::m_nVisualize & BGame::TAnalyzerVis::VEHICLE) {
  if(BGame::m_nVisualize & BGame::VEHICLE) {
    if((GetCamera()->m_locMode != BCamera::INCAR) && (!BGame::m_bSceneEditorMode)) {
      m_vehicle.DrawPropeller();
    }
  }

  // If hit detected, play crash sound
  if(m_vehicle.m_bHitDetected) {
    SoundModule::PlayCrashSound(m_vehicle.m_dHitSpeed * 6.0);
    BVector vTmp(0, 0, 0);
    //FIXME
    /*SoundModule::Update3DSounds(m_vehicle.m_vLocation, 
                                m_vehicle.m_pBodyPoint[0].m_vector * 300.0,
                                m_camera.m_vLocation, 
                                m_camera.m_orientation, 
                                vTmp); // Wrong. Use camera's velocity.*/
  }

  return nOffTime;
}



//*****************************************************************************
void BSimulation::PaintSceneObjects() {
  // Render all objects within world sphere
  BScene *pScene = GetScene();
  BCamera *pCamera = GetCamera();
  int nObject;

  // First shadows

  glEnable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CW);
  glEnable(GL_COLOR_MATERIAL);
  glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
  glCullFace(GL_BACK);

  glDisable(GL_TEXTURE_2D); 
  OpenGLHelpers::SetColorFull(0, 0, 0, 0.3);
  for(nObject = 0; nObject < pScene->m_nObjects; ++nObject) {
    pScene->m_pObjects[nObject].m_bVisible = true;
    BVector vCamToObj = pScene->m_pObjects[nObject].m_vCenter - pCamera->m_vLocation;
    double dProjOnCamForward = pCamera->m_orientation.m_vForward.ScalarProduct(vCamToObj);
    // see if object is visible
    double dDist = (pScene->m_pObjects[nObject].m_vLocation - pCamera->m_vLocation).Length();
    if((dProjOnCamForward > -pScene->m_pObjects[nObject].m_dRadius) && 
       (vCamToObj.Length() < cdWorldHemisphereRadius)) {
      if(pScene->m_pObjects[nObject].m_bHasShadow) {
        glPushMatrix();
        glTranslated(pScene->m_pObjects[nObject].m_vLocation.m_dX,
                     pScene->m_pObjects[nObject].m_vLocation.m_dY,
                     0.0);
        glRotated(pScene->m_pObjects[nObject].m_dZRotation, 0, 0, -1);
        glScaled(pScene->m_pObjects[nObject].m_dScale2, 
                 pScene->m_pObjects[nObject].m_dScale2,
                 1.0);
        pScene->m_pObjects[nObject].DrawObject(true);
        glPopMatrix();
      }
    } else {
      pScene->m_pObjects[nObject].m_bVisible = false;
    }
  }

  glDisable(GL_COLOR_MATERIAL);
  glDisable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);

  // Then objects

  for(nObject = 0; nObject < pScene->m_nObjects; ++nObject) {
    if(pScene->m_pObjects[nObject].m_bVisible) {
      glPushMatrix();
      glTranslated(pScene->m_pObjects[nObject].m_vLocation.m_dX,
                   pScene->m_pObjects[nObject].m_vLocation.m_dY,
                   pScene->m_pObjects[nObject].m_vLocation.m_dZ);
      glRotated(pScene->m_pObjects[nObject].m_dZRotation, 0, 0, -1);
      glScaled(pScene->m_pObjects[nObject].m_dScale2, 
               pScene->m_pObjects[nObject].m_dScale2,
               pScene->m_pObjects[nObject].m_dScale2);
      pScene->m_pObjects[nObject].DrawObject(false);
      glPopMatrix();
    }
  }
  glFrontFace(GL_CCW);
}



//*****************************************************************************
void BSimulation::CheckHeliPartForDamage(THeliPart part, BVector vPoint, int nInfo) {
  static BVector vNormal;
  static double dTmp, dDepth;
  if((PointUnderGround(vPoint, vNormal, dTmp, dDepth, dTmp) > 0.0 && dDepth > 0.0) ||
     (PointInsideObject(vPoint, vNormal, dTmp, dDepth) > 0.0 && dDepth > 0.0)) {
    if(m_vehicle.m_rotor.m_bHeliModeActivating && (m_vehicle.m_rotor.m_nHeliMode < 300)) {
      m_vehicle.m_rotor.m_bHeliModeActivating = false;
      BMessages::Show(40, "tmp", "ROTOR DAMAGE IMMINENT!", 3, true);
    } else {
      DamageHeliPart(part, nInfo);
      BMessages::Show(40, "tmp", "ROTOR DAMAGED!", 3, true, 1, 0, 0);
    }
  }
}

//*****************************************************************************
void BSimulation::DamageHeliPart(THeliPart part, int nInfo) {
  m_vehicle.m_rotor.m_bHeliOK = false;
  switch(part) {
    case LEFTHATCH:
      m_vehicle.m_rotor.m_bHeliHatchesOK = false;
      m_vehicle.m_rotor.m_dHeliLeftHatchOffsetAngle = 0.1 + Random(0.1);
      break;
    case RIGHTHATCH:
      m_vehicle.m_rotor.m_bHeliHatchesOK = false;
      m_vehicle.m_rotor.m_dHeliRightHatchOffsetAngle = 0.1 + Random(0.1);
      break;
    case COVER:
      m_vehicle.m_rotor.m_bHeliCoverOK = false;
      break;
    case BLADE:
      m_vehicle.m_rotor.m_bHeliBladeOK[nInfo] = false;
      m_vehicle.m_rotor.m_dHeliBladePower = ((m_vehicle.m_rotor.m_bHeliBladeOK[0] ? 1.0 : 0.0) + 
                                             (m_vehicle.m_rotor.m_bHeliBladeOK[1] ? 1.0 : 0.0) + 
                                             (m_vehicle.m_rotor.m_bHeliBladeOK[2] ? 1.0 : 0.0)) / 3.0;
      if(m_vehicle.m_rotor.m_dHeliBladePower < 0.01) {
        SoundModule::SetHeliSoundVolume(0);
      }
      break;
  }
}


//*****************************************************************************
const double g_cdPI = 3.141592654;
static const double dPI2 = 2.0 * g_cdPI;

//*****************************************************************************
void BSimulation::DrawRotor() {
  static bool bSoundStarted = false;
  static double dAngle = Random(dPI2);
  int i;

  double dScaler = 1.0;

  // Draw base axis
  BVector vTranslate;
  if(m_vehicle.m_rotor.m_nHeliMode < 100) {
    vTranslate = m_vehicle.m_orientation.m_vUp * -1;
  } else if(m_vehicle.m_rotor.m_nHeliMode >= 200) {
    vTranslate.Set(0, 0, 0);
  } else {
    double dPhase = (double(m_vehicle.m_rotor.m_nHeliMode) - 100.0) / 100.0;
    vTranslate = m_vehicle.m_orientation.m_vUp * -1 * (1.0 - dPhase);
  }  
  glShadeModel(GL_FLAT);
  OpenGLHelpers::SetColorFull(0.3, 0.3, 0.3, 0);
  BVector vBase = m_vehicle.ToWorldCoord(m_vehicle.m_rotor.m_vExitPoint);
  BVector vCenter = vTranslate + vBase;
  BVector vCenterUp = vCenter + m_vehicle.m_orientation.m_vUp * 0.5;
  if(!m_vehicle.m_rotor.m_bHeliCoverOK) {
    vCenterUp -= m_vehicle.m_orientation.m_vUp * 0.3;
    vCenterUp += m_vehicle.m_orientation.m_vForward * cos(dAngle) * 0.1 + 
                 m_vehicle.m_orientation.m_vRight   * sin(dAngle) * 0.1;
  }
  BVector vAxisBase = vCenter;
  BVector vStrip[14];
  BVector vNormals[14];
  for(i = 0; i < 4; ++i) {
    vNormals[i] = m_vehicle.m_orientation.m_vForward * cos(dAngle + double(i) * dPI2 / 3.0) * 0.05 + 
                  m_vehicle.m_orientation.m_vRight   * sin(dAngle + double(i) * dPI2 / 3.0) * 0.05;
    vStrip[i * 2] = 
      vCenter + 
      m_vehicle.m_orientation.m_vForward * cos(dAngle + double(i) * dPI2 / 3.0) * 0.05 + 
      m_vehicle.m_orientation.m_vRight   * sin(dAngle + double(i) * dPI2 / 3.0) * 0.05;
    vStrip[i * 2 + 1] = 
      vCenterUp + 
      m_vehicle.m_orientation.m_vForward * cos(dAngle + double(i) * dPI2 / 3.0) * 0.05 + 
      m_vehicle.m_orientation.m_vRight   * sin(dAngle + double(i) * dPI2 / 3.0) * 0.05;
  }
  OpenGLHelpers::TriangleStripWithNormals(vStrip, vNormals, 8);
 
  // Draw hood
  glShadeModel(GL_SMOOTH);
  vCenter = vCenterUp + m_vehicle.m_orientation.m_vUp * 0.1;
  for(i = 0; i < 7; ++i) {
    vNormals[i] = m_vehicle.m_orientation.m_vForward * cos(dAngle + double(i) * dPI2 / 6.0) * 0.1 + 
                  m_vehicle.m_orientation.m_vRight   * sin(dAngle + double(i) * dPI2 / 6.0) * 0.1;
    vStrip[i * 2] = 
      vCenterUp + 
      m_vehicle.m_orientation.m_vForward * cos(dAngle + double(i) * dPI2 / 6.0) * 0.1 + 
      m_vehicle.m_orientation.m_vRight   * sin(dAngle + double(i) * dPI2 / 6.0) * 0.1;
    vStrip[i * 2 + 1] = 
      vCenter + 
      m_vehicle.m_orientation.m_vForward * cos(dAngle + double(i) * dPI2 / 6.0) * 0.1 + 
      m_vehicle.m_orientation.m_vRight   * sin(dAngle + double(i) * dPI2 / 6.0) * 0.1;
  }
  OpenGLHelpers::TriangleStripWithNormals(vStrip, vNormals, 14);

  // Draw hood cover
  vStrip[0] = vCenter;
  for(i = 0; i < 7; ++i) {
    vStrip[i + 1] = 
      vCenter + 
      m_vehicle.m_orientation.m_vForward * cos(dAngle + double(i) * dPI2 / 6.0) * 0.1 + 
      m_vehicle.m_orientation.m_vRight   * sin(dAngle + double(i) * dPI2 / 6.0) * 0.1;
  }
  glNormal3f(0, 0, -1);
  OpenGLHelpers::TriangleFan(vStrip, 8);

  CheckHeliPartForDamage(COVER, vCenter);

  BVector vTmp;

  glShadeModel(GL_FLAT);

  OpenGLHelpers::SetColorFull(0.2, 0.2, 0.2, 1);
  if(m_vehicle.m_rotor.m_nHeliMode < 200) {
    // Draw blades retracted
    DrawRetractedBlades(vCenterUp, 0.1, 0.01, dAngle);
  } else if(m_vehicle.m_rotor.m_nHeliMode < 300) {
    if(bSoundStarted) {
      SoundModule::StopHeliSound();
      bSoundStarted = false;      
    }
    // Draw blades opening/closing
    double dPhase = (double(m_vehicle.m_rotor.m_nHeliMode) - 200.0) / 100.0;
    if(dPhase < 0.01) {
      dPhase = 0.01;
    }
    DrawRetractedBlades(vCenterUp, 0.1, dPhase, dAngle);
  } else if(m_vehicle.m_rotor.m_nHeliMode < 400) {
    if(!bSoundStarted) {
      SoundModule::StartHeliSound();
      bSoundStarted = true;
    }
    double dPhase = (double(m_vehicle.m_rotor.m_nHeliMode) - 300.0) / 100.0;
    SoundModule::SetHeliSoundPhase(dPhase, m_vehicle.m_rotor.m_dHeliBladePower);
    // Draw blades picking up/slowing down speed
    DrawRetractedBlades(vCenterUp, 0.1, 1.0, dAngle);
    dAngle += (0.5 + Random(0.01)) * dPhase;
  } else {
    // Draw three faint triangles to represent rotor blades
    OpenGLHelpers::SetColorFull(0, 0, 0, 0.3);
    //glDisable(GL_DEPTH_TEST);
    OpenGLHelpers::SwitchToTexture(0);
    BTextures::Use(BTextures::SHADOW);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glNormal3f(m_vehicle.m_orientation.m_vUp.m_dX, 
               m_vehicle.m_orientation.m_vUp.m_dY, 
               m_vehicle.m_orientation.m_vUp.m_dZ);

    bool bRotorLeft = false;
    for(i = 0; i < 3; ++i) {
      if(!m_vehicle.m_rotor.m_bHeliBladeOK[i]) {
        dAngle += ((120.0 / 360.0) * dPI2);
        continue;
      }
      bRotorLeft = true;
      glBegin(GL_TRIANGLE_STRIP);
      for(double f = 0; f < 5.9; f += 1.0) {        
        vTmp = vCenterUp + m_vehicle.m_orientation.m_vUp * 0.03 * 3.0 +
               m_vehicle.m_orientation.m_vForward * 
               cos(dAngle + f * 0.2) * 0.1 + 
               m_vehicle.m_orientation.m_vRight * 
               sin(dAngle + f * 0.2) * 0.1;
        glTexCoord2f(8.0 / 16.0 + f / 5.0 * 0.45, 4.0 / 16.0);
        glVertex3f(vTmp.m_dX, vTmp.m_dY, vTmp.m_dZ);

        vTmp = vCenterUp + m_vehicle.m_orientation.m_vUp * 0.03 * 3.0 +
               m_vehicle.m_orientation.m_vForward * 
               cos(dAngle + f * 0.2) * m_vehicle.m_rotor.m_dBladeLength + 
               m_vehicle.m_orientation.m_vRight * 
               sin(dAngle + f * 0.2) * m_vehicle.m_rotor.m_dBladeLength;
        glTexCoord2f(8.0 / 16.0 + f / 5.0 * 0.45, 4.0 / 16.0);
        glVertex3f(vTmp.m_dX, vTmp.m_dY, vTmp.m_dZ);
      }
      glEnd();

      CheckHeliPartForDamage(BLADE, vTmp, i);

      dAngle += ((120.0 / 360.0) * dPI2);
    }
    if(!BGame::m_bFrozen) {
      dAngle += 0.5 + Random(0.01);
      if(bRotorLeft) {
        // Add dust particles if we are close to ground
        double dTmp, depth;
        if((depth = fabs(PointUnderGround(m_vehicle.m_vLocation, vTmp, dTmp, dTmp, dTmp))) < 60.0) {
          for(int i = 0; i < m_nPhysicsStepsBetweenRender / 2; ++i) {
            BVector vRand = RandomVector(1.0);
            vRand.m_dZ = 0.0;
            vRand.ToUnitLength();
            BVector vDir = (vRand + BVector(0, 0, -0.5)) * 0.2;
            vRand = vRand * 4.0;
            vRand = vRand + m_vehicle.m_vLocation;
            vRand.m_dZ = m_vehicle.m_vLocation.m_dZ + depth;
            CreateDustCloudAt(vRand, 3, (30.0 - depth) / 30.0, vDir);
          }
        }
      }
    }
    // glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
  }

  // Proceed to next step
  double dFactor = 60.0 / g_dRate;
  if(m_vehicle.m_rotor.m_bHeliModeActivating && m_vehicle.m_rotor.m_nHeliMode < 300) {
    m_vehicle.m_rotor.m_dHeliMode += (2.0 * dFactor);
    m_vehicle.m_rotor.m_nHeliMode = int(m_vehicle.m_rotor.m_dHeliMode);
  } else if(m_vehicle.m_rotor.m_bHeliModeActivating && m_vehicle.m_rotor.m_nHeliMode < 400) {
    m_vehicle.m_rotor.m_dHeliMode += (1.0 * dFactor);
    m_vehicle.m_rotor.m_nHeliMode = int(m_vehicle.m_rotor.m_dHeliMode);
  } else if(!m_vehicle.m_rotor.m_bHeliModeActivating && m_vehicle.m_rotor.m_nHeliMode > 0) {
    if(m_vehicle.m_rotor.m_nHeliMode < 300) {
      m_vehicle.m_rotor.m_dHeliMode -= (2.0 * dFactor);
      m_vehicle.m_rotor.m_nHeliMode = int(m_vehicle.m_rotor.m_dHeliMode);
    } else {
      m_vehicle.m_rotor.m_dHeliMode -= (1.0 * dFactor);
      m_vehicle.m_rotor.m_nHeliMode = int(m_vehicle.m_rotor.m_dHeliMode);
    }
    if(m_vehicle.m_rotor.m_nHeliMode <= 99) {
      m_vehicle.m_rotor.m_dHeliMode = 0.0;
      m_vehicle.m_rotor.m_nHeliMode = 0;
    }
  }
  OpenGLHelpers::SetColorFull(1, 1, 1, 1);
  glNormal3f(0, 0, -1);
}


//*****************************************************************************
void BSimulation::DrawRetractedBlades(BVector& rvCenter, 
                                      double dRad, 
                                      double dPhase, 
                                      double dAngle) {
  double dBladeLen = m_vehicle.m_rotor.m_dBladeLength - dRad;
  double dPartLen = dBladeLen / 8.0;
  BVector vUp  = m_vehicle.m_orientation.m_vUp * cos(dPhase * g_cdPI / 2.0) * dPartLen;
  for(int i = 0; i < 3; ++i) {
    if(!m_vehicle.m_rotor.m_bHeliBladeOK[i]) {
      dAngle += ((120.0 / 360.0) * dPI2);
      continue;
    }
    BVector vOut = (m_vehicle.m_orientation.m_vForward * 
                    cos(dAngle) * dPartLen + 
                    m_vehicle.m_orientation.m_vRight * 
                    sin(dAngle) * dPartLen) * 
                   sin(dPhase * g_cdPI / 2.0);
    BVector v1 = m_vehicle.m_orientation.m_vForward * 
                 cos(dAngle - 1) * dRad + 
                 m_vehicle.m_orientation.m_vRight * 
                 sin(dAngle - 1) * dRad;
    BVector v2 = m_vehicle.m_orientation.m_vForward * 
                 cos(dAngle + 1) * dRad + 
                 m_vehicle.m_orientation.m_vRight * 
                 sin(dAngle + 1) * dRad;
    glBegin(GL_TRIANGLE_STRIP);
    bool bGoUp = true;
    BVector vPos = rvCenter;
    for(int j = 0; j < 9; ++j) {
      BVector vNormal;
      if(bGoUp) { 
        vNormal = vOut + vUp;
      } else {
        vNormal = vOut - vUp;
      }
      vNormal.ToUnitLength();
      glNormal3f(vNormal.m_dX, vNormal.m_dY, vNormal.m_dZ);
      glVertex3f(vPos.m_dX + v1.m_dX, 
                 vPos.m_dY + v1.m_dY, 
                 vPos.m_dZ + v1.m_dZ);
      glNormal3f(vNormal.m_dX, vNormal.m_dY, vNormal.m_dZ);
      glVertex3f(vPos.m_dX + v2.m_dX, 
                 vPos.m_dY + v2.m_dY, 
                 vPos.m_dZ + v2.m_dZ);

      if((j == 1) || (j == 8)) {
        CheckHeliPartForDamage(BLADE, vPos, i);
      }

      if(bGoUp) { 
        vPos += vUp;
      } else {
        vPos -= vUp;
      }
      vPos += vOut;
      bGoUp = !bGoUp;
    }
    glEnd();
    dAngle += ((120.0 / 360.0) * dPI2);
  }
}







//*****************************************************************************
void BSimulation::DrawJet() {
  static bool bSoundStarted = false;

  BVector vLinen[10];

  // Draw hatch covers in correct phase
  /*
  glShadeModel(GL_FLAT);
  double dHatchPhase;
  if(m_vehicle.m_jet.m_nJetMode > 50) {
    dHatchPhase = 1.0;
  } else {
    dHatchPhase = double(m_vehicle.m_jet.m_nJetMode) / 50.0; 
  }
  dHatchPhase;

  // Draw upper hatch
  OpenGLHelpers::SwitchToTexture(0);
  BTextures::Use(BTextures::CAR);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 128.0f);
  BVector v1 = m_vehicle.m_pBodyPoint[5].m_vLocation * 0.8 +
               m_vehicle.m_pBodyPoint[14].m_vLocation * 0.2;
  BVector v2 = m_vehicle.m_pBodyPoint[5].m_vLocation * 0.2 +
               m_vehicle.m_pBodyPoint[14].m_vLocation * 0.8;
  BVector v3 = m_vehicle.m_pBodyPoint[6].m_vLocation * 0.8 +
               m_vehicle.m_pBodyPoint[15].m_vLocation * 0.2;
  double dHatch1Len = (v3 - v1).Length();
  BVector vTmp = (v3 - v1) * 0.3;
  v1 += vTmp;
  v2 += vTmp;

  vLinen[1] = v3;
  vLinen[2] = v1;
  vLinen[4] = m_vehicle.m_pBodyPoint[6].m_vLocation * 0.2 +
              m_vehicle.m_pBodyPoint[15].m_vLocation * 0.8;
  vLinen[5] = v2;

  dHatch1Len *= 0.7;
  vTmp.ToUnitLength();
  glNormal3f(vTmp.m_dX, -vTmp.m_dZ, vTmp.m_dY);
  vTmp = m_vehicle.m_orientation.m_vForward * -dHatch1Len * cos(dHatchPhase - g_cdPI / 4.0) +
         m_vehicle.m_orientation.m_vUp * dHatch1Len * sin(dHatchPhase - g_cdPI / 4.0);
  OpenGLHelpers::SetColorFull(0.5, 0.5, 0.5, 0);

  glBegin(GL_TRIANGLE_STRIP);
  glTexCoord2f(0.4, 0.3);
  glVertex3f(v1.m_dX, v1.m_dY, v1.m_dZ);
  glTexCoord2f(2.0 - 0.4, 0.3);
  glVertex3f(v2.m_dX, v2.m_dY, v2.m_dZ);
  v3 = v1 + vTmp;
  glTexCoord2f(0.4, 1.0);
  glVertex3f(v3.m_dX, v3.m_dY, v3.m_dZ);
  v3 = v2 + vTmp;
  glTexCoord2f(2.0 - 0.4, 1.0);
  glVertex3f(v3.m_dX, v3.m_dY, v3.m_dZ);
  glEnd();

  BVector v11 = v1;
  BVector v12 = v2;
  BVector v13 = v1 + vTmp;
  BVector v14 = v2 + vTmp;

  // Draw lower hatch
  v1 = m_vehicle.m_pBodyPoint[7].m_vLocation * 0.8 +
       m_vehicle.m_pBodyPoint[16].m_vLocation * 0.2;
  v2 = m_vehicle.m_pBodyPoint[7].m_vLocation * 0.2 +
       m_vehicle.m_pBodyPoint[16].m_vLocation * 0.8;
  v3 = m_vehicle.m_pBodyPoint[6].m_vLocation * 0.8 +
       m_vehicle.m_pBodyPoint[15].m_vLocation * 0.2;
  double dHatch2Len = (v3 - v1).Length();
  vTmp = (v3 - v1) * 0.3;
  v1 += vTmp;
  v2 += vTmp;

  vLinen[0] = v1;
  vLinen[3] = v2;

  dHatch2Len *= 0.7;
  vTmp.ToUnitLength();
  glNormal3f(vTmp.m_dX, -vTmp.m_dZ, vTmp.m_dY);
  vTmp = m_vehicle.m_orientation.m_vForward * -dHatch2Len * cos(-dHatchPhase*2.0 + asin(1.0/5.0) + g_cdPI / 2.0) +
         m_vehicle.m_orientation.m_vUp * dHatch2Len * sin(-dHatchPhase*2.0 + asin(1.0/5.0) + g_cdPI / 2.0);
  OpenGLHelpers::SetColorFull(0.5, 0.5, 0.5, 0);
  glBegin(GL_TRIANGLE_STRIP);
  glTexCoord2f(0.4, 0.3 * 0.7);
  glVertex3f(v1.m_dX, v1.m_dY, v1.m_dZ);
  glTexCoord2f(2.0 - 0.4, 0.3 * 0.7);
  glVertex3f(v2.m_dX, v2.m_dY, v2.m_dZ);
  v3 = v1 + vTmp;
  glTexCoord2f(0.4, 0.0);
  glVertex3f(v3.m_dX, v3.m_dY, v3.m_dZ);
  v3 = v2 + vTmp;
  glTexCoord2f(2.0 - 0.4, 0.0);
  glVertex3f(v3.m_dX, v3.m_dY, v3.m_dZ);
  glEnd();

  BVector v21 = v1;
  BVector v22 = v2;
  BVector v23 = v1 + vTmp;
  BVector v24 = v2 + vTmp;

  glDisable(GL_TEXTURE_2D);

  // Draw hatch insides
  OpenGLHelpers::SetColorFull(0.1, 0.1, 0.1, 0);
  // OpenGLHelpers::SetColorFull(0.3, 0.3, 0.3, 0);
  BVector vTiny = (v13 - v11).CrossProduct(v12 - v11);
  vTiny.ToUnitLength();
  vTiny = vTiny * 0.05;
  glBegin(GL_TRIANGLE_STRIP);
  glVertex3f(v11.m_dX, v11.m_dY, v11.m_dZ);
  glVertex3f(v12.m_dX, v12.m_dY, v12.m_dZ);
  glVertex3f(v11.m_dX + vTiny.m_dX, v11.m_dY + vTiny.m_dY, v11.m_dZ + vTiny.m_dZ);
  glVertex3f(v12.m_dX + vTiny.m_dX, v12.m_dY + vTiny.m_dY, v12.m_dZ + vTiny.m_dZ);
  glVertex3f(v13.m_dX + vTiny.m_dX, v13.m_dY + vTiny.m_dY, v13.m_dZ + vTiny.m_dZ);
  glVertex3f(v14.m_dX + vTiny.m_dX, v14.m_dY + vTiny.m_dY, v14.m_dZ + vTiny.m_dZ);
  glVertex3f(v13.m_dX, v13.m_dY, v13.m_dZ);
  glVertex3f(v14.m_dX, v14.m_dY, v14.m_dZ);
  glEnd();
  glBegin(GL_TRIANGLE_STRIP);
  glVertex3f(v11.m_dX, v11.m_dY, v11.m_dZ);
  glVertex3f(v13.m_dX, v13.m_dY, v13.m_dZ);
  glVertex3f(v11.m_dX + vTiny.m_dX, v11.m_dY + vTiny.m_dY, v11.m_dZ + vTiny.m_dZ);
  glVertex3f(v13.m_dX + vTiny.m_dX, v13.m_dY + vTiny.m_dY, v13.m_dZ + vTiny.m_dZ);
  glEnd();
  glBegin(GL_TRIANGLE_STRIP);
  glVertex3f(v12.m_dX, v12.m_dY, v12.m_dZ);
  glVertex3f(v14.m_dX, v14.m_dY, v14.m_dZ);
  glVertex3f(v12.m_dX + vTiny.m_dX, v12.m_dY + vTiny.m_dY, v12.m_dZ + vTiny.m_dZ);
  glVertex3f(v14.m_dX + vTiny.m_dX, v14.m_dY + vTiny.m_dY, v14.m_dZ + vTiny.m_dZ);
  glEnd();

  OpenGLHelpers::SetColorFull(0.1, 0.1, 0.1, 0);
  // OpenGLHelpers::SetColorFull(0.3, 0.3, 0.3, 0);
  vTiny = (v22 - v21).CrossProduct(v23 - v21);
  vTiny.ToUnitLength();
  vTiny = vTiny * 0.05;
  glBegin(GL_TRIANGLE_STRIP);
  glVertex3f(v21.m_dX, v21.m_dY, v21.m_dZ);
  glVertex3f(v22.m_dX, v22.m_dY, v22.m_dZ);
  glVertex3f(v21.m_dX + vTiny.m_dX, v21.m_dY + vTiny.m_dY, v21.m_dZ + vTiny.m_dZ);
  glVertex3f(v22.m_dX + vTiny.m_dX, v22.m_dY + vTiny.m_dY, v22.m_dZ + vTiny.m_dZ);
  glVertex3f(v23.m_dX + vTiny.m_dX, v23.m_dY + vTiny.m_dY, v23.m_dZ + vTiny.m_dZ);
  glVertex3f(v24.m_dX + vTiny.m_dX, v24.m_dY + vTiny.m_dY, v24.m_dZ + vTiny.m_dZ);
  glVertex3f(v23.m_dX, v23.m_dY, v23.m_dZ);
  glVertex3f(v24.m_dX, v24.m_dY, v24.m_dZ);
  glEnd();
  glBegin(GL_TRIANGLE_STRIP);
  glVertex3f(v21.m_dX, v21.m_dY, v21.m_dZ);
  glVertex3f(v23.m_dX, v23.m_dY, v23.m_dZ);
  glVertex3f(v21.m_dX + vTiny.m_dX, v21.m_dY + vTiny.m_dY, v21.m_dZ + vTiny.m_dZ);
  glVertex3f(v23.m_dX + vTiny.m_dX, v23.m_dY + vTiny.m_dY, v23.m_dZ + vTiny.m_dZ);
  glEnd();
  glBegin(GL_TRIANGLE_STRIP);
  glVertex3f(v22.m_dX, v22.m_dY, v22.m_dZ);
  glVertex3f(v24.m_dX, v24.m_dY, v24.m_dZ);
  glVertex3f(v22.m_dX + vTiny.m_dX, v22.m_dY + vTiny.m_dY, v22.m_dZ + vTiny.m_dZ);
  glVertex3f(v24.m_dX + vTiny.m_dX, v24.m_dY + vTiny.m_dY, v24.m_dZ + vTiny.m_dZ);
  glEnd();

  // Draw base cowl
  glShadeModel(GL_SMOOTH);
  OpenGLHelpers::SetColorFull(0.25, 0.25, 0.25, 0);
  BVector vStart;
  BVector vTranslate = m_vehicle.m_orientation.m_vUp * 0.1 + 
                       m_vehicle.m_orientation.m_vForward * 
                       (1.0 - double(m_vehicle.m_jet.m_nJetMode - 50.0) / 50.0);
  if(m_vehicle.m_jet.m_nJetMode < 50) {
    vTranslate = m_vehicle.m_orientation.m_vUp * 0.1 + 
                 m_vehicle.m_orientation.m_vForward;
  }
  vStart = (m_vehicle.m_pBodyPoint[6].m_vLocation + 
            m_vehicle.m_pBodyPoint[15].m_vLocation) * 0.5 +
            m_vehicle.m_orientation.m_vForward * 0.4 +
            vTranslate;

  vLinen[6] = vStart + m_vehicle.m_orientation.m_vForward * 0.4 - m_vehicle.m_orientation.m_vUp * 0.45 - m_vehicle.m_orientation.m_vRight * 0.45;
  vLinen[7] = vStart + m_vehicle.m_orientation.m_vForward * 0.4 + m_vehicle.m_orientation.m_vUp * 0.45 - m_vehicle.m_orientation.m_vRight * 0.45;
  vLinen[8] = vStart + m_vehicle.m_orientation.m_vForward * 0.4 - m_vehicle.m_orientation.m_vUp * 0.45 + m_vehicle.m_orientation.m_vRight * 0.45;
  vLinen[9] = vStart + m_vehicle.m_orientation.m_vForward * 0.4 + m_vehicle.m_orientation.m_vUp * 0.45 + m_vehicle.m_orientation.m_vRight * 0.45;

  BVector vEnd = vStart + m_vehicle.m_orientation.m_vForward * -0.8;
  BVector vStrip[42];
  glBegin(GL_TRIANGLE_STRIP);
  for(i = 0; i < 21; ++i) {
    vStrip[i * 2] = 
      vStart + m_vehicle.m_orientation.m_vForward * 0.4 +
      m_vehicle.m_orientation.m_vUp      * cos(double(i) * dPI2 / 20.0) * 0.42 + 
      m_vehicle.m_orientation.m_vRight   * sin(double(i) * dPI2 / 20.0) * 0.42;
    vStrip[i * 2 + 1] = 
      vEnd + 
      m_vehicle.m_orientation.m_vUp      * cos(double(i) * dPI2 / 20.0) * 0.3 + 
      m_vehicle.m_orientation.m_vRight   * sin(double(i) * dPI2 / 20.0) * 0.3;
    BVector vNormal = vStrip[i * 2] - vStart + m_vehicle.m_orientation.m_vForward * 0.05;
    vNormal.ToUnitLength();
    glNormal3f(vNormal.m_dX, vNormal.m_dY, vNormal.m_dZ);
    glVertex3f(vStrip[i * 2].m_dX,     vStrip[i * 2].m_dY,     vStrip[i * 2].m_dZ);
    vNormal = vStrip[i * 2 + 1] - vEnd + m_vehicle.m_orientation.m_vForward * -0.05;
    glNormal3f(vNormal.m_dX, vNormal.m_dY, vNormal.m_dZ);
    glVertex3f(vStrip[i * 2 + 1].m_dX, vStrip[i * 2 + 1].m_dY, vStrip[i * 2 + 1].m_dZ);    
  }
  glEnd();

  // Draw inner chrome cowl
  OpenGLHelpers::SetColorFull(0.5, 0.5, 0.5, 0);
  GLfloat fLight[4];
  fLight[0] = 1.0f;
  fLight[1] = 1.0f;
  fLight[2] = 1.0f;
  fLight[3] = 1.0f;
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, fLight);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 100.0f);
  vStart = (m_vehicle.m_pBodyPoint[6].m_vLocation + 
            m_vehicle.m_pBodyPoint[15].m_vLocation) * 0.5 +
            m_vehicle.m_orientation.m_vForward * 0.2 +
            vTranslate;
  vEnd = vStart + m_vehicle.m_orientation.m_vForward * -0.8;
  glBegin(GL_TRIANGLE_STRIP);
  for(i = 0; i < 21; ++i) {
    vStrip[i * 2] = 
      vStart + 
      m_vehicle.m_orientation.m_vUp      * cos(double(i) * dPI2 / 20.0) * 0.35 + 
      m_vehicle.m_orientation.m_vRight   * sin(double(i) * dPI2 / 20.0) * 0.35;
    vStrip[i * 2 + 1] = 
      vEnd + 
      m_vehicle.m_orientation.m_vUp      * cos(double(i) * dPI2 / 20.0) * 0.25 + 
      m_vehicle.m_orientation.m_vRight   * sin(double(i) * dPI2 / 20.0) * 0.25;
    BVector vNormal = vStrip[i * 2] - vStart + m_vehicle.m_orientation.m_vForward * 0.05;
    vNormal.ToUnitLength();
    glNormal3f(vNormal.m_dX, vNormal.m_dY, vNormal.m_dZ);
    glVertex3f(vStrip[i * 2].m_dX,     vStrip[i * 2].m_dY,     vStrip[i * 2].m_dZ);
    vNormal = vStrip[i * 2 + 1] - vEnd + m_vehicle.m_orientation.m_vForward * -0.05;
    glNormal3f(vNormal.m_dX, vNormal.m_dY, vNormal.m_dZ);
    glVertex3f(vStrip[i * 2 + 1].m_dX, vStrip[i * 2 + 1].m_dY, vStrip[i * 2 + 1].m_dZ);
  }
  glEnd();
  // OpenGLHelpers::TriangleStrip(vStrip, 42);


  // Draw inner jet glow
  double dJetGlow;
  if(m_vehicle.m_jet.m_nJetMode <= 50) {
    dJetGlow = 0.0;
  } else {
    dJetGlow = double(m_vehicle.m_jet.m_nJetMode - 50) / 50.0; 
  }
  OpenGLHelpers::SetColorFull(0.65 * dJetGlow, 0.6 * dJetGlow, 1.0 * dJetGlow, 0);
  BVector vStart = (m_vehicle.m_pBodyPoint[6].m_vLocation + 
            m_vehicle.m_pBodyPoint[15].m_vLocation) * 0.5 +
            m_vehicle.m_orientation.m_vForward * -0.4 +
            vTranslate;
  vStrip[0] = vStart;
  for(i = 0; i < 21; ++i) {
    vStrip[i + 1] = 
      vStart + 
      m_vehicle.m_orientation.m_vUp      * cos(double(i) * dPI2 / 20.0) * 0.25 + 
      m_vehicle.m_orientation.m_vRight   * sin(double(i) * dPI2 / 20.0) * 0.25;
  }
  OpenGLHelpers::TriangleFan(vStrip, 22);

  // Draw linen
  OpenGLHelpers::SetColorFull(0.1, 0.1, 0.1, 0);
  vStrip[0] = vLinen[1];
  vStrip[1] = vLinen[6];
  vStrip[2] = vLinen[2];
  vStrip[3] = vLinen[7];
  vStrip[4] = vLinen[5];
  vStrip[5] = vLinen[9];
  vStrip[6] = vLinen[4];
  vStrip[7] = vLinen[8];
  OpenGLHelpers::TriangleStrip(vStrip, 8);
  vStrip[0] = vLinen[6];
  vStrip[1] = vLinen[7];
  vStrip[2] = vLinen[8];
  vStrip[3] = vLinen[9];
  OpenGLHelpers::TriangleStrip(vStrip, 4);
  vStrip[0] = vLinen[1];
  vStrip[1] = vLinen[6];
  vStrip[2] = vLinen[0];
  vStrip[3] = vLinen[8];
  vStrip[4] = vLinen[3];
  vStrip[5] = vLinen[4];
  OpenGLHelpers::TriangleStrip(vStrip, 6);

  // Draw jet flames
  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_FALSE);
  OpenGLHelpers::SetColorFull(1, 1, 1, 1);
  if(m_vehicle.m_jet.m_nJetMode > 99) {
    if(!bSoundStarted) {
      SoundModule::StartJetSound();
      bSoundStarted = true;
    }
    double dJetLen = 0.5;
    vStart = (m_vehicle.m_pBodyPoint[6].m_vLocation + 
              m_vehicle.m_pBodyPoint[15].m_vLocation) * 0.5 +
              m_vehicle.m_orientation.m_vForward * -(0.6 + Random(0.3)) +
              vTranslate;
    if(m_vehicle.m_dAccelerationFactor > 0.99) {
      if(m_vehicle.m_bAccelerating) {
        if(dJetLen < 3.0) {
          dJetLen = 3.0;
        }
        vEnd = vStart + m_vehicle.m_orientation.m_vForward * -dJetLen;
      } else {
        dJetLen = 0.5;
        vEnd = vStart + m_vehicle.m_orientation.m_vForward * -dJetLen;
      }
    } else {
      if(m_vehicle.m_bAccelerating) {
        if(dJetLen < 3.0) {
          dJetLen = 0.5 + m_vehicle.m_dAccelerationFactor * 3.0;
        }
        vEnd = vStart + m_vehicle.m_orientation.m_vForward * -dJetLen;
      } else {
        dJetLen = 0.5;
        vEnd = vStart + m_vehicle.m_orientation.m_vForward * -dJetLen;
      }
    }

    SoundModule::SetJetSoundPhase((dJetLen - 0.5) / 2.5);

    for(i = 0; i < 21; ++i) {
      vStrip[i * 2] = 
        vStart + 
        m_vehicle.m_orientation.m_vUp      * cos(double(i) * dPI2 / 20.0) * 0.25 + 
        m_vehicle.m_orientation.m_vRight   * sin(double(i) * dPI2 / 20.0) * 0.25;
      vStrip[i * 2 + 1] = 
        vEnd + 
        m_vehicle.m_orientation.m_vUp      * cos(double(i) * dPI2 / 20.0) * 0.20 + 
        m_vehicle.m_orientation.m_vRight   * sin(double(i) * dPI2 / 20.0) * 0.20;
    }

    glPushAttrib(GL_ENABLE_BIT | GL_TEXTURE_2D);
    OpenGLHelpers::SwitchToTexture(0);
    BTextures::Use(BTextures::SHADOW);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBegin(GL_TRIANGLE_STRIP);
    for(i = 0; i < 42; ++i) {
      if((i % 2) == 0) {
        BVector vNormal = vStrip[i / 2] - vStart;
        vNormal.ToUnitLength();
        glNormal3f(vNormal.m_dX, vNormal.m_dY, vNormal.m_dZ);
      }
      if(((i / 2) % 2) == 0) {
        if((i % 2) == 0) {
          glTexCoord2f(1.0 / 16.0, 8.5 / 16.0);
        } else {
          glTexCoord2f(1.0 / 16.0, 9.0 / 16.0);
        }
      } else {
        if((i % 2) == 0) {
          glTexCoord2f(2.0 / 16.0, 8.5 / 16.0);
        } else {
          glTexCoord2f(2.0 / 16.0, 9.0 / 16.0);
        }
      }
      glVertex3f(vStrip[i].m_dX, vStrip[i].m_dY, vStrip[i].m_dZ);
    }
    glEnd();
    // Fade out
    vStart = vEnd;
    if(m_vehicle.m_bAccelerating) {
      vEnd = vStart + m_vehicle.m_orientation.m_vForward * -dJetLen;
    } else {
      vEnd = vStart + m_vehicle.m_orientation.m_vForward * -dJetLen;
    }
    for(i = 0; i < 21; ++i) {
      vStrip[i * 2] = 
        vStart + 
        m_vehicle.m_orientation.m_vUp      * cos(double(i) * dPI2 / 20.0) * 0.20 + 
        m_vehicle.m_orientation.m_vRight   * sin(double(i) * dPI2 / 20.0) * 0.20;
      vStrip[i * 2 + 1] = vEnd;
    }
    glBegin(GL_TRIANGLE_STRIP);
    for(i = 0; i < 42; ++i) {
      if((i % 2) == 0) {
        BVector vNormal = vStrip[i / 2] - vStart;
        vNormal.ToUnitLength();
        glNormal3f(vNormal.m_dX, vNormal.m_dY, vNormal.m_dZ);
      }
      if(((i / 2) % 2) == 0) {
        if((i % 2) == 0) {
          glTexCoord2f(1.0 / 16.0, 9.0 / 16.0);
        } else {
          glTexCoord2f(1.0 / 16.0, 12.0 / 16.0);
        }
      } else {
        if((i % 2) == 0) {
          glTexCoord2f(2.0 / 16.0, 9.0 / 16.0);
        } else {
          glTexCoord2f(2.0 / 16.0, 12.0 / 16.0);
        }
      }
      glVertex3f(vStrip[i].m_dX, vStrip[i].m_dY, vStrip[i].m_dZ);
    }
    glEnd();
    glPopAttrib();
    glDisable(GL_BLEND);
  } else {
    if(bSoundStarted) {
      SoundModule::StopJetSound();
      bSoundStarted = false;
    }
  }
  glDepthMask(GL_TRUE);

  // Proceed to next step
  double dFactor = 60.0 / g_dRate;
  if(m_vehicle.m_jet.m_bJetModeActivating && m_vehicle.m_jet.m_nJetMode < 100) {
    m_vehicle.m_jet.m_dJetMode += 2.0 * dFactor;
    m_vehicle.m_jet.m_nJetMode = int(m_vehicle.m_jet.m_dJetMode);
  } else if(!m_vehicle.m_jet.m_bJetModeActivating && m_vehicle.m_jet.m_nJetMode > 0) {
    m_vehicle.m_jet.m_dJetMode -= 2.0 * dFactor;
    m_vehicle.m_jet.m_nJetMode = int(m_vehicle.m_jet.m_dJetMode);
    if(m_vehicle.m_jet.m_nJetMode <= 1) {
      m_vehicle.m_jet.m_dJetMode = 0.0;
      m_vehicle.m_jet.m_nJetMode = 0;
    }
  }
  */
}



//*****************************************************************************
// PHYSICS SIMULATION
//*****************************************************************************



//*****************************************************************************
void BSimulation::EnsureVehicleIsOverGround() {
  int i;

  double dMaxRad = 0.0;

  // First find biggest wheel
  for(i = 0; i < m_vehicle.m_nWheels; ++i) {
    if(m_vehicle.m_pWheel[i]->m_dRadius > dMaxRad) {
      dMaxRad = m_vehicle.m_pWheel[i]->m_dRadius;
    }
  }

  double dMaxLift = 0.0;
  for(i = 0; i < m_vehicle.m_nBodyPoints; ++i) {
    BVector vNormal;
    BVector v = m_vehicle.m_pBodyPoint[i].m_vLocation;
    v.m_dZ += dMaxRad;
    double depth = PointUnderGroundShadow(v, vNormal);
    if(depth > dMaxLift) {
      dMaxLift = depth;
    }
  }
  BVector val_bvector(0, 0, -1.0 - dMaxLift);
  m_vehicle.Move(val_bvector);
  UpdateCar();
}



//*****************************************************************************
void BSimulation::UpdateGasStationLocation() {
  m_pCenterBlock = FindTerrainBlock(m_vehicle.m_vLocation);
  if(m_pCenterBlock->m_bHasGaso) {
    m_pCenterBlock->m_objectArray[0]->m_vLocation = m_pCenterBlock->m_vGasoCenter;
    m_pCenterBlock->m_objectArray[0]->m_vCenter   = m_pCenterBlock->m_vGasoCenter;
  }
}



//*****************************************************************************
void BSimulation::UpdateCarLocation() {
  int i;
  m_vehicle.m_vLocation.Set(0, 0, 0);  
  for(i = 0; i < m_vehicle.m_nBodyPoints; ++i) {
    m_vehicle.m_vLocation += m_vehicle.m_pBodyPoint[i].m_vLocation;
  }
  m_vehicle.m_vLocation = m_vehicle.m_vLocation * (1.0 / double(m_vehicle.m_nBodyPoints));
}





//*****************************************************************************
void BSimulation::UpdateCar() {
  // Place car
  UpdateCarLocation();

  UpdateGasStationLocation();

  // Determine car orientation
  m_vehicle.m_orientation.m_vForward = (m_vehicle.m_pBodyPoint[m_vehicle.m_nForwardPoints[1]].m_vLocation - m_vehicle.m_pBodyPoint[m_vehicle.m_nForwardPoints[0]].m_vLocation) +
                                       (m_vehicle.m_pBodyPoint[m_vehicle.m_nForwardPoints[3]].m_vLocation - m_vehicle.m_pBodyPoint[m_vehicle.m_nForwardPoints[2]].m_vLocation);
  m_vehicle.m_orientation.m_vForward.ToUnitLength();
  m_vehicle.m_orientation.m_vRight = (m_vehicle.m_pBodyPoint[m_vehicle.m_nRightPoints[1]].m_vLocation - m_vehicle.m_pBodyPoint[m_vehicle.m_nRightPoints[0]].m_vLocation) +
                                     (m_vehicle.m_pBodyPoint[m_vehicle.m_nRightPoints[3]].m_vLocation - m_vehicle.m_pBodyPoint[m_vehicle.m_nRightPoints[2]].m_vLocation);
  m_vehicle.m_orientation.m_vRight.ToUnitLength();
  m_vehicle.m_orientation.m_vUp = m_vehicle.m_orientation.m_vRight.CrossProduct(m_vehicle.m_orientation.m_vForward);
  m_vehicle.m_orientation.m_vUp.ToUnitLength();
  m_vehicle.m_orientation.m_vUp = m_vehicle.m_orientation.m_vUp * -1.0;

  // Determine car vector and speed
  m_vehicle.m_vector = (m_vehicle.m_pBodyPoint[m_vehicle.m_nForwardPoints[0]].m_vector + 
                        m_vehicle.m_pBodyPoint[m_vehicle.m_nForwardPoints[1]].m_vector +
                        m_vehicle.m_pBodyPoint[m_vehicle.m_nForwardPoints[2]].m_vector +
                        m_vehicle.m_pBodyPoint[m_vehicle.m_nForwardPoints[3]].m_vector) * 0.25;
  m_vehicle.m_dSpeed = m_vehicle.m_vector.Length();

  // Place wheels
  if(m_bSteeringAidOn) {
    m_vehicle.m_dSteeringAid = 1.0 - (m_vehicle.m_dSpeed / 0.3);
    if(m_vehicle.m_dSteeringAid < 0.3) {
      m_vehicle.m_dSteeringAid = 0.3;
    }
  } else {
    m_vehicle.m_dSteeringAid = 1.0;
  }
  for(int w = 0; w < m_vehicle.m_nWheels; ++w) {
    BWheel *pWheel = dynamic_cast<BWheel *>(m_vehicle.m_pWheel[w]);
    BVector vLoc = pWheel->m_vSuspBasePoint +
                   pWheel->m_vSuspDir * 
                   pWheel->m_dSuspRelaxedDistance +
                   pWheel->m_vSuspDir * 
                  -pWheel->m_dSuspension;
    pWheel->m_vLocation = m_vehicle.ToWorldCoord(vLoc);
    pWheel->m_orientation = m_vehicle.m_orientation;
    if(pWheel->m_bTurns) {
      // Apply steering to turning wheels
      pWheel->m_orientation.m_vForward += pWheel->m_orientation.m_vRight * m_vehicle.m_dTurn * m_vehicle.m_dSteeringAid * (pWheel->m_dThrow / 40.0);
      pWheel->m_orientation.m_vForward.ToUnitLength();
      pWheel->m_orientation.m_vRight = pWheel->m_orientation.m_vUp.CrossProduct(pWheel->m_orientation.m_vForward);
      pWheel->m_orientation.m_vRight.ToUnitLength();
    }
  }
}





//*****************************************************************************
void BSimulation::CreateSmokeTrails() {
  // Create smoke/vapor trails from the trailpoints
  static int nSkipper = 0;
  if((++nSkipper % 2) && (m_vehicle.m_dSpeed > 0.2)) {
    for(int i = 0; i < m_vehicle.m_nTrailpoints; ++i) {

      BVector vWindCorrected = m_vehicle.m_pBodyPoint[m_vehicle.m_nTrailpoint[i]].m_vector - BGame::m_vWindDirection * BGame::m_dWindStrength * 0.6;
      BVector vDir2 = vWindCorrected;
      vDir2.ToUnitLength();

      // Calculate angle of attack factor
      BVector vMaxDrag = m_vehicle.m_orientation.m_vRight   *  m_vehicle.m_pAeroPoint[m_vehicle.m_nTrailpoint[i]].m_vMaxDragDir.m_dX +
                         m_vehicle.m_orientation.m_vForward *  m_vehicle.m_pAeroPoint[m_vehicle.m_nTrailpoint[i]].m_vMaxDragDir.m_dY +
                         m_vehicle.m_orientation.m_vUp      * -m_vehicle.m_pAeroPoint[m_vehicle.m_nTrailpoint[i]].m_vMaxDragDir.m_dZ;
      BVector vDir = vMaxDrag;
      vDir.ToUnitLength();

      double dAoA = fabs(vDir.ScalarProduct(vDir2)) * 20.0;
      if(dAoA > 1.0) {
        dAoA = 1.0;
      }
      dAoA = pow(dAoA, 2.0);

      // create new trailpoint
      int nPnt = (m_vehicle.m_nTrailpointHead + m_vehicle.m_nTrailpointEntries) % 100;
      m_vehicle.m_trailpointEntry[i][nPnt].m_vLocation = m_vehicle.m_pBodyPoint[m_vehicle.m_nTrailpoint[i]].m_vLocation;
      m_vehicle.m_trailpointEntry[i][nPnt].m_vRight    = m_vehicle.m_orientation.m_vRight;
      m_vehicle.m_trailpointEntry[i][nPnt].m_vUp       = m_vehicle.m_orientation.m_vUp;
      m_vehicle.m_trailpointEntry[i][nPnt].m_dAlpha    = (m_vehicle.m_dSpeed - 0.2) / 0.6 + Random(0.1);
      m_vehicle.m_trailpointEntry[i][nPnt].m_dAlpha = pow(m_vehicle.m_trailpointEntry[i][nPnt].m_dAlpha, 2.0) * dAoA;
      if(m_vehicle.m_trailpointEntry[i][nPnt].m_dAlpha > 1.0) {
        m_vehicle.m_trailpointEntry[i][nPnt].m_dAlpha = 1.0;
      }
    }

    // proceed one trailpoint
    if(m_vehicle.m_nTrailpointEntries < 100) {
      ++(m_vehicle.m_nTrailpointEntries);
    } else {
      m_vehicle.m_nTrailpointHead = (m_vehicle.m_nTrailpointHead + 1) % 100;
    }
  } 
}

//*****************************************************************************
void BSimulation::ApplySteering() {

  // Check for joystick info
  if(true) { // Always support keyboard as well
    // Use keyboard turning
    // If turning, do so
    if(m_vehicle.m_bTurningLeft) {
      // Turn left
      if(m_vehicle.m_dTurn > -0.7)  {
        m_vehicle.m_dTurn -= m_dTurnFactor;
      }
      if(m_vehicle.m_bHasAirplaneControls) {
        if(m_vehicle.m_airplane.m_dRudder > -m_vehicle.m_airplane.m_dUseRudderForTurn)  {
          m_vehicle.m_airplane.m_dRudder -= m_dAirTurnFactor;
        }
        if(m_vehicle.m_airplane.m_dAilerons > -1.0)  {
          m_vehicle.m_airplane.m_dAilerons -= m_dAirTurnFactor;
        }
      }
    } else if(m_vehicle.m_bTurningRight) {
      // Turn right
      if(m_vehicle.m_dTurn < 0.7)  {
        m_vehicle.m_dTurn += m_dTurnFactor;
      }
      if(m_vehicle.m_bHasAirplaneControls) {
        if(m_vehicle.m_airplane.m_dRudder < m_vehicle.m_airplane.m_dUseRudderForTurn)  {
          m_vehicle.m_airplane.m_dRudder += m_dAirTurnFactor;
        }
        if(m_vehicle.m_airplane.m_dAilerons < 1.0)  {
          m_vehicle.m_airplane.m_dAilerons += m_dAirTurnFactor;
        }
      }
    } else {
      // Center wheels
      if(m_vehicle.m_dTurn > m_dTurnFactor * 3.0) {
        m_vehicle.m_dTurn -= m_dTurnFactor * 3.0;
      } else if(m_vehicle.m_dTurn < -m_dTurnFactor) {
        m_vehicle.m_dTurn += m_dTurnFactor * 3.0;
      } else {
        m_vehicle.m_dTurn = 0.0;
      }

      if(m_vehicle.m_bHasAirplaneControls) {
        if(m_vehicle.m_airplane.m_dRudder > m_dAirTurnFactor * 0.25) {
          m_vehicle.m_airplane.m_dRudder -= m_dAirTurnFactor * 0.25;
        } else if(m_vehicle.m_airplane.m_dRudder < -m_dAirTurnFactor * 0.25) {
          m_vehicle.m_airplane.m_dRudder += m_dAirTurnFactor * 0.25;
        } else {
          m_vehicle.m_airplane.m_dRudder = 0.0;
        }
        if(m_vehicle.m_airplane.m_dAilerons > m_dAirTurnFactor * 0.25) {
          m_vehicle.m_airplane.m_dAilerons -= m_dAirTurnFactor * 0.25;
        } else if(m_vehicle.m_airplane.m_dAilerons < -m_dAirTurnFactor * 0.25) {
          m_vehicle.m_airplane.m_dAilerons += m_dAirTurnFactor * 0.25;
        } else {
          m_vehicle.m_airplane.m_dAilerons = 0.0;
        }
      }
    }
    if(m_vehicle.m_bHasAirplaneControls) {
      if(m_vehicle.m_bAccelerating) {
        if(m_vehicle.m_airplane.m_dElevator > (-1.0 - m_vehicle.m_airplane.m_dElevTrim * 0.3))  {
          m_vehicle.m_airplane.m_dElevator -= m_dAirTurnFactor;
        }
      } else if(m_vehicle.m_bReversing) {
        if(m_vehicle.m_airplane.m_dElevator < (1.0 - m_vehicle.m_airplane.m_dElevTrim * 0.3))  {
          m_vehicle.m_airplane.m_dElevator += m_dAirTurnFactor;
        }
      } else {
        if(m_vehicle.m_airplane.m_dElevator > m_dAirTurnFactor) {
          m_vehicle.m_airplane.m_dElevator -= m_dAirTurnFactor;
        } else if(m_vehicle.m_airplane.m_dElevator < -m_dAirTurnFactor) {
          m_vehicle.m_airplane.m_dElevator += m_dAirTurnFactor;
        } else {
          m_vehicle.m_airplane.m_dElevator = 0.0;
        }
      }
      if(m_vehicle.m_bPropeller) {
        if(m_vehicle.m_dPropellerFactor < 1.0) {
          m_vehicle.m_dPropellerFactor += m_dPropThrustFactor;
        }
      } else if(m_vehicle.m_bPropReverse) {
        if(m_vehicle.m_dPropellerFactor > -0.5) {
          m_vehicle.m_dPropellerFactor -= m_dPropThrustFactor;
        }
      } else {
        // Return prop to idle
        if(m_vehicle.m_dPropellerFactor > (m_dPropThrustFactor * 2.0)) {
          m_vehicle.m_dPropellerFactor -= m_dPropThrustFactor;
        } else if(m_vehicle.m_dPropellerFactor < -(m_dPropThrustFactor * 2.0)) {
          m_vehicle.m_dPropellerFactor += m_dPropThrustFactor;
        }
      }
      if(m_vehicle.m_dKerosine <= 0.0) {
        m_vehicle.m_dPropellerFactor = 0.0;
      }
    }
  }
} 



//*****************************************************************************
void BSimulation::MoveCarPoints() {
  for(int i = 0; i < m_vehicle.m_nBodyPoints; ++i) {
    m_vehicle.m_pBodyPoint[i].m_vLocation += (m_vehicle.m_pBodyPoint[i].m_vector * m_dPhysicsFraction);
  }
} 



//*****************************************************************************
double BSimulation::PointInsideObject(BVector& rvPoint, BVector& rvNormal, double &rdFriction, double &rdBaseDepth) {
  double depth;
  for(int o = 0; o < m_pCenterBlock->m_nObjects; ++o) {
    if((depth = m_pCenterBlock->m_objectArray[o]->PointIsInsideObject(rvPoint, rvNormal, rdFriction, rdBaseDepth)) > 0.0) {
      return depth;
    }
  }
  return -1.0;
}






//*****************************************************************************
BTerrainBlock *BSimulation::FindTerrainBlock(BVector &rvPoint) {
  // Find the block where the point lies.
  BTerrainBlock *pCenter = m_terrain.m_ringVisible.GetHead();
  if(pCenter->PointIsInsideBlock(rvPoint)) {
    return pCenter;
  } else {
    // loop through the list to find the correct block
    BTerrainBlock *pRet = pCenter->m_pNext;
    while(pRet != pCenter) {
      if(pRet->PointIsInsideBlock(rvPoint)) {
        return pRet;
      }
      pRet = pRet->m_pNext;
    }
  }
  return 0;
}





//*****************************************************************************
double BSimulation::PointUnderGround(BVector vPoint, 
                                     BVector& rvNormal, 
                                     double &rdFriction, 
                                     double &rdBaseDepth, 
                                     double &rdThermoLoss) {
  // Find corresponding triangle and check depth from there

  BTerrainBlock *pBlock = FindTerrainBlock(vPoint);
  if(pBlock) {
    vPoint.m_dX -= pBlock->m_vCorner.m_dX;
    vPoint.m_dY -= pBlock->m_vCorner.m_dY;

    static double dX, dY, dXRes, dYRes;
    double dTileSize = pBlock->m_dTileSize;
    int    nStep     = pBlock->m_nStep;
    int    nTiles    = pBlock->m_nSize - 1;
    dX = vPoint.m_dX / dTileSize;
    dY = vPoint.m_dY / dTileSize;

    // Square has been found. Check which triangle to use
    int x = int(dX);
    int y = int(dY);
    dXRes = dX - double(x);
    dYRes = dY - double(y);
    if(dXRes > dYRes) {
      // Check against triangle 1
      rvNormal = pBlock->m_pTiles[y * (nTiles / nStep) + x].m_vNormal1;
      // calculate depth
      double d1 = -pBlock->HeightAt(x       * nStep, y       * nStep);
      double d2 = -pBlock->HeightAt((x + 1) * nStep, y       * nStep);
      double d4 = -pBlock->HeightAt((x + 1) * nStep, (y + 1) * nStep);
      double dRet = vPoint.m_dZ - (d1 + dXRes * (d2 - d1) + dYRes * (d4 - d2));
      rdFriction = pBlock->m_pTiles[y * (nTiles / nStep) + x].m_dFriction;
      rdBaseDepth = rvNormal.ScalarProduct(BVector(0, 0, -dRet)) - g_dEarthQuake;
      if(vPoint.m_dZ > 0.0) {
        if(rdBaseDepth < 0.0) {
          // Under water
          rvNormal.Set(0, 0, -1);
          rdFriction = 0.0003;
          rdThermoLoss = 0.0;
        }
        return vPoint.m_dZ - g_dEarthQuake;
      } else {
        return rdBaseDepth;
      }
    } else {
      // Check against triangle 2
      rvNormal = pBlock->m_pTiles[y * (nTiles / nStep) + x].m_vNormal2;
      // calculate depth
      double d1 = -pBlock->HeightAt(x       * nStep, y       * nStep);
      double d3 = -pBlock->HeightAt(x       * nStep, (y + 1) * nStep);
      double d4 = -pBlock->HeightAt((x + 1) * nStep, (y + 1) * nStep);
      double dRet = vPoint.m_dZ - (d1 + dYRes * (d3 - d1) + dXRes * (d4 - d3));
      rdFriction = pBlock->m_pTiles[y * (nTiles / nStep) + x].m_dFriction;
      rdBaseDepth = rvNormal.ScalarProduct(BVector(0, 0, -dRet)) - g_dEarthQuake;
      if(vPoint.m_dZ > 0.0) {
        if(rdBaseDepth < 0.0) {
          // Under water
          rvNormal.Set(0, 0, -1);
          rdFriction = 0.0003;
          rdThermoLoss = 0.0;
        }
        return vPoint.m_dZ - g_dEarthQuake;
      } else {
        return rdBaseDepth;
      }
    }
  } else {
    // Something's wrong. Correct block was not found. 
    //ASSERT(FALSE);
    rdFriction = 0.5; 
    rdBaseDepth = vPoint.m_dZ - g_dEarthQuake;
    rvNormal.Set(0.0, 0.0, -1.0);
    return vPoint.m_dZ - g_dEarthQuake;
  }
}



//*****************************************************************************
double BSimulation::PointUnderGroundShadow(BVector vPoint, BVector& rvNormal) {
  // Find corresponding triangle and check depth from there
  BVector vObjectTest = vPoint;
  double dRet = vPoint.m_dZ;
  rvNormal.Set(0, 0, -1);

  BTerrainBlock *pBlock = FindTerrainBlock(vPoint);
  if(pBlock) {
    vPoint.m_dX -= pBlock->m_vCorner.m_dX;
    vPoint.m_dY -= pBlock->m_vCorner.m_dY;

    static double dX, dY, dXRes, dYRes;
    double dTileSize = pBlock->m_dTileSize;
    int    nStep     = pBlock->m_nStep;
    int    nTiles    = pBlock->m_nSize - 1;
    dX = vPoint.m_dX / dTileSize;
    dY = vPoint.m_dY / dTileSize;

    // Square has been found. Check which triangle to use
    int x = int(dX);
    int y = int(dY);
    dXRes = dX - double(x);
    dYRes = dY - double(y);
    if(dXRes > dYRes) {
      // Check against triangle 1
      rvNormal = pBlock->m_pTiles[y * (nTiles / nStep) + x].m_vNormal1;
      // calculate depth
      double d1 = -pBlock->HeightAt(x       * nStep, y       * nStep);
      double d2 = -pBlock->HeightAt((x + 1) * nStep, y       * nStep);
      double d4 = -pBlock->HeightAt((x + 1) * nStep, (y + 1) * nStep);
      dRet = vPoint.m_dZ - (d1 + dXRes * (d2 - d1) + dYRes * (d4 - d2));
    } else {
      // Check against triangle 2
      rvNormal = pBlock->m_pTiles[y * (nTiles / nStep) + x].m_vNormal2;
      // calculate depth
      double d1 = -pBlock->HeightAt(x       * nStep, y       * nStep);
      double d3 = -pBlock->HeightAt(x       * nStep, (y + 1) * nStep);
      double d4 = -pBlock->HeightAt((x + 1) * nStep, (y + 1) * nStep);
      dRet = vPoint.m_dZ - (d1 + dYRes * (d3 - d1) + dXRes * (d4 - d3));
    }
  } else {
    // Something's wrong. Correct block was not found. 
    //ASSERT(FALSE);
    rvNormal.Set(0.0, 0.0, -1.0);
    return vPoint.m_dZ;
  }

  // Check for water surface shadow
  if((vPoint.m_dZ < 2.0) && ((vPoint.m_dZ - dRet) > 0.01)) {
    dRet = vPoint.m_dZ;
  }

  // check if there's an object that the shadow should be cast on
  bool bChanged = false;
  double depth = PointUnderObjectsShadow(dRet, vObjectTest, rvNormal, bChanged);

  return depth;
}

//*****************************************************************************
double BSimulation::PointUnderObjectsShadow(double& rdCandidate, BVector& rvPoint, BVector& rvNormal, bool& rbChanged) {
  BObject *pCastObj;
  rbChanged = false;
  static BVector vUp(0, 0, -1);
  double dSmallestDepth = 999999.9;
  int nSelFace = -1;
  for(int o = 0; o < m_pCenterBlock->m_nObjects; ++o) {
    BObject *pObj = m_pCenterBlock->m_objectArray[o];
    if((rvPoint - pObj->m_vCenter).Length() < pObj->m_dRadius) {
      for(int nPart = 0; nPart < pObj->m_nCollDetParts; ++nPart) {
        for(int nFace = 0; nFace < pObj->m_pCollDetPart[nPart].m_nFaces; ++nFace) {
          if(pObj->m_pCollDetPart[nPart].m_pFace[nFace].m_vNormal.m_dZ < -0.01) { // Up facing surface
            double depth = pObj->m_pCollDetPart[nPart].m_pFace[nFace].m_vNormal.ScalarProduct(rvPoint - (pObj->m_pCollDetPart[nPart].m_pFace[nFace].m_vPoint[0] + pObj->m_vLocation));
            // double dProjDepth = (pObj->m_pCollDetPart[nPart].m_pFace[nFace].m_vNormal * depth).ScalarProduct(vUp);
            double dProjDepth = depth;
            if((dProjDepth > -0.5) && (fabs(dProjDepth) < fabs(dSmallestDepth))) {
              nSelFace = nFace;
              dSmallestDepth = dProjDepth;
              pCastObj = pObj;
              rvNormal = pObj->m_pCollDetPart[nPart].m_pFace[nFace].m_vNormal;
            }
          }
        }
      }
    }
  }
  
  if((fabs(dSmallestDepth) < fabs(rdCandidate)) && 
     !pCastObj->m_boundary.PointIsOutside(rvPoint.m_dX, rvPoint.m_dY)) {
    rbChanged = true;
    return -dSmallestDepth;
  } else {
    return rdCandidate;
  }
}


//*****************************************************************************
double BSimulation::Friction(BVector& rPoint) {
  return 0.5; // NOT READY Use correct ground friction!
}

//*****************************************************************************
double BSimulation::Friction(BBodyPoint& rPoint) {
  return rPoint.m_dFriction * 0.5; // NOT READY Use correct ground friction!
}




//*****************************************************************************
// CAMERA SETUP
//*****************************************************************************


extern double g_dRate;


//*****************************************************************************
//void BSimulation::SetUpCamera(CRect *pRect) {
void BSimulation::SetUpCamera() {
  // Always point towards the camera, always try to stay behind car
  static int nMode = 0;
  const static double cdTransitionSpeedMax = 0.15;
  static double dTransitionSpeedMax = cdTransitionSpeedMax;
  static double dTransitionSpeed = 0.0;
  BVector vUp(0, 0, -1);
  BVector vForward;

  BVector location = m_camera.m_vLocation;
  if(m_camera.m_locMode == BCamera::FIXED) {
    m_camera.m_vLocation = m_camera.m_vFixLocation;
    vForward = (m_vehicle.m_vLocation - m_camera.m_vLocation);
    if(dTransitionSpeedMax != cdTransitionSpeedMax) {
      dTransitionSpeed = 0.0;
      dTransitionSpeedMax = cdTransitionSpeedMax;
    }
  } else {
    if(m_camera.m_locMode == BCamera::FOLLOW) {
      if(((m_vehicle.m_dSpeed) < (m_vehicle.m_bHasAirplaneControls ? 0.15 : 0.03)) || 
         (m_vehicle.m_rotor.m_nHeliMode)) {

        if(m_vehicle.m_rotor.m_nHeliMode) {
          dTransitionSpeedMax = cdTransitionSpeedMax * 2.0;
        }

        // If stopped or in heli mode, place camera behind car
        if(nMode != 1) {
          dTransitionSpeed = 0.0;
        }
        nMode = 1;
        // vTowardsBack += m_vehicle.m_orientation.m_vRight * -2.0;
        m_camera.m_vLocation = m_vehicle.m_vLocation + m_vehicle.m_orientation.m_vForward * -6.0;
        m_camera.m_vLocation.m_dZ = m_vehicle.m_vLocation.m_dZ + m_camera.m_dFollowHeight;
      } else {
        if(nMode != 2) {
          dTransitionSpeed = 0.0;
        }
        nMode = 2;
        // If moving, place camera behind car, on car's trail
        BVector vSpeed = m_vehicle.m_vector;
        vSpeed.ToUnitLength();
        m_camera.m_vLocation = m_vehicle.m_vLocation + vSpeed * -6.0;
        m_camera.m_vLocation.m_dZ = m_camera.m_vLocation.m_dZ + m_camera.m_dFollowHeight;
      }
      vForward = (m_vehicle.m_vLocation + m_vehicle.m_vector * 10.0 - m_camera.m_vLocation);
      if((dTransitionSpeedMax != cdTransitionSpeedMax) && (dTransitionSpeedMax != (cdTransitionSpeedMax * 2.0))) {
        dTransitionSpeed = 0.0;
        dTransitionSpeedMax = cdTransitionSpeedMax;
      }
    } else if(m_camera.m_locMode == BCamera::OVERVIEW) {
      // Place camera high above
      if(m_camera.m_bInitLoc) {
        m_camera.m_bInitLoc = false;
        BVector vTowardsBack = m_vehicle.m_orientation.m_vForward * -1.0;
        vTowardsBack.m_dZ = 0.0;
        vTowardsBack.ToUnitLength();
        m_camera.m_vOverview = vTowardsBack * 60.0 + BVector(0, 0, -40.0);
      }
      m_camera.m_vLocation = m_vehicle.m_vLocation + m_camera.m_vOverview;
      vForward = (m_vehicle.m_vLocation - m_camera.m_vLocation);
      if(dTransitionSpeedMax != cdTransitionSpeedMax) {
        dTransitionSpeed = 0.0;
        dTransitionSpeedMax = cdTransitionSpeedMax;
      }
    } else if(m_camera.m_locMode == BCamera::INCAR) {
      // Place camera over hood
      BVector vLoc = m_vehicle.m_vLocation + m_vehicle.m_orientation.m_vUp;
      m_camera.m_vLocation = vLoc;
      vUp      = m_vehicle.m_orientation.m_vUp;
      vForward = m_vehicle.m_orientation.m_vForward;
      dTransitionSpeedMax = 1.0;
      dTransitionSpeed = 1.0;
    } else if(m_camera.m_locMode == BCamera::ONSIDE) {
      // Place camera closely on left side of car
      BVector vLoc = m_vehicle.m_vLocation - m_vehicle.m_orientation.m_vRight * 1.5;
      m_camera.m_vLocation = vLoc;
      vUp = m_vehicle.m_orientation.m_vUp;
      vForward = m_vehicle.m_orientation.m_vForward + vUp * 0.2;
      vForward.ToUnitLength();
      dTransitionSpeedMax = 1.0;
      dTransitionSpeed = 1.0;
    } else if(m_camera.m_locMode == BCamera::SIDEVIEW) {
      // Place camera on left side of car
      BVector vLoc = m_vehicle.m_vLocation - m_vehicle.m_orientation.m_vRight * 6.0 + m_vehicle.m_orientation.m_vUp * 2.0;
      m_camera.m_vLocation = vLoc;
      vUp = BVector(0, 0, -1);
      vForward = m_vehicle.m_vLocation - vLoc;
      vForward.ToUnitLength();
      dTransitionSpeedMax = 1.0;
      dTransitionSpeed = 1.0;
    }    
  }

  double dCalibXSpeed = dTransitionSpeed; // pow(dTransitionSpeed, 1 / (g_dRate / 60.0));

  /*
  double dCalibXSpeed2 = 1.0;

  GLdouble modelMatrix[16];
  GLdouble projMatrix[16];
  GLint    viewport[4];
  BVector  vCarOnScreen;

  glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);
  glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);
  glGetIntegerv(GL_VIEWPORT, viewport);

  if(gluProject(m_vehicle.m_vLocation.m_dX, 
                m_vehicle.m_vLocation.m_dY, 
                m_vehicle.m_vLocation.m_dZ,
                modelMatrix,
                projMatrix,
                viewport,
                &(vCarOnScreen.m_dX),
                &(vCarOnScreen.m_dY),
                &(vCarOnScreen.m_dZ)) && pRect) {
    double dFactorY = fabs(vCarOnScreen.m_dY - (pRect->Height() / 2.0)) / (pRect->Height() / 2.0);
    double dFactorX = fabs(vCarOnScreen.m_dX - (pRect->Width() / 2.0)) / (pRect->Width() / 2.0);
    if(dFactorY > 1.0) {
      dFactorY = 1.0;
    }
    if(dFactorX > 1.0) {
      dFactorX = 1.0;
    }
    dFactorY *= dFactorY;
    dFactorX *= dFactorX;
    double dFactor = dFactorX * dFactorY * 0.1;

    dCalibXSpeed = (dCalibXSpeed * (1.0 - dFactor)) + (dCalibXSpeed2 * dFactor);
  }
  */

  m_camera.m_vLocation = location * (1.0 - dCalibXSpeed) + 
                         m_camera.m_vLocation * dCalibXSpeed;

  if(dTransitionSpeed < dTransitionSpeedMax) {
    dTransitionSpeed += 0.01;
  }

  BOrientation orientation = m_camera.m_orientation;
  m_camera.m_orientation.m_vForward = vForward;
  m_camera.m_orientation.m_vUp = vUp;
  m_camera.m_orientation.m_vRight = m_camera.m_orientation.m_vForward.CrossProduct(m_camera.m_orientation.m_vUp);
  m_camera.m_orientation.Normalize();

  if(m_camera.m_locMode == BCamera::INCAR) {
    m_camera.m_orientation.m_vForward.m_dX = m_camera.m_orientation.m_vForward.m_dX * 0.9 + orientation.m_vForward.m_dX * (1.0 - 0.9);
    m_camera.m_orientation.m_vUp.m_dX      = m_camera.m_orientation.m_vUp.m_dX      * 0.9 + orientation.m_vUp.m_dX      * (1.0 - 0.9);
    m_camera.m_orientation.m_vRight.m_dX   = m_camera.m_orientation.m_vRight.m_dX   * 0.9 + orientation.m_vRight.m_dX   * (1.0 - 0.9);

    m_camera.m_orientation.m_vForward.m_dY = m_camera.m_orientation.m_vForward.m_dY * 0.9 + orientation.m_vForward.m_dY * (1.0 - 0.9);
    m_camera.m_orientation.m_vUp.m_dY      = m_camera.m_orientation.m_vUp.m_dY      * 0.9 + orientation.m_vUp.m_dY      * (1.0 - 0.9);
    m_camera.m_orientation.m_vRight.m_dY   = m_camera.m_orientation.m_vRight.m_dY   * 0.9 + orientation.m_vRight.m_dY   * (1.0 - 0.9);

    m_camera.m_orientation.m_vForward.m_dZ = m_camera.m_orientation.m_vForward.m_dZ * 0.2 + orientation.m_vForward.m_dZ * (1.0 - 0.2);
    m_camera.m_orientation.m_vUp.m_dZ      = m_camera.m_orientation.m_vUp.m_dZ      * 0.2 + orientation.m_vUp.m_dZ      * (1.0 - 0.2);
    m_camera.m_orientation.m_vRight.m_dZ   = m_camera.m_orientation.m_vRight.m_dZ   * 0.2 + orientation.m_vRight.m_dZ   * (1.0 - 0.2);
    
    //m_camera.m_orientation.m_vForward = m_camera.m_orientation.m_vForward * 0.4 + orientation.m_vForward * 0.6;
    //m_camera.m_orientation.m_vUp      = m_camera.m_orientation.m_vUp      * 0.4 + orientation.m_vUp      * 0.6;
    //m_camera.m_orientation.m_vRight   = m_camera.m_orientation.m_vRight   * 0.4 + orientation.m_vRight   * 0.6;
  } else {
    m_camera.m_orientation.m_vForward.m_dX = m_camera.m_orientation.m_vForward.m_dX * dTransitionSpeed * 0.9 + orientation.m_vForward.m_dX * (1.0 - dTransitionSpeed * 0.9);
    m_camera.m_orientation.m_vUp.m_dX      = m_camera.m_orientation.m_vUp.m_dX      * dTransitionSpeed * 0.9 + orientation.m_vUp.m_dX      * (1.0 - dTransitionSpeed * 0.9);
    m_camera.m_orientation.m_vRight.m_dX   = m_camera.m_orientation.m_vRight.m_dX   * dTransitionSpeed * 0.9 + orientation.m_vRight.m_dX   * (1.0 - dTransitionSpeed * 0.9);

    m_camera.m_orientation.m_vForward.m_dY = m_camera.m_orientation.m_vForward.m_dY * dTransitionSpeed * 0.9 + orientation.m_vForward.m_dY * (1.0 - dTransitionSpeed * 0.9);
    m_camera.m_orientation.m_vUp.m_dY      = m_camera.m_orientation.m_vUp.m_dY      * dTransitionSpeed * 0.9 + orientation.m_vUp.m_dY      * (1.0 - dTransitionSpeed * 0.9);
    m_camera.m_orientation.m_vRight.m_dY   = m_camera.m_orientation.m_vRight.m_dY   * dTransitionSpeed * 0.9 + orientation.m_vRight.m_dY   * (1.0 - dTransitionSpeed * 0.9);

    m_camera.m_orientation.m_vForward.m_dZ = m_camera.m_orientation.m_vForward.m_dZ * dTransitionSpeed * 0.2 + orientation.m_vForward.m_dZ * (1.0 - dTransitionSpeed * 0.2);
    m_camera.m_orientation.m_vUp.m_dZ      = m_camera.m_orientation.m_vUp.m_dZ      * dTransitionSpeed * 0.2 + orientation.m_vUp.m_dZ      * (1.0 - dTransitionSpeed * 0.2);
    m_camera.m_orientation.m_vRight.m_dZ   = m_camera.m_orientation.m_vRight.m_dZ   * dTransitionSpeed * 0.2 + orientation.m_vRight.m_dZ   * (1.0 - dTransitionSpeed * 0.2);
  }
  m_camera.m_orientation.Normalize();
}




//*****************************************************************************
// EFFECTS: TRAILS
//*****************************************************************************


//*****************************************************************************
void BSimulation::UpdateTrails() {
  bool bSkidding = false;
  for(int w = 0; w < m_vehicle.m_nWheels; ++w) {
    BWheel *pWheel = dynamic_cast<BWheel *>(m_vehicle.m_pWheel[w]);
    if(m_vehicle.m_dSpeed > 0.01) {
      if(pWheel->m_nGroundHits > (m_nPhysicsStepsBetweenRender / 2)) { // 5 = 15/3, 15 = number of updates between visualization
        bSkidding = true;
        int nPrev = pWheel->m_nTrailHead;

        // Insert new trail point
        if(pWheel->m_nTrailPoints < g_cnMaxTrailPoints) {
          ++pWheel->m_nTrailPoints;
        }
        pWheel->m_nTrailHead = (pWheel->m_nTrailHead + 1) % g_cnMaxTrailPoints;
        BVector vNormal, vNormalObj;
        double dTmp, dBaseDepth;
        double depth = PointUnderGround(pWheel->m_vLocSample, vNormal, dTmp, dBaseDepth, dTmp);
        bool bChanged = false;
        depth = PointUnderObjectsShadow(depth, pWheel->m_vLocSample, vNormalObj, bChanged);
        BVector vLoc;
        if(bChanged) {
          vLoc = pWheel->m_vLocSample + vNormalObj * depth;
        } else {
          vLoc = pWheel->m_vLocSample + vNormal * dBaseDepth;
        }
        BVector v1 = vLoc + pWheel->m_orientation.m_vRight * -(pWheel->m_dWidth / 2.0);
        BVector v2 = vLoc + pWheel->m_orientation.m_vRight * (pWheel->m_dWidth / 2.0);
        double d;
        d = PointUnderGround(v1, vNormal, dTmp, dBaseDepth, dTmp);
        d = PointUnderObjectsShadow(d, v1, vNormalObj, bChanged);
        if(bChanged) {
          v1 += vNormalObj * d;
        } else {
          v1 += vNormal * dBaseDepth;
        }
        d = PointUnderGround(v2, vNormal, dTmp, dBaseDepth, dTmp);
        d = PointUnderObjectsShadow(d, v2, vNormalObj, bChanged);
        if(bChanged) {
          v2 += vNormalObj * d;
        } else {
          v2 += vNormal * dBaseDepth;
        }
        pWheel->m_pTrailPoint[pWheel->m_nTrailHead].m_vLocation[0] = v1;
        pWheel->m_pTrailPoint[pWheel->m_nTrailHead].m_vLocation[1] = v2;
        pWheel->m_pTrailPoint[pWheel->m_nTrailHead].m_bStart = false;
        pWheel->m_pTrailPoint[pWheel->m_nTrailHead].m_bEnd   = false;
        pWheel->m_pTrailPoint[pWheel->m_nTrailHead].m_dStrength = double(pWheel->m_nGroundHits) / m_nPhysicsSteps;

        if(pWheel->m_nTrailPoints > 1) {
          if(pWheel->m_pTrailPoint[nPrev].m_bEnd) {
            pWheel->m_pTrailPoint[pWheel->m_nTrailHead].m_bStart = true;
          }
        }

      } else {
        if(pWheel->m_nTrailPoints) {
          // Mark end of this trail
          pWheel->m_pTrailPoint[pWheel->m_nTrailHead].m_bEnd = true;
        }
      }      
    } else {
      if(pWheel->m_nTrailPoints) {
        // Mark end of this trail
        pWheel->m_pTrailPoint[pWheel->m_nTrailHead].m_bEnd = true;
      }
    }
    pWheel->m_nGroundHits = 0;
  }

  // Update skid sound volume
  if(bSkidding) {
    if(m_vehicle.m_dSpeedKmh < 400.0) {
      SoundModule::SetSkidSoundVolume(int(m_vehicle.m_dSpeedKmh / 4.0));
    } else {
      SoundModule::SetSkidSoundVolume(100);
    }
  } else {
    SoundModule::SetSkidSoundVolume(0);
  }
}



//*****************************************************************************
void BSimulation::UpdateDustClouds() {

  double dTimeRatio = double(m_nPhysicsStepsBetweenRender) / 11.0;
  double dTimeRatio2 = 1.0;
  if(BGame::m_bSlowMotion) {
    dTimeRatio = 0.3;
    dTimeRatio2 = 0.15;
  }

  // Create new clouds
  static int nSkipper = 0;
  if(!BGame::m_bSlowMotion || ((++nSkipper) % 2)) {
    for(int w = 0; w < m_vehicle.m_nWheels; ++w) {
      BWheel *pWheel = dynamic_cast<BWheel *>(m_vehicle.m_pWheel[w]);
      if(m_vehicle.m_dSpeed > 0.1) {
        if(pWheel->m_bTouchesGround) {
          // Generate dust cloud at wheel position
          BVector vHitPoint = pWheel->m_vLocation + BVector(0, 0, pWheel->m_dDepth);
          CreateDustCloudAt(vHitPoint, 1, -1.0, BVector(0, 0, 0), pWheel->m_dRadius);
        }
      }
      // Prepare for next render frame
      pWheel->m_bTouchesGround = false;
    }
  }

  // Update all clouds
  BVector vWind = BGame::m_vWindDirection * BGame::m_dWindStrength * m_nPhysicsStepsBetweenRender;
  int i;
  for(i = 0; i < m_nDustClouds1; ++i) {
    m_dustClouds1[i].m_dAlpha -= 0.005 * dTimeRatio;
    if(m_dustClouds1[i].m_dAlpha < 0.0) {
      m_dustClouds1[i].m_dAlpha = 0.0;
    }
    m_dustClouds1[i].m_dSize += 0.05 * dTimeRatio;
    if(m_dustClouds1[i].m_dSize > 10.0) {
      m_dustClouds1[i].m_dSize = 10.0;
    }
    m_dustClouds1[i].m_vLocation += m_dustClouds1[i].m_vector * dTimeRatio2 + vWind * (m_dustClouds1[i].m_dSize / 2.0);
    double dTmp = m_dustClouds1[i].m_vector.m_dZ;
    m_dustClouds1[i].m_vector = m_dustClouds1[i].m_vector * 0.999 + RandomVector(0.03) * dTimeRatio2;
    m_dustClouds1[i].m_vector.m_dZ = dTmp;
  }
  for(i = 0; i < m_nDustClouds2; ++i) {
    m_dustClouds2[i].m_dAlpha *= 0.9;
    if(m_dustClouds2[i].m_dAlpha < 0.0) {
      m_dustClouds2[i].m_dAlpha = 0.0;
    }
    m_dustClouds2[i].m_dSize += 0.1 * dTimeRatio;
    if(m_dustClouds2[i].m_dSize > 10.0) {
      m_dustClouds2[i].m_dSize = 10.0;
    }
    m_dustClouds2[i].m_vLocation += m_dustClouds2[i].m_vector * dTimeRatio2 + vWind * (m_dustClouds2[i].m_dSize / 2.0);
    double dTmp = m_dustClouds2[i].m_vector.m_dZ;
    m_dustClouds2[i].m_vector = m_dustClouds2[i].m_vector * 0.999 + RandomVector(0.02) * dTimeRatio2;
    m_dustClouds2[i].m_vector.m_dZ = dTmp;
  }
  for(i = 0; i < m_nDustClouds3; ++i) {
    m_dustClouds3[i].m_dAlpha -= 0.03 * dTimeRatio;
    if(m_dustClouds3[i].m_dAlpha < 0.0) {
      m_dustClouds3[i].m_dAlpha = 0.0;
    }
    m_dustClouds3[i].m_dSize += 0.1 * dTimeRatio;
    if(m_dustClouds3[i].m_dSize > 10.0) {
      m_dustClouds3[i].m_dSize = 10.0;
    }
    m_dustClouds3[i].m_vLocation += m_dustClouds3[i].m_vector * dTimeRatio2 + vWind * (m_dustClouds3[i].m_dSize / 2.0);
    double dTmp = m_dustClouds3[i].m_vector.m_dZ;
    m_dustClouds3[i].m_vector = m_dustClouds3[i].m_vector * 0.999 + RandomVector(0.02) * dTimeRatio2;
    m_dustClouds3[i].m_vector.m_dZ = dTmp;
  }
}


//*****************************************************************************
void BSimulation::CreateDustCloudAt(BVector vHitPoint, int nSlot, double dInitialAlpha, BVector vInitial, double dOffset) {

  vHitPoint.m_dZ -= dOffset;

  double dR, dG, dB;
  BTerrain::GetColorForHeight(-vHitPoint.m_dZ, dR, dG, dB);
  dR = dR + (1.0 - dR) / 2.0 - 0.3;
  dG = dG + (1.0 - dG) / 2.0 - 0.3;
  dB = dB + (1.0 - dB) / 2.0 - 0.3;
  if(nSlot == 1) {
    m_nDustCloudsHead1 = (m_nDustCloudsHead1 + 1) % g_cnDustClouds1;
    m_dustClouds1[m_nDustCloudsHead1].m_dAlpha = 0.1 + 0.7 * (m_vehicle.m_dSpeed - 0.1) * 5.0;
    m_dustClouds1[m_nDustCloudsHead1].m_dSize = 0.1;
    m_dustClouds1[m_nDustCloudsHead1].m_vLocation = vHitPoint;
    m_dustClouds1[m_nDustCloudsHead1].m_vector = m_vehicle.m_vector * 0.95 + BVector(0, 0, -0.01);
    m_dustClouds1[m_nDustCloudsHead1].m_color.m_dX = dR;
    m_dustClouds1[m_nDustCloudsHead1].m_color.m_dY = dG;
    m_dustClouds1[m_nDustCloudsHead1].m_color.m_dZ = dB;
    ++m_nDustClouds1;
    if(m_nDustClouds1 > g_cnDustClouds1) {
      m_nDustClouds1 = g_cnDustClouds1;
    }
  } else if(nSlot == 2) {
    static int nSkipper = 0;
    if(!(++nSkipper % 3)) {
      m_nDustCloudsHead2 = (m_nDustCloudsHead2 + 1) % g_cnDustClouds2;
      m_dustClouds2[m_nDustCloudsHead2].m_dAlpha = 0.1 + 0.7 * (m_vehicle.m_dSpeed - 0.1) * 5.0;
      m_dustClouds2[m_nDustCloudsHead2].m_dSize = 0.1;
      m_dustClouds2[m_nDustCloudsHead2].m_vLocation = vHitPoint;
      m_dustClouds2[m_nDustCloudsHead2].m_vector = m_vehicle.m_vector * 0.95 + BVector(0, 0, -0.01);
      m_dustClouds2[m_nDustCloudsHead2].m_color.m_dX = dR;
      m_dustClouds2[m_nDustCloudsHead2].m_color.m_dY = dG;
      m_dustClouds2[m_nDustCloudsHead2].m_color.m_dZ = dB;
      ++m_nDustClouds2;
      if(m_nDustClouds2 > g_cnDustClouds2) {
        m_nDustClouds2 = g_cnDustClouds2;
      }
    }
  } else {
    m_nDustCloudsHead3 = (m_nDustCloudsHead3 + 1) % g_cnDustClouds3;
    m_dustClouds3[m_nDustCloudsHead3].m_dAlpha = dInitialAlpha;
    m_dustClouds3[m_nDustCloudsHead3].m_dSize = 0.1;
    m_dustClouds3[m_nDustCloudsHead3].m_vLocation = vHitPoint;
    m_dustClouds3[m_nDustCloudsHead3].m_vector = vInitial;
    m_dustClouds3[m_nDustCloudsHead3].m_color.m_dX = dR;
    m_dustClouds3[m_nDustCloudsHead3].m_color.m_dY = dG;
    m_dustClouds3[m_nDustCloudsHead3].m_color.m_dZ = dB;
    ++m_nDustClouds3;
    if(m_nDustClouds3 > g_cnDustClouds3) {
      m_nDustClouds3 = g_cnDustClouds3;
    }
  }
} 


//*****************************************************************************
void BSimulation::DrawShadowAndTrails() {
  static BVector vShadow[5];
  static int i;

  glDisable(GL_CULL_FACE);
  // Shadow
  //if(m_camera.m_locMode != BCamera::TCameraLoc::INCAR) {
  if(m_camera.m_locMode != BCamera::INCAR) {
    OpenGLHelpers::SetColorFull(1, 1, 1, 1);

    vShadow[0] = m_vehicle.m_vLocation + m_vehicle.m_orientation.m_vRight * -(m_vehicle.m_dVisualWidth / 2.0) + m_vehicle.m_orientation.m_vForward *  (m_vehicle.m_dVisualLength / 2.0);
    vShadow[1] = m_vehicle.m_vLocation + m_vehicle.m_orientation.m_vRight * -(m_vehicle.m_dVisualWidth / 2.0) + m_vehicle.m_orientation.m_vForward * -(m_vehicle.m_dVisualLength / 2.0);
    vShadow[2] = m_vehicle.m_vLocation + m_vehicle.m_orientation.m_vRight *  (m_vehicle.m_dVisualWidth / 2.0) + m_vehicle.m_orientation.m_vForward *  (m_vehicle.m_dVisualLength / 2.0);
    vShadow[3] = m_vehicle.m_vLocation + m_vehicle.m_orientation.m_vRight *  (m_vehicle.m_dVisualWidth / 2.0) + m_vehicle.m_orientation.m_vForward * -(m_vehicle.m_dVisualLength / 2.0);
    for(i = 0; i < 4; ++i) {
      vShadow[i].m_dZ = vShadow[i].m_dZ - PointUnderGroundShadow(vShadow[i], vShadow[4]);
    }
    BVector vCenter = (vShadow[0] + vShadow[1] + vShadow[2] + vShadow[3]) * 0.25;
    vShadow[0] = (vShadow[0] - vCenter) * 1.3 + vCenter;
    vShadow[1] = (vShadow[1] - vCenter) * 1.3 + vCenter;
    vShadow[2] = (vShadow[2] - vCenter) * 1.3 + vCenter;
    vShadow[3] = (vShadow[3] - vCenter) * 1.3 + vCenter;

    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0.0, 0.0);
    glVertex3f(vShadow[0].m_dX, vShadow[0].m_dY, vShadow[0].m_dZ);
    glTexCoord2f(0.0, 0.5);
    glVertex3f(vShadow[1].m_dX, vShadow[1].m_dY, vShadow[1].m_dZ);
    glTexCoord2f(0.5, 0.0);
    glVertex3f(vShadow[2].m_dX, vShadow[2].m_dY, vShadow[2].m_dZ);
    glTexCoord2f(0.5, 0.5);
    glVertex3f(vShadow[3].m_dX, vShadow[3].m_dY, vShadow[3].m_dZ);
    glEnd();  
  }

  // Trails
  DrawTrails();
  glEnable(GL_CULL_FACE);
}

//*****************************************************************************
void BSimulation::DrawTrails() {
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  static BVector vTrail[4];
  // Loop through all wheels and draw their trails
  bool bBeginDone = false;
  for(int w = 0; w < m_vehicle.m_nWheels; ++w) {
    BWheel *pWheel = dynamic_cast<BWheel *>(m_vehicle.m_pWheel[w]);
    for(int i = pWheel->m_nTrailPoints - 1; i >= 0; --i) {
      int nTrailPoint = ((pWheel->m_nTrailHead + g_cnMaxTrailPoints) - i) % g_cnMaxTrailPoints;

      if(pWheel->m_pTrailPoint[nTrailPoint].m_bStart) {
        OpenGLHelpers::SetColorFull(1, 1, 1, 0);
      } else if(pWheel->m_pTrailPoint[nTrailPoint].m_bEnd) {
        OpenGLHelpers::SetColorFull(1, 1, 1, 0);
      } else {
        double dColor = pWheel->m_pTrailPoint[nTrailPoint].m_dStrength * 0.75;
        if(dColor > 1.0) {
          dColor = 1.0;
        }
        OpenGLHelpers::SetColorFull(1, 1, 1, dColor);
      }

      if(!bBeginDone) {
        glBegin(GL_TRIANGLE_STRIP);
        bBeginDone = true;
      }

      // Draw rectangle from previous to current
      vTrail[0] = pWheel->m_pTrailPoint[nTrailPoint].m_vLocation[1];
      glTexCoord2f(6.0/16.0, 11.0/16.0);
      glVertex3f(vTrail[0].m_dX, vTrail[0].m_dY, vTrail[0].m_dZ);
      vTrail[1] = pWheel->m_pTrailPoint[nTrailPoint].m_vLocation[0];
      glTexCoord2f(14.0/16.0, 11.0/16.0);
      glVertex3f(vTrail[1].m_dX, vTrail[1].m_dY, vTrail[1].m_dZ);

      if(pWheel->m_pTrailPoint[nTrailPoint].m_bEnd || (i == 0)) {
        glEnd();
        bBeginDone = false;
      }

    }
  }
}




//*****************************************************************************
void BSimulation::DrawSmokeTrails() {
  // OpenGLHelpers::SwitchToTexture(0);
  glDisable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  glEnable(GL_COLOR_MATERIAL);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

  // Draw the smoke trails
  int i;

  for(i = 0; i < m_vehicle.m_nTrailpoints; ++i) {

    glBegin(GL_TRIANGLE_STRIP);
    int nLoop = 0;
    int nEntry = m_vehicle.m_nTrailpointHead;
    while(nLoop < m_vehicle.m_nTrailpointEntries) {
      double dAlphaFactor = double(nLoop) / 100.0;
      double dLen = sqrt(double(100 - nLoop) / 100.0);

      if(m_vehicle.m_trailpointEntry[i][nEntry].m_dAlpha < 0.0) {
        m_vehicle.m_trailpointEntry[i][nEntry].m_dAlpha = 0.0;
      }

      glColor4d(1, 1, 1, 0.0);
      BVector vLoc = m_vehicle.m_trailpointEntry[i][nEntry].m_vLocation - m_vehicle.m_trailpointEntry[i][nEntry].m_vRight * dLen;
      glVertex3f(vLoc.m_dX, vLoc.m_dY, vLoc.m_dZ);
      glColor4d(1, 1, 1, m_vehicle.m_trailpointEntry[i][nEntry].m_dAlpha * dAlphaFactor);      
      vLoc = m_vehicle.m_trailpointEntry[i][nEntry].m_vLocation;
      glVertex3f(vLoc.m_dX, vLoc.m_dY, vLoc.m_dZ);

      ++nLoop;
      nEntry = (nEntry + 1) % 100;
    }
    glEnd();

    glBegin(GL_TRIANGLE_STRIP);
    nLoop = 0;
    nEntry = m_vehicle.m_nTrailpointHead;
    while(nLoop < m_vehicle.m_nTrailpointEntries) {
      double dAlphaFactor = double(nLoop) / 100.0;
      double dLen = sqrt(double(100 - nLoop) / 100.0);
      
      glColor4d(1, 1, 1, m_vehicle.m_trailpointEntry[i][nEntry].m_dAlpha * dAlphaFactor);
      BVector vLoc = m_vehicle.m_trailpointEntry[i][nEntry].m_vLocation;
      glVertex3f(vLoc.m_dX, vLoc.m_dY, vLoc.m_dZ);
      glColor4d(1, 1, 1, 0.0);
      vLoc = m_vehicle.m_trailpointEntry[i][nEntry].m_vLocation + m_vehicle.m_trailpointEntry[i][nEntry].m_vRight * dLen;
      glVertex3f(vLoc.m_dX, vLoc.m_dY, vLoc.m_dZ);
      
      ++nLoop;
      nEntry = (nEntry + 1) % 100;
    }
    glEnd();

  }

  glDisable(GL_BLEND);
  glDisable(GL_COLOR_MATERIAL);

  for(i = 0; i < m_vehicle.m_nTrailpoints; ++i) {
    for(int nLoop = 0; nLoop < 100; ++nLoop) {
      m_vehicle.m_trailpointEntry[i][nLoop].m_dAlpha -= 0.01;
    }
  }
}


double g_dExtraAlpha = 0.0;

//*****************************************************************************
void BSimulation::DrawClouds() {
  OpenGLHelpers::SetColorFull(1, 1, 1, 1);
  OpenGLHelpers::SwitchToTexture(0);
  BTextures::Use(BTextures::CLOUD);
  glDepthMask(GL_FALSE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  g_dExtraAlpha = 0.0;
  int nFurthest;
  double dDist = 99999.9;
  double dDistant = 0;
  int nDrawThis = 0;
  for(int nCloud = 0; nCloud < m_nClouds; ++nCloud) {

    dDistant = 0;
    for(nFurthest = 0; nFurthest < m_nClouds; ++nFurthest) {
      double d = (m_cloud[nFurthest].m_vCenter - m_camera.m_vLocation).Length();
      if((d > dDistant) && 
         (d < dDist)) {
        nDrawThis = nFurthest;
        dDistant = d;
      }
    }
    dDist = dDistant;

    int i, j;
    int    nOrder[10];
    double dDist[10];
    // Determine draw order
    for(i = 0; i < m_cloud[nDrawThis].m_nPuffs; ++i) {
      dDist[i] = -1.0;
    }
    for(i = 0; i < m_cloud[nDrawThis].m_nPuffs; ++i) {
      j = 0;
      BVector vToPuff = m_cloud[nDrawThis].m_puff[i].m_vLocation - m_camera.m_vLocation;
      double dDistance2 = (m_camera.m_vLocation - m_cloud[nDrawThis].m_puff[i].m_vLocation).Length();
      double dDistance = fabs(m_camera.m_orientation.m_vForward.ScalarProduct(vToPuff));
      if(dDistance2 < (m_cloud[nDrawThis].m_puff[i].m_dRadius * 0.9)) {
        g_dExtraAlpha += 1.0 - (dDistance2 / (m_cloud[nDrawThis].m_puff[i].m_dRadius * 0.9));
      }
      while((j <= i) && (dDist[j] > dDistance)) {
        ++j;
      }
      if(j < i) {
        // make room for j'th
        for(int k = i - 1; k >= j; --k) {
          nOrder[k + 1] = nOrder[k];
          dDist[k + 1] = dDist[k];
        }
      }
      nOrder[j] = i;
      dDist[j] = dDistance;
    }
    if(g_dExtraAlpha > 1.0) {
      g_dExtraAlpha = 1.0;
    }


    BVector vUp(0, 0, -1);
    glBegin(GL_QUADS);
    for(i = 0; i < m_cloud[nDrawThis].m_nPuffs; ++i) {
      // Determine orientation
      BVector vRight = m_camera.m_orientation.m_vForward.CrossProduct(vUp);
      vRight.ToUnitLength();
      BVector vCloudUp = m_camera.m_orientation.m_vForward.CrossProduct(vRight);
      vCloudUp.ToUnitLength();
    
      double dRad = m_cloud[nDrawThis].m_puff[nOrder[i]].m_dRadius;

      BVector 
      vTmp = m_cloud[nDrawThis].m_puff[nOrder[i]].m_vLocation - vRight * dRad + vCloudUp * dRad;
      glTexCoord2f(0, 0);
      glVertex3f(vTmp.m_dX, vTmp.m_dY, vTmp.m_dZ);
      vTmp = m_cloud[nDrawThis].m_puff[nOrder[i]].m_vLocation + vRight * dRad + vCloudUp * dRad;
      glTexCoord2f(1, 0);
      glVertex3f(vTmp.m_dX, vTmp.m_dY, vTmp.m_dZ);
      vTmp = m_cloud[nDrawThis].m_puff[nOrder[i]].m_vLocation + vRight * dRad - vCloudUp * dRad;
      glTexCoord2f(1, 1);
      glVertex3f(vTmp.m_dX, vTmp.m_dY, vTmp.m_dZ);
      vTmp = m_cloud[nDrawThis].m_puff[nOrder[i]].m_vLocation - vRight * dRad - vCloudUp * dRad;
      glTexCoord2f(0, 1);
      glVertex3f(vTmp.m_dX, vTmp.m_dY, vTmp.m_dZ);
    }
    glEnd();
  }

  glDisable(GL_BLEND);
  glDepthMask(GL_TRUE);
};


//*****************************************************************************
void BSimulation::DrawDustClouds() {
  if((m_nDustClouds1 > 0) || 
     (m_nDustClouds2 > 0) || 
     (m_nDustClouds3 > 0) || 
     BGame::m_bWindActive) {
    OpenGLHelpers::SwitchToTexture(0);
    BTextures::Use(BTextures::DUSTCLOUD);
    //glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    BVector v1, v2, v3, v4;

    BVector vToUp;
    BVector vToCamera;
    BVector vToRight;

    // First Dust
    int i;
    for(i = 0; i < m_nDustClouds1; ++i) {
      if(i % 40) {
        // Determine orientation
        vToUp.Set(0, 0, -1);
        vToCamera = m_camera.m_vLocation - m_dustClouds1[i].m_vLocation;
        vToCamera.ToUnitLength();
        vToRight = vToCamera.CrossProduct(vToUp);
        vToRight.ToUnitLength();
        vToUp = vToRight.CrossProduct(vToCamera);
        vToUp.ToUnitLength();
      }

      // Draw rectangle to represent the dust cloud

      if(m_dustClouds1[i].m_dAlpha > 0.01) {
        OpenGLHelpers::SetColorFull(m_dustClouds1[i].m_color.m_dX, 
                                    m_dustClouds1[i].m_color.m_dY, 
                                    m_dustClouds1[i].m_color.m_dZ, 
                                    m_dustClouds1[i].m_color.m_dX * m_dustClouds1[i].m_dAlpha / 2.0);
        v1 = m_dustClouds1[i].m_vLocation - vToRight * m_dustClouds1[i].m_dSize - vToUp * m_dustClouds1[i].m_dSize;
        v2 = m_dustClouds1[i].m_vLocation - vToRight * m_dustClouds1[i].m_dSize + vToUp * m_dustClouds1[i].m_dSize;
        v3 = m_dustClouds1[i].m_vLocation + vToRight * m_dustClouds1[i].m_dSize + vToUp * m_dustClouds1[i].m_dSize;
        v4 = m_dustClouds1[i].m_vLocation + vToRight * m_dustClouds1[i].m_dSize - vToUp * m_dustClouds1[i].m_dSize;
        glBegin(GL_QUADS);
        glTexCoord2f(0, 0);
        glVertex3f(v1.m_dX, v1.m_dY, v1.m_dZ);
        glTexCoord2f(0, 1);
        glVertex3f(v2.m_dX, v2.m_dY, v2.m_dZ);
        glTexCoord2f(1, 1);
        glVertex3f(v3.m_dX, v3.m_dY, v3.m_dZ);
        glTexCoord2f(1, 0);
        glVertex3f(v4.m_dX, v4.m_dY, v4.m_dZ);
        glEnd();
      }
    }

    for(i = 0; i < m_nDustClouds2; ++i) {
      if(i % 40) {
        // Determine orientation
        vToUp.Set(0, 0, -1);
        vToCamera = m_camera.m_vLocation - m_dustClouds2[i].m_vLocation;
        vToCamera.ToUnitLength();
        vToRight = vToCamera.CrossProduct(vToUp);
        vToRight.ToUnitLength();
        vToUp = vToRight.CrossProduct(vToCamera);
        vToUp.ToUnitLength();
      }

      // Draw rectangle to represent the dust cloud

      if(m_dustClouds2[i].m_dAlpha > 0.01) {
        OpenGLHelpers::SetColorFull(m_dustClouds2[i].m_color.m_dX, 
                                    m_dustClouds2[i].m_color.m_dY, 
                                    m_dustClouds2[i].m_color.m_dZ, 
                                    m_dustClouds2[i].m_color.m_dX * m_dustClouds2[i].m_dAlpha / 2.0);
        v1 = m_dustClouds2[i].m_vLocation - vToRight * m_dustClouds2[i].m_dSize - vToUp * m_dustClouds2[i].m_dSize;
        v2 = m_dustClouds2[i].m_vLocation - vToRight * m_dustClouds2[i].m_dSize + vToUp * m_dustClouds2[i].m_dSize;
        v3 = m_dustClouds2[i].m_vLocation + vToRight * m_dustClouds2[i].m_dSize + vToUp * m_dustClouds2[i].m_dSize;
        v4 = m_dustClouds2[i].m_vLocation + vToRight * m_dustClouds2[i].m_dSize - vToUp * m_dustClouds2[i].m_dSize;
        glBegin(GL_QUADS);
        glTexCoord2f(0, 0);
        glVertex3f(v1.m_dX, v1.m_dY, v1.m_dZ);
        glTexCoord2f(0, 1);
        glVertex3f(v2.m_dX, v2.m_dY, v2.m_dZ);
        glTexCoord2f(1, 1);
        glVertex3f(v3.m_dX, v3.m_dY, v3.m_dZ);
        glTexCoord2f(1, 0);
        glVertex3f(v4.m_dX, v4.m_dY, v4.m_dZ);
        glEnd();
      }
    }

    for(i = 0; i < m_nDustClouds3; ++i) {
      if(i % 10) {
        // Determine orientation
        vToUp.Set(0, 0, -1);
        vToCamera = m_camera.m_vLocation - m_dustClouds3[i].m_vLocation;
        vToCamera.ToUnitLength();
        vToRight = vToCamera.CrossProduct(vToUp);
        vToRight.ToUnitLength();
        vToUp = vToRight.CrossProduct(vToCamera);
        vToUp.ToUnitLength();
      }

      // Draw rectangle to represent the dust cloud

      if(m_dustClouds3[i].m_dAlpha > 0.01) {
        OpenGLHelpers::SetColorFull(m_dustClouds3[i].m_color.m_dX, 
                                    m_dustClouds3[i].m_color.m_dY, 
                                    m_dustClouds3[i].m_color.m_dZ, 
                                    m_dustClouds3[i].m_color.m_dX * m_dustClouds3[i].m_dAlpha / 2.0);
        v1 = m_dustClouds3[i].m_vLocation - vToRight * m_dustClouds3[i].m_dSize - vToUp * m_dustClouds3[i].m_dSize;
        v2 = m_dustClouds3[i].m_vLocation - vToRight * m_dustClouds3[i].m_dSize + vToUp * m_dustClouds3[i].m_dSize;
        v3 = m_dustClouds3[i].m_vLocation + vToRight * m_dustClouds3[i].m_dSize + vToUp * m_dustClouds3[i].m_dSize;
        v4 = m_dustClouds3[i].m_vLocation + vToRight * m_dustClouds3[i].m_dSize - vToUp * m_dustClouds3[i].m_dSize;
        glBegin(GL_QUADS);
        glTexCoord2f(0, 0);
        glVertex3f(v1.m_dX, v1.m_dY, v1.m_dZ);
        glTexCoord2f(0, 1);
        glVertex3f(v2.m_dX, v2.m_dY, v2.m_dZ);
        glTexCoord2f(1, 1);
        glVertex3f(v3.m_dX, v3.m_dY, v3.m_dZ);
        glTexCoord2f(1, 0);
        glVertex3f(v4.m_dX, v4.m_dY, v4.m_dZ);
        glEnd();
      }
    }

    if(BGame::m_bWindActive) {
      // Draw wind particles
      OpenGLHelpers::SetColorFull(1, 1, 1, 1);
      glEnable(GL_COLOR_MATERIAL);
      glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
      vToUp.Set(0, 0, -1);
      vToCamera = m_camera.m_orientation.m_vForward * -1.0;
      vToCamera.ToUnitLength();
      vToRight = vToCamera.CrossProduct(vToUp);
      vToRight.ToUnitLength();
      vToUp = vToRight.CrossProduct(vToCamera);
      vToUp.ToUnitLength();
      // double dSize = 0.15;
      double dSize = 23.0;
      for(i = 0; i < g_cnWindParticles; ++i) {
        double dAlpha = (BGame::m_vWindParticles[i] - GetCamera()->m_vLocation).Length();
        dAlpha = dAlpha  / 125.0;
        if(dAlpha > 1.0) {
          dAlpha = 1.0;
        }
      
        // glColor4f(1, 1, 1, BGame::m_dBaseWindStrength * (1.0 - dAlpha));
        glColor4d(1, 1, 1, 0.25 * (1.0 - dAlpha));
        vToRight = BVector(1, 0, 0);
        vToUp = BVector(0, 1, 0);
        v1 = BGame::m_vWindParticles[i] - vToRight * dSize - vToUp * dSize;
        v2 = BGame::m_vWindParticles[i] - vToRight * dSize + vToUp * dSize;
        v3 = BGame::m_vWindParticles[i] + vToRight * dSize + vToUp * dSize;
        v4 = BGame::m_vWindParticles[i] + vToRight * dSize - vToUp * dSize;
        glBegin(GL_QUADS);
        glTexCoord2f(0, 0);
        glVertex3f(v1.m_dX, v1.m_dY, v1.m_dZ);
        glTexCoord2f(0, 1);
        glVertex3f(v2.m_dX, v2.m_dY, v2.m_dZ);
        glTexCoord2f(1, 1);
        glVertex3f(v3.m_dX, v3.m_dY, v3.m_dZ);
        glTexCoord2f(1, 0);
        glVertex3f(v4.m_dX, v4.m_dY, v4.m_dZ);
        glEnd();
      }
      glDisable(GL_COLOR_MATERIAL);
    }

    glDepthMask(GL_TRUE);
    //glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
  }
}






//*****************************************************************************
void BSimulation::AddTrackingTarget(string sId, BVector vLoc, double dRed, double dGreen, double dBlue) {
  // Add a tracking target to the tracking list
  // targets are drawn in the multi-purpose compass panel
  BTrackingTarget *pNew = new BTrackingTarget;
  pNew->m_sId = sId;
  pNew->m_vLoc = vLoc;
  pNew->m_dRed = dRed;
  pNew->m_dGreen = dGreen;
  pNew->m_dBlue = dBlue;
  pNew->m_pNext = 0;

  if(!m_targets) {
    // add first
    m_targets = pNew;
  } else {
    BTrackingTarget *p = m_targets;
    while(p->m_pNext) {
      p = p->m_pNext;
    }
    p->m_pNext = pNew;
  }
}


//*****************************************************************************
void BSimulation::RemoveTrackingTarget(string sId) {
  BTrackingTarget *pPrev = m_targets;
  BTrackingTarget *p = m_targets;
  while(p) {
    //if(p->m_sId.CompareNoCase(sId) == 0) {
    if(p->m_sId.compare(sId) == 0) {
      if(p == m_targets) {
        // delete first
        m_targets = p->m_pNext;
      } else {
        // delete not first
        pPrev->m_pNext = p->m_pNext;
      }
      delete p;
      return;
    }

    // Proceed to next
    pPrev = p;
    p = p->m_pNext;
  }
}












//*****************************************************************************
// RECORDING
//*****************************************************************************

void BSimulation::StartRecording() {
  m_fp = fopen("sound.txt", "w");
}

void BSimulation::StopRecording() {
  fclose(m_fp);
  m_fp = 0;
}




















// Old wheel simulation with basepoints 1 and 2.

#if 0


//*****************************************************************************
static double ViscosityDamped(double dSusp, double dForce) {
  // If force is small, allow it all
  // If force is strong, damp it to absord the shock
  double dForceFactor;
  if(fabs(dSusp) > 0.3) {
    dForceFactor = 0.0;
  } else {
    // dForceFactor = fabs(dSusp) / 0.4;
    dForceFactor = fabs(dSusp) / 0.3;
    dForceFactor = 1.0 - dForceFactor;
  }
  
  return dSusp + (dForce * dForceFactor);
}


//*****************************************************************************
void BSimulation::ApplyWheelForces(BWheel& rWheel) {
  rWheel.m_bInGround = true;
  // BVector vGroundNormal;
  // double dGroundFriction, dBaseDepth;
  // (void) PointUnderGround(rWheel.m_vLocSample, vGroundNormal, dGroundFriction, dBaseDepth);
  BVector vSpeed = rWheel.m_pBodyPoint[rWheel.m_nBodyPoint].m_vector;
  vSpeed.ToUnitLength();
  double dThermoLoss = fabs(vSpeed.ScalarProduct(rWheel.m_vGroundNormal));
  dThermoLoss *= dThermoLoss;
  if(rWheel.m_dDepth > 0.015) {
    rWheel.m_dDepth -= 0.015;
    // First apply ground force as with any point touching the ground
    // 1) calculate f = Scalar(wheel.up, ground.normal)
    double dSuspFactor = fabs(rWheel.m_orientation.m_vUp.ScalarProduct(rWheel.m_vGroundNormal)); 
    // dSuspFactor = dSuspFactor * dSuspFactor * dSuspFactor;

    // 2) Modify wheel.susp with f * wheel.depth
    if(rWheel.m_dBaseDepth > 0.0) {
      rWheel.m_dSuspension = ViscosityDamped(rWheel.m_dSuspension, dSuspFactor * rWheel.m_dBaseDepth);
    }

    // Apply thermodynamic loss

    /*
    m_vehicle.m_pBodyPoint[rWheel.m_nBasePoint1].m_vector = m_vehicle.m_pBodyPoint[rWheel.m_nBasePoint1].m_vector *
                                                        (1.0 - 0.1 * dThermoLoss);
    m_vehicle.m_pBodyPoint[rWheel.m_nBasePoint2].m_vector = m_vehicle.m_pBodyPoint[rWheel.m_nBasePoint2].m_vector *
                                                        (1.0 - 0.1 * dThermoLoss);
    */

    m_vehicle.m_pBodyPoint[rWheel.m_nBodyPoint].m_vector = m_vehicle.m_pBodyPoint[rWheel.m_nBodyPoint].m_vector *
                                                       (1.0 - 0.2 * dThermoLoss);

    // 3) add ((1.0 - f) * wheel.depth) * ground.normal to base points 1 and 2
    if(rWheel.m_vGroundNormal.m_dZ < -0.85) {
      rWheel.m_vGroundNormal.Set(0, 0, -1);
    }

    m_vehicle.m_pBodyPoint[rWheel.m_nBodyPoint].m_vector += rWheel.m_vGroundNormal *
                                                     ((1.0 - dSuspFactor) * rWheel.m_dDepth * 0.05);

    // 4) Modify suspPoint's vector with wheel.up * 
    //    (wheel.suspension_wants_to_be_at_rest)
    //    (maybe all 3 points, suspPoint, basepoint1 and basepoint2)
    BVector vToRight;
    vToRight = BVector(0, 0, -1).CrossProduct(rWheel.m_orientation.m_vForward);
    vToRight.ToUnitLength();
    double dRightFactor = rWheel.m_orientation.m_vUp.ScalarProduct(vToRight);
    vToRight = vToRight * dRightFactor;
    BVector vWheelSortaUp = rWheel.m_orientation.m_vUp - vToRight;
    vWheelSortaUp.ToUnitLength();
    //BVector vSuspForce = rWheel.m_orientation.m_vUp * 
    //                    (rWheel.m_dSuspension * dSuspFactor * 0.02);
    BVector vSuspForce = vWheelSortaUp * 
                        (rWheel.m_dSuspension * dSuspFactor * 0.02);
    m_vehicle.m_pBodyPoint[rWheel.m_nBodyPoint].m_vector  += vSuspForce * 2.0;
    //m_vehicle.m_pBodyPoint[rWheel.m_nBasePoint1].m_vector += vSuspForce * 0.25;
    //m_vehicle.m_pBodyPoint[rWheel.m_nBasePoint2].m_vector += vSuspForce * 0.25;
    rWheel.m_dDepth += 0.015;
  }

  // Then frictions (brake and wheel orientation based)
  double dFrictionFactor = 0.001;
  if(m_vehicle.m_bBreaking) {
    dFrictionFactor = g_cdBrakesFriction * rWheel.m_dFriction * rWheel.m_dGroundFriction;
    dFrictionFactor = BreakProfile(dFrictionFactor);
  }

  // Break wheel if too strong force
  /*
  if(((1.0 - rWheel.m_dTTT) * rWheel.m_dFriction * dGroundFriction > 0.035) && 
    (rWheel.m_pBodyPoint[rWheel.m_nBodyPoint].m_vector.Length() > 0.2)) {
    rWheel.m_bBroken = true;
    rWheel.m_dRadius *= 0.85;
    rWheel.m_dFriction -= 0.1;
  }
  */

  dFrictionFactor = max(dFrictionFactor, (1.0 - rWheel.m_dTTT) * rWheel.m_dFriction * rWheel.m_dGroundFriction);

  // Apply ground oriented friction
  double dLossFactor = 1.0 - (dFrictionFactor * 0.2);

  //rWheel.m_pBodyPoint[rWheel.m_nBasePoint1].m_vector = (rWheel.m_pBodyPoint[rWheel.m_nBasePoint1].m_vector) * 
  //                                                   dLossFactor;
  //rWheel.m_pBodyPoint[rWheel.m_nBasePoint2].m_vector = (rWheel.m_pBodyPoint[rWheel.m_nBasePoint2].m_vector) * 
  //                                                   dLossFactor;
  rWheel.m_pBodyPoint[rWheel.m_nBodyPoint].m_vector = (rWheel.m_pBodyPoint[rWheel.m_nBodyPoint].m_vector) * 
                                                    dLossFactor;

  // Then acceleration
  if(m_vehicle.m_bAccelerating || m_vehicle.m_bReversing) { // 4WD

    //rWheel.m_pBodyPoint[rWheel.m_nBasePoint1].m_vector += rWheel.m_orientation.m_vForward * 
    //                                                   rWheel.m_dGroundFriction *
    //                                                   rWheel.m_dFriction *
    //                                                   m_dAccelerationFactor *
    //                                                   (m_vehicle.m_bAccelerating ? m_vehicle.m_dAccelerationFactor : -m_vehicle.m_dReversingFactor);
    //rWheel.m_pBodyPoint[rWheel.m_nBasePoint2].m_vector += rWheel.m_orientation.m_vForward * 
    //                                                   rWheel.m_dGroundFriction *
    //                                                   rWheel.m_dFriction *
    //                                                   m_dAccelerationFactor *
    //                                                   (m_vehicle.m_bAccelerating ? m_vehicle.m_dAccelerationFactor : -m_vehicle.m_dReversingFactor);

    rWheel.m_pBodyPoint[rWheel.m_nBodyPoint].m_vector += rWheel.m_orientation.m_vForward * 
                                                      rWheel.m_dGroundFriction *
                                                      rWheel.m_dFriction *
                                                      m_dAccelerationFactor * 4.0 *
                                                      (m_vehicle.m_bAccelerating ? m_vehicle.m_dAccelerationFactor : -m_vehicle.m_dReversingFactor);
  }

  // Apply turning
  if(!m_vehicle.m_bBreaking ) { // && !rWheel.m_bRear
    BVector vWheelVector = rWheel.m_pBodyPoint[rWheel.m_nBodyPoint].m_vector;
    BVector vIdeal = rWheel.m_orientation.m_vForward;
    if((vIdeal.ScalarProduct(vWheelVector) < 0.0) && 
       !m_vehicle.m_bAccelerating) {
      vIdeal = vIdeal * -1.0;
    }
    vIdeal.ToUnitLength();

    BVector vReality = vWheelVector;
    double  dRealLen = vReality.Length();
    vReality.ToUnitLength();

    double dEffect = rWheel.m_dTTT * rWheel.m_dFriction * rWheel.m_dGroundFriction * 0.05;

    rWheel.m_pBodyPoint[rWheel.m_nBodyPoint].m_vector = vIdeal * dRealLen * dEffect +
                                                     rWheel.m_pBodyPoint[rWheel.m_nBodyPoint].m_vector *
                                                     (1.0 - dEffect);  
  }

  // Record hit point for trails
  if(dFrictionFactor > 0.001) {
    ++rWheel.m_nGroundHits;
  }
}

  if(rWheel.m_dDepth > 0.015) {
    rWheel.m_dDepth -= 0.015;
    // First apply ground force as with any point touching the ground
    // 1) calculate f = Scalar(wheel.up, ground.normal)
    double dSuspFactor = fabs(rWheel.m_orientation.m_vUp.ScalarProduct(rWheel.m_vGroundNormal)); 
    // dSuspFactor = dSuspFactor * dSuspFactor * dSuspFactor;

    // 2) Modify wheel.susp with f * wheel.depth
    if(rWheel.m_dBaseDepth > 0.0) {
      rWheel.m_dSuspension = ViscosityDamped(rWheel.m_dSuspension, dSuspFactor * rWheel.m_dBaseDepth);
    }

    // Apply thermodynamic loss

    pBodyPoint->m_vector = pBodyPoint->m_vector * (1.0 - 0.2 * dThermoLoss);

    // 3) add ((1.0 - f) * wheel.depth) * ground.normal to base points 1 and 2
    if(rWheel.m_vGroundNormal.m_dZ < -0.85) {
      rWheel.m_vGroundNormal.Set(0, 0, -1);
    }

    pBodyPoint->m_vector += rWheel.m_vGroundNormal * ((1.0 - dSuspFactor) * rWheel.m_dDepth * 0.05);

    // 4) Modify suspPoint's vector with wheel.up * 
    //    (wheel.suspension_wants_to_be_at_rest)
    //    (maybe all 3 points, suspPoint, basepoint1 and basepoint2)
    BVector vToRight;
    vToRight = BVector(0, 0, -1).CrossProduct(rWheel.m_orientation.m_vForward);
    vToRight.ToUnitLength();
    double dRightFactor = rWheel.m_orientation.m_vUp.ScalarProduct(vToRight);
    vToRight = vToRight * dRightFactor;
    BVector vWheelSortaUp = rWheel.m_orientation.m_vUp - vToRight;
    vWheelSortaUp.ToUnitLength();
    BVector vSuspForce = vWheelSortaUp * (rWheel.m_dSuspension * dSuspFactor * 0.02);
    pBodyPoint->m_vector += vSuspForce * 2.0;
    rWheel.m_dDepth += 0.015;
  }


// Old wheel simulation without slide control (in comments).

//*****************************************************************************
void BSimulation::ApplyWheelForces(BWheel& rWheel) {
  rWheel.m_bInGround = true;

  BBodyPoint* pBodyPoint = &(rWheel.m_pBodyPoint[rWheel.m_nBodyPoint]);

  BVector vSpeed = pBodyPoint->m_vector;
  vSpeed.ToUnitLength();
  double dThermoLoss = fabs(vSpeed.ScalarProduct(rWheel.m_vGroundNormal));
  dThermoLoss *= (1.0 - fabs(vSpeed.ScalarProduct(rWheel.m_orientation.m_vForward)));

  if(rWheel.m_dDepth > 0.05) {
    rWheel.m_dDepth -= 0.05;

    // Smooth the depth function so that we don't jump so much over the surface
    // double dSmoothDepth = rWheel.m_dDepth * 10.0;
    // dSmoothDepth = dSmoothDepth - (( 2 * dSmoothDepth) / (pow(2, pow(dSmoothDepth + 1, 2))));
    // dSmoothDepth /= 10.0;

    double dPerpendicularity = fabs(rWheel.m_orientation.m_vUp.ScalarProduct(rWheel.m_vGroundNormal)); 
    double dPrevSuspension = rWheel.m_dSuspension;
    double dNewSuspension  = dPrevSuspension + dPerpendicularity * rWheel.m_dDepth;

    double dSuspGets = ((dPrevSuspension + dNewSuspension) * 0.5) / rWheel.m_dMaxSuspThrow;

    if(dSuspGets > 1.0) {
      dSuspGets = 1.0;
    }
    dSuspGets = 1.0 - pow(dSuspGets, 1.0 / rWheel.m_dSuspStiffness);

    // Viscosity damping
    double dViscDamp = rWheel.m_dDepth / rWheel.m_dMaxSuspThrow;
    if(dViscDamp > 1.0) {
      dViscDamp = 1.0;
    }

    dSuspGets *= (1.0 - dViscDamp);

    rWheel.m_dSuspension += dSuspGets * rWheel.m_dDepth;

	  double dBodyPointGets = 1.0 - dSuspGets;


    // 4) Modify suspPoint's vector with wheel.up * 
    //    (wheel.suspension_wants_to_be_at_rest)
    //    (maybe all 3 points, suspPoint, basepoint1 and basepoint2)
    //BVector vToRight;
    //vToRight = BVector(0, 0, -1).CrossProduct(rWheel.m_orientation.m_vForward);
    //vToRight.ToUnitLength();
    //double dRightFactor = rWheel.m_orientation.m_vUp.ScalarProduct(vToRight);
    //vToRight = vToRight * dRightFactor;
    //BVector vWheelSortaUp = rWheel.m_orientation.m_vUp - vToRight;
    //vWheelSortaUp.ToUnitLength();

    double dSlide = fabs(rWheel.m_vGroundNormal.ScalarProduct(BVector(0, 0, -1)));
    dSlide *= 1.3;
    if(dSlide > 1.0) {
      dSlide = 1.0;
    }
    BVector vRightOnGround = rWheel.m_orientation.m_vForward.CrossProduct(BVector(0, 0, -1));
    vRightOnGround.ToUnitLength();
    double dCorr = rWheel.m_vGroundNormal.ScalarProduct(vRightOnGround);
    BVector vCorr = rWheel.m_vGroundNormal + vRightOnGround * -(dCorr * 0.85 * dSlide);

    //pBodyPoint->m_vector += vWheelSortaUp * (dBodyPointGets * rWheel.m_dDepth * GroundHardnessAt(rWheel.m_vLocSample) * 0.5);
    //          pBodyPoint->m_vector += rWheel.m_vGroundNormal * 
    pBodyPoint->m_vector += vCorr *
                            (dBodyPointGets * rWheel.m_dDepth * GroundHardnessAt(rWheel.m_vLocSample) * 0.5);
    //pBodyPoint->m_vector += (vRight * dSideways +
    //                         rWheel.m_vGroundNormal) * 
    //                         (dBodyPointGets * dSmoothDepth * GroundHardnessAt(rWheel.m_vLocSample) * 0.5);

    // Apply thermodynamic loss
    pBodyPoint->m_vector = pBodyPoint->m_vector * (1.0 - 0.3 * dBodyPointGets * dThermoLoss);
    
    // Apply ground oriented thermoloss (to stop the ridiculous bounching)
    double dOnNormal = pBodyPoint->m_vector.ScalarProduct(rWheel.m_vGroundNormal);
    // pBodyPoint->m_vector = pBodyPoint->m_vector + rWheel.m_vGroundNormal * -dOnNormal * 0.25; // was * 0.15
    pBodyPoint->m_vector = pBodyPoint->m_vector + rWheel.m_vGroundNormal * -dOnNormal * 0.2; // was * 0.15

    rWheel.m_dDepth += 0.05;
  }

  // Then frictions (brake and wheel orientation based)
  double dFrictionFactor = 0.00099;
  if(m_vehicle.m_bBreaking) {
    dFrictionFactor = g_cdBrakesFriction * rWheel.m_dBrakeFactor * rWheel.m_dFriction * rWheel.m_dGroundFriction;
    dFrictionFactor = BreakProfile(dFrictionFactor);
  } else {
    dFrictionFactor = max(dFrictionFactor, (1.0 - rWheel.m_dTTT) * rWheel.m_dFriction * rWheel.m_dGroundFriction);
  }

  // Apply ground oriented friction
  double dLossFactor = 1.0 - (dFrictionFactor * 0.1);
  pBodyPoint->m_vector = (pBodyPoint->m_vector) * dLossFactor;

  // Then acceleration
  if(m_vehicle.m_bAccelerating || m_vehicle.m_bReversing) { // 4WD
    pBodyPoint->m_vector += rWheel.m_orientation.m_vForward * 
                            // rWheel.m_dGroundFriction *
                            // rWheel.m_dFriction *
                            (rWheel.m_vGroundNormal.m_dZ * rWheel.m_vGroundNormal.m_dZ) * 
                            rWheel.m_dDriveFactor *
                            m_dAccelerationFactor * 
                            m_vehicle.m_dHorsePowers * 
                            (m_vehicle.m_bAccelerating ? m_vehicle.m_dAccelerationFactor : -m_vehicle.m_dReversingFactor);
  }

  // Apply turning
  if(!m_vehicle.m_bBreaking ) {
    BVector vWheelVector = pBodyPoint->m_vector;
    BVector vIdeal = rWheel.m_orientation.m_vForward;
    if((vIdeal.ScalarProduct(vWheelVector) < 0.0) && 
       !m_vehicle.m_bAccelerating) {
      vIdeal = vIdeal * -1.0;
    }
    vIdeal.ToUnitLength();

    BVector vReality = vWheelVector;
    double  dRealLen = vReality.Length();
    vReality.ToUnitLength();

    // double dEffect = rWheel.m_dTTT * rWheel.m_dFriction * rWheel.m_dGroundFriction * 0.05;
    double dEffect = rWheel.m_dTTT * rWheel.m_dFriction * rWheel.m_dGroundFriction * 0.2;

    pBodyPoint->m_vector = vIdeal * dRealLen * dEffect + pBodyPoint->m_vector * (1.0 - dEffect);
  }

  // Record hit point for trails
  if(dFrictionFactor > 0.001) {
    ++rWheel.m_nGroundHits;
  }
}


#endif






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
