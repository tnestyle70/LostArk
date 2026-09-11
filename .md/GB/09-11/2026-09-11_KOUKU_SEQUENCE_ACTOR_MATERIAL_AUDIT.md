# 쿠크 시퀀서의 세이튼 모델·재질·애니메이션·종료 인계 조사

작성일: 2026-09-11. 현재 작업 트리 `codex/kouku-full-material-lighting-restoration`, HEAD `e26cd2b2`의 파일과 설치된 Resources를 읽었다. 이 문서는 조사 결과이며 제품 구현 계획이나 구현 완료 보고가 아니다. 기존 dirty C++·JSON·Resources를 수정하지 않았다.

## G00. 현재 확인된 결론

사용자가 말한 “시퀀서에 있는 두 개”와 정확히 일치하는 저장 문서는 `Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json`이다. 두 항목은 **연출_팝업북**, **연출_1관문 피날레**다. 팝업북에서 보이는 세이튼은 전투 보스와 다른 **MN_RPCT_00 Deploy prop**이고, 현재 1관문 전투 보스는 **MN_RPCT_05 CNpc**다.

다만 **다른 모델이라는 사실과 재질 복원이 빠졌다는 판단은 구분해야 한다.** 09-11 native material 작업이 이미 현재 소스에 들어 있어, 컷신 MN_RPCT_00의 3개 재질과 전투 MN_RPCT_05의 5개 재질 모두 BossCatalog의 원본 material override를 소비한다. 이전 계획서의 “Deploy가 override를 소비하지 않는다”는 설명은 구현 전 상태다. 현재 컷신만 계속 미복원 재질이라고 단정할 근거는 없다. 현재 실행 중인 EXE/DLL/CSO의 동일성이나 사용자가 본 실제 화면은 이번 조사에서 확인하지 않았다.

원하는 “복원된 세이튼으로 시퀀스를 재생하고, 종료 직후 같은 전투 대상으로 이어가기”에는 **모델/clip 연결**, **재생 상태 복구**, **Server 전투 entity 인계**가 함께 필요하다. 지금은 컷신 actor를 숨기는 데서 끝나며 전투 actor로 넘기는 계약이 없다. 아래 조사는 후속 구현 범위를 구체화하기 위한 근거다.

## G01. 실제 두 Sequence와 현재 구역의 대응

| 독립 Sequencer 저장 항목 | stable pattern ID | 길이 | 현재 Animation lane | 실제 내용 |
|---|---|---:|---:|---|
| 연출_팝업북 | `KAKULSAYDON_G1_PATTERN_1` | 37,800ms | 0개 | WORLD 7개: 맵 5묶음, 책, 세이튼 Deploy. CAMERA `2Stage.book` |
| 연출_1관문 피날레 | `KAKULSAYDON_G1_PATTERN_2` | 21,010ms | 0개 | WORLD `world.sequence.instance.circusfinale`, CAMERA `1Stage.finale` |

두 항목의 `actorProfileId`는 `MN_RPCZ_00`, `targetBossPlacementId`는 `boss.kakulsaydon.g1.kouku`다. 이것은 현재 목록의 작성 metadata이며, 팝업북 화면의 몸체를 결정하지 않는다. `animationOccurrences=[]`인 패턴은 `MainApp.cpp:1621`에서 `hasAnimation=false`로 판정하고, presentation clock과 WORLD 경로로 재생한다. 실제 몸체의 identity는 WORLD binding이 결정한다.

근거 위치:

- `Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json:1525`: 두 항목과 빈 animation lane.
- 같은 파일 `:674`, `:691`: `팝업북_책`, `팝업북_쿠크` WORLD 참조.
- 같은 파일 `:1490`, `:1508`: 두 CAMERA 참조.
- `Client/Private/MainApp.cpp:1618`: animation 유무에 따라 preview backend를 선택하고 `Begin_KoukuWorldPreview`에 연결.
- `Client/Private/KoukuSaydonActionWorkbench.cpp:1340`: 독립 Sequence workspace의 Server Play 요청을 거부.
- `Client/Private/MainApp_SequenceViewer.cpp:104`: 별도 Sequence Viewer는 Gameplay trigger 목록 등을 읽으므로 “독립 Sequencer 두 개”와 전체 WorldSequence 수를 섞지 않아야 한다.

