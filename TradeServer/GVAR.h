#pragma once
#include <qsqldatabase.h>
#include <map>
#include <qstring.h>
#include <memory>
#include "Trader.h"
#include "InstructionPort.h"

extern QSqlDatabase DATABASE;

extern std::map<QString, std::shared_ptr<Trader>> TRADERS;

extern InstructionPort *instructionPort;

//函数声明
namespace init{
	//连接数据库
	void connectToDatabase();

	//更新合约信息，订阅行情并打开端口
	void initBroadcast();

	//初始化账户-策略持仓表
	void initStrategyPosition();

	//初始化交易账户
	void initAccounts();

	//初始化指令端口
	void initInstructionPort();
}