#include "Ssm/SsmSmBase.h"   // class implemented
#include "Util/LnMemory.h"   // USES strcmp



bool 
SsmSmBase::IsClass(const char* name) const
{
	// Is it any of the classes we are?
	//
	if (strcmp(name, ClassName()) == 0
		|| strcmp(name, "SsmSmBase") == 0
		|| strcmp(name, "Typeable") == 0)
		return true;


	// It's not
	//
	return false;

}// IsClass



bool               
SsmSmBase::IsSsmState(const char* pCompareTo) const
{
	return (strcmp(pCompareTo, SsmCurrentState()) == 0)
		? true
		: false;

}// IsSsmState
