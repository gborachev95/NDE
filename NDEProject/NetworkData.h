#pragma once
#ifndef NetworkData_H
#define NetworkData_H
#include <stdint.h>
#include <DirectXMath.h>

#define MAX_USERS 5
#define MAX_OBJECT 19

// Controls used for m_actions
#define FORWARD 0
#define LEFT 1
#define BACKWARD 2
#define RIGHT 3
#define JUMP 4
#define RUN 5
#define ACTION 6
#define SECONDARY_ACTION 7

#define CHECK_BIT(var, pos) ((var >> pos) & 1 )

struct NetworkPlayers
{
	uint8_t             m_active;
	uint8_t             m_checkWin;
	uint8_t             m_actions[MAX_USERS];
	uint8_t             m_effectApplied[MAX_USERS];
	uint32_t            m_sound[MAX_USERS]; 
	uint32_t            m_animation[MAX_USERS];
	float               m_playerMouseX[MAX_USERS]; 
	float               m_gameTime; 
	DirectX::XMFLOAT4X4 m_worldMatrix[MAX_USERS];
};

struct NetworkObjects
{   
	uint32_t            m_active;
	uint32_t            m_particleEffect;
	uint8_t             m_carried[MAX_OBJECT];
	DirectX::XMFLOAT4X4 m_worldMatrix[MAX_OBJECT];
};
#endif