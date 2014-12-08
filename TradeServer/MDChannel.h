#pragma once

#include "ThostFtdcUserApiStruct.h"

#include <qtcpserver.h>
#include <qtcpsocket.h>
#include <qobject.h>
#include <memory>

class MDChannel:public QObject{
	Q_OBJECT
public:
	static std::shared_ptr<MDChannel> getInstance();
public slots:
	void writeToSocket(CThostFtdcDepthMarketDataField *data);
private:
	MDChannel();
	MDChannel(const MDChannel &) = delete;
	MDChannel & operator=(const MDChannel &) = delete;
private slots:
	void setSocket();
private:
	//全局单例变量
	static std::shared_ptr<MDChannel> channel;

	QTcpServer *server;
	QTcpSocket *socket = nullptr;
};