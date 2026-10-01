# Deploy 바닥 CPU 이동 피킹 복구

## G00. 현재 원인과 변경 범위

기준은 `9497dad14`다. `9403f6d49`에서 일반 이동을 CPU 표면 검사로 바꾸면서
각 Level의 resolver가 `CMapPlacementRuntime`만 검사하게 됐다. 발탄의 파괴 가능한
바닥 A/B 네 배치와 난간 두 배치는 별도 `CDeployPropRuntime`에 있으므로 대상에서 빠졌다.
쿠크의 펼쳐지는 종이 다리도 같은 Deploy 경로다. Server navigation과 송신 계약은 유지한다.

## G01. 기존 객체의 실제 표면 조회

`Client/Public/DeployPropObject.h`와 `Client/Private/DeployPropObject.cpp`에
`Try_PickMovementSurface`를 추가한다. 정적 객체는 Render가 고르는 현재 intact/fractured
모델과 실제 world transform으로 기존 `CModel::Try_PickStaticSurface`를 호출한다.
pass0와 같은 BACK cull을 쓰고 모델을 복제하거나 입력마다 geometry를 만들지 않는다.
애니메이션 객체는 기존 `Try_PickCurrentPose`를 사용한다. 이 경로는 현재 bone palette를
읽으며 양면 삼각형 검사다. bind-pose bounds를 별도 선행 필터로 사용하지 않는다.

숨김·opacity·camera suppression·파괴 source suppression은 Render의 조건과 맞춘다.
날아가는 debris를 새 이동 바닥으로 등록하지 않는다. 실패는 출력과 기존 이동을 보존한다.

## G02. 현재 runtime과 Level 연결

`Client/Public/DeployPropRuntime.h`와 `Client/Private/DeployPropRuntime.cpp`에
같은 이름의 조회를 추가한다. 유한 ray와 거리 상한을 검증하고 현재 entry의 가장 가까운
표면만 반환한다. scene replacement 동안에는 기본 맵과 동일하게 조회를 거절한다.

Valtan/Kouku의 기존 resolver에서 일반 맵 hit 거리와 Deploy hit 거리를 비교한다.
일반 맵이 없으면 Deploy만으로 성공할 수 있고, 앞에 맵이 있으면 뒤 Deploy가 가로채지 않는다.
별도 manager, 저장 데이터, protocol, Server 권한 변경은 없다. 기존 H/CPP만 확장하므로
제품 `.vcxproj`와 `.filters` 신규 항목은 없다.

## G03. 검증과 Release 전달

생산 함수 추출 검사로 숨김/파괴/현재 모델과 transform/nearest/실패 보존을 확인하고
기존 CPU 이동 dispatch 회귀를 실행한다. 설치된 발탄 A/B의 실제 geometry도 확인한다.
Release Product Build 뒤 새 receipt와 현재 Data/DataFiles로 별도 PICKING-FIX portable ZIP을
만들고 manifest/hash/CRC와 launcher `--check`를 확인한다. 기존 FINAL ZIP은 보존한다.
Client/UI는 실행하지 않는다. 사용자가 외곽 돌 이동, 파괴 뒤 클릭, 쿠크 다리 펼친 뒤 이동을
직접 확인하며 자동 수치 검증을 화면 성공으로 기록하지 않는다.
