
#include "Ssm/SsmCoordinatorActor.h"     // class implemented
#include "Util/Dbc.h"                    // USES design-by-contract
#include "Util/Debug.h"                  // USES debug output
#include "Util/Log.h"                    // USES logging
#include "Util/LnAutoPtr.h"              // USES LnAutoPtr
#include "Com/LnObjectHolder.h"          // USE object holder
#include "Com/ComService.h"              // USES Com object
#include "Com/AvListMsg.h"               // USES av list message
#include "Ssm/SsmCoordinatorService.h"   // USES singleton interface
#include "Ssm/SsmNameFactory.h"          // USES name factory
#include "Ssm/SmCondition.h"			 // USES state machine condition
#include "Ssm/ResolveConditionsJob.h"    // USES conditions job
#include "Ssm/StartActionsJob.h"         // USES start actions job
#include "Com/ReliabilityType.h"         // USES reliability type
#include "Util/RefPtr.h"                 // USES ref ccounter
#include "Osencap/LnMutex.h"             // HASA mutex
#include "Osencap/LnLockGuard.h"         // USES locker
#include "System/System.h"				 // USES
#include "Util/RefGuard.h"				 // USES
#include "Lics/LicsPublic.h"			 // USES name server availability
#include "Ssm/SyncSm.h"                  // USES SyncSm
#include "Util/Bits64Util.h"
#include "Ssm/SsmPublic.h"				// USES
#include "Ssm/ReinitializeDependentSsmsJob.h"	// USES
#include "Ssm/InjectEventJob.h"			// USES
#include "Ssm/ReinitializeSsmJob.h"		// USES
#include "Osencap/LnTick.h"				// USES
#include "Util/PropertiesFactory.h"		// USES


SsmCoordinatorActor::SsmCoordinatorActor(
	Properties*  cfg,
	const char*  groupModuleName, 
    const char*  uniqueModuleName,
    int          msgQsize,
    int          stackSize, 
    TaskPriority priority)
    :	Actor(groupModuleName, uniqueModuleName, cfg, msgQsize, stackSize, priority),
		mpDependencyProps(0),
		mpDefaultsProps(0),
		mpDependencyFactory(0),
        mSourceId(0),
        mpTimer(0),
		mpSmToCondMap(0),
		mpRegisteredSms(0),
		mpOrderedRegisteredSms(0),
		mMaxOrderedIndex(0),
		mpChangeReceiverProps(0),
        mpImplementProps(0),
        mpPublishProps(0),
		mpSerializePool(0),
		mpOutstandingSubscriptions(0),
		mpNeedingToBeUnsubscribed(0),
		mIsNameServerAvailable(false),
		mIsCoordinatingServiceStarted(false),
		mIsCoordinatingServiceTriggered(false)
		
{
	// Create the dependency factory.
	//
	mpDependencyFactory= new SsmDependencyFactory(*this);

	// Build up the module description. It needs to built up on the fly so 
	// output level values can be included.
	//
	static strstream description;
	description << "\
Module ssm represents the synchronized finate state machine service. \n\
\n\
The following output levels are controllable from ps: \n";

	description 
<< "  SSM_TRACE_REQ       = " << SSM_TRACE_REQ        << " - Trace incommming requests. " <<endl
 
<< '\0';


	// Set the description for this module. There's no need to copy the
	// stream because the call str() freezes the buffer and gives it us.
	//
	MetaData().Value("DESCRIPTION", description.str());


	// Create the map of state machines to conditions that are dependent on
	// them.
	//
	mpSmToCondMap= new Properties(15, "SmToCondMap");


	// Create the list for storing unacknowledged subscriptions.
	//
	mpOutstandingSubscriptions= new Properties(15, "UnackedSubscriptions");

	mpNeedingToBeUnsubscribed= new Properties(15, "NeedingToBeUnsubscribed");



	// Create a list of registered state machines.
	//
	mpRegisteredSms= new Properties(15, "RegisteredSms");
	mpOrderedRegisteredSms= new Properties(15, "OrderedRegisteredSms");


	// Create the policy governing how state changes are received.
	// We want it to be reliable.
	//
	mpChangeReceiverProps= ComService::Singleton()->SubscribePropsFactory();
	mpChangeReceiverProps->Notify(true);
	mpChangeReceiverProps->Reliability(RELIABLE);

    mpImplementProps = ComService::Singleton()->ImplementPropsFactory();
	mpImplementProps->Reliability(RELIABLE);

    mpPublishProps = ComService::Singleton()->PublishPropsFactory();
    mpPublishProps->Reliability(RELIABLE);

	// Create and start the worker pool used to serialize initial condition
	// requests. There's one entry in the pool so all requests are queued
	// and executed in FIFO.
	//
	mpSerializePool= new WorkerPool(
		1,
        "SsmWorkerPool", 
        "SsmWorkerPool",
		0,
        "SsmWorker", 
        "SsmWorker",
        SSM_MSGQ_SIZE);

    LnStatus rc= mpSerializePool->JumpToPrimary();
	LASSERT(rc == LN_OK, this, "SSM_ERROR", "WORKER_POOL_FAIL",
		"Couldn't start worker pool.");


	// The tick count acts as a source ID.
	//
	mSourceId= LnTick::GetTicks();



	// Turn on debug by default.
	//
	//Output::GroupOn(groupModuleName);

}// SsmCoordinatorActor



SsmCoordinatorActor::~SsmCoordinatorActor()
{
    D2(this, "~SsmCoordinatorActor");

	DeregisterForMonitoredAttributes();
	
	if (mpDependencyProps) 
		mpDependencyProps->Destroy();

	if (mpDefaultsProps) 
		mpDefaultsProps->Destroy();

	if (mpRegisteredSms)   
		mpRegisteredSms->Destroy();

	if (mpOrderedRegisteredSms)   
		mpOrderedRegisteredSms->Destroy();

	if (mpOutstandingSubscriptions)   
		mpOutstandingSubscriptions->Destroy();

	if (mpNeedingToBeUnsubscribed)   
		mpNeedingToBeUnsubscribed->Destroy();

	if (mpSerializePool)   
		mpSerializePool->JumpToExit();
	
    delete mpChangeReceiverProps;	

    delete mpImplementProps;    
    
    delete mpPublishProps;    

}// ~SsmCoordinatorActor



