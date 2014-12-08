#pragma once

#include "ThostFtdcTraderApi.h"

//封装api命令的接口

class ApiCommand{
public:
	virtual ~ApiCommand();
	virtual int execute() = 0;
protected:
	ApiCommand(int &requestID, CThostFtdcTraderApi *api);
	int &requestID;
	CThostFtdcTraderApi *api;
};