# 2026-09-05 발탄 잡기 왼손 presentation attachment RESULT

브랜치 `GB/KoukuSaydon-DataFormat`, 기준 HEAD `e61123d4`. 대응 계획은
`2026-09-05_VALTAN_GRAB_LEFT_HAND_PRESENTATION_ATTACHMENT_IMPLEMENTATION_PLAN.md`가 소유한다.
이 문서는 현재 working tree의 실제 G01 구현과 이 세션이 직접 실행한 검증 증거만 기록한다.

현재 상태는 **G01 구현 반영, targeted Python/C++ harness PASS, Debug Product 정본 runner exit 0 PASS(2회차),
Debug Core/FullDiagnostic 미실행, 사용자 화면 판정 대기**다.

## 1. 범위와 완료 상태

| 범위 | 구현 상태 | 검증 상태 |
|---|---|---|
| G01 원인 확정: C3가 파츠 합성 뒤(Level Update)에 실행돼 한 번도 화면에 나오지 않았다 | 실측 완료 | `GameInstance.cpp` 156행 순서, `PartObject.cpp` 47행, `Part_Body.cpp` 53행으로 확인 |
| G01 `CCharacter::Update` 안 파츠 합성 전 socket 위치 writer | 구현 반영 | grip 5/5, Product compile PASS |
| G01 `CValtan` `IPlayerHandGripSocketSource` 구현과 encounter-wide grip admission | 구현 반영 | grip 5/5, Product compile PASS |
| G01 replication → Character attachment 연결 | 구현 반영 | grip 5/5, 인접 33/33 PASS |
| G01 release 0.2초 합류 blend | 구현 반영 | compile PASS, 화면 판정 PENDING |
| G01 `Compose_WorldPosition` 수학 harness | 구현 반영 | `PlayerHandGripTransformContractTests` 4/4 PASS |
| G01 계약 주석·Tool 안내·팀 문서 교정 | 반영 | `git diff --check` PASS |
| Debug Product 정본 runner | 실행 | 1회차 receipt gate FAIL(동시 편집), 2회차 exit 0 PASS(4.1절) |
| Debug Core/FullDiagnostic | 미실행 | PENDING |
| Server+Client TRASH·CATCH_BREATH 화면 판정 | 미실행 | 사용자 판정 PENDING |
| G02 Server capture anchor를 손 offset으로 | 미구현 | 후속 계획 |

## 2. 실제 구현

### 2.1 원인

`Update_Engine`은 Object Update → Physics → Level Update(replication) → Late_Update 순서다. `Part_Body`와
`Part_Equipment`는 Object Update 단계의 `Update()`에서 `child × parent`를 `m_CombinedWorldMatrix`에 확정하고
Render는 그 값을 바인딩한다. 09-04 이전 C3는 Level Update에서 container transform을 덮어썼으므로 파츠에 한 번도
반영되지 않았고, 다음 프레임 `Update_NetworkTransform`이 S1로 되돌렸다. 09-04 G01이 C3를 지운 뒤에는 어떤 writer도
플레이어를 손으로 옮기지 않았다.

### 2.2 Client writer

- `Client/Public/PlayerHandGripTransform.h`: `PLAYER_HAND_GRIP_SOCKET_VIEW`, `IPlayerHandGripSocketSource`,
  `CPlayerHandGripTransform::Compose_WorldPosition` 추가. 기존 `PLAYER_HAND_GRIP_LOCAL_OFFSET`과 ±10 m
  validation은 그대로다. 변위는 socket translation + owner right·rightM + world up·upM + owner look·forwardM이고
  socket basis scale은 결과에 들어가지 않는다.
- `Client/Public/Character.h`, `Client/Private/Character.cpp`: `Apply_NetworkAttachment`,
  `Clear_NetworkAttachment`, `Update_NetworkAttachmentTransform`과 attachment 멤버 6개.
  `CCharacter::Update`는 `Update_NetworkTransform` 직후, `__super::Update`(파츠 합성) 직전에
  `Update_NetworkAttachmentTransform`을 호출해 GRABBED 동안 `POSITION`만 socket 위치로 교체한다. yaw는 Server yaw
  그대로다. release 첫 프레임부터 0.2초 smoothstep으로 마지막 손 위치에서 C1 보간 위치에 합류하고, 10 m 초과
  teleport reset은 blend를 취소한다. `Apply_NetworkAction`의 도달 불가 중복 GRABBED 분기 2곳을 삭제했고 실제
  잡힘 pose는 기존과 같은 IDLE loop다.
