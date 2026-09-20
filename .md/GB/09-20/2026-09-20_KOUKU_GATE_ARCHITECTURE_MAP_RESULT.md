# 쿠크세이튼 1·2·3관문과 빙고 — 현재 프로젝트 구조 지도 (앵콜 컷신 삽입 준비)

작성일: 2026-09-20
요청: "우리가 쿠크 1관문 2관문 3관문을 어떻게 만들었는지 프레임워크를 보고 알려 달라." 방금 원본에서 찾은 앵콜 컷신(3관문 클리어 직후 → 빙고)을 이 프로젝트에 넣을 때 필요한 기존 구조를 파악하는 것이 목적이다.
범위: **읽기 전용 조사**다. 코드·데이터·Resources를 수정하지 않았고 빌드·publisher·Client/Server 실행·commit도 하지 않았다. 이 문서 하나만 새로 만들었다.

근거 표기: **[소스]** 소스 파일을 직접 읽음, **[데이터]** 저장소 JSON·게시본을 직접 읽거나 계산함, **[문서]** 저장소·데스크톱 문서를 읽음, **[추정]** 위 근거로 추론함, **[미확인]** 확인하지 못함.

## 한눈에 보기

관문 하나의 흐름(현재 서버 코드 기준)은 이렇다. `PREPARING`(Client 준비 확인, 10초) → `CINEMATIC`(입장 연출 Sequence) → `COMBAT`(보스 소환, 저작된 Flow 패턴을 순서대로 재생, 카드미로가 열리면 `WAIT_MINIGAME`) → 주 보스 사망 → `WAIT_GATE`(관문 클리어 표시와 MVP, 그다음 전원 승인 투표) → 다음 관문의 `CINEMATIC`.

- **1관문**: 광장 트리거 `1Stage_Final`이 팝업북 피날레 Sequence를 재생하며 레이드를 시작한다. 입장 연출은 `1관문_통합_시퀀스`(61.7초), 클리어 연출은 없다. Flow 13개.
- **2관문**: 입장 `2관문_진입컷씬`(27.0초), 클리어 `2관문_클리어`(35.4초). 주 보스는 쿠크, 대형 세이튼이 보조 보스. Flow 14개(번들 10개 포함). 카드미로·조커찾기가 여기에 붙는다.
- **3관문**: 입장 `3관문_진입`(18.7초), 클리어 연출 없음. 보스 세이튼 한 마리. Flow 9개(마리오 1~4페이즈, 분신 소환, 감전빔, 쇼타임 등).
- **빙고**: 서버에 **빙고판 판정 코드**(칸 채우기·폭탄·망치)와 `boss.kakulsaydon.bingo.saydon` 배치, 카메라만 있는 `빙고_최종엔딩씬`이 있다. 하지만 **3관문에서 빙고로 넘어가는 제품 전이는 없고, 빙고를 시작·판정하는 코드는 Debug 명령뿐이다.**
- **3관문 클리어 뒤 지금**: 서버가 3관문 클리어 비트를 세우고 Client가 **진짜** 클리어 UI(엠블럼+효과음) → MVP 화면 → 돌아가기(EXIT)·재시작 버튼을 띄우고 끝난다.

## 1. 서버 상태머신과 진입점

관문을 다루는 서버 파일은 세 곳이다.

- `Server/Private/GameRoom_KoukuRaidFlow.cpp`(447줄): 레이드 실행 상태 `m_KoukuRaid`와 단계 전이
- `Server/Private/GameRoom_GateProgress.cpp`(334줄): 관문 표, 클리어 표시, 전원 승인 투표
- `Server/Private/GameRoom_KoukuPlayerCommands.cpp`, `KoukuSaydonLogicRuntime.cpp`: 빙고판 등 Kouku 전용 로직

**단계(phase)**: `KOUKUSAYDON_RAID_PHASE { INACTIVE, PREPARING, CINEMATIC, COMBAT, WAIT_GATE, WAIT_MINIGAME, COMPLETE, ABORTED }` (`Shared/Public/Network/PacketMessages.h:3184`). 상태 전체는 `S2C_KOUKUSAYDON_RAID_STATE`(같은 파일 3198행)로 복제한다. 필드는 phase, gate id, Sequence composition id, **Sequence pattern id**, 시작·종료 tick, Flow 항목 index, 참가자 목록, ready 마스크 등이다. **[소스]**

