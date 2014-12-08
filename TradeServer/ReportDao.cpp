#include "ReportDao.h"
#include "GVAR.h"
#include "BackgroundTrader.h"
#include <qsqlquery.h>
#include <qvariant.h>
//#include <qsqlerror.h>

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

//根据成交回报更新回报表格
void ReportDao::updateReportTable(CThostFtdcTradeField *report, QString &strategyId){
	//开仓回报
	if (report->OffsetFlag == THOST_FTDC_OF_Open){
		QSqlQuery insertOpen(DATABASE);
		insertOpen.prepare("insert into open_traded_report (trade_date,trade_id,trade_time,investor_id, "
			" strategy_id,system_id,order_ref,instrument_id,direction,volume,open_price,to_be_close ) values "
			" (:date,:trade_id,:time,:investor_id,:strategy_id,:system_id,:order_ref,:instrument_id,:direction, "
			"  :volume,:open_price,:to_be_close)");
		insertOpen.bindValue(":date", report->TradeDate);
		insertOpen.bindValue(":trade_id", report->TradeID);
		insertOpen.bindValue(":time", report->TradeTime);
		insertOpen.bindValue(":investor_id", report->InvestorID);
		insertOpen.bindValue(":strategy_id", strategyId);
		insertOpen.bindValue(":system_id", report->OrderSysID);
		insertOpen.bindValue(":order_ref", atoi(report->OrderRef));
		insertOpen.bindValue(":instrument_id", report->InstrumentID);
		QString direction;
		if (report->Direction == THOST_FTDC_D_Buy){
			direction = "b";
		}
		else
		{
			direction = "s";
		}
		insertOpen.bindValue(":direction", direction);
		insertOpen.bindValue(":volume", report->Volume);
		insertOpen.bindValue(":open_price", report->Price);
		insertOpen.bindValue(":to_be_close", report->Volume);
		insertOpen.exec();
	}
	//平仓回报
	if (report->OffsetFlag == THOST_FTDC_OF_Close ||
		report->OffsetFlag == THOST_FTDC_OF_CloseToday || report->OffsetFlag == THOST_FTDC_OF_CloseYesterday){
		QSqlQuery insertClose(DATABASE);
		insertClose.prepare("insert into close_traded_report (trade_date,trade_id,trade_time,investor_id, "
			" strategy_id,system_id,order_ref,instrument_id,direction,volume,close_price,today_flag ) values "
			" (:date,:trade_id,:time,:investor_id,:strategy_id,:system_id,:order_ref,:instrument_id,:direction, "
			"  :volume,:close_price,:today_flag)");
		insertClose.bindValue(":date", report->TradeDate);
		insertClose.bindValue(":trade_id", report->TradeID);
		insertClose.bindValue(":time", report->TradeTime);
		insertClose.bindValue(":investor_id", report->InvestorID);
		insertClose.bindValue(":strategy_id", strategyId);
		insertClose.bindValue(":system_id", report->OrderSysID);
		insertClose.bindValue(":order_ref", atoi(report->OrderRef));
		insertClose.bindValue(":instrument_id", report->InstrumentID);
		QString direction;
		if (report->Direction == THOST_FTDC_D_Buy){
			direction = "b";
		}
		else
		{
			direction = "s";
		}
		insertClose.bindValue(":direction", direction);
		insertClose.bindValue(":volume", report->Volume);
		insertClose.bindValue(":close_price", report->Price);
		QString today_flag;
		if (report->OffsetFlag == THOST_FTDC_OF_CloseToday){
			today_flag = "t";
		}
		else{
			today_flag = "y";
		}
		insertClose.bindValue(":today_flag", today_flag);
		insertClose.exec();
		//生成静态权益
		generateStaticProfit(report, strategyId);
	}
}

