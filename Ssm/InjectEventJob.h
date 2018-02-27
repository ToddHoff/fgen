#ifndef _InjectEventJob_h_
#define _InjectEventJob_h_

#include "Com/Job.h"
#include "Ssm/SsmCoordinatorActor.h"


/**
 * Perform the InjectEvent operation in a job.
 */
class InjectEventJob : public Job
{
public:
	/**
	 * Create a new InjectEventJob.
	 *
	 * @param rCoordinator The SSM coordinator the operation is on.
	 *  MEMORY: BORROWED.
	 * @param rRequest	   The request to implement. MEMORY: GIVEN.
	 */
	InjectEventJob(
		SsmCoordinatorActor&	rCoordinator,
		AvListMsg&				rRequest);


	/**
	 * Destroy this object.
	 */
	~InjectEventJob();


	/**
	 * If the initial conditions could not all be determined then
	 * then an assertion is thrown.
	 */
	void Perform();

private:
	SsmCoordinatorActor& mrCoordinator;
	AvListMsg&			 mrRequest;
};


#endif // _InjectEventJob_h_