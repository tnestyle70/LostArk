# 쿠크 저작·패턴·렌더링 정본 구현 결과

## G00. 상태와 보존 경계

authoring 설치와 revision2346 런타임 게시를 완료했다. 명시적 빙고 Flow 연결과 실행 중 occurrence 전환 수정을 포함한 Product11 정상 Debug Build가 PASS이며, 변경 관련 Product/Raid/Valtan lifecycle/skill stages native 검사와 Client admission도 모두 exit0 PASS다. 광역 전체 contract suite의 부분 실행은 G07에 별도로 기록하며 전체 PASS로 확대하지 않는다. 시작 브랜치 `codex/kouku-timeline-local-preview`의 기존 미커밋 변경을 보존했다. 자동 Client 실행·Reload·화면 캡처는 하지 않았다. UI 동작과 실제 화면의 최종 확인은 아래 소스·수치 검증과 구분한다.

## G01. 공통 규칙과 도구

AGENTS.md와 CLAUDE.md에 팀장이 조율한 모든 렌더링 옵션의 정본·변경 권한을 기록했다. 현재 Mario1~4의 FXAA/AA OFF를 임의로 복원하거나 덮어쓰지 않는다. 이 작업의 후보는 RenderingProfiles를 수정하지 않는다.

WorldObjectTool의 단일 Motion과 합성 Motion transport에 `Play at Player`를 추가했다. 기존 previewAtCharacter 경로를 사용하며 저장된 placement Transform은 변경하지 않는다. 기존 Composition placement로 열면 원점에 재생되던 상황을 명시적으로 바꿀 수 있다.

## G02. 피해 표시와 전멸 판정

Shared protocol111은 `DAMAGE_HIT_FLAG::ABSORB`를 추가한다. 실제 shield 소비량만 흡수 이벤트로 보내고, MainApp은 파란 `흡수` 문자열로 표시한다. 부분 흡수는 잔여 실제 HP 피해와 분리하며 CombatHUDViewModel의 outgoing DPS에 흡수량을 합산하지 않는다. wire 검증은 양수 흡수량·stagger0·counter없음·card symbol없음을 요구한다.

Server의 `bEncounterWipe`는 명시적인 레이드 실패 판정이다. 보호막·개인 무적·카운터·피해 경감·죽음 방지·붙잡힘·낙하 상태와 관계없이 살아 있는 참가자의 HP를0으로 만들고 기존 사망 정리 경로를 사용한다. 일반 피해 판정은 이 플래그를 사용하지 않는다.

## G03. 최종 빙고 요청

일반 폭탄은 입장 후30초에 첫 표식, 이후20초 간격이다. 표식6초→공백2초→바닥폭탄4초이며 세 번째 폭탄도 표식 후12초에 폭발한다.

세 번째 표식에 시작하는 작은 Parent의 순서는 이동→블랙홀 첫 클립5.4초→메두사6.428초→블랙홀 본 클립·이펙트13초→폭발이다. 블랙홀 폭발은 Parent 시작 후24.828초다. 전체 빙고 반복은 별도 Flow로 풀고 보드·폭탄은 encounter가 계속 소유한다.

`BINGO_COMPLETED_LINES` Duration은 세 번째 폭탄의 실제 폭발·판 뒤집기 결과를 한 번 판정한다. 빨간 가로·세로 합계3줄 이상이면 `PLAYER_INVULNERABILITY` Result로30초 무적을 준다. 대각선은 제외한다. 블랙홀 폭발은 성공이면 보스13줄 감소·생존, 실패면 전원 전멸이다.

후속 확정에서 자동 시작 시점은 일반3관문 입장이 아니라 빙고 페이즈 진입이다. 기존 동시 actor Bundle을 바꾸지 않고 Flow의 `bingoSpecialPatternId`로 일반 반복과 P107을 같은 전투 묶음에서 연결한다. Boss Tool의 Pattern Flow에서 특수 Parent를 선택·저장하며 제품에는 유효한 참조가 필수다. 매 세 번째 머리 표식에 일반 occurrence를 정리하고 특수 Parent를 실행한 뒤 중단했던 일반 entry를 처음부터 재생한다.

P107은 저작에서 자식3개지만 제품에서는10개 stage의 fixed timeline으로 합쳐진다. 기존 전역 검색이 runtime ParentChildren를 요구해 이 Parent를 놓치는 실제 문제가 있었고 exact Flow 참조로 고쳤다. source 순서는5.4초 첫 클립+6.428초 메두사+13초 블랙홀이며 폭발24.828초, 회복 포함 완료32.628초다.

## G04. 설치된 데이터와 공통 수치

