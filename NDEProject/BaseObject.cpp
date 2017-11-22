#include "BaseObject.h"
#include "..//FBXInporter//FBXLib.h"
#include "DDSTextureLoader.h"
#include <fstream>
#include "includes.h"
#include "Blender.h"
#include "Player.h"
#include "Runner.h"

#define RENDER_BONES 0
#define WIREFRAME 0

BaseObject::BaseObject()
{
	m_nextAnimation = NULL;
	m_lastNextAnimation = NULL;
	m_force = { 0, 0, 0 };
	m_currBlender = NULL;
	m_hBox = nullptr;
	m_animBusy = false;
	m_quad = nullptr;
	m_ground = nullptr;
	m_wall = nullptr;
	m_inAir = false;
	m_vertecies = nullptr;
	m_active = true;
	m_numInstances = 0;
	m_target = false;
}

BaseObject::BaseObject(OBJECT_ID _id, float _mass, bool _inAir)
{
	m_nextAnimation = NULL;
	m_lastNextAnimation = NULL;
	m_force = { 0, 0, 0 };
	m_currBlender = NULL;
	m_id = _id;
	m_mass = _mass;
	m_inAir = _inAir;
	m_hBox = nullptr;
	m_animBusy = false;
	m_quad = nullptr;
	m_ground = nullptr;
	m_wall = nullptr;
	m_vertecies = nullptr;
	m_active = true;
	m_numInstances = 0;
	m_target = false;
}

BaseObject::~BaseObject()
{
#if RENDER_BONES
	for (unsigned int i = 0; i < m_bonesObjects.size(); ++i)
		delete	m_bonesObjects[i];
#endif
	if (m_vertecies)
	{
		delete m_vertecies;
		m_vertecies = nullptr;
	}

	if (m_indexList)
	{
		delete m_indexList;
		m_indexList = nullptr;
	}
	if (GetID() == RUNNER_ID)
	{
		{
			delete m_indexList;
			m_indexListINV = nullptr;
		}
	}

	if (m_hBox)
	{
		delete m_hBox;
		m_hBox = nullptr;
	}

	if (m_quad)
	{
		unsigned int num = m_quad->d_list.size();
		for (size_t i = 0; i < num; i++)
			delete m_quad->d_list[i];
		delete m_quad;
		m_quad = nullptr;
	}
	else if (m_tris.size() > 0)
	{
		m_tris.clear();
	}

	for (size_t i = 0; i < m_animations.size(); i++)
		delete m_animations[i];
	m_animations.clear();

	if (m_numInstances != 0)
		delete m_instanceMatricis;
	
	Blender::ClearBlenders();
}

vector<Transform>& BaseObject::GetBone() 
{ 
	return m_interpedFrame.m_bones; 
}

void BaseObject::InstantiateModel(ID3D11Device* _device, std::string _filePath, XMFLOAT3 _position, float _shine, GRAPHICS _type)
{
	// FOR DEBUGGING
	//printf("%s\n", _filePath);
	m_graphicsType = _type;
	//FBXImporter::ExportObject(_filePath, _shine);
	//LoadBinaryFile(_filePath);
	ReadObject(_filePath, _shine);
	CreateVertexBuffer(_device);
	CreateIndexBuffer(_device);
	CreateConstBuffer(_device);
    XMStoreFloat4x4(&m_worldToShader.worldMatrix, XMMatrixIdentity());

	SetPosition(_position.x, _position.y, _position.z);
}

// Instantiates the object using an FBX file
void BaseObject::InstantiateFBX(ID3D11Device* _device, std::string _filePath, XMFLOAT3 _position, float _shine, GRAPHICS _type)
{
	// Gets us the function from the DLL
	//OpenFBXFile LoadFBXFile = LinkFBXDll();
	m_graphicsType = _type;

	//m_currAnimation = new Animation();
	//FBXImporter::LoadFBXFile(_filePath, temp_vertices, vertexIndices, m_bindBones, *m_currAnimation);
	
	// Loading from binary
	//temp_vertices.clear();
	//vertexIndices.clear();
	//m_bindBones.clear();
	//m_currAnimation.m_keyFrame.clear();
	//LoadBinaryFile(_filePath, temp_vertices, vertexIndices);
	//LoadBinaryFile(_filePath, m_bindBones);
	//LoadBinaryFile(_filePath, m_currAnimation);

#if WIREFRAME
	// wire frame
	if (m_id == RUNNER_ID)
	{
		D3D11_RASTERIZER_DESC rs_solidDescrip;
		rs_solidDescrip.AntialiasedLineEnable = false;
		rs_solidDescrip.CullMode = D3D11_CULL_BACK;
		rs_solidDescrip.DepthBias = 0;
		rs_solidDescrip.DepthBiasClamp = 0.0f;
		rs_solidDescrip.DepthClipEnable = true;
		rs_solidDescrip.FillMode = D3D11_FILL_SOLID;
		rs_solidDescrip.FrontCounterClockwise = false;
		rs_solidDescrip.MultisampleEnable = false;
		rs_solidDescrip.ScissorEnable = false;
		rs_solidDescrip.SlopeScaledDepthBias = 0.0f;
	
		D3D11_RASTERIZER_DESC rs_wireframeDescrip = rs_solidDescrip;
		rs_wireframeDescrip.FillMode = D3D11_FILL_WIREFRAME;
		_device->CreateRasterizerState(&rs_wireframeDescrip, &d_rsWireframe);
	}
#endif

	if (_type == ANIMATED)
		LoadAnimations(_filePath, _shine);
	else
	{
		vector<unsigned int> vertexIndices, vertexIndiciesINV, uvIndices, normalIndices;
		vector<VERTEX> temp_vertices;
		vector<vector<VERTEX>> temp_tris;
		Animation newAnim;
		newAnim.SetTime(-1);
		FBXImporter::LoadFBXFile(_filePath, temp_vertices, temp_tris, vertexIndices, vertexIndiciesINV, m_bindBones, newAnim);

		m_numVerts = temp_vertices.size();
		m_vertecies = new VERTEX[m_numVerts];
		m_numTris = temp_tris.size();

		// Setting vertecies
		for (unsigned int i = 0; i < m_numVerts; ++i)
		{
			m_vertecies[i] = temp_vertices[i];
			// Setting the shine value
			m_vertecies[i].uv.z = _shine;
		}
		//for (unsigned int i = 0; i < m_numTris; ++1)
		//{

		m_numIndicies = vertexIndices.size();
		m_indexList = new unsigned int[m_numIndicies];
		for (unsigned int i = 0; i < m_numIndicies; ++i)
			m_indexList[i] = vertexIndices[i];

		m_indexList = new unsigned int[m_numIndicies];
		for (unsigned int i = 0; i < m_numIndicies; ++i)
			m_indexListINV[i] = vertexIndiciesINV[i];
	}
	ComputeTangents();
	CreateVertexBuffer(_device);
	CreateIndexBuffer(_device);
	CreateConstBuffer(_device);
	
	XMStoreFloat4x4(&m_worldToShader.worldMatrix, XMMatrixIdentity());
	SetPosition(_position.x, _position.y, _position.z);

	// Setting up the bones
	for (unsigned int i = 0; i < m_bindBones.size(); ++i)
		XMStoreFloat4x4(&m_bonesToGPU.bones[i], m_bindBones[i].m_worldMatrix);
	m_bonesToGPU.positionOffset = m_worldToShader.worldMatrix;

	Blender::InitBlenders();
	
#if RENDER_BONES	
	for (unsigned int i = 0; i < GetBindBones().size(); ++i)
	{
		XMFLOAT3 objPos = XMFLOAT3(m_bonesToGPU.bones[i].r[3].m128_f32[0], m_bonesToGPU.bones[i].r[3].m128_f32[1], m_bonesToGPU.bones[i].r[3].m128_f32[2]);
		BaseObject* bone = new BaseObject();
		bone->InstantiateModel(_device, "..\\NDEProject\\Assets\\boneSphere.obj", objPos, 0, OBJECT);
		m_bonesObjects.push_back(bone);
	}
#endif
}

