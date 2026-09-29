# World Movie의 Effect Tool V1 편집과 런타임 카메라

## G00. 목표와 현재 연결

`All Effects → World → Character Selection Movies`의 Open Editor는 현재 WORLD Sequencer만
열고, Effect 박스에서 V1을 열면 Movie를 Stop한다. V1은 Movie 배우와 독립적인 모델 preview를
사용한다. Camera Box Detail의 key와 실제 pose는 읽기 전용 표시이고 일반 Apply row는 재생을
중지한다. 이 작업은 같은 Movie owner를 V1 element 편집과 camera key 편집에서도 유지한다.

정본은 기존 `Data/Camera/ClassSelection.cinematics.json`, SL00 WorldSequences와
`Data/Effects/Authored/*.effect.json`이다. 원본 데이터의 외부 교체나 별도 Movie player를 추가하지
않는다. 기존 09-25 FOUR_CLASS_SELECTION_MOVIES PLAN/RESULT의 admission·저장 계약을 따른다.

## G01. V1 Movie 편집 진입과 재생

`Effect_Tool.h`, `Effect_Tool_Workspace.cpp`, `Effect_Tool_ResourceBrowser.cpp`, `Effect_Tool.cpp`와
`MainApp.cpp`에서 typed class/phase/Effect ID를 연결한다. Open Editor는 선택 Movie의 실제
Effect 문서를 V1 Current Effect에 열고 element 목록·Detail을 노출한다. Movie의 Intro/Loop
Effect 목록을 선택해 같은 V1 문서를 전환한다. 기존 미저장 문서 보호를 유지한다.

Play All·Pause·Stop·seek는 Level 소유 `CClassSelectionPresentation`에 전달한다. V1 Model View는
해당 Movie의 재생 상태와 Effect 목록을 표시하고 기존 WORLD 배우·애니메이션을 함께 사용한다.
카메라/다른 row 편집은 같은 WORLD Sequencer를 연다. 서로 다른 모델 preview가 Movie를 덮지 않는다.

## G02. Element draft의 Movie 반영

`Effect_Tool_Playback.cpp`의 stage와 기존 ClassSelection/Effect presentation 경로를 연결한다.
검증된 V1 draft는 해당 Movie의 Effect target에만 임시 적용하며 배우·카메라·시계는 유지한다.
실패는 현재 문서와 재생을 보존하고 이유를 표시한다. Save는 기존 V1 저장·prepared target 갱신을
사용한다. Movie 편집 종료·다른 문서 이동과 Level 종료의 임시 override 정리를 확인한다.

`Effect_DocumentRenderer`의 기존 prepared resource 빌더로 catalog에 게시하지 않는 불변 target을
준비한다. `Effect_PresentationService`는 기존 level-owned active/pending handle을 현재 transform
history에서 검증한 뒤 교체한다. `ClassSelectionPresentation`이 target 수명을 소유하고 다음
Intro/Loop·seek spawn에 전달한다. active 교체는 기존 visibility와 handle을 유지하고 scene 비용은
교체 후 집합으로 한 번 계산한다. `Effect_Object.h`의 visibility 조회는 이 보존에만 사용한다.

## G03. Camera Box Detail

`SequencerTool.cpp/.h`, `ClassSelectionPresentation.cpp/.h`,
`ClassSelectionPresentation_Authoring.cpp`에서 선택한 camera key의 Eye·LookAt·FOV를 편집한다.
Camera-only Apply는 유효한 동일 stable row를 기존 scene/draft에 반영하고 현재 Movie 시각에서
카메라를 다시 평가한다. 재생 token·pause·phase를 유지하며 Save movie는 기존 freshness 병합을 쓴다.
다른 row의 full admission과 실패 보존 경로는 유지한다.

## G04. 검증과 인계

