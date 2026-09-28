# 발탄 Play Pattern 표현·카운터·추적 수정 결과

## G00. 원인과 적용 범위

초기 게시 generation의171개 artifact는 저장 원본 hash와 일치했다. Product 미승격이
처음의 누락 원인은 아니었다. Floor Wipe는 최종 피해로 마지막 플레이어가 사망한 다음
tick에 stable-ID 재생이 중단돼99/101ms cue와 회복 동작을 놓쳤다. 최종 타격 확정 뒤의
SECOND_SMASH/RECOVERY 및 HIGH_JUMP LAND/RECOVERY를 완주하도록 연결했다.

추적 도끼의 경고·폭발은 공중 보스 root에 붙어 있었다. 경고 decal depth6m에 대해 보스는
9m 위에 있고, LAND 폭발은 실제 착지267ms보다 이른201ms에 공중 위치를 snapshot했다.
Server의 고정 착지점을 Shared snapshot과 기존 V1 world-root 경로로 전달하고 AIRBORNE와
LAND 두 cue를 `pattern.landing.snapshot`으로 바꿨다. 사용자가 수정 대상을 추적 도끼로
정정해 SIX_PIZZA STEP_03은 기존 root/follow를 보존했다. 현재 실행 protocol은119다.

최종 실제 snapshot 검증에서 새 landing 계약이 요구하는 pattern 시작 tick의 연결 누락을
추가로 찾았다. Valtan은 기존 action 시작 tick만 기록하고 pattern 시작 tick은 0으로 남아
새 landing snapshot이 codec에서 거절됐다. BeginPattern에 시작 tick을 기록하고 완료·사망에
정리하는 lifecycle 연결을 추가했다. codec 검증을 완화하거나 테스트 좌표를 바꾸지 않았다.

SIX_PIZZA의 지연 장판은 source 컷씬에서 owner 전체 삭제가 먼저 일어나 보이지 않았다.
명시적 유한 tail만 숨김·시계 유지로 바꾸고 일반 몸체 cue와 명시 Stop 정리는 보존했다.
Preview의0.7초 공백은 별도 notify 수명을 그대로 옮긴 데이터였다. 사용자 요청대로 첫
표시11초부터 cue 종료19.033초까지 연속 표시하며 중복 요소를 비활성화했다. 3시·9시
COMBO_STEP_10에 직접 연결돼 있던 피자 V1 cue 두 개만 제거했다.

## G01. 저장본·카운터·사운드

사용자가 저장한 TRIPLE_COUNTER FAIL_3의 독립891ms cue를 보존했다. 게시 검사에 걸린 것은
그 cue가 아니라 기존 V2 binding의 저장1169ms와 템플릿900ms 불일치였다. exact occurrence
timing override를 두 validator가 공유하도록 연결해1169ms만 인정하며 gameplay hit900ms와
다른 binding 검사는 유지했다. 임의900ms 복원이나 범용 검사 제외는 하지 않았다.

TRIPLE_COUNTER와 TRASH의 정확한 counter stage에 Q/W/E/R 명중 및 활성 COUNTER guard의
방향 예외를 연결한다. 실제 명중·거리·finite·window 검사를 보존하고 Server 성공 event와
기존 GROGGY_FOLLOWUP 전이를 사용한다. 일반 타격과 다른 패턴의 방향 조건은 보존한다.

추적 도끼 반복 충격음의 실제 cue는 `Valtan.combatobjectsoundcues.json`의
`cue.sound.valtan.combatobject.high-jump.target-axe.hit.01`이다. Server hit와 V2 visual hit는
1200ms로 같으나 ProjExp1 네 WAV의 -48dB 최초 신호가61~312ms로 달랐다. 해당 cue만
playbackOffsetMs50을 설정했다. 이는 원음 앞50ms를 건너뛰는 보정이며 Server hit를 앞당기는
변경이 아니다. Animation Tool의 Server Semantic Event Sound → WAV Start Offset(ms)에서
수정할 수 있고 Preview Impact WAV도 같은 값을 소비한다. 실제 청감은 사용자가 확인한다.

피자는 첫 sector 종료 1초 전, 모아치기 두 변형과 3시·9시는 검격 시작 1초 전에 추적을
멈춘다. 5패턴 17개 Stage에 endMs와 responseScale을 저장하고 기존 Showtime의 fixed
tick 회전 helper에 1/3 응답 배수를 사용한다. 다음 Stage의 중앙 FacePoint가 고정 방향을
덮어쓰지 않도록 같은 저작 aim 경계에서 처리했다. Server와 Preview가 같은 계산을 사용한다.

