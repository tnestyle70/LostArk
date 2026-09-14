# 외곽불 통합 미리보기 결과

현재 사용 방법은 아래 G04다. G02/G03의 간이 편집 안내는 폐기하고 기존 Object Detail 전체 편집기를 사용한다.

## G02. 현재 반영 상태 — Object Resources 통합 그룹

G01의 독립 Sequencer Benchmark 방식은 사용자가 원하는 위치가 아니므로 폐기했다. 현재 진입점은 **World Object Tool → Object Resources → Map → 외곽불_전체 [Group]**다. 기존 G01 패턴과 이번 작업에서 추가했던 WORLD 6행만 제거했고 다른 Sequence 내용은 보존했다. Sequence revision은 6→7이다.

- 모델 없는 Object Resource `world.object.group.kouku.g3.outer_fire.all`에 기존 D/E/F × CW/CCW 모션 6개의 stable instance ID를 참조시켰다. 원래 모션은 복제하거나 삭제하지 않았다.
- WORLD authoring revision 1699→1700. 기존 6모션의 위치, 크기, 반경, 속도, 방향, emission은 모두 그대로다. 6모션 × 각 10개 = 60개, 각 36000ms 설정이다.
- 그룹 부모를 선택하면 같은 Object Sequencer에서 Play/Pause/Stop/Restore/seek를 사용한다. Object Detail에는 각 모션의 Enabled, Map Position, Orbit Radius가 표시된다. 그룹 선택 시 Preview at Character 기본값은 꺼진다.
- 반경 조절은 revolutionOffset XZ와 각 authored emission XZ를 같은 비율로 변경한다. Y, 중심, 위상은 보존한다. Save는 기존 authoring 저장 및 publisher 경로를 이용한다.
- 기존 Object preview player에 복수 instance를 stage하고 한 시계로 sample한다. 새 별도 렌더링 경로나 Benchmark 시계를 사용하지 않는다. 준비 실패 시 기존 preview를 유지하고, 모든 모션을 끄면 기존 preview도 정지한다.
- C++ 문서 codec과 PowerShell publisher에 모델 없는 그룹 및 참조 검증을 함께 추가했다. 기존 C++ 파일 6개를 수정했고 새 C++ 파일이나 프로젝트/filter 등록은 없다. 팀 Area guide와 PLAN에 현재 계약 및 전체 수정 코드를 반영했다.

## 자동 검증

- WorldSequences 공식 Validate / Publish / Check 통과. runtime과 authoring JSON이 동일하며 revision 1700이다.
- 기존 member 데이터 전체 불변, 추가 그룹 외 WORLD semantic 불변, 잘못 추가한 Benchmark 항목 제거, 파일별 BOM/인코딩/줄바꿈 보존 검사 통과.
- 공전 중심 수치 검사: emission과 yaw 회전된 revolutionOffset의 최대 잔차 8.114737735862443e-7m. 반경 비례 변경 후에도 중심 보존 검사 통과.
- PythonUTF8=1 환경의 집중 unittest 5개 통과: 유효/무효 그룹 publisher, transactional document, invalid references/enums/scales, loaded selection coherence, WorldSequences-only publish rollback. 누락/중복/빈 member, 모델을 가진 그룹, default motion, LOOP/PLAYER member, 중첩 그룹 binding 거부 및 빈 materialBindings 배열 허용을 포함한다.
- 독립 비평에서 발견한 전체 member 비활성화 정지 누락과 빈 materialBindings 배열의 C++/publisher 불일치를 수정했다. 해당 검증을 다시 통과했다.
- Client x64 Debug C++ 컴파일 성공(오류 0). 별도 `_BuildLinkAction;DeployClientRuntimeDependencies` 성공(exit 0), 실제 `Client/Bin/Debug/Client.exe` 갱신 확인(2026-09-12 22:15:27, 54808576 bytes). 기존 인코딩 경고와 DirectXTK PDB 누락 링크 경고는 남아 있다.
- 일반 Build는 변경하지 않은 대형 FXC 컴파일이 오래 걸려 이번 작업이 시작한 MSBuild 프로세스 트리만 확인 후 중단했다. 직접 Link target은 입력 구성 없이 생략되어 실제 링크 검증으로 세지 않았고, 위 `_BuildLinkAction`으로 다시 실제 EXE를 생성했다. 따라서 전체 Debug/Release 정본 regression 완료로 보고하지 않는다.
- 관련 파일의 git diff --check 통과. 대규모 dirty worktree를 자동 stage/commit/push하지 않았다.

