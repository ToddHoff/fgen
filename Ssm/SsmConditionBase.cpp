
#include "Ssm/SsmConditionBase.h"   // class implemented
#include "Ssm/SsmDependencyBase.h"



SsmConditionBase::~SsmConditionBase()
{

}// SsmConditionBase


ostream&         
operator << (ostream& o, const SsmConditionBase& val)
{ 
	val.SsmConditionBase::Dump(o);

	return o;

}// <<


ostream&        
SsmConditionBase::Dump(ostream& o, DumpType /*fmt*/, int /*depth*/) const
{
   o << "SsmConditionBase:\n"
	 << "Name       = " << GetConditionName() << endl
	 << "Description= " << Description() << endl
	 <<endl;

   return o;

}// <<

