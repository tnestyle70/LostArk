# 2026-09-07 쿠크 Gate·부모 분류·재생 묶음 통합 구현 계획서

> 문서 종류: 구현 계획서. 코드 전문을 포함하는 디테일 계획서가 아니다.
> 상태: 사용자 구현 승인 후 Gate·묶음 저작, Preview, Server 재생 경계 구현과 자동 검증·빌드·배포를 완료했다. 검증 증거와 남은 사용자 화면 확인은 대응 RESULT에서 관리한다.
> 기준점: `Fix KoukuSaydon Complete Play` 완료 뒤의 미커밋 변경을 보존했으며 작업 브랜치는 `codex/kouku-gate-pattern-bundles`다. Composition v3/revision 87, Shared protocol 66을 사용한다.

사용자는 쿠크와 거대세이튼의 패턴을 각각 튜닝하고, 저장된 묶음을 선택해 두 보스를 함께 재생하려고 한다. F1의 Gate 선택 방식은 유지한다. Composition Patterns에도 Gate를 먼저 선택하는 구조를 적용하고, 부모 분류·재생 묶음·자식 패턴을 구분한다. Shift 다중 선택은 실행 계약이 아니다.

## G00. 선행 작업과 구현 기준점

### 현재 확인한 상태

| 항목 | 2026-09-07 문서 작성 시점의 실측 |
|---|---|
| 선행 작업 | Codex 작업 `Fix KoukuSaydon Complete Play`, ID `01a07a6c-5f5a-78e2-947d-284d23b653f6` |
| 작업 상태 | 조회 당시 진행 중이며 최종 배포·파일 확인 단계라고 보고됨. 완료나 merge로 간주하지 않음 |
| 작업 디렉터리 / 브랜치 | `C:/Users/user/Desktop/LostArk` / `codex/kouku-complete-play-revision` |
| HEAD | `13fa34d7df9f37d1c697ddefe1e5e7aebfdafef9`. 이후 다수의 사용자·선행 작업 변경이 미커밋 상태 |
| Composition 정본 | `Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json`, formatVersion 2, revision 86 |
| 저장 패턴 | PRODUCT 6개, DRAFT 3개. Gate 2 등장 두 패턴은 조사 시점에 빈 DRAFT |
| 기존 Gate 2 자식 후보 | `KAKULSAYDON_G1_PATTERN_8` / Kouku, `KAKULSAYDON_G1_PATTERN_9` / Large Saydon |
| 현재 선택·저장 | `actorProfileId`로 목록을 나눔. Gate·부모·묶음의 명시 저장 관계는 없음 |
| F1 Complete Play | Gate 선택 UI가 있지만 `MainApp.cpp`의 재생 목록은 Gate 1 Product에 제한됨 |
| Server 재생 | 방의 audition 상태가 보스 하나와 Logic ledger 하나를 소유함 |
| Client 재생 | Product presentation은 보스 entity별 session 사용. 저작 Preview는 단일 pattern/session 사용 |

파일 위치에 `Gate1` 또는 기존 ID에 `G1`이 있다는 이유로 새 Gate 정보를 추론하지 않는다. 이번 변경에서는 기존 정본 경로와 기존 pattern ID를 보존하고 명시 Gate 필드로 분류한다. 물리 경로 전면 이동은 포함하지 않는다.

### 선행 작업 종료 후 이어받는 순서

1. 선행 작업의 최종 응답, 실제 diff와 다음 문서를 다시 읽는다.
   - `2026-09-07_KOUKU_COMPLETE_PLAY_REVISION_IMPLEMENTATION_PLAN.md` / `RESULT.md`
   - `2026-09-07_KOUKU_PATTERN_STAGING_AND_DARK_SCENE_IMPLEMENTATION_PLAN.md` / `RESULT.md`
   - `2026-09-07_WORLD_OBJECT_TOOL_IMPLEMENTATION_PLAN.md` / `RESULT.md`
2. 최신 Composition, Product, protocol version, Server catalog revision을 다시 측정한다. 이 문서의 86을 코드나 publish 명령에 고정하지 않는다.
3. 선행 작업의 Object Motion, WORLD 수명, Scene Profile 격리·blend 검증, reset yaw와 사용자 저장값을 보존한다.
4. 소유권 불명확한 dirty 파일을 자동 stage/commit하지 않는다. 선행 작업이 확정한 기준점에서 `codex/` 기능 브랜치를 사용한다. 다른 실행 중 작업과 공유한 checkout의 브랜치를 임의로 전환하지 않는다.
5. 이 문서를 최신 기준점에 맞춰 갱신한 뒤 G01부터 구현한다. 사용자의 `끝났다 바로 반영 들어가자` 승인으로 구현을 시작했으며 선행 변경은 `out/KoukuGateBundles/baseline`에 보존했다.

LAN 설정 결과는 server-host이며 endpoint는 `192.168.0.4:7777`이다. 조회 당시 not-listening은 설정 실패가 아니다. Client와 UI는 에이전트가 실행·조작하지 않는다.

## G01. Gate와 저장 관계

### 최종 계층과 역할

```text
Composition Patterns
Gate       [2관문]
Model View [All / Kouku / Large Saydon]

2관문
├─ 세이튼등장                            [부모 분류]
│  └─ 세이튼등장_동시                    [재생 묶음]
│     ├─ 쿠크_세이튼등장                 [Kouku]
│     └─ 거대세이튼_세이튼등장           [Large Saydon]
├─ 쿠크_독립패턴                         [Kouku]
└─ 거대세이튼_독립패턴                   [Large Saydon]
```

| 개념 | 저장 책임 | 재생 여부 |
|---|---|---|
| Gate | 관문 식별과 허용되는 보스 대상 | 분류·F1 관문 진입 |
| 부모 분류 | Gate 안의 이름 있는 폴더, 묶음의 정리 | 직접 재생하지 않음 |
| 재생 묶음 | 자식 pattern ID, 시작 offset, 공통 연출 | 연결된 자식 전체를 한 실행 단위로 재생 |
| 자식 / 독립 패턴 | 대상 보스, 모델, 기존 Stage와 모든 상세 occurrence | 선택한 패턴만 재생 가능 |
| Model View | 편집 세션의 표시 필터 | 저장 관계와 실행 범위를 바꾸지 않음 |

부모는 실제 GameObject나 보스가 아니다. 묶음도 모델을 소유하지 않는다. 패턴의 `NORMAL/MECHANIC` 분류와 `DRAFT/PRODUCT` 상태는 이 계층과 별도로 유지한다.

### 저장 계약

기존 Composition Document의 CAS Save와 parse → validate → stage → commit을 확장한다. 새 정본 파일이나 복제된 자식 타임라인을 만들지 않는다. 현재 formatVersion 2에서 마이그레이션 가능한 새 버전을 추가하며, 선행 작업이 버전을 변경했다면 그 최신 버전 위에서 번호를 확정한다.

| 저장 단위 | 추가하거나 유지할 정보 |
|---|---|
| Gate 식별 | `gateId`. 1·2·3관문·빙고를 구분하며 모르는 ID는 오류 |
| 부모 분류 | stable `folderId`, `gateId`, `displayName` |
| 재생 묶음 | stable `bundleId`, `gateId`, `folderId`, `displayName`, `authoringStatus`, 자식 연결, 공통 연출 |
| 자식 연결 | stable `memberId`, `patternId`, 정수 `startOffsetMs` |
| 기존 패턴 | 기존 `patternId`, `actorProfileId`, Stage/occurrence를 보존하고 `gateId`, `targetBossPlacementId` 추가 |
| 공통 연출 | 묶음 소유의 Camera·Scene Profile 참조와 명시 시간. 기존 resource definition과 occurrence 계약 재사용 |

자식 연결의 정본은 묶음의 members다. 같은 관계를 pattern의 parent ID에도 중복 저장하지 않는다. 한 패턴을 같은 Gate의 여러 묶음에서 재사용할 수 있으며 편집은 같은 pattern ID의 정의를 변경한다. 여러 곳에서 사용 중이면 Detail에 참조 목록을 보여준다. 어떤 묶음에도 연결되지 않은 패턴은 Gate 직하의 독립 패턴으로 표시한다.

