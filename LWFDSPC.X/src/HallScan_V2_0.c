/*
 * File:   HallScan.c
 * Author: A12673
 *
 * Created on 2016/1/6, 9:52
 */


#include "control2.h"
#include "HallScan_EE.h"
#include <stdlib.h>
#include "userparms.h"
#include "general.h"

extern UGF_T uGF;

tFindHallAngle FindHallAngle;
extern volatile unsigned int HallState;
extern volatile unsigned int OldHallState;

extern volatile signed int HallAngle;
unsigned int FindHallAngleCnt = 0;
extern unsigned int Startup_Lock;
unsigned int MotorAlignLockTime;
extern volatile unsigned int Speed;
volatile unsigned int AngleStepByPWM;
volatile unsigned long  AngleStepLong = ANGLESTEP;
unsigned long TempLong;
extern volatile unsigned long Period;
//tFlashCtrl FlashCtrl;
extern volatile unsigned int CNState;
extern volatile unsigned int CNStateOld;
extern volatile signed int MinPeriod;
extern unsigned long PeriodAverage;
signed int HallAngleFltr = 0; 
extern volatile int16_t thetaElectrical;
int PeriodFilter;
long int PeriodStateVar;

inline void GetInitHallAngleAuto_Inline(void);
inline void GetHallAngleAuto_Inline (void);


inline void GetHallAngleDirAuto_Inline (void)
{
    if(FindHallAngleCnt == 9)
    {
        switch (HallState)
        {
            case 5:
                if(OldHallState == 4)  // angle increases by 5 1 3 2 6 4 5 1 3 2 6 4
                    uGF.DirectionAuto = 0;                
                else if(OldHallState == 1)                  // angle increases by 5 4 6 2 3 1
                    uGF.DirectionAuto = 1;
                else
                //    TestOut2 = !TestOut2;
                break;
            case 1:
                if(OldHallState == 5)
                {
                    uGF.DirectionAuto = 0;                
                }
                else if(OldHallState == 3)
                {
                    uGF.DirectionAuto = 1;                  
                }
                else
                //    TestOut2 = !TestOut2;

                break;
            case 3:
                if(OldHallState == 1)
                {
                    uGF.DirectionAuto = 0;
                }
                else if(OldHallState == 2)
                {
                    uGF.DirectionAuto = 1;                
                }
                else
                 //   TestOut2 = !TestOut2;          
                break;
            case 2:
                if(OldHallState == 3)
                {
                    uGF.DirectionAuto = 0;                
                }
                else if(OldHallState == 6)
                {
                    uGF.DirectionAuto = 1;                
                }
                 else
                //    TestOut2 = !TestOut2;          
                break;
            case 6:
                if(OldHallState == 2)
                {
                    uGF.DirectionAuto = 0;                
                }
                else if(OldHallState == 4)
                { 
                    uGF.DirectionAuto = 1;                
                }
                else
                //    TestOut2 = !TestOut2;           
                break;
            case 4:
                if(OldHallState == 6)
                {
                    uGF.DirectionAuto = 0;                
                }
                else if(OldHallState == 5)
                {
                    uGF.DirectionAuto = 1;                
                }
                else
                //    TestOut2 = !TestOut2;
                break;
            default:

                break;
        }
        
    }

}
inline void GetHallAngleAuto_Inline (void)
{
    switch (HallState)
    {
        case 5:
            if(OldHallState == 4)  // angle increases by 5 1 3 2 6 4 5 1 3 2 6 4
                uGF.Direction = 0;                
            else if(OldHallState == 1)                  // angle increases by 5 4 6 2 3 1
                uGF.Direction = 1;
            else
             //   TestOut2 = !TestOut2;
            break;
        case 1:
            if(OldHallState == 5)
            {
                uGF.Direction = 0;                
            }
            else if(OldHallState == 3)
            {
                uGF.Direction = 1;                  
            }
            else
             //   TestOut2 = !TestOut2;

            break;
        case 3:
            if(OldHallState == 1)
            {
                uGF.Direction = 0;
            }
            else if(OldHallState == 2)
            {
                uGF.Direction = 1;                
            }
            else
             //   TestOut2 = !TestOut2;         
            break;
        case 2:
            if(OldHallState == 3)
            {
                uGF.Direction = 0;                
            }
            else if(OldHallState == 6)
            {
                uGF.Direction = 1;                
            }
             else
             //   TestOut2 = !TestOut2;          
            break;
        case 6:
            if(OldHallState == 2)
            {
                uGF.Direction = 0;                
            }
            else if(OldHallState == 4)
            { 
                uGF.Direction = 1;                
            }
            else
             ///   TestOut2 = !TestOut2;          
            break;
        case 4:
            if(OldHallState == 6)
            {
                uGF.Direction = 0;                
            }
            else if(OldHallState == 5)
            {
                uGF.Direction = 1;                
            }
            else
            //    TestOut2 = !TestOut2;
            break;
        default:

            break;
    }        
    OldHallState = HallState;
    
    if(uGF.Direction == uGF.DirectionAuto )
       HallAngle = FindHallAngle.Pos[HallState] + 16384;
    else
       HallAngle = FindHallAngle.Pos[HallState] + 10923 + 16384; 

    HallAngleFltr = HallAngle;
        
}