**시작**
- Release에서는 요청 `START`가 거절되고(`GameRoom_KoukuRaidFlow.cpp:33-36`) 월드 입장 트리거가 시작한다. 1관문 입장 트리거 `1Stage_Final`(`Data/Worlds/LV_LUT_MIDNIGHTC_ED/Gameplay.world.json`, playSequence `world.sequence.instance.circusfinale`)을 밟으면 `Broadcast_WorldSequencePlay`가 이 Sequence가 GATE1의 입장 Sequence(`gate->strEntrySequenceInstanceId`)임을 알아보고 `Begin_KoukuRaidPreparation`을 호출한다(`GameRoom_PartyWorld.cpp:489-513`). **[소스]**
- `Begin_KoukuRaidPreparation`(`GameRoom_KoukuRaidFlow.cpp:71`)은 저작 문서 revision·Flow·도착 위치·네비 바닥을 모두 검증하고, 방에 1~4명이 있어야 하며 전원이 살아 있고 이동 중이 아니어야 한다. 통과하면 `PREPARING`을 방송한다. 각 Client가 문서를 준비하고 `READY`를 회신해야 하며(`Apply_KoukuRaidReadiness`, 160행) 전원이 준비되면 다음 tick에 `CINEMATIC`이 시작된다(`Update_KoukuRaid`, 373-388행). 10초 안에 안 되면 중단한다. **[소스]**

**진행(`Update_KoukuRaid`, 365행)**
- `CINEMATIC`: 게시된 도착 슬롯(RAIDARRIVAL)을 지정된 ms에 참가자에게 적용한다(391-405행). 종료 tick이 되면 클리어 연출이면 `GATE3` 입장 연출로, 아니면 `Start_KoukuRaidCombat`로 간다(406-411행). **클리어 연출 끝의 분기가 "다음은 3관문 입장"으로 고정되어 있다.**
- `COMBAT`: `Start_KoukuRaidCombat`(259행)이 주 보스와 Flow가 쓰는 모든 보스 배치를 소환하고 `Start_KoukuRaidEntry`(287행)가 Flow 항목을 하나씩 audition 방식으로 재생한다. 항목 사이에 `waitAfterMs`를 기다린다. 카드미로가 열려 있으면 `WAIT_MINIGAME`으로 바뀐다(418-445행).
- `WAIT_GATE`: 시간이 지나도 넘어가지 않는다(413-417행). **[소스]**

**관문 클리어**
- 주 보스가 죽으면 `Notify_KoukuRaidBossDeath`(312행; 호출은 `GameRoom_BossSimulation.cpp:2780`)가 `WAIT_GATE`로 바꾸고 `m_GateProgress.iCurrentGate`, `iClearedMask` 비트를 세운 뒤 `S2C_GATE_PROGRESS_STATE`를 방송한다(320-327행). 레이드가 없는 방은 `Notify_GateBossDeath`(`GameRoom_GateProgress.cpp:72`)가 같은 일을 한다. **[소스]**
- 관문 표는 코드에 3개뿐이다(`GameRoom_GateProgress.cpp:30-35`): 1관문 `boss.kakulsaydon.g1.saydon` 착지 (−2.45, 1.32, 740.37), 2관문 `g2.big-saydon`+`g2.kouku` (3.38, 10.56, 323.92), 3관문 `g3.saydon` (−2.45, 1.32, 945.17). `Gate_Count()`는 쿠크 방에서 3이다(42행). **[소스]**

**다음 관문 투표**
- `Handle_GateProgressPropose`(93행): 레이드 소유자만 제안할 수 있고 `WAIT_GATE`에서만 `ADVANCE`가 가능하다. **`iCurrent >= Gate_Count()`이면 `ADVANCE`를 거절한다**(113-115행) — 3관문 뒤로는 넘어갈 수 없다. 전원 수락하면(`Close_GateProgressVote`, 190행) `Advance_KoukuRaidGate`(330행)가 실행된다. 재시작(`RESTART`)은 같은 관문을 다시 세운다. **[소스]**
- `Advance_KoukuRaidGate`: 다음 관문 입장 연출로 가되(`Begin_KoukuRaidCinematic`), **현재 관문이 2일 때만** `clear=true`로 `GATE2`의 클리어 연출을 먼저 재생한다(350-351행). 즉 2→3 전환만 "클리어 연출 → 다음 관문 입장 연출"이고 1→2는 클리어 연출 없이 바로 입장 연출이다. 참가자 HP·자원·쿨타임을 초기화한다(353-362행). **[소스]**

**중단·종료**: `Stop_KoukuRaid(reason, completed)`(210행)가 `COMPLETE`/`ABORTED`로 바꾸고 카드미로·상호작용 상태를 정리한다. 참가자가 방을 나가거나 gameplay revision이 바뀌면 자동 중단한다(369-371행). **[소스]**

## 2. 관문별 구성

### 게시된 관문 정의(서버가 읽는 값)
`Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap`의 `RAIDGATE` 행이다. 이 파일은 Kouku 도메인 publisher가 `Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json`과 `Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json`에서 만드는 생성물로 보인다(`Tools/KoukuSaydonPipeline/Publish-KoukuRaidRows.ps1`이 있으나 이번에 실행 경로를 따라가지 않았다 [추정]). 값은 게시본에서 직접 읽었다. **[데이터]**

