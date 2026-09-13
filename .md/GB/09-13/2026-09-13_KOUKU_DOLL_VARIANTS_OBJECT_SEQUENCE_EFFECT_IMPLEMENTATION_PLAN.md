# 괴기스러운 인형 크기 구분과 Object·Sequence Effect 동시 재생

## G00. 현재 호출자와 변경 범위

기존 `world.object.kouku.odd_doll`은 `MN_CDMD_00.wmodel`을 `modelPreScale=0.01`로 읽는다.
Object의 Effect 목록은 V2 GROUP/LEAF만 제공하며 새 행은 MOTION_END로 생성된다.
Sequence는 이미 Kouku Workbench의 V1/V2 Effect 목록과 presentation preview를 소비한다.
따라서 기존 Object Motion과 World Sequence 재생기를 확장하고 Sequence의 같은 시작 시점 연결을 보완한다.

현재 브랜치 `codex/kouku-donut-ball-motion`에는 여러 작업의 미커밋 변경이 있다.
이번 기능에 필요한 부분만 추가하며 자동 stage/commit하지 않는다.
실행 중 Client의 미저장 편집 여부를 확인하고 데이터 후보를 최신 저장본에 병합한다.

## G01. 작은 인형과 큰 인형

Area authoring `Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json`이
두 Object 정의와 각 Motion instance를 소유한다. 기존 stable ID와 기존 배치는 보존하고
표시명을 `괴기스러운 인형 - 작은 사이즈`로 바꾸며 큰 인형에 새 stable ID를 부여한다.
쿠크 NPC 480641/480742의 ModelSize 160과 480746/480747의 100을 기준으로 큰 크기를 1.6배로 둔다.
모델 geometry, preScale, materialSource와 object 배율을 구분하고 다른 객체에 배율을 전파하지 않는다.

## G02. Object의 Effect 선택·재생·추가

`WorldObjectTool.h/.cpp`는 기존 metadata inventory로 V1 Effect와 V2 Effect를 함께 보여준다.
선택된 Motion과 Effect에서 검증된 후보 문서를 만드는 함수를 Play와 Append가 함께 사용한다.
새 Effect는 `TIME`, `startMs=0`으로 모델과 함께 시작하며 기존 MOTION_END 행은 유지한다.
Play는 후보만 재생하고 Append는 검증 후 메모리 문서에 commit한다. 저장 전까지 원본 JSON은 바뀌지 않는다.
Pause/Seek/Stop은 같은 WorldSequencePlayer의 시계를 사용하고 실패하면 기존 편집과 재생을 보존한다.

`WorldSequenceDocument`와 해당 publisher는 `V1_EFFECT` 종류와 명시적 object follow/bone 계약을
함께 읽고 검증·저장한다. `WorldSequencePlayer`는 기존 Effect presentation service로 준비·생성·seek·정리한다.
실제 Object의 CModel과 pose를 본 입력으로 제공하며 다른 보스나 synthetic anchor로 대체하지 않는다.

## G03. Sequence에서 같은 모델과 Effect 재생

`KoukuSaydonActionWorkbench`의 기존 Effect 선택과 Append·Play 경로를 사용한다.
저장된 Object Motion을 WORLD 행에 붙이면 그 Motion의 모델·Effect를 같은 시계로 소비한다.
독립 Effect 추가는 선택 대상과 시작 시간을 명확히 하며 모델 종료 뒤로만 붙는 흐름을 만들지 않는다.
실제 consumer가 있는 기존 public 계약만 확장하고 중복 runtime을 추가하지 않는다.

## G04. 검증과 실행 경계

변경 JSON과 project XML parse, 기존 WorldSequence publisher의 저장·검증 및 관련 focused test,
변경 C++의 정상 증분 Debug Product Build와 `git diff --check`를 실행한다.
기존 H/CPP를 확장하므로 새 C++ 파일 및 project/filter 등록은 필요하지 않다.
실행 중 파일 점유와 미저장 draft는 먼저 보존하며 Client/UI를 에이전트가 실행하거나 캡처하지 않는다.
사용자가 수행할 작은/큰 크기, 불꽃 위치·방향, Play/Append/Pause/Seek/Save 재생 확인은 RESULT에서 구분한다.

## G05. 추가 요청: 포탈 World 배치와 화면 수축·큐브

독립 Effect Play All은 기존 player-at-play 앵커 외에 명시한 고정 World 위치와 yaw를 소비한다.
`Use Player Pos`는 현재 위치·방향을 고정하고 `Use Mouse Pos`는 기존 world picking의 한 번 클릭을 받는다.
`MainApp`의 gameplay 입력 처리 전에 클릭을 소비하며 실패·취소는 이전 위치를 보존한다.
Effect preview와 Append는 같은 session 배치값을 사용한다.

