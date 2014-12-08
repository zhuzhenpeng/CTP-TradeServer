#pragma once

#include <qobject.h>
#include <qtcpserver.h>
#include <qtcpsocket.h>
#include <mutex>

class InstructionPort:public QObject{
	Q_OBJECT
public:
	explicit InstructionPort(QObject *parent = 0);
	void writeBackResult(QString result);
private slots:
	void newConnection();
	void readyToRead();
private:
	QTcpServer *server;
	QTcpSocket *socket = nullptr;
	std::mutex writeBackMutex;
};