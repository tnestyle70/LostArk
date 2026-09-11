# Server 초기화 PATTERNLOGICPUSH 데이터 복구 결과

## 원인과 조치

`Process gameplay generation failed to initialize. Status=Boss logic push requires paired bounded range and duration`은 현재 C++와 오래된 실행 데이터의 불일치였다. 게시 전 `Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap` 620~626행에는 P21의 PATTERNLOGICPUSH에 `BOSS_FORWARD`가 추가된 9열 행 7개가 있었다. 현재 `GameplayCatalog.cpp`와 `Publish-GameplayBalance.ps1`의 계약은 8열이고, 정본에는 P24의 거리 2m/시간 242ms 한 건만 있다.

실행 데이터 원본은 `out/ServerStartupPush20260911/Gameplay.before.bootstrap`에 보존했다. Gameplay 단독 Publish는 이전 Kouku Product와 정본의 불일치를 검출하고 실패했다. 이어 `Tools/Build/Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon`으로 정본 revision263의 Product/Map/World/Gameplay를 정상 게시했다. bootstrap을 직접 편집하거나 parser 검증을 완화하지 않았다. 게시 후 push 행은 P24 한 건, 8열이다.

## 실제 검증

- Kouku domain publisher 4개 PASS. `out/ServerStartupPush20260911/publish-owner.log`.
- 기존 Debug `Server.exe --headless --bind-address 0.0.0.0 --smoke-timeout-ms 15000` 실행: 초기화 성공, 0.0.0.0:7777 LISTEN 관찰, 자동 종료 exit0. `out/ServerStartupPush20260911/server-smoke.log`.
- 게시 JSON 5개 parse 및 `git diff --check` 통과. 게시된 Git 관리 파일의 정규화 diff는 없다.
- C++ 변경이 없어 재컴파일하지 않았다. 사용 중인 Debug Server.exe를 직접 실행해 확인했다.
- 검증용 Server는 종료됐다. Client/UI 실행과 화면 검증은 수행하지 않았다. 사용자는 Visual Studio의 `Server + Client` profile을 Ctrl+F5로 실행한다.

통합 시퀀서 순차 재생/1관문 모델 연결 작업은 사용자의 긴급 오류 우선 요청으로 구현 전 보류했다.
