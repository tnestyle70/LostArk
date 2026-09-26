# 쿠크 플레이테스트 복구와 최신 저장본 게시 결과

## G01. 보존과 실제 반영

수정 전 게시 패턴·시퀀스·렌더링과 대응 저작 입력1887파일을 `BackupData/2026-09-26_085649_before-raid-fixes`에 해시 검증해 보관했다. 실제 교체 전 Action/WORLD 및 두 Character WModel은 `BackupData/2026-09-26_091911_raid-recovery-install-before`에 별도 보관했다. 최신 디스크의101개 stable-field 변경과 엔딩4배우 필드를 병합하고 해시 재확인·원자 교체·자기 변경 rollback을 사용했다.

Action2419 / WORLD2279 / Sequence177이다. JSON span을 재사용해 무관한 숫자 표기·포맷까지 보존했다. 현재 Action 원본 대비 +59/-25줄, WORLD +76/-55줄이다. rendering 저작/게시본·카메라·사운드·Sequence 보호15파일은 시작 백업과 byte 동일하다. 소스 증거는 `out/KoukuPlaytestRecovery20260926/install-receipt.json`, `protected-data-review.json`이다.

## G02. 원작 엔딩과 같은 타임라인에서 직접 편집

원본 SCENE01B로 재생성한4배우 full animation은 기존 native payload와 byte 동일하다. 이를 원래25구간(saydon1=11, saydon2=4, kouku1=4, kouku2=6)으로 나눠 독립 native clip을 설치했다. 모델 geometry·재질·골격·기존 전투 clip은 그대로다. 이전 실패한 saydon1 Duplicate에 따른3.366초 추가와 마지막 hold를 제거해 무기 포함5WORLD 모두0~49083ms/span49083/speed1로 복원했다.

손 뻗기 clip은12967~16333ms이며 해당 카메라 cut 시작12967ms와 맞는다. 카메라12구간은0~49083ms 연속이다. 사운드2개의 시작0/source0/길이51185·50965ms와 자막은 기존 Sequence177 그대로이며 전체52042ms 안에 있다. 이는 저장 시간과 원본 pose sampling의 정합성 검사이며 실제 화면·청취 완료를 뜻하지 않는다.

Sequencer 상단 WORLD Animation 행의 가운데 이동과 양끝 trim을 `WorldObjectTool::Edit_AnimationTimeline`에 연결했다. native 범위·앞뒤 구간·stable clip identity를 검사하고 다른 클립을 밀지 않는다. 첫 clip 시작이0보다 커도 시작 전 Source In 자세를 유지한다. WORLD만 편집해도 Save가 활성화되고, 연결된 source-only 저장은 Action/Sequence 파일을 바꾸거나 자동 publish하지 않는다. 편집되지 않은 WORLD만 사용하는 제품 재생은 저장본 기준을 유지한다.

검증: 원본2.41M keys/1.6M bone poses 비교(max component error1.3674e-5), actual 편집11종37검사, Collider27테스트, Map/Composition delayed-first parity PASS. 실제 설치 WModel catalog→편집 API→Workbench Save→WORLD 저장/재로드 PASS. Action/Sequence byte 보존과 동시 저장 충돌 시 외부bytes/dirty/pending 보존을 확인했다. 증거 `out/KoukuOriginalSplit20260926/original-split-receipt.json`, `out/KoukuWorldDirectTimeline20260926/{direct-timeline-receipt,save-receipt}.json`, `out/KoukuPlaytestRecovery20260926/independent-ending-install-review.json`.

### G02-1. 후속 조사: 서로 다른 배우를 섞은 WORLD Animation 표시 행

이번 사용자 저장본의 saydon1 .04는12967~14703ms, .05 시작은16333ms로 실제 공백1630ms다.
그러나 오류가 난 선택 clip은 kouku1 .02이며 시작15700ms와 앞 .01 끝15700ms가 붙어 있어
공백0ms다. 시간 구간으로 행을 채우는 기존 표시가 두 배우의 clip을 같은 시각 행에 섞어 보여
다른 배우의 빈 구간을 선택 clip의 공백처럼 보이게 했다. 이 경우 앞당기기 거절만으로 native
범위나 이웃 검증이 잘못됐다고 판단하지 않는다.

후속 수정 범위는 Workbench의 World occurrence+slot별 고정 표시 행, 배우 label/tooltip과
WorldObjectTool의 이웃 충돌/Motion 범위 초과 사유 구분이다. 검증 조건과 기존 clip 시계는
유지한다. 사용자 dirty Sequence Composition·WorldSequences, 카메라·음향과 다른 lane의
시간은 변경하지 않고 자동 publish하지 않는다. 위 G02의 이전 검증과 구분하여 이번 후속의
Workbench/WorldObjectTool 격리 컴파일과 실제 편집 본문의11사례37검사를 통과했다.
기존 draft 보존과 이웃/Motion 경계 거절을 확인했으며 실제 화면은 사용자 확인 범위다.

원본 `SCENE01B`의 Matinee32 variablelinks → SeqVar_Object → actor를 대조했다.
아래 `1/2`는 변환기 라벨이며, 원본에서는 서로 다른 UObject다. actor 경로의 공통 앞부분은
`theworld.persistentlevel.`이다.

| 변환기 라벨 | 원본 Matinee 그룹 / export | 원본 actor / export | 실제 skeletal mesh | Slomo 반영 전 원본 장면 시각(ms) |
|---|---|---|---|---|
| saydon1 | 세이튼_1 / 49 | efskeletalmeshactorlookinfomat_4 / 37 | MN_RPCT_05 | 0–30933 |
| saydon2 | 쿠크세이튼 / 63 | efskeletalmeshactorlookinfomat_0 / 34 | MN_RPCT_05 | 30933–41067 |
| kouku1 | 쿠크 / 58 | efskeletalmeshactorlookinfomat_1 / 35 | MN_RPCZ_00 | 12967–19400 |
| kouku2 | 쿠크2 / 64 | efskeletalmeshactorlookinfomat_3 / 36 | MN_RPCZ_00 | 28542–30933 |

같은 종의 두 actor는 mesh를 재사용하지만 별도 위치·가시 구간을 갖는다. 특히 kouku2는
원본에서 `base=37`, `bHardAttach=true`, `baseBoneName=bip001-l_wing0102`로 saydon1에
부착된다. 따라서 같은 모델이라는 이유로 두 actor를 중복 투영으로 보고 삭제·병합하면 원본
배우와 부착 관계를 잃는다. 당시 WORLD는 위 원본 장면 시각을 그대로 저장했다. 아래 G02-2에서 Slomo를 포함한
실제 재생 시각으로 교체했다. 이 확인은 실제 GPU의 중복 표시 여부나 이번 행 분리의 최종 화면 PASS를 뜻하지 않는다.
근거는 `Tools/KoukuSaydonPipeline/build_bingo_ending_actors.py`의 `SPECS`와 `world_rows`,
`out/KoukuOriginalWorldRebuild20260926/SCENE01B.json`,
`out/KoukuEndingTimeline20260926/source-clip-timelines.json`이다.

### G02-2. 원본 시간축·배우·카메라·재질·음향 복원

범위는 사용자가 선택한 `빙고_최종엔딩씬`이다. 같은 장면의 Sequence P9와 Action P75를
함께 수정하고 별도 앵콜 P10은 보존했다. 사용자가 편집 중임을 밝혀 후보 검증까지 분리한 뒤,
“exe 종료했어, 끝나면 전부 다 반영해줘”라는 명시 승인 후 최신 디스크를 다시 읽어 반영했다.

