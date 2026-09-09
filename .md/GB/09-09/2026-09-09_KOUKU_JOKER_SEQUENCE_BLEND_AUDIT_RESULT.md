# 조커 찾기 시퀀스의 0.1초 애니메이션 블렌딩 적용 상태

작성일: 2026-09-09. 상태: **현재 코드·Source/Product 읽기 전용 조사**.

현재 조커 찾기 Composition 재생에는 시퀀스/clip 경계를 잇는 **0.1초 crossfade가 적용되어 있지 않다.**
animation keyframe 사이의 위치·회전 보간과 다른 clip/occurrence로 전환할 때의 crossfade는 구분한다.
일반 캐릭터의 crossfade 구현이 있다는 사실만으로 조커 찾기의 실제 소비자에도 적용됐다고 볼 수 없다.

## 현재 호출 경로

| 경로 | 현재 값과 처리 | 근거 |
|---|---|---|
| 일반 CCharacter | `CLIP_BLEND_SECONDS=0.12f`, 공통 animation setter에 전달 | [상수](C:/Users/user/Desktop/LostArk/Client/Private/Character.cpp:34), [호출](C:/Users/user/Desktop/LostArk/Client/Private/Character.cpp:2192) |
| 조커 찾기 Workbench 단독/Bundle Preview | `Set_Animation(..., 0.f)` 후 pause·track seek·`Update_Animation(0.f)` | [Sample_BundlePreview](C:/Users/user/Desktop/LostArk/Client/Private/KoukuSaydonPresentationPlayer.cpp:1794) |
| Composition Model View 경로 | `Start_PreviewClip(..., 0.f)` 후 정지 seek | [Animation_Tool](C:/Users/user/Desktop/LostArk/Client/Private/Animation_Tool.cpp:12219) |
| 별도 기존 Local Pattern Preview | 설정값을 사용하며 기본 0.12초 | [호출](C:/Users/user/Desktop/LostArk/Client/Private/Animation_Tool.cpp:12373), [설정 기본값](C:/Users/user/Desktop/LostArk/Client/Public/Animation_Tool.h:897) |
| Server Product | 보통 0.05초, `unblendedBoneContact`면 0초. 이후 서버 action age seek에서 `Skip_Blend()` | [전환](C:/Users/user/Desktop/LostArk/Client/Private/ClientReplication.cpp:3638), [Skip_Blend](C:/Users/user/Desktop/LostArk/Client/Private/ClientReplication.cpp:3670) |

actorProfileId가 있는 조커 찾기 P12/P13/P14는 단독 Play도 임시 Bundle을 만들고
`Begin_BundlePreview`로 들어간다. 따라서 별도 Local Pattern Preview의 0.12초 설정을 소비하지 않는다.
[실제 분기](C:/Users/user/Desktop/LostArk/Client/Private/MainApp.cpp:1541)를 확인했다.

조사 시점 Source/Product는 revision 176이며 P12는 2개, P13은 21개, P14는 1개 clip이다.
전부 playRate 1이고 Source occurrence에는 blend duration field가 없다.
P13의 Product binding 21개 모두 `unblendedBoneContact=true`다.
[현재 binding](C:/Users/user/Desktop/LostArk/Data/Animation/Authored/KoukuSaydon/KoukuSaydon.patternbindings.json:838)과
[기존 G11 결과](C:/Users/user/Desktop/LostArk/.md/GB/09-07/2026-09-07_KOUKU_GATE_PATTERN_BUNDLE_RESULT.md:117)는
서버의 baked bone contact pose와 제품 전환의 50ms blend 차이를 제거하려고 이 flag를 만들었다는 점에서 일치한다.
이 기록을 사용자가 어제 요청한 0.1초 적용의 완료 증거로 읽지 않는다.

## Engine 보간의 실제 조건

[CModel::Set_Animation](C:/Users/user/Desktop/LostArk/Engine/Private/Model.cpp:125)은 animation index가 달라질 때만
`Begin_AnimBlend`를 호출한다. 같은 clip 재시작이나 source window 변경에는 새 blend를 만들지 않는다.
[Model.h](C:/Users/user/Desktop/LostArk/Engine/Public/Model.h:131)의 기본 blend 인자도 0초다.
`Notify_Clip`은 Effect 발생 통지이며 이 값을 켜는 기능이 아니다.

실제 blend는 이전 local bone pose를 저장하고 새 pose의 position/scale을 lerp, rotation을 quaternion slerp한다.
[Bone.cpp](C:/Users/user/Desktop/LostArk/Engine/Private/Bone.cpp:44)의 계산은 존재한다.
다만 현재 blend elapsed는 animation update에 전달되는 `delta * animationSpeed`로 진행한다.
따라서 0.12초는 2배속에서 실제 약 0.06초가 되고, `Update_Animation(0)`만 반복하면 시간이 진행되지 않는다.

## 필요한 후속 범위와 이번 조사 경계

0.1초 전환을 실제 적용하려면 연속 재생의 occurrence/sequence 경계에서 이전 표시 pose를 유지하고,
같은 clip의 재시작도 식별해 100ms 동안 새 pose로 전환해야 한다. 사용자 seek/scrub, late join과
일반 연속 재생을 같은 무조건 `Skip_Blend` 처리로 덮지 않아야 한다.
속도 변경에도 100ms wall time을 의미할지 데이터 계약으로 정하고 preview와 Product가 같은 정책을 소비해야 한다.

P13은 본 궤적을 Server CONTACT가 소비하므로 단순히 0→0.1 상수를 바꾸면 화면의 뿅망치와
서버 접촉 궤적이 다시 어긋날 수 있다. 전환 pose와 bake/sampling 정책까지 같은 변경에서 맞추고,
CONTACT 결과와 손/무기 Effect anchor를 함께 확인해야 한다.