삭제된 레이저 BOX7개는 HEAD 저장본의 치수를 복원해 현재 이펙트 시계와 연결한다. 레이저 넉백6m, 바주카12m로 마지막 요청을 반영한다. 노란 장판 폭발은 P79의 현재 저장된 피해·넉백·창을 P119/P122에 공유하며 각 맵의 위치는 유지한다.

후보 생성·필드 병합·검증 코드는 `out/KoukuAuthoring20260925/integrate.py`에 있다. source revision과 stable ID로 최신 디스크 변경을 보존하며 writer lock·hash 재검사·백업·원자적 교체를 사용했다. 13개 문서 설치는 `out/KoukuAuthoring20260925/applied.receipt.json`, 마지막 바주카 표시명 교정은 `label-correction.receipt.json`에 기록했다. 초기 통합 revision은2345이며, 후속 Flow 참조 필드만 같은 CAS 절차로 추가한 최신 revision은2346이다. `bingo-flow-link.receipt.json`과 transaction `kouku-raid-ca1d88465e164e5ebe52166cc474b9b5`에 변경·백업을 기록했다. 백업은 각 receipt의 transactionPath에 있으며 전체 파일을 이전 저장본으로 덮어쓰지 않았다.

검증 overlay에서115개 패턴과 GATE1/GATE2/GATE3/BINGO의 네 Flow가 모두 게시 가능한 상태였다. 작업 전 런타임에는 GATE1 Flow가 빠져 있었고, P7 룰렛 WORLD 수명을 실제 피해 창 끝까지 맞춘 최종 후보는 이를 통과했다. 3/20/45/51/110/111의 기존 빈 초안은 제품 패턴으로 승격하지 않았다.

`Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon -ExpectedKoukuSaydonSourceRevision 2345` 최종 실행은 네 domain 모두 PASS다. 패턴115개·Stage577개, 맵 placement3369개, 월드 및 gameplay bootstrap82,441행/23,883,405bytes를 게시했다. 실행 로그는 `out/KoukuAuthoring20260925/publish-complete.log`, domain receipt는 `out/BuildPipeline/receipts`에 있다. 게시 당시 상한131,072행/67,108,864bytes 이내다.

후속 Flow 참조를 반영한 `-ExpectedKoukuSaydonSourceRevision 2346` 게시도 exit0으로 완료했다. product/world/gameplay는 PASS, map은 정본 일치로 REUSED다. 총504,629ms이며 gameplay는82,442행/23,883,456bytes다. `RAIDBINGOSPECIAL BINGO KAKULSAYDON_G1_PATTERN_107`이 정확히 한 행으로 게시됐고 원본 revision2346 hash가 설치 receipt와 일치한다. 로그는 `out/KoukuAuthoring20260925/publish-flow-link.log`다.

| 요청 | 반영한 동작 |
|---|---|
| 공 분열·개별 폭발 | 5회→3회, g0..g3 총15개 공의 독립 폭발 Collider와 기존 첫 bounce를 포함한16개 피해 창. 초기 BOX half extent1m·최대HP10%는 PROJECT_TUNED 수치 |
| 불 접촉과 인형·공 | 일반61개 BOX 접촉100HP/100ms, 공10개·인형 입20개 접촉10HP/100ms 및 광기3%/100ms. 실제 효과 수명과 damageable WORLD의 생존에 연결 |
| Box Detail | Duration 반복 간격, 연결된 Success/Fail/Timeout Result 수치, 카드병정3종 수·nav 반경, 카드낙하 min/max 소수 배율을 저장·다시 열기에 연결 |
| 애니메이션·앵커 | 백스텝9개 Collider가 실제 본 TRS를 따름. P81/P118 rolling ball은 BODY 기준이며 G3의 실제 Saydon scope를 분리 |
| 타겟·복제 | 순간 Trigger는 최근접 생존자를 바라봄. 추적은 몸체·플레이어 반경+0.5m 이내에서 다음 패턴으로 진행. 십자 화염은 실제 본체·복제체별 접촉 틱 |
| 주사위·회전카드 | 원본 배출 축180도, HEART→SPADE→CLUB→DIAMOND Trigger. 이전 생성 Duration73 삭제·속박86 보존. 접촉 효과×1.5/Y+0.5, 보라 잔상5개 복구 |
| 시선·메두사·재질 | 노란 시선의 preview Area 연결, 파란 V2의 수명을 빨간 장판 끝과 일치. 메두사 붉은 외곽 복구·root Y 하강 억제. 바주카는 원본 MODEL238 재질 |
| 대형 세이튼 바람 | 기존10% 피해·16m·1500ms 등을 Result에서 편집. 이름에 의존한 강도 분기를 만들지 않음 |

실제 설치된 모델·본·셰이더·이펙트의 상세 측정과 검증은 아래 분담 RESULT에 있다.

## G05. 확인한 검증

