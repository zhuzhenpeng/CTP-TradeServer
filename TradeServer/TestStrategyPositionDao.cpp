#include "TestStrategyPositionDao.h"
#include "StrategyPositionDao.h"
#include "GVAR.h"

#include <qtest.h>
#include <memory>
using std::shared_ptr;
using std::make_shared;

void TestStrategyPositionDao::testBuyOpen(){
	//连接数据库
	init::connectToDatabase();

	//更新合约信息，订阅行情并打开端口
	init::initBroadcast();

	//初始化账户-策略持仓表
	init::initStrategyPosition();

	//初始化交易账户
	init::initAccounts();

	//初始化指令端口
	init::initInstructionPort();

	shared_ptr<StrategyPositionDao> dao = make_shared<StrategyPositionDao>();

	CThostFtdcTradeField *pTrade = new CThostFtdcTradeField();
	pTrade->Direction = THOST_FTDC_D_Buy;
	pTrade->OffsetFlag = THOST_FTDC_OF_Open;
	strcpy(pTrade->InvestorID, "00000004");
	pTrade->Volume = 100;
	strcpy(pTrade->InstrumentID, "IF1409");

	QString strategyId = "001";
	dao->updatePosition(pTrade, strategyId);
}

//QTEST_MAIN(TestStrategyPositionDao)