void BaseObject::InitializeInstances(XMFLOAT4X4* _matrixData, unsigned int _numInstances)
{
	if (m_numInstances != 0)
		delete m_instanceMatricis;


	m_numInstances = _numInstances;
	m_instanceMatricis = new XMFLOAT4X4[_numInstances];

	for (unsigned int i = 0; i < _numInstances; ++i)
		m_instanceMatricis[i] = _matrixData[i];
}

void BaseObject::Input()
{
}

void BaseObject::Update()
{
	if (m_active)
	{
		m_timer.Signal();
		UpdateAnimation();
	}
}

// Draws the object to the screen
void BaseObject::Render()
{
	if (m_active)
	{
		unsigned int stride = sizeof(VERTEX);
		unsigned int offset = 0;

		// Bound shaders
		Graphics::GetDeviceContext()->VSSetShader(Graphics::m_vsShaders[m_graphicsType].p, NULL, NULL);

		if (m_target)
		{
			//XMStoreFloat4x4(&m_instanceMatricis[1], XMMatrixIdentity() * XMMatrixScaling(1.1f, 1.1f, 1.1f));
			//m_instanceMatricis[1]._42 -= 0.4f;

			//InitializeInstances(mat, INSTANCES);

			Graphics::GetDeviceContext()->VSSetShader(Graphics::m_vsShaders[m_graphicsType].p, NULL, NULL);
			Graphics::GetDeviceContext()->PSSetShader(Graphics::m_psShaders[TARGET].p, NULL, NULL);
		}
		else
		{
			if (GetID() == RUNNER_ID || GetID() == MONSTER_ID)
			{
				//XMMATRIX world = XMLoadFloat4x4(&m_instanceMatricis[0]);
				//XMStoreFloat4x4(&m_instanceMatricis[0], world * XMMatrixScaling(0.1f, 0.1f, 0.1f));

				XMStoreFloat4x4(&m_instanceMatricis[0], XMMatrixIdentity());
				XMStoreFloat4x4(&m_instanceMatricis[1], XMMatrixIdentity());

				// FUN STUFF
				//XMStoreFloat4x4(&m_instanceMatricis[0], XMMatrixIdentity() * XMMatrixScaling(0.1f, 1.1f, 1.1f));
				//XMStoreFloat4x4(&m_instanceMatricis[1], XMMatrixIdentity() * XMMatrixScaling(1.1f, 1.1f, 0.1f));
			}

			Graphics::GetDeviceContext()->PSSetShader(Graphics::m_psShaders[m_graphicsType].p, NULL, NULL);
		}
		
		Graphics::GetDeviceContext()->IASetInputLayout(Graphics::m_layouts[m_graphicsType].p);
		// Set the buffers for the current object
		Graphics::GetDeviceContext()->IASetVertexBuffers(0, 1, &m_vertexBuffer.p, &stride, &offset);
		Graphics::GetDeviceContext()->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
		// HINT:: WIREFRAMES D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP
		Graphics::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Set the shader resource - for the texture
		Graphics::GetDeviceContext()->PSSetShaderResources(0, 1, &m_defShaderResourceView.p);
		// If there is normal mapping
		if (m_normalShaderResourceView.p != nullptr)
			Graphics::GetDeviceContext()->PSSetShaderResources(1, 1, &m_normalShaderResourceView.p);
		// If there is specular mapping
		if (m_specularShaderResourceView.p != nullptr)
			Graphics::GetDeviceContext()->PSSetShaderResources(2, 1, &m_specularShaderResourceView.p);
		// If there is specular mapping
		if (m_emissiveShaderResourceView.p != nullptr)
			Graphics::GetDeviceContext()->PSSetShaderResources(3, 1, &m_emissiveShaderResourceView.p);
		// Setting the object const buffer
		Graphics::GetDeviceContext()->VSSetConstantBuffers(0, 1, &m_constBuffer.p);
		Graphics::GetDeviceContext()->PSSetConstantBuffers(0, 1, &m_constBuffer.p);

		// Constant buffer mapping
		D3D11_MAPPED_SUBRESOURCE mapSubresource;
		ZeroMemory(&mapSubresource, sizeof(mapSubresource));
		Graphics::GetDeviceContext()->Map(m_constBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mapSubresource);
		memcpy(mapSubresource.pData, &m_worldToShader, sizeof(OBJECT_TO_VRAM));
		Graphics::GetDeviceContext()->Unmap(m_constBuffer, NULL);

		if (m_graphicsType == ANIMATED)
		{
			Graphics::GetDeviceContext()->VSSetConstantBuffers(2, 1, &m_bonesConstBuffer.p);
			// Bones data
			ZeroMemory(&mapSubresource, sizeof(mapSubresource));
			Graphics::GetDeviceContext()->Map(m_bonesConstBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mapSubresource);
			memcpy(mapSubresource.pData, &m_bonesToGPU, sizeof(BONES_TO_VRAM));
			Graphics::GetDeviceContext()->Unmap(m_bonesConstBuffer, NULL);
		}

		if (m_graphicsType == INSTANCED || m_graphicsType == ANIMATED)
		{
			Graphics::GetDeviceContext()->VSSetConstantBuffers(0, 1, &m_instanceBuffer.p);

			ZeroMemory(&mapSubresource, sizeof(mapSubresource));
			Graphics::GetDeviceContext()->Map(m_instanceBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mapSubresource);
			memcpy(mapSubresource.pData, &(*m_instanceMatricis), sizeof(XMFLOAT4X4)*m_numInstances);
			Graphics::GetDeviceContext()->Unmap(m_instanceBuffer, NULL);

			Graphics::GetDeviceContext()->DrawIndexedInstanced(m_numIndicies, m_numInstances, 0, 0, 0);

			//if (m_target)
			//{
			//	//XMStoreFloat4x4(&m_instanceMatricis[0], XMMatrixIdentity() * XMMatrixScaling(1.05f, 1.05f, 1.05f));
			//	//XMStoreFloat4x4(&m_instanceMatricis[1], XMMatrixIdentity() * XMMatrixScaling(1.05f, 1.05f, 1.05f));
			//	//m_instanceMatricis[1]._42 = m_instanceMatricis[0]._42 - 0.1f;
			//	Graphics::GetDeviceContext()->IASetIndexBuffer(m_indexBufferINV, DXGI_FORMAT_R32_UINT, 0);
			//	Graphics::GetDeviceContext()->DrawIndexedInstanced(m_numIndicies, m_numInstances, 0, 0, 0);
			//}
		}
		else // Draw the object
			Graphics::GetDeviceContext()->DrawIndexed(m_numIndicies, 0, 0);
	

		// Turn off shaders
		if (m_normalShaderResourceView.p != nullptr)
			Graphics::GetDeviceContext()->PSSetShaderResources(1, 1, &m_defShaderResourceView.p);
		if (m_specularShaderResourceView.p != nullptr)
			Graphics::GetDeviceContext()->PSSetShaderResources(2, 1, &m_defShaderResourceView.p);
		if (m_emissiveShaderResourceView.p != nullptr)
			Graphics::GetDeviceContext()->PSSetShaderResources(3, 1, &m_defShaderResourceView.p);
#if RENDER_BONES
		//if (m_id == RUNNER_ID)
		//	_context->RSSetState(d_rsWireframe);
		auto kb = Graphics::single_keyboard->GetState();
		if (m_bonesObjects.size() > 0)
		{
			if (d_frame >= GetCurrAnimation()->m_keyFrame.size())
				d_frame = 0;
			if (kb.Down && d_frame < GetCurrAnimation()->m_keyFrame.size() - 1)
				d_frame++;
			if (kb.Up && d_frame > 0)
				d_frame--;
		}

		for (unsigned int i = 0; i < m_bonesObjects.size(); ++i)
		{
			//m_bonesObjects[i]->SetWorldMatrix(XMMatrixMultiply(GetCurrAnimation()->m_keyFrame[d_frame]->m_bones[i].m_worldMatrix, m_worldToShader.worldMatrix));
			//m_bonesObjects[i]->SetWorldMatrix(XMMatrixMultiply(GetBindBones()[i].m_worldMatrix, m_worldToShader.worldMatrix));
			m_bonesObjects[i]->SetWorldMatrix(XMMatrixMultiply(m_bonesToGPU.bones[i], m_worldToShader.worldMatrix));
			m_bonesObjects[i]->Render();
	}
#endif
}


}

