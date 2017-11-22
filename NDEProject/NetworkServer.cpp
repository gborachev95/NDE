#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "NetworkServer.h"
#include "PlayState.h"
#include "Graphics.h"
#include "ObjManager.h"
#include "Player.h"
#include "Log.h"
#include "XTime.h"
#include <iostream>
#include "Physics.h"
#include "Collision.h"
#include "Runner.h"
#include "Monster.h"
#include "BaseItem.h"
#include "PocketSand.h"
#include "MagnetGranade.h"
#include "BlastGrenade.h"
#include "Trap.h"
#include <DirectXColors.h>
#include "Particle.h"

// Defines
#define ALLPLAYS 0
#define NOT_IN_CAGE 0

// Globals
//SOCKET        g_broadcastSocket;

const float GAME_END = 120.0f;
int DEBUGGING = 0;

void testfunc(void *_param)
{
	//int res = -27;
	//
	//// Create broadcast address
	//sockaddr_in sockAddress;
	//ZeroMemory(&sockAddress, sizeof(sockAddress));
	//sockAddress.sin_family = AF_INET;
	//sockAddress.sin_port = htons(32327);
	//sockAddress.sin_addr.s_addr = inet_addr("10.63.63.255");
	//
	//// Bind the socket.
	////res = bind(g_broadcastSocket, (SOCKADDR *)&sockAddress, sizeof(sockaddr_in));
	//char broadcast = 'a';
	//res = setsockopt(m_serverSocket, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
	//while (1/*m_gameData.m_gameTime <= 0.0f*/) // || m_broadcast
	//{
	//	res = sendto(m_serverSocket, (char*)(_param), strlen((char*)(_param)), 0, (sockaddr *)&sockAddress, sizeof(sockAddress));
	//	Sleep(3000);
	//}
}

using namespace NetworkingPlatform;

// Constructor
NetworkServer::NetworkServer()
{
}

// Destructor
NetworkServer::~NetworkServer()
{
}

