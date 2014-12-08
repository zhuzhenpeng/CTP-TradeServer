#include "MDBroadcast.h"
#include <qdebug.h>

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

using std::shared_ptr;
using std::make_shared;

shared_ptr<MDBroadcast> MDBroadcast::broadcast = nullptr;

shared_ptr<MDBroadcast> MDBroadcast::getInstance(){
	if (broadcast != nullptr){
		return broadcast;
	}
	else
	{
		broadcast = shared_ptr<MDBroadcast>(new MDBroadcast());
		return broadcast;
	}
}

MDBroadcast::~MDBroadcast(){
	if (api != nullptr){
		api->RegisterSpi(nullptr);
		api->Release();
		api = nullptr;
	}
}

void MDBroadcast::subscribeInstruments(std::vector<QString> &instruments){
	qDebug() << "正在订阅合约:";
	int count = instruments.size();
	char* *allInstruments = new char*[count];
	for (int i = 0; i < count; i++){
		allInstruments[i] = new char[7];
		strcpy(allInstruments[i], instruments.at(i).toStdString().c_str());
		qDebug() << allInstruments[i];
	}
	api->SubscribeMarketData(allInstruments, count);
}

bool MDBroadcast::isReadyToSubscribe(){
	return readyToSubscribe;
}

void MDBroadcast::setChannel(shared_ptr<MDChannel> channel){
	this->channel = channel;
	connect(this, SIGNAL(broadcastData(CThostFtdcDepthMarketDataField *)), channel.get(), SLOT(writeToSocket(CThostFtdcDepthMarketDataField*)));
}

/******************************私有辅助函数***********************************************/

MDBroadcast::MDBroadcast(){
	api = CThostFtdcMdApi::CreateFtdcMdApi("./spi_con/");
	api->RegisterSpi(this);
	api->RegisterFront("tcp://asp-sim2-md1.financial-trading-platform.com:26213");	 //模拟盘
	//api->RegisterFront("tcp://180.169.75.19:41213");								//实盘
	api->Init();
}


/*****************************spi回调函数\***********************************************/

//当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用
void MDBroadcast::OnFrontConnected() {
	//连接上服务器之后自动请求登陆
	qDebug() << "正在请求登陆行情服务器...";
	CThostFtdcReqUserLoginField loginField;
	strcpy(loginField.BrokerID, "");
	strcpy(loginField.UserID, "");
	strcpy(loginField.Password, "");
	api->ReqUserLogin(&loginField, 0);
}

///登录请求响应
void MDBroadcast::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo,
	int nRequestID, bool bIsLast) {
	if (pRspInfo->ErrorID == 0){
		qDebug() << "已登陆行情服务器，正在打开本地行情服务器...";
		qDebug() << "进行相关行情订阅...";
		readyToSubscribe = true;
	}
	else{
		qDebug() << "登录失败:" << pRspInfo->ErrorID << " " << pRspInfo->ErrorMsg;
		abort();
	}
}

//行情回报响应
void MDBroadcast::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData){
	//qDebug() << "收到" << pDepthMarketData->InstrumentID << "行情";
	emit broadcastData(pDepthMarketData);
	//qDebug() << pDepthMarketData->InstrumentID[0];
	//qDebug() << pDepthMarketData->InstrumentID[1];
	//qDebug() << pDepthMarketData->InstrumentID[2];
	//qDebug() << pDepthMarketData->InstrumentID[3];
	//qDebug() << pDepthMarketData->InstrumentID[4];
	//qDebug() << pDepthMarketData->InstrumentID[5];
}