- `Client/Public/Valtan.h`, `Client/Private/Valtan.cpp`: `CValtan`이 `IPlayerHandGripSocketSource`를 구현한다.
  `Try_Get_PlayerHandGripSocketView`는 `bip001-l-hand` bone × `Try_Get_PresentationRootMatrix`와 actor transform을
  돌려주고 dormant/bone 부재면 false다. `Reload_PlayerHandGripLocalOffset_WhileAdmitted`가 joined presentation
  reload의 한 단계로 `ValtanEncounter.json`의 CAPTURE `gripLocalOffset`을 encounter-wide 단일 값으로 admission하며
  값 불일치·범위 밖·bone 부재는 fail-closed로 이전 cache를 보존한다. ghost pool donor copy도 같은 bone 검증 뒤 값을
  복사한다.
- `Client/Private/ClientReplication.cpp` `Apply_WorldSnapshot` player loop: GRABBED면 owner world entity의
  `pValtan`을 `Apply_NetworkAttachment`로 넘기고, 아니면 `Clear_NetworkAttachment`한다. owner presentation이
  없거나 isolated면 조용히 S1 fallback이고, owner는 있는데 admitted grip이 없을 때만 presentation failure 문자열을
  남긴다.

### 2.3 Server와 Shared

Server `Capture_PlayerAttachment`, `Update_PlayerAttachment`, `Release_PlayerAttachment`, `Prepare_ArenaEjection`과
packet schema는 바꾸지 않았다. `PacketMessages.h`의 GRABBED, `PLAYER_ATTACHMENT_SLOT`, attachment offset 주석만
"Client가 owner presentation socket 위에 몸통을 그리고 Server 위치는 판정·release 정본"으로 교정했다. Server가 보는
잡힌 플레이어 위치는 계속 capture 지점 기준 S1이다.

### 2.4 Tool과 문서

- `ValtanActionWorkbench.cpp` Left-hand Grip Detail 안내를 runtime 적용 계약으로 교체했다.
- `.md/TEAM/발탄인수인계서.md`, `.md/TEAM/TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md`의 잡기 계약 문단을 교체했다.
- `Tools/ValtanPipeline/test_valtan_grip_local_offset_contract.py`의 Client source oracle을 "파츠 합성 전
  `CCharacter::Update` 단일 writer, Level Update writer 금지, Valtan socket/grip 소유" 계약으로 교체했다.
- `Tools/ValtanPatternAuditionServiceHarness/Private/PlayerHandGripTransformContractTests.cpp`를 추가하고
  vcxproj/filters/main gate에 등록했다.

## 3. 자동 검증

| 검증 | 결과 |
|---|---|
| `Tools.ValtanPipeline.test_valtan_grip_local_offset_contract` | 5/5 PASS |
| `test_valtan_combat_object_hit_effect_presentation_contract` + `test_valtan_pattern_target_effect_anchor_contract` + `test_world_entity_spawn_revision_contract` | 33/33 PASS |
| `ValtanPatternAuditionServiceHarness` (MSBuild Debug x64 standalone 후 실행) | exit 0, `PlayerHandGripTransformContractTests: 4/4 passed` 포함 전 suite PASS |
| `git diff --check` (Client, Shared, Tools, .md/TEAM) | PASS (기존 파일의 LF/CRLF 혼재 경고만 출력) |
| 변경 C++ 파일 line ending | 기존 CRLF 유지, `ClientReplication.cpp`/`PacketMessages.h`의 기존 LF 수 불변 |

## 4. Debug Product 정본 runner

`Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product` 1회차:

- Engine/Shared/Server/Client compile·link 성공, `Client.exe` 04:03:16 생성, `Compiled shader closure PASS for
  Debug|x64`, `product.debug.receipt.json` 작성.