ostream&         
operator<< (ostream& o, const SsmCoordinatorActor& val)
{
	val.Dump(o);

	return o;

}//


ostream&  
SsmCoordinatorActor::Dump(ostream& o, DumpType /*fmt*/, int /*depth*/) const
{
	LnLockGuard door(((SsmCoordinatorActor&)(*this)).mProtection);

	o	<< "Ssm=\n"
		<< "IsCoordinatingServiceStarted  =" << mIsCoordinatingServiceStarted << endl
		<< "IsCoordinatingServiceTriggered=" << mIsCoordinatingServiceTriggered << endl
		<< "IsNameServerAvailable         =" << mIsNameServerAvailable << endl
		<< (Actor&) (*this) <<endl;

	o << endl << "Live Dependencies:" <<endl;
	RWTPtrSlistIterator<SsmDependencyBase> iter(((SsmCoordinatorActor&) *this).mDependencyList);

	for (SsmDependencyBase* dep= 0; (dep= (SsmDependencyBase*) iter()); )
	{
		o << "--------------------" <<endl;

		o << *dep;

	}// foreach dependency

	o << endl;

//	o << "Live Sm to Condition Map:" <<endl;
//	o << *this->mpSmToCondMap;
//	o << endl;


	o << endl << "Serialization Pool:" <<endl;
	o << *mpSerializePool <<endl;

    // Dump SyncSm TimeStamps
    LnAutoPtr<PropertiesIter> ssm_iter= mpRegisteredSms->AutoIterator();
    Property* prop= 0;
    int i=0;
    while ((prop= (*ssm_iter)()) != 0)
    {
        uint64 diffTime;	  
        char   ElapsedTimeString[80];


		SyncSm* ssm= (SyncSm*) prop->AsVoidp();
		REQUIRE(ssm);

        diffTime = ssm->GetStopSyncTime().Milliseconds() - 
                   ssm->GetStartSyncTime().Milliseconds();
        Bits64Util::Uint64ToString(diffTime, ElapsedTimeString);  

        o << i <<  ": "     << ssm->GetSsmName() << endl;

        o << "    Start="   << ssm->GetStartSyncTime() \
          << "    Stop="    << ssm->GetStopSyncTime()  \
          << "    Elapsed=" << ElapsedTimeString << endl;
        i++;

    }// while ssm



    return o;

}// Dump



LnStatus	
SsmCoordinatorActor::Register(SsmSmBase& rSm)
{
	SET_XCEPTION_IF(
		mpDependencyProps == 0,
		LN_FAIL, 
		"SSM", "NO_DEPENDENCIES", "state", 0, 
		"Cannot register when there are no dependency properties.");


	LnLockGuard door(mProtection);

	// Must register before service has started.
	//
	SET_XCEPTION_IF(
		mIsCoordinatingServiceTriggered == true,
		LN_FAIL, 
		"SSM", "INVALID_STATE", "state", 0, 
		"Cannot register with SsmCoordinator after service start triggered.");


	// Queue up registration so it occurs in the coordinator's thread.
	//
	LnObjectHolder* msg= new LnObjectHolder(SsmPublic::RegisterOp(), (void*) &rSm);

	return this->HandleMsg(msg);

}// Register


LnStatus
SsmCoordinatorActor::AddSsm(SsmSmBase& rSm)
{
	SET_XCEPTION_IF(
		mpRegisteredSms->IsExist(rSm.SsmName()) == true,
		LN_FAIL, 
		SsmPublic::SsmErrorAv(), "BAD_CFG", "state", 0, 
		"Ssm already exists in registered list. ssm=" << rSm.SsmName());


	Properties* dependency_props= FindDependencyForSsm(rSm.SsmName());
	if (dependency_props == 0) return LN_FAIL;


	SET_XCEPTION_IF(
		dependency_props->IsExist(SsmPublic::OrderAn()) == false,
		LN_FAIL, 
		SsmPublic::SsmErrorAv(), "BAD_CFG", "state", 0, 
		"Attribute order does not exist for ssm=" << rSm.SsmName());


	uint32 dependency_order= dependency_props->AsUint32(SsmPublic::OrderAn());


	SET_XCEPTION_IF(
		mpOrderedRegisteredSms->IsExist(dependency_order) == true,
		LN_FAIL, 
		SsmPublic::SsmErrorAv(), "BAD_CFG", "state", 0, 
		"Dependency already exists at order=" << dependency_order <<
		" newssm=" << rSm.SsmName() << " existingssm=" 
		<< mpOrderedRegisteredSms->AsString(dependency_order));


	// Create the dependency object.
	//
	SsmDependencyBase* ssm_dep= mpDependencyFactory->Create(*dependency_props);
	if (ssm_dep == 0) return LN_FAIL;


	// Insert it into a list of all dependencies.
	//
	mDependencyList.insert(ssm_dep);


	// Insert the state machine into the list of state machines
	// this coordinator manages.
	//
	mpRegisteredSms->Value(rSm.SsmName(), (void*) &rSm);


	// Maintain the ordered list of SMs.
	//
	mpOrderedRegisteredSms->Value(dependency_order, (char*) rSm.SsmName(), true, true);
	if (dependency_order > mMaxOrderedIndex) mMaxOrderedIndex= dependency_order;

	return LN_OK;

}// AddSsm


void	
SsmCoordinatorActor::HandleRegisterOp(SsmSmBase& rSm)
{
	D1(this, "HandleRegisterOp: name=" << rSm.SsmName());

	REQUIRE(mIsCoordinatingServiceStarted == false);
	REQUIRE(mIsCoordinatingServiceTriggered == false);

	LnStatus rc= AddSsm(rSm);
	LXASSERT(rc == LN_OK, LnException::Get());


	// Register as a listener for state machine state changes.
	// These will be published over Com for other coordinators to
	// pick up.
	//
	rSm.AddObserver(this);


	// Register the state machine in the name space.
	//
	RegisterSmNameSpace(rSm.SsmName());


}// HandleRegisterOp


