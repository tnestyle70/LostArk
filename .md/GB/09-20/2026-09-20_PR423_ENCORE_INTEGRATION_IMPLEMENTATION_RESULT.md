# PR #423와 앵콜 컷신 병합 결과

## G00. 기준과 작업 범위

PR #423의 `b1a84dc45e451ee9ea8830e1d7a0089f83c9dad6`에 PR #422까지 병합된 main `6a0d7264aee56bf4a799b945f656d2c7d42239df`를 합쳤다. `codex/pr423-kouku-encore-integration` 별도 worktree에서 충돌을 해결했으며 원래 `GB/koukubugfix-bingo` checkout의 저작 데이터와 실행 파일은 교체하지 않았다.

## G01. 보존한 데이터와 소비자

- 동일 P96을 사용하던 두 패턴을 분리했다. 기존 `빙고_반복전투`는 P96, incoming `앵콜컷신`은 P97이며 앵콜 내부 stage/action/occurrence ID를 함께 변경했다. Sequence 앵콜 P10은 유지했다.
- Action revision 1918, Sequence revision 120, WorldSequence revision 2143이다. 기존 P75/Sequence P9 최종 엔딩과 자막·사운드, 빙고 폭탄·망치 사운드, red_flip instance를 보존했다. Independent에는 망치 경고 화살표와 앵콜 fade를 모두 남겼다.
- authored 최종 감사 119/119 PASS. 기존 배열의 순서와 각 항목의 전체 내용, incoming 앵콜의 camera/fade/world/template/model/material 참조를 비교했다. World의 최종 resource/template/instance 수는 461/277/334다.
- PR #422/main에서 GATE1이 Product에 없던 원인은 P78의 명시적 lifetime 누락이었다. Stage 합 17,165ms보다 Logic 끝 22,114/22,980ms가 길어 flow 전체가 제외됐다. #423의 `durationMs: 22981`을 유지했으며 병합 후 Encounter에는 GATE1/GATE2/GATE3/BINGO가 모두 생성됐다.
- PR #422는 앵콜 배우 애니메이션을 추가했다. 별도 신규 death 패턴 추가 근거는 없으며 BINGO 사망 후 자동 재생은 기존 Sequence P9다. 새 P97/P10을 자동 사망 전환으로 연결하는 변경은 이번 병합에 포함하지 않았다.
- 자동 병합된 C++의 베른 trigger hold, 컷신 주변 숨김, 자막·사운드 수명, 빙고·관문 전환과 Shared packet 소비자를 확인했다. 병합 때문에 추가할 C++ 수정이나 새 translation unit은 없었다.

## G02. 게시와 리소스

`Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon -ExpectedKoukuSaydonSourceRevision 1918`로 Product, 쿠크 맵, world, balance를 게시했다. PASS, 303,409ms. Encounter와 patternbindings의 sourceRevision은 모두 1918이며 Product 87패턴/8번들/474스테이지, presentation binding 592개다. WorldSequence authoring/runtime의 JSON 내용은 동일하다.

`Publish-Compositions.ps1 -Mode Publish`도 PASS다. 이 4개 facade/reference product는 기존 `runtimeEligible=false` 계약이며 현재 Action/Sequence의 직접 실행 게시 증거와 구분한다. source manifest와 receipt의 hash/size 검사는 모두 통과했다. Valtan의 실행 내용은 main과 같고 변경은 병합된 공용 source manifest 반영이다.

격리 폴더의 긴 경로 때문에 첫 쿠크 게시의 Map 단계가 실패했고 기존 출력은 transaction rollback으로 복원됐다. 임시 `R:\LostArk` 경로로 동일 publisher를 재실행해 통과했다. 검증기나 경로 안전성 검사를 완화하지 않았다. R 드라이브 최상위를 repository로 사용한 중간 시도는 경계 검사에서 출력 전에 거절됐다.

앵콜 배우의 Git 제외 리소스 18개, 53,104,168B가 로컬 Resources에 존재한다. 초기 검사에서 GBResources에 없던 파일을 해당 폴더에 복사하고 hash를 검증했다. 이후 사용자가 추가 리소스를 다시 받았다고 확인했다. 리소스 바이너리는 PR에 추가하지 않았다.

## G03. 검증 상태

변경 JSON/XML 20개 parse, 원본/생성물 revision과 4개 raid gate, stable ID 보존·참조 검사, staged `git diff --check`는 PASS다. Engine/Shared/Server Debug Build도 PASS다. Client Product Build, Python 및 focused Server 계약 검사 결과는 완료 후 이 항목에 기록한다.

원래 checkout의 LAN 설정은 server-host로 완료됐다. 격리 경로 전용 EXE 방화벽 규칙은 관리자 권한이 필요해 갱신하지 않았으며, 이 경계는 컴파일·headless 계약 검사와 별개다. Client/UI와 실제 화면·청취 검증은 수행하지 않았다.

검증 로그와 영수증은 작업 worktree의 `out/Pr423Merge`에 저장했다. 앞서 논의한 신규 collider/knockback, 주사위 SOUND 추가, Title Category 구현은 이번 PR 충돌 해결에서 새로 구현한 항목이 아니다.
