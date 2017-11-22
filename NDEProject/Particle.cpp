#include "Particle.h"
#include "Graphics.h"
#include "DDSTextureLoader.h"
#include "BaseObject.h"
#include "Collision.h"

Particle::Particle()
{
	m_particleList = nullptr;
	m_vertices = 0;
	m_vertexBuffer = 0;
	m_indexBuffer = 0;
	m_particle = nullptr;
	m_indexCount = 0;
	ZeroMemory(&m_modData, sizeof(MOD_TO_VRAM));
}

Particle::~Particle()
{
	delete m_particle;
	delete[] m_vertices;
	delete[] m_particleList;
}

void Particle::Initialize(const wchar_t*  _path, XMFLOAT3 _position, XMFLOAT3 _minTra, XMFLOAT3 _maxTra, unsigned int _partNum, float _spawnRate, float _size, unsigned int _particleType)
{
	// Set the maximum number of particles allowed in the particle system.
	m_maxParticles = _partNum;

	if (!m_particle)
	{
		m_particle = new BaseObject();
		m_particle->InstantiateModel(Graphics::GetDevice() , "..\\NDEProject\\Assets\\boneSphere.obj", XMFLOAT3(0,0,0), 0, UI);
		m_particle->TextureObject(Graphics::GetDevice(), _path);
		CreateDDSTextureFromFile(Graphics::GetDevice(), _path, NULL, &m_texture.p);
		m_particle->SetPosition(_position.x, _position.y, _position.z);
		SetPosition(_position);

		// Load the texture that is used for the particles.

		if (m_indexCount == 0)
		// Create the buffers that will be used to render the particles with.
			InitializeBuffers();
	
		m_division = 180.0f / 3.141519f;
	}

	// Set the random deviation of where the particles can be located when emitted.

	m_particleType = _particleType;

	if (m_particleType == MAGNET)
	{
		m_particleDeviationX = /*-66.0f*/10.0f;
		m_particleDeviationY = /*-66.0f*/10.0f;
		m_particleDeviationZ = /*-66.0f*/10.0f;
	}
	else
	{
		m_particleDeviationX = /*-66.0f*/0.2f;
		m_particleDeviationY = /*-66.0f*/0.1f;
		m_particleDeviationZ = /*-66.0f*/-0.5f;
	}

	// Set the speed and speed variation of particles.
	// FOR SAND
	//m_particleVelocity = 4.0f;
	m_particleVelocity = 1.5f;
	m_particleVelocityVariation = 0.2f;

	// Set the physical size of the particles.
	m_particleSize = _size;

	// Set the number of particles to emit per second.
	m_particlesPerSecond = _spawnRate;

	//We then create the particle array based on the maximum number of particles that will be used.
	// Create the particle list.
	if (!m_particleList)
		m_particleList = new ParticleOBJ[m_maxParticles];

	m_minTrajectory = _minTra;
	m_maxTrajectory = _maxTra;

	//Set each particle in the array to inactive to begin with.
	// Initialize the particle list.
	for (unsigned int i = 0; i < m_maxParticles; i++)
	{
		m_particleList[i].pToV.active = false;
		m_particleList[i].m_age = 0;
	}

	//Initialize the two counters to zero to start with.
	// Initialize the current particle count to zero since none are emitted yet.
	m_currentParticleCount = 0;
	// Clear the initial accumulated time for the particle per second emission rate.
	m_accumulatedTime = 0.0f;

}

