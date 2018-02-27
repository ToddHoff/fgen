//////////////////////////////////////////////////////////////////////
// FILE             : SsmCoordinatorTimer.h
//
// AUTHOR           : Helen Mao 09/20/03
//
//
//          Copyright (c) 2003 Ciena Corporation Inc.
//                      All Rights Reserved
//
//  No part of this program may be photocopied, reproduced, or
//  translated to another programming language without the prior
//  written consent of Ciena Corporation, Inc
//////////////////////////////////////////////////////////////////////
#ifndef _SsmCoordinatorTimer_h_
#define _SsmCoordinatorTimer_h_

#include "Project\LnTypes.h"            // USES LnStatus
#include "TimerUtil\LnTmTimer.h"        // ISA LnTmTimer

class SsmCoordinatorActor;

#define SSM_SYNC_TIMEOUT     600000  // 10 minutes
#define SSM_CM_SYNC_TIMEOUT  1200000 // 20 minutes
#define SSM_TM_SYNC_TIMEOUT  600000  // 10 minutes

///////////////////////////////////////////////////////////////////////////////
    /**
 *  SsmCoordinatorTimer extends LnTmTimer Class to implement timers for SsmCoordinator 
 *  It is used to fix the message loss problem in R3.0 SSM.
     */
///////////////////////////////////////////////////////////////////////////////
class SsmCoordinatorTimer : public LnTmTimer
{
public:


    /*
     * Creates the object. Timer is not started
     * @param rServer -- reference to the SsmCoordinatorActor class.
     */
    SsmCoordinatorTimer(SsmCoordinatorActor* pSsm,int timeOut=SSM_SYNC_TIMEOUT); 

    /*
     * Destructor for the object
     */
    virtual  ~SsmCoordinatorTimer(){};

    /*
     * Starts the  timer
     */
    LnStatus  StartTimer(void);

    /*
     * Starts the timer with given timeout period
     *
     * @param timeOut -- Timeout period for the timer  
     */
    LnStatus  StartTimer(int timeOut);


    /*
     * Cancel the timer
     */
    LnStatus CancelTimer();

    /*
     * Set the time out value
     */
    void SetTimeOutValue(int t);

    /**
    *   Handle the timer expiry. Virtual function in parent class. 
    *   Implemented by SsmCoordinatorTimer Class.
    *   @param None
    *   @return  None
    */
    virtual void    HandleTimer();


private:
    SsmCoordinatorActor* mpSsm;
    int mTimeOut;
}; 

#endif // _SsmCoordinatorTimer_h_
