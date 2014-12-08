#include "BackgroundTrader.h"
#include "ThostFtdcTraderApi.h"
#include "GVAR.h"
#include <qdebug.h>
#include <qsqlquery.h>
#include <qvariant.h>
#include <qstringlist.h>
#include <qdir.h>
#include <vector>
#include <thread>
#include <chrono>

using std::shared_ptr;
using std::make_shared;
using std::make_pair;
using std::map;

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

shared_ptr<BackgroundTrader> BackgroundTrader::bgTrader = nullptr;

std::shared_ptr<BackgroundTrader> BackgroundTrader::getInstance(){
	if (bgTrader != nullptr){
		return bgTrader;
	}
	else{
		bgTrader = shared_ptr<BackgroundTrader>(new BackgroundTrader());
		return bgTrader;
	}
}

BackgroundTrader::~BackgroundTrader(){
	if (api != nullptr){
		api->RegisterSpi(nullptr);
		api->Release();
		api = nullptr;
	}
}

const QString & BackgroundTrader::getTradingDate() const{
	return tradingDate;
}

const map<QString, shared_ptr<InstrumentInfo>> & BackgroundTrader::getInstruments() const{
	return interestedInstruments;
}

/***************************私有辅助函数**********************************************************/

//构造函数
BackgroundTrader::BackgroundTrader(){
	accountID = make_shared<AccountID>();
	//从数据库中读取后台账户的信息:后台账号为实盘账号83601689
	QSqlQuery query(DATABASE);
	QString initialize_AccountID_SQL = "select * from account where investor_id = 83601689";
	if (query.exec(initialize_AccountID_SQL) && query.next()){
		QString investor_id = query.value("investor_id").toString();
		QString password = query.value("password").toString();
		QString broker_id = query.value("broker_id").toString();
		QString front_address = query.value("front_address").toString();
		accountID->setInvestorID(investor_id);
		accountID->setPassword(password);
		accountID->setBrokerID(broker_id);
		accountID->setFrontAddress(front_address);
	}
	//把后台账号的文件单独放在一个文件当中
	QString dirName = "user/background";
	QDir conFileDir(dirName);
	if (!conFileDir.exists()){
		conFileDir.mkpath(".");
	}
	api = CThostFtdcTraderApi::CreateFtdcTraderApi((dirName + "/").toStdString().c_str());
	api->RegisterSpi(this);
	//订阅共有流、私有流，注册前置机并初始化
	api->SubscribePublicTopic(THOST_TERT_RESTART);
	api->SubscribePrivateTopic(THOST_TERT_RESTART);
	char *frontAddress = new char[100];
	strcpy(frontAddress, accountID->getFrontAddress().toStdString().c_str());
	api->RegisterFront(frontAddress);
	api->Init();
	while (true){
		if (loginFlag){
			init();
			break;
		}
	}
}

//初始化对象，获得交易日期、策略需要的合约信息
void BackgroundTrader::init(){
	//从数据库中读取策略感兴趣的所有合约，初始化这些合约信息，并把它们写回数据库的合约信息表
	QSqlQuery query(DATABASE);
	QString read_all_interested_instruments = "select interested_instruments from strategy";
	query.exec(read_all_interested_instruments);
	while (query.next()){
		QString rawString = query.value(0).toString();
		initInterestedInstruments(rawString);
	}
	supplementInstrumentInfo();
	tradingDate = QString(api->GetTradingDay());
}

//从"策略表"读出所有所有感兴趣的合约
void BackgroundTrader::initInterestedInstruments(QString &rawString){
	if (rawString.contains(";")){
		//按照约定，包含;号表示该策略对多个合约感兴趣，因此要进一步拆分得到所有合约名字
		auto instrumentIDs = rawString.split(";");
		for (auto &instrumentID : instrumentIDs){
			auto instrumentInfo = make_shared<InstrumentInfo>();
			auto trimmedID = instrumentID.trimmed();
			instrumentInfo->setId(trimmedID);
			interestedInstruments.insert(make_pair(trimmedID, instrumentInfo));
		}
	}
	else
	{
		auto instrumentInfo = make_shared<InstrumentInfo>();
		auto trimmedID = rawString.trimmed();
		instrumentInfo->setId(trimmedID);
		interestedInstruments.insert(make_pair(trimmedID, instrumentInfo));
	}
}

