#ifndef _StartActionsJob_h_
#define _StartActionsJob_h_


#include "Com/Job.h"
#include "Ssm/SsmCoordinatorActor.h"
#include "Ssm/SsmDependencyBase.h"


/**
 * Create
 */
class StartActionsJob : public Job
{
public:
	StartActionsJob(
		SsmCoordinatorActor& rCoordinator,
		SsmDependencyBase&   rDependency,
		const char*          pDescription= "StartActionsJob");

	~StartActionsJob();

	void Perform();

private:
	SsmCoordinatorActor& mrCoordinator;
	SsmDependencyBase&   mrDependency;
	RWCString            mDescription;
};


#endif // _StartActionsJob_h_