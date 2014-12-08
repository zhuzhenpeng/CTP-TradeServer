#pragma once

#include <qstring.h>

//记录账户的基本信息：用户ID，交易账号，账号密码，经纪商代码，前置机地址
class AccountID{
public:
	AccountID() = default;
	AccountID(const AccountID&) = delete;
	AccountID& operator=(const AccountID&) = delete;
	void setInvestorID(const QString &investorID);
	void setPassword(const QString &password);
	void setBrokerID(const QString &brokerID);
	void setFrontAddress(const QString &frontAddress);
	const QString &getInvestorID() const;
	const QString &getPassword() const;
	const QString &getBrokerID() const;
	const QString &getFrontAddress() const;
private:
	QString investorID;
	QString password;
	QString brokerID;
	QString frontAddress;
};