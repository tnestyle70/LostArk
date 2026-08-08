# 발탄 아레나 World Destruction 저작 파이프라인

작성일: 2026-08-07
대상 Area: `LV_LUT_HEARTRB_ED`
문서 역할: MapTool `World Destruction` 모드와 벽/바닥 파괴 수직 슬라이스의 전체 반영 코드 정본

## 0. 현재 반영 상태 실측

이 절의 모든 숫자는 2026-08-07에 저장소 파일과 발행 산출물에서 직접 읽은 값이다. 문서 인용이 아니다.

### 0.1 Deploy 자산과 배치

`Data/Maps/Imported/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.deployassets` 9행, `.deployplacements` 85행.

```text
assetId            kind     placements  stateOffActionId  fractured wmodel
DEPLOY_ITR_02306   STATIC            5           3705102  있음
DEPLOY_ITR_02307   STATIC            4           3705103  있음
DEPLOY_ITR_02308   STATIC            6           3705104  있음
DEPLOY_ITR_02309   STATIC            2           3705105  있음
DEPLOY_ITR_02310   STATIC            3           3705106  있음
DEPLOY_ITR_02311   STATIC            2           3705107  있음
DEPLOY_ITR_02315   STATIC           28                 0  있음
DEPLOY_ITR_02316   STATIC           27                 0  있음
DEPLOY_ITR_02326   ANIM              8           3705101  없음
```

`destructible`은 85행 전부 `1`, `uniformScale`은 전부 `1`, `position.y`는 `23.0222412`와 `23.04` 두 값뿐이다.

`DEPLOY_ITR_02306` INTACT는 submesh 3 / vertex 13,999, FRACTURED는 submesh 4 / vertex 26,585이다.
`DEPLOY_ITR_02326`은 catalog의 fractured 경로가 빈 문자열이고 `fractured/` 폴더 자체가 없다.

### 0.2 ITR_02326 애니메이션 실측

`DEPLOY_ITR_02326_INTACT.wmodel`의 `WMOD` 헤더 `animationCount`는 4다. 스켈레톤은 24본이고
`b_piece_00` ~ `b_piece_14` 15개가 파편 본이다.

```text
clip           channels  durationTicks  tps   seconds
ao_hit1_1            22            1.0   30     0.033
ao_off               22           61.0   30     2.033
ao_on                22            1.0   30     0.033
ao_spawn             22           30.0   30     1.000
```

`ao_off`의 `b_piece_00` 키프레임은 다음과 같다.

```text
0.000 ~ 1.833s  pos (0.093, 0.366, -0.459) 고정, scale 1.0
1.867s          pos (0.456, 0.620, -0.701)
1.900s          scale 1.0 -> 0.0
1.933s          pos.z -0.105 -> 6.615   (이미 scale 0이라 화면에 없음)
```

즉 `ao_off`는 파편이 날아가는 연출이 아니라 2초 정지 후 소멸이다. 파편 비행 연출은 이 데이터로 만들 수 없다.

### 0.3 배치 공간 구조

85개 배치 중심은 `x=155.358, z=-122.509`이다. 중심 거리 히스토그램에서 6~8m 구간이 3개뿐이라
안쪽 링 20개(`ITR_02315` 10, `ITR_02316` 6, `ITR_02326` 4)와 바깥 링 65개로 갈린다.
`stateOffActionId != 0`인 30개 중 26개가 바깥 링이고, 안쪽 링의 4개는 전부 `ITR_02326`이다.

`triggerBinaryOccurrenceCount`는 반경과 동심원으로 맞물린다.

```text
trig=2   8개   4.3 ~  6.1 m
trig=4  19개   4.2 ~ 11.7 m
trig=5  53개   7.6 ~ 14.2 m
trig=6   5개  11.6 ~ 13.3 m
```

첫 슬라이스 후보 그룹의 방위각은 다음과 같다.

```text
3705102 / ITR_02306 / 5개   d=11.04 az= 20.3 | d=11.27 az= 79.5 | d= 9.19 az=180.4
                            d= 9.24 az=248.3 | d=10.34 az=317.7
3705101 / ITR_02326 / 8개   d=10.79 az= 17.6 | d= 5.97 az= 55.8 | d=10.09 az=104.0
                            d= 4.61 az=149.5 | d= 8.54 az=199.2 | d= 4.24 az=257.7
                            d= 9.86 az=293.3 | d= 5.72 az=342.6
```

### 0.4 코드 경로 실측

```text
CMapTool::TOOL_MODE                    MAP_ASSETS / WORLD_GAMEPLAY / NAVIGATION / CAMERA 4개
CMapTool::Set_DeployPhase              MapTool.cpp:3307, 호출자 0개
CDeployPropRuntime::Set_State          호출자 0개
CDeployPropRuntime::Set_State_All      Set_DeployPhase 안에서만 호출
CLevel_ValtanArena::Initialize         MapTool.cpp가 아닌 제품 Level에서 Load_Area 수행
CDeployPropObject::Initialize          ANIM이면 "on" 논리 클립 바인딩
CDeployPropObject::Update              m_State == FRACTURED 일 때만 Play_Animation
CDeployPropObject::Set_State           FRACTURED 진입 시 "off" 논리 클립으로 교체
Client CWorldGameplayDocument          DESTROYABLE / SET_CONDITION / SET_DESTROYABLE_STATE 파서 보유
Publish-WorldGameplay.ps1:401          playerSpawn/npc/boss/triggerBox/collisionBox 외 전부 throw
Server + Shared                        destroyable 관련 심볼 0개
CServerNavigation                      Load / Find_Path(const) / Project_Point(const) 뿐, 격자 변경 API 없음
CMapTool::Collect_NavigationBakePlacements  m_Placements만 순회, m_DeployRuntime 미참조
CPlayerSkillSystem                     Project_Point 성공 시 projected.y만 사용, x/z는 원좌표
```

### 0.5 Navigation 실측

`Server/Bin/DataFiles/Navigation/LV_LUT_HEARTRB_ED.navgrid`는 392×312 셀, cell 0.5m,
origin `(-6, -165)`, 122,304셀 중 21,381셀(17.5%)이 walkable이다.

85개 벽 원점 셀은 **전부 walkable**이고, 원점 ±1m(5×5셀)까지 전부 열린 벽이 75개다.
`Data/Navigation/LV_LUT_HEARTRB_ED.navblockers` 헤더의 region 수는 `0`이다.

따라서 이 기능의 navigation 계약은 "파괴되면 열린다"가 아니라 **"무결일 때 막는다"** 이며,
아레나 바닥 붕괴는 그 반대 방향이다. 두 방향을 같은 boolean으로 표현하지 않는다.

### 0.6 Encounter 실측

`Data/Encounters/Valtan/ValtanEncounter.json`은 `lostark.encounter-profile` formatVersion 3,
`fixedTickHz` 30, pattern 31개다. pattern 당 최대 stage 7개, 최대 `durationMs` 8000,
최대 `triggerHealthBar` 159다. Server가 이 문서를 실제로 소비한다.

```text
GameplayCatalog.cpp:154   hitShape 문자열 -> BOSS_PATTERN_HIT_SHAPE
ValtanBrain.cpp:186       EnterPatternStage 가 stage별 strActionId / 히트 shape를 확정
GameRoom.cpp:880          snapshot.iPatternStageIndex 전송
```

파괴 관련 pattern 실측값은 다음과 같다.

```text
VALTAN_ARMOR_BREAK_OPENING  triggerHealthBar 159
  WALL_CHARGE   WINDUP    1500ms  hitShape NONE
  GROGGY        ACTIVE    5000ms  hitShape NONE
  RECOVERY      RECOVERY  1200ms  hitShape NONE

VALTAN_ARENA_BREAK_80  triggerHealthBar 80   (VALTAN_ARENA_BREAK_33 은 33, 구조 동일)
  CUTSCENE      WINDUP    2500ms  hitShape NONE
  LANDING       ACTIVE     900ms  CIRCLE outer 12.0  hitCount 1
  SPIN          ACTIVE    1200ms  CIRCLE outer 10.0  hitCount 3 interval 350ms
  RECOVERY      RECOVERY  1500ms  hitShape NONE
```

