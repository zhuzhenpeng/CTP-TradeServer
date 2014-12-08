#include "Trader.h"
#include "LoginCommand.h"
#include "ComfirmSettlementCommand.h"
#include "InsertOrderCommand.h"
#include "WithdrawOrderCommand.h"
#include "QueryFundCommand.h"
#include "QueryPositionCommand.h"
#include "BackgroundTrader.h"
#include "GVAR.h"
#include <qsqlquery.h>
#include <qdir.h>
#include <qvariant.h>
#include <qcoreapplication.h>
#include <set>
#include <vector>
#include <qdebug.h>
//#include <qsqlerror.h>

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

using std::shared_ptr;
using std::make_shared;
using std::set;
using std::make_pair;
using std::vector;

Trader::Trader(shared_ptr<AccountID> id){
	this->id = id;
	qDebug() << "正在初始化账户" << id->getInvestorID() << "信息";
	initOrders();
	initOrderRef();
	initOrderFilter();
	initReportFilter();
	readyToTrade();
	//每5分钟查询一次资金
	queryFundTimer = new QTimer(this);
	queryFundTimer->setSingleShot(false);
	connect(queryFundTimer, SIGNAL(timeout()), this, SLOT(updateFund()));
	queryFundTimer->start(5 * 60 * 1000);
}

//通过交易指令生成报单并执行
void Trader::generateAndExecuteOrder(QByteArray *instruction, int volume, double price){
	auto order = make_shared<Order>();
	//解析字段
	QString instructionId = instruction->mid(0, 15).trimmed();
	QString investorId = instruction->mid(15, 16).trimmed();
	QString strategyId = instruction->mid(31, 10).trimmed();
	QString instrumentId = instruction->mid(41, 6).trimmed();
	QString direction = instruction->mid(47, 1);
	QString open_close_flag = instruction->mid(48, 1);
	QString limit_flag = instruction->mid(49, 1);
	order->setInvestorId(investorId);
	order->setInstructionId(instructionId);
	order->setStrategyId(strategyId);
	order->setInstrumentId(instrumentId);
	order->setOriginalVolume(volume);
	order->setTradedVolume(0);
	order->setRestVolume(volume);
	order->setPrice(price);
	order->setDirection(direction.at(0).toLatin1());
	order->setOpenCloseFlag(open_close_flag.at(0).toLatin1());
	order->setSequenceId(-1);
	order->setOrderStatus('a');
	//if (open_close_flag == "1"){
	//	//当收到平仓字段时对合约进行判断，如果是上海期货的合约则自动平今
	//	auto info = BackgroundTrader::getInstance()->getInstruments();
	//	if (info[instrumentId]->getExchangeId() == "SHFE"){
	//		delete instruction;
	//		qDebug() << "进行平今";
	//		splitSHFEOrder(order, limit_flag);
	//		return;
	//	}
	//}
	//执行指令
	executeOrder(order, limit_flag);
	delete instruction;
}

//撤单
void Trader::cancleOrder(Order *order){
	//获得交易所代码
	auto instruments = BackgroundTrader::getInstance()->getInstruments();
	auto &info = instruments[order->getInstrumentId()];
	const QString &exchangeID = info->getExchangeId();
	//设置撤单信息
	CThostFtdcInputOrderActionField *orderField = new CThostFtdcInputOrderActionField();
	strcpy(orderField->BrokerID, id->getBrokerID().toStdString().c_str());
	strcpy(orderField->InvestorID, id->getInvestorID().toStdString().c_str());
	//strcpy(orderField->ExchangeID, exchangeID.toStdString().c_str());
	strcpy(orderField->ExchangeID, "SHFE");	//测试环境下全都是上期所
	strcpy(orderField->OrderSysID, order->getSystemId().toStdString().c_str());
	itoa(order->getOrderRef(), orderField->OrderRef, 10);
	orderField->ActionFlag = THOST_FTDC_AF_Delete;	//删除报单 '0'
	shared_ptr<ApiCommand> command = make_shared<WithdrawOrderCommand>(api, orderField, requestID);
	commandQueue.addCommand(command);
}