inline void GetInitHallAngleAuto_Inline(void)
{
    HallState = 0;
    HallState = (unsigned int)I_HALL_U_PIN + ((unsigned int)I_HALL_V_PIN * 2) + ((unsigned int)I_HALL_W_PIN * 4);
    HallAngle = FindHallAngle.Pos[HallState] + 5461 + 16384;
    thetaElectrical = HallAngle;  // 30 degrees shift
}

inline void CalculateParkAngleHall(void)
{
    
    if(abs(Speed) <= Q15(0.05))
    {
        AngleStepByPWM = 0;            
        if (uGF.Direction == uGF.DirectionAuto)
            //HallAngle = FindHallAngle.Pos[HallState] + 5461 + 16384;
            HallAngle = FindHallAngle.Pos[HallState] +16384;
        else
            //HallAngle = FindHallAngle.Pos[HallState] + 5461 + 16384;
            HallAngle = FindHallAngle.Pos[HallState] + 10923 + 16384;
        thetaElectrical = HallAngle;
       // Still calculate the Period filter for the transition to sinewave                               
     //---------------------------------------------------------------------

        PeriodStateVar+= (((long int)Period - (long int)PeriodFilter)*1500);
        if(PeriodStateVar >= 30000*32768)
            PeriodStateVar = 30000*32768;
        else if(PeriodStateVar <= -30000*32768)
            PeriodStateVar = -30000*32768;
        PeriodFilter = (int)(PeriodStateVar>>15);  
        TempLong = __builtin_divud(AngleStepLong,(unsigned int)PeriodFilter);	
       // TempLong = __builtin_divud(AngleStepLong,(unsigned int)PeriodAverage);
    //----------------------------------------------------------------------    
        //AngleStepByPWM = TempLong;   

    }

   else
   {          
     //----------------------------------------------------------------------
        PeriodStateVar+= (((long int)Period - (long int)PeriodFilter)*1500);
        if(PeriodStateVar >= 30000*32768)
            PeriodStateVar = 30000*32768;
        else if(PeriodStateVar <= -30000*32768)
            PeriodStateVar = -30000*32768;
        PeriodFilter = (int)(PeriodStateVar>>15);  

        if(PeriodFilter < MinPeriod)
            PeriodFilter = MinPeriod + 1;
        TempLong = __builtin_divud(AngleStepLong,(unsigned int)PeriodFilter);           // Use PeriodFilter
       // TempLong = __builtin_divud(AngleStepLong,(unsigned int)PeriodAverage);	// Use PeriodAverage
      //------------------------------------------------------------------------
      //  TempLong = __builtin_divud(AngleStepLong,(unsigned int)Period);           // Use Period
     //-------------------------------------------------------------------------   
        AngleStepByPWM = TempLong;     
        if (uGF.Direction == uGF.DirectionAuto)
        {
            if(HallState == 5) 
            {
                if(thetaElectrical > HallAngleFltr)
                {
                    thetaElectrical += (AngleStepByPWM >>1);
                    HallAngleFltr += AngleStepByPWM;
                    if(thetaElectrical <= HallAngleFltr)
                      thetaElectrical = HallAngleFltr;
                }                       
                else
                {
                   thetaElectrical += (AngleStepByPWM << 1);
                   HallAngleFltr += AngleStepByPWM;
                   if(thetaElectrical >= HallAngleFltr)
                      thetaElectrical = HallAngleFltr;
                }

            } 
            else
                thetaElectrical += AngleStepByPWM;  

        }
        else
        {
            if(HallState == 5) 
            {
                if(thetaElectrical < HallAngleFltr)
                {
                    thetaElectrical -= (AngleStepByPWM >> 1);
                    HallAngleFltr -= AngleStepByPWM;
                    if(thetaElectrical >= HallAngleFltr)
                      thetaElectrical = HallAngleFltr;
                }                       
                else
                {
                   thetaElectrical -= (AngleStepByPWM << 1);
                   HallAngleFltr -= AngleStepByPWM;
                   if(thetaElectrical <= HallAngleFltr)
                      thetaElectrical = HallAngleFltr;
                }
            } 
            else
                thetaElectrical -= AngleStepByPWM;                  
        }               
    }            
       
}
 