`WALL_CHARGE`는 이름과 달리 이동 속도, 최대 거리, 충돌 대상이 없는 1.5초 WINDUP이다.
실제 돌진 판정은 G10에서 새로 만든다.

### 0.7 Boss presentation 실측

`Data/Actors/BossCatalog.json`의 `presentationClips`는 `idle / chase / patternWindup /
patternActive / patternRecovery / dead` 6개뿐이다. `CValtan::Apply_NetworkState`는 snapshot의
`actionId`를 `m_strServerActionId`에 저장만 하고 클립 선택에는 `WORLD_ENTITY_ACTION` enum만 쓴다.
따라서 pattern별 고유 클립은 아직 없다.

### 0.8 Effect 실측

`Data/Effects/EffectCatalog.json`에 admitted effect 16개가 있고 전부 DimensionMaster다. Valtan은 0개다.
`EFFECT_SPAWN_DESC`는 `std::weak_ptr<CCharacter> pOwner` 와 `strAnchorSlotId`를 요구하며
world transform 필드가 없다. 따라서 world-space effect spawn 경로가 G12에서 필요하다.

## 1. G 경계

| G | 범위 | 종료 증거 |
|---|---|---|
| G1 | MapTool `World Destruction` 읽기 전용 진단 모드 | 5번째 모드 표시, encounter/deploy/world/nav 네 정본을 한 화면에서 읽음, 저장 경로 0개 |
| G2 | `ValtanWorldEvents.json` 문서와 strict parser, atomic save | 그룹 0개 문서 Save -> Load 왕복, 잘못된 문서에서 기존 문서 보존 |
| G3 | 그룹 저작 UI와 벽 한 그룹 작성 | `3705102` 5개를 한 그룹으로 저장하고 재로드해 동일 |
| G4 | 개별 `Set_State` 미리보기, 벽 선택·간편 편집기, nav region 연결 | 벽 선택 -> 정보/설정/강조가 한 화면에 보이고 INTACT/FRACTURED 전환이 보임. nav region 자동 셀 계산은 별도 완료 증거 필요 |
| G5 | Publisher의 destroyable / world event / navblocker 검증 | `Publish-WorldGameplay.ps1 -Mode Validate` 통과, 잘못된 참조에서 실패 |
| G6 | Server `INTACT -> BREAKING -> FRACTURED` 상태와 collision receiver | `Server.exe --contract-test` 통과 |
| G7 | Shared full sync / delta / late join | `NetworkProtocolHarness` failures 0 |
| G8 | Client `CDeployPropRuntime::Set_State` 서버 연결 | Server 명령으로만 벽이 바뀌고 재접속 Client도 같은 상태 |
| G9 | MapTool 타임라인과 파괴 미리보기 | stage 시간축 Play/Pause/Restart/현재 시점 복사. 실제 Valtan clip 동기 재생은 actionId→clip 계약 뒤 검증 |
| G10 | 돌진 충돌 판정과 스킬 이동 sweep 보강 | 벽이 실제로 이동을 막고 `Q`로 통과되지 않음 |
| G11 | 아레나 바닥 붕괴와 낙사 hazard | `VALTAN_ARENA_BREAK_80` 에서 반대 polarity가 동작 |
| G12 | 파괴 Effect / Camera / Audio 와 world-space effect spawn | Valtan admitted effect로 파편·먼지 재생 |
| G13 | 나머지 그룹 확장 | 85개 중 저작 대상 전부가 같은 계약으로 동작 |

첫 완료 단위는 G1~G8이다. 벽 한 그룹이 Server 확정으로 부서지고 collision/nav가 함께 열리며
재접속 Client가 같은 상태를 받는 것까지가 하나의 검증 단위다.

### 1.1 2026-08-08 벽 중심 간편 편집기 보강

기존 Group/Mutation/Binding 원본 구조는 `Advanced Graph Editor`로 보존한다. 기본 화면은 사용자가
내부 stable ID를 직접 조립하지 않도록 다음 순서만 노출한다.

```text
벽 선택(월드 또는 목록)
-> 선택 벽의 asset/runtimePlacementId/위치/원본 근거/그룹 표시
-> Valtan pattern과 stage 선택
-> stage 시작/선택 시점/종료/보스 충돌 조건 선택
-> 그룹 BREAKING 표시 시간과 enabled 여부 선택
-> group/mutation/binding stable ID 자동 생성
-> 전체 외부 참조 검증
-> atomic Save
```

월드 선택은 `Target_PickPos`가 돌려준 실제 렌더 표면점을 DeployProp world AABB 소유권과
대조한다. 실패는 무음으로 삼키지 않고 `no rendered surface`와 `not a loaded DeployProp`을
구분한다. 목록 선택도 같은 `Select_DestructionWall()` 경로를 사용해 그룹 역조회와 와이어
강조가 항상 같이 갱신된다.

시간축은 `ValtanEncounter.json`의 stage 누적 시간만 재생한다. 현재 제품에는 actionId별 실제
clip binding이 없으므로 이 화면은 정확한 발탄 애니메이션을 재생한다고 주장하지 않는다.
`patternWindup / patternActive / patternRecovery` 공통 클립이라는 현재 런타임 한계를 화면에
명시한다.

간편 Apply는 `CWorldDestructionDocument` 복사본에 group/member/mutation/binding을 모두 적용한
뒤 성공할 때만 교체한다. 자동 ID는 선택 벽 순서가 아니라 stable group identity와 semantic
hash를 사용하고, 같은 semantic 중복이나 hash 충돌은 저장하지 않는다. Save 직전에는 문서의
모든 DeployProp, pattern/stage, Collision Box, navigation region 외부 참조를 다시 검증한다.
같은 navigation region을 두 그룹이 동시에 소유하거나, enabled binding이 빈 벽 그룹·0-cell
navigation region·비활성 Collision Box를 가리키면 저장을 거부한다. World Gameplay 또는
Navigation에 미저장 변경이 있으면 world-events 단독 저장도 거부하고 `Save All` 순서를 요구한다.
Encounter와 WorldEvents를 함께 Reload할 때는 두 파일을 모두 stage하고 서로 검증한 뒤 한 번에
commit한다. Encounter 단독 Reload와 Area 전환도 같은 cross-document 검증을 통과한 문서만
commit한다. enabled binding은 정확히 하나의 `BOSS_VALTAN` 배치가 `ENCOUNTER_VALTAN`에 연결된
Area에서만 저장한다. 새 binding은 Server 수직 슬라이스가 닫히기 전까지 기본 `enabled=false`다.

## 2. 최초 G1 경계 (2026-08-07 기록)

아래 절은 최초 G1 계획을 보존한 기록이다. 현재 코드는 1.1절과 대응 RESULT에 적힌 대로
G2/G3, G4 일부, G9의 데이터 시간축 일부까지 확장됐다.

G1은 **읽기 전용 진단 모드**다. 저장 경로, dirty 플래그, runtime 상태 변경을 만들지 않는다.

G1이 하는 일:

1. `TOOL_MODE::WORLD_DESTRUCTION`을 5번째 모드로 추가한다.
2. `Data/Encounters/Valtan/ValtanEncounter.json`을 읽기 전용으로 strict parse하는
   `CEncounterPatternReference`를 추가한다.
3. pattern / stage / 누적 offset / Server tick 환산을 표로 보여준다.
4. `m_DeployRuntime`의 85개 배치를 asset·stateOffActionId·trig 별로 보여준다.
5. `m_WorldGameplayDocument`의 dormant `destroyable` 행을 보여준다.
6. `m_RuntimeBlockerDocument`의 nav region을 보여준다.
7. 0절에서 실측한 미구현 경계를 잠긴 진단 문구로 고정 표시한다.