LnStatus	
SsmCoordinatorActor::Dependencies(const char* pProps)
{
	istrstream input((char*) pProps);
	Properties* props= new Properties(input);

	return Dependencies(*props);

}// Dependencies



LnStatus	
SsmCoordinatorActor::Dependencies(const char* pProps[])
{
	// Build up one string out of the array of strings. 
	// Automatically insert the order number which indicates
	// the order in which operations are queued. We can build
	// it automatically when the array form is used because
	// we just assume the order in the array is the required
	// order.
	//
	RWCString concat; // string with concatenation of all lines
	uint32 order_idx= 0; // current order number

	// Extra padding needed to get past the type definition to insert 
	// the order attribute.
	//
	uint32 len_after_type= 0;

	// Loop through every string in the array and form one large
	// string that can be used to create a properties.
	//
	for (int i= 0; pProps[i] != 0; i++)
	{
		RWCString line= pProps[i];

		// If it's a dependency definition then order can be
		// inserted.
		//
		size_t found_idx= line.index("T='DependencyDefinition'>");
		if (found_idx == RW_NPOS)
		{
			// Try the alternate spelling of T.
			//
			found_idx= line.index("TYPE='DependencyDefinition'>");
			len_after_type= 5;
		}
		else
		{
			len_after_type= 2;
		}

		if (found_idx != RW_NPOS)
		{
			// Build up the order index and append it after the
			// T string.
			//
			RWCString order= "<U32 N='order'>";
			order.append(order_idx++);
			order.append("</U32>\n");

			line.insert(found_idx + order.length() + len_after_type, order);
		}

		concat.append(line);

	}// foreach line 

	// Use the string form to create a properties from the descriptor 
	// string.
	//
	return Dependencies(concat.data());

}// Dependencies


LnStatus	
SsmCoordinatorActor::Dependencies(Properties& rProps)
{
	REQUIRE(mpDependencyProps == 0);

	LnLockGuard door(mProtection);

	mpDependencyProps= &rProps;

	// Setup defaults.
	//
	Properties* tmp= mpDependencyProps->AsPropertiesp("Defaults");
	if (tmp)
	{
		mpDefaultsProps= PropertiesFactory::Copy(*tmp);
	}
	else
	{
		mpDefaultsProps= new Properties(1, "Defaults");
	}

	D1(this, "Defaults   Props=" << *mpDefaultsProps);
	D1(this, "Dependency Props=" << *mpDependencyProps);

	return LN_OK;

}// Dependencies



void 
SsmCoordinatorActor::RegisterSmNameSpace(const char* pSmName)
{
	// The coordinator registers to implement all operations for
	// the state machine.
	//
	RWCString obj_name;
	SsmNameFactory::CalcSmObjName(obj_name, pSmName);

	D1(this, "RegisterSmNameSpace:sm=" << pSmName << " obj=" << obj_name);

	LnStatus rc= ComService::Singleton()->Implement(this, obj_name, SsmPublic::GetStateOp(), mpImplementProps);
	LXASSERT(rc == LN_OK, LnException::Get());

    

	rc= ComService::Singleton()->Implement(this, obj_name, SsmPublic::InjectEventOp(), mpImplementProps);
	LXASSERT(rc == LN_OK, LnException::Get());


	// Preannounce the publication. This helps in making sure the
	// delay on the initial publish doesn't screw up the timing.
	//
	rc= ComService::Singleton()->PublishAnnounce(
			0, obj_name, SsmPublic::UpdateDependencyOp());

	LXASSERT(rc == LN_OK, LnException::Get());


}// RegisterSmNameSpace


void           
SsmCoordinatorActor::ImplementMsg(LnObject* msg)
{
    REQUIRE(msg != 0);

    // Look at the operation to determine the handler.
    // Each handler is reponsible for deleting the message.
    //
    if (msg->IsClass(AvListMsg::ThisClassName()))
    {
        // Cast the message to the right type.
        //
        AvListMsg* req= (AvListMsg*) msg;

		D1(this, "ImplementMsg: av_req=" << *req);

		// Dispatch based on the operation.
		//
        if (req->IsOp(SsmPublic::GetStateOp()))
		{
			// Get the state of a SSM is handled immediately so SSMs can
			// resolve.
			//
			ImplementGetStateOp(*req);
		}
		else if (req->IsOp(SsmPublic::InjectEventOp()))
        {
			// Inject events are serialized behind reinitialize and resolve
			// initial conditions so that the state machine are in known
			// states before the new state is applied.
			//
			req->AddRef();
			InjectEventJob* job= new InjectEventJob(*this, *req);
			this->QueueSerializedWork(*job);
        }
		else if (req->IsOp(SsmPublic::UpdateDependencyOp()))
		{
			// Dependencies that are updated because of reinitialization
			// must be evaluated immediately so the SSM is in the correct
			// state. Dependencies that are not because of initialization
			// must be queued behind reinitialize and resolve initial
			// conditions so the state machines are in known states befire
			// the update is applied.
			//
			bool is_initializing= req->AsBool(SsmPublic::IsInitializingAn());

			if (is_initializing)
			{
				ImplementUpdateDependencyOp(*req);
			}
			else
			{
				req->AddRef();
				ReinitializeSsmJob* job= new ReinitializeSsmJob(*this, *req);
				this->QueueSerializedWork(*job);
			}
        }
        else
		{
            // There was no handler for the message. Log it.
            //
            LWARN("BAD_MSG", "NO_HANDLER", "Msg", this,
                "ImplementMsg: don't understand the message CLASS=" << msg->ClassName());
        }
    }
	else if (msg->IsClass("ComNotification") == true)
	{
		ComNotification* notification= (ComNotification*) msg;

		if (notification->IsName("SUBSCRIPTION_VALID") == true)
		{
			HandleSubscriptionNotification(*notification);
		}
		else
		{
		    // There was no handler for the notification. Log it.
            //
            LWARN("BAD_MSG", "NO_HANDLER", "Msg", this,
                "ImplementMsg: unexpected ComNotification=" << *notification);
		}
	}
    else if (msg->IsClass("LnObjectHolder") == true)
    {
		LnObjectHolder* holder= (LnObjectHolder*) msg;
		if (holder->IsDataClass(SsmPublic::RegisterOp()) == true)
		{
			SsmSmBase* pSm= (SsmSmBase*) holder->Data();
			HandleRegisterOp(*pSm);
		}
		else if (holder->IsDataClass("StartCoordinationService") == true)
		{
			StartCoordinationService();
		}
		else
		{
			// There was no handler for the command. Log it.
            //
            LWARN("BAD_MSG", "NO_HANDLER", "Msg", this,
                "ImplementMsg: unexpected LnObjectHolder=" << holder->DataClassName());
		}
	}
    else
    {
        // There was no handler for the message. Log it.
        //
        LWARN("BAD_MSG", "NO_HANDLER", "Msg", this,
            "ImplementMsg: don't understand the message  class=" << msg->ClassName());
    }

	// Make sure every message is deleted.
	//
    msg->Destroy();

}// ImplementMsg


