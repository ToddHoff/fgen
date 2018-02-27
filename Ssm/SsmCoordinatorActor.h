
#ifndef _SsmCoordinatorActor_h_
#define _SsmCoordinatorActor_h_

#include "Project/LnTypes.h"            // common types
#include "Ssm/SsmSmBase.h"              // USES base fsm
#include "Util/Properties.h"            // HASA properties
#include "Util/Output.h"                // USES output
#include "Actor/Actor.h"                // ISA actor
#include "Ssm/SsmDependencyFactory.h"   // HASA factory
#include "rw/tpslist.h"                 // HASA list
#include "Util/Listener.h"              // ISA listener
#include "rw/tphasht.h"                 // HASA hash table
#include "Com/SubscribeProps.h"         // HASA subscriber props
#include "Com/ImplementProps.h"         // HASA implement props
#include "Com/PublishProps.h"           // HASA publish props
#include "Com/WorkerPool.h"             // HASA worker pool
#include "Com/ComNotification.h"        // USES
#include "Com/AvListMsg.h"				// USES
#include "SsmCoordinatorTimer.h"        // USES timer


// FORWARD REFERENCES
//
class SfmsCoordinatorService;
class SsmConditionFactory;
class SsmTest;
class ReinitializeDependentSsmsJob;
class InjectEventJob;
class ReinitializeSsmJob;
class FailoverTest;
class SsmDependencyFactory;


/**
 * SsmCoordinatorActor implements a distributed dependency service for driving
 * and coordinating distributed state machines. State machines are expected to 
 * register with SsmCoordinatorActor through its singleton interface.
 * When the SsmCoordinatorActor is moved to PRIMARY it will evaluate all
 * conditions and fire the appropriate actions.
 *
 * @see ccdoc.index.pkg.Ssm.html
 */
class SsmCoordinatorActor : public Actor, public Listener
{
public:

// TYPES

	/**
     * Some default configuration parameters for actors.
	 * <B>SSM_TRACE_REQ</B> - Debug level for tracicing requests.
     * <B>SSM_MSGQ_SIZE</B> - The default queue size, in number of  
	 *    messages. Default 300.<BR>
     * <B>SSM_STACK_SIZE</B> - The default stack size, in bytes. Default is 32K.
     */
	enum
	{
		SSM_TRACE_REQ  = Output::LEVEL_START,
		SSM_MSGQ_SIZE  = 1000,
		SSM_STACK_SIZE = 128*1024
	};
 



// LIFECYLE

    /**
     * Create SsmCoordinatorActor.
	 *
	 * Configuration parameters:
     * <OL> 
     * </OL>
     *
     * <B>Preconditions</B><BR>
     *
	 * @param cfg Configuration parameters. Memory: GIVEN.
	 *
	 * @see AaaPs
     * @see #ExitActor
     * @see Actor
     */
    SsmCoordinatorActor(
		Properties*  cfg               = 0,
        const char*  groupModuleName   = "ssm", 
        const char*  uniqueModuleName  = "ssm",
        int          msgQsize          = SSM_MSGQ_SIZE,
        int          stackSize         = SSM_STACK_SIZE, 
        TaskPriority priority          = TASK_DEFAULT_PRIORITY);
        

// OPERATOR


    /**
     * Output object to a given stream.
     *
     * @param s the output stream to write to.
     * @param o the object to print.
     *
     * @return the stream to written to.
     */
    friend ostream&         operator<< (ostream& s, const SsmCoordinatorActor& o);


// OPERATIONS

