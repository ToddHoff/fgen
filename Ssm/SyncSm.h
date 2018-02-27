#ifndef _SyncSm_h_
#define _SyncSm_h_

#include "Ssm/SsmSmBase.h"      // ISA Ssm state machine
#include "Ssm/SyncSmBase.h"     // ISA Sync state machine
#include "Util/Typeable.h"      // ISA typeable
#include "Com/MsgHandler.h"     // HASA handler
#include "rw/cstring.h"         // HASA string
#include "Util/LnTime.h"
 

/**
 * SyncSm is the base class for slaves syncing from a master sync source, using 
 * the Ssm framework. The purpose of SyncSm is to provide a very easy 
 * interface for developers wanting to use the sync state machine in the Ssm 
 * framework. It takes care of most all the work interfacing to the Ssm 
 * framework and Sync state machine. See the Sync.sm for a description
 * of the Sync state machine.
 *
 * <H3> Using SyncSm </H3>
 *
 * <H4> Derive a Class from SyncSm </H4>
 *
 * Derive a class from SyncSm and implement all state machine action methods.
 * Currently the action methods are: MoveSyncing. This method should know
 * how to sync data from an appropriate source using an appropriate protocol.
 * The Sync state machine does not help with the sync protocol, that's an
 * application issue.
 *
 * The derived class will probably require a pointer to a context passed
 * in via the constructor. Usually an Actor object would be the context.
 *
 * <H4> Make it Part of an Actor </H4>
 *
 * Make your class derived from SyncSm a member attribute of an Actor object. 
 * State machines do not have their own thread context and require some other 
 * object to provide a thread context. Objects other than Actor can be used, 
 * but Actors and Ssm state machines play well together.
 *
 *
 * <H3> Thread Safety </H3>
 *
 * Using SyncSm with an Actor guarantees that all state machine actions
 * are executed in the Actor's thread. Events are automatically queued
 * to an Actor and are then automatically executed by the Actor in the
 * Actor's thread. Because events cause actions to be called, actions
 * will always be executed in the Actor's thread.
 *
 *
 * <H3> Issue Your Own Events </H3>
 *
 * The Ssm coordinator is not the only object that can issue events
 * to a state machine. Events can be emitted by anyone. The available
 * events can be found in class SyncSmBase. <B>DO NOT</B> call events
 * directly, this could possible cause multi-threading problems. Instead
 * call <B>InjectEvent</B>, passing it the name of the event. For
 * example:  
 * <PRE>
 * LnStatus rc= InjectDoneSyncing()
 * </PRE>
 *
 * <H3> Handling Sync Failures </H3>
 *
 * If while syncing a failure occurs then start syncing again using 
 * the event <B>StartSyncing</B>. This will transition the state machine 
 * from state SYNCING back to state SYNCING. The implementation of
 * MoveSyncing will have to handle this transition.
 *
 *
 * <H3> Indicating Syncing has Completed </H3>
 *
 * All the events in the SyncSmBase state machine are available
 * to SyncSm. To indicate the sync process has completed successfully
 * invoke the event method <B>DoneSyncing</B>. For example:
 * <PRE>
 * InjectDoneSyncing();
 * </PRE>
 *
 *
 * <H3> Pull Protocol </H3>
 *
 * Syncing data from a source can require a lot of data be downloaded.
 * It's not possible for a data source to just blast out (push) hundreds
 * of data messages and expect sync clients will get all the data. 
 * A push protocol is possible when only one message will be sent to
 * this client. If more that one message will be sent then the client
 * has no idea when the data source is done sending all its data and
 * it has no idea if it got all of the data.
 *
 * Instead, a "cursor" protocol should be used to <B>pull</B> data from
 * the source.. A cursor protocol allows the client to download data at a 
 * rate controlled by the client. For a standard cursor protocol
 * see the cursor protocol specified by component Ps.
 *
 *
 * <H3> Master Sync Source Behaviour </H3>
 *
 * The use of a cursor protocol simplifies the master data supplier's
 * behaviour considerably. The client, say during a sync failure,
 * can ask the master again for the first batch of data. This would
 * tell the master to restart the cursor. No complicated master
 * state machine is necessary.
 *
 *
 * See the package documentation for Ssm information.
 *
 * @see Ps
 */
class SyncSm : public SsmSmBase, public SyncSmBase
{
public:
	/** 
	 * Construct the state machine.
	 *
	 * @param pSsmName The globally unique name of the the Ssm state 
	 *    machine. Memory: COPIED.
	 * @param rHandler The message handler to which Action objects are queued.
	 *    It must queue to a separate thread so all state machine actions
	 *    can be executed in the correct thread context. The Action object
	 *    encapsulates event method invocations on the state machine.
	 *    If the handler is not an Actor the handler must call the Action's
	 *    Doit method in order trigger the event. This will inturn call any
	 *    state machine action methods associated with the transition.
	 *    Derived classes are expected to implement
	 *    state machine actions. See class documentation for more details.
	 * @param pModule Optional module pointer, used to associate the state
	 *    machine with a debug controller.
	 *
	 * @see Action
	 * @see #FwdMoveSyncing
	 */
	SyncSm(const char* pSsmName, MsgHandler& rHandler, Module* pModule= 0);


   /**
    * Implement move syncing behaviour. Derived class are expected
	* to implement this method and provide any behaviour associated
	* with the slave syncing from the master.
    *
    * @return LnStatus
    */
	virtual LnStatus        MoveSyncing(void);

	virtual LnStatus        MoveOutOfSync(void);

	virtual LnStatus        MoveSynced(void);


