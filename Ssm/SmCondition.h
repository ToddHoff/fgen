#ifndef _SmCondition_h_
#define _SmCondition_h_


#include "Ssm/SsmConditionBase.h"   // ISA condition
#include "Ssm/SsmDependencyBase.h"  // USES dependencies
#include "rw/cstring.h"             // HASA string
#include "Osencap/LnMutex.h"          // HASA mutex



/**
 * SmCondition is a condition based on a state machine state. 
 */
class SmCondition : public SsmConditionBase
{
public:
// LIFECYLCE

	/**
	 * Construct and initialize a condition.
	 *
	 * @param rDependency The dependency the condition
	 *    is associated with.
	 * @param pSmName The name of the state machine the condition is dependent
	 *    on.
	 * @param pGoalState The state the state machine needs to be in for the 
	 *    condition to be satisfied.
	 * @param pResolvePolicy The policy to use for resolving initial conditions.
	 *    The default is to resolve initial conditions. If it is set to 
	 *    "NoInitial" the the condition will not be initially resolved, it will
	 *    instead be driven by events.
	 */
	SmCondition(
		SsmDependencyBase& rDependency, 
		const char* pSmName, 
		const char* pGoalState,
		const char* pResolvePolicy= 0);


	/**
	 * Destroy the object. It's virtual so all derived classes will be
	 * deleted as well.
	 */
	virtual ~SmCondition();


// OPERATORS


   /**
    * Output object to a given stream.
    *
    * @param s The output stream to write to.
    * @param o The object to print.
    *
    * @return The stream written to.
    */
	friend ostream&         operator << (ostream& s, const SmCondition& o);


    /**
     * Output the object to a stream. Because this method
     * is virtual the most derived class is dumped. Derived class should override
	 * this method to dump actor specific state. Remeber to chain the dump
	 * methods together.
     *
     * @param s The output stream to write to.
     * @param fmt The format of the output to write to the stream.
     * @param depth The indentation level this object should be output at.
     *
     * @return The stream written to.
	 *
	 * @see Module#Dump
     */
    virtual ostream&        Dump(ostream& s, DumpType fmt= DUMP_DEBUG , int depth= 0) const;


// ACTIONS

	/**
	 * An event arrived for the condition. It's up to the condition
	 * to understand the event and determine if the
	 *
	 * @param pEvent The event for the condition to evaluate.
	 *    Memory: BORROWED.
	 */
	virtual LnStatus InjectEvent(Typeable* pEvent);


	/**
	 * Is the condition satisfied?
	 *
	 * @return true if this condition has been satisfied.
	 */
	virtual bool IsSatisfied(void) const;


	/**
	 * Set the condition to false.
	 */
	virtual void Clear(void);


	/**
	 * Get this initial condition for this condition. It will block
	 * until the condition is resolved or a non-recoverable failure
	 * has occured.
	 *
	 * @return LN_OK if the condition was determined, LN_FAIL if
	 *    the condition could not be determined.
	 *
	 * @exception ComExecption
	 * @exception TBD
	 */
	virtual LnStatus DetermineInitialCondition(void);



	/**
	 * Return true if this condition should have its initial condition
	 * determined.
	 *
	 * @return true If this condition should have its initial condition
	 *    determined.
	 */
	virtual bool IsResolveInitial(void) const;


	/**
	 * Return the dependency associated with the condition.
	 *
	 * @return  Return the dependency associated with the condition.
	 */
	SsmDependencyBase&  Dependency(void);

	/**
	 * Description of the condition. It should be some string that can
	 * be used to know which condition it is. Mainly used for debugging.
	 *
	 * @return A description of the condition.
	 */
	virtual const char* Description(void) const;


	/**
	 * Get the condition name.
	 */
	virtual const char* GetConditionName(void) const;


	/**
	 * Get the source ID.
	 */
	virtual uint32 GetSourceId(void) const;

private:
	SsmDependencyBase&  mrDependency;
	RWCString           mSmName;
	RWCString           mGoalState;
	RWCString           mRememberedState;
	RWCString           mNameSpaceObjName;
	RWCString           mResolvePolicy;
	LnMutex             mProtection;      // mutex for data structures
	uint32				mSourceId;

};




#endif // _SmCondition_h_