- **GATE1**: flow `kakulsaydon.flow.gate1`(13항목), 입장 패턴 `KAKULSAYDON_G1_PATTERN_4`(61,662ms), 클리어 패턴 `NONE`, 주 보스 `boss.kakulsaydon.g1.saydon`, 입장 트리거 Sequence `world.sequence.instance.circusfinale`. 도착 슬롯 4개가 42.25~43.52초에 (−5.3~−1.2, 1.32, 738.5~742.5)에 놓인다.
- **GATE2**: flow 14항목, 입장 `PATTERN_3`(27,000ms), 클리어 `PATTERN_5`(35,368ms), 주 보스 `boss.kakulsaydon.g2.kouku`. 도착 슬롯 4개가 26,999ms에 (3.38~5.93, 10.56, 323.92). 클리어 도착(CLEAR) 행은 없다.
- **GATE3**: flow 9항목, 입장 `PATTERN_7`(18,658ms), 클리어 `NONE`, 주 보스 `boss.kakulsaydon.g3.saydon`. 도착 슬롯 4개가 18,657ms에 (−2.45~0.10, 1.32, 945.17).

`RAIDGATE` 파서는 gate id로 **GATE1/2/3만 허용**한다(`Server/Private/GameplayCatalog.cpp:4631`). `PATTERNTARGET`(패턴이 어느 관문 보스를 대상으로 하는지)은 `BINGO`도 허용한다(4617행). 파서는 클리어 패턴과 클리어 길이가 함께 `NONE`/0이어야 한다는 검사만 하므로(4635행), GATE3에 클리어 패턴을 넣는 것 자체는 파서가 막지 않는다. **[소스]**

### 보스 배치와 관문 매핑
`Data/Worlds/LV_LUT_MIDNIGHTC_ED/Gameplay.world.json`의 비활성 boss 배치 6개: `g1.kouku`(22, −0.1, −62; 1관문 Flow의 주 보스는 아니고 `g1.saydon`이 주 보스), `g1.saydon`(−0.1, 1.3, 737.5), `g2.big-saydon`(10.2, 10.0, 317.8), `g2.kouku`(6.4, 10.6, 321.3), `g3.saydon`(−0.1, 1.3, 942.3), `bingo.saydon`(−0.6, 0, 1147.4). 관문 진입 시 서버가 해당 배치를 소환한다. **[데이터]**

### 패턴 구성(`Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json`, rev 1753)
패턴 93개: GATE1 19, GATE2 27(대형 세이튼 11 + 쿠크 16), GATE3 46, BINGO 1. 제품(PRODUCT) 상태는 9개뿐이고 나머지 84개는 DRAFT다. Flow가 참조하는 패턴 중 DRAFT가 GATE1 10/13, GATE2 16/17, GATE3 9/9이지만 서버 게시는 통과한다(게시본 RAIDFLOWSTEP 13/14/9개 확인). **[데이터]**

3관문 Flow는 마리오 1~4페이즈(각 126.1초), 분신 소환 2종, 감전빔, `쇼타임_연출`(5초), `세이튼_쇼타임`(21단계 58.4초) 순서다. 앵콜 관련 패턴은 `PATTERN_62 빙고 | 앵콜세이튼 | 블랙홀빔`(GATE3 대상, 4.5초)이 있지만 3관문 Flow에는 들어 있지 않다. **[데이터]**

### 연출용 Sequence 패턴(`Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json`, rev 65)
9개 모두 DRAFT다. `연출_팝업북`(P1, 40.7초), `연출_1관문 피날레`(P2, 21.0초), `1관문_연출`(P8), `1관문_통합_시퀀스`(P4, 61.7초), `2관문_진입컷씬`(P3, 27.0초), `2관문_클리어`(P5, 35.4초), `2관문_카드미로`(P6, 11.9초, 미리보기 전용), `3관문_진입`(P7, 18.7초), `빙고_최종엔딩씬`(P9, BINGO, 49.1초). **[데이터]**

### 특수 기믹의 위치
마리오 1~4 트리거·카드미로·조커찾기·룰렛·광대 형태는 `Gameplay.world.json`의 트리거와 Composition의 Logic/World occurrence로 붙는다. 카드미로는 2관문 번들(`카드미로_동시`)과 서버 `CKoukuCardMazeRuntime`, 마리오는 3관문 Flow와 방 소유 핸들러(`Begin_MarioTriggerMove`)에 연결된다. **[데이터·소스; 기믹 내부 동작은 이번에 따라가지 않았다]**

## 3. 관문 전환과 서버/Client 경계

**서버가 소유하는 것**: phase, 시작·종료 tick(`iStartTick`, `iEndTick`), 어느 Sequence pattern을 재생하는지(`strSequencePatternId`), 도착 슬롯 위치와 시각, 참가자 이동·HP 초기화, 보스 소환·사망, 관문 클리어 비트, 투표. **[소스]**