void 
SsmCoordinatorActor::AddSmToConditionAssoc(const char* pSsmName, SsmConditionBase& rCond)
{
	REQUIRE(pSsmName != 0);

	// Get the properties object for the specific state machine.
	//
	Properties* sm_props= mpSmToCondMap->AsPropertiesp(pSsmName);
	if (sm_props == 0)
	{
		// If there isn't a properties object for the state machine then
		// create one.
		//
		sm_props= new Properties(1, pSsmName);
		mpSmToCondMap->Value(sm_props);
	}


	// Add the condition in as beeing associated with the state machine.
	// We store the condition in as a void*, which is bad form.
	//
	sm_props->Value(sm_props->Entries()+1, (void*) &rCond);


	// Subscribe to state machine change events for this state machine.
	//
	RWCString obj_name;
	SsmNameFactory::CalcSmObjName(obj_name, pSsmName);

	D1(this, "AddSmToConditionAssoc: subscribing for state change events obj=" << obj_name);


	LnStatus rc= ComService::Singleton()->Subscribe(
		this, 
		obj_name, 
		SsmPublic::UpdateDependencyOp(), 
		mpChangeReceiverProps);

	if (rc == LN_OK)
	{
		// Put ourselves in the list of subscriptions so we can be mapped
		// to a subscription notification.
		//
		RWCString subscribed_key= obj_name;
		subscribed_key+= SsmPublic::UpdateDependencyOp();

		mpOutstandingSubscriptions->Value(subscribed_key);
		mpNeedingToBeUnsubscribed->Value(
			mpNeedingToBeUnsubscribed->Entries(),
			(char*) obj_name.data(),
			true, true);
	}
	else
	{
		// The map has multiple of the same state machines so we can get
		// duplicate subscriptions.
		//
		LXASSERT(LnException::Get()->IsReason("DUPLICATE_SUBSCRIBE"), LnException::Get());

		LnException::Clear();
	}
	


}// AddSmToConditionAssoc



LnStatus        
SsmCoordinatorActor::ImplementStateInitializing(const char* /*substate*/)
{
    D1(this, "SsmCoordinatorActor:ImplementStateInitializing");

	// Install ourselves as the singleton, unless we are told not to.
	//
	if (MetaData().IsExist("IS_NOT_SINGLETON") == false)
		SsmCoordinatorService::Singleton(this);

#if 1
    // new ssm timer here
    System* pSystem = System::Singleton();
    CHECK(pSystem);


    if ( strcmp(pSystem->GetProperty("txnHost.type"),"TM") == 0)
    {   
        LMSG("TM SSM init timer\n");
        mpTimer = new SsmCoordinatorTimer(this);
    }
    else
        mpTimer = NULL;
#endif

	RegisterForMonitoredAttributes();


	// Mark that we've entered state initializing.
	//
	ModuleState(MODULE_STATE_INITIALIZING);

    return LN_OK;

}// ImplementStateInitializing



 
LnStatus 
SsmCoordinatorActor::ImplementStateDown(const char* /*substate*/)
{
    D1(this, "SsmCoordinatorActor:ImplementStateDown");

	LnAutoPtr<PropertiesIter> iter= mpRegisteredSms->AutoIterator();
    Property* prop= 0;
    while ((prop= (*iter)()) != 0)
    {
		SsmSmBase* sm= (SsmSmBase*) prop->AsVoidp();

		D1(this, "SsmCoordinatorActor:unimplement: for ssm=" << sm->SsmName());

		RWCString obj_name;
		SsmNameFactory::CalcSmObjName(obj_name, sm->SsmName());


		ComService::Singleton()->UnImplement(this, obj_name, SsmPublic::GetStateOp());
		LnException::Clear();

		ComService::Singleton()->UnImplement(this, obj_name, SsmPublic::InjectEventOp());
		LnException::Clear();

	}// while ssms


	for (uint32 i= 0; i < (uint32) mpNeedingToBeUnsubscribed->Entries(); i++)
	{
		RWCString obj_name= mpNeedingToBeUnsubscribed->AsString(i);

		D1(this, "SsmCoordinatorActor:unsubscribe: for obj=" << obj_name);

		ComService::Singleton()->UnSubscribe(
			this, 
			obj_name, 
			SsmPublic::UpdateDependencyOp());

		LnException::Clear();
	}


	// Mark that we've entered state down. We don't really
	// do anything in this state.
	//
	ModuleState(MODULE_STATE_DOWN);

    return LN_OK;

}// ImplementStateDown



LnStatus
SsmCoordinatorActor::ImplementStateSecondary(const char* /*substate*/)
{
	D1(this, "SsmCoordinatorActor:ImplementStateSecondary");

	// Nothing special to do in secondary state.
	//
	ModuleState(MODULE_STATE_SECONDARY);

	return LN_OK;

}// ImplementStateSecondary



LnStatus
SsmCoordinatorActor::ImplementStatePrimary(const char* /*substate*/)
{
	LMSG("SsmCoordinatorActor:ImplementStatePrimary");
	// We are now primary. 
	//
	ModuleState(MODULE_STATE_PRIMARY);

	// Moving to primary can trigger starting the coordination service
	// if some other conditions have already been satisfied.
	//
	LnLockGuard door(mProtection);

	if (IsNeedStartCoordinating())
	{
		LMSG("SsmCoordinatorActor: triggering start on going to primary.");
		TriggerStartCoordinationService();
	}
	else
	{
		D1(this, "SsmCoordinatorActor:ImplementStatePrimary: not starting service yet.");
	}

	return LN_OK;

}// ImplementStatePrimary


