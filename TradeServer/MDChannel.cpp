#include "MDChannel.h"
#include <qbytearray.h>
#include <qdebug.h>
#include <qthread.h>

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

using std::shared_ptr;
using std::make_shared;

shared_ptr<MDChannel> MDChannel::channel = nullptr;

shared_ptr<MDChannel> MDChannel::getInstance(){
	if (channel != nullptr){
		return channel;
	}
	else{
		channel = shared_ptr<MDChannel>(new MDChannel());
		return channel;
	}
}

MDChannel::MDChannel(){
	server = new QTcpServer(this);
	connect(server, SIGNAL(newConnection()), this, SLOT(setSocket()));
	if (server->listen(QHostAddress::LocalHost, 9999)){
		qDebug() << "已开启本地行情服务器,端口号为:" << 9999;
	}
	else{
		qDebug() << "无法打开行情服务器";
		abort();
	}
}

//在获得行情数据之后向socket中写入行情
void MDChannel::writeToSocket(CThostFtdcDepthMarketDataField *data){
	if (socket != nullptr){
		qDebug() << "写入socket";
		QByteArray block;
		QDataStream out(&block, QIODevice::WriteOnly);
		out << data->InstrumentID;
		if (data->InstrumentID[5] == 0){
			out << ' ';
		}
		socket->write(block);
		socket->flush();
	}
}

//当有新的连接时，socket自动指向最新的连接
void MDChannel::setSocket(){
	if (socket != nullptr){
		socket->close();
	}
	socket = server->nextPendingConnection();
}