G1이 하지 않는 일: 그룹 생성, 상태 변경, 저장, publisher 호출, Server 통신.

G1 종료 증거:

```text
Debug Client 빌드 성공
Lobby -> Test -> LEVEL::DEVELOPMENT -> F1 -> Map Tool
World Destruction 라디오가 5번째로 보임
Valtan Area에서 Encounter 31 pattern, Deploy 85행, Nav region 0개 표시
Bern/Character Select/Training Map 선택 시 "no destruction authoring source" 표시
Reload 버튼으로만 JSON을 다시 읽고 매 프레임 읽지 않음
손상된 ValtanEncounter.json 에서 기존 표시가 유지되고 상태 문자열만 실패로 바뀜
Has_UnsavedAuthoring() 이 이 모드 사용 후에도 false
```

## 3. 자료구조와 불변식

```text
정의 데이터   ValtanEncounter.json 의 patternId -> stages[]  (Balance Tool 소유, MapTool은 읽기만)
             .deployassets 의 assetId -> intact/fractured prototype tag
인스턴스     .deployplacements 의 runtimePlacementId -> transform + stateOffActionId
             Gameplay.world.json 의 placementId(kind=destroyable) -> deployRuntimePlacementId
             .navblockers 의 region id -> conditionId + cells[]
런타임 객체   CDeployPropRuntime 이 runtimePlacementId -> shared_ptr<CDeployPropObject> 보유

불변식
  patternId 는 문서 안에서 유일하고 stageId 는 pattern 안에서 유일하다.
  stage 누적 offset 은 앞선 stage duration 의 합이며 문서에 저장하지 않고 로드시 계산한다.
  Server tick 환산은 ceil(ms * fixedTickHz / 1000) 한 곳에서만 수행한다.
  encounter reference 는 Save 함수를 갖지 않는다. 쓰기 정본은 Balance Tool 이다.
  로드 실패 시 기존 문서를 유지하고 상태 문자열만 교체한다.

복잡도  P=pattern 31, S=stage<=7, D=deploy placement 85
  로드 O(P*S), 화면 O(P + D), 저장 없음
```

## 4. 파일 목록

| 구분 | 절대 경로 | 역할 |
|---|---|---|
| 추가 | `C:/Users/USER/source/졸업팀폴/LostArk/Client/Public/EncounterPatternReference.h` | Encounter pattern/stage 읽기 전용 참조 계약 |
| 추가 | `C:/Users/USER/source/졸업팀폴/LostArk/Client/Private/EncounterPatternReference.cpp` | 위 계약의 strict parse 구현 |
| 수정 | `C:/Users/USER/source/졸업팀폴/LostArk/Client/Public/MapTool.h` | 5번째 모드, encounter reference 멤버, 패널 함수 선언 |
| 수정 | `C:/Users/USER/source/졸업팀폴/LostArk/Client/Private/MapTool.cpp` | 모드 라디오, 패널 switch, 패널 구현, Valtan descriptor 확장 |

`MapTool.h`와 `MapTool.cpp`는 CP949 인코딩이다. 편집 도구로 전체를 다시 쓰지 말고 Visual Studio에서
해당 블록만 교체한다. 새 두 파일은 UTF-8 BOM 없음과 영문 주석으로 작성한다.

## 5. 파일별 전체 구현 코드

### 5-1. C:/Users/USER/source/졸업팀폴/LostArk/Client/Public/EncounterPatternReference.h

변경 종류: 추가
적용 위치: 새 파일

`Client_Defines.h`는 `Engine_Defines.h`를 포함하지 않는다. `NS_BEGIN`, `bool_t`, `char_t`,
`f32_t`가 모두 `Engine_Defines.h`에 있으므로 이 저장소의 모든 Client 문서 헤더처럼 둘 다 include한다.

```cpp
#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

NS_BEGIN(Client)

/* One authored stage of a boss pattern. iStartOffsetMs is derived at load time
   from the preceding stage durations and is never stored in the document. */
struct ENCOUNTER_STAGE_REFERENCE final
{
	std::string stageId;
	std::string actionId;
	std::string stageKind;
	uint32_t iDurationMs = 0;
	uint32_t iStartOffsetMs = 0;
	std::string hitShape;
	std::string serverDamageProfileId;
};

struct ENCOUNTER_PATTERN_REFERENCE final
{
	std::string patternId;
	std::string displayName;
	std::string actionId;
	std::string selectionMode;
	uint32_t iTriggerHealthBar = 0;
	uint32_t iTotalDurationMs = 0;
	std::vector<ENCOUNTER_STAGE_REFERENCE> stages;
};

/* Read-only view of Data/Encounters/<Boss>/<Boss>Encounter.json.

   The editable draft of that document is owned by CBalanceTool, which parses,
   edits and saves it. This class exists so authoring tools that only need to
   resolve stable patternId/stageId/actionId can do so without holding an
   editable draft, and it deliberately has no Save path. If the encounter
   schema changes, the exact key lists in the matching cpp must change in the
   same commit; unknown properties are rejected instead of ignored. */
class CEncounterPatternReference final
{
public:
	static constexpr uint32_t MAX_PATTERN_COUNT = 256u;
	static constexpr uint32_t MAX_STAGE_COUNT = 64u;
	static constexpr uint32_t MAX_STAGE_DURATION_MS = 60000u;
	static constexpr uint32_t MAX_FIXED_TICK_HZ = 240u;

public:
	bool_t Load(
		const std::filesystem::path& path,
		std::string& outStatus);
	void Clear();

public:
	bool_t Is_Ready() const { return m_isReady; }
	const std::string& Get_EncounterId() const { return m_EncounterId; }
	const std::string& Get_BossArchetypeId() const
	{
		return m_BossArchetypeId;
	}
	uint32_t Get_FixedTickHz() const { return m_iFixedTickHz; }
	const std::vector<ENCOUNTER_PATTERN_REFERENCE>& Get_Patterns() const
	{
		return m_Patterns;
	}
	const ENCOUNTER_PATTERN_REFERENCE* Find_Pattern(
		const std::string& patternId) const;

	static uint32_t To_ServerTick(
		uint32_t milliseconds,
		uint32_t fixedTickHz);

private:
	std::string m_EncounterId;
	std::string m_BossArchetypeId;
	uint32_t m_iFixedTickHz = 0;
	std::vector<ENCOUNTER_PATTERN_REFERENCE> m_Patterns;
	bool_t m_isReady = false;
};

NS_END
```

### 5-2. C:/Users/USER/source/졸업팀폴/LostArk/Client/Private/EncounterPatternReference.cpp

변경 종류: 추가
적용 위치: 새 파일

