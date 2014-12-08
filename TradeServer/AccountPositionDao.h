#pragma once

#include "ThostFtdcUserApiStruct.h"

class AccountPositionDao{
public:
	//更新账户持仓情况
	void updatePosition(CThostFtdcInvestorPositionField *positionInfo);
};