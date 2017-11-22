#include "Player.h"
#include "ObjManager.h"
#include "Collision.h"
#include "Physics.h"
#include "ClientWrapper.h"
#include "Runner.h"
#include "Monster.h"
#include "SoundManager.h"

#define NO_SERVER 0

Player::Player() : BaseObject(PLAYER_ID, 0.0f, false)
{
	m_moveSpeed = 0.0f;
	m_client = nullptr;
	m_isStunned = false;
	m_actions = false;
	m_head._11 = -10000;
	XMStoreFloat4x4(&m_rhand, XMMatrixIdentity());
	m_isAlive = true;
	m_shiftInverted = false;
}

Player::Player(OBJECT_ID _id, float _speed, float _mass) : BaseObject(_id,_mass,false)
{
	m_moveSpeed = _speed;
	m_client = nullptr;
	m_isStunned = false;
	m_actions = false;
	m_mouseDeltaX = 960.0f;
	m_head._11 = -10000;
	XMStoreFloat4x4(&m_rhand, XMMatrixIdentity());
	m_isAlive = true;
	m_shiftInverted = false;
}

Player::~Player()
{
}

void Player::Input()
{
	BaseObject::Input();	

	MoveAsAClient();
	JumpAsAClient();
	ActionAsClient();
	AlternateActionAsClient();
	if (m_client)
		m_client->SendData(-m_mouseDeltaX, m_actions);
}

void Player::Update()
{
	BaseObject::Update();
	//XMStoreFloat4x4(&m_rFoot, GetBone()[m_rFootInd].m_worldMatrix);
	//XMStoreFloat4x4(&m_lFoot, GetBone()[m_lFootInd].m_worldMatrix);
	//XMStoreFloat4x4(&m_head, GetBone()[m_headInd].m_worldMatrix);

	if (GetBone().size() > 0)
	{
		if (GetID() == MONSTER_ID)
			XMStoreFloat4x4(&m_head, GetBone()[8].m_worldMatrix);
		else if (GetID() == RUNNER_ID)
		{
			XMStoreFloat4x4(&m_head, GetBone()[13].m_worldMatrix);
			XMStoreFloat4x4(&m_rhand, GetBone()[7].m_worldMatrix);
		}
	}
	float sandEffect;
	sandEffect = Graphics::GetPostData().data[1];
	if (sandEffect > 0.0f)
	{
		sandEffect -= float(Graphics::GetTime().Delta()) * 0.2f;
		Graphics::SetPostData(sandEffect);
	}

#if NO_SERVER
	if (!m_client)
	{
		XMFLOAT4X4 air;
		XMStoreFloat4x4(&air, GetWorldMatrix());
		Physics::ApplyForce(this, air, 0.0001f);
		SetWorldMatrix(XMLoadFloat4x4(&air));
	}
#endif
}

void Player::Render()
{
	BaseObject::Render();
}

