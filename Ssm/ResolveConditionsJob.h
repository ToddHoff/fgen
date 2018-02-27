#ifndef _ResolveConditionsJob_h_
#define _ResolveConditionsJob_h_


#include "Com/Job.h"
#include "Ssm/SsmCoordinatorActor.h"
#include "Ssm/SsmDependencyBase.h"


/**
 * Create
 */
class ResolveConditionsJob : public Job
{
public:
	ResolveConditionsJob(
		SsmCoordinatorActor& rCoordinator,
		SsmDependencyBase&   rDependency,
		const char*          pDescription= "ResolveConditionsJob");

	~ResolveConditionsJob();

	/**
	 * If the initial conditions could not all be determined then
	 * then an assertion is thrown.
	 */
	void Perform();

private:
	SsmCoordinatorActor& mrCoordinator;
	SsmDependencyBase&   mrDependency;
	RWCString            mDescription;
};


#endif // _ResolveConditionsJob_h_