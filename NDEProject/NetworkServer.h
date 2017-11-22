#pragma once
#ifndef NetworkServer_H
#define NetworkServer_H
#include "NetworkPlatform.h"
#include "XTime.h"

class Graphics;
enum PLAYER_SOUND_ID
{
	RUNNER_IDLE_SOUND, RUNNER_DEATH_SOUND,
	MONSTER_IDLE_SOUND, MONSTER_CRAW_SOUND, MONSTER_POUNCE_SOUND,
	PUSHED_SOUND, FOOTSTEP_SOUND, JUMP_SOUND, SWOSH_SOUND, KEY_PICKUP,
	MAX_SOUND_PLAYER
};
enum SOUND_ID
{
	MENU_BACKGROUND_SOUND, GAME_BACKGROUND_SOUND, START_SCREECH_SOUND, START_GATE_MONSTER_SOUND,
	START_GATE_RUNNER_SOUND, COUNTDOWN_SOUND, BUTTON_CLICK_SOUND, LOSE_SOUND, WIN_SOUND, SOUND_MAX
};

class NetworkServer
{
	bool           m_active;
	SOCKET         m_serverSocket;
	sockaddr_in    m_playerAddress[MAX_USERS];
	uint8_t        m_currNumOfPlayers;
	unsigned int   m_playerTimer[MAX_USERS];
	NetworkPlayers m_gameData;
	NetworkObjects m_gameItems;
	uint32_t       m_playerScore[MAX_USERS];
	POINT          m_oldMousePos;
	XTime          m_timer;
	double         m_ableToUseItem;
	bool           m_start;
	bool           m_keypressed[MAX_USERS];
	bool           m_broadcast;
	bool           m_restartGame;
	bool           d_mouseTest;
	unsigned int   m_LevelIndex;
	unsigned int   m_runnersCageIndex;
	unsigned int   m_monsterCageIndex;
	
public:
	NetworkServer();
	~NetworkServer();
	int Initialize(const char* _address, uint16_t _port);
	int Update();
	void Stop();

private:
	int ParseMessage(sockaddr_in& _source, NetworkingPlatform::NetworkMessage& _message);
	int SendOkay(sockaddr_in& _destination);
	int SendFull(sockaddr_in& _destination);
	int SendState();
	void SendClose();
	void SendEndScreen();
	int SendServerMessage(sockaddr_in& _destination, NetworkingPlatform::NetworkMessage& _message);
	void ConnectClient(int _player, sockaddr_in& _source);
	void DisconnectClient(int _player);
	void UpdateState();
	void UpdateItems();
	unsigned int GetOpenIndex();
	void InitializePlayer(unsigned int _player);
	char* FindServer();
	void BroadcastServer(void *_param);
	void StopBroadcast();
	void RestartGame();
	void RestartPlayer(unsigned int _player);
	//bool ItemRespawnCheck();


	// Client state updates
	void Move_Player(unsigned int _player);
	void Jump_Player(unsigned int _player);
	bool PickUP_Runner(unsigned int _player);
	void DropItem_Runner(unsigned int _player);
	void Pounce_Monster(unsigned int _player);
	void Swipe_Monster(unsigned int _player);
	bool CheckWinCondition_Player(unsigned int _player);
	void Action_Runner(unsigned int _player);
	void Alternate_Action_Runner(unsigned int _player);
	void Collision_Player(unsigned int _player);
	void SetAnimations(int _player, int _anim);
	void SetSounds(int _player, int _sound);
	void LookAt(DirectX::XMFLOAT3 &_f, DirectX::XMFLOAT4X4 world, DirectX::XMFLOAT4X4 forward);
	void StartPlayer(unsigned int _player);
	void MoveStartGate(unsigned int _ID);
	void Move_Direction(DirectX::XMMATRIX& _world, unsigned int _row, float _speed, float _mult);

	void InitializeState();
};

#endif