## 사용자 확인과 남은 경계

Client를 사용자가 다시 실행한 뒤 World Object Tool → Object Resources → Map에서 **외곽불_전체 [Group] 부모 이름**을 선택하고 같은 툴의 Play를 누른다. 자식 이름을 선택하면 기존 단일 모션 편집으로 들어간다. 부모 선택 상태의 Object Detail에서 6모션의 Orbit Radius를 각각 조절하고 Save한다. Map Position은 기존 아레나 중심 값을 유지했다.

Client/Server는 에이전트가 실행·종료하거나 UI 조작·캡처하지 않았다. 최종 확인 시 두 프로세스 모두 실행 중이지 않았다. 실제 60개 동시 표시, 개별 반경 조절의 육안 결과, UI Save/Reload 왕복과 실패 시 화면 보존은 사용자 수동 확인이 남아 있다. native parser 실행형 그룹 하네스와 Release/전체 regression은 수행하지 않았다. 화면 PASS로 기록하지 않는다.

G01 변경 전 Sequence 백업과 G02의 변경 전 JSON/소스 백업은 `C:/Users/USER/.codex/worktrees/7395/LostArk/.codex_tmp/` 및 `outer_fire_group/` 아래 보관했다. 기존 원본 6모션은 그대로이므로 원래 개별 편집도 계속 가능하다.

## G03. 단일 항목과 Object Sequencer 여섯 행 편집

사용자 확인: Object Resources에서 `외곽불_전체` 하나를 클릭하면 Object Sequencer에 여섯 개가 동시에 펼쳐지고 함께 재생/개별 수정하는 형태를 요청했으며, 확인 질문에 `그러치`로 승인했다. G02의 `[Group]` 폴더와 그 아래 개별 선택 형태는 이번 UI에서 제거했다.

- 통합 항목은 자식 없는 Selectable 한 줄이며 `외곽불_전체` 이름만 표시한다. 클릭 시 Object Sequencer를 열고 기존 여섯 member를 공통 0ms 정지 상태로 sample한다.
- 여섯 행의 이름, 공통 시간막대, Enabled, Map Position, Orbit Radius를 Object Sequencer 안에 접힘 없이 표시한다. Play/Pause/Stop과 시계는 하나이며 각 행의 시간막대를 드래그해도 여섯 행이 함께 seek한다. Object Detail은 통합 이름 편집과 사용 안내만 표시한다.
- 기존 개별 D/E/F Object와 모션은 다른 기존 목록에 보존했다. 데이터 구조/저장 경로/Level/player/Shader는 이번 G에서 수정하지 않았다. authored/runtime worldsequences bytes가 시작 시점과 같음을 확인했다.
- C++ 변경은 `Client/Private/WorldObjectTool.cpp` 한 파일. H/프로젝트/filter 신규 등록 없음. G03 PLAN에 전체 코드와 팀 Area 가이드의 사용 경로를 갱신했다.
- 독립 비평에서 선택→Seek(0), dirty→전체 preview 재구성, lifetime 및 ImGui ID/table 짝을 검토했으며 새 결함은 발견되지 않았다. 화면 실행 증거가 아닌 읽기 검토다.
- JSON/XML parse와 scoped git diff --check 통과. 기존 focused unittest 2개(그룹 resource publisher 정상/부정 입력, loaded selection coherence) 통과, 8.393초. 실제 통합 화면의 Save/Reload 왕복은 사용자 확인이 남아 있다.

현재 사용 순서: Client 재실행 → World Object Tool → Object Resources → Map → **외곽불_전체** 클릭 → Object Sequencer의 여섯 행에서 위치/반경 조절 → Play → Save. 창이 작으면 Object Sequencer를 늘리거나 스크롤해 아래 행을 확인한다. 기존처럼 그룹 자식을 선택하는 단계는 없다.

사용자의 Client 종료 확인 뒤 빌드했다. 에이전트는 Client/UI를 실행·조작·캡처하지 않았고 화면 PASS로 판정하지 않는다. 자동 stage/commit/push도 하지 않았다.

