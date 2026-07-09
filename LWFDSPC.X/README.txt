This readme document tells the correct motor and hall connections to the board.

MOTOR and HALL CONNECTIONS:
-------------------------------------------------------------------------------
|Board Terminals     |       Wires Coming From the Motor
|   A                |              Phase A
|   B                |              Phase C
|   C                |              Phase B
-------------------------------------------------------------------------------
|   Hall A           |              Hall A
|   Hall B           |              Hall C
|   Hall C           |              Hall B
-------------------------------------------------------------------------------

This code has a provision for real time diagnostics using X2CScope.
For testing and diagnostic purposes, the user needs to uncomment all these Functions: 
    DiagnosticsInit();      --located in the initialization
    DiagnosticsStepMain();  --          inside the while loop
    DiagnosticsStepIsr();   --          in the _ADCAN17Interrupt() 