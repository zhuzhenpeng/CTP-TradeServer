#pragma once

#include "ThostFtdcTraderApi.h"
#include "AccountID.h"
#include "AccountFund.h"
#include <memory>
#include <map>
#include <qtimer.h>
#include <qstring.h>
#include "Order.h"
#include "OrderDao.h"
#include "CommandQueue.h"
#include "AccountPositionDao.h"
#include "FundDao.h"
#include "StrategyPositionDao.h"
#include "ReportDao.h"
#include <qbytearray.h>
#include <qobject.h>

class Trader :public QObject,public CThostFtdcTraderSpi{
	Q_OBJECT
public:
	Trader(std::shared_ptr<AccountID> id);

	bool isTradable(){ return tradable; };

	//通过交易指令生成报单并执行
	void generateAndExecuteOrder(QByteArray *instruction, int volume, double price);

	//撤单
	void cancleOrder(Order *order);
private:
	/****************************Api交易函数****************************************/
	//登录
	void login();
	//确认结算
	void comfirmSettlement();

	//查询
	void queryFund();										//查询资金
	void queryPosition(QString instrument = "");			//查询持仓是一支合约的总体情况
	/****************************Api交易函数****************************************/

	/****************************Spi回调函数****************************************/
	///当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
	void OnFrontConnected() override;

	///登录请求响应
	void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
		CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;

	//投资者结算结果确认响应
	void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm,
		CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;

	///报单录入请求响应(参数不通过)
	void OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder,
		CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;

	///撤单操作请求响应(参数不通过)
	void OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction,
		CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;

	///报单通知
	void OnRtnOrder(CThostFtdcOrderField *pOrder) override;

	///成交通知
	void OnRtnTrade(CThostFtdcTradeField *pTrade) override;

	///请求查询投资者持仓响应
	void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition,
		CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;

	///请求查询资金账户响应
	void OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount,
		CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
	/****************************Spi回调函数****************************************/

	/****************************辅助函数****************************************/
	//初始化活跃报单(本地)
	void initOrders();

	//初始化报单过滤器(本地)
	void initOrderFilter();

	//初始化最大报单引用(本地)
	void initOrderRef();

	//初始化成交回报过滤器(本地)
	void initReportFilter();

	//与交易所建立连接，进入准备交易的状态(非本地)
	void readyToTrade();

	//报单引用自增
	void increaseRef(){ maxOrderRef++; }

	//对上海期货的平仓报单尝试进行平今操作
	void splitSHFEOrder(std::shared_ptr<Order> &order, QString limit_flag);

	//解析报单并发出成交
	void executeOrder(std::shared_ptr<Order> order, QString limit_flag);

	//根据建仓指令计算建仓成本
	double calculateOpenCost(const QString &instructionId);

	//根据平仓指令计算平仓收回资金
	double calculateCloseRegain(const QString &instructionId);

	//报单交易结束时，清理map
	void cleanOrderMap(const QString &instructionId);

	//从撤单中生成新的报单
	void generateOrderFromCanceledOrder(const std::shared_ptr<Order> &order);

	//计算某报单的回报
	void calculateOrder(std::shared_ptr<Order> order);
	/****************************辅助函数****************************************/
private slots:
	//查询更新资金情况
	void updateFund();
private:
	//账户信息
	std::shared_ptr<AccountID> id;
	//资金情况
	std::shared_ptr<AccountFund> fund;

	//本地的最大报单引用
	int maxOrderRef;

	//未处理的报单集合(每个报单由orderRef唯一确认)
	std::map<int, std::shared_ptr<Order>> orders;
	//报单DAO
	OrderDao orderDao;
	//报单引用过滤器
	std::set<int> orderFilter;
	//用户-策略持仓DAO
	StrategyPositionDao strategyPositionDao;
	//成交回报过滤器
	std::set<QString> reportFilter;
	//成交回报DAO
	ReportDao reportDao;

	//交易api
	CThostFtdcTraderApi *api;
	//请求编号
	int requestID = 0;
	//api请求队列
	CommandQueue commandQueue;

	//是否可以进行交易
	bool tradable = false;

	//用户持仓DAO
	AccountPositionDao accountPositionDao;
	//用户资金DAO
	FundDao fundDao;
	//查询资金的定时器
	QTimer *queryFundTimer;

	//待计算报单的引用集合
	std::set<int> calculateOrders;
};