#include "Sound/Sound_Manager.h"
#include <cmath>

#pragma push_macro("new")
#undef new
#include <fmod.hpp>
#include <fmod_errors.h>
#pragma pop_macro("new")

namespace
{
	void Write_FMOD_Error(const char_t* pOperation, FMOD_RESULT eResult)
	{
#ifdef _DEBUG
		char_t szMessage[512] = {};
		sprintf_s(szMessage, "[LostArk][FMOD] %s failed: %s\n", pOperation, FMOD_ErrorString(eResult));
		OutputDebugStringA(szMessage);
#else
		UNREFERENCED_PARAMETER(pOperation);
		UNREFERENCED_PARAMETER(eResult);
#endif
	}
}

CSound_Manager::CSound_Manager()
{
}

CSound_Manager::~CSound_Manager()
{
	for (const auto& [id, channel] : m_CueChannels) if (channel) channel->stop();
	m_CueChannels.clear();
	Stop_LoopingSound();
	Stop_Music();
	for (auto& SoundPair : m_Sounds)
	{
		if (nullptr != SoundPair.second)
			SoundPair.second->release();
	}
	m_Sounds.clear();

	if (nullptr != m_pSystem)
	{
		m_pSystem->close();
		m_pSystem->release();
		m_pSystem = nullptr;
	}
}

HRESULT CSound_Manager::Initialize()
{
	FMOD_RESULT eResult = FMOD::System_Create(&m_pSystem);
	if (FMOD_OK != eResult || nullptr == m_pSystem)
	{
		Write_FMOD_Error("System_Create", eResult);
		return E_FAIL;
	}

	eResult = m_pSystem->init(128, FMOD_INIT_NORMAL, nullptr);
	if (FMOD_OK != eResult)
	{
		Write_FMOD_Error("System::init", eResult);
		return E_FAIL;
	}

	return S_OK;
}

HRESULT CSound_Manager::Play_Sound(const wstring_t& strSoundFilePath, f32_t fVolume)
{
	FMOD::Sound* pSound = Find_Or_LoadSound(strSoundFilePath, false);
	if (nullptr == pSound)
		return E_FAIL;

	FMOD::Channel* pChannel = nullptr;
	FMOD_RESULT eResult = m_pSystem->playSound(pSound, nullptr, false, &pChannel);
	if (FMOD_OK != eResult || nullptr == pChannel)
	{
		Write_FMOD_Error("System::playSound", eResult);
		return E_FAIL;
	}

	eResult = pChannel->setVolume(fVolume);
	if (FMOD_OK != eResult)
	{
		Write_FMOD_Error("Channel::setVolume", eResult);
		return E_FAIL;
	}

	return S_OK;
}

uint64_t CSound_Manager::Play_SoundCue(const wstring_t& path, f32_t volume, uint32_t ageMs)
{
	if (!std::isfinite(volume) || volume < 0.f || volume > 4.f || !m_pSystem) return 0u;
	auto* sound = Find_Or_LoadSound(path, false);
	if (!sound) return 0u;
	unsigned int length = 0;
	if (sound->getLength(&length, FMOD_TIMEUNIT_MS) != FMOD_OK || ageMs >= length) return 0u;
	FMOD::Channel* channel = nullptr;
	if (m_pSystem->playSound(sound, nullptr, true, &channel) != FMOD_OK || !channel) return 0u;
	if (channel->setVolume(volume) != FMOD_OK || channel->setPosition(ageMs, FMOD_TIMEUNIT_MS) != FMOD_OK ||
		channel->setPaused(false) != FMOD_OK)
	{ channel->stop(); return 0u; }
	const uint64_t handle = m_iNextCueHandle++;
	m_CueChannels.emplace(handle, channel);
	return handle;
}
void CSound_Manager::Pause_SoundCue(uint64_t handle, bool_t paused)
{
	const auto found = m_CueChannels.find(handle);
	if (found != m_CueChannels.end()) found->second->setPaused(paused);
}
void CSound_Manager::Seek_SoundCue(uint64_t handle, uint32_t ageMs)
{
	const auto found = m_CueChannels.find(handle);
	if (found != m_CueChannels.end()) found->second->setPosition(ageMs, FMOD_TIMEUNIT_MS);
}
void CSound_Manager::Stop_SoundCue(uint64_t handle)
{
	const auto found = m_CueChannels.find(handle);
	if (found != m_CueChannels.end()) { found->second->stop(); m_CueChannels.erase(found); }
}

