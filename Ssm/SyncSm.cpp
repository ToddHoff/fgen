
#include "Ssm/SyncSm.h"			// class implemented
#include "Util/Bits64Util.h"
#include "Util/Dbc.h"			// USES


SyncSm::SyncSm(const char* pName, MsgHandler& rHandler, Module* pModule)
	:	SyncSmBase(pModule),
		mName(pName),
		mrHandler(rHandler),
        mIsAlreadySynced(false),
        mStartSyncCount(0),
		mStopSyncCount(0),
		mGoOutOfSyncCount(0)
{
	REQUIRE(pName);

}// SyncSm



LnStatus        
SyncSm::FwdEvent(Action* action) const
{
    LnStatus rc;
	rc = mrHandler.HandleMsg(action);
    if (rc != LN_OK) CHECK(0);

    return rc;

}// FwdMoveSyncing 





void
SyncSm::SmChangedState()
{
	// Tell the Sfsm observers a state has changed.
	//
	NotifyObservers(this, "SM_MOVED");

}// SmChangedState



LnStatus        
SyncSm::MoveSyncing(void) 
{ 
	D1(mpModule, "SyncSm::MoveSyncing: ssm=" << SsmName());

	return InjectDoneSyncing();
 
}// MoveSyncing


LnStatus        
SyncSm::MoveOutOfSync(void) 
{ 
	D1(mpModule, "SyncSm::MoveOutOfSyncing: ssm=" << SsmName());
    LMSG("SyncSm::MoveOutOfSync: ssm="<<SsmName());

	return LN_OK; 

}// MoveOutOfSync


LnStatus        
SyncSm::MoveSynced(void) 
{ 
	D1(mpModule, "SyncSm::MoveSynced: ssm=" << SsmName());

    SetAlreadySynced(true);

	return LN_OK; 

}// MoveSynced





//--- Start Changes for Performance analysis ----------------
//    Override this function to capture 
//    - StartSyncing
//    - DoneSyncing 
//    TimeStamps and counts in the correspond ssm object.


LnStatus  
SyncSm::InjectEvent(const char* pEventName)
{
    if (strcmp("DoneSyncing", pEventName) == 0)
    {
        // Get timestamp & count for Stop Syncing.
	    mDoneSyncTime = LnTime::Now();
        mStopSyncCount++;
        return FwdEvent(new SyncSmBaseAction(this, &SyncSmBase::DoneSyncing));
    }
    else if (strcmp("StartSyncing", pEventName) == 0)
    {
        // Get timestamp & count for Start Syncing.
        mStartSyncTime = LnTime::Now();
        mStartSyncCount++;

        return FwdEvent(new SyncSmBaseAction(this, &SyncSmBase::StartSyncing));
    }
    else if (strcmp("GoOutOfSync", pEventName) == 0)
    {
        // Get timestamp & count for Start Syncing.
        mStartSyncTime = LnTime::Now();
        mStartSyncCount++;

        return FwdEvent(new SyncSmBaseAction(this, &SyncSmBase::GoOutOfSync));
    }
	else
	{
		REQUIRE(0);
	}


    return LN_FAIL;

}// InjectEvent





//--- End   Changes for Performance analysis ----------------