기존 파일의 인코딩을 유지한다. 새 C++ 파일을 만들지 않으므로 project/filter 등록 변경은 없다.
정상 Debug Product 증분 Build, 관련 native 편집 검증의 재사용 가능 범위, `git diff --check`를
확인한다. 원본 JSON을 직접 수정하지 않으며 저장 검증은 임시 사본에서 한다. RESULT에는 실제
실행한 검사와 미실행한 UI 확인을 분리한다. 사용자 화면 경로는 World Movie Open Editor →
Effect/element 선택 → Detail 변경 → Play All 및 Camera Box Detail 위치 변경 → Save movie다.

## G05. 실제 버튼 경로와 선택 구간 재생 보완 (2026-09-27)

현재 Movie asset은 `effect.classselect.*`지만 Element 행 Solo가 기존 recovery/world ID 검사에서
빠져 일반 preview filter 거절로 끝난다. 선택 묶음은 내부 helper에 연결되지만 현재 시각을
재개하기만 하며 Stop 뒤 시작과 선택 구간 반복이 없다. WORLD Play All도 V1 격리 복원 없이
Level Play만 호출한다. 사용자 요청에 따라 실제 버튼 진입점을 같은 Movie owner로 연결한다.

Effect Tool은 전체 Effect draft와 dependency를 포함한 선택 투영을 함께 전달한다.
ClassSelectionPresentation은 전체 draft와 선택 target을 별도로 보존하고 원본 PSC age를
phase source time과 movie time으로 변환한다. 선택한 occurrence의 구간에서 Solo는 한 번 재생 후
일시정지하고 Group은 같은 구간을 반복한다. 배우와 카메라는 기존 WORLD clock을 소비하며 다른
Effect occurrence는 선택 preview 동안 제외한다. Play All은 어느 창에서 눌러도 선택 범위를 해제하고
편집한 전체 문서로 Intro부터 시작한다. Stop은 초안을 보존한다. 실패한 준비는 기존 초안과 재생을
보존하고, 편집 종료는 임시 target과 범위를 정리한다.

Element/Solo/marked group/anchor group/manual group/family의 실제 명령과 부모 emitter 편집을
함께 조사한다. V1에도 Movie speed를 노출하고 현재 scope와 source/movie 시각을 표시한다.
Timeline은 원본 source 시간 계약과 stable box/key ID를 유지하며 seek·row 편집의 실패 보존을
검증한다. 새 runtime/타이머를 추가하지 않고 기존 등록 파일을 확장하며 인코딩을 유지한다.

검증은 과거 helper 직접 호출을 실제 Try_SoloElement·Group·Play All entry로 교체한다.
현재 Product OBJ를 사용한 창 없는 native fixture에서 Stop→Solo, 범위 종료, Group 반복,
Pause/Seek/배속, 두 창의 Play All, 편집·저장·재로드 및 실패 보존을 검사한다. Product Debug build와
관련 JSON/XML parse·git diff --check를 실행한다. Client/UI 실행과 최종 화면 판정은 사용자가 한다.
## G06. Movie 원본 재질 및 카메라 감사 (2026-09-27)

정상 DimensionMaster A는 비교 기준이며 수정 대상은 Movie occurrence다. 다섯 Movie의
참조 Effect·element·World object·재질·texture를 현재 설치본과 원본 receipt에 대조한다.
확인한 source shader의 primitive RGB/opacity 누락은 해당 PS/VS와 MIC 상수 소유가
확정된 program에만 복구하고 generator와 설치 HLSLI를 함께 유지한다. 다른 render option은
변경하지 않는다. 원본 PSC freeze, shader 출력, 실제 draw 제출을 별도 검사한다.

카메라는 다섯 클래스의 Intro/Loop 전체 Director cut, camera와 parent dummy, raw FRotator,
Eye/LookAt/Up·FOV를 원본과 대조한다. 설치 camera→실제 pipeline과 모델의 face/head 투영도
검사해 카메라 key와 배우 pose 문제를 구분한다. 얼굴 중심을 맞추는 임의 높이 보정은 하지
않고 원본과 다른 변환이나 bake가 확인된 범위만 고친다. 리소스 교체가 필요하면 후보와
검증을 먼저 준비하고 최신 저장본·해시·백업·원자 교체 계약을 따른다.