원인은 네 배우의 존재 자체가 아니다. 원본 Matinee에 별도 UObject 네 개가 있으며 전반·후반
세이튼, 독립 쿠크, 왼쪽 날개에 부착된 쿠크다. 이름으로 중복 제거하지 않고 stable ID를 유지한다.
누락된 Slomo191로 원본49.083335876초와 실제52.316501957초가 달라졌고, 기존 카메라·배우·
Effect·자막·음향이 같은 시계를 소비하지 못했다. constant 위치키 경계17ms의 가짜 이동과
본 부착 궤적의 중간 표본 오차도 별도로 확인했다.

- `source_scene_clock.py`가 source→elapsed 적분과 역변환을 공유한다. 카메라12 stable ID,
  배우 full4/split25, 가시성, 자막을 실제 시간으로 변환한다. 본편 끝은52317ms다.
- full4를 먼저 변환한 동일 WANM에서25개 분할 클립을 잘라 경계 자세를 일치시켰다.
  geometry/material/skeleton과 다른 animation payload는 바이트 그대로 보존했다.
  가시 구간 WORLD 변환 오차는1mm/0.05° 이내이며, 모델 key component 오차와1ms step
  양자화는 별도 receipt에 기록했다. 원본 root-motion 옵션 우선순위는 추정해서 변경하지 않았다.
- 카메라는 최대80키가 필요하여 Tool/Level/Publisher의 공통 상한을128로 맞췄다.
  매 실제 ms 대조 오차는 eye4.525mm, 자세0.099° 이내다. 설치 WANM의 실제 본 투영 대조는
  1080p에서 최대0.116px다. 이 수치는 최종 GPU 화면 판정과 구분한다.
- 누락된 무기251/247만 추가했다. 모자252/246은 기존 `CWorldSequenceObject`가 이미 그리므로
  중복 객체를 만들지 않는다. `dead`는 독립 낙하 무기가 아닌251의 원본 MIC에 연결한다.
  선택적 `materialTracks`는 exact material/parameter를 기존 program32 packer와 같은 객체에
  전달한다. 실제 Prototype/Clone/Layer→CModel→재질 bind/draw의7상태와 되감기를 검증했다.
- 공유 V1 Effect40 element의 원본 내부 시계는 유지하고 이 occurrence에만
  `effectSourceTimeKeys`를 적용한다. 저장·투영·재생·trim·capture 역변환을 연결했다.
  fixed MAP만 허용하며 source-in/fit/loop/follow/fade override와 혼용하지 않는다.
  V2 trim도 asset lifetime을 바꾸지 않고 원본 곡선을 같은 시각에 샘플한다.
- 원본 Fade98을 기존 V2 ScreenPost의 장면 전용 암전으로 복원했다. 소유 occurrence의
  종료/Stop이 정리한다. 마지막 파티클의 원래3초 꼬리를 보존해 전체 창은55275ms다.
- 음성 시작100ms, BGM Play 지연10ms, source Stop48974.121ms→wall52207.287ms로 복구했다.
  원본 Wwise bank의 단일 재생을 유지하고 WAV를 늘이거나 반복하지 않는다. BGM은 원본
  샘플 수/48kHz를 유지해53743.688ms에 자연 종료한다. 원본 Stop fade는 기존 wwiser 근사
  곡선을 사용했으며 Wwise bit-exact라고 기록하지 않는다. 실제 catalog variant까지 연결했다.

검증은 Debug 일반 Product Build 두 차례 PASS(최종
`out/BuildPipeline/runs/20260926T115844905Z-debug-product.json`), native Sound timeline/Effect
clock Save/reopen·trim·잘못된 키6종 거절 PASS, 카메라 집중5개+19검사 PASS,
Composition WORLD27검사 및 공식 전체 source4개 PASS다. 전체 WORLD/Camera 후보는
공식 Map publisher reader로 읽었다. 추가 원본 clock/schema Python 검사와 Fade leaf 검증도
통과했다. 기존 Scene Profile에 빈 effect clock이 투영되던 회귀와 live anchor 변경의 clock
제약 누락을 독립 리뷰에서 찾아 같은 변경에 수정했다.

반영12파일은 `out/KoukuEndingRecovery20260926/BackupData-20260926-210254`에 이전 bytes와
hash를 백업하고 교체 직전 재확인·파일별 원자 교체를 적용했다. 사용자 카드미로/빙고 해머
WORLD 편집, 별도 블랙홀/메두사 Effect 편집, 다른 패턴 및 렌더링 튜닝을 보존했다.
원본 복원 후보의 1차 게시 완료: 저장 Action2420=Encounter2420=patternbindings2420이며 Sequence181을
네 RAIDGATE가 모두 소비한다. WORLD2281/Camera94는 공식 Map publisher로 게시했고
저장본과 런타임 구조가 일치한다. Kouku projector, Gameplay publisher, Composition publisher는
모두 PASS다. BINGO clear는 기존 stable pattern9를 유지하고 전체 duration55275ms다.
최종 Composition sourceManifestId는
`7ecd1ae38db3e06b6fae5c51fe5a36e686f1e30b75c32df1408a03c999697848`이다.

사용자가21:15:56에 실행한 Server/Client보다 Gameplay 게시21:16:26 및 Composition
게시21:16:43이 늦어 두 프로세스의 한 차례 재실행을 안내했다. Debug EXE는 이미 최신이므로
재빌드가 필요하지 않다. 이후 사용자는 “지렷다 됏어 ㅋㅋㅋ”라고 정상 동작을 확인했다.
이는 사용자가 확인한 현재 장면의 동작 성공이며 아래 미복원 항목까지 원본과 동일하다는
검증은 아니다. 확인 중인 EXE와 런타임 데이터는 추가 교체하지 않았다.

이번 G02-2 후속 변경은 Debug와 Release 일반 Product Build 모두 PASS다. Release는
사용자가 Server/Client 종료를 확인한 뒤 정식 도구로 수행했으며 Engine/Shared/Server/Client
컴파일·배포를 완료했다. 최종 Release 증거는
`out/BuildPipeline/runs/20260926T123132776Z-release-product.json`이다. 필수 런타임 파일
누락과 catalog 불일치는0이다. 아래 G06의 이전 Release PASS(2419/177)와 구분한다.
설치/게시 증거는
`out/KoukuEndingRecovery20260926/final-publication-validation.json`과 각 publish log,
`BackupData-20260926-210254/install-manifest.json`에 남겼다.

사용자 요청에 따라 바탕화면 `GBResources`에도 최종 리소스3개를 전달했다. 기존
`Character/KoukuSaton/MN_RPCT_05/MN_RPCT_05.wmodel`은 백업 후 교체했고,
`Character/KoukuSaton/MN_RPCZ_00/MN_RPCZ_00.wmodel`과
`Sound/KoukuSaton/Events/bgm_midnightc_ed_m20_scene_finish.source-clock-stop.wav`를
추가했다. 전달본 총422487918bytes의 SHA-256은 설치본 및 install-manifest와 모두 일치한다.
이 전달 작업은 Client 설치본과 Data를 변경하지 않았다. 검증 기록은
`out/KoukuEndingGBResourcesDelivery20260926/delivery-receipt.json`이다.

1차 게시본(Action2420/Sequence181)을 Debug Server의 `--kouku-product-contract-test`로 읽은 headless 검증도
333검사 PASS, failures0, exit0으로 완료됐다. 사용자의 제품 Server/Client와 별도인
계약 검사 프로세스는 정상 종료했고 실제 UI를 실행하지 않았다. 로그는
`out/KoukuEndingRecovery20260926/server-product-contract.log`다.

