#include "MagnetGranade.h"
#include "Particle.h"
#define PARTICLE 1
MagnetGranade::MagnetGranade(unsigned int _serverId) : BaseItem(_serverId)
{
	SetIsCarried(false);
	SetItemType(MAGNET_GRENADE_ITEM);
	SetParent(nullptr);
	InstantiateModel(Graphics::GetDevice(), "..\\NDEProject\\Assets\\OBJ Files\\Grenade.obj", XMFLOAT3(0.0f, 0.0f, 0.0f), 0, OBJECT);
	TextureObject(Graphics::GetDevice(), L"..\\NDEProject\\Assets\\Textures\\Grenades\\MagnetGrenade_Def.dds", 
		L"..\\NDEProject\\Assets\\Textures\\Grenades\\Grenade_Norm.dds", L"..\\NDEProject\\Assets\\Textures\\Grenades\\Grenade_Spec.dds");
	//AttachHitBox(2.0f, 2.0f, 0.0f, CAPSULE_COL);
#if PARTICLE
	m_particleEffect = new Particle();
	m_particleEffect->Initialize(L"..\\NDEProject\\Assets\\Textures\\PAR_EXPLOSION.dds", 
		XMFLOAT3{ GetWorldMatrix().r[3].m128_f32[0], GetWorldMatrix().r[3].m128_f32[1], GetWorldMatrix().r[3].m128_f32[2] },
		XMFLOAT3{ 30, 30, 30 }, XMFLOAT3{ 60, 60, 60 }, 200, 0.0001f, 0.5f, MAGNET);
#endif

	XMStoreFloat4x4(&m_cam, XMMatrixIdentity());
}

MagnetGranade::~MagnetGranade()
{
#if PARTICLE
	delete m_particleEffect;
#endif
}

void MagnetGranade::Input()
{
	BaseItem::Input();
#if PARTICLE
	m_particleEffect->Input();
#endif
}

void MagnetGranade::Update()
{
	if (!GetActive())
		return;

	BaseItem::Update();


#if PARTICLE

	if (IsParticleActive())
	{
		m_particleEffect->SetBillBoardVector(m_cam);
		m_particleEffect->SetPosition(XMFLOAT3(GetWorldMatrix().r[3].m128_f32[0], GetWorldMatrix().r[3].m128_f32[1] - 0.5f, GetWorldMatrix().r[3].m128_f32[2]));

		//srand((unsigned int)time(NULL));
		//float ex = float(rand() % 100) / 100;
		m_particleEffect->Update(true, XMFLOAT4{ .75f, .20f, .75f, 1.0f});
	}
#endif
}

void MagnetGranade::Render()
{
	if (!GetActive())
		return;

#if PARTICLE

	if (IsParticleActive())
		m_particleEffect->Render();
#endif

	if (!IsParticleActive())
		BaseItem::Render();
}

// Server function
void MagnetGranade::Action(XMFLOAT4X4* _serverMatrices)
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
		f.y = 3.0f;
		f.z = newWorld.m128_f32[2] + pf.z;
		SetForce(f);
		SetDangerous(true);
	}
}

int MagnetGranade::OnCollision()
{
	if (GetDangerous())
	{
		XMFLOAT3 f = GetForce();
		Capsule* hitBox = dynamic_cast<Capsule*>(GetHitBox());
		hitBox->m_Radius = 15.0f;
	}
	return 0;
}

void MagnetGranade::Init()
{
	m_particleEffect->ResetParticle();
	Capsule* hitBox = dynamic_cast<Capsule*>(GetHitBox());
	hitBox->m_Radius = 2.0f;
	SetCaster(GetParent());
	SetIsColliding(false);
	m_particleEffect->SetTransparency(0);
}