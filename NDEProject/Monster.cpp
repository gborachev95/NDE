#include "Monster.h"

Monster::Monster()
{
}

Monster::Monster(BaseObject* _push1, BaseObject* _push2, float _speed, float _mass) : Player(MONSTER_ID, _speed, _mass)
{
	m_swipeBox.type = PUSH_COL;
	m_swipeBox.m_active = false;
	m_swipeBox.m_Radius = 2.0f;
	m_isDeadly = false;
	m_charge = 1.0f;
}

Monster::Monster(float _speed, float _mass) : Player(MONSTER_ID, _speed, _mass)
{
	m_swipeBox.type = PUSH_COL;
	m_swipeBox.m_active = false;
	m_swipeBox.m_Radius = 2.0;
	m_isDeadly = false;
	m_charge = 1.0f;
}

Monster::~Monster()
{
}

void Monster::Input()
{
	//m_attackBox.m_active = false;
	//if (!GetIsStunned())
	//{

		Pounce();
		// Has to happen last
		Player::Input();
	//}
}

void Monster::Update()
{
	Player::Update();
	UpdateSwipeBox();

	//if (f.x < .02f && f.x > -.02f && f.z < .02f && f.z > -.02f && !GetInAir())
	//	SetIsStunned(false);
	//push1->SetWorldMatrix(l);
	//push2->SetWorldMatrix(r);
}

void Monster::Render()
{
	Player::Render();
}

void Monster::UpdateSwipeBox()
{
	m_swipeBox.m_Segment.m_Start.x = GetWorldMatrix().r[3].m128_f32[0];
	m_swipeBox.m_Segment.m_Start.y = GetWorldMatrix().r[3].m128_f32[1];
	m_swipeBox.m_Segment.m_Start.z = GetWorldMatrix().r[3].m128_f32[2];
	
	m_swipeBox.m_Segment.m_End.x = GetWorldMatrix().r[3].m128_f32[0];
	m_swipeBox.m_Segment.m_End.y = GetWorldMatrix().r[3].m128_f32[1];
	m_swipeBox.m_Segment.m_End.z = GetWorldMatrix().r[3].m128_f32[2];
	
	m_swipeBox.m_Segment.m_Start.x = m_swipeBox.m_Segment.m_Start.x + GetWorldMatrix().r[0].m128_f32[0] * 1.2f + GetWorldMatrix().r[2].m128_f32[0] * -2.0f;
	m_swipeBox.m_Segment.m_Start.y += 4;
	m_swipeBox.m_Segment.m_Start.z = m_swipeBox.m_Segment.m_Start.z - GetWorldMatrix().r[0].m128_f32[2] * -1.2f + GetWorldMatrix().r[2].m128_f32[2] * -2.0f;
	
	m_swipeBox.m_Segment.m_End.x = m_swipeBox.m_Segment.m_End.x - GetWorldMatrix().r[0].m128_f32[0] * 1.2f + GetWorldMatrix().r[2].m128_f32[0] * -2.0f;
	m_swipeBox.m_Segment.m_End.y += 4;
	m_swipeBox.m_Segment.m_End.z = m_swipeBox.m_Segment.m_End.z + GetWorldMatrix().r[0].m128_f32[2] * -1.2f + GetWorldMatrix().r[2].m128_f32[2] * -2.0f;
}

void Monster::Pounce()
{
	//// TODO:: RE-WRITE THIS TO BE USED BY THE SERVER
	//auto kb = Graphics::single_keyboard->GetState();
	//
	//if (!GetInAir() && GetAsyncKeyState(VK_LBUTTON))
	//{
	//	m_isDeadly = true;
	//	XMFLOAT3 f = GetForce();
	//
	//	XMMATRIX worldMatrix = GetWorldMatrix();
	//
	//	XMMATRIX newWorld = worldMatrix;
	//
	//	newWorld.r[3].m128_f32[0] += worldMatrix.r[2].m128_f32[0] * -GetMoveSpeed() * 8.0f;
	//	newWorld.r[3].m128_f32[1] += worldMatrix.r[2].m128_f32[1] * -GetMoveSpeed() * 8.0f;
	//	newWorld.r[3].m128_f32[2] += worldMatrix.r[2].m128_f32[2] * -GetMoveSpeed() * 8.0f;
	//
	//	newWorld -= worldMatrix;
	//	f.x = newWorld.r[3].m128_f32[0];
	//	f.y = newWorld.r[3].m128_f32[1] + 8.0f;
	//	f.z = newWorld.r[3].m128_f32[2];
	//
	//	SetForce(f);
	//	SetInAir(true);
	//}

	// MOVED TO PLAYER
	//uint8_t action = GetActions();
	//if (GetAsyncKeyState(VK_LBUTTON))
	//{
	//	action |= (1 << PUSH);
	//	SetActions(action);
	//}
	//else
	//{
	//	action &= ~(1 << PUSH);
	//	SetActions(action);
	//}
}