// Creates the vertex buffer for that object
void BaseObject::CreateVertexBuffer(ID3D11Device* _device)
{
	// Create the vertex buffer storing vertsPoints
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.ByteWidth = sizeof(VERTEX) * m_numVerts;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.StructureByteStride = sizeof(VERTEX);

	// Setting the resource data
	D3D11_SUBRESOURCE_DATA resourceData;
	ZeroMemory(&resourceData, sizeof(resourceData));
	resourceData.pSysMem = m_vertecies;
	_device->CreateBuffer(&bufferDesc, &resourceData, &m_vertexBuffer.p);
}

// Creates the index buffer for that object
void BaseObject::CreateIndexBuffer(ID3D11Device* _device)
{
	// Fill in a buffer description.
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.ByteWidth = sizeof(unsigned int) * m_numIndicies;
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.StructureByteStride = sizeof(unsigned int);

	// Define the resource data.
	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(D3D11_SUBRESOURCE_DATA));
	initData.pSysMem = m_indexList;

	// Create the buffer with the device.
	_device->CreateBuffer(&bufferDesc, &initData, &m_indexBuffer.p);

	if (GetID() == RUNNER_ID)
	{
		// Fill in a buffer description.
		D3D11_BUFFER_DESC bufferDesc;
		ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bufferDesc.ByteWidth = sizeof(unsigned int) * m_numIndicies;
		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bufferDesc.StructureByteStride = sizeof(unsigned int);

		// Define the resource data.
		D3D11_SUBRESOURCE_DATA initData;
		ZeroMemory(&initData, sizeof(D3D11_SUBRESOURCE_DATA));
		initData.pSysMem = m_indexListINV;

		// Create the buffer with the device.
		_device->CreateBuffer(&bufferDesc, &initData, &m_indexBufferINV.p);
	}
}

// Textures the object 
void BaseObject::TextureObject(ID3D11Device* _device, const wchar_t*  _filePathToDefuse, const wchar_t*  _filePathToNormalMap, const wchar_t*  _filePathToSpecular, const wchar_t*  _filePathToEmissive)
{

	CreateDDSTextureFromFile(_device, _filePathToDefuse, NULL, &m_defShaderResourceView.p);
	if (_filePathToNormalMap)
		CreateDDSTextureFromFile(_device, _filePathToNormalMap, NULL, &m_normalShaderResourceView.p);
	if (_filePathToSpecular)
		CreateDDSTextureFromFile(_device, _filePathToSpecular, NULL, &m_specularShaderResourceView.p);
	if (_filePathToEmissive)
		CreateDDSTextureFromFile(_device, _filePathToSpecular, NULL, &m_emissiveShaderResourceView.p);
}

// Sets the world matrix of the object
void BaseObject::SetWorldMatrix(XMMATRIX& _matrix)
{
	UpdateHitBox(&_matrix);
	XMStoreFloat4x4(&m_bonesToGPU.positionOffset, _matrix);
	XMStoreFloat4x4(&m_worldToShader.worldMatrix, _matrix);
}

// Moves the object according to the passed in values
void BaseObject::SetPosition(float _x, float _y, float _z)
{
	m_bonesToGPU.positionOffset._41 = _x;
	m_bonesToGPU.positionOffset._42 = _y;
	m_bonesToGPU.positionOffset._43 = _z;

	m_worldToShader.worldMatrix._41 = _x;
	m_worldToShader.worldMatrix._42 = _y;
	m_worldToShader.worldMatrix._43 = _z;

	UpdateHitBox(&XMLoadFloat4x4(&m_worldToShader.worldMatrix));
}

/*
Used for normal mapping.
Followed http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-13-normal-mapping/ tutorial
Parameters:
[in] _verts       - vertecies of object
[in] _uvs         - uv coordinates of object
[in] _normals     - normals of object
*/
void BaseObject::ComputeTangents()
{
	vector<XMFLOAT4> temp_tangents;
	vector<XMFLOAT4> temp_bitangents;

	for (unsigned int i = 0; i < m_numVerts; i += 3)
	{
		// Getting the triangles 
		XMFLOAT4 tempV0 = m_vertecies[i].transform;
		XMFLOAT4 tempV1 = m_vertecies[i + 1].transform;
		XMFLOAT4 tempV2 = m_vertecies[i + 2].transform;

		XMFLOAT4 tempUV0 = m_vertecies[i].uv;
		XMFLOAT4 tempUV1 = m_vertecies[i + 1].uv;
		XMFLOAT4 tempUV2 = m_vertecies[i + 2].uv;

		// Edges of the triangle 
		XMFLOAT4 deltaPos1{ tempV1.x - tempV0.x, tempV1.y - tempV0.y, tempV1.z - tempV0.z,0 };
		XMFLOAT4 deltaPos2{ tempV2.x - tempV0.x, tempV2.y - tempV0.y, tempV2.z - tempV0.z,0 };

		XMFLOAT4 deltaUV1 = { tempUV1.x - tempUV0.x, tempUV1.y - tempUV0.y, 0,0 };
		XMFLOAT4 deltaUV2 = { tempUV2.x - tempUV0.x, tempUV2.y - tempUV0.y, 0 ,0 };

		float ratio = (1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x));

		// Calculating tangent
		XMFLOAT4 tempTangent1{ deltaPos1.x * deltaUV2.y, deltaPos1.y * deltaUV2.y, deltaPos1.z * deltaUV2.y,0 };
		XMFLOAT4 tempTangent2{ deltaPos2.x * deltaUV1.y, deltaPos2.y * deltaUV1.y, deltaPos2.z * deltaUV1.y,0 };
		XMFLOAT4 tempTangent3{ tempTangent1.x - tempTangent2.x, tempTangent1.y - tempTangent2.y, tempTangent1.z - tempTangent2.z,0 };
		XMFLOAT4 tangent{ tempTangent3.x*ratio, tempTangent3.y*ratio, tempTangent3.z*ratio,0 };

		// Calculating bitangent
		XMFLOAT4 tempBitangent{ tempTangent2.x - tempTangent1.x, tempTangent2.y - tempTangent1.y, tempTangent2.z - tempTangent1.z,0 };
		XMFLOAT4 bitangent{ tempBitangent.x*ratio, tempBitangent.y*ratio, tempBitangent.z*ratio,0 };

		// Setting them 
		m_vertecies[i].tangents = tangent;
		m_vertecies[i + 1].tangents = tangent;
		m_vertecies[i + 2].tangents = tangent;

		m_vertecies[i].bitangents = bitangent;
		m_vertecies[i + 1].bitangents = bitangent;
		m_vertecies[i + 2].bitangents = bitangent;
	}
}

