#include "NetworkPlatform.h"

namespace NetworkingPlatform
{
	// Constructor.
	NetworkMessage::NetworkMessage(NETWORK_IO _type)
	{
		m_type = _type;
		m_begin = 0;
		m_end = 0;
	}

	// Returns the size of the buffer in bytes.
	int NetworkMessage::GetBufferSize()
	{
		return MESSAGE_SIZE;
	}

	// Gets the data's buffer so a message can be read into it.
	char* NetworkMessage::GetReadBuffer()
	{
		if (m_type == NETWORK_OUTPUT)
			return "Tried to get read buffer for output message NetworkMessage.getReadBuffer()";

		return m_dataBuffer;

	}

	// Gets the data's buffer so that the data within it can be sent.
	const char* NetworkMessage::GetSendBuffer()
	{
		if (m_type == NETWORK_INPUT)
			return "Tried to get send buffer for input message NetworkMessage.getSendBuffer()";

		return m_dataBuffer;
	}

	// Get the length of the message.
	int NetworkMessage::GetLength()
	{
		return m_end;
	}

	// Set the end of the message (only used after receiving a message.)
	void NetworkMessage::SetEnd(int _end)
	{
		m_end = _end;
	}

	// Reads length bytes into data from message, starting at offset.
	// (Returns bytes read.)
	int NetworkMessage::Read(char* data, int offset, int length)
	{
		return RawRead((uint8_t*)data + offset, length, true);
	}

	// Reads length bytes into data from message until a null terminator
	// is read (returns bytes read.)
	int NetworkMessage::ReadString(char* data, int length)
	{
		return RawRead((uint8_t*)data, length, 0, true);
	}

	// Will read a data type from the message and return it.
	int8_t NetworkMessage::ReadByte()
	{
		char result;
		RawRead((uint8_t*)&result, 1, false);
		return result;
	}

	int16_t NetworkMessage::ReadShort()
	{
		int16_t result;
		RawRead((uint8_t*)&result, 2, false);
		return ntohs(result);
	}

	int32_t NetworkMessage::ReadInt()
	{
		int32_t result;
		RawRead((uint8_t*)&result, 4, false);
		return ntohl(result);
	}

	float NetworkMessage::ReadFloat()
	{
		unsigned int result;
		RawRead((uint8_t*)&result, sizeof(float), false);
		return ntohf(result);
	}

	DirectX::XMFLOAT4X4 NetworkMessage::ReadFloat4x4()
	{
		DirectX::XMFLOAT4X4 mat;

		mat._11 = ReadFloat();
		mat._12 = ReadFloat();
		mat._13 = ReadFloat();
		mat._14 = ReadFloat();

		mat._21 = ReadFloat();
		mat._22 = ReadFloat();
		mat._23 = ReadFloat();
		mat._24 = ReadFloat();

		mat._31 = ReadFloat();
		mat._32 = ReadFloat();
		mat._33 = ReadFloat();
		mat._34 = ReadFloat();

		mat._41 = ReadFloat();
		mat._42 = ReadFloat();
		mat._43 = ReadFloat();
		mat._44 = ReadFloat();

		return mat;
	}

	void NetworkMessage::ReadGameData(NetworkPlayers& _data, unsigned int _player)
	{
		_data.m_active = ReadByte();
		_data.m_checkWin = ReadByte();
		_data.m_actions[_player] = ReadByte();
		_data.m_effectApplied[_player] = ReadByte();
		_data.m_animation[_player] = ReadInt();
		_data.m_sound[_player] = ReadInt();
		_data.m_playerMouseX[_player] = ReadFloat();
		_data.m_gameTime = ReadFloat();
		_data.m_worldMatrix[_player] = ReadFloat4x4();
	}

	void NetworkMessage::ReadObjectGameData(NetworkObjects& _data)
	{
		for (unsigned int i = 0; i < MAX_OBJECT; ++i)
		{
			_data.m_carried[i] = ReadByte();
			_data.m_active = ReadInt();
			_data.m_particleEffect = ReadInt();
			_data.m_worldMatrix[i] = ReadFloat4x4();
		}
	}

	// Writes length bytes into message from data, starting at offset.
	// (Returns bytes written.)
	void NetworkMessage::Write(const char* data, int offset, int length)
	{
		RawWrite((uint8_t*)data + offset, length);
	}

	// Writes bytes into message from data until a null terminator
	// is written to the buffer.
	void NetworkMessage::WriteString(const char *data)
	{
		RawWrite((uint8_t*)data, (int)strlen(data) + 1, 0);
	}

	void NetworkMessage::WriteString(const char *data, int length)
	{
		RawWrite((uint8_t*)data, length, 0);
	}

	// Will write a data type to the message buffer without sending it.
	void NetworkMessage::WriteByte(int8_t output)
	{
		RawWrite((uint8_t*)&output, 1);
	}

	void NetworkMessage::WriteShort(int16_t output)
	{
		output = htons(output);
		RawWrite((uint8_t*)&output, 2);
	}

	void NetworkMessage::WriteInt(int32_t output)
	{
		output = htonl(output);
		RawWrite((uint8_t*)&output, 4);
	}

