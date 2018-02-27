
#include "Ssm/InjectEventJob.h"		// class implemented
#include "Util/Debug.h"
#include "Util/Dbc.h"				// USES desing by contract
#include "Ssm/SsmPublic.h"			// USES


InjectEventJob::InjectEventJob(
	SsmCoordinatorActor&	rCoordinator,
	AvListMsg&				rRequest)
	:	mrCoordinator(rCoordinator),
		mrRequest(rRequest)
{
	const char* sm_name= mrRequest.AsString(SsmPublic::SmNameAn());
	REQUIRE(sm_name != 0);

	D1(&rCoordinator, "InjectEventJob: create ssm=" << sm_name);

}// InjectEventJob


InjectEventJob::~InjectEventJob()
{
	mrRequest.Destroy();

}// ~InjectEventJob


void
InjectEventJob::Perform()
{
	D1(&mrCoordinator, "InjectEventJob:Perform:");

	mrCoordinator.ImplementInjectEventOp(mrRequest);
	
	// All done.
	//
	Finished();

}// Perform
