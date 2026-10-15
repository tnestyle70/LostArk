// System/Sound/group 소유와 cue handle·음악/반복 채널 제어를 구현한다.
#include "Sound/Sound_Manager.h"
// volume과 배속의 NaN/무한대 입력을 거절한다.
#include <cmath>

// FMOD System·Sound·Channel·ChannelGroup 생성/재생 API를 사용한다.
#include <fmod.hpp>
// Debug 진단에 FMOD_RESULT의 오류 설명을 붙인다.
#include <fmod_errors.h>

namespace
{
	// Debug에서만 backend 오류를 debugger에 남긴다. Release에서는 인수를 소비 표시하고 별도 메시지를 출력하지 않는다.
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

// 멤버 기본값으로 시작한다. 실제 backend 준비와 실패 처리는 Create→Initialize가 담당한다.
CSound_Manager::CSound_Manager()
{
}

// 추적 cue와 음악/반복 재생을 멈춘 뒤 cache Sound, 하위 group, System 순으로 해제한다.
// cache 잠금만으로 소멸을 보호하지 않으므로 소유자는 다른 호출이 끝난 뒤 manager를 파괴해야 한다.
CSound_Manager::~CSound_Manager()
{
	for (const auto& [id, channel] : m_CueChannels)
		if (channel)
			channel->stop();
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

// GameInstance용 새 manager의 backend를 준비한다. 중간 실패는 HRESULT로 알리고 Create가 부분 자원을 폐기한다.
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

// MASTER 아래 음악/효과음/UI 하위 group을 연결한다. 이후 옵션 volume은 이미 재생 중인 같은 group 채널에도 적용된다.
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

// group 포인터를 차용해 반환한다. MASTER는 System에서 얻고 하위 group은 manager가 보관한 값을 사용한다.
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
	case SOUND_CATEGORY::MUSIC:
		return m_pMusicGroup;
	case SOUND_CATEGORY::EFFECT:
		return m_pEffectGroup;
	case SOUND_CATEGORY::INTERFACE:
		return m_pInterfaceGroup;
	default:
		return nullptr;
	}
}

// 일반/cue/반복 효과음의 group을 경로 부분 문자열로 선택한다. 경로 정규화나 대소문자 변환은 여기서 수행하지 않는다.
FMOD::ChannelGroup* CSound_Manager::Pick_OneShotGroup(const wstring_t& strSoundFilePath) const
{
	// 호출자가 해석한 절대 경로에도 사용할 수 있도록 두 경로 구분자의 Sound/UI 부분을 찾는다.
	// prefix가 아닌 부분 문자열 검사이며 대소문자는 그대로 비교한다.
	if (wstring_t::npos != strSoundFilePath.find(L"Sound\\UI\\") ||
		wstring_t::npos != strSoundFilePath.find(L"Sound/UI/"))
	{
		return m_pInterfaceGroup;
	}
	return m_pEffectGroup;
}