## G02. 확인한 검증

- Shared protocol119 전체 하네스1338 PASS, failure0.
- Client landing authoring native6/6, cinematic tail native21, 실제 effect codec의60Hz 표본482개에서
  후보 장판 누락0개(이전299개). GPU 화면 검증은 아니다.
- 착지 Python pipeline3, Composition8, exact timing29 검증 PASS.
- 사운드 Document native27, 두 Python consumer9 검증 PASS.
- 실제 Server/Preview 회전 함수와 Shared helper native931항목, stage aim Python3 검증 PASS.
- 최종 게시 generation `bf3fd23179ecf53799bec3c30426b46e716c7ce550473bda557bdc54a9ccdd54`의
  artifact171/171이 현재 파일 hash·크기와 일치한다. 현재 저장본으로 재계산한 generation도 같다.
- HIGH_JUMP 두 cue의 월드 착지 anchor, SIX 착지 root/follow 보존, FAIL_3 독립891ms cue,
  terrain의 잘못된 sector 두 cue 제거를 최종 게시 소비 경로에서 확인했다.
- GameplayBalance Publish 성공 로그는 `out/ValtanPatternPresentation20260928/publish-gameplay-final.log`.
- Composition과 V2 Publish도 PASS. 로그는 같은 폴더의 `publish-composition-final.log`와
  `publish-v2-final.log`다. 저장 원본·게시 파일·현재 실행 메모리의 갱신은 구분한다.
- Engine·Shared·Server·Client Debug Product 빌드 PASS. 최초 최종 통합 기록은
  `out/BuildPipeline/runs/20260928T145205781Z-debug-product.json`이며 시작 tick과 World 메뉴
  연결까지 포함한 최종 재빌드는 `out/BuildPipeline/runs/20260928T150526625Z-debug-product.json`이다.

기존 lifecycle 전체 실행은183 PASS/5 FAIL이었다. 네 실패는 저장된 Ghost followup, Dash6897ms,
phase3 COMBAT_OBJECT_VOLLEY와 이전 고정 기대값의 불일치다. 나머지 HIGH_JUMP snapshot 실패는
위 pattern 시작 tick 누락을 수정한 뒤 실제 snapshot 집중 재실행에서 통과했다. 기존 광역검사를
다시 실행하거나 네 기대값을 변경하지 않았으므로 이 전체 실행을 PASS로 기록하지 않는다.
해당 로그는 `out/ValtanPatternPresentation20260928/server-lifecycle-test.log`다.

## G03. 최종 검증 기록과 화면 경계

최종 Debug Product 빌드와 GameplayBalance·Composition·V2·Map Effects 게시를 완료했다.
실제 Server의 `--valtan-presentation-contract-test`는52 PASS, failures0, exit0이다.
마지막 플레이어가 사망한 floor wipe의 끝 동작·회복·완료, prehit 사망/Stop/퇴장 경계,
HIGH_JUMP late/coalesced snapshot의 공중·착지·회복·완료, 고정 착지점과 pattern 시작 tick,
두 카운터 패턴의 앞뒤·좌우 명중/거리/실패/투사체/guard/파란 문구 event/그로기를 확인했다.
로그는 `out/ValtanPatternPresentation20260928/server-presentation-final-test.log`다.

World 구슬·바훈투르·지형의 실제 Server 검사49 PASS와 protocol1338 PASS도 완료했다.
별도 [World Ether 결과](2026-09-28_VALTAN_WORLD_ETHER_RESULT.md)에 위치와 게시 계약을 기록했다.
최종 게시171개 hash와 연속 sector482개 표본 근거는
`out/ValtanPatternPreviewParity20260928/publish-audit/final-published-closure-audit.json`이다.
변경 JSON9개 parse와 `git diff --check`를 통과했다.

Client/UI 자동 실행은 하지 않았다. 실제 장판·착지 이펙트와 무적/카운터 문구, 도끼 청감은
Server와 Client를 같은 최신 protocol119 빌드로 다시 실행해 사용자가 확인한다. 설치 파일의
게시 완료를 실행 중 메모리 Reload나 GPU 화면 확인으로 기록하지 않는다. 요청한 구현과
빌드·게시 작업은 완료했으며 남은 작업은 사용자 화면 확인과 구슬의 최종 위치 조절이다.
