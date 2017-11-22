#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "NetworkClient.h"
#include <mutex>
#include "ObjManager.h"
#include "BaseItem.h"
#include "Player.h"
#include "Game.h"
#include "PlayState.h"
#include "SoundManager.h"
#include "Runner.h"
#include "Log.h"

// Defines
#define DEBUGGING 0

using namespace NetworkingPlatform;

NetworkClient::NetworkClient()
{
}

NetworkClient::~NetworkClient()
{
}

bool is_connected(fd_set _listen,fd_set& _return)
{
	//	Use select to find ready sockets;
	FD_ZERO(&_return);
	
	for (unsigned int o = 0; o < _listen.fd_count; ++o)
		FD_SET(_listen.fd_array[o], &_return);

	return true;
}

// Initializes the client
int NetworkClient::Initialize(const char* _address, uint16_t _port)
{
#if DEBUGGING
	LogSetUp(L"NDE");

	printf("STARTING CLIENT\n");
#endif
	char* useAddress = const_cast<char*>(_address);

	// If no address was passed check for local
	//if (useAddress[0] == '\0')
	//	useAddress = FindServer();
	//// If no local, exit
	//if (useAddress[0] == '\0')
	//	return NETWORK_DISCONNECT;

	// Initialize data
	InitializeState();

	// Initialize listener
	FD_ZERO(&m_ListenersSockets);

	// Set connection
	//Create a socket
	m_clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (m_clientSocket == INVALID_SOCKET)
	{
		Stop();
#if DEBUGGING
		printf("DEBUG::CLIENT EXITED ON CREATION OF SOCKET\n");
#endif
		return NETWORK_DISCONNECT;
	}

	sockaddr_in sockAddress;
	ZeroMemory(&sockAddress, sizeof(sockAddress));
	sockAddress.sin_family = AF_INET;
	sockAddress.sin_port = htons(_port);
	sockAddress.sin_addr.s_addr = inet_addr(_address);

	if (sockAddress.sin_addr.s_addr == -1)
	{
#if DEBUGGING
		printf("DEBUG::CLIENT's ADDRESS INVALID\n");
#endif
		Stop();
		return NETWORK_ADDRESS_ERROR;
	}

	// Connect to the server 
	int result = connect(m_clientSocket, (sockaddr*)&sockAddress, sizeof(sockAddress));
	if (result == INVALID_SOCKET)
	{
#if DEBUGGING
		printf("DEBUG::CLIENT CANNOT CONNECT TO SERVERS SOCKET\n");
#endif
		Stop();
		return NETWORK_SETUP_ERROR;
	}
	// SEND CONNECT MESSAGE TO SERVER
	NetworkMessage messageToServer(NETWORK_OUTPUT);
	messageToServer.WriteByte((int8_t)NETWORK_CL_CONNECT);
	if (sendNetMessage(m_clientSocket, messageToServer) <= 0)
	{
#if DEBUGGING
		printf("DEBUG::CLIENT CANNOT SEND INITIAL MESSAGE TO SERVER\n");
#endif
		Stop();
		return NETWORK_DISCONNECT;
	}

	// Check if you can communicate with the server 
	// Adding the listen sock to the set
	FD_SET(m_clientSocket, &m_ListenersSockets);
	fd_set currrec;
	is_connected(m_ListenersSockets, currrec);
	timeval timeout{ 2,2 };
	select(0, &currrec, NULL, NULL, &timeout);
	if (!FD_ISSET(m_clientSocket, &currrec))
	{
#if DEBUGGING
		printf("DEBUG::CLIENT CANNOT COMMUNICATE WITH SERVER\n");
#endif
		Stop();
		return NETWORK_DISCONNECT;
	}


	// Get response from server 
	NetworkMessage message(NETWORK_INPUT);
	int msgSize = recvNetMessage(m_clientSocket, message);
	if (msgSize <= 0)
	{
#if DEBUGGING
		printf("DEBUG::CLIENT DID NOT RECEIVE A MESSAGE FROM SERVER\n");
#endif
		Stop();
		return NETWORK_DISCONNECT;
	}
	uint8_t serverAnswer = message.ReadByte();

	// Set the players index 
	m_player = message.ReadByte();
	printf("PlayerIndex received from server %d\n", m_player);
	if (serverAnswer != NETWORK_SV_OKAY)
	{
#if DEBUGGING
		printf("DEBUG::CLIENT SERVER IS FULL\n");
#endif
		Stop();
		return NETWORK_DISCONNECT;
	}

	// Setting client to be running 
	SetActive(true);
#if DEBUGGING
	printf("DEBUG::CLIENT EXECUTING\n");
#endif

	return NETWORK_SUCCESS;
}

