#ifndef _SmAction_h_
#define _SmAction_h_

#include "Util/LnStream.h"          // USES streams
#include "Ssm/SsmActionBase.h"		// ISA action
#include "rw/cstring.h"             // HASA string
#include "Ssm/SsmDependencyBase.h"  // USES dependencies
#include "Com/InvokeProps.h"        // HASA invoke props


/**
 * SmAction is an action that knows how to send an event to a state
 * machine.
 */
class SmAction : public SsmActionBase
{
public:
// LIFECYCLE

	/**
	 * Construct and initialize an action.
	 *
	 * @param rDependency The dependency the action
	 *    is associated with.
	 * @param pSmName The name of the state machine the action should send
	 *    an event to.
	 * @param pEvent The event the state machine should be sent.
	 */
	SmAction(SsmDependencyBase& rDependency, const char* pSmName, const char* pEvent);


	/**
	 * Destroy the object. It's virtual so all derived classes will be
	 * deleted as well.
	 */
	virtual ~SmAction();


// OPERATORS

   /**
    * Output object to a given stream.
    *
    * @param s The output stream to write to.
    * @param o The object to print.
    *
    * @return The stream written to.
    */
	friend ostream&         operator << (ostream& s, const SmAction& o);


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


// OPERATIONS

	/**
	 * Set the condition to false.
	 */
	virtual void Doit(void);


	/**
	 * Return the dependency associated with the action.
	 *
	 * @return  Return the dependency associated with the action.
	 */
	virtual SsmDependencyBase&  Dependency(void);

private:
	SsmDependencyBase&  mrDependency;
	RWCString           mSmName;
	RWCString           mEvent;
	RWCString           mNameSpaceObjName;
	InvokeProps*        mpInvokeProps;

};


#endif // _SmAction_h_