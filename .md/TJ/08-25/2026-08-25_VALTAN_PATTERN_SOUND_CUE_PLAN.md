# 2026-08-25 발탄 패턴 사운드 큐 PLAN

이 문서는 `.md/GB/local.md`의 "새 파일은 전체 코드, 기존 파일은 전체 반영본" 규칙을 원문 그대로
따르지 않는다. 이번 작업은 사용자가 문서를 보고 손으로 타이핑하는 방식이 아니라 에이전트가 저장소에
직접 반영하는 방식이라, `Valtan.cpp`(1426줄)/`Valtan.h`(376줄) 전체를 미변경 부분까지 그대로 복사해
넣는 건 정보 가치 없이 토큰만 태운다. 대신 기존 파일은 정확한 삽입 기준점 + 새로 추가되는 코드 전문만
싣고, 완전히 새로운 파일만 처음부터 끝까지 싣는다. 최종 반영 상태는 실제 저장소 diff와 RESULT 문서가
정본이다.

## 0. 배경

플레이어 6개 클래스는 `.animevents`의 SOUND row를 `CCharacter::Update_SoundCues()`가 매 프레임
"지금 재생 중인 클립 + 경과 시간"을 스스로 판단해 직접 소비한다 (Client 로컬 애니메이션 상태 기준).

발탄은 다르다. 발탄의 이펙트 큐(`Valtan.patterneffectcues.json`)는 Server가 내려주는
`m_strServerActionId`/`m_iServerActionStartTick`/`m_iServerPatternSequence`를 근거로
`CValtan::Spawn_DuePatternEffectCues()`가 트리거하며, 그 전에 `CValtan::Load_PatternEffectCues()`가
매 큐를 실제 클립 타이밍(`Client::CActionPresentationTimeline::Resolve_ClipDuration`/
`Resolve_CueWallOffset`)으로 미리 검증해둔다. 사운드도 같은 서버 권위 트리거 경로를 타야 하며,
플레이어처럼 매 프레임 로컬 클립을 스캔하는 방식을 새로 만들면 안 된다(같은 역할의 두 번째 트리거
경로 금지).

## 1. 원본 데이터 확인 (완료, 실측)

- `Data/Animation/Reference/Valtan/Valtan.animevents` (read-only 저작 참고, 1173 row, 아직 Authored
  승격 안 됨)에 실제 SOUND row가 **261개** 존재한다. 예:
  ```
  "mesh_att_battle_12_01" SOUND startms=250 payload="S_Mob_G_Voltan1.G_Voltan1_Attack12_Cast1" src=orig
  ```
- payload의 bank는 4종류: `S_Mob_G_Voltan1`, `S_Mob_G_Voltan2`(몬스터 보이스/타격음, 실제 매핑 대상),
  `S_BGM_CommanderRaid`, `S_Systems`(이번 스코프 제외 — 아래 5절 참고).
- 실제 wav는 `D:\로아 리소스\Sound\Boss\Valtan\Voltan1\`, `...\Voltan2\`에 있으며 파일명이
  `g_voltan2_freezeend1__<해시>.wav` 식으로 payload의 eventname과 대소문자만 다르게 정확히 일치한다
  (`Voltan3` 폴더도 있지만 현재 authored 패턴에서 참조되는 row가 없어 이번 스코프 제외).
- `Data/Animation/Authored/Valtan/Valtan.patternbindings.json`의 `bindings[].clips[].clip` 필드가
  `.animevents`의 클립 이름과 **완전히 동일한 문자열**(`"mesh_att_battle_12_01"` 등)이다. 실측:
  ```json
  {
   "actionId": "valtan.attack.swing.windup",
   "clips": [{ "clipOccurrenceId": "valtan.attack.swing.windup.clip.01",
     "clip": "mesh_idle_battle_1", "sourceStartMs": 0, "playMs": 0,
     "playRate": 1.0, "loop": true }]
  }
  ```
- `Data/Encounters/Valtan/ValtanEncounter.json`의 `patterns[].stages[].actionId`가
  `patternbindings.json`의 `bindings[].actionId`와 같은 문자열로 조인된다. 실측:
  ```json
  { "patternId": "VALTAN_SWING", "stages": [
    { "stageId": "WINDUP", "actionId": "valtan.attack.swing.windup", "durationMs": 400, ... }
  ]}
  ```

즉 3-way exact-string join이 전부 실측으로 확인됐고 추측은 없다:

```
Valtan.animevents(clip, startms, payload)
  -> patternbindings.json 의 clips[].clip == clip 인 항목 (actionId, clipOccurrenceId, sourceStartMs 등)
  -> ValtanEncounter.json 의 stages[].actionId == actionId 인 항목 (patternId, stageId, durationMs)