- 정상 Debug Product Build: Engine/Shared/Server/Client 컴파일·링크·배포 PASS. 주요 변경 빌드는 `out/BuildPipeline/runs/20260924T190839023Z-debug-product.json`, 본 회전 검증은 `20260924T191256373Z-debug-product.json`, 최종 용량 계약까지 포함한 빌드는 `20260924T192958176Z-debug-product.json`이다. Client 주요 빌드는 OBJ101·CSO45, 최종 용량 빌드는 Client OBJ173·CSO0이다. Clean/Rebuild·shader skip을 사용하지 않았다. 실행 파일은 `Client/Bin/Debug/Client.exe`, `Server/Bin/Debug/Server.exe`다.
- 마지막 Engine FXC14개의 재실행은 첫 Engine 빌드 도중 공통 LightPrograms include를03:46:03에 다시 저장한 결과다. 전후 command/include 기록에서 toolset·옵션·출력 경로는 동일했고 마지막 Client FXC는0개였다. 마지막 Client의 CSO14개 변경은 Engine 배포 복사다. source-ready 통보는 내용·개행 정리까지 끝낸 뒤 해야 한다.
- 후속 빙고 Flow 연결과 교정한 테스트를 포함한 Product10 정상 Debug Build도 PASS다. receipt는 `out/BuildPipeline/runs/20260924T201837120Z-debug-product.json`, 로그는 `product-tenth.log`다. Engine/Shared OBJ0, Server OBJ37, Client OBJ83, 전체 CSO0이며 링크·배포도 통과했다. 앞선 Product9의 테스트 weak_ptr 직접 접근 오류는 lock()으로 교정했다.
- 새 Flow optional 저장/제품 필수 참조, 같은 gate·boss·actor, finite Parent·단일 폭발, 일반 entry/Bundle 중복 거절과 PS exact supplemental row를 포함한 Python15 tests PASS. 최신 저작 native harness 증분 Build와 `--kouku-fixed-damage-contract`도 PASS다. 실제 P107 Flow 참조의 저장·재열기, 잘못된 참조 거절 시 이전 정상 문서 보존, 미지정 draft 허용과 live source bytes 보존을 확인했다. 실행 로그는 `out/KoukuAuthoring20260925/authoring-flow-final.log`다.
- NetworkProtocolHarness Debug Build 및 실행: `failures : 0`. 새 흡수 이벤트의 wire 왕복·부적절한 bookkeeping 거절을 포함한다.
- 최신 ValtanPatternAuditionServiceHarness Debug Build 및 `--kouku-fixed-damage-contract`: PASS. Result 수치·공유 정의 COW·100ms Duration·카드병정 소수 반경·빙고 threshold3 저장/11 거절·30초 보호 저장/다시 열기·실패 시 draft 보존을 검사한다. 로그는 `out/KoukuAuthoring20260925/authoring-final-build.log`와 `authoring-final.log`다. 큰 기존 Workbench 번역 단위의 컴파일을 위해 하네스에 `/bigobj`를 추가했다.
- Server의 Bingo/반복 접촉/주사위/bundle focused 검증은 모두 `failures0`이었다. 후속 게시 데이터의 전체 `--contract-test`는 G07의 부분 실행 기록을 따르며 전체 PASS로 간주하지 않는다. 최종 검증은 변경과 직접 관련된 Product/Raid/Valtan lifecycle/skill stages selector로 구분한다.
- 최종 변경 목록 JSON/XML24개 parse와 `git diff --check` 통과. 흡수 경로의 실제 글꼴에 `흡`/`수` 두 글자가 존재한다.
- 첫 게시에서 BOSS 본 track의 key yaw를 identity로 강제하던 기존 PowerShell/Server 검증과 새 본 회전 계약의 충돌을 발견했다. 두 소비자는 normalized planar quaternion을 허용하고 exact clock·identity baseline·visible·unit scale 검증을 유지하도록 교정했다. 부적절한 geometry6종 거절과 실제2345의27개 region·2,850개 본 key 검증 PASS. 실패 게시의 임시 파일 점유로 남았던 Encounter도 CAS/백업을 통해 이전 게시본으로 복구한 뒤 재게시했다. `publish-recovery.receipt.json`에 복구 증거가 있다.
- 전체 lifetime의 인형 입 본 궤적 등으로 bootstrap82,441행이 필요해 기존65,536행 한도에서 게시가 거부됐다. 기존150tracks는 그대로이며 증가41,432행 중 인형39,132행이 대부분이다. 임의 key 삭제 없이 공통131,072행·64MiB의 유한 용량으로 연결했다. 개별4096keys·정규화 geometry·정확한 행수·후행 행 거부와 Debug draft16MiB를 유지한다. Python 기존/경계4tests PASS. 실제 게시본 Client admission과 Server의 새 용량 경계 검사는 통과했다.
- 최신 Client presentation generation admission 전체 검사: PASS(exit0). 제품 82,441행의 exact receipt/currentness/closure와 실패 시 보존을 검증했다. 최초 실패는 테스트 fixture만 이전 `Data/Effects/V2/Bindings` 경로를 사용한 탓이며 실제 typed `Data/Valtan/Published` 경로로 5곳을 교정했다. 제품 코드·데이터·검증 강도는 바꾸지 않았다. 성공 로그는 `out/KoukuAuthoring20260925/admission-final-fixed.log`다.
- 실제 Box Detail draft producer의 최대 P92 dependency closure 11개를 실행했다. `gameplay.rows` 6,957,898bytes/23,544행, `encounter.json` 14,716,566bytes로 기존16MiB 안이며 SHA/admission도 PASS다. 전체23MiB bootstrap을 그대로 draft로 넣는 기존 native fixture와 실제 전송 범위를 구분했다. 근거는 `out/KoukuAuthoring20260925/draft-largest-request/output/admission.json` 및 `prepare.log`다.
- revision2346 게시 후 Client presentation admission, Valtan lifecycle 및 skill stages focused 검사는 모두 exit0 PASS다. 로그는 `admission-flow-final.log`, `server-valtan-focused-final.log`, `server-skill-stages-final.log`다. Kouku Product는 `server-product-flow-final.log`에서 exit0/failures0이며, 마지막 전환 버그 수정 뒤 Product11의 Raid도 `server-raid-flow-interrupt-fixed.log`에서 exit0/failures0으로 완료했다. Product 316개와 Raid 1,712개 assertion이 통과했다.
- 실제 Client 화면, 최종 시각 품질, G1 전환 프레임 시간은 아직 사용자 확인 전이다.

