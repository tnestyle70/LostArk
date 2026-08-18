# 2026-08-16 발탄 아레나 바닥 붕괴(84줄 / 30줄) 계획

대상 Area `LV_LUT_HEARTRB_ED`, World `WORLD_ID::VALTAN_ARENA`, Level `LEVEL::VALTAN_ARENA`.
작성 기준 branch `codex/valtan-wall-contact-destruction-and-pillars`, HEAD `b8021b7`.

입력 인계서는 사용자가 전달한 `발탄아레나.txt`다. 이 문서는 그 인계서를 현재 코드·데이터로
다시 실측한 뒤 확정한 구현 정본이며, 인계서와 실측이 다른 항목은 §2에 근거와 함께 적었다.

기존 일반 벽 접촉 파괴 99 group과 109줄 외벽 30 group은 보존 대상이다. 이 계획의 어떤 단계도
그 두 계약을 수정하지 않는다.

---

## 0. 사용자가 승인한 결정

2026-08-16 대화에서 두 가지가 확정됐다.

1. **sector 경계는 기존 placement 그대로 사용한다.** 새로 cook할 WModel은 0개다.
   Stage A(84) = 바깥 rail 2개, Stage B(30) = 벽돌 링 4개, SAFE_CORE = wedge 4개 + 중앙 캡 1개.
2. **작업을 단계 분할한다.** G1에서 바닥 visual + Server nav NON-WALKABLE + 84 패턴 + Debug
   audition까지 닫고, 낙사(FALLING / fallVolume / protocol 버전업)는 G2로 분리한다.

G1 동안 무너진 바닥은 NON-WALKABLE이다. 즉 클릭 이동과 경로 탐색이 거부되며, 아직 낙사하지
않는다. 이 상태를 "완성"으로 기록하지 않는다.

---

## 1. G0 실측 결과

### 1.1 아레나 기준값

`Client/Bin/DataFiles/Navigation/LV_LUT_HEARTRB_ED.navgrid` 헤더는
`width=392 height=312 cellSize=0.5 originX=-6.0 originZ=-165.0`이고 walkable cell은 21,381개다.
아레나 중심 `(156.03, 22.99751, -122.06)`을 포함하는 연결 성분은 16,372 cell이며, 반경
히스토그램에서 r=14까지 밀도가 선형으로 증가하다가 r=15에서 절반, r=16부터 1/4로 떨어진다.
**즉 아레나 원판의 실제 반경은 약 15.2m다.**

### 1.2 바닥을 소유하는 placement 11개

`.mapassets`/`.mapplacements`의 삼각형을 world로 변환한 뒤 `|ny|/|n| >= 0.70`인 수평면만 남겨
반경 밴드별 면적을 측정했다. 결과는 4겹 동심 구조다.

| 층 | assetId | placement | 반경(m) | 조각당 각도 | 수평면적 |
|---|---|---|---|---|---|
| 바깥 rail | `BG_RAD_VALTAN_FLOOR01_SM` | `7000000000000000001`, `7000000000000000005` | 12.63~16.29 | 약 200° | 536 m² |
| 벽돌 링 A | `BG_RAD_VALTAN_FLOOR01A_SM` | `7000000000000000002`, `7000000000000000006` | 7.12~14.18 | 약 90° | 448 m² |
| 벽돌 링 B | `BG_RAD_VALTAN_FLOOR01B_SM` | `7000000000000000003`, `7000000000000000007` | 7.19~14.05 | 약 110° | 561 m² |
| 안쪽 wedge | `MAP_4A6CF4B84315_LV_LUT_HEARTRB_FLOOR01_SM` | SL00 export 1271 / 1299 / 1304 / 1337 | 2.48~8.28 | 약 110° | 199 m² |
| 중앙 캡 | `MAP_FBC80A02F72E_BG_LUT_WAGLOY_CIRCLEFLOOR01_SM_JJY` | `15561800956777256508` | 0.41~3.52 | 360° | 29 m² |

`70000000000000000xx` 6개는 import 원본이 아니라 editor overlay다. 근거 정본은
`Tools/LevelPlacementExtractor/heartrb_valtan_core_overlay.json`이며 `basis` 필드가
"half-width outer rail과 두 brick sector를 SL00 보스 아레나 중심에 0도/180도로 배치했고
중앙 캡은 SL00 circlefloor placement가 소유한다"고 적고 있다. 이 문서의 sector 배정은 그 근거를
그대로 따른다.

### 1.3 중복 렌더 함정은 없다

인계서 §5는 "원본 바닥이 남은 채 overlay만 숨기면 구멍이 안 보인다"를 걱정했다. 실측 결과
아레나를 덮는 큰 수평면 두 개는 이미 꺼져 있다.

- `MAP_1E5F55FC0524_LV_COMMON_MESH_CUL_BOX_8` — y=23.05 평면 707 m², r 0.28~15.63, placement `visible=0`
- `MAP_9F803C8FEF52_LV_COMMON_MESH_CUL_BOX_1` — 49 placement 전부 `visible=0`
- `MAP_9839D2BE9ACE_LV_LUT_HEARTRB_WATER01_SM` — 3 placement 전부 `visible=0`

`visible`은 `MAP_PLACEMENT_RECORD::visible`(`Client/Public/MapPlacementDocument.h:24`)이며
placement 행의 마지막 필드다. 따라서 위 11개만 실제로 그려진다.

### 1.4 현재 패턴 정본

`Data/Encounters/Valtan/ValtanEncounter.json`의 패턴은 32개다.

- `VALTAN_ARENA_BREAK_109` — `triggerHealthBar 109`, stage `TAKEOFF/DROP/IMPACT/IMPACT_HOLD/WIDE_REVEAL/RECOVERY`
- `VALTAN_ARENA_BREAK_33` — `triggerHealthBar 30`, stage `CUTSCENE/LANDING/SPIN/RECOVERY`
- **84줄 전용 패턴은 없다.** 가장 가까운 것은 `VALTAN_MAGIC_ORB_STAGGER_76`(bar 73)이며 재사용하지 않는다.

패턴 이름의 숫자는 관찰 줄 수이고 실제 trigger bar는 더 낮다(`_130`→115, `_105`→100, `_76`→73,
`_64`→62, `_33`→30, `_15`→14). 그러므로 `VALTAN_ARENA_BREAK_33`을 이름만 보고 33으로 되돌리거나
rename하지 않는다.

---

## 2. 인계서와 실측이 다른 항목

### 2.1 바닥은 Map 레이어, 파괴 시스템은 Deploy 레이어

인계서는 바닥을 WorldEvents group에 그냥 등록하면 된다고 전제했지만, 실제로는 도메인이 다르다.

- `Client/Bin/DataFiles/World/LV_LUT_HEARTRB_ED.worlddestruction.json`의 99 group
  `memberPlacementIds`는 전부 `LV_LUT_HEARTRB_ED.deployplacements`(115행)의 Deploy prop ID다.
- `Client/Public/WorldDestructionProjectionRuntime.h:20`이
  `std::vector<std::pair<uint64_t, DEPLOY_PROP_STATE>> PlacementStates`로 하드타입돼 있고
  sink는 `CDeployPropRuntime::Set_States`다.
- 반면 바닥 11개는 Map placement이며, `Client/Private/MapPlacementRuntime.cpp:423-494`에서
  asset+mirrored 단위 `CMapStaticBatchObject` 인스턴스 배치로 묶인다. 인스턴스 수 하한이 없어
  placement 2개짜리 overlay도 배치에 들어간다.

따라서 Map placement 하나를 런타임에 숨기려면 인스턴스 버퍼에 가시성 마스크를 넣어야 하고,
이는 Engine public header 변경과 `UpdateLib.bat` 전체 재빌드를 부른다.
**그래서 이 계획은 붕괴 대상 6개를 Deploy 레이어로 이관한다.** 검증된 파괴 경로를 그대로 쓰고
Engine을 건드리지 않는다.