```cpp
#include "EncounterPatternReference.h"

#include "DataJson.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <initializer_list>
#include <sstream>
#include <unordered_set>

namespace
{
	using namespace Client;

	constexpr const char_t* SCHEMA = "lostark.encounter-profile";
	constexpr uint32_t FORMAT_VERSION = 3u;

	bool_t Read_TextFile(
		const std::filesystem::path& path,
		std::string& outText)
	{
		std::ifstream input(path, std::ios::binary);
		if (!input.is_open())
			return false;
		std::ostringstream buffer;
		buffer << input.rdbuf();
		if (input.bad())
			return false;
		outText = buffer.str();
		return true;
	}

	/* Rejects documents that carry properties this reader does not understand,
	   so a schema change fails loudly instead of being silently dropped. */
	bool_t Is_ExactObject(
		const DATA_JSON_VALUE& value,
		const std::initializer_list<const char_t*> keys)
	{
		if (!value.Is_Object() || value.Get_Object().size() != keys.size())
			return false;
		return std::all_of(keys.begin(), keys.end(),
			[&value](const char_t* key)
			{
				return nullptr != value.Find(key);
			});
	}

	bool_t Read_String(
		const DATA_JSON_VALUE& parent,
		const char_t* key,
		const bool_t allowEmpty,
		std::string& outValue)
	{
		const DATA_JSON_VALUE* value = parent.Find(key);
		if (nullptr == value || !value->Is_String() ||
			(!allowEmpty && value->Get_String().empty()))
		{
			return false;
		}
		outValue = value->Get_String();
		return true;
	}

	bool_t Read_Unsigned(
		const DATA_JSON_VALUE& parent,
		const char_t* key,
		const uint32_t maximum,
		uint32_t& outValue)
	{
		const DATA_JSON_VALUE* value = parent.Find(key);
		if (nullptr == value || !value->Is_Number())
			return false;
		const double number = value->Get_Number();
		if (!std::isfinite(number) || std::floor(number) != number ||
			number < 0.0 || number > static_cast<double>(maximum))
		{
			return false;
		}
		outValue = static_cast<uint32_t>(number);
		return true;
	}

	bool_t Is_FiniteNumber(
		const DATA_JSON_VALUE& parent,
		const char_t* key)
	{
		const DATA_JSON_VALUE* value = parent.Find(key);
		return nullptr != value && value->Is_Number() &&
			std::isfinite(value->Get_Number());
	}

	bool_t Is_FiniteNumberArray(
		const DATA_JSON_VALUE& parent,
		const char_t* key)
	{
		const DATA_JSON_VALUE* value = parent.Find(key);
		if (nullptr == value || !value->Is_Array())
			return false;
		return std::all_of(value->Get_Array().begin(),
			value->Get_Array().end(),
			[](const DATA_JSON_VALUE& element)
			{
				return element.Is_Number() &&
					std::isfinite(element.Get_Number());
			});
	}
}

bool_t Client::CEncounterPatternReference::Load(
	const std::filesystem::path& path,
	std::string& outStatus)
{
	std::string text;
	if (path.empty() || !Read_TextFile(path, text))
	{
		outStatus = "Encounter reference is unreadable: " + path.string();
		return false;
	}

	std::string parseError;
	DATA_JSON_VALUE root;
	if (!CDataJson::Parse(text, root, parseError))
	{
		outStatus = "Encounter reference parse failed: " + parseError;
		return false;
	}
	if (!Is_ExactObject(root, {
			"schema", "formatVersion", "encounterId", "bossArchetypeId",
			"authority", "fixedTickHz", "states", "patterns" }))
	{
		outStatus = "Encounter reference root has unexpected properties";
		return false;
	}

	std::string schema;
	std::string encounterId;
	std::string bossArchetypeId;
	std::string authority;
	uint32_t formatVersion = 0u;
	uint32_t fixedTickHz = 0u;
	if (!Read_String(root, "schema", false, schema) || SCHEMA != schema ||
		!Read_Unsigned(root, "formatVersion", FORMAT_VERSION, formatVersion) ||
		FORMAT_VERSION != formatVersion ||
		!Read_String(root, "encounterId", false, encounterId) ||
		!Read_String(root, "bossArchetypeId", false, bossArchetypeId) ||
		!Read_String(root, "authority", false, authority) ||
		!Read_Unsigned(root, "fixedTickHz", MAX_FIXED_TICK_HZ, fixedTickHz) ||
		0u == fixedTickHz)
	{
		outStatus = "Encounter reference header is invalid";
		return false;
	}

	const DATA_JSON_VALUE* states = root.Find("states");
	const DATA_JSON_VALUE* patterns = root.Find("patterns");
	if (nullptr == states || !states->Is_Array() ||
		nullptr == patterns || !patterns->Is_Array() ||
		patterns->Get_Array().empty() ||
		patterns->Get_Array().size() > MAX_PATTERN_COUNT)
	{
		outStatus = "Encounter reference pattern array is invalid";
		return false;
	}

	std::vector<ENCOUNTER_PATTERN_REFERENCE> staged;
	staged.reserve(patterns->Get_Array().size());
	std::unordered_set<std::string> patternIds;

	for (const DATA_JSON_VALUE& entry : patterns->Get_Array())
	{
		if (!Is_ExactObject(entry, {
				"patternId", "displayName", "actionId", "sourceActionIds",
				"selectionMode", "minimumHealthBar", "maximumHealthBar",
				"triggerHealthBar", "triggerOrder", "selectionWeight",
				"maximumConsecutiveUses", "minimumRange", "maximumRange",
				"stages" }))
		{
			outStatus = "Encounter pattern has unexpected properties";
			return false;
		}

		ENCOUNTER_PATTERN_REFERENCE pattern;
		uint32_t ignored = 0u;
		if (!Read_String(entry, "patternId", false, pattern.patternId) ||
			!Read_String(entry, "displayName", false, pattern.displayName) ||
			!Read_String(entry, "actionId", false, pattern.actionId) ||
			!Read_String(entry, "selectionMode", false,
				pattern.selectionMode) ||
			!Read_Unsigned(entry, "minimumHealthBar", 1000u, ignored) ||
			!Read_Unsigned(entry, "maximumHealthBar", 1000u, ignored) ||
			!Read_Unsigned(entry, "triggerHealthBar", 1000u,
				pattern.iTriggerHealthBar) ||
			!Read_Unsigned(entry, "triggerOrder", 1000u, ignored) ||
			!Read_Unsigned(entry, "selectionWeight", 1000u, ignored) ||
			!Read_Unsigned(entry, "maximumConsecutiveUses", 1000u, ignored) ||
			!Is_FiniteNumber(entry, "minimumRange") ||
			!Is_FiniteNumber(entry, "maximumRange") ||
			!Is_FiniteNumberArray(entry, "sourceActionIds"))
		{
			outStatus = "Encounter pattern field is invalid";
			return false;
		}
		if (!patternIds.insert(pattern.patternId).second)
		{
			outStatus = "Duplicate encounter pattern: " + pattern.patternId;
			return false;
		}

		const DATA_JSON_VALUE* stages = entry.Find("stages");
		if (nullptr == stages || !stages->Is_Array() ||
			stages->Get_Array().empty() ||
			stages->Get_Array().size() > MAX_STAGE_COUNT)
		{
			outStatus = "Encounter pattern stage array is invalid: " +
				pattern.patternId;
			return false;
		}

		std::unordered_set<std::string> stageIds;
		for (const DATA_JSON_VALUE& stageEntry : stages->Get_Array())
		{
			if (!Is_ExactObject(stageEntry, {
					"stageId", "actionId", "stageKind", "durationMs",
					"hitShape", "hitOuterRadius", "hitInnerRadius",
					"hitAngleDegrees", "hitLength", "hitHalfWidth",
					"hitCount", "hitIntervalMs", "serverDamageProfileId" }))
			{
				outStatus = "Encounter stage has unexpected properties: " +
					pattern.patternId;
				return false;
			}

			ENCOUNTER_STAGE_REFERENCE stage;
			uint32_t stageIgnored = 0u;
			if (!Read_String(stageEntry, "stageId", false, stage.stageId) ||
				!Read_String(stageEntry, "actionId", false, stage.actionId) ||
				!Read_String(stageEntry, "stageKind", false,
					stage.stageKind) ||
				!Read_Unsigned(stageEntry, "durationMs",
					MAX_STAGE_DURATION_MS, stage.iDurationMs) ||
				!Read_String(stageEntry, "hitShape", false, stage.hitShape) ||
				!Read_Unsigned(stageEntry, "hitCount", 1000u, stageIgnored) ||
				!Read_Unsigned(stageEntry, "hitIntervalMs",
					MAX_STAGE_DURATION_MS, stageIgnored) ||
				!Read_String(stageEntry, "serverDamageProfileId", true,
					stage.serverDamageProfileId) ||
				!Is_FiniteNumber(stageEntry, "hitOuterRadius") ||
				!Is_FiniteNumber(stageEntry, "hitInnerRadius") ||
				!Is_FiniteNumber(stageEntry, "hitAngleDegrees") ||
				!Is_FiniteNumber(stageEntry, "hitLength") ||
				!Is_FiniteNumber(stageEntry, "hitHalfWidth"))
			{
				outStatus = "Encounter stage field is invalid: " +
					pattern.patternId;
				return false;
			}
			if (!stageIds.insert(stage.stageId).second)
			{
				outStatus = "Duplicate encounter stage: " +
					pattern.patternId + "/" + stage.stageId;
				return false;
			}

			stage.iStartOffsetMs = pattern.iTotalDurationMs;
			pattern.iTotalDurationMs += stage.iDurationMs;
			pattern.stages.push_back(std::move(stage));
		}

		staged.push_back(std::move(pattern));
	}

	m_EncounterId = std::move(encounterId);
	m_BossArchetypeId = std::move(bossArchetypeId);
	m_iFixedTickHz = fixedTickHz;
	m_Patterns = std::move(staged);
	m_isReady = true;
	outStatus = "Loaded encounter reference: " + m_EncounterId + " (" +
		std::to_string(m_Patterns.size()) + " patterns)";
	return true;
}

void Client::CEncounterPatternReference::Clear()
{
	m_EncounterId.clear();
	m_BossArchetypeId.clear();
	m_iFixedTickHz = 0u;
	m_Patterns.clear();
	m_isReady = false;
}

const Client::ENCOUNTER_PATTERN_REFERENCE*
Client::CEncounterPatternReference::Find_Pattern(
	const std::string& patternId) const
{
	const auto iter = std::find_if(m_Patterns.begin(), m_Patterns.end(),
		[&patternId](const ENCOUNTER_PATTERN_REFERENCE& pattern)
		{
			return pattern.patternId == patternId;
		});
	return m_Patterns.end() == iter ? nullptr : &(*iter);
}

uint32_t Client::CEncounterPatternReference::To_ServerTick(
	const uint32_t milliseconds,
	const uint32_t fixedTickHz)
{
	if (0u == fixedTickHz)
		return 0u;
	const uint64_t numerator =
		static_cast<uint64_t>(milliseconds) *
		static_cast<uint64_t>(fixedTickHz) + 999ull;
	return static_cast<uint32_t>(numerator / 1000ull);
}
```

