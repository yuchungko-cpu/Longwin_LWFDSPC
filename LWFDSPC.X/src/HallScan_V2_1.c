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
#include "./longwin/codeSw.h"

extern UGF_T uGF;

tFindHallAngle FindHallAngle;
extern volatile unsigned int HallState;
extern volatile unsigned int OldHallState;
extern volatile uint16_t HallPeriod; 
extern volatile uint16_t HallPeriodFiltered; 
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
extern volatile signed int HallMinPeriod;
extern unsigned long PeriodAverage;
signed int HallAngleFltr = 0; 
extern volatile int16_t thetaElectrical;
int PeriodFilter;
long int PeriodStateVar;
int PeriodFilter;
long int PeriodStateVar;
void GetInitHallAngle(void);
void GetHallAngleAuto_Inline(void);


inline void GetHallAngleDirAuto_Inline (void)
{
    if(FindHallAngleCnt == 9)
    {
        switch (HallState)
        {
            case 5:
                if(OldHallState == 4)  // angle increases by 5 1 3 2 6 4 5 1 3 2 6 4
                    uGF.DirectionDefault = 0;                
                else if(OldHallState == 1)                  // angle increases by 5 4 6 2 3 1
                    uGF.DirectionDefault = 1;
                else
                //    TestOut2 = !TestOut2;
                break;
            case 1:
                if(OldHallState == 5)
                {
                    uGF.DirectionDefault = 0;                
                }
                else if(OldHallState == 3)
                {
                    uGF.DirectionDefault = 1;                  
                }
                else
                //    TestOut2 = !TestOut2;

                break;
            case 3:
                if(OldHallState == 1)
                {
                    uGF.DirectionDefault = 0;
                }
                else if(OldHallState == 2)
                {
                    uGF.DirectionDefault = 1;                
                }
                else
                 //   TestOut2 = !TestOut2;          
                break;
            case 2:
                if(OldHallState == 3)
                {
                    uGF.DirectionDefault = 0;                
                }
                else if(OldHallState == 6)
                {
                    uGF.DirectionDefault = 1;                
                }
                 else
                //    TestOut2 = !TestOut2;          
                break;
            case 6:
                if(OldHallState == 2)
                {
                    uGF.DirectionDefault = 0;                
                }
                else if(OldHallState == 4)
                { 
                    uGF.DirectionDefault = 1;                
                }
                else
                //    TestOut2 = !TestOut2;           
                break;
            case 4:
                if(OldHallState == 6)
                {
                    uGF.DirectionDefault = 0;                
                }
                else if(OldHallState == 5)
                {
                    uGF.DirectionDefault = 1;                
                }
                else
                //    TestOut2 = !TestOut2;
                break;
            default:

                break;
        }
        
    }

}
void GetHallAngleAuto_Inline(void)
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
    
    if(uGF.Direction == uGF.DirectionDefault )
       HallAngle = FindHallAngle.Pos[HallState] + 16384;
    else
       HallAngle = FindHallAngle.Pos[HallState] + 10923 + 16384; 

    thetaElectrical = HallAngle;
        
}

void GetInitHallAngle(void)
{
    HallState = 0;
    HallState = (unsigned int)I_HALL_U_PIN + ((unsigned int)I_HALL_V_PIN * 2) + ((unsigned int)I_HALL_W_PIN * 4);
    HallAngle = FindHallAngle.Pos[HallState] + 5461 + 16384;
    thetaElectrical = HallAngle;  // 30 degrees shift
}

void CalculateParkAngleHall(void)
{
#ifdef SixStepStart        
        if(abs(Speed) <= Q15(0.02))
        {
            AngleStepByPWM = 0;            
            if (uGF.Direction == uGF.DirectionDefault)
                HallAngle = FindHallAngle.Pos[HallState] + 5461 + 16384;// HURST@CW

            else
                HallAngle = FindHallAngle.Pos[HallState] + 5461 + 16384;// Hurst@CCW

            thetaElectrical = HallAngle;
                             
         //---------------------------------------------------------------------
           // Still calculate the Period filter for the transition to sinewave             
//            PeriodStateVar+= (((long int)HallPeriod- (long int)PeriodFilter)*1000);
//            if(PeriodStateVar >= 30000*32768)
//                PeriodStateVar = 30000*32768;
//            else if(PeriodStateVar <= -30000*32768)
//                PeriodStateVar = -30000*32768;
//            PeriodFilter = (int)(PeriodStateVar>>15);  
//            TempLong = __builtin_divud(AngleStepLong,(unsigned int)PeriodFilter);           
        //----------------------------------------------------------------------                        
        }

       else
       {          
         //----------------------------------------------------------------------
//            PeriodStateVar+= (((long int)HallPeriod- (long int)PeriodFilter)*1000);
//            if(PeriodStateVar >= 30000*32768)
//                PeriodStateVar = 30000*32768;
//            else if(PeriodStateVar <= -30000*32768)
//                PeriodStateVar = -30000*32768;
//            PeriodFilter = (int)(PeriodStateVar>>15);  
//            
//            if(PeriodFilter < HallMinPeriod)
//                PeriodFilter = HallMinPeriod + 1;
//            TempLong = __builtin_divud(AngleStepLong,(unsigned int)PeriodFilter);	           
          //------------------------------------------------------------------------
            TempLong = __builtin_divud(AngleStepLong,(unsigned int)HallPeriodFiltered);	
         //-------------------------------------------------------------------------   
            AngleStepByPWM = TempLong;     
            if (uGF.Direction == uGF.DirectionDefault)
            {
                thetaElectrical += AngleStepByPWM;  
                if((thetaElectrical - HallAngle) > 10922)
                    thetaElectrical = HallAngle + 10922;
            }
            else
            {
                thetaElectrical -= AngleStepByPWM;  
                if((HallAngle - thetaElectrical) > 10922)
                    thetaElectrical = HallAngle - 10922;        
            }    
       }

#else 
    // Start without Six-Step driving
    //----------------------------------------------------------------------
//       PeriodStateVar+= (((long int)Period - (long int)PeriodFilter)*100);
//       if(PeriodStateVar >= 30000*32768)
//           PeriodStateVar = 30000*32768;
//       else if(PeriodStateVar <= -30000*32768)
//           PeriodStateVar = -30000*32768;
//       PeriodFilter = (int)(PeriodStateVar>>15);  
//
//       if(PeriodFilter < HallMinPeriod)
//           PeriodFilter = HallMinPeriod + 1;
       TempLong = __builtin_divud(AngleStepLong,(unsigned int)HallPeriodFiltered);	

     //------------------------------------------------------------------------
     //  TempLong = __builtin_divud(AngleStepLong,(unsigned int)Period);	
    //-------------------------------------------------------------------------   
       AngleStepByPWM = TempLong;     
       if (uGF.Direction == uGF.DirectionDefault)
       {
           thetaElectrical += AngleStepByPWM;  
/****  Comment out the 2 line below to disable the boundary check  ****/         
           if((thetaElectrical - HallAngle) > 10922)
               thetaElectrical = HallAngle + 10922;
       }
       else
       {
           thetaElectrical -= AngleStepByPWM;  
/****  Comment out the 2 line below to disable the boundary check  ****/             
           if((HallAngle - thetaElectrical) > 10922)
               thetaElectrical = HallAngle - 10922;        
       }            
                        
 
#endif 
}
 