void
SsmCoordinatorActor::StartCoordinationService(void)
{
	LMSG("SsmCoordinatorActor: StartCoordinationService");
    
	// Prevent the service from being started more than once.
	//
	LnLockGuard guard(mProtection);
	
	// Check if the service was started while we were blocked.
	//
	if (mIsCoordinatingServiceStarted == true) return;

	// Consider the service started.
	//
	mIsCoordinatingServiceStarted= true;

	// By this time all our state machines have registered in the
	// name space. Now take the steps necessary to start this coordinator
	// participating in driving its state machines and distributed state
	// machines.
	//
	mpSerializePool->SuspendHiring();

	ReinitializeDependentSms();
	ResolveInitialConditions();
	ActivateStartActions();

	mpSerializePool->ResumeHiring();


}// StartCoordinationService


void
SsmCoordinatorActor::ReinitializeDependentSms()
{
	D1(this, "SsmCoordinatorActor:ReinitializeDependentSms");


    for (uint32 i= 0; i <= mMaxOrderedIndex; i++)
    {
		if (mpOrderedRegisteredSms->IsExist(i) == false)
		{
			continue;
		}

		RWCString ssm_name= mpOrderedRegisteredSms->AsString(i);
		REQUIRE(ssm_name.length() > 0);

		SsmSmBase* sm= GetRegisteredSsm(ssm_name);
		REQUIRE(sm);

		D1(this, "SsmCoordinatorActor:ReinitializeDependentSms: queuing ssm="
			<< sm->SsmName() << " order=" << i);


		// Create the job that will resolve the conditions.
		//
		ReinitializeDependentSsmsJob* job= new ReinitializeDependentSsmsJob(*this, *sm);
		
		QueueSerializedWork(*job);

    }// while sms

}// ReinitializeDependentSms


void
SsmCoordinatorActor::ResolveInitialConditions(void)
{
	D1(this, "SsmCoordinatorActor:ResolveInitialConditions");


	// Queue up jobs for determining initial conditions for all dependencies.
	// Jobs are executed in order and the block forever until the condition
	// is resolved.
	//
	RWTPtrSlistIterator<SsmDependencyBase> diter(mDependencyList);

	SsmDependencyBase* dep= 0;
	for (dep= 0; (dep= (SsmDependencyBase*) diter()); )
	{
		D1(this, "SsmCoordinatorActor:ResolveInitialConditions: queuing dep="
			<< dep->Name());

		// Create the job that will resolve the conditions.
		//
		ResolveConditionsJob* job= new ResolveConditionsJob(*this, *dep);
		
		QueueSerializedWork(*job);

	}// foreach state machine

}// ResolveInitialConditions



bool
SsmCoordinatorActor::IsNeedStartCoordinating(void) const
{
	// The coordinator can only start once several different
	// conditions are satisfied.
	//
	if (mIsCoordinatingServiceStarted == false
		&& mIsCoordinatingServiceTriggered == false
		&& ModuleState() == MODULE_STATE_PRIMARY 
		&& mpOutstandingSubscriptions->Entries() <= 0
		&& mIsNameServerAvailable == true)
	{
		return true;
	}
	else
	{
		return false;
	}


}// IsNeedStartCoordinating


void
SsmCoordinatorActor::ActivateStartActions(void)
{
	D1(this, "SsmCoordinatorActor:ActivateStartActions");

	// Queue up jobs for executing start actions. It is very important
	// to have all startup actions occur after all initial conditions
	// are resolved. This prevents a state machine from transition
	// twice, once because the conditions are resolved, and once when
	// an action is triggered.
	//
	RWTPtrSlistIterator<SsmDependencyBase> iter(mDependencyList);

	SsmDependencyBase* dep;
	for (dep= 0; (dep= (SsmDependencyBase*) iter()); )
	{
		// Don't queue up a job unless there are actions.
		//
		if (dep->StartActions().isEmpty()) continue;

		// Create the job that will resolve the conditions.
		//
		D1(this, "SsmCoordinatorActor:ActivateStartActions: startaction=" << *dep);
		StartActionsJob* job= new StartActionsJob(*this, *dep);
	
		QueueSerializedWork(*job);

	}// foreach state machine


}// ActivateStartActions


void 
SsmCoordinatorActor::SubjectChanged(
	const Typeable* subject,
    const char*     changeType)
{

	if (strcmp(changeType, "SM_MOVED") == 0)
	{
		// Handle changes to state machines.
		//
		if (subject->IsClass("SsmSmBase"))
		{
			SsmSmBase* sm= (SsmSmBase*) subject;
			PublishStateChange(*sm);
		}
	}
	else if (subject->IsClass("Properties"))
	{
		Properties* changes= (Properties*) subject;
		RefGuard guard_changes(changes);

	
		if (changes->IsExist(LicsPublic::LicsNameServerAvailableAn()))
		{
			LnLockGuard door(mProtection);

			mIsNameServerAvailable= changes->AsBool(LicsPublic::LicsNameServerAvailableAn());

			LMSG("SsmCoordinatorActor: got notified name server availability is=" 
				<< mIsNameServerAvailable);


			// If the name server becomes available we may be in a position
			// to start coordination service.
			//
			if (IsNeedStartCoordinating())
			{
				LMSG("SsmCoordinatorActor: triggering start on name server.");
				TriggerStartCoordinationService();
			}
			else
			{
				D1(this, "SsmCoordinatorActor:SubjectChanged: not starting service yet.");
			}
		}

	}
	else
	{
		// don't care about other type of changes
	}

}// SubjectChanged


