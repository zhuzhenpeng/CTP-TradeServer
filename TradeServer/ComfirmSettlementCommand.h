#pragma once

#include "ApiCommand.h"

class ComfirmSettlementCommand :public ApiCommand{
public:
	ComfirmSettlementCommand(CThostFtdcTraderApi *api, CThostFtdcSettlementInfoConfirmField *comfirmField, int &requestID);
	int execute() override;
private:
	CThostFtdcSettlementInfoConfirmField *comfirmField;
};