**최종 전달 revision은 Action2420 / Sequence182 / WORLD2281 / Camera94다.** 사용자가
Debug 확인 중 저장한 Sequence182의 duration52350ms, stage52343ms, BGM52305ms,
Effect52314ms 및 Fade52324ms를 원본 후보181로 되돌리지 않고 그대로 보존했다.
Effect117개 clock key의 trim 종료값은 원래 clock을52314ms에 샘플한 값과 일치한다.
최신 저장본은 `BackupData-user-sequence182`와 hash receipt로 별도 보존했다.

최초 Gameplay 재게시 시 stale Product 검사가 기존 Encounter 내용을 거절해 기존 게시본을
유지했다. 이후 Kouku projector→Gameplay publisher→Composition publisher 순서로 다시
게시해 모두 PASS했다. 저장 Action2420=Encounter2420=Client patternbindings2420=
Server Product2420, 저장 Sequence182=Server 네 RAIDGATE182를 실제 파일에서 대조했다.
BINGO clear는 기존 pattern9와 사용자 저장 duration52350ms다. WORLD2281/Camera94는
source/runtime 전체 문서 동일, 설치12파일은 최신 사용자 Sequence182를 포함해 기대 hash와
일치하며 GBResources3파일도 설치본과 동일하다. 기존333검사는181 게시본 기준이고,
182의 마지막 변경은 공식 publisher 검증·clock trim 검증·최종 pins/hash 대조로 확인했다.
최종 기록은 `out/KoukuEndingRecovery20260926/final-publication-validation.json`,
`out/KoukuEndingCamera20260926/sequence182-readonly-check.json`이다.

Release 클라이언트4개를 실제로 띄운 동시 접속/UI 검증은 실행하지 않았다. 빌드·게시 성공이나
이전333개 Server 계약 검사 결과를4클라 Release 화면 검증 성공으로 기록하지 않는다.

**아직 원본 재현이 완료되지 않은 항목:** cam02 원본 DOF에는 현재 depth/CoC 소비 경로가
없다. CameraShake의 세 축 진폭/주파수/Random 초기위상 원본은 확보했지만 native 회전 단위·
위상·envelope의 재생 경로는 미복원이다. g_fog named target은 원본 ExponentialHeightFog로
찾았지만 Track18의 `heightfogcomponent0` 경로와 실제 component 이름의 차이로 활성 binding을
확정하지 못했다. brightness0인 Spot lightshaft도 보이지 않는다고 단정할 수 없으며 원본
shaft carrier는 미복원이다. 따라서 전체 원본과 완전히 같은 컷신이 완성됐다고 판정하지 않는다.
Client 화면·실제 소리는 사용자가 확인하며, native bind/draw 성공을 화면 동등성으로 대신하지 않는다.

근거: `out/KoukuEndingActorsClock20260926`, `out/KoukuEndingCamera20260926`,
`out/KoukuEndingEffectAudit20260926`, `out/KoukuEndingAudioClock20260926`,
`out/KoukuEndingCompositionValidation20260926`, `out/KoukuEndingRecovery20260926`의 receipt/log.

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

## G07. 예고 방향·컷신 가시성·Complete Play 로드 복구

세토 카드미로32개·빙고 망치16개 warning occurrence의 yaw만180° 교정했다.
source StartRotation0.5turn/바닥 축 정렬과 DDS 문양을 합성하면 emitter +Z가
문양 전방인데 기존 WORLD가 -Z를 가정했다.36개 세토 lane·20개 망치 anchor
실례에서 이동 방향과의 내적이-1에서+1로 바뀐다. 망치20개 실제 이동 끝점은
Shared Kouku_BingoHammerPath와 최대0.00005000012m 차이로 경로 자체가 맞았다.
공유 Effect/DDS/shader/크기·수명·피해 경로는 변경하지 않았다.

WORLD2282를 최신2281 저장본에49필드(revision포함)만 병합하고 교체 직전 hash
확인·백업·원자 교체했다. 공식 WORLD reader와 Scope WorldSequences Publish
통과. 증거는 out/KoukuDirectionFix20260926 및
out/KoukuDirectionVisibility20260926/map-publish.log다.

컷신은 기본 플레이어 숨김이고1관문 입장·카드미로·3관문 입장만 표시한다.
실제3관문 입장은 Sequence P5/gate2.clear camera도 사용한다. 최종 재확인에서
P5 앞16710ms는2관문 클리어이고 뒤18658ms가 독립 P7과 같은3관문 입장임을
확인했다. clear camera11~18만 표시 예외로 제한하고1~10과 P5의 camera 공백은
기본 숨김으로 둔다. Character의 독립 억제 상태 하나를 Part weak owner로 조회하여
몸체·장비·탈것·그림자 draw에서 합성한다. camera가 Late_Update 뒤 시작하는
프레임도 MainApp의 기존 cinematic 동기화에서 갱신한다. 종료/실패/Level이탈
때 컷신 억제만 해제하고 기존 Server/source/장비 가시성은 보존한다.
최종 실제 함수 MSVC 계약67검사 및 draw진입9곳 검사 통과. 앞clear10샷 숨김,
뒤entry8샷 표시와 P5 초기/후반 camera 공백 숨김을 포함한다. 증거는
out/KoukuCutsceneVisibility20260926이다.

22:00:25 pid32332의 Kouku.product.load 로그와 같은 실패를 기존 native Product
loader에서 재현했다. 추적 폭탄의 normalized targetedCombatVisuals6개가 빈
effectSourceTimeKeys를 저장하여 전체 presentation 로드를 막았다. 투영은
명시적 빈 배열도 생략하고 native/authoring reader는 empty/absent를 동일한
미사용 상태로 읽는다. 비배열·1키·4096초과·속성·범위·단조 오류는 계속 거절한다.
정상 원본clock1개를 유지했다. READY는 실제 전체 presentation reader 검증을
포함하며 같은 source 준비의 성공/실패를 캐시한다. supplied bytes 파싱 중
asset mutex 재진입을 피하고 canonical의 optional light/scene 실패 격리와
draft의 strict 정책은 보존한다. 실패한 새 준비는 기존 clip/source/epoch를
교체하지 않는다.

기존 실패/생략대조11검사, 수정 reader11검사, READY·authoring·격리17검사 및
Python 투영3검사 통과. 독립 읽기 검토에서 이 reader 범위의 재현 가능한
P0/P1은 발견되지 않았다. 증거는 out/KoukuProductSourceClock20260926이다.
다음 G08까지 합친 Product 빌드·최종 publish 결과는 아래에 별도 기록한다.
Client/UI 및 실제4클라 화면은 에이전트가 실행하지 않았다.

## G08. 쇼타임 폭발 피해와 메두사 활성 시야 판정

쇼타임 P35의 직사각 장판10회에 실제 폭발 Effect의 세 그룹을 대조했다.
각 occurrence의 폭발 시작 기준0/400/700ms, local Z=-6/0/+6에 맞춰
ENTER_AREA 창30개와 COLLIDER occurrence30개를 추가했다. 박스는 전체
3×3×6m, 활성34ms이며 각 창의 접촉은 플레이어마다 한 번만 소비한다.
기존 logic97→result538의 최대 HP10% 피해와 AWAY_FROM_CONTACT 4m/1000ms,
높이2m ballistic 강제 넉백을 재사용한다. 다른 폭발 창이 겹치면 각 폭발이
독립적으로 판정한다.27개 BOSS_START 창은 해당 예고 시점의 보스 transform을
고정하고3개 MAP 창은 WORLD 좌표를 사용한다. Effect·음향·기존 이동·타이밍은
변경하지 않았다. 사전 실제 projector의 창·region30개 투영이 통과했다.

