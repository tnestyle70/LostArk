# 캡틴 통합 저작 환경·쿠크세이튼 수직 슬라이스 구현 계획

## 1. 목표

`c75e3623`의 Action Presentation Workbench를 기준점으로 삼아 다음 계약을 한 변경 단위로 닫는다.

1. Debug Developer Tools는 한 도구를 열 때 다른 도구를 닫지 않는다.
2. 하나의 `Resource Files` 트리가 `Resources`와 Git 관리 `Data` 정본을 도메인별로 보여 주고,
   선택한 항목을 기존 Effect V1/V2, UI, Map, Boss, Camera, Animation/Presentation 화면으로 보낸다.
3. Valtan 패턴 재생은 모든 도구에서 같은 `CValtanPatternAuditionService`를 사용한다. 로컬 Model
   Preview는 저작 보조일 뿐 `Complete Play` 성공으로 표시하지 않는다.
4. Valtan 경기장 벽·파편·collision·navigation은 Client visibility 토글이 아니라 기존 Server
   destruction transaction과 arena preset으로만 바꾼다.
5. KakulSaydon 추출물은 사용자 표시 컬렉션명 `KakulSaydon`으로 탐색하되, 이미 추출된 package 및
   map source identity `LV_LUT_MIDNIGHTC_ED`를 stable asset/Area ID로 보존한다.
6. Kakul은 Resource intake만으로 Product boss처럼 위장하지 않는다. World, Server admission,
   navigation, encounter/pattern, Client Level descriptor가 검증된 범위까지 단계적으로 연다.
7. 팀 LAN 정본을 사용자 지정 `192.168.0.14:7777`로 한 번에 갱신한다.
8. Debug/Release Product와 관련 Core/contract를 빌드해 시간과 실패 원인을 RESULT에 기록한다.

## 2. 현재 기준과 병합 경계

- 구현 worktree: `C:/Users/user/Desktop/CodexWorkTree/LostArk-captain-unified`
- branch: `codex/captain-unified-authoring-kakul`
- base: `c75e3623 feat: integrate action presentation workbench`
- 원본 `C:/Users/user/Desktop/LostArk`는 대규모 미커밋 삭제·수정과 Kakul 물리 리소스가 섞여 있다.
  검증 전에는 원본을 자동 stage, reset, overwrite하지 않는다.
- `LostArk-pipeline-cleanup`과 `lostark-structure-cleanup-0829`는 같은 파일을 서로 다른 정책으로
  대량 삭제한 WIP다. 둘 중 하나를 통째로 합치지 않고, 현재 변경에 필요한 작은 계약만 실측해
  이식한다.

## 3. G별 구현

### G1. 비독점 Debug tool workspace

- `DEBUG_TOOL` 하나만 기억하는 상태를 도구별 visibility 상태로 교체한다.
- `EnsureDebugTool`은 생성·focus·visible만 담당하고 Map/Camera/다른 도구를 닫지 않는다.
- MainApp은 열린 도구를 모두 render하고, Animation/Boss/Camera update의 visibility 인수도 각 도구
  상태를 사용한다.
- Developer Tools에는 각 도구의 Open/Close 체크와 `Close All`을 둔다.

### G2. Resource Files orchestration tree

- MainApp이 시작/수동 Refresh 시 한 번만 manifest를 만든다. 매 프레임 recursive scan하지 않는다.
- 물리 리소스 root와 저작 Data root를 도메인별로 묶는다.
  - Character/Animation
  - Boss/Pattern
  - Effect V1
  - Effect V2
  - Sound
  - Map/World/Navigation
  - UI
  - Camera/Rendering
- 항목은 workspace-relative stable path를 저장하고 절대 경로나 `..`를 저작 JSON에 쓰지 않는다.
- 선택은 해당 기존 tool을 열고 가능한 경우 typed deep-link를 전달한다. 아직 exact-selection API가
  없는 도메인은 경로와 이유를 표시하고 tool root를 연다.

### G3. Complete Play

- 공용 control은 stable encounter/pattern ID만 받는다.
- Valtan은 `CValtanPatternAuditionService::Submit`을 사용하며 다음 조건을 모두 검사한다.
  - active Level이 Server-approved Valtan Arena
  - network connected
  - replicated boss placement 존재
  - current admitted pattern inventory에 ID 존재
