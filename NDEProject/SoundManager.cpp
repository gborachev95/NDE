#include "SoundManager.h"
#include <Dbt.h>
#include "Graphics.h"


// Statics
std::unique_ptr<DirectX::AudioEngine>                      SoundManager::m_audioEngine;
std::unique_ptr<DirectX::SoundEffect>                      SoundManager::m_soundEffect[MAX_SOUND_PLAYER + SOUND_MAX];
std::vector<std::unique_ptr<DirectX::SoundEffectInstance>> SoundManager::m_playerSounds[MAX_SOUND_USERS];
std::vector<std::unique_ptr<DirectX::SoundEffectInstance>> SoundManager::m_sounds;

bool                                                       SoundManager::m_initialized = false;
bool                                                       SoundManager::m_runningSounds[MAX_SOUND_USERS][MAX_SOUND_PLAYER];

SoundManager::SoundManager()
{
}

SoundManager::~SoundManager()
{
}

void SoundManager::Initialize()
{
	if (!m_initialized)
	{
		m_initialized = true;
		// Create DirectXTK for Audio objects
		AUDIO_ENGINE_FLAGS eflags = AudioEngine_Default;
#ifdef _DEBUG
		eflags = eflags | AudioEngine_Debug;
#endif
		m_audioEngine.reset(new AudioEngine(eflags));

		for (unsigned int userIndex = 0; userIndex < MAX_SOUND_USERS; ++userIndex)
			m_playerSounds[userIndex].resize(MAX_SOUND_PLAYER);

		// Setting the sounds
		m_soundEffect[RUNNER_IDLE_SOUND].reset(new SoundEffect(m_audioEngine.get(), L"..\\NDEProject\\Assets\\Audio\\BackgroundAmbiance.wav"));
		m_soundEffect[RUNNER_DEATH_SOUND].reset(new SoundEffect(m_audioEngine.get(), L"..\\NDEProject\\Assets\\Audio\\Player\\2.SFX_Death.wav"));
		m_soundEffect[MONSTER_IDLE_SOUND].reset(new SoundEffect(m_audioEngine.get(), L"..\\NDEProject\\Assets\\Audio\\Player\\11.SFX_Demon-Idle.wav"));
		m_soundEffect[MONSTER_CRAW_SOUND].reset(new SoundEffect(m_audioEngine.get(), L"..\\NDEProject\\Assets\\Audio\\Player\\11.SFX_Demon-Idle.wav"));
		m_soundEffect[MONSTER_POUNCE_SOUND].reset(new SoundEffect(m_audioEngine.get(), L"..\\NDEProject\\Assets\\Audio\\Player\\4.SFX_Pounce.wav"));
		m_soundEffect[PUSHED_SOUND].reset(new SoundEffect(m_audioEngine.get(), L"..\\NDEProject\\Assets\\Audio\\Player\\3.SFX_Push.wav"));
		m_soundEffect[FOOTSTEP_SOUND].reset(new SoundEffect(m_audioEngine.get(), L"..\\NDEProject\\Assets\\Audio\\Player\\Walk.wav"));
		m_soundEffect[JUMP_SOUND].reset(new SoundEffect(m_audioEngine.get(), L"..\\NDEProject\\Assets\\Audio\\Player\\Swoosh.wav"));
		m_soundEffect[SWOSH_SOUND].reset(new SoundEffect(m_audioEngine.get(), L"..\\NDEProject\\Assets\\Audio\\Player\\Swoosh.wav"));
		m_soundEffect[KEY_PICKUP_SOUND].reset(new SoundEffect(m_audioEngine.get(), L"..\\NDEProject\\Assets\\Audio\\Player\\6.SFX_KeyPickup.wav"));

		m_soundEffect[MENU_BACKGROUND_SOUND + MAX_SOUND_PLAYER].reset(new SoundEffect(m_audioEngine.get(), L"..\\NDEProject\\Assets\\Audio\\01_Music_Menu_Loop.wav"));
		m_soundEffect[GAME_BACKGROUND_SOUND + MAX_SOUND_PLAYER].reset(new SoundEffect(m_audioEngine.get(), L"..\\NDEProject\\Assets\\Audio\\01_Music_Main_Loop.wav"));
		m_soundEffect[START_SCREECH_SOUND + MAX_SOUND_PLAYER].reset(new SoundEffect(m_audioEngine.get(), L"..\\NDEProject\\Assets\\Audio\\Scary-Titus_Calen-1449371204.wav"));
		m_soundEffect[START_GATE_MONSTER_SOUND + MAX_SOUND_PLAYER].reset(new SoundEffect(m_audioEngine.get(), L"..\\NDEProject\\Assets\\Audio\\5.SFX_Start-Gates-Monster.wav"));
		m_soundEffect[START_GATE_RUNNER_SOUND + MAX_SOUND_PLAYER].reset(new SoundEffect(m_audioEngine.get(), L"..\\NDEProject\\Assets\\Audio\\10.SFX_Falling-Rocks.wav"));
		m_soundEffect[COUNTDOWN_SOUND + MAX_SOUND_PLAYER].reset(new SoundEffect(m_audioEngine.get(), L"..\\NDEProject\\Assets\\Audio\\7.SFX_Count-Down.wav"));
		m_soundEffect[BUTTON_CLICK_SOUND + MAX_SOUND_PLAYER].reset(new SoundEffect(m_audioEngine.get(), L"..\\NDEProject\\Assets\\Audio\\BUTTON_CLICK.wav"));
		m_soundEffect[LOSE_SOUND + MAX_SOUND_PLAYER].reset(new SoundEffect(m_audioEngine.get(), L"..\\NDEProject\\Assets\\Audio\\8.SFX_Lose.wav"));
		m_soundEffect[WIN_SOUND + MAX_SOUND_PLAYER].reset(new SoundEffect(m_audioEngine.get(), L"..\\NDEProject\\Assets\\Audio\\9.SFX_Win-03.wav"));
	

		for (unsigned int userIndex = 0; userIndex < MAX_SOUND_USERS; ++userIndex)
		{
			for (unsigned int soundIndex = 0; soundIndex < MAX_SOUND_PLAYER; ++soundIndex)
			{
				m_playerSounds[userIndex][soundIndex] = m_soundEffect[soundIndex]->CreateInstance();
				//m_playerSounds[userIndex][soundIndex]->SetVolume(0.2f);
			}
		}
		m_sounds.resize(SOUND_MAX);
		for (unsigned int soundIndex = 0; soundIndex < SOUND_MAX; ++soundIndex)
			m_sounds[soundIndex] = m_soundEffect[soundIndex + MAX_SOUND_PLAYER]->CreateInstance();

		m_sounds[MENU_BACKGROUND_SOUND]->Play(true);
		m_sounds[GAME_BACKGROUND_SOUND]->Play(true);
		m_sounds[GAME_BACKGROUND_SOUND]->Pause();;
	}
}

