# 2026-08-28 플레이어 스킬 SHAKE notify 카메라 셰이크 PLAN

작성자: JS · branch `feature/player-skill-camera-shake` (main `cd120501` 기준)
선행: `../08-21/2026-08-21_PLAYER_CAMERA_SHAKE_DATA_RESULT.md` (SHAKE 원본 스펙 추출 완료, main 포함 `1343e367`)

## 목표

창술사·차원술사·도화가·워로드 4직업의 스킬 클립에 박힌 `SHAKE` notify 시각에 원작 스펙대로
로컬 플레이어의 gameplay follow 카메라를 흔들고(3축 오프셋) FOV 펀치(줌인/아웃)를 준다.
Client 표현 전용이며 Server·Shared·Data 변경 없음.

## 실측 근거 (2026-08-28)

- **원작 데이터는 LPK에 없고 이미 저장소에 있다.** `SourceData/LPK/.../TableData` 792개 SQLite 중
  shake/oscillation 컬럼을 가진 테이블은 0개. 카메라 관련 테이블(`EFTable_CameraSetting`,
  `IsometricCamera`, `CameraContentsSetting`)은 클래스별 기본 카메라 프리셋(FOV 50, pitch -45,
  ZoomDist 1600cm, DOF)뿐이다. 셰이크 스펙은 `.upk` 클립의 `CEFActionNotify_ViewShake` 인라인
  `CEFCameraViewShake` 블록이며 08-21에 4직업 396건 전량을 `Authored/<C>.animevents`의 SHAKE
  `payload="dur=..;in=..;out=..;x=a,f;y=a,f;z=a,f;fov=a,f"`로 추출해 커밋했다.
  Gunslinger/Slayer(각 122/124행)는 구추출이라 `payload=""` → 이번 범위 밖, 파서는 skip.
- 값 분포(4직업): `dur` 0.05~4.5s, `in` 0~2.0, `out` 0~1.24, 13행은 `in+out > dur`(겹침 허용 필요).
  x/y/z 진폭 0~20(원작 cm), 주파수 0~200. `fov` 진폭 -30~+15(도), 주파수 0~5. 진폭 0이면 그 축 비활성.
  진폭≠0/주파수=0 행은 fov 2건뿐(원작 공식대로 sin(0)=0 → 무효과, 그대로 둠).
- 오실레이터 의미는 UE3 `CameraModifier_CameraShake` 관례: `amp × sin(freq × t)` (freq = rad/s,
  초기 위상 0), 블렌드는 `in`/`out` 구간 선형, `LocOscillation` x/y/z는 **카메라 로컬 forward/right/up**,
  `FOVOscillation`은 FOV 도 단위 가산(음수 = 줌인). 저장소 내 선례: `ValtanCinematicCameraController.cpp`
  `Apply_CueShake`가 84 rad/s sin 오실레이터를 이미 사용.
- 카메라: gameplay 카메라는 Bern/CharacterSelect/Valtan 모두 `CCamera_Free` follow 모드
  (`Late_Update → Update_FollowCamera → Update_PipeLine`, fFovy 60, offset (0.4, 7.5, 4.5) ≈ 8.7m).
  `CCamera::m_fFovy`는 protected라 파생에서 직접 가산 가능(Engine 무변경). `Update_FollowCamera`의
  lerp 분기는 현재 POSITION을 읽으므로 이전 프레임 셰이크 오프셋을 먼저 빼야 스무딩이 오염되지 않는다.
  Bern은 `fFollowResponse = 0` → 매 프레임 direct set. 시네마틱 `Presentation Override` 활성 중엔
  `Update_PipeLine`이 applied pose로 덮어쓰므로 셰이크 적용을 건너뛴다.
- 발화 시점: `CCharacter::Update_SoundCues`(Character.cpp:493)가 stage wall-clock 창
  `(prev, current]`에 든 cue를 loop epoch 포함해 정확히 한 번 발화하는 로직을 이미 가진다. SHAKE도
  동일 로직으로, `Is_LocallyControlled()`인 Character만 발화한다. late snapshot catch-up은
  `fCurrentStageWallSeconds - fOccurrenceWallSeconds`를 초기 elapsed로 넘긴다(Effect cue와 동일 개념).
- 파서: `CAnimationEffectCueDocument::Load_FromText`(AnimationEffectCueDocument.cpp:604)가
  HIT/SOUND/EFFECT 행을 분기하고 그 외 kind는 `continue`. SOUND는 모델에 없는 클립·빈 payload를 skip.
- 하네스: `Tools/ActionPresentationTimelineHarness`는 Engine.lib 없이 Client 순수 cpp를 컴파일한다
  (`ValtanCinematicCameraController.cpp` 등). `AnimationEffectCueDocument.cpp`는 `CEffectCatalog`에
  의존해 하네스에 못 넣으므로 payload 파서와 오실레이터를 새 `CCameraShakeService`에 순수 함수로 두고
  문서 파서는 이를 호출만 한다 → 하네스가 파서·오실레이터·blend·수명 계약을 검증한다.
- 인코딩: `Camera_Free.h/.cpp`는 CP949(ISO-8859 판정, 한글 주석 깨짐) → ASCII 구간만 수정하고 파일 변환
  금지. `Character.cpp` UTF-8, `Character.h`/`AnimationEffectCueDocument.*` ASCII. 새 파일 UTF-8(BOM 없음).

## 계약 (결정)

