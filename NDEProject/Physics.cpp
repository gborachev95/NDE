#include "Physics.h"
#include "Player.h"
#include "Collision.h"
#include "Monster.h"
#include "Runner.h"
#include "ObjManager.h"

Physics::Physics()
{
}


Physics::~Physics()
{
}

void Physics::ApplyForce(BaseObject* _obj, XMFLOAT4X4& worldMatrix, float _deltaTime, BaseObject* _level, unsigned int& _animOut)
{
	// Multipliers
	float mult = 3.0f;

	XMFLOAT3 force = _obj->GetForce();

	XMMATRIX w =_obj->GetWorldMatrix();
	w.r[3].m128_f32[0] += force.x * _obj->GetMass() * _deltaTime;
	w.r[3].m128_f32[2] += force.z * _obj->GetMass() * _deltaTime;

	_obj->SetWorldMatrix(w);
	WithinBounds(_obj,worldMatrix, _level, _deltaTime, _animOut);

	XMFLOAT3 play; XMStoreFloat3(&play, _obj->GetWorldMatrix().r[3]);

	force = _obj->GetForce();

	if (_obj->GetInAir())
	{
		force.y -= _obj->GetMass() * _deltaTime * 8.0f;
		worldMatrix._42 += force.y * _obj->GetMass() * _deltaTime;
	}
	else
		force.y = 0;

	worldMatrix._41 += force.x * _obj->GetMass() * _deltaTime;
	worldMatrix._43 += force.z * _obj->GetMass() * _deltaTime;

	if (!_obj->GetInAir())
	{
		force.x -= force.x * _obj->GetMass() * _deltaTime * mult;
		force.z -= force.z * _obj->GetMass() * _deltaTime * mult;
	}
	//else
	//{
	//	float velocityDec = 0.01f; // Alters the rate at which force is decremeted every pass
	//	force.x -= force.x * _obj->GetMass() * _deltaTime * mult * velocityDec;
	//	force.z -= force.z * _obj->GetMass() * _deltaTime * mult * velocityDec;
	//}
	_obj->SetForce(force);

	if (_obj->GetID() == RUNNER_ID && _obj->GetCurrAnimation()->GetName() == DEATH_ANIM)
	{
		XMVECTOR sideVector = { worldMatrix._11, worldMatrix._12, worldMatrix._13 };
		XMVECTOR colPoint = XMLoadFloat3(&dynamic_cast<Runner*>(_obj)->GetCollisionPoint());
		XMVECTOR angle = XMVector3Dot(colPoint, sideVector);
		XMMATRIX rot;


		if (angle.m128_f32[0] < 0.01f && angle.m128_f32[0] > -0.01f)
			return;

		if (angle.m128_f32[0] < 0.0f)
		{
			rot = XMMatrixMultiply(XMMatrixRotationY(angle.m128_f32[0] * _deltaTime * 6.0f), _obj->GetWorldMatrix());
			XMStoreFloat4x4(&worldMatrix, rot);
		}
		else
		{
			rot = XMMatrixMultiply(XMMatrixRotationY(angle.m128_f32[0] * _deltaTime * 6.0f), _obj->GetWorldMatrix());
			XMStoreFloat4x4(&worldMatrix, rot);
		}
	}
}

