# 거대 서커스 망치 원본 재조사

작성일: 2026-09-09. 범위: 설치된 원본 LPK/UPK 읽기 전용 조사. 런타임 구현·리소스 교체 요청이 아니다.

## 결론

`망치 실물이 없으므로 순수 파티클이며 원본 수치는 추출할 수 없다`는 종전 요약은 근거가 부족하다.
원본 NPC 테이블에 **초대형 서커스 망치**가 존재하며, 전용 `MN_UMAC_01` SkeletalMesh, 재질, 텍스처, AnimSet 및 소환 시 공격 스킬로 연결된다.
현재 프로젝트의 `bingo.hammer_1`에 지정된 `WP_MN_RPCT_06`과는 다른 모델 ID다.

단, 이 조사에서 `4219851의 이벤트 → 특정 소환 노드 → 빙고판을 가로지르는 망치`의 전체 연결은 확정하지 않았다.
NPC 이름·분류 및 전용 동작 존재는 확인했지만, 이것만으로 사용자가 본 빙고 연출의 정확한 occurrence라고 단정하거나 현재 모델을 교체하지 않는다.

## 원본 위치와 읽기 방법

- 게임 루트: `C:/ProgramData/Smilegate/Games/LOSTARK/EFGame`
- 테이블: `data2.lpk`, `/EFGame_Extra/ClientData/TableData/EFTable_Npc.db`
- 모델 정의: `data4.lpk`, `/EFGame_Extra/ClientData/XmlData/LookInfo/Monster/EFDLChar_MN_UMAC_01.MN_UMAC_01.loa`
- 액션: `data3.lpk`, `/EFGame_Extra/ClientData/XmlData/Action/MN_UMAC_01.loa`
- 모델 패키지: `ReleasePC/Packages/9G1MT9XB1BZ6E7MQE464S5P.upk` = `MN_UMAC_01`
- 이펙트 패키지: `ReleasePC/Packages/YGI3SBI3SVBZD3I18GHJMH26U.upk` = `FX_MN_UMAC_01`

기존 LPK 추출 helper의 복호화 결과를 메모리에서 읽고 SQLite deserialize로 NPC 행을 조회했다.
UPK는 기존 `extract_ue3_placements.py`의 summary/decompress/name/import/export/tagged-property parser로 조회했다.
원본 게임 파일을 변경하지 않았다.

## 확인된 NPC 연결

| NPC ID | 원본 Comment1 | 모델 | ModelSize 원시값 | SummonSpawnSkillId |
|---|---|---|---:|---:|
| 480711 | 초대형 서커스 망치_즉시 | EFDLChar_MN_UMAC_01.MN_UMAC_01 | 100 | 4222501 |
| 480721 | 초대형 서커스 망치_3초뒤 | EFDLChar_MN_UMAC_01.MN_UMAC_01 | 100 | 4222502 |
| 480751 | 초대형 서커스 망치_3초뒤 | EFDLChar_MN_UMAC_01.MN_UMAC_01 | 50 | 4222502 |

세 행의 ClassifyIndex는 `3708100`이다. ModelSize 원시값을 현재 엔진의 미터 또는 transform scale과 동일시하지 않는다.

## 전용 메시·재질·텍스처

LookInfo가 직접 참조하고 UPK export에서도 확인한 객체:

- `SkeletalMesh'MN_UMAC_01.Mesh.MN_UMAC_01_SK'`
- `MaterialInstanceConstant'MN_UMAC_01.Mat.MN_UMAC_01_MI'`
- `AnimSet'MN_UMAC_01.Ani.MN_UMAC_01_Ani'`
- `PhysicsAsset'MN_UMAC_01.Ani.MN_UMAC_01_Physics'`

재질의 texture parameter 참조를 확인했다:

| 재질 parameter | 원본 Texture2D |
|---|---|
| texture_diffuse | MN_UMAC_01.Tex.mn_umac_01_d |
| texture_normal | MN_UMAC_01.Tex.mn_umac_01_n |
| texture_specular | MN_UMAC_01.Tex.mn_umac_01_s |

이는 단순히 이름이 비슷한 파일을 찾은 것이 아니라 NPC → LookInfo → mesh/material → texture 연결이다.
현재 `Client/Bin/Resources/Character`, `Effect/KoukuSaydon` 파일명 검색에서는 이 모델 ID의 runtime 자산을 찾지 못했다. 이번 작업에서 추출·cook·배포하지 않았다.

