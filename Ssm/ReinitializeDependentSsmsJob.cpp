
#include "Ssm/ReinitializeDependentSsmsJob.h"	// class implemented
#include "Util/Debug.h"
#include "Util/Dbc.h"							// USES desing by contract


ReinitializeDependentSsmsJob::ReinitializeDependentSsmsJob(
	SsmCoordinatorActor&	rCoordinator,
	SsmSmBase&				rReinitializedSsm)
	:	mrCoordinator(rCoordinator),
		mrReinitializedSsm(rReinitializedSsm)
{

}// ReinitializeDependentSsmsJob


ReinitializeDependentSsmsJob::~ReinitializeDependentSsmsJob()
{

}// ~ReinitializeDependentSsmsJob


void
ReinitializeDependentSsmsJob::Perform()
{
	D1(&mrCoordinator, "ReinitializeDependentSsmsJob:Perform: ssm=" 
		<<mrReinitializedSsm.SsmName());

	// Tell all the dependent SSMs about the change in this SSM.
	//
	mrCoordinator.PublishStateChange(mrReinitializedSsm, true /*initiailizing*/);
	
	// All done.
	//
	Finished();

}// Perform