void Player::Move(XMMATRIX &old)
{
	if (m_client)
		MoveAsAClient();

#if NO_SERVER
else
{
	auto kb = Graphics::single_keyboard->GetState();
	XMMATRIX newPos = GetWorldMatrix();

	// Get the cursor position
	POINT mousePos;
	ZeroMemory(&mousePos, sizeof(POINT));
	GetCursorPos(&mousePos);
	//SetCursorPos(250, 250);

	if (kb.W || kb.S)
	{
		if (kb.W)
		{
			if (kb.LeftShift)
			{
				newPos.r[3] = newPos.r[3] + newPos.r[2] * -m_moveSpeed * 3;
				if (GetCurrAnimation() != GetAnimations()[RUN_ANIM])
					SetNextAnimation(GetAnimations()[RUN_ANIM]);
			}
			else
			{
				newPos.r[3] = newPos.r[3] + newPos.r[2] * -m_moveSpeed;
				if (GetCurrAnimation() != GetAnimations()[WALK_ANIM])
					SetNextAnimation(GetAnimations()[WALK_ANIM]);
			}
		}
		else if (kb.S)
		{
			newPos.r[3] = newPos.r[3] + newPos.r[2] * m_moveSpeed;
			if (GetCurrAnimation() != GetAnimations()[WALK_ANIM])
				SetNextAnimation(GetAnimations()[WALK_ANIM]);
		}
	}
	else if (GetCurrAnimation() != GetAnimations()[IDLE_ANIM])
		SetNextAnimation(GetAnimations()[IDLE_ANIM]);

	// Calculating the difference of the mouse position
	float deltaX = float(m_oldMousePos.x - mousePos.x);

	// Rotating when holding left key
	if (GetAsyncKeyState(VK_LBUTTON))
	{
		newPos = XMMatrixMultiply(XMMatrixRotationY(deltaX * 0.0005f), newPos);
		XMMATRIX temp = newPos;
		temp.r[3] = old.r[3];
		old = temp;
	}

	m_oldMousePos = mousePos;

	if (kb.D)
	{
		newPos.r[3] = newPos.r[3] + newPos.r[0] * -m_moveSpeed;
		//if (GetCurrAnimation() != GetAnimations()[_ANIM])
			//SetNextAnimation(GetAnimations()[_ANIM]);
	}
	else if (kb.A)
	{
		newPos.r[3] = newPos.r[3] + newPos.r[0] * m_moveSpeed;
		//if (GetCurrAnimation() != GetAnimations()[_ANIM])
		//SetNextAnimation(GetAnimations()[_ANIM]);
	}

	SetWorldMatrix(newPos);
	if (kb.W || kb.S)
		UpdateHitBox(&newPos);
}
#endif
}

void Player::Jump()
{
	if (m_client)
		JumpAsAClient();

#if NO_SERVER
	else
	{
		auto kb = Graphics::single_keyboard->GetState();
		if (kb.Space)
		{
			XMMATRIX newPos = GetWorldMatrix();
			XMMATRIX old = GetWorldMatrix();
			if (kb.Space && !GetInAir())
			{
				//m_jump += 0.04f;
				SetInAir(true);
				//SetNextAnimation(GetAnimations()[Animation::JUMP_ANIM]);
			}
	
			SetWorldMatrix(newPos);
			UpdateHitBox(&newPos);
		}
	}
#endif
}

void Player::MoveAsAClient()
{
	if (m_client)
	{
		// Mouse input
		POINT mousePos;
		ZeroMemory(&mousePos, sizeof(POINT));
		GetCursorPos(&mousePos);
		// Calculating the difference of the mouse position
		m_mouseDeltaX = float(960 - mousePos.x);

		// Keyboard input
		auto kb = Graphics::single_keyboard->GetState();

		if (kb.M && !m_keyPressed)
		{
			InvertShift();
			m_keyPressed = true;
		}
		else if (!kb.M)
			m_keyPressed = false;

		// Moving Forward and Backward
		if (kb.W || kb.S || kb.LeftShift)
		{
			if (kb.W)
			{
				m_actions |= (1 << FORWARD);
				if (!m_shiftInverted)
				{
					if (kb.LeftShift)
						m_actions |= (1 << RUN);
					else
						m_actions &= ~(1 << RUN);
				}
				else if (m_shiftInverted)
				{
					if (kb.LeftShift)
						m_actions &= ~(1 << RUN);
					else
						m_actions |= (1 << RUN);
				}


			}
			else if (kb.S)
			{
				m_actions &= ~(1 << FORWARD);
				m_actions |= (1 << BACKWARD);
			}
		}
		else
		{
			m_actions &= ~(1 << FORWARD);
			m_actions &= ~(1 << RUN);
			m_actions &= ~(1 << BACKWARD);
		}

		// Moving Left and Right
		if (kb.D || kb.A)
		{
			if (kb.D)
			{
				m_actions |= (1 << RIGHT);
				m_actions &= ~(1 << LEFT);
			}
			else if (kb.A)
			{
				m_actions |= (1 << LEFT);
				m_actions &= ~(1 << RIGHT);
			}
		}
		else
		{
			m_actions &= ~(1 << RIGHT);
			m_actions &= ~(1 << LEFT);
		}
	}
}

