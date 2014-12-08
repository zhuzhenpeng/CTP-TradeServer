#pragma once

#include "ThostFtdcUserApiStruct.h"
#include <qstring.h>

class ReportDao{
public:
	//根据成交回报更新回报表格
	void updateReportTable(CThostFtdcTradeField *report,QString &strategyId);
private:
	//如果是平仓回报，在更新完回报表格之后生成静态权益
	void generateStaticProfit(CThostFtdcTradeField *report, QString &strategyId);
	//计算静态权益
	double calculateStaticProfit_today(QString instrumentID, double openPrice, double closePrice, 
		int volume,char closeDirection);
	double calculateStaticProfit(QString instrumentID, double openPrice, double closePrice, 
		int volume,char closeDirection);
};