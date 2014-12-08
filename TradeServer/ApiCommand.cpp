#include "ApiCommand.h"

ApiCommand::ApiCommand(int &requestID, CThostFtdcTraderApi *api):requestID(requestID){
	this->api = api;
	requestID++;
}

ApiCommand::~ApiCommand(){
	api = nullptr;
}