// Creates constant buffer
void BaseObject::CreateConstBuffer(ID3D11Device* _device)
{
	// Creating the constant buffer for the world
	D3D11_BUFFER_DESC constBufferDesc;
	ZeroMemory(&constBufferDesc, sizeof(D3D11_BUFFER_DESC));
	constBufferDesc.ByteWidth = sizeof(OBJECT_TO_VRAM);
	constBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	constBufferDesc.StructureByteStride = sizeof(float);
	constBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	_device->CreateBuffer(&constBufferDesc, NULL, &m_constBuffer.p);

	if (m_graphicsType == ANIMATED)
	{
		// Creating constant buffer for the bones
		ZeroMemory(&constBufferDesc, sizeof(D3D11_BUFFER_DESC));
		constBufferDesc.ByteWidth = sizeof(BONES_TO_VRAM);
		constBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		constBufferDesc.StructureByteStride = sizeof(float);
		constBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		_device->CreateBuffer(&constBufferDesc, NULL, &m_bonesConstBuffer.p);

		// Creating constant buffer for instansing
		ZeroMemory(&constBufferDesc, sizeof(D3D11_BUFFER_DESC));
		constBufferDesc.ByteWidth = sizeof(XMFLOAT4X4) *2;
		constBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		constBufferDesc.StructureByteStride = sizeof(XMFLOAT4X4);
		constBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		D3D11_SUBRESOURCE_DATA initData;
		ZeroMemory(&initData, sizeof(D3D11_SUBRESOURCE_DATA));
		initData.pSysMem = m_instanceMatricis;

		_device->CreateBuffer(&constBufferDesc, &initData, &m_instanceBuffer.p);
	}

	if (m_graphicsType == INSTANCED)
	{
		// Creating constant buffer for instansing
		ZeroMemory(&constBufferDesc, sizeof(D3D11_BUFFER_DESC));
		constBufferDesc.ByteWidth = sizeof(XMFLOAT4X4) *m_numInstances;
		constBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		constBufferDesc.StructureByteStride = sizeof(XMFLOAT4X4);
		constBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		D3D11_SUBRESOURCE_DATA initData;
		ZeroMemory(&initData, sizeof(D3D11_SUBRESOURCE_DATA));
		initData.pSysMem = m_instanceMatricis;

		_device->CreateBuffer(&constBufferDesc, &initData, &m_instanceBuffer.p);
	}
}

// Load binary file verticies
void BaseObject::LoadBinaryFile(std::string _filePath, vector<VERTEX>& _vertecies, vector<unsigned int>& _indices)
{
	FILE* file = nullptr;
	FileInfo::ExporterHeader header;
	fstream binFile;

	string binName;
	const char* theName = strrchr(_filePath.c_str(), '\\');
	binName = strrchr(theName, theName[1]);
	binName.pop_back();
	binName.pop_back();
	binName.pop_back();
	binName.pop_back();
	binName.append("_mesh.bin");

	if (header.ReadHeader(&file, binName.c_str(), _filePath.c_str()))
	{
		binFile.open(binName.c_str(), std::ios::in | std::ios::binary);
		if (binFile.is_open())
		{
			binFile.seekp(sizeof(FileInfo::ExporterHeader), ios::beg);
			_vertecies.resize(header.mesh.numPoints);
			binFile.read((char*)&_vertecies[0], (header.mesh.numPoints * header.mesh.vertSize));
			_indices.resize(header.mesh.numIndex);
			binFile.read((char*)&_indices[0], (header.mesh.numIndex * sizeof(unsigned int)));
		}
		binFile.close();
	}
}

// Load binary file bones
void BaseObject::LoadBinaryFile(std::string _filePath, vector<Transform>& _bones)
{
	FILE* file = nullptr;
	FileInfo::ExporterHeader header;
	fstream binFile;
	string binName;

	const char* theName = strrchr(_filePath.c_str(), '\\');
	binName = strrchr(theName, theName[1]);
	binName.pop_back();
	binName.pop_back();
	binName.pop_back();
	binName.pop_back();
	binName.append("_bindpose.bin");

	if (header.ReadHeader(&file, binName.c_str(), _filePath.c_str()))
	{
		binFile.open(binName.c_str(), std::ios::in | std::ios::binary);
		if (binFile.is_open())
		{
			binFile.seekp(sizeof(FileInfo::ExporterHeader), ios::beg);
			_bones.resize(header.bind.numBones);
			binFile.read((char*)&_bones[0], header.bind.numBones * sizeof(Transform));
		}
		binFile.close();
	}
}

// Load binary file animation
void BaseObject::LoadBinaryFile(std::string _filePath, Animation& _animation)
{
	FILE* file = nullptr;
	FileInfo::ExporterHeader header;
	fstream binFile;
	string binName;

	const char* theName = strrchr(_filePath.c_str(), '\\');
	binName = strrchr(theName, theName[1]);
	binName.pop_back();
	binName.pop_back();
	binName.pop_back();
	binName.pop_back();
	binName.append("_anim.bin");

	if (header.ReadHeader(&file, binName.c_str(), _filePath.c_str()))
	{
		binFile.open(binName.c_str(), std::ios::in | std::ios::binary);
		if (binFile.is_open())
		{
			binFile.seekp(sizeof(FileInfo::ExporterHeader), ios::beg);
			binFile.read((char*)&_animation, sizeof(Animation) - sizeof(vector<KeyFrame>));
			_animation.m_keyFrame.resize(header.anim.numFrames);
			for (size_t i = 0; i < header.anim.numFrames; i++)
			{
				binFile.read((char*)&_animation.m_keyFrame[i], sizeof(KeyFrame) - sizeof(vector<Transform>));
				_animation.m_keyFrame[i]->m_bones.resize(header.anim.numBones);
				for (size_t j = 0; j < header.anim.numBones; j++)
				{
					binFile.read((char*)&_animation.m_keyFrame[i]->m_bones[j], sizeof(Transform));
				}
			}
		}
		binFile.close();
	}
}

// Load Binary File (OBJ)
// Note to Compute Tangents after receiving data
void BaseObject::LoadBinaryFile(std::string _filePath)
{
	fstream binFile;
	string binName;

	string theName = strrchr(_filePath.c_str(), '\\');
	//binName = strrchr(theName.c_str(), theName[1]);
	theName.erase(0, 1);
	binName = theName;
	binName.pop_back();
	binName.pop_back();
	binName.pop_back();
	binName.pop_back();
	binName.append("_obj.bin");

	binFile.open(binName.c_str(), std::ios::in | std::ios::binary);
	if (binFile.is_open())
	{
		binFile.read((char*)&m_numVerts, sizeof(unsigned int));
		m_vertecies = new VERTEX[m_numVerts];
		binFile.read((char*)&m_vertecies[0], sizeof(VERTEX) * m_numVerts);
		binFile.read((char*)&m_numIndicies, sizeof(unsigned int));
		m_indexList = new unsigned int[m_numIndicies];
		binFile.read((char*)&m_indexList[0], sizeof(unsigned int) * m_numIndicies);
	}
	binFile.close();

	ComputeTangents();
}

void BaseObject::SetHitBox(XMFLOAT3 newStart, XMFLOAT3 newEnd, float rad)
{
	Capsule capper;
	capper.m_Segment.m_Start = newStart;
	capper.m_Segment.m_End = newEnd;
	capper.m_Radius = rad;
	dynamic_cast<Capsule*>(m_hBox)->SetHitbox(&capper);
}