| 항목 | 값 | 이유 |
|---|---|---|
| 데이터 정본 | 현재 `Authored/<C>.animevents` SHAKE payload 그대로. 포맷 버전·publisher 변경 없음 | 08-21 추출이 이미 정본 |
| 파싱 위치 | `ANIMATION_EFFECT_CUE_DOCUMENT::Shakes` (SOUND와 같은 문서) | 두 번째 animevents 리더 금지 |
| 파싱 정책 | 모델에 없는 클립·빈 payload → 행 skip. payload가 있는데 형식 오류(키 누락/중복/미지 키/비수치/`dur<=0`/음수 freq) → 문서 실패(HIT/EFFECT와 동일 strict) | payload는 추출기 생성물, 손상은 숨기지 않음 |
| 오실레이터 | `amp × sin(freq × t)`, 위상 0. envelope = `min(1, t/in) × min(1, (dur-t)/out)`, in/out 0이면 해당 인자 1. `t >= dur`면 종료 | UE3 관례 + `in+out>dur` 13행 처리 |
| 축 매핑 | payload x→카메라 LOOK, y→RIGHT, z→UP (정규화 basis). 단위 cm → `SHAKE_TRANSLATION_METERS_PER_UNIT = 0.01f` | 원작 LocOscillation 카메라 로컬 |
| FOV | `m_fFovy = base + Σ fovDelta`, `[10, 170]` clamp. base는 `CAMERA_DESC::fFovy` | 도 단위 1:1 |
| 합성 | 활성 셰이크 오프셋·FOV delta 합산, 최대 8개(초과 시 가장 오래된 것 폐기) | 다단 히트 연타 |
| 소유 | 정적 `CCameraShakeService` (Trigger/Sample/Clear). Character → 서비스 → 카메라. Character는 카메라를 모르고 카메라는 Character를 모른다 | MainApp seam 불필요, Engine 무변경 |
| 시계 | `CCamera_Free::Late_Update`가 follow/free 무관하게 프레임당 정확히 한 번 `Sample(dt)` 호출. free 모드·override 중엔 시계만 진행하고 적용 안 함 | 카메라가 표현 소유자 |
| 발화 대상 | `Is_LocallyControlled()` Character만 | 남의 스킬로 내 화면이 흔들리지 않음 |
| 캘리브레이션 | 0.01 배율·FOV 1:1은 초기값. 최종 감은 사용자가 Client에서 육안 대조 후 상수 조정 | 화면 판정은 사용자 전용 |
| 궁극기 `UltimateSkillCameraControl` | 범위 밖(별도 후속) | 미해석 notify |

## G1. SHAKE payload 파서 + 순수 오실레이터 (`CCameraShakeService`, 문서 `Shakes`)

**추가 파일**: `Client/Public/CameraShakeService.h`, `Client/Private/CameraShakeService.cpp`
**수정 파일**: `Client/Public/AnimationEffectCueDocument.h`, `Client/Private/AnimationEffectCueDocument.cpp`,
`Client/Default/Client.vcxproj`, `Client/Default/Client.vcxproj.filters`

### 5-1. `Client/Public/CameraShakeService.h` (새 파일, UTF-8 BOM 없음)

- `CAMERA_SHAKE_OSCILLATOR`: 한 축의 (진폭, 주파수). 진폭은 원본 단위(cm 또는 도) 그대로 보관, 주파수 rad/s.
- `CAMERA_SHAKE_SPEC`: payload 한 행 = 지속·blend in/out·forward/right/up/fov 오실레이터. 파서 출력이자 Trigger 입력.
- `CAMERA_SHAKE_SAMPLE`: 한 프레임 합산 결과(카메라 로컬 forward/right/up 원본 단위, FOV delta 도).
- `CCameraShakeService`: `Parse_PayloadSpec`/`Evaluate`는 순수(하네스 대상), `Trigger`/`Sample`/`Clear`는
  process-global `s_Instances` 상태. `INSTANCE::fElapsedSeconds`는 `Sample`만 증가시키고 `Trigger`가
  catch-up 초기값을 준다.

```cpp
#pragma once

#include "Client_Defines.h"

#include <string>
#include <string_view>
#include <vector>

NS_BEGIN(Client)

/* amplitude * sin(frequency * t). Frequency is radians per second; amplitude
   keeps the authored source unit (cm for translation, degrees for FOV). */
struct CAMERA_SHAKE_OSCILLATOR final
{
	f32_t fAmplitude = 0.f;
	f32_t fFrequency = 0.f;
};

/* One SHAKE payload: "dur=..;in=..;out=..;x=a,f;y=a,f;z=a,f;fov=a,f".
   x/y/z are camera-local forward/right/up. */
struct CAMERA_SHAKE_SPEC final
{
	f32_t fDurationSeconds = 0.f;
	f32_t fBlendInSeconds = 0.f;
	f32_t fBlendOutSeconds = 0.f;
	CAMERA_SHAKE_OSCILLATOR Forward;
	CAMERA_SHAKE_OSCILLATOR Right;
	CAMERA_SHAKE_OSCILLATOR Up;
	CAMERA_SHAKE_OSCILLATOR Fov;
};

struct CAMERA_SHAKE_SAMPLE final
{
	f32_t fForward = 0.f;
	f32_t fRight = 0.f;
	f32_t fUp = 0.f;
	f32_t fFovDeltaDegrees = 0.f;
};

/* Character presentation triggers; the gameplay follow camera samples once per
   frame and applies. No CGameInstance access, so the harness compiles it alone. */
class CCameraShakeService final
{
public:
	static bool_t Parse_PayloadSpec(
		std::string_view Payload,
		CAMERA_SHAKE_SPEC& OutSpec,
		std::string& strOutStatus);
	static bool_t Evaluate(
		const CAMERA_SHAKE_SPEC& Spec,
		f32_t fElapsedSeconds,
		CAMERA_SHAKE_SAMPLE& OutSample);

	static void Trigger(
		const CAMERA_SHAKE_SPEC& Spec,
		f32_t fInitialElapsedSeconds);
	static bool_t Sample(
		f32_t fTimeDelta,
		CAMERA_SHAKE_SAMPLE& OutSample);
	static void Clear();

private:
	struct INSTANCE final
	{
		CAMERA_SHAKE_SPEC Spec;
		f32_t fElapsedSeconds = 0.f;
	};
	static std::vector<INSTANCE> s_Instances;
};

NS_END
```

