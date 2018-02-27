//////////////////////////////////////////////////////////////////////
// FILE             : SsmCoordiantorTimer.cpp
//
// AUTHOR           : Helen Mao 09/20/03
//
// Revision HISTORY : 
//
//          Copyright (c) 2003  Ciena Corporation Inc.
//                      All Rights Reserved
//
//  No part of this program may be photocopied, reproduced, or
//  translated to another programming language without the prior
//  written consent of Ciena Corporation, Inc.
//////////////////////////////////////////////////////////////////////

#include "SsmCoordinatorTimer.h"

#include "Util\Log.h"                       // USES Logging
#include "Util\Dbc.h"                       // USES design-by-contract
#include "SsmCoordinatorActor.h"

///////////////////////////////////////////////////////////////////////////////
// SSM Coordinator Timer Class Implementation
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
//Function: SsmCoordinatorTimer::SsmCoordinatorTimer
SsmCoordinatorTimer::SsmCoordinatorTimer(SsmCoordinatorActor* pSsm,int timeOut)
               :LnTmTimer(pSsm),
                mpSsm(pSsm),
                mTimeOut(timeOut)
{

}



//=============================================================================
//Destructor
//SsmCoordinatorTimer::~SsmCoordinatorTimer() 
//{

//}



//=============================================================================
//Function: StartTimer
LnStatus
SsmCoordinatorTimer::StartTimer(void)
{
    LMSG("SsmCoordinator Start Timer\n");
	return Start(mTimeOut);
}



//=============================================================================
//Function: StartTimer
LnStatus
SsmCoordinatorTimer::StartTimer(int timeOut)
{
    LMSG("SsmCoordinator Start Timer\n");
	return Start(timeOut);
}

//=============================================================================
//Function: CancelTimer
LnStatus
SsmCoordinatorTimer::CancelTimer()
{
    LMSG("SsmCoordinator Cancel Timer\n");
    return Cancel();
}

//=============================================================================
// Funciton: set time out value
void
SsmCoordinatorTimer::SetTimeOutValue(int t)
{
    if (t<=0)
        t=SSM_CM_SYNC_TIMEOUT;
    mTimeOut = t;
}


//=============================================================================
//Function: SsmCoordinatorTimer::HandleTimer
void
SsmCoordinatorTimer::HandleTimer()
{
    //When Timer expires, we assert.
    LMSG("SsmCoordinator Timer Expired, Reboot the module\n");
    CHECK(0);
}