void BaseObject::SetHitBox(AABB& _aabb, XMMATRIX* _newPosition)
{
	_aabb.m_max.x = -FLT_MAX;
	_aabb.m_max.y = -FLT_MAX;
	_aabb.m_max.z = -FLT_MAX;

	_aabb.m_min.x = FLT_MAX;
	_aabb.m_min.y = FLT_MAX;
	_aabb.m_min.z = FLT_MAX;

	for (int i = 0; i < GetVertLength(); ++i)
	{
		XMVECTOR vertPos = XMLoadFloat4(&GetVerts()[i].transform);
		vertPos = XMVector3Transform(vertPos, (*_newPosition));
		XMFLOAT3 vPos;
		XMStoreFloat3(&vPos, vertPos);

		if (vPos.x < _aabb.m_min.x)
			_aabb.m_min.x = vPos.x;
		if (vPos.x > _aabb.m_max.x)
			_aabb.m_max.x = vPos.x;

		if (vPos.y < _aabb.m_min.y)
			_aabb.m_min.y = vPos.y;
		if (vPos.y > _aabb.m_max.y)
			_aabb.m_max.y = vPos.y;

		if (vPos.z < _aabb.m_min.z)
			_aabb.m_min.z = vPos.z;
		if (vPos.z > _aabb.m_max.z)
			_aabb.m_max.z = vPos.z;
	}
}

void BaseObject::UpdateHitBox(XMMATRIX* new_pos)
{
	HitBox* colliderBox = GetHitBox();
	if (!colliderBox)
		return;

	if (colliderBox->type == CAPSULE_COL)
	{
		Capsule * colliderCapsule = dynamic_cast<Capsule*>(colliderBox);

		Player* player;
		XMFLOAT3 newStart;
		XMFLOAT3 newEnd;

		if (m_id == MONSTER_ID || m_id == RUNNER_ID)
		{
			float dif = colliderCapsule->m_Segment.m_End.y - colliderCapsule->m_Segment.m_Start.y;
			float newStartY = new_pos->r[3].m128_f32[1] + colliderCapsule->m_Radius + .05f;
			float newEndY = new_pos->r[3].m128_f32[1] + colliderCapsule->m_Radius + dif + .05f;

			newStart = { new_pos->r[3].m128_f32[0], newStartY, new_pos->r[3].m128_f32[2] };
			//XMFLOAT3 newStart = this->GetBone()
			newEnd = { new_pos->r[3].m128_f32[0], newEndY, new_pos->r[3].m128_f32[2] };


			player = dynamic_cast<Player*>(this);
		
			XMFLOAT3 newStart2 = { (player->GetLeftFoot()._41 + player->GetRightFoot()._41) / 2,
				((player->GetLeftFoot()._42 + player->GetRightFoot()._42) / 2),
				(player->GetLeftFoot()._43 + player->GetRightFoot()._43) / 2 };

			if (player->GetHead()._11 != -10000)
			{
				XMMATRIX boneSpace = XMMatrixMultiply(XMLoadFloat4x4(&player->GetHead()), *new_pos);
				XMFLOAT3 newEnd2 = { newStart.x, boneSpace.r[3].m128_f32[1] - 1.0f, newStart.z };
				newEnd = newEnd2;
			}
		}
		else if (m_id == ITEM_ID)
		{
			newStart = { new_pos->r[3].m128_f32[0], new_pos->r[3].m128_f32[1], new_pos->r[3].m128_f32[2] };
			newEnd = { new_pos->r[3].m128_f32[0], new_pos->r[3].m128_f32[1], new_pos->r[3].m128_f32[2] };
		}
		else
		{
			float dif = colliderCapsule->m_Segment.m_End.y - colliderCapsule->m_Segment.m_Start.y;
			float newStartY = new_pos->r[3].m128_f32[1] + colliderCapsule->m_Radius + .05f;
			float newEndY = new_pos->r[3].m128_f32[1] + colliderCapsule->m_Radius + dif + .05f;

			newStart = { new_pos->r[3].m128_f32[0], newStartY, new_pos->r[3].m128_f32[2] };
			newEnd = { new_pos->r[3].m128_f32[0], newEndY, new_pos->r[3].m128_f32[2] };
		}

		SetHitBox(newStart, newEnd, colliderCapsule->m_Radius);
	}
	else if (colliderBox->type == AABB_COL)
	{
		AABB* colliderAABB = dynamic_cast<AABB*>(colliderBox);
		SetHitBox((*colliderAABB), new_pos);
	}	

	m_hBox->m_active = m_active;
}

void BaseObject::AttachHitBox(int type)
{
	if (type == AABB_COL)
	{
		m_hBox = new AABB();
		m_hBox->type = AABB_COL;

		dynamic_cast<AABB*>(m_hBox)->m_max.x = -FLT_MAX;
		dynamic_cast<AABB*>(m_hBox)->m_max.y = -FLT_MAX;
		dynamic_cast<AABB*>(m_hBox)->m_max.z = -FLT_MAX;

		dynamic_cast<AABB*>(m_hBox)->m_min.x = FLT_MAX;
		dynamic_cast<AABB*>(m_hBox)->m_min.y = FLT_MAX;
		dynamic_cast<AABB*>(m_hBox)->m_min.z = FLT_MAX;

		SetHitBox((*dynamic_cast<AABB*>(m_hBox)), &GetWorldMatrix());
	}
}

void BaseObject::AttachHitBox(float mult, float heightEnd, float heightStart, int type)
{
	if (type == CAPSULE_COL)
	{
		if (m_bindBones.size() > 0)
		{
			XMFLOAT4X4 lFut, rFut, hed, bone;
			XMStoreFloat4x4(&lFut, m_bindBones[0].m_worldMatrix);
			XMStoreFloat4x4(&rFut, m_bindBones[0].m_worldMatrix);
			XMStoreFloat4x4(&hed, m_bindBones[0].m_worldMatrix);

			unsigned int lfoot = 0, rfoot = 0, head = 0;

			for (unsigned int i = 1; i < m_bindBones.size(); i++)
			{
				XMStoreFloat4x4(&bone, m_bindBones[i].m_worldMatrix);

				if (bone._42 > hed._42)
				{
					XMStoreFloat4x4(&hed, m_bindBones[i].m_worldMatrix);
					head = i;
				}
				else if (bone._42 < lFut._42)
				{
					XMStoreFloat4x4(&lFut, m_bindBones[i].m_worldMatrix);
					lfoot= i;
				}
				else if (bone._42 < rFut._42)
				{
					XMStoreFloat4x4(&rFut, m_bindBones[i].m_worldMatrix);
					rfoot = i;
				}
			}
			dynamic_cast<Player*>(this)->SetLeftFootIndex(lfoot);
			dynamic_cast<Player*>(this)->SetRightFootIndex(rfoot);
			dynamic_cast<Player*>(this)->SetHeadIndex(head);
		}

		XMFLOAT3 start, end;
		XMStoreFloat3(&start,GetWorldMatrix().r[3]);
		XMStoreFloat3(&end, GetWorldMatrix().r[3]);
		
		//resize the height
		start.y += heightStart;
		end.y += heightEnd;
		
		m_hBox = new Capsule();
		m_hBox->type = CAPSULE_COL;
		SetHitBox(start, end, .12f * mult);
	}
	else if (type == AABB_COL)
	{
		m_hBox = new AABB();
		m_hBox->type = AABB_COL;

		dynamic_cast<AABB*>(m_hBox)->m_max.x = -FLT_MAX;
		dynamic_cast<AABB*>(m_hBox)->m_max.y = -FLT_MAX;
		dynamic_cast<AABB*>(m_hBox)->m_max.z = -FLT_MAX;

		dynamic_cast<AABB*>(m_hBox)->m_min.x = FLT_MAX;
		dynamic_cast<AABB*>(m_hBox)->m_min.y = FLT_MAX;
		dynamic_cast<AABB*>(m_hBox)->m_min.z = FLT_MAX;

		SetHitBox((*dynamic_cast<AABB*>(m_hBox)), &GetWorldMatrix());
		return;
	}
}

