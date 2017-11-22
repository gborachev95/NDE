#include "KeyItem.h"
#include "Graphics.h"

KeyItem::KeyItem(unsigned int _serverId) : BaseItem(_serverId)
{
	SetIsCarried(false);
	SetItemType(KEY_ITEM);
	SetParent(nullptr);
	InstantiateModel(Graphics::GetDevice(), "..\\NDEProject\\Assets\\KeyBIG.obj", XMFLOAT3(0.0f, 0.0f, 0.0f), 0, OBJECT);
	TextureObject(Graphics::GetDevice(), L"..\\NDEProject\\Assets\\Textures\\DEF_KEY_TEXTURE.dds");//DEF_CHEETO_TEXTURE.dds");
	//AttachHitBox(25.0f, -10000.0f, -10000.0f, CAPSULE_COL);
}

KeyItem::~KeyItem()
{
}

void KeyItem::Input()
{
	BaseItem::Input();
}

void KeyItem::Update()
{
	BaseItem::Update();
}

void KeyItem::Render()
{
	BaseItem::Render();
}

void KeyItem::Action(XMFLOAT4X4* _serverMatrices)
{
	if (GetIsCarried())
	{

	}
}
int KeyItem::OnCollision()
{
	return BaseItem::OnCollision();
}
