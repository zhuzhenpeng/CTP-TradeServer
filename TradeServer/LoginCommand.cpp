#include "LoginCommand.h"

LoginCommand::LoginCommand(CThostFtdcTraderApi *api, CThostFtdcReqUserLoginField *loginField, int &requestID)
	:ApiCommand(requestID,api){
	this->loginField = loginField;
}

int LoginCommand::execute(){
	return api->ReqUserLogin(loginField, requestID);
}