#pragma once

// 공통 엔진 타입·문자열/map·Win32 창/경로 API를 사용한다.
#include "Engine_Defines.h"
// 음악과 반복 효과음에 각각 실패 시 이전 채널을 보존하는 교체 슬롯을 둔다.
#include "Sound/TrackedSoundChannel.h"

// Sound cache의 조회/중복 등록만 보호한다. cue와 volume 등 manager 전체를 잠그는 mutex는 아니다.
#include <mutex>

// 헤더에서는 backend 포인터만 선언한다. 실제 FMOD 구현 헤더는 CPP에서 포함한다.
namespace FMOD
{
	class System;
	class Sound;
	class Channel;
	class ChannelGroup;
}

NS_BEGIN(Engine)

// 옵션 창이 조절하는 재생 group이다. FMOD의 MASTER 아래 MUSIC/EFFECT/INTERFACE를 연결한다.
// Play_Music은 MUSIC을 사용하고, 기존 두 인자 일반/cue/반복 효과음은 경로로 INTERFACE/EFFECT를 고른다.
// 세 인자 Play_Sound는 호출자가 지정한 category의 group을 사용한다.
enum class SOUND_CATEGORY : uint32_t
{
	MASTER,
	MUSIC,
	EFFECT,
	INTERFACE,
	END
};

// CGameInstance가 소유하는 FMOD 재생 경계다. System·로드한 Sound·생성한 하위 group은 release 책임이 있다.
// Channel은 FMOD가 관리하고 manager는 재생 제어용 포인터/handle만 추적한다.
// cache 외의 상태에는 잠금이 없으므로 소유자는 재생 제어·프레임 Update·종료를 직렬화해야 한다.
class CSound_Manager final
{
private:
	CSound_Manager();

public:
	~CSound_Manager();

public:
	// CGameInstance의 추적하지 않는 1회 효과음이다. 재생 시작 뒤 volume 설정이 실패해도 시작된 채널을 stop하지 않는다.
	HRESULT Play_Sound(const wstring_t& strSoundFilePath, f32_t fVolume);

	//Client가 정한 재생 group을 사용한다. 기존 두 인자 호출은 경로 분류를 유지한다.
    HRESULT Play_Sound(const wstring_t& strSoundFilePath, f32_t fVolume, SOUND_CATEGORY eCategory);
	//재생 없이 같은 경로, 반복 여부의 Sound Cache만 준비한다.
    HRESULT Prepare_Sound(const wstring_t& strSoundFilePath, bool_t bLoop = false);

	// 재생 위치/일시정지/배속을 설정한 독립 cue handle을 반환한다. 0은 실패이며 위치와 종료 경계는 ms다.
	// endMs=0은 별도 구간 끝이 없고, 설정 단계 실패는 새 채널을 stop한다. handle 순환/할당 예외는 별도로 복구하지 않는다.
	uint64_t Play_SoundCue(const wstring_t& path, f32_t volume, 
		uint32_t ageMs = 0u, bool_t paused = false, f32_t playbackRate = 1.f, uint32_t endMs = 0u);
	// Sound를 필요하면 cache에 로드해 ms 길이를 읽는다. 실패하면 durationMs를 유지한다.
	bool_t Get_SoundDurationMs(const wstring_t& path, uint32_t& durationMs);
	// 등록된 채널의 isPlaying 결과를 읽는다. 일시정지 여부를 별도로 검사하지 않으며 false만으로 종료 원인을 구분하지 않는다.
	bool_t Is_SoundCueActive(uint64_t handle) const;
	// 등록 cue에 pause를 요청한다. 미등록은 무시하며 backend 반환 오류는 호출자에게 전달하지 않는다.
	void Pause_SoundCue(uint64_t handle, bool_t paused);
    // 유한한 0 초과 16 이하 배율만 setPitch로 요청한다. 채널 시간 동기화는 별도 함수의 책임이다.
    void Set_SoundCuePlaybackRate(uint64_t handle, f32_t playbackRate);
	// 지정한 ms로 seek한다. 보관한 endMs 이상이면 cue를 stop/해제하고 그 외 backend seek 오류는 반환하지 않는다.
	void Seek_SoundCue(uint64_t handle, uint32_t ageMs);
    // 외부 시계의 ms와 현재 위치 오차가 maxDriftMs보다 클 때만 seek한다.
    // false는 재생 종료/조회 실패/등록 해제를 포함한다. true가 seek 성공이나 시계 일치를 보장하지는 않는다.
    bool_t Synchronize_SoundCue(uint64_t handle, uint32_t ageMs, uint32_t maxDriftMs = 100u);
	// 해당 handle의 채널에 stop을 요청하고 재생/구간 종료 map에서 제거한다. 다른 음악/반복 슬롯은 바꾸지 않는다.
	void Stop_SoundCue(uint64_t handle);

	// 음악과 별도 슬롯의 반복 효과음이다. 새 재생 준비에 실패하면 이전 반복 채널을 유지한다.
	HRESULT Play_LoopingSound(const wstring_t& strSoundFilePath, f32_t fVolume);
	void Stop_LoopingSound();