/****************************Api交易函数****************************************/
//登录
void Trader::login(){
	CThostFtdcReqUserLoginField *loginField = new CThostFtdcReqUserLoginField();
	strcpy(loginField->BrokerID, id->getBrokerID().toStdString().c_str());
	strcpy(loginField->UserID, id->getInvestorID().toStdString().c_str());
	strcpy(loginField->Password, id->getPassword().toStdString().c_str());
	//把指令放到队列尾部,下面各条指令的执行方法类似
	shared_ptr<ApiCommand> command = make_shared<LoginCommand>(api, loginField, requestID);
	commandQueue.addCommand(command);
}

//确认计算
void Trader::comfirmSettlement(){
	CThostFtdcSettlementInfoConfirmField *comfirmField = new CThostFtdcSettlementInfoConfirmField();
	strcpy(comfirmField->BrokerID, id->getBrokerID().toStdString().c_str());
	strcpy(comfirmField->InvestorID, id->getInvestorID().toStdString().c_str());
	shared_ptr<ApiCommand> command = make_shared<ComfirmSettlementCommand>(api, comfirmField, requestID);
	commandQueue.addCommand(command);
}

//查询资金
void Trader::queryFund() {
	CThostFtdcQryTradingAccountField *accountField = new CThostFtdcQryTradingAccountField();
	strcpy(accountField->BrokerID, id->getBrokerID().toStdString().c_str());
	strcpy(accountField->InvestorID, id->getInvestorID().toStdString().c_str());
	shared_ptr<ApiCommand> command = make_shared<QueryFundCommand>(api, accountField, requestID);
	commandQueue.addCommand(command);
}

//查询客户总体持仓情况
void Trader::queryPosition(QString instrument) {
	CThostFtdcQryInvestorPositionField *accountField = new CThostFtdcQryInvestorPositionField();
	strcpy(accountField->BrokerID, id->getBrokerID().toStdString().c_str());
	strcpy(accountField->InvestorID, id->getInvestorID().toStdString().c_str());
	strcpy(accountField->InstrumentID, instrument.toStdString().c_str());
	shared_ptr<ApiCommand> command = make_shared<QueryPositionCommand>(api, accountField, requestID);
	commandQueue.addCommand(command);
}
/****************************Api交易函数****************************************/

/****************************Spi回调函数****************************************/
///当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
void Trader::OnFrontConnected(){
	login();
}

///登录请求响应
void Trader::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	if (pRspInfo == nullptr || pRspInfo->ErrorID == 0){
		comfirmSettlement();	//确认结算结果
	}
	else
	{
		qDebug() << pRspUserLogin->UserID << "登录失败, 错误信息:" << pRspInfo->ErrorID << QString::fromLocal8Bit(pRspInfo->ErrorMsg);
	}
}

//投资者结算结果确认响应
void Trader::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	if (pRspInfo == nullptr || pRspInfo->ErrorID == 0){
		tradable = true;
		qDebug() << "账户:" << pSettlementInfoConfirm->InvestorID << "已经可以交易";
	}
	else
	{
		qDebug() << "账户:" << pSettlementInfoConfirm->InvestorID << "确认结算失败";
	}
}

///报单录入请求响应(参数不通过)
void Trader::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	qDebug() << "报单录入请求响应(参数不通过)";
	qDebug() << "错误代码:" << pRspInfo->ErrorID << "错误信息:" << QString::fromLocal8Bit(pRspInfo->ErrorMsg);
}

///撤单操作请求响应(参数不通过)
void Trader::OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	qDebug() << "撤单操作请求响应(参数不通过)";
	qDebug() << "错误代码:" << pRspInfo->ErrorID << "错误信息:" << QString::fromLocal8Bit(pRspInfo->ErrorMsg);
}

