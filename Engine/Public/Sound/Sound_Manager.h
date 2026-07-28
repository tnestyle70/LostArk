#pragma once

#include "Engine_Defines.h"

namespace FMOD
{
	class System;
	class Sound;
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
	void Update();

private:
	HRESULT Initialize();
	FMOD::Sound* Find_Or_LoadSound(const wstring_t& strSoundFilePath);

private:
	FMOD::System* m_pSystem = { nullptr };
	map<wstring_t, FMOD::Sound*> m_Sounds;

public:
	static unique_ptr<CSound_Manager> Create();
};

NS_END