//두 재생 진입점이 기존 focus/cache/channel/volume 처리를 공유한다.
//재생 후 volume 설정 실패는 기존처럼 오류만 반환하고 시작된 채널은 중지하지 않는다.
HRESULT CSound_Manager::Play_OneShotSound(const wstring_t& strSoundFilePath,
    f32_t fVolume, FMOD::ChannelGroup* pGroup)
{
    if (!Update_ApplicationFocusMute())
    {
        return E_FAIL;
    }

    FMOD::Sound* pSound = Find_Or_LoadSound(strSoundFilePath, false);
    if (nullptr == pSound)
    {
        return E_FAIL;
    }

    FMOD::Channel* pChannel = nullptr;
    FMOD_RESULT eResult = m_pSystem->playSound(pSound, pGroup, false, &pChannel);
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

// 옵션의 원하는 정책을 먼저 저장하고 즉시 적용을 시도한다. backend 실패 시 다음 Update가 다시 평가한다.
void CSound_Manager::Set_MuteOnFocusLoss(const bool_t bMute)
{
	if (m_bMuteOnFocusLoss == bMute)
		return;
	m_bMuteOnFocusLoss = bMute;
	// 포커스가 그대로여도 옵션 변경을 반영하도록 적용 완료 캐시를 무효화한다.
	m_bFocusMuteInitialized = false;
	(void)Update_ApplicationFocusMute();
}

// 유한 volume을 0~1로 제한한 뒤 backend 성공 시 저장값을 갱신한다. focus mute flag는 건드리지 않는다.
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

// 마지막 성공 저장값을 반환한다. END만 별도 기본값으로 처리하므로 임의 정수로 만든 enum은 전달하지 않는다.
f32_t CSound_Manager::Get_CategoryVolume(const SOUND_CATEGORY eCategory) const
{
	if (SOUND_CATEGORY::END == eCategory)
		return 1.f;
	return m_CategoryVolumes[ETOUI(eCategory)];
}

// 같은 process의 전경 창인지 매번 확인하되 원하는 mute가 바뀔 때만 backend에 적용한다.
// 실패하면 적용 완료 캐시를 갱신하지 않아 이후 호출에서 재시도한다.
bool_t CSound_Manager::Update_ApplicationFocusMute()
{
	if (!m_pSystem)
		return false;

	// 같은 process가 만든 분리된 도구 창도 이 앱의 전경 창으로 본다.
	// 다른 Client process의 창은 별개이며 mute-on-loss가 켜졌을 때 이 process의 출력을 음소거한다.
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
	// MASTER mute만 바꾼다. 사용자 volume, cue 위치와 명시적으로 요청한 pause 상태는 그대로 둔다.
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

// 추적 handle이 필요 없는 1회 효과음을 바로 시작한다. 재생 후 volume 설정 실패는 오류만 반환하고 시작된 채널은 멈추지 않는다.
HRESULT CSound_Manager::Play_Sound(const wstring_t& strSoundFilePath, f32_t fVolume)
{
    return Play_OneShotSound(strSoundFilePath, fVolume, Pick_OneShotGroup(strSoundFilePath));
}
//명시한 category만 선택하고, 동일한 재생 본문으로 전달한다.
HRESULT CSound_Manager::Play_Sound(const wstring_t& strSoundFilePath, f32_t fVolume, SOUND_CATEGORY eCategory)
{
	//이게 왜 필요한 거지?  default일 때 E_INVALIDAG; 이거 때문에? 
    //END나 정수 변환으로 들어온 미지원 값은 E_INVALIDARG로 구분한다.
    //지원하는 category인데 group을 얻지 못한 경우는 아래에서 E_FAIL이다.
	switch (eCategory)
	{
		case SOUND_CATEGORY::MASTER:
		case SOUND_CATEGORY::MUSIC:
        case SOUND_CATEGORY::EFFECT:
        case SOUND_CATEGORY::INTERFACE:
            break;
        default:
            return E_INVALIDARG;
	}

	FMOD::ChannelGroup* pGroup = Find_CategoryGroup(eCategory);

	if (nullptr == pGroup)
	{
        return E_FAIL;
	}

    return Play_OneShotSound(strSoundFilePath, fVolume, pGroup);
}

//자원 cache만 준비한다. 채널 생성이나 재생은 하지 않는다.
HRESULT CSound_Manager::Prepare_Sound(const wstring_t& strSoundFilePath, bool_t bLoop)
{
	if (strSoundFilePath.empty())
	{
        return E_INVALIDARG;
	}

	if (nullptr == m_pSystem)
	{
        return E_FAIL;
	}

    //HRESULT의 실패 값을 bool로 바꾸면 1이 되어 SUCCEEDED에서 성공으로 판정된다.
    //준비 결과의 성공/실패 코드를 HRESULT 그대로 반환한다.
    return nullptr != Find_Or_LoadSound(strSoundFilePath, bLoop) ? S_OK : E_FAIL;
}

// 독립 cue를 paused 상태로 준비하고 위치/volume/배속/최종 pause가 모두 적용되면 handle을 등록한다.
// 위치·끝은 ms, 배속은 무차원 비율이다. 설정 실패는 새 채널을 stop하며 map 할당 예외의 별도 rollback은 없다.
uint64_t CSound_Manager::Play_SoundCue(
	const wstring_t& path, f32_t volume, uint32_t ageMs,
	bool_t paused, f32_t playbackRate, uint32_t endMs)
{
	if (!std::isfinite(playbackRate) || playbackRate <= 0.f || playbackRate > 16.f ||
		!std::isfinite(volume) || volume < 0.f || volume > 4.f || !m_pSystem ||
		!Update_ApplicationFocusMute())
		return 0u;
	auto* sound = Find_Or_LoadSound(path, false);
	if (!sound)
		return 0u;
	unsigned int length = 0;
	if (sound->getLength(&length, FMOD_TIMEUNIT_MS) != FMOD_OK || ageMs >= length || (endMs && ageMs >= endMs))
		return 0u;
	FMOD::Channel* channel = nullptr;
	if (m_pSystem->playSound(sound, Pick_OneShotGroup(path), true, &channel) != FMOD_OK || !channel)
		return 0u;
	if (channel->setVolume(volume) != FMOD_OK || channel->setPosition(ageMs, FMOD_TIMEUNIT_MS) != FMOD_OK ||
		channel->setPitch(playbackRate) != FMOD_OK ||
		channel->setPaused(paused) != FMOD_OK)
	{
		channel->stop();
		return 0u;
	}
	// 준비한 채널을 process-local 증가 handle에 연결한다. 정수 순환/중복 삽입 실패에 대한 별도 처리는 없다.
	const uint64_t handle = m_iNextCueHandle++;
	m_CueChannels.emplace(handle, channel);
	if (endMs)
		m_CueEndPositionsMs.emplace(handle, endMs);
	return handle;
}

// 1회 재생 모드의 Sound를 cache에서 찾거나 로드해 길이를 읽는다. 유효한 양수 길이를 얻은 경우에만 출력을 바꾼다.
bool_t CSound_Manager::Get_SoundDurationMs(const wstring_t& path, uint32_t& durationMs)
{
	if (!m_pSystem)
		return false;
	auto* sound = Find_Or_LoadSound(path, false);
	unsigned int length = 0u;
	if (!sound || sound->getLength(&length, FMOD_TIMEUNIT_MS) != FMOD_OK || !length)
		return false;
	durationMs = length;
	return true;
}

// 미등록 또는 backend isPlaying 실패/false이면 false다. pause 상태를 별도로 조회하는 함수는 아니다.
bool_t CSound_Manager::Is_SoundCueActive(uint64_t handle) const
{
	const auto found = m_CueChannels.find(handle);
	if (found == m_CueChannels.end())
		return false;
	bool playing = false;
	return found->second->isPlaying(&playing) == FMOD_OK && playing;
}

// 등록된 cue에만 pause를 요청한다. backend 결과는 반환하지 않고 map의 추적 상태도 그대로 둔다.
void CSound_Manager::Pause_SoundCue(uint64_t handle, bool_t paused)
{
	const auto found = m_CueChannels.find(handle);
	if (found != m_CueChannels.end())
		found->second->setPaused(paused);
}

// 지원 범위 안의 배율을 setPitch로 전달한다. 잘못된 값/미등록 handle은 무시하고 backend 오류는 별도로 알리지 않는다.
void CSound_Manager::Set_SoundCuePlaybackRate(uint64_t handle, f32_t rate)
{
	if (!std::isfinite(rate) || rate <= 0.f || rate > 16.f)
		return;
	const auto found = m_CueChannels.find(handle);
	if (found != m_CueChannels.end())
		found->second->setPitch(rate);
}

// 구간 끝 이상이면 cue를 종료하고 그 외에는 backend에 ms seek를 요청한다. 실제 파일 길이에 대한 오류는 backend 결과를 따로 전달하지 않는다.
void CSound_Manager::Seek_SoundCue(uint64_t handle, uint32_t ageMs)
{
	const auto found = m_CueChannels.find(handle);
	if (found != m_CueChannels.end())
	{
		const auto End = m_CueEndPositionsMs.find(handle);
		if (End != m_CueEndPositionsMs.end() && ageMs >= End->second)
			Stop_SoundCue(handle);
		else
			found->second->setPosition(ageMs, FMOD_TIMEUNIT_MS);
	}
}

// 외부 시계와 ms 오차를 비교해 허용 범위를 넘을 때만 seek한다.
// 반환은 마지막 재생 유효성 조회다. seek 실패를 따로 확인하거나 최종 위치 오차를 재검사하지 않는다.
bool_t CSound_Manager::Synchronize_SoundCue(uint64_t handle, uint32_t ageMs, uint32_t maxDriftMs)
{
	const auto found = m_CueChannels.find(handle);
	if (found == m_CueChannels.end())
		return false;
	bool playing = false;
	unsigned int positionMs = 0u;
	if (found->second->isPlaying(&playing) != FMOD_OK || !playing ||
		found->second->getPosition(&positionMs, FMOD_TIMEUNIT_MS) != FMOD_OK)
		return false;
	const auto driftMs = positionMs > ageMs ? positionMs - ageMs : ageMs - positionMs;
	if (driftMs > maxDriftMs)
		Seek_SoundCue(handle, ageMs);
	return Is_SoundCueActive(handle);
}

// stop 요청 결과와 무관하게 handle과 선택 구간 끝을 제거한다. 같은 handle로 이후 pause/seek 요청은 아무 일도 하지 않는다.
void CSound_Manager::Stop_SoundCue(uint64_t handle)
{
	const auto found = m_CueChannels.find(handle);
	if (found != m_CueChannels.end())
	{
		found->second->stop();
		m_CueChannels.erase(found);
	}
	m_CueEndPositionsMs.erase(handle);
}

// 음악 group과 전용 추적 슬롯으로 공통 교체 경로를 호출한다. loop 여부는 Sound cache key에 포함한다.
HRESULT CSound_Manager::Play_Music(const wstring_t& strSoundFilePath,
	f32_t fVolume, const bool_t bLoop)
{
	return Play_TrackedSound(strSoundFilePath, fVolume, bLoop, m_MusicChannel, m_pMusicGroup);
}

// 음악과 별도의 효과음 슬롯을 무한 반복 모드로 교체한다. 경로가 UI 아래라면 INTERFACE group을 사용한다.
HRESULT CSound_Manager::Play_LoopingSound(const wstring_t& strSoundFilePath,
	f32_t fVolume)
{
	return Play_TrackedSound(strSoundFilePath, fVolume, true, m_LoopingSoundChannel,
		Pick_OneShotGroup(strSoundFilePath));
}

// 음악/반복 효과음의 새 채널을 먼저 준비한다. callback의 false는 임시 채널만 stop하고 기존 슬롯을 유지한다.
// 새 채널의 pause 해제까지 성공하면 기존 채널을 stop하고 슬롯을 바꾼다. 기존 채널 stop 오류까지 복원하지는 않는다.
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

// 음악 추적 슬롯만 중지한다. cue/반복 효과음은 각각의 소유 슬롯이 관리한다.
void CSound_Manager::Stop_Music()
{
	m_MusicChannel.Stop();
}

// 반복 효과음 슬롯만 중지하고 추적 포인터를 비운다.
void CSound_Manager::Stop_LoopingSound()
{
	m_LoopingSoundChannel.Stop();
}

// CGameInstance의 프레임 갱신이다. 포커스 정책, 선택 구간 끝/종료 cue 정리 후 backend update를 호출한다.
void CSound_Manager::Update()
{
	if (nullptr == m_pSystem)
		return;

	(void)Update_ApplicationFocusMute();

	// endMs 도달은 stop한 뒤 제거한다. 자연 종료/조회 실패는 등록만 제거하며 반복/음악 채널은 이 목록과 별개다.
	for (auto it = m_CueChannels.begin(); it != m_CueChannels.end();)
	{
		bool playing = false;
		unsigned int positionMs = 0u;
		const auto End = m_CueEndPositionsMs.find(it->first);
		const bool_t bRangeEnded = End != m_CueEndPositionsMs.end() &&
			it->second->getPosition(&positionMs, FMOD_TIMEUNIT_MS) == FMOD_OK && positionMs >= End->second;
		if (bRangeEnded || it->second->isPlaying(&playing) != FMOD_OK || !playing)
		{
			if (bRangeEnded)
				it->second->stop();
			m_CueEndPositionsMs.erase(it->first);
			it = m_CueChannels.erase(it);
		}
		else
			++it;
	}
	const FMOD_RESULT eResult = m_pSystem->update();
	if (FMOD_OK != eResult)
		Write_FMOD_Error("System::update", eResult);
}

// cache key는 입력 경로 문자열과 반복 모드다. 반환 Sound는 manager가 release하므로 호출자는 차용만 한다.
// mutex는 map 조회/등록만 보호하며 파일 변환·FMOD 로드는 잠금 밖이다. 중복 생성은 등록 때 하나만 보관한다.
FMOD::Sound* CSound_Manager::Find_Or_LoadSound(const wstring_t& strSoundFilePath, bool_t bLoop)
{
	const pair<wstring_t, bool_t> SoundKey{ strSoundFilePath, bLoop };
	{
		lock_guard<mutex> Lock(m_SoundsMutex);
		const auto SoundIter = m_Sounds.find(SoundKey);
		if (m_Sounds.end() != SoundIter)
			return SoundIter->second;
	}

	// Windows wide 경로를 종료 null까지 포함하는 UTF-8 문자열로 바꿔 FMOD 파일 입력에 전달한다.
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

	// 반복 Sound에 무한 loop count를 요청한다. 이 setLoopCount 반환 오류를 별도로 검사하지 않는 기존 동작을 유지한다.
	if (bLoop)
		pSound->setLoopCount(-1);

	lock_guard<mutex> Lock(m_SoundsMutex);
	// 동시에 같은 key가 먼저 등록됐다면 이번에 만든 Sound만 release하고 기존 cache 값을 반환한다.
	const auto [SoundIter, bInserted] = m_Sounds.emplace(SoundKey, pSound);
	if (!bInserted)
		pSound->release();
	return SoundIter->second;
}

// 부분 초기화 실패도 소멸자가 정리하도록 unique_ptr 안에서 준비하고 성공한 소유권만 GameInstance에 반환한다.
unique_ptr<CSound_Manager> CSound_Manager::Create()
{
	auto pInstance = unique_ptr<CSound_Manager>(new CSound_Manager());
	if (FAILED(pInstance->Initialize()))
		return nullptr;

	return pInstance;
}