///报单通知
void Trader::OnRtnOrder(CThostFtdcOrderField *pOrder){
	qDebug() << "报单回报";
	qDebug()
		<< "交易所编号:" << pOrder->ExchangeID
		<< "合约代码:" << pOrder->InstrumentID
		<< "报单引用:" << pOrder->OrderRef
		<< "买卖方向:" << pOrder->Direction
		<< "组合开平标志:" << pOrder->CombOffsetFlag
		<< "价格:" << pOrder->LimitPrice
		<< "数量:" << pOrder->VolumeTotalOriginal
		<< "今成交数量:" << pOrder->VolumeTraded
		<< "剩余数量:" << pOrder->VolumeTotal
		<< "报单编号（判断报单是否有效）:" << pOrder->OrderSysID
		<< "报单状态:" << pOrder->OrderStatus
		<< "报单日期:" << pOrder->InsertDate
		<< "序号:" << pOrder->SequenceNo;
	int orderRef = atoi(pOrder->OrderRef);
	if (orderFilter.find(orderRef) == orderFilter.end()){	//报单过滤
		auto &order = orders[orderRef];
		order->update(pOrder);
		if (order->getUpdateFlag()){		//判断报单是否进行了更新，因为update中有顺序编号过滤
			switch (order->getOrderStatus()){
				//报单交易完成、报单错误都要准备计算费用，等成交回报后就进行结算
			case 'f':
			case 'w':
				calculateOrders.insert(order->getOrderRef());
				orderDao.updateOrderTable(order);
				break;
				//撤单
			case 'c':
				generateOrderFromCanceledOrder(order);
				orderDao.updateOrderTable(order);
				break;
			default:
				break;
			}
		}
		order->recoverUpdateFlag();
	}
}

///成交回报
void Trader::OnRtnTrade(CThostFtdcTradeField *pTrade){
	qDebug() << "成交回报";
	QString tradedID(pTrade->TradeID);
	//过滤
	if (reportFilter.find(tradedID) == reportFilter.end()){
		auto order = orders[atoi(pTrade->OrderRef)];
		QString strategyId = order->getStrategyId();
		//根据开、平放入不同的成交回报表中
		reportDao.updateReportTable(pTrade, strategyId);
		//更新策略持仓情况
		strategyPositionDao.updatePosition(pTrade, strategyId);
		//更新过滤器
		reportFilter.insert(tradedID);
		//更新用户持仓情况
		queryPosition();
		//更新资金
		queryFund();
		//查看该回报是否是某个报单的最后回报(因为成交回报总在报单回报之后，因此要计算费用必须等成交回报)
		if (calculateOrders.find(atoi(pTrade->OrderRef)) != calculateOrders.end()){
			auto order = orders[atoi(pTrade->OrderRef)];
			calculateOrder(order);
		}
	}
}

///请求查询投资者持仓响应
void Trader::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	qDebug() << "请求查询投资者" << id->getInvestorID() << "持仓响应";
	if (pRspInfo == nullptr || pRspInfo->ErrorID == 0){
		accountPositionDao.updatePosition(pInvestorPosition);
	}
}

///请求查询资金账户响应
void Trader::OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	qDebug() << "请求查询资金账户" << id->getInvestorID() << "响应";
	if (pRspInfo == nullptr || pRspInfo->ErrorID == 0){
		fundDao.logFund(pTradingAccount);
		fundDao.updateFund(pTradingAccount);
	}
}
/****************************Spi回调函数****************************************/

/****************************辅助函数******************************************/
//查询更新资金情况
void Trader::updateFund(){
	queryFund();
}

//初始化活跃报单(本地)
void Trader::initOrders(){
	qDebug() << "正在初始化账户活跃报单信息...";
	const QString &investorId = id->getInvestorID();
	const QString &date = BackgroundTrader::getInstance()->getTradingDate();
	shared_ptr< set<shared_ptr<Order>> > results = orderDao.getActivedOrders(investorId, date);
	for (auto order : (*results)){
		int orderRef = order->getOrderRef();
		orders.insert(make_pair(orderRef, order));
	}
}

//初始化报单过滤器(本地)
void Trader::initOrderFilter(){
	qDebug() << "正在生成账户报单过滤器...";
	const QString &investorId = id->getInvestorID();
	const QString &date = BackgroundTrader::getInstance()->getTradingDate();
	orderDao.initOrderFilter(investorId, date, orderFilter);
}

//初始化最大报单引用(本地)
void Trader::initOrderRef(){
	qDebug() << "正在读取账户最大报单引用信息...";
	const QString &investorId = id->getInvestorID();
	const QString &date = BackgroundTrader::getInstance()->getTradingDate();
	maxOrderRef = orderDao.getMaximumOrderRef(investorId, date);
	++maxOrderRef;		//总比数据库中的多一
}

