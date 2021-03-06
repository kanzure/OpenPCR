/*
 *  thermocycler.h - OpenPCR control software.
 *  Copyright (C) 2010-2011 Josh Perfetto. All Rights Reserved.
 *
 *  OpenPCR control software is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as published
 *  by the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  OpenPCR control software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with
 *  the OpenPCR control software.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _THERMOCYCLER_H_
#define _THERMOCYCLER_H_

#include "PID_v1.h"
#include "program.h"

class Display;
class SerialControl;
  
class Thermocycler {
public:
  enum ProgramState {
    EOff = 0,
    EStartup,
    EStopped,
    ELidWait,
    ERunning,
    EComplete,
    EError,
    EClear //for Display clearing only
  };
  
  enum ThermalState {
    EHolding = 0,
    EHeating,
    ECooling,
    EIdle
  };
  
  enum ThermalDirection {
    OFF,
    HEAT,
    COOL
  };
  
  Thermocycler(boolean restarted);
  ~Thermocycler();
  
  // accessors
  ProgramState GetProgramState() { return iProgramState; }
  ThermalState GetThermalState();
  Step* GetCurrentStep() { return ipCurrentStep; }
  Cycle* GetDisplayCycle() { return ipDisplayCycle; }
  int GetNumCycles();
  int GetCurrentCycleNum();
  const char* GetProgName() { return iszProgName; }
  Display* GetDisplay() { return ipDisplay; }
  ProgramComponentPool<Cycle, 4>& GetCyclePool() { return iCyclePool; }
  ProgramComponentPool<Step, 20>& GetStepPool() { return iStepPool; }
  
  boolean Ramping() { return iRamping; }
  int GetPeltierPwm() { return iPeltierPwm; }
  float GetPlateTemp() { return iPlateTemp; }
  float GetLidTemp() { return iLidTemp; }
  unsigned long GetTimeRemainingS() { return iEstimatedTimeRemainingS; }
  unsigned long GetElapsedTimeS() { return (millis() - iProgramStartTimeMs) / 1000; }
  
  // control
  void SetProgram(Cycle* pProgram, Cycle* pDisplayCycle, const char* szProgName, int lidTemp); //takes ownership of cycles
  void Stop();
  PcrStatus Start();
  void ProcessCommand(SCommand& command);
  
  // internal
  void Loop();
  
private:
  void CheckPower();
  void ReadLidTemp();
  void ReadPlateTemp();
  void ControlPeltier();
  void ControlLid();
  void UpdateEta();
 
  //util functions
  void SetPlateTarget(double target);
  void SetLidTarget(double target);
  void SetPeltier(ThermalDirection dir, int pwm);
  uint8_t mcp342xWrite(uint8_t config);
  uint8_t mcp342xRead(int32_t &data);
  float TableLookup(const unsigned long lookupTable[], unsigned int tableSize, int startValue, unsigned long searchValue);
  float TableLookup(const unsigned int lookupTable[], unsigned int tableSize, int startValue, unsigned long searchValue);
  
private:
  // constants
  static const int PLATE_TEMP_SENSOR_PIN = 0;
  
  // components
  Display* ipDisplay;
  SerialControl* ipSerialControl;
  ProgramComponentPool<Cycle, 4> iCyclePool;
  ProgramComponentPool<Step, 20> iStepPool;
  
  // state
  ProgramState iProgramState;
  double iPlateTemp;
  double iTargetPlateTemp;
  double iLidTemp;
  double iTargetLidTemp;
  Cycle* ipProgram;
  Cycle* ipDisplayCycle;
  char iszProgName[21];
  Step* ipCurrentStep;
  unsigned long iCycleStartTime;
  boolean iRamping;
  boolean iDecreasing;
  enum ControlMode {
    EBangBang,
    EPID
  };
  boolean iRestarted;
  
  ControlMode iPlateControlMode;
  ControlMode iLidControlMode;
  
  // peltier control
  PID iPlatePid;
  PID iLidPid;
  ThermalDirection iThermalDirection; //holds actual real-time state
  double iPeltierPwm;
  double iLidPwm;
  
  // program eta calculation
  unsigned long iProgramStartTimeMs;
  unsigned long iProgramHoldDurationS;
  double iProgramRampDegrees;
  double iElapsedRampDegrees;
  unsigned long iElapsedRampDurationMs;
  double iRampStartTemp;
  unsigned long iRampStartTime;
  unsigned long iEstimatedTimeRemainingS;
  boolean iHasCooled;
};

#endif