	// 레벨/전투가 교체·중지하는 음악 채널 하나를 추적한다. bLoop=false는 1회, true는 무한 반복 Sound를 사용한다.
	// 새 채널 설정이 성공한 뒤 기존 음악을 중지한다. 반복 효과음 슬롯은 별도로 유지한다.
	HRESULT Play_Music(const wstring_t& strSoundFilePath, f32_t fVolume,
		bool_t bLoop = true);
	void Stop_Music();

	// 유한한 volume을 0~1로 제한해 group에 적용하고 성공 시 저장값을 갱신한다. 기존 재생에도 group volume이 적용된다.
	// 사용자 volume=0과 창 포커스에 따른 MASTER mute는 별도 상태로 관리한다.
	HRESULT Apply_CategoryVolume(SOUND_CATEGORY eCategory, f32_t fVolume);
	// FMOD를 재조회하지 않고 마지막 성공 저장값을 읽는다. END는 1이며 그 외 유효 enum은 호출자 전제다.
	f32_t Get_CategoryVolume(SOUND_CATEGORY eCategory) const;
	// true이면 다른 process가 전경 창을 소유할 때 MASTER를 mute한다. false이면 배경 재생을 허용한다.
	// 설정을 저장하고 즉시 재평가하며 backend 실패는 반환하지 않고 다음 Update에서 다시 시도한다.
	void Set_MuteOnFocusLoss(bool_t bMute);

	// CGameInstance 프레임 갱신에서 focus mute와 끝난 cue를 정리한 뒤 FMOD update를 호출한다.
	void Update();

private:
	// Create가 System·128채널 초기화·하위 group·초기 mute를 준비한다. 실패 객체는 Create의 소멸 경로가 정리한다.
	HRESULT Initialize();
	// MASTER를 차용해 세 하위 group을 생성/연결한다. 중간 실패의 생성 완료 group은 소멸자가 정리한다.
	HRESULT Ready_CategoryGroups();
	// 전경 창의 process ID로 같은 앱의 분리 창도 활성으로 본다. backend mute 적용 성공 때만 캐시 상태를 갱신한다.
	bool_t Update_ApplicationFocusMute();
	// MASTER 또는 생성한 하위 group을 차용한다. 미초기화/지원하지 않는 enum은 nullptr다.
	FMOD::ChannelGroup* Find_CategoryGroup(SOUND_CATEGORY eCategory) const;
	// 경로에 대소문자를 구분하는 Sound/UI/ 또는 Sound\UI\가 있으면 INTERFACE, 아니면 EFFECT다.
	FMOD::ChannelGroup* Pick_OneShotGroup(const wstring_t& strSoundFilePath) const;
	
	//두 공개 one-shot 진입점이 group 선택 뒤 같은 재생 실패 처리를 사용한다.
    HRESULT Play_OneShotSound(const wstring_t& strSoundFilePath, f32_t fVolume, FMOD::ChannelGroup* pGroup);

	// 경로를 UTF-8로 바꿔 Sound를 로드/cache한다. 반환은 cache 소유의 차용 포인터이며 실패는 nullptr다.
	FMOD::Sound* Find_Or_LoadSound(const wstring_t& strSoundFilePath, bool_t bLoop);
	// 음악/반복 슬롯의 공통 준비 경로다. 새 채널을 paused로 만들고 volume/재생 시작이 성공한 뒤 슬롯을 교체한다.
	HRESULT Play_TrackedSound(const wstring_t& strSoundFilePath, f32_t fVolume,
		bool_t bLoop, CTrackedSoundChannel<FMOD::Channel>& channel,
		FMOD::ChannelGroup* pGroup);

private:
	FMOD::System* m_pSystem = { nullptr };
	// Initialize에서 MASTER에 연결한 하위 group이다. 소멸자가 각각 release하며 새 재생에 해당 group을 지정한다.
	FMOD::ChannelGroup* m_pMusicGroup = { nullptr };
	FMOD::ChannelGroup* m_pEffectGroup = { nullptr };
	FMOD::ChannelGroup* m_pInterfaceGroup = { nullptr };
	f32_t m_CategoryVolumes[ETOUI(SOUND_CATEGORY::END)] = { 1.f, 1.f, 1.f, 1.f };
	bool_t m_bMuteOnFocusLoss = true;
	bool_t m_bFocusMuteInitialized = false;
	bool_t m_bFocusMuted = true;
	// loop mode를 설정한 Sound를 (경로, 반복 여부)별로 따로 cache한다.
	// 같은 WAV를 1회 효과음과 반복 음악으로 사용해도 서로의 Sound 설정을 바꾸지 않는다.
	map<pair<wstring_t, bool_t>, FMOD::Sound*> m_Sounds;
	mutex m_SoundsMutex;
	// cue handle은 이 manager 수명 안의 식별자다. backend 채널 메모리는 소유하지 않고 endMs는 별도 map으로 관리한다.
	map<uint64_t, FMOD::Channel*> m_CueChannels;
	map<uint64_t, uint32_t> m_CueEndPositionsMs;
	uint64_t m_iNextCueHandle = 1u;
	CTrackedSoundChannel<FMOD::Channel> m_MusicChannel;
	CTrackedSoundChannel<FMOD::Channel> m_LoopingSoundChannel;

public:
	// GameInstance가 소유할 manager를 만든다. Initialize 실패는 부분 FMOD 자원을 정리하고 nullptr를 반환한다.
	static unique_ptr<CSound_Manager> Create();
};

NS_END
