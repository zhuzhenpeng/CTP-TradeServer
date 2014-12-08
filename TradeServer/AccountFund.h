#pragma once

class AccountFund{
public:
	AccountFund() = default;
	AccountFund(const AccountFund&) = delete;
	AccountFund& operator=(const AccountFund&) = delete;

	//各私有变量的getter和setter
	void setPreBalance(const double &preBalance);
	const double & getPreBalance();

	void setDeposit(const double &deposit);
	const double & getDeposit();

	void setWithdraw(const double &withdraw);
	const double & getWithdraw();

	void setAvalible(const double &avalible);
	const double & getAvalible();

	void setCurrrentMargin(const double &currentMargin);
	const double & getCurrentMargin();

	void setFrozenMargin(const double &frozenMargin);
	const double & getFrozenMargin();

	void setCommission(const double &commission);
	const double & getCommission();

	void setCloseProfit(const double &closeProfit);
	const double & getCloseProfit();

	void setPositionProfit(const double &positionProfit);
	const double & getPositionProfit();
private:
	double preBalance = 0.0;		//上次结算准备金
	double deposit = 0.0;			//入金金额
	double withdraw = 0.0;		//出金金额
	double avalible = 0.0;		//可用资金
	double currentMargin = 0.0;	//当前保证金总额
	double frozenMargin = 0.0;	//冻结保证金
	double commission = 0.0;		//手续费
	double closeProfit = 0.0;		//平仓盈亏
	double positionProfit = 0.0;	//持仓盈亏
};