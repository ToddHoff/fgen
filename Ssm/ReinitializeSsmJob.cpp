
#include "Ssm/ReinitializeSsmJob.h"	// class implemented
#include "Util/Debug.h"
#include "Util/Dbc.h"				// USES desing by contract
#include "Ssm/SsmPublic.h"			// USES



ReinitializeSsmJob::ReinitializeSsmJob(
	SsmCoordinatorActor&	rCoordinator,
	AvListMsg&				rRequest)
	:	mrCoordinator(rCoordinator),
		mrRequest(rRequest)
{
	const char* sm_name= mrRequest.AsString(SsmPublic::SmNameAn());
	REQUIRE(sm_name != 0);

	D1(&rCoordinator, "ReinitializeSsmJob: create ssm=" << sm_name);

}// ReinitializeSsmJob


ReinitializeSsmJob::~ReinitializeSsmJob()
{
	mrRequest.Destroy();

}// ~ReinitializeSsmJob


void
ReinitializeSsmJob::Perform()
{
	D1(&mrCoordinator, "ReinitializeSsmJob:Perform:");

	mrCoordinator.ImplementUpdateDependencyOp(mrRequest);
	
	// All done.
	//
	Finished();

}// Perform
