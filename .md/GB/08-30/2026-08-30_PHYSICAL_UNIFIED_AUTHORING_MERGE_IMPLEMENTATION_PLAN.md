# 물리 폴더 통합 저작 환경 선택 병합 구현 계획서

## 목표

`C:\Users\user\Desktop\LostArk`를 하나의 실행·빌드 정본으로 유지하면서
`C:\Users\user\Desktop\CodexWorkTree\LostArk-captain-unified`의 통합 저작 기능만 선택 이식한다.
통합 worktree 전체를 병합하지 않고 원본 물리 폴더의 Effect V1 `171/171`, KakulSaydon·Sound·UI·Map
물리 리소스와 다른 세션의 최신 변경을 보존한다.

사용자 화면의 종료 상태는 다음과 같다.

- Debug Developer Tools에서 여러 Tool을 동시에 열고 같은 버튼을 다시 눌러 닫는다.
- Effect V1/V2, Boss, Camera, Map, UI, Workbench의 Complete Play는 Valtan Arena의 Server 권위 패턴 재생으로 수렴한다.
- Arena Active는 Tool마다 로컬 벽 상태를 따로 만들지 않고 공용 패널과 Workbench가 같은 Server 복제 상태를 표시한다.
- Workbench는 한 번의 `Save`로 내부 validate, authoring save, product projection, revision apply를 순서대로 실행한다.
- 발탄 도끼 충돌 sound cue는 Server combat-object hit ID에서 Client asset playback까지 한 계약으로 연결한다.
- KakulSaydon은 현재 추출 리소스 intake와 Development geometry preview까지만 admission하고, World/Nav/Boss/Pattern/Sound가 없는 Product Server Level은 차단한다.

## 시작 상태 실측

- 원본 브랜치: `codex/release-runtime-finalization`, 기준 HEAD `9174f17c`
- 통합 브랜치: `codex/captain-unified-authoring-kakul`, 기준 HEAD `f6111177`
- 통합 브랜치는 원본 HEAD의 후손이지만 원본 Effect 정리는 미커밋 물리 변경이어서 포함하지 않는다.
- 원본 Effect V1은 Catalog 171 / Authored 171 / 미등록 Authored 0이다.
- 통합 worktree의 Effect V1 298 / Authored 468과 추가 물리 Effect pack은 병합 금지 대상이다.
- 원본 `Client/Bin/Resources`의 `Fonts, Character, Deploy, Effect, Map, Sound, UI` 일곱 root가 물리 리소스 정본이며 통합 worktree 리소스는 그 부분집합이다.

## G00. 선택 병합 경계 고정

두 worktree의 공통 조상, 파일별 diff, 프로젝트 등록, Data·Resources closure를 먼저 비교한다.
파일 전체 복사 대신 다음 셋 중 하나로 분류한다.

1. 통합본과 정확히 같은 파일
2. 원본의 최신 변경과 통합 기능을 함께 보존하는 semantic union
3. Effect cleanup 또는 물리 리소스 회귀를 일으키므로 가져오지 않는 파일

종료 증거는 Effect `171/171`, `Client.vcxproj`의 Data/Effects 개별 `None` 0개, Kakul·Sound 물리 파일 보존이다.

## G01. 다중 Tool과 공용 Complete Play

`CMainApp`이 Tool별 visible bit와 명시적 viewport/input owner를 소유한다. Toolbar 버튼은 open/hide toggle이며
Tool 하나를 열 때 다른 Tool을 닫지 않는다. 각 도메인 Tool의 Complete Play 진입점은 `CBossTool::Play_ServerPattern`과
기존 typed audition service를 재사용한다. Tool은 socket을 직접 호출하거나 local Valtan AI를 제품 정답으로 만들지 않는다.

공용 Resource Files index는 현재 Developer Tools 안에서 Data와 Resources를 읽기 전용으로 나열하고 정확한 owner Tool을 연다.
독립된 넓은 하단 browser, waveform 편집, drag/drop asset picker는 이번 선택 병합의 구현 범위가 아니다.

## G02. Workbench와 Arena Server 상태

`CAnimation_Tool`의 Action Presentation Workbench에 다음 joined view를 결합한다.

- Server stage clock, hit/collider, hit schedule, push/knockdown 수치
- animation occurrence와 pattern blank duration
- Effect V1/V2, Sound, Camera/Shake, World event, Combat Object lane
- combat-object hit sound asset 선택과 저장
- 한 개의 사용자 `Save`

Arena mutation은 `VALTAN_ARENA_PRESET` typed command만 허용한다.

- Fresh / All Walls
- Phase 2 / Walls Gone
- Break 3 O'Clock
- Break 9 O'Clock
- Break 3 + 9

