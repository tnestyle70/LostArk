# 대형 세이튼 손 잡기와 피자 마지막 찍기 — 직접 구현용 근거

2026-10-14. **제품 C++·저작 JSON·게시 데이터에는 이 작업의 변경을 적용하지 않았다.** 아래 후보는 사용자가 검토해 직접 구현할 코드다. 실행한 것은 out 전용 수치 fixture이며 Client/UI/제품 Server listener를 시작하지 않았다. 사용자의 마무리 요청에 따라 추가 컴파일·실험은 중단했다.

## 지금 확인된 결론

P17은 Server가 잡히기 직전 플레이어와 보스 root 사이의 상대 위치를 계속 보존하고, Client는 실제 왼손 본 위치에 표시하는 차이가 있다. 기존 피해 BOX를 크게 바꾸는 것보다 **Server의 잡힌 플레이어 실제 위치를 동일한 손 pose로 갱신하는 수정**이 맞는다. 현재 설치 WModel로 다시 계산한 손 위치는 네 피해 BOX 안에 들어간다. 아래 손 추적 후보는 아직 제품에 적용하지 않았다.

P26은 이미 MAP → Server WORLD CYLINDER다. 현재 bundle 5를 실제 `CGameRoom::Tick`으로 재생해 **마지막 찍기의 피해와 넉백이 적용되는 것을 재현했다.** 따라서 WORLD 설정을 다시 바꾸거나 임의로 콜라이더를 확대하는 수정을 제안하지 않는다. 사용자가 관찰한 실전 실패의 원인은 아직 확정하지 않았다.

## P17: 현재 코드와 좌표의 차이

저작 정본은 `Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json`, revision 2500이다. `KOUKUSAYDON_G1_PATTERN_17.logic.1`의 CAPTURE 결과는 `BOSS_LEFT_HAND`, grip forward/up/right 모두 0이다. 참조 Hold는 `.logic.2`, 2029~7484 ms다.

`Server/Private/GameRoom_PlayerSimulation.cpp`의 `Capture_PlayerAttachment`는 잡힌 순간의 위치 차이를 boss-local offset으로 저장한다. `Update_PlayerAttachment`는 그 값을 현재 boss root/yaw로 복원한다. 애니메이션 손 본을 읽지 않는다. `Client/Private/Npc.cpp::Try_Get_PlayerHandGripSocketView`는 `bip001-l-hand` 본 누적 행렬과 보스 root를 곱한다. `Client/Public/PlayerHandGripTransform.h::Compose_WorldPosition`은 손 위치에 보스 수평 right/forward와 월드 up 거리 보정을 더한다.

현재 실제 고정 tick에서 측정한 값은 다음과 같다. 단위는 body preScale을 포함한 보스 로컬 m다.

| 피해 창 | 첫 elapsed tick / ms | 손 local XYZ | 기존 피해 BOX 포함 | 잡기 전 local X=8 유지 시 |
|---|---:|---|---|---|
| `.logic.5` | 128 / 4266.667 | 2.986141, 1.872237, 0.842714 | 포함 | 제외 |
| `.logic.9` | 145 / 4833.333 | 3.056019, 1.706235, 1.049023 | 포함 | 제외 |
| `.logic.10` | 160 / 5333.333 | 3.142766, 2.159642, 0.754131 | 포함 | 제외 |
| `.logic.11` | 215 / 7166.667 | 3.119938, 0.397867, 1.928220 | 포함 | 제외 |

기존 local X=3.1은 네 BOX에 모두 포함된다. 이는 잡힌 출발 위치에 따라 Server 피해가 달라질 수 있다는 근거다. **현재 BOX는 일반 BOSS BOX라 XZ 점 포함 분기를 사용한다.** 이 실측만으로 Client 화면의 3D 박스 정확도까지 검증했다고 말할 수 없다.

## P17: 후보 코드가 바꾸는 책임

전체 변경 파일은 `candidate.manifest.json`의 7개뿐이다. `candidate.patch`에 정확한 추가/교체 diff가 있고 `prepare_candidates.py`에는 기존 코드 기준 anchor 문자열과 교체문 전체가 있다. 아래 경로는 모두 이 문서가 있는 out 폴더 아래다.

| 전체 파일 후보 | 역할 |
|---|---|
| `candidate/Server/Public/GameplayCatalog.h` | `BOSS_CAPTURE_HAND_KEY{ iTimeMs, LocalPosition }` 및 window의 `CaptureHandKeys` |
| `candidate/Server/Private/GameplayCatalog.cpp` | `PATTERNLOGICHANDKEY` 읽기, finite/정렬/ordinal/범위/전체 Hold endpoint 검증 |
| `candidate/Server/Public/KoukuSaydonLogicRuntime.h` | 요청의 손 local 값, sampler/적용 함수 선언 |
| `candidate/Server/Private/KoukuSaydonLogicRuntime.cpp` | 현재 tick 손 보간, 소유 중인 capture player 갱신, 초기 capture 요청에 좌표 복사 |
| `candidate/Server/Private/GameRoom_KoukuPlayerCommands.cpp` | 기존 capture commit 성공 뒤 window index와 초기 손 위치 반영 |
| `candidate/Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py` | 실제 설치 body pose를 Hold 전체 시간에 구워 encounter `captureHandKeys` 생성 |
| `candidate/Tools/KoukuSaydonPipeline/KoukuBootstrapRows.ps1` | encounter key 검증과 새 bootstrap row 생성 |

새 C++ 파일은 없으므로 `.vcxproj`/`.filters` 추가는 없다. `verify_rows.ps1`이 candidate 폴더에 복사한 두 PS 의존 파일은 **변경 대상이 아니다.** 폴더 전체 덮어쓰기를 하지 말고 manifest의 7개 delta만 검토한다. 다른 에이전트의 조커 변경도 같은 LogicRuntime H/CPP에 있으므로 두 전체 파일을 순서대로 덮어쓰지 않고 diff를 누적 병합해야 한다.

핵심 삽입 기준:

1. `GameplayCatalog.h`: `BOSS_PATTERN_LOGIC_WINDOW` 앞에 typed hand key, window의 `strHoldLogicOccurrenceId` 뒤에 key vector.
2. `GameplayCatalog.cpp`: `PATTERNLOGICOUTCOME` 파서 앞에 hand key 파서. 최종 captureCount 검증 뒤에 정확한 Hold endpoint 검증.
3. `KoukuSaydonLogicRuntime.cpp`: `Update` 정의 앞에 `Try_SampleCaptureHandLocalPosition`/`Apply_CaptureHandLocalPosition`. Update의 ledger 유효성 검사 뒤, parent deadline/contact loop 전에 기존 capture player 갱신. Update 끝에 새 capture 요청의 손 위치 채우기.
4. `GameRoom_KoukuPlayerCommands.cpp`: `Capture_PlayerAttachment` 성공 블록에서 Answers SUCCESS를 쓰기 전에 window index와 초기 pose commit.
5. projector: `project_encounter` 앞에 `_project_capture_hand_keys`; source window를 `logic_windows.append(window)` 하기 직전에 호출.
6. PS: window exact-property 목록에 optional field 추가. 기존 SUCCESS/FAIL/TIMEOUT 결과 row loop 직전에 key 검증/row 생성.

`iTimeMs`는 capture 이후 시간이 아니라 **패턴 시작 이후 ms**다. `LocalPosition`은 보스 월드 root가 없는 local m다. publisher는 기존 bone bake의 애니메이션 clip/source time/blend, body preScale, stage root-motion 제거 정책을 재사용한다. grip은 본 회전·본 축 배율이 아니라 owner의 수평 축이므로 local에 `[rightM,upM,forwardM]` 순서로 더한다.

```text
L(t) = baked body left-hand pose(t) + [rightM, upM, forwardM]
worldX = bossX + cos(yaw)*Lx + sin(yaw)*Lz
worldY = bossY + Ly
worldZ = bossZ - sin(yaw)*Lx + cos(yaw)*Lz
```

`Try_SampleCaptureHandLocalPosition`은 두 key 사이를 선형 보간하고 실패하면 출력값을 보존한다. `Apply_CaptureHandLocalPosition`은 finite world 위치가 완성된 뒤 player XYZ와 다음 tick용 local offset을 함께 갱신한다. `iAttachmentWindowIndex`는 어느 capture window의 손 track인지 가리킨다. 기존 owner entity, pattern sequence, Hold deadline 검증을 유지한다. 다른 보스/다른 발생의 잡기에는 적용하지 않는다.

실행 순서는 `GameRoom::Tick → Update_Players → Update_WorldEntities → Apply_BossMotion/Apply_StageRootMotion → LogicRuntime::Update → 접촉 판정 → Apply_KoukuLogicOutput`이다. early `Update_PlayerAttachment`만 변경하면 현재 boss motion보다 한 tick 전 손 위치로 판정할 수 있다. 후보는 이번 tick의 boss motion 뒤, 접촉 판정 전에 실제 위치를 갱신한다. 초기 capture의 상태 commit은 기존대로 Room이 한다.

`p17_capture_hand_keys.candidate.json`과 `hand.rows.candidate.tsv`는 **검토 예시**다. 저작 JSON에 277개 key를 직접 붙이지 않는다. 실제 반영 때 publisher가 최신 저장본과 설치 WModel에서 다시 생성해야 한다. 기존 4개 피해 BOX는 이 후보에서 변경하지 않는다.

## P17: 실행한 검증과 남은 검증

- 현재 source/WModel에서 Hold 전체 hand key 277개를 생성했다. base 파일 SHA-256은 `audit.receipt.json`에 있다.
- 후보 projector가 같은 277개를 생성하는 것을 비교했다: `projector.verification.json`.
- 기존 전체 encounter를 후보 PS로 검증·row 생성했다. 277개 ordinal 정렬 통과, 마지막 Hold endpoint 손상은 거절했다: `rows.verification.json`.
- 전체 후보 `GameplayCatalog.cpp`, `KoukuSaydonLogicRuntime.cpp`, `GameRoom_KoukuPlayerCommands.cpp`를 일관된 candidate header overlay로 /c 컴파일했다: `compile.log`.
- 현 소스의 `Resolve_LogicRegionTransform`/`Contains_LogicRegion`와 후보 sampler/위치 함수의 실제 본문을 추출한 native fixture 26건이 통과했다. P17 네 피해 시점 × 보스 yaw 0/90/180/270도, 기존 local X=8 실패, P26 geometry 여섯 경우다: `fixture.run.log`.

**후보 전체 Server 링크·신규 parser를 통한 실제 full bootstrap load·잡기 후 네 번 HP 감소·수명 종료/다중 플레이어 회귀·사용자 화면 판정은 아직 하지 않았다.** /c와 수치 검증을 제품 기능 완료로 간주하지 않는다. 사용자가 구현한 뒤 해당 항목을 검증해야 한다.

## P26: 실제 bundle 5 room tick 재현

`p26_room_fixture.cpp`는 기존 `Run_ServerKoukuObjectOverlapContractTests` friend 경계로 현재 `CGameRoom`을 만들고 현 bootstrap을 읽는다. 원래 product contract object는 링크에서 제외했다. 제품 Server EXE를 실행하지 않았고 listener/socket을 열지 않았다. 현재 product 빌드 OBJ와 Shared Debug lib를 링크했으며 candidate hand ABI를 섞지 않았다. `room.rsp`/`room.cmd`와 `room.compile.log`, `room.run.log`에 재현 입력과 결과가 있다.

동일한 `koukusaydon.bundle.5`의 P25/P26 두 actor를 실행했다. 두 scenario 모두 전체 `CGameRoom::Tick(1/30)` 경로를 밟았다.

| scenario | P26 직전 상태 | elapsed 510 / 실제 tick 511 결과 |
|---|---|---|
| 0: 처음부터 찍기 중심에 서 있음 | 앞선 pizza 진행으로 HP 0, DEAD | ledger 창은 열리지만 피해·push 없음 |
| 1: elapsed 509에 한 번 생존·일반 상태와 중심 좌표 배치 | HP 10000, NONE, ready, 무적/시정/보호막 없음 | HP **9048**, push **1.50000012초**, answer SUCCESS, damage event **1** |

scenario 1의 접촉 좌표는 `(4.34000015, 10.5600004, 323.100006)`이다. 다음 tick에는 `(4.34000015,10.7995558,322.877777)`로 이동했고 push 시간이 1.46666682초로 줄었다. elapsed 513에서 창이 닫혀도 이미 시작한 넉백은 계속된다. 현재 bundle clock, WORLD anchor, overlap, HP와 push 경로가 연결됨을 확인했다.

이 fixture는 실제 사용자의 입력·피격 이력·실행 중 generation을 재현하지 않는다. scenario 0의 죽음이 **사용자의 실패 원인이라고 확정할 수 없다.** scenario 1은 원인 분리를 위해 직전 player 상태를 의도적으로 정상화했다.

현재 P26 값:

```text
KOUKUSAYDON_G1_PATTERN_26.logic.1: ENTER_AREA, 16997~17097 ms
KOUKUSAYDON_G1_PATTERN_26.presentation.7: WORLD CYLINDER
center = [4.3400001526, 10, 323.100006104], radius = 4.8999999166, halfY = 1.25
MAX_HP_PERCENT_DAMAGE 10
pushRangeM=10, pushMs=1500, AWAY_FROM_CONTACT, forcePush=true,
pushCanLeaveArena=true, pushBallistic=true
```

vertical 포함식은 `abs(playerY + 0.90 - 10) <= 1.25 + 0.90 + 0.001`이다. 중심 XZ에서 지면 Y 10/10.56은 포함되고 Y 13 또는 0은 제외된다. XZ는 플레이어 반경 0.45+margin 0.001도 포함하므로 약 5.351 m까지 접촉할 수 있다. WORLD이므로 boss root/yaw를 다시 더하지 않는다.

## P26: 사용자의 실패를 좁힐 관측점과 조건별 조치

추측성 수정 대신 위 room fixture의 동일 로그 형식을 사용한다. 관측할 값은 bundle/member ID, pattern/sequence/start tick, elapsed 505~515, player XYZ/HP/action/ready, invulnerability/time-stop/protection, window opened/closed/answer, damage event, push remaining이다.

1. **P26 또는 window가 없음/시각이 다름:** active/pinned gameplay revision, source revision, 저장된 bundle 멤버를 비교한다. 설치 파일만 교체된 상태와 현재 실행 중 generation은 다르다. 최신 저장본 게시 후 새로운 패턴 실행으로 새 generation이 들어갔는지 확인한다.
2. **HP 0/DEAD/FALLING/GRABBED, TimeStop:** 현재 admission이 의도대로 거절할 수 있다. 무조건 우회하는 코드를 넣지 않는다. 앞선 P25 피해/상태를 먼저 확인한다.
3. **살아 있지만 XZ/높이 밖:** 현재 발 원점과 위 식을 대조한다. 실제 의도한 망치 위치가 다르다고 확인된 경우에만 `.presentation.7`의 MAP 위치·높이·크기를 최신 디스크 저장본의 해당 필드에 수정한다. WORLD 중복 root 변환은 하지 않는다.
4. **inside/answer SUCCESS이나 HP 변화 없음:** `CKoukuSaydonLogicRuntime::Apply_Result`의 `Apply_WorldToPlayer` 반환값을 debugger로 관찰한다. HolyCharm, Esther/Ronaun guard, invulnerability는 ABSORBED 경로다. 현재 Logic은 contact 성공과 HP 적용 성공을 구분하지 않고 answer를 먼저 SUCCESS로 기록한다.
5. **HP가 줄었지만 push 0:** `CPlayerSkillSystem::Can_ArmPlayerHitReaction`에서 forcePush라도 pattern-bound, arena-ejection, DEAD/FALLING/GRABBED/TRIGGER_MOVE/WALL_CLIMB 등을 거절한다. Esther guard 처리도 피해 후 reaction 전 반환할 수 있다. 이 상태가 실제로 확인되기 전에는 전역 예외를 완화하지 않는다.
6. **push 양수인데 화면이 정지:** 다음 Server snapshot XYZ를 먼저 비교한다. 위 fixture에서는 다음 tick부터 실제 XYZ가 변했다. Server가 이동했다면 Client replication/presentation 소비를 별도 조사한다.

현재 근거로 P26 제품 코드/데이터 교체안은 없다. **실제 bundle tick fixture 자체가 정확히 실행 가능한 진단 코드**이며, 새 임시 제품 로그 훅을 넣을 필요 없이 지금 두 조건의 차이를 재현한다.

## 게시·버전·미리보기 경계

현 bootstrap version은 38이다. 후보는 version 숫자를 임의로 올리지 않았지만 새 `PATTERNLOGICHANDKEY` row를 추가한다. 구 Server는 미지원 row를 거절하고, 신 parser는 손 key가 없는 기존 P17 capture 자료를 거절한다. 따라서 후보 구현과 새 자료 게시를 같은 배포 단위로 준비해야 한다. 초기 load 실패와 active generation hot-load 실패는 다르며, 실패한 staged catalog가 기존 active를 덮지 않는지 반드시 검증한다. 이 새로운 필수 자료 계약을 팀 사용서/RESULT에 명시해야 한다.

Workbench 미저장 preview는 현재 메모리 draft다. 저장한 composition, 생성 encounter, 설치 bootstrap, 실행 중 Server pinned generation은 각각 다른 단계다. 이번 out key 생성은 어느 단계도 교체하지 않았다. 사용자의 직접 수정이 끝나고 저장한 뒤 최신 파일 hash를 다시 확인하고 필요한 delta만 병합한다.

아래는 **아직 실행하지 않은**, 직접 구현·저장 후 사용하는 기존 게시 진입점이다. 먼저 새 Server가 컴파일되는지 확인하고, 저장본 검토가 끝난 뒤 게시한다. Python은 실제 Python 실행파일이 PATH에 있어야 한다.

```powershell
python -B Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py --repository-root . --mode publish
powershell -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-GameplayBalance.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-GameplayBalance.ps1 -Mode Publish
```

첫 명령은 authoring projection을 만들고, Gameplay publisher가 실제 Server bootstrap을 만든다. publish만으로 현재 실행 중 Server의 pinned pattern이 바뀌었다고 설명하지 않는다. 사용자 저장/빌드/새 generation 확인 순서가 끝난 뒤 P17 실제 HP 4회·해제와 P26 같은 bundle 재현을 다시 본다.


## 반영 순서와 겹치는 파일

이 문서의 전체 C++ 정본은 G1·G2 조커 변경과 손 추적 변경을 합친 **누적 후보**다. 제품 파일에는 쓰지 않았다.
각 후보의 개별 컴파일은 성공했지만, 누적 병합본의 새 빌드는 사용자 요청에 따라 아직 하지 않았다.
아래 patch는 현재 제품 기준 손 추적 변경만 표시한다. G1·G2를 이미 구현했다면 해당 변경을 유지하면서 이 patch의 새 부분을 적용한다.
전체 파일을 사용자 최신 수정 위에 덮어쓰지 않는다. 신규 C++ 파일은 없으며 vcxproj/filter 항목도 추가하지 않는다.

1. GameplayCatalog의 key 타입·창 필드와 typed row parsing/검증을 연결한다.
2. LogicRuntime의 보간·위치 적용과 Room의 최초 capture commit을 연결한다.
3. projector와 bootstrap row 작성기를 함께 반영한다. 새 parser 빌드 전에 새 행을 기존 Server에 게시하지 않는다.
4. 현재 저장본에서 저작→projection→게시를 실행하고 새 Server generation과 실제 손·피해 시각을 확인한다.

## 손 추적의 정확한 변경 블록

```diff
--- a/Server/Public/GameplayCatalog.h
+++ b/Server/Public/GameplayCatalog.h
@@ -785,6 +785,14 @@
 
 	// Workbench에서 게시한 Kouku 판정창 하나다. iStartMs/iDurationMs는 pattern 시계 기준이며 eKind가 사용할 값을 결정한다.
 	// 시간·영역·대상과 결과 정의를 담고, 현재 완료 여부/중복 접촉/고정 영역은 LogicRuntime의 ledger가 소유한다.
+	// 게시자가 실제 설치 본 pose에서 만든 손 위치다. 시간은 패턴 시작 이후 ms,
+	// 위치는 body preScale을 포함한 보스 로컬 m이며 보스 월드 root는 포함하지 않는다.
+	struct BOSS_CAPTURE_HAND_KEY final
+	{
+		std::uint32_t iTimeMs = 0u;
+		std::array<float, 3u> LocalPosition{};
+	};
+
 	struct BOSS_PATTERN_LOGIC_WINDOW final
 	{
 		std::string strWindowId;
@@ -829,6 +837,8 @@
 		float fBossChargeDistanceM = 0.f;
         float fChargeYawOffsetDegrees = 0.f;
         std::string strHoldLogicOccurrenceId;
+		// CAPTURE 창이 소유한다. 참조한 Hold 전체 수명을 덮어 피해창 종료 뒤에도 손을 따라간다.
+		std::vector<BOSS_CAPTURE_HAND_KEY> CaptureHandKeys;
 	};
 
 	enum class BOSS_PATTERN_MECHANIC_TRIGGER_KIND : std::uint8_t
--- a/Server/Public/KoukuSaydonLogicRuntime.h
+++ b/Server/Public/KoukuSaydonLogicRuntime.h
@@ -126,6 +126,8 @@
         LostArk::Shared::PLAYER_ATTACHMENT_SLOT eAttachmentSlot = LostArk::Shared::PLAYER_ATTACHMENT_SLOT::NONE;
         std::uint32_t iHoldEndTick = 0u;
         std::uint32_t iWindowIndex = 0u;
+        bool bHasHandLocalPosition = false;
+        std::array<float, 3u> HandLocalPosition{};
     };
 
 	/* Update가 룸에 요청하는 송신·후속 패턴·포획 결과다. 플레이어·보스 상태는 직접 갱신하지만 송신과 실행 목록 소유권은 룸에 둔다. */
@@ -186,6 +188,13 @@
 			KOUKUSAYDON_LOGIC_LEDGER& ledger,
 			std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
 			SERVER_WORLD_ENTITY* pBoss);
+		// 게시된 손 key를 현재 패턴 시각에 선형 보간한다. 실패 시 outLocal은 보존한다.
+		static bool Try_SampleCaptureHandLocalPosition(const BOSS_PATTERN_LOGIC_WINDOW& window,
+			double patternElapsedMs, std::array<float, 3u>& outLocal) noexcept;
+		// LogicRuntime의 접촉 판정 직전과 GameRoom의 잡기 commit 직후에만 호출한다.
+		// 현재 보스 root/yaw를 한 번 적용하고 서버 실제 발 원점과 다음 tick용 local offset을 함께 갱신한다.
+		static bool Apply_CaptureHandLocalPosition(SERVER_PLAYER& player, const SERVER_WORLD_ENTITY& boss,
+			const std::array<float, 3u>& local) noexcept;
 		static void Update(
 			SERVER_WORLD_ENTITY& boss,
 			const BOSS_PATTERN_DEFINITION& pattern,
--- a/Server/Private/KoukuSaydonLogicRuntime.cpp
+++ b/Server/Private/KoukuSaydonLogicRuntime.cpp
@@ -1747,6 +1747,48 @@
 	}
 }
 
+// 현재 pattern clock을 사용한다. 반올림된 capture 시작 tick을 다시 빼면
+// 30 Hz 경계에서 Client의 clip 시계와 어긋나므로 key 자체를 절대 pattern ms로 게시한다.
+bool LostArk::Server::CKoukuSaydonLogicRuntime::Try_SampleCaptureHandLocalPosition(
+	const BOSS_PATTERN_LOGIC_WINDOW& window, const double patternElapsedMs,
+	std::array<float, 3u>& outLocal) noexcept
+{
+	const auto& keys = window.CaptureHandKeys;
+	if (keys.empty() || !std::isfinite(patternElapsedMs) || patternElapsedMs < keys.front().iTimeMs)
+		return false;
+	const double time = (std::min)(patternElapsedMs, static_cast<double>(keys.back().iTimeMs));
+	const auto next = std::upper_bound(keys.begin(), keys.end(), time,
+		[](const double ms, const BOSS_CAPTURE_HAND_KEY& key) { return ms < key.iTimeMs; });
+	const auto& left = next == keys.begin() ? keys.front() : *(next - 1);
+	const auto& right = next == keys.end() ? left : *next;
+	const float alpha = right.iTimeMs > left.iTimeMs ?
+		static_cast<float>((time - left.iTimeMs) / (right.iTimeMs - left.iTimeMs)) : 0.f;
+	std::array<float, 3u> staged{};
+	for (std::size_t axis = 0u; axis < staged.size(); ++axis)
+	{
+		staged[axis] = left.LocalPosition[axis] + (right.LocalPosition[axis] - left.LocalPosition[axis]) * alpha;
+		if (!std::isfinite(staged[axis])) return false;
+	}
+	outLocal = staged;
+	return true;
+}
+
+bool LostArk::Server::CKoukuSaydonLogicRuntime::Apply_CaptureHandLocalPosition(
+	SERVER_PLAYER& player, const SERVER_WORLD_ENTITY& boss, const std::array<float, 3u>& local) noexcept
+{
+	// D3D 행 벡터의 수평 회전: X'=cos*yaw*X+sin*yaw*Z, Z'=-sin*yaw*X+cos*yaw*Z.
+	// 본 샘플에는 이미 모델 preScale과 grip의 right/up/forward 거리 보정이 포함되어 있다.
+	const float yaw = boss.fYawDegrees * 0.017453292519943295f;
+	const std::array<float, 3u> world{boss.fPositionX + std::cos(yaw) * local[0] + std::sin(yaw) * local[2],
+		boss.fPositionY + local[1], boss.fPositionZ - std::sin(yaw) * local[0] + std::cos(yaw) * local[2]};
+	for (const float value : world) if (!std::isfinite(value)) return false;
+	player.fAttachmentLocalOffsetX = local[0];
+	player.fAttachmentLocalOffsetY = local[1];
+	player.fAttachmentLocalOffsetZ = local[2];
+	player.fPositionX = world[0]; player.fPositionY = world[1]; player.fPositionZ = world[2];
+	return true;
+}
+
 void LostArk::Server::CKoukuSaydonLogicRuntime::Update(
 	SERVER_WORLD_ENTITY& boss,
 	const BOSS_PATTERN_DEFINITION& pattern,
@@ -1761,6 +1803,22 @@
 {
 	if (!ledger.Is_Active() || ledger.strPatternId != pattern.strPatternId)
 		return;
+	// GameRoom은 이번 tick의 boss motion/root motion을 적용한 뒤 이 함수를 호출한다.
+	// early Update_Players에서 이전 손 위치를 root로 복원했더라도 여기서 현재 pose로
+	// 갱신한 다음 피해 영역을 판정한다. 다른 보스/다른 pattern 발생의 잡기는 건드리지 않는다.
+	for (auto& [playerId, captured] : players)
+	{
+		(void)playerId;
+		if (!Is_CurrentBossHandCapture(captured, boss.iNetEntityId, boss.iPatternSequence, serverTick) ||
+			captured.iAttachmentWindowIndex >= pattern.LogicWindows.size()) continue;
+		const auto& captureWindow = pattern.LogicWindows[captured.iAttachmentWindowIndex];
+		if (captureWindow.CaptureHandKeys.empty()) continue;
+		std::array<float, 3u> local{};
+		if (!Try_SampleCaptureHandLocalPosition(captureWindow,
+			static_cast<double>(serverTick - ledger.iPatternStartTick) * (1000.0 / 30.0), local) ||
+			!Apply_CaptureHandLocalPosition(captured, boss, local))
+			captured.iAttachmentEndTick = serverTick;
+	}
 	// Parent deadlines cancel a child before contact, charge, or end-tick judgement.
 	for (auto& state : ledger.Windows)
 	{
@@ -2437,6 +2495,15 @@
 			break;
 		}
 	}
+	// 잡기의 상태 commit은 GameRoom이 담당한다. 그 직후 같은 tick의 손 좌표를 적용할
+	// 값만 요청에 복사하여 객체/본 포인터 수명이나 Client pose에 의존하지 않는다.
+	for (auto& request : outOutput.CaptureRequests)
+	{
+		if (request.iWindowIndex >= pattern.LogicWindows.size()) continue;
+		request.bHasHandLocalPosition = Try_SampleCaptureHandLocalPosition(
+			pattern.LogicWindows[request.iWindowIndex],
+			static_cast<double>(serverTick - ledger.iPatternStartTick) * (1000.0 / 30.0), request.HandLocalPosition);
+	}
 }
 
 bool LostArk::Server::CKoukuSaydonLogicRuntime::Record_InteractionSlot(
--- a/Server/Private/GameRoom_KoukuPlayerCommands.cpp
+++ b/Server/Private/GameRoom_KoukuPlayerCommands.cpp
@@ -879,6 +879,15 @@
         if (Capture_PlayerAttachment(request.iPlayerNetEntityId, boss.iNetEntityId,
             request.eAttachmentSlot, serverTick, request.iHoldEndTick, boss.iPatternSequence))
         {
+            auto& captured = m_Players.at(player->second);
+            captured.iAttachmentWindowIndex = request.iWindowIndex;
+            if (request.bHasHandLocalPosition &&
+                !CKoukuSaydonLogicRuntime::Apply_CaptureHandLocalPosition(captured, boss, request.HandLocalPosition))
+            {
+                captured.iAttachmentEndTick = serverTick;
+                m_strStatus = "Capture hand position was not finite after room commit";
+                continue;
+            }
             member->LogicLedger.Windows[request.iWindowIndex].Answers[player->second] = KOUKUSAYDON_LOGIC_ANSWER::SUCCESS;
             member->LogicLedger.Windows[request.iWindowIndex].InsidePlayers.insert(player->second);
         }
--- a/Server/Private/GameplayCatalog.cpp
+++ b/Server/Private/GameplayCatalog.cpp
@@ -3525,6 +3525,33 @@
 				window->PatternIds.emplace_back(fields[i]);
 			}
 			window->iCompletionCount = count;
+		}
+		else if (!fields.empty() && "PATTERNLOGICHANDKEY" == fields[0])
+		{
+			if (fields.size() != 9u)
+			{ m_strStatus = "Capture hand key field count is invalid"; return false; }
+			const auto owners = m_BossPatterns.find(std::string(fields[1]));
+			if (owners == m_BossPatterns.end())
+			{ m_strStatus = "Capture hand encounter missing"; return false; }
+			const auto pattern = std::find_if(owners->second.begin(), owners->second.end(),
+				[&](const auto& row) { return row.strPatternId == fields[2]; });
+			if (pattern == owners->second.end())
+			{ m_strStatus = "Capture hand pattern missing"; return false; }
+			const auto window = std::find_if(pattern->LogicWindows.begin(), pattern->LogicWindows.end(),
+				[&](const auto& row) { return row.strWindowId == fields[3]; });
+			if (window == pattern->LogicWindows.end() || window->eKind != BOSS_PATTERN_LOGIC_KIND::ENTER_AREA)
+			{ m_strStatus = "Capture hand ENTER_AREA missing"; return false; }
+			BOSS_CAPTURE_HAND_KEY key{};
+			std::uint32_t ordinal = 0u;
+			if (!ParseNumber(fields[4], ordinal) || ordinal != window->CaptureHandKeys.size() || ordinal >= 4096u ||
+				!ParseNumber(fields[5], key.iTimeMs) || key.iTimeMs > 600000u ||
+				(!window->CaptureHandKeys.empty() && key.iTimeMs <= window->CaptureHandKeys.back().iTimeMs))
+			{ m_strStatus = "Capture hand key order/time invalid"; return false; }
+			for (std::size_t axis = 0u; axis < 3u; ++axis)
+				if (!ParseNumber(fields[6u + axis], key.LocalPosition[axis]) ||
+					!std::isfinite(key.LocalPosition[axis]) || std::abs(key.LocalPosition[axis]) > 100000.f)
+				{ m_strStatus = "Capture hand boss-local position invalid"; return false; }
+			window->CaptureHandKeys.push_back(key);
 		}
 		else if (!fields.empty() && "PATTERNLOGICOUTCOME" == fields[0])
 		{
@@ -8333,6 +8360,18 @@
                 if (captureCount > 1 || (captureCount == 1 && window.OnSuccess.size() != 1u) ||
                     (captureCount == 1) != !window.strHoldLogicOccurrenceId.empty())
                     return fail("One capture Success requires one explicit Hold reference");
+                if (captureCount == 1)
+                {
+                    const auto hold = std::find_if(pattern.LogicWindows.begin(), pattern.LogicWindows.end(),
+                        [&](const auto& row) { return row.strWindowId == window.strHoldLogicOccurrenceId; });
+                    if (hold == pattern.LogicWindows.end() || hold->eKind != BOSS_PATTERN_LOGIC_KIND::ATTACHMENT_HOLD ||
+                        window.CaptureHandKeys.size() < 2u ||
+                        window.CaptureHandKeys.front().iTimeMs != hold->iStartMs ||
+                        window.CaptureHandKeys.back().iTimeMs != static_cast<std::uint64_t>(hold->iStartMs) + hold->iDurationMs)
+                        return fail("Capture hand keys must cover the complete Hold lifetime");
+                }
+                else if (!window.CaptureHandKeys.empty())
+                    return fail("Only CAPTURE_PLAYER owns hand keys");
                 if (!window.strHoldLogicOccurrenceId.empty() && window.eKind != BOSS_PATTERN_LOGIC_KIND::ENTER_AREA)
                     return fail("Only ENTER_AREA owns a Hold reference");
                 if (window.eKind == BOSS_PATTERN_LOGIC_KIND::ATTACHMENT_HOLD &&
--- a/Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py
+++ b/Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py
@@ -4997,6 +4997,32 @@
     return {**document, "patterns": patterns} if changed else document
 
 
+def _project_capture_hand_keys(document, pattern, capture_box, window, root, cache, suppress_root_motion):
+    """Client 손 소켓과 같은 body pose + owner-yaw grip을 서버 로컬 m로 굽는다.
+
+    저작 JSON에 파생 key를 저장하지 않는다. source와 설치 WModel을 publisher의
+    기존 freshness transaction 안에서 읽고, CAPTURE 창의 참조 Hold 전체를 덮는다.
+    """
+    captures = [row for row in window["onSuccess"] if row["kind"] == "CAPTURE_PLAYER"]
+    if not captures:
+        return None
+    if len(captures) != 1 or captures[0]["attachmentSlot"] != "BOSS_LEFT_HAND":
+        raise CompositionError("Capture hand baking requires one BOSS_LEFT_HAND result")
+    holds = [row for row in pattern["logicOccurrences"]
+             if row["occurrenceId"] == capture_box.get("holdLogicOccurrenceId") and row.get("enabled", True)]
+    if len(holds) != 1:
+        raise CompositionError("Capture hand baking requires an enabled explicit Hold")
+    grip = captures[0]["gripLocalOffset"]  # forward/up/right, boss horizontal axes, metres
+    track = _project_bone_collider_track(pattern, holds[0], {
+        "anchorKind": "BOSS", "followBoss": True, "bone": "bip001-l-hand",
+        "boneTarget": "BODY", "boneRotation": "TARGET_YAW",
+        "positionOffset": [0, 0, 0], "rotationDegrees": [0, 0, 0]},
+        root, cache, suppress_root_motion, resolve_animation_blend_windows(document, pattern))
+    return [{"timeMs": track["startMs"] + key["timeMs"], "positionOffset": [
+        key["positionOffset"][0] + grip["rightM"], key["positionOffset"][1] + grip["upM"],
+        key["positionOffset"][2] + grip["forwardM"]]} for key in track["keys"]]
+
+
 def project_encounter(document: dict[str, Any], root: Path = REPOSITORY_ROOT) -> dict[str, Any]:
     document = _project_disabled_bingo_controls(document)
     source_logics = {logic["logicId"]: logic for logic in document.get("logics", [])}
@@ -5077,6 +5103,9 @@
                 window.update(_project_object_overlap_target(document, source, box, logic, logics, world_sequences))
             elif kind == "OBJECT_CONTACT":
                 window.update(_project_object_contact_targets(document, source, box, logic, logics, world_sequences))
+            hand_keys = _project_capture_hand_keys(document, source, box, window, root, bone_cache, bool(root_motion))
+            if hand_keys is not None:
+                window["captureHandKeys"] = hand_keys
             logic_windows.append(window)
         if world_boxes:
             model_cache = session.models if session is not None else {}
--- a/Tools/KoukuSaydonPipeline/KoukuBootstrapRows.ps1
+++ b/Tools/KoukuSaydonPipeline/KoukuBootstrapRows.ps1
@@ -1186,6 +1186,7 @@
 		if ($null -ne $window.PSObject.Properties['repeatIntervalMs']) { $windowProperties += 'repeatIntervalMs' }
 		if ($null -ne $window.PSObject.Properties['ownerWorldOccurrenceId']) { $windowProperties += 'ownerWorldOccurrenceId' }
 		if ($null -ne $window.PSObject.Properties['holdLogicOccurrenceId']) { $windowProperties += 'holdLogicOccurrenceId' }
+		if ($null -ne $window.PSObject.Properties['captureHandKeys']) { $windowProperties += 'captureHandKeys' }
 		if ($null -ne $window.PSObject.Properties['cancelAtEnd']) {
 			$windowProperties += 'cancelAtEnd'
 			if ($window.cancelAtEnd -isnot [bool]) { throw 'Logic cancelAtEnd must be boolean' }
@@ -1605,6 +1606,36 @@
 				}
 			}
 		}
+		$captures = @($window.onSuccess | Where-Object { $_.kind -ceq 'CAPTURE_PLAYER' })
+		if ($captures.Count -gt 0) {
+			$handKeys = @($window.captureHandKeys)
+			$holds = @($koukuPattern.logicWindows | Where-Object { $_.windowId -ceq $window.holdLogicOccurrenceId })
+			if ($captures.Count -ne 1 -or $window.captureHandKeys -isnot [Array] -or
+				$handKeys.Count -lt 2 -or $handKeys.Count -gt 4096 -or $holds.Count -ne 1 -or
+				$handKeys[0].timeMs -ne $holds[0].startMs -or
+				$handKeys[-1].timeMs -ne ([uint64]$holds[0].startMs + [uint64]$holds[0].durationMs)) {
+				throw 'Capture hand keys must cover the complete Hold lifetime'
+			}
+			$previousHandMs = -1
+			for ($handIndex = 0; $handIndex -lt $handKeys.Count; ++$handIndex) {
+				$key = $handKeys[$handIndex]
+				Assert-ExactProperties $key @('timeMs','positionOffset') 'Capture hand key'
+				Assert-JsonInteger $key.timeMs 'Capture hand key time' 0 600000
+				if ($key.timeMs -le $previousHandMs -or $key.positionOffset -isnot [Array] -or @($key.positionOffset).Count -ne 3) {
+					throw 'Capture hand keys require ordered time and three local position components'
+				}
+				$handRow = @('PATTERNLOGICHANDKEY', $koukuEncounterDocument.encounterId, $koukuPattern.patternId,
+					$window.windowId, $handIndex, $key.timeMs)
+				foreach ($component in $key.positionOffset) {
+					Assert-JsonNumber $component 'Capture hand position'
+					if ([Math]::Abs([double]$component) -gt 100000) { throw 'Capture hand position outside bounds' }
+					$handRow += Format-InvariantSignedFloat $component 'Capture hand position'
+				}
+				$patternRows.Add(($handRow -join "`t"))
+				$previousHandMs = $key.timeMs
+			}
+		}
+		elseif ($null -ne $window.PSObject.Properties['captureHandKeys']) { throw 'Only CAPTURE_PLAYER owns hand keys' }
 		foreach ($slotName in @('SUCCESS','FAIL','TIMEOUT')) {
 			$slotProperty = 'onTimeout'
 			if ($slotName -ceq 'SUCCESS') { $slotProperty = 'onSuccess' }
```


## Server/Public/GameplayCatalog.h 전체 코드

수정 대상: [GameplayCatalog.h](C:/Users/tnest/Desktop/LostArk/Server/Public/GameplayCatalog.h)

```cpp
#pragma once

#include "Gameplay/AttackHitTemplate.h"

#include "Network/PacketMessages.h"

#include <array>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace LostArk::Server
{
	// 게시자와 Server 로더가 함께 사용하는 gameplay bootstrap 버전이다. 로더는 이 버전과 다른 입력을 거부한다.
	// 그래 이 bootstrap의 불일치 때문에 컴파일 에러 나고 뭐 그런 경우가 많았는데,
	// 왜 이게 강제가 되어야 하고, 이 bootstrap에 대한 것이 필요한지 조차, 그 구조가 왜 그렇게 설계가 되어야 하는지도 좀 
	// 의문이 들었다. 왜 bootstrap이 필요한 구조인 거야? 
	inline constexpr std::uint32_t GAMEPLAY_BOOTSTRAP_VERSION =
		LostArk::Shared::GAMEPLAY_BOOTSTRAP_FORMAT_VERSION;

	// 클립에 저작된 누적 이동의 한 표본이다. 플레이어 스킬과 보스 stage가 각자의 경과 ms로 샘플링한다.
	// 전방/옆/위 변위는 m이며, 현재 위치에 표본 전체를 반복 더하지 않고 구간 차이를 소비한다.
	struct ROOT_MOTION_SAMPLE
	{
		std::uint32_t iTimeMs = 0;
		float fForward = 0.f;
		float fLateral = 0.f;
		// 선택적인 네 번째 성분이다. 수평 이동만 읽는 소비자는 이 수직 값을 사용하지 않는다.
		float fUp = 0.f;
	};

	// 스킬 시계에 예약할 타격 정의다. PLAYER_SKILL_DEFINITION 또는 combo stage가 목록을 소유하고 PlayerSkillSystem이 소비한다.
	// 반복 횟수와 예약 시간은 정의이며, 이미 소비한 타격과 대상별 적용 기록은 실행 상태 쪽에서 관리한다.
	struct PLAYER_SKILL_HIT final
	{
		// 0은 기존 복합 타격이고 독립 결과 채널을 쓰는 데이터는 아래 종류를 구분한다.
		// 1은 피해/부위/밀기, 2는 무력화, 3은 카운터다. 소비자는 해당 채널의 값만 적용한다.
		std::uint32_t iResultKind = 0u;
		std::uint32_t iTimeMs = 0;
		std::uint32_t iRepeatCount = 1;
		std::uint32_t iRepeatMs = 0;
		// AreaType 1은 원/링(range, inner), 2는 전방 box(range 길이, width), 3은 부채꼴(range, angle, inner)이다.
		std::uint32_t iAreaType = 0;
		float fRange = 0.f;
		float fAngleDegrees = 0.f;
		float fWidth = 0.f;
		float fHeight = 0.f;
		float fOffset = 0.f;
		float fInner = 0.f;
		std::uint32_t iMaxTargets = 0;
		// 이 타격의 밀기 기간은 ms, 거리는 m다. 기간 0이면 이동하지 않으며 음수 거리는 공격 원점 쪽으로 당긴다.
		std::uint32_t iPushMs = 0;
		float fPushRange = 0.f;
		std::uint32_t iDurationMs = 0;
	};

	enum class PLAYER_PROJECTILE_KIND : std::uint8_t
	{
		MISSILE,
		FIXAREA,
		GRENADE,
		TRACE
	};

	// 전투 오브젝트 자체가 적용할 타격이다. contact는 접촉 대상별 반복 횟수/간격을, timed는 생성 후 iTimeMs 일정을 사용한다.
	// 도형 원점과 전방은 시전자의 현재 자세가 아니라 해당 오브젝트의 자세를 기준으로 한다.
	struct PLAYER_PROJECTILE_HIT final
	{
		bool isContact = false;
		PLAYER_SKILL_HIT Hit;
	};

	// 시전자 자세에서 전방/오른쪽 offset으로 생성할지, 거리 제한을 거친 조준 지점에 생성할지 정한다.
	enum class PLAYER_PROJECTILE_ORIGIN : std::uint8_t
	{
		CASTER,
		AIM
	};

	// 스킬 action의 iTimeMs에 생성하는 오브젝트 정의다. 이동 속도는 m/s, 거리와 반지름은 m, 수명은 ms다.
	// FIXAREA는 원점에 머무르며 나머지 이동 종류의 실제 경로는 CombatObjectRuntime이 해석한다.
	struct PLAYER_SKILL_PROJECTILE final
	{
		std::uint32_t iTimeMs = 0;
		PLAYER_PROJECTILE_KIND eKind = PLAYER_PROJECTILE_KIND::MISSILE;
		PLAYER_PROJECTILE_ORIGIN eOrigin = PLAYER_PROJECTILE_ORIGIN::CASTER;
		float fOffsetForward = 0.f;
		float fOffsetRight = 0.f;
		float fSpeed = 0.f;
		float fMinDistance = 0.f;
		float fMaxDistance = 0.f;
		std::uint32_t iLifeMs = 0;
		float fRadius = 0.f;
		std::vector<PLAYER_PROJECTILE_HIT> Hits;
	};

	// 실행 중인 action이 다음 입력을 허용하는 ms 구간이다. 스킬/이동/회피는 각자 다른 목록을 사용한다.
	// 콤보 stage가 있으면 해당 stage의 시계를 기준으로 검사한다.
	struct PLAYER_CANCEL_WINDOW final
	{
		std::uint32_t iStartMs = 0;
		std::uint32_t iEndMs = 0;
	};

	enum class PLAYER_CANCEL_INPUT : std::uint8_t
	{
		SKILL,
		MOVE,
		DODGE,
	};

	// 콤보/홀드/카운터의 한 단계 정의다. 단계 전환 때 실행 시계를 초기화하므로 타격/이동/취소 시간도 이 단계 기준이다.
	struct PLAYER_COMBO_STAGE final
	{
		std::uint32_t iActionDurationMs = 0;
		std::uint32_t iHitTimeMs = 0;
		// 피해 발생 시각과 다음 action/stage로 넘어가는 시각을 별도로 저장한다.
		std::uint32_t iComboAdvanceMs = 0;
		std::uint32_t iInputOpenMs = 0;
		std::uint32_t iInputCloseMs = 0;
		// stage 전환 때 action 시계를 초기화하므로 이동/타격/취소 구간도 각 stage가 소유한다.
		std::vector<ROOT_MOTION_SAMPLE> RootMotion;
		std::vector<PLAYER_SKILL_HIT> Hits;
		std::vector<PLAYER_SKILL_PROJECTILE> Projectiles;
		std::vector<PLAYER_CANCEL_WINDOW> SkillCancelWindows;
		std::vector<PLAYER_CANCEL_WINDOW> MoveCancelWindows;
		std::vector<PLAYER_CANCEL_WINDOW> DodgeCancelWindows;
	};

	// stable SKILL_ID로 조회할 승인된 스킬 정의다. 입력 slot/재사용 시간/자원 비용과 판정·이동 목록을 함께 보관한다.
	// 실행 중 HP, cooldown, combo 진행 상태는 PlayerSkillSystem이 갱신하는 SERVER_PLAYER가 소유한다.
	struct PLAYER_SKILL_DEFINITION
	{
		LostArk::Shared::SKILL_ID iSkillId = LostArk::Shared::INVALID_SKILL_ID;
		LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass =
			LostArk::Shared::CHARACTER_CLASS_ID::END;
		std::string strInputSlot;
		std::string strActionId;
		std::string strDamageProfileId;
		std::uint32_t iCooldownMs = 0;
		std::uint32_t iActionDurationMs = 0;
		std::uint32_t iHitTimeMs = 0;
		std::uint32_t iResourceCost = 0;
		// Try_Start가 스킬 시작을 승인할 때 한 번 차감하는 직업 identity 비용이다. 부족하면 시작을 거부하며 0이면 차감하지 않는다.
		std::uint32_t iIdentityCost = 0;
		// 이 프로젝트가 조정하는 보스 상호작용 값이다. 0이면 해당 채널을 제공하지 않으며 일반 HP 피해와 분리한다.
		std::uint32_t iStaggerDamage = 0;
		std::uint32_t iPartDamage = 0;
		std::uint32_t iCounterPower = 0;
		float fMovementDistance = 0.f;
		// 추출한 RootMotion 표본을 고치지 않고 소비하는 수평 이동량에 곱하는 배율이다.
		// 1이면 기존 변위를 유지하고 action 길이는 바꾸지 않아 배율이 크면 같은 시간에 더 멀리 이동한다.
		float fRootMotionScale = 1.f;
		// 가디언 나이트의 불씨 획득/소비와 소켓 잠금 정의다. 다른 스킬은 기본 0/false를 사용한다.
		// 현재 불씨와 잠긴 소켓 수는 이 정의가 아니라 SERVER_PLAYER가 소유한다.
		std::uint32_t iEmberGain = 0;
		std::uint32_t iEmberCost = 0;
		bool locksEmberSocket = false;
		float fMaximumRange = 0.f;
		LostArk::Shared::SKILL_TARGET_INTENT_KIND eTargetIntent =
			LostArk::Shared::SKILL_TARGET_INTENT_KIND::AIM_POINT;
		float fTargetMaximumRange = 0.f;
		bool requiresWalkableTarget = false;
		LostArk::Shared::PLAYER_SKILL_KIND eSkillKind =
			LostArk::Shared::PLAYER_SKILL_KIND::ACTIVE;
		LostArk::Shared::PLAYER_STANCE_ID eRequiredStance =
			LostArk::Shared::PLAYER_STANCE_ID::NONE;
		LostArk::Shared::PLAYER_STANCE_ID eSetsStance =
			LostArk::Shared::PLAYER_STANCE_ID::NONE;
		std::vector<PLAYER_COMBO_STAGE> ComboStages;
		std::vector<ROOT_MOTION_SAMPLE> RootMotion;
		std::vector<PLAYER_SKILL_HIT> Hits;
		std::vector<PLAYER_SKILL_PROJECTILE> Projectiles;
		std::vector<PLAYER_CANCEL_WINDOW> SkillCancelWindows;
		std::vector<PLAYER_CANCEL_WINDOW> MoveCancelWindows;
		std::vector<PLAYER_CANCEL_WINDOW> DodgeCancelWindows;
	};

	// 기존 갑옷 plate 정의다. iPlateIndex는 게시 순서와 Client 부위 표시를 연결한다.
	// iDurability는 타격 횟수가 아닌 내구도 양이며 기존 경로는 GROGGY에서 소비한다.
	// typed Parts가 있으면 별도의 damage condition으로 판정한 결과를 기존 plate에 반영한다.
	// valtan boss의 armor에 대한 정보를 가지는 구조체이다.
	struct BOSS_ARMOR_PLATE
	{
		std::uint32_t iPlateIndex = 0;
		std::uint32_t iDurability = 0;
		std::uint32_t iDefense = 0;
	};

	enum class BOSS_PHASE_POLICY_KIND : std::uint8_t
	{
		// HP 비율로 phase를 정하는 정책이다. 기준 percent는 BOSS_PHASE_POLICY의 값으로 저장한다.
		HEALTH_PERCENT_THRESHOLD,
		// 저작 stage의 SET_GAMEPLAY_PHASE 명령으로 phase를 전환한다. profile에 같은 pattern/stage 연결을 중복 저장하지 않는다.
		AUTHORED_PATTERN_EVENT
	};

	struct BOSS_PHASE_POLICY final
	{
		BOSS_PHASE_POLICY_KIND eKind =
			BOSS_PHASE_POLICY_KIND::HEALTH_PERCENT_THRESHOLD;
		std::uint32_t iThresholdPercent = 0;
	};

	// archetype ID로 찾는 보스 기본 수치다. entity 생성 시 복사할 HP/방어/이동/부위 설정이며 현재 전투 상태는 아니다.
	struct BOSS_RUNTIME_PROFILE
	{
		std::string strArchetypeId;
		std::string strEncounterId;
		std::uint32_t iMaximumHp = 0;
		// 피해 산정에 사용할 별도 기준 HP다. 0이면 실제 최대 HP를 기준으로 삼는다.
		std::uint32_t iDamageReferenceHp = 0;
		std::uint32_t Get_DamageReferenceHp() const noexcept
		{
			return iDamageReferenceHp ? iDamageReferenceHp : iMaximumHp;
		}
		std::uint32_t iMaximumHealthBars = 0;
		// 이 보스의 피해 profile을 수치로 바꿀 때 사용하는 공격력이다.
		std::uint32_t iAttackPower = 0;
		// Server가 대상의 중심 거리로 판정할 때 원형 몸체의 가장자리까지 포함하기 위한 반지름이다.
		float fCollisionRadius = 0.f;
		float fEngageDistance = 0.f;
		float fMoveSpeed = 0.f;
		BOSS_PHASE_POLICY PhasePolicy;
		// 기존 plate가 없는 보스는 이 목록의 방어 기여가 없다. typed 부위/보호막/무적 등 다른 정책까지 없다는 뜻은 아니다.
		std::vector<BOSS_ARMOR_PLATE> ArmorPlates;
	};

	enum class BOSS_PATTERN_SELECTION
	{
		NORMAL,
		HEALTH_BAR,
		// 일반 가중치나 체력줄 자동 선택 후보에서 제외하는 저작 패턴이다.
		// 명시적 ID 요청/저작 후속 연결 등 승인된 경로가 실행하며 이 값만으로 실행을 시작하지 않는다.
		AUDITION_ONLY
	};

	// 일반 패턴 후보가 요구하는 갑옷 상태다. ANY는 상태를 제한하지 않고 plate가 없는 상태는 STRIPPED 쪽으로 판정한다.
	enum class BOSS_PATTERN_ARMOR_REQUIREMENT
	{
		ANY,
		ARMORED,
		STRIPPED
	};

	// 패턴 후보의 phase 조건이다. phase를 계산하거나 변경하는 책임은 boss profile의 정책과 stage 명령에 있다.
	enum class BOSS_PATTERN_PHASE_REQUIREMENT
	{
		ANY,
		PHASE_ONE,
		PHASE_TWO
	};

	enum class BOSS_PATTERN_HIT_SHAPE
	{
		NONE,
		CIRCLE,
		RING,
		CONE,
		BOX,
		CROSS,
		SIX_DIRECTIONS
	};

	enum class BOSS_COMBAT_OBJECT_KIND : std::uint8_t
	{
		FIXED_AREA,
		MISSILE
	};

	enum class BOSS_COMBAT_OBJECT_ORIGIN_POLICY : std::uint8_t
	{
		BOSS_POSITION,
		LOCKED_TARGET_UNTIL_FIRST_PULSE,
		// 생성 시 승인한 살아 있는 플레이어마다 대상을 잠근 오브젝트를 만든다. 생성 이후 참가자 변화가 기존 생성 개수를 바꾸지는 않는다.
		LOCKED_TARGET_PER_ALIVE_PLAYER
	};

	enum class BOSS_COMBAT_OBJECT_DIRECTION_POLICY : std::uint8_t
	{
		NONE,
		PATTERN_FACING_AT_SPAWN,
		RADIAL_INWARD,
		// 방사형 생성의 각 순번이 다음 생성 슬롯을 향하게 한다. 마지막 순번의 다음 슬롯은 0이다.
		NEXT_RADIAL_SLOT
	};

	enum class BOSS_COMBAT_OBJECT_HIT_TRIGGER : std::uint8_t
	{
		CONTACT,
		TIMED
	};

	// 독립 보스 전투 오브젝트의 접촉/예약 타격 정의다. 반복 소비와 현재 위치는 CCombatObjectRuntime이 따로 소유한다.
	struct BOSS_COMBAT_OBJECT_HIT final
	{
		std::string strHitId;
		BOSS_COMBAT_OBJECT_HIT_TRIGGER eTrigger =
			BOSS_COMBAT_OBJECT_HIT_TRIGGER::TIMED;
		std::uint32_t iAtMs = 0;
		std::uint32_t iRepeatCount = 1;
		std::uint32_t iRepeatIntervalMs = 0;
		BOSS_PATTERN_HIT_SHAPE eHitShape = BOSS_PATTERN_HIT_SHAPE::NONE;
		float fHitOuterRadius = 0.f;
		float fHitInnerRadius = 0.f;
		float fHitAngleDegrees = 0.f;
		float fHitLength = 0.f;
		float fHitHalfWidth = 0.f;
		std::string strDamageProfileId;
		float fPushRangeM = 0.f;
		std::uint32_t iPushMs = 0;
		bool bKnockdown = false;
		std::uint32_t iDownMs = 0;
	};

	// 전투 오브젝트의 같은 수명 시계에서 발생하는 표시 pulse다. 도형이나 피해 profile은 가지지 않는다.
	// GameRoom이 stable event ID를 복제하고 Client가 실제 연출을 찾는다.
	// 오브젝트 수명 중 게시된 Client visual ID를 알릴 시점이다. 이 행만으로 HP 피해를 적용하지 않는다.
	struct BOSS_COMBAT_OBJECT_PRESENTATION_PULSE final
	{
		std::string strPresentationEventId;
		std::uint32_t iAtMs = 0;
	};

	// 소유 보스의 지정 action 타격 도형과 연쇄 오브젝트를 연결한다. CCombatObjectRuntime::Apply_OwnerHits가 지연을 계산해 활성화한다.
	// 다음 보스 패턴을 고르는 분기가 아니라 오브젝트의 경고/예약 타격 시점을 여는 정의다.
	struct BOSS_COMBAT_OBJECT_OWNER_HIT_CHAIN final
	{
		std::string strTriggerActionId;
		std::string strArmedPresentationEventId;
		std::uint32_t iDelayMs = 0u;
	};

	// 전투 오브젝트 archetype 정의다. 공유 Catalog가 소유하고 각 실행 인스턴스는 생성 시 필요한 값과 진행 상태를 갖는다.
	struct BOSS_COMBAT_OBJECT_DEFINITION final
	{
		std::string strEncounterId;
		std::string strCombatObjectArchetypeId;
		std::string strClientVisualId;
		std::string strOwnerPatternId;
		std::string strOwnerStageActionId;
		BOSS_COMBAT_OBJECT_KIND eKind = BOSS_COMBAT_OBJECT_KIND::FIXED_AREA;
		BOSS_COMBAT_OBJECT_ORIGIN_POLICY eOriginPolicy =
			BOSS_COMBAT_OBJECT_ORIGIN_POLICY::BOSS_POSITION;
		BOSS_COMBAT_OBJECT_DIRECTION_POLICY eDirectionPolicy =
			BOSS_COMBAT_OBJECT_DIRECTION_POLICY::NONE;
		float fOffsetForwardM = 0.f;
		float fOffsetRightM = 0.f;
		float fSpeedMps = 0.f;
		float fMaximumDistanceM = 0.f;
		std::uint32_t iMovementStartDelayMs = 0u;
		bool bExpireOnDistanceEnd = true;
		float fCoverRadiusM = 0.f;
		std::uint32_t iLifeMs = 0;
		std::uint32_t iExpectedEventCount = 0;
		BOSS_COMBAT_OBJECT_OWNER_HIT_CHAIN OwnerHitChain;
		std::vector<BOSS_COMBAT_OBJECT_HIT> Hits;
		std::vector<LostArk::Shared::ATTACK_HIT_TEMPLATE> AttackTemplates;
		std::vector<BOSS_COMBAT_OBJECT_PRESENTATION_PULSE> PresentationPulses;
	};

	enum class BOSS_PATTERN_STAGE_KIND
	{
		WINDUP,
		ACTIVE,
		RECOVERY,
		// 무력화 노출 단계다. 기존 갑옷 내구도 경로가 이 상태를 요구하며 typed 부위 조건은 별도다.
		// 복제 action 상태는 PATTERN_ACTIVE로 연결한다.
		GROGGY,
		// 부위 파괴에 대한 반응 stage다. 복제 action 상태는 PATTERN_RECOVERY로 연결한다.
		PART_BREAK
	};

	// 일반 시간 순서 진행에서 건너뛰고 사건 분기로 진입할 stage 종류인지 판별한다.
	// 실제 벽 충돌/부위 파괴 결과와 다음 action ID의 연결은 Branches가 소유한다.
	// Brain의 순차 진행이 건너뛸 GROGGY/PART_BREAK 단계를 식별한다.
	inline bool Is_EventEnteredStage(const BOSS_PATTERN_STAGE_KIND kind)
	{
		return BOSS_PATTERN_STAGE_KIND::GROGGY == kind ||
			BOSS_PATTERN_STAGE_KIND::PART_BREAK == kind;
	}
	enum class BOSS_PATTERN_CATEGORY : std::uint8_t
	{
		NORMAL,
		IMPORTANT,
		MECHANIC
	};

	enum class BOSS_PATTERN_TARGET_POLICY : std::uint8_t
	{
		NONE,
		NEAREST_EACH_TICK,
		LOCK_NEAREST_ON_START,
		LOCK_RANDOM_ALIVE_ON_START,
		// 패턴 시작 전 보스 방향의 뒤쪽에서 승인 대상을 하나 고른다. 후보가 없으면 일반 생존 대상 풀로 돌아간다.
		LOCK_RANDOM_ALIVE_BEHIND_ON_START
	};

	enum class BOSS_PATTERN_AIM_POLICY : std::uint8_t
	{
		NONE,
		TRACK_TARGET_EACH_TICK,
		LOCK_FACING_ON_START,
		FACE_MOTION_ANCHOR
	};

	enum class BOSS_PATTERN_STAGE_OUTCOME : std::uint8_t
	{
		TIMEOUT,
		COUNTER_HIT,
		STAGGER_BROKEN,
		HEALTH_DAMAGE_THRESHOLD_REACHED,
		WALL_CONTACT,
		PART_DESTROYED,
		PROP_DESTROYED,
		SUMMON_DEAD,
		ALL_PLAYERS_GRABBED,
		ANY_PLAYER_GRABBED,
		NAVIGATION_BLOCKED
	};

	enum class BOSS_PATTERN_BOSS_RESPONSE_KIND : std::uint8_t
	{
		NONE,
		// 방어/무적/보호막 처리 후 실제 감소한 HP를 누적한다. 현재 occurrence의 기준 도달 결과는 한 번만 발행한다.
		ACCUMULATED_HEALTH_DAMAGE
	};

	enum class BOSS_PATTERN_COUNTER_PROXY_KIND : std::uint8_t
	{
		NONE,
		BOSS_LOCAL_CIRCLE,
		// 공격 시전 원점이 보스의 닫힌 전방 반평면에 있는지 검사한다. 저작 각은 180도이며 반지름 제한을 두지 않는다.
		BOSS_FORWARD_ARC
	};

	// stage 결과와 후속 stage/실패/완료를 연결한다. 같은 결과의 실제 소비는 Brain의 분기 선택 순서를 따른다.
	struct BOSS_PATTERN_STAGE_BRANCH final
	{
		BOSS_PATTERN_STAGE_OUTCOME eOutcome =
			BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT;
		// 같은 패턴의 다음 action ID다. vector index가 아니며 cross-pattern 목표와 동시에 사용하지 않는다.
		std::string strNextActionId;
		// 같은 encounter의 후속 패턴 ID다. 로더가 대상 없는 AUDITION_ONLY 연결을 검증한다.
		// Brain은 현재 종료를 먼저 기록하고 다음 tick에 고정한 정의 세대의 후속을 시작한다.
		std::string strNextPatternId;
	};

	enum class BOSS_PATTERN_STAGE_ACTION_TRIGGER : std::uint8_t
	{
		ENTER,
		EXIT
	};

	enum class BOSS_PATTERN_STAGE_ACTION_KIND : std::uint8_t
	{
		SET_BOSS_FLAG,
		SET_STAGGER_GAUGE,
		SET_SHIELD,
		SET_PLAYER_BIND,
		SET_PLAYER_SILENCE,
		SET_GAMEPLAY_PHASE,
		SPAWN_COMBAT_OBJECT,
		SPAWN_COMBAT_OBJECT_VOLLEY,
		// GameRoom이 기존 대상 선택 경로에서 살아 있는 대상을 다시 잠근다. 현재 pattern 실행을 새로 만들지는 않는다.
		RETARGET_RANDOM_ALIVE,
		// 이 보스에 부착된 플레이어를 해제한다. 해제 모드를 구분하여 단순 해제와 날리기 입력이 섞이지 않게 한다.
		RELEASE_GRABBED_PLAYERS,
		DAMAGE_GRABBED_PLAYERS,
		EXECUTE_GRABBED_PLAYERS,
		RETURN_TO_ARENA_CENTER,
		// FinishPattern이 이번 종료에 예약한 추적 대기만 소모한다. 순서 커서와 Debug 완료 대기는 별도 상태다.
		SUPPRESS_INTER_STEP_PURSUIT
	};

	enum class BOSS_GRABBED_RELEASE_MODE : std::uint8_t
	{
		NONE,
		HOLD,
		OPPOSITE_KNOCKBACK,
		ARENA_EJECTION
	};

	enum class BOSS_COMBAT_OBJECT_VOLLEY_POLICY : std::uint8_t
	{
		NONE,
		PER_ALIVE_PLAYER,
		// 현재 Server 보스 자세를 중심으로 방사형 생성한다. 저작 각도는 보스 yaw에 상대적이다.
		BOSS_RELATIVE,
		// 방 입장 때 보관한 보스 spawn 위치를 아레나 중심으로 쓴다. 각도는 월드 기준이므로 현재 보스 방향을 더하지 않는다.
		ARENA_CENTER
	};

	enum class BOSS_COMBAT_OBJECT_LAYOUT_KIND : std::uint8_t
	{
		SINGLE,
		RADIAL
	};

	enum class BOSS_COMBAT_OBJECT_ARENA_ANCHOR_POLICY : std::uint8_t
	{
		NONE,
		// 방 입장 시 navigation에 투영한 spawn 위치다. 패턴 실행 중 움직이는 현재 보스 위치와 구분한다.
		BOSS_SPAWN_POSITION
	};

	// 한 번의 stage action이 생성할 묶음 배치다. 원점/방향 정책과 발사 간격은 실제 오브젝트 생성 경로가 해석한다.
	struct BOSS_COMBAT_OBJECT_VOLLEY final
	{
		BOSS_COMBAT_OBJECT_VOLLEY_POLICY ePolicy =
			BOSS_COMBAT_OBJECT_VOLLEY_POLICY::NONE;
		std::uint32_t iCountPerResolvedTarget = 1u;
		BOSS_COMBAT_OBJECT_LAYOUT_KIND eLayout =
			BOSS_COMBAT_OBJECT_LAYOUT_KIND::SINGLE;
		float fRadiusM = 0.f;
		float fStartAngleDegrees = 0.f;
		float fAngleStepDegrees = 0.f;
		bool bAllowOverlap = false;
		std::uint32_t iMaximumTotalObjects = 1u;
		// stage 시계로 예약하는 전체 wave 수다. 첫 offset이 0이면 첫 wave를 stage ENTER에서 생성한다.
		std::uint32_t iSpawnCount = 1u;
		std::uint32_t iFirstSpawnOffsetMs = 0u;
		std::uint32_t iSpawnIntervalMs = 0u;
		std::uint32_t iArenaRandomCount = 0u;
		float fArenaRandomRadiusM = 0.f;
		float fArenaHeightToleranceM = 0.f;
		BOSS_COMBAT_OBJECT_ARENA_ANCHOR_POLICY eArenaAnchorPolicy =
			BOSS_COMBAT_OBJECT_ARENA_ANCHOR_POLICY::NONE;
	};

	// stage ENTER 또는 EXIT에서 GameRoom에 요청할 동작이다. 보스 Brain의 단계 진행과 플레이어/prop 변경 책임을 나눈다.
	struct BOSS_PATTERN_STAGE_ACTION final
	{
		BOSS_PATTERN_STAGE_ACTION_TRIGGER eTrigger =
			BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER;
		BOSS_PATTERN_STAGE_ACTION_KIND eKind =
			BOSS_PATTERN_STAGE_ACTION_KIND::SET_BOSS_FLAG;
		std::string strTargetId;
		std::uint32_t iValue = 0;
		std::uint32_t iDurationMs = 0;
		BOSS_GRABBED_RELEASE_MODE eReleaseMode =
			BOSS_GRABBED_RELEASE_MODE::NONE;
		float fReleaseSpeedMps = 0.f;
		// ARENA_EJECTION에서 보스 방향에 더하는 해제 각도다. 0은 기본 후방 해제이며 HOLD/OPPOSITE_KNOCKBACK은 0을 유지한다.
		float fReleaseYawOffsetDegrees = 0.f;
		BOSS_COMBAT_OBJECT_VOLLEY Volley;
	};

	enum class BOSS_PATTERN_PLAYER_RESPONSE : std::uint8_t
	{
		DAMAGE,
		CAPTURE
	};

	enum class BOSS_PATTERN_HIT_ANCHOR_KIND : std::uint8_t
	{
		BOSS_CURRENT,
		STAGE_ORIGIN
	};

	enum class BOSS_PATTERN_HIT_ACTIVATION_KIND : std::uint8_t
	{
		PULSE_SCHEDULE,
		ACTIVE_WINDOW
	};

	enum class BOSS_PATTERN_PART_DAMAGE_POLICY : std::uint8_t
	{
		NORMAL,
		// 부위 공격력이 양수이면 현재 저작 damage condition에 맞는 첫 부위를 파괴하는 정책이다.
		DESTROY_FIRST_ELIGIBLE
	};

	enum class BOSS_PATTERN_STAGE_MOTION_KIND : std::uint8_t
	{
		NONE,
		FORWARD,
		TO_ARENA_CENTER,
		PORTAL_TARGET_RUSH,
		PORTAL_CROSS_ARENA
	};

	struct BOSS_PATTERN_STAGE_MOTION final
	{
		BOSS_PATTERN_STAGE_MOTION_KIND eKind =
			BOSS_PATTERN_STAGE_MOTION_KIND::NONE;
		std::uint32_t iRetargetDelayMs = 0u;
		float fSpeedMps = 0.f;
		float fDistance = 0.f;
		std::uint32_t iCornerIndex = 0u;
		float fHalfExtentsX = 0.f;
		float fHalfExtentsZ = 0.f;
		// stage의 누적 루트모션 표본이다. 값이 있으면 해당 이동 경로에서 고정 거리 보간보다 표본을 우선 사용한다.
		// 계산한 이동은 GameRoom의 navigation/충돌 검사를 거치므로 표본의 위치를 무조건 확정하지 않는다.
		std::vector<ROOT_MOTION_SAMPLE> RootMotion;
	};

	// Kouku의 판정 종류다. 종료 시 판정, 매 tick 접촉, 보스 단위 무력화, 플레이어 입력 등 종류에 따라 처리 시점이 다르다.
	// 종류 값만으로 결과를 적용하지 않고 LogicWindow의 시간/영역/결과 목록을 함께 소비한다.
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

	struct BOSS_LOGIC_CONTACT_MOTION final
	{
		std::string strTargetWorldOccurrenceId;
		std::string strMotionInstanceId;
	};

	enum class BOSS_LOGIC_PUSH_DIRECTION : std::uint8_t
	{
		AWAY_FROM_BOSS,
		BOSS_FORWARD,
		AWAY_FROM_CONTACT
	};

	// Kouku 판정의 성공/실패/시간 만료 결과 목록 요소다. eKind에 해당하는 값만 결과 적용 경로가 사용한다.
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

	// 영역의 기준 자세를 고르는 정책이다. WORLD와 보스 현재/등장/판정창 시작 또는 별도 captureStart 기준을 구분한다.
	enum class BOSS_LOGIC_REGION_ANCHOR : std::uint8_t
	{
		WORLD,
		BOSS_CURRENT,
		BOSS_SPAWN,
		BOSS_START
	};
	// Resolve_LogicRegionTransform이 읽는 track 내부 ms 표본이다. offset은 m, Y/W는 yaw 전용 quaternion 성분이다.
	// offset/scale은 선형 보간하고 회전은 quaternion의 짧은 경로를 따른다. 가시성은 해당 구간의 왼쪽 key 값을 쓴다.
	// GripPosition이 있으면 월드 위치를 직접 보간하며 보스 기준 변환을 다시 적용하지 않는다.
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
	// 판정 영역이 World Object의 저작 이동을 따라가도록 baseline과 key 목록을 보관한다. vector가 표본을 소유한다.
	// bSmoothStep은 보간 비율에 적용한다. baseline scale은 영역 중심/크기에 적용하며 key translation 자체에는 곱하지 않는다.
	// BOSS_CURRENT는 pattern 경과 ms에서 시작/delay를 빼고, 다른 기준은 시작을 30Hz tick으로 올림한 뒤 경과를 계산한다.
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
	// LogicRuntime의 영역 판정 입력이다. 중심/길이는 m, yaw/부채꼴 각도는 degree이며 도형 flag에 맞는 크기만 소비한다.
	// WORLD는 보스 변환을 더하지 않는다. BOSS_CURRENT는 현재 보스 XZ/yaw, BOSS_SPAWN은 등장 XZ 가산만 사용한다.
	// 두 보스 기준의 직접 Y 가산은 cylinder 또는 linearMotion일 때만 수행한다.
	// BOSS_START는 지정 시각에 도달한 호출의 현재 보스 자세와 지정 시각의 track 표본을 조합해 WORLD 도형으로 고정하고 Y도 보완한다.
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

	// 접촉을 확인할 World occurrence의 stable ID와 월드 XZ 원이다. 표시 instance ID와 개별 occurrence ID를 함께 구분한다.
	struct BOSS_LOGIC_CONTACT_TARGET final
	{
		std::string strWorldOccurrenceId;
		std::string strWorldInstanceId;
		float fWorldX = 0.f,
			fWorldZ = 0.f,
			fRadiusM = 0.f;
	};

	// Workbench에서 게시한 Kouku 판정창 하나다. iStartMs/iDurationMs는 pattern 시계 기준이며 eKind가 사용할 값을 결정한다.
	// 시간·영역·대상과 결과 정의를 담고, 현재 완료 여부/중복 접촉/고정 영역은 LogicRuntime의 ledger가 소유한다.
	// 게시자가 실제 설치 본 pose에서 만든 손 위치다. 시간은 패턴 시작 이후 ms,
	// 위치는 body preScale을 포함한 보스 로컬 m이며 보스 월드 root는 포함하지 않는다.
	struct BOSS_CAPTURE_HAND_KEY final
	{
		std::uint32_t iTimeMs = 0u;
		std::array<float, 3u> LocalPosition{};
	};

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
		// CAPTURE 창이 소유한다. 참조한 Hold 전체 수명을 덮어 피해창 종료 뒤에도 손을 따라간다.
		std::vector<BOSS_CAPTURE_HAND_KEY> CaptureHandKeys;
	};

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

	enum class ALBION_AIRBORNE_PHASE : std::uint8_t
	{
		NONE,
		JUMP,
		SELECT_PLAYER,
		APPEAR_PLAYER,
		DISAPPEAR,
		CENTER,
		SLAM
	};

	inline constexpr std::size_t KOUKU_SUMMON_MAX_PATTERN_SPAWNS = 16u;
	enum class BOSS_PATTERN_SUMMON_ANCHOR_KIND : std::uint8_t
	{
		BOSS,
		MAP
	};

	struct BOSS_PATTERN_SUMMON_PATTERN_SPAWN final
	{
		std::string strSpawnId;
		std::string strPatternId;
		BOSS_PATTERN_SUMMON_ANCHOR_KIND eAnchorKind = BOSS_PATTERN_SUMMON_ANCHOR_KIND::BOSS;
		// MAP은 절대 월드 위치/방향, BOSS는 소유자 기준 위치/방향으로 해석한다.
		std::array<float, 3u> PositionOffset{};
		float fYawOffsetDegrees = 0.f;
	};

	// Showtime 무작위 발사 묶음의 판정 template과 Client visual ID다. 표시 수명과 실제 명중 여부는 별도 상태로 처리한다.
	struct BOSS_SHOWTIME_RANDOM_VOLLEY final
	{
		std::vector<LostArk::Shared::ATTACK_HIT_TEMPLATE> Hits;
		std::string strClientVisualId;
		std::uint32_t iLifetimeMs = 0u;
	};

	// 고정 폭탄 하나를 소유자의 최종 추적 부채꼴과 한 번 대조하기 위한 정의다.
	struct BOSS_SHOWTIME_BOMB_RESOLUTION final
	{
		std::uint32_t iBodyStartMs = 0u;
		std::string strBodyVisualId,
			strExplosionVisualId;
		std::uint32_t iExplosionLifetimeMs = 0u;
		std::array<float, 3u> BodyPosition{};
		BOSS_LOGIC_REGION FanRegion;
	};

	// 빙고 폭탄의 표식/낙하/폭발 일정을 ms로 저장한다. 로더가 아래 범위와 30Hz 변환 뒤의 슬롯 상한을 함께 검사한다.
	struct BOSS_BINGO_BOARD_SETTINGS final
	{
		bool bWindowOnly = false;
		std::uint32_t iFirstBombDelayMs = 30000u,
			iBombIntervalMs = 20000u;
		std::uint32_t iBombMarkMs = 6000u,
			iBombDropDelayMs = 2000u,
			iBombFuseMs = 4000u;
		std::uint32_t iInitialMarkedCells = 2u;
		// 값만 검사하고 상태를 변경하지 않는다. ms 합과 올림한 tick 합을 모두 검사하여 네 interval 범위에 들어오는지 확인한다.
		[[nodiscard]] bool Is_Valid() const noexcept
		{
			const auto ticks = [](const std::uint32_t ms)
			{
				return (std::uint64_t(ms) * 30u + 999u) / 1000u;
			};
			return iFirstBombDelayMs <= 600000u &&
				iBombIntervalMs && iBombIntervalMs <= 600000u &&
				iBombMarkMs && iBombMarkMs <= 600000u &&
				iBombDropDelayMs && iBombDropDelayMs <= 600000u &&
				iBombFuseMs >= 250u && iBombFuseMs <= 80000u &&
				iInitialMarkedCells <= 25u &&
				std::uint64_t(iBombMarkMs) + iBombDropDelayMs + iBombFuseMs <= std::uint64_t(iBombIntervalMs) * 4u &&
				ticks(iBombMarkMs) + ticks(iBombDropDelayMs) + ticks(iBombFuseMs) <= ticks(iBombIntervalMs) * 4u;
		}
	};

	// pattern 시계의 iStartMs에 GameRoom이 실행할 기믹 요청이다. eKind에 따라 HUD/이동/소환/빙고 입력을 선택한다.
	// 이 정의는 요청 데이터이며 현재 대상 선택, 생성 entity, 예약 횟수는 방의 실행 상태가 관리한다.
	// 더 못 쪼갤까? 왜 이렇게 구성이 되어야만 할까?
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

	// World sequence가 플레이어를 받칠 수 있는 tick 구간과 월드 XZ 원/높이 정의다. 표시 event ID와는 다른 서버 판정 입력이다.
	struct BOSS_PATTERN_WORLD_SUPPORT_WINDOW final
	{
		std::uint32_t iStartOffsetTicks = 0u;
		std::uint32_t iEndOffsetTicks = 0u;
		float fCenterX = 0.f;
		float fCenterZ = 0.f;
		float fHeightY = 0.f;
		float fRadiusM = 0.f;
	};

	// World sequence의 명시 배치 변환이다. 위치는 월드 m, 회전은 degree, scale은 축별 배율이다.
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

	// World 표시와 함께 Server가 만들 전투 몸체의 HP/중심/반지름이다. 실제 entity ID와 현재 HP는 실행 인스턴스에 둔다.
	struct BOSS_PATTERN_WORLD_COMBAT_BODY final
	{
		std::uint32_t iMaximumHp = 0u;
		float fCenterX = 0.f,
			fCenterY = 0.f,
			fCenterZ = 0.f;
		float fRadiusM = 0.f;
	};

	// pattern 시계에 맞춰 Client World 표시를 요청하고 선택적으로 Server 받침/전투 몸체 정의를 연결한다.
	// SupportWindows의 offset tick은 이 cue의 시작 tick 기준이다. 받침 위치는 position offset과 선택적인 spawn-anchor 차이를 더한다.
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

	// pattern 일정에 연결한 Client scene profile ID와 유지/전환 ms다. Server는 렌더링 자원을 직접 해석하지 않는다.
	struct BOSS_PATTERN_SCENE_PROFILE final
	{
		std::string strProfileId;
		std::uint32_t iStartMs = 0u;
		std::uint32_t iDurationMs = 0u;
		std::uint32_t iBlendMs = 0u;
	};

	// encounter의 광기 정책이다. 최대값 0은 기본값을 유지하고 hold ms는 광대 형태 유지 시간을 정한다.
	struct BOSS_ENCOUNTER_MADNESS_POLICY final
	{
		std::uint32_t iMaximum = 0u;
		std::uint32_t iClownHoldMs = 0u;
		std::uint32_t iDamageGainPercent = 100u;
		std::uint32_t iBallGainPercent = 10u,
			iBallMultiplierPercent = 200u;
		float fBallRadiusM = 2.f;
		std::uint32_t iDollGainPercent = 10u,
			iDollMultiplierPercent = 200u;
		float fDollRadiusM = 4.f;
		std::uint32_t iSpecialIntervalMs = 1000u;
	};

	// Brain이 순서/분기로 진입할 단계 정의다. 타격·이동·카운터·결과 연결을 보관하고 경과 시간/소비 횟수는 보스 실행 상태에 둔다.
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

	enum class BOSS_PATTERN_MOTION_KIND : std::uint8_t
	{
		NONE,
		LEAP_TO_ANCHOR,
		// 저장해 둔 마지막 대상 위치가 있으면 그 위치를 착지점으로 사용하고 없으면 저작 anchor로 돌아간다.
		LEAP_TO_TARGET
	};

	// Server가 계산할 패턴 단위 도약의 기준 위치와 시간 정의다. stage 일반 이동과 별도로 소비한다.
	// 착지 X/Y/Z는 월드 m, 꼭대기 높이는 기준 Y에 더하는 m다.
	struct BOSS_PATTERN_MOTION
	{
		BOSS_PATTERN_MOTION_KIND eKind = BOSS_PATTERN_MOTION_KIND::NONE;
		std::string strAnchorId;
		float fLandingX = 0.f;
		float fLandingY = 0.f;
		float fLandingZ = 0.f;
		float fApexHeight = 0.f;
		// 하강 이동을 맡은 stage index다. 이륙과 이 stage 사이 구간은 꼭대기 높이를 유지한다.
		std::uint32_t iTravelStageIndex = 1u;
		// 이륙/이동 stage 안에서 도약이 실제로 진행되는 ms 구간이다. 예고/회복 시간을 도약 길이와 구분한다.
		std::uint32_t iTakeoffStartMs = 0u;
		std::uint32_t iTakeoffEndMs = 0u;
		std::uint32_t iTravelStartMs = 0u;
		std::uint32_t iTravelEndMs = 0u;
		bool bMoveToAnchorBeforeTakeoff = false;
	};

	enum class BOSS_PATTERN_ROTATION_SELECTION_MODE : std::uint8_t
	{
		// Candidates를 선택 풀로 사용하며 후보별 weight/enabled를 유지한다.
		// 거리/phase/갑옷/재사용 대기/연속 사용 제한은 pattern 정의와 함께 검사한다.
		WEIGHTED_POOL,
		// 저작 순서의 각 항목을 한 번씩 도입한 뒤 encounter의 가중치 선택으로 연결한다.
		ORDERED_INTRO_THEN_WEIGHTED,
		// 해당 체력 구간에서 PatternIds의 순번을 반복한다. 같은 ID가 여러 번 있어도 순번을 합치지 않는다.
		ORDERED_LOOP
	};

	// 관리되는 가중 pool의 패턴 ID와 선택 가중치다. 실행 당시 거리/재사용 등의 조건은 Brain이 추가 검사한다.
	struct BOSS_PATTERN_ROTATION_CANDIDATE final
	{
		std::string strPatternId;
		std::uint32_t iSelectionWeight = 0u;
		bool bEnabled = false;
	};

	// 특정 gameplay phase와 체력줄 구간에 속하는 rotation의 선택 창이다. 실제 포함 비교는 Catalog 조회 함수가 수행한다.
	struct BOSS_PATTERN_ROTATION_WINDOW final
	{
		std::string strWindowId;
		std::string strSelectionSetId;
		std::uint32_t iGameplayPhase = 0u;
		std::uint32_t iFromHealthBar = 0u;
		std::uint32_t iToHealthBar = 0u;
		std::uint32_t iExpectedCandidateCount = 0u;

		// window ID의 존재만 검사한다. phase/체력줄/후보 수가 올바른지는 bootstrap 로더의 별도 검증 책임이다.
		[[nodiscard]] bool Is_Defined() const noexcept
		{
			return !strWindowId.empty();
		}
	};

	// 체력 기믹 사이의 일반 패턴 선택 구간이다. 체력줄이 감소하므로 높은 from에서 낮은 to 방향으로 해석한다.
	struct BOSS_PATTERN_ROTATION_DEFINITION
	{
		std::string strEncounterId;
		std::string strRotationId;
		BOSS_PATTERN_ROTATION_SELECTION_MODE eSelectionMode =
			BOSS_PATTERN_ROTATION_SELECTION_MODE::ORDERED_INTRO_THEN_WEIGHTED;
		std::uint32_t iFromHealthBar = 0;
		std::uint32_t iToHealthBar = 0;
		std::uint32_t iExpectedStepCount = 0;
		// WEIGHTED_POOL은 Window/Candidates를, 순서형 모드는 PatternIds를 사용한다. 로더가 모드별 필드 조합을 검사한다.
		BOSS_PATTERN_ROTATION_WINDOW Window;
		std::vector<BOSS_PATTERN_ROTATION_CANDIDATE> Candidates;
		std::vector<std::string> PatternIds;
	};

	enum class BOSS_PATTERN_SEQUENCE_MODE : std::uint8_t
	{
		// 저작 항목을 순서대로 한 번 소비하고 마지막 뒤에는 대기한다. 일반 가중치 선택으로 자동 복귀하지 않는다.
		ORDERED_ONCE_THEN_IDLE,
		// sequence는 Play All용으로 유지하고 자동 전투는 체력 기믹과 rotation을 사용한다.
		HEALTH_BAR_ROTATIONS
	};

	// 순서가 있는 패턴 ID와 사이 추적 시간을 소유한다. eMode가 자동 순서 실행 또는 체력줄/rotation 사용을 구분한다.
	struct BOSS_PATTERN_SEQUENCE_DEFINITION final
	{
		std::string strEncounterId;
		std::string strSequenceId;
		// 자동 시작에서 기존 intro 전에 선택할 수 있는 입장 연출 패턴 ID다.
		std::string strEntranceCinematicPatternId;
		BOSS_PATTERN_SEQUENCE_MODE eMode =
			BOSS_PATTERN_SEQUENCE_MODE::ORDERED_ONCE_THEN_IDLE;
		std::uint32_t iInterStepPursuitMs = 0u;
		std::uint32_t iInterStepPursuitTicks = 0u;
		std::uint32_t iExpectedStepCount = 0u;
		std::vector<std::string> PatternIds;
		// 마지막을 제외한 패턴 occurrence 뒤의 추적 대기다. 로더는 이전 단일 기간 형식도 항목별 목록으로 확장한다.
		std::vector<std::uint32_t> TransitionPursuitMs;
		std::vector<std::uint32_t> TransitionPursuitTicks;
	};

	enum class BOSS_PATTERN_FINALE_KIND : std::uint8_t
	{
		NONE,
		GHOST_PORTAL_LOOP
	};

	// 패턴이 시작할 피날레 보조 보스/portal 생성 정책이다. 동시에 살아 있는 개체와 다음 생성 tick은 실행 상태에서 관리한다.
	struct BOSS_PATTERN_FINALE final
	{
		BOSS_PATTERN_FINALE_KIND eKind = BOSS_PATTERN_FINALE_KIND::NONE;
		std::string strGhostArchetypeId;
		std::vector<std::string> GhostPatternIds;
		float fSpawnHalfExtentsX = 0.f;
		float fSpawnHalfExtentsZ = 0.f;
		std::uint32_t iMaximumActiveGhosts = 0u;
		// 0이면 보조 유령이 사라진 다음 fixed tick의 기존 보충 경로를 사용한다.
		std::uint32_t iAuxiliarySpawnIntervalMs = 0u;
		std::uint32_t iPortalSpawnIntervalMs = 7900u;
	};

	// 묶음 실행에서 누가 어떤 패턴을 언제 시작할지 지정한다. member ID는 같은 패턴의 서로 다른 실행을 구분한다.
	struct BOSS_PATTERN_BUNDLE_MEMBER final
	{
		std::string strMemberId;
		std::string strPatternId;
		std::string strTargetBossPlacementId;
		std::uint32_t iStartOffsetMs = 0u;
	};
	// 관문/encounter에 속한 병렬 패턴 묶음 정의다. GameRoom의 member scheduler가 목록과 시작 offset을 소비한다.
	struct BOSS_PATTERN_BUNDLE_DEFINITION final
	{
		std::string strBundleId;
		std::string strEncounterId;
		std::string strGateId;
		std::vector<BOSS_PATTERN_BUNDLE_MEMBER> Members;
	};


	// 관문 진행 목록의 한 항목이다. bBundle에 따라 target ID를 패턴 또는 묶음으로 해석하고 완료 뒤 대기 시간을 적용한다.
	struct KOUKU_RAID_FLOW_ENTRY final
	{
		std::string strEntryId,
			strTargetId;
		bool bBundle = false;
		std::uint32_t iWaitAfterMs = 0u;
	};
	// 관문 진입/종료 연출의 플레이어 도착 위치다. 슬롯과 pattern 시계의 시작 ms를 보관한다.
	struct KOUKU_RAID_ARRIVAL final
	{
		std::string strOccurrenceId;
		bool bClear = false;
		std::uint32_t iSlot = 0u,
			iStartMs = 0u;
		std::array<float, 3u> Position{};
	};
	// 연속된 항목 범위를 체력줄 조건까지 반복한다. bTransitionAtGroupEnd는 조건을 만족해도 그룹 끝까지 진행할지 정한다.
	struct KOUKU_RAID_FLOW_GROUP final
	{
		std::string strGroupId;
		std::uint32_t iFirstEntry = 0u,
			iLastEntry = 0u;
		std::optional<std::uint32_t> RepeatUntilHealthBars;
		bool bTransitionAtGroupEnd = false;
	};
	// GameRoom의 Kouku 관문 진행이 읽는 전체 일정이다. 아래 조회 함수는 정의를 바꾸지 않고 다음 index만 계산한다.
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

	// BossMotion 경로의 pattern 기준 ms/월드 위치 표본이다. KoukuBrain이 인접 표본을 보간해 실제 보스 위치를 갱신한다.
	struct BOSS_PATTERN_BOSS_MOTION_KEY
	{
		std::uint32_t iTimeMs = 0u;
		std::array<float, 3u> Position{};
	};

	// Kouku 보스의 pattern 기준 시작/끝 ms와 월드 이동 정의다. key가 있는 경우 중간 경로도 함께 보관한다.
	struct BOSS_PATTERN_BOSS_MOTION
	{
		std::uint32_t iStartMs = 0u;
		std::uint32_t iEndMs = 1u;
		std::array<float, 3u> StartPosition{};
		std::array<float, 3u> EndPosition{};
		float fYawDegrees = 0.f;
		std::vector<BOSS_PATTERN_BOSS_MOTION_KEY> Keys;
	};

    // 보존된 Parent가 member scheduler에 예약하는 자식 occurrence다. ID/시작/기간을 유지하고 animation stage로 강제 합치지 않는다.
    struct BOSS_PATTERN_PARENT_CHILD final
    {
        std::string strOccurrenceId;
        std::string strPatternId;
        std::uint32_t iStartMs = 0u;
        std::uint32_t iDurationMs = 0u;
    };

	// encounter별 보스 패턴 하나의 읽기 전용 정의다. 선택 조건, stage 목록과 Kouku의 독립 저작 lane을 한 stable pattern ID로 연결한다.
	// 현재 선택/단계/타격 소비/기믹 결과는 각 Brain과 GameRoom 실행 상태에 두므로 이 정의와 수명을 구분한다.
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

	enum class BOSS_PART_DAMAGE_CONDITION : std::uint8_t
	{
		ALWAYS,
		GROGGY_ONLY
	};

	// 보스의 typed 부위 정의다. entity의 현재 내구도/파괴 상태와 분리하며 GROGGY 제한은 각 부위의 eDamageCondition으로 정한다.
	struct BOSS_PART_DEFINITION final
	{
		std::string strBossArchetypeId;
		std::string strPartId;
		std::uint32_t iStateMask = 0;
		std::uint32_t iMaximumDurability = 0;
		std::uint32_t iDamageReductionPercent = 0;
		BOSS_PART_DAMAGE_CONDITION eDamageCondition =
			BOSS_PART_DAMAGE_CONDITION::ALWAYS;
	};

	enum class VALTAN_TIMELINE_ARENA_STATE : std::uint8_t
	{
		FRESH,
		ORDINARY_WALLS_GONE,
		ALL_WALLS_GONE,
		FLOOR84_GONE,
		FLOOR84_AND_30_GONE,
		// Debug 프리셋이다. 제품 진행에서는 84줄 이후 30줄 지형 상태로 넘어간다.
		FLOOR30_GONE
	};

	enum class VALTAN_TIMELINE_PROP_STATE : std::uint8_t
	{
		HIDDEN,
		FOUR_PILLARS_INTACT
	};

	enum class VALTAN_TIMELINE_ENTRY_TYPE : std::uint8_t
	{
		MECHANIC,
		NORMAL
	};

	// Timeline의 한 행에서 실행할 pattern ID와 반복 횟수다. 실행은 기존 ValtanBrain의 패턴 경로를 사용한다.
	struct VALTAN_TIMELINE_PATTERN_ACTION final
	{
		std::string strPatternId;
		std::uint32_t iRepeat = 0;
	};

	// Debug에서 선택할 시간 순서 occurrence다. 한 행에 여러 패턴을 묶어도 각 action은 기존 CValtanBrain 경로로 실행한다.
	struct VALTAN_TIMELINE_ROW final
	{
		std::string strRowId;
		// strRowId에서 FNV-1a32로 만든 wire 명령 키다. 화면 순번과 분리하여 저작 순서를 바꿔도 같은 ID를 유지한다.
		std::uint32_t iCommandId = 0;
		std::uint32_t iOrdinal = 0;
		std::uint32_t iSectionHealthBar = 0;
		VALTAN_TIMELINE_ENTRY_TYPE eEntryType =
			VALTAN_TIMELINE_ENTRY_TYPE::NORMAL;
		VALTAN_TIMELINE_ARENA_STATE eArenaState =
			VALTAN_TIMELINE_ARENA_STATE::FRESH;
		VALTAN_TIMELINE_PROP_STATE ePropState =
			VALTAN_TIMELINE_PROP_STATE::HIDDEN;
		std::vector<VALTAN_TIMELINE_PATTERN_ACTION> PatternActions;
	};

	// 편집 도구/Server가 공유할 발탄 발생 순서 목록이다. 행의 wire command ID와 표시 순번은 서로 다른 값이다.
	struct VALTAN_TIMELINE_DEFINITION final
	{
		std::string strEncounterId;
		std::string strTimelineId;
		std::vector<VALTAN_TIMELINE_ROW> Rows;
	};

	// 직업별 기본 수치와 자원 정책이다. 승인된 직업을 적용할 때 실행 player 상태를 초기화하고 이후 증감은 그 상태에 기록한다.
	struct PLAYER_RUNTIME_PROFILE
	{
		LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass =
			LostArk::Shared::CHARACTER_CLASS_ID::END;
		std::uint32_t iMaximumHp = 0;
		std::uint32_t iMaximumResource = 0;
		// 스킬 시전 중이 아닐 때 fixed tick 누산기로 회복하는 초당 자원량이다. 실제 벽시계 시간을 직접 읽는 필드는 아니다.
		std::uint32_t iResourceRegenPerSecond = 0;
		std::uint32_t iAttackPower = 0;
		std::uint32_t iDefense = 0;
		// 치명타 확률 percent와 치명타 피해 배율 percent다. 확률 0이면 치명타 추첨이 발생하지 않는다.
		std::uint32_t iCriticalChancePercent = 0;
		std::uint32_t iCriticalDamagePercent = 200;
		float fMoveSpeed = 0.f;
		// 방어 stance를 유지하는 동안 기본 이동 속도에 곱하는 값이다. 1이면 속도를 바꾸지 않는다.
		float fDefenseStanceMoveSpeedScale = 1.f;
		// 직업 identity의 최대값이다. 0이면 해당 profile의 identity 갱신 경로는 즉시 반환한다.
		std::uint32_t iMaximumIdentity = 0;
		std::uint32_t iIdentityRegenPerSecond = 0;
		// 게이지를 사용하는 stance를 유지할 때 초당 소모한다. 0에 도달하면 기본 stance로 복귀한다.
		std::uint32_t iIdentityDrainPerSecond = 0;
		// stance 변경 스킬의 처리 시점에 충분한 게이지가 있을 때 한 번 차감한다. 부족하면 현재 구현은 차감 없이 전환한다.
		std::uint32_t iIdentityStanceSwitchCost = 0;
		// 자동 회복 중 최대값에 도달하면 0으로 순환하는 게이지다.
		// 현재 Update_Identity는 그 지점에서 게이지만 되돌리며 별도 한 바퀴 완료 효과를 발행하지 않는다.
		std::uint32_t iIdentityCyclic = 0;
		LostArk::Shared::PLAYER_STANCE_ID eDefaultStance =
			LostArk::Shared::PLAYER_STANCE_ID::NONE;
	};

	// 가디언 나이트의 identity/불씨 정책이다. 기본 stance의 명중은 게이지를 채우고 용 stance는 설정한 기간으로 소모한다.
	// 최대 소켓 수에서 잠긴 수를 빼 불씨 용량을 정하고, 스킬이 소비한 불씨당 추가 피해 percent를 계산한다.
	struct GUARDIAN_EMBER_PROFILE
	{
		std::uint32_t iGaugeGainPerHit = 0;
		std::uint32_t iDragonDurationMs = 0;
		std::uint32_t iMaximumSockets = 0;
		std::uint32_t iDamageBonusPercentPerOrb = 0;
	};

	// Server가 게시 bootstrap을 읽어 소유하는 gameplay 정의 집합이다. GameRoom/Brain/PlayerSkillSystem은 stable ID로 조회한다.
	// Catalog 요소를 가리키는 조회 포인터/참조는 이 객체의 컨테이너를 차용한다. 성공한 재로드/교체/파괴 뒤에는 재사용하면 안 된다.
	// 내부 잠금은 없으며 호출자는 활성 catalog의 수명과 교체 시점을 관리한다. 실패 사유 문자열은 조회 정의와 별도로 갱신될 수 있다.
	class CGameplayCatalog final
	{
	public:
		// 관문 공통 무력화 최대치를 값으로 돌려준다. 현재 보스의 남은 무력화량은 entity 실행 상태에 있다.
		[[nodiscard]] std::uint32_t Get_RaidStaggerMaximum() const noexcept
		{
			return m_iRaidStaggerMaximum;
		}
		// Server 시작 로드가 게시 Gameplay.bootstrap과 선택적 numeric receipt를 임시 Catalog에 검증하고 성공 시 교체한다.
		// false이면 기존 정의를 유지하되 Get_Status의 실패 설명은 바뀔 수 있다.
		bool Load();
		// 숫자 편집 목록과 별도 revision을 만든다. 값 수정과 연출/동작의 identity를 분리하는 Balance Test 입력이다.
		bool Build_NumericBalanceSnapshot(
			std::vector<LostArk::Shared::BALANCE_NUMERIC_ENTRY>& entries,
			LostArk::Shared::GameplayDataRevision& numericRevision,
			std::string& status) const;
		// active의 현재 숫자와 changes의 before 값을 대조한 뒤 허용 수치만 바꾼 bootstrap을 재검증한다.
		// 성공할 때 이 Catalog와 bootstrapBytes를 함께 갱신하며 choreography/presentation 연결은 유지한다.
		bool Load_NumericBalancePatch(const CGameplayCatalog& active,
			const std::vector<LostArk::Shared::BALANCE_NUMERIC_CHANGE>& changes,
			std::string& bootstrapBytes);
		// choreography의 구조 위에 numeric에서 같은 키의 값을 대조하여 숫자 patch 경로로 재검증한다.
		// 차이가 없으면 choreography를 복사하며, 숫자 Catalog 전체로 구조 정의를 교체하는 함수는 아니다.
		bool Load_NumericBalanceValues(const CGameplayCatalog& choreography,
			const CGameplayCatalog& numeric);
		// 로드 때 보존한 Kouku/그 외 원문 행으로 header와 count를 다시 붙인다. 현재 struct를 범용 직렬화하는 함수는 아니다.
		[[nodiscard]] std::string Export_BootstrapBytes() const;
		// 전체 bootstrap과 숫자 칸을 마스킹한 행의 digest를 계산한다. 결과는 receipt 검증에 쓰이며 Catalog 정의를 바꾸지 않는다.
		bool Build_NumericBalanceReceiptHashes(
			LostArk::Shared::GameplayDataRevision& bootstrapHash,
			LostArk::Shared::GameplayDataRevision& nonNumericRowsHash,
			std::string& status) const;

		// 게시된 Kouku 행과 activeGameplay의 그 외 행을 합쳐 검증한다. 현재 process의 다른 보스/플레이어 정의를 디스크 값으로 덮지 않는다.
		bool Load_PublishedKoukuProduct(const CGameplayCatalog& activeGameplay);
		// Debug audition 한 번에 사용할 메모리 후보를 검증한다. 이 함수가 디스크에 publish하지는 않는다.
		bool Load_DraftKoukuProduct(const CGameplayCatalog& activeGameplay,
			const std::string& rows, const LostArk::Shared::GameplayDataRevision& expectedRowsRevision);
		// 정규 절대 경로의 Gameplay.bootstrap 후보를 읽고 예상 content hash를 대조한다.
		// 승인되면 ActiveRevision은 child 파일 hash 대신 인자로 검증한 parent revision을 사용한다.
		bool Load_FromBootstrap(
			const std::filesystem::path& bootstrapPath,
			const LostArk::Shared::GameplayDataRevision& expectedBootstrapRevision,
			const LostArk::Shared::GameplayDataRevision& parentRevision);
		// Balance와 스킬 초기화가 순회할 정의 map의 const 참조다. 순서는 unordered_map 계약을 따른다.
		[[nodiscard]] const auto& Get_Skills() const noexcept
		{
			return m_Skills;
		}
		// 포인터를 반환하는 Find_*는 stable ID 조회이며 없으면 nullptr다. 입력으로 실행 entity의 현재 상태를 바꾸지 않는다.
		const PLAYER_SKILL_DEFINITION* Find_Skill(
			LostArk::Shared::SKILL_ID skillId) const;
		const BOSS_RUNTIME_PROFILE* Find_Boss(
			const std::string& archetypeId) const;
		const std::vector<BOSS_PART_DEFINITION>* Find_BossParts(
			const std::string& archetypeId) const;
		const std::vector<BOSS_PATTERN_DEFINITION>* Find_BossPatterns(
			const std::string& encounterId) const;
		const BOSS_COMBAT_OBJECT_DEFINITION* Find_BossCombatObject(
			const std::string& combatObjectArchetypeId) const;
		const VALTAN_TIMELINE_DEFINITION* Find_ValtanTimeline(
			const std::string& encounterId) const;
		// 표시 순번이 아닌 command ID로 행을 찾는다. command ID 0 또는 없는 encounter/행이면 nullptr다.
		const VALTAN_TIMELINE_ROW* Find_ValtanTimelineRow(
			const std::string& encounterId,
			std::uint32_t commandId) const;
		// rotation 조회의 phase 필터는 Window가 정의된 관리 구간에만 적용한다.
		// from 줄은 포함하고 to 줄은 제외하는 구간을 찾는다. 해당 구간이 없으면 nullptr다.
		const BOSS_PATTERN_ROTATION_DEFINITION* Find_BossPatternRotation(
			const std::string& encounterId,
			std::uint32_t gameplayPhase,
			std::uint32_t healthBar) const;
		// encounter의 sequence 정의를 조회한다. Brain이 eMode를 보고 순서 전용 실행과 체력 rotation 실행을 구분한다.
		const BOSS_PATTERN_SEQUENCE_DEFINITION* Find_BossPatternSequence(
			const std::string& encounterId) const;
		const KOUKU_RAID_GATE_DEFINITION* Find_KoukuRaidGate(const std::string& gateId) const;
		const BOSS_PATTERN_BUNDLE_DEFINITION* Find_BossPatternBundle(const std::string& bundleId) const;
		// Kouku 저작 composition revision 숫자다. bootstrap content hash와 별개이며 다른 encounter 조회는 0이다.
		[[nodiscard]] std::uint32_t Find_KoukuSaydonProductSourceRevision(
			const std::string& encounterId) const noexcept;
		// non-Kouku 행의 revision이 유효하고 같은지 비교한다. Kouku reload가 기존 플레이어/보스 balance와 Valtan 기준을 유지하는 데 쓴다.
		[[nodiscard]] bool Has_SameNonKoukuGameplay(
			const CGameplayCatalog& other) const noexcept;
		// 해당 encounter의 광기 정책이다. 없으면 nullptr를 반환한다.
		[[nodiscard]] const BOSS_ENCOUNTER_MADNESS_POLICY* Find_KoukuMadnessPolicy(
			const std::string& encounterId) const noexcept;
		// encounter의 intro pattern ID 참조를 돌려준다. 없는 경우 정적인 빈 문자열 참조다.
		const std::string& Find_IntroPatternId(
			const std::string& encounterId) const;
		const PLAYER_RUNTIME_PROFILE* Find_Player(
			LostArk::Shared::CHARACTER_CLASS_ID characterClass) const;
		// 해당 class의 불씨 정책을 찾는다. 등록된 정책이 없으면 nullptr다.
		const GUARDIAN_EMBER_PROFILE* Find_EmberProfile(
			LostArk::Shared::CHARACTER_CLASS_ID characterClass) const;
		// 공격력에 적용할 피해 비율 percent다. 알 수 없는 profile ID는 0을 반환한다.
		std::uint32_t Find_DamageRatePercent(
			const std::string& damageProfileId) const;

		// 피해 수치 정의다. coefficient/addend가 있으면 공격력 * basis-point 계수 / 10000 + 고정 가산을 사용한다.
		// 두 값이 모두 0이면 기존 iRatePercent 계산으로 돌아간다.
		struct DAMAGE_PROFILE final
		{
			std::uint32_t iRatePercent = 0;
			std::uint32_t iAttackCoefficientBp = 0;
			std::uint32_t iDamageAddend = 0;
			// 플레이어 피해 추첨에서 기준값 주변에 적용할 변동 percent다. 0이면 그 변동을 적용하지 않는다.
			std::uint32_t iDamageSpreadPercent = 0;
			// 스킬 전체 시전의 보스 체력줄 기준 피해량이다. 개별 sub-hit 분배는 스킬 소비자가 맡는다.
			std::uint32_t iBossHealthBarDamage = 0;
		};
		[[nodiscard]] const DAMAGE_PROFILE* Find_DamageProfile(
			const std::string& damageProfileId) const;

		// 스킬이 buff를 줄 대상을 구분한다. 지속 시간/비율과 대상 정의를 게시 데이터에서 함께 읽는다.
		enum class SKILL_BUFF_TARGET : std::uint8_t
		{
			SELF,
			ALLY,
			ENEMY
		};
		struct SKILL_BUFF_DEFINITION final
		{
			std::uint32_t iSkillId = 0;
			std::uint32_t iBuffId = 0;
			SKILL_BUFF_TARGET eTarget = SKILL_BUFF_TARGET::SELF;
			std::uint32_t iDurationMs = 0;
			std::int32_t iDamageDealtPercent = 0;
			std::int32_t iDamageTakenPercent = 0;
			std::int32_t iAttackSpeedPercent = 0;
			// 시전자 class profile의 최대 HP 중 이 percent를 보호막 양으로 계산한다.
			std::uint32_t iShieldPercentOfMaxHp = 0;
			// 일반 MONSTER의 동작을 멈출 ms다. CServerBuffRuntime은 보스에 같은 stun을 적용하지 않는다.
			std::uint32_t iStunMs = 0;
			// 일반 치명 피해에서 HP 1을 남기고 부여할 무적 ms다. 공용 피격 경로가 발동 시 사망 방지 buff를 소비한다.
			std::uint32_t iDeathDenyInvulnerableMs = 0;
		};
		[[nodiscard]] const std::vector<SKILL_BUFF_DEFINITION>* Find_SkillBuffs(
			std::uint32_t skillId) const;
		[[nodiscard]] const SKILL_BUFF_DEFINITION* Find_SkillBuff(
			std::uint32_t buffId) const;
		// 계수/가산값 중 하나라도 있으면 uint64 중간값으로 계산하고 1~UINT32_MAX로 제한한다. 둘 다 0이면 아래 percent 경로를 쓴다.
		// 치명타/편차/방어/대상 HP 변경은 이 함수 밖의 피격 경로 책임이다.
		static std::uint32_t Resolve_Damage(
			std::uint32_t attackPower, const DAMAGE_PROFILE& profile);

		// 공격력 * percent를 정수 피해로 바꾼다. 둘 중 하나가 0이면 0이며 그 외는 1~UINT32_MAX로 제한한다.
		static std::uint32_t Resolve_Damage(
			std::uint32_t attackPower, std::uint32_t damageRatePercent);
		// 프로젝트 공통 방어식이다. rawDamage * 100 / (100 + defense)를 사용하며 양수 피해는 최소 1이다.
		// 특정 타격이 방어를 우회할지는 호출 경로의 정책이 정한다.
		static std::uint32_t Apply_Defense(
			std::uint32_t rawDamage, std::uint32_t defense);

		// 검증한 활성 gameplay 세대다. 명시 candidate 로드에서는 child bootstrap hash와 별도인 parent revision을 노출한다.
		[[nodiscard]] const LostArk::Shared::GameplayDataRevision&
			Get_ActiveRevision() const noexcept
		{
			return m_ActiveRevision;
		}
		// bootstrap이 승인한 Client 표시 세대의 content identity다. Server가 Client asset을 로드하는 경로는 아니다.
		[[nodiscard]] const LostArk::Shared::GameplayDataRevision&
			Get_ValtanPresentationGenerationId() const noexcept
		{
			return m_ValtanPresentationGenerationId;
		}
		// 최근 로드/검증 상태 문자열의 차용 참조다. 다음 검증 호출 때 내용이 바뀔 수 있다.
		const std::string& Get_Status() const
		{
			return m_strStatus;
		}

	private:
		// 크기 제한과 읽기 완료를 검사한 뒤 공용 byte parser로 전달한다. 외부 path 선택과 revision 입력은 공개 Load 경로가 정한다.
		bool Load_BootstrapPath(
			const std::filesystem::path& bootstrapPath,
			const LostArk::Shared::GameplayDataRevision* expectedBootstrapRevision,
			const LostArk::Shared::GameplayDataRevision* parentRevision);
		// 모든 로드 경로가 사용하는 parse/참조 검증/commit 경계다. 주요 정의 map은 실패 시 rollback한다.
		// 현재 raid 무력화 최대치 멤버는 이 rollback에 포함되지 않으므로 모든 멤버 보존을 전제로 활성 객체에 직접 재로드하면 안 된다.
		// 공개 Load와 ServerApp의 후보 로드처럼 별도 Catalog에서 검증한다. 아래 부분 parser의 출력도 실패 시 일부 작성될 수 있다.
		bool Load_BootstrapBytes(
			const std::string& bootstrapBytes,
			const LostArk::Shared::GameplayDataRevision* expectedBootstrapRevision,
			const LostArk::Shared::GameplayDataRevision* parentRevision);
		// 스킬/stage가 같은 packed RootMotion 형식과 시간 순서 검사를 공유한다. 실패 이유는 m_strStatus에 남긴다.
		bool Parse_RootMotionSamples(
			std::string_view packed,
			std::uint32_t sampleCount,
			std::uint32_t limitMs,
			std::vector<ROOT_MOTION_SAMPLE>& outSamples);
		// 정렬된 타격 목록과 반복 합계 상한을 검사한다. 출력은 먼저 비우며 실패 시 이미 읽은 일부 타격이 남을 수 있다.
		bool Parse_SkillHits(
			std::string_view packed,
			std::uint32_t hitCount,
			std::uint32_t limitMs,
			std::vector<PLAYER_SKILL_HIT>& outHits);
		// 시작:종료 ms 구간을 순서와 겹침 조건에 맞춰 읽는다. 파서 출력은 실패 시 부분 작성될 수 있다.
		bool Parse_CancelWindows(
			std::string_view packed,
			std::uint32_t windowCount,
			std::uint32_t limitMs,
			std::vector<PLAYER_CANCEL_WINDOW>& outWindows);
		// areaType/range/angle/width/height/offset/inner/maxTargets/pushMs/pushRange 형식을 공통 해석한다.
		bool Parse_HitShapeExtent(
			const std::string_view* fields,
			PLAYER_SKILL_HIT& outHit);
		// SKILLPROJ/SKILLSTAGEPROJ의 index/생성 시각/이동/타격 목록을 읽는다.
		// 행 순서로 추가하며 잘못된 생성 시각 순서나 index를 거부한다.
		bool Parse_SkillProjectile(
			const std::vector<std::string_view>& fields,
			std::size_t firstField,
			std::uint32_t limitMs,
			std::vector<PLAYER_SKILL_PROJECTILE>& outProjectiles);

		// 각 map이 정의의 값과 하위 vector/string을 소유한다. stable ID 조회를 위해 해시 map을 쓰며 실행 순서는 별도 vector 정의로 보관한다.
		std::unordered_map<LostArk::Shared::SKILL_ID, PLAYER_SKILL_DEFINITION>
			m_Skills;
		std::unordered_map<std::string, BOSS_RUNTIME_PROFILE> m_Bosses;
		std::unordered_map<std::string, std::vector<BOSS_PART_DEFINITION>>
			m_BossParts;
		std::unordered_map<std::string, std::vector<BOSS_PATTERN_DEFINITION>>
			m_BossPatterns;
		std::unordered_map<std::string, BOSS_COMBAT_OBJECT_DEFINITION>
			m_BossCombatObjects;
		std::unordered_map<std::string, VALTAN_TIMELINE_DEFINITION>
			m_ValtanTimelines;
		std::unordered_map<std::string, std::string> m_IntroPatternIdByEncounter;
		std::unordered_map<std::string,
			std::vector<BOSS_PATTERN_ROTATION_DEFINITION>>
			m_BossPatternRotations;
		std::unordered_map<std::string, BOSS_PATTERN_SEQUENCE_DEFINITION>
			m_BossPatternSequences;
		std::unordered_map<std::string, BOSS_PATTERN_BUNDLE_DEFINITION> m_BossPatternBundles;
		std::unordered_map<std::string, KOUKU_RAID_GATE_DEFINITION> m_KoukuRaidGates;
		std::uint32_t m_iKoukuSaydonProductSourceRevision = 0u;
		std::uint32_t m_iRaidStaggerMaximum = 0u;
		std::unordered_map<std::string, BOSS_ENCOUNTER_MADNESS_POLICY>
			m_KoukuMadnessPolicies;
		std::unordered_map<LostArk::Shared::CHARACTER_CLASS_ID,
			PLAYER_RUNTIME_PROFILE> m_Players;
		std::unordered_map<LostArk::Shared::CHARACTER_CLASS_ID,
			GUARDIAN_EMBER_PROFILE> m_EmberProfiles;
		std::unordered_map<std::string, std::uint32_t>
			m_DamageRatePercentByProfileId;
		std::unordered_map<std::string, DAMAGE_PROFILE> m_DamageProfileById;
		std::unordered_map<std::uint32_t, std::vector<SKILL_BUFF_DEFINITION>>
			m_SkillBuffsBySkillId;
		std::unordered_map<std::uint32_t, SKILL_BUFF_DEFINITION> m_SkillBuffById;
		// 전체 활성 세대, Kouku 외 내용의 digest, Client presentation 세대를 구분해 재로드/입장 계약에 사용한다.
		LostArk::Shared::GameplayDataRevision m_ActiveRevision{};
		LostArk::Shared::GameplayDataRevision m_NonKoukuGameplayRevision{};
		LostArk::Shared::GameplayDataRevision
			m_ValtanPresentationGenerationId{};
		// domain별 원문 행을 보존하여 Kouku 전용 재로드와 numeric patch가 무관한 domain을 그대로 유지하도록 한다.
		std::string m_KoukuBootstrapRows;
		std::string m_NonKoukuBootstrapRows;
		std::string m_strStatus;
	};

	// 공용 피격 경로가 기본 단축키 Q/W/E/R 중 counter power가 있는 스킬인지 확인한다. 이 함수 자체는 카운터 성공을 판정하지 않는다.
	inline bool Is_PrimaryCounterSkill(const PLAYER_SKILL_DEFINITION& skill)
	{
		return 0u != skill.iCounterPower &&
			("Q" == skill.strInputSlot || "W" == skill.strInputSlot ||
			 "E" == skill.strInputSlot || "R" == skill.strInputSlot);
	}

	// SPACE에 배치된 ACTIVE/HOLD를 회피 스킬로 분류한다. 이 함수는 분류만 하며 입력 취소 금지는 호출자가 적용한다.
	inline bool Is_DodgeSkill(const PLAYER_SKILL_DEFINITION& skill)
	{
		return "SPACE" == skill.strInputSlot &&
			(LostArk::Shared::PLAYER_SKILL_KIND::ACTIVE == skill.eSkillKind ||
				LostArk::Shared::PLAYER_SKILL_KIND::HOLD == skill.eSkillKind);
	}

	// 현재 action 또는 combo stage의 입력별 취소 구간을 조회한다. 시간 입력은 초이며 ms로 환산해 [시작, 종료)로 검사한다.
	// 목록이 없거나 경과 시간이 음수/NaN이면 false다. 실행 상태나 다음 스킬은 이 함수에서 변경하지 않는다.
	inline bool Is_InsideCancelWindow(
		const PLAYER_SKILL_DEFINITION& running,
		const std::uint32_t comboStage,
		const float actionElapsedSeconds,
		const PLAYER_CANCEL_INPUT input)
	{
		const std::size_t stageIndex =
			0u == comboStage ? 0u : comboStage - 1u;
		const bool hasStage =
			(LostArk::Shared::PLAYER_SKILL_KIND::COMBO == running.eSkillKind ||
				LostArk::Shared::PLAYER_SKILL_KIND::HOLD == running.eSkillKind ||
				LostArk::Shared::PLAYER_SKILL_KIND::COUNTER == running.eSkillKind) &&
			stageIndex < running.ComboStages.size();
		const auto select = [input](const auto& owner)
			-> const std::vector<PLAYER_CANCEL_WINDOW>&
		{
			switch (input)
			{
			case PLAYER_CANCEL_INPUT::MOVE:
				return owner.MoveCancelWindows;
			case PLAYER_CANCEL_INPUT::DODGE:
				return owner.DodgeCancelWindows;
			default:
				return owner.SkillCancelWindows;
			}
		};
		const std::vector<PLAYER_CANCEL_WINDOW>& windows = hasStage ?
			select(running.ComboStages[stageIndex]) : select(running);
		if (windows.empty() || !(actionElapsedSeconds >= 0.f))
			return false;
		const double elapsedMs =
			static_cast<double>(actionElapsedSeconds) * 1000.0;
		for (const PLAYER_CANCEL_WINDOW& window : windows)
		{
			if (elapsedMs >= static_cast<double>(window.iStartMs) &&
				elapsedMs < static_cast<double>(window.iEndMs))
			{
				return true;
			}
		}
		return false;
	}
}
```


## Server/Public/KoukuSaydonLogicRuntime.h 전체 코드

수정 대상: [KoukuSaydonLogicRuntime.h](C:/Users/tnest/Desktop/LostArk/Server/Public/KoukuSaydonLogicRuntime.h)

```cpp
#pragma once

#include "GameplayCatalog.h"
#include "ServerPlayer.h"
#include "ServerWorldEntity.h"
#include "ServerNavigation.h"

#include <array>
#include <cstdint>
#include <map>
#include <set>
#include <tuple>
#include <utility>
#include <string>
#include <vector>

namespace LostArk::Server
{
	class CServerCollisionSystem;
	/* 판정 window의 플레이어별 응답이다. NONE은 연속 window에서는 미응답, 종료 tick 판정에서는 아직 판정 전이다. */
	enum class KOUKUSAYDON_LOGIC_ANSWER : std::uint8_t
	{
		NONE,
		SUCCESS,
		FAIL,
		TIMEOUT
	};

	/* Build가 패턴 시작 tick을 기준으로 만든 window 진행 상태다. Update가 저작 pattern과 이 상태를 함께 소비한다. */
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

	/* 예정된 Server tick을 기다리는 저작 world sequence·scene profile cue다. */
	struct KOUKUSAYDON_LOGIC_CUE_STATE final
	{
		std::uint32_t iIndex = 0u;
		std::uint32_t iStartTick = 0u;
		bool bStarted = false;
	};

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

	/* 룸이 패턴 발생별로 소유하는 Logic 진행 기록이다. 패턴 시작 때 만들고 종료 때 버린다. */
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
        // Build가 이번 실행의 조커 카드 위치 차이(월드 m)를 한 번 결정한다. ID·회전·scale은 저작 정의를 유지한다.
        // 최초 World PLAY와 OBJECT_CONTACT가 같이 읽고 ledger 종료 시 함께 폐기한다. Room tick 스레드만 접근한다.
        std::map<std::string, std::array<float, 3u>> WorldPositionOffsets;
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

    struct KOUKUSAYDON_LOGIC_CAPTURE_REQUEST final
    {
        LostArk::Shared::NET_ENTITY_ID iPlayerNetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
        LostArk::Shared::PLAYER_ATTACHMENT_SLOT eAttachmentSlot = LostArk::Shared::PLAYER_ATTACHMENT_SLOT::NONE;
        std::uint32_t iHoldEndTick = 0u;
        std::uint32_t iWindowIndex = 0u;
        bool bHasHandLocalPosition = false;
        std::array<float, 3u> HandLocalPosition{};
    };

	/* Update가 룸에 요청하는 송신·후속 패턴·포획 결과다. 플레이어·보스 상태는 직접 갱신하지만 송신과 실행 목록 소유권은 룸에 둔다. */
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

	/* 고정 Server tick에서 저작 Logic의 RESULT를 판정·적용한다. */
	// 쿠크 패턴의 Logic window와 Server tick을 비교해 플레이어·보스 상태와 룸에 돌려줄 결과를 계산한다.
	// 발생별 진행 상태는 호출자가 넘기는 ledger에 보관하고, 송신·패턴 선택·stage 전환은 룸이 맡는다.
	class CKoukuSaydonLogicRuntime final
	{
	public:
        // Only physical contact hazards are offered to companion avoidance.
        [[nodiscard]] static float Predict_ContactRisk(const SERVER_WORLD_ENTITY& boss,
            const BOSS_PATTERN_DEFINITION& pattern, const KOUKUSAYDON_LOGIC_LEDGER& ledger,
            const SERVER_PLAYER& probe, std::uint32_t serverTick, std::uint32_t horizonMs);

		static constexpr std::uint32_t DANCE_POSE_COUNT = 4u;

		static void Assign_EncounterCard(SERVER_PLAYER& player,
			LostArk::Shared::NET_ENTITY_ID encounterOwnerId, std::uint32_t serverTick,
			std::uint8_t unavailableSymbols = 0u);

		static bool Can_EnterMarioEntry(const SERVER_PLAYER& player) noexcept;
		// MARIO_ENTER만 성공 결과로 가진 영역 진입 window 또는 PATTERN_COMPLETION_COUNT chain root를 찾는다.
		static bool Has_MarioEntry(const BOSS_PATTERN_DEFINITION& pattern) noexcept;
		// 영역이 있는 첫 Mario 진입 결과를 반환한다. chain root만으로 진입하는 패턴이면 nullptr다.
		static const BOSS_PATTERN_LOGIC_RESULT* Find_MarioEntryResult(const BOSS_PATTERN_DEFINITION& pattern) noexcept;
		// 자식 패턴 실행 중에도 룸이 보관한 root 영역으로 검사한다. honorWindowEnd가 true일 때만 저작 종료 시각에 portal을 닫는다.
		static bool Is_InsideMarioEntry(const BOSS_PATTERN_DEFINITION& root,
			const SERVER_WORLD_ENTITY& anchor, const SERVER_PLAYER& player,
			std::uint32_t elapsedTicks, bool honorWindowEnd = false, std::uint8_t* matchedStage = nullptr);
		// 접촉 판정과 같은 저작 영역 변환과 타원 부채꼴 경계를 사용한다.
		[[nodiscard]] static bool Is_PointInsideRegion(const BOSS_LOGIC_REGION& region,
			const SERVER_WORLD_ENTITY& boss, float worldX, float worldZ) noexcept;
		// 현재 권위 mechanic 구간을 읽어 snapshot의 gauge로 투영한다.
		static void Project_MechanicGauge(const SERVER_WORLD_ENTITY& boss,
			const BOSS_PATTERN_DEFINITION& pattern, const KOUKUSAYDON_LOGIC_LEDGER& ledger,
			std::uint32_t serverTick, LostArk::Shared::BOSS_COMBAT_SNAPSHOT& snapshot);
		static void Build(
			const BOSS_PATTERN_DEFINITION& pattern,
			const SERVER_WORLD_ENTITY& boss,
			std::uint32_t startTick,
			KOUKUSAYDON_LOGIC_LEDGER& outLedger);
		/* 추가 판정 없이 ledger를 비우고 보스 방어·결과 및 이 발생이 소유한 플레이어 bind를 정리한다. */
		static void Discard(
			KOUKUSAYDON_LOGIC_LEDGER& ledger,
			std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
			SERVER_WORLD_ENTITY* pBoss);
		// 게시된 손 key를 현재 패턴 시각에 선형 보간한다. 실패 시 outLocal은 보존한다.
		static bool Try_SampleCaptureHandLocalPosition(const BOSS_PATTERN_LOGIC_WINDOW& window,
			double patternElapsedMs, std::array<float, 3u>& outLocal) noexcept;
		// LogicRuntime의 접촉 판정 직전과 GameRoom의 잡기 commit 직후에만 호출한다.
		// 현재 보스 root/yaw를 한 번 적용하고 서버 실제 발 원점과 다음 tick용 local offset을 함께 갱신한다.
		static bool Apply_CaptureHandLocalPosition(SERVER_PLAYER& player, const SERVER_WORLD_ENTITY& boss,
			const std::array<float, 3u>& local) noexcept;
		static void Update(
			SERVER_WORLD_ENTITY& boss,
			const BOSS_PATTERN_DEFINITION& pattern,
			KOUKUSAYDON_LOGIC_LEDGER& ledger,
			std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
			const CGameplayCatalog& catalog,
			const BOSS_ENCOUNTER_MADNESS_POLICY* pMadnessPolicy,
			std::uint32_t serverTick,
			std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents,
			KOUKUSAYDON_LOGIC_OUTPUT& outOutput,
			const CServerNavigation* navigation = nullptr,
			const CServerCollisionSystem* collision = nullptr);
		/* 플레이어에게 복제한 slot 배치로 DANCE 입력을 해석한다. 열린 window의 첫 응답만 기록한다. */
		static bool Record_InteractionSlot(
			KOUKUSAYDON_LOGIC_LEDGER& ledger,
			const BOSS_PATTERN_DEFINITION& pattern,
			const SERVER_PLAYER& player,
			LostArk::Shared::INTERACTION_SLOT slot,
			std::string& outStatus);
		/* 플레이어 HUD·slot·광기 상한·광대 만료를 갱신한다. dance slot은 패턴 발생별 한 번 섞어 HUD와 판정이 같은 배치를 읽는다. */
		static void Update_PlayerModes(
			std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
			const KOUKUSAYDON_LOGIC_LEDGER* pActiveLedger,
			const BOSS_ENCOUNTER_MADNESS_POLICY* pMadnessPolicy,
			std::uint32_t serverTick);
		static bool Update_PlayerFear(SERVER_PLAYER& player, std::uint32_t serverTick);
		static void Apply_Result(
			SERVER_PLAYER& player,
			const BOSS_PATTERN_LOGIC_RESULT& result,
			const SERVER_WORLD_ENTITY& boss,
			const CGameplayCatalog& catalog,
			const BOSS_ENCOUNTER_MADNESS_POLICY* pMadnessPolicy,
			std::uint32_t serverTick,
			std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents,
			const std::array<float, 2u>* pContactCenter = nullptr, bool encounterWipe = true);
		/* 살아 있는 일반 플레이어를 광대로 바꾼다. durationMs가 0이면 encounter 유지 시간, 없으면 기본 시간을 쓴다. */
		static void Transform_ToClown(
			SERVER_PLAYER& player,
			const BOSS_ENCOUNTER_MADNESS_POLICY* pMadnessPolicy,
			std::uint32_t serverTick,
			std::uint32_t durationMs);
		/* 공격 원점 방향이 활성 방패 각도 안이면 true다. 호출자는 원래 피해 대신 반사를 처리한다. */
		[[nodiscard]] static bool Is_ShieldReflected(
			const SERVER_WORLD_ENTITY& boss,
			float sourceX,
			float sourceZ) noexcept;
		[[nodiscard]] static bool Has_ReachedTick(
			std::uint32_t serverTick, std::uint32_t targetTick) noexcept;
		[[nodiscard]] static std::uint32_t Add_Ticks(
			std::uint32_t tick, std::uint32_t ticks) noexcept;
		[[nodiscard]] static std::uint32_t Ticks_FromMs(std::uint32_t ms) noexcept;

	private:
		static void Capture_CardMazeEntryRoster(KOUKUSAYDON_LOGIC_LEDGER& ledger,
			const std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players);
		static void Reveal_CardMazeEntryPlayers(const KOUKUSAYDON_LOGIC_LEDGER& ledger,
			std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players);
		static bool Stage_CardMazeEntryPlayers(const BOSS_PATTERN_MECHANIC_TRIGGER& trigger,
			KOUKUSAYDON_LOGIC_LEDGER& ledger, std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
			const CServerNavigation* navigation, const CServerCollisionSystem* collision, std::string& outStatus);
		static bool Enter_CardMaze(const BOSS_PATTERN_MECHANIC_TRIGGER& trigger,
			KOUKUSAYDON_LOGIC_LEDGER& ledger, std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
			const CServerNavigation* navigation, const CServerCollisionSystem* collision, std::string& outStatus);
		static void Capture_StartRegions(const SERVER_WORLD_ENTITY& boss,
			const BOSS_PATTERN_LOGIC_WINDOW& window, KOUKUSAYDON_LOGIC_WINDOW_STATE& state,
			std::uint32_t patternElapsedTicks);
		static void Open_Window(
			SERVER_WORLD_ENTITY& boss,
			const BOSS_PATTERN_LOGIC_WINDOW& window,
			KOUKUSAYDON_LOGIC_WINDOW_STATE& state,
			const KOUKUSAYDON_LOGIC_LEDGER& ledger,
			std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players);
		static void Close_Window(
			SERVER_WORLD_ENTITY& boss,
			const BOSS_PATTERN_LOGIC_WINDOW& window,
			KOUKUSAYDON_LOGIC_WINDOW_STATE& state,
			std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players);
		static void Apply_Results(
			const std::vector<BOSS_PATTERN_LOGIC_RESULT>& results,
			KOUKUSAYDON_LOGIC_WINDOW_STATE& state,
			SERVER_PLAYER* pPlayer,
			std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
			const SERVER_WORLD_ENTITY& boss,
			const CGameplayCatalog& catalog,
			const BOSS_ENCOUNTER_MADNESS_POLICY* pMadnessPolicy,
			std::uint32_t serverTick,
			std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents,
			KOUKUSAYDON_LOGIC_OUTPUT& outOutput,
			const std::set<LostArk::Shared::PLAYER_ID>& invulnerablePlayers,
			const std::array<float, 2u>* pContactCenter = nullptr,
			bool spatialContact = false);
		static KOUKUSAYDON_LOGIC_ANSWER Judge_Roulette(
			const BOSS_PATTERN_LOGIC_WINDOW& window,
			const SERVER_WORLD_ENTITY& boss,
			const SERVER_PLAYER& player) noexcept;
		static KOUKUSAYDON_LOGIC_ANSWER Judge_Gaze(
			const BOSS_PATTERN_LOGIC_WINDOW& window,
			const SERVER_WORLD_ENTITY& boss,
			const SERVER_PLAYER& player) noexcept;
		static bool Is_Judgeable(const SERVER_PLAYER& player) noexcept;
		static bool Can_ReceiveSpatialContact(const SERVER_PLAYER& player) noexcept;
	};

	/* 룸이 소유하는 빙고 상태다. 폭탄·망치 단계와 보드 진행을 값으로 보관한다. */
	/* 25비트 white는 모든 해골 칸, red는 완성된 행·열의 영구 칸이다. 보상 소비 여부는 별도 Server line mask로 관리한다. */
	class CKoukuBingoRuntime final
	{
	public:
		void Reset() noexcept
		{
			m_iWhiteMask = 0u;
			m_iRedMask = 0u;
			m_iConsumedLineMask = 0u;
			m_Bombs = {};
			m_Hammer = {};
		}
		/* 보드 안의 지정 칸을 채우고 완성된 행·열을 승격한다. 보드 밖 비트는 무시한다. */
		void Fill(std::uint32_t cellMask) noexcept;
		/* red 칸은 유지하고 폭발 범위의 나머지 해골 칸을 반전한 뒤 행·열을 승격한다. white에는 red도 포함하며 대각선은 승격하지 않는다. */
		void Detonate(std::uint32_t cellMask) noexcept;
		[[nodiscard]] std::uint32_t Get_WhiteMask() const noexcept { return m_iWhiteMask; }
		[[nodiscard]] std::uint32_t Get_RedMask() const noexcept { return m_iRedMask; }
		[[nodiscard]] std::uint32_t Count_CompletedRowsAndColumns() const noexcept;
		[[nodiscard]] std::uint32_t Count_UnconsumedRowsAndColumns() const noexcept;
		/* 행·열의 고정 순서로 미소비 완성 줄을 count개 소비한다. count가 0이거나 줄이 부족하면 상태를 유지한다. */
		bool Consume_CompletedRowsAndColumns(std::uint32_t count) noexcept;
		/* 해당 위치의 보드 칸이 완성된 red 줄에 속하는지 검사한다. 전멸 공격의 안전 구역 판정에 사용한다. */
		[[nodiscard]] bool Is_Safe(float x, float z) const noexcept;

		/* 폭탄 slot은 운반자 ID와 예정 tick을 보관한다. 실제 플레이어 조회와 낙하 위치 결정은 룸이 담당한다. */
		struct BOMB final
		{
			LostArk::Shared::BINGO_BOMB_PHASE ePhase =
				LostArk::Shared::BINGO_BOMB_PHASE::NONE;
			LostArk::Shared::NET_ENTITY_ID iCarrierNetEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID;
			float fPositionX = 0.f;
			float fPositionZ = 0.f;
			std::uint32_t iDetonateTick = 0u;
			// 머리 표식이 사라진 뒤 지면 생성 전까지 slot을 예약하는 tick이다.
			std::uint32_t iPlantTick = 0u;
			std::uint32_t iMarkOrdinal = 0u;
			// 폭탄마다 시간을 캡처해 뒤의 보드 설정 변경이 대기 중인 낙하·폭발을 바꾸지 못하게 한다.
			std::uint32_t iDropDelayMs = LostArk::Shared::KOUKU_BINGO_BOMB_DROP_DELAY_MS;
			std::uint32_t iFuseMs = LostArk::Shared::KOUKU_BINGO_BOMB_FUSE_MS;
		};
		using BOMB_SLOTS = std::array<BOMB,
			static_cast<std::size_t>(LostArk::Shared::KOUKU_BINGO_MAX_BOMBS)>;

		/* 운반자 ID가 유효하고 빈 slot이 있으면 표식한다. 같은 운반자의 MARKED가 이미 있으면 거절해 표식 시간을 재시작하지 않는다. */
		bool Start_Bomb(LostArk::Shared::NET_ENTITY_ID carrier,
			std::uint32_t detonateTick, std::uint32_t markOrdinal = 0u,
			std::uint32_t dropDelayMs = LostArk::Shared::KOUKU_BINGO_BOMB_DROP_DELAY_MS,
			std::uint32_t fuseMs = LostArk::Shared::KOUKU_BINGO_BOMB_FUSE_MS) noexcept;
		/* MARKED 또는 낙하 예약 slot을 지정 위치에 설치하고 fuseTick을 폭발 tick으로 저장한다. */
		void Plant_Bomb(std::size_t slot, float x, float z,
			std::uint32_t fuseTick) noexcept;
		void Queue_BombPlant(std::size_t slot, float x, float z, std::uint32_t plantTick) noexcept;
		void Clear_Bomb(std::size_t slot) noexcept;
		[[nodiscard]] const BOMB_SLOTS& Get_Bombs() const noexcept { return m_Bombs; }

		/* 망치가 없을 때 유효한 anchor와 시간으로 시작한다. anchor 선택은 룸이 맡는다. */
		bool Start_Hammer(std::int32_t anchor, std::uint32_t startTick,
			std::uint32_t endTick) noexcept;
		/* 도달한 종료 tick에 망치를 한 단계 전진시키고 마지막 단계 뒤에는 비운다. 망치가 남아 있으면 true다. */
		bool Advance_Hammer(std::uint32_t tick) noexcept;
		[[nodiscard]] const LostArk::Shared::BINGO_HAMMER_SNAPSHOT&
			Get_Hammer() const noexcept { return m_Hammer; }

	private:
		/* Promotes every line that is now completely white. */
		void Promote_Lines() noexcept;

		std::uint32_t m_iWhiteMask = 0u;
		std::uint32_t m_iRedMask = 0u;
		std::uint32_t m_iConsumedLineMask = 0u;
		BOMB_SLOTS m_Bombs{};
		LostArk::Shared::BINGO_HAMMER_SNAPSHOT m_Hammer{};
	};

	// 미로의 역할·문양·처치 진행과 대기 참가자를 소유한다. entity 생성·피격·제거와 송신은 룸이 담당한다.
	class CKoukuCardMazeRuntime final
	{
	public:
		enum class PHASE : std::uint8_t
		{
			INACTIVE,
			HUNTING,
			COMPLETE
		};

		struct SPAWN_REQUEST final
		{
			LostArk::Shared::MECHANIC_CARD_SYMBOL eSuit = LostArk::Shared::MECHANIC_CARD_SYMBOL::NONE;
			float fPositionX = 0.f;
			float fPositionY = 0.f;
			float fPositionZ = 0.f;
			float fYawDegrees = 0.f;
		};

		/* What one landed hammer hit changed. */
		struct HIT_OUTCOME final
		{
			bool bStartMarch = false;
			bool bKillCounted = false;
			bool bHunterComplete = false;
			bool bAllComplete = false;
		};

		static constexpr std::uint8_t KILL_TARGET = 1u;
		static constexpr std::uint32_t TARGETS_PER_SUIT = 1u;
		/* 일반 망치의 피해 판정은 입력 후 30Hz 기준 30tick이다. Hammer_HitTickOffset은 interactionIndex 1에만 12tick을 적용한다. */
		static constexpr std::uint32_t HAMMER_HIT_TICK_OFFSET = 30u;
		static constexpr std::uint32_t Hammer_HitTickOffset(std::uint32_t interactionIndex) noexcept
		{ return 1u == interactionIndex ? 12u : HAMMER_HIT_TICK_OFFSET; }
		static constexpr float HAMMER_RANGE_M = 2.4f;
		/* cos(60 degrees): the swing covers a 120 degree arc in front. */
		static constexpr float HAMMER_HALF_ANGLE_COS = 0.5f;
		/* 일반 망치 입력 피해량이다. 미로 표적의 피격 adapter는 별도로 남은 HP 전부를 소비한다. */
		static constexpr std::uint32_t HAMMER_RAW_DAMAGE = 100u;
		/* MAZE HUD의 망치 skill ID다. 미로 표적은 다른 skill의 타격을 거절한다. */
		static constexpr LostArk::Shared::SKILL_ID HAMMER_SKILL_ID = 56411u;
		static constexpr const char* SPAWN_GROUP_TAG = "cardmaze.targets";
		/* The box a hammer swing must reach to take the telescope. */
		static constexpr const char* TELESCOPE_PLACEMENT_ID = "cardmaze.telescope";
		/* 입장 30tick 뒤 망원경 위에 생성하는 광대 상자다. 별도 group으로 일반 몬스터 피해를 받고 파괴 뒤 망원경을 연다. */
		static constexpr const char* CLOWN_BOX_ARCHETYPE_ID = "MONSTER_KOUKU_CLOWN_BOX";
		static constexpr const char* CLOWN_BOX_SPAWN_GROUP_TAG = "cardmaze.clownbox";
		static constexpr std::uint32_t CLOWN_BOX_SPAWN_DELAY_TICKS = 30u;
		/* CardMiro 영역의 world m 경계다. 0.5m 격자 116×114칸과 망원경 중심 좌표에 대응한다. */
		static constexpr float MAZE_MIN_X = -24.97f;
		static constexpr float MAZE_MAX_X = 33.03f;
		static constexpr float MAZE_MIN_Z = 1323.35f;
		static constexpr float MAZE_MAX_Z = 1380.35f;
		static constexpr float CENTER_X = 0.28f;
		static constexpr float CENTER_Z = 1351.65f;
		static constexpr float CENTER_KEEPOUT_M = 5.f;
		static constexpr float SPAWN_SPACING_M = 3.f;
		static constexpr float PLAYER_KEEPOUT_M = 4.f;
		static constexpr std::uint32_t SAMPLE_ATTEMPTS = 256u;

		static const char* Archetype_ForSuit(LostArk::Shared::MECHANIC_CARD_SYMBOL suit) noexcept;

		/* 역할·문양·표적 위치를 대기 상태로 준비한다. spawn 성공 뒤 Commit이 플레이어에 적용하고 실패 시 Abort로 버린다. */
		bool Plan(
			LostArk::Shared::PLAYER_ID claimantId,
			const std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
			const CServerNavigation& navigation,
			std::uint32_t serverTick,
			std::vector<SPAWN_REQUEST>& outSpawns,
			std::string& outStatus);
		void Register_Target(LostArk::Shared::NET_ENTITY_ID entityId,
			LostArk::Shared::MECHANIC_CARD_SYMBOL suit);
		void Commit(std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players);
		void Abort();
		[[nodiscard]] bool Is_Target(LostArk::Shared::NET_ENTITY_ID entityId) const noexcept;
		/* A hammer hit the room already landed on a registered target. */
		HIT_OUTCOME On_TargetHit(SERVER_PLAYER& hunter, const SERVER_WORLD_ENTITY& target, bool killed);
		/* 진행 중인 사냥에서 문양을 가진 참가자가 모두 나갔으면 true를 반환해 룸의 종료 정리를 요청한다. */
		bool Remove_Player(LostArk::Shared::PLAYER_ID playerId);
		void Reset(std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players);
		bool Can_Hit(const SERVER_PLAYER& player, LostArk::Shared::NET_ENTITY_ID targetId) const;
		void Retire_Target(LostArk::Shared::NET_ENTITY_ID targetId);
		bool Sample_Corridor(LostArk::Shared::MECHANIC_CARD_SYMBOL suit,
			const std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
			const std::vector<SERVER_WORLD_ENTITY>& entities, const CServerNavigation& navigation,
			std::uint32_t seed, SPAWN_REQUEST& out, bool avoidPersonalExits = false) const;
		void Reset_Progress(SERVER_PLAYER& player);
		bool Toggle_Telescope(SERVER_PLAYER& player);
		[[nodiscard]] bool Is_SoloHunter(LostArk::Shared::PLAYER_ID playerId) const noexcept;
		void Mark_Escaped(SERVER_PLAYER& player);
		bool All_LivingCentral(const std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players) const;
		static bool In_SafeZone(float x, float z) noexcept;
		void Complete() { m_ePhase = PHASE::COMPLETE; }
		[[nodiscard]] PHASE Get_Phase() const noexcept { return m_ePhase; }
		[[nodiscard]] LostArk::Shared::PLAYER_ID Get_TelescopeOwner() const noexcept { return m_iTelescopeOwner; }
		[[nodiscard]] const std::map<LostArk::Shared::NET_ENTITY_ID, LostArk::Shared::MECHANIC_CARD_SYMBOL>&
			Get_Targets() const noexcept { return m_Targets; }

	private:
		struct PARTICIPANT final
		{
			LostArk::Shared::CARD_MAZE_ROLE eRole = LostArk::Shared::CARD_MAZE_ROLE::NONE;
			LostArk::Shared::MECHANIC_CARD_SYMBOL eSuit = LostArk::Shared::MECHANIC_CARD_SYMBOL::NONE;
			std::uint8_t iKills = 0u;
			bool escaped = false;
		};
		static void Apply_ToPlayer(const PARTICIPANT& participant, SERVER_PLAYER& player) noexcept;

		PHASE m_ePhase = PHASE::INACTIVE;
		LostArk::Shared::PLAYER_ID m_iTelescopeOwner = LostArk::Shared::INVALID_PLAYER_ID;
		std::map<LostArk::Shared::PLAYER_ID, PARTICIPANT> m_Participants;
		LostArk::Shared::PLAYER_ID m_iPendingTelescopeOwner = LostArk::Shared::INVALID_PLAYER_ID;
		std::map<LostArk::Shared::PLAYER_ID, PARTICIPANT> m_PendingParticipants;
		/* Raised targets by entity id and the suit they show. */
		std::map<LostArk::Shared::NET_ENTITY_ID, LostArk::Shared::MECHANIC_CARD_SYMBOL> m_Targets;
		/* Each felled target counts once, however long its body stays. */
		std::set<LostArk::Shared::NET_ENTITY_ID> m_CountedKills;
		bool m_bMarchStarted = false;
	};
}
```


## Server/Private/KoukuSaydonLogicRuntime.cpp 전체 코드

수정 대상: [KoukuSaydonLogicRuntime.cpp](C:/Users/tnest/Desktop/LostArk/Server/Private/KoukuSaydonLogicRuntime.cpp)

```cpp
#include "KoukuSaydonLogicRuntime.h"

#include "ServerCombatHitRuntime.h"
#include "PlayerSkillSystem.h"
#include "KoukuSaydonBrain.h"
#include "ServerCollisionSystem.h"
#include "ServerNavigation.h"
#include "Gameplay/CombatCollisionContract.h"
#include "Gameplay/WorldCollisionContract.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <map>
#include <string>
#include <utility>
#include <cmath>
#include <limits>
//Ananymous Namespace 안에 존재하는 변수 함수, 왜 LostArk::Server 내부에 넣으면 안 되는 이유가 있을까?
namespace
{
	constexpr std::uint64_t SERVER_TICK_HZ = 30u;
	constexpr double DEGREES_PER_RADIAN = 57.295779513082320876;
	constexpr float DEFAULT_CLOWN_HOLD_MS = 15000.f;

    bool Has_LethalResult(const std::vector<LostArk::Server::BOSS_PATTERN_LOGIC_RESULT>& results)
    {
        using namespace LostArk::Server;

        return std::any_of(results.begin(), results.end(), [](const auto& result) {
            return result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::INSTANT_DEATH ||
                (result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::MAX_HP_PERCENT_DAMAGE && result.iPercent >= 100u);
        });
    }

	bool Try_CounterLanding(const LostArk::Server::SERVER_WORLD_ENTITY& boss,
		const LostArk::Server::CServerNavigation* navigation,
		const LostArk::Server::CServerCollisionSystem* collision, float& landingY)
	{
		using namespace LostArk::Server;
		SERVER_NAV_POINT ground{}, spawnGround{};
		if (!navigation || !collision || !std::isfinite(boss.fPositionY) ||
			!std::isfinite(boss.fSpawnPositionY) ||
			!navigation->Is_PointWalkableExact(boss.fPositionX, boss.fPositionZ) ||
			!navigation->Sample_Position(boss.fPositionX, boss.fPositionZ, ground) ||
			!navigation->Sample_Position(boss.fSpawnPositionX, boss.fSpawnPositionZ, spawnGround)) return false;
		// Restore the placement's ground-relative height, not an airborne clip's last up.
		// This preserves intentionally raised boss placements in other gates.
		landingY = ground.y + (boss.fSpawnPositionY - spawnGround.y);
		return std::isfinite(landingY) && collision->Is_CirclePositionClear(
			boss.fPositionX, landingY, boss.fPositionZ, boss.fCollisionRadius,
			boss.fCollisionRadius, boss.fCollisionRadius, boss.iNetEntityId);
	}

	/* Stable server-owned card selection for one encounter admission. */
	std::uint64_t Mix(std::uint64_t value) noexcept
	{
		value += 0x9E3779B97F4A7C15ull;
		value = (value ^ (value >> 30)) * 0xBF58476D1CE4E5B9ull;
		value = (value ^ (value >> 27)) * 0x94D049BB133111EBull;
		return value ^ (value >> 31);
	}

    // Build 전용: 성공 RESULT가 가리키는 조커와 같은 접촉 목록의 카드 7개를 찾는다.
    // Catalog는 읽기 전용이다. 검증 중 실패하면 해당 묶음의 원래 배치를 유지하고 부분 이동값은 넣지 않는다.
    void Build_JokerCardOffsets(
        const LostArk::Server::BOSS_PATTERN_DEFINITION& pattern,
        const LostArk::Server::SERVER_WORLD_ENTITY& boss,
        const std::uint32_t startTick,
        std::map<std::string, std::array<float, 3u>>& outOffsets)
    {
        using namespace LostArk::Server;
        for (const auto& window : pattern.LogicWindows)
        {
            if (window.eKind != BOSS_PATTERN_LOGIC_KIND::OBJECT_CONTACT || window.ContactTargets.size() != 7u)
            {
                continue;
            }
            for (const auto& result : window.OnSuccess)
            {
                if (result.eKind != BOSS_PATTERN_LOGIC_RESULT_KIND::COMPLETE_LOGIC_WINDOW ||
                    result.strContactTargetWorldOccurrenceId.empty() ||
                    outOffsets.contains(result.strContactTargetWorldOccurrenceId))
                {
                    continue;
                }
                const std::string& jokerId = result.strContactTargetWorldOccurrenceId;
                std::map<std::string, const BOSS_PATTERN_WORLD_SEQUENCE*> cards;
                bool valid = true;
                std::size_t jokerCount = 0u;
                for (const auto& target : window.ContactTargets)
                {
                    const BOSS_PATTERN_WORLD_SEQUENCE* sequence = nullptr;
                    for (const auto& candidate : pattern.WorldSequences)
                    {
                        if (candidate.strOccurrenceId != target.strWorldOccurrenceId)
                        {
                            continue;
                        }
                        if (sequence)
                        {
                            valid = false;
                            break;
                        }
                        sequence = &candidate;
                    }
                    if (!valid || !sequence || !sequence->Placement || sequence->CombatBody ||
                        !sequence->SupportWindows.empty() || sequence->bAuthoredMadness || sequence->bAnchorBossSpawn ||
                        sequence->fPositionOffsetX != 0.f || sequence->fPositionOffsetY != 0.f || sequence->fPositionOffsetZ != 0.f ||
                        sequence->strInstanceId != target.strWorldInstanceId ||
                        !std::isfinite(target.fWorldX) || !std::isfinite(target.fWorldZ) ||
                        !std::isfinite(target.fRadiusM) || target.fRadiusM <= 0.f ||
                        outOffsets.contains(target.strWorldOccurrenceId) ||
                        !cards.emplace(target.strWorldOccurrenceId, sequence).second)
                    {
                        valid = false;
                        break;
                    }
                    const bool isJoker = sequence->strInstanceId == "world.object.instance.kouku.joker_card";
                    if ((!isJoker && sequence->strInstanceId != "world.object.instance.kouku.card") ||
                        isJoker != (sequence->strOccurrenceId == jokerId))
                    {
                        valid = false;
                        break;
                    }
                    jokerCount += isJoker ? 1u : 0u;
                    const auto& p = *sequence->Placement;
                    if (!std::isfinite(p.fPositionX) || !std::isfinite(p.fPositionY) || !std::isfinite(p.fPositionZ) ||
                        !std::isfinite(p.fRotationXDegrees) || !std::isfinite(p.fRotationYDegrees) || !std::isfinite(p.fRotationZDegrees) ||
                        !std::isfinite(p.fScaleX) || !std::isfinite(p.fScaleY) || !std::isfinite(p.fScaleZ) ||
                        p.fScaleX <= 0.f || p.fScaleY <= 0.f || p.fScaleZ <= 0.f)
                    {
                        valid = false;
                        break;
                    }
                }
                if (!valid || jokerCount != 1u || cards.size() != 7u || !cards.contains(jokerId))
                {
                    continue;
                }
                const auto& jokerSequence = *cards.at(jokerId);
                const auto& joker = *jokerSequence.Placement;
                // 같은 회전·배율·출현 시각의 정적 카드에만 평행이동 공식을 적용한다.
                // 다른 orientation/scale을 허용하려면 구워진 접촉 중심도 inverse TRS→새 TRS로 바꾸는 별도 계약이 필요하다.
                for (const auto& [id, sequence] : cards)
                {
                    (void)id;
                    const auto& p = *sequence->Placement;
                    if (p.fRotationXDegrees != joker.fRotationXDegrees || p.fRotationYDegrees != joker.fRotationYDegrees ||
                        p.fRotationZDegrees != joker.fRotationZDegrees || p.fScaleX != joker.fScaleX ||
                        p.fScaleY != joker.fScaleY || p.fScaleZ != joker.fScaleZ ||
                        sequence->iStartMs != jokerSequence.iStartMs || sequence->iDurationMs != jokerSequence.iDurationMs)
                    {
                        valid = false;
                        break;
                    }
                }
                if (!valid)
                {
                    continue;
                }
                // stable occurrence 문자열을 해시한다. vector 순서나 displayName은 실행 identity가 아니다.
                std::uint64_t identity = 14695981039346656037ull;
                for (const unsigned char value : jokerId)
                {
                    identity = (identity ^ value) * 1099511628211ull;
                }
                const std::uint64_t seed = Mix(identity ^ (static_cast<std::uint64_t>(boss.iNetEntityId) << 32u) ^
                    boss.iPatternSequence ^ startTick);
                auto selected = cards.begin();
                std::advance(selected, static_cast<std::ptrdiff_t>(seed % cards.size()));
                const auto& slot = *selected->second->Placement;
                std::map<std::string, std::array<float, 3u>> staged;
                for (const auto& [id, sequence] : cards)
                {
                    (void)sequence;
                    staged.emplace(id, std::array<float, 3u>{0.f, 0.f, 0.f});
                }
                staged.at(jokerId) = {slot.fPositionX - joker.fPositionX, slot.fPositionY - joker.fPositionY, slot.fPositionZ - joker.fPositionZ};
                staged.at(selected->first) = {joker.fPositionX - slot.fPositionX, joker.fPositionY - slot.fPositionY, joker.fPositionZ - slot.fPositionZ};
                // 필요한 노드는 이미 지역 map에 있다. Room 소유 ledger에 묶음 전체를 넘겨 표시와 판정이 같은 결과를 읽게 한다.
                outOffsets.merge(staged);
            }
        }
    }

	float Wrap180(float degrees) noexcept
	{
		degrees = std::fmod(degrees, 360.f);
		if (degrees > 180.f)
			degrees -= 360.f;
		if (degrees <= -180.f)
			degrees += 360.f;
		return degrees;
	}

	float Wrap360(float degrees) noexcept
	{
		degrees = std::fmod(degrees, 360.f);
		if (degrees < 0.f)
			degrees += 360.f;
		return degrees;
	}

	std::uint32_t Percent_Of(const std::uint32_t value, const std::uint32_t percent) noexcept
	{
		const std::uint64_t scaled =
			(static_cast<std::uint64_t>(value) * percent + 50u) / 100u;
		return static_cast<std::uint32_t>((std::min<std::uint64_t>)(
			scaled, (std::numeric_limits<std::uint32_t>::max)()));
	}


}

std::uint32_t LostArk::Server::CKoukuSaydonLogicRuntime::Ticks_FromMs(
	const std::uint32_t ms) noexcept
{
	const std::uint64_t ticks =
		(static_cast<std::uint64_t>(ms) * SERVER_TICK_HZ + 999u) / 1000u;
	return static_cast<std::uint32_t>((std::min<std::uint64_t>)(
		ticks, (std::numeric_limits<std::uint32_t>::max)() - 1u));
}

std::uint32_t LostArk::Server::CKoukuSaydonLogicRuntime::Add_Ticks(
	const std::uint32_t tick, const std::uint32_t ticks) noexcept
{
	// Tick 0 is the reserved "never" value, so a wrap skips it.
	const std::uint32_t result = tick + ticks;
	return 0u == result ? 1u : result;
}

bool LostArk::Server::CKoukuSaydonLogicRuntime::Has_ReachedTick(
	const std::uint32_t serverTick, const std::uint32_t targetTick) noexcept
{
	if (0u == targetTick || 0u == serverTick)
		return false;
	return static_cast<std::int32_t>(serverTick - targetTick) >= 0;
}

bool LostArk::Server::CKoukuSaydonLogicRuntime::Is_Judgeable(
	const SERVER_PLAYER& player) noexcept
{
	return player.Is_Human() && Can_ReceiveSpatialContact(player);
}

bool LostArk::Server::CKoukuSaydonLogicRuntime::Can_ReceiveSpatialContact(
	const SERVER_PLAYER& player) noexcept
{
	using namespace LostArk::Shared;
	return 0u != player.iCurrentHp && player.isCombatReady &&
		PLAYER_ACTION_STATE::DEAD != player.eAction &&
		PLAYER_ACTION_STATE::FALLING != player.eAction &&
		PLAYER_ACTION_STATE::GRABBED != player.eAction;
}

void LostArk::Server::CKoukuSaydonLogicRuntime::Build(
	const BOSS_PATTERN_DEFINITION& pattern,
	const SERVER_WORLD_ENTITY& boss,
	const std::uint32_t startTick,
	KOUKUSAYDON_LOGIC_LEDGER& outLedger)
{
	KOUKUSAYDON_LOGIC_LEDGER ledger{};
	ledger.strPatternId = pattern.strPatternId;
	ledger.iPatternSequence = boss.iPatternSequence;
	ledger.iPatternStartTick = startTick;
	std::map<std::string, std::pair<std::uint32_t, std::uint32_t>> scheduledDance;
	for (std::size_t index = 0u; index < pattern.LogicWindows.size(); ++index)
	{
		const BOSS_PATTERN_LOGIC_WINDOW& window = pattern.LogicWindows[index];
		KOUKUSAYDON_LOGIC_WINDOW_STATE state{};
		state.iWindowIndex = static_cast<std::uint32_t>(index);
		if (window.iRepeatIntervalMs && !window.strOwnerWorldOccurrenceId.empty())
			for (const auto& world : pattern.WorldSequences)
				if (world.strOccurrenceId == window.strOwnerWorldOccurrenceId && world.bAuthoredMadness)
				{
					if (world.strInstanceId == "world.object.instance.kouku.mario_circus_ball.aura") state.iMadnessSource = 1u;
					else if (world.strInstanceId == "world.object.instance.kouku.odd_doll.large.att_battle_2_01" ||
						world.strInstanceId == "world.object.instance.kouku.odd_doll.att_battle_2_01") state.iMadnessSource = 2u;
					state.iMadnessContactIntervalMs = window.iRepeatIntervalMs;
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
        }
		if (BOSS_PATTERN_LOGIC_KIND::POSE_INPUT == window.eKind)
		{
			ledger.bDanceActive = true;
			const auto parentScope = window.strWindowId.find(".pattern.");
			const auto lane = window.strWindowId.rfind(".logic.");
			if (parentScope != std::string::npos && lane != std::string::npos && lane > parentScope)
			{
				const auto scope = window.strWindowId.substr(0u, lane);
				const auto end = window.iStartMs + window.iDurationMs;
				const auto found = scheduledDance.find(scope);
				if (found == scheduledDance.end()) scheduledDance.emplace(scope, std::make_pair(window.iStartMs, end));
				else
				{
					found->second.first = (std::min)(found->second.first, window.iStartMs);
					found->second.second = (std::max)(found->second.second, end);
				}
			}
			else ledger.eHudMode = LostArk::Shared::KOUKU_HUD_MODE::DANCE;
		}
		ledger.Windows.push_back(std::move(state));
		if (window.eKind == BOSS_PATTERN_LOGIC_KIND::OBJECT_CONTACT)
			ledger.ContactWindowOrder.push_back(static_cast<std::uint32_t>(index));
	}
	for (const auto& [scope, interval] : scheduledDance)
	{
		(void)scope;
		ledger.ScheduledDanceIntervals.emplace_back(Add_Ticks(startTick, Ticks_FromMs(interval.first)),
			Add_Ticks(startTick, Ticks_FromMs(interval.second)));
	}
	std::sort(ledger.ContactWindowOrder.begin(), ledger.ContactWindowOrder.end(), [&](const auto a, const auto b) {
		const auto& first = pattern.LogicWindows[a]; const auto& second = pattern.LogicWindows[b];
		return first.iContactPriority != second.iContactPriority ? first.iContactPriority > second.iContactPriority : first.strWindowId < second.strWindowId;
	});
	for (std::size_t index = 0u; index < pattern.MechanicTriggers.size(); ++index)
	{
		const auto& trigger = pattern.MechanicTriggers[index];
		if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::SHOWTIME_PLAYER_TARGETS ||
			trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TRACK_TARGET ||
			trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::PURSUIT_PROJECTILES ||
			trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_RANDOM_TARGET_PRESENTATION)
		{
			KOUKUSAYDON_PLAYER_TARGET_WINDOW_STATE state;
			state.iTriggerIndex = static_cast<std::uint32_t>(index);
			state.iStartTick = Add_Ticks(startTick, Ticks_FromMs(trigger.iStartMs));
			state.iEndTick = Add_Ticks(startTick, Ticks_FromMs(trigger.iStartMs + trigger.iDurationMs));
			// A one-shot pursuit Trigger owns a birth tick, even when its visual box is shorter than one tick.
			if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::PURSUIT_PROJECTILES && trigger.iSpawnIntervalMs == 0u &&
				!Has_ReachedTick(state.iEndTick, Add_Ticks(state.iStartTick, 1u)))
				state.iEndTick = Add_Ticks(state.iStartTick, 1u);
			if (trigger.ShowtimeBomb)
				state.iBombStartTick = Add_Ticks(startTick, Ticks_FromMs(trigger.ShowtimeBomb->iBodyStartMs));
			state.iNextFixedTick = state.iStartTick;
			state.iNextRandomTick = state.iStartTick;
			ledger.PlayerTargetWindows.push_back(std::move(state));
			// Presentation targeting still selects exactly once through its start cue.
			if (trigger.eKind != BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_RANDOM_TARGET_PRESENTATION) continue;
		}
		KOUKUSAYDON_LOGIC_CUE_STATE cue{};
		cue.iIndex = static_cast<std::uint32_t>(index);
		cue.iStartTick = Add_Ticks(startTick, Ticks_FromMs(pattern.MechanicTriggers[index].iStartMs));
		ledger.MechanicTriggers.push_back(cue);
	}
	std::stable_sort(ledger.MechanicTriggers.begin(), ledger.MechanicTriggers.end(), [&](const auto& a, const auto& b) {
		return pattern.MechanicTriggers[a.iIndex].iStartMs < pattern.MechanicTriggers[b.iIndex].iStartMs;
	});
	for (std::size_t index = 0u; index < pattern.WorldSequences.size(); ++index)
	{
		KOUKUSAYDON_LOGIC_CUE_STATE cue{};
		cue.iIndex = static_cast<std::uint32_t>(index);
		cue.iStartTick = Add_Ticks(startTick,
			Ticks_FromMs(pattern.WorldSequences[index].iStartMs));
		ledger.WorldSequences.push_back(cue);
	}
    Build_JokerCardOffsets(pattern, boss, startTick, ledger.WorldPositionOffsets);
	outLedger = std::move(ledger);
}

namespace
{
	const LostArk::Server::BOSS_LOGIC_REGION& Window_Region(
		const LostArk::Server::BOSS_LOGIC_REGION& region, const LostArk::Server::KOUKUSAYDON_LOGIC_WINDOW_STATE& state)
	{
		const auto found = state.FrozenRegions.find(region.strRegionId);
		return found == state.FrozenRegions.end() ? region : found->second;
	}

	struct LOGIC_REGION_TRANSFORM final
	{
		float centerX = 0.f, centerZ = 0.f, yaw = 0.f;
		float halfX = 0.f, halfZ = 0.f, radius = 0.f, radiusX = 0.f, radiusZ = 0.f;
		float centerY = 0.f, halfY = 0.f;
		bool hasGrip = false;
		std::array<float, 3u> grip{};
	};

	bool Resolve_LogicRegionTransform(const LostArk::Server::BOSS_LOGIC_REGION& region,
		const LostArk::Server::SERVER_WORLD_ENTITY& boss,
		const double patternElapsedTicks, LOGIC_REGION_TRANSFORM& outTransform, const bool requireVisible = true) noexcept
	{
		using namespace LostArk::Server;
		// BOSS_START is resolved transactionally when its owning window opens.
		if (region.eAnchor == BOSS_LOGIC_REGION_ANCHOR::BOSS_START) return false;
		float centerX = region.fCenterX, centerZ = region.fCenterZ, yaw = region.fYawDegrees;
		float halfX = region.fHalfX, halfZ = region.fHalfZ, radius = region.fRadiusM;
		float radiusX = region.fRadiusXM > 0.f ? region.fRadiusXM : radius;
		float radiusZ = region.fRadiusZM > 0.f ? region.fRadiusZM : radius;
		float centerY = region.fCenterY, halfY = region.fHalfY;
		if (region.bLinearMotion)
		{
			if (region.iMotionDurationMs == 0u) return false;
			const double elapsedMs = patternElapsedTicks * (1000.0 / 30.0);
			const float alpha = static_cast<float>(std::clamp(
				(elapsedMs - region.iMotionStartMs) / region.iMotionDurationMs, 0.0, 1.0));
			centerX += region.EndPositionOffset[0] * alpha;
			centerY += region.EndPositionOffset[1] * alpha;
			centerZ += region.EndPositionOffset[2] * alpha;
			const float sx = 1.f + (region.EndScale[0] - 1.f) * alpha;
			const float sy = 1.f + (region.EndScale[1] - 1.f) * alpha;
			const float sz = 1.f + (region.EndScale[2] - 1.f) * alpha;
			halfX *= sx; halfY *= sy; halfZ *= sz;
			radius *= (std::max)(sx, sz); radiusX *= sx; radiusZ *= sz;
			if (region.bCircle || region.bCylinder) radius = (std::max)(radiusX, radiusZ);
		}
		bool hasGrip = false; std::array<float, 3u> grip{};
		if (region.WorldTrack.bEnabled)
		{
			const auto& track = region.WorldTrack;
			if (track.Keys.empty()) return false;
			const std::uint32_t startTicks = CKoukuSaydonLogicRuntime::Ticks_FromMs(track.iStartMs);
			if (patternElapsedTicks < startTicks) return false;
			const double delayed = BOSS_LOGIC_REGION_ANCHOR::BOSS_CURRENT == region.eAnchor ?
				static_cast<double>(patternElapsedTicks) * (1000.0 / 30.0) - track.iStartMs - track.iStartDelayMs :
				static_cast<double>(patternElapsedTicks - startTicks) * (1000.0 / 30.0) - track.iStartDelayMs;
			if (delayed < 0.0) return false;
			const double timeMs = (std::min)(static_cast<double>(track.iDurationMs), delayed * track.fPlaybackSpeed);
			const auto next = std::upper_bound(track.Keys.begin(), track.Keys.end(), timeMs,
				[](const double time, const BOSS_LOGIC_WORLD_TRANSFORM_KEY& key) { return time < key.iTimeMs; });
			const auto& left = next == track.Keys.begin() ? track.Keys.front() : *(next - 1);
			const auto& right = next == track.Keys.end() ? left : *next;
			if (requireVisible && !left.bVisible) return false;
			float factor = right.iTimeMs > left.iTimeMs ?
				static_cast<float>((timeMs - left.iTimeMs) / (right.iTimeMs - left.iTimeMs)) : 0.f;
			factor = (std::clamp)(factor, 0.f, 1.f);
			if (track.bSmoothStep) factor = factor * factor * (3.f - 2.f * factor);
			const auto mix = [factor](const float a, const float b) { return a + (b - a) * factor; };
			const float sx = track.fBaselineScaleX * mix(left.fScaleX, right.fScaleX);
			const float sy = track.fBaselineScaleY * mix(left.fScaleY, right.fScaleY);
			const float sz = track.fBaselineScaleZ * mix(left.fScaleZ, right.fScaleZ);
			if (!(sx > 0.f && sz > 0.f)) return false;
			const float baselineRadians = track.fBaselineYawDegrees * 0.017453292519943295f;
			const float ox = mix(left.fOffsetX, right.fOffsetX), oz = mix(left.fOffsetZ, right.fOffsetZ);
			float qy = right.fRotationY, qw = right.fRotationW;
			float dot = left.fRotationY * qy + left.fRotationW * qw;
			if (dot < 0.f) { qy = -qy; qw = -qw; dot = -dot; }
			dot = (std::clamp)(dot, -1.f, 1.f);
			float leftWeight = 1.f - factor, rightWeight = factor;
			if (dot < 0.9995f)
			{
				const float omega = std::acos(dot), sine = std::sin(omega);
				leftWeight = std::sin((1.f - factor) * omega) / sine;
				rightWeight = std::sin(factor * omega) / sine;
			}
			qy = left.fRotationY * leftWeight + qy * rightWeight;
			qw = left.fRotationW * leftWeight + qw * rightWeight;
			const float qLength = std::sqrt(qy * qy + qw * qw);
			if (!(qLength > 0.f)) return false;
			qy /= qLength; qw /= qLength;
			const float worldYaw = track.fBaselineYawDegrees +
				std::atan2(2.f * qw * qy, 1.f - 2.f * qy * qy) * 57.29577951308232f;
			const float worldRadians = worldYaw * 0.017453292519943295f;
			const float localCenterX = centerX, localCenterY = centerY, localCenterZ = centerZ;
			centerX = track.fBaselineX + std::cos(baselineRadians) * ox + std::sin(baselineRadians) * oz +
				std::cos(worldRadians) * localCenterX * sx + std::sin(worldRadians) * localCenterZ * sz;
			centerZ = track.fBaselineZ - std::sin(baselineRadians) * ox + std::cos(baselineRadians) * oz -
				std::sin(worldRadians) * localCenterX * sx + std::cos(worldRadians) * localCenterZ * sz;
			yaw += worldYaw; halfX *= sx; halfZ *= sz; radius *= sx; radiusX *= sx; radiusZ *= sz;
			if (region.bCircle || region.bCylinder) radius = (std::max)(radiusX, radiusZ);
			centerY = track.fBaselineY + mix(left.fOffsetY, right.fOffsetY) + localCenterY * sy;
			halfY *= sy;
			hasGrip = left.bHasGripPosition && right.bHasGripPosition;
			if (hasGrip) for (size_t axis = 0; axis < 3u; ++axis) grip[axis] = mix(left.GripPosition[axis], right.GripPosition[axis]);
		}
		if (BOSS_LOGIC_REGION_ANCHOR::BOSS_CURRENT == region.eAnchor)
		{
			// A baked bone tip is boss-local. Apply the live authoritative root once,
			// after its track and the authored TARGET_YAW offset have been combined.
			const float radians = boss.fYawDegrees * 0.017453292519943295f;
			const float localX = centerX, localZ = centerZ;
			centerX = boss.fPositionX + std::cos(radians) * localX + std::sin(radians) * localZ;
			centerZ = boss.fPositionZ - std::sin(radians) * localX + std::cos(radians) * localZ;
			yaw += boss.fYawDegrees;
			if (region.bCylinder || region.bLinearMotion) centerY += boss.fPositionY;
		}
		if (BOSS_LOGIC_REGION_ANCHOR::BOSS_SPAWN == region.eAnchor)
		{
			centerX += boss.fSpawnPositionX;
			centerZ += boss.fSpawnPositionZ;
			if (region.bCylinder || region.bLinearMotion) centerY += boss.fSpawnPositionY;
		}
		outTransform = { centerX, centerZ, yaw, halfX, halfZ, radius, radiusX, radiusZ, centerY, halfY, hasGrip, grip };
		return true;
	}

	bool Contains_LogicRegion(const LostArk::Server::BOSS_LOGIC_REGION& region,
		const LostArk::Server::SERVER_WORLD_ENTITY& boss,
		const LostArk::Server::SERVER_PLAYER& player, const std::uint32_t patternElapsedTicks = 0u) noexcept
	{
		LOGIC_REGION_TRANSFORM transform;
		if (!Resolve_LogicRegionTransform(region, boss, patternElapsedTicks, transform)) return false;
		const auto [centerX, centerZ, yaw, halfX, halfZ, radius, radiusX, radiusZ, centerY, halfY, hasGrip, grip] = transform;
		if (region.bCylinder || region.bLinearMotion)
		{
			using namespace LostArk::Shared::WorldCollision;
			using namespace LostArk::Shared::CombatCollision;
			if (!std::isfinite(player.fPositionY) || !std::isfinite(centerY) || !(halfY > 0.f) ||
				std::abs(player.fPositionY + PLAYER_CENTER_OFFSET_Y - centerY) >
					halfY + PLAYER_HALF_EXTENT_Y + CONTACT_MARGIN) return false;
			const BODY_CIRCLE_XZ body{player.fPositionX, player.fPositionZ, PLAYER_HALF_EXTENT_X + CONTACT_MARGIN};
			const float inner = region.fInnerRadiusM * radius / region.fRadiusM;
			if (inner > 0.f && !Circle_IntersectsRing(body, centerX, centerZ, inner, radius)) return false;
			if (region.bCircle || region.bCylinder) return Circles_Overlap(CIRCLE_XZ{centerX, centerZ, radius}, body);
			const float radians = yaw * 0.017453292519943295f;
			const float fx = std::sin(radians), fz = std::cos(radians);
			if (region.bSector) return Circle_IntersectsEllipticSector(body, centerX, centerZ, fx, fz,
				radiusX, radiusZ, region.fHalfAngleDegrees * 2.f, region.bReverseSector);
			return Circle_IntersectsForwardBox(body, centerX - fx * halfZ, centerZ - fz * halfZ, fx, fz, 2.f * halfZ, halfX);
		}
		if (hasGrip && !region.bSector && !region.bCircle)
		{
			using namespace LostArk::Shared::WorldCollision;
			using namespace LostArk::Shared::CombatCollision;
			if (!std::isfinite(player.fPositionY) ||
				std::abs(player.fPositionY + PLAYER_CENTER_OFFSET_Y - centerY) >
					halfY + PLAYER_HALF_EXTENT_Y + CONTACT_MARGIN) return false;
			const float radians = yaw * 0.017453292519943295f;
			const float forwardX = std::sin(radians), forwardZ = std::cos(radians);
			return Circle_IntersectsForwardBox(
				{ player.fPositionX, player.fPositionZ, PLAYER_HALF_EXTENT_X + CONTACT_MARGIN },
				centerX - forwardX * halfZ, centerZ - forwardZ * halfZ,
				forwardX, forwardZ, 2.f * halfZ, halfX);
		}
		if (hasGrip && std::abs(player.fPositionY - centerY) > halfY) return false;
		const float dx = player.fPositionX - centerX, dz = player.fPositionZ - centerZ;
		if (!std::isfinite(dx) || !std::isfinite(dz)) return false;
		const float radians = yaw * 0.017453292519943295f;
		const float localX = std::cos(radians) * dx - std::sin(radians) * dz;
		const float localZ = std::sin(radians) * dx + std::cos(radians) * dz;
		if (!region.bSector && !region.bCircle)
			return std::fabs(localX) <= halfX && std::fabs(localZ) <= halfZ;
		const float distanceSq = dx * dx + dz * dz;
		const float inner = region.fInnerRadiusM * radius / region.fRadiusM;
		if (inner > 0.f && distanceSq < inner * inner) return false;
		if (region.bCircle) return distanceSq <= radius * radius;
		const float nx = localX / radiusX, nz = localZ / radiusZ;
		if (nx * nx + nz * nz > 1.f) return false;
		if (region.bReverseSector && region.fHalfAngleDegrees <= 0.f) return true;
		if (region.bReverseSector && region.fHalfAngleDegrees >= 180.f) return false;
		if (distanceSq < 0.000001f) return false;
		const float angle = std::atan2(nx, nz) * 57.29577951308232f;
		// A shared radial edge belongs to exactly one sector.
		const bool inside = angle >= -region.fHalfAngleDegrees && angle < region.fHalfAngleDegrees;
		return region.bReverseSector ? !inside : inside;
	}

	// Clip the fixed-tick interval against vertical slabs, then the horizontal
	// swept volume. A thin rising pillar must still hit between two sampled poses.
	bool Swept_LinearColliderContainsPlayer(const LostArk::Server::BOSS_LOGIC_REGION& region,
		const LostArk::Server::SERVER_WORLD_ENTITY& boss, const LostArk::Server::SERVER_PLAYER& player,
		const std::uint32_t elapsedTicks, const std::uint32_t windowStartTicks, const std::uint32_t windowEndTicks) noexcept
	{
		using namespace LostArk::Shared::WorldCollision;
		if (!region.bLinearMotion || region.WorldTrack.bEnabled || region.bSector || elapsedTicks <= windowStartTicks) return false;
		const double begin = (std::max)(double(elapsedTicks) - 1.0, double(windowStartTicks));
		const double end = (std::min)(double(elapsedTicks), double(windowEndTicks));
		LOGIC_REGION_TRANSFORM a, b;
		if (end <= begin || !Resolve_LogicRegionTransform(region, boss, begin, a) ||
			!Resolve_LogicRegionTransform(region, boss, end, b)) return false;
		double low = 0.0, high = 1.0;
		const auto bound = [&](double first, double last) {
			if (!std::isfinite(first) || !std::isfinite(last)) return false;
			const double delta = last - first;
			if (std::abs(delta) < 1e-12) return first <= 0.0;
			const double crossing = -first / delta;
			if (delta > 0.0) high = (std::min)(high, crossing); else low = (std::max)(low, crossing);
			return low <= high;
		};
		const auto slab = [&](double first, double last, double firstHalf, double lastHalf) {
			return bound(first - firstHalf, last - lastHalf) && bound(-first - firstHalf, -last - lastHalf);
		};
		const double bodyY = player.fPositionY + PLAYER_CENTER_OFFSET_Y;
		if (!slab(bodyY - a.centerY, bodyY - b.centerY,
			a.halfY + PLAYER_HALF_EXTENT_Y + CONTACT_MARGIN, b.halfY + PLAYER_HALF_EXTENT_Y + CONTACT_MARGIN)) return false;
		const double bodyRadius = PLAYER_HALF_EXTENT_X + CONTACT_MARGIN;
		const double ax = player.fPositionX - a.centerX, az = player.fPositionZ - a.centerZ;
		const double bx = player.fPositionX - b.centerX, bz = player.fPositionZ - b.centerZ;
		if (region.bCylinder || region.bCircle)
		{
			// Hollow circles keep the sampled test: sweeping only the outer disc would fill the hole.
			if (region.fInnerRadiusM > 0.f) return false;
			const double r = a.radius + bodyRadius, dr = b.radius - a.radius, dx = bx - ax, dz = bz - az;
			const double q2 = dx * dx + dz * dz - dr * dr;
			const double q1 = 2.0 * (ax * dx + az * dz - r * dr), q0 = ax * ax + az * az - r * r;
			const auto q = [&](double t) { return (q2 * t + q1) * t + q0; };
			double nearest = (std::min)(q(low), q(high));
			if (q2 > 1e-12) nearest = (std::min)(nearest, q(std::clamp(-q1 / (2.0 * q2), low, high)));
			return nearest <= 0.0;
		}
		const double yaw = a.yaw * .017453292519943295;
		const double x0 = std::cos(yaw) * ax - std::sin(yaw) * az, x1 = std::cos(yaw) * bx - std::sin(yaw) * bz;
		const double z0 = std::sin(yaw) * ax + std::cos(yaw) * az, z1 = std::sin(yaw) * bx + std::cos(yaw) * bz;
		// Split where the nearest box edge changes; within each interval distance is quadratic.
		std::array<double, 6u> cuts{low, high}; size_t count = 2u;
		const auto split = [&](double first, double last) {
			const double delta = last - first;
			if (std::abs(delta) < 1e-12) return;
			const double t = -first / delta;
			if (t > low && t < high) cuts[count++] = t;
		};
		split(x0 - a.halfX, x1 - b.halfX); split(-x0 - a.halfX, -x1 - b.halfX);
		split(z0 - a.halfZ, z1 - b.halfZ); split(-z0 - a.halfZ, -z1 - b.halfZ);
		std::sort(cuts.begin(), cuts.begin() + count);
		const auto gap = [](double first, double last, double halfFirst, double halfLast, double t) {
			return (std::max)(0.0, std::abs(first + (last - first) * t) - (halfFirst + (halfLast - halfFirst) * t));
		};
		for (size_t piece = 1u; piece < count; ++piece)
		{
			const double begin = cuts[piece - 1u], end = cuts[piece];
			const double x = gap(x0, x1, a.halfX, b.halfX, begin), z = gap(z0, z1, a.halfZ, b.halfZ, begin);
			const double dx = gap(x0, x1, a.halfX, b.halfX, end) - x, dz = gap(z0, z1, a.halfZ, b.halfZ, end) - z;
			const double length = dx * dx + dz * dz;
			const double t = length > 1e-18 ? std::clamp(-(x * dx + z * dz) / length, 0.0, 1.0) : 0.0;
			if ((x + dx * t) * (x + dx * t) + (z + dz * t) * (z + dz * t) <= bodyRadius * bodyRadius) return true;
		}
		return false;
	}

	// Sweep only the existing centered WORLD circle contract. Each visible
	// key interval is independent, so a hidden gap, delayed birth or expired
	// lifetime cannot become a phantom segment between two visible samples.
	bool Swept_CenteredWorldCircleContainsPlayer(const LostArk::Server::BOSS_LOGIC_REGION& region,
		const LostArk::Server::SERVER_WORLD_ENTITY& boss,
		const LostArk::Server::SERVER_PLAYER& player, const std::uint32_t elapsedTicks,
		const std::uint32_t windowStartTicks, const std::uint32_t windowEndTicks) noexcept
	{
		using namespace LostArk::Server;
		using namespace LostArk::Shared::CombatCollision;
		if (region.bCylinder || region.bLinearMotion || !region.bCircle || region.fInnerRadiusM > 0.f || !region.WorldTrack.bEnabled || region.fCenterX != 0.f || region.fCenterZ != 0.f ||
			region.eAnchor == BOSS_LOGIC_REGION_ANCHOR::BOSS_CURRENT || elapsedTicks <= windowStartTicks ||
			!std::isfinite(player.fPositionX) || !std::isfinite(player.fPositionZ)) return false;
		const auto& track = region.WorldTrack;
		if (track.Keys.size() < 2u || !(track.fPlaybackSpeed > 0.f)) return false;
		const double beginTick = (std::max)(double(elapsedTicks) - 1.0, double(windowStartTicks));
		const double endTick = (std::min)(double(elapsedTicks), double(windowEndTicks));
		const double originTick = CKoukuSaydonLogicRuntime::Ticks_FromMs(track.iStartMs) + track.iStartDelayMs * .03;
		const double beginMs = (beginTick - originTick) / .03 * track.fPlaybackSpeed;
		const auto next = std::upper_bound(track.Keys.begin(), track.Keys.end(), beginMs,
			[](const double time, const BOSS_LOGIC_WORLD_TRANSFORM_KEY& key) { return time < key.iTimeMs; });
		const auto firstIndex = next == track.Keys.begin() ? 0u : static_cast<std::size_t>(next - track.Keys.begin() - 1u);
		for (std::size_t index = firstIndex; index + 1u < track.Keys.size(); ++index)
		{
			const auto& left = track.Keys[index];
			const auto& right = track.Keys[index + 1u];
			if (originTick + left.iTimeMs * .03 / track.fPlaybackSpeed >= endTick) break;
			if (!left.bVisible || left.iTimeMs >= right.iTimeMs) continue;
			const double begin = (std::max)(beginTick, originTick + left.iTimeMs * .03 / track.fPlaybackSpeed);
			const double end = (std::min)(endTick, originTick + right.iTimeMs * .03 / track.fPlaybackSpeed);
			if (end <= begin) continue;
			LOGIC_REGION_TRANSFORM first, last;
			// At the terminal key the pose is still the preceding visible segment's
			// endpoint. Its visibility applies only to the next segment.
			if (!Resolve_LogicRegionTransform(region, boss, begin, first, false) ||
				!Resolve_LogicRegionTransform(region, boss, end, last, false) ||
				std::abs(first.radius - last.radius) > .00001f) continue;
			if (Segment_IntersectsCircle(first.centerX, first.centerZ, last.centerX, last.centerZ,
				CIRCLE_XZ{player.fPositionX, player.fPositionZ, first.radius})) return true;
		}
		return false;
	}

    // Same visible key intervals as the circle sweep. New Object Tool floor
    // boxes keep a fixed yaw; translation and dimensions interpolate per key.
    bool Swept_WorldBoxContainsPlayer(const LostArk::Server::BOSS_LOGIC_REGION& region,
        const LostArk::Server::SERVER_WORLD_ENTITY& boss, const LostArk::Server::SERVER_PLAYER& player,
        const uint32_t elapsedTicks, const uint32_t windowStartTicks, const uint32_t windowEndTicks) noexcept
    {
        using namespace LostArk::Server;
        if (region.bCylinder || region.bLinearMotion || region.bCircle || region.bSector || !region.WorldTrack.bEnabled ||
            region.eAnchor == BOSS_LOGIC_REGION_ANCHOR::BOSS_CURRENT || elapsedTicks <= windowStartTicks) return false;
        const auto& track = region.WorldTrack;
        if (track.Keys.size() < 2u || track.bSmoothStep || !(track.fPlaybackSpeed > 0.f)) return false;
        const double beginTick = (std::max)(double(elapsedTicks) - 1.0, double(windowStartTicks));
        const double endTick = (std::min)(double(elapsedTicks), double(windowEndTicks));
        const double originTick = CKoukuSaydonLogicRuntime::Ticks_FromMs(track.iStartMs) + track.iStartDelayMs * .03;
        const double beginMs = (beginTick - originTick) / .03 * track.fPlaybackSpeed;
        const auto next = std::upper_bound(track.Keys.begin(), track.Keys.end(), beginMs,
            [](double time, const auto& key) { return time < key.iTimeMs; });
        const size_t firstIndex = next == track.Keys.begin() ? 0u : size_t(next - track.Keys.begin() - 1u);
        for (size_t i = firstIndex; i + 1u < track.Keys.size(); ++i)
        {
            const auto& left = track.Keys[i]; const auto& right = track.Keys[i + 1u];
            if (originTick + left.iTimeMs * .03 / track.fPlaybackSpeed >= endTick) break;
            if (!left.bVisible || left.iTimeMs >= right.iTimeMs) continue;
            const double begin = (std::max)(beginTick, originTick + left.iTimeMs * .03 / track.fPlaybackSpeed);
            const double end = (std::min)(endTick, originTick + right.iTimeMs * .03 / track.fPlaybackSpeed);
            LOGIC_REGION_TRANSFORM a, b;
            if (end <= begin || !Resolve_LogicRegionTransform(region, boss, begin, a, false) ||
                !Resolve_LogicRegionTransform(region, boss, end, b, false) || std::abs(Wrap180(a.yaw - b.yaw)) > .0001f) continue;
            double low = 0.0, high = 1.0;
            const auto bound = [&](double first, double last) {
                if (!std::isfinite(first) || !std::isfinite(last)) return false;
                const double delta = last - first;
                if (std::abs(delta) < 1e-12) return first <= 0.0;
                const double cross = -first / delta;
                if (delta > 0.0) high = (std::min)(high, cross); else low = (std::max)(low, cross);
                return low <= high;
            };
            const auto axis = [&](double first, double last, double halfFirst, double halfLast) {
                return bound(first - halfFirst, last - halfLast) && bound(-first - halfFirst, -last - halfLast);
            };
            const double radians = a.yaw * .017453292519943295;
            const double cosine = std::cos(radians), sine = std::sin(radians);
            const double ax = player.fPositionX - a.centerX, az = player.fPositionZ - a.centerZ;
            const double bx = player.fPositionX - b.centerX, bz = player.fPositionZ - b.centerZ;
            if (a.hasGrip)
            {
                using namespace LostArk::Shared::WorldCollision;
                // Clip to the part of this visible interval where the body's
                // vertical span intersects the hook, then test its round XZ body.
                if (!axis(player.fPositionY + PLAYER_CENTER_OFFSET_Y - a.centerY,
                    player.fPositionY + PLAYER_CENTER_OFFSET_Y - b.centerY,
                    a.halfY + PLAYER_HALF_EXTENT_Y + CONTACT_MARGIN,
                    b.halfY + PLAYER_HALF_EXTENT_Y + CONTACT_MARGIN)) continue;
                const double x0 = cosine * ax - sine * az, x1 = cosine * bx - sine * bz;
                const double z0 = sine * ax + cosine * az, z1 = sine * bx + cosine * bz;
                std::array<double, 6u> cuts{ low, high };
                std::size_t count = 2u;
                const auto split = [&](double first, double last) {
                    const double delta = last - first;
                    if (std::abs(delta) > 1e-12)
                    {
                        const double t = -first / delta;
                        if (t > low && t < high) cuts[count++] = t;
                    }
                };
                split(x0 - a.halfX, x1 - b.halfX); split(-x0 - a.halfX, -x1 - b.halfX);
                split(z0 - a.halfZ, z1 - b.halfZ); split(-z0 - a.halfZ, -z1 - b.halfZ);
                std::sort(cuts.begin(), cuts.begin() + count);
                const auto gap = [](double first, double last, double halfFirst, double halfLast, double t) {
                    return (std::max)(0.0, std::abs(first + (last - first) * t) -
                        (halfFirst + (halfLast - halfFirst) * t));
                };
                const double radius = PLAYER_HALF_EXTENT_X + CONTACT_MARGIN;
                for (std::size_t piece = 1u; piece < count; ++piece)
                {
                    const double begin = cuts[piece - 1u], end = cuts[piece];
                    const double gx = gap(x0, x1, a.halfX, b.halfX, begin);
                    const double gz = gap(z0, z1, a.halfZ, b.halfZ, begin);
                    const double dx = gap(x0, x1, a.halfX, b.halfX, end) - gx;
                    const double dz = gap(z0, z1, a.halfZ, b.halfZ, end) - gz;
                    const double lengthSquared = dx * dx + dz * dz;
                    const double t = lengthSquared > 1e-18 ?
                        std::clamp(-(gx * dx + gz * dz) / lengthSquared, 0.0, 1.0) : 0.0;
                    if ((gx + dx * t) * (gx + dx * t) + (gz + dz * t) * (gz + dz * t) <= radius * radius)
                        return true;
                }
                continue;
            }
            if (axis(cosine * ax - sine * az, cosine * bx - sine * bz, a.halfX, b.halfX) &&
                axis(sine * ax + cosine * az, sine * bx + cosine * bz, a.halfZ, b.halfZ) &&
                (!a.hasGrip || axis(player.fPositionY - a.centerY, player.fPositionY - b.centerY, a.halfY, b.halfY))) return true;
        }
        return false;
    }

	bool Intersects_LogicRegion(const LostArk::Server::BOSS_LOGIC_REGION& region,
		const LostArk::Server::SERVER_WORLD_ENTITY& boss,
		const LostArk::Shared::CombatCollision::BODY_CIRCLE_XZ& target,
		const std::uint32_t patternElapsedTicks) noexcept
	{
		using namespace LostArk::Shared::CombatCollision;
		LOGIC_REGION_TRANSFORM transform;
		if (!Resolve_LogicRegionTransform(region, boss, patternElapsedTicks, transform)) return false;
		const auto [centerX, centerZ, yaw, halfX, halfZ, radius, radiusX, radiusZ, centerY, halfY, hasGrip, grip] = transform;
		const float inner = region.fInnerRadiusM * radius / region.fRadiusM;
		if (inner > 0.f && !Circle_IntersectsRing(target, centerX, centerZ, inner, radius)) return false;
		if (region.bCircle || region.bCylinder) return Circles_Overlap(CIRCLE_XZ{ centerX, centerZ, radius }, target);
		const float radians = yaw * 0.017453292519943295f;
		const float forwardX = std::sin(radians), forwardZ = std::cos(radians);
		if (region.bSector && inner > 0.f)
			return region.fHalfAngleDegrees >= 180.f || Circle_IntersectsCone(target, centerX, centerZ,
				forwardX, forwardZ, radius, region.fHalfAngleDegrees * 2.f);
		if (region.bSector)
			return Circle_IntersectsEllipticSector(target, centerX, centerZ, forwardX, forwardZ,
				radiusX, radiusZ, region.fHalfAngleDegrees * 2.f, region.bReverseSector);
		// The shared box starts at its rear edge and extends along local +Z.
		return Circle_IntersectsForwardBox(target, centerX - forwardX * halfZ,
			centerZ - forwardZ * halfZ, forwardX, forwardZ, halfZ * 2.f, halfX);
	}

    const LostArk::Server::BOSS_LOGIC_WORLD_TRANSFORM_KEY* Resolve_HookReleaseKey(
        const LostArk::Server::BOSS_LOGIC_REGION& region) noexcept
    {
        using namespace LostArk::Server;
        const auto& track = region.WorldTrack;
        if (!track.bEnabled || track.Keys.size() < 2u) return nullptr;
        auto last = track.Keys.size() - 1u;
        while (last && !track.Keys[last].bVisible) --last;
        if (!track.Keys[last].bVisible) return nullptr;
        const auto position = [](const BOSS_LOGIC_WORLD_TRANSFORM_KEY& key) {
            return key.bHasGripPosition ? key.GripPosition :
                std::array<float, 3u>{key.fOffsetX, key.fOffsetY, key.fOffsetZ};
        };
        const auto end = position(track.Keys[last]);
        auto arrived = last;
        while (arrived)
        {
            const auto previous = position(track.Keys[arrived - 1u]);
            if (!track.Keys[arrived - 1u].bVisible || std::abs(previous[0] - end[0]) > .001f ||
                std::abs(previous[1] - end[1]) > .001f || std::abs(previous[2] - end[2]) > .001f) break;
            --arrived;
        }
        // A stationary carrier retains its authored deadline. A baked hook's final
        // upward exit carries only the hook: release at the beginning of that ascent.
        auto ascent = arrived;
        // Strict rise stops at the end of a level approach/hold; allowing equal
        // heights would rewind through that approach and drop the player early.
        while (region.eAnchor == BOSS_LOGIC_REGION_ANCHOR::WORLD && ascent &&
            track.Keys[ascent].bHasGripPosition && track.Keys[ascent - 1u].bVisible &&
            track.Keys[ascent - 1u].bHasGripPosition &&
            track.Keys[ascent - 1u].GripPosition[1] < track.Keys[ascent].GripPosition[1])
            --ascent;
        if (ascent < arrived && end[1] - track.Keys[ascent].GripPosition[1] >
            2.f * LostArk::Shared::WorldCollision::PLAYER_HALF_EXTENT_Y)
            return &track.Keys[ascent];
        return arrived ? &track.Keys[arrived] : nullptr;
    }

    double Hook_TrackTimeMs(const LostArk::Server::BOSS_LOGIC_REGION& region,
        const std::uint32_t patternElapsedTicks) noexcept
    {
        const auto& track = region.WorldTrack;
        const auto startTicks = LostArk::Server::CKoukuSaydonLogicRuntime::Ticks_FromMs(track.iStartMs);
        const double elapsed = region.eAnchor == LostArk::Server::BOSS_LOGIC_REGION_ANCHOR::BOSS_CURRENT ?
            double(patternElapsedTicks) * (1000.0 / 30.0) - track.iStartMs :
            double(patternElapsedTicks - (std::min)(patternElapsedTicks, startTicks)) * (1000.0 / 30.0);
        return (elapsed - track.iStartDelayMs) * track.fPlaybackSpeed;
    }

	/* Puts the player on the region that just caught them. The region rides a
	World Object's transform track, so this is what "hanging from the hook"
	means on the Server: one attachment naming that region, and a deadline the
	room enforces even if this pattern stops running. A baked grip supplies
	XYZ; legacy regions without a grip keep the existing ground-drag height. */
	bool Hang_PlayerOnRegion(LostArk::Server::SERVER_PLAYER& player,
		const LostArk::Server::BOSS_LOGIC_REGION& region,
		const LostArk::Server::SERVER_WORLD_ENTITY& boss, const std::uint32_t patternElapsedTicks,
		const std::uint32_t windowIndex, const std::uint32_t regionIndex,
		const std::uint32_t releaseTick, const std::uint32_t serverTick,
		const LostArk::Server::CServerNavigation* navigation) noexcept
	{
		using namespace LostArk::Shared;
        const auto* release = Resolve_HookReleaseKey(region);
        if (release && release->bHasGripPosition && !region.WorldTrack.Keys.empty() &&
            region.WorldTrack.Keys.back().bHasGripPosition &&
            region.WorldTrack.Keys.back().GripPosition[1] - release->GripPosition[1] >
                2.f * LostArk::Shared::WorldCollision::PLAYER_HALF_EXTENT_Y &&
            Hook_TrackTimeMs(region, patternElapsedTicks) >= release->iTimeMs) return false;
		LOGIC_REGION_TRANSFORM transform;
		if (0u == releaseTick || 0u == player.iCurrentHp ||
			PLAYER_ACTION_STATE::DEAD == player.eAction || PLAYER_ACTION_STATE::FEAR == player.eAction ||
			!Resolve_LogicRegionTransform(region, boss, patternElapsedTicks, transform) ||
			!std::isfinite(transform.centerX) || !std::isfinite(transform.centerZ) ||
			!std::isfinite(transform.yaw))
			return false;
		/* The hook lanes cross cells this Area never baked as walkable, so a
		navigation refusal must not cancel the catch. Sample the floor when the
		grid can answer and otherwise keep the height the runner already had. */
		LostArk::Server::SERVER_NAV_POINT ground{};
		const bool hasGround = nullptr != navigation &&
			navigation->Resolve_TraversalStep(player.fPositionX, player.fPositionZ,
				transform.centerX, transform.centerZ, ground);
		player.iAttachmentOwnerNetEntityId = boss.iNetEntityId;
		player.eAttachmentSlot = PLAYER_ATTACHMENT_SLOT::WORLD_HOOK_TIP;
		player.iAttachmentPatternSequence = boss.iPatternSequence;
		player.fAttachmentLocalOffsetX = 0.f;
		player.fAttachmentLocalOffsetY = 0.f;
		player.fAttachmentLocalOffsetZ = 0.f;
		// Hook travels on local +X; player animations face local +Z.
		player.fAttachmentYawOffsetDegrees = 90.f;
		player.iAttachmentWindowIndex = windowIndex;
		player.iAttachmentRegionIndex = regionIndex;
		player.iAttachmentReleaseTick = releaseTick;
		player.eAction = PLAYER_ACTION_STATE::GRABBED;
		player.iActionStartTick = serverTick;
		player.iCurrentSkillId = INVALID_SKILL_ID;
		player.Clear_SkillTarget();
		player.iComboStage = 0u;
		player.hasBufferedComboInput = false;
		player.hasReleasedHold = false;
		player.PendingCommand.Clear();
		player.TriggerMove = {};
		player.hasMoveGoal = false;
		player.MovePath.clear();
		player.iMovePathIndex = 0u;
		player.isCombatReady = false;
		player.fPositionX = transform.hasGrip ? transform.grip[0] : transform.centerX;
		if (transform.hasGrip) player.fPositionY = transform.grip[1];
		else if (hasGround) player.fPositionY = ground.y;
		player.fPositionZ = transform.hasGrip ? transform.grip[2] : transform.centerZ;
		player.fYawDegrees = Wrap180(transform.yaw + 90.f);
		return true;
	}

    bool Resolve_HookReleaseGround(const LostArk::Server::CServerNavigation* navigation,
        const std::array<float, 3u>& grip, LostArk::Server::SERVER_NAV_POINT& ground) noexcept
    {
        if (!navigation) return false;
        // Grip height is above the deck. An unbaked cell under it has no valid
        // same-level reference, so admit the ordinary spawn projection only
        // within one player body of the endpoint, on this exact navigation grid.
        constexpr float maximumDistance = 2.f * LostArk::Shared::WorldCollision::PLAYER_HALF_EXTENT_Y;
        const auto nearbyFloor = [&] {
            return std::isfinite(ground.x) && std::isfinite(ground.y) && std::isfinite(ground.z) &&
                std::abs(ground.y - grip[1]) <= maximumDistance &&
                std::hypot(ground.x - grip[0], ground.z - grip[2]) <= maximumDistance &&
                navigation->Is_InSameNavigationGrid(grip[0], grip[2], ground.x, ground.z) &&
                navigation->Is_PointWalkableExact(ground.x, ground.z, ground.y);
        };
        if (navigation->Project_PointOnSameLevel(grip[0], grip[2], ground, grip[1]) && nearbyFloor()) return true;
        return navigation->Project_Point(grip[0], grip[2], ground, grip[1]) && nearbyFloor();
    }

    /* The grip carries the player until its terminal ascent, not until its
    airborne final key. The room still owns the actual attachment release. */
    bool Drag_HookedPlayer(LostArk::Server::SERVER_PLAYER& player,
        const LostArk::Server::BOSS_LOGIC_REGION& region,
        const LostArk::Server::SERVER_WORLD_ENTITY& boss,
        const std::uint32_t patternElapsedTicks, const std::uint32_t serverTick,
        const LostArk::Server::CServerNavigation* navigation) noexcept
    {
        using namespace LostArk::Server;
        const auto* release = Resolve_HookReleaseKey(region);
        if (region.eAnchor == BOSS_LOGIC_REGION_ANCHOR::WORLD && release && release->bHasGripPosition &&
            Hook_TrackTimeMs(region, patternElapsedTicks) >= release->iTimeMs)
        {
            // Baked grip XYZ is already in world metres, just as in
            // Resolve_LogicRegionTransform; never apply the boss basis again.
            const auto& position = release->GripPosition;
            SERVER_NAV_POINT ground{};
            if (Resolve_HookReleaseGround(navigation, position, ground))
            {
                player.fPositionX = ground.x; player.fPositionY = ground.y; player.fPositionZ = ground.z;
                player.hasMoveGoal = false;
                return false;
            }
            // A failed floor admission must not resume the upward carrier or release
            // an airborne body. Retry this same boundary while the carrier owns it.
            player.fPositionX = position[0]; player.fPositionY = position[1]; player.fPositionZ = position[2];
            // Player attachment deadlines run before Kouku logic on the next
            // room tick, so keep ownership through that pre-logic update.
            player.iAttachmentReleaseTick = CKoukuSaydonLogicRuntime::Add_Ticks(serverTick, 2u);
            return true;
        }
        LOGIC_REGION_TRANSFORM transform;
        const bool visible = Resolve_LogicRegionTransform(region, boss, patternElapsedTicks, transform);
        if ((!visible && !Resolve_LogicRegionTransform(region, boss, patternElapsedTicks, transform, false)) ||
            !std::isfinite(transform.centerX) || !std::isfinite(transform.centerZ) || !std::isfinite(transform.yaw))
            return false;
        SERVER_NAV_POINT ground{};
        const bool hasGround = navigation && navigation->Resolve_TraversalStep(player.fPositionX,
            player.fPositionZ, transform.centerX, transform.centerZ, ground);
        player.fPositionX = transform.hasGrip ? transform.grip[0] : transform.centerX;
        if (transform.hasGrip) player.fPositionY = transform.grip[1];
        else if (hasGround) player.fPositionY = ground.y;
        player.fPositionZ = transform.hasGrip ? transform.grip[2] : transform.centerZ;
        player.fYawDegrees = Wrap180(transform.yaw + player.fAttachmentYawOffsetDegrees);
        player.hasMoveGoal = false;
        player.isCombatReady = false;
        if (!visible) return false;
        return !release || Hook_TrackTimeMs(region, patternElapsedTicks) < release->iTimeMs;
    }
}

float LostArk::Server::CKoukuSaydonLogicRuntime::Predict_ContactRisk(
    const SERVER_WORLD_ENTITY& boss, const BOSS_PATTERN_DEFINITION& pattern,
    const KOUKUSAYDON_LOGIC_LEDGER& ledger, const SERVER_PLAYER& probe,
    const std::uint32_t serverTick, const std::uint32_t horizonMs)
{
    if (!ledger.Is_Active() || !boss.iCurrentHp || ledger.iPatternSequence != boss.iPatternSequence)
        return 0.f;
    float risk = 0.f;
    const auto horizon = Ticks_FromMs((std::min)(horizonMs, 5000u));
    for (const auto& state : ledger.Windows)
    {
        if (state.bClosed || state.iWindowIndex >= pattern.LogicWindows.size()) continue;
        const auto& window = pattern.LogicWindows[state.iWindowIndex];
        if (window.eKind != BOSS_PATTERN_LOGIC_KIND::ENTER_AREA &&
            window.eKind != BOSS_PATTERN_LOGIC_KIND::AREA_OVERLAP) continue;
        if (!window.strOwnerWorldOccurrenceId.empty() && ledger.RetiredWorldOccurrences.contains(window.strOwnerWorldOccurrenceId)) continue;
        if (window.eKind == BOSS_PATTERN_LOGIC_KIND::ENTER_AREA && !window.bRearmOnExit &&
            !window.bRepeatAfterKnockback && !window.iRepeatIntervalMs)
        {
            const auto answer = state.Answers.find(probe.iPlayerId);
            if (answer != state.Answers.end() && answer->second == KOUKUSAYDON_LOGIC_ANSWER::SUCCESS) continue;
        }
        const auto& results = window.bInsideIsFail ? window.OnFail : window.OnSuccess;
        float severity = 0.f;
        for (const auto& result : results)
        {
            if (result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::INSTANT_DEATH) severity = 5.f;
            else if (result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::FIXED_DAMAGE ||
                result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::MAX_HP_PERCENT_DAMAGE ||
                result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::FEAR ||
                result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::CAPTURE_PLAYER ||
                result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::GRAB_TO_WORLD_OBJECT)
                severity = (std::max)(severity, 1.f);
        }
        if (!severity) continue;
        bool contact = false;
        for (std::uint32_t step = 0u; step <= horizon && !contact; ++step)
        {
            const auto tick = Add_Ticks(serverTick, step);
            if (!Has_ReachedTick(tick, state.iStartTick)) continue;
            const bool end = Has_ReachedTick(tick, state.iEndTick);
            if (end && tick != state.iEndTick) break;
            if (window.iRepeatIntervalMs && end) break;
            if (window.eKind == BOSS_PATTERN_LOGIC_KIND::AREA_OVERLAP && !window.iRepeatIntervalMs && !end) continue;
            const auto elapsed = tick - ledger.iPatternStartTick;
            for (const auto& authored : window.CardRegions)
            {
                const auto& region = Window_Region(authored, state);
                if (Contains_LogicRegion(region, boss, probe, elapsed) ||
                    Swept_LinearColliderContainsPlayer(region, boss, probe, elapsed,
                        state.iStartTick - ledger.iPatternStartTick, state.iEndTick - ledger.iPatternStartTick) ||
                    Swept_CenteredWorldCircleContainsPlayer(region, boss, probe, elapsed,
                        state.iStartTick - ledger.iPatternStartTick, state.iEndTick - ledger.iPatternStartTick) ||
                    Swept_WorldBoxContainsPlayer(region, boss, probe, elapsed,
                        state.iStartTick - ledger.iPatternStartTick, state.iEndTick - ledger.iPatternStartTick))
                { contact = true; break; }
            }
        }
        if (contact) risk += severity;
    }
    return risk;
}

void LostArk::Server::CKoukuSaydonLogicRuntime::Assign_EncounterCard(
	SERVER_PLAYER& player, const LostArk::Shared::NET_ENTITY_ID encounterOwnerId,
	const std::uint32_t serverTick, const std::uint8_t unavailableSymbols)
{
	using namespace LostArk::Shared;
	if (player.Is_Guide()) return;
	if (0u == player.iCurrentHp || PLAYER_ACTION_STATE::DEAD == player.eAction ||
		INVALID_NET_ENTITY_ID == encounterOwnerId)
		return;
	if (MECHANIC_CARD_SYMBOL::NONE != player.eMechanicCardSymbol &&
		MECHANIC_CARD_COLOR::NONE != player.eMechanicCardColor)
		return;
	const std::uint64_t roll = Mix((static_cast<std::uint64_t>(encounterOwnerId) << 32) ^
		(static_cast<std::uint64_t>(player.iPlayerId) << 8) ^ serverTick);
	std::uint8_t available[4]{};
	std::uint32_t count = 0u;
	for (std::uint8_t symbol = 1u; symbol <= 4u; ++symbol)
		if (0u == (unavailableSymbols & (1u << (symbol - 1u)))) available[count++] = symbol;
	if (!count) return;
	player.eMechanicCardSymbol = static_cast<MECHANIC_CARD_SYMBOL>(available[roll % count]);
	// Color is independent of the remaining suit deck.
	player.eMechanicCardColor = 0u == (Mix(roll) & 1u) ? MECHANIC_CARD_COLOR::RED : MECHANIC_CARD_COLOR::BLACK;
}

void LostArk::Server::CKoukuSaydonLogicRuntime::Discard(
	KOUKUSAYDON_LOGIC_LEDGER& ledger,
	std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
	SERVER_WORLD_ENTITY* const pBoss)
{
	using namespace LostArk::Shared;
	if (!ledger.Is_Active())
		return;
	Reveal_CardMazeEntryPlayers(ledger, players);
	if (nullptr != pBoss)
	{
		pBoss->iKoukuItemStaggerMaximum = 0u;
		pBoss->iKoukuItemStaggerCredit = 0u;
		pBoss->iKoukuDamageReductionWindows = 0u;
		pBoss->bKoukuShieldActive = false;
		pBoss->fKoukuShieldArcDegrees = 0.f;
		pBoss->KoukuShieldRegions.clear();
        (void)CBossCombatRuntime::Set_Flag(pBoss->BossCombat, SERVER_BOSS_COMBAT_FLAG::COUNTERABLE, false);
        CBossCombatRuntime::Clear_PatternOutcomes(*pBoss);
	}
	for (const auto& state : ledger.Windows)
		for (const auto& [id, entityId] : state.BoundPlayers)
		{
			const auto found = players.find(id);
			if (found != players.end() && found->second.iNetEntityId == entityId && pBoss &&
				found->second.iPatternBindOwnerNetEntityId == pBoss->iNetEntityId &&
				found->second.iPatternBindSequence == ledger.iPatternSequence)
				found->second.Clear_PatternBindStatus();
		}
	if (pBoss)
		for (auto& [id, player] : players)
			if (player.bPatternBound && player.iPatternBindOwnerNetEntityId == pBoss->iNetEntityId &&
				player.iPatternBindSequence == ledger.iPatternSequence) player.Clear_PatternBindStatus();
	ledger = {};
}

void LostArk::Server::CKoukuSaydonLogicRuntime::Capture_CardMazeEntryRoster(
	KOUKUSAYDON_LOGIC_LEDGER& ledger, const std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players)
{
	if (ledger.bCardMazeEntryRosterCaptured) return;
	ledger.bCardMazeEntryRosterCaptured = true;
	for (const auto& [id, player] : players)
		if (player.Is_Human() && (Is_Judgeable(player) || !player.iCurrentHp) && player.eCardMazeRole == LostArk::Shared::CARD_MAZE_ROLE::NONE)
			ledger.CardMazeEntryPlayers.push_back(id);
	std::sort(ledger.CardMazeEntryPlayers.begin(), ledger.CardMazeEntryPlayers.end(), [&](auto left, auto right) {
		const float lx = players.at(left).fPositionX, rx = players.at(right).fPositionX;
		return lx != rx ? lx > rx : left < right;
	});
}

void LostArk::Server::CKoukuSaydonLogicRuntime::Reveal_CardMazeEntryPlayers(
	const KOUKUSAYDON_LOGIC_LEDGER& ledger, std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players)
{
	for (const auto id : ledger.CardMazeEntryPlayers)
		if (auto found = players.find(id); found != players.end())
			found->second.CardMaze.flags &= static_cast<std::uint8_t>(~LostArk::Shared::CARD_MAZE_ENTRY_HIDDEN);
}

bool LostArk::Server::CKoukuSaydonLogicRuntime::Stage_CardMazeEntryPlayers(
	const BOSS_PATTERN_MECHANIC_TRIGGER& trigger, KOUKUSAYDON_LOGIC_LEDGER& ledger,
	std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
	const CServerNavigation* navigation, const CServerCollisionSystem* collision, std::string& outStatus)
{
	using namespace LostArk::Shared;
	Capture_CardMazeEntryRoster(ledger, players);
	const auto reject = [&](const char* reason) {
		Reveal_CardMazeEntryPlayers(ledger, players);
		// Keep the captured flag: later hide/enter cues must not recapture a rejected run.
		ledger.CardMazeEntryPlayers.clear();
		ledger.iCardMazeNextHiddenPlayer = 0u;
		outStatus = std::string("Card maze staging preserved all player positions: ") + reason;
		return false;
	};
	if (!navigation || !navigation->Is_Loaded() || trigger.PlayerEntryPositions.empty() ||
		trigger.PlayerEntryPositions.size() > 4u || ledger.CardMazeEntryPlayers.empty() ||
		ledger.CardMazeEntryPlayers.size() > trigger.PlayerEntryPositions.size())
		return reject("navigation, roster or destination count is unavailable");
	std::vector<SERVER_NAV_POINT> grounds;
	for (const auto& position : trigger.PlayerEntryPositions)
	{
		SERVER_NAV_POINT ground{};
		if (std::any_of(position.begin(), position.end(), [](float value) { return !std::isfinite(value) || std::abs(value) > 100000.f; }) ||
			!navigation->Is_PointWalkableExact(position[0], position[2]) ||
			!navigation->Sample_Position(position[0], position[2], ground) || std::abs(ground.y - position[1]) > 1.f)
			return reject("a destination is not on its authored navigation floor");
		grounds.push_back(ground);
	}
	std::vector<std::pair<PLAYER_ID, SERVER_PLAYER>> staged;
	for (std::size_t slot = 0u; slot < ledger.CardMazeEntryPlayers.size(); ++slot)
	{
		const auto id = ledger.CardMazeEntryPlayers[slot];
		const auto found = players.find(id);
		if (found == players.end()) return reject("an entry participant is no longer present");
		const auto& source = found->second;
		const auto& position = trigger.PlayerEntryPositions[slot];
		if ((source.iCurrentHp && !Is_Judgeable(source)) || source.bPatternBound || source.TriggerMove.isActive || source.iMarioStage ||
			source.bArenaEjectionActive || source.CardMaze.transferStartTick || source.eCardMazeRole != CARD_MAZE_ROLE::NONE ||
			(collision && !collision->Is_PlayerPositionClear(position[0], grounds[slot].y, position[2], source.iNetEntityId)))
			return reject("a participant or destination is unavailable");
		SERVER_PLAYER player = source;
		player.Clear_CardMazeState(); player.Clear_KoukuInteractionState();
		player.fPositionX = position[0]; player.fPositionY = grounds[slot].y; player.fPositionZ = position[2];
		player.hasMoveGoal = false; player.MovePath.clear(); player.iMovePathIndex = 0u;
		player.PendingCommand.Clear(); player.Clear_SkillTarget(); player.Projectiles.clear();
		player.eAction = player.iCurrentHp ? PLAYER_ACTION_STATE::NONE : PLAYER_ACTION_STATE::DEAD; player.iCurrentSkillId = INVALID_SKILL_ID;
		player.iActionStartTick = 0u; player.fActionElapsedSeconds = 0.f;
		player.iComboStage = 0u; player.hasBufferedComboInput = false;
		player.fKnockbackRemainingSeconds = 0.f;
		staged.emplace_back(id, std::move(player));
	}
	for (auto& [id, player] : staged) players.at(id) = std::move(player);
	ledger.iCardMazeNextHiddenPlayer = 0u;
	outStatus = "Card maze cinematic staging committed";
	return true;
}

bool LostArk::Server::CKoukuSaydonLogicRuntime::Enter_CardMaze(
	const BOSS_PATTERN_MECHANIC_TRIGGER& trigger, KOUKUSAYDON_LOGIC_LEDGER& ledger,
	std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
	const CServerNavigation* navigation, const CServerCollisionSystem* collision, std::string& outStatus)
{
	using namespace LostArk::Shared;
	Capture_CardMazeEntryRoster(ledger, players);
	const auto reject = [&](const char* reason) {
		Reveal_CardMazeEntryPlayers(ledger, players);
		outStatus = std::string("Card maze entry preserved all player positions: ") + reason;
		return false;
	};
	SERVER_NAV_POINT ground{};
	if (!navigation || !navigation->Is_Loaded() ||
		!navigation->Is_PointWalkableExact(trigger.fTeleportX, trigger.fTeleportZ) ||
		!navigation->Sample_Position(trigger.fTeleportX, trigger.fTeleportZ, ground) ||
		std::abs(ground.y - trigger.fTeleportY) > 1.f ||
		!CKoukuCardMazeRuntime::In_SafeZone(trigger.fTeleportX, trigger.fTeleportZ))
		return reject("destination is not on the maze central navigation");
	std::vector<std::pair<PLAYER_ID, SERVER_PLAYER>> staged;
	for (const auto id : ledger.CardMazeEntryPlayers)
	{
		const auto found = players.find(id);
		if (found == players.end()) continue;
		const auto& source = found->second;
		if ((source.iCurrentHp && !Is_Judgeable(source)) || source.bPatternBound || source.TriggerMove.isActive ||
			source.bArenaEjectionActive || source.CardMaze.transferStartTick ||
			source.eCardMazeRole != CARD_MAZE_ROLE::NONE ||
			(collision && !collision->Is_PlayerPositionClear(trigger.fTeleportX, ground.y, trigger.fTeleportZ, source.iNetEntityId)))
			return reject("a participant or destination is unavailable");
		SERVER_PLAYER player = source;
		player.Clear_CardMazeState();
		player.Clear_KoukuInteractionState();
		player.eKoukuAreaHudMode = KOUKU_HUD_MODE::MAZE;
		player.eMadnessForm = PLAYER_MADNESS_FORM::NORMAL;
		player.fPositionX = trigger.fTeleportX; player.fPositionY = ground.y; player.fPositionZ = trigger.fTeleportZ;
		player.hasMoveGoal = false; player.MovePath.clear(); player.iMovePathIndex = 0u;
		player.PendingCommand.Clear(); player.Clear_SkillTarget(); player.Projectiles.clear();
		player.eAction = player.iCurrentHp ? PLAYER_ACTION_STATE::NONE : PLAYER_ACTION_STATE::DEAD; player.iCurrentSkillId = INVALID_SKILL_ID;
		player.iActionStartTick = 0u; player.fActionElapsedSeconds = 0.f;
		player.iComboStage = 0u; player.hasBufferedComboInput = false;
		player.fKnockbackRemainingSeconds = 0.f;
		staged.emplace_back(id, std::move(player));
	}
	if (staged.empty()) return reject("no entry participant remains");
	for (auto& [id, player] : staged) players.at(id) = std::move(player);
	Reveal_CardMazeEntryPlayers(ledger, players);
	outStatus = "Card maze entry committed; strike the central telescope with Q to begin";
	return true;
}

bool LostArk::Server::CKoukuSaydonLogicRuntime::Is_PointInsideRegion(
	const BOSS_LOGIC_REGION& region, const SERVER_WORLD_ENTITY& boss,
	const float worldX, const float worldZ) noexcept
{
	if (!std::isfinite(worldX) || !std::isfinite(worldZ) || !std::isfinite(boss.fPositionX) ||
		!std::isfinite(boss.fPositionZ) || !std::isfinite(boss.fYawDegrees)) return false;
	LOGIC_REGION_TRANSFORM transform;
	if (!region.bSector || region.bReverseSector || region.bLinearMotion || region.WorldTrack.bEnabled ||
		!Resolve_LogicRegionTransform(region, boss, 0u, transform) ||
		!(transform.radiusX > 0.f) || !(transform.radiusZ > 0.f)) return false;
	// A bomb is a point, so no player radius or shared-edge ownership is added.
	// This is the inclusive point-interior branch of the shared elliptic sector.
	const double radians = double(transform.yaw) * 0.017453292519943295;
	const double dx = double(worldX) - transform.centerX, dz = double(worldZ) - transform.centerZ;
	const double nx = (std::cos(radians) * dx - std::sin(radians) * dz) / transform.radiusX;
	const double nz = (std::sin(radians) * dx + std::cos(radians) * dz) / transform.radiusZ;
	return nx * nx + nz * nz <= 1.0 &&
		std::abs(std::atan2(nx, nz)) <= double(region.fHalfAngleDegrees) * 0.017453292519943295;
}

void LostArk::Server::CKoukuSaydonLogicRuntime::Capture_StartRegions(
	const SERVER_WORLD_ENTITY& boss, const BOSS_PATTERN_LOGIC_WINDOW& window,
	KOUKUSAYDON_LOGIC_WINDOW_STATE& state, const std::uint32_t patternElapsedTicks)
{
	for (const auto& region : window.CardRegions)
	{
		if (region.eAnchor != BOSS_LOGIC_REGION_ANCHOR::BOSS_START ||
			state.FrozenRegions.contains(region.strRegionId)) continue;
		const auto captureStartMs = region.iAnchorCaptureStartMs == 0xffffffffu ? window.iStartMs : region.iAnchorCaptureStartMs;
		if (patternElapsedTicks < Ticks_FromMs(captureStartMs)) continue;
		auto source = region;
		source.eAnchor = BOSS_LOGIC_REGION_ANCHOR::BOSS_CURRENT;
		source.bLinearMotion = false;
		LOGIC_REGION_TRANSFORM transform;
		if (!Resolve_LogicRegionTransform(source, boss, double(captureStartMs) * .03, transform)) continue;
		auto frozen = region;
		frozen.eAnchor = BOSS_LOGIC_REGION_ANCHOR::WORLD;
		frozen.WorldTrack = {};
		frozen.fCenterX = transform.centerX; frozen.fCenterZ = transform.centerZ;
		frozen.fCenterY = transform.centerY + ((source.bCylinder || source.bLinearMotion) ? 0.f : boss.fPositionY);
		frozen.fYawDegrees = transform.yaw;
		frozen.fHalfX = transform.halfX; frozen.fHalfY = transform.halfY; frozen.fHalfZ = transform.halfZ;
		frozen.fRadiusM = transform.radius; frozen.fRadiusXM = transform.radiusX; frozen.fRadiusZM = transform.radiusZ;
		const float yaw = (transform.yaw - region.fYawDegrees) * .017453292519943295f;
		const float x = region.EndPositionOffset[0], z = region.EndPositionOffset[2];
		frozen.EndPositionOffset[0] = std::cos(yaw) * x + std::sin(yaw) * z;
		frozen.EndPositionOffset[2] = -std::sin(yaw) * x + std::cos(yaw) * z;
		state.FrozenRegions.emplace(region.strRegionId, std::move(frozen));
	}
}

void LostArk::Server::CKoukuSaydonLogicRuntime::Open_Window(
	SERVER_WORLD_ENTITY& boss,
	const BOSS_PATTERN_LOGIC_WINDOW& window,
	KOUKUSAYDON_LOGIC_WINDOW_STATE& state,
	const KOUKUSAYDON_LOGIC_LEDGER& ledger,
	std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players)
{
	using namespace LostArk::Shared;
	(void)ledger;
	state.bOpened = true;
	state.Answers.clear();
	state.InsidePlayers.clear();
	state.NextContactHitTicks.clear();

	Capture_StartRegions(boss, window, state, Ticks_FromMs(window.iStartMs));
	switch (window.eKind)
	{
	case BOSS_PATTERN_LOGIC_KIND::CARD_DICE_BIND:
	{
		std::vector<SERVER_PLAYER*> eligible;
		for (auto& [id, player] : players)
			if (Is_Judgeable(player) && !player.bPatternBound && !player.iMarioStage &&
				!player.TriggerMove.isActive && player.eCardMazeRole == CARD_MAZE_ROLE::NONE)
				eligible.push_back(&player);
		if (eligible.empty()) break;
		auto* freePlayer = eligible[Mix((static_cast<std::uint64_t>(boss.iPatternSequence) << 32u) ^
			boss.iNetEntityId ^ state.iStartTick) % eligible.size()];
		state.iFreePlayerNetEntityId = freePlayer->iNetEntityId;
		boss.iTargetEntityId = boss.iPatternTargetEntityId = freePlayer->iNetEntityId;
		for (auto* player : eligible)
		{
			if (player == freePlayer) continue;
			player->fPatternBindRestoreX = player->fPositionX;
			player->fPatternBindRestoreY = player->fPositionY;
			player->fPatternBindRestoreZ = player->fPositionZ;
			player->fPatternBindRestoreYawDegrees = player->fYawDegrees;
			player->bPatternBindRestoreCombatReady = player->isCombatReady;
			player->bPatternBound = true;
			player->iPatternBindOwnerNetEntityId = boss.iNetEntityId;
			player->iPatternBindSequence = boss.iPatternSequence;
			player->iPatternBindEndTick = state.iEndTick;
			player->eAction = PLAYER_ACTION_STATE::NONE;
			player->iCurrentSkillId = INVALID_SKILL_ID;
			player->iActionStartTick = 0u;
			player->fActionElapsedSeconds = 0.f;
			player->Clear_SkillTarget();
			player->iComboStage = 0u;
			player->hasBufferedComboInput = false;
			player->hasReleasedHold = false;
			player->PendingCommand.Clear();
			player->hasMoveGoal = false;
			player->MovePath.clear();
			player->iMovePathIndex = 0u;
			player->iFearEndTick = 0u;
			player->strFearPresentationId.clear();
			player->iKnockdownEndTick = 0u;
			player->fKnockbackRemainingSeconds = 0.f;
			state.BoundPlayers.emplace(player->iPlayerId, player->iNetEntityId);
		}
		break;
	}
	case BOSS_PATTERN_LOGIC_KIND::ROULETTE_CARD_MATCH:
		for (auto& [playerId, player] : players)
			if (Is_Judgeable(player))
				state.Answers[playerId] = KOUKUSAYDON_LOGIC_ANSWER::NONE;
		break;
    case BOSS_PATTERN_LOGIC_KIND::COUNTER_WINDOW:
        (void)CBossCombatRuntime::Set_Flag(boss.BossCombat, SERVER_BOSS_COMBAT_FLAG::COUNTERABLE, true);
        break;
	case BOSS_PATTERN_LOGIC_KIND::BOSS_DAMAGE_REDUCTION:
		++boss.iKoukuDamageReductionWindows;
		break;
	case BOSS_PATTERN_LOGIC_KIND::STAGGER_WINDOW:
		boss.iKoukuItemStaggerMaximum = window.iThreshold;
		boss.iKoukuItemStaggerCredit = 0u;
		boss.bKoukuShieldActive = window.fShieldArcDegrees > 0.f;
		boss.fKoukuShieldArcDegrees = window.fShieldArcDegrees;
		boss.fKoukuShieldNormalYawOffsetDegrees = window.fNormalYawOffsetDegrees;
		boss.KoukuShieldRegions = window.CardRegions;
		break;
	case BOSS_PATTERN_LOGIC_KIND::OBJECT_OVERLAP:
	case BOSS_PATTERN_LOGIC_KIND::OBJECT_CONTACT:
	case BOSS_PATTERN_LOGIC_KIND::EXTERNAL_SIGNAL:
    case BOSS_PATTERN_LOGIC_KIND::ATTACHMENT_HOLD:
		// One world object is judged independently of the party roster.
		break;
	case BOSS_PATTERN_LOGIC_KIND::GAZE_REAL_BOSS:
	case BOSS_PATTERN_LOGIC_KIND::POSE_INPUT:
	default:
		for (auto& [playerId, player] : players)
		{
			if (Is_Judgeable(player))
				state.Answers[playerId] = KOUKUSAYDON_LOGIC_ANSWER::NONE;
		}
		break;
	}
}

void LostArk::Server::CKoukuSaydonLogicRuntime::Close_Window(
	SERVER_WORLD_ENTITY& boss,
	const BOSS_PATTERN_LOGIC_WINDOW& window,
	KOUKUSAYDON_LOGIC_WINDOW_STATE& state,
	std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players)
{
	using namespace LostArk::Shared;
	if (state.bClosed) return;
	state.bClosed = true;
	if (state.bOpened && window.eKind == BOSS_PATTERN_LOGIC_KIND::BOSS_DAMAGE_REDUCTION && boss.iKoukuDamageReductionWindows)
		--boss.iKoukuDamageReductionWindows;
    if (window.eKind == BOSS_PATTERN_LOGIC_KIND::COUNTER_WINDOW)
        (void)CBossCombatRuntime::Set_Flag(boss.BossCombat, SERVER_BOSS_COMBAT_FLAG::COUNTERABLE, false);
	for (const auto& [id, entityId] : state.BoundPlayers)
	{
		const auto found = players.find(id);
		if (found != players.end() && found->second.iNetEntityId == entityId &&
			found->second.iPatternBindOwnerNetEntityId == boss.iNetEntityId &&
			found->second.iPatternBindSequence == boss.iPatternSequence)
			found->second.Clear_PatternBindStatus();
	}
	state.BoundPlayers.clear();
	(void)players;
	if (BOSS_PATTERN_LOGIC_KIND::STAGGER_WINDOW == window.eKind)
	{
		boss.iKoukuItemStaggerMaximum = 0u;
		boss.iKoukuItemStaggerCredit = 0u;
		boss.bKoukuShieldActive = false;
		boss.fKoukuShieldArcDegrees = 0.f;
		boss.KoukuShieldRegions.clear();
	}
}

LostArk::Server::KOUKUSAYDON_LOGIC_ANSWER
LostArk::Server::CKoukuSaydonLogicRuntime::Judge_Roulette(
	const BOSS_PATTERN_LOGIC_WINDOW& window,
	const SERVER_WORLD_ENTITY& boss,
	const SERVER_PLAYER& player) noexcept
{
	using namespace LostArk::Shared;
	if (!window.CardRegions.empty())
	{
		const BOSS_LOGIC_REGION* matched = nullptr;
		for (const auto& region : window.CardRegions)
			if (Contains_LogicRegion(region, boss, player))
			{
				if (nullptr != matched) return KOUKUSAYDON_LOGIC_ANSWER::FAIL;
				matched = &region;
			}
		if (nullptr == matched) return KOUKUSAYDON_LOGIC_ANSWER::TIMEOUT;
		return matched->eCardSymbol == player.eMechanicCardSymbol &&
			matched->eCardColor == player.eMechanicCardColor ?
			KOUKUSAYDON_LOGIC_ANSWER::SUCCESS : KOUKUSAYDON_LOGIC_ANSWER::FAIL;
	}
	if (window.iSectorCount == 0u ||
		window.SectorSymbols.size() != window.iSectorCount ||
		MECHANIC_CARD_SYMBOL::NONE == player.eMechanicCardSymbol)
		return KOUKUSAYDON_LOGIC_ANSWER::FAIL;
	const float dx = player.fPositionX - window.fCenterX;
	const float dz = player.fPositionZ - window.fCenterZ;
	if (!std::isfinite(dx) || !std::isfinite(dz))
		return KOUKUSAYDON_LOGIC_ANSWER::FAIL;
	if (window.fOuterRadiusM > 0.f &&
		dx * dx + dz * dz > window.fOuterRadiusM * window.fOuterRadiusM)
		return KOUKUSAYDON_LOGIC_ANSWER::FAIL;
	const float degrees = static_cast<float>(std::atan2(dx, dz) * DEGREES_PER_RADIAN);
	const float relative = Wrap360(degrees - window.fStopYawDegrees);
	const float sectorSpan = 360.f / static_cast<float>(window.iSectorCount);
	std::size_t sector = static_cast<std::size_t>(relative / sectorSpan);
	if (sector >= window.iSectorCount)
		sector = window.iSectorCount - 1u;
	return window.SectorSymbols[sector] == player.eMechanicCardSymbol ?
		KOUKUSAYDON_LOGIC_ANSWER::SUCCESS : KOUKUSAYDON_LOGIC_ANSWER::FAIL;
}

LostArk::Server::KOUKUSAYDON_LOGIC_ANSWER
LostArk::Server::CKoukuSaydonLogicRuntime::Judge_Gaze(
	const BOSS_PATTERN_LOGIC_WINDOW& window,
	const SERVER_WORLD_ENTITY& boss,
	const SERVER_PLAYER& player) noexcept
{
	const float dx = boss.fPositionX - player.fPositionX;
	const float dz = boss.fPositionZ - player.fPositionZ;
	if (!std::isfinite(dx) || !std::isfinite(dz) || !std::isfinite(player.fYawDegrees))
		return KOUKUSAYDON_LOGIC_ANSWER::FAIL;
	const auto insideAnswer = window.bInsideIsFail ?
		KOUKUSAYDON_LOGIC_ANSWER::FAIL : KOUKUSAYDON_LOGIC_ANSWER::SUCCESS;
	const auto outsideAnswer = window.bInsideIsFail ?
		KOUKUSAYDON_LOGIC_ANSWER::SUCCESS : KOUKUSAYDON_LOGIC_ANSWER::FAIL;
	const float distanceSq = dx * dx + dz * dz;
	// Standing on the boss counts as facing it: no direction to judge.
	if (distanceSq < 0.01f)
		return insideAnswer;
	if (window.fMaxDistanceM > 0.f &&
		distanceSq > window.fMaxDistanceM * window.fMaxDistanceM)
		return outsideAnswer;
	const float toBoss = static_cast<float>(std::atan2(dx, dz) * DEGREES_PER_RADIAN);
	const float difference = Wrap180(toBoss - player.fYawDegrees);
	return std::fabs(difference) <= window.fHalfAngleDegrees ? insideAnswer : outsideAnswer;
}

bool LostArk::Server::CKoukuSaydonLogicRuntime::Is_ShieldReflected(
	const SERVER_WORLD_ENTITY& boss,
	const float sourceX,
	const float sourceZ) noexcept
{
	for (const auto& retained : boss.KoukuRetainedLogicOwners)
		if (const auto owner = retained.lock(); owner && Is_ShieldReflected(*owner, sourceX, sourceZ))
			return true;
	if (!boss.bKoukuShieldActive || boss.fKoukuShieldArcDegrees <= 0.f)
		return false;
	const auto reflects = [sourceX, sourceZ](const float centerX, const float centerZ,
		const float yaw, const float halfAngle, const float radiusX = 1.f, const float radiusZ = 1.f)
	{
		const float dx = sourceX - centerX, dz = sourceZ - centerZ;
		if (!std::isfinite(dx) || !std::isfinite(dz) || dx * dx + dz * dz < 0.0001f)
			return false;
		const float radians = yaw * .017453292519943295f;
		const float x = (std::cos(radians) * dx - std::sin(radians) * dz) / radiusX;
		const float z = (std::sin(radians) * dx + std::cos(radians) * dz) / radiusZ;
		return std::fabs(static_cast<float>(std::atan2(x, z) * DEGREES_PER_RADIAN)) <= halfAngle;
	};
	if (!boss.KoukuShieldRegions.empty())
	{
		const float radians = boss.fYawDegrees * 0.017453292519943295f;
		for (const auto& region : boss.KoukuShieldRegions)
		{
			// A ranged attacker is reflected by direction too; the sector radius
			// is its debug drawing extent, not an attack-distance restriction.
			if (!region.bSector || region.bReverseSector || region.eAnchor != BOSS_LOGIC_REGION_ANCHOR::BOSS_CURRENT)
				continue;
			const float centerX = boss.fPositionX + std::cos(radians) * region.fCenterX + std::sin(radians) * region.fCenterZ;
			const float centerZ = boss.fPositionZ - std::sin(radians) * region.fCenterX + std::cos(radians) * region.fCenterZ;
			if (reflects(centerX, centerZ, boss.fYawDegrees + region.fYawDegrees, region.fHalfAngleDegrees,
				region.fRadiusXM > 0.f ? region.fRadiusXM : region.fRadiusM,
				region.fRadiusZM > 0.f ? region.fRadiusZM : region.fRadiusM))
				return true;
		}
		return false;
	}
	return reflects(boss.fPositionX, boss.fPositionZ,
		boss.fYawDegrees + boss.fKoukuShieldNormalYawOffsetDegrees, boss.fKoukuShieldArcDegrees * 0.5f);
}

void LostArk::Server::CKoukuSaydonLogicRuntime::Transform_ToClown(
	SERVER_PLAYER& player,
	const BOSS_ENCOUNTER_MADNESS_POLICY* const pMadnessPolicy,
	const std::uint32_t serverTick,
	const std::uint32_t durationMs)
{
	using namespace LostArk::Shared;
	if (player.Is_Guide() || 0u == player.iCurrentHp || PLAYER_ACTION_STATE::DEAD == player.eAction)
		return;
	const std::uint32_t holdMs = 0u != durationMs ? durationMs :
		(nullptr != pMadnessPolicy && 0u != pMadnessPolicy->iClownHoldMs ?
			pMadnessPolicy->iClownHoldMs :
			static_cast<std::uint32_t>(DEFAULT_CLOWN_HOLD_MS));
	player.bKoukuPatternOwnsClown = false;
	player.eMadnessForm = PLAYER_MADNESS_FORM::CLOWN;
	player.iMadnessFormEndTick = Add_Ticks(serverTick, Ticks_FromMs(holdMs));
	player.iCurrentMadness = 0u; player.dMadnessRemainder = 0.;
}

bool LostArk::Server::CKoukuSaydonLogicRuntime::Update_PlayerFear(
    SERVER_PLAYER& player, const std::uint32_t serverTick)
{
    using namespace LostArk::Shared;
    if (PLAYER_ACTION_STATE::FEAR == player.eAction &&
        (Has_ReachedTick(serverTick, player.iFearEndTick) || player.iCurrentHp == 0u))
    {
        player.eAction = player.iCurrentHp == 0u ? PLAYER_ACTION_STATE::DEAD : PLAYER_ACTION_STATE::NONE;
        player.iActionStartTick = 0u;
        player.fActionElapsedSeconds = 0.f;
        player.PendingCommand.Clear();
    }
    if (PLAYER_ACTION_STATE::FEAR != player.eAction)
    {
        player.iFearEndTick = 0u;
        player.strFearPresentationId.clear();
        return false;
    }
    return true;
}

void LostArk::Server::CKoukuSaydonLogicRuntime::Apply_Result(
	SERVER_PLAYER& player,
	const BOSS_PATTERN_LOGIC_RESULT& result,
	const SERVER_WORLD_ENTITY& boss,
	const CGameplayCatalog& catalog,
	const BOSS_ENCOUNTER_MADNESS_POLICY* const pMadnessPolicy,
	const std::uint32_t serverTick,
	std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents,
	const std::array<float, 2u>* const pContactCenter, const bool encounterWipe)
{
	using namespace LostArk::Shared;
	switch (result.eKind)
	{
	case BOSS_PATTERN_LOGIC_RESULT_KIND::PLAYER_INVULNERABILITY:
	{
		if (!player.iCurrentHp || !result.iDurationMs) break;
		const auto until = Add_Ticks(serverTick, Ticks_FromMs(result.iDurationMs));
		if (!player.iInvulnerableEndTick || Has_ReachedTick(until, player.iInvulnerableEndTick)) player.iInvulnerableEndTick = until;
		break;
	}
	case BOSS_PATTERN_LOGIC_RESULT_KIND::INSTANT_DEATH:
	case BOSS_PATTERN_LOGIC_RESULT_KIND::MAX_HP_PERCENT_DAMAGE:
	case BOSS_PATTERN_LOGIC_RESULT_KIND::FIXED_DAMAGE:
	{
		if (result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::FIXED_DAMAGE && !result.iDamageAmount) break;
		SERVER_WORLD_TO_PLAYER_HIT hit{};
		hit.iRawDamage =
			BOSS_PATTERN_LOGIC_RESULT_KIND::INSTANT_DEATH == result.eKind ?
				(std::max)(1u, player.iCurrentHp) :
			BOSS_PATTERN_LOGIC_RESULT_KIND::FIXED_DAMAGE == result.eKind ? result.iDamageAmount :
				(std::max)(1u, Percent_Of(player.iMaximumHp, result.iPercent));
		hit.fSourceX = boss.fPositionX;
		hit.fSourceZ = boss.fPositionZ;
		hit.fPushRangeM = result.fPushRangeM;
		hit.iPushMs = result.iPushMs;
		hit.bForcePush = result.bForcePush;
		hit.bPushCanLeaveArena = result.bPushCanLeaveArena;
		hit.bPushBallistic = result.bPushBallistic;
		hit.fPushHeightM = result.fPushHeightM;
		if (result.ePushDirection == BOSS_LOGIC_PUSH_DIRECTION::AWAY_FROM_CONTACT)
		{
			if (!pContactCenter) break;
			hit.fSourceX = (*pContactCenter)[0];
			hit.fSourceZ = (*pContactCenter)[1];
		}
		if (result.ePushDirection == BOSS_LOGIC_PUSH_DIRECTION::BOSS_FORWARD)
		{
			hit.bUsePushDirection = true;
			const float radians = (boss.fYawDegrees + result.fPushYawOffsetDegrees) * 0.017453292519943295f;
			hit.fPushDirectionX = std::sin(radians);
			hit.fPushDirectionZ = std::cos(radians);
		}
		hit.iServerTick = serverTick;
		hit.bIgnoreDefense = true;
		hit.bIgnoreCounter = true;
		hit.bInstantDeath = result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::INSTANT_DEATH ||
			(result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::MAX_HP_PERCENT_DAMAGE && result.iPercent >= 100u);
		hit.bEncounterWipe = encounterWipe && hit.bInstantDeath;
		if (pContactCenter && Is_CurrentBossHandCapture(player, boss.iNetEntityId, boss.iPatternSequence, serverTick))
		{
			hit.iCaptureOwnerId = boss.iNetEntityId;
			hit.iCapturePatternSequence = boss.iPatternSequence;
		}
		(void)CServerCombatHitRuntime::Apply_WorldToPlayer(
			player, hit, catalog, outDamageEvents);
		break;
	}
	case BOSS_PATTERN_LOGIC_RESULT_KIND::MADNESS_GAUGE_ADD_PERCENT:
	{
		if (player.Is_Guide() || 0u == player.iMaximumMadness)
			break;
		const std::uint32_t gain = Percent_Of(player.iMaximumMadness, result.iPercent);
		const std::uint64_t total =
			static_cast<std::uint64_t>(player.iCurrentMadness) + gain;
		player.iCurrentMadness = static_cast<std::uint32_t>(
			(std::min<std::uint64_t>)(total, player.iMaximumMadness));
		if (player.iCurrentMadness >= player.iMaximumMadness)
			Transform_ToClown(player, pMadnessPolicy, serverTick, 0u);
		break;
	}
	case BOSS_PATTERN_LOGIC_RESULT_KIND::FEAR:
        if (!player.Has_TimeStop(serverTick) && !player.Has_HolyCharmProtection(serverTick) &&
            Can_ReceiveSpatialContact(player) && !player.bPatternBound &&
            PLAYER_ACTION_STATE::GRABBED != player.eAction &&
			PLAYER_ACTION_STATE::TRIGGER_MOVE != player.eAction &&
			PLAYER_ACTION_STATE::WALL_CLIMB != player.eAction &&
            PLAYER_ACTION_STATE::FEAR != player.eAction)
        {
            player.eAction = PLAYER_ACTION_STATE::FEAR;
            player.iActionStartTick = serverTick == 0u ? 1u : serverTick;
            player.iFearEndTick = Add_Ticks(player.iActionStartTick, Ticks_FromMs(result.iDurationMs));
            player.strFearPresentationId = result.strFearPresentationId;
            player.iCurrentSkillId = INVALID_SKILL_ID;
            player.Clear_SkillTarget();
            player.iComboStage = 0u;
            player.hasBufferedComboInput = false;
            player.hasReleasedHold = false;
            player.PendingCommand.Clear();
            player.hasMoveGoal = false;
            player.MovePath.clear();
            player.iMovePathIndex = 0u;
            player.fActionElapsedSeconds = 0.f;
            player.iKnockdownEndTick = 0u;
            player.fKnockbackRemainingSeconds = 0.f;
        }
        break;
	case BOSS_PATTERN_LOGIC_RESULT_KIND::CLOWN_TRANSFORM:
		Transform_ToClown(player, pMadnessPolicy, serverTick, result.iDurationMs);
		break;
	case BOSS_PATTERN_LOGIC_RESULT_KIND::FOLLOWUP_PATTERN:
	case BOSS_PATTERN_LOGIC_RESULT_KIND::PLAY_WORLD_OBJECT_MOTION:
	case BOSS_PATTERN_LOGIC_RESULT_KIND::PLAY_CONTACT_WORLD_OBJECT_MOTION:
	case BOSS_PATTERN_LOGIC_RESULT_KIND::COMPLETE_LOGIC_WINDOW:
	/* The grab needs the exact region that caught this player, which only the
	ENTER_AREA seam in Update knows, so it is applied there. */
	case BOSS_PATTERN_LOGIC_RESULT_KIND::GRAB_TO_WORLD_OBJECT:
	case BOSS_PATTERN_LOGIC_RESULT_KIND::NONE:
	default:
		break;
	}
}

bool LostArk::Server::CKoukuSaydonLogicRuntime::Can_EnterMarioEntry(
	const SERVER_PLAYER& player) noexcept
{
	return Is_Judgeable(player) && player.eMadnessForm == LostArk::Shared::PLAYER_MADNESS_FORM::CLOWN &&
		!player.iMarioStage && !player.TriggerMove.isActive &&
		!player.bPatternBound && !player.bArenaEjectionActive &&
		player.iAttachmentOwnerNetEntityId == LostArk::Shared::INVALID_NET_ENTITY_ID;
}

const LostArk::Server::BOSS_PATTERN_LOGIC_RESULT* LostArk::Server::CKoukuSaydonLogicRuntime::Find_MarioEntryResult(
	const BOSS_PATTERN_DEFINITION& pattern) noexcept
{
	for (const auto& window : pattern.LogicWindows)
		if (window.eKind == BOSS_PATTERN_LOGIC_KIND::ENTER_AREA && !window.CardRegions.empty() &&
			window.OnSuccess.size() == 1u && window.OnSuccess.front().eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::MARIO_ENTER)
			return &window.OnSuccess.front();
	return nullptr;
}

bool LostArk::Server::CKoukuSaydonLogicRuntime::Has_MarioEntry(
	const BOSS_PATTERN_DEFINITION& pattern) noexcept
{
	return Find_MarioEntryResult(pattern) != nullptr ||
		std::any_of(pattern.LogicWindows.begin(), pattern.LogicWindows.end(),
			[](const auto& window) { return window.eKind == BOSS_PATTERN_LOGIC_KIND::PATTERN_COMPLETION_COUNT; });
}

bool LostArk::Server::CKoukuSaydonLogicRuntime::Is_InsideMarioEntry(
	const BOSS_PATTERN_DEFINITION& root, const SERVER_WORLD_ENTITY& anchor,
	const SERVER_PLAYER& player, const std::uint32_t elapsedTicks, const bool honorWindowEnd, std::uint8_t* const matchedStage)
{
	using namespace LostArk::Shared::WorldCollision;
	if (!Can_EnterMarioEntry(player)) return false;
	// Movement stops one contact margin short of a blocking body. Admit the
	// player's body at that same boundary rather than waiting for its centre.
	const LostArk::Shared::CombatCollision::BODY_CIRCLE_XZ body{
		player.fPositionX, player.fPositionZ, PLAYER_HALF_EXTENT_X + CONTACT_MARGIN };
	for (const auto& window : root.LogicWindows)
	{
		if (window.eKind != BOSS_PATTERN_LOGIC_KIND::ENTER_AREA || elapsedTicks < Ticks_FromMs(window.iStartMs) ||
			(honorWindowEnd && elapsedTicks >= Ticks_FromMs(window.iStartMs + window.iDurationMs)) ||
			window.OnSuccess.size() != 1u || window.OnSuccess.front().eKind != BOSS_PATTERN_LOGIC_RESULT_KIND::MARIO_ENTER) continue;
		for (const auto& region : window.CardRegions)
		{
			// Mario admission outlives the pattern ledger; the room already owns
			// its frozen birth root in MarioEntryAnchor and passes it as anchor.
			auto sampled = region;
			if (sampled.eAnchor == BOSS_LOGIC_REGION_ANCHOR::BOSS_START)
				sampled.eAnchor = BOSS_LOGIC_REGION_ANCHOR::BOSS_CURRENT;
			LOGIC_REGION_TRANSFORM transform;
			if (!Resolve_LogicRegionTransform(sampled, anchor, elapsedTicks, transform) ||
				((transform.hasGrip || sampled.bCylinder || sampled.bLinearMotion) &&
				 std::abs(player.fPositionY + PLAYER_CENTER_OFFSET_Y - transform.centerY) >
					transform.halfY + PLAYER_HALF_EXTENT_Y + CONTACT_MARGIN)) continue;
			if (Intersects_LogicRegion(sampled, anchor, body, elapsedTicks))
			{
				if (matchedStage) *matchedStage = window.OnSuccess.front().iMarioEntryStage;
				return true;
			}
		}
	}
	return false;
}

void LostArk::Server::CKoukuSaydonLogicRuntime::Apply_Results(
	const std::vector<BOSS_PATTERN_LOGIC_RESULT>& results,
	KOUKUSAYDON_LOGIC_WINDOW_STATE& state,
	SERVER_PLAYER* const pPlayer,
	std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
	const SERVER_WORLD_ENTITY& boss,
	const CGameplayCatalog& catalog,
	const BOSS_ENCOUNTER_MADNESS_POLICY* const pMadnessPolicy,
	const std::uint32_t serverTick,
	std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents,
	KOUKUSAYDON_LOGIC_OUTPUT& outOutput,
	const std::set<LostArk::Shared::PLAYER_ID>& invulnerablePlayers,
	const std::array<float, 2u>* const pContactCenter,
	const bool spatialContact)
{
	const auto isDamage = [](const BOSS_PATTERN_LOGIC_RESULT& result) {
		return result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::INSTANT_DEATH ||
			result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::MAX_HP_PERCENT_DAMAGE ||
			result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::FIXED_DAMAGE;
	};
	const bool hasDamage = std::any_of(results.begin(), results.end(), isDamage);
	const bool hasAuthoredMadness = std::any_of(results.begin(), results.end(), [](const auto& result) {
		return result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::MADNESS_GAUGE_ADD_PERCENT;
	});
	const bool independentMadness = spatialContact && pContactCenter && state.iMadnessSource != 0u;
	std::set<LostArk::Shared::PLAYER_ID> damagedPlayers;
	// Ordinary explicit gauge follows landed HP damage. Ball/doll contact owns its
	// gauge independently of damage mitigation, shields and damage-blocking effects.
	// Explicit amounts still replace automatic HP-proportional gain for this verdict.
	std::vector<const BOSS_PATTERN_LOGIC_RESULT*> ordered;
	ordered.reserve(results.size());
	for (const auto& result : results)
		if (!hasDamage || result.eKind != BOSS_PATTERN_LOGIC_RESULT_KIND::MADNESS_GAUGE_ADD_PERCENT) ordered.push_back(&result);
	if (hasDamage) for (const auto& result : results)
		if (result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::MADNESS_GAUGE_ADD_PERCENT) ordered.push_back(&result);
	for (const auto* resultPointer : ordered)
	{
		const auto& result = *resultPointer;
		// A companion may touch a hazard but cannot answer or complete a raid mechanic.
		if (pPlayer && pPlayer->Is_Guide() && (!spatialContact ||
			(!isDamage(result) && result.eKind != BOSS_PATTERN_LOGIC_RESULT_KIND::FEAR &&
			 result.eKind != BOSS_PATTERN_LOGIC_RESULT_KIND::CAPTURE_PLAYER &&
			 result.eKind != BOSS_PATTERN_LOGIC_RESULT_KIND::GRAB_TO_WORLD_OBJECT))) continue;
        if (BOSS_PATTERN_LOGIC_RESULT_KIND::CAPTURE_PLAYER == result.eKind)
        {
            if (pPlayer && Can_ReceiveSpatialContact(*pPlayer) && pPlayer->eAction != LostArk::Shared::PLAYER_ACTION_STATE::FEAR && state.iHoldEndTick &&
                Has_ReachedTick(serverTick, state.iHoldStartTick) && !Has_ReachedTick(serverTick, state.iHoldEndTick) &&
                std::none_of(outOutput.CaptureRequests.begin(), outOutput.CaptureRequests.end(), [&](const auto& request) {
                    return request.iPlayerNetEntityId == pPlayer->iNetEntityId;
                }))
                outOutput.CaptureRequests.push_back({pPlayer->iNetEntityId, result.eAttachmentSlot, state.iHoldEndTick, state.iWindowIndex});
            continue;
        }
		if (BOSS_PATTERN_LOGIC_RESULT_KIND::PLAY_WORLD_OBJECT_MOTION == result.eKind)
		{
			if (state.AppliedWorldMotions.emplace(result.strTargetWorldInstanceId, result.strMotionInstanceId).second)
			{
				KOUKUSAYDON_LOGIC_WORLD_PLAY play;
				play.strInstanceId = result.strMotionInstanceId;
				play.strTargetSequenceInstanceId = result.strTargetWorldInstanceId;
				play.iStartTick = serverTick;
				outOutput.WorldSequencePlays.push_back(std::move(play));
			}
			continue;
		}
		if (BOSS_PATTERN_LOGIC_RESULT_KIND::FOLLOWUP_PATTERN == result.eKind)
		{
			if (!result.strPatternId.empty())
				outOutput.FollowupPatternIds.push_back(result.strPatternId);
			continue;
		}
		const bool lethal = result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::INSTANT_DEATH ||
			(result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::MAX_HP_PERCENT_DAMAGE && result.iPercent >= 100u);
		const auto apply = [&](SERVER_PLAYER& player) {
			// A solved same-pattern safe zone cancels its verdict before generic lethal-hit bypasses.
			if (invulnerablePlayers.contains(player.iPlayerId) &&
				(result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::INSTANT_DEATH ||
				 result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::MAX_HP_PERCENT_DAMAGE ||
				 result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::FIXED_DAMAGE ||
				 result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::FEAR)) return;
			if (result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::MADNESS_GAUGE_ADD_PERCENT &&
				((independentMadness && !Can_ReceiveSpatialContact(player)) ||
				 (hasDamage && ((!independentMadness && !damagedPlayers.contains(player.iPlayerId)) ||
				  !player.iCurrentHp || player.eMadnessForm != LostArk::Shared::PLAYER_MADNESS_FORM::NORMAL)))) return;
			const auto automaticGain = player.iMadnessDamageGainPercent;
			const auto hpBefore = player.iCurrentHp;
			const auto firstDamageEvent = outDamageEvents.size();
			if (hasAuthoredMadness && isDamage(result)) player.iMadnessDamageGainPercent = 0u;
			if (state.iMadnessSource && pMadnessPolicy && pMadnessPolicy->iSpecialIntervalMs &&
				result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::MADNESS_GAUGE_ADD_PERCENT)
			{
				const bool ball = state.iMadnessSource == 1u;
				const double gain = static_cast<double>(player.iMaximumMadness) *
					(ball ? pMadnessPolicy->iBallGainPercent : pMadnessPolicy->iDollGainPercent) / 100. *
					(ball ? pMadnessPolicy->iBallMultiplierPercent : pMadnessPolicy->iDollMultiplierPercent) / 100. *
					state.iMadnessContactIntervalMs / pMadnessPolicy->iSpecialIntervalMs;
				CServerCombatHitRuntime::Add_MadnessGauge(player, gain);
				if (player.iMaximumMadness && player.iCurrentMadness >= player.iMaximumMadness)
					Transform_ToClown(player, pMadnessPolicy, serverTick, 0u);
			}
			else Apply_Result(player, result, boss, catalog, pMadnessPolicy, serverTick, outDamageEvents, pContactCenter,
				!spatialContact && lethal);
			player.iMadnessDamageGainPercent = automaticGain;
			// Special gauge-only contact has no combat-text occurrence when shielded.
			// Preserve actual HP damage and every event from earlier results/targets.
			if (independentMadness && isDamage(result))
				outDamageEvents.erase(std::remove_if(outDamageEvents.begin() + firstDamageEvent, outDamageEvents.end(),
					[&](const auto& event) { return !event.isOutgoing && event.iTargetNetEntityId == player.iNetEntityId &&
						event.eHitFlag == LostArk::Shared::DAMAGE_HIT_FLAG::ABSORB; }), outDamageEvents.end());
			if (isDamage(result) && player.iCurrentHp < hpBefore) damagedPlayers.insert(player.iPlayerId);
		};
		if (nullptr != pPlayer)
		{
			apply(*pPlayer);
			continue;
		}
		/* Boss-level windows punish or reward the whole living raid. */
		for (auto& [playerId, player] : players)
		{
			(void)playerId;
			if (Is_Judgeable(player) || (!spatialContact && player.Is_Human() && player.iCurrentHp && lethal))
				apply(player);
		}
	}
}

// 현재 pattern clock을 사용한다. 반올림된 capture 시작 tick을 다시 빼면
// 30 Hz 경계에서 Client의 clip 시계와 어긋나므로 key 자체를 절대 pattern ms로 게시한다.
bool LostArk::Server::CKoukuSaydonLogicRuntime::Try_SampleCaptureHandLocalPosition(
	const BOSS_PATTERN_LOGIC_WINDOW& window, const double patternElapsedMs,
	std::array<float, 3u>& outLocal) noexcept
{
	const auto& keys = window.CaptureHandKeys;
	if (keys.empty() || !std::isfinite(patternElapsedMs) || patternElapsedMs < keys.front().iTimeMs)
		return false;
	const double time = (std::min)(patternElapsedMs, static_cast<double>(keys.back().iTimeMs));
	const auto next = std::upper_bound(keys.begin(), keys.end(), time,
		[](const double ms, const BOSS_CAPTURE_HAND_KEY& key) { return ms < key.iTimeMs; });
	const auto& left = next == keys.begin() ? keys.front() : *(next - 1);
	const auto& right = next == keys.end() ? left : *next;
	const float alpha = right.iTimeMs > left.iTimeMs ?
		static_cast<float>((time - left.iTimeMs) / (right.iTimeMs - left.iTimeMs)) : 0.f;
	std::array<float, 3u> staged{};
	for (std::size_t axis = 0u; axis < staged.size(); ++axis)
	{
		staged[axis] = left.LocalPosition[axis] + (right.LocalPosition[axis] - left.LocalPosition[axis]) * alpha;
		if (!std::isfinite(staged[axis])) return false;
	}
	outLocal = staged;
	return true;
}

bool LostArk::Server::CKoukuSaydonLogicRuntime::Apply_CaptureHandLocalPosition(
	SERVER_PLAYER& player, const SERVER_WORLD_ENTITY& boss, const std::array<float, 3u>& local) noexcept
{
	// D3D 행 벡터의 수평 회전: X'=cos*yaw*X+sin*yaw*Z, Z'=-sin*yaw*X+cos*yaw*Z.
	// 본 샘플에는 이미 모델 preScale과 grip의 right/up/forward 거리 보정이 포함되어 있다.
	const float yaw = boss.fYawDegrees * 0.017453292519943295f;
	const std::array<float, 3u> world{boss.fPositionX + std::cos(yaw) * local[0] + std::sin(yaw) * local[2],
		boss.fPositionY + local[1], boss.fPositionZ - std::sin(yaw) * local[0] + std::cos(yaw) * local[2]};
	for (const float value : world) if (!std::isfinite(value)) return false;
	player.fAttachmentLocalOffsetX = local[0];
	player.fAttachmentLocalOffsetY = local[1];
	player.fAttachmentLocalOffsetZ = local[2];
	player.fPositionX = world[0]; player.fPositionY = world[1]; player.fPositionZ = world[2];
	return true;
}

void LostArk::Server::CKoukuSaydonLogicRuntime::Update(
	SERVER_WORLD_ENTITY& boss,
	const BOSS_PATTERN_DEFINITION& pattern,
	KOUKUSAYDON_LOGIC_LEDGER& ledger,
	std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
	const CGameplayCatalog& catalog,
	const BOSS_ENCOUNTER_MADNESS_POLICY* const pMadnessPolicy,
	const std::uint32_t serverTick,
	std::vector<LostArk::Shared::DAMAGE_EVENT>& outDamageEvents,
	KOUKUSAYDON_LOGIC_OUTPUT& outOutput,
	const CServerNavigation* navigation, const CServerCollisionSystem* collision)
{
	if (!ledger.Is_Active() || ledger.strPatternId != pattern.strPatternId)
		return;
	// GameRoom은 이번 tick의 boss motion/root motion을 적용한 뒤 이 함수를 호출한다.
	// early Update_Players에서 이전 손 위치를 root로 복원했더라도 여기서 현재 pose로
	// 갱신한 다음 피해 영역을 판정한다. 다른 보스/다른 pattern 발생의 잡기는 건드리지 않는다.
	for (auto& [playerId, captured] : players)
	{
		(void)playerId;
		if (!Is_CurrentBossHandCapture(captured, boss.iNetEntityId, boss.iPatternSequence, serverTick) ||
			captured.iAttachmentWindowIndex >= pattern.LogicWindows.size()) continue;
		const auto& captureWindow = pattern.LogicWindows[captured.iAttachmentWindowIndex];
		if (captureWindow.CaptureHandKeys.empty()) continue;
		std::array<float, 3u> local{};
		if (!Try_SampleCaptureHandLocalPosition(captureWindow,
			static_cast<double>(serverTick - ledger.iPatternStartTick) * (1000.0 / 30.0), local) ||
			!Apply_CaptureHandLocalPosition(captured, boss, local))
			captured.iAttachmentEndTick = serverTick;
	}
	// Parent deadlines cancel a child before contact, charge, or end-tick judgement.
	for (auto& state : ledger.Windows)
	{
		if (state.bClosed || state.iWindowIndex >= pattern.LogicWindows.size()) continue;
		const auto& window = pattern.LogicWindows[state.iWindowIndex];
		if ((!window.strOwnerWorldOccurrenceId.empty() && ledger.RetiredWorldOccurrences.contains(window.strOwnerWorldOccurrenceId)) ||
			(window.bCancelAtEnd && Has_ReachedTick(serverTick, state.iEndTick)))
		{
			state.bChargeStopped = true;
			Close_Window(boss, window, state, players);
		}
	}
	/* Anyone already hanging rides the same authored region that caught them.
	The region is catalog data, so this keeps working after the window that
	judged them closed, and the moment the World Object's track hides the hook
	the deadline collapses to now and the room releases them. */
	for (auto& [hangingPlayerId, hanging] : players)
	{
		(void)hangingPlayerId;
		if (LostArk::Shared::PLAYER_ACTION_STATE::GRABBED != hanging.eAction ||
			LostArk::Shared::PLAYER_ATTACHMENT_SLOT::WORLD_HOOK_TIP != hanging.eAttachmentSlot ||
			hanging.iAttachmentOwnerNetEntityId != boss.iNetEntityId ||
			hanging.iAttachmentPatternSequence != boss.iPatternSequence ||
			hanging.iAttachmentWindowIndex >= pattern.LogicWindows.size())
			continue;
		const BOSS_PATTERN_LOGIC_WINDOW& carrier = pattern.LogicWindows[hanging.iAttachmentWindowIndex];
		if (hanging.iAttachmentRegionIndex >= carrier.CardRegions.size())
			continue;
		if (!Drag_HookedPlayer(hanging, carrier.CardRegions[hanging.iAttachmentRegionIndex],
			boss, serverTick - ledger.iPatternStartTick, serverTick, navigation))
			hanging.iAttachmentReleaseTick = serverTick;
	}
	for (KOUKUSAYDON_LOGIC_CUE_STATE& cue : ledger.MechanicTriggers)
	{
		if (cue.bStarted || cue.iIndex >= pattern.MechanicTriggers.size() ||
			!Has_ReachedTick(serverTick, cue.iStartTick))
			continue;
		cue.bStarted = true;
		const auto& trigger = pattern.MechanicTriggers[cue.iIndex];
		if (BOSS_PATTERN_MECHANIC_TRIGGER_KIND::HUD_ENTER == trigger.eKind)
			ledger.eHudMode = trigger.eHudMode;
		else if (BOSS_PATTERN_MECHANIC_TRIGGER_KIND::CARD_MAZE_STAGE_PLAYERS == trigger.eKind)
			(void)Stage_CardMazeEntryPlayers(trigger, ledger, players, navigation, collision, outOutput.strStatus);
		else if (BOSS_PATTERN_MECHANIC_TRIGGER_KIND::CARD_MAZE_HIDE_NEXT == trigger.eKind)
		{
			Capture_CardMazeEntryRoster(ledger, players);
			if (ledger.iCardMazeNextHiddenPlayer < ledger.CardMazeEntryPlayers.size())
			{
				const auto found = players.find(ledger.CardMazeEntryPlayers[ledger.iCardMazeNextHiddenPlayer++]);
				if (found != players.end() && Is_Judgeable(found->second))
					found->second.CardMaze.flags |= LostArk::Shared::CARD_MAZE_ENTRY_HIDDEN;
			}
		}
		else if (BOSS_PATTERN_MECHANIC_TRIGGER_KIND::CARD_MAZE_ENTER == trigger.eKind)
			(void)Enter_CardMaze(trigger, ledger, players, navigation, collision, outOutput.strStatus);
		else
		{
			auto scheduled = trigger;
			if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::CROSS_DIRECTION_CLONES &&
				Has_ReachedTick(serverTick, Add_Ticks(ledger.iPatternStartTick, Ticks_FromMs(trigger.iStartMs + trigger.iDurationMs))))
				continue;
			if (pattern.bFixedTimelineClock &&
				(trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::REAL_GAZE_TELEPORT ||
				 trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::SUMMON_PATTERNS))
			{
				const auto deadline = Add_Ticks(ledger.iPatternStartTick, Ticks_FromMs(trigger.iStartMs + trigger.iDurationMs));
				if (Has_ReachedTick(serverTick, deadline)) continue;
				const auto remainingTicks = deadline >= serverTick ? deadline - serverTick :
					(std::numeric_limits<std::uint32_t>::max)() - serverTick + deadline;
				// Clone expiry is relative to actual spawn; preserve the absolute Parent deadline.
				scheduled.iDurationMs = static_cast<std::uint32_t>(static_cast<std::uint64_t>(remainingTicks) * 1000u / SERVER_TICK_HZ);
			}
			outOutput.MechanicTriggers.push_back(std::move(scheduled));
		}
	}
	for (KOUKUSAYDON_LOGIC_CUE_STATE& cue : ledger.WorldSequences)
	{
		if (cue.bStarted || cue.iIndex >= pattern.WorldSequences.size() ||
			!Has_ReachedTick(serverTick, cue.iStartTick))
			continue;
		cue.bStarted = true;
		const BOSS_PATTERN_WORLD_SEQUENCE& sequence = pattern.WorldSequences[cue.iIndex];
		KOUKUSAYDON_LOGIC_WORLD_PLAY play;
		play.strInstanceId = sequence.strInstanceId; play.fPlaybackSpeed = sequence.fPlaybackSpeed;
		play.iDurationMs = sequence.iDurationMs; play.strOccurrenceId = sequence.strOccurrenceId; play.iStartTick = cue.iStartTick;
		play.Placement = sequence.Placement;
        const auto positionOffset = ledger.WorldPositionOffsets.find(sequence.strOccurrenceId);
        if (play.Placement && positionOffset != ledger.WorldPositionOffsets.end())
        {
            // packet에 실리는 Placement도 접촉 판정과 같은 이번 실행의 이동값을 사용한다.
            play.Placement->fPositionX += positionOffset->second[0u];
            play.Placement->fPositionY += positionOffset->second[1u];
            play.Placement->fPositionZ += positionOffset->second[2u];
        }
		play.CombatBody = sequence.CombatBody;
		play.bAuthoredMadness = sequence.bAuthoredMadness;
		if (!play.Placement)
		{
			play.fPositionOffsetX = sequence.fPositionOffsetX + (sequence.bAnchorBossSpawn ? boss.fSpawnPositionX - sequence.fAnchorPositionX : 0.f);
			play.fPositionOffsetY = sequence.fPositionOffsetY + (sequence.bAnchorBossSpawn ? boss.fSpawnPositionY - sequence.fAnchorPositionY : 0.f);
			play.fPositionOffsetZ = sequence.fPositionOffsetZ + (sequence.bAnchorBossSpawn ? boss.fSpawnPositionZ - sequence.fAnchorPositionZ : 0.f);
		}
		outOutput.WorldSequencePlays.push_back(std::move(play));
	}
    // Capture a charge once at trigger entry, then sample the existing boss-motion
    // path before the body-follow contact regions inspect this tick's position.
    for (auto& state : ledger.Windows)
    {
        if (state.bClosed || state.iWindowIndex >= pattern.LogicWindows.size() ||
            !Has_ReachedTick(serverTick, state.iStartTick)) continue;
        const auto& window = pattern.LogicWindows[state.iWindowIndex];
        if (window.fBossChargeDistanceM <= 0.f || state.bChargeStopped) continue;
        if (!state.bChargeCaptured)
        {
            state.bChargeCaptured = true;
            std::vector<const SERVER_PLAYER*> candidates;
            for (const auto& [id, player] : players)
                if (Is_Judgeable(player)) candidates.push_back(&player);
            if (candidates.empty())
            { state.bChargeStopped = true; outOutput.strStatus = "boss charge has no living target"; continue; }
            const auto* target = candidates[Mix(static_cast<std::uint64_t>(boss.iPatternSequence) << 32u ^
                boss.iNetEntityId ^ state.iStartTick) % candidates.size()];
            const float dx = target->fPositionX - boss.fPositionX, dz = target->fPositionZ - boss.fPositionZ;
            const float length = std::sqrt(dx * dx + dz * dz);
            if (!std::isfinite(length) || length <= .0001f)
            { state.bChargeStopped = true; outOutput.strStatus = "boss charge target has no direction"; continue; }
            BOSS_PATTERN_BOSS_MOTION motion;
            motion.iStartMs = window.iStartMs; motion.iEndMs = window.iStartMs + window.iDurationMs;
            motion.StartPosition = {boss.fPositionX, boss.fPositionY, boss.fPositionZ};
            motion.EndPosition = {boss.fPositionX + dx / length * window.fBossChargeDistanceM,
                boss.fPositionY, boss.fPositionZ + dz / length * window.fBossChargeDistanceM};
            // The authored clip may face a different native axis; its body yaw
            // must not rotate the captured world-space travel vector.
            motion.fYawDegrees = static_cast<float>(std::atan2(dx, dz) * DEGREES_PER_RADIAN) + window.fChargeYawOffsetDegrees;
            state.ChargeMotion = motion;
            boss.iTargetEntityId = target->iNetEntityId;
            boss.iPatternTargetEntityId = target->iNetEntityId;
            boss.bHasPatternTargetLastPosition = true;
            boss.fPatternTargetLastPositionX = target->fPositionX;
            boss.fPatternTargetLastPositionY = target->fPositionY;
            boss.fPatternTargetLastPositionZ = target->fPositionZ;
        }
        if (!state.ChargeMotion) continue;
        const auto& motion = *state.ChargeMotion;
        const auto position = CKoukuSaydonBrain::Sample_BossMotion(motion, serverTick - ledger.iPatternStartTick);
        SERVER_NAV_POINT destination{position[0], position[1], position[2]};
        bool navigationBlocked = false;
        if (navigation && (!navigation->Has_LineOfSight(boss.fPositionX, boss.fPositionZ, destination.x, destination.z) ||
            !navigation->Resolve_TraversalStep(boss.fPositionX, boss.fPositionZ, destination.x, destination.z, destination)))
        {
            // Endpoint heights alone can skip a blocked cell or clip its corner.
            // Stop at the same last navigable point used by native root-motion recoil.
            const double dx = double(destination.x) - boss.fPositionX, dz = double(destination.z) - boss.fPositionZ;
            const double length = std::hypot(dx, dz);
            double low = 0., high = 1.;
            bool found = false;
            SERVER_NAV_POINT clamped{};
            for (unsigned iteration = 0u; iteration < 40u && (high - low) * length > .001; ++iteration)
            {
                const double middle = (low + high) * .5;
                const float x = static_cast<float>(boss.fPositionX + dx * middle);
                const float z = static_cast<float>(boss.fPositionZ + dz * middle);
                SERVER_NAV_POINT candidate{};
                if (navigation->Has_LineOfSight(boss.fPositionX, boss.fPositionZ, x, z) &&
                    navigation->Resolve_TraversalStep(boss.fPositionX, boss.fPositionZ, x, z, candidate))
                { low = middle; clamped = candidate; found = true; }
                else high = middle;
            }
            if (!found || low * length < .001)
            { state.bChargeStopped = true; outOutput.strStatus = "boss charge stopped at navigation boundary"; continue; }
            destination = clamped;
            navigationBlocked = true;
        }
        bool blocked = false;
        if (collision && !collision->Resolve_CircleMove(boss.fPositionX, boss.fPositionY, boss.fPositionZ,
            destination.x, destination.y, destination.z, boss.fCollisionRadius, boss.fCollisionRadius,
            boss.fCollisionRadius, destination.x, destination.y, destination.z, blocked, boss.iNetEntityId, false))
        { state.bChargeStopped = true; outOutput.strStatus = "boss charge collision resolution failed"; continue; }
        if (navigation && (!navigation->Has_LineOfSight(boss.fPositionX, boss.fPositionZ, destination.x, destination.z) ||
            !navigation->Resolve_TraversalStep(boss.fPositionX, boss.fPositionZ, destination.x, destination.z, destination)))
        { state.bChargeStopped = true; outOutput.strStatus = "boss charge collision destination is not navigable"; continue; }
        boss.fPositionX = destination.x; boss.fPositionY = destination.y; boss.fPositionZ = destination.z;
        boss.fYawDegrees = motion.fYawDegrees;
        state.bChargeStopped = navigationBlocked || blocked || Has_ReachedTick(serverTick, state.iEndTick);
        if (navigationBlocked) outOutput.strStatus = "boss charge stopped at navigation boundary";
        else if (blocked) outOutput.strStatus = "boss charge stopped at collision boundary";
    }
	// Fixed Effect anchors can be born before their colliders. Capture on the
	// same Server clock before the later window opens, retaining one immutable basis.
	for (auto& state : ledger.Windows)
		if (!state.bClosed && state.iWindowIndex < pattern.LogicWindows.size())
			Capture_StartRegions(boss, pattern.LogicWindows[state.iWindowIndex], state,
				serverTick - ledger.iPatternStartTick);
	// Gather protection before any result, independently of authored window order.
	// Protection belongs to this pattern execution; only presentation pulses form a room-wide union.
	std::set<LostArk::Shared::PLAYER_ID> invulnerablePlayers;
	for (const auto& state : ledger.Windows)
	{
		if (state.bClosed || state.iWindowIndex >= pattern.LogicWindows.size() ||
			!Has_ReachedTick(serverTick, state.iStartTick) ||
			(Has_ReachedTick(serverTick, state.iEndTick) && serverTick != state.iEndTick)) continue;
		const auto& window = pattern.LogicWindows[state.iWindowIndex];
		if (window.eKind != BOSS_PATTERN_LOGIC_KIND::INVULNERABILITY_ZONE || window.iThreshold > 4u) continue;
		for (const auto& region : window.CardRegions)
		{
			std::vector<LostArk::Shared::PLAYER_ID> occupants;
			for (const auto& [playerId, player] : players)
				if (Is_Judgeable(player) && Contains_LogicRegion(Window_Region(region, state), boss, player,
					serverTick - ledger.iPatternStartTick)) occupants.push_back(playerId);
			// Every Collider owns its exact count; overlapping valid regions form a union.
			if (window.iThreshold == 0u || occupants.size() == window.iThreshold)
				invulnerablePlayers.insert(occupants.begin(), occupants.end());
		}
	}
	for (const auto playerId : invulnerablePlayers)
	{
		auto& player = players.at(playerId);
		const bool continuous = player.iInvulnerabilityZoneContactTick != 0u &&
			(player.iInvulnerabilityZoneContactTick == serverTick ||
			 Add_Ticks(player.iInvulnerabilityZoneContactTick, 1u) == serverTick);
		if (!continuous || player.iInvulnerabilityZonePulseTick == 0u ||
			Has_ReachedTick(serverTick, Add_Ticks(player.iInvulnerabilityZonePulseTick, SERVER_TICK_HZ * 2u)))
			player.iInvulnerabilityZonePulseTick = serverTick;
		player.iInvulnerabilityZoneContactTick = serverTick;
	}
	// Resolve all contacts before any duration timeout, including the final tick.
	// A group's priority order is fixed at Build; one card consumes one result per strike.
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
                const auto positionOffset = ledger.WorldPositionOffsets.find(target.strWorldOccurrenceId);
                const float offsetX = positionOffset == ledger.WorldPositionOffsets.end() ? 0.f : positionOffset->second[0u];
                const float offsetZ = positionOffset == ledger.WorldPositionOffsets.end() ? 0.f : positionOffset->second[2u];
                // 저작 target 중심에 구워진 template local offset을 유지한 채 카드와 같은 평행이동을 적용한다.
                const LostArk::Shared::CombatCollision::BODY_CIRCLE_XZ circle{
                    target.fWorldX + offsetX, target.fWorldZ + offsetZ, target.fRadiusM};
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
	}
	for (KOUKUSAYDON_LOGIC_WINDOW_STATE& state : ledger.Windows)
	{
		if (state.bClosed || state.iWindowIndex >= pattern.LogicWindows.size())
			continue;
		const BOSS_PATTERN_LOGIC_WINDOW& window = pattern.LogicWindows[state.iWindowIndex];
		if (!state.bOpened)
		{
			if (!Has_ReachedTick(serverTick, state.iStartTick))
				continue;
			Open_Window(boss, window, state, ledger, players);
		}
		const bool reachedEnd = Has_ReachedTick(serverTick, state.iEndTick);
		switch (window.eKind)
		{
		case BOSS_PATTERN_LOGIC_KIND::BINGO_COMPLETED_LINES:
			if (ledger.iBingoLineJudgementTick && Has_ReachedTick(ledger.iBingoLineJudgementTick, state.iStartTick) &&
				!Has_ReachedTick(ledger.iBingoLineJudgementTick, state.iEndTick) && Has_ReachedTick(serverTick, ledger.iBingoLineJudgementTick))
			{
				const bool passed = ledger.iBingoCompletedLines >= window.iThreshold;
				auto results = passed ? window.OnSuccess : window.OnFail;
				if (passed && ledger.bBingoLineRewardApplied)
					std::erase_if(results, [](const auto& result) {
						return result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::PLAYER_INVULNERABILITY; });
				for (auto& [id, player] : players)
					if (player.iCurrentHp && player.eAction != LostArk::Shared::PLAYER_ACTION_STATE::DEAD)
					{
						Apply_Results(results, state, &player, players, boss, catalog, pMadnessPolicy,
							serverTick, outDamageEvents, outOutput, invulnerablePlayers);
						if (passed) for (const auto& result : results)
							if (result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::PLAYER_INVULNERABILITY && result.iDurationMs)
							{
								const auto until = Add_Ticks(serverTick, Ticks_FromMs(result.iDurationMs));
								if (!player.iKoukuBingoLineProtectionEndTick || Has_ReachedTick(until, player.iKoukuBingoLineProtectionEndTick))
									player.iKoukuBingoLineProtectionEndTick = until;
							}
					}
				outOutput.BingoLineCompletion = passed;
				Close_Window(boss, window, state, players);
			}
			else if (reachedEnd) Close_Window(boss, window, state, players);
			break;
		case BOSS_PATTERN_LOGIC_KIND::BOSS_DAMAGE_REDUCTION:
		case BOSS_PATTERN_LOGIC_KIND::INVULNERABILITY_ZONE:
			if (reachedEnd) Close_Window(boss, window, state, players);
			break;
		case BOSS_PATTERN_LOGIC_KIND::OBJECT_CONTACT:
			break; // Contact phase above owns the complete short window.
		case BOSS_PATTERN_LOGIC_KIND::ATTACHMENT_HOLD:
            if (reachedEnd) Close_Window(boss, window, state, players);
            break;
		case BOSS_PATTERN_LOGIC_KIND::EXTERNAL_SIGNAL:
			if (reachedEnd)
			{
				Close_Window(boss, window, state, players);
				Apply_Results(window.OnTimeout, state, nullptr, players, boss, catalog, pMadnessPolicy,
					serverTick, outDamageEvents, outOutput, invulnerablePlayers);
				outOutput.strStatus = "external signal window timed out";
			}
			break;
		case BOSS_PATTERN_LOGIC_KIND::CARD_DICE_BIND:
			if (reachedEnd) Close_Window(boss, window, state, players);
			else boss.iTargetEntityId = boss.iPatternTargetEntityId = state.iFreePlayerNetEntityId;
			break;
        case BOSS_PATTERN_LOGIC_KIND::COUNTER_WINDOW:
        {
            auto& signals = boss.BossCombat.PendingOutcomes;
            const auto counter = std::find_if(signals.begin(), signals.end(), [&](const auto& signal) {
                return signal.iPatternSequence == ledger.iPatternSequence &&
                    signal.strPatternId == boss.strPatternId &&
                    signal.eOutcome == BOSS_PATTERN_STAGE_OUTCOME::COUNTER_HIT &&
                    Has_ReachedTick(signal.iServerTick, state.iStartTick) &&
                    !Has_ReachedTick(signal.iServerTick, Add_Ticks(state.iEndTick, 1u));
            });
            if (counter != signals.end())
            {
                float landingY = boss.fPositionY;
                if (window.bEndsPatternOnSuccess && !Try_CounterLanding(boss, navigation, collision, landingY))
                {
                    // The hit was already applied once by combat. Keep only its signal
                    // pending; retrying the landing must never reapply damage or stagger.
                    if (reachedEnd)
                    {
                        signals.erase(counter);
                        Close_Window(boss, window, state, players);
                        Apply_Results(window.OnTimeout, state, nullptr, players, boss, catalog,
                            pMadnessPolicy, serverTick, outDamageEvents, outOutput, invulnerablePlayers);
                    }
                    outOutput.strStatus = reachedEnd ? "counter window timed out: landing rejected by navigation or collision" :
                        "counter landing rejected by navigation or collision";
                    break;
                }
                if (window.bEndsPatternOnSuccess)
                {
                    boss.fPositionY = landingY;
                    outOutput.bCounterSuccessLanded = true;
                }
                signals.erase(counter);
                Close_Window(boss, window, state, players);
                Apply_Results(window.OnSuccess, state, nullptr, players, boss, catalog,
                    pMadnessPolicy, serverTick, outDamageEvents, outOutput, invulnerablePlayers);
                if (window.bEndsPatternOnSuccess) outOutput.bEndPatternEarly = true;
                outOutput.strStatus = "counter window succeeded";
            }
            else if (reachedEnd)
            {
                Close_Window(boss, window, state, players);
                Apply_Results(window.OnTimeout, state, nullptr, players, boss, catalog,
                    pMadnessPolicy, serverTick, outDamageEvents, outOutput, invulnerablePlayers);
                outOutput.strStatus = "counter window timed out";
            }
            break;
        }
		case BOSS_PATTERN_LOGIC_KIND::STAGGER_WINDOW:
		{
			const auto maximum = catalog.Get_RaidStaggerMaximum() ? catalog.Get_RaidStaggerMaximum() : window.iThreshold;
			boss.iKoukuItemStaggerMaximum = maximum;
			const std::uint64_t progress = boss.iKoukuItemStaggerCredit;
			if (maximum > 0u && progress >= maximum)
			{
				Close_Window(boss, window, state, players);
				Apply_Results(window.OnSuccess, state, nullptr, players, boss, catalog,
					pMadnessPolicy, serverTick, outDamageEvents, outOutput, invulnerablePlayers);
				if (window.bEndsPatternOnSuccess)
				{
					outOutput.bEndPatternEarly = true;
					outOutput.bStaggerSuccess = true;
				}
				// Existing skill HP contribution and item-only stagger credit share this window.
				// Publish its one-shot success without inventing damage or a contributor.
				if (outDamageEvents.size() < LostArk::Shared::MAX_DAMAGE_EVENTS)
				{
					LostArk::Shared::DAMAGE_EVENT success{};
					success.iTargetNetEntityId = boss.iNetEntityId;
					success.fPositionX = boss.fPositionX; success.fPositionY = boss.fPositionY; success.fPositionZ = boss.fPositionZ;
					success.isOutgoing = true; success.isStaggerSuccess = true;
					outDamageEvents.push_back(success);
				}
				outOutput.strStatus = "stagger window succeeded";
			}
			else if (reachedEnd)
			{
				Close_Window(boss, window, state, players);
				Apply_Results(window.OnTimeout, state, nullptr, players, boss, catalog,
					pMadnessPolicy, serverTick, outDamageEvents, outOutput, invulnerablePlayers);
				outOutput.strStatus = "stagger window timed out";
			}
			break;
		}
		case BOSS_PATTERN_LOGIC_KIND::ROULETTE_CARD_MATCH:
		case BOSS_PATTERN_LOGIC_KIND::GAZE_REAL_BOSS:
		{
            if (window.bGazeDuringWindow)
            {
                // Use the same player-facing judgement as the end-tick gaze, within [start, end).
                if (reachedEnd) { Close_Window(boss, window, state, players); break; }
                for (auto& [playerId, player] : players)
                {
                    if ((!Is_Judgeable(player) && !(player.Is_Human() && player.iCurrentHp && Has_LethalResult(window.OnFail))) ||
                        state.Answers[playerId] == KOUKUSAYDON_LOGIC_ANSWER::FAIL ||
                        Judge_Gaze(window, boss, player) != KOUKUSAYDON_LOGIC_ANSWER::FAIL) continue;
                    state.Answers[playerId] = KOUKUSAYDON_LOGIC_ANSWER::FAIL;
                    Apply_Results(window.OnFail, state, &player, players, boss, catalog, pMadnessPolicy,
                        serverTick, outDamageEvents, outOutput, invulnerablePlayers);
                }
                break;
            }
			if (!reachedEnd)
				break;
			/* Judge before the cards go back so the card that was dealt is the
			card that is compared. */
			std::vector<std::pair<LostArk::Shared::PLAYER_ID, KOUKUSAYDON_LOGIC_ANSWER>> verdicts;
			for (auto& [playerId, player] : players)
			{
				if (!player.Is_Human() || !player.iCurrentHp) continue;
				const KOUKUSAYDON_LOGIC_ANSWER answer =
					BOSS_PATTERN_LOGIC_KIND::ROULETTE_CARD_MATCH == window.eKind ?
						Judge_Roulette(window, boss, player) : Judge_Gaze(window, boss, player);
                const auto& results = answer == KOUKUSAYDON_LOGIC_ANSWER::SUCCESS ? window.OnSuccess :
                    answer == KOUKUSAYDON_LOGIC_ANSWER::FAIL ? window.OnFail : window.OnTimeout;
                if (!Is_Judgeable(player) && !Has_LethalResult(results)) continue;
				state.Answers[playerId] = answer;
				verdicts.emplace_back(playerId, answer);
			}
			Close_Window(boss, window, state, players);
			for (const auto& [playerId, answer] : verdicts)
			{
				const auto found = players.find(playerId);
				if (players.end() == found)
					continue;
				Apply_Results(
					KOUKUSAYDON_LOGIC_ANSWER::SUCCESS == answer ? window.OnSuccess :
					(KOUKUSAYDON_LOGIC_ANSWER::FAIL == answer ? window.OnFail : window.OnTimeout),
					state, &found->second, players, boss, catalog, pMadnessPolicy, serverTick,
					outDamageEvents, outOutput, invulnerablePlayers);
			}
			outOutput.strStatus = "end-tick window judged";
			break;
		}
		case BOSS_PATTERN_LOGIC_KIND::OBJECT_OVERLAP:
		{
			const LostArk::Shared::CombatCollision::BODY_CIRCLE_XZ target{
				window.fTargetWorldX, window.fTargetWorldZ, window.fTargetRadiusM };
			const bool inside = !window.strTargetWorldInstanceId.empty() &&
				std::any_of(window.CardRegions.begin(), window.CardRegions.end(),
					[&](const BOSS_LOGIC_REGION& region) {
						return Intersects_LogicRegion(Window_Region(region, state), boss, target, serverTick - ledger.iPatternStartTick);
					});
			if (inside)
			{
				Close_Window(boss, window, state, players);
				Apply_Results(window.bInsideIsFail ? window.OnFail : window.OnSuccess,
					state, nullptr, players, boss, catalog, pMadnessPolicy, serverTick, outDamageEvents, outOutput, invulnerablePlayers);
				outOutput.strStatus = "object overlap window judged";
			}
			else if (reachedEnd)
			{
				Close_Window(boss, window, state, players);
				Apply_Results(window.OnTimeout, state, nullptr, players, boss, catalog,
					pMadnessPolicy, serverTick, outDamageEvents, outOutput, invulnerablePlayers);
				outOutput.strStatus = "object overlap window timed out";
			}
			break;
		}
		case BOSS_PATTERN_LOGIC_KIND::AREA_OVERLAP:
		case BOSS_PATTERN_LOGIC_KIND::ENTER_AREA:
		{
			const bool enter = BOSS_PATTERN_LOGIC_KIND::ENTER_AREA == window.eKind;
			// Repeating contact emits only inside [birth, expiry), matching the visible Collider lifetime.
			if (window.iRepeatIntervalMs && reachedEnd)
			{ Close_Window(boss, window, state, players); break; }
			// Mario admission and its party-wide failure are committed by the room.
			// Other participants must not time out merely because one player entered.
			if (enter && window.OnSuccess.size() == 1u &&
				window.OnSuccess.front().eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::MARIO_ENTER)
			{
				if (reachedEnd) Close_Window(boss, window, state, players);
				break;
			}
			if (!enter && !window.iRepeatIntervalMs && !reachedEnd) break;
			const auto harmful = [](const auto& results) {
				return std::any_of(results.begin(), results.end(), [](const auto& result) {
					return result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::INSTANT_DEATH ||
						result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::MAX_HP_PERCENT_DAMAGE ||
						result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::FIXED_DAMAGE;
				});
			};
			const auto& contactResults = window.bInsideIsFail ? window.OnFail : window.OnSuccess;
			const bool lethalContact = std::any_of(contactResults.begin(), contactResults.end(), [](const auto& result) {
				return result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::INSTANT_DEATH ||
					(result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::MAX_HP_PERCENT_DAMAGE && result.iPercent >= 100u);
			});
			const bool captureContact = enter && harmful(contactResults);
			for (auto& [playerId, player] : players)
			{
				const bool ownedCapture = captureContact && Is_CurrentBossHandCapture(
					player, boss.iNetEntityId, boss.iPatternSequence, serverTick);
                const bool receivesContact = !player.Has_TimeStop(serverTick) && (Can_ReceiveSpatialContact(player) || ownedCapture);
                const bool lethalTimeout = reachedEnd && player.Is_Human() && Has_LethalResult(window.OnTimeout);
				if (!player.iCurrentHp || (!lethalContact && !receivesContact && !lethalTimeout) || (enter && !window.bRearmOnExit && !window.bRepeatAfterKnockback && !window.iRepeatIntervalMs &&
					KOUKUSAYDON_LOGIC_ANSWER::SUCCESS == state.Answers[playerId]))
					continue;
				const auto caught = std::find_if(window.CardRegions.begin(), window.CardRegions.end(),
					[&](const BOSS_LOGIC_REGION& region) {
						const auto elapsed = serverTick - ledger.iPatternStartTick;
						const auto& sampledRegion = Window_Region(region, state);
						return Contains_LogicRegion(sampledRegion, boss, player, elapsed) ||
							(enter && (Swept_LinearColliderContainsPlayer(sampledRegion, boss, player, elapsed,
								state.iStartTick - ledger.iPatternStartTick, state.iEndTick - ledger.iPatternStartTick) ||
								Swept_CenteredWorldCircleContainsPlayer(sampledRegion, boss, player, elapsed,
								state.iStartTick - ledger.iPatternStartTick, state.iEndTick - ledger.iPatternStartTick) ||
                                Swept_WorldBoxContainsPlayer(sampledRegion, boss, player, elapsed,
                                    state.iStartTick - ledger.iPatternStartTick, state.iEndTick - ledger.iPatternStartTick)));
					});
				const bool inside = window.CardRegions.end() != caught;
                // A timeout exception may reach this point solely for an outside verdict.
                if (inside && !lethalContact && !receivesContact) continue;
				if (player.Is_Guide() && !inside)
                { state.InsidePlayers.erase(playerId); continue; }
				const bool captureCandidate = inside && enter && !window.OnSuccess.empty() &&
					(window.OnSuccess.front().eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::CAPTURE_PLAYER ||
					 window.OnSuccess.front().eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::GRAB_TO_WORLD_OBJECT);
				if (window.iRepeatIntervalMs)
				{
					if (inside)
					{
						const auto next = state.NextContactHitTicks.find(playerId);
						if (next != state.NextContactHitTicks.end() && !Has_ReachedTick(serverTick, next->second)) continue;
					}
					else if (const auto previous = state.Answers.find(playerId); previous != state.Answers.end() &&
						previous->second != KOUKUSAYDON_LOGIC_ANSWER::NONE) continue;
				}
				if (enter && window.bRepeatAfterKnockback)
				{
					if (inside)
					{
						const auto next = state.NextContactHitTicks.find(playerId);
						if (!CPlayerSkillSystem::Can_ArmPlayerHitReaction(player, serverTick, window.OnSuccess.front().bForcePush) ||
							(next != state.NextContactHitTicks.end() && !Has_ReachedTick(serverTick, next->second))) continue;
					}
					else
					{
						const auto answer = state.Answers.find(playerId);
						if (answer != state.Answers.end() && answer->second != KOUKUSAYDON_LOGIC_ANSWER::NONE) continue;
					}
				}
				if (enter && window.bRearmOnExit)
				{
					if (inside)
					{
						if (captureCandidate)
						{
							if (state.InsidePlayers.contains(playerId)) continue;
						}
						else if (!state.InsidePlayers.insert(playerId).second) continue;
					}
					else
					{
						state.InsidePlayers.erase(playerId);
						// Timeout belongs only to players who never entered this window.
						const auto answer = state.Answers.find(playerId);
						if (answer != state.Answers.end() && answer->second != KOUKUSAYDON_LOGIC_ANSWER::NONE) continue;
					}
				}
				if (!inside && !reachedEnd) continue;
				// Capture is committed by the room; an ineligible candidate remains retryable.
				if (!captureCandidate)
					state.Answers[playerId] = inside ? (window.bInsideIsFail ? KOUKUSAYDON_LOGIC_ANSWER::FAIL :
						KOUKUSAYDON_LOGIC_ANSWER::SUCCESS) : KOUKUSAYDON_LOGIC_ANSWER::TIMEOUT;
				const std::vector<BOSS_PATTERN_LOGIC_RESULT>& answered = inside ?
					(window.bInsideIsFail ? window.OnFail : window.OnSuccess) : window.OnTimeout;
				LOGIC_REGION_TRANSFORM contactTransform{};
				std::array<float, 2u> contactCenter{};
				const bool hasContactCenter = inside && Resolve_LogicRegionTransform(Window_Region(*caught, state), boss,
					serverTick - ledger.iPatternStartTick, contactTransform, false);
				if (hasContactCenter) contactCenter = { contactTransform.centerX, contactTransform.centerZ };
				Apply_Results(answered, state, &player, players,
					boss, catalog, pMadnessPolicy, serverTick, outDamageEvents, outOutput, invulnerablePlayers,
					hasContactCenter ? &contactCenter : nullptr, true);
				if (inside && window.iRepeatIntervalMs)
					state.NextContactHitTicks[playerId] = Add_Ticks(serverTick, Ticks_FromMs(window.iRepeatIntervalMs));
				if (inside && enter && window.bRepeatAfterKnockback && !answered.empty())
					state.NextContactHitTicks[playerId] = Add_Ticks(serverTick, Ticks_FromMs(answered.front().iPushMs));
				/* Only this seam knows which region caught this player, and that
				region is the thing that will carry them, so the grab happens here
				rather than in the typed result switch. */
				if (inside && std::any_of(answered.begin(), answered.end(),
					[](const BOSS_PATTERN_LOGIC_RESULT& result)
					{ return BOSS_PATTERN_LOGIC_RESULT_KIND::GRAB_TO_WORLD_OBJECT == result.eKind; }))
					if (Hang_PlayerOnRegion(player, *caught, boss,
						serverTick - ledger.iPatternStartTick, state.iWindowIndex,
						static_cast<std::uint32_t>(caught - window.CardRegions.begin()),
						(!caught->WorldTrack.Keys.empty() && caught->WorldTrack.Keys.front().bHasGripPosition) ?
                            Add_Ticks(ledger.iPatternStartTick, Ticks_FromMs(caught->WorldTrack.iStartMs + caught->WorldTrack.iDurationMs)) :
                            state.iEndTick, serverTick, navigation))
					{
						state.Answers[playerId] = KOUKUSAYDON_LOGIC_ANSWER::SUCCESS;
						state.InsidePlayers.insert(playerId);
					}
			}
			if (reachedEnd) Close_Window(boss, window, state, players);
			break;
		}
		case BOSS_PATTERN_LOGIC_KIND::POSE_INPUT:
		{
			if (!reachedEnd)
				break;
			std::vector<std::pair<LostArk::Shared::PLAYER_ID, KOUKUSAYDON_LOGIC_ANSWER>> verdicts;
			for (auto& [playerId, player] : players)
			{
				if (!player.Is_Human() || !player.iCurrentHp) continue;
				const auto answer = state.Answers.find(playerId);
                const auto value = state.Answers.end() == answer ? KOUKUSAYDON_LOGIC_ANSWER::NONE : answer->second;
                const auto& results = value == KOUKUSAYDON_LOGIC_ANSWER::SUCCESS ? window.OnSuccess :
                    value == KOUKUSAYDON_LOGIC_ANSWER::FAIL ? window.OnFail : window.OnTimeout;
                if (!Is_Judgeable(player) && !Has_LethalResult(results)) continue;
				verdicts.emplace_back(playerId, value);
			}
			Close_Window(boss, window, state, players);
			for (const auto& [playerId, answer] : verdicts)
			{
				const auto found = players.find(playerId);
				if (players.end() == found)
					continue;
				const std::vector<BOSS_PATTERN_LOGIC_RESULT>& results =
					KOUKUSAYDON_LOGIC_ANSWER::SUCCESS == answer ? window.OnSuccess :
					(KOUKUSAYDON_LOGIC_ANSWER::FAIL == answer ? window.OnFail : window.OnTimeout);
				Apply_Results(results, state, &found->second, players, boss, catalog,
					pMadnessPolicy, serverTick, outDamageEvents, outOutput, invulnerablePlayers);
			}
			outOutput.strStatus = "pose window judged";
			break;
		}
		default:
			if (reachedEnd)
				Close_Window(boss, window, state, players);
			break;
		}
	}
	// 잡기의 상태 commit은 GameRoom이 담당한다. 그 직후 같은 tick의 손 좌표를 적용할
	// 값만 요청에 복사하여 객체/본 포인터 수명이나 Client pose에 의존하지 않는다.
	for (auto& request : outOutput.CaptureRequests)
	{
		if (request.iWindowIndex >= pattern.LogicWindows.size()) continue;
		request.bHasHandLocalPosition = Try_SampleCaptureHandLocalPosition(
			pattern.LogicWindows[request.iWindowIndex],
			static_cast<double>(serverTick - ledger.iPatternStartTick) * (1000.0 / 30.0), request.HandLocalPosition);
	}
}

bool LostArk::Server::CKoukuSaydonLogicRuntime::Record_InteractionSlot(
	KOUKUSAYDON_LOGIC_LEDGER& ledger,
	const BOSS_PATTERN_DEFINITION& pattern,
	const SERVER_PLAYER& player,
	const LostArk::Shared::INTERACTION_SLOT slot,
	std::string& outStatus)
{
	using namespace LostArk::Shared;
	if (!ledger.Is_Active() || ledger.strPatternId != pattern.strPatternId)
	{
		outStatus = "no running pattern owns a dance window";
		return false;
	}
	if (!Is_Valid_InteractionSlot(slot))
	{
		outStatus = "slot is out of range";
		return false;
	}
	const std::int8_t pose = player.ModeSkillIndexBySlot[static_cast<std::size_t>(slot)];
	if (pose < 0)
	{
		outStatus = "empty slot";
		return false;
	}
	for (KOUKUSAYDON_LOGIC_WINDOW_STATE& state : ledger.Windows)
	{
		if (!state.bOpened || state.bClosed ||
			state.iWindowIndex >= pattern.LogicWindows.size())
			continue;
		const BOSS_PATTERN_LOGIC_WINDOW& window = pattern.LogicWindows[state.iWindowIndex];
		if (BOSS_PATTERN_LOGIC_KIND::POSE_INPUT != window.eKind)
			continue;
		auto answer = state.Answers.find(player.iPlayerId);
		if (state.Answers.end() == answer)
			answer = state.Answers.emplace(player.iPlayerId, KOUKUSAYDON_LOGIC_ANSWER::NONE).first;
		if (KOUKUSAYDON_LOGIC_ANSWER::NONE != answer->second)
		{
			outStatus = "already answered";
			return false;
		}
		answer->second = static_cast<std::uint32_t>(pose) == window.iPoseIndex ?
			KOUKUSAYDON_LOGIC_ANSWER::SUCCESS : KOUKUSAYDON_LOGIC_ANSWER::FAIL;
		outStatus = KOUKUSAYDON_LOGIC_ANSWER::SUCCESS == answer->second ?
			"pose matched" : "pose mismatched";
		return true;
	}
	outStatus = "no open dance window";
	return false;
}

void LostArk::Server::CKoukuSaydonLogicRuntime::Update_PlayerModes(
	std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
	const KOUKUSAYDON_LOGIC_LEDGER* const pActiveLedger,
	const BOSS_ENCOUNTER_MADNESS_POLICY* const pMadnessPolicy,
	const std::uint32_t serverTick)
{
	using namespace LostArk::Shared;
	KOUKU_HUD_MODE ledgerMode = nullptr != pActiveLedger && pActiveLedger->Is_Active() ?
		pActiveLedger->eHudMode : KOUKU_HUD_MODE::NONE;
	if (ledgerMode == KOUKU_HUD_MODE::NONE && pActiveLedger && pActiveLedger->Is_Active() &&
		std::any_of(pActiveLedger->ScheduledDanceIntervals.begin(), pActiveLedger->ScheduledDanceIntervals.end(),
			[&](const auto& interval) { return Has_ReachedTick(serverTick, interval.first) && !Has_ReachedTick(serverTick, interval.second); }))
		ledgerMode = KOUKU_HUD_MODE::DANCE;
	for (auto& [playerId, player] : players)
	{
		(void)playerId;
		if (player.Is_Guide())
		{
			player.Clear_KoukuInteractionState();
			player.Clear_KoukuAssignedCard();
			player.eMadnessForm = PLAYER_MADNESS_FORM::NORMAL;
			player.iCurrentMadness = player.iMadnessDamageGainPercent = 0u;
			player.dMadnessRemainder = 0.;
			continue;
		}
		const KOUKU_HUD_MODE patternMode = nullptr != pActiveLedger &&
			0u != player.iKoukuSuppressedPatternSequence &&
			player.iKoukuSuppressedPatternSequence == pActiveLedger->iPatternSequence ?
			KOUKU_HUD_MODE::NONE : ledgerMode;
		player.iMadnessDamageGainPercent = pMadnessPolicy ? pMadnessPolicy->iDamageGainPercent : 0u;
		if (!pMadnessPolicy) player.dMadnessRemainder = 0.;
		if (nullptr != pMadnessPolicy && 0u != pMadnessPolicy->iMaximum)
		{
			player.iMaximumMadness = pMadnessPolicy->iMaximum;
			player.iCurrentMadness = (std::min)(player.iCurrentMadness, player.iMaximumMadness);
			if (player.iCurrentMadness >= player.iMaximumMadness)
				Transform_ToClown(player, pMadnessPolicy, serverTick, 0u);
		}
		if (PLAYER_MADNESS_FORM::CLOWN == player.eMadnessForm &&
			0u != player.iMadnessFormEndTick &&
			Has_ReachedTick(serverTick, player.iMadnessFormEndTick))
		{
			if (KOUKU_HUD_MODE::NONE == patternMode &&
				KOUKU_HUD_MODE::NONE == player.eKoukuAreaHudMode &&
				KOUKU_HUD_MODE::NONE == player.eDebugKoukuHudModeOverride)
			{
				player.eMadnessForm = PLAYER_MADNESS_FORM::NORMAL;
				player.iMadnessFormEndTick = 0u;
			}
			// Retain an expired hold through Mario so leaving the mode restores
			// Normal on the next tick instead of creating a permanent clown.
		}
		for (std::int8_t& index : player.ModeSkillIndexBySlot)
			index = KOUKU_HUD_SLOT_EMPTY;
		const bool alive = 0u != player.iCurrentHp && PLAYER_ACTION_STATE::DEAD != player.eAction;
		if (!alive)
		{
			// The area's HUD presentation also applies to dead spectators. Keep
			// its mode while clearing every actionable slot and polymorph state.
			const auto areaMode = player.eKoukuAreaHudMode == KOUKU_HUD_MODE::MAZE ?
				KOUKU_HUD_MODE::MAZE : KOUKU_HUD_MODE::NONE;
			player.Clear_KoukuInteractionState();
			player.eKoukuAreaHudMode = areaMode;
			player.eKoukuHudMode = patternMode == KOUKU_HUD_MODE::DANCE ? patternMode : areaMode;
			player.eMadnessForm = PLAYER_MADNESS_FORM::NORMAL;
			continue;
		}
		KOUKU_HUD_MODE mode = KOUKU_HUD_MODE::NONE != patternMode ?
			patternMode : (KOUKU_HUD_MODE::NONE != player.eKoukuAreaHudMode ?
				player.eKoukuAreaHudMode : player.eDebugKoukuHudModeOverride);
		if (KOUKU_HUD_MODE::NONE != patternMode &&
			PLAYER_MADNESS_FORM::NORMAL == player.eMadnessForm)
		{
			player.bKoukuPatternOwnsClown = true;
			player.eMadnessForm = PLAYER_MADNESS_FORM::CLOWN;
		}
		else if (KOUKU_HUD_MODE::NONE == patternMode && player.bKoukuPatternOwnsClown)
		{
			player.bKoukuPatternOwnsClown = false;
			if (KOUKU_HUD_MODE::NONE == mode && 0u == player.iMadnessFormEndTick)
				player.eMadnessForm = PLAYER_MADNESS_FORM::NORMAL;
		}
		if (KOUKU_HUD_MODE::NONE == mode && PLAYER_MADNESS_FORM::CLOWN == player.eMadnessForm)
			mode = KOUKU_HUD_MODE::POLYMORPH;
		// A mode change cannot reinterpret an interaction animation already in flight.
		if (player.eKoukuHudMode != mode && (PLAYER_ACTION_STATE::INTERACTION == player.eAction ||
			PLAYER_ACTION_STATE::SKILL == player.eAction))
		{
			player.eAction = PLAYER_ACTION_STATE::NONE;
			player.iCurrentSkillId = INVALID_SKILL_ID;
			player.Clear_SkillTarget();
			player.iComboStage = 0u;
			player.hasBufferedComboInput = false;
			player.iActionStartTick = 0u;
			player.fActionElapsedSeconds = 0.f;
		}
		player.eKoukuHudMode = mode;
		std::uint32_t count = 0u;
		switch (mode)
		{
		case KOUKU_HUD_MODE::DANCE: count = 4u; break;
		case KOUKU_HUD_MODE::POLYMORPH: count = 3u; break;
		case KOUKU_HUD_MODE::MARIO: count = 2u; break;
		case KOUKU_HUD_MODE::MAZE: count = 2u; break;
		case KOUKU_HUD_MODE::NONE: break;
		default: break;
		}
		for (std::uint32_t index = 0u; index < count; ++index)
			player.ModeSkillIndexBySlot[index] = static_cast<std::int8_t>(index);
	}
}

namespace
{
	constexpr LostArk::Shared::MECHANIC_CARD_SYMBOL CARD_MAZE_SUITS[4] = {
		LostArk::Shared::MECHANIC_CARD_SYMBOL::HEART,
		LostArk::Shared::MECHANIC_CARD_SYMBOL::SPADE,
		LostArk::Shared::MECHANIC_CARD_SYMBOL::CLUB,
		LostArk::Shared::MECHANIC_CARD_SYMBOL::DIAMOND,
	};

	/* Server-owned xorshift seeded from the claim tick and claimant, so a
	deal is reproducible from the room's own clock and nothing else. */
	struct CARD_MAZE_RANDOM final
	{
		std::uint64_t iState;
		std::uint32_t Next() noexcept
		{
			iState ^= iState << 13;
			iState ^= iState >> 7;
			iState ^= iState << 17;
			return static_cast<std::uint32_t>(iState >> 32);
		}
		float Unit() noexcept
		{
			return static_cast<float>(Next() & 0xFFFFFFu) / 16777215.f;
		}
	};

	float DistanceXZ(const float ax, const float az, const float bx, const float bz) noexcept
	{
		const float dx = ax - bx;
		const float dz = az - bz;
		return std::sqrt(dx * dx + dz * dz);
	}
}

void LostArk::Server::CKoukuBingoRuntime::Promote_Lines() noexcept
{
	using namespace LostArk::Shared;
	for (std::int32_t line = 0; line < KOUKU_BINGO_SIDE * 2; ++line)
	{
		const std::uint32_t lineMask = Kouku_BingoLineMask(line);
		if (0u != lineMask && lineMask == (m_iWhiteMask & lineMask))
			m_iRedMask |= lineMask;
	}
}

void LostArk::Server::CKoukuBingoRuntime::Fill(const std::uint32_t cellMask) noexcept
{
	using namespace LostArk::Shared;
	m_iWhiteMask |= cellMask & KOUKU_BINGO_ALL_CELLS_MASK;
	Promote_Lines();
}

void LostArk::Server::CKoukuBingoRuntime::Detonate(const std::uint32_t cellMask) noexcept
{
	using namespace LostArk::Shared;
	/* A finished bingo is never disturbed, so red cells drop out first and
	the rest is decided against the board as it stands right now. */
	const std::uint32_t affected =
		(cellMask & KOUKU_BINGO_ALL_CELLS_MASK) & ~m_iRedMask;
	m_iWhiteMask ^= affected;
	Promote_Lines();
}

std::uint32_t LostArk::Server::CKoukuBingoRuntime::Count_CompletedRowsAndColumns() const noexcept
{
	std::uint32_t count = 0u;
	for (std::uint32_t line = 0u; line < LostArk::Shared::KOUKU_BINGO_SIDE * 2u; ++line)
	{
		const auto mask = LostArk::Shared::Kouku_BingoLineMask(line);
		if ((m_iRedMask & mask) == mask) ++count;
	}
	return count;
}

std::uint32_t LostArk::Server::CKoukuBingoRuntime::Count_UnconsumedRowsAndColumns() const noexcept
{
	std::uint32_t count = 0u;
	for (std::uint32_t line = 0u; line < LostArk::Shared::KOUKU_BINGO_SIDE * 2u; ++line)
	{
		const auto mask = LostArk::Shared::Kouku_BingoLineMask(line);
		if (!(m_iConsumedLineMask & (1u << line)) && (m_iRedMask & mask) == mask) ++count;
	}
	return count;
}

bool LostArk::Server::CKoukuBingoRuntime::Consume_CompletedRowsAndColumns(const std::uint32_t count) noexcept
{
	if (!count || Count_UnconsumedRowsAndColumns() < count) return false;
	auto remaining = count;
	for (std::uint32_t line = 0u; line < LostArk::Shared::KOUKU_BINGO_SIDE * 2u && remaining; ++line)
	{
		const auto bit = 1u << line;
		const auto mask = LostArk::Shared::Kouku_BingoLineMask(line);
		if (!(m_iConsumedLineMask & bit) && (m_iRedMask & mask) == mask)
		{ m_iConsumedLineMask |= bit; --remaining; }
	}
	return true;
}

bool LostArk::Server::CKoukuBingoRuntime::Start_Bomb(
	const LostArk::Shared::NET_ENTITY_ID carrier,
	const std::uint32_t detonateTick, const std::uint32_t markOrdinal,
	const std::uint32_t dropDelayMs, const std::uint32_t fuseMs) noexcept
{
	using namespace LostArk::Shared;
	if (INVALID_NET_ENTITY_ID == carrier)
		return false;
	std::size_t freeSlot = m_Bombs.size();
	for (std::size_t slot = 0u; slot < m_Bombs.size(); ++slot)
	{
		const BOMB& bomb = m_Bombs[slot];
		if (BINGO_BOMB_PHASE::NONE == bomb.ePhase && !bomb.iPlantTick)
		{
			if (freeSlot == m_Bombs.size())
				freeSlot = slot;
			continue;
		}
		if (BINGO_BOMB_PHASE::MARKED == bomb.ePhase &&
			bomb.iCarrierNetEntityId == carrier)
		{
			return false;
		}
	}
	if (freeSlot == m_Bombs.size())
		return false;
	BOMB& started = m_Bombs[freeSlot];
	started = {};
	started.ePhase = BINGO_BOMB_PHASE::MARKED;
	started.iCarrierNetEntityId = carrier;
	started.iDetonateTick = detonateTick;
	started.iMarkOrdinal = markOrdinal;
	started.iDropDelayMs = dropDelayMs;
	started.iFuseMs = fuseMs;
	return true;
}

void LostArk::Server::CKoukuBingoRuntime::Plant_Bomb(
	const std::size_t slot, const float x, const float z,
	const std::uint32_t fuseTick) noexcept
{
	using namespace LostArk::Shared;
	if (slot >= m_Bombs.size() ||
		(BINGO_BOMB_PHASE::MARKED != m_Bombs[slot].ePhase && !m_Bombs[slot].iPlantTick))
	{
		return;
	}
	BOMB& bomb = m_Bombs[slot];
	bomb.ePhase = BINGO_BOMB_PHASE::PLANTED;
	bomb.iCarrierNetEntityId = INVALID_NET_ENTITY_ID;
	bomb.fPositionX = x;
	bomb.fPositionZ = z;
	bomb.iDetonateTick = fuseTick;
	bomb.iPlantTick = 0u;
}

void LostArk::Server::CKoukuBingoRuntime::Queue_BombPlant(
	const std::size_t slot, const float x, const float z, const std::uint32_t plantTick) noexcept
{
	using namespace LostArk::Shared;
	if (slot >= m_Bombs.size() || m_Bombs[slot].ePhase != BINGO_BOMB_PHASE::MARKED || !plantTick) return;
	auto& bomb = m_Bombs[slot];
	bomb.ePhase = BINGO_BOMB_PHASE::NONE;
	bomb.iCarrierNetEntityId = INVALID_NET_ENTITY_ID;
	bomb.fPositionX = x; bomb.fPositionZ = z;
	bomb.iPlantTick = plantTick; bomb.iDetonateTick = 0u;
}

void LostArk::Server::CKoukuBingoRuntime::Clear_Bomb(
	const std::size_t slot) noexcept
{
	if (slot < m_Bombs.size())
		m_Bombs[slot] = {};
}

bool LostArk::Server::CKoukuBingoRuntime::Start_Hammer(
	const std::int32_t anchor, const std::uint32_t startTick,
	const std::uint32_t endTick) noexcept
{
	using namespace LostArk::Shared;
	if (BINGO_HAMMER_PHASE::NONE != m_Hammer.ePhase ||
		!Is_KoukuBingoHammerAnchor(anchor) || endTick <= startTick)
	{
		return false;
	}
	m_Hammer.iAnchor = anchor;
	m_Hammer.ePhase = BINGO_HAMMER_PHASE::RAISED;
	m_Hammer.iPhaseStartTick = startTick;
	m_Hammer.iPhaseEndTick = endTick;
	return true;
}

bool LostArk::Server::CKoukuBingoRuntime::Advance_Hammer(
	const std::uint32_t tick) noexcept
{
	using namespace LostArk::Shared;
	if (BINGO_HAMMER_PHASE::NONE == m_Hammer.ePhase)
		return false;
	if (tick < m_Hammer.iPhaseEndTick)
		return true;
	/* Each phase hands its end tick to the next one, so the three windows
	stay contiguous however late this tick arrives. */
	const auto ticksFor = [](const std::uint32_t milliseconds) noexcept
	{
		return (milliseconds * 30u + 999u) / 1000u;
	};
	m_Hammer.iPhaseStartTick = m_Hammer.iPhaseEndTick;
	if (BINGO_HAMMER_PHASE::RAISED == m_Hammer.ePhase)
	{
		m_Hammer.ePhase = BINGO_HAMMER_PHASE::DESCENDING;
		m_Hammer.iPhaseEndTick +=
			ticksFor(KOUKU_BINGO_HAMMER_DESCEND_MS);
		return true;
	}
	if (BINGO_HAMMER_PHASE::DESCENDING == m_Hammer.ePhase)
	{
		m_Hammer.ePhase = BINGO_HAMMER_PHASE::SWEEPING;
		m_Hammer.iPhaseEndTick +=
			ticksFor(KOUKU_BINGO_HAMMER_SWEEP_MS);
		return true;
	}
	m_Hammer = {};
	return false;
}

bool LostArk::Server::CKoukuBingoRuntime::Is_Safe(
	const float x, const float z) const noexcept
{
	using namespace LostArk::Shared;
	const std::int32_t cell = Kouku_BingoCellAt(x, z);
	if (!Is_KoukuBingoCell(cell))
		return false;
	return 0u != (m_iRedMask & (1u << cell));
}

const char* LostArk::Server::CKoukuCardMazeRuntime::Archetype_ForSuit(
	const LostArk::Shared::MECHANIC_CARD_SYMBOL suit) noexcept
{
	using namespace LostArk::Shared;
	switch (suit)
	{
	case MECHANIC_CARD_SYMBOL::HEART: return "MONSTER_KOUKU_CARD_HEART";
	case MECHANIC_CARD_SYMBOL::SPADE: return "MONSTER_KOUKU_CARD_SPADE";
	case MECHANIC_CARD_SYMBOL::CLUB: return "MONSTER_KOUKU_CARD_CLUB";
	case MECHANIC_CARD_SYMBOL::DIAMOND: return "MONSTER_KOUKU_CARD_DIAMOND";
	default: return "";
	}
}

bool LostArk::Server::CKoukuCardMazeRuntime::Plan(
	const LostArk::Shared::PLAYER_ID claimantId,
	const std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
	const CServerNavigation& navigation,
	const std::uint32_t serverTick,
	std::vector<SPAWN_REQUEST>& outSpawns,
	std::string& outStatus)
{
	using namespace LostArk::Shared;
	outSpawns.clear();
	m_PendingParticipants.clear();
	m_iPendingTelescopeOwner = INVALID_PLAYER_ID;
	const auto claimant = players.find(claimantId);
	if (players.end() == claimant || claimant->second.Is_Guide() || 0u == claimant->second.iCurrentHp)
	{
		outStatus = "Card maze claimant is not a living player";
		return false;
	}
	if (PHASE::HUNTING == m_ePhase)
	{
		outStatus = "Card maze is already running";
		return false;
	}
	/* Hunters are every other living player, in PlayerId order so the same
	room deals the same way whatever the map iteration happens to be. */
	std::vector<PLAYER_ID> hunters;
	for (const auto& [playerId, player] : players)
	{
		if (player.Is_Human() && playerId != claimantId && 0u != player.iCurrentHp)
			hunters.push_back(playerId);
	}
	if (hunters.size() > 3u)
		hunters.resize(3u);
	// F1 and normal entry use the same single-player rehearsal in both builds.
	// Dead members of a multiplayer room must not silently change its role rules.
	const bool soloTest = std::count_if(players.begin(), players.end(),
        [](const auto& entry) { return entry.second.Is_Human(); }) == 1;
	if (hunters.empty() && !soloTest)
	{
		outStatus = "Card maze needs at least one other living player to hunt";
		return false;
	}
	CARD_MAZE_RANDOM random{
		Mix((static_cast<std::uint64_t>(serverTick) << 32) ^ static_cast<std::uint64_t>(claimantId)) | 1ull };
	MECHANIC_CARD_SYMBOL deck[4] = {
		CARD_MAZE_SUITS[0], CARD_MAZE_SUITS[1], CARD_MAZE_SUITS[2], CARD_MAZE_SUITS[3] };
	for (std::uint32_t index = 3u; index > 0u; --index)
	{
		const std::uint32_t swapWith = random.Next() % (index + 1u);
		std::swap(deck[index], deck[swapWith]);
	}
	PARTICIPANT owner{};
	owner.eRole = CARD_MAZE_ROLE::TELESCOPE;
	// Multiplayer telescope owners remain suitless; solo is a hunter with overhead access.
	if (soloTest)
	{
		owner.eRole = CARD_MAZE_ROLE::HUNTER;
		owner.eSuit = deck[0];
	}
	std::vector<std::pair<PLAYER_ID, MECHANIC_CARD_SYMBOL>> dealt;
	if (soloTest) dealt.emplace_back(claimantId, owner.eSuit);
	for (std::size_t hunterIndex = 0u; hunterIndex < hunters.size(); ++hunterIndex)
		dealt.emplace_back(hunters[hunterIndex], deck[hunterIndex]);
	m_PendingParticipants[claimantId] = owner;
	std::vector<std::pair<float, float>> taken;
	for (const auto& [dealtId, dealtSuit] : dealt)
	{
		if (dealtId != claimantId)
		{
			PARTICIPANT hunter{};
			hunter.eRole = CARD_MAZE_ROLE::HUNTER;
			hunter.eSuit = dealtSuit;
			m_PendingParticipants[dealtId] = hunter;
		}
		for (std::uint32_t target = 0u; target < TARGETS_PER_SUIT; ++target)
		{
			bool placed = false;
			for (std::uint32_t attempt = 0u; attempt < SAMPLE_ATTEMPTS && !placed; ++attempt)
			{
				/* A random point in the maze rectangle projected onto the
				nearest walkable cell: the CardMiro region is 0.5 m, so the
				result is a corridor cell, never the top of a card wall. */
				const float sampleX = MAZE_MIN_X + (MAZE_MAX_X - MAZE_MIN_X) * random.Unit();
				const float sampleZ = MAZE_MIN_Z + (MAZE_MAX_Z - MAZE_MIN_Z) * random.Unit();
				SERVER_NAV_POINT point{};
				if (!navigation.Project_Point(sampleX, sampleZ, point) ||
					point.x < MAZE_MIN_X || point.x > MAZE_MAX_X ||
					point.z < MAZE_MIN_Z || point.z > MAZE_MAX_Z ||
					std::abs(point.y + .01f) > .5f ||
					DistanceXZ(point.x, point.z, CENTER_X, CENTER_Z) < CENTER_KEEPOUT_M)
				{
					continue;
				}
				bool clear = true;
				for (const auto& [playerId, player] : players)
				{
					(void)playerId;
					if (0u != player.iCurrentHp &&
						DistanceXZ(point.x, point.z, player.fPositionX, player.fPositionZ) < PLAYER_KEEPOUT_M)
					{
						clear = false;
						break;
					}
				}
				for (std::size_t takenIndex = 0u; clear && takenIndex < taken.size(); ++takenIndex)
				{
					if (DistanceXZ(point.x, point.z, taken[takenIndex].first, taken[takenIndex].second) < SPAWN_SPACING_M)
						clear = false;
				}
				if (!clear)
					continue;
				std::vector<SERVER_NAV_POINT> route;
				if (!navigation.Find_Path(CENTER_X, CENTER_Z, point.x, point.z, route) || route.empty() ||
					DistanceXZ(route.back().x, route.back().z, point.x, point.z) > .75f) continue;
				SPAWN_REQUEST request{};
				request.eSuit = dealtSuit;
				request.fPositionX = point.x;
				request.fPositionY = point.y;
				request.fPositionZ = point.z;
				request.fYawDegrees = 360.f * random.Unit();
				outSpawns.push_back(request);
				taken.emplace_back(point.x, point.z);
				placed = true;
			}
			if (!placed)
			{
				outStatus = "Card maze found no free corridor cell for a target";
				m_PendingParticipants.clear();
				outSpawns.clear();
				return false;
			}
		}
	}
	m_iPendingTelescopeOwner = claimantId;
	outStatus = "Card maze planned";
	return true;
}

void LostArk::Server::CKoukuCardMazeRuntime::Register_Target(
	const LostArk::Shared::NET_ENTITY_ID entityId,
	const LostArk::Shared::MECHANIC_CARD_SYMBOL suit)
{
	m_Targets[entityId] = suit;
}

void LostArk::Server::CKoukuCardMazeRuntime::Commit(
	std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players)
{
	m_Participants = m_PendingParticipants;
	m_iTelescopeOwner = m_iPendingTelescopeOwner;
	m_PendingParticipants.clear();
	m_iPendingTelescopeOwner = LostArk::Shared::INVALID_PLAYER_ID;
	m_CountedKills.clear();
	m_bMarchStarted = false;
	m_ePhase = PHASE::HUNTING;
	for (const auto& [playerId, participant] : m_Participants)
	{
		const auto player = players.find(playerId);
		if (players.end() != player)
		{
			player->second.CardMaze = {};
			player->second.eKoukuAreaHudMode = LostArk::Shared::KOUKU_HUD_MODE::MAZE;
			player->second.CardMaze.flags = playerId == m_iTelescopeOwner ? 1u : 0u;
			Apply_ToPlayer(participant, player->second);
		}
	}
}

void LostArk::Server::CKoukuCardMazeRuntime::Abort()
{
	m_PendingParticipants.clear();
	m_iPendingTelescopeOwner = LostArk::Shared::INVALID_PLAYER_ID;
	m_Targets.clear();
}

bool LostArk::Server::CKoukuCardMazeRuntime::Is_Target(
	const LostArk::Shared::NET_ENTITY_ID entityId) const noexcept
{
	return m_Targets.end() != m_Targets.find(entityId);
}

void LostArk::Server::CKoukuCardMazeRuntime::Apply_ToPlayer(
	const PARTICIPANT& participant, SERVER_PLAYER& player) noexcept
{
	using namespace LostArk::Shared;
	player.eCardMazeRole = participant.eRole;
	player.eCardMazeSuit = participant.eSuit;
	player.iCardMazeKills = participant.iKills;
	player.iCardMazeKillTarget = MECHANIC_CARD_SYMBOL::NONE != participant.eSuit ? KILL_TARGET : 0u;
}

LostArk::Server::CKoukuCardMazeRuntime::HIT_OUTCOME
LostArk::Server::CKoukuCardMazeRuntime::On_TargetHit(
	SERVER_PLAYER& hunter, const SERVER_WORLD_ENTITY& target, const bool killed)
{
	using namespace LostArk::Shared;
	HIT_OUTCOME outcome{};
	const auto registered = m_Targets.find(target.iNetEntityId);
	const auto participant = m_Participants.find(hunter.iPlayerId);
	/* Only a dealt suit striking its own targets advances anything: a wrong
	suit, or the suitless owner, takes the damage the room already applied
	and nothing more. */
	if (PHASE::HUNTING != m_ePhase || m_Targets.end() == registered ||
		m_Participants.end() == participant ||
		participant->second.eSuit != registered->second)
	{
		return outcome;
	}
	if (!m_bMarchStarted)
	{
		m_bMarchStarted = true;
		outcome.bStartMarch = true;
	}
	if (killed && m_CountedKills.insert(target.iNetEntityId).second &&
		participant->second.iKills < KILL_TARGET)
	{
		++participant->second.iKills;
		outcome.bKillCounted = true;
		if (KILL_TARGET == participant->second.iKills)
		{
			outcome.bHunterComplete = true;
			// The room stages a separate walkable exit after the matching kill.
			// Failed corridor selection keeps this earned progress for the next tick.
			bool allComplete = true;
			for (const auto& [playerId, other] : m_Participants)
			{
				(void)playerId;
				if (MECHANIC_CARD_SYMBOL::NONE != other.eSuit && other.iKills < KILL_TARGET)
				{
					allComplete = false;
					break;
				}
			}
			// The matching kill unlocks a personal exit; central arrival completes the run.
			(void)allComplete;
		}
		Apply_ToPlayer(participant->second, hunter);
	}
	return outcome;
}

bool LostArk::Server::CKoukuCardMazeRuntime::Remove_Player(
	const LostArk::Shared::PLAYER_ID playerId)
{
	using namespace LostArk::Shared;
	if (PHASE::INACTIVE == m_ePhase)
		return false;
	m_Participants.erase(playerId);
	if (playerId == m_iTelescopeOwner)
		m_iTelescopeOwner = INVALID_PLAYER_ID;
	if (PHASE::HUNTING != m_ePhase)
		return false;
	std::size_t hunters = 0u;
	std::size_t hunting = 0u;
	for (const auto& [id, participant] : m_Participants)
	{
		(void)id;
		if (MECHANIC_CARD_SYMBOL::NONE == participant.eSuit)
			continue;
		++hunters;
		if (participant.iKills < KILL_TARGET)
			++hunting;
	}
	if (0u == hunters)
		return true;
	(void)hunting;
	return false;
}

bool LostArk::Server::CKoukuCardMazeRuntime::In_SafeZone(float x, float z) noexcept
{
	return DistanceXZ(x, z, CENTER_X, CENTER_Z) <= CENTER_KEEPOUT_M;
}

bool LostArk::Server::CKoukuCardMazeRuntime::Can_Hit(
	const SERVER_PLAYER& player, LostArk::Shared::NET_ENTITY_ID targetId) const
{
	const auto p = m_Participants.find(player.iPlayerId);
	const auto t = m_Targets.find(targetId);
	return m_ePhase == PHASE::HUNTING && p != m_Participants.end() && t != m_Targets.end() &&
		!p->second.escaped && p->second.iKills < KILL_TARGET && p->second.eSuit == t->second;
}

void LostArk::Server::CKoukuCardMazeRuntime::Retire_Target(LostArk::Shared::NET_ENTITY_ID id)
{
	m_Targets.erase(id);
	m_CountedKills.erase(id);
}

bool LostArk::Server::CKoukuCardMazeRuntime::Sample_Corridor(
	LostArk::Shared::MECHANIC_CARD_SYMBOL suit,
	const std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
	const std::vector<SERVER_WORLD_ENTITY>& entities, const CServerNavigation& navigation,
	std::uint32_t seed, SPAWN_REQUEST& out, const bool avoidPersonalExits) const
{
	CARD_MAZE_RANDOM random{ Mix(static_cast<std::uint64_t>(seed) ^ (static_cast<std::uint64_t>(suit) << 32)) | 1ull };
	for (std::uint32_t attempt = 0u; attempt < SAMPLE_ATTEMPTS; ++attempt)
	{
		const float x = MAZE_MIN_X + (MAZE_MAX_X - MAZE_MIN_X) * random.Unit();
		const float z = MAZE_MIN_Z + (MAZE_MAX_Z - MAZE_MIN_Z) * random.Unit();
		SERVER_NAV_POINT point{};
		if (!navigation.Is_PointWalkableExact(x, z) || !navigation.Sample_Position(x, z, point) ||
			std::abs(point.y + .01f) > .5f || In_SafeZone(point.x, point.z)) continue;
		bool clear = true;
		for (const auto& [id, player] : players)
		{
			(void)id;
			if (player.iCurrentHp && DistanceXZ(point.x, point.z, player.fPositionX, player.fPositionZ) < PLAYER_KEEPOUT_M)
			{ clear = false; break; }
			if (avoidPersonalExits && player.iCurrentHp && (player.CardMaze.flags & 4u) &&
				DistanceXZ(point.x, point.z, player.CardMaze.exitX, player.CardMaze.exitZ) < SPAWN_SPACING_M)
			{ clear = false; break; }
		}
		for (const auto& entity : entities)
			if (entity.iCurrentHp && Is_Target(entity.iNetEntityId) &&
				DistanceXZ(point.x, point.z, entity.fPositionX, entity.fPositionZ) < SPAWN_SPACING_M) clear = false;
		if (!clear) continue;
		std::vector<SERVER_NAV_POINT> route;
		if (!navigation.Find_Path(CENTER_X, CENTER_Z, point.x, point.z, route) || route.empty() ||
			DistanceXZ(route.back().x, route.back().z, point.x, point.z) > .75f) continue;
		out = {suit, point.x, point.y, point.z, 360.f * random.Unit()};
		return true;
	}
	return false;
}

void LostArk::Server::CKoukuCardMazeRuntime::Reset_Progress(SERVER_PLAYER& player)
{
	const auto p = m_Participants.find(player.iPlayerId);
	if (p == m_Participants.end() || p->second.escaped) return;
	p->second.iKills = 0u;
	p->second.escaped = false;
	player.CardMaze.flags &= static_cast<std::uint8_t>(~6u);
	player.CardMaze.exitX = player.CardMaze.exitY = player.CardMaze.exitZ = 0.f;
	Apply_ToPlayer(p->second, player);
}

bool LostArk::Server::CKoukuCardMazeRuntime::Toggle_Telescope(SERVER_PLAYER& player)
{
	const auto p = m_Participants.find(player.iPlayerId);
	if (p == m_Participants.end() || player.iPlayerId != m_iTelescopeOwner ||
		!In_SafeZone(player.fPositionX, player.fPositionZ)) return false;
	player.CardMaze.flags ^= 1u;
	player.hasMoveGoal = false;
	player.MovePath.clear();
	return true;
}

bool LostArk::Server::CKoukuCardMazeRuntime::Is_SoloHunter(
	LostArk::Shared::PLAYER_ID playerId) const noexcept
{
	const auto p = m_Participants.find(playerId);
	return playerId == m_iTelescopeOwner && m_Participants.size() == 1u && p != m_Participants.end() &&
		p->second.eRole == LostArk::Shared::CARD_MAZE_ROLE::HUNTER;
}

void LostArk::Server::CKoukuCardMazeRuntime::Mark_Escaped(SERVER_PLAYER& player)
{
	const auto p = m_Participants.find(player.iPlayerId);
	if (p == m_Participants.end()) return;
	p->second.escaped = true;
	player.CardMaze.flags = static_cast<std::uint8_t>((player.CardMaze.flags | 2u) & ~5u);
}

bool LostArk::Server::CKoukuCardMazeRuntime::All_LivingCentral(
	const std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players) const
{
	bool anyLiving = false;
	for (const auto& [id, participant] : m_Participants)
	{
		const auto p = players.find(id);
		if (p == players.end() || !p->second.iCurrentHp) continue;
		anyLiving = true;
		if (!In_SafeZone(p->second.fPositionX, p->second.fPositionZ) || p->second.CardMaze.transferStartTick ||
			(participant.eSuit != LostArk::Shared::MECHANIC_CARD_SYMBOL::NONE && !participant.escaped)) return false;
	}
	return anyLiving;
}

void LostArk::Server::CKoukuCardMazeRuntime::Reset(
	std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players)
{
	for (const auto& [playerId, participant] : m_Participants)
	{
		(void)participant;
		const auto player = players.find(playerId);
		if (players.end() != player)
			player->second.Clear_CardMazeState();
	}
	m_Participants.clear();
	m_PendingParticipants.clear();
	m_iTelescopeOwner = LostArk::Shared::INVALID_PLAYER_ID;
	m_iPendingTelescopeOwner = LostArk::Shared::INVALID_PLAYER_ID;
	m_Targets.clear();
	m_CountedKills.clear();
	m_bMarchStarted = false;
	m_ePhase = PHASE::INACTIVE;
}

void LostArk::Server::CKoukuSaydonLogicRuntime::Project_MechanicGauge(
	const SERVER_WORLD_ENTITY& boss, const BOSS_PATTERN_DEFINITION& pattern,
	const KOUKUSAYDON_LOGIC_LEDGER& ledger, const std::uint32_t serverTick,
	LostArk::Shared::BOSS_COMBAT_SNAPSHOT& snapshot)
{
	using LostArk::Shared::BOSS_MECHANIC_GAUGE_KIND;
	snapshot.eMechanicGaugeKind = BOSS_MECHANIC_GAUGE_KIND::NONE;
	snapshot.iCurrentMechanicGauge = snapshot.iMaximumMechanicGauge = 0u;
	if (!boss.iCurrentHp || !ledger.Is_Active() || boss.strPatternId != pattern.strPatternId ||
		ledger.strPatternId != pattern.strPatternId || ledger.iPatternSequence != boss.iPatternSequence)
		return;
	for (const auto& state : ledger.Windows)
	{
		if (!state.bOpened || state.bClosed || state.iWindowIndex >= pattern.LogicWindows.size() ||
			!Has_ReachedTick(serverTick, state.iStartTick) || Has_ReachedTick(serverTick, state.iEndTick)) continue;
		const auto& window = pattern.LogicWindows[state.iWindowIndex];
		if (window.eKind != BOSS_PATTERN_LOGIC_KIND::STAGGER_WINDOW || !window.iThreshold) continue;
		const auto maximum = boss.iKoukuItemStaggerMaximum;
		snapshot.eMechanicGaugeKind = BOSS_MECHANIC_GAUGE_KIND::STAGGER;
		snapshot.iMaximumMechanicGauge = maximum;
		snapshot.iCurrentMechanicGauge = maximum - (std::min)(boss.iKoukuItemStaggerCredit, maximum);
		return;
	}
	// The typed detonation is scheduled in parent time too: its preceding 13 s
	// are the black-hole damage interval. This never judges or cancels a pattern.
	constexpr std::uint32_t blackHoleMs = 13000u;
	for (const auto& cue : ledger.MechanicTriggers)
	{
		if (cue.bStarted || cue.iIndex >= pattern.MechanicTriggers.size()) continue;
		const auto& trigger = pattern.MechanicTriggers[cue.iIndex];
		if (trigger.eKind != BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BINGO_DETONATION ||
			trigger.iStartMs < blackHoleMs || !boss.iMaximumHp) continue;
		const auto start = Add_Ticks(ledger.iPatternStartTick, Ticks_FromMs(trigger.iStartMs - blackHoleMs));
		if (!Has_ReachedTick(serverTick, start) || Has_ReachedTick(serverTick, cue.iStartTick)) continue;
		snapshot.eMechanicGaugeKind = BOSS_MECHANIC_GAUGE_KIND::BOSS_HP;
		snapshot.iCurrentMechanicGauge = (std::min)(boss.iCurrentHp, boss.iMaximumHp);
		snapshot.iMaximumMechanicGauge = boss.iMaximumHp;
		return;
	}
}
```


## Server/Private/GameRoom_KoukuPlayerCommands.cpp 전체 코드

수정 대상: [GameRoom_KoukuPlayerCommands.cpp](C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom_KoukuPlayerCommands.cpp)

```cpp
#include "GameRoom.h"

#include "ClientSession.h"
#include "ServerCombatHitRuntime.h"

#include "Network/PacketMessages.h"
#include "Network/PacketWriter.h"
#include "Gameplay/WorldCollisionContract.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <new>
#include <set>
#include <string_view>
#include <utility>

#include "GameRoom_Internal.h"

using namespace GameRoomDetail;

void LostArk::Server::CGameRoom::Handle_DebugTeleportToPlacement(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_DEBUG_TELEPORT_TO_PLACEMENT& request)
{
#ifndef _DEBUG
	(void)sessionId;
	(void)request;
	return;
#else
	using namespace LostArk::Shared;
	if (request.strPlacementId == "player.spawn.kouku.party01")
	{
		const auto session = Find_Session(sessionId);
		if (!session) return;
		S2C_DEBUG_TELEPORT_TO_POSITION_RESULT result{};
		result.iRequestSequence = request.iRequestSequence; result.eWorldId = m_eWorldId;
		const auto owner = m_PlayerIdBySessionId.find(sessionId);
		const auto player = owner != m_PlayerIdBySessionId.end() ? m_Players.find(owner->second) : m_Players.end();
		if (player != m_Players.end() && player->second.iSessionId == sessionId)
			result = Apply_DebugReturnToKoukuStart(player->second, request.iRequestSequence);
		SERVER_ROOM_RUNTIME_FAILURE failure{};
		if (Try_GetRuntimeFailure(failure))
		{ session->Request_Close(); return; }
		CPacketWriter writer;
		if (!Write_Message(writer, result) || !session->Send_Frame(
			PACKET_TYPE::S2C_DEBUG_TELEPORT_TO_POSITION_RESULT, writer.Get_Buffer()))
			session->Request_Close();
		return;
	}
	constexpr const char* STAGE_WAYPOINT_PREFIX = "stage.kouku.";
	if (WORLD_ID::KOUKUSAYDON_ARENA != m_eWorldId ||
		0u == request.iRequestSequence ||
		0u != request.strPlacementId.rfind(STAGE_WAYPOINT_PREFIX, 0u))
	{
		return;
	}

	const auto sessionIter = m_PlayerIdBySessionId.find(sessionId);
	if (sessionIter == m_PlayerIdBySessionId.end())
		return;
	const auto playerIter = m_Players.find(sessionIter->second);
	if (playerIter == m_Players.end())
		return;
	SERVER_PLAYER& player = playerIter->second;
	if (0u == player.iCurrentHp || PLAYER_ACTION_STATE::DEAD == player.eAction ||
		PLAYER_ACTION_STATE::FALLING == player.eAction)
	{
		return;
	}

	const WORLD_BOOTSTRAP_PLACEMENT* waypoint =
		Find_Placement(request.strPlacementId);
	if (nullptr == waypoint ||
		WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN != waypoint->eKind ||
		!m_ServerCollisionSystem.Is_PlayerSpawnClear(*waypoint) ||
		!m_ServerNavigation.Is_PointWalkableExact(
			waypoint->fPositionX, waypoint->fPositionZ))
	{
		return;
	}
	SERVER_NAV_POINT ground{};
	if (!m_ServerNavigation.Sample_Position(
		waypoint->fPositionX, waypoint->fPositionZ, ground))
	{
		return;
	}

	player.fPositionX = ground.x;
	player.fPositionY = ground.y;
	player.fPositionZ = ground.z;
	player.fYawDegrees = waypoint->fYawDegrees;
	Reset_PlayerForDebugTeleport(player);
	Update_MarioControlState(player);
#endif
}

void LostArk::Server::CGameRoom::Handle_DebugBingoFill(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_DEBUG_BINGO_FILL& request)
{
#ifdef _DEBUG
	using namespace LostArk::Shared;
	/* The board only exists in the Kouku arena, and only a session that owns
	a player in this room may paint it. */
	if (WORLD_ID::KOUKUSAYDON_ARENA != m_eWorldId ||
		request.eWorldId != m_eWorldId ||
		m_PlayerIdBySessionId.end() == m_PlayerIdBySessionId.find(sessionId))
	{
		return;
	}
	if (request.bReset)
		m_KoukuBingo.Reset();
	m_KoukuBingo.Fill(request.iCellMask);
	m_strStatus = "Bingo board filled by Debug request";
#else
	(void)sessionId;
	(void)request;
#endif
}

void LostArk::Server::CGameRoom::Handle_DebugBingoBomb(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_DEBUG_BINGO_BOMB& request)
{
#ifdef _DEBUG
	using namespace LostArk::Shared;
	/* Same admission as the board fill: the Kouku arena only, and only a
	session that owns a living player in this room. */
	if (WORLD_ID::KOUKUSAYDON_ARENA != m_eWorldId ||
		request.eWorldId != m_eWorldId)
	{
		return;
	}
	const auto session = m_PlayerIdBySessionId.find(sessionId);
	if (m_PlayerIdBySessionId.end() == session)
		return;
	const auto player = m_Players.find(session->second);
	if (m_Players.end() == player || 0u == player->second.iCurrentHp)
		return;
	/* The mark duration converted to fixed 30 Hz room ticks. */
	const std::uint32_t detonateTick = m_iServerTick +
		(KOUKU_BINGO_BOMB_MARK_MS * SERVER_TICK_HZ + 999u) / 1000u;
	m_strStatus = m_KoukuBingo.Start_Bomb(
		player->second.iNetEntityId, detonateTick) ?
		"Bingo bomb marked by Debug request" :
		"Bingo bomb refused: already marked or no free slot";
#else
	(void)sessionId;
	(void)request;
#endif
}

void LostArk::Server::CGameRoom::Handle_DebugBingoHammer(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_DEBUG_BINGO_HAMMER& request)
{
#ifdef _DEBUG
	using namespace LostArk::Shared;
	if (WORLD_ID::KOUKUSAYDON_ARENA != m_eWorldId ||
		request.eWorldId != m_eWorldId ||
		m_PlayerIdBySessionId.end() == m_PlayerIdBySessionId.find(sessionId))
	{
		return;
	}
	/* One of the twenty row/column ends. The diagonals stay a bingo rule
	and are never swept. */
	const std::int32_t anchor = static_cast<std::int32_t>(
		Mix_DeterministicRandom(
			static_cast<std::uint64_t>(m_iServerTick) * 1099511628211ull +
			static_cast<std::uint64_t>(sessionId)) %
		static_cast<std::uint64_t>(KOUKU_BINGO_HAMMER_ANCHOR_COUNT));
	/* The motion is authored: one World Sequence template per sweep direction
	and one instance per anchor, so naming the instance is all the presentation
	needs. The phase clock below stays authoritative - the sequence shows those
	same five seconds rather than deciding them. */
	if (m_KoukuBingo.Start_Hammer(anchor, m_iServerTick,
		m_iServerTick +
			(KOUKU_BINGO_HAMMER_WARNING_MS * SERVER_TICK_HZ + 999u) / 1000u))
	{
		Broadcast_WorldSequencePlay(
			"world.sequence.instance.kouku.bingo.hammer.anchor." +
			std::to_string(anchor));
		m_strStatus = "Bingo hammer started by Debug request";
	}
	else
	{
		m_strStatus = "Bingo hammer refused: one is already running";
	}
#else
	(void)sessionId;
	(void)request;
#endif
}

void LostArk::Server::CGameRoom::Stop_KoukuBingoDuration(const bool clearBoard)
{
    using namespace LostArk::Shared;
    for (const auto& hammer : m_KoukuBingoDuration.Hammers)
        if (hammer.anchor >= 0) Broadcast_WorldSequencePlay(
            "world.sequence.instance.kouku.bingo.hammer.anchor." + std::to_string(hammer.anchor),
            1.f, 0.f, 0.f, 0.f, 0u, {}, WORLD_SEQUENCE_OPERATION::STOP);
    const auto nextBomb = m_KoukuBingoDuration.iNextBombTick;
    const auto nextHammer = m_KoukuBingoDuration.iNextHammerTick;
    const auto nextMadness = m_KoukuBingoDuration.iNextMadnessTick;
    m_KoukuBingoDuration = {};
    if (!clearBoard)
    {
        // A repeated Parent resumes these deadlines instead of losing the
        // 50 s mark/hammer at the boundary and introducing a ten-second gap.
        m_KoukuBingoDuration.iNextBombTick = nextBomb;
        m_KoukuBingoDuration.iNextHammerTick = nextHammer;
        m_KoukuBingoDuration.iNextMadnessTick = nextMadness;
    }
    if (clearBoard)
    {
        for (std::size_t slot = 0u; slot < m_KoukuBingo.Get_Bombs().size(); ++slot)
            Broadcast_WorldSequencePlay("world.sequence.instance.kouku.bingo.bomb.planted.slot." + std::to_string(slot),
                1.f, 0.f, 0.f, 0.f, 0u, {}, WORLD_SEQUENCE_OPERATION::STOP);
        m_KoukuBingo.Reset(); m_iKoukuBingoBoardEpoch = 0u;
        for (auto& [id, player] : m_Players) player.iKoukuBingoLineProtectionEndTick = 0u;
    }
}

void LostArk::Server::CGameRoom::Activate_KoukuBingoBoard(const std::uint32_t tick)
{
    using namespace LostArk::Shared;
    auto& duration = m_KoukuBingoDuration;
    if (duration.bActivated || !duration.iOwnerId || !Has_ReachedServerTick(tick, duration.iActivationTick)) return;
    duration.bActivated = true;
    if (duration.bInitializeCells)
    {
        auto random = Mix_DeterministicRandom((std::uint64_t(m_iKoukuBingoBoardEpoch) << 32u) | duration.iActivationTick);
        const auto first = std::uint32_t(random % KOUKU_BINGO_CELL_COUNT);
        // Preserve the exact existing two-cell seed/layout; additional cells are distinct.
        auto next = first;
        std::uint32_t mask = 0u;
        for (std::uint32_t count = 0u; count < duration.Settings.iInitialMarkedCells; ++count)
        {
            if (count == 1u) next = (first + 1u + std::uint32_t((random >> 8u) % (KOUKU_BINGO_CELL_COUNT - 1))) % KOUKU_BINGO_CELL_COUNT;
            else if (count > 1u) { random = Mix_DeterministicRandom(random); next = std::uint32_t(random % KOUKU_BINGO_CELL_COUNT); }
            while (mask & (1u << next)) next = (next + 1u) % KOUKU_BINGO_CELL_COUNT;
            mask |= 1u << next;
        }
        m_KoukuBingo.Fill(mask);
        duration.bInitializeCells = false;
    }
    if (!duration.iNextBombTick) duration.iNextBombTick = Add_ServerTicksSkippingReservedZero(duration.iActivationTick,
        CKoukuSaydonLogicRuntime::Ticks_FromMs(duration.Settings.iFirstBombDelayMs));
    if (!duration.iNextHammerTick) duration.iNextHammerTick = Add_ServerTicksSkippingReservedZero(duration.iActivationTick,
        CKoukuSaydonLogicRuntime::Ticks_FromMs(KOUKU_BINGO_HAMMER_INTERVAL_MS));
    if (!duration.iNextMadnessTick) duration.iNextMadnessTick = Add_ServerTicksSkippingReservedZero(duration.iActivationTick, SERVER_TICK_HZ);
}

void LostArk::Server::CGameRoom::Begin_KoukuBingoDuration(const SERVER_WORLD_ENTITY& owner,
    const BOSS_PATTERN_MECHANIC_TRIGGER& trigger, const std::uint32_t tick)
{
    using namespace LostArk::Shared;
    const auto settings = trigger.BingoBoardSettings.value_or(BOSS_BINGO_BOARD_SETTINGS{});
    if (owner.strArchetypeId != "BOSS_KOUKUSAYDON_BINGO_SAYDON" || !trigger.iDurationMs ||
        trigger.iDurationMs > 600000u || !trigger.BingoHammerHalfExtentsM || !settings.Is_Valid()) return;
    const auto epoch = Is_KoukuRaidRunning() ? m_KoukuRaid.State.iRunEpoch : m_KoukuSaydonPatternAudition.iRoomAuditionEpoch;
    if (!epoch) return;
    const bool initializeCells = m_iKoukuBingoBoardEpoch != epoch;
    if (initializeCells)
    {
        Stop_KoukuBingoDuration(true);
        m_iKoukuBingoBoardEpoch = epoch;
    }
    else if (m_KoukuBingoDuration.bEncounterOwned && m_KoukuBingoDuration.iOwnerId == owner.iNetEntityId) return;
    else Stop_KoukuBingoDuration(false);
    auto& duration = m_KoukuBingoDuration;
    duration.iOwnerId = owner.iNetEntityId; duration.iPatternSequence = owner.iPatternSequence;
    duration.Settings = settings; duration.bInitializeCells = initializeCells;
    duration.fHammerHalfForwardM = (*trigger.BingoHammerHalfExtentsM)[0];
    duration.fHammerHalfWidthM = (*trigger.BingoHammerHalfExtentsM)[1];
    duration.bEncounterOwned = Is_KoukuRaidRunning() && m_KoukuRaid.State.strGateId == "BINGO";
    // Raid control windows are relative to combat entry, never the currently playing attack.
    // A standalone trigger is already fired at its authored start offset by the Pattern clock.
    const auto baseTick = duration.bEncounterOwned && m_KoukuRaid.State.iStartTick ? m_KoukuRaid.State.iStartTick : tick;
    duration.iActivationTick = duration.bEncounterOwned ? Add_ServerTicksSkippingReservedZero(baseTick,
        CKoukuSaydonLogicRuntime::Ticks_FromMs(trigger.iStartMs)) : tick;
    duration.iEndTick = Add_ServerTicksSkippingReservedZero(duration.iActivationTick, CKoukuSaydonLogicRuntime::Ticks_FromMs(trigger.iDurationMs));
    if (duration.bEncounterOwned && !settings.bWindowOnly) duration.iEndTick = 0u;
    if (!settings.bWindowOnly) if (const auto* catalog = Resolve_KoukuProductCatalog())
    {
        std::string status;
        const auto* pattern = CKoukuSaydonBrain::Find_AnimationOnlyPattern(*catalog, owner.strPatternId, status);
        if (pattern && !pattern->strParentLoopStartOccurrenceId.empty()) duration.iEndTick = 0u;
    }
    Activate_KoukuBingoBoard(tick);
}

void LostArk::Server::CGameRoom::Update_KoukuBingo(const std::uint32_t tick)
{
	using namespace LostArk::Shared;
	if (WORLD_ID::KOUKUSAYDON_ARENA != m_eWorldId)
		return;

    auto& duration = m_KoukuBingoDuration;
    const SERVER_WORLD_ENTITY* owner = nullptr;
    if (duration.bEncounterOwned)
    {
        const auto found = std::find_if(m_WorldEntities.begin(), m_WorldEntities.end(), [&](const auto& entity) { return entity.iNetEntityId == duration.iOwnerId; });
        if (found != m_WorldEntities.end()) owner = &*found;
    }
    else if (duration.iOwnerId) owner = Find_KoukuOccurrenceOwner(duration.iOwnerId, duration.iPatternSequence);
    if (duration.iOwnerId && (!owner || !owner->iCurrentHp ||
        (!duration.bEncounterOwned && !duration.Settings.bWindowOnly && duration.iEndTick && Has_ReachedServerTick(tick, duration.iEndTick))))
    { Stop_KoukuBingoDuration(false); owner = nullptr; }
    if (owner) Activate_KoukuBingoBoard(tick);
    const bool activeWindow = owner && duration.bActivated &&
        (!duration.iEndTick || !Has_ReachedServerTick(tick, duration.iEndTick));
    if (owner)
    {
        std::vector<SERVER_PLAYER*> alive;
        for (auto& [id, player] : m_Players)
            if (player.iCurrentHp && player.eAction != PLAYER_ACTION_STATE::DEAD &&
                Is_KoukuBingoCell(Kouku_BingoCellAt(player.fPositionX, player.fPositionZ))) alive.push_back(&player);
        if (activeWindow && Has_ReachedServerTick(tick, duration.iNextHammerTick))
        {
            const auto random = Mix_DeterministicRandom((std::uint64_t(duration.iPatternSequence) << 32u) | tick);
            const int axis = int(random % 2u) * KOUKU_BINGO_SIDE;
            // Preserve the authored two-hammer layout and its intervening row/column.
            static constexpr std::array<std::array<int, 2u>, 6u> pairs{{{0, 2}, {0, 3}, {0, 4}, {1, 3}, {1, 4}, {2, 4}}};
            const auto pair = pairs[(random >> 4u) % pairs.size()];
            const std::array<int, 2u> lines{axis + pair[0], axis + pair[1]};
            for (std::size_t slot = 0; slot < lines.size(); ++slot)
            {
                auto& hammer = duration.Hammers[slot];
                hammer.anchor = lines[slot] * 2 + int((random >> (12u + slot)) & 1u); hammer.startTick = tick;
                Broadcast_WorldSequencePlay("world.sequence.instance.kouku.bingo.hammer.anchor." + std::to_string(hammer.anchor));
            }
            duration.iNextHammerTick = Add_ServerTicksSkippingReservedZero(tick, CKoukuSaydonLogicRuntime::Ticks_FromMs(KOUKU_BINGO_HAMMER_INTERVAL_MS));
        }
        const auto* catalog = Resolve_KoukuProductCatalog();
        const auto* madness = catalog ? catalog->Find_KoukuMadnessPolicy("ENCOUNTER_KOUKUSAYDON_G1") : nullptr;
        if (activeWindow && Has_ReachedServerTick(tick, duration.iNextMadnessTick))
        {
            for (auto* player : alive)
            {
                const auto cell = Kouku_BingoCellAt(player->fPositionX, player->fPositionZ);
                if (!(m_KoukuBingo.Get_WhiteMask() & (1u << cell))) continue;
                if (!catalog) continue;
                BOSS_PATTERN_LOGIC_RESULT gain;
                gain.eKind = BOSS_PATTERN_LOGIC_RESULT_KIND::MADNESS_GAUGE_ADD_PERCENT;
                gain.iPercent = 5u;
                CKoukuSaydonLogicRuntime::Apply_Result(*player, gain, *owner, *catalog, madness, tick, m_TickDamageEvents);
            }
            duration.iNextMadnessTick = Add_ServerTicksSkippingReservedZero(tick, SERVER_TICK_HZ);
        }
        for (auto& hammer : duration.Hammers)
        {
            if (hammer.anchor < 0) continue;
            const float ageMs = float(tick - hammer.startTick) * 1000.f / SERVER_TICK_HZ;
            constexpr float sweepStart = float(KOUKU_BINGO_HAMMER_WARNING_MS + KOUKU_BINGO_HAMMER_DESCEND_MS);
            if (ageMs < sweepStart) continue;
            if (ageMs >= sweepStart + KOUKU_BINGO_HAMMER_SWEEP_MS) { hammer = {}; continue; }
            const auto path = Kouku_BingoHammerPath(hammer.anchor);
            const float dx = path.fEndX - path.fStartX, dz = path.fEndZ - path.fStartZ;
            const float length = std::hypot(dx, dz), fx = dx / length, fz = dz / length;
            const float previous = (std::clamp)((ageMs - 1000.f / SERVER_TICK_HZ - sweepStart) / KOUKU_BINGO_HAMMER_SWEEP_MS, 0.f, 1.f);
            const float current = (std::clamp)((ageMs - sweepStart) / KOUKU_BINGO_HAMMER_SWEEP_MS, 0.f, 1.f);
            // Published WORLD head dimensions include the saved Object scale.
            // Sweep the grounded head between ticks, never the tall chain above it.
            for (auto* player : alive)
            {
                if (!catalog || !player->iCurrentHp || !CombatCollision::Circle_IntersectsForwardBox(
                    {player->fPositionX, player->fPositionZ, WorldCollision::PLAYER_HALF_EXTENT_X + WorldCollision::CONTACT_MARGIN},
                    path.fStartX + dx * previous - fx * duration.fHammerHalfForwardM,
                    path.fStartZ + dz * previous - fz * duration.fHammerHalfForwardM,
                    fx, fz, 2.f * duration.fHammerHalfForwardM + length * (current - previous), duration.fHammerHalfWidthM)) continue;
                BOSS_PATTERN_LOGIC_RESULT death; death.eKind = BOSS_PATTERN_LOGIC_RESULT_KIND::INSTANT_DEATH;
                CKoukuSaydonLogicRuntime::Apply_Result(*player, death, *owner, *catalog, madness, tick, m_TickDamageEvents);
            }
        }
    }
	(void)m_KoukuBingo.Advance_Hammer(tick);
	const auto& bombs = m_KoukuBingo.Get_Bombs();
	for (std::size_t slot = 0u; slot < bombs.size(); ++slot)
	{
        if (bombs[slot].iPlantTick && Has_ReachedServerTick(tick, bombs[slot].iPlantTick))
        {
            const auto x = bombs[slot].fPositionX, z = bombs[slot].fPositionZ;
            const auto fuseTick = Add_ServerTicksSkippingReservedZero(bombs[slot].iPlantTick,
                CKoukuSaydonLogicRuntime::Ticks_FromMs(bombs[slot].iFuseMs));
            m_KoukuBingo.Plant_Bomb(slot, x, z, fuseTick);
            Broadcast_WorldSequencePlay("world.sequence.instance.kouku.bingo.bomb.planted.slot." +
                std::to_string(slot), float(KOUKU_BINGO_BOMB_FUSE_MS) / float(bombs[slot].iFuseMs), x, 0.f, z);
        }
		if (BINGO_BOMB_PHASE::PLANTED == bombs[slot].ePhase)
		{
			/* Toggle non-red cells and promote complete rows/columns before
			claiming newly completed lines. The board owns permanent reward identity. */
			if (!Has_ReachedServerTick(tick, bombs[slot].iDetonateTick))
				continue;
			m_KoukuBingo.Detonate(Kouku_BingoCrossMask(Kouku_BingoCellAt(
				bombs[slot].fPositionX, bombs[slot].fPositionZ)));
			const auto* product = Resolve_KoukuProductCatalog();
			const auto* gate = product ? product->Find_KoukuRaidGate("BINGO") : nullptr;
			const auto* patterns = gate ? product->Find_BossPatterns(gate->strEncounterId) : nullptr;
			const BOSS_PATTERN_LOGIC_WINDOW* reward = nullptr;
			if (patterns)
				for (const auto& pattern : *patterns)
					if (pattern.strPatternId == gate->strBingoSpecialPatternId)
						for (const auto& window : pattern.LogicWindows)
							if (window.eKind == BOSS_PATTERN_LOGIC_KIND::BINGO_COMPLETED_LINES && window.iThreshold &&
								std::any_of(window.OnSuccess.begin(), window.OnSuccess.end(), [](const auto& result) {
									return result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::PLAYER_INVULNERABILITY && result.iDurationMs; }))
								reward = &window;
			const auto eligible = [&](const PLAYER_ID id, const SERVER_PLAYER& player) {
				return player.Is_Human() && player.iCurrentHp && player.eAction != PLAYER_ACTION_STATE::DEAD &&
					(!Is_KoukuRaidRunning() || std::find(m_KoukuRaid.PlayerIds.begin(), m_KoukuRaid.PlayerIds.end(), id) != m_KoukuRaid.PlayerIds.end());
			};
			bool claimed = false;
			if (owner && reward && std::any_of(m_Players.begin(), m_Players.end(), [&](const auto& row) { return eligible(row.first, row.second); }))
				while (m_KoukuBingo.Consume_CompletedRowsAndColumns(reward->iThreshold)) claimed = true;
			if (claimed)
			{
				const auto* policy = product->Find_KoukuMadnessPolicy(gate->strEncounterId);
				for (auto& [id, player] : m_Players)
					if (eligible(id, player))
						for (const auto& result : reward->OnSuccess)
							if (result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::PLAYER_INVULNERABILITY)
							{
								CKoukuSaydonLogicRuntime::Apply_Result(player, result, *owner, *product, policy, tick, m_TickDamageEvents);
								const auto until = CKoukuSaydonLogicRuntime::Add_Ticks(tick, CKoukuSaydonLogicRuntime::Ticks_FromMs(result.iDurationMs));
								if (!player.iKoukuBingoLineProtectionEndTick || CKoukuSaydonLogicRuntime::Has_ReachedTick(until, player.iKoukuBingoLineProtectionEndTick))
									player.iKoukuBingoLineProtectionEndTick = until;
							}
				duration.bLineRewardSinceLastJudgement = true;
			}
			if (bombs[slot].iMarkOrdinal && bombs[slot].iMarkOrdinal % 3u == 0u)
			{
				duration.iLastLineJudgementTick = tick;
				duration.bLastLineCompletionSucceeded = duration.bLineRewardSinceLastJudgement;
				const auto publish = [&](KOUKUSAYDON_PATTERN_AUDITION_MEMBER& member) {
					member.LogicLedger.iBingoLineJudgementTick = tick;
					member.LogicLedger.iBingoCompletedLines = duration.bLineRewardSinceLastJudgement && reward ? reward->iThreshold : 0u;
					// The formation tick already applied the authored protection result.
					member.LogicLedger.bBingoLineRewardApplied = duration.bLineRewardSinceLastJudgement;
				};
				for (auto& member : m_KoukuSaydonPatternAudition.Members) publish(member);
				for (auto& tail : m_KoukuSaydonPatternAudition.Tails) publish(tail.Member);
				duration.bLineRewardSinceLastJudgement = false;
			}
			m_KoukuBingo.Clear_Bomb(slot);
			continue;
		}
		if (BINGO_BOMB_PHASE::MARKED != bombs[slot].ePhase)
			continue;
		/* The carrier owns the drop position, so a carrier that died or
		left cancels its own mark instead of planting at a stale place. */
		const SERVER_PLAYER* carrier = nullptr;
		for (const auto& entry : m_Players)
		{
			if (entry.second.Is_Human() && entry.second.iNetEntityId == bombs[slot].iCarrierNetEntityId &&
				0u != entry.second.iCurrentHp)
			{
				carrier = &entry.second;
				break;
			}
		}
		if (nullptr == carrier)
		{
			m_KoukuBingo.Clear_Bomb(slot);
			continue;
		}
		if (tick >= bombs[slot].iDetonateTick)
		{
            const auto cell = Kouku_BingoCellAt(carrier->fPositionX, carrier->fPositionZ);
            if (!Is_KoukuBingoCell(cell)) { m_KoukuBingo.Clear_Bomb(slot); continue; }
            const auto x = Kouku_BingoCellCenterX(cell), z = Kouku_BingoCellCenterZ(cell);
            m_KoukuBingo.Queue_BombPlant(slot, x, z,
                Add_ServerTicksSkippingReservedZero(bombs[slot].iDetonateTick,
                    CKoukuSaydonLogicRuntime::Ticks_FromMs(bombs[slot].iDropDelayMs)));

		}
	}
    // The authored board clock survives ordinary pattern changes and stops new marks at its window end.
    if (activeWindow && Has_ReachedServerTick(tick, duration.iNextBombTick))
    {
        std::vector<NET_ENTITY_ID> alive;
        for (const auto& [id, player] : m_Players)
            if (player.Is_Human() && player.iCurrentHp && player.eAction != PLAYER_ACTION_STATE::DEAD &&
                Is_KoukuBingoCell(Kouku_BingoCellAt(player.fPositionX, player.fPositionZ))) alive.push_back(player.iNetEntityId);
        if (!alive.empty())
        {
            const auto pick = Mix_DeterministicRandom(std::uint64_t(tick) ^ (std::uint64_t(duration.iPatternSequence) << 32u)) % alive.size();
            if (m_KoukuBingo.Start_Bomb(alive[pick], Add_ServerTicksSkippingReservedZero(tick,
                CKoukuSaydonLogicRuntime::Ticks_FromMs(duration.Settings.iBombMarkMs)), duration.iMarkedBombCount + 1u,
                duration.Settings.iBombDropDelayMs, duration.Settings.iBombFuseMs))
            {
                ++duration.iMarkedBombCount;
                if (duration.bEncounterOwned && duration.iMarkedBombCount % 3u == 0u)
                { duration.bSpecialPatternPending = true; duration.bLastLineCompletionSucceeded = false; duration.iLastLineJudgementTick = 0u; }
            }
        }
        duration.iNextBombTick = Add_ServerTicksSkippingReservedZero(tick, CKoukuSaydonLogicRuntime::Ticks_FromMs(duration.Settings.iBombIntervalMs));
    }
}

void LostArk::Server::CGameRoom::Handle_DebugSetMadnessForm(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_DEBUG_SET_MADNESS_FORM& request)
{
	using namespace LostArk::Shared;
	const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
	if (nullptr == session)
		return;
	S2C_DEBUG_SET_MADNESS_FORM_RESULT result{};
	result.iRequestSequence = request.iRequestSequence;
	result.eWorldId = m_eWorldId;
	result.eResult = DEBUG_MADNESS_FORM_RESULT::REJECTED_SESSION;
	const auto binding = m_PlayerIdBySessionId.find(sessionId);
	const auto player = binding == m_PlayerIdBySessionId.end() ?
		m_Players.end() : m_Players.find(binding->second);
	if (player != m_Players.end() && player->second.iSessionId == sessionId)
		result = Apply_DebugMadnessForm(player->second, request);
	CPacketWriter writer;
	if (!Write_Message(writer, result) || !session->Send_Frame(
		PACKET_TYPE::S2C_DEBUG_SET_MADNESS_FORM_RESULT, writer.Get_Buffer()))
	{
		session->Request_Close();
	}
}

LostArk::Shared::S2C_DEBUG_SET_MADNESS_FORM_RESULT
LostArk::Server::CGameRoom::Apply_DebugMadnessForm(
	SERVER_PLAYER& player,
	const LostArk::Shared::C2S_DEBUG_SET_MADNESS_FORM& request)
{
	using namespace LostArk::Shared;
	S2C_DEBUG_SET_MADNESS_FORM_RESULT result{};
	result.iRequestSequence = request.iRequestSequence;
	result.eWorldId = m_eWorldId;
	result.eActiveForm = player.eMadnessForm;
#ifndef _DEBUG
	result.eResult = DEBUG_MADNESS_FORM_RESULT::REJECTED_DISABLED;
	return result;
#else
	if (request.eWorldId != m_eWorldId)
	{
		result.eResult = DEBUG_MADNESS_FORM_RESULT::REJECTED_WRONG_WORLD;
		return result;
	}
	const auto& previous = player.LastDebugMadnessFormResult;
	if (0u != request.iRequestSequence &&
		request.iRequestSequence == previous.iRequestSequence)
		return previous;
	if (!Is_NewerSequence(request.iRequestSequence, previous.iRequestSequence))
	{
		result.eResult = DEBUG_MADNESS_FORM_RESULT::REJECTED_STALE_SEQUENCE;
		return result;
	}
	const auto reject = [&player, &result](const DEBUG_MADNESS_FORM_RESULT reason)
	{
		result.eResult = reason;
		player.LastDebugMadnessFormResult = result;
		return result;
	};
	/* The form only swaps the presented body. A dead, falling, grabbed or
	pattern-bound player keeps the body its current action was authored on. */
	if (0u != player.iMarioStage || 0u == player.iCurrentHp || PLAYER_ACTION_STATE::DEAD == player.eAction ||
		PLAYER_ACTION_STATE::FALLING == player.eAction ||
		PLAYER_ACTION_STATE::GRABBED == player.eAction || player.bPatternBound ||
		INVALID_NET_ENTITY_ID != player.iAttachmentOwnerNetEntityId)
		return reject(DEBUG_MADNESS_FORM_RESULT::REJECTED_PLAYER_STATE);
	if (request.eForm == player.eMadnessForm)
		return reject(DEBUG_MADNESS_FORM_RESULT::REJECTED_SAME_FORM);
	player.Clear_KoukuInteractionState();
	player.eMadnessForm = request.eForm;
	if (PLAYER_MADNESS_FORM::NORMAL == request.eForm &&
		nullptr != Active_KoukuPlayerLedger())
		player.iKoukuSuppressedPatternSequence = Active_KoukuPlayerLedger()->iPatternSequence;
	player.eAction = PLAYER_ACTION_STATE::NONE;
	player.iCurrentSkillId = INVALID_SKILL_ID;
	player.Clear_SkillTarget();
	player.iComboStage = 0u;
	player.hasBufferedComboInput = false;
	player.TriggerMove = {};
	player.iActionStartTick = 0u;
	player.fActionElapsedSeconds = 0.f;
	player.PendingCommand.Clear();
	result.eResult = DEBUG_MADNESS_FORM_RESULT::ACCEPTED;
	result.eActiveForm = player.eMadnessForm;
	player.LastDebugMadnessFormResult = result;
	return result;
#endif
}

void LostArk::Server::CGameRoom::Handle_InteractionSlot(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_INTERACTION_SLOT& request)
{
	using namespace LostArk::Shared;
	const auto binding = m_PlayerIdBySessionId.find(sessionId);
	const auto player = binding == m_PlayerIdBySessionId.end() ?
		m_Players.end() : m_Players.find(binding->second);
	if (player == m_Players.end() || player->second.iSessionId != sessionId ||
		request.eWorldId != m_eWorldId ||
		!Is_Valid_InteractionSlot(request.eSlot) ||
		KOUKU_HUD_MODE::NONE == player->second.eKoukuHudMode)
		return;
	SERVER_PLAYER& state = player->second;
	if (KOUKU_HUD_MODE::MARIO == state.eKoukuHudMode &&
		(0u == state.iMarioStage || !state.isCombatReady || state.TriggerMove.isActive ||
		 (state.iSilenceEndTick && !Has_ReachedServerTick(m_iServerTick, state.iSilenceEndTick))))
		return;
	const std::int8_t skillIndex =
		state.ModeSkillIndexBySlot[static_cast<std::size_t>(request.eSlot)];
	if (skillIndex < 0)
	{
		m_strStatus = "KoukuSaydon interaction slot is empty in the current HUD mode";
		return;
	}
	/* Every interaction skill is one fixed-length action today: the press is
	refused while any action runs, exactly like a class skill, and Update_Players
	returns the action to NONE after KOUKU_INTERACTION_ACTION_MS. */
	/* The next swing is also the hunter's own input, so it may cut a spent
	one short; the shortened MAZE cooldown below is what paces the chain. */
	(void)Cancel_MazeHammerRecovery(state, m_iServerTick);
	if (state.bPatternBound || state.fKnockbackRemainingSeconds > 0.f ||
		0u == state.iCurrentHp || PLAYER_ACTION_STATE::NONE != state.eAction)
	{
		m_strStatus = "KoukuSaydon interaction slot refused while another action runs";
		return;
	}
	if (!Is_NewerSequence(request.iRequestSequence, state.iLastKoukuInteractionSequence))
		return;
	state.iLastKoukuInteractionSequence = request.iRequestSequence;
	const SKILL_ID cooldownId = Kouku_InteractionCooldownSkillId(
		state.eKoukuHudMode, static_cast<std::uint32_t>(skillIndex));
	const auto cooldown = state.CooldownEndTickBySkillId.find(cooldownId);
	if (state.CooldownEndTickBySkillId.end() != cooldown &&
		!CKoukuSaydonLogicRuntime::Has_ReachedTick(m_iServerTick, cooldown->second))
		return;
	state.eAction = PLAYER_ACTION_STATE::INTERACTION;
	state.iCurrentSkillId = static_cast<SKILL_ID>(skillIndex);
	state.iActionStartTick =
		(std::numeric_limits<std::uint32_t>::max)() == m_iServerTick ?
		1u : m_iServerTick + 1u;
	/* The maze hammer chains at swing speed; every other mode keeps the pose
	cooldown it was authored with. */
	state.CooldownEndTickBySkillId[cooldownId] = CKoukuSaydonLogicRuntime::Add_Ticks(
		state.iActionStartTick,
		CKoukuSaydonLogicRuntime::Ticks_FromMs(
			KOUKU_HUD_MODE::MAZE == state.eKoukuHudMode ?
				KOUKU_MAZE_HAMMER_COOLDOWN_MS : KOUKU_INTERACTION_COOLDOWN_MS));
	state.fActionElapsedSeconds = 0.f;
	state.iComboStage = 0u;
	state.hasBufferedComboInput = false;
	state.hasMoveGoal = false;
	state.MovePath.clear();
	state.iMovePathIndex = 0;
	state.Clear_SkillTarget();
	state.PendingCommand.Clear();
	m_strStatus = "KoukuSaydon interaction slot started mode skill " +
		std::to_string(skillIndex);
	/* The dance answer is judged only while an authored POSE_INPUT window of
	the running pattern is open; the Debug dance override plays the pose alone. */
	if (KOUKU_HUD_MODE::DANCE == state.eKoukuHudMode &&
		nullptr != Active_KoukuPlayerLedger())
	{
		const CGameplayCatalog* pinnedCatalog = Resolve_KoukuProductCatalog();
		std::string status;
		const BOSS_PATTERN_DEFINITION* pattern = nullptr == pinnedCatalog ? nullptr :
			CKoukuSaydonBrain::Find_AnimationOnlyPattern(
				*pinnedCatalog, Active_KoukuPlayerLedger()->strPatternId, status);
		if (nullptr != pattern)
		{
			(void)CKoukuSaydonLogicRuntime::Record_InteractionSlot(
				*Active_KoukuPlayerLedger(), *pattern, state,
				request.eSlot, status);
			m_strStatus = "KoukuSaydon dance input: " + status;
		}
	}
}

void LostArk::Server::CGameRoom::Handle_DebugSetKoukuHudMode(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_DEBUG_SET_KOUKU_HUD_MODE& request)
{
	using namespace LostArk::Shared;
	const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
	if (nullptr == session)
		return;
	S2C_DEBUG_SET_KOUKU_HUD_MODE_RESULT result{};
	result.iRequestSequence = request.iRequestSequence;
	result.eWorldId = m_eWorldId;
	result.eResult = DEBUG_KOUKU_HUD_MODE_RESULT::REJECTED_SESSION;
	const auto binding = m_PlayerIdBySessionId.find(sessionId);
	const auto player = binding == m_PlayerIdBySessionId.end() ?
		m_Players.end() : m_Players.find(binding->second);
	if (player != m_Players.end() && player->second.iSessionId == sessionId)
		result = Apply_DebugKoukuHudMode(player->second, request);
	CPacketWriter writer;
	if (!Write_Message(writer, result) || !session->Send_Frame(
		PACKET_TYPE::S2C_DEBUG_SET_KOUKU_HUD_MODE_RESULT, writer.Get_Buffer()))
	{
		session->Request_Close();
	}
}

LostArk::Shared::S2C_DEBUG_SET_KOUKU_HUD_MODE_RESULT
LostArk::Server::CGameRoom::Apply_DebugKoukuHudMode(
	SERVER_PLAYER& player,
	const LostArk::Shared::C2S_DEBUG_SET_KOUKU_HUD_MODE& request)
{
	using namespace LostArk::Shared;
	S2C_DEBUG_SET_KOUKU_HUD_MODE_RESULT result{};
	result.iRequestSequence = request.iRequestSequence;
	result.eWorldId = m_eWorldId;
	result.eActiveOverride = player.eDebugKoukuHudModeOverride;
	if (request.eWorldId != m_eWorldId)
	{
		result.eResult = DEBUG_KOUKU_HUD_MODE_RESULT::REJECTED_WRONG_WORLD;
		return result;
	}
#ifndef _DEBUG
	if (m_eWorldId != WORLD_ID::KOUKUSAYDON_ARENA ||
		(KOUKU_HUD_MODE::NONE != request.eMode && KOUKU_HUD_MODE::MARIO != request.eMode &&
		 KOUKU_HUD_MODE::MAZE != request.eMode))
	{
		result.eResult = DEBUG_KOUKU_HUD_MODE_RESULT::REJECTED_UNSUPPORTED_MODE;
		return result;
	}
#endif
	const auto& previous = player.LastDebugKoukuHudModeResult;
	if (0u != request.iRequestSequence &&
		request.iRequestSequence == previous.iRequestSequence)
		return previous;
	if (!Is_NewerSequence(request.iRequestSequence, previous.iRequestSequence))
	{
		result.eResult = DEBUG_KOUKU_HUD_MODE_RESULT::REJECTED_STALE_SEQUENCE;
		return result;
	}
	/* The clown HUD comes from the madness form alone. Mario, maze and the
	dance party may be forced so the HUD and its slot clips can be checked
	before the gimmick that owns them exists; a running dance pattern still
	wins over the forced dance layout. */
	if (KOUKU_HUD_MODE::NONE != request.eMode &&
		KOUKU_HUD_MODE::POLYMORPH != request.eMode &&
		KOUKU_HUD_MODE::MARIO != request.eMode &&
		KOUKU_HUD_MODE::MAZE != request.eMode &&
		KOUKU_HUD_MODE::DANCE != request.eMode)
	{
		result.eResult = DEBUG_KOUKU_HUD_MODE_RESULT::REJECTED_UNSUPPORTED_MODE;
		player.LastDebugKoukuHudModeResult = result;
		return result;
	}
	if (0u == player.iCurrentHp || PLAYER_ACTION_STATE::DEAD == player.eAction ||
		PLAYER_ACTION_STATE::FALLING == player.eAction ||
		PLAYER_ACTION_STATE::GRABBED == player.eAction || player.bPatternBound)
	{
		result.eResult = DEBUG_KOUKU_HUD_MODE_RESULT::REJECTED_UNSUPPORTED_MODE;
		player.LastDebugKoukuHudModeResult = result;
		return result;
	}
	player.Clear_KoukuInteractionState();
	if (nullptr != Active_KoukuPlayerLedger())
		player.iKoukuSuppressedPatternSequence = Active_KoukuPlayerLedger()->iPatternSequence;
	player.eMadnessForm = (KOUKU_HUD_MODE::NONE == request.eMode || KOUKU_HUD_MODE::MAZE == request.eMode) ?
		PLAYER_MADNESS_FORM::NORMAL : PLAYER_MADNESS_FORM::CLOWN;
	player.iMadnessFormEndTick = 0u;
	player.eAction = PLAYER_ACTION_STATE::NONE;
	player.iCurrentSkillId = INVALID_SKILL_ID;
	player.Clear_SkillTarget();
	player.iComboStage = 0u;
	player.hasBufferedComboInput = false;
	player.TriggerMove = {};
	player.iActionStartTick = 0u;
	player.fActionElapsedSeconds = 0.f;
	player.PendingCommand.Clear();
	player.eDebugKoukuHudModeOverride = request.eMode;
	result.eResult = DEBUG_KOUKU_HUD_MODE_RESULT::ACCEPTED;
	result.eActiveOverride = player.eDebugKoukuHudModeOverride;
	player.LastDebugKoukuHudModeResult = result;
	return result;
}

void LostArk::Server::CGameRoom::Update_KoukuPlayerModes(
	const std::uint32_t serverTick)
{
	using namespace LostArk::Shared;
	const BOSS_ENCOUNTER_MADNESS_POLICY* policy =
		WORLD_ID::KOUKUSAYDON_ARENA == m_eWorldId ?
			m_GameplayCatalog.Active().Find_KoukuMadnessPolicy(
				std::string(KOUKUSAYDON_G1_ENCOUNTER_ID)) :
			nullptr;
	const KOUKUSAYDON_LOGIC_LEDGER* ledger = nullptr;
	if (nullptr != Active_KoukuPlayerLedger())
	{
		ledger = Active_KoukuPlayerLedger();
		if (const auto* product = Resolve_KoukuProductCatalog())
			policy = product->Find_KoukuMadnessPolicy(std::string(KOUKUSAYDON_G1_ENCOUNTER_ID));
	}
	CKoukuSaydonLogicRuntime::Update_PlayerModes(
		m_Players, ledger, policy, serverTick);
	if (WORLD_ID::KOUKUSAYDON_ARENA != m_eWorldId) return;
	bool mazeExpired = false;
	for (auto& [id, player] : m_Players)
	{
		const bool mario = player.iMarioStage != 0u;
		const bool maze = player.eKoukuAreaHudMode == KOUKU_HUD_MODE::MAZE &&
			m_KoukuCardMaze.Get_Phase() != CKoukuCardMazeRuntime::PHASE::COMPLETE;
		if (!player.Is_Human() || !player.iCurrentHp || (!mario && !maze))
		{ player.iKoukuMinigameEndTick = 0u; continue; }
		if (!player.iKoukuMinigameEndTick)
			player.iKoukuMinigameEndTick = Add_ServerTicksSkippingReservedZero(serverTick, 90u * SERVER_TICK_HZ);
		if (!Has_ReachedServerTick(serverTick, player.iKoukuMinigameEndTick)) continue;
		if (maze) { mazeExpired = true; continue; }
		SERVER_WORLD_TO_PLAYER_HIT death{};
		death.iRawDamage = player.iCurrentHp; death.iServerTick = serverTick;
		death.bEncounterWipe = death.bIgnoreDefense = death.bIgnoreCounter = true;
		(void)CServerCombatHitRuntime::Apply_WorldToPlayer(player, death, m_GameplayCatalog.Active(), m_TickDamageEvents);
		player.iKoukuMinigameEndTick = 0u;
		Update_MarioControlState(player);
	}
	if (mazeExpired)
	{
		for (auto& [id, player] : m_Players)
		{
			if (!player.Is_Human() || !player.iCurrentHp) continue;
			SERVER_WORLD_TO_PLAYER_HIT death{};
			death.iRawDamage = player.iCurrentHp; death.iServerTick = serverTick;
			death.bEncounterWipe = death.bIgnoreDefense = death.bIgnoreCounter = true;
			(void)CServerCombatHitRuntime::Apply_WorldToPlayer(player, death, m_GameplayCatalog.Active(), m_TickDamageEvents);
			player.iKoukuMinigameEndTick = 0u;
		}
		Reset_CardMaze();
		m_strStatus = "Card maze exceeded its ninety-second escape deadline; party defeated";
	}
}

void LostArk::Server::CGameRoom::Apply_KoukuGateEntryCard(
	SERVER_PLAYER& player, const SERVER_WORLD_ENTITY& boss)
{
	using namespace LostArk::Shared;
	if (WORLD_ID::KOUKUSAYDON_ARENA != m_eWorldId)
		return;
	if ("BOSS_KOUKUSAYDON_G1_KOUKU" != boss.strArchetypeId &&
		"BOSS_KOUKUSAYDON_G1_SAYDON" != boss.strArchetypeId)
	{ player.Clear_KoukuAssignedCard(); return; }
	std::uint8_t used = 0u;
	for (const auto& [id, other] : m_Players)
	{
		if (other.Is_Guide() || id == player.iPlayerId) continue;
		const auto symbol = static_cast<std::uint8_t>(other.eMechanicCardSymbol);
		if (symbol >= 1u && symbol <= 4u) used |= static_cast<std::uint8_t>(1u << (symbol - 1u));
	}
	const auto symbol = static_cast<std::uint8_t>(player.eMechanicCardSymbol);
	if (symbol >= 1u && symbol <= 4u && (used & (1u << (symbol - 1u))))
		player.Clear_KoukuAssignedCard();
	CKoukuSaydonLogicRuntime::Assign_EncounterCard(player, boss.iNetEntityId, m_iServerTick, used);
}

bool LostArk::Server::CGameRoom::Apply_KoukuLogicOutput(
	const KOUKUSAYDON_LOGIC_OUTPUT& output,
	SERVER_WORLD_ENTITY& boss,
	const std::uint32_t serverTick)
{
	using namespace LostArk::Shared;
	auto* member = Find_KoukuAuditionMember(boss.iNetEntityId, boss.iPatternSequence);
	if (!member) return false;
    for (const auto& request : output.CaptureRequests)
    {
        const auto player = m_PlayerIdByEntityId.find(request.iPlayerNetEntityId);
        if (player == m_PlayerIdByEntityId.end() || request.iWindowIndex >= member->LogicLedger.Windows.size() ||
            member->LogicLedger.iPatternSequence != boss.iPatternSequence ||
            member->LogicLedger.Windows[request.iWindowIndex].iHoldEndTick != request.iHoldEndTick)
        { m_strStatus = "Kouku capture candidate lost its owning Hold window"; continue; }
        if (Capture_PlayerAttachment(request.iPlayerNetEntityId, boss.iNetEntityId,
            request.eAttachmentSlot, serverTick, request.iHoldEndTick, boss.iPatternSequence))
        {
            auto& captured = m_Players.at(player->second);
            captured.iAttachmentWindowIndex = request.iWindowIndex;
            if (request.bHasHandLocalPosition &&
                !CKoukuSaydonLogicRuntime::Apply_CaptureHandLocalPosition(captured, boss, request.HandLocalPosition))
            {
                captured.iAttachmentEndTick = serverTick;
                m_strStatus = "Capture hand position was not finite after room commit";
                continue;
            }
            member->LogicLedger.Windows[request.iWindowIndex].Answers[player->second] = KOUKUSAYDON_LOGIC_ANSWER::SUCCESS;
            member->LogicLedger.Windows[request.iWindowIndex].InsidePlayers.insert(player->second);
        }
        else
            m_strStatus = "Kouku capture candidate was no longer eligible at room commit";
    }
	for (const auto& trigger : output.MechanicTriggers)
		m_PendingKoukuMechanicTriggers.push_back({ boss.iNetEntityId, boss.iPatternSequence, trigger });
	if (output.BingoLineCompletion.has_value() && m_KoukuBingoDuration.iLastLineJudgementTick)
		m_KoukuBingoDuration.bLastLineCompletionSucceeded = *output.BingoLineCompletion;
	for (const KOUKUSAYDON_LOGIC_WORLD_PLAY& play : output.WorldSequencePlays)
	{
		S2C_WORLD_SEQUENCE_PLAY message;
		message.strSequenceInstanceId = play.strInstanceId; message.strTargetSequenceInstanceId = play.strTargetSequenceInstanceId;
		message.fPlaybackSpeed = play.fPlaybackSpeed; message.fPositionOffsetX = play.fPositionOffsetX;
		message.fPositionOffsetY = play.fPositionOffsetY; message.fPositionOffsetZ = play.fPositionOffsetZ; message.iDurationMs = play.iDurationMs;
		message.iRunEpoch = m_KoukuSaydonPatternAudition.iRoomAuditionEpoch; message.strMemberId = member->strMemberId;
		message.strCueId = "world." + std::to_string(boss.iPatternSequence) + "." + std::to_string(member->iNextWorldCue++);
		message.strOccurrenceId = play.strOccurrenceId;
		message.iStartTick = play.iStartTick ? play.iStartTick : serverTick; message.iServerTick = serverTick;
		message.iBossNetEntityId = boss.iNetEntityId; message.iPatternSequence = boss.iPatternSequence;
		if (play.CombatBody && !play.bAuthoredMadness) { message.bUntilDestroyed = true; message.iDurationMs = 0u; }
		if (play.Placement)
		{
			const auto& placement = *play.Placement; message.bHasPlacement = true;
			message.fWorldPositionX = placement.fPositionX; message.fWorldPositionY = placement.fPositionY; message.fWorldPositionZ = placement.fPositionZ;
			message.fWorldRotationXDegrees = placement.fRotationXDegrees; message.fWorldRotationYDegrees = placement.fRotationYDegrees; message.fWorldRotationZDegrees = placement.fRotationZDegrees;
			message.fWorldScaleX = placement.fScaleX; message.fWorldScaleY = placement.fScaleY; message.fWorldScaleZ = placement.fScaleZ;
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
	}
	/* A follow-up joins the audition right after the running slot, with a
	one-tick transition, so Play All and Play Selected both continue into it. */
	auto* scheduleMember = Find_KoukuAuditionMember(boss.iNetEntityId);
	if (!scheduleMember) scheduleMember = member;
	const bool resumeCompleted = scheduleMember->bCompleted && !output.FollowupPatternIds.empty();
	std::size_t insertAt = scheduleMember->iPatternIndex + 1u;
	for (const std::string& followup : output.FollowupPatternIds)
	{
		if (insertAt > scheduleMember->PatternIds.size())
			break;
		scheduleMember->PatternIds.insert(
			scheduleMember->PatternIds.begin() + insertAt, followup);
		scheduleMember->TransitionTicks.insert(
			scheduleMember->TransitionTicks.begin() + (insertAt - 1u), 1u);
        if (scheduleMember->bParentSequenceStarted && !scheduleMember->bParentSequenceLoops &&
            scheduleMember->iPatternIndex <= scheduleMember->iParentLastIndex)
        {
            // A counter's authored groggy follow-up is another child completion,
            // not permission to skip the rest of Mario's Parent.
            ++scheduleMember->iParentLastIndex;
            ++scheduleMember->iCompletionChainCount;
        }
		++insertAt;
	}
	if (resumeCompleted)
	{
		++scheduleMember->iPatternIndex;
		scheduleMember->bCompleted = false;
		scheduleMember->ePhase = KOUKUSAYDON_PATTERN_AUDITION_PHASE::PENDING;
		scheduleMember->iNextStartTick = CKoukuSaydonLogicRuntime::Add_Ticks(serverTick, 1u);
		m_KoukuSaydonPatternAudition.ePhase = KOUKUSAYDON_PATTERN_AUDITION_PHASE::PENDING;
	}
	if (!output.strStatus.empty())
		m_strStatus = "KoukuSaydon logic: " + output.strStatus;
	if (!output.bEndPatternEarly)
		return false;
	if (output.bCounterSuccessLanded || output.bStaggerSuccess)
	{
		// Counter and stagger success interrupt this member's hazards immediately.
		// Natural completion may keep tails; interruption must not.
		Stop_KoukuWorldOwner(member->strMemberId);
		m_CombatObjectRuntime.Cancel_Source(boss.iNetEntityId);
		std::erase_if(m_PendingKoukuMechanicTriggers, [&](const auto& trigger) {
			return trigger.iBossEntityId == boss.iNetEntityId && trigger.iPatternSequence == boss.iPatternSequence;
		});
	}
	m_KoukuSaydonBrain.Complete_Pattern(boss, serverTick);
	return true;
}

LostArk::Shared::S2C_DEBUG_TELEPORT_TO_POSITION_RESULT
LostArk::Server::CGameRoom::Apply_DebugReturnToKoukuStart(SERVER_PLAYER& player, const std::uint32_t requestSequence)
{
	using namespace LostArk::Shared;
	S2C_DEBUG_TELEPORT_TO_POSITION_RESULT result{};
	result.iRequestSequence = requestSequence; result.eWorldId = m_eWorldId;
	if (m_eWorldId != WORLD_ID::KOUKUSAYDON_ARENA)
	{ result.eResult = DEBUG_TELEPORT_RESULT::REJECTED_WRONG_WORLD; return result; }
	if (requestSequence != 0u && requestSequence == player.LastDebugTeleportResult.iRequestSequence)
		return player.LastDebugTeleportResult;
	if (!Is_NewerSequence(requestSequence, player.LastDebugTeleportResult.iRequestSequence))
	{ result.eResult = DEBUG_TELEPORT_RESULT::REJECTED_STALE_SEQUENCE; return result; }
	const auto* spawn = Find_Placement("player.spawn.kouku.party01");
	if (!spawn || spawn->eKind != WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN || !spawn->isEnabled)
	{ result.eResult = DEBUG_TELEPORT_RESULT::REJECTED_INVALID_POSITION; return result; }
	// Keep this room's world, ground sampler, log and build-specific wave policy.
	// Initialize replaces trigger latches; clear per-player entry/debounce state
	// in the staged copy before committing any live player or entity changes.
	CServerTriggerSystem triggers = m_ServerTriggerSystem;
	for (const auto& [playerId, current] : m_Players)
	{
		(void)current;
		triggers.Remove_Player(playerId);
	}
	triggers.Set_HonourTriggerOnce(false);
	std::string status;
	if (!triggers.Initialize(m_WorldBootstrap.Get_Placements(), status))
	{ m_strStatus = status; result.eResult = DEBUG_TELEPORT_RESULT::REJECTED_DISABLED; return result; }
	C2S_DEBUG_TELEPORT_TO_POSITION placement{};
	placement.iRequestSequence = requestSequence; placement.eWorldId = WORLD_ID::KOUKUSAYDON_ARENA;
	placement.fPositionX = spawn->fPositionX; placement.fPositionY = spawn->fPositionY; placement.fPositionZ = spawn->fPositionZ;
	result = Apply_DebugTeleportToPosition(player, placement);
	if (result.eResult != DEBUG_TELEPORT_RESULT::ACCEPTED) return result;
	// Navigation, height, collision and player state were accepted. This explicit
	// shared-arena reset removes boss-owned state; other players are not teleported.
	player.fYawDegrees = spawn->fYawDegrees;
	Clear_KoukuSaydonPatternAudition();
	if (!Despawn_KoukuSaydonArenaDebugEntities(true))
	{
		result.eResult = DEBUG_TELEPORT_RESULT::REJECTED_DISABLED;
		player.LastDebugTeleportResult = result;
		Mark_RuntimeFailure("kouku-start.boss-reset");
		return result;
	}
	m_ServerTriggerSystem = std::move(triggers);
	std::unordered_set<std::string> entrySequences;
	for (const auto& trigger : m_WorldBootstrap.Get_Placements())
		for (const auto& action : trigger.TriggerActions)
			if (action.eKind == WORLD_TRIGGER_ACTION_KIND::PLAY_SEQUENCE && entrySequences.insert(action.strTargetId).second)
				Broadcast_WorldSequencePlay(action.strTargetId, 1.f, 0.f, 0.f, 0.f, 0u, {}, WORLD_SEQUENCE_OPERATION::STOP);
	Reset_CardMaze();
	Stop_KoukuBingoDuration(true);
	m_iNextMarioEntryStage = 1u;
	player.Clear_KoukuInteractionState();
	player.Clear_KoukuAssignedCard();
	m_strStatus = "KoukuSaydon arena bosses and entry triggers reset; requesting player returned to authored start.";
	return result;
}

void LostArk::Server::CGameRoom::Reset_PlayerForDebugTeleport(SERVER_PLAYER& player)
{
	using namespace LostArk::Shared;
	player.Clear_MarioControl();
	player.eAction = PLAYER_ACTION_STATE::NONE;
	player.iCurrentSkillId = INVALID_SKILL_ID;
	player.Clear_SkillTarget();
	player.iActionStartTick = 0u;
	player.Clear_Attachment();
	player.Clear_PatternBindStatus();
	player.Clear_SilenceStatus();
	player.fFallVelocityY = 0.f;
	player.iFallDeathTick = 0u;
	player.fActionElapsedSeconds = 0.f;
	player.hasAppliedSkillDamage = false;
	player.iAppliedHitMask = 0u;
	player.HitWindowTargets.clear();
	player.iSpawnedProjectileMask = 0u;
	player.Projectiles.clear();
	m_CombatObjectRuntime.Cancel_Source(player.iNetEntityId);
	player.iComboStage = 0u;
	player.hasBufferedComboInput = false;
	player.PendingCommand.Clear();
	player.hasReleasedHold = false;
	player.TriggerMove = {};
	player.hasMoveGoal = false;
	player.MovePath.clear();
	player.iMovePathIndex = 0u;
	player.fKnockbackDirectionX = 0.f;
	player.fKnockbackDirectionZ = 0.f;
	player.fKnockbackSpeed = 0.f;
	player.fKnockbackRemainingSeconds = 0.f;
	player.iKnockdownEndTick = 0u;
	player.iHitReactionGraceEndTick = 0u;
	player.isCombatReady = true;
	m_ServerTriggerSystem.Remove_Player(player.iPlayerId);
}

void LostArk::Server::CGameRoom::Handle_DebugTeleportToPosition(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_DEBUG_TELEPORT_TO_POSITION& request)
{
	using namespace LostArk::Shared;
	const auto session = Find_Session(sessionId);
	if (nullptr == session)
		return;
	S2C_DEBUG_TELEPORT_TO_POSITION_RESULT result{};
	result.iRequestSequence = request.iRequestSequence;
	result.eWorldId = m_eWorldId;
	const auto binding = m_PlayerIdBySessionId.find(sessionId);
	const auto player = binding == m_PlayerIdBySessionId.end() ?
		m_Players.end() : m_Players.find(binding->second);
	if (player != m_Players.end() && player->second.iSessionId == sessionId)
		result = Apply_DebugTeleportToPosition(player->second, request);
	CPacketWriter writer;
	if (!Write_Message(writer, result) || !session->Send_Frame(
		PACKET_TYPE::S2C_DEBUG_TELEPORT_TO_POSITION_RESULT, writer.Get_Buffer()))
	{
		session->Request_Close();
	}
}

LostArk::Shared::S2C_DEBUG_TELEPORT_TO_POSITION_RESULT
LostArk::Server::CGameRoom::Apply_DebugTeleportToPosition(
	SERVER_PLAYER& player,
	const LostArk::Shared::C2S_DEBUG_TELEPORT_TO_POSITION& request)
{
	using namespace LostArk::Shared;
	S2C_DEBUG_TELEPORT_TO_POSITION_RESULT result{};
	result.iRequestSequence = request.iRequestSequence;
	result.eWorldId = m_eWorldId;
#ifndef _DEBUG
	if (m_eWorldId != WORLD_ID::VALTAN_ARENA && m_eWorldId != WORLD_ID::KOUKUSAYDON_ARENA)
	{
		result.eResult = DEBUG_TELEPORT_RESULT::REJECTED_DISABLED;
		return result;
	}
#endif
	if (request.eWorldId != m_eWorldId)
	{
		result.eResult = DEBUG_TELEPORT_RESULT::REJECTED_WRONG_WORLD;
		return result;
	}
	const auto& previous = player.LastDebugTeleportResult;
	if (0u != request.iRequestSequence &&
		request.iRequestSequence == previous.iRequestSequence)
		return previous;
	if (!Is_NewerSequence(request.iRequestSequence, previous.iRequestSequence))
	{
		result.eResult = DEBUG_TELEPORT_RESULT::REJECTED_STALE_SEQUENCE;
		return result;
	}
	const auto reject = [&player, &result](const DEBUG_TELEPORT_RESULT reason)
	{
		result.eResult = reason;
		player.LastDebugTeleportResult = result;
		return result;
	};
	SERVER_NAV_POINT ground{};
	const auto validation = Validate_DebugTeleportDestination(player, request, ground);
	if (validation != DEBUG_TELEPORT_RESULT::ACCEPTED) return reject(validation);
	/* Validation is complete before any action, movement, projectile or
	trigger state is cleared. Only this session's player is mutated. */
	Reset_PlayerForDebugTeleport(player);
	player.fPositionX = ground.x;
	player.fPositionY = ground.y;
	player.fPositionZ = ground.z;
	result.eResult = DEBUG_TELEPORT_RESULT::ACCEPTED;
	Update_MarioControlState(player);
	Guide_AnchorArrived(player);
	result.fPositionX = ground.x;
	result.fPositionY = ground.y;
	result.fPositionZ = ground.z;
	player.LastDebugTeleportResult = result;
	return result;
}

LostArk::Shared::DEBUG_TELEPORT_RESULT LostArk::Server::CGameRoom::Validate_DebugTeleportDestination(
	const SERVER_PLAYER& player, const LostArk::Shared::C2S_DEBUG_TELEPORT_TO_POSITION& request,
	SERVER_NAV_POINT& ground, const LostArk::Shared::NET_ENTITY_ID ignoredBodyId)
{
	using namespace LostArk::Shared;
	if (0u == player.iCurrentHp || PLAYER_ACTION_STATE::DEAD == player.eAction ||
		PLAYER_ACTION_STATE::FALLING == player.eAction ||
		PLAYER_ACTION_STATE::GRABBED == player.eAction || player.bPatternBound ||
		INVALID_NET_ENTITY_ID != player.iAttachmentOwnerNetEntityId)
		return (DEBUG_TELEPORT_RESULT::REJECTED_PLAYER_STATE);
	if (!std::isfinite(request.fPositionX) || !std::isfinite(request.fPositionY) ||
		!std::isfinite(request.fPositionZ) || std::abs(request.fPositionX) > 100000.f ||
		std::abs(request.fPositionY) > 100000.f || std::abs(request.fPositionZ) > 100000.f)
		return (DEBUG_TELEPORT_RESULT::REJECTED_INVALID_POSITION);
	if (!m_ServerNavigation.Is_PointWalkableExact(request.fPositionX, request.fPositionZ) ||
		!m_ServerNavigation.Sample_Position(request.fPositionX, request.fPositionZ, ground))
		return (DEBUG_TELEPORT_RESULT::REJECTED_NAVIGATION);
	/* The single-layer Server grid owns Y. A picked roof/prop/other deck must
	not silently land on whatever unrelated floor happens to share its XZ.
	One metre covers authored mesh-vs-cell sampling variation, never whole decks. */
	constexpr float MAX_PICKED_GROUND_HEIGHT_ERROR = 1.f;
	if (!std::isfinite(ground.y) ||
		std::abs(request.fPositionY - ground.y) > MAX_PICKED_GROUND_HEIGHT_ERROR)
		return (DEBUG_TELEPORT_RESULT::REJECTED_HEIGHT);
	Refresh_PlayerBlockingBodies();
	if (!m_ServerCollisionSystem.Is_PlayerPositionClear(
		ground.x, ground.y, ground.z, ignoredBodyId == INVALID_NET_ENTITY_ID ? player.iNetEntityId : ignoredBodyId))
		return (DEBUG_TELEPORT_RESULT::REJECTED_COLLISION);
	return DEBUG_TELEPORT_RESULT::ACCEPTED;
}

bool LostArk::Server::CGameRoom::Configure_MarioRail(
	SERVER_PLAYER& player, const std::string& arrivalPlacementId)
{
	using namespace LostArk::Shared;
	if (0u == player.iMarioStage || player.iMarioStage > 4u || player.TriggerMove.isActive ||
		!std::isfinite(player.fPositionX) || !std::isfinite(player.fPositionZ))
		return false;
	const auto lane = std::find_if(MARIO_LANES.begin(), MARIO_LANES.end(),
		[&player, &arrivalPlacementId](const MARIO_LANE_BINDING& candidate)
		{ return candidate.stage == player.iMarioStage && arrivalPlacementId == candidate.arrival; });
	const auto* arrival = lane == MARIO_LANES.end() ? nullptr : Find_Placement(lane->arrival);
	const auto* exit = lane == MARIO_LANES.end() ? nullptr : Find_Placement(lane->exit);
	// The admitted stage uses arrival as rail metadata, not as a new trigger entry.
	// Disabled go boxes remain excluded by ServerTriggerSystem (including F1 entry).
	if (nullptr == arrival || nullptr == exit || !exit->isEnabled ||
		WORLD_BOOTSTRAP_KIND::TRIGGER_BOX != arrival->eKind ||
		WORLD_BOOTSTRAP_KIND::TRIGGER_BOX != exit->eKind || arrival->TriggerActions.size() != 1u ||
		WORLD_TRIGGER_ACTION_KIND::MOVE_PLAYER != arrival->TriggerActions.front().eKind)
	{
		player.bMarioRailReady = false;
		std::cout << "[MarioRail] invalid authored binding: " << arrivalPlacementId << '\n';
		return false;
	}
	const double dx = static_cast<double>(exit->fPositionX) - player.fPositionX;
	const double dz = static_cast<double>(exit->fPositionZ) - player.fPositionZ;
	const double length = std::hypot(dx, dz);
	if (!std::isfinite(length) || length < 0.1)
	{
		player.bMarioRailReady = false;
		std::cout << "[MarioRail] unusable arrival-to-exit axis: " << arrivalPlacementId << '\n';
		return false;
	}
	player.fMarioRailOriginX = player.fPositionX;
	player.fMarioRailOriginZ = player.fPositionZ;
	player.fMarioRailRightX = static_cast<float>(dx / length) * lane->rightSign;
	player.fMarioRailRightZ = static_cast<float>(dz / length) * lane->rightSign;
	player.strMarioRailArrivalId = arrivalPlacementId;
	player.bMarioRailReady = true;
	player.iMarioMoveExpiryTick = 0u;
	player.fMarioDirectionX = player.fMarioDirectionZ = 0.f;
	return true;
}

std::uint8_t LostArk::Server::CGameRoom::Begin_MarioStageObjects(const std::uint8_t stage)
{
	if (stage < 1u || stage > 4u) return 0u;
	for (const auto& [id, participant] : m_Players)
	{
		(void)id;
		if (participant.Is_Human() && participant.iMarioStage == stage && participant.iCurrentHp && participant.iMarioLayoutVariant)
			return participant.iMarioLayoutVariant;
	}
	const std::string groupId = "spawn.mario" + std::to_string(stage) + ".source";
	const auto& groups = m_SpawnGroupBootstrap.Get_Groups();
	const auto definition = std::find_if(groups.begin(), groups.end(), [&groupId](const auto& group) {
		return group.strSpawnGroupId == groupId;
	});
	if (definition == groups.end())
	{
		m_strStatus = "Mario source spawn group missing: " + groupId;
		return 0u;
	}
	// Validate every anchor before changing the old group or admitting this run.
	for (const auto& wave : definition->Waves)
		for (const auto& entry : wave.Entries)
		{
			const auto* anchor = m_SpawnGroupBootstrap.Find_Anchor(entry.strAnchorId);
			SERVER_NAV_POINT ground{};
			if (!anchor || !m_SpawnGroupBootstrap.Find_Profile(entry.strArchetypeId) ||
				!m_ServerNavigation.Sample_Position(anchor->fPositionX, anchor->fPositionZ, ground) ||
				std::abs(ground.y - anchor->fPositionY) > .25f)
			{
				m_strStatus = "Mario source anchor/profile rejected: " + entry.strAnchorId;
				return 0u;
			}
		}
	Reset_MarioStageObjects(stage);
	if (!m_SpawnGroupRuntime.Activate(groupId))
	{
		m_strStatus = "Mario source spawn activation failed: " + groupId;
		return 0u;
	}
	const auto roll = std::uniform_int_distribution<unsigned>{0u, 99u}(m_MarioLayoutRandom);
	return roll < 34u ? 1u : (roll < 67u ? 2u : 3u);
}

void LostArk::Server::CGameRoom::Reset_MarioStageObjects(const std::uint8_t stage)
{
	if (stage < 5u) m_MarioPoppedBalls[stage] = 0u;
	const std::string groupId = "spawn.mario" + std::to_string(stage) + ".source";
	for (auto entity = m_WorldEntities.begin(); entity != m_WorldEntities.end();)
	{
		if (entity->strSpawnGroupId != groupId) { ++entity; continue; }
		m_CombatObjectRuntime.Cancel_Source(entity->iNetEntityId);
		Broadcast_WorldEntityDespawned(entity->iNetEntityId);
		entity = m_WorldEntities.erase(entity);
	}
	if (!Broadcast_CombatObjectLifecycle()) Mark_RuntimeFailure("mario.reset.combat-object-lifecycle");
	(void)m_SpawnGroupRuntime.Reset_Group(groupId);
}

void LostArk::Server::CGameRoom::Cleanup_EmptyMarioStages()
{
	if (m_eWorldId != LostArk::Shared::WORLD_ID::KOUKUSAYDON_ARENA) return;
	for (std::uint8_t stage = 1u; stage <= 4u; ++stage)
	{
		const bool occupied = std::any_of(m_Players.begin(), m_Players.end(), [stage](const auto& pair) {
			return pair.second.Is_Human() && pair.second.iMarioStage == stage && pair.second.iCurrentHp;
		});
		if (!occupied && m_SpawnGroupRuntime.Is_ActiveOrCompleted("spawn.mario" + std::to_string(stage) + ".source"))
			Reset_MarioStageObjects(stage);
	}
}

void LostArk::Server::CGameRoom::Reset_MarioContactAction(SERVER_PLAYER& player)
{
	const bool combatReady = player.isCombatReady;
	Cancel_PlayerActionForPatternStatus(player);
	player.isCombatReady = combatReady;
	player.iFearEndTick = 0u;
	player.strFearPresentationId.clear();
}

bool LostArk::Server::CGameRoom::Enter_MarioFromPattern(SERVER_PLAYER& player, const std::uint8_t stage)
{
	using namespace LostArk::Shared;
	if (m_eWorldId != WORLD_ID::KOUKUSAYDON_ARENA || stage < 1u || stage > 4u ||
		!CKoukuSaydonLogicRuntime::Can_EnterMarioEntry(player)) return false;
	const auto* intro = Find_Placement("Mario" + std::to_string(stage) + "_Intro");
	SERVER_NAV_POINT ground;
	if (!intro || !intro->isEnabled || intro->eKind != WORLD_BOOTSTRAP_KIND::TRIGGER_BOX ||
		!m_ServerNavigation.Is_PointWalkableExact(intro->fPositionX, intro->fPositionZ) ||
		!m_ServerNavigation.Sample_Position(intro->fPositionX, intro->fPositionZ, ground))
	{ m_strStatus = "Mario entry destination is unavailable in published navigation"; return false; }
	SERVER_PLAYER candidate = player;
	candidate.fPositionX = ground.x; candidate.fPositionY = ground.y; candidate.fPositionZ = ground.z;
	if (!CServerTriggerSystem::Contains_Placement(*intro, candidate) ||
		!m_ServerCollisionSystem.Is_PlayerPositionClear(candidate.fPositionX, candidate.fPositionY, candidate.fPositionZ, player.iNetEntityId))
	{ m_strStatus = "Mario entry destination is outside the Intro box or blocked"; return false; }
	// Stage action interruption on the detached player. The Debug teleport
	// helper also cancels live objects/trigger occupancy, so it is not a preflight.
	candidate.Clear_MarioControl();
	Reset_MarioContactAction(candidate);
	Update_MarioControlState(candidate);
	if (candidate.iMarioStage != stage) return false;
	m_CombatObjectRuntime.Cancel_Source(player.iNetEntityId);
	m_ServerTriggerSystem.Remove_Player(player.iPlayerId);
	player = std::move(candidate);
	return true;
}

void LostArk::Server::CGameRoom::Update_MarioControlState(SERVER_PLAYER& player)
{
	using namespace LostArk::Shared;
	// Keep the stage and return pin during descent; death, not fall start,
	// releases Mario. A dead return never completes the minigame successfully.
	if (WORLD_ID::KOUKUSAYDON_ARENA == m_eWorldId && player.iMarioStage <= 4u &&
		(player.iMarioStage > 0u || player.MarioReturnPosition.has_value()))
	{
		if (PLAYER_ACTION_STATE::FALLING == player.eAction && player.iCurrentHp) return;
		if (!player.iCurrentHp || PLAYER_ACTION_STATE::DEAD == player.eAction)
		{
			SERVER_NAV_POINT destination{};
			if (!Resolve_MarioReturnDestination(player, destination)) return;
			player.fPositionX = destination.x;
			player.fPositionY = destination.y;
			player.fPositionZ = destination.z;
			player.Clear_MarioControl();
			player.KoukuFallRevivePosition.reset();
			player.bKoukuFallDeath = false;
			player.Clear_KoukuInteractionState();
			player.eMadnessForm = PLAYER_MADNESS_FORM::NORMAL;
			player.TriggerMove = {};
			player.PendingCommand.Clear();
			player.Clear_Attachment();
			player.Clear_PatternBindStatus();
			player.fKnockbackRemainingSeconds = player.fKnockbackSpeed = 0.f;
			player.bArenaEjectionActive = player.bKnockbackBallistic = false;
			player.iCurrentHp = 0u;
			player.eAction = PLAYER_ACTION_STATE::DEAD;
			player.isCombatReady = false;
			m_CombatObjectRuntime.Cancel_Source(player.iNetEntityId);
			m_ServerTriggerSystem.Remove_Player(player.iPlayerId);
			return;
		}
	}
	if (WORLD_ID::KOUKUSAYDON_ARENA != m_eWorldId || player.iMarioStage > 4u ||
		0u == player.iCurrentHp || PLAYER_ACTION_STATE::DEAD == player.eAction ||
		PLAYER_ACTION_STATE::FALLING == player.eAction)
	{
		player.Clear_MarioControl();
		return;
	}
	if (0u != player.iMarioStage)
	{
		/* A finish is the terminal exit in this stage's authored lane graph.
		Its destination can be edited to the arena rather than the old go box. */
		const bool terminalExit = player.TriggerMove.isActive &&
			std::any_of(MARIO_LANES.begin(), MARIO_LANES.end(), [&player](const auto& lane) {
				return lane.stage == player.iMarioStage && player.TriggerMove.strSourcePlacementId == lane.exit;
			}) && !std::any_of(MARIO_LANES.begin(), MARIO_LANES.end(), [&player](const auto& lane) {
				return lane.stage == player.iMarioStage && player.TriggerMove.strSourcePlacementId == lane.arrival;
			});
		const auto* entrance = Find_Placement("Mario" + std::to_string(player.iMarioStage) + "_go");
		if (terminalExit || (player.TriggerMove.isActive && nullptr != entrance &&
			std::abs(player.TriggerMove.fTargetX - entrance->fPositionX) < 0.05f &&
			std::abs(player.TriggerMove.fTargetY - entrance->fPositionY) < 0.05f &&
			std::abs(player.TriggerMove.fTargetZ - entrance->fPositionZ) < 0.05f))
		{
			player.Clear_MarioControl(true);
			return;
		}
		player.eMadnessForm = PLAYER_MADNESS_FORM::CLOWN;
		player.eKoukuAreaHudMode = KOUKU_HUD_MODE::MARIO;
		if (!player.bMarioRailReady && !player.TriggerMove.isActive &&
			PLAYER_ACTION_STATE::NONE == player.eAction)
			(void)Configure_MarioRail(player, player.strMarioRailArrivalId);
		return;
	}
	if (player.eMadnessForm != PLAYER_MADNESS_FORM::CLOWN ||
		player.bPatternBound || INVALID_NET_ENTITY_ID != player.iAttachmentOwnerNetEntityId ||
		(PLAYER_ACTION_STATE::NONE != player.eAction &&
			PLAYER_ACTION_STATE::TRIGGER_MOVE != player.eAction &&
			PLAYER_ACTION_STATE::WALL_CLIMB != player.eAction))
		return;
	// Returning directly from an Intro box must not immediately admit that stage again.
	if (player.TriggerMove.isActive && std::any_of(MARIO_LANES.begin(), MARIO_LANES.end(), [&player](const auto& lane) {
		return player.TriggerMove.strSourcePlacementId == lane.exit && !std::any_of(MARIO_LANES.begin(), MARIO_LANES.end(),
			[&lane](const auto& next) { return next.stage == lane.stage && std::string_view(next.arrival) == lane.exit; });
	})) return;
	for (std::uint8_t stage = 1u; stage <= 4u; ++stage)
	{
		const auto* intro = Find_Placement("Mario" + std::to_string(stage) + "_Intro");
		if (nullptr == intro || !intro->isEnabled || WORLD_BOOTSTRAP_KIND::TRIGGER_BOX != intro->eKind ||
			!CServerTriggerSystem::Contains_Placement(*intro, player))
			continue;
		const std::uint8_t layout = Begin_MarioStageObjects(stage);
		if (!layout)
		{
			if ((m_iServerTick % 30u) == 0u) std::cerr << "[MarioLayout] " << m_strStatus << '\n';
			return;
		}
		player.iMarioLayoutVariant = layout;
		player.ePreMarioForm = player.eMadnessForm;
		player.iMarioStage = stage;
		Begin_MarioBallChallenge(player);
		player.eMadnessForm = PLAYER_MADNESS_FORM::CLOWN;
		/* MARIO is what deals the two interaction slots and what the Client
		checks before it submits Q/W at all. The only authored source is
		Mario1_go's movePlayer, and every MarioN_go is a disabled config row
		whose action never fires, so the stage sets it here instead. */
		player.eKoukuAreaHudMode = KOUKU_HUD_MODE::MARIO;
		player.strMarioRailArrivalId = "Mario" + std::to_string(stage) + "_go";
		player.bMarioRailReady = false;
		player.iMarioMoveExpiryTick = 0u;
		player.fMarioDirectionX = player.fMarioDirectionZ = 0.f;
		player.hasMoveGoal = false;
		player.MovePath.clear();
		player.iMovePathIndex = 0u;
		player.PendingCommand.Clear();
		if (!player.TriggerMove.isActive)
			(void)Configure_MarioRail(player, player.strMarioRailArrivalId);
		std::cout << "[MarioControl] player=" << player.iPlayerId << " stage=" << static_cast<unsigned>(stage)
			<< " layout=" << static_cast<unsigned>(layout) << '\n';
		return;
	}
}

bool LostArk::Server::CGameRoom::Resolve_MarioReturnDestination(
	const SERVER_PLAYER& player, SERVER_NAV_POINT& outPoint) const
{
	std::array<float, 3u> position{};
	if (player.MarioReturnPosition) position = *player.MarioReturnPosition;
	else
	{
		const auto* gate3 = Find_Placement("stage.kouku.sl05");
		if (!gate3 || gate3->eKind != WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN) return false;
		position = { gate3->fPositionX, gate3->fPositionY, gate3->fPositionZ };
	}
	return std::all_of(position.begin(), position.end(), [](const float value) { return std::isfinite(value); }) &&
		m_ServerNavigation.Is_PointWalkableExact(position[0], position[2], position[1]) &&
		m_ServerNavigation.Sample_Position(position[0], position[2], outPoint, position[1]) &&
		std::abs(position[1] - outPoint.y) <= .25f;
}

void LostArk::Server::CGameRoom::Handle_MarioReturn(
	const SESSION_ID sessionId, const LostArk::Shared::C2S_MARIO_RETURN& request)
{
	using namespace LostArk::Shared;
	const auto session = Find_Session(sessionId);
	if (!session) return;
	S2C_MARIO_RETURN_RESULT result{};
	result.iClientSequence = request.iClientSequence;
	result.eWorldId = m_eWorldId;
	result.eResult = MARIO_RETURN_RESULT::REJECTED_PLAYER_STATE;
	const auto binding = m_PlayerIdBySessionId.find(sessionId);
	const auto player = binding == m_PlayerIdBySessionId.end() ? m_Players.end() : m_Players.find(binding->second);
	if (player != m_Players.end() && player->second.iSessionId == sessionId)
		result = Apply_MarioReturn(player->second, request);
	CPacketWriter writer;
	if (!Write_Message(writer, result) || !session->Send_Frame(
		PACKET_TYPE::S2C_MARIO_RETURN_RESULT, writer.Get_Buffer())) session->Request_Close();
}

LostArk::Shared::S2C_MARIO_RETURN_RESULT LostArk::Server::CGameRoom::Apply_MarioReturn(
	SERVER_PLAYER& player, const LostArk::Shared::C2S_MARIO_RETURN& request)
{
	using namespace LostArk::Shared;
	S2C_MARIO_RETURN_RESULT result{};
	result.iClientSequence = request.iClientSequence;
	result.eWorldId = m_eWorldId;
	if (m_eWorldId != WORLD_ID::KOUKUSAYDON_ARENA || request.eWorldId != m_eWorldId)
	{ result.eResult = MARIO_RETURN_RESULT::REJECTED_WRONG_WORLD; return result; }
	const auto& previous = player.LastMarioReturnResult;
	if (request.iClientSequence && request.iClientSequence == previous.iClientSequence) return previous;
	if (!Is_NewerSequence(request.iClientSequence, previous.iClientSequence))
	{ result.eResult = MARIO_RETURN_RESULT::REJECTED_STALE_SEQUENCE; return result; }
	const auto reject = [&player, &result](const MARIO_RETURN_RESULT reason) {
		result.eResult = reason; player.LastMarioReturnResult = result; return result;
	};
	if (!player.iMarioStage || player.iMarioStage > 4u) return reject(MARIO_RETURN_RESULT::REJECTED_OUTSIDE_MARIO);
	if (!player.isCombatReady || !player.iCurrentHp || player.eAction != PLAYER_ACTION_STATE::NONE ||
		player.TriggerMove.isActive || player.bPatternBound || player.fKnockbackRemainingSeconds > 0.f ||
		player.iAttachmentOwnerNetEntityId != INVALID_NET_ENTITY_ID)
		return reject(MARIO_RETURN_RESULT::REJECTED_PLAYER_STATE);
	if (Mario_MatchingBallCount(player) < 3u)
	{
		m_strStatus = "Break three balls of the marked colour before leaving Mario";
		return reject(MARIO_RETURN_RESULT::REJECTED_PLAYER_STATE);
	}
	// Resolve the terminal authored exit of this stage; the Client supplies no target.
	const auto lane = std::find_if(MARIO_LANES.begin(), MARIO_LANES.end(), [&player](const auto& candidate) {
		return candidate.stage == player.iMarioStage && !std::any_of(MARIO_LANES.begin(), MARIO_LANES.end(),
			[&candidate](const auto& next) { return next.stage == candidate.stage && std::string_view(next.arrival) == candidate.exit; });
	});
	const auto* exit = lane == MARIO_LANES.end() ? nullptr : Find_Placement(lane->exit);
	if (!exit || !exit->isEnabled || exit->eKind != WORLD_BOOTSTRAP_KIND::TRIGGER_BOX ||
		exit->TriggerActions.size() != 1u)
		return reject(MARIO_RETURN_RESULT::REJECTED_DESTINATION);
	const auto& action = exit->TriggerActions.front();
	SERVER_NAV_POINT ground{};
	if (action.eKind != WORLD_TRIGGER_ACTION_KIND::MOVE_PLAYER || !std::isfinite(action.fDurationSeconds) ||
		action.fDurationSeconds <= 0.f || !std::isfinite(action.fArcHeight) ||
		!std::isfinite(action.fTargetX) || !std::isfinite(action.fTargetY) || !std::isfinite(action.fTargetZ) ||
		!Resolve_MarioReturnDestination(player, ground) ||
		(!player.MarioReturnPosition && (std::abs(action.fTargetX - ground.x) > .05f ||
			std::abs(action.fTargetZ - ground.z) > .05f || std::abs(action.fTargetY - ground.y) > .25f)))
		return reject(MARIO_RETURN_RESULT::REJECTED_DESTINATION);
	Refresh_PlayerBlockingBodies();
	if (!m_ServerCollisionSystem.Is_PlayerPositionClear(ground.x, ground.y, ground.z, player.iNetEntityId))
		return reject(MARIO_RETURN_RESULT::REJECTED_DESTINATION);
	SERVER_PLAYER candidate = player;
	WORLD_TRIGGER_ACTION move = action;
	move.fTargetX = ground.x; move.fTargetY = ground.y; move.fTargetZ = ground.z;
	if (!CServerTriggerSystem::Begin_MovePlayer(candidate, move, m_iServerTick ? m_iServerTick : 1u))
		return reject(MARIO_RETURN_RESULT::REJECTED_PLAYER_STATE);
	candidate.TriggerMove.strSourcePlacementId = exit->strPlacementId;
	Update_MarioControlState(candidate);
	result.eResult = MARIO_RETURN_RESULT::ACCEPTED;
	candidate.LastMarioReturnResult = result;
	player = std::move(candidate);
	m_strStatus = "Mario return started; phase progression waits for the authoritative landing";
	return result;
}

void LostArk::Server::CGameRoom::Handle_MarioMove(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_MARIO_MOVE& request)
{
	using namespace LostArk::Shared;
	const auto binding = m_PlayerIdBySessionId.find(sessionId);
	if (binding == m_PlayerIdBySessionId.end())
		return;
	const auto found = m_Players.find(binding->second);
	if (found == m_Players.end() || found->second.iSessionId != sessionId)
		return;
	SERVER_PLAYER& player = found->second;
	const bool stop = MARIO_DIRECTION::STOP == request.eDirection;
	if (WORLD_ID::KOUKUSAYDON_ARENA != m_eWorldId || request.eWorldId != m_eWorldId ||
		!Is_NewerSequence(request.iClientSequence, player.iLastMarioMoveSequence) ||
		(!stop && MARIO_DIRECTION::LEFT != request.eDirection && MARIO_DIRECTION::RIGHT != request.eDirection))
		return;
	player.iLastMarioMoveSequence = request.iClientSequence;
	if (0u == player.iMarioStage || player.iMarioStage > 4u || 0u == player.iCurrentHp)
		return;
	/* A key-up packet during a jump only releases the directional lease. It
		never cancels the independently owned scripted motion or its action. */
	if (stop)
	{
		player.iMarioMoveExpiryTick = 0u;
		player.fMarioDirectionX = player.fMarioDirectionZ = 0.f;
		player.hasMoveGoal = false;
		player.MovePath.clear();
		player.iMovePathIndex = 0u;
		return;
	}
	if (!player.bMarioRailReady || PLAYER_ACTION_STATE::NONE != player.eAction || player.bPatternBound ||
		player.TriggerMove.isActive || player.fKnockbackRemainingSeconds > 0.f ||
		INVALID_NET_ENTITY_ID != player.iAttachmentOwnerNetEntityId)
		return;
	const float sign = MARIO_DIRECTION::RIGHT == request.eDirection ? 1.f : -1.f;
	player.fMarioDirectionX = player.fMarioRailRightX * sign;
	player.fMarioDirectionZ = player.fMarioRailRightZ * sign;
	player.iMarioMoveExpiryTick = m_iServerTick + 9u;
	if (0u == player.iMarioMoveExpiryTick)
		player.iMarioMoveExpiryTick = 1u;
}

void LostArk::Server::CGameRoom::Update_MarioMoveGoal(
	SERVER_PLAYER& player, const std::uint32_t updateTick)
{
	if (0u == player.iMarioStage)
		return;
	if (!player.bMarioRailReady || 0u == player.iMarioMoveExpiryTick || Has_ReachedServerTick(updateTick, player.iMarioMoveExpiryTick))
	{
		player.iMarioMoveExpiryTick = 0u;
		player.fMarioDirectionX = player.fMarioDirectionZ = 0.f;
		player.hasMoveGoal = false;
		player.MovePath.clear();
		player.iMovePathIndex = 0u;
		return;
	}
	/* Feed the ordinary one-step navigation/collision mover, not Find_Path:
		left/right must stop at a gap rather than walking an A* detour. */
	if (!player.MovePath.empty())
		player.MovePath.clear();
	player.iMovePathIndex = 0u;
	player.fMoveGoalX = player.fPositionX + player.fMarioDirectionX * 0.75f;
	player.fMoveGoalZ = player.fPositionZ + player.fMarioDirectionZ * 0.75f;
	Project_MarioRailPoint(player, player.fMoveGoalX, player.fMoveGoalZ);
	player.hasMoveGoal = true;
}

void LostArk::Server::CGameRoom::Handle_DebugMarioJump(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_DEBUG_MARIO_JUMP& request)
{
	using namespace LostArk::Shared;
	const auto session = Find_Session(sessionId);
	if (nullptr == session)
		return;
	S2C_DEBUG_MARIO_JUMP_RESULT result{};
	result.iClientSequence = request.iClientSequence;
	result.eWorldId = m_eWorldId;
	result.eResult = DEBUG_MARIO_JUMP_RESULT::REJECTED_PLAYER_STATE;
	const auto binding = m_PlayerIdBySessionId.find(sessionId);
	const auto player = binding == m_PlayerIdBySessionId.end() ?
		m_Players.end() : m_Players.find(binding->second);
	if (player != m_Players.end() && player->second.iSessionId == sessionId)
		result = Apply_DebugMarioJump(player->second, request);
	CPacketWriter writer;
	if (!Write_Message(writer, result) || !session->Send_Frame(
		PACKET_TYPE::S2C_DEBUG_MARIO_JUMP_RESULT, writer.Get_Buffer()))
	{
		session->Request_Close();
	}
	std::cout << "[DebugMarioJump] session=" << sessionId
		<< " sequence=" << result.iClientSequence
		<< " result=" << static_cast<unsigned>(result.eResult) << '\n';
}

LostArk::Shared::S2C_DEBUG_MARIO_JUMP_RESULT
LostArk::Server::CGameRoom::Apply_DebugMarioJump(
	SERVER_PLAYER& player,
	const LostArk::Shared::C2S_DEBUG_MARIO_JUMP& request)
{
	using namespace LostArk::Shared;
	S2C_DEBUG_MARIO_JUMP_RESULT result{};
	result.iClientSequence = request.iClientSequence;
	result.eWorldId = m_eWorldId;
#ifndef _DEBUG
	// The legacy wire name is retained; Release admits only a live raid roster actor.
	if (!Is_KoukuRaidRunning() || (m_KoukuRaid.State.ePhase != KOUKUSAYDON_RAID_PHASE::COMBAT &&
		m_KoukuRaid.State.ePhase != KOUKUSAYDON_RAID_PHASE::WAIT_MINIGAME) ||
		std::find(m_KoukuRaid.PlayerIds.begin(), m_KoukuRaid.PlayerIds.end(), player.iPlayerId) == m_KoukuRaid.PlayerIds.end() ||
		!m_PlayerIdBySessionId.contains(player.iSessionId) || m_PlayerIdBySessionId.at(player.iSessionId) != player.iPlayerId)
	{
		result.eResult = DEBUG_MARIO_JUMP_RESULT::REJECTED_DISABLED;
		return result;
	}
#endif
	if (WORLD_ID::KOUKUSAYDON_ARENA != m_eWorldId || request.eWorldId != m_eWorldId)
	{
		result.eResult = DEBUG_MARIO_JUMP_RESULT::REJECTED_WRONG_WORLD;
		return result;
	}
	const auto& previous = player.LastDebugMarioJumpResult;
	if (0u != request.iClientSequence && request.iClientSequence == previous.iClientSequence)
		return previous;
	if (!Is_NewerSequence(request.iClientSequence, previous.iClientSequence))
	{
		result.eResult = DEBUG_MARIO_JUMP_RESULT::REJECTED_STALE_SEQUENCE;
		return result;
	}
	const auto reject = [&player, &result](const DEBUG_MARIO_JUMP_RESULT reason)
	{
		result.eResult = reason;
		player.LastDebugMarioJumpResult = result;
		return result;
	};
	if (0u == player.iMarioStage || player.iMarioStage > 4u || !player.bMarioRailReady)
		return reject(DEBUG_MARIO_JUMP_RESULT::REJECTED_OUTSIDE_MARIO);
	if (0u == player.iCurrentHp || PLAYER_ACTION_STATE::NONE != player.eAction ||
		player.TriggerMove.isActive || player.bPatternBound || player.bArenaEjectionActive ||
		player.fKnockbackRemainingSeconds > 0.f || player.iKnockdownEndTick > m_iServerTick ||
		INVALID_NET_ENTITY_ID != player.iAttachmentOwnerNetEntityId)
		return reject(DEBUG_MARIO_JUMP_RESULT::REJECTED_PLAYER_STATE);
	if (MARIO_DIRECTION::LEFT != request.eDirection && MARIO_DIRECTION::RIGHT != request.eDirection)
		return reject(DEBUG_MARIO_JUMP_RESULT::REJECTED_INVALID_TARGET);

	const auto sameStageGrid = [this, &player](const float x, const float z)
	{
		return 1u == player.iMarioStage ?
			m_ServerNavigation.Is_InSameNavigationGrid(player.fPositionX, player.fPositionZ, x, z) :
			m_ServerNavigation.Is_InSameDetailRegion(player.fPositionX, player.fPositionZ, x, z);
	};
	const auto* entrance = Find_Placement("Mario" + std::to_string(player.iMarioStage) + "_go");
	if (nullptr == entrance || entrance->TriggerActions.size() != 1u ||
		WORLD_TRIGGER_ACTION_KIND::MOVE_PLAYER != entrance->TriggerActions.front().eKind ||
		!sameStageGrid(entrance->TriggerActions.front().fTargetX, entrance->TriggerActions.front().fTargetZ))
		return reject(DEBUG_MARIO_JUMP_RESULT::REJECTED_OUTSIDE_MARIO);
	SERVER_NAV_POINT startGround{};
	if (!m_ServerNavigation.Is_PointWalkableExact(player.fPositionX, player.fPositionZ) ||
		!m_ServerNavigation.Sample_Position(player.fPositionX, player.fPositionZ, startGround) ||
		!std::isfinite(player.fPositionY) || !std::isfinite(startGround.y) ||
		std::abs(player.fPositionY - startGround.y) > 0.25f)
		return reject(DEBUG_MARIO_JUMP_RESULT::REJECTED_PLAYER_STATE);

	const float sign = MARIO_DIRECTION::RIGHT == request.eDirection ? 1.f : -1.f;
	const float directionX = player.fMarioRailRightX * sign;
	const float directionZ = player.fMarioRailRightZ * sign;
	Refresh_PlayerBlockingBodies();
	for (int quarterMetres = 16; quarterMetres >= 3; --quarterMetres)
	{
		const float leapDistance = static_cast<float>(quarterMetres) * 0.25f;
		float x = player.fPositionX + directionX * leapDistance;
		float z = player.fPositionZ + directionZ * leapDistance;
		Project_MarioRailPoint(player, x, z);
		SERVER_NAV_POINT landing{};
		if (!sameStageGrid(x, z) ||
			!m_ServerNavigation.Is_PointWalkableExact(x, z) ||
			!m_ServerNavigation.Sample_Position(x, z, landing) || !std::isfinite(landing.y) ||
			std::abs(landing.y - startGround.y) > 1.f ||
			!m_ServerCollisionSystem.Is_PlayerPositionClear(x, landing.y, z, player.iNetEntityId))
			continue;
		WORLD_TRIGGER_ACTION jump{};
		jump.eKind = WORLD_TRIGGER_ACTION_KIND::MOVE_PLAYER;
		jump.fTargetX = landing.x;
		jump.fTargetY = landing.y;
		jump.fTargetZ = landing.z;
		jump.fDurationSeconds = 0.6f;
		jump.fArcHeight = 1.5f;
		if (!CServerTriggerSystem::Begin_MovePlayer(player, jump, 0u == m_iServerTick ? 1u : m_iServerTick))
			return reject(DEBUG_MARIO_JUMP_RESULT::REJECTED_PLAYER_STATE);
		player.iMarioMoveExpiryTick = 0u;
		player.fMarioDirectionX = player.fMarioDirectionZ = 0.f;
		/* Begin_MovePlayer only changes the scripted displacement/action. HP,
		world-sequence state and trigger membership intentionally stay intact. */
		result.eResult = DEBUG_MARIO_JUMP_RESULT::ACCEPTED;
		player.LastDebugMarioJumpResult = result;
		return result;
	}
	return reject(DEBUG_MARIO_JUMP_RESULT::REJECTED_NO_LANDING);
}
```


## Server/Private/GameplayCatalog.cpp 전체 코드

수정 대상: [GameplayCatalog.cpp](C:/Users/tnest/Desktop/LostArk/Server/Private/GameplayCatalog.cpp)

```cpp
#include "GameplayCatalog.h"
#include "KoukuSaydonBrain.h"

#include <Windows.h>
#include <bcrypt.h>

#include <algorithm>
#include <charconv>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <iomanip>
#include <map>
#include <locale>
#include <tuple>
#include <limits>
#include <sstream>
#include <string_view>
#include <unordered_set>
#include <vector>

#pragma comment(lib, "bcrypt.lib")

namespace
{
	/* publisher의 maximumDamageRatePercent와 같은 상한을 사용해 검증·로드의 허용 범위를 맞춘다. */
	constexpr std::uint32_t MAXIMUM_DAMAGE_RATE_PERCENT = 100000u;
	// Matches the publisher bound for the complete gameplay document.
	constexpr std::uint32_t MAXIMUM_GAMEPLAY_BOOTSTRAP_ROWS =
		LostArk::Shared::GAMEPLAY_BOOTSTRAP_MAX_ROWS;
	/* snapshot은 bit마다 한 plate를 나타내므로 전송 가능한 bit 수 안으로 부위 개수를 제한한다. */
	constexpr std::size_t MAXIMUM_BOSS_ARMOR_PLATES =
		LostArk::Shared::MAX_WORLD_ENTITY_ARMOR_PLATES;

	bool Calculate_GameplayDataRevision(
		const std::string_view payload,
		LostArk::Shared::GameplayDataRevision& outRevision)
	{
		if (payload.size() > (std::numeric_limits<ULONG>::max)())
			return false;

		BCRYPT_ALG_HANDLE algorithm = nullptr;
		BCRYPT_HASH_HANDLE hash = nullptr;
		DWORD hashObjectSize = 0u;
		DWORD bytesWritten = 0u;
		DWORD hashSize = 0u;
		std::vector<unsigned char> hashObject;
		LostArk::Shared::GameplayDataRevision revision{};
		bool succeeded = false;
		if (0 <= BCryptOpenAlgorithmProvider(
				&algorithm, BCRYPT_SHA256_ALGORITHM, nullptr, 0u) &&
			0 <= BCryptGetProperty(
				algorithm, BCRYPT_OBJECT_LENGTH,
				reinterpret_cast<PUCHAR>(&hashObjectSize),
				sizeof(hashObjectSize), &bytesWritten, 0u) &&
			0 <= BCryptGetProperty(
				algorithm, BCRYPT_HASH_LENGTH,
				reinterpret_cast<PUCHAR>(&hashSize),
				sizeof(hashSize), &bytesWritten, 0u) &&
			hashSize == revision.Bytes.size())
		{
			hashObject.resize(hashObjectSize);
			if (0 <= BCryptCreateHash(
					algorithm, &hash, hashObject.data(), hashObjectSize,
					nullptr, 0u, 0u) &&
				0 <= BCryptHashData(
					hash,
					reinterpret_cast<PUCHAR>(
						const_cast<char*>(payload.data())),
					static_cast<ULONG>(payload.size()), 0u) &&
				0 <= BCryptFinishHash(
					hash, revision.Bytes.data(), hashSize, 0u) &&
				revision.Is_Valid())
			{
				succeeded = true;
			}
		}
		if (nullptr != hash)
			BCryptDestroyHash(hash);
		if (nullptr != algorithm)
			BCryptCloseAlgorithmProvider(algorithm, 0u);
		if (succeeded)
			outRevision = revision;
		return succeeded;
	}

	std::filesystem::path Resolve_DataRoot()
	{
		wchar_t configured[32768]{};
		const DWORD configuredLength = GetEnvironmentVariableW(
			L"LOSTARK_SERVER_DATA_ROOT", configured,
			static_cast<DWORD>(std::size(configured)));
		if (0u != configuredLength && configuredLength < std::size(configured))
			return std::filesystem::path(configured).lexically_normal();

		wchar_t modulePath[32768]{};
		const DWORD moduleLength = GetModuleFileNameW(
			nullptr, modulePath, static_cast<DWORD>(std::size(modulePath)));
		if (0u == moduleLength || moduleLength >= std::size(modulePath))
			return {};
		return std::filesystem::path(modulePath).parent_path().parent_path() /
			L"DataFiles";
	}

	std::vector<std::string_view> SplitTabs(const std::string& line)
	{
		std::vector<std::string_view> fields;
		const std::string_view view(line);
		std::size_t start = 0;
		while (true)
		{
			const std::size_t tab = view.find('\t', start);
			fields.push_back(view.substr(
				start, std::string_view::npos == tab ? tab : tab - start));
			if (std::string_view::npos == tab)
				break;
			start = tab + 1;
		}
		return fields;
	}

	void StripCarriageReturn(std::string& line)
	{
		if (!line.empty() && '\r' == line.back())
			line.pop_back();
	}

	template<typename T>
	bool ParseNumber(const std::string_view value, T& output)
	{
		const auto result = std::from_chars(
			value.data(), value.data() + value.size(), output);
		return std::errc{} == result.ec &&
			result.ptr == value.data() + value.size();
	}

	bool IsStableId(const std::string_view value)
	{
		return !value.empty() && value.size() <= 128u &&
			std::all_of(value.begin(), value.end(), [](const unsigned char character)
			{
				return 0 != std::isalnum(character) || character == '_' ||
					character == '-' || character == '.';
			});
	}

	bool ParseSkillKind(
		const std::string_view value,
		LostArk::Shared::PLAYER_SKILL_KIND& output)
	{
		using LostArk::Shared::PLAYER_SKILL_KIND;
		if ("ACTIVE" == value)
			output = PLAYER_SKILL_KIND::ACTIVE;
		else if ("COMBO" == value)
			output = PLAYER_SKILL_KIND::COMBO;
		else if ("HOLD" == value)
			output = PLAYER_SKILL_KIND::HOLD;
		else if ("COUNTER" == value)
			output = PLAYER_SKILL_KIND::COUNTER;
		else if ("STANDUP" == value)
			output = PLAYER_SKILL_KIND::STANDUP;
		else
			return false;
		return true;
	}

	bool ParseCharacterClass(
		const std::string_view value,
		LostArk::Shared::CHARACTER_CLASS_ID& output)
	{
		using LostArk::Shared::CHARACTER_CLASS_ID;
		if ("LANCE_MASTER" == value)
			output = CHARACTER_CLASS_ID::LANCE_MASTER;
		else if ("GUNSLINGER" == value)
			output = CHARACTER_CLASS_ID::GUNSLINGER;
		else if ("SLAYER" == value)
			output = CHARACTER_CLASS_ID::SLAYER;
		else if ("ARTIST" == value)
			output = CHARACTER_CLASS_ID::ARTIST;
		else if ("DIMENSIONMASTER" == value)
			output = CHARACTER_CLASS_ID::DIMENSIONMASTER;
		else if ("WARLORD" == value)
			output = CHARACTER_CLASS_ID::WARLORD;
		else if ("GUARDIANKNIGHT" == value)
			output = CHARACTER_CLASS_ID::GUARDIANKNIGHT;
		else
			return false;
		return true;
	}

	bool ParseStance(
		const std::string_view value,
		LostArk::Shared::PLAYER_STANCE_ID& output)
	{
		using LostArk::Shared::PLAYER_STANCE_ID;
		if ("NONE" == value)
			output = PLAYER_STANCE_ID::NONE;
		else if ("LANCE_MASTER_LONG_SPEAR" == value)
			output = PLAYER_STANCE_ID::LANCE_MASTER_LONG_SPEAR;
		else if ("LANCE_MASTER_SHORT_SPEAR" == value)
			output = PLAYER_STANCE_ID::LANCE_MASTER_SHORT_SPEAR;
		else if ("WARLORD_NORMAL" == value)
			output = PLAYER_STANCE_ID::WARLORD_NORMAL;
		else if ("WARLORD_DEFENSE" == value)
			output = PLAYER_STANCE_ID::WARLORD_DEFENSE;
		else if ("GUARDIANKNIGHT_HUMAN" == value)
			output = PLAYER_STANCE_ID::GUARDIANKNIGHT_HUMAN;
		else if ("GUARDIANKNIGHT_DRAGON" == value)
			output = PLAYER_STANCE_ID::GUARDIANKNIGHT_DRAGON;
		else
			return false;
		return true;
	}

	bool ParseBossPatternSelection(
		const std::string_view value,
		LostArk::Server::BOSS_PATTERN_SELECTION& output)
	{
		using LostArk::Server::BOSS_PATTERN_SELECTION;
		if ("NORMAL" == value)
			output = BOSS_PATTERN_SELECTION::NORMAL;
		else if ("HEALTH_BAR" == value)
			output = BOSS_PATTERN_SELECTION::HEALTH_BAR;
		else if ("AUDITION_ONLY" == value)
			output = BOSS_PATTERN_SELECTION::AUDITION_ONLY;
		else
			return false;
		return true;
	}

	bool ParseBossPatternRotationSelectionMode(
		const std::string_view value,
		LostArk::Server::BOSS_PATTERN_ROTATION_SELECTION_MODE& output)
	{
		using LostArk::Server::BOSS_PATTERN_ROTATION_SELECTION_MODE;
		if ("WEIGHTED_POOL" == value)
			output = BOSS_PATTERN_ROTATION_SELECTION_MODE::WEIGHTED_POOL;
		else if ("ORDERED_LOOP" == value)
			output = BOSS_PATTERN_ROTATION_SELECTION_MODE::ORDERED_LOOP;
		else if ("ORDERED_INTRO_THEN_WEIGHTED" == value)
			output = BOSS_PATTERN_ROTATION_SELECTION_MODE::
				ORDERED_INTRO_THEN_WEIGHTED;
		else
			return false;
		return true;
	}

	bool ParseBossPatternArmorRequirement(
		const std::string_view value,
		LostArk::Server::BOSS_PATTERN_ARMOR_REQUIREMENT& output)
	{
		using LostArk::Server::BOSS_PATTERN_ARMOR_REQUIREMENT;
		if ("ANY" == value)
			output = BOSS_PATTERN_ARMOR_REQUIREMENT::ANY;
		else if ("ARMORED" == value)
			output = BOSS_PATTERN_ARMOR_REQUIREMENT::ARMORED;
		else if ("STRIPPED" == value)
			output = BOSS_PATTERN_ARMOR_REQUIREMENT::STRIPPED;
		else
			return false;
		return true;
	}

	bool ParseBossPatternPhaseRequirement(
		const std::string_view value,
		LostArk::Server::BOSS_PATTERN_PHASE_REQUIREMENT& output)
	{
		using LostArk::Server::BOSS_PATTERN_PHASE_REQUIREMENT;
		if ("ANY" == value)
			output = BOSS_PATTERN_PHASE_REQUIREMENT::ANY;
		else if ("PHASE_ONE" == value)
			output = BOSS_PATTERN_PHASE_REQUIREMENT::PHASE_ONE;
		else if ("PHASE_TWO" == value)
			output = BOSS_PATTERN_PHASE_REQUIREMENT::PHASE_TWO;
		else
			return false;
		return true;
	}

	bool ParseBossPatternHitShape(
		const std::string_view value,
		LostArk::Server::BOSS_PATTERN_HIT_SHAPE& output)
	{
		using LostArk::Server::BOSS_PATTERN_HIT_SHAPE;
		if ("NONE" == value)
			output = BOSS_PATTERN_HIT_SHAPE::NONE;
		else if ("CIRCLE" == value)
			output = BOSS_PATTERN_HIT_SHAPE::CIRCLE;
		else if ("RING" == value)
			output = BOSS_PATTERN_HIT_SHAPE::RING;
		else if ("CONE" == value)
			output = BOSS_PATTERN_HIT_SHAPE::CONE;
		else if ("BOX" == value)
			output = BOSS_PATTERN_HIT_SHAPE::BOX;
		else if ("CROSS" == value)
			output = BOSS_PATTERN_HIT_SHAPE::CROSS;
		else if ("SIX_DIRECTIONS" == value)
			output = BOSS_PATTERN_HIT_SHAPE::SIX_DIRECTIONS;
		else
			return false;
		return true;
	}

	bool ParseBossPatternPlayerResponse(
		const std::string_view value,
		LostArk::Server::BOSS_PATTERN_PLAYER_RESPONSE& output)
	{
		using LostArk::Server::BOSS_PATTERN_PLAYER_RESPONSE;
		if ("DAMAGE" == value)
			output = BOSS_PATTERN_PLAYER_RESPONSE::DAMAGE;
		else if ("CAPTURE" == value)
			output = BOSS_PATTERN_PLAYER_RESPONSE::CAPTURE;
		else
			return false;
		return true;
	}

	bool ParseBossPatternBossResponseKind(
		const std::string_view value,
		LostArk::Server::BOSS_PATTERN_BOSS_RESPONSE_KIND& output)
	{
		using LostArk::Server::BOSS_PATTERN_BOSS_RESPONSE_KIND;
		if ("ACCUMULATED_HEALTH_DAMAGE" == value)
		{
			output = BOSS_PATTERN_BOSS_RESPONSE_KIND::
				ACCUMULATED_HEALTH_DAMAGE;
		}
		else
		{
			return false;
		}
		return true;
	}

	bool ParseBossPatternCounterProxyKind(
		const std::string_view value,
		LostArk::Server::BOSS_PATTERN_COUNTER_PROXY_KIND& output)
	{
		using LostArk::Server::BOSS_PATTERN_COUNTER_PROXY_KIND;
		if ("BOSS_LOCAL_CIRCLE" == value)
			output = BOSS_PATTERN_COUNTER_PROXY_KIND::BOSS_LOCAL_CIRCLE;
		else if ("BOSS_FORWARD_ARC" == value)
			output = BOSS_PATTERN_COUNTER_PROXY_KIND::BOSS_FORWARD_ARC;
		else
			return false;
		return true;
	}

	bool ParseBossPatternHitAnchorKind(
		const std::string_view value,
		LostArk::Server::BOSS_PATTERN_HIT_ANCHOR_KIND& output)
	{
		using LostArk::Server::BOSS_PATTERN_HIT_ANCHOR_KIND;
		if ("BOSS_CURRENT" == value)
			output = BOSS_PATTERN_HIT_ANCHOR_KIND::BOSS_CURRENT;
		else if ("STAGE_ORIGIN" == value)
			output = BOSS_PATTERN_HIT_ANCHOR_KIND::STAGE_ORIGIN;
		else
			return false;
		return true;
	}

	bool ParseBossPatternHitActivationKind(
		const std::string_view value,
		LostArk::Server::BOSS_PATTERN_HIT_ACTIVATION_KIND& output)
	{
		using LostArk::Server::BOSS_PATTERN_HIT_ACTIVATION_KIND;
		if ("PULSE_SCHEDULE" == value)
			output = BOSS_PATTERN_HIT_ACTIVATION_KIND::PULSE_SCHEDULE;
		else if ("ACTIVE_WINDOW" == value)
			output = BOSS_PATTERN_HIT_ACTIVATION_KIND::ACTIVE_WINDOW;
		else
			return false;
		return true;
	}

	bool ParsePlayerAttachmentSlot(
		const std::string_view value,
		LostArk::Shared::PLAYER_ATTACHMENT_SLOT& output)
	{
		using LostArk::Shared::PLAYER_ATTACHMENT_SLOT;
		if ("NONE" == value)
			output = PLAYER_ATTACHMENT_SLOT::NONE;
		else if ("BOSS_LEFT_HAND" == value)
			output = PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND;
		else
			return false;
		return true;
	}

	bool ParseBossPatternStageKind(
		const std::string_view value,
		LostArk::Server::BOSS_PATTERN_STAGE_KIND& output)
	{
		using LostArk::Server::BOSS_PATTERN_STAGE_KIND;
		if ("WINDUP" == value)
			output = BOSS_PATTERN_STAGE_KIND::WINDUP;
		else if ("ACTIVE" == value)
			output = BOSS_PATTERN_STAGE_KIND::ACTIVE;
		else if ("RECOVERY" == value)
			output = BOSS_PATTERN_STAGE_KIND::RECOVERY;
		else if ("GROGGY" == value)
			output = BOSS_PATTERN_STAGE_KIND::GROGGY;
		else if ("PART_BREAK" == value)
			output = BOSS_PATTERN_STAGE_KIND::PART_BREAK;
		else
			return false;
		return true;
	}

	bool ParseBossCombatObjectKind(
		const std::string_view value,
		LostArk::Server::BOSS_COMBAT_OBJECT_KIND& output)
	{
		using LostArk::Server::BOSS_COMBAT_OBJECT_KIND;
		if ("FIXED_AREA" == value)
			output = BOSS_COMBAT_OBJECT_KIND::FIXED_AREA;
		else if ("MISSILE" == value)
			output = BOSS_COMBAT_OBJECT_KIND::MISSILE;
		else
			return false;
		return true;
	}

	bool ParseBossCombatObjectOriginPolicy(
		const std::string_view value,
		LostArk::Server::BOSS_COMBAT_OBJECT_ORIGIN_POLICY& output)
	{
		using LostArk::Server::BOSS_COMBAT_OBJECT_ORIGIN_POLICY;
		if ("BOSS_POSITION" == value)
			output = BOSS_COMBAT_OBJECT_ORIGIN_POLICY::BOSS_POSITION;
		else if ("LOCKED_TARGET_UNTIL_FIRST_PULSE" == value)
			output = BOSS_COMBAT_OBJECT_ORIGIN_POLICY::
				LOCKED_TARGET_UNTIL_FIRST_PULSE;
		else if ("LOCKED_TARGET_PER_ALIVE_PLAYER" == value)
			output = BOSS_COMBAT_OBJECT_ORIGIN_POLICY::
				LOCKED_TARGET_PER_ALIVE_PLAYER;
		else
			return false;
		return true;
	}

	bool ParseBossCombatObjectDirectionPolicy(
		const std::string_view value,
		LostArk::Server::BOSS_COMBAT_OBJECT_DIRECTION_POLICY& output)
	{
		using LostArk::Server::BOSS_COMBAT_OBJECT_DIRECTION_POLICY;
		if ("NONE" == value)
			output = BOSS_COMBAT_OBJECT_DIRECTION_POLICY::NONE;
		else if ("PATTERN_FACING_AT_SPAWN" == value)
			output = BOSS_COMBAT_OBJECT_DIRECTION_POLICY::
				PATTERN_FACING_AT_SPAWN;
		else if ("RADIAL_INWARD" == value)
			output = BOSS_COMBAT_OBJECT_DIRECTION_POLICY::RADIAL_INWARD;
		else if ("NEXT_RADIAL_SLOT" == value)
			output = BOSS_COMBAT_OBJECT_DIRECTION_POLICY::NEXT_RADIAL_SLOT;
		else
			return false;
		return true;
	}

	bool ParseBossCombatObjectHitTrigger(
		const std::string_view value,
		LostArk::Server::BOSS_COMBAT_OBJECT_HIT_TRIGGER& output)
	{
		using LostArk::Server::BOSS_COMBAT_OBJECT_HIT_TRIGGER;
		if ("CONTACT" == value)
			output = BOSS_COMBAT_OBJECT_HIT_TRIGGER::CONTACT;
		else if ("TIMED" == value)
			output = BOSS_COMBAT_OBJECT_HIT_TRIGGER::TIMED;
		else
			return false;
		return true;
	}

	bool ParseBossPatternCategory(
		const std::string_view value,
		LostArk::Server::BOSS_PATTERN_CATEGORY& output)
	{
		using LostArk::Server::BOSS_PATTERN_CATEGORY;
		if ("NORMAL" == value)
			output = BOSS_PATTERN_CATEGORY::NORMAL;
		else if ("IMPORTANT" == value)
			output = BOSS_PATTERN_CATEGORY::IMPORTANT;
		else if ("MECHANIC" == value)
			output = BOSS_PATTERN_CATEGORY::MECHANIC;
		else
			return false;
		return true;
	}

	bool ParseBossPatternTargetPolicy(
		const std::string_view value,
		LostArk::Server::BOSS_PATTERN_TARGET_POLICY& output)
	{
		using LostArk::Server::BOSS_PATTERN_TARGET_POLICY;
		if ("NONE" == value)
			output = BOSS_PATTERN_TARGET_POLICY::NONE;
		else if ("NEAREST_EACH_TICK" == value)
			output = BOSS_PATTERN_TARGET_POLICY::NEAREST_EACH_TICK;
		else if ("LOCK_NEAREST_ON_START" == value)
			output = BOSS_PATTERN_TARGET_POLICY::LOCK_NEAREST_ON_START;
		else if ("LOCK_RANDOM_ALIVE_ON_START" == value)
			output = BOSS_PATTERN_TARGET_POLICY::LOCK_RANDOM_ALIVE_ON_START;
		else if ("LOCK_RANDOM_ALIVE_BEHIND_ON_START" == value)
			output =
				BOSS_PATTERN_TARGET_POLICY::LOCK_RANDOM_ALIVE_BEHIND_ON_START;
		else
			return false;
		return true;
	}

	bool ParseBossPatternPartDamagePolicy(
		const std::string_view value,
		LostArk::Server::BOSS_PATTERN_PART_DAMAGE_POLICY& output)
	{
		using LostArk::Server::BOSS_PATTERN_PART_DAMAGE_POLICY;
		if ("NORMAL" == value)
			output = BOSS_PATTERN_PART_DAMAGE_POLICY::NORMAL;
		else if ("DESTROY_FIRST_ELIGIBLE" == value)
			output =
				BOSS_PATTERN_PART_DAMAGE_POLICY::DESTROY_FIRST_ELIGIBLE;
		else
			return false;
		return true;
	}

	bool ParseBossPatternAimPolicy(
		const std::string_view value,
		LostArk::Server::BOSS_PATTERN_AIM_POLICY& output)
	{
		using LostArk::Server::BOSS_PATTERN_AIM_POLICY;
		if ("NONE" == value)
			output = BOSS_PATTERN_AIM_POLICY::NONE;
		else if ("TRACK_TARGET_EACH_TICK" == value)
			output = BOSS_PATTERN_AIM_POLICY::TRACK_TARGET_EACH_TICK;
		else if ("LOCK_FACING_ON_START" == value)
			output = BOSS_PATTERN_AIM_POLICY::LOCK_FACING_ON_START;
		else if ("FACE_MOTION_ANCHOR" == value)
			output = BOSS_PATTERN_AIM_POLICY::FACE_MOTION_ANCHOR;
		else
			return false;
		return true;
	}

	bool ParseBossPatternLogicKind(
		const std::string_view value,
		LostArk::Server::BOSS_PATTERN_LOGIC_KIND& output)
	{
		using LostArk::Server::BOSS_PATTERN_LOGIC_KIND;
		if ("ROULETTE_CARD_MATCH" == value)
			output = BOSS_PATTERN_LOGIC_KIND::ROULETTE_CARD_MATCH;
		else if ("GAZE_REAL_BOSS" == value)
			output = BOSS_PATTERN_LOGIC_KIND::GAZE_REAL_BOSS;
		else if ("POSE_INPUT" == value)
			output = BOSS_PATTERN_LOGIC_KIND::POSE_INPUT;
		else if ("CARD_DICE_BIND" == value)
			output = BOSS_PATTERN_LOGIC_KIND::CARD_DICE_BIND;
		else if ("BOSS_DAMAGE_REDUCTION" == value)
			output = BOSS_PATTERN_LOGIC_KIND::BOSS_DAMAGE_REDUCTION;
		else if ("STAGGER_WINDOW" == value)
			output = BOSS_PATTERN_LOGIC_KIND::STAGGER_WINDOW;
		else if ("INVULNERABILITY_ZONE" == value)
			output = BOSS_PATTERN_LOGIC_KIND::INVULNERABILITY_ZONE;
		else if ("BINGO_COMPLETED_LINES" == value)
			output = BOSS_PATTERN_LOGIC_KIND::BINGO_COMPLETED_LINES;
		else if ("AREA_OVERLAP" == value)
			output = BOSS_PATTERN_LOGIC_KIND::AREA_OVERLAP;
		else if ("ENTER_AREA" == value)
			output = BOSS_PATTERN_LOGIC_KIND::ENTER_AREA;
		else if ("OBJECT_OVERLAP" == value)
			output = BOSS_PATTERN_LOGIC_KIND::OBJECT_OVERLAP;
		else if ("OBJECT_CONTACT" == value)
			output = BOSS_PATTERN_LOGIC_KIND::OBJECT_CONTACT;
		else if ("ATTACHMENT_HOLD" == value)
            output = BOSS_PATTERN_LOGIC_KIND::ATTACHMENT_HOLD;
		else if ("COUNTER_WINDOW" == value)
			output = BOSS_PATTERN_LOGIC_KIND::COUNTER_WINDOW;
		else if ("PATTERN_COMPLETION_COUNT" == value)
			output = BOSS_PATTERN_LOGIC_KIND::PATTERN_COMPLETION_COUNT;
		else if ("EXTERNAL_SIGNAL" == value)
			output = BOSS_PATTERN_LOGIC_KIND::EXTERNAL_SIGNAL;
		else
			return false;
		return true;
	}

	bool ParseBossPatternLogicResultKind(
		const std::string_view value,
		LostArk::Server::BOSS_PATTERN_LOGIC_RESULT_KIND& output)
	{
		using LostArk::Server::BOSS_PATTERN_LOGIC_RESULT_KIND;
		if ("NONE" == value)
			output = BOSS_PATTERN_LOGIC_RESULT_KIND::NONE;
		else if ("INSTANT_DEATH" == value)
			output = BOSS_PATTERN_LOGIC_RESULT_KIND::INSTANT_DEATH;
		else if ("FIXED_DAMAGE" == value)
			output = BOSS_PATTERN_LOGIC_RESULT_KIND::FIXED_DAMAGE;
		else if ("PLAYER_INVULNERABILITY" == value)
			output = BOSS_PATTERN_LOGIC_RESULT_KIND::PLAYER_INVULNERABILITY;
		else if ("MAX_HP_PERCENT_DAMAGE" == value)
			output = BOSS_PATTERN_LOGIC_RESULT_KIND::MAX_HP_PERCENT_DAMAGE;
		else if ("MADNESS_GAUGE_ADD_PERCENT" == value)
			output = BOSS_PATTERN_LOGIC_RESULT_KIND::MADNESS_GAUGE_ADD_PERCENT;
		else if ("CAPTURE_PLAYER" == value)
            output = BOSS_PATTERN_LOGIC_RESULT_KIND::CAPTURE_PLAYER;
		else if ("FEAR" == value)
			output = BOSS_PATTERN_LOGIC_RESULT_KIND::FEAR;
		else if ("CLOWN_TRANSFORM" == value)
			output = BOSS_PATTERN_LOGIC_RESULT_KIND::CLOWN_TRANSFORM;
		else if ("MARIO_ENTER" == value)
			output = BOSS_PATTERN_LOGIC_RESULT_KIND::MARIO_ENTER;
		else if ("FOLLOWUP_PATTERN" == value)
			output = BOSS_PATTERN_LOGIC_RESULT_KIND::FOLLOWUP_PATTERN;
		else if ("PLAY_WORLD_OBJECT_MOTION" == value)
			output = BOSS_PATTERN_LOGIC_RESULT_KIND::PLAY_WORLD_OBJECT_MOTION;
		else if ("PLAY_CONTACT_WORLD_OBJECT_MOTION" == value)
			output = BOSS_PATTERN_LOGIC_RESULT_KIND::PLAY_CONTACT_WORLD_OBJECT_MOTION;
		else if ("COMPLETE_LOGIC_WINDOW" == value)
			output = BOSS_PATTERN_LOGIC_RESULT_KIND::COMPLETE_LOGIC_WINDOW;
		else if ("GRAB_TO_WORLD_OBJECT" == value)
			output = BOSS_PATTERN_LOGIC_RESULT_KIND::GRAB_TO_WORLD_OBJECT;
		else
			return false;
		return true;
	}

	bool ParseMechanicCardSymbol(
		const std::string_view value,
		LostArk::Shared::MECHANIC_CARD_SYMBOL& output)
	{
		using LostArk::Shared::MECHANIC_CARD_SYMBOL;
		if ("HEART" == value)
			output = MECHANIC_CARD_SYMBOL::HEART;
		else if ("SPADE" == value)
			output = MECHANIC_CARD_SYMBOL::SPADE;
		else if ("CLUB" == value)
			output = MECHANIC_CARD_SYMBOL::CLUB;
		else if ("DIAMOND" == value)
			output = MECHANIC_CARD_SYMBOL::DIAMOND;
		else
			return false;
		return true;
	}

	/* "HEART,SPADE,..." into typed symbols; "-" is the empty list. */
	bool ParseMechanicCardSymbols(
		const std::string_view packed,
		std::vector<LostArk::Shared::MECHANIC_CARD_SYMBOL>& output)
	{
		output.clear();
		if ("-" == packed)
			return true;
		std::size_t begin = 0u;
		while (begin <= packed.size())
		{
			const std::size_t end = packed.find(',', begin);
			const std::string_view token = packed.substr(begin,
				std::string_view::npos == end ? std::string_view::npos : end - begin);
			LostArk::Shared::MECHANIC_CARD_SYMBOL symbol{};
			if (!ParseMechanicCardSymbol(token, symbol) || output.size() >= 64u)
				return false;
			output.push_back(symbol);
			if (std::string_view::npos == end)
				break;
			begin = end + 1u;
		}
		return true;
	}

	bool ParseBossPatternStageOutcome(
		const std::string_view value,
		LostArk::Server::BOSS_PATTERN_STAGE_OUTCOME& output)
	{
		using LostArk::Server::BOSS_PATTERN_STAGE_OUTCOME;
		if ("TIMEOUT" == value)
			output = BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT;
		else if ("COUNTER_HIT" == value)
			output = BOSS_PATTERN_STAGE_OUTCOME::COUNTER_HIT;
		else if ("STAGGER_BROKEN" == value)
			output = BOSS_PATTERN_STAGE_OUTCOME::STAGGER_BROKEN;
		else if ("HEALTH_DAMAGE_THRESHOLD_REACHED" == value)
		{
			output = BOSS_PATTERN_STAGE_OUTCOME::
				HEALTH_DAMAGE_THRESHOLD_REACHED;
		}
		else if ("WALL_CONTACT" == value)
			output = BOSS_PATTERN_STAGE_OUTCOME::WALL_CONTACT;
		else if ("PART_DESTROYED" == value)
			output = BOSS_PATTERN_STAGE_OUTCOME::PART_DESTROYED;
		else if ("PROP_DESTROYED" == value)
			output = BOSS_PATTERN_STAGE_OUTCOME::PROP_DESTROYED;
		else if ("SUMMON_DEAD" == value)
			output = BOSS_PATTERN_STAGE_OUTCOME::SUMMON_DEAD;
		else if ("ALL_PLAYERS_GRABBED" == value)
			output = BOSS_PATTERN_STAGE_OUTCOME::ALL_PLAYERS_GRABBED;
		else if ("NAVIGATION_BLOCKED" == value)
			output = BOSS_PATTERN_STAGE_OUTCOME::NAVIGATION_BLOCKED;
		else if ("ANY_PLAYER_GRABBED" == value)
			output = BOSS_PATTERN_STAGE_OUTCOME::ANY_PLAYER_GRABBED;
		else
			return false;
		return true;
	}

	bool ParseBossPatternStageActionTrigger(
		const std::string_view value,
		LostArk::Server::BOSS_PATTERN_STAGE_ACTION_TRIGGER& output)
	{
		using LostArk::Server::BOSS_PATTERN_STAGE_ACTION_TRIGGER;
		if ("ENTER" == value)
			output = BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER;
		else if ("EXIT" == value)
			output = BOSS_PATTERN_STAGE_ACTION_TRIGGER::EXIT;
		else
			return false;
		return true;
	}

	bool ParseBossPatternStageActionKind(
		const std::string_view value,
		LostArk::Server::BOSS_PATTERN_STAGE_ACTION_KIND& output)
	{
		using LostArk::Server::BOSS_PATTERN_STAGE_ACTION_KIND;
		if ("SET_BOSS_FLAG" == value)
			output = BOSS_PATTERN_STAGE_ACTION_KIND::SET_BOSS_FLAG;
		else if ("SET_STAGGER_GAUGE" == value)
			output = BOSS_PATTERN_STAGE_ACTION_KIND::SET_STAGGER_GAUGE;
		else if ("SET_SHIELD" == value)
			output = BOSS_PATTERN_STAGE_ACTION_KIND::SET_SHIELD;
		else if ("SET_PLAYER_BIND" == value)
			output = BOSS_PATTERN_STAGE_ACTION_KIND::SET_PLAYER_BIND;
		else if ("SET_PLAYER_SILENCE" == value)
			output = BOSS_PATTERN_STAGE_ACTION_KIND::SET_PLAYER_SILENCE;
		else if ("SET_GAMEPLAY_PHASE" == value)
			output = BOSS_PATTERN_STAGE_ACTION_KIND::SET_GAMEPLAY_PHASE;
		else if ("SPAWN_COMBAT_OBJECT" == value)
			output = BOSS_PATTERN_STAGE_ACTION_KIND::SPAWN_COMBAT_OBJECT;
		else if ("RETARGET_RANDOM_ALIVE" == value)
			output = BOSS_PATTERN_STAGE_ACTION_KIND::RETARGET_RANDOM_ALIVE;
		else if ("RELEASE_GRABBED_PLAYERS" == value)
			output = BOSS_PATTERN_STAGE_ACTION_KIND::RELEASE_GRABBED_PLAYERS;
		else if ("DAMAGE_GRABBED_PLAYERS" == value)
			output = BOSS_PATTERN_STAGE_ACTION_KIND::DAMAGE_GRABBED_PLAYERS;
		else if ("RETURN_TO_ARENA_CENTER" == value)
			output = BOSS_PATTERN_STAGE_ACTION_KIND::RETURN_TO_ARENA_CENTER;
		else if ("EXECUTE_GRABBED_PLAYERS" == value)
			output = BOSS_PATTERN_STAGE_ACTION_KIND::EXECUTE_GRABBED_PLAYERS;
		else if ("SUPPRESS_INTER_STEP_PURSUIT" == value)
			output = BOSS_PATTERN_STAGE_ACTION_KIND::SUPPRESS_INTER_STEP_PURSUIT;
		else
			return false;
		return true;
	}

	bool ParseBossGrabbedReleaseMode(
		const std::string_view value,
		LostArk::Server::BOSS_GRABBED_RELEASE_MODE& output)
	{
		using LostArk::Server::BOSS_GRABBED_RELEASE_MODE;
		if ("NONE" == value)
			output = BOSS_GRABBED_RELEASE_MODE::NONE;
		else if ("HOLD" == value)
			output = BOSS_GRABBED_RELEASE_MODE::HOLD;
		else if ("ARENA_EJECTION" == value)
			output = BOSS_GRABBED_RELEASE_MODE::ARENA_EJECTION;
		else if ("OPPOSITE_KNOCKBACK" == value)
			output = BOSS_GRABBED_RELEASE_MODE::OPPOSITE_KNOCKBACK;
		else
			return false;
		return true;
	}

	bool IsValidBossPatternStageAction(
		const LostArk::Server::BOSS_PATTERN_STAGE_ACTION_TRIGGER trigger,
		const LostArk::Server::BOSS_PATTERN_STAGE_ACTION_KIND kind,
		const std::string_view targetId,
		const std::uint32_t value,
		const std::uint32_t durationMs,
		const LostArk::Server::BOSS_GRABBED_RELEASE_MODE releaseMode,
		const float releaseSpeedMps,
		const float releaseYawOffsetDegrees)
	{
		using LostArk::Server::BOSS_PATTERN_STAGE_ACTION_KIND;
		using LostArk::Server::BOSS_PATTERN_STAGE_ACTION_TRIGGER;
		using LostArk::Server::BOSS_GRABBED_RELEASE_MODE;
		if (!std::isfinite(releaseSpeedMps) || releaseSpeedMps < 0.f ||
			releaseSpeedMps > 50.f || !std::isfinite(releaseYawOffsetDegrees) ||
			std::abs(releaseYawOffsetDegrees) > 180.f)
			return false;
		if (BOSS_PATTERN_STAGE_ACTION_KIND::RELEASE_GRABBED_PLAYERS == kind)
		{
			if ("boss.attachment.left-hand" != targetId || 0u != value)
				return false;
			if (BOSS_GRABBED_RELEASE_MODE::HOLD == releaseMode)
				return 0.f == releaseSpeedMps && 0u == durationMs &&
					0.f == releaseYawOffsetDegrees;
			return (BOSS_GRABBED_RELEASE_MODE::OPPOSITE_KNOCKBACK == releaseMode ||
				BOSS_GRABBED_RELEASE_MODE::ARENA_EJECTION == releaseMode) &&
				releaseSpeedMps > 0.f && durationMs > 0u && durationMs <= 5000u &&
				(BOSS_GRABBED_RELEASE_MODE::ARENA_EJECTION == releaseMode ||
				 0.f == releaseYawOffsetDegrees);
		}
		if (BOSS_PATTERN_STAGE_ACTION_KIND::SET_PLAYER_BIND == kind)
		{
			return BOSS_GRABBED_RELEASE_MODE::NONE == releaseMode &&
				0.f == releaseSpeedMps && 0.f == releaseYawOffsetDegrees &&
				"player.status.bind" == targetId &&
				((BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER == trigger &&
				  5000u == value && durationMs >= 100u && durationMs <= 120000u) ||
				 (BOSS_PATTERN_STAGE_ACTION_TRIGGER::EXIT == trigger &&
				  0u == value && 0u == durationMs));
		}
		if (BOSS_PATTERN_STAGE_ACTION_KIND::SET_PLAYER_SILENCE == kind)
		{
			return BOSS_GRABBED_RELEASE_MODE::NONE == releaseMode &&
				0.f == releaseSpeedMps && 0.f == releaseYawOffsetDegrees &&
				"player.status.silence" == targetId &&
				BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER == trigger &&
				1u == value && durationMs >= 100u && durationMs <= 120000u;
		}
		if (BOSS_GRABBED_RELEASE_MODE::NONE != releaseMode ||
			0.f != releaseSpeedMps || 0.f != releaseYawOffsetDegrees ||
			0u != durationMs)
		{
			return false;
		}
		if (BOSS_PATTERN_STAGE_ACTION_KIND::RETURN_TO_ARENA_CENTER == kind)
		{
			return BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER == trigger &&
				"boss.arena.center" == targetId && 1u == value;
		}
		if (BOSS_PATTERN_STAGE_ACTION_KIND::RETARGET_RANDOM_ALIVE == kind)
		{
			return BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER == trigger &&
				("boss.target.pattern" == targetId || "boss.target.nearest" == targetId) &&
				1u == value;
		}
		if (BOSS_PATTERN_STAGE_ACTION_KIND::DAMAGE_GRABBED_PLAYERS == kind ||
			BOSS_PATTERN_STAGE_ACTION_KIND::EXECUTE_GRABBED_PLAYERS == kind)
		{
			return BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER == trigger &&
				0u == value &&
				(BOSS_PATTERN_STAGE_ACTION_KIND::EXECUTE_GRABBED_PLAYERS == kind ?
					"boss.attachment.left-hand" == targetId :
					0u == targetId.find("damage."));
		}
		if (BOSS_PATTERN_STAGE_ACTION_KIND::SUPPRESS_INTER_STEP_PURSUIT == kind)
		{
			return BOSS_PATTERN_STAGE_ACTION_TRIGGER::EXIT == trigger &&
				"boss.sequence.inter-step-pursuit" == targetId &&
				0u == value && 0u == durationMs &&
				BOSS_GRABBED_RELEASE_MODE::NONE == releaseMode &&
				0.f == releaseSpeedMps && 0.f == releaseYawOffsetDegrees;
		}
		const bool isSet = BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER == trigger;
		if ((isSet && 0u == value) || (!isSet && 0u != value))
			return false;
		switch (kind)
		{
		case BOSS_PATTERN_STAGE_ACTION_KIND::SET_BOSS_FLAG:
			return value <= 1u &&
				("boss.flag.groggy" == targetId ||
				 "boss.flag.invulnerable" == targetId ||
				 "boss.flag.counterable" == targetId);
		case BOSS_PATTERN_STAGE_ACTION_KIND::SET_STAGGER_GAUGE:
			return "boss.gauge.stagger" == targetId;
		case BOSS_PATTERN_STAGE_ACTION_KIND::SET_SHIELD:
			return "boss.gauge.shield" == targetId;
		case BOSS_PATTERN_STAGE_ACTION_KIND::SET_PLAYER_BIND:
		case BOSS_PATTERN_STAGE_ACTION_KIND::SET_PLAYER_SILENCE:
			return false;
		case BOSS_PATTERN_STAGE_ACTION_KIND::SET_GAMEPLAY_PHASE:
			return isSet && "boss.phase.gameplay" == targetId &&
				(2u == value || 3u == value);
		case BOSS_PATTERN_STAGE_ACTION_KIND::SPAWN_COMBAT_OBJECT:
			return isSet && 1u == value && IsStableId(targetId);
		case BOSS_PATTERN_STAGE_ACTION_KIND::SPAWN_COMBAT_OBJECT_VOLLEY:
			return false;
		case BOSS_PATTERN_STAGE_ACTION_KIND::RETURN_TO_ARENA_CENTER:
		case BOSS_PATTERN_STAGE_ACTION_KIND::RETARGET_RANDOM_ALIVE:
		case BOSS_PATTERN_STAGE_ACTION_KIND::RELEASE_GRABBED_PLAYERS:
		case BOSS_PATTERN_STAGE_ACTION_KIND::DAMAGE_GRABBED_PLAYERS:
		case BOSS_PATTERN_STAGE_ACTION_KIND::EXECUTE_GRABBED_PLAYERS:
		case BOSS_PATTERN_STAGE_ACTION_KIND::SUPPRESS_INTER_STEP_PURSUIT:
			return false;
		}
		return false;
	}

	bool IsStatefulBossPatternStageAction(
		const LostArk::Server::BOSS_PATTERN_STAGE_ACTION_KIND kind)
	{
		return LostArk::Server::BOSS_PATTERN_STAGE_ACTION_KIND::
			SPAWN_COMBAT_OBJECT != kind &&
			LostArk::Server::BOSS_PATTERN_STAGE_ACTION_KIND::
				SPAWN_COMBAT_OBJECT_VOLLEY != kind &&
			LostArk::Server::BOSS_PATTERN_STAGE_ACTION_KIND::
				SET_GAMEPLAY_PHASE != kind &&
			LostArk::Server::BOSS_PATTERN_STAGE_ACTION_KIND::
				RETARGET_RANDOM_ALIVE != kind &&
			LostArk::Server::BOSS_PATTERN_STAGE_ACTION_KIND::
				RELEASE_GRABBED_PLAYERS != kind &&
			LostArk::Server::BOSS_PATTERN_STAGE_ACTION_KIND::
				DAMAGE_GRABBED_PLAYERS != kind &&
			LostArk::Server::BOSS_PATTERN_STAGE_ACTION_KIND::
				EXECUTE_GRABBED_PLAYERS != kind &&
			LostArk::Server::BOSS_PATTERN_STAGE_ACTION_KIND::
				SUPPRESS_INTER_STEP_PURSUIT != kind &&
			/* 침묵은 ENTER가 Server 만료 시각을 정하는 player 상태다. 같은 pattern occurrence의 EXIT로 해제하지 않는다. */
			LostArk::Server::BOSS_PATTERN_STAGE_ACTION_KIND::
				SET_PLAYER_SILENCE != kind &&
			LostArk::Server::BOSS_PATTERN_STAGE_ACTION_KIND::
				RETURN_TO_ARENA_CENTER != kind;
	}

	bool HasFiniteBossStageGraph(const LostArk::Server::BOSS_PATTERN_DEFINITION& pattern)
	{
		std::vector<std::uint8_t> visited(pattern.Stages.size(), 0u);
		const auto visit = [&](auto&& self, const std::size_t index) -> bool
		{
			if (1u == visited[index]) return false;
			if (2u == visited[index]) return true;
			visited[index] = 1u;
			for (const auto& branch : pattern.Stages[index].Branches)
			{
				if (branch.strNextActionId.empty()) continue;
				const auto next = std::find_if(pattern.Stages.begin(), pattern.Stages.end(),
					[&branch](const auto& stage)
					{ return stage.strActionId == branch.strNextActionId; });
				if (next == pattern.Stages.end()) return false;
				const auto& source = pattern.Stages[index];
				// 아래 Trash의 명시적 재시도 edge만 순환을 허용한다.
				if ((pattern.strPatternId == "VALTAN_TRASH" || pattern.strPatternId == "VALTAN_TRASH_CATCH_IF") &&
					(source.strStageId == "RETRY_EXHAUSTED" || source.strStageId == "CATCH_SLAM" ||
					 source.strStageId == "EXECUTE_TAIL") && next->strStageId == "RECHARGE_WAIT_02" &&
					source.Branches.size() == 1u &&
					branch.eOutcome == LostArk::Server::BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT)
					continue;
				if (!self(self, static_cast<std::size_t>(next - pattern.Stages.begin())))
					return false;
			}
			visited[index] = 2u;
			return true;
		};
		for (std::size_t index = 0u; index < pattern.Stages.size(); ++index)
			if (!visit(visit, index)) return false;
		return !pattern.Stages.empty();
	}

	bool HasFiniteBossPatternFollowupGraph(
		const std::vector<LostArk::Server::BOSS_PATTERN_DEFINITION>& patterns)
	{
		constexpr std::size_t MAX_FOLLOWUP_DEPTH = 32u;
		std::unordered_map<std::string, std::size_t> patternIndexById;
		for (std::size_t index = 0u; index < patterns.size(); ++index)
		{
			if (!patternIndexById.emplace(patterns[index].strPatternId, index).second)
				return false;
		}
		std::vector<std::uint8_t> visited(patterns.size(), 0u);
		std::vector<std::size_t> longestDepth(patterns.size(), 0u);
		const auto visit = [&](auto&& self, const std::size_t index) -> bool
		{
			if (1u == visited[index]) return false;
			if (2u == visited[index]) return true;
			visited[index] = 1u;
			std::size_t maximumChildDepth = 0u;
			for (const auto& stage : patterns[index].Stages)
			{
				for (const auto& branch : stage.Branches)
				{
					if (branch.strNextPatternId.empty()) continue;
					const auto target = patternIndexById.find(
						branch.strNextPatternId);
					if (patternIndexById.end() == target ||
						!self(self, target->second))
					{
						return false;
					}
					maximumChildDepth = (std::max)(
						maximumChildDepth,
						1u + longestDepth[target->second]);
					if (maximumChildDepth > MAX_FOLLOWUP_DEPTH)
						return false;
				}
			}
			longestDepth[index] = maximumChildDepth;
			visited[index] = 2u;
			return true;
		};
		for (std::size_t index = 0u; index < patterns.size(); ++index)
			if (!visit(visit, index)) return false;
		return true;
	}

	bool ParseBossPhasePolicyKind(
		const std::string_view value,
		LostArk::Server::BOSS_PHASE_POLICY_KIND& output)
	{
		using LostArk::Server::BOSS_PHASE_POLICY_KIND;
		if ("HEALTH_PERCENT_THRESHOLD" == value)
			output = BOSS_PHASE_POLICY_KIND::HEALTH_PERCENT_THRESHOLD;
		else if ("AUTHORED_PATTERN_EVENT" == value)
			output = BOSS_PHASE_POLICY_KIND::AUTHORED_PATTERN_EVENT;
		else
			return false;
		return true;
	}

	std::string BuildBossPatternStageActionStateKey(
		const LostArk::Server::BOSS_PATTERN_STAGE_ACTION_KIND kind,
		const std::string_view targetId)
	{
		return std::to_string(static_cast<std::uint32_t>(kind)) + "\n" +
			std::string(targetId);
	}

	bool ParseBossPartDamageCondition(
		const std::string_view value,
		LostArk::Server::BOSS_PART_DAMAGE_CONDITION& output)
	{
		using LostArk::Server::BOSS_PART_DAMAGE_CONDITION;
		if ("ALWAYS" == value)
			output = BOSS_PART_DAMAGE_CONDITION::ALWAYS;
		else if ("GROGGY_ONLY" == value)
			output = BOSS_PART_DAMAGE_CONDITION::GROGGY_ONLY;
		else
			return false;
		return true;
	}

	bool ParseValtanTimelineArenaState(
		const std::string_view value,
		LostArk::Server::VALTAN_TIMELINE_ARENA_STATE& output)
	{
		using LostArk::Server::VALTAN_TIMELINE_ARENA_STATE;
		if ("FRESH" == value)
			output = VALTAN_TIMELINE_ARENA_STATE::FRESH;
		else if ("ORDINARY_WALLS_GONE" == value)
			output = VALTAN_TIMELINE_ARENA_STATE::ORDINARY_WALLS_GONE;
		else if ("ALL_WALLS_GONE" == value)
			output = VALTAN_TIMELINE_ARENA_STATE::ALL_WALLS_GONE;
		else if ("FLOOR84_GONE" == value)
			output = VALTAN_TIMELINE_ARENA_STATE::FLOOR84_GONE;
		else if ("FLOOR84_AND_30_GONE" == value)
			output = VALTAN_TIMELINE_ARENA_STATE::FLOOR84_AND_30_GONE;
		else
			return false;
		return true;
	}

	std::uint32_t Calculate_ValtanTimelineCommandId(
		const std::string_view rowId)
	{
		std::uint32_t hash = 2166136261u;
		for (const unsigned char character : rowId)
		{
			hash ^= character;
			hash *= 16777619u;
		}
		return hash;
	}

	bool ParseValtanTimelinePropState(
		const std::string_view value,
		LostArk::Server::VALTAN_TIMELINE_PROP_STATE& output)
	{
		using LostArk::Server::VALTAN_TIMELINE_PROP_STATE;
		if ("HIDDEN" == value)
			output = VALTAN_TIMELINE_PROP_STATE::HIDDEN;
		else if ("FOUR_PILLARS_INTACT" == value)
			output = VALTAN_TIMELINE_PROP_STATE::FOUR_PILLARS_INTACT;
		else
			return false;
		return true;
	}

	bool ParseValtanTimelineEntryType(
		const std::string_view value,
		LostArk::Server::VALTAN_TIMELINE_ENTRY_TYPE& output)
	{
		using LostArk::Server::VALTAN_TIMELINE_ENTRY_TYPE;
		if ("MECHANIC" == value)
			output = VALTAN_TIMELINE_ENTRY_TYPE::MECHANIC;
		else if ("NORMAL" == value)
			output = VALTAN_TIMELINE_ENTRY_TYPE::NORMAL;
		else
			return false;
		return true;
	}
}

bool LostArk::Server::CGameplayCatalog::Parse_RootMotionSamples(
	const std::string_view packed,
	const std::uint32_t sampleCount,
	const std::uint32_t limitMs,
	std::vector<ROOT_MOTION_SAMPLE>& outSamples)
{
	outSamples.clear();
	outSamples.reserve(sampleCount);
	std::size_t cursor = 0;
	while (cursor <= packed.size())
	{
		const std::size_t comma = packed.find(',', cursor);
		const std::string_view token{
			packed.data() + cursor,
			(std::string::npos == comma ? packed.size() : comma) - cursor };
		const std::size_t first = token.find(':');
		const std::size_t second = std::string_view::npos == first ?
			std::string_view::npos : token.find(':', first + 1);
		const std::size_t third = std::string_view::npos == second ?
			std::string_view::npos : token.find(':', second + 1);
		ROOT_MOTION_SAMPLE sample{};
		if (std::string_view::npos == second ||
			!ParseNumber(token.substr(0, first), sample.iTimeMs) ||
			!ParseNumber(token.substr(first + 1, second - first - 1),
				sample.fForward) ||
			!ParseNumber(token.substr(second + 1, third == std::string_view::npos ?
				std::string_view::npos : third - second - 1), sample.fLateral) ||
			(third != std::string_view::npos &&
			 (token.find(':', third + 1) != std::string_view::npos ||
			  !ParseNumber(token.substr(third + 1), sample.fUp))) ||
			!std::isfinite(sample.fForward) ||
			!std::isfinite(sample.fLateral) || !std::isfinite(sample.fUp) ||
			sample.iTimeMs > limitMs ||
			(!outSamples.empty() && sample.iTimeMs <= outSamples.back().iTimeMs))
		{
			m_strStatus = "Root motion sample is invalid";
			return false;
		}
		outSamples.push_back(sample);
		if (std::string::npos == comma)
			break;
		cursor = comma + 1;
	}
	if (outSamples.size() != sampleCount)
	{
		m_strStatus = "Root motion sample count does not match";
		return false;
	}
	return true;
}

bool LostArk::Server::CGameplayCatalog::Parse_SkillHits(
	const std::string_view packed,
	const std::uint32_t hitCount,
	const std::uint32_t limitMs,
	std::vector<PLAYER_SKILL_HIT>& outHits)
{
	outHits.clear();
	outHits.reserve(hitCount);
	std::uint32_t subHits = 0;
	std::size_t cursor = 0;
	while (cursor <= packed.size())
	{
		const std::size_t comma = packed.find(',', cursor);
		const std::string_view token{
			packed.data() + cursor,
			(std::string::npos == comma ? packed.size() : comma) - cursor };
		std::string_view fields[16];
		std::size_t fieldCount = 0;
		std::size_t start = 0;
		while (fieldCount < 16)
		{
			const std::size_t colon = token.find(':', start);
			fields[fieldCount++] = token.substr(start,
				std::string_view::npos == colon ? std::string_view::npos : colon - start);
			if (std::string_view::npos == colon)
				break;
			start = colon + 1;
		}
		PLAYER_SKILL_HIT hit{};
		if ((13u != fieldCount && 14u != fieldCount && 15u != fieldCount) ||
			(fieldCount >= 14u && (!ParseNumber(fields[13], hit.iResultKind) || hit.iResultKind > 3u)) ||
			(15u == fieldCount && (!ParseNumber(fields[14], hit.iDurationMs) || hit.iDurationMs > 10000u)) ||
			!ParseNumber(fields[0], hit.iTimeMs) ||
			!ParseNumber(fields[1], hit.iRepeatCount) ||
			!ParseNumber(fields[2], hit.iRepeatMs) ||
			!Parse_HitShapeExtent(fields + 3, hit) ||
			hit.iTimeMs > limitMs ||
			0u == hit.iRepeatCount || hit.iRepeatCount > 64u ||
			(hit.iRepeatCount > 1u && 0u == hit.iRepeatMs) ||
			static_cast<std::uint64_t>(hit.iTimeMs) +
				static_cast<std::uint64_t>(hit.iRepeatCount - 1u) *
					hit.iRepeatMs + hit.iDurationMs > limitMs ||
			(!outHits.empty() && hit.iTimeMs < outHits.back().iTimeMs))
		{
			m_strStatus = "Skill hit shape is invalid";
			return false;
		}
		subHits += hit.iRepeatCount;
		outHits.push_back(hit);
		if (std::string::npos == comma)
			break;
		cursor = comma + 1;
	}
	if (outHits.size() != hitCount || subHits > 192u)
	{
		m_strStatus = "Skill hit shape count does not match";
		return false;
	}
	return true;
}

bool LostArk::Server::CGameplayCatalog::Parse_CancelWindows(
	const std::string_view packed,
	const std::uint32_t windowCount,
	const std::uint32_t limitMs,
	std::vector<PLAYER_CANCEL_WINDOW>& outWindows)
{
	outWindows.clear();
	outWindows.reserve(windowCount);
	std::size_t cursor = 0;
	while (cursor <= packed.size())
	{
		const std::size_t comma = packed.find(',', cursor);
		const std::string_view token{
			packed.data() + cursor,
			(std::string::npos == comma ? packed.size() : comma) - cursor };
		const std::size_t colon = token.find(':');
		PLAYER_CANCEL_WINDOW window{};
		if (std::string_view::npos == colon ||
			!ParseNumber(token.substr(0, colon), window.iStartMs) ||
			!ParseNumber(token.substr(colon + 1), window.iEndMs) ||
			window.iEndMs <= window.iStartMs ||
			window.iEndMs > limitMs ||
			/* publisher가 겹친 구간을 합치므로 이전 종료 뒤에 시작하지 않는 구간은 거부한다. */
			(!outWindows.empty() && window.iStartMs <= outWindows.back().iEndMs))
		{
			m_strStatus = "Skill cancel window is invalid";
			return false;
		}
		outWindows.push_back(window);
		if (std::string::npos == comma)
			break;
		cursor = comma + 1;
	}
	if (outWindows.size() != windowCount)
	{
		m_strStatus = "Skill cancel window count does not match";
		return false;
	}
	return true;
}

bool LostArk::Server::CGameplayCatalog::Parse_HitShapeExtent(
	const std::string_view* fields,
	PLAYER_SKILL_HIT& hit)
{
	return ParseNumber(fields[0], hit.iAreaType) &&
		ParseNumber(fields[1], hit.fRange) &&
		ParseNumber(fields[2], hit.fAngleDegrees) &&
		ParseNumber(fields[3], hit.fWidth) &&
		ParseNumber(fields[4], hit.fHeight) &&
		ParseNumber(fields[5], hit.fOffset) &&
		ParseNumber(fields[6], hit.fInner) &&
		ParseNumber(fields[7], hit.iMaxTargets) &&
		ParseNumber(fields[8], hit.iPushMs) &&
		ParseNumber(fields[9], hit.fPushRange) &&
		hit.iPushMs <= 10000u &&
		std::isfinite(hit.fPushRange) &&
		hit.fPushRange >= -50.f && hit.fPushRange <= 50.f &&
		(0u != hit.iPushMs || 0.f == hit.fPushRange) &&
		hit.iAreaType >= 1u && hit.iAreaType <= 3u &&
		std::isfinite(hit.fRange) && hit.fRange > 0.f &&
		std::isfinite(hit.fAngleDegrees) && hit.fAngleDegrees >= 0.f &&
		hit.fAngleDegrees <= 360.f &&
		(3u == hit.iAreaType || 0.f == hit.fAngleDegrees) &&
		std::isfinite(hit.fWidth) && hit.fWidth >= 0.f &&
		(2u == hit.iAreaType) == (hit.fWidth > 0.f) &&
		std::isfinite(hit.fHeight) && hit.fHeight >= 0.f &&
		std::isfinite(hit.fOffset) &&
		std::isfinite(hit.fInner) && hit.fInner >= 0.f &&
		hit.fInner < hit.fRange &&
		(2u != hit.iAreaType || 0.f == hit.fInner) &&
		hit.iMaxTargets <= 64u;
}

bool LostArk::Server::CGameplayCatalog::Parse_SkillProjectile(
	const std::vector<std::string_view>& fields,
	const std::size_t firstField,
	const std::uint32_t limitMs,
	std::vector<PLAYER_SKILL_PROJECTILE>& outProjectiles)
{
	/* index timeMs kind origin offsetForward offsetRight speed minDistance
	maxDistance lifeMs radius hitCount hits */
	std::uint32_t index = 0;
	std::uint32_t origin = 0;
	std::uint32_t hitCount = 0;
	PLAYER_SKILL_PROJECTILE projectile{};
	if (fields.size() != firstField + 13u ||
		!ParseNumber(fields[firstField], index) ||
		!ParseNumber(fields[firstField + 1u], projectile.iTimeMs) ||
		!ParseNumber(fields[firstField + 3u], origin) || origin > 1u ||
		!ParseNumber(fields[firstField + 4u], projectile.fOffsetForward) ||
		!ParseNumber(fields[firstField + 5u], projectile.fOffsetRight) ||
		!ParseNumber(fields[firstField + 6u], projectile.fSpeed) ||
		!ParseNumber(fields[firstField + 7u], projectile.fMinDistance) ||
		!ParseNumber(fields[firstField + 8u], projectile.fMaxDistance) ||
		!ParseNumber(fields[firstField + 9u], projectile.iLifeMs) ||
		!ParseNumber(fields[firstField + 10u], projectile.fRadius) ||
		!ParseNumber(fields[firstField + 11u], hitCount) ||
		!std::isfinite(projectile.fOffsetForward) ||
		projectile.fOffsetForward < -50.f || projectile.fOffsetForward > 50.f ||
		!std::isfinite(projectile.fOffsetRight) ||
		projectile.fOffsetRight < -50.f || projectile.fOffsetRight > 50.f ||
		index != outProjectiles.size() || index >= 8u ||
		projectile.iTimeMs > limitMs ||
		(!outProjectiles.empty() &&
			projectile.iTimeMs < outProjectiles.back().iTimeMs) ||
		0u == projectile.iLifeMs || projectile.iLifeMs > 600000u ||
		!std::isfinite(projectile.fSpeed) || projectile.fSpeed < 0.f ||
		projectile.fSpeed > 1000.f ||
		!std::isfinite(projectile.fMinDistance) || projectile.fMinDistance < 0.f ||
		!std::isfinite(projectile.fMaxDistance) || projectile.fMaxDistance < 0.f ||
		!std::isfinite(projectile.fRadius) || projectile.fRadius < 0.f ||
		hitCount < 1u || hitCount > 48u)
	{
		m_strStatus = "Skill projectile row is invalid";
		return false;
	}
	projectile.eOrigin = 0u == origin ?
		PLAYER_PROJECTILE_ORIGIN::CASTER : PLAYER_PROJECTILE_ORIGIN::AIM;
	const std::string_view kind = fields[firstField + 2u];
	if ("MISSILE" == kind)
		projectile.eKind = PLAYER_PROJECTILE_KIND::MISSILE;
	else if ("FIXAREA" == kind)
		projectile.eKind = PLAYER_PROJECTILE_KIND::FIXAREA;
	else if ("GRENADE" == kind)
		projectile.eKind = PLAYER_PROJECTILE_KIND::GRENADE;
	else if ("TRACE" == kind)
		projectile.eKind = PLAYER_PROJECTILE_KIND::TRACE;
	else
	{
		m_strStatus = "Skill projectile kind is invalid";
		return false;
	}
	/* A fixed area is the one kind that does not move. */
	if ((PLAYER_PROJECTILE_KIND::FIXAREA == projectile.eKind) !=
		(0.f == projectile.fSpeed))
	{
		m_strStatus = "Skill projectile motion is invalid";
		return false;
	}
	const std::string_view packed = fields[firstField + 12u];
	std::size_t cursor = 0;
	bool sawTimed = false;
	std::uint32_t timedCount = 0u;
	while (cursor <= packed.size())
	{
		const std::size_t comma = packed.find(',', cursor);
		const std::string_view token{
			packed.data() + cursor,
			(std::string::npos == comma ? packed.size() : comma) - cursor };
		std::string_view hitFields[16];
		std::size_t fieldCount = 0;
		std::size_t start = 0;
		while (fieldCount < 16)
		{
			const std::size_t colon = token.find(':', start);
			hitFields[fieldCount++] = token.substr(start,
				std::string_view::npos == colon ? std::string_view::npos : colon - start);
			if (std::string_view::npos == colon)
				break;
			start = colon + 1;
		}
		PLAYER_PROJECTILE_HIT hit{};
		std::uint32_t trigger = 0;
		if ((14u != fieldCount && 15u != fieldCount) ||
			(15u == fieldCount && (!ParseNumber(hitFields[14], hit.Hit.iResultKind) || hit.Hit.iResultKind > 3u)) ||
			!ParseNumber(hitFields[0], trigger) || trigger > 1u ||
			!ParseNumber(hitFields[1], hit.Hit.iTimeMs) ||
			!ParseNumber(hitFields[2], hit.Hit.iRepeatCount) ||
			!ParseNumber(hitFields[3], hit.Hit.iRepeatMs) ||
			!Parse_HitShapeExtent(hitFields + 4, hit.Hit) ||
			hit.Hit.iTimeMs > 600000u ||
			0u == hit.Hit.iRepeatCount || hit.Hit.iRepeatCount > 64u ||
			(hit.Hit.iRepeatCount > 1u && 0u == hit.Hit.iRepeatMs))
		{
			m_strStatus = "Skill projectile hit is invalid";
			return false;
		}
		hit.isContact = 0u == trigger;
		/* contact hits lead at time 0, then timed hits in schedule order */
		if ((hit.isContact && (0u != hit.Hit.iTimeMs || sawTimed)) ||
			(!hit.isContact && !projectile.Hits.empty() &&
				!projectile.Hits.back().isContact &&
				hit.Hit.iTimeMs < projectile.Hits.back().Hit.iTimeMs))
		{
			m_strStatus = "Skill projectile hit order is invalid";
			return false;
		}
		sawTimed = sawTimed || !hit.isContact;
		if (!hit.isContact) timedCount += hit.Hit.iRepeatCount;
		if (timedCount > 192u)
		{
			m_strStatus = "Skill projectile timed hits exceed the bounded mask";
			return false;
		}
		projectile.Hits.push_back(hit);
		if (std::string::npos == comma)
			break;
		cursor = comma + 1;
	}
	if (projectile.Hits.size() != hitCount)
	{
		m_strStatus = "Skill projectile hit count is invalid";
		return false;
	}
	outProjectiles.push_back(std::move(projectile));
	return true;
}

// GameRoom 초기화와 게시 데이터 재로드에서 사용하는 Catalog 로드 진입점이다.
// 모든 검증은 임시 Catalog에서 수행하고, 성공한 경우에만 현재 데이터를 교체한다.
// 실패하면 기존 게임 데이터는 유지하며 m_strStatus에 실패 사유를 남긴다.
bool LostArk::Server::CGameplayCatalog::Load()
{
	// 데이터 경로를 먼저 확정한다.
	// 경로를 찾지 못하면 현재 Catalog를 건드리지 않고 로드를 중단한다.
	const std::filesystem::path dataRoot = Resolve_DataRoot();

	if (dataRoot.empty())
	{
		m_strStatus = "Gameplay data root could not be resolved";
		return false;
	}

	// 게시된 게임 데이터는 임시 Catalog에 읽는다.
	// 이후 게시 확인서 검증이 실패해도 현재 사용 중인 정의가 바뀌지 않도록 한다.
	CGameplayCatalog staged;

	if (!staged.Load_BootstrapPath(
		dataRoot / L"Gameplay" / L"Gameplay.bootstrap",
		nullptr,
		nullptr))
	{
		m_strStatus = staged.Get_Status();
		return false;
	}

	// 수치 밸런스 게시 확인서는 선택 입력이다.
	// 파일이 없으면 bootstrap 로드 결과를 사용하지만,
	// 존재 여부를 확인하는 과정 자체가 실패했다면 로드를 중단한다.
	const auto receiptPath =
		dataRoot / L"Gameplay" / L"NumericBalance.active.json";

	std::error_code error;
	const bool hasReceipt = std::filesystem::exists(receiptPath, error);

	if (error)
	{
		m_strStatus = "Numeric balance receipt cannot be inspected";
		return false;
	}

	if (hasReceipt)
	{
		// 확인서는 게시된 데이터의 식별 정보를 담는다.
		// Gameplay.bootstrap을 대신하는 게임 데이터로 읽지 않으며,
		// 파싱 전에 크기가 1~2048바이트인지 확인한다.
		std::ifstream stream(
			receiptPath,
			std::ios::binary | std::ios::ate);

		if (!stream || stream.tellg() <= 0 || stream.tellg() > 2048)
		{
			m_strStatus = "Numeric balance receipt size is invalid";
			return false;
		}

		stream.seekg(0);

		// 문자열 키와 문자열 값으로 이루어진 필드를 읽는다.
		// 중복 키와 다섯 개를 초과하는 필드는 읽는 단계에서 거부한다.
		std::map<std::string, std::string> fields;
		char token = 0;
		bool valid = bool(stream >> token) && token == '{';

		while (valid)
		{
			std::string key;
			std::string value;

			// 키 뒤에 ':'가 있어야 해당 필드의 값을 읽을 수 있다.
			stream >> std::ws;

			if (stream.peek() != '"' ||
				!(stream >> std::quoted(key) >> token) ||
				token != ':')
			{
				valid = false;
				break;
			}

			stream >> std::ws;

			if (stream.peek() != '"' ||
				!(stream >> std::quoted(value)) ||
				!fields.emplace(key, value).second ||
				fields.size() > 5u)
			{
				valid = false;
				break;
			}

			// '}'이면 객체 읽기를 마치고, ','이면 다음 필드를 읽는다.
			if (!(stream >> token))
			{
				valid = false;
				break;
			}

			if (token == '}')
			{
				break;
			}

			if (token != ',')
			{
				valid = false;
				break;
			}
		}

		stream >> std::ws;

		const char* receiptMismatchStatus =
			"Numeric balance receipt does not match the validated published bootstrap";

		// 확인서 전체를 읽었는지와 지원하는 schema인지 확인한다.
		// 형식 검증이 끝나기 전에는 필드 값을 revision으로 해석하지 않는다.
		if (!valid ||
			stream.peek() != std::char_traits<char>::eof() ||
			fields.size() != 5u ||
			fields["schema"] != "lostark.numeric-balance-active")
		{
			m_strStatus = receiptMismatchStatus;
			return false;
		}

		// parent는 검증 성공 후 Catalog가 사용할 기준 revision이다.
		// expected 값들은 확인서에 기록된 게시 데이터의 기대 해시다.
		LostArk::Shared::GameplayDataRevision parent;

		LostArk::Shared::GameplayDataRevision expectedBootstrap;
		LostArk::Shared::GameplayDataRevision expectedNumeric;
		LostArk::Shared::GameplayDataRevision expectedNonNumeric;

		if (!LostArk::Shared::Try_Parse_GameplayDataRevision(
				fields["parentGameplayRevision"], parent) ||
			!LostArk::Shared::Try_Parse_GameplayDataRevision(
				fields["bootstrapContentSha256"], expectedBootstrap) ||
			!LostArk::Shared::Try_Parse_GameplayDataRevision(
				fields["numericRevision"], expectedNumeric) ||
			!LostArk::Shared::Try_Parse_GameplayDataRevision(
				fields["nonNumericRowsSha256"], expectedNonNumeric))
		{
			m_strStatus = receiptMismatchStatus;
			return false;
		}

		// 방금 로드한 bootstrap이 확인서에서 지정한 파일인지 먼저 확인한다.
		if (staged.Get_ActiveRevision() != expectedBootstrap)
		{
			m_strStatus = receiptMismatchStatus;
			return false;
		}

		// 임시 Catalog에서 실제 해시를 계산한다.
		// 전체 bootstrap, 수치 항목, 비수치 구조를 각각 대조하여
		// 서로 다른 게시 결과가 섞인 상태를 거부한다.
		LostArk::Shared::GameplayDataRevision actualBootstrap;
		LostArk::Shared::GameplayDataRevision actualNumeric;
		LostArk::Shared::GameplayDataRevision actualNonNumeric;

		std::vector<LostArk::Shared::BALANCE_NUMERIC_ENTRY> entries;

		if (!staged.Build_NumericBalanceReceiptHashes(
				actualBootstrap, actualNonNumeric, m_strStatus) ||
			!staged.Build_NumericBalanceSnapshot(
				entries, actualNumeric, m_strStatus))
		{
			m_strStatus = receiptMismatchStatus;
			return false;
		}

		if (actualBootstrap != expectedBootstrap ||
			actualNumeric != expectedNumeric ||
			actualNonNumeric != expectedNonNumeric)
		{
			m_strStatus = receiptMismatchStatus;
			return false;
		}

		// 게시 데이터의 일치가 확인된 뒤 기준 revision을 채택한다.
		staged.m_ActiveRevision = parent;
	}

	// 현재 Catalog를 교체하는 지점은 여기 한 곳이다.
	// 앞선 어느 단계에서 실패하더라도 기존 게임 데이터는 유지된다.
	*this = std::move(staged);
	return true;
}

bool LostArk::Server::CGameplayCatalog::Load_PublishedKoukuProduct(
	const CGameplayCatalog& activeGameplay)
{
	const auto dataRoot = Resolve_DataRoot();
	const auto repositoryRoot = dataRoot.parent_path().parent_path().parent_path();
	const auto ownerScript = repositoryRoot / L"Tools" / L"Build" / L"Invoke-BuildDomainOwner.ps1";
	std::error_code error;
	struct PUBLISH_READ_LOCK final
	{
		HANDLE handle = INVALID_HANDLE_VALUE;
		~PUBLISH_READ_LOCK() { if (handle != INVALID_HANDLE_VALUE) CloseHandle(handle); }
	} publishLock;
	if (std::filesystem::is_regular_file(ownerScript, error))
	{
		const auto lockPath = repositoryRoot / L"out" / L"BuildPipeline" /
			L"receipts" / L"locks" / L"runtime-owner.lock";
		publishLock.handle = CreateFileW(lockPath.c_str(), GENERIC_READ,
			FILE_SHARE_READ, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (publishLock.handle == INVALID_HANDLE_VALUE)
		{
			m_strStatus = "KoukuSaydon publish is in progress or its transaction lock is unavailable; retry after Publish completes";
			return false;
		}
	}
	CGameplayCatalog published;
	if (!published.Load())
	{
		m_strStatus = published.Get_Status();
		return false;
	}
	if (activeGameplay.m_NonKoukuBootstrapRows.empty() ||
		published.m_KoukuBootstrapRows.empty())
	{
		m_strStatus = "KoukuSaydon reload requires an admitted active baseline and published encounter";
		return false;
	}
	// 현재 process의 balance 행을 보존하고 게시 encounter 행만 결합해 참조를 다시 검증한다.
	const std::string rows = activeGameplay.m_NonKoukuBootstrapRows +
		published.m_KoukuBootstrapRows;
	const std::string joined = "LOSTARK_GAMEPLAY_BOOTSTRAP\t" +
		std::to_string(GAMEPLAY_BOOTSTRAP_VERSION) + "\t" +
		std::to_string(std::count(rows.begin(), rows.end(), '\n')) + "\n" + rows;
	return Load_BootstrapBytes(joined, nullptr, nullptr);
}

bool LostArk::Server::CGameplayCatalog::Load_DraftKoukuProduct(
	const CGameplayCatalog& activeGameplay, const std::string& rows,
	const LostArk::Shared::GameplayDataRevision& expectedRowsRevision)
{
	LostArk::Shared::GameplayDataRevision actualRevision;
	if (rows.empty() || rows.size() > LostArk::Shared::MAX_KOUKUSAYDON_DRAFT_BYTES ||
		rows.back() != '\n' || rows.find('\0') != std::string::npos ||
		!expectedRowsRevision.Is_Valid() || !Calculate_GameplayDataRevision(rows, actualRevision) ||
		actualRevision != expectedRowsRevision || activeGameplay.m_NonKoukuBootstrapRows.empty())
	{ m_strStatus = "Kouku draft size, checksum, rows or active baseline is invalid"; return false; }
	// Client draft는 해당 encounter domain만 교체한다. 유효한 형식이어도 player·balance·다른 encounter 행은 거부한다.
	std::istringstream input(rows);
	std::string line;
	while (std::getline(input, line))
	{
		StripCarriageReturn(line);
		const auto fields = SplitTabs(line);
		if (fields.size() < 2u)
		{ m_strStatus = "Kouku draft contains an empty or malformed row"; return false; }
		const auto kind = fields[0];
		const bool encounterRow = fields[1] == "ENCOUNTER_KOUKUSAYDON_G1" &&
			(kind.starts_with("PATTERN") || kind == "KOUKUMADNESS" ||
			 kind == "KOUKUSAYDONPRODUCTREVISION" || kind == "ENCOUNTERINTRO");
		const bool joinedRow = kind == "PATTERNTARGET" || kind == "PATTERNBUNDLE" ||
			kind == "PATTERNBUNDLEMEMBER" || kind == "RAIDGATE" ||
			kind == "RAIDFLOWSTEP" || kind == "RAIDFLOWGROUP" || kind == "RAIDARRIVAL" || kind == "RAIDBINGOSPECIAL";
		if (!encounterRow && !joinedRow)
		{ m_strStatus = "Kouku draft contains a row outside its encounter"; return false; }
	}
	const std::string joinedRows = activeGameplay.m_NonKoukuBootstrapRows + rows;
	const std::string bootstrap = "LOSTARK_GAMEPLAY_BOOTSTRAP\t" +
		std::to_string(GAMEPLAY_BOOTSTRAP_VERSION) + "\t" +
		std::to_string(std::count(joinedRows.begin(), joinedRows.end(), '\n')) + "\n" + joinedRows;
	CGameplayCatalog staged;
	if (!staged.Load_BootstrapBytes(bootstrap, nullptr, &activeGameplay.m_ActiveRevision))
	{ m_strStatus = staged.Get_Status(); return false; }
	if (!staged.Has_SameNonKoukuGameplay(activeGameplay))
	{ m_strStatus = "Kouku draft cannot replace player/boss balance or another encounter"; return false; }
	// Sequence draft의 maximum·hold만 가져오고 현재 승인된 Balance Test 튜닝을 유지한다.
	for (auto& [id, policy] : staged.m_KoukuMadnessPolicies)
		if (const auto active = activeGameplay.m_KoukuMadnessPolicies.find(id);
			active != activeGameplay.m_KoukuMadnessPolicies.end())
		{
			const auto maximum = policy.iMaximum, hold = policy.iClownHoldMs;
			policy = active->second;
			policy.iMaximum = maximum; policy.iClownHoldMs = hold;
		}
	*this = std::move(staged);
	m_strStatus = "Kouku memory draft admitted for one audition";
	return true;
}

bool LostArk::Server::CGameplayCatalog::Load_FromBootstrap(
	const std::filesystem::path& bootstrapPath,
	const LostArk::Shared::GameplayDataRevision& expectedBootstrapRevision,
	const LostArk::Shared::GameplayDataRevision& parentRevision)
{
	if (!expectedBootstrapRevision.Is_Valid() || !parentRevision.Is_Valid() ||
		bootstrapPath.empty() || !bootstrapPath.is_absolute())
	{
		m_strStatus = "Staged gameplay bootstrap admission input is invalid";
		return false;
	}

	std::error_code error;
	const std::filesystem::path normalized = bootstrapPath.lexically_normal();
	if (normalized != bootstrapPath)
	{
		m_strStatus = "Staged gameplay bootstrap path is not canonical";
		return false;
	}
	const std::filesystem::file_status linkStatus =
		std::filesystem::symlink_status(normalized, error);
	if (error || !std::filesystem::is_regular_file(linkStatus) ||
		normalized.filename() != L"Gameplay.bootstrap")
	{
		m_strStatus = "Staged gameplay bootstrap is not a regular canonical artifact";
		return false;
	}
	const std::filesystem::path canonical =
		std::filesystem::canonical(normalized, error);
	if (error || canonical != normalized)
	{
		m_strStatus = "Staged gameplay bootstrap path is not canonical";
		return false;
	}

	return Load_BootstrapPath(
		canonical, &expectedBootstrapRevision, &parentRevision);
}

bool LostArk::Server::CGameplayCatalog::Load_BootstrapPath(
	const std::filesystem::path& path,
	const LostArk::Shared::GameplayDataRevision* expectedBootstrapRevision,
	const LostArk::Shared::GameplayDataRevision* parentRevision)
{
	std::ifstream bootstrapFile(path, std::ios::binary | std::ios::ate);
	if (!bootstrapFile)
	{
		m_strStatus = "Missing gameplay bootstrap: " + path.string();
		return false;
	}
	const auto byteCount = static_cast<std::streamoff>(bootstrapFile.tellg());
	if (byteCount <= 0 || byteCount > LostArk::Shared::GAMEPLAY_BOOTSTRAP_MAX_BYTES)
	{
		m_strStatus = "Gameplay bootstrap byte size is outside the supported bound";
		return false;
	}
	std::string bootstrapBytes(static_cast<std::size_t>(byteCount), '\0');
	bootstrapFile.seekg(0, std::ios::beg);
	bootstrapFile.read(bootstrapBytes.data(), static_cast<std::streamsize>(bootstrapBytes.size()));
	if (!bootstrapFile || bootstrapFile.gcount() != static_cast<std::streamsize>(bootstrapBytes.size()) ||
		bootstrapFile.peek() != std::char_traits<char>::eof())
	{
		m_strStatus = "Gameplay bootstrap could not be read";
		return false;
	}
	return Load_BootstrapBytes(bootstrapBytes, expectedBootstrapRevision, parentRevision);
}

bool LostArk::Server::CGameplayCatalog::Load_BootstrapBytes(
	const std::string& bootstrapBytes,
	const LostArk::Shared::GameplayDataRevision* expectedBootstrapRevision,
	const LostArk::Shared::GameplayDataRevision* parentRevision)
{
	if (bootstrapBytes.empty() || bootstrapBytes.size() > LostArk::Shared::GAMEPLAY_BOOTSTRAP_MAX_BYTES)
	{
		m_strStatus = "Gameplay bootstrap byte size is outside the supported bound";
		return false;
	}
	using SKILL_MAP = decltype(m_Skills);
	using BOSS_MAP = decltype(m_Bosses);
	using BOSS_PART_MAP = decltype(m_BossParts);
	using PATTERN_MAP = decltype(m_BossPatterns);
	using BOSS_COMBAT_OBJECT_MAP = decltype(m_BossCombatObjects);
	using INTRO_MAP = decltype(m_IntroPatternIdByEncounter);
	using ROTATION_MAP = decltype(m_BossPatternRotations);
	using SEQUENCE_MAP = decltype(m_BossPatternSequences);
	using BUNDLE_MAP = decltype(m_BossPatternBundles);
	using TIMELINE_MAP = decltype(m_ValtanTimelines);
	using PLAYER_MAP = decltype(m_Players);
	using DAMAGE_MAP = decltype(m_DamageRatePercentByProfileId);
	using KOUKU_MADNESS_MAP = decltype(m_KoukuMadnessPolicies);
	struct LOAD_ROLLBACK final
	{
		SKILL_MAP& skills;
		BOSS_MAP& bosses;
		BOSS_PART_MAP& bossParts;
		PATTERN_MAP& patterns;
		BOSS_COMBAT_OBJECT_MAP& bossCombatObjects;
		INTRO_MAP& intros;
		ROTATION_MAP& rotations;
		SEQUENCE_MAP& sequences;
		BUNDLE_MAP& bundles;
		TIMELINE_MAP& timelines;
		PLAYER_MAP& players;
		DAMAGE_MAP& damages;
		LostArk::Shared::GameplayDataRevision& presentationGenerationId;
		std::uint32_t& koukuSaydonProductSourceRevision;
		KOUKU_MADNESS_MAP& koukuMadnessPolicies;
		SKILL_MAP previousSkills;
		BOSS_MAP previousBosses;
		BOSS_PART_MAP previousBossParts;
		PATTERN_MAP previousPatterns;
		BOSS_COMBAT_OBJECT_MAP previousBossCombatObjects;
		INTRO_MAP previousIntros;
		ROTATION_MAP previousRotations;
		SEQUENCE_MAP previousSequences;
		BUNDLE_MAP previousBundles;
		TIMELINE_MAP previousTimelines;
		PLAYER_MAP previousPlayers;
		DAMAGE_MAP previousDamages;
		LostArk::Shared::GameplayDataRevision previousPresentationGenerationId{};
		std::uint32_t previousKoukuSaydonProductSourceRevision = 0u;
		KOUKU_MADNESS_MAP previousKoukuMadnessPolicies;
		bool committed = false;

		LOAD_ROLLBACK(
			SKILL_MAP& skillTarget,
			BOSS_MAP& bossTarget,
			BOSS_PART_MAP& bossPartTarget,
			PATTERN_MAP& patternTarget,
			BOSS_COMBAT_OBJECT_MAP& bossCombatObjectTarget,
			INTRO_MAP& introTarget,
			ROTATION_MAP& rotationTarget,
			SEQUENCE_MAP& sequenceTarget,
			BUNDLE_MAP& bundleTarget,
			TIMELINE_MAP& timelineTarget,
			PLAYER_MAP& playerTarget,
			DAMAGE_MAP& damageTarget,
			LostArk::Shared::GameplayDataRevision& presentationGenerationTarget,
			std::uint32_t& koukuSaydonProductSourceRevisionTarget,
			KOUKU_MADNESS_MAP& koukuMadnessPolicyTarget)
			: skills(skillTarget)
			, bosses(bossTarget)
			, bossParts(bossPartTarget)
			, patterns(patternTarget)
			, bossCombatObjects(bossCombatObjectTarget)
			, intros(introTarget)
			, rotations(rotationTarget)
			, sequences(sequenceTarget)
			, bundles(bundleTarget)
			, timelines(timelineTarget)
			, players(playerTarget)
			, damages(damageTarget)
			, presentationGenerationId(presentationGenerationTarget)
			, koukuSaydonProductSourceRevision(
				koukuSaydonProductSourceRevisionTarget)
			, koukuMadnessPolicies(koukuMadnessPolicyTarget)
			, previousSkills(std::move(skillTarget))
			, previousBosses(std::move(bossTarget))
			, previousBossParts(std::move(bossPartTarget))
			, previousPatterns(std::move(patternTarget))
			, previousBossCombatObjects(std::move(bossCombatObjectTarget))
			, previousIntros(std::move(introTarget))
			, previousRotations(std::move(rotationTarget))
			, previousSequences(std::move(sequenceTarget))
			, previousBundles(std::move(bundleTarget))
			, previousTimelines(std::move(timelineTarget))
			, previousPlayers(std::move(playerTarget))
			, previousDamages(std::move(damageTarget))
			, previousPresentationGenerationId(presentationGenerationTarget)
			, previousKoukuSaydonProductSourceRevision(
				koukuSaydonProductSourceRevisionTarget)
			, previousKoukuMadnessPolicies(std::move(koukuMadnessPolicyTarget))
		{
		}

		~LOAD_ROLLBACK()
		{
			if (committed)
				return;
			skills = std::move(previousSkills);
			bosses = std::move(previousBosses);
			bossParts = std::move(previousBossParts);
			patterns = std::move(previousPatterns);
			bossCombatObjects = std::move(previousBossCombatObjects);
			intros = std::move(previousIntros);
			rotations = std::move(previousRotations);
			sequences = std::move(previousSequences);
			bundles = std::move(previousBundles);
			timelines = std::move(previousTimelines);
			players = std::move(previousPlayers);
			damages = std::move(previousDamages);
			presentationGenerationId = previousPresentationGenerationId;
			koukuSaydonProductSourceRevision =
				previousKoukuSaydonProductSourceRevision;
			koukuMadnessPolicies = std::move(previousKoukuMadnessPolicies);
		}
	};
	LOAD_ROLLBACK rollback{
		m_Skills, m_Bosses, m_BossParts, m_BossPatterns, m_BossCombatObjects,
		m_IntroPatternIdByEncounter, m_BossPatternRotations,
		m_BossPatternSequences, m_BossPatternBundles,
		m_ValtanTimelines, m_Players,
		m_DamageRatePercentByProfileId,
		m_ValtanPresentationGenerationId,
		m_iKoukuSaydonProductSourceRevision,
		m_KoukuMadnessPolicies };
	// 선택적 raid metadata도 같은 load 성공·실패에 함께 반영한다.
	struct RAID_ROLLBACK final
	{
		decltype(m_KoukuRaidGates)& target;
		decltype(m_KoukuRaidGates) previous;
		bool& committed;
		~RAID_ROLLBACK() { if (!committed) target = std::move(previous); }
	} raidRollback{m_KoukuRaidGates, std::move(m_KoukuRaidGates), rollback.committed};
	// 보조 profile과 skill을 같은 세대로 교체한다. reload 실패 시 모든 조회 view를 복원한다.
	struct PROFILE_ROLLBACK final
	{
		decltype(m_EmberProfiles)& embers;
		decltype(m_EmberProfiles) previousEmbers;
		decltype(m_DamageProfileById)& damages;
		decltype(m_DamageProfileById) previousDamages;
		decltype(m_SkillBuffsBySkillId)& buffsBySkill;
		decltype(m_SkillBuffsBySkillId) previousBuffsBySkill;
		decltype(m_SkillBuffById)& buffsById;
		decltype(m_SkillBuffById) previousBuffsById;
		bool& committed;
		~PROFILE_ROLLBACK()
		{
			if (committed) return;
			embers = std::move(previousEmbers);
			damages = std::move(previousDamages);
			buffsBySkill = std::move(previousBuffsBySkill);
			buffsById = std::move(previousBuffsById);
		}
	} profileRollback{m_EmberProfiles, std::move(m_EmberProfiles),
		m_DamageProfileById, std::move(m_DamageProfileById),
		m_SkillBuffsBySkillId, std::move(m_SkillBuffsBySkillId),
		m_SkillBuffById, std::move(m_SkillBuffById), rollback.committed};
	m_DamageProfileById.clear();
	m_SkillBuffsBySkillId.clear();
	m_SkillBuffById.clear();
	m_KoukuRaidGates.clear();
	m_Skills.clear();
	m_Bosses.clear();
	m_BossParts.clear();
	m_BossPatterns.clear();
	m_BossCombatObjects.clear();
	m_IntroPatternIdByEncounter.clear();
	m_BossPatternRotations.clear();
	m_BossPatternSequences.clear();
	m_BossPatternBundles.clear();
	m_ValtanTimelines.clear();
	m_Players.clear();
	m_EmberProfiles.clear();
	m_KoukuMadnessPolicies.clear();
	m_DamageRatePercentByProfileId.clear();
	m_ValtanPresentationGenerationId = {};
	m_iKoukuSaydonProductSourceRevision = 0u;
	m_iRaidStaggerMaximum = 0u;

	LostArk::Shared::GameplayDataRevision admittedRevision{};
	if (!Calculate_GameplayDataRevision(bootstrapBytes, admittedRevision))
	{
		m_strStatus = "Gameplay bootstrap revision could not be calculated";
		return false;
	}
	if (nullptr != expectedBootstrapRevision &&
		admittedRevision != *expectedBootstrapRevision)
	{
		m_strStatus = "Staged gameplay bootstrap content revision mismatch";
		return false;
	}
	std::istringstream input{ bootstrapBytes };

	std::string line;
	if (!std::getline(input, line))
	{
		m_strStatus = "Gameplay bootstrap is empty";
		return false;
	}
	StripCarriageReturn(line);
	const std::vector<std::string_view> header = SplitTabs(line);
	std::uint32_t version = 0;
	std::uint32_t rowCount = 0;
	if (3u != header.size() || "LOSTARK_GAMEPLAY_BOOTSTRAP" != header[0] ||
		!ParseNumber(header[1], version) ||
		GAMEPLAY_BOOTSTRAP_VERSION != version ||
		!ParseNumber(header[2], rowCount) || 0u == rowCount || rowCount > MAXIMUM_GAMEPLAY_BOOTSTRAP_ROWS)
	{
		m_strStatus = "Gameplay bootstrap header is invalid";
		return false;
	}

	std::unordered_set<LostArk::Shared::SKILL_ID> skillCombatTraitOwners;
	/* 실제로 재조정한 skill만 갖는 선택적 행이므로 모든 skill의 포함을 요구하지 않는다. */
	std::unordered_set<LostArk::Shared::SKILL_ID> skillRootMotionScaleOwners;
	std::unordered_set<LostArk::Shared::SKILL_ID> skillTargetOwners;
	std::unordered_set<LostArk::Shared::SKILL_ID> emberSkillOwners;
	std::unordered_set<std::string> patternPolicyOwners;
	std::unordered_set<std::string> patternSourceOwners;
	std::unordered_set<std::string> patternFinaleIntervalOwners;
	std::unordered_set<std::string> patternVerticalOffsetOwners;
	std::unordered_set<std::string> patternStageVerticalOffsetOwners;
	std::unordered_set<std::string> patternStagePartDamageOwners;
	std::unordered_set<std::string> patternStageCounterProxyOwners;
	std::unordered_set<std::string> patternStageBossResponseOwners;
	for (std::uint32_t row = 0; row < rowCount; ++row)
	{
		if (!std::getline(input, line))
		{
			m_strStatus = "Gameplay bootstrap row is truncated";
			return false;
		}
		StripCarriageReturn(line);
		const std::vector<std::string_view> fields = SplitTabs(line);
		if (!fields.empty() && "KOUKUSAYDONPRODUCTREVISION" == fields[0])
		{
			std::uint32_t sourceRevision = 0u;
			if (4u != fields.size() ||
				"ENCOUNTER_KOUKUSAYDON_G1" != fields[1] ||
				"BOSS_KOUKUSAYDON_G1_KOUKU" != fields[2] ||
				0u != m_iKoukuSaydonProductSourceRevision ||
				!ParseNumber(fields[3], sourceRevision) || 0u == sourceRevision)
			{
				m_strStatus =
					"KoukuSaydon Product source revision row is invalid or duplicated";
				return false;
			}
			m_iKoukuSaydonProductSourceRevision = sourceRevision;
		}
		else if (!fields.empty() && "PATTERNPRESENTATIONGENERATION" == fields[0])
		{
			if (3u != fields.size() || "ENCOUNTER_VALTAN" != fields[1] ||
				m_ValtanPresentationGenerationId.Is_Valid() ||
				!LostArk::Shared::Try_Parse_GameplayDataRevision(
					fields[2], m_ValtanPresentationGenerationId))
			{
				m_strStatus =
					"Valtan presentation generation row is invalid or duplicated";
				return false;
			}
		}
		else if (!fields.empty() && "RAIDSTAGGER" == fields[0])
		{
			if (fields.size() != 2u || m_iRaidStaggerMaximum ||
				!ParseNumber(fields[1], m_iRaidStaggerMaximum) || !m_iRaidStaggerMaximum || m_iRaidStaggerMaximum > 1000000000u)
			{ m_strStatus = "Invalid or duplicate raid stagger policy"; return false; }
		}
		else if (!fields.empty() && "DAMAGE" == fields[0])
		{
			std::uint32_t ratePercent = 0;
			DAMAGE_PROFILE profile{};
			/* 이전 행은 고정 percent만 갖는다. coefficient·addend는 그 뒤에 추가된 선택 필드다. */
			const bool hasFormula = 5u == fields.size() || 6u == fields.size() || 7u == fields.size();
			const bool hasSpread = 6u == fields.size() || 7u == fields.size();
			const bool hasBossHealthBars = 7u == fields.size();
			if ((3u != fields.size() && !hasFormula) || !IsStableId(fields[1]) ||
				!ParseNumber(fields[2], ratePercent) || 0u == ratePercent ||
				ratePercent > MAXIMUM_DAMAGE_RATE_PERCENT ||
				(hasFormula &&
					(!ParseNumber(fields[3], profile.iAttackCoefficientBp) ||
					 !ParseNumber(fields[4], profile.iDamageAddend))) ||
				(hasSpread &&
					(!ParseNumber(fields[5], profile.iDamageSpreadPercent) ||
					 profile.iDamageSpreadPercent >= 100u)) ||
				(hasBossHealthBars &&
					(!ParseNumber(fields[6], profile.iBossHealthBarDamage) ||
					 profile.iBossHealthBarDamage > 1000u)) ||
				!m_DamageRatePercentByProfileId.emplace(
					std::string(fields[1]), ratePercent).second)
			{
				m_strStatus = "Damage profile row is invalid";
				return false;
			}
			profile.iRatePercent = ratePercent;
			m_DamageProfileById.emplace(std::string(fields[1]), profile);
		}
		else if (!fields.empty() && "SKILLBUFF" == fields[0])
		{
			SKILL_BUFF_DEFINITION buff{};
			std::string target;
			if (11u != fields.size() ||
				!ParseNumber(fields[1], buff.iSkillId) ||
				!ParseNumber(fields[2], buff.iBuffId) ||
				!ParseNumber(fields[4], buff.iDurationMs) ||
				0u == buff.iDurationMs || buff.iDurationMs > 600000u ||
				!ParseNumber(fields[5], buff.iDamageDealtPercent) ||
				!ParseNumber(fields[6], buff.iDamageTakenPercent) ||
				!ParseNumber(fields[7], buff.iAttackSpeedPercent) ||
				!ParseNumber(fields[8], buff.iShieldPercentOfMaxHp) ||
				buff.iShieldPercentOfMaxHp > 1000u ||
				!ParseNumber(fields[9], buff.iStunMs) || buff.iStunMs > 60000u ||
				!ParseNumber(fields[10], buff.iDeathDenyInvulnerableMs) ||
				buff.iDeathDenyInvulnerableMs > 60000u)
			{
				m_strStatus = "Skill buff row is invalid";
				return false;
			}
			target = fields[3];
			if ("SELF" == target) buff.eTarget = SKILL_BUFF_TARGET::SELF;
			else if ("ALLY" == target) buff.eTarget = SKILL_BUFF_TARGET::ALLY;
			else if ("ENEMY" == target) buff.eTarget = SKILL_BUFF_TARGET::ENEMY;
			else
			{
				m_strStatus = "Skill buff target is invalid";
				return false;
			}
			std::vector<SKILL_BUFF_DEFINITION>& list =
				m_SkillBuffsBySkillId[buff.iSkillId];
			for (const SKILL_BUFF_DEFINITION& existing : list)
			{
				if (existing.iBuffId == buff.iBuffId)
				{
					m_strStatus = "Skill buff row is duplicated";
					return false;
				}
			}
			list.push_back(buff);
			m_SkillBuffById.emplace(buff.iBuffId, buff);
		}
		else if (!fields.empty() && "SKILL" == fields[0])
		{
			PLAYER_SKILL_DEFINITION skill{};
			if (16u != fields.size() ||
				!ParseNumber(fields[1], skill.iSkillId) ||
				LostArk::Shared::INVALID_SKILL_ID == skill.iSkillId ||
				!ParseCharacterClass(fields[2], skill.eCharacterClass) ||
				!IsStableId(fields[3]) || !IsStableId(fields[4]) ||
				!ParseNumber(fields[5], skill.iCooldownMs) ||
				!ParseNumber(fields[6], skill.iActionDurationMs) ||
				!ParseNumber(fields[7], skill.iHitTimeMs) ||
				!ParseNumber(fields[8], skill.iResourceCost) ||
				!ParseNumber(fields[9], skill.iIdentityCost) ||
				!ParseNumber(fields[10], skill.fMovementDistance) ||
				!ParseNumber(fields[11], skill.fMaximumRange) ||
				(!fields[12].empty() && !IsStableId(fields[12])) ||
				!ParseSkillKind(fields[13], skill.eSkillKind) ||
				!ParseStance(fields[14], skill.eRequiredStance) ||
				!ParseStance(fields[15], skill.eSetsStance) ||
				(LostArk::Shared::PLAYER_SKILL_KIND::ACTIVE == skill.eSkillKind &&
					0u == skill.iCooldownMs) ||
				0u == skill.iActionDurationMs ||
				/* 시전 시작과 동시에 맞을 수 있어 iHitTimeMs의 0은 허용하고 상한만 검사한다. */
				skill.iHitTimeMs > skill.iActionDurationMs ||
				/* 각 행에서는 적용할 class pool을 확정할 수 없어 전체 로드 뒤 최대 pool과 cost를 비교한다. */
				!std::isfinite(skill.fMovementDistance) ||
				!std::isfinite(skill.fMaximumRange) ||
				skill.fMovementDistance < 0.f ||
				(fields[12].empty() ?
					(skill.fMaximumRange != 0.f || 0u != skill.iHitTimeMs) :
					skill.fMaximumRange <= 0.f))
			{
				m_strStatus = "Player skill row is invalid";
				return false;
			}
			skill.strInputSlot = fields[3];
			skill.strActionId = fields[4];
			skill.strDamageProfileId = fields[12];
			if (!m_Skills.emplace(skill.iSkillId, std::move(skill)).second)
			{
				m_strStatus = "Duplicate player skill ID";
				return false;
			}
		}
		else if (!fields.empty() && "SKILLCOMBATTRAITS" == fields[0])
		{
			LostArk::Shared::SKILL_ID ownerSkillId =
				LostArk::Shared::INVALID_SKILL_ID;
			std::uint32_t staggerDamage = 0;
			std::uint32_t partDamage = 0;
			std::uint32_t counterPower = 0;
			if (5u != fields.size() ||
				!ParseNumber(fields[1], ownerSkillId) ||
				!ParseNumber(fields[2], staggerDamage) ||
				!ParseNumber(fields[3], partDamage) ||
				!ParseNumber(fields[4], counterPower) ||
				staggerDamage > 1000000u || partDamage > 1000000u ||
				counterPower > 1000000u)
			{
				m_strStatus = "Player skill combat traits row is invalid";
				return false;
			}
			const auto owner = m_Skills.find(ownerSkillId);
			const bool isQuickCounterSlot = m_Skills.end() != owner &&
				("Q" == owner->second.strInputSlot ||
				 "W" == owner->second.strInputSlot ||
				 "E" == owner->second.strInputSlot ||
				 "R" == owner->second.strInputSlot);
			if (m_Skills.end() == owner ||
				!skillCombatTraitOwners.insert(ownerSkillId).second ||
				(0u != counterPower &&
				 LostArk::Shared::PLAYER_SKILL_KIND::COUNTER !=
					owner->second.eSkillKind && !isQuickCounterSlot))
			{
				m_strStatus =
					"Player skill combat traits have no owner or are duplicated";
				return false;
			}
			owner->second.iStaggerDamage = staggerDamage;
			owner->second.iPartDamage = partDamage;
			owner->second.iCounterPower = counterPower;
		}
		else if (!fields.empty() && "SKILLROOTMOTIONSCALE" == fields[0])
		{
			LostArk::Shared::SKILL_ID ownerSkillId =
				LostArk::Shared::INVALID_SKILL_ID;
			float scale = 1.f;
			if (3u != fields.size() ||
				!ParseNumber(fields[1], ownerSkillId) ||
				!ParseNumber(fields[2], scale) ||
				!std::isfinite(scale) || scale <= 0.f || scale > 8.f)
			{
				m_strStatus = "Player skill root motion scale row is invalid";
				return false;
			}
			const auto owner = m_Skills.find(ownerSkillId);
			if (m_Skills.end() == owner ||
				!skillRootMotionScaleOwners.insert(ownerSkillId).second)
			{
				m_strStatus =
					"Player skill root motion scale has no owner or is duplicated";
				return false;
			}
			owner->second.fRootMotionScale = scale;
		}
		else if (!fields.empty() && "SKILLTARGET" == fields[0])
		{
			LostArk::Shared::SKILL_ID ownerSkillId =
				LostArk::Shared::INVALID_SKILL_ID;
			float maximumRange = 0.f;
			std::uint32_t requiresWalkable = 0u;
			if (5u != fields.size() ||
				!ParseNumber(fields[1], ownerSkillId) ||
				"GROUND_POINT" != fields[2] ||
				!ParseNumber(fields[3], maximumRange) ||
				!ParseNumber(fields[4], requiresWalkable) ||
				!std::isfinite(maximumRange) || maximumRange <= 0.f ||
				1u != requiresWalkable)
			{
				m_strStatus = "Player skill target row is invalid";
				return false;
			}
			const auto owner = m_Skills.find(ownerSkillId);
			if (m_Skills.end() == owner ||
				!skillTargetOwners.insert(ownerSkillId).second ||
				LostArk::Shared::PLAYER_SKILL_KIND::ACTIVE !=
					owner->second.eSkillKind ||
				maximumRange != owner->second.fMaximumRange)
			{
				m_strStatus =
					"Player skill target has no owner, is duplicated, or conflicts";
				return false;
			}
			owner->second.eTargetIntent =
				LostArk::Shared::SKILL_TARGET_INTENT_KIND::GROUND_POINT;
			owner->second.fTargetMaximumRange = maximumRange;
			owner->second.requiresWalkableTarget = true;
		}
		else if (!fields.empty() && "SKILLEMBER" == fields[0])
		{
			LostArk::Shared::SKILL_ID ownerSkillId =
				LostArk::Shared::INVALID_SKILL_ID;
			std::uint32_t gain = 0u;
			std::uint32_t cost = 0u;
			std::uint32_t locks = 0u;
			if (5u != fields.size() ||
				!ParseNumber(fields[1], ownerSkillId) ||
				!ParseNumber(fields[2], gain) ||
				!ParseNumber(fields[3], cost) ||
				!ParseNumber(fields[4], locks) ||
				locks > 1u || (0u == gain) == (0u == cost) ||
				(1u == locks && 0u == cost))
			{
				m_strStatus = "Player ember skill row is invalid";
				return false;
			}
			/* 모든 skill 뒤의 PLAYEREMBER를 읽은 다음 socket pool·소유 class의 범위를 검증한다. */
			const auto owner = m_Skills.find(ownerSkillId);
			if (m_Skills.end() == owner ||
				!emberSkillOwners.insert(ownerSkillId).second ||
				(1u == locks &&
					LostArk::Shared::PLAYER_STANCE_ID::GUARDIANKNIGHT_HUMAN !=
						owner->second.eRequiredStance))
			{
				m_strStatus =
					"Player ember skill has no owner, is duplicated, or locks outside the human form";
				return false;
			}
			owner->second.iEmberGain = gain;
			owner->second.iEmberCost = cost;
			owner->second.locksEmberSocket = 1u == locks;
		}
		else if (!fields.empty() && "SKILLSTAGE" == fields[0])
		{
			LostArk::Shared::SKILL_ID ownerSkillId =
				LostArk::Shared::INVALID_SKILL_ID;
			std::uint32_t stageIndex = 0;
			PLAYER_COMBO_STAGE stage{};
			if (8u != fields.size() ||
				!ParseNumber(fields[1], ownerSkillId) ||
				!ParseNumber(fields[2], stageIndex) ||
				!ParseNumber(fields[3], stage.iActionDurationMs) ||
				!ParseNumber(fields[4], stage.iHitTimeMs) ||
				!ParseNumber(fields[5], stage.iComboAdvanceMs) ||
				!ParseNumber(fields[6], stage.iInputOpenMs) ||
				!ParseNumber(fields[7], stage.iInputCloseMs) ||
				0u == stage.iActionDurationMs ||
				stage.iHitTimeMs > stage.iComboAdvanceMs ||
				stage.iComboAdvanceMs > stage.iActionDurationMs ||
				stage.iInputCloseMs > stage.iActionDurationMs ||
				(0u == stage.iInputCloseMs ? 0u != stage.iInputOpenMs :
					stage.iInputOpenMs >= stage.iInputCloseMs))
			{
				m_strStatus = "Combo stage row is invalid";
				return false;
			}
			const auto owner = m_Skills.find(ownerSkillId);
			// stage는 소유 skill 뒤에 순서대로 와야 한다. 알 수 없는 skill·건너뛴 index는 잘못된 bootstrap이다.
			if (owner == m_Skills.end() ||
				(LostArk::Shared::PLAYER_SKILL_KIND::COMBO !=
					owner->second.eSkillKind &&
					LostArk::Shared::PLAYER_SKILL_KIND::HOLD !=
						owner->second.eSkillKind &&
					LostArk::Shared::PLAYER_SKILL_KIND::COUNTER !=
						owner->second.eSkillKind) ||
				stageIndex != owner->second.ComboStages.size())
			{
				m_strStatus = "Combo stage does not follow its skill";
				return false;
			}
			owner->second.ComboStages.push_back(stage);
		}
		else if (!fields.empty() && "SKILLROOTMOTION" == fields[0])
		{
			LostArk::Shared::SKILL_ID ownerSkillId =
				LostArk::Shared::INVALID_SKILL_ID;
			std::uint32_t sampleCount = 0;
			if (4u != fields.size() ||
				!ParseNumber(fields[1], ownerSkillId) ||
				!ParseNumber(fields[2], sampleCount) ||
				sampleCount < 2u || sampleCount > 512u)
			{
				m_strStatus = "Root motion row is invalid";
				return false;
			}
			const auto owner = m_Skills.find(ownerSkillId);
			if (owner == m_Skills.end() || !owner->second.RootMotion.empty())
			{
				m_strStatus = "Root motion does not follow its skill";
				return false;
			}
			std::vector<ROOT_MOTION_SAMPLE> samples;
			if (!Parse_RootMotionSamples(
				fields[3], sampleCount, owner->second.iActionDurationMs, samples))
			{
				return false;
			}
			owner->second.RootMotion = std::move(samples);
		}
		else if (!fields.empty() && "PATTERNSTAGEROOTMOTION" == fields[0])
		{
			/* 같은 encounter의 PATTERNSTAGE 뒤에 오는 curve다. 없는 stage나 한 stage의 중복 curve는 거부한다. */
			std::uint32_t stageIndex = 0;
			std::uint32_t sampleCount = 0;
			if (6u != fields.size() ||
				!ParseNumber(fields[3], stageIndex) ||
				!ParseNumber(fields[4], sampleCount) ||
				sampleCount < 2u || sampleCount > 512u)
			{
				m_strStatus = "Pattern stage root motion row is invalid";
				return false;
			}
			const auto encounter = m_BossPatterns.find(std::string(fields[1]));
			if (encounter == m_BossPatterns.end())
			{
				m_strStatus = "Pattern stage root motion names an unknown encounter";
				return false;
			}
			const auto pattern = std::find_if(
				encounter->second.begin(), encounter->second.end(),
				[&fields](const BOSS_PATTERN_DEFINITION& candidate)
				{ return candidate.strPatternId == fields[2]; });
			if (encounter->second.end() == pattern ||
				stageIndex >= pattern->Stages.size() ||
				BOSS_PATTERN_STAGE_MOTION_KIND::PORTAL_TARGET_RUSH ==
					pattern->Stages[stageIndex].Motion.eKind ||
				!pattern->Stages[stageIndex].Motion.RootMotion.empty())
			{
				m_strStatus = "Pattern stage root motion does not follow its stage";
				return false;
			}
			std::vector<ROOT_MOTION_SAMPLE> samples;
			if (!Parse_RootMotionSamples(
				fields[5], sampleCount,
				pattern->Stages[stageIndex].iDurationMs, samples))
			{
				return false;
			}
			pattern->Stages[stageIndex].Motion.RootMotion = std::move(samples);
		}
		else if (!fields.empty() && "SKILLSTAGEROOTMOTION" == fields[0])
		{
			LostArk::Shared::SKILL_ID ownerSkillId =
				LostArk::Shared::INVALID_SKILL_ID;
			std::uint32_t stageIndex = 0;
			std::uint32_t sampleCount = 0;
			if (5u != fields.size() ||
				!ParseNumber(fields[1], ownerSkillId) ||
				!ParseNumber(fields[2], stageIndex) ||
				!ParseNumber(fields[3], sampleCount) ||
				sampleCount < 2u || sampleCount > 512u)
			{
				m_strStatus = "Root motion row is invalid";
				return false;
			}
			const auto owner = m_Skills.find(ownerSkillId);
			// 소유 SKILLCOMBOSTAGE가 먼저 읽혔으므로 범위 밖 stage index는 거부한다.
			if (owner == m_Skills.end() ||
				stageIndex >= owner->second.ComboStages.size() ||
				!owner->second.ComboStages[stageIndex].RootMotion.empty())
			{
				m_strStatus = "Root motion does not follow its skill";
				return false;
			}
			PLAYER_COMBO_STAGE& stage = owner->second.ComboStages[stageIndex];
			std::vector<ROOT_MOTION_SAMPLE> samples;
			if (!Parse_RootMotionSamples(
				fields[4], sampleCount, stage.iActionDurationMs, samples))
			{
				return false;
			}
			stage.RootMotion = std::move(samples);
		}
		else if (!fields.empty() && "SKILLCANCEL" == fields[0])
		{
			LostArk::Shared::SKILL_ID ownerSkillId =
				LostArk::Shared::INVALID_SKILL_ID;
			std::uint32_t windowCount = 0;
			if (5u != fields.size() ||
				!ParseNumber(fields[1], ownerSkillId) ||
				("SKILL" != fields[2] && "MOVE" != fields[2] &&
					"DODGE" != fields[2]) ||
				!ParseNumber(fields[3], windowCount) ||
				windowCount < 1u || windowCount > 8u)
			{
				m_strStatus = "Skill cancel row is invalid";
				return false;
			}
			const auto owner = m_Skills.find(ownerSkillId);
			if (owner == m_Skills.end())
			{
				m_strStatus = "Skill cancel row does not follow its skill";
				return false;
			}
			std::vector<PLAYER_CANCEL_WINDOW>& windows =
				"SKILL" == fields[2] ? owner->second.SkillCancelWindows :
				"MOVE" == fields[2] ? owner->second.MoveCancelWindows :
				owner->second.DodgeCancelWindows;
			if (!windows.empty())
			{
				m_strStatus = "Skill cancel row is duplicated";
				return false;
			}
			if (!Parse_CancelWindows(
				fields[4], windowCount, owner->second.iActionDurationMs, windows))
			{
				return false;
			}
		}
		else if (!fields.empty() && "SKILLSTAGECANCEL" == fields[0])
		{
			LostArk::Shared::SKILL_ID ownerSkillId =
				LostArk::Shared::INVALID_SKILL_ID;
			std::uint32_t stageIndex = 0;
			std::uint32_t windowCount = 0;
			if (6u != fields.size() ||
				!ParseNumber(fields[1], ownerSkillId) ||
				!ParseNumber(fields[2], stageIndex) ||
				("SKILL" != fields[3] && "MOVE" != fields[3] &&
					"DODGE" != fields[3]) ||
				!ParseNumber(fields[4], windowCount) ||
				windowCount < 1u || windowCount > 8u)
			{
				m_strStatus = "Skill stage cancel row is invalid";
				return false;
			}
			const auto owner = m_Skills.find(ownerSkillId);
			if (owner == m_Skills.end() ||
				stageIndex >= owner->second.ComboStages.size())
			{
				m_strStatus = "Skill stage cancel row does not follow its skill";
				return false;
			}
			PLAYER_COMBO_STAGE& stage = owner->second.ComboStages[stageIndex];
			std::vector<PLAYER_CANCEL_WINDOW>& windows =
				"SKILL" == fields[3] ? stage.SkillCancelWindows :
				"MOVE" == fields[3] ? stage.MoveCancelWindows :
				stage.DodgeCancelWindows;
			if (!windows.empty())
			{
				m_strStatus = "Skill stage cancel row is duplicated";
				return false;
			}
			if (!Parse_CancelWindows(
				fields[5], windowCount, stage.iActionDurationMs, windows))
			{
				return false;
			}
		}
		else if (!fields.empty() && "SKILLHIT" == fields[0])
		{
			LostArk::Shared::SKILL_ID ownerSkillId =
				LostArk::Shared::INVALID_SKILL_ID;
			std::uint32_t hitCount = 0;
			if (4u != fields.size() ||
				!ParseNumber(fields[1], ownerSkillId) ||
				!ParseNumber(fields[2], hitCount) ||
				hitCount < 1u || hitCount > 192u)
			{
				m_strStatus = "Skill hit row is invalid";
				return false;
			}
			const auto owner = m_Skills.find(ownerSkillId);
			if (owner == m_Skills.end() || !owner->second.Hits.empty() ||
				owner->second.strDamageProfileId.empty() ||
				LostArk::Shared::PLAYER_SKILL_KIND::ACTIVE !=
					owner->second.eSkillKind)
			{
				m_strStatus = "Skill hit row does not follow its skill";
				return false;
			}
			std::vector<PLAYER_SKILL_HIT> hits;
			if (!Parse_SkillHits(
				fields[3], hitCount, owner->second.iActionDurationMs, hits))
			{
				return false;
			}
			owner->second.Hits = std::move(hits);
		}
		else if (!fields.empty() && "SKILLSTAGEHIT" == fields[0])
		{
			LostArk::Shared::SKILL_ID ownerSkillId =
				LostArk::Shared::INVALID_SKILL_ID;
			std::uint32_t stageIndex = 0;
			std::uint32_t hitCount = 0;
			if (5u != fields.size() ||
				!ParseNumber(fields[1], ownerSkillId) ||
				!ParseNumber(fields[2], stageIndex) ||
				!ParseNumber(fields[3], hitCount) ||
				hitCount < 1u || hitCount > 192u)
			{
				m_strStatus = "Skill stage hit row is invalid";
				return false;
			}
			const auto owner = m_Skills.find(ownerSkillId);
			if (owner == m_Skills.end() ||
				owner->second.strDamageProfileId.empty() ||
				stageIndex >= owner->second.ComboStages.size() ||
				!owner->second.ComboStages[stageIndex].Hits.empty())
			{
				m_strStatus = "Skill stage hit row does not follow its skill";
				return false;
			}
			PLAYER_COMBO_STAGE& stage = owner->second.ComboStages[stageIndex];
			std::vector<PLAYER_SKILL_HIT> hits;
			if (!Parse_SkillHits(
				fields[4], hitCount, stage.iActionDurationMs, hits))
			{
				return false;
			}
			stage.Hits = std::move(hits);
		}
		else if (!fields.empty() && "SKILLPROJ" == fields[0])
		{
			LostArk::Shared::SKILL_ID ownerSkillId =
				LostArk::Shared::INVALID_SKILL_ID;
			if (fields.size() < 2u || !ParseNumber(fields[1], ownerSkillId))
			{
				m_strStatus = "Skill projectile row is invalid";
				return false;
			}
			const auto owner = m_Skills.find(ownerSkillId);
			if (owner == m_Skills.end() ||
				owner->second.strDamageProfileId.empty() ||
				LostArk::Shared::PLAYER_SKILL_KIND::ACTIVE !=
					owner->second.eSkillKind)
			{
				m_strStatus = "Skill projectile row does not follow its skill";
				return false;
			}
			if (!Parse_SkillProjectile(fields, 2u,
				owner->second.iActionDurationMs, owner->second.Projectiles))
			{
				return false;
			}
		}
		else if (!fields.empty() && "SKILLSTAGEPROJ" == fields[0])
		{
			LostArk::Shared::SKILL_ID ownerSkillId =
				LostArk::Shared::INVALID_SKILL_ID;
			std::uint32_t stageIndex = 0;
			if (fields.size() < 3u || !ParseNumber(fields[1], ownerSkillId) ||
				!ParseNumber(fields[2], stageIndex))
			{
				m_strStatus = "Skill stage projectile row is invalid";
				return false;
			}
			const auto owner = m_Skills.find(ownerSkillId);
			if (owner == m_Skills.end() ||
				owner->second.strDamageProfileId.empty() ||
				stageIndex >= owner->second.ComboStages.size())
			{
				m_strStatus = "Skill stage projectile row does not follow its skill";
				return false;
			}
			PLAYER_COMBO_STAGE& stage = owner->second.ComboStages[stageIndex];
			if (!Parse_SkillProjectile(fields, 3u,
				stage.iActionDurationMs, stage.Projectiles))
			{
				return false;
			}
		}
		else if (!fields.empty() && "BOSS" == fields[0])
		{
			BOSS_RUNTIME_PROFILE boss{};
			if ((11u != fields.size() && 12u != fields.size()) || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) ||
				!ParseNumber(fields[3], boss.iMaximumHp) ||
				!ParseNumber(fields[4], boss.iMaximumHealthBars) ||
				!ParseNumber(fields[5], boss.iAttackPower) ||
				!ParseNumber(fields[6], boss.fCollisionRadius) ||
				!ParseNumber(fields[7], boss.fEngageDistance) ||
				!ParseNumber(fields[8], boss.fMoveSpeed) ||
				!ParseBossPhasePolicyKind(fields[9], boss.PhasePolicy.eKind) ||
				!ParseNumber(fields[10], boss.PhasePolicy.iThresholdPercent) ||
				(12u == fields.size() &&
					(!ParseNumber(fields[11], boss.iDamageReferenceHp) || 0u == boss.iDamageReferenceHp)) ||
				0u == boss.iMaximumHp || 0u == boss.iMaximumHealthBars ||
				boss.iMaximumHealthBars > 1000u || 0u == boss.iAttackPower ||
				!std::isfinite(boss.fCollisionRadius) ||
				boss.fCollisionRadius <= 0.f ||
				!std::isfinite(boss.fEngageDistance) ||
				!std::isfinite(boss.fMoveSpeed) || boss.fEngageDistance <= 0.f ||
				boss.fMoveSpeed <= 0.f ||
				(BOSS_PHASE_POLICY_KIND::HEALTH_PERCENT_THRESHOLD ==
					boss.PhasePolicy.eKind &&
					(0u == boss.PhasePolicy.iThresholdPercent ||
					 boss.PhasePolicy.iThresholdPercent >= 100u)) ||
				(BOSS_PHASE_POLICY_KIND::AUTHORED_PATTERN_EVENT ==
					boss.PhasePolicy.eKind &&
					0u != boss.PhasePolicy.iThresholdPercent))
			{
				m_strStatus = "Boss profile row is invalid";
				return false;
			}
			boss.strArchetypeId = fields[1];
			boss.strEncounterId = fields[2];
			if (!m_Bosses.emplace(boss.strArchetypeId, std::move(boss)).second)
			{
				m_strStatus = "Duplicate boss archetype ID";
				return false;
			}
		}
		else if (!fields.empty() && "BOSSARMOR" == fields[0])
		{
			BOSS_ARMOR_PLATE plate{};
			if (5u != fields.size() || !IsStableId(fields[1]) ||
				!ParseNumber(fields[2], plate.iPlateIndex) ||
				!ParseNumber(fields[3], plate.iDurability) ||
				!ParseNumber(fields[4], plate.iDefense) ||
				0u == plate.iDurability || 0u == plate.iDefense ||
				plate.iDefense > 10000u)
			{
				m_strStatus = "Boss armour plate row is invalid";
				return false;
			}
			const auto owner = m_Bosses.find(std::string(fields[1]));
			if (m_Bosses.end() == owner)
			{
				m_strStatus = "Boss armour plate has no boss profile";
				return false;
			}
			/* Client의 부위 순서와 대응하도록 빈 번호 없는 오름차순 index를 사용한다. */
			if (plate.iPlateIndex != owner->second.ArmorPlates.size() ||
				owner->second.ArmorPlates.size() >= MAXIMUM_BOSS_ARMOR_PLATES)
			{
				m_strStatus = "Boss armour plate index is out of order";
				return false;
			}
			owner->second.ArmorPlates.push_back(plate);
		}
		else if (!fields.empty() && "BOSSPART" == fields[0])
		{
			BOSS_PART_DEFINITION part{};
			if (7u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) ||
				!ParseNumber(fields[3], part.iStateMask) ||
				!ParseNumber(fields[4], part.iMaximumDurability) ||
				!ParseNumber(fields[5], part.iDamageReductionPercent) ||
				!ParseBossPartDamageCondition(fields[6], part.eDamageCondition) ||
				0u == part.iStateMask ||
				0u != (part.iStateMask & (part.iStateMask - 1u)) ||
				0u == part.iMaximumDurability ||
				part.iMaximumDurability > 1000000u ||
				0u == part.iDamageReductionPercent ||
				part.iDamageReductionPercent > 90u ||
				m_Bosses.end() == m_Bosses.find(std::string(fields[1])))
			{
				m_strStatus = "Boss part row is invalid";
				return false;
			}
			part.strBossArchetypeId = fields[1];
			part.strPartId = fields[2];
			auto& parts = m_BossParts[part.strBossArchetypeId];
			if (std::any_of(parts.begin(), parts.end(),
				[&part](const BOSS_PART_DEFINITION& existing)
				{
					return existing.strPartId == part.strPartId ||
						existing.iStateMask == part.iStateMask;
				}))
			{
				m_strStatus = "Duplicate boss part ID or state mask";
				return false;
			}
			parts.push_back(std::move(part));
		}
		else if (!fields.empty() && "BOSSCOMBATOBJECT" == fields[0])
		{
			BOSS_COMBAT_OBJECT_DEFINITION definition{};
			const bool hasMovementLifetimePolicy =
				17u == fields.size() || 18u == fields.size();
			const bool hasCoverRadius =
				16u == fields.size() || 18u == fields.size();
			std::uint32_t expireOnDistanceEnd = 1u;
			const std::size_t coverRadiusIndex =
				hasMovementLifetimePolicy ? 15u : 13u;
			const std::size_t lifeIndex = coverRadiusIndex +
				(hasCoverRadius ? 1u : 0u);
			const std::size_t eventCountIndex = lifeIndex + 1u;
			if ((15u != fields.size() && 16u != fields.size() &&
				 17u != fields.size() && 18u != fields.size()) ||
				!IsStableId(fields[1]) ||
				!IsStableId(fields[2]) || !IsStableId(fields[3]) ||
				!IsStableId(fields[4]) || !IsStableId(fields[5]) ||
				!ParseBossCombatObjectKind(fields[6], definition.eKind) ||
				!ParseBossCombatObjectOriginPolicy(
					fields[7], definition.eOriginPolicy) ||
				!ParseBossCombatObjectDirectionPolicy(
					fields[8], definition.eDirectionPolicy) ||
				!ParseNumber(fields[9], definition.fOffsetForwardM) ||
				!ParseNumber(fields[10], definition.fOffsetRightM) ||
				!ParseNumber(fields[11], definition.fSpeedMps) ||
				!ParseNumber(fields[12], definition.fMaximumDistanceM) ||
				(hasMovementLifetimePolicy &&
					(!ParseNumber(fields[13], definition.iMovementStartDelayMs) ||
					 !ParseNumber(fields[14], expireOnDistanceEnd) ||
					 expireOnDistanceEnd > 1u)) ||
				(hasCoverRadius &&
					!ParseNumber(fields[coverRadiusIndex], definition.fCoverRadiusM)) ||
				!ParseNumber(fields[lifeIndex], definition.iLifeMs) ||
				!ParseNumber(fields[eventCountIndex], definition.iExpectedEventCount) ||
				!std::isfinite(definition.fOffsetForwardM) ||
				!std::isfinite(definition.fOffsetRightM) ||
				!std::isfinite(definition.fSpeedMps) ||
				!std::isfinite(definition.fMaximumDistanceM) ||
				!std::isfinite(definition.fCoverRadiusM) ||
				std::abs(definition.fOffsetForwardM) > 100.f ||
				std::abs(definition.fOffsetRightM) > 100.f ||
				definition.fSpeedMps < 0.f || definition.fSpeedMps > 1000.f ||
				definition.fMaximumDistanceM < 0.f ||
				definition.fMaximumDistanceM > 1000.f ||
				definition.iMovementStartDelayMs > 600000u ||
				definition.fCoverRadiusM < 0.f ||
				definition.fCoverRadiusM > 100.f ||
				0u == definition.iLifeMs || definition.iLifeMs > 600000u ||
				0u == definition.iExpectedEventCount ||
				definition.iExpectedEventCount > 16u)
			{
				m_strStatus = "Boss combat object row is invalid";
				return false;
			}
			definition.bExpireOnDistanceEnd = 0u != expireOnDistanceEnd;
			const bool fixedArea = BOSS_COMBAT_OBJECT_KIND::FIXED_AREA ==
				definition.eKind;
			/* 고정 범위를 pattern의 단일 대상 또는 volley의 생존 player별 위치에 배치한다. */
			const bool lockedOrigin =
				BOSS_COMBAT_OBJECT_ORIGIN_POLICY::
					LOCKED_TARGET_UNTIL_FIRST_PULSE == definition.eOriginPolicy ||
				BOSS_COMBAT_OBJECT_ORIGIN_POLICY::
					LOCKED_TARGET_PER_ALIVE_PLAYER == definition.eOriginPolicy;
			const bool birthPoseOrigin = BOSS_COMBAT_OBJECT_ORIGIN_POLICY::
				BOSS_POSITION == definition.eOriginPolicy;
			const bool validFixedArea = fixedArea && (lockedOrigin || birthPoseOrigin) &&
				BOSS_COMBAT_OBJECT_DIRECTION_POLICY::NONE ==
					definition.eDirectionPolicy &&
				(birthPoseOrigin || (0.f == definition.fOffsetForwardM &&
				 0.f == definition.fOffsetRightM)) && 0.f == definition.fSpeedMps &&
				0.f == definition.fMaximumDistanceM &&
				0u == definition.iMovementStartDelayMs &&
				definition.bExpireOnDistanceEnd;
			const double maximumTravelM =
				static_cast<double>(definition.fSpeedMps) *
				(static_cast<double>(definition.iLifeMs -
					definition.iMovementStartDelayMs) / 1000.0);
			const bool validMissileDirection =
				BOSS_COMBAT_OBJECT_DIRECTION_POLICY::PATTERN_FACING_AT_SPAWN ==
					definition.eDirectionPolicy ||
				BOSS_COMBAT_OBJECT_DIRECTION_POLICY::RADIAL_INWARD ==
					definition.eDirectionPolicy ||
				BOSS_COMBAT_OBJECT_DIRECTION_POLICY::NEXT_RADIAL_SLOT ==
					definition.eDirectionPolicy;
			const bool validMissile = !fixedArea &&
				BOSS_COMBAT_OBJECT_ORIGIN_POLICY::BOSS_POSITION ==
					definition.eOriginPolicy &&
				validMissileDirection &&
				definition.iMovementStartDelayMs < definition.iLifeMs &&
				definition.fSpeedMps > 0.f && definition.fMaximumDistanceM > 0.f &&
				maximumTravelM + 0.00001 >= definition.fMaximumDistanceM;
			if (!validFixedArea && !validMissile)
			{
				m_strStatus = "Boss combat object motion contract is invalid";
				return false;
			}
			definition.strEncounterId = fields[1];
			definition.strCombatObjectArchetypeId = fields[2];
			definition.strClientVisualId = fields[3];
			definition.strOwnerPatternId = fields[4];
			definition.strOwnerStageActionId = fields[5];
			if (!m_BossCombatObjects.emplace(
				definition.strCombatObjectArchetypeId,
				std::move(definition)).second)
			{
				m_strStatus = "Duplicate boss combat object archetype ID";
				return false;
			}
		}
		else if (!fields.empty() && "BOSSCOMBATOBJECTOWNERHITCHAIN" == fields[0])
		{
			std::uint32_t delayMs = 0u;
			if (6u != fields.size() || !IsStableId(fields[1]) || !IsStableId(fields[2]) ||
				!IsStableId(fields[3]) || !ParseNumber(fields[4], delayMs) ||
				delayMs == 0u || delayMs > 600000u || !IsStableId(fields[5]))
			{
				m_strStatus = "Boss combat object owner hit chain row is invalid";
				return false;
			}
			const auto owner = m_BossCombatObjects.find(std::string(fields[2]));
			if (owner == m_BossCombatObjects.end() || owner->second.strEncounterId != fields[1] ||
				!owner->second.OwnerHitChain.strTriggerActionId.empty())
			{
				m_strStatus = "Boss combat object owner hit chain has no unique owner";
				return false;
			}
			owner->second.OwnerHitChain = {std::string(fields[3]), std::string(fields[5]), delayMs};
		}
		else if (!fields.empty() && "BOSSCOMBATOBJECTHIT" == fields[0])
		{
			BOSS_COMBAT_OBJECT_HIT hit{};
			std::uint32_t hitIndex = 0;
			std::uint32_t knockdown = 0;
			if (20u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) || !ParseNumber(fields[3], hitIndex) ||
				!IsStableId(fields[4]) ||
				!ParseBossCombatObjectHitTrigger(fields[5], hit.eTrigger) ||
				!ParseNumber(fields[6], hit.iAtMs) ||
				!ParseNumber(fields[7], hit.iRepeatCount) ||
				!ParseNumber(fields[8], hit.iRepeatIntervalMs) ||
				!ParseBossPatternHitShape(fields[9], hit.eHitShape) ||
				!ParseNumber(fields[10], hit.fHitOuterRadius) ||
				!ParseNumber(fields[11], hit.fHitInnerRadius) ||
				!ParseNumber(fields[12], hit.fHitAngleDegrees) ||
				!ParseNumber(fields[13], hit.fHitLength) ||
				!ParseNumber(fields[14], hit.fHitHalfWidth) ||
				!IsStableId(fields[15]) ||
				!ParseNumber(fields[16], hit.fPushRangeM) ||
				!ParseNumber(fields[17], hit.iPushMs) ||
				!ParseNumber(fields[18], knockdown) || knockdown > 1u ||
				!ParseNumber(fields[19], hit.iDownMs) ||
				0u == hit.iRepeatCount || hit.iRepeatCount > 64u ||
				(1u == hit.iRepeatCount ? 0u != hit.iRepeatIntervalMs :
					0u == hit.iRepeatIntervalMs) ||
				!std::isfinite(hit.fHitOuterRadius) ||
				!std::isfinite(hit.fHitInnerRadius) ||
				!std::isfinite(hit.fHitAngleDegrees) ||
				!std::isfinite(hit.fHitLength) ||
				!std::isfinite(hit.fHitHalfWidth) ||
				!std::isfinite(hit.fPushRangeM) ||
				std::abs(hit.fPushRangeM) > 20.f ||
				((0.f == hit.fPushRangeM) != (0u == hit.iPushMs)) ||
				((0u != knockdown) != (0u != hit.iDownMs)))
			{
				m_strStatus = "Boss combat object hit row is invalid";
				return false;
			}
			const auto owner = m_BossCombatObjects.find(std::string(fields[2]));
			if (m_BossCombatObjects.end() == owner ||
				owner->second.strEncounterId != std::string(fields[1]) ||
				hitIndex != owner->second.Hits.size() ||
				hitIndex >= owner->second.iExpectedEventCount ||
				std::any_of(owner->second.Hits.begin(), owner->second.Hits.end(),
					[&fields](const BOSS_COMBAT_OBJECT_HIT& existing)
					{
						return existing.strHitId == fields[4];
					}))
			{
				m_strStatus = "Boss combat object hit does not follow its owner";
				return false;
			}
			const std::uint64_t lastPulseMs = hit.iAtMs +
				static_cast<std::uint64_t>(hit.iRepeatCount - 1u) *
				hit.iRepeatIntervalMs;
			const bool fixedArea = BOSS_COMBAT_OBJECT_KIND::FIXED_AREA ==
				owner->second.eKind;
			if (lastPulseMs >= owner->second.iLifeMs ||
				(fixedArea && BOSS_COMBAT_OBJECT_HIT_TRIGGER::TIMED !=
					hit.eTrigger) ||
				(!fixedArea && (BOSS_COMBAT_OBJECT_HIT_TRIGGER::CONTACT !=
					hit.eTrigger || 0u != hit.iAtMs)))
			{
				m_strStatus = "Boss combat object hit timing is invalid";
				return false;
			}
			const bool zeroAngleBox = 0.f == hit.fHitAngleDegrees &&
				0.f == hit.fHitLength && 0.f == hit.fHitHalfWidth;
			bool validShape = false;
			switch (hit.eHitShape)
			{
			case BOSS_PATTERN_HIT_SHAPE::CIRCLE:
				validShape = hit.fHitOuterRadius > 0.f &&
					0.f == hit.fHitInnerRadius && zeroAngleBox;
				break;
			case BOSS_PATTERN_HIT_SHAPE::RING:
				validShape = hit.fHitOuterRadius > hit.fHitInnerRadius &&
					hit.fHitInnerRadius > 0.f && zeroAngleBox;
				break;
			case BOSS_PATTERN_HIT_SHAPE::CONE:
				validShape = hit.fHitAngleDegrees > 0.f &&
					hit.fHitAngleDegrees <= 180.f && hit.fHitLength > 0.f &&
					0.f == hit.fHitOuterRadius && 0.f == hit.fHitInnerRadius &&
					0.f == hit.fHitHalfWidth;
				break;
			case BOSS_PATTERN_HIT_SHAPE::BOX:
			case BOSS_PATTERN_HIT_SHAPE::CROSS:
			case BOSS_PATTERN_HIT_SHAPE::SIX_DIRECTIONS:
				validShape = hit.fHitLength > 0.f && hit.fHitHalfWidth > 0.f &&
					0.f == hit.fHitOuterRadius && 0.f == hit.fHitInnerRadius &&
					0.f == hit.fHitAngleDegrees;
				break;
			case BOSS_PATTERN_HIT_SHAPE::NONE:
				break;
			}
			if (!validShape)
			{
				m_strStatus = "Boss combat object hit shape is invalid";
				return false;
			}
			hit.strHitId = fields[4];
			hit.strDamageProfileId = fields[15];
			hit.bKnockdown = 0u != knockdown;
			owner->second.Hits.push_back(std::move(hit));
		}
		else if (!fields.empty() &&
			"BOSSCOMBATOBJECTPRESENTATION" == fields[0])
		{
			BOSS_COMBAT_OBJECT_PRESENTATION_PULSE pulse{};
			std::uint32_t pulseIndex = 0u;
			if (6u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) || !ParseNumber(fields[3], pulseIndex) ||
				!IsStableId(fields[4]) || !ParseNumber(fields[5], pulse.iAtMs))
			{
				m_strStatus =
					"Boss combat object presentation row is invalid";
				return false;
			}
			const auto owner = m_BossCombatObjects.find(std::string(fields[2]));
			if (m_BossCombatObjects.end() == owner ||
				owner->second.strEncounterId != std::string(fields[1]) ||
				pulseIndex != owner->second.PresentationPulses.size() ||
				owner->second.Hits.size() + pulseIndex >=
					owner->second.iExpectedEventCount ||
				pulse.iAtMs > owner->second.iLifeMs ||
				std::any_of(
					owner->second.Hits.begin(), owner->second.Hits.end(),
					[&fields](const BOSS_COMBAT_OBJECT_HIT& existing)
					{
						return existing.strHitId == fields[4];
					}) ||
				std::any_of(
					owner->second.PresentationPulses.begin(),
					owner->second.PresentationPulses.end(),
					[&fields](
						const BOSS_COMBAT_OBJECT_PRESENTATION_PULSE& existing)
					{
						return existing.strPresentationEventId == fields[4];
					}))
			{
				m_strStatus =
					"Boss combat object presentation does not follow its owner";
				return false;
			}
			pulse.strPresentationEventId = fields[4];
			owner->second.PresentationPulses.push_back(std::move(pulse));
		}
		else if (!fields.empty() && "PATTERN" == fields[0])
		{
			BOSS_PATTERN_DEFINITION pattern{};
			std::uint32_t invulnerableFlag = 0u;
			if (17u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) || !IsStableId(fields[3]) ||
				!ParseBossPatternSelection(fields[4], pattern.eSelection) ||
				!ParseNumber(fields[5], pattern.iMinimumHealthBar) ||
				!ParseNumber(fields[6], pattern.iMaximumHealthBar) ||
				!ParseNumber(fields[7], pattern.iTriggerHealthBar) ||
				!ParseNumber(fields[8], pattern.iTriggerOrder) ||
				!ParseNumber(fields[9], pattern.iSelectionWeight) ||
				!ParseNumber(fields[10], pattern.iMaximumConsecutiveUses) ||
				!ParseNumber(fields[11], pattern.fMinimumRange) ||
				!ParseNumber(fields[12], pattern.fMaximumRange) ||
				!ParseNumber(fields[13], pattern.iExpectedStageCount) ||
				!ParseBossPatternArmorRequirement(
					fields[14], pattern.eArmorRequirement) ||
				!ParseBossPatternPhaseRequirement(
					fields[15], pattern.ePhaseRequirement) ||
				!ParseNumber(fields[16], invulnerableFlag) ||
				invulnerableFlag > 1u ||
				!std::isfinite(pattern.fMinimumRange) ||
				!std::isfinite(pattern.fMaximumRange) ||
				pattern.fMinimumRange < 0.f ||
				pattern.fMaximumRange <= pattern.fMinimumRange ||
				0u == pattern.iExpectedStageCount ||
				pattern.iExpectedStageCount > 64u)
			{
				m_strStatus = "Boss pattern row is invalid";
				return false;
			}
			pattern.bInvulnerableWhileRunning = 1u == invulnerableFlag;
			pattern.strEncounterId = fields[1];
			pattern.strPatternId = fields[2];
			pattern.strActionId = fields[3];
			auto& patterns = m_BossPatterns[pattern.strEncounterId];
			if (std::any_of(patterns.begin(), patterns.end(),
				[&pattern](const BOSS_PATTERN_DEFINITION& existing)
				{
					return existing.strPatternId == pattern.strPatternId ||
						existing.strActionId == pattern.strActionId;
				}))
			{
				m_strStatus = "Duplicate boss pattern or action ID";
				return false;
			}
			patterns.push_back(std::move(pattern));
		}
		else if (!fields.empty() && "PATTERNAUTHORINGMANAGED" == fields[0])
		{
			if (3u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]))
			{
				m_strStatus = "Boss managed pattern row is invalid";
				return false;
			}
			const auto owners = m_BossPatterns.find(std::string(fields[1]));
			if (m_BossPatterns.end() == owners)
			{
				m_strStatus = "Boss managed pattern encounter is missing";
				return false;
			}
			const auto owner = std::find_if(owners->second.begin(), owners->second.end(),
				[&fields](const BOSS_PATTERN_DEFINITION& pattern)
				{ return pattern.strPatternId == fields[2]; });
			if (owners->second.end() == owner || owner->bAuthoringMasterManaged)
			{
				m_strStatus = "Boss managed pattern is missing or duplicated";
				return false;
			}
			owner->bAuthoringMasterManaged = true;
		}
		else if (!fields.empty() && "PATTERNBOSS" == fields[0])
		{
			/* 앞서 읽은 쿠크 pattern을 실행할 boss archetype이다. 모든 BOSS 행을 읽은 뒤 boss 자체도 검증한다. */
			if (4u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) || !IsStableId(fields[3]))
			{
				m_strStatus = "Boss pattern audition boss row is invalid";
				return false;
			}
			const auto owners = m_BossPatterns.find(std::string(fields[1]));
			if (m_BossPatterns.end() == owners)
			{
				m_strStatus = "Boss pattern audition boss encounter is missing";
				return false;
			}
			const auto owner = std::find_if(owners->second.begin(), owners->second.end(),
				[&fields](const BOSS_PATTERN_DEFINITION& pattern)
				{ return pattern.strPatternId == fields[2]; });
			if (owners->second.end() == owner ||
				owner->AuditionBossArchetypeIds.end() != std::find(
					owner->AuditionBossArchetypeIds.begin(),
					owner->AuditionBossArchetypeIds.end(), fields[3]) ||
				owner->AuditionBossArchetypeIds.size() >= 8u)
			{
				m_strStatus = "Boss pattern audition boss is missing, duplicated, or over capacity";
				return false;
			}
			owner->AuditionBossArchetypeIds.emplace_back(fields[3]);
		}
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
				!std::isfinite(window.fOuterRadiusM) || window.fOuterRadiusM < 0.f ||
				!ParseNumber(fields[12], window.fStopYawDegrees) ||
				!std::isfinite(window.fStopYawDegrees) ||
				!ParseNumber(fields[13], window.fHalfAngleDegrees) ||
				!std::isfinite(window.fHalfAngleDegrees) ||
				window.fHalfAngleDegrees < 0.f || window.fHalfAngleDegrees > 180.f ||
				!ParseNumber(fields[14], window.fMaxDistanceM) ||
				!std::isfinite(window.fMaxDistanceM) || window.fMaxDistanceM < 0.f ||
				!ParseNumber(fields[15], window.iPoseIndex) || window.iPoseIndex > 7u ||
				!ParseNumber(fields[16], window.iThreshold) ||
				(window.eKind == BOSS_PATTERN_LOGIC_KIND::INVULNERABILITY_ZONE && window.iThreshold > 4u) ||
				!ParseNumber(fields[17], window.fShieldArcDegrees) ||
				!std::isfinite(window.fShieldArcDegrees) ||
				window.fShieldArcDegrees < 0.f || window.fShieldArcDegrees > 360.f ||
				!ParseNumber(fields[18], endsPattern) || endsPattern > 1u ||
				!ParseMechanicCardSymbols(fields[19], window.SectorSymbols))
			{
				m_strStatus = "Boss pattern logic window row is invalid";
				return false;
			}
			if (fields.size() >= 21u && (!ParseNumber(fields[20], window.fNormalYawOffsetDegrees) ||
				!std::isfinite(window.fNormalYawOffsetDegrees)))
			{ m_strStatus = "Boss shield normal offset is invalid"; return false; }
			if (fields.size() >= 22u)
			{
				if (fields[21] != "0" && fields[21] != "1")
				{ m_strStatus = "Boss Logic inside outcome is invalid"; return false; }
				window.bInsideIsFail = fields[21] == "1";
			}
			if (BOSS_PATTERN_LOGIC_KIND::OBJECT_OVERLAP == window.eKind)
			{
				if (fields.size() != 26u || !IsStableId(fields[22]) ||
					!ParseNumber(fields[23], window.fTargetWorldX) || !std::isfinite(window.fTargetWorldX) ||
					!ParseNumber(fields[24], window.fTargetWorldZ) || !std::isfinite(window.fTargetWorldZ) ||
					!ParseNumber(fields[25], window.fTargetRadiusM) || !std::isfinite(window.fTargetRadiusM) ||
					window.fTargetRadiusM < .01f || window.fTargetRadiusM > 1000.f ||
					std::abs(window.fTargetWorldX) > 100000.f || std::abs(window.fTargetWorldZ) > 100000.f)
				{ m_strStatus = "World Object overlap target is invalid"; return false; }
				window.strTargetWorldInstanceId = std::string(fields[22]);
			}
			else if (fields.size() == 26u)
			{ m_strStatus = "Only OBJECT_OVERLAP takes a target circle"; return false; }
			window.strWindowId = std::string(fields[4]);
			window.bEndsPatternOnSuccess = 1u == endsPattern;
			const auto owners = m_BossPatterns.find(std::string(fields[1]));
			if (m_BossPatterns.end() == owners)
			{
				m_strStatus = "Boss pattern logic window encounter is missing";
				return false;
			}
			const auto owner = std::find_if(owners->second.begin(), owners->second.end(),
				[&fields](const BOSS_PATTERN_DEFINITION& pattern)
				{ return pattern.strPatternId == fields[2]; });
			if (owners->second.end() == owner ||
				owner->LogicWindows.size() != windowIndex ||
				owner->LogicWindows.size() >= 128u ||
				std::any_of(owner->LogicWindows.begin(), owner->LogicWindows.end(),
					[&window](const BOSS_PATTERN_LOGIC_WINDOW& existing)
					{ return existing.strWindowId == window.strWindowId; }))
			{
				m_strStatus = "Boss pattern logic window owner is missing, out of order, or duplicated";
				return false;
			}
			owner->LogicWindows.push_back(std::move(window));
		}
        else if (!fields.empty() && "PATTERNLOGICGAZE" == fields[0])
        {
            if (fields.size() != 4u || !IsStableId(fields[1]) || !IsStableId(fields[2]) || !IsStableId(fields[3]))
            { m_strStatus = "Gaze window policy row is invalid"; return false; }
            const auto owners = m_BossPatterns.find(std::string(fields[1]));
            if (owners == m_BossPatterns.end()) { m_strStatus = "Gaze window encounter is missing"; return false; }
            const auto pattern = std::find_if(owners->second.begin(), owners->second.end(),
                [&](const auto& row) { return row.strPatternId == fields[2]; });
            if (pattern == owners->second.end()) { m_strStatus = "Gaze window pattern is missing"; return false; }
            const auto window = std::find_if(pattern->LogicWindows.begin(), pattern->LogicWindows.end(),
                [&](const auto& row) { return row.strWindowId == fields[3]; });
            if (window == pattern->LogicWindows.end() || window->eKind != BOSS_PATTERN_LOGIC_KIND::GAZE_REAL_BOSS ||
                !window->bInsideIsFail || window->bGazeDuringWindow)
            { m_strStatus = "Gaze window policy requires one facing-is-fail gaze"; return false; }
            window->bGazeDuringWindow = true;
        }
		else if (!fields.empty() && "PATTERNLOGICCANCEL" == fields[0])
		{
			if (fields.size() != 4u || !IsStableId(fields[1]) || !IsStableId(fields[2]) || !IsStableId(fields[3]))
			{ m_strStatus = "Boss logic cancellation row is invalid"; return false; }
			const auto owners = m_BossPatterns.find(std::string(fields[1]));
			if (owners == m_BossPatterns.end())
			{ m_strStatus = "Boss logic cancellation encounter is missing"; return false; }
			const auto owner = std::find_if(owners->second.begin(), owners->second.end(),
				[&](const auto& value) { return value.strPatternId == fields[2]; });
			if (owner == owners->second.end())
			{ m_strStatus = "Boss logic cancellation pattern is missing"; return false; }
			const auto window = std::find_if(owner->LogicWindows.begin(), owner->LogicWindows.end(),
				[&](const auto& value) { return value.strWindowId == fields[3]; });
			if (window == owner->LogicWindows.end() || window->bCancelAtEnd)
			{ m_strStatus = "Boss logic cancellation window is missing or duplicated"; return false; }
			window->bCancelAtEnd = true;
		}
        else if (!fields.empty() && "PATTERNLOGICWORLDOWNER" == fields[0])
        {
            if (fields.size() != 5u || !IsStableId(fields[1]) || !IsStableId(fields[2]) || !IsStableId(fields[3]) || !IsStableId(fields[4]))
            { m_strStatus = "Contact WORLD owner row is invalid"; return false; }
            const auto owners = m_BossPatterns.find(std::string(fields[1]));
            if (owners == m_BossPatterns.end()) { m_strStatus = "Contact WORLD owner encounter is missing"; return false; }
            const auto pattern = std::find_if(owners->second.begin(), owners->second.end(), [&](const auto& row) { return row.strPatternId == fields[2]; });
            if (pattern == owners->second.end()) { m_strStatus = "Contact WORLD owner pattern is missing"; return false; }
            const auto window = std::find_if(pattern->LogicWindows.begin(), pattern->LogicWindows.end(), [&](const auto& row) { return row.strWindowId == fields[3]; });
            if (window == pattern->LogicWindows.end() || !window->strOwnerWorldOccurrenceId.empty() ||
                (window->eKind != BOSS_PATTERN_LOGIC_KIND::ENTER_AREA && window->eKind != BOSS_PATTERN_LOGIC_KIND::AREA_OVERLAP))
            { m_strStatus = "Contact WORLD owner requires one contact window"; return false; }
            window->strOwnerWorldOccurrenceId = fields[4];
        }
        else if (!fields.empty() && "PATTERNLOGICTICK" == fields[0])
        {
            std::uint32_t interval = 0u;
            if (fields.size() != 5u || !IsStableId(fields[1]) || !IsStableId(fields[2]) ||
                !IsStableId(fields[3]) || !ParseNumber(fields[4], interval) || !interval || interval > 600000u)
            { m_strStatus = "Collider tick interval is invalid"; return false; }
            const auto owners = m_BossPatterns.find(std::string(fields[1]));
            if (owners == m_BossPatterns.end()) { m_strStatus = "Collider tick encounter is missing"; return false; }
            const auto pattern = std::find_if(owners->second.begin(), owners->second.end(),
                [&](const auto& row) { return row.strPatternId == fields[2]; });
            if (pattern == owners->second.end()) { m_strStatus = "Collider tick pattern is missing"; return false; }
            const auto window = std::find_if(pattern->LogicWindows.begin(), pattern->LogicWindows.end(),
                [&](const auto& row) { return row.strWindowId == fields[3]; });
            if (window == pattern->LogicWindows.end() || (window->eKind != BOSS_PATTERN_LOGIC_KIND::ENTER_AREA &&
                window->eKind != BOSS_PATTERN_LOGIC_KIND::AREA_OVERLAP) || window->iRepeatIntervalMs || window->bRearmOnExit || window->bRepeatAfterKnockback)
            { m_strStatus = "Collider tick requires one ENTER_AREA or AREA_OVERLAP repeat policy"; return false; }
            window->iRepeatIntervalMs = interval;
        }
		else if (!fields.empty() && "PATTERNLOGICREARM" == fields[0])
		{
			if (fields.size() != 5u || !IsStableId(fields[1]) || !IsStableId(fields[2]) ||
				!IsStableId(fields[3]) || (fields[4] != "ON_REENTER" && fields[4] != "AFTER_KNOCKBACK"))
			{ m_strStatus = "Boss logic rearm row is invalid"; return false; }
			const auto owners = m_BossPatterns.find(std::string(fields[1]));
			if (owners == m_BossPatterns.end())
			{ m_strStatus = "Boss logic rearm encounter is missing"; return false; }
			const auto pattern = std::find_if(owners->second.begin(), owners->second.end(),
				[&](const auto& row) { return row.strPatternId == fields[2]; });
			if (pattern == owners->second.end())
			{ m_strStatus = "Boss logic rearm pattern is missing"; return false; }
			const auto window = std::find_if(pattern->LogicWindows.begin(), pattern->LogicWindows.end(),
				[&](const auto& row) { return row.strWindowId == fields[3]; });
			if (window == pattern->LogicWindows.end() || window->eKind != BOSS_PATTERN_LOGIC_KIND::ENTER_AREA || window->bRearmOnExit || window->bRepeatAfterKnockback || window->iRepeatIntervalMs)
			{ m_strStatus = "Boss logic rearm needs one ENTER_AREA owner"; return false; }
			window->bRearmOnExit = fields[4] == "ON_REENTER";
			window->bRepeatAfterKnockback = fields[4] == "AFTER_KNOCKBACK";
		}
        else if (!fields.empty() && "PATTERNLOGICCHARGE" == fields[0])
        {
            float distance = 0.f, yawOffset = 0.f;
            if ((fields.size() != 5u && fields.size() != 6u) || !IsStableId(fields[1]) || !IsStableId(fields[2]) ||
                !IsStableId(fields[3]) || !ParseNumber(fields[4], distance) || !std::isfinite(distance) ||
                distance <= 0.f || distance > 1000.f || (fields.size() == 6u &&
                (!ParseNumber(fields[5], yawOffset) || !std::isfinite(yawOffset) || std::abs(yawOffset) > 360.f)))
            { m_strStatus = "Boss charge row is invalid"; return false; }
            const auto owners = m_BossPatterns.find(std::string(fields[1]));
            if (owners == m_BossPatterns.end()) { m_strStatus = "Boss charge encounter is missing"; return false; }
            const auto pattern = std::find_if(owners->second.begin(), owners->second.end(),
                [&](const auto& row) { return row.strPatternId == fields[2]; });
            if (pattern == owners->second.end() || pattern->BossMotion)
            { m_strStatus = "Boss charge pattern is missing or owns absolute motion"; return false; }
            const auto window = std::find_if(pattern->LogicWindows.begin(), pattern->LogicWindows.end(),
                [&](const auto& row) { return row.strWindowId == fields[3]; });
            if (window == pattern->LogicWindows.end() || window->eKind != BOSS_PATTERN_LOGIC_KIND::ENTER_AREA ||
                window->fBossChargeDistanceM != 0.f)
            { m_strStatus = "Boss charge needs a unique ENTER_AREA window"; return false; }
            for (const auto& other : pattern->LogicWindows)
                if (other.fBossChargeDistanceM > 0.f && other.iStartMs < window->iStartMs + window->iDurationMs &&
                    window->iStartMs < other.iStartMs + other.iDurationMs)
                { m_strStatus = "Boss charge windows overlap"; return false; }
            window->fBossChargeDistanceM = distance;
            window->fChargeYawOffsetDegrees = yawOffset;
        }
        else if (!fields.empty() && "PATTERNLOGICHOLD" == fields[0])
        {
            if (fields.size() != 5u || !IsStableId(fields[1]) || !IsStableId(fields[2]) ||
                !IsStableId(fields[3]) || !IsStableId(fields[4]))
            { m_strStatus = "Attachment hold reference row is invalid"; return false; }
            const auto owners = m_BossPatterns.find(std::string(fields[1]));
            if (owners == m_BossPatterns.end()) { m_strStatus = "Attachment hold encounter is missing"; return false; }
            const auto pattern = std::find_if(owners->second.begin(), owners->second.end(),
                [&](const auto& row) { return row.strPatternId == fields[2]; });
            if (pattern == owners->second.end()) { m_strStatus = "Attachment hold pattern is missing"; return false; }
            const auto trigger = std::find_if(pattern->LogicWindows.begin(), pattern->LogicWindows.end(),
                [&](const auto& row) { return row.strWindowId == fields[3]; });
            const auto hold = std::find_if(pattern->LogicWindows.begin(), pattern->LogicWindows.end(),
                [&](const auto& row) { return row.strWindowId == fields[4]; });
            if (trigger == pattern->LogicWindows.end() || hold == pattern->LogicWindows.end() ||
                trigger->eKind != BOSS_PATTERN_LOGIC_KIND::ENTER_AREA || hold->eKind != BOSS_PATTERN_LOGIC_KIND::ATTACHMENT_HOLD ||
                !trigger->strHoldLogicOccurrenceId.empty() || hold->iStartMs > trigger->iStartMs ||
                std::uint64_t(hold->iStartMs) + hold->iDurationMs < std::uint64_t(trigger->iStartMs) + trigger->iDurationMs)
            { m_strStatus = "Attachment hold must uniquely reference a containing Hold window in the same pattern"; return false; }
            trigger->strHoldLogicOccurrenceId = fields[4];
        }
		else if (!fields.empty() && "PATTERNLOGICREGION" == fields[0])
		{
			BOSS_LOGIC_REGION region{};
			std::uint32_t ordinal = 0u;
			if ((19u != fields.size() && 20u != fields.size() && 21u != fields.size() && 22u != fields.size()) || !IsStableId(fields[1]) || !IsStableId(fields[2]) ||
				!IsStableId(fields[3]) || !ParseNumber(fields[4], ordinal) || ordinal >= 64u || !IsStableId(fields[5]))
			{ m_strStatus = "Boss Logic region identity is invalid"; return false; }
			region.strRegionId = std::string(fields[5]);
			if ("BOSS_CURRENT" == fields[6]) region.eAnchor = BOSS_LOGIC_REGION_ANCHOR::BOSS_CURRENT;
			else if ("BOSS_START" == fields[6]) region.eAnchor = BOSS_LOGIC_REGION_ANCHOR::BOSS_START;
			else if ("BOSS_SPAWN" == fields[6]) region.eAnchor = BOSS_LOGIC_REGION_ANCHOR::BOSS_SPAWN;
			else if ("WORLD" != fields[6]) { m_strStatus = "Boss Logic region anchor is invalid"; return false; }
			region.bReverseSector = "REVERSE_SECTOR" == fields[7];
			region.bSector = "SECTOR" == fields[7] || region.bReverseSector;
			region.bCylinder = "CYLINDER" == fields[7];
			region.bCircle = "CIRCLE" == fields[7] || region.bCylinder;
			if (!region.bSector && !region.bCircle && "BOX" != fields[7]) { m_strStatus = "Boss Logic region shape is invalid"; return false; }
			float* numbers[] = { &region.fCenterX, &region.fCenterY, &region.fCenterZ, &region.fYawDegrees,
				&region.fHalfX, &region.fHalfY, &region.fHalfZ, &region.fRadiusM, &region.fHalfAngleDegrees };
			for (std::size_t index = 0u; index < 9u; ++index)
				if (!ParseNumber(fields[index + 8u], *numbers[index]) || !std::isfinite(*numbers[index]))
				{ m_strStatus = "Boss Logic region geometry is invalid"; return false; }
			if (region.fHalfX <= 0.f || region.fHalfY <= 0.f || region.fHalfZ <= 0.f ||
				region.fRadiusM <= 0.f || (region.bReverseSector ? region.fHalfAngleDegrees < 0.f : region.fHalfAngleDegrees <= 0.f) || region.fHalfAngleDegrees > 180.f)
			{ m_strStatus = "Boss Logic region dimensions are invalid"; return false; }
			if (fields.size() >= 21u && (!region.bSector ||
				!ParseNumber(fields[19], region.fRadiusXM) || !ParseNumber(fields[20], region.fRadiusZM) ||
				!std::isfinite(region.fRadiusXM) || !std::isfinite(region.fRadiusZM) ||
				region.fRadiusXM <= 0.f || region.fRadiusZM <= 0.f))
			{ m_strStatus = "Boss Logic sector axes are invalid"; return false; }
			if ((fields.size() == 20u || fields.size() == 22u) &&
				(!ParseNumber(fields.back(), region.fInnerRadiusM) || !std::isfinite(region.fInnerRadiusM) ||
				 region.fInnerRadiusM < 0.f || region.fInnerRadiusM >= region.fRadiusM ||
				 (region.fInnerRadiusM > 0.f && (region.bCylinder || (!region.bCircle && !region.bSector) || region.bReverseSector ||
				  (region.fRadiusXM > 0.f && (std::abs(region.fRadiusXM - region.fRadiusM) > .0001f || std::abs(region.fRadiusZM - region.fRadiusM) > .0001f))))))
			{ m_strStatus = "Boss Logic inner radius requires an exact circular ring or annular sector"; return false; }
			if ("NONE" != fields[17])
			{
				std::vector<LostArk::Shared::MECHANIC_CARD_SYMBOL> symbols;
				if (!ParseMechanicCardSymbols(fields[17], symbols) || symbols.size() != 1u)
				{ m_strStatus = "Boss Logic region card suit is invalid"; return false; }
				region.eCardSymbol = symbols.front();
			}
			if ("RED" == fields[18]) region.eCardColor = LostArk::Shared::MECHANIC_CARD_COLOR::RED;
			else if ("BLACK" == fields[18]) region.eCardColor = LostArk::Shared::MECHANIC_CARD_COLOR::BLACK;
			else if ("NONE" != fields[18]) { m_strStatus = "Boss Logic region card color is invalid"; return false; }
			if (("NONE" == fields[17]) != ("NONE" == fields[18]))
			{ m_strStatus = "Boss Logic region card pair is incomplete"; return false; }
			const auto owners = m_BossPatterns.find(std::string(fields[1]));
			if (owners == m_BossPatterns.end()) { m_strStatus = "Boss Logic region encounter is missing"; return false; }
			const auto owner = std::find_if(owners->second.begin(), owners->second.end(),
				[&](const BOSS_PATTERN_DEFINITION& value) { return value.strPatternId == fields[2]; });
			if (owner == owners->second.end()) { m_strStatus = "Boss Logic region pattern is missing"; return false; }
			const auto window = std::find_if(owner->LogicWindows.begin(), owner->LogicWindows.end(),
				[&](const BOSS_PATTERN_LOGIC_WINDOW& value) { return value.strWindowId == fields[3]; });
			if (window == owner->LogicWindows.end() || window->CardRegions.size() != ordinal ||
				std::any_of(window->CardRegions.begin(), window->CardRegions.end(),
					[&](const BOSS_LOGIC_REGION& value) { return value.strRegionId == region.strRegionId; }))
			{ m_strStatus = "Boss Logic region window/order is invalid"; return false; }
			if (window->eKind == BOSS_PATTERN_LOGIC_KIND::STAGGER_WINDOW &&
				(!region.bSector || region.bReverseSector || region.eAnchor != BOSS_LOGIC_REGION_ANCHOR::BOSS_CURRENT ||
				 region.eCardSymbol != LostArk::Shared::MECHANIC_CARD_SYMBOL::NONE))
			{ m_strStatus = "Shield reflection requires boss-pivot sector regions without card mapping"; return false; }
			region.iMotionStartMs = window->iStartMs;
			region.iMotionDurationMs = window->iDurationMs;
			window->CardRegions.push_back(std::move(region));
		}
		else if (!fields.empty() && "PATTERNLOGICREGIONCAPTURE" == fields[0])
		{
			std::uint32_t ordinal = 0u, captureStartMs = 0u;
			if (fields.size() != 6u || !IsStableId(fields[1]) || !IsStableId(fields[2]) ||
				!IsStableId(fields[3]) || !ParseNumber(fields[4], ordinal) || !ParseNumber(fields[5], captureStartMs))
			{ m_strStatus = "Collider anchor capture identity or time is invalid"; return false; }
			const auto owners = m_BossPatterns.find(std::string(fields[1]));
			if (owners == m_BossPatterns.end()) { m_strStatus = "Collider capture encounter is missing"; return false; }
			const auto pattern = std::find_if(owners->second.begin(), owners->second.end(),
				[&](const auto& row) { return row.strPatternId == fields[2]; });
			if (pattern == owners->second.end()) { m_strStatus = "Collider capture pattern is missing"; return false; }
			const auto window = std::find_if(pattern->LogicWindows.begin(), pattern->LogicWindows.end(),
				[&](const auto& row) { return row.strWindowId == fields[3]; });
			if (window == pattern->LogicWindows.end() || ordinal >= window->CardRegions.size() || captureStartMs > window->iStartMs)
			{ m_strStatus = "Collider capture must precede its owning window"; return false; }
			auto& region = window->CardRegions[ordinal];
			if (region.eAnchor != BOSS_LOGIC_REGION_ANCHOR::BOSS_START || region.iAnchorCaptureStartMs != 0xffffffffu)
			{ m_strStatus = "Collider capture requires one fixed boss anchor"; return false; }
			region.iAnchorCaptureStartMs = captureStartMs;
		}
        else if (!fields.empty() && "PATTERNLOGICREGIONMOTION" == fields[0])
        {
            std::uint32_t ordinal = 0u;
            if (fields.size() != 11u || !IsStableId(fields[1]) || !IsStableId(fields[2]) ||
                !IsStableId(fields[3]) || !ParseNumber(fields[4], ordinal))
            { m_strStatus = "Collider motion identity is invalid"; return false; }
            const auto owners = m_BossPatterns.find(std::string(fields[1]));
            if (owners == m_BossPatterns.end()) { m_strStatus = "Collider motion encounter is missing"; return false; }
            const auto pattern = std::find_if(owners->second.begin(), owners->second.end(),
                [&](const auto& row) { return row.strPatternId == fields[2]; });
            if (pattern == owners->second.end()) { m_strStatus = "Collider motion pattern is missing"; return false; }
            const auto window = std::find_if(pattern->LogicWindows.begin(), pattern->LogicWindows.end(),
                [&](const auto& row) { return row.strWindowId == fields[3]; });
            if (window == pattern->LogicWindows.end() || ordinal >= window->CardRegions.size() ||
                (window->eKind != BOSS_PATTERN_LOGIC_KIND::ENTER_AREA && window->eKind != BOSS_PATTERN_LOGIC_KIND::AREA_OVERLAP &&
                 window->eKind != BOSS_PATTERN_LOGIC_KIND::OBJECT_CONTACT && window->eKind != BOSS_PATTERN_LOGIC_KIND::OBJECT_OVERLAP))
            { m_strStatus = "Collider motion requires an overlap region"; return false; }
            auto& region = window->CardRegions[ordinal];
            if (region.bLinearMotion) { m_strStatus = "Collider motion is duplicated"; return false; }
            for (std::size_t axis = 0; axis < 3u; ++axis)
                if (!ParseNumber(fields[5u + axis], region.EndPositionOffset[axis]) ||
                    !std::isfinite(region.EndPositionOffset[axis]) || std::abs(region.EndPositionOffset[axis]) > 200000.f ||
                    !ParseNumber(fields[8u + axis], region.EndScale[axis]) ||
                    !std::isfinite(region.EndScale[axis]) || region.EndScale[axis] <= 0.f || region.EndScale[axis] > 10000000.f)
                { m_strStatus = "Collider end position or size is invalid"; return false; }
            if ((region.bCircle || region.fInnerRadiusM > 0.f) && std::abs(region.EndScale[0] - region.EndScale[2]) > .0001f)
            { m_strStatus = "Hollow Collider motion needs equal X/Z scale"; return false; }
            region.bLinearMotion = true;
            region.iMotionStartMs = window->iStartMs;
            region.iMotionDurationMs = window->iDurationMs;
        }
		else if (!fields.empty() && ("PATTERNLOGICREGIONWORLD" == fields[0] || "PATTERNLOGICREGIONWORLDKEY" == fields[0]))
		{
			const bool isKey = "PATTERNLOGICREGIONWORLDKEY" == fields[0];
			if (isKey ? (fields.size() != 16u && fields.size() != 19u) : fields.size() != 17u)
			{ m_strStatus = "Boss Logic WORLD row width is invalid"; return false; }
			const auto owners = m_BossPatterns.find(std::string(fields[1]));
			if (owners == m_BossPatterns.end()) { m_strStatus = "Boss Logic WORLD encounter is missing"; return false; }
			const auto owner = std::find_if(owners->second.begin(),owners->second.end(),
				[&](const BOSS_PATTERN_DEFINITION& row) { return row.strPatternId == fields[2]; });
			if (owner == owners->second.end()) { m_strStatus = "Boss Logic WORLD pattern is missing"; return false; }
			const auto window = std::find_if(owner->LogicWindows.begin(),owner->LogicWindows.end(),
				[&](const BOSS_PATTERN_LOGIC_WINDOW& row) { return row.strWindowId == fields[3]; });
			if (window == owner->LogicWindows.end() || (BOSS_PATTERN_LOGIC_KIND::ENTER_AREA != window->eKind && BOSS_PATTERN_LOGIC_KIND::AREA_OVERLAP != window->eKind &&
				BOSS_PATTERN_LOGIC_KIND::OBJECT_OVERLAP != window->eKind && BOSS_PATTERN_LOGIC_KIND::OBJECT_CONTACT != window->eKind))
			{ m_strStatus = "Boss Logic WORLD Trigger window is missing"; return false; }
			const auto region = std::find_if(window->CardRegions.begin(),window->CardRegions.end(),
				[&](const BOSS_LOGIC_REGION& row) { return row.strRegionId == fields[4]; });
			if (region == window->CardRegions.end() || (BOSS_LOGIC_REGION_ANCHOR::BOSS_CURRENT == region->eAnchor &&
				window->eKind != BOSS_PATTERN_LOGIC_KIND::OBJECT_CONTACT && window->eKind != BOSS_PATTERN_LOGIC_KIND::ENTER_AREA && window->eKind != BOSS_PATTERN_LOGIC_KIND::AREA_OVERLAP))
			{ m_strStatus = "Boss Logic WORLD region is missing or uses a bone track outside Object Contact / Enter Area"; return false; }
			auto& track = region->WorldTrack;
			if (!isKey)
			{
				std::uint32_t smooth = 0u;
				if (track.bEnabled || !ParseNumber(fields[5],track.iStartMs) ||
					!ParseNumber(fields[6],track.iStartDelayMs) || !ParseNumber(fields[7],track.iDurationMs) ||
					track.iDurationMs == 0u || !ParseNumber(fields[8],track.fPlaybackSpeed) ||
					!std::isfinite(track.fPlaybackSpeed) || track.fPlaybackSpeed <= 0.f ||
					!ParseNumber(fields[9],smooth) || smooth > 1u)
				{ m_strStatus = "Boss Logic WORLD clock is invalid"; return false; }
				float* numbers[] = {&track.fBaselineX,&track.fBaselineY,&track.fBaselineZ,&track.fBaselineYawDegrees,
					&track.fBaselineScaleX,&track.fBaselineScaleY,&track.fBaselineScaleZ};
				for (std::size_t index = 0u; index < 7u; ++index)
					if (!ParseNumber(fields[index+10u],*numbers[index]) || !std::isfinite(*numbers[index]))
					{ m_strStatus = "Boss Logic WORLD baseline is invalid"; return false; }
				if (track.fBaselineScaleX <= 0.f || track.fBaselineScaleY <= 0.f || track.fBaselineScaleZ <= 0.f ||
					((region->bCircle || region->bSector) && std::fabs(track.fBaselineScaleX-track.fBaselineScaleZ) > 0.0001f))
				{ m_strStatus = "Boss Logic WORLD baseline scale is invalid"; return false; }
				track.bEnabled = true; track.bSmoothStep = 1u == smooth;
			}
			else
			{
				BOSS_LOGIC_WORLD_TRANSFORM_KEY key{};
				std::uint32_t ordinal = 0u, visible = 0u;
				if (!track.bEnabled || !ParseNumber(fields[5],ordinal) || ordinal != track.Keys.size() || ordinal >= 4096u ||
					!ParseNumber(fields[6],key.iTimeMs) || key.iTimeMs > track.iDurationMs ||
					(!track.Keys.empty() && track.Keys.back().iTimeMs >= key.iTimeMs) ||
					!ParseNumber(fields[15],visible) || visible > 1u)
				{ m_strStatus = "Boss Logic WORLD key clock/order is invalid"; return false; }
				float* numbers[] = {&key.fOffsetX,&key.fOffsetY,&key.fOffsetZ,&key.fRotationY,&key.fRotationW,
					&key.fScaleX,&key.fScaleY,&key.fScaleZ};
				for (std::size_t index = 0u; index < 8u; ++index)
					if (!ParseNumber(fields[index+7u],*numbers[index]) || !std::isfinite(*numbers[index]))
					{ m_strStatus = "Boss Logic WORLD key transform is invalid"; return false; }
				if (key.fScaleX < 0.f || key.fScaleY < 0.f || key.fScaleZ < 0.f ||
					((region->bCircle || region->bSector) && std::fabs(key.fScaleX-key.fScaleZ) > 0.0001f) ||
					std::fabs(key.fRotationY*key.fRotationY+key.fRotationW*key.fRotationW-1.f) > 0.001f)
				{ m_strStatus = "Boss Logic WORLD key scale/quaternion is invalid"; return false; }
				key.bHasGripPosition = fields.size() == 19u;
				if (key.bHasGripPosition)
					for (size_t axis = 0; axis < 3u; ++axis)
						if (!ParseNumber(fields[16u + axis], key.GripPosition[axis]) || !std::isfinite(key.GripPosition[axis]) || std::abs(key.GripPosition[axis]) > 100000.f)
						{ m_strStatus = "Hook grip position must be finite world metres."; return false; }
				if (!track.Keys.empty() && track.Keys.front().bHasGripPosition != key.bHasGripPosition)
				{ m_strStatus = "Hook grip positions must be present on every WORLD key."; return false; }
				key.bVisible = 1u == visible; track.Keys.push_back(key);
			}
		}
		else if (!fields.empty() && "PATTERNLOGICCHAIN" == fields[0])
		{
			std::uint32_t count = 0u;
			if (fields.size() < 6u || fields.size() > 21u || !ParseNumber(fields[4], count) || count < 1u || count > fields.size() - 5u)
			{ m_strStatus = "Pattern completion chain count is invalid"; return false; }
			const auto owners = m_BossPatterns.find(std::string(fields[1]));
			if (owners == m_BossPatterns.end()) { m_strStatus = "Pattern chain encounter missing"; return false; }
			const auto pattern = std::find_if(owners->second.begin(), owners->second.end(), [&](const auto& row) { return row.strPatternId == fields[2]; });
			if (pattern == owners->second.end()) { m_strStatus = "Pattern chain owner missing"; return false; }
			const auto window = std::find_if(pattern->LogicWindows.begin(), pattern->LogicWindows.end(), [&](const auto& row) { return row.strWindowId == fields[3]; });
			if (window == pattern->LogicWindows.end() || window->eKind != BOSS_PATTERN_LOGIC_KIND::PATTERN_COMPLETION_COUNT || !window->PatternIds.empty())
			{ m_strStatus = "Pattern chain window missing or duplicated"; return false; }
			for (std::size_t i = 5u; i < fields.size(); ++i)
			{
				if (!IsStableId(fields[i]) || fields[i] == pattern->strPatternId || std::find(window->PatternIds.begin(), window->PatternIds.end(), fields[i]) != window->PatternIds.end())
				{ m_strStatus = "Pattern chain candidate is invalid or duplicated"; return false; }
				window->PatternIds.emplace_back(fields[i]);
			}
			window->iCompletionCount = count;
		}
		else if (!fields.empty() && "PATTERNLOGICHANDKEY" == fields[0])
		{
			if (fields.size() != 9u)
			{ m_strStatus = "Capture hand key field count is invalid"; return false; }
			const auto owners = m_BossPatterns.find(std::string(fields[1]));
			if (owners == m_BossPatterns.end())
			{ m_strStatus = "Capture hand encounter missing"; return false; }
			const auto pattern = std::find_if(owners->second.begin(), owners->second.end(),
				[&](const auto& row) { return row.strPatternId == fields[2]; });
			if (pattern == owners->second.end())
			{ m_strStatus = "Capture hand pattern missing"; return false; }
			const auto window = std::find_if(pattern->LogicWindows.begin(), pattern->LogicWindows.end(),
				[&](const auto& row) { return row.strWindowId == fields[3]; });
			if (window == pattern->LogicWindows.end() || window->eKind != BOSS_PATTERN_LOGIC_KIND::ENTER_AREA)
			{ m_strStatus = "Capture hand ENTER_AREA missing"; return false; }
			BOSS_CAPTURE_HAND_KEY key{};
			std::uint32_t ordinal = 0u;
			if (!ParseNumber(fields[4], ordinal) || ordinal != window->CaptureHandKeys.size() || ordinal >= 4096u ||
				!ParseNumber(fields[5], key.iTimeMs) || key.iTimeMs > 600000u ||
				(!window->CaptureHandKeys.empty() && key.iTimeMs <= window->CaptureHandKeys.back().iTimeMs))
			{ m_strStatus = "Capture hand key order/time invalid"; return false; }
			for (std::size_t axis = 0u; axis < 3u; ++axis)
				if (!ParseNumber(fields[6u + axis], key.LocalPosition[axis]) ||
					!std::isfinite(key.LocalPosition[axis]) || std::abs(key.LocalPosition[axis]) > 100000.f)
				{ m_strStatus = "Capture hand boss-local position invalid"; return false; }
			window->CaptureHandKeys.push_back(key);
		}
		else if (!fields.empty() && "PATTERNLOGICOUTCOME" == fields[0])
		{
			BOSS_PATTERN_LOGIC_RESULT result{};
			std::uint32_t ordinal = 0u;
			if ((10u != fields.size() && 11u != fields.size() && 12u != fields.size() && 14u != fields.size()) || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) || !IsStableId(fields[3]) ||
				("SUCCESS" != fields[4] && "FAIL" != fields[4] && "TIMEOUT" != fields[4]) ||
				!ParseNumber(fields[5], ordinal) || ordinal > 3u ||
				!ParseBossPatternLogicResultKind(fields[6], result.eKind) ||
				BOSS_PATTERN_LOGIC_RESULT_KIND::NONE == result.eKind ||
				!ParseNumber(fields[7], result.iPercent) || result.iPercent > 100u ||
				(result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::MAX_HP_PERCENT_DAMAGE && result.iPercent == 0u) ||
				!ParseNumber(fields[8], result.iDurationMs) ||
				("-" != fields[9] && !IsStableId(fields[9])))
			{
				m_strStatus = "Boss pattern logic outcome row is invalid";
				return false;
			}
            if (result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::CAPTURE_PLAYER)
            {
                if (fields.size() != 14u || fields[10] != "BOSS_LEFT_HAND" || result.iPercent || result.iDurationMs ||
                    !ParseNumber(fields[11], result.GripLocalOffset[0]) || !ParseNumber(fields[12], result.GripLocalOffset[1]) ||
                    !ParseNumber(fields[13], result.GripLocalOffset[2]) ||
                    std::any_of(result.GripLocalOffset.begin(), result.GripLocalOffset.end(), [](const float value) {
                        return !std::isfinite(value) || std::abs(value) > 10.f;
                    }))
                { m_strStatus = "Capture result requires BOSS_LEFT_HAND and bounded forward/up/right grip"; return false; }
                result.eAttachmentSlot = LostArk::Shared::PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND;
            }
            else if (fields.size() == 14u)
            { m_strStatus = "Only capture results carry an attachment slot and grip"; return false; }
            if (result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::FIXED_DAMAGE)
            {
                if (fields.size() != 11u || result.iPercent || result.iDurationMs || fields[9] != "-" ||
                    !ParseNumber(fields[10], result.iDamageAmount) || !result.iDamageAmount || result.iDamageAmount > 1000000000u)
                { m_strStatus = "Fixed damage requires 1..1000000000 HP and no percent/duration/target"; return false; }
            }
            else if (result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::PLAYER_INVULNERABILITY)
            {
                if (fields.size() != 10u || result.iPercent || !result.iDurationMs || result.iDurationMs > 600000u || fields[9] != "-")
                { m_strStatus = "Player invulnerability requires only a bounded duration"; return false; }
            }
            else if (result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::FEAR)
            {
                if (fields.size() != 11u || !IsStableId(fields[10]) || result.iDurationMs == 0u ||
                    result.iDurationMs > 600000u || result.iPercent != 0u)
                { m_strStatus = "Fear outcome needs a bounded duration and stable presentation ID"; return false; }
                result.strFearPresentationId = fields[10];
            }
            else if (result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::MARIO_ENTER && fields.size() == 11u)
            {
                // 선택적 11번째 필드는 저작 Mario stage다. 10필드 행은 현재 room의 stage counter를 사용한다.
                std::uint32_t stage = 0u;
                if (!ParseNumber(fields[10], stage) || stage < 1u || stage > 4u || result.iPercent || result.iDurationMs)
                { m_strStatus = "Mario entry stage must be 1..4 without percent or duration"; return false; }
                result.iMarioEntryStage = static_cast<std::uint8_t>(stage);
            }
            else if (fields.size() == 11u)
            { m_strStatus = "Only fear carries a presentation ID and only Mario entry carries a stage"; return false; }
			result.strPatternId = "-" == fields[9] ? "" : std::string(fields[9]);
			if (12u == fields.size())
			{
				result.strTargetWorldInstanceId = "-" == fields[10] ? "" : std::string(fields[10]);
				result.strMotionInstanceId = "-" == fields[11] ? "" : std::string(fields[11]);
			}
			const bool worldMotion = BOSS_PATTERN_LOGIC_RESULT_KIND::PLAY_WORLD_OBJECT_MOTION == result.eKind;
			if ((worldMotion && (!IsStableId(result.strTargetWorldInstanceId) || !IsStableId(result.strMotionInstanceId) ||
				result.iPercent != 0u || result.iDurationMs != 0u || !result.strPatternId.empty())) ||
				(!worldMotion && (!result.strTargetWorldInstanceId.empty() || !result.strMotionInstanceId.empty())))
			{
				m_strStatus = "Boss pattern world-object motion outcome IDs are invalid";
				return false;
			}
			if (result.eKind != BOSS_PATTERN_LOGIC_RESULT_KIND::MARIO_ENTER &&
				(BOSS_PATTERN_LOGIC_RESULT_KIND::FOLLOWUP_PATTERN == result.eKind) != !result.strPatternId.empty())
			{
				m_strStatus = "Boss pattern logic outcome follow-up target is inconsistent";
				return false;
			}
			const auto owners = m_BossPatterns.find(std::string(fields[1]));
			if (m_BossPatterns.end() == owners)
			{
				m_strStatus = "Boss pattern logic outcome encounter is missing";
				return false;
			}
			const auto owner = std::find_if(owners->second.begin(), owners->second.end(),
				[&fields](const BOSS_PATTERN_DEFINITION& pattern)
				{ return pattern.strPatternId == fields[2]; });
			if (owners->second.end() == owner)
			{
				m_strStatus = "Boss pattern logic outcome has no pattern owner";
				return false;
			}
			const auto window = std::find_if(owner->LogicWindows.begin(), owner->LogicWindows.end(),
				[&fields](const BOSS_PATTERN_LOGIC_WINDOW& candidate)
				{ return candidate.strWindowId == fields[3]; });
			if (owner->LogicWindows.end() == window)
			{
				m_strStatus = "Boss pattern logic outcome has no window owner";
				return false;
			}
            if (window->eKind == BOSS_PATTERN_LOGIC_KIND::ATTACHMENT_HOLD ||
                (result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::CAPTURE_PLAYER &&
                 (window->eKind != BOSS_PATTERN_LOGIC_KIND::ENTER_AREA || fields[4] != "SUCCESS")))
            { m_strStatus = "Capture belongs to ENTER_AREA Success; Hold has no outcomes"; return false; }
			if (window->eKind == BOSS_PATTERN_LOGIC_KIND::OBJECT_OVERLAP &&
				(!worldMotion || result.strTargetWorldInstanceId != window->strTargetWorldInstanceId))
			{ m_strStatus = "OBJECT_OVERLAP Result must apply a motion to the same target World Object"; return false; }
			if ((BOSS_PATTERN_LOGIC_RESULT_KIND::GRAB_TO_WORLD_OBJECT == result.eKind) &&
				(window->eKind != BOSS_PATTERN_LOGIC_KIND::ENTER_AREA || fields[4] != "SUCCESS" ||
					result.iPercent || result.iDurationMs))
			{ m_strStatus = "A world-object grab only answers an ENTER_AREA success and carries no value"; return false; }
			const bool contactResult = result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::PLAY_CONTACT_WORLD_OBJECT_MOTION ||
				result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::COMPLETE_LOGIC_WINDOW;
			if ((contactResult && (window->eKind != BOSS_PATTERN_LOGIC_KIND::OBJECT_CONTACT || fields[4] != "SUCCESS" ||
				result.iPercent || result.iDurationMs)) || (window->eKind == BOSS_PATTERN_LOGIC_KIND::OBJECT_CONTACT && !contactResult))
			{ m_strStatus = "Object contact only accepts contact motions and completion signals in Success"; return false; }
			std::vector<BOSS_PATTERN_LOGIC_RESULT>& slot =
				"SUCCESS" == fields[4] ? window->OnSuccess :
				("FAIL" == fields[4] ? window->OnFail : window->OnTimeout);
			if (slot.size() != ordinal || slot.size() >= 4u)
			{
				m_strStatus = "Boss pattern logic outcome ordinal is out of order or over capacity";
				return false;
			}
			slot.push_back(std::move(result));
		}
		else if (!fields.empty() && "PATTERNLOGICPUSH" == fields[0])
		{
			std::uint32_t ordinal = 0u, pushMs = 0u;
			float rangeM = 0.f, heightM = 0.f;
			if (fields.size() == 14u && (!ParseNumber(fields[13], heightM) || !std::isfinite(heightM) || heightM < 0.f || heightM > 100.f))
			{ m_strStatus = "Ballistic height is invalid"; return false; }
			const bool ballistic = fields.size() >= 13u && fields[12] == "1";
			if ((fields.size() != 8u && fields.size() != 9u && fields.size() != 11u && fields.size() != 12u && fields.size() != 13u && fields.size() != 14u) || !IsStableId(fields[1]) || !IsStableId(fields[2]) || !IsStableId(fields[3]) ||
				(fields.size() >= 9u && fields[8] != "AWAY_FROM_BOSS" && fields[8] != "BOSS_FORWARD" && fields[8] != "AWAY_FROM_CONTACT") ||
				(fields.size() >= 11u && ((fields[9] != "0" && fields[9] != "1") || (fields[10] != "0" && fields[10] != "1"))) ||
				(fields[4] != "SUCCESS" && fields[4] != "FAIL" && fields[4] != "TIMEOUT") ||
				!ParseNumber(fields[5], ordinal) || ordinal > 3u ||
				!ParseNumber(fields[6], rangeM) || !std::isfinite(rangeM) || rangeM < 0.f || (rangeM == 0.f && heightM == 0.f) || rangeM > (ballistic ? 100.f : 20.f) ||
				!ParseNumber(fields[7], pushMs) || pushMs == 0u || pushMs > (ballistic ? 5000u : 600000u) ||
				(fields.size() >= 13u && fields[12] != "0" && fields[12] != "1") ||
				(heightM > 0.f && !ballistic) || (ballistic && pushMs < 100u))
			{ m_strStatus = "Boss logic push requires paired bounded range and duration"; return false; }
			const auto owners = m_BossPatterns.find(std::string(fields[1]));
			if (owners == m_BossPatterns.end())
			{ m_strStatus = "Boss logic push encounter is missing"; return false; }
			const auto pattern = std::find_if(owners->second.begin(), owners->second.end(),
				[&](const auto& row) { return row.strPatternId == fields[2]; });
			if (pattern == owners->second.end())
			{ m_strStatus = "Boss logic push pattern is missing"; return false; }
			const auto window = std::find_if(pattern->LogicWindows.begin(), pattern->LogicWindows.end(),
				[&](const auto& row) { return row.strWindowId == fields[3]; });
			if (window == pattern->LogicWindows.end())
			{ m_strStatus = "Boss logic push window is missing"; return false; }
			auto& outcomes = fields[4] == "SUCCESS" ? window->OnSuccess :
				(fields[4] == "FAIL" ? window->OnFail : window->OnTimeout);
			if (ordinal >= outcomes.size() || (outcomes[ordinal].eKind != BOSS_PATTERN_LOGIC_RESULT_KIND::MAX_HP_PERCENT_DAMAGE && outcomes[ordinal].eKind != BOSS_PATTERN_LOGIC_RESULT_KIND::FIXED_DAMAGE) ||
				outcomes[ordinal].fPushRangeM != 0.f || outcomes[ordinal].iPushMs != 0u)
			{ m_strStatus = "Boss logic push needs one max-HP damage outcome"; return false; }
			outcomes[ordinal].fPushRangeM = rangeM;
			outcomes[ordinal].iPushMs = pushMs;
			outcomes[ordinal].bPushBallistic = ballistic;
			outcomes[ordinal].fPushHeightM = heightM;
			if (fields.size() >= 9u && fields[8] == "AWAY_FROM_CONTACT")
				outcomes[ordinal].ePushDirection = BOSS_LOGIC_PUSH_DIRECTION::AWAY_FROM_CONTACT;
			if (fields.size() >= 9u && fields[8] == "BOSS_FORWARD")
				outcomes[ordinal].ePushDirection = BOSS_LOGIC_PUSH_DIRECTION::BOSS_FORWARD;
			if (fields.size() >= 11u)
			{
				outcomes[ordinal].bForcePush = fields[9] == "1";
				outcomes[ordinal].bPushCanLeaveArena = fields[10] == "1";
			}
			if (fields.size() >= 12u && (!ParseNumber(fields[11], outcomes[ordinal].fPushYawOffsetDegrees) ||
				!std::isfinite(outcomes[ordinal].fPushYawOffsetDegrees) || std::abs(outcomes[ordinal].fPushYawOffsetDegrees) > 360.f ||
				(outcomes[ordinal].fPushYawOffsetDegrees != 0.f && outcomes[ordinal].ePushDirection != BOSS_LOGIC_PUSH_DIRECTION::BOSS_FORWARD)))
			{ m_strStatus = "Boss logic push yaw offset requires bounded BOSS_FORWARD push"; return false; }
		}
		else if (!fields.empty() && (fields[0] == "PATTERNLOGICCONTACTTARGET" || fields[0] == "PATTERNLOGICCONTACTGROUP" ||
			fields[0] == "PATTERNLOGICCONTACTMOTION" || fields[0] == "PATTERNLOGICSIGNAL"))
		{
			const bool targetRow = fields[0] == "PATTERNLOGICCONTACTTARGET";
			const bool groupRow = fields[0] == "PATTERNLOGICCONTACTGROUP";
			const bool motionRow = fields[0] == "PATTERNLOGICCONTACTMOTION";
			if (fields.size() != (targetRow ? 9u : groupRow ? 6u : 8u) ||
				!IsStableId(fields[1]) || !IsStableId(fields[2]) || !IsStableId(fields[3]))
			{ m_strStatus = "Object contact row width or owner identity is invalid"; return false; }
			const auto owners = m_BossPatterns.find(std::string(fields[1]));
			if (owners == m_BossPatterns.end()) { m_strStatus = "Object contact encounter is missing"; return false; }
			const auto pattern = std::find_if(owners->second.begin(), owners->second.end(),
				[&](const auto& row) { return row.strPatternId == fields[2]; });
			if (pattern == owners->second.end()) { m_strStatus = "Object contact pattern is missing"; return false; }
			const auto window = std::find_if(pattern->LogicWindows.begin(), pattern->LogicWindows.end(),
				[&](const auto& row) { return row.strWindowId == fields[3]; });
			if (window == pattern->LogicWindows.end() || window->eKind != BOSS_PATTERN_LOGIC_KIND::OBJECT_CONTACT)
			{ m_strStatus = "Object contact window is missing or has the wrong kind"; return false; }
			if (targetRow)
			{
				BOSS_LOGIC_CONTACT_TARGET target;
				if (!IsStableId(fields[4]) || !IsStableId(fields[5]) || window->ContactTargets.size() >= 64u ||
					!ParseNumber(fields[6], target.fWorldX) || !std::isfinite(target.fWorldX) || std::fabs(target.fWorldX) > 100000.f ||
					!ParseNumber(fields[7], target.fWorldZ) || !std::isfinite(target.fWorldZ) || std::fabs(target.fWorldZ) > 100000.f ||
					!ParseNumber(fields[8], target.fRadiusM) || !std::isfinite(target.fRadiusM) || target.fRadiusM < .01f || target.fRadiusM > 1000.f ||
					std::any_of(window->ContactTargets.begin(), window->ContactTargets.end(), [&](const auto& row) { return row.strWorldOccurrenceId == fields[4]; }))
				{ m_strStatus = "Object contact target identity or circle is invalid"; return false; }
				target.strWorldOccurrenceId = fields[4]; target.strWorldInstanceId = fields[5];
				window->ContactTargets.push_back(std::move(target));
			}
			else if (groupRow)
			{
				if (window->bHasContactGroup || (fields[4] != "-" && !IsStableId(fields[4])) ||
					!ParseNumber(fields[5], window->iContactPriority) || window->iContactPriority > 1000u)
				{ m_strStatus = "Object contact group or priority is invalid"; return false; }
				window->strContactGroupId = fields[4] == "-" ? "" : std::string(fields[4]);
				window->bHasContactGroup = true;
			}
			else
			{
				std::uint32_t ordinal = 0u;
				if (fields[4] != "SUCCESS" || !ParseNumber(fields[5], ordinal) || ordinal >= window->OnSuccess.size())
				{ m_strStatus = "Object contact result slot or ordinal is invalid"; return false; }
				auto& result = window->OnSuccess[ordinal];
				if (motionRow)
				{
					if (result.eKind != BOSS_PATTERN_LOGIC_RESULT_KIND::PLAY_CONTACT_WORLD_OBJECT_MOTION ||
						!IsStableId(fields[6]) || !IsStableId(fields[7]) || result.ContactMotions.size() >= 64u ||
						std::any_of(result.ContactMotions.begin(), result.ContactMotions.end(), [&](const auto& row) { return row.strTargetWorldOccurrenceId == fields[6]; }))
					{ m_strStatus = "Object contact motion mapping is invalid or duplicated"; return false; }
					result.ContactMotions.push_back({ std::string(fields[6]), std::string(fields[7]) });
				}
				else
				{
					if (result.eKind != BOSS_PATTERN_LOGIC_RESULT_KIND::COMPLETE_LOGIC_WINDOW || !result.strTargetLogicOccurrenceId.empty() ||
						!IsStableId(fields[6]) || (fields[7] != "-" && !IsStableId(fields[7])))
					{ m_strStatus = "Object contact completion signal is invalid or duplicated"; return false; }
					result.strTargetLogicOccurrenceId = fields[6];
					result.strContactTargetWorldOccurrenceId = fields[7] == "-" ? "" : std::string(fields[7]);
				}
			}
		}
		else if (!fields.empty() && "PATTERNCROSSDIRECTION" == fields[0])
		{
			BOSS_PATTERN_MECHANIC_TRIGGER trigger{};
			trigger.eKind = BOSS_PATTERN_MECHANIC_TRIGGER_KIND::CROSS_DIRECTION_CLONES;
			if ((fields.size() != 11u && fields.size() != 12u) || !IsStableId(fields[1]) || !IsStableId(fields[2]) || !IsStableId(fields[3]) ||
				!ParseNumber(fields[4], trigger.iStartMs) || !ParseNumber(fields[5], trigger.iDurationMs) ||
				trigger.iDurationMs == 0u || trigger.iDurationMs > 600000u || !IsStableId(fields[10]))
			{ m_strStatus = "Cross direction row has invalid identity or duration"; return false; }
			trigger.strTriggerId = fields[3]; trigger.strCloneEndStageId = fields[10];
			for (std::size_t index = 6u; index < 10u; ++index)
			{
				if (!IsStableId(fields[index]) || fields[index] == fields[2] ||
					std::find(trigger.DirectionPatternIds.begin(), trigger.DirectionPatternIds.end(), fields[index]) != trigger.DirectionPatternIds.end())
				{ m_strStatus = "Cross direction references must be four distinct child Patterns"; return false; }
				trigger.DirectionPatternIds.emplace_back(fields[index]);
			}
			if (fields.size() == 12u)
			{
				if (!IsStableId(fields[11]) || std::find(trigger.DirectionPatternIds.begin(), trigger.DirectionPatternIds.end(), fields[11]) == trigger.DirectionPatternIds.end())
				{ m_strStatus = "Cross direction real Pattern must be one of its four candidates"; return false; }
				trigger.strRealPatternId = fields[11];
			}
			const auto encounter = m_BossPatterns.find(std::string(fields[1]));
			if (encounter == m_BossPatterns.end()) { m_strStatus = "Cross direction encounter is missing"; return false; }
			const auto parent = std::find_if(encounter->second.begin(), encounter->second.end(), [&](const auto& row) { return row.strPatternId == fields[2]; });
			if (parent == encounter->second.end() || parent->MechanicTriggers.size() >= 64u ||
				std::any_of(parent->MechanicTriggers.begin(), parent->MechanicTriggers.end(), [&](const auto& row) { return row.strTriggerId == trigger.strTriggerId; }))
			{ m_strStatus = "Cross direction parent or occurrence is missing or duplicated"; return false; }
			parent->MechanicTriggers.push_back(std::move(trigger));
		}
		else if (!fields.empty() && "PATTERNMECHANICTRIGGER" == fields[0])
		{
			BOSS_PATTERN_MECHANIC_TRIGGER trigger{};
			std::uint32_t mode = 0u;
			if ((16u != fields.size() && 17u != fields.size() && 20u != fields.size() && 25u != fields.size()) || !IsStableId(fields[1]) || !IsStableId(fields[2]) ||
				!IsStableId(fields[3]) || !ParseNumber(fields[5], trigger.iStartMs) ||
				!ParseNumber(fields[6], trigger.iDurationMs) || 0u == trigger.iDurationMs ||
				!ParseNumber(fields[7], mode) || mode >= static_cast<std::uint32_t>(LostArk::Shared::KOUKU_HUD_MODE::END) ||
				!ParseNumber(fields[8], trigger.fTeleportX) || !std::isfinite(trigger.fTeleportX) ||
				!ParseNumber(fields[9], trigger.fTeleportY) || !std::isfinite(trigger.fTeleportY) ||
				!ParseNumber(fields[10], trigger.fTeleportZ) || !std::isfinite(trigger.fTeleportZ) ||
				fields[15] != "1")
			{
				m_strStatus = "KoukuSaydon mechanic trigger row is invalid";
				return false;
			}
			if (fields.size() >= 17u && (!ParseNumber(fields[16], trigger.fFaceCenterYawOffsetDegrees) ||
				!std::isfinite(trigger.fFaceCenterYawOffsetDegrees)))
			{ m_strStatus = "KoukuSaydon face-center offset is invalid"; return false; }
			if (fields.size() >= 20u && (!ParseNumber(fields[17], trigger.iCountPerPlayer) ||
				!ParseNumber(fields[18], trigger.fPlayerEffectRadiusM) || !std::isfinite(trigger.fPlayerEffectRadiusM) ||
				!ParseNumber(fields[19], trigger.iEffectLifetimeMs)))
			{ m_strStatus = "KoukuSaydon player effect values are invalid"; return false; }
			if (25u == fields.size())
			{
				if (!ParseNumber(fields[20], trigger.iArenaRandomCount) || trigger.iArenaRandomCount > 32u ||
					!ParseNumber(fields[21], trigger.fArenaRandomRadiusM) || !std::isfinite(trigger.fArenaRandomRadiusM) ||
					!ParseNumber(fields[22], trigger.fArenaHeightToleranceM) || !std::isfinite(trigger.fArenaHeightToleranceM) ||
					!ParseNumber(fields[23], trigger.fArenaMinimumSpacingM) || !std::isfinite(trigger.fArenaMinimumSpacingM) ||
					(fields[24] != "0" && fields[24] != "1") ||
					trigger.fArenaRandomRadiusM < 0.f || trigger.fArenaRandomRadiusM > 100.f ||
					trigger.fArenaHeightToleranceM < 0.f || trigger.fArenaHeightToleranceM > 10.f ||
					trigger.fArenaMinimumSpacingM < 0.f || trigger.fArenaMinimumSpacingM > 20.f ||
					(trigger.iArenaRandomCount > 0u && (trigger.fArenaRandomRadiusM <= 0.f || trigger.fArenaHeightToleranceM <= 0.f || trigger.fArenaMinimumSpacingM <= 0.f)) ||
					(trigger.iArenaRandomCount == 0u && (trigger.fArenaRandomRadiusM != 0.f || trigger.fArenaHeightToleranceM != 0.f || trigger.fArenaMinimumSpacingM != 0.f)))
				{ m_strStatus = "KoukuSaydon arena random layout is invalid"; return false; }
				trigger.bRandomPlayerOnly = fields[24] == "1";
				if (trigger.bRandomPlayerOnly && (trigger.iCountPerPlayer != 1u || trigger.fPlayerEffectRadiusM != 0.f))
				{ m_strStatus = "KoukuSaydon random player layout is invalid"; return false; }
			}
			if (fields[4] != "ALBION_BLUE_CIRCLE" && (trigger.iCountPerPlayer != 0u || trigger.fPlayerEffectRadiusM != 0.f || trigger.iEffectLifetimeMs != 0u ||
				trigger.iArenaRandomCount != 0u || trigger.bRandomPlayerOnly))
			{ m_strStatus = "Non-Albion trigger carries player effect values"; return false; }
			trigger.strTriggerId = std::string(fields[3]);
			trigger.eHudMode = static_cast<LostArk::Shared::KOUKU_HUD_MODE>(mode);
			if (fields[4] == "REAL_GAZE_TELEPORT")
			{
				trigger.eKind = BOSS_PATTERN_MECHANIC_TRIGGER_KIND::REAL_GAZE_TELEPORT;
				if (!IsStableId(fields[11]))
					return false;
				trigger.strClonePatternId = std::string(fields[11]);
				for (std::size_t index = 12u; index < 15u; ++index)
				{
					std::uint32_t hour = 0u;
					if (!ParseNumber(fields[index], hour) || hour < 2u || hour > 12u ||
						trigger.ClockHours.end() != std::find(trigger.ClockHours.begin(), trigger.ClockHours.end(), hour))
						return false;
					trigger.ClockHours.push_back(hour);
				}
			}
			else if (fields[4] == "SUMMON_PATTERNS")
			{
				trigger.eKind = BOSS_PATTERN_MECHANIC_TRIGGER_KIND::SUMMON_PATTERNS;
				if (mode != 0u || fields[11] != "-" || fields[12] != "0" || fields[13] != "0" || fields[14] != "0" ||
					trigger.fTeleportX != 0.f || trigger.fTeleportY != 0.f || trigger.fTeleportZ != 0.f || trigger.fFaceCenterYawOffsetDegrees != 0.f)
				{ m_strStatus = "Summon Pattern trigger carries unrelated values"; return false; }
			}
			else if (fields[4] == "ALBION_BLUE_CIRCLE")
			{
				trigger.eKind = BOSS_PATTERN_MECHANIC_TRIGGER_KIND::ALBION_BLUE_CIRCLE;
				if (trigger.iCountPerPlayer < 1u || trigger.iCountPerPlayer > 8u ||
					trigger.fPlayerEffectRadiusM < 0.f || trigger.fPlayerEffectRadiusM > 20.f ||
					((trigger.iCountPerPlayer == 1u) != (trigger.fPlayerEffectRadiusM == 0.f)) ||
					trigger.iEffectLifetimeMs < 1u || trigger.iEffectLifetimeMs > 600000u ||
					mode != 0u || fields[11] != "-" || fields[12] != "0" || fields[13] != "0" || fields[14] != "0" ||
					trigger.fTeleportX != 0.f || trigger.fTeleportY != 0.f || trigger.fTeleportZ != 0.f || trigger.fFaceCenterYawOffsetDegrees != 0.f)
				{ m_strStatus = "KoukuSaydon Albion layout is invalid"; return false; }
			}
			else if (fields[4] == "ALBION_AIRBORNE")
			{
				trigger.eKind = BOSS_PATTERN_MECHANIC_TRIGGER_KIND::ALBION_AIRBORNE;
				if (fields.size() != 25u || mode != 0u || fields[11] != "-" || fields[12] != "0" || fields[13] != "0" || fields[14] != "0" ||
					trigger.fFaceCenterYawOffsetDegrees != 0.f)
				{ m_strStatus = "Albion airborne base row carries unrelated values"; return false; }
			}
			else if (fields[4] == "BINGO_BOARD" || fields[4] == "BINGO_DETONATION")
			{
				trigger.eKind = fields[4] == "BINGO_BOARD" ? BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BINGO_BOARD :
					BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BINGO_DETONATION;
                if (mode != 0u || trigger.iDurationMs > 600000u || fields[11] != "-" ||
                    fields[12] != "0" || fields[13] != "0" || fields[14] != "0" ||
                    trigger.fTeleportX != 0.f || trigger.fTeleportY != 0.f || trigger.fTeleportZ != 0.f || trigger.fFaceCenterYawOffsetDegrees != 0.f)
                { m_strStatus = "Bingo duration carries unrelated values"; return false; }
			}
			else if (fields[4] == "BOSS_TRACK_TARGET" || fields[4] == "BOSS_RANDOM_TARGET" || fields[4] == "BOSS_RANDOM_TARGET_PRESENTATION")
			{
				trigger.eKind = fields[4] == "BOSS_RANDOM_TARGET_PRESENTATION" ? BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_RANDOM_TARGET_PRESENTATION :
					fields[4] == "BOSS_RANDOM_TARGET" ? BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_RANDOM_TARGET :
					BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TRACK_TARGET;
				if (mode != 0u || trigger.iDurationMs > 600000u || fields[11] != "-" ||
					fields[12] != "0" || fields[13] != "0" || fields[14] != "0" ||
					trigger.fTeleportX != 0.f || trigger.fTeleportY != 0.f || trigger.fTeleportZ != 0.f || trigger.fFaceCenterYawOffsetDegrees != 0.f)
				{ m_strStatus = "Boss tracking duration carries unrelated values"; return false; }
			}
			else if (fields[4] == "BOSS_TELEPORT_XZ" || fields[4] == "BOSS_TELEPORT_GROUNDED" || fields[4] == "BOSS_TELEPORT_FACE_CENTER" || fields[4] == "MARIO_PHASE2_PLAYERS")
			{
				trigger.eKind = fields[4] == "MARIO_PHASE2_PLAYERS" ? BOSS_PATTERN_MECHANIC_TRIGGER_KIND::MARIO_PHASE2_PLAYERS :
					fields[4] == "BOSS_TELEPORT_FACE_CENTER" ? BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TELEPORT_FACE_CENTER :
					fields[4] == "BOSS_TELEPORT_GROUNDED" ? BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TELEPORT_GROUNDED :
					BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TELEPORT_XZ;
				if (mode != 0u || fields[11] != "-" || fields[12] != "0" || fields[13] != "0" || fields[14] != "0" ||
					trigger.fFaceCenterYawOffsetDegrees != 0.f || std::abs(trigger.fTeleportX) > 100000.f ||
					std::abs(trigger.fTeleportY) > 100000.f || std::abs(trigger.fTeleportZ) > 100000.f)
				{ m_strStatus = "Boss XZ teleport carries invalid coordinates or unrelated values"; return false; }
			}
			else if (fields[4] == "CARD_RAIN_SOLDIERS" || fields[4] == "CARD_MAZE_HIDE_NEXT" || fields[4] == "CARD_MAZE_ENTER" || fields[4] == "CARD_MAZE_STAGE_PLAYERS")
			{
				trigger.eKind = fields[4] == "CARD_RAIN_SOLDIERS" ? BOSS_PATTERN_MECHANIC_TRIGGER_KIND::CARD_RAIN_SOLDIERS :
					fields[4] == "CARD_MAZE_STAGE_PLAYERS" ? BOSS_PATTERN_MECHANIC_TRIGGER_KIND::CARD_MAZE_STAGE_PLAYERS :
					fields[4] == "CARD_MAZE_HIDE_NEXT" ? BOSS_PATTERN_MECHANIC_TRIGGER_KIND::CARD_MAZE_HIDE_NEXT : BOSS_PATTERN_MECHANIC_TRIGGER_KIND::CARD_MAZE_ENTER;
				if (mode != 0u || fields[11] != "-" || fields[12] != "0" || fields[13] != "0" || fields[14] != "0" ||
					trigger.fFaceCenterYawOffsetDegrees != 0.f ||
					(trigger.eKind != BOSS_PATTERN_MECHANIC_TRIGGER_KIND::CARD_MAZE_ENTER &&
					 (trigger.fTeleportX != 0.f || trigger.fTeleportY != 0.f || trigger.fTeleportZ != 0.f)))
				{ m_strStatus = "Card maze trigger carries unrelated values"; return false; }
			}
			else if (fields[4] != "HUD_ENTER" || fields[11] != "-" ||
				fields[12] != "0" || fields[13] != "0" || fields[14] != "0")
				return false;
			const auto owners = m_BossPatterns.find(std::string(fields[1]));
			if (m_BossPatterns.end() == owners)
				return false;
			const auto owner = std::find_if(owners->second.begin(), owners->second.end(),
				[&fields](const BOSS_PATTERN_DEFINITION& pattern) { return pattern.strPatternId == fields[2]; });
			if (owners->second.end() == owner || owner->MechanicTriggers.size() >= 64u)
				return false;
			owner->MechanicTriggers.push_back(std::move(trigger));
		}
		else if (!fields.empty() && "PATTERNCARDRAINSOLDIERS" == fields[0])
		{
			std::array<std::uint32_t, 3u> counts{};
			float radiusMin = 0.f, radiusMax = 0.f;
			std::uint32_t maxHp = 0u, damage = 0u;
			if ((fields.size() != 9u && fields.size() != 11u) || !IsStableId(fields[1]) || !IsStableId(fields[2]) || !IsStableId(fields[3]) ||
				!ParseNumber(fields[4], counts[0]) || !ParseNumber(fields[5], counts[1]) || !ParseNumber(fields[6], counts[2]) ||
				!ParseNumber(fields[7], radiusMin) || !ParseNumber(fields[8], radiusMax) ||
				(fields.size() == 11u && (!ParseNumber(fields[9], maxHp) || !ParseNumber(fields[10], damage) ||
				 maxHp > 2000000000u || damage > 2000000000u)) ||
				std::any_of(counts.begin(), counts.end(), [](auto count) { return count > 32u; }) ||
				counts[0] + counts[1] + counts[2] == 0u || counts[0] + counts[1] + counts[2] > 64u ||
				!std::isfinite(radiusMin) || !std::isfinite(radiusMax) || radiusMin < 0.f || radiusMax > 100.f || radiusMin > radiusMax)
			{ m_strStatus = "Card rain soldier tuning is invalid"; return false; }
			const auto encounter = m_BossPatterns.find(std::string(fields[1]));
			if (encounter == m_BossPatterns.end()) return false;
			const auto pattern = std::find_if(encounter->second.begin(), encounter->second.end(), [&](const auto& row) { return row.strPatternId == fields[2]; });
			if (pattern == encounter->second.end()) return false;
			const auto trigger = std::find_if(pattern->MechanicTriggers.begin(), pattern->MechanicTriggers.end(), [&](const auto& row) { return row.strTriggerId == fields[3]; });
			if (trigger == pattern->MechanicTriggers.end() || trigger->eKind != BOSS_PATTERN_MECHANIC_TRIGGER_KIND::CARD_RAIN_SOLDIERS || trigger->bHasSoldierTuning)
			{ m_strStatus = "Card rain tuning owner is missing or duplicated"; return false; }
			trigger->SoldierCounts = counts; trigger->fSoldierSpawnRadiusMinM = radiusMin;
			trigger->fSoldierSpawnRadiusMaxM = radiusMax; trigger->bHasSoldierTuning = true;
			trigger->iSoldierMaxHp = maxHp; trigger->iSoldierDamage = damage;
		}
		else if (!fields.empty() && ("PATTERNBINGOBOARD" == fields[0] || "PATTERNBINGOCONTROL" == fields[0]))
		{
			const bool control = fields[0] == "PATTERNBINGOCONTROL";
			if (fields.size() != (control ? 4u : 11u) || !IsStableId(fields[1]) || !IsStableId(fields[2]) || !IsStableId(fields[3]))
			{ m_strStatus = "Bingo settings row identity is invalid"; return false; }
			const auto encounter = m_BossPatterns.find(std::string(fields[1]));
			if (encounter == m_BossPatterns.end()) return false;
			const auto pattern = std::find_if(encounter->second.begin(), encounter->second.end(), [&](const auto& row) { return row.strPatternId == fields[2]; });
			if (pattern == encounter->second.end()) return false;
			const auto trigger = std::find_if(pattern->MechanicTriggers.begin(), pattern->MechanicTriggers.end(), [&](const auto& row) { return row.strTriggerId == fields[3]; });
			if (trigger == pattern->MechanicTriggers.end() || trigger->eKind != BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BINGO_BOARD ||
				(control ? trigger->bBingoControlOnly : trigger->BingoBoardSettings.has_value()))
			{ m_strStatus = "Bingo settings owner is missing, incompatible or duplicated"; return false; }
			if (control) trigger->bBingoControlOnly = true;
			else
			{
				BOSS_BINGO_BOARD_SETTINGS settings;
				if ((fields[4] != "ENCOUNTER" && fields[4] != "WINDOW") ||
					!ParseNumber(fields[5], settings.iFirstBombDelayMs) || !ParseNumber(fields[6], settings.iBombIntervalMs) ||
					!ParseNumber(fields[7], settings.iBombMarkMs) || !ParseNumber(fields[8], settings.iBombDropDelayMs) ||
					!ParseNumber(fields[9], settings.iBombFuseMs) || !ParseNumber(fields[10], settings.iInitialMarkedCells) || !settings.Is_Valid())
				{ m_strStatus = "Bingo board timing or initial cells are invalid"; return false; }
				settings.bWindowOnly = fields[4] == "WINDOW";
				trigger->BingoBoardSettings = settings;
			}
		}
		else if (!fields.empty() && "PATTERNBINGOHAMMER" == fields[0])
		{
			std::array<float, 2u> half{};
			if (fields.size() != 6u || !IsStableId(fields[1]) || !IsStableId(fields[2]) || !IsStableId(fields[3]))
			{ m_strStatus = "Bingo hammer row identity is invalid"; return false; }
			for (std::size_t axis = 0; axis < half.size(); ++axis)
				if (!ParseNumber(fields[4u + axis], half[axis]) || !std::isfinite(half[axis]) || half[axis] <= 0.f || half[axis] > 100.f)
				{ m_strStatus = "Bingo hammer half extent is invalid"; return false; }
			const auto encounter = m_BossPatterns.find(std::string(fields[1]));
			if (encounter == m_BossPatterns.end()) return false;
			const auto pattern = std::find_if(encounter->second.begin(), encounter->second.end(), [&](const auto& row) { return row.strPatternId == fields[2]; });
			if (pattern == encounter->second.end()) return false;
			const auto trigger = std::find_if(pattern->MechanicTriggers.begin(), pattern->MechanicTriggers.end(), [&](const auto& row) { return row.strTriggerId == fields[3]; });
			if (trigger == pattern->MechanicTriggers.end() || trigger->eKind != BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BINGO_BOARD || trigger->BingoHammerHalfExtentsM)
			{ m_strStatus = "Bingo hammer owner or duplicate is invalid"; return false; }
			trigger->BingoHammerHalfExtentsM = half;
		}
		else if (!fields.empty() && "PATTERNCARDMAZESTAGING" == fields[0])
		{
			std::uint32_t slot = 0u;
			std::array<float, 3u> position{};
			if (fields.size() != 8u || !IsStableId(fields[1]) || !IsStableId(fields[2]) || !IsStableId(fields[3]) ||
				!ParseNumber(fields[4], slot) || slot >= 4u)
			{ m_strStatus = "Card maze staging row identity or slot is invalid"; return false; }
			for (std::size_t axis = 0u; axis < position.size(); ++axis)
				if (!ParseNumber(fields[5u + axis], position[axis]) || !std::isfinite(position[axis]) || std::abs(position[axis]) > 100000.f)
				{ m_strStatus = "Card maze staging position is invalid"; return false; }
			const auto owners = m_BossPatterns.find(std::string(fields[1]));
			if (owners == m_BossPatterns.end()) return false;
			const auto owner = std::find_if(owners->second.begin(), owners->second.end(),
				[&fields](const auto& pattern) { return pattern.strPatternId == fields[2]; });
			if (owner == owners->second.end()) return false;
			const auto trigger = std::find_if(owner->MechanicTriggers.begin(), owner->MechanicTriggers.end(),
				[&fields](const auto& row) { return row.strTriggerId == fields[3]; });
			if (trigger == owner->MechanicTriggers.end() || trigger->eKind != BOSS_PATTERN_MECHANIC_TRIGGER_KIND::CARD_MAZE_STAGE_PLAYERS ||
				trigger->PlayerEntryPositions.size() != slot)
			{ m_strStatus = "Card maze staging slots must follow their owner in contiguous order"; return false; }
			trigger->PlayerEntryPositions.push_back(position);
		}
		else if (!fields.empty() && "PATTERNALBIONAIRBORNE" == fields[0])
		{
			if ((fields.size() != 7u && fields.size() != 10u) || !IsStableId(fields[1]) || !IsStableId(fields[2]) || !IsStableId(fields[3]))
			{ m_strStatus = "Albion airborne supplemental identity is invalid"; return false; }
			const auto encounter = m_BossPatterns.find(std::string(fields[1]));
			if (encounter == m_BossPatterns.end()) { m_strStatus = "Albion airborne encounter is missing"; return false; }
			const auto pattern = std::find_if(encounter->second.begin(), encounter->second.end(), [&](const auto& row) { return row.strPatternId == fields[2]; });
			if (pattern == encounter->second.end()) { m_strStatus = "Albion airborne pattern is missing"; return false; }
			const auto trigger = std::find_if(pattern->MechanicTriggers.begin(), pattern->MechanicTriggers.end(), [&](const auto& row) { return row.strTriggerId == fields[3]; });
			if (trigger == pattern->MechanicTriggers.end() || trigger->eKind != BOSS_PATTERN_MECHANIC_TRIGGER_KIND::ALBION_AIRBORNE ||
				trigger->eAirbornePhase != ALBION_AIRBORNE_PHASE::NONE || !ParseNumber(fields[5], trigger->fAirborneHeightM) ||
				!std::isfinite(trigger->fAirborneHeightM) || !ParseNumber(fields[6], trigger->iAirborneDurationMs))
			{ m_strStatus = "Albion airborne owner, duplicate or values are invalid"; return false; }
			if (fields[4] == "JUMP") trigger->eAirbornePhase = ALBION_AIRBORNE_PHASE::JUMP;
			else if (fields[4] == "SELECT_PLAYER") trigger->eAirbornePhase = ALBION_AIRBORNE_PHASE::SELECT_PLAYER;
			else if (fields[4] == "APPEAR_PLAYER") trigger->eAirbornePhase = ALBION_AIRBORNE_PHASE::APPEAR_PLAYER;
			else if (fields[4] == "DISAPPEAR") trigger->eAirbornePhase = ALBION_AIRBORNE_PHASE::DISAPPEAR;
			else if (fields[4] == "CENTER") trigger->eAirbornePhase = ALBION_AIRBORNE_PHASE::CENTER;
			else if (fields[4] == "SLAM") trigger->eAirbornePhase = ALBION_AIRBORNE_PHASE::SLAM;
			else { m_strStatus = "Albion airborne phase is unknown"; return false; }
			if (fields.size() == 10u)
			{
				if (trigger->eAirbornePhase != ALBION_AIRBORNE_PHASE::SELECT_PLAYER || fields[7] != "SELECT" ||
					(!fields[8].empty() && !IsStableId(fields[8])) || !ParseNumber(fields[9], trigger->iSelectedEffectLifetimeMs) ||
					fields[8].empty() != (trigger->iSelectedEffectLifetimeMs == 0u))
				{ m_strStatus = "Selected airborne position or Effect fields are invalid"; return false; }
				trigger->bCaptureAirborneTargetPosition = true;
				trigger->strSelectedEffectVisualId = fields[8];
			}
		}
		else if (!fields.empty() && "PATTERNTRACKBOMB" == fields[0])
		{
			BOSS_SHOWTIME_BOMB_RESOLUTION bomb;
			if (fields.size() != 18u || !IsStableId(fields[1]) || !IsStableId(fields[2]) ||
				!IsStableId(fields[3]) || !ParseNumber(fields[4], bomb.iBodyStartMs) ||
				!IsStableId(fields[5]) || !IsStableId(fields[6]) || fields[5] == fields[6] ||
				!ParseNumber(fields[7], bomb.iExplosionLifetimeMs) ||
				!bomb.iExplosionLifetimeMs || bomb.iExplosionLifetimeMs > 600000u)
			{ m_strStatus = "Showtime bomb attachment identity or lifetime is invalid"; return false; }
			for (std::size_t axis = 0u; axis < 3u; ++axis)
				if (!ParseNumber(fields[8u + axis], bomb.BodyPosition[axis]) ||
					!std::isfinite(bomb.BodyPosition[axis]) || std::abs(bomb.BodyPosition[axis]) > 100000.f)
				{ m_strStatus = "Showtime bomb MAP position is invalid"; return false; }
			auto& fan = bomb.FanRegion;
			float* values[] = { &fan.fCenterX, &fan.fCenterY, &fan.fCenterZ, &fan.fYawDegrees,
				&fan.fRadiusXM, &fan.fRadiusZM, &fan.fHalfAngleDegrees };
			for (std::size_t index = 0u; index < 7u; ++index)
				if (!ParseNumber(fields[11u + index], *values[index]) || !std::isfinite(*values[index]))
				{ m_strStatus = "Showtime bomb fan contains a non-finite value"; return false; }
			if (std::abs(fan.fCenterX) > 1000.f || std::abs(fan.fCenterY) > 1000.f ||
				std::abs(fan.fCenterZ) > 1000.f || std::abs(fan.fYawDegrees) > 360.f ||
				fan.fRadiusXM <= 0.f || fan.fRadiusXM > 1000.f || fan.fRadiusZM <= 0.f || fan.fRadiusZM > 1000.f ||
				fan.fHalfAngleDegrees <= 0.f || fan.fHalfAngleDegrees >= 180.f)
			{ m_strStatus = "Showtime bomb fan is outside its sector bounds"; return false; }
			const auto owners = m_BossPatterns.find(std::string(fields[1]));
			if (owners == m_BossPatterns.end()) { m_strStatus = "Showtime bomb encounter is missing"; return false; }
			const auto pattern = std::find_if(owners->second.begin(), owners->second.end(),
				[&](const auto& row) { return row.strPatternId == fields[2]; });
			if (pattern == owners->second.end()) { m_strStatus = "Showtime bomb pattern is missing"; return false; }
			const auto trigger = std::find_if(pattern->MechanicTriggers.begin(), pattern->MechanicTriggers.end(),
				[&](const auto& row) { return row.strTriggerId == fields[3]; });
			if (trigger == pattern->MechanicTriggers.end() || trigger->ShowtimeBomb ||
				trigger->eKind != BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TRACK_TARGET ||
				trigger->fFollowSpeedScale != 0.f || trigger->iDurationMs <= 34u ||
				bomb.iBodyStartMs >= std::uint64_t(trigger->iStartMs) + trigger->iDurationMs)
			{ m_strStatus = "Showtime bomb needs one rotate-only tracking duration and an earlier body birth"; return false; }
			bomb.strBodyVisualId = fields[5]; bomb.strExplosionVisualId = fields[6];
			fan.strRegionId = fields[3]; fan.eAnchor = BOSS_LOGIC_REGION_ANCHOR::BOSS_CURRENT;
			fan.bSector = true; fan.fRadiusM = (std::max)(fan.fRadiusXM, fan.fRadiusZM);
			trigger->ShowtimeBomb = std::move(bomb);
		}
		else if (!fields.empty() && "PATTERNTRACKMOVE" == fields[0])
		{
			/* 모든 PATTERNMECHANICTRIGGER 뒤에서 기존 BOSS_TRACK_TARGET occurrence를 stable ID로 찾아 보완한다. */
			float scale = 0.f;
			if (fields.size() != 5u || !IsStableId(fields[1]) || !IsStableId(fields[2]) || !IsStableId(fields[3]) ||
				!ParseNumber(fields[4], scale) || !std::isfinite(scale) || scale < .01f || scale > 10.f)
			{ m_strStatus = "Boss tracking follow row is invalid"; return false; }
			const auto owners = m_BossPatterns.find(std::string(fields[1]));
			if (owners == m_BossPatterns.end()) { m_strStatus = "Boss tracking follow encounter is missing"; return false; }
			const auto pattern = std::find_if(owners->second.begin(), owners->second.end(),
				[&](const auto& row) { return row.strPatternId == fields[2]; });
			if (pattern == owners->second.end() || pattern->BossMotion)
			{ m_strStatus = "Boss tracking follow pattern is missing or owns absolute motion"; return false; }
			const auto trigger = std::find_if(pattern->MechanicTriggers.begin(), pattern->MechanicTriggers.end(),
				[&](const auto& row) { return row.strTriggerId == fields[3]; });
			if (trigger == pattern->MechanicTriggers.end() ||
				trigger->eKind != BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TRACK_TARGET ||
				trigger->fFollowSpeedScale != 0.f)
			{ m_strStatus = "Boss tracking follow needs one unauthored BOSS_TRACK_TARGET trigger"; return false; }
			// window 하나는 이동 owner 하나만 갖는다. charge가 이미 소유한 body에 다른 이동 owner를 겹치지 않는다.
			for (const auto& window : pattern->LogicWindows)
				if (window.fBossChargeDistanceM > 0.f &&
					window.iStartMs < trigger->iStartMs + trigger->iDurationMs &&
					trigger->iStartMs < window.iStartMs + window.iDurationMs)
				{ m_strStatus = "Boss tracking follow overlaps a charge window"; return false; }
			trigger->fFollowSpeedScale = scale;
		}
		else if (!fields.empty() && "PATTERNPURSUITPROJECTILES" == fields[0])
		{
			BOSS_PATTERN_MECHANIC_TRIGGER trigger{};
			trigger.eKind = BOSS_PATTERN_MECHANIC_TRIGGER_KIND::PURSUIT_PROJECTILES;
			std::uint32_t homing = 0u;
			if ((fields.size() != 18u && fields.size() != 19u) || !IsStableId(fields[1]) || !IsStableId(fields[2]) || !IsStableId(fields[3]) ||
				!ParseNumber(fields[4], trigger.iStartMs) || !ParseNumber(fields[5], trigger.iDurationMs) ||
				trigger.iDurationMs == 0u || trigger.iDurationMs > 600000u || !IsStableId(fields[10]) ||
				!ParseNumber(fields[11], trigger.fProjectileSpeedMps) || !ParseNumber(fields[12], trigger.fProjectileContactRadiusM) ||
				!ParseNumber(fields[13], trigger.fProjectileSpawnRadiusM) || !ParseNumber(fields[14], trigger.iProjectileLifetimeMs) ||
				!ParseNumber(fields[15], trigger.iSpawnIntervalMs) || !ParseNumber(fields[16], homing) || homing > 1u ||
				!ParseNumber(fields[17], trigger.iProjectileCountPerWave) ||
				(fields.size() == 19u && !ParseNumber(fields[18], trigger.fProjectileMaxDistanceM)))
			{ m_strStatus = "Pursuit projectile row is invalid"; return false; }
			bool ended = false;
			for (std::size_t index = 6u; index < 10u; ++index)
			{
				if (fields[index] == "-") { ended = true; continue; }
				if (ended || !IsStableId(fields[index])) { m_strStatus = "Pursuit visual pool is invalid"; return false; }
				trigger.ProjectileVisualIds.emplace_back(fields[index]);
			}
			trigger.strTriggerId = fields[3]; trigger.strContactVisualId = fields[10]; trigger.bProjectileHoming = homing != 0u;
			const auto owners = m_BossPatterns.find(std::string(fields[1]));
			if (owners == m_BossPatterns.end()) { m_strStatus = "Pursuit encounter is missing"; return false; }
			const auto owner = std::find_if(owners->second.begin(), owners->second.end(), [&](const auto& pattern) { return pattern.strPatternId == fields[2]; });
			if (owner == owners->second.end() || owner->MechanicTriggers.size() >= 64u ||
				std::any_of(owner->MechanicTriggers.begin(), owner->MechanicTriggers.end(), [&](const auto& row) { return row.strTriggerId == fields[3]; }))
			{ m_strStatus = "Pursuit occurrence owner is invalid"; return false; }
			owner->MechanicTriggers.push_back(std::move(trigger));
		}
        else if (!fields.empty() && "PATTERNPURSUITCARDS" == fields[0])
        {
            std::vector<LostArk::Shared::MECHANIC_CARD_SYMBOL> symbols;
            if (fields.size() != 5u || !IsStableId(fields[1]) || !IsStableId(fields[2]) || !IsStableId(fields[3]) ||
                !ParseMechanicCardSymbols(fields[4], symbols) || symbols.empty() || symbols.size() > 4u)
            { m_strStatus = "Pursuit card symbols are invalid"; return false; }
            const auto owners = m_BossPatterns.find(std::string(fields[1]));
            if (owners == m_BossPatterns.end()) { m_strStatus = "Pursuit card encounter is missing"; return false; }
            const auto owner = std::find_if(owners->second.begin(), owners->second.end(), [&](const auto& pattern) { return pattern.strPatternId == fields[2]; });
            if (owner == owners->second.end()) { m_strStatus = "Pursuit card pattern is missing"; return false; }
            const auto trigger = std::find_if(owner->MechanicTriggers.begin(), owner->MechanicTriggers.end(), [&](const auto& row) { return row.strTriggerId == fields[3]; });
            if (trigger == owner->MechanicTriggers.end() || trigger->eKind != BOSS_PATTERN_MECHANIC_TRIGGER_KIND::PURSUIT_PROJECTILES ||
                !trigger->ProjectileCardSymbols.empty() || symbols.size() != trigger->ProjectileVisualIds.size())
            { m_strStatus = "Pursuit card symbols must match the visual pool exactly once"; return false; }
            trigger->ProjectileCardSymbols = std::move(symbols);
        }
		else if (!fields.empty() && "PATTERNSHOWTIMETARGETS" == fields[0])
		{
			BOSS_PATTERN_MECHANIC_TRIGGER trigger{};
			trigger.eKind = BOSS_PATTERN_MECHANIC_TRIGGER_KIND::SHOWTIME_PLAYER_TARGETS;
			if (fields.size() != 11u || !IsStableId(fields[1]) || !IsStableId(fields[2]) || !IsStableId(fields[3]) ||
				!ParseNumber(fields[4], trigger.iStartMs) || !ParseNumber(fields[5], trigger.iDurationMs) ||
				trigger.iDurationMs == 0u || trigger.iDurationMs > 600000u ||
				(fields[6] != "-" && !IsStableId(fields[6])) || (fields[7] != "-" && !IsStableId(fields[7])) || (fields[6] != "-" && fields[6] == fields[7]) ||
				!ParseNumber(fields[8], trigger.iFixedLifetimeMs) || (fields[6] == "-") != (trigger.iFixedLifetimeMs == 0u) || trigger.iFixedLifetimeMs > 600000u ||
				!ParseNumber(fields[9], trigger.iSpawnIntervalMs) || trigger.iSpawnIntervalMs == 0u || trigger.iSpawnIntervalMs > 600000u ||
				!ParseNumber(fields[10], trigger.fFollowSpeedScale) || !std::isfinite(trigger.fFollowSpeedScale) ||
				trigger.fFollowSpeedScale < .01f || trigger.fFollowSpeedScale > 10.f)
			{ m_strStatus = "Showtime player-target window is invalid"; return false; }
			const auto owners = m_BossPatterns.find(std::string(fields[1]));
			if (owners == m_BossPatterns.end())
			{ m_strStatus = "Showtime player-target encounter is missing"; return false; }
			const auto owner = std::find_if(owners->second.begin(), owners->second.end(),
				[&](const auto& pattern) { return pattern.strPatternId == fields[2]; });
			if (owner == owners->second.end() || owner->MechanicTriggers.size() >= 64u ||
				std::any_of(owner->MechanicTriggers.begin(), owner->MechanicTriggers.end(),
					[&](const auto& row) { return row.strTriggerId == fields[3]; }))
			{ m_strStatus = "Showtime player-target owner or occurrence is invalid"; return false; }
			trigger.strTriggerId = fields[3];
			if (fields[6] != "-") trigger.strFixedVisualId = fields[6];
			if (fields[7] != "-") trigger.strTrackingVisualId = fields[7];
			owner->MechanicTriggers.push_back(std::move(trigger));
		}
		else if (!fields.empty() && "PATTERNSHOWTIMERANDOM" == fields[0])
		{
			BOSS_SHOWTIME_RANDOM_VOLLEY volley;
			std::uint32_t ordinal = 0u, interval = 0u;
			float radius = 0.f, height = 0.f, minScale = 1.f, maxScale = 1.f;
			std::string anchor = "BOSS_SPAWN";
			if ((fields.size() != 10u && fields.size() != 13u) || !IsStableId(fields[1]) || !IsStableId(fields[2]) || !IsStableId(fields[3]) ||
				!ParseNumber(fields[4], ordinal) || ordinal >= 32u || !IsStableId(fields[5]) ||
				!ParseNumber(fields[6], volley.iLifetimeMs) || volley.iLifetimeMs == 0u || volley.iLifetimeMs > 600000u ||
				!ParseNumber(fields[7], interval) || interval == 0u || interval > 600000u ||
				!ParseNumber(fields[8], radius) || !std::isfinite(radius) || radius <= 0.f || radius > 1000.f ||
				!ParseNumber(fields[9], height) || !std::isfinite(height) || height <= 0.f || height > 10.f)
			{ m_strStatus = "Showtime random volley row is invalid"; return false; }
            if (fields.size() == 13u)
            {
                anchor = fields[10];
                if ((anchor != "BOSS_SPAWN" && anchor != "BOSS") || !ParseNumber(fields[11], minScale) || !ParseNumber(fields[12], maxScale) ||
                    !std::isfinite(minScale) || !std::isfinite(maxScale) || minScale < .01f || maxScale > 10.f || maxScale < minScale)
                { m_strStatus = "Showtime random anchor or scale is invalid"; return false; }
            }
			const auto owners = m_BossPatterns.find(std::string(fields[1]));
			if (owners == m_BossPatterns.end())
			{ m_strStatus = "Showtime random volley encounter is missing"; return false; }
			const auto pattern = std::find_if(owners->second.begin(), owners->second.end(),
				[&](const auto& row) { return row.strPatternId == fields[2]; });
			if (pattern == owners->second.end())
			{ m_strStatus = "Showtime random volley pattern is missing"; return false; }
			const auto trigger = std::find_if(pattern->MechanicTriggers.begin(), pattern->MechanicTriggers.end(),
				[&](const auto& row) { return row.strTriggerId == fields[3]; });
			if (trigger == pattern->MechanicTriggers.end() || trigger->eKind != BOSS_PATTERN_MECHANIC_TRIGGER_KIND::SHOWTIME_PLAYER_TARGETS ||
				trigger->RandomVolleys.size() != ordinal || (!trigger->RandomVolleys.empty() &&
				(trigger->iRandomSpawnIntervalMs != interval || trigger->fRandomArenaRadiusM != radius || trigger->fRandomArenaHeightToleranceM != height ||
                 trigger->strRandomAnchorKind != anchor || trigger->fRandomScaleMin != minScale || trigger->fRandomScaleMax != maxScale)))
			{ m_strStatus = "Showtime random volley owner, order or repeated settings do not match"; return false; }
			volley.strClientVisualId = fields[5];
			trigger->iRandomSpawnIntervalMs = interval; trigger->fRandomArenaRadiusM = radius; trigger->fRandomArenaHeightToleranceM = height;
			trigger->strRandomAnchorKind = anchor; trigger->fRandomScaleMin = minScale; trigger->fRandomScaleMax = maxScale;
			trigger->RandomVolleys.push_back(std::move(volley));
		}
		else if (!fields.empty() && "PATTERNATTACKHIT" == fields[0])
		{
			// 게시된 한 행을 임시 공격 정의로 읽는다. 앞의 ID와 role/set/ordinal은
			// 이 판정을 보관할 stage 또는 동적 trigger, 그 안의 배열 위치를 가리킨다.
			// 허용된 필드 수를 먼저 검사하므로 뒤의 필드 접근은 이 범위 안에서만 한다.
			LostArk::Shared::ATTACK_HIT_TEMPLATE hit;
			std::uint32_t set = 0u, ordinal = 0u;
			if ((fields.size() != 25u &&
				 fields.size() != 27u &&
				 fields.size() != 28u &&
				 fields.size() != 30u) ||
				!IsStableId(fields[1]) ||
				!IsStableId(fields[2]) ||
				!IsStableId(fields[3]) ||
				!ParseNumber(fields[5], set) ||
				!ParseNumber(fields[6], ordinal) ||
				set >= 32u ||
				ordinal >= 32u)
			{
				m_strStatus = "Attack template header is invalid";
				return false;
			}

			hit.strHitId = fields[7];
			hit.strTrigger = fields[8];
			hit.strShape = fields[13];
			// 숫자 밸런스 조회용 키는 게시 행의 소유자와 순서로 만든다.
			// 저작 ID를 새로 발급하거나 네트워크 ID로 사용하는 값은 아니다.
			hit.strNumericBalanceId = "H|" + std::string(fields[1]) + "|" +
				std::string(fields[2]) + "|" + std::string(fields[3]) + "|" +
				std::string(fields[4]) + "|" + std::string(fields[5]) + "|" +
				std::string(fields[6]);

			hit.strDamageKind = fields[22];
			if (fields[24] != "-")
			{
				hit.strDamageProfileId = fields[24];
			}
			// 여기서는 문자열을 수치로 변환한다. 시간(ms), 거리(m), 각도(degree)의
			// 허용 범위와 shape/damage 조합은 소유자의 수명을 정한 뒤 공용 검증기로 검사한다.
			if (!ParseNumber(fields[9], hit.iAtMs) ||
				!ParseNumber(fields[10], hit.iEndMs) ||
				!ParseNumber(fields[11], hit.iRepeatCount) ||
				!ParseNumber(fields[12], hit.iRepeatIntervalMs) ||
				!ParseNumber(fields[14], hit.fRadiusM) ||
				!ParseNumber(fields[15], hit.fInnerRadiusM) ||
				!ParseNumber(fields[16], hit.fLengthM) ||
				!ParseNumber(fields[17], hit.fHalfWidthM) ||
				!ParseNumber(fields[18], hit.fAngleDegrees) ||
				!ParseNumber(fields[19], hit.fOffsetForwardM) ||
				!ParseNumber(fields[20], hit.fOffsetRightM) ||
				!ParseNumber(fields[21], hit.fYawOffsetDegrees) ||
				!ParseNumber(fields[23], hit.iDamagePercent))
			{
				m_strStatus = "Attack template number is invalid";
				return false;
			}
			if (fields.size() >= 27u &&
				(!ParseNumber(fields[25], hit.fRiseHeightM) ||
				 !ParseNumber(fields[26], hit.iPushMs)))
			{
				m_strStatus = "Attack template rise height or duration is invalid";
				return false;
			}
			if (fields.size() >= 28u && !ParseNumber(fields[27], hit.fPushRangeM))
			{
				m_strStatus = "Attack template push range is invalid";
				return false;
			}
			if (fields.size() == 30u)
			{
				if (fields[28] != "0" && fields[28] != "1")
				{
					m_strStatus = "Attack forcePush is invalid";
					return false;
				}
				hit.ForcePush = fields[28] == "1";
				hit.strPushDirection = fields[29];
			}

			// ID 형식이 올바른 것과 실제 소유자가 존재하는 것은 다른 조건이다.
			// 먼저 게시 catalog 안의 encounter와 pattern을 찾아 해당 배열에만 연결한다.
			const auto owners = m_BossPatterns.find(std::string(fields[1]));
			if (owners == m_BossPatterns.end())
			{
				m_strStatus = "Attack encounter is missing";
				return false;
			}
			const auto pattern = std::find_if(
				owners->second.begin(), owners->second.end(),
				[&](const auto& row)
				{
					return row.strPatternId == fields[2];
				});
			if (pattern == owners->second.end())
			{
				m_strStatus = "Attack pattern is missing";
				return false;
			}

			if (fields[4] == "STAGE")
			{
				const auto stage = std::find_if(
					pattern->Stages.begin(), pattern->Stages.end(),
					[&](const auto& row)
					{
						return row.strActionId == fields[3];
					});
				// Stage 판정은 한 배열에 게시된 순서대로 들어온다. PROFILE의 문법뿐
				// 아니라 실제 피해 profile 조회도 이 catalog 계층에서 확인한다.
				if (set != 0u ||
					stage == pattern->Stages.end() ||
					stage->AttackContacts.size() != ordinal ||
					(hit.strDamageKind == "PROFILE" &&
					 !Find_DamageRatePercent(hit.strDamageProfileId)))
				{
					m_strStatus = "Stage contact owner, order or profile is invalid";
					return false;
				}
				stage->AttackContacts.push_back(std::move(hit));
				// 판정은 stage 종료 전에 있어야 한다. duration 0을 먼저 거부해
				// duration - 1의 unsigned underflow를 막는다. 실패하면 이 함수의
				// LOAD_ROLLBACK이 로드 전 패턴 저장소를 복원하므로 부분 행을 남기지 않는다.
				if (!stage->iDurationMs ||
					!LostArk::Shared::Validate_AttackHitTemplates(
						stage->AttackContacts, stage->iDurationMs - 1u))
				{
					m_strStatus = "Stage contact shape, time or damage is invalid";
					return false;
				}
				continue;
			}

			const auto trigger = std::find_if(
				pattern->MechanicTriggers.begin(), pattern->MechanicTriggers.end(),
				[&](const auto& row)
				{
					return row.strTriggerId == fields[3];
				});
			if (trigger == pattern->MechanicTriggers.end())
			{
				m_strStatus = "Attack dynamic owner is missing";
				return false;
			}
			// destination은 trigger가 소유한 vector를 이번 행 처리 동안만 빌린다.
			// role/set에 맞는 배열과 그 수명(ms)을 함께 골라 다른 owner의 판정을 섞지 않는다.
			std::vector<LostArk::Shared::ATTACK_HIT_TEMPLATE>* destination = nullptr;
			std::uint32_t lifetime = 0u;
			if (trigger->eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::SHOWTIME_PLAYER_TARGETS)
			{
				if (fields[4] == "FIXED" && set == 0u && !trigger->strFixedVisualId.empty())
				{
					destination = &trigger->FixedHits;
					lifetime = trigger->iFixedLifetimeMs;
				}
				else if (fields[4] == "TRACKING" && set == 0u && !trigger->strTrackingVisualId.empty())
				{
					destination = &trigger->TrackingHits;
					lifetime = trigger->iDurationMs;
				}
				else if (fields[4] == "RANDOM" && set < trigger->RandomVolleys.size())
				{
					destination = &trigger->RandomVolleys[set].Hits;
					lifetime = trigger->RandomVolleys[set].iLifetimeMs;
				}
			}
			else if (trigger->eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::ALBION_BLUE_CIRCLE &&
				fields[4] == "ALBION" && set == 0u)
			{
				destination = &trigger->FixedHits;
				lifetime = trigger->iEffectLifetimeMs;
			}
			else if (trigger->eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::ALBION_AIRBORNE &&
				fields[4] == "SELECTED" && set == 0u &&
				trigger->eAirbornePhase == ALBION_AIRBORNE_PHASE::SELECT_PLAYER &&
				trigger->bCaptureAirborneTargetPosition &&
				!trigger->strSelectedEffectVisualId.empty())
			{
				destination = &trigger->FixedHits;
				lifetime = trigger->iSelectedEffectLifetimeMs;
			}
			else if (trigger->eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::PURSUIT_PROJECTILES &&
				fields[4] == "PROJECTILE" && set == 0u)
			{
				destination = &trigger->ProjectileHits;
				lifetime = trigger->iProjectileLifetimeMs ?
					trigger->iProjectileLifetimeMs : 600000u;
			}
			if (!destination || destination->size() != ordinal ||
				(hit.strDamageKind == "PROFILE" &&
				 !Find_DamageRatePercent(hit.strDamageProfileId)))
			{
				m_strStatus = "Attack template owner, order or profile is invalid";
				return false;
			}
			destination->push_back(std::move(hit));
			// 앞선 행까지 포함해 ID 중복과 값의 조합을 검증한다. 이 단계도 최종
			// catalog commit 전이므로 실패하면 LOAD_ROLLBACK이 이전 패턴 저장소를 복원한다.
			if (!LostArk::Shared::Validate_AttackHitTemplates(*destination, lifetime))
			{
				m_strStatus = "Attack template shape, window or damage is invalid";
				return false;
			}
		}
		else if (!fields.empty() && "PATTERNSUMMONSPAWN" == fields[0])
		{
			BOSS_PATTERN_SUMMON_PATTERN_SPAWN spawn{};
			if ((fields.size() != 10u && fields.size() != 11u) || !IsStableId(fields[1]) || !IsStableId(fields[2]) || !IsStableId(fields[3]) ||
				!IsStableId(fields[4]) || !IsStableId(fields[5]) || fields[2] == fields[5])
			{ m_strStatus = "Summon Pattern spawn identity is invalid"; return false; }
			if (fields.size() == 11u)
			{
				if (fields[10] != "MAP" && fields[10] != "BOSS")
				{ m_strStatus = "Summon Pattern anchor kind is invalid"; return false; }
				if (fields[10] == "MAP") spawn.eAnchorKind = BOSS_PATTERN_SUMMON_ANCHOR_KIND::MAP;
			}
			const float positionLimit = spawn.eAnchorKind == BOSS_PATTERN_SUMMON_ANCHOR_KIND::MAP ? 100000.f : 1000.f;
			for (std::size_t axis = 0u; axis < 3u; ++axis)
				if (!ParseNumber(fields[6u + axis], spawn.PositionOffset[axis]) ||
					!std::isfinite(spawn.PositionOffset[axis]) || std::abs(spawn.PositionOffset[axis]) > positionLimit)
				{ m_strStatus = "Summon Pattern local offset is invalid"; return false; }
			if (!ParseNumber(fields[9], spawn.fYawOffsetDegrees) || !std::isfinite(spawn.fYawOffsetDegrees) ||
				std::abs(spawn.fYawOffsetDegrees) > 360.f)
			{ m_strStatus = "Summon Pattern yaw offset is invalid"; return false; }
			const auto owners = m_BossPatterns.find(std::string(fields[1]));
			if (owners == m_BossPatterns.end())
			{ m_strStatus = "Summon Pattern encounter is missing"; return false; }
			const auto owner = std::find_if(owners->second.begin(), owners->second.end(),
				[&](const auto& pattern) { return pattern.strPatternId == fields[2]; });
			if (owner == owners->second.end())
			{ m_strStatus = "Summon Pattern owner is missing"; return false; }
			const auto trigger = std::find_if(owner->MechanicTriggers.begin(), owner->MechanicTriggers.end(),
				[&](const auto& row) { return row.strTriggerId == fields[3]; });
			if (trigger == owner->MechanicTriggers.end() || trigger->eKind != BOSS_PATTERN_MECHANIC_TRIGGER_KIND::SUMMON_PATTERNS ||
				trigger->PatternSpawns.size() >= KOUKU_SUMMON_MAX_PATTERN_SPAWNS || std::any_of(trigger->PatternSpawns.begin(), trigger->PatternSpawns.end(),
					[&](const auto& row) { return row.strSpawnId == fields[4]; }))
			{ m_strStatus = "Summon Pattern trigger or spawn cardinality is invalid"; return false; }
			spawn.strSpawnId = std::string(fields[4]); spawn.strPatternId = std::string(fields[5]);
			trigger->PatternSpawns.push_back(std::move(spawn));
		}
        else if (!fields.empty() && "PATTERNPARENTCHILD" == fields[0])
        {
            BOSS_PATTERN_PARENT_CHILD child; std::uint32_t loop = 0u;
            if (fields.size() != 8u || fields[1] != "ENCOUNTER_KOUKUSAYDON_G1" ||
                !IsStableId(fields[2]) || !IsStableId(fields[3]) || !IsStableId(fields[4]) || fields[2] == fields[4] ||
                !ParseNumber(fields[5], child.iStartMs) || !ParseNumber(fields[6], child.iDurationMs) ||
                !child.iDurationMs || std::uint64_t(child.iStartMs) + child.iDurationMs > 600000u ||
                !ParseNumber(fields[7], loop) || loop > 1u)
            { m_strStatus = "Retained Parent child row is invalid"; return false; }
            const auto owners = m_BossPatterns.find(std::string(fields[1]));
            if (owners == m_BossPatterns.end()) { m_strStatus = "Parent encounter is missing"; return false; }
            const auto owner = std::find_if(owners->second.begin(), owners->second.end(), [&](const auto& row) { return row.strPatternId == fields[2]; });
            if (owner == owners->second.end() || owner->ParentChildren.size() >= 128u ||
                (loop && !owner->strParentLoopStartOccurrenceId.empty()) ||
                std::any_of(owner->ParentChildren.begin(), owner->ParentChildren.end(), [&](const auto& row) { return row.strOccurrenceId == fields[3]; }) ||
                (!owner->ParentChildren.empty() && std::uint64_t(owner->ParentChildren.back().iStartMs) + owner->ParentChildren.back().iDurationMs > child.iStartMs))
            { m_strStatus = "Parent child owner, order or loop is invalid"; return false; }
            child.strOccurrenceId = fields[3]; child.strPatternId = fields[4];
            if (loop) owner->strParentLoopStartOccurrenceId = child.strOccurrenceId;
            owner->ParentChildren.push_back(std::move(child));
        }
		else if (!fields.empty() && "PATTERNTIMELINE" == fields[0])
        {
            std::uint32_t lifetime = 0u;
            if (fields.size() != 4u || fields[1] != "ENCOUNTER_KOUKUSAYDON_G1" || !IsStableId(fields[2]) ||
                !ParseNumber(fields[3], lifetime) || lifetime == 0u || lifetime > 600000u)
            { m_strStatus = "Kouku row timeline lifetime is invalid"; return false; }
            const auto owners = m_BossPatterns.find(std::string(fields[1]));
            if (owners == m_BossPatterns.end())
            { m_strStatus = "Kouku row timeline encounter is missing"; return false; }
            const auto owner = std::find_if(owners->second.begin(), owners->second.end(),
                [&](const auto& value) { return value.strPatternId == fields[2]; });
            if (owner == owners->second.end() || owner->iTimelineDurationMs)
            { m_strStatus = "Kouku row timeline owner is missing or duplicated"; return false; }
            owner->iTimelineDurationMs = lifetime;
        }
		else if (!fields.empty() && "PATTERNFIXEDTIMELINE" == fields[0])
		{
			if (fields.size() != 3u || !IsStableId(fields[1]) || !IsStableId(fields[2]))
			{ m_strStatus = "Boss fixed timeline row is invalid"; return false; }
			const auto owners = m_BossPatterns.find(std::string(fields[1]));
			if (owners == m_BossPatterns.end())
			{ m_strStatus = "Boss fixed timeline encounter is missing"; return false; }
			const auto owner = std::find_if(owners->second.begin(), owners->second.end(),
				[&](const auto& value) { return value.strPatternId == fields[2]; });
			if (owner == owners->second.end() || owner->bFixedTimelineClock)
			{ m_strStatus = "Boss fixed timeline pattern is missing or duplicated"; return false; }
			owner->bFixedTimelineClock = true;
		}
		else if (!fields.empty() && "PATTERNBOSSMOTION" == fields[0])
		{
			BOSS_PATTERN_BOSS_MOTION motion;
			if (fields.size() != 12u || !IsStableId(fields[1]) || !IsStableId(fields[2]) ||
				!ParseNumber(fields[3], motion.iStartMs) || !ParseNumber(fields[4], motion.iEndMs) ||
				motion.iStartMs >= motion.iEndMs || motion.iEndMs > 600000u ||
				!ParseNumber(fields[11], motion.fYawDegrees) || !std::isfinite(motion.fYawDegrees) ||
				std::abs(motion.fYawDegrees) > 360.f)
			{ m_strStatus = "Boss Motion row has invalid timing or yaw"; return false; }
			for (std::size_t axis = 0u; axis < 3u; ++axis)
				if (!ParseNumber(fields[5u + axis], motion.StartPosition[axis]) ||
					!ParseNumber(fields[8u + axis], motion.EndPosition[axis]) ||
					!std::isfinite(motion.StartPosition[axis]) || !std::isfinite(motion.EndPosition[axis]) ||
					std::abs(motion.StartPosition[axis]) > 100000.f || std::abs(motion.EndPosition[axis]) > 100000.f)
				{ m_strStatus = "Boss Motion position is invalid"; return false; }
			const auto owners = m_BossPatterns.find(std::string(fields[1]));
			if (owners == m_BossPatterns.end()) return false;
			const auto owner = std::find_if(owners->second.begin(), owners->second.end(),
				[&fields](const BOSS_PATTERN_DEFINITION& pattern) { return pattern.strPatternId == fields[2]; });
			if (owner == owners->second.end() || owner->BossMotion || owner->bResetBossToSpawn ||
				owner->strEncounterId != "ENCOUNTER_KOUKUSAYDON_G1") return false;
			owner->BossMotion = motion;
		}
		else if (!fields.empty() && "PATTERNBOSSMOTIONKEY" == fields[0])
		{
			std::uint32_t index = 0u;
			BOSS_PATTERN_BOSS_MOTION_KEY key;
			if (fields.size() != 8u || !IsStableId(fields[1]) || !IsStableId(fields[2]) ||
				!ParseNumber(fields[3], index) || index >= 512u || !ParseNumber(fields[4], key.iTimeMs)) return false;
			for (std::size_t axis = 0u; axis < key.Position.size(); ++axis)
				if (!ParseNumber(fields[5u + axis], key.Position[axis]) || !std::isfinite(key.Position[axis]) || std::abs(key.Position[axis]) > 100000.f)
				{ m_strStatus = "Boss Motion key position is invalid"; return false; }
			const auto owners = m_BossPatterns.find(std::string(fields[1]));
			if (owners == m_BossPatterns.end()) return false;
			const auto owner = std::find_if(owners->second.begin(), owners->second.end(),
				[&fields](const auto& pattern) { return pattern.strPatternId == fields[2]; });
			if (owner == owners->second.end() || !owner->BossMotion) return false;
			auto& motion = *owner->BossMotion;
			if (motion.Keys.size() != index || key.iTimeMs < motion.iStartMs || key.iTimeMs > motion.iEndMs ||
				(!motion.Keys.empty() && key.iTimeMs <= motion.Keys.back().iTimeMs))
			{ m_strStatus = "Boss Motion keys must follow their owner in strictly increasing order"; return false; }
			motion.Keys.push_back(key);
		}
		else if (!fields.empty() && "PATTERNSPAWNRESET" == fields[0])
		{
			if ((4u != fields.size() && 5u != fields.size()) || fields[3] != "1")
			{
				m_strStatus = "Boss pattern spawn-reset row is invalid";
				return false;
			}
			const auto owners = m_BossPatterns.find(std::string(fields[1]));
			if (owners == m_BossPatterns.end())
				return false;
			const auto owner = std::find_if(owners->second.begin(), owners->second.end(),
				[&fields](const BOSS_PATTERN_DEFINITION& pattern) { return pattern.strPatternId == fields[2]; });
			if (owner == owners->second.end() || owner->bResetBossToSpawn || owner->BossMotion)
				return false;
			if (5u == fields.size())
			{
				float yaw = 0.f;
				if (!ParseNumber(fields[4], yaw) || !std::isfinite(yaw) || std::abs(yaw) > 360.f)
				{
					m_strStatus = "Boss pattern spawn-reset yaw is invalid";
					return false;
				}
				owner->ResetBossYawDegrees = yaw;
			}
			owner->bResetBossToSpawn = true;
		}
		else if (!fields.empty() && "PATTERNWORLDSEQUENCE" == fields[0])
		{
			BOSS_PATTERN_WORLD_SEQUENCE sequence{};
			if ((6u != fields.size() && 9u != fields.size() && 13u != fields.size() && 14u != fields.size() && 15u != fields.size()) || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) || !ParseNumber(fields[3], sequence.iStartMs) ||
				!IsStableId(fields[4]) ||
				!ParseNumber(fields[5], sequence.fPlaybackSpeed) ||
				!std::isfinite(sequence.fPlaybackSpeed) ||
				sequence.fPlaybackSpeed < 0.05f || sequence.fPlaybackSpeed > 16.f)
			{
				m_strStatus = "Boss pattern world sequence row is invalid";
				return false;
			}
			if (9u <= fields.size() &&
				(!ParseNumber(fields[6], sequence.fPositionOffsetX) || !std::isfinite(sequence.fPositionOffsetX) ||
				 !ParseNumber(fields[7], sequence.fPositionOffsetY) || !std::isfinite(sequence.fPositionOffsetY) ||
				 !ParseNumber(fields[8], sequence.fPositionOffsetZ) || !std::isfinite(sequence.fPositionOffsetZ)))
			{
				m_strStatus = "Boss pattern world sequence position offset is invalid";
				return false;
			}
			if (13u <= fields.size())
			{
				if ((fields[9] != "NONE" && fields[9] != "BOSS_SPAWN") ||
					!ParseNumber(fields[10], sequence.fAnchorPositionX) || !std::isfinite(sequence.fAnchorPositionX) ||
					!ParseNumber(fields[11], sequence.fAnchorPositionY) || !std::isfinite(sequence.fAnchorPositionY) ||
					!ParseNumber(fields[12], sequence.fAnchorPositionZ) || !std::isfinite(sequence.fAnchorPositionZ))
				{
					m_strStatus = "Boss pattern world sequence anchor is invalid";
					return false;
				}
				sequence.bAnchorBossSpawn = fields[9] == "BOSS_SPAWN";
			}
			if (14u <= fields.size() && (!ParseNumber(fields[13], sequence.iDurationMs) ||
				sequence.iDurationMs == 0u || sequence.iDurationMs > 600000u))
			{ m_strStatus = "Boss pattern world sequence lifetime is invalid"; return false; }
			if (15u == fields.size())
			{
				if (!IsStableId(fields[14])) { m_strStatus = "World source occurrence ID is invalid"; return false; }
				sequence.strOccurrenceId = fields[14];
			}
			sequence.strInstanceId = std::string(fields[4]);
			const auto owners = m_BossPatterns.find(std::string(fields[1]));
			if (m_BossPatterns.end() == owners)
			{
				m_strStatus = "Boss pattern lane owner encounter is missing";
				return false;
			}
			const auto owner = std::find_if(owners->second.begin(), owners->second.end(),
					[&fields](const BOSS_PATTERN_DEFINITION& pattern)
					{ return pattern.strPatternId == fields[2]; });
			if (m_BossPatterns.end() == owners || owners->second.end() == owner ||
				owner->WorldSequences.size() >= 128u || (!sequence.strOccurrenceId.empty() &&
					std::any_of(owner->WorldSequences.begin(), owner->WorldSequences.end(), [&](const auto& row) {
						return row.strOccurrenceId == sequence.strOccurrenceId; })))
			{
				m_strStatus = "Boss pattern world sequence owner is missing, over capacity, or repeats an occurrence ID";
				return false;
			}
			owner->WorldSequences.push_back(std::move(sequence));
		}
		else if (!fields.empty() && "PATTERNWORLDAUTHOREDMADNESS" == fields[0])
		{
			if (fields.size() != 4u || !IsStableId(fields[1]) || !IsStableId(fields[2]) || !IsStableId(fields[3])) return false;
			const auto encounter = m_BossPatterns.find(std::string(fields[1]));
			if (encounter == m_BossPatterns.end()) return false;
			const auto pattern = std::find_if(encounter->second.begin(), encounter->second.end(), [&](const auto& row) { return row.strPatternId == fields[2]; });
			if (pattern == encounter->second.end()) return false;
			const auto cue = std::find_if(pattern->WorldSequences.begin(), pattern->WorldSequences.end(), [&](const auto& row) { return row.strOccurrenceId == fields[3]; });
			if (cue == pattern->WorldSequences.end() || cue->bAuthoredMadness || !cue->iDurationMs) return false;
			cue->bAuthoredMadness = true;
		}
		else if (!fields.empty() && "PATTERNWORLDPLACEMENT" == fields[0])
		{
			BOSS_PATTERN_WORLD_PLACEMENT placement;
			if (fields.size() != 13u || !IsStableId(fields[1]) || !IsStableId(fields[2]) || !IsStableId(fields[3]))
			{ m_strStatus = "World placement row width or occurrence identity is invalid"; return false; }
			float* values[] = { &placement.fPositionX, &placement.fPositionY, &placement.fPositionZ,
				&placement.fRotationXDegrees, &placement.fRotationYDegrees, &placement.fRotationZDegrees,
				&placement.fScaleX, &placement.fScaleY, &placement.fScaleZ };
			for (std::size_t index = 0u; index < 9u; ++index)
				if (!ParseNumber(fields[index + 4u], *values[index]) || !std::isfinite(*values[index]) ||
					(index < 3u ? std::abs(*values[index]) > 100000.f : index < 6u ? std::abs(*values[index]) > 36000.f :
					 *values[index] < .001f || *values[index] > 1000.f))
				{ m_strStatus = "World placement transform is invalid or out of range"; return false; }
			const auto owners = m_BossPatterns.find(std::string(fields[1]));
			if (owners == m_BossPatterns.end()) { m_strStatus = "World placement encounter is missing"; return false; }
			const auto pattern = std::find_if(owners->second.begin(), owners->second.end(), [&](const auto& row) { return row.strPatternId == fields[2]; });
			if (pattern == owners->second.end()) { m_strStatus = "World placement pattern is missing"; return false; }
			const auto cue = std::find_if(pattern->WorldSequences.begin(), pattern->WorldSequences.end(), [&](const auto& row) { return row.strOccurrenceId == fields[3]; });
			if (cue == pattern->WorldSequences.end() || cue->Placement || !cue->SupportWindows.empty() || cue->bAnchorBossSpawn ||
				cue->fPositionOffsetX != 0.f || cue->fPositionOffsetY != 0.f || cue->fPositionOffsetZ != 0.f ||
				cue->fAnchorPositionX != 0.f || cue->fAnchorPositionY != 0.f || cue->fAnchorPositionZ != 0.f)
			{ m_strStatus = "World placement needs one unplaced occurrence with neutral legacy offsets and no support surface"; return false; }
			cue->Placement = placement;
		}
		else if (!fields.empty() && "PATTERNWORLDCOMBAT" == fields[0])
		{
			BOSS_PATTERN_WORLD_COMBAT_BODY body;
			if (fields.size() != 9u || !IsStableId(fields[1]) || !IsStableId(fields[2]) || !IsStableId(fields[3]) ||
				!ParseNumber(fields[4], body.iMaximumHp) || !body.iMaximumHp || body.iMaximumHp > 1000000000u ||
				!ParseNumber(fields[5], body.fCenterX) || !ParseNumber(fields[6], body.fCenterY) || !ParseNumber(fields[7], body.fCenterZ) ||
				!ParseNumber(fields[8], body.fRadiusM) || !std::isfinite(body.fCenterX) || !std::isfinite(body.fCenterY) ||
				!std::isfinite(body.fCenterZ) || !std::isfinite(body.fRadiusM) || std::abs(body.fCenterX) > 100000.f ||
				std::abs(body.fCenterY) > 100000.f || std::abs(body.fCenterZ) > 100000.f || body.fRadiusM <= .001f || body.fRadiusM > 1000.f)
			{ m_strStatus = "World combat body row is invalid"; return false; }
			const auto owners = m_BossPatterns.find(std::string(fields[1]));
			if (owners == m_BossPatterns.end()) { m_strStatus = "World combat encounter is missing"; return false; }
			const auto pattern = std::find_if(owners->second.begin(), owners->second.end(), [&](const auto& row) { return row.strPatternId == fields[2]; });
			if (pattern == owners->second.end()) { m_strStatus = "World combat pattern is missing"; return false; }
			const auto cue = std::find_if(pattern->WorldSequences.begin(), pattern->WorldSequences.end(), [&](const auto& row) { return row.strOccurrenceId == fields[3]; });
			if (cue == pattern->WorldSequences.end() || cue->CombatBody || cue->bAnchorBossSpawn || !cue->SupportWindows.empty())
			{ m_strStatus = "World combat body requires one stationary occurrence without a support surface"; return false; }
			cue->CombatBody = body;
		}
		else if (!fields.empty() && "PATTERNWORLDSUPPORT" == fields[0])
		{
			BOSS_PATTERN_WORLD_SUPPORT_WINDOW support;
			if (fields.size() != 10u || !IsStableId(fields[1]) || !IsStableId(fields[2]) || !IsStableId(fields[3]) ||
				!ParseNumber(fields[4], support.iStartOffsetTicks) || !ParseNumber(fields[5], support.iEndOffsetTicks) ||
				support.iStartOffsetTicks >= support.iEndOffsetTicks || support.iEndOffsetTicks > 18000u ||
				!ParseNumber(fields[6], support.fCenterX) || !ParseNumber(fields[7], support.fCenterZ) ||
				!ParseNumber(fields[8], support.fHeightY) || !ParseNumber(fields[9], support.fRadiusM) ||
				!std::isfinite(support.fCenterX) || !std::isfinite(support.fCenterZ) || !std::isfinite(support.fHeightY) ||
				std::abs(support.fCenterX) > 100000.f || std::abs(support.fCenterZ) > 100000.f || std::abs(support.fHeightY) > 100000.f ||
				!std::isfinite(support.fRadiusM) || support.fRadiusM <= 0.f || support.fRadiusM > 1000.f)
			{ m_strStatus = "World support surface row is invalid"; return false; }
			const auto owners = m_BossPatterns.find(std::string(fields[1]));
			if (owners == m_BossPatterns.end()) { m_strStatus = "World support encounter missing"; return false; }
			const auto pattern = std::find_if(owners->second.begin(), owners->second.end(),
				[&](const auto& row) { return row.strPatternId == fields[2]; });
			if (pattern == owners->second.end()) { m_strStatus = "World support pattern missing"; return false; }
			const auto cue = std::find_if(pattern->WorldSequences.begin(), pattern->WorldSequences.end(),
				[&](const auto& row) { return row.strOccurrenceId == fields[3]; });
			if (cue == pattern->WorldSequences.end() || cue->Placement || cue->CombatBody || cue->SupportWindows.size() >= 32u ||
				support.iEndOffsetTicks > (static_cast<std::uint64_t>(cue->iDurationMs) * 30u + 999u) / 1000u ||
				(!cue->SupportWindows.empty() && support.iStartOffsetTicks < cue->SupportWindows.back().iEndOffsetTicks))
			{ m_strStatus = "World support cue missing, windows overlap or exceed cue lifetime"; return false; }
			cue->SupportWindows.push_back(support);
		}
		else if (!fields.empty() && "PATTERNSCENEPROFILE" == fields[0])
		{
			BOSS_PATTERN_SCENE_PROFILE profile{};
			if (7u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) || !ParseNumber(fields[3], profile.iStartMs) ||
				!ParseNumber(fields[4], profile.iDurationMs) ||
				!ParseNumber(fields[5], profile.iBlendMs) || profile.iBlendMs > 600000u ||
				!IsStableId(fields[6]))
			{
				m_strStatus = "Boss pattern scene profile row is invalid";
				return false;
			}
			profile.strProfileId = std::string(fields[6]);
			const auto owners = m_BossPatterns.find(std::string(fields[1]));
			if (m_BossPatterns.end() == owners)
			{
				m_strStatus = "Boss pattern lane owner encounter is missing";
				return false;
			}
			const auto owner = std::find_if(owners->second.begin(), owners->second.end(),
					[&fields](const BOSS_PATTERN_DEFINITION& pattern)
					{ return pattern.strPatternId == fields[2]; });
			if (m_BossPatterns.end() == owners || owners->second.end() == owner ||
				owner->SceneProfiles.size() >= 16u)
			{
				m_strStatus = "Boss pattern scene profile owner is missing or over capacity";
				return false;
			}
			owner->SceneProfiles.push_back(std::move(profile));
		}
		else if (!fields.empty() && "KOUKUMADNESS" == fields[0])
		{
			BOSS_ENCOUNTER_MADNESS_POLICY policy{};
			if ((4u != fields.size() && 12u != fields.size()) || !IsStableId(fields[1]) ||
				!ParseNumber(fields[2], policy.iMaximum) || 0u == policy.iMaximum ||
				policy.iMaximum > 1000000u ||
				!ParseNumber(fields[3], policy.iClownHoldMs) ||
				policy.iClownHoldMs > 600000u ||
				(12u == fields.size() &&
				 (!ParseNumber(fields[4], policy.iDamageGainPercent) || policy.iDamageGainPercent > 10000u ||
				  !ParseNumber(fields[5], policy.iBallGainPercent) || policy.iBallGainPercent > 100u ||
				  !ParseNumber(fields[6], policy.iBallMultiplierPercent) || policy.iBallMultiplierPercent > 10000u ||
				  !ParseNumber(fields[7], policy.fBallRadiusM) || !std::isfinite(policy.fBallRadiusM) || policy.fBallRadiusM <= 0.f || policy.fBallRadiusM > 20.f ||
				  !ParseNumber(fields[8], policy.iDollGainPercent) || policy.iDollGainPercent > 100u ||
				  !ParseNumber(fields[9], policy.iDollMultiplierPercent) || policy.iDollMultiplierPercent > 10000u ||
				  !ParseNumber(fields[10], policy.fDollRadiusM) || !std::isfinite(policy.fDollRadiusM) || policy.fDollRadiusM <= 0.f || policy.fDollRadiusM > 20.f ||
				  !ParseNumber(fields[11], policy.iSpecialIntervalMs) || policy.iSpecialIntervalMs < 100u || policy.iSpecialIntervalMs > 60000u)) ||
				!m_KoukuMadnessPolicies.emplace(std::string(fields[1]), policy).second)
			{
				m_strStatus = "KoukuSaydon madness policy row is invalid or duplicated";
				return false;
			}
		}
		else if (!fields.empty() && "PATTERNVERTICALOFFSET" == fields[0])
		{
			float verticalOffsetM = 0.f;
			if (4u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) ||
				!ParseNumber(fields[3], verticalOffsetM) ||
				!std::isfinite(verticalOffsetM) || 0.f == verticalOffsetM ||
				std::fabs(verticalOffsetM) > 100.f)
			{
				m_strStatus = "Boss pattern vertical-offset row is invalid";
				return false;
			}
			const auto ownerMap = m_BossPatterns.find(std::string(fields[1]));
			const std::string ownerKey =
				std::string(fields[1]) + "\n" + std::string(fields[2]);
			if (m_BossPatterns.end() == ownerMap ||
				!patternVerticalOffsetOwners.insert(ownerKey).second)
			{
				m_strStatus =
					"Boss pattern vertical-offset owner is missing or duplicated";
				return false;
			}
			const auto owner = std::find_if(
				ownerMap->second.begin(), ownerMap->second.end(),
				[&fields](const BOSS_PATTERN_DEFINITION& pattern)
				{ return pattern.strPatternId == fields[2]; });
			if (ownerMap->second.end() == owner || 0.f != owner->fVerticalOffsetM)
			{
				m_strStatus =
					"Boss pattern vertical-offset has no pattern owner or is duplicated";
				return false;
			}
			owner->fVerticalOffsetM = verticalOffsetM;
		}
		else if (!fields.empty() && "PATTERNSTAGEVERTICALOFFSET" == fields[0])
		{
			float verticalOffsetM = 0.f;
			if (5u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) || !IsStableId(fields[3]) ||
				!ParseNumber(fields[4], verticalOffsetM) ||
				!std::isfinite(verticalOffsetM) || 0.f == verticalOffsetM ||
				std::fabs(verticalOffsetM) > 100.f)
			{
				m_strStatus = "Boss pattern stage vertical-offset row is invalid";
				return false;
			}
			const auto ownerMap = m_BossPatterns.find(std::string(fields[1]));
			const std::string ownerKey = std::string(fields[1]) + "\n" +
				std::string(fields[2]) + "\n" + std::string(fields[3]);
			if (m_BossPatterns.end() == ownerMap ||
				!patternStageVerticalOffsetOwners.insert(ownerKey).second)
			{
				m_strStatus =
					"Boss pattern stage vertical-offset owner is missing or duplicated";
				return false;
			}
			const auto owner = std::find_if(
				ownerMap->second.begin(), ownerMap->second.end(),
				[&fields](const BOSS_PATTERN_DEFINITION& pattern)
				{ return pattern.strPatternId == fields[2]; });
			if (ownerMap->second.end() == owner)
			{
				m_strStatus =
					"Boss pattern stage vertical-offset has no pattern owner";
				return false;
			}
			const auto stage = std::find_if(
				owner->Stages.begin(), owner->Stages.end(),
				[&fields](const BOSS_PATTERN_STAGE_DEFINITION& candidate)
				{ return candidate.strActionId == fields[3]; });
			if (owner->Stages.end() == stage || 0.f != stage->fVerticalOffsetM)
			{
				m_strStatus =
					"Boss pattern stage vertical-offset has no stage owner or is duplicated";
				return false;
			}
			stage->fVerticalOffsetM = verticalOffsetM;
		}
		else if (!fields.empty() && "PATTERNPOLICY" == fields[0])
		{
			BOSS_PATTERN_CATEGORY category = BOSS_PATTERN_CATEGORY::NORMAL;
			BOSS_PATTERN_TARGET_POLICY targetPolicy =
				BOSS_PATTERN_TARGET_POLICY::NONE;
			BOSS_PATTERN_AIM_POLICY aimPolicy = BOSS_PATTERN_AIM_POLICY::NONE;
			std::uint32_t minimumPhase = 0;
			std::uint32_t maximumPhase = 0;
			if (8u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) ||
				!ParseBossPatternCategory(fields[3], category) ||
				!ParseNumber(fields[4], minimumPhase) ||
				!ParseNumber(fields[5], maximumPhase) ||
				!ParseBossPatternTargetPolicy(fields[6], targetPolicy) ||
				!ParseBossPatternAimPolicy(fields[7], aimPolicy) ||
				0u == minimumPhase || maximumPhase < minimumPhase ||
				maximumPhase > 3u)
			{
				m_strStatus = "Boss pattern policy row is invalid";
				return false;
			}
			const auto ownerMap = m_BossPatterns.find(std::string(fields[1]));
			if (m_BossPatterns.end() == ownerMap)
			{
				m_strStatus = "Boss pattern policy has no encounter";
				return false;
			}
			const auto owner = std::find_if(
				ownerMap->second.begin(), ownerMap->second.end(),
				[&fields](const BOSS_PATTERN_DEFINITION& pattern)
				{ return pattern.strPatternId == fields[2]; });
			const std::string ownerKey =
				std::string(fields[1]) + "\n" + std::string(fields[2]);
			const bool targetNone =
				BOSS_PATTERN_TARGET_POLICY::NONE == targetPolicy;
			const bool aimNeedsNoTarget = BOSS_PATTERN_AIM_POLICY::NONE == aimPolicy ||
				BOSS_PATTERN_AIM_POLICY::FACE_MOTION_ANCHOR == aimPolicy;
			const bool targetTracks =
				BOSS_PATTERN_TARGET_POLICY::NEAREST_EACH_TICK == targetPolicy;
			const bool aimTracks =
				BOSS_PATTERN_AIM_POLICY::TRACK_TARGET_EACH_TICK == aimPolicy;
			const bool targetLocks =
				BOSS_PATTERN_TARGET_POLICY::LOCK_NEAREST_ON_START == targetPolicy ||
				BOSS_PATTERN_TARGET_POLICY::LOCK_RANDOM_ALIVE_ON_START == targetPolicy ||
				BOSS_PATTERN_TARGET_POLICY::LOCK_RANDOM_ALIVE_BEHIND_ON_START ==
					targetPolicy;
			const bool aimLocks =
				BOSS_PATTERN_AIM_POLICY::LOCK_FACING_ON_START == aimPolicy;
			const bool policyIsCoherent =
				(targetNone && aimNeedsNoTarget) ||
				(targetTracks && aimTracks) ||
				(targetLocks && (aimLocks || aimTracks));
			if (ownerMap->second.end() == owner ||
				!patternPolicyOwners.insert(ownerKey).second ||
				!policyIsCoherent)
			{
				m_strStatus =
					"Boss pattern policy has no owner, is duplicated, or is incoherent";
				return false;
			}
			owner->eCategory = category;
			owner->eTargetPolicy = targetPolicy;
			owner->eAimPolicy = aimPolicy;
			owner->iMinimumPhase = minimumPhase;
			owner->iMaximumPhase = maximumPhase;
		}
		else if (!fields.empty() && "PATTERNSOURCE" == fields[0])
		{
			std::uint32_t primaryActionId = 0u;
			std::uint32_t shapeCount = 0u;
			std::uint32_t cooldownMs = 0u;
			std::uint32_t cooldownTicks = 0u;
			std::uint32_t rangeUnits = 0u;
			std::uint32_t approachUnits = 0u;
			std::uint32_t turnDegrees = 0u;
			if (10u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) ||
				!ParseNumber(fields[3], primaryActionId) ||
				!ParseNumber(fields[4], shapeCount) ||
				!ParseNumber(fields[5], cooldownMs) ||
				!ParseNumber(fields[6], cooldownTicks) ||
				!ParseNumber(fields[7], rangeUnits) ||
				!ParseNumber(fields[8], approachUnits) ||
				!ParseNumber(fields[9], turnDegrees) ||
				(0u == primaryActionId && "ENCOUNTER_KOUKUSAYDON_G1" != fields[1]) ||
				shapeCount > 256u ||
				cooldownMs > 600000u || rangeUnits > 100000u ||
				approachUnits > 100000u || turnDegrees > 360u ||
				cooldownTicks != static_cast<std::uint32_t>(
					(static_cast<std::uint64_t>(cooldownMs) * 30u + 999u) /
					1000u))
			{
				m_strStatus = "Boss pattern source timing row is invalid";
				return false;
			}
			auto ownerMap = m_BossPatterns.find(std::string(fields[1]));
			if (m_BossPatterns.end() == ownerMap)
			{
				m_strStatus = "Boss pattern source timing has no encounter";
				return false;
			}
			const auto owner = std::find_if(
				ownerMap->second.begin(), ownerMap->second.end(),
				[&fields](const BOSS_PATTERN_DEFINITION& pattern)
				{ return pattern.strPatternId == fields[2]; });
			const std::string sourceOwnerKey =
				std::string(fields[1]) + "\n" + std::string(fields[2]);
			if (ownerMap->second.end() == owner ||
				!patternSourceOwners.insert(sourceOwnerKey).second)
			{
				m_strStatus =
					"Boss pattern source timing has no owner or is duplicated";
				return false;
			}
			owner->iSourcePrimaryActionId = primaryActionId;
			owner->iSourceShapeCount = shapeCount;
			owner->iSourceCooldownMs = cooldownMs;
			owner->iSourceCooldownTicks = cooldownTicks;
			owner->iSourceRangeUnits = rangeUnits;
			owner->iSourceApproachUnits = approachUnits;
			owner->iSourceTurnDegrees = turnDegrees;
		}
		else if (!fields.empty() && "ENCOUNTERINTRO" == fields[0])
		{
			if (3u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]))
			{
				m_strStatus = "Encounter intro row is invalid";
				return false;
			}
			const std::string introEncounterId(fields[1]);
			if (!m_IntroPatternIdByEncounter.emplace(
				introEncounterId, std::string(fields[2])).second)
			{
				m_strStatus = "Duplicate encounter intro row";
				return false;
			}
		}
		else if (!fields.empty() && "PATTERNFINALE" == fields[0])
		{
			BOSS_PATTERN_FINALE finale{};
			if (fields.size() < 9u || fields.size() > 72u ||
				!IsStableId(fields[1]) ||
				!IsStableId(fields[2]) || "GHOST_PORTAL_LOOP" != fields[3] ||
				!IsStableId(fields[4]) ||
				!ParseNumber(fields[5], finale.fSpawnHalfExtentsX) ||
				!ParseNumber(fields[6], finale.fSpawnHalfExtentsZ) ||
				!ParseNumber(fields[7], finale.iMaximumActiveGhosts) ||
				!std::isfinite(finale.fSpawnHalfExtentsX) ||
				!std::isfinite(finale.fSpawnHalfExtentsZ) ||
				finale.fSpawnHalfExtentsX < 1.f || finale.fSpawnHalfExtentsX > 100.f ||
				finale.fSpawnHalfExtentsZ < 1.f || finale.fSpawnHalfExtentsZ > 100.f ||
				finale.iMaximumActiveGhosts < 1u ||
				finale.iMaximumActiveGhosts > 64u)
			{
				m_strStatus = "Boss finale row is invalid";
				return false;
			}
			std::unordered_set<std::string> childPatternIds;
			for (std::size_t index = 8u; index < fields.size(); ++index)
			{
				if (!IsStableId(fields[index]) ||
					!childPatternIds.emplace(fields[index]).second)
				{
					m_strStatus = "Boss finale child pattern identity is invalid or duplicated";
					return false;
				}
				finale.GhostPatternIds.emplace_back(fields[index]);
			}
			const auto ownerMap = m_BossPatterns.find(std::string(fields[1]));
			if (m_BossPatterns.end() == ownerMap)
			{
				m_strStatus = "Boss finale has no encounter owner";
				return false;
			}
			const auto owner = std::find_if(ownerMap->second.begin(),
				ownerMap->second.end(), [&fields](const BOSS_PATTERN_DEFINITION& p)
				{ return p.strPatternId == fields[2]; });
			if (ownerMap->second.end() == owner ||
				BOSS_PATTERN_FINALE_KIND::NONE != owner->Finale.eKind)
			{
				m_strStatus = "Boss finale owner is missing or duplicated";
				return false;
			}
			finale.eKind = BOSS_PATTERN_FINALE_KIND::GHOST_PORTAL_LOOP;
			finale.strGhostArchetypeId = std::string(fields[4]);
			owner->Finale = std::move(finale);
		}
		else if (!fields.empty() && "PATTERNFINALEINTERVAL" == fields[0])
		{
			std::uint32_t auxiliaryMs = 0u, portalMs = 0u;
			if (5u != fields.size() || !IsStableId(fields[1]) || !IsStableId(fields[2]) ||
				!ParseNumber(fields[3], auxiliaryMs) || auxiliaryMs > 600000u ||
				!ParseNumber(fields[4], portalMs) || portalMs > 600000u ||
				!patternFinaleIntervalOwners.emplace(std::string(fields[1]) + ":" + std::string(fields[2])).second)
			{
				m_strStatus = "Boss finale interval row is invalid or duplicated";
				return false;
			}
			const auto ownerMap = m_BossPatterns.find(std::string(fields[1]));
			if (m_BossPatterns.end() == ownerMap)
			{
				m_strStatus = "Boss finale interval has no encounter owner";
				return false;
			}
			const auto owner = std::find_if(ownerMap->second.begin(), ownerMap->second.end(),
				[&fields](const BOSS_PATTERN_DEFINITION& p) { return p.strPatternId == fields[2]; });
			if (ownerMap->second.end() == owner || BOSS_PATTERN_FINALE_KIND::GHOST_PORTAL_LOOP != owner->Finale.eKind)
			{
				m_strStatus = "Boss finale interval has no declared finale owner";
				return false;
			}
			owner->Finale.iAuxiliarySpawnIntervalMs = auxiliaryMs;
			owner->Finale.iPortalSpawnIntervalMs = portalMs;
		}
		else if (!fields.empty() && "PATTERNMOTION" == fields[0])
		{
			BOSS_PATTERN_MOTION motion{};
			if (14u != fields.size() && 15u != fields.size())
			{
				m_strStatus = "Boss pattern motion row is invalid";
				return false;
			}
			const bool leapsToTarget = "LEAP_TO_TARGET" == fields[3];
			if (!IsStableId(fields[1]) ||
				!IsStableId(fields[2]) ||
				("LEAP_TO_ANCHOR" != fields[3] && !leapsToTarget) ||
				!IsStableId(fields[4]) ||
				!ParseNumber(fields[5], motion.fLandingX) ||
				!ParseNumber(fields[6], motion.fLandingY) ||
				!ParseNumber(fields[7], motion.fLandingZ) ||
				!ParseNumber(fields[8], motion.fApexHeight) ||
				!ParseNumber(fields[9], motion.iTravelStageIndex) ||
				!ParseNumber(fields[10], motion.iTakeoffStartMs) ||
				!ParseNumber(fields[11], motion.iTakeoffEndMs) ||
				!ParseNumber(fields[12], motion.iTravelStartMs) ||
				!ParseNumber(fields[13], motion.iTravelEndMs) ||
				!std::isfinite(motion.fLandingX) ||
				!std::isfinite(motion.fLandingY) ||
				!std::isfinite(motion.fLandingZ) ||
				!std::isfinite(motion.fApexHeight) ||
				motion.fApexHeight <= 0.f || motion.fApexHeight > 200.f ||
				0u == motion.iTravelStageIndex ||
				motion.iTakeoffStartMs >= motion.iTakeoffEndMs ||
				motion.iTravelStartMs >= motion.iTravelEndMs)
			{
				m_strStatus = "Boss pattern motion row is invalid";
				return false;
			}
			if (15u == fields.size())
			{
				std::uint32_t moveToAnchor = 0u;
				if (!ParseNumber(fields[14], moveToAnchor) || moveToAnchor > 1u ||
					(1u == moveToAnchor && (leapsToTarget ||
						0u == motion.iTakeoffStartMs)))
				{
					m_strStatus = "Boss pre-takeoff movement is invalid";
					return false;
				}
				motion.bMoveToAnchorBeforeTakeoff = 1u == moveToAnchor;
			}
			motion.eKind = leapsToTarget ?
				BOSS_PATTERN_MOTION_KIND::LEAP_TO_TARGET :
				BOSS_PATTERN_MOTION_KIND::LEAP_TO_ANCHOR;
			motion.strAnchorId = std::string(fields[4]);
			const std::string motionEncounterId(fields[1]);
			auto& motionPatterns = m_BossPatterns[motionEncounterId];
			const auto owner = std::find_if(
				motionPatterns.begin(), motionPatterns.end(),
				[&fields](const BOSS_PATTERN_DEFINITION& candidate)
				{
					return candidate.strPatternId == fields[2];
				});
			if (owner == motionPatterns.end() ||
				BOSS_PATTERN_MOTION_KIND::NONE != owner->Motion.eKind)
			{
				m_strStatus =
					"Boss pattern motion has no owner or is duplicated";
				return false;
			}
			owner->Motion = std::move(motion);
		}
		else if (!fields.empty() && "PATTERNSTAGE" == fields[0])
		{
			BOSS_PATTERN_STAGE_DEFINITION stage{};
			std::uint32_t stageIndex = 0u;
			std::uint32_t knockdownFlag = 0u;
			const bool hasPlayerResponseFields = 24u == fields.size();
			if ((22u != fields.size() && !hasPlayerResponseFields) ||
				!IsStableId(fields[1]) ||
				!IsStableId(fields[2]) ||
				!ParseNumber(fields[3], stageIndex) ||
				!IsStableId(fields[4]) || !IsStableId(fields[5]) ||
				!ParseBossPatternStageKind(fields[6], stage.eStageKind) ||
				!ParseNumber(fields[7], stage.iDurationMs) ||
				!ParseBossPatternHitShape(fields[8], stage.eHitShape) ||
				!ParseNumber(fields[9], stage.fHitOuterRadius) ||
				!ParseNumber(fields[10], stage.fHitInnerRadius) ||
				!ParseNumber(fields[11], stage.fHitAngleDegrees) ||
				!ParseNumber(fields[12], stage.fHitLength) ||
				!ParseNumber(fields[13], stage.fHitHalfWidth) ||
				!ParseNumber(fields[14], stage.iHitCount) ||
				!ParseNumber(fields[15], stage.iHitIntervalMs) ||
				!ParseNumber(fields[16], stage.iHitDelayMs) ||
				("-" != fields[17] && !IsStableId(fields[17])) ||
				!ParseNumber(fields[18], stage.fPushRangeM) ||
				!ParseNumber(fields[19], stage.iPushMs) ||
				!ParseNumber(fields[20], knockdownFlag) ||
				!ParseNumber(fields[21], stage.iDownMs) ||
				(hasPlayerResponseFields &&
				 (!ParseBossPatternPlayerResponse(
					fields[22], stage.ePlayerResponse) ||
				  !ParsePlayerAttachmentSlot(fields[23], stage.eAttachmentSlot))) ||
				0u == stage.iDurationMs ||
				(BOSS_PATTERN_HIT_SHAPE::NONE == stage.eHitShape &&
					0u != stage.iHitDelayMs) ||
				stage.iHitDelayMs >= stage.iDurationMs ||
				!std::isfinite(stage.fHitOuterRadius) ||
				!std::isfinite(stage.fHitInnerRadius) ||
				!std::isfinite(stage.fHitAngleDegrees) ||
				!std::isfinite(stage.fHitLength) ||
				!std::isfinite(stage.fHitHalfWidth) ||
				!std::isfinite(stage.fPushRangeM) ||
				std::fabs(stage.fPushRangeM) > 20.f ||
				knockdownFlag > 1u ||
				(0.f != stage.fPushRangeM && 0u == stage.iPushMs) ||
				(0.f == stage.fPushRangeM && 0u != stage.iPushMs) ||
				(1u == knockdownFlag && 0u == stage.iDownMs) ||
				(0u == knockdownFlag && 0u != stage.iDownMs) ||
				(BOSS_PATTERN_PLAYER_RESPONSE::DAMAGE == stage.ePlayerResponse &&
				 LostArk::Shared::PLAYER_ATTACHMENT_SLOT::NONE !=
					stage.eAttachmentSlot) ||
				(BOSS_PATTERN_PLAYER_RESPONSE::CAPTURE == stage.ePlayerResponse &&
				 (LostArk::Shared::PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND !=
					stage.eAttachmentSlot ||
				  BOSS_PATTERN_HIT_SHAPE::NONE == stage.eHitShape ||
				  0.f != stage.fPushRangeM || 0u != stage.iPushMs ||
				  0u != knockdownFlag || 0u != stage.iDownMs)))
			{
				m_strStatus = "Boss pattern stage row is invalid";
				return false;
			}
			stage.bKnockdown = 1u == knockdownFlag;
			const auto ownerMap = m_BossPatterns.find(std::string(fields[1]));
			if (m_BossPatterns.end() == ownerMap)
			{
				m_strStatus = "Boss pattern stage has no encounter";
				return false;
			}
			const auto owner = std::find_if(
				ownerMap->second.begin(), ownerMap->second.end(),
				[&fields](const BOSS_PATTERN_DEFINITION& pattern)
				{ return pattern.strPatternId == fields[2]; });
			if (ownerMap->second.end() == owner ||
				stageIndex != owner->Stages.size() ||
				stageIndex >= owner->iExpectedStageCount)
			{
				m_strStatus = "Boss pattern stage does not follow its pattern";
				return false;
			}
			stage.strStageId = fields[4];
			stage.strActionId = fields[5];
			stage.strDamageProfileId =
				"-" == fields[17] ? "" : std::string(fields[17]);
			owner->Stages.push_back(std::move(stage));
		}
		else if (!fields.empty() && "PATTERNSTAGEHITOFFSET" == fields[0])
		{
			std::uint32_t hitIndex = 0u;
			std::uint32_t hitOffsetMs = 0u;
			if (6u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) || !IsStableId(fields[3]) ||
				!ParseNumber(fields[4], hitIndex) ||
				!ParseNumber(fields[5], hitOffsetMs))
			{
				m_strStatus = "Boss pattern stage hit offset row is invalid";
				return false;
			}
			const auto ownerMap = m_BossPatterns.find(std::string(fields[1]));
			if (m_BossPatterns.end() == ownerMap)
			{
				m_strStatus = "Boss pattern stage hit offset has no encounter";
				return false;
			}
			const auto owner = std::find_if(
				ownerMap->second.begin(), ownerMap->second.end(),
				[&fields](const BOSS_PATTERN_DEFINITION& pattern)
				{ return pattern.strPatternId == fields[2]; });
			if (ownerMap->second.end() == owner)
			{
				m_strStatus = "Boss pattern stage hit offset has no pattern owner";
				return false;
			}
			const auto stage = std::find_if(
				owner->Stages.begin(), owner->Stages.end(),
				[&fields](const BOSS_PATTERN_STAGE_DEFINITION& candidate)
				{ return candidate.strActionId == fields[3]; });
			if (owner->Stages.end() == stage ||
				hitIndex != stage->HitOffsetsMs.size() ||
				hitIndex >= stage->iHitCount ||
				hitOffsetMs >= stage->iDurationMs ||
				(!stage->HitOffsetsMs.empty() &&
				 hitOffsetMs <= stage->HitOffsetsMs.back()))
			{
				m_strStatus =
					"Boss pattern stage hit offsets are not ordered or owned";
				return false;
			}
			stage->HitOffsetsMs.push_back(hitOffsetMs);
		}
		else if (!fields.empty() && "PATTERNSTAGEHITAUTHORITY" == fields[0])
		{
			BOSS_PATTERN_HIT_ANCHOR_KIND anchorKind =
				BOSS_PATTERN_HIT_ANCHOR_KIND::BOSS_CURRENT;
			BOSS_PATTERN_HIT_ACTIVATION_KIND activationKind =
				BOSS_PATTERN_HIT_ACTIVATION_KIND::PULSE_SCHEDULE;
			float forwardOffsetM = 0.f;
			float rightOffsetM = 0.f;
			float yawOffsetDegrees = 0.f;
			std::uint32_t startMs = 0u;
			std::uint32_t lifetimeMs = 0u;
			if (12u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) || !IsStableId(fields[3]) ||
				!ParseBossPatternHitAnchorKind(fields[4], anchorKind) ||
				!ParseNumber(fields[5], forwardOffsetM) ||
				!ParseNumber(fields[6], rightOffsetM) ||
				!ParseNumber(fields[7], yawOffsetDegrees) ||
				!ParseBossPatternHitActivationKind(fields[8], activationKind) ||
				!ParseNumber(fields[9], startMs) ||
				!ParseNumber(fields[10], lifetimeMs) ||
				((BOSS_PATTERN_HIT_ACTIVATION_KIND::ACTIVE_WINDOW == activationKind &&
				  "ONCE" != fields[11]) ||
				 (BOSS_PATTERN_HIT_ACTIVATION_KIND::PULSE_SCHEDULE == activationKind &&
				  "NONE" != fields[11])) ||
				!std::isfinite(forwardOffsetM) ||
				!std::isfinite(rightOffsetM) ||
				!std::isfinite(yawOffsetDegrees) ||
				std::fabs(forwardOffsetM) > 1000.f ||
				std::fabs(rightOffsetM) > 1000.f ||
				std::fabs(yawOffsetDegrees) > 360.f)
			{
				m_strStatus = "Boss pattern stage hit authority row is invalid";
				return false;
			}
			const auto ownerMap = m_BossPatterns.find(std::string(fields[1]));
			if (m_BossPatterns.end() == ownerMap)
			{
				m_strStatus = "Boss pattern stage hit authority has no encounter";
				return false;
			}
			const auto owner = std::find_if(
				ownerMap->second.begin(), ownerMap->second.end(),
				[&fields](const BOSS_PATTERN_DEFINITION& pattern)
				{ return pattern.strPatternId == fields[2]; });
			if (ownerMap->second.end() == owner)
			{
				m_strStatus = "Boss pattern stage hit authority has no pattern owner";
				return false;
			}
			const auto stage = std::find_if(
				owner->Stages.begin(), owner->Stages.end(),
				[&fields](const BOSS_PATTERN_STAGE_DEFINITION& candidate)
				{ return candidate.strActionId == fields[3]; });
			if (owner->Stages.end() == stage ||
				BOSS_PATTERN_HIT_SHAPE::NONE == stage->eHitShape ||
				BOSS_PATTERN_HIT_ANCHOR_KIND::BOSS_CURRENT != stage->eHitAnchorKind ||
				0.f != stage->fHitAnchorForwardOffsetM ||
				0.f != stage->fHitAnchorRightOffsetM ||
				0.f != stage->fHitAnchorYawOffsetDegrees ||
				BOSS_PATTERN_HIT_ACTIVATION_KIND::PULSE_SCHEDULE !=
					stage->eHitActivationKind ||
				0u != stage->iHitActivationStartMs ||
				0u != stage->iHitActivationLifetimeMs ||
				(BOSS_PATTERN_HIT_ACTIVATION_KIND::ACTIVE_WINDOW == activationKind &&
				 (0u == lifetimeMs ||
				  static_cast<std::uint64_t>(startMs) + lifetimeMs >
					stage->iDurationMs)))
			{
				m_strStatus =
					"Boss pattern stage hit authority is duplicated or not owned";
				return false;
			}
			stage->eHitAnchorKind = anchorKind;
			stage->fHitAnchorForwardOffsetM = forwardOffsetM;
			stage->fHitAnchorRightOffsetM = rightOffsetM;
			stage->fHitAnchorYawOffsetDegrees = yawOffsetDegrees;
			stage->eHitActivationKind = activationKind;
			stage->iHitActivationStartMs = startMs;
			stage->iHitActivationLifetimeMs = lifetimeMs;
		}
		else if (!fields.empty() && "PATTERNSTAGEPARTDAMAGE" == fields[0])
		{
			BOSS_PATTERN_PART_DAMAGE_POLICY policy =
				BOSS_PATTERN_PART_DAMAGE_POLICY::NORMAL;
			if (5u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) || !IsStableId(fields[3]) ||
				!ParseBossPatternPartDamagePolicy(fields[4], policy))
			{
				m_strStatus =
					"Boss pattern stage part-damage row is invalid";
				return false;
			}
			const std::string ownerKey = std::string(fields[1]) + "\n" +
				std::string(fields[2]) + "\n" + std::string(fields[3]);
			const auto ownerMap = m_BossPatterns.find(std::string(fields[1]));
			if (m_BossPatterns.end() == ownerMap ||
				!patternStagePartDamageOwners.insert(ownerKey).second)
			{
				m_strStatus =
					"Boss pattern stage part-damage owner is missing or duplicated";
				return false;
			}
			const auto owner = std::find_if(
				ownerMap->second.begin(), ownerMap->second.end(),
				[&fields](const BOSS_PATTERN_DEFINITION& pattern)
				{ return pattern.strPatternId == fields[2]; });
			if (ownerMap->second.end() == owner)
			{
				m_strStatus =
					"Boss pattern stage part-damage has no pattern owner";
				return false;
			}
			const auto stage = std::find_if(
				owner->Stages.begin(), owner->Stages.end(),
				[&fields](const BOSS_PATTERN_STAGE_DEFINITION& candidate)
				{ return candidate.strActionId == fields[3]; });
			if (owner->Stages.end() == stage)
			{
				m_strStatus =
					"Boss pattern stage part-damage has no stage owner";
				return false;
			}
			stage->ePartDamagePolicy = policy;
		}
		else if (!fields.empty() && "PATTERNSTAGECOUNTERPROXY" == fields[0])
		{
			BOSS_PATTERN_COUNTER_PROXY_KIND kind =
				BOSS_PATTERN_COUNTER_PROXY_KIND::NONE;
			float forwardOffsetM = 0.f;
			float rightOffsetM = 0.f;
			float radiusM = 0.f;
			float arcDegrees = 0.f;
			if (9u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) || !IsStableId(fields[3]) ||
				!ParseBossPatternCounterProxyKind(fields[4], kind) ||
				!ParseNumber(fields[5], forwardOffsetM) ||
				!ParseNumber(fields[6], rightOffsetM) ||
				!ParseNumber(fields[7], radiusM) ||
				!ParseNumber(fields[8], arcDegrees) ||
				!std::isfinite(forwardOffsetM) ||
				!std::isfinite(rightOffsetM) || !std::isfinite(radiusM) ||
				!std::isfinite(arcDegrees))
			{
				m_strStatus = "Boss pattern stage counter-proxy row is invalid";
				return false;
			}
			const bool validCircle =
				BOSS_PATTERN_COUNTER_PROXY_KIND::BOSS_LOCAL_CIRCLE == kind &&
				std::fabs(forwardOffsetM) <= 20.f &&
				std::fabs(rightOffsetM) <= 20.f && radiusM > 0.f &&
				radiusM <= 20.f && 0.f == arcDegrees;
			const bool validForwardArc =
				BOSS_PATTERN_COUNTER_PROXY_KIND::BOSS_FORWARD_ARC == kind &&
				0.f == forwardOffsetM && 0.f == rightOffsetM &&
				0.f == radiusM && 180.f == arcDegrees;
			if (!validCircle && !validForwardArc)
			{
				m_strStatus =
					"Boss pattern stage counter-proxy geometry is invalid";
				return false;
			}
			const std::string ownerKey = std::string(fields[1]) + "\n" +
				std::string(fields[2]) + "\n" + std::string(fields[3]);
			const auto ownerMap = m_BossPatterns.find(std::string(fields[1]));
			if (m_BossPatterns.end() == ownerMap ||
				!patternStageCounterProxyOwners.insert(ownerKey).second)
			{
				m_strStatus =
					"Boss pattern stage counter-proxy owner is missing or duplicated";
				return false;
			}
			const auto owner = std::find_if(
				ownerMap->second.begin(), ownerMap->second.end(),
				[&fields](const BOSS_PATTERN_DEFINITION& pattern)
				{ return pattern.strPatternId == fields[2]; });
			if (ownerMap->second.end() == owner)
			{
				m_strStatus =
					"Boss pattern stage counter-proxy has no pattern owner";
				return false;
			}
			const auto stage = std::find_if(
				owner->Stages.begin(), owner->Stages.end(),
				[&fields](const BOSS_PATTERN_STAGE_DEFINITION& candidate)
				{ return candidate.strActionId == fields[3]; });
			if (owner->Stages.end() == stage || stage->bHasCounterProxy)
			{
				m_strStatus =
					"Boss pattern stage counter-proxy has no stage owner or is duplicated";
				return false;
			}
			stage->bHasCounterProxy = true;
			stage->eCounterProxyKind = kind;
			stage->fCounterProxyForwardOffsetM = forwardOffsetM;
			stage->fCounterProxyRightOffsetM = rightOffsetM;
			stage->fCounterProxyRadiusM = radiusM;
			stage->fCounterProxyArcDegrees = arcDegrees;
		}
		else if (!fields.empty() && "PATTERNSTAGERESPONSE" == fields[0])
		{
			BOSS_PATTERN_BOSS_RESPONSE_KIND kind =
				BOSS_PATTERN_BOSS_RESPONSE_KIND::NONE;
			std::uint32_t threshold = 0u;
			if (6u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) || !IsStableId(fields[3]) ||
				!ParseBossPatternBossResponseKind(fields[4], kind) ||
				!ParseNumber(fields[5], threshold) || 0u == threshold)
			{
				m_strStatus = "Boss pattern stage response row is invalid";
				return false;
			}
			const std::string ownerKey = std::string(fields[1]) + "\n" +
				std::string(fields[2]) + "\n" + std::string(fields[3]);
			const auto ownerMap = m_BossPatterns.find(std::string(fields[1]));
			if (m_BossPatterns.end() == ownerMap ||
				!patternStageBossResponseOwners.insert(ownerKey).second)
			{
				m_strStatus =
					"Boss pattern stage response owner is missing or duplicated";
				return false;
			}
			const auto owner = std::find_if(
				ownerMap->second.begin(), ownerMap->second.end(),
				[&fields](const BOSS_PATTERN_DEFINITION& pattern)
				{ return pattern.strPatternId == fields[2]; });
			if (ownerMap->second.end() == owner)
			{
				m_strStatus = "Boss pattern stage response has no pattern owner";
				return false;
			}
			const auto stage = std::find_if(
				owner->Stages.begin(), owner->Stages.end(),
				[&fields](const BOSS_PATTERN_STAGE_DEFINITION& candidate)
				{ return candidate.strActionId == fields[3]; });
			if (owner->Stages.end() == stage ||
				BOSS_PATTERN_BOSS_RESPONSE_KIND::NONE != stage->eBossResponseKind ||
				0u != stage->iBossResponseThreshold)
			{
				m_strStatus =
					"Boss pattern stage response has no stage owner or is duplicated";
				return false;
			}
			stage->eBossResponseKind = kind;
			stage->iBossResponseThreshold = threshold;
		}
		/* PATTERN 뒤, step 앞에서 빈 span 목록을 만들고 후속 행이 순서대로 채운다. */
		else if (!fields.empty() && "PATTERNROTATION" == fields[0])
		{
			std::uint32_t fromBar = 0u;
			std::uint32_t toBar = 0u;
			std::uint32_t stepCount = 0u;
			BOSS_PATTERN_ROTATION_SELECTION_MODE selectionMode =
				BOSS_PATTERN_ROTATION_SELECTION_MODE::
					ORDERED_INTRO_THEN_WEIGHTED;
			if (7u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) || !ParseNumber(fields[3], fromBar) ||
				!ParseNumber(fields[4], toBar) ||
				!ParseBossPatternRotationSelectionMode(
					fields[5], selectionMode) ||
				!ParseNumber(fields[6], stepCount) ||
				0u == stepCount || stepCount > 32u || fromBar <= toBar)
			{
				m_strStatus = "Boss pattern rotation row is invalid";
				return false;
			}
			std::vector<BOSS_PATTERN_ROTATION_DEFINITION>& rotations =
				m_BossPatternRotations[std::string(fields[1])];
			for (const BOSS_PATTERN_ROTATION_DEFINITION& existing : rotations)
			{
				if (existing.strRotationId == fields[2])
				{
					m_strStatus = "Boss pattern rotation ID is duplicated";
					return false;
				}
			}
			BOSS_PATTERN_ROTATION_DEFINITION staged{};
			staged.strEncounterId = fields[1];
			staged.strRotationId = fields[2];
			staged.eSelectionMode = selectionMode;
			staged.iFromHealthBar = fromBar;
			staged.iToHealthBar = toBar;
			staged.iExpectedStepCount = stepCount;
			rotations.push_back(std::move(staged));
		}
		/* v20 weighted 행은 weight·enabled를 소유하며 고유한 순번으로 WEIGHTED_POOL에만 연결한다. */
		else if (!fields.empty() &&
			"PATTERNROTATIONCANDIDATE" == fields[0])
		{
			std::uint32_t candidateIndex = 0u;
			std::uint32_t selectionWeight = 0u;
			std::uint32_t enabled = 0u;
			if (7u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) ||
				!ParseNumber(fields[3], candidateIndex) ||
				!IsStableId(fields[4]) ||
				!ParseNumber(fields[5], selectionWeight) ||
				0u == selectionWeight || selectionWeight > 100000u ||
				!ParseNumber(fields[6], enabled) || enabled > 1u)
			{
				m_strStatus = "Boss pattern rotation candidate row is invalid";
				return false;
			}
			const auto patternMap = m_BossPatterns.find(std::string(fields[1]));
			if (m_BossPatterns.end() == patternMap ||
				patternMap->second.end() == std::find_if(
					patternMap->second.begin(), patternMap->second.end(),
					[&fields](const BOSS_PATTERN_DEFINITION& pattern)
					{
						return pattern.strPatternId == fields[4] &&
							BOSS_PATTERN_SELECTION::NORMAL == pattern.eSelection;
					}))
			{
				m_strStatus =
					"Boss pattern rotation candidate names no normal pattern";
				return false;
			}
			const auto rotationMap =
				m_BossPatternRotations.find(std::string(fields[1]));
			if (m_BossPatternRotations.end() == rotationMap)
			{
				m_strStatus = "Boss pattern rotation candidate has no span";
				return false;
			}
			const auto rotation = std::find_if(
				rotationMap->second.begin(), rotationMap->second.end(),
				[&fields](const BOSS_PATTERN_ROTATION_DEFINITION& candidate)
				{ return candidate.strRotationId == fields[2]; });
			if (rotationMap->second.end() == rotation ||
				BOSS_PATTERN_ROTATION_SELECTION_MODE::WEIGHTED_POOL !=
					rotation->eSelectionMode ||
				candidateIndex != rotation->Candidates.size() ||
				candidateIndex >= rotation->iExpectedStepCount ||
				rotation->Candidates.end() != std::find_if(
					rotation->Candidates.begin(), rotation->Candidates.end(),
					[&fields](const BOSS_PATTERN_ROTATION_CANDIDATE& candidate)
					{ return candidate.strPatternId == fields[4]; }))
			{
				m_strStatus =
					"Boss pattern rotation candidates violate the managed tagged union";
				return false;
			}
			BOSS_PATTERN_ROTATION_CANDIDATE candidate{};
			candidate.strPatternId = fields[4];
			candidate.iSelectionWeight = selectionWeight;
			candidate.bEnabled = 0u != enabled;
			rotation->Candidates.push_back(std::move(candidate));
		}
		/* 자신의 span과 pattern이 이미 읽힌 뒤 step을 순서대로 추가한다. */
		else if (!fields.empty() && "PATTERNROTATIONSTEP" == fields[0])
		{
			std::uint32_t stepIndex = 0u;
			if (5u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) || !ParseNumber(fields[3], stepIndex) ||
				!IsStableId(fields[4]))
			{
				m_strStatus = "Boss pattern rotation step row is invalid";
				return false;
			}
			const auto patternMap = m_BossPatterns.find(std::string(fields[1]));
			if (m_BossPatterns.end() == patternMap ||
				patternMap->second.end() == std::find_if(
					patternMap->second.begin(), patternMap->second.end(),
					[&fields](const BOSS_PATTERN_DEFINITION& pattern)
					{
						return pattern.strPatternId == fields[4] &&
							BOSS_PATTERN_SELECTION::NORMAL == pattern.eSelection;
					}))
			{
				m_strStatus = "Boss pattern rotation step names no normal pattern";
				return false;
			}
			const auto rotationMap =
				m_BossPatternRotations.find(std::string(fields[1]));
			if (m_BossPatternRotations.end() == rotationMap)
			{
				m_strStatus = "Boss pattern rotation step has no span";
				return false;
			}
			const auto rotation = std::find_if(
				rotationMap->second.begin(), rotationMap->second.end(),
				[&fields](const BOSS_PATTERN_ROTATION_DEFINITION& candidate)
				{ return candidate.strRotationId == fields[2]; });
			if (rotationMap->second.end() == rotation ||
				(BOSS_PATTERN_ROTATION_SELECTION_MODE::ORDERED_INTRO_THEN_WEIGHTED != rotation->eSelectionMode &&
				 BOSS_PATTERN_ROTATION_SELECTION_MODE::ORDERED_LOOP != rotation->eSelectionMode) ||
				stepIndex != rotation->PatternIds.size() ||
				stepIndex >= rotation->iExpectedStepCount)
			{
				m_strStatus =
					"Boss pattern rotation steps violate the legacy tagged union";
				return false;
			}
			rotation->PatternIds.emplace_back(fields[4]);
		}
		else if (!fields.empty() && "PATTERNROTATIONWINDOW" == fields[0])
		{
			std::uint32_t gameplayPhase = 0u;
			std::uint32_t fromBar = 0u;
			std::uint32_t toBar = 0u;
			std::uint32_t candidateCount = 0u;
			if (9u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) || !IsStableId(fields[3]) ||
				!ParseNumber(fields[4], gameplayPhase) ||
				0u == gameplayPhase || gameplayPhase > 3u ||
				!IsStableId(fields[5]) ||
				!ParseNumber(fields[6], fromBar) ||
				!ParseNumber(fields[7], toBar) || fromBar <= toBar ||
				!ParseNumber(fields[8], candidateCount) ||
				0u == candidateCount || candidateCount > 32u)
			{
				m_strStatus = "Boss pattern rotation window row is invalid";
				return false;
			}
			const auto rotationMap =
				m_BossPatternRotations.find(std::string(fields[1]));
			if (m_BossPatternRotations.end() == rotationMap)
			{
				m_strStatus = "Boss pattern rotation window has no span";
				return false;
			}
			const auto rotation = std::find_if(
				rotationMap->second.begin(), rotationMap->second.end(),
				[&fields](const BOSS_PATTERN_ROTATION_DEFINITION& candidate)
				{ return candidate.strRotationId == fields[2]; });
			if (rotationMap->second.end() == rotation ||
				(BOSS_PATTERN_ROTATION_SELECTION_MODE::WEIGHTED_POOL != rotation->eSelectionMode &&
				 BOSS_PATTERN_ROTATION_SELECTION_MODE::ORDERED_LOOP != rotation->eSelectionMode) || rotation->Window.Is_Defined() ||
				fromBar != rotation->iFromHealthBar ||
				toBar != rotation->iToHealthBar ||
				candidateCount != rotation->iExpectedStepCount)
			{
				m_strStatus =
					"Boss pattern rotation window mismatches its managed span";
				return false;
			}
			for (const BOSS_PATTERN_ROTATION_DEFINITION& existing :
				rotationMap->second)
			{
				if (!existing.Window.Is_Defined()) continue;
				if (existing.Window.strWindowId == fields[3] ||
					existing.Window.strSelectionSetId == fields[5])
				{
					m_strStatus =
						"Boss pattern rotation managed window identity is duplicated";
					return false;
				}
			}
			rotation->Window.strWindowId = fields[3];
			rotation->Window.iGameplayPhase = gameplayPhase;
			rotation->Window.strSelectionSetId = fields[5];
			rotation->Window.iFromHealthBar = fromBar;
			rotation->Window.iToHealthBar = toBar;
			rotation->Window.iExpectedCandidateCount = candidateCount;
		}
		else if (!fields.empty() && "PATTERNTARGET" == fields[0])
		{
			if (fields.size() != 4u || !IsStableId(fields[1]) || !IsStableId(fields[3]) ||
				(fields[2] != "GATE1" && fields[2] != "GATE2" && fields[2] != "GATE3" && fields[2] != "BINGO"))
			{ m_strStatus = "KoukuSaydon pattern target is invalid"; return false; }
			BOSS_PATTERN_DEFINITION* owner = nullptr;
			for (auto& [encounter, patterns] : m_BossPatterns)
				for (auto& pattern : patterns) if (pattern.strPatternId == fields[1]) owner = &pattern;
			if (!owner || !owner->strGateId.empty())
			{ m_strStatus = "KoukuSaydon pattern target owner is missing or duplicated"; return false; }
			owner->strGateId = fields[2]; owner->strTargetBossPlacementId = fields[3];
		}
		else if (!fields.empty() && "RAIDGATE" == fields[0])
		{
			KOUKU_RAID_GATE_DEFINITION gate;
			if ((fields.size() != 12u && fields.size() != 13u && fields.size() != 14u) ||
				(fields.size() >= 13u && fields[12] != "NONE" && (fields[2] != "GATE1" || !IsStableId(fields[12]))) ||
				(fields.size() == 14u && !IsStableId(fields[13])) ||
				!IsStableId(fields[1]) || (fields[2] != "GATE1" && fields[2] != "GATE2" && fields[2] != "GATE3" && fields[2] != "BINGO") ||
				!IsStableId(fields[3]) || !IsStableId(fields[4]) || !ParseNumber(fields[5], gate.iSequenceRevision) || !gate.iSequenceRevision ||
				(fields[6] != "NONE" && !IsStableId(fields[6])) || !ParseNumber(fields[7], gate.iIntroDurationMs) || gate.iIntroDurationMs > 600000u ||
                ((fields[6] == "NONE") != (gate.iIntroDurationMs == 0u)) ||
                (fields[2] != "BINGO" && fields[6] == "NONE") ||
				(fields[8] != "NONE" && !IsStableId(fields[8])) || !ParseNumber(fields[9], gate.iClearDurationMs) || gate.iClearDurationMs > 600000u ||
				((fields[8] == "NONE") != (gate.iClearDurationMs == 0u)) || !IsStableId(fields[10]) ||
				!ParseNumber(fields[11], gate.iExpectedEntryCount) || !gate.iExpectedEntryCount || gate.iExpectedEntryCount > 256u ||
				m_KoukuRaidGates.contains(std::string(fields[2])))
			{ m_strStatus = "Kouku raid gate metadata is invalid or duplicated"; return false; }
			gate.strEncounterId = fields[1]; gate.strGateId = fields[2]; gate.strFlowId = fields[3]; gate.strSequenceCompositionId = fields[4];
			if (fields[6] != "NONE") gate.strIntroPatternId = fields[6]; if (fields[8] != "NONE") gate.strClearPatternId = fields[8]; gate.strPrimaryBossPlacementId = fields[10];
			if (fields.size() >= 13u && fields[12] != "NONE") gate.strEntrySequenceInstanceId = fields[12];
			if (fields.size() == 14u) gate.strLoopStartEntryId = fields[13];
			m_KoukuRaidGates.emplace(gate.strGateId, std::move(gate));
		}
		else if (!fields.empty() && "RAIDBINGOSPECIAL" == fields[0])
		{
			if (fields.size() != 3u || fields[1] != "BINGO" || !IsStableId(fields[2]))
			{ m_strStatus = "Bingo special parent reference is invalid"; return false; }
			auto gate = m_KoukuRaidGates.find(std::string(fields[1]));
			if (gate == m_KoukuRaidGates.end() || !gate->second.strBingoSpecialPatternId.empty())
			{ m_strStatus = "Bingo special parent owner is missing or duplicated"; return false; }
			gate->second.strBingoSpecialPatternId = fields[2];
		}
		else if (!fields.empty() && "RAIDFLOWSTEP" == fields[0])
		{
			std::uint32_t index = 0u, wait = 0u;
			if (fields.size() != 7u || !ParseNumber(fields[2], index) || !IsStableId(fields[3]) || (fields[4] != "PATTERN" && fields[4] != "BUNDLE") ||
				!IsStableId(fields[5]) || !ParseNumber(fields[6], wait) || wait > 600000u)
			{ m_strStatus = "Kouku raid flow step is invalid"; return false; }
			auto gate = m_KoukuRaidGates.find(std::string(fields[1]));
			if (gate == m_KoukuRaidGates.end() || index != gate->second.Entries.size() || index >= gate->second.iExpectedEntryCount ||
				std::any_of(gate->second.Entries.begin(), gate->second.Entries.end(), [&](const auto& e) { return e.strEntryId == fields[3]; }))
			{ m_strStatus = "Kouku raid flow order or identity is invalid"; return false; }
			gate->second.Entries.push_back({std::string(fields[3]), std::string(fields[5]), fields[4] == "BUNDLE", wait});
		}
		else if (!fields.empty() && "RAIDFLOWGROUP" == fields[0])
		{
			KOUKU_RAID_FLOW_GROUP group;
			std::uint32_t index = 0u, threshold = 0u;
			if (fields.size() != 8u || !ParseNumber(fields[2], index) || !IsStableId(fields[3]) ||
				!IsStableId(fields[4]) || !IsStableId(fields[5]) ||
				(fields[6] != "NONE" && (!ParseNumber(fields[6], threshold) || threshold > 1000u)) ||
				(fields[7] != "PATTERN_END" && fields[7] != "GROUP_END") || (fields[6] == "NONE" && fields[7] != "PATTERN_END"))
			{ m_strStatus = "Kouku raid flow group properties are invalid"; return false; }
			auto gate = m_KoukuRaidGates.find(std::string(fields[1]));
			if (gate == m_KoukuRaidGates.end() || index != gate->second.EntryGroups.size() || index >= 64u)
			{ m_strStatus = "Kouku raid flow group owner or order is invalid"; return false; }
			const auto& entries = gate->second.Entries;
			const auto first = std::find_if(entries.begin(), entries.end(), [&](const auto& row) { return row.strEntryId == fields[4]; });
			const auto last = std::find_if(entries.begin(), entries.end(), [&](const auto& row) { return row.strEntryId == fields[5]; });
			if (first == entries.end() || last == entries.end() || first > last ||
				(!gate->second.EntryGroups.empty() && static_cast<std::uint32_t>(first - entries.begin()) <= gate->second.EntryGroups.back().iLastEntry) ||
				std::any_of(gate->second.EntryGroups.begin(), gate->second.EntryGroups.end(), [&](const auto& row) { return row.strGroupId == fields[3]; }) ||
				(fields[6] != "NONE" && !gate->second.strLoopStartEntryId.empty()))
			{ m_strStatus = "Kouku raid flow group ranges must be ordered and disjoint saved entries"; return false; }
			group.strGroupId = fields[3]; group.iFirstEntry = static_cast<std::uint32_t>(first - entries.begin());
			group.iLastEntry = static_cast<std::uint32_t>(last - entries.begin());
			if (fields[6] != "NONE") group.RepeatUntilHealthBars = threshold;
			group.bTransitionAtGroupEnd = fields[7] == "GROUP_END";
			gate->second.EntryGroups.push_back(std::move(group));
		}
		else if (!fields.empty() && "RAIDARRIVAL" == fields[0])
		{
			KOUKU_RAID_ARRIVAL arrival;
			if (fields.size() != 9u || (fields[2] != "INTRO" && fields[2] != "CLEAR") || !IsStableId(fields[3]) ||
				!ParseNumber(fields[4], arrival.iSlot) || arrival.iSlot > 3u || !ParseNumber(fields[5], arrival.iStartMs) ||
				!ParseNumber(fields[6], arrival.Position[0]) || !ParseNumber(fields[7], arrival.Position[1]) || !ParseNumber(fields[8], arrival.Position[2]))
			{ m_strStatus = "Kouku raid arrival is invalid"; return false; }
			auto gate = m_KoukuRaidGates.find(std::string(fields[1])); arrival.bClear = fields[2] == "CLEAR"; arrival.strOccurrenceId = fields[3];
			if (gate == m_KoukuRaidGates.end() || arrival.iStartMs > (arrival.bClear ? gate->second.iClearDurationMs : gate->second.iIntroDurationMs) ||
				(arrival.bClear && gate->second.strClearPatternId.empty()) ||
				std::any_of(arrival.Position.begin(), arrival.Position.end(), [](float v) { return !std::isfinite(v) || std::abs(v) > 100000.f; }) ||
				std::any_of(gate->second.Arrivals.begin(), gate->second.Arrivals.end(), [&](const auto& a) { return a.strOccurrenceId == arrival.strOccurrenceId || (a.bClear == arrival.bClear && a.iSlot == arrival.iSlot); }))
			{ m_strStatus = "Kouku raid arrival owner, time, or slot is invalid"; return false; }
			gate->second.Arrivals.push_back(std::move(arrival));
		}
		else if (!fields.empty() && "PATTERNBUNDLE" == fields[0])
		{
			if (fields.size() != 4u || !IsStableId(fields[1]) || !IsStableId(fields[2]) ||
				(fields[3] != "GATE1" && fields[3] != "GATE2" && fields[3] != "GATE3" && fields[3] != "BINGO") ||
				m_BossPatternBundles.contains(std::string(fields[1])))
			{ m_strStatus = "KoukuSaydon bundle is invalid or duplicated"; return false; }
			BOSS_PATTERN_BUNDLE_DEFINITION bundle;
			bundle.strBundleId = fields[1]; bundle.strEncounterId = fields[2]; bundle.strGateId = fields[3];
			m_BossPatternBundles.emplace(bundle.strBundleId, std::move(bundle));
		}
		else if (!fields.empty() && "PATTERNBUNDLEMEMBER" == fields[0])
		{
			std::uint32_t offset = 0u;
			if (fields.size() != 6u || !IsStableId(fields[1]) || !IsStableId(fields[2]) ||
				!IsStableId(fields[3]) || !IsStableId(fields[4]) || !ParseNumber(fields[5], offset) || offset > 600000u)
			{ m_strStatus = "KoukuSaydon bundle member is invalid"; return false; }
			auto bundle = m_BossPatternBundles.find(std::string(fields[1]));
			if (bundle == m_BossPatternBundles.end() || bundle->second.Members.size() >= LostArk::Shared::MAX_KOUKUSAYDON_BUNDLE_MEMBERS)
			{ m_strStatus = "KoukuSaydon bundle member owner or count is invalid"; return false; }
			for (const auto& member : bundle->second.Members)
				if (member.strMemberId == fields[2] || member.strPatternId == fields[3] || member.strTargetBossPlacementId == fields[4])
				{ m_strStatus = "KoukuSaydon bundle member or actor is duplicated"; return false; }
			bundle->second.Members.push_back({std::string(fields[2]), std::string(fields[3]), std::string(fields[4]), offset});
		}
		else if (!fields.empty() && "PATTERNSEQUENCE" == fields[0])
		{
			std::uint32_t interStepPursuitMs = 0u;
			std::uint32_t stepCount = 0u;
			if ((6u != fields.size() && 7u != fields.size()) || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) ||
				(7u == fields.size() && "NONE" != fields[6] && !IsStableId(fields[6])) ||
				("ORDERED_ONCE_THEN_IDLE" != fields[3] && "HEALTH_BAR_ROTATIONS" != fields[3]) ||
				!ParseNumber(fields[4], interStepPursuitMs) ||
				interStepPursuitMs < 100u || interStepPursuitMs > 10000u ||
				!ParseNumber(fields[5], stepCount) ||
				0u == stepCount ||
				stepCount > LostArk::Shared::MAX_VALTAN_PATTERN_FLOW_SLOTS ||
				m_BossPatternSequences.contains(std::string(fields[1])))
			{
				m_strStatus = "Boss pattern sequence row is invalid or duplicated";
				return false;
			}
			BOSS_PATTERN_SEQUENCE_DEFINITION sequence{};
			sequence.strEncounterId = fields[1];
			sequence.strSequenceId = fields[2];
			if (7u == fields.size() && "NONE" != fields[6])
				sequence.strEntranceCinematicPatternId = fields[6];
			sequence.eMode = "HEALTH_BAR_ROTATIONS" == fields[3] ?
				BOSS_PATTERN_SEQUENCE_MODE::HEALTH_BAR_ROTATIONS :
				BOSS_PATTERN_SEQUENCE_MODE::ORDERED_ONCE_THEN_IDLE;
			sequence.iInterStepPursuitMs = interStepPursuitMs;
			sequence.iInterStepPursuitTicks = static_cast<std::uint32_t>(
				(static_cast<std::uint64_t>(interStepPursuitMs) * 30u + 999u) /
				1000u);
			sequence.iExpectedStepCount = stepCount;
			m_BossPatternSequences.emplace(
				sequence.strEncounterId, std::move(sequence));
		}
		else if (!fields.empty() && "PATTERNSEQUENCESTEP" == fields[0])
		{
			std::uint32_t stepIndex = 0u;
			std::uint32_t pursuitAfterMs = 0u;
			if ((5u != fields.size() && 6u != fields.size()) ||
				!IsStableId(fields[1]) ||
				!IsStableId(fields[2]) ||
				!ParseNumber(fields[3], stepIndex) || !IsStableId(fields[4]))
			{
				m_strStatus = "Boss pattern sequence step row is invalid";
				return false;
			}
			const auto sequence =
				m_BossPatternSequences.find(std::string(fields[1]));
			const auto patternMap = m_BossPatterns.find(std::string(fields[1]));
			const bool hasPattern = patternMap != m_BossPatterns.end() &&
				patternMap->second.end() != std::find_if(
					patternMap->second.begin(), patternMap->second.end(),
					[&fields](const BOSS_PATTERN_DEFINITION& pattern)
					{
						return pattern.strPatternId == fields[4];
					});
			const bool hasNextStep =
				m_BossPatternSequences.end() != sequence &&
				stepIndex + 1u < sequence->second.iExpectedStepCount;
			if (6u == fields.size() &&
				(!ParseNumber(fields[5], pursuitAfterMs) ||
				 (hasNextStep && pursuitAfterMs > 10000u) ||
				 (!hasNextStep && 0u != pursuitAfterMs)))
			{
				m_strStatus =
					"Boss pattern sequence transition pursuit is invalid";
				return false;
			}
			if (m_BossPatternSequences.end() == sequence ||
				sequence->second.strSequenceId != fields[2] ||
				stepIndex != sequence->second.PatternIds.size() ||
				stepIndex >= sequence->second.iExpectedStepCount ||
				!hasPattern)
			{
				m_strStatus =
					"Boss pattern sequence steps violate their ordered contract";
				return false;
			}
			sequence->second.PatternIds.emplace_back(fields[4]);
			if (hasNextStep)
			{
				if (5u == fields.size())
					pursuitAfterMs = sequence->second.iInterStepPursuitMs;
				sequence->second.TransitionPursuitMs.push_back(pursuitAfterMs);
				sequence->second.TransitionPursuitTicks.push_back(
					static_cast<std::uint32_t>((
						static_cast<std::uint64_t>(pursuitAfterMs) * 30u + 999u) /
						1000u));
			}
		}
		else if (!fields.empty() && "PATTERNSTAGEACTION" == fields[0])
		{
			std::uint32_t actionOrder = 0;
			BOSS_PATTERN_STAGE_ACTION action{};
			const bool hasReleaseFields = 12u == fields.size() ||
				13u == fields.size();
			if ((10u != fields.size() && !hasReleaseFields) ||
				!IsStableId(fields[1]) ||
				!IsStableId(fields[2]) || !IsStableId(fields[3]) ||
				!ParseNumber(fields[4], actionOrder) ||
				!ParseBossPatternStageActionTrigger(fields[5], action.eTrigger) ||
				!ParseBossPatternStageActionKind(fields[6], action.eKind) ||
				!IsStableId(fields[7]) ||
				!ParseNumber(fields[8], action.iValue) ||
				!ParseNumber(fields[9], action.iDurationMs) ||
				(hasReleaseFields &&
				 (!ParseBossGrabbedReleaseMode(
					fields[10], action.eReleaseMode) ||
				  !ParseNumber(fields[11], action.fReleaseSpeedMps) ||
				  (13u == fields.size() &&
				   !ParseNumber(fields[12], action.fReleaseYawOffsetDegrees)))) ||
				!IsValidBossPatternStageAction(
					action.eTrigger, action.eKind, fields[7],
					action.iValue, action.iDurationMs,
					action.eReleaseMode, action.fReleaseSpeedMps,
					action.fReleaseYawOffsetDegrees))
			{
				m_strStatus = "Boss pattern stage action row is invalid";
				return false;
			}
			const auto ownerMap = m_BossPatterns.find(std::string(fields[1]));
			if (m_BossPatterns.end() == ownerMap)
			{
				m_strStatus = "Boss pattern stage action has no encounter";
				return false;
			}
			const auto owner = std::find_if(
				ownerMap->second.begin(), ownerMap->second.end(),
				[&fields](const BOSS_PATTERN_DEFINITION& pattern)
				{ return pattern.strPatternId == fields[2]; });
			if (ownerMap->second.end() == owner)
			{
				m_strStatus = "Boss pattern stage action has no pattern owner";
				return false;
			}
			const auto stage = std::find_if(
				owner->Stages.begin(), owner->Stages.end(),
				[&fields](const BOSS_PATTERN_STAGE_DEFINITION& candidate)
				{ return candidate.strActionId == fields[3]; });
			if (owner->Stages.end() == stage ||
				(BOSS_PATTERN_STAGE_ACTION_KIND::SET_PLAYER_SILENCE ==
					action.eKind && action.iDurationMs < stage->iDurationMs) ||
				actionOrder != stage->Actions.size() || actionOrder >= 8u ||
				std::any_of(stage->Actions.begin(), stage->Actions.end(),
					[&action, &fields](const BOSS_PATTERN_STAGE_ACTION& existing)
					{
						return existing.eTrigger == action.eTrigger &&
							existing.eKind == action.eKind &&
							existing.strTargetId == fields[7];
					}))
			{
				m_strStatus =
					"Boss pattern stage action has no stage owner or is duplicated";
				return false;
			}
			action.strTargetId = fields[7];
			stage->Actions.push_back(std::move(action));
		}
		else if (!fields.empty() && "PATTERNSTAGEVOLLEY" == fields[0])
		{
			std::uint32_t actionOrder = 0u;
			std::uint32_t allowOverlap = 0u;
			BOSS_PATTERN_STAGE_ACTION action{};
			action.eKind =
				BOSS_PATTERN_STAGE_ACTION_KIND::SPAWN_COMBAT_OBJECT_VOLLEY;
			if (22u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) || !IsStableId(fields[3]) ||
				!ParseNumber(fields[4], actionOrder) ||
				!ParseBossPatternStageActionTrigger(fields[5], action.eTrigger) ||
				BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER != action.eTrigger ||
				!IsStableId(fields[6]) ||
				("PER_ALIVE_PLAYER" != fields[7] &&
				 "BOSS_RELATIVE" != fields[7] &&
				 "ARENA_CENTER" != fields[7]) ||
				!ParseNumber(fields[8],
					action.Volley.iCountPerResolvedTarget) ||
				!ParseNumber(fields[10], action.Volley.fRadiusM) ||
				!ParseNumber(fields[11], action.Volley.fStartAngleDegrees) ||
				!ParseNumber(fields[12], action.Volley.fAngleStepDegrees) ||
				!ParseNumber(fields[13], allowOverlap) || allowOverlap > 1u ||
				!ParseNumber(fields[14], action.Volley.iMaximumTotalObjects) ||
				!ParseNumber(fields[15], action.Volley.iSpawnCount) ||
				!ParseNumber(fields[16], action.Volley.iFirstSpawnOffsetMs) ||
				!ParseNumber(fields[17], action.Volley.iSpawnIntervalMs) ||
				!ParseNumber(fields[18], action.Volley.iArenaRandomCount) ||
				!ParseNumber(fields[19], action.Volley.fArenaRandomRadiusM) ||
				!ParseNumber(fields[20], action.Volley.fArenaHeightToleranceM) ||
				action.Volley.iCountPerResolvedTarget < 1u ||
				action.Volley.iCountPerResolvedTarget > 8u ||
				static_cast<std::uint64_t>(
					action.Volley.iMaximumTotalObjects) <
					static_cast<std::uint64_t>(
						action.Volley.iCountPerResolvedTarget) +
					static_cast<std::uint64_t>(
						action.Volley.iArenaRandomCount) ||
				action.Volley.iMaximumTotalObjects > 64u ||
				action.Volley.iSpawnCount < 1u ||
				action.Volley.iSpawnCount > 8u ||
				(1u == action.Volley.iSpawnCount ?
					0u != action.Volley.iSpawnIntervalMs :
					0u == action.Volley.iSpawnIntervalMs) ||
				action.Volley.iArenaRandomCount > 32u ||
				!std::isfinite(action.Volley.fRadiusM) ||
				action.Volley.fRadiusM < 0.f ||
				!std::isfinite(action.Volley.fStartAngleDegrees) ||
				!std::isfinite(action.Volley.fAngleStepDegrees) ||
				!std::isfinite(action.Volley.fArenaRandomRadiusM) ||
				!std::isfinite(action.Volley.fArenaHeightToleranceM) ||
				action.Volley.fArenaRandomRadiusM < 0.f ||
				action.Volley.fArenaHeightToleranceM < 0.f)
			{
				m_strStatus = "Boss pattern stage volley row is invalid";
				return false;
			}
			action.Volley.ePolicy = "PER_ALIVE_PLAYER" == fields[7] ?
				BOSS_COMBAT_OBJECT_VOLLEY_POLICY::PER_ALIVE_PLAYER :
				("ARENA_CENTER" == fields[7] ?
					BOSS_COMBAT_OBJECT_VOLLEY_POLICY::ARENA_CENTER :
					BOSS_COMBAT_OBJECT_VOLLEY_POLICY::BOSS_RELATIVE);
			action.Volley.bAllowOverlap = 0u != allowOverlap;
			if ("SINGLE" == fields[9])
				action.Volley.eLayout =
					BOSS_COMBAT_OBJECT_LAYOUT_KIND::SINGLE;
			else if ("RADIAL" == fields[9])
				action.Volley.eLayout =
					BOSS_COMBAT_OBJECT_LAYOUT_KIND::RADIAL;
			else
			{
				m_strStatus = "Boss pattern stage volley layout kind is invalid";
				return false;
			}
			if ("NONE" == fields[21])
				action.Volley.eArenaAnchorPolicy =
					BOSS_COMBAT_OBJECT_ARENA_ANCHOR_POLICY::NONE;
			else if ("BOSS_SPAWN_POSITION" == fields[21])
				action.Volley.eArenaAnchorPolicy =
					BOSS_COMBAT_OBJECT_ARENA_ANCHOR_POLICY::BOSS_SPAWN_POSITION;
			else
			{
				m_strStatus = "Boss pattern stage volley arena anchor is invalid";
				return false;
			}
			const bool isMulti =
				action.Volley.iCountPerResolvedTarget > 1u;
			const bool perAlivePlayer =
				BOSS_COMBAT_OBJECT_VOLLEY_POLICY::PER_ALIVE_PLAYER ==
					action.Volley.ePolicy;
			/* ARENA_CENTER도 BOSS_RELATIVE처럼 결정적인 다중 root 방사 파동을 사용하며 arena 무작위 추가분은 없다. */
			const bool bossRelative =
				BOSS_COMBAT_OBJECT_VOLLEY_POLICY::BOSS_RELATIVE ==
					action.Volley.ePolicy ||
				BOSS_COMBAT_OBJECT_VOLLEY_POLICY::ARENA_CENTER ==
					action.Volley.ePolicy;
			if ((isMulti &&
				(BOSS_COMBAT_OBJECT_LAYOUT_KIND::RADIAL !=
					action.Volley.eLayout || action.Volley.fRadiusM <= 0.f ||
				 action.Volley.fAngleStepDegrees == 0.f ||
				 action.Volley.bAllowOverlap ||
				 action.Volley.fAngleStepDegrees *
					static_cast<float>(action.Volley.iCountPerResolvedTarget) >
					360.000001f)) ||
				(!isMulti &&
					(BOSS_COMBAT_OBJECT_LAYOUT_KIND::SINGLE !=
						action.Volley.eLayout ||
					 action.Volley.fRadiusM != 0.f ||
					 action.Volley.fStartAngleDegrees != 0.f ||
					 action.Volley.fAngleStepDegrees != 0.f ||
					 action.Volley.bAllowOverlap)) ||
				(0u == action.Volley.iArenaRandomCount ?
					(BOSS_COMBAT_OBJECT_ARENA_ANCHOR_POLICY::NONE !=
						action.Volley.eArenaAnchorPolicy ||
					 action.Volley.fArenaRandomRadiusM != 0.f ||
					 action.Volley.fArenaHeightToleranceM != 0.f) :
					(BOSS_COMBAT_OBJECT_ARENA_ANCHOR_POLICY::
						BOSS_SPAWN_POSITION !=
						action.Volley.eArenaAnchorPolicy ||
					 action.Volley.fArenaRandomRadiusM <= 0.f ||
					 action.Volley.fArenaHeightToleranceM <= 0.f)) ||
				(bossRelative &&
					(!isMulti || 1u != action.Volley.iSpawnCount ||
					 0u != action.Volley.iArenaRandomCount)))
			{
				m_strStatus = "Boss pattern stage volley layout is invalid";
				return false;
			}
			action.strTargetId = fields[6];
			action.iValue = action.Volley.iCountPerResolvedTarget;
			const auto ownerMap = m_BossPatterns.find(std::string(fields[1]));
			if (m_BossPatterns.end() == ownerMap)
			{
				m_strStatus = "Boss pattern stage volley has no encounter";
				return false;
			}
			const auto owner = std::find_if(
				ownerMap->second.begin(), ownerMap->second.end(),
				[&fields](const BOSS_PATTERN_DEFINITION& pattern)
				{ return pattern.strPatternId == fields[2]; });
			if (ownerMap->second.end() == owner)
			{
				m_strStatus = "Boss pattern stage volley has no pattern owner";
				return false;
			}
			const auto stage = std::find_if(
				owner->Stages.begin(), owner->Stages.end(),
				[&fields](const BOSS_PATTERN_STAGE_DEFINITION& candidate)
				{ return candidate.strActionId == fields[3]; });
			const std::uint64_t lastSpawnOffsetMs =
				static_cast<std::uint64_t>(action.Volley.iFirstSpawnOffsetMs) +
				static_cast<std::uint64_t>(action.Volley.iSpawnCount - 1u) *
					action.Volley.iSpawnIntervalMs;
			if (owner->Stages.end() == stage ||
				lastSpawnOffsetMs >= stage->iDurationMs ||
				actionOrder != stage->Actions.size() || actionOrder >= 8u ||
				std::any_of(stage->Actions.begin(), stage->Actions.end(),
					[&action](const BOSS_PATTERN_STAGE_ACTION& existing)
					{
						return existing.eTrigger == action.eTrigger &&
							existing.strTargetId == action.strTargetId;
					}))
			{
				m_strStatus =
					"Boss pattern stage volley owner is missing or duplicated";
				return false;
			}
			stage->Actions.push_back(std::move(action));
		}
		else if (!fields.empty() && "PATTERNSTAGEBRANCH" == fields[0])
		{
			BOSS_PATTERN_STAGE_BRANCH branch{};
			if (6u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) || !IsStableId(fields[3]) ||
				!ParseBossPatternStageOutcome(fields[4], branch.eOutcome) ||
				("-" != fields[5] && !IsStableId(fields[5])))
			{
				m_strStatus = "Boss pattern stage branch row is invalid";
				return false;
			}
			const auto ownerMap = m_BossPatterns.find(std::string(fields[1]));
			if (m_BossPatterns.end() == ownerMap)
			{
				m_strStatus = "Boss pattern stage branch has no encounter";
				return false;
			}
			const auto owner = std::find_if(
				ownerMap->second.begin(), ownerMap->second.end(),
				[&fields](const BOSS_PATTERN_DEFINITION& pattern)
				{ return pattern.strPatternId == fields[2]; });
			if (ownerMap->second.end() == owner)
			{
				m_strStatus = "Boss pattern stage branch has no pattern owner";
				return false;
			}
			const auto stage = std::find_if(
				owner->Stages.begin(), owner->Stages.end(),
				[&fields](const BOSS_PATTERN_STAGE_DEFINITION& candidate)
				{ return candidate.strActionId == fields[3]; });
			if (owner->Stages.end() == stage || stage->Branches.size() >= 8u ||
				std::any_of(stage->Branches.begin(), stage->Branches.end(),
					[&branch](const BOSS_PATTERN_STAGE_BRANCH& existing)
					{ return existing.eOutcome == branch.eOutcome; }))
			{
				m_strStatus =
					"Boss pattern stage branch has no stage owner or is duplicated";
				return false;
			}
			branch.strNextActionId =
				"-" == fields[5] ? "" : std::string(fields[5]);
			stage->Branches.push_back(std::move(branch));
		}
		else if (!fields.empty() && "PATTERNSTAGEFOLLOWUP" == fields[0])
		{
			BOSS_PATTERN_STAGE_BRANCH branch{};
			if (6u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) || !IsStableId(fields[3]) ||
				!ParseBossPatternStageOutcome(fields[4], branch.eOutcome) ||
				!IsStableId(fields[5]))
			{
				m_strStatus = "Boss pattern stage follow-up row is invalid";
				return false;
			}
			const auto ownerMap = m_BossPatterns.find(std::string(fields[1]));
			if (m_BossPatterns.end() == ownerMap)
			{
				m_strStatus = "Boss pattern stage follow-up has no encounter";
				return false;
			}
			const auto owner = std::find_if(
				ownerMap->second.begin(), ownerMap->second.end(),
				[&fields](const BOSS_PATTERN_DEFINITION& pattern)
				{ return pattern.strPatternId == fields[2]; });
			if (ownerMap->second.end() == owner)
			{
				m_strStatus = "Boss pattern stage follow-up has no pattern owner";
				return false;
			}
			const auto stage = std::find_if(
				owner->Stages.begin(), owner->Stages.end(),
				[&fields](const BOSS_PATTERN_STAGE_DEFINITION& candidate)
				{ return candidate.strActionId == fields[3]; });
			if (owner->Stages.end() == stage || stage->Branches.size() >= 8u ||
				std::any_of(stage->Branches.begin(), stage->Branches.end(),
					[&branch](const BOSS_PATTERN_STAGE_BRANCH& existing)
					{ return existing.eOutcome == branch.eOutcome; }))
			{
				m_strStatus =
					"Boss pattern stage follow-up has no stage owner or is duplicated";
				return false;
			}
			branch.strNextPatternId = std::string(fields[5]);
			stage->Branches.push_back(std::move(branch));
		}
		else if (!fields.empty() && "PATTERNSTAGEAIM" == fields[0])
		{
			if ((5u != fields.size() && 7u != fields.size()) || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) || !IsStableId(fields[3]) ||
				("NEAREST_EACH_TICK" != fields[4] && "PATTERN_TARGET" != fields[4]))
			{
				m_strStatus = "Boss pattern stage aim row is invalid";
				return false;
			}
			const auto ownerMap = m_BossPatterns.find(std::string(fields[1]));
			if (m_BossPatterns.end() == ownerMap)
			{
				m_strStatus = "Boss pattern stage aim has no encounter";
				return false;
			}
			const auto owner = std::find_if(ownerMap->second.begin(), ownerMap->second.end(),
				[&fields](const BOSS_PATTERN_DEFINITION& pattern)
				{ return pattern.strPatternId == fields[2]; });
			if (ownerMap->second.end() == owner)
			{
				m_strStatus = "Boss pattern stage aim has no pattern owner";
				return false;
			}
			const auto stage = std::find_if(owner->Stages.begin(), owner->Stages.end(),
				[&fields](const BOSS_PATTERN_STAGE_DEFINITION& candidate)
				{ return candidate.strActionId == fields[3]; });
			if (owner->Stages.end() == stage || stage->bTrackNearestTarget || stage->bTrackPatternTarget)
			{
				m_strStatus = "Boss pattern stage aim has no stage owner or is duplicated";
				return false;
			}
			stage->bTrackNearestTarget = "NEAREST_EACH_TICK" == fields[4];
			stage->bTrackPatternTarget = "PATTERN_TARGET" == fields[4];
			if (7u == fields.size())
			{
				stage->bHasAimEnd = fields[5] != "-";
				stage->bHasAimResponseScale = fields[6] != "-";
				if ((stage->bHasAimEnd && (!ParseNumber(fields[5], stage->iAimEndMs) ||
					stage->iAimEndMs > stage->iDurationMs)) ||
					(stage->bHasAimResponseScale && (!ParseNumber(fields[6], stage->fAimResponseScale) ||
					!std::isfinite(stage->fAimResponseScale) || stage->fAimResponseScale < .01f ||
					stage->fAimResponseScale > 10.f)))
				{
					m_strStatus = "Boss pattern stage aim window or response is invalid";
					return false;
				}
			}
		}
		else if (!fields.empty() && "PATTERNSTAGEMOTION" == fields[0])
		{
			BOSS_PATTERN_STAGE_MOTION motion{};
			if ((6u != fields.size() && 8u != fields.size()) ||
				!IsStableId(fields[1]) || !IsStableId(fields[2]) ||
				!IsStableId(fields[3]))
			{
				m_strStatus = "Boss pattern stage motion row is invalid";
				return false;
			}
			if (6u == fields.size() && "FORWARD" == fields[4] &&
				ParseNumber(fields[5], motion.fDistance) &&
				std::isfinite(motion.fDistance) && motion.fDistance > 0.f &&
				motion.fDistance <= 1000.f)
			{
				motion.eKind = BOSS_PATTERN_STAGE_MOTION_KIND::FORWARD;
			}
			else if (6u == fields.size() && "TO_ARENA_CENTER" == fields[4] &&
				"0" == fields[5])
			{
				motion.eKind = BOSS_PATTERN_STAGE_MOTION_KIND::TO_ARENA_CENTER;
			}
			else if (8u == fields.size() &&
				"PORTAL_TARGET_RUSH" == fields[4] &&
				ParseNumber(fields[5], motion.iRetargetDelayMs) &&
				ParseNumber(fields[6], motion.fSpeedMps) &&
				ParseNumber(fields[7], motion.fDistance) &&
				std::isfinite(motion.fSpeedMps) &&
				std::isfinite(motion.fDistance) &&
				motion.fSpeedMps > 0.f && motion.fSpeedMps <= 1000.f &&
				motion.fDistance > 0.f && motion.fDistance <= 1000.f)
			{
				motion.eKind = BOSS_PATTERN_STAGE_MOTION_KIND::PORTAL_TARGET_RUSH;
			}
			else if (8u == fields.size() && "PORTAL_CROSS_ARENA" == fields[4] &&
				ParseNumber(fields[5], motion.iCornerIndex) &&
				ParseNumber(fields[6], motion.fHalfExtentsX) &&
				ParseNumber(fields[7], motion.fHalfExtentsZ) &&
				motion.iCornerIndex < 4u && std::isfinite(motion.fHalfExtentsX) &&
				std::isfinite(motion.fHalfExtentsZ) &&
				motion.fHalfExtentsX >= 1.f && motion.fHalfExtentsX <= 100.f &&
				motion.fHalfExtentsZ >= 1.f && motion.fHalfExtentsZ <= 100.f)
			{
				motion.eKind = BOSS_PATTERN_STAGE_MOTION_KIND::PORTAL_CROSS_ARENA;
			}
			else
			{
				m_strStatus = "Boss pattern stage motion policy is invalid";
				return false;
			}
			const auto ownerMap = m_BossPatterns.find(std::string(fields[1]));
			if (m_BossPatterns.end() == ownerMap)
			{
				m_strStatus = "Boss pattern stage motion has no encounter";
				return false;
			}
			const auto owner = std::find_if(
				ownerMap->second.begin(), ownerMap->second.end(),
				[&fields](const BOSS_PATTERN_DEFINITION& pattern)
				{ return pattern.strPatternId == fields[2]; });
			if (ownerMap->second.end() == owner)
			{
				m_strStatus = "Boss pattern stage motion has no pattern owner";
				return false;
			}
			const auto stage = std::find_if(
				owner->Stages.begin(), owner->Stages.end(),
				[&fields](const BOSS_PATTERN_STAGE_DEFINITION& candidate)
				{ return candidate.strActionId == fields[3]; });
			if (owner->Stages.end() == stage ||
				BOSS_PATTERN_STAGE_MOTION_KIND::NONE != stage->Motion.eKind)
			{
				m_strStatus =
					"Boss pattern stage motion has no stage owner or is duplicated";
				return false;
			}
			if (BOSS_PATTERN_STAGE_MOTION_KIND::PORTAL_TARGET_RUSH ==
				motion.eKind)
			{
				const double travelEndMs =
					static_cast<double>(motion.iRetargetDelayMs) +
					static_cast<double>(motion.fDistance) /
					static_cast<double>(motion.fSpeedMps) * 1000.0;
				std::uint32_t expectedOffsetMs = motion.iRetargetDelayMs;
				bool validOffsets = !stage->HitOffsetsMs.empty();
				for (const std::uint32_t offsetMs : stage->HitOffsetsMs)
				{
					validOffsets = validOffsets &&
						offsetMs == expectedOffsetMs &&
						static_cast<double>(offsetMs) < travelEndMs;
					expectedOffsetMs += 50u;
				}
				validOffsets = validOffsets &&
					static_cast<double>(expectedOffsetMs) + 0.000001 >=
						travelEndMs;
				if (travelEndMs > static_cast<double>(stage->iDurationMs) +
						0.000001 || !validOffsets)
				{
					m_strStatus =
						"Boss portal target-rush motion exceeds its stage or has unsafe swept-hit offsets";
					return false;
				}
			}
			stage->Motion = motion;
		}
		else if (!fields.empty() && "PATTERNWALLCONTACT" == fields[0])
		{
			std::uint32_t stageIndex = 0u;
			if (6u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) || !ParseNumber(fields[3], stageIndex) ||
				!IsStableId(fields[4]) || !IsStableId(fields[5]))
			{
				m_strStatus = "Boss pattern wall-contact row is invalid";
				return false;
			}
			const auto ownerMap = m_BossPatterns.find(std::string(fields[1]));
			if (m_BossPatterns.end() == ownerMap)
			{
				m_strStatus = "Boss pattern wall-contact has no encounter";
				return false;
			}
			const auto owner = std::find_if(
				ownerMap->second.begin(), ownerMap->second.end(),
				[&fields](const BOSS_PATTERN_DEFINITION& pattern)
				{ return pattern.strPatternId == fields[2]; });
			if (ownerMap->second.end() == owner ||
				stageIndex >= owner->Stages.size())
			{
				m_strStatus = "Boss pattern wall-contact has no stage owner";
				return false;
			}
			BOSS_PATTERN_STAGE_DEFINITION& stage = owner->Stages[stageIndex];
			if (stage.strStageId != fields[4] ||
				stage.strActionId != fields[5] || stage.bWallContact ||
				BOSS_PATTERN_STAGE_KIND::ACTIVE != stage.eStageKind ||
				BOSS_PATTERN_HIT_SHAPE::NONE == stage.eHitShape ||
				0u == stage.iHitCount)
			{
				m_strStatus = "Boss pattern wall-contact join is invalid";
				return false;
			}
			stage.bWallContact = true;
		}
		/* publisher가 PATTERNSTAGE 뒤로 정렬하므로 보완할 stage가 이미 존재해야 한다. */
		else if (!fields.empty() && "PATTERNSTAGECOVERPIERCE" == fields[0])
		{
			std::uint32_t stageIndex = 0u;
			if (6u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) || !ParseNumber(fields[3], stageIndex) ||
				!IsStableId(fields[4]) || !IsStableId(fields[5]))
			{
				m_strStatus = "Boss pattern cover-pierce row is invalid";
				return false;
			}
			const auto ownerMap = m_BossPatterns.find(std::string(fields[1]));
			if (m_BossPatterns.end() == ownerMap)
			{
				m_strStatus = "Boss pattern cover-pierce has no encounter";
				return false;
			}
			const auto owner = std::find_if(
				ownerMap->second.begin(), ownerMap->second.end(),
				[&fields](const BOSS_PATTERN_DEFINITION& pattern)
				{ return pattern.strPatternId == fields[2]; });
			if (ownerMap->second.end() == owner ||
				stageIndex >= owner->Stages.size())
			{
				m_strStatus = "Boss pattern cover-pierce has no stage owner";
				return false;
			}
			BOSS_PATTERN_STAGE_DEFINITION& stage = owner->Stages[stageIndex];
			if (stage.strStageId != fields[4] ||
				stage.strActionId != fields[5] || stage.bPiercesCover ||
				BOSS_PATTERN_HIT_SHAPE::NONE == stage.eHitShape ||
				0u == stage.iHitCount)
			{
				m_strStatus = "Boss pattern cover-pierce join is invalid";
				return false;
			}
			stage.bPiercesCover = true;
		}
		/* slot마다 고정 폭 행을 읽어 stage에 모은다. 같은 slot을 다른 edge에 중복 배정하지 않는 것은 publisher가 문서 전체에서 검증한다. */
		else if (!fields.empty() && "PATTERNSTAGEPROPBREAK" == fields[0])
		{
			std::uint32_t stageIndex = 0u;
			if (8u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) || !ParseNumber(fields[3], stageIndex) ||
				!IsStableId(fields[4]) || !IsStableId(fields[5]) ||
				!IsStableId(fields[6]) || !IsStableId(fields[7]))
			{
				m_strStatus = "Boss pattern prop-break row is invalid";
				return false;
			}
			const auto ownerMap = m_BossPatterns.find(std::string(fields[1]));
			if (m_BossPatterns.end() == ownerMap)
			{
				m_strStatus = "Boss pattern prop-break has no encounter";
				return false;
			}
			const auto owner = std::find_if(
				ownerMap->second.begin(), ownerMap->second.end(),
				[&fields](const BOSS_PATTERN_DEFINITION& pattern)
				{ return pattern.strPatternId == fields[2]; });
			if (ownerMap->second.end() == owner ||
				stageIndex >= owner->Stages.size())
			{
				m_strStatus = "Boss pattern prop-break has no stage owner";
				return false;
			}
			BOSS_PATTERN_STAGE_DEFINITION& stage = owner->Stages[stageIndex];
			const std::string slotId(fields[7]);
			if (stage.strStageId != fields[4] ||
				stage.strActionId != fields[5] ||
				(!stage.strPropBreakSetId.empty() &&
					stage.strPropBreakSetId != fields[6]) ||
				stage.PropBreakSlotIds.end() != std::find(
					stage.PropBreakSlotIds.begin(),
					stage.PropBreakSlotIds.end(), slotId))
			{
				m_strStatus = "Boss pattern prop-break join is invalid";
				return false;
			}
			stage.strPropBreakSetId = std::string(fields[6]);
			stage.PropBreakSlotIds.push_back(slotId);
		}
		/* publisher가 PATTERNSTAGE 뒤로 정렬하므로 보완할 stage가 이미 존재해야 한다. */
		else if (!fields.empty() && "PATTERNSTAGECHARGE" == fields[0])
		{
			std::uint32_t stageIndex = 0u;
			if (6u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) || !ParseNumber(fields[3], stageIndex) ||
				!IsStableId(fields[4]) || !IsStableId(fields[5]))
			{
				m_strStatus = "Boss pattern charge-impact row is invalid";
				return false;
			}
			const auto ownerMap = m_BossPatterns.find(std::string(fields[1]));
			if (m_BossPatterns.end() == ownerMap)
			{
				m_strStatus = "Boss pattern charge-impact has no encounter";
				return false;
			}
			const auto owner = std::find_if(
				ownerMap->second.begin(), ownerMap->second.end(),
				[&fields](const BOSS_PATTERN_DEFINITION& pattern)
				{ return pattern.strPatternId == fields[2]; });
			if (ownerMap->second.end() == owner ||
				stageIndex >= owner->Stages.size())
			{
				m_strStatus = "Boss pattern charge-impact has no stage owner";
				return false;
			}
			BOSS_PATTERN_STAGE_DEFINITION& stage = owner->Stages[stageIndex];
			/* WALL_CONTACT는 charge를 즉시 저작 반응으로 끝낸다. 갑옷 개방 charge는 GROGGY, 일반 charge는 접촉·시간 만료 모두 RECOVERY를 사용할 수 있다. */
			const auto wallBranch = std::find_if(
				stage.Branches.begin(), stage.Branches.end(),
				[](const BOSS_PATTERN_STAGE_BRANCH& branch)
				{
					return BOSS_PATTERN_STAGE_OUTCOME::WALL_CONTACT ==
						branch.eOutcome;
				});
			const auto timeoutBranch = std::find_if(
				stage.Branches.begin(), stage.Branches.end(),
				[](const BOSS_PATTERN_STAGE_BRANCH& branch)
				{
					return BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT ==
						branch.eOutcome;
				});
			const BOSS_PATTERN_STAGE_DEFINITION* impactStage =
				stageIndex + 1u < owner->Stages.size() ?
					&owner->Stages[stageIndex + 1u] : nullptr;
			const bool hasLocalImpact = stage.Branches.end() != wallBranch;
			const bool isGroggyImpact = nullptr != impactStage &&
				BOSS_PATTERN_STAGE_KIND::GROGGY == impactStage->eStageKind;
			const bool isConvergedRecovery = nullptr != impactStage &&
				BOSS_PATTERN_STAGE_KIND::RECOVERY == impactStage->eStageKind &&
				stage.Branches.end() != timeoutBranch &&
				timeoutBranch->strNextActionId == impactStage->strActionId;
			if (stage.strStageId != fields[4] ||
				stage.strActionId != fields[5] || stage.bChargeImpact ||
				(hasLocalImpact &&
				 (nullptr == impactStage ||
				  wallBranch->strNextActionId != impactStage->strActionId ||
				  (!isGroggyImpact && !isConvergedRecovery))) ||
				owner->fMaximumRange <= 0.f)
			{
				m_strStatus = "Boss pattern charge-impact join is invalid";
				return false;
			}
			stage.bChargeImpact = true;
		}
		else if (!fields.empty() && "VALTANTIMELINE" == fields[0])
		{
			std::uint32_t rowCount = 0u;
			if (4u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) ||
				!ParseNumber(fields[3], rowCount) || 0u == rowCount ||
				rowCount > 256u)
			{
				m_strStatus = "Valtan timeline row is invalid";
				return false;
			}
			VALTAN_TIMELINE_DEFINITION definition{};
			definition.strEncounterId = fields[1];
			definition.strTimelineId = fields[2];
			definition.Rows.resize(rowCount);
			if (!m_ValtanTimelines.emplace(
				definition.strEncounterId, std::move(definition)).second)
			{
				m_strStatus = "Duplicate Valtan timeline row";
				return false;
			}
		}
		else if (!fields.empty() && "VALTANTIMELINEROW" == fields[0])
		{
			VALTAN_TIMELINE_ROW timelineRow{};
			std::uint32_t actionCount = 0u;
			if (11u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) ||
				!ParseNumber(fields[3], timelineRow.iCommandId) ||
				!ParseNumber(fields[4], timelineRow.iOrdinal) ||
				!IsStableId(fields[5]) ||
				!ParseNumber(fields[6], timelineRow.iSectionHealthBar) ||
				!ParseValtanTimelineEntryType(fields[7], timelineRow.eEntryType) ||
				!ParseValtanTimelineArenaState(fields[8], timelineRow.eArenaState) ||
				!ParseValtanTimelinePropState(fields[9], timelineRow.ePropState) ||
				!ParseNumber(fields[10], actionCount) ||
				0u == timelineRow.iOrdinal ||
				0u == timelineRow.iCommandId ||
				timelineRow.iCommandId !=
					Calculate_ValtanTimelineCommandId(fields[5]) ||
				0u == timelineRow.iSectionHealthBar ||
				timelineRow.iSectionHealthBar > 1000u ||
				0u == actionCount || actionCount > 8u)
			{
				m_strStatus = "Valtan timeline occurrence row is invalid";
				return false;
			}
			const auto owner =
				m_ValtanTimelines.find(std::string(fields[1]));
			if (m_ValtanTimelines.end() == owner ||
				owner->second.strTimelineId != fields[2] ||
				timelineRow.iOrdinal > owner->second.Rows.size())
			{
				m_strStatus = "Valtan timeline occurrence has no owner";
				return false;
			}
			VALTAN_TIMELINE_ROW& slot =
				owner->second.Rows[timelineRow.iOrdinal - 1u];
			if (0u != slot.iOrdinal)
			{
				m_strStatus = "Duplicate Valtan timeline occurrence ordinal";
				return false;
			}
			if (slot.PatternActions.size() > actionCount)
			{
				m_strStatus =
					"Valtan timeline occurrence action count is invalid";
				return false;
			}
			timelineRow.strRowId = fields[5];
			timelineRow.PatternActions = std::move(slot.PatternActions);
			timelineRow.PatternActions.resize(actionCount);
			slot = std::move(timelineRow);
		}
		else if (!fields.empty() && "VALTANTIMELINEPATTERN" == fields[0])
		{
			std::uint32_t ordinal = 0u;
			std::uint32_t actionIndex = 0u;
			VALTAN_TIMELINE_PATTERN_ACTION action{};
			if (7u != fields.size() || !IsStableId(fields[1]) ||
				!IsStableId(fields[2]) || !ParseNumber(fields[3], ordinal) ||
				!ParseNumber(fields[4], actionIndex) ||
				!IsStableId(fields[5]) ||
				!ParseNumber(fields[6], action.iRepeat) ||
				0u == ordinal || 0u == actionIndex || actionIndex > 8u ||
				0u == action.iRepeat || action.iRepeat > 4u)
			{
				m_strStatus = "Valtan timeline pattern action row is invalid";
				return false;
			}
			const auto owner = m_ValtanTimelines.find(std::string(fields[1]));
			if (m_ValtanTimelines.end() == owner ||
				owner->second.strTimelineId != fields[2] ||
				ordinal > owner->second.Rows.size())
			{
				m_strStatus = "Valtan timeline pattern action has no owner";
				return false;
			}
			VALTAN_TIMELINE_ROW& timelineRow = owner->second.Rows[ordinal - 1u];
			if ((0u != timelineRow.iOrdinal &&
				timelineRow.iOrdinal != ordinal) ||
				(0u != timelineRow.iOrdinal &&
				 actionIndex > timelineRow.PatternActions.size()))
			{
				m_strStatus = "Valtan timeline pattern action has no occurrence owner";
				return false;
			}
			if (timelineRow.PatternActions.size() < actionIndex)
				timelineRow.PatternActions.resize(actionIndex);
			VALTAN_TIMELINE_PATTERN_ACTION& slot =
				timelineRow.PatternActions[actionIndex - 1u];
			if (!slot.strPatternId.empty() || 0u != slot.iRepeat)
			{
				m_strStatus = "Duplicate Valtan timeline pattern action index";
				return false;
			}
			action.strPatternId = fields[5];
			slot = std::move(action);
		}
		else if (!fields.empty() && "PLAYER" == fields[0])
		{
			PLAYER_RUNTIME_PROFILE player{};
			/* 기존 15필드 뒤에 치명 확률·배율이 붙는다. 두 필드가 없으면 이전처럼 치명타가 없다. */
			const bool hasCritical = 17u == fields.size();
			if ((15u != fields.size() && !hasCritical) ||
				!ParseCharacterClass(fields[1], player.eCharacterClass) ||
				!ParseNumber(fields[2], player.iMaximumHp) ||
				!ParseNumber(fields[3], player.iMaximumResource) ||
				!ParseNumber(fields[4], player.iResourceRegenPerSecond) ||
				!ParseNumber(fields[5], player.iAttackPower) ||
				!ParseNumber(fields[6], player.iDefense) ||
				!ParseNumber(fields[7], player.fMoveSpeed) ||
				!ParseNumber(fields[8], player.fDefenseStanceMoveSpeedScale) ||
				!ParseNumber(fields[9], player.iMaximumIdentity) ||
				!ParseNumber(fields[10], player.iIdentityRegenPerSecond) ||
				!ParseNumber(fields[11], player.iIdentityDrainPerSecond) ||
				!ParseNumber(fields[12], player.iIdentityStanceSwitchCost) ||
				!ParseNumber(fields[13], player.iIdentityCyclic) ||
				!ParseStance(fields[14], player.eDefaultStance) ||
				(hasCritical &&
					(!ParseNumber(fields[15], player.iCriticalChancePercent) ||
					 !ParseNumber(fields[16], player.iCriticalDamagePercent) ||
					 player.iCriticalChancePercent > 100u ||
					 player.iCriticalDamagePercent < 100u)) ||
				0u == player.iMaximumHp || 0u == player.iMaximumResource ||
				0u == player.iResourceRegenPerSecond ||
				player.iResourceRegenPerSecond > player.iMaximumResource ||
				0u == player.iAttackPower || 0u == player.iDefense ||
				!std::isfinite(player.fMoveSpeed) || player.fMoveSpeed <= 0.f ||
				!std::isfinite(player.fDefenseStanceMoveSpeedScale) ||
				player.fDefenseStanceMoveSpeedScale <= 0.f ||
				player.fDefenseStanceMoveSpeedScale > 1.f ||
				player.iIdentityCyclic > 1u ||
				(0u == player.iMaximumIdentity &&
					(0u != player.iIdentityRegenPerSecond ||
						0u != player.iIdentityDrainPerSecond ||
						0u != player.iIdentityStanceSwitchCost ||
						0u != player.iIdentityCyclic)) ||
				/* 순환 gauge는 감쇠·자세 전환 비용과 함께 쓰지 않는다. skill별 identity cost와 pool 상한은 모든 SKILL 행을 읽은 뒤 검증한다. */
				(0u != player.iIdentityCyclic &&
					(0u != player.iIdentityDrainPerSecond ||
						0u != player.iIdentityStanceSwitchCost ||
						0u == player.iIdentityRegenPerSecond)) ||
				player.iIdentityStanceSwitchCost > player.iMaximumIdentity ||
				!m_Players.emplace(player.eCharacterClass, player).second)
			{
				m_strStatus = "Player profile row is invalid";
				return false;
			}
		}
		else if (!fields.empty() && "PLAYEREMBER" == fields[0])
		{
			LostArk::Shared::CHARACTER_CLASS_ID characterClass =
				LostArk::Shared::CHARACTER_CLASS_ID::END;
			GUARDIAN_EMBER_PROFILE ember{};
			if (6u != fields.size() ||
				!ParseCharacterClass(fields[1], characterClass) ||
				!ParseNumber(fields[2], ember.iGaugeGainPerHit) ||
				!ParseNumber(fields[3], ember.iDragonDurationMs) ||
				!ParseNumber(fields[4], ember.iMaximumSockets) ||
				!ParseNumber(fields[5], ember.iDamageBonusPercentPerOrb) ||
				0u == ember.iGaugeGainPerHit || ember.iDragonDurationMs < 1000u ||
				ember.iDragonDurationMs > 600000u ||
				0u == ember.iMaximumSockets || ember.iMaximumSockets > 64u ||
				ember.iDamageBonusPercentPerOrb > 1000u ||
				!m_EmberProfiles.emplace(characterClass, ember).second)
			{
				m_strStatus = "Player ember profile row is invalid";
				return false;
			}
		}
		else
		{
			m_strStatus = "Unknown gameplay bootstrap row kind";
			return false;
		}
	}

	if (std::getline(input, line) || m_Skills.empty() || m_Players.empty() ||
		m_Bosses.empty() || m_BossPatterns.empty() ||
		m_BossCombatObjects.empty() ||
		m_ValtanTimelines.empty() ||
		m_DamageRatePercentByProfileId.empty() ||
		!m_ValtanPresentationGenerationId.Is_Valid())
	{
		m_strStatus = "Gameplay bootstrap has trailing rows or missing definitions";
		return false;
	}
	const bool hasKoukuSaydonBoss =
		m_Bosses.contains("BOSS_KOUKUSAYDON_G1_KOUKU");
	const bool hasKoukuSaydonPatterns =
		m_BossPatterns.contains("ENCOUNTER_KOUKUSAYDON_G1");
	const bool hasKoukuSaydonSequence =
		m_BossPatternSequences.contains("ENCOUNTER_KOUKUSAYDON_G1");
	const bool hasAnyKoukuSaydonProductComponent =
		hasKoukuSaydonBoss || hasKoukuSaydonPatterns || hasKoukuSaydonSequence ||
		0u != m_iKoukuSaydonProductSourceRevision;
	if (hasAnyKoukuSaydonProductComponent &&
		(!hasKoukuSaydonBoss || !hasKoukuSaydonPatterns ||
		 !hasKoukuSaydonSequence ||
		 0u == m_iKoukuSaydonProductSourceRevision))
	{
		m_strStatus =
			"KoukuSaydon Product source revision and catalog rows are incomplete";
		return false;
	}
	if (skillCombatTraitOwners.size() != m_Skills.size())
	{
		m_strStatus = "Player skill combat traits are incomplete";
		return false;
	}
	/* 행 정렬 때문에 skill cost의 pool 검사는 로드 뒤에 한다. 전체 class 중 최대 pool을 공통 상한으로 사용한다. */
	std::uint32_t largestResourcePool = 0;
	std::uint32_t largestIdentityPool = 0;
	for (const auto& [characterClass, player] : m_Players)
	{
		(void)characterClass;
		largestResourcePool =
			(std::max)(largestResourcePool, player.iMaximumResource);
		largestIdentityPool =
			(std::max)(largestIdentityPool, player.iMaximumIdentity);
	}
	for (const auto& [skillId, skill] : m_Skills)
	{
		(void)skillId;
		const auto* damageProfile = Find_DamageProfile(skill.strDamageProfileId);
		if (nullptr != damageProfile && 0u != damageProfile->iBossHealthBarDamage &&
			LostArk::Shared::PLAYER_SKILL_KIND::ACTIVE != skill.eSkillKind)
		{
			m_strStatus = "Boss health-bar damage requires a single-stage ACTIVE skill";
			return false;
		}
		const bool isCombo = LostArk::Shared::PLAYER_SKILL_KIND::COMBO ==
			skill.eSkillKind;
		const bool validStageCount = isCombo ?
			(skill.ComboStages.size() >= 2u && skill.ComboStages.size() <= 8u) :
			(LostArk::Shared::PLAYER_SKILL_KIND::HOLD == skill.eSkillKind ?
				skill.ComboStages.size() == 3u :
				(LostArk::Shared::PLAYER_SKILL_KIND::COUNTER == skill.eSkillKind ?
					skill.ComboStages.size() == 2u : skill.ComboStages.empty()));
		if (!validStageCount ||
			(isCombo &&
				(skill.ComboStages.back().iComboAdvanceMs !=
					skill.ComboStages.back().iActionDurationMs ||
				 0u != skill.ComboStages.back().iInputOpenMs ||
				 0u != skill.ComboStages.back().iInputCloseMs)))
		{
			m_strStatus = "Player skill stage contract is invalid";
			return false;
		}
		if (isCombo)
		{
			for (std::size_t stageIndex = 0u;
				stageIndex < skill.ComboStages.size(); ++stageIndex)
			{
				const PLAYER_COMBO_STAGE& stage = skill.ComboStages[stageIndex];
				const bool isFinalStage =
					stageIndex + 1u == skill.ComboStages.size();
				const bool isAutomaticStage = !isFinalStage &&
					0u == stage.iInputOpenMs && 0u == stage.iInputCloseMs;
				if ((isAutomaticStage &&
						stage.iComboAdvanceMs != stage.iActionDurationMs) ||
					(!isFinalStage && !isAutomaticStage &&
						(stage.iInputOpenMs >= stage.iInputCloseMs ||
							stage.iInputCloseMs > stage.iActionDurationMs)))
				{
					m_strStatus = "Player combo stage input contract is invalid";
					return false;
				}
				std::uint64_t latestRequiredFireMs = 0u;
				for (const PLAYER_SKILL_HIT& hit : stage.Hits)
				{
					latestRequiredFireMs = (std::max)(latestRequiredFireMs,
						static_cast<std::uint64_t>(hit.iTimeMs) +
						static_cast<std::uint64_t>(hit.iRepeatCount - 1u) *
							hit.iRepeatMs);
				}
				for (const PLAYER_SKILL_PROJECTILE& projectile : stage.Projectiles)
				{
					latestRequiredFireMs = (std::max)(latestRequiredFireMs,
						static_cast<std::uint64_t>(projectile.iTimeMs));
				}
				if (stage.iComboAdvanceMs < latestRequiredFireMs)
				{
					m_strStatus =
						"Player combo boundary precedes a hit or projectile spawn";
					return false;
				}
			}
		}
		if (!skill.strDamageProfileId.empty() &&
			0u == Find_DamageRatePercent(skill.strDamageProfileId))
		{
			m_strStatus = "Player skill references missing damage profile";
			return false;
		}
		if (skill.iResourceCost > largestResourcePool ||
			skill.iIdentityCost > largestIdentityPool)
		{
			m_strStatus = "Player skill costs more than any class can hold";
			return false;
		}
	}
	std::unordered_set<LostArk::Shared::CHARACTER_CLASS_ID> classesWithIdentitySkillCost;
	for (const auto& [skillId, skill] : m_Skills)
	{
		(void)skillId;
		if (0u != skill.iIdentityCost)
			classesWithIdentitySkillCost.insert(skill.eCharacterClass);
	}
	for (const auto& [characterClass, player] : m_Players)
	{
		/* ember class는 용 형상의 유지 gauge를 소비한다. */
		if (0u != player.iMaximumIdentity &&
			0u == player.iIdentityDrainPerSecond &&
			0u == player.iIdentityStanceSwitchCost &&
			0u == player.iIdentityCyclic &&
			!m_EmberProfiles.contains(characterClass) &&
			!classesWithIdentitySkillCost.contains(characterClass))
		{
			m_strStatus = "Player identity gauge never spends";
			return false;
		}
	}
	for (const auto& [characterClass, ember] : m_EmberProfiles)
	{
		const PLAYER_RUNTIME_PROFILE* owner = Find_Player(characterClass);
		if (nullptr == owner || 0u == owner->iMaximumIdentity ||
			ember.iGaugeGainPerHit > owner->iMaximumIdentity)
		{
			m_strStatus = "Player ember profile has no identity gauge to fill";
			return false;
		}
	}
	for (const auto& [skillId, skill] : m_Skills)
	{
		(void)skillId;
		if (0u == skill.iEmberGain && 0u == skill.iEmberCost)
			continue;
		const GUARDIAN_EMBER_PROFILE* ember =
			Find_EmberProfile(skill.eCharacterClass);
		if (nullptr == ember || skill.iEmberGain > ember->iMaximumSockets ||
			skill.iEmberCost > ember->iMaximumSockets)
		{
			m_strStatus = "Player ember skill belongs to a class without an ember profile";
			return false;
		}
	}
	std::unordered_set<std::string> dependentBossArchetypeIds;
	for (const auto& [encounterId, patterns] : m_BossPatterns)
	{
		for (const BOSS_PATTERN_DEFINITION& pattern : patterns)
		{
			if (BOSS_PATTERN_FINALE_KIND::NONE == pattern.Finale.eKind)
				continue;
			const auto ghost = m_Bosses.find(pattern.Finale.strGhostArchetypeId);
			const auto ghostParts = m_BossParts.find(
				pattern.Finale.strGhostArchetypeId);
			if (m_Bosses.end() == ghost ||
				ghost->second.strEncounterId != encounterId ||
				(m_BossParts.end() != ghostParts && !ghostParts->second.empty()) ||
				pattern.bInvulnerableWhileRunning ||
				pattern.Finale.iMaximumActiveGhosts < 1u ||
				pattern.Finale.iMaximumActiveGhosts > 64u ||
				pattern.Finale.iAuxiliarySpawnIntervalMs > 600000u ||
				pattern.Finale.iPortalSpawnIntervalMs > 600000u ||
				pattern.Finale.GhostPatternIds.empty() ||
				pattern.Finale.GhostPatternIds.size() > 64u)
			{
				m_strStatus = "Boss finale dependent archetype is invalid";
				return false;
			}
			dependentBossArchetypeIds.insert(ghost->first);
			if ("ENCOUNTER_VALTAN" == encounterId &&
				"VALTAN_GHOST_FINALE" == pattern.strPatternId)
			{
				const std::vector<std::string> expectedGhostPatterns = {
					"VALTAN_WHIRLWIND",
					"VALTAN_FOUR_SLASH",
					"VALTAN_SEQUENCE_FOUR",
					"VALTAN_CROSS",
					"VALTAN_CHARGE",
					"VALTAN_CHARGE_2"
				};
				const std::vector<std::string> currentGhostPatterns(expectedGhostPatterns.begin(), expectedGhostPatterns.begin() + 4u);
				if (pattern.Finale.GhostPatternIds != expectedGhostPatterns &&
					pattern.Finale.GhostPatternIds != currentGhostPatterns)
				{
					m_strStatus =
						"Valtan ghost finale auxiliary skill pool is invalid";
					return false;
				}
			}
			std::unordered_set<std::string> childPatternIds;
			for (const std::string& patternId : pattern.Finale.GhostPatternIds)
			{
				const auto attack = std::find_if(patterns.begin(), patterns.end(),
					[&patternId](const BOSS_PATTERN_DEFINITION& candidate)
					{ return candidate.strPatternId == patternId; });
				if (!childPatternIds.insert(patternId).second ||
					patterns.end() == attack ||
					BOSS_PATTERN_FINALE_KIND::NONE != attack->Finale.eKind)
				{
					m_strStatus = "Boss finale dependent attack is missing or recursive";
					return false;
				}
				if (!HasFiniteBossStageGraph(*attack))
				{
					m_strStatus = "Boss finale dependent attack is not finite";
					return false;
				}
			}
		}
	}
	std::unordered_set<std::string> spawnedBossCombatObjectIds;
	for (const auto& [archetypeId, boss] : m_Bosses)
	{
		const auto foundParts = m_BossParts.find(archetypeId);
		const bool isValtanPrimary = "BOSS_VALTAN" == archetypeId &&
			"ENCOUNTER_VALTAN" == boss.strEncounterId;
		/* G1 encounter의 BOSS_KOUKUSAYDON_ archetype은 별도 부위 정의가 없어도 허용한다. */
		const bool isKoukuSaydonGateOne =
			"ENCOUNTER_KOUKUSAYDON_G1" == boss.strEncounterId &&
			archetypeId.starts_with("BOSS_KOUKUSAYDON_");
		if ((m_BossParts.end() == foundParts || foundParts->second.empty()) &&
			!dependentBossArchetypeIds.contains(archetypeId) &&
			!isKoukuSaydonGateOne)
		{
			m_strStatus = "Boss has no part definitions";
			return false;
		}
		std::uint32_t totalDamageReductionPercent = 0u;
		const std::vector<BOSS_PART_DEFINITION> emptyParts;
		const auto& bossParts = m_BossParts.end() == foundParts ?
			emptyParts : foundParts->second;
		for (const BOSS_PART_DEFINITION& part : bossParts)
		{
			if (part.strBossArchetypeId != archetypeId)
			{
				m_strStatus = "Boss part owner is invalid";
				return false;
			}
			totalDamageReductionPercent += part.iDamageReductionPercent;
		}
		if (totalDamageReductionPercent >= 100u)
		{
			m_strStatus = "Boss part damage reduction total is invalid";
			return false;
		}
		if (dependentBossArchetypeIds.contains(archetypeId))
		{
			const bool hasPrimary = std::any_of(m_Bosses.begin(), m_Bosses.end(),
				[&](const auto& entry)
				{
					return entry.second.strEncounterId == boss.strEncounterId &&
						!dependentBossArchetypeIds.contains(entry.first);
				});
			if (!hasPrimary)
			{
				m_strStatus = "Dependent boss encounter has no independent owner";
				return false;
			}
			/* The encounter graph is validated once by its primary profile. */
			continue;
		}
		const auto foundPatterns = m_BossPatterns.find(boss.strEncounterId);
		if (m_BossPatterns.end() == foundPatterns || foundPatterns->second.empty())
		{
			m_strStatus = "Boss encounter has no runtime patterns";
			return false;
		}
		std::size_t gameplayPhaseActionCount = 0u;
		bool hasExactValtanArenaBreakPhaseAction = false;
		bool hasExactValtanGhostRespawnPhaseAction = false;
		bool hasExactValtanHighJumpTypedVolley = false;
		std::size_t valtanHighJumpTypedVolleyCount = 0u;
		bool hasExactValtanGhostPortalTypedVolley = false;
		std::size_t valtanGhostPortalTypedVolleyCount = 0u;
		std::unordered_set<std::uint64_t> healthMechanicOrderKeys;
		for (const BOSS_PATTERN_DEFINITION& pattern : foundPatterns->second)
		{
            if (pattern.iTimelineDurationMs && (!isKoukuSaydonGateOne ||
                !CKoukuSaydonBrain::Validate_AnimationOnlyPattern(pattern, m_strStatus)))
            { if (m_strStatus.empty()) m_strStatus = "Independent row lifetime requires a valid Kouku pattern"; return false; }
			const bool hasAirborne = std::any_of(pattern.MechanicTriggers.begin(), pattern.MechanicTriggers.end(),
				[](const auto& trigger) { return trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::ALBION_AIRBORNE ||
                    trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::PURSUIT_PROJECTILES; });
            const bool hasShowtimeTargets = std::any_of(pattern.MechanicTriggers.begin(), pattern.MechanicTriggers.end(),
                [](const auto& trigger) { return trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::SHOWTIME_PLAYER_TARGETS; });
            const bool hasPhysicalHook = std::any_of(pattern.LogicWindows.begin(), pattern.LogicWindows.end(), [](const auto& window) {
                return std::any_of(window.CardRegions.begin(), window.CardRegions.end(), [](const auto& region) {
                    return std::any_of(region.WorldTrack.Keys.begin(), region.WorldTrack.Keys.end(), [](const auto& key) { return key.bHasGripPosition; }); }); });
            if ((hasAirborne || hasShowtimeTargets || hasPhysicalHook) && (!isKoukuSaydonGateOne || !CKoukuSaydonBrain::Validate_AnimationOnlyPattern(pattern, m_strStatus)))
			{ if (m_strStatus.empty()) m_strStatus = "Albion airborne requires a Kouku Product pattern"; return false; }
			for (const auto& trigger : pattern.MechanicTriggers)
			{
				if ((trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::CARD_MAZE_STAGE_PLAYERS) != !trigger.PlayerEntryPositions.empty() ||
					trigger.PlayerEntryPositions.size() > 4u)
				{ m_strStatus = "Card maze staging requires one to four destinations on its own trigger"; return false; }
				if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::CROSS_DIRECTION_CLONES)
				{
					SERVER_WORLD_ENTITY candidate{};
					std::size_t selected = 0u; std::array<std::uint32_t, 4u> durations{};
					if (!isKoukuSaydonGateOne || !CKoukuSaydonBrain::Validate_AnimationOnlyPattern(pattern, m_strStatus) ||
						!CKoukuSaydonBrain::Select_CrossDirection(candidate, pattern, trigger, *this, selected, durations, m_strStatus))
					{ if (m_strStatus.empty()) m_strStatus = "Cross direction requires a Kouku Product parent"; return false; }
				}
				if (trigger.eKind != BOSS_PATTERN_MECHANIC_TRIGGER_KIND::SUMMON_PATTERNS) continue;
				if (!isKoukuSaydonGateOne || trigger.PatternSpawns.empty() || trigger.PatternSpawns.size() > KOUKU_SUMMON_MAX_PATTERN_SPAWNS)
				{ m_strStatus = "Summon Pattern trigger requires one to sixteen spawns"; return false; }
				for (const auto& spawn : trigger.PatternSpawns)
				{
					const auto child = std::find_if(foundPatterns->second.begin(), foundPatterns->second.end(),
						[&](const auto& row) { return row.strPatternId == spawn.strPatternId; });
					if (child == foundPatterns->second.end())
					{ m_strStatus = "Summoned Pattern is unavailable: " + spawn.strPatternId; return false; }
					if (!CKoukuSaydonBrain::Validate_SummonedPattern(pattern, *child, m_strStatus, true)) return false;
					std::uint64_t childDurationMs = 0u;
					for (const auto& stage : child->Stages) childDurationMs += stage.iDurationMs;
					if (childDurationMs > trigger.iDurationMs)
					{ m_strStatus = "Summoned Pattern exceeds its occurrence lifetime: " + spawn.strPatternId; return false; }
				}
			}
			for (const std::string& auditionArchetypeId :
				pattern.AuditionBossArchetypeIds)
			{
				const auto auditionBoss = m_Bosses.find(auditionArchetypeId);
				if (!isKoukuSaydonGateOne || m_Bosses.end() == auditionBoss ||
					auditionBoss->second.strEncounterId != pattern.strEncounterId ||
					dependentBossArchetypeIds.contains(auditionArchetypeId))
				{
					m_strStatus =
						"Boss pattern audition boss is not an arena boss of its encounter";
					return false;
				}
			}
			const bool requiresFiniteGraph =
				BOSS_PATTERN_FINALE_KIND::NONE != pattern.Finale.eKind ||
				"VALTAN_TRASH" == pattern.strPatternId ||
				"VALTAN_TRASH_CATCH_IF" == pattern.strPatternId ||
				"VALTAN_TRASH_CATCH_SUCCESS" == pattern.strPatternId ||
				"VALTAN_TRASH_CATCH_FAIL" == pattern.strPatternId;
			if (requiresFiniteGraph && !HasFiniteBossStageGraph(pattern))
			{
				m_strStatus = "Boss bounded pattern stage graph is cyclic or dangling";
				return false;
			}
			const std::string patternPolicyKey =
				pattern.strEncounterId + "\n" + pattern.strPatternId;
			const bool hasVerticalOffset = 0.f != pattern.fVerticalOffsetM;
			const bool validVerticalOffset =
				std::isfinite(pattern.fVerticalOffsetM) &&
				std::fabs(pattern.fVerticalOffsetM) <= 100.f &&
				(!hasVerticalOffset ||
				 (patternVerticalOffsetOwners.contains(patternPolicyKey) &&
				  BOSS_PATTERN_MOTION_KIND::NONE == pattern.Motion.eKind));
			const bool isNormal =
				BOSS_PATTERN_SELECTION::NORMAL == pattern.eSelection;
			const bool isHealthMechanic =
				BOSS_PATTERN_SELECTION::HEALTH_BAR == pattern.eSelection;
			const bool isAuditionOnly =
				BOSS_PATTERN_SELECTION::AUDITION_ONLY == pattern.eSelection;
			const bool validNormalSelection = isNormal &&
				pattern.iMinimumHealthBar >= 1u &&
				pattern.iMaximumHealthBar >= pattern.iMinimumHealthBar &&
				pattern.iMaximumHealthBar <= boss.iMaximumHealthBars &&
				0u == pattern.iTriggerHealthBar && 0u == pattern.iTriggerOrder &&
				pattern.iSelectionWeight > 0u &&
				pattern.iMaximumConsecutiveUses > 0u;
			const bool hasNoHealthWindow =
				0u == pattern.iMinimumHealthBar &&
				0u == pattern.iMaximumHealthBar;
			const bool hasValidHealthWindow =
				pattern.iMinimumHealthBar >= 1u &&
				pattern.iMaximumHealthBar >= pattern.iMinimumHealthBar &&
				pattern.iMaximumHealthBar <= boss.iMaximumHealthBars;
			const bool validHealthSelection = isHealthMechanic &&
				(hasNoHealthWindow || hasValidHealthWindow) &&
				pattern.iTriggerHealthBar >= 1u &&
				pattern.iTriggerHealthBar <= boss.iMaximumHealthBars &&
				pattern.iTriggerOrder > 0u &&
				0u == pattern.iSelectionWeight;
			const bool validAuditionSelection = isAuditionOnly &&
				0u == pattern.iMinimumHealthBar &&
				0u == pattern.iMaximumHealthBar &&
				0u == pattern.iTriggerHealthBar &&
				0u == pattern.iTriggerOrder &&
				0u == pattern.iSelectionWeight &&
				0u == pattern.iMaximumConsecutiveUses &&
				(BOSS_PATTERN_CATEGORY::MECHANIC != pattern.eCategory ||
				 isKoukuSaydonGateOne);
			const bool validSelection = validNormalSelection ||
				validHealthSelection || validAuditionSelection;
			// 수동 저작 audition clip에는 추출 skill 출처가 없다. publisher가 PATTERNSOURCE를 생략하는 경우만 허용하고 일반 pattern·잘못된 명시 행은 계속 거부한다.
			const bool isAuthoredValtanAudition =
				"ENCOUNTER_VALTAN" == pattern.strEncounterId && isAuditionOnly &&
				!patternSourceOwners.contains(patternPolicyKey) &&
				0u == pattern.iSourcePrimaryActionId &&
				0u == pattern.iSourceShapeCount &&
				0u == pattern.iSourceCooldownMs &&
				0u == pattern.iSourceCooldownTicks &&
				0u == pattern.iSourceRangeUnits &&
				0u == pattern.iSourceApproachUnits &&
				0u == pattern.iSourceTurnDegrees;
			if (!validSelection || !validVerticalOffset ||
				!patternPolicyOwners.contains(patternPolicyKey) ||
				(BOSS_PATTERN_AIM_POLICY::FACE_MOTION_ANCHOR ==
					pattern.eAimPolicy &&
				 BOSS_PATTERN_MOTION_KIND::LEAP_TO_ANCHOR !=
					pattern.Motion.eKind) ||
				(BOSS_PATTERN_MOTION_KIND::NONE != pattern.Motion.eKind &&
				 (pattern.Stages.empty() ||
				  0u == pattern.Motion.iTravelStageIndex ||
				  pattern.Motion.iTravelStageIndex >= pattern.Stages.size() ||
				  pattern.Motion.iTakeoffStartMs >=
					pattern.Motion.iTakeoffEndMs ||
				  pattern.Motion.iTakeoffEndMs >
					pattern.Stages.front().iDurationMs ||
				  pattern.Motion.iTravelStartMs >=
					pattern.Motion.iTravelEndMs ||
				  pattern.Motion.iTravelEndMs >
					pattern.Stages[pattern.Motion.iTravelStageIndex].iDurationMs)) ||
				(0u == pattern.iSourcePrimaryActionId && !isKoukuSaydonGateOne &&
				 !isAuthoredValtanAudition) ||
				pattern.Stages.size() != pattern.iExpectedStageCount)
			{
				m_strStatus = "Boss pattern selection or stage count is invalid";
				return false;
			}
			if (BOSS_PATTERN_SELECTION::HEALTH_BAR == pattern.eSelection)
			{
				const std::uint64_t mechanicOrderKey =
					(static_cast<std::uint64_t>(pattern.iTriggerHealthBar) << 32u) |
					static_cast<std::uint64_t>(pattern.iTriggerOrder);
				if (!healthMechanicOrderKeys.insert(mechanicOrderKey).second)
				{
					m_strStatus =
						"Boss health mechanic trigger bar/order pair is duplicated";
					return false;
				}
			}
			if (pattern.BossMotion)
			{
				const auto& motion = *pattern.BossMotion;
				if (motion.Keys.empty() ? motion.StartPosition[1] != motion.EndPosition[1] :
					(motion.Keys.size() < 2u || motion.Keys.size() > 512u ||
					 motion.Keys.front().iTimeMs != motion.iStartMs || motion.Keys.back().iTimeMs != motion.iEndMs ||
					 motion.Keys.front().Position != motion.StartPosition || motion.Keys.back().Position != motion.EndPosition))
				{ m_strStatus = "Boss Motion keys must include both exact endpoints, or retain the linear base height"; return false; }
				std::uint64_t durationMs = 0u;
				for (const auto& stage : pattern.Stages) durationMs += stage.iDurationMs;
				if (!isKoukuSaydonGateOne || pattern.bResetBossToSpawn || pattern.ResetBossYawDegrees ||
					pattern.BossMotion->iEndMs > durationMs ||
					std::any_of(pattern.MechanicTriggers.begin(), pattern.MechanicTriggers.end(), [](const auto& trigger)
					{ return trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::REAL_GAZE_TELEPORT ||
						(trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TELEPORT_XZ ||
                         trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BOSS_TELEPORT_GROUNDED); }))
				{ m_strStatus = "Boss Motion exceeds its Pattern or conflicts with another position policy"; return false; }
			}
			std::unordered_set<std::string> activeStageActions;
			bool patternOwnsBossResponse = false;
			for (std::size_t stageIndex = 0u;
				stageIndex < pattern.Stages.size(); ++stageIndex)
			{
				const BOSS_PATTERN_STAGE_DEFINITION& stage =
					pattern.Stages[stageIndex];
				// 행 단위 검증 뒤, 완성된 stage의 판정 시각과 기존 pulse schedule을 대조한다.
				// contacts가 없으면 기존 stage 판정을 그대로 사용한다. contacts가 있는 경우는
				// 피해 pulse와 허용된 이동만 사용할 수 있어 별도의 판정 시계를 만들지 않는다.
				if (!LostArk::Shared::Validate_StageAttackContacts(
						stage.AttackContacts, stage.iDurationMs, stage.HitOffsetsMs,
						stage.iHitCount, stage.iHitDelayMs, stage.iHitIntervalMs) ||
					(!stage.AttackContacts.empty() &&
					 (stage.eHitActivationKind != BOSS_PATTERN_HIT_ACTIVATION_KIND::PULSE_SCHEDULE ||
					  stage.ePlayerResponse != BOSS_PATTERN_PLAYER_RESPONSE::DAMAGE ||
					  (stage.Motion.eKind != BOSS_PATTERN_STAGE_MOTION_KIND::NONE &&
					   stage.Motion.eKind != BOSS_PATTERN_STAGE_MOTION_KIND::TO_ARENA_CENTER))))
				{
					m_strStatus = "Stage contacts do not match ordinary pulse authority";
					return false;
				}
				const bool zeroShapeValues =
					0.f == stage.fHitOuterRadius &&
					0.f == stage.fHitInnerRadius &&
					0.f == stage.fHitAngleDegrees &&
					0.f == stage.fHitLength &&
					0.f == stage.fHitHalfWidth;
				bool validShape = false;
				switch (stage.eHitShape)
				{
				case BOSS_PATTERN_HIT_SHAPE::NONE:
					validShape = zeroShapeValues && 0u == stage.iHitCount &&
						0u == stage.iHitIntervalMs &&
						stage.HitOffsetsMs.empty() &&
						stage.strDamageProfileId.empty();
					break;
				case BOSS_PATTERN_HIT_SHAPE::CIRCLE:
					validShape = stage.fHitOuterRadius > 0.f &&
						0.f == stage.fHitInnerRadius &&
						0.f == stage.fHitAngleDegrees &&
						0.f == stage.fHitLength && 0.f == stage.fHitHalfWidth;
					break;
				case BOSS_PATTERN_HIT_SHAPE::RING:
					validShape = stage.fHitOuterRadius > stage.fHitInnerRadius &&
						stage.fHitInnerRadius > 0.f &&
						0.f == stage.fHitAngleDegrees &&
						0.f == stage.fHitLength && 0.f == stage.fHitHalfWidth;
					break;
				case BOSS_PATTERN_HIT_SHAPE::CONE:
					validShape = stage.fHitAngleDegrees > 0.f &&
						stage.fHitAngleDegrees <= 180.f && stage.fHitLength > 0.f &&
						0.f == stage.fHitOuterRadius &&
						0.f == stage.fHitInnerRadius && 0.f == stage.fHitHalfWidth;
					break;
				case BOSS_PATTERN_HIT_SHAPE::BOX:
				case BOSS_PATTERN_HIT_SHAPE::CROSS:
				case BOSS_PATTERN_HIT_SHAPE::SIX_DIRECTIONS:
					validShape = stage.fHitLength > 0.f && stage.fHitHalfWidth > 0.f &&
						0.f == stage.fHitOuterRadius &&
						0.f == stage.fHitInnerRadius &&
						0.f == stage.fHitAngleDegrees;
					break;
				}
				if (BOSS_PATTERN_HIT_SHAPE::NONE != stage.eHitShape)
				{
					const bool hasExplicitHitOffsets =
						!stage.HitOffsetsMs.empty();
					const bool validExplicitHitOffsets =
						hasExplicitHitOffsets &&
						stage.HitOffsetsMs.size() == stage.iHitCount &&
						0u == stage.iHitIntervalMs && 0u == stage.iHitDelayMs &&
						stage.HitOffsetsMs.back() < stage.iDurationMs &&
						std::adjacent_find(
							stage.HitOffsetsMs.begin(), stage.HitOffsetsMs.end(),
							std::greater_equal<std::uint32_t>()) ==
							stage.HitOffsetsMs.end();
					const bool validLegacyHitSchedule =
						!hasExplicitHitOffsets &&
						BOSS_PATTERN_HIT_ACTIVATION_KIND::PULSE_SCHEDULE ==
							stage.eHitActivationKind &&
						(1u == stage.iHitCount ? 0u == stage.iHitIntervalMs :
							stage.iHitIntervalMs > 0u) &&
						static_cast<std::uint64_t>(stage.iHitDelayMs) +
							static_cast<std::uint64_t>(stage.iHitCount - 1u) *
							stage.iHitIntervalMs < stage.iDurationMs;
					const bool validActiveWindow =
						BOSS_PATTERN_HIT_ACTIVATION_KIND::ACTIVE_WINDOW ==
							stage.eHitActivationKind &&
						0u == stage.iHitCount && 0u == stage.iHitIntervalMs &&
						0u == stage.iHitDelayMs && stage.HitOffsetsMs.empty() &&
						stage.iHitActivationLifetimeMs > 0u &&
						static_cast<std::uint64_t>(stage.iHitActivationStartMs) +
							stage.iHitActivationLifetimeMs <= stage.iDurationMs;
					validShape = validShape &&
						((stage.iHitCount > 0u &&
						  BOSS_PATTERN_HIT_ACTIVATION_KIND::PULSE_SCHEDULE ==
							stage.eHitActivationKind &&
						  (validExplicitHitOffsets || validLegacyHitSchedule) &&
						  0u == stage.iHitActivationStartMs &&
						  0u == stage.iHitActivationLifetimeMs) ||
						 validActiveWindow) &&
						0u != Find_DamageRatePercent(stage.strDamageProfileId);
				}
				const bool validHitAnchor =
					std::isfinite(stage.fHitAnchorForwardOffsetM) &&
					std::isfinite(stage.fHitAnchorRightOffsetM) &&
					std::isfinite(stage.fHitAnchorYawOffsetDegrees) &&
					std::fabs(stage.fHitAnchorForwardOffsetM) <= 1000.f &&
					std::fabs(stage.fHitAnchorRightOffsetM) <= 1000.f &&
					std::fabs(stage.fHitAnchorYawOffsetDegrees) <= 360.f &&
					(BOSS_PATTERN_HIT_SHAPE::NONE != stage.eHitShape ||
					 (BOSS_PATTERN_HIT_ANCHOR_KIND::BOSS_CURRENT ==
						stage.eHitAnchorKind &&
					  0.f == stage.fHitAnchorForwardOffsetM &&
					  0.f == stage.fHitAnchorRightOffsetM &&
					  0.f == stage.fHitAnchorYawOffsetDegrees &&
					  BOSS_PATTERN_HIT_ACTIVATION_KIND::PULSE_SCHEDULE ==
						stage.eHitActivationKind &&
					  0u == stage.iHitActivationStartMs &&
					  0u == stage.iHitActivationLifetimeMs));
				const bool validPortalHitAuthority =
					(BOSS_PATTERN_STAGE_MOTION_KIND::PORTAL_TARGET_RUSH !=
						stage.Motion.eKind &&
					 BOSS_PATTERN_STAGE_MOTION_KIND::PORTAL_CROSS_ARENA !=
						stage.Motion.eKind) ||
					(BOSS_PATTERN_HIT_ANCHOR_KIND::BOSS_CURRENT ==
						stage.eHitAnchorKind &&
					 0.f == stage.fHitAnchorForwardOffsetM &&
					 0.f == stage.fHitAnchorRightOffsetM &&
					 0.f == stage.fHitAnchorYawOffsetDegrees &&
					 BOSS_PATTERN_HIT_ACTIVATION_KIND::PULSE_SCHEDULE ==
						stage.eHitActivationKind);
				const bool validPlayerResponse =
					(BOSS_PATTERN_PLAYER_RESPONSE::DAMAGE ==
						stage.ePlayerResponse &&
					 LostArk::Shared::PLAYER_ATTACHMENT_SLOT::NONE ==
						stage.eAttachmentSlot) ||
					(BOSS_PATTERN_PLAYER_RESPONSE::CAPTURE ==
						stage.ePlayerResponse &&
					 LostArk::Shared::PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND ==
						stage.eAttachmentSlot &&
					 BOSS_PATTERN_HIT_SHAPE::NONE != stage.eHitShape &&
						 0.f == stage.fPushRangeM && 0u == stage.iPushMs &&
						 !stage.bKnockdown && 0u == stage.iDownMs);
				const bool validCounterProxy =
					(!stage.bHasCounterProxy &&
					 BOSS_PATTERN_COUNTER_PROXY_KIND::NONE ==
						stage.eCounterProxyKind &&
					 0.f == stage.fCounterProxyForwardOffsetM &&
					 0.f == stage.fCounterProxyRightOffsetM &&
					 0.f == stage.fCounterProxyRadiusM &&
					 0.f == stage.fCounterProxyArcDegrees) ||
					(stage.bHasCounterProxy &&
					 BOSS_PATTERN_HIT_SHAPE::NONE == stage.eHitShape &&
					 std::isfinite(stage.fCounterProxyForwardOffsetM) &&
					 std::isfinite(stage.fCounterProxyRightOffsetM) &&
					 std::isfinite(stage.fCounterProxyRadiusM) &&
					 std::isfinite(stage.fCounterProxyArcDegrees) &&
					 ((BOSS_PATTERN_COUNTER_PROXY_KIND::BOSS_LOCAL_CIRCLE ==
						stage.eCounterProxyKind &&
					   std::fabs(stage.fCounterProxyForwardOffsetM) <= 20.f &&
					   std::fabs(stage.fCounterProxyRightOffsetM) <= 20.f &&
					   stage.fCounterProxyRadiusM > 0.f &&
					   stage.fCounterProxyRadiusM <= 20.f &&
					   0.f == stage.fCounterProxyArcDegrees) ||
					  (BOSS_PATTERN_COUNTER_PROXY_KIND::BOSS_FORWARD_ARC ==
						stage.eCounterProxyKind &&
					   0.f == stage.fCounterProxyForwardOffsetM &&
					   0.f == stage.fCounterProxyRightOffsetM &&
					   0.f == stage.fCounterProxyRadiusM &&
					   180.f == stage.fCounterProxyArcDegrees)));
				const bool hasBossResponse =
					BOSS_PATTERN_BOSS_RESPONSE_KIND::NONE !=
						stage.eBossResponseKind;
				const bool validBossResponse =
					(!hasBossResponse && 0u == stage.iBossResponseThreshold) ||
					(BOSS_PATTERN_BOSS_RESPONSE_KIND::ACCUMULATED_HEALTH_DAMAGE ==
						stage.eBossResponseKind &&
					 stage.iBossResponseThreshold > 0u &&
					 !pattern.bInvulnerableWhileRunning &&
					 !patternOwnsBossResponse);
				if (hasBossResponse)
					patternOwnsBossResponse = true;
				const std::string stageVerticalOffsetKey =
					pattern.strEncounterId + "\n" + pattern.strPatternId + "\n" +
					stage.strActionId;
				const bool hasStaggerResponse = std::any_of(
					stage.Branches.begin(), stage.Branches.end(),
					[](const BOSS_PATTERN_STAGE_BRANCH& branch)
					{ return branch.eOutcome == BOSS_PATTERN_STAGE_OUTCOME::STAGGER_BROKEN; });
				const bool hasStageVerticalOffset =
					0.f != stage.fVerticalOffsetM;
				const bool validStageVerticalOffset =
					std::isfinite(stage.fVerticalOffsetM) &&
					std::fabs(stage.fVerticalOffsetM) <= 100.f &&
					(!hasStageVerticalOffset ||
					 ((hasBossResponse || hasStaggerResponse) &&
					  BOSS_PATTERN_MOTION_KIND::NONE == pattern.Motion.eKind &&
					  BOSS_PATTERN_STAGE_MOTION_KIND::NONE == stage.Motion.eKind &&
					  patternStageVerticalOffsetOwners.contains(
						stageVerticalOffsetKey)));
				if (!validShape || !validHitAnchor || !validPortalHitAuthority ||
					!validPlayerResponse ||
					!validCounterProxy || !validBossResponse ||
					!validStageVerticalOffset ||
					(stage.bWallContact &&
						(BOSS_PATTERN_STAGE_KIND::ACTIVE != stage.eStageKind ||
						 BOSS_PATTERN_HIT_SHAPE::NONE == stage.eHitShape ||
						 0u == stage.iHitCount)) ||
					std::any_of(pattern.Stages.begin() + stageIndex + 1u,
						pattern.Stages.end(),
						[&stage](const BOSS_PATTERN_STAGE_DEFINITION& other)
						{
							return stage.strStageId == other.strStageId ||
								stage.strActionId == other.strActionId;
						}))
				{
					m_strStatus = "Boss pattern stage hit contract is invalid";
					return false;
				}
				const std::size_t timeoutCount = static_cast<std::size_t>(
					std::count_if(stage.Branches.begin(), stage.Branches.end(),
						[](const BOSS_PATTERN_STAGE_BRANCH& branch)
						{
							return BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT ==
								branch.eOutcome;
						}));
				bool validBranches = 1u == timeoutCount && !stage.Branches.empty();
				bool hasWallContactOutcome = false;
				bool hasHealthDamageThresholdOutcome = false;
				for (const BOSS_PATTERN_STAGE_BRANCH& branch : stage.Branches)
				{
					validBranches = validBranches &&
						(branch.strNextActionId.empty() ||
						 branch.strNextPatternId.empty());
					if (!branch.strNextActionId.empty())
					{
						const auto target = std::find_if(
							pattern.Stages.begin(), pattern.Stages.end(),
							[&branch](const BOSS_PATTERN_STAGE_DEFINITION& candidate)
							{
								return candidate.strActionId == branch.strNextActionId;
							});
						validBranches = validBranches &&
							target != pattern.Stages.end() &&
							branch.strNextActionId != stage.strActionId;
					}
					if (!branch.strNextPatternId.empty())
					{
						const auto target = std::find_if(
							foundPatterns->second.begin(), foundPatterns->second.end(),
							[&branch](const BOSS_PATTERN_DEFINITION& candidate)
							{
								return candidate.strPatternId ==
									branch.strNextPatternId;
							});
						validBranches = validBranches &&
							target != foundPatterns->second.end() &&
							branch.strNextPatternId != pattern.strPatternId &&
							BOSS_PATTERN_SELECTION::AUDITION_ONLY ==
								target->eSelection &&
							BOSS_PATTERN_TARGET_POLICY::NONE ==
								target->eTargetPolicy &&
							BOSS_PATTERN_AIM_POLICY::NONE == target->eAimPolicy;
						if (BOSS_PATTERN_STAGE_OUTCOME::
							HEALTH_DAMAGE_THRESHOLD_REACHED == branch.eOutcome)
						{
							validBranches = validBranches &&
								target != foundPatterns->second.end() &&
								!target->Stages.empty() &&
								BOSS_PATTERN_STAGE_KIND::GROGGY ==
									target->Stages.front().eStageKind;
						}
					}
					if (BOSS_PATTERN_STAGE_OUTCOME::
						HEALTH_DAMAGE_THRESHOLD_REACHED == branch.eOutcome)
					{
						hasHealthDamageThresholdOutcome = true;
						validBranches = validBranches && hasBossResponse &&
							!branch.strNextPatternId.empty();
					}
					if (BOSS_PATTERN_STAGE_OUTCOME::WALL_CONTACT == branch.eOutcome)
					{
						hasWallContactOutcome = true;
						validBranches = validBranches &&
							BOSS_PATTERN_STAGE_MOTION_KIND::FORWARD ==
								stage.Motion.eKind;
					}
					if (BOSS_PATTERN_STAGE_OUTCOME::NAVIGATION_BLOCKED ==
						branch.eOutcome)
					{
						validBranches = validBranches &&
							BOSS_PATTERN_PLAYER_RESPONSE::CAPTURE == stage.ePlayerResponse &&
							(BOSS_PATTERN_STAGE_MOTION_KIND::FORWARD == stage.Motion.eKind ||
								!stage.Motion.RootMotion.empty());
					}
				}
				validBranches = validBranches &&
					(hasBossResponse == hasHealthDamageThresholdOutcome);
				if (stage.bChargeImpact && !hasWallContactOutcome)
					validBranches = false;
				bool validActions = true;
				bool hasScheduledVolleyClock = false;
				std::uint32_t scheduledVolleyFirstOffsetMs = 0u;
				std::uint32_t scheduledVolleyCount = 0u;
				std::uint32_t scheduledVolleyIntervalMs = 0u;
				for (const BOSS_PATTERN_STAGE_ACTION& action : stage.Actions)
				{
					if (BOSS_PATTERN_STAGE_ACTION_KIND::
							SPAWN_COMBAT_OBJECT_VOLLEY != action.eKind &&
						!IsValidBossPatternStageAction(
							action.eTrigger, action.eKind, action.strTargetId,
							action.iValue, action.iDurationMs,
							action.eReleaseMode, action.fReleaseSpeedMps,
							action.fReleaseYawOffsetDegrees))
					{
						validActions = false;
						continue;
					}
					if (BOSS_PATTERN_STAGE_ACTION_KIND::SET_PLAYER_SILENCE ==
						action.eKind && action.iDurationMs < stage.iDurationMs)
					{
						validActions = false;
						continue;
					}
					if (BOSS_PATTERN_STAGE_ACTION_KIND::SET_GAMEPLAY_PHASE ==
						action.eKind)
					{
						++gameplayPhaseActionCount;
						const bool exactArenaBreak =
							BOSS_PATTERN_SELECTION::HEALTH_BAR == pattern.eSelection &&
							BOSS_PATTERN_CATEGORY::MECHANIC == pattern.eCategory &&
							"VALTAN_ARENA_BREAK_109" == pattern.strPatternId &&
							"IMPACT" == stage.strStageId &&
							"valtan.mechanic.arena-break-109.impact" ==
								stage.strActionId &&
							BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER ==
								action.eTrigger &&
							"boss.phase.gameplay" == action.strTargetId &&
							2u == action.iValue && 0u == action.iDurationMs;
						const bool exactGhostRespawn =
							BOSS_PATTERN_SELECTION::AUDITION_ONLY == pattern.eSelection &&
							BOSS_PATTERN_CATEGORY::NORMAL == pattern.eCategory &&
							"VALTAN_GHOST_RESPAWN_AUDITION" == pattern.strPatternId &&
							"STEP_01" == stage.strStageId &&
							"valtan.sequence.respawn.step-01" == stage.strActionId &&
							BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER ==
								action.eTrigger &&
							"boss.phase.gameplay" == action.strTargetId &&
							3u == action.iValue && 0u == action.iDurationMs;
						validActions = validActions &&
							(exactArenaBreak || exactGhostRespawn);
						if ("BOSS_VALTAN" == archetypeId &&
							"ENCOUNTER_VALTAN" == boss.strEncounterId &&
							exactArenaBreak)
						{
							hasExactValtanArenaBreakPhaseAction = true;
						}
						if ("BOSS_VALTAN" == archetypeId &&
							"ENCOUNTER_VALTAN" == boss.strEncounterId &&
							exactGhostRespawn)
						{
							hasExactValtanGhostRespawnPhaseAction = true;
						}
						continue;
					}
					if (BOSS_PATTERN_STAGE_ACTION_KIND::SPAWN_COMBAT_OBJECT ==
							action.eKind ||
						BOSS_PATTERN_STAGE_ACTION_KIND::SPAWN_COMBAT_OBJECT_VOLLEY ==
							action.eKind)
					{
						const auto combatObject = m_BossCombatObjects.find(
							action.strTargetId);
						const bool targetLocksOnStart =
							BOSS_PATTERN_TARGET_POLICY::LOCK_NEAREST_ON_START ==
								pattern.eTargetPolicy ||
							BOSS_PATTERN_TARGET_POLICY::LOCK_RANDOM_ALIVE_ON_START ==
								pattern.eTargetPolicy ||
							BOSS_PATTERN_TARGET_POLICY::
								LOCK_RANDOM_ALIVE_BEHIND_ON_START ==
								pattern.eTargetPolicy;
						const bool fixedArea = combatObject !=
							m_BossCombatObjects.end() &&
							BOSS_COMBAT_OBJECT_KIND::FIXED_AREA ==
								combatObject->second.eKind;
						const bool isVolley =
							BOSS_PATTERN_STAGE_ACTION_KIND::
								SPAWN_COMBAT_OBJECT_VOLLEY == action.eKind;
						// player별 volley는 stage 진입의 새 pose를 읽으며 pattern이 고정한 단일 target을 사용하지 않는다.
						const bool perPlayerFixedAreaVolley = fixedArea && isVolley &&
							BOSS_COMBAT_OBJECT_VOLLEY_POLICY::PER_ALIVE_PLAYER == action.Volley.ePolicy &&
							BOSS_COMBAT_OBJECT_ORIGIN_POLICY::LOCKED_TARGET_PER_ALIVE_PLAYER ==
								combatObject->second.eOriginPolicy;
						const bool isStrugglingUnderfootVolley = [&]()
						{
							if ("VALTAN_STRUGGLING" != pattern.strPatternId)
								return false;
							for (std::uint32_t ordinal = 1u; ordinal <= 6u; ++ordinal)
							{
								const std::string suffix = "0" + std::to_string(ordinal);
								if ("STEP_06_TARGET_" + suffix == stage.strStageId &&
									"valtan.sequence.warp-jump-four-hand-twohand-roar-roar-dead.step-06.target-" + suffix == stage.strActionId &&
									"combatobject.valtan.struggling.underfoot-" + suffix == action.strTargetId)
									return true;
							}
							return false;
						}();
						const bool allowsIndependentInlineHitRockVolley =
							isVolley && 1u == stage.Actions.size() &&
							(isStrugglingUnderfootVolley ||
							 ("VALTAN_GROUND_ROAR" == pattern.strPatternId &&
							  "STEP_01" == stage.strStageId &&
							  "valtan.sequence.sequence.400440.0.step-01" ==
								stage.strActionId &&
							  "combatobject.valtan.ground-roar.rock" ==
								action.strTargetId) ||
							 ("VALTAN_STRUGGLING" == pattern.strPatternId &&
							  "STEP_04" == stage.strStageId &&
							  "valtan.sequence.warp-jump-four-hand-twohand-roar-roar-dead.step-04" ==
								stage.strActionId &&
							  "combatobject.valtan.struggling.rock-pillar" ==
								action.strTargetId) ||
							 ("VALTAN_STRUGGLING" == pattern.strPatternId &&
							  "STEP_08" == stage.strStageId &&
							  "valtan.sequence.warp-jump-four-hand-twohand-roar-roar-dead.step-08" == stage.strActionId &&
							  "combatobject.valtan.struggling.rock-pillar" == action.strTargetId) ||
							 ("VALTAN_TERRAIN_DESTRUCTION_3_OCLOCK" == pattern.strPatternId &&
							  "COMBO_STEP_18" == stage.strStageId &&
							  "valtan.mechanic.terrain-destruction-3.combo.step-18" == stage.strActionId &&
							  "combatobject.valtan.terrain-3.combo-rock" == action.strTargetId) ||
							 ("VALTAN_TERRAIN_DESTRUCTION_9_OCLOCK" == pattern.strPatternId &&
							  "COMBO_STEP_18" == stage.strStageId &&
							  "valtan.mechanic.terrain-destruction-9.combo.step-18" == stage.strActionId &&
							  "combatobject.valtan.terrain-9.combo-rock" == action.strTargetId));
						const bool isScheduledVolley = isVolley &&
							(0u != action.Volley.iFirstSpawnOffsetMs ||
							 action.Volley.iSpawnCount > 1u);
						if (isScheduledVolley)
						{
							if (!hasScheduledVolleyClock)
							{
								hasScheduledVolleyClock = true;
								scheduledVolleyFirstOffsetMs =
									action.Volley.iFirstSpawnOffsetMs;
								scheduledVolleyCount = action.Volley.iSpawnCount;
								scheduledVolleyIntervalMs =
									action.Volley.iSpawnIntervalMs;
							}
							else if (scheduledVolleyFirstOffsetMs !=
									action.Volley.iFirstSpawnOffsetMs ||
								scheduledVolleyCount != action.Volley.iSpawnCount ||
								scheduledVolleyIntervalMs !=
									action.Volley.iSpawnIntervalMs)
							{
								validActions = false;
							}
						}
						const bool isValtanGhostPortal = isVolley &&
							"BOSS_VALTAN" == archetypeId &&
							"ENCOUNTER_VALTAN" == boss.strEncounterId &&
							"VALTAN_GHOST_PORTAL_ONCE" == pattern.strPatternId;
						if (isValtanGhostPortal)
							++valtanGhostPortalTypedVolleyCount;
						if (isVolley && "BOSS_VALTAN" == archetypeId &&
							"ENCOUNTER_VALTAN" == boss.strEncounterId &&
							"VALTAN_HIGH_JUMP" == pattern.strPatternId)
						{
							++valtanHighJumpTypedVolleyCount;
						}
						if (isVolley && "BOSS_VALTAN" == archetypeId &&
							"ENCOUNTER_VALTAN" == boss.strEncounterId &&
							"VALTAN_HIGH_JUMP" == pattern.strPatternId &&
							"AIRBORNE" == stage.strStageId &&
							"valtan.attack.high-jump.airborne" == stage.strActionId &&
							stage.iDurationMs >= 4000u &&
							!stage.Actions.empty() && &action == &stage.Actions.front() &&
							"combatobject.valtan.high-jump.target-axe" ==
								action.strTargetId &&
							BOSS_COMBAT_OBJECT_VOLLEY_POLICY::PER_ALIVE_PLAYER ==
								action.Volley.ePolicy &&
							1u == action.Volley.iCountPerResolvedTarget &&
							BOSS_COMBAT_OBJECT_LAYOUT_KIND::SINGLE ==
								action.Volley.eLayout &&
							!action.Volley.bAllowOverlap &&
							/* 1333ms 간격의 3파동이다. 생존 player별 추적 도끼와 spawn 주변 무작위 4개를 포함해 총 36개로 제한한다. */
							36u == action.Volley.iMaximumTotalObjects &&
							3u == action.Volley.iSpawnCount &&
							0u == action.Volley.iFirstSpawnOffsetMs &&
							1333u == action.Volley.iSpawnIntervalMs &&
							4u == action.Volley.iArenaRandomCount &&
							14.f == action.Volley.fArenaRandomRadiusM &&
							1.f == action.Volley.fArenaHeightToleranceM &&
							BOSS_COMBAT_OBJECT_ARENA_ANCHOR_POLICY::BOSS_SPAWN_POSITION ==
								action.Volley.eArenaAnchorPolicy)
						{
							hasExactValtanHighJumpTypedVolley = true;
						}
						const bool validBossRelativeMissile =
							m_BossCombatObjects.end() != combatObject &&
							(BOSS_COMBAT_OBJECT_DIRECTION_POLICY::RADIAL_INWARD ==
								combatObject->second.eDirectionPolicy ||
							 BOSS_COMBAT_OBJECT_DIRECTION_POLICY::NEXT_RADIAL_SLOT ==
								combatObject->second.eDirectionPolicy) &&
							BOSS_COMBAT_OBJECT_VOLLEY_POLICY::BOSS_RELATIVE ==
								action.Volley.ePolicy;
						const bool validNextRadialSlotLayout =
							m_BossCombatObjects.end() == combatObject ||
							BOSS_COMBAT_OBJECT_DIRECTION_POLICY::NEXT_RADIAL_SLOT !=
								combatObject->second.eDirectionPolicy ||
							(isVolley &&
							 BOSS_COMBAT_OBJECT_VOLLEY_POLICY::BOSS_RELATIVE ==
								action.Volley.ePolicy &&
							 BOSS_COMBAT_OBJECT_LAYOUT_KIND::RADIAL == action.Volley.eLayout &&
							 action.Volley.iCountPerResolvedTarget >= 2u &&
							 action.Volley.iMaximumTotalObjects ==
								action.Volley.iCountPerResolvedTarget);
						if (isValtanGhostPortal &&
							"ACTIVE" == stage.strStageId &&
							"valtan.ghost.portal-once.active" == stage.strActionId &&
							1900u == stage.iDurationMs &&
							!stage.Actions.empty() && &action == &stage.Actions.front() &&
							"combatobject.valtan.ghost.portal-charge" ==
								action.strTargetId &&
							BOSS_COMBAT_OBJECT_VOLLEY_POLICY::BOSS_RELATIVE ==
								action.Volley.ePolicy &&
							3u == action.Volley.iCountPerResolvedTarget &&
							BOSS_COMBAT_OBJECT_LAYOUT_KIND::RADIAL ==
								action.Volley.eLayout &&
							std::fabs(action.Volley.fRadiusM - 13.5f) < 0.0001f &&
							30.f == action.Volley.fStartAngleDegrees &&
							120.f == action.Volley.fAngleStepDegrees &&
							!action.Volley.bAllowOverlap &&
							3u == action.Volley.iMaximumTotalObjects &&
							1u == action.Volley.iSpawnCount &&
							0u == action.Volley.iFirstSpawnOffsetMs &&
							0u == action.Volley.iSpawnIntervalMs &&
							0u == action.Volley.iArenaRandomCount &&
							std::fabs(combatObject->second.fSpeedMps - 17.9866814632f) <
								0.0001f &&
							std::fabs(combatObject->second.fMaximumDistanceM -
								23.3826859022f) < 0.0001f &&
							300u == combatObject->second.iMovementStartDelayMs &&
							!combatObject->second.bExpireOnDistanceEnd &&
							1900u == combatObject->second.iLifeMs)
						{
							hasExactValtanGhostPortalTypedVolley = true;
						}
						if (m_BossCombatObjects.end() == combatObject ||
							combatObject->second.strEncounterId != pattern.strEncounterId ||
							combatObject->second.strOwnerPatternId != pattern.strPatternId ||
							combatObject->second.strOwnerStageActionId !=
								stage.strActionId ||
							(!allowsIndependentInlineHitRockVolley &&
							 BOSS_PATTERN_HIT_SHAPE::NONE != stage.eHitShape) ||
							!spawnedBossCombatObjectIds.insert(
								action.strTargetId).second ||
							!validNextRadialSlotLayout ||
							(fixedArea && !targetLocksOnStart && !perPlayerFixedAreaVolley &&
								BOSS_COMBAT_OBJECT_ORIGIN_POLICY::BOSS_POSITION !=
									combatObject->second.eOriginPolicy) ||
							(isVolley &&
							 ((BOSS_COMBAT_OBJECT_VOLLEY_POLICY::PER_ALIVE_PLAYER ==
									action.Volley.ePolicy &&
								BOSS_COMBAT_OBJECT_ORIGIN_POLICY::
									LOCKED_TARGET_PER_ALIVE_PLAYER !=
									combatObject->second.eOriginPolicy) ||
							  ((BOSS_COMBAT_OBJECT_VOLLEY_POLICY::BOSS_RELATIVE ==
									action.Volley.ePolicy ||
								BOSS_COMBAT_OBJECT_VOLLEY_POLICY::ARENA_CENTER ==
									action.Volley.ePolicy) &&
								BOSS_COMBAT_OBJECT_ORIGIN_POLICY::BOSS_POSITION !=
									combatObject->second.eOriginPolicy) ||
							  BOSS_COMBAT_OBJECT_VOLLEY_POLICY::NONE ==
									action.Volley.ePolicy)) ||
							(!fixedArea && !validBossRelativeMissile &&
							 BOSS_PATTERN_AIM_POLICY::LOCK_FACING_ON_START !=
								pattern.eAimPolicy))
						{
							m_strStatus =
								"Boss combat object spawn action join is invalid: " +
								pattern.strPatternId + "/" + stage.strStageId + "/" + action.strTargetId;
							return false;
						}
						continue;
					}
					if (BOSS_PATTERN_STAGE_ACTION_KIND::DAMAGE_GRABBED_PLAYERS ==
						action.eKind && 0u == Find_DamageRatePercent(action.strTargetId))
					{
						m_strStatus = "Boss grabbed-player damage profile is missing";
						return false;
					}
					if ((BOSS_PATTERN_STAGE_ACTION_KIND::DAMAGE_GRABBED_PLAYERS == action.eKind ||
						BOSS_PATTERN_STAGE_ACTION_KIND::EXECUTE_GRABBED_PLAYERS == action.eKind) &&
						(stage.Actions.size() != 1u || stage.eHitShape != BOSS_PATTERN_HIT_SHAPE::NONE))
					{
						m_strStatus = "Boss grabbed-player impact must own its stage transaction";
						return false;
					}
					if (!IsStatefulBossPatternStageAction(action.eKind))
						continue;
					const std::string stateKey =
						BuildBossPatternStageActionStateKey(
							action.eKind, action.strTargetId);
					if (BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER == action.eTrigger)
						validActions = activeStageActions.insert(stateKey).second &&
							validActions;
					else
						validActions = 1u == activeStageActions.erase(stateKey) &&
							validActions;
				}
				const bool hasCounterHit = std::any_of(
					stage.Branches.begin(), stage.Branches.end(),
					[](const BOSS_PATTERN_STAGE_BRANCH& branch)
					{
						return BOSS_PATTERN_STAGE_OUTCOME::COUNTER_HIT ==
							branch.eOutcome;
					});
				const bool hasPartDestroyed = std::any_of(
					stage.Branches.begin(), stage.Branches.end(),
					[](const BOSS_PATTERN_STAGE_BRANCH& branch)
					{
						return BOSS_PATTERN_STAGE_OUTCOME::PART_DESTROYED ==
							branch.eOutcome;
					});
				const bool hasStaggerBroken = std::any_of(
					stage.Branches.begin(), stage.Branches.end(),
					[](const BOSS_PATTERN_STAGE_BRANCH& branch)
					{
						return BOSS_PATTERN_STAGE_OUTCOME::STAGGER_BROKEN ==
							branch.eOutcome;
					});
				const auto hasStageAction = [&stage](
					const BOSS_PATTERN_STAGE_ACTION_TRIGGER trigger,
					const BOSS_PATTERN_STAGE_ACTION_KIND kind,
					const std::string_view targetId)
				{
					return std::any_of(stage.Actions.begin(), stage.Actions.end(),
						[trigger, kind, targetId](
							const BOSS_PATTERN_STAGE_ACTION& action)
						{
							return trigger == action.eTrigger && kind == action.eKind &&
								targetId == action.strTargetId;
						});
				};
				const auto hasBossFlagValue = [&stage](
					const BOSS_PATTERN_STAGE_ACTION_TRIGGER trigger,
					const std::string_view targetId,
					const std::uint32_t value)
				{
					return std::any_of(stage.Actions.begin(), stage.Actions.end(),
						[trigger, targetId, value](
							const BOSS_PATTERN_STAGE_ACTION& action)
						{
							return trigger == action.eTrigger &&
								BOSS_PATTERN_STAGE_ACTION_KIND::SET_BOSS_FLAG ==
									action.eKind &&
								targetId == action.strTargetId &&
								value == action.iValue;
						});
				};
				const auto hasBossFlag = [&stage](const std::string_view targetId)
				{
					return std::any_of(stage.Actions.begin(), stage.Actions.end(),
						[targetId](const BOSS_PATTERN_STAGE_ACTION& action)
						{
							return BOSS_PATTERN_STAGE_ACTION_KIND::SET_BOSS_FLAG ==
									action.eKind && targetId == action.strTargetId;
						});
				};
				const bool hasCounterableFlag = hasBossFlag("boss.flag.counterable");
				const bool hasGroggyFlag = hasBossFlag("boss.flag.groggy");
				if (pattern.bAuthoringMasterManaged && hasCounterHit)
				{
					const auto counterBranch = std::find_if(
						stage.Branches.begin(), stage.Branches.end(),
						[](const BOSS_PATTERN_STAGE_BRANCH& branch)
						{
							return BOSS_PATTERN_STAGE_OUTCOME::COUNTER_HIT ==
								branch.eOutcome;
						});
					const std::size_t sourceIndex = static_cast<std::size_t>(
						&stage - pattern.Stages.data());
					const BOSS_PATTERN_STAGE_DEFINITION* counterTarget = nullptr;
					bool counterTargetIsForward = false;
					if (counterBranch != stage.Branches.end() &&
						!counterBranch->strNextActionId.empty())
					{
						const auto localTarget = std::find_if(
							pattern.Stages.begin(), pattern.Stages.end(),
							[&counterBranch](
								const BOSS_PATTERN_STAGE_DEFINITION& candidate)
							{
								return candidate.strActionId ==
									counterBranch->strNextActionId;
							});
						if (localTarget != pattern.Stages.end())
						{
							counterTarget = &*localTarget;
							counterTargetIsForward = static_cast<std::size_t>(
								localTarget - pattern.Stages.begin()) > sourceIndex;
						}
					}
					else if (counterBranch != stage.Branches.end() &&
						!counterBranch->strNextPatternId.empty())
					{
						const auto followupPattern = std::find_if(
							foundPatterns->second.begin(), foundPatterns->second.end(),
							[&counterBranch](const BOSS_PATTERN_DEFINITION& candidate)
							{
								return candidate.strPatternId ==
									counterBranch->strNextPatternId;
							});
						if (followupPattern != foundPatterns->second.end() &&
							!followupPattern->Stages.empty())
						{
							counterTarget = &followupPattern->Stages.front();
							/* pattern 간 순서는 각자의 stage index가 아닌 유한 follow-up graph가 결정한다. */
							counterTargetIsForward = true;
						}
					}
					const auto timeoutBranch = std::find_if(
						stage.Branches.begin(), stage.Branches.end(),
						[](const BOSS_PATTERN_STAGE_BRANCH& branch)
						{
							return BOSS_PATTERN_STAGE_OUTCOME::TIMEOUT ==
								branch.eOutcome;
						});
					const auto timeoutTarget = timeoutBranch == stage.Branches.end() ?
						pattern.Stages.end() : std::find_if(
							pattern.Stages.begin(), pattern.Stages.end(),
							[&timeoutBranch](
								const BOSS_PATTERN_STAGE_DEFINITION& candidate)
							{
								return candidate.strActionId ==
									timeoutBranch->strNextActionId;
							});
					const bool timeoutTargetIsForward =
						timeoutTarget != pattern.Stages.end() &&
						static_cast<std::size_t>(
							timeoutTarget - pattern.Stages.begin()) > sourceIndex;
					const bool counterTargetKindIsSupported =
						nullptr != counterTarget &&
						(BOSS_PATTERN_STAGE_KIND::WINDUP ==
							counterTarget->eStageKind ||
						 BOSS_PATTERN_STAGE_KIND::GROGGY ==
							counterTarget->eStageKind ||
						 BOSS_PATTERN_STAGE_KIND::RECOVERY ==
							counterTarget->eStageKind);
					const bool counterTargetIsGroggy =
						nullptr != counterTarget &&
						BOSS_PATTERN_STAGE_KIND::GROGGY ==
							counterTarget->eStageKind;
					const auto targetHasGroggyAction =
						[&counterTarget](
							const BOSS_PATTERN_STAGE_ACTION_TRIGGER trigger,
							const std::uint32_t value)
					{
							return nullptr != counterTarget &&
								std::any_of(
									counterTarget->Actions.begin(),
									counterTarget->Actions.end(),
									[trigger, value](
										const BOSS_PATTERN_STAGE_ACTION& action)
									{
										return trigger == action.eTrigger &&
											BOSS_PATTERN_STAGE_ACTION_KIND::SET_BOSS_FLAG ==
												action.eKind &&
											"boss.flag.groggy" == action.strTargetId &&
											value == action.iValue;
									});
						};
					const bool counterSourceKindIsSupported =
						BOSS_PATTERN_STAGE_KIND::WINDUP == stage.eStageKind ||
						(BOSS_PATTERN_STAGE_KIND::ACTIVE == stage.eStageKind &&
						 BOSS_PATTERN_COUNTER_PROXY_KIND::BOSS_FORWARD_ARC ==
							stage.eCounterProxyKind);
					validBranches = validBranches &&
						counterSourceKindIsSupported &&
						counterTargetIsForward && timeoutTargetIsForward &&
						counterTargetKindIsSupported;
					validActions = validActions &&
						hasBossFlagValue(
							BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER,
							"boss.flag.counterable", 1u) &&
						hasBossFlagValue(
							BOSS_PATTERN_STAGE_ACTION_TRIGGER::EXIT,
							"boss.flag.counterable", 0u) &&
						(!counterTargetIsGroggy ||
						 (targetHasGroggyAction(
							 BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER, 1u) &&
						  targetHasGroggyAction(
							  BOSS_PATTERN_STAGE_ACTION_TRIGGER::EXIT, 0u)));
				}
				if (pattern.bAuthoringMasterManaged)
				{
					if (!hasCounterHit && hasCounterableFlag)
						validActions = false;
					if (BOSS_PATTERN_STAGE_KIND::GROGGY == stage.eStageKind)
					{
						validActions = validActions &&
							hasBossFlagValue(
								BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER,
								"boss.flag.groggy", 1u) &&
							hasBossFlagValue(
								BOSS_PATTERN_STAGE_ACTION_TRIGGER::EXIT,
								"boss.flag.groggy", 0u);
					}
					else if (hasGroggyFlag)
					{
						validActions = false;
					}
				}
				if ((stage.bHasCounterProxy && !hasCounterHit) ||
					(BOSS_PATTERN_PART_DAMAGE_POLICY::DESTROY_FIRST_ELIGIBLE ==
						stage.ePartDamagePolicy && !hasPartDestroyed))
				{
					validBranches = false;
				}
				if (hasStaggerBroken)
				{
					validActions = validActions &&
						hasStageAction(BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER,
							BOSS_PATTERN_STAGE_ACTION_KIND::SET_STAGGER_GAUGE,
							"boss.gauge.stagger") &&
						hasStageAction(BOSS_PATTERN_STAGE_ACTION_TRIGGER::EXIT,
							BOSS_PATTERN_STAGE_ACTION_KIND::SET_STAGGER_GAUGE,
							"boss.gauge.stagger");
				}
				if (!validBranches || !validActions)
				{
					m_strStatus =
						"Boss pattern stage branch or action contract is invalid: " +
						pattern.strPatternId + "/" + stage.strStageId;
					return false;
				}
			}
			if (!activeStageActions.empty())
			{
				m_strStatus = "Boss pattern stage action lifetime is not closed";
				return false;
			}
		}
		if (!HasFiniteBossPatternFollowupGraph(foundPatterns->second))
		{
			m_strStatus =
				"Boss cross-pattern follow-up graph is cyclic or dangling";
			return false;
		}
		const bool expectsAuthoredPhaseAction =
			BOSS_PHASE_POLICY_KIND::AUTHORED_PATTERN_EVENT ==
				boss.PhasePolicy.eKind;
		const std::size_t expectedGameplayPhaseActionCount =
			isValtanPrimary ? 2u : (isKoukuSaydonGateOne ? 0u : 1u);
		if ((expectsAuthoredPhaseAction &&
			 expectedGameplayPhaseActionCount != gameplayPhaseActionCount) ||
			(!expectsAuthoredPhaseAction && 0u != gameplayPhaseActionCount))
		{
			m_strStatus =
				"Boss phase policy and gameplay phase actions differ";
			return false;
		}
		if (expectsAuthoredPhaseAction && isValtanPrimary &&
			(!hasExactValtanArenaBreakPhaseAction ||
			 !hasExactValtanGhostRespawnPhaseAction))
		{
			m_strStatus =
				"Valtan authored phase actions are not the exact phase-two/three edges";
			return false;
		}
		if (expectsAuthoredPhaseAction && isValtanPrimary &&
			(1u != valtanHighJumpTypedVolleyCount ||
			 !hasExactValtanHighJumpTypedVolley))
		{
			m_strStatus =
				"Valtan high jump does not own the typed AIRBORNE volley";
			return false;
		}
		if (expectsAuthoredPhaseAction && isValtanPrimary &&
			(1u != valtanGhostPortalTypedVolleyCount ||
			 !hasExactValtanGhostPortalTypedVolley))
		{
			m_strStatus =
				"Valtan ghost portal does not own the exact simultaneous triangle volley";
			return false;
		}
	}
	std::unordered_set<std::string> bossCombatObjectVisualIds;
	for (const auto& [combatObjectArchetypeId, combatObject] :
		m_BossCombatObjects)
	{
		(void)combatObjectArchetypeId;
		if (combatObject.Hits.size() + combatObject.PresentationPulses.size() !=
				combatObject.iExpectedEventCount ||
			(combatObject.fCoverRadiusM > 0.f &&
				(BOSS_COMBAT_OBJECT_KIND::FIXED_AREA != combatObject.eKind ||
				 combatObject.Hits.empty() ||
				 std::any_of(combatObject.Hits.begin(), combatObject.Hits.end(),
					[](const BOSS_COMBAT_OBJECT_HIT& hit)
					{
						return BOSS_COMBAT_OBJECT_HIT_TRIGGER::TIMED != hit.eTrigger;
					}))) ||
			!bossCombatObjectVisualIds.insert(combatObject.strClientVisualId).second)
		{
			m_strStatus = "Boss combat object event count or visual ID is invalid";
			return false;
		}
		if (!combatObject.OwnerHitChain.strTriggerActionId.empty())
		{
			const auto* patterns = Find_BossPatterns(combatObject.strEncounterId);
			const BOSS_PATTERN_STAGE_DEFINITION* triggerStage = nullptr;
			if (patterns != nullptr)
				for (const auto& pattern : *patterns)
					if (pattern.strPatternId == combatObject.strOwnerPatternId)
						for (const auto& stage : pattern.Stages)
							if (stage.strActionId == combatObject.OwnerHitChain.strTriggerActionId)
								triggerStage = &stage;
			if (combatObject.eKind != BOSS_COMBAT_OBJECT_KIND::FIXED_AREA || combatObject.fCoverRadiusM <= 0.f ||
				combatObject.Hits.size() != 1u || combatObject.Hits[0].iRepeatCount != 1u ||
				combatObject.Hits[0].eTrigger != BOSS_COMBAT_OBJECT_HIT_TRIGGER::TIMED ||
				!combatObject.PresentationPulses.empty() || triggerStage == nullptr ||
				triggerStage->eHitShape != BOSS_PATTERN_HIT_SHAPE::CONE || triggerStage->iHitCount == 0u ||
				static_cast<std::uint64_t>(combatObject.OwnerHitChain.iDelayMs) + combatObject.Hits[0].iAtMs >= combatObject.iLifeMs ||
				combatObject.OwnerHitChain.strArmedPresentationEventId == combatObject.Hits[0].strHitId)
			{
				m_strStatus = "Boss combat object owner hit chain requires one timed hit and an owner cone action";
				return false;
			}
		}
		for (const BOSS_COMBAT_OBJECT_HIT& hit : combatObject.Hits)
		{
			if (0u == Find_DamageRatePercent(hit.strDamageProfileId))
			{
				m_strStatus =
					"Boss combat object references missing damage profile";
				return false;
			}
		}
	}
	if (spawnedBossCombatObjectIds.size() != m_BossCombatObjects.size())
	{
		m_strStatus = "Boss combat object definitions and spawn actions differ";
		return false;
	}
	std::size_t patternCount = 0u;
	for (const auto& [encounterId, patterns] : m_BossPatterns)
	{
		(void)encounterId;
		patternCount += patterns.size();
		for (const auto& pattern : patterns) for (const auto& trigger : pattern.MechanicTriggers)
		{
			if (trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BINGO_BOARD && !trigger.BingoHammerHalfExtentsM)
			{ m_strStatus = "Bingo board is missing its published WORLD head geometry"; return false; }
			if (trigger.bBingoControlOnly && (pattern.strGateId != "BINGO" || pattern.MechanicTriggers.size() != 1u ||
				!pattern.LogicWindows.empty() || !pattern.WorldSequences.empty() || !pattern.SceneProfiles.empty() ||
				!pattern.ParentChildren.empty() || pattern.BossMotion || pattern.bResetBossToSpawn || pattern.ResetBossYawDegrees ||
				pattern.Motion.eKind != BOSS_PATTERN_MOTION_KIND::NONE || pattern.fVerticalOffsetM != 0.f ||
				std::any_of(pattern.Stages.begin(), pattern.Stages.end(), [](const auto& stage) {
					return stage.eHitShape != BOSS_PATTERN_HIT_SHAPE::NONE || !stage.Actions.empty() ||
						!stage.AttackContacts.empty() || stage.Motion.eKind != BOSS_PATTERN_STAGE_MOTION_KIND::NONE ||
						!stage.Motion.RootMotion.empty(); })))
			{ m_strStatus = "Bingo control-only marker cannot own another active lane"; return false; }
		}
        for (const auto& parent : patterns) if (!parent.ParentChildren.empty())
        {
            const auto failParent = [&](const char* reason) { m_strStatus = "Parent " + parent.strPatternId + ": " + reason; return false; };
            const bool repeating = !parent.strParentLoopStartOccurrenceId.empty();
            if (repeating && (parent.strGateId != "BINGO" || parent.strTargetBossPlacementId != "boss.koukusaydon.bingo.saydon" ||
                parent.BossMotion || parent.bResetBossToSpawn ||
                !parent.LogicWindows.empty() || !parent.WorldSequences.empty() || !parent.SceneProfiles.empty() ||
                parent.MechanicTriggers.size() != 1u || parent.MechanicTriggers.front().eKind != BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BINGO_BOARD ||
                parent.MechanicTriggers.front().iStartMs != 0u || parent.MechanicTriggers.front().iDurationMs != parent.iTimelineDurationMs))
                return failParent("Retained owner must contain only one continuous Bingo board");
            bool loop = !repeating; std::uint64_t end = 0u;
            if (!repeating) for (const auto& stage : parent.Stages) end += stage.iDurationMs;
            for (const auto& entry : parent.ParentChildren)
            {
                loop = loop || entry.strOccurrenceId == parent.strParentLoopStartOccurrenceId;
                const auto child = std::find_if(patterns.begin(), patterns.end(), [&](const auto& row) { return row.strPatternId == entry.strPatternId; });
                if (child == patterns.end() || child->Stages.empty() || !child->ParentChildren.empty() ||
                    child->strPatternId == parent.strPatternId || child->strGateId != parent.strGateId ||
                    child->strTargetBossPlacementId != parent.strTargetBossPlacementId || child->AuditionBossArchetypeIds != parent.AuditionBossArchetypeIds)
                    return failParent("Child must be a complete independent Pattern on the same boss");
                std::uint32_t stageMs = 0u; for (const auto& stage : child->Stages) stageMs += stage.iDurationMs;
                if (entry.iDurationMs != (std::max)(stageMs, child->iTimelineDurationMs) || entry.iStartMs < end)
                    return failParent("Child window must preserve its complete source duration");
                for (const auto& window : child->LogicWindows)
                {
                    if (window.eKind == BOSS_PATTERN_LOGIC_KIND::PATTERN_COMPLETION_COUNT) return failParent("Nested completion chains are unsupported");
                    for (const auto* outcomes : {&window.OnSuccess, &window.OnFail, &window.OnTimeout})
                        for (const auto& outcome : *outcomes)
                            if ((repeating && outcome.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::FOLLOWUP_PATTERN) || outcome.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::MARIO_ENTER)
                                return failParent("Child cannot insert another chain");
                }
                end = std::uint64_t(entry.iStartMs) + entry.iDurationMs;
            }
            if (!loop || end != parent.iTimelineDurationMs) return failParent("Missing loop start or invalid Parent duration");
        }
		for (const auto& pattern : patterns)
			for (const auto& window : pattern.LogicWindows)
			{
				const auto fail = [&](const char* reason) { m_strStatus = "Pattern " + pattern.strPatternId + " / " + window.strWindowId + ": " + reason; return false; };
                if (!window.strOwnerWorldOccurrenceId.empty() && std::count_if(pattern.WorldSequences.begin(), pattern.WorldSequences.end(),
                    [&](const auto& cue) { return cue.strOccurrenceId == window.strOwnerWorldOccurrenceId && cue.CombatBody.has_value(); }) != 1)
                    return fail("Contact owner must resolve one damageable WORLD occurrence");
                const auto chainCount = std::count_if(pattern.LogicWindows.begin(), pattern.LogicWindows.end(), [](const auto& row) {
                    return row.eKind == BOSS_PATTERN_LOGIC_KIND::PATTERN_COMPLETION_COUNT;
                });
                if (chainCount > 1) return fail("One pattern may own only one completion chain");
                if (window.eKind == BOSS_PATTERN_LOGIC_KIND::PATTERN_COMPLETION_COUNT)
                {
                    if (window.PatternIds.empty() || window.PatternIds.size() > 16u || window.iCompletionCount < 1u ||
                        window.iCompletionCount > window.PatternIds.size() || !window.CardRegions.empty() ||
                        window.OnSuccess.size() > 1u || (!window.OnSuccess.empty() && window.OnSuccess.front().eKind != BOSS_PATTERN_LOGIC_RESULT_KIND::FOLLOWUP_PATTERN) ||
                        !window.OnFail.empty() || !window.OnTimeout.empty())
                        return fail("Completion chain requires a bounded pool, count and at most one Success followup");
                    std::unordered_set<std::string> candidateIds;
                    for (const auto& id : window.PatternIds)
                    {
                        const auto child = std::find_if(patterns.begin(), patterns.end(), [&](const auto& row) { return row.strPatternId == id; });
                        if (id == pattern.strPatternId || !candidateIds.insert(id).second || child == patterns.end() || child->Stages.empty() ||
                            child->strGateId != pattern.strGateId || child->AuditionBossArchetypeIds != pattern.AuditionBossArchetypeIds ||
                            child->strTargetBossPlacementId != pattern.strTargetBossPlacementId)
                            return fail("Completion candidates must be distinct playable patterns for the same Gate and boss");
                        for (const auto& childWindow : child->LogicWindows)
                        {
                            if (childWindow.eKind == BOSS_PATTERN_LOGIC_KIND::PATTERN_COMPLETION_COUNT)
                                return fail("Completion candidates cannot contain another chain");
                            for (const auto* outcomes : { &childWindow.OnSuccess, &childWindow.OnFail, &childWindow.OnTimeout })
                                for (const auto& outcome : *outcomes)
                                    if (outcome.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::FOLLOWUP_PATTERN || outcome.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::MARIO_ENTER)
                                        return fail("Completion candidates cannot insert another pattern or Mario entry");
                        }
                    }
                }
                else if (!window.PatternIds.empty() || window.iCompletionCount) return fail("Only a completion chain owns candidates");
                for (const auto* outcomes : { &window.OnSuccess, &window.OnFail, &window.OnTimeout })
                    for (const auto& outcome : *outcomes)
                        // 이 window는 chain root와 일반 Gate 3 pattern 모두가 사용하므로 chain은 선택적이다.
                        if (outcome.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::MARIO_ENTER &&
                            (pattern.strGateId != "GATE3" || window.eKind != BOSS_PATTERN_LOGIC_KIND::ENTER_AREA ||
                             outcomes != &window.OnSuccess || outcomes->size() != 1u || window.CardRegions.empty() || outcome.iMarioEntryStage > 4u))
                            return fail("Mario entry is the sole ENTER_AREA Success with regions on a Gate 3 pattern");

				for (const auto* results : { &window.OnSuccess, &window.OnFail, &window.OnTimeout })
					for (const auto& result : *results)
						if (result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::PLAY_WORLD_OBJECT_MOTION &&
							std::count_if(pattern.WorldSequences.begin(), pattern.WorldSequences.end(), [&](const auto& world) {
								return world.strInstanceId == result.strTargetWorldInstanceId;
							}) > 1)
							return fail("Legacy World motion target is ambiguous; use occurrence-bound contact motion");
                const auto captureCount = std::count_if(window.OnSuccess.begin(), window.OnSuccess.end(), [](const auto& result) {
                    return result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::CAPTURE_PLAYER;
                });
                if (captureCount > 1 || (captureCount == 1 && window.OnSuccess.size() != 1u) ||
                    (captureCount == 1) != !window.strHoldLogicOccurrenceId.empty())
                    return fail("One capture Success requires one explicit Hold reference");
                if (captureCount == 1)
                {
                    const auto hold = std::find_if(pattern.LogicWindows.begin(), pattern.LogicWindows.end(),
                        [&](const auto& row) { return row.strWindowId == window.strHoldLogicOccurrenceId; });
                    if (hold == pattern.LogicWindows.end() || hold->eKind != BOSS_PATTERN_LOGIC_KIND::ATTACHMENT_HOLD ||
                        window.CaptureHandKeys.size() < 2u ||
                        window.CaptureHandKeys.front().iTimeMs != hold->iStartMs ||
                        window.CaptureHandKeys.back().iTimeMs != static_cast<std::uint64_t>(hold->iStartMs) + hold->iDurationMs)
                        return fail("Capture hand keys must cover the complete Hold lifetime");
                }
                else if (!window.CaptureHandKeys.empty())
                    return fail("Only CAPTURE_PLAYER owns hand keys");
                if (!window.strHoldLogicOccurrenceId.empty() && window.eKind != BOSS_PATTERN_LOGIC_KIND::ENTER_AREA)
                    return fail("Only ENTER_AREA owns a Hold reference");
                if (window.eKind == BOSS_PATTERN_LOGIC_KIND::ATTACHMENT_HOLD &&
                    (!window.CardRegions.empty() || !window.OnSuccess.empty() || !window.OnFail.empty() ||
                     !window.OnTimeout.empty() || window.bInsideIsFail || window.bEndsPatternOnSuccess || window.fBossChargeDistanceM != 0.f))
                    return fail("Attachment Hold owns only its lifetime");
				if (window.eKind == BOSS_PATTERN_LOGIC_KIND::EXTERNAL_SIGNAL)
				{
					if (window.bInsideIsFail || !window.OnFail.empty() || !window.CardRegions.empty()) return fail("External signal accepts Success/Timeout and no Collider regions or inside Fail");
					continue;
				}
				for (const auto& region : window.CardRegions)
				{
					const auto& track = region.WorldTrack;
					if (!track.bEnabled) continue;
					if (track.Keys.empty()) return fail("Contact WORLD track has no keys");
					if (region.eAnchor != BOSS_LOGIC_REGION_ANCHOR::BOSS_CURRENT) continue;
					if (track.iStartMs != window.iStartMs || track.iDurationMs != window.iDurationMs ||
						track.iStartDelayMs || track.fPlaybackSpeed != 1.f || track.bSmoothStep ||
						track.fBaselineX != 0.f || track.fBaselineY != 0.f || track.fBaselineZ != 0.f || track.fBaselineYawDegrees != 0.f ||
						track.fBaselineScaleX != 1.f || track.fBaselineScaleY != 1.f || track.fBaselineScaleZ != 1.f ||
						track.Keys.front().iTimeMs != 0u || track.Keys.back().iTimeMs != window.iDurationMs)
						return fail("Contact bone track requires the exact trigger clock, endpoints and identity baseline");
					for (const auto& key : track.Keys)
						if (!key.bVisible ||
							key.fScaleX != 1.f || key.fScaleY != 1.f || key.fScaleZ != 1.f)
							return fail("Contact bone track must preserve visibility and authored scale");
				}
				if (window.eKind != BOSS_PATTERN_LOGIC_KIND::OBJECT_CONTACT) continue;
				if (!window.bHasContactGroup || window.ContactTargets.empty() || window.CardRegions.empty() ||
					window.OnSuccess.empty() || !window.OnFail.empty() || !window.OnTimeout.empty() || window.bEndsPatternOnSuccess || window.bInsideIsFail)
					return fail("Object contact requires targets, regions and Success only");
				const std::uint64_t endMs = std::uint64_t(window.iStartMs) + window.iDurationMs;
				for (const auto& target : window.ContactTargets)
				{
					const auto world = std::find_if(pattern.WorldSequences.begin(), pattern.WorldSequences.end(),
						[&](const auto& row) { return row.strOccurrenceId == target.strWorldOccurrenceId; });
					if (world == pattern.WorldSequences.end() || world->strInstanceId != target.strWorldInstanceId ||
						world->iStartMs > window.iStartMs || std::uint64_t(world->iStartMs) + world->iDurationMs < endMs)
						return fail("Contact target WORLD occurrence identity or lifetime differs");
				}
				std::size_t motionCount = 0u;
				for (const auto& result : window.OnSuccess)
				{
					if (result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::PLAY_CONTACT_WORLD_OBJECT_MOTION)
					{
						if (++motionCount > 1u || result.ContactMotions.size() != window.ContactTargets.size())
							return fail("One contact motion result must map every target exactly once");
						for (const auto& mapping : result.ContactMotions)
							if (std::none_of(window.ContactTargets.begin(), window.ContactTargets.end(),
								[&](const auto& target) { return target.strWorldOccurrenceId == mapping.strTargetWorldOccurrenceId; }))
								return fail("Contact motion references a target outside this strike");
					}
					else if (result.eKind == BOSS_PATTERN_LOGIC_RESULT_KIND::COMPLETE_LOGIC_WINDOW)
					{
						const auto parent = std::find_if(pattern.LogicWindows.begin(), pattern.LogicWindows.end(),
							[&](const auto& row) { return row.strWindowId == result.strTargetLogicOccurrenceId; });
						if (parent == pattern.LogicWindows.end() || parent->eKind != BOSS_PATTERN_LOGIC_KIND::EXTERNAL_SIGNAL ||
							parent->iStartMs > window.iStartMs || std::uint64_t(parent->iStartMs) + parent->iDurationMs < endMs)
							return fail("Completion signal needs an enclosing External Signal window");
						if (!result.strContactTargetWorldOccurrenceId.empty() && std::none_of(window.ContactTargets.begin(), window.ContactTargets.end(),
							[&](const auto& target) { return target.strWorldOccurrenceId == result.strContactTargetWorldOccurrenceId; }))
							return fail("Completion signal target is not part of this strike");
					}
					else return fail("Object contact has an unsupported result");
				}
				if (!window.strContactGroupId.empty())
					for (const auto& other : pattern.LogicWindows)
					{
						if (&other == &window || other.eKind != BOSS_PATTERN_LOGIC_KIND::OBJECT_CONTACT ||
							other.strContactGroupId != window.strContactGroupId ||
							(std::uint64_t(other.iStartMs) * 30u + 999u) / 1000u != (std::uint64_t(window.iStartMs) * 30u + 999u) / 1000u) continue;
						if (other.iStartMs != window.iStartMs || other.iDurationMs != window.iDurationMs || other.iContactPriority == window.iContactPriority)
							return fail("One contact group strike requires the same window and distinct priorities");
					}
			}
	}
	if (patternPolicyOwners.size() != patternCount)
	{
		m_strStatus = "Boss pattern policies are incomplete";
		return false;
	}
	for (const auto& [encounterId, timeline] : m_ValtanTimelines)
	{
		const BOSS_RUNTIME_PROFILE* ownerBoss = nullptr;
		std::size_t ownerBossCount = 0u;
		for (const auto& [archetypeId, boss] : m_Bosses)
		{
			if (boss.strEncounterId != encounterId ||
				dependentBossArchetypeIds.contains(archetypeId))
				continue;
			ownerBoss = &boss;
			++ownerBossCount;
		}
		const auto patterns = m_BossPatterns.find(encounterId);
		if (1u != ownerBossCount || nullptr == ownerBoss ||
			m_BossPatterns.end() == patterns ||
			timeline.strEncounterId != encounterId ||
			timeline.strTimelineId.empty() || timeline.Rows.empty())
		{
			m_strStatus = "Valtan timeline owner contract is invalid";
			return false;
		}
		std::unordered_set<std::string> rowIds;
		std::unordered_set<std::uint32_t> commandIds;
		VALTAN_TIMELINE_ARENA_STATE previousArenaState =
			VALTAN_TIMELINE_ARENA_STATE::FRESH;
		std::uint32_t previousSectionHealthBar = ownerBoss->iMaximumHealthBars;
		for (std::size_t index = 0u; index < timeline.Rows.size(); ++index)
		{
			const VALTAN_TIMELINE_ROW& row = timeline.Rows[index];
			const std::uint8_t arenaRank =
				static_cast<std::uint8_t>(row.eArenaState);
			const std::uint8_t previousArenaRank =
				static_cast<std::uint8_t>(previousArenaState);
			if (row.iOrdinal != index + 1u || !IsStableId(row.strRowId) ||
				!rowIds.insert(row.strRowId).second ||
				0u == row.iCommandId ||
				row.iCommandId !=
					Calculate_ValtanTimelineCommandId(row.strRowId) ||
				!commandIds.insert(row.iCommandId).second ||
				0u == row.iSectionHealthBar ||
				row.iSectionHealthBar > ownerBoss->iMaximumHealthBars ||
				row.iSectionHealthBar > previousSectionHealthBar ||
				row.PatternActions.empty() || row.PatternActions.size() > 8u ||
				arenaRank < previousArenaRank ||
				arenaRank > previousArenaRank + 1u ||
				(0u == index &&
				 (row.iSectionHealthBar != ownerBoss->iMaximumHealthBars ||
				  VALTAN_TIMELINE_ARENA_STATE::FRESH != row.eArenaState)))
			{
				m_strStatus = "Valtan timeline occurrence contract is invalid";
				return false;
			}
			previousArenaState = row.eArenaState;
			previousSectionHealthBar = row.iSectionHealthBar;
			for (const VALTAN_TIMELINE_PATTERN_ACTION& action : row.PatternActions)
			{
				if (!IsStableId(action.strPatternId) || 0u == action.iRepeat ||
					action.iRepeat > 4u ||
					patterns->second.end() == std::find_if(
						patterns->second.begin(), patterns->second.end(),
						[&action](const BOSS_PATTERN_DEFINITION& candidate)
						{
							return candidate.strPatternId == action.strPatternId;
						}))
				{
					m_strStatus = "Valtan timeline pattern join is invalid";
					return false;
				}
			}
		}
	}

	/* intro는 rotation 행에 없는 자동 선택 진입점이다. normal만 허용해 AUDITION_ONLY pattern이 전투에 자동 진입하지 않게 한다. */
	for (const auto& [encounterId, introPatternId] :
		m_IntroPatternIdByEncounter)
	{
		const auto patterns = m_BossPatterns.find(encounterId);
		if (m_BossPatterns.end() == patterns ||
			patterns->second.end() == std::find_if(
				patterns->second.begin(), patterns->second.end(),
				[&introPatternId](const BOSS_PATTERN_DEFINITION& pattern)
				{
					return pattern.strPatternId == introPatternId &&
						BOSS_PATTERN_SELECTION::NORMAL == pattern.eSelection;
				}))
		{
			m_strStatus = "Encounter intro names no normal pattern";
			return false;
		}
	}

	for (const auto& [sequenceEncounterId, sequence] :
		m_BossPatternSequences)
	{
		const auto patterns = m_BossPatterns.find(sequenceEncounterId);
		if (m_BossPatterns.end() == patterns ||
			sequence.strEncounterId != sequenceEncounterId ||
			sequence.strSequenceId.empty() ||
			(BOSS_PATTERN_SEQUENCE_MODE::ORDERED_ONCE_THEN_IDLE != sequence.eMode &&
			 BOSS_PATTERN_SEQUENCE_MODE::HEALTH_BAR_ROTATIONS != sequence.eMode) ||
			sequence.PatternIds.size() != sequence.iExpectedStepCount ||
			sequence.TransitionPursuitMs.size() + 1u !=
				sequence.PatternIds.size() ||
			sequence.TransitionPursuitTicks.size() !=
				sequence.TransitionPursuitMs.size())
		{
			m_strStatus = "Boss pattern sequence tagged shape is incomplete";
			return false;
		}
		if (!sequence.strEntranceCinematicPatternId.empty())
		{
			const auto cinematic = std::find_if(patterns->second.begin(), patterns->second.end(),
				[&sequence](const auto& pattern)
				{ return pattern.strPatternId == sequence.strEntranceCinematicPatternId; });
			if (patterns->second.end() == cinematic ||
				sequence.strEntranceCinematicPatternId == Find_IntroPatternId(sequenceEncounterId) ||
				BOSS_PATTERN_SELECTION::NORMAL != cinematic->eSelection ||
				BOSS_PATTERN_TARGET_POLICY::NONE != cinematic->eTargetPolicy ||
				BOSS_PATTERN_AIM_POLICY::NONE != cinematic->eAimPolicy ||
				!cinematic->bInvulnerableWhileRunning)
			{
				m_strStatus = "Boss entrance cinematic must be a separate invulnerable targetless normal pattern";
				return false;
			}
		}
		// 순번이 occurrence를 구분하므로 같은 저작 pattern도 다시 실행할 수 있다.
		for (const std::string& patternId : sequence.PatternIds)
		{
			const auto pattern = std::find_if(
				patterns->second.begin(), patterns->second.end(),
				[&patternId](const BOSS_PATTERN_DEFINITION& candidate)
				{ return candidate.strPatternId == patternId; });
			if (patterns->second.end() == pattern)
			{
				m_strStatus = "Boss pattern sequence contains an unknown step";
				return false;
			}
		}
	}

	for (const auto& [rotationEncounterId, rotations] : m_BossPatternRotations)
	{
		(void)rotationEncounterId;
		for (std::size_t rotationIndex = 0u;
			rotationIndex < rotations.size(); ++rotationIndex)
		{
			const BOSS_PATTERN_ROTATION_DEFINITION& rotation =
				rotations[rotationIndex];
			if (BOSS_PATTERN_ROTATION_SELECTION_MODE::WEIGHTED_POOL ==
				rotation.eSelectionMode)
			{
				if (!rotation.Window.Is_Defined() ||
					!rotation.PatternIds.empty() ||
					rotation.Candidates.size() != rotation.iExpectedStepCount ||
					rotation.Window.iExpectedCandidateCount !=
						rotation.iExpectedStepCount ||
					rotation.Window.iFromHealthBar != rotation.iFromHealthBar ||
					rotation.Window.iToHealthBar != rotation.iToHealthBar)
				{
					m_strStatus =
						"Boss managed weighted rotation tagged shape is incomplete";
					return false;
				}
				std::unordered_set<std::string> uniquePoolIds;
				bool hasEnabledCandidate = false;
				for (const BOSS_PATTERN_ROTATION_CANDIDATE& candidate :
					rotation.Candidates)
				{
					if (!uniquePoolIds.insert(candidate.strPatternId).second)
					{
						m_strStatus =
							"Boss weighted pattern pool contains a duplicate";
						return false;
					}
					hasEnabledCandidate = hasEnabledCandidate || candidate.bEnabled;
				}
				if (!hasEnabledCandidate)
				{
					m_strStatus =
						"Boss managed weighted rotation has no enabled candidate";
					return false;
				}
			}
			else if (BOSS_PATTERN_ROTATION_SELECTION_MODE::ORDERED_LOOP == rotation.eSelectionMode)
			{
				if (!rotation.Window.Is_Defined() || !rotation.Candidates.empty() ||
					rotation.PatternIds.size() != rotation.iExpectedStepCount ||
					rotation.Window.iExpectedCandidateCount != rotation.iExpectedStepCount ||
					rotation.Window.iFromHealthBar != rotation.iFromHealthBar ||
					rotation.Window.iToHealthBar != rotation.iToHealthBar)
				{
					m_strStatus = "Boss ordered loop rotation tagged shape is incomplete";
					return false;
				}
			}
			else if (rotation.Window.Is_Defined() ||
				!rotation.Candidates.empty() ||
				rotation.PatternIds.size() != rotation.iExpectedStepCount)
			{
				/* 이전 ORDERED 형식은 중복 step을 유지하며 tag 형식·정확한 순번 개수만 제한한다. */
				m_strStatus =
					"Boss legacy ordered rotation tagged shape is incomplete";
				return false;
			}

			for (std::size_t priorIndex = 0u;
				priorIndex < rotationIndex; ++priorIndex)
			{
				const BOSS_PATTERN_ROTATION_DEFINITION& prior =
					rotations[priorIndex];
				const bool rangesOverlap =
					rotation.iFromHealthBar > prior.iToHealthBar &&
					prior.iFromHealthBar > rotation.iToHealthBar;
				if (!rangesOverlap) continue;
				const bool bothManaged =
					rotation.Window.Is_Defined() && prior.Window.Is_Defined();
				if (!bothManaged || rotation.Window.iGameplayPhase ==
					prior.Window.iGameplayPhase)
				{
					m_strStatus =
						"Boss pattern rotation spans overlap in one lookup domain";
					return false;
				}
			}
		}
	}

	/* 직접 bootstrap을 승인할 때 Valtan의 phase 경계와 인접 rotation 구간이 함께 일관적인지 확인한다. */
	const auto valtanRotations =
		m_BossPatternRotations.find("ENCOUNTER_VALTAN");
	for (const auto& [bundleId, bundle] : m_BossPatternBundles)
	{
		const auto* patterns = Find_BossPatterns(bundle.strEncounterId);
		if (!patterns || bundle.Members.empty()) { m_strStatus = "KoukuSaydon bundle has no admitted members"; return false; }
		for (const auto& member : bundle.Members)
		{
			const auto pattern = std::find_if(patterns->begin(), patterns->end(), [&](const auto& candidate) { return candidate.strPatternId == member.strPatternId; });
			if (pattern == patterns->end() || pattern->strGateId != bundle.strGateId || pattern->strTargetBossPlacementId != member.strTargetBossPlacementId)
			{ m_strStatus = "KoukuSaydon bundle member Gate/placement differs from its pattern"; return false; }
		}
	}
	const auto valtanSequence =
		m_BossPatternSequences.find("ENCOUNTER_VALTAN");
	const auto valtanPatterns = m_BossPatterns.find("ENCOUNTER_VALTAN");
	const auto valtanBoss = m_Bosses.find("BOSS_VALTAN");
	const bool expectsAuthoredValtanTopology =
		m_Bosses.end() != valtanBoss &&
		"ENCOUNTER_VALTAN" == valtanBoss->second.strEncounterId &&
		BOSS_PHASE_POLICY_KIND::AUTHORED_PATTERN_EVENT ==
			valtanBoss->second.PhasePolicy.eKind;
	if (expectsAuthoredValtanTopology &&
		(m_BossPatternRotations.end() == valtanRotations ||
		 m_BossPatternSequences.end() == valtanSequence ||
		 m_BossPatterns.end() == valtanPatterns))
	{
		/* 전용 sequence가 없으면 거부한다. 이전 자동 selector로 돌아가 비활성 pattern을 다시 실행하지 않는다. */
		m_strStatus =
			"Valtan authored phase topology has no exclusive pattern sequence";
		return false;
	}
	if (m_BossPatternRotations.end() != valtanRotations &&
		m_BossPatterns.end() != valtanPatterns)
	{
		std::uint32_t finalPhaseOneBoundary =
			(std::numeric_limits<std::uint32_t>::max)();
		std::uint32_t firstLegacyFromBar = 0u;
		for (const BOSS_PATTERN_ROTATION_DEFINITION& rotation :
			valtanRotations->second)
		{
			if (rotation.Window.Is_Defined() && 1u == rotation.Window.iGameplayPhase)
			{
				finalPhaseOneBoundary = (std::min)(
					finalPhaseOneBoundary, rotation.Window.iToHealthBar);
			}
			else if (BOSS_PATTERN_ROTATION_SELECTION_MODE::ORDERED_LOOP == rotation.eSelectionMode &&
				2u == rotation.Window.iGameplayPhase)
			{
				firstLegacyFromBar = (std::max)(firstLegacyFromBar, rotation.iFromHealthBar - 1u);
			}
			else if (BOSS_PATTERN_ROTATION_SELECTION_MODE::
				ORDERED_INTRO_THEN_WEIGHTED == rotation.eSelectionMode)
			{
				firstLegacyFromBar = (std::max)(
					firstLegacyFromBar, rotation.iFromHealthBar);
			}
		}
		std::uint32_t phaseTransitionBar = 0u;
		std::uint32_t phaseTransitionCount = 0u;
		for (const BOSS_PATTERN_DEFINITION& pattern : valtanPatterns->second)
		{
			if (BOSS_PATTERN_SELECTION::HEALTH_BAR != pattern.eSelection)
				continue;
			for (const BOSS_PATTERN_STAGE_DEFINITION& stage : pattern.Stages)
			{
				for (const BOSS_PATTERN_STAGE_ACTION& action : stage.Actions)
				{
					if (BOSS_PATTERN_STAGE_ACTION_KIND::SET_GAMEPLAY_PHASE !=
						action.eKind || 2u != action.iValue)
					{
						continue;
					}
					phaseTransitionBar = pattern.iTriggerHealthBar;
					++phaseTransitionCount;
				}
			}
		}
		if (1u != phaseTransitionCount ||
			(std::numeric_limits<std::uint32_t>::max)() ==
				finalPhaseOneBoundary || 0u == firstLegacyFromBar ||
			phaseTransitionBar != finalPhaseOneBoundary ||
			phaseTransitionBar != firstLegacyFromBar + 1u)
		{
			m_strStatus =
				"Valtan phase-transition mechanic and rotation topology diverge";
			return false;
		}
	}

	/* 디스크를 다시 읽지 않고 이미 parse한 byte를 hash한다. target·bundle 행은 검증된 owner를 통해 scope를 구한다. */
	std::string nonKoukuRows = "LOSTARK_NON_KOUKU_GAMEPLAY\t" +
		std::to_string(GAMEPLAY_BOOTSTRAP_VERSION) + "\n";
	std::string koukuBootstrapRows, nonKoukuBootstrapRows;
	std::istringstream domainInput{ bootstrapBytes };
	std::getline(domainInput, line);
	const auto* koukuPatterns = Find_BossPatterns("ENCOUNTER_KOUKUSAYDON_G1");
	while (std::getline(domainInput, line))
	{
		StripCarriageReturn(line);
		const auto fields = SplitTabs(line);
		bool koukuOwned = false;
		if (fields.size() >= 2u)
		{
			const auto kind = fields[0];
			koukuOwned = fields[1] == "ENCOUNTER_KOUKUSAYDON_G1" &&
				(kind.starts_with("PATTERN") || kind == "KOUKUMADNESS" ||
				 kind == "KOUKUSAYDONPRODUCTREVISION" || kind == "ENCOUNTERINTRO");
			if (kind == "RAIDGATE" || kind == "RAIDFLOWSTEP" || kind == "RAIDFLOWGROUP" || kind == "RAIDARRIVAL" || kind == "RAIDBINGOSPECIAL") koukuOwned = true;
			if (kind == "PATTERNTARGET" && nullptr != koukuPatterns)
				koukuOwned = std::any_of(koukuPatterns->begin(), koukuPatterns->end(),
					[&fields](const auto& pattern) { return pattern.strPatternId == fields[1]; });
			if (kind == "PATTERNBUNDLE" || kind == "PATTERNBUNDLEMEMBER")
			{
				const auto* bundle = Find_BossPatternBundle(std::string(fields[1]));
				koukuOwned = nullptr != bundle &&
					bundle->strEncounterId == "ENCOUNTER_KOUKUSAYDON_G1";
			}
		}
		auto& domainRows = koukuOwned ? koukuBootstrapRows : nonKoukuBootstrapRows;
		domainRows.append(line); domainRows.push_back('\n');
		if (!koukuOwned) { nonKoukuRows.append(line); nonKoukuRows.push_back('\n'); }
	}
	LostArk::Shared::GameplayDataRevision nonKoukuRevision{};
	if (!Calculate_GameplayDataRevision(nonKoukuRows, nonKoukuRevision))
	{
		m_strStatus = "Gameplay non-Kouku domain revision could not be calculated";
		return false;
	}
	for (const auto& [gateId, gate] : m_KoukuRaidGates)
	{
		if (gate.Entries.size() != gate.iExpectedEntryCount || gate.strEncounterId != "ENCOUNTER_KOUKUSAYDON_G1")
		{ m_strStatus = "Kouku raid flow is incomplete or outside its encounter"; return false; }
		if (gateId == "BINGO")
		{
			const auto* patterns = Find_BossPatterns(gate.strEncounterId);
			const auto special = patterns ? std::find_if(patterns->begin(), patterns->end(), [&](const auto& pattern) {
				return pattern.strPatternId == gate.strBingoSpecialPatternId;
			}) : std::vector<BOSS_PATTERN_DEFINITION>::const_iterator{};
			if (!patterns || special == patterns->end() || special->strGateId != gateId ||
				special->strTargetBossPlacementId != gate.strPrimaryBossPlacementId || !special->bFixedTimelineClock ||
				special->Stages.empty() || !special->strParentLoopStartOccurrenceId.empty())
			{ m_strStatus = "Bingo flow needs its exact finite special Parent in the same boss scope"; return false; }
			std::uint64_t specialDurationMs = 0u;
			for (const auto& stage : special->Stages) specialDurationMs += stage.iDurationMs;
			specialDurationMs = (std::max)(specialDurationMs, std::uint64_t(special->iTimelineDurationMs));
			if (!specialDurationMs || specialDurationMs > 600000u)
			{ m_strStatus = "Bingo special Parent duration must be finite and bounded"; return false; }
			unsigned detonations = static_cast<unsigned>(std::count_if(special->MechanicTriggers.begin(), special->MechanicTriggers.end(),
				[](const auto& trigger) { return trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BINGO_DETONATION; }));
			for (const auto& child : special->ParentChildren)
			{
				const auto found = std::find_if(patterns->begin(), patterns->end(), [&](const auto& pattern) { return pattern.strPatternId == child.strPatternId; });
				if (found == patterns->end() || found->strGateId != gateId || found->strTargetBossPlacementId != gate.strPrimaryBossPlacementId)
				{ m_strStatus = "Bingo special Parent child scope is invalid"; return false; }
				detonations += static_cast<unsigned>(std::count_if(found->MechanicTriggers.begin(), found->MechanicTriggers.end(),
					[](const auto& trigger) { return trigger.eKind == BOSS_PATTERN_MECHANIC_TRIGGER_KIND::BINGO_DETONATION; }));
			}
			if (detonations != 1u)
			{ m_strStatus = "Bingo special Parent must own exactly one detonation"; return false; }
		}
		if (!gate.strLoopStartEntryId.empty() && std::none_of(gate.Entries.begin(), gate.Entries.end(),
			[&](const auto& entry) { return entry.strEntryId == gate.strLoopStartEntryId; }))
		{ m_strStatus = "Kouku raid loop start is not an entry of its flow"; return false; }
		for (const auto& entry : gate.Entries)
		{
			if (gateId == "BINGO" && !entry.bBundle && entry.strTargetId == gate.strBingoSpecialPatternId)
			{ m_strStatus = "Bingo special Parent cannot also be a normal flow entry"; return false; }
			if (entry.bBundle)
			{
				const auto* bundle = Find_BossPatternBundle(entry.strTargetId);
				if (!bundle || bundle->strEncounterId != gate.strEncounterId || bundle->strGateId != gateId)
				{ m_strStatus = "Kouku raid flow bundle scope mismatch"; return false; }
				if (gateId == "BINGO" && std::any_of(bundle->Members.begin(), bundle->Members.end(),
					[&](const auto& member) { return member.strPatternId == gate.strBingoSpecialPatternId; }))
				{ m_strStatus = "Bingo special Parent cannot also be a normal flow bundle member"; return false; }
			}
			else
			{
				const auto* patterns = Find_BossPatterns(gate.strEncounterId);
				if (!patterns || std::none_of(patterns->begin(), patterns->end(), [&](const auto& p) { return p.strPatternId == entry.strTargetId && p.strGateId == gateId; }))
				{ m_strStatus = "Kouku raid flow pattern scope mismatch"; return false; }
			}
		}
	}
	rollback.committed = true;
	m_KoukuBootstrapRows = std::move(koukuBootstrapRows);
	m_NonKoukuBootstrapRows = std::move(nonKoukuBootstrapRows);
	m_NonKoukuGameplayRevision = nonKoukuRevision;
	m_ActiveRevision = nullptr == parentRevision ?
		admittedRevision : *parentRevision;
	m_strStatus = nullptr == parentRevision ?
		"Loaded gameplay bootstrap" :
		"Loaded staged gameplay parent revision";
	return true;
}

bool LostArk::Server::CGameplayCatalog::Has_SameNonKoukuGameplay(
	const CGameplayCatalog& other) const noexcept
{
	return m_NonKoukuGameplayRevision.Is_Valid() &&
		m_NonKoukuGameplayRevision == other.m_NonKoukuGameplayRevision;
}

const LostArk::Server::PLAYER_RUNTIME_PROFILE*
LostArk::Server::CGameplayCatalog::Find_Player(
	const LostArk::Shared::CHARACTER_CLASS_ID characterClass) const
{
	const auto iter = m_Players.find(characterClass);
	return m_Players.end() == iter ? nullptr : &iter->second;
}

const LostArk::Server::GUARDIAN_EMBER_PROFILE*
LostArk::Server::CGameplayCatalog::Find_EmberProfile(
	const LostArk::Shared::CHARACTER_CLASS_ID characterClass) const
{
	const auto iter = m_EmberProfiles.find(characterClass);
	return m_EmberProfiles.end() == iter ? nullptr : &iter->second;
}

const LostArk::Server::PLAYER_SKILL_DEFINITION*
LostArk::Server::CGameplayCatalog::Find_Skill(
	const LostArk::Shared::SKILL_ID skillId) const
{
	const auto iter = m_Skills.find(skillId);
	return m_Skills.end() == iter ? nullptr : &iter->second;
}

const LostArk::Server::BOSS_RUNTIME_PROFILE*
LostArk::Server::CGameplayCatalog::Find_Boss(
	const std::string& archetypeId) const
{
	const auto iter = m_Bosses.find(archetypeId);
	return m_Bosses.end() == iter ? nullptr : &iter->second;
}

const std::vector<LostArk::Server::BOSS_PART_DEFINITION>*
LostArk::Server::CGameplayCatalog::Find_BossParts(
	const std::string& archetypeId) const
{
	const auto iter = m_BossParts.find(archetypeId);
	return m_BossParts.end() == iter ? nullptr : &iter->second;
}

const std::vector<LostArk::Server::BOSS_PATTERN_DEFINITION>*
LostArk::Server::CGameplayCatalog::Find_BossPatterns(
	const std::string& encounterId) const
{
	const auto iter = m_BossPatterns.find(encounterId);
	return m_BossPatterns.end() == iter ? nullptr : &iter->second;
}

const LostArk::Server::BOSS_COMBAT_OBJECT_DEFINITION*
LostArk::Server::CGameplayCatalog::Find_BossCombatObject(
	const std::string& combatObjectArchetypeId) const
{
	const auto iter = m_BossCombatObjects.find(combatObjectArchetypeId);
	return m_BossCombatObjects.end() == iter ? nullptr : &iter->second;
}

const LostArk::Server::VALTAN_TIMELINE_DEFINITION*
LostArk::Server::CGameplayCatalog::Find_ValtanTimeline(
	const std::string& encounterId) const
{
	const auto iter = m_ValtanTimelines.find(encounterId);
	return m_ValtanTimelines.end() == iter ? nullptr : &iter->second;
}

const LostArk::Server::VALTAN_TIMELINE_ROW*
LostArk::Server::CGameplayCatalog::Find_ValtanTimelineRow(
	const std::string& encounterId,
	const std::uint32_t commandId) const
{
	if (0u == commandId)
		return nullptr;
	const VALTAN_TIMELINE_DEFINITION* timeline =
		Find_ValtanTimeline(encounterId);
	if (nullptr == timeline)
		return nullptr;
	const auto row = std::find_if(
		timeline->Rows.begin(), timeline->Rows.end(),
		[commandId](const VALTAN_TIMELINE_ROW& candidate)
		{
			return candidate.iCommandId == commandId;
		});
	return timeline->Rows.end() == row ? nullptr : &*row;
}

const LostArk::Server::BOSS_PATTERN_ROTATION_DEFINITION*
LostArk::Server::CGameplayCatalog::Find_BossPatternRotation(
	const std::string& encounterId,
	const std::uint32_t gameplayPhase,
	const std::uint32_t healthBar) const
{
	const auto iter = m_BossPatternRotations.find(encounterId);
	if (m_BossPatternRotations.end() == iter)
		return nullptr;
	for (const BOSS_PATTERN_ROTATION_DEFINITION& rotation : iter->second)
	{
		/* 정의된 managed window는 phase를 맞춘다. window가 없는 이전 ORDERED 행에는 이 phase 필터를 적용하지 않는다. */
		if (rotation.Window.Is_Defined() && rotation.Window.iGameplayPhase != gameplayPhase)
		{
			continue;
		}
		/* 감소하는 체력 bar의 구간은 to < healthBar <= from이다. 다음 기믹의 bar에서는 현재 구간이 끝난다. */
		if (healthBar <= rotation.iFromHealthBar &&
			healthBar > rotation.iToHealthBar)
		{
			return &rotation;
		}
	}
	return nullptr;
}

const LostArk::Server::BOSS_PATTERN_SEQUENCE_DEFINITION*
LostArk::Server::CGameplayCatalog::Find_BossPatternSequence(
	const std::string& encounterId) const
{
	const auto iter = m_BossPatternSequences.find(encounterId);
	return m_BossPatternSequences.end() == iter ? nullptr : &iter->second;
}

const LostArk::Server::BOSS_ENCOUNTER_MADNESS_POLICY*
LostArk::Server::CGameplayCatalog::Find_KoukuMadnessPolicy(
	const std::string& encounterId) const noexcept
{
	const auto found = m_KoukuMadnessPolicies.find(encounterId);
	return m_KoukuMadnessPolicies.end() == found ? nullptr : &found->second;
}

std::uint32_t
LostArk::Server::CGameplayCatalog::Find_KoukuSaydonProductSourceRevision(
	const std::string& encounterId) const noexcept
{
	return "ENCOUNTER_KOUKUSAYDON_G1" == encounterId ?
		m_iKoukuSaydonProductSourceRevision : 0u;
}

const std::string& LostArk::Server::CGameplayCatalog::Find_IntroPatternId(
	const std::string& encounterId) const
{
	static const std::string EMPTY;
	const auto iter = m_IntroPatternIdByEncounter.find(encounterId);
	return m_IntroPatternIdByEncounter.end() == iter ? EMPTY : iter->second;
}

std::uint32_t LostArk::Server::CGameplayCatalog::Find_DamageRatePercent(
	const std::string& damageProfileId) const
{
	const auto iter = m_DamageRatePercentByProfileId.find(damageProfileId);
	return m_DamageRatePercentByProfileId.end() == iter ? 0u : iter->second;
}

const LostArk::Server::CGameplayCatalog::SKILL_BUFF_DEFINITION*
LostArk::Server::CGameplayCatalog::Find_SkillBuff(
	const std::uint32_t buffId) const
{
	const auto iter = m_SkillBuffById.find(buffId);
	return m_SkillBuffById.end() == iter ? nullptr : &iter->second;
}

const std::vector<LostArk::Server::CGameplayCatalog::SKILL_BUFF_DEFINITION>*
LostArk::Server::CGameplayCatalog::Find_SkillBuffs(
	const std::uint32_t skillId) const
{
	const auto iter = m_SkillBuffsBySkillId.find(skillId);
	return m_SkillBuffsBySkillId.end() == iter ? nullptr : &iter->second;
}

const LostArk::Server::CGameplayCatalog::DAMAGE_PROFILE*
LostArk::Server::CGameplayCatalog::Find_DamageProfile(
	const std::string& damageProfileId) const
{
	const auto iter = m_DamageProfileById.find(damageProfileId);
	return m_DamageProfileById.end() == iter ? nullptr : &iter->second;
}

std::uint32_t LostArk::Server::CGameplayCatalog::Resolve_Damage(
	const std::uint32_t attackPower,
	const DAMAGE_PROFILE& profile)
{
	if (0u == profile.iAttackCoefficientBp && 0u == profile.iDamageAddend)
		return Resolve_Damage(attackPower, profile.iRatePercent);
	const std::uint64_t scaled =
		static_cast<std::uint64_t>(attackPower) *
		static_cast<std::uint64_t>(profile.iAttackCoefficientBp) / 10000ull +
		static_cast<std::uint64_t>(profile.iDamageAddend);
	return scaled < 1ull ? 1u :
		static_cast<std::uint32_t>((std::min<std::uint64_t>)(
			scaled, (std::numeric_limits<std::uint32_t>::max)()));
}

std::uint32_t LostArk::Server::CGameplayCatalog::Resolve_Damage(
	const std::uint32_t attackPower,
	const std::uint32_t damageRatePercent)
{
	if (0u == attackPower || 0u == damageRatePercent)
		return 0u;
	const std::uint64_t scaled =
		(static_cast<std::uint64_t>(attackPower) *
			static_cast<std::uint64_t>(damageRatePercent)) / 100ull;
	return scaled < 1ull ? 1u :
		static_cast<std::uint32_t>((std::min<std::uint64_t>)(
			scaled, (std::numeric_limits<std::uint32_t>::max)()));
}

std::uint32_t LostArk::Server::CGameplayCatalog::Apply_Defense(
	const std::uint32_t rawDamage,
	const std::uint32_t defense)
{
	if (0u == rawDamage)
		return 0u;
	const std::uint64_t mitigated =
		(static_cast<std::uint64_t>(rawDamage) * 100ull) /
		(100ull + static_cast<std::uint64_t>(defense));
	return mitigated < 1ull ? 1u :
		static_cast<std::uint32_t>((std::min<std::uint64_t>)(
			mitigated, (std::numeric_limits<std::uint32_t>::max)()));
}

const LostArk::Server::BOSS_PATTERN_BUNDLE_DEFINITION*
LostArk::Server::CGameplayCatalog::Find_BossPatternBundle(const std::string& bundleId) const
{
	const auto found = m_BossPatternBundles.find(bundleId);
	return found == m_BossPatternBundles.end() ? nullptr : &found->second;
}

const LostArk::Server::KOUKU_RAID_GATE_DEFINITION* LostArk::Server::CGameplayCatalog::Find_KoukuRaidGate(const std::string& gateId) const
{
	const auto found = m_KoukuRaidGates.find(gateId);
	return found == m_KoukuRaidGates.end() ? nullptr : &found->second;
}


namespace
{
    using NUMERIC_DOMAIN = LostArk::Shared::BALANCE_DOMAIN;
    struct NUMERIC_COLUMN final
    {
        NUMERIC_DOMAIN domain;
        std::string id;
        const char* field;
        std::size_t column;
        bool integral;
    };
    std::vector<NUMERIC_COLUMN> Numeric_Columns(const std::vector<std::string_view>& fields)
    {
        std::vector<NUMERIC_COLUMN> columns;
        if (fields.size() < 2u) return columns;
        auto add = [&](NUMERIC_DOMAIN domain, const char* field, std::size_t column, bool integral = true)
        {
            if (column < fields.size()) columns.push_back({domain, std::string(fields[1]), field, column, integral});
        };
        if (fields[0] == "PLAYER")
        {
            add(NUMERIC_DOMAIN::PLAYER, "maximumHp", 2); add(NUMERIC_DOMAIN::PLAYER, "maximumResource", 3);
            add(NUMERIC_DOMAIN::PLAYER, "resourceRegenPerSecond", 4); add(NUMERIC_DOMAIN::PLAYER, "attackPower", 5);
            add(NUMERIC_DOMAIN::PLAYER, "defense", 6); add(NUMERIC_DOMAIN::PLAYER, "moveSpeed", 7, false);
            add(NUMERIC_DOMAIN::PLAYER, "defenseStanceMoveSpeedScale", 8, false); add(NUMERIC_DOMAIN::PLAYER, "maximumIdentity", 9);
            add(NUMERIC_DOMAIN::PLAYER, "identityRegenPerSecond", 10); add(NUMERIC_DOMAIN::PLAYER, "identityDrainPerSecond", 11);
            add(NUMERIC_DOMAIN::PLAYER, "identityStanceSwitchCost", 12); add(NUMERIC_DOMAIN::PLAYER, "criticalChancePercent", 15);
            add(NUMERIC_DOMAIN::PLAYER, "criticalDamagePercent", 16);
        }
        else if (fields[0] == "SKILL")
        {
            add(NUMERIC_DOMAIN::SKILL, "cooldownMs", 5); add(NUMERIC_DOMAIN::SKILL, "actionDurationMs", 6);
            add(NUMERIC_DOMAIN::SKILL, "hitTimeMs", 7); add(NUMERIC_DOMAIN::SKILL, "resourceCost", 8);
            add(NUMERIC_DOMAIN::SKILL, "identityCost", 9); add(NUMERIC_DOMAIN::SKILL, "movementDistance", 10, false);
            add(NUMERIC_DOMAIN::SKILL, "maximumRange", 11, false);
        }
        else if (fields[0] == "SKILLCOMBATTRAITS")
        {
            add(NUMERIC_DOMAIN::SKILL, "staggerDamage", 2); add(NUMERIC_DOMAIN::SKILL, "partDamage", 3);
        }
        else if (fields[0] == "DAMAGE")
        {
            double coefficient = 0., addend = 0.;
            if (fields.size() < 5u || (ParseNumber(fields[3], coefficient) && ParseNumber(fields[4], addend) && coefficient == 0. && addend == 0.))
                add(NUMERIC_DOMAIN::DAMAGE, "damageRatePercent", 2);
            add(NUMERIC_DOMAIN::DAMAGE, "attackCoefficientBp", 3); add(NUMERIC_DOMAIN::DAMAGE, "damageAddend", 4);
            add(NUMERIC_DOMAIN::DAMAGE, "damageSpreadPercent", 5); add(NUMERIC_DOMAIN::DAMAGE, "bossHealthBarDamage", 6);
        }
        else if (fields[0] == "BOSS")
        {
            add(NUMERIC_DOMAIN::BOSS, "maximumHp", 3); add(NUMERIC_DOMAIN::BOSS, "maximumHealthBars", 4);
            add(NUMERIC_DOMAIN::BOSS, "attackPower", 5); add(NUMERIC_DOMAIN::BOSS, "collisionRadius", 6, false);
            add(NUMERIC_DOMAIN::BOSS, "engageDistance", 7, false); add(NUMERIC_DOMAIN::BOSS, "moveSpeed", 8, false);
            add(NUMERIC_DOMAIN::BOSS, "damageReferenceHp", 11);
        }
        else if (fields[0] == "KOUKUMADNESS")
        {
            add(NUMERIC_DOMAIN::MADNESS, "damageGainPercent", 4); add(NUMERIC_DOMAIN::MADNESS, "ballGainPercent", 5);
            add(NUMERIC_DOMAIN::MADNESS, "ballMultiplierPercent", 6); add(NUMERIC_DOMAIN::MADNESS, "ballRadiusM", 7, false);
            add(NUMERIC_DOMAIN::MADNESS, "dollGainPercent", 8); add(NUMERIC_DOMAIN::MADNESS, "dollMultiplierPercent", 9);
            add(NUMERIC_DOMAIN::MADNESS, "dollRadiusM", 10, false); add(NUMERIC_DOMAIN::MADNESS, "specialIntervalMs", 11);
            for (auto& column : columns) column.id = "KOUKUSAYDON";
        }
        else if (fields[0] == "PATTERNLOGICOUTCOME" && fields.size() >= 10u &&
            (fields[6] == "MAX_HP_PERCENT_DAMAGE" || fields[6] == "FIXED_DAMAGE"))
        {
            add(NUMERIC_DOMAIN::PATTERN_DAMAGE, fields[6] == "FIXED_DAMAGE" ? "fixedDamage" : "maxHpDamagePercent",
                fields[6] == "FIXED_DAMAGE" ? 10u : 7u);
            if (!columns.empty()) columns.back().id = "O|" + std::string(fields[1]) + "|" + std::string(fields[2]) + "|" +
                std::string(fields[3]) + "|" + std::string(fields[4]) + "|" + std::string(fields[5]);
        }
        else if (fields[0] == "PATTERNATTACKHIT" && fields.size() >= 25u && fields[22] == "MAX_HP_PERCENT")
        {
            add(NUMERIC_DOMAIN::PATTERN_DAMAGE, "maxHpDamagePercent", 23u);
            columns.back().id = "H|" + std::string(fields[1]) + "|" + std::string(fields[2]) + "|" +
                std::string(fields[3]) + "|" + std::string(fields[4]) + "|" + std::string(fields[5]) + "|" + std::string(fields[6]);
        }
        else if (fields[0] == "RAIDSTAGGER" && fields.size() == 2u)
        {
            add(NUMERIC_DOMAIN::STAGGER, "staggerGaugeMaximum", 1);
            columns.back().id = "RAID_COMMON";
        }
        return columns;
    }
    auto Numeric_Key(const NUMERIC_DOMAIN domain, const std::string& id, const std::string& field)
    { return std::make_tuple(domain, id, field); }
    std::string Numeric_Text(const double value)
    {
        std::ostringstream text; text.imbue(std::locale::classic());
        text << std::setprecision(std::numeric_limits<double>::max_digits10) << value;
        return text.str();
    }
}

std::string LostArk::Server::CGameplayCatalog::Export_BootstrapBytes() const
{
    const std::string rows = m_NonKoukuBootstrapRows + m_KoukuBootstrapRows;
    return "LOSTARK_GAMEPLAY_BOOTSTRAP\t" + std::to_string(GAMEPLAY_BOOTSTRAP_VERSION) + "\t" +
        std::to_string(std::count(rows.begin(), rows.end(), '\n')) + "\n" + rows;
}

bool LostArk::Server::CGameplayCatalog::Build_NumericBalanceSnapshot(
    std::vector<LostArk::Shared::BALANCE_NUMERIC_ENTRY>& entries,
    LostArk::Shared::GameplayDataRevision& numericRevision, std::string& status) const
{
    std::vector<LostArk::Shared::BALANCE_NUMERIC_ENTRY> staged;
    std::istringstream input(m_NonKoukuBootstrapRows + m_KoukuBootstrapRows);
    std::string line;
    while (std::getline(input, line))
    {
        StripCarriageReturn(line);
        const auto fields = SplitTabs(line);
        for (const auto& column : Numeric_Columns(fields))
        {
            double value = 0.;
            if (!ParseNumber(fields[column.column], value) || !std::isfinite(value) || column.id.size() > 128u)
            { status = "Published numeric field cannot be represented"; return false; }
            staged.push_back({column.domain, column.id, column.field, value, column.integral});
        }
    }
    std::sort(staged.begin(), staged.end(), [](const auto& a, const auto& b)
    { return Numeric_Key(a.eDomain, a.strId, a.strField) < Numeric_Key(b.eDomain, b.strId, b.strField); });
    std::string canonical;
    for (std::size_t i = 0; i < staged.size(); ++i)
    {
        const auto& entry = staged[i];
        if (i && Numeric_Key(entry.eDomain, entry.strId, entry.strField) ==
            Numeric_Key(staged[i - 1].eDomain, staged[i - 1].strId, staged[i - 1].strField))
        { status = "Published numeric field identity is duplicated"; return false; }
        canonical += std::to_string(static_cast<unsigned>(entry.eDomain)) + "\t" + entry.strId + "\t" + entry.strField +
            "\t" + Numeric_Text(entry.fValue) + "\t" + (entry.isIntegral ? "1\n" : "0\n");
    }
    if (staged.empty() || !Calculate_GameplayDataRevision(canonical, numericRevision))
    { status = "Numeric revision could not be calculated"; return false; }
    entries = std::move(staged); status.clear(); return true;
}

bool LostArk::Server::CGameplayCatalog::Load_NumericBalancePatch(
    const CGameplayCatalog& active,
    const std::vector<LostArk::Shared::BALANCE_NUMERIC_CHANGE>& changes,
    std::string& bootstrapBytes)
{
    using KEY = std::tuple<NUMERIC_DOMAIN, std::string, std::string>;
    std::map<KEY, const LostArk::Shared::BALANCE_NUMERIC_CHANGE*> pending;
    if (changes.empty() || changes.size() > 4096u)
    { m_strStatus = "Numeric patch is empty or exceeds the bounded catalogue field set"; return false; }
    for (const auto& change : changes)
    {
        if (!std::isfinite(change.fBefore) || !std::isfinite(change.fValue) || change.fValue < 0. ||
            !pending.emplace(Numeric_Key(change.eDomain, change.strId, change.strField), &change).second)
        { m_strStatus = "Numeric patch contains a duplicate or invalid value"; return false; }
    }
    std::istringstream input(active.m_NonKoukuBootstrapRows + active.m_KoukuBootstrapRows);
    std::string rows, line;
    while (std::getline(input, line))
    {
        StripCarriageReturn(line);
        const auto fields = SplitTabs(line);
        std::map<std::size_t, std::string> replacements;
        for (const auto& column : Numeric_Columns(fields))
        {
            const auto found = pending.find(Numeric_Key(column.domain, column.id, column.field));
            if (found == pending.end()) continue;
            const auto& change = *found->second;
            double before = 0.;
            if (!ParseNumber(fields[column.column], before) || before != change.fBefore ||
                (column.domain == NUMERIC_DOMAIN::STAGGER && change.fValue <= 0.) ||
                (column.integral && (std::floor(change.fValue) != change.fValue || change.fValue > (std::numeric_limits<std::uint32_t>::max)())))
            { m_strStatus = "Numeric field changed since it was loaded, or requires a bounded whole number: " + change.strId + "." + change.strField; return false; }
            replacements.emplace(column.column, Numeric_Text(change.fValue));
            pending.erase(found);
        }
        if (replacements.empty()) rows += line;
        else for (std::size_t column = 0; column < fields.size(); ++column)
        {
            if (column) rows += '\t';
            const auto replacement = replacements.find(column);
            rows += replacement == replacements.end() ? std::string(fields[column]) : replacement->second;
        }
        rows += '\n';
    }
    if (!pending.empty()) { m_strStatus = "Numeric patch names an unknown or noneditable field"; return false; }
    const std::string stagedBytes = "LOSTARK_GAMEPLAY_BOOTSTRAP\t" + std::to_string(GAMEPLAY_BOOTSTRAP_VERSION) + "\t" +
        std::to_string(std::count(rows.begin(), rows.end(), '\n')) + "\n" + rows;
    CGameplayCatalog staged;
    if (!staged.Load_BootstrapBytes(stagedBytes, nullptr, &active.m_ActiveRevision))
    { m_strStatus = staged.Get_Status(); return false; }
    if (staged.m_ValtanPresentationGenerationId != active.m_ValtanPresentationGenerationId)
    { m_strStatus = "Numeric patch changed presentation identity"; return false; }
    *this = std::move(staged); bootstrapBytes = stagedBytes;
    m_strStatus = "Validated numeric balance against the existing gameplay catalogue";
    return true;
}

bool LostArk::Server::CGameplayCatalog::Load_NumericBalanceValues(
    const CGameplayCatalog& choreography, const CGameplayCatalog& numeric)
{
    std::vector<LostArk::Shared::BALANCE_NUMERIC_ENTRY> oldEntries, newEntries;
    LostArk::Shared::GameplayDataRevision ignored;
    if (!choreography.Build_NumericBalanceSnapshot(oldEntries, ignored, m_strStatus) ||
        !numeric.Build_NumericBalanceSnapshot(newEntries, ignored, m_strStatus)) return false;
    std::map<std::tuple<NUMERIC_DOMAIN, std::string, std::string>, double> values;
    for (const auto& entry : newEntries) values.emplace(Numeric_Key(entry.eDomain, entry.strId, entry.strField), entry.fValue);
    std::vector<LostArk::Shared::BALANCE_NUMERIC_CHANGE> changes;
    for (const auto& entry : oldEntries)
    {
        const auto value = values.find(Numeric_Key(entry.eDomain, entry.strId, entry.strField));
        if (value != values.end() && value->second != entry.fValue)
            changes.push_back({entry.eDomain, entry.strId, entry.strField, entry.fValue, value->second});
    }
    if (changes.empty()) { *this = choreography; return true; }
    std::string ignoredBytes;
    return Load_NumericBalancePatch(choreography, changes, ignoredBytes);
}


bool LostArk::Server::CGameplayCatalog::Build_NumericBalanceReceiptHashes(
    LostArk::Shared::GameplayDataRevision& bootstrapHash,
    LostArk::Shared::GameplayDataRevision& nonNumericRowsHash, std::string& status) const
{
    const auto bootstrap = Export_BootstrapBytes();
    std::istringstream input(bootstrap);
    std::string line, masked;
    while (std::getline(input, line))
    {
        StripCarriageReturn(line);
        const auto fields = SplitTabs(line);
        std::unordered_set<std::size_t> numeric;
        for (const auto& column : Numeric_Columns(fields)) numeric.insert(column.column);
        // coefficient·addend 수식 때문에 UI에서 숨겨져도 rate는 수치로 유지한다.
        if (!fields.empty() && fields[0] == "DAMAGE") numeric.insert(2u);
        for (std::size_t i = 0; i < fields.size(); ++i)
        {
            if (i) masked += '\t';
            masked += numeric.contains(i) ? "#numeric" : std::string(fields[i]);
        }
        masked += '\n';
    }
    if (!Calculate_GameplayDataRevision(bootstrap, bootstrapHash) || !Calculate_GameplayDataRevision(masked, nonNumericRowsHash))
    { status = "Numeric receipt hashes could not be calculated"; return false; }
    status.clear(); return true;
}
```


## p17_capture_hand_keys.candidate.json

```json
{
  "patternId": "KOUKUSAYDON_G1_PATTERN_17",
  "windowId": "KOUKUSAYDON_G1_PATTERN_17.logic.1",
  "captureHandKeys": [
    {
      "timeMs": 2029,
      "positionOffset": [
        4.424593773,
        0.455920281,
        2.519139769
      ]
    },
    {
      "timeMs": 2033,
      "positionOffset": [
        4.476010065,
        0.443536489,
        2.385082978
      ]
    },
    {
      "timeMs": 2034,
      "positionOffset": [
        4.476010065,
        0.443536489,
        2.385082978
      ]
    },
    {
      "timeMs": 2066,
      "positionOffset": [
        5.138144203,
        0.435482451,
        1.12584845
      ]
    },
    {
      "timeMs": 2067,
      "positionOffset": [
        5.138144203,
        0.435482451,
        1.12584845
      ]
    },
    {
      "timeMs": 2100,
      "positionOffset": [
        5.428145153,
        0.404348392,
        -0.189962485
      ]
    },
    {
      "timeMs": 2133,
      "positionOffset": [
        5.145589261,
        0.350987419,
        -1.658785802
      ]
    },
    {
      "timeMs": 2134,
      "positionOffset": [
        5.145589261,
        0.350987419,
        -1.658785802
      ]
    },
    {
      "timeMs": 2166,
      "positionOffset": [
        4.634127162,
        0.330060217,
        -3.201656698
      ]
    },
    {
      "timeMs": 2167,
      "positionOffset": [
        4.634127162,
        0.330060217,
        -3.201656698
      ]
    },
    {
      "timeMs": 2200,
      "positionOffset": [
        4.140481604,
        0.362695774,
        -4.377895179
      ]
    },
    {
      "timeMs": 2233,
      "positionOffset": [
        3.424615213,
        0.386309496,
        -4.526708393
      ]
    },
    {
      "timeMs": 2234,
      "positionOffset": [
        3.424615213,
        0.386309496,
        -4.526708393
      ]
    },
    {
      "timeMs": 2266,
      "positionOffset": [
        2.864183965,
        0.361580841,
        -4.33087509
      ]
    },
    {
      "timeMs": 2267,
      "positionOffset": [
        2.864183965,
        0.361580841,
        -4.33087509
      ]
    },
    {
      "timeMs": 2300,
      "positionOffset": [
        2.788269177,
        0.357471391,
        -4.322851828
      ]
    },
    {
      "timeMs": 2333,
      "positionOffset": [
        2.733252666,
        0.354362771,
        -4.318873414
      ]
    },
    {
      "timeMs": 2334,
      "positionOffset": [
        2.733252666,
        0.354362771,
        -4.318873414
      ]
    },
    {
      "timeMs": 2366,
      "positionOffset": [
        2.697794213,
        0.352325489,
        -4.31643759
      ]
    },
    {
      "timeMs": 2367,
      "positionOffset": [
        2.697794213,
        0.352325489,
        -4.31643759
      ]
    },
    {
      "timeMs": 2400,
      "positionOffset": [
        2.680119843,
        0.351303151,
        -4.313412381
      ]
    },
    {
      "timeMs": 2433,
      "positionOffset": [
        2.678295096,
        0.35118624,
        -4.307977856
      ]
    },
    {
      "timeMs": 2434,
      "positionOffset": [
        2.678295096,
        0.35118624,
        -4.307977856
      ]
    },
    {
      "timeMs": 2466,
      "positionOffset": [
        2.690280391,
        0.351788443,
        -4.298465206
      ]
    },
    {
      "timeMs": 2467,
      "positionOffset": [
        2.690280391,
        0.351788443,
        -4.298465206
      ]
    },
    {
      "timeMs": 2500,
      "positionOffset": [
        2.713950186,
        0.352950731,
        -4.283285116
      ]
    },
    {
      "timeMs": 2533,
      "positionOffset": [
        2.747234836,
        0.354379416,
        -4.26088774
      ]
    },
    {
      "timeMs": 2534,
      "positionOffset": [
        2.747234836,
        0.354379416,
        -4.26088774
      ]
    },
    {
      "timeMs": 2566,
      "positionOffset": [
        2.788082971,
        0.355824499,
        -4.229672163
      ]
    },
    {
      "timeMs": 2567,
      "positionOffset": [
        2.788082971,
        0.355824499,
        -4.229672163
      ]
    },
    {
      "timeMs": 2600,
      "positionOffset": [
        2.834496432,
        0.357027243,
        -4.187953909
      ]
    },
    {
      "timeMs": 2633,
      "positionOffset": [
        2.884604153,
        0.357699468,
        -4.133898123
      ]
    },
    {
      "timeMs": 2634,
      "positionOffset": [
        2.884604153,
        0.357699468,
        -4.133898123
      ]
    },
    {
      "timeMs": 2666,
      "positionOffset": [
        2.936629192,
        0.357562537,
        -4.06550057
      ]
    },
    {
      "timeMs": 2667,
      "positionOffset": [
        2.936629192,
        0.357562537,
        -4.06550057
      ]
    },
    {
      "timeMs": 2700,
      "positionOffset": [
        2.988523023,
        0.356407796,
        -3.980939562
      ]
    },
    {
      "timeMs": 2733,
      "positionOffset": [
        3.061456692,
        0.353812745,
        -3.877590664
      ]
    },
    {
      "timeMs": 2734,
      "positionOffset": [
        3.061456692,
        0.353812745,
        -3.877590664
      ]
    },
    {
      "timeMs": 2766,
      "positionOffset": [
        3.171428479,
        0.346389982,
        -3.698587901
      ]
    },
    {
      "timeMs": 2767,
      "positionOffset": [
        3.171428479,
        0.346389982,
        -3.698587901
      ]
    },
    {
      "timeMs": 2800,
      "positionOffset": [
        3.309737222,
        0.333098081,
        -3.40959791
      ]
    },
    {
      "timeMs": 2833,
      "positionOffset": [
        3.463355628,
        0.317412006,
        -3.042070881
      ]
    },
    {
      "timeMs": 2834,
      "positionOffset": [
        3.463355628,
        0.317412006,
        -3.042070881
      ]
    },
    {
      "timeMs": 2866,
      "positionOffset": [
        3.61988886,
        0.302538016,
        -2.625808259
      ]
    },
    {
      "timeMs": 2867,
      "positionOffset": [
        3.61988886,
        0.302538016,
        -2.625808259
      ]
    },
    {
      "timeMs": 2900,
      "positionOffset": [
        3.767405202,
        0.2913601,
        -2.188699009
      ]
    },
    {
      "timeMs": 2933,
      "positionOffset": [
        3.895115517,
        0.284949227,
        -1.67891897
      ]
    },
    {
      "timeMs": 2934,
      "positionOffset": [
        3.895115517,
        0.284949227,
        -1.67891897
      ]
    },
    {
      "timeMs": 2966,
      "positionOffset": [
        3.987778177,
        0.28128332,
        -1.077589831
      ]
    },
    {
      "timeMs": 2967,
      "positionOffset": [
        3.987778177,
        0.28128332,
        -1.077589831
      ]
    },
    {
      "timeMs": 3000,
      "positionOffset": [
        4.026714002,
        0.278905338,
        -0.461441536
      ]
    },
    {
      "timeMs": 3033,
      "positionOffset": [
        4.026962015,
        0.276264298,
        0.092480328
      ]
    },
    {
      "timeMs": 3034,
      "positionOffset": [
        4.026962015,
        0.276264298,
        0.092480328
      ]
    },
    {
      "timeMs": 3066,
      "positionOffset": [
        4.032694932,
        0.276521372,
        0.297808047
      ]
    },
    {
      "timeMs": 3067,
      "positionOffset": [
        4.032694932,
        0.276521372,
        0.297808047
      ]
    },
    {
      "timeMs": 3100,
      "positionOffset": [
        4.004430835,
        0.29583245,
        0.313729661
      ]
    },
    {
      "timeMs": 3133,
      "positionOffset": [
        3.967033091,
        0.315653857,
        0.325197453
      ]
    },
    {
      "timeMs": 3134,
      "positionOffset": [
        3.967033091,
        0.315653857,
        0.325197453
      ]
    },
    {
      "timeMs": 3166,
      "positionOffset": [
        3.921411274,
        0.335881299,
        0.33269935
      ]
    },
    {
      "timeMs": 3167,
      "positionOffset": [
        3.921411274,
        0.335881299,
        0.33269935
      ]
    },
    {
      "timeMs": 3200,
      "positionOffset": [
        3.868475682,
        0.356410981,
        0.336708646
      ]
    },
    {
      "timeMs": 3233,
      "positionOffset": [
        3.809152745,
        0.37715355,
        0.337724148
      ]
    },
    {
      "timeMs": 3234,
      "positionOffset": [
        3.809152745,
        0.37715355,
        0.337724148
      ]
    },
    {
      "timeMs": 3266,
      "positionOffset": [
        3.744367104,
        0.398012943,
        0.336251224
      ]
    },
    {
      "timeMs": 3267,
      "positionOffset": [
        3.744367104,
        0.398012943,
        0.336251224
      ]
    },
    {
      "timeMs": 3300,
      "positionOffset": [
        3.675045423,
        0.418892316,
        0.332819458
      ]
    },
    {
      "timeMs": 3333,
      "positionOffset": [
        3.602134644,
        0.439712533,
        0.32795271
      ]
    },
    {
      "timeMs": 3334,
      "positionOffset": [
        3.602134644,
        0.439712533,
        0.32795271
      ]
    },
    {
      "timeMs": 3366,
      "positionOffset": [
        3.5265672,
        0.460393425,
        0.322203619
      ]
    },
    {
      "timeMs": 3367,
      "positionOffset": [
        3.5265672,
        0.460393425,
        0.322203619
      ]
    },
    {
      "timeMs": 3400,
      "positionOffset": [
        3.449290228,
        0.480845061,
        0.316158966
      ]
    },
    {
      "timeMs": 3433,
      "positionOffset": [
        3.371254309,
        0.501010417,
        0.310366967
      ]
    },
    {
      "timeMs": 3434,
      "positionOffset": [
        3.371254309,
        0.501010417,
        0.310366967
      ]
    },
    {
      "timeMs": 3466,
      "positionOffset": [
        3.293332272,
        0.52081355,
        0.305420545
      ]
    },
    {
      "timeMs": 3467,
      "positionOffset": [
        3.293332272,
        0.52081355,
        0.305420545
      ]
    },
    {
      "timeMs": 3500,
      "positionOffset": [
        3.216654436,
        0.540182331,
        0.301916307
      ]
    },
    {
      "timeMs": 3533,
      "positionOffset": [
        3.189239573,
        0.551814523,
        0.285873005
      ]
    },
    {
      "timeMs": 3534,
      "positionOffset": [
        3.189239573,
        0.551814523,
        0.285873005
      ]
    },
    {
      "timeMs": 3566,
      "positionOffset": [
        3.182699181,
        0.567386368,
        0.267508097
      ]
    },
    {
      "timeMs": 3567,
      "positionOffset": [
        3.182699181,
        0.567386368,
        0.267508097
      ]
    },
    {
      "timeMs": 3600,
      "positionOffset": [
        3.191017941,
        0.604282731,
        0.251224244
      ]
    },
    {
      "timeMs": 3633,
      "positionOffset": [
        3.217129378,
        0.678243215,
        0.245484822
      ]
    },
    {
      "timeMs": 3634,
      "positionOffset": [
        3.217129378,
        0.678243215,
        0.245484822
      ]
    },
    {
      "timeMs": 3666,
      "positionOffset": [
        3.269194815,
        0.80520242,
        0.26186223
      ]
    },
    {
      "timeMs": 3667,
      "positionOffset": [
        3.269194815,
        0.80520242,
        0.26186223
      ]
    },
    {
      "timeMs": 3700,
      "positionOffset": [
        3.352041234,
        0.992137682,
        0.304633891
      ]
    },
    {
      "timeMs": 3733,
      "positionOffset": [
        3.452543789,
        1.232545307,
        0.352588994
      ]
    },
    {
      "timeMs": 3734,
      "positionOffset": [
        3.452543789,
        1.232545307,
        0.352588994
      ]
    },
    {
      "timeMs": 3766,
      "positionOffset": [
        3.549127968,
        1.518975442,
        0.387288367
      ]
    },
    {
      "timeMs": 3767,
      "positionOffset": [
        3.549127968,
        1.518975442,
        0.387288367
      ]
    },
    {
      "timeMs": 3800,
      "positionOffset": [
        3.62293812,
        1.835334544,
        0.407044048
      ]
    },
    {
      "timeMs": 3833,
      "positionOffset": [
        3.661643486,
        2.161132494,
        0.411981549
      ]
    },
    {
      "timeMs": 3834,
      "positionOffset": [
        3.661643486,
        2.161132494,
        0.411981549
      ]
    },
    {
      "timeMs": 3866,
      "positionOffset": [
        3.661835453,
        2.473561657,
        0.404716841
      ]
    },
    {
      "timeMs": 3867,
      "positionOffset": [
        3.661835453,
        2.473561657,
        0.404716841
      ]
    },
    {
      "timeMs": 3900,
      "positionOffset": [
        3.629423317,
        2.750506837,
        0.390075258
      ]
    },
    {
      "timeMs": 3933,
      "positionOffset": [
        3.577881915,
        2.973091425,
        0.374059187
      ]
    },
    {
      "timeMs": 3934,
      "positionOffset": [
        3.577881915,
        2.973091425,
        0.374059187
      ]
    },
    {
      "timeMs": 3966,
      "positionOffset": [
        3.487479142,
        3.198014912,
        0.362084493
      ]
    },
    {
      "timeMs": 3967,
      "positionOffset": [
        3.487479142,
        3.198014912,
        0.362084493
      ]
    },
    {
      "timeMs": 3999,
      "positionOffset": [
        3.543767926,
        3.318116695,
        0.260299878
      ]
    },
    {
      "timeMs": 4000,
      "positionOffset": [
        3.545482591,
        3.321367978,
        0.25677636
      ]
    },
    {
      "timeMs": 4033,
      "positionOffset": [
        3.577391653,
        3.4994892,
        0.191226728
      ]
    },
    {
      "timeMs": 4034,
      "positionOffset": [
        3.577391653,
        3.4994892,
        0.191226728
      ]
    },
    {
      "timeMs": 4066,
      "positionOffset": [
        3.599683173,
        3.621214417,
        0.181759806
      ]
    },
    {
      "timeMs": 4067,
      "positionOffset": [
        3.599683173,
        3.621214417,
        0.181759806
      ]
    },
    {
      "timeMs": 4100,
      "positionOffset": [
        3.612407014,
        3.632867819,
        0.201396842
      ]
    },
    {
      "timeMs": 4133,
      "positionOffset": [
        3.569338504,
        3.56594363,
        0.246406227
      ]
    },
    {
      "timeMs": 4134,
      "positionOffset": [
        3.569338504,
        3.56594363,
        0.246406227
      ]
    },
    {
      "timeMs": 4166,
      "positionOffset": [
        3.453420092,
        3.45065601,
        0.327588408
      ]
    },
    {
      "timeMs": 4167,
      "positionOffset": [
        3.453420092,
        3.45065601,
        0.327588408
      ]
    },
    {
      "timeMs": 4200,
      "positionOffset": [
        3.229350809,
        2.943486293,
        0.495797172
      ]
    },
    {
      "timeMs": 4233,
      "positionOffset": [
        2.965526913,
        2.233577916,
        0.652043713
      ]
    },
    {
      "timeMs": 4234,
      "positionOffset": [
        2.965526913,
        2.233577916,
        0.652043713
      ]
    },
    {
      "timeMs": 4266,
      "positionOffset": [
        2.986140712,
        1.872237178,
        0.842713923
      ]
    },
    {
      "timeMs": 4267,
      "positionOffset": [
        2.986140712,
        1.872237178,
        0.842713923
      ]
    },
    {
      "timeMs": 4300,
      "positionOffset": [
        2.988956103,
        1.847695447,
        0.840507855
      ]
    },
    {
      "timeMs": 4333,
      "positionOffset": [
        2.97985908,
        1.872262556,
        0.84274375
      ]
    },
    {
      "timeMs": 4334,
      "positionOffset": [
        2.97985908,
        1.872262556,
        0.84274375
      ]
    },
    {
      "timeMs": 4366,
      "positionOffset": [
        2.956871653,
        1.834468179,
        0.949621234
      ]
    },
    {
      "timeMs": 4367,
      "positionOffset": [
        2.956871653,
        1.834468179,
        0.949621234
      ]
    },
    {
      "timeMs": 4400,
      "positionOffset": [
        2.929446528,
        1.783873634,
        1.088755398
      ]
    },
    {
      "timeMs": 4433,
      "positionOffset": [
        2.905421316,
        1.746461214,
        1.204496409
      ]
    },
    {
      "timeMs": 4434,
      "positionOffset": [
        2.905421316,
        1.746461214,
        1.204496409
      ]
    },
    {
      "timeMs": 4466,
      "positionOffset": [
        2.888502363,
        1.746723216,
        1.246274389
      ]
    },
    {
      "timeMs": 4467,
      "positionOffset": [
        2.888502363,
        1.746723216,
        1.246274389
      ]
    },
    {
      "timeMs": 4500,
      "positionOffset": [
        2.876016399,
        1.785742748,
        1.221268329
      ]
    },
    {
      "timeMs": 4533,
      "positionOffset": [
        2.864341209,
        1.843767694,
        1.172593198
      ]
    },
    {
      "timeMs": 4534,
      "positionOffset": [
        2.864341209,
        1.843767694,
        1.172593198
      ]
    },
    {
      "timeMs": 4566,
      "positionOffset": [
        2.855325042,
        1.912902107,
        1.105141099
      ]
    },
    {
      "timeMs": 4567,
      "positionOffset": [
        2.855325042,
        1.912902107,
        1.105141099
      ]
    },
    {
      "timeMs": 4600,
      "positionOffset": [
        2.851077781,
        1.986120035,
        1.023528399
      ]
    },
    {
      "timeMs": 4633,
      "positionOffset": [
        2.862208739,
        2.056360682,
        0.933237637
      ]
    },
    {
      "timeMs": 4634,
      "positionOffset": [
        2.862208739,
        2.056360682,
        0.933237637
      ]
    },
    {
      "timeMs": 4666,
      "positionOffset": [
        2.887029849,
        2.116539331,
        0.840432967
      ]
    },
    {
      "timeMs": 4667,
      "positionOffset": [
        2.887029849,
        2.116539331,
        0.840432967
      ]
    },
    {
      "timeMs": 4700,
      "positionOffset": [
        2.908248257,
        2.159822544,
        0.751982138
      ]
    },
    {
      "timeMs": 4733,
      "positionOffset": [
        2.923473046,
        2.179033614,
        0.675239257
      ]
    },
    {
      "timeMs": 4734,
      "positionOffset": [
        2.923473046,
        2.179033614,
        0.675239257
      ]
    },
    {
      "timeMs": 4766,
      "positionOffset": [
        3.044778589,
        1.790651563,
        0.664418707
      ]
    },
    {
      "timeMs": 4767,
      "positionOffset": [
        3.044778589,
        1.790651563,
        0.664418707
      ]
    },
    {
      "timeMs": 4800,
      "positionOffset": [
        3.072551299,
        1.719155104,
        0.863180028
      ]
    },
    {
      "timeMs": 4833,
      "positionOffset": [
        3.056019326,
        1.706234857,
        1.049022558
      ]
    },
    {
      "timeMs": 4834,
      "positionOffset": [
        3.056019326,
        1.706234857,
        1.049022558
      ]
    },
    {
      "timeMs": 4866,
      "positionOffset": [
        2.995119628,
        1.649624418,
        1.153795096
      ]
    },
    {
      "timeMs": 4867,
      "positionOffset": [
        2.995119628,
        1.649624418,
        1.153795096
      ]
    },
    {
      "timeMs": 4900,
      "positionOffset": [
        2.917821665,
        1.580091893,
        1.243396681
      ]
    },
    {
      "timeMs": 4933,
      "positionOffset": [
        2.848428574,
        1.542502061,
        1.287889861
      ]
    },
    {
      "timeMs": 4934,
      "positionOffset": [
        2.848428574,
        1.542502061,
        1.287889861
      ]
    },
    {
      "timeMs": 4966,
      "positionOffset": [
        2.805683039,
        1.582948345,
        1.262889074
      ]
    },
    {
      "timeMs": 4967,
      "positionOffset": [
        2.805683039,
        1.582948345,
        1.262889074
      ]
    },
    {
      "timeMs": 5000,
      "positionOffset": [
        2.785663221,
        1.699918215,
        1.17786798
      ]
    },
    {
      "timeMs": 5033,
      "positionOffset": [
        2.77282063,
        1.852533121,
        1.062922008
      ]
    },
    {
      "timeMs": 5034,
      "positionOffset": [
        2.77282063,
        1.852533121,
        1.062922008
      ]
    },
    {
      "timeMs": 5066,
      "positionOffset": [
        2.767354483,
        2.028091762,
        0.926162611
      ]
    },
    {
      "timeMs": 5067,
      "positionOffset": [
        2.767354483,
        2.028091762,
        0.926162611
      ]
    },
    {
      "timeMs": 5100,
      "positionOffset": [
        2.769384937,
        2.214131782,
        0.775868903
      ]
    },
    {
      "timeMs": 5133,
      "positionOffset": [
        2.792436707,
        2.398431407,
        0.620822559
      ]
    },
    {
      "timeMs": 5134,
      "positionOffset": [
        2.792436707,
        2.398431407,
        0.620822559
      ]
    },
    {
      "timeMs": 5166,
      "positionOffset": [
        2.828613931,
        2.569277803,
        0.470314104
      ]
    },
    {
      "timeMs": 5167,
      "positionOffset": [
        2.828613931,
        2.569277803,
        0.470314104
      ]
    },
    {
      "timeMs": 5200,
      "positionOffset": [
        2.845076472,
        2.714953149,
        0.334309315
      ]
    },
    {
      "timeMs": 5233,
      "positionOffset": [
        2.833979834,
        2.823819406,
        0.223153626
      ]
    },
    {
      "timeMs": 5234,
      "positionOffset": [
        2.833979834,
        2.823819406,
        0.223153626
      ]
    },
    {
      "timeMs": 5266,
      "positionOffset": [
        2.853548386,
        2.377187724,
        0.434770081
      ]
    },
    {
      "timeMs": 5267,
      "positionOffset": [
        2.853548386,
        2.377187724,
        0.434770081
      ]
    },
    {
      "timeMs": 5300,
      "positionOffset": [
        2.987622509,
        2.238508798,
        0.605034079
      ]
    },
    {
      "timeMs": 5333,
      "positionOffset": [
        3.142766476,
        2.159642236,
        0.754130555
      ]
    },
    {
      "timeMs": 5334,
      "positionOffset": [
        3.142766476,
        2.159642236,
        0.754130555
      ]
    },
    {
      "timeMs": 5366,
      "positionOffset": [
        3.269915617,
        2.207745684,
        0.811991368
      ]
    },
    {
      "timeMs": 5367,
      "positionOffset": [
        3.269915617,
        2.207745684,
        0.811991368
      ]
    },
    {
      "timeMs": 5400,
      "positionOffset": [
        3.342962236,
        2.443358421,
        0.733571413
      ]
    },
    {
      "timeMs": 5433,
      "positionOffset": [
        3.40136236,
        2.805448262,
        0.554790215
      ]
    },
    {
      "timeMs": 5434,
      "positionOffset": [
        3.40136236,
        2.805448262,
        0.554790215
      ]
    },
    {
      "timeMs": 5466,
      "positionOffset": [
        3.487475178,
        3.19801768,
        0.362079406
      ]
    },
    {
      "timeMs": 5467,
      "positionOffset": [
        3.487475178,
        3.19801768,
        0.362079406
      ]
    },
    {
      "timeMs": 5500,
      "positionOffset": [
        3.337478092,
        3.065351209,
        0.360584082
      ]
    },
    {
      "timeMs": 5533,
      "positionOffset": [
        3.164218377,
        2.883818992,
        0.347583728
      ]
    },
    {
      "timeMs": 5534,
      "positionOffset": [
        3.164218377,
        2.883818992,
        0.347583728
      ]
    },
    {
      "timeMs": 5566,
      "positionOffset": [
        2.983448138,
        2.68555741,
        0.320581478
      ]
    },
    {
      "timeMs": 5567,
      "positionOffset": [
        2.983448138,
        2.68555741,
        0.320581478
      ]
    },
    {
      "timeMs": 5600,
      "positionOffset": [
        2.815040393,
        2.502948549,
        0.283671451
      ]
    },
    {
      "timeMs": 5633,
      "positionOffset": [
        2.67933739,
        2.364619545,
        0.249277463
      ]
    },
    {
      "timeMs": 5634,
      "positionOffset": [
        2.67933739,
        2.364619545,
        0.249277463
      ]
    },
    {
      "timeMs": 5666,
      "positionOffset": [
        2.597732894,
        2.293210779,
        0.235113737
      ]
    },
    {
      "timeMs": 5667,
      "positionOffset": [
        2.597732894,
        2.293210779,
        0.235113737
      ]
    },
    {
      "timeMs": 5700,
      "positionOffset": [
        2.55797742,
        2.261056432,
        0.234825086
      ]
    },
    {
      "timeMs": 5733,
      "positionOffset": [
        2.529202566,
        2.228073495,
        0.230798868
      ]
    },
    {
      "timeMs": 5734,
      "positionOffset": [
        2.529202566,
        2.228073495,
        0.230798868
      ]
    },
    {
      "timeMs": 5766,
      "positionOffset": [
        2.509950686,
        2.195819641,
        0.224350262
      ]
    },
    {
      "timeMs": 5767,
      "positionOffset": [
        2.509950686,
        2.195819641,
        0.224350262
      ]
    },
    {
      "timeMs": 5800,
      "positionOffset": [
        2.4987356,
        2.16590117,
        0.216847383
      ]
    },
    {
      "timeMs": 5833,
      "positionOffset": [
        2.494229095,
        2.139999179,
        0.2096555
      ]
    },
    {
      "timeMs": 5834,
      "positionOffset": [
        2.494229095,
        2.139999179,
        0.2096555
      ]
    },
    {
      "timeMs": 5866,
      "positionOffset": [
        2.495003213,
        2.119780303,
        0.204221222
      ]
    },
    {
      "timeMs": 5867,
      "positionOffset": [
        2.495003213,
        2.119780303,
        0.204221222
      ]
    },
    {
      "timeMs": 5900,
      "positionOffset": [
        2.499719979,
        2.106954561,
        0.202041375
      ]
    },
    {
      "timeMs": 5933,
      "positionOffset": [
        2.507038231,
        2.103260027,
        0.204638893
      ]
    },
    {
      "timeMs": 5934,
      "positionOffset": [
        2.507038231,
        2.103260027,
        0.204638893
      ]
    },
    {
      "timeMs": 5966,
      "positionOffset": [
        2.515586172,
        2.110492008,
        0.213601489
      ]
    },
    {
      "timeMs": 5967,
      "positionOffset": [
        2.515586172,
        2.110492008,
        0.213601489
      ]
    },
    {
      "timeMs": 6000,
      "positionOffset": [
        2.523961597,
        2.130485965,
        0.230516675
      ]
    },
    {
      "timeMs": 6033,
      "positionOffset": [
        2.530675903,
        2.165072788,
        0.257002068
      ]
    },
    {
      "timeMs": 6034,
      "positionOffset": [
        2.530675903,
        2.165072788,
        0.257002068
      ]
    },
    {
      "timeMs": 6066,
      "positionOffset": [
        2.534123963,
        2.216137314,
        0.294637255
      ]
    },
    {
      "timeMs": 6067,
      "positionOffset": [
        2.534123963,
        2.216137314,
        0.294637255
      ]
    },
    {
      "timeMs": 6100,
      "positionOffset": [
        2.530837743,
        2.285567883,
        0.344882131
      ]
    },
    {
      "timeMs": 6133,
      "positionOffset": [
        2.518206291,
        2.375244889,
        0.409101413
      ]
    },
    {
      "timeMs": 6134,
      "positionOffset": [
        2.518206291,
        2.375244889,
        0.409101413
      ]
    },
    {
      "timeMs": 6166,
      "positionOffset": [
        2.495592995,
        2.486934135,
        0.488454916
      ]
    },
    {
      "timeMs": 6167,
      "positionOffset": [
        2.495592995,
        2.486934135,
        0.488454916
      ]
    },
    {
      "timeMs": 6200,
      "positionOffset": [
        2.48932074,
        2.758116336,
        0.664420434
      ]
    },
    {
      "timeMs": 6233,
      "positionOffset": [
        2.474622099,
        3.243288967,
        0.944799373
      ]
    },
    {
      "timeMs": 6234,
      "positionOffset": [
        2.474622099,
        3.243288967,
        0.944799373
      ]
    },
    {
      "timeMs": 6266,
      "positionOffset": [
        2.400782429,
        3.802149078,
        1.228827585
      ]
    },
    {
      "timeMs": 6267,
      "positionOffset": [
        2.400782429,
        3.802149078,
        1.228827585
      ]
    },
    {
      "timeMs": 6300,
      "positionOffset": [
        2.266755222,
        4.297370432,
        1.464850599
      ]
    },
    {
      "timeMs": 6333,
      "positionOffset": [
        2.084357845,
        4.637020735,
        1.65302511
      ]
    },
    {
      "timeMs": 6334,
      "positionOffset": [
        2.084357845,
        4.637020735,
        1.65302511
      ]
    },
    {
      "timeMs": 6366,
      "positionOffset": [
        1.83605727,
        4.861797924,
        1.827187792
      ]
    },
    {
      "timeMs": 6367,
      "positionOffset": [
        1.83605727,
        4.861797924,
        1.827187792
      ]
    },
    {
      "timeMs": 6400,
      "positionOffset": [
        1.533585096,
        5.058736257,
        2.003438696
      ]
    },
    {
      "timeMs": 6433,
      "positionOffset": [
        1.209754289,
        5.237954791,
        2.166179333
      ]
    },
    {
      "timeMs": 6434,
      "positionOffset": [
        1.209754289,
        5.237954791,
        2.166179333
      ]
    },
    {
      "timeMs": 6466,
      "positionOffset": [
        0.894177617,
        5.405953939,
        2.3051919
      ]
    },
    {
      "timeMs": 6467,
      "positionOffset": [
        0.894177617,
        5.405953939,
        2.3051919
      ]
    },
    {
      "timeMs": 6500,
      "positionOffset": [
        0.614201137,
        5.563542569,
        2.418968494
      ]
    },
    {
      "timeMs": 6533,
      "positionOffset": [
        0.397593996,
        5.703626542,
        2.512989072
      ]
    },
    {
      "timeMs": 6534,
      "positionOffset": [
        0.397593996,
        5.703626542,
        2.512989072
      ]
    },
    {
      "timeMs": 6566,
      "positionOffset": [
        0.274101123,
        5.812584114,
        2.595580688
      ]
    },
    {
      "timeMs": 6567,
      "positionOffset": [
        0.274101123,
        5.812584114,
        2.595580688
      ]
    },
    {
      "timeMs": 6600,
      "positionOffset": [
        0.215726175,
        5.893008661,
        2.678217367
      ]
    },
    {
      "timeMs": 6633,
      "positionOffset": [
        0.173653395,
        5.95768069,
        2.765520657
      ]
    },
    {
      "timeMs": 6634,
      "positionOffset": [
        0.173653395,
        5.95768069,
        2.765520657
      ]
    },
    {
      "timeMs": 6666,
      "positionOffset": [
        0.148795846,
        6.006950592,
        2.854565056
      ]
    },
    {
      "timeMs": 6667,
      "positionOffset": [
        0.148795846,
        6.006950592,
        2.854565056
      ]
    },
    {
      "timeMs": 6700,
      "positionOffset": [
        0.142485074,
        6.041739487,
        2.942205291
      ]
    },
    {
      "timeMs": 6733,
      "positionOffset": [
        0.155466692,
        6.063360725,
        3.025239001
      ]
    },
    {
      "timeMs": 6734,
      "positionOffset": [
        0.155466692,
        6.063360725,
        3.025239001
      ]
    },
    {
      "timeMs": 6766,
      "positionOffset": [
        0.188368815,
        6.073113427,
        3.100511528
      ]
    },
    {
      "timeMs": 6767,
      "positionOffset": [
        0.188368815,
        6.073113427,
        3.100511528
      ]
    },
    {
      "timeMs": 6800,
      "positionOffset": [
        0.242242756,
        6.072186223,
        3.165013505
      ]
    },
    {
      "timeMs": 6833,
      "positionOffset": [
        0.317978675,
        6.06154715,
        3.215924163
      ]
    },
    {
      "timeMs": 6834,
      "positionOffset": [
        0.317978675,
        6.06154715,
        3.215924163
      ]
    },
    {
      "timeMs": 6866,
      "positionOffset": [
        0.416494852,
        6.041989507,
        3.25050739
      ]
    },
    {
      "timeMs": 6867,
      "positionOffset": [
        0.416494852,
        6.041989507,
        3.25050739
      ]
    },
    {
      "timeMs": 6900,
      "positionOffset": [
        0.53882126,
        6.01404791,
        3.266270935
      ]
    },
    {
      "timeMs": 6933,
      "positionOffset": [
        0.686403971,
        5.978217494,
        3.260832895
      ]
    },
    {
      "timeMs": 6934,
      "positionOffset": [
        0.686403971,
        5.978217494,
        3.260832895
      ]
    },
    {
      "timeMs": 6966,
      "positionOffset": [
        1.143760008,
        6.035881938,
        3.081238404
      ]
    },
    {
      "timeMs": 6967,
      "positionOffset": [
        1.143760008,
        6.035881938,
        3.081238404
      ]
    },
    {
      "timeMs": 7000,
      "positionOffset": [
        1.910135412,
        5.97298943,
        2.707682191
      ]
    },
    {
      "timeMs": 7033,
      "positionOffset": [
        2.546215245,
        5.267974298,
        2.355836334
      ]
    },
    {
      "timeMs": 7034,
      "positionOffset": [
        2.546215245,
        5.267974298,
        2.355836334
      ]
    },
    {
      "timeMs": 7066,
      "positionOffset": [
        2.806484972,
        2.868443512,
        2.116923982
      ]
    },
    {
      "timeMs": 7067,
      "positionOffset": [
        2.806484972,
        2.868443512,
        2.116923982
      ]
    },
    {
      "timeMs": 7100,
      "positionOffset": [
        3.125281803,
        0.40064534,
        1.931819564
      ]
    },
    {
      "timeMs": 7133,
      "positionOffset": [
        3.125156709,
        0.400634297,
        1.931808431
      ]
    },
    {
      "timeMs": 7134,
      "positionOffset": [
        3.125156709,
        0.400634297,
        1.931808431
      ]
    },
    {
      "timeMs": 7166,
      "positionOffset": [
        3.119937661,
        0.397866931,
        1.928219866
      ]
    },
    {
      "timeMs": 7167,
      "positionOffset": [
        3.119937661,
        0.397866931,
        1.928219866
      ]
    },
    {
      "timeMs": 7200,
      "positionOffset": [
        3.113927777,
        0.394647127,
        1.924012102
      ]
    },
    {
      "timeMs": 7233,
      "positionOffset": [
        3.109055573,
        0.391955123,
        1.920458804
      ]
    },
    {
      "timeMs": 7234,
      "positionOffset": [
        3.109055573,
        0.391955123,
        1.920458804
      ]
    },
    {
      "timeMs": 7266,
      "positionOffset": [
        3.109389621,
        0.391970019,
        1.920437969
      ]
    },
    {
      "timeMs": 7267,
      "positionOffset": [
        3.109389621,
        0.391970019,
        1.920437969
      ]
    },
    {
      "timeMs": 7300,
      "positionOffset": [
        2.946555033,
        0.494118332,
        2.00177849
      ]
    },
    {
      "timeMs": 7333,
      "positionOffset": [
        2.743272584,
        0.606837409,
        2.077923324
      ]
    },
    {
      "timeMs": 7334,
      "positionOffset": [
        2.743272584,
        0.606837409,
        2.077923324
      ]
    },
    {
      "timeMs": 7366,
      "positionOffset": [
        2.479644936,
        0.700661546,
        2.127418294
      ]
    },
    {
      "timeMs": 7367,
      "positionOffset": [
        2.479644936,
        0.700661546,
        2.127418294
      ]
    },
    {
      "timeMs": 7400,
      "positionOffset": [
        2.190385829,
        0.794159074,
        2.177518138
      ]
    },
    {
      "timeMs": 7433,
      "positionOffset": [
        1.968519188,
        0.891522587,
        2.203275649
      ]
    },
    {
      "timeMs": 7434,
      "positionOffset": [
        1.968519188,
        0.891522587,
        2.203275649
      ]
    },
    {
      "timeMs": 7466,
      "positionOffset": [
        1.884637021,
        0.995072601,
        2.176113337
      ]
    },
    {
      "timeMs": 7467,
      "positionOffset": [
        1.884637021,
        0.995072601,
        2.176113337
      ]
    },
    {
      "timeMs": 7484,
      "positionOffset": [
        1.884637021,
        0.995072601,
        2.176113337
      ]
    }
  ]
}
```


## hand.rows.candidate.tsv

```text
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	0	2029	4.424593773	0.455920281	2.519139769
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	1	2033	4.476010065	0.443536489	2.385082978
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	2	2034	4.476010065	0.443536489	2.385082978
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	3	2066	5.138144203	0.435482451	1.12584845
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	4	2067	5.138144203	0.435482451	1.12584845
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	5	2100	5.428145153	0.404348392	-0.189962485
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	6	2133	5.145589261	0.350987419	-1.658785802
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	7	2134	5.145589261	0.350987419	-1.658785802
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	8	2166	4.634127162	0.330060217	-3.201656698
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	9	2167	4.634127162	0.330060217	-3.201656698
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	10	2200	4.140481604	0.362695774	-4.377895179
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	11	2233	3.424615213	0.386309496	-4.526708393
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	12	2234	3.424615213	0.386309496	-4.526708393
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	13	2266	2.864183965	0.361580841	-4.33087509
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	14	2267	2.864183965	0.361580841	-4.33087509
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	15	2300	2.788269177	0.357471391	-4.322851828
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	16	2333	2.733252666	0.354362771	-4.318873414
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	17	2334	2.733252666	0.354362771	-4.318873414
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	18	2366	2.697794213	0.352325489	-4.31643759
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	19	2367	2.697794213	0.352325489	-4.31643759
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	20	2400	2.680119843	0.351303151	-4.313412381
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	21	2433	2.678295096	0.35118624	-4.307977856
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	22	2434	2.678295096	0.35118624	-4.307977856
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	23	2466	2.690280391	0.351788443	-4.298465206
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	24	2467	2.690280391	0.351788443	-4.298465206
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	25	2500	2.713950186	0.352950731	-4.283285116
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	26	2533	2.747234836	0.354379416	-4.26088774
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	27	2534	2.747234836	0.354379416	-4.26088774
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	28	2566	2.788082971	0.355824499	-4.229672163
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	29	2567	2.788082971	0.355824499	-4.229672163
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	30	2600	2.834496432	0.357027243	-4.187953909
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	31	2633	2.884604153	0.357699468	-4.133898123
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	32	2634	2.884604153	0.357699468	-4.133898123
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	33	2666	2.936629192	0.357562537	-4.06550057
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	34	2667	2.936629192	0.357562537	-4.06550057
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	35	2700	2.988523023	0.356407796	-3.980939562
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	36	2733	3.061456692	0.353812745	-3.877590664
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	37	2734	3.061456692	0.353812745	-3.877590664
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	38	2766	3.171428479	0.346389982	-3.698587901
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	39	2767	3.171428479	0.346389982	-3.698587901
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	40	2800	3.309737222	0.333098081	-3.40959791
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	41	2833	3.463355628	0.317412006	-3.042070881
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	42	2834	3.463355628	0.317412006	-3.042070881
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	43	2866	3.61988886	0.302538016	-2.625808259
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	44	2867	3.61988886	0.302538016	-2.625808259
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	45	2900	3.767405202	0.2913601	-2.188699009
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	46	2933	3.895115517	0.284949227	-1.67891897
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	47	2934	3.895115517	0.284949227	-1.67891897
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	48	2966	3.987778177	0.28128332	-1.077589831
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	49	2967	3.987778177	0.28128332	-1.077589831
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	50	3000	4.026714002	0.278905338	-0.461441536
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	51	3033	4.026962015	0.276264298	0.092480328
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	52	3034	4.026962015	0.276264298	0.092480328
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	53	3066	4.032694932	0.276521372	0.297808047
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	54	3067	4.032694932	0.276521372	0.297808047
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	55	3100	4.004430835	0.29583245	0.313729661
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	56	3133	3.967033091	0.315653857	0.325197453
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	57	3134	3.967033091	0.315653857	0.325197453
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	58	3166	3.921411274	0.335881299	0.33269935
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	59	3167	3.921411274	0.335881299	0.33269935
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	60	3200	3.868475682	0.356410981	0.336708646
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	61	3233	3.809152745	0.37715355	0.337724148
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	62	3234	3.809152745	0.37715355	0.337724148
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	63	3266	3.744367104	0.398012943	0.336251224
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	64	3267	3.744367104	0.398012943	0.336251224
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	65	3300	3.675045423	0.418892316	0.332819458
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	66	3333	3.602134644	0.439712533	0.32795271
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	67	3334	3.602134644	0.439712533	0.32795271
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	68	3366	3.5265672	0.460393425	0.322203619
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	69	3367	3.5265672	0.460393425	0.322203619
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	70	3400	3.449290228	0.480845061	0.316158966
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	71	3433	3.371254309	0.501010417	0.310366967
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	72	3434	3.371254309	0.501010417	0.310366967
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	73	3466	3.293332272	0.52081355	0.305420545
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	74	3467	3.293332272	0.52081355	0.305420545
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	75	3500	3.216654436	0.540182331	0.301916307
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	76	3533	3.189239573	0.551814523	0.285873005
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	77	3534	3.189239573	0.551814523	0.285873005
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	78	3566	3.182699181	0.567386368	0.267508097
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	79	3567	3.182699181	0.567386368	0.267508097
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	80	3600	3.191017941	0.604282731	0.251224244
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	81	3633	3.217129378	0.678243215	0.245484822
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	82	3634	3.217129378	0.678243215	0.245484822
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	83	3666	3.269194815	0.80520242	0.26186223
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	84	3667	3.269194815	0.80520242	0.26186223
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	85	3700	3.352041234	0.992137682	0.304633891
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	86	3733	3.452543789	1.232545307	0.352588994
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	87	3734	3.452543789	1.232545307	0.352588994
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	88	3766	3.549127968	1.518975442	0.387288367
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	89	3767	3.549127968	1.518975442	0.387288367
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	90	3800	3.62293812	1.835334544	0.407044048
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	91	3833	3.661643486	2.161132494	0.411981549
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	92	3834	3.661643486	2.161132494	0.411981549
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	93	3866	3.661835453	2.473561657	0.404716841
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	94	3867	3.661835453	2.473561657	0.404716841
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	95	3900	3.629423317	2.750506837	0.390075258
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	96	3933	3.577881915	2.973091425	0.374059187
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	97	3934	3.577881915	2.973091425	0.374059187
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	98	3966	3.487479142	3.198014912	0.362084493
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	99	3967	3.487479142	3.198014912	0.362084493
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	100	3999	3.543767926	3.318116695	0.260299878
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	101	4000	3.545482591	3.321367978	0.25677636
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	102	4033	3.577391653	3.4994892	0.191226728
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	103	4034	3.577391653	3.4994892	0.191226728
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	104	4066	3.599683173	3.621214417	0.181759806
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	105	4067	3.599683173	3.621214417	0.181759806
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	106	4100	3.612407014	3.632867819	0.201396842
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	107	4133	3.569338504	3.56594363	0.246406227
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	108	4134	3.569338504	3.56594363	0.246406227
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	109	4166	3.453420092	3.45065601	0.327588408
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	110	4167	3.453420092	3.45065601	0.327588408
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	111	4200	3.229350809	2.943486293	0.495797172
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	112	4233	2.965526913	2.233577916	0.652043713
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	113	4234	2.965526913	2.233577916	0.652043713
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	114	4266	2.986140712	1.872237178	0.842713923
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	115	4267	2.986140712	1.872237178	0.842713923
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	116	4300	2.988956103	1.847695447	0.840507855
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	117	4333	2.97985908	1.872262556	0.84274375
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	118	4334	2.97985908	1.872262556	0.84274375
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	119	4366	2.956871653	1.834468179	0.949621234
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	120	4367	2.956871653	1.834468179	0.949621234
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	121	4400	2.929446528	1.783873634	1.088755398
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	122	4433	2.905421316	1.746461214	1.204496409
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	123	4434	2.905421316	1.746461214	1.204496409
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	124	4466	2.888502363	1.746723216	1.246274389
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	125	4467	2.888502363	1.746723216	1.246274389
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	126	4500	2.876016399	1.785742748	1.221268329
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	127	4533	2.864341209	1.843767694	1.172593198
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	128	4534	2.864341209	1.843767694	1.172593198
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	129	4566	2.855325042	1.912902107	1.105141099
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	130	4567	2.855325042	1.912902107	1.105141099
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	131	4600	2.851077781	1.986120035	1.023528399
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	132	4633	2.862208739	2.056360682	0.933237637
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	133	4634	2.862208739	2.056360682	0.933237637
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	134	4666	2.887029849	2.116539331	0.840432967
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	135	4667	2.887029849	2.116539331	0.840432967
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	136	4700	2.908248257	2.159822544	0.751982138
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	137	4733	2.923473046	2.179033614	0.675239257
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	138	4734	2.923473046	2.179033614	0.675239257
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	139	4766	3.044778589	1.790651563	0.664418707
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	140	4767	3.044778589	1.790651563	0.664418707
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	141	4800	3.072551299	1.719155104	0.863180028
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	142	4833	3.056019326	1.706234857	1.049022558
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	143	4834	3.056019326	1.706234857	1.049022558
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	144	4866	2.995119628	1.649624418	1.153795096
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	145	4867	2.995119628	1.649624418	1.153795096
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	146	4900	2.917821665	1.580091893	1.243396681
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	147	4933	2.848428574	1.542502061	1.287889861
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	148	4934	2.848428574	1.542502061	1.287889861
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	149	4966	2.805683039	1.582948345	1.262889074
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	150	4967	2.805683039	1.582948345	1.262889074
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	151	5000	2.785663221	1.699918215	1.17786798
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	152	5033	2.77282063	1.852533121	1.062922008
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	153	5034	2.77282063	1.852533121	1.062922008
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	154	5066	2.767354483	2.028091762	0.926162611
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	155	5067	2.767354483	2.028091762	0.926162611
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	156	5100	2.769384937	2.214131782	0.775868903
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	157	5133	2.792436707	2.398431407	0.620822559
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	158	5134	2.792436707	2.398431407	0.620822559
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	159	5166	2.828613931	2.569277803	0.470314104
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	160	5167	2.828613931	2.569277803	0.470314104
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	161	5200	2.845076472	2.714953149	0.334309315
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	162	5233	2.833979834	2.823819406	0.223153626
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	163	5234	2.833979834	2.823819406	0.223153626
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	164	5266	2.853548386	2.377187724	0.434770081
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	165	5267	2.853548386	2.377187724	0.434770081
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	166	5300	2.987622509	2.238508798	0.605034079
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	167	5333	3.142766476	2.159642236	0.754130555
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	168	5334	3.142766476	2.159642236	0.754130555
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	169	5366	3.269915617	2.207745684	0.811991368
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	170	5367	3.269915617	2.207745684	0.811991368
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	171	5400	3.342962236	2.443358421	0.733571413
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	172	5433	3.40136236	2.805448262	0.554790215
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	173	5434	3.40136236	2.805448262	0.554790215
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	174	5466	3.487475178	3.19801768	0.362079406
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	175	5467	3.487475178	3.19801768	0.362079406
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	176	5500	3.337478092	3.065351209	0.360584082
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	177	5533	3.164218377	2.883818992	0.347583728
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	178	5534	3.164218377	2.883818992	0.347583728
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	179	5566	2.983448138	2.68555741	0.320581478
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	180	5567	2.983448138	2.68555741	0.320581478
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	181	5600	2.815040393	2.502948549	0.283671451
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	182	5633	2.67933739	2.364619545	0.249277463
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	183	5634	2.67933739	2.364619545	0.249277463
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	184	5666	2.597732894	2.293210779	0.235113737
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	185	5667	2.597732894	2.293210779	0.235113737
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	186	5700	2.55797742	2.261056432	0.234825086
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	187	5733	2.529202566	2.228073495	0.230798868
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	188	5734	2.529202566	2.228073495	0.230798868
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	189	5766	2.509950686	2.195819641	0.224350262
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	190	5767	2.509950686	2.195819641	0.224350262
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	191	5800	2.4987356	2.16590117	0.216847383
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	192	5833	2.494229095	2.139999179	0.2096555
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	193	5834	2.494229095	2.139999179	0.2096555
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	194	5866	2.495003213	2.119780303	0.204221222
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	195	5867	2.495003213	2.119780303	0.204221222
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	196	5900	2.499719979	2.106954561	0.202041375
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	197	5933	2.507038231	2.103260027	0.204638893
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	198	5934	2.507038231	2.103260027	0.204638893
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	199	5966	2.515586172	2.110492008	0.213601489
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	200	5967	2.515586172	2.110492008	0.213601489
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	201	6000	2.523961597	2.130485965	0.230516675
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	202	6033	2.530675903	2.165072788	0.257002068
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	203	6034	2.530675903	2.165072788	0.257002068
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	204	6066	2.534123963	2.216137314	0.294637255
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	205	6067	2.534123963	2.216137314	0.294637255
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	206	6100	2.530837743	2.285567883	0.344882131
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	207	6133	2.518206291	2.375244889	0.409101413
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	208	6134	2.518206291	2.375244889	0.409101413
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	209	6166	2.495592995	2.486934135	0.488454916
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	210	6167	2.495592995	2.486934135	0.488454916
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	211	6200	2.48932074	2.758116336	0.664420434
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	212	6233	2.474622099	3.243288967	0.944799373
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	213	6234	2.474622099	3.243288967	0.944799373
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	214	6266	2.400782429	3.802149078	1.2288275849999999
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	215	6267	2.400782429	3.802149078	1.2288275849999999
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	216	6300	2.266755222	4.297370432	1.464850599
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	217	6333	2.084357845	4.637020735	1.65302511
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	218	6334	2.084357845	4.637020735	1.65302511
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	219	6366	1.83605727	4.861797924	1.827187792
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	220	6367	1.83605727	4.861797924	1.827187792
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	221	6400	1.533585096	5.058736257	2.003438696
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	222	6433	1.209754289	5.237954791	2.166179333
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	223	6434	1.209754289	5.237954791	2.166179333
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	224	6466	0.894177617	5.405953939	2.3051919
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	225	6467	0.894177617	5.405953939	2.3051919
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	226	6500	0.614201137	5.563542569	2.418968494
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	227	6533	0.397593996	5.703626542	2.512989072
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	228	6534	0.397593996	5.703626542	2.512989072
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	229	6566	0.274101123	5.812584114	2.595580688
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	230	6567	0.274101123	5.812584114	2.595580688
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	231	6600	0.215726175	5.893008661	2.678217367
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	232	6633	0.173653395	5.95768069	2.765520657
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	233	6634	0.173653395	5.95768069	2.765520657
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	234	6666	0.148795846	6.006950592	2.854565056
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	235	6667	0.148795846	6.006950592	2.854565056
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	236	6700	0.142485074	6.041739487	2.942205291
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	237	6733	0.155466692	6.063360725	3.025239001
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	238	6734	0.155466692	6.063360725	3.025239001
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	239	6766	0.188368815	6.073113427	3.100511528
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	240	6767	0.188368815	6.073113427	3.100511528
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	241	6800	0.242242756	6.072186223	3.165013505
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	242	6833	0.317978675	6.06154715	3.215924163
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	243	6834	0.317978675	6.06154715	3.215924163
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	244	6866	0.416494852	6.041989507	3.25050739
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	245	6867	0.416494852	6.041989507	3.25050739
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	246	6900	0.53882126	6.01404791	3.266270935
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	247	6933	0.686403971	5.978217494	3.260832895
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	248	6934	0.686403971	5.978217494	3.260832895
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	249	6966	1.143760008	6.035881938	3.081238404
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	250	6967	1.143760008	6.035881938	3.081238404
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	251	7000	1.910135412	5.97298943	2.707682191
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	252	7033	2.546215245	5.267974298	2.355836334
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	253	7034	2.546215245	5.267974298	2.355836334
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	254	7066	2.806484972	2.868443512	2.116923982
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	255	7067	2.806484972	2.868443512	2.116923982
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	256	7100	3.125281803	0.40064534	1.931819564
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	257	7133	3.125156709	0.400634297	1.931808431
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	258	7134	3.125156709	0.400634297	1.931808431
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	259	7166	3.119937661	0.397866931	1.928219866
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	260	7167	3.119937661	0.397866931	1.928219866
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	261	7200	3.113927777	0.394647127	1.924012102
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	262	7233	3.109055573	0.391955123	1.920458804
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	263	7234	3.109055573	0.391955123	1.920458804
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	264	7266	3.109389621	0.391970019	1.920437969
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	265	7267	3.109389621	0.391970019	1.920437969
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	266	7300	2.946555033	0.494118332	2.00177849
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	267	7333	2.743272584	0.606837409	2.077923324
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	268	7334	2.743272584	0.606837409	2.077923324
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	269	7366	2.479644936	0.700661546	2.127418294
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	270	7367	2.479644936	0.700661546	2.127418294
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	271	7400	2.190385829	0.794159074	2.177518138
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	272	7433	1.968519188	0.891522587	2.203275649
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	273	7434	1.968519188	0.891522587	2.203275649
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	274	7466	1.884637021	0.995072601	2.176113337
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	275	7467	1.884637021	0.995072601	2.176113337
PATTERNLOGICHANDKEY	ENCOUNTER_KOUKUSAYDON_G1	KOUKUSAYDON_G1_PATTERN_17	KOUKUSAYDON_G1_PATTERN_17.logic.1	276	7484	1.884637021	0.995072601	2.176113337
```