현재 WorldSequence 문서는 revision **427**, template **154개**, instance **190개**다. 154개가 모두 컷신인 것은 아니다. 맵 이동, 책, 카드, 공, 마리오, 소품 모션을 포함한다. Authoring과 `Client/Bin/DataFiles/Map/LV_LUT_MIDNIGHTC_ED.worldsequences.json`의 JSON semantic은 같다. 바이트는 서식 차이로 다르다.

현재 구역 이름도 구분해야 한다.

| 경로 | 현재 위치/연결 |
|---|---|
| 기존 팝업북 trigger | `Gameplay.world.json:20`의 `2-1Stage_Move` → `world.sequence.instance.original_kouku` |
| 컷신 세이튼 Deploy5 | `(-0.2883, 1.3253, 737.629)` |
| 현재 G1 전투 세이튼 | `boss.kakulsaydon.g1.saydon`, `(-0.0700000003, 1.32000005, 942.330017)`, 기본 disabled |
| F1의 1관문 진입 | `Level_KakulSaydonArena.cpp:1903`에서 G1 세이튼 placement와 별도 player destination을 선택 |

두 세이튼의 Z는 약 **204.701m** 다르다. 현재 팝업북/`2Stage.book`을 G1 Debug 전투 진입과 자동으로 같은 사건으로 취급할 수 없다. 어느 장면을 G1 입장 컷신으로 쓸지는 후속 연결에서 명시적으로 정해야 하며, 이번 조사에서는 구역과 저장 이름을 바꾸지 않았다.

## G02. 화면에 그려지는 모델과 재질의 실제 호출 경로

### 팝업북 세이튼

```text
KoukuSaydonSequenceComposition.json / 팝업북_쿠크
→ world.sequence.instance.original_kouku
→ sequence.LV_LUT_MIDNIGHTC_ED.kouku_reveal
→ animated.prop / DEPLOY_PLACEMENT / targetId 5
→ DEPLOY_BOSS_MN_RPCT_00
→ Character/KoukuSaton/MN_RPCT_00/MN_RPCT_00.wmodel
→ CDeployPropRuntime::Ensure_AreaPrototypes
→ CActorCatalog::Build_ModelLoadDescription
→ CModel → CMaterial
→ CDeployPropObject::Render_Animated
→ Bind_DeferredMaterialInputs → source character Base/Light shader
```

`worldsequences.json:358491`이 실제 Deploy5 binding이고, `:209554`가 몸체의 clip/transform timeline이다. `Data/Maps/Imported/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.deployassets:4`가 모델 ID, `Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.deployplacements:6`이 배치다. 런타임 deployassets/placements에도 같은 actor ID와 model ID가 존재한다. `DeployPropCatalog.cpp:139`는 runtime Map 경로의 두 문서를 읽는다.

### 1관문 전투 세이튼

```text
Server world entity / boss.kakulsaydon.g1.saydon
→ Shared spawn/snapshot
→ Client replication / boss presentation
→ BOSS_KAKULSAYDON_G1_SAYDON
→ Character/KoukuSaton/MN_RPCT_05/MN_RPCT_05.wmodel
→ CKoukuSaydonPresentationAssetService::Ensure_Prototypes
→ CActorCatalog::Build_ModelLoadDescription
→ CModel → CMaterial → CNpc/Character body rendering
```

`Data/Actors/BossCatalog.json:173`이 G1 전투 boss definition이다. `KoukuSaydonPresentationAssetService.cpp:512`는 body descriptor, `:536`은 분리 weapon descriptor를 얻는다. `ActorCatalog.cpp:867`은 모델의 Resources-relative ID를 검증한 뒤, `:919`의 공유 보스 model override를 읽어 stage하고 마지막에 출력 descriptor를 commit한다.

### 현재 native material 입력