## G06. 화면 확인과 런타임 경계

G1 전환은 기존 prepared clone에 popup-book endpoint pose를 숨긴 상태로 미리 Sample하고 실패 시 pending owner를 rollback하도록 변경했다. `WorldSequence.HiddenPose.Prewarm`, `Kouku.GateObjects.Prepare/Activate/Commit`, `Kouku.Cinematic.StopPreview` profiler scope로 전환 구간을 분리했다. 사용자 F7 capture 없이 프레임 멈춤의 원인이나 해결을 확정하지 않는다. 기존 Reset_ForReuse와 commit Seek 비용은 여전히 실측 대상이다.

게시된 파일과 실행 중 도구의 메모리 draft, 실행 중 Server 상태는 별개다. 최종 빌드·게시 뒤 새 Server/Client로 실행하고 저작 도구는 사용자가 저장된 변경을 확인한 뒤 Reload해야 한다. 이 작업은 UI Reload나 미저장 편집 폐기를 자동 수행하지 않았다.

상세 구현·수치 근거:

- [Logic·Box Detail 결과](2026-09-25_KOUKU_RESULT_TUNING_IMPLEMENTATION_RESULT.md)
- [Server 패턴 결과](2026-09-25_KOUKU_PATTERN_RUNTIME_IMPLEMENTATION_RESULT.md)
- [이펙트·재질·본 측정 결과](2026-09-25_KOUKU_EFFECT_RECOVERY_RESULT.md)

## G07. 오래된 테스트 전제 재검토

광역 `--contract-test`는 관찰32fail 상태에서 변경 관련 focused 검사로 전환하기 위해 중단했다. 전체 PASS로 기록하지 않는다. process stack은 다음 revision loader로 진행 중이었고 마지막 로그의 정체는 redirect stdout 버퍼 때문이었다. 최초 최종 Raid 검사는23fail(standalone18+구형 Bingo Parent 기대5)이었다. 원본 실패 로그를 보존했고 새 focused 결과는 별도로 기록한다.

- 실제 마리오 입장은 CLOWN을 요구한다. positive fixture에 이 상태를 설정하되 NORMAL 등9개 거절·범위·시계 경계는 유지했다. 이 검사는 게이지 축적부터 변신까지 전체 흐름을 증명하는 검사가 아니다.
- 현재 게시된 Lance resource10000/Counter34580 비용410을 catalog에서 읽어 fixture를 초기화한다. 기존 resource100은 시작 자체가 거부되는 상태였다. Counter 전환·무피해·groggy 검증은 유지한다.
- G1/G3 boss placement는 disabled다. 명시적 생성과 all-despawn을 구분하고 후자는 두 동적 boss 모두 제거하며 Esther sentinel을 보존한다. 실제 F1 이동 좌표(-2.45,1.32,740.37)로 충돌 검증을 수행한다. enabled NPC 실행 검증까지 했다고 확대하지 않는다.
- standalone restart fixture는 실제 spawn 경로의 `Note_GatePlacementRaised`를 누락했다. source gate·clear 상태 확인을 추가해 현재 관문을 먼저 설정한다.
- P88은 `1마리오_1페이즈`로 실제 자식5개가4.667~41.109초 이어진다. 4.667초 조기 종료 추정은 철회했다. 실제 P88의 자식·P33 후속·8.083~41.109초 입장 창·피해 WORLD를 보존하는 검증과 자식 없는 별도 synthetic entry 검증을 분리했다. 제품 P88 타이밍이나 runtime을 이 추정 때문에 변경하지 않았다.


