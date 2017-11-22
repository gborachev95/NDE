#include "Light.h"
#include "Collision.h"

#define LIGHT_VISUAL 0


#define CELL_WIDTH 5
#define CELL_HEIGHT 5
#define CELL_DEPTH 5
#define CELLS CELL_WIDTH * CELL_HEIGHT * CELL_DEPTH
Light::Light()
{
}

Light::Light(CComPtr<ID3D11Device> _device, LIGHT_TYPE _type, XMFLOAT4 _position, XMFLOAT3 _direction, float _radius, COLOR _color)
{
	m_type = _type;
	m_numLights = 1;

	m_lightData = new LIGHT_TO_VRAM;
    m_lightData->transform = _position;
	m_lightData->direction = _direction;
	m_lightData->radius = _radius;
	m_lightData->color.SetColor(_color.r, _color.g, _color.b, _color.a);
	CreateConstBuffer(_device);

#if LIGHT_VISUAL 
	// Debugging
	BaseObject* obj = new BaseObject(BASE_ID, 0.0f, false);
	obj->InstantiateModel(Graphics::GetDevice(), "..\\NDEProject\\Assets\\boneSphere.obj", XMFLOAT3(_position.x, _position.y, _position.z), 0, OBJECT);
	obj->SetWorldMatrix(XMMatrixIdentity());
	m_debugObj.push_back(obj);
#endif
}

Light::Light(CComPtr<ID3D11Device> _device, LIGHT_TYPE _type, vector<XMFLOAT4> _position, vector<XMFLOAT3> _direction, vector<float> _radius, vector<COLOR> _color, unsigned int _numLights, float _dist)
{
	m_bufferStruct = new BUFFER_STRUCT[CELLS];

	// Size of the grid
	XMFLOAT3 max = XMFLOAT3(157.937744f, 129.208206f, 101.357941f);
	XMFLOAT3 min = XMFLOAT3(-111.172050f, -14.9827614f, -178.151398f);
	XMFLOAT3 totalSize = XMFLOAT3(max.x - min.x, max.y - min.y, max.z - min.z);
	XMFLOAT3 cubeSize = XMFLOAT3(totalSize.x / CELL_WIDTH, totalSize.y / CELL_HEIGHT, totalSize.z / CELL_DEPTH);
	
	// Setting the light type and number
	m_type = _type;
	m_numLights = _numLights;

	// Making the lights
	m_lightData = new LIGHT_TO_VRAM[m_numLights];

	// Initializing the lights
	for (unsigned int i = 0; i < m_numLights; ++i)
	{
		m_lightData[i].transform = _position[i];
		m_lightData[i].direction = _direction[i];
		m_lightData[i].radius = _radius[i]; //7.209f;
		m_lightData[i].color.SetColor(_color[i].r, _color[i].g, _color[i].b, _color[i].a);
#if LIGHT_VISUAL 
		// Debugging
		BaseObject* obj = new BaseObject(BASE_ID, 0.0f, false);
		obj->InstantiateModel(Graphics::GetDevice(), "..\\NDEProject\\Assets\\boneSphere.obj", XMFLOAT3(_position[i].x, _position[i].y, _position[i].z), 0, OBJECT);
		obj->SetWorldMatrix(XMMatrixIdentity());
		m_debugObj.push_back(obj);
#endif
	}

	// Initializing the struct buffer
	for (unsigned int bufX = 0; bufX < CELL_WIDTH; ++bufX)
	{
		for (unsigned int bufY = 0; bufY < CELL_HEIGHT; ++bufY)
		{
			for (unsigned int bufZ = 0; bufZ < CELL_DEPTH; ++bufZ)
			{
				XMFLOAT4 cubePos = XMFLOAT4((
					(bufX * cubeSize.x) + min.x) + cubeSize.x * 0.5f,
					(bufY * cubeSize.y) + min.y + cubeSize.y * 0.5f,
					(bufZ * cubeSize.z) + min.z + cubeSize.z * 0.5f, 1.0f);

				XMFLOAT3 offsetRatio = XMFLOAT3(
					(cubePos.x - min.x) / totalSize.x,
					(cubePos.y - min.y) / totalSize.y,
					(cubePos.z - min.z) / totalSize.z);
				//Index = (ratio.z * depth) * Wcell * Hcell + (ratio.y * height) * width + (ratio.x * width)  
				unsigned int index = int(offsetRatio.z * CELL_DEPTH) * CELL_WIDTH * CELL_HEIGHT + int(offsetRatio.y * CELL_HEIGHT) * CELL_WIDTH + int(offsetRatio.x * CELL_WIDTH);
				// Hardcoding the position
				//m_lightData[index].transform = cubePos;
				// Color
				//m_lightData[index].color.SetColor((rand() % 10) + 0.1f, (rand() % 10) + 0.1f, (rand() % 10) + 0.1f, 1.0f);
#if 1
				for (unsigned int lightIndex = 0; lightIndex < m_numLights; ++lightIndex)
				{
					// Checking in which cell the light is in
					float dist = Collision::DistanceFormula(XMFLOAT3(cubePos.x, cubePos.y, cubePos.z),
						XMFLOAT3(m_lightData[lightIndex].transform.x, m_lightData[lightIndex].transform.y, m_lightData[lightIndex].transform.z));
					if (dist < _dist)
						m_bufferStruct[index].index = lightIndex;
					//else
					//	m_bufferStruct[index].index = 0;
				}
#endif
				//m_bufferStruct[index].index = index;
				m_bufferStruct[index].color[0] = (rand() % 10) + 0.1f;
				m_bufferStruct[index].color[1] = (rand() % 10) + 0.1f;
				m_bufferStruct[index].color[2] = (rand() % 10) + 0.1f;
				m_bufferStruct[index].color[3] = (rand() % 10) + 0.1f;
			}
		}
	}

	CreateConstBuffer(_device);
	CreateStructBuffer();
}