### 5-2. `Client/Private/CameraShakeService.cpp` (새 파일, UTF-8 BOM 없음)

- `Parse_PayloadSpec`: `;` 분리 → `key=value` → 7개 키 정확히 한 번씩. `dur > 0`, `in/out >= 0`,
  오실레이터 `a,f`는 유한수·`f >= 0`. 실패 시 `OutSpec` 불변.
- `Evaluate`: `t >= dur`이면 false(종료). envelope 곱 → 축별 sin.
- `Trigger`: 초기 elapsed가 이미 dur 이상이면 등록하지 않음. 8개 초과 시 가장 오래된 것 제거.
- `Sample`: 모든 인스턴스 elapsed += dt, 종료된 것 제거, 나머지 합산. 활성이 하나도 없으면 false.

```cpp
#include "CameraShakeService.h"

#include <cmath>
#include <cstdlib>

namespace
{
	constexpr size_t MAX_ACTIVE_SHAKES = 8u;

	bool Parse_Number(const std::string_view Text, f32_t& Out)
	{
		if (Text.empty())
			return false;
		const std::string Owned(Text);
		char* pEnd = nullptr;
		Out = std::strtof(Owned.c_str(), &pEnd);
		return pEnd == Owned.c_str() + Owned.size() && std::isfinite(Out);
	}

	bool Parse_Oscillator(
		const std::string_view Text,
		Client::CAMERA_SHAKE_OSCILLATOR& Out)
	{
		const size_t Comma = Text.find(',');
		if (std::string_view::npos == Comma)
			return false;
		return Parse_Number(Text.substr(0u, Comma), Out.fAmplitude) &&
			Parse_Number(Text.substr(Comma + 1u), Out.fFrequency) &&
			Out.fFrequency >= 0.f;
	}

	f32_t Oscillate(
		const Client::CAMERA_SHAKE_OSCILLATOR& Oscillator,
		const f32_t fElapsedSeconds)
	{
		if (0.f == Oscillator.fAmplitude)
			return 0.f;
		return Oscillator.fAmplitude *
			std::sin(Oscillator.fFrequency * fElapsedSeconds);
	}
}

std::vector<Client::CCameraShakeService::INSTANCE>
	Client::CCameraShakeService::s_Instances;

bool_t Client::CCameraShakeService::Parse_PayloadSpec(
	const std::string_view Payload,
	CAMERA_SHAKE_SPEC& OutSpec,
	std::string& strOutStatus)
{
	CAMERA_SHAKE_SPEC Staged;
	bool bHasDuration = false;
	bool bHasBlendIn = false;
	bool bHasBlendOut = false;
	bool bHasForward = false;
	bool bHasRight = false;
	bool bHasUp = false;
	bool bHasFov = false;
	size_t Cursor = 0u;
	while (Cursor <= Payload.size())
	{
		size_t End = Payload.find(';', Cursor);
		if (std::string_view::npos == End)
			End = Payload.size();
		const std::string_view Field = Payload.substr(Cursor, End - Cursor);
		Cursor = End + 1u;
		const size_t Equal = Field.find('=');
		if (std::string_view::npos == Equal || 0u == Equal)
		{
			strOutStatus = "SHAKE payload field is not key=value: " +
				std::string(Field);
			return false;
		}
		const std::string_view Key = Field.substr(0u, Equal);
		const std::string_view Value = Field.substr(Equal + 1u);
		bool bDuplicate = false;
		bool bParsed = false;
		if ("dur" == Key)
		{
			bDuplicate = bHasDuration;
			bHasDuration = true;
			bParsed = Parse_Number(Value, Staged.fDurationSeconds) &&
				Staged.fDurationSeconds > 0.f;
		}
		else if ("in" == Key)
		{
			bDuplicate = bHasBlendIn;
			bHasBlendIn = true;
			bParsed = Parse_Number(Value, Staged.fBlendInSeconds) &&
				Staged.fBlendInSeconds >= 0.f;
		}
		else if ("out" == Key)
		{
			bDuplicate = bHasBlendOut;
			bHasBlendOut = true;
			bParsed = Parse_Number(Value, Staged.fBlendOutSeconds) &&
				Staged.fBlendOutSeconds >= 0.f;
		}
		else if ("x" == Key)
		{
			bDuplicate = bHasForward;
			bHasForward = true;
			bParsed = Parse_Oscillator(Value, Staged.Forward);
		}
		else if ("y" == Key)
		{
			bDuplicate = bHasRight;
			bHasRight = true;
			bParsed = Parse_Oscillator(Value, Staged.Right);
		}
		else if ("z" == Key)
		{
			bDuplicate = bHasUp;
			bHasUp = true;
			bParsed = Parse_Oscillator(Value, Staged.Up);
		}
		else if ("fov" == Key)
		{
			bDuplicate = bHasFov;
			bHasFov = true;
			bParsed = Parse_Oscillator(Value, Staged.Fov);
		}
		else
		{
			strOutStatus = "SHAKE payload has an unknown key: " +
				std::string(Key);
			return false;
		}
		if (bDuplicate)
		{
			strOutStatus = "SHAKE payload repeats key: " + std::string(Key);
			return false;
		}
		if (!bParsed)
		{
			strOutStatus = "SHAKE payload value is invalid for key: " +
				std::string(Key);
			return false;
		}
	}
	if (!bHasDuration || !bHasBlendIn || !bHasBlendOut || !bHasForward ||
		!bHasRight || !bHasUp || !bHasFov)
	{
		strOutStatus = "SHAKE payload is missing a required key.";
		return false;
	}
	OutSpec = Staged;
	return true;
}

bool_t Client::CCameraShakeService::Evaluate(
	const CAMERA_SHAKE_SPEC& Spec,
	const f32_t fElapsedSeconds,
	CAMERA_SHAKE_SAMPLE& OutSample)
{
	OutSample = {};
	if (!std::isfinite(fElapsedSeconds) || fElapsedSeconds < 0.f ||
		Spec.fDurationSeconds <= 0.f ||
		fElapsedSeconds >= Spec.fDurationSeconds)
	{
		return false;
	}
	f32_t fEnvelope = 1.f;
	if (Spec.fBlendInSeconds > 0.f && fElapsedSeconds < Spec.fBlendInSeconds)
		fEnvelope *= fElapsedSeconds / Spec.fBlendInSeconds;
	const f32_t fRemainingSeconds = Spec.fDurationSeconds - fElapsedSeconds;
	if (Spec.fBlendOutSeconds > 0.f && fRemainingSeconds < Spec.fBlendOutSeconds)
		fEnvelope *= fRemainingSeconds / Spec.fBlendOutSeconds;
	OutSample.fForward = Oscillate(Spec.Forward, fElapsedSeconds) * fEnvelope;
	OutSample.fRight = Oscillate(Spec.Right, fElapsedSeconds) * fEnvelope;
	OutSample.fUp = Oscillate(Spec.Up, fElapsedSeconds) * fEnvelope;
	OutSample.fFovDeltaDegrees = Oscillate(Spec.Fov, fElapsedSeconds) * fEnvelope;
	return true;
}

void Client::CCameraShakeService::Trigger(
	const CAMERA_SHAKE_SPEC& Spec,
	const f32_t fInitialElapsedSeconds)
{
	if (!std::isfinite(fInitialElapsedSeconds) || fInitialElapsedSeconds < 0.f ||
		Spec.fDurationSeconds <= 0.f ||
		fInitialElapsedSeconds >= Spec.fDurationSeconds)
	{
		return;
	}
	if (s_Instances.empty())
		s_Instances.reserve(MAX_ACTIVE_SHAKES);
	if (s_Instances.size() >= MAX_ACTIVE_SHAKES)
		s_Instances.erase(s_Instances.begin());
	s_Instances.push_back({ Spec, fInitialElapsedSeconds });
}

bool_t Client::CCameraShakeService::Sample(
	const f32_t fTimeDelta,
	CAMERA_SHAKE_SAMPLE& OutSample)
{
	OutSample = {};
	const f32_t fStep =
		std::isfinite(fTimeDelta) && fTimeDelta > 0.f ? fTimeDelta : 0.f;
	bool_t bActive = false;
	for (size_t i = 0u; i < s_Instances.size();)
	{
		INSTANCE& Instance = s_Instances[i];
		Instance.fElapsedSeconds += fStep;
		CAMERA_SHAKE_SAMPLE Part;
		if (!Evaluate(Instance.Spec, Instance.fElapsedSeconds, Part))
		{
			s_Instances.erase(s_Instances.begin() + i);
			continue;
		}
		OutSample.fForward += Part.fForward;
		OutSample.fRight += Part.fRight;
		OutSample.fUp += Part.fUp;
		OutSample.fFovDeltaDegrees += Part.fFovDeltaDegrees;
		bActive = true;
		++i;
	}
	return bActive;
}

void Client::CCameraShakeService::Clear()
{
	s_Instances.clear();
}
```