메두사 P94의 기존 공유 logic36은 보존하고 전용 logic555를 추가했다.
gazeDuringWindow=true이며 플레이어 yaw와 플레이어→세이튼 방향의 차이를
기존 Judge_Gaze로 비교한다. 진짜 세이튼 찾기와 동일한 정면±45°/최대30m다.
얼굴 Effect와 같은[3167,6428)ms에 바라보면 기존 result37의3초 FEAR를
플레이어별 한 번 적용한다. parent P107에서는 자식 시작5400ms가 더해져
[8567,11828)ms다. 보스 yaw·카메라 yaw는 사용하지 않는다. FEAR가 끝난 뒤
같은 창에서 다시 보아도 연장하지 않고 창 종료 시 추가 판정을 하지 않는다.

기본 gazeDuringWindow=false는 기존 DURATION 종료 시 판정을 유지한다.
새 옵션은 Client 정의·Parse/Validate/Serialize/Workbench, Python 투영,
PATTERNLOGICGAZE bootstrap, Server catalog·Brain·실제 판정까지 연결했다.
true는 GAZE_REAL_BOSS/insideOutcome=FAIL/Fail 결과만 허용한다. 잘못된 타입,
종류·결과·중복 bootstrap은 거절한다. 진짜 세이튼 찾기의 기존 종료 판정은
변경하지 않았다. 신규 C++ 파일이나 프로젝트 항목은 추가하지 않았다.

Action2420 최신 저장본의 해당 stable ID와 ordinal만12개 원문 범위로 병합해
2421을 설치했다. 교체 직전 SHA 확인·백업·원자 교체를 수행했고 기존 숫자
표기와 무관한 저장 필드는 보존했다. 설치 SHA는
de4ff6499d17d816847fb2866570a9f2dc8d81f85e1d8763e9ad157b897a957c다.
증거는 out/KoukuDirectionVisibility20260926/action-merge-receipt.json,
out/KoukuShowtimeCollision20260926 및 out/KoukuProductSourceClock20260926이다.

추가 검증은 실제 최신 Debug Product OBJ와 설치된 bootstrap을 사용했다. 쇼타임은
Build→Update→Damage→HitReaction642검사/failures0이다.30개 창, 박스 경계±2mm,
예고 시작 transform 고정,0/12/21tick, 한 창1회 피해,10묶음의 공중 연속 피격과
강제 넉백 재시작·최초 지면 높이 보존을 확인했다. 수평 이동·착지 적분은 코드 검토
범위이며 Client 화면의 넉백을 검증했다고 기록하지 않는다.

메두사는 기존 Run_KoukuFearAndCounterContracts의 실제 실행60검사/failures0,
새 활성 시야 assertion8개를 포함한다. Python2검사, 실제 PowerShell bootstrap6case,
최종 Action2421의 native Parse→Validate→Serialize→Reopen→Validate도 통과했다.
독립 읽기 검토에서 이 신규 GAZE 연결의 재현 가능한 P0/P1은 찾지 못했다.
최종 게시2421 patternbindings4,307,741bytes/114patterns도 실제 native 전체 parser를
통과했다. 이는 Client/GPU 화면 검증과 구분한다.

### G07·G08 최종 빌드와 게시

일반 Product Build(Debug/Release, SkipBuild=false) 모두 PASS다. Debug107331ms의
정본 receipt는 out/BuildPipeline/runs/20260926T133654465Z-debug-product.json,
Release123730ms는20260926T133916814Z-release-product.json이다. Engine→Shared→
Server→Client 컴파일·링크와 구성별 SDK/shader/runtime DLL 배포를 마쳤다.
기존 인코딩/SDK warning은 남아 있으며 오류0이다. Clean/Rebuild나 Client 실행은
하지 않았다.

WORLD→Kouku projector→Gameplay→Composition 공식 Publish가 모두 통과했다.
저장 Action2421=Encounter2421=patternbindings2421=Server Product2421이며,
Sequence182=네 RAIDGATE182다. WORLD2282/Camera94는 source/runtime 의미가
완전히 같고, 게시된 Effect source clock은 정상118키 한 개만 남는다. Server
PATTERNLOGICGAZE2행과 쇼타임30개 피해/넉백 행을 실제 파일에서 대조했다.
JSON10개, 프로젝트 XML4개 parse와 git diff --check를 통과했다. 증거는
out/KoukuDirectionVisibility20260926/final-publication-build-check.json이다.

사용자 실행용 Start-4Clients-Debug.cmd/Start-4Clients-Release.cmd를 같은 out 폴더에
준비하고 구성별 Client.exe 존재·경로를 확인했다. 각 스크립트는 Client/Default에서
기존 EXE4개만 실행하고 빌드·설치를 하지 않는다. 에이전트는 실행하지 않았다.
22:42 KST 공유192.168.0.22:7777 probe는 not-listening이다. 해당 Server PC의
새 바이너리·게시본 시작과 실제4클라 표시/접속/효과 확인은 남아 있다.
로컬 빌드·파일 publish가 원격 Server 프로세스까지 갱신했다고 기록하지 않는다.

최종 Server 실행은 Debug Product333검사와 Release Raid·공포1747검사가 각각
failures0으로 완료됐다. 뒤이어 진행 중이던 중복 Debug Raid 실행은514검사/
발견 실패0 시점에 사용자 재빌드를 위한 Server.exe 점유 해제 목적으로 중단했다.
이 중복 실행을 전체 PASS로 기록하지 않는다. Debug의 새 판정은 위60/642검사,
최종 게시본 소비는333검사로 완료됐다. 증거는 같은 out의 server-contracts.json이다.
22:52 KST 재확인에서도 공유 endpoint는 not-listening이며 에이전트 소유 제품
프로세스는 모두 종료했다.

P5 예외 경계를 마지막으로 좁힌 뒤 Product Build를 다시 완료했다. 최종 Debug
receipt는20260926T135506317Z-debug-product.json(16420ms), Release는
20260926T135558666Z-release-product.json(30987ms)이며 두 설정 모두 PASS다.
Level CPP 한 OBJ씩 재컴파일·Client 재링크·runtime 배포했고 서버·게시 데이터는
변경하지 않았다. 최종67개 컷신 CPU검사와 위333/1747/60/642 Server 검증을
사용하며 서버 회귀를 반복하지 않았다. 마지막 publication/build check는 이 두
최종 receipt와 현재 소스 SHA를 사용해 다시 PASS했고, 확인 시점에 Client/Server/
MSBuild 프로세스가 없어 사용자 빌드와 충돌하는 에이전트 파일 점유가 없다.

## G09. 09-27 저장본 게시와 후속 플레이테스트 수정

사용자가 저장하고 종료한 Action2441/WORLD2282를 백업한 뒤 stable ID별로 병합했다.
Action2442는 1관문 돌진카운터 뒤 P104 7개, 3관문 돌진카운터 뒤 P125 6개를
추가한다. 2관문 나팔 P105 14개를 제거하고 거미카운터 bundle6 뒤 팡파레 P23
7개를 넣었으며 3관문 P40 6개를 제거했다. 수정한 flow group 경계도 기존 entry ID로
갱신했다. 사용자 음향·카드 위치·칼날 삭제·카드미로 회전·레이저 추적·넉백 2000ms·
포탈 연장은 보존했다. saved-fields-audit.json의 65개 차이는 승인된 변경 범위뿐이다.

실제 게시 실패는 World 검증기의 encounter optional schema에 trackBombs가 빠진
경계였다. bounded optional array 검증을 연결했고 실제 GetEncounterProfiles 4검사를
통과했다. 공식 Kouku owner 전체 게시도 sourceRevision2442로 PASS했다. Product
114패턴/9번들/577단계, Map, World, Gameplay 순서를 완료했으며 world.gameplay
8456ms, 전체303531ms다. Gameplay bootstrap은109686행/32375932byte이다.
게시 증거는 out/KoukuReleasePlaytest20260927/publish-owner.log다.