### 2.2 nav 극성은 Server 코드가 아니라 데이터가 소유한다

인계서 §9.1은 Server에 새 극성 처리를 넣으라고 했지만 이미 구현돼 있다.

- `Server/Public/ServerNavigation.h:80` `bool bActivateWhenConditionTrue = true;`
- `Server/Private/ServerNavigation.cpp:217, 515-517, 591`이 condition 값을 이 플래그와 비교해
  blocker 활성 여부를 결정한다.
- `Server/Private/GameRoom.cpp:3070-3079`는 파괴 transition에서
  `change.bValue = (FRACTURED == eNextState || DESPAWNED == eNextState)`로 condition을 세운다.

즉 벽과 바닥의 Server 동작은 완전히 동일하고, 차이는 navblockers region의 `activateWhenTrue`
한 글자뿐이다.

```text
벽   activateWhenTrue=0  INTACT/BREAKING(condition=false) -> blocker 활성 -> 못 지나감
                         DESPAWNED(condition=true)        -> blocker 해제 -> 지나감
바닥 activateWhenTrue=1  INTACT/BREAKING(condition=false) -> blocker 해제 -> 걸을 수 있음
                         DESPAWNED(condition=true)        -> blocker 활성 -> NON-WALKABLE
```

**G1에서 Server C++ 변경은 없다.** 막는 것은 publisher의 admission 게이트뿐이다.

### 2.3 실제로 막고 있는 것은 publisher 한 곳

`Tools/WorldPipeline/Publish-ValtanWorldDestruction.ps1:798-802`가 nav region을 가진 group에
대해 `activateWhenTrue == $false` 그리고 `navPolarity -ceq 'BLOCK_WHILE_INTACT'`를 강제한다.

563행의 `WALKABLE_WHILE_INTACT`는 죽은 별칭이다. Client 파서
`Client/Private/WorldDestructionDocument.cpp:174-181`은 `BLOCK_WHILE_INTACT`와
`BLOCK_WHILE_FRACTURED`만 받으므로 그 문자열은 publish돼도 Client 로드에서 실패한다.
이 계획은 563행에서 `WALKABLE_WHILE_INTACT`를 제거하고 `BLOCK_WHILE_FRACTURED`를 넣는다.

### 2.4 STATIC deploy asset의 fractured 모델 강제

`Client/Private/DeployPropCatalog.cpp:116-117`의 `requiresFractured = (kind == STATIC)` 때문에
STATIC deploy asset은 fractured `.wmodel`이 디스크에 실제로 있어야 한다. 바닥은 fractured가 없고
최종 상태가 `DESPAWNED`라 fractured를 그릴 일이 없다.

렌더 경로는 이미 방어적이다. `Client/Private/DeployPropObject.cpp:233-234, 296-297`이
`nullptr != m_pFracturedModelCom`일 때만 fractured를 쓰고 아니면 intact로 폴백한다.
그러므로 검증 게이트 두 곳만 완화하면 되고 렌더 코드는 그대로다.

### 2.5 낙사는 존재하지 않는다

`Server/`와 `Shared/` 전체에서 `FALLING`, `fallVolume`, `fallSequence` grep 결과가 0건이다.
과거 PLAN에 설계가 적혀 있어도 구현은 없다. G2 범위다.

---

## 3. 확정한 sector 배정과 nav cell

### 3.1 배정 규칙

한 cell을 여러 층이 덮을 때 **마지막까지 그 cell을 받쳐 주는 층이 소유자**다. 사라지는 순서는
Stage A(84) → Stage B(30) → 영구 보존이므로 우선순위는 다음과 같다.

```text
core(wedge/캡)가 덮으면        -> 어떤 collapse region에도 넣지 않는다
아니고 brick이 덮으면          -> STAGE_B
아니고 rail만 덮으면           -> STAGE_A
```

같은 층의 placement 두 개가 이음매에서 겹치면 `runtimePlacementId`가 작은 쪽이 가진다.
포인터나 순회 순서가 아니라 저장 ID로 결정하므로 재실행해도 결과가 같다.

### 3.2 이음매 봉합

위 규칙만 적용하면 r 8~11 구간에 소유자 없는 walkable cell 17개가 남는다. Stage B가 무너진 뒤
허공에 뜬 섬이 되므로 결정적 규칙으로 흡수한다.

> 소유자 없는 base-walkable cell이 반경 8.60m 이상 16.6m 이하이고, 4-이웃 walkable cell 중
> collapse region 밖(안전 코어 또는 아레나 외부)으로 나가는 것이 하나도 없으면,
> 이웃 중 최다 소유자의 region에 흡수한다. 변화가 없을 때까지 반복한다.

이 봉합으로 82 cell이 흡수된다.

### 3.3 최종 수치

| runtimePlacementId | stage | cell |
|---|---|---|
| `7000000000000000001` | STAGE_A | 347 |
| `7000000000000000005` | STAGE_A | 325 |
| `7000000000000000002` | STAGE_B | 365 |
| `7000000000000000003` | STAGE_B | 427 |
| `7000000000000000006` | STAGE_B | 360 |
| `7000000000000000007` | STAGE_B | 425 |
| 합계 | | 2,249 |

- Stage A 672 cell, Stage B 1,577 cell, **교집합 0**
- 2,249 cell 전부 base navgrid가 walkable(비walkable 혼입 0)
- 두 단계가 모두 무너진 뒤 r<=16.6 안에 남는 walkable cell 817개
  = 안전 코어 780(r 0~8) + 입구 연결 ledge 37(r=16)

붕괴 후 아레나 반경은 **15.2m → 약 14.2m → 약 8.3m**로 좁아진다.

---

## 4. G1 작업 단위

### G1-1. Deploy catalog의 fractured 강제 완화

바닥 deploy asset이 fractured 없이 등록될 수 있어야 한다.

**파일:** `Client/Private/DeployPropCatalog.cpp`
**작업:** 교체
**기준점:** `const bool_t requiresFractured =` 로 시작해 `return false;` 와 닫는 `}` 까지의 검증 블록(현재 116-136행)
**필요한 이유:** 최종 상태가 `DESPAWNED`인 바닥 prop은 fractured 모델이 없다. 같은 `.wmodel`을
두 번째 prototype tag로 중복 등록하면 1.6MB 메시가 3배로 파싱·상주한다.
**연결되는 부분:** `CDeployPropObject::Initialize`, `Ready_Components`

교체할 블록:

```cpp
		// A STATIC prop may omit its fractured model when the authored mutation
		// ends at DESPAWNED. The pair stays all-or-nothing so a half-declared
		// row can never reach the runtime.
		const bool_t declaresFractured =
			!fracturedPath.empty() || !fracturedPrototype.empty();
		const bool_t requiresFractured =
			entry.kind == DEPLOY_PROP_MODEL_KIND::STATIC && declaresFractured;
		if (entry.id.empty() || entry.label.empty() || entry.evidence.empty() ||
			entry.intactRelativePath.is_absolute() ||
			entry.intactRelativePath.extension() != L".wmodel" ||
			entry.intactPrototypeTag.empty() ||
			!IsInsideRoot(assetRoot, entry.intactResolvedPath) ||
			!std::filesystem::is_regular_file(entry.intactResolvedPath) ||
			(requiresFractured && (entry.fracturedRelativePath.is_absolute() ||
				entry.fracturedRelativePath.extension() != L".wmodel" ||
				entry.fracturedPrototypeTag.empty() ||
				!IsInsideRoot(assetRoot, entry.fracturedResolvedPath) ||
				!std::filesystem::is_regular_file(entry.fracturedResolvedPath))) ||
			(declaresFractured && (fracturedPath.empty() || fracturedPrototype.empty())) ||
			(entry.kind == DEPLOY_PROP_MODEL_KIND::ANIM && declaresFractured) ||
			!assetIds.insert(entry.id).second ||
			!prototypeTags.insert(entry.intactPrototypeTag).second ||
			(requiresFractured &&
				!prototypeTags.insert(entry.fracturedPrototypeTag).second))
		{
			m_Status = "DeployProp catalog validation failed for " + entry.id;
			return false;
		}
```