//如果是平仓回报，在更新完回报表格之后生成静态权益
void ReportDao::generateStaticProfit(CThostFtdcTradeField *report, QString &strategyId){
	int volume = report->Volume;
	while (volume > 0){
		QString openDate;
		QString openTradeId;
		double openPrice;
		int openVolume;
		QSqlQuery selectOpen(DATABASE);
		if (report->OffsetFlag == THOST_FTDC_OF_CloseToday){
			//平今仓
			selectOpen.prepare("select trade_date,trade_id,open_price,volume from open_traded_report "
				" where trade_date=:date and investor_id=:id and strategy_id=:strategy_id and instrument_id=:instrument_id and "
				" to_be_close!=0 order by trade_time limit 1");
			selectOpen.bindValue(":date", report->TradeDate);
		}
		else{
			//平旧仓
			QSqlQuery selectOpen(DATABASE);
			selectOpen.prepare("select trade_date,trade_id,open_price,volume from open_traded_report "
				" where investor_id=:id and strategy_id=:strategy_id and instrument_id=:instrument_id and "
				" to_be_close!=0 order by trade_time limit 1");		//sql语句中不限定日期
		}
		selectOpen.bindValue(":id", report->InvestorID);
		selectOpen.bindValue(":strategy_id", strategyId);
		selectOpen.bindValue(":instrument_id", report->InstrumentID);
		selectOpen.exec();
		//qDebug() << "111:" << selectOpen.lastError().databaseText();
		selectOpen.next();
		openDate = selectOpen.value("trade_date").toString();
		openTradeId = selectOpen.value("trade_id").toString();
		openPrice = selectOpen.value("open_price").toDouble();
		openVolume = selectOpen.value("volume").toInt();

		//开仓量大于平仓量
		if (openVolume >= volume){
			// 更新开仓回报中的to_be_close字段
			QSqlQuery updateOpen(DATABASE);
			updateOpen.prepare("update open_traded_report set to_be_close=to_be_close-:volume where "
				" trade_date=:date and trade_id=:id");
			updateOpen.bindValue(":volume", volume);
			updateOpen.bindValue(":date", report->TradeDate);
			updateOpen.bindValue(":id", openTradeId);
			updateOpen.exec();
			//qDebug() << "222:" << updateOpen.lastError().databaseText();
			//生成静态权益记录
			QSqlQuery closeProfit(DATABASE);
			closeProfit.prepare("insert into close_profit (close_date,close_id,open_date,open_id,close_price, "
				" open_price,volume,profit) values (:close_date,:close_id,:open_date,:open_id,:close_price, "
				" :open_price,:volume,:profit)");
			closeProfit.bindValue(":close_date", report->TradeDate);
			closeProfit.bindValue(":close_id", report->TradeID);
			closeProfit.bindValue(":open_date", openDate);
			closeProfit.bindValue(":open_id", openTradeId);
			closeProfit.bindValue(":close_price", report->Price);
			closeProfit.bindValue(":open_price", openPrice);
			closeProfit.bindValue(":volume", volume);
			double profit = 0;
			if (report->OffsetFlag == THOST_FTDC_OF_CloseToday){
				profit = calculateStaticProfit_today(report->InstrumentID, openPrice,
					report->Price, volume, report->Direction);
			}
			else{
				profit = calculateStaticProfit(report->InstrumentID, openPrice,
					report->Price, volume, report->Direction);
			}
			closeProfit.bindValue(":profit", profit);
			closeProfit.exec();
			//qDebug() << "333:" << closeProfit.lastError().databaseText();
			return;		//直接返回即可
		}
		else{
			//开仓量小于平仓量
			// 更新开仓回报中的to_be_close字段
			QSqlQuery updateOpen(DATABASE);
			updateOpen.prepare("update set to_be_close=0 where "
				" trade_date=:date,trade_id=:id");		//to_be_close字段清零
			updateOpen.bindValue(":date", report->TradeDate);
			updateOpen.bindValue(":id", openTradeId);
			updateOpen.exec();
			//生成静态权益记录
			QSqlQuery closeProfit(DATABASE);
			closeProfit.prepare("insert into close_profit (close_date,close_id,open_date,open_id,close_price, "
				" open_price,volume,profit) values (:close_date,:close_id,:open_date,:open_id,:close_price, "
				" :open_price,:volume,:profit)");
			closeProfit.bindValue(":close_date", report->TradeDate);
			closeProfit.bindValue(":close_id", report->TradeID);
			closeProfit.bindValue(":open_date", openDate);
			closeProfit.bindValue(":open_id", openTradeId);
			closeProfit.bindValue(":close_price", report->Price);
			closeProfit.bindValue(":open_price", openPrice);
			closeProfit.bindValue(":volume", openVolume);	//量也变成开仓量
			double profit = 0;
			if (report->OffsetFlag == THOST_FTDC_OF_CloseToday){
				profit = calculateStaticProfit_today(report->InstrumentID, openPrice,
					report->Price, openVolume, report->Direction);
			}
			else{
				profit = calculateStaticProfit(report->InstrumentID, openPrice,
					report->Price, openVolume, report->Direction);
			}
			closeProfit.bindValue(":profit", profit);
			closeProfit.exec();
			//qDebug() << "444:" << closeProfit.lastError().databaseText();
			volume -= openVolume;
		}
	}
}

