#include "Sound/Sound_Manager.h"

#include "GameInstance.h"
#include "Profiler.h"

#include <chrono>

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

HRESULT CSound_Manager::Play_Music(const wstring_t& strSoundFilePath, f32_t fVolume)
{
	Stop_Music();

	FMOD::Sound* pSound = Find_Or_LoadSound(strSoundFilePath, true);
	if (nullptr == pSound)
		return E_FAIL;

	FMOD_RESULT eResult = m_pSystem->playSound(pSound, nullptr, false, &m_pMusicChannel);
	if (FMOD_OK != eResult || nullptr == m_pMusicChannel)
	{
		Write_FMOD_Error("System::playSound (music)", eResult);
		m_pMusicChannel = nullptr;
		return E_FAIL;
	}

	eResult = m_pMusicChannel->setVolume(fVolume);
	if (FMOD_OK != eResult)
	{
		Write_FMOD_Error("Channel::setVolume (music)", eResult);
		return E_FAIL;
	}

	return S_OK;
}

void CSound_Manager::Stop_Music()
{
	if (nullptr == m_pMusicChannel)
		return;

	m_pMusicChannel->stop();
	m_pMusicChannel = nullptr;
}

void CSound_Manager::Update()
{
	if (nullptr == m_pSystem)
		return;

	const FMOD_RESULT eResult = m_pSystem->update();
	if (FMOD_OK != eResult)
		Write_FMOD_Error("System::update", eResult);
}

FMOD::Sound* CSound_Manager::Find_Or_LoadSound(const wstring_t& strSoundFilePath, bool_t bLoop)
{
	const auto SoundIter = m_Sounds.find(strSoundFilePath);
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
	const auto loadBegin = std::chrono::steady_clock::now();
	const FMOD_RESULT eResult = m_pSystem->createSound(
		strUtf8Path.c_str(), eMode, nullptr, &pSound);
	const uint64_t loadMicroseconds = static_cast<uint64_t>(
		std::chrono::duration_cast<std::chrono::microseconds>(
			std::chrono::steady_clock::now() - loadBegin).count());
	if (CProfiler* pProfiler = CGameInstance::Get().Get_Profiler())
	{
		pProfiler->Add_Counter(EProfilerCounter::AudioFirstLoads, 1u);
		pProfiler->Add_Counter(
			EProfilerCounter::AudioFirstLoadMicroseconds,
			loadMicroseconds);
	}
	if (FMOD_OK != eResult || nullptr == pSound)
	{
		Write_FMOD_Error("System::createSound", eResult);
		return nullptr;
	}

	if (bLoop)
		pSound->setLoopCount(-1);

	m_Sounds.emplace(strSoundFilePath, pSound);
	return pSound;
}

unique_ptr<CSound_Manager> CSound_Manager::Create()
{
	auto pInstance = unique_ptr<CSound_Manager>(new CSound_Manager());
	if (FAILED(pInstance->Initialize()))
		return nullptr;

	return pInstance;
}
