#pragma once

#include "ThostFtdcMdApi.h"
#include "MDChannel.h"
#include <memory>
#include <vector>

class MDBroadcast :public QObject, public CThostFtdcMdSpi{
	Q_OBJECT
public:
	static std::shared_ptr<MDBroadcast> getInstance();
	~MDBroadcast();

	void subscribeInstruments(std::vector<QString> &instruments);
	void setChannel(std::shared_ptr<MDChannel> channel);
	bool isReadyToSubscribe();
signals:
	void broadcastData(CThostFtdcDepthMarketDataField *pDepthMarketData);
private:
	//当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用
	void OnFrontConnected() override;

	///登录请求响应
	void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, 
		int nRequestID, bool bIsLast) override;

	//行情回报响应
	void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData) override;
private:
	MDBroadcast();
	MDBroadcast(const MDBroadcast &) = delete;
	MDBroadcast & operator=(const MDBroadcast &) = delete;
private:
	//全局单例变量
	static std::shared_ptr<MDBroadcast>  broadcast;

	CThostFtdcMdApi *api;
	std::shared_ptr<MDChannel> channel = nullptr;
	bool readyToSubscribe = false;
}; 