## G07. Object·World 공통 타임라인 배치와 조작 (2026-09-27)

사용자가 Boss/Sequence와 같은 행 구성과 전용 편집 도구 연결을 승인했다. 현재 공통 shell과
CompositionTimeline의 box 그림은 공유하지만 Object/World의 행 배치는 별도다. 기존 Saydon과
Effect Sequencer의 시간 겹침 기준 lane packing을 공용 helper로 옮긴다. 표시 행 번호는 저장 ID가
아니며 box의 stable ID·소유자·animation slot과 source/movie 시간 의미를 유지한다.

World는 같은 종류의 비중첩 box를 같은 행에 배치하고 배우 animation은 occurrence/slot을 유지한다.
World Model·Material·Light 등 장기 유지 항목은 기본 접힌 그룹에서 유지 구간과 개수를 보이며,
펼치기·검색·현재 시간 활성 항목 필터를 제공한다. Object의 부모/Motion overview와 단일 Motion도
같은 배치 규칙을 사용한다. 접기는 재생·mute와 무관한 세션 상태다. 선택·이동·양끝 trim은
각 기존 owner의 검증된 Apply를 사용하며 불가능한 항목은 이유와 편집 가능 범위를 명시한다.

## G08. Sequence Camera Tool과 원본 저장 연결 (2026-09-27)

Camera box에서 Open Sequence Camera Tool을 열어 현재 sequence의 컷과 키를 시간순으로 표시한다.
World와 ALT V는 키 편집 UI를 공유하고 문서·재생 clock·좌표계·FOV 축·저장은 기존 owner가 소유한다.
키 시간(ms 및 frame 표시), Eye/LookAt/Up/FOV, 보간, 키 추가·삭제·이동, 구간 위치 보정을 편집한다.
Save는 미적용 초안까지 검증·반영한 후 실제 재생 소비자의 정본을 저장한다. 실패 시 초안을 보존한다.

ALT V는 preview 투영 시 원본 effect/camera ID와 clip 시간 변환을 보존하거나 원본 camera 자체를
명시적으로 편집한다. action arrangement Save를 product recovery camera Save로 오인하지 않게 한다.
최신 effectsequence의 camera 필드만 stable ID·baseline 기준으로 병합하며 다른 Effect·animation·sound·
localOnlyElementIds를 보존한다. 저장 뒤 명시적인 기존 product camera 재준비 경로로 연결한다.
공용 편집 코드를 새 파일에 두면 Client.vcxproj와 filters에 현재 물리 분류대로 필요한 등록만 추가한다.

## G09. Save·Publish 상태와 복원 잔여 점검 (2026-09-27)

Save는 authoring 저장, Publish는 해당 owner의 검증된 제품 반영이다. 카메라 키는 Client 표현이며
Server에는 기존 action/pattern의 시작·단계·판정 계약만 전달한다. 원격 Server 자동 배포로 설명하지
않는다. World의 기존 WorldSequences publisher 및 카메라 직접 소비/재바인딩을 재사용하고,
ALT V의 폐기된 Effect publisher를 새로 만들지 않는다. 미저장/저장/반영 실패 상태를 분리한다.

이미 복구한 native primitive opacity/RGBA와 설치 shader의 실제 compile 결과를 마무리한다.
잔여 회색 후보는 실제 source ABI·재질·texture·scene 입력으로 좁히고 확인된 누락만 수정한다.
정상 DimensionMaster A, 렌더링 옵션과 다른 세션의 사용자 튜닝은 보존한다.

## G10. 완료 검증

공용 lane 배치의 비중첩·중첩·identity 보존, 실제 box 편집의 source/movie 시간 변환과 실패 보존,
카메라 키 수정→저장→재로드→제품 sample의 일치를 검증한다. 기존 Solo/Group/Play All 회귀도
필요한 변경 경계에서 확인한다. 정상 Debug Product build, 관련 JSON/XML parse와 diff check를
실행한다. Save 검증은 sandbox 사본을 사용한다. UI 조작과 최종 시각 판정은 사용자가 수행한다.

