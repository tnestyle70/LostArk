# 쿠크 사운드·장판·마리오·HUD 후속 구현 결과

## G00. 적용 기준과 현재 상태

사운드·장판·마리오·HUD 구현과 리소스 설치·게시, 공식 Product Debug/Release 빌드를 완료했다.
머리의 pose 변형 보정은 적용했지만, 사용자가 지적한 두피 밀착/볼륨 소실의 원인·해결은
확인되지 않았다. 이 둘을 같은 완료로 기록하지 않는다.

사용자가 반영을 승인한 최신 저장 Composition revision2465를 기준으로 요청 필드를 병합해
2466으로 저장했다. 기존 미커밋 편집을 보존했으며 다른 패턴·렌더링 옵션을 바꾸지 않았다.
WorldSequence는2284→2285다. 기존 installer의 backup·교체 직전 hash·ReplaceFileW displaced
검사·자기 변경 rollback을 사용했다. `out/KoukuAudioMarioHud20260927/integration`에 실제
baseline/candidate/변경 field/설치 receipt를 기록했다. 작업 브랜치는 기존
`GB/Valtan-Patttern-Complete`, 시작 HEAD는369987261a1e99c1de98a370c26b69acf3fc7023이다.

## G01. 원본 사운드와 그룹별 재생

현재 설치된 원작 Korean Wwise SOUND_MOB_GLOBAL3 은행의 실제 event/layer/random/gain을
해석해 11개 event, 197개 WAV를 생성하고 CharacterSoundCatalog에 연결했다. 새 Composition
SOUND145개와 기존 발사음7개 수정, resource7개를 반영했다. 원본 notify만 추출하던 누락을
Projectile event 기준으로 보완했다.

| 구간 | 연결한 이벤트/정책 | SOUND 발생 수 |
|---|---|---:|
| 쇼타임 사각 세 폭발 | G_KoukuSatan1_Attack33_ProjExp1 | 30 |
| 사각 공습탄 비행 | G_Satan1_Attack05_Proj1, 원본 visual보다 지연된 AkEvent 시각 유지 | 30 |
| 고정 폭탄 착탄 | G_KoukuSatan1_Attack11_ProjExp1 | 15 |
| 총구 발사 | G_KoukuSatan1_Attack23_Cast3 | 14 |
| 남는 불 장판 | G_KoukuSatan1_Attack23_Proj1 | 15 |
| 추적 구간 | G_KoukuSatan1_Attack23_Proj2의 5초 cycle을 소유 구간에 연결 | 3 |
| 원형·도넛 | G_Satan1_Attack05_ProjExp1 / Attack06_ProjExp1 | 5 / 17 |
| 무지개 격자 | Attack11_ProjExp1, 두 grid 각각10개 원본 projectile 시각 | 20 |
| 팡파레 도넛 | 사용자 허용 fallback인 G_Kouku1_Attack07_ProjExp1 | 3 |

표의152회는 신규145개와 기존7개 수정의 합이다. 고정/무작위 타겟의20개 신규 SOUND는
원본 occurrence ID를 유지하는 일반 lane 대신 publisher의 targeted visual template으로
정규화된다. 실제 게시 template payload를 다시 비교해 이20개와 일반 lane125개를 확인했다.
무지개에서 한 Projectile이 사용하는 좌우/복수 emitter를 중복 발음시키지 않는다.
팡파레의 기존 나팔/발사음은 유지했고, 별도 도넛 폭발 event를 확인하지 못해 피자 착탄음을
6206/6703/7323ms Collider 시각에 추가했다.

