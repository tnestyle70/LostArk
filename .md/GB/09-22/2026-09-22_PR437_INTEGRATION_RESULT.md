# PR437 통합 결과

## G00 — 충돌 해결

PR437 `b6e34a748`에 main `1b125a501`을 병합했다. 충돌 두 파일의 protocol 번호를102로 통합하고 ember snapshot3필드, 웨이브 재소환 요청, WALL_CLIMB를 모두 보존했다. 현재 wire를 설명하는 CLAUDE 문장도 교정했다. Desktop 폴더의 쿠크 미커밋 변경에는 손대지 않았다.

## G00 — 실행 검증

Shared 및 NetworkProtocolHarness Debug 빌드 성공. NetworkProtocolHarness 실제 실행 `failures : 0`. 기존 WORLD 검증 두 건은 STOP_CUE 추가 이후 초기값이 불완전한 fixture와 untilDestroyed1바이트 추가 후 오래된 placement flag offset 때문에 실패했다. 현재 serializer 계약에 맞는 fixture로 교정했으며 reader/writer를 완화하지 않았다. 변경 JSON/XML parse 및 diff check 성공.

격리 worktree 최초 Product 빌드는 Engine C++/HLSL 컴파일 후 LFS pointer인 DirectXTKd.lib로 링크 실패했다. 필요한 Engine third-party LFS 실물을 받아 증분 Build 재실행 중이다. Product 완료와 Client 화면 확인은 아직 기록하지 않는다.
