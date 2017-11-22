#pragma once
#ifndef BaseObject_H
#define BaseObject_H
#include "Graphics.h"
#include "Animation.h"
#include "Transform.h"
#include "XTime.h"
#include "Quad.h"
#include "KeyFrame.h"

class Blender;

/*__declspec(align(16)) */class BaseObject
{
	// Graphics variables
	CComPtr<ID3D11Buffer>              m_indexBuffer;
	CComPtr<ID3D11Buffer>              m_indexBufferINV;
	CComPtr<ID3D11Buffer>              m_vertexBuffer;
	CComPtr<ID3D11Buffer>              m_instanceBuffer;
	CComPtr<ID3D11Texture2D>           m_texture;
	CComPtr<ID3D11ShaderResourceView>  m_defShaderResourceView;
	CComPtr<ID3D11ShaderResourceView>  m_normalShaderResourceView;
	CComPtr<ID3D11ShaderResourceView>  m_specularShaderResourceView;
	CComPtr<ID3D11ShaderResourceView>  m_emissiveShaderResourceView;
	CComPtr<ID3D11Buffer>              m_constBuffer;
	CComPtr<ID3D11Buffer>              m_bonesConstBuffer;
	OBJECT_TO_VRAM                     m_worldToShader;
	// Object variables
	VERTEX                             *m_vertecies;
	unsigned int                       *m_indexList;
	unsigned int                       *m_indexListINV;
	unsigned int                        m_numVerts;
	unsigned int                        m_numTris;
	unsigned int                        m_numIndicies;
	Blender                            *m_currBlender;
	Animation                          *m_currAnimation;
	Animation                          *m_nextAnimation;
	Animation                          *m_lastNextAnimation;
	vector<Animation*>                  m_animations;
	HitBox                             *m_hBox;
	std::vector<Transform>              m_bindBones;
	std::vector<BaseObject*>            m_bonesObjects;
	GRAPHICS                            m_graphicsType;
	OBJECT_ID                           m_id;
	BONES_TO_VRAM                       m_bonesToGPU;
	bool                                m_inAir;
	bool                                m_active;
	bool                                m_target;
	float                               m_mass;
	XTime                               m_timer;
	XMFLOAT3                            m_force;
	bool                                m_animBusy;
	Tri*                                m_ground;
	Tri*                                m_wall;
	std::vector<Tri>                    m_tris;
	Quad*                               m_quad;
	KeyFrame                            m_interpedFrame;
	XMFLOAT4X4*                         m_instanceMatricis;
    unsigned int                        m_numInstances;
	// Debugging variables
	int									d_frame;
	ID3D11RasterizerState*				d_rsSolid;
	ID3D11RasterizerState*				d_rsWireframe;
	unsigned int                        m_index;
	
	// Private methods that are called only inside of the class
private:
	void CreateVertexBuffer(ID3D11Device* _device);
	void CreateIndexBuffer(ID3D11Device* _device);
	void ComputeTangents();
	bool ReadObject(string filePath, float _shine);
	void CreateConstBuffer(ID3D11Device* _device);

protected:
	// Loader
	void LoadBinaryFile(std::string _filePath, std::vector<VERTEX>& _vertecies, std::vector<unsigned int>& _indices);
	void LoadBinaryFile(std::string _filePath, std::vector<Transform>& _bones);
	void LoadBinaryFile(std::string _filePath, Animation& _animation);
	void LoadBinaryFile(std::string _filePath);

	// Public methods that are called outside of the class
public:
	// Construcor
	BaseObject();
	BaseObject(OBJECT_ID _id, float _mass, bool _inAir);

	// Destructor
	virtual ~BaseObject();

	void InstantiateModel(ID3D11Device* _device, std::string _filePath, XMFLOAT3 _position, float _shine, GRAPHICS _type);
	void InstantiateFBX(ID3D11Device* _device, std::string _filePath, XMFLOAT3 _position, float _shine, GRAPHICS _type);
	void InitializeInstances(XMFLOAT4X4* _matrixData, unsigned int _numInstances);

	virtual void Input();
	virtual void Update();
	virtual void Render();

	void TextureObject(ID3D11Device* _device, const wchar_t*  _filePathToDefuse, const wchar_t*  _filePathToNormalMap = nullptr, const wchar_t*  _filePathToSpecular = nullptr, const wchar_t*  _filePathToEmissive = L"..\\NDEProject\\Assets\\Textures\\NoEmission.dds");
	void UpdateAnimation();
	void SetHitBox(XMFLOAT3 newStart, XMFLOAT3 newEnd, float rad);
	void SetHitBox(AABB& _aabb, XMMATRIX* _newPosition);
	void UpdateHitBox(XMMATRIX* new_pos);
	void AttachHitBox(float mult, float heightEnd, float heightStart, int type);
	void AttachHitBox(int type);
	void LoadAnimations(std::string _filePath, float _shine);
	void TriInit();
	void InvertWinding();

	// Getters
	XMMATRIX GetWorldMatrix() {	return XMLoadFloat4x4(&m_worldToShader.worldMatrix);}
	Animation* GetCurrAnimation() { return m_currAnimation; }
	Animation* GetNextAnimation() { return m_nextAnimation; }
	std::vector<Animation*> GetAnimations() { return m_animations; }
	std::vector<Transform> GetBindBones() { return m_bindBones; }
	GRAPHICS GetGraphicsType() { return m_graphicsType;}
	OBJECT_ID GetID() { return m_id; }
	Blender* GetBlender() { return m_currBlender; }
	bool GetAnimBusy() { return m_animBusy; }
	vector<Transform>& GetBone();
	XMMATRIX GetBoneOffset() { return XMLoadFloat4x4(&m_bonesToGPU.positionOffset); }
	unsigned int* GetIndexList() { return m_indexList; }
	bool GetActive() { return m_active; }
	HitBox* GetHitBox() { return m_hBox; }
	VERTEX * GetVerts() { return m_vertecies; }
	int GetVertLength() { return m_numVerts; }
	bool GetInAir() { return m_inAir; }
	float GetMass() { return m_mass; }
	float GetTimerDelta() { return float(m_timer.Delta()); }
	XMFLOAT3 GetForce() { return m_force; }
	Tri* GetGround() { return m_ground; }
	Tri* GetWall() { return m_wall; }
	vector<Tri>* GetTris() { return &m_tris; }
	Quad* GetQuad() { return m_quad; }
	bool GetTarget() { return m_target; }
	unsigned int GetIndex() { return m_index; }

	// Setters
	void SetWorldMatrix(XMMATRIX& _matrix);
	void SetCurrAnimation(Animation* anim) { m_currAnimation = anim; }
	void SetNextAnimation(Animation* anim) { m_nextAnimation = anim; }
	void SetPosition(float _x, float _y, float _z);
	void SetGraphType(GRAPHICS _type) { m_graphicsType = _type; }
	void SetID(OBJECT_ID _id) { m_id = _id; }
	void SetInAir(bool _inAir) { m_inAir = _inAir; }
	float SetMass(float _mass) { m_mass = _mass; }
	void SetBlendDur(Blender* _blender) { m_currBlender = _blender; }
	void SetForce(XMFLOAT3 _force) { m_force = _force; }
	void SetAnimBusy(bool _animStatus) { m_animBusy = _animStatus; }
	void SetGround(Tri* _grd) { m_ground = _grd; }
	void SetWall(Tri* _wall) { m_wall = _wall; }
	void SetActive(bool _active) { m_active = _active; }
	void SetQuadTri(float _x, float _y, float _z);
	void ResetTris();
	void SetTargeted(bool _target) { m_target = _target; }
	void SetIndex(unsigned int _ind) { m_index = _ind; }

	// Alligning by 16 bytes so we don't get a warning 
	//void* operator new(size_t i){	return _mm_malloc(i,16);}
	//void operator delete(void* p){_mm_free(p);}

	//BaseObject& operator=(const BaseObject& _obj);

};
#endif