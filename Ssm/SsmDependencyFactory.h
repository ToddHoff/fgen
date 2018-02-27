#ifndef _SsmDependencyFactory_h_
#define _SsmDependencyFactory_h_


#include "Util/Properties.h"          // USES properties
#include "Ssm/SsmDependencyBase.h"    // USES condition
#include "Ssm/SsmConditionFactory.h"  // HASA condition factory
#include "Ssm/SsmActionFactory.h"     // HASA action factory

class SsmCoordinatorActor;


/**
 * SsmDependencyFactory is responsible for creating the correct type
 * of dependency from a properties description.
 *
 * See the Ssm package documentation for more details.
 */
class SsmDependencyFactory
{
public:
	/** 
	 * Create and initialize the factory.
	 *
	 * @param rCoordinator The coordinator associated with the factory.
	 */
	SsmDependencyFactory(SsmCoordinatorActor& rCoordinator);

	/**
	 * Create knows how to take a properties description of a dependency
	 * and return a dependency object of the correct type.
	 *
	 * @param rDescription A properties object describing a dependency.
	 *
	 * @return 0 - if the dependency could not be created from the
	 *   properties description; a dependency object.
	 */
	virtual SsmDependencyBase*  Create(Properties&  rDescription);

private:
	SsmConditionFactory   mConditionFactory;
	SsmActionFactory      mActionFactory;
	SsmCoordinatorActor&  mrCoordinator;

};



#endif // _SsmDependencyFactory_h_