**Client가 하는 것**: 방송된 `S2C_KOUKUSAYDON_RAID_STATE`를 받아(`ClientReplication.cpp:419-423`) 자기 Composition 문서에서 그 pattern을 펼쳐(`Try_ExpandPatternDocument`, `MainApp.cpp:1356`) **서버 tick 기준 시계**로 카메라·이펙트·월드 시퀀스를 표본 재생한다. `PREPARING`에서 문서를 준비했는지 확인해 `READY`를 회신하고(`MainApp.cpp:1240-1290`), `CINEMATIC`이 아니면 연출을 반납한다(1294-1330행). **[소스]** 이 함수들의 이름과 문자열에 `Debug`/`Complete Play`가 들어 있지만 Release 입장도 같은 경로를 쓴다 **[추정: 서버가 Release 시작을 같은 함수로 시작하기 때문]**.

**Client 클리어 UI(현재 3관문 끝)**: `Apply_GateProgressState`(`Level_KakulSaydonArena.cpp:2437`)가 서버 클리어 비트가 새로 켜지는 순간 `Trigger_RaidClear`(2362행)를 호출한다(2467-2479행). 이것이 `RaidClear_Kouku_Layout.json`의 "던전 클리어" 엠블럼을 재생하고 효과음 `sys_raid_success1`을 92프레임에 울린다(`Update_RaidClear`, 2313-2360행). 243프레임에 끝나면 MVP 결과 화면을 띄운다. 그다음 진행 버튼이 마지막 관문이면 **EXIT**(돌아가기)로 바뀐다(2541행). **[소스]**

**열린 아레나 전투 연결**: 카드미로나 Flow 패턴이 진행되는 동안 서버 `Update_KoukuRaid`가 주 보스 존재를 매 tick 검사하고(420-421행) 보스가 사망 이벤트 없이 사라지면 중단한다. **[소스]**

## 4. 컷신 제작 방식과 사례

컷신 하나는 파일 네 곳이 stable ID로 이어져 있다(데스크톱 `쿠크_컷신_팀장방식_설명.md`, 2026-09-13 **[문서]**; 데이터로 대조 **[데이터]**).

- **시간표**: Composition의 패턴 1개 = 컷신 1개. stage 길이, world/presentation/sceneProfile occurrence(시작 ms, 길이, 자원 ID).
- **움직이는 물체**: `worldsequences.json`(`Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/`, 이 Area의 정본은 템플릿 270, 인스턴스 326, objectResource 457). 물체 하나가 objectResource → template(transform 키) → instance의 3단이다.
- **카메라**: `camerashots.json`의 shot(activation `PATTERN_ONLY`).
- **이펙트·암전**: `Data/Effects/Authored/*.effect.json`(V1)과 `Data/Effects/V2/Authored/kouku.*.fade.black.effectv2.json`(암전).
- **구운 배우 모델**: `Client/Bin/Resources/Map/KakulSaydon/SourceSequences/<prefix>/…`(Git 비추적, Drive).

Composition이 다루는 연출 자원 종류는 **CAMERA, EFFECT, LIGHT, COLLIDER, SOUND** 다섯 가지뿐이다(두 Composition의 presentationResources 집계). **자막(SUBTITLE)이나 화면 오버레이(UI) 종류는 없다.** Client 소스에서도 시네마틱 자막 UI는 찾지 못했다(`Subtitle` 검색 결과는 `RaidEntryPreviewView.cpp`, `Level_CharacterSelect.cpp`뿐이며 쿠크 연출과 무관). **[데이터·소스]**

**원본을 옮긴 사례: `3관문_진입`(P7)** — 원본 Matinee에서 자동 변환한 방식(팀장 방식)이다.
1. 도구 `Tools/KoukuSaydonPipeline/build_source_sequences.py`의 `CONFIGS`에 한 줄을 넣는다: `id=7, name='3관문_진입', prefix='kouku.gate3.intro', scene='SCENE02A', matinee=63, data=117, duration=35368, start=16710, gate='GATE3', combat=True`. 2관문 클리어와 **같은 Matinee**를 16,710ms부터 잘라 쓴 것이다. **[문서]**
2. 도구가 UPK를 저장소의 UE3 파서로 읽어(원본은 읽기만) 카메라 트랙을 표본화해 shot으로 나누고(UE3 cm Z-up → `[x·0.01, z·0.01, −y·0.01]` m, 수평 FOV → 16:9 수직 FOV), 배우 애니메이션은 30fps로 다시 샘플해 클립 하나(WANM)로 굽고(`SourceSequences/<prefix>/<Label>.wmodel`, 재질은 원본 보스 모델의 것을 `materialSourceModelAssetId`로 빌림), 원본 암전 트랙은 V2 ScreenPost 문서의 `intensityKeys`로 옮긴다. 원작 ParticleSystem은 별도 도구(`build_kouku_all_source_effects.py`, `build_source_sequence_effects.py`)가 먼저 V1 이펙트 문서로 투영해 둔다.
3. `--install`로 정본 JSON 3개(Composition·worldsequences·camerashots)에 stable ID만 병합한다(바이트 baseline을 먼저 대조).
4. `Publish-MapAuthoring.ps1 -AreaId LV_LUT_MIDNIGHTC_ED -Scope WorldSequences`로 worldsequences를 runtime에 게시하고, 서버 승인 재생(Complete Play)까지 쓰려면 `Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon`(게시본 `Gameplay.bootstrap`의 RAIDGATE 행)까지 한다. 새 `SourceSequences/<prefix>` 폴더는 Drive로 전달한다.
5. Client는 Composition·camerashots를 Data에서 직접 읽고 위 서버 tick 시계로 재생한다.

