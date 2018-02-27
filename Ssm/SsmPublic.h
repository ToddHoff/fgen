#ifndef _SsmPublic_h_
#define _SsmPublic_h_


/**
 * SsmPublic contains constants used in the Ssm implementation. 
 * Uses these accessors instead of embedding the strings directly in your code.
 */
class SsmPublic
{
public:
	/**
	 * Value register operation.
	 */
	static inline const char* RegisterOp()      { return  "RegisterOp"; }


	/**
	 * Value for GetState operation.
	 */
	static inline const char* GetStateOp()      { return  "/op/GetState"; }


	/**
	 * Value for GetState operation.
	 */
	static inline const char* GetStateReplyOp()      { return  "/op/GetState/reply"; }


	/**
	 * Value for InjectEvent roperation.
	 */
	static inline const char* InjectEventOp()      { return  "/op/InjectEvent"; }


	/**
	 * Value for InjectEvent reply operation.
	 */
	static inline const char* InjectEventReplyOp()      { return  "/op/InjectEvent/reply"; }


	/**
	 * Value for UpdateDependency operation.
	 */
	static inline const char* UpdateDependencyOp()      { return  "/event/UpdateDependency"; }


	/**
	 * Value for state machine name attribute.
	 */
	static inline const char* SmNameAn()      { return  "sm"; }


	/**
	 * Value for state machine event attribute.
	 */
	static inline const char* EventNameAn()      { return  "event"; }


	/**
	 * Value for state machine source ID attribute.
	 */
	static inline const char* SourceIdAn()      { return  "sid"; }


	/**
	 * Value for state machine state attribute.
	 */
	static inline const char* StateNameAn()      { return  "state"; }


	/**
	 * Value for SendSmEvent attribute value, used in dependency description.
	 */
	static inline const char* SendSmEventAv()   { return  "SendSmEvent"; }


	/**
	 * Value for SmCondition attribute value, used in dependency description.
	 */
	static inline const char* SmConditionAv()   { return  "SmCondition"; }


	/**
	 * Value for ResolvePolicy attribute, used in dependency description.
	 */
	static inline const char* ResolvePolicyAn()   { return  "ResolvePolicy"; }


	/**
	 * Value for IsInitializing attribute. 
	 */
	static inline const char* IsInitializingAn()   { return  "init"; }


	/**
	 * Value for SSM error type. 
	 */
	static inline const char* SsmErrorAv()   { return  "SSM_ERROR"; }


	/**
	 * Value for invalid request error.
	 */
	static inline const char* InvalidReqErrorAv()   { return  "INVALID_REQUEST"; }


	/**
	 * Value for invalid request error.
	 */
	static inline const char* InvalidCfgErrorAv()   { return  "INVALID_CFG"; }


	/**
	 * Attribute name for startactions.
	 */
	static inline const char* StartActionsAn()   { return  "startactions"; }


	/**
	 * Attribute name for actions.
	 */
	static inline const char* ActionsAn()   { return  "actions"; }


	/**
	 * Attribute name for undoactions.
	 */
	static inline const char* UndoActionsAn()   { return  "undoactions"; }


	/**
	 * Attribute name for conditions.
	 */
	static inline const char* ConditionsAn()   { return  "conditions"; }


	/**
	 * Attribute name for DependencyDefinition.
	 */
	static inline const char* DependencyDefinitionAn()   { return  "DependencyDefinition"; }


	/**
	 * Attribute name for order.
	 */
	static inline const char* OrderAn()   { return  "order"; }

};


#endif // SsmPublic