- G03 빌드 검증: Client x64 Debug `ClCompile;_BuildLinkAction;DeployClientRuntimeDependencies` exit 0. EXE 갱신 2026-09-12 22:59:57, 54810112 bytes. 셰이더 target은 실행하지 않았다. 기존 인코딩 및 DirectXTK PDB 경고는 남아 있으며 전체 Debug/Release regression과 사용자 화면 검증은 이번 결과에 포함하지 않는다.

## G04. 기존 전체 Motion 편집기 복구와 여섯 행 통합 타임라인

사용자가 첨부한 23:36:44 Object Detail의 기존 Physics / Motion / Emission, Authored Emissions, Selected Key 화면을 그대로 사용하도록 교정했다. G03의 간이 위치·반경 편집은 제거했다. 기존 데이터와 UI 입력을 별도로 복제하지 않고 원래 Render_Detail/Render_KeyEditor를 직접 연결했다.

- `m_SelectedGroup`은 전체 재생 scope, 기존 Object/Instance 선택은 편집 member로 분리했다. 통합 항목 클릭은 첫 member를 선택하고 전체 0ms preview를 연다. 이후 행/키 또는 Editing Motion을 바꾸어도 Stop, clock reset, solo 전환을 하지 않는다.
- Object Sequencer에는 기존 DrawRuler/DrawBox 기반 공통 시간축과 모든 member의 Transform 키/animation/effect 행이 표시된다. 내부 키의 시간 드래그와 선택은 오른쪽 기존 Key Editor와 연결된다. 딜레이와 재생 속도를 고려해 로컬 키 시간과 전체 시각을 변환한다.
- Object Detail에는 통합 이름/Editing Motion 다음 기존 Motion 전체 설정, Physics, Emissions, Effect Rows, Selected Key, Animation Clips가 이어진다. 기본 첫 member가 선택되므로 빈 안내문만 표시하지 않는다. 일반 Object를 선택하면 원래 단독 편집으로 나간다.
- 기존 그룹 WORLD/STOP 계약은 유지하며 group에서 On Complete는 Stop으로 제한한다. 각 member의 Lifetime/Playback Speed/Start Delay와 다른 기존 편집 항목은 유지한다. Save/readback/CAS/publisher, Append Effect/Clip도 기존 member 경로를 사용한다.
- 소스 수정은 WorldObjectTool.h/.cpp와 기존 source integration test 파일이다. PLAN에는 현재 전체 코드, 팀 Area guide에는 현재 사용 경로를 반영했다. 신규 C++/프로젝트/filter/Shader/Level/player/Server 변경 없음.

검증: source integration guard와 기존 그룹 publisher/selection 테스트 총 3개 통과(31.623초). 그룹 선택에서 Stop/clock reset이 없는지, 전체 편집기와 키/animation/effect 행이 연결되는지 검사했다. 실제 UI 실행 테스트가 아니라 source 구조 검사임을 구분한다. 독립 비평에서 append/save/reload/sidebar 전환, disabled/lifetime 및 좌표 시간 변환을 검토했고 새 결함은 발견되지 않았다. authoring/runtime JSON bytes가 이번 G 시작값과 같고 JSON/XML parse, scoped git diff --check를 통과했다.

사용 방법: 실제 프로젝트 Debug Client 실행 → World Object Tool → Object Resources → Map → **외곽불_전체** → Object Sequencer에서 편집할 행/키 선택 → **Object Detail의 기존 전체 편집 항목**에서 수정 → 전체 Play/seek → Save. Editing Motion으로도 편집 대상을 바꿀 수 있다. 선택 변경 중 전체 표시와 재생 시각 유지, 개수/개별 offset/키 편집 결과, Save/Reload 왕복은 사용자 화면 확인이 남아 있다.

Client 종료는 사용자가 확인했으며 에이전트는 Client/UI를 실행·조작·캡처하지 않았다. 기존 사용자 저장값을 임의 변경하지 않았고 자동 stage/commit/push하지 않았다. 전체 Debug/Release regression과 화면 PASS는 이번 검증에 포함하지 않는다.

- G04 Client Debug 컴파일·링크: `ClCompile;_BuildLinkAction;DeployClientRuntimeDependencies` exit 0. EXE 2026-09-12 23:47:20, 54817280 bytes. FXC target 미실행. 기존 인코딩/DirectXTK PDB 경고는 유지된다.