Active checkbox는 직접 수정하는 local flag가 아니라 ordinary walls/debris, outer ring, 3시 floor/collision/Nav,
9시 floor/collision/Nav의 Server actual read-only 상태다. debris actor 수, active collision 수, active nav region 수와
navigation revision도 함께 표시한다.

## G03. Sound, KakulSaydon, 8인 Raid 데이터

발탄 high-jump axe의 combat-object archetype/hit ID와 `G_Voltan2_Attack09_ProjExp1` 이벤트를 연결한다.
Shared protocol event, Server runtime emission, Client cue document/catalog/playback과 WAV 4개의 Git LFS 최소 dependency closure를 함께 검증한다.

KakulSaydon은 원본 이름의 Effect/UI/Character/Map 물리 roots와 8개 Data intake 문서를 Client DataFiles에 등록한다.
MapTool Development geometry preview는 허용하되 Product Server admission은 수직 슬라이스 완성 전까지 거부한다.

Valtan Arena player spawn slot과 Server admission 상한은 8로 확장하고 party 상한 4는 그대로 유지한다.

## G04. Solution과 검증 비용 정리

`Framework.sln` 기본 Build는 Engine, Shared, Server, Client만 유지한다.
EffectRenderContractHarness와 ValtanFourPlayerHarness는 solution과 표준 runner에서 제거한다.
남겨 둔 EffectRender source는 제품 CPP를 재컴파일하지 않는 좁은 WARP probe만 보존한다.

Core에는 다음 빠른 gate를 직접 연결한다.

- Action Presentation Workbench joined-domain contract
- KakulSaydon resource intake/admission contract
- Valtan 8-player capacity contract
- Effect Tool V2 document contract
- Team LAN endpoint contract
- Effect source/resource closure
- protocol v46 harness
- Character Select private/shared world isolation live harness

검증 명령은 다음 두 개를 정본으로 사용한다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Core -AllowLocalEffectResources
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Release -Profile Core -AllowLocalEffectResources
```

`-AllowLocalEffectResources`는 Git에서 제외한 물리 DDS pack을 사용하는 현재 저장소 계약을 명시한다.
이 옵션 없이 Core가 untracked Effect dependency를 거부하는 것은 누락이 아니라 strict Git-only 재현 gate다.

## G05. 물리 정본 Git 승격과 PR 슬라이스 결합

현재 물리 폴더의 미커밋 상태를 잃지 않도록 원본에서 stash, reset, checkout을 실행하지 않는다.
최신 `origin/main`에서 별도 통합 worktree와 `codex/physical-authoritative-unification` 브랜치를 만들고,
원본 물리 폴더의 tracked diff와 검토한 untracked source/Data/document를 그 위에 재현한다.

커밋은 다음 검증 단위로 나눈다.

1. Effect V1 `171/171`과 도달 불가능한 레거시 corpus 삭제
2. 다중 Tool, Workbench, Complete Play와 project/solution 계약
3. Arena/Debris/Nav, combat-object sound, 8인 Server 계약
4. KakulSaydon intake와 Development preview admission
5. 저장소 규칙, build profile, 문서 이동과 PLAN/RESULT

열린 PR #265의 Camera 파일은 현재 Complete Play 통합과 직접 겹치므로 통째 merge하지 않는다.
PR의 앞 세 커밋에서 Cut 생성/삭제, Capture Pos/Pos List, 빈 Cut 계약만 현재 Camera Tool에 semantic union한다.
Bern entrance camera와 ESC skip은 Level/Data/project 등록을 포함한 별도 수직 슬라이스로 결합한다.
PR의 기존 RESULT는 그대로 복사하지 않고 실제 통합 빌드와 수동 검증 경계에 맞게 교정한다.

새 저장소나 orphan history는 만들지 않는다. 기존 main 계보, PR ancestry, blame, LFS 추적 파일을 보존하고
Git 제외 `Client/Bin/Resources` 물리 pack은 현재 폴더의 별도 runtime dependency로 유지하되, 이번 Sound binding이 새로 요구하는 WAV 4개만 exact feature closure로 Git LFS 추적한다.

## 완료 경계

- 자동 완료: 선택 병합, project/filter 등록, Effect 171/171, Debug/Release Product와 Core, protocol/isolation harness
- 사용자 수동 검증: Debug Server+Client 진입, 다중 Tool toggle, Complete Play, Arena preset/Active 시각 결과와 axe sound
- 후속 수직 슬라이스: Kakul Product World/Nav/Boss/Pattern/Sound, 독립형 넓은 Resource Files browser와 waveform 저작
- 알려진 FullDiagnostic 차단: Ghost Valtan model 2개와 DDS 6개가 물리 폴더에 없음
- Git 완료: 최신 main 기반 통합 브랜치 push, 변경 슬라이스별 커밋, main 대상 통합 PR 생성
