# 쿠크 Sequence 재생과 Object 편집 구현 결과

## G00. 반영 범위와 상태

소스·데이터 반영 및 아래 수치 검증을 완료했다. Client/UI 실행·조작·캡처는 하지 않았다.
최종 화면·사용자 입력 판정은 사용자 확인 경계다. 기존 MainApp/Composition/Map 사용자
변경을 보존했고 Server Mario 변경과 같은 파일에서는 Preview 함수만 별도로 수정했다.
신규 C++ 파일은 없다. 전체 Product 빌드는 통합 작업에서 진행한다.

## G01. 작은 오망성 및 19,389ms 재생 중단

실제 작은 오망성 정본은 `effect.kouku.gate3.mario.boss.pentagram.full.restore`다.
Resource ID `kakulsaydon.effect.7e765b32937e215a2fef`와 마리오 Pattern 34가 같은 정본을
소비하며 `pantagram` 철자의 다른 asset/alias는 확인되지 않았다.

MainApp은 새 Resource Preview를 시작한 뒤 이전 Animation Preview를 정리하면서 정상
정리 메시지 `No KoukuSaydon composition preview is playing`을 공용 상태에 덮어썼다.
정리용 상태를 별도 지역 문자열로 분리했다. 실제 V1 실패는 준비 큐의 failure receipt를
`Get_ProductCuePreparationFailure`로 읽고 occurrence의 `failureStatus`에 처음 이유를
보존한다. 뒤의 성공/Queued 메시지가 앞의 실패 근거를 가리지 않는다.

19,389ms capture boundary에서는 이전 popup source-material-carriers의 커튼 준비 실패가
남아 있었다. `bg_rad_koukusaton_curtain01c_sm.wmodel`은 정상 decode됐지만 내장 material의
세 texture가 새 Effect/Meshes 상대 위치에 없었다. 원본 Map의 동일 hash 파일을 설치했다.
재질 override가 있어도 CModel은 내장 material 의존성을 먼저 준비하므로 mesh만 복사하면
실패한다. `Tools/KoukuSaydonPipeline/install_popup_curtain_materials.py --install`이 정확한
mesh hash와 세 원본 texture hash를 검증한 뒤 누락 파일만 설치한다. 다른 기존 파일은
덮어쓰지 않는다. `--install` 없이 설치 상태를 검사할 수 있다.

capture hold는 모든 과거 failed row 때문에 전체 preview를 중단하지 않고 실제 capture
occurrence의 실패만 처리한다. actor Pattern을 가진 bundle 경로도 동일한 boundary/한 프레임
대기를 소비하도록 연결했다. Preview의 V1 targets를 먼저 priority queue에 넣고 준비 시간이
짧은 Effect window를 소진하지 않도록 준비가 settled될 때까지 clock을 유지한다.

현재 설치 상태에서 작은 오망성의 Product 준비 실패 자체는 재현되지 않았다. 실제 오류
메시지 보존, cold preparation clock, 설치 누락, capture의 과도한 실패 전파를 각각 교정했다.

## G02. Object timeline 선택·복제

Motion timeline에 Stage/Transform/Animation/Effect 이름을 표시하며 Animation은 slot별 한
행에 배치한다. 개별 animation/effect box의 hit target이 Detail 선택과 연결된다. Animation
Detail의 Duplicate Clip, Effect Detail의 Duplicate Selected Effect, timeline의 Duplicate
Selected Box가 같은 `CWorldSequenceDocument::Duplicate_TimelineBox`를 호출한다.

문서가 candidate를 검증한 뒤 대상 template만 교체한다. Animation duplicate는 현재 clip의
window 뒤에 같은 clip을 삽입하고 뒤의 같은 slot clip을 이동한다. Effect duplicate는 새 stable
ID와 TIME 시작점을 만든다. 필요한 motion 끝 key만 hold로 연장하며 기존 template 주소와
원본 key/Effect 값을 보존한다. 용량·600초 제한·잘못된 선택은 문서와 선택 결과를 보존한다.

실제 작은 괴기스러운 인형 flame template의 11,334ms 값을 사용한 native probe는 Effect와
Animation을 두 번씩 복제해 34,002ms 세 window를 만들고 Save→Load equivalent, template
주소 보존, ID 분리, 실패 rollback까지 13 checks를 통과했다. fixture는 기존 source의 단일
motion과 model/effect 연결을 복사하고 다른 motion 참조만 제거한 입력이며 UI 조작 증거는 아니다.