bool Physics::WithinBounds(BaseObject* _obj, DirectX::XMFLOAT4X4& worldMatrix, BaseObject* _level, float _deltaTime, unsigned int& _animOut)
{
	Capsule* hB = dynamic_cast<Capsule*>(_obj->GetHitBox());
	Monster* mon;
	BaseItem* item;
	if (_obj->GetID() == MONSTER_ID)
		mon = dynamic_cast<Monster*>(_obj);
	else if (_obj->GetID() == ITEM_ID)
	{
		item = dynamic_cast<BaseItem*>(_obj);
		if (item->GetItemType() == POCKET_SAND_ITEM)
			return false;
	}
	XMVECTOR hBPoint = XMLoadFloat3(&hB->m_Segment.m_End);
	XMFLOAT3 startPoint = hB->m_Segment.m_Start;
	XMFLOAT3 yEndPoint = hB->m_Segment.m_Start;
	yEndPoint.y += 4;
	XMFLOAT3 endPoint = hB->m_Segment.m_End;
	
	if (endPoint.y - startPoint.y < 2.5)
		endPoint.y += 2;
	//endPoint.y -= 1;

	XMFLOAT3 next = endPoint;
	next.x += _obj->GetForce().x * _obj->GetMass() * _deltaTime * 10;
	next.y -= 1;
	next.z += _obj->GetForce().z * _obj->GetMass() * _deltaTime * 10;

	startPoint.y -= 200.0f;
	//endPoint.y += 800.0f;

	XMVECTOR dir = XMLoadFloat3(&XMFLOAT3(0.0f, -FLT_MAX, 0.0f));
	float t = 0.0f;
	std::vector<Tri*> tris;
	
	int colCount = 0;
	Quad* temp = _level->GetQuad();
	//XMVECTOR outNorm;
	XMFLOAT3 forward;

	while (temp->m_quadrants.size() != 0)
	{
		if (worldMatrix._41 < temp->m_BL.m_max.x)
		{
			if (worldMatrix._43 < temp->m_BL.m_max.z)
				temp = temp->m_quadrants[BOTTOM_LEFT_QUAD];
			else
				temp = temp->m_quadrants[UP_LEFT_QUAD];
		}
		else
		{
			if (worldMatrix._43  < temp->m_BR.m_max.z)
				temp = temp->m_quadrants[BOTTOM_RIGHT_QUAD];
			else
				temp = temp->m_quadrants[UP_RIGHT_QUAD];
		}
	}

	tris = temp->m_tris;

	//vector<Tri*> ground;
	XMVECTOR cp1 = { -10000, -10000, -10000 }, cp2 = { -10000, -10000, -10000 };
	
	_obj->SetGround(nullptr);
	_obj->SetWall(nullptr);

	if (ObjManager::GetObjects()[ObjManager::GetObjects().size() - 2]->GetWorldMatrix().r[3].m128_f32[1] < 15.0f)
	{
		// Checking collision for start gates
		std::vector<Tri>* start = ObjManager::GetObjects()[ObjManager::GetObjects().size() - 3]->GetTris();

		if (_obj->GetID() == RUNNER_ID) // cage for runner
		{
			for (size_t i = 0; i < start->size(); i++)
				if (Collision::LineSegment2Triangle(cp1, XMLoadFloat4(&start[0][i].m_one.transform), XMLoadFloat4(&start[0][i].m_two.transform),
					XMLoadFloat4(&start[0][i].m_three.transform), XMLoadFloat3(&start[0][i].m_normal), XMLoadFloat3(&startPoint), XMLoadFloat3(&yEndPoint), 0.0f))
					_obj->SetGround(&ObjManager::GetObjects()[ObjManager::GetObjects().size() - 3]->GetTris()[0][i]);
		}

	    start = ObjManager::GetObjects()[ObjManager::GetObjects().size() - 2]->GetTris();
		for (size_t i = 0; i < start->size(); i++) // cage for monsters
			if (Collision::LineSegment2Triangle(cp2, XMLoadFloat4(&start[0][i].m_one.transform), XMLoadFloat4(&start[0][i].m_two.transform),
				XMLoadFloat4(&start[0][i].m_three.transform), XMLoadFloat3(&start[0][i].m_normal), XMLoadFloat3(&endPoint), XMLoadFloat3(&next), hB->m_Radius)
				&& Collision::WallSafetyCheck(_obj, &start[0][i]))
			{
				_obj->SetForce(XMFLOAT3(0, _obj->GetForce().y, 0));
				_obj->SetWall(&ObjManager::GetObjects()[ObjManager::GetObjects().size() - 2]->GetTris()[0][i]);
				Collision::WallCollision(_obj, _deltaTime);
			}
	}

	for (size_t i = 0; i < tris.size(); i++)
	{
		if (tris[i]->m_normal.y > 0.5f && Collision::LineSegment2Triangle(cp1, XMLoadFloat4(&tris[i]->m_one.transform), XMLoadFloat4(&tris[i]->m_two.transform),
			XMLoadFloat4(&tris[i]->m_three.transform), XMLoadFloat3(&tris[i]->m_normal), XMLoadFloat3(&startPoint), XMLoadFloat3(&yEndPoint), 0.0f))
		//if (tris[i]->m_normal.y > 0.7f && Collision::LineSegment2Triangle(cp, tris[i], startPoint, endPoint, 0.0f))
		{
				_obj->SetGround(tris[i]);
		}
		//if (Collision::IntersectMovingSphereTriangle(XMLoadFloat4(&tris[i]->m_one.transform), XMLoadFloat4(&tris[i]->m_two.transform),
		//	XMLoadFloat4(&tris[i]->m_three.transform), XMLoadFloat3(&tris[i]->m_normal), XMLoadFloat3(&next), XMLoadFloat3(&forward),
		//	hB->m_Radius, t, outNorm))
		else if (tris[i]->m_normal.y < 0.7f && Collision::LineSegment2Triangle(cp2, XMLoadFloat4(&tris[i]->m_one.transform), XMLoadFloat4(&tris[i]->m_two.transform),
			XMLoadFloat4(&tris[i]->m_three.transform), XMLoadFloat3(&tris[i]->m_normal), XMLoadFloat3(&endPoint), XMLoadFloat3(&next), hB->m_Radius)
			)//&& Collision::WallSafetyCheck(_obj, tris[i]))
		{
			colCount++;

			if (_obj->GetID() == ITEM_ID)
				item->SetIsColliding(true);

			_obj->SetWall(tris[i]);
			//if (!_obj->GetGround())
			//{
			//	worldMatrix._41 = cp2.m128_f32[0] + tris[i]->m_normal.x * hB->m_Radius;
			//	worldMatrix._43 = cp2.m128_f32[2] + tris[i]->m_normal.z * hB->m_Radius;
			//}
			Collision::WallCollision(_obj, _deltaTime);
		}
	}
	XMFLOAT3 cp = { -10000, -10000, -10000 };
	XMStoreFloat3(&cp, cp1);

	//if (_obj->GetWall())
	//	Collision::WallCollision(_obj, _deltaTime);
	//if (cp.y == -10000 && _obj->GetWall())
	//	Collision::WallCollision(_obj, _deltaTime);

	if (_obj->GetInAir())
	{
		if (_obj->GetID() != ITEM_ID && !_obj->GetCurrAnimation()->GetDeath() && !dynamic_cast<Player*>(_obj)->GetIsStunned())
		{
			// Specifying the distance to start a landing animation based on player's downward velocity
			if (hB->m_Segment.m_Start.y - hB->m_Radius <= cp.y + 5.0f * -_obj->GetForce().y / 4.5f)
			{

				if (_obj->GetID() == MONSTER_ID && mon->GetIsDeadly())
				{
					_animOut = POUNCE_LANDING_ANIM;
					if (_obj->GetNextAnimation()->GetName() != POUNCE_LANDING_ANIM)
					{
						_obj->GetAnimations()[POUNCE_LANDING_ANIM]->SetTime(5);
						if (_obj->GetAnimBusy())
							_obj->SetAnimBusy(false);
					}
				}
				else
				{
					_animOut = LANDING_ANIM;
					if (_obj->GetNextAnimation()->GetName() != LANDING_ANIM)
					{
						_obj->GetAnimations()[LANDING_ANIM]->SetTime(5);
						if (_obj->GetAnimBusy())
							_obj->SetAnimBusy(false);
					}

				}
			}

			if (_animOut != POUNCE_LANDING_ANIM && _obj->GetID() == MONSTER_ID && mon->GetIsDeadly())
				_animOut = POUNCE_IDLE_ANIM;
			else if (_animOut != LANDING_ANIM && _animOut != POUNCE_LANDING_ANIM)
				_animOut = FALLING_ANIM;
		}
	}
	// This happens only for items 
	if (_obj->GetID() == ITEM_ID)
	{
		if (worldMatrix._42 - cp.y < 3.2f && _obj->GetForce().y <= 1.0f)
		{
			_obj->SetInAir(false);
			XMMATRIX worldObject = _obj->GetWorldMatrix();
			if (cp.y > -100) // If out of bounds, don't set your y to garbage
				worldObject.r[3].m128_f32[1] = cp.y + 3.0f;
			XMStoreFloat4x4(&worldMatrix, worldObject);

			if (item->GetActive())
				item->SetIsColliding(true);
		}
		else
			_obj->SetInAir(true);
	}
	else
	{
		// This happens only for players
		if (cp.y == -10000) // if out of bounds, don't set your y to garbage
			return false;

		else if (worldMatrix._42 - cp.y < .2f && _obj->GetForce().y <= 1.0f)
		{
			if (_obj->GetInAir())
				_obj->SetAnimBusy(false);

			_obj->SetInAir(false);
			XMMATRIX worldObject = _obj->GetWorldMatrix();
			worldObject.r[3].m128_f32[1] = cp.y;
			XMStoreFloat4x4(&worldMatrix, worldObject);

			if (_obj->GetID() == MONSTER_ID)
				mon->SetIsDeadly(false);
		}
		else
			_obj->SetInAir(true);
	}
	
	//printf("%i", colCount);
	
	if (colCount > 0)
		return true;

	return false;
}

