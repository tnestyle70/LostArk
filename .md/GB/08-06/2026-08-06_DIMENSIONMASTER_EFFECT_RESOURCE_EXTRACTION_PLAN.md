# 차원술사 이펙트 리소스 추출·Imported 초안 적용 계획

## 목표

차원술사부터 원본 Cascade 정보를 실행 가능한 Effect Document 초안으로 옮긴다. 첫 골든 케이스는
`F / 2050500 / 업의 경계` 포탈이다.

완료 흐름은 다음과 같다.

```text
원본 Skill Effect Notify
  -> ParticleSystem 12개와 재생 시각 복원
  -> 각 System의 LOD0 Emitter 88개 분할
  -> 외부 공용 Module 참조까지 추출
  -> 현재 Effect Detail 값으로 변환
  -> Tool에서 Load 가능한 Imported .effect.json
  -> Preview와 Model View에서 눈 검수
  -> 사람이 튜닝한 뒤 Save As Authored
```

자동 변환은 완성품을 추측하는 과정이 아니다. 빈 문서 대신 원본 근거가 있는 초기값을 제공하는
과정이다. 모든 변환값은 `EXACT`, `APPROXIMATION`, `UNSUPPORTED` 중 하나로 영수증에 남긴다.

## 현재 실측

- F timeline의 ParticleSystem: 12개
- LOD0 Emitter: 88개
- Sprite Particle 후보: 53개
- Mesh Particle 후보: 28개
- Decal 후보: 2개
- Light/Post처럼 현재 Effect Element로 표현할 수 없는 Emitter: 5개
- Dynamic Material Parameter entry: 140개
- Burst entry: 25개
- Runtime Mesh/Texture binding: 151/151 해결
- Material binding: 78/81 해결
- 현재 `skill.2050500.imported-effect-draft.json`은 참고 자료이며 실제 `elements`가 없다.
- 현재 source receipt의 `external_graph_package_not_loaded`는 LOD0 기준 659회다. 이는 공용
  Lifetime/Velocity/Size/Color Module을 다른 패키지에서 재사용하기 때문이다.

## 정본 파일과 의존성

| 파일 | 필요한 이유 | 연결 |
|---|---|---|
| `Tools/LevelPlacementExtractor/extract_ue3_particle_module_closure.py` | F LOD0가 참조하는 외부 공용 Module과 Distribution만 정확히 추출한다. | source receipt의 unresolved object path -> 원본 UPK |
| `Tools/LevelPlacementExtractor/build_imported_effect_documents.py` | Emitter와 Module을 현재 v6 Effect Detail로 변환한다. | normalized graph + module closure + runtime bindings -> `.effect.json` |
| `Tools/LevelPlacementExtractor/test_build_imported_effect_documents.py` | Emitter 분할, burst 분할, 값 변환과 미지원 보존을 고정한다. | importer 회귀 검증 |
| `Data/Effects/Imported/DimensionMaster/Modules/skill.2050500.external-module-closure.json` | 외부 Module 원본 근거를 Git 관리 가능한 작은 자료로 보존한다. | 변환기의 입력 |
| `Data/Effects/Imported/DimensionMaster/Converted/effect.dimensionmaster.skill.2050500.imported.effect.json` | Effect Tool이 바로 Load/Preview할 수 있는 실행 초안이다. | Data Files Imported -> Active Document |
| `Data/Effects/Imported/DimensionMaster/Converted/skill.2050500.element-conversion-receipt.json` | Element별 자동 변환과 수동 튜닝 경계를 보존한다. | 원본 node/parameter -> Tool 값 |

새 C++ 파일은 추가하지 않는다. Tool은 이미 유효한 Imported `.effect.json`을 로드할 수 있으므로,
우선 변환 데이터 경로를 닫는다. 실제 검수에서 현재 runtime 표현력이 부족한 항목이 시각적으로
핵심임이 확인될 때만 C++ schema/playback을 확장한다.

## 변환 원칙

### Emitter 분할

- `ParticleSystem -> Emitter -> 첫 LOD(LOD0)`만 사용한다.
- Module node는 별도 Element가 아니라 소속 Emitter의 Detail 값이 된다.
- 같은 Emitter의 여러 Burst는 현재 runtime의 단일 시작 Burst 계약에 맞춰 시각별 Element로
  분할한다.