// Initializes the state of the game.
void NetworkClient::InitializeState()
{
	for (unsigned int i = 0; i < MAX_USERS; ++i)
	{
		m_playerScore[i] = 0;
		// Game data initialization
		m_gameData.m_active = false;
		m_gameData.m_checkWin = false;
		m_gameData.m_actions[i] = false;
		m_gameData.m_animation[i] = false;
		m_gameData.m_animation[i] |= (1 << IDLE_ANIM);
		m_gameData.m_playerMouseX[i] = 0.0f;
		m_gameData.m_gameTime = 0.0f;
		XMStoreFloat4x4(&m_gameData.m_worldMatrix[i], XMMatrixIdentity());
		m_gameData.m_worldMatrix[i]._41 = i*5.0f;
		ObjManager::RefreshObject(i);
	}
}

char* NetworkClient::FindServer()
{
	char* ip = "";
	// Adding the listen sock to the set
	FD_SET(m_clientSocket, &m_ListenersSockets);
	fd_set currrec;
	is_connected(m_ListenersSockets, currrec);
	timeval timeout{ 2,2 };
	select(0, &currrec, NULL, NULL, &timeout);
	if (!FD_ISSET(m_clientSocket, &currrec))
	{
		Stop();
		return ip;
	}
	return ip;
}

// Receive and process messages from the server.
int NetworkClient::Run()
{
	while (CHECK_BIT(m_gameData.m_active,m_player))
	{
		NetworkMessage message(NETWORK_INPUT);
		if (recvNetMessage(m_clientSocket, message) <= 0)
		{
			SetActive(false);
			Stop();
			return NETWORK_DISCONNECT;
		}

		int8_t type = message.ReadByte();
		switch (type)
		{
		case NETWORK_SV_SNAPSHOT:
		{
			// Read in the player data
			for (unsigned int i = 0; i < MAX_USERS; ++i)
				message.ReadGameData(m_gameData, i);

			// Read in the items data
			message.ReadObjectGameData(m_gameItems);

			// Update the client with the new received data
			Update();
			break;
		}
		case NETWORK_SV_CL_CLOSE:
		{
			SetActive(false);
			if (m_player != 0)
				Stop();
			return NETWORK_SHUTDOWN;
			break;
		}
		case NETWORK_SV_END_DATA:
		{
			// Writing the score
			for (unsigned int i = 0; i < MAX_USERS; ++i)
				m_playerScore[i] = message.ReadInt();

			break;
		}
		}

		// Since it's UDP we have to send message that we are still connected
#if 1 
		++m_ticTimer;
		if (m_ticTimer >= 50)
		{
			m_ticTimer = 0;
			SendAlive();
		}		
#endif
	}
	return NETWORK_SUCCESS;
}

// Clean up and shut down the client.
void NetworkClient::Stop()
{
#if DEBUGGING
	printf("DEBUG::CLIENT %d EXITING\n",m_player);
#endif
	SendClose();
	shutdown(m_clientSocket, SD_BOTH);
}

