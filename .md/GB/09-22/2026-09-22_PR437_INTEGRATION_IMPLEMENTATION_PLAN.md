# PR437 통합 구현 계획

## G00 — Shared protocol 충돌 해결

기준은 PR437 `b6e34a748`과 main `1b125a501`이다. 기존 Desktop 작업 폴더의 쿠크 미커밋 변경을 유지하고 별도 worktree에서 병합한다.

`PacketType.h`의 양쪽 protocol 변경을 모두 유지하며 통합 번호102를 사용한다. `PacketMessages.h/.cpp`의 ember3필드, wave request, WALL_CLIMB 정의와 writer/reader 자동 병합 결과를 검토한다. `NetworkProtocolHarness.cpp`는 현재 번호 단언과 실제 snapshot round-trip/잘못된 입력 보존을 확인한다. CLAUDE의 현재 wire 설명을 함께 교정한다. 새 C++ 파일이나 project 등록은 없다.

검증은 Shared/NetworkProtocolHarness 컴파일과 실제 harness, 변경 JSON/XML parse, diff check다. Product 빌드는 병합된 실제 소스 기준으로 수행하며 Client/UI는 실행하지 않는다. 검증된 merge commit을 PR437 head에 정상 push한 뒤 PR을 병합하고 최신 main을 수신한다. Desktop 미커밋 복원 시 파일 전체 ours/theirs를 사용하지 않는다.