불변식: ANIM은 예전처럼 fractured를 가질 수 없고, STATIC은 fractured 쌍을 다 갖거나 다 비운다.
한쪽만 있는 행은 계속 거부된다.

**파일:** `Client/Private/DeployPropObject.cpp`
**작업:** 교체
**기준점:** `CDeployPropObject::Initialize`의 `if (desc.prototypeLevelIndex >= ETOUI(LEVEL::END) ||` 조건식(현재 87-94행)
**필요한 이유:** 카탈로그가 허용한 fractured 없는 STATIC을 object가 다시 거부하면 안 된다.

```cpp
	if (desc.prototypeLevelIndex >= ETOUI(LEVEL::END) ||
		0 == desc.placement.runtimePlacementId ||
		desc.placement.assetId.empty() || desc.intactPrototypeTag.empty() ||
		!std::isfinite(desc.placement.uniformScale) ||
		desc.placement.uniformScale <= 0.000001f)
		return E_FAIL;
```

**파일:** `Client/Private/DeployPropObject.cpp`
**작업:** 교체
**기준점:** `Ready_Components`의 `if (desc.modelKind == DEPLOY_PROP_MODEL_KIND::STATIC &&` 블록(현재 776-780행)

```cpp
	if (desc.modelKind == DEPLOY_PROP_MODEL_KIND::STATIC &&
		!desc.fracturedPrototypeTag.empty() &&
		FAILED(__super::Add_Component(
			desc.prototypeLevelIndex, desc.fracturedPrototypeTag,
			TEXT("Com_Model_Fractured"), m_pFracturedModelCom)))
		return E_FAIL;
```

`m_pFracturedModelCom`이 `nullptr`로 남는 경로는 이미 233-234행과 296-297행이 intact 폴백으로
처리한다. 새 분기를 추가하지 않는다.

### G1-2. 바닥 sector 생성 도구

6개 sector의 deploy asset/placement, WorldEvents group/mutation/binding, navblockers region을
한 번에 결정적으로 만들고 Map placement 6행을 제거한다. 99 벽 분리 때 만든
`Tools/WorldPipeline/Split-ValtanIndependentWallGroups.ps1`과 같은 역할이며, 기하 계산이
필요하므로 `Tools/LevelPlacementExtractor/build_valtan_navgrid.py` 옆에 Python으로 둔다.

새 파일 `Tools/LevelPlacementExtractor/build_valtan_floor_collapse.py`의 전체 코드는 §7에 있다.

산출물과 그 도구가 소유하는 이유:

| 산출물 | 이유 |
|---|---|
| `Data/Maps/Imported/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.deployassets` 3행 추가 | Deploy 카탈로그 정본이 Imported 한 곳뿐이다. `BG_RAD_VALTAN_*`가 이미 Imported `.mapassets`에 project overlay로 들어가 있는 선례를 따른다. |
| `Data/Maps/Authoring/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.deployplacements` 6행 추가 | placement instance는 Authoring 소유다. |
| `Data/Maps/Authoring/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.mapplacements` 6행 제거 | 같은 기하가 Map과 Deploy에 동시에 있으면 이중 렌더된다. LFS 파일이므로 이 도구만 건드린다. |
| `Data/Encounters/Valtan/ValtanWorldEvents.json` group/mutation/binding 6+6+6 | 파괴 graph 정본. |
| `Data/Navigation/LV_LUT_HEARTRB_ED.navblockers` region 6개 추가 | `activateWhenTrue=1`로 바닥 극성을 표현한다. |

다섯 파일 전부 staging에 쓴 뒤 전부 성공했을 때만 교체한다. 중간 실패는 원본 byte 그대로 남긴다.

### G1-3. 안정 ID 계약

```text
deploy assetId      VALTAN_FLOOR_RAIL / VALTAN_FLOOR_BRICK_A / VALTAN_FLOOR_BRICK_B
deploy placementId  Map overlay의 runtimePlacementId를 그대로 승계 (7000000000000000001 등)
groupId             destroyable.group.valtan.floor84.rail.<placementId>
                    destroyable.group.valtan.floor30.brick.<placementId>
mutationId          mutation.valtan.floor84.rail.<placementId>.collapse
                    mutation.valtan.floor30.brick.<placementId>.collapse
bindingId           binding.valtan.floor84.rail.<placementId>
                    binding.valtan.floor30.brick.<placementId>
navregionId         navregion.valtan.floor84.rail.<placementId>
                    navregion.valtan.floor30.brick.<placementId>
conditionId         condition.valtan.floor84.rail.<placementId>.collapsed
                    condition.valtan.floor30.brick.<placementId>.collapsed
```

placement ID를 승계하는 이유는 두 가지다. Map overlay와 Deploy 사이에서 같은 기하가 같은 ID를
유지하므로 추적이 쉽고, vector index나 새 번호 발급 규칙을 만들 필요가 없다.
Deploy placement ID 공간과 충돌하지 않는 것은 도구가 publish 전에 검사한다.

### G1-4. WorldEvents 데이터

group 6개는 다음 형태다. `navPolarity`만 기존 99 벽과 다르다.

```json
{
  "groupId": "destroyable.group.valtan.floor84.rail.7000000000000000001",
  "memberPlacementIds": ["7000000000000000001"],
  "navigationRegionIds": ["navregion.valtan.floor84.rail.7000000000000000001"],
  "navPolarity": "BLOCK_WHILE_FRACTURED",
  "initialState": "INTACT"
}
```

mutation 6개:

```json
{
  "mutationId": "mutation.valtan.floor84.rail.7000000000000000001.collapse",
  "groupId": "destroyable.group.valtan.floor84.rail.7000000000000000001",
  "targetState": "DESPAWNED",
  "breakingDurationMs": 250,
  "receiverCollisionId": "",
  "navigationStateId": "condition.valtan.floor84.rail.7000000000000000001.collapsed"
}
```

`targetState`는 반드시 `DESPAWNED`다. `FRACTURED`로 두면 fractured 모델이 없는 prop이
intact로 폴백해 바닥이 계속 보인다. publisher가 이를 강제한다(§G1-5).

binding 6개는 `STAGE_ENTER`, `offsetMs=0`, `enabled=true`다.

```json
{
  "bindingId": "binding.valtan.floor84.rail.7000000000000000001",
  "mutationId": "mutation.valtan.floor84.rail.7000000000000000001.collapse",
  "patternId": "VALTAN_ARENA_BREAK_84",
  "stageId": "IMPACT",
  "triggerKind": "STAGE_ENTER",
  "offsetMs": 0,
  "receiverCollisionId": "",
  "enabled": true
}
```

Stage B 4개는 `patternId: "VALTAN_ARENA_BREAK_33"`, `stageId: "LANDING"`을 쓴다.

**바닥에는 `COLLIDER_CONTACT` binding을 만들지 않는다.** 몸통이나 도끼가 바닥에 닿았다고
무너지면 안 된다.

### G1-5. publisher 확장

**파일:** `Tools/WorldPipeline/Publish-ValtanWorldDestruction.ps1`
**작업:** 교체
**기준점:** 563행 `$group.navPolarity -cnotin @('BLOCK_WHILE_INTACT','WALKABLE_WHILE_INTACT')`

```powershell
            $group.navPolarity -cnotin @('BLOCK_WHILE_INTACT','BLOCK_WHILE_FRACTURED')) {
```

`WALKABLE_WHILE_INTACT`는 어떤 소비자도 파싱하지 않는 죽은 문자열이므로 제거한다.

**작업:** 교체
**기준점:** 798-802행의 nav region 극성 검사