| 모델 | materialName | 원본 MIC | 현재 family |
|---|---|---|---|
| MN_RPCT_00 | `mn_rpct_00_parts_mi` | `mn_rpct_00.mat.mn_rpct_00_parts_mi` | `source.character.monster-6ff78ae19259.v1` |
| MN_RPCT_00 | `mn_rpct_00_mi` | `mn_rpct_00.mat.mn_rpct_00_mi` | `source.character.monster-d1e4d915a43a.v1` |
| MN_RPCT_00 | `mn_rpct_01_mi` | `mn_rpct_01.mat.mn_rpct_01_mi` | `source.character.monster-3b27d7fb1b53.v1` |
| MN_RPCT_05 | `mn_rpct_05_mi` | `mn_rpct_05.mat.mn_rpct_05_mi` | `source.character.monster-6ff78ae19259.v1` |
| MN_RPCT_05 | `mn_rpct_05-1_mi_loc_int` | `mn_rpct_05.mat.mn_rpct_05-1_mi_loc_int` | `source.character.monster-pbr-masked.v1` |
| MN_RPCT_05 | `mn_rpct_05-2_mi` | `mn_rpct_05.mat.mn_rpct_05-2_mi` | `source.character.monster-pbr-masked.v1` |
| MN_RPCT_05 | `mn_rpct_05-3_mi` | `mn_rpct_05.mat.mn_rpct_05-3_mi` | `source.character.monster-6ff78ae19259.v1` |
| MN_RPCT_05 | `mn_rpcz_00_mi` | `mn_rpcz_00.mat.mn_rpcz_00_mi` | `source.character.monster-6ff78ae19259.v1` |

BossCatalog의 해당 시작점은 `:2922`(00), `:3814`(05)다. 두 모델의 8 override가 참조하는 고유 texture ID **31개가 모두 Resources에 존재**했다. 05의 마지막 slot은 쿠크 mesh/material이 포함되는 차이이므로, 00→05는 단순 색상 교정이 아니라 몸체 구성 변경이다.

09-11 [보스·컷신·소품 native material 결과](2026-09-11_KOUKU_BOSS_PROP_NATIVE_MATERIAL_IMPLEMENTATION_RESULT.md)는 당시 33모델/50slot의 실제 CModel create→clone→native bind와 Product 결과를 기록했다. 이 세션은 그 검사를 재실행하지 않았다. 현재 BossCatalog 전체는 다른 복원 작업도 포함해 **38모델/61slot**이므로 전체 수를 당시 쿠크 범위의 33/50과 혼동하지 않는다.

## G03. 팝업북의 현재 12개 애니메이션 연결

현재 세이튼은 애니메이션이 없는 것이 아니라 **Action Workbench Animation lane 바깥의 WorldSequence.animationTracks**에서 다음 순서로 재생된다. 이 표의 clip 길이는 설치된 MN_RPCT_05 WModel의 WANM duration/ticksPerSecond를 직접 읽은 값이고, 모든 항목은 MN_RPCT_00에도 존재한다.

| 시퀀스 구간(ms) | 실제 clipName | 원 clip 길이(ms) | playbackRate | loop / holdLastFrame |
|---|---|---:|---:|---|
| 0–5,055 | `rpct00_walk_normal_1` | 1,600 | 1.0 | true / false |
| 5,055–9,330 | `rpct00_idle_normal_1` | 3,000 | 0.7 | true / false |
| 9,330–11,777 | `rpct00_idle_phase1_start_1` | 5,266.667 | 0.8 | false / true |
| 11,777–14,938 | `rpct00_att_battle_25_01` | 3,000 | 1.0 | false / true |
| 14,938–17,559 | `rpct00_att_battle_13_02` | 2,666.667 | 1.0 | false / true |
| 17,559–21,412 | `rpct00_att_battle_13_04` | 6,166.667 | 0.7 | false / true |
| 21,412–22,175 | `rpct00_evt2_blk6` | 1,000 | 0.7 | false / true |
| 22,175–23,828 | `rpct00_evt2_rpct_talk_01` | 3,100 | 0.7 | false / true |
| 23,828–26,694 | `rpct00_walk_normal_1` | 1,600 | 0.7 | true / false |
| 26,694–27,978 | `rpct00_idle_phase1_start_1` | 5,266.667 | 0.7 | false / true |
| 27,978–33,775 | `rpct00_att_battle_13_01` | 4,666.666 | 0.63 | false / true |
| 33,775–37,800 | `rpct00_att_battle_13_02` | 2,666.667 | 0.7 | false / true |