이번 추가 요청은 적용 상태 질문이므로 animation/Server 코드는 수정하지 않았다.
코드·문서·JSON의 호출/값을 확인했으며 Client 재생·시퀀스 시각 확인·캡처는 수행하지 않았다.
동시에 진행한 차원술사 1.5배 외형 변경은 이 crossfade 적용 작업과 별개다.

## 100ms 전환과 Server CONTACT의 실제 시간 대조

후속 질문에 맞춰 revision 176의 P12/P13/P14 모든 내부 occurrence 경계를 읽었다.
[실측 JSON](C:/Users/user/Desktop/LostArk/out/JokerBlend20260909/transition_contact_audit.json)은
Source 누적 시간, 실제 projector의 Server stage 시작 시간, CONTACT window와 baked region key 수를 함께 기록한다.
범위는 제안된 `[전환 시각, 전환 시각+100ms)`이며 현재 구현에 blend를 켠 실행 결과는 아니다.

| Pattern | occurrence / 내부 전환 | 같은 clip 재시작 | 다른 clip 전환 | 이후 100ms에 CONTACT가 있는 전환 |
|---|---:|---:|---:|---:|
| P12 쿠크 조커찾기 | 2 / 1 | 0 | 1 | 0 |
| P13 대형 세이튼 조커찾기 | 21 / 20 | 5 | 15 | 3 |
| P14 대형 세이튼 조커찾기 성공 | 1 / 0 | 0 | 0 | 0 |

P13의 겹치는 세 경계는 모두 `ao_att_battle_1_03 → ao_att_battle_1_01`이다.
아래 occurrence와 logic ID는 `KAKULSAYDON_G1_PATTERN_13.` 접두사를 생략했다.

| 새 occurrence | Source 전환 ms | Product 전환 ms | 활성 CONTACT 구간 ms | 해당 logic | blend 중 Server tick |
|---|---:|---:|---|---|---|
| animation.16 | 5664 | 5666.667 | 4858–5902 | logic.2, logic.3 | 170, 171, 172 |
| animation.12 | 11930 | 12000 | 11244–12289 | logic.14, logic.17 | 360, 361, 362 |
| animation.27 | 20891 | 21033.333 | 20205–21250 | logic.22, logic.23 | 631, 632, 633 |

세 경계 모두 제안된 100ms 전체가 CONTACT 창 안에 있다. 각 타격의 중앙 1개와 측면 2개,
총 9개 region의 현재 baked track에도 해당 Product 구간 안의 key가 region마다 5개씩 존재한다.
이는 판정이 실행될 수 있는 창과 궤적 샘플이 겹친다는 뜻이다. 카드별 이미 소비된 접촉, 실제 교차 여부,
성공/실패 횟수를 실행으로 검증한 것은 아니다.

P13의 나머지 17개 경계와 P12의 1개 경계는 이 100ms 동안 CONTACT가 없다.
같은 clip 재시작 5개는 전부 이쪽에 속한다. 다음 CONTACT가 남은 경계 중 가장 짧은 여유도
100ms 종료 후 71.667ms다. 따라서 해당 전환들에서 정확히 100ms에 pose가 수렴하고
원래 source clock과 root motion을 유지한다면, 시간 겹침 때문에 Server CONTACT 궤적을 바꿀 필요는 없다.
P13 성공으로 P14가 시작되는 패턴 간 동적 전환은 이 내부 경계 집계에 포함하지 않았다.

Server 시계는 Source stage duration의 단순 합과 같지 않다.
[_bone_bake_stage_origins](C:/Users/user/Desktop/LostArk/Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py:2281)는
30Hz 올림과 최초 진입 tick 보정을 적용한다. P13 마지막 내부 전환은 Source 27185ms,
Product 27366.667ms로 181.667ms 차이다. 보간과 재bake는 이 실제 Product clock을 소비해야 한다.

[GameplayCatalog](C:/Users/user/Desktop/LostArk/Server/Private/GameplayCatalog.cpp:2592)는
`PATTERNLOGICREGIONWORLD/KEY`를 typed CONTACT region track으로 읽고 clock·순서·transform을 검증한다.
[LogicRuntime](C:/Users/user/Desktop/LostArk/Server/Private/KoukuSaydonLogicRuntime.cpp:159)은
`BOSS_CURRENT` track을 절대 pattern elapsed tick에서 샘플하고,
[CONTACT 평가](C:/Users/user/Desktop/LostArk/Server/Private/KoukuSaydonLogicRuntime.cpp:628)는 종료 tick까지 timeout보다 먼저 처리한다.
[GameRoom](C:/Users/user/Desktop/LostArk/Server/Private/GameRoom.cpp:6764)이 pinned gameplay revision의 이 경로를 호출한다.

따라서 보간 적용 시 Server 보정을 전역 필수 조건으로 확대할 필요는 없다.
다만 세 CONTACT 활성 전환까지 자연스럽게 연결하려면 화면과 같은 전환 pose를 기존 bone bake가
샘플하도록 맞춰야 한다. CONTACT 시각을 임의 이동하거나 Client 판정으로 우회하는 방법은 이 조사에서 제안하지 않는다.
현재의 pattern 전체 `unblendedBoneContact`만 제거하는 것으로는 이 일치를 보장하지 못한다.

실행한 검사는 읽기 전용 시간 대조 스크립트 1회와 결과 JSON parse다. Source·Encounter·binding
3개 입력 SHA-256의 검사 전후 동일함을 확인했다. 런타임 코드와 Product JSON은 수정하지 않았고,
이번 조사에서 컴파일, Client/Server 실행, 실제 접촉 결과·시각 검증은 하지 않았다.