```

## 2. 새 Authored 문서: `Valtan.patternsoundcues.json`

`Valtan.patterneffectcues.json`과 형제 파일, 같은 스키마 철학(바인딩→오컬런스→패턴/스테이지 join
결과를 미리 구워둔 문서)이되 이펙트 전용 필드(`localTransform`, `followPolicy`, `V1EffectAssetId`,
`anchorSlotId` 본 검증)는 없다. 사운드는 공간 부착이 없으므로 앵커가 필요 없다.

```json
{
 "schema": "lostark.valtan-pattern-sound-cues",
 "formatVersion": 1,
 "ownerArchetypeId": "BOSS_VALTAN",
 "cues": [
  {
   "bindingId": "cue.sound.valtan.attack.swing.windup.clip.01",
   "occurrenceId": "cue.sound.valtan.attack.swing.windup.clip.01.occurrence.01",
   "patternId": "VALTAN_SWING",
   "stageId": "WINDUP",
   "actionId": "valtan.attack.swing.windup",
   "clipOccurrenceId": "valtan.attack.swing.windup.clip.01",
   "soundBank": "S_Mob_G_Voltan1",
   "soundEvent": "G_Voltan1_Attack12_Cast1",
   "repeatPolicy": "once",
   "startMs": 250
  }
 ]
}
```

`stageDurationMs`는 JSON에 저장하지 않는다 — `Valtan.patterneffectcues.json`의 실제 스키마도 저장하지
않고 매 로드 시점에 `Encounter.Find_Pattern(patternId)`로 새로 구해서 쓴다(스테이지 duration이 나중에
밸런스 패치로 바뀌어도 사운드 큐 문서를 다시 굽지 않아도 되게, 그리고 저장된 값과 실제 Encounter 값이
어긋나는 stale 데이터 문제를 원천적으로 없애기 위해). `CValtanPatternSoundCueDocument::Parse_Text`도
같은 방식으로 매 로드 시점에 derive한다.

- `repeatPolicy`는 이펙트와 동일한 개념(ONCE/EACH_LOOP)을 재사용한다 — 발탄의 루프성 클립(예:
  groggy loop)에서 SOUND row가 반복 재생을 의도하는 경우가 실제로 있다(`mesh_abn_groggy_1_loop`).
- 한 (actionId, clipOccurrenceId) 쌍에 SOUND row가 여러 개면 `occurrence.02`, `.03`처럼 순번을
  매겨 별도 cue row로 만든다(같은 clip에 같은 startms가 중복될 수 있어 — 실측: `"mesh_abn_groggy_1_end"`
  에 `startms=1`인 SOUND row가 2개 존재 — occurrenceId만으로 유일성을 보장한다).

## 3. 생성 스크립트: `Tools/ValtanPipeline/build_valtan_pattern_sound_cues.py`

`build_sound_catalog.py`처럼 순수 조회/생성 스크립트다. Server/Room 상태를 건드리지 않고 Data만
읽고 쓴다.

처리 순서:
1. `Data/Animation/Reference/Valtan/Valtan.animevents`를 파싱해 SOUND row만 추출
   (`(clipName, startMs, bank, eventName)` 튜플 리스트). `src=orig`만 채택 — 다른 src 값이 있다면
   보고만 하고 스킵(현재 261개 SOUND row는 전부 `src=orig`으로 확인됨).
2. `S_BGM_CommanderRaid`/`S_Systems` bank는 이번 스코프에서 스킵하고 개수만 stderr로 보고한다
   (5절 참고, 몬스터 보이스/타격음이 아니라 별도 트리거 경로가 필요한 데이터라 섞으면 안 됨).
3. `Data/Animation/Authored/Valtan/Valtan.patternbindings.json`을 읽어
   `clip문자열 -> [(actionId, clipOccurrenceId, sourceStartMs, playRate, loop)]` 역인덱스를 만든다
   (한 clip 문자열이 여러 binding에서 재사용될 수 있으므로 리스트).
4. `Data/Encounters/Valtan/ValtanEncounter.json`을 읽어
   `actionId -> (patternId, stageId, durationMs)` 역인덱스를 만든다.
5. SOUND row마다 2번 인덱스로 clip을 조인한다. 매치가 0개면 `unmatchedClip`으로 보고만 하고 계속
   진행한다(실패로 중단하지 않음 — GunSlinger/Slayer 사운드 갭과 같은 이유로, 아직 authored되지 않은
   패턴의 클립일 수 있다). 매치된 각 (actionId, clipOccurrenceId)에 대해 4번 인덱스로 actionId를 다시
   조인한다. 매치 0개면 `unmatchedAction`으로 보고하고 계속 진행한다.
6. 최종 매치마다 2절 스키마의 cue row 하나를 만든다. `occurrenceId`의 순번은 같은
   `(actionId, clipOccurrenceId)` 쌍 안에서 1부터 증가.
7. `Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json`을 들여쓰기 1(space) 유지하며 쓴다.
8. 매치/미매치 개수를 stdout에 요약 출력한다(플레이어 RESULT 문서와 같은 형식의 커버리지 표를 만들
   수 있도록).

## 4. wav 배치 + 카탈로그 확장

`build_sound_catalog.py`를 확장하지 않고 별도 스크립트
`Tools/CharacterAnimationIntake/build_valtan_sound_catalog.py`를 새로 만든다 — 원본 폴더 구조
자체가 다르기 때문이다(`Sound/Character/<Class>/` 대 `Sound/Boss/Valtan/Voltan{1,2}/`).
`build_sound_catalog.py`와 로직 중복이 생기지만, `resolve_source_folder`에 발탄 전용 분기를
끼워 넣는 것보다 원본 폴더 레이아웃이 근본적으로 다른 두 개의 소스를 억지로 한 함수에 넣지 않는
쪽이 더 명확하다.

처리 순서:
1. `Valtan.patternsoundcues.json`의 `soundEvent` 필드를 전부 모은다(중복 제거).
2. `<raw_sound_root>/Sound/Boss/Valtan/Voltan1/`과 `.../Voltan2/`를 각각 스캔해
   `eventname.lower() + "__"` 접두사로 파일을 찾는다(플레이어와 동일한 변형-파일 매칭 규칙).
3. 매치된 파일을 `Client/Bin/Resources/Sound/Character/Valtan/`(평면 폴더, Voltan1/2 구분 없이 —
   이벤트 이름 자체에 이미 `Voltan1`/`Voltan2`가 들어있어 파일명 충돌이 없음, 실측 확인됨)로 복사한다.
4. `Data/Sound/CharacterSoundCatalog.json`의 기존 `"classes"` 객체에 `"Valtan"` 키를 새로
   추가한다(파일 포맷을 바꾸지 않음 — `CSoundCueCatalog::Load`가 이미 임의의 클래스 이름을 받아들이는
   범용 `classes.<이름>.<이벤트>` 구조라 C++ 쪽 수정이 필요 없다, `Client/Public/SoundCueCatalog.h`
   실측 확인).
5. 미매치 이벤트를 stdout에 보고한다.

## 5. 이번 스코프에서 제외하는 것 (실측 근거, 추측 아님)

- `S_BGM_CommanderRaid`/`S_Systems` bank의 SOUND row. 몬스터 보이스가 아니라 레이드 BGM 전환/시스템
  알림용으로 보이며, `UI/System`의 실제 raw 파일명이 `1891807024-0148-event__<해시>.wav`처럼
  숫자 ID라 이벤트 이름 문자열 매칭이 안 된다(실측 확인) — 별도 숫자 ID 매핑 조사가 필요한 작업이라
  이번 PLAN에 포함하지 않는다.
- `Voltan3` 폴더의 wav(728개 중 일부) — 현재 `Valtan.patternbindings.json`/`ValtanEncounter.json`에
  이걸 참조하는 phase 3 패턴이 authored되지 않아 조인 결과가 항상 0매치로 나온다(팀 결정: phase-one
  pattern master만 우선 완료된 상태).
- `CValtanPatternEffectCueDocument`가 가진 Product 카탈로그 admission/V1 alias/GPU prepared-cache
  경로는 사운드에 그대로 옮기지 않는다. 사운드는 `CSoundCueCatalog::Find_Variants`가 이미 동기·즉시
  조회이고 GPU 준비 큐가 필요 없어서, 그 복잡도를 그대로 복제하면 존재하지 않는 문제를 해결하는
  코드가 된다.

## 6. C++ 반영 — 새 파일

### 6.1 `Client/Public/ValtanPatternSoundCueDocument.h` (신규, 전체)

```cpp
#pragma once

