#pragma once

#include <qstring.h>
#include <qdatetime.h>
#include <qtimer.h>
#include "ThostFtdcUserApiStruct.h"

class Order:public QObject{
	Q_OBJECT
public:
	Order(QObject *parent = 0);
	//getter
	const QString& getInvestorId() const;
	const QString& getStrategyId() const;
	const QString& getInstructionId() const;
	const QDate& getDate() const;
	const int& getOrderRef() const;
	const QString& getSystemId() const;
	const int& getSequenceId() const;
	const QString& getInstrumentId() const;
	const char& getDirection() const;
	const char& getOpenCloseFlag() const;
	const double& getPrice() const;
	const int& getOriginalVolume() const;
	const int& getTradedVolume() const;
	const int& getRestVolume() const;
	const char& getOrderStatus() const;
	//setter
	void setInvestorId(const QString &);
	void setDate(const QDate &);
	void setOrderRef(const int &);
	void setStrategyId(const QString &);
	void setInstructionId(const QString &);
	void setSystemId(const QString &);
	void setSequenceId(const int &);
	void setInstrumentId(const QString &);
	void setDirection(const char &);
	void setOpenCloseFlag(const char &);
	void setPrice(const double &);
	void setOriginalVolume(const int &);
	void setTradedVolume(const int &);
	void setRestVolume(const int &);
	void setOrderStatus(const char &);
	//根据报单回报更新并执行相关动作
	void update(CThostFtdcOrderField *pOrder);
	//获得更新标志
	bool getUpdateFlag(){
		return updateFlag;
	}
	//重置更新标志
	void recoverUpdateFlag(){
		updateFlag = false;
	}
signals:
	void refreshTimer(int msec);
	void stopTimer();
private slots:
	void cancelThisOrder();
private:
	/*******************************属性***********************************************/
	//三个标志确定唯一的报单
	QString investorId;
	QDate date;
	int orderRef;

	QString strategyId;		//策略Id
	QString instructionId;	//指令Id

	QString systemId;	//由上交所返回来的，报单系统编号,撤单时可以使用
	int sequenceId = -1;	//报单最新顺序的编号

	//描述报单的信息
	QString instrumentId;
	char direction;		//'b'代表买入,'s'代表卖出
	/*	开仓 '0'
		平仓 '1'
		强平 '2'
		平今 '3'
		平昨 '4'
		强减 '5'
		本地强平 '6'	*/
	char openCloseFlag;	
	double price;		
	int originalVolume;
	int tradedVolume;
	int restVolume;
	/*	'f' 全部完成
		'a' 活跃状态
		'c' 撤单
		'w' 错单*/
	char orderStatus;
	/*******************************属性***********************************************/
	QTimer *timer;
	bool updateFlag = false;
};