// Send the player's input to the server.
int  NetworkClient::SendData(float _mouseX, uint8_t _actionsInput)
{
	m_gameData.m_actions[m_player] = _actionsInput;
	m_gameData.m_playerMouseX[m_player] = _mouseX;
	// Transmit the player's input status.
	NetworkMessage message(NETWORK_OUTPUT);
	message.WriteByte(NETWORK_CL_DATA);
	message.WriteByte(m_gameData.m_actions[m_player]);
	message.WriteFloat(m_gameData.m_playerMouseX[m_player]);
	sendNetMessage(m_clientSocket, message);
		
	return NETWORK_SUCCESS;
}

// Copies the current item data into the player stuct
void  NetworkClient::GetPlayerData(NetworkPlayers& _gameData)
{
	for (unsigned int i = 0; i < MAX_USERS; ++i)
	{
	    _gameData.m_active = m_gameData.m_active;
		_gameData.m_checkWin = m_gameData.m_checkWin;
		_gameData.m_actions[i] = m_gameData.m_actions[i]; // TODO :: TAKE IT OUT
		_gameData.m_effectApplied[i] = m_gameData.m_effectApplied[i];
		_gameData.m_animation[i] = m_gameData.m_animation[i];
		_gameData.m_sound[i] = m_gameData.m_sound[i];
		_gameData.m_playerMouseX[i] = m_gameData.m_playerMouseX[i]; // TODO:: TAKE IT OUT
		_gameData.m_gameTime = m_gameData.m_gameTime; // TODO:: TAKE IT OUT

		_gameData.m_worldMatrix[i] = m_gameData.m_worldMatrix[i];
	}
}

// Copies the current item data into the item stuct
void  NetworkClient::GetItemData(NetworkObjects& _gameData)
{
	for (unsigned int i = 0; i < MAX_OBJECT; ++i)
	{
		_gameData.m_active = m_gameItems.m_active;
		_gameData.m_particleEffect = m_gameItems.m_particleEffect;
		_gameData.m_carried[i] = m_gameItems.m_carried[i];
		_gameData.m_worldMatrix[i] = m_gameItems.m_worldMatrix[i];
	}
}

// Sends a CL_ALIVE message to the server (private, suggested)
int  NetworkClient::SendAlive()
{
	NetworkMessage message(NETWORK_OUTPUT);
	message.WriteByte(NETWORK_CL_ALIVE);
	if (sendNetMessage(m_clientSocket, message) <= 0)
	{
		Stop();
		return NETWORK_DISCONNECT;
	}

	return NETWORK_SUCCESS;
}

// Sends a SV_CL_CLOSE message to the server (private, suggested)
void NetworkClient::SendClose()
{
	NetworkMessage message(NETWORK_OUTPUT);
	message.WriteByte(NETWORK_SV_CL_CLOSE);
	sendNetMessage(m_clientSocket, message);
}

