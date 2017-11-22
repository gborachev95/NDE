#pragma once
#ifndef Player_H
#define Player_H
#include "BaseObject.h"
#include "NetworkData.h"
#include "BaseItem.h"

class ClientWrapper;
class Collision;
class Physics;


class Player : public BaseObject
{
	float                 m_moveSpeed;
	ClientWrapper         *m_client;
	uint8_t               m_actions;
	float                 m_mouseDeltaX;
	bool                  m_isStunned;
	POST_EFFECT_TYPE      m_effect;
	unsigned int          m_sound;
	XMFLOAT4X4            m_rhand, m_lFoot, m_rFoot, m_head;
	unsigned int          m_headInd, m_rFootInd, m_lFootInd;
	bool                  m_isAlive;
	bool                  m_shiftInverted;
	bool                  m_keyPressed;
	XMFLOAT4X4            m_forwardVec;
public:

	// Constructor
	Player();
	Player(OBJECT_ID _id, float _speed, float _mass);

	// Destructor
	virtual ~Player();

	virtual void Input();
	virtual void Update();
	virtual void Render();
	void Move(XMMATRIX &old);
	void Jump();
	vector<unsigned int> CheckCollision(XMFLOAT3* cP, bool& _hands, bool& _feet);
	// Client functions
	void MoveAsAClient();
	void JumpAsAClient();
	void ActionAsClient();
	void PlayEffect(POST_EFFECT_TYPE _effect);
	void AlternateActionAsClient();
	void InvertShift();

	// Getters
	float GetMoveSpeed() { return m_moveSpeed; }
	ClientWrapper* GetClient() { return m_client; }
	bool GetIsStunned() { return m_isStunned; }
	uint8_t GetActions() { return m_actions; }
	POST_EFFECT_TYPE  GetEffect() { return m_effect; }
	XMFLOAT4X4 GetLHand() { return m_rhand; }
	unsigned int GetHeadIndex() { return m_headInd; }
	unsigned int GetLeftFootIndex() { return m_lFootInd; }
	unsigned int GetRightFootIndex() { return m_rFootInd; }
	XMFLOAT4X4 GetHead() { return m_head; }
	XMFLOAT4X4 GetLeftFoot() { return m_lFoot; }
	XMFLOAT4X4 GetRightFoot() { return m_rFoot; }
	bool GetIsAlive() { return m_isAlive; }
	XMFLOAT4X4 GetView() { return m_forwardVec; }

	// Setters
	void SetMoveSpeed(float _moveSpeed) { m_moveSpeed = _moveSpeed; }
	void SetClient(ClientWrapper* _client) { m_client = _client; }
	void SetIsStunned(bool _stun) { m_isStunned = _stun; }
	void SetActions(uint8_t _actions) { m_actions = _actions; }
	void SetEffect(POST_EFFECT_TYPE _effect) { m_effect = _effect; }
	void SetMousePos(float _mousePos) { m_mouseDeltaX = _mousePos; }
	void SetHeadIndex(unsigned int _ind) { m_headInd = _ind; }
	void SetLeftFootIndex(unsigned int _ind) { m_lFootInd = _ind; }
	void SetRightFootIndex(unsigned int _ind) { m_rFootInd = _ind; }
	void SetIsAlive(bool _isAlive) { m_isAlive = _isAlive; }
	void SetView(XMFLOAT4X4 _vec) { m_forwardVec = _vec; }
};
#endif
