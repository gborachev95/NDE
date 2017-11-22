#include "Runner.h"
#include "BaseItem.h"
#include "KeyItem.h"
#include "ClientWrapper.h"

Runner::Runner(BaseObject* _push)
{
}

Runner::Runner(BaseObject* _push1, BaseObject* _push2, float _speed, float _mass) : Player(RUNNER_ID, _speed, _mass)
{
	m_pushBox.type = PUSH_COL;
	m_pushBox.m_active = false;
	m_pushBox.m_Radius = 5.5f;
	m_tripBox.type = PUSH_COL;
	m_tripBox.m_active = false;
	m_tripBox.m_Radius = 0.2f;
	m_availability = true;
	m_item = false;
	push1 = _push1;
	push2 = _push2;
}

Runner::Runner(float _speed, float _mass) : Player(RUNNER_ID, _speed, _mass)
{
	m_pushBox.type = PUSH_COL;
	m_pushBox.m_active = false;
	m_pushBox.m_Radius = 2.0f;
	m_tripBox.type = PUSH_COL;
	m_tripBox.m_active = false;
	m_tripBox.m_Radius = 1.5f;
	m_availability = true;
	m_item = false;
	push1 = nullptr;
	push2 = nullptr;
	trip1 = nullptr;
	trip2 = nullptr;
}

Runner::~Runner()
{
}

void Runner::Input()
{
	UpdatePushAndTripBox();
	DropAsCilent();
	PickUPAsClient();
	Player::Input();
}

void Runner::Update()
{
	Player::Update();

	if (push1)
	{
		//XMFLOAT3 sweetSpot;
		//sweetSpot.x = GetWorldMatrix().r[3].m128_f32[0] - GetWorldMatrix().r[2].m128_f32[0] * 2.0f;
		//sweetSpot.y = GetWorldMatrix().r[3].m128_f32[1];
		//sweetSpot.z = GetWorldMatrix().r[3].m128_f32[2] - GetWorldMatrix().r[2].m128_f32[2] * 2.0f;
		//sweetSpot.y += 4.0f;
		//push1->SetPosition(sweetSpot.x, sweetSpot.y, sweetSpot.z);

		push1->SetPosition(m_pushBox.m_Segment.m_End.x, m_pushBox.m_Segment.m_End.y, m_pushBox.m_Segment.m_End.z);
		push2->SetPosition(m_pushBox.m_Segment.m_Start.x, m_pushBox.m_Segment.m_Start.y, m_pushBox.m_Segment.m_Start.z);
		trip1->SetPosition(m_tripBox.m_Segment.m_End.x, m_tripBox.m_Segment.m_End.y, m_tripBox.m_Segment.m_End.z);
		trip2->SetPosition(m_tripBox.m_Segment.m_Start.x, m_tripBox.m_Segment.m_Start.y, m_tripBox.m_Segment.m_Start.z);
	}
}

void Runner::Render()
{
	Player::Render();
}

