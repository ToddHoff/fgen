

#include "Ssm/SsmDependencyBase.h"     // class implemented
#include "Util/Dbc.h"                  // USES design by contract
#include "Util/Debug.h"                // USES debug
#include "Ssm/SsmCoordinatorActor.h"   // USES coordinator



SsmDependencyBase::SsmDependencyBase(
	SsmCoordinatorActor& rCoordinator,
	const char*          pName,
	const char*          pDescription)
	:	mrCoordinator(rCoordinator),
		mEvalState(NOT_REACHED)
{
	mName= (pName) ? pName : "none";
	mDescription= (pDescription) ? pDescription : "none";


}// SsmDependencyBase


SsmDependencyBase::~SsmDependencyBase()
{

}// SsmDependencyBase



ostream&         
operator << (ostream& s, const SsmDependencyBase& o)
{ 
	SsmDependencyBase* non_const= (SsmDependencyBase*) &o;
	SsmSmBase* sm= non_const->Coordinator().GetRegisteredSsm(o.mName);

	s << "SsmDependencyBase:" <<endl;
	s << "   Name           = " << o.mName <<endl;
	s << "   Description    = " << o.mDescription <<endl;
	s << "   SsmCurrentState= " << sm->SsmCurrentState() << endl;	

	s << "   Conditions:" <<endl;

	RWTPtrSlistIterator<SsmConditionBase> citer(((SsmDependencyBase&) o).mConditionList);
	for (SsmConditionBase* cond= 0; (cond= (SsmConditionBase*) citer()); )
	{
		cond->Dump(s);

	}// foreach condition


	s << endl << "   Start Actions:" <<endl;
	RWTPtrSlistIterator<SsmActionBase> saiter(((SsmDependencyBase&) o).mStartActionList);

	SsmActionBase* action= 0;
	for (action= 0; (action= (SsmActionBase*) saiter()); )
	{
		action->Dump(s);

	}// foreach action



	s <<  endl << "   Actions:" <<endl;
	RWTPtrSlistIterator<SsmActionBase> aiter(((SsmDependencyBase&) o).mActionList);

	for (action= 0; (action= (SsmActionBase*) aiter()); )
	{
		action->Dump(s);

	}// foreach action


	s <<  endl << "   UndoActions:" <<endl;
	RWTPtrSlistIterator<SsmActionBase> uaiter(((SsmDependencyBase&) o).mUndoActionList);

	for (action= 0; (action= (SsmActionBase*) uaiter()); )
	{
		action->Dump(s);

	}// foreach action



	return s; 

}// <<


const char*  
SsmDependencyBase::Name(void) const
{ return mName.data(); }


const char*  
SsmDependencyBase::Description(void) const
{ return mDescription.data(); }



SsmCoordinatorActor& 
SsmDependencyBase::Coordinator()
{ return mrCoordinator; }


void 
SsmDependencyBase::AddCondition(SsmConditionBase& condition)
{
	// Add the condition to the list.
	//
	mConditionList.insert(&condition);

}// AddCondition


void 
SsmDependencyBase::AddAction(SsmActionBase& action)
{
	// Add the condition to the list.
	//
	mActionList.insert(&action);

}// AddAction


void 
SsmDependencyBase::AddUndoAction(SsmActionBase& action)
{
	// Add the condition to the list.
	//
	mUndoActionList.insert(&action);

}// AddUndoAction


void 
SsmDependencyBase::AddStartAction(SsmActionBase& action)
{
	// Add the condition to the list.
	//
	mStartActionList.insert(&action);

}// AddAction


SsmConditionBase*		
SsmDependencyBase::GetCondition(const char* pConditionName)
{
	RWTPtrSlistIterator<SsmConditionBase> iter(mConditionList);

	for (SsmConditionBase* cond= 0; (cond= (SsmConditionBase*) iter()); )
	{
		RWCString name= cond->GetConditionName();

		if (name == pConditionName)
		{
			return cond;
		}

	}// foreach condition

	return 0;

}// GetCondition


