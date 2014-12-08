#include "Order.h"
#include "GVAR.h"
#include <qcoreapplication.h>

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

Order::Order(QObject *parent) :QObject(parent){
	timer = new QTimer(this);
	timer->setSingleShot(true);
	connect(this, SIGNAL(refreshTimer(int)), timer, SLOT(start(int)));
	connect(this, SIGNAL(stopTimer()), timer, SLOT(stop()));
	connect(timer, SIGNAL(timeout()), this, SLOT(cancelThisOrder()));
}

//getter
const QString& Order::getInvestorId() const{
	return investorId;
}
const QString& Order::getStrategyId() const{
	return strategyId;
}
const QString& Order::getInstructionId() const{
	return instructionId;
}
const QDate& Order::getDate() const{
	return date;
}
const int& Order::getOrderRef() const{
	return orderRef;
}
const QString& Order::getSystemId() const{
	return systemId;
}
const int& Order::getSequenceId() const{
	return sequenceId;
}
const QString& Order::getInstrumentId() const{
	return instrumentId;
}
const char& Order::getDirection() const{
	return direction;
}
const char& Order::getOpenCloseFlag() const{
	return openCloseFlag;
}
const double& Order::getPrice() const{
	return price;
}
const int& Order::getOriginalVolume() const{
	return originalVolume;
}
const int& Order::getTradedVolume() const{
	return tradedVolume;
}
const int& Order::getRestVolume() const{
	return restVolume;
}
const char& Order::getOrderStatus() const{
	return orderStatus;
}
//setter方法
void Order::setInvestorId(const QString &id){
	investorId = id;
}
void Order::setDate(const QDate &date){
	this->date = date;
}
void Order::setOrderRef(const int &orderRef){
	this->orderRef = orderRef;
}
void Order::setStrategyId(const QString &id){
	strategyId = id;
}
void Order::setInstructionId(const QString &id){
	instructionId = id;
}
void Order::setSystemId(const QString &id){
	systemId = id;
}
void Order::setSequenceId(const int &id){
	sequenceId = id;
}
void Order::setInstrumentId(const QString &id){
	instrumentId = id;
}
void Order::setDirection(const char &direction){
	this->direction = direction;
}
void Order::setOpenCloseFlag(const char &flag){
	openCloseFlag = flag;
}
void Order::setPrice(const double &price){
	this->price = price;
}
void Order::setOriginalVolume(const int &volume){
	originalVolume = volume;
}
void Order::setTradedVolume(const int &volume){
	tradedVolume = volume;
}
void Order::setRestVolume(const int &volume){
	restVolume = volume;
}
void Order::setOrderStatus(const char &status){
	orderStatus = status;
}

//根据报单回报更新并执行相关动作
void Order::update(CThostFtdcOrderField *pOrder){
	if (pOrder != nullptr && pOrder->SequenceNo >= sequenceId){
		date = QDate::fromString(pOrder->InsertDate, "yyyyMMdd");
		systemId = QString(pOrder->OrderSysID);
		sequenceId = pOrder->SequenceNo;
		tradedVolume = pOrder->VolumeTraded;
		restVolume = pOrder->VolumeTotal;
		if (pOrder->OrderStatus == '0'){
			emit stopTimer();
			orderStatus = 'f';
		}
		else if (pOrder->OrderStatus == '5' && systemId.isEmpty()){
			emit stopTimer();
			orderStatus = 'w';
		}
		else if (pOrder->OrderStatus == '5' && !systemId.isEmpty()){
			emit stopTimer();
			orderStatus = 'c';
		}
		else if (pOrder->OrderStatus == '1' || pOrder->OrderStatus == '3'){
			emit refreshTimer(3000);
			orderStatus = 'a';
		}
		updateFlag = true;
	}
}

//当计时器结束时，报单自动取消自己
void Order::cancelThisOrder(){
	auto &trader = TRADERS[investorId];
	trader->cancleOrder(this);
}