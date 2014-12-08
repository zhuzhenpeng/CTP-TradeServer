#include "AccountFund.h"

void AccountFund::setPreBalance(const double &preBalance){
	this->preBalance = preBalance;
}
const double & AccountFund::getPreBalance(){
	return preBalance;
}

void AccountFund::setDeposit(const double &deposit){
	this->deposit = deposit;
}
const double & AccountFund::getDeposit(){
	return deposit;
}

void AccountFund::setWithdraw(const double &withdraw){
	this->withdraw = withdraw;
}
const double & AccountFund::getWithdraw(){
	return withdraw;
}

void AccountFund::setAvalible(const double &avalible){
	this->avalible = avalible;
}
const double & AccountFund::getAvalible(){
	return avalible;
}

void AccountFund::setCurrrentMargin(const double &currentMargin){
	this->currentMargin = currentMargin;
}
const double & AccountFund::getCurrentMargin(){
	return currentMargin;
}

void AccountFund::setFrozenMargin(const double &frozenMargin){
	this->frozenMargin = frozenMargin;
}
const double & AccountFund::getFrozenMargin(){
	return frozenMargin;
}

void AccountFund::setCommission(const double &commission){
	this->commission = commission;
}
const double & AccountFund::getCommission(){
	return commission;
}

void AccountFund::setCloseProfit(const double &closeProfit){
	this->closeProfit = closeProfit;
}
const double & AccountFund::getCloseProfit(){
	return closeProfit;
}

void AccountFund::setPositionProfit(const double &positionProfit){
	this->positionProfit = positionProfit;
}
const double & AccountFund::getPositionProfit(){
	return positionProfit;
}