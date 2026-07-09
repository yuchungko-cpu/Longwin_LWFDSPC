/*
 * File:   HallScan.c
 * Author: A12673
 *
 * Created on 2016?1?6?, ?? 9:52
 */

#include "HallScan_EE.h"
#include "control.h"
#include <p33Exxxx.h>
#include <stdlib.h>
#include "park.h"
// UserParms.h path should be changed according to the main project 
// Choose the right dsPIC part number
//#include "../../MCLV2_INTOP_Hall_FOC_Auto2_EEPROM/UserParms.h"
//#include "../../MCLV2_ExtOP_Hall_FOC_Auto2_EEPROM/UserParms.h"
//#include "../../YHH_Hub_MCLV2_ExtOP_Hall_FOC_Auto2_EEPROM/UserParms.h"
#include "userparms.h"
#include "general.h"
#include "codeSw.h"

tFindHallAngle FindHallAngle;
extern volatile unsigned int HallState;
extern volatile unsigned int OldHallState;
extern tuGF uGF;
extern volatile signed int HallAngle;
extern tParkParm   ParkParm; 
unsigned int FindHallAngleCnt = 0;
extern unsigned int Startup_Lock;
unsigned int MotorAlignLockTime;
extern volatile unsigned int Speed;
volatile unsigned int AngleStepByPWM;
volatile unsigned long  AngleStepLong = ANGLESTEP;
unsigned long TempLong;
extern volatile unsigned long Period;
extern volatile int AngleLatch;
extern volatile unsigned int TMR2Latch;
tFlashCtrl FlashCtrl;
extern volatile unsigned int CNState;
extern volatile unsigned int CNStateOld;
extern volatile signed int MinPeriod;

inline void GetHallAngleDirAuto_Inline (void)
{
    if(FindHallAngleCnt == 9)
    {
        switch (HallState)
        {
            case 5:
                if(OldHallState == 4)  // angle increases by 5 1 3 2 6 4 5 1 3 2 6 4
                    uGF.bit.DirectionAuto = 0;                
                else if(OldHallState == 1)                  // angle increases by 5 4 6 2 3 1
                    uGF.bit.DirectionAuto = 1;
                else
                //    TestOut2 = !TestOut2;
                break;
            case 1:
                if(OldHallState == 5)
                {
                    uGF.bit.DirectionAuto = 0;                
                }
                else if(OldHallState == 3)
                {
                    uGF.bit.DirectionAuto = 1;                  
                }
                else
                //    TestOut2 = !TestOut2;

                break;
            case 3:
                if(OldHallState == 1)
                {
                    uGF.bit.DirectionAuto = 0;
                }
                else if(OldHallState == 2)
                {
                    uGF.bit.DirectionAuto = 1;                
                }
                else
                 //   TestOut2 = !TestOut2;          
                break;
            case 2:
                if(OldHallState == 3)
                {
                    uGF.bit.DirectionAuto = 0;                
                }
                else if(OldHallState == 6)
                {
                    uGF.bit.DirectionAuto = 1;                
                }
                 else
                //    TestOut2 = !TestOut2;          
                break;
            case 6:
                if(OldHallState == 2)
                {
                    uGF.bit.DirectionAuto = 0;                
                }
                else if(OldHallState == 4)
                { 
                    uGF.bit.DirectionAuto = 1;                
                }
                else
                //    TestOut2 = !TestOut2;           
                break;
            case 4:
                if(OldHallState == 6)
                {
                    uGF.bit.DirectionAuto = 0;                
                }
                else if(OldHallState == 5)
                {
                    uGF.bit.DirectionAuto = 1;                
                }
                else
                //    TestOut2 = !TestOut2;
                break;
            default:

                break;
        }
        
    }

}
//------------------------------------------------------------------------------
// Hurst Long: Iq_ref>0 --> CCW:513264 (face the shaft end)
//------------------------------------------------------------------------------
inline void GetHallAngleAuto_Inline (void)
{
    switch (HallState)
    {
        case 5:
            if(OldHallState == 4)  // angle increases by 5 1 3 2 6 4 5 1 3 2 6 4
                uGF.bit.Direction = 0;                
            else if(OldHallState == 1)                  // angle increases by 5 4 6 2 3 1
                uGF.bit.Direction = 1;
            else
             //   TestOut2 = !TestOut2;
            break;
        case 1:
            if(OldHallState == 5)
            {
                uGF.bit.Direction = 0;                
            }
            else if(OldHallState == 3)
            {
                uGF.bit.Direction = 1;                  
            }
            else
             //   TestOut2 = !TestOut2;

            break;
        case 3:
            if(OldHallState == 1)
            {
                uGF.bit.Direction = 0;
            }
            else if(OldHallState == 2)
            {
                uGF.bit.Direction = 1;                
            }
            else
             //   TestOut2 = !TestOut2;         
            break;
        case 2:
            if(OldHallState == 3)
            {
                uGF.bit.Direction = 0;                
            }
            else if(OldHallState == 6)
            {
                uGF.bit.Direction = 1;                
            }
             else
             //   TestOut2 = !TestOut2;          
            break;
        case 6:
            if(OldHallState == 2)
            {
                uGF.bit.Direction = 0;                
            }
            else if(OldHallState == 4)
            { 
                uGF.bit.Direction = 1;                
            }
            else
             ///   TestOut2 = !TestOut2;          
            break;
        case 4:
            if(OldHallState == 6)
            {
                uGF.bit.Direction = 0;                
            }
            else if(OldHallState == 5)
            {
                uGF.bit.Direction = 1;                
            }
            else
            //    TestOut2 = !TestOut2;
            break;
        default:

            break;
    }        
    OldHallState = HallState;    
    if(uGF.bit.Direction == uGF.bit.DirectionAuto )
       HallAngle = FindHallAngle.Pos[HallState] + 16384;
    else
       HallAngle = FindHallAngle.Pos[HallState] + 10923 + 16384; 
    
    ParkParm.qAngle = HallAngle;
        
    
}

