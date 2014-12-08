#include "AccountPositionDao.h"
#include <qsqlquery.h>
#include <qvariant.h>
#include "GVAR.h"

//#include <qsqlerror.h>

void AccountPositionDao::updatePosition(CThostFtdcInvestorPositionField *positionInfo){
	//先检查是否存在该账户的持仓纪录
	QSqlQuery exist(DATABASE);
	exist.prepare("select investor_id from account_position where investor_id=:id and instrument_id=:instru ");
	exist.bindValue(":id", positionInfo->InvestorID);
	exist.bindValue(":instru", positionInfo->InstrumentID);
	exist.exec();
	//qDebug() << "000:" << exist.lastError().databaseText();
	if (exist.next()){
		//如果存在就更新
		QSqlQuery update(DATABASE);
		if (positionInfo->PosiDirection == THOST_FTDC_PD_Long){	//多头持仓
			update.prepare(" update account_position set long_position=:long_position , long_profit=:long_profit, "
				" long_margin=:long_margin,query_date=:date where investor_id=:id and instrument_id=:instru ");
			update.bindValue(":long_position", positionInfo->Position);
			update.bindValue(":long_profit", positionInfo->PositionProfit);
			update.bindValue("long_margin", positionInfo->UseMargin);
			update.bindValue(":date", positionInfo->TradingDay);
			update.bindValue(":id", positionInfo->InvestorID);
			update.bindValue(":instru", positionInfo->InstrumentID);
			update.exec();
			//qDebug() << "111:"<<update.lastError().databaseText();
		}
		if (positionInfo->PosiDirection == THOST_FTDC_PD_Short){ //空头持仓
			update.prepare(" update account_position set short_position=:short_position,short_profit=:short_profit, "
				" short_margin=:short_margin,query_date=:date where investor_id=:id and instrument_id=:instru ");
			update.bindValue(":short_position", positionInfo->Position);
			update.bindValue(":short_profit", positionInfo->PositionProfit);
			update.bindValue(":short_margin", positionInfo->UseMargin);
			update.bindValue(":date", positionInfo->TradingDay);
			update.bindValue(":id", positionInfo->InvestorID);
			update.bindValue(":instru", positionInfo->InstrumentID);
			update.exec();
			//qDebug() << "222:" << update.lastError().databaseText();
		}
	}
	else
	{
		//否则就插入
		QSqlQuery insert(DATABASE);
		if (positionInfo->PosiDirection == THOST_FTDC_PD_Long){	//多头持仓
			insert.prepare("insert into account_position (investor_id,instrument_id,long_position,long_profit, "
				" long_margin,query_date) values (:id,:instru,:long_position,:long_profit,:long_margin,:date) ");
			insert.bindValue(":id", positionInfo->InvestorID);
			insert.bindValue(":instru", positionInfo->InstrumentID);
			insert.bindValue(":long_position", positionInfo->Position);
			insert.bindValue(":long_profit", positionInfo->PositionProfit);
			insert.bindValue(":long_margin", positionInfo->UseMargin);
			insert.bindValue(":date", positionInfo->TradingDay);
			insert.exec();
			//qDebug() << "333:" << insert.lastError().databaseText();
		}
		if (positionInfo->PosiDirection == THOST_FTDC_PD_Short){ //空头持仓
			insert.prepare("insert into account_position (investor_id,instrument_id,short_position,short_profit, "
				" short_margin,query_date) values (:id,:instru,:short_position,:short_profit,:short_margin,:date) ");
			insert.bindValue(":id", positionInfo->InvestorID);
			insert.bindValue(":instru", positionInfo->InstrumentID);
			insert.bindValue(":short_position", positionInfo->Position);
			insert.bindValue(":short_profit", positionInfo->PositionProfit);
			insert.bindValue(":short_margin", positionInfo->UseMargin);
			insert.bindValue(":date", positionInfo->TradingDay);
			insert.exec();
			//qDebug() << "444:" << insert.lastError().databaseText();
		}
	}
}