Light::~Light()
{
#if LIGHT_VISUAL 
	for (unsigned int i = 0; i < m_debugObj.size(); ++i)
		delete m_debugObj[i];
#endif

	if (m_numLights > 1)
		delete m_bufferStruct;

	delete m_lightData;
}

void Light::Input()
{
	ToggleLight(true);
	if (m_type == POINT_LIGHT)
	{
		for (unsigned int i = 0; i < m_numLights; ++i)
		{
			if (GetAsyncKeyState(VK_LEFT))
				m_lightData[i].transform.x += 0.1f;
			else if (GetAsyncKeyState(VK_RIGHT))
				m_lightData[i].transform.x -= 0.1f;

			if (GetAsyncKeyState(VK_UP))
				m_lightData[i].transform.z -= 0.1f;
			else if (GetAsyncKeyState(VK_DOWN))
				m_lightData[i].transform.z += 0.1f;

			if (GetAsyncKeyState('C'))
				m_lightData[i].transform.y += 0.1f;
			else if (GetAsyncKeyState('V'))
				m_lightData[i].transform.y -= 0.1f;

			if (GetAsyncKeyState('K'))
			{
				m_lightData[i].radius += 10.0f;
				if (m_lightData[i].radius > 1000.0f)
					m_lightData[i].radius = 1000.0f;
			}
			else if (GetAsyncKeyState('L'))
			{
				m_lightData[i].radius -= 10.0f;
				if (m_lightData[i].radius < 0.0f)
					m_lightData[i].radius = 0.0f;
			}
		}
	}
}

void Light::Update()
{
#if LIGHT_VISUAL 
	for (unsigned int i = 0; i < m_debugObj.size(); ++i)
		m_debugObj[i]->SetPosition(m_lightData[i].transform.x, m_lightData[i].transform.y, m_lightData[i].transform.z);
#endif
	if (!GetAsyncKeyState('I') && !GetAsyncKeyState('O') && !GetAsyncKeyState('P') && m_keyPressed)
		m_keyPressed = false;
}

void Light::Render(CComPtr<ID3D11DeviceContext> _deviceContext) const
{
#if LIGHT_VISUAL
	for (unsigned int i = 0; i < m_debugObj.size(); ++i)
	{
		if (m_lightData[i].color.a == 1.0f)
			m_debugObj[i]->Render();
	}
#endif
	MapLight(_deviceContext);
}

