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

	if (nullptr != m_pMusicGroup)
	{
		m_pMusicGroup->release();
		m_pMusicGroup = nullptr;
	}
	if (nullptr != m_pEffectGroup)
	{
		m_pEffectGroup->release();
		m_pEffectGroup = nullptr;
	}
	if (nullptr != m_pInterfaceGroup)
	{
		m_pInterfaceGroup->release();
		m_pInterfaceGroup = nullptr;
	}

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

	if (FAILED(Ready_CategoryGroups()))
		return E_FAIL;

	return Update_ApplicationFocusMute() ? S_OK : E_FAIL;
}

HRESULT CSound_Manager::Ready_CategoryGroups()
{
	FMOD::ChannelGroup* pMaster = nullptr;
	FMOD_RESULT eResult = m_pSystem->getMasterChannelGroup(&pMaster);
	if (FMOD_OK != eResult || nullptr == pMaster)
	{
		Write_FMOD_Error("System::getMasterChannelGroup (categories)", eResult);
		return E_FAIL;
	}

	struct GROUP_DESC final
	{
		const char_t* pName;
		FMOD::ChannelGroup** ppGroup;
	};
	const GROUP_DESC Groups[] =
	{
		{ "LostArk.Music", &m_pMusicGroup },
		{ "LostArk.Effect", &m_pEffectGroup },
		{ "LostArk.Interface", &m_pInterfaceGroup },
	};
	for (const GROUP_DESC& Desc : Groups)
	{
		eResult = m_pSystem->createChannelGroup(Desc.pName, Desc.ppGroup);
		if (FMOD_OK != eResult || nullptr == *Desc.ppGroup)
		{
			Write_FMOD_Error("System::createChannelGroup", eResult);
			return E_FAIL;
		}
		eResult = pMaster->addGroup(*Desc.ppGroup);
		if (FMOD_OK != eResult)
		{
			Write_FMOD_Error("ChannelGroup::addGroup", eResult);
			return E_FAIL;
		}
	}
	return S_OK;
}

FMOD::ChannelGroup* CSound_Manager::Find_CategoryGroup(
	const SOUND_CATEGORY eCategory) const
{
	if (nullptr == m_pSystem)
		return nullptr;
	switch (eCategory)
	{
	case SOUND_CATEGORY::MASTER:
	{
		FMOD::ChannelGroup* pMaster = nullptr;
		if (FMOD_OK != m_pSystem->getMasterChannelGroup(&pMaster))
			return nullptr;
		return pMaster;
	}
	case SOUND_CATEGORY::MUSIC:		return m_pMusicGroup;
	case SOUND_CATEGORY::EFFECT:	return m_pEffectGroup;
	case SOUND_CATEGORY::INTERFACE:	return m_pInterfaceGroup;
	default:						return nullptr;
	}
}

FMOD::ChannelGroup* CSound_Manager::Pick_OneShotGroup(const wstring_t& strSoundFilePath) const
{
	/* The runtime asset tree already sorts interface sounds under Sound/UI/; that folder is
	the bus. Both separators, because callers hand over resolved absolute paths. */
	if (wstring_t::npos != strSoundFilePath.find(L"Sound\\UI\\") ||
		wstring_t::npos != strSoundFilePath.find(L"Sound/UI/"))
	{
		return m_pInterfaceGroup;
	}
	return m_pEffectGroup;
}

void CSound_Manager::Set_MuteOnFocusLoss(const bool_t bMute)
{
	if (m_bMuteOnFocusLoss == bMute)
		return;
	m_bMuteOnFocusLoss = bMute;
	/* Re-evaluate now rather than on the next focus change. */
	m_bFocusMuteInitialized = false;
	(void)Update_ApplicationFocusMute();
}

HRESULT CSound_Manager::Apply_CategoryVolume(
	const SOUND_CATEGORY eCategory, const f32_t fVolume)
{
	if (SOUND_CATEGORY::END == eCategory)
		return E_INVALIDARG;
	if (!std::isfinite(fVolume))
		return E_INVALIDARG;
	FMOD::ChannelGroup* pGroup = Find_CategoryGroup(eCategory);
	if (nullptr == pGroup)
		return E_FAIL;

	const f32_t fClamped = fVolume < 0.f ? 0.f : (fVolume > 1.f ? 1.f : fVolume);
	const FMOD_RESULT eResult = pGroup->setVolume(fClamped);
	if (FMOD_OK != eResult)
	{
		Write_FMOD_Error("ChannelGroup::setVolume (category)", eResult);
		return E_FAIL;
	}
	m_CategoryVolumes[ETOUI(eCategory)] = fClamped;
	return S_OK;
}

f32_t CSound_Manager::Get_CategoryVolume(const SOUND_CATEGORY eCategory) const
{
	if (SOUND_CATEGORY::END == eCategory)
		return 1.f;
	return m_CategoryVolumes[ETOUI(eCategory)];
}

bool_t CSound_Manager::Update_ApplicationFocusMute()
{
	if (!m_pSystem)
		return false;

	// Match Winters: detached editor windows still belong to this application,
	// while another Client process must never share its audible output.
	DWORD foregroundProcessId = 0u;
	if (const HWND foreground = GetForegroundWindow())
		GetWindowThreadProcessId(foreground, &foregroundProcessId);
	const bool_t muted = m_bMuteOnFocusLoss && foregroundProcessId != GetCurrentProcessId();
	if (m_bFocusMuteInitialized && muted == m_bFocusMuted)
		return true;

	FMOD::ChannelGroup* master = nullptr;
	FMOD_RESULT result = m_pSystem->getMasterChannelGroup(&master);
	if (FMOD_OK != result || !master)
	{
		Write_FMOD_Error("System::getMasterChannelGroup (focus)", result);
		return false;
	}
	// Mute output only: keep authored volume, cue time, and explicit pause intact.
	result = master->setMute(muted);
	if (FMOD_OK != result)
	{
		Write_FMOD_Error("ChannelGroup::setMute (focus)", result);
		return false;
	}
	m_bFocusMuted = muted;
	m_bFocusMuteInitialized = true;
	return true;
}

