#pragma once

#include "ApiCommand.h"

class InsertOrderCommand :public ApiCommand{
public:
	InsertOrderCommand(CThostFtdcTraderApi *api, CThostFtdcInputOrderField *orderField, int &requestID);
	int execute() override;
private:
	CThostFtdcInputOrderField *orderField;
};