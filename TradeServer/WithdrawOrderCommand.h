#pragma once

#include "ApiCommand.h"

class WithdrawOrderCommand :public ApiCommand{
public:
	WithdrawOrderCommand(CThostFtdcTraderApi *api, CThostFtdcInputOrderActionField *orderField, int &requestID);
	int execute() override;
private:
	CThostFtdcInputOrderActionField *orderField;
};