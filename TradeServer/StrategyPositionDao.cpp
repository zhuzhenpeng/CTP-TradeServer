#include "StrategyPositionDao.h"
#include "BackgroundTrader.h"
#include "GVAR.h"
#include <qsqlquery.h>
#include <qvariant.h>
#include <qdebug.h>

//根据成交回报更新策略持仓情况
void StrategyPositionDao::updatePosition(CThostFtdcTradeField *pTrade, QString strategyId){
	//买入开仓
	if (pTrade->Direction == THOST_FTDC_D_Buy && pTrade->OffsetFlag == THOST_FTDC_OF_Open){
		QSqlQuery updateLong(DATABASE);
		updateLong.prepare("update strategy_position set long_position=long_position+:position,today_long_position=today_long_position+:today_position "
			" where investor_id=:investor and strategy_id=:strategy and instrument_id=:instrument ");
		updateLong.bindValue(":position", pTrade->Volume);
		updateLong.bindValue(":investor", pTrade->InvestorID);
		updateLong.bindValue(":strategy", strategyId);
		updateLong.bindValue(":instrument", pTrade->InstrumentID);
		//判断是否是上期的合约来记录今日仓位
		if (isSHFE(pTrade->InstrumentID)){
			updateLong.bindValue(":today_position", pTrade->Volume);
		}
		else
		{
			updateLong.bindValue(":today_position", 0);
		}
		updateLong.exec();
	}
	//买入平仓
	if (pTrade->Direction == THOST_FTDC_D_Buy && ( pTrade->OffsetFlag == THOST_FTDC_OF_Close ||
		pTrade->OffsetFlag == THOST_FTDC_OF_CloseToday || pTrade->OffsetFlag == THOST_FTDC_OF_CloseYesterday)){
		QSqlQuery updateLong(DATABASE);
		updateLong.prepare("update strategy_position set short_position=short_position-:position,today_long_position=today_long_position-:today_position "
			" where investor_id=:investor and strategy_id=:strategy and instrument_id=:instrument ");
		updateLong.bindValue(":position", pTrade->Volume);
		updateLong.bindValue(":investor", pTrade->InvestorID);
		updateLong.bindValue(":strategy", strategyId);
		updateLong.bindValue(":instrument", pTrade->InstrumentID);
		//判断是否是上期的合约来记录今日仓位
		if (isSHFE(pTrade->InstrumentID)){
			updateLong.bindValue(":today_position", pTrade->Volume);
		}
		else
		{
			updateLong.bindValue(":today_position", 0);
		}
		updateLong.exec();
	}
	//卖出开仓
	if (pTrade->Direction == THOST_FTDC_D_Sell && pTrade->OffsetFlag == THOST_FTDC_OF_Open){
		QSqlQuery updateShort(DATABASE);
		updateShort.prepare("update strategy_position set short_position=short_position+:position,today_short_position=today_short_position+:today_position "
			" where investor_id=:investor and strategy_id=:strategy and instrument_id=:instrument ");
		updateShort.bindValue(":position", pTrade->Volume);
		updateShort.bindValue(":investor", pTrade->InvestorID);
		updateShort.bindValue(":strategy", strategyId);
		updateShort.bindValue(":instrument", pTrade->InstrumentID);
		//判断是否是上期的合约来记录今日仓位
		if (isSHFE(pTrade->InstrumentID)){
			updateShort.bindValue(":today_position", pTrade->Volume);
		}
		else
		{
			updateShort.bindValue(":today_position", 0);
		}
		updateShort.exec();
	}
	//卖出平仓
	if (pTrade->Direction == THOST_FTDC_D_Sell && (pTrade->OffsetFlag == THOST_FTDC_OF_Close ||
		pTrade->OffsetFlag == THOST_FTDC_OF_CloseToday || pTrade->OffsetFlag == THOST_FTDC_OF_CloseYesterday)){
		QSqlQuery updateShort(DATABASE);
		updateShort.prepare("update strategy_position set long_position=long_position-:position,today_short_position=today_short_position-:today_position "
			" where investor_id=:investor and strategy_id=:strategy and instrument_id=:instrument ");
		updateShort.bindValue(":position", pTrade->Volume);
		updateShort.bindValue(":investor", pTrade->InvestorID);
		updateShort.bindValue(":strategy", strategyId);
		updateShort.bindValue(":instrument", pTrade->InstrumentID);
		//判断是否是上期的合约来记录今日仓位
		if (isSHFE(pTrade->InstrumentID)){
			updateShort.bindValue(":today_position", pTrade->Volume);
		}
		else
		{
			updateShort.bindValue(":today_position", 0);
		}
		updateShort.exec();
	}
}