일반·붉은 톱날은 네 World template에 시작음과 2초/3초 지연 반복음을 별도 lane으로 연결했다.
원본 시작음까지 매번 반복하는 대신 반복 layer에만 optional `loopToDuration`을 적용한다.
톱날 반복 layer는 현재 원본 첫 variant를 사용하며 Wwise의 매 cycle random 재선택이나
AvoidRepeat 상태까지 복제한 것은 아니다. full-event catalog는 native branch weight를 보존한다.
실제 `Apply_Sounds`, `Retire_Sounds`, pause·rate·seek 소비자와 C++ load/save 및 Map publisher를
확장했다. motion 교체/NEXT/cutoff·수명 종료·Stop에서 반복음을 정리하고 기존 one-shot tail은
유지한다. 현재 반복은 미디어 cycle을 재생 clock의 modulo로 다시 여는 방식이다.

근거: `out/KoukuProjectileAudio20260927`의 source/render/install receipt,
`candidate-summary.json`, `sound-runtime-test.log`, `installer-test.log`.
새 installer는 같은 hash WAV를 재교체하지 않고 새 WAV의 동시 생성과 catalog의 늦은 저장을
보존한다. 의도한 충돌·rollback fixture가 통과했다.

## G02. 고정 무지개 장판과 사각 낙하탄

P38 두 grid는 P47 문양장판과 같은 `BOSS + followBoss=false`로 시작 보스 pose를 한 번
고정한다. MAP 절대원점으로 바꾸지 않았다.20개 Collider도 각 grid의
`anchorPresentationOccurrenceId`를 공유하고 followBoss=false다. 게시된 Server bootstrap에서
captureStartMs3359/8900이 각각10개임을 확인했다. 사용자 offset/회전/scale와 Collider 시각은
보존했다.

사각 폭발 그룹에 기존 원본 mesh/material을 쓰는 미사일3개를 추가했다. 기존48개 element는
내부 startDelay만1초 늦췄으며10개 Composition occurrence를1초 앞당기고 수명을1초 늘려,
폭발과 Collider의 실제 시각을 유지했다. 미사일은 각각 X/Z=[0,-6],[0,0],[0,6]으로 고정하고
10m 위에서 원본 가속도 -20m/s²로 떨어진다. 파생 복사본만 수평 scatter를 제거하고 한 번
발생·1초 수명·폭발까지 alpha유지로 설정했다. 독립 원본 leaf는 변경하지 않았다.

실제 Effect codec/playback의 CPU1603개 particle sample은 모두 finite다. 세 미사일의 X/Z가
일정하고 Y가 감소하며 마지막 sample은 바닥 부근(-0.167..0.167m)이다. 기존48개 element는
clock 이외 모든 field가 동일함을 비교했다. GPU 화면이나 최종 연출 체감의 검증은 아니다.
근거: `out/KoukuAudioMarioHud20260927/effects`와 `integration/semantic-validation.json`.

## G03. 마리오 참가자와 색상

`KoukuSaydonPresentationPlayer`는 MARIO_PHASE2_PLAYERS trigger가 있는 패턴의
`boss.kouku.curtain_1`을 로컬 Server-admitted Mario stage1~4 참가자에게만 중지·소비한다.
외부 플레이어·Preview·다른 커튼은 유지한다. 실제 소비 조건384조합과 TU 컴파일을 통과했다.

공 색상은 기존 Server의 실제 입장 경로가 이미1~3 uniform random으로 새로 선택하고 있다.
별도의 Client 난수나 중복 Server 로직을 추가하지 않았다. Mario1~4 각96회, 총384회 입장과
1544검사에서 세 색 선택·입장 중 유지·퇴장 제거·다음 입장 재선정·marker 전달이 통과했다.
임의 선택이므로 연속으로 같은 색이 나오는 것은 가능하다.
근거: `out/KoukuMarioParticipant20260927`.

## G04. 체력바·무력화 표시와 위치 저장

F1 Health bar positions를 무력화·아군·일반 몬스터·쿠크세이튼·쿠크·발탄의 여섯 X/Y 묶음으로
확장했다. 일반 몬스터에는 카드 병정과 진입지점 몬스터가 함께 속한다. 기존 Y 값과 무관한
HUD 설정을 보존하며, 옛 문서에 보스별 값이 없으면 기존 enemy X/Y를 상속한다. 첫 저장에서
개별 값을 명시해 이후 일반 몬스터 조정이 보스 값을 바꾸지 않게 했다. 축별 충돌 검사와
최신 JSON 병합·backup·atomic save·실패 보존을 유지했다.