void Particle::Input()
{
#if 0
	if (Graphics::single_keyboard->GetState().D1)
	{
		m_particleDeviationX += float(Graphics::GetTime().Delta()) * 10.0f;
		m_particleDeviationY += float(Graphics::GetTime().Delta()) * 10.0f;
		m_particleDeviationZ += float(Graphics::GetTime().Delta()) * 10.0f;
	}
	else if (Graphics::single_keyboard->GetState().D2)
	{
		m_particleDeviationX -= float(Graphics::GetTime().Delta()) * 10.0f;
		m_particleDeviationY -= float(Graphics::GetTime().Delta()) * 10.0f;
		m_particleDeviationZ -= float(Graphics::GetTime().Delta()) * 10.0f;
	}

	if (Graphics::single_keyboard->GetState().D3)
		m_particleVelocity += float(Graphics::GetTime().Delta()) * 10.0f;
	else if (Graphics::single_keyboard->GetState().D4)
		m_particleVelocity -= float(Graphics::GetTime().Delta()) * 10.0f;

	if (Graphics::single_keyboard->GetState().D5)
		m_particleVelocityVariation += float(Graphics::GetTime().Delta()) * 10.0f;
	else if (Graphics::single_keyboard->GetState().D6)
		m_particleVelocityVariation -= float(Graphics::GetTime().Delta()) * 10.0f;
#endif
}

void Particle::Update(bool _emit, XMFLOAT4 _color)
{
	m_particle->SetPosition(m_position.x, m_position.y, m_position.z);

	// Release old particles.
	KillParticles();

	// Emit new particles.
	if (_emit)
		EmitParticles();

	// Update the position of the particles.
	UpdateParticles(_color);

	// Update the dynamic vertex buffer with the new position of each particle.
    UpdateBuffers(Graphics::GetDeviceContext());
}

void Particle::Render()
{
	// 
	D3D11_MAPPED_SUBRESOURCE mapSubresource;
	Graphics::GetDeviceContext()->PSSetConstantBuffers(1, 1, &m_constBuffer.p);
	ZeroMemory(&mapSubresource, sizeof(mapSubresource));
	Graphics::GetDeviceContext()->Map(m_constBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mapSubresource);
	memcpy(mapSubresource.pData, &m_modData, sizeof(MOD_TO_VRAM));
	Graphics::GetDeviceContext()->Unmap(m_constBuffer, NULL);

	m_particle->Render();
	// Put the vertex and index buffers on the graphics pipeline to prepare them for drawing.
	RenderBuffers(Graphics::GetDeviceContext());
}

void Particle::ShutdownParticleSystem()
{
	// Release the particle list.
	if (m_particleList)
	{
		delete[] m_particleList;
		m_particleList = 0;
	}
}

void Particle::InitializeBuffers()
{
	unsigned long* indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;

	// Set the maximum number of vertices in the vertex array.
	m_vertexCount = m_maxParticles * 6;

	// Set the maximum number of indices in the index array.
	m_indexCount = m_vertexCount;

	// Create the vertex array for the particles that will be rendered.
	m_vertices = new P_VERTEX[m_vertexCount];

	// Create the index array.
	indices = new unsigned long[m_indexCount];

	// Initialize vertex array to zeros at first.
	memset(m_vertices, 0, (sizeof(P_VERTEX) * m_vertexCount));

	// Initialize the index array.
	for (unsigned int i = 0; i < m_indexCount; i++)
		indices[i] = i;

	// Set up the description of the dynamic vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = sizeof(P_VERTEX) * m_vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = m_vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now finally create the vertex buffer.
	Graphics::GetDevice()->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);

	// Set up the description of the static index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	Graphics::GetDevice()->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);

	// Release the index array since it is no longer needed.
	delete[] indices;
	indices = 0;

	CreateConstBuffer();
}