void 
SsmCoordinatorActor::PublishStateChange(SsmSmBase& rSm, bool isInitialization)
{
    LnStatus rc;
	// Nobody cares about SYNCING state. This is just a hack to make sure
	// its abscence is OK. Later this should be specied in the dependency
	// description file.
	//
	if (strcmp(rSm.SsmCurrentState(), "SYNCING") == 0)
	{
		return;
	}

	D1(this, "PublishStateChange: sm=" << rSm.SsmName() << " state=" 
		<< rSm.SsmCurrentState() << " SourceId=" << mSourceId
		<< " isinit=" << isInitialization);

	// Create the name of the state machine in the name space.
	//
	RWCString obj_name;
	SsmNameFactory::CalcSmObjName(obj_name, rSm.SsmName());

#if 1
    if(mpTimer)
    {
        mpTimer->CancelTimer();
        if (!IsLastSsm(rSm))
        {
            rc = mpTimer->StartTimer();
            LXASSERT(rc==LN_OK,LnException::Get());
        }
    }
#endif

	// Create the state machine change notification event.
	//
	AvListMsg* event= new AvListMsg(1, 0, SsmPublic::UpdateDependencyOp());
	event->Value(SsmPublic::StateNameAn(), (char*) rSm.SsmCurrentState(), true, true);
	event->Value(SsmPublic::SmNameAn(), (char*) rSm.SsmName(), true, true);
	event->Value(SsmPublic::SourceIdAn(), mSourceId);
	event->Value(SsmPublic::IsInitializingAn(), isInitialization);

	
	// Publish the state change event. We don't care if it succeeds or not.
	//
	rc= ComService::Singleton()->Publish(
		0, 
		obj_name, 
		SsmPublic::UpdateDependencyOp(), 
		event, 
		mpPublishProps);


    LXASSERT(rc == LN_OK, LnException::Get());



}// PublishStateChange



void
SsmCoordinatorActor::HandleSubscriptionNotification(ComNotification& rNotification)
{
	D1(this, "HandleSubscriptionNotification:" << rNotification);


	if (mpOutstandingSubscriptions->Entries() <= 0)
	{
		D1(this, "HandleSubscriptionNotification:" 
			<< "the subscription list is empty and we got a notification");
		return;
	}


	// Get the name of the first item in the list so we can delete it.
	// Since we don't know which subscription the notification is for
	// we just have to delete something off the list and the only
	// way to get a key value is iterate through the list once to get
	// the first key.
	//
	LnAutoPtr<PropertiesIter> iter= mpOutstandingSubscriptions->AutoIterator(); 

	RWCString first_key;
	Property* prop= 0;
	while ((prop= (*iter)()) != 0)
	{
		first_key= prop->Name();
		break;
	}
	REQUIRE(first_key.length() > 0);

	D1(this, "HandleSubscriptionNotification: removing subscription. Key=" << first_key);

	mpOutstandingSubscriptions->Remove(first_key);

	// A subscription notifications may trigger starting the coordination service
	// if some other conditions have already been satisfied.
	//
	LnLockGuard door(mProtection);

	if (IsNeedStartCoordinating())
	{
		LMSG("SsmCoordinatorActor: triggering start when outstanding "
			<< " subscriptions went to 0");

		TriggerStartCoordinationService();	
	}
	else
	{
		D1(this, "SsmCoordinatorActor:HandleSubscriptionNotification: not starting service yet.");
	}


}// HandleSubscriptionNotification



void 
SsmCoordinatorActor::RegisterForMonitoredAttributes(void)
{

        // Detect initial setting in case the name server is already up when 
        // we come up. If it is available we don't have to trigger coordination
        // to start because we can't be ready to start in this system state.
        //
        Properties* system_props= System::Singleton()->GetMutableProps();

        if(system_props->IsExist(LicsPublic::LicsNameServerAvailableAn()))
        {
                mIsNameServerAvailable= system_props->AsBool(LicsPublic::LicsNameServerAvailableAn());

                if (mIsNameServerAvailable)
                {
                        LMSG("SsmCoordinatorActor: name server is already available.");
                }
        }



	// Register for name space availability.
	//
	Properties* monitored_attributes = new Properties(1);
	monitored_attributes->Value(LicsPublic::LicsNameServerAvailableAn());

	System::Singleton()->AddObserver(this, monitored_attributes);


}// RegisterForMonitoredAttributes



void 
SsmCoordinatorActor::DeregisterForMonitoredAttributes(void)
{
	// No longer need notifications.
	//
	System::Singleton()->DeleteObserver(this);

}// DeregisterForMonitoredAttributes



void
SsmCoordinatorActor::TriggerStartCoordinationService()
{
	LnLockGuard door(mProtection);

	// Check if the service was triggered while we were blocked.
	//
	if (mIsCoordinatingServiceTriggered) return;

	// Service is now considered triggered.
	//
	mIsCoordinatingServiceTriggered= true;

	
	// Queue up registration so it occurs in the coordinator's thread
	// and after any current registration requests.
	//
	LnObjectHolder* msg= new LnObjectHolder("StartCoordinationService");
	LnStatus rc= this->HandleMsg(msg);
	LXASSERT(rc == LN_OK, LnException::Get());

}// TriggerStartCoordinationService





void ssmSetIsAvail()
{
	static Property* spAvailLicsNs= 0;
	if (spAvailLicsNs == 0)
	{
		// First time through this method so create the beacon property
		// and publish it. There's only one thread at a time through here
		// so we don't need any protection.
		//
		spAvailLicsNs= new Property(LicsPublic::LicsNameServerAvailableAn(), true);

		System* pSystem= System::Singleton();
		CHECK(pSystem);

		pSystem->PublishProp(spAvailLicsNs);
		LnException::Clear(); // not worth dying over
	}
	else
	{
		Properties* pSysProps= System::Singleton()->GetMutableProps();

		// Tell the world about our new availability status.
		//
		pSysProps->Value(LicsPublic::LicsNameServerAvailableAn(), true);
	}

}// ssmSetIsAvail