거대 세이튼의 작은 HP는 항상 숨기며, 카드미로에서는 플레이어와 네 문양 카드 병정의 작은
HP를 숨긴다. 안정적인 archetype ID를 읽기 전용 HUD 상태에 전달해 카테고리를 구분한다.
발탄 마력구 active window는 기존 Server response/stagger 진행을 주황 무력화 게이지로
표시한다. 쿠크1관문·Mario2페이즈·Bingo와 같은
`UI/BossUI/boss_bar_fill_orange.png` 및 중립 tint, 공통 무력화 X/Y를 소비한다. 해당 PNG
참조는 기존 저장본에도 올바르게 존재했으므로 JSON을 불필요하게 다시 바꾸지 않았다.

production 함수 본문으로 구성한 CPU58검사, 네 변경 TU 컴파일, JSON/리소스 해시 검사가
통과했다. save/reload·카테고리·축별 외부 병합·충돌 보존·maze/BigSaydon 필터·발탄 종료·빙고
상태 보존을 포함한다. 근거: `out/KoukuHudGrouping20260928`.

### G04 후속 — 카메라 이동 시 체력바 앵커 오차

작은 NPC 체력바는 현재 프레임의 NPC transform과 camera View/Projection을 사용하며,
sprite 위치도 즉시 반영된다. 조사한 경로에서 한 프레임 지연이나 camera의 중복 적용은
발견되지 않았다. 문제는 `BoundsHead`가 skin palette 적용 전 bind 정점을 실제 머리 위치로
사용한 것이다. WMeshReader는 raw position을 유지하고 bind bounds는 그 position에
preTransform만 적용한다. 실제 화면 모델은 inverse bind와 현재 bone pose까지 적용한다.

설치 G1 쿠크의 기존 앵커는 root 위 0.061292m, idle `bip001-head`는 2.285386m이며,
실제 idle mesh의 최고점은 3.32294m다. G2 쿠크는 기존 0.058006m 대 head 1.620339m다.
잘못된 월드 기준점과 실제 머리를 투영하면 camera 이동에 따른 화면 위치 차이가 생긴다.
현재 저장된 X/Y 값은 모두 0이므로 저장 offset 보정이 원인이었다고 기록하지 않는다.
설치 모델 해시·9개 idle head sample·3개 실제 skinned mesh bounds와 camera 해석 샘플은
`out/KoukuHealthAnchor20260928/anchor-evidence.json`, 재현은 같은 폴더의 `inspect_anchor.py`다.
저장된 camera 방향/FOV를 사용한 해석 예시에서 camera가 옆으로 4m 이동할 때 G1 쿠크의
기존 anchor와 head 사이 상대 X 오차가 약 40.41px 변한다. 이는 사용자 라이브 화면 측정이
아닌 설치 모델 기반 수치 예시이며, 새 helper는 실제 head 자체를 투영한다.

`WorldHealthBarView.cpp`의 CNpc anchor만 현재 `bip001-head` combined matrix의 위치를
NPC world로 변환하도록 고쳤다. bone에 이미 포함된 preScale를 다시 곱하지 않는다.
head bone이 없을 때만 기존 bounds fallback을 사용하고 non-finite anchor는 표시하지 않는다.
플레이어·발탄·화면 상단 고정 보스 HUD 및 기존 X/Y 저장값은 변경하지 않았다.
작업 전부터 있던 위치 그룹/숨김 조건 등 다른 미커밋 변경도 보존했다.

해당 TU MSVC 14.44 `/Zs /Y-`와 `git diff --check` PASS. 컴파일 로그는
`out/ValtanTimelineReadability20260927/healthbar-compile.log`다. 이번 후속 변경은 사용자 요청에
따라 제품 빌드·링크를 하지 않았고 Client/UI 화면 확인도 수행하지 않았다. 위의 앞선
Debug/Release 빌드 완료 기록은 이번 앵커 수정이 들어간 실행 파일의 완료를 뜻하지 않는다.

