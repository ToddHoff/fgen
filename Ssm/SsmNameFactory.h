#ifndef _SsmNameFactory_h_
#define _SsmNameFactory_h_

#include "rw/cstring.h"   // USES string

/**
 * SsmNameFactory knows how to make the various names used in Ssm.
 */
class SsmNameFactory
{
public:

	/**
	 * Return the Com object name for a particular state machine in the
	 * name space.
	 *
	 * @param objName The Com object name for a particular state machine in the
	 *   name space. It will have the form "/obj/name".
	 * @param pSsmName The name of the state machine. 
	 *
	 * @return The reference to objName.
	 */
	static RWCString& CalcSmObjName(RWCString& objName, const char* pSsmName);

};



#endif // _SsmNameFactory_h_
