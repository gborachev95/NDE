#include "Button.h"
#include "Graphics.h"
#include "Collision.h"
#include <iostream>

Button::Button()
{
	InstantiateModel(Graphics::GetDevice(), "..\\NDEProject\\Assets\\OBJ Files\\MAIN_ButtonUI.obj", XMFLOAT3(0.0f, 0.0f, 0.0f), 1.0f, UI);
	AttachHitBox(AABB_COL);
}

Button::Button(float _x, float _y, float _z, BUTTON_TYPE _type)
{
	InstantiateModel(Graphics::GetDevice(), "..\\NDEProject\\Assets\\OBJ Files\\MAIN_Button.obj", XMFLOAT3(_x, _y,_z), 1.0f, OBJECT);
	AttachHitBox(AABB_COL);
	
	switch (_type)
	{
	case PLAY_BUTTON:
		TextureObject(Graphics::GetDevice(), L"..\\NDEProject\\Assets\\Button Textures\\DEF_PLAY_BUTTON_MAP.dds", L"..\\NDEProject\\Assets\\Button Textures\\NORM_PLAY_BUTTON_MAP.dds");
		break;
	case START_BUTTON:
		TextureObject(Graphics::GetDevice(), L"..\\NDEProject\\Assets\\Button Textures\\DEF_START_BUTTON_MAP.dds", L"..\\NDEProject\\Assets\\Button Textures\\NORM_START_BUTTON_MAP.dds");
		break;
	case OPTIONS_BUTTON:
		TextureObject(Graphics::GetDevice(), L"..\\NDEProject\\Assets\\Button Textures\\DEF_OPTIONS_BUTTON_MAP.dds", L"..\\NDEProject\\Assets\\Button Textures\\NORM_OPTIONS_BUTTON_MAP.dds");
		break;
	case CREDITS_BUTTON:
		TextureObject(Graphics::GetDevice(), L"..\\NDEProject\\Assets\\Button Textures\\DEF_CREDITS_BUTTON.dds", L"..\\NDEProject\\Assets\\Button Textures\\NORM_CREDITS_BUTTON.dds");
		break;
	case EXIT_BUTTON:
		TextureObject(Graphics::GetDevice(), L"..\\NDEProject\\Assets\\Button Textures\\DEF_EXIT_BUTTON_MAP.dds", L"..\\NDEProject\\Assets\\Button Textures\\NORM_EXIT_BUTTON_MAP.dds");
		break;
	case JOIN_BUTTON:
		TextureObject(Graphics::GetDevice(), L"..\\NDEProject\\Assets\\Button Textures\\DEF_JOIN_BUTTON_MAP.dds", L"..\\NDEProject\\Assets\\Button Textures\\NORM_JOIN_BUTTON_MAP.dds");
		break;
	case HOST_BUTTON:
		TextureObject(Graphics::GetDevice(), L"..\\NDEProject\\Assets\\Button Textures\\DEF_HOST_BUTTON_MAP.dds", L"..\\NDEProject\\Assets\\Button Textures\\NORM_HOST_BUTTON_MAP.dds");
		break;
	case BACK_BUTTON:
		TextureObject(Graphics::GetDevice(), L"..\\NDEProject\\Assets\\Button Textures\\DEF_BACK_BUTTON_MAP.dds", L"..\\NDEProject\\Assets\\Button Textures\\NORM_BACK_BUTTON.dds");
		break;
	case RESUME_BUTTON:
		TextureObject(Graphics::GetDevice(), L"..\\NDEProject\\Assets\\Button Textures\\DEF_RESUME_BUTTON_MAP.dds", L"..\\NDEProject\\Assets\\Button Textures\\NORM_RESUME_BUTTON_MAP.dds");
		break;
	default:
		break;
	}

	m_type = _type;
	m_scaled = false;
}

Button::~Button()
{

}

void Button::Input()
{
	BaseUI::Input();
}

void Button::Update()
{
	BaseUI::Update();
}

void Button::Render()
{
	BaseUI::Render();
}

bool Button::OnCollision()
{
	XMMATRIX selected = XMMatrixIdentity();
	selected = XMMatrixMultiply(XMMatrixRotationY(BaseObject::GetTimerDelta()), GetWorldMatrix());

	if (!m_scaled)
	{
		m_scaled = true;
		selected = XMMatrixMultiply(XMMatrixScaling(1.6f, 1.6f, 1.6f), selected);
	}
	selected.r[3] = GetWorldMatrix().r[3];
	SetWorldMatrix(selected);

	return true;
}

bool Button::DetectCollision()
{
	float mouseZ = -20.0f;
	if (GetGraphicsType() == UI)
		mouseZ = 0.0f;

	POINT mousePosition;
	GetCursorPos(&mousePosition);
	ScreenToClient(Graphics::GetWindow(), &mousePosition);
	XMFLOAT3 position = XMFLOAT3(float(mousePosition.x), float(mousePosition.y), mouseZ);
	// Converting to a ratio
	position.x /= Graphics::GetBackBufferWidth();
	position.y /= Graphics::GetBackBufferHeight();

	if (GetGraphicsType() != UI)
	{
		XMFLOAT3 cP = { 0, 0, 0 };
		//position.x = (float((Graphics::GetBackBufferWidth() * 0.5f) - position.x) * 0.05f) + 5.0f;
		//position.y = (float((Graphics::GetBackBufferHeight() * 0.5f) - position.y) * 0.05f) + 5.0f;
		// Converting te ratio to be between -25 and 25
		position.x = ((-50 * position.x) + 25);
		position.y = ((-50 * position.y) + 25) * (-1);

		Sphere mouseCollider;
		mouseCollider.m_Center = position;
		mouseCollider.m_Radius = 3.27f;
		if (Collision::SphereToAABB(mouseCollider, *(dynamic_cast<AABB*>(GetHitBox())), &cP))
			return OnCollision();
		else
		{
			XMMATRIX notSelected = XMMatrixIdentity();
			notSelected.r[3] = GetWorldMatrix().r[3];
			SetWorldMatrix(notSelected);

			m_scaled = false;
		}
	}
	else
	{
		XMVECTOR buttonPos = GetWorldMatrix().r[3];
		// Converting te ratio to be between -1 and 1
		position.x = ((-2 * position.x) + 1) * (-1);
		position.y = (-2 * position.y) + 1;
		// Getting the distance between the two points
		XMVECTOR mouseToButton = XMVectorSubtract(buttonPos, XMLoadFloat3(&position));
	    // Checking the distance
		if (abs(mouseToButton.m128_f32[0]) < 0.25f && abs(mouseToButton.m128_f32[1]) < 0.25f)
			return OnCollision();
		else
		{
			XMMATRIX notSelected = XMMatrixIdentity();
			notSelected = XMMatrixMultiply(XMMatrixScaling(0.1f, 0.05f, 0.1f), notSelected);
			notSelected.r[3] = GetWorldMatrix().r[3];
			SetWorldMatrix(notSelected);

			m_scaled = false;
		}
	}
	return false;
}