inline void GetInitHallAngleAuto_Inline(void)
{
    HallState = 0;
    HallState = (unsigned int)I_HALL_U_PIN + ((unsigned int)I_HALL_V_PIN * 2) + ((unsigned int)I_HALL_W_PIN * 4);
    HallAngle = FindHallAngle.Pos[HallState] + 5461 + 16384;
    ParkParm.qAngle = HallAngle;  // 30 degrees shift
}

inline void CalculateParkAngleHall(void)
{
    static int PeriodFilter;
    static long int PeriodStateVar;
    if(uGF.bit.FindHallStart)
    {
        if (Startup_Lock < MotorAlignLockTime)
        {
            Startup_Lock += 1; // This variable is incremented until
            // lock time expires, them the open loop
            // ramp begins
            ParkParm.qAngle = 0;    // test rotor alignment
            FindHallAngleCnt = 0;
        }
        else      
            ParkParm.qAngle += 1;

    }
    else
    {
#ifdef SixStepStart        
        if(abs(Speed) <= Q15(0.1))
        {
            AngleStepByPWM = 0;            
            if (uGF.bit.Direction == uGF.bit.DirectionAuto)
                HallAngle = FindHallAngle.Pos[HallState] + 5461 + 16384;// HURST@CW

            else
                HallAngle = FindHallAngle.Pos[HallState] + 5461 + 16384;// Hurst@CCW

            ParkParm.qAngle = HallAngle;
           // Still calculate the Period filter for the transition to sinewave                               
         //---------------------------------------------------------------------
           
            PeriodStateVar+= (((long int)Period - (long int)PeriodFilter)*100);
            if(PeriodStateVar >= 30000*32768)
                PeriodStateVar = 30000*32768;
            else if(PeriodStateVar <= -30000*32768)
                PeriodStateVar = -30000*32768;
            PeriodFilter = (int)(PeriodStateVar>>15);  
            TempLong = __builtin_divud(AngleStepLong,(unsigned int)PeriodFilter);	
        //----------------------------------------------------------------------    
            //AngleStepByPWM = TempLong;   
                     
        }

       else
       {          
         //----------------------------------------------------------------------
            PeriodStateVar+= (((long int)Period - (long int)PeriodFilter)*100);
            if(PeriodStateVar >= 30000*32768)
                PeriodStateVar = 30000*32768;
            else if(PeriodStateVar <= -30000*32768)
                PeriodStateVar = -30000*32768;
            PeriodFilter = (int)(PeriodStateVar>>15);  
            
            if(PeriodFilter < MinPeriod)
                PeriodFilter = MinPeriod + 1;
            TempLong = __builtin_divud(AngleStepLong,(unsigned int)PeriodFilter);	
            
          //------------------------------------------------------------------------
          //  TempLong = __builtin_divud(AngleStepLong,(unsigned int)Period);	
         //-------------------------------------------------------------------------   
            AngleStepByPWM = TempLong;     
            if (uGF.bit.Direction == uGF.bit.DirectionAuto)
            {
                ParkParm.qAngle += AngleStepByPWM;  
                if((ParkParm.qAngle - HallAngle) > 10922)
                    ParkParm.qAngle = HallAngle + 10922;
            }
            else
            {
                ParkParm.qAngle -= AngleStepByPWM;  
                if((HallAngle - ParkParm.qAngle) > 10922)
                    ParkParm.qAngle = HallAngle - 10922;        
            }            
       }    
#else 
    // Start without Six-Step driving
    //----------------------------------------------------------------------
       PeriodStateVar+= (((long int)Period - (long int)PeriodFilter)*100);
       if(PeriodStateVar >= 30000*32768)
           PeriodStateVar = 30000*32768;
       else if(PeriodStateVar <= -30000*32768)
           PeriodStateVar = -30000*32768;
       PeriodFilter = (int)(PeriodStateVar>>15);  

       if(PeriodFilter < MinPeriod)
           PeriodFilter = MinPeriod + 1;
       TempLong = __builtin_divud(AngleStepLong,(unsigned int)PeriodFilter);	

     //------------------------------------------------------------------------
     //  TempLong = __builtin_divud(AngleStepLong,(unsigned int)Period);	
    //-------------------------------------------------------------------------   
       AngleStepByPWM = TempLong;     
       if (uGF.bit.Direction == uGF.bit.DirectionAuto)
       {
           ParkParm.qAngle += AngleStepByPWM;  
/****  Comment out the 2 line below to disable the boundary check  ****/         
           if((ParkParm.qAngle - HallAngle) > 10922)
               ParkParm.qAngle = HallAngle + 10922;
       }
       else
       {
           ParkParm.qAngle -= AngleStepByPWM;  
/****  Comment out the 2 line below to disable the boundary check  ****/             
           if((HallAngle - ParkParm.qAngle) > 10922)
               ParkParm.qAngle = HallAngle - 10922;        
       }            
                 
               
#endif
        
    }
 
}
inline void FindHallPosition_Inline(void)
{
       
    //HallState = 0;
    //HallState = (unsigned int)HALL_A + ((unsigned int)HALL_B * 2) + ((unsigned int)HALL_C * 4);
    HallState = CNState;
    if(OldHallState != HallState)
    {
        if((FindHallAngleCnt >= 3)&&(FindHallAngleCnt <= 8))   // Get middle part of Hall
            FindHallAngle.Pos[HallState] = AngleLatch; 
        GetHallAngleDirAuto_Inline();
        OldHallState = HallState;        
        if(++FindHallAngleCnt >= 12)
        {
            uGF.bit.FindHallStart = 0; 
            FindHallAngleCnt = 0;
            HallAngle = FindHallAngle.Pos[HallState] + 5461 + 16384;
            ParkParm.qAngle = HallAngle;  // 30 degrees shift 
            FlashCtrl.WriteFlashStart = 1;
            uGF.bit.RunMotor = 0;
        }            
    }
}     