### G04 후속 — 주황 fill 복구·무력화 전용 크기 (2026-09-28)

보라색 원인은 scene rendering option이 아니라 UI 겹침이었다. `BossUI.json`의 주황
`Boss_StaggerFill` 뒤에 같은 rect의 불투명 `Boss_StaggerTrack`이 생성되고 같은 layer의
stable sort가 그 순서를 보존한다. 실제 track PNG 중앙은 RGBA(145,114,216,255), 주황
PNG 중앙은 (198,89,24,255)였다. UI는 scene final 뒤에 그려진다. 기존 PNG 참조·중립 tint만
확인한 앞선 검증으로는 이 덮임을 발견하지 못했다.

`MainApp.cpp`에서 보라 track을 항상 숨겼다. 활성 기믹 중에는 기존 빈 배경을 유지하고
주황 fill이 `min(current, maximum) / maximum`으로 줄어든다. 0이면 fill만 숨기고 활성
구간의 빈 바는 남는다. 서버 기믹이 끝나면 기존 조건대로 행 전체를 숨긴다. 발탄 마력구,
쿠크 1관문, 마리오 2단계, 빙고 블랙홀의 상태·수치·표시 구간은 변경하지 않았다.

사용자 요청대로 기본 폭은 기존의 1/3, 두께는 기존 그대로다. 기준 1280×720에서 frame
폭은 496→165.333px, fill은 493→164.333px다. `MainApp.h/.cpp`에서 원본 세 slot의
rect를 한 번 보관하고 frame 중심 기준으로 함께 scale한 뒤 기존 X/Y offset을 적용한다.
다른 HP·광기 게이지 크기는 그대로다. Debug/Release F1 `Health bar positions`의
`Stagger width`와 `Stagger thickness`는 0.1..3배를 각각 조절한다.
`Save positions and stagger size`/`Reload saved positions and size`로 기존 위치와 함께
저장·재로드한다. optional `mechanicWidthScale`/`mechanicHeightScale`이 없으면 1/3과 1이다.
최신 디스크의 독립 필드 병합, 동일 필드 충돌·공유 lock·재검사·백업·원자 교체를 유지한다.

이번 수정은 MainApp 두 파일이며 새 C++/프로젝트 등록은 없다. 저작 JSON과 PNG는 직접
변경하지 않았다. 확인 시점 디스크의 mechanic Y=7, Saydon Y=-92, 일반 몬스터 Y=-40과
광기 설정을 보존한다. UI 자료 parse와 해시는
`out/MechanicBarSizing20260928/ui-data-evidence.json`에 있다.

MainApp TU를 실제 Debug/Release 빌드 옵션에서 각각 독립 출력으로 컴파일해 오류 0을
확인했다. 기존 C4819/C4828 경고는 남는다. 근거는 같은 폴더의 `compile-Debug.json`,
`compile-Release.json`과 로그다. 생산 parser/배치/저장 메서드를 추출한 native CPU
163검사로 기본 폭, frame/fill 중심 관계, 반복 크기·위치 변경의 비누적, 유효 범위,
저장·재로드·독립 축 병합·같은 필드 충돌·lock·잘못된 입력의 기존 상태 보존을 확인했다.
현재 실제 `KoukuHudModes.json`을 임시 fixture로 복사해 크기 변경·저장 후 기존 modes,
madness, X/Y, 알 수 없는 필드까지 동일하게 보존되는 것도 확인했다.
`probe/test.log`, `probe/extraction-receipt.json`에 실행 증거를 남겼다.
독립 코드 리뷰와 `git diff --check`도 통과했다.