//初始化成交回报过滤器(本地)
void Trader::initReportFilter(){
	qDebug() << "正在生成账户成交回报过滤器...";
	const QString &investorId = id->getInvestorID();
	const QString &date = BackgroundTrader::getInstance()->getTradingDate();
	//查询平仓成交回报
	QSqlQuery queryClose(DATABASE);
	queryClose.prepare(" select trade_id from close_traded_report where investor_id=:id and trade_date=:date ");
	queryClose.bindValue(":id", investorId);
	queryClose.bindValue(":date", date);
	queryClose.exec();
	while (queryClose.next()){
		QString &trade_id = queryClose.value(0).toString();
		reportFilter.insert(trade_id);
	}
	//查询开仓成交回报
	QSqlQuery queryOpen(DATABASE);
	queryOpen.prepare(" select * from open_traded_report where investor_id=:id and trade_date=:date ");
	queryOpen.bindValue(":id", investorId);
	queryOpen.bindValue(":date", date);
	queryOpen.exec();
	while (queryOpen.next()){
		QString &trade_id = queryOpen.value("trade_id").toString();
		reportFilter.insert(trade_id);
	}
}

//与交易所建立连接，进入准备交易的状态(非本地)
void Trader::readyToTrade(){
	qDebug() << "账户即将登陆...";
	//把ctp的通讯文件放在不同的各个客户的文件夹中
	QString dirName = "user/" + id->getInvestorID() + "/con_Files";
	QDir conFileDir(dirName);
	if (!conFileDir.exists()){
		conFileDir.mkpath(".");
	}
	api = CThostFtdcTraderApi::CreateFtdcTraderApi((dirName + "/").toStdString().c_str());
	api->RegisterSpi(this);
	//订阅共有流、私有流
	api->SubscribePublicTopic(THOST_TERT_RESTART);
	api->SubscribePrivateTopic(THOST_TERT_RESTART);
	//注册前置机
	char *frontAddress = new char[100];
	strcpy(frontAddress, id->getFrontAddress().toStdString().c_str());
	api->RegisterFront(frontAddress);
	//开启请求队列
	commandQueue.start();
	api->Init();
}

//对上海期货的平仓报单尝试进行平今操作
void Trader::splitSHFEOrder(std::shared_ptr<Order> &order, QString limit_flag){
	const QString &investorId = order->getInvestorId();
	const QString &strategyId = order->getStrategyId();
	const QString &instrumentId = order->getInstructionId();
	char direction = order->getDirection();
	int todayPosition = strategyPositionDao.getTodayPosition(investorId, strategyId, instrumentId, direction);
	//如果需要平仓的数量小于今日持仓数量，则直接修改原有报单
	if (order->getOriginalVolume() <= todayPosition){
		order->setOpenCloseFlag('3');
		executeOrder(order, limit_flag);
	}
	//否则把进仓全部平掉再平旧仓
	else{
		int oldPosition = order->getOriginalVolume() - todayPosition;
		//平进仓
		order->setOriginalVolume(todayPosition);
		order->setOpenCloseFlag('3');
		executeOrder(order, limit_flag);
		//平旧仓
		auto oldOrder = make_shared<Order>();
		oldOrder->setInstructionId(order->getInstructionId());
		oldOrder->setInvestorId(order->getInvestorId());
		oldOrder->setStrategyId(order->getStrategyId());
		oldOrder->setInstrumentId(order->getInstrumentId());
		oldOrder->setOriginalVolume(oldPosition);
		oldOrder->setPrice(order->getPrice());
		oldOrder->setDirection(order->getDirection());
		oldOrder->setOpenCloseFlag('4');
		executeOrder(oldOrder, limit_flag);
	}
}

