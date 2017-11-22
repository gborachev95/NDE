#pragma once
#ifndef ClientWrapper_H
#define ClientWrapper_H
#include <stdint.h>
#include "NetworkData.h"

class NetworkClient;

class ClientWrapper
{
	NetworkClient* m_client;
public:
	ClientWrapper();
	~ClientWrapper();
	int Initialize(const char* address, uint16_t port);
	int Run();
	void Stop();
	int SendData(float _mouseX, uint8_t _actionsInput);

	// Getters
	void GetPlayerData(NetworkPlayers& _data);
	void GetItemData(NetworkObjects& _data);
	NetworkClient* GetClient();
	unsigned int GetClientID();

};

#endif