    /**
     * Output the object to a stream. Because this method is virtual the most
	 * derived class is dumped. Derived classes should
	 * chain together the dump methods for its base classes and any
	 * important member attributes.
	 *
	 * <H3> Example </H3>
	 *
	 * <H4> Chaining Dump Methods </H4>
	 *
	 * In this example class SomeClass is derived from AnotherClass. We need
	 * to chain together the dump methods so we get a full picture
	 * of an object.
	 *
	 * <PRE>
	 * ostream&
	 * SomeClass::Dump(ostream& o, DumpType fmt, int depth)
	 * {
	 *    s &lt;&lt; "SomeClass:\n"
	 *      &lt;&lt; "Attribute=" &lt;&lt; mAttribute << "\n";
	 *    AnotherClass::Dump(s, fmt, depth+1);
	 *    s  &lt;&lt; endl;
	 *    
	 *    return s;
     * }// Dump
	 *
	 * AnotherClass::Dump() is calling the dump method for AnotherClass,
	 * which SomeClass is derived from. This is how the dump methods are 
	 * chained together.
	 * </PRE>
	 *
	 * <H4> Implement the &lt;&lt; Operator Using Dump </H4>
	 *
	 * Dump is nice because it can be overridden by derived classes, which allows
	 * it to dump an entire object hierarchy. The output operator &lt;&lt; can't be 
	 * overridden and will only dump the object the operator is defined for.
	 * But the output operator is very convenient to use. To get the best of both
	 * worlds it's common to implement the &lt;&lt; operator in terms of dump.
	 *
	 * <PRE>
	 * friend ostream& operator&lt;&lt; (ostream& o, const SomeClass& val)
	 * {
	 *	  val.Dump(o);
	 *	  return o;
	 * }
	 * </PRE>
	 *
     * @param s The output stream to write to.
     * @param fmt The type of dump to perform. 
     * @param depth The depth in the object hierarchy. It is mainly used 
     *   for pretty printing output.
     *
     * @return The stream written to.
     */
    virtual ostream&  Dump(ostream& s, DumpType fmt= DUMP_DEBUG , int depth= 0) const;


	/**
	 * Set the dependency rules from a properties string. See the package
	 * documentation for more details. 
	 *
	 * @param pProps A string containing the XML representation of
	 *    properties describing dependencies for the coordinator.
	 *    Memory: BORROWED.
	 *
	 * @see ccdoc.index.pkg.Ssm.html
	 */
	LnStatus	Dependencies(const char* pProps);


	/**
	 * Set the dependency rules from an array of strings. See the package
	 * documentation for more details. This way of setting dependencies
	 * is provided to get around a bug/feature of Visual C++ that fixes
	 * literal string sizes to a max of 2K. A real dependencies list is
	 * larger than 2K. The way around this is to break up up the dependency
	 * list into chunks. The chunks are passed as an array of strings.
	 * The array of strings will be concatenated into one big string.
	 * The last array entry should be 0.
	 *
	 * @param pProps An array of strings containing the XML representation of
	 *    properties describing dependencies for the coordinator. 
	 *    Memory: BORROWED.
	 *
	 * @see ccdoc.index.pkg.Ssm.html
	 */
	LnStatus	Dependencies(const char* pProps[]);



	/**
	 * Set the dependency rules from a properties object. See the package
	 * documentation for more details. 
	 *
	 * @param rProps A properties object describing dependencies for 
	 *    the coordinator. Memory: GIVEN.
	 *
	 * @see ccdoc.index.pkg.Ssm.html
	 */
	LnStatus	Dependencies(Properties& rProps);


	/**
	 * Register a state machine with the coordinator.
	 *
	 * @param rSm The state machine to register. Memory: PBORROWED.
	 */
	LnStatus	Register(SsmSmBase& rSm);


	/**
	 * Dump the Ssm times (Start-time, Stop-time, Elapsed-time
         * to sync up each of the ssms.
	 */
         void DumpSsmTimes(void);

	/**
	 * Dump the Elapsed times to sync up each of the ssms.
	 */
         void DumpSsmElapsedTimes(void);


	/**
	 * Get the source ID assigned to the coordinator.
	 */
	uint32		GetSourceId(void) const
	{ return mSourceId; }


	/**
	 * Get a registered SSM by name.
	 *
	 * @return 0 means not found.
	 */
	SsmSmBase*	GetRegisteredSsm(const char* pSsmName);


	/**
	 * Get the condition for a particular dependency.
	 */
	SsmConditionBase*	GetCondition(
		const char* pDependencyName,
		const char* pConditionName);

    /**
     * Inject an  event to a specific SSM
     *
     * The events includes: OOS and SYNCING
     *
     **/
    void ForceStateChange(const char* pSsmName, const char* state);