### 5-3. `Client/Public/AnimationEffectCueDocument.h`

변경 종류: 추가
적용 위치 1: `#include "HitAreaWire.h"` 바로 아래

```cpp
#include "CameraShakeService.h"
```

적용 위치 2: `struct ANIMATION_PROJECTILE_CUE final { ... };` 닫는 `};` 바로 아래,
`struct ANIMATION_EFFECT_CUE_DOCUMENT final` 바로 위

```cpp
/* A "SHAKE" .animevents row whose payload carries the source view-shake spec.
   Rows with an empty payload (legacy extraction) are skipped at parse time. */
struct ANIMATION_CAMERA_SHAKE_CUE final
{
    std::string strClipName;
    uint32_t iStartMs = 0u;
    CAMERA_SHAKE_SPEC Spec;
};
```

적용 위치 3: `ANIMATION_EFFECT_CUE_DOCUMENT` 안 `std::vector<ANIMATION_SOUND_CUE> Sounds;` 바로 아래

```cpp
    std::vector<ANIMATION_CAMERA_SHAKE_CUE> Shakes;
```

### 5-4. `Client/Private/AnimationEffectCueDocument.cpp`

변경 종류: 함수 내부 블록 추가
적용 위치: `Load_FromText`(7인자 private 오버로드) 안, SOUND 분기 `if ("SOUND" == Tokens[1]) { ... continue; }`의
닫는 `}` 바로 아래, `if ("EFFECT" != Tokens[1]) continue;` 바로 위

```cpp
        if ("SHAKE" == Tokens[1])
        {
            /* Presentation only, same leniency as SOUND for clips outside this
            model and for legacy rows without a spec. A present spec must parse:
            it is extractor output, so corruption fails the document like HIT. */
            if (!Is_AvailableClip(Tokens[0]))
                continue;
            bool ShakeFieldsValid = false;
            const auto ShakeFields = Make_Fields(Tokens, 2u, ShakeFieldsValid);
            if (!ShakeFieldsValid)
            {
                strOutStatus = "Animation SHAKE row has an invalid or duplicate field.";
                return false;
            }
            const auto PayloadField = ShakeFields.find("payload");
            if (ShakeFields.end() == PayloadField ||
                PayloadField->second.empty())
            {
                continue;
            }
            ANIMATION_CAMERA_SHAKE_CUE Shake;
            Shake.strClipName = Tokens[0];
            const auto StartField = ShakeFields.find("startms");
            if (ShakeFields.end() == StartField ||
                !Parse_UInt(StartField->second, Shake.iStartMs))
            {
                strOutStatus = "Animation SHAKE row has an invalid startms.";
                return false;
            }
            std::string SpecStatus;
            if (!CCameraShakeService::Parse_PayloadSpec(
                PayloadField->second, Shake.Spec, SpecStatus))
            {
                strOutStatus = "Animation SHAKE payload is invalid: " + SpecStatus;
                return false;
            }
            Staged.Shakes.push_back(std::move(Shake));
            continue;
        }
```

