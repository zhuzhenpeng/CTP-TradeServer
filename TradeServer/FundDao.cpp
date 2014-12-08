#include "FundDao.h"
#include "BackgroundTrader.h"
#include "GVAR.h"
#include <qsqlquery.h>
#include <qvariant.h>
#include <qdatetime.h>

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

//记录每天资金的变化
void FundDao::logFund(CThostFtdcTradingAccountField *fund){
	QSqlQuery exist(DATABASE);
	exist.prepare(" select investor_id from history_fund where investor_id=:id and record_time=:date ");
	exist.bindValue(":id", fund->AccountID);
	exist.bindValue(":date", BackgroundTrader::getInstance()->getTradingDate());
	exist.exec();
	if (exist.next()){	//如果已经存在当日的资金记录则更新
		QSqlQuery update(DATABASE);
		update.prepare("update history_fund set pre_balance=:pre_balance,deposit=:deposit,withdraw=:withdraw, "
			" available=:available,current_margin=:current_margin,frozen_margin=:frozen_margin, "
			" commission=:commission,close_profit=:close_profit,position_profit=:position_profit "
			" where investor_id=:id and record_time=:date");
		update.bindValue(":id", fund->AccountID);
		update.bindValue(":date", BackgroundTrader::getInstance()->getTradingDate());
		update.bindValue(":pre_balance", fund->PreBalance);
		update.bindValue(":deposit", fund->Deposit);
		update.bindValue(":withdraw", fund->Withdraw);
		update.bindValue(":available", fund->Available);
		update.bindValue(":current_margin", fund->CurrMargin);
		update.bindValue(":frozen_margin", fund->FrozenMargin);
		update.bindValue(":commission", fund->Commission);
		update.bindValue(":close_profit", fund->CloseProfit);
		update.bindValue(":position_profit", fund->PositionProfit);
		update.exec();
	}
	else{	//否则插入一条新的记录
		QSqlQuery insert(DATABASE);
		insert.prepare("insert into history_fund (investor_id,record_time,pre_balance,deposit,withdraw,available, "
			" current_margin,frozen_margin,commission,close_profit,position_profit) values "
			" (:id,:date,:pre_balance,:deposit,:withdraw,:available,:current_margin,:frozen_margin,:commission, "
			" :close_profit,:position_profit )");
		insert.bindValue(":id", fund->AccountID);
		insert.bindValue(":date", BackgroundTrader::getInstance()->getTradingDate());
		insert.bindValue(":pre_balance", fund->PreBalance);
		insert.bindValue(":deposit", fund->Deposit);
		insert.bindValue(":withdraw", fund->Withdraw);
		insert.bindValue(":available", fund->Available);
		insert.bindValue(":current_margin", fund->CurrMargin);
		insert.bindValue(":frozen_margin", fund->FrozenMargin);
		insert.bindValue(":commission", fund->Commission);
		insert.bindValue(":close_profit", fund->CloseProfit);
		insert.bindValue(":position_profit", fund->PositionProfit);
		insert.exec();
	}
}

//记录最新的资金情况
void FundDao::updateFund(CThostFtdcTradingAccountField *fund){
	QSqlQuery exist(DATABASE);
	exist.prepare(" select investor_id from fund where investor_id=:id ");
	exist.bindValue(":id", fund->AccountID);
	exist.exec();
	if (exist.next()){
		//存在该用户的记录，进行更新
		QSqlQuery update(DATABASE);
		update.prepare("update fund set pre_balance=:pre_balance,deposit=:deposit,withdraw=:withdraw, "
			" available=:available,current_margin=:current_margin,frozen_margin=:frozen_margin, "
			" commission=:commission,close_profit=:close_profit,position_profit=:position_profit "
			" where investor_id=:id ");
		update.bindValue(":id", fund->AccountID);
		update.bindValue(":pre_balance", fund->PreBalance);
		update.bindValue(":deposit", fund->Deposit);
		update.bindValue(":withdraw", fund->Withdraw);
		update.bindValue(":available", fund->Available);
		update.bindValue(":current_margin", fund->CurrMargin);
		update.bindValue(":frozen_margin", fund->FrozenMargin);
		update.bindValue(":commission", fund->Commission);
		update.bindValue(":close_profit", fund->CloseProfit);
		update.bindValue(":position_profit", fund->PositionProfit);
		update.exec();
	}
	else
	{
		//不存在该用户的记录，插入
		QSqlQuery insert(DATABASE);
		insert.prepare("insert into fund (investor_id,pre_balance,deposit,withdraw,available, "
			" current_margin,frozen_margin,commission,close_profit,position_profit) values "
			" (:id,:pre_balance,:deposit,:withdraw,:available,:current_margin,:frozen_margin,:commission, "
			" :close_profit,:position_profit )");
		insert.bindValue(":id", fund->AccountID);
		insert.bindValue(":pre_balance", fund->PreBalance);
		insert.bindValue(":deposit", fund->Deposit);
		insert.bindValue(":withdraw", fund->Withdraw);
		insert.bindValue(":available", fund->Available);
		insert.bindValue(":current_margin", fund->CurrMargin);
		insert.bindValue(":frozen_margin", fund->FrozenMargin);
		insert.bindValue(":commission", fund->Commission);
		insert.bindValue(":close_profit", fund->CloseProfit);
		insert.bindValue(":position_profit", fund->PositionProfit);
		insert.exec();
	}
}

//更新策略的可用资金
void FundDao::updateStrategyFund(QString investorId, QString strategyId, double money){
	QSqlQuery update(DATABASE);
	update.prepare(" update account_strategy set available = available + :money where "
		" investor_id = :investor_id and strategy_id = :strategy_id ");
	update.bindValue(":money", money);
	update.bindValue(":investor_id", investorId);
	update.bindValue(":strategy_id", strategyId);
	update.exec();
}