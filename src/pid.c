
#include "pid.h"



int PID_Loc(int SetValue, int ActualValue, PID_LocTypeDef *PID)
{
 int PIDLoc; //posición
 
 PID->Ek = SetValue - ActualValue;
 PID->LocSum += PID->Ek; // error acumulativo
 
 PIDLoc = PID->Kp * PID->Ek + (PID->Ki * PID->LocSum) + PID->Kd * (PID->Ek1 - PID->Ek);
 
 PID->Ek1 = PID->Ek; return PIDLoc;
}


int PID(int SetValue, int ActualValue, PID_IncTypeDef *PID)
{
  int PIDInc;    // incremental
  PID->Error = SetValue - ActualValue;
  PID->CumError += PID->Error;//*100;
  int rateError =(PID->Error - PID->LastError);///1000
  PIDInc = (PID->Kp * PID->Error) + (PID->Ki * PID->CumError) + (PID->Kd * rateError);
  PID->LastError = PID->Error;
 
  if(PIDInc > PID->max)
  {
    PIDInc = PID->max;
  }
  else if(PIDInc < PID->min)
  {
    PIDInc = PID->min;
  }
  return PIDInc;
}
