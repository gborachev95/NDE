#pragma once
#include "BaseUI.h"

enum BUTTON_TYPE {PLAY_BUTTON, START_BUTTON, OPTIONS_BUTTON, CREDITS_BUTTON, EXIT_BUTTON, JOIN_BUTTON, HOST_BUTTON, BACK_BUTTON, QUIT_BUTTON, RESUME_BUTTON, RESIZE_BUTTON};
class Button : public BaseUI
{
	BUTTON_TYPE  m_type;
	bool         m_scaled; 
public:
	// Constructor
	Button();
	Button(float _x, float _y, float _z, BUTTON_TYPE _type);

	// Destructor
	~Button();

	void Input();
	void Update();
	void Render();
	bool OnCollision();
	bool DetectCollision();

	// Setters 
	void SetButtonType(BUTTON_TYPE _type) { m_type = _type; }

	// Getters
	BUTTON_TYPE GetButtonType() { return m_type; }
};

