# 베른 최초 입장 컷신 1회 재생

## 원인과 수정

기존 m_bEntranceCinematicDone은 CLevel_Bern 인스턴스 수명만 유지돼 레이드 복귀로
새 Level이 생성될 때 false가 됐다. Level_Bern.cpp의 main-thread process-session 상태로
실제 카메라 pose 적용 또는 ESC 스킵을 기억하며 Ready_EntranceCinematic에서 재입장을
차단한다. Level 종료가 이 기록을 지우지 않는다. 로드/카메라 소유권/첫 pose 실패는
기록을 소비하지 않아 다음 입장에서 재시도할 수 있다.

Client 재실행 시 초기화된다. 계정/캐릭터별 영구 시청 기록은 추가하지 않았다.
카메라 원본, 재생 시간, ESC 및 follow camera 복구 경로는 유지했다.

## 검증과 사용

- test_bern_entrance_camera_contract.py: 기존3개와 session latch 구조 검사1개 PASS.
- Debug Product 빌드 PASS: out/BuildPipeline/runs/20260923T020203534Z-debug-product.json.
- git diff --check PASS. 데이터/Shared/Server 변경 없음, publisher 불필요.
- Client/UI 실행은 하지 않았다. 새 Client에서 최초 베른 입장 재생 또는 ESC 스킵 후
  발탄/쿠크를 다녀오면 반복되지 않는지 사용자가 확인한다.
