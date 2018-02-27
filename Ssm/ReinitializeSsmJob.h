#ifndef _ReinitializeSsmJob_h_
#define _ReinitializeSsmJob_h_

#include "Com/Job.h"
#include "Ssm/SsmCoordinatorActor.h"


/**
 * Perform the ReinitializeSsm operation in a job.
 */
class ReinitializeSsmJob : public Job
{
public:
	/**
	 * Create a new ReinitializeSsmJob.
	 *
	 * @param rCoordinator The SSM coordinator the operation is on.
	 *  MEMORY: BORROWED.
	 * @param rRequest	   The request to implement. MEMORY: GIVEN.
	 */
	ReinitializeSsmJob(
		SsmCoordinatorActor&	rCoordinator,
		AvListMsg&				rRequest);


	/**
	 * Destroy this object.
	 */
	~ReinitializeSsmJob();


	/**
	 * If the initial conditions could not all be determined then
	 * then an assertion is thrown.
	 */
	void Perform();

private:
	SsmCoordinatorActor& mrCoordinator;
	AvListMsg&			 mrRequest;
};


#endif // _ReinitializeSsmJob_h_