#include "QueryPositionCommand.h"

QueryPositionCommand::QueryPositionCommand(CThostFtdcTraderApi *api, CThostFtdcQryInvestorPositionField *accountField,
	int &requestID) :ApiCommand(requestID, api){
	this->accountField = accountField;
}

int QueryPositionCommand::execute(){
	return api->ReqQryInvestorPosition(accountField, requestID);
}