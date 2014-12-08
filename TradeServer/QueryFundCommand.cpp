#include "QueryFundCommand.h"

QueryFundCommand::QueryFundCommand(CThostFtdcTraderApi *api, CThostFtdcQryTradingAccountField *accountField,
	int &requestID) :ApiCommand(requestID, api){
	this->accountField = accountField;
}

int QueryFundCommand::execute(){
	return api->ReqQryTradingAccount(accountField, requestID);
}