- 마지막 `Assert-BuildRunStability`가 `Build source identity changed after compilation started.`로 throw.
  같은 working tree에서 다른 세션이 컴파일 중 `Client/Private/KoukuSaydonActionWorkbench.cpp`(04:03:02) 등
  KoukuSaydon 파일을 편집해 dirty identity가 바뀌었다. 이 세션이 만든 파일은 빌드 시작 전에 모두 저장됐다.
- 컴파일 오류 0건. 이 세션이 만든 파일에는 기존과 같은 C4819 code page 경고만 있다.

### 4.1 2회차 재시도

같은 명령을 04:08:52에 다시 실행했다. 결과는 exit 0, `Compiled shader closure PASS for Debug|x64`,
`Build and validation completed: Debug / Product`이며 receipt `out/BuildPipeline/receipts/product.debug.receipt.json`과
run evidence `out/BuildPipeline/runs/20260904T191142072Z-debug-product-e6306ef9.json`이 남았다. 1회차 gate FAIL은
동시 편집 때문이었고 이 변경 자체의 Product 계약은 PASS다.

## 5. 사용자 수동 검증

에이전트는 Client/UI를 실행하거나 visual PASS를 대신 판정하지 않았다. `Server + Client` profile로 다음을 확인한다.

1. Valtan Arena 진입 후 `VALTAN_TRASH` 돌진 STEP_08(또는 RETRY_RUSH)에 맞는 순간, 맞은 자리에 남지 않고 곧바로
   발탄 왼손 위치로 몸통이 이동하는지 확인한다. `CATCH_COUNTER → CATCH_PRE_IMPACT → CATCH_SLAM` 동안 손이
   움직이면 몸통·무기·nameplate가 함께 따라가는지 확인한다.
2. `VALTAN_CATCH_BREATH` STEP_02 뒤잡기에 맞으면 STEP_03 4초 동안 왼손에 붙어 있고, STEP_04 던지기에서 손
   위치에서 출발해 0.2초 안에 Server 던지기 경로로 합류한 뒤 arena 밖으로 날아가는지 확인한다.
3. 잡힌 동안 F1 `Show Combat Colliders`의 player wire가 몸통과 같은 위치에 있는지 확인한다. Server 판정 위치는
   여전히 맞은 자리이므로 다른 플레이어 skill이 잡힌 플레이어를 때리는 판정은 손 위치와 다를 수 있다.
4. 손 위치가 몸통과 어긋나면 `Data/Valtan/Valtan.gameplay.json` CAPTURE 7곳의 `gripLocalOffset`
   (`forwardM`/`upM`/`rightM`, boss yaw frame·world up 미터)을 같은 값으로 조정하고 projector로 Product를 다시
   투영한 뒤 Client를 재실행한다. 7곳 값이 다르면 Valtan presentation admission이 실패한다.
5. 발탄이 빠르게 회전·휘두를 때 몸통이 손보다 한 프레임 늦게 따라오는 정도가 허용 범위인지 판단한다.
   `Layer_Player`가 `Layer_WorldEntity`보다 먼저 갱신되므로 이 지연은 설계상 존재한다.

## 6. 남은 경계

- Server capture anchor(S1)는 여전히 맞은 자리 기준이다. release 순간 Server 던지기 시작점과 손 위치의 차이를
  줄이려면 G02(CAPTURE hit authored boss-local anchor → publisher → bootstrap → `GameplayCatalog` →
  `Capture_PlayerAttachment`)를 별도 수직 슬라이스로 구현해야 한다. Client bone 위치를 Server로 보내지 않는다.
- 한 프레임 손 지연을 없애려면 Engine `CPartObject`에 재합성 API를 추가하거나 layer 갱신 순서를 바꿔야 하며 이번
  범위가 아니다.
- Debug Core/FullDiagnostic는 미실행이다. 다른 세션의 편집이 멈춘 뒤 정본 runner를 다시 실행한다.
- 구현은 공유 dirty working tree에 반영됐으며 이 RESULT 작성 과정에서 stage/commit/push하지 않았다.