조커 logic557의 DURATION을 BOSS_RANDOM_TARGET과 tracking occurrence P13.presentation.55로
연결하고 이전 trigger3개를 비활성화했다. Server가 랜덤 표적을 선정하며 Client는
저작한 세 구간 동안 머리 표식과 노란 시선만 그 표적을 따라간다. BigSaydon RPCT06의
기존 basis/−90도 보정을 유지하고 Duration 중 보스 실제 yaw는 변경하지 않는다.
종료 시점의 조준점 고정은 아래 G11의 후속 요청으로 연결한다. 이 단계의 실제 Debug
객체의 Kouku bundle123검사, native11검사와 projector1검사를 통과했다. 증거는
out/KoukuJokerPublish20260927이다.

쇼타임의 고정 장판은 source local Y+0.4와 실제 nav 지면 약1.3176m를 대조했다.
STATIC MAP8개만 정상 Append와 같은 Y1.320000052로 높였다. Server-controlled
7개는 nav 지면+상대Y−0.2+sourceY0.4로 약1.5176m이므로 상대값을 유지했다.
사용자 후속 지시에 따라 첫 프레임·이펙트 시간 샘플링 경로는 변경하지 않았다.
공유 asset·shader·밝기·렌더링 옵션도 변경하지 않았다. 실제 Client 표시 판정은 남는다.

마리오는 지정 색 공3개 파괴를 최종 이동과 직접 복귀 양쪽에서 검사한다. 1인은 진입자,
2~4인은 바깥 참가자 한 명에게 표식을 보내고 전체1~2인 메이든0명,3~4인1명이다.
표식 대상은 메이든·사망 상태여도 표시 대상이며 진입자의 사망·실제 복귀·취소 때만
제거한다. Mario4의 같은 색4개 배치도 목표는3개다. protocol114의 marker color를
기존 replication으로 전달하고 Client의 실제 머리 본 위에 원본 계열 이펙트를 만든다.
빨강·파랑·노랑 authored asset3개와 catalog/tree/프로젝트 None 등록을 추가했다.
필요한 DDS3개가 기존 Resources에 있어 새 Resources/GBResources 추가 파일은 없다.
Protocol232검사와 실제 source3색×3particle, 고정 source 시각의 이동 pivot 추종 검사를
통과했다. 화면 검증으로 대신 기록하지 않는다. 증거는 out/KoukuMarioChallenge20260927이다.

추가 실제 Server focused 실행은 새 DebugTeleport fixture 두 블록과 최신 제품 객체를
사용해564PASS/0FAIL이다.1~4인384회 표식 선정·수명과 Mario1~4의 공0/1/2개 탈출
거절,3개 달성 후 terminal flight·착지·표식 제거를 확인했다. 별도 out의 probe만
실행했고 Client나 표준 Server 제품 프로세스를 띄우지 않았다. 증거는 같은 playtest
out의 mario-required-probe-receipt.json이다.

빙고 망치의 설치 모델7396정점·배율1에서 지면 footprint를 계산했다. 기존3.21×4.69m는
축이 바뀌고1.8배가 중복된 값이며 최종은 진행축2.611918×가로축1.785544m이다.
WORLD4개 template의 head collider만 수정해2283을 게시했다. PATTERNBINGOHAMMER와
Debug/Release F1의 Bingo Hammer Colliders가 같은 값을 사용한다. 실제 서버20anchor의
시작 전·경계 안팎·종료6000ms까지161검사 통과. Release에서 Debug 전용 grip API를
참조하던 compile 오류는 guard로 분리했고 망치 head의 공통 샘플 경로는 유지했다.

최종 검사에서 자동 빙고 진입의 기존 synthetic BINGO_BOARD 호출이 새 필수 geometry를
전달하지 않는 제품 회귀를 추가로 발견했다. 빈 trigger 대신 현재 pinned catalog의
빙고 flow에 연결된 실제 BINGO_BOARD를 전달하도록 RaidFlow 호출부만 고쳤다.
수치 fallback이나 Begin 검증 완화는 없다. 수정 전 동일1인 구간20PASS/2FAIL,
수정 후1~4인의 실제 G3 clear→Encore→entry idle→첫 opener→다음 flow/stage에서
98PASS/0FAIL이다. 보드 epoch와 원래 폭탄·망치 deadline 보존을 포함한다. 증거는
out/KoukuBingoIdle20260927이다. 앞서 실행한 broad DebugTeleport는580.2초에 중단했고
arming/preserve4FAILURE를 포함하므로 전체 PASS로 기록하지 않는다. 필수 Mario와
실패한 빙고 구간은 각각 위564검사와98검사로 별도 완료했다.

잡기 P17은 같은 보스·같은 pattern sequence·활성 BOSS_LEFT_HAND hold에 한하여
저작 피해를 허용한다. 원래4개 피해 창·앵커는 바꾸지 않았다. 실제 정상/잡힌 대상
모두1000→600HP이고 다른 owner·과거 sequence·만료·WORLD_HOOK·범위 밖·무적은
1000HP를 유지한다. 일반 world hit 면역도 유지한다. 망치·잡기 증거는
out/KoukuHammerGrab20260927/final-receipt.json이다.

카드미로 진입은 사망자를 시작점으로 이동하면서 HP0/DEAD/전투 불가를 보존한다.
사망자는 추적자나 상호작용 대상이 되지 않으며 navigation 실패는 전체 위치를 rollback한다.
입장 전/컷신 중 사망과 실패 보존을 포함한 실제 Debug Server102검사 통과. MAZE/DANCE
HUD 모드에서 전투·보스·게이지·미니맵 이미지와 텍스트를 숨기고 기믹 입력과 사망 복귀창은
유지한다. Client/UI는 실행하지 않았다. 카드미로 증거는 같은 playtest out의
card-maze-receipt.json이다.

## G10. 주사위 동문양 접촉 해제 누락

현재 게시 P78은 이미 자유1/속박N−1을 선택하고, 카드가 자유 참가자를 추적하면서
모든 판정 가능 플레이어에게 접촉하며 다른 문양은 최대HP90% 피해를 주고 있었다.
이번에 재현한 실제 누락은 동문양 접촉이 피해만 면제하고 Clear_PatternBindStatus를
호출하지 않는 점이다. 수정 전 같은 실제 함수 검사에서102PASS/10FAIL을 확인했다.
접촉 지점에서 카드의 boss owner와 pattern sequence가 일치하는 동문양 속박만
해제하도록8줄을 추가했다. 자유 대상 선정·Shared ABI·Data를 다시 만들지 않았다.

수정 후 실제 게시 P78의1~4인 시작 경계, 두 번의 Update_Players와 C2S_MOVE,
실제 pursuit spawn과 속박 참가자의 가로막기, 동문양0피해/다른문양90%, 다음 tick
해제 유지와 정확한 창 종료, 다른 owner/sequence의 속박 보호가160PASS/0FAIL이다.
Client 실제 Update_DiceBindVisuals 소비32검사와 projector1검사도 통과했다. 속박N−1과
해제 효과,6초 종료, 메이든 제외를 확인했다. 증거는 out/KoukuDiceServer20260927과
out/KoukuDiceClient20260927이다.