이 절차의 함정과 변환 규칙은 메모리 `kouku-popup-cutscene-pipeline`, `matinee-actor-to-worldsequence-conversion-rules`, `kouku-gate2-intro-backdrop-parents`에도 정리되어 있다: `cim_constant` 키는 다음 키 시각에 순간이동(계단), AnimControl 슬롯은 층 구조, 소품은 부모 카메라 더미의 Move 트랙으로 움직인다, 박스 수명이 모션보다 짧으면 도중에 복구된다, `enterCombatOnFinish`는 관문당 하나만 허용된다. **[문서]**

**앵콜과 같은 계열의 이미 있는 작업**: 09-10 `앵콜 컷신 / 진짜 마무리 컷신` 조사·구현 계획서(PLAN 1,195줄, G00~G26; 데스크톱 `앵콜컷신 진짜 마무리 컷신.txt`와 SHA256 일치를 그 RESULT가 기록)가 이미 앵콜의 원본 후보(SCENE07A `interpdata_23` 23,333ms), `fakeui`와 깨진 유리 재질, 화면 합성 순서, 서버 전이 초안, 단계별 구현 순서를 담고 있다. 구현은 하지 않았고 계획서다. 진단 산출물은 `out/EncoreFinalPlan/`(SCENE07A.raw.json 등)에 지금도 있다. **[문서·데이터]**

## 5. 빙고의 현재 상태와 3관문 → 빙고 전이

**있는 것**
- 공유 상수: 5×5 판, 칸 3.04m, 원점 (−6.08, 1140.8), 칸 인덱스·12줄 마스크·십자 마스크(`Shared/Public/Network/PacketMessages.h:1006-1069`). 원본 빙고판 착지 (−3.04, 0, 1149.96)은 이 판의 (열 1, 행 3) 칸에 해당한다(계산). **[소스·계산]**
- 서버 `CKoukuBingoRuntime`(`KoukuSaydonLogicRuntime.cpp:1838-1961`): `Fill`(칸 채우기+줄 완성 승격), `Detonate`(폭탄 십자), 폭탄 슬롯(MARKED→PLANTED), 망치 스윕. `Update_KoukuBingo`(`GameRoom_KoukuPlayerCommands.cpp:202`)가 **모든 tick**(`GameRoom.cpp:1016`) 폭탄 fuse와 망치 진행을 처리하고 상태는 스냅샷으로 복제된다(`GameRoom_Replication.cpp:727-735`). 망치·폭탄은 `world.sequence.instance.kouku.bingo.hammer.anchor.N`, `...bomb.planted.slot.N` 시퀀스로 표현된다. **[소스]**
- 비활성 `bingo.saydon` 배치와 `BOSS_KAKULSAYDON_BINGO_SAYDON`(BossCatalog·BossProfiles), Composition의 BINGO 패턴 `PATTERN_75 빙고_최종엔딩씬`(49,083ms 1단계), Sequence의 같은 이름 P9(카메라 12박스만; 배우·암전·조명·이펙트 없음, 09-15 결과 G08). Client F1 Debug 관문 목록에 4번째 "빙고 - 앵콜을 외친 쿠크세이튼"(위치 (−3.4, 0, 1147.44))이 있다(`Level_KakulSaydonArena.cpp:3069`, `MainApp.cpp:10366`). **[소스·데이터]**

**없는 것(소스 검색으로 확정)**
- `m_KoukuBingo`를 바꾸는 호출은 **`_DEBUG` 전용 핸들러 3개**(`Handle_DebugBingoFill/Bomb/Hammer`, `GameRoom_KoukuPlayerCommands.cpp:104-200`, 모두 `#ifdef _DEBUG`)와 방 리셋(711행)뿐이다. 제품 코드가 빙고를 시작하거나 칸을 채우거나 승리를 판정하는 곳은 없다. `Is_Safe`(줄 위 안전 판정)는 "wipe attack이 소비자가 될 때까지 계약 테스트만 묻는다"고 주석에 적혀 있다(`KoukuSaydonLogicRuntime.h`). **[소스]**
- 3관문 클리어 → 빙고 전이: **없다.** 3관문 주 보스 사망은 위 1절의 `WAIT_GATE`+클리어 비트로 끝나고, 관문이 3개뿐이라 `ADVANCE`도 거절된다. Client는 진짜 클리어 UI → MVP → EXIT/RESTART를 그린다. **[소스]**
- 원본 빙고 국면의 구성원(투명 몬스터 25명 = 빙고판 칸, 빙고담당 12명, 관제탑, 앵콜을 외친 쿠크세이튼)에 해당하는 제품 로직: 없다. 서버에는 칸·폭탄·망치의 수치 상태만 있다. **[소스; 원본 구성은 09-20 원본 조사 문서]**
- 앵콜 컷신 재생, 가짜 클리어 UI 전용 경로, 깨진 유리 후처리, 시네마틱 자막: 없다. **[소스·데이터]**