`Load`/`Load_ForProductPrewarm`/`Staged` 이동은 그대로 `Shakes`를 함께 옮기므로 추가 변경 없음.

### 5-5. `Client/Default/Client.vcxproj`

적용 위치 1: `<ClInclude Include="..\Public\ActionPresentationTimeline.h" />` 바로 아래

```xml
	<ClInclude Include="..\Public\CameraShakeService.h" />
```

적용 위치 2: `<ClCompile Include="..\Private\ActionPresentationTimeline.cpp" />` 바로 아래

```xml
	<ClCompile Include="..\Private\CameraShakeService.cpp" />
```

### 5-6. `Client/Default/Client.vcxproj.filters` (기존 필터 `02.GameObjects\06. Camera` 사용, 새 Filter 불필요)

적용 위치 1: `<ClCompile Include="..\private\Camera_Free.cpp">` 항목 바로 아래

```xml
    <ClCompile Include="..\Private\CameraShakeService.cpp">
      <Filter>02.GameObjects\06. Camera</Filter>
    </ClCompile>
```

적용 위치 2: `<ClInclude Include="..\public\Camera_Free.h">` 항목 바로 아래

```xml
    <ClInclude Include="..\Public\CameraShakeService.h">
      <Filter>02.GameObjects\06. Camera</Filter>
    </ClInclude>
```

**G1 종료 증거**: Client Debug 빌드 성공. `Character Effect cue load isolated` 로그가 4직업에서 새로 뜨지
않음(SHAKE 396행 전부 파싱). G4 하네스 PASS.

## G2. 카메라 적용 (`CCamera_Free`)

**수정 파일**: `Client/Public/Camera_Free.h`, `Client/Private/Camera_Free.cpp` — 둘 다 CP949, ASCII 구간만 편집.

### 5-7. `Client/Public/Camera_Free.h`

적용 위치 1: private 함수 선언 `void Update_FreeCamera(f32_t fTimeDelta);` 바로 아래

```cpp
	void Remove_AppliedCameraShake();
	void Apply_CameraShake(f32_t fTimeDelta);
```

적용 위치 2: private 멤버 `bool_t m_allowCapturedKeyboardInput = false;` 바로 아래

```cpp
	float3_t			m_vAppliedShakeOffset = {};
	f32_t				m_fBaseFovy = 60.f;
```

- `m_vAppliedShakeOffset`: 직전 프레임에 POSITION에 더한 월드 오프셋. 다음 프레임 follow 계산 전에 반드시
  빼고 0으로 초기화한다. 카메라가 위치를 direct set하는 경로(초기화 분기, `Frame_Area`)에서도 0으로 되돌린다.
- `m_fBaseFovy`: `CAMERA_DESC::fFovy` 원본. `m_fFovy = m_fBaseFovy + delta`로 매 프레임 재유도.

### 5-8. `Client/Private/Camera_Free.cpp`

적용 위치 1: `#include "Transform.h"` 바로 아래

```cpp
#include "CameraShakeService.h"
```

적용 위치 2: 익명 namespace가 없으므로 include 블록 아래, 생성자 정의 바로 위

```cpp
namespace
{
	constexpr f32_t SHAKE_TRANSLATION_METERS_PER_UNIT = 0.01f;
	constexpr f32_t MIN_SHAKE_FOVY = 10.f;
	constexpr f32_t MAX_SHAKE_FOVY = 170.f;
}
```

적용 위치 3: `Initialize` 안 `if (FAILED(__super::Initialize(pArg))) return E_FAIL;` 바로 아래, `return S_OK;` 위

```cpp
	m_fBaseFovy = m_fFovy;
	m_vAppliedShakeOffset = {};
	CCameraShakeService::Clear();
```

적용 위치 4: `Late_Update` 함수 전체 교체

```cpp
void CCamera_Free::Late_Update(f32_t fTimeDelta)
{
	Remove_AppliedCameraShake();
	if (!m_bFollowEnabled)
	{
		CAMERA_SHAKE_SAMPLE Unused;
		CCameraShakeService::Sample(fTimeDelta, Unused);
		return;
	}

	Update_FollowCamera(fTimeDelta);
	Apply_CameraShake(fTimeDelta);
	if (m_bFollowEnabled)
		__super::Update_PipeLine();
}
```

적용 위치 5: `Frame_Area` 안 `m_bFollowInitialized = false;` 바로 아래

```cpp
	m_vAppliedShakeOffset = {};
```

적용 위치 6: `Update_FollowCamera` 안 direct-set 분기 `m_bFollowInitialized = true;` 바로 위

```cpp
		m_vAppliedShakeOffset = {};
```

적용 위치 7: `Update_FreeCamera` 정의 바로 뒤, `Create` 바로 앞에 두 함수 정의 추가