//补充所有合约的信息，如果本地数据库没有则向服务器查询
void BackgroundTrader::supplementInstrumentInfo(){
	QSqlQuery query(DATABASE);
	query.prepare("select * from instrument_info where id = :id");
	for (auto &item : interestedInstruments){
		query.bindValue(":id", item.first);
		query.exec();
		if (query.next()){
			auto &info = item.second;
			info->setName(query.value("name").toString());
			info->setExchangeId(query.value("exchange_id").toString());
			info->setDeadline(query.value("deadline").toDate());
			info->setMarginRate(query.value("margin_rate").toDouble());
			info->setMutiplier(query.value("multiplier").toInt());
			info->setMinimumUnit(query.value("minimum_unit").toDouble());
			double oc = query.value("oc").toDouble();
			double oc_rate = query.value("oc_rate").toDouble();
			info->setOpenCommission(oc > oc_rate ? oc : oc_rate);	//手续费率和手续费选择一个是有效的，无效的在数据库中置0
			double cc = query.value("cc").toDouble();
			double cc_rate = query.value("cc_rate").toDouble();
			info->setCloseCommission(cc > cc_rate ? cc : cc_rate);
			double today_cc = query.value("today_cc").toDouble();
			double today_cc_rate = query.value("today_cc_rate").toDouble();
			info->setCloseTodayCommission(today_cc > today_cc_rate ? today_cc : today_cc_rate);
		}
		else{
			newInstruments.push(item.first);	//往新合约队列中添加合约
		}
	}
	//对于数据库中之前没存相关资料的合约，逐个发送查询到服务器进行查询，在返回中写入数据库并保存该对象
	while (!newInstruments.empty() && readyForNext()){
		auto &instruName = newInstruments.front();
		finishBasicQuery = false;
		finishCommissionQuery = false;
		qDebug() << "正在向服务器查询:" << instruName << "信息...";
		//查询合约基本信息
		CThostFtdcQryInstrumentField *a = new CThostFtdcQryInstrumentField();
		strcpy(a->InstrumentID, instruName.toStdString().c_str());
		strcpy(a->ExchangeID, "");
		api->ReqQryInstrument(a, getRequestID());
		std::this_thread::sleep_for(std::chrono::seconds(1));	//受流限制，每次查询间隔1秒
		//查询合约手续费
		CThostFtdcQryInstrumentCommissionRateField *b = new CThostFtdcQryInstrumentCommissionRateField();
		strcpy(b->BrokerID, accountID->getBrokerID().toStdString().c_str());
		strcpy(b->InvestorID, accountID->getInvestorID().toStdString().c_str());
		strcpy(b->InstrumentID, instruName.toStdString().c_str());
		api->ReqQryInstrumentCommissionRate(b, getRequestID());
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}

//判断newInstruments队列中是否继续处理下一项
bool BackgroundTrader::readyForNext(){
	return (finishBasicQuery && finishCommissionQuery);
}

int BackgroundTrader::getRequestID(){
	requestID++;
	return requestID;
}

void BackgroundTrader::errorInstrumentID(char *id){
	if (id == nullptr){
		qDebug() << "错误合约名字:" << newInstruments.front() << "，请检查数据库的策略信息";
		abort();
	}
}

/***************************Spi事件回调函数*******************************************************/

///当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
void BackgroundTrader::OnFrontConnected(){
	//连接到服务器之后自动登陆,让账户与服务器连接上
	CThostFtdcReqUserLoginField *loginField = new CThostFtdcReqUserLoginField();
	strcpy(loginField->BrokerID, accountID->getBrokerID().toStdString().c_str());
	strcpy(loginField->UserID, accountID->getInvestorID().toStdString().c_str());
	strcpy(loginField->Password, accountID->getPassword().toStdString().c_str());
	api->ReqUserLogin(loginField, getRequestID());
}

///登录请求响应
void BackgroundTrader::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){
	if (pRspInfo == nullptr || pRspInfo->ErrorID == 0){
		loginFlag = true;
		qDebug() << "已连接上服务器，进行本地初始化...";
	}
	else{
		qDebug() << "后台账户登录失败!";
		abort();
	}
}