## G03. 팝업북과 실제 세이튼 Pattern

원본 boss Pattern 36 `세이튼_1관문연출`은 13 clip/40,652ms다. 명목 animation blend Logic 52에
triggerKind가 없어 실제 blend가 없었다. 다른 Pattern의 Logic 52는 유지하고 P36 전용
`kakulsaydon.g1.logic.62` ANIMATION_BLEND에 기존 다섯 window를 연결했다.

Sequencer revision 43에 P36의 실제 clip/source range/play rate를 보존한 Pattern 8
`1관문_연출`을 추가했다. P1과 P4의 Pattern row가 이를 참조한다. 기존 `world.13`의
DEPLOY_BOSS_MN_RPCT_00 puppet 행은 제거하고 그 WORLD에 붙던 spotlight는 실제 BOSS
anchor로 옮겼다. actual actor는 기존 BossCatalog의 MN_RPCT_05 CModel/weapon clone을
사용한다. 걷기·나머지 clip과 다섯 Logic blend가 기존 native animation/root-motion sampler로
이어진다. 확장된 child animation들은 clip마다 행을 늘리지 않고 한 Animation 행에 표시한다.

전체 13 clip이 끝나도록 P1은 37,800→40,652ms, P4는 58,810→61,662ms로 연장했다.
P4 child 시작 21,010ms 및 기존 portal/capture/Effect의 모든 시작점은 유지했다. 마지막
camera/scene profile의 끝 window만 함께 연장해 끝 두 clip 중 시점이 먼저 돌아가지 않게 했다.
원본 WORLD/Effect의 동작 속도와 lifetime은 유지한다.

단일 actor Pattern은 기존 Level WORLD preview owner에 world/lighting 관리를 맡긴다.
그 결과 popup map-light 준비, 구형 book 숨김, 펼친 맵/기본 아레나의 배타적 visibility 계약을
유지하면서 actor animation만 bundle sampler가 담당한다. WORLD가 먼저 sample된 뒤 해당
actor Effect/light가 같은 clock을 소비한다.

## G04. 실행한 검증과 남은 사용자 확인

- `out/KoukuSequenceFix20260914/compile.ps1`: WorldSequenceDocument, WorldObjectTool,
  KoukuSaydonActionWorkbench, KoukuSaydonPresentationPlayer, Effect_PresentationService,
  MainApp의 Debug isolated CL 6 TUs PASS.
- `native_probe.cpp`: pentagram, portal-collapse, portal-context, popup-light, popup-carrier의
  실제 Product CPU stage/Playback Seek PASS(5 documents).
- `gpu_probe.cpp`: UI/window 없는 D3D11 WARP에서 같은 5개 실제 document의
  Stage_Document 및 Stage_VisualProgramTarget PASS. capture render/visual 판정은 하지 않았다.
- `duplicate_probe.cpp`: native duplicate/Save/Load/failure-preserve 13 checks PASS.
- `intro_probe.cpp`: Sequencer parse 및 serialize roundtrip PASS. P1 expansion
  40,652ms/13 clips/5 blends, P4 61,662ms/14 clips(첫 idle 포함)/5 blends PASS.
  설치 MN_RPCT_05 WModel을 실제 decode하고 모든 expanded clip 이름과 마지막 source sample
  범위를 native duration으로 검사해 각각 13/14 endpoints PASS.
- `curtain-texture-closure.json`: exact mesh와 embedded texture 3개 SHA-256 일치.
- 수정 source의 `git diff --check` PASS. 소스 C++의 기존 UTF-8/CRLF를 유지했다.

사용자는 새 Product 빌드 후 F1에서 작은 오망성 Play Effect, 마리오1 preview,
Sequencer `1관문_통합_시퀀스`의 19,389ms 통과 및 61,662ms 완료를 확인한다. 팝업북 마지막
두 clip과 카메라·조명의 화면 결과도 확인한다. Object에서 작은/큰 괴기스러운 인형의 화염
motion을 열어 Animation/Effect box를 각각 선택→Duplicate→Save→다시 열기 순서로 확인한다.
ScreenPost/재질 복원의 최종 시각 PASS는 이 문서에 기록하지 않는다.
