#include "Trap.h"



Trap::Trap(unsigned int _serverId) : BaseItem(_serverId)
{
	SetIsCarried(false);
	SetItemType(TRAP_ITEM);
	SetParent(nullptr);
	InstantiateModel(Graphics::GetDevice(), "..\\NDEProject\\Assets\\shield.obj", XMFLOAT3(0.0f, 0.0f, 0.0f), 0, OBJECT);
	TextureObject(Graphics::GetDevice(), L"..\\NDEProject\\Assets\\Textures\\PAR_FIRE.dds");
	//AttachHitBox(2.0f, 2.0f, 0.0f, CAPSULE_COL);
}


Trap::~Trap()
{
}

void Trap::Input()
{
	BaseItem::Input();
}

void Trap::Update()
{
	BaseItem::Update();
}

void Trap::Render()
{
	BaseItem::Render();
}
	 
void Trap::Action(XMFLOAT4X4* _serverMatrices)
{
	if (GetIsCarried())
	{
		XMFLOAT3 f = GetForce();
		XMFLOAT3 pf = GetParent()->GetForce();
		XMVECTOR newWorld = GetWorldMatrix().r[3];
		XMVECTOR forward = GetParent()->GetWorldMatrix().r[2];

		newWorld.m128_f32[0] = forward.m128_f32[0] * 20.0f;
		newWorld.m128_f32[1] = forward.m128_f32[1] * 20.0f;
		newWorld.m128_f32[2] = forward.m128_f32[2] * 20.0f;

		newWorld = XMVectorSubtract(forward, newWorld);

		f.x = newWorld.m128_f32[0] + pf.x;
		f.y = 5.0f;
		f.z = newWorld.m128_f32[2] + pf.z;
		SetForce(f);
		//SetDangerous(true);

	}
}

int Trap::OnCollision()
{
	if (GetDangerous())
	{
		XMFLOAT3 f = GetForce();
		if (f.x < 0.5f && f.x > -0.5f && f.z < 0.5f && f.z > -0.5f)
		{
			Capsule* hitBox = dynamic_cast<Capsule*>(GetHitBox());
			hitBox->m_Radius = 15.0f;
			hitBox->m_Segment.m_Start.y = 2.0f;
			hitBox->m_Segment.m_End.y = 0.0f;
		}
	}
	return 0;
}