과거 네 명이 처음부터 전원 고정되고 이펙트도 없던 동일 세션의 원인은 확정하지 않았다.
정상 초기 상태는1~4인 모두 자유1명이고, 유효한 다른 owner의 선행4속박은 주사위
참가 자격이0명인 별개 상태였다. owner sequence가 끝나면 다음 player tick에 모두
해제됨을 확인했다. G03의 메이든 이펙트 분리와 G07의 전체 presentation 로드 오류를
과거 주사위 전원 고정의 직접 원인으로 기록하지 않는다. GPU/4클라 화면은 사용자 확인이다.

### 주사위 외곽 링 지속 재생 추가 수정

사용자가 제공한 원작 이미지의 원형 테두리를 빙고 망치 생성광과 대조했다. 주사위는
Spotlight01의 fm_d_ring_013/native2841, 빙고는 HammerAura01의 별도 ring sprite와
fm_d_ring_008/native2869이므로 같은 source element가 아니다. 속박 floor의 원본11요소는
누락 없이 설치돼 있었으나 지속 링 emitter6/23의 생략된 Required EmitterLoops 기본값0을
기존 importer가1로 투영했고, runtime hold occurrence도 loop-to-duration을 켜지 않았다.
새로 디코드한 Engine Required→ParticleModule→Core Object CDO chain으로0을 확인했다.

floor의 두 stable element에서 loopCount1→0만 바꿨고 실제 차이는2byte다. Client는
hold에만 기존 bLoopEffectToDuration을 켜는1줄을 추가했다. 기존 Sample의 bounded
source-loop60초를 사용하며 Server의 속박 해제가 먼저 도착하면 즉시 finite release로
넘어간다. 원본 단발 emitter3, 나머지9요소, mesh·재질·크기·색·방향·release·빙고는 보존했다.
두 파일은 최신 SHA 재확인·백업·원자 교체와 자기 변경 rollback 경로로 설치했다.

실제 native CPU 비교에서 현재본은4초부터 링6/23이0이고 loopCount만 바꾼 후보도5초부터0이다.
두 수정의 조합은5/8/10/14/20/59.5초에 링2종이 각각4개 유지된다. 알파·크기·행렬이 유효하며
emitter3은3초부터0으로 재시작하지 않는다.60초는 기존 occurrence 제거가 객체를 정리한다.
Client actual 함수 검사는 hold 반복/finite release를 포함해40PASS로 갱신했다. 증거는
out/KoukuDiceRing20260927/native-comparison-receipt.json 및 installation-receipt.json이다.

공식 전체 Effect source validator도 실행했으나 기존 v15
effect.kouku.gate1.blade-dance.circle.impact에서 선행 검증기 과잉 제약으로 실패했다.
해당 파일은 HEAD와 의미·정규화 bytes가 같다. Python은 v15에 explicit runtimeCarrier가
없으면 무조건 거절하지만 실제 native Codec은 일반 sourceRecipe particle을 허용한다.
같은 기존 자산의 native Load→Stage→Seek는 성공했고0.5초에85입자를 확인했다.
이 전체 검사를 PASS로 기록하지 않는다. 변경한 링·마리오 asset의 native 검사와 공식
Kouku owner 게시 결과는 별도로 위에 기록했다. 이번 변경 밖의 asset/검증기를 수정하지
않았다. 분류 근거는 v15-existing-classification-receipt.json이다.

## G11. 조커 Duration 종료 위치의 고정 조준

이전 BOSS_RANDOM_TARGET_PRESENTATION은 시작 cue에서 ID를 선택했지만 기존
PlayerTargetWindows에 등록되지 않아 종료 소비자가 없었다. 같은 typed trigger를 기존
ledger에 연결하고, 시작에 선택한 동일 ID를 유지한 채 정확한 END tick에서 현재 XYZ를
한 번 저장한다. 이후 플레이어 이동과 일반 stage retarget은 고정 조준을 덮지 않는다.
다음 Duration과 Brain Begin/Finish·실제 Abort는 occurrence 소유 yaw를 복구한다.
표적이 죽거나 퇴장하면 마지막 유효 좌표를 사용하며 다른 플레이어를 새로 고르지 않는다.

사용자가 저장한 구간0..2658 /6244..8902 /12587..17910ms는 그대로다. 기존 snapshot의
boss yaw로 원본 망치 궤적과 사거리를 조준하며, 망치 모델·Effect·BOSS_CURRENT Collider는
같은 WEAPON b_rpct_01/root를 소비한다. 목표 XYZ로 무기를 평행 이동하거나 본체를
순간 이동시키지 않는다. Shared/Data ABI 추가는 없고 protocol114를 유지한다.

새 Debug의 전체 종속 객체를 사용한 실제 게시 P13 room/stage 검사는21PASS/0FAIL이다.
시작tick1 기준 END81/269/539에서 capture하고 실제 망치 stage148/339/609까지
플레이어를 이동시켜도 pin 유지, 다음 window와 종료/취소/새 패턴의 원복을 확인했다.
같은 새 객체의 Run_KoukuBundles126PASS/0FAIL은 기존 즉시 random target·retarget·
root motion·bundle/Complete Play reload도 포함한다. Client actual interpolation과
Random_TargetWindow/Pivot34검사도 PASS다.17개 관련 occurrence의 공통 weapon root
연결을 대조했으며 packet/GPU 화면 검증으로 대신 기록하지 않았다. 증거는
out/KoukuJokerAim20260927의 p13-receipt.json, server-receipt.json, native-receipt.json이다.

### G09~G11 최종 빌드·게시·실행 준비

주사위 링과 조커 END 조준까지 포함한 일반 Product Debug/Release Build 모두 PASS다.
최종 Debug43837ms receipt는 out/BuildPipeline/runs/20260926T174302617Z-debug-product.json,
Release65373ms는20260926T174434068Z-release-product.json이다. SkipBuild=false이며
Engine/Shared/Server/Client 컴파일·링크·runtime 배포가 끝났다. 기존 인코딩/SDK/PDB
warning은 남고 오류는0이다. Clean/Rebuild와 Client/UI 실행은 하지 않았다.

최종 공식 게시 join21검사 PASS: Action2442=Encounter/patternbindings/Server Product,
WORLD2283/Camera94는 source/runtime JSON 의미 일치, Sequence182 input SHA 일치다.
공식 owner4개 receipt의 입력718개/출력41개 size·SHA가 모두 현재 파일과 일치한다.
링 floor는 CLAUDE 계약대로 Data/Effects/Authored를 직접 소비하므로 별도 복사·publish
대상이 아니다. 변경 JSON27개·프로젝트 XML2개 parse와 git diff --check도 PASS다.
위 전체 Effect validator의 기존 과잉 제약은 이 결과와 구분한다.

out/KoukuReleasePlaytest20260927에 Start-4Clients-Debug.cmd와 Start-4Clients-Release.cmd를
준비했다. 두 스크립트는 Client/Default에서 해당 구성의 기존 EXE4개를 실행하고
LOSTARK_SERVER_HOST=192.168.0.22를 사용한다. 에이전트는 실행하지 않았다.
02:45 KST 공유 endpoint192.168.0.22:7777은 not-listening이다. 공유 Server PC도 새
protocol114 바이너리와 게시본으로 실행해야 하며 로컬 publish가 원격 process를 갱신한
것은 아니다. Client/Server/MSBuild 표준 제품 프로세스 점유가 없는 상태를 확인했다.
최종4클라 화면·입력·망치/장판/링 표시 판정은 사용자 확인으로 남는다. 최종 확인서는
같은 폴더의 final-ready-check.json, final-publication-check.json, final-file-validation.json이다.


## G12. 카운터·무력화 성공 폰트

