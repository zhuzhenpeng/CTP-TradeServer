#include "AccountID.h"

void AccountID::setInvestorID(const QString &investorID){
	this->investorID = investorID;
}

void AccountID::setPassword(const QString &password){
	this->password = password;
}

void AccountID::setBrokerID(const QString &brokerID){
	this->brokerID = brokerID;
}

void AccountID::setFrontAddress(const QString &frontAddress){
	this->frontAddress = frontAddress;
}

const QString &AccountID::getInvestorID() const{
	return investorID;
}

const QString &AccountID::getPassword() const{
	return password;
}

const QString &AccountID::getBrokerID() const{
	return brokerID;
}

const QString &AccountID::getFrontAddress() const{
	return frontAddress;
}