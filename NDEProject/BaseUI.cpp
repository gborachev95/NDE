#include "BaseUI.h"

BaseUI::BaseUI() : BaseObject(BASE_ID, 0.0f, false)
{
	m_uiToGPU.color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_uiToGPU.uvTranslation = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	CreateConstBuffer(); // HINT:: DOES NOT NEED THAT MANY CONST BUFFERS
}

BaseUI::~BaseUI()
{
}

void BaseUI::Input()
{
	BaseObject::Input();
}

void BaseUI::Update()
{
	m_uiToGPU.uvTranslation.x += /*0.0078f;*/float(Graphics::GetTime().Delta()) * 10.1f;

	if (m_uiToGPU.uvTranslation.x > 1.0f)
	{
		m_uiToGPU.uvTranslation.x = 0.0f;

		//m_uiToGPU.uvTranslation.y += 0.0078f;
		//if (m_uiToGPU.uvTranslation.y > 1.0f)
		//	m_uiToGPU.uvTranslation.y = 0.0f;
	}


	BaseObject::Update();
}

void BaseUI::Render()
{
	if (BaseObject::GetGraphicsType() == UI)
	{
		D3D11_MAPPED_SUBRESOURCE mapSubresource;
		Graphics::GetDeviceContext()->PSSetConstantBuffers(1, 1, &m_UIConstBuffer.p);
		// Bones data
		ZeroMemory(&mapSubresource, sizeof(mapSubresource));
		Graphics::GetDeviceContext()->Map(m_UIConstBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mapSubresource);
		memcpy(mapSubresource.pData, &m_uiToGPU, sizeof(MOD_TO_VRAM));
		Graphics::GetDeviceContext()->Unmap(m_UIConstBuffer, NULL);
	}
	BaseObject::Render();
}

void BaseUI::CreateConstBuffer()
{
	D3D11_BUFFER_DESC constBufferDesc;
	ZeroMemory(&constBufferDesc, sizeof(D3D11_BUFFER_DESC));
	constBufferDesc.ByteWidth = sizeof(MOD_TO_VRAM);
	constBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	constBufferDesc.StructureByteStride = sizeof(float);
	constBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	Graphics::GetDevice()->CreateBuffer(&constBufferDesc, NULL, &m_UIConstBuffer.p);
}