Server의 일반 stagger gauge break와 쿠크 HP 기준 STAGGER_WINDOW close edge를
isStaggerSuccess로 전달한다. 카운터의 기존 isCounterSuccess와 독립적이므로 같은 hit의
두 성공도 각각 표시된다. DAMAGE_EVENT의 실제 피해량0을 유지한 성공 pulse를 허용하고
incoming/HEAL/ABSORB의 잘못된 성공 조합은 packet validation에서 거절한다.

CombatHUDViewModel이 피해0 성공을 보존하며 MainApp의 기존 Font_EventDamage 경로가
파란색 카운터(RGB .15/.55/1), 노란색 무력화(1/.9/0)를 그린다. 피해 숫자 표시 OFF에서도
성공은 유지하고, 한 tick의 많은 hit가 성공 문구를 FIFO에서 지우지 않도록 일반 숫자를
먼저 제거한다. 기존 피해·DPS 수치에 성공용 가짜 피해를 추가하지 않는다. 새 세션의
낮아진 serverTick은 기존 렌더 cursor를 초기화한다. 설치된 YoonGasiIIM.spritefont의
11361 glyph를 읽어 여섯 한글 글자 존재를 확인했으며 새 폰트 resource는 없다.

wire는 protocol115다. 같은 변경의 Mario hit source도 이 버전에 포함한다. Shared와
Server/Client를 같은 버전으로 빌드해야 한다. NetworkProtocolHarness Debug 정상
증분 Build와 실제 실행은1296 PASS/failures0이다. 프로토콜114에서 추가된 marker-color
1byte가 빠진 기존 snapshot 크기 기대값도 보정했다. 증거는
out/KoukuCombatFollowup20260927/protocol-build.log, protocol-test.log다.

## G13. 문양 출생 위치와 중앙 포탈12개

P47의 문양 presentation.2/.3은 실제로 MAP 절대좌표였다. 두 effect occurrence를
bone 없는 BOSS, positionOffset0, followBoss=false로 바꿨다. Client는 첫 표시 frame의
현재 보스 위치가 아니라 occurrence 시작 시각의 root history/exact root를 한 번 읽고
고정 pivot을 사용한다. 이후 bone pose·보스 이동으로 effect를 갱신하지 않는다. 기존
크기·회전·sourceStart4201ms·시간은 유지했다. 해당 문양에는 플레이어 위치로 생성하는
활성 SELECT 장판 Logic이 없으며, 서버 판정 우회나 두 번째 이펙트 런타임은 추가하지 않았다.

사용자 추가 요청의 마리오1~4 1페이즈는 P88/P91/P92/P93이다. 각3개 포탈을 같은 패턴의
presentation.2 큰 중앙 오망성 위치(-0.0700000003, 1.3200000525, 942.3300170898)로
맞추고 MAP/followBoss=false로 저장했다. 총12개다. Action2442→2443이며 최신 저장본의
해당 필드만 병합하고 교체 직전 hash 확인·백업·원자 교체를 수행했다. 마리오 FXAA OFF를
포함한 렌더링 설정은 변경하지 않았다. 증거는 같은 폴더의 anchor-changes.json이다.

## G14. 알비온 대상과 마리오 전투 사운드 격리

공용 boss random selection, 기존 target 재조회와 pursuit, 알비온의 SELECT/APPEAR/
BLUE_CIRCLE에서 iMarioStage가 있는 플레이어를 제외한다. 전장에 유효 대상이 없으면
target ID를 비우고 보스의 현재 navigation ground를 고정 표적으로 사용한다. 선택 후
플레이어가 마리오로 이동해도 재검증하므로 마리오 위치를 추적하지 않는다.

로컬 Mario stage가 있으면 Composition SOUND의 재생 중 handle을 중단하고 해당
occurrence를 소비한 상태로 유지한다. CWorldSequencePlayer의 기존 soundTracks와
retired tail에도 audience callback을 적용했다. 공유 전장 시각·시계는 계속 진행하며
마리오 intro는 입장한 해당 stage 참가자만 듣는다. Kouku NPC hit reaction도 추적 가능한
sound handle로 바꾸어 입장 시 중단한다. 마리오 자체 공격·피격과 BGM은 별도 소비자를
유지한다. 일반·child·bundle·targeted·FEAR sound, owned WORLD와 tail의 실제 호출 경로를
읽어 대조했으며, 이 검토를 실제4클라 청취 결과로 기록하지 않는다.

## G15. 1관문 선행 트리거 원본 사운드

원본 SCENE03A의 RemoteEvent37081_113→Matinee7/InterpData862→Track1239가
circuspopup을200ms에 재생한다. 기존 팝업북 Matinee0의 movetoinsideofcircustent와
별개인 선행 트리거여서 기존 컷신 sound import에 빠져 있었다. circus_finale WORLD
template에 sound.kouku.source.circusfinale.circuspopup 한 soundTrack을 추가했다.
WORLD2283→2284이며 다른 animation/camera/visual timing은 유지했다.

Wwise event814076959→Play503324210→Layer93205929의 두 원본 media642900640/
592867755를 원본 동시 layer로 복원했다. Resources-relative 설치 위치는
Sound/KoukuSaton/Events/scene_midnightc_ed_circuspopup.source.wav, 길이10267ms다.
기존 WorldSequence soundTracks 소비자의 seek/pause/stop/instance 수명으로 재생한다.
다른 컷신 재생 경로나 중복 Composition SOUND를 추가하지 않았다.

공식 WorldSequences Validate와 KoukuSaydon owner의4개 domain 게시 모두 PASS다.
게시 로그는 out/KoukuCombatFollowup20260927/publish-kouku.log이며
Gate1 원본·Wwise·물리 설치 증거는 out/KoukuGate1TriggerAudio20260927에 있다.
원본 추적과 decode·설치는 실제 스피커에서 들리는 시점·음량의 사용자 확인과 구분한다.


### G14 후속. 다른 플레이어와 에스더 전투음

사용자의 추가 확인에 따라 Character의 일반 skill, vehicle skill/locomotion/mount,
interaction sound5개 재생 지점을 Play_CombatSound로 모았다. 로컬 Mario는 자신의
소리를 듣고, 비로컬 Character는 로컬 HUD의 Mario 상태에서 재생하지 않는다. Character가
소유한 SoundCue handle만 정리해 입장 전부터 재생하던 소리도 중단한다. cue timeline과
submitted 상태는 계속 진행하므로 복귀 시 밀린 소리가 재생되지 않는다.

Esther PLAYER_ACTION ESTHER_CAST와 NPC_ACTION은 별도 Play_Due 소비자이므로
같은 audience 검사와 서비스 소유 handle 정리를 추가했다. Kouku Level은 replication
적용 직후 tail 정리를 호출한다. 입장 snapshot에서 local player를 remote action보다
먼저 적용해 배열 순서로 이전 stage0를 읽는 문제를 닫았다. 초기 network-state 미준비를
청취 허용 조건으로 사용하지 않아 새 remote의 첫 mount sound도 차단한다.

실제 Character의 새3함수를 추출한 CPU 검증17PASS/0FAIL과5개 호출 경로 대조를 완료했다.
이는 mock audio handle 기반 조건·수명 검증이며 실제4클라 청취 검증을 대신하지 않는다.
증거는 character-audio-receipt.json, character-audio-test.log다.

### G15 후속. 마리오 표식과 원본 공격·피격

Server는 마리오1~4에 동일한 색·표시 상태를 전달하고 있었다. Client가 세 색 표식을 모두
product prewarm하고, animated head matrix 대신 actor translation+고정2.45m 높이와
identity rotation을 사용한다. 색별 준비와 bone 의존을 제거했으며 보고된1/4의 최종 표시와
전 클래스에서의 높이는 사용자 화면 확인으로 남긴다.

