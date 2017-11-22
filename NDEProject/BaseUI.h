#pragma once
#include "BaseObject.h"
class BaseUI : public BaseObject
{
	MOD_TO_VRAM           m_uiToGPU;
	CComPtr<ID3D11Buffer> m_UIConstBuffer;
public:
	BaseUI();
	~BaseUI();
	void Input();
	void Update();
	void Render();

	// Setters
	void SetToVramData(MOD_TO_VRAM _uiToVram) { m_uiToGPU = _uiToVram; }

	void CreateConstBuffer();

};