```cpp
void CCamera_Free::Remove_AppliedCameraShake()
{
	if (0.f != m_vAppliedShakeOffset.x ||
		0.f != m_vAppliedShakeOffset.y ||
		0.f != m_vAppliedShakeOffset.z)
	{
		m_pTransformCom->Set_State(
			STATE::POSITION,
			XMVectorSetW(
				m_pTransformCom->Get_State(STATE::POSITION) -
					XMLoadFloat3(&m_vAppliedShakeOffset),
				1.f));
		m_vAppliedShakeOffset = {};
	}
	m_fFovy = m_fBaseFovy;
}

void CCamera_Free::Apply_CameraShake(f32_t fTimeDelta)
{
	CAMERA_SHAKE_SAMPLE Sample;
	const bool_t bActive = CCameraShakeService::Sample(fTimeDelta, Sample);
	if (!bActive || !m_bFollowEnabled || Is_PresentationOverrideActive())
		return;

	const vector_t vLook =
		XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK));
	const vector_t vRight =
		XMVector3Normalize(m_pTransformCom->Get_State(STATE::RIGHT));
	const vector_t vUp =
		XMVector3Normalize(m_pTransformCom->Get_State(STATE::UP));
	const vector_t vOffset =
		(vLook * Sample.fForward + vRight * Sample.fRight + vUp * Sample.fUp) *
		SHAKE_TRANSLATION_METERS_PER_UNIT;
	XMStoreFloat3(&m_vAppliedShakeOffset, vOffset);
	m_pTransformCom->Set_State(
		STATE::POSITION,
		XMVectorSetW(
			m_pTransformCom->Get_State(STATE::POSITION) + vOffset,
			1.f));
	m_fFovy = std::clamp(
		m_fBaseFovy + Sample.fFovDeltaDegrees,
		MIN_SHAKE_FOVY, MAX_SHAKE_FOVY);
}
```

흐름: `Late_Update` → 이전 오프셋 제거·FOV 원복 → (free면 시계만 진행) → follow 계산(LookAt 포함) →
서비스 Sample → 오프셋을 카메라 basis로 월드 변환해 POSITION에 가산, FOV 가산 → `Update_PipeLine`.
LookAt 이후에 위치만 옮기므로 시선 방향은 유지된다(원작 LocOscillation = 위치만).
override 활성 중엔 `Update_PipeLine`이 applied pose로 덮으므로 적용을 건너뛰고 오프셋은 0으로 남는다.

**G2 종료 증거**: 빌드 성공. F6 free 모드 전환·복귀, Valtan 시네마틱 중·후에 카메라 튐 없음(사용자 확인).

## G3. Character 발화

**수정 파일**: `Client/Public/Character.h`, `Client/Private/Character.cpp`

### 5-9. `Client/Public/Character.h`

적용 위치 1: private 멤버 `f32_t m_fPreviousSoundCueStageWallSeconds = -1.f;` 바로 아래

```cpp
	f32_t m_fPreviousShakeCueStageWallSeconds = -1.f;
```

적용 위치 2: private 함수 선언 `void Update_SoundCues();` 바로 아래

```cpp
	void Update_CameraShakeCues();
```

### 5-10. `Client/Private/Character.cpp`

적용 위치 1: `#include "AnimationSkillBindingDocument.h"` 바로 아래

```cpp
#include "CameraShakeService.h"
```

적용 위치 2: `Reset_EffectCueCursor` 안 `m_fPreviousSoundCueStageWallSeconds = -1.f;` 바로 아래

```cpp
	m_fPreviousShakeCueStageWallSeconds = -1.f;
```

적용 위치 3: `Update` 안 `Update_SoundCues();` 바로 아래

```cpp
	Update_CameraShakeCues();
```

적용 위치 4: `Update_SoundCues` 정의 닫는 `}` 바로 뒤, `Commit_PendingClipChains` 정의 바로 앞

```cpp
void CCharacter::Update_CameraShakeCues()
{
	if (!m_isLocallyControlled || m_EffectCueDocument.Shakes.empty())
		return;
	if (nullptr == m_pBodyModel || nullptr == m_pChain ||
		0u == m_iEffectActionStartTick || m_iChainStage < 0 ||
		m_iChainStage >= static_cast<int32_t>(m_pChain->stages.size()))
		return;
	std::vector<ACTION_PRESENTATION_CLIP_TIMING> Timings;
	if (!Build_ActiveStageTimeline(Timings))
		return;
	const std::vector<CLIP_STEP>& Clips =
		m_pChain->stages[m_iChainStage].clips;
	const f32_t fCurrentStageWallSeconds = (std::max)(
		0.f, m_fActionPresentationSeconds);
	const f32_t fPreviousStageWallSeconds =
		m_fPreviousShakeCueStageWallSeconds;

	for (std::size_t iCue = 0u;
		iCue < m_EffectCueDocument.Shakes.size(); ++iCue)
	{
		const ANIMATION_CAMERA_SHAKE_CUE& Cue = m_EffectCueDocument.Shakes[iCue];
		for (std::size_t iClip = 0u; iClip < Clips.size(); ++iClip)
		{
			if (Cue.strClipName != Clips[iClip].clip)
				continue;
			f32_t fSourceDurationSeconds = 0.f;
			f32_t fWallDurationSeconds = 0.f;
			if (!CActionPresentationTimeline::Resolve_ClipDuration(
				Timings[iClip], fSourceDurationSeconds, fWallDurationSeconds))
			{
				continue;
			}
			const f32_t fCueSourceSeconds =
				static_cast<f32_t>(Cue.iStartMs) * 0.001f;
			f32_t fFirstOccurrenceWallSeconds = 0.f;
			if (!CActionPresentationTimeline::Resolve_CueWallOffset(
				Timings, iClip, fCueSourceSeconds, 0u,
				fFirstOccurrenceWallSeconds))
			{
				continue;
			}

			uint64_t iFirstEpoch = 0u;
			uint64_t iLastEpoch = 0u;
			if (Timings[iClip].bLoop)
			{
				if (fCurrentStageWallSeconds <
					fFirstOccurrenceWallSeconds)
				{
					continue;
				}
				if (fPreviousStageWallSeconds >=
					fFirstOccurrenceWallSeconds)
				{
					iFirstEpoch = static_cast<uint64_t>(std::floor(
						(fPreviousStageWallSeconds -
							fFirstOccurrenceWallSeconds) /
						fWallDurationSeconds)) + 1u;
				}
				iLastEpoch = static_cast<uint64_t>(std::floor((std::max)(
					0.f, fCurrentStageWallSeconds -
						fFirstOccurrenceWallSeconds) /
					fWallDurationSeconds));
				if (iFirstEpoch > iLastEpoch)
					continue;
				if (iLastEpoch - iFirstEpoch + 1u >
					MAX_EFFECT_CUE_OCCURRENCES_PER_UPDATE)
				{
					iFirstEpoch = iLastEpoch -
						MAX_EFFECT_CUE_OCCURRENCES_PER_UPDATE + 1u;
				}
			}

			for (uint64_t iEpoch = iFirstEpoch;
				iEpoch <= iLastEpoch; ++iEpoch)
			{
				f32_t fOccurrenceWallSeconds = 0.f;
				if (!CActionPresentationTimeline::Resolve_CueWallOffset(
					Timings, iClip, fCueSourceSeconds, iEpoch,
					fOccurrenceWallSeconds) ||
					fOccurrenceWallSeconds <=
						fPreviousStageWallSeconds ||
					fOccurrenceWallSeconds > fCurrentStageWallSeconds)
				{
					continue;
				}
				/* The shake runs on the wall clock like the source notify; a
				late snapshot joins it mid-way instead of restarting it. */
				CCameraShakeService::Trigger(
					Cue.Spec,
					(std::max)(0.f,
						fCurrentStageWallSeconds - fOccurrenceWallSeconds));

				if (iEpoch == (std::numeric_limits<uint64_t>::max)())
					break;
			}
		}
	}
	m_fPreviousShakeCueStageWallSeconds = fCurrentStageWallSeconds;
}
```