### 5-3. C:/Users/USER/source/졸업팀폴/LostArk/Client/Public/MapTool.h

변경 종류: 함수 교체 (다섯 블록)
적용 위치: include 목록 / `TOOL_MODE` / `EDITOR_AREA_DESCRIPTOR` / Frame Render 선언 / 멤버 변수

#### 5-3-1. include 추가

기준점: `#include "DeployPropRuntime.h"` 바로 아래

```cpp
#include "DeployPropRuntime.h"
#include "EncounterPatternReference.h"
```

#### 5-3-2. `TOOL_MODE` 전체 교체

```cpp
	enum class TOOL_MODE
	{
		MAP_ASSETS,
		WORLD_GAMEPLAY,
		WORLD_DESTRUCTION,
		NAVIGATION,
		CAMERA,
	};
```

#### 5-3-3. `EDITOR_AREA_DESCRIPTOR` 전체 교체

```cpp
	struct EDITOR_AREA_DESCRIPTOR
	{
		std::string areaId;
		std::string label;
		std::filesystem::path sourceCatalog;
		std::filesystem::path sourcePlacements;
		std::filesystem::path sourceDeployCatalog;
		std::filesystem::path sourceDeployPlacements;
		std::filesystem::path navigationSource;
		std::filesystem::path navigationPaint;
		std::filesystem::path navigationBlockers;
		std::filesystem::path gameplayDocument;
		std::filesystem::path encounterReference;
		EDITOR_NAVIGATION_POLICY navigationPolicy =
			EDITOR_NAVIGATION_POLICY::NONE;
		EDITOR_GAMEPLAY_POLICY gameplayPolicy =
			EDITOR_GAMEPLAY_POLICY::NONE;
		bool_t allowNavigationBootstrap = false;
	};
```

#### 5-3-4. Frame Render 선언 추가

기준점: `void Render_SpawnGroupsPanel();` 바로 아래, `void Render_ModeBar();` 바로 위

```cpp
	void Render_SpawnGroupsPanel();
	void Render_WorldDestructionPanel(bool_t isAssetTest);
	void Render_DestructionEncounterSource();
	void Render_DestructionDeployList();
	void Render_DestructionWorldRows();
	void Render_DestructionNavigationRegions();
	void Render_DestructionDiagnostics();
	void Render_ModeBar();
```

#### 5-3-5. World Destruction 저작 선언 추가

기준점: `std::filesystem::path Get_SpawnGroupsPath() const;` 바로 아래, `/* Queries */` 바로 위

```cpp
	std::filesystem::path Get_SpawnGroupsPath() const;

	/* World Destruction Authoring */
	bool_t Load_EncounterReference();
```

#### 5-3-6. 멤버 변수 추가

기준점: `bool_t m_bWorldTriggerOnce = true;` 바로 아래, `/* Navigation State */` 바로 위

```cpp
	bool_t m_bWorldTriggerOnce = true;

	/* World Destruction State */
	CEncounterPatternReference m_EncounterReference;
	std::string m_EncounterReferenceStatus =
		"Press Reload Encounter Reference";
	std::string m_SelectedDestructionPatternId;
	uint64_t m_iSelectedDeployPlacementId = 0;
	bool_t m_bDestructionOnlyWithOffAction = false;
	char m_DestructionDeployFilter[128]{};
```

### 5-4. C:/Users/USER/source/졸업팀폴/LostArk/Client/Private/MapTool.cpp

변경 종류: 함수 교체 (두 함수) + 함수 추가 (일곱 함수) + 블록 추가 (Valtan descriptor)
적용 위치: `Render_ActiveMode` / `Render_ModeBar` / `Load_EditorAreaRegistry`

#### 5-4-1. `Render_ActiveMode` 전체 교체

```cpp
void Client::CMapTool::Render_ActiveMode(bool_t isAssetTest)
{
	switch (m_eToolMode)
	{
	case TOOL_MODE::MAP_ASSETS:
		Render_MapAssetsPanel(isAssetTest);
		break;

	case TOOL_MODE::WORLD_GAMEPLAY:
		Render_WorldGameplayPanel(isAssetTest);
		break;

	case TOOL_MODE::WORLD_DESTRUCTION:
		ImGui::BeginDisabled(!isAssetTest);
		Render_WorldDestructionPanel(isAssetTest);
		ImGui::EndDisabled();
		break;

	case TOOL_MODE::NAVIGATION:
		ImGui::BeginDisabled(!isAssetTest);
		Render_NavigationPanel();
		ImGui::EndDisabled();
		break;

	case TOOL_MODE::CAMERA:
		ImGui::BeginDisabled(!isAssetTest);
		Render_CameraPanel();
		ImGui::EndDisabled();
		break;
	}
}
```

#### 5-4-2. `Render_ModeBar` 전체 교체