#include "Client_Defines.h"

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

NS_BEGIN(Client)

class CEncounterPatternReference;
struct BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT;

enum class VALTAN_PATTERN_SOUND_REPEAT_POLICY : uint8_t
{
	ONCE,
	EACH_LOOP,
	END
};

struct VALTAN_PATTERN_SOUND_CUE final
{
	std::string strBindingId;
	std::string strOccurrenceId;
	std::string strPatternId;
	std::string strStageId;
	std::string strActionId;
	std::string strClipOccurrenceId;
	std::string strSoundBank;
	std::string strSoundEvent;
	VALTAN_PATTERN_SOUND_REPEAT_POLICY eRepeatPolicy =
		VALTAN_PATTERN_SOUND_REPEAT_POLICY::ONCE;
	uint32_t iStartMs = 0u;
	uint32_t iStageDurationMs = 0u;
};

struct VALTAN_PATTERN_SOUND_CUE_DOCUMENT final
{
	uint32_t iFormatVersion = 1u;
	std::string strOwnerArchetypeId;
	std::vector<VALTAN_PATTERN_SOUND_CUE> Cues;
};

/* Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json의 런타임 소비자.
CValtanPatternEffectCueDocument와 같은 (patternbindings clip 문자열) -> (Encounter actionId) 조인
규약을 쓰지만, 이미 build_valtan_pattern_sound_cues.py가 그 조인을 소스에서 구워뒀으므로 이 클래스는
JSON 필드 자체와 clip occurrence 존재 여부/스테이지 duration 범위만 검증한다 -- Effect 쪽의 Product
카탈로그 admission, V1 alias, anchor bone 검증은 사운드에 해당 사항이 없어 가져오지 않는다. */
class CValtanPatternSoundCueDocument final
{
public:
	static std::filesystem::path Resolve_Path();
	static bool_t Parse_Text(
		std::string_view Text,
		const CEncounterPatternReference& Encounter,
		const BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT& AnimationBindings,
		VALTAN_PATTERN_SOUND_CUE_DOCUMENT& InOutDocument,
		std::string& strOutStatus);
	static bool_t Load_Source(
		VALTAN_PATTERN_SOUND_CUE_DOCUMENT& InOutDocument,
		std::string& strOutStatus);
};