//解析报单并发出交易信号
void Trader::executeOrder(std::shared_ptr<Order> order, QString limit_flag){
	CThostFtdcInputOrderField *orderField = new CThostFtdcInputOrderField();
	strcpy(orderField->BrokerID, id->getBrokerID().toStdString().c_str());
	strcpy(orderField->InvestorID, id->getInvestorID().toStdString().c_str());
	strcpy(orderField->InstrumentID, order->getInstrumentId().toStdString().c_str());
	itoa(maxOrderRef, orderField->OrderRef, 10);
	order->setOrderRef(maxOrderRef);
	orders.insert(make_pair(maxOrderRef, order));			//把报单插入到map中
	orderDao.updateOrderTable(order);						//把报单存入数据库中
	increaseRef();	//自增orderRef
	if (limit_flag == "1"){
		orderField->OrderPriceType = THOST_FTDC_OPT_AnyPrice;		//市价
		orderField->LimitPrice = 0;									//价格
	}
	else{
		orderField->OrderPriceType = THOST_FTDC_OPT_LimitPrice;		//限价
		orderField->LimitPrice = order->getPrice();
	}
	if (order->getDirection() == 'b'){
		orderField->Direction = THOST_FTDC_D_Buy;					//买 
	}
	else{
		orderField->Direction = THOST_FTDC_D_Sell;					//卖
	}
	if (order->getOpenCloseFlag() == '0'){
		orderField->CombOffsetFlag[0] = THOST_FTDC_OF_Open;				//开仓
	}
	else{
		orderField->CombOffsetFlag[0] = THOST_FTDC_OF_CloseToday;		//测试环境都用平今
	}
	//if (order->getOpenCloseFlag() == '1'){
	//	orderField->CombOffsetFlag[0] = THOST_FTDC_OF_Close;				//平仓
	//}
	//if (order->getOpenCloseFlag() == '3'){
	//	orderField->CombOffsetFlag[0] = THOST_FTDC_OF_CloseToday;		//平今
	//}
	//if (order->getOpenCloseFlag() == '4'){
	//	orderField->CombOffsetFlag[0] = THOST_FTDC_OF_CloseYesterday;	//平昨
	//}
	orderField->VolumeTotalOriginal = order->getOriginalVolume();		//数量
	//以下是固定的字段
	orderField->CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;		//投机 
	orderField->TimeCondition = THOST_FTDC_TC_GFD;				//当日有效 '3'
	orderField->VolumeCondition = THOST_FTDC_VC_AV;				//任何数量 '1'
	orderField->MinVolume = 1;
	orderField->ContingentCondition = THOST_FTDC_CC_Immediately;	//立即触发'1'
	orderField->ForceCloseReason = THOST_FTDC_FCC_NotForceClose;	//非强平 '0'
	orderField->IsAutoSuspend = 0;
	orderField->UserForceClose = 0;
	shared_ptr<ApiCommand> command = make_shared<InsertOrderCommand>(api, orderField, requestID);
	commandQueue.addCommand(command);
}

//根据建仓指令计算建仓成本
double Trader::calculateOpenCost(const QString &instructionId){
	auto &tradingDate = BackgroundTrader::getInstance()->getTradingDate();
	auto instruments = BackgroundTrader::getInstance()->getInstruments();
	//获得合约
	QString instrumentId;
	QSqlQuery getInstrumentId;
	getInstrumentId.prepare("select instrument_id from orders where instruction_id=:instruction_id and order_date=:order_date limit 1");
	getInstrumentId.bindValue(":instruction_id", instructionId);
	getInstrumentId.bindValue(":order_date", tradingDate);
	getInstrumentId.exec();
	while (getInstrumentId.next()){
		instrumentId = getInstrumentId.value(0).toString();
	}
	auto instrument = instruments[instrumentId];
	//获得system_id
	QSqlQuery getSysId;
	getSysId.prepare("select system_id from orders where instruction_id=:instruction_id and order_date=:order_date ");
	getSysId.bindValue(":instruction_id", instructionId);
	getSysId.bindValue(":order_date", tradingDate);
	getSysId.exec();
	vector<QString> sysIds;
	while (getSysId.next()){
		sysIds.push_back(getSysId.value(0).toString());
	}
	if (sysIds.empty()){
		//如果集合为空，说明该指令执行失败，没有执行建仓，因此成本为0
		return 0;
	}
	//从成交回报表中获得每次开仓的数量、价格，然后计算成本
	double cost = 0;
	double marginRate = instrument->getMarginRate();
	double openCommission = instrument->getOpenCommission();
	int multiplier = instrument->getMultiplier();
	for (auto &sysid : sysIds){
		QSqlQuery getVolumeAndPrice;
		getVolumeAndPrice.prepare("select volume,open_price from open_traded_report where system_id=:system_id and "
			" trade_date=:trade_date ");
		getVolumeAndPrice.bindValue(":system_id", sysid);
		getVolumeAndPrice.bindValue(":trade_date", tradingDate);
		getVolumeAndPrice.exec();
		while (getVolumeAndPrice.next()){
			int volume = getVolumeAndPrice.value("volume").toInt();
			double openPrice = getVolumeAndPrice.value("open_price").toDouble();
			if (openCommission < 1){
				cost += openPrice*marginRate*multiplier*volume + openPrice*multiplier*volume*openCommission;
			}
			else{
				cost += openPrice*marginRate*multiplier*volume + volume*openCommission;
			}
		}
	}
	return cost;
}

