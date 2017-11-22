#include "PocketSand.h"
#include "Graphics.h"
#include "Runner.h"
#include "Particle.h"
#include "Camera.h"

#define PARTICLE 1

PocketSand::PocketSand(unsigned int _serverId) : BaseItem(_serverId)
{
	SetIsCarried(false);
	SetItemType(POCKET_SAND_ITEM);
	SetParent(nullptr);
	InstantiateModel(Graphics::GetDevice(), "..\\NDEProject\\Assets\\ITEM_PocketSand.obj", XMFLOAT3(0.0f, 0.0f, 0.0f), 0, OBJECT);
	TextureObject(Graphics::GetDevice(), L"..\\NDEProject\\Assets\\Textures\\DEF_SAND_TEXTURE.dds");
//	AttachHitBox(2.0f, 2.0f, 0.0f, CAPSULE_COL);
	m_caster = nullptr;
#if PARTICLE
	m_particleEffect = new Particle();
	m_particleEffect->Initialize(L"..\\NDEProject\\Assets\\Textures\\PAR_CORE2.dds", XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3{ 1, 1, 1 }, XMFLOAT3{ 1, 1, 1 }, 50, 0.2f, 0.25f, WEIGHTED);
#endif

	//XMStoreFloat4x4(&m_cam, XMMatrixIdentity());
}

PocketSand::~PocketSand()
{
#if PARTICLE
	delete m_particleEffect;
#endif
}

void PocketSand::Input()
{
	BaseItem::Input();
#if PARTICLE
	m_particleEffect->Input();
#endif

}

void PocketSand::Update()
{
	if (!GetActive())
		BaseItem::Update();

#if PARTICLE
	//m_particleEffect->GetParticle()->SetWorldMatrix(XMMatrixMultiply(XMMatrixRotationY(float(Graphics::GetTime().Delta())), m_particleEffect->GetParticle()->GetWorldMatrix()));
	
	//if (m_caster->GetCurrAnimation()->GetName() == PUSHED_ANIM && m_caster->GetCurrAnimation()->GetCurrFrame()->m_frameNum > m_caster->GetCurrAnimation()->GetOutOfBlendFrame())

	m_particleEffect->SetBillBoardVector(m_cam);
	
	if (IsParticleActive())
		m_particleEffect->Update(false, XMFLOAT4{ .75f, .55f, .25f, 1.0f });
	else
	{
		m_particleEffect->SetPosition(XMFLOAT3(GetWorldMatrix().r[3].m128_f32[0], GetWorldMatrix().r[3].m128_f32[1] - 0.5f, GetWorldMatrix().r[3].m128_f32[2]));
		m_particleEffect->Update(true, XMFLOAT4{ .75f, .55f, .25f, 1.0f });
	}
#endif
}

void PocketSand::Render()
{
	if (!GetActive())
		return;

//#if PARTICLE
	if (IsParticleActive() || GetParent())
		m_particleEffect->Render();
//#endif
	//if (GetDangerous() || GetIsAvailable())
	//	return;
	BaseItem::Render();
}

// Server function
void PocketSand::Action(XMFLOAT4X4* _serverMatrices, XMFLOAT3 _force)
{
	if (GetIsCarried())
	{
		//XMFLOAT3 pf = GetParent()->GetForce();
		//

		XMFLOAT3 f = GetForce();
		XMFLOAT3 pf = GetParent()->GetForce();
		XMVECTOR newWorld = GetWorldMatrix().r[3];
		XMVECTOR forward = GetParent()->GetWorldMatrix().r[2];

		newWorld.m128_f32[0] = forward.m128_f32[0] * 20.0f;
		newWorld.m128_f32[1] = forward.m128_f32[1] * 20.0f;
		newWorld.m128_f32[2] = forward.m128_f32[2] * 20.0f;

		newWorld = XMVectorSubtract(forward, newWorld);

		f.x = newWorld.m128_f32[0] + pf.x;
		f.y = 3.0f;
		f.z = newWorld.m128_f32[2] + pf.z;

		//_force.x *= -1.8f;
		//_force.y = 17 - _force.y;
		////_force.y *= -1;
		//_force.z *= -1.8f;
		SetForce(f);
		SetDangerous(true);

		Capsule* hitBox = dynamic_cast<Capsule*>(GetHitBox());
		hitBox->m_Radius *= 3.0f;
		hitBox->m_Segment.m_Start.y = 0.0f;
		hitBox->m_Segment.m_End.y = 0.0f;

		m_particleEffect->SetTransparency(0);
		m_particleEffect->Initialize(L"..\\NDEProject\\Assets\\Textures\\PAR_CORE2.dds", XMFLOAT3(0.0f, 0.0f, 0.0f), 
			XMFLOAT3{ 30, 20, -10 }, XMFLOAT3{ 60, 1, 30 }, 50, 0.007f, 0.25f, WEIGHTED);
	}
}

int PocketSand::OnCollision()
{
	if (GetDangerous())
	{
 		XMFLOAT3 f = GetForce();
		if (f.x < 0.5f && f.x > -0.5f && f.z < 0.5f && f.z > -0.5f)
		{
			Capsule* hitBox = dynamic_cast<Capsule*>(GetHitBox());
			hitBox->m_Radius = 3.0f;
			hitBox->m_Segment.m_Start.y = 2.0f;
			hitBox->m_Segment.m_End.y = 0.0f;
			SetDangerous(true);
		}
	}
	return 0;
}

void PocketSand::Init()
{
	SetCaster(GetParent());
	GetParticle()->Initialize(L"..\\NDEProject\\Assets\\Textures\\PAR_CORE2.dds", XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3{ 1, 1, 1 }, XMFLOAT3{ 1, 1, 1 }, 50, 0.2f, 0.25f, WEIGHTED);
	m_particleEffect->ResetParticle();
	SetIsColliding(false);
	m_particleEffect->SetTransparency(0);
}