## G11. Character Select 로딩 후 static batch shadow 종료 복구

114/114 Effect 준비 후 발생한 종료의 실제 PID 로그를 보존하고, 설치된 Character Select 맵의
wind-enabled material을 CMapStaticBatchObject::Render_Shadow로 재현한다. 현재 CPU가 필수로
바인딩하는 foliage wind program과 shader 소비자의 누락을 원본 프로그램별로 대조한다.
필수 바인딩을 무시하거나 바람·그림자를 끄지 않고 기존 static/instanced 소비자를 복구한다.
실패 로그에는 batch asset, mesh, pass, 단계와 HRESULT를 남겨 다음 실패의 원인을 보존한다.
창 없는 실제 Product OBJ/CSO 검증과 최소 Debug Product 빌드로 확인하며 Client/UI 실행과
최종 화면 판정은 사용자가 수행한다. Server 프로세스와 렌더링 옵션은 변경하지 않는다.

G11 실측 보완: 실제 SL00에는 wind-enabled mesh가 없으며, 필수 shader uniform도 설치 CSO에 있다.
CMaterial::Bind_SurfaceTexture의 ComPtr 주소 차용이 WRL operator&의 출력 인자 변환을 호출해
소유 SRV를 해제하는 결함을 확인했다. 해당 10개 주소 취득을 std::addressof로 바꾸고 기존
복사 감소 최적화는 유지한다. 이 원인 수정과 실제 전체 batch의 before/after 검증을 우선하며,
미설치 wind 프로그램 후보는 이번 종료 수정에 섞지 않는다.

## G12. Movie WORLD 검사와 현재 위치 F6 자유 카메라 (2026-09-27)

사용자가 WORLD 항목별 선택·Solo/Mute, Movie 제외 저장, 두 도구의 같은 시간 재생과
F6 자유 카메라 구현을 승인했다. 기존 ClassSelectionPresentation과 WorldSequencePlayer를
확장한다. Effect Tool과 WORLD Action Workbench/Sequencer는 동일한 검사 상태와 명령을
소비하며 별도 Movie 런타임이나 모델 복제 경로를 만들지 않는다.

각 WORLD 행은 instance/slot/object stable ID, WModel, 재질, 실제 sampled XYZ와 authored
visibility를 표시한다. 장면 피킹은 현재 표시된 모델과 그 mesh를 식별한다. Solo/Mute와
선택 강조는 draw만 제어하며 시간, 배우 pose, bone provider를 유지한다. 현재 Movie에서
제외한 object ID는 클래스 Movie의 optional excludedWorldObjectIds에 저장한다. Intro/Loop가
참조하는 같은 리소스의 표시를 제외하되 전역 리소스와 원본 visibility는 삭제하지 않는다.
최신 디스크·stable ID 3-way merge와 기존 Save/Reload 실패 보존 절차를 재사용한다.
임시 Solo/Mute, 검사 카메라, 검색 상태는 저장하지 않는다.

명시적인 Play는 Movie 카메라로 시작한다. 재생·일시정지 중 F6는 현재 Eye/LookAt/Up/FOV를
유지한 자유 카메라와 현재 Movie 시각의 카메라 사이를 전환한다. Seek, phase 전환, Solo가
자유 카메라를 강제로 회수하지 않는다. 자유 카메라에서 Stop은 위치를 유지하며 다음 F6는
서버 플레이어 follow로 돌아간다. 두 창은 같은 owner를 사용한다.