revision2346 최초 Raid focused 실행에서는 빙고 자동 진입·35개 entry와 반복 tail은 통과했으나 세 번째 표식의 특수 Parent 전환 검사가 실패했다. `Start_KoukuBingoSpecialPattern`의 preflight에 진행 중 boss를 그대로 전달해 `Evaluate_KoukuSaydonPatternAudition`이 Busy로 거절하는 제품 오류였다. 현재 소유된 대상의 복사본을 `Abort_Pattern`으로 취소 후 상태로 만든 뒤 기존 admission을 통과한 경우에만 live occurrence를 정리하도록 수정했다. 이 수정은 Data 재게시를 요구하지 않는다. Product11 정상 Debug Build는 PASS이며 Server OBJ2/링크1, Engine/Shared/Client OBJ0, 전체 CSO0이다. receipt는 `out/BuildPipeline/runs/20260924T203557169Z-debug-product.json`, 로그는 `product-eleventh.log`다. 이 빌드의 Raid 재검증도 exit0/failures0으로 완료했다.

Product11 Raid 재실행은 `server-raid-flow-interrupt-fixed.log`에서 실제 normal entry ACTIVE, 잘못된 revision의 preflight 거절 시 모든 live actor·기존 occurrence·receipt·board·epoch 보존, 실제 3번째·6번째 mark의 특수 Parent 실행·폭발·복귀 검사를 통과했다. 누적 stage 상대 tick은0/162/189/249/314/355/745/751/786/939, 폭발745tick, 완료979tick이며 보스13줄 피해 이벤트를 폭발 tick에 한 번 확인했다. 최종 selector 전체도 exit0/failures0으로 정상 종료했다. 별도 데이터 재게시나 Client/UI 실행 없이 검증했다.

## G08. 2·3관문 체력별 반복 그룹 후속 반영

최신 저장본2346에서 G2/G3 Flow와 P116 시선 occurrence만 병합해 revision2347을 설치했다.
`out/KoukuHealthFlow20260925/install.receipt.json`은 installed이며 transaction은
`kouku-raid-7bab64fcfde249578b6347f1ec073ba1`이다. source SHA256은
`d5708616d6498fd815c9bcdd729c6c6b13b362c5d4d36e53640cf17085f17c7a`다.
writer lock·설치 직전 source hash 재확인·백업·CAS·원자 교체를 사용했다.

G2는79 entries/13 groups다. 140→125줄은 P105→106→105→85→15→21→24,
125줄 등장 뒤는99→86→87→27을 더한11개를 반복한다.125 등장,110 파1빨2,
95 조커,80 카드미로,55 피자,25 조커를 한 번씩 실행한다. 피자는 기존P25 단독 대신
기존Bundle5의 전체 연결을 사용한다. 09-22 결과에 남은 사용자 확정과 현재Action4219713을
대조해 첫 바닥 폭발을P105로 확정했으며, 별도P23을 대신 연결하지 않았다.

G3는66 entries/11 groups다. 마지막 사용자 공통 목록의 P52→119→59→116→118→39→38→
117→43→40을 반복하며155 P88,125 P91,90 P76→P35,80 P92,55 P93을 실행한다.
P116.logic.2는 시작0ms·34ms의 BOSS_TRACK_TARGET Trigger를 사용한다. P119의 Effect4개와
Collider3개는 이미3관문 좌표로 저장돼 추가 이동하지 않았다. 기존Mario Parent children과
P33 후속, G1/BINGO Flow, 다른 모든 패턴은 보존했다.

`mario-children-r2347.json`에서 네 Parent의 내부5개 동작을 이번 상세 요청과 직접 대조했다.
P88 화염파동→1초추적→정면바람→추적→공구르기 카운터, P91 무지개댄스→추적→화염파동→
추적→돌진 카운터, P92 백스텝화염링→추적→알비온→추적→분신소환, P93 노란장판→추적→
십자화염→추적→알비온이 모두 일치한다. source의 Mario 표시명은 일관된
`N마리오_1페이즈`를 유지하며 요청 문장의3/4페이즈 표기를 별도 전투 단계로 만들지 않았다.