```cpp
void Client::CMapTool::Render_ModeBar()
{
	if (ImGui::RadioButton(
		"Map Assets",
		TOOL_MODE::MAP_ASSETS == m_eToolMode))
	{
		m_eToolMode = TOOL_MODE::MAP_ASSETS;
		m_bWorldGameplayPlacementArmed = false;
	}
	ImGui::SameLine();
	if (ImGui::RadioButton(
		"World Gameplay",
		TOOL_MODE::WORLD_GAMEPLAY == m_eToolMode))
	{
		m_eToolMode = TOOL_MODE::WORLD_GAMEPLAY;
		m_ePlacementState = PLACEMENT_STATE::IDLE;
		m_bNavigationStrokeActive = false;
	}
	ImGui::SameLine();
	if (ImGui::RadioButton(
		"World Destruction",
		TOOL_MODE::WORLD_DESTRUCTION == m_eToolMode))
	{
		m_eToolMode = TOOL_MODE::WORLD_DESTRUCTION;
		m_ePlacementState = PLACEMENT_STATE::IDLE;
		m_bWorldGameplayPlacementArmed = false;
		m_bWorldTriggerTargetPickArmed = false;
		m_bSpawnAnchorPlacementArmed = false;
		m_bNavigationStrokeActive = false;
	}
	ImGui::SameLine();
	if (ImGui::RadioButton(
		"Navigation",
		TOOL_MODE::NAVIGATION == m_eToolMode))
	{
		m_eToolMode = TOOL_MODE::NAVIGATION;
		m_ePlacementState = PLACEMENT_STATE::IDLE;
		m_bWorldGameplayPlacementArmed = false;
	}
	ImGui::SameLine();
	if (ImGui::RadioButton(
		"Camera",
		TOOL_MODE::CAMERA == m_eToolMode))
	{
		m_eToolMode = TOOL_MODE::CAMERA;
		m_ePlacementState = PLACEMENT_STATE::IDLE;
		m_bWorldGameplayPlacementArmed = false;
		m_bNavigationStrokeActive = false;
	}
}
```

#### 5-4-3. Valtan descriptor 확장

기준점: `Load_EditorAreaRegistry` 안의 `descriptor.navigationPolicy = EDITOR_NAVIGATION_POLICY::SOURCE_PAINT_BLOCKERS;` 로 끝나는 `if (descriptor.areaId == "LV_LUT_HEARTRB_ED")` 블록 전체 교체

```cpp
		if (descriptor.areaId == "LV_LUT_HEARTRB_ED")
		{
			std::string blockers;
			if (!ReadRequiredString(*selected, "navigationBlockers", blockers))
			{
				m_Status = "Valtan navigation blocker source is missing";
				return false;
			}
			descriptor.navigationBlockers = ResolveDataCatalogPath(blockers);
			descriptor.navigationPolicy =
				EDITOR_NAVIGATION_POLICY::SOURCE_PAINT_BLOCKERS;
			/* The editor area registry already hardcodes which four areas the
			   Map Tool opens, so the destruction reference path is declared the
			   same way instead of adding a field to the shared MapCatalog
			   schema. The panel cross-checks the loaded encounterId against the
			   boss placement in this area's gameplay document. */
			descriptor.encounterReference = CProjectDataRoot::Resolve(
				L"Encounters/Valtan/ValtanEncounter.json");
			if (descriptor.encounterReference.empty())
			{
				m_Status = "Valtan encounter reference path is invalid";
				return false;
			}
		}
```

#### 5-4-4. 새 함수 정의 일곱 개

적용 위치: `void Client::CMapTool::Render_ModeBar()` 정의 바로 위

