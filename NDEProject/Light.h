#pragma once
#ifndef Light_H
#define Light_H
#include "includes.h"
#include "BaseObject.h"

enum LIGHT_TYPE { DIR_LIGHT, POINT_LIGHT, SPOT_LIGHT,MAX_LIGHT };

class Light
{
	bool                              m_keyPressed;
	LIGHT_TYPE                        m_type;
	LIGHT_TO_VRAM*                    m_lightData;
	CComPtr<ID3D11Buffer>             m_constBuffer;
	CComPtr<ID3D11ShaderResourceView> m_resourceViewStruct;
    CComPtr<ID3D11Buffer>             m_structBuffer;
	vector<BaseObject*>               m_debugObj;
	unsigned int                      m_numLights;
	BUFFER_STRUCT                     *m_bufferStruct;
private:
	void CreateStructBuffer();
    void CreateConstBuffer(CComPtr<ID3D11Device> _device);
public:
	Light();
	Light(CComPtr<ID3D11Device> _device, LIGHT_TYPE _type, XMFLOAT4 _position, XMFLOAT3 _direction, float _radius, COLOR _color);
	Light(CComPtr<ID3D11Device> _device, LIGHT_TYPE _type, std::vector<XMFLOAT4> _position, std::vector<XMFLOAT3> _direction, std::vector<float> _radius, std::vector<COLOR> _color, unsigned int _numLights, float _dist);

	~Light();

	void Input();
	void Update();
	void Render(CComPtr<ID3D11DeviceContext> _deviceContext) const;
	void ToggleLight(bool _toggle);
	void MapLight(CComPtr<ID3D11DeviceContext> _deviceContext) const;

	// Setters
	void SetNumLights(unsigned int _numLights) { m_numLights = _numLights; }
	void SetLightStatus(float _status) { m_lightData[0].color.SetColor(m_lightData[0].color.r, m_lightData[0].color.g, m_lightData[0].color.b,_status); }
	// Getters
	unsigned int GetNumLights() { return m_numLights; }
};
#endif
