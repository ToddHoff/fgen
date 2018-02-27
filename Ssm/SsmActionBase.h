#ifndef _SsmActionBase_h_
#define _SsmActionBase_h_

#include "Util/LnStream.h"          // USES streams
#include "Util/Dumpable.h"          // ISA dumpable

class SsmDependencyBase;

/**
 * SsmActionBase is a base class for actions in Ssm. Classes
 * are expected to derive from this class and implement specific
 * action behaviour.
 */
class SsmActionBase : public Dumpable
{
public:
// LIFECYCLE

	/**
	 * Destroy the object. It's virtual so all derived classes will be
	 * deleted as well.
	 */
	virtual ~SsmActionBase();


// OPERATORS

   /**
    * Output object to a given stream.
    *
    * @param s The output stream to write to.
    * @param o The object to print.
    *
    * @return The stream written to.
    */
	friend ostream&         operator << (ostream& s, const SsmActionBase& o);


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
	 * The implementaiton for the action.
	 */
	virtual void Doit(void) = 0;


	/**
	 * Return the dependency associated with the action.
	 *
	 * @return  Return the dependency associated with the action.
	 */
	virtual SsmDependencyBase&  Dependency(void) = 0;


};


#endif // _SsmActionBase_h_