void SsmCoordinatorActor::DumpSsmTimes ()
{
    LnAutoPtr<PropertiesIter> ssm_iter= mpRegisteredSms->AutoIterator();
    Property* prop= 0;
    int i=0;
    while ((prop= (*ssm_iter)()) != 0)
    {
        uint64 diffTime;	  
        char   ElapsedTimeString[80];


		SyncSm* ssm= (SyncSm*) prop->AsVoidp();
		REQUIRE(ssm);

        diffTime = ssm->GetStopSyncTime().Milliseconds() - 
                   ssm->GetStartSyncTime().Milliseconds();
        Bits64Util::Uint64ToString(diffTime, ElapsedTimeString);  

        LMSG(i <<  ": "        << ssm->GetSsmName()         \
               << "("          << ssm->GetStartSyncCount()  \
               << ","          << ssm->GetStopSyncCount()   \
               << "):"                                      \
            );


        LMSG ( "    Start="   << ssm->GetStartSyncTime() \
            << "    Stop="    << ssm->GetStopSyncTime()  \
            << "    Elapsed=" << ElapsedTimeString);
        i++;

    }// while ssm
}


SsmSmBase*
SsmCoordinatorActor::GetRegisteredSsm(const char* pSsmName)
{
	REQUIRE(pSsmName);

	SsmSmBase* sm= (SsmSmBase*) mpRegisteredSms->AsVoidp(pSsmName);

	return sm;

}// GetRegisteredSsm


void 
SsmCoordinatorActor::ImplementGetStateOp(AvListMsg& req)
{
	// Get the name of the state machine the operation is for.
	//
	const char* sm_name= req.AsString(SsmPublic::SmNameAn());

	SsmSmBase* sm= GetRegisteredSsm(sm_name);
	if (sm)
	{
		D1(this, "ImplementGetStateOp: there is a sm=" << sm_name);

		AvListMsg* reply= new AvListMsg(1, &req);
		reply->Value(SsmPublic::SmNameAn(), (char*) sm_name, true, true);
		reply->Value(SsmPublic::StateNameAn(), (char*)  sm->SsmCurrentState());
		reply->Value(SsmPublic::SourceIdAn(), mSourceId);

		LnStatus rc= ComService::Singleton()->Reply(reply);
		if (rc != LN_OK)
		{
			LnException::Clear();
		}

	}
	else
	{
		D1(this, "ImplementGetStateOp: no state machine for sm=" << sm_name);
	}

}// ImplementGetStateOp


LnStatus 
SsmCoordinatorActor::InjectEvent(
	const char*	pSsmName, 
	uint32		sourceId,
	const char* pEvent)
{
	REQUIRE(pEvent);

	SsmSmBase* sm= GetRegisteredSsm(pSsmName);

	SET_XCEPTION_IF(
		sm == 0, LN_FAIL, 
		SsmPublic::SsmErrorAv(), SsmPublic::InvalidReqErrorAv(), "Ssm", 0, 
		"Ssm:InjectEvent: ssm not found: name=" << pSsmName <<
		" SourceId=" << sourceId);


	SET_XCEPTION_IF(
		sourceId != mSourceId, LN_FAIL, 
		SsmPublic::SsmErrorAv(), SsmPublic::InvalidReqErrorAv(), "SourceId", 0, 
		"Ssm:InjectEvent: mismatch SourceIds: name=" << pSsmName <<
		" cursid=" << mSourceId << " newsid=" << sourceId);

	D1(this, "Ssm:InjectEvent name=" << pSsmName <<
		" cursid=" << mSourceId << " newsid=" << sourceId <<
		" event=" << pEvent);

	
	// Tell the state machine about its new event.
	//
	LnStatus rc= sm->SsmEvent(pEvent);

	return rc;

}// InjectEvent


void 
SsmCoordinatorActor::ImplementInjectEventOp(AvListMsg& req)
{
	// Get the name of the state machine the operation is for.
	//
	const char* sm_name	= req.AsString(SsmPublic::SmNameAn());
	uint32 source_id	= req.AsUint32(SsmPublic::SourceIdAn());
	const char* event	= req.AsString(SsmPublic::EventNameAn());

	LnStatus rc= InjectEvent(sm_name, source_id, event);


	AvListMsg* reply= new AvListMsg(1, &req);
	reply->Value(SsmPublic::SmNameAn(), (char*) sm_name, true, true);
	reply->Value(SsmPublic::SourceIdAn(), mSourceId);

	if (rc == LN_FAIL)
	{
		reply->MarkRequestFailed(LnException::Take());
	}

	rc= ComService::Singleton()->Reply(reply);
	if (rc != LN_OK)
	{
		LnException::Clear();
		reply->Destroy();
	}

}// ImplementInjectEventOp


SsmDependencyBase*	
SsmCoordinatorActor::GetDependency(const char* pDependencyName)
{
	RWTPtrSlistIterator<SsmDependencyBase> diter(mDependencyList);

	SsmDependencyBase* dep= 0;
	for (dep= 0; (dep= (SsmDependencyBase*) diter()); )
	{
		RWCString name= dep->Name();

		if (name == pDependencyName)
		{
			return dep;
		}
	
	}// foreach state machine

	return 0;

}// GetDependency


SsmConditionBase*	
SsmCoordinatorActor::GetCondition(
	const char* pDependencyName,
	const char* pConditionName)
{
	SsmDependencyBase* dep= GetDependency(pDependencyName);
	if (dep == 0) return 0;

	return dep->GetCondition(pConditionName);

}// GetCondition





void 
SsmCoordinatorActor::ImplementUpdateDependencyOp(AvListMsg& req)
{
	// Get the name of state machine the event is for.
	//
	const char* sm_name= req.AsString(SsmPublic::SmNameAn());
	REQUIRE(sm_name);

	// Get the conditions the state machine associated with.
	//
	Properties* conditions=  mpSmToCondMap->AsPropertiesp(sm_name);

	LnStatus rc= LN_OK;
	if (rc == LN_OK && conditions)
	{
		// Loop through each condition associated with the state 
		// machine and tell it about the event so it can do whatever
		// it needs to do.
		//
		LnAutoPtr<PropertiesIter> iter= conditions->AutoIterator();
		Property* prop= 0;
		while ((prop= (*iter)()) != 0)
		{

			// Tell the condition about the event.
			//
			SsmConditionBase* condition= (SsmConditionBase*) prop->AsVoidp();

			D1(this, "Ssm:UpdateDependency: update dependency= " <<
				condition->Dependency().Name() << " for ssm=" << sm_name
				<< " cond=" << condition->GetConditionName());

			rc= condition->InjectEvent(&req);
			if (rc == LN_FAIL)
			{
				break;
			}
					
			// Because a condition has possibly been updated, tell the dependency
			// to check its conditions for possible action firings.
			//
			condition->Dependency().Evaluate();

		}// foreach property
	}

	LnException::Clear();

#if 0
	AvListMsg* reply= new AvListMsg(1, &req);
	reply->Value(SsmPublic::SmNameAn(), (char*) sm_name, true, true);
	reply->Value(SsmPublic::SourceIdAn(), mSourceId);

	if (rc == LN_FAIL)
	{
		reply->MarkRequestFailed(LnException::Take());
	}

	rc= ComService::Singleton()->Reply(reply);
	if (rc != LN_OK)
	{
		LnException::Clear();
		reply->Destroy();
	}
#endif


}// ImplementUpdateDependencyOp



