
#include "Ssm/SmCondition.h"           // class implemented
#include "Util/Dbc.h"                  // USES design by contract
#include "Com/AvListMsg.h"             // USES av message
#include "Util/Debug.h"                // USES debug
#include "Com/ComService.h"            // USES com 
#include "Ssm/SsmNameFactory.h"        // USES name factor
#include "Com/ComException.h"          // USES com exception
#include "Ssm/SsmCoordinatorActor.h"   // USES coordinator
#include "Osencap/LnLockGuard.h"       // USES lock guard
#include "Ssm/SsmPublic.h"				// USES


SmCondition::SmCondition(
	SsmDependencyBase& rDependency, 
	const char* pSmName, 
	const char* pGoalState,
	const char* pResolvePolicy) 
	:	mrDependency(rDependency),
		mSourceId(0)
{
	REQUIRE(pSmName != 0);
	REQUIRE(pGoalState != 0);

	mSmName           = pSmName;
	mGoalState        = pGoalState;
    mRememberedState  = "SmConditionInit";

	if (pResolvePolicy) mResolvePolicy= pResolvePolicy;

	SsmNameFactory::CalcSmObjName(mNameSpaceObjName, pSmName);

}// SmCondition


SmCondition::~SmCondition()
{

}// SmCondition


ostream&         
operator << (ostream& o, const SmCondition& val)
{ 
//	val.SmCondition::Dump(o);

	val.Dump(o);

	return o;

}// <<


ostream&        
SmCondition::Dump(ostream& o, DumpType /*fmt*/, int /*depth*/) const
{
   o << "SmCondition=" << "\n"
	 << "   SmName           =" << mSmName             <<"\n"
	 << "   GoalState        =" << mGoalState          <<"\n"
	 << "   RememberedState  =" << mRememberedState    <<"\n"
	 << "   Description      =" << Description()       <<"\n"
	 << "   SourceId         =" << dec << mSourceId           <<"\n"
	 << endl;

   return o;

}// <<


uint32 
SmCondition::GetSourceId(void) const
{ return mSourceId; }


SsmDependencyBase& 
SmCondition::Dependency(void) 
{ return mrDependency; }


const char* 
SmCondition::Description(void) const
{ return mSmName.data(); }


const char* 
SmCondition::GetConditionName(void) const
{ return mSmName.data(); }


LnStatus 
SmCondition::InjectEvent(Typeable* pEvent)
{
	SET_XCEPTION_IF(
		pEvent->IsClass(AvListMsg::ThisClassName()) == false,
		LN_FAIL, 
		"SSM", "NO_DEPENDENCIES", "state", 0, 
		"SmCondition::InjectEvent: Unhandled message type=" << pEvent->ClassName());

	AvListMsg* av= (AvListMsg*) pEvent;

	const char* state= av->AsString(SsmPublic::StateNameAn());
	uint32 source_id= av->AsUint32(SsmPublic::SourceIdAn());
	bool is_initializing= av->AsBool(SsmPublic::IsInitializingAn());


	SET_XCEPTION_IF(
		state == 0,
		LN_FAIL, 
		SsmPublic::SsmErrorAv(), SsmPublic::InvalidReqErrorAv(), "state", 0, 
		"SmCondition::InjectEvent: State is required=" << *av);

	D1(&Dependency().Coordinator(), "Ssm:SmCondition:InjectEvent: dep="
		<< Dependency().Name() << " ssm=" << mSmName <<
		" newstate=" << state << " curstate=" << mRememberedState <<
		" cursid=" << mSourceId << " newsid=" << source_id << 
		" isinit=" << is_initializing);

	LnLockGuard lock(mProtection);

    
    if (is_initializing ||
        ( (strcmp(mRememberedState, "SmConditionInit") == 0) && 
             (strcmp(state,"OUT_OF_SYNC") == 0 )) )
    {
        mSourceId = source_id;
    }
    else if ((strcmp(mRememberedState, "SmConditionInit") == 0) && 
             (strcmp(state,"OUT_OF_SYNC") != 0 ))
    {
        DetermineInitialCondition();
    }
    else
	{
		SET_XCEPTION_IF(
			source_id != mSourceId,
			LN_FAIL, 
			SsmPublic::SsmErrorAv(), SsmPublic::InvalidReqErrorAv(), "SourceId", 0, 
			"SmCondition::InjectEvent: mismatch of sourceIds. Current="
			<< mSourceId << " Set=" << source_id);
	}



	mRememberedState= state;

    	D1(&Dependency().Coordinator(), "Ssm:SmCondition:InjectEvent: dep="
		<< Dependency().Name() << " ssm=" << mSmName <<
		" newstate=" << state << " curstate=" << mRememberedState <<
		" cursid=" << mSourceId << " newsid=" << source_id << 
		" isinit=" << is_initializing);


	return LN_OK;

}// InjectEvent



bool 
SmCondition::IsSatisfied(void) const
{
	// The condition is satisfied if the goal state and the state machines
	// state are the same.
	//
	LnLockGuard lock(((SmCondition&) *this).mProtection);

	return (mGoalState.compareTo(mRememberedState, RWCString::ignoreCase) == 0)
			? true 
			: false;

}// IsSatisfied



void 
SmCondition::Clear(void)
{
	LnLockGuard lock(mProtection);

	mRememberedState= "";

}// Clear



bool 
SmCondition::IsResolveInitial(void) const
{

	return (mResolvePolicy.compareTo("NoInitial", RWCString::ignoreCase) == 0)
			? false 
			: true;	

}// IsResolveInitial



LnStatus 
SmCondition::DetermineInitialCondition()
{
	D1(&Dependency().Coordinator(), 
		"SmCondition:DetermineInitialCondition sm=" << mSmName
		<< " dep=" << Dependency().Name());


	// Create the get state request.
	//
	AvListMsg* req= new AvListMsg(1, 0, SsmPublic::GetStateOp());
	req->Value(SsmPublic::SmNameAn(), (char*) mSmName.data(), true, true);
	req->Value("FromDep", (char*) Dependency().Name(), true, true);


	// Invoke the operation on the state machine whos state we depend
	// on. This operation will retry for a very long time.
	//
	LnObject* reply      = 0;
	bool is_cancel       = false;
	unsigned int  retries= 3000;
	LnStatus rc= ComService::Singleton()->RetrySyncInvoke(
		AvListMsg::ThisClassName(),
		SsmPublic::GetStateReplyOp(),
		reply, 
		mNameSpaceObjName, 
		SsmPublic::GetStateOp(),
		req, 
		retries, 
		is_cancel, 
		5000 /*5 seconds*/);

	// Nothing to do on failure. The exception should be set.
	//
	if (rc == LN_FAIL)
	{
		D1(&Dependency().Coordinator(), 
				"Ssm:DetermineInitialCondition ssm=" << mSmName <<
				"Can't get condition for reason ComException= " << *LnException::Get());
	}
	else
	{
		// Got a AvListMsg as the reply
		//
		AvListMsg* av_reply= (AvListMsg*) reply;

		// Force initializing semantics. We want the source id and
		// state to be believed.
		//
		av_reply->Value(SsmPublic::IsInitializingAn(), true);


		rc= InjectEvent(av_reply);
	}


	if (reply) reply->Destroy();


	// Return the resulting status.
	//
	return rc;

}//  DetermineInitialCondition