void Particle::EmitParticles()
{
	bool found;
	float positionX, positionY, positionZ, velocity;
	int index, i, j;

	m_accumulatedTime += float(Graphics::GetTime().Delta());

	// If there are particles to emit then emit one per frame.
	while ((m_currentParticleCount < (m_maxParticles - 1)) && m_accumulatedTime > 0)
	{
		// Check if it is time to emit a new particle or not.
			m_accumulatedTime -= m_particlesPerSecond;

		m_currentParticleCount++;

		// Now generate the randomized particle properties.
		positionX = (((float)rand() - (float)rand()) / RAND_MAX) * m_particleDeviationX;
		positionY = (((float)rand() - (float)rand()) / RAND_MAX) * m_particleDeviationY;
		positionZ = (((float)rand() - (float)rand()) / RAND_MAX) * m_particleDeviationZ;

		velocity = m_particleVelocity + (((float)rand() - (float)rand()) / RAND_MAX) * m_particleVelocityVariation;

		// Now since the particles need to be rendered from back to front for blending we have to sort the particle array.
		// We will sort using Z depth so we need to find where in the list the particle should be inserted.
		index = 0;
		found = false;
		while (!found)
		{
			if ((m_particleList[index].pToV.active == false) || (m_particleList[index].pToV.position.z < positionZ))
				found = true;
			else
				index++;
		}

		// Now that we know the location to insert into we need to copy the array over by one position from the index to make room for the new particle.
		i = m_currentParticleCount;
		j = i - 1;

		while (i != index)
		{
			m_particleList[i].pToV.position.x = m_particleList[j].pToV.position.x;
			m_particleList[i].pToV.position.y = m_particleList[j].pToV.position.y;
			m_particleList[i].pToV.position.z = m_particleList[j].pToV.position.z;
			//m_particleList[i].pToV.color.x = m_particleList[j].pToV.color.x;
			//m_particleList[i].pToV.color.y = m_particleList[j].pToV.color.y;
			//m_particleList[i].pToV.color.z = m_particleList[j].pToV.color.z;
			m_particleList[i].pToV.velocity = m_particleList[j].pToV.velocity;
			m_particleList[i].pToV.active = m_particleList[j].pToV.active;
			m_particleList[i].m_trajectory = m_particleList[j].m_trajectory;
			m_particleList[i].m_age = m_particleList[j].m_age;
			m_particleList[index].world._41 = m_particleList[j].world._41;
			m_particleList[index].world._42 = m_particleList[j].world._42;
			m_particleList[index].world._43 = m_particleList[j].world._43;
			m_particleList[index].world._44 = 1;
			i--;
			j--;
		}

		// Now insert it into the particle array in the correct depth order.
		m_particleList[index].pToV.position.x = positionX;
		m_particleList[index].pToV.position.y = positionY;
		m_particleList[index].pToV.position.z = positionZ;
		//m_particleList[index].pToV.color.x = red;
		//m_particleList[index].pToV.color.y = green;
		//m_particleList[index].pToV.color.z = blue;
		m_particleList[index].pToV.velocity = velocity;
		m_particleList[index].pToV.active = true;
		SetInitialVelocity(index, m_minTrajectory, m_maxTrajectory);
		m_particleList[index].m_age = 0;
		XMStoreFloat4x4(&m_particleList[index].world, XMMatrixIdentity());
		m_particleList[index].world._41 = positionX;
		m_particleList[index].world._42 = positionY;
		m_particleList[index].world._43 = positionZ;
		m_particleList[index].world._44 = 1;
	}

	// TODO::NEEDS TO BE REFRESHED 
	// UV scrolling
    m_modData.uvTranslation.y += float(Graphics::GetTime().Delta()) * 0.1f;
}

