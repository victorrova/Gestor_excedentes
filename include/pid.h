#ifndef PID_H
#define PID_H


typedef struct
{
 float Kp;  // Coeficiente proporcional Proporcional
 float Ki;  // coeficiente integral Integral
 float Kd;  // coeficiente derivativo
 
 int Ek;  // Error actual
 int Ek1; // Error anterior e (k-1)
 int Ek2; // El error anterior e (k-2)
 int LocSum; // Posición de integración acumulativa
}PID_LocTypeDef;


typedef struct
{
  float Kp;                       // Coeficiente proporcional Proporcional
  float Ki;                       // coeficiente integral Integral
  float Kd;                       // coeficiente derivativo
 
  int Error;                       // Error actual
  int CumError;                      // Error anterior e (k-1)
  int LastError;                      // El error anterior e (k-2)
  int max;
  int min;
}PID_IncTypeDef;



int PID(int SetValue, int ActualValue, PID_IncTypeDef *PID);
int PID_Loc(int SetValue, int ActualValue, PID_LocTypeDef *PID);


#endif