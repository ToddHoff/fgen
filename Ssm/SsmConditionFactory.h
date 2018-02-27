#ifndef _SsmConditionFactory_h_
#define _SsmConditionFactory_h_


#include "Util/Properties.h"       // USES properties
#include "Ssm/SsmConditionBase.h"  // USES condition

class SsmDependencyBase;


/**
 * SsmConditionFactory is responsible for creating the correct type
 * of condition from a properties description.
 *
 * See the Ssm package documentation for more details.
 */
class SsmConditionFactory
{
public:

	/**
	 * Create knows how to take a properties description of a condition
	 * and return a condition object of the correct type.
	 *
	 * @param rDependency  The dependency the condition belongs to.
	 * @param rDescription A properties object describing a condition.
	 *
	 * @return 0 - if the condition could not be created from the
	 *   properties description; a condition object.
	 */
	virtual SsmConditionBase*  Create(
		SsmDependencyBase& rDependency, 
		Properties&        rDescription);

};



#endif // _SsmConditionFactory_h_