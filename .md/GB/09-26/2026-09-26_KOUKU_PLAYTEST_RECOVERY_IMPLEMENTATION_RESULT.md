# 쿠크 플레이테스트 복구와 최신 저장본 게시 결과

## G01. 보존과 실제 반영

수정 전 게시 패턴·시퀀스·렌더링과 대응 저작 입력1887파일을 `BackupData/2026-09-26_085649_before-raid-fixes`에 해시 검증해 보관했다. 실제 교체 전 Action/WORLD 및 두 Character WModel은 `BackupData/2026-09-26_091911_raid-recovery-install-before`에 별도 보관했다. 최신 디스크의101개 stable-field 변경과 엔딩4배우 필드를 병합하고 해시 재확인·원자 교체·자기 변경 rollback을 사용했다.

Action2419 / WORLD2279 / Sequence177이다. JSON span을 재사용해 무관한 숫자 표기·포맷까지 보존했다. 현재 Action 원본 대비 +59/-25줄, WORLD +76/-55줄이다. rendering 저작/게시본·카메라·사운드·Sequence 보호15파일은 시작 백업과 byte 동일하다. 소스 증거는 `out/KoukuPlaytestRecovery20260926/install-receipt.json`, `protected-data-review.json`이다.

## G02. 원작 엔딩과 같은 타임라인에서 직접 편집

원본 SCENE01B로 재생성한4배우 full animation은 기존 native payload와 byte 동일하다. 이를 원래25구간(saydon1=11, saydon2=4, kouku1=4, kouku2=6)으로 나눠 독립 native clip을 설치했다. 모델 geometry·재질·골격·기존 전투 clip은 그대로다. 이전 실패한 saydon1 Duplicate에 따른3.366초 추가와 마지막 hold를 제거해 무기 포함5WORLD 모두0~49083ms/span49083/speed1로 복원했다.

손 뻗기 clip은12967~16333ms이며 해당 카메라 cut 시작12967ms와 맞는다. 카메라12구간은0~49083ms 연속이다. 사운드2개의 시작0/source0/길이51185·50965ms와 자막은 기존 Sequence177 그대로이며 전체52042ms 안에 있다. 이는 저장 시간과 원본 pose sampling의 정합성 검사이며 실제 화면·청취 완료를 뜻하지 않는다.

Sequencer 상단 WORLD Animation 행의 가운데 이동과 양끝 trim을 `WorldObjectTool::Edit_AnimationTimeline`에 연결했다. native 범위·앞뒤 구간·stable clip identity를 검사하고 다른 클립을 밀지 않는다. 첫 clip 시작이0보다 커도 시작 전 Source In 자세를 유지한다. WORLD만 편집해도 Save가 활성화되고, 연결된 source-only 저장은 Action/Sequence 파일을 바꾸거나 자동 publish하지 않는다. 편집되지 않은 WORLD만 사용하는 제품 재생은 저장본 기준을 유지한다.

검증: 원본2.41M keys/1.6M bone poses 비교(max component error1.3674e-5), actual 편집11종37검사, Collider27테스트, Map/Composition delayed-first parity PASS. 실제 설치 WModel catalog→편집 API→Workbench Save→WORLD 저장/재로드 PASS. Action/Sequence byte 보존과 동시 저장 충돌 시 외부bytes/dirty/pending 보존을 확인했다. 증거 `out/KoukuOriginalSplit20260926/original-split-receipt.json`, `out/KoukuWorldDirectTimeline20260926/{direct-timeline-receipt,save-receipt}.json`, `out/KoukuPlaytestRecovery20260926/independent-ending-install-review.json`.

## G03. 패턴·전조·주사위 이펙트

## 패턴 저작 필드와 분신 실제 보스 연결

Action revision2418 / WORLD revision2278의 사용자 저장본에서 101개 stable-ID 필드 연산을 준비했다. root가 최신 저장본에 병합·revision을 올리고 실제 게시를 소유한다. 후보는 Data/Resources를 직접 교체하지 않았다.