현재 실행 중인 Client는 이 수정 전 EXE이므로 이번 변경의 제품 링크·실행 화면 확인을
완료했다고 기록하지 않는다. Client/UI는 자동 실행하거나 종료하지 않았다. 첨부 화면에서
광대 얼굴이 붙은 작은 바는 `Madness_State_0`이며 로컬 플레이어를 따라가는 광기 게이지다.
이는 보스 머리 위 빨간 HP와 다르다. 사용자가 말한 이동 문제가 별도의 빨간 HP에서도
재현되는지는 미확인이고, 이번 변경에서 기존 head anchor를 다시 수정하지 않았다.

## G05. 창술사 Movie 머리

사용자 추가 설명에 따라 원인 판단을 정정한다. 사용자가 지적한 것은 두피에 납작하게 붙은
모발의 볼륨 문제다. 아래 검사는 목/몸통 영향으로 생긴 모발 변형을 확인한 것으로, 그 문제가
두피 밀착의 원인임을 입증하지 않았다.5865개 정점 위치는 변경 전후 동일하고, 기존 head100%
정점2472개는 binding까지 동일하다. 당시에는 두피와의 간격/교차나 정수리 볼륨을 측정하지
않았으며, 아래 후속 비교도 이 패치로 사용자의 두피 밀착 문제가 해결됐다는 증거는 아니다.

FT43 donor의 neck/spine 가중치가 Movie 자세를 따르면서 head 기준 최대34.08cm 변형되는
것을 확인했다. bind/rest 및 finite 성공만으로 이 변형을 검출할 수 없었다. 파생 Movie 헤어만
head bind를 사용하도록 수정했다. 정점5865개·UV·normal·triangle·weight 수치, native170 재질,
Movie208본과 Intro/Loop 원본 clip을 보존했다. 일반 장착 FT43과 원본 FT06은 변경하지 않았다.

실제 키와 중간 시각390개에서 원형 오차2.74e-13cm, 실제
`CActorCatalog -> CModel -> CWorldSequenceObject::Sample`42자세에서 Movie본 오차0,
scale 상대오차 최대6.53e-6을 확인했다. head추종형이므로 별도 긴 모발 물리 흔들림은 없다.
Runtime/GBResources FT43_Hair.wmodel의 SHA256은
`57f4f9cd16b90a579f9ee3e0c64f9702ba6f1a649987dc8f06b65aba316d95c8`로 같다.
World JSON의 배우/애니메이션 연결은 이미 이 asset을 사용하므로 변경하지 않았다.
근거: `out/LanceMovieHairFix20260927`의 shape/native/설치 receipt.

### 후속 비교: Movie·일반 플레이·커스터마이징

현재 디스크의 일반 플레이 장착값과 커스터마이징 기본 선택(index32)은 모두 FT43이며,
Movie도 FT43 파생 모델과 같은 donor native170 재질을 사용한다. 일반 body에 내장된
FT08 submesh7은 초기생성과 장비 재적용 모두 숨겨져 중복 표시되지 않는다. Movie 얼굴과
헤어의 Intro139키·Loop2키 TRS/애니메이션 track은 서로 동일하고 scaleMultiplier는1이다.
두 경로 모두 native170을 forward pass9(양면·depth read-only·alpha blend)로 그린다.
얼굴은 Movie 전용 fighter_face.slot1/face02와 일반 LanceMaster.wmodel/face00으로 다르다.
실행 중 사용자가 선택한 헤어·morph·염색은 디스크 기본값과 구분하며 관측하지 않았다.

