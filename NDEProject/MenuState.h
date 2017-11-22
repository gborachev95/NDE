#pragma once
#include "BaseState.h"
#include <vector>
#include <DirectXMath.h>

class Button;
class Camera;
class Light;
class Particle;
class BaseObject;

class MenuState : public BaseState
{
	Camera*                  m_mainCamera;
	std::vector<Button*>     m_buttons;
	std::vector<Button*>     m_optionsButtons;
	std::vector<Light*>      m_sceneLight;
	DirectX::XMFLOAT3        m_buttonPositions[4];
	std::vector<Particle*>   m_particleSys;
	std::vector<BaseObject*> m_envObj;
	BaseObject*              m_cursor;
	BaseObject*              m_creditsQuad;
	bool                     m_options;
	bool                     m_credits;
	bool                     m_buttonClick;
private:
	void LoadButtons();
	void Options();
	void Credits();
	void UpdateCursor();
public:
	MenuState();
	~MenuState();

    void Input();
    void Update();
    void Render() const;
    void Enter();
    void Exit();
    void Init();

};