    /**
     * Set the sync time out value for SSM subsystem
     **/
    void SetSyncTimeOutValue(int t);


protected:
	// Allow PsService to be a friend so it can destruct us on an error.
	//
	friend SfmsCoordinatorService;
	friend SsmConditionFactory;
	friend ReinitializeDependentSsmsJob;
	friend InjectEventJob;
	friend ReinitializeSsmJob;


	/**
	 * Associate a state machine and condition. Every state change of every 
	 * state machine is published. Conditions that are dependent on a state
	 * machine state change need to register themselves here so they will
	 * be informed of the change. The coordinator proxies for all conditions
	 * in the name space.
	 *
	 * This method called because of Register command. The register command
	 * is queued to the coordinator so it happens in the thread of the
	 * coordinator.
	 *
	 * @param pSsmName The name of the state machine.
	 * @param 
	 */
	void AddSmToConditionAssoc(const char* pSsmName, SsmConditionBase& rCond);


	/**
	 * The coordinator listens for change events for all registered state 
	 * machines.
	 */
	virtual void SubjectChanged (const Typeable* subject,
                                 const char*     changeType );



	/**
	 * Register a state machine's Com operations and events.
	 * Every state machine has a small number of entries
	 * in the name space.
	 *
	 * @param pSmName The name of the state machine.
	 */
	void RegisterSmNameSpace(const char* pSmName);


	/**
	 * Publish the current state of the state machine. This
	 * tells all dependent state machines what the state of
	 * the state machine is.
	 *
	 * @param rSm The state machine having its state published.
	 * @param isInitialization Is the state change because the SSM is
	 *  being initialized.
	 */
	void PublishStateChange(SsmSmBase& rSm, bool isInitialization= false);


   /**
    * Overrided to handle messages sent to this Actor.
    * 
    * Typically only work waiting for a worker is sent to the
    * pool Actor.
    *
    * @param msg The message to handle.
    */
	virtual void            ImplementMsg(LnObject* msg);


	/**
	 * Move Ps to state initializing. Ps registers with com for messages,
	 * starts its worker pool, and creates  its adapater.
	 *
	 * @return LN_FAIL If the actor could not be move to the specified state.
	 * @retrun LN_OK If the actor could move to the specified state.
	 */
	virtual LnStatus ImplementStateInitializing(const char* substate= 0);


	/**
	 * Move Ps to state down. Ps deregisters with com for messages,
	 * stops its worker pool, and deletes its adapater.
	 *
	 * @return LN_FAIL If the actor could not be move to the specified state.
	 * @retrun LN_OK If the actor could move to the specified state.
	 */
	virtual LnStatus ImplementStateDown(const char* substate= 0);


	/**
	 * Currently not implemented.
	 */
	virtual LnStatus ImplementStateSecondary(const char* substate= 0);


	/**
	 * Currently not implemented. State initializing accomplishes everything
	 * we need for now.
	 */
	virtual LnStatus ImplementStatePrimary(const char* substate= 0);


    /**
     * Prevent a destructor from being called.  SsmCoordinatorActor should be killed 
	 * by moving to state destructed.
     */
    virtual ~SsmCoordinatorActor(void);


	/**
	 * Start the primary coordination service. Until then state machines can
	 * only register.
	 */
	void StartCoordinationService();


	/**
	 * Called when a subscription notification arrives.
	 */
	void HandleSubscriptionNotification(ComNotification& rNotification);


	/**
	 * Called when a registration request arrives.
	 */
	void HandleRegisterOp(SsmSmBase& rSm);


	/**
	 * Trigger the start actions to start actioning. Start actions are what
	 * trigger the SSMs so in a way this starts the primary functionality
	 * of the coordinator.
	 */
	void ActivateStartActions(void);


	/**
	 * Tell all SSMs dependent on SSMs running on this coordinator
	 * that they should reinitialize their states because one of
	 * the SSMs they depend on has restarted.
	 */
	void ReinitializeDependentSms();


	/**
	 * Start the resolution of the initial conditions for all registered
	 * state machines.
	 */
	void ResolveInitialConditions(void);


