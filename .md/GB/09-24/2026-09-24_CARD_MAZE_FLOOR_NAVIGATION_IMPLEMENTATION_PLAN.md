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