//根据平仓指令计算平仓收回资金
double Trader::calculateCloseRegain(const QString &instructionId){
	auto &tradingDate = BackgroundTrader::getInstance()->getTradingDate();
	auto instruments = BackgroundTrader::getInstance()->getInstruments();
	//获得合约
	QString instrumentId;
	QSqlQuery getInstrumentId;
	getInstrumentId.prepare("select instrument_id from orders where instruction_id=:instruction_id and order_date=:order_date limit 1");
	getInstrumentId.bindValue(":instruction_id", instructionId);
	getInstrumentId.bindValue(":order_date", tradingDate);
	getInstrumentId.exec();
	while (getInstrumentId.next()){
		instrumentId = getInstrumentId.value(0).toString();
	}
	auto instrument = instruments[instrumentId];
	//获得system_id
	QSqlQuery getSysId;
	getSysId.prepare("select system_id from orders where instruction_id=:instruction_id and order_date=:order_date ");
	getSysId.bindValue(":instruction_id", instructionId);
	getSysId.bindValue(":order_date", tradingDate);
	getSysId.exec();
	vector<QString> sysIds;
	while (getSysId.next()){
		sysIds.push_back(getSysId.value(0).toString());
	}
	if (sysIds.empty()){
		//如果集合为空，说明该指令执行失败，没有执行平仓，因此收回为0
		return 0;
	}
	//从成交回报表中获得每次平仓的数量、价格，然后收回
	double regain = 0;
	double marginRate = instrument->getMarginRate();
	double openCommission = instrument->getOpenCommission();
	int multiplier = instrument->getMultiplier();
	for (auto &sysid : sysIds){
		QSqlQuery getVolumeAndPrice;
		getVolumeAndPrice.prepare("select volume,close_price,today_flag from close_traded_report where system_id=:system_id and "
			" trade_date=:trade_date ");
		getVolumeAndPrice.bindValue(":system_id", sysid);
		getVolumeAndPrice.bindValue(":trade_date", tradingDate);
		getVolumeAndPrice.exec();
		while (getVolumeAndPrice.next()){
			int volume = getVolumeAndPrice.value("volume").toInt();
			double closePrice = getVolumeAndPrice.value("close_price").toDouble();
			char todayFlag = getVolumeAndPrice.value("today_flag").toString().at(0).toLatin1();
			//如果不是平进仓
			if (todayFlag == 'y'){
				if (openCommission < 1){
					regain += closePrice*marginRate*multiplier*volume - closePrice*multiplier*volume*openCommission;
				}
				else{
					regain += closePrice*marginRate*multiplier*volume - volume*openCommission;
				}
			}
			else{
				//否则就是平今仓,免手续费
				if (openCommission < 1){
					regain += closePrice*marginRate*multiplier*volume;
				}
				else{
					regain += closePrice*marginRate*multiplier*volume;
				}
			}
		}
	}
	return regain;
}

void Trader::cleanOrderMap(const QString &instructionId){
	auto &tradingDate = BackgroundTrader::getInstance()->getTradingDate();
	QSqlQuery getOrderRefs;
	getOrderRefs.prepare(" select order_ref from orders where investor_id = :investor_id and order_date = :order_date "
		" and instruciton_id = :instruction_id ");
	getOrderRefs.bindValue(":investor_id", id->getInvestorID());
	getOrderRefs.bindValue(":order_date", tradingDate);
	getOrderRefs.bindValue(":instruction_id", instructionId);
	getOrderRefs.exec();
	vector<int> refs;
	while (getOrderRefs.next()){
		refs.push_back(getOrderRefs.value(0).toInt());
	}
	for (auto &ref : refs){
		orders.erase(ref);
		orderFilter.insert(ref);
	}
}

