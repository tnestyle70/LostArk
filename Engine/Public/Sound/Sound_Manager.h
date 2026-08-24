#pragma once

#include "Engine_Defines.h"

namespace FMOD
{
	class System;
	class Sound;
	class Channel;
}

NS_BEGIN(Engine)

class CSound_Manager final
{
private:
	CSound_Manager();

public:
	~CSound_Manager();

public:
	HRESULT Play_Sound(const wstring_t& strSoundFilePath, f32_t fVolume);

	/* Looping background music -- unlike Play_Sound (fire-and-forget one-shot SFX), only one
	track plays at a time and the caller can stop it later (e.g. Lobby BGM ending when the level
	transitions away). Starting a new track stops whatever was already playing first. */
	HRESULT Play_Music(const wstring_t& strSoundFilePath, f32_t fVolume);
	void Stop_Music();

	void Update();

private:
	HRESULT Initialize();
	FMOD::Sound* Find_Or_LoadSound(const wstring_t& strSoundFilePath, bool_t bLoop);

private:
	FMOD::System* m_pSystem = { nullptr };
	map<wstring_t, FMOD::Sound*> m_Sounds;
	FMOD::Channel* m_pMusicChannel = { nullptr };

public:
	static unique_ptr<CSound_Manager> Create();
};

NS_END
