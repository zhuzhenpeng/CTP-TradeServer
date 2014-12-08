#pragma once

#include "ApiCommand.h"

class QueryPositionCommand :public ApiCommand{
public:
	QueryPositionCommand(CThostFtdcTraderApi *api, CThostFtdcQryInvestorPositionField *accountField, int &requestID);
	int execute() override;
private:
	CThostFtdcQryInvestorPositionField *accountField;
};