#pragma once
#include <vector>
#define MAX_SOUND_USERS 5

#define AUDIO 1
#include "DXAudio\Audio.h"

enum PLAYER_SOUND_ID 
{
	RUNNER_IDLE_SOUND, RUNNER_DEATH_SOUND, 
	MONSTER_IDLE_SOUND, MONSTER_CRAW_SOUND, MONSTER_POUNCE_SOUND,
	PUSHED_SOUND, FOOTSTEP_SOUND, JUMP_SOUND, SWOSH_SOUND, KEY_PICKUP_SOUND,
	MAX_SOUND_PLAYER
};
enum SOUND_ID
{
	MENU_BACKGROUND_SOUND, GAME_BACKGROUND_SOUND, START_SCREECH_SOUND,START_GATE_MONSTER_SOUND,
	START_GATE_RUNNER_SOUND,COUNTDOWN_SOUND, BUTTON_CLICK_SOUND, LOSE_SOUND, WIN_SOUND, SOUND_MAX
};

class SoundManager
{
	static std::unique_ptr<DirectX::AudioEngine>                      m_audioEngine;
	static std::unique_ptr<DirectX::SoundEffect>                      m_soundEffect[MAX_SOUND_PLAYER + SOUND_MAX];
	static std::vector<std::unique_ptr<DirectX::SoundEffectInstance>> m_playerSounds[MAX_SOUND_USERS];
	static std::vector<std::unique_ptr<DirectX::SoundEffectInstance>> m_sounds;
	static bool                                                       m_initialized;
	static bool                                                       m_runningSounds[MAX_SOUND_USERS][MAX_SOUND_PLAYER];

public:
	SoundManager();
	~SoundManager();

	static void Initialize();
	static void Update();
	static void PlayPlayerSounds(unsigned int _player);
	static void PlaySoundEffct(unsigned int _sound, bool _loop);
	static void SetSoundVolume(unsigned int _sound, float _volume);
	static void SoundManager::SetPlayerSoundVolume(unsigned int _player, unsigned int _sound, float _volume);
	// Getters
	static bool IsInitialized() { return m_initialized; }
	static DirectX::AudioEngine* GetAudioEngine() { return m_audioEngine.get(); }
	static DirectX::SoundEffectInstance* GetPlayerSound(unsigned int _soundIndex, unsigned int _userIndex) { return m_playerSounds[_userIndex][_soundIndex].get(); }
	static DirectX::SoundEffectInstance* GetSound(unsigned int _soundIndex) { return m_sounds[_soundIndex].get(); }
	static DirectX::SoundEffect* GetSoundEffect(unsigned int _sound) { return m_soundEffect[_sound].get(); }
	static unsigned int GetPlayersSoundsSize() { return m_playerSounds[0].size(); }
	static void Shutdown() { m_audioEngine.reset(); }
	static void Mute();
	// Setters 
	static void SetRunningSound(unsigned int _player, unsigned int _sound, bool _state);

};