현재 장면 수축은 native screen-post 후처리 단계에서 검은 배경과 캡처 텍스처를 불투명 합성하므로
그 전에 그린 포탈까지 가린다. 기존 screen-post material interface에 scene replacement 여부를
추가하고 이 material만 forward blend 전에 합성한다. 기존 일반 후처리와 UI 순서는 유지한다.
장면 캡처 해상도를 낮추지 않고 표시 영역을 줄이며, 수축 중심은 occurrence world root의 화면 투영이다.
같은 SceneHDR·Bloom 캡처를 쓰는 차원술사 Alt+V의 camera mesh와 animated cube 경로는
기존 ALT178 material·camera-fit·30Hz 데이터를 기준으로 실제 연결을 재검증한다.
검은 바깥 화면 유지는 1관문 도입 포탈 수축에만 적용한다. 차원술사 Alt+V는 기존 배경과
연출을 유지하고 캡처 화면의 큐브 정렬만 수정한다. Alt+V에 검은 배경이나 scene replacement를 추가하지 않는다.

조사 결과 camera mesh와 중앙 animated cube의 종착 basis가 다르므로 실제 cue 첫 pose와
설치 geometry bounds를 기존 renderer의 framing 소비자에 연결한다. animated CModel의 기존
Has_LocalBounds는 NONANIM 전용이므로 이를 재사용했다고 가정하지 않는다. 기존 WModel
decode에서 bind geometry bounds를 읽기 전용 metadata로 보관하고 clone/reset까지 유지한다.
기존 culling bounds의 뜻은 바꾸지 않으며 현재 Alt+V 소비자가 새 조회를 직접 사용한다.

새 수축 Effect와 Sequence 연결은 최신 원본에서 후보를 만들고 기존 미저장 편집과 파일 freshness를
확인한 뒤 적용한다. 이전 후보 전체 Composition으로 최신 사용자 문서를 덮지 않는다.

## G06. 추가 요청: 커튼 화면 연출

월드 커튼 alias는 맵 11개 placement 이동이고 화면 커튼은 V2 ScreenPost Leaf의 별도 companion이다.
화면 커튼은 Presentation Manager의 ScreenOverlay를 통해 screen UV로 그리므로 UI 오브젝트를
새로 만들거나 월드 mesh를 직교 투영으로 바꾸지 않는다. Sequence Append에서 기존 companion 연결을
보존하고 Object Preview도 같은 saved resource association을 소비한다. Leaf 목록을 숨기지 않는다.
댄스 타임의 현재 3500ms 이동(1050ms 내려옴, 2450ms부터 올라감)과 실제 표시 여부를 구분한다.

## G07. 추가 요청: 관문 기본 상태와 외곽불

1관문 세이튼은 책이 펼쳐진 별도 공간을 기본 전투 위치로 사용한다. 현재 1·3관문이 같은
945m 부근으로 이동하는 Debug gate와 Server boss placement를 실제 펼친 바닥·navigation 근거로
분리한다. 이동·보스 생성은 기존 typed Server command/approval을 유지하며 Client Transform으로
우회하지 않는다. 펼친 책과 무대의 끝 pose는 기존 WorldSequencePlayer를 사용해 유지한다.
Sequence가 책 연출을 소유할 때 기본 펼침 상태를 잠시 양도하고 Stop/완료 뒤 복귀한다.
보스 중심 이동 뒤에는 BOSS_SPAWN을 따르는 행과 절대 World 좌표를 구분해 대조한다.
Gaze REAL_GAZE_TELEPORT는 기존 중심에 대한 상대 위치를 유지하도록 같은 -204.8m를 적용하고,
NONE anchor의 커튼 group offset도 실제 MAP 기준 위치와 연결 Pattern을 확인해 함께 옮긴다.
이미 BOSS_SPAWN을 소비하는 룰렛 anchor는 추가 이동하지 않는다. 기존 WORLD/Effect 행을
추가·삭제하거나 다른 관문의 Pattern을 일괄 이동하지 않는다. 실제 Server clone 원형 배치와
navigation에 새 target/분신 위치가 모두 들어오는지 격리 CPU 검사 후 원본·runtime을 적용한다.

3관문 입장은 새 `world.object.group.kouku.g3.outer_fire`의 6개 Motion을 함께 시작한다.
기존 외곽불 Object는 보존하며 새 D_CCW와 E_CW만 E 두 줄 간격만큼 반경을 늘린다.
현재 저장본 E_CW/CCW의 pivot 반경은 둘 다 11.7m이므로 화면의 두 줄 간격을 임의 추측하지 않는다.
사용자의 저장 또는 거리 응답 전에는 간격에 의존하는 후보를 적용하지 않는다.
새 6개 Motion은 LOOP이며 관문 전환·레벨 종료가 재생 수명과 정리를 소유한다.