```cpp
void Client::CMapTool::Render_WorldDestructionPanel(bool_t isAssetTest)
{
	const EDITOR_AREA_DESCRIPTOR* descriptor = Get_ActiveEditorArea();
	ImGui::Text("Area: %s",
		nullptr != descriptor ? descriptor->label.c_str() : "NONE");
	ImGui::SameLine();
	ImGui::TextUnformatted(
		"| Read-only diagnostics. This mode never writes a document.");
	ImGui::Separator();

	if (!isAssetTest)
	{
		ImGui::TextUnformatted(
			"Enter the Debug Map Editor workspace to inspect this Area.");
		return;
	}
	if (nullptr == descriptor || descriptor->encounterReference.empty())
	{
		ImGui::TextUnformatted(
			"This Area declares no destruction authoring source.");
		ImGui::TextUnformatted("Select Valtan in the workspace bar.");
		return;
	}

	Render_DestructionEncounterSource();
	Render_DestructionDeployList();
	Render_DestructionWorldRows();
	Render_DestructionNavigationRegions();
	Render_DestructionDiagnostics();
}

void Client::CMapTool::Render_DestructionEncounterSource()
{
	if (!ImGui::CollapsingHeader("Encounter Source",
		ImGuiTreeNodeFlags_DefaultOpen))
	{
		return;
	}

	if (ImGui::Button("Reload Encounter Reference"))
		Load_EncounterReference();
	ImGui::SameLine();
	ImGui::TextWrapped("%s", m_EncounterReferenceStatus.c_str());
	if (!m_EncounterReference.Is_Ready())
		return;

	const uint32_t fixedTickHz = m_EncounterReference.Get_FixedTickHz();
	ImGui::Text("Encounter %s | Boss %s | %u Hz | %zu patterns",
		m_EncounterReference.Get_EncounterId().c_str(),
		m_EncounterReference.Get_BossArchetypeId().c_str(),
		fixedTickHz,
		m_EncounterReference.Get_Patterns().size());

	/* Authoring cross-check: the boss placement of this Area must reference the
	   same encounter this reference document declares. */
	const WORLD_GAMEPLAY_PLACEMENT* bossPlacement = nullptr;
	for (const WORLD_GAMEPLAY_PLACEMENT& placement :
		m_WorldGameplayDocument.Get_Placements())
	{
		if (WORLD_PLACEMENT_KIND::BOSS == placement.eKind)
		{
			bossPlacement = &placement;
			break;
		}
	}
	if (nullptr == bossPlacement)
	{
		ImGui::TextColored(ImVec4(1.f, 0.85f, 0.2f, 1.f),
			"Gameplay document has no boss placement to cross-check.");
	}
	else if (bossPlacement->encounterId !=
		m_EncounterReference.Get_EncounterId())
	{
		ImGui::TextColored(ImVec4(1.f, 0.4f, 0.4f, 1.f),
			"Boss placement %s references %s, not %s.",
			bossPlacement->placementId.c_str(),
			bossPlacement->encounterId.c_str(),
			m_EncounterReference.Get_EncounterId().c_str());
	}

	if (ImGui::BeginTable("DestructionPatterns", 4,
		ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
		ImGuiTableFlags_ScrollY,
		ImVec2(0.f, 180.f)))
	{
		ImGui::TableSetupColumn("patternId");
		ImGui::TableSetupColumn("HP bar");
		ImGui::TableSetupColumn("stages");
		ImGui::TableSetupColumn("total ms");
		ImGui::TableHeadersRow();
		for (const ENCOUNTER_PATTERN_REFERENCE& pattern :
			m_EncounterReference.Get_Patterns())
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			const bool_t selected =
				m_SelectedDestructionPatternId == pattern.patternId;
			if (ImGui::Selectable(pattern.patternId.c_str(), selected,
				ImGuiSelectableFlags_SpanAllColumns))
			{
				m_SelectedDestructionPatternId = pattern.patternId;
			}
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%u", pattern.iTriggerHealthBar);
			ImGui::TableSetColumnIndex(2);
			ImGui::Text("%zu", pattern.stages.size());
			ImGui::TableSetColumnIndex(3);
			ImGui::Text("%u", pattern.iTotalDurationMs);
		}
		ImGui::EndTable();
	}

	const ENCOUNTER_PATTERN_REFERENCE* pattern =
		m_SelectedDestructionPatternId.empty() ? nullptr :
		m_EncounterReference.Find_Pattern(m_SelectedDestructionPatternId);
	if (nullptr == pattern)
	{
		ImGui::TextUnformatted("Select a pattern to inspect its stages.");
		return;
	}

	ImGui::Text("%s | actionId %s | selection %s",
		pattern->displayName.c_str(),
		pattern->actionId.c_str(),
		pattern->selectionMode.c_str());
	if (ImGui::BeginTable("DestructionStages", 7,
		ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
	{
		ImGui::TableSetupColumn("stageId");
		ImGui::TableSetupColumn("kind");
		ImGui::TableSetupColumn("start ms");
		ImGui::TableSetupColumn("duration ms");
		ImGui::TableSetupColumn("start tick");
		ImGui::TableSetupColumn("hitShape");
		ImGui::TableSetupColumn("actionId");
		ImGui::TableHeadersRow();
		for (const ENCOUNTER_STAGE_REFERENCE& stage : pattern->stages)
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::TextUnformatted(stage.stageId.c_str());
			ImGui::TableSetColumnIndex(1);
			ImGui::TextUnformatted(stage.stageKind.c_str());
			ImGui::TableSetColumnIndex(2);
			ImGui::Text("%u", stage.iStartOffsetMs);
			ImGui::TableSetColumnIndex(3);
			ImGui::Text("%u", stage.iDurationMs);
			ImGui::TableSetColumnIndex(4);
			ImGui::Text("%u", CEncounterPatternReference::To_ServerTick(
				stage.iStartOffsetMs, fixedTickHz));
			ImGui::TableSetColumnIndex(5);
			ImGui::TextUnformatted(stage.hitShape.c_str());
			ImGui::TableSetColumnIndex(6);
			ImGui::TextUnformatted(stage.actionId.c_str());
		}
		ImGui::EndTable();
	}
}

void Client::CMapTool::Render_DestructionDeployList()
{
	if (!ImGui::CollapsingHeader("Deploy Props",
		ImGuiTreeNodeFlags_DefaultOpen))
	{
		return;
	}

	const std::vector<DEPLOY_RUNTIME_ENTRY>& entries =
		m_DeployRuntime.Get_Entries();
	ImGui::Text("Loaded placements: %zu | Runtime: %s",
		entries.size(),
		m_DeployRuntime.Is_Loaded() ? "READY" : "NOT LOADED");
	ImGui::SetNextItemWidth(220.f);
	ImGui::InputText("Asset filter", m_DestructionDeployFilter,
		IM_ARRAYSIZE(m_DestructionDeployFilter));
	ImGui::SameLine();
	ImGui::Checkbox("Only rows with stateOffActionId",
		&m_bDestructionOnlyWithOffAction);

	if (!ImGui::BeginTable("DestructionDeployRows", 6,
		ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
		ImGuiTableFlags_ScrollY,
		ImVec2(0.f, 240.f)))
	{
		return;
	}
	ImGui::TableSetupColumn("runtimePlacementId");
	ImGui::TableSetupColumn("assetId");
	ImGui::TableSetupColumn("stateOff");
	ImGui::TableSetupColumn("trig");
	ImGui::TableSetupColumn("position");
	ImGui::TableSetupColumn("state");
	ImGui::TableHeadersRow();

	const std::string filter = m_DestructionDeployFilter;
	for (const DEPLOY_RUNTIME_ENTRY& entry : entries)
	{
		if (m_bDestructionOnlyWithOffAction &&
			0u == entry.placement.stateOffActionId)
		{
			continue;
		}
		if (!filter.empty() &&
			std::string::npos == entry.placement.assetId.find(filter))
		{
			continue;
		}

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		const std::string idText =
			std::to_string(entry.placement.runtimePlacementId);
		const bool_t selected = m_iSelectedDeployPlacementId ==
			entry.placement.runtimePlacementId;
		if (ImGui::Selectable(idText.c_str(), selected,
			ImGuiSelectableFlags_SpanAllColumns))
		{
			m_iSelectedDeployPlacementId =
				entry.placement.runtimePlacementId;
		}
		ImGui::TableSetColumnIndex(1);
		ImGui::TextUnformatted(entry.placement.assetId.c_str());
		ImGui::TableSetColumnIndex(2);
		ImGui::Text("%u", entry.placement.stateOffActionId);
		ImGui::TableSetColumnIndex(3);
		ImGui::Text("%u", entry.placement.triggerBinaryOccurrenceCount);
		ImGui::TableSetColumnIndex(4);
		ImGui::Text("%.2f, %.2f, %.2f",
			entry.placement.position.x,
			entry.placement.position.y,
			entry.placement.position.z);
		ImGui::TableSetColumnIndex(5);
		ImGui::TextUnformatted(nullptr != entry.object ? "SPAWNED" : "NONE");
	}
	ImGui::EndTable();
}

void Client::CMapTool::Render_DestructionWorldRows()
{
	if (!ImGui::CollapsingHeader("World Gameplay destroyable rows"))
		return;

	size_t destroyableCount = 0;
	if (ImGui::BeginTable("DestructionWorldRows", 4,
		ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
	{
		ImGui::TableSetupColumn("placementId");
		ImGui::TableSetupColumn("deployRuntimePlacementId");
		ImGui::TableSetupColumn("initialState");
		ImGui::TableSetupColumn("enabled");
		ImGui::TableHeadersRow();
		for (const WORLD_GAMEPLAY_PLACEMENT& placement :
			m_WorldGameplayDocument.Get_Placements())
		{
			if (WORLD_PLACEMENT_KIND::DESTROYABLE != placement.eKind)
				continue;
			++destroyableCount;
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::TextUnformatted(placement.placementId.c_str());
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%llu",
				static_cast<unsigned long long>(
					placement.deployRuntimePlacementId));
			ImGui::TableSetColumnIndex(2);
			ImGui::TextUnformatted(
				CWorldGameplayDocument::DestroyableState_ToString(
					placement.eInitialState));
			ImGui::TableSetColumnIndex(3);
			ImGui::TextUnformatted(placement.isEnabled ? "true" : "false");
		}
		ImGui::EndTable();
	}
	ImGui::Text("destroyable rows: %zu", destroyableCount);
	ImGui::TextWrapped(
		"The Client document parser accepts this kind, but "
		"Publish-WorldGameplay.ps1 rejects every kind outside "
		"playerSpawn/npc/boss/triggerBox/collisionBox. G5 opens that gate.");
}

void Client::CMapTool::Render_DestructionNavigationRegions()
{
	if (!ImGui::CollapsingHeader("Navigation blocker regions"))
		return;

	ImGui::Text("Document: %s | regions: %zu",
		m_RuntimeBlockerDocument.Is_Ready() ? "READY" : "NOT READY",
		m_RuntimeBlockerDocument.Get_RegionCount());
	if (0 == m_RuntimeBlockerDocument.Get_RegionCount())
	{
		ImGui::TextWrapped(
			"No region is authored yet. The baked grid treats every wall "
			"footprint as walkable, so an INTACT wall has to add a blocker "
			"rather than remove one.");
		return;
	}

	if (!ImGui::BeginTable("DestructionNavRegions", 4,
		ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
	{
		return;
	}
	ImGui::TableSetupColumn("regionId");
	ImGui::TableSetupColumn("conditionId");
	ImGui::TableSetupColumn("activateWhenTrue");
	ImGui::TableSetupColumn("cells");
	ImGui::TableHeadersRow();
	for (size_t index = 0;
		index < m_RuntimeBlockerDocument.Get_RegionCount();
		++index)
	{
		const NAV_RUNTIME_BLOCKER_REGION* region =
			m_RuntimeBlockerDocument.Get_Region(index);
		if (nullptr == region)
			continue;
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::TextUnformatted(region->id.c_str());
		ImGui::TableSetColumnIndex(1);
		ImGui::TextUnformatted(region->conditionId.c_str());
		ImGui::TableSetColumnIndex(2);
		ImGui::TextUnformatted(
			region->activateWhenConditionTrue ? "true" : "false");
		ImGui::TableSetColumnIndex(3);
		ImGui::Text("%u",
			m_RuntimeBlockerDocument.Get_RegionCellCount(index));
	}
	ImGui::EndTable();
}

void Client::CMapTool::Render_DestructionDiagnostics()
{
	if (!ImGui::CollapsingHeader("Diagnostics"))
		return;

	ImGui::TextWrapped(
		"Measured on 2026-08-07. These are locked notes, not runtime state.");
	ImGui::BulletText(
		"Server and Shared contain no destroyable state, message or receiver.");
	ImGui::BulletText(
		"CDeployPropRuntime::Set_State has no product caller; only the dead "
		"Set_DeployPhase calls Set_State_All.");
	ImGui::BulletText(
		"All 85 wall footprints are walkable in the published navgrid, so "
		"navigation must block while INTACT and open while FRACTURED.");
	ImGui::BulletText(
		"Arena floor collapse is the opposite polarity of a wall and must not "
		"share one boolean.");
	ImGui::BulletText(
		"CServerNavigation exposes only Load/Find_Path/Project_Point and "
		"cannot mutate the grid after load.");
	ImGui::BulletText(
		"CPlayerSkillSystem keeps the raw X/Z after Project_Point, so skill "
		"movement passes through any blocker until G10.");
	ImGui::BulletText(
		"DEPLOY_ITR_02326 has no fractured mesh and its ao_off clip holds "
		"still for 1.833s then scales to zero at 1.900s.");
}
```

