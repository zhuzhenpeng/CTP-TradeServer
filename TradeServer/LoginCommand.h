#pragma once

#include "ApiCommand.h"

class LoginCommand:public ApiCommand{
public:
	LoginCommand(CThostFtdcTraderApi *api, CThostFtdcReqUserLoginField *loginField, int &requestID);
	int execute() override;
private:
	CThostFtdcReqUserLoginField *loginField;
};