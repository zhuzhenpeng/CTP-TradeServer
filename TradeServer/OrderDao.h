#pragma once

#include "Order.h"
//#include <mutex>
#include <set>
#include <memory>

class OrderDao{
public:
	//获得投资者当日活跃的报单，系统初始化时调用
	std::shared_ptr<std::set<std::shared_ptr<Order>>> getActivedOrders(const QString &id,const QString &date);
	//获得投资者某日的最大报单引用，系统初始化时调用
	int getMaximumOrderRef(const QString &id, const QString &date);
	//初始化过滤报单编号集合
	void initOrderFilter(const QString &id, const QString &date, std::set<int> &orderFilter);
	//更新系统中的报单表,如果不存在则插入，否则更新
	void updateOrderTable(const std::shared_ptr<Order> &order);
};