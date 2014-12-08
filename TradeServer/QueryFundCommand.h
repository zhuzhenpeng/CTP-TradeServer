#pragma once

#include "ApiCommand.h"

class QueryFundCommand :public ApiCommand{
public:
	QueryFundCommand(CThostFtdcTraderApi *api, CThostFtdcQryTradingAccountField *accountField, int &requestID);
	int execute() override;
private:
	CThostFtdcQryTradingAccountField *accountField;
};