void
SsmCoordinatorActor::QueueSerializedWork(Job& rWorkToDo)
{
	// Hire a worker and pass it a job to do. The job executed in
	// the worker's thread.
	//
	LnStatus rc= mpSerializePool->HireWorker(&rWorkToDo);
	LXASSERT(rc == LN_OK, LnException::Get());

}// QueueSerializedWork



extern "C"
void dumpSsmTimes ()
{
    SsmCoordinatorActor* ssm_ptr = SsmCoordinatorService::Singleton();
    ssm_ptr->DumpSsmTimes();
}


void SsmCoordinatorActor::DumpSsmElapsedTimes ()
{
    LnAutoPtr<PropertiesIter> ssm_iter= mpRegisteredSms->AutoIterator();
    Property* prop= 0;
    int i=0;
    while ((prop= (*ssm_iter)()) != 0)
    {
        uint64 diffTime;	  
        char   ElapsedTimeString[80];


		SyncSm* ssm= (SyncSm*) prop->AsVoidp();
        REQUIRE(ssm);
   
        diffTime = ssm->GetStopSyncTime().Milliseconds() - 
                   ssm->GetStartSyncTime().Milliseconds();
        Bits64Util::Uint64ToString(diffTime, ElapsedTimeString);  

        LMSG(i <<  ": "        << ssm->GetSsmName()         \
               << "("          << ssm->GetStartSyncCount()  \
               << ","          << ssm->GetStopSyncCount()   \
               << "):"   );
        LMSG(     "    Ssm Elapsed Sync Time =" << ElapsedTimeString);
        i++;
    }
}



Properties*
SsmCoordinatorActor::FindDependencyForSsm(const char* pSsmName)
{
	REQUIRE(mpDependencyProps);

	Properties* ssm_dependency_props= 0;

	// Loop through all dependencies. If the dependencies are associated
	// with the passed in state machine the bring the dependencies into
	// this coordinator.
	//
	LnAutoPtr<PropertiesIter> iter= mpDependencyProps->AutoIterator();
	Property* prop= 0;

	while (ssm_dependency_props == 0 && ((prop= (*iter)()) != 0)) 
	{   
		//prop->Dump(cout); 

		// We only care about properties objects as they contain the
		// dependency info.
		//
		if (prop->IsKind(Property::PROPERTY_PROPERTIESP) == false)
			continue;

		Properties* props= prop->AsPropertiesp();

		// We only care about dependency definitions.
		//
		if (strcmp(props->Type(), "DependencyDefinition") != 0)
			continue;


		// Get the runlocation so we can check if the dependency
		// should be run with the registering state machine. If
		// a location was not provided then use the dependency name
		// as the default.
		//
		RWCString sm= props->Name();

		Properties* location_props= props->AsPropertiesp("runlocation");
		if (location_props)
		{
			sm= location_props->AsString(SsmPublic::SmNameAn());
		}
		REQUIRE(sm.length() > 0);

		if (sm == pSsmName)
		{
			ssm_dependency_props= props; // found it
		}

	}// while properties


	SET_XCEPTION_IF(
		ssm_dependency_props == 0,
		0, 
		SsmPublic::SsmErrorAv(), "NO_DEPENDENCIES", "state", 0, 
		"No dependency for ssm=" << pSsmName);

	return ssm_dependency_props;

}// FindDependencyForSsm




extern "C"
void dumpSsmElapsedTimes ()
{
    SsmCoordinatorActor* ssm_ptr = SsmCoordinatorService::Singleton();
    ssm_ptr->DumpSsmElapsedTimes();
}



void SsmCoordinatorActor::ForceStateChange(const char* pSsmName, const char *state)
{
    if (pSsmName==NULL)
        return;
    if (strcmp(state,"GoOutOfSync")!=0 &&
        strcmp(state,"StartSyncing")!=0)
        return;

    SsmSmBase* sm= GetRegisteredSsm(pSsmName);
    LMSG("SsmForceStateChange Got the sm ="<<sm<<endl);
    if (sm==NULL)
        return;
    
    LnStatus rc = sm->SsmEvent(state);

    if (rc != LN_OK)
    {
        LnException::Clear();
    }

    LMSG("SsmForceStateChnage rc = "<<rc<<endl);
    return;

}

extern "C"
void SsmForceStateChange(const char *pSsmName, const char *state)
{

    LMSG("SsmForceStateChange Start\n");
    SsmCoordinatorActor* ssm_ptr = SsmCoordinatorService::Singleton();
    ssm_ptr->ForceStateChange(pSsmName,state);
}

// Hard Coded Temp function
// Will update later 
bool SsmCoordinatorActor::IsLastSsm(SsmSmBase &rSm)
{
    if (  ( strcmp(rSm.SsmName(), "TxnCorbaNotifySsm") == 0) ||
        ( strcmp(rSm.SsmName(), "PcChainPullSsm") == 0) )
        return true;
    else
        return false;
}

#if 1
void SsmCoordinatorActor::SetSyncTimeOutValue(int t)
{
    mpTimer->SetTimeOutValue(t);
}
#endif

#if 1
extern "C"
void SsmSetSyncTimeOutValue(int t)
{
    SsmCoordinatorActor* ssm_ptr = SsmCoordinatorService::Singleton();
    ssm_ptr->SetSyncTimeOutValue(t*1000);
    LMSG("SsmCoordiantorTimer Set Timeout Value to be"<<t*1000);
}
#endif