- Chevron: 원본 Effect element scale[6,6,6]을 유지하고 모든 바닥 occurrence scale을[1,1,1]로 맞췄다. native StartSize[.5,1]을 포함해 타일 하나의 실제 바닥 크기는3×6m이다. 빙고 해머4방향에는4개, 카드미로4방향에는8개를 같은 크기로 반복하며 간격과 겹침으로 끝점을 맞춘다. native particle 높이 offset만 차감했다. 카드미로 lane 간격5.12m보다 표시 폭3m가 작다.
- P11 파1빨2: Collider나 다른 row가 참조하지 않는 이전 blue-position Duration인 logic.10만 제거했다. 사용자가 늘린 red2와 전조 생성 시각을 유지했다.
- Gate2: entry 이후 첫 반복에서 원래16번째였던 P99 불어날리기 직전에 P17 잡기를 삽입했다. 따라서 잡기16번째/불어날리기17번째가 된다. HP repeat group6개 모두 같은 순서를 반복하며 HP 경계·다른 entry·대기시간은 유지했다. P17의 사용자 rectangle4개를 기존10% max-HP 결과에 연결하고 중복되는 예전 circle damage2개를 껐다.
- P26 대형세이튼 피자: 이전 joker의 무작위 facing이 남는 resetBossToSpawn=false를 해당 패턴에서만 true로 고치고 현재 giant spawn yaw226.5를 사용했다. 쿠크 피자의 사용자 yaw는 건드리지 않았다.
- P65 십자분신: 새 optional Summon.realPatternId를 왼쪽 자식 P63으로 지정했다. absent일 때 이전 nearest-spawn 선택을 유지한다. authoring parser/validator/save, Workbench, projector/bootstrap, Server catalog/Brain 및 Client local preview까지 같은 stable ID를 소비한다. directionPatternIds의 front/back/left/right 순서를 바꾸지 않는다.

## 대형세이튼 불뿜기 collider와 tick

일반 세이튼의 비교 대상은 P102 mouth flame인 logic.9/10/11이다. 앞의 폭발·장판 logic.1~8의100ms tick과 구분했다. 일반 mouth flame 각 창은1555~6449ms, duration4894ms / repeat1632ms여서3회다. 대형세이튼 P27은 실제 세 mouth FX가 함께 살아 있는2366~4706ms, duration2340ms / repeat780ms로3회다. 따라서 타격 횟수와100 damage+광기1% 결과를 맞추되 간격 자체는 각 애니메이션 길이에 맞춰 다르다.

P27의 기존 넓은 고정 박스 하나를 세 입의 bip001-mouth와 각 FX 방향을 따르는 같은 단위 박스3개로 바꿨다. 각 halfExtents[.9,2,4.5], scale1을 사용한다. 3개 Collider가 같은 logic.3 판정을 공유해 겹친 지점의 플레이어가 같은 tick에 중복 타격을 받지 않게 했다. 설치 WModel의 실제 bone 경로를 사용한 region projection이 통과했다. 화면에서의 최종 방향·가시성은 사용자 확인 범위다.

## 아이언메이든에서 주사위 카드 속박 효과 제외

원인은 Client Update_DiceBindVisuals가 snapshot.isPatternBound 전체에 카드 바닥과 release 효과를 붙이던 것이다. Server의 CARD_DICE_BIND와 MARIO_PHASE2_PLAYERS는 같은 일반 속박 bit와 endTick을 사용하므로, 이 bit만으로 이펙트 종류를 정하면 아이언메이든에도 주사위 효과가 붙는다.

Publisher가 enabled CARD_DICE_BIND를 가진 패턴에만 optional Product diceBindVisual:true를 투영한다. 현재 정본에서는 P78만 해당하며 P33 아이언메이든에는 표식이 없다. Client는 현재 Server boss pattern의 게시된 표식이 있을 때만 신규 dice hold를 시작한다. 이미 시작한 같은 endTick의 hold는 보스 패턴 전환 뒤 마지막 snapshot까지 유지하고 실제 해제 bit에서 기존 카드 match release를 재생한다. 다른 endTick의 non-dice bind는 기존 cache를 종료하며 hold/release를 생성하지 않는다. Shared packet/Server ABI와 실제 속박 판정은 변경하지 않았다. Product 재게시와 새 Client 빌드가 필요하다.

검증:

- dice projection, fixed-real direction, legacy cross policy, invalid cross policy 집중 unittest4개 PASS.
- 현재 Update_DiceBindVisuals 본문을 그대로 추출한 CPU contract harness12개 PASS: maiden bind/unbind 무효과, dice late join과 원래 asset, 패턴 전환 유지, 카드 조기 해제, release 종료, 새 maiden으로 바뀔 때 stale cue 정리, 사망과 사망 보스 처리. Engine 표시 호출과 snapshot만 mock했으며 Client/GPU를 실행하거나 visual PASS라고 기록하지 않았다.
- debug_party 소유 전체 Server90 TU 격리 컴파일·링크 성공. realPatternId를 포함한 --kouku-bundle-contract-test120 PASS,0 failures.
- 변경 파일 git diff --check PASS. root가 Client 전체 빌드·최신 저장본 병합·공식 domain publish 및 최종 revision 검증을 수행한다.