void Particle::UpdateParticles(XMFLOAT4 _color)
{
	// Each frame we update all the particles by making them move downwards using their position, velocity, and the frame time.
	// FOR HOLDING SAND
	//for (unsigned int i = 0; i<m_currentParticleCount; i++)
	//	m_particleList[i].pToV.position.y = m_particleList[i].pToV.position.y - (m_particleList[i].pToV.velocity * float(Graphics::GetTime().Delta()));

	for (unsigned int i = 0; i<m_currentParticleCount; i++)
	{
		//int numX = rand() % 100 - 50;
		//int numY = rand() % 80 - 40;
		//m_particleList[i].pToV.position.x = m_particleList[i].pToV.position.x - (m_particleList[i].pToV.velocity * float(Graphics::GetTime().Delta()) * numX * .2f);
		//m_particleList[i].pToV.position.y = m_particleList[i].pToV.position.y - (m_particleList[i].pToV.velocity * float(Graphics::GetTime().Delta()) * numY * .2f);
		//m_particleList[i].pToV.position.z = m_particleList[i].pToV.position.z - (m_particleList[i].pToV.velocity * float(Graphics::GetTime().Delta()));

		m_particleList[i].m_age += float(Graphics::GetTime().Delta());
		if (m_particleType == WEIGHTED)
		{
			m_particleList[i].pToV.position.x = 0;
			m_particleList[i].pToV.position.y = m_particleList[i].pToV.position.y - (m_particleList[i].pToV.velocity * float(Graphics::GetTime().Delta()) * m_particleList[i].m_trajectory.y * .2f) - m_particleList[i].m_age * 0.5f;
			m_particleList[i].pToV.position.z = 0;
		}
		else if (m_particleType == SMOKE)
		{
			m_particleList[i].pToV.position.x = m_particleList[i].pToV.position.x - (m_particleList[i].pToV.velocity * float(Graphics::GetTime().Delta()) * m_particleList[i].m_trajectory.x * .2f) * (1.0f / m_particleList[i].m_age * 0.1f);
			m_particleList[i].pToV.position.y = m_particleList[i].pToV.position.y - (m_particleList[i].pToV.velocity * float(Graphics::GetTime().Delta()) * m_particleList[i].m_trajectory.y * .2f) * (1.0f / m_particleList[i].m_age * 0.1f);
			m_particleList[i].pToV.position.z = m_particleList[i].pToV.position.z - (m_particleList[i].pToV.velocity * float(Graphics::GetTime().Delta()) * m_particleList[i].m_trajectory.z * .2f) * (1.0f / m_particleList[i].m_age * 0.1f);
		}
		else if (m_particleType == MAGNET)
		{
			m_particleList[i].pToV.position.x = m_particleList[i].pToV.position.x - (m_particleList[i].pToV.velocity * float(Graphics::GetTime().Delta()) * m_particleList[i].m_trajectory.x);
			m_particleList[i].pToV.position.y = m_particleList[i].pToV.position.y - (m_particleList[i].pToV.velocity * float(Graphics::GetTime().Delta()) * m_particleList[i].m_trajectory.y);
			m_particleList[i].pToV.position.z = m_particleList[i].pToV.position.z - (m_particleList[i].pToV.velocity * float(Graphics::GetTime().Delta()) * m_particleList[i].m_trajectory.z);
		}
		m_particleList[i].pToV.color.x = _color.x;
		m_particleList[i].pToV.color.y = _color.y;
		m_particleList[i].pToV.color.z = _color.z;
		m_particleList[i].pToV.color.w = _color.w;
	}
	return;
}

void Particle::KillParticles()
{
	// Kill all the particles that have gone below a certain height range.
	for (unsigned int i = 0; i<m_maxParticles; i++)
	{
		bool die = false;
		if (m_particleType == MAGNET)
		{
			if ((m_particleList[i].pToV.active == true) && Collision::DistanceFormula(XMFLOAT3{ m_particleList[i].pToV.position.x, m_particleList[i].pToV.position.y, m_particleList[i].pToV.position.z }, XMFLOAT3{ 0, 0, 0 }) < 0.5f)
				die = true;
		}
		else
		{
			if ((m_particleList[i].pToV.active == true) && (m_particleList[i].m_age > 10))
				die = true;
			//else: Near death... Get it?
		}
		if (die)
		{
			m_particleList[i].pToV.active = false;
			m_currentParticleCount--;

			// Now shift all the live particles back up the array to erase the destroyed particle and keep the array sorted correctly.
			for (unsigned int j = i; j < m_maxParticles - 1; j++)
			{
				m_particleList[j].pToV.position.x = m_particleList[j + 1].pToV.position.x;
				m_particleList[j].pToV.position.y = m_particleList[j + 1].pToV.position.y;
				m_particleList[j].pToV.position.z = m_particleList[j + 1].pToV.position.z;
				m_particleList[j].pToV.color.x = m_particleList[j + 1].pToV.color.x;
				m_particleList[j].pToV.color.y= m_particleList[j + 1].pToV.color.y;
				m_particleList[j].pToV.color.z = m_particleList[j + 1].pToV.color.z;
				m_particleList[j].pToV.color.w = m_particleList[j + 1].pToV.color.w;
				m_particleList[j].pToV.velocity = m_particleList[j + 1].pToV.velocity;
				m_particleList[j].pToV.active = m_particleList[j + 1].pToV.active;
				m_particleList[j].m_trajectory = m_particleList[j + 1].m_trajectory;
				m_particleList[j].m_age = m_particleList[j + 1].m_age;
			}
		}
	}

	return;
}

