#ifndef _SsmActionFactory_h_
#define _SsmActionFactory_h_


#include "Util/Properties.h"       // USES properties
#include "Ssm/SsmActionBase.h"  // USES condition

class SsmDependencyBase;


/**
 * SsmActionFactory is responsible for creating the correct type
 * of dependency from a properties description.
 *
 * See the Ssm package documentation for more details.
 */
class SsmActionFactory
{
public:

	/**
	 * Create knows how to take a properties description of a dependency
	 * and return a dependency object of the correct type.
	 *
	 * @param rDependency  The dependency the action belongs to.
	 * @param rDescription A properties object describing a dependency.
	 *
	 * @return 0 - if the dependency could not be created from the
	 *   properties description; a dependency object.
	 */
	virtual SsmActionBase*  Create(
		SsmDependencyBase& rDependency, 
		Properties&        rDescription);

};



#endif // _SsmActionFactory_h_