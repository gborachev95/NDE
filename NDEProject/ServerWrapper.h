#pragma once
#ifndef ServerWrapper_H
#define ServerWrapper_H
#include "NetworkPlatform.h"

class NetworkServer;

class ServerWrapper
{
	NetworkServer *m_server;

public:
	ServerWrapper();
	~ServerWrapper();
	int  Initialize(const char* _address, uint16_t _port);
	int  Update();
	void Stop();
	unsigned int GetNumOfUsers();
};
#endif