void Player::JumpAsAClient()
{
	if (m_client)
	{
		auto kb = Graphics::single_keyboard->GetState();
		if (kb.Space)
			m_actions |= (1 << JUMP);
		else
			m_actions &= ~(1 << JUMP);
	}
}

void Player::ActionAsClient()
{
	if (m_client)
	{
		if (Graphics::single_mouse->GetState().leftButton)
			m_actions |= (1 << ACTION);
		else
			m_actions &= ~(1 << ACTION);
	}
}

void Player::AlternateActionAsClient()
{
	if (m_client)
	{
		if (Graphics::single_mouse->GetState().rightButton)
			m_actions |= (1 << SECONDARY_ACTION);
		else
			m_actions &= ~(1 << SECONDARY_ACTION);
	}
}

vector<unsigned int> Player::CheckCollision(XMFLOAT3 *cP, bool& _hands, bool& _feet)
{
	vector<unsigned int> collidersIndex;
	BaseObject* obj = nullptr;

	for (unsigned int i = 0; i < ObjManager::GetObjects().size(); i++)
	{
		obj = ObjManager::GetObjects()[i];

		if (!obj->GetActive() || !obj->GetHitBox() || !obj->GetHitBox()->m_active)
			continue;

		if (obj != this)
		{
			switch (obj->GetHitBox()->type)
			{
			case CAPSULE_COL:
			{
				if (obj->GetID() == RUNNER_ID)
				{
					if (Collision::CapsuleToCapsule(*(dynamic_cast<Capsule*>(GetHitBox())), dynamic_cast<Runner*>(obj)->GetPushBox()) && dynamic_cast<Runner*>(obj)->GetPushBox().m_active)
					{
						_hands = true;
						collidersIndex.push_back(i);
					}
					if (Collision::CapsuleToCapsule(*(dynamic_cast<Capsule*>(GetHitBox())), dynamic_cast<Runner*>(obj)->GetTripBox()) && dynamic_cast<Runner*>(obj)->GetTripBox().m_active)
					{
						_feet = true;
						collidersIndex.push_back(i);
					}
				}
				else if (obj->GetID() == MONSTER_ID)
				{
					if (Collision::CapsuleToCapsule(*(dynamic_cast<Capsule*>(GetHitBox())), dynamic_cast<Monster*>(obj)->GetSwipeBox()))
						if (dynamic_cast<Monster*>(obj)->GetSwipeBox().m_active)
						{
							_hands = true;
							collidersIndex.push_back(i);
						}
				}
				if (Collision::CapsuleToCapsule(*(dynamic_cast<Capsule*>(GetHitBox())), *(dynamic_cast<Capsule*>(obj->GetHitBox()))))
				{
					XMFLOAT3 f = { obj->GetWorldMatrix().r[3].m128_f32[0], obj->GetWorldMatrix().r[3].m128_f32[1], obj->GetWorldMatrix().r[3].m128_f32[2] };
					cP = &f;
					collidersIndex.push_back(i);
				}
				continue;
			}
			//case AABB_COL:
			//{
			//	Capsule *player = dynamic_cast<Capsule*>(GetHitBox());
			//	Sphere feet;
			//	feet.m_Center = player->m_Segment.m_Start;
			//	feet.m_Radius = player->m_Radius;
			//	AABB* hB = (dynamic_cast<AABB*>(ObjManager::AccessObject(i).GetHitBox()));
			//
			//	XMFLOAT3 cP = { 0, 0, 0 };
			//	if (Collision::SphereToAABB(feet, *hB, &cP))
			//		collidersIndex.push_back(i);
			//}
			}
		}
	}
	
	return collidersIndex;
}

void Player::PlayEffect(POST_EFFECT_TYPE _effect)
{
	switch (_effect)
	{
	case NO_EFFECT:
		break;
	case FLASH_EFFECT:
		Graphics::SetPostData(2.0f);
		break;
	default:
		break;
	}
}

void Player::InvertShift()
{
	if (m_shiftInverted)
		m_shiftInverted = false;
	else if (!m_shiftInverted)
		m_shiftInverted = true;
}