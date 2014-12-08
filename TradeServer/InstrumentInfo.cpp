#include "InstrumentInfo.h"

void InstrumentInfo::setId(const QString &id){
	this->id = id;
}
void InstrumentInfo::setName(const QString &name){
	this->name = name;
}
void InstrumentInfo::setExchangeId(const QString &exchangeId){
	this->exchangeId = exchangeId;
}
void InstrumentInfo::setDeadline(const QDate &deadline){
	this->deadline = deadline;
}
void InstrumentInfo::setMarginRate(const double &marginRate){
	this->marginRate = marginRate;
}
void InstrumentInfo::setMutiplier(const int &multiplier){
	this->multiplier = multiplier;
}
void InstrumentInfo::setOpenCommission(const double &commission){
	this->openCommission = commission;
}
void InstrumentInfo::setCloseCommission(const double &commission){
	this->closeCommission = commission;
}
void InstrumentInfo::setCloseTodayCommission(const double &commission){
	this->closeTodayCommission = commission;
}
void InstrumentInfo::setMinimumUnit(const double &minimumUnit){
	this->minimumUnit = minimumUnit;
}
void InstrumentInfo::setTradable(const bool &tradable){
	this->tradable = tradable;
}


const QString & InstrumentInfo::getId() const{
	return id;
}
const QString & InstrumentInfo::getName() const{
	return name;
}
const QString & InstrumentInfo::getExchangeId() const{
	return exchangeId;
}
const QDate & InstrumentInfo::getDeadline() const{
	return deadline;
}
const double & InstrumentInfo::getMarginRate() const{
	return marginRate;
}
const int & InstrumentInfo::getMultiplier() const{
	return multiplier;
}
const double & InstrumentInfo::getOpenCommission() const{
	return openCommission;
}
const double & InstrumentInfo::getCloseCommission() const{
	return closeCommission;
}
const double & InstrumentInfo::getCloseTodayCommission() const{
	return closeTodayCommission;
}
const double & InstrumentInfo::getMinimumUnit() const{
	return minimumUnit;
}
const bool & InstrumentInfo::isTradable() const{
	return tradable;
}

//为了放在set集合中，重载 < 运算符
bool InstrumentInfo::operator<(const InstrumentInfo &i){
	return id < i.getId();
}