void Light::CreateConstBuffer(CComPtr<ID3D11Device> _device)
{
	if (m_numLights > 1)
	{
		D3D11_BUFFER_DESC constBufferDesc;
		ZeroMemory(&constBufferDesc, sizeof(D3D11_BUFFER_DESC));
		constBufferDesc.ByteWidth = sizeof(LIGHT_TO_VRAM) * m_numLights;
		constBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		constBufferDesc.StructureByteStride = sizeof(float);
		constBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		_device->CreateBuffer(&constBufferDesc, NULL, &m_constBuffer.p);
	}
	else
	{
		D3D11_BUFFER_DESC constBufferDesc;
		ZeroMemory(&constBufferDesc, sizeof(D3D11_BUFFER_DESC));
		constBufferDesc.ByteWidth = sizeof(LIGHT_TO_VRAM);
		constBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		constBufferDesc.StructureByteStride = sizeof(float);
		constBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		_device->CreateBuffer(&constBufferDesc, NULL, &m_constBuffer.p);
	}
}

void Light::MapLight(CComPtr<ID3D11DeviceContext> _deviceContext) const
{
	switch (m_type)
	{
	case DIR_LIGHT:
		_deviceContext->PSSetConstantBuffers(2, 1, &m_constBuffer.p);
		break;
	case POINT_LIGHT:
		_deviceContext->PSSetConstantBuffers(3, 1, &m_constBuffer.p);
		break;
	case SPOT_LIGHT:
		_deviceContext->PSSetConstantBuffers(4, 1, &m_constBuffer.p);
		break;
	}


	if (m_numLights > 1)
	{
		Graphics::GetDeviceContext()->PSSetShaderResources(4, 1, &m_resourceViewStruct.p);

		D3D11_MAPPED_SUBRESOURCE mapSubresource;
		ZeroMemory(&mapSubresource, sizeof(mapSubresource));
		_deviceContext->Map(m_constBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mapSubresource);
		memcpy(mapSubresource.pData, &(*m_lightData), sizeof(LIGHT_TO_VRAM) * m_numLights);
		_deviceContext->Unmap(m_constBuffer, NULL);
	}
	else
	{
		D3D11_MAPPED_SUBRESOURCE mapSubresource;
		ZeroMemory(&mapSubresource, sizeof(mapSubresource));
		_deviceContext->Map(m_constBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mapSubresource);
		memcpy(mapSubresource.pData, &(*m_lightData), sizeof(LIGHT_TO_VRAM));
		_deviceContext->Unmap(m_constBuffer, NULL);
	}
}

void Light::ToggleLight(bool _toggle)
{
		switch (m_type)
		{
		case DIR_LIGHT:
		{
			if (GetAsyncKeyState('O') && !m_keyPressed)
			{
				for (unsigned int i = 0; i < m_numLights; ++i)
				{
					if (m_lightData[i].color.a == 1.0f)
						m_lightData[i].color.a = 0.0f;
					else
						m_lightData[i].color.a = 1.0f;
				}
				m_keyPressed = true;
			}

			break;
		}
		case POINT_LIGHT:
		{
			if (GetAsyncKeyState('P') && !m_keyPressed)
			{
				for (unsigned int i = 0; i < m_numLights; ++i)
				{
					if (m_lightData[i].color.a == 1.0f)
						m_lightData[i].color.a = 0.0f;
					else
						m_lightData[i].color.a = 1.0f;
				}

				m_keyPressed = true;
			}
			break;
		}
		case SPOT_LIGHT:
		{
			if (GetAsyncKeyState('I') && !m_keyPressed)
			{
				for (unsigned int i = 0; i < m_numLights; ++i)
				{
					if (m_lightData[i].color.a == 1.0f)
						m_lightData[i].color.a = 0.0f;
					else
						m_lightData[i].color.a = 1.0f;
				}

				m_keyPressed = true;
			}
			break;
		}
	}
}

// Create struct buffer for screen 
void Light::CreateStructBuffer()
{
	
	// Creating the const buffer 
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.ByteWidth = sizeof(BUFFER_STRUCT) * CELLS;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bufferDesc.StructureByteStride = sizeof(BUFFER_STRUCT);
	bufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(D3D11_SUBRESOURCE_DATA));
	initData.pSysMem = m_bufferStruct;

	Graphics::GetDevice()->CreateBuffer(&bufferDesc, &initData, &m_structBuffer.p);


	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.ElementOffset = 0;
	srvDesc.Buffer.ElementWidth = CELLS;

	Graphics::GetDevice()->CreateShaderResourceView(m_structBuffer, &srvDesc, &m_resourceViewStruct.p);

}