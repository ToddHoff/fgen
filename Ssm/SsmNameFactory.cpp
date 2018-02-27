#include "Ssm/SsmNameFactory.h"  // class implemented
#include "Util/Dbc.h"            // USES design by contract


RWCString&
SsmNameFactory::CalcSmObjName(RWCString& objName, const char* pSsmName)
{
	REQUIRE(pSsmName != 0);

	objName.append("/ssm");
	objName.append("/");
	objName.append(pSsmName);

	return objName;

}// CalcObjNameSpace

