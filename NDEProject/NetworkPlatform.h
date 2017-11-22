#pragma once
#ifndef NetworkPlatform_H
#define NetworkPlatform_H
#include <winsock2.h>
#include <Ws2tcpip.h>
#include "stdint.h"
#include "NetworkData.h"
#include <DirectXMath.h>

#pragma comment(lib, "Ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS

enum NETWORK_IO { NETWORK_INPUT, NETWORK_OUTPUT };
enum NETWORK_RETURN_VALUES { NETWORK_SUCCESS, NETWORK_SHUTDOWN, NETWORK_DISCONNECT, NETWORK_BIND_ERROR, NETWORK_CONNECT_ERROR, NETWORK_SETUP_ERROR, NETWORK_STARTUP_ERROR, NETWORK_ADDRESS_ERROR, NETWORK_PARAMETER_ERROR, NETWORK_MESSAGE_ERROR };
enum NETWORK_GAME_STATES { NETWORK_DISCONNECTED, NETWORK_WAITING, NETWORK_RUNNING, NETWORK_GAMEOVER };
enum NETWORK_MESSAGE_TYPE { NETWORK_CL_CONNECT, NETWORK_CL_DATA, NETWORK_CL_ALIVE, NETWORK_SV_OKAY, NETWORK_SV_FULL, NETWORK_SV_SNAPSHOT, NETWORK_SV_CL_CLOSE,NETWORK_SV_END_DATA };

// The size of the data buffer.
#define MESSAGE_SIZE 2048 // 2048

namespace NetworkingPlatform
{
	// Stream for sending & receiving network data.
	class NetworkMessage
	{
		char       m_dataBuffer[MESSAGE_SIZE];
		int        m_begin, m_end;
		NETWORK_IO m_type;
	public:
		NetworkMessage(NETWORK_IO _type);
		int GetBufferSize();
		char* GetReadBuffer();
		const char* GetSendBuffer();
		int GetLength();
		void SetEnd(int _end);
		int Read(char* data, int offset, int length);
		int ReadString(char* data, int length);
		int8_t ReadByte();
		int16_t ReadShort();
		int32_t ReadInt();
		float ReadFloat();
		DirectX::XMFLOAT4X4 ReadFloat4x4();
		void ReadGameData(NetworkPlayers& _data, unsigned int _player);
		void ReadObjectGameData(NetworkObjects& _data);
		void Write(const char* data, int offset, int length);
		void WriteString(const char *data);
		void WriteString(const char *data, int length);
		void WriteByte(int8_t output);
		void WriteShort(int16_t output);
		void WriteInt(int32_t output);
		void WriteFloat(float output);
		void WriteFloat4x4(DirectX::XMFLOAT4X4 output);
		void WriteGameData(NetworkPlayers output, unsigned int _player);
		void WriteObjectGameData(NetworkObjects output);
		void Reset(NETWORK_IO _type);
		void Reset();
		int BytesAvailable();

	private:
		int RawRead(uint8_t* _data, int _length, bool _partialOkay);
		void RawWrite(const uint8_t* _data, int _length);
		int RawRead(uint8_t* _data, int _length, int _delimiter, bool _partialOkay);
		void RawWrite(const uint8_t* _data, int _length, int _delimiter);
	};

	// Helpr functions
	int recvNetMessage(SOCKET s, NetworkMessage& messages);
	int recvfromNetMessage(SOCKET s, NetworkMessage& message, sockaddr_in* source);

	int sendNetMessage(SOCKET s, NetworkMessage& message);
	int sendtoNetMessage(SOCKET s, NetworkMessage& message, const sockaddr_in* source);
	//typedef int socklen_t;

	// Winsock startup function.
	inline int startup()
	{
		WSADATA wsadata;
		return WSAStartup(WINSOCK_VERSION, &wsadata);
	}

	// Winsock shutdown function
	inline int shutdown()
	{
		return WSACleanup();
	}

	// Standard sockets close() function
	inline int close(SOCKET s)
	{
		return closesocket(s);
	}

	// Standard io-control function
	inline int ioctl(SOCKET s, long cmd, u_long *argp)
	{
		return ioctlsocket(s, cmd, argp);
	}

	// A helper function to identify the last network error.
	inline int getError()
	{
		return WSAGetLastError();
	}
}
#endif