기존 C++ 파일의 인코딩을 유지하고 새 검사 DTO/공용 UI/owner 구현 파일은 Client.vcxproj와
filters의 해당 물리 분류에 등록한다. 실제 Product 소비자를 사용한 창 없는 native fixture로
카메라 전환, draw 격리 중 anchor 유지, 잘못된 ID 실패 보존, 제외 저장·재로드를 검증한다.
현재 사용자 Client/Server는 종료하거나 조작하지 않는다. 실행 중 EXE/DLL을 교체하지 않는
최소 컴파일과 JSON/XML parse 및 diff check를 수행하고 링크·사용자 화면 확인을 구분한다.
회색 plane의 원인 확정과 실제 제외할 항목의 선택은 새 검사 기능에서 사용자가 확인한다.

## G13. Composition Sequencer의 Movie 숨김 조작 노출 (2026-09-29)

`SequencerTool.cpp`의 WORLD session은 모델 검사 UI를 Preview 창에만 그린다. 사용자가
첨부한 Composition Sequencer에는 선택 막대만 있으므로 그 창 위에서 모델 Mute/Solo,
Movie 제외/복원과 Save movie를 바로 실행하고 전체 Movie visibility 창을 열도록 연결한다.
숨김 상태를 막대에도 표시하며 선택된 Effect에서는 Element 편집기로 바로 진입한다.
기존 inspection stable ID, draw gate, excludedWorldObjectIds 저장과 실패 보존을 재사용한다.

`ClassMovieInspector.cpp`에는 목록 행마다 Mute/Solo와 Movie 포함 여부를 표시해 선택 후
아래까지 스크롤하지 않아도 조작할 수 있게 한다. 임시 Mute와 저장되는 제외를 구분한다.
`Effect_Tool_Workspace.cpp`의 Movie controls에는 현재 선택 Element의 Mute/Solo와 저장되는
Visible 초안 및 Save Changes를 노출한다. 현재 Detail draft를 보존해 같은 검증·preview·save
소비자로 전달하며, 이전 Solo target이 새 선택의 Mute 대상으로 남는 경우를 교정한다.

기존 세 C++ 파일만 수정하므로 project/filter 신규 등록은 없다. 기존 UTF-8와 CRLF를 유지한다.
최소 Product 컴파일과 diff 검사, 실제 owner/소비자 연결을 확인한다. Client 실행·UI 조작·
회색 요소의 최종 선택과 화면 판정은 사용자가 수행한다. 이 변경에서 요소를 임의로 지우거나
현재 저장된 Effect/Movie 데이터와 렌더링 옵션을 수정하지 않는다.

G13 경계 보완: 마지막 Element도 Mute와 Visible OFF가 가능해야 한다. `Effect_Tool.h`의
typed `previewVisibility` callback을 `MainApp.cpp`가 기존 Movie owner로 전달한다.
`ClassSelectionPresentation::Preview_EffectDocument`의 optional draw ID 집합은 full 문서와
동일한 prepared target을 사용하되 empty mask도 허용하여 sim/clock과 저장 visibility를
보존한다. 개별·그룹 Mute는 이 경로로 갱신하고 Play All이 완전한 draft를 복구한다.
`Effect_DocumentCodec_RuntimeValidation.cpp`는 schema 검증에 성공한 비어 있지 않은 문서가
모든 Element/ModelCue를 명시적으로 숨긴 경우에만 no-draw로 허용한다. 빈 문서와 visible인
미지원 carrier의 거부는 유지하며, 이는 마지막 Visible OFF의 Save와 다시 읽기에 필요하다.
새 C++ 파일은 없고 기존 두 header와 여섯 TU가 최종 변경 범위다.

G13 사용자 조작 보완: WORLD 선택의 `Delete from Movie`와 Delete 키는 기존
excludedWorldObjectIds 변경을 요청한다. 키는 Composition Sequencer에 focus가 있고
입력창·popup·drag·미적용 row·publish가 없을 때만 받으며 Effect나 Background는 삭제하지
않는다. 명령을 frame 끝까지 stable ID로 지연하여 목록 순회 중 객체를 바꾸지 않는다.
Element Mute는 stable ID를 누적하고 Unmute는 그 ID만 해제해 같은 시각에서 비교한다.