같은 묶음에 같은 pattern 또는 같은 대상 보스를 두 번 연결하지 않는다. 묶음 중첩과 Gate 간 연결은 이번 범위에서 허용하지 않는다. 묶음의 순서는 표시 순서이고 시작 시각은 offset이 결정한다. 이름의 `_동시`/`_독립` 문자열을 실행 분기로 해석하지 않는다.

### 관문과 보스 대상

F1과 Composition은 같은 stable Gate 식별을 사용한다. 기존 `Get_DebugGates`의 관문 진입·HUD·플레이어 위치 책임은 유지하며, authoring 목록은 공통 Gate 정보로 표시한다. 현재 F1의 HUD focus 하나를 묶음 전체의 재생 대상으로 재사용하지 않는다.

2관문의 대상은 기존 placement `boss.kakulsaydon.g2.kouku`와 `boss.kakulsaydon.g2.big-saydon`이다. 각각 모델 `MN_RPCZ_00`, `MN_RPCT_06`과 검증한다. 모델은 클립 호환성, placement는 실제 실행 대상이다. 같은 모델을 쓰는 1·3관문 세이튼과 빙고도 Gate/placement로 구별한다.

신규 pattern에는 선택한 Gate의 실제 대상만 제시하고 대상 모델을 표시한다. 기존 데이터 마이그레이션은 현재 사용처와 원래 대상의 근거로 고정한다. 조사 시점 기존 PRODUCT 6개는 1관문을 유지하고 등장 DRAFT 8·9는 사용자 합의에 따라 2관문으로 배치한다. 후속 저장 항목은 최신 데이터를 확인한 뒤 매핑하며 표시 이름이나 모델만으로 관문을 일괄 추측하지 않는다.

빈 DRAFT 폴더·묶음·패턴은 생성·저장할 수 있다. 실행 가능한 묶음은 유효한 자식 한 개 이상을 요구하며 동시 재생 확인은 두 보스로 수행한다. 일부 자식이 DRAFT이면 묶음 PRODUCT 게시를 거절하고 원인을 표시한다. 빈 등장 DRAFT의 애니메이션·Effect·이동을 임의 제작해 PRODUCT로 승격하지 않는다.

**G01 종료 증거:** 기존 ID와 저작값 보존, Gate/target 매핑, 빈 DRAFT round-trip, 관계 오류와 손상 항목 격리, 관련 publisher의 이전 생성물 보존 확인.

## G02. Composition Patterns와 생성·편집

기존 `CKoukuSaydonActionWorkbench`의 Patterns 패널을 확장한다. 별도 상위 툴 창을 만들지 않는다.

| 명령 | 생성 위치와 동작 |
|---|---|
| Create Parent | 선택 Gate 아래에 이름 있는 부모 분류 생성 |
| Create Bundle | 선택 부모 아래에 이름 있는 빈 재생 묶음 생성 |
| Create Pattern | Gate·대상 보스·묶음을 표시하고 생성. 묶음 없음이면 Gate 직하 독립 패턴 |
| Link Existing Pattern | 같은 Gate의 기존 pattern ID를 선택 묶음에 연결 |
| Remove From Bundle | 연결만 제거. 패턴 정의와 튜닝은 보존 |
| Delete Pattern | 참조 중이면 참조 목록과 해제 방법을 표시하고 삭제 거절 |
| Delete Bundle / Parent | 자식이 남아 있으면 먼저 연결 해제/이동하도록 안내. 묵시적 cascade 삭제 없음 |

Create Pattern의 입력은 `Name`, `Target Boss`, `Bundle`이다. 묶음 선택 시 Bundle 기본값을 채우고 `생성 위치: 2관문 > 세이튼등장 > 세이튼등장_동시`를 표시한다. Gate 또는 부모만 선택했다고 첫 자식이나 새 묶음을 자동 선택·생성하지 않는다.

Model View의 기본은 All이다. 특정 모델을 선택하면 해당 모델의 독립 패턴과 그 모델이 참여한 부모·묶음을 보여준다. 보이는 묶음을 펼치면 모든 자식을 유지하고 일치하는 모델을 강조한다. Complete Play는 숨겨진 필터 상태가 아니라 저장된 전체 members를 실행한다. `2개 동시 재생` 같은 실행 대상 수를 버튼 옆에 명시한다.

새 패턴 대상의 기본값은 특정 Model View에서 가져올 수 있지만 실제 Target Boss 선택은 별도로 보여준다. Model View 변경으로 기존 pattern의 Gate·대상·묶음 관계를 수정하지 않는다. 필터 때문에 선택 항목이 사라지면 명시적으로 선택을 해제하며 보이지 않는 항목을 편집하지 않는다.

**G02 종료 증거:** Create/Link/Remove/Save/Reload 후 동일 트리와 ID, Model View에 의한 비변경, 부모 선택 시 첫 자식 자동 선택 없음, 참조 중 삭제 보존.

## G03. 선택별 Detail·Sequencer와 Preview

### 하나의 패널에서 선택 종류에 따라 전환

| 선택 | Detail | Sequencer | Play |
|---|---|---|---|
| Gate / 부모 분류 | 이름·연결 목록 등 분류 정보 | 묶음 또는 패턴 선택 안내 | 비활성 |
| 재생 묶음 | 이름·자식 연결·대상·offset·공통 연출 | 자식 한 개당 한 행 + 공통 Camera/Scene 행 | 묶음 미리보기 |
| 자식 / 독립 패턴 | 현재 패턴의 상세 설정 | 기존 Animation·Logic·World·Effect·Light 등 | 해당 패턴 미리보기 |

선택 상태는 `NONE/GATE/FOLDER/BUNDLE/PATTERN`과 stable ID로 구별한다. 기존 selectedPatternId 하나로 부모 선택을 흉내 내지 않는다. 묶음 선택 시 남아 있는 자식 Stage/box 선택·pending Append·drag 상태를 해제하여 이전 패턴을 수정하지 못하게 한다.

```text
묶음: 세이튼등장_동시
시간          0ms                                   →
쿠크          [ 쿠크_세이튼등장 ─────────── ]
거대세이튼    [ 거대세이튼_세이튼등장 ───────────────── ]
Camera        [ 묶음 공통 카메라 ]
Scene Profile [ 묶음 공통 암전 ]
```

자식 막대의 길이는 자식의 저작 duration으로 계산한다. 막대 이동은 member offset만 변경한다. 자식 막대 resize로 내부 clip 속도나 Stage 길이를 변경하지 않는다. 자식 행 더블클릭은 같은 자식 PATTERN 선택으로 전환하며 상단 경로에서 원래 묶음으로 돌아갈 수 있다. Pattern 편집값은 Document에 유지한다.

묶음의 표시 길이는 자식의 `offset + authored duration`과 공통 연출의 끝 중 최대값이다. 이것은 저작 Preview 길이이며, 조건 분기가 있는 Server Complete Play의 실제 완료 시각을 강제하지 않는다. 두 자식의 시작은 기본 0ms이다. offset은 음수가 아닌 정수이며 고정 tick에서 실제 적용되는 시작 시간을 함께 표시한다.

### Preview 실행 계약

기존 animation preview, CModel, `CKoukuSaydonPresentationPlayer`와 WorldSequence 경로를 확장한다. 현재 단일 Preview session을 두 번 Begin하여 첫 대상을 덮어쓰는 구현은 금지한다. 묶음 Preview는 members별 대상 모델·pivot/session과 하나의 공통 clock을 가진다. 보스의 저작 placement를 기준으로 각 pivot을 resolve한다.

Play/Pause/Seek/Stop은 공통 clock으로 모든 자식을 제어한다. 자식의 local time은 묶음 시각에서 offset을 뺀 값이며 시작 전에는 자식 occurrence를 만들지 않는다. offset의 tick 환산은 Server와 동일한 올림 정책을 사용하고 실제 적용 시각을 UI에 보여준다. Seek 반복에서 Effect·WORLD가 중복 생성되지 않게 기존 샘플링과 정리 경계를 확장한다.

Preview는 연출 시간 확인이며 Server collider 결과를 로컬에서 판정하지 않는다. 게임 조건·분기 검증은 Complete Play가 담당한다. Logic UI가 있는 것만으로 로컬 판정이 지원된다고 표시하지 않는다.

