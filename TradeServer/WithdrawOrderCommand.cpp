#include "WithdrawOrderCommand.h"

WithdrawOrderCommand::WithdrawOrderCommand(CThostFtdcTraderApi *api, CThostFtdcInputOrderActionField *orderField,
	int &requestID) :ApiCommand(requestID, api){
	this->orderField = orderField;
}

int WithdrawOrderCommand::execute(){
	return api->ReqOrderAction(orderField, requestID);
}