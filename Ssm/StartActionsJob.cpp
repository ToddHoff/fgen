#include "Ssm/StartActionsJob.h"  // class implemented
#include "Util/Debug.h"


StartActionsJob::StartActionsJob(
	SsmCoordinatorActor& rCoordinator,
	SsmDependencyBase&   rDependency,
	const char*          pDescription)
	:	mrCoordinator(rCoordinator),
		mrDependency(rDependency),
		mDescription(pDescription)
{

}// StartActionsJob


StartActionsJob::~StartActionsJob()
{
	//D3(&mrCoordinator, "~StartActionsJob: " << mDescription);

}// ~StartActionsJob


void
StartActionsJob::Perform()
{
	D1(&mrCoordinator, "StartActionsJob:Perform: getting initial conditions dep name= " <<
	   mrDependency.Name());


	// If the dependency has any start actions fire them now.
	//
	mrDependency.FireStartActions();


	// All done.
	//
	Finished();

}// Perform