void BaseObject::UpdateAnimation()
{
	if (m_graphicsType != ANIMATED)
		return;
	
	// Creating a blender if a new next
	if (m_lastNextAnimation != m_nextAnimation)
	{
		Blender* b;
	
		b = Blender::GetFreeBlender();
	
		if (!m_currBlender)
			b->SetBlender(&m_currAnimation, &m_nextAnimation, &m_animations[FALLING_ANIM]);
		else
			b->SetBlender(&m_currBlender, &m_nextAnimation, &m_animations[FALLING_ANIM]);
		m_currBlender = b;
	}
	
	// Interpolating if we don't have a blender
	
	bool end = false;
	m_interpedFrame = *m_currAnimation->m_keyFrame[m_currAnimation->m_keyFrame.size() - 1];

	if (!m_currBlender)
	{
		m_currAnimation->Process(float(m_timer.Delta()), m_interpedFrame, end);
		Animation::Update(m_bonesToGPU.bones, m_bindBones, m_bonesObjects, m_interpedFrame);
		
		if (end)
			m_animBusy = false;
		return;
	}
	
	m_lastNextAnimation = m_nextAnimation;
	
	// Blending
	m_interpedFrame = m_currBlender->Blend(m_interpedFrame,float(m_timer.Delta()), 1.0f, GetTimerDelta(), end);

	Animation::Update(m_bonesToGPU.bones, m_bindBones, m_bonesObjects,m_interpedFrame);

	if (end)
		m_animBusy = false;
	
	// Clean up
	if (m_currBlender->GetBlendDur() <= 0)
	{
		m_currAnimation = m_nextAnimation;
		m_currBlender->Refresh();
		m_currBlender = nullptr;
	}
}

