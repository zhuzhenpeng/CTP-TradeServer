#include "GVAR.h"
#include "StrategyPositionDao.h"
#include "BackgroundTrader.h"
#include "MDBroadcast.h"
#include "AccountID.h"
#include "Trader.h"
#include <qthread.h>
#include <qfile.h>
#include <qsqlquery.h>
#include <qdebug.h>
#include <memory>

using std::make_shared;
using std::shared_ptr;
using std::map;


#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

QSqlDatabase DATABASE;

map<QString, shared_ptr<Trader>> TRADERS;

InstructionPort *instructionPort = nullptr;

//连接数据库
void init::connectToDatabase(){
	DATABASE = QSqlDatabase::addDatabase("QMYSQL");
	DATABASE.setHostName("127.0.0.1");
	DATABASE.setDatabaseName("trade_server");
	DATABASE.setUserName("root");
	DATABASE.setPassword("");
	if (!DATABASE.open()){
		qDebug() << "数据库连接错误！";
		abort();
	}
	qDebug() << "数据库连接成功!";
}

//更新合约信息，订阅行情并打开端口
void init::initBroadcast(){
	//用以存储数据库中新加入的合约代号
	std::vector<QString> toBeSubscribeInstru;
	//初始化后台账号
	auto instruments = BackgroundTrader::getInstance()->getInstruments();
	qDebug() << "共有" << instruments.size() << "个合约";
	for (auto &item : instruments){
		auto &s = item.second;
		qDebug() << s->getName() << "合约乘数:" << s->getMarginRate() << "最小变动单位:" << s->getMinimumUnit() << "手续费:"
			<< s->getCloseCommission() << s->getOpenCommission() << s->getCloseTodayCommission();
		toBeSubscribeInstru.push_back(item.first);
	}
	qDebug() << "今天是:" << BackgroundTrader::getInstance()->getTradingDate();
	//初始化行情广播
	auto broacast = MDBroadcast::getInstance();
	while (true){
		if (broacast->isReadyToSubscribe()){
			broacast->subscribeInstruments(toBeSubscribeInstru);
			break;
		}
	}
	//为行情广播注册通道
	QThread *channelThread = new QThread();
	auto channel = MDChannel::getInstance();
	channel->moveToThread(channelThread);
	channelThread->start();
	broacast->setChannel(channel);
}

//初始化账户-策略持仓表
void init::initStrategyPosition(){
	StrategyPositionDao::synStrategyPosition();
	StrategyPositionDao::refreshDaily();
}

//初始化交易交易账户
void init::initAccounts(){
	QFile iniFile("user/acount.ini");
	if (!iniFile.open(QIODevice::ReadOnly | QIODevice::Text)){
		qDebug() << "cannot find front.ini";
		abort();
	}
	QTextStream in(&iniFile);
	QString investorID;
	while (!in.atEnd()){
		investorID = in.readLine();
		//根据账户ID从表中读取其它数据 
		QSqlQuery query(DATABASE);
		query.prepare("select password,broker_id,front_address from account where investor_id=:id ");
		query.bindValue(":id", investorID);
		query.exec();
		query.next();
		if (query.isNull(0)){
			qDebug() << "数据库中没有账户:" << investorID << "信息，请检查配置文件与数据库";
			abort();
		}
		QString &password = query.value("password").toString();
		QString &broker_id = query.value("broker_id").toString();
		QString &front_address = query.value("front_address").toString();
		auto accountID = make_shared<AccountID>();
		accountID->setInvestorID(investorID);
		accountID->setPassword(password);
		accountID->setFrontAddress(front_address);
		accountID->setBrokerID(broker_id);
		auto trader = make_shared<Trader>(accountID);
		//把账户放入全局变量中
		TRADERS.insert(std::make_pair(investorID, trader));
	}
	iniFile.close();
}

//初始化指令端口
void init::initInstructionPort(){
	instructionPort = new InstructionPort();
}