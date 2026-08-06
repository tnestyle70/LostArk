# 차원술사 체험 모드 스킬 바인딩·Effect Resource Index 구현 결과

## 1. 결론

차원술사 체험 모드의 body animation과 Effect 정본을 `PlayerSkills.json`과
`DimensionMaster.skillbindings.json`의 교집합으로 다시 고정했다. 과거 후보 스킬 ID를 현재 슬롯 이름으로
재사용하지 않으며, 현재 제품 카탈로그에는 기본 공격 aggregate와 BA1~BA4를 포함한 16개 Effect만 게시된다.

업로드 이미지에서 보인 연결 오류도 정본 ID 기준으로 분리했다.

- 첫 두 장은 현재 S가 아니라 과거 후보 `T=2050510`을 재생한 증거다.
- 다음 여섯 장은 현재 R이 아니라 과거 후보 `R=2050190`을 재생한 증거다.
- 마지막 원작 이미지는 현재 정본 `R=2050180 / foldcut`의 비교 자료다.

따라서 흰색·보라색 사각형과 잘못된 연출의 1차 원인은 단순 스케일 튜닝이 아니라 잘못된 skill/effect
identity였다. 과거 후보 문서는 Imported/Authored 근거로 보존하지만 Assembly, WFX, Runtime Catalog에는
게시하지 않는다.

## 2. 확정된 체험 모드 계약

| 입력 | skillId | body clip | Effect |
|---|---:|---|---|
| LMB | 2050010 | `battle_1_01~04` | aggregate + BA1~BA4 |
| Q | 2050100 | `nailstrike_01` | `effect.dimensionmaster.skill.2050100` |
| W | 2050120 | `dimensionalleap_01~03` | `effect.dimensionmaster.skill.2050120` |
| E | 2050160 | `overslash_01~05` | `effect.dimensionmaster.skill.2050160` |
| R | 2050180 | `foldcut` | `effect.dimensionmaster.skill.2050180` |
| A | 2050210 | `willowrend` | `effect.dimensionmaster.skill.2050210` |
| S | 2050220 | `momentaryrift` | `effect.dimensionmaster.skill.2050220` |
| D | 2050240 | `telekinesisthrust_01/04` | `effect.dimensionmaster.skill.2050240` |
| F | 2050230 | `chronorecoil` | `effect.dimensionmaster.skill.2050230` |
| T | 2050500 | `dimensionprison` | `effect.dimensionmaster.skill.2050500` |
| V | 2050520 | `timewave` | `effect.dimensionmaster.skill.2050520` |
| ALT_V | 2050540 | `super_timewave` | `effect.dimensionmaster.skill.2050540` |

`PlayerSkills.json`에서 비어 있던 Q/W/E/R/F/V의 `effectId`를 위 정본으로 채웠고 provenance receipt도
같은 변경에서 동기화했다. `DimensionMaster.animevents`의 과거 후보 6행은 제거했다. 명시 animation cue가
없는 현재 ACTIVE 스킬은 기존 `Spawn_FallbackEffect`가 `PlayerSkills.effectId`의 complete Effect를 한 번
시작한다. body animation과 Effect는 서로 종속된 재생물이 아니라 동일한 승인 action에서 시작하는 두
presentation 출력이다.

## 3. Resource Index

`Data/Effects/ResourceIndex/DimensionMaster` 아래에 Q/W/E/R/A/S/D/F/T/V/ALT_V 폴더와 class index를
생성했다. 바이너리를 슬롯별로 복제하지 않고 다음 정보만 역참조 가능한 JSON으로 보존한다.

- 실제 body clip, skillId, effectId
- Mesh/Texture/Model Resources-relative asset ID
- Material identity와 runtime shader profile
- Element/Emitter/Model Cue 사용처
- 물리 리소스 PRESENT/MISSING 상태

최종 결과는 `11/11 INDEX_READY`, 고유 asset 384개, 고유 Material identity 300개이며 물리 리소스는
전부 PRESENT다. 두 번째 생성은 `changedFileCount=0`으로 멱등성을 통과했다. T에는 Mesh 16개,
Texture 127개, Summon Model 1개와 Model Cue 1개가 기록된다.

Effect Tool의 기존 `Assembly -> Component -> Emitter -> Resources -> Modules` 트리와 선택 Element의
resource 표시는 그대로 사용한다. class Resource Index를 직접 필터링하는 별도 하단 Resource Library UI는
이번 실행기 경계에 포함하지 않았으며 다음 Tool 수직 슬라이스다.

## 4. 이미지 보존

업로드 이미지는 다음 폴더에 원본 PNG로 보존했다.

`C:/Users/user/Desktop/로스트아크이펙트이미지/차원술사_복원`

- `DimensionMaster_T_2050510_MISMATCH00~01.png`
- `DimensionMaster_R_2050190_MISMATCH00~05.png`
- `DimensionMaster_R_2050180_REFERENCE00.png`

이 이미지는 timeline이나 transform 값을 역산하는 정본이 아니라, 정본 데이터 실행 후 Material·blend·노출
차이를 찾는 GPU A/B 기준으로만 사용한다.

## 5. 검증

- Gameplay balance Validate: 6 profiles / 125 skills / 77 damage profiles / 1 boss
- skill binding fast harness: failures 0
- Resource Index: 11/11 ready, missing 0, 두 번째 생성 변경 0
- Effect project registration: 429 files / 47 filters, dangling 0
- ProjectAudit: 72 checks PASS
- Client startup smoke: Debug Client 12초 생존 PASS

## 6. 남은 수동 경계

현재 ID와 리소스 정본 연결은 완료됐다. 아직 수행하지 않은 것은 실제 GPU 창에서 현재 R 2050180과
T 2050500을 선택해 저장된 원작 PNG와 동일 카메라로 비교하는 작업이다. 그 비교 전에는 과거 이미지의
크기·색을 기준으로 수동 scale/emissive 값을 넣지 않는다.