//计算今日静态权益
double ReportDao::calculateStaticProfit_today(QString instrumentID, double openPrice,
	double closePrice, int volume, char closeDirection){
	//'0'买,'1'卖
	auto instruments = BackgroundTrader::getInstance()->getInstruments();
	auto info = instruments[instrumentID];
	int multiplier = info->getMultiplier();
	double openCommission = info->getOpenCommission();
	double totalOpenCommission = 0;
	if (openCommission < 1){
		//手续费=点数*合约乘数*手数*百分比
		totalOpenCommission = openPrice*multiplier*volume*openCommission;
	}
	else{
		//手续费=手数*每手钱数
		totalOpenCommission = volume*openCommission;
	}
	double closeCommission = info->getCloseTodayCommission();	//获取今日平仓结算价
	double totalCloseCommission = 0;
	if (closeCommission < 1){
		totalCloseCommission = closePrice*multiplier*volume*closeCommission;
	}
	else{
		totalCloseCommission = volume*closeCommission;
	}
	double totalCommission = totalCloseCommission + totalOpenCommission;
	//计算点数变化盈亏
	//平多头
	double profit = (closePrice - openPrice)*volume*multiplier;
	//平空头
	if (closeDirection == '0'){
		profit = -profit;
	}
	//最终结果
	profit = profit - totalCloseCommission;
	qDebug() << "volume:" << volume;
	qDebug() << "close price:" << closePrice;
	qDebug() << "open price:" << openPrice;
	qDebug() << "multipler:" << multiplier;
	qDebug() << "commission：" << totalCloseCommission;
	qDebug() << "---profit---:" << profit;
	return profit;
}

//计算静态权益
double ReportDao::calculateStaticProfit(QString instrumentID, double openPrice,
	double closePrice, int volume, char closeDirection){
	//'0'买,'1'卖
	auto instruments = BackgroundTrader::getInstance()->getInstruments();
	auto info = instruments[instrumentID];
	int multiplier = info->getMultiplier();
	double openCommission = info->getOpenCommission();
	double totalOpenCommission = 0;
	if (openCommission < 1){
		//手续费=点数*合约乘数*手数*百分比
		totalOpenCommission = openPrice*multiplier*volume*openCommission;
	}
	else{
		//手续费=手数*每手钱数
		totalOpenCommission = volume*openCommission;
	}
	double closeCommission = info->getCloseCommission();	//获取昨日平仓结算价
	double totalCloseCommission = 0;
	if (closeCommission < 1){
		totalCloseCommission = closePrice*multiplier*volume*closeCommission;
	}
	else{
		totalCloseCommission = volume*closeCommission;
	}
	double totalCommission = totalCloseCommission + totalOpenCommission;
	//计算点数变化盈亏
	double profit = (closePrice - openPrice)*volume*multiplier;
	if (closeDirection == '0'){
		profit = -profit;
	}
	//最终结果
	profit -= totalCloseCommission;
	return profit;
}