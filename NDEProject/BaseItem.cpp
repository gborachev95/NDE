#include "BaseItem.h"
#include "Graphics.h"
#include "Runner.h"
#include "ObjManager.h"
#include "Collision.h"

BaseItem::BaseItem(unsigned int _serverId) : BaseObject(ITEM_ID, 2.0f, false)
{
	m_carried = false;
	m_type = BASE_ITEM;
	m_parent = nullptr;
	SetToServerID(_serverId);
	SetDangerous(false);
	SetInAir(true);
	ResetExpireTime();
	AttachHitBox(25.0f, 5.2f, 0.5f, CAPSULE_COL);
	m_respawnTime = 0.0f;
	m_isColliding = false;
	SetTargeted(false);
	m_particle = false;
	m_activateTime = 0.0f;
}

BaseItem::~BaseItem()
{
}

void BaseItem::Input()
{
	BaseObject::Input();
}

void BaseItem::Update()
{
	BaseObject::Update();
	//if (GetActivatedTime() > 0.5f)
	//	m_dangerous = true;
}

void BaseItem::Render()
{
	if (GetActive())
		BaseObject::Render();
}

void BaseItem::Carried()
{
	if (GetIsCarried())
	{
		XMMATRIX newLocation = GetParent()->GetWorldMatrix();
		newLocation.r[3].m128_f32[0] = dynamic_cast<Runner*>(m_parent)->GetPushBox().m_Segment.m_Start.x;
		newLocation.r[3].m128_f32[1] = dynamic_cast<Runner*>(m_parent)->GetPushBox().m_Segment.m_Start.y;
		newLocation.r[3].m128_f32[2] = dynamic_cast<Runner*>(m_parent)->GetPushBox().m_Segment.m_Start.z;
		SetWorldMatrix(newLocation);
	}
}

void BaseItem::Action(XMFLOAT4X4* _serverMatrices)
{
}

bool BaseItem::CheckExpiration(float _delta)
{
	if (GetItemType() == TRAP_ITEM)
		return false;
	if (IsParticleActive())
	{
		float expireAt = 30.0f;
		switch (m_type)
		{
		case POCKET_SAND_ITEM:
			expireAt = 40.0f;
			break;
		case ADRENALINE_ITEM:
			break;
		case FREEZING_GRENADE_ITEM:
			break;
		case MAGNET_GRENADE_ITEM:
			expireAt = 200.0f;
			break;
		case BLAST_GRENADE_ITEM:
		{
			expireAt = 100.0f;
			break;
		}
		}
		AddToExpireTime(float(Graphics::GetTime().Delta()) * .6f);
		AddToActivatedTime(float(Graphics::GetTime().Delta()) * .6f);
		if (GetExpirationTime() > expireAt)
		{
			SetDangerous(false);
			SetIsColliding(false);
			SetParticleActive(false);
			m_expireTime = 0.0f;
			m_activateTime = 0.0f;
			return true;
		}
	}
	return false;
}

vector<unsigned int> BaseItem::CheckCollision(XMFLOAT3 *cP)
{
	vector<unsigned int> collidersIndex;

	for (unsigned int i = 0; i < ObjManager::GetObjects().size(); i++)
	{
		if (!ObjManager::AccessObject(i).GetHitBox())
			continue;

		if (&ObjManager::AccessObject(i) != this)
		{
			switch (ObjManager::AccessObject(i).GetHitBox()->type)
			{
			case AABB_COL:
			{
				Capsule *player = dynamic_cast<Capsule*>(GetHitBox());
				Sphere feet;
				feet.m_Center = player->m_Segment.m_Start;
				feet.m_Radius = player->m_Radius;

				XMFLOAT3 cP = { 0, 0, 0 };
				if (Collision::SphereToAABB(feet, *(dynamic_cast<AABB*>(ObjManager::AccessObject(i).GetHitBox())), &cP))
					collidersIndex.push_back(i);
				break;
			}
			}
		}
	}
	return collidersIndex;
}

int BaseItem::OnCollision()
{
#if 0
	XMFLOAT3 cP = { 0, 0, 0 };
	
	vector<unsigned int> colliderIndex = CheckCollision(&cP);
	for (unsigned int i = 0; i < colliderIndex.size(); ++i)
	{
		BaseObject* collidingWith = &ObjManager::AccessObject(colliderIndex[i]);
	
		if (collidingWith == NULL)
			return 0;
	
		// If the collision is happening with a capsule
		if (collidingWith->GetHitBox()->type == COL_TYPE::CAPSULE_COL)
			return CAPSULE_COL;
		else if (collidingWith->GetHitBox()->type == COL_TYPE::AABB_COL)
		{
			XMFLOAT3 f = { GetForce().x, 0.00f, GetForce().z };
			SetInAir(false);
			SetForce(f);
			return AABB_COL;
		}
	}
#endif
	return 0;
}

void BaseItem::Init()
{

}