HRESULT CSound_Manager::Play_Sound(const wstring_t& strSoundFilePath, f32_t fVolume)
{
	if (!Update_ApplicationFocusMute())
		return E_FAIL;
	FMOD::Sound* pSound = Find_Or_LoadSound(strSoundFilePath, false);
	if (nullptr == pSound)
		return E_FAIL;

	FMOD::Channel* pChannel = nullptr;
	FMOD_RESULT eResult = m_pSystem->playSound(pSound, Pick_OneShotGroup(strSoundFilePath), false, &pChannel);
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

uint64_t CSound_Manager::Play_SoundCue(const wstring_t& path, f32_t volume, uint32_t ageMs, bool_t paused, f32_t playbackRate)
{
	if (!std::isfinite(playbackRate) || playbackRate <= 0.f || playbackRate > 16.f || !std::isfinite(volume) || volume < 0.f || volume > 4.f || !m_pSystem ||
		!Update_ApplicationFocusMute()) return 0u;
	auto* sound = Find_Or_LoadSound(path, false);
	if (!sound) return 0u;
	unsigned int length = 0;
	if (sound->getLength(&length, FMOD_TIMEUNIT_MS) != FMOD_OK || ageMs >= length) return 0u;
	FMOD::Channel* channel = nullptr;
	if (m_pSystem->playSound(sound, Pick_OneShotGroup(path), true, &channel) != FMOD_OK || !channel) return 0u;
	if (channel->setVolume(volume) != FMOD_OK || channel->setPosition(ageMs, FMOD_TIMEUNIT_MS) != FMOD_OK ||
		channel->setPitch(playbackRate) != FMOD_OK ||
		channel->setPaused(paused) != FMOD_OK)
	{ channel->stop(); return 0u; }
	const uint64_t handle = m_iNextCueHandle++;
	m_CueChannels.emplace(handle, channel);
	return handle;
}
bool_t CSound_Manager::Get_SoundDurationMs(const wstring_t& path, uint32_t& durationMs)
{
	if (!m_pSystem) return false;
	auto* sound = Find_Or_LoadSound(path, false);
	unsigned int length = 0u;
	if (!sound || sound->getLength(&length, FMOD_TIMEUNIT_MS) != FMOD_OK || !length) return false;
	durationMs = length;
	return true;
}
bool_t CSound_Manager::Is_SoundCueActive(uint64_t handle) const
{
	const auto found = m_CueChannels.find(handle);
	if (found == m_CueChannels.end()) return false;
	bool playing = false;
	return found->second->isPlaying(&playing) == FMOD_OK && playing;
}
void CSound_Manager::Pause_SoundCue(uint64_t handle, bool_t paused)
{
	const auto found = m_CueChannels.find(handle);
	if (found != m_CueChannels.end()) found->second->setPaused(paused);
}
void CSound_Manager::Set_SoundCuePlaybackRate(uint64_t handle, f32_t rate)
{
    if (!std::isfinite(rate) || rate <= 0.f || rate > 16.f) return;
    const auto found = m_CueChannels.find(handle);
    if (found != m_CueChannels.end()) found->second->setPitch(rate);
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
	return Play_TrackedSound(strSoundFilePath, fVolume, bLoop, m_MusicChannel, m_pMusicGroup);
}

HRESULT CSound_Manager::Play_LoopingSound(const wstring_t& strSoundFilePath,
	f32_t fVolume)
{
	return Play_TrackedSound(strSoundFilePath, fVolume, true, m_LoopingSoundChannel,
		Pick_OneShotGroup(strSoundFilePath));
}

HRESULT CSound_Manager::Play_TrackedSound(const wstring_t& strSoundFilePath,
	f32_t fVolume, const bool_t bLoop, CTrackedSoundChannel<FMOD::Channel>& channel,
	FMOD::ChannelGroup* pGroup)
{
	if (!Update_ApplicationFocusMute())
		return E_FAIL;
	FMOD::Sound* pSound = Find_Or_LoadSound(strSoundFilePath, bLoop);
	if (nullptr == pSound)
		return E_FAIL;

	return channel.Try_Replace([&](FMOD::Channel*& staged)
		{
			FMOD_RESULT result = m_pSystem->playSound(pSound, pGroup, true, &staged);
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

	(void)Update_ApplicationFocusMute();

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
	{
		lock_guard<mutex> Lock(m_SoundsMutex);
		const auto SoundIter = m_Sounds.find(SoundKey);
		if (m_Sounds.end() != SoundIter)
			return SoundIter->second;
	}

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

	lock_guard<mutex> Lock(m_SoundsMutex);
	const auto [SoundIter, bInserted] = m_Sounds.emplace(SoundKey, pSound);
	if (!bInserted)
		pSound->release();
	return SoundIter->second;
}

unique_ptr<CSound_Manager> CSound_Manager::Create()
{
	auto pInstance = unique_ptr<CSound_Manager>(new CSound_Manager());
	if (FAILED(pInstance->Initialize()))
		return nullptr;

	return pInstance;
}