## 6. 앵콜 컷신을 넣을 자리 (제안만, 구현하지 않음)

원본 흐름은 `3관문 보스 Event_07 → 전원 이동((−6.10,1.31,948.11)) → 컷신 320 → 앵콜 컷신 331(23.3초) → 전원 빙고판 이동((−3.04,0,1149.96)) → 빙고 39명 스폰`이다(원본 조사 문서). 프로젝트의 대응은 이렇다.

**(a) 서버 상태·전이**
1. **시작 지점**: `Notify_KoukuRaidBossDeath`(`GameRoom_KoukuRaidFlow.cpp:312`)에서 종료된 관문이 3관문일 때 `WAIT_GATE` 대신 앵콜 연출로 들어간다. 원본의 `Event_07`(보스 AI 내부 조건, 미확인)은 "3관문 주 보스 사망"으로 대체하는 것이 자연스럽다. **[추정]**
2. **클리어 비트 지연**: 같은 함수(322-327행)와 `Notify_GateBossDeath`(88-90행)가 **관문 클리어 비트를 세우는 것이 Client의 진짜 클리어 UI를 켜는 트리거**다. 3관문에서는 이 비트를 앵콜·빙고 이후로 미뤄야 앵콜 전에 진짜 클리어가 뜨지 않는다. 원본도 진짜 클리어·보상은 빙고 이후 유닛 3152에서만 나온다. **[소스·추정]**
3. **기존 도구 재사용**: 게시본 `RAIDGATE`의 GATE3 행에 클리어 패턴(앵콜)과 길이(23,333ms), `RAIDARRIVAL GATE3 CLEAR` 행에 플레이어별 도착 슬롯·시각을 넣으면 `Begin_KoukuRaidCinematic(gate, clear=true)`와 도착 처리(391-405행)가 컷신 중 이동을 이미 수행한다. **필요한 코드 변경**: 클리어 연출 끝 분기(408-409행)를 "2관문이면 3관문 입장"이 아니라 관문별 다음 목적지로 일반화해야 한다. 원본처럼 컷신 전에 한 번 더 이동시키려면 도착 슬롯을 두 번 쓰는 것을 검토한다. **[소스·추정]**
4. **빙고를 무엇으로 두느냐**: (A) 4번째 관문으로 두면 `RAIDGATE` 파서(GATE1~3만 허용, `GameplayCatalog.cpp:4631`), `KOUKU_GATES` 표와 `Gate_Count()`(3), Client 진행 UI가 3관문을 가정하는 부분(`RaidGateProgress_Layout.json`의 아이콘 슬롯 3개와 `RaidGateProgressView.cpp:117`의 상한 3), 프로토콜 호환을 함께 바꿔야 한다. (B) 3관문의 후속 단계로 두면 gate index는 3 그대로이고 앵콜 종료 시 주 보스를 `boss.kakulsaydon.bingo.saydon`으로 교체하는 새 전이 코드가 필요하다. 어느 쪽이 나은지는 결정 사항이다. **[추정]**
5. **phase 열거형**: 기존 `CINEMATIC`을 재사용하면 `S2C_KOUKUSAYDON_RAID_STATE`의 `strSequencePatternId`만 앵콜 패턴으로 바뀌므로 protocol 변경이 없다(추정). 새 phase 값(예: `ENCORE`, `BINGO`)을 추가하면 Shared 변경이라 protocol 번호를 올려야 하고 Server·Client를 같은 빌드로 맞춰야 한다. **[추정]**
6. **입력 잠금**: `CINEMATIC` 중 이동·스킬 잠금이 서버에서 어떻게 강제되는지 이번에 따라가지 않았다. 기존 관문 입장 연출과 같은 경로를 쓰는지 확인이 필요하다. **[미확인]**
7. **죽은 보스 연출 수명**: 09-10 계획서 G14는 "사망한 보스를 대상으로 한 재생은 DEAD 가드에 걸린다"고 지적한다. 앵콜은 주 보스 사망 직후 재생하므로 연출 배우를 전투 entity와 별개 수명의 proxy로 두는 기존 방식(P5·P7처럼 world sequence 배우)을 써야 한다. **[문서]**
8. **빙고 국면 자체**: 앵콜이 끝난 뒤 갈 곳이 있어야 한다. 빙고 시작·칸 채우기·승패·진짜 클리어(원본 유닛 3152)·보상은 제품 코드가 없어 **별도 수직 슬라이스**다. 앵콜을 먼저 넣으면 전원이 빙고판에 도착하는 데까지만 동작한다. **[소스]**