같은 clip의 반복 사용을 제거하면 **9종**이다. 책은 별도 Deploy7에서 `bg_rad_koukusaton_book.ao_evt2_book02`를 사용하며 template 2,370ms, instance playbackSpeed 약0.7, loop=false, holdLastFrame=true다.

`WorldSequencePlayer.cpp:940` 이후는 현재 구간에 해당하는 track을 찾고 `(localMs-startMs) × playbackRate / clipDurationMs`로 정규화 시간을 구한다. loop는 나머지 연산, hold는 마지막 프레임으로 처리한다. `DeployPropObject.cpp:454`는 exact clip 이름을 resolve하고 `Set_Animation → Skip_Blend → pause → Set_AnimTrackPosition → Play_Animation(0)` 순서로 샘플한다.

따라서 단순히 clip을 처음부터 끝까지 순차 연결하면 같은 컷신이 되지 않는다. 표의 **trim 구간, loop, 속도와 원본 camera clock을 함께 유지**해야 한다. 현재 Deploy sampler는 매 샘플 `Skip_Blend`를 호출하므로 원본 Matinee의 별도 blend weight가 자동으로 재현되는 구조도 아니다. 이 차이의 화면 영향은 이번 세션에서 판정하지 않았다.

## G04. 00의 연출을 05에 연결할 수 있는가

설치된 두 WModel을 `ModelAssetConverter.exe info`로 열었으며 둘 다 animation 249개와 skeleton이 존재한다. 추가로 `WFormatTypes.h`의 WMOD/WSKL/WANM layout과 실제 decoder의 section offset 기준을 따라 read-only Python으로 내용을 대조했다.

| 비교 항목 | 현재 실측 |
|---|---|
| skeleton node 수 | 양쪽 168 |
| clip 이름 집합 | 249/249 일치 |
| clip별 channel count, durationTicks, ticksPerSecond | 249/249 일치 |
| clip별 channel metadata | 249/249 일치 |
| node 이름/parent | 167개 일치, 마지막 node167은 `mn_rpct_00_sk.mo` / `mn_rpct_05_sk.mo` |
| 같은 이름 node의 rest matrix 최대 float 차이 | 약1.699e-6 |
| 비교한 animation key 수 | 10,486,902 |
| position/rotation/scale key 최대 float 차이 | 약5.782e-6 / 6.795e-6 / 1.193e-6 |
| 팝업북이 쓰는 clip 존재 | 양쪽 모두 12 track / 9 unique clip 존재 |
| WModel embedded animation eventCount | 양쪽의 249 clip 모두 0 |

**05가 자기 내부 clip을 재생하도록 바꾸는 데 필요한 clip은 이미 모두 있다.** 새 애니메이션 추출이나 bone 이름 추측을 먼저 할 이유는 없다. 그러나 09-03 [모델·애니메이션 인벤토리](../09-03/2026-09-03_KAKUL_SAYDON_MODEL_AND_ANIMATION_INVENTORY_ANALYSIS.md)의 “165 bone, skeleton·clip 완전히 동일”이라는 표현은 현재 물리 파일의 exact count/byte 비교와 다르다. 이번 수치는 전체 동일 바이트를 보증하지 않는다. 모델별 자기 skeleton과 자기 clip을 사용하고 실측한 호환 범위를 근거로 삼는다.

