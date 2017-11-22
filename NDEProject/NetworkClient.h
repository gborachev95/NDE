#pragma once
#ifndef NetworkingClient_H
#define NetworkingClient_H
#include "NetworkPlatform.h"

class PlayState;

class NetworkClient
{
	SOCKET         m_clientSocket;
	uint8_t        m_player;
	unsigned int   m_ticTimer;
	NetworkPlayers m_gameData;
	NetworkObjects m_gameItems;
	uint32_t       m_playerScore[MAX_USERS];
	fd_set         m_ListenersSockets;
	

public:
	NetworkClient();
	~NetworkClient();

    int  Initialize(const char* _address, uint16_t _port);
	int  Run();
	void Stop();
	int  SendData(float _mouseX, uint8_t _actionsInput);

	// Getters
	uint8_t GetID() { return m_player; }
	bool GetActive() { return CHECK_BIT(m_gameData.m_active,m_player); }
	void GetPlayerData(NetworkPlayers& _gameData);
	void GetItemData(NetworkObjects& _gameData);
	// Setters
	void SetActive(bool _active)
	{
		if (_active)
			m_gameData.m_active |= (1 << m_player);
		else
			m_gameData.m_active &= ~(1 << m_player);
	}
private:
	int  SendAlive();
	void SendClose();
	void Update();
	void InitializeState();
	void PlaySounds();
	char* FindServer();
};
#endif