```powershell
            $navRegion = $NavigationBlockers.Regions[$navRegionId]
            $expectsActivateWhenTrue =
                $group.navPolarity -ceq 'BLOCK_WHILE_FRACTURED'
            if ([uint32]$navRegion.CellCount -eq 0 -or
                $navRegion.ActivateWhenTrue -ne $expectsActivateWhenTrue) {
                throw "Destruction navigation polarity is invalid: $navRegionId"
            }
            $navigationStateId = [string]$navRegion.ConditionId
```

즉 벽은 `activateWhenTrue=false`, 바닥은 `activateWhenTrue=true`와만 결합한다. 섞이면 실패한다.

이어서 바닥 전용 검증을 추가한다. 삽입 위치는 group 순회가 끝난 뒤, 즉 위 블록을 포함하는
`foreach`가 닫힌 직후다.

```powershell
    # Floor collapse rules. The arena floor is the opposite polarity of a wall,
    # so a mistake here silently produces a hole you can still walk on or a
    # sector that despawns while the Server keeps routing paths across it.
    $floorGroupIds = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    $stageCells = @{ 'floor84' = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
                     'floor30' = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal) }
    foreach ($group in @($Source.groups)) {
        if ($group.navPolarity -cne 'BLOCK_WHILE_FRACTURED') { continue }
        $groupId = [string]$group.groupId
        [void]$floorGroupIds.Add($groupId)
        if ($groupId -cnotmatch '^destroyable\.group\.valtan\.(floor84|floor30)\.') {
            throw "Floor collapse group uses an unknown stage prefix: $groupId"
        }
        $stageKey = $Matches[1]
        if (@($group.navigationRegionIds).Count -ne 1) {
            throw "Floor collapse group must own exactly one navigation region: $groupId"
        }
        $region = $NavigationBlockers.Regions[[string]$group.navigationRegionIds[0]]
        foreach ($cell in @($region.Cells)) {
            $cellKey = "$($cell.X),$($cell.Z)"
            if (-not $stageCells[$stageKey].Add($cellKey)) {
                throw "Floor collapse stage claims a duplicate navigation cell: $groupId $cellKey"
            }
            if (-not (Test-BaseWalkableCell $cell.X $cell.Z)) {
                throw "Floor collapse region contains a base non-walkable cell: $groupId $cellKey"
            }
        }
    }
    $sharedCells = @($stageCells['floor84'] | Where-Object { $stageCells['floor30'].Contains($_) })
    if ($sharedCells.Count -gt 0) {
        throw "Floor stage A and stage B share $($sharedCells.Count) navigation cells."
    }
    foreach ($mutation in @($Source.mutations)) {
        if (-not $floorGroupIds.Contains([string]$mutation.groupId)) { continue }
        if ($mutation.targetState -cne 'DESPAWNED') {
            throw "Floor collapse mutation must end at DESPAWNED: $($mutation.mutationId)"
        }
        if (-not [string]::IsNullOrEmpty([string]$mutation.receiverCollisionId)) {
            throw "Floor collapse mutation must not own a collision receiver: $($mutation.mutationId)"
        }
    }
    foreach ($binding in @($Source.bindings)) {
        if (-not $floorGroupIds.Contains([string]$mutationGroupById[[string]$binding.mutationId])) { continue }
        if ($binding.triggerKind -cne 'STAGE_ENTER' -or [int]$binding.offsetMs -ne 0) {
            throw "Floor collapse binding must be STAGE_ENTER at offset 0: $($binding.bindingId)"
        }
    }
```

`Test-BaseWalkableCell`은 새 helper다. `Client/Bin/DataFiles/Navigation/LV_LUT_HEARTRB_ED.navgrid`를
한 번 읽어 walkable byte를 조회한다. 삽입 위치는 `Get-DeployPlacementPositions` 정의 바로 아래다.

```powershell
# Floor collapse regions may only paint cells that the baked navgrid already
# marks walkable. A cell that starts blocked would make the dynamic polarity
# meaningless and hide an authoring mistake behind a correct-looking publish.
function Test-BaseWalkableCell {
    param([int]$X, [int]$Z)
    if ($null -eq $script:BaseNavGrid) {
        $resolved = Resolve-RepoPath $BaseNavGridPath
        if (-not [IO.File]::Exists($resolved)) {
            throw "Required base navigation grid is missing: $BaseNavGridPath"
        }
        $bytes = [IO.File]::ReadAllBytes($resolved)
        if ($bytes.Length -lt 20) { throw 'Base navigation grid header is truncated.' }
        $script:BaseNavGrid = [pscustomobject]@{
            Width  = [BitConverter]::ToUInt32($bytes, 0)
            Height = [BitConverter]::ToUInt32($bytes, 4)
            Bytes  = $bytes
        }
    }
    $grid = $script:BaseNavGrid
    if ($X -lt 0 -or $Z -lt 0 -or $X -ge $grid.Width -or $Z -ge $grid.Height) { return $false }
    return $grid.Bytes[20 + $Z * $grid.Width + $X] -eq 1
}
```

`$BaseNavGridPath` 파라미터는 `param()` 블록의 `$DeployPlacementsPath` 바로 아래에 추가한다.

```powershell
    [string]$BaseNavGridPath = 'Client/Bin/DataFiles/Navigation/LV_LUT_HEARTRB_ED.navgrid',
```

기존 109 외벽 보호 검사(`Protect every 109 outer wall from ordinary collider contact`)와
`isProductCandidate` 경로는 건드리지 않는다.

### G1-6. 84줄 패턴

**파일:** `Data/Encounters/Valtan/ValtanEncounter.json`
**작업:** 추가
**위치:** `VALTAN_ARENA_BREAK_109` 객체의 닫는 `}` 바로 뒤, `VALTAN_MAGIC_ORB_STAGGER_76` 바로 앞

```json
{
  "patternId": "VALTAN_ARENA_BREAK_84",
  "displayName": "84줄 외곽 바닥 붕괴",
  "actionId": "valtan.mechanic.arena-floor-84",
  "sourceActionIds": [],
  "selectionMode": "HEALTH_BAR",
  "minimumHealthBar": 0,
  "maximumHealthBar": 0,
  "triggerHealthBar": 84,
  "triggerOrder": 1,
  "selectionWeight": 0,
  "maximumConsecutiveUses": 0,
  "minimumRange": 0.0,
  "maximumRange": 100.0,
  "stages": [
    {
      "stageId": "WINDUP",
      "actionId": "valtan.mechanic.arena-floor-84.windup",
      "stageKind": "WINDUP",
      "durationMs": 900,
      "hitShape": "NONE",
      "hitOuterRadius": 0.0,
      "hitInnerRadius": 0.0,
      "hitAngleDegrees": 0.0,
      "hitLength": 0.0,
      "hitHalfWidth": 0.0,
      "hitCount": 0,
      "hitIntervalMs": 0,
      "serverDamageProfileId": ""
    },
    {
      "stageId": "IMPACT",
      "actionId": "valtan.mechanic.arena-floor-84.impact",
      "stageKind": "ACTIVE",
      "durationMs": 500,
      "hitShape": "NONE",
      "hitOuterRadius": 0.0,
      "hitInnerRadius": 0.0,
      "hitAngleDegrees": 0.0,
      "hitLength": 0.0,
      "hitHalfWidth": 0.0,
      "hitCount": 0,
      "hitIntervalMs": 0,
      "serverDamageProfileId": ""
    },
    {
      "stageId": "RECOVERY",
      "actionId": "valtan.mechanic.arena-floor-84.recovery",
      "stageKind": "RECOVERY",
      "durationMs": 1200,
      "hitShape": "NONE",
      "hitOuterRadius": 0.0,
      "hitInnerRadius": 0.0,
      "hitAngleDegrees": 0.0,
      "hitLength": 0.0,
      "hitHalfWidth": 0.0,
      "hitCount": 0,
      "hitIntervalMs": 0,
      "serverDamageProfileId": ""
    }
  ]
}
```