void Particle::UpdateBuffers(ID3D11DeviceContext*)
{
	int index;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	P_VERTEX* verticesPtr;


	// Initialize vertex array to zeros at first.
	memset(m_vertices, 0, (sizeof(P_VERTEX) * m_vertexCount));

	// Now build the vertex array from the particle list array.  Each particle is a quad made out of two triangles.
	index = 0;

	for (unsigned int i = 0; i < m_currentParticleCount; i++)
	{
		for (size_t j = 0; j < 6; j++)
		{
			float xStuff = m_particleList[i].pToV.position.x, yStuff = m_particleList[i].pToV.position.y;
			XMFLOAT4 uv;
			if (j == 0)
				uv = XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f);
			else if (j == 1 || j == 4)
				uv = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
			else if (j == 2 || j == 3)
				uv = XMFLOAT4(1.0f, 1.0f, 0.0f, 0.0f);
			else
				uv = XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f);

			if (j == 0 || j == 1 || j == 4)
				xStuff -= m_particleSize;
			else
				xStuff += m_particleSize;

			if (j == 0 || j == 2 || j == 3)
				yStuff -= m_particleSize;
			else
				yStuff += m_particleSize;

			m_vertices[index].position = XMFLOAT4(xStuff, yStuff, m_particleList[i].pToV.position.z, 0.0f);
			m_vertices[index].uv = uv;
			m_vertices[index].color = XMFLOAT4(m_particleList[i].pToV.color.x, m_particleList[i].pToV.color.y, m_particleList[i].pToV.color.z, m_particleList[i].pToV.color.w);
			//m_vertices[index].side = XMFLOAT4(1, 0, 0, 1);
			//m_vertices[index].up = XMFLOAT4(0, 1, 0, 1);
			//m_vertices[index].forward = XMFLOAT4(0, 0, 1, 1);
			//XMStoreFloat4(&m_vertices[index].side, XMVECTOR{ m_particleList[i].world._11, m_particleList[i].world._12, m_particleList[i].world._13, m_particleList[i].world._14 });
			//XMStoreFloat4(&m_vertices[index].up, XMVECTOR{ m_particleList[i].world._21, m_particleList[i].world._22, m_particleList[i].world._23, m_particleList[i].world._24 });
			//XMStoreFloat4(&m_vertices[index].forward, XMVECTOR{ m_particleList[i].world._31, m_particleList[i].world._32, m_particleList[i].world._33, m_particleList[i].world._34 });
			XMStoreFloat4(&m_vertices[index].side, XMVECTOR{ m_particleList[i].toShader._11, m_particleList[i].toShader._12, m_particleList[i].toShader._13, m_particleList[i].world._14 });
			XMStoreFloat4(&m_vertices[index].up, XMVECTOR{ m_particleList[i].toShader._21, m_particleList[i].toShader._22, m_particleList[i].toShader._23, m_particleList[i].world._24 });
			XMStoreFloat4(&m_vertices[index].forward, XMVECTOR{ m_particleList[i].toShader._31, m_particleList[i].toShader._32, m_particleList[i].toShader._33, m_particleList[i].world._34 });
			index++;
		}
	}

	// Lock the vertex buffer.
	Graphics::GetDeviceContext()->Map(m_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	// Get a pointer to the data in the vertex buffer.
	verticesPtr = (P_VERTEX*)mappedResource.pData;

	// Copy the data into the vertex buffer.
	memcpy(verticesPtr, (void*)m_vertices, (sizeof(P_VERTEX) * m_vertexCount));

	// Unlock the vertex buffer.
	Graphics::GetDeviceContext()->Unmap(m_vertexBuffer, 0);
}

