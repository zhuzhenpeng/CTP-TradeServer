#include "OrderDao.h"
#include "GVAR.h"
#include "BackgroundTrader.h"
#include <qsqlquery.h>
#include <qvariant.h>
#include <qstring.h>
#include <qdebug.h>

using std::set;
using std::shared_ptr;
using std::make_shared;

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

//获得投资者当日活跃的报单，系统初始化时调用
shared_ptr<set<shared_ptr<Order>>> OrderDao::getActivedOrders(const QString &id, const QString &date){
	QSqlQuery query(DATABASE);
	query.prepare("select * from orders where investor_id=:id and order_date=:date and order_status=:status ");
	query.bindValue(":id", id);
	query.bindValue(":date", date);
	query.bindValue(":status", "a");	//获取活跃的报单
	query.exec();
	auto orders = make_shared<set<shared_ptr<Order>>>();
	while (query.next()){
		QString &investor_id = query.value("investor_id").toString();
		QDate &date = query.value("order_date").toDate();
		int orderRef = query.value("order_ref").toInt();
		QString &strategy_id = query.value("strategy_id").toString();
		QString &instruction_id = query.value("instruction_id").toString();
		QString &system_id = query.value("system_id").toString();
		int sequence_num = query.value("sequence_num").toInt();
		QString	&instrument_id = query.value("instrument_id").toString();
		char direction = query.value("direction").toString().at(0).toLatin1();
		char open_close_flag = query.value("close_open_flag").toString().at(0).toLatin1();
		double price = query.value("price").toDouble();
		int original_volume = query.value("original_volume").toInt();
		int traded_volume = query.value("traded_volume").toInt();
		int rest_volume = query.value("rest_volume").toInt();
		char order_status = query.value("order_status").toString().at(0).toLatin1();
		//初始化对象并赋值
		auto activeOrder = make_shared<Order>();
		activeOrder->setInvestorId(investor_id);
		activeOrder->setDate(date);
		activeOrder->setOrderRef(orderRef);
		activeOrder->setStrategyId(strategy_id);
		activeOrder->setInstructionId(instruction_id);
		activeOrder->setSystemId(system_id);
		activeOrder->setSequenceId(sequence_num);
		activeOrder->setInstrumentId(instrument_id);
		activeOrder->setDirection(direction);
		activeOrder->setOpenCloseFlag(open_close_flag);
		activeOrder->setPrice(price);
		activeOrder->setOriginalVolume(original_volume);
		activeOrder->setTradedVolume(traded_volume);
		activeOrder->setRestVolume(rest_volume);
		activeOrder->setOrderStatus(order_status);
		//把新生成对象放入结果集中
		orders->insert(activeOrder);
	}
	return orders;
}

//获得投资者某日的最大报单引用，系统初始化时调用
int OrderDao::getMaximumOrderRef(const QString &id, const QString &date){
	//lock_guard<mutex> guard(m);
	QSqlQuery query;
	query.prepare(" select max(order_ref) from orders where investor_id=:id and order_date=:date ");
	query.bindValue(":id", id);
	query.bindValue(":date", date);
	query.exec();
	int result;
	//返回的结果只可能有一条
	if (query.next()){
		result = query.value(0).toInt();
	}
	else{
		//结果集为空，则最大报单引用为0
		result = 0;
	}
	return result;
}

//初始化过滤报单编号集合
void OrderDao::initOrderFilter(const QString &id, const QString &date,set<int> &orderFilter){
	QSqlQuery query;
	//从集合中搜出状态不为活跃的报单
	query.prepare(" select order_ref from orders where investor_id=:id and order_date=:date and "
		" order_status!=:status " );
	query.bindValue(":id", id);
	query.bindValue(":date", date);
	query.bindValue(":status", "a");
	query.exec();
	while (query.next()){
		orderFilter.insert(query.value(0).toInt());
	}
}

//更新系统中的报单表,如果不存在则插入，否则更新
void OrderDao::updateOrderTable(const std::shared_ptr<Order> &order){
	QSqlQuery exist(DATABASE);
	exist.prepare("select investor_id from orders where investor_id=:id and order_date=:date and order_ref=:ref ");
	exist.bindValue(":id", order->getInvestorId());
	exist.bindValue(":date", BackgroundTrader::getInstance()->getTradingDate());
	exist.bindValue(":ref", order->getOrderRef());
	exist.exec();
	//不存在则插入
	while (!exist.next()){
		QSqlQuery insert(DATABASE);
		insert.prepare("insert into orders (investor_id,order_ref,strategy_id,instruction_id,sequence_num,instrument_id, "
			" direction,close_open_flag,price,original_volume,traded_volume,rest_volume,order_status,order_date ) values "
			" (:id,:ref,:strategy_id,:instruction_id,:sequence_num,:instrument_id,:direction,:close_open_flag, "
			" :price,:original_volume,:traded_volume,:rest_volume,:order_status,:order_date) ");
		insert.bindValue(":id", order->getInvestorId());
		insert.bindValue(":ref", order->getOrderRef());
		insert.bindValue(":strategy_id", order->getStrategyId());
		insert.bindValue(":instruction_id", order->getInstructionId());
		insert.bindValue(":sequence_num", order->getSequenceId());
		insert.bindValue(":instrument_id", order->getInstrumentId());
		insert.bindValue(":direction", QString(order->getDirection()));
		insert.bindValue(":close_open_flag", QString(order->getOpenCloseFlag()));
		insert.bindValue(":price", order->getPrice());
		insert.bindValue(":original_volume", order->getOriginalVolume());
		insert.bindValue(":traded_volume", order->getTradedVolume());
		insert.bindValue(":rest_volume", order->getRestVolume());
		insert.bindValue(":order_status", QString(order->getOrderStatus()));
		insert.bindValue(":order_date", BackgroundTrader::getInstance()->getTradingDate());
		insert.exec();
		return;
	}
	//否则更新
	QSqlQuery updateOrder(DATABASE);
	updateOrder.prepare("update orders set order_date=:order_date,system_id=:system_id,sequence_num=:sequence,traded_volume=:traded,rest_volume=:rest, "
		" order_status=:status where investor_id=:id and order_date=:date and order_ref=:ref ");
	updateOrder.bindValue(":order_date", order->getDate());
	updateOrder.bindValue(":system_id", order->getSystemId());
	updateOrder.bindValue(":sequence", order->getSequenceId());
	updateOrder.bindValue(":traded", order->getTradedVolume());
	updateOrder.bindValue(":rest", order->getRestVolume());
	updateOrder.bindValue(":status", QString(order->getOrderStatus()));
	updateOrder.bindValue(":id", order->getInvestorId());
	updateOrder.bindValue(":date", order->getDate());
	updateOrder.bindValue(":ref", order->getOrderRef());
	//qDebug() <<"日期："<< order->getDate();
	//qDebug() <<"系统编号："<< order->getSystemId();
	//qDebug() <<"顺序编号："<< order->getSequenceId();
	//qDebug() <<"已交易数量："<< order->getTradedVolume();
	//qDebug() <<"剩余数量："<< order->getRestVolume();
	//qDebug() <<"报单状态："<< order->getOrderStatus();
	//qDebug() <<"投资者编号："<< order->getInvestorId();
	//qDebug() <<"报单引用："<< order->getOrderRef();
	updateOrder.exec();
}