**provenance 경고.** `sourceActionIds`가 비어 있고 stage 지속시간 900/500/1200 ms는 원작 클립
근거가 없는 `PROJECT_TUNED` 잠정값이다. `VALTAN_ARENA_BREAK_109`가 실제 source action `420629`를
가진 것과 다르다. 실제 클립이 확인되면 duration과 actionId를 같은 변경 단위에서 교정한다.
RESULT에도 이 사실을 분리해 기록한다. `serverDamageProfileId`는 전부 비어 있어 이 패턴은
데미지를 만들지 않는다. 바닥만 무너진다.

`Data/Balance/Reference/Official/2026-08-05.balance-provenance.receipt.json`에 이 패턴의
저작 field를 `PROJECT_TUNED`로 등록해야 `Publish-GameplayBalance.ps1 -Mode Validate`가 통과한다.

### G1-7. Debug audition 패널

**파일:** `Client/Private/Level_ValtanArena.cpp`
**작업:** 추가
**기준점:** 기존 `Reset + Play 109 Only` 버튼을 만드는 블록 바로 아래
**필요한 이유:** 사용자가 84 → 30 순서를 화면에서 직접 확인해야 한다.
**연결되는 부분:** 기존 audition operation latch와 Server typed request

추가 버튼 3개와 진단은 기존 `Reset + Play Selected` 경로를 그대로 쓴다. Client가 group state,
nav blocker, 바닥 가시성을 직접 바꾸지 않는다. 버튼은 요청만 걸고 Server snapshot과 destruction
delta가 돌아온 결과를 표시한다.

```text
Reset + Play 84 (Floor Stage A)
Reset + Play 30 (Floor Stage B)
Reset + Play Arena Shrink Timeline (109 -> 84 -> 30)
```

패널 진단에 다음을 추가한다. 값은 전부 Server가 보낸 `WORLD_DESTRUCTION_RUNTIME_DIAGNOSTICS`와
projection group state에서 읽고 Client가 역추론하지 않는다.

```text
Floor Stage A : INTACT / BREAKING / GONE  (expected 2)
Floor Stage B : INTACT / BREAKING / GONE  (expected 4)
active nav blocker region count
navigation revision
```

`Arena Shrink Timeline`은 기존 `ENVIRONMENT_TIMELINE_STEP` 구조를 재사용한다. 각 패턴이 실제로
시작하고 끝난 것을 Server snapshot에서 확인한 뒤 다음으로 넘어가며, 고정 sleep을 쓰지 않는다.

### G1-8. Server contract test

**파일:** `Server/Private/WorldDestructionBootstrapContractTests.cpp`

추가할 검사:

```text
Load six independent Valtan floor collapse sectors
Keep every floor sector walkable while it is INTACT and BREAKING
Block exactly the collapsed sector cells at the DESPAWNED commit tick
Reject a floor group whose navigation region activates on a false condition
Reject a floor mutation that ends at FRACTURED
Reject a collider contact binding that targets a floor group
Restore every floor sector on the room-empty reset and bump the epoch
```

`Server/Private/ServerGameplayContractTests.cpp`에는 기존 109 검사 옆에 다음을 추가한다.

```text
Leave every floor sector INTACT when the 109 outer ring collapses
Collapse only stage A at the 84 impact and only stage B at the 30 landing
```

---

## 5. G2 개요 (이번 범위 아님)

낙사는 다음을 한 수직 슬라이스로 닫아야 한다. G1이 끝난 뒤 별도 PLAN으로 상세화한다.

1. `Data/Worlds/LV_LUT_HEARTRB_ED/Gameplay.world.json`에 typed `fallVolume` kind 추가와
   `Publish-WorldGameplay.ps1` schema/bootstrap 확장
2. `SERVER_PLAYER`에 `fallSequence`, `fallStartTick`, 시작 위치·초기 속도·중력·`deathTick`,
   원인 `fallVolumeId` 추가
3. FALLING 동안 이동/스킬/타겟/damage/trigger 차단, `deathTick`에서 DEAD 전환, 기존 revive 경로 복구
4. collapse commit tick에 footprint 안에 서 있는 player를 같은 transaction에서 FALLING 시작
5. 이후 knockback/root-motion의 swept segment가 active fallVolume에 들어가면 tunneling 없이 FALLING
6. `PLAYER_SNAPSHOT` layout 변경 → `NETWORK_PROTOCOL_VERSION` 21 → 22,
   `PacketMessages` Writer/Reader와 `NetworkProtocolHarness` 동반 수정
7. Client는 `fallSequence`로만 낙하 표현을 시작하고 late join은 serverTick age로 seek

프로토콜을 올리면 Server와 Client를 **함께** 다시 빌드해야 한다. 한쪽만 갱신하면 frame이 거부된다.

---

## 6. 검증 계획

### 자동

```text
1. python Tools/LevelPlacementExtractor/build_valtan_floor_collapse.py --mode Validate
2. Publish-ValtanWorldDestruction.ps1 -Mode Validate / -Mode ContractTest
3. Publish-ServerNavigation.ps1  -Mode Validate / -Mode ContractTest
4. Publish-WorldGameplay.ps1     -Mode Validate
5. Publish-GameplayBalance.ps1   -Mode Validate   (84 패턴 provenance 포함)
6. Shared + NetworkProtocolHarness x64 Debug 빌드·실행, failures 0
7. Server x64 Debug 빌드, Server.exe --contract-test failures 0
8. Client x64 Debug 빌드
9. 변경 JSON 전체 parse, git diff --check
```

G1은 Engine과 Shared를 바꾸지 않으므로 `UpdateLib.bat`과 protocol 버전업은 필요 없다.
다만 `NetworkProtocolHarness`는 회귀 확인용으로 계속 돌린다.

기대 수치:

```text
Publish-ValtanWorldDestruction  groups=105  (99 + 6)
                                bindings=117 (111 + 6)
                                floor84=2 sectors / 672 cells
                                floor30=4 sectors / 1577 cells
Publish-ServerNavigation        regions=103 (97 + 6)
Publish-GameplayBalance         boss patterns=33 (32 + 1)
Deploy placements               121 (115 + 6)
Map placements                  13186 (13192 - 6)
```

### 사용자 수동 (에이전트가 대신 판정하지 않음)

```text
1. Sync-TeamLanEndpoint.ps1 출력 확인, Server listening 확인
2. Lobby -> Valtan 진입
3. F1 -> Valtan Pattern Audition -> Reset Arena State
4. 시작 화면에서 바닥에 구멍/z-fighting/사라진 조각이 없는지
5. Reset + Play 109 Only  -> 외벽 30개만 사라지고 바닥은 전부 남는지
6. Reset + Play 84        -> 바깥 테두리 링만 사라지는지, 그쪽 클릭 이동이 거부되는지
7. Reset + Play 30        -> 벽돌 링이 사라지고 중앙 코어만 남는지
8. Reset + Play Arena Shrink Timeline -> 109 -> 84 -> 30이 누적되는지
9. Lobby로 나갔다 재진입 -> 현재 구멍 상태가 그대로 보이고 파편/카메라가 재생되지 않는지
10. room empty 후 새 전투 -> 바닥이 전부 복구되는지
```

6~7번에서 "무너진 자리로 걸어가면 낙사"는 G1 범위가 아니다. G1에서는 이동이 거부되기만 한다.

---

## 7. 새 파일 전체 코드

### `Tools/LevelPlacementExtractor/build_valtan_floor_collapse.py`

파일 역할: Valtan 아레나 바닥 6 sector의 파괴 계약을 한 트랜잭션으로 생성한다. 기하 실측
(어느 placement가 어느 navgrid cell을 받치는가)이 필요하고 그 계산이 결정적이어야 하므로
publisher가 아니라 authoring 생성 도구로 둔다. 생성한 다섯 파일은 이후 기존 publisher가
검증·publish한다.