**(b) 저작 데이터**
- Sequence Composition에 앵콜 패턴 1개를 팀장 방식 A로 추가: `CONFIGS`에 `scene='SCENE07A'`, `matinee`/`data`=원본 조사가 찾은 `efseqact_matinee_23`/`interpdata_23`의 export 번호(원본 조사 문서 2.5절에 이름은 있으나 이 도구가 쓰는 번호 형식으로는 이번에 확인하지 않았다 [미확인]), `duration=23333`, `start=0`, `gate='GATE3'`, `combat=False`.
- 원본이 SCENE07A 하나의 카메라 `cam`(shotnumber 10)에서 부모 체인으로 화면을 움직이므로 shot은 카메라 하나를 표본화하는 형태가 된다.
- 배우 `쿠크세이튼_03`(mesh `mn_rpct_05_sk`)은 AnimControl 슬롯(a·b·fc1)과 float weight 곡선을 30fps 클립으로 구워야 한다. 09-10 계획서 부록 C에 슬롯별 키가 있다.
- 암전은 기존 V2 fade 문서 방식(`kouku.gate3.intro.fade.black.effectv2.json` 형태)을 그대로 쓴다. 이펙트 5종(`par_e_shot_01` 등)은 원본 particle을 못 읽으므로 이름만으로 V1/V2 자산을 새로 저작해야 한다. **[문서]**
- **자막 3줄과 가짜 클리어 UI·깨진 유리는 Composition 연출 자원 종류에 없다.** 새 자원 종류(예: 자막, 화면 오버레이)를 Composition 스키마·projector·Client 재생기에 추가하거나 별도 view가 필요하다. **[데이터]**

**(c) Client 표현**
- 가짜 클리어 UI: `RaidClear_Kouku_Layout.json`과 `Update_RaidClear`의 엠블럼 재생 부분을 참고할 수 있지만, 현재 `Trigger_RaidClear`는 관문 클리어 비트, MVP, EXIT 버튼과 묶여 있어 그대로 호출하면 진짜 클리어가 된다. 앵콜 전용으로 엠블럼·빛 표시만 하는 view가 필요하다. 원본 엠블럼은 SWF `vs.epicgatecommanderresulttest`이고 이것이 Kouku 클리어 아트와 같은지는 확인하지 못했다. **[소스·미확인]**
- 깨진 유리: 화면 후처리 재질이다. 월드 이펙트만으로는 UI가 멀쩡히 남는다는 것이 09-10 계획서의 결론이다. 렌더러의 offscreen 합성 지원 확인이 필요하다. **[문서·미확인]**
- 자막: 시네마틱 자막 UI가 없다(위). `cin.37081_12_01~03` 세 줄을 10.2초·12.4초·16.7초에 띄우는 UI를 새로 만들어야 한다.
- 화면 페이드: 기존 V2 ScreenPost fade 자원과 `Update_TriggerMoveFade`를 참고할 수 있다.

**(d) 원본에서 새로 추출·변환할 것**: 원본 조사 문서(`.md/GB/09-20/2026-09-20_KOUKU_ENCORE_CUTSCENE_ORIGINAL_DATA_RESULT.md`)의 2.5절 목록이다 — Matinee 23의 배우·카메라·조명, 깨진 유리 재질과 `opacity`/`type` 키, 파티클 5종 이름, AkEvent 4개(`..._fakeclear`, `..._fakeclear_skip`, `..._koukustopclearingdungeon`, `..._popup1`), SWF 엠블럼.

**재사용 가능**: Matinee→WorldSequence 변환 규칙, 팀장 방식 파이프라인, `RAIDARRIVAL` 도착 이동, `Begin_KoukuRaidCinematic`의 클리어 연출 경로, 서버 tick 시계 재생, 카메라 shot 파이프라인, 보스 모델 `MN_RPCT_05`, 기존 암전 V2 fade, 09-10 계획서 전체(화면 합성 순서·서버 전이 초안·부록). 단 09-10 계획서의 서버 전이 이름(`ENCORE_PREPARING`/`ENCORE_PLAYING`/`BINGO_HANDOFF`)은 그 뒤 09-19에 들어온 현재 레이드 상태머신(PREPARING/READY, WAIT_GATE, 투표)보다 먼저 쓰인 것이므로 위 (a)의 현재 구조에 맞게 다시 매핑해야 한다. **[문서·소스]**

