#include "Ssm/SsmActionFactory.h"	// class implemented
#include "Ssm/SmAction.h"			// USE action
#include "Util/Dbc.h"				// USES design by contract
#include "Ssm/SsmPublic.h"			// USES


SsmActionBase*  
SsmActionFactory::Create(SsmDependencyBase& rDependency, Properties& rDescription)
{
	if (strcmp(rDescription.Type(), SsmPublic::SendSmEventAv()) == 0)
	{
		const char* sm   = rDescription.AsString(SsmPublic::SmNameAn());
		const char* event= rDescription.AsString(SsmPublic::EventNameAn());
		REQUIRE(sm != 0);
		REQUIRE(event != 0);

		return new SmAction(rDependency, sm, event);
	}

	REQUIRE(0);

	return 0;

}// Create