// Updates the client with the server data
void NetworkClient::Update()
{
	PlayState::SetCheckWin(m_gameData.m_checkWin);
	PlayState::SetGameTime(m_gameData.m_gameTime);
	for (unsigned int i = 0; i < MAX_USERS; ++i)
		PlayState::SetUsersScore(i, m_playerScore[i]);

	// Update
	for (unsigned int i = 0; i < ObjManager::GetObjects().size(); ++i)
	{
		BaseObject* currObj = ObjManager::GetObjects()[i];

		// Update players
		if ((currObj->GetID() == PLAYER_ID) || (currObj->GetID() == RUNNER_ID) || (currObj->GetID() == MONSTER_ID))
		{
			// Set the position of the player
			currObj->SetWorldMatrix(XMLoadFloat4x4(&m_gameData.m_worldMatrix[i]));
			// Set the animation of the player
			for (unsigned int bitIndex = 0; bitIndex < 32; ++bitIndex)
			{
				// Checking animations in first byte
				if (CHECK_BIT(m_gameData.m_animation[i], bitIndex))
				{
					currObj->SetNextAnimation(currObj->GetAnimations()[bitIndex]);
					break;
				}
			}

			// Set sound. soundIndex indicates the index of the soundEffectInstance, bitIndex is the sound on the server
			for (unsigned int soundIndex = 0; soundIndex < 32; ++soundIndex)
			{
				if (CHECK_BIT(m_gameData.m_sound[i], soundIndex))
					SoundManager::SetRunningSound(i, soundIndex, true);
				//else
				//	SoundManager::SetRunningSound(i, soundIndex, false);
			}
		}
		else if (currObj->GetID() == ITEM_ID) // Update items
		{
			BaseItem* currItem = dynamic_cast<BaseItem*>(currObj);
			if (currItem->GetItemType() != DOOR_ITEM)
			{
				unsigned int itemID = currItem->GetServerID();
				currObj->SetWorldMatrix(XMLoadFloat4x4(&m_gameItems.m_worldMatrix[itemID]));

				currItem->SetActive(CHECK_BIT(m_gameItems.m_active, currItem->GetServerID()));

				currItem->SetParticleActive(CHECK_BIT(m_gameItems.m_particleEffect, currItem->GetServerID()));


				if (currItem->GetItemType() == KEY_ITEM)
				{
					for (unsigned int inx = 0; inx < MAX_USERS; ++inx)
					{
						if (CHECK_BIT(m_gameItems.m_carried[itemID], inx))
						{
							dynamic_cast<Runner*>(ObjManager::GetObjects()[inx])->SetItem(currItem);
							currItem->SetParent(ObjManager::GetObjects()[inx]);
							ObjManager::GetObjects()[inx]->SetTargeted(true);
							currItem->SetIsCarried(true);
							break;
						}
						else
						{
							ObjManager::GetObjects()[inx]->SetTargeted(false);
							currItem->SetParent(nullptr);
							currItem->SetIsCarried(false);
						}
					}
				}
			}
		}
		else if (currObj->GetID() == RUNNERSTART_ID)
		{
			float difX = fabs(currObj->GetWorldMatrix().r[3].m128_f32[0] - m_gameItems.m_worldMatrix[MAX_OBJECT - 2]._41);
			float difZ = fabs(currObj->GetWorldMatrix().r[3].m128_f32[2] - m_gameItems.m_worldMatrix[MAX_OBJECT - 2]._43);
			currObj->SetPosition(m_gameItems.m_worldMatrix[MAX_OBJECT - 2]._41, m_gameItems.m_worldMatrix[MAX_OBJECT - 2]._42, m_gameItems.m_worldMatrix[MAX_OBJECT - 2]._43);
			currObj->SetQuadTri(-difX, 0, -difZ);
		}
		else if (currObj->GetID() == MONSTERSTART_ID)
		{
			float difY = fabs(currObj->GetWorldMatrix().r[3].m128_f32[1] - m_gameItems.m_worldMatrix[MAX_OBJECT - 1]._42);
			if (difY > 10 && currObj->GetTris()->size() > 0)
				difY = 0;
			currObj->SetPosition(m_gameItems.m_worldMatrix[MAX_OBJECT - 1]._41, m_gameItems.m_worldMatrix[MAX_OBJECT - 1]._42, m_gameItems.m_worldMatrix[MAX_OBJECT - 1]._43);
			if (currObj->GetTris()->size() > 0)
				currObj->SetQuadTri(0, difY, 0);
		}

		// Checking for a screen effect 
		if (m_player >= 0 && m_player < MAX_USERS)
		{
			for (unsigned int bitIndex = 0; bitIndex < 8; ++bitIndex)
			{
				if (CHECK_BIT(m_gameData.m_effectApplied[m_player], bitIndex))
				{
					dynamic_cast<Player*>(ObjManager::GetObjects()[m_player])->PlayEffect(POST_EFFECT_TYPE(bitIndex));
					break;
				}
			}
		}
	}
}
