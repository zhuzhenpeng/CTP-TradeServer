#pragma once

#include "ThostFtdcTraderApi.h"
#include "InstrumentInfo.h"
#include "AccountID.h"
#include "ThostFtdcTraderApi.h"
#include <qobject.h>
#include <memory>
#include <qstring.h>
#include <map>
#include <queue>

//该交易账户主要用于连接ctp获取交易日、交易合约等信息，并不进行买卖交易
//单例模式
class BackgroundTrader :public CThostFtdcTraderSpi{
public:
	static std::shared_ptr<BackgroundTrader> getInstance();
	~BackgroundTrader();
	const QString & getTradingDate() const;
	const std::map<QString, std::shared_ptr<InstrumentInfo>> & getInstruments() const;
private:
	///当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
	void OnFrontConnected() override;

	///登录请求响应
	void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
		CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;

	///请求查询合约手续费率响应
	void OnRspQryInstrumentCommissionRate(CThostFtdcInstrumentCommissionRateField *pInstrumentCommissionRate,
		CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;

	///请求查询合约响应
	void OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument,
		CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;

private:
	BackgroundTrader();
	BackgroundTrader(const BackgroundTrader &) = delete;
	BackgroundTrader & operator=(const BackgroundTrader &) = delete;
	
	void init();
	void initInterestedInstruments(QString &rawString);
	void supplementInstrumentInfo();
	int getRequestID();
	bool readyForNext();
	void errorInstrumentID(char *id);
private:
	//单例全局变量
	static std::shared_ptr<BackgroundTrader> bgTrader ;

	std::shared_ptr<AccountID> accountID;
	CThostFtdcTraderApi *api;
	QString tradingDate;

	//存放所有策略感兴趣的合约
	std::map<QString,std::shared_ptr<InstrumentInfo>> interestedInstruments;

	bool loginFlag = false;
	int requestID = 0;

	//新合约队列以及队列锁
	std::queue<QString> newInstruments;
	bool finishBasicQuery = true;
	bool finishCommissionQuery = true;
};