// Initializes the server. (NOTE: Does not wait for player connections!)
int NetworkServer::Initialize(const char* _address, uint16_t _port)
{
	// Sets up the console
#if DEBUGGING
	LogSetUp(L"NDE");
	system("CLS");
	//printf("STARTING SERVER\n");
#endif
	// The passed in address
	//char* useAddress = const_cast<char*>(_address);

	// If no address was passed check for local
	//if (useAddress[0] == '\0')
	//	useAddress = FindServer();
	// If no local, exit
	//if (useAddress[0] == '\0')
	//	return NETWORK_DISCONNECT;

	InitializeState();
	m_gameData.m_gameTime = 0.0f; 

	//Create a socket
	m_serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (m_serverSocket == INVALID_SOCKET)
	{
		Stop();
		return NETWORK_DISCONNECT;
	}

	sockaddr_in   serverAddr;
	ZeroMemory(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(_address);
	serverAddr.sin_port = htons(_port);
	
	// Makes it non-blocking
	unsigned long mode = 1;
	ioctlsocket(m_serverSocket, FIONBIO, &mode);
	
	// Bind the socket.
	if (bind(m_serverSocket, (SOCKADDR *)&serverAddr, sizeof(sockaddr_in)) == SOCKET_ERROR)
	{
		Stop();
		return NETWORK_BIND_ERROR;
	}
	//if (useAddress != _address)
	//_beginthread(testfunc, 0, useAddress);

	// Mark the server as active.
	m_active = true;
	return NETWORK_SUCCESS;
}

// Initializes the player data for a current _player
void NetworkServer::InitializePlayer(unsigned int _player)
{
#if DEBUGGING
	printf("Clearing player %d\n", _player);
#endif
	m_keypressed[_player] = false;
	m_playerAddress[_player].sin_addr.s_addr = INADDR_NONE;
	m_playerAddress[_player].sin_port = 0;
	m_playerScore[_player] = 0;

	// Game data initialization
	m_gameData.m_active &= ~(1 << _player);
	m_gameData.m_checkWin &= ~(1 << _player);
	m_gameData.m_actions[_player] = false;
	m_gameData.m_effectApplied[_player] = false;
	m_gameData.m_animation[_player] = false;
	m_gameData.m_animation[_player] |= (1 << IDLE_ANIM);
	SetSounds(_player, RUNNER_IDLE_SOUND);
	m_gameData.m_playerMouseX[_player] = 0.0f;

	XMStoreFloat4x4(&m_gameData.m_worldMatrix[_player], XMMatrixIdentity());

#if ALLPLAYS 
	m_gameData.m_worldMatrix[_player]._41 = (_player*7.0f);
#else
	m_gameData.m_worldMatrix[_player]._41 = -1000;
#endif
	m_gameData.m_worldMatrix[_player]._42 = 0.0f;
	m_gameData.m_worldMatrix[_player]._43 = 0.0f;
}

// Initializes the state of the game.
void NetworkServer::InitializeState()
{
	d_mouseTest = false;
	// Initializing speed indeces
	m_LevelIndex = ObjManager::GetObjects().size() - 1;
	m_monsterCageIndex = ObjManager::GetObjects().size() - 2;
	m_runnersCageIndex = ObjManager::GetObjects().size() - 3;

	// Initializing game variables
	m_start = false;
	m_currNumOfPlayers = 0;
	m_ableToUseItem = 0.0f;
	m_restartGame = false;

	ObjManager::RefreshObjects();
	for (unsigned int i = 0; i < MAX_USERS; ++i)
		InitializePlayer(i);

	// Soft initialize of items
	for (unsigned int i = 0; i < MAX_OBJECT; ++i)
	{
		m_gameItems.m_active |= (1 << i);
		m_gameItems.m_particleEffect &= ~(1 << i);
		m_gameItems.m_carried[i] = false;
		XMStoreFloat4x4(&m_gameItems.m_worldMatrix[i], XMMatrixIdentity());
	}


	for (size_t i = 9; i < 17; i++)
		m_gameItems.m_active &= ~(1 << i);

	// Initializing boxes positions
	XMFLOAT3 pos[8] = {
	{ -5.743f, 13.609f, -87.756f },
	{ -53.823f, -1.927f, 55.884f },
	{ 52.674f, -11.599f, 56.941f },
	{ -105.937f, -5.220f, -23.900f },
	{ -36.020f, 37.356f, 21.716f },
	{ -15.323f, 48.978f, -28.443f },
	{ 65.806f, -11.997f, -100.676f },
	{ 54.406f, 41.691f, 50.307f } };

	for (size_t i = 1; i < 9; i++)
	{
		m_gameItems.m_worldMatrix[i]._41 = pos[i - 1].x;
		m_gameItems.m_worldMatrix[i]._42 = pos[i - 1].y;
		m_gameItems.m_worldMatrix[i]._43 = pos[i - 1].z;
	}

	// Gates
	ObjManager::AccessObject(m_monsterCageIndex).ResetTris();
	ObjManager::AccessObject(m_monsterCageIndex).SetWorldMatrix(XMMatrixIdentity());
	ObjManager::AccessObject(m_runnersCageIndex).ResetTris();
	ObjManager::AccessObject(m_runnersCageIndex).SetWorldMatrix(XMMatrixIdentity());
}

// Stops the server.
void NetworkServer::Stop()
{
	// Sends a "close" message to each client.
	SendClose();
	// Shuts down the server gracefully (update method should exit with SHUTDOWN code.)
	shutdown(m_serverSocket, SD_BOTH);
	m_active = false;
#if DEBUGGING
	printf("DEBUG::SERVER SHUTTING DOWN\n");
#endif
}

// Parses a message and responds if necessary. (private, suggested)
int NetworkServer::ParseMessage(sockaddr_in& _source, NetworkMessage& _message)
{
	// Parse a message from client "source."
	uint8_t msgType = _message.ReadByte();

	switch (msgType)
	{
	case  NETWORK_CL_CONNECT:
	{
		if (SendOkay(_source) == NETWORK_SUCCESS)
		{
			unsigned int player = GetOpenIndex();
			if (GetOpenIndex() < MAX_USERS)
				ConnectClient(player, _source);
		}
		else
			SendFull(_source);
	
		break;
	}
	case  NETWORK_CL_DATA:
	{
		for (unsigned int i = 0; i < m_currNumOfPlayers; ++i)
		{
			if ((m_playerAddress[i].sin_addr.s_addr == _source.sin_addr.s_addr) && (m_playerAddress[i].sin_port == _source.sin_port))
			{
				// Read Player data
				m_gameData.m_actions[i] = _message.ReadByte();
				m_gameData.m_playerMouseX[i] = _message.ReadFloat();
				break;
			}
		}
		break;
	}
	case  NETWORK_CL_ALIVE:
	{
#if 1
		for (unsigned int i = 0; i < m_currNumOfPlayers; ++i)
		{
			if ((m_playerAddress[i].sin_addr.s_addr == _source.sin_addr.s_addr) && (m_playerAddress[i].sin_port == _source.sin_port))
			{
				m_playerTimer[i] = 0;
				break;
			}
		}
#endif
		break;
	}
	case  NETWORK_SV_CL_CLOSE:
	{
		unsigned int indexPlayer = 0;
		for ( indexPlayer = 0; indexPlayer < MAX_USERS; ++indexPlayer)
			if ((m_playerAddress[indexPlayer].sin_addr.s_addr == _source.sin_addr.s_addr) && (m_playerAddress[indexPlayer].sin_port == _source.sin_port))
				break;

		// If it is the server send close to all clients
		if (indexPlayer == 0)
		{
#if DEBUGGING
			printf("DEBUG::HOST LEFT\n");
#endif
			SendClose();
			return NETWORK_SHUTDOWN;
		}
		else
			DisconnectClient(indexPlayer);

		break;
	}
	}

	return NETWORK_SUCCESS;
}

// Sends the "NETWORK_SV_OKAY" message to destination. (private, suggested)
int NetworkServer::SendOkay(sockaddr_in& _destination)
{
	NetworkMessage msg(NETWORK_OUTPUT);
	if (m_currNumOfPlayers < MAX_USERS)
	{
		msg.WriteByte(NETWORK_SV_OKAY);
		msg.WriteByte(m_currNumOfPlayers);
		SendServerMessage(_destination, msg);
	}
	else
		return NETWORK_SV_FULL;

	return NETWORK_SUCCESS;
}

// Sends the "NETWORK_SV_FULL" message to destination. (private, suggested)
int NetworkServer::SendFull(sockaddr_in& _destination)
{
	NetworkMessage msg(NETWORK_OUTPUT);
	if (m_currNumOfPlayers >= MAX_USERS)
	{
		msg.WriteByte(NETWORK_SV_FULL);
		SendServerMessage(_destination, msg);
	}
	else

		return NETWORK_SV_FULL;

	return NETWORK_SUCCESS;
}

// Sends the current snapshot to all players. (private, suggested)
int NetworkServer::SendState()
{
	unsigned int i = 0;
	NetworkMessage msg(NETWORK_OUTPUT);
	msg.WriteByte(NETWORK_SV_SNAPSHOT);

	// Writing the players data
	for (i = 0; i < MAX_USERS; ++i)
		msg.WriteGameData(m_gameData, i);

	// Writing items data 
	msg.WriteObjectGameData(m_gameItems);

	// Send the game state to each client.
	for (i = 0; i < m_currNumOfPlayers; ++i)
	{
#if DEBUGGING
		printf("%d\n", msg.BytesAvailable());
#endif
		SendServerMessage(m_playerAddress[i], msg);
	}

	// TEST: Not needed. If taken out, add it to when score is refreshed.
	SendEndScreen();

	return NETWORK_SUCCESS;
}

// Sends the "NETWORK_SV_CL_CLOSE" message to all clients. (private, suggested)
void NetworkServer::SendClose()
{
	// TODO:: FIX THIS. WHEN CLIENT EXITS THE SERVER SHUTS DOWN
	for (unsigned int i = 0; i < m_currNumOfPlayers; ++i)
	{
		NetworkMessage message(NETWORK_OUTPUT);
		message.WriteByte(NETWORK_SV_CL_CLOSE);
		sendtoNetMessage(m_serverSocket, message, &m_playerAddress[i]);
	}
#if DEBUGGING
	printf("DEBUG::SENDING CLOSE MESSAGES\n");
#endif
}

// Sends the "NETWORK_SV_END_DATA"
void NetworkServer::SendEndScreen()
{
	unsigned int i = 0;
	NetworkMessage msg(NETWORK_OUTPUT);
	msg.WriteByte(NETWORK_SV_END_DATA);

	// Writing the score
	for (i = 0; i < MAX_USERS; ++i)
		msg.WriteInt(m_playerScore[i]);

	// Send the game state to each client.
	for (i = 0; i < m_currNumOfPlayers; ++i)
		SendServerMessage(m_playerAddress[i], msg);
}

// Server message-sending helper method. (private, suggested)
int NetworkServer::SendServerMessage(sockaddr_in& _destination, NetworkMessage& _message)
{
	sendtoNetMessage(m_serverSocket, _message, (sockaddr_in*)(&_destination));
	return NETWORK_SUCCESS;
}

// Marks a client as connected and adjusts the game state.
void NetworkServer::ConnectClient(int _player, sockaddr_in& _source)
{
	m_gameData.m_active |= (1 << _player);
	m_playerAddress[_player] = _source;
	m_playerTimer[_player] = 0;
	StartPlayer(_player);
#if DEBUGGING
	printf("DEBUG::CLIENT %d CONNECTED\n", _player);
#endif
	++m_currNumOfPlayers;
}

// Marks a client as disconnected and adjusts the game state.
void NetworkServer::DisconnectClient(int _player)
{
	InitializePlayer(_player);
	ObjManager::RefreshObject(_player);
#if DEBUGGING
	printf("DEBUG::CLIENT %d LEFT\n",_player);
#endif
	--m_currNumOfPlayers;
	//if (!CHECK_BIT(m_gameData.m_active, i))

	//if (m_currNumOfPlayers == 1)
	//	m_checkWin = false;
}

// Gets an open index for a player that is trying to connect
unsigned int NetworkServer::GetOpenIndex()
{
	for (unsigned int i = 0; i < MAX_USERS; ++i)
	{
		if (!CHECK_BIT(m_gameData.m_active, i))
			return i;
	}
	return 777;
}

// Gets the local IP address
char* NetworkServer::FindServer()
{
	char * szLocalIP = "";

	// Gets the name of the host
	char szHostName[255];
	gethostname(szHostName, 255);

	// Gets the host data
	struct hostent *host_entry = nullptr;
	host_entry = gethostbyname(szHostName);
	// Gets the IP address from the 
	if (host_entry->h_length > 0)
		szLocalIP = inet_ntoa(*(struct in_addr*)*host_entry->h_addr_list);

	return szLocalIP;
}

// Broadcasting server, helper
void NetworkServer::BroadcastServer(void *_param)
{
	//int res = -27;
	//m_broadcastSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	//
	//// Create broadcast address
	//sockaddr_in sockAddress;
	//ZeroMemory(&sockAddress, sizeof(sockAddress));
	//sockAddress.sin_family = AF_INET;
	//sockAddress.sin_port = htons(32327);
	//sockAddress.sin_addr.s_addr = inet_addr("255.255.255.255");
	//
	//// Bind the socket.
	//res = bind(m_broadcastSocket, (SOCKADDR *)&sockAddress, sizeof(sockaddr_in));
	//
	//while (m_gameData.m_gameTime <= 0.0f) // || m_broadcast
	//{
	//	res = sendto(m_broadcastSocket, (char*)(_param), strlen((char*)(_param)), 0, (sockaddr *)&sockAddress, sizeof(sockAddress));
	//	Sleep(3000);
	//}
	//
	//closesocket(m_broadcastSocket);
}

// Update the server
int NetworkServer::Update()
{
	m_timer.Signal();
	int result = NETWORK_SUCCESS;

	if (!m_active)
		return NETWORK_SHUTDOWN;

	// Allocate a socket address to store the source of messages.
	sockaddr_in msgAddress;
	NetworkMessage clientMsg(NETWORK_INPUT);

	// Get player input and process it.
	if (recvfromNetMessage(m_serverSocket, clientMsg, &msgAddress) <= 0)
	{
		int lastError = WSAGetLastError();
		if (lastError != WSAEWOULDBLOCK && lastError != WSAECONNRESET)
		{
#if DEBUGGING
			printf("DEBUG::EWOULDBLOCK FAILED");
#endif
			Stop();
			return NETWORK_DISCONNECT;
		}
	}
	else
		result = ParseMessage(msgAddress, clientMsg);

#if 0
	// Check for connection
	for (unsigned int i = 0; i < m_currNumOfPlayers; ++i)
	{
	// If any player's timer exceeds, "disconnect" the player.
	if (m_playerTimer[i] > 50)
		DisconnectClient(i);
	if (m_playerAddress[0].sin_addr.s_addr == INADDR_NONE)
		return NETWORK_SHUTDOWN;
	}
#endif

	// Update the state and send the current snapshot to each player.
	RestartGame();
	UpdateState();
	SendState();

	

	if (Graphics::single_keyboard->GetState().B)
		++m_playerScore[0];
	else if (Graphics::single_keyboard->GetState().N)
		--m_playerScore[0];
	return result;
}

// Updates the state of the game.
void NetworkServer::UpdateState()
{
	m_ableToUseItem += m_timer.Delta();
	for (unsigned int i = 0; i < m_currNumOfPlayers; ++i)
		++m_playerTimer[i];
	
	// THIS NEEDS TO GO AWAY
	if (Graphics::single_keyboard->GetState().Enter)
		m_start = true;
	if (Graphics::single_keyboard->GetState().D8)
	{
		for (unsigned int i = 0; i < MAX_USERS; ++i)
			m_playerScore[i] = 0;
	}
	
	if (m_start)
	{
		m_gameData.m_gameTime += (float)m_timer.Delta();

		if (m_gameData.m_gameTime < 10.0f && m_gameData.m_gameTime > 5.0f)
			MoveStartGate(RUNNERSTART_ID);
		else if (m_gameData.m_gameTime < 17.0f && m_gameData.m_gameTime > 12.0f)
			MoveStartGate(MONSTERSTART_ID);
	}

	for (unsigned int i = 0; i < MAX_USERS; ++i)
	{
#if ALLPLAYS
#else
		if (CHECK_BIT(m_gameData.m_active,i))
		{
#endif
			if (ObjManager::AccessObject(i).GetID() == RUNNER_ID || ObjManager::AccessObject(i).GetID() == MONSTER_ID)
			{
				Player* player = dynamic_cast<Player*>(&ObjManager::AccessObject(i));

				m_gameData.m_effectApplied[i] &= ~(1 << FLASH_EFFECT);
				
				if (m_gameData.m_animation[i] & (1 << GETTING_UP_ANIM) || (m_gameData.m_animation[i] & (1 << KICK_ANIM)))
					if (ObjManager::AccessObject(i).GetAnimBusy() == false)
						player->SetIsStunned(false);
				
				if ((m_gameData.m_animation[i] & (1 << TRIPPING_ANIM)) && ObjManager::AccessObject(i).GetID() == RUNNER_ID)
					if (ObjManager::AccessObject(i).GetAnimBusy() == false)
					{
						SetAnimations(i, GETTING_UP_ANIM);
						ObjManager::AccessObject(i).GetAnimations()[GETTING_UP_ANIM]->SetTime(5);
						ObjManager::AccessObject(i).SetAnimBusy(true);
					}
				
				BaseObject* level = ObjManager::GetObjects()[ObjManager::GetObjects().size() - 1];
				unsigned int animOut = 100;
				Physics::ApplyForce(&ObjManager::AccessObject(i), m_gameData.m_worldMatrix[i], float(m_timer.Delta()), level, animOut);
				
				if (animOut < 100)
					SetAnimations(i, animOut);

				if (!player->GetIsStunned())
				{
					Move_Player(i);
					Action_Runner(i);
					Alternate_Action_Runner(i);
				}

				Collision_Player(i);
				
				if (!player->GetIsStunned())
				{
					Pounce_Monster(i);
					Swipe_Monster(i);
					Jump_Player(i);
				}
				
				CheckWinCondition_Player(i);
				
				if (!CHECK_BIT(m_gameData.m_actions[i], JUMP) && !CHECK_BIT(m_gameData.m_actions[i], SECONDARY_ACTION)
					&& !CHECK_BIT(m_gameData.m_actions[i], ACTION) && !CHECK_BIT(m_gameData.m_actions[i], SECONDARY_ACTION))
					m_keypressed[i] = false;
				
				// Reseting the player when he is dead
				if (ObjManager::GetObjects()[i]->GetCurrAnimation()->GetIsDone())
					RestartPlayer(i);

			}
#if ALLPLAYS
#else
		}
#endif
	}
	UpdateItems();
}

// Updates the item objects on the level
void NetworkServer::UpdateItems()
{
	// Update position of item if it is being carried
	for (unsigned int itemIndex = 0; itemIndex < MAX_OBJECT; ++itemIndex)
	{
		BaseItem *currItem = dynamic_cast<BaseItem*>(ObjManager::GetObjects()[itemIndex + MAX_USERS]);
		if (currItem->GetRespawnTime() != 0.0f && currItem->GetItemType() == SPAWN_ITEM)
		{
			currItem->SetRespawnTime(currItem->GetRespawnTime() - float(m_timer.Delta()));
		
			if (currItem->GetRespawnTime() <= 0.0f && ObjManager::CheckForItems())
			{
				currItem->SetRespawnTime(0.0f);
				currItem->SetActive(true);
				m_gameItems.m_worldMatrix[currItem->GetServerID()]._42 -= 1000.0f;
				m_gameItems.m_active |= (1 << currItem->GetServerID());
			}
		}

		if (CHECK_BIT(m_gameItems.m_active, currItem->GetServerID()))
		{
			// Makes the items disappear
			if (currItem->GetItemType() != KEY_ITEM)
				if (currItem->CheckExpiration(float(m_timer.Delta())))
				{
					m_gameItems.m_active &= ~(1 << currItem->GetServerID());
					m_gameItems.m_particleEffect &= ~(1 << currItem->GetServerID());
				}

			// TODO::Needs to work only for Runners
			// NOTE::Will give issues with more than 8 players
			for (unsigned int parentIndex = 0; parentIndex < MAX_USERS; ++parentIndex)
			{
				// Check if item is being carried
				if (CHECK_BIT(m_gameItems.m_carried[itemIndex], parentIndex))
				{
					XMMATRIX boneWorld = XMLoadFloat4x4(&dynamic_cast<Player*>(ObjManager::GetObjects()[parentIndex])->GetLHand());
					boneWorld = XMMatrixMultiply(boneWorld, ObjManager::GetObjects()[parentIndex]->GetWorldMatrix());
					XMStoreFloat4x4(&m_gameItems.m_worldMatrix[itemIndex], boneWorld);
				}
			}

			if (currItem->GetItemType() != DOOR_ITEM && !currItem->GetParent()) //currItem->GetItemType() != DOOR_ITEM  !currItem->GetIsAvailable()
			{
				XMFLOAT3 currForce = currItem->GetForce();
				BaseObject* level = ObjManager::GetObjects()[ObjManager::GetObjects().size() - 1];
				unsigned int blank = 0;
				if ((currItem->GetItemType() == SPAWN_ITEM && currItem->GetGround()) == false)
					if (currForce.x > 0.2f || currForce.y > 0.2f || currForce.z > 0.2f || currItem->GetInAir())
						Physics::ApplyForce(&ObjManager::AccessObject(itemIndex + MAX_USERS), m_gameItems.m_worldMatrix[itemIndex], float(m_timer.Delta()), level, blank);

				// Set the old to the oldest not coliding position
				BaseItem* currItem = dynamic_cast<BaseItem*>(ObjManager::GetObjects()[itemIndex + MAX_USERS]);
				if (currItem->GetIsColliding() && currItem->GetDangerous())
				{
					if (currItem->GetItemType() == BLAST_GRENADE_ITEM || currItem->GetItemType() == MAGNET_GRENADE_ITEM)
						m_gameItems.m_particleEffect |= (1 << currItem->GetServerID());
					currItem->OnCollision();
				}
				// Update hitbox to current items position
				currItem->UpdateHitBox(&XMLoadFloat4x4(&m_gameItems.m_worldMatrix[itemIndex]));
				// Rotate object
				XMStoreFloat4x4(&m_gameItems.m_worldMatrix[itemIndex], XMMatrixMultiply(XMMatrixRotationY(float(m_timer.Delta() * 3.0f)), XMLoadFloat4x4(&m_gameItems.m_worldMatrix[itemIndex])));
			}
		}
	}
}

// Takes care of players movement
void NetworkServer::Move_Player(unsigned int _player)
{
	Player* currPlayer = dynamic_cast<Player*>(ObjManager::GetObjects()[_player]);
	Monster* mon;

	if (currPlayer->GetID() == MONSTER_ID)
		mon = dynamic_cast<Monster*>(currPlayer);

	XMFLOAT4X4 worldMatrix = m_gameData.m_worldMatrix[_player];
	XMFLOAT4X4 oldWorldMatrix = m_gameData.m_worldMatrix[_player];

	XMFLOAT3 f = currPlayer->GetForce();
	XMMATRIX newWorld = XMLoadFloat4x4(&worldMatrix);
	float aerialInfluence = 0.2f;
	
	if (CHECK_BIT(m_gameData.m_actions[_player], FORWARD) || CHECK_BIT(m_gameData.m_actions[_player], BACKWARD))
	{
		if (CHECK_BIT(m_gameData.m_actions[_player], FORWARD))
		{
			if (currPlayer->GetInAir())
			{
				Move_Direction(newWorld, 2, -currPlayer->GetMoveSpeed(), aerialInfluence);
			}
			else if ((CHECK_BIT(m_gameData.m_actions[_player], RUN) && currPlayer->GetID() == RUNNER_ID) || (currPlayer->GetID() == MONSTER_ID && mon->GetCharge() == 1.0 && CHECK_BIT(m_gameData.m_actions[_player], RUN)))
			{
				float multiplier = 3.0f;
	
				if (currPlayer->GetID() == RUNNER_ID)
				{
					BaseItem* carriedItem = dynamic_cast<Runner*>(currPlayer)->GetItem();
					if (carriedItem)
						if (carriedItem->GetItemType() == KEY_ITEM)
							multiplier = 2.2f;
				}
				Move_Direction(newWorld, 2, -currPlayer->GetMoveSpeed(), multiplier);
	
				if (!currPlayer->GetAnimBusy())
					SetAnimations(_player, RUN_ANIM);
			}
			else
			{
				Move_Direction(newWorld, 2, -currPlayer->GetMoveSpeed(), 1.0f);
	
				if (!currPlayer->GetAnimBusy())
				{
					if (currPlayer->GetID() == MONSTER_ID && mon->GetCharge() > 1.0)
					{
						SetAnimations(_player, CRAWL_FORWARD_ANIM);
						currPlayer->GetAnimations()[CRAWL_FORWARD_ANIM];
					}
					else
					{
						SetAnimations(_player, WALK_ANIM);
						currPlayer->GetAnimations()[WALK_ANIM]->SetReverse(false);
					}
				}
			}
		}
		else if (CHECK_BIT(m_gameData.m_actions[_player], BACKWARD))
		{
			if (currPlayer->GetInAir())
			{
				Move_Direction(newWorld, 2, currPlayer->GetMoveSpeed(), aerialInfluence);
			}
			else
			{
				Move_Direction(newWorld, 2, currPlayer->GetMoveSpeed(), 0.6f);
	
				if (!currPlayer->GetAnimBusy())
				{
					if (currPlayer->GetID() == MONSTER_ID && mon->GetCharge() > 1.0)
					{
						SetAnimations(_player, CRAWL_BACKWARD_ANIM);
						currPlayer->GetAnimations()[CRAWL_BACKWARD_ANIM]->SetReverse(true);
					}
					else
					{
						SetAnimations(_player, WALK_BACKWARD_ANIM);
						currPlayer->GetAnimations()[WALK_BACKWARD_ANIM]->SetReverse(true);
					}
				}
			}
		}
	}
	else
	{
		if (!currPlayer->GetAnimBusy() && !currPlayer->GetInAir())
		{
			if (currPlayer->GetID() == MONSTER_ID && mon->GetCharge() > 1.0)
				SetAnimations(_player, POUNCE_CHARGE_ANIM);
			else
			{
				SetAnimations(_player, IDLE_ANIM);
				SetSounds(_player, RUNNER_IDLE_SOUND);
			}
		}
	}
	if (CHECK_BIT(m_gameData.m_actions[_player], RIGHT))
	{
		if (currPlayer->GetInAir())
		{
			Move_Direction(newWorld, 0, -currPlayer->GetMoveSpeed(), aerialInfluence);
		}
		else if (CHECK_BIT(m_gameData.m_actions[_player], RUN))
			Move_Direction(newWorld, 0, -currPlayer->GetMoveSpeed(), 0.4f);
		else
		{
			Move_Direction(newWorld, 0, -currPlayer->GetMoveSpeed(), 0.8f);
	
			if (!currPlayer->GetAnimBusy() && (m_gameData.m_animation[_player] & (1 << WALK_ANIM)) == 0)
			{
				if (currPlayer->GetID() == MONSTER_ID && mon->GetCharge() > 1.0)
				{
					SetAnimations(_player, CRAWL_RIGHT_ANIM);
					currPlayer->GetAnimations()[CRAWL_RIGHT_ANIM];
				}
				else
				{
					SetAnimations(_player, STRAFE_RIGHT_ANIM);
					currPlayer->GetAnimations()[STRAFE_RIGHT_ANIM];
				}
			}
		}
	}
	else if (CHECK_BIT(m_gameData.m_actions[_player], LEFT))
	{
		if (currPlayer->GetInAir())
		{
			Move_Direction(newWorld, 0, currPlayer->GetMoveSpeed(), aerialInfluence);
		}
		else if (CHECK_BIT(m_gameData.m_actions[_player], RUN))
			Move_Direction(newWorld, 0, currPlayer->GetMoveSpeed(), 0.4f);
		else
		{
			Move_Direction(newWorld, 0, currPlayer->GetMoveSpeed(), 0.8f);
	
			if (!currPlayer->GetAnimBusy() && (m_gameData.m_animation[_player] & (1 << WALK_ANIM)) == 0)
			{
				if (currPlayer->GetID() == MONSTER_ID && mon->GetCharge() > 1.0)
				{
					SetAnimations(_player, CRAWL_LEFT_ANIM);
					currPlayer->GetAnimations()[CRAWL_LEFT_ANIM];
				}
				else
				{
					SetAnimations(_player, STRAFE_LEFT_ANIM);
					currPlayer->GetAnimations()[STRAFE_LEFT_ANIM];
				}
			}
		}
	}
	XMMATRIX newMat = newWorld;
	XMMATRIX oldMat = XMLoadFloat4x4(&worldMatrix);
	XMVECTOR newPos = newMat.r[3];
	XMVECTOR oldPos = oldMat.r[3];
	XMFLOAT3 store;
	XMStoreFloat3(&store, XMVectorSubtract(oldPos, newPos));
	f.x -= store.x;
	f.z -= store.z;
	
	currPlayer->SetForce(f);
	
	// Set the sound for walking
	if (CHECK_BIT(m_gameData.m_actions[_player], FORWARD) || CHECK_BIT(m_gameData.m_actions[_player], BACKWARD) || CHECK_BIT(m_gameData.m_actions[_player], LEFT) || CHECK_BIT(m_gameData.m_actions[_player], RIGHT) || CHECK_BIT(m_gameData.m_actions[_player], RUN))
		SetSounds(_player, FOOTSTEP_SOUND);

	// Rotating the character
#if DEBUGGING
	printf("SERVER DELTA: %f\n", float(m_timer.Delta()));
	printf("SERVER MOUSE: %f\n", m_gameData.m_playerMouseX[0]);
#endif
	if (currPlayer->GetInAir())
		XMStoreFloat4x4(&worldMatrix, XMMatrixMultiply(XMMatrixRotationY(m_gameData.m_playerMouseX[_player] * float(m_timer.Delta()) * 0.3f), XMLoadFloat4x4(&worldMatrix)));
	else 
		XMStoreFloat4x4(&worldMatrix, XMMatrixMultiply(XMMatrixRotationY(m_gameData.m_playerMouseX[_player] * float(m_timer.Delta()) * 0.9f), XMLoadFloat4x4(&worldMatrix)));

	XMFLOAT4X4 tempWorld = worldMatrix;
	tempWorld._41 = oldWorldMatrix._41;
	tempWorld._42 = oldWorldMatrix._42;
	tempWorld._43 = oldWorldMatrix._43;
	oldWorldMatrix = tempWorld;

	m_gameData.m_worldMatrix[_player] = worldMatrix;
}

// Takes care of players jump
void NetworkServer::Jump_Player(unsigned int _player)
{
	Player* currPlayer = dynamic_cast<Player*>(ObjManager::GetObjects()[_player]);

	if (CHECK_BIT(m_gameData.m_actions[_player], JUMP) && !currPlayer->GetInAir() && !m_keypressed[_player])
	{
		m_keypressed[_player] = true;
		XMFLOAT3 f = { currPlayer->GetForce().x * 1.4f, 0.0f, currPlayer->GetForce().z * 1.4f };

		float range = 10.0f;

		if (f.x > range)
			f.x = range;
		else if (f.x < -range)
			f.x = -range;
		if (f.z > range)
			f.z = range;
		else if (f.z < -range)
			f.z = -range;

		if (currPlayer->GetID() == RUNNER_ID)
			f.y += 10.0f;
		else if (currPlayer->GetID() == MONSTER_ID)
			f.y += 10.0f;
		currPlayer->SetForce(f);
		currPlayer->SetInAir(true);
		SetAnimations(_player, JUMP_ANIM);
		currPlayer->SetAnimBusy(true);
		currPlayer->GetAnimations()[JUMP_ANIM]->SetTime(5);
		// Set sound for jumping
		SetSounds(_player, JUMP_SOUND);
	}
}

// Takes care of monster pounce
void NetworkServer::Pounce_Monster(unsigned int _player)
{
	if (ObjManager::GetObjects()[_player]->GetID() == MONSTER_ID)
	{
		Monster* currPlayer = dynamic_cast<Monster*>(ObjManager::GetObjects()[_player]);
		if (!currPlayer->GetInAir())
		{
			if (CHECK_BIT(m_gameData.m_actions[_player], ACTION) && !currPlayer->GetInAir() && currPlayer->GetCharge() != 0.0f)
			{
				SetSounds(_player, MONSTER_CRAW_SOUND);
				
				// Set sound for jumping
				float MAX_CHARGE = 2.4f;
				float CHARGE_RATE = 0.7f;
				if (currPlayer->GetCharge() < MAX_CHARGE)
					currPlayer->SetCharge(currPlayer->GetCharge() + float(m_timer.Delta()) * CHARGE_RATE);
				else
					currPlayer->SetCharge(MAX_CHARGE);
			}
			else
			{

				if (currPlayer->GetCharge() > 1.0f)
				{
					float initialForce = 0.08f;
					XMFLOAT3 f = currPlayer->GetForce();
					LookAt(f, m_gameData.m_worldMatrix[_player], currPlayer->GetView());
	
					f.x *= -currPlayer->GetMoveSpeed() * currPlayer->GetCharge() * initialForce;
					f.y = 11 - f.y;
					f.y *= currPlayer->GetMoveSpeed() * currPlayer->GetCharge() * initialForce * 0.8f;
					f.z *= -currPlayer->GetMoveSpeed() * currPlayer->GetCharge() * initialForce;
	
					SetSounds(_player, MONSTER_POUNCE_SOUND);

					currPlayer->SetForce(f);
					currPlayer->SetInAir(true);
					currPlayer->SetIsDeadly(true);
					currPlayer->SetCharge(1.0);
					
				}
				else if (!CHECK_BIT(m_gameData.m_actions[_player], ACTION) && currPlayer->GetCharge() == 0.0f)
					currPlayer->SetCharge(1.0f);
			}
		}
	}
}

// Swipe action for monster
void NetworkServer::Swipe_Monster(unsigned int _player)
{
	if (ObjManager::GetObjects()[_player]->GetID() == MONSTER_ID)
	{
		Monster* currPlayer = dynamic_cast<Monster*>(ObjManager::GetObjects()[_player]);
		if (CHECK_BIT(m_gameData.m_actions[_player], SECONDARY_ACTION) && !currPlayer->GetInAir())
		{
			if (!m_keypressed[_player])
			{
				m_keypressed[_player] = true;

				if (CHECK_BIT(m_gameData.m_actions[_player], ACTION))
					currPlayer->SetCharge(0.0f);
				else if (!currPlayer->GetAnimBusy())
				{
					currPlayer->SetSwipeBox(true);
					SetAnimations(_player, SWIPE_ANIM);
					currPlayer->GetAnimations()[SWIPE_ANIM]->SetTime(5);
					currPlayer->SetAnimBusy(true);
					SetSounds(_player, PUSHED_SOUND);
				}
			}
			else
				currPlayer->SetSwipeBox(false);
		}
	}
}

// Pick up an item
bool NetworkServer::PickUP_Runner(unsigned int _player)
{
	if (ObjManager::GetObjects()[_player]->GetID() == RUNNER_ID)
	{
		Runner* currPlayer = dynamic_cast<Runner*>(ObjManager::GetObjects()[_player]);
		if (CHECK_BIT(m_gameData.m_actions[_player],ACTION) && currPlayer->GetAvailability())
		{
			m_ableToUseItem = 0.0f;
			XMFLOAT3 cP = { 0, 0, 0 };
			bool hands = false, feet = false;
			vector<unsigned int> collidersIndex = currPlayer->CheckCollision(&cP, hands, feet);

			if (collidersIndex.size() == 0)
				return false;

			BaseObject* collidingWith;
			for (unsigned int i = 0; i < collidersIndex.size(); ++i)
			{
				collidingWith = &ObjManager::AccessObject(collidersIndex[i]);
				if (collidingWith->GetID() == ITEM_ID)
					break;
			}

			/* Pick up the item. The carried uint8_t identifies which item is the player trying to pick up
			Each item can be carried only by one player and that is indicated by the bit that is turned on.*/
			if ((collidingWith->GetID() == ITEM_ID) && currPlayer->GetAvailability() && !currPlayer->GetInAir() && !dynamic_cast<BaseItem*>(collidingWith)->GetParent())
			{
				BaseItem* collidingWithItem = (dynamic_cast<BaseItem*>(collidingWith));
				if (collidingWithItem->GetItemType() == KEY_ITEM || (collidingWithItem->GetItemType() != DOOR_ITEM && collidingWithItem->GetItemType() != SPAWN_ITEM) && !collidingWithItem->GetDangerous())
				{
					// Check to see if anybody else is carrying the item before picking it up
					bool held = false;
					for (unsigned int heldBy = 0; heldBy < m_currNumOfPlayers; ++heldBy)
						if (CHECK_BIT(m_gameItems.m_carried[collidingWithItem->GetServerID()], heldBy))
							held = true;

					if (!held)
					{
						// Set the item to carried state
						m_gameItems.m_carried[collidingWithItem->GetServerID()] |= (1 << _player);
						collidingWithItem->SetParent(currPlayer);
						collidingWithItem->SetIsCarried(true);
						currPlayer->SetAvailability(false);
						currPlayer->SetItem(collidingWithItem);
						SetSounds(_player, KEY_PICKUP);
					}
					else
						return false;
				}
				else if (collidingWithItem->GetItemType() == SPAWN_ITEM)
				{
					// Set the item to carried state
					collidingWithItem->SetRespawnTime(5);
					m_gameItems.m_active &= ~(1 << collidingWithItem->GetServerID());
					BaseItem* item = ObjManager::GetAvailableItem(collidingWithItem);
					m_gameItems.m_active |= (1 << item->GetServerID());
					m_gameItems.m_particleEffect &= ~(1 << item->GetServerID());
					m_gameItems.m_worldMatrix[collidingWithItem->GetServerID()]._42 += 1000.0f;
					item->SetParent(currPlayer);
					item->Init();
					m_gameItems.m_carried[item->GetServerID()] |= (1 << _player);
					item->SetIsCarried(true);
					currPlayer->SetAvailability(false);
					currPlayer->SetItem(item);
				}

				return true;
			}
 		}
	}
	return false;
}

// Checks to see if a player won the game
bool NetworkServer::CheckWinCondition_Player(unsigned int _player)
{
	Player* currPlayer = dynamic_cast<Player*>(ObjManager::GetObjects()[_player]);
	
	if (currPlayer->GetID() == RUNNER_ID) // Win condition for the Runners
	{
		if (!m_restartGame)
		{
			XMFLOAT3 cP = { 0, 0, 0 };
			bool hands = false, feet = false;

			vector<unsigned int> collidersIndex = currPlayer->CheckCollision(&cP, hands, feet);

			if (collidersIndex.size() == 0)
				return false;

			BaseObject* collidingWith;
			for (unsigned int i = 0; i < collidersIndex.size(); ++i)
			{
				collidingWith = &ObjManager::AccessObject(collidersIndex[i]);
				if (collidingWith->GetID() == ITEM_ID)
					if (dynamic_cast<BaseItem*>(collidingWith)->GetItemType() == DOOR_ITEM)
						break;
			}

			Runner* currRunner = dynamic_cast<Runner*>(currPlayer);

			// Check if the capsule is a item
			if (collidingWith->GetID() == ITEM_ID &&  currRunner->GetItem())
			{
				BaseItem* collidingWithItem = dynamic_cast<BaseItem*>(collidingWith);
				if (collidingWithItem->GetItemType() == DOOR_ITEM)
				{
					if (currRunner->GetItem()->GetItemType() == KEY_ITEM)
					{
						// WIN 
						m_restartGame = true;
						++m_playerScore[_player];

						currPlayer->SetForce(XMFLOAT3(0.0f, 0.0f, 0.0f));
						m_gameData.m_checkWin |= (1 << _player);
						if (m_gameData.m_gameTime < GAME_END)
							m_gameData.m_gameTime = GAME_END;
						
						//RestartPlayer(_player);
						return true;
					}
				}
			}
		}
	}
	else if (currPlayer->GetID() == MONSTER_ID) // Win condition for the Mosters
	{
#if 0 
		uint8_t aliveRunners = m_currNumOfPlayers - 1;
		for (unsigned int i = 0; i < MAX_USERS; ++i)
		{  
			Player* currRunner = dynamic_cast<Player*>(ObjManager::GetObjects()[i]);
			if (currRunner->GetID() == RUNNER_ID && CHECK_BIT(m_gameData.m_active, i))
			{
				m_checkWin = true;
				if (!currRunner->GetIsAlive()) // Has to be get alive
					--aliveRunners;
			}
		}
		
		if (m_checkWin)
		{
			if (aliveRunners == 0)
			{
				// WIN 
				m_restartGame = true;
				currPlayer->SetForce(XMFLOAT3(0.0f, 0.0f, 0.0f));
				m_gameData.m_checkWin |= (1 << _player);
				return true;
			}
		}
#endif
		if (!m_restartGame)
		{
			if (m_gameData.m_gameTime > GAME_END)
			{
				++m_playerScore[_player];
				m_restartGame = true;
				currPlayer->SetForce(XMFLOAT3(0.0f, 0.0f, 0.0f));
				m_gameData.m_checkWin |= (1 << _player);
			
			}
		}
	}
	return false;
}

// Handles the first actuon of the player
void  NetworkServer::Action_Runner(unsigned int _player)
{
	if (ObjManager::GetObjects()[_player]->GetID() == RUNNER_ID)
	{
		Player* currPlayer = dynamic_cast<Player*>(ObjManager::GetObjects()[_player]);

		if (ObjManager::GetObjects()[_player]->GetID() == RUNNER_ID)
		{
			Runner* currPlayer = dynamic_cast<Runner*>(ObjManager::GetObjects()[_player]);

			if (CHECK_BIT(m_gameData.m_actions[_player], ACTION) && !m_keypressed[_player])
			{
				m_keypressed[_player] = true;


				// Check if runner holds an item. If he does not push
				if (m_ableToUseItem > 0.2f && PickUP_Runner(_player))
					return;

				if (!currPlayer->GetItem())
				{
					m_ableToUseItem = 0.0f;
					currPlayer->SetPushBoxActive(true);
					//currPlayer->SetIsStunned(true);

					SetAnimations(_player, PUSH_ANIM);
					currPlayer->GetAnimations()[PUSH_ANIM]->SetTime(5);
					currPlayer->SetAnimBusy(true);
				}
				else if (m_ableToUseItem > 0.2f)
				{
					m_ableToUseItem = 0.0f;
					// Get the item the player is carrying
					BaseItem* currItem = currPlayer->GetItem();
					if (currItem)
					{
						ITEM_TYPE type = currItem->GetItemType();
						XMFLOAT3 f;
						//LookAt(f, m_gameData.m_worldMatrix[_player], currPlayer->GetView());
						// Apply the action
						switch (type)
						{
						case POCKET_SAND_ITEM:
						{
							m_gameItems.m_particleEffect |= (1 << currItem->GetServerID());
							dynamic_cast<PocketSand*>(currItem)->Action(m_gameItems.m_worldMatrix, f);
							break;
						}
						case MAGNET_GRENADE_ITEM:
							dynamic_cast<MagnetGranade*>(currItem)->Action(m_gameItems.m_worldMatrix);
							break;
						case BLAST_GRENADE_ITEM:
							dynamic_cast<BlastGrenade*>(currItem)->Action(m_gameItems.m_worldMatrix);
							break;
						case TRAP_ITEM:
							dynamic_cast<Trap*>(currItem)->Action(m_gameItems.m_worldMatrix);
							break;
						}
						if (type != KEY_ITEM)
						{
							// Set the item to not carried state
							m_gameItems.m_carried[currItem->GetServerID()] &= ~(1 << _player);
							currItem->SetParent(nullptr);
							currItem->SetIsCarried(false);
							currItem->SetInAir(true);
							currPlayer->SetAvailability(true);
							currPlayer->SetItem(nullptr);

							SetAnimations(_player, PUSH_ANIM);
							currPlayer->GetAnimations()[PUSH_ANIM]->SetTime(5);
							currPlayer->SetAnimBusy(true);
						}
					}
				}
			}
			else
			{
				
				currPlayer->SetPushBoxActive(false);
			}
		}
	}
}

// Handles the second action of the player
void NetworkServer::Alternate_Action_Runner(unsigned int _player)
{
	if (ObjManager::GetObjects()[_player]->GetID() == RUNNER_ID)
	{
		Player* currPlayer = dynamic_cast<Player*>(ObjManager::GetObjects()[_player]);

		if (ObjManager::GetObjects()[_player]->GetID() == RUNNER_ID)
		{
			Runner* currPlayer = dynamic_cast<Runner*>(ObjManager::GetObjects()[_player]);

			if (CHECK_BIT(m_gameData.m_actions[_player], SECONDARY_ACTION) && currPlayer->GetItem() && !m_keypressed[_player])
			{
				m_keypressed[_player] = true;
				DropItem_Runner(_player);
			}
			else if (CHECK_BIT(m_gameData.m_actions[_player], SECONDARY_ACTION) && !m_keypressed[_player])
			{
				m_keypressed[_player] = true;
				// Check if runner holds an item. If he does not push
				if (!currPlayer->GetItem() && currPlayer->GetCurrAnimation()->GetName() != KICK_ANIM  && !currPlayer->GetAnimBusy() && !currPlayer->GetInAir())
				{
					m_ableToUseItem = 0.0f;
					currPlayer->SetTripBoxActive(true);
					SetAnimations(_player, KICK_ANIM);
					currPlayer->GetAnimations()[KICK_ANIM]->SetTime(5);
					currPlayer->SetAnimBusy(true);
					currPlayer->SetIsStunned(true);
					
					XMFLOAT3 f = { m_gameData.m_worldMatrix[_player]._41, m_gameData.m_worldMatrix[_player]._42, m_gameData.m_worldMatrix[_player]._43 };
					LookAt(f, m_gameData.m_worldMatrix[_player], m_gameData.m_worldMatrix[_player]);
					f.x *= 10.2f;
					f.z *= 10.2f;
					currPlayer->SetForce(f);
				}
			}
			else
				currPlayer->SetTripBoxActive(false);
		}
	}
}

// Takes care of players collision
void NetworkServer::Collision_Player(unsigned int _player)
{
	// Set the old to the oldest not coliding position
	Player* currPlayer = dynamic_cast<Player*>(ObjManager::GetObjects()[_player]);

	XMFLOAT3 cP = { 0, 0, 0 };
	bool hands = false, feet = false;

	vector<unsigned int> colliderIndex = currPlayer->CheckCollision(&cP, hands, feet);
	for (unsigned int i = 0; i < colliderIndex.size(); ++i)
	{
		BaseObject* collidingWith = &ObjManager::AccessObject(colliderIndex[i]);

		if (collidingWith == NULL)
			return;
		if (!collidingWith->GetActive())
			return;

		// If the collision is happening with a capsule
		if (collidingWith->GetHitBox()->type == COL_TYPE::CAPSULE_COL)
		{
			XMFLOAT3 f = currPlayer->GetForce();
			XMFLOAT3 orig = currPlayer->GetForce();
			XMStoreFloat3(&f, currPlayer->GetWorldMatrix().r[3] - collidingWith->GetWorldMatrix().r[3]);

			// Check collision with item
			if (collidingWith->GetID() == ITEM_ID)
			{
				// Apply items effect
				BaseItem* currItem = dynamic_cast<BaseItem*>(collidingWith);
				if (currItem->GetDangerous())
				{
					ITEM_TYPE itemType = currItem->GetItemType();
					if (itemType == POCKET_SAND_ITEM)
					{
						if (dynamic_cast<PocketSand*>(currItem)->GetCaster() != currPlayer)
							m_gameData.m_effectApplied[_player] |= (1 << FLASH_EFFECT);
					}
					if (itemType == MAGNET_GRENADE_ITEM) // Apply magnet functionality
				    {
						if (dynamic_cast<MagnetGranade*>(currItem)->GetCaster() == currPlayer && !currItem->IsParticleActive())
							continue;
						
						XMMATRIX objPosition = XMLoadFloat4x4(&m_gameData.m_worldMatrix[_player]);
						XMMATRIX itemPosition = currItem->GetWorldMatrix();
						
						XMVECTOR forcePullDir = XMVectorSubtract(itemPosition.r[3], objPosition.r[3]);
						
						XMFLOAT3 myPos = { currPlayer->GetWorldMatrix().r[3].m128_f32[0], currPlayer->GetWorldMatrix().r[3].m128_f32[1], currPlayer->GetWorldMatrix().r[3].m128_f32[2] };
						XMFLOAT3 itemPos = { currItem->GetWorldMatrix().r[3].m128_f32[0], currItem->GetWorldMatrix().r[3].m128_f32[1], currItem->GetWorldMatrix().r[3].m128_f32[2] };
						float dis = Collision::DistanceFormula(myPos, itemPos);
						
						float t = dis / 16.0f;
						t = 1 - t;
						
						forcePullDir.m128_f32[0] *= 0.001f; //0.00076f;
						forcePullDir.m128_f32[2] *= 0.001f;
						
						orig.x += forcePullDir.m128_f32[0] * t;
						orig.z += forcePullDir.m128_f32[2] * t;
							  
						currPlayer->SetForce(orig);
						currItem->SetIsColliding(true);
				   	}
					else if (itemType == BLAST_GRENADE_ITEM) // Apply blast functionality
					{
						if (currItem->GetActivatedTime() < 2.0f)
						{
							if (dynamic_cast<BlastGrenade*>(currItem)->GetCaster() == currPlayer && !currItem->IsParticleActive())
								continue;
							
							XMFLOAT3 myPos = { currPlayer->GetWorldMatrix().r[3].m128_f32[0], currPlayer->GetWorldMatrix().r[3].m128_f32[1], currPlayer->GetWorldMatrix().r[3].m128_f32[2] };
							XMFLOAT3 itemPos = { currItem->GetWorldMatrix().r[3].m128_f32[0], currItem->GetWorldMatrix().r[3].m128_f32[1], currItem->GetWorldMatrix().r[3].m128_f32[2] };
							
							float dis = Collision::DistanceFormula(myPos, itemPos);
							
							float t = dis / 14.0f;
							t = 1 - t;
							
							XMMATRIX objPosition = XMLoadFloat4x4(&m_gameData.m_worldMatrix[_player]);
							XMMATRIX itemPosition = currItem->GetWorldMatrix();

							XMVECTOR forcePushDir = XMVectorSubtract(objPosition.r[3], itemPosition.r[3]);

							f.x = forcePushDir.m128_f32[0] * 15.0f;
							//f.y = forcePushDir.m128_f32[1] * 10.0f * t;
							f.z = forcePushDir.m128_f32[2] * 15.0f;
							currPlayer->SetForce(f);
							currItem->SetIsColliding(true);
						}
					}
					else if (itemType == TRAP_ITEM) // Apply trap functionality
					{
						currItem->SetActive(false);
						currPlayer->SetIsStunned(true);
						currItem->SetIsColliding(true);
					}
				}
			}
			else
			{
				// Check if the capsule is a monster
				if (collidingWith->GetID() == MONSTER_ID)
				{
					if (currPlayer->GetID() == RUNNER_ID && dynamic_cast<Monster*>(collidingWith)->GetIsDeadly())
					{
						// rotate the player when they are hit
						dynamic_cast<Runner*>(currPlayer)->SetCollisionPoint(f);

						SetAnimations(_player, DEATH_ANIM);
						currPlayer->GetAnimations()[DEATH_ANIM]->SetTime(0);
						currPlayer->GetAnimations()[DEATH_ANIM]->SetFrame(1);
						currPlayer->SetIsStunned(true);
						currPlayer->SetAnimBusy(true);
						currPlayer->SetIsAlive(false);
						DropItem_Runner(_player);
						f.x *= 1.1f;
						f.z *= 1.1f;
						currPlayer->SetForce(f);
						return;
					}
					else if (hands)
					{
						f.x *= 5.5f;
						f.z *= 5.5f;

						SetAnimations(_player, TRIPPING_ANIM);
						currPlayer->GetAnimations()[TRIPPING_ANIM]->SetTime(5);
						currPlayer->SetAnimBusy(true);
						currPlayer->SetIsStunned(true);
						DropItem_Runner(_player);
						SetSounds(_player, PUSHED_SOUND);
					}
				}

				// Check if the capsule is a runner
				if (collidingWith->GetID() == RUNNER_ID)
				{
					Runner* colRunner = dynamic_cast<Runner*>(collidingWith);

					if (hands) // PUSH
					{
						XMFLOAT3 sweetSpot;
						sweetSpot.x = collidingWith->GetWorldMatrix().r[3].m128_f32[0] - collidingWith->GetWorldMatrix().r[2].m128_f32[0] * 1.7f;
						sweetSpot.y = collidingWith->GetWorldMatrix().r[3].m128_f32[1];
						sweetSpot.z = collidingWith->GetWorldMatrix().r[3].m128_f32[2] - collidingWith->GetWorldMatrix().r[2].m128_f32[2] * 1.7f;
						
						XMFLOAT3 myPos = { currPlayer->GetWorldMatrix().r[3].m128_f32[0], currPlayer->GetWorldMatrix().r[3].m128_f32[1], currPlayer->GetWorldMatrix().r[3].m128_f32[2] };
						XMFLOAT3 theirPos = { collidingWith->GetWorldMatrix().r[3].m128_f32[0], collidingWith->GetWorldMatrix().r[3].m128_f32[1], collidingWith->GetWorldMatrix().r[3].m128_f32[2] };
						XMFLOAT3 closest;
						Collision::ClosestPointToLine(theirPos, sweetSpot, f, &closest);

						float dis = Collision::DistanceFormula(closest, myPos);
						dis = dis / 2.5f;
						dis = 1.0f - dis;
						float range = 5.0f;//Collision::DistanceFormula(sweetSpot, theirPos);

						f.x += f.x * 7.5f * dis;
						f.z += f.z * 7.5f * dis;

						if (currPlayer->GetID() == RUNNER_ID && !currPlayer->GetAnimBusy() && currPlayer->GetCurrAnimation()->GetName() != DEATH_ANIM)
						{
							SetAnimations(_player, PUSHED_ANIM);
							currPlayer->GetAnimations()[PUSHED_ANIM]->SetTime(5);
							currPlayer->SetAnimBusy(true);
							//currPlayer->SetIsStunned(true);
							DropItem_Runner(_player);
							SetSounds(_player, PUSHED_SOUND);
						}
						
						//Runner* currRunner = dynamic_cast<Runner*>(currPlayer);
						//
						//if (currPlayer->GetID() == RUNNER_ID && currRunner->GetItem())
						//{
						//
						//}

						//if (currPlayer->GetID() == MONSTER_ID)
						//	currPlayer->SetIsStunned(true);
					}
					if (feet) // TRIP
					{
						if (currPlayer->GetID() == RUNNER_ID && !currPlayer->GetAnimBusy())
						{
							LookAt(f, m_gameData.m_worldMatrix[_player], m_gameData.m_worldMatrix[_player]);
							f.x *= 2.8f;
							f.z *= 2.8f;

							if (currPlayer->GetCurrAnimation()->GetName() != DEATH_ANIM)
							{
								SetAnimations(_player, TRIPPING_ANIM);
								currPlayer->GetAnimations()[TRIPPING_ANIM]->SetTime(5);
								currPlayer->SetAnimBusy(true);
								currPlayer->SetIsStunned(true);
								SetSounds(_player, PUSHED_SOUND);
						}
							DropItem_Runner(_player);
						}
					}
				}
				// If collision with capsule with no additional effects bounce back
				f.x *= 1.3f;
				f.y *= 1.3f;
				f.z *= 1.3f;

				// Set the force to the current applied force
				currPlayer->SetForce(f);
			}
		}
#if 0
		else if (collidingWith->GetHitBox()->type == COL_TYPE::AABB_COL)
		{
			XMFLOAT3 f = { currPlayer->GetForce().x, 0.05f, currPlayer->GetForce().z };
			currPlayer->SetInAir(false);
			if (currPlayer->GetID() == MONSTER_ID)
				dynamic_cast<Monster*>(currPlayer)->SetIsDeadly(false);
			currPlayer->SetForce(f);
		}
#endif
	}
}

// Sets the animations
void NetworkServer::SetAnimations(int _player, int _anim)
{
	// Refresh animations
	m_gameData.m_animation[_player] = false;

	//for (size_t i = 0; i < 32; i++)
	//	m_gameData.m_animation[_player] &= ~(1 << i);

	// Turn on the correct animation
	m_gameData.m_animation[_player] |= (1 << _anim);
}

// Sets the sounds
void NetworkServer::SetSounds(int _player, int _sound)
{
	// Refresh sounds
	m_gameData.m_sound[_player] = 0;
	// Turn on the right sound
	m_gameData.m_sound[_player] |= (1 << _sound);
}

// Applies force at a certain direction
void NetworkServer::LookAt(DirectX::XMFLOAT3 &_f, DirectX::XMFLOAT4X4 _world, DirectX::XMFLOAT4X4 _forward)
{
	_world._41 += _forward._31;
	_world._42 += _forward._32;
	_world._43 += _forward._33;

	XMMATRIX newMat = XMLoadFloat4x4(&_world);
	XMMATRIX oldMat = XMLoadFloat4x4(&_forward);
	newMat = oldMat - newMat;

	_f.x = newMat.r[3].m128_f32[0];
	_f.y = newMat.r[3].m128_f32[1];
	_f.z = newMat.r[3].m128_f32[2];
}

// Set the player to his starting position
void NetworkServer::StartPlayer(unsigned int _player)
{
#if NOT_IN_CAGE
	m_gameData.m_worldMatrix[_player]._41 = _player * 7.0f;
#else

	if (CHECK_BIT(m_gameData.m_active, _player))
	{
		if (_player == 0)// ROUGHLY MONSTER START POS
		{
			m_gameData.m_worldMatrix[_player]._41 = 115.0f;
			m_gameData.m_worldMatrix[_player]._42 = -8.0f;
			m_gameData.m_worldMatrix[_player]._43 = 14.0f;
		}
		else if (_player < 4)
		{
			m_gameData.m_worldMatrix[_player]._42 = 7.0f;
			// RUNNER START POS for first 3 players
			m_gameData.m_worldMatrix[_player]._41 = 98.0f - _player * 6.0f;
			m_gameData.m_worldMatrix[_player]._43 = 10.0f - _player * 1.0f;
		}
		else
		{
			m_gameData.m_worldMatrix[_player]._42 = 7.0f;
			// RUNNER START POS for first 3 players
			m_gameData.m_worldMatrix[_player]._41 = 101.0f - (_player - 3) * 6.0f;
			m_gameData.m_worldMatrix[_player]._43 = 0.0f - (_player - 3) * 1.0f;
		}
	}
#endif

}

// Move the start gates
void NetworkServer::MoveStartGate(unsigned int _ID)
{
	if (_ID == RUNNERSTART_ID)
	{
		if (m_gameItems.m_worldMatrix[MAX_OBJECT - 2]._41 > -27.0f)
		{
			m_gameItems.m_worldMatrix[MAX_OBJECT - 2]._41 -= 0.001f;
			m_gameItems.m_worldMatrix[MAX_OBJECT - 2]._43 -= 0.00025f;
		}
	}
	else if (_ID == MONSTERSTART_ID)
	{
		if (m_gameItems.m_worldMatrix[MAX_OBJECT - 1]._42 < 15.0f)
			m_gameItems.m_worldMatrix[MAX_OBJECT - 1]._42 += 0.001f;
	}
}

// Move in a direction, helper function
void NetworkServer::Move_Direction(XMMATRIX& _world, unsigned int _row, float _speed, float _mult)
{
	_world.r[3].m128_f32[0] += _world.r[_row].m128_f32[0] * _speed * _mult * float(m_timer.Delta());
	_world.r[3].m128_f32[1] += _world.r[_row].m128_f32[1] * _speed * _mult * float(m_timer.Delta());
	_world.r[3].m128_f32[2] += _world.r[_row].m128_f32[2] * _speed * _mult * float(m_timer.Delta());
}

// Drop item for runner
void NetworkServer::DropItem_Runner(unsigned int _player)
{
	// Dropping the item
	Runner* currRunner = dynamic_cast<Runner*>(ObjManager::GetObjects()[_player]);
	if (currRunner->GetItem())
	{
		BaseItem* Item = currRunner->GetItem();
		// Set the item to carried state
		m_gameItems.m_carried[Item->GetServerID()] &= ~(1 << _player);
		Item->SetParent(nullptr);
		Item->SetIsCarried(false);
		Item->SetInAir(true);
		//m_gameItems.m_particleEffect &= ~(1 << Item->GetServerID());
		currRunner->SetAvailability(true);
		currRunner->SetItem(nullptr);
	}
}

// Refresh the game state
void NetworkServer::RestartGame()
{
	if (m_restartGame && (m_gameData.m_gameTime > (GAME_END + 0.3f)))
	{
		SendState();
		SendEndScreen();

		m_gameData.m_gameTime = 0.0f;
		m_ableToUseItem = 0.0f;
		m_restartGame = false;
		m_start = false;
		// Refreshing all objecs for the server 
		ObjManager::RefreshObjects();

		// Refreshing players 
		for (unsigned int i = 0; i < MAX_USERS; ++i)
		{
			// Game data initialization 
			m_gameData.m_checkWin &= ~(1 << i);
			m_gameData.m_actions[i] = false;
			m_gameData.m_effectApplied[i] = false;
			m_gameData.m_animation[i] = false;
			m_gameData.m_animation[i] |= (1 << IDLE_ANIM);
			SetSounds(i,RUNNER_IDLE_SOUND);
			m_gameData.m_playerMouseX[i] = 0.0f;
			StartPlayer(i);
		}

		// Refreshing items 
		for (unsigned int i = 0; i < MAX_OBJECT; ++i)
		{
			m_gameItems.m_active |= (1 << i);
			m_gameItems.m_particleEffect &= ~(1 << i);
			m_gameItems.m_carried[i] = false;
			XMStoreFloat4x4(&m_gameItems.m_worldMatrix[i], XMMatrixIdentity());
		}

		// Refreshing the ground for the boxes 
		for (size_t i = 0; i < ObjManager::GetObjects().size(); i++)
		{
			if (ObjManager::GetObjects()[i]->GetID() == ITEM_ID && dynamic_cast<BaseItem*>(ObjManager::GetObjects()[i])->GetItemType() == SPAWN_ITEM)
				ObjManager::GetObjects()[i]->SetGround(nullptr);
		}
		// Refreshing boxes positions 
		XMFLOAT3 pos[8] = {
			{ -5.743f, 13.609f, -87.756f },
			{ -53.823f, -1.927f, 55.884f },
			{ 52.674f, -11.599f, 56.941f },
			{ -105.937f, -5.220f, -23.900f },
			{ -36.020f, 37.356f, 21.716f },
			{ -15.323f, 48.978f, -28.443f },
			{ 65.806f, -11.997f, -100.676f },
			{ 54.406f, 41.691f, 50.307f } };
		for (size_t i = 1; i < 9; i++)
		{
			m_gameItems.m_worldMatrix[i]._41 = pos[i - 1].x;
			m_gameItems.m_worldMatrix[i]._42 = pos[i - 1].y;
			m_gameItems.m_worldMatrix[i]._43 = pos[i - 1].z;
		}
		// Setting items to not render
		for (size_t i = 9; i < MAX_OBJECT - 2; i++)
			m_gameItems.m_active &= ~(1 << i);

		// Cages refresh
		ObjManager::AccessObject(m_monsterCageIndex).ResetTris();
		ObjManager::AccessObject(m_monsterCageIndex).SetWorldMatrix(XMMatrixIdentity());
		ObjManager::AccessObject(m_runnersCageIndex).ResetTris();
		ObjManager::AccessObject(m_runnersCageIndex).SetWorldMatrix(XMMatrixIdentity());
	}
}

// Refresh a player
void NetworkServer::RestartPlayer(unsigned int _player)
{
	ObjManager::RefreshObject(_player);
	// Game data initialization
	//m_gameData.m_checkWin &= ~(1 << _player);
	m_gameData.m_actions[_player] = false;
	m_gameData.m_effectApplied[_player] = false;
	m_gameData.m_animation[_player] = false;
	m_gameData.m_animation[_player] |= (1 << IDLE_ANIM);
	SetSounds(_player, RUNNER_IDLE_SOUND);
	m_gameData.m_playerMouseX[_player] = 0.0f;
	XMStoreFloat4x4(&m_gameData.m_worldMatrix[_player], XMMatrixIdentity());
	StartPlayer(_player);
}