세 색 공은 실제 MN_PPCC_00 death4194520/21/22의 Par_X_PPCC_Expl_01/02/03 원본
각10emitters를 복원해 기존 smoke를 교체했다. pop mask의 새 edge가 한 번 생성하며
입장 전에 이미 터진 공은 숨기기만 한다. 비행 공은 Server의 실제 접촉만 FLYING_BALL로
표시하고 shield absorption에도 source를 보존한다. 일반 피격을 비행 공으로 추측하지 않는다.
비행 공의18emitters는 기존 원본 RHCN 폭발을 fuse/model 없이0초로 잘라0.22배 적용한
프로젝트용 피격 표현이다. 해당 원작 비행 공과 동일한 asset이라고 주장하지 않는다.

원본 Mario는 Polymorph4166/MN_REUP_07의 Q42784/W42785이며 일반 광대 MN_RPCZ의
폭탄·나팔과 다르다. 실제 Q의4개 FX 시스템25emitters와 JumpClown1 음원을 사용한다.
현재 프로젝트의 body/망치 clip은 유지하고 원본Shot500ms를 현재 contact400ms에 맞춰
notify 시계0.8만 적용했다. 원본 notify TRS·color/lifetime/rotation-rate override는 보존했다.

원본 StartControl/weapon socket이 현재 body에 없어 실제 설치 WP_MN_RHKP_07_Static과
원본 weapon의1002개 같은 UV 정점을 대조했다. hand-frame cook 변환의 최대오차는
7.0554e-8m이며 원본 handedness 변환1회와 설치 pitch220도/preScale1.313을 적용했다.
실제 body의 bip001-r-hand에2개 follow FX를 retarget하고 다른2개는 root snapshot이다.
이 수치·bone 존재 검증과 최종 움직이는 본 부착·GPU 화면 판정은 분리한다.

native program5140~5168 총29개와 distortion companion14개를 기존 carrier에 연결했다.
공 폭발의 exact PS70820928f0b4b14486394a81b0f88c89만 기존 decal 입력에 없던 receiver
normal과 projector 두 축을 공급한다. 해당profile5140에만 적용하며 다른 decal에는
전파하지 않았다. source instruction SHA와 CB0 row8/9의 원본 축 부호를 대조했다.
mesh carrier5120도 Client project/filter와 기존 shader registry에 등록했다.

최종5개 effect/73emitters의 공식 validator 내부 field 검사와 resource closure는 PASS다.
새 Q 교체 뒤 재검사도 PASS다. 신규 음원은37WAV/30,154,228bytes(관문1+Mario36),
effect 의존 Resources는82파일/8,606,816bytes다. 모두 설치본과 Desktop/GBResources의
동일 상대경로 SHA256이 일치한다. authored JSON과 shader code는 Git 소유다.
초기 일반 광대 음원·다른 망치 후보는 이번 작업의 추가분만 확인해 제거/교체했다.

최종 sound는 Q/W 각3events와 Damage/Down/StandUp/Death, 공 파괴를 포함한다.
기상은 살아 있는 로컬 Mario의 knockdown 종료 edge에서만 재생한다. Character의
optional soundCues는 검증→stage→기존 binding 교체를 유지하고 승인된 action 시계를 쓴다.
증거는 out/KoukuMarioPresentation20260927/RESULT.md, hammer-source/weapon-retarget.json,
hammer-fx/retarget-receipt.json, fx-resource-install-receipt.json 및
out/KoukuCombatFollowup20260927/audio-resource-final.json, effect-focused-validation.json이다.

### G12~G15 서버 검증과 최종 게시

Server/Shared Debug 증분 Build는27.96초/경고0/오류0이었다. KoukuSupportSurface328PASS,
SkillStages64PASS이며 failures0이다. 첫 KoukuSupport 실행은 후반 catalog 검증의
LOSTARK_SERVER_DATA_ROOT 미설정으로 실패했고, 현재 게시본과 SHA가 같은 격리 root를
지정해 전체 재실행했다. 실패 로그를 보존했으며 새 gameplay 검사를 건너뛰지 않았다.

공식 Kouku owner4개 domain 게시 뒤 최종 Effect/Sound catalog로 gameplay.balance를
다시 게시했다. Action/Encounter/bindings/bootstrap2443, WORLD2284가 일치하며 포탈12개와
P47장판2개의 저작/투영 필드도 일치한다. 공용 catalog를 포함한 Valtan presentation generation
106edb25186d63f83c038fe3ac14f41b4a2948e268a2b23c6a53361798c31629의146artifact size/SHA가
현재 파일과 일치한다. 이전 generation과의 변경은 기존 main의 에스더와 이번 Mario의
공용 catalog2개뿐이며 Valtan gameplay를 수정한 결과가 아니다. 증거는
publication-check-final.json, publish-gameplay-final.log, server-test-receipt.json이다.


### G15 native CPU 검사

최종5종 effect를 현재 Debug 객체로 native Codec Load→Playback Stage→Seek했다.
0/.05/.1/.2/.4/.8/1.5/3/5초×5종 모두 성공했고 world/color/basis-scale nonfinite는0이다.
누적 particle sample은 빨강215/파랑216/노랑215/비행피격557/망치76이다. 문서SHA는
검사 전후 동일하고 linked Debug source/header dependency stale/missing도0이다.

Q는 실제 socketLocalTRS와 identity bone parent fixture를 공급한 CPU 검증이다.
0.4초11/0.8초52/1.5초13입자가 있으나 실제 animated bone 부착과 GPU 표시는 사용자
확인으로 남긴다. 일부 짧은 emitter는 샘플 시각 사이에 끝나 원소별 range가 null이며
이를 모든 원소의 표시 성공으로 확대하지 않는다. 증거는 mario-native-probe-receipt.json과
종별 mario-native-*.jsonl이다.


### G12~G15 최종 제품 빌드·전달 상태

최종 일반 Product Debug/Release Build 모두 PASS이며 SkipBuild=false다. Debug는
20260926T193822443Z-debug-product.json(13384ms), Release는
20260926T195311159Z-release-product.json(831422ms)이다. 초기 Debug의100개 셰이더/
201개 OBJ 컴파일 뒤 사용자 추가 청취 범위를 반영한 C++ 증분을 완료했다. 최종 Release는
100개 CSO/205개 OBJ를 갱신했다. Clean/Rebuild는 실행하지 않았다. 공용 shader include
변경으로 Shader_VtxAnimMeshBinary까지 다시 컴파일되어 빌드 시간이 길어졌다.
기존 인코딩·형변환·PDB·HLSL 경고는 남으며 오류0이다.

Engine/Shared/Server/Client의 컴파일·링크·배포, Engine DLL 설치 hash 일치, 새5120
mesh/particle 및 decal CSO와 양쪽 EXE 존재를 확인했다. 변경 JSON15개/XML2개 parse,
기존 Client C++17개 인코딩/CRLF 보존과 git diff --check도 PASS다. 최종 receipt는
out/KoukuCombatFollowup20260927/final-build-ready.json과 changed-file-validation.json이다.

Client/UI와4클라를 에이전트가 실행하지 않았다. 마리오1~4의 cold 표식, 이동 중 표식 높이,
실제 망치 본 부착과 크기, 관문 진입음 동기·음량, 마리오/본진4클라 청취는 사용자 화면·
청취 확인으로 남긴다. Server/Client 모두 protocol115의 새 실행 파일과 게시본을 사용해야
하며 로컬 파일 게시가 원격 Server process를 갱신한 것은 아니다. 기존 사용자 RESULT의
미커밋 기록을 보존하고 이번 G12~G15 추가 기록만 기능 커밋에 포함한다.