void Runner::UpdatePushAndTripBox()
{
	float pushBoxWidth = 0.2f;
	float pushBoxLength = 2.6f;
	float tripBoxWidth = 0.8f;
	float tripBoxLength = 2.2f;

	m_pushBox.m_Segment.m_Start.x = GetWorldMatrix().r[3].m128_f32[0];
	m_pushBox.m_Segment.m_Start.y = GetWorldMatrix().r[3].m128_f32[1];
	m_pushBox.m_Segment.m_Start.z = GetWorldMatrix().r[3].m128_f32[2];

	m_pushBox.m_Segment.m_End.x = GetWorldMatrix().r[3].m128_f32[0];
	m_pushBox.m_Segment.m_End.y = GetWorldMatrix().r[3].m128_f32[1];
	m_pushBox.m_Segment.m_End.z = GetWorldMatrix().r[3].m128_f32[2];

	m_pushBox.m_Segment.m_Start.x = m_pushBox.m_Segment.m_Start.x + GetWorldMatrix().r[0].m128_f32[0] * pushBoxWidth + GetWorldMatrix().r[2].m128_f32[0] * -pushBoxLength;
	m_pushBox.m_Segment.m_Start.y += 4.0f;
	m_pushBox.m_Segment.m_Start.z = m_pushBox.m_Segment.m_Start.z - GetWorldMatrix().r[0].m128_f32[2] * -pushBoxWidth + GetWorldMatrix().r[2].m128_f32[2] * -pushBoxLength;

	m_pushBox.m_Segment.m_End.x = m_pushBox.m_Segment.m_End.x - GetWorldMatrix().r[0].m128_f32[0] * pushBoxWidth + GetWorldMatrix().r[2].m128_f32[0] * -pushBoxLength;
	m_pushBox.m_Segment.m_End.y += 4.0f;
	m_pushBox.m_Segment.m_End.z = m_pushBox.m_Segment.m_End.z + GetWorldMatrix().r[0].m128_f32[2] * -pushBoxWidth + GetWorldMatrix().r[2].m128_f32[2] * -pushBoxLength;

	m_tripBox.m_Segment.m_Start.x = GetWorldMatrix().r[3].m128_f32[0];
	m_tripBox.m_Segment.m_Start.y = GetWorldMatrix().r[3].m128_f32[1];
	m_tripBox.m_Segment.m_Start.z = GetWorldMatrix().r[3].m128_f32[2];

	m_tripBox.m_Segment.m_End.x = GetWorldMatrix().r[3].m128_f32[0];
	m_tripBox.m_Segment.m_End.y = GetWorldMatrix().r[3].m128_f32[1];
	m_tripBox.m_Segment.m_End.z = GetWorldMatrix().r[3].m128_f32[2];

	m_tripBox.m_Segment.m_Start.x = m_tripBox.m_Segment.m_Start.x + GetWorldMatrix().r[0].m128_f32[0] * tripBoxWidth + GetWorldMatrix().r[2].m128_f32[0] * -tripBoxLength;
	m_tripBox.m_Segment.m_Start.y += 1.0f;
	m_tripBox.m_Segment.m_Start.z = m_tripBox.m_Segment.m_Start.z - GetWorldMatrix().r[0].m128_f32[2] * -tripBoxWidth + GetWorldMatrix().r[2].m128_f32[2] * -tripBoxLength;

	m_tripBox.m_Segment.m_End.x = m_tripBox.m_Segment.m_End.x - GetWorldMatrix().r[0].m128_f32[0] * tripBoxWidth + GetWorldMatrix().r[2].m128_f32[0] * -tripBoxLength;
	m_tripBox.m_Segment.m_End.y += 1.0f;
	m_tripBox.m_Segment.m_End.z = m_tripBox.m_Segment.m_End.z + GetWorldMatrix().r[0].m128_f32[2] * -tripBoxWidth + GetWorldMatrix().r[2].m128_f32[2] * -tripBoxLength;
}

void Runner::PickUPAsClient()
{
	if (GetClient())
	{
		/*uint8_t actions = GetActions();
		if (Graphics::single_mouse->GetState().leftButton && m_availability)
		{
			actions |= (1 << ACTION);
			SetActions(actions);
		}
		else
		{
			actions &= ~(1 << ACTION);
			SetActions(actions);
		}*/
		
		//uint8_t actions = GetActions();
		//if (Graphics::single_mouse->GetState().leftButton && m_availability)
		//{
		//	actions |= (1 << PICK_UP);
		//	SetActions(actions);
		//}
	}
}

void Runner::DropAsCilent()
{
	if (GetClient())
	{
		//uint8_t actions = GetActions();
		//if (Graphics::single_mouse->GetState().rightButton)
		//{
		//	actions |= (1 << SECONDARY_ACTION);
		//	SetActions(actions);
		//}
		//else
		//{
		//	actions &= ~(1 << SECONDARY_ACTION);
		//	SetActions(actions);
		//}
		//uint8_t actions = GetActions();
		//if (Graphics::single_mouse->GetState().rightButton)
		//{
		//	actions &= ~(1 << PICK_UP);
		//	SetActions(actions);
		//}

	}
}

#if 0 
void Runner::PickUp()
{
	if (Graphics::single_keyboard->GetState().Z && !m_availability)
	{
		XMFLOAT3 collisionPoint;
		bool hands = false;
		//vector<unsigned int> collidersIndex = CheckCollision(&cP, hands);

		BaseObject* currObj = CheckCollision(&collisionPoint, hands);

		if (currObj)
		{
			if (currObj->GetID() == ITEM_ID)
			{
				BaseItem * item = dynamic_cast<BaseItem*>(currObj);
				if (item->GetParent() == nullptr)
				{
					item->SetParent(this);
					item->SetIsCarried(true);
					m_availability = true;
					m_item = item;
				}
			}
		}
	}
}
void Runner::Drop()
{
	if (Graphics::single_keyboard->GetState().X && m_availability)
	{
		m_item->SetParent(nullptr);
		m_item->SetIsCarried(false);
		m_availability = false;
		m_item = nullptr;
	}
}
#endif