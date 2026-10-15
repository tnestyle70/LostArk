# 쿠크 전체 흐름 학습: Server 현재 구현 근거

읽는 순서: [00 처음부터 읽기](C:/Users/tnest/Desktop/LostArk/.md/GB/10-14/2026-10-14_KOUKU_FULL_PATTERN_WALKTHROUGH_LEARNING_RESULT.md) → [01 Python 파싱·투영](C:/Users/tnest/Desktop/LostArk/.md/GB/10-14/2026-10-14_KOUKU_JSON_PIPELINE_LEARNING_RESULT.md) → [02 Server 구조체·판정](C:/Users/tnest/Desktop/LostArk/.md/GB/10-14/2026-10-14_KOUKU_SERVER_RUNTIME_LEARNING_RESULT.md) → [03 Client 저작·애니메이션](C:/Users/tnest/Desktop/LostArk/.md/GB/10-14/2026-10-14_KOUKU_CLIENT_ANIMATION_LEARNING_RESULT.md) → [04 Collider·좌표·피해](C:/Users/tnest/Desktop/LostArk/.md/GB/10-14/2026-10-14_KOUKU_COLLIDER_RUNTIME_LEARNING_RESULT.md) → [05 전체 패턴 지도](C:/Users/tnest/Desktop/LostArk/.md/GB/10-14/2026-10-14_KOUKU_PATTERN_ATLAS_LEARNING_RESULT.md)

2026-10-14 디스크 저장본 기준. 이 자료 작성으로 제품 코드·저작 JSON·게시 bootstrap·PPT를 수정하지 않았고 새 빌드/실행도 하지 않았다. 아래 코드 블록은 별도의 대체 구현이 아니라 현재 제품 소스에서 추출한 원문이다. 함수·행 번호도 추출 시점 기준이다.

읽는 순서는 **저작 정의와 배치 → projection → 게시 행 → Catalog 정의 → 실행 상태 → 30Hz 판정 → Room 확정 → packet**이다. Server는 한글 패턴 이름을 보고 동작을 고르지 않는다. stable ID로 정의를 찾고 영문 kind를 enum으로 바꾼 후 해당 처리 분기를 실행한다.

2026-10-14 구조 분리는 제품에 반영되어 `GameRoom_BossSimulation.cpp`는 tick 순서만 남았고 WorldEntity/Valtan/KoukuMechanics/KoukuTargeting으로 구현이 나뉘었다. 클래스와 상태 소유자는 여전히 하나의 `CGameRoom`이다. 이전 수동 가이드의 P17 손 표본 추적 후보와 G1/G2 랜덤 위치 후보는 현재 제품에 적용된 코드로 설명하지 않는다. 현재 `KOUKUSAYDON_LOGIC_CAPTURE_REQUEST`에는 player/slot/hold-end/window-index만 있고 `CaptureHandKeys`는 없다.

## 1. JSON이 Server에 들어가기까지

`Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json`은 저작 정의와 occurrence를 가진다. `logicId`는 재사용할 정의, `occurrenceId`는 특정 패턴 시간축에 놓인 상자다. 같은 Logic 정의를 다른 startMs에 여러 번 놓을 수 있으므로 두 ID가 필요하다. `displayName`은 편집·표시용이며 서버 dispatcher의 조건이 아니다.

실제 `logicType` 값은 **DURATION / TRIGGER / RESULT** 세 종류다. HOLD라는 별도 type은 없다. 잡기 유지 상자는 DURATION + `judgementKind=ATTACHMENT_HOLD`이며 접촉 Trigger의 `holdLogicOccurrenceId`가 그 상자를 참조한다.

Python projection은 저작의 정의를 occurrence별로 해석하여 `Data/Encounters/KoukuSaydon/KoukuSaydonEncounter.json`의 `patterns[].stages/logicWindows/mechanicTriggers/worldSequences/sceneProfiles`와 presentation binding으로 만든다. RESULT 정의는 독립 실행 window가 되지 않고 해당 occurrence의 `onSuccess/onFail/onTimeout` 배열에 붙는다. DURATION의 일부 종류(추적·빙고·분신)와 일반 TRIGGER는 mechanic trigger로 투영된다. ENTER_AREA/OBJECT_CONTACT Trigger는 실제 겹침 판정이 필요하므로 LogicWindow가 된다. ANIMATION_BLEND는 presentation projection에서 소비된다.

PowerShell `Add-KoukuBootstrapRows`는 이미 투영된 Encounter를 검증하고 tab-separated 행으로 직렬화한다. Server가 이 Python/PowerShell 스크립트를 매 tick 실행하거나 원본 animation JSON을 매 tick 파싱하는 구조는 아니다. Server Catalog는 `Gameplay.bootstrap` 행을 읽고 C++ 정의를 만든다. 빌드·게시 및 명시적인 Product/Draft admission 경로가 데이터 세대를 연결한다.

- [Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py](<C:/Users/tnest/Desktop/LostArk/Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py:251>)
- [Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py](<C:/Users/tnest/Desktop/LostArk/Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py:156>)
- [Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py](<C:/Users/tnest/Desktop/LostArk/Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py:5107>)
- [Tools/KoukuSaydonPipeline/KoukuBootstrapRows.ps1](<C:/Users/tnest/Desktop/LostArk/Tools/KoukuSaydonPipeline/KoukuBootstrapRows.ps1:708>)
- [Server/Private/GameplayCatalog.cpp](<C:/Users/tnest/Desktop/LostArk/Server/Private/GameplayCatalog.cpp:1419>)

### PATTERNLOGIC의 실제 필드 순서

아래 index는 C++의 `fields[index]`와 같다. 일반 row는 22개 필드이며 OBJECT_OVERLAP만 4개를 추가해 26개다. parser는 20/21개인 기존 형식도 받지만 현재 publisher가 보통 내보내는 행은 22개다. `-`는 빈 선택 필드의 게시 표기다.

| index | 게시 필드 | C++ 도착지 / 의미 |
|---:|---|---|
|0|PATTERNLOGIC|행 종류|
|1|encounterId|어느 encounter의 pattern vector인지|
|2|patternId|어느 `BOSS_PATTERN_DEFINITION`인지|
|3|windowIndex|현재 vector 크기와 같아야 하는 0-based 게시 순서|
|4|windowId|`strWindowId`, 실제 Logic occurrence ID|
|5|kind|`eKind`, 영어 문자열을 enum으로 변환|
|6|startMs|`iStartMs`, 패턴 시작 상대 ms|
|7|durationMs|`iDurationMs`, 판정창 길이; 0 금지|
|8|sectorCount|`iSectorCount`, 룰렛 sector 수|
|9,10|centerX,centerZ|`fCenterX/Z`, 룰렛 중심 m|
|11|outerRadiusM|`fOuterRadiusM`|
|12|stopYawDegrees|`fStopYawDegrees`, 정지 룰렛 방향|
|13|halfAngleDegrees|`fHalfAngleDegrees`, 시선 허용 반각|
|14|maxDistanceM|`fMaxDistanceM`, 시선 최대 거리|
|15|poseIndex|`iPoseIndex`, 요구 포즈 index|
|16|threshold|`iThreshold`, kind별 무력화/보호/빙고 기준|
|17|shieldArcDegrees|`fShieldArcDegrees`|
|18|endsPatternOnSuccess|`bEndsPatternOnSuccess`, 0/1|
|19|sectorSymbols|`SectorSymbols`, comma 목록 또는 `-`|
|20|normalYawOffsetDegrees|`fNormalYawOffsetDegrees`|
|21|insideFail|`bInsideIsFail`, 안쪽을 Fail로 해석하는 정책|
|22~25|targetWorldInstanceId,targetWorldX,targetWorldZ,targetRadiusM|OBJECT_OVERLAP에서만 대상 원 정의|

부속 row는 같은 encounter+pattern+windowId를 찾아 이미 만들어진 window에 붙는다. `PATTERNLOGICOUTCOME`의 기본 순서는 **tag / encounter / pattern / window / SUCCESS·FAIL·TIMEOUT / ordinal / result kind / percent / durationMs / followupPatternId**다. kind별 motion/grip/fixed-damage 등의 추가 필드는 parser와 publisher가 함께 검증한다. `PATTERNLOGICCONTACTMOTION`은 이 결과 ordinal을 찾은 다음 `targetWorldOccurrenceId → motionInstanceId`를 추가한다. `PATTERNLOGICSIGNAL`은 대상 Logic occurrence와 접촉 대상 필터를 추가한다.

`PATTERNLOGICREGION`은 **tag / encounter / pattern / window / regionOrdinal / regionId / anchorKind / shape / centerXYZ / yaw / halfXYZ / radius / halfAngle / cardSymbol / cardColor**, 이후 선택적인 sector-axis/inner-radius 필드다. `PATTERNLOGICREGIONWORLD`와 key row가 시간별 변환을 붙인다. region ID는 도형의 저작 발생 identity이며 bone 문자열을 Server가 직접 해석하는 필드가 아니다.

[Tools/KoukuSaydonPipeline/KoukuBootstrapRows.ps1](<C:/Users/tnest/Desktop/LostArk/Tools/KoukuSaydonPipeline/KoukuBootstrapRows.ps1:1350>) — 1350~1367행. 아래는 현재 제품 소스 원문이다.

```powershell
		$patternRows.Add((@(
			'PATTERNLOGIC', $koukuEncounterDocument.encounterId,
			$koukuPattern.patternId, $windowIndex, $window.windowId, $windowKind,
			[uint32]$window.startMs, [uint32]$window.durationMs,
			[uint32]$window.sectorCount,
			(Format-InvariantSignedFloat $window.centerX 'KoukuSaydon logic window centerX'),
			(Format-InvariantSignedFloat $window.centerZ 'KoukuSaydon logic window centerZ'),
			(Format-InvariantFloat $window.outerRadiusM 'KoukuSaydon logic window outerRadiusM'),
			(Format-InvariantFloat $window.stopYawDegrees 'KoukuSaydon logic window stopYawDegrees'),
			(Format-InvariantFloat $window.halfAngleDegrees 'KoukuSaydon logic window halfAngleDegrees'),
			(Format-InvariantFloat $window.maxDistanceM 'KoukuSaydon logic window maxDistanceM'),
			[uint32]$window.poseIndex, [uint32]$window.threshold,
			(Format-InvariantFloat $window.shieldArcDegrees 'KoukuSaydon logic window shieldArcDegrees'),
			$endsPatternFlag, $symbolText, (Format-InvariantSignedFloat $window.normalYawOffsetDegrees 'KoukuSaydon shield normal offset'), $insideFail) + $objectTargetFields -join "`t"))
		if ($windowKind -ceq 'PATTERN_COMPLETION_COUNT') {
			$patternRows.Add((@('PATTERNLOGICCHAIN', $koukuEncounterDocument.encounterId,
				$koukuPattern.patternId, $window.windowId, $window.completionCount) + @($window.patternIds) -join "`t"))
		}
