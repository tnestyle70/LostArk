# PR #423와 앵콜 컷신 병합 결과

## G00. 기준과 작업 범위

PR #423의 `b1a84dc45e451ee9ea8830e1d7a0089f83c9dad6`에 PR #422까지 병합된 main `6a0d7264aee56bf4a799b945f656d2c7d42239df`를 합쳤다. `codex/pr423-kouku-encore-integration` 별도 worktree에서 충돌을 해결했으며 원래 `GB/koukubugfix-bingo` checkout의 저작 데이터와 실행 파일은 교체하지 않았다.

## G01. 보존한 데이터와 소비자

- 동일 P96을 사용하던 두 패턴을 분리했다. 기존 `빙고_반복전투`는 P96, incoming `앵콜컷신`은 P97이며 앵콜 내부 stage/action/occurrence ID를 함께 변경했다. Sequence 앵콜 P10은 유지했다.
- Action revision 1918, Sequence revision 120, WorldSequence revision 2143이다. 기존 P75/Sequence P9 최종 엔딩과 자막·사운드, 빙고 폭탄·망치 사운드, red_flip instance를 보존했다. Independent에는 망치 경고 화살표와 앵콜 fade를 모두 남겼다.
- authored 최종 감사 119/119 PASS. 기존 배열의 순서와 각 항목의 전체 내용, incoming 앵콜의 camera/fade/world/template/model/material 참조를 비교했다. World의 최종 resource/template/instance 수는 461/277/334다.
- PR #422/main에서 GATE1이 Product에 없던 원인은 P78의 명시적 lifetime 누락이었다. Stage 합 17,165ms보다 Logic 끝 22,114/22,980ms가 길어 flow 전체가 제외됐다. #423의 `durationMs: 22981`을 유지했으며 병합 후 Encounter에는 GATE1/GATE2/GATE3/BINGO가 모두 생성됐다.
- PR #422는 앵콜 배우 애니메이션을 추가했다. 별도 신규 death 패턴 추가 근거는 없으며 BINGO 사망 후 자동 재생은 기존 Sequence P9다. 새 P97/P10을 자동 사망 전환으로 연결하는 변경은 이번 병합에 포함하지 않았다.
- 자동 병합된 제품 C++의 베른 trigger hold, 컷신 주변 숨김, 자막·사운드 수명, 빙고·관문 전환과 Shared packet 소비자를 확인했다. 제품 runtime 추가 수정이나 새 translation unit은 없으며 아래 G03의 기존 검사 기대값만 교정했다.

## G02. 게시와 리소스

`Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon -ExpectedKoukuSaydonSourceRevision 1918`로 Product, 쿠크 맵, world, balance를 게시했다. PASS, 303,409ms. Encounter와 patternbindings의 sourceRevision은 모두 1918이며 Product 87패턴/8번들/474스테이지, presentation binding 592개다. WorldSequence authoring/runtime의 JSON 내용은 동일하다.

`Publish-Compositions.ps1 -Mode Publish`도 PASS다. 이 4개 facade/reference product는 기존 `runtimeEligible=false` 계약이며 현재 Action/Sequence의 직접 실행 게시 증거와 구분한다. source manifest와 receipt의 hash/size 검사는 모두 통과했다. Valtan의 실행 내용은 main과 같고 변경은 병합된 공용 source manifest 반영이다.

격리 폴더의 긴 경로 때문에 첫 쿠크 게시의 Map 단계가 실패했고 기존 출력은 transaction rollback으로 복원됐다. 임시 `R:\LostArk` 경로로 동일 publisher를 재실행해 통과했다. 검증기나 경로 안전성 검사를 완화하지 않았다. R 드라이브 최상위를 repository로 사용한 중간 시도는 경계 검사에서 출력 전에 거절됐다.

앵콜 배우의 Git 제외 리소스 18개, 53,104,168B가 로컬 Resources에 존재한다. 초기 검사에서 GBResources에 없던 파일을 해당 폴더에 복사하고 hash를 검증했다. 이후 사용자가 추가 리소스를 다시 받았다고 확인했다. 리소스 바이너리는 PR에 추가하지 않았다.

## G03. 검증 상태

변경 JSON/XML 20개 parse, 원본/생성물 revision과 4개 raid gate, stable ID 보존·참조 검사, staged `git diff --check`는 PASS다. pushed SHA `7ac5ee167df08792d2ec3f527531e9882648d430`의 authored 추가 감사 94/94 PASS이며 C++ 자동 병합과 Composition 산출물 감사에서도 병합 결함은 없었다.

- Engine/Shared/Server/Client Debug Product Build PASS, 누락 runtime input 0개. 빌드 receipt는 `out/BuildPipeline/runs/20260920T123054226Z-debug-product.json`이다. 기존 shader·인코딩 경고는 남아 있다.
- `Owner Server` 게시 PASS, 386,821ms(앞선 owner lock 대기 203,802ms 포함). 쿠크뿐 아니라 베른 등 Server world/navigation과 시작 시 필요한 catalog를 같은 저작 원본에서 준비했다.
- Python focused 23/23 PASS, 3.969초. lifetime·Logic 행·world 참조·Sequence import·camera/sound·flow order·raid flow·subtitle·현재 Encounter admission을 검사했다. native WModel read 0회, Composition 검사 전후 SHA 동일. 처음 실행한 전체 projection 모듈은 실제 원본 root-motion bake를 반복하는 광역 검사여서 중단했으며 통과한 것으로 기록하지 않는다.
- 실제 Debug Server의 bingo 70/70, card-maze 69/69, kouku-object-overlap 88/88, kouku-bundle 94/94 PASS.
- 최초 `--debug-teleport-contract-test`는 Mario4 T2 착지 방향 단언 1건이 실패했다. main이 runtime `rightSign`과 초기 lane 검사를 -1로 바꿨으나 기존 착지 검사는 계속 +normalize(T5−landing)을 기대했다. 해당 기대값과 설명만 교정했다. T5 방향은 `(0.990494,-0.137559)`, 올바른 오른쪽은 `(-0.990494,0.137559)`이고 카메라 오른쪽과 내적은 `+0.999415`다. runtime/data, 존재·길이 조건과 오차 허용값은 유지했다. 해당 Server TU 재컴파일·링크 PASS. 동일 관문 전환 검사 재실행은 7,909/7,909 PASS, 150,208ms다. 5개 focused Server 검사 합계는 8,230 PASS / 0 FAIL이다.

원래 checkout의 LAN 설정은 server-host로 완료됐다. 격리 경로 전용 EXE 방화벽 규칙은 관리자 권한이 필요해 갱신하지 않았으며, 이 경계는 컴파일·headless 계약 검사와 별개다. Client/UI와 실제 화면·청취 검증은 수행하지 않았다.

검증 로그와 영수증은 작업 worktree의 `out/Pr423Merge`에 저장했다. 앞서 논의한 신규 collider/knockback, 주사위 SOUND 추가, Title Category 구현은 이번 PR 충돌 해결에서 새로 구현한 항목이 아니다.

## G04. PR 반영

통합 PR #424는 사용자가 2026-09-20 21:34:11 KST에 병합했다. main merge SHA는 `8bce13181a3477eff2325fc3e7717ddeae206477`이며 원래 #423도 자동으로 MERGED 처리됐다. 이 시점에 로컬에 남아 있던 Mario4 검사 기대값과 이 PLAN/RESULT의 최종 검증 기록 3개 파일은 별도 마무리 변경으로 분리한다. 제품 runtime과 authoring은 #424에 반영된 상태 그대로다.
