
#include "Ssm/SsmConditionFactory.h"    // class implemented
#include "Ssm/SmCondition.h"
#include "Util/Dbc.h"                   // USES design by contract
#include "Ssm/SsmCoordinatorActor.h"
#include "Ssm/SsmPublic.h"


SsmConditionBase*  
SsmConditionFactory::Create(SsmDependencyBase& rDependency, Properties& rCondition)
{
	if (strcmp(rCondition.Type(), SsmPublic::SmConditionAv()) == 0)
	{
		// Extract and validate the properties info.
		//
		const char* sm     = rCondition.AsString(SsmPublic::SmNameAn());
		const char* state  = rCondition.AsString(SsmPublic::StateNameAn());
		const char* resolve= rCondition.AsString(SsmPublic::ResolvePolicyAn());
		REQUIRE(sm != 0);
		REQUIRE(state != 0);

		// Create the condition from the properties info.
		//
		SmCondition* cond= new SmCondition(rDependency, sm, state, resolve);


		// Add the condition to the state machine dependenecy.
		//
		rDependency.Coordinator().AddSmToConditionAssoc(sm, *cond);


		return cond;
	}

	REQUIRE(0);

	return 0;

}// Create