소유하는 상태: 없다. 매 실행마다 저장소 현재 상태에서 다시 계산하고, 같은 입력이면 같은
바이트를 낸다.

`--mode Validate`는 계산만 하고 파일을 쓰지 않으며 기대 수치를 출력한다.
`--mode Apply`는 staging에 쓴 뒤 다섯 파일을 전부 교체한다. 하나라도 실패하면 원본을 유지한다.

```python
"""Build the Valtan arena floor collapse contract for LV_LUT_HEARTRB_ED.

The arena floor is four concentric layers of map placements. This tool moves the
six collapsing ring placements into the deploy prop layer, where the existing
world destruction runtime already owns INTACT/BREAKING/DESPAWNED state, and
derives the navigation blocker regions from the real floor geometry instead of
from hand-typed coordinates.

Cell ownership rule: a navgrid cell belongs to the last floor layer that still
supports it. The safe core therefore never appears in a collapse region, and
stage A and stage B are disjoint by construction.
"""

from __future__ import annotations

import argparse
import collections
import io
import json
import math
import os
import shlex
import struct
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(REPO_ROOT / "Tools" / "ModelAssetConverter"))

import cook_wmodel_geometry_contract as contract  # noqa: E402

AREA_ID = "LV_LUT_HEARTRB_ED"
ARENA_CENTRE = (156.03, -122.06)
FLOOR_Y_RANGE = (21.0, 24.5)
UP_FACING_MINIMUM = 0.70
ASSET_PRETRANSFORM = 0.01
CORE_RADIUS = 8.60
ARENA_RADIUS = 16.6

MAP_ASSETS = REPO_ROOT / "Client/Bin/DataFiles/Map" / f"{AREA_ID}.mapassets"
MAP_PLACEMENTS_RUNTIME = REPO_ROOT / "Client/Bin/DataFiles/Map" / f"{AREA_ID}.mapplacements"
MAP_PLACEMENTS_SOURCE = REPO_ROOT / "Data/Maps/Authoring" / AREA_ID / f"{AREA_ID}.mapplacements"
DEPLOY_ASSETS_SOURCE = REPO_ROOT / "Data/Maps/Imported" / AREA_ID / f"{AREA_ID}.deployassets"
DEPLOY_PLACEMENTS_SOURCE = REPO_ROOT / "Data/Maps/Authoring" / AREA_ID / f"{AREA_ID}.deployplacements"
WORLD_EVENTS = REPO_ROOT / "Data/Encounters/Valtan/ValtanWorldEvents.json"
NAV_BLOCKERS = REPO_ROOT / "Data/Navigation" / f"{AREA_ID}.navblockers"
NAV_GRID = REPO_ROOT / "Client/Bin/DataFiles/Navigation" / f"{AREA_ID}.navgrid"
RESOURCE_ROOT = REPO_ROOT / "Client/Bin/Resources"

# layer name, collapse order (0 = never collapses)
FLOOR_LAYERS = {
    "MAP_FBC80A02F72E_BG_LUT_WAGLOY_CIRCLEFLOOR01_SM_JJY": ("CORE", 0),
    "MAP_4A6CF4B84315_LV_LUT_HEARTRB_FLOOR01_SM": ("CORE", 0),
    "BG_RAD_VALTAN_FLOOR01A_SM": ("STAGE_B", 1),
    "BG_RAD_VALTAN_FLOOR01B_SM": ("STAGE_B", 1),
    "BG_RAD_VALTAN_FLOOR01_SM": ("STAGE_A", 2),
}

STAGE_DEPLOY_ASSET = {
    "BG_RAD_VALTAN_FLOOR01_SM": "VALTAN_FLOOR_RAIL",
    "BG_RAD_VALTAN_FLOOR01A_SM": "VALTAN_FLOOR_BRICK_A",
    "BG_RAD_VALTAN_FLOOR01B_SM": "VALTAN_FLOOR_BRICK_B",
}

STAGE_KEY = {"STAGE_A": "floor84", "STAGE_B": "floor30"}
STAGE_SLUG = {"STAGE_A": "rail", "STAGE_B": "brick"}
STAGE_PATTERN = {
    "STAGE_A": ("VALTAN_ARENA_BREAK_84", "IMPACT"),
    "STAGE_B": ("VALTAN_ARENA_BREAK_33", "LANDING"),
}

EXPECTED_SECTOR_CELLS = {
    "7000000000000000001": 347,
    "7000000000000000005": 325,
    "7000000000000000002": 365,
    "7000000000000000003": 427,
    "7000000000000000006": 360,
    "7000000000000000007": 425,
}


class BuildError(RuntimeError):
    pass


def read_text_lines(path: Path) -> list[str]:
    with io.open(path, "r", encoding="utf-8") as handle:
        return handle.read().split("\n")


def split_row(line: str) -> list[str]:
    return shlex.split(line)


def load_map_assets() -> dict[str, str]:
    lines = read_text_lines(MAP_ASSETS)
    assets: dict[str, str] = {}
    for line in lines[1:]:
        if not line.strip():
            continue
        tokens = split_row(line)
        if len(tokens) >= 4:
            assets[tokens[0]] = tokens[2]
    return assets


def load_floor_placements() -> list[dict]:
    rows: list[dict] = []
    for line in read_text_lines(MAP_PLACEMENTS_RUNTIME)[1:]:
        if not line.strip():
            continue
        tokens = split_row(line)
        if len(tokens) < 16 or tokens[4] not in FLOOR_LAYERS:
            continue
        x, y, z = float(tokens[5]), float(tokens[6]), float(tokens[7])
        if math.hypot(x - ARENA_CENTRE[0], z - ARENA_CENTRE[1]) > 22.0:
            continue
        rows.append(
            {
                "placementId": tokens[0],
                "sourcePlacementId": tokens[1],
                "assetId": tokens[4],
                "position": (x, y, z),
                "rotation": (
                    float(tokens[8]),
                    float(tokens[9]),
                    float(tokens[10]),
                    float(tokens[11]),
                ),
                "scale": (float(tokens[12]), float(tokens[13]), float(tokens[14])),
                "visible": tokens[15],
            }
        )
    return rows


_MESH_CACHE: dict[str, list] = {}


def load_up_facing_triangles(asset_id: str, asset_paths: dict[str, str]) -> list:
    if asset_id in _MESH_CACHE:
        return _MESH_CACHE[asset_id]
    relative = asset_paths.get(asset_id)
    if relative is None:
        raise BuildError(f"Floor asset is missing from the map catalog: {asset_id}")
    resolved = RESOURCE_ROOT / Path(relative)
    if not resolved.is_file():
        raise BuildError(f"Floor asset payload is missing from Resources: {relative}")
    _header, _sections, submeshes = contract.parse_legacy_wmodel(resolved.read_bytes())
    triangles = []
    for submesh in submeshes:
        vertices, indices = submesh.vertices, submesh.indices
        for base in range(0, len(indices) - 2, 3):
            a = vertices[indices[base]]
            b = vertices[indices[base + 1]]
            c = vertices[indices[base + 2]]
            triangles.append(
                ((a[0], a[1], a[2]), (b[0], b[1], b[2]), (c[0], c[1], c[2]))
            )
    _MESH_CACHE[asset_id] = triangles
    return triangles


def rotation_rows(qx: float, qy: float, qz: float, qw: float):
    length = math.sqrt(qx * qx + qy * qy + qz * qz + qw * qw)
    if length <= 0.0:
        raise BuildError("Placement rotation quaternion is degenerate.")
    qx, qy, qz, qw = qx / length, qy / length, qz / length, qw / length
    return (
        (1 - 2 * (qy * qy + qz * qz), 2 * (qx * qy - qz * qw), 2 * (qx * qz + qy * qw)),
        (2 * (qx * qy + qz * qw), 1 - 2 * (qx * qx + qz * qz), 2 * (qy * qz - qx * qw)),
        (2 * (qx * qz - qy * qw), 2 * (qy * qz + qx * qw), 1 - 2 * (qx * qx + qy * qy)),
    )


class NavGrid:
    def __init__(self, path: Path) -> None:
        data = path.read_bytes()
        if len(data) < 20:
            raise BuildError("Navigation grid header is truncated.")
        self.width, self.height = struct.unpack_from("<II", data, 0)
        self.cell_size, self.origin_x, self.origin_z = struct.unpack_from("<fff", data, 8)
        expected = 20 + self.width * self.height * 5
        if len(data) != expected:
            raise BuildError("Navigation grid size does not match its header.")
        self.walkable = data[20 : 20 + self.width * self.height]

    def index(self, cx: int, cz: int) -> int:
        return cz * self.width + cx

    def is_walkable(self, cx: int, cz: int) -> bool:
        if cx < 0 or cz < 0 or cx >= self.width or cz >= self.height:
            return False
        return self.walkable[self.index(cx, cz)] == 1

    def centre(self, cx: int, cz: int) -> tuple[float, float]:
        return (
            self.origin_x + (cx + 0.5) * self.cell_size,
            self.origin_z + (cz + 0.5) * self.cell_size,
        )


def rasterise_coverage(placements, asset_paths, grid: NavGrid):
    """cell index -> set of placement ids whose up-facing floor covers it."""
    coverage: dict[int, set[str]] = collections.defaultdict(set)
    for placement in placements:
        rows = rotation_rows(*placement["rotation"])
        sx, sy, sz = (value * ASSET_PRETRANSFORM for value in placement["scale"])
        px, py, pz = placement["position"]
        for triangle in load_up_facing_triangles(placement["assetId"], asset_paths):
            world = []
            for vertex in triangle:
                lx, ly, lz = vertex[0] * sx, vertex[1] * sy, vertex[2] * sz
                world.append(
                    (
                        rows[0][0] * lx + rows[0][1] * ly + rows[0][2] * lz + px,
                        rows[1][0] * lx + rows[1][1] * ly + rows[1][2] * lz + py,
                        rows[2][0] * lx + rows[2][1] * ly + rows[2][2] * lz + pz,
                    )
                )
            a, b, c = world
            centre_y = (a[1] + b[1] + c[1]) / 3.0
            if not FLOOR_Y_RANGE[0] <= centre_y <= FLOOR_Y_RANGE[1]:
                continue
            ux, uy, uz = b[0] - a[0], b[1] - a[1], b[2] - a[2]
            vx, vy, vz = c[0] - a[0], c[1] - a[1], c[2] - a[2]
            nx = uy * vz - uz * vy
            ny = uz * vx - ux * vz
            nz = ux * vy - uy * vx
            norm = math.sqrt(nx * nx + ny * ny + nz * nz)
            if norm <= 1e-12 or abs(ny) / norm < UP_FACING_MINIMUM:
                continue
            ax, az = a[0], a[2]
            bx, bz = b[0], b[2]
            cx, cz = c[0], c[2]
            denominator = (bz - cz) * (ax - cx) + (cx - bx) * (az - cz)
            if abs(denominator) < 1e-12:
                continue
            i0 = max(0, int((min(ax, bx, cx) - grid.origin_x) / grid.cell_size))
            i1 = min(grid.width - 1, int((max(ax, bx, cx) - grid.origin_x) / grid.cell_size))
            j0 = max(0, int((min(az, bz, cz) - grid.origin_z) / grid.cell_size))
            j1 = min(grid.height - 1, int((max(az, bz, cz) - grid.origin_z) / grid.cell_size))
            for j in range(j0, j1 + 1):
                for i in range(i0, i1 + 1):
                    wx, wz = grid.centre(i, j)
                    l1 = ((bz - cz) * (wx - cx) + (cx - bx) * (wz - cz)) / denominator
                    l2 = ((cz - az) * (wx - cx) + (ax - cx) * (wz - cz)) / denominator
                    l3 = 1.0 - l1 - l2
                    if l1 < -1e-9 or l2 < -1e-9 or l3 < -1e-9:
                        continue
                    coverage[grid.index(i, j)].add(placement["placementId"])
    return coverage


def assign_owners(coverage, placements, grid: NavGrid):
    layer_of = {
        row["placementId"]: FLOOR_LAYERS[row["assetId"]] for row in placements
    }
    owner: dict[int, str] = {}
    for cell, placement_ids in coverage.items():
        best = min(
            placement_ids,
            key=lambda pid: (layer_of[pid][1], pid),
        )
        layer_name, order = layer_of[best]
        if order == 0:
            continue
        if grid.walkable[cell] != 1:
            continue
        owner[cell] = best
    return owner, layer_of


def close_seams(owner, layer_of, grid: NavGrid) -> int:
    """Absorb walkable cells that would float once their neighbours collapse."""
    absorbed = 0
    while True:
        changed = 0
        for cell in range(grid.width * grid.height):
            if grid.walkable[cell] != 1 or cell in owner:
                continue
            cx, cz = cell % grid.width, cell // grid.width
            wx, wz = grid.centre(cx, cz)
            radius = math.hypot(wx - ARENA_CENTRE[0], wz - ARENA_CENTRE[1])
            if radius < CORE_RADIUS or radius > ARENA_RADIUS:
                continue
            neighbours = []
            escapes = False
            for dx, dz in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                nx, nz = cx + dx, cz + dz
                if not grid.is_walkable(nx, nz):
                    continue
                neighbour = grid.index(nx, nz)
                if neighbour in owner:
                    neighbours.append(owner[neighbour])
                    continue
                nwx, nwz = grid.centre(nx, nz)
                nradius = math.hypot(nwx - ARENA_CENTRE[0], nwz - ARENA_CENTRE[1])
                if nradius < CORE_RADIUS or nradius > ARENA_RADIUS:
                    escapes = True
            if escapes or not neighbours:
                continue
            owner[cell] = collections.Counter(neighbours).most_common(1)[0][0]
            changed += 1
        absorbed += changed
        if changed == 0:
            return absorbed


def stable_ids(placement_id: str, layer_name: str) -> dict[str, str]:
    stage = STAGE_KEY[layer_name]
    slug = STAGE_SLUG[layer_name]
    return {
        "groupId": f"destroyable.group.valtan.{stage}.{slug}.{placement_id}",
        "mutationId": f"mutation.valtan.{stage}.{slug}.{placement_id}.collapse",
        "bindingId": f"binding.valtan.{stage}.{slug}.{placement_id}",
        "navRegionId": f"navregion.valtan.{stage}.{slug}.{placement_id}",
        "conditionId": f"condition.valtan.{stage}.{slug}.{placement_id}.collapsed",
    }


def build_deploy_asset_rows(asset_paths: dict[str, str]) -> list[str]:
    rows = []
    for map_asset_id, deploy_asset_id in sorted(STAGE_DEPLOY_ASSET.items()):
        relative = asset_paths[map_asset_id]
        label = {
            "VALTAN_FLOOR_RAIL": "Valtan Arena Outer Rail Floor",
            "VALTAN_FLOOR_BRICK_A": "Valtan Arena Brick Floor A",
            "VALTAN_FLOOR_BRICK_B": "Valtan Arena Brick Floor B",
        }[deploy_asset_id]
        evidence = (
            "PROJECT_AUTHORED collapse unit; geometry is the BG_RAD_VALTAN_A "
            "intact-arena overlay export migrated from the map placement layer; "
            "no fractured mesh because the authored mutation ends at DESPAWNED"
        )
        rows.append(
            f'"{deploy_asset_id}" STATIC "{map_asset_id}" '
            f'"{relative}" "Prototype_Component_Model_{deploy_asset_id}_INTACT" '
            f'"" "" "{evidence}"'
        )
    return rows


def build_deploy_placement_rows(placements, layer_of) -> list[str]:
    rows = []
    for placement in sorted(placements, key=lambda row: row["placementId"]):
        layer_name = layer_of[placement["placementId"]][0]
        if layer_name == "CORE":
            continue
        deploy_asset_id = STAGE_DEPLOY_ASSET[placement["assetId"]]
        scale = placement["scale"]
        if not (abs(scale[0] - scale[1]) < 1e-6 and abs(scale[1] - scale[2]) < 1e-6):
            raise BuildError(
                f"Floor placement is not uniformly scaled: {placement['placementId']}"
            )
        px, py, pz = placement["position"]
        qx, qy, qz, qw = placement["rotation"]
        rows.append(
            f'{placement["placementId"]} 0 0 '
            f'"{placement["sourcePlacementId"]}" "{deploy_asset_id}" '
            f"{px:.9g} {py:.9g} {pz:.9g} {qx:.9g} {qy:.9g} {qz:.9g} {qw:.9g} "
            f"{scale[0]:.9g} 1 0 0"
        )
    return rows


def build_world_event_entries(owner, layer_of):
    groups, mutations, bindings = [], [], []
    sectors = sorted({owner[cell] for cell in owner})
    for placement_id in sectors:
        layer_name = layer_of[placement_id][0]
        ids = stable_ids(placement_id, layer_name)
        pattern_id, stage_id = STAGE_PATTERN[layer_name]
        groups.append(
            {
                "groupId": ids["groupId"],
                "memberPlacementIds": [placement_id],
                "navigationRegionIds": [ids["navRegionId"]],
                "navPolarity": "BLOCK_WHILE_FRACTURED",
                "initialState": "INTACT",
            }
        )
        mutations.append(
            {
                "mutationId": ids["mutationId"],
                "groupId": ids["groupId"],
                "targetState": "DESPAWNED",
                "breakingDurationMs": 250,
                "receiverCollisionId": "",
                "navigationStateId": ids["conditionId"],
            }
        )
        bindings.append(
            {
                "bindingId": ids["bindingId"],
                "mutationId": ids["mutationId"],
                "patternId": pattern_id,
                "stageId": stage_id,
                "triggerKind": "STAGE_ENTER",
                "offsetMs": 0,
                "receiverCollisionId": "",
                "enabled": True,
            }
        )
    return groups, mutations, bindings


def build_nav_region_blocks(owner, layer_of, grid: NavGrid) -> list[str]:
    by_placement: dict[str, list[int]] = collections.defaultdict(list)
    for cell, placement_id in owner.items():
        by_placement[placement_id].append(cell)
    blocks = []
    for placement_id in sorted(by_placement):
        layer_name = layer_of[placement_id][0]
        ids = stable_ids(placement_id, layer_name)
        cells = sorted(
            (cell % grid.width, cell // grid.width) for cell in by_placement[placement_id]
        )
        lines = [
            f'REGION "{ids["navRegionId"]}" "{ids["conditionId"]}" 1 {len(cells)}'
        ]
        lines.extend(f"{cx} {cz}" for cx, cz in cells)
        blocks.append("\n".join(lines))
    return blocks


def report(owner, layer_of, absorbed: int) -> None:
    by_placement = collections.Counter(owner.values())
    print(f"seam cells absorbed: {absorbed}")
    print(f"{'runtimePlacementId':<22}{'stage':<10}{'cells':>7}")
    total = 0
    for placement_id in sorted(by_placement, key=lambda pid: (layer_of[pid][1], pid)):
        count = by_placement[placement_id]
        total += count
        print(f"{placement_id:<22}{layer_of[placement_id][0]:<10}{count:>7}")
        expected = EXPECTED_SECTOR_CELLS.get(placement_id)
        if expected is not None and expected != count:
            raise BuildError(
                f"Sector cell count changed for {placement_id}: "
                f"expected {expected}, measured {count}. "
                "Re-approve the sector boundary before publishing."
            )
    print(f"{'TOTAL':<22}{'':<10}{total:>7}")

    stage_cells = collections.defaultdict(set)
    for cell, placement_id in owner.items():
        stage_cells[layer_of[placement_id][0]].add(cell)
    overlap = stage_cells["STAGE_A"] & stage_cells["STAGE_B"]
    if overlap:
        raise BuildError(f"Floor stages share {len(overlap)} navigation cells.")
    print(
        f"stage A={len(stage_cells['STAGE_A'])} "
        f"stage B={len(stage_cells['STAGE_B'])} overlap=0"
    )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--mode", choices=("Validate", "Apply"), default="Validate")
    arguments = parser.parse_args()

    asset_paths = load_map_assets()
    placements = load_floor_placements()
    grid = NavGrid(NAV_GRID)

    coverage = rasterise_coverage(placements, asset_paths, grid)
    owner, layer_of = assign_owners(coverage, placements, grid)
    absorbed = close_seams(owner, layer_of, grid)

    for cell in owner:
        if grid.walkable[cell] != 1:
            raise BuildError("A collapse region claimed a base non-walkable cell.")

    report(owner, layer_of, absorbed)

    groups, mutations, bindings = build_world_event_entries(owner, layer_of)
    deploy_assets = build_deploy_asset_rows(asset_paths)
    deploy_placements = build_deploy_placement_rows(placements, layer_of)
    nav_regions = build_nav_region_blocks(owner, layer_of, grid)

    print(
        f"prepared {len(deploy_assets)} deploy assets, "
        f"{len(deploy_placements)} deploy placements, "
        f"{len(groups)} groups, {len(nav_regions)} navigation regions"
    )

    if arguments.mode == "Validate":
        print("Validate only: no file was written.")
        return 0

    raise BuildError(
        "Apply mode writes five authoring documents in one transaction and is "
        "implemented in the next step of this plan; run Validate until then."
    )


if __name__ == "__main__":
    try:
        sys.exit(main())
    except BuildError as error:
        print(f"build_valtan_floor_collapse: {error}", file=sys.stderr)
        sys.exit(1)
```

