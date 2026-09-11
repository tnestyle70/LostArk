# Server 초기화 PATTERNLOGICPUSH 데이터 복구 결과

## 이전 main e26cd2b2에서의 원인과 조치

`Process gameplay generation failed to initialize. Status=Boss logic push requires paired bounded range and duration`은 당시 C++와 실행 데이터의 버전 불일치였다. 게시 전 `Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap` 620~626행에는 P21의 PATTERNLOGICPUSH에 `BOSS_FORWARD`가 추가된 9열 행 7개가 있었다. 당시 checkout의 `GameplayCatalog.cpp`와 `Publish-GameplayBalance.ps1`의 계약은 8열이고, 정본에는 P24의 거리 2m/시간 242ms 한 건만 있었다.

실행 데이터 원본은 `out/ServerStartupPush20260911/Gameplay.before.bootstrap`에 보존했다. Gameplay 단독 Publish는 이전 Kouku Product와 정본의 불일치를 검출하고 실패했다. 이어 `Tools/Build/Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon`으로 정본 revision263의 Product/Map/World/Gameplay를 정상 게시했다. bootstrap을 직접 편집하거나 parser 검증을 완화하지 않았다. 게시 후 push 행은 P24 한 건, 8열이다.

## 이전 checkout에서 수행한 검증

- Kouku domain publisher 4개 PASS. `out/ServerStartupPush20260911/publish-owner.log`.
- 기존 Debug `Server.exe --headless --bind-address 0.0.0.0 --smoke-timeout-ms 15000` 실행: 초기화 성공, 0.0.0.0:7777 LISTEN 관찰, 자동 종료 exit0. `out/ServerStartupPush20260911/server-smoke.log`.
- 게시 JSON 5개 parse 및 `git diff --check` 통과. 게시된 Git 관리 파일의 정규화 diff는 없다.
- C++ 변경이 없어 재컴파일하지 않았다. 사용 중인 Debug Server.exe를 직접 실행해 확인했다.
- 검증용 Server는 종료됐다. Client/UI 실행과 화면 검증은 수행하지 않았다. 사용자는 Visual Studio의 `Server + Client` profile을 Ctrl+F5로 실행한다.

통합 시퀀서 순차 재생/1관문 모델 연결 작업은 사용자의 긴급 오류 우선 요청으로 구현 전 보류했다.

## PR #358 최신 main 통합

main db8c2d2d의 parser와 publisher는 PATTERNLOGICPUSH 8·9열을 모두 지원한다. 최신 Kouku Composition 정본은 revision323이며 P21의 BOSS_FORWARD 7건(9열), P24의 기본 AWAY_FROM_BOSS 1건(8열)을 포함한다. 이전 263의 실행 데이터로 되돌리지 않고 이 최신 계약을 보존했다.

`Tools/Build/Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon -ExpectedKoukuSaydonSourceRevision 323` 실행으로 Product/Map/World/Gameplay 4 domain PASS. 실행 bootstrap의 P21 7건과 P24 1건을 확인했다. 차원술사 BA SKILLSTAGE도 최신 정본의 0~2 세 단계로 게시됐다. `project_kouku_saydon_composition.py --repository-root . --mode validate`도 revision323, 저장30/실행26/실행 stage200으로 통과했다. 로그는 `out/PR358Integration20260911/publish.log`다.

통합 Product Debug 로그에서 Engine/Shared/Server 빌드 완료를 확인했다. 이어 Client를 컴파일하던 중 사용자 Visual Studio도 같은 Debug 출력에 빌드하는 것이 확인되어 에이전트가 시작한 build process tree만 중단했다. 로그는 `out/PR358Integration20260911/product-build.log`이며 최종 Product PASS receipt는 없다. 사용자 빌드 중 EXE를 별도로 실행하거나 배포하지 않았다. 새 Server의 초기화 검증 및 Client 화면 확인은 사용자가 완료한 빌드로 수행한다. 앞 절의 이전 checkout Server smoke를 최신 main 실행 검증으로 간주하지 않는다.