void BaseObject::LoadAnimations(std::string _filePath, float _shine)
{
	// Local variables
	vector<unsigned int> vertexIndices, vertexIndicesINV, uvIndices, normalIndices;
	vector<VERTEX> temp_vertices;
	vector<vector<VERTEX>> temp_tris;

	// Find number of files in specified folder
	int counter = 0;
	WIN32_FIND_DATA ffd;
	HANDLE hFind = INVALID_HANDLE_VALUE;

	string dir;
	if (int(_filePath.find("Monster")) > -1)
		dir = "Monster";
	else if (int(_filePath.find("Runner")) > -1)
		dir = "Runner";

	std::vector<wstring> strs;
	// Start iterating over the files in the path directory.
	hFind = ::FindFirstFileA(_filePath.c_str(), (LPWIN32_FIND_DATAA)&ffd);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do // Managed to locate and create an handle to that folder.
		{
			wstring temp = (wstring)ffd.cFileName;
			if (temp[temp.size() - 2] == 'b' && temp[temp.size() - 1] == 'x')
			{
				counter++;
				strs.push_back((wstring)ffd.cFileName);
			}
		} while (::FindNextFile(hFind, &ffd) == TRUE);
		::FindClose(hFind);
	}
	else {
		printf("Failed to find path: %s", _filePath.c_str());
	}

	_filePath.pop_back();
	m_animations.resize(counter);
	for (int i = 0; i < counter; i++)
	{
		size_t j = 0;

		Animation* newAnim = new Animation();
		string newStr;
		for (; j < strs[i].size(); j++)
			newStr.push_back((char)strs[i][j]);

		_filePath.append(newStr);

		FBXImporter::LoadFBXFile(_filePath, temp_vertices, temp_tris, vertexIndices, vertexIndicesINV, m_bindBones, *newAnim);

		newAnim->SetDeath(false);

		if (dir == "Runner")
		{
			if (newStr == "Idle.fbx")
			{
				m_animations[IDLE_ANIM] = newAnim;
				newAnim->SetName(IDLE_ANIM);
				newAnim->SetLooping(true);
				newAnim->SetInToBlendTime(.25f);
				newAnim->SetOutOfBlendFrame(2);
			}
			else if (newStr == "Walk.fbx")
			{
				m_animations[WALK_ANIM] = newAnim;
				newAnim->SetName(WALK_ANIM);
				newAnim->SetLooping(true);
				newAnim->SetInToBlendTime(.25f);
				newAnim->SetOutOfBlendFrame(2);
			}
			else if (newStr == "WalkBackwards.fbx")
			{
				m_animations[WALK_BACKWARD_ANIM] = newAnim;
				newAnim->SetName(WALK_BACKWARD_ANIM);
				newAnim->SetLooping(true);
				newAnim->SetInToBlendTime(.5f);
				newAnim->SetOutOfBlendFrame(2);
			}
			else if (newStr == "Run.fbx")
			{
				m_animations[RUN_ANIM] = newAnim;
				newAnim->SetName(RUN_ANIM);
				newAnim->SetLooping(true);
				newAnim->SetInToBlendTime(.25f);
				newAnim->SetOutOfBlendFrame(2);
			}
			else if (newStr == "Jump.fbx")
			{
				m_animations[JUMP_ANIM] = newAnim;
				newAnim->SetName(JUMP_ANIM);
				newAnim->SetLooping(false);
				newAnim->SetInToBlendTime(.2f);
				newAnim->SetOutOfBlendFrame(7);
			}
			else if (newStr == "Falling.fbx")
			{
				m_animations[FALLING_ANIM] = newAnim;
				newAnim->SetName(FALLING_ANIM);
				newAnim->SetLooping(true);
				newAnim->SetInToBlendTime(.25f);
				newAnim->SetOutOfBlendFrame(2);
			}
			else if (newStr == "Landing.fbx")
			{
				m_animations[LANDING_ANIM] = newAnim;
				newAnim->SetName(LANDING_ANIM);
				newAnim->SetLooping(false);
				newAnim->SetInToBlendTime(.25f);
				newAnim->SetOutOfBlendFrame(10);
			}
			else if (newStr == "StrafeLeft.fbx")
			{
				m_animations[STRAFE_LEFT_ANIM] = newAnim;
				newAnim->SetName(STRAFE_LEFT_ANIM);
				newAnim->SetLooping(true);
				newAnim->SetInToBlendTime(.25f);
				newAnim->SetOutOfBlendFrame(12);
			}
			else if (newStr == "StrafeRight.fbx")
			{
				m_animations[STRAFE_RIGHT_ANIM] = newAnim;
				newAnim->SetName(STRAFE_RIGHT_ANIM);
				newAnim->SetLooping(true);
				newAnim->SetInToBlendTime(.25f);
				newAnim->SetOutOfBlendFrame(12);
			}
			else if (newStr == "Death.fbx")
			{
				m_animations[DEATH_ANIM] = newAnim;
				newAnim->SetName(DEATH_ANIM);
				newAnim->SetLooping(false);
				newAnim->SetInToBlendTime(.25f);
				newAnim->SetOutOfBlendFrame(2);
				newAnim->SetDeath(true);
			}
			else if (newStr == "Push.fbx")
			{
				m_animations[PUSH_ANIM] = newAnim;
				newAnim->SetName(PUSH_ANIM);
				newAnim->SetLooping(false);
				newAnim->SetInToBlendTime(.15f);
				newAnim->SetOutOfBlendFrame(8);
			}
			else if (newStr == "Pushed.fbx")
			{
				m_animations[PUSHED_ANIM] = newAnim;
				newAnim->SetName(PUSHED_ANIM);
				newAnim->SetLooping(false);
				newAnim->SetInToBlendTime(.25f);
				newAnim->SetOutOfBlendFrame(2);
			}
			else if (newStr == "Kick.fbx")
			{
				m_animations[KICK_ANIM] = newAnim;
				newAnim->SetName(KICK_ANIM);
				newAnim->SetLooping(false);
				newAnim->SetInToBlendTime(.25f);
				newAnim->SetOutOfBlendFrame(6);
			}
			else if (newStr == "GettingUp.fbx")
			{
				m_animations[GETTING_UP_ANIM] = newAnim;
				newAnim->SetName(GETTING_UP_ANIM);
				newAnim->SetLooping(false);
				newAnim->SetInToBlendTime(.5f);
				newAnim->SetOutOfBlendFrame(10);
			}
			else if (newStr == "Tripping.fbx")
			{
				m_animations[TRIPPING_ANIM] = newAnim;
				newAnim->SetName(TRIPPING_ANIM);
				newAnim->SetLooping(false);
				newAnim->SetInToBlendTime(.25f);
				newAnim->SetOutOfBlendFrame(10);
			}
		}
		else if (dir == "Monster")
		{
			if (newStr == "Idle.fbx")
			{
				m_animations[IDLE_ANIM] = newAnim;
				newAnim->SetName(IDLE_ANIM);
				newAnim->SetLooping(true);
				newAnim->SetInToBlendTime(.2f);
				newAnim->SetOutOfBlendFrame(2);
			}
			else if (newStr == "Walk.fbx")
			{
				m_animations[WALK_ANIM] = newAnim;
				newAnim->SetName(WALK_ANIM);
				newAnim->SetLooping(true);
				newAnim->SetInToBlendTime(.25f);
				newAnim->SetOutOfBlendFrame(2);
			}
			else if (newStr == "WalkBackwards.fbx")
			{
				m_animations[WALK_BACKWARD_ANIM] = newAnim;
				newAnim->SetName(WALK_BACKWARD_ANIM);
				newAnim->SetLooping(true);
				newAnim->SetInToBlendTime(.5f);
				newAnim->SetOutOfBlendFrame(2);
			}
			else if (newStr == "Run.fbx")
			{
				m_animations[RUN_ANIM] = newAnim;
				newAnim->SetName(RUN_ANIM);
				newAnim->SetLooping(true);
				newAnim->SetInToBlendTime(.25f);
				newAnim->SetOutOfBlendFrame(2);
			}
			else if (newStr == "Jump.fbx")
			{
				m_animations[JUMP_ANIM] = newAnim;
				newAnim->SetName(JUMP_ANIM);
				newAnim->SetLooping(false);
				newAnim->SetInToBlendTime(.2f);
				newAnim->SetOutOfBlendFrame(7);
			}
			else if (newStr == "Falling.fbx")
			{
				m_animations[FALLING_ANIM] = newAnim;
				newAnim->SetName(FALLING_ANIM);
				newAnim->SetLooping(true);
				newAnim->SetInToBlendTime(.25f);
				newAnim->SetOutOfBlendFrame(2);
			}
			else if (newStr == "Landing.fbx")
			{
				m_animations[LANDING_ANIM] = newAnim;
				newAnim->SetName(LANDING_ANIM);
				newAnim->SetLooping(false);
				newAnim->SetInToBlendTime(.25f);
				newAnim->SetOutOfBlendFrame(30);
			}
			else if (newStr == "StrafeLeft.fbx")
			{
				m_animations[STRAFE_LEFT_ANIM] = newAnim;
				newAnim->SetName(STRAFE_LEFT_ANIM);
				newAnim->SetLooping(true);
				newAnim->SetInToBlendTime(.25f);
				newAnim->SetOutOfBlendFrame(12);
			}
			else if (newStr == "StrafeRight.fbx")
			{
				m_animations[STRAFE_RIGHT_ANIM] = newAnim;
				newAnim->SetName(STRAFE_RIGHT_ANIM);
				newAnim->SetLooping(true);
				newAnim->SetInToBlendTime(.25f);
				newAnim->SetOutOfBlendFrame(12);
			}
			else if (newStr == "PounceCharge.fbx")
			{
				m_animations[POUNCE_CHARGE_ANIM] = newAnim;
				newAnim->SetName(POUNCE_CHARGE_ANIM);
				newAnim->SetLooping(true);
				newAnim->SetInToBlendTime(.25f);
				newAnim->SetOutOfBlendFrame(2);
			}
			else if (newStr == "PounceStart.fbx")
			{
				m_animations[POUNCE_START_ANIM] = newAnim;
				newAnim->SetName(POUNCE_START_ANIM);
				newAnim->SetLooping(false);
				newAnim->SetInToBlendTime(.25f);
				newAnim->SetOutOfBlendFrame(2);
			}
			else if (newStr == "PounceIdle.fbx")
			{
				m_animations[POUNCE_IDLE_ANIM] = newAnim;
				newAnim->SetName(POUNCE_IDLE_ANIM);
				newAnim->SetLooping(true);
				newAnim->SetInToBlendTime(.25f);
				newAnim->SetOutOfBlendFrame(2);
			}
			else if (newStr == "PounceLanding.fbx")
			{
				m_animations[POUNCE_LANDING_ANIM] = newAnim;
				newAnim->SetName(POUNCE_LANDING_ANIM);
				newAnim->SetLooping(false);
				newAnim->SetInToBlendTime(.25f);
				newAnim->SetOutOfBlendFrame(12);
			}
			else if (newStr == "CrawlForward.fbx")
			{
				m_animations[CRAWL_FORWARD_ANIM] = newAnim;
				newAnim->SetName(CRAWL_FORWARD_ANIM);
				newAnim->SetLooping(true);
				newAnim->SetInToBlendTime(.25f);
				newAnim->SetOutOfBlendFrame(2);
			}
			else if (newStr == "CrawlBackward.fbx")
			{
				m_animations[CRAWL_BACKWARD_ANIM] = newAnim;
				newAnim->SetName(CRAWL_BACKWARD_ANIM);
				newAnim->SetLooping(true);
				newAnim->SetInToBlendTime(.25f);
				newAnim->SetOutOfBlendFrame(2);
			}
			else if (newStr == "CrawlLeft.fbx")
			{
				m_animations[CRAWL_LEFT_ANIM] = newAnim;
				newAnim->SetName(CRAWL_LEFT_ANIM);
				newAnim->SetLooping(true);
				newAnim->SetInToBlendTime(.25f);
				newAnim->SetOutOfBlendFrame(2);
			}
			else if (newStr == "CrawlRight.fbx")
			{
				m_animations[CRAWL_RIGHT_ANIM] = newAnim;
				newAnim->SetName(CRAWL_RIGHT_ANIM);
				newAnim->SetLooping(true);
				newAnim->SetInToBlendTime(.25f);
				newAnim->SetOutOfBlendFrame(2);
			}
			else if (newStr == "Swipe.fbx")
			{
				m_animations[SWIPE_ANIM] = newAnim;
				newAnim->SetName(SWIPE_ANIM);
				newAnim->SetLooping(false);
				newAnim->SetInToBlendTime(.25f);
				newAnim->SetOutOfBlendFrame(10);
			}
		}

		//m_animations[i - 2] = newAnim;
		for (size_t k = 0; k < j; k++)
			_filePath.pop_back();
	}

	m_currAnimation = m_nextAnimation = m_lastNextAnimation = GetAnimations()[IDLE_ANIM];

	// Setting the members
	m_numVerts = temp_vertices.size();
	m_vertecies = new VERTEX[m_numVerts];

	// Setting vertecies
	for (unsigned int i = 0; i < m_numVerts; ++i)
	{
		m_vertecies[i] = temp_vertices[i];
		// Setting the shine value
		m_vertecies[i].uv.z = _shine;
	}
	m_numIndicies = vertexIndices.size();
	m_indexList = new unsigned int[m_numIndicies];
	for (unsigned int i = 0; i < m_numIndicies; ++i)
		m_indexList[i] = vertexIndices[i];

	m_indexListINV = new unsigned int[m_numIndicies];
	for (unsigned int i = 0; i < m_numIndicies; ++i)
		m_indexListINV[i] = vertexIndicesINV[i];
}

