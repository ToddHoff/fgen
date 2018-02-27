
#include "Ssm/ResolveConditionsJob.h"  // class implemented
#include "Util/Debug.h"
#include "Util/Dbc.h"                  // USES desing by contract


ResolveConditionsJob::ResolveConditionsJob(
	SsmCoordinatorActor& rCoordinator,
	SsmDependencyBase&   rDependency,
	const char*          pDescription)
	:	mrCoordinator(rCoordinator),
		mrDependency(rDependency),
		mDescription(pDescription)
{

}// ResolveConditionsJob


ResolveConditionsJob::~ResolveConditionsJob()
{
	//D3(&mrCoordinator, "~ResolveConditionsJob: " << mDescription);

}// ~ResolveConditionsJob


void
ResolveConditionsJob::Perform()
{
	D1(&mrCoordinator, "ResolveConditionsJob:Perform: getting initial conditions depname= " 
		<< mrDependency.Name());

	// Tell the dependency to go check its initial conditions.
	//
	LnStatus rc= mrDependency.DetermineInitialConditions();

	// If an initial condition could not be resolved then an
	// assertion is reased.
	//
	LASSERT(rc == LN_OK, &mrCoordinator, "SSM_ERROR", 
		"FAIL_GET_INITIAL_COND", *LnException::Get());

	// Trigger actions if conditions have been met.
	//
	mrDependency.Evaluate();


	// All done.
	//
	Finished();

}// Perform