- Effect V1/V2, Boss, Workbench와 Resource Files의 버튼 문구 및 상태를 같은 `Complete Play
  (Server/Arena)` 의미로 맞춘다.
- Map/UI에서 패턴 resource를 선택한 경우에도 동일 orchestration control을 보여 주되, 로컬 clip이나
  UI preview를 Server 실행으로 가장하지 않는다.

### G4. Valtan arena state와 편집 가능한 계약

- 기존 Fresh, Circle/Walls Gone, 3시, 9시, 3+9 preset의 Server transaction을 보존한다.
- Active checkbox는 replicated destruction state를 읽는 사실 표시로만 둔다. 독립 checkbox를 임의의
  부분 mutation으로 바꾸지 않고, 변경 명령은 Fresh, Walls Gone, 3시, 9시, 3+9의 exact Server
  preset으로만 제출한다. 벽 mesh만 숨기지 않고 collision, debris, nav generation이 동일 revision으로
  적용됐는지 상태를 표시한다.
- Workbench의 pattern/stage lane은 duration과 combat-object hit/sound coverage를 계속 편집·저장한다.
- knockback, airborne, direction/speed, collider는 실제 Server schema에 존재하는 typed field만 노출한다.
  의미가 확인되지 않은 임의 JSON 숫자는 만들지 않는다.

### G5. KakulSaydon admission

- intake receipt와 실제 물리 파일을 다시 집계한다.
- 물리 탐색 표시는 `KakulSaydon` 컬렉션으로 통일하되 WModel 내부 상대 의존성과 package root를
  깨뜨리는 이동은 하지 않는다. 필요하면 collection manifest가 원본 stable path를 가리킨다.
- playable sound mapping이 없는 `.loa` metadata는 Sound asset으로 가장하지 않고 `unresolved`로
  표시한다. event→WEM/PCK 근거가 확인된 항목만 `Resources/Sound/KakulSaydon`에 둔다.
- 최소 Area는 `LV_LUT_MIDNIGHTC_ED` map source와 placement parser 검증을 통과해야 한다.
- 신규 Product Level을 추가한다면 `LEVEL`, `WORLD_ID`, registry, loader, world bootstrap, Server room,
  Client replication/entry를 같은 변경에서 추가한다. 이 closure가 닫히지 않으면 Debug resource/map
  audition으로만 표기하고 Server boss play를 비활성화한다.

### G6. 팀 endpoint

- `Tools/Network/TeamLanEndpoint.json`
- Client code/debugger default
- `AGENTS.md`, `CLAUDE.md`, `.md/TEAM/README.md`, handbook, network guide
- endpoint contract tests

위 항목을 `192.168.0.14:7777`로 함께 바꾸고 `Sync-TeamLanEndpoint.ps1`로 현재 PC 역할과 listener
상태를 확인한다. 격리 harness의 `127.0.0.1`은 유지한다.

### G7. 빌드·회귀와 물리 폴더 반영

검증 순서:

1. JSON/XML parse, project/filter registration, `git diff --check`
2. 변경 domain Python contract와 publisher validation
3. `Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product`
4. 관련 Core/FullDiagnostic 중 dependency closure가 있는 항목
5. `Invoke-BuildAndRegression.ps1 -Configuration Release -Profile Product`

Client/UI는 실행하지 않는다. 사용자가 Server와 Debug/Release Client를 직접 실행할 정확한 순서를
RESULT에 적는다. 전용 branch가 clean commit과 자동 검증을 확보하기 전에는 원본 물리 폴더에 코드
병합을 강행하지 않는다.

## 4. 완료 판정

- 자동 검증 PASS와 수동 검증 대기를 분리한다.
- Kakul playable sound, material, navigation, boss pattern 중 근거가 없는 부분을 완료로 기록하지 않는다.
- Resource tree에 보이는 항목, 실제 파일, Data binding, Product runtime consumer를 구분해 수량과 누락을
  RESULT에 남긴다.
- 원본 반영은 충돌 없는 commit 또는 명시된 patch/merge 경로로만 수행한다.