```

[Server/Private/GameplayCatalog.cpp](<C:/Users/tnest/Desktop/LostArk/Server/Private/GameplayCatalog.cpp:3111>) — 3111~3126행. 아래는 현재 제품 소스 원문이다.

```cpp
		else if (!fields.empty() && "PATTERNLOGIC" == fields[0])
		{
			/* 쿠크 판정 window의 행 순서가 저작 상자 순서다. 뒤따르는 outcome은 windowId로 연결한다. */
			BOSS_PATTERN_LOGIC_WINDOW window{};
			std::uint32_t windowIndex = 0u;
			std::uint32_t endsPattern = 0u;
			if ((20u != fields.size() && 21u != fields.size() && 22u != fields.size() && 26u != fields.size()) || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) || !ParseNumber(fields[3], windowIndex) ||
				!IsStableId(fields[4]) ||
				!ParseBossPatternLogicKind(fields[5], window.eKind) ||
				!ParseNumber(fields[6], window.iStartMs) ||
				!ParseNumber(fields[7], window.iDurationMs) || 0u == window.iDurationMs ||
				!ParseNumber(fields[8], window.iSectorCount) || window.iSectorCount > 64u ||
				!ParseNumber(fields[9], window.fCenterX) || !std::isfinite(window.fCenterX) ||
				!ParseNumber(fields[10], window.fCenterZ) || !std::isfinite(window.fCenterZ) ||
				!ParseNumber(fields[11], window.fOuterRadiusM) ||
```

### Pattern / Stage / World 기본 행도 같은 방식으로 읽기

| 행 | 0-based 필드 순서 |
|---|---|
|PATTERN (17개)|0 tag, 1 encounter, 2 pattern, 3 action, 4 selection, 5 minimumHealthBar, 6 maximumHealthBar, 7 triggerHealthBar, 8 triggerOrder, 9 selectionWeight, 10 maximumConsecutiveUses, 11 minimumRange, 12 maximumRange, 13 stageCount, 14 armorRequirement, 15 phaseRequirement, 16 invulnerable|
|PATTERNSTAGE (현재 쿠크 22개)|0 tag, 1 encounter, 2 pattern, 3 ordinal, 4 stageId, 5 actionId, 6 stageKind, 7 durationMs, 8 hitShape, 9 outerRadius, 10 innerRadius, 11 hitAngleDegrees, 12 hitLength, 13 hitHalfWidth, 14 hitCount, 15 hitIntervalMs, 16 hitDelayMs, 17 damageProfileId, 18 pushRangeM, 19 pushMs, 20 knockdown, 21 downMs|
|PATTERNWORLDSEQUENCE (15개)|0 tag, 1 encounter, 2 pattern, 3 startMs, 4 sequenceInstanceId, 5 playbackSpeed, 6~8 positionOffsetXYZ, 9 anchorKind, 10~12 anchorPositionXYZ, 13 durationMs, 14 occurrenceId|
|PATTERNMECHANICTRIGGER (25개)|0 tag, 1 encounter, 2 pattern, 3 triggerId, 4 kind, 5 startMs, 6 durationMs, 7 hudMode, 8~10 teleportXYZ, 11 clonePatternId, 12~14 clockHours, 15 고정 검증 literal 1(struct 필드에 대입하지 않음), 16 faceCenterYawOffsetDegrees, 17 countPerPlayer, 18 radiusM, 19 effectLifetimeMs, 20 arenaRandomCount, 21 arenaRandomRadiusM, 22 arenaHeightToleranceM, 23 arenaMinimumSpacingM, 24 randomPlayerOnly|

현재 쿠크 PATTERNSTAGE publisher는 hitShape=NONE, hit 수치=0, damageProfile='-'를 강제한다. 실제 공격은 Logic/AttackContacts/기믹 출력 경로에서 만든다. 따라서 animation stage에 ACTIVE라는 이름이 있다고 그 자체로 모든 공격 피해가 생기는 것은 아니다. root motion은 PATTERNSTAGEROOTMOTION, 추가 접촉은 PATTERNATTACKHIT 등 별도 row가 연결한다. 공유 parser의 24필드 stage 변형은 player response/attachment slot을 추가하지만 현재 쿠크 기본 emit은 22개다.

- [Tools/KoukuSaydonPipeline/KoukuBootstrapRows.ps1](<C:/Users/tnest/Desktop/LostArk/Tools/KoukuSaydonPipeline/KoukuBootstrapRows.ps1:919>)
- [Tools/KoukuSaydonPipeline/KoukuBootstrapRows.ps1](<C:/Users/tnest/Desktop/LostArk/Tools/KoukuSaydonPipeline/KoukuBootstrapRows.ps1:1044>)
- [Tools/KoukuSaydonPipeline/KoukuBootstrapRows.ps1](<C:/Users/tnest/Desktop/LostArk/Tools/KoukuSaydonPipeline/KoukuBootstrapRows.ps1:2073>)
- [Tools/KoukuSaydonPipeline/KoukuBootstrapRows.ps1](<C:/Users/tnest/Desktop/LostArk/Tools/KoukuSaydonPipeline/KoukuBootstrapRows.ps1:2159>)
- [Server/Private/GameplayCatalog.cpp](<C:/Users/tnest/Desktop/LostArk/Server/Private/GameplayCatalog.cpp:3009>)
- [Server/Private/GameplayCatalog.cpp](<C:/Users/tnest/Desktop/LostArk/Server/Private/GameplayCatalog.cpp:5148>)
- [Server/Private/GameplayCatalog.cpp](<C:/Users/tnest/Desktop/LostArk/Server/Private/GameplayCatalog.cpp:4595>)

### Catalog 로드의 성공/실패 경계

1. `Load()`가 data root를 결정하고 **임시 CGameplayCatalog staged**에 파일을 읽는다. `NumericBalance.active.json`이 있다면 bootstrap/숫자/비숫자 hash와 parent revision을 대조한다. 마지막 `*this = std::move(staged)` 전에는 현재 catalog가 바뀌지 않는다.
2. `Load_BootstrapPath()`는 최대 크기와 전체 파일 읽기 성공을 검사해 bytes를 만든다.
3. `Load_BootstrapBytes()`는 기존 map들을 rollback 객체가 보존한 채 새 row를 읽는다. header 이름·버전·행 수와 내용 revision, 각 row의 enum/숫자/finite/ID/소유 관계/중복/순서, 마지막 cross-reference까지 검사한다. 이 함수가 내부 map을 채운다는 사실과 live Catalog를 바로 교체한다는 것은 다르다.
4. 후반 전체 정의 검증까지 통과한 뒤 `rollback.committed=true`로 보존 객체의 복원을 막고 active revision 및 분리된 Kouku/non-Kouku row를 확정한다. 실패 시 rollback 소멸자가 이전 map을 복원한다. 바깥 Load도 실패한 staged를 버린다.
5. Product/Draft 경로는 `Load_PublishedKoukuProduct` / `Load_DraftKoukuProduct`를 통해 별도 세대를 만든다. Draft는 active non-Kouku 행과 허용된 Kouku 행을 합친 뒤 같은 parser를 사용하며 비쿠크 변경을 거절한다. 실행 중인 member는 `shared_ptr<const CGameplayCatalog>`와 gameplay revision을 고정해 다른 게시 세대와 섞지 않는다.

현재 설치 header는 `LOSTARK_GAMEPLAY_BOOTSTRAP	38	109868`이다. header가 새 row 지원까지 자동으로 보장하는 것은 아니다. 새 row/struct 계약을 추가할 때 publisher/parser/version/admission을 함께 다루어야 한다.

- [Server/Private/GameplayCatalog.cpp](<C:/Users/tnest/Desktop/LostArk/Server/Private/GameplayCatalog.cpp:1744>)
- [Server/Private/GameplayCatalog.cpp](<C:/Users/tnest/Desktop/LostArk/Server/Private/GameplayCatalog.cpp:1773>)
- [Server/Private/GameplayCatalog.cpp](<C:/Users/tnest/Desktop/LostArk/Server/Private/GameplayCatalog.cpp:1796>)
- [Server/Private/GameplayCatalog.cpp](<C:/Users/tnest/Desktop/LostArk/Server/Private/GameplayCatalog.cpp:8831>)
- [Server/Private/GameplayCatalog.cpp](<C:/Users/tnest/Desktop/LostArk/Server/Private/GameplayCatalog.cpp:1604>)
- [Server/Private/GameplayCatalog.cpp](<C:/Users/tnest/Desktop/LostArk/Server/Private/GameplayCatalog.cpp:547>)
- [Server/Private/GameplayCatalog.cpp](<C:/Users/tnest/Desktop/LostArk/Server/Private/GameplayCatalog.cpp:589>)

## 2. 정의와 실행 상태를 분리해서 읽기

| 자료 | 소유자·수명 | 담는 내용 |
|---|---|---|
|BOSS_PATTERN_DEFINITION 및 내부 vector|CGameplayCatalog 세대; pin이 살아 있는 동안 유지|읽기 전용 저작 결과. 어떤 동작을 언제 할 것인지|
|SERVER_WORLD_ENTITY|CGameRoom의 살아 있는 boss actor|현재 HP/XYZ/yaw/action/stage index/start tick/sequence|
|KOUKUSAYDON_PATTERN_AUDITION_STATE|CGameRoom 한 실행 epoch|member 목록, pinned catalog, world cues, support, tail|
|KOUKUSAYDON_PATTERN_AUDITION_MEMBER|bundle 내 한 보스 실행|pattern 순서와 next tick, 현재 LogicLedger, cue 매핑|
|KOUKUSAYDON_LOGIC_LEDGER|해당 pattern 발생; 자연 종료 후 tail이 소유할 수도 있음|window 열림/닫힘/답/이미 접촉한 카드/우선순위/예정 cue|
|KOUKUSAYDON_LOGIC_OUTPUT|한 Update 호출의 지역 결과|Room에 확정·송신을 요청할 world play/후속 패턴/capture/mechanic|
|S2C_WORLD_SEQUENCE_PLAY|직렬화되어 각 session 송신 큐로 전달|정의 전체 대신 어떤 occurrence에 어떤 motion을 언제 적용할지|

`std::vector` 값이 정의/상태 요소를 직접 소유한다. pattern/window를 `const&`로 넘기는 호출은 그 기간 차용한다. entity vector를 순회하는 동안 새로운 entity를 추가하면 참조가 무효화될 수 있으므로 소환/순간이동 같은 Room 작업을 pending queue에 모아 순회 뒤에 확정한다. 소스 파일을 나누었어도 별도 스레드나 별도 저장소를 만든 것이 아니다.

### Pattern / Stage 필드 읽기

`BOSS_PATTERN_DEFINITION`의 ID 5개는 encounter/pattern/gate/target placement/action의 연결 키다. `bAuthoringMasterManaged`는 저작 정본 관리 여부, `Motion/Finale/fVerticalOffsetM`는 패턴 수준 이동·종료·높이 정책이다. `eSelection/eArmorRequirement/ePhaseRequirement/bInvulnerableWhileRunning/eCategory/eTargetPolicy/eAimPolicy`는 선택·요구 상태·무적·분류·대상·조준 정책이다. `iMinimumPhase/iMaximumPhase/iMinimumHealthBar/iMaximumHealthBar/iTriggerHealthBar/iTriggerOrder/iSelectionWeight/iMaximumConsecutiveUses/fMinimumRange/fMaximumRange`는 선택 조건이다. `iSourcePrimaryActionId/iSourceShapeCount/iSourceCooldownMs/iSourceCooldownTicks/iSourceRangeUnits/iSourceApproachUnits/iSourceTurnDegrees`는 원본 action metadata이며 쿠크 접촉창의 현재 실행 시간이 아니다. `iExpectedStageCount`는 게시 stage 수 검증값이다.

`AuditionBossArchetypeIds`는 재생 가능한 모델/보스 종류; `Stages`는 actor의 순차 animation action; `LogicWindows`는 별도의 판정 시간축; `bResetBossToSpawn/ResetBossYawDegrees/BossMotion`은 시작 위치 및 저작 이동; `bFixedTimelineClock`은 공통 ms 시계 사용; `iTimelineDurationMs`는 독립 occurrence 전체 길이; `ParentChildren/strParentLoopStartOccurrenceId`는 자식 scheduler와 반복 시작; `MechanicTriggers/WorldSequences/SceneProfiles`는 각각 권위 기믹·월드 cue·장면 연출 정의다.

Stage는 공유 자료형이므로 Valtan 타격/분기 필드도 가진다. 쿠크 Brain의 일반 stage 진행은 `Stages` vector 순서로 다음 stage를 선택한다. `STAGE_13`이라는 ID 숫자가 13번째 순서를 뜻하지 않는다.

| Stage 필드 | 의미·소비 |
|---|---|
|strStageId/strActionId/strDamageProfileId|단계 identity / Client가 clip을 찾는 action identity / 피해 profile|
|eStageKind/iDurationMs|WINDUP·ACTIVE·RECOVERY와 단계 길이|
|eHitShape/ePlayerResponse/eAttachmentSlot|타격 도형/플레이어 반응/부착 슬롯|
|ePartDamagePolicy/bHasCounterProxy/eCounterProxyKind/fCounterProxyForwardOffsetM/fCounterProxyRightOffsetM/fCounterProxyRadiusM/fCounterProxyArcDegrees|부위 손상/카운터 대리 도형 존재·정책·치수|
|eBossResponseKind/iBossResponseThreshold|무력화 등 boss 반응 정책과 기준|
|fVerticalOffsetM|stage의 높이 오프셋|
|fHitOuterRadius/fHitInnerRadius/fHitAngleDegrees/fHitLength/fHitHalfWidth|공격 도형 치수와 각도|
|iHitCount/iHitIntervalMs/iHitDelayMs/HitOffsetsMs/AttackContacts|반복 타격 수·간격·지연·명시 시각·접촉 템플릿|
|eHitAnchorKind/fHitAnchorForwardOffsetM/fHitAnchorRightOffsetM/fHitAnchorYawOffsetDegrees|hit 도형 기준·로컬 오프셋·방향|
|eHitActivationKind/iHitActivationStartMs/iHitActivationLifetimeMs|별도 활성 시각/수명|
|fPushRangeM/iPushMs/bKnockdown/iDownMs|피격 밀기와 넘어짐|
|bWallContact/bChargeImpact/bPiercesCover|벽 접촉·돌진 충격·엄폐 관통|
|Motion|이동·root motion 설정과 표본|
|bTrackNearestTarget/bTrackPatternTarget/bHasAimEnd/iAimEndMs/bHasAimResponseScale/fAimResponseScale|최근접/패턴 타겟 추적·조준 종료 시각·조준 응답 배율|
|strPropBreakSetId/PropBreakSlotIds|stage에서 파괴할 prop 연결|
|Branches/Actions|공유 stage 분기 및 ENTER/EXIT action 정의. 쿠크 Brain 진행에서 전부 자동 소비된다는 뜻은 아님|

### BOSS_PATTERN_DEFINITION

[Server/Public/GameplayCatalog.h](<C:/Users/tnest/Desktop/LostArk/Server/Public/GameplayCatalog.h:1433>)

```cpp
struct BOSS_PATTERN_DEFINITION
	{
		std::string strEncounterId;
		std::string strPatternId;
		std::string strGateId;
		std::string strTargetBossPlacementId;
		std::string strActionId;
		bool bAuthoringMasterManaged = false;
		BOSS_PATTERN_MOTION Motion;
		BOSS_PATTERN_FINALE Finale;
		// 패턴 진입 기준 Y에 한 번 더하는 값이다. 종료 정리에서 stage 오프셋과 분리해 복원한다.
		float fVerticalOffsetM = 0.f;
		BOSS_PATTERN_SELECTION eSelection = BOSS_PATTERN_SELECTION::NORMAL;
		BOSS_PATTERN_ARMOR_REQUIREMENT eArmorRequirement =
			BOSS_PATTERN_ARMOR_REQUIREMENT::ANY;
		BOSS_PATTERN_PHASE_REQUIREMENT ePhaseRequirement =
			BOSS_PATTERN_PHASE_REQUIREMENT::ANY;
		// 이 패턴 실행 동안 외부 player hit 경로가 보스 무적을 적용하도록 하는 정책이다.
		bool bInvulnerableWhileRunning = false;
		BOSS_PATTERN_CATEGORY eCategory = BOSS_PATTERN_CATEGORY::NORMAL;
		BOSS_PATTERN_TARGET_POLICY eTargetPolicy =
			BOSS_PATTERN_TARGET_POLICY::NONE;
		BOSS_PATTERN_AIM_POLICY eAimPolicy = BOSS_PATTERN_AIM_POLICY::NONE;
		std::uint32_t iMinimumPhase = 1;
		std::uint32_t iMaximumPhase = 1;
		std::uint32_t iMinimumHealthBar = 0;
		std::uint32_t iMaximumHealthBar = 0;
		std::uint32_t iTriggerHealthBar = 0;
		std::uint32_t iTriggerOrder = 0;
		std::uint32_t iSelectionWeight = 0;
		std::uint32_t iMaximumConsecutiveUses = 0;
		float fMinimumRange = 0.f;
		float fMaximumRange = 0.f;
		// Valtan 원본 주 action의 metadata다. 원본 단위 범위와 Server가 m 단위로 조정한 선택 범위는 서로 다른 값이다.
		std::uint32_t iSourcePrimaryActionId = 0;
		std::uint32_t iSourceShapeCount = 0;
		std::uint32_t iSourceCooldownMs = 0;
		std::uint32_t iSourceCooldownTicks = 0;
		std::uint32_t iSourceRangeUnits = 0;
		std::uint32_t iSourceApproachUnits = 0;
		std::uint32_t iSourceTurnDegrees = 0;
		std::uint32_t iExpectedStageCount = 0;
		// Kouku audition에서 이 패턴을 재생할 수 있는 보스 archetype 목록이다. 실제 살아 있는 대상 승인은 GameRoom이 수행한다.
		std::vector<std::string> AuditionBossArchetypeIds;
		std::vector<BOSS_PATTERN_STAGE_DEFINITION> Stages;
		// stage 시계 옆에서 Kouku Logic/기믹/World/Scene 연출을 평가할 게시 정의다. 실제 진행 상태는 entity의 ledger가 소유한다.
		std::vector<BOSS_PATTERN_LOGIC_WINDOW> LogicWindows;
		bool bResetBossToSpawn = false;
		bool bFixedTimelineClock = false;
        // 0이면 stage 길이 합을 사용한다. 독립 occurrence의 끝이 더 늦다고 stage 시간을 늘리지는 않는다.
        std::uint32_t iTimelineDurationMs = 0u;
        // 보존한 Parent의 자식 목록이다. 확장한 animation stage 대신 기존 멤버 예약 경로로 실행한다.
        std::vector<BOSS_PATTERN_PARENT_CHILD> ParentChildren;
        std::string strParentLoopStartOccurrenceId;
		std::optional<float> ResetBossYawDegrees;
		std::optional<BOSS_PATTERN_BOSS_MOTION> BossMotion;
		std::vector<BOSS_PATTERN_MECHANIC_TRIGGER> MechanicTriggers;
		std::vector<BOSS_PATTERN_WORLD_SEQUENCE> WorldSequences;
		std::vector<BOSS_PATTERN_SCENE_PROFILE> SceneProfiles;
	};
```

### BOSS_PATTERN_STAGE_DEFINITION

[Server/Public/GameplayCatalog.h](<C:/Users/tnest/Desktop/LostArk/Server/Public/GameplayCatalog.h:1094>)

```cpp
struct BOSS_PATTERN_STAGE_DEFINITION
	{
		std::string strStageId;
		std::string strActionId;
		std::string strDamageProfileId;
		BOSS_PATTERN_STAGE_KIND eStageKind =
			BOSS_PATTERN_STAGE_KIND::WINDUP;
		BOSS_PATTERN_HIT_SHAPE eHitShape =
			BOSS_PATTERN_HIT_SHAPE::NONE;
		// DAMAGE는 일반 타격이다. CAPTURE도 타격을 먼저 처리한 뒤 살아 있는 대상의 typed 부착을 GameRoom에 요청한다.
		BOSS_PATTERN_PLAYER_RESPONSE ePlayerResponse =
			BOSS_PATTERN_PLAYER_RESPONSE::DAMAGE;
		LostArk::Shared::PLAYER_ATTACHMENT_SLOT eAttachmentSlot =
			LostArk::Shared::PLAYER_ATTACHMENT_SLOT::NONE;
		BOSS_PATTERN_PART_DAMAGE_POLICY ePartDamagePolicy =
			BOSS_PATTERN_PART_DAMAGE_POLICY::NORMAL;
		// 카운터 허용 stage에서 시전자 원점을 추가로 제한한다. 원형 offset은 보스 전방/오른쪽 m 기준이다.
		// 전방 반평면 정책은 arc 각도만 사용한다.
		bool bHasCounterProxy = false;
		BOSS_PATTERN_COUNTER_PROXY_KIND eCounterProxyKind =
			BOSS_PATTERN_COUNTER_PROXY_KIND::NONE;
		float fCounterProxyForwardOffsetM = 0.f;
		float fCounterProxyRightOffsetM = 0.f;
		float fCounterProxyRadiusM = 0.f;
		float fCounterProxyArcDegrees = 0.f;
		BOSS_PATTERN_BOSS_RESPONSE_KIND eBossResponseKind =
			BOSS_PATTERN_BOSS_RESPONSE_KIND::NONE;
		std::uint32_t iBossResponseThreshold = 0u;
		// stage에만 적용하는 Y 오프셋이다. 다음 stage 경계에서 복원하며 pattern 단위 오프셋과 별도로 관리한다.
		float fVerticalOffsetM = 0.f;
		std::uint32_t iDurationMs = 0;
		float fHitOuterRadius = 0.f;
		float fHitInnerRadius = 0.f;
		float fHitAngleDegrees = 0.f;
		float fHitLength = 0.f;
		float fHitHalfWidth = 0.f;
		std::uint32_t iHitCount = 0;
		std::uint32_t iHitIntervalMs = 0;
		// stage 시작부터 첫 타격까지의 ms다. 균등 반복 경로는 이 값에서 iHitIntervalMs 간격으로 진행한다.
		std::uint32_t iHitDelayMs = 0;
		// 균등 간격으로 표현하지 못하는 타격의 stage 기준 ms 목록이다. 비어 있으면 delay/count/interval 경로를 사용한다.
		std::vector<std::uint32_t> HitOffsetsMs;
		std::vector<LostArk::Shared::ATTACK_HIT_TEMPLATE> AttackContacts;
		// 판정 원점을 Effect 연출과 분리한다. 기본값은 현재 보스 자세이며 STAGE_ORIGIN은 stage 진입 자세를 사용한다.
		BOSS_PATTERN_HIT_ANCHOR_KIND eHitAnchorKind =
			BOSS_PATTERN_HIT_ANCHOR_KIND::BOSS_CURRENT;
		float fHitAnchorForwardOffsetM = 0.f;
		float fHitAnchorRightOffsetM = 0.f;
		float fHitAnchorYawOffsetDegrees = 0.f;
		// ACTIVE_WINDOW는 활성 구간 매 tick에 접촉을 평가하고 허용된 대상을 기록한다.
		// PULSE_SCHEDULE은 count/delay/offset 일정의 판정 기회를 소비한다.
		BOSS_PATTERN_HIT_ACTIVATION_KIND eHitActivationKind =
			BOSS_PATTERN_HIT_ACTIVATION_KIND::PULSE_SCHEDULE;
		std::uint32_t iHitActivationStartMs = 0u;
		std::uint32_t iHitActivationLifetimeMs = 0u;
		// 이 stage 타격의 밀기 거리 m와 기간 ms다. 음수 거리는 판정 원점 쪽으로 당긴다.
		float fPushRangeM = 0.f;
		std::uint32_t iPushMs = 0;
		bool bKnockdown = false;
		std::uint32_t iDownMs = 0;
		// 공격 도형을 벽 파괴 contact 후보로 사용할지 정한다. 모든 HP 피해가 벽을 부수는 것은 아니다.
		bool bWallContact = false;
		// GameRoom이 이 stage의 돌진 벽 접촉을 검사하도록 한다. 실제 이동은 Motion/RootMotion 경로가 계산한다.
		// 충돌 뒤 stage는 저작된 WALL_CONTACT 분기에서 결정하며 반드시 바로 다음 GROGGY라는 뜻은 아니다.
		bool bChargeImpact = false;
		// 현재 stage가 encounter prop 엄폐 검사를 건너뛸지 정한다. 이 flag의 허용 여부는 로더의 저작 계약이 검사한다.
		bool bPiercesCover = false;
		BOSS_PATTERN_STAGE_MOTION Motion;
		// stage에만 적용할 추적/조준 종료 조건이다. 설정되지 않은 항목은 pattern의 기본 조준 정책을 따른다.
		bool bTrackNearestTarget = false;
		bool bTrackPatternTarget = false;
		bool bHasAimEnd = false;
		std::uint32_t iAimEndMs = 0u;
		bool bHasAimResponseScale = false;
		float fAimResponseScale = 1.f;
		// 이 stage 진입에서 부술 encounter prop set/slot ID다. prop은 생성한 패턴보다 오래 살 수 있어 파괴 연결을 별도로 저장한다.
		std::string strPropBreakSetId;
		std::vector<std::string> PropBreakSlotIds;
		std::vector<BOSS_PATTERN_STAGE_BRANCH> Branches;
		std::vector<BOSS_PATTERN_STAGE_ACTION> Actions;
	};
```

### LogicWindow / Region / Result 필드 읽기

`BOSS_PATTERN_LOGIC_WINDOW`는 게시된 창 정의이며 현재 열렸는지는 저장하지 않는다.

| 필드 | 의미 |
|---|---|
|strWindowId/eKind/iStartMs/iDurationMs|occurrence identity·판정 방식·패턴 상대 시작/길이|
|PatternIds/iCompletionCount|PATTERN_COMPLETION_COUNT에서 자식 후보와 필요한 완료 횟수|
|iSectorCount/fCenterX/fCenterZ/fOuterRadiusM/fStopYawDegrees/SectorSymbols|룰렛 영역과 카드 문양|
|fHalfAngleDegrees/fMaxDistanceM/bGazeDuringWindow|시선 반각/거리와 창 동안 판정 여부|
|iPoseIndex|POSE_INPUT에서 요구하는 포즈|
|iThreshold/fShieldArcDegrees/fNormalYawOffsetDegrees|무력화/보호/빙고 기준과 방패 방향|
|CardRegions|판정 영역 목록. 이름이 카드여도 일반 공격/포털/잡기 도형도 보관|
|bInsideIsFail|안쪽 결과를 실패로 해석|
|bRearmOnExit/bRepeatAfterKnockback/iRepeatIntervalMs|이탈 후 재무장·넉백 종료 후 재접촉·반복 간격|
|bEndsPatternOnSuccess/bCancelAtEnd|성공 시 actor 패턴 종료 요청 / 부모 종료 시 결과 없이 닫기|
|strOwnerWorldOccurrenceId|파괴된 World 소유자의 접촉창을 폐기하기 위한 연결|
|OnSuccess/OnFail/OnTimeout|각 결과 슬롯의 RESULT 목록; 자동으로 전부 실행하는 것이 아니라 해당 kind 분기가 선택|
|strTargetWorldInstanceId/fTargetWorldX/fTargetWorldZ/fTargetRadiusM|OBJECT_OVERLAP의 단일 대상 원|
|ContactTargets/strContactGroupId/iContactPriority/bHasContactGroup|OBJECT_CONTACT의 후보 목록과 소비 그룹·우선순위·부속행 존재 여부|
|fBossChargeDistanceM/fChargeYawOffsetDegrees|진입 시 target 방향을 고정하는 돌진 거리·표시 yaw 보정|
|strHoldLogicOccurrenceId|포획 Trigger를 소유하는 ATTACHMENT_HOLD 창|

Region의 `eAnchor`는 기준 위치 정책이다. WORLD는 이미 월드 좌표, BOSS_CURRENT는 현재 보스 XZ/yaw, BOSS_SPAWN은 spawn XZ 가산, BOSS_START는 지정 시각의 보스 자세를 한 번 캡처한다. 직접 Y 가산 조건은 도형/선형 이동에 따라 다르므로 이름만 보고 XYZ에 항상 같은 식을 적용하면 안 된다. 정확한 설명은 현재 struct 주석과 Resolve_LogicRegionTransform/Capture_StartRegions를 함께 읽는다.

`strRegionId`는 도형 identity; `bSector/bReverseSector/bCircle/bCylinder`는 도형 선택이며 모두 false면 box 경로다. `bLinearMotion/iMotionStartMs/iMotionDurationMs/EndPositionOffset/EndScale`는 끝 위치·크기까지의 선형 변화다. `iAnchorCaptureStartMs`의 0xffffffff는 별도 시작 미지정이다. `WorldTrack`은 구운 변환 표본. `fCenterXYZ/fYawDegrees/fHalfXYZ/fRadiusM/fInnerRadiusM/fHalfAngleDegrees/fRadiusXM/ZM`는 m/degree 치수이며 `eCardSymbol/eCardColor`는 카드 판정 metadata다.

Track의 `bEnabled/iStartMs/iStartDelayMs/iDurationMs/fPlaybackSpeed/bSmoothStep`은 표본 시계. `fBaselineXYZ/fBaselineYawDegrees/fBaselineScaleXYZ`는 기준 배치이고 `Keys`가 시간 순 표본을 소유한다. Key의 `iTimeMs/fOffsetXYZ/fRotationY/W/fScaleXYZ/bVisible`는 이동·yaw quaternion·크기·표시 여부. `bHasGripPosition/GripPosition`은 World hook의 별도 월드 잡기 지점이며, 현재 제품에 없는 P17 boss hand 표본 후보와 구분한다.

Result의 `eKind`가 사용할 필드를 정한다. `iPercent/iDamageAmount`는 백분율/고정 피해, `iDurationMs`는 상태 유지. `strPatternId`는 후속 패턴, `strTargetWorldInstanceId/strMotionInstanceId`는 일반 World motion, `ContactMotions`는 실제 접촉 occurrence별 motion 매핑, `strTargetLogicOccurrenceId/strContactTargetWorldOccurrenceId`는 완료 신호와 대상 필터. `strFearPresentationId`는 공포 표시 연결, `iMarioEntryStage`는 0 자동/1~4 지정 stage, `eAttachmentSlot/GripLocalOffset`은 부착 슬롯과 forward/up/right 오프셋이다. `fPushRangeM/fPushHeightM/iPushMs/ePushDirection/bForcePush/bPushCanLeaveArena/bPushBallistic/fPushYawOffsetDegrees`는 넉백 거리·높이·시간·방향·강제·낙사·포물선·yaw 정책이다. 콜라이더가 직접 속도를 소유하는 구조는 아니다.

### BOSS_PATTERN_LOGIC_KIND

[Server/Public/GameplayCatalog.h](<C:/Users/tnest/Desktop/LostArk/Server/Public/GameplayCatalog.h:603>)

```cpp
enum class BOSS_PATTERN_LOGIC_KIND : std::uint8_t
	{
		CARD_DICE_BIND,
		ROULETTE_CARD_MATCH,
		GAZE_REAL_BOSS,
		POSE_INPUT,
		STAGGER_WINDOW,
		AREA_OVERLAP,
		ENTER_AREA,
		OBJECT_OVERLAP,
		OBJECT_CONTACT,
		EXTERNAL_SIGNAL,
		COUNTER_WINDOW,
        ATTACHMENT_HOLD,
		PATTERN_COMPLETION_COUNT,
		INVULNERABILITY_ZONE,
		BINGO_COMPLETED_LINES,
		BOSS_DAMAGE_REDUCTION
	};
```

### BOSS_PATTERN_LOGIC_RESULT_KIND

[Server/Public/GameplayCatalog.h](<C:/Users/tnest/Desktop/LostArk/Server/Public/GameplayCatalog.h:623>)

```cpp
enum class BOSS_PATTERN_LOGIC_RESULT_KIND : std::uint8_t
	{
		NONE,
		INSTANT_DEATH,
		MAX_HP_PERCENT_DAMAGE,
		MADNESS_GAUGE_ADD_PERCENT,
		CLOWN_TRANSFORM,
		FEAR,
		FOLLOWUP_PATTERN,
		PLAY_WORLD_OBJECT_MOTION,
		PLAY_CONTACT_WORLD_OBJECT_MOTION,
		COMPLETE_LOGIC_WINDOW,
		CAPTURE_PLAYER,
		// 플레이어를 판정한 World Object 영역에 잡아 이동을 따라가게 한다.
		// 게시 검증은 ENTER_AREA의 WorldTrack 영역 연결을 요구하며 단순 정적 영역 포획과 구분한다.
		GRAB_TO_WORLD_OBJECT,
		MARIO_ENTER,
		FIXED_DAMAGE,
		PLAYER_INVULNERABILITY
	};
```

### BOSS_LOGIC_CONTACT_MOTION

[Server/Public/GameplayCatalog.h](<C:/Users/tnest/Desktop/LostArk/Server/Public/GameplayCatalog.h:644>)

```cpp
struct BOSS_LOGIC_CONTACT_MOTION final
	{
		std::string strTargetWorldOccurrenceId;
		std::string strMotionInstanceId;
	};
```

### BOSS_PATTERN_LOGIC_RESULT

[Server/Public/GameplayCatalog.h](<C:/Users/tnest/Desktop/LostArk/Server/Public/GameplayCatalog.h:658>)

```cpp
struct BOSS_PATTERN_LOGIC_RESULT final
	{
		BOSS_PATTERN_LOGIC_RESULT_KIND eKind = BOSS_PATTERN_LOGIC_RESULT_KIND::NONE;
		std::uint32_t iPercent = 0u;
		std::uint32_t iDurationMs = 0u;
		std::string strPatternId;
		std::string strTargetWorldInstanceId;
		std::string strMotionInstanceId;
		std::vector<BOSS_LOGIC_CONTACT_MOTION> ContactMotions;
		std::string strTargetLogicOccurrenceId;
		std::string strContactTargetWorldOccurrenceId;
		std::string strFearPresentationId;
		// MARIO_ENTER에서만 사용한다. 1~4는 명시 stage이고 0은 현재 방의 순번을 따른다.
		std::uint8_t iMarioEntryStage = 0u;
        LostArk::Shared::PLAYER_ATTACHMENT_SLOT eAttachmentSlot = LostArk::Shared::PLAYER_ATTACHMENT_SLOT::NONE;
        // 전방/위/오른쪽 m 오프셋이다. 부착 연출 위치를 전달하는 값이다.
        std::array<float, 3u> GripLocalOffset{};
		float fPushRangeM = 0.f;
		float fPushHeightM = 0.f;
		std::uint32_t iPushMs = 0u;
		//Collider를 통해서 보스 기준으로  어느 정도 밀려날 것인지를 정하기 위한 struct BOSS_PATTERN_LOGIC_RESULT의
		//구조체 변수이다. AWAY_FROM_BOSS의 경우 boss의 위치를 기준으로 플레이어의 방향을 구하는 벡터를 쏘고, 해당
		//벡터를 기준으로 플레이어를 해당하는 방향과 collider에 정해진 속도만큼 날려보낸다.
		// 이 enum은 밀기 방향을 고른다. 이동 거리 fPushRangeM의 단위는 m, 지속 시간 iPushMs의 단위는 ms다.
		// 방향과 거리/기간을 받은 실제 피격·이동 소비자가 위치 변화를 계산하며 Collider가 속도 값을 직접 소유하지는 않는다.
		BOSS_LOGIC_PUSH_DIRECTION ePushDirection = BOSS_LOGIC_PUSH_DIRECTION::AWAY_FROM_BOSS;
		bool bForcePush = false;
		bool bPushCanLeaveArena = false;
		bool bPushBallistic = false;
		float fPushYawOffsetDegrees = 0.f;
		// 기존 aggregate 초기화 순서를 유지하도록 뒤에 둔 고정 피해량이다.
		std::uint32_t iDamageAmount = 0u;
	};
```

### BOSS_LOGIC_REGION_ANCHOR

[Server/Public/GameplayCatalog.h](<C:/Users/tnest/Desktop/LostArk/Server/Public/GameplayCatalog.h:693>)

```cpp
enum class BOSS_LOGIC_REGION_ANCHOR : std::uint8_t
	{
		WORLD,
		BOSS_CURRENT,
		BOSS_SPAWN,
		BOSS_START
	};
```

### BOSS_LOGIC_WORLD_TRANSFORM_KEY

[Server/Public/GameplayCatalog.h](<C:/Users/tnest/Desktop/LostArk/Server/Public/GameplayCatalog.h:703>)

```cpp
struct BOSS_LOGIC_WORLD_TRANSFORM_KEY final
	{
		std::uint32_t iTimeMs = 0u;
		float fOffsetX = 0.f,
			fOffsetY = 0.f,
			fOffsetZ = 0.f;
		float fRotationY = 0.f,
			fRotationW = 1.f;
		float fScaleX = 1.f,
			fScaleY = 1.f,
			fScaleZ = 1.f;
		bool bVisible = true;
		bool bHasGripPosition = false;
		// 월드 m 단위로 구운 물리 잡기 지점이다. bHasGripPosition이 있을 때 사용한다.
		std::array<float, 3u> GripPosition{};
	};
```

### BOSS_LOGIC_WORLD_TRANSFORM_TRACK

[Server/Public/GameplayCatalog.h](<C:/Users/tnest/Desktop/LostArk/Server/Public/GameplayCatalog.h:722>)

```cpp
struct BOSS_LOGIC_WORLD_TRANSFORM_TRACK final
	{
		bool bEnabled = false;
		std::uint32_t iStartMs = 0u,
			iStartDelayMs = 0u,
			iDurationMs = 0u;
		float fPlaybackSpeed = 1.f;
		bool bSmoothStep = false;
		float fBaselineX = 0.f,
			fBaselineY = 0.f,
			fBaselineZ = 0.f;
		float fBaselineYawDegrees = 0.f;
		float fBaselineScaleX = 1.f,
			fBaselineScaleY = 1.f,
			fBaselineScaleZ = 1.f;
		std::vector<BOSS_LOGIC_WORLD_TRANSFORM_KEY> Keys;
	};
```

### BOSS_LOGIC_REGION

[Server/Public/GameplayCatalog.h](<C:/Users/tnest/Desktop/LostArk/Server/Public/GameplayCatalog.h:743>)

```cpp
struct BOSS_LOGIC_REGION final
	{
		std::string strRegionId;
		BOSS_LOGIC_REGION_ANCHOR eAnchor = BOSS_LOGIC_REGION_ANCHOR::WORLD;
		bool bSector = false;
		bool bReverseSector = false;
		bool bCircle = false;
		bool bCylinder = false;
		bool bLinearMotion = false;
		std::uint32_t iMotionStartMs = 0u,
			iMotionDurationMs = 0u;
		// 미설정이면 해당 region 판정창의 시작 시각을 사용한다.
		std::uint32_t iAnchorCaptureStartMs = 0xffffffffu;
		std::array<float, 3u> EndPositionOffset{};
		std::array<float, 3u> EndScale{ 1.f, 1.f, 1.f };
		BOSS_LOGIC_WORLD_TRANSFORM_TRACK WorldTrack;
		float fCenterX = 0.f,
			fCenterY = 0.f,
			fCenterZ = 0.f;
		float fYawDegrees = 0.f;
		float fHalfX = 1.f,
			fHalfY = 1.f,
			fHalfZ = 1.f;
		float fRadiusM = 1.f,
			fInnerRadiusM = 0.f,
			fHalfAngleDegrees = 45.f;
		// 부채꼴의 선택적인 X/Z 반경이다. 0이면 기본 반경을 사용한다.
		float fRadiusXM = 0.f,
			fRadiusZM = 0.f;
		LostArk::Shared::MECHANIC_CARD_SYMBOL eCardSymbol = LostArk::Shared::MECHANIC_CARD_SYMBOL::NONE;
		LostArk::Shared::MECHANIC_CARD_COLOR eCardColor = LostArk::Shared::MECHANIC_CARD_COLOR::NONE;
	};
```

### BOSS_LOGIC_CONTACT_TARGET

[Server/Public/GameplayCatalog.h](<C:/Users/tnest/Desktop/LostArk/Server/Public/GameplayCatalog.h:777>)

```cpp
struct BOSS_LOGIC_CONTACT_TARGET final
	{
		std::string strWorldOccurrenceId;
		std::string strWorldInstanceId;
		float fWorldX = 0.f,
			fWorldZ = 0.f,
			fRadiusM = 0.f;
	};
```

### BOSS_PATTERN_LOGIC_WINDOW

[Server/Public/GameplayCatalog.h](<C:/Users/tnest/Desktop/LostArk/Server/Public/GameplayCatalog.h:788>)

```cpp
struct BOSS_PATTERN_LOGIC_WINDOW final
	{
		std::string strWindowId;
		std::vector<std::string> PatternIds;
		std::uint32_t iCompletionCount = 0u;
		BOSS_PATTERN_LOGIC_KIND eKind = BOSS_PATTERN_LOGIC_KIND::ROULETTE_CARD_MATCH;
		std::uint32_t iStartMs = 0u;
		std::uint32_t iDurationMs = 0u;
		std::uint32_t iSectorCount = 0u;
		float fCenterX = 0.f;
		float fCenterZ = 0.f;
		float fOuterRadiusM = 0.f;
		float fStopYawDegrees = 0.f;
		std::vector<LostArk::Shared::MECHANIC_CARD_SYMBOL> SectorSymbols;
		float fHalfAngleDegrees = 0.f;
		float fMaxDistanceM = 0.f;
        bool bGazeDuringWindow = false;
		std::uint32_t iPoseIndex = 0u;
		std::uint32_t iThreshold = 0u;
		float fShieldArcDegrees = 0.f;
		float fNormalYawOffsetDegrees = 0.f;
		std::vector<BOSS_LOGIC_REGION> CardRegions;
		bool bInsideIsFail = false;
		bool bRearmOnExit = false;
		bool bRepeatAfterKnockback = false;
		std::uint32_t iRepeatIntervalMs = 0u;
		bool bEndsPatternOnSuccess = false;
		// 부모 일정이 자식 창을 잘라 끝낸 경우 결과 판정 없이 닫는다.
		bool bCancelAtEnd = false;
		std::string strOwnerWorldOccurrenceId;
		std::vector<BOSS_PATTERN_LOGIC_RESULT> OnSuccess;
		std::vector<BOSS_PATTERN_LOGIC_RESULT> OnFail;
		std::vector<BOSS_PATTERN_LOGIC_RESULT> OnTimeout;
		std::string strTargetWorldInstanceId;
		float fTargetWorldX = 0.f,
			fTargetWorldZ = 0.f;
		float fTargetRadiusM = 0.f;
		std::vector<BOSS_LOGIC_CONTACT_TARGET> ContactTargets;
		std::string strContactGroupId;
		std::uint32_t iContactPriority = 0u;
		bool bHasContactGroup = false;
		float fBossChargeDistanceM = 0.f;
        float fChargeYawOffsetDegrees = 0.f;
        std::string strHoldLogicOccurrenceId;
	};
```

## 3. 모든 Server Logic kind의 실제 실행 분기

영문 kind를 enum으로 바꾸는 parser와 실제 처리자는 별도 단계다. 판정 종류가 16개라고 모든 종류가 Update의 동일 switch 안에서 처리되는 것은 아니다. 아래는 현재 지원되는 전체 목록이다.

| Server kind | 무엇을 판단/유지하는가 | 주요 소비 위치 |
|---|---|---|
|CARD_DICE_BIND|자유 플레이어 한 명을 정하고 다른 플레이어를 묶어 추적 대상 유지|Open_Window 1173, Update 2101|
|ROULETTE_CARD_MATCH|플레이어 카드와 룰렛 영역 문양 일치|Update 2190|
|GAZE_REAL_BOSS|실제 boss 방향을 보는지, 내부/외부 결과 정책|Update 2191|
|POSE_INPUT|열린 창 동안 서버가 받은 interaction slot을 포즈 답으로 기록|Record_InteractionSlot 2442, Update 2404|
|STAGGER_WINDOW|무력화 누적과 threshold; 성공/시간 만료|Open_Window 1228, Update 2154|
|AREA_OVERLAP|종료 시 또는 반복 간격의 플레이어 영역 겹침|Update 2265|
|ENTER_AREA|접촉 진입, 재무장/넉백/반복 정책, Mario 예외|Update 2266|
|OBJECT_OVERLAP|하나의 대상 World 원과 region이 겹치는지|Update 2240|
|OBJECT_CONTACT|여러 World occurrence 접촉을 priority로 처리|Update 1983의 선처리; 일반 switch 2087은 이미 처리되어 break|
|EXTERNAL_SIGNAL|다른 contact의 COMPLETE_LOGIC_WINDOW 신호 성공, 없으면 timeout|Update 2030 선처리 / 2092 timeout|
|COUNTER_WINDOW|counterable 상태와 검증된 카운터 착지|Open_Window 1222, Update 2105|
|ATTACHMENT_HOLD|포획 결과가 참조하는 유효 기간; 자체 region/result 없음|Build 157, Update 2089, Room capture commit|
|PATTERN_COMPLETION_COUNT|자식 패턴 완료 수와 Mario 복귀를 이어주는 chain|GameRoom_KoukuAudition::Start_KoukuCompletionChain 및 Queue_KoukuCompletionChainSuccess|
|INVULNERABILITY_ZONE|해당 발생에서 보호 플레이어 집합을 먼저 수집|Update 1955 선처리 / 2084 종료|
|BINGO_COMPLETED_LINES|빙고 완성 줄 수로 성공/실패|Update 2056|
|BOSS_DAMAGE_REDUCTION|열린 창의 피해 감소 참조 수를 증가/정리|Open_Window 1225 / Close_Window 1263|

| RESULT kind | 실제 처리 책임 |
|---|---|
|NONE|C++ 기본값/무동작. 저작 OUTCOME_KINDS에는 없는 parser 허용값|
|INSTANT_DEATH / MAX_HP_PERCENT_DAMAGE / FIXED_DAMAGE|Apply_Result에서 피해량을 만들고 공통 player hit 경로에 적용|
|MADNESS_GAUGE_ADD_PERCENT|광기 상태 증가. 같은 결과 목록에 피해가 있으면 피해를 먼저 처리|
|CLOWN_TRANSFORM|Transform_ToClown으로 상태/만료 시각 설정|
|FEAR|공포 상태와 표시 요청 갱신|
|PLAYER_INVULNERABILITY|플레이어 무적 만료 tick 설정|
|FOLLOWUP_PATTERN|Apply_Results가 output.FollowupPatternIds에 넣고 Room이 scheduler에 삽입|
|PLAY_WORLD_OBJECT_MOTION|Apply_Results가 지정 instance를 대상으로 WorldSequencePlays 생성|
|PLAY_CONTACT_WORLD_OBJECT_MOTION|OBJECT_CONTACT 선처리가 실제 맞은 occurrence의 매핑만 선택|
|COMPLETE_LOGIC_WINDOW|OBJECT_CONTACT 선처리에서 신호 집합에 추가 후 EXTERNAL_SIGNAL 성공|
|CAPTURE_PLAYER|Apply_Results가 capture 후보 요청; Room이 소유 Hold/sequence/현재 자격 재검증 후 부착 확정|
|GRAB_TO_WORLD_OBJECT|ENTER_AREA 판정이 실제 맞은 region을 attachment carrier로 기록, 이후 Drag_HookedPlayer로 추적|
|MARIO_ENTER|Room이 진입 예약·다른 플레이어 실패/전이·복귀를 관리|

이 분리는 RESULT 하나가 곧 별도 C++ 클래스 하나라는 뜻이 아니다. 서로 다른 layer의 권한을 지키기 위해 enum 분기가 직접 상태를 바꾸거나 output 요청을 만드는 방식으로 나뉜다.

- [Server/Private/KoukuSaydonLogicRuntime.cpp](<C:/Users/tnest/Desktop/LostArk/Server/Private/KoukuSaydonLogicRuntime.cpp:1750>)
- [Server/Private/KoukuSaydonLogicRuntime.cpp](<C:/Users/tnest/Desktop/LostArk/Server/Private/KoukuSaydonLogicRuntime.cpp:1438>)
- [Server/Private/KoukuSaydonLogicRuntime.cpp](<C:/Users/tnest/Desktop/LostArk/Server/Private/KoukuSaydonLogicRuntime.cpp:1624>)
- [Server/Private/GameRoom_KoukuAudition.cpp](<C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom_KoukuAudition.cpp:1250>)
- [Server/Private/GameRoom_KoukuAudition.cpp](<C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom_KoukuAudition.cpp:1483>)

### Mechanic trigger 쪽의 전체 enum

이 목록은 LogicWindow와 별도 vector다. HUD_ENTER / CARD_MAZE_STAGE_PLAYERS / CARD_MAZE_HIDE_NEXT / CARD_MAZE_ENTER는 LogicRuntime::Update 시작 cue 단계에서 처리한다. SHOWTIME_PLAYER_TARGETS / BOSS_TRACK_TARGET / PURSUIT_PROJECTILES는 PlayerTargetWindows를 만들어 GameRoom_KoukuTargeting에서 지속 평가한다. BOSS_RANDOM_TARGET_PRESENTATION은 시작 대상 선택과 지속 표시 추적을 함께 가진다.

나머지 REAL_GAZE_TELEPORT / SUMMON_PATTERNS / ALBION_BLUE_CIRCLE / ALBION_AIRBORNE / BOSS_TELEPORT_XZ / BOSS_TELEPORT_FACE_CENTER / BOSS_TELEPORT_GROUNDED / MARIO_PHASE2_PLAYERS / CROSS_DIRECTION_CLONES / BINGO_BOARD / BINGO_DETONATION / CARD_RAIN_SOLDIERS / BOSS_RANDOM_TARGET 등은 output → PendingKoukuMechanicTriggers → `Commit_KoukuMechanicTriggers`에서 상태/생성을 확정한다. 따라서 TRIGGER라는 단어만 보고 모두 단일 즉시 callback이라고 이해하면 일부 지속 추적을 놓친다.

### BOSS_PATTERN_MECHANIC_TRIGGER_KIND

[Server/Public/GameplayCatalog.h](<C:/Users/tnest/Desktop/LostArk/Server/Public/GameplayCatalog.h:834>)

```cpp
enum class BOSS_PATTERN_MECHANIC_TRIGGER_KIND : std::uint8_t
	{
		HUD_ENTER,
		REAL_GAZE_TELEPORT,
		CARD_MAZE_HIDE_NEXT,
		CARD_MAZE_ENTER,
		ALBION_BLUE_CIRCLE,
		SUMMON_PATTERNS,
		SHOWTIME_PLAYER_TARGETS,
		BOSS_TELEPORT_XZ,
		BOSS_TELEPORT_FACE_CENTER,
		MARIO_PHASE2_PLAYERS,
		BOSS_TRACK_TARGET,
		ALBION_AIRBORNE,
		CROSS_DIRECTION_CLONES,
		PURSUIT_PROJECTILES,
		BINGO_BOARD,
		BINGO_DETONATION,
		BOSS_TELEPORT_GROUNDED,
		CARD_MAZE_STAGE_PLAYERS,
		CARD_RAIN_SOLDIERS,
		BOSS_RANDOM_TARGET,
		BOSS_RANDOM_TARGET_PRESENTATION
	};
```

- [Server/Private/KoukuSaydonLogicRuntime.cpp](<C:/Users/tnest/Desktop/LostArk/Server/Private/KoukuSaydonLogicRuntime.cpp:129>)
- [Server/Private/GameRoom_KoukuMechanics.cpp](<C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom_KoukuMechanics.cpp:338>)
- [Server/Private/GameRoom_KoukuTargeting.cpp](<C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom_KoukuTargeting.cpp:332>)

## 4. Begin / Stage clock / Logic clock

`Begin_Pattern`은 room이 만든 boss 후보 복사본에 대해 revision, tick, 생존, 기존 실행 부재, 쿠크 정의 유효성을 확인한다. 통과하면 `strPatternId/iPatternStartTick/iPatternSequence`를 고정하고 첫 Stage에 들어간다. sequence는 같은 pattern ID가 다시 실행돼도 발생을 구분하며 예약값 0을 건너뛴다. 여기서 network 송신이나 다른 entity spawn까지 실행하지 않는다.

Stage에는 두 시계가 있다. `bFixedTimelineClock=true`면 전체 stage의 누적 ms 경계와 `patternElapsedTicks*1000`을 비교한다. 매 stage의 길이를 각각 tick 올림해 누적하지 않으므로 장기 드리프트를 줄이고, 늦게 호출돼 여러 경계가 지났으면 현재 누적 위치의 stage를 고른다. false면 현재 action start부터 경과를 보며 한 호출에 최대 한 stage 진행한다. `iActionStartTick/fActionElapsedSeconds/strActionId`가 snapshot을 통해 Client clip 재생 시점에 연결된다.

LogicWindow는 Stage 진입 이벤트에 종속되지 않는다. `Build`가 **patternStartTick + ceil(startMs*30/1000)**와 **patternStartTick + ceil((startMs+durationMs)*30/1000)**로 window의 절대 시작/끝 tick을 만든다. 기간을 따로 올림해서 더하는 식과 다르다. 시작을 다른 Stage로 바꾼다고 window가 따라 움직이는 것이 아니므로 편집할 때 각 lane의 시간 정렬을 확인해야 한다.

`Update_KoukuSaydonBoss`는 현재 tick의 BossMotion/StageRootMotion을 적용한 후 LogicRuntime을 평가하고, 결과를 Room에 확정한 다음 Brain::Update로 stage 완료를 진행한다. 독립 lane이 animation보다 늦게 끝나는 경우 Retain_KoukuPatternTail/Update_KoukuPatternTails가 소유자 복사본과 ledger를 남길 수 있다. early 성공/취소에서는 별도 종료 정리가 적용된다.

[Server/Private/KoukuSaydonBrain.cpp](<C:/Users/tnest/Desktop/LostArk/Server/Private/KoukuSaydonBrain.cpp:1258>) — 1258~1290행. 아래는 현재 제품 소스 원문이다.

```cpp
bool LostArk::Server::CKoukuSaydonBrain::Begin_Pattern(
	SERVER_WORLD_ENTITY& boss,
	const BOSS_PATTERN_DEFINITION& pattern,
	const LostArk::Shared::GameplayDataRevision& revision,
	const std::uint32_t serverTick,
	std::string& status) const
{
	if (!revision.Is_Valid() || 0u == serverTick || 0u == boss.iCurrentHp ||
		SERVER_ENTITY_ACTION::DEAD == boss.eAction || !boss.strPatternId.empty() ||
		KOUKUSAYDON_G1_ENCOUNTER_ID != boss.strEncounterId || !Validate_AnimationOnlyPattern(pattern, status))
	{
		if (status.empty())
			status = "KoukuSaydon pattern cannot begin from the current boss state";
		return false;
	}
	// 새 occurrence의 접촉/방향/공중/소환 시작 기록을 비우고 revision과 sequence를 고정한다. sequence도 예약값 0을 건너뛴다.
	boss.Restore_KoukuPresentationAim();
	boss.PinnedDefinitionRevision = revision;
	boss.KoukuContactLedger.reset();
	boss.KoukuDirectionPlayback.reset();
	boss.iKoukuDirectionEndTick = 0u;
	boss.bKoukuDirectionPlaybackComplete = false;
	boss.PatternTerminalReceipt = {};
	boss.bPatternRootGrounded = false;
	boss.AlbionAirborne = {};
	boss.KoukuSummonStartedTriggers.clear();
	boss.strPatternId = pattern.strPatternId;
	boss.iPatternStartTick = serverTick;
	boss.iPatternSequence =
		boss.iPatternSequence == (std::numeric_limits<std::uint32_t>::max)() ? 1u : boss.iPatternSequence + 1u;
	Enter_Stage(boss, pattern.Stages.front(), 0u, serverTick, true);
	Apply_BossMotion(boss, pattern, serverTick);
	status.clear();
```

[Server/Private/KoukuSaydonBrain.cpp](<C:/Users/tnest/Desktop/LostArk/Server/Private/KoukuSaydonBrain.cpp:1364>) — 1364~1402행. 아래는 현재 제품 소스 원문이다.

```cpp
	{
		// 각 stage마다 올림한 기간을 누적하지 않고 pattern 공통 시계의 누적 ms 경계와 비교한다.
		// Logic/이펙트와 같은 시작 시계를 사용하며, 지연된 호출은 이미 지난 여러 stage 경계를 한 번에 넘길 수 있다.
		const std::uint64_t ticks = serverTick >= boss.iPatternStartTick
			? serverTick - boss.iPatternStartTick
			: static_cast<std::uint64_t>((std::numeric_limits<std::uint32_t>::max)() - boss.iPatternStartTick) +
				serverTick;
		std::uint64_t startMs = 0u;
		std::uint32_t selected = 0u;
		while (selected < pattern->Stages.size() &&
			   ticks * MILLISECONDS_PER_SECOND >=
				   (startMs + pattern->Stages[selected].iDurationMs) * SERVER_TICK_HZ)
			startMs += pattern->Stages[selected++].iDurationMs;
		if (selected == pattern->Stages.size())
		{
			Finish_Pattern(boss, serverTick, SERVER_BOSS_PATTERN_TERMINAL_RESULT::COMPLETED);
			status.clear();
			return KOUKUSAYDON_BRAIN_UPDATE_RESULT::PATTERN_COMPLETED;
		}
		const bool changed = selected != boss.iPatternStageIndex;
		if (changed)
			Enter_Stage(boss, pattern->Stages[selected], selected, serverTick, false);
		const auto offset = static_cast<std::uint32_t>(
			(startMs * SERVER_TICK_HZ + MILLISECONDS_PER_SECOND - 1u) / MILLISECONDS_PER_SECOND);
		boss.iActionStartTick = boss.iPatternStartTick + offset;
		if (boss.iActionStartTick < boss.iPatternStartTick)
			++boss.iActionStartTick;
		if (boss.iActionStartTick == 0u)
			boss.iActionStartTick = 1u;
		boss.fActionElapsedSeconds =
			static_cast<float>((std::max)(0.0,
										  static_cast<double>(ticks) / SERVER_TICK_HZ -
											  static_cast<double>(startMs) / MILLISECONDS_PER_SECOND));
		status.clear();
		return changed ? KOUKUSAYDON_BRAIN_UPDATE_RESULT::STAGE_CHANGED
					   : KOUKUSAYDON_BRAIN_UPDATE_RESULT::RUNNING;
	}
	const std::uint64_t elapsedTicks = Stage_ElapsedTicks(boss, serverTick);
	boss.fActionElapsedSeconds = static_cast<float>(elapsedTicks) / static_cast<float>(SERVER_TICK_HZ);
```

[Server/Private/KoukuSaydonLogicRuntime.cpp](<C:/Users/tnest/Desktop/LostArk/Server/Private/KoukuSaydonLogicRuntime.cpp:153>) — 153~167행. 아래는 현재 제품 소스 원문이다.

```cpp
					break;
				}
		state.iStartTick = Add_Ticks(startTick, Ticks_FromMs(window.iStartMs));
		state.iEndTick = Add_Ticks(startTick,
			Ticks_FromMs(window.iStartMs + window.iDurationMs));
        if (!window.strHoldLogicOccurrenceId.empty())
        {
            const auto hold = std::find_if(pattern.LogicWindows.begin(), pattern.LogicWindows.end(), [&](const auto& row) {
                return row.strWindowId == window.strHoldLogicOccurrenceId && row.eKind == BOSS_PATTERN_LOGIC_KIND::ATTACHMENT_HOLD;
            });
            if (hold != pattern.LogicWindows.end())
            {
                state.iHoldStartTick = Add_Ticks(startTick, Ticks_FromMs(hold->iStartMs));
                state.iHoldEndTick = Add_Ticks(startTick, Ticks_FromMs(hold->iStartMs + hold->iDurationMs));
            }
```

### Ledger 필드와 정리 수명

WindowState의 `iWindowIndex`는 고정된 정의 vector 조회용이며 저장 stable ID 자체가 아니다. `iStartTick/iEndTick/iHoldStartTick/iHoldEndTick`은 절대 tick. `bOpened/bClosed`는 중복 진입/정리 방지, `bChargeCaptured/bChargeStopped/ChargeMotion`은 돌진의 1회 목표 고정과 경로 상태다. `AppliedWorldMotions`는 occurrence+motion 중복 방지; `ContactedWorldOccurrences`는 이 창이 이미 접촉한 World 발생; `Answers`는 플레이어별 SUCCESS/FAIL/TIMEOUT; `InsidePlayers/NextContactHitTicks`는 재접촉/주기 판정; `FrozenRegions`는 BOSS_START의 고정된 도형이다. `iMadnessSource/iMadnessContactIntervalMs`는 실제 반복접촉 광기 출처, `iFreePlayerNetEntityId/BoundPlayers`는 주사위 구속의 대상과 해제 소유권이다.

Ledger의 `strPatternId/iPatternSequence/iPatternStartTick`은 실행 identity. `bDanceActive/ScheduledDanceIntervals/eHudMode`는 포즈 HUD; `Windows`가 각 창 상태를 소유한다. `RetiredWorldOccurrences`는 파괴된 owner의 판정 금지 집합. `iBingoLineJudgementTick/iBingoCompletedLines/bBingoLineRewardApplied`는 줄 판정과 보상 중복 방지. `ContactWindowOrder`는 priority 정렬 결과; `ConsumedContactGroups`는 group+strike-start+target occurrence 소비 집합; `AppliedContactMotionPriorities`는 높은 우선순위 motion이 낮은 motion으로 덮이지 않도록 유지한다. `WorldSequences/MechanicTriggers/PlayerTargetWindows`는 예정 cue/지속 tracking 상태. `bTrackingTargetReached`는 조기 종료 요청. 카드미로 roster 배열과 captured/next index는 당시 입장 순서를 고정한다.

Build는 새 지역 ledger를 만든 뒤 outLedger에 이동한다. Discard는 소유 bind/상태를 정리하고 ledger를 비운다. 공유 Catalog의 definition vector에 이 실행 상태를 기록하지 않으므로 같은 정의를 다른 actor/member가 사용해도 결과가 섞이지 않는다.

### KOUKUSAYDON_LOGIC_WINDOW_STATE

[Server/Public/KoukuSaydonLogicRuntime.h](<C:/Users/tnest/Desktop/LostArk/Server/Public/KoukuSaydonLogicRuntime.h:30>)

```cpp
struct KOUKUSAYDON_LOGIC_WINDOW_STATE final
	{
		std::uint32_t iWindowIndex = 0u;
		std::uint32_t iStartTick = 0u;
		std::uint32_t iEndTick = 0u;
        std::uint32_t iHoldStartTick = 0u, iHoldEndTick = 0u;
		bool bOpened = false;
		bool bClosed = false;
		bool bChargeCaptured = false;
		bool bChargeStopped = false;
		std::optional<BOSS_PATTERN_BOSS_MOTION> ChargeMotion;
		std::set<std::pair<std::string, std::string>> AppliedWorldMotions;
		std::set<std::string> ContactedWorldOccurrences;
		// 주기적으로 접촉하는 공·인형의 WORLD 소유자를 구분한다. 나머지 피해는 HP 기반 광기 증가를 따른다.
		std::uint8_t iMadnessSource = 0u;
		std::uint32_t iMadnessContactIntervalMs = 0u;
		std::map<LostArk::Shared::PLAYER_ID, KOUKUSAYDON_LOGIC_ANSWER> Answers;
		std::set<LostArk::Shared::PLAYER_ID> InsidePlayers;
		std::map<LostArk::Shared::PLAYER_ID, std::uint32_t> NextContactHitTicks;
		std::map<std::string, BOSS_LOGIC_REGION> FrozenRegions;
		LostArk::Shared::NET_ENTITY_ID iFreePlayerNetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
		std::map<LostArk::Shared::PLAYER_ID, LostArk::Shared::NET_ENTITY_ID> BoundPlayers;
	};
```

### KOUKUSAYDON_LOGIC_CUE_STATE

[Server/Public/KoukuSaydonLogicRuntime.h](<C:/Users/tnest/Desktop/LostArk/Server/Public/KoukuSaydonLogicRuntime.h:55>)

```cpp
struct KOUKUSAYDON_LOGIC_CUE_STATE final
	{
		std::uint32_t iIndex = 0u;
		std::uint32_t iStartTick = 0u;
		bool bStarted = false;
	};
```

### KOUKUSAYDON_PLAYER_TARGET_WINDOW_STATE

[Server/Public/KoukuSaydonLogicRuntime.h](<C:/Users/tnest/Desktop/LostArk/Server/Public/KoukuSaydonLogicRuntime.h:62>)

```cpp
struct KOUKUSAYDON_PLAYER_TARGET_WINDOW_STATE final
	{
		std::uint32_t iTriggerIndex = 0u;
		std::uint32_t iStartTick = 0u, iEndTick = 0u, iNextFixedTick = 0u;
		std::uint32_t iLastUpdateTick = 0u;
		std::uint32_t iNextRandomTick = 0u, iRandomWaveOrdinal = 0u;
		std::uint32_t iBombStartTick = 0u;
		LostArk::Shared::COMBAT_OBJECT_ID iBombBodyObjectId = 0u;
		std::optional<bool> BombInsideAtEnd;
		LostArk::Shared::NET_ENTITY_ID iSelectedTargetNetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
		std::optional<std::array<float, 3u>> LastSelectedTargetPosition;
		bool bClosed = false;
		std::map<LostArk::Shared::PLAYER_ID, LostArk::Shared::COMBAT_OBJECT_ID> TrackingObjects;
	};
```

### KOUKUSAYDON_LOGIC_LEDGER

[Server/Public/KoukuSaydonLogicRuntime.h](<C:/Users/tnest/Desktop/LostArk/Server/Public/KoukuSaydonLogicRuntime.h:78>)

```cpp
struct KOUKUSAYDON_LOGIC_LEDGER final
	{
		std::string strPatternId;
		std::uint32_t iPatternSequence = 0u;
		std::uint32_t iPatternStartTick = 0u;
		bool bDanceActive = false;
		std::vector<std::pair<std::uint32_t, std::uint32_t>> ScheduledDanceIntervals;
		LostArk::Shared::KOUKU_HUD_MODE eHudMode = LostArk::Shared::KOUKU_HUD_MODE::NONE;
		std::vector<KOUKUSAYDON_LOGIC_WINDOW_STATE> Windows;
		std::set<std::string> RetiredWorldOccurrences;
		std::uint32_t iBingoLineJudgementTick = 0u, iBingoCompletedLines = 0u;
		bool bBingoLineRewardApplied = false;
		std::vector<std::uint32_t> ContactWindowOrder;
		std::set<std::tuple<std::string, std::uint32_t, std::string>> ConsumedContactGroups;
		// 이번 패턴에서 먼저 적용된 높은 우선순위의 카드 상태를 낮은 우선순위 반응이 덮지 못하게 한다.
		std::map<std::pair<std::string, std::string>, std::uint32_t> AppliedContactMotionPriorities;
		std::vector<KOUKUSAYDON_LOGIC_CUE_STATE> WorldSequences;
		std::vector<KOUKUSAYDON_LOGIC_CUE_STATE> MechanicTriggers;
		std::vector<KOUKUSAYDON_PLAYER_TARGET_WINDOW_STATE> PlayerTargetWindows;
		bool bTrackingTargetReached = false;
		// 입장 시 roster를 world X 내림차순, 동률이면 PlayerId 순으로 한 번 캡처한다.
		bool bCardMazeEntryRosterCaptured = false;
		std::vector<LostArk::Shared::PLAYER_ID> CardMazeEntryPlayers;
		std::size_t iCardMazeNextHiddenPlayer = 0u;

		[[nodiscard]] bool Is_Active() const noexcept { return !strPatternId.empty(); }
	};
```

### KOUKUSAYDON_LOGIC_WORLD_PLAY

[Server/Public/KoukuSaydonLogicRuntime.h](<C:/Users/tnest/Desktop/LostArk/Server/Public/KoukuSaydonLogicRuntime.h:106>)

```cpp
struct KOUKUSAYDON_LOGIC_WORLD_PLAY final
	{
		std::string strInstanceId;
		float fPlaybackSpeed = 1.f;
		float fPositionOffsetX = 0.f;
		float fPositionOffsetY = 0.f;
		float fPositionOffsetZ = 0.f;
		std::uint32_t iDurationMs = 0u;
		std::string strTargetSequenceInstanceId;
		std::string strOccurrenceId;
		std::uint32_t iStartTick = 0u;
		std::string strTargetWorldOccurrenceId;
		bool bAuthoredMadness = false;
		std::optional<BOSS_PATTERN_WORLD_COMBAT_BODY> CombatBody;
		std::optional<BOSS_PATTERN_WORLD_PLACEMENT> Placement;
	};
```

### KOUKUSAYDON_LOGIC_CAPTURE_REQUEST

[Server/Public/KoukuSaydonLogicRuntime.h](<C:/Users/tnest/Desktop/LostArk/Server/Public/KoukuSaydonLogicRuntime.h:123>)

```cpp
struct KOUKUSAYDON_LOGIC_CAPTURE_REQUEST final
    {
        LostArk::Shared::NET_ENTITY_ID iPlayerNetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
        LostArk::Shared::PLAYER_ATTACHMENT_SLOT eAttachmentSlot = LostArk::Shared::PLAYER_ATTACHMENT_SLOT::NONE;
        std::uint32_t iHoldEndTick = 0u;
        std::uint32_t iWindowIndex = 0u;
    };
```

### KOUKUSAYDON_LOGIC_OUTPUT

[Server/Public/KoukuSaydonLogicRuntime.h](<C:/Users/tnest/Desktop/LostArk/Server/Public/KoukuSaydonLogicRuntime.h:132>)

```cpp
struct KOUKUSAYDON_LOGIC_OUTPUT final
	{
		std::vector<KOUKUSAYDON_LOGIC_WORLD_PLAY> WorldSequencePlays;
		std::vector<BOSS_PATTERN_MECHANIC_TRIGGER> MechanicTriggers;
		std::vector<std::string> FollowupPatternIds;
        std::vector<KOUKUSAYDON_LOGIC_CAPTURE_REQUEST> CaptureRequests;
		bool bEndPatternEarly = false;
		// 패턴을 끝내는 counter의 검증된 착지가 commit된 뒤에만 설정한다.
		bool bCounterSuccessLanded = false;
		bool bStaggerSuccess = false;
		std::optional<bool> BingoLineCompletion;
		std::string strStatus;
	};
```

## 5. Room 권위와 bundle / parent / raid

수신 thread는 packet을 검증해 ROOM_COMMAND payload를 만들고 Enqueue_Detailed로 넣는다. queue mutex는 입력 전달을 보호한다. 실제 player/boss/container 변경은 room worker의 Tick이 명령을 가져와 수행한다. 큐에 들어갔다는 사실은 패턴이 성공적으로 시작했다는 뜻이 아니다.

Evaluate_KoukuSaydonPatternAudition은 scope/revision/요청 순번/살아 있는 placement/중복 boss/모든 후속 정의를 확인하고 staged run을 만든다. bundle member는 각기 boss entity와 pattern 목록을 가지며 **commonStartTick + ceil(startOffsetMs*30/1000)**로 시작 예약된다. Prepare_KoukuAuditionTick은 이번 tick에 시작할 모든 boss 복사본, ledger, 지원 지면을 준비한 다음 due actor들을 먼저 확정한다. 그 뒤에만 각 member가 첫 판정을 한다. 일부 멤버만 검사하기 전에 live 실행을 시작하는 구조가 아니다.

`Update_WorldEntities`의 새 파일은 Prepare → Mario entry commit → tail/clone → entity 순회 → despawn → mechanic commit 순서를 유지한다. 일반 boss tick에서 LogicRuntime의 output은 `Apply_KoukuLogicOutput`으로 넘어간다. 포획은 Hold/sequence/자격 재검사 후 확정; 소환/순간이동은 pending mechanic으로 보내 순회 뒤 확정; 후속 pattern은 member 목록 뒤에 삽입; 종료 플래그는 Brain Complete로 연결된다.

Parent는 자식 pattern ID/occurrence/start/duration을 보관하고 `Start_KoukuParentSequence`가 기존 member scheduler에 자식 목록과 대기 tick을 삽입한다. animation stage로 모두 합친 한 거대 actor clip이 아니다. 반복 Parent는 loop index를 갖고, 비반복 Parent는 완료 수를 관리한다. PATTERN_COMPLETION_COUNT도 Room chain 경로를 쓴다.

Raid는 그 위의 관문 진행이다. `KOUKU_RAID_GATE_DEFINITION`의 Entries가 pattern 또는 bundle을 참조하며, EntryGroups는 HP 조건 반복 구간을 지정한다. `Update_KoukuRaid`가 PREPARING/CINEMATIC/전투/관문 전환을 관리한다. 준비 때 전원 ready와 pinned revision을 확인하고 같은 audition/bundle 실행 경로로 진입한다. actor Stage, member Pattern, bundle, Parent, raid flow는 서로 다른 진행 단위다.

- [Server/Private/ServerApp.cpp](<C:/Users/tnest/Desktop/LostArk/Server/Private/ServerApp.cpp:3117>)
- [Server/Private/GameRoom.cpp](<C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom.cpp:277>)
- [Server/Private/GameRoom.cpp](<C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom.cpp:858>)
- [Server/Private/GameRoom_KoukuAudition.cpp](<C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom_KoukuAudition.cpp:86>)
- [Server/Private/GameRoom_KoukuAudition.cpp](<C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom_KoukuAudition.cpp:1055>)
- [Server/Private/GameRoom_BossSimulation.cpp](<C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom_BossSimulation.cpp:8>)
- [Server/Private/GameRoom_WorldEntitySimulation.cpp](<C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom_WorldEntitySimulation.cpp:17>)
- [Server/Private/GameRoom_KoukuAudition.cpp](<C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom_KoukuAudition.cpp:1533>)
- [Server/Private/GameRoom_KoukuRaidFlow.cpp](<C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom_KoukuRaidFlow.cpp:607>)

### BOSS_PATTERN_BUNDLE_MEMBER

[Server/Public/GameplayCatalog.h](<C:/Users/tnest/Desktop/LostArk/Server/Public/GameplayCatalog.h:1303>)

```cpp
struct BOSS_PATTERN_BUNDLE_MEMBER final
	{
		std::string strMemberId;
		std::string strPatternId;
		std::string strTargetBossPlacementId;
		std::uint32_t iStartOffsetMs = 0u;
	};
```

### BOSS_PATTERN_BUNDLE_DEFINITION

[Server/Public/GameplayCatalog.h](<C:/Users/tnest/Desktop/LostArk/Server/Public/GameplayCatalog.h:1311>)

```cpp
struct BOSS_PATTERN_BUNDLE_DEFINITION final
	{
		std::string strBundleId;
		std::string strEncounterId;
		std::string strGateId;
		std::vector<BOSS_PATTERN_BUNDLE_MEMBER> Members;
	};
```

### BOSS_PATTERN_PARENT_CHILD

[Server/Public/GameplayCatalog.h](<C:/Users/tnest/Desktop/LostArk/Server/Public/GameplayCatalog.h:1423>)

```cpp
struct BOSS_PATTERN_PARENT_CHILD final
    {
        std::string strOccurrenceId;
        std::string strPatternId;
        std::uint32_t iStartMs = 0u;
        std::uint32_t iDurationMs = 0u;
    };
```

### KOUKU_RAID_FLOW_ENTRY

[Server/Public/GameplayCatalog.h](<C:/Users/tnest/Desktop/LostArk/Server/Public/GameplayCatalog.h:1321>)

```cpp
struct KOUKU_RAID_FLOW_ENTRY final
	{
		std::string strEntryId,
			strTargetId;
		bool bBundle = false;
		std::uint32_t iWaitAfterMs = 0u;
	};
```

### KOUKU_RAID_FLOW_GROUP

[Server/Public/GameplayCatalog.h](<C:/Users/tnest/Desktop/LostArk/Server/Public/GameplayCatalog.h:1338>)

```cpp
struct KOUKU_RAID_FLOW_GROUP final
	{
		std::string strGroupId;
		std::uint32_t iFirstEntry = 0u,
			iLastEntry = 0u;
		std::optional<std::uint32_t> RepeatUntilHealthBars;
		bool bTransitionAtGroupEnd = false;
	};
```

### KOUKU_RAID_GATE_DEFINITION

[Server/Public/GameplayCatalog.h](<C:/Users/tnest/Desktop/LostArk/Server/Public/GameplayCatalog.h:1347>)

```cpp
struct KOUKU_RAID_GATE_DEFINITION final
	{
		std::string strEncounterId,
			strGateId,
			strFlowId,
			strSequenceCompositionId;
		std::string strIntroPatternId,
			strClearPatternId,
			strPrimaryBossPlacementId;
		std::string strEntrySequenceInstanceId;
		std::string strLoopStartEntryId;
		std::string strBingoSpecialPatternId;
		std::uint32_t iSequenceRevision = 0u,
			iIntroDurationMs = 0u,
			iClearDurationMs = 0u,
			iExpectedEntryCount = 0u;
		std::vector<KOUKU_RAID_FLOW_ENTRY> Entries;
		std::vector<KOUKU_RAID_FLOW_GROUP> EntryGroups;
		std::vector<KOUKU_RAID_ARRIVAL> Arrivals;
		// 아직 시작하지 않은 그룹의 첫 항목에서 체력 조건이 이미 충족되면 그 범위를 건너뛴다.
		// HP 비율은 정수 곱으로 비교하고, 반환값 Entries.size()는 다음 항목이 없다는 표식이다.
		std::uint32_t Resolve_ReadyEntry(std::uint32_t current, std::uint32_t hp,
			std::uint32_t maximumHp, std::uint32_t maximumBars) const noexcept
		{
			// 큰 피해가 여러 체력 경계를 넘겨도 아직 시작하지 않은 반복 구간만 건너뛴다.
			// 현재 index가 그룹의 첫 항목일 때만 넘기므로 사이의 독립 기믹 항목을 통째로 생략하지 않는다.
			for (const auto& group : EntryGroups)
				if (group.RepeatUntilHealthBars &&
					current == group.iFirstEntry && maximumHp && maximumBars &&
					static_cast<std::uint64_t>(hp) * maximumBars <= static_cast<std::uint64_t>(*group.RepeatUntilHealthBars) * maximumHp)
					current = group.iLastEntry + 1u;
			return current < Entries.size() ?
				current : static_cast<std::uint32_t>(Entries.size());
		}
		// GameRoom이 현재 occurrence와 동적 후속 실행을 모두 끝낸 뒤 다음 순번을 계산할 때 호출한다.
		std::uint32_t Resolve_NextCompletedEntry(std::uint32_t current, std::uint32_t hp,
			std::uint32_t maximumHp, std::uint32_t maximumBars) const noexcept
		{
			if (current >= Entries.size())
				return static_cast<std::uint32_t>(Entries.size());
			for (const auto& group : EntryGroups)
			{
				if (!group.RepeatUntilHealthBars ||
					current < group.iFirstEntry || current > group.iLastEntry)
					continue;
				const bool reached = maximumHp && maximumBars &&
					static_cast<std::uint64_t>(hp) * maximumBars <= static_cast<std::uint64_t>(*group.RepeatUntilHealthBars) * maximumHp;
				if (reached && (!group.bTransitionAtGroupEnd || current == group.iLastEntry))
					return Resolve_ReadyEntry(group.iLastEntry + 1u, hp, maximumHp, maximumBars);
				if (current == group.iLastEntry)
					return group.iFirstEntry;
				break;
			}
			return Resolve_ReadyEntry(current + 1u, hp, maximumHp, maximumBars);
		}
	};
```

BundleMember는 memberId/patternId/target placement/start offset 네 값으로 실행 대상을 지정한다. BundleDefinition은 bundle/encounter/gate ID와 Members를 소유한다. ParentChild는 occurrenceId/patternId/startMs/durationMs다. RaidEntry는 entry identity/target identity/bBundle/waitAfterMs다. RaidGroup은 group identity/첫·마지막 entry index/선택적인 HP 기준/그룹 끝까지 진행 정책이다. RaidGate의 ID들은 encounter/gate/flow/sequence composition/intro/clear/primary boss/entry sequence/loop entry/bingo special을 연결하고, revision·intro/clear 길이·expected count 및 Entries/EntryGroups/Arrivals가 전체 일정이다. Resolve_ReadyEntry와 Resolve_NextEntry는 이 정의를 읽어 index를 계산하며 live raid 상태를 정의에 저장하지 않는다.

Member의 PatternIds/TransitionTicks/iPatternIndex/iNextStartTick은 순차 스케줄, iScheduledStartTick은 bundle 최초 예약, iPatternSequence는 현재 발생이다. bCompleted/bOwnsPlayerMode는 완료와 HUD 소유 여부. MarioEntryAnchor 및 MarioEntry... 값은 root의 포털 자세·시계와 실제 입장한 player/session/entity를 고정한다. CompletionChain/Parent 필드는 자식 완료와 loop/복귀 대기를 보관한다. iNextWorldCue와 두 map은 아래 카드 motion의 정확한 재생 대상을 찾는다.

RunState의 ePhase/iOwnerSessionId/Request/iRoomAuditionEpoch/iCommonStartTick은 승인된 실행 범위, PinnedGameplayRevision/iPinnedSourceRevision/pProductGeneration은 정확한 데이터 세대, Members/WorldPlays/SupportSchedule/Tails는 실행 중 소유 목록이다. pProductGeneration의 shared_ptr가 정의 수명을 지킨다.

### KOUKUSAYDON_PATTERN_AUDITION_MEMBER

[Server/Public/GameRoom.h](<C:/Users/tnest/Desktop/LostArk/Server/Public/GameRoom.h:964>)

```cpp
struct KOUKUSAYDON_PATTERN_AUDITION_MEMBER final
		{
			std::string strMemberId;
			LostArk::Shared::NET_ENTITY_ID iBossEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
			KOUKUSAYDON_PATTERN_AUDITION_PHASE ePhase = KOUKUSAYDON_PATTERN_AUDITION_PHASE::PENDING;
			std::vector<std::string> PatternIds;
			std::vector<std::uint32_t> TransitionTicks;
			std::size_t iPatternIndex = 0u;
			std::uint32_t iNextStartTick = 0u;
			std::uint32_t iScheduledStartTick = 0u;
			std::uint32_t iPatternSequence = 0u;
			bool bCompleted = false;
			bool bOwnsPlayerMode = false;
			KOUKUSAYDON_LOGIC_LEDGER LogicLedger;
			// 진입 root가 후속 패턴들이 끝날 때까지 포털 기준 객체를 보관한다.
			std::optional<SERVER_WORLD_ENTITY> MarioEntryAnchor;
			std::string strMarioEntryPatternId;
			std::uint32_t iMarioEntryStartTick = 0u;
			std::uint8_t iMarioEntryStage = 0u;
			bool bMarioEntryConsumed = false;
			// 나중에 방에 남은 사람이 아니라 실제 입장에 성공한 플레이어·session·entity를 고정한다.
			bool bMarioSoloReturnRequired = false;
			LostArk::Shared::PLAYER_ID iMarioEntrantPlayerId = 0u;
			SESSION_ID iMarioEntrantSessionId = INVALID_SESSION_ID;
			LostArk::Shared::NET_ENTITY_ID iMarioEntrantNetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
			bool bMarioReturnCompleted = false;
			std::size_t iCompletionChainFirstIndex = 0u;
			std::uint32_t iCompletionChainCount = 0u;
			std::uint32_t iCompletionChainCompleted = 0u;
			std::string strCompletionChainSuccessPatternId;
			bool bCompletionChainStarted = false;
            bool bParentSequenceStarted = false;
            bool bParentSequenceLoops = false;
            std::size_t iParentLoopIndex = 0u;
            std::size_t iParentLastIndex = 0u;
			bool bCompletionChainAwaitingReturn = false;
			bool bCompletionChainSuccessQueued = false;
			std::uint32_t iNextWorldCue = 1u;
			std::unordered_map<std::string, std::string> WorldCueByInstance;
			std::unordered_map<std::string, std::string> WorldCueByOccurrence;
		};
```

### KOUKUSAYDON_PATTERN_AUDITION_STATE

[Server/Public/GameRoom.h](<C:/Users/tnest/Desktop/LostArk/Server/Public/GameRoom.h:1020>)

```cpp
struct KOUKUSAYDON_PATTERN_AUDITION_STATE final
		{
			KOUKUSAYDON_PATTERN_AUDITION_PHASE ePhase = KOUKUSAYDON_PATTERN_AUDITION_PHASE::INACTIVE;
			SESSION_ID iOwnerSessionId = INVALID_SESSION_ID;
			LostArk::Shared::C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST Request;
			std::uint32_t iRoomAuditionEpoch = 0u;
			std::uint32_t iCommonStartTick = 0u;
			LostArk::Shared::GameplayDataRevision PinnedGameplayRevision{};
			std::uint32_t iPinnedSourceRevision = 0u;
			// 전체 gameplay는 PinnedGameplayRevision을 따른다. 이 shared_ptr는 이번 실행의 Kouku 패턴·Logic 정의를 유지한다.
			std::shared_ptr<const CGameplayCatalog> pProductGeneration;
			std::vector<KOUKUSAYDON_PATTERN_AUDITION_MEMBER> Members;
			std::vector<LostArk::Shared::S2C_WORLD_SEQUENCE_PLAY> WorldPlays;
			std::vector<KOUKU_SCHEDULED_SUPPORT_SURFACE> SupportSchedule;
			std::vector<KOUKU_PATTERN_TAIL> Tails;
		};
```

## 6. P13 한 카드 접촉을 끝까지 추적

예제는 세 번째 망치 구간 20205~21250ms와 조커 `KOUKUSAYDON_G1_PATTERN_13.world.11`이다. 아래는 정적인 정의/코드 경로 설명이다. 이번 조사에서 실제 게임을 재생해 망치와 조커가 맞는 tick을 관측했다는 뜻은 아니다. 접촉이 성립하는 순간 이후 경로를 추적한다.

| 역할 | 저작 Logic 정의 ID | 해당 시간축 occurrence | kind/priority |
|---|---|---|---|
|카드 뒤집기 Trigger|koukusaydon.g1.logic.22|KOUKUSAYDON_G1_PATTERN_13.logic.23|OBJECT_CONTACT / 200|
|카드 들썩임 Trigger|koukusaydon.g1.logic.23|KOUKUSAYDON_G1_PATTERN_13.logic.22|OBJECT_CONTACT / 100|
|뒤집기 Result|koukusaydon.g1.logic.24|위 뒤집기 occurrence의 SUCCESS[0]|PLAY_CONTACT_WORLD_OBJECT_MOTION|
|들썩임 Result|koukusaydon.g1.logic.25|위 들썩임 occurrence의 SUCCESS[0]|PLAY_CONTACT_WORLD_OBJECT_MOTION|
|조커 찾기 완료 Result|koukusaydon.g1.logic.26|뒤집기 occurrence의 SUCCESS[1]|COMPLETE_LOGIC_WINDOW; target world.11만|

따라서 ‘logic22~25’라는 말만으로는 어느 층의 번호인지 결정할 수 없다. P13에는 `.logic.24/.logic.25`라는 배치 창이 없으며 두 번호는 RESULT 정의를 가리킨다. 공통 group은 `joker.hammer.contact`, window duration은 1045ms. 서버 상대 시작/끝은 **607 / 638 tick**이다. End와 정확히 같은 tick은 OBJECT_CONTACT 선처리에서 한 번 판단한 후 닫는다.

조커 center는 X=2.1440000534, Z=317.7470092773, radius=1.25m다. 다른 여섯 카드는 같은 `world.object.instance.kouku.card` template을 공유하므로 instance 문자열만으로는 실제 어느 카드인지 구분할 수 없다. `.world.5`~`.world.11` occurrence가 그 차이를 보존한다. 현재 제품은 이 저장 위치를 사용하며 랜덤 후보가 적용된 상태로 설명하지 않는다.

### 실제 설치된 행

[Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap](<C:/Users/tnest/Desktop/LostArk/Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap:1124>)

```text
1124: PATTERNLOGIC	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_13	2	KOUKUSAYDON_G1_PATTERN_13.logic.6	EXTERNAL_SIGNAL	4573	24717	0	0	0	0	0	0	0	0	0	0	1	-	0	0
1127: PATTERNLOGIC	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_13	5	KOUKUSAYDON_G1_PATTERN_13.logic.22	OBJECT_CONTACT	20205	1045	0	0	0	0	0	0	0	0	0	0	0	-	0	0
1128: PATTERNLOGIC	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_13	6	KOUKUSAYDON_G1_PATTERN_13.logic.23	OBJECT_CONTACT	20205	1045	0	0	0	0	0	0	0	0	0	0	0	-	0	0
1803: PATTERNLOGICCONTACTGROUP	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_13	KOUKUSAYDON_G1_PATTERN_13.logic.22	joker.hammer.contact	100
1804: PATTERNLOGICCONTACTGROUP	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_13	KOUKUSAYDON_G1_PATTERN_13.logic.23	joker.hammer.contact	200
1839: PATTERNLOGICCONTACTTARGET	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_13	KOUKUSAYDON_G1_PATTERN_13.logic.22	KOUKUSAYDON_G1_PATTERN_13.world.11	world.object.instance.kouku.joker_card	2.1440000534057617	317.74700927734375	1.25
1846: PATTERNLOGICCONTACTTARGET	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_13	KOUKUSAYDON_G1_PATTERN_13.logic.23	KOUKUSAYDON_G1_PATTERN_13.world.11	world.object.instance.kouku.joker_card	2.1440000534057617	317.74700927734375	1.25
1921: PATTERNLOGICOUTCOME	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_13	KOUKUSAYDON_G1_PATTERN_13.logic.6	SUCCESS	0	FOLLOWUP_PATTERN	0	0	KOUKUSAYDON_G1_PATTERN_14
1922: PATTERNLOGICOUTCOME	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_13	KOUKUSAYDON_G1_PATTERN_13.logic.6	TIMEOUT	0	INSTANT_DEATH	0	0	-
1941: PATTERNLOGICOUTCOME	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_13	KOUKUSAYDON_G1_PATTERN_13.logic.22	SUCCESS	0	PLAY_CONTACT_WORLD_OBJECT_MOTION	0	0	-
1948: PATTERNLOGICCONTACTMOTION	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_13	KOUKUSAYDON_G1_PATTERN_13.logic.22	SUCCESS	0	KOUKUSAYDON_G1_PATTERN_13.world.11	world.object.instance.kouku.joker_card_hop
1949: PATTERNLOGICOUTCOME	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_13	KOUKUSAYDON_G1_PATTERN_13.logic.23	SUCCESS	0	PLAY_CONTACT_WORLD_OBJECT_MOTION	0	0	-
1950: PATTERNLOGICOUTCOME	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_13	KOUKUSAYDON_G1_PATTERN_13.logic.23	SUCCESS	1	COMPLETE_LOGIC_WINDOW	0	0	-
1957: PATTERNLOGICCONTACTMOTION	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_13	KOUKUSAYDON_G1_PATTERN_13.logic.23	SUCCESS	0	KOUKUSAYDON_G1_PATTERN_13.world.11	world.object.instance.kouku.joker_card_flip
1958: PATTERNLOGICSIGNAL	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_13	KOUKUSAYDON_G1_PATTERN_13.logic.23	SUCCESS	1	KOUKUSAYDON_G1_PATTERN_13.logic.6	KOUKUSAYDON_G1_PATTERN_13.world.11
2889: PATTERNLOGICREGION	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_13	KOUKUSAYDON_G1_PATTERN_13.logic.22	0	KOUKUSAYDON_G1_PATTERN_13.presentation.17	BOSS_CURRENT	BOX	-2.2999999523162842	0	2.5499999523162842	0	1	1	0.40000000596046448	3	45	NONE	NONE
2890: PATTERNLOGICREGION	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_13	KOUKUSAYDON_G1_PATTERN_13.logic.22	1	KOUKUSAYDON_G1_PATTERN_13.presentation.18	BOSS_CURRENT	BOX	-2.5	0	-2.5999999046325684	1.2999999523162842	1	1	0.64999997615814209	3	45	NONE	NONE
2891: PATTERNLOGICREGION	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_13	KOUKUSAYDON_G1_PATTERN_13.logic.23	0	KOUKUSAYDON_G1_PATTERN_13.presentation.16	BOSS_CURRENT	BOX	0	0	0	0	1	1	0.550000011920929	3	45	NONE	NONE
3785: PATTERNLOGICREGIONWORLD	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_13	KOUKUSAYDON_G1_PATTERN_13.logic.22	KOUKUSAYDON_G1_PATTERN_13.presentation.17	20205	0	1045	1	0	0	0	0	0	1	1	1
3786: PATTERNLOGICREGIONWORLD	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_13	KOUKUSAYDON_G1_PATTERN_13.logic.22	KOUKUSAYDON_G1_PATTERN_13.presentation.18	20205	0	1045	1	0	0	0	0	0	1	1	1
3787: PATTERNLOGICREGIONWORLD	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_13	KOUKUSAYDON_G1_PATTERN_13.logic.23	KOUKUSAYDON_G1_PATTERN_13.presentation.16	20205	0	1045	1	0	0	0	0	0	1	1	1
```

행을 객체로 따라가면 `.logic.23`은 `eKind=OBJECT_CONTACT`, `ContactTargets[world.11]`, `CardRegions[presentation.16]`, `iContactPriority=200`, `OnSuccess` 결과 두 개를 가진다. `.logic.22`는 priority100과 presentation.17/.18 두 영역, hop 결과 하나다. region이 BOSS_CURRENT이고 구운 weapon track이 붙으므로 world 카드 원과 비교하기 전에 현재 boss와 시간 표본을 적용한다.

실행 흐름은 다음과 같다.

1. Build가 ContactWindowOrder를 priority 내림차순, 동률 window ID 순으로 고정한다. 200이 100보다 먼저다.
2. Update의 contact 선처리가 열려 있는 window를 방문한다. 이미 이 window에서 맞은 occurrence는 건너뛴다. 과거에 더 높은 priority motion이 적용된 카드도 낮은 반응에서 제외한다.
3. `Intersects_LogicRegion`이 region의 현재 변환과 target의 XZ 원을 비교한다. 카드 예제는 player HP 판정이 아니다.
4. 실제 겹친 target을 ContactedWorldOccurrences에 넣고 `(contactGroupId, state.iStartTick, targetWorldOccurrenceId)`를 ConsumedContactGroups에 넣는다. 이미 소비했다면 이 strike의 낮은 반응은 실행하지 않는다. 이는 **카드별·strike별** 소비이며 전체 패턴에서 카드 한 장만 맞게 막는 제한이 아니다.
5. SUCCESS[0]은 실제 target occurrence를 ContactMotions에서 찾아 `joker_card_flip` 요청을 만든다. 출력에는 motion instance, target template instance, 정확한 target World occurrence, 시작 tick이 들어간다. priority200을 ledger에 보관한다.
6. SUCCESS[1]은 contact filter가 `.world.11`일 때만 `.logic.6`을 completedWindows에 넣는다. 일반 카드를 뒤집으면 이 신호는 나오지 않는다.
7. 같은 tick에 EXTERNAL_SIGNAL `.logic.6`의 성공 결과를 처리한다. OnTimeout보다 앞선 단계다. `.logic.6`의 endsPatternOnSuccess 때문에 output.bEndPatternEarly도 켜진다. 아래 실제 bootstrap의 SUCCESS는 후속 `KOUKUSAYDON_G1_PATTERN_14`, TIMEOUT은 즉사다.
8. Room이 WorldCueByOccurrence에서 `.world.11`의 원래 cue ID를 찾는다. flip 요청 packet의 strTargetCueId에 넣어 Client가 그 카드 한 개의 기존 재생을 찾게 한다. 같은 template의 다른 카드가 뒤집히지 않는 이유다.
9. Room은 followup을 member scheduler에 삽입하고 Brain Complete를 호출한다. Client는 수신한 motion cue로 실제 flip/hop을 재생하며 Server는 이미 확정한 카드 접촉 결과를 Client 화면에 다시 물어보지 않는다.

priority200의 뜻은 높은 피해량이 아니라 겹친 반응의 우선권이다. `AppliedWorldMotions`는 같은 occurrence+motion 중복, `ContactedWorldOccurrences`는 같은 창 재접촉 중복, `ConsumedContactGroups`는 동일 strike 반응 충돌, `AppliedContactMotionPriorities`는 뒤집힌 카드를 나중의 낮은 hop이 덮는 것을 각각 막는다. 네 기록은 서로 같은 역할이 아니다.

[Server/Private/KoukuSaydonLogicRuntime.cpp](<C:/Users/tnest/Desktop/LostArk/Server/Private/KoukuSaydonLogicRuntime.cpp:198>) — 198~202행. 아래는 현재 제품 소스 원문이다.

```cpp
	std::sort(ledger.ContactWindowOrder.begin(), ledger.ContactWindowOrder.end(), [&](const auto a, const auto b) {
		const auto& first = pattern.LogicWindows[a]; const auto& second = pattern.LogicWindows[b];
		return first.iContactPriority != second.iContactPriority ? first.iContactPriority > second.iContactPriority : first.strWindowId < second.strWindowId;
	});
	for (std::size_t index = 0u; index < pattern.MechanicTriggers.size(); ++index)
```

[Server/Private/KoukuSaydonLogicRuntime.cpp](<C:/Users/tnest/Desktop/LostArk/Server/Private/KoukuSaydonLogicRuntime.cpp:1983>) — 1983~2040행. 아래는 현재 제품 소스 원문이다.

```cpp
	std::set<std::string> completedWindows;
	for (const auto index : ledger.ContactWindowOrder)
	{
		auto& state = ledger.Windows[index];
		const auto& window = pattern.LogicWindows[index];
		if (state.bClosed || !Has_ReachedTick(serverTick, state.iStartTick)) continue;
		if (!state.bOpened) Open_Window(boss, window, state, ledger, players);
		const bool reachedEnd = Has_ReachedTick(serverTick, state.iEndTick);
		if (!reachedEnd || serverTick == state.iEndTick)
			for (const auto& target : window.ContactTargets)
			{
				if (state.ContactedWorldOccurrences.contains(target.strWorldOccurrenceId)) continue;
				const auto motionPriority = ledger.AppliedContactMotionPriorities.find({window.strContactGroupId, target.strWorldOccurrenceId});
				if (!window.strContactGroupId.empty() && motionPriority != ledger.AppliedContactMotionPriorities.end() &&
					motionPriority->second > window.iContactPriority) continue;
				const LostArk::Shared::CombatCollision::BODY_CIRCLE_XZ circle{ target.fWorldX, target.fWorldZ, target.fRadiusM };
				if (!std::any_of(window.CardRegions.begin(), window.CardRegions.end(), [&](const auto& region) {
					return Intersects_LogicRegion(Window_Region(region, state), boss, circle, serverTick - ledger.iPatternStartTick);
				})) continue;
				state.ContactedWorldOccurrences.insert(target.strWorldOccurrenceId);
				if (!window.strContactGroupId.empty() && !ledger.ConsumedContactGroups.emplace(
					window.strContactGroupId, state.iStartTick, target.strWorldOccurrenceId).second) continue;
				for (const auto& result : window.OnSuccess)
				{
					if (result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::PLAY_CONTACT_WORLD_OBJECT_MOTION)
					{
						const auto motion = std::find_if(result.ContactMotions.begin(), result.ContactMotions.end(),
							[&](const auto& row) { return row.strTargetWorldOccurrenceId == target.strWorldOccurrenceId; });
						if (motion != result.ContactMotions.end() && state.AppliedWorldMotions.emplace(
							target.strWorldOccurrenceId, motion->strMotionInstanceId).second)
						{
							KOUKUSAYDON_LOGIC_WORLD_PLAY play;
							play.strInstanceId = motion->strMotionInstanceId; play.strTargetSequenceInstanceId = target.strWorldInstanceId;
							play.strTargetWorldOccurrenceId = target.strWorldOccurrenceId; play.iStartTick = serverTick;
							outOutput.WorldSequencePlays.push_back(std::move(play));
							if (!window.strContactGroupId.empty())
								ledger.AppliedContactMotionPriorities[{window.strContactGroupId, target.strWorldOccurrenceId}] = window.iContactPriority;
						}
					}
					else if (result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::COMPLETE_LOGIC_WINDOW &&
						(result.strContactTargetWorldOccurrenceId.empty() || result.strContactTargetWorldOccurrenceId == target.strWorldOccurrenceId))
						completedWindows.insert(result.strTargetLogicOccurrenceId);
				}
			}
		if (reachedEnd) Close_Window(boss, window, state, players);
	}
	for (auto& state : ledger.Windows)
	{
		const auto& window = pattern.LogicWindows[state.iWindowIndex];
		if (state.bClosed || window.eKind != BOSS_PATTERN_LOGIC_KIND::EXTERNAL_SIGNAL ||
			!completedWindows.contains(window.strWindowId) || !Has_ReachedTick(serverTick, state.iStartTick) ||
			(Has_ReachedTick(serverTick, state.iEndTick) && serverTick != state.iEndTick)) continue;
		if (!state.bOpened) Open_Window(boss, window, state, ledger, players);
		Close_Window(boss, window, state, players);
		Apply_Results(window.OnSuccess, state, nullptr, players, boss, catalog, pMadnessPolicy,
			serverTick, outDamageEvents, outOutput, invulnerablePlayers);
		outOutput.bEndPatternEarly |= window.bEndsPatternOnSuccess;
		outOutput.strStatus = "external signal window succeeded";
```

[Server/Private/GameRoom_KoukuPlayerCommands.cpp](<C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom_KoukuPlayerCommands.cpp:910>) — 910~928행. 아래는 현재 제품 소스 원문이다.

```cpp
		}
		if (play.CombatBody && !Stage_KoukuWorldBody(*play.CombatBody, message, play.bAuthoredMadness)) continue;
		if (!play.strTargetSequenceInstanceId.empty())
		{
			const auto& cues = play.strTargetWorldOccurrenceId.empty() ? member->WorldCueByInstance : member->WorldCueByOccurrence;
			const auto target = cues.find(play.strTargetWorldOccurrenceId.empty() ? play.strTargetSequenceInstanceId : play.strTargetWorldOccurrenceId);
			if (target == cues.end()) { m_strStatus = "World motion target has no owned occurrence"; continue; }
			if (std::any_of(m_KoukuDamageableWorldCues.begin(), m_KoukuDamageableWorldCues.end(), [&](const auto& cue) {
				return cue.Play.iRunEpoch == message.iRunEpoch && cue.Play.strMemberId == message.strMemberId && cue.Play.strCueId == target->second; }))
			{ m_strStatus = "Damageable World Object cannot be retargeted to an unbaked motion"; continue; }
			message.strTargetCueId = target->second;
		}
		else
		{
			member->WorldCueByInstance[play.strInstanceId] = message.strCueId;
			if (!play.strOccurrenceId.empty()) member->WorldCueByOccurrence[play.strOccurrenceId] = message.strCueId;
		}
		m_KoukuSaydonPatternAudition.WorldPlays.push_back(message);
		Broadcast_OwnedWorldSequence(message);
```

## 7. Server에서 Client로 전달되는 것

boss의 현재 action은 일반 world snapshot으로 간다. GameRoom_Replication은 `strPatternId/strActionId/iActionStartTick/iPatternStartTick/iPatternStageIndex/iPatternSequence/XYZ/yaw` 등 현재 상태를 구성한다. boss 내부의 `strPatternStageId` 문자열을 그대로 보내는 대신 이 snapshot은 stage index와 action ID를 보낸다. Client의 action binding은 그 action ID에 맞는 animation clip을 선택한다. Server는 모든 렌더 프레임의 bone matrix를 packet으로 보내지 않는다. HP/상태/넉백은 서버 player state와 damage event로 내려간다.

별도 World animation은 `S2C_WORLD_SEQUENCE_PLAY`다. `strSequenceInstanceId`가 새 World/모션 정의, `strTargetSequenceInstanceId`가 기존 대상 template, `strTargetCueId`가 실제 발생이다. `iRunEpoch/strMemberId/strCueId/iBossNetEntityId/iPatternSequence`가 실행 소유 범위를 고정한다. `strOccurrenceId`는 새 cue의 저작 발생을 연결한다. `iStartTick/iServerTick`은 지연 수신 시 재생 경과를 맞추는 기준이다. `fPlaybackSpeed/fPositionOffsetXYZ/iDurationMs`는 속도·위치·수명, `bHasPlacement` 이하 TRS는 명시 배치다. `bUntilDestroyed/iCombatBodyNetEntityId`는 서버 파괴 가능한 World object의 생존 계약이다.

Room의 Broadcast_OwnedWorldSequence는 Write_Message로 직렬화한 뒤 각 room player session으로 보낸다. 직렬화 실패는 status에 남고 송신 실패 session은 닫기 요청을 한다. packet을 보냈다는 사실과 Client가 해당 리소스를 성공적으로 생성/표시했다는 사실은 별도로 확인해야 한다. 본 자료는 서버 정의·분기·패킷 연결을 확인한 자료이며 시각 결과 판정은 포함하지 않는다.

- [Server/Private/GameRoom_Replication.cpp](<C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom_Replication.cpp:494>)
- [Server/Private/GameRoom_Replication.cpp](<C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom_Replication.cpp:746>)
- [Server/Private/GameRoom_KoukuAudition.cpp](<C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom_KoukuAudition.cpp:742>)
- [Shared/Private/Network/PacketMessages.cpp](<C:/Users/tnest/Desktop/LostArk/Shared/Private/Network/PacketMessages.cpp:7211>)

### S2C_WORLD_SEQUENCE_PLAY

[Shared/Public/Network/PacketMessages.h](<C:/Users/tnest/Desktop/LostArk/Shared/Public/Network/PacketMessages.h:3643>)

```cpp
struct S2C_WORLD_SEQUENCE_PLAY
	{
		WORLD_SEQUENCE_OPERATION eOperation = WORLD_SEQUENCE_OPERATION::PLAY;
		std::string strSequenceInstanceId;
		// 저작된 instance 재생 속도에 곱하는 배율이다. 1이면 원래 속도이며 패턴에서 회전·낙하 연출의 속도를 조절할 수 있다.
		float fPlaybackSpeed = 1.f;
		float fPositionOffsetX = 0.f;
		float fPositionOffsetY = 0.f;
		float fPositionOffsetZ = 0.f;
		std::uint32_t iDurationMs = 0u; // 0이면 저작 sequence의 수명을 사용한다.
		// bUntilDestroyed가 true인 Server 소유 combat object는 고정 종료 시간 대신 파괴·명시 취소로 종료한다.
		bool bUntilDestroyed = false;
		// 해당 cue가 소유하며 Server가 승인한 살아 있는 combat body일 때만 유효한 entity ID를 담는다.
		NET_ENTITY_ID iCombatBodyNetEntityId = INVALID_NET_ENTITY_ID;
		// 비어 있으면 새 sequence를 시작한다. 값이 있으면 그 기존 instance에 motion을 적용한다.
		std::string strTargetSequenceInstanceId;
		std::uint32_t iRunEpoch = 0u; // 0은 Debug 재생에 소속되지 않은 일반 map sequence다.
		std::string strMemberId;
		std::string strCueId;
		std::string strOccurrenceId;
		std::string strTargetCueId;
		std::uint32_t iStartTick = 0u;
		std::uint32_t iServerTick = 0u;
		NET_ENTITY_ID iBossNetEntityId = INVALID_NET_ENTITY_ID;
		std::uint32_t iPatternSequence = 0u;
		// 소유 WORLD box는 저장 instance의 배치를 덮어쓸 수 있다.
		// motion은 대상 cue의 배치를 유지하므로 별도의 placement 변경을 요청하지 않는다.
		bool bHasPlacement = false;
		float fWorldPositionX = 0.f, fWorldPositionY = 0.f, fWorldPositionZ = 0.f;
		float fWorldRotationXDegrees = 0.f, fWorldRotationYDegrees = 0.f, fWorldRotationZDegrees = 0.f;
		float fWorldScaleX = 1.f, fWorldScaleY = 1.f, fWorldScaleZ = 1.f;
	};
```

### 피해/넉백이 내려가는 별도 경로

카드 motion과 일반 player 피해는 결과 경로가 다르다. `Apply_Result`의 피해 kind는 `SERVER_WORLD_TO_PLAYER_HIT`에 raw damage, 공격 원점, 밀기 거리/시간/방향, 현재 tick을 채워 `CServerCombatHitRuntime::Apply_WorldToPlayer`로 전달한다. 고정 피해는 iDamageAmount, 최대 HP 비율은 maximum HP, 즉사는 현재 HP를 기준으로 한다. 공통 runtime은 현재 사망/전투 준비/낙하/잡힘 상태와 owned capture, 시간정지/보호/무적 등을 확인한 다음 보호막·HP·damage event를 갱신하고 살아 있으면 기존 피격 이동 경로에 밀기를 전달한다.

따라서 Collider가 화면에 보이거나 geometry가 겹쳤다는 사실만으로 HP/넉백까지 적용되었다고 확정할 수 없다. 창이 열렸는지 → 실제 권위 좌표가 겹쳤는지 → 결과 목록이 선택됐는지 → hit admission과 보호 상태가 통과했는지 → HP/event/이동 값이 생성됐는지를 단계별로 확인해야 한다. 이는 특정 P26 결함을 확정한 설명이 아니라 현재 코드의 판정 경계다.

[Server/Private/KoukuSaydonLogicRuntime.cpp](<C:/Users/tnest/Desktop/LostArk/Server/Private/KoukuSaydonLogicRuntime.cpp:1458>) / [Server/Private/ServerCombatHitRuntime.cpp](<C:/Users/tnest/Desktop/LostArk/Server/Private/ServerCombatHitRuntime.cpp:664>)

### World / mechanic 정의 보충

WorldPlacement는 월드 위치 m/회전 degree/scale 배율이다. WorldCombatBody는 최대 HP·월드 중심·반지름을 담은 정의이며 현재 entity ID나 HP를 보관하지 않는다. WorldSequence는 instance/occurrence/start/duration/speed/offset, spawn 기준 보정, support window, 광기 출처 및 선택적인 combat body/placement를 가진다. SceneProfile은 profile identity와 시작/유지/blend ms다. 표시 및 렌더링 자원의 실제 내용은 Client 책임이다.

MechanicTrigger는 아래 enum별로 사용하는 필드가 다른 공유 자료형이다. 핵심 identity/kind/start/duration 외에 HUD/teleport/clone/clock, 플레이어 target·projectile·빙고·airborne·summon별 매개변수가 모여 있다. parser가 kind와 허용되는 row를 검증하므로 모든 필드가 매 trigger에서 동시에 쓰이는 값은 아니다. 우선 현재 패턴의 eKind를 확인한 뒤 해당 소비 함수를 읽는다.

### BOSS_PATTERN_WORLD_PLACEMENT

[Server/Public/GameplayCatalog.h](<C:/Users/tnest/Desktop/LostArk/Server/Public/GameplayCatalog.h:1024>)

```cpp
struct BOSS_PATTERN_WORLD_PLACEMENT final
	{
		float fPositionX = 0.f,
			fPositionY = 0.f,
			fPositionZ = 0.f;
		float fRotationXDegrees = 0.f,
			fRotationYDegrees = 0.f,
			fRotationZDegrees = 0.f;
		float fScaleX = 1.f,
			fScaleY = 1.f,
			fScaleZ = 1.f;
	};
```

### BOSS_PATTERN_WORLD_COMBAT_BODY

[Server/Public/GameplayCatalog.h](<C:/Users/tnest/Desktop/LostArk/Server/Public/GameplayCatalog.h:1038>)

```cpp
struct BOSS_PATTERN_WORLD_COMBAT_BODY final
	{
		std::uint32_t iMaximumHp = 0u;
		float fCenterX = 0.f,
			fCenterY = 0.f,
			fCenterZ = 0.f;
		float fRadiusM = 0.f;
	};
```

### BOSS_PATTERN_WORLD_SEQUENCE

[Server/Public/GameplayCatalog.h](<C:/Users/tnest/Desktop/LostArk/Server/Public/GameplayCatalog.h:1049>)

```cpp
struct BOSS_PATTERN_WORLD_SEQUENCE final
	{
		std::uint32_t iDurationMs = 0u;
		std::string strInstanceId;
		std::string strOccurrenceId;
		std::uint32_t iStartMs = 0u;
		float fPlaybackSpeed = 1.f;
		float fPositionOffsetX = 0.f;
		float fPositionOffsetY = 0.f;
		float fPositionOffsetZ = 0.f;
		bool bAnchorBossSpawn = false;
		float fAnchorPositionX = 0.f;
		float fAnchorPositionY = 0.f;
		float fAnchorPositionZ = 0.f;
		std::vector<BOSS_PATTERN_WORLD_SUPPORT_WINDOW> SupportWindows;
		bool bAuthoredMadness = false;
		std::optional<BOSS_PATTERN_WORLD_COMBAT_BODY> CombatBody;
		std::optional<BOSS_PATTERN_WORLD_PLACEMENT> Placement;
	};
```

### BOSS_PATTERN_SCENE_PROFILE

[Server/Public/GameplayCatalog.h](<C:/Users/tnest/Desktop/LostArk/Server/Public/GameplayCatalog.h:1070>)

```cpp
struct BOSS_PATTERN_SCENE_PROFILE final
	{
		std::string strProfileId;
		std::uint32_t iStartMs = 0u;
		std::uint32_t iDurationMs = 0u;
		std::uint32_t iBlendMs = 0u;
	};
```

### BOSS_PATTERN_MECHANIC_TRIGGER

[Server/Public/GameplayCatalog.h](<C:/Users/tnest/Desktop/LostArk/Server/Public/GameplayCatalog.h:937>)

```cpp
struct BOSS_PATTERN_MECHANIC_TRIGGER final
	{
		std::string strTriggerId;
		BOSS_PATTERN_MECHANIC_TRIGGER_KIND eKind = BOSS_PATTERN_MECHANIC_TRIGGER_KIND::HUD_ENTER;
		std::uint32_t iStartMs = 0u;
		std::uint32_t iDurationMs = 0u;
		LostArk::Shared::KOUKU_HUD_MODE eHudMode = LostArk::Shared::KOUKU_HUD_MODE::NONE;
		float fTeleportX = 0.f;
		float fTeleportY = 0.f;
		float fTeleportZ = 0.f;
		// 게시자가 MAP 이펙트 occurrence에서 해석한 순서별 월드 진입 목적지다.
		std::vector<std::array<float, 3u>> PlayerEntryPositions;
		std::string strClonePatternId;
		std::vector<std::uint32_t> ClockHours;
		float fFaceCenterYawOffsetDegrees = 0.f;
		std::uint32_t iCountPerPlayer = 0u;
		float fPlayerEffectRadiusM = 0.f;
		std::uint32_t iEffectLifetimeMs = 0u;
		std::uint32_t iArenaRandomCount = 0u;
		float fArenaRandomRadiusM = 0.f;
		float fArenaHeightToleranceM = 0.f;
		float fArenaMinimumSpacingM = 0.f;
		bool bRandomPlayerOnly = false;
		// 게시된 시각 그룹의 stable ID다. Server는 실제 이펙트 asset 경로를 해석하지 않는다.
		std::string strFixedVisualId;
		std::string strTrackingVisualId;
		std::uint32_t iFixedLifetimeMs = 0u;
		std::uint32_t iSpawnIntervalMs = 0u;
		float fFollowSpeedScale = 0.f;
		std::vector<BOSS_SHOWTIME_RANDOM_VOLLEY> RandomVolleys;
		std::vector<LostArk::Shared::ATTACK_HIT_TEMPLATE> FixedHits,
			TrackingHits,
			ProjectileHits;
		std::uint32_t iRandomSpawnIntervalMs = 0u;
		float fRandomArenaRadiusM = 0.f,
			fRandomArenaHeightToleranceM = 0.f;
		std::string strRandomAnchorKind = "BOSS_SPAWN";
		float fRandomScaleMin = 1.f,
			fRandomScaleMax = 1.f;
		ALBION_AIRBORNE_PHASE eAirbornePhase = ALBION_AIRBORNE_PHASE::NONE;
		float fAirborneHeightM = 0.f;
		std::uint32_t iAirborneDurationMs = 0u;
		bool bCaptureAirborneTargetPosition = false;
		std::string strSelectedEffectVisualId;
		std::uint32_t iSelectedEffectLifetimeMs = 0u;
		std::vector<BOSS_PATTERN_SUMMON_PATTERN_SPAWN> PatternSpawns;
		std::vector<std::string> DirectionPatternIds;
		std::string strCloneEndStageId;
		std::string strRealPatternId;
		std::vector<std::string> ProjectileVisualIds;
		std::vector<LostArk::Shared::MECHANIC_CARD_SYMBOL> ProjectileCardSymbols;
		std::string strContactVisualId;
		// 0이면 거리 상한을 두지 않고 수명에 따른 이동 종료를 사용한다.
		float fProjectileMaxDistanceM = 0.f;
		float fProjectileSpeedMps = 0.f,
			fProjectileContactRadiusM = 0.f,
			fProjectileSpawnRadiusM = 0.f;
		std::uint32_t iProjectileLifetimeMs = 0u,
			iProjectileCountPerWave = 0u;
		bool bProjectileHoming = false;
		std::array<std::uint32_t, 3u> SoldierCounts{ 1u, 1u, 1u };
		float fSoldierSpawnRadiusMinM = 3.f,
			fSoldierSpawnRadiusMaxM = 6.f;
		// 0이면 profile 기본값을 사용한다. 양수 고정 피해는 방어 계산을 우회하며 다른 보호 효과는 공용 피격 경로가 처리한다.
		std::uint32_t iSoldierMaxHp = 0u,
			iSoldierDamage = 0u;
		bool bHasSoldierTuning = false;
		std::optional<BOSS_SHOWTIME_BOMB_RESOLUTION> ShowtimeBomb;
		// 저작 Object scale을 반영한 월드 m 반길이다. 경로 전방/가로 두 축을 저장한다.
		std::optional<std::array<float, 2u>> BingoHammerHalfExtentsM;
		std::optional<BOSS_BINGO_BOARD_SETTINGS> BingoBoardSettings;
		// 게시자가 animation/다른 활성 lane이 없는 빙고 제어 전용 패턴인지 검증하여 전달한 값이다.
		bool bBingoControlOnly = false;
	};
```

## 8. 학습할 때 직접 확인할 질문

1. 지금 보고 있는 ID는 reusable definition ID인가, pattern occurrence ID인가, 실행 cue ID인가?
2. 바꾼 시간은 Stage의 duration인가, Logic의 start/duration인가, World cue의 start/duration인가?
3. 도형은 WORLD인가, 현재 boss/시작 boss 기준인가? 값의 단위와 track baseline이 무엇인가?
4. 판정은 player body와 비교하는가, World target circle과 비교하는가?
5. 결과를 LogicRuntime이 직접 적용하는가, Room의 commit이 필요한가?
6. 같은 pattern ID가 재시작됐을 때 sequence/epoch/ledger가 새로 만들어지는가?
7. 저작 JSON, projection JSON, 설치 bootstrap, 실행 중 pinned generation 중 어디까지 바뀌었는가?

이 질문을 P13 한 window에 답한 뒤 다른 패턴의 kind만 바꾸어 따라가면 전체 패턴을 같은 구조로 읽을 수 있다. 특정 한국어 이름마다 별도 C++ 함수가 생겨야 하는 구조가 아니다.

## 조사 범위와 남은 경계

이번 결과는 읽기·원문 추출·선택 JSON/TSV 구조 확인이다. 새로운 빌드, 서버 실행, packet 실제 수신, Client/UI 조작, P17/P26/랜덤화 후보 적용은 하지 않았다. P17/P26 이전 fixture 결론은 별도 manual-colliders 문서의 조건부 결과이며 본 일반 흐름 자료에서 사용자 현상의 원인 확정으로 확대하지 않는다.

### 추출 시점 파일 지문

| 파일 | SHA-256 |
|---|---|
|Server/Public/GameplayCatalog.h|`93dba3e9f343baf7fd6d303ed5a415070659f073a395b2f88c4aa77515ec10ae`|
|Server/Private/GameplayCatalog.cpp|`2916259bc07cef56ea3d03f392f33f900a7c4d355be72dc0301373e85bf29eb9`|
|Server/Private/KoukuSaydonLogicRuntime.cpp|`d13633589ddad5a2a7e7dbee98dffcf6cabf326ef32b9db13151c7f331f024c1`|
|Server/Public/KoukuSaydonLogicRuntime.h|`8d48b55e94bc32c120e90e41fd8d669d32ed37ee92cb176928ce71b56c680652`|
|Server/Private/KoukuSaydonBrain.cpp|`309f88289c3bd9640883005360d9def03e576d62af800d5921bbbd797d86ae4e`|
|Server/Private/GameRoom_KoukuAudition.cpp|`ab549be22aa5ca0c577ddfbe3d87a13fa85a865a048cfd190f6407770391a2bb`|
|Server/Private/GameRoom_KoukuPlayerCommands.cpp|`e97a597fec0e615b648ba97214862581c58fddd71acd1531812705a76d67b52b`|
|Server/Private/GameRoom_KoukuMechanics.cpp|`6b78ca10e25ac2769159ba65ed4008e170f95c4ed3a176d87858190ceda94970`|
|Server/Private/GameRoom_KoukuTargeting.cpp|`151a7e0f439648e19e81260a4882d70dc6ff0168b39adb65b1ad46ac1710ab5d`|
|Tools/KoukuSaydonPipeline/KoukuBootstrapRows.ps1|`206aa3201364fcb7eb4322ddff1446c0ad45774b6025cda1d0f98a36b7136a33`|
|Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py|`b32d990b34e31e91649dfa4429c371d0fe674ec9cbd9d5e01473279f8d82148c`|
|Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap|`07b8259a63ccb9e12c92dc0607ae4bd7fb75c74edc6585c9d8f63947ff106ef6`|
|Shared/Public/Network/PacketMessages.h|`cb3c37e62eafe16c17fd649075a48d1a0ef594ee583e0850ba769fb87c511b18`|
|Server/Public/GameRoom.h|`18734ee9220c9017ed6cf339241d610e44daaa4cf3cf4e5b727a1abccad60ee9`|