## 원본 애니메이션 및 부가 이펙트

UPK의 AnimSequence 8개를 확인했다. 주요 clip의 직접 읽은 값:

| clip | SequenceLength 초 | NumFrames |
|---|---:|---:|
| respawn_1 | 약 1.166667 | 34 |
| idle_battle_1 | 약 1.666667 | 51 |
| att_battle_1 | 약 3.333333 | 101 |

나머지는 `idle_normal_1`, `att_battle_11`, `att_battle_2_01/02/03`이다.
SequenceLength는 asset의 clip 길이이며 실제 공격 전체 시간·재생 속도·궤적의 최종 월드 수치로 단정하지 않는다.

Action `4222501`의 원본 이름은 `초대형 서커스 망치_공격`, `4222502`는 `초대형 서커스 망치_공격_3초`다.
액션의 Anim notify에 `Respawn_1`, `Att_Battle_1`이 있고, 4222502에는 `Idle_Battle_1`도 있다.
부가 particle notify가 다음을 참조한다:

- `FX_MN_UMAC_01.Par_X_UMAC_Wind_01`
- `fx_mn_umac_01.Par_X_UMAC_Arrow_01_LOC_INT` (4222502)

따라서 이 자산의 구조는 메시/애니메이션을 가진 본체와 별도 파티클 표현이 함께 존재하는 구조다.

## 종전 액션 요약 재검사

`MN_RPCT_05.loa`의 Action ID `4219851`, 이름 `세이튼_거대 망치 요청`을 직접 읽었다.
원본 decoded byte 구간은 `2514398..2518902`이며 4,504 bytes다.
이 action에는 7개 Stage가 있고 `CEFActionNotify_Anim`, 확률 기반 stage 전환 notify 및 신호 notify가 있다.
신호 문자열은 `Event_06`, `Event_07`뿐 아니라 **`Event_08`, `Event_09`도 존재**한다.
분기별 선택과 실행 순서는 이 문자열 나열만으로 확정하지 않는다.

현재 37081 `TriggerMapData.loa`에서 Event_06/07의 단순 문자열 검색은 룰렛·아이언메이든·빙고 이동 등의 다른 unit에서도 일치한다.
이벤트 이름만으로 거대 망치 소환 연결을 확정하면 안 된다. 37081 DeployData의 NPC placement 스캔에서는 위 세 NPC ID가 검출되지 않았다.
이는 해당 정의 또는 메시가 없다는 증거가 아니며, 동적 소환 경로/활성 사용 여부가 미확정이라는 경계다.

## 파티클 읽기 가능 범위

기존 `extract_ue3_particle_graph.py`로 실제 원본을 읽었다:

| package | particle systems | graph objects | property parse errors | decoded raw distributions |
|---|---:|---:|---:|---:|
| FX_MN_RPCT_06_X | 81 | 5584 | 0 | 5920 |
| FX_MN_UMAC_01 | 5 | 351 | 0 | 374 |

태그 속성 파싱 성공은 원본 렌더링의 완전한 재현이나 상속·외부 모듈까지 전부 해석했다는 뜻이 아니다.
그러나 `파티클 속성을 하나도 읽지 못하므로 원본 숫자가 없다`는 일반화는 이 측정과 맞지 않는다.

## 현재 프로젝트와 완료 경계

현재 `Data/Effects/V2/Authored/bingo.hammer_1.effectv2.json`은 `effectType: Mesh`,
`Character/KoukuSaton/WP_MN_RPCT_06/WP_MN_RPCT_06.wmodel` 및 해당 diffuse DDS를 소비하며 scale start/end는 `[5,5,5]`다.
이는 프로젝트에서 저작한 값이며 이번에 발견한 `MN_UMAC_01`의 원본 scale/동작으로 검증된 값이 아니다.

완료: 원본 NPC/LookInfo/mesh/material/texture/animation/attack/particle 참조 조사 및 기록.
미완료: 빙고 특정 occurrence와 신호-소환 연결 확정, 원본 모델 육안 대조, 변환/배포, 런타임 반영.
Client/Server 실행, UI 조작, 화면 캡처, 시각 PASS 판정, C++ 변경, 리소스 교체는 수행하지 않았다.
