#include "InsertOrderCommand.h"

InsertOrderCommand::InsertOrderCommand(CThostFtdcTraderApi *api, CThostFtdcInputOrderField *orderField,
	int &requestID) :ApiCommand(requestID, api){
	this->orderField = orderField;
}

int InsertOrderCommand::execute(){
	return api->ReqOrderInsert(orderField, requestID);
}