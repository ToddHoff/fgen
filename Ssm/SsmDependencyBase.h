#ifndef _SsmDependencyBase_h_
#define _SsmDependencyBase_h_

#include "rw/tpslist.h"               // HASA list
#include "Ssm/SsmConditionBase.h"     // HASA condition list
#include "Ssm/SsmActionBase.h"        // HASA action list
#include "Util/LnStream.h"            // USES streams
#include "rw/cstring.h"               // HASA string

class SsmCoordinatorActor;


/**
 * SsmDependency represents an Ssm dependency. A dependency is a list of
 * conditions. Each condition has a goal state. Each condition has a remembered
 * state that tracks the current value of the condition. The condition is updated
 * by SSMs when they change states. 
 *
 * Three sets of actions are supported: start actions, undo actions, and
 * goal actions. When all conditions have reached their goal state the
 * evalation state is REACHED. The initial evaluation state is NOT_REACHED.
 * When the REACHED state is attained the goal actions are executed.
 * If in the REACHES state and any conditions become such that their goal
 * state is not met then the evaluation state becomes NOT_REACHES and the
 * undo actions are executed. 
 */
class SsmDependencyBase
{
public:
	/**
	 * Evaluation states. Used to determine when to call FireActions
	 * and FireUndoActions.
	 */
	typedef enum
	{ 
		REACHED,
		NOT_REACHED

	} EvaluationState;


// LIFECYCLE

	/**
	 * Construct and initialize the object.
	 */
	SsmDependencyBase(SsmCoordinatorActor& rCoordinator, const char* pName, const char* pDescription);


	/**
	 * Destroy the object. It's virtual so all derived classes will be
	 * deleted as well.
	 */
	virtual ~SsmDependencyBase();


// OPERATORS

	friend ostream&         operator << (ostream& s, const SsmDependencyBase& o);


// ACTIONS

	/**
	 * Set all conditions to false.
	 */
	virtual void ClearConditions(void);


	/**
	 * Are all conditions true?
	 */
	virtual bool IsConditionsSatisfied(void);


	/**
	 * Execute all start actions associated with the dependency.
	 */
	virtual void FireStartActions(void);


	/**
	 * Execute all actions associated with going from NOT_REACHED
	 * to REACHED evaluation state.
	 */
	virtual void FireActions(void);


	/**
	 * Execute all actions associated with going from REACHED to
	 * NOT_REACHED state.
	 */
	virtual void FireUndoActions(void);


	/**
	 * Evaluate the dependency and execute actions as necessary.
	 */
	virtual void Evaluate(void);


	/**
	 * Tell all conditions to dermine their initial conditions.
	 *
	 * @return LN_OK if all conditions were determined. LN_FAIL
	 *   if a condition could not be satisifed. An exception will
	 *   be set.
	 *
	 * @exception Various.
	 */
	virtual LnStatus DetermineInitialConditions();


	/**
	 * Return the dependencies' conditions.
	 */
	RWTPtrSlist<SsmConditionBase>& Conditions();


	/**
	 * Return the dependencies' actions.
	 */
	RWTPtrSlist<SsmActionBase>& Actions();


	/**
	 * Add a new condition to the dependency.
	 *
	 * @param condition The condition to add. Memory: GIVEN.
	 */
	void AddCondition(SsmConditionBase& condition);


	/**
	 * Add an action that should be executed when the dependency
	 * is created.
	 *
	 * @param action The action to add. Memory: GIVEN.
	 */
	void AddStartAction(SsmActionBase& action);


	/**
	 * Add a new action to the dependency.
	 *
	 * @param action The action to add. Memory: GIVEN.
	 */
	void AddAction(SsmActionBase& action);


	/**
	 * Add a new undo action to the dependency.
	 *
	 * @param action The action to add. Memory: GIVEN.
	 */
	void AddUndoAction(SsmActionBase& action);


	/**
	 * Return the dependencies' start actions.
	 */
	RWTPtrSlist<SsmActionBase>& StartActions();


	/**
	 * Return the dependencies' undo actions.
	 */
	RWTPtrSlist<SsmActionBase>& UndoActions();


	/**
	 * Return the dependency name.
	 */
	const char*  Name(void) const;


	/**
	 * Return the dependency description.
	 */
	const char*  Description(void) const;



	/**
	 * Return the coordinator the dependency is assigned to.
	 *
	 * @return The coordinator the dependency is assigned to.
	 */
	SsmCoordinatorActor& Coordinator();


	/** 
	 * Get condition by name.
	 */
	SsmConditionBase* GetCondition(const char* pConditionName);


private:
	RWTPtrSlist<SsmConditionBase>  mConditionList;   // keep a list of conditions
	RWTPtrSlist<SsmActionBase>     mStartActionList; // keep a list of start actions
	RWTPtrSlist<SsmActionBase>     mActionList;      // keep a list of actions
	SsmCoordinatorActor&           mrCoordinator;    // the dependencies coordinator
	RWCString                      mName;            // name of the dependency
	RWCString                      mDescription;     // description
	EvaluationState                mEvalState;		
	RWTPtrSlist<SsmActionBase>     mUndoActionList;  // keep a list of actions


};


// INLINES
//
inline 	RWTPtrSlist<SsmConditionBase>& SsmDependencyBase::Conditions() 
{ return mConditionList; }


inline 	RWTPtrSlist<SsmActionBase>& SsmDependencyBase::Actions() 
{ return mActionList; }


inline 	RWTPtrSlist<SsmActionBase>& SsmDependencyBase::StartActions() 
{ return mStartActionList; }


inline 	RWTPtrSlist<SsmActionBase>& SsmDependencyBase::UndoActions() 
{ return mUndoActionList; }


#endif // _SsmDependencyBase_h_