증거: field-manifest.json, projected-collider-receipt.json, dice-bind.test.log, dice-bind-receipt.json 및 out/KoukuServerPlaytestRecovery20260926의 compile/test receipt.


## G04. Server 판정과 재입장

- Iron Maiden formation의 Cancel_PlayerActionForPatternStatus와 bound 매틱에서 `isCombatReady=false`가 남아 피해 판정을 막던 원인을 수정했다. 원래 readiness를 보존하고 bound 입력 잠금은 유지한다.
- CLOWN만 hook 포획에서 제외하던 조건을 제거했다. 기존 Server player identity로 포획·복구한다.
- Gate2 전투면10.56m/의자6.51m인데 기존 사망면5.56m라 의자에 착지하면 생존했다. Gate2 카지노 영역의 낙사 깊이는1m로 제한하고 다른 영역의5m 정책은 유지한다.
- 마지막 player 퇴장 때 Kouku/Valtan 관문 진행·clear/vote·raid/Mario/maze 상태를 초기화한다. 사람이 남은 방은 보존한다.

전체 Server90 TU 격리 컴파일·링크 PASS. 실제 Leave/reset26, hook·chair·overlap806, bound 실제 매틱+두 이동 칼날76, cross-direction bundle120검사 PASS. 3인·4인 captive 모두 두 칼날에 사망했으며 fixture가 readiness를 강제로 복구하지 않았다. `out/KoukuServerPlaytestRecovery20260926/{compile,test}-receipt.json`.

## G05. Effect 편집과 hover

블랙홀/메두사의 Effect codec은 legacy ENCORE를 받아도 모델 소비자가 BINGO만 받아 Play All이 실패했다. decode에서 ENCORE→BINGO로 정규화하고 validator/생성기를 canonical BINGO로 맞췄다. 두 파일의 metadata→Load→Drawable→SourceProp→SourceModel 준비 PASS. 블랙홀5clip/20.8초, 메두사1clip/11.1초다. authored Effect JSON은 수정하지 않았다. 증거 `out/KoukuEffectOpen20260926/chain/receipt.json`.

ClientReplication의 combat hover picking/상태와5Level 호출을 제거했다. UI hover, 클릭 이동·공격 picking, hit flash는 유지한다. 재사용 renderer의 hover API를 삭제해 다른 rendering 계약을 흔들지는 않았다.

## G06. 검증과 남은 확인

Debug 일반 Product Build PASS(`out/BuildPipeline/runs/20260926T002317423Z-debug-product.json`). Compiler warning은 기존 Level.h 인코딩과 shader 경고를 포함하며 error는 없다. WorldSequences 공식 publisher PASS. Kouku projector는 Action2419, saved121/product114 patterns, saved11/product9 bundles,577stages를 투영했다.

Release 일반 Product Build도 PASS(`out/BuildPipeline/runs/20260926T003304315Z-release-product.json`,563812ms). 마지막 Gameplay Publish PASS 후 저장 Action2419=Encounter2419=Presentation2419=Server Product2419를 확인했고, 네 RAIDGATE의 Sequence revision이 모두177로 저장본과 일치한다. WORLD2279 source/runtime SHA도 동일하다. 변경 JSON5개와 프로젝트 XML4개 parse, 최종 git diff --check PASS. 새 게시본을 실제 Debug Server.exe의 `--kouku-product-contract-test`로 읽어 333검사/failures0까지 확인했다. 이 명령은 UI나 제품 서버 시작이 아닌 headless contract 모드다. 최종 증거는 `out/KoukuPlaytestRecovery20260926/final-publish-build-review.json`이다. 최종 게시본과 대응 저작56파일은 `BackupData/2026-09-26_093639_published-ready`에도 추가 백업했다. Client/arena UI는 실행하지 않았다. 새 Server/Client로4인 재입장·실제 표시/소리와 편집 조작 확인은 사용자 범위다. 기존 무관한 미커밋 변경과 동시 Valtan 작업은 유지했으며 별도 commit/push는 하지 않았다.