embedded eventCount=0은 특히 중요하다. **현재 WModel clip 재생만으로 원본 이펙트가 자동 발사되지 않는다.** 이 수치는 원본 게임에 notify가 없다는 뜻이 아니다. 프로젝트의 cooked WModel에 그 이벤트가 담겨 있지 않다는 뜻이다. 이번 두 독립 Sequence에는 Effect presentation occurrence도 없으며, 세이튼/책 template에도 effectTracks가 없다. “올바른 clip을 고르면 빠졌던 이펙트가 모두 돌아온다”는 가설을 이 구조가 뒷받침하지는 않는다.

## G05. 종료·재시작·전투 인계의 현재 동작

| 재생 경로 | 현재 종료 처리 | 전투로의 연결 |
|---|---|---|
| 독립 Sequencer Preview | 원래 Deploy 상태와 맵 visibility를 보관. Stop/owner 변경/실패 시 `Debug_StopCompositionWorldPreview`로 복구 | 없음. 독립 Sequence workspace의 Server Play는 비활성 |
| Server trigger로 팝업북 실행 | `Start_PopupBookCutscene`이 book/boss를 보이고 `original_` prefix 7개 instance를 시작 | 원본 컷신 시작/party 배치만 있음 |
| 팝업북 제품 재생 종료 | `Update_CutsceneBossRetire`가 unfolding set을 감추고 standing arena를 복구, Deploy5를 `DESPAWNED` | MN_RPCT_05 생성·활성·pose handoff 없음 |
| F1 G1 활성화 | typed world entity despawn/spawn와 player teleport 결과를 기다림 | 별도 Debug 명령 경로이며 팝업북 종료 콜백이 아님 |

정확한 구현 위치:

- `Level_KakulSaydonArena.cpp:602`: 기존 WORLD preview 정리 후 Deploy state와 맵 visibility baseline 수집.
- `Level_KakulSaydonArena.cpp:694`: `Stop_All(targets,true)` 후 Deploy 상태/standing arena visibility 복구. 복구 실패 상태를 남기고 다음 시작을 막는다.
- `WorldSequencePlayer.cpp:712`: 빌린 Deploy preview를 해제.
- `DeployPropObject.cpp:495`: 시작 전 animation index·track position·loop·pause와 배치 pose를 복구.
- `Level_KakulSaydonArena.cpp:1292`: 제품 팝업북 시작.
- `Level_KakulSaydonArena.cpp:1384`: 제품 컷신 boss retire.
- `Level_KakulSaydonArena.cpp:1413`: Server가 `original_kouku`를 요청하면 전체 팝업북에 연결.
- `Server/Private/GameRoom.cpp:5403`: 팝업북 시작 시 파티를 Z737 부근으로 옮김. 전투 세이튼 activation이 아님.
- `Level_KakulSaydonArena.cpp:1942`: 별도 Debug gate 활성화.

현재 제품 `Start_PopupBookCutscene`은 모든 7개 instance를 완전히 stage한 뒤 단일 commit하는 구조는 아니다. 하나라도 시작하면 부분 실패를 `Cutscene started without ...`로 보고한다. 따라서 후속 “연출 끝 즉시 전투” 연결에서는 **시작의 부분 성공, 중지, 실패, 재진입**까지 같은 상태 전이로 정리해야 한다. 이를 현재 이미 갖춘 seamless handoff로 기록하면 안 된다.

## G06. 요청한 방향으로 정리할 때의 권장 연결

다음은 조사에서 도출한 구현 방향이며 이번 세션의 구현 결과가 아니다.