	/**
	 * Should the coordinator process start actions? It's ready when:
	 * 1. the coordinator is primary.
	 * 2. the name server is available.
	 * 3. the number of outstanding subscriptions is 0.
	 */
	bool IsNeedStartCoordinating(void) const;
    bool IsLastSsm(SsmSmBase &rSm);


private:
	friend SsmTest;
	friend FailoverTest;
	friend SsmDependencyFactory;

	Properties*                     mpDependencyProps;
	Properties*                     mpDefaultsProps;
	SsmDependencyFactory*           mpDependencyFactory;
	RWTPtrSlist<SsmDependencyBase>  mDependencyList;   // keep a list of dependencies
	uint32							mSourceId;
    SsmCoordinatorTimer *           mpTimer;

	/**
	 * Get a dependency by name.
	 */
	SsmDependencyBase*	GetDependency(const char* pDependencyName);


	/**
	 * Inject an event into a SSM.
	 */
	LnStatus InjectEvent(
		const char*	pSsmName, 
		uint32		sourceId,
		const char* pEvent);


	/**
	 * Register for name server availability notifications.
	 */
	void RegisterForMonitoredAttributes();


	/**
	 * DeRegister for name server availability notifications.
	 */
	void DeregisterForMonitoredAttributes();


	/**
	 * Trigger the starting of the coordination service by queueing a start
	 * request to this actor. The request is always queued so we can guarantee
	 * any registration requests in the queue are processed before service is
	 * started.
	 */
	void TriggerStartCoordinationService();


	/**
	 * Implementation for the GetState operation.
	 */
	void ImplementGetStateOp(AvListMsg& rReq);


	/**
	 * Implementation for the InjectEvent operation.
	 */
	void ImplementInjectEventOp(AvListMsg& rReq);


	/**
	 * Implementation for the UpdateDependency operation.
	 */
	void ImplementUpdateDependencyOp(AvListMsg& rReq);


	/**
	 * A worker pool is used to serialize som operations.
	 */
	void QueueSerializedWork(Job& rWorkToDo);


	/**
	 * Find the dependency for a ssm from the dependency list.
	 */
	Properties* FindDependencyForSsm(const char* pSsmName);


	/**
	 * Add a new registered SSM into the system.
	 */
	LnStatus AddSsm(SsmSmBase& rSm);


	/**
	 * Maintain a list mapping condition to the state machines the conditions
	 * are dependent on.
	 */
	Properties*                     mpSmToCondMap;


	/**
     * Hash list of registered state machines.
     */
    Properties*                     mpRegisteredSms;


	/**
     * The order in which SSMs should be reinitialized.
	 * Follows the order in the dependency list.
     */
    Properties*                     mpOrderedRegisteredSms;


	/**
	 * The maximum order index in the dependency list.
	 * By tracking this number we can iterate 0..max
	 * to get all the dependencies in the correct order.
	 */
	uint32							mMaxOrderedIndex;


	/**
	 * The properties governing receiving state changes.
	 */
	SubscribeProps*                mpChangeReceiverProps;


	/**
	 * The properties governing impelement the ops of ssm.
	 */
	ImplementProps*                mpImplementProps;


        /**
	 * The properties governing publish the ops of ssm.
	 */
	PublishProps*                  mpPublishProps;

	/**
	 * Worker pool used to serialize operations.
	 */
	WorkerPool*                    mpSerializePool;


	/**
     * A list of subscriptions that have not yet been acked by Com.
     */
    Properties*                    mpOutstandingSubscriptions;


	/**
     * A list of subscriptions that need to be unsubscribed from
	 * when the coordinator is going down.
     */
    Properties*                    mpNeedingToBeUnsubscribed;

	
	/**
	 * Object scope mutex.
	 */
    LnMutex				           mProtection;


	/**
	 * Flag indicating if the name server is alive.
	 */
	bool							mIsNameServerAvailable;


	/**
	 * Flag indication if the coordinator has started its primary
	 * coordinating service.
	 */
	bool							mIsCoordinatingServiceStarted;


	/**
	 * Flag indication if the coordinator has started has been triggered 
	 * to start.
	 */
	bool							mIsCoordinatingServiceTriggered;

};


extern "C"
{
	// Publish LicsNsAvail to trigger coordinators.
	//
	void ssmSetIsAvail();
};



#endif // SsmCoordinatorActor_h_