일반 얼굴 submesh3만 분리하고 Movie의 반사 basis를 같은 head bind 좌표로 맞춰 측정했다.
FT43의 대응5865정점 최대 위치 차이는1.44e-14cm로 사실상 동일하다. 정수리 중앙의 같은
수직 ray73개에서 최상단 얼굴 삼각형과 모발 삼각형 사이 간격은 일반
0.391~3.564cm(중앙값1.245), Movie0.516~3.887cm(중앙값1.354)다. 따라서 설치된 기본
기하에서 Movie 정수리가 더 밀착돼 있다는 가설이나 단순 전체 Y offset 누락은 입증되지
않았다. 이 값은 bind 기본 기하의 바깥쪽 triangle 표면이며 texture alpha·얼굴 morph·실제
재생 자세·최종 GPU 화면의 유효 볼륨 측정이 아니다. 전체 body의 최고점을 두피로 쓰면 숨겨진
FT08 정점이 섞이므로 그런 수치는 폐기했다. 두피 밀착의 최종 원인은 여전히 미확정이며
추가 offset·scale·모발 형태 변경은 하지 않았다.

근거: `out/LanceMovieHairAppearanceInvestigation20260928/appearance-paths.json`,
`measure_bind_gap.py`, `bind-scalp-comparison.json`.

## G06. 게시·리소스·빌드

`Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon -ExpectedKoukuSaydonSourceRevision 2466`은
koukusaydon.product, map.kakulsaydon, world.gameplay, gameplay.balance 네 domain 모두 PASS다.
총316632ms. 생성된 patternbindings/Encounter/Client WorldSequence/Server Gameplay도 반영했다.
현재 published sourceRevision2466, World revision2285와 실제 Server capture rows를 확인했다.

사용자는 이전 리소스를 `Desktop/GBResources2.zip`으로 이미 공유했고 이번 세션의 변경분만
전달하도록 요청했다. 기존 의존 리소스를 재복사한 것은 전달 범위 오류였으므로 정정했다.
GBResources에는 이번에 생성한 WAV197개와 이번에 수정한 Movie FT43_Hair.wmodel1개,
총198개(749,078,046bytes)만 남겼다. 기존 Cast3/피자7개, 사각 그룹 의존35개, HUD PNG5개,
변경 없는 헤어 의존6개는 설치 receipt 및 현재 SHA256을 확인한 뒤
`out/KoukuAudioMarioHud20260927/excluded-existing-resources`로 옮겨 보존했다.
Client/Bin/Resources와 GBResources2.zip은 변경하지 않았다. 보존198개·제외53개의 경로/해시/
사유는 `out/KoukuAudioMarioHud20260927/session-resource-delivery.json`에 기록했다.

공식 Product Debug 빌드·링크·배포는 PASS이며 receipt는
`out/BuildPipeline/runs/20260927T125818461Z-debug-product.json`이다.
최초 전체 빌드에서 새 optional JSON helper의 인수 오류1개를 발견해 기존 Is_ObjectShape로
수정한 뒤 통과했다. 새 제품 C++ 파일이나 프로젝트 등록 변경은 없다.
공식 Product Release 빌드·링크·배포도 PASS이며 receipt는
`out/BuildPipeline/runs/20260927T130136916Z-release-product.json`이다.
두 구성 모두 Engine/Shared/Server/Client를 순서대로 Build했고 SkipBuild는 false다.
최종 receipt의 missingRuntimeInputs/invalidRuntimeInputs는 모두 빈 배열이다.
기존 혼합 인코딩 C4819/C4828 및 외부 DirectXTK PDB 부재 LNK4099 경고는 남아 있다.
오류0인 빌드 성공이며 경고0이라고 기록하지 않는다.

변경된 JSON8개와 생성 JSON1개, 프로젝트/filters XML2개 parse, 변경 C++13개 encoding/CRLF
보존, Python syntax 및 `git diff --check`가 통과했다. 실제 실행 EXE/DLL과 EngineSDK,
임시 native probe 산출물은 소스 변경에 포함하지 않았다. 검증 receipt와 로그는
`out/KoukuAudioMarioHud20260927`에 있다.

Client/UI는 자동 실행하지 않았다. 실제 머리 외형, 낙하탄/고정 장판 화면, 사운드의 음량과
반복 경계, F1 위치 편집의 최종 화면 판정은 사용자 확인 범위다. 디스크 게시/빌드 성공을
실행 중 편집기 메모리 draft 또는 Server의 자동 재로드로 기록하지 않는다.