1. **표시 모델과 소유권을 먼저 하나로 정한다.** 사용자가 원하는 복원된 G1 전투 세이튼은 현재 `MN_RPCT_05 / boss.kakulsaydon.g1.saydon`이다. 기존 00 컷신의 원본 외형을 보존할지, 05의 쿠크 파츠까지 보이게 할지 결과가 달라진다. 재질만 문제라면 00에도 native 입력이 이미 연결되어 있으므로 모델 교체 없이 현재 실행 산출물의 반영 여부를 먼저 확인할 수 있다.
2. **05의 같은 9개 clip으로 12개 구간을 옮긴다.** 기존 Composition의 animation occurrence 또는 existing boss presentation 경로를 소비한다. start/trim/playbackRate/loop, world root transform, camera 시간을 함께 유지한다. 같은 시간에 `original_kouku` Deploy WORLD와 새 05 Animation을 둘 다 켜 두면 두 actor가 생기므로 몸체 lane의 owner는 하나여야 한다.
3. **재질 경로는 지금의 ActorCatalog→CModel→CMaterial을 유지한다.** 컷신 전용 두 번째 loader/shader/runtime을 만들 이유가 없다. 00의 세 번째 MIC나 05의 쿠크 slot을 같은 재질 이름으로 덮어씌우지 않는다. 실제 모델 ID별 입력을 사용한다.
4. **Preview의 복구와 제품 전투 인계를 구분해 완성한다.** Preview Stop/Replay는 시작 전 clip·pause·transform·visibility를 되돌려 다시 재생 가능해야 한다. 제품 전투는 stable placement/entity, Server encounter activation 또는 phase transition, Client replicated presentation 활성화로 이어져야 한다. 컷신 prop을 CNpc로 임의 승격하거나 Client가 전투 entity를 로컬 spawn하지 않는다.
5. **G1 입장 위치·camera·전투 spawn을 같이 연결한다.** 현재 204.7m 떨어진 Z737 컷신과 Z942 전투 위치를 그대로 두고 모델 ID만 바꾸면 즉시 전투 인계가 되지 않는다. cutscene에서 combat까지 Server가 승인한 위치/phase를 유지하고, camera owner 반환·입력 복구·stop/failure 경로를 함께 검사한다.
6. **이펙트는 clip 목록과 별도 occurrence로 묶는다.** 본체 action clip, 손/무기 clip, 원본 Effect occurrence의 birth/duration/anchor, camera/UI/sound를 한 패턴 시간축에서 확인한다. `full.restore`라는 이름만 붙이는 것으로 원본 emitter/재질/수명까지 복원되지는 않는다. 사용자 캐릭터 이펙트의 수명 가설과 전체 보스 패턴별 비교는 통합 조사 문서가 담당한다.

## G07. 수행한 검증과 남은 확인

| 구분 | 이번 세션 결과 |
|---|---|
| LAN | `server-host`, TCP7777 LocalSubnet ready, `192.168.0.14:7777` not-listening. 설정 완료 |
| 작업 트리 | 기존 대규모 dirty 확인. 자동 stage/commit/push와 복구 없음 |
| Sequence 데이터 | 독립 JSON, Authoring/Runtime WorldSequence, BossCatalog parse 성공 |
| runtime 배포 대응 | worldsequences revision427의 semantic 일치. Deploy catalog/placement는 줄바꿈 정규화 후 전문 일치 |
| 설치 리소스 | 두 WModel `info` 성공, 각249clip. 8 material override의 고유 texture31개 존재 |
| clip/skeleton 비교 | WMOD/WSKL/WANM layout을 실제 파일에서 읽어 G04 수치 산출 |
| 제품 변경 | 없음. 이번 문서 한 개 추가 |
| 문서 검사 | UTF-8 읽기·빈 파일 아님·줄 끝 공백0, 문서 범위 `git diff --check` 성공. 미추적 문서는 별도 본문 검사로 보완 |
| 컴파일·publisher | 제품 변경이 없어 실행하지 않음. 이전 RESULT의 실행 증거와 구분 |
| Client/Server/UI | 자율 실행·조작·화면 캡처 없음 |
| 화면 및 전투 인계 | 사용자가 아직 확인하지 않은 상태. visual PASS와 seamless handoff 완료로 기록하지 않음 |

사용자 수동 확인 경로는 Visual Studio `Server + Client` profile → Lobby의 `KoukuSaydon` → F1 → `Open Sequencer Benchmark` → `연출_팝업북` → `Play Sequence`다. 독립 시퀀서에서 종료/Reset/재재생 상태와 외형을 먼저 확인할 수 있다. 기존 제품 trigger나 F1 관문 버튼은 서로 다른 실행 경로이므로 결과를 구분해서 기록해야 한다.