**위험·의존성·작업 순서 후보**
1. **위험 1 — 갈 곳이 없다**: 빙고 제품 로직이 없어서 앵콜을 먼저 넣으면 빙고판 도착에서 멈춘다. 앵콜 뒤에 임시로 `bingo.saydon`만 소환할지, 빙고 최소 슬라이스를 먼저 만들지 결정이 필요하다.
2. **위험 2 — 진짜 클리어 오발동**: 3관문 클리어 비트를 세우는 두 곳(`Notify_KoukuRaidBossDeath`, `Notify_GateBossDeath`)을 함께 지연하지 않으면 Client가 진짜 클리어 UI와 EXIT 버튼을 앵콜 전에 띄운다.
3. **위험 3 — 프로토콜**: phase 열거형이나 새 메시지를 추가하면 protocol이 올라간다.
4. **위험 4 — 4인 동기**: `PREPARING`의 문서 준비 확인이 앵콜 패턴 문서까지 포함하는지(3관문 시작 시점에 미리 준비하는지) 확인이 필요하다. **[미확인]**
5. **순서 후보**: ① 서버 전이만 먼저(3관문 사망 → 앵콜 `CINEMATIC`(카메라만 있는 패턴) → 전원 빙고판 이동 → `bingo.saydon` 소환)를 4인으로 검증 ② 원본 변환(카메라·배우·암전) ③ 가짜 클리어 UI·자막·유리 합성 ④ 파티클·사운드 ⑤ 빙고 제품 로직과 진짜 클리어.

## 7. 확인하지 못한 것

- Client가 `CINEMATIC` 중 입력을 막는 정확한 경로(서버·Client 양쪽). 실제로 4인으로 실행해 보지 않았다.
- (확인됨, 위 조사 이후 추가) `RaidGateProgress` UI는 3관문을 고정 가정한다. `Data/UI/RaidGateProgress/RaidGateProgress_Layout.json`에 관문 아이콘 슬롯이 3개이고 `CRaidGateProgressView::Set_Raid`가 관문 수를 `std::min(iGateCount, 3u)`로 제한한다(`Client/Private/RaidGateProgressView.cpp:117`). 빙고를 4번째 관문으로 두려면 이 레이아웃과 상한도 바꿔야 한다. **[소스·데이터]**
- 앵콜 원본 Matinee를 `build_source_sequences.py`가 쓰는 export 번호로 지정하는 값. 원본 조사 문서에는 이름(`efseqact_matinee_23`)만 있다.
- 렌더러가 UI 이전에 scene color를 쓰는 후처리(깨진 유리)를 지원하는지.
- 카드미로·마리오 기믹의 서버 내부 동작 세부, Flow 패턴 재생(audition) 내부.
- 2026-09-04 컷신 전체 추출본(`바탕 화면\쿠크_컷신_전체추출_20260904`)은 메모리에 기록이 있으나 이번에 데스크톱에서 폴더를 찾지 못했다(SCENE07A가 포함되었다는 기록은 메모리 문장만). `out/EncoreFinalPlan/`의 SCENE07A.raw.json은 존재한다.
- `Data/KoukuSaydon/Gate1/`에 `KoukuSaydonComposition.json.tmp.34788.16412.75234687`(1.2MB, 09-19 00:50) 임시 파일이 남아 있다. git 상태에는 나타나지 않는다. 건드리지 않았고 원인은 확인하지 않았다.
- 테스트(`ServerGameplayContractTests_KoukuRaid.cpp`, `..._Bingo.cpp`)는 읽거나 실행하지 않았다. 화면 확인도 하지 않았다.

## 근거 파일 목록

- 서버: `Server/Private/GameRoom_KoukuRaidFlow.cpp`, `GameRoom_GateProgress.cpp`, `GameRoom_KoukuPlayerCommands.cpp`(빙고), `GameRoom_PartyWorld.cpp`(입장), `GameRoom_BossSimulation.cpp`(사망 호출), `KoukuSaydonLogicRuntime.cpp`(빙고), `GameplayCatalog.cpp`(RAIDGATE 파서)
- 공유: `Shared/Public/Network/PacketMessages.h`(phase, 상태, 빙고 상수)
- Client: `Client/Private/Level_KakulSaydonArena.cpp`(클리어 UI, 관문 진행, Debug 관문 표), `MainApp.cpp`(CINEMATIC 재생, F1 관문), `ClientReplication.cpp`
- 데이터: `Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json`, `Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json`, `Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/*.worldsequences.json`, `Data/Worlds/LV_LUT_MIDNIGHTC_ED/Gameplay.world.json`, 게시본 `Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap`
- 문서: `.md/GB/09-20/2026-09-20_KOUKU_ENCORE_CUTSCENE_ORIGINAL_DATA_RESULT.md`, `.md/GB/09-10/2026-09-10_KOUKU_ENCORE_FINAL_CUTSCENES_PLAN.md`·`_RESULT.md`, `.md/GB/09-15/2026-09-15_KOUKU_FOUR_CUTSCENES_CAMERA_MAP_RESULT.md`, 데스크톱 `쿠크_컷신_팀장방식_설명.md`, 메모리 `kouku-popup-cutscene-pipeline` 등