//根据账户策略关系表同步策略持仓表
//1.账户有了新的策略之后建立记录
//2.账户与策略解除关系后删除记录
void StrategyPositionDao::synStrategyPosition(){
	//1.账户有了新的策略之后建立记录
	QSqlQuery accountStrategy(DATABASE);
	accountStrategy.exec("select investor_id,strategy_id from account_strategy");
	while (accountStrategy.next()){
		QString investor_id = accountStrategy.value("investor_id").toString();
		QString strategy_id = accountStrategy.value("strategy_id").toString();
		//从strategy_position中检查是否存在相应的账户-策略仓位
		QSqlQuery exist;
		exist.prepare("select investor_id from strategy_position where investor_id=:investor_id and strategy_id=:strategy_id");
		exist.exec();
		exist.next();	//滚到记录集中
		if (exist.isNull("investor_id")){
			//如果不存在，检查该策略有什么感兴趣的合约，然后分别为这些合约建立账户-策略合约仓位
			QSqlQuery strategyInstruments;
			strategyInstruments.prepare("select interested_instruments from strategy where id=:id ");
			strategyInstruments.bindValue(":id", strategy_id);
			strategyInstruments.exec();
			strategyInstruments.next();
			QString rawInstruments = strategyInstruments.value(0).toString();
			auto instruments = rawInstruments.trimmed().split(";");
			for (auto &instrument : instruments){
				QSqlQuery insert;
				insert.prepare("insert into strategy_position (investor_id,strategy_id,instrument_id,today) "
					" values (:investor_id,:strategy_id,:instrument_id,:today) ");
				insert.bindValue(":investor_id", investor_id);
				insert.bindValue(":strategy_id", strategy_id);
				insert.bindValue(":instrument_id", instrument);
				insert.bindValue(":today", BackgroundTrader::getInstance()->getTradingDate());
				insert.exec();
			}
		}
	}
	//2.账户与策略解除关系后删除记录
	QSqlQuery strategyPositions(DATABASE);
	strategyPositions.exec("select investor_id,strategy_id from strategy_position");
	while (strategyPositions.next()){
		QString investor_id = strategyPositions.value("investor_id").toString();
		QString strategy_id = strategyPositions.value("strategy_id").toString();
		//从account_strategy表中检查是否存在相应的账户-策略关系
		QSqlQuery exist;
		exist.prepare("select investor_id from account_strategy where investor_id=:investor_id and strategy_id=:strategy_id");
		exist.bindValue(":investor_id", investor_id);
		exist.bindValue(":strategy_id", strategy_id);
		exist.exec();
		exist.next();
		if (exist.isNull("investor_id")){
			QSqlQuery deleteSQL;
			deleteSQL.prepare("delete from strategy_position where investor_id=:investor_id and strategy_id=:strategy_id ");
			deleteSQL.bindValue(":investor_id", investor_id);
			deleteSQL.bindValue(":strategy_id", strategy_id);
			deleteSQL.exec();
		}
	}
}

//程序在新的交易日运行时，刷新策略持仓表中的时间记录，把今日持仓清零
void StrategyPositionDao::refreshDaily(){
	const QString &today = BackgroundTrader::getInstance()->getTradingDate();
	QSqlQuery refresh(DATABASE);
	refresh.prepare("update strategy_position set today_long_position=0,today_short_position=0 "
		" where today!=:today");
	refresh.bindValue(":today", today);
	refresh.exec();
	QSqlQuery setToday(DATABASE);
	setToday.prepare("update strategy_position set today=:today");
	setToday.bindValue(":today", today);
	setToday.exec();
}

bool StrategyPositionDao::isSHFE(QString instrumentID){
	auto instruments = BackgroundTrader::getInstance()->getInstruments();
	auto &instrumentInfo = instruments[instrumentID];
	if (instrumentInfo->getExchangeId() == "SHFE"){
		return true;
	}
	else
	{
		return false;
	}
}

int StrategyPositionDao::getTodayPosition(const QString &investorId, const QString &strategyId,
	const QString &instrumentId, char direction){
	QSqlQuery query;
	query.prepare("select today_long_position,today_short_position from strategy_position "
		" where investor_id=:investor_id and strategy_id=:strategy_id and instrument_id=:instrument_id ");
	query.bindValue(":investor_id", investorId);
	query.bindValue(":strategy_id", strategyId);
	query.bindValue(":instrument_id", instrumentId);
	query.exec();
	query.next();
	if (direction == 'b'){
		return query.value("today_long_position").toInt();
	}
	else
	{
		return query.value("today_short_position").toInt();
	}
}