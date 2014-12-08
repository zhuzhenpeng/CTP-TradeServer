#pragma once

#include <qstring.h>
#include "ThostFtdcUserApiStruct.h"

class FundDao{
public:
	FundDao() = default;
	//记录每天资金变化
	void logFund(CThostFtdcTradingAccountField *fund);
	//记录最新的资金情况
	void updateFund(CThostFtdcTradingAccountField *fund);
	//更新策略的可用资金
	void updateStrategyFund(QString investorId, QString strategyId, double money);
};