void 
SsmDependencyBase::ClearConditions(void)
{
	RWTPtrSlistIterator<SsmConditionBase> iter(mConditionList);

	for (SsmConditionBase* cond= 0; (cond= (SsmConditionBase*) iter()); )
	{
		cond->Clear();

	}// foreach condition

}// ClearConditions


bool 
SsmDependencyBase::IsConditionsSatisfied(void)
{
	// If there aren't any condition then conditions can't be satisfied.
	// Otherwise we could get in an infinite loop.
	//
	if (mConditionList.entries() <= 0) return false;


	RWTPtrSlistIterator<SsmConditionBase> iter(mConditionList);

	for (SsmConditionBase* cond= 0; (cond= (SsmConditionBase*) iter()); )
	{
		// If any condition is false then the conditions can't be satisfied.
		//
        D1(&Coordinator(), "SsmDependencyBase::IsConditionsSatisfied: " << cond->GetConditionName()<<" result: "<<cond->IsSatisfied());
		if (cond->IsSatisfied() == false) 
			return false;

	}// foreach condition


	// Getting here means all conditions were satisfied.
	//
	return true; 

}// IsConditionsSatisfied


void 
SsmDependencyBase::FireStartActions(void)
{
	D1(&Coordinator(), "SsmDependencyBase::FireStartActions: " << Name());


	RWTPtrSlistIterator<SsmActionBase> iter(mStartActionList);

	for (SsmActionBase* action= 0; (action= (SsmActionBase*) iter()); )
	{
		action->Doit();

	}// foreach action


}// FireStartActions


void 
SsmDependencyBase::FireActions(void)
{
	D1(&Coordinator(), "SsmDependencyBase::FireActions: " << Name());


	RWTPtrSlistIterator<SsmActionBase> iter(mActionList);

	for (SsmActionBase* action= 0; (action= (SsmActionBase*) iter()); )
	{
		action->Doit();

	}// foreach action


}// FireActions


void 
SsmDependencyBase::FireUndoActions(void)
{
	D1(&Coordinator(), "SsmDependencyBase::FireUndoActions: " << Name());


	RWTPtrSlistIterator<SsmActionBase> iter(mUndoActionList);

	for (SsmActionBase* action= 0; (action= (SsmActionBase*) iter()); )
	{
		action->Doit();

	}// foreach action

}// FireUndoActions


LnStatus
SsmDependencyBase::DetermineInitialConditions(void)
{
	D1(&Coordinator(), "SsmDependencyBase::DetermineInitialConditions Name=" << Name());

	RWTPtrSlistIterator<SsmConditionBase> iter(mConditionList);

	for (SsmConditionBase* cond= 0; (cond= (SsmConditionBase*) iter()); )
	{
        D3(&Coordinator(),"::::::::::::::::::::cond: "<<cond<<"IsResovle: "<<cond->IsResolveInitial());
		if (cond->IsResolveInitial() == true)
		{
			continue;
		}

		LnStatus rc= cond->DetermineInitialCondition();

		// If the condition could be determined then we bail, there's
		// nothing for us to do.
		//
		if (rc != LN_OK) 
		{
			LnException::Clear();
		}

	}// foreach condition


	D1(&Coordinator(), "SsmDependencyBase::DetermineInitialConditions resolved Name=" << Name());

	return LN_OK;

}// DetermineInitialConditions



void 
SsmDependencyBase::Evaluate(void)
{
	//D1(&Coordinator(), "SsmDependencyBase::Evaluate: " << Name());

	EvaluationState prev_eval_state= mEvalState;

	// If all the conditions are satisfied then execute the associated actions.
	//
	if (IsConditionsSatisfied())
	{
		mEvalState= REACHED;
	}
	else
	{
		mEvalState= NOT_REACHED;
	}
	
    D1(&Coordinator(), "SsmDependencyBase::Evaluate:"<<Name()<<"  prev_eval_state" << prev_eval_state
        <<"  current eval state" << mEvalState);

	if (prev_eval_state == NOT_REACHED && mEvalState == REACHED)
	{
		FireActions();
	}
	else if (prev_eval_state == REACHED && mEvalState == NOT_REACHED)
	{
		FireUndoActions();
	}


}// Evaluate

