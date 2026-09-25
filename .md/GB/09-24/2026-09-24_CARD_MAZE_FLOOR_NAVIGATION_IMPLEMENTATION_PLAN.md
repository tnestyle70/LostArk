# 카드미로 스폰 높이의 바닥 navigation

## G00. 실제 source와 현재 높이

`cardmaze.telescope`와 Composition logic40의 입장 Y는 -0.01m다.
현재 CardMiro source에는 -0.01m 외에 약1.84m 카드 윗면,2.67m 장식과
-15.522m 아래 표면이 섞인다. 기존 bake bounds의 높이는37.9m다.

현재 표시 placement `10296705976280178153`의 WModel을 직접 decode하면
800개 삼각형이 모두 Y=-0.01m이며 스폰과 통로 표본5개를 지지한다.
실제 extent는 X[-21.755868,21.755868], Z[1329.924182,1373.435918]이다.
asset 이름 추측이 아니라 실제 transform과 triangle 표본으로 바닥을 선택한다.

## G01. 기존 CNavGridBaker 소비와 게시

기존 `CNavGridBaker::Build/Save_Source`를 headless scratch caller로 실행한다.
XZ 원점·크기·0.5m 셀과 region ID를 유지하고, 수직 bounds는 스폰 Y 중심0.1m로
줄여 저장한다. 검증된 실제 바닥 placement의 삼각형을 사용한다. 가짜 평면이나
높이 일괄 치환은 만들지 않는다. 기존 navpaint의 바닥 위 벽 차단은 보존한다. 새 source에서 실제 바닥 밖이 된 셀의
BLOCKED override는 NO_SURFACE 자체가 차단하므로 중복 행만 제거한다.

후보를 검사한 뒤 최신 source hash가 최초 읽은 값과 같을 때 백업·원자 교체한다.
`Publish-ServerNavigation.ps1 -AreaId LV_LUT_MIDNIGHTC_ED`로 Client/Server에
같은 결과를 게시한다. 다른 region의 byte 변경 여부를 확인한다.

## G02. 검증

source의 실제 지지셀 높이, 벽 차단, 스폰 연결과 실제 ServerNavigation 경로를 확인한다.
Client 피킹의 화면 ray→바닥→typed move command 소비도 조사한다.
새 C++/project 등록은 없다. 화면 클릭과 Debug/Release 체감은 사용자 확인 대상이다.

## G03. 반투명 바닥의 실제 피킹 표면

후속 사용자 캡처에서 클릭 표식이 바닥 아래에 찍히고 이동하지 않는 증상을 확인했다.
반사 재질의 외형은 사용자가 정상으로 확인했다. 현재 카드 바닥은 BLEND 경로여서
불투명 geometry가 쓰는 `Target_PickPos`에 포함되지 않는다. Controller는 유효한 아래 배경면의
XYZ를 그대로 사용하고 player Y 평면 fallback을 하지 않는다. 카메라 ray가 비스듬하면
목표 XZ까지 달라져 Server의 정상 CardMiro grid에서 거부될 수 있다.

Nav나 바닥 재질을 바꾸지 않고 기존 MapAssetObject의 실제 mesh와 world matrix를 피킹에 쓴다.
Engine의 범용 `RENDERGROUP::PICKING`은 모든 조명·decal·post/UI/debug 소비가 끝난 뒤
`Target_PickPos` 하나에만 no-clear로 기록한다. 이 target의 W는 frame 중 조명 보조 데이터이므로
G-buffer 단계에서 바닥 위치만 덮지 않는다. 끝난 frame의 피킹 값만 보완하고 다음 frame의
기존 G-buffer 초기화가 다시 정본 geometry 데이터를 만든다.

Client의 기존 정확한 CardMaze floor placement/asset 정책만 이 큐에 opt-in한다.
Static mesh shader에는 끝에 피킹 pass를 추가하여 기존 pass index를 보존한다. 기존 world/view/proj와
cull/mirror를 사용하고 실제 불투명 depth로 가려진 바닥은 기록하지 않는다. 모델·Transform·alpha·
reflection·nav data는 변경하지 않는다. 새 C++ 파일과 프로젝트 등록 변경은 없다.

불투명 렌더 직후의 depth를 피킹 전용 texture/DSV에 복사한다. 늦은 피킹은 이 복사본만 사용해
UI·Nav debug의 depth 기록에 영향을 받지 않고, 실제 화면 depth도 변경하지 않는다.
Final의 material debug view도 PickPos.W를 읽으므로 모든 화면 단계 뒤라는 순서를 유지한다.

실제 shader compile/깊이 가림·위치 수치 검사와 정상 Product Build를 실행한다.
사용자의 실제 화면 클릭·이동은 자동 확인으로 기록하지 않는다.

## G04. Character Select 진입 실패의 shader family 계약 수정

추가한 Picking pass25~27이 base와 source-group variant 모두에 BASE(1)를 선언했다.
Engine CShader는 base가 BASE이면 variant가 UNAVAILABLE(2)여야 한다고 검사하므로
static mesh shader 생성이 실패한다. Loader가 실패 정리를 위해 Effect 작업을 취소하면서
후속 already-cancelled 문구가 최초 원인처럼 표시된다. Effect mailbox를 다시 열어 우회하지 않는다.

기존 ChargeAfterimage와 같이 base에서만 picking pixel shader를 만들고,
variant에는 같은 pass 이름과 vertex input signature를 유지하며 UNAVAILABLE을 선언한다.
Ready_StaticMeshShader는 실제 shader 생성 실패 단계를 status에 남긴다.
새 제품 C++ 파일과 project 등록은 없다.

기본 FX 단독 검사에 더해 실제 CShader factory로 base와 14개 source-group variant를 함께
로드하여 변경 전 실패와 변경 후 성공을 확인한다. 기존 피킹 GPU 수치 검사를 다시 실행하고
정상 Debug Product Build를 수행한다. Client 화면 진입과 클릭은 사용자 확인으로 남긴다.