HRESULT CSound_Manager::Play_Music(const wstring_t& strSoundFilePath,
	f32_t fVolume, const bool_t bLoop)
{
	return Play_TrackedSound(strSoundFilePath, fVolume, bLoop, m_MusicChannel);
}

HRESULT CSound_Manager::Play_LoopingSound(const wstring_t& strSoundFilePath,
	f32_t fVolume)
{
	return Play_TrackedSound(strSoundFilePath, fVolume, true, m_LoopingSoundChannel);
}

HRESULT CSound_Manager::Play_TrackedSound(const wstring_t& strSoundFilePath,
	f32_t fVolume, const bool_t bLoop, CTrackedSoundChannel<FMOD::Channel>& channel)
{
	FMOD::Sound* pSound = Find_Or_LoadSound(strSoundFilePath, bLoop);
	if (nullptr == pSound)
		return E_FAIL;

	return channel.Try_Replace([&](FMOD::Channel*& staged)
		{
			FMOD_RESULT result = m_pSystem->playSound(pSound, nullptr, true, &staged);
			if (FMOD_OK != result || nullptr == staged)
			{
				Write_FMOD_Error("System::playSound (tracked)", result);
				return false;
			}
			result = staged->setVolume(fVolume);
			if (FMOD_OK != result)
			{
				Write_FMOD_Error("Channel::setVolume (tracked)", result);
				return false;
			}
			result = staged->setPaused(false);
			if (FMOD_OK != result)
			{
				Write_FMOD_Error("Channel::setPaused (tracked)", result);
				return false;
			}
			return true;
		}) ? S_OK : E_FAIL;
}

void CSound_Manager::Stop_Music()
{
	m_MusicChannel.Stop();
}

void CSound_Manager::Stop_LoopingSound()
{
	m_LoopingSoundChannel.Stop();
}

void CSound_Manager::Update()
{
	if (nullptr == m_pSystem)
		return;

	for (auto it = m_CueChannels.begin(); it != m_CueChannels.end();)
	{
		bool playing = false;
		if (it->second->isPlaying(&playing) != FMOD_OK || !playing) it = m_CueChannels.erase(it);
		else ++it;
	}
	const FMOD_RESULT eResult = m_pSystem->update();
	if (FMOD_OK != eResult)
		Write_FMOD_Error("System::update", eResult);
}

FMOD::Sound* CSound_Manager::Find_Or_LoadSound(const wstring_t& strSoundFilePath, bool_t bLoop)
{
	const pair<wstring_t, bool_t> SoundKey{ strSoundFilePath, bLoop };
	const auto SoundIter = m_Sounds.find(SoundKey);
	if (m_Sounds.end() != SoundIter)
		return SoundIter->second;

	const int32_t iPathLength = WideCharToMultiByte(
		CP_UTF8, 0, strSoundFilePath.c_str(), -1, nullptr, 0, nullptr, nullptr);
	if (0 == iPathLength)
		return nullptr;

	string strUtf8Path(static_cast<size_t>(iPathLength), '\0');
	if (0 == WideCharToMultiByte(
		CP_UTF8, 0, strSoundFilePath.c_str(), -1,
		strUtf8Path.data(), iPathLength, nullptr, nullptr))
		return nullptr;

	FMOD::Sound* pSound = nullptr;
	const FMOD_MODE eMode = bLoop ? (FMOD_LOOP_NORMAL | FMOD_2D) : FMOD_DEFAULT;
	const FMOD_RESULT eResult = m_pSystem->createSound(
		strUtf8Path.c_str(), eMode, nullptr, &pSound);
	if (FMOD_OK != eResult || nullptr == pSound)
	{
		Write_FMOD_Error("System::createSound", eResult);
		return nullptr;
	}

	if (bLoop)
		pSound->setLoopCount(-1);

	m_Sounds.emplace(SoundKey, pSound);
	return pSound;
}

unique_ptr<CSound_Manager> CSound_Manager::Create()
{
	auto pInstance = unique_ptr<CSound_Manager>(new CSound_Manager());
	if (FAILED(pInstance->Initialize()))
		return nullptr;

	return pInstance;
}