///请求查询合约响应
void BackgroundTrader::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	errorInstrumentID(pInstrument->InstrumentID);
	if (pRspInfo == nullptr || pRspInfo->ErrorID == 0){
		QSqlQuery query(DATABASE);
		query.prepare("insert into instrument_info (id,name,exchange_id,deadline,margin_rate,multiplier,minimum_unit) "
			" values (:id,:name,:exchange_id,:deadline,:margin_rate,:multiplier,:minimum_unit)");
		query.bindValue(":id", pInstrument->InstrumentID);
		query.bindValue(":name", QString::fromLocal8Bit(pInstrument->InstrumentName));
		query.bindValue(":exchange_id", pInstrument->ExchangeID);
		query.bindValue(":deadline", pInstrument->ExpireDate);
		query.bindValue(":margin_rate", pInstrument->LongMarginRatio);
		query.bindValue(":multiplier", pInstrument->VolumeMultiple);
		query.bindValue(":minimum_unit", pInstrument->PriceTick);
		query.exec();
		//写入内存对象中
		auto &info = interestedInstruments[QString(pInstrument->InstrumentID)];
		info->setName(QString::fromLocal8Bit(pInstrument->InstrumentName));
		info->setExchangeId(QString(pInstrument->ExchangeID));
		info->setDeadline(QDate::fromString(QString(pInstrument->ExpireDate), "yyyyMMdd"));
		info->setMarginRate(pInstrument->LongMarginRatio);
		info->setMutiplier(pInstrument->VolumeMultiple);
		info->setMinimumUnit(pInstrument->PriceTick);
		finishBasicQuery = true;
		if (readyForNext()){
			newInstruments.pop();
		}
	}
}

///请求查询合约手续费率响应
void BackgroundTrader::OnRspQryInstrumentCommissionRate(CThostFtdcInstrumentCommissionRateField *pInstrumentCommissionRate,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	errorInstrumentID(pInstrumentCommissionRate->InstrumentID);
	if (pRspInfo == nullptr || pRspInfo->ErrorID == 0){
		QSqlQuery query(DATABASE);
		query.prepare("update instrument_info set oc=:oc , oc_rate=:oc_rate , cc=:cc , cc_rate=:cc_rate ," 
			" today_cc=:today_cc , today_cc_rate=:today_cc_rate where id=:id ");
		query.bindValue(":id", newInstruments.front());
		double &oc = pInstrumentCommissionRate->OpenRatioByVolume;
		query.bindValue(":oc", oc);
		double &oc_rate = pInstrumentCommissionRate->OpenRatioByMoney;
		query.bindValue(":oc_rate", oc_rate);
		double &cc = pInstrumentCommissionRate->OpenRatioByVolume;
		query.bindValue(":cc", cc);
		double &cc_rate = pInstrumentCommissionRate->CloseRatioByMoney;
		query.bindValue(":cc_rate", cc_rate);
		double &today_cc = pInstrumentCommissionRate->CloseTodayRatioByVolume;
		query.bindValue(":today_cc", today_cc);
		double &today_cc_rate = pInstrumentCommissionRate->CloseTodayRatioByMoney;
		query.bindValue(":today_cc_rate", today_cc_rate);
		query.exec();
		//写入内存对象中
		auto &info = interestedInstruments[newInstruments.front()];
		info->setOpenCommission(oc > oc_rate ? oc : oc_rate);
		info->setCloseCommission(cc > cc_rate ? cc : cc_rate);
		info->setCloseTodayCommission(today_cc > today_cc_rate ? today_cc : today_cc_rate);
		finishCommissionQuery = true;
		if (readyForNext()){
			newInstruments.pop();
		}
	}
}