void Particle::RenderBuffers(ID3D11DeviceContext*)
{
	unsigned int stride;
	unsigned int offset;

	// Set vertex buffer stride and offset.
	stride = sizeof(P_VERTEX);
	offset = 0;

	Graphics::GetDeviceContext()->VSSetShader(Graphics::m_vsShaders[PARTICLE].p, NULL, NULL);
	Graphics::GetDeviceContext()->PSSetShader(Graphics::m_psShaders[PARTICLE].p, NULL, NULL);
	Graphics::GetDeviceContext()->IASetInputLayout(Graphics::m_layouts[PARTICLE].p);
	// Set the vertex buffer to active in the input assembler so it can be rendered.
	Graphics::GetDeviceContext()->IASetVertexBuffers(0, 1, &m_vertexBuffer.p, &stride, &offset);

	// Set the index buffer to active in the input assembler so it can be rendered.
	Graphics::GetDeviceContext()->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Set the type of primitive that should be rendered from this vertex buffer.
	Graphics::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	//D3D11_MAPPED_SUBRESOURCE mapSubresource;
	//Graphics::GetDeviceContext()->PSSetConstantBuffers(1, 1, &m_constBuffer.p);
	//// Bones data
	//ZeroMemory(&mapSubresource, sizeof(mapSubresource));
	//Graphics::GetDeviceContext()->Map(m_constBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mapSubresource);
	//memcpy(mapSubresource.pData, &m_scrolling, sizeof(UI_TO_VRAM));
	//Graphics::GetDeviceContext()->Unmap(m_constBuffer, NULL);
	

	Graphics::GetDeviceContext()->DrawIndexed(m_indexCount, 0, 0);
}

