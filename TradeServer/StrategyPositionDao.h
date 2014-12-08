#pragma once

#include "ThostFtdcUserApiStruct.h"
#include <qstring.h>

class StrategyPositionDao{
public:
	//根据成交回报更新策略持仓情况
	void updatePosition(CThostFtdcTradeField *pTrade, QString strategyId);

	//根据账户策略关系表同步策略持仓表
	//1.账户有了新的策略之后建立记录
	//2.账户与策略解除关系后删除记录
	static void synStrategyPosition();

	//程序在新的交易日运行时，刷新策略持仓表中的时间记录，把今日持仓清零
	static void refreshDaily();

	//根据账号、策略和合约返回该合约的今日持仓数量
	int getTodayPosition(const QString &investorId, const QString &strategyId, const QString &instrumentId, char direction);
	
private:
	bool isSHFE(QString instrumentID);
};