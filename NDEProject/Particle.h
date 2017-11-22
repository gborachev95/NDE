#pragma once
#include "includes.h"
class BaseObject;
enum PARTICLE_TYPE {WEIGHTED, SMOKE, MAGNET};
class Particle
{
private:
	float                             m_particleDeviationX, m_particleDeviationY, m_particleDeviationZ;
	float                             m_particleVelocity, m_particleVelocityVariation;
	float                             m_particleSize, m_particlesPerSecond;
	unsigned int                      m_maxParticles;
	unsigned int                      m_currentParticleCount;
	float                             m_accumulatedTime;
	ParticleOBJ*                      m_particleList;
	unsigned int                      m_vertexCount, m_indexCount;
	P_VERTEX*                         m_vertices;
	CComPtr<ID3D11Buffer>             m_vertexBuffer, m_indexBuffer;
	CComPtr<ID3D11ShaderResourceView> m_texture;
	XMFLOAT3                          m_age;
	XMFLOAT3                          m_color;
	XMFLOAT3                          m_position;
	XMFLOAT3                          m_minTrajectory;
	XMFLOAT3                          m_maxTrajectory;
	BaseObject*                       m_particle;
	float                             m_division; 
	unsigned int                      m_particleType;
	MOD_TO_VRAM                       m_modData;
	CComPtr<ID3D11Buffer>             m_constBuffer;
	// SORT LIST
	// DEAD LIST
public:
	Particle();
	~Particle();

	//The class functions are the regular initialize, shutdown, frame, and render.However note that the Frame function is where we do all the work of
	//updating, sorting, and rebuilding the of vertex buffer each frame so the particles can be rendered correctly.
	void Initialize(const wchar_t*  _path, XMFLOAT3 _position, XMFLOAT3 _minTra, XMFLOAT3 _maxTra, unsigned int _partNum, float _spawnRate, float _size, unsigned int _particleType);

	void Input();
	void Update(bool _emit, XMFLOAT4 _color);
	void Render();
	void ResetParticle();

	// Setters
	void SetParticleDeviation(XMFLOAT3 _deviantion) { m_particleDeviationX = _deviantion.x; m_particleDeviationY = _deviantion.y; m_particleDeviationZ = _deviantion.z; }
	void SetParticleVelocity(float _velocity) {	m_particleVelocity = _velocity; }
	void SetParticleSize(float _size) { m_particleSize = _size; }
	void SetBillBoardVector(XMFLOAT4X4 _camView);
	void SetPosition(XMFLOAT3 _pos) { m_position = _pos; }
	void SetTransparency(float _t) { m_modData.uvTranslation.y = _t; }
	// Getters 
	int GetIndexCount() { return m_indexCount; }
	BaseObject* GetParticle() { return m_particle; }
private:
	void ShutdownParticleSystem();
	void InitializeBuffers();
	void EmitParticles();
	void UpdateParticles(XMFLOAT4 _color);
	void KillParticles();
	void UpdateBuffers(ID3D11DeviceContext*);
	void RenderBuffers(ID3D11DeviceContext*);
	void CreateConstBuffer();
	void SetInitialVelocity(int _ind, XMFLOAT3 _minTra, XMFLOAT3 _maxTra);
};