`Update_SoundCues`와 창 계산이 같으므로 이후 SOUND/SHAKE 공통 헬퍼로 추출할 여지가 있으나 이번 G는
기존 함수를 건드리지 않는다.

**G3 종료 증거**: 빌드 성공. Character Select(창술사)에서 LMB 평타·Q/W 스킬 시 화면이 흔들리고 `flm_sk_dragonupfly`
(`fov=-25`)류에서 줌인 펀치가 보임(사용자 육안). 원격 캐릭터의 스킬에는 반응 없음.

## G4. 하네스 (`ActionPresentationTimelineHarness`)

**수정 파일**: `Tools/ActionPresentationTimelineHarness/Default/ActionPresentationTimelineHarness.vcxproj`,
`Tools/ActionPresentationTimelineHarness/Private/ActionPresentationTimelineHarness.cpp`

### 5-11. vcxproj

적용 위치: `<ClCompile Include="..\..\..\Client\Private\ActionPresentationTimeline.cpp">...</ClCompile>` 바로 아래

```xml
    <ClCompile Include="..\..\..\Client\Private\CameraShakeService.cpp">
      <Link>Client\CameraShakeService.cpp</Link>
    </ClCompile>
```

### 5-12. ActionPresentationTimelineHarness.cpp

적용 위치 1: `#include "ActionPresentationTimeline.h"` 바로 아래

```cpp
#include "CameraShakeService.h"
```

적용 위치 2: 익명 namespace 안 `VerifyAdjacentExplicitSourceWindows()` 정의 바로 앞