NS_END
```

### 6.2 `Client/Private/ValtanPatternSoundCueDocument.cpp` (신규)

`CValtanPatternEffectCueDocument::Parse_Text`(402~679줄)와 같은 조인/검증 뼈대를 사운드 필드만
남기고 구현한다 — `CEncounterPatternReference`로 `actionId -> (patternId, stageId, durationMs)`
조회, `BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT`로 `(actionId, clipOccurrenceId)` 존재 확인,
`iStartMs < iStageDurationMs` 범위 검증. 실제 코드는 구현 시점에 `CEncounterPatternReference`/
`BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT`의 정확한 public 조회 함수 시그니처를 다시 확인한 뒤
작성한다(현재 PLAN 시점에서는 `ValtanPatternEffectCueDocument.cpp`가 이미 쓰고 있는 것과 동일한
호출을 재사용할 계획이며, 새 시그니처를 추측해서 싣지 않는다).

## 7. C++ 반영 — 기존 파일 삽입 지점

### 7.1 `Client/Public/Valtan.h`

- 기준점: `void Load_PatternEffectCues();` 선언 바로 아래.
- 추가: `void Load_PatternSoundCues();` 선언.
- 기준점: `void Spawn_DuePatternEffectCues(const f32_t fActionAgeSeconds);` 선언 바로 아래.
- 추가: `void Spawn_DuePatternSoundCues(const f32_t fActionAgeSeconds);` 선언.
- 기준점: `m_PatternEffectCuesByActionId`, `m_AttemptedPatternEffectOccurrenceKeys`,
  `m_bPatternEffectCueScanAgeValid`, `m_fPatternEffectCueScanAgeSeconds` 멤버 선언부 바로 아래.
- 추가: 동일한 4종 멤버를 `PatternSoundCue`/`AttemptedPatternSoundOccurrenceKeys`/
  `PatternSoundCueScanAge*` 이름으로 미러링.

### 7.2 `Client/Private/Valtan.cpp`

- 기준점: `void CValtan::Load_PatternEffectCues() { ... }` 함수 전체(452~582줄) 바로 아래.
- 추가: `Load_PatternSoundCues()` 정의 — `CValtanPatternSoundCueDocument::Load_Source` 호출,
  `m_PatternClipByActionId`로 clip occurrence 재검증(Effect와 동일 이유: 문서 자체가 오래된 clip을
  가리킬 수 있으므로 로드 시점에 다시 확인), `m_PatternSoundCuesByActionId`에 스테이징.
- 기준점: `void CValtan::Spawn_DuePatternEffectCues(...)` 함수 전체 바로 아래.
- 추가: `Spawn_DuePatternSoundCues(...)` 정의 — 같은 `fActionAgeSeconds`/
  `m_AttemptedPatternSoundOccurrenceKeys` dedup 패턴으로 due한 cue를 찾고,
  `CGameInstance::Get().Play_Sound(...)`를 호출한다(스폰할 GameObject가 없으므로 Effect 경로의
  `CEffectPresentationService` 스폰 대신 바로 재생 — 발탄은 위치 앵커가 필요 없는 월드 사운드로
  취급, `strSoundBank`+"."+`strSoundEvent`를 `CSoundCueCatalog::Find_Variants("Valtan",
  strSoundEvent)`에 넘긴다).
- 기준점: `Load_PatternEffectCues();`를 호출하는 지점(초기화 시퀀스) 바로 아래.
- 추가: `Load_PatternSoundCues();` 호출.
- 기준점: `Spawn_DuePatternEffectCues(fActionAgeSeconds);`를 호출하는 지점(`Update` 내부) 바로 아래.
- 추가: `Spawn_DuePatternSoundCues(fActionAgeSeconds);` 호출.

## 8. 검증

- `python -c "import json; json.load(open(...))"`로 `Valtan.patternsoundcues.json` 파싱 확인.
- 생성 스크립트 stdout의 매치/미매치 개수를 RESULT에 표로 남긴다(플레이어 RESULT와 같은 형식).
- Debug 빌드(Engine -> UpdateLib -> Client)로 컴파일 확인.
- 실행 검증은 사용자 전용 경계라 에이전트가 대신 PASS로 기록하지 않는다 — Valtan 입장 후 실제 패턴
  발동 시 보이스/타격음이 들리는지는 사용자가 직접 확인한다.