| 예외 | 표시와 처리 |
|---|---|
| 빈 묶음 | `연결된 패턴이 없습니다`, Play 비활성 |
| 누락·손상된 자식 / 대상 부재 | 오류 행과 해당 ID 표시, 묶음 Play 비활성. 첫 정상 자식으로 fallback 없음 |
| 선택 변경 | 기존 로컬 Preview 정리, 새 선택 cursor 0. 이전/default Sequence를 표시하지 않음 |
| Preview 시작 준비 실패 | 새 요청 rollback과 오류 표시. 같은 선택에서 재시작 준비 실패이면 기존 정상 session 보존 |
| 게시되지 않은 DRAFT | 저작 Preview는 허용, Server Complete Play는 게시 상태 안내 |
| Server Complete Play 중 편집 선택 변경 | UI 선택 변경만 수행. Server 실행을 암묵 중단하거나 다른 대상을 재생하지 않음 |

**G03 종료 증거:** 부모/묶음/자식 왕복 시 편집 대상 일치, 개별 데이터 보존, 묶음 offset·공통 cursor, 두 Preview 대상 유지, 시작 실패 rollback과 Stop 정리. 실제 화면 확인은 사용자 담당.

## G04. Product와 F1 Complete Play 목록

정본 흐름은 기존 Composition → `project_kouku_saydon_composition.py` → encounter/patternbindings → Gameplay publisher → Server catalog를 사용한다. F1은 게시된 Product를 읽고 authoring JSON 직접 읽기로 우회하지 않는다. Workbench는 DRAFT 포함, F1은 PRODUCT 기준이라는 차이를 유지한다.

F1의 Gate 선택 UI와 관문 진입 동작은 유지한다. `gateHasProduct = Gate 1` 제한을 실제 gateId별 Product 조회로 바꾸고, 같은 폴더·묶음·패턴 관계로 목록을 만든다. 표시할 게시 항목이 없는 부모는 숨기되 데이터가 없는 관문은 정상 빈 목록으로 안내한다.

| F1 선택 | Complete Play |
|---|---|
| 부모 분류 | 비활성, 실행할 묶음 선택 안내 |
| 묶음 | 대상 보스·자식 이름·offset을 나열하고 `Complete Play — N개 재생` |
| 묶음의 자식 | 선택 자식만 재생. 묶음 공통 연출과 형제는 실행하지 않음 |
| 독립 패턴 | 선택 패턴만 재생 |

묶음 실행 대상은 Server가 같은 revision의 게시된 bundle ID에서 resolve한다. Client가 임의 pattern ID 배열이나 모델 index만 보내어 원본 검증을 우회하지 않는다. 기존 Complete Play All의 순차 실행 의미는 유지하고 묶음 동시 재생 버튼으로 바꾸지 않는다. bundle ID를 기존 pattern ID 목록에 끼워 넣지 않는다.

문서 revision, Product의 Gate/부모/묶음/자식 관계와 bootstrap을 같은 게시 단위에서 일치시킨다. C++/Python/PowerShell의 exact property 검사, Client BossTool·presentation reader, Server catalog reader를 모두 갱신한다. 선행 작업의 Scene Profile row 격리와 일반 Effect fade 검증은 유지한다.

**G04 종료 증거:** Gate별 게시 목록, 원본/생성물 revision 일치, 묶음의 전체 자식 검증, DRAFT 게시 거절, 손상된 관계에서 정상 목록·기존 Product 보존.

## G05. Server의 묶음 시작·완료·중단

기존 `CKoukuSaydonPatternAuditionService` → typed command → Shared → `CGameRoom` audition 경계를 확장한다. 묶음 요청은 stable bundle ID와 Gate·revision scope를 한 번 전달한다. UI가 socket을 직접 호출하지 않는다.

현재 방의 audition owner는 보스 한 개, pattern 순서와 Logic ledger를 가진다. 이를 하나의 실행 session 아래 참여자별 boss entity, member ID, 현재 pattern/branch 상태, Logic ledger, scheduled start tick과 완료 상태를 소유하도록 확장한다. 두 번째 별도 보스 runtime을 만들지 않고 기존 `CKoukuSaydonBrain::Begin_Pattern`과 update를 각 참여자에 사용한다.

### 시작과 시간

1. 게시된 bundle과 pinned revision을 resolve한다.
2. 전체 자식·Gate·placement·archetype·모델 호환, 살아 있는 대상, busy 상태, reset 위치/yaw와 필요한 runtime 입력을 검증한다.
3. 중복 actor, 누락 대상 또는 잘못된 자식이 있으면 전체 미실행으로 거절한다. reset·발사·broadcast를 먼저 수행하지 않는다.
4. 모든 검증이 성공하면 하나의 session으로 commit하고 기준 Server tick을 확정한다.
5. 각 자식은 `startTick + ceil(startOffsetMs * fixedTickHz / 1000)`에 시작한다. 0ms인 두 자식은 같은 tick에서 시작한다.
6. 참여자별 snapshot의 pattern 시작 시각과 묶음의 시작·진행·종료 상태를 Client가 소비한다. loop 전체가 끝난 뒤 snapshot이 나가도록 하여 actor 처리 순서에 따른 한 tick 차이를 만들지 않는다.

예약된 자식도 시작 전부터 session이 소유한다. 다른 audition이 그 보스를 빼앗지 못한다. 시작 이후 대상 사망·despawn·Gate 전환 등으로 실행 불가가 되면 묶음을 ABORT하고 참여자의 관련 실행을 정리한다. 이미 재생된 시간을 되돌렸다고 보고하지 않는다.

### 완료와 수명

- 기본 완료 정책은 모든 자식 완료다. 먼저 끝난 자식 때문에 나머지와 공통 연출을 종료하지 않는다.
- 먼저 끝난 보스는 종료 자세/idle 정책대로 대기하고 스스로 새 패턴을 시작하지 않는다. 재생 완료와 session 소유권 해제를 구분한다.
- FOLLOWUP_PATTERN 등 기존 분기는 해당 참여자의 실행 흐름으로 이어진다. 최종 분기까지 끝나야 그 참여자가 완료된다. 한 자식의 결과가 방 전체 Logic ledger나 형제의 상태를 지우지 않는다.
- 명시 Stop, 실행 취소, 마지막 플레이어 퇴장, owner 상실·Gate 전환의 정리는 모든 참여자와 묶음 공통 연출에 적용한다.
- restart는 기존 묶음 정리 뒤 같은 게시 revision에서 새 실행 ID와 시작 tick으로 시작한다. 늦게 도착한 이전 실행의 응답/cue는 새 실행을 건드리지 않는다.
- 자식 단독 실행은 같은 실행 경로의 참여자 한 명으로 처리한다. 다른 보스는 기존 Gate 대기 상태를 유지한다.

`GameRoom.cpp`의 `auditionOwnsAnotherBoss`는 단일 boss ID 비교에서 참여자 집합 포함 여부로 바꾼다. 다른 보스를 즉시 abort한다고 가정하지 않는다. 현재 guard는 비선택 보스를 idle로 유지하므로 이 정책을 보존해 집합으로 확장한다.

Shared 요청·결과·lifecycle과 Client 상태는 session ID, bundle/member, 실제 보스, pinned revision과 시작 시각을 서로 연결해야 한다. protocol version은 선행 작업의 최종 번호 다음으로 올리고 모든 reader/writer와 기존 protocol 검사를 같은 변경 단위에서 갱신한다.

**G05 종료 증거:** 두 보스 공통 시작 tick, offset 시작, 단독 실행 보존, 한쪽 실패 시 무변경, 조기 완료·분기·Stop·despawn 정리, 중복/지연 요청 격리.

## G06. Client의 묶음 표현과 공통 연출

기존 `CKoukuSaydonPresentationPlayer`의 보스 entity별 Product session과 Server 시작 tick을 재사용한다. 모든 Client는 Server가 확정한 시간을 기준으로 연출을 샘플링한다. 다른 Client의 선택한 Model View나 로컬 클릭 시각은 재생에 영향을 주지 않는다.