```cpp
	bool VerifyCameraShakeSpec()
	{
		using Client::CAMERA_SHAKE_SAMPLE;
		using Client::CAMERA_SHAKE_SPEC;
		using Client::CCameraShakeService;

		CAMERA_SHAKE_SPEC Spec;
		std::string Status;
		if (!Require(CCameraShakeService::Parse_PayloadSpec(
				"dur=0.5;in=0;out=0.2;x=5,60;y=5,60;z=8,70;fov=-20,1",
				Spec, Status),
			"authored SHAKE payload did not parse") ||
			!Require(NearlyEqual(Spec.fDurationSeconds, 0.5f) &&
				NearlyEqual(Spec.fBlendInSeconds, 0.f) &&
				NearlyEqual(Spec.fBlendOutSeconds, 0.2f) &&
				NearlyEqual(Spec.Forward.fAmplitude, 5.f) &&
				NearlyEqual(Spec.Forward.fFrequency, 60.f) &&
				NearlyEqual(Spec.Up.fAmplitude, 8.f) &&
				NearlyEqual(Spec.Up.fFrequency, 70.f) &&
				NearlyEqual(Spec.Fov.fAmplitude, -20.f) &&
				NearlyEqual(Spec.Fov.fFrequency, 1.f),
			"SHAKE payload fields changed"))
		{
			return false;
		}

		const CAMERA_SHAKE_SPEC Untouched = Spec;
		const char* const InvalidPayloads[] = {
			"",
			"dur=0.5;in=0;out=0.2;x=5,60;y=5,60;z=8,70",
			"dur=0.5;in=0;out=0.2;x=5,60;y=5,60;z=8,70;fov=-20,1;fov=0,0",
			"dur=0;in=0;out=0.2;x=5,60;y=5,60;z=8,70;fov=-20,1",
			"dur=0.5;in=0;out=0.2;x=5;y=5,60;z=8,70;fov=-20,1",
			"dur=0.5;in=0;out=0.2;x=5,-60;y=5,60;z=8,70;fov=-20,1",
			"dur=0.5;in=0;out=0.2;x=5,60;y=5,60;z=8,70;fov=-20,1;rot=1,1",
			"dur=abc;in=0;out=0.2;x=5,60;y=5,60;z=8,70;fov=-20,1",
		};
		for (const char* pPayload : InvalidPayloads)
		{
			CAMERA_SHAKE_SPEC Parsed = Untouched;
			if (!Require(!CCameraShakeService::Parse_PayloadSpec(
					pPayload, Parsed, Status),
				"invalid SHAKE payload was accepted") ||
				!Require(NearlyEqual(Parsed.fDurationSeconds,
					Untouched.fDurationSeconds) &&
					NearlyEqual(Parsed.Fov.fAmplitude,
						Untouched.Fov.fAmplitude),
				"failed SHAKE parse modified the output spec"))
			{
				return false;
			}
		}

		CAMERA_SHAKE_SAMPLE Sample;
		const float fQuarterForward = 3.14159265f / (2.f * 60.f);
		if (!Require(CCameraShakeService::Evaluate(Spec, 0.f, Sample) &&
				NearlyEqual(Sample.fForward, 0.f) &&
				NearlyEqual(Sample.fFovDeltaDegrees, 0.f),
			"shake at t=0 is not at zero phase") ||
			!Require(CCameraShakeService::Evaluate(
				Spec, fQuarterForward, Sample) &&
				std::fabs(Sample.fForward - 5.f) < 0.001f &&
				std::fabs(Sample.fRight - 5.f) < 0.001f,
			"forward/right quarter-period amplitude changed") ||
			!Require(CCameraShakeService::Evaluate(Spec, 0.4f, Sample) &&
				std::fabs(Sample.fForward -
					5.f * std::sin(60.f * 0.4f) * 0.5f) < 0.001f &&
				std::fabs(Sample.fFovDeltaDegrees -
					-20.f * std::sin(0.4f) * 0.5f) < 0.001f,
			"blend-out envelope at half fade changed") ||
			!Require(!CCameraShakeService::Evaluate(Spec, 0.5f, Sample) &&
				NearlyEqual(Sample.fForward, 0.f),
			"expired shake still produced an offset") ||
			!Require(!CCameraShakeService::Evaluate(Spec, -0.1f, Sample),
			"negative elapsed time was evaluated"))
		{
			return false;
		}

		CAMERA_SHAKE_SPEC Overlapping;
		if (!Require(CCameraShakeService::Parse_PayloadSpec(
				"dur=0.3;in=0.3;out=0.3;x=0,0;y=0,0;z=0,0;fov=10,0.5",
				Overlapping, Status),
			"overlapping blend payload did not parse") ||
			!Require(CCameraShakeService::Evaluate(
				Overlapping, 0.15f, Sample) &&
				std::fabs(Sample.fFovDeltaDegrees -
					10.f * std::sin(0.5f * 0.15f) * 0.5f * 0.5f) < 0.001f,
			"in+out>dur envelope product changed"))
		{
			return false;
		}

		CCameraShakeService::Clear();
		CAMERA_SHAKE_SAMPLE Summed;
		if (!Require(!CCameraShakeService::Sample(0.016f, Summed),
			"empty shake service reported activity"))
		{
			return false;
		}
		CCameraShakeService::Trigger(Spec, 0.f);
		CCameraShakeService::Trigger(Spec, fQuarterForward);
		CCameraShakeService::Trigger(Spec, 0.5f);
		if (!Require(CCameraShakeService::Sample(0.f, Summed) &&
				std::fabs(Summed.fForward - 5.f) < 0.001f,
			"two active shakes did not sum (and the expired one was queued)") ||
			!Require(CCameraShakeService::Sample(0.5f, Summed) == false &&
				NearlyEqual(Summed.fForward, 0.f),
			"advancing past the duration did not retire every shake"))
		{
			return false;
		}
		CCameraShakeService::Trigger(Spec, 0.f);
		CCameraShakeService::Clear();
		if (!Require(!CCameraShakeService::Sample(0.f, Summed),
			"Clear left an active shake"))
		{
			return false;
		}
		return true;
	}
```

적용 위치 3: `main()` 첫 조건 `if (!VerifyAdjacentExplicitSourceWindows() ||` 바로 아래에 한 줄 추가

```cpp
		!VerifyCameraShakeSpec() ||
```

**G4 종료 증거**: `Tools\ActionPresentationTimelineHarness\Bin\Debug\ActionPresentationTimelineHarness.exe` 출력
`ActionPresentationTimelineHarness: PASS`.

## 적용 순서와 검증

1. G1 새 파일 2개 + 문서 파서 + vcxproj/filters → G2 카메라 → G3 Character → G4 하네스.
2. 빌드 (Engine 무변경이라 UpdateLib 불필요):

```powershell
$env:NoDefaultCurrentDirectoryInExePath=''
$env:PATH = "$env:SystemRoot\system32;$env:SystemRoot;C:\Program Files\Blender Foundation\Blender 5.0\5.0\python\bin;" + $env:PATH
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug
```

   정본 스크립트가 `ActionPresentationTimelineHarness`를 빌드·실행한다(`Invoke-BuildAndRegression.ps1:129, 241`).
3. 실행(사용자): `Framework.slnLaunch` Server + Client → Lobby → Character Select(창술사) →
   LMB 평타 (`flm_att_battle_1_0x`, x/y/z 2~3cm 60rad/s, 0.15~0.3s) → Q/W 등 ACTIVE 스킬 →
   `flm_sk_dragonupfly` 바인딩 스킬에서 FOV -25 줌인 펀치. F6로 free 전환 중엔 흔들림 없음, 복귀 시 튐 없음.
   원격 플레이어(두 번째 Client) 스킬에는 반응하지 않음.
4. 성공 시: `Character Effect cue load isolated` 로그 미출력, 셰이크·줌 육안 확인(사용자 서면 판정),
   하네스 PASS, `git diff --check` clean.
5. 실패 입력: `LanceMaster.animevents`의 SHAKE 행 하나를 `payload="dur=0;..."`로 바꾸면 Character가
   `Character Effect cue load isolated: Animation SHAKE payload is invalid: ...`를 남기고 이전(빈) 문서를 유지해
   EFFECT/HIT까지 격리된다(strict 정책 확인 후 원복). `payload=""`로 바꾸면 그 행만 skip.

## 캘리브레이션 (사용자 판정 후)

- 진폭 배율 `SHAKE_TRANSLATION_METERS_PER_UNIT`(0.01) — 원작 카메라 거리 16m 대비 우리 8.7m라 체감이 더 클 수 있음.
- 초기 위상 0 고정(원작은 축별 random offset 옵션 존재) — 동일 스킬이 매번 같은 방향으로 시작.
- 궁극기 `CEFActionNotify_UltimateSkillCameraControl`은 미해석·범위 밖.