G1과 같은 PATTERN_END 전환이며 현재 root와 자동 후속이 완료된 뒤 체력 기믹으로 간다.
새 Server runtime이나 C++ 변경은 없다. 전체 document/Flow 검증, 변환 재적용 동등성,
변환 순서 독립성, 기존 stable row·대기시간 및 무관한 데이터 보존, 전체 entry의 단일
그룹 포함을 독립 검토했다. `combined-final-independent-review.json`에 PASS를 기록했고,
기존 `test_raid_flow_projection.py`15개도 exit0 PASS다(`flow-tests.log`).

2347의 KoukuSaydon owner 게시가 exit0/PASS로 완료됐다(`publish.log`). 전체507,970ms,
product432,100ms·world7,496ms·gameplay66,156ms PASS이며 map은75ms REUSED다.
실행 bootstrap은82,567행/23,897,413bytes이고 SHA256은
`aed91d1cfa77fb7bf8f6efeb74c63af37ff326a2553e69976a6de6d91aa1f415`다.

`published-health-flow-verification.json`의9개 확인 항목이 모두 PASS다. source·transaction
backup·product/gameplay/projection receipt의 hash 연결, 실제 RAIDFLOWSTEP/RAIDFLOWGROUP
전행과 wait·그룹 경계·기믹 순서,4개Mario의 저작/투영/PATTERNPARENTCHILD 보존,
Retail 및 게시된G2 140/G3 180줄, 검증 도중 입력 불변을 대조했다. 이 검사는 파일과
계약 대조이며 실제 Client 플레이를 수행한 것으로 기록하지 않는다.

현재 게시본에 대해 기존 native ValtanPatternAuditionServiceHarness의
`--presentation-generation-admission-contract`도 exit0 PASS다(`client-admission.log`).
실제 Client 데이터 수용 경로의 receipt/currentness/closure 검사를 통과했다.
이번 후속은 데이터 변경이므로 C++·shader 재빌드는 수행하지 않았다. 변경 JSON은 기존
validator·publisher로 parse/검증했고 요청에 해당하는 데이터·문서의 `git diff --check`도 PASS다.

시작 발차기·오허·공3개 연출은 현재1마리오 시작 모션과 같은지 사용자에게 확인 중이다.
기존 추정Action4219902는 투척/Grenade이며 공NPC 소환이 없어 연결하지 않았다.
별도Action4219944의 공소환과 현재P88의Action4219911은 서로 다른 모션이다.
이 미확정 연출을 이번 설치의 완료 항목에 포함하지 않는다. 게시 파일·실행 중 Server·
도구 메모리·사용자 화면 판정은 각각 구분한다.

## G09. Complete Play 분열 공 WORLD 준비 거절 수정

사용자가 GATE1 `Complete Play - Sequences + Pattern Flow`를 실행한 화면에서
`Player 1 preparation failed: Complete Play WORLD group has no admitted motions:
world.object.kouku.saydon.circus.split`을 확인했다. 이는 Server 재생 전 Client 준비
거절이며, 앞선 Server Raid 검사1,712개와 Client presentation generation admission은
실제 Level의 이 WORLD group 해석 경로를 검증하지 않았으므로 화면 오류를 배제하지 못했다.

`Level_KakulSaydonArena.cpp`의 `CompositionWorldMotions`는 실제 모델에 연결된 모션에도
`colliderTracks`가 있으면 빈 목록을 반환했다. 최신 분열 공은 g0~g3에 폭발 Collider를
가지므로 정상 모션4개가 모두 거절됐다. source/runtime Map revision2215의 그룹4개를
대조했으며 해당 조건 제거로 허용이3개에서4개가 되고 circus.split만 새로 통과한다.

공통 helper의 Collider 존재 거절만 제거했다. 준비·실제 Prepare/Play/Span/Pivot가
같은 수정을 소비한다. 기존 WORLD anchor, 실제 단일 model binding, walkableSurface와
combatBody·중첩 group 금지, STOP/LOOP 및 enabled 검사는 보존했다. Collider는 기존
publisher가 Server geometry로 bake하고 Client는 본 검사 및 `_DEBUG` 표시만 수행한다.
이번 변경으로 Client 피해 판정이나 별도 런타임을 추가하지 않았다. C++ UTF-8 BOM과
CRLF를 유지했고 데이터·렌더링 설정·저작 revision·게시 파일은 변경하지 않았다.

검증:

- `python -m unittest Tools.KoukuSaydonPipeline.test_project_kouku_saydon_composition -k world_group -v`: 4 tests PASS. collider 자체를 거절하던 구형 negative를 실제 source/설치본의 positive로 교정했다. 모션4개, 독립 방출15개, 피해 창16개(2+2+4+8), 전체·개별 member 결과 동등성, disabled 제외, 입력 불변을 확인한다. 잘못된 Collider는 실제 bake에서 계속 거절하고 walkable/combatBody 등13종 기존 negative를 유지한다.
- `python -m unittest Tools.KoukuSaydonPipeline.test_world_object_collider -v`: 22 tests PASS. 로그는 `out/KoukuAuthoring20260925/complete-play-world-collider-tests.log`다.
- 독립 실제 데이터·수정 코드 검토 PASS. `out/KoukuAuthoring20260925/complete-play-world-group-review.json`은 source/runtime 일치, 해당 단일 guard 이외 helper 로직 보존, 설치 bootstrap의 P83 피해 창16개를 기록한다. 이 검토를 native/UI 실행으로 간주하지 않는다.
- 변경 범위 `git diff --check` PASS. 신규 C++ 파일과 프로젝트 등록 변경은 없다.
- 첫 Debug Product Build는 기존 재질 생성 헤더의 비ASCII 문자열/CP949 해석 오류로 Client 컴파일에 실패했다. 로그는 `out/KoukuAuthoring20260925/complete-play-world-group-build.log`, receipt는 `out/BuildPipeline/runs/20260925T005450140Z-debug-product.json`이다. G10의 문자열 교정과 최종 빌드가 이 수정도 함께 검증한다.

실제 수정 EXE의 Complete Play 재시도와 화면 판정은 사용자 확인 전이다. Client/UI를
자동 실행·조작하지 않았으며 데이터 재게시는 필요하지 않다.

## G10. 분열 공 원기둥 판정과 받는 피해 기준

사용자 최종 결정은 공의 원형 폭발에 CYLINDER를 적용하고 기존 최대 HP 10%를 유지하는
것이다. 10%는 원작의 공식 피해 계수를 복원한 값이 아니라 PROJECT_TUNED 값이다.
공격력·방어력 차이에 따른 일반 전투 피해와 구분한다.

기존 피해 처리에서 최대 HP 13,200의 공 한 번은 기본 1,320이며 서로 다른 두 폭발은
기본 2,640이다. MAX_HP_PERCENT_DAMAGE는 방어력과 카운터를 무시한다. 시간 무적은
피해를 막고, 받는 피해 증감 버프를 적용한 다음 보호막을 먼저 차감한다. 사망 방지 버프가
있으면 치명 피해에서 HP 1을 남기고 해당 버프를 소모한다. 화면 피해 숫자는 실제 감소한
HP를 사용한다. 같은 폭발의 적중은 보호막·무적에 막혀도 소비되므로 재진입으로 다시
피해를 주지 않는다. 빙고 실패 전멸의 개인 보호 우회 경로와는 별개다.

일반 화염은 현재 고정 100HP/100ms이다. 이전 500HP/500ms와 지속 접촉 시 기본 초당
피해 약 1,000은 같고 판정 빈도가 5배다. 현재 source의 이전 500HP/500ms 정의는 사용
occurrence가 없으며, 구형 trigger512의 occurrence는 disabled다. 인형·공의 불은 별도
10HP/100ms 및 광기 3%/100ms다. 분열 공 폭발의 34ms 창에는 화염의 반복 틱을 복사하지 않는다.

WORLD Collider의 optional shape는 기존 BOX를 기본값으로 유지하고 CYLINDER를 추가한다.
halfExtents는 [radius, halfHeight, radius]를 사용한다. Client codec·편집·Debug 와이어,
Map publisher와 기존 Server worldTrack bake를 연결하며 새 피해 런타임은 만들지 않는다.
비균등 X/Z scale은 큰 축으로 원 반경을 정하고 중심 계산 이후에 key를 정규화한다.

설치 전 후보와 독립 검토:

- `out/KoukuAuthoring20260925/cylinder10percent.py --validate`: PASS. Map2216/Action2348
  후보의 공 Collider5개에 shape만 추가한다. Action은 관련 제품 재생성을 위한 revision만 증가한다.
- `out/KoukuAuthoring20260925/cylinder10percent/independent-review.json`: PASS.
  16개 창의 ID·시작/종료·중심·높이·10%·키 시계·visibility를 기존 BOX 결과와 대조했다.
  이 공들은 X/Z scale이 이미 같아 실제 scale key 변경은 0개다. live source bytes는 유지했다.
