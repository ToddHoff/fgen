#ifndef _ReinitializeDependentSsmsJob_h_
#define _ReinitializeDependentSsmsJob_h_


#include "Com/Job.h"
#include "Ssm/SsmCoordinatorActor.h"
#include "Ssm/SsmDependencyBase.h"


/**
 * Tell all other SSM dependent on a SSM that it should reinitialize
 * because the SSM they are dependent on is reinitializing.
 */
class ReinitializeDependentSsmsJob : public Job
{
public:
	ReinitializeDependentSsmsJob(
		SsmCoordinatorActor&	rCoordinator,
		SsmSmBase&				rReinitializedSsm);

	~ReinitializeDependentSsmsJob();

	/**
	 * If the initial conditions could not all be determined then
	 * then an assertion is thrown.
	 */
	void Perform();

private:
	SsmCoordinatorActor& mrCoordinator;
	SsmSmBase&           mrReinitializedSsm;
};


#endif // _ReinitializeDependentSsmsJob_h_