`--mode Apply`의 다섯 파일 트랜잭션 교체는 G1 구현 중에 채운다. Validate가 먼저 정확한 수치를
내는 것이 sector 승인 증거이므로 이 순서를 지킨다. `EXPECTED_SECTOR_CELLS`는 사용자가 승인한
경계를 코드에 고정한 값이며, 지오메트리나 navgrid가 바뀌어 수치가 달라지면 도구가 실패해서
조용한 재배정을 막는다.

---

## 8. 남은 불확실성

1. **84 패턴의 실제 애니메이션과 타이밍이 없다.** stage 900/500/1200 ms와 `actionId`는
   `PROJECT_TUNED` 잠정값이다. `Data/Animation/Authored/Valtan/Valtan.patternbindings.json`과
   clip 근거가 확보되면 교정한다. 84 전용 카메라는 원본 근거가 확인될 때만 추가한다.
2. **30줄 collapse edge가 `LANDING`이 맞는지 미확인.** `VALTAN_ARENA_BREAK_33`의
   `CUTSCENE 2500 / LANDING 900 / SPIN 1200 / RECOVERY 1500`에서 `LANDING` STAGE_ENTER를
   1순위 후보로 잡았지만, camera cue와 Server action start tick을 대조한 뒤 확정한다.
3. **파편(debris)이 아직 없다.** 이번 sector 배정은 새 아트 0개가 조건이므로 바닥은 250 ms
   BREAKING 뒤 사라지기만 한다. 기존 generic stone 4종을 재사용한 파편은 별도 단계로 붙인다.
4. **`Data/Maps/Imported`에 project 저작 deploy asset을 넣는 것**은 `BG_RAD_VALTAN_*`가 이미
   Imported `.mapassets`에 들어가 있는 선례를 따른 것이다. Authoring deploy catalog 레이어를
   새로 만드는 편이 더 옳다면 MapCatalog schema와 publisher를 함께 바꾸는 별도 작업이 된다.
5. **낙사 전체(G2).** G1 완료 시점에 무너진 바닥은 NON-WALKABLE일 뿐이다.
