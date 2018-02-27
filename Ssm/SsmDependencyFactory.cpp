#include "Ssm/SsmDependencyFactory.h"   // class implemented
#include "Util/Dbc.h"                   // USE design by contract
#include "Ssm/SsmCoordinatorActor.h"    //
#include "Util/Debug.h"                 // USES debug
#include "Util/Log.h"                   // USES loggin
#include "Ssm/SsmPublic.h"				// USES


SsmDependencyFactory::SsmDependencyFactory(SsmCoordinatorActor& rCoordinator)
	:	mrCoordinator(rCoordinator)

{

}// SsmDependencyFactory


SsmDependencyBase*  
SsmDependencyFactory::Create(Properties& rDescription)
{
	REQUIRE(strcmp(rDescription.Type(), SsmPublic::DependencyDefinitionAn()) == 0);

	// Create an emtpy dependency object. We only have one kind of dependency
	// now so we can hard code its creation.
	//
	SsmDependencyBase* dep= new SsmDependencyBase(
		mrCoordinator, 
		rDescription.Name(),
		rDescription.AsString("description"));

	D1(&mrCoordinator, "SsmDependencyFactory:Create: dep=" << dep->Name());


	// Create all the start actions.
	//
	Properties* start_actions= rDescription.AsPropertiesp(SsmPublic::StartActionsAn());
	if (start_actions)
	{
		for (int i= 0; i < start_actions->Entries(); i++)
		{
			Properties* action= start_actions->AsPropertiesp(i);
			REQUIRE(action != 0);

			SsmActionBase* obj= mActionFactory.Create(*dep, *action);
			REQUIRE(obj != 0);

			dep->AddStartAction(*obj);

		}// foreach action
	}


	// Create all the conditions.
	//
	Properties* conditions= rDescription.AsPropertiesp(SsmPublic::ConditionsAn());

	SET_XCEPTION_IF(
		conditions == 0 && start_actions == 0,
		0, 
		SsmPublic::SsmErrorAv(), SsmPublic::InvalidCfgErrorAv(), "", 0, 
		"No conditions for dep=" << dep->Name());

	if (conditions)
	{
		for (int i= 0; i < conditions->Entries(); i++)
		{
			Properties* condition= conditions->AsPropertiesp(i);
			REQUIRE(condition != 0);

			SsmConditionBase* obj= mConditionFactory.Create(*dep, *condition);
			REQUIRE(obj != 0);

			dep->AddCondition(*obj);

		}// foreach condition
	}


	// Create all the actions.
	//
	Properties* do_actions= rDescription.AsPropertiesp(SsmPublic::ActionsAn());
	if (do_actions == 0 && start_actions == 0)
	{
		// Try defaults if not provided. If the start_actions were provided
		// it's not necessary to have any actions.
		//
		do_actions= mrCoordinator.mpDefaultsProps->AsPropertiesp(SsmPublic::ActionsAn());

		if (do_actions)
		{
			// Use the dependency name as the SM name by default.
			//
			Properties* first_action= do_actions->AsPropertiesp(0);
			REQUIRE(first_action);

			first_action->Value(
				SsmPublic::SmNameAn(), 
				(char*) rDescription.Name(),
				true, true);
		}
	}

	SET_XCEPTION_IF(
		do_actions == 0 && start_actions == 0,
		0, 
		SsmPublic::SsmErrorAv(), SsmPublic::InvalidCfgErrorAv(), "", 0, 
		"No actions for dep=" << dep->Name());

	if (do_actions)
	{
		for (int i= 0; i < do_actions->Entries(); i++)
		{
			Properties* action= do_actions->AsPropertiesp(i);
			REQUIRE(action != 0);

			SsmActionBase* obj= mActionFactory.Create(*dep, *action);
			REQUIRE(obj != 0);

			dep->AddAction(*obj);

		}// foreach action
	}


	// Create all the undo actions.
	//
	Properties* undo_actions= rDescription.AsPropertiesp(SsmPublic::UndoActionsAn());
	if (undo_actions == 0)
	{
		// Try defaults if undo_actions were not provided.
		//
		undo_actions= mrCoordinator.mpDefaultsProps->AsPropertiesp(
			SsmPublic::UndoActionsAn());
		if (undo_actions)
		{
			// Use the dependency name as the SM name by default.
			//
			Properties* first_action= undo_actions->AsPropertiesp(0);
			REQUIRE(first_action);

			first_action->Value(
				SsmPublic::SmNameAn(), 
				(char*) rDescription.Name(),
				true, true);
		}
	}


	SET_XCEPTION_IF(
		undo_actions == 0,
		0, 
		SsmPublic::SsmErrorAv(), SsmPublic::InvalidCfgErrorAv(), "", 0, 
		"No undoactions for dep=" << dep->Name());

	if (undo_actions)
	{
		for (int i= 0; i < undo_actions->Entries(); i++)
		{
			Properties* action= undo_actions->AsPropertiesp(i);
			REQUIRE(action != 0);

			SsmActionBase* obj= mActionFactory.Create(*dep, *action);
			REQUIRE(obj != 0);

			dep->AddUndoAction(*obj);

		}// foreach action
	}


	return dep;

}// Create
