#include "Ssm/SmAction.h"              // class implemented
#include "Util/Dbc.h"                  // USES design by contract
#include "Com/AvListMsg.h"             // USES av message
#include "Util/Debug.h"                // USES debug
#include "Ssm/SsmNameFactory.h"        // USES names
#include "Com/ComService.h"            // USES com
#include "Ssm/SsmCoordinatorActor.h"   // USES coordinator
#include "Ssm/SsmPublic.h"				// USES


SmAction::SmAction(SsmDependencyBase& rDependency, const char* pSmName, const char* pEvent) 
	:	mrDependency(rDependency)
{
	REQUIRE(pSmName != 0);
	REQUIRE(pEvent != 0);

	mSmName                = pSmName;
	mEvent                 = pEvent;
	SsmNameFactory::CalcSmObjName(mNameSpaceObjName, pSmName);
 
    mpInvokeProps = ComService::Singleton()->InvokePropsFactory();
    mpInvokeProps->Reliability(RELIABLE);

}// SmAction



SmAction::~SmAction()
{ 
    delete mpInvokeProps;

}// SmAction


ostream&         
operator << (ostream& o, const SmAction& val)
{ 
	val.SmAction::Dump(o);

	return o;

}// <<


ostream&        
SmAction::Dump(ostream& o, DumpType /*fmt*/, int /*depth*/) const
{
	o << "SmAction=" << "\n"
	 << "   this       =" << hex << (void*) this <<"\n"
	 << "   SmName     =" << mSmName             <<"\n"
	 << "   Event      =" << mEvent              <<"\n"
	 << endl;

   return o;

}// <<


void 
SmAction::Doit(void)
{
	D1(&Dependency().Coordinator(), "Doit sm=" << mSmName << " event=" << mEvent);


	// Create the event request.
	//
	AvListMsg* req= new AvListMsg(1, 0, SsmPublic::InjectEventOp());
	req->Value(SsmPublic::EventNameAn(), (char*) mEvent.data(), true, true);
	req->Value(SsmPublic::SmNameAn(), (char*) mSmName.data(), true, true);
	req->Value(SsmPublic::SourceIdAn(), Dependency().Coordinator().GetSourceId());
	req->Value("FromDep", (char*) Dependency().Name(), true, true);


  	// Tell the state machine about its new event. We don't care about
  	// a return value. Nothing to do on a failure. We can't block
	// waiting for a reply or we'll deadlock if the invoke is on the
	// same coordinator as that services the operation.
   	//
    ComService::Singleton()->Invoke(
  		0,
  		mNameSpaceObjName.data(),
		SsmPublic::InjectEventOp(),
 		req,
        Com::INVOKE_TIMEOUT_MSECS,
        mpInvokeProps);

  	LnException::Clear();  


}// Doit


SsmDependencyBase& 
SmAction::Dependency(void) 
{ return mrDependency; }