- Skill Effect Notify의 재생 시각을 모든 Element의 `startDelaySeconds`에 더한다.

### 현재 Detail로 직접 옮길 값

- Required: local space, emitter duration, loop, screen alignment
- Spawn: continuous rate, burst count/time
- Lifetime: min/max seconds
- Velocity: initial min/max, constant acceleration
- Size: start/end size
- Color: 시작/끝 color multiply와 emissive 근사값
- Material: Base/Noise/Mask/Emissive/Dissolve texture 후보, UV speed/tile, distortion/emissive scalar
- TypeDataMesh: source mesh model과 material texture
- TypeDataDecal: Decal Element와 size/depth 초기값

### 처음부터 수동 경계로 남길 값

- 원본 Material shader graph 자체와 현재 `effect.standard` HLSL의 차이
- Mesh Particle의 개별 입자 instancing/rotation/velocity
- Light, FilmNoise, ZoomBlur, RGBNoise
- Cylinder/Sphere/Surface location처럼 현재 Particle schema에 없는 spawn volume
- 임의 곡선의 중간 key, VelocityOverLife, RotationOverLife, Dynamic Parameter의 커스텀 의미
- 원본 socket/pivot와 프로젝트 Model View pivot의 최종 정렬
- 색, 밝기, bloom, distortion, dissolve의 최종 미감

위 항목은 삭제하지 않고 conversion receipt에서 원본 값과 이유를 보존한다.

## 구현·검증 순서

1. source receipt의 잘못된 `unsupportedOrUnresolved` 키 소비를 `unsupportedUnresolved`로 고친다.
2. F LOD0 외부 참조 object path만 물리 UPK에서 추출하고 Distribution closure를 만든다.
3. 88 Emitter를 실제 Element/미지원 항목으로 분할한다.
4. v6 Effect Detail 기본 구조를 채우고 runtime asset ID가 실제 존재하는지 검증한다.
5. Imported `.effect.json`과 conversion receipt를 원자적으로 생성한다.
6. Python 단위 테스트, JSON parse, Effect audit, ProjectAudit를 실행한다.
7. Client x64 Debug를 빌드한다.
8. F1 Effect Tool에서 Imported 문서를 Load하고 Dimension Summon 모델의 두 clip과 함께 Preview한다.

## 완료 조건

- F Imported 문서가 Data Files에서 비활성 참고 행이 아니라 Load 가능한 행으로 보인다.
- 문서의 모든 drawable Element가 필요한 Base 또는 Mesh Model resource를 가진다.
- `EXACT / APPROXIMATION / UNSUPPORTED` 합계가 조사한 LOD0 Emitter 및 파라미터 근거와 맞는다.
- 기존 Authored F placeholder를 자동 덮어쓰지 않는다.
- Imported 초안의 눈 검수 후에만 `Save As -> Authored`로 승격한다.
- 자동 변환 완료와 수동 튜닝 완료를 RESULT에서 분리한다.

## 확정한 얇은 런타임 확장

원본 Cascade 전체를 두 번째 런타임으로 복제하지 않는다. 현재 Effect Document와
Playback의 의미를 유지하면서 F 복원 효과가 큰 두 항목만 확장한다.

| 확장 | 들어가는 위치 | 필요한 이유 |
|---|---|---|
| Particle의 선택적 `meshModel` binding | 기존 `particle` Element의 Resources | TypeDataMesh 28개를 정적 Mesh 한 개가 아니라 실제 particle spawn/lifetime/velocity로 재생한다. |
| `initialPositionMin/Max` | `EFFECT_PARTICLE_DESC`, codec, Tool, Playback | 단순 Cascade StartLocation 범위를 입자별 초기 위치로 보존한다. |

Mesh Particle는 여섯 번째 Element kind를 만들지 않는다. `particle`이 `meshModel`을
가졌는지로 renderer shape를 결정한다. 기존 v5/v6 문서에 새 위치 필드가 없으면 0 범위를
default로 사용하므로 호환 load는 유지한다.

여러 Burst는 시간별 Element 분할로 표현하고, Size/Color 곡선은 기존 Linear Lerp로 먼저
근사한다. Light, screen-space Post Effect, Orbit과 임의 곡선은 이번 얇은 확장에 넣지 않는다.