	/**
	 * Return the name of the Fsm.
	 *
	 * @return The name of the Fsm.
	 */
   virtual const char* SsmName(void) { return mName.data(); };


   /**
    * Inject the event DoneSyncing.
	*/
	virtual LnStatus InjectDoneSyncing(void)
	{ return InjectEvent("DoneSyncing"); }


   /**
    * Inject the event StartSyncing.
	*/
	virtual LnStatus InjectStartSyncing(void)
	{ return InjectEvent("StartSyncing"); }


   /**
    * Inject the event GoOutOfSync.
	*/
	virtual LnStatus InjectGoOutOfSync(void)
	{ return InjectEvent("GoOutOfSync"); }

    /**
     * Set and Get IsAlreadySynced
     */
    bool IsAlreadySynced() const {return mIsAlreadySynced;}
    void SetAlreadySynced(bool flag) {mIsAlreadySynced=flag;}


protected:


// SyncSmBase Interface

   /**
	* This method is called by the Sync state machine to properly forward the
	* state machine event to the correct thread context. In our case this
	* context is represented generically by the message handler. 
	*
    * @return LnStatus
    */
   virtual LnStatus        FwdEvent(Action* action) const;


   /**
    * This method is called when the Sync state machine has changed state.
	* This method acts as an adaptor between the Sync state machine and the
	* Ssm state machine.
	* When it is called the Ssm listener is called to notifify the coordinator
	* a state change has occurred.
	*/
   virtual void            SmChangedState();

public:

// SsmSmBase Interface

	/**
	 * Tell the FSM an event has occurred. It is expected to turn
	 * the named event into a "real" event in the state machine.
	 * This method acts as an adaptor between the Sync state machine and the
	 * Ssm state machine.
	 *
	 * @param pEventName the name of the event to drive the state machine.
	 *
	 * @return LN_OK If the event was recognized and the transition was valid.
	 * @return LN_FAIL If the event wasn't recognized or no valid transition
	 *    was available for the event.
	 */
	virtual LnStatus  SsmEvent(const char* pEventName) { return InjectEvent(pEventName); }


	/**
	 * Return the current state of the state machine. 	 
	 * This method acts as an adaptor between the Sync state machine and the
	 * Ssm state machine.
	 *
	 * @return The current state of the state machine.
	 */
	virtual const char* SsmCurrentState(void) const { return CurrentStateName(); }


//--- Start Changes for Performance analysis ----------------

	//Override the InjectEvent syncSmBase Class function
	//So we can capture the StartSync and DoneSync Times
	//And Put the ssm Name onto an iterator list for use
	//In Dcraft command to dump individual ssm execution times
    virtual LnStatus        InjectEvent(const char* pEventName);


	/**
	 * Return the  name of the state machine. 	 
	 * For use by Dcraft or debug commands to
	 * dump out the SSM timing report.
	 *
	 * @return the name of the state machine.
	 */
	const char*   GetSsmName(void) const;


	/**
	 * Return the Elapsed Sync Time for the state machine.
	 *
	 * @return The sync time for the state machine.
	 */
	void GetSsmSyncTime(LnTime&);

	/**
	 * Return the  Start Time of the state machine.
	 *
	 * @return the Start Time of the state machine.
	 */
	const LnTime& GetStartSyncTime(void) const;
	const int GetStartSyncCount(void) const;


	/**
	 * Return the Stop/Done Time for the state machine.
	 *
	 * @return the Stop/Done Time for the state machine.
	 */
	const LnTime& GetStopSyncTime(void) const;
	const int GetStopSyncCount(void) const;


	/**
	 * Return the Stop/Done Time for the state machine.
	 *
	 * @return the Stop/Done Time for the state machine.
	 */
	const LnTime& GetGoOutOfSyncTime(void) const;
	const int GetGoOutOfSyncCount(void) const;


//--- End   Changes for Performance analysis ----------------


private:
	RWCString   mName;
	MsgHandler& mrHandler;
    bool        mIsAlreadySynced;

//--- Start Changes for Performance analysis ----------------
//	  Add TimeStamps members StartSyncTime and DoneSyncTime
//	  For later dumping via a Dcraft command

	LnTime mStartSyncTime;
	LnTime mDoneSyncTime;
	LnTime mGoOutOfSyncTime;
    int mStartSyncCount;
    int mStopSyncCount;
	int mGoOutOfSyncCount;
//--- End   Changes for Performance analysis ----------------
};




inline	const char*
SyncSm::GetSsmName(void) const
{
   return ((const char *)mName);

}// GetSsmName


inline void   
SyncSm::GetSsmSyncTime(LnTime& rdiffTime)
{
     rdiffTime = LnTime(mDoneSyncTime.Milliseconds() - mStartSyncTime.Milliseconds());
	 return;

}// GetSsmSyncTime


inline	const LnTime&
SyncSm::GetStartSyncTime(void) const
{
   return mStartSyncTime;

}// GetStartSyncTime


inline	const LnTime&
SyncSm::GetGoOutOfSyncTime(void) const
{
   return mGoOutOfSyncTime;

}// GetGotOutOfSyncTime



inline	const LnTime&
SyncSm::GetStopSyncTime(void) const
{
   return mDoneSyncTime;

}// GetStopSyncTime


inline	const int
SyncSm::GetStartSyncCount(void) const
{
   return mStartSyncCount;
 
}// GetStartSyncCount


inline	const int
SyncSm::GetGoOutOfSyncCount(void) const
{
   return mGoOutOfSyncCount;

}// GetGetOutOfSyncCount


inline	const int
SyncSm::GetStopSyncCount(void) const
{
   return (mStopSyncCount);

}// GetStopSyncCount


#endif // _SyncSm_h_