#include "ComfirmSettlementCommand.h"

ComfirmSettlementCommand::ComfirmSettlementCommand(CThostFtdcTraderApi *api,
	CThostFtdcSettlementInfoConfirmField *comfirmField, int &requestID) :ApiCommand(requestID, api){
	this->comfirmField = comfirmField;
}

int ComfirmSettlementCommand::execute(){
	return api->ReqSettlementInfoConfirm(comfirmField, requestID);
}