#### 5-4-5. `Load_EncounterReference` 정의 추가

적용 위치: `std::filesystem::path Client::CMapTool::Get_SpawnGroupsPath() const` 정의 바로 아래

```cpp
bool_t Client::CMapTool::Load_EncounterReference()
{
	const EDITOR_AREA_DESCRIPTOR* descriptor = Get_ActiveEditorArea();
	if (nullptr == descriptor || descriptor->encounterReference.empty())
	{
		m_EncounterReferenceStatus =
			"Active Area declares no encounter reference";
		return false;
	}

	std::string status;
	if (!m_EncounterReference.Load(descriptor->encounterReference, status))
	{
		m_EncounterReferenceStatus = status;
		return false;
	}
	m_EncounterReferenceStatus = status;
	if (nullptr == m_EncounterReference.Find_Pattern(
		m_SelectedDestructionPatternId))
	{
		m_SelectedDestructionPatternId.clear();
	}
	return true;
}
```

## 6. 프로젝트 등록

`Client.vcxproj`와 `Client.vcxproj.filters`는 직접 편집하지 않는다. Visual Studio에서 등록한다.

```text
솔루션 탐색기 -> Client 프로젝트 -> 01.Public 필터 -> 추가 -> 기존 항목
  Client/Public/EncounterPatternReference.h

솔루션 탐색기 -> Client 프로젝트 -> 02.Private 필터 -> 추가 -> 기존 항목
  Client/Private/EncounterPatternReference.cpp
```

Visual Studio가 `<ClInclude Include="..\Public\EncounterPatternReference.h" />`와
`<ClCompile Include="..\Private\EncounterPatternReference.cpp" />`를 최소 변경으로 추가한다.
기존 필터 구조는 재배치하지 않는다.

## 7. 적용 순서와 검증

### 7.1 적용 순서

```text
1. Client/Public/EncounterPatternReference.h 생성 (UTF-8, BOM 없음)
2. Client/Private/EncounterPatternReference.cpp 생성 (UTF-8, BOM 없음)
3. Visual Studio에서 두 파일을 Client 프로젝트에 등록
4. Client/Public/MapTool.h 의 5-3-1 ~ 5-3-6 여섯 블록 반영 (CP949 유지)
5. Client/Private/MapTool.cpp 의 5-4-1 ~ 5-4-5 다섯 블록 반영 (CP949 유지)
```

`Engine/Public`을 건드리지 않으므로 `UpdateLib.bat`은 다시 돌리지 않아도 된다.

### 7.2 빌드

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug
```

Client만 빠르게 확인하려면 Visual Studio에서 `Client` 프로젝트만 다시 빌드한다.
실행 중인 `Client.exe`가 있으면 링크가 실패하므로 먼저 종료한다.

### 7.3 실행 절차

```text
Framework.slnLaunch 의 Server + Client 프로필로 실행
Lobby -> Test  (Server 승인 대기)
LEVEL::DEVELOPMENT 진입 후 F1 -> Developer Tools -> Map Tool
워크스페이스 바에서 Valtan 선택, status 가 commit 될 때까지 대기
모드 바에서 World Destruction 선택
Encounter Source 의 Reload Encounter Reference 클릭
```

### 7.4 성공 시 보이는 결과

```text
모드 바가 Map Assets | World Gameplay | World Destruction | Navigation | Camera 다섯 개
Encounter Source
  "Loaded encounter reference: ENCOUNTER_VALTAN (31 patterns)"
  Encounter ENCOUNTER_VALTAN | Boss BOSS_VALTAN | 30 Hz | 31 patterns
  VALTAN_ARMOR_BREAK_OPENING 선택 시 stage 3행
    WALL_CHARGE  WINDUP    start 0     dur 1500  start tick 0
    GROGGY       ACTIVE    start 1500  dur 5000  start tick 45
    RECOVERY     RECOVERY  start 6500  dur 1200  start tick 195
  VALTAN_ARENA_BREAK_80 선택 시 stage 4행, LANDING 의 hitShape 가 CIRCLE
Deploy Props
  Loaded placements: 85
  "Only rows with stateOffActionId" 체크 시 30행
  필터에 ITR_02306 입력 시 5행, stateOff 3705102
World Gameplay destroyable rows
  destroyable rows: 0 과 publisher fail-closed 안내
Navigation blocker regions
  Document READY, regions 0 과 "INTACT 일 때 막아야 한다" 안내
```

Bern, Character Select, Training Map을 선택하면
`This Area declares no destruction authoring source.`만 보인다.

### 7.5 실패 입력에서 기존 상태 유지 확인

```text
1. Valtan 에서 Reload 를 눌러 31 patterns 가 보이는 상태를 만든다.
2. Data/Encounters/Valtan/ValtanEncounter.json 을 복사해 백업한다.
3. 원본의 최상위에 "debug": 1 한 줄을 추가한다.
4. Reload 를 다시 누른다.
   status 가 "Encounter reference root has unexpected properties" 로 바뀌고
   pattern 표와 stage 표는 직전 31 patterns 를 그대로 유지해야 한다.
5. 백업으로 되돌리고 Reload 하면 다시 정상 문구가 나온다.
6. Map Tool 을 닫거나 Area 를 바꿀 때 Save/Discard 를 묻지 않는다.
   Has_UnsavedAuthoring() 이 이 모드로 인해 true 가 되지 않기 때문이다.
```

### 7.6 완료 보고에 분리해서 적을 것

```text
구현 완료      G1 코드 반영과 프로젝트 등록
자동 검증      Debug Client 빌드, ProjectAudit, git diff --check
수동 검증      위 7.3~7.5 실행 결과
미완료         G2 이후 전부. destroyable publisher 게이트, Server 상태,
              Shared 복제, 동적 navigation, 돌진 충돌, 파괴 Effect
```

## 8. 이 문서가 바꾸는 팀 문서

G1은 public 계약을 바꾸지 않으므로 팀 문서를 아직 갱신하지 않는다.
G5에서 publisher가 `destroyable`을 admission하면 다음 세 곳을 같은 변경 단위에서 교정한다.

```text
.md/TEAM/AREA_DATA_LAYER_GUIDE.md      4절 destroyable fail-closed 문장
.md/TEAM/UNIFIED_DATA_MANAGEMENT_ARCHITECTURE.md   14.3절과 21절 상태표
.md/TEAM/BALANCE_TOOL_OWNER_HANDOFF.md 8절의 formatVersion 2 표기
```

0절 조사에서 확인한 다음 기존 문서 오류도 해당 영역을 건드릴 때 함께 고친다.

```text
AREA_DATA_LAYER_GUIDE 2절   Valtan navigation "62x63" -> 실제 392x312
UNIFIED 13.1절              "runtime 은 첫 pattern 만 소비" -> 31 pattern 과 typed hitShape 소비
UNIFIED 21절                multi-hit/shape combat timeline "TARGET" -> Valtan 경로는 CURRENT
UNIFIED 5.1 / 14.5절        NPC "PARTIAL" -> 21절의 NPC_BEDA CURRENT 와 불일치
UNIFIED 5.2절               SpawnGroups.world.json "추가할 목표 루트" -> 이미 구현됨
```