void Particle::SetBillBoardVector(XMFLOAT4X4 _camView)
{
	// Get the position of the camera.
	//XMVECTOR cameraPosition = XMMatrixInverse(0,XMLoadFloat4x4(&_camView)).r[3];
	//XMVECTOR particlePosition = m_particle->GetWorldMatrix().r[3];
	//
	//// Calculate the rotation that needs to be applied to the billboard model to face the current camera position using the arc tangent function.
	//float angle = atan2(particlePosition.m128_f32[0] - cameraPosition.m128_f32[0], particlePosition.m128_f32[2] - cameraPosition.m128_f32[2]) * m_division;
	//
	//// Convert rotation into radians.
	//float rotation = angle * 0.0174532925f;
	//
	//// Setup the rotation the billboard at the origin using the world matrix.
	//m_particle->SetWorldMatrix(XMMatrixMultiply(m_particle->GetWorldMatrix(),XMMatrixRotationY(rotation)));

#if 1
	// BILLBOARDING ON THE PARTICLES
	//for (size_t i = 0; i < m_maxParticles; i++)
	//{
	//	XMMATRIX world = XMLoadFloat4x4(&m_particleList[i].world);
	//	XMVECTOR side = XMLoadFloat4(&XMFLOAT4{ m_particleList[i].world._11, m_particleList[i].world._12, m_particleList[i].world._13, 1 });
	//	XMVECTOR particle = XMLoadFloat4(&XMFLOAT4{ m_particleList[i].world._41, m_particleList[i].world._42, m_particleList[i].world._43, 1 });
	//	XMVECTOR camPos = XMMatrixInverse(0, XMLoadFloat4x4(&_camView)).r[3];
	//	XMVECTOR dir = particle - camPos;
	//	XMVECTOR normalized = XMVector3Normalize(dir);
	//	
	//	XMVECTOR x = XMVector3Cross(XMVECTOR{ 0,1,0 }, normalized);
	//	x = XMVector3Normalize(x);
	//	XMVECTOR y = XMVector3Cross(normalized, x);
	//	y = XMVector3Normalize(y);
	//	world.r[0] = x;
	//	world.r[1] = y;
	//	world.r[2] = normalized;
	//	
	//	//XMVECTOR xDot = XMVector3Dot(normalized, side) * -1.5f;
	//	//XMMATRIX rot = XMMatrixMultiply(XMMatrixRotationY(xDot.m128_f32[0]), XMMatrixIdentity());
	//	//rot.r[3] = particle;
	//	//world = rot * m_particle->GetWorldMatrix();
	//	//world = world * XMLoadFloat4x4(&m_particleList[i].world);
	//
	//	XMStoreFloat4x4(&m_particleList[i].toShader, world);
	//}

	// BILLBOARDING ON THE EMITTER
	XMMATRIX world = m_particle->GetWorldMatrix();
	world *= XMMatrixRotationY(45);

	XMVECTOR particle = m_particle->GetWorldMatrix().r[3];
	XMVECTOR camPos = XMMatrixInverse(0, XMLoadFloat4x4(&_camView)).r[3];
	XMVECTOR dir = particle - camPos;
	XMVECTOR normalized = XMVector3Normalize(dir);
	
	XMVECTOR x = XMVector3Cross(XMVECTOR{ 0,1,0 }, normalized);
	x = XMVector3Normalize(x);
	XMVECTOR y = XMVector3Cross(normalized, x);
	y = XMVector3Normalize(y);
	world.r[0] = x;
	world.r[1] = y;
	world.r[2] = normalized;
	
	// ALTERNATIVE METHOD
	//XMVECTOR xDot = XMVector3Dot(normalized, m_particle->GetWorldMatrix().r[0]) * -1.5f;
	//world = XMMatrixMultiply(XMMatrixRotationY(xDot.m128_f32[0]), world);
	
	m_particle->SetWorldMatrix(world);



	// Setup the translation matrix from the billboard model.
	//D3DXMatrixTranslation(&translateMatrix, modelPosition.x, modelPosition.y, modelPosition.z);

	// Finally combine the rotation and translation matrices to create the final world matrix for the billboard model.
	//D3DXMatrixMultiply(&worldMatrix, &worldMatrix, &translateMatrix);


	// My stuff
	//XMVECTOR forwardVector = XMVector3Normalize(_camView - m_particle->GetWorldMatrix().r[3]);
	//XMVECTOR angle = XMVector3Dot(forwardVector, m_particle->GetWorldMatrix().r[2]);
	//XMMATRIX newMat = XMMatrixMultiply(XMMatrixRotationY(angle.m128_f32[0]), m_particle->GetWorldMatrix());
	//m_particle->SetWorldMatrix(newMat);
#endif
}

void Particle::CreateConstBuffer()
{
	D3D11_BUFFER_DESC constBufferDesc;
	ZeroMemory(&constBufferDesc, sizeof(D3D11_BUFFER_DESC));
	constBufferDesc.ByteWidth = sizeof(MOD_TO_VRAM);
	constBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	constBufferDesc.StructureByteStride = sizeof(float);
	constBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	Graphics::GetDevice()->CreateBuffer(&constBufferDesc, NULL, &m_constBuffer.p);
}

void Particle::SetInitialVelocity(int _ind, XMFLOAT3 _minTra, XMFLOAT3 _maxTra)
{
	if (m_particleType == MAGNET)
	{
		m_particleList[_ind].m_trajectory = XMFLOAT3{ m_particleList[_ind].pToV.position.x, 
			m_particleList[_ind].pToV.position.y, m_particleList[_ind].pToV.position.z };
	}
	else
	{
		m_particleList[_ind].m_trajectory.x = float(rand() % (int)_maxTra.x - _minTra.x);
		m_particleList[_ind].m_trajectory.y = float(rand() % (int)_maxTra.y - _minTra.y);
		m_particleList[_ind].m_trajectory.z = float(rand() % (int)_maxTra.z - _minTra.z);
	}
}

void Particle::ResetParticle()
{
	unsigned int count = m_currentParticleCount;
	// Kill all the particles that have gone below a certain height range.
	for (unsigned int i = 0; i < count; i++)
	{
		m_particleList[i].pToV.active = false;
		m_currentParticleCount--;
	}
}