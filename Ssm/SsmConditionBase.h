#ifndef _SsmConditionBase_h_
#define _SsmConditionBase_h_

#include "Util/Typeable.h"          // USES typeable
#include "Util/LnStream.h"          // USES streams
#include "Util/Dumpable.h"          // ISA dumpable


class SsmDependencyBase;


/**
 * SsmConditionBase is a base class for conditions in Ssm. Classes
 * are expected to derive from this class and implement specific
 * condition behaviour.
 */
class SsmConditionBase : public Dumpable
{
public:
// LIFECYLCE


	/**
	 * Destroy the object. It's virtual so all derived classes will be
	 * deleted as well.
	 */
	virtual ~SsmConditionBase();



// OPERATORS

   /**
    * Output object to a given stream.
    *
    * @param s The output stream to write to.
    * @param o The object to print.
    *
    * @return The stream written to.
    */
	friend ostream&         operator << (ostream& s, const SsmConditionBase& o);


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
	virtual LnStatus InjectEvent(Typeable* pEvent) = 0;


	/**
	 * Is the condition satisfied?
	 *
	 * @return true if this condition has been satisfied.
	 */
	virtual bool IsSatisfied(void) const = 0;


	/**
	 * Set the condition to false.
	 */
	virtual void Clear(void) = 0;


	/**
	 * Return true if this condition should have its initial condition
	 * determined.
	 *
	 * @return true If this condition should have its initial condition
	 *    determined.
	 */
	virtual bool IsResolveInitial(void) const = 0;


	/**
	 * Return the dependency associated with the condition.
	 *
	 * @return  Return the dependency associated with the condition.
	 */
	virtual SsmDependencyBase&  Dependency(void) = 0;


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
	virtual LnStatus DetermineInitialCondition(void) = 0;


	/**
	 * Description of the condition. It should be some string that can
	 * be used to know which condition it is. Mainly used for debugging.
	 *
	 * @return A description of the condition.
	 */
	virtual const char* Description(void) const = 0;


	/**
	 * Get the condition name.
	 */
	virtual const char* GetConditionName(void) const = 0;


	/**
	 * Get the source ID.
	 */
	virtual uint32 GetSourceId(void) const = 0;


private:

};




#endif // _SsmConditionBase_h_