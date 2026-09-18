# 쿠크 World 리소스 검색과 MAP 묶음 위치 편집 결과

## G01. 확인한 원인과 실제 변경

통합 계획은 `2026-09-18_KOUKU_RAID_COLLIDER_SOUND_INTEGRATION_IMPLEMENTATION_PLAN.md`를 따른다. 기존 사운드 후보와 authoring JSON은 이 작업에서 변경하지 않았다. 현재 데이터의 `kakulsaydon.g1.world.37`은 `원본_갈고리`, `.world.38`은 `바닥_즉사칼날`이며 각각 원본 15개와 8개 emission을 소유한 저장 모션을 가리킨다.

`Render_WorldResources`의 상단 Object 목록은 기본 모션만 표시했다. 두 항목은 같은 Object의 비기본 모션이므로 그 목록에 나타나지 않았다. 기존 하단 Composition World 목록은 검색 대상도 아니었다. 이제 정확한 저장 World 목록을 위에 표시하고 같은 검색 입력으로 표시명, World ID, instance ID를 찾는다. 선택한 World stable ID를 그대로 `Append_WorldBox`에 넘기며 기본 모션으로 치환하지 않는다. 추가 성공 후 같은 박스의 미리보기를 요청한다.

## G02. 기존 MAP 위치와 그룹 전체 변환

`KOUKU_WORLD_SEQUENCE_RESOURCE::SavedPosition`은 저장 모션의 instance 원점이다. `CMainApp::RefreshWorldObjectResources`가 기존 saved document generation/revision 갱신 시 채우므로 매 프레임 파일을 읽지 않는다. 여러 모션의 그룹은 활성 member의 공통 원점을 사용한다. 현재 네 그룹 모두 공통 원점을 갖고 있으며 외곽 불 그룹은 `(0, 1.29999995, 942.080017)`, 두 서커스 그룹은 `(0, 0, 0)`이다. 원점이 다른 그룹은 기존 runtime의 단일 Placement로 상대 간격을 보존할 수 없으므로 지원 상태를 거짓으로 넓히지 않는다.

기존 Placement가 없는 WORLD/NONE 박스도 Box Detail의 `MAP group position`을 표시한다. 초기값은 source instance 원점과 World definition offset의 합이며, 표시만으로 저장 데이터를 바꾸지 않는다. 숫자를 바꾸면 기존 `Set_WorldBoxPlacement → Commit_Candidate → Queue_WorldBoxPreview → Refresh_WorldPlacementAuthoring` 경로를 사용한다. `Sample_ObjectWorld`는 같은 Placement를 모든 emission에 적용하고, 여러 모션인 그룹의 Level 미리보기는 각 member의 `Set_ObjectPlacement`를 갱신한다. 위치·회전·배율은 각 방출 오브젝트의 내부 배치·이동을 포함한 묶음에 적용된다.

`Queue_WorldBoxPreview`는 Placement 없는 선택 박스도 포함한다. Box Detail의 `Preview placements` 역시 Placement 유무로 숨기지 않는다. 기존 활성 박스는 현재 모션 시각을 유지하고 live transform만 변경한다. 다른 legacy 박스를 임의로 전부 활성화하지 않는다.

기존 map/deploy placement에 바인딩된 시퀀스는 Object Placement 지원으로 바꾸지 않았다. legacy BOSS_SPAWN 정의도 자동 MAP 변환하지 않는다. 원래 anchor를 유지하거나 기존 Place near character 명령으로 명시적으로 배치하는 계약을 보존했다.

## G03. 실행한 검증과 남은 확인

- `KoukuSaydonActionWorkbench.cpp`, `MainApp.cpp`를 변경한 최종 헤더와 함께 실제 MSVC Debug 설정으로 독립 컴파일했다. 두 TU 모두 성공했다. 기존 Camera_Free/SDK 인코딩 경고는 남아 있다.
- 원본 갈고리 15개와 즉사칼날 8개를 source fixture로 읽었다. production `Sample_ObjectWorld`, `Find_Track`, `Sample_Track`, `Clamp01` 본문을 바꾸지 않고 격리 컴파일하고 production WorldSequenceDocument codec을 연결했다. private 접근을 위한 헤더 복사본은 out에만 있다. 소스 함수 SHA는 `probe/sampler-source.json`에 기록했다.
- 23개 emission을 0/500/3000ms에서 비교했다. legacy 원점+offset을 Placement로 전환했을 때 전체 행렬을 보존하고, 공통 위치·yaw 90도·scale 1.25를 적용한 행렬이 기대한 공통 변환과 일치했다. 279 checks / 0 failures다. 처음 전체 runtime TU를 연결한 probe는 renderer/service 의존 unresolved symbol로 실패했으며, 검증 범위를 순수 sampler와 실제 codec으로 좁혀 최종 통과했다.
- 현재 JSON parse와 변경 파일 `git diff --check`가 통과했다. authoring JSON, Resources 설치, runtime publish는 이 작업에서 수행하지 않았다.

증거는 `out/KoukuWorldPicker20260918/data-evidence.json`, `compile/compile-exit.txt`, `compile/compile.log`, `probe/run.log`에 있다. 새 제품 소스 파일이나 project/filter 등록은 없다.

Client/UI를 실행하거나 조작하지 않았다. 새 빌드에서 Resources의 검색으로 `원본_갈고리`와 `즉사칼날`을 선택·추가한 뒤, Box Detail의 `MAP group position`/rotation을 조정했을 때 전체 묶음이 즉시 움직이는 실제 화면과 Save/Reload 결과는 사용자 확인 대상이다. 수치 검사만으로 GPU/화면 성공을 기록하지 않는다.

## World Box → Object Tool 미리보기 문맥 연결

World Box의 Edit This Motion, Edit Object, 연결 Motion 편집 요청은 박스의 MAP position/rotation/scale을 일시적인 preview context로 전달한다. Placement가 없는 기존 WORLD/NONE 박스는 캐시된 저장 Motion 원점과 World offset을 더한 동일 위치를 사용한다. MainApp → WorldObjectTool → Level의 Debug_BeginWorldObjectPreview → 기존 WorldSequencePlayer::Play의 OBJECT_PLACEMENT 경로로 소비하므로 Motion document를 이동시키거나 공유 데이터에 복사하지 않는다.

단일 Motion, 복합 그룹, 선택 Effect audition은 같은 context를 사용한다. 다른 Object 선택과 Workbench deactivation은 context를 비운다. 같은 그룹의 member 선택은 유지한다. Preview at Character는 일시적으로 context를 우회하고 해제하면 박스 위치로 돌아온다. toolbar의 Use saved Motion position은 context를 명시적으로 해제한다.

`out/KoukuWorldBoxContext20260918/compile/compile-exit.txt`의 Client 5 TU 컴파일은 exit 0이다. KoukuSaydonActionWorkbench, MainApp, MainApp_WorldLevel, WorldObjectTool, Level_KakulSaydonArena_WorldObjects를 확인했다. 변경 7파일의 원래 BOM/CRLF 보존과 git diff --check 통과, source hash는 `verification.json`에 기록했다. EngineSDK 기존 CP949 주석의 C4828 경고는 남아 있다. Data/리소스 설치와 Client/UI 실행은 하지 않았다. 최종 Product 링크와 사용자 화면 판정은 통합 작업에서 진행한다.