Camera·Scene Profile은 묶음의 공통 트랙으로 저작한다. 묶음 재생에서 자식의 같은 전역 연출이 겹치면 게시/재생 검증에서 소유권 충돌을 표시한다. entity 순회 순서의 마지막 값이 우연히 이기는 현재 방식을 묶음 정책으로 사용하지 않는다. 기존 단독 패턴의 전역 연출을 조용히 삭제하거나 이동하지 않으며, 묶음 연결 시 공통 트랙으로 옮길 충돌을 명시적으로 해결한다.

공통 연출은 묶음 clock에서 한 번만 실행한다. 각 occurrence는 자신의 수명을 지키고, 묶음 종료/중단은 아직 살아 있는 공통 연출을 정리·복원한다. 모든 자식이 끝난 뒤 공통 트랙이 남으면 전체 정리하며, 자식 하나의 완료는 공통 복원을 유발하지 않는다. 자식 단독 실행에서는 그 자식에 저장된 연출만 재생한다.

보스 anchor Effect·Light와 자식 소유 WORLD는 각 자식의 대상/pivot을 사용한다. 모든 플레이어 대상 Light나 공용 WORLD를 양쪽에 중복 배치해 중복 생성하지 않게 저작 단계에서 실행 대상을 확인한다. 이번 공통 트랙 지원은 Camera·Scene Profile부터 연결하고, 다른 family를 소비자 없이 공통 lane으로 열지 않는다.

WORLD와 Effect occurrence의 실행 identity는 bundle 실행 세대·member·occurrence를 구별한다. 같은 asset ID를 쓴다고 같은 활성 객체로 취급하지 않는다. 기존 쿠크 원본 ID나 WorldSequence resource ID는 유지하고 실행 instance 소유권을 분리한다. 자식 Stop이 형제의 object/Effect를 해제하지 않게 기존 WorldSequence 및 Server WORLD cue 경계를 확인한다. 선행 작업에서 추가한 `PLAY_WORLD_OBJECT_MOTION`의 target occurrence와 lifetime도 이 소유권 안에서 유지한다.

묶음 공통 상태는 기존 typed replication/presentation 경계로 전달한다. Client는 전용 로컬 boss spawn이나 로컬 gameplay 판단을 추가하지 않는다. 보스별 session이 있다는 사실만으로 그룹 Preview·Server 실행·공통 연출이 이미 완성됐다고 처리하지 않는다.

**G06 종료 증거:** member별 anchor/instance 분리, Camera·Scene 충돌 검출과 한 번의 복원, WORLD/모션 수명 보존, Stop/Seek/restart의 중복 생성·형제 정리 방지. 최종 외형은 사용자 확인.

## G07. 파일별 변경 책임과 검증·인계

### 변경할 기존 파일

| 파일 또는 경로 | 이번 변경 책임 |
|---|---|
| `Client/Public/KoukuSaydonCompositionDocument.h`, `Client/Private/KoukuSaydonCompositionDocument.cpp` | Gate·folder·bundle·member 저장, 관계 검증, 이전 버전 읽기와 CAS 저장 |
| `Client/Public/KoukuSaydonActionWorkbench.h`, `Client/Private/KoukuSaydonActionWorkbench.cpp` | Gate/Model View, 생성/연결, 선택 종류, 묶음 Detail·Sequencer·Preview 명령 |
| `Client/Public/CompositionWorkbenchSession.h`, 기존 `SequencerTool`·MainApp Preview 소비 경로 | 실제 필요 범위의 typed 묶음 transport와 선택/Preview 연결 |
| `Client/Private/MainApp.cpp`, `Client/Public/MainApp.h` | F1 Gate별 Product 트리, 선택 종류·실행 대상 표시, 기존 Preview 명령 소비 |
| `Client/Public/Level_KakulSaydonArena.h`, `Client/Private/Level_KakulSaydonArena.cpp` | Gate stable 식별과 보스 대상 resolve, Preview의 각 대상·pivot, WORLD 이벤트 경계 |
| `Client/Public/KoukuSaydonBossTool.h`, `Client/Private/KoukuSaydonBossTool.cpp` | Product Gate/묶음 목록 reader와 실행 요청 |
| `Client/Public/KoukuSaydonPatternAuditionService.h`, `Client/Private/KoukuSaydonPatternAuditionService.cpp` | 묶음 요청·진행·완료 상태, typed command 전달 |
| `Client/Public/KoukuSaydonPresentationPlayer.h`, `Client/Private/KoukuSaydonPresentationPlayer.cpp` | 묶음 Preview와 공통 연출, 기존 entity별 Product session 연결 |
| 기존 `WorldSequencePlayer`·`ClientReplication`·typed command sink 경로 | 실행 세대/member별 WORLD·cue 소유권과 실제 소비자 연결에 필요한 변경 |
| `Shared/Public/Network/PacketMessages.h`, `Shared/Private/Network/PacketMessages.cpp`, `Shared/Public/Network/PacketType.h` | 묶음 request/result/lifecycle와 공통 clock, 새 protocol 계약 |
| `Server/Public/GameRoom.h`, `Server/Private/GameRoom.cpp` | 단일 room session의 참여자 집합, 원자 시작·상태·정리 |
| `Server/Public/GameplayCatalog.h`, `Server/Private/GameplayCatalog.cpp` | 게시된 Gate·묶음 정의 읽기와 revision/참조 검증 |
| 기존 `KoukuSaydonBrain`·`KoukuSaydonLogicRuntime` | 공통 tick에서 참여자별 실행, 분기와 ledger·정리 범위 |
| `Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py` | Gate·folder·bundle과 공통 연출을 기존 Product로 투영 |
| `Tools/GameplayPipeline/Publish-GameplayBalance.ps1` | 묶음 정의를 Server bootstrap에 검증·게시 |
| 기존 Composition 정본과 생성되는 encounter/patternbindings | 기존 저작값 유지, Gate/연결 관계 추가, 동일 source revision 배포 |

신규 C++ 파일은 현재 계획에 없다. 기존 프로젝트 등록을 유지한다. 구현 중 실제 소비 분리를 위해 새 파일이 필요해지면 같은 변경에서 해당 `.vcxproj`와 `.vcxproj.filters`의 필요한 항목만 추가하고 XML parse·컴파일로 확인한다. 리소스 바이너리 변경·추출·Drive 재배포는 이 기능의 선행 조건이 아니다.

### 기능 검증 순서