## G08. 추가 요청: 관문 조명과 카메라 점멸

관문별 조명은 현재 MapLight source를 보존한 활성화 문서와 기존 Scene Profile로 분리한다.
3관문은 warm 1관문 spotlight를 끄고 원본 cyan 3관문 spotlight를 켠다. 기본 directional,
ambient와 camera EnvironmentRegions의 재적용까지 함께 다뤄 카메라 이동이 어두운 설정을
다시 밝히지 않게 한다. MainApp은 관문 상태 전환 시에만 profile을 바꾸고 Sequence의 현재
scene profile을 매 프레임 덮지 않는다. profile/리소스 검증 실패는 기존 화면을 보존하고 보고한다.

기존 transient 64→384 수정 이후 실행 파일이 사용 중임을 확인했으므로 사용자의 점멸 관찰을
구 EXE로 단정하지 않는다. 실제 source provider·frustum·budget·scene environment 소비자를
대조하고 재현한 원인만 수정한다. 다른 재질·광원에 임의 밝기/회전을 전파하지 않는다.

## G09. 추가 요청: 책·커튼 재질

설치된 book 3개 slot의 기존 CModel→CMaterial source binding을 먼저 확인하고 이미 원본과
연결된 재질의 색을 임의 보정하지 않는다. 커튼 MAP 배치의 원본 BG01/BG01c 재질을 component별로
대조해 기존 donor와 정확한 native material 소비 경로로 복원한다. Effect shader ABI를 맵 재질에
직접 삽입하지 않는다. 모든 후보는 기존 미저장 문서·Product guard를 확인한 뒤 활성화한다.

## G10. Effect 목록의 한 줄 분류

캐릭터 `Skill | Input ...` 목록과 같은 Effect Resources / All Effects의 쿠크 표시 경로를 사용한다.
`Effect_Tool_ResourceBrowser.cpp::Render_SavedAuthoredEffectSection`의 중간 CATEGORY_NODE 트리와
Saved Effects root 접기를 없애고 각 Effect를 같은 들여쓰기의 한 줄로 나열한다.
표시는 `1관문 | 연출 | ... | 이름`, `1관문 | 패턴 | ... | 이름` 형식이며
1관문, 2관문, 3관문, 공통 순으로 정렬한다. 저장된 category metadata와 표시명을 우선한다.

개별 Effect 상세는 기존 캐릭터 Skill 행처럼 열어 Open Editor/Play All/Append Group을 사용한다.
stable asset ID는 항목 식별과 tooltip에 유지하며 같은 이름끼리 선택 상태가 섞이지 않게 한다.
검색은 표시 분류·이름·asset ID를 포함한다. 원본 Effect·category JSON과 재생·저장 동작은 바꾸지 않는다.
원본 파일 인코딩, 현재 미커밋 변경과 pending document load의 편집 보존을 유지한다.
최소 TU 컴파일과 실제 inventory의 정렬/표시 대조 후 다음 제품 빌드에 포함한다.

## G11. 포탈을 시퀀스 폭죽 박스 위치로 이동

사용자가 기준점을 `시퀀스 폭죽 박스의 저장 위치`로 확정했다. `KoukuSaydonSequenceComposition.json`의 `KAKULSAYDON_G1_PATTERN_4.presentation.30`에 저장된 `[59.75699996948242, 0.49000000953674316, -94.59600067138672]m`를 읽어 포탈 `.presentation.32`의 `positionOffset`에 복사한다. 변경 직전 최신 문서를 기준으로 대상 위치와 revision만 갱신하고, 포탈의 start/duration·resource·rotation·scale 및 다른 모든 박스는 보존한다. 내부 source transform의 centered 원점은 그대로 두고 MAP placement로 한 번 이동한다.

현재 `Box Detail → Use Mouse Position`은 기존 MainApp 표면 picking과 stable request 검사를 사용한다. 성공한 좌표는 선택한 Effect MAP 박스의 위치 편집과 preview에 반영되고, `Save`가 staged placement를 검증해 문서에 저장한다. 위치만 바꿀 때 Apply는 필수가 아니다. 기존 커서/paused 상태를 유지하므로 수명 밖에서 보고 있다면 박스 Preview로 시작부터 재생한다. 이 동작과 실패·취소 보존을 현재 코드 및 제품 빌드 기록에서 확인한다.

Sequence는 `CProjectDataRoot` 아래 원본을 직접 읽는 local preview/Save workspace다. 데이터 이동은 새 EXE나 Server publish를 필요로 하지 않는다. JSON parse와 대상 외 의미/byte 보존, 기존 실제 Composition reader의 재로드로 확인하고 Client/UI의 화면 검증은 사용자가 직접 한다.