- Python collider25개와 projection/실데이터/무관 owner 금지3개: PASS.
- Server의 기존 `--kouku-object-overlap-contract-test`에 원통 단일·중첩·모서리/높이 제외·
  한 창 중복 금지·34ms 종료를 확인하는7개 검사를 추가했다. 최종 실행은684 PASS/실패0이다.
  로그는 `out/KoukuAuthoring20260925/cylinder10percent-server-overlap-final.log`다.
  실제 게시본처럼0/33ms visible=true,34ms visible=false를 구성하고30Hz 반올림 종료 tick을
  밖에서 처리한 뒤 재진입시켜 무피해와 window closed를 함께 확인했다. 최초 fixture는
  마지막 key가 계속 visible인 채 종료 tick 이전에 진입시키던 잘못된 전제가 있어 교정했다.
  Server gameplay 런타임은 바꾸지 않았다. 교정한 검사CPP만 동일MSBuild로 정규 `/t:Build`하여
  성공했고 로그는 `cylinder10percent-server-final-build.log`다.

- Map publisher의 실제 roundtrip 13가지 사례: PASS. 기존/명시 BOX·CYLINDER 보존과
  잘못된 shape·비균등 반경·CYLINDER HOOK 거절을 확인했다. PS AST와 변경 diff 검사도 PASS다.
- 최신 디스크 hash 재확인·writer lock·백업·원자 교체로 Map2216/Action2348 설치 완료.
  `out/KoukuAuthoring20260925/cylinder10percent/install.receipt.json`과
  `out/transactions/kouku-raid-09b3a88441a24fa7a99263fdfc59ca44`에 근거를 보존한다.

- 공식 KoukuSaydon owner publish revision2348: PASS. product537831ms, map·world·gameplay를
  포함한 전체721249ms이며 로그는 `out/KoukuAuthoring20260925/cylinder10percent-publish.log`다.
- 게시 후 `cylinder10percent/verify_published.py`: PASS. 원본/게시 Map 전체 semantic 일치,
  원본 bake와 Encounter의16개 geometry 일치, 실제 Gameplay bootstrap의16개 결과·region·
  worldTrack와48개 key가 CYLINDER/10%로 일치한다. Python TSV 검사이며 native 실행은 별도다.
- 최신 원본·설치본으로 WORLD group 회귀5개 PASS:
  `out/KoukuAuthoring20260925/cylinder10percent-world-group-final.log`.

빌드 차단 원인은 기존 `SourceCharacterMaterialParameters.h`에 추가된 native1523의 한글
parameter key4개를 CP949 컴파일러가 오독한 것이었다. `build_vehicle_source_material.py`의
C++ 문자열 생성은 UTF-8 bytes를3자리 octal escape로 내보내도록 교정하고 현재 헤더4개도
같은 bytes로 바꿨다. 역치환 시 헤더 전체 원본 bytes가 동일하다. 파일 인코딩·재질 값·
HLSL·전역 컴파일 charset은 바꾸지 않았다.

생성기 관련 회귀8개와 실제 MSVC CP949의53바이트 동등성 static_assert 컴파일도 PASS다.
근거는 `out/KoukuAuthoring20260925/material-literal-encoding-check.log`다.

두 번째 Product Build의 셰이더 단계는 통과했으나 `Level_CharacterSelect.cpp`의 새 클래스
영화 표시명5개도 같은 CP949 문자열 해석 오류를 냈다. 해당 문자열만 동일 UTF-8 bytes의
octal escape로 교정했으며 역치환한 파일 전체 bytes가 이전 저장본과 같음을 확인했다.
기존 파일 인코딩·CRLF와 다른 작업의 변경은 보존했다. 같은 빌드에서는 Engine 단계 완료 뒤
추가된 `Set_SoundCuePlaybackRate` 선언이 이전 EngineSDK에 없어 Client 컴파일이 실패했다.
SDK를 수동 편집하지 않고 최신 Engine부터 정규 Product Build를 다시 실행했다.
실패 receipt는 `out/BuildPipeline/runs/20260925T012937827Z-debug-product.json`,
후속 로그는 `out/KoukuAuthoring20260925/product-build-confirm.log`다.

최종 Debug Product Build는 Engine·Shared·Server·Client 모두 PASS다. 전체145670ms,
Client130025ms이며 `MapTool_Area.cpp`, `Level_CharacterSelect.cpp`,
`WorldSequencePlayer_Objects.cpp`의 실제 재컴파일과 Client 링크·Engine DLL/CSO 배치를 확인했다.
receipt는 `out/BuildPipeline/runs/20260925T013413528Z-debug-product.json`이다.
새 사운드 선언을 포함한 Engine/Public과 EngineSDK의 GameInstance.h SHA도 동일하다.
기존 C4819·셰이더 경고와 DirectXTK PDB 부재 LNK4099 경고는 남아 있으며 오류는0이다.
최종 변경 범위 `git diff --check`도 PASS다. Product Build가 데이터 게시를 대신하지는 않으며
위 revision2348의 명시적 publish와 구분한다.

현재 상태: 코드·데이터 설치, 공식 게시, Server 실행 검증 및 최종 Debug Product Build 완료.
실제 화면의 폭발 크기·피격 체감 확인은 사용자 확인 전이다.