1. 기존 Composition projector 검사에 Gate/관계/참조 보존·게시 실패 사례를 추가해 실행한다. 별도 검사 프레임워크는 만들지 않는다.
2. 기존 Server contract에서 같은 tick 두 보스 시작, offset, 두 번째 대상 실패 시 부분 reset 없음, 조기 완료·분기·Stop을 확인한다.
3. 기존 NetworkProtocolHarness에서 변경 message의 왕복·잘못된 수·ID·enum·길이·버전 거절과 기존 단독 요청을 확인한다.
4. 변경 기능의 최소 컴파일 후 최종 Product 빌드와 해당 domain publish를 수행한다. 최신 revision을 확인해 기존 명령을 사용한다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon
python Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py --mode validate
git diff --check
```

실제 publish에는 실행 직전 읽은 revision을 `ExpectedKoukuSaydonSourceRevision`에 전달하여 동시에 저장된 다른 값을 덮어쓰지 않게 한다. publisher가 생성한 파일을 수동 편집하지 않는다. 바뀐 JSON/XML parse를 수행하고 컴파일·게시·자동 검사·수동 화면 검증 상태를 RESULT에 구분한다.

### 사용자 화면 확인

사용자는 저장 후 필요한 경우 Server와 Client를 종료하고 새 빌드를 실행한다. LAN host에서는 Visual Studio의 `Server + Client` profile을 Ctrl+F5로 시작한다.

- Composition → 2관문 → 부모 생성 → 묶음 생성 → 쿠크/거대세이튼 자식 생성 또는 기존 패턴 연결 → 각 자식 저작·Save.
- 묶음 선택 시 두 행, 자식 선택 시 상세 트랙, 부모 선택 시 안내만 표시되는지 확인.
- 두 자식의 저작 Preview와 묶음 공통 cursor·Seek·Stop을 확인.
- 실행할 자식과 묶음을 PRODUCT로 게시하고 Server 재시작 후 F1 → 2관문에서 목록 확인.
- 자식 Complete Play는 하나만, 묶음 Complete Play는 각 위치의 두 보스가 함께 시작하는지 확인.
- 한쪽 조기 완료, 서로 다른 offset, 공통 암전·카메라, Stop 뒤 복원을 확인.
- 기존 1관문 무력화·진짜 세이튼 찾기·댄스타임·룰렛과 Object Motion 연결의 영향 여부를 확인.

기존 등장 두 자식이 계속 빈 DRAFT이면 화면/실행 검증용으로 임의 클립을 영구 저장하지 않는다. 별도 미저장 저작 상태로 확인하거나 사용자가 실제 클립을 연결한 뒤 PRODUCT를 검사한다. 사용자 확인 전에는 visual PASS로 기록하지 않는다.

### 완료 문서와 후속 작업

구현 결과는 같은 폴더의 `2026-09-07_KOUKU_GATE_PATTERN_BUNDLE_RESULT.md`에 실제 완료·미완료와 검증 근거를 기록한다. 현재는 RESULT를 만들지 않는다. 실제 public 계약이 바뀐 시점에만 `CLAUDE.md`, `.md/TEAM/ANIMATION_TOOL_OWNER_HANDOFF.md`, `.md/TEAM/TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md`를 갱신한다.

이 계획은 Gate와 저장 묶음의 저작·재생 연결을 완성하는 범위다. Gate 2 전체 패턴 제작, 공 연출의 최종 튜닝, 무작위 춤 선택 로직, 범용 중첩 묶음과 임의 병렬 encounter scheduler는 별도 범위다. `Fix KoukuSaydon Complete Play`의 완료와 이 계획의 구현 완료를 구분한다.

## G08. Composition Sequencer 선택 묶음 편집 후속

`KoukuSaydonActionWorkbench`의 Duplicate는 선택된 Stage와 Animation의 owner Stage 순서를 사용한다. 완전 선택 Stage는 clock과 모든 자식을 보존하고, Animation만 선택한 Stage는 선택된 자식 구간으로 사본을 잘라 새 Stage를 만든다. 부모·자식 동시 선택은 한 번만 복제한다. 사본 전체를 가장 오른쪽 선택 owner Stage 뒤에 연속 삽입하고 원본 Stage와 다른 Pattern을 보존한다. 이는 PRODUCT의 Stage별 animation 계약을 유지하며 원본 Stage의 미선택 tail을 자르지 않는다.

Earlier/Later와 좌우 방향키는 선택 Stage와 선택 Animation의 owner를 중복 제거해 같은 방향으로 한 칸씩 이동한다. 연속 선택은 순서가 유지되고 이동 방향의 경계에 선택이 붙어 있으면 전체 이동을 거절하고 원래 순서를 보존한다. 선택 ID는 commit 이후에도 유지한다. ID·시간 한도와 현재 Document 검증을 통과한 후보만 한 번 commit하며 실패하면 draft·ordinal·저장 파일이 바뀌지 않는다.

기존 H/CPP와 기존 BossCompositionDocumentContractTests의 선택 편집 사례만 갱신한다. 새 C++ 파일이나 project/filter 등록은 없다. 최소 Client compile/link, 기존 native editor의 저장·재로드·실패 보존 사례, XML parse와 diff check를 실행한다. Client 입력 및 실제 배치는 사용자 확인으로 남긴다.

## G09. Parent 직속 독립 Pattern 생성

Create Pattern의 Parent와 Bundle을 별도 선택한다. Parent를 선택하고 Bundle을 None으로 두면 해당 Parent 직속의 독립 Pattern을 생성하며, None Parent는 기존 Gate 직속 위치다. 선택한 Parent 아래 Bundle만 생성 대상으로 제시한다. 트리 Parent 선택은 새 Pattern의 기본 Parent만 지정하고 Bundle을 자동 생성하지 않는다.

기존 Pattern struct에 optional folderId를 저장하고, 누락은 기존 Gate 직속을 유지한다. 같은 Gate의 정상 Parent만 허용하며 잘못된 Parent는 원문 보존/검증 실패 경계를 따른다. Bundle 생성은 기존 members 관계만 저장하고 별도 folderId를 중복 지정하지 않는다. 직접 Parent에 배치한 Pattern을 나중에 Bundle에서 참조해도 원래 독립 분류 위치를 유지한다.

Workbench의 Create, 기존 Pattern Detail의 Parent 변경, Model View, Parent tree/Detail, Parent 삭제 금지와 Save/Reload를 함께 연결한다. 기존 Pattern의 Parent 변경은 저장된 Stage·Animation·Logic·Bundle 참조를 유지하고 분류 위치만 바꾼다. source를 소비하는 다른 saved/Product 목록도 실제 소비자에 필요한 분류 metadata를 유지한다. gameplay 실행은 기존 Pattern ID 및 Bundle member 계약을 그대로 사용한다. 사용자 `조커찾기` Parent나 성공 Pattern 데이터는 자동 생성·이동하지 않는다. 검증은 기존 native editor 및 Python projector의 정상 왕복, 기존 문서 호환, 누락·다른 Gate Parent 실패 보존, 필요한 Client 컴파일에 한정한다.


## G10. Collider 카드 접촉 Trigger와 공유 Logic 확장

중앙/가장자리 Collider가 여러 저장 카드 배치를 대상으로 타격창 동안 접촉을 판정하고, 맞은 카드별로 뒤집기/들썩기 Result를 실행한다. 기존 DURATION OBJECT_OVERLAP의 단일 고정 대상 계약은 보존한다. 신규 TRIGGER OBJECT_CONTACT는 안정적인 WORLD occurrence ID 목록과 반지름을 가지며, 각 target은 같은 Pattern에 배치한 카드 한 개와 정적 XZ 판정 원으로 resolve한다. 같은 saved 카드 instance를 여러 박스로 배치해도 occurrence ID와 실제 target cue로 구분한다. 타격 시점은 작성자가 Trigger 창의 Start/Lifetime으로 지정한다. 모델 본 또는 내려치는 높이를 Server 판정으로 추측하지 않는다.

Shared Logic window와 Logic definition 선택은 카드 접촉 Trigger를 모두 지원한다. 기존 공용 window 시간 동기화를 사용하며 Trigger의 피해 편집은 ENTER_AREA에만 적용한다. 카드별 접촉은 window마다 한 번 소비하고, 같은 contactGroupId/타격 시작시점에는 명시 priority가 높은 Trigger를 먼저 확정해 중앙·가장자리 중복 반응을 막는다. 같은 우선순위의 겹치는 그룹 창은 게시 단계에서 거절한다.

새 PLAY_CONTACT_WORLD_OBJECT_MOTION Result는 target→saved motion 매핑을 저장하여 접촉한 카드만 기존 World Motion typed 경로로 반응시킨다. COMPLETE_LOGIC_WINDOW Result는 optional 접촉 target 조건을 만족할 때 같은 Pattern의 DURATION EXTERNAL_SIGNAL window에 성공을 전달한다. EXTERNAL_SIGNAL의 성공은 기존 OnSuccess를 실행하고 종료 정책에 따라 Pattern을 끝내며, 실패 없이 제한시간이 끝나면 기존 OnTimeout을 실행한다. 타격 판정을 Timeout보다 먼저 처리하여 같은 마지막 tick의 성공을 보존한다. 사용자의 성공 Pattern/전멸 정의를 자동 생성·재배치하지 않는다.

Document parse/validate/stage/commit, Python projector, Gameplay publisher, Server catalog/runtime, Client shared-window 편집 및 기존 typed World presentation 소비를 같은 변경에 연결한다. 새 Shared wire나 두 번째 모델/충돌 runtime은 만들지 않는다. 기존 native editor/Python projector/Server contract의 접촉 다중 target·1회 소비·priority·타격창 밖·성공 후 timeout 취소·잘못된 mapping 저장/publish 거절 사례를 검증한다. Client/Server 최소 컴파일 및 최종 EXE 링크·배포 후 화면은 사용자 확인으로 남긴다.


## G11. 뿅망치 끝 Bone에 Collider 부착

Collider의 BOSS Anchor 아래에서 BODY/WEAPON과 실제 모델의 Bone을 선택한다. 기본 BODY는 이전 저장값과 호환된다. 실제 망치는 MN_RPCT_06의 별도 WP_MN_RPCT_06 모델이므로 weapon Bone combined → weapon local → body socket → body root 계층으로 끝점 위치를 구한다. Preview와 제품 CNpc가 같은 body/weapon clip mapping 및 source clock을 소비하며, 모델에 없는 Bone은 조용한 boss pivot 대체 없이 오류로 표시한다. Bone 이름은 이미 로드한 CModel의 이름 목록을 typed AnimationTargetService view로 제공하고 렌더마다 파일을 디코딩하지 않는다.

Source occurrence의 optional boneTarget=BODY/WEAPON과 기존 bone을 저장한다. Collider는 기존 TARGET_YAW 정책을 유지하여 Bone의 끝점 위치를 추적하고, offset·형상 방향은 보스 방향과 저작 Rotation을 사용한다. 망치 본의 pitch/roll은 끝점 위치 계산에 반영하지만 수평 gameplay 형상 자체의 pitch/roll로 해석하지 않는다. Collider를 활성화할 타격 순간은 Trigger의 짧은 Start/Lifetime이 소유한다.

publisher는 기존 ModelAssetConverter Python WModel reader/pose sampler를 재사용해 현재 source Stage·clip·sourceStart·playRate·end policy 및 BossCatalog model/weapon attachment 값에서 30Hz boss-local 끝점 track을 만든다. Product Stage는 source Stage와 1:1이며 기존 Brain의 누적 ceil tick과 첫 entry tick을 반영한 action-start clock을 사용한다. 정수 ms key 사이에 놓인 30Hz 판정 시각은 같은 pose의 양쪽 key로 감싸 전환 tick 보간 오차를 피한다. Preview는 저작 ms 시계의 실제 표시 pose를 직접 추적한다. 마지막 신규 CONTACT/EXTERNAL_SIGNAL deadline까지 최종 stage 수명을 유지하여 판정 전에 Pattern이 끝나지 않게 한다. 서버는 수치 track에 자신의 boss 위치/방향을 합성하며 모델 파일이나 Client snapshot을 판정 권위로 사용하지 않는다. 기존 WORLD/BOSS_SPAWN track은 유지한다. 잘못된 asset·Bone·clip·clock·표현 범위는 게시를 거절하고 기존 runtime 자료를 보존한다.

기존 Engine·Client·Server·projector 파일을 확장하며 새 C++ 파일은 만들지 않는다. Engine getter 변경은 Engine 빌드와 SDK 배포 후 Client 빌드로 검증한다. 실제 WModel의 BODY/WEAPON bone 좌표 및 시간 변화, 모델 전처리·attachment 합성, Server BOSS_CURRENT track과 timeout/priority 기존검사를 확인하고, 최종 육안 및 실제 append/배치는 사용자에게 남긴다.

## G12. World 목록·배치 후속의 사용자 중단과 재계획

사용자 요청으로 WORLD occurrence별 배치를 연결하던 중, Object Tool/Action Workbench 목록 통합·Map/Anchor(Character/Boss)·Save 즉시 적용을 먼저 계획하기로 변경됐다. 다른 작업 우선 지시에 따라 구현·빌드·배포와 다른 작업 메시지 전송을 중단했다. 그 전에 작성한 부분 변경은 기존 dirty worktree에 보존되어 있으며 통합 완료/빌드 검증 상태가 아니다.

현재 후속 계획의 정본은 [World Object Tool 구현 계획 G10~G15](2026-09-07_WORLD_OBJECT_TOOL_IMPLEMENTATION_PLAN.md)다. 이 문서의 기존 Gate/Bundle/CONTACT/Bone 완료 내용과 새 목록/배치/Save 통합 계획을 혼동하지 않는다. 사용자 재개 승인 후 Object 단일 목록·Append·절대 Map Transform·Save 자동 적용을 반영했다. 실제 완료 범위와 검증은 World Object Tool RESULT G10을 따른다.


## G13. Logic 겹침 행 표시

2026-09-08 사용자 요청: 같은 시점의 Logic 박스가 겹치면 후순위 박스를 한 행 위에 표시한다.
Workbench Render_Timeline은 Animation/Presentation에는 interval 행 배정이 있으나 Logic은
단일 logicLaneY만 사용하고 있다. 기존 최소 8px 박스 폭과 24px 행 높이를 그대로 사용해 Logic의
시작 시각·유효 표시 폭으로 행을 배정한다. 동시 시작은 저장 순서를 유지하며 후순위 충돌을 위에
배치하고, 행 공간만큼 전체 canvas와 아래 트랙을 확장한다. InvisibleButton과 DrawBox가 같은
logicBoxY를 사용한다. 저장 시간·ID·Logic 연결은 변경하지 않는다. 새 파일/프로젝트 등록 없이
Client 최소 컴파일과 diff check, 겹침·경계·축소 시 수치 배치를 확인한다. 실제 UI 선택은 사용자 확인이다.


## G14. 이름만 생성한 Trigger의 Collider 연결 안내

2026-09-08 Composition revision121의 사용자 Trigger 두 개는 logicType=TRIGGER만 있고 triggerKind는
비어 있었다. Collider 목록은 ENTER_AREA/OBJECT_CONTACT만 허용하므로 저장된 이름이 숨겨졌다.
Publish 여부와 무관한 draft 목록 필터 문제다. Logic definition 목록에서 이름만 있는 Trigger를 표시하고
설정 필요 상태를 구분한다. Card Contact 선택은 미저장 입력을 OBJECT_CONTACT/radius1로 stage하고
기존 Render_LogicDefinitionValues를 재사용해 대상 카드 선택 후 Apply Values로 검증·commit한다.
대상을 임의 지정하거나 빈 targets 허용으로 검증을 완화하지 않는다. Shared Logic은 설정된 현재
Pattern 창만 연결하고, 명시 ENTER_AREA 선택 전에는 별도 damage 생성 UI로 유도하지 않는다.
기존 C++ UI만 변경하며 JSON/Server/runtime 계약은 유지한다. 최소 Client 컴파일과 diff check,
기존 카드 Contact 생성·연결·Save/Reload 검사 결과를 구분해 기록한다.


## G15. 모든 박스 트랙의 공통 겹침 배치

2026-09-08 사용자 화면에서 Logic 2/Logic의 분리된 이름과 World의 겹친 카드가 확인됐다.
Collider와 동일하게 각 트랙 이름은 하나만 표시하고, 겹친 박스를 트랙 내부에서 아래로 쌓는다.
Render_Timeline의 Animation/Logic/Presentation별 row packing을 하나의 interval→lane 배정으로
합치고 Summon/World/Scene Profile도 같은 함수에 넣는다. 각 lane은 최소1행을 가지며 다음 lane은
직전 lane의 실제 행 수 뒤에 시작한다. 단일 rowY 계산을 draw/hit test/marquee/CAMERA tail에 사용한다.
카메라 return tail은 실제 그리는 최소8px 본문 폭+blendOut 폭으로 충돌 검사한다.
Stages는 누적 clock으로 이어지는 순차 구간이므로 기존 순서행을 유지하며 저장/시간/ID를 바꾸지 않는다.
새 파일이나 project/filter 등록은 없다. 실제 현재 카드 목록으로 interval 배치의 비겹침을 확인하고
최소 Client 컴파일/Debug Product 배포, 인코딩 보존 및 diff check를 수행한다. 사용자가 EXE를 종료했으며
화면/선택 결과는 배포 후 사용자가 확인한다. G13의 Logic만 위로 쌓는 규칙과 숫자 행 이름은 폐기한다.


## G16 — 쿠크 본체의 구간 이동과 원본 수직 pose 유지 (2026-09-08)

PATTERN_8의 `bossMotion`은 월드 시작점 `(2.04, 10.56, 316.95)`, 도착점 `(11.79, 10.56, 326.79)`, 패턴 시각 `1870~5780ms`, 고정 yaw `314.7368`을 저작한다. 시작 전에는 시작점, 종료 후에는 도착점을 유지한다. Server brain은 절대 패턴 시각에서 XZ를 선형 보간하고 기존 entity snapshot에 결과를 싣는다. 30Hz fixed tick에서는 경계가 최대 한 tick 늦게 관측된다. Client의 기존 b_root 수평 억제를 유지하고 원본 local Z(변환 후 월드 Y) pose를 그대로 재생하므로 별도 포물선이나 높이 곡선을 더하지 않는다. Server body의 기준 Y는 10.56이다.

기존 Valtan stage motion은 상대 전진/portal 이동 계약이므로 이 절대 위치 구간을 억지로 변환하지 않는다. optional pattern `bossMotion` 하나를 Composition parse/validate/save, Pattern Detail, projector, Gameplay publisher/reader, Kouku brain까지 연결한다. 새 Shared packet이나 두 번째 모델 런타임은 추가하지 않는다. resetBossToSpawn과 동시 사용 및 Y가 다른 양 끝점은 거부한다. 구간은 패턴 전체 시간 안에 있어야 하며 모든 수치는 유한해야 한다. 시작/도착의 navigation 유효성은 audition의 stage 전 검사로 기존 상태를 보존한다.

Play Bundle은 같은 Composition 공용 sampler를 써서 동일한 구간을 재생한다. 각 공의 생성점도 occurrence 시작 시각과 emission 시각을 합쳐 이 sampler로 고정하고, 생성 후에는 개별 공 궤적이 책임진다. 이동 종료 후 model의 원본 수평 왕복을 다시 더하지 않는다. Animation Model Reference의 제자리 비교 용도는 유지한다.

변경 대상은 기존 Composition H/CPP, ActionWorkbench Pattern Detail, PresentationPlayer의 boss 이동 구간, Level의 preview actor 초기화, GameplayCatalog H/CPP, KoukuSaydonBrain H/CPP, GameRoom admission/update, 기존 projector와 두 publisher다. 새 C++ 파일이나 project/filter 등록은 없다. 검증은 기존 projector unit test와 Kouku focused Server contract test에 정상 보간·끝점 유지·잘못된 구간 거부를 추가하고, root가 최소 컴파일과 해당 domain publish를 실행한다. Client 실행과 화면의 9시 방향·원본 점프 높이는 사용자가 확인한다.

## G17. 조커찾기 망치 Collider의 선택 Preview와 기본 앵커 연결

2026-09-08 사용자가 망치 앵커로 Play하면 중앙·주변 Collider가 함께 움직이도록 구현을 요청했다.
정밀 위치·크기 튜닝은 사용자가 맡는다. 현재 source revision133의 PATTERN_13 Collider 네 개는
BOSS/BODY, 빈 bone, offset0이며, 별도 망치 모델의 끝부분 본은 저장되지 않았다.
실제 WP_MN_RPCT_06 모델에서 내려치는 말단 b_rpct_01과 반대 말단 b_rpct_03을 확인했다.
기존 네 occurrence에 WEAPON/b_rpct_01을 연결하고 ID·시간·크기·offset·Logic은 보존한다.

선택 Box의 Preview는 MainApp의 animation 없는 resource preview로 전달되고, 이 경로는
플레이어 body/pivot를 사용하므로 세이튼 WEAPON view가 빠져 wire를 격리한다. named Bone Collider의
선택 Preview를 기존 Pattern Preview 경로로 보내 원래 actor·body/weapon animation과 패턴 시각을
보존한다. 미저장 Detail 값은 임시 Preview copy에만 적용하고 저장된 draft를 바꾸지 않는다.
일반 resource 형상 preview와 World/Effect 경로는 유지한다. 별도 모델 또는 앵커 runtime은 만들지 않는다.

수정 대상은 기존 KoukuSaydonActionWorkbench H/CPP, MainApp의 기존 Preview 소비 함수와
BossCompositionDocumentContractTests, source Composition이다. 새 C++ 파일/project 등록은 없다.
기존 native editor 검사에 source actor/clip/시각과 WEAPON/edit 보존, 잘못된 참조 실패 시 기존 상태
보존을 확인한다. 최소 Client 컴파일·Debug 링크/배포, 변경 JSON parse·diff check를 수행한다.
Client/UI 실행·캡처는 하지 않으며 중앙/주변 배치와 최종 말단 위치는 사용자 확인으로 남긴다.


## G18. 패턴별 root 수직 높이 조절 (2026-09-08)

현재 G2 쿠크는 bodyModelPreScale0.012053/presentationScale1에서 rpcz00_att_battle_7_01의 b_root가 5433.333ms에17.846225m 상승한다. PATTERN_8의 optional animationRootVerticalScale을0.8로 저장해 최고14.276980m로 낮춘다. 누락은1이며0..1의 유한 수만 허용한다. 모델 크기·animation 시각·수평 bossMotion은 보존한다. 수평 도착5780ms와 원본 pose 착지 약6100ms는 서로 다른 시점이다.

Engine CModel은 기존 root suppression의 보존축에 rest+(sample-rest)*scale을 적용한다. 실제 Play_Animation과 비파괴 bone sampler는 같은 helper를 소비한다. 미처리 root translation을 보관해 다음 프레임의 unkeyed fallback과 Begin_AnimBlend 원본을 복구하고 배율 중복을 막는다. Setter는 유효하지 않은 값에서 기존 상태를 유지한다.

Composition H/CPP와 Pattern Detail은 배율의 parse/validate/save/edit를 소유한다. 기존 projector가 각 Product action binding으로 배율을 보내고 body Bone Collider sample/cache도 같은 배율을 사용한다. Kouku action presentation reader가 stage한 값을 CNpc Play_NetworkAction의 기본값1인 인자로 넘긴다. Server snapshot의 새 action edge와 late-join seek 전에 적용하며 idle/death/다른 action은 기본값1로 복구한다. Preview는 같은 model setter를 animation 평가 전에 적용하고 actor 해제 시1로 복구한다. Model Reference는1을 유지한다.

새 Server 이동·Shared packet·모델 runtime·C++ 파일/project 등록은 없다. 기존 문서 native 테스트에 저장/재로드/잘못된 값 보존을 추가하고, 기존 WModel/projector 검사에는 실제 b_root 최고높이·원본 및 배율별 cache 격리·착지·0배율·Product action별 투영을 확인할 최소 케이스를 추가한다. live/비파괴 pose의 같은 helper 사용과 blend 반복 방지·clone 값 복사·복구는 현재 호출 경로를 검토한다. 실제 CModel을 평가하는 기존 자동 검사 경로가 없으므로 새 하네스를 만들거나 이 항목을 실행 PASS로 기록하지 않는다. 기본1에서는 기존 외부 BoneLocal 편집을 보존하도록 원본 cache 복구와 배율 산술을 생략한다. 빌드와 publish는 root가 수행하고 Client 조작과 최종 시각 판정은 사용자에게 남긴다.


## G19. Stop 포즈 유지와 정지 상태 타임라인 Scrub

Workbench Stop 버튼은 현재 clock을 보존한 PAUSE로 연결하고 기존 완전 해제는 Reset으로 제공한다.
내부 선택 변경/Reload의 Stop cleanup은 그대로 유지한다. Pattern과 Bundle ruler는 실행 여부와
관계없이 현재 cursor를 실제 preview에 전달하며, inactive 상태에는 기존 preview를 paused로 stage한다.
정확한 끝시각은 scrub에서 유지하고 Play에서는 기존0초 재시작 정책을 사용한다. Bundle consumer도
paused 끝시각에서 자동 해제하지 않는다. Collider Detail Preview는 같은 Pattern의 paused clock을
유지하면서 임시 편집값과 원본 BODY/WEAPON clip을 다시 샘플한다. Collider의 실제 활성 구간은 바꾸지 않는다.

실제 UI와 연결된 public transport API를 기존 native editor harness의 짧은 transport 모드에서 검사한다.
기존 전체 editor 검사는 반복하지 않는다. 신규 C++ 파일/project 등록이나 runtime 경로는 없으며
최소 Client 컴파일·필요 데이터 publish·scoped diff 검증 후 사용자에게 뿅망치 정지/드래그/Preview를 안내한다.

## G20. 조커찾기 대형 세이튼의 Bundle 중복 표시 정리 (2026-09-08)

source revision135의 GATE2 조커찾기 Bundle kakulsaydon.bundle.3은 이미 PATTERN_12 쿠크와 PATTERN_13 대형 세이튼을 멤버로 가진다. PATTERN_13에 folderId kakulsaydon.folder.3도 저장되어 Workbench의 Parent 직속 목록과 Bundle 멤버 목록에 같은 패턴이 두 번 표시된다. PATTERN_13의 folderId만 제거하고 revision136으로 갱신한다. 기존 Bundle 멤버 ID/offset, Pattern/clip/Collider/Logic/World/WEAPON Bone과 다른 원본 값은 보존한다. 별도 PATTERN_14 성공 표현은 동일 boss placement의 다른 패턴이며 현재 Bundle의 중복 actor 검증과 맞지 않으므로 자동으로 합치거나 삭제하지 않는다.

원본 JSON byte 기준 CAS를 확인하고 기존 Composition validator, JSON parse, 변경 전후 semantic 비교 및 diff check로 revision과 한 필드만 달라졌음을 검증한다. 코드나 저장 ID, Bundle 실행 순서를 바꾸지 않는다. publisher와 최종 빌드는 root 통합 작업에서 실행하며 Client/UI는 사용자가 확인한다. G19의 끝시각 정지 흐름에 맞춰 PresentationPlayer Resume은 기존 Seek_Preview(0)를 재사용하여 끝 clock에서 처음으로 돌아가고, paused 상태에서의 끝 pose 유지와 정상 재생의 자동 종료는 보존한다.

## G21. 조커찾기 Collider 실행 연결과 Stage 방향 통합 (2026-09-08)

G21 구현 시작 때 source revision144 이후 사용자 저장값에는 카드7개 배치와 망치 Collider6개, 이름만 있는
Trigger22/23 및 검색기한 Logic20/성공21이 있다. Collider Logic definition 선택과 Apply Values는
정의 편집만 수행하고 실제 logicOccurrenceId는 별도 Append에서 만들어져 연결이 빠진다.
기존 Apply 경로를 정의값·해당 시각 Logic occurrence·Collider 참조의 한 candidate commit으로
합친다. 같은 정의의 첫 시간창으로 이동하지 않고 정확한 시간창 또는 명시 Shared를 재사용한다.
손상/미완성 정의는 오류를 보존하고 기존 draft와 파일을 유지한다. 공개 Set_ColliderLogicValues는
정의값·정확한 Logic 시간창·Collider link를 한 candidate에서 검증한다. Apply Values와 Apply가
같은 저장 경로를 사용하며 명시 Shared 선택의 시간, 다음 타격의 별도 시간과 기존 결과 연결을 보존한다.

개별 P13은 Model Reference의 숨겨진 model yaw -90도, Bundle은 catalog pre-yaw0과 실제
placement yaw226.5도를 사용했다. MainApp은 actorProfileId가 있는 Pattern을 기존 임시 Bundle
Preview로 전달해 같은 actor/weapon/offset 기준을 사용한다. 명시 Model Reference는 유지한다.
Stage의 optional retargetOnEnter는 strict bool/기본false이며 bRetargetOnEnter, 기본 equality,
Composition parse/validate/save 및 Set_StageRetargetOnEnter/Stage Detail checkbox에 연결한다.
잘못된 타입은 원문을 보존해 격리하고 fixed-yaw bossMotion과의 동시 사용은 거부한다.
projector는 기존 RETARGET_RANDOM_ALIVE ENTER action만 생성한다. Brain은 해당 action만 허용하고
Gameplay publisher의 Kouku stage strict-field 검사도 optional `actions`를 같은 계약으로 받는다.
정확히 한 개의 ENTER/RETARGET_RANDOM_ALIVE/boss.target.pattern/value=1/durationMs=0만 허용하며,
BossMotion과 함께 쓰는 행은 Brain과 동일하게 거절한다. 기존 10-field PATTERNSTAGEACTION 행과
stage action ordinal 0으로 직렬화하여 Server의 기존 parser/action 소비자에 전달한다.
bootstrap 정렬은 같은 Logic window/slot의 OUTCOME 전체를 CONTACTMOTION/SIGNAL보다 먼저 기록한다.
World occurrence의 SEQUENCE 행도 PLACEMENT/SUPPORT보다 먼저 기록하며 각 종류 내부 원래 순서는 보존한다.
기존 숫자 자연 정렬과 공용 stage action/volley 순서는 유지하고 실제 생성물에서 의존 순서를 검사한다.
GameRoom은 Begin/Stage 변경 시 기존 action 소비자를 한 번 호출한다. Server가 선택한 alive player의
lastXYZ/yaw를 고정하며 Preview도 Stage 진입에서 현재 player 표본을 저장하고 다음 표본까지 유지한다.
STAGE_3/8/16만 활성화하고 원래 clip, 시간, 사용자 Collider offset/size와 카드 배치는 유지한다.
실제 대형세이튼 얼굴·눈·망치의 전방은 actor-local +X다. BIG_SAYDON의 retarget에만 일반 +Z 기준
atan2 결과의 -90도 보정을 Server/Preview에 적용한다. 다른 archetype과 모델 pre-yaw·geometry는
변경하지 않는다. 타깃 선택·yaw 유지 검사와 실제 망치축 정렬의 사용자 확인을 구분한다.

사용자가 확정한 중앙 뒤집기/HOLD, 양옆 들썩임/NEXT Idle, 조커 중앙 성공→P14를 기존
OBJECT_CONTACT→PLAY_CONTACT_WORLD_OBJECT_MOTION/COMPLETE_LOGIC_WINDOW→EXTERNAL_SIGNAL
→FOLLOWUP_PATTERN으로 연결한다. 기존 전체기한의 Timeout 결과는 유지하며 짧은 접촉 창에는
실패/Timeout을 넣지 않는다. 같은 카드가 뒤집힌 후 낮은 우선순위 들썩임으로 되돌아가지 않도록
동일 contactGroup의 적용된 motion priority를 Pattern ledger에서 보존한다. 같은 우선순위 반복은
유지하고 새 Pattern/Reset에서 ledger가 초기화된다. 세번째8_03의 실제 b_rpct_01 최저시각20191ms를
포함하고 Server 누적 tick의 최저시각20300ms도 포함하는20134~20484ms(350ms) 창에 기존 중앙/양옆 geometry를 복제한다. P12/P13/P14와 Bundle3을 PRODUCT로
연결하고 playAll 목록을 기존 authored 순서로 갱신한다. 최신 source bytes CAS를 거쳐 다른 저장을 보존한다.

수정은 기존 Workbench/Composition H·CPP, MainApp, PresentationPlayer/Level Preview, Brain,
GameRoom/LogicRuntime, projector 및 기존 native/Python/Server focused 검사다. 새 C++ 파일과
project/filter 등록은 없다. 최소 Product 컴파일, 해당domain publish, JSON/XML parse/diff check,
Apply/Save/Reload 실패보존과 retarget·contact·followup 검사를 실행한다. Client/UI 화면은 사용자가 확인한다.

Patterns 목록의 선택 Pattern에는 기존 Set_PatternAuthoringStatus를 호출하는 Set Pattern to PRODUCT
버튼을 둔다. 검증 실패는 기존 draft를 유지하고 성공은 저장 전 상태만 변경한다. 기존 Save가
CAS 저장과 ready PRODUCT publish를 수행하며 실제 Server 적용에는 재시작이 필요함을 표시한다.
G21의 데이터 연결은 최신 사용자 저장본을 보존한 revision149 기준이고, 검증 결과는 RESULT G21에 둔다.

## G22. 세이튼 룰렛 PRODUCT와 서버 보행면 (2026-09-08)

구현 시작 때 P7 세이튼_룰렛은 DRAFT였고 world.sequence.instance.8에는 walkableSurface(radiusM2.5,
localHeightM0.0263129994)가 저장돼 있다. 기존 투영은 반경10m의 정적 보행면과WORLD 창을 만든다.
P7을 PRODUCT로 활성화하고 같은 Server navigation 표면으로 플레이어와 정지한 Kouku 보스의
Y를 함께 샘플한다. CServerNavigation의 현재 support 목록 읽기 accessor를 Refresh의 실제 소비자로
연결해 표면 생성/제거 시 해당 원판에 있는 보스만 보정한다. scripted BossMotion·사망·다른위치보스는
유지하며 제거/Stop 뒤에는 기존 지면을 다시 샘플한다. 카드 instance에는 보행면을 추가하지 않는다.
기존 support-surface contract에 정지보스 상승·종료·Stop·대상밖높이보존을 추가하고 publisher와
Server 검사를 실행한다. 필요없는 navigation bake나 Map binary 변경은 없다.