void SoundManager::Update()
{
	for (unsigned int i = 0; i < MAX_SOUND_USERS; ++i)
		PlayPlayerSounds(i);

	m_audioEngine->Update();
}

void SoundManager::PlayPlayerSounds(unsigned int _player)
{
	for (unsigned int i = 0; i < MAX_SOUND_PLAYER; ++i)
	{
		if (m_runningSounds[_player][i])
		{
			m_runningSounds[_player][i] = false;
			if (m_playerSounds[_player][i]->GetState() != DirectX::SoundState::PLAYING)
				m_playerSounds[_player][i]->Play();
		
		}			
	}
}

void SoundManager::PlaySoundEffct(unsigned int _sound, bool _loop)
{
	if (_sound < MAX_SOUND_PLAYER && _sound >= 0)
	{
		if (m_sounds[_sound]->GetState() != DirectX::SoundState::PLAYING)
			m_sounds[_sound]->Play(_loop);
	}
}

void SoundManager::SetSoundVolume(unsigned int _sound, float _volume)
{
	m_sounds[_sound]->SetVolume(_volume);
}

void SoundManager::SetPlayerSoundVolume(unsigned int _player, unsigned int _sound, float _volume)
{
	m_playerSounds[_player][_sound]->SetVolume(_volume);
}

void SoundManager::SetRunningSound(unsigned int _player, unsigned int _sound, bool _state)
{ 
	m_runningSounds[_player][_sound] = _state;
}

void SoundManager::Mute()
{
	for (unsigned int i = 0; i < m_sounds.size(); ++i)
		SetSoundVolume(i,0.0f);

	for (unsigned int j = 0; j < MAX_SOUND_USERS; ++j)
		for (unsigned int i = 0; i < m_playerSounds[j].size(); ++i)
			SetPlayerSoundVolume(j, i, 0.0f);
}