# 베른 실제 계단 연결 복구 계획

## G00. 현재 막힘의 원인

기존 50×347 source/paint와 1m traversal policy를 조사했다. 북쪽 진입 계단과 08-25의 세 ridge는 그대로 보존한다. 남쪽 계단 입구 row47(z=-152.188004), x23..29는 인접 지상 약50.58m 대신 아치 상단54.5~56.3m를 선택해 남쪽 NPC3명의 성분을 spawn에서 분리한다.

## G01. 실제 설치 모델에 근거한 paint 복구

정본 배치 LV_BER_BERNCASTLE_T_SL00:export:5671의 STAIR02D 설치 WModel은 geometryPreScale0.01, 실제 배치/회전을 적용했을 때 해당7개셀 모두 y=50.576641m의 평평한 면을 제공한다. 이7개셀만 WALKABLE+실측높이로 다시 굽는다. 다른 지붕·벽·절벽과 1m step 정책은 유지한다. 기존08-06 기능증명용 collision.bern.editor-proof는 disabled로 바꾸어 보이는 바닥 위 인공벽을 제거한다. 원본 정의는 보존해 collision unit fixture가 계속 검증할 수 있다.

## G02. 소비자와 검증

Data/Navigation paint → Publish-ServerNavigation → Server/Client navgrid, Data/Worlds Gameplay → Publish-WorldGameplay → Server collision의 기존 경로로 배포한다. Server navigation 계약에 남쪽 계단 끝 경로와 네 번째 ridge 높이를 추가한다. 기존 collision 회귀는 제품의 disabled 상태와 격리 fixture의 enabled 상태를 분리한다. 실제 입력·경로·publisher·Server 실행을 검증하며 Client 화면 보행은 사용자가 직접 확인한다. 새 C++ 파일과 project/filter 등록은 필요 없다.
