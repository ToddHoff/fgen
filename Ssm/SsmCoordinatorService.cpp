
#include "Ssm/SsmCoordinatorService.h"  // class implemented
#include "Osencap/LnLockGuard.h"  
#include "Util/LnException.h"             // USES exception
#include "Util/Dbc.h"                     // USES design by contract
   

// CLASS SCOPE
//
SsmCoordinatorActor*  SsmCoordinatorService::mpSingleton;
static LnMutex  protection;    // mutext protecting singleton

SsmCoordinatorActor*
SsmCoordinatorService::Singleton(Properties* cfg)
{

    // First check to see if the singleton has been created.
    //
    if (mpSingleton == 0)
    {
        // Block all but the first creator.
        //
        LnLockGuard lock(protection);

        // Check again just in case someone had created it
        // while we were blocked.
        //
        if (mpSingleton == 0)
        {
            // Create the singleton SsmCoordinatorActor object. It's assigned
            // to a temporary so other accessors don't see
            // the singleton as created before it really is.
            //
            SsmCoordinatorActor* inprocess_singleton= new SsmCoordinatorActor(cfg);
			if (LnException::IsException()) return 0;

			// Don't proceed until the actor has moved to start state. It moves
			// to start state on its own.
			//
			LnStatus status= inprocess_singleton->WaitTil(Module::MODULE_STATE_SYSTEM_START);
 			SET_XCEPTION_IF(status != LN_OK, 0, "MOVE_STATE_SYSTEM_START","SYSTEM_START", "SsmCoordinatorService", 0,  *inprocess_singleton);

			// Move to state initializing. Let the actor initialize itself.
			//
			inprocess_singleton->MoveState(Module::MODULE_STATE_INITIALIZING);
			status= inprocess_singleton->WaitTil(Module::MODULE_STATE_INITIALIZING);
			SET_XCEPTION_IF(status != LN_OK, 0, "MOVE_STATE_ERROR","INITIALIZING", "SsmCoordinatorService", 0,  *inprocess_singleton);
 
			// Move to state primary.
			//
			inprocess_singleton->MoveState(Module::MODULE_STATE_PRIMARY);
			status= inprocess_singleton->WaitTil(Module::MODULE_STATE_PRIMARY);
			SET_XCEPTION_IF(status != LN_OK, 0, "MOVE_STATE_PRIMARY","PRIMARY", "SsmCoordinatorService", 0,  *inprocess_singleton);

            // The singleton is created successfully so assign it.
            //
            mpSingleton= inprocess_singleton;

      }// still not created

   }// not created

   // Return the created singleton.
   //
   return mpSingleton;

}// Singleton


void
SsmCoordinatorService::Singleton(SsmCoordinatorActor* pSsm)
{
	REQUIRE(mpSingleton == 0);

	mpSingleton= pSsm;

}// Set