//从撤单中生成新的报单
void Trader::generateOrderFromCanceledOrder(const shared_ptr<Order> &order){
	shared_ptr<Order> newOrder = make_shared<Order>();
	newOrder->moveToThread(QCoreApplication::instance()->thread());
	newOrder->setInvestorId(order->getInvestorId());
	newOrder->setStrategyId(order->getStrategyId());
	newOrder->setInstructionId(order->getInstructionId());
	newOrder->setInstrumentId(order->getInstrumentId());
	newOrder->setDirection(order->getDirection());
	newOrder->setOpenCloseFlag(order->getOpenCloseFlag());
	double price = order->getPrice();
	//获得该合约的最小价格变动单位，如果买则买价增加，如果卖则卖价减小
	auto instruments = BackgroundTrader::getInstance()->getInstruments();
	auto &info = instruments[order->getInstrumentId()];
	double minPrice = info->getMinimumUnit();
	if (order->getDirection() == 'b'){
		price += minPrice;
	}
	else{
		price -= minPrice;
	}
	newOrder->setPrice(price);
	newOrder->setOriginalVolume(order->getRestVolume());
	newOrder->setTradedVolume(0);
	newOrder->setRestVolume(newOrder->getOriginalVolume());
	newOrder->setOpenCloseFlag(order->getOpenCloseFlag());
	newOrder->setOrderStatus('a');
	if (newOrder->getOpenCloseFlag() == '1'){
		//当收到平仓字段时对合约进行判断，如果是上海期货的合约则自动平今
		if (info->getExchangeId() == "SHFE"){
			splitSHFEOrder(newOrder, "0");			//限价单标志0
			return;
		}
	}
	//执行指令
	executeOrder(newOrder, "0");
}

//计算某报单的回报
void Trader::calculateOrder(std::shared_ptr<Order> order){
	switch (order->getOrderStatus()){
	case 'f':
		if (order->getOpenCloseFlag() == '0'){
			double cost = -calculateOpenCost(order->getInstructionId());
			QString report = order->getInstructionId() + "," + QString::number(cost);
			qDebug() << "指令" << order->getInstructionId() << "的开仓费用：" << cost;
			instructionPort->writeBackResult(report);
			//更新策略资金
			QString investorId = order->getInvestorId();
			QString strategyId = order->getStrategyId();
			fundDao.updateStrategyFund(investorId, strategyId, cost);
		}
		else{
			double regain = calculateCloseRegain(order->getInstructionId());
			QString report = order->getInstructionId() + "," + QString::number(regain);
			qDebug() << "指令" << order->getInstructionId() << "的平仓收回：" << regain;
			instructionPort->writeBackResult(report);
			QString investorId = order->getInvestorId();
			QString strategyId = order->getStrategyId();
			fundDao.updateStrategyFund(investorId, strategyId, regain);
		}
		cleanOrderMap(order->getInstructionId());
		calculateOrders.erase(order->getOrderRef());		//从待计算集合中移除引用
		break;
		//报单错误
	case 'w':
		if (order->getOpenCloseFlag() == '0'){
			double cost = -calculateOpenCost(order->getInstructionId());
			QString report = order->getInstructionId() + "," + QString::number(cost);
			qDebug() << "指令" << order->getInstructionId() << "的开仓费用：" << cost;
			instructionPort->writeBackResult(report);
			//更新策略资金
			QString investorId = order->getInvestorId();
			QString strategyId = order->getStrategyId();
			fundDao.updateStrategyFund(investorId, strategyId, cost);
		}
		else{
			double regain = calculateCloseRegain(order->getInstructionId());
			QString report = order->getInstructionId() + "," + QString::number(regain);
			qDebug() << "指令" << order->getInstructionId() << "的平仓收回：" << regain;
			instructionPort->writeBackResult(report);
			QString investorId = order->getInvestorId();
			QString strategyId = order->getStrategyId();
			fundDao.updateStrategyFund(investorId, strategyId, regain);
			calculateOrders.erase(order->getOrderRef());		//从待计算集合中移除引用
		}
		cleanOrderMap(order->getInstructionId());
		break;
	default:
		break;
	}
}
/****************************辅助函数******************************************/