	void NetworkMessage::WriteFloat(float output)
	{
		unsigned int fl = htonf(output);
		RawWrite((uint8_t*)&fl, sizeof(float));
	}

	void NetworkMessage::WriteFloat4x4(DirectX::XMFLOAT4X4 output)
	{
		WriteFloat(output._11);
		WriteFloat(output._12);
		WriteFloat(output._13);
		WriteFloat(output._14);

		WriteFloat(output._21);
		WriteFloat(output._22);
		WriteFloat(output._23);
		WriteFloat(output._24);

		WriteFloat(output._31);
		WriteFloat(output._32);
		WriteFloat(output._33);
		WriteFloat(output._34);

		WriteFloat(output._41);
		WriteFloat(output._42);
		WriteFloat(output._43);
		WriteFloat(output._44);
	}

	void NetworkMessage::WriteGameData(NetworkPlayers output, unsigned int _player)
	{
			WriteByte(output.m_active);
			WriteByte(output.m_checkWin);
			WriteByte(output.m_actions[_player]);
			WriteByte(output.m_effectApplied[_player]);
			WriteInt(output.m_animation[_player]);
			WriteInt(output.m_sound[_player]);
			WriteFloat(output.m_playerMouseX[_player]);
			WriteFloat(output.m_gameTime);
			WriteFloat4x4(output.m_worldMatrix[_player]);
	}

	void NetworkMessage::WriteObjectGameData(NetworkObjects output)
	{
		for (unsigned int i = 0; i < MAX_OBJECT; ++i)
		{
			WriteByte(output.m_carried[i]);
			WriteInt(output.m_active);
			WriteInt(output.m_particleEffect);
			WriteFloat4x4(output.m_worldMatrix[i]);
		}
	}

	// Clears all data in the data buffer.
	void NetworkMessage::Reset(NETWORK_IO _type)
	{
		m_type = _type;
		m_begin = 0;
		m_end = 0;
	}

	// Clears all data in the data buffer and resets the type.
	void NetworkMessage::Reset()
	{
		m_begin = 0;
		m_end = 0;
	}

	// Returns the number of bytes available (for reading or writing.)
	int NetworkMessage::BytesAvailable()
	{
		if (m_type == NETWORK_INPUT)
			return m_end - m_begin;
		
		return MESSAGE_SIZE - m_end;
			// else Unknown message type in NetworkMessage.bytesAvailable()
	}

	int NetworkMessage::RawRead(uint8_t* _data, int _length, bool _partialOkay)
	{
		return RawRead(_data, _length, -1, _partialOkay);
	}

	void NetworkMessage::RawWrite(const uint8_t* _data, int _length)
	{
		RawWrite(_data, _length, -1);
	}

	int NetworkMessage::RawRead(uint8_t* _data, int _length, int _delimiter, bool _partialOkay)
	{
		if (m_type == NETWORK_OUTPUT)
			return 0; // Tried to read data from output message in NetworkMessage.rawRead();

		int bytes = 0;
		for (int index = m_begin; bytes != _length; index++)
		{
			if (index == m_end)
			{
				if (_partialOkay)
					break;
				else
					return 0; // Tried to read past end of data in NetworkMessage.rawRead();
			}

			_data[bytes] = m_dataBuffer[index];
			bytes++;

			if (_data[bytes - 1] == _delimiter)
				break;
		}

		m_begin += bytes;

		return bytes;
	}

	void NetworkMessage::RawWrite(const uint8_t* _data, int _length, int _delimiter)
	{
		if (m_type == NETWORK_INPUT)
			return; // Tried to write data to input message in NetworkMessage.rawWrite();
	
		for (int bytes = 0; bytes != _length; bytes++)
		{
			if (m_end >= MESSAGE_SIZE)
				return; // Tried to write past end of buffer in NetworkMessage.rawWrite();
	
			m_dataBuffer[m_end] = _data[bytes];
			m_end++;
	
			if (_data[bytes] == _delimiter)
				break;
		}
	}
	
	int recvNetMessage(SOCKET s, NetworkMessage& message)
	{
		int result;
	
		result = recv(s, message.GetReadBuffer(), message.GetBufferSize(), 0);
	
		if (result > 0)
		{
			message.Reset();
			message.SetEnd(result);
		}
	
		return result;
	}
	
	int recvfromNetMessage(SOCKET s, NetworkMessage& message, sockaddr_in* source)
	{
		int result;
		int addrLen = sizeof(sockaddr_in);
	
		result = recvfrom(s, message.GetReadBuffer(), message.GetBufferSize(), 0, (sockaddr*)source, &addrLen);
	
		if (result > 0)
		{
			message.Reset();
			message.SetEnd(result);
		}
	
		return result;
	}
	
	int sendNetMessage(SOCKET s, NetworkMessage& message)
	{
		return send(s, message.GetSendBuffer(), message.GetLength(), 0);
	}
	
	int sendtoNetMessage(SOCKET s, NetworkMessage& message, const sockaddr_in* source)
	{
		int addrLen = sizeof(sockaddr_in);
	
		return sendto(s, message.GetSendBuffer(), message.GetLength(), 0, (sockaddr*)source, addrLen);
	}
}