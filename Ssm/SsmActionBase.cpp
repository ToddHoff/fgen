#include "Ssm/SsmActionBase.h"   // class implemented


SsmActionBase::~SsmActionBase()
{

}// SsmActionBase


ostream&         
operator << (ostream& o, const SsmActionBase& val)
{ 
	//val.SsmActionBase::Dump(o);

	val.Dump(o);

	return o;

}// <<


ostream&        
SsmActionBase::Dump(ostream& o, DumpType /*fmt*/, int /*depth*/) const
{
   o << "SsmActionBase= " << hex << (void*) this <<endl;

   return o;

}// <<
