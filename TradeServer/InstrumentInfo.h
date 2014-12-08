#pragma once

#include <qstring.h>
#include <qdatetime.h>

//合约信息：id、名字、交易所编号、最后交割日、保证金率、合约乘数、保证金率、手续费、最小变动单位、现在是否可以交易
class InstrumentInfo{
public:
	InstrumentInfo() = default;
	InstrumentInfo(const InstrumentInfo&) = delete;
	InstrumentInfo& operator=(const InstrumentInfo&) = delete;
	void setId(const QString &id);
	void setName(const QString &name);
	void setExchangeId(const QString &exchangeId);
	void setDeadline(const QDate &deadline);
	void setMarginRate(const double &marginRate);
	void setMutiplier(const int &multiplier);
	void setOpenCommission(const double &commission);
	void setCloseCommission(const double &commission);
	void setCloseTodayCommission(const double &commission);
	void setMinimumUnit(const double &minimumUnit);
	void setTradable(const bool &tradable);

	const QString & getId() const;
	const QString & getName() const;
	const QString & getExchangeId() const;
	const QDate & getDeadline() const;
	const double & getMarginRate() const;
	const int & getMultiplier() const;
	const double & getOpenCommission() const;
	const double & getCloseCommission() const;
	const double & getCloseTodayCommission() const;
	const double & getMinimumUnit() const;
	const bool & isTradable() const;

	//为了放在set集合中，重载 < 运算符
	bool operator<(const InstrumentInfo &i);
private:
	QString id;
	QString name;
	QString exchangeId;
	QDate deadline;
	double marginRate;
	int multiplier;
	double openCommission;
	double closeCommission;
	double closeTodayCommission;
	double minimumUnit;
	bool tradable;
};