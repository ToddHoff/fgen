
#ifndef _SsmSmBase_h_
#define _SsmSmBase_h_

#include "Project/LnTypes.h"  // common types
#include "Util/Listenable.h"  // ISA listenable
#include "Util/Typeable.h"    // ISA typeable

/**
 * SsmBase is a base class for all FSMs using the Ssm coordinator
 * service. The base class is derived from Listenable so other objects
 * can listen for state changes in a FSM.
 */
class SsmSmBase : public Listenable, public Typeable
{
public:
	/**
	 * Tell the FSM an event has occurred. It is expected to turn
	 * the named event into a "real" event in the state machine.
	 *
	 * @param pEventName the name of the event to drive the state machine.
	 *
	 * @return LN_OK If the event was recognized and the transition was valid.
	 * @return LN_FAIL If the event wasn't recognized or no valid transition
	 *    was available for the event.
	 */
	virtual LnStatus  SsmEvent(const char* pEventName) = 0;


	/**
	 * Return the current state of the state machine.
	 *
	 * @return The current state of the state machine.
	 */
	virtual const char* SsmCurrentState(void) const = 0;


	/**
	 * Return the name of the Fsm.
	 *
	 * @return The name of the Fsm.
	 */
	virtual const char* SsmName(void) = 0;


	/**
	 * Return true if the current state, returned by SsmCurrentState,
	 * matches the state specified by the
	 * first parameter. The comparison is case sensitive.
	 *
	 * @param pCompareTo The state to compare against the current state.
	 *
	 * @return true if the current state matches the first parameter.
	 *   The comparison is case sensitive.
	 */
	bool               IsSsmState(const char* pCompareTo) const;


// Typeable interace

    /**
     * Return the class name of the object.
     *
     * @return The class name of the object. Memory: BORROWED.
     */
    virtual const char* ClassName(void) const { return "SsmSmBase"; }


    /**
     * Is the runtime class the same as the passed in class? Derived class
     * are responsible for chaining together IsClass calls so multiply derived
     * classes can be tested against.
     *
     * @param name The name of the class to compare against this object's class.
     *
     * @return true If the name of the class is the same as <B>name</B>.
     * @return false If the name of the class is not the same as <B>name</B>.
     */
    virtual bool IsClass(const char* name) const;


};


#endif // _SsmSmBase_h_