void BaseObject::TriInit()
{
	unsigned int realNumVerts = m_numVerts;
	m_tris.resize(realNumVerts / 3); // cause we're getting triple the amount of tris for some reason
	m_quad = new Quad;
	m_quad->m_tris.resize(m_tris.size());

	int j = 0;
	for (size_t i = 0; i < realNumVerts; i+=3)
	{
		m_tris[j].d_name = std::to_string(j);

		m_tris[j].m_one = m_vertecies[i];
		m_tris[j].m_two = m_vertecies[i+1];
		m_tris[j].m_three = m_vertecies[i+2];

		m_tris[j].m_normal.x = (m_tris[j].m_one.normals.x + m_tris[j].m_two.normals.x + m_tris[j].m_three.normals.x) / 3;
		m_tris[j].m_normal.y = (m_tris[j].m_one.normals.y + m_tris[j].m_two.normals.y + m_tris[j].m_three.normals.y) / 3;
		m_tris[j].m_normal.z = (m_tris[j].m_one.normals.z + m_tris[j].m_two.normals.z + m_tris[j].m_three.normals.z) / 3;
		
		m_tris[j].m_center.x = (m_tris[j].m_one.transform.x + m_tris[j].m_two.transform.x + m_tris[j].m_three.transform.x) / 3;
		m_tris[j].m_center.y = (m_tris[j].m_one.transform.y + m_tris[j].m_two.transform.y + m_tris[j].m_three.transform.y) / 3;
		m_tris[j].m_center.z = (m_tris[j].m_one.transform.z + m_tris[j].m_two.transform.z + m_tris[j].m_three.transform.z) / 3;
		
		m_quad->m_tris[j] = &m_tris[j];
		j++;
	}

	for (size_t i = 0; i < m_tris.size(); i++)
	{
		for (size_t j = i + 1; j < m_tris.size() - i; j++)
		{
			if (m_tris[i].m_center.x == m_tris[j].m_center.x && m_tris[i].m_center.y == m_tris[j].m_center.y &&
				m_tris[i].m_center.z == m_tris[j].m_center.z)
				int e = 3;
		}
	}

	AABB base;
	SetHitBox(base, &XMLoadFloat4x4(&m_worldToShader.worldMatrix));

	std::vector<Quad*> temp;
	temp.push_back(m_quad);
	temp[0]->m_name = "BASE";
	temp[0]->m_ALL = base;
	temp[0]->d_base = temp[0];

	while (temp.size() != 0)
	{
		if (temp[0]->m_tris.size() > 200)
		{
			temp[0]->Divide(temp[0]->m_ALL);
			temp[0]->Distribute();

			for (size_t i = 0; i < 4; i++)
				temp.push_back(temp[0]->m_quadrants[i]);
		}

		temp.erase(temp.begin());
	}
}

/*
Reads vertecies, uvs, normals and other object stuff from a .obj file
Followed http://www.opengl-tutorial.org/beginners-tutorials/tutorial-7-model-loading/ tutorial
*/
bool BaseObject::ReadObject(string _filePath, float _shine)
{
	// Local variables
	vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	vector<XMFLOAT4> temp_vertices;
	vector<XMFLOAT4> temp_normals;
	vector<XMFLOAT4> temp_uvs;
	FILE *file;

	// Opening file - "r" -> read in
	fopen_s(&file, _filePath.c_str(), "r");
	// Check if file opened
	if (file == NULL)
		return false;

	// Looping until the file finishes
	while (true)
	{
		// Read the first word of the line
		char lineHeader[128];
		int res = fscanf_s(file, "%s", lineHeader, _countof(lineHeader));

		// Check if the file has finished
		if (res == EOF)
			break;

		// Check if the line is a vertex
		if (strcmp(lineHeader, "v") == 0)
		{
			XMFLOAT4 vertex;
			fscanf_s(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			temp_vertices.push_back(vertex);
		}
		// Check if the line is a UV
		else if (strcmp(lineHeader, "vt") == 0)
		{
			XMFLOAT4 uv;
			fscanf_s(file, "%f %f\n", &uv.x, &uv.y);
			temp_uvs.push_back(uv);
		}
		// Check if the line is a normal
		else if (strcmp(lineHeader, "vn") == 0)
		{
			XMFLOAT4 normal;
			fscanf_s(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			temp_normals.push_back(normal);
		}
		// Check if the line is a index
		else if (strcmp(lineHeader, "f") == 0)
		{
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf_s(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches != 9)
				return false;

			// Setting the indicies for the vertecies
			vertexIndices.push_back(vertexIndex[0] - 1);
			vertexIndices.push_back(vertexIndex[1] - 1);
			vertexIndices.push_back(vertexIndex[2] - 1);

			// Setting the indicies for the UVs
			uvIndices.push_back(uvIndex[0] - 1);
			uvIndices.push_back(uvIndex[1] - 1);
			uvIndices.push_back(uvIndex[2] - 1);

			// Setting the indicies for the normals
			normalIndices.push_back(normalIndex[0] - 1);
			normalIndices.push_back(normalIndex[1] - 1);
			normalIndices.push_back(normalIndex[2] - 1);
		}
	}

	// Setting vertecies member
	m_numVerts = vertexIndices.size();
	m_vertecies = new VERTEX[m_numVerts];
	for (unsigned int i = 0; i < m_numVerts; ++i)
	{
		// Setting vertecies
		m_vertecies[i].transform = temp_vertices[vertexIndices[i]];
		// Setting normals
		m_vertecies[i].normals = temp_normals[normalIndices[i]];
		// Setting UVs
		m_vertecies[i].uv.x = temp_uvs[uvIndices[i]].x;
		m_vertecies[i].uv.y = temp_uvs[uvIndices[i]].y;
		m_vertecies[i].uv.z = _shine;

	}

	// Computing the tangents and bitangents
	ComputeTangents();
	// Setting indecies member
	m_numIndicies = vertexIndices.size();
	m_indexList = new unsigned int[m_numIndicies];
	for (unsigned int i = 0; i < m_numIndicies; ++i)
		m_indexList[i] = i;

	// Return true if everything went right
	return true;
}

void BaseObject::SetQuadTri(float _x, float _y, float _z)
{
	if (_y > 14)
		_y *= -1;

	for (size_t i = 0; i < m_tris.size(); i++)
	{
		m_tris[i].m_one.transform.x += _x;
		m_tris[i].m_one.transform.y += _y;
		m_tris[i].m_one.transform.z += _z;
		m_tris[i].m_two.transform.x += _x;
		m_tris[i].m_two.transform.y += _y;
		m_tris[i].m_two.transform.z += _z;
		m_tris[i].m_three.transform.x += _x;
		m_tris[i].m_three.transform.y += _y;
		m_tris[i].m_three.transform.z += _z;
	}
}

void BaseObject::ResetTris()
{
	XMFLOAT4X4 world = m_worldToShader.worldMatrix;
	for (size_t i = 0; i < m_tris.size(); i++)
	{
		m_tris[i].m_one.transform.x -= world._41;
		m_tris[i].m_one.transform.y -= world._42;
		m_tris[i].m_one.transform.z -= world._43;
		m_tris[i].m_two.transform.x -= world._41;
		m_tris[i].m_two.transform.y -= world._42;
		m_tris[i].m_two.transform.z -= world._43;
		m_tris[i].m_three.transform.x -= world._41;
		m_tris[i].m_three.transform.y -= world._42;
		m_tris[i].m_three.transform.z -= world._43;
	}
}

void BaseObject::InvertWinding()
{
	m_indexListINV = new unsigned int[m_numIndicies];

	for (size_t i = 0; i < m_numIndicies / 2; i++)
	{
		m_indexListINV[i] = m_indexList[i + 1];
		m_indexListINV[i + 1] = m_indexList[i];
		m_indexListINV[i + 2] = m_indexList[i + 2];
	}
}