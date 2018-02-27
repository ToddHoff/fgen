// NOTE: this code is auto generated. Your changes will be 
// written over. Derive from the base class to make changes.


#include "Osencap/LnLockGuard.h"
#include "Osencap/LnOsTime.h"
#include "SyncSmBase.h"

#define RE_SYNCING_SLEEP_SEC 10


SyncSmBase::SyncSmBase(Module* module, const char* pDebugId)
   : mProtection(LN_SEM_Q_PRIORITY, false/*no priinv*/)
{
   mState= OUT_OF_SYNC;
   mPrevState= OUT_OF_SYNC;
   mPrevDifferentState= OUT_OF_SYNC;
   mDebugId= pDebugId;
   mpModule= module;

}

LnStatus
SyncSmBase::GoOutOfSync()
{
   D(mpModule, Output::LEVEL_L2, mDebugId << ": " << "SyncSmBase:GoOutOfSync:start");
   LnStatus rc= LN_OK;

   LnLockGuard lock(mProtection);
   switch (CurrentState())
   {
   case SYNCING:
      {
         D(mpModule, Output::LEVEL_L2, mDebugId << ": " <<  "SYNCING: NEXT=OUT_OF_SYNC");
      NextState(OUT_OF_SYNC);
      break;
      }
   break;

   case SYNCED:
      {
         D(mpModule, Output::LEVEL_L2, mDebugId << ": " <<  "SYNCED: NEXT=OUT_OF_SYNC");
      NextState(OUT_OF_SYNC);
      break;
      }
   break;

   default:
   {
      D(mpModule, Output::LEVEL_L2, mDebugId << ": " << "UNHANDLED EVENT=GoOutOfSync");
   }
   break;

   }// switch

   return rc;

}// GoOutOfSync


LnStatus
SyncSmBase::DoneSyncing()
{
   D(mpModule, Output::LEVEL_L2, mDebugId << ": " << "SyncSmBase:DoneSyncing:start");
   LnStatus rc= LN_OK;

   LnLockGuard lock(mProtection);
   switch (CurrentState())
   {
   case SYNCING:
      {
         D(mpModule, Output::LEVEL_L2, mDebugId << ": " <<  "SYNCING: NEXT=SYNCED");
      NextState(SYNCED);
      break;
      }
   break;

   default:
   {
      D(mpModule, Output::LEVEL_L2, mDebugId << ": " << "UNHANDLED EVENT=DoneSyncing");
   }
   break;

   }// switch

   return rc;

}// DoneSyncing


LnStatus
SyncSmBase::StartSyncing()
{
   D(mpModule, Output::LEVEL_L2, mDebugId << ": " << "SyncSmBase:StartSyncing:start");
   LnStatus rc= LN_OK;

   LnLockGuard lock(mProtection);
   switch (CurrentState())
   {
   case OUT_OF_SYNC:
      {
         D(mpModule, Output::LEVEL_L2, mDebugId << ": " <<  "OUT_OF_SYNC: NEXT=SYNCING");
      NextState(SYNCING);
      break;
      }
   break;

   case SYNCING:
      {
         D(mpModule, Output::LEVEL_L2, mDebugId << ": " <<  "SYNCING: NEXT=SYNCING");
         SLEEP (RE_SYNCING_SLEEP_SEC*1000);
      NextState(SYNCING);
      break;
      }
   break;


   default:
   {
      D(mpModule, Output::LEVEL_L2, mDebugId << ": " << "UNHANDLED EVENT=StartSyncing");
   }
   break;

   }// switch

   return rc;

}// StartSyncing


void
SyncSmBase::NextState(State state)
{
   // Call any code that should executed when leaving a state.
   // Don't trigger call if transitioning to the same state.
   //  
   DoOnExit(CurrentState());

   // Set the prvious sate.
   mPrevState= mState;

   // Set the prvious sate.
   mPrevDifferentState= mState;

   // Set the new state.
   mState= state;

   SmChangedState();

   // Call any code that should executed when entering a state.
   // Don't trigger call if transitioning to the same state.
   //
   DoOnEntry(CurrentState());

}// NextState


void
SyncSmBase::DoOnEntry(State state)
{

   LnStatus rc= LN_OK;

   switch (state)
   {
   case SYNCED:
      rc= MoveSynced();
   break;

   case OUT_OF_SYNC:
      rc= MoveOutOfSync();
   break;

   case SYNCING:
      rc= MoveSyncing();
   break;


	default:
		break;

   }// switch

}// DoOnEntry


void
SyncSmBase::DoOnExit(State /*state*/)
{

}// DoOnExit


LnStatus  
SyncSmBase::InjectEvent(const char* pEventName)
{
   if (strcmp("GoOutOfSync", pEventName) == 0)
      return FwdEvent(new SyncSmBaseAction(this, &SyncSmBase::GoOutOfSync));

   if (strcmp("DoneSyncing", pEventName) == 0)
      return FwdEvent(new SyncSmBaseAction(this, &SyncSmBase::DoneSyncing));

   if (strcmp("StartSyncing", pEventName) == 0)
      return FwdEvent(new SyncSmBaseAction(this, &SyncSmBase::StartSyncing));


   return LN_FAIL;

}// InjectEvent


const char*
SyncSmBase::CurrentStateName(void) const
{
   switch (CurrentState())
   {
   case OUT_OF_SYNC: return "OUT_OF_SYNC";
   case SYNCING: return "SYNCING";
   case SYNCED: return "SYNCED";

   default:
      break;

   }// switch

   return 0;

}// CurrentStateName()
