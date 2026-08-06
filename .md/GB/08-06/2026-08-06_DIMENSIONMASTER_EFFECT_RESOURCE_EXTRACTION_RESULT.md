# 차원술사 F 이펙트 리소스 추출·Imported 초안 적용 결과

## 결론

차원술사 `F / 2050500 / 업의 경계`의 Cascade 근거를 현재 Effect Document v6로
변환했다. 결과는 참고용 Draft가 아니라 Effect Tool의 `Data Files > Imported`에서
직접 Load할 수 있는 실행 문서다.

원본 Cascade 전체를 복제하지 않고 현재 구조를 얇게 확장했다.

- 기존 `particle` Element에 선택적 `meshModel` binding을 허용했다.
- 단순 StartLocation 범위를 `initialPositionMin/Max`로 추가했다.
- 여러 Burst는 시작 시각별 Element로 분할했다.
- Size/Color 곡선은 기존 시작/끝 Lerp 값으로 근사했다.
- Light, 화면 공간 Post Effect, Orbit과 복잡한 곡선은 억지 변환하지 않았다.

기존 Authored F placeholder는 덮어쓰지 않았다. Imported 문서를 사람이 확인한 뒤
`Save As -> Authored`로 승격하는 계약을 유지한다.

## 생성 결과

| 항목 | 결과 |
|---|---:|
| 원본 ParticleSystem | 12 |
| 조사한 LOD0 Emitter | 88 |
| 변환한 Emitter | 83 |
| 미지원 Emitter | 5 |
| Burst 분할 후 Element | 97 |
| Particle Element | 95 |
| Decal Element | 2 |
| Mesh-backed Particle | 31 |
| 총 Particle budget | 886 |
| 외부 Package | 100 |
| 외부 Module 요청 | 658 |
| 외부 Module 미해결 | 0 |

리소스 slot 결과는 Base 97, Noise 58, Mask 37, Emissive 26, Dissolve 19,
Mesh Model 31이다. 모든 기록된 runtime asset ID의 실제 파일 존재도 확인했다.

현재 런타임으로 표현하지 못한 5개 Emitter는 다음과 같다.

- Light renderer 2개
- ZoomBlur, RGBNoise, FilmNoise 화면 공간 Post Effect 3개

## 추가·수정한 파일과 연결

| 파일 | 반영 내용 | 연결 |
|---|---|---|
| `extract_ue3_particle_module_closure.py` | F LOD0의 외부 Module/Distribution closure를 추출한다. | 원본 UPK -> 외부 Module 근거 |
| `build_imported_effect_documents.py` | Emitter 단위로 Detail 값과 runtime resource를 변환한다. | normalized graph + closure -> v6 문서 |
| `skill.2050500.external-module-closure.json` | 해결한 658개 외부 참조 근거를 보존한다. | importer 입력 |
| `effect.dimensionmaster.skill.2050500.imported.effect.json` | Tool이 Load하는 97 Element 실행 초안이다. | Data Files Imported -> Active Document |
| `skill.2050500.element-conversion-receipt.json` | Element별 근거와 근사·미지원 경계를 보존한다. | 원본 Emitter -> Detail 값 추적 |
| `Effect_AuthoringDocument.h` | Particle 초기 위치 min/max를 추가했다. | Tool/codec/playback 공통 계약 |
| `Effect_DocumentCodec.cpp` | 선택 필드 parse/save/validate와 Particle의 meshModel을 허용했다. | JSON <-> Effect Document |
| `Effect_Playback.cpp` | 입자별 초기 위치를 결정하고 normalized life를 계산한다. | Detail 값 -> 재생 상태 |
| `Effect_DocumentRenderer.cpp` | meshModel이 있는 Particle을 기존 CModel 경로로 그린다. | Particle 평가값 -> Mesh shader |
| `Effect_Tool.cpp` | Initial Position Min/Max 편집 UI를 추가했다. | Effect Detail -> Document 값 |
| `Publish-Effects.ps1` | 새 선택 필드와 mesh-backed Particle 저장 계약을 검증한다. | Authored publish 검증 |

새 Element kind와 두 번째 model runtime은 만들지 않았다. Mesh Particle도 기존
`particle -> playback -> CModel/CMaterial` 경로를 소비한다.

## Effect Tool에서 확인할 결과

1. F1로 Effect Tool을 연다.
2. `Data Files`에서 Domain `DimensionMaster`, Source `Converted`를 선택한다.
3. `effect.dimensionmaster.skill.2050500.imported` 문서를 Load한다.
4. `All Effects` 트리에는 원본 System 그룹 아래 총 97개 Element가 나타난다.
5. Particle 95개 중 31개는 Mesh Model slot을 사용하고, 나머지는 Base texture를 사용한다.
6. Detail의 Particle 영역에서 `Initial Position Min/Max`를 편집할 수 있다.
7. Imported 문서를 저장하려면 기존 문서를 덮지 않고 `Save As -> Authored`를 사용한다.

자동 하네스는 이 문서가 parse/validate되고 drawable이며 Mesh-backed Particle이 존재함을
확인했다. 실제 GPU 화면에서 포탈의 색·크기·밝기·피벗이 원작과 같은지는 수동 눈 검수가
남아 있다.

## 수동 튜닝 경계

- 원본 procedural material 17개는 texture parameter만으로 모양을 복원할 수 없어 명시적인
  흰색 Base fallback을 사용한다. Material shader 의미를 눈으로 다시 맞춰야 한다.
- Base/Noise/Mask/Emissive/Dissolve 자동 분류는 원본 parameter binding 근거를 사용하지만,
  최종 의미는 thumbnail과 Preview로 확인해야 한다.
- 원본 위치·속도·가속도·크기는 프로젝트 단위에 맞춰 `0.01` 스케일을 가정했다.
- Mesh Particle의 개별 회전, Orbit, 완전한 3축 size curve는 현재 Detail로 완전 복원되지 않는다.
- Sphere/Cylinder spawn volume, 임의 곡선의 중간 key, Dynamic Parameter의 사용자 의미는
  receipt에 근거만 남겼다.
- Light 2개와 화면 공간 Post Effect 3개는 현재 Effect Document 밖이다.
- `DimensionMaster_DimensionSummon.wmodel`의 두 animation clip 생성·동기화는 particle
  Element가 아니라 Animation Effect Cue/Model View 연결 단계다.

따라서 이번 완료 범위는 `원본 근거 -> 실행 가능한 초기 배치 -> Tool 편집 가능`이다.
`원작과 동일한 최종 미감`과 `Dimension Summon animation cue 연결`은 수동 튜닝 및 다음
presentation 작업 경계다.

## 검증 결과

- Python extraction/importer: 63 tests PASS
- Effect pipeline: mesh-backed Particle 정상 publish와 Base override 실패 rollback PASS
- Effect Tool final audit: PASS (`code=50`, `documents=11`, `resources=4`, `palette=2662`, `cues=14`)
- ClientFrontendHarness Debug: PASS, `failures : 0`
  - 30/60/144 FPS deterministic playback
  - 잘못된 initial position 범위 rollback
  - DimensionMaster F Imported 문서 drawable 및 Mesh Particle 확인
- ProjectAudit: 69 checks PASS
- Client x64 Debug: PASS
- Client x64 Release: PASS
- Client Debug startup smoke: `Client/Default`에서 10초 생존, 즉시 abort 없음

Debug 최종 증분 빌드는 다른 세션에서 수정한 최신 `MapTool.cpp`를 실제로 다시 컴파일하고
Client.exe를 링크했다. Release 최종 증분 빌드도 같은 MapTool SHA-256이 빌드 전후 유지된
상태에서 통과했다. 따라서 현재 스냅샷의 Effect와 MapTool 통합 컴파일은 확인했다.

다만 F1에서 Imported 문서를 직접 선택한 GPU 화면 검수와 MapTool 창 열기, 저장, 재로드
같은 수동 기능 smoke는 이번 Effect 작업에서 실행하지 않았다. 다른 세션이 이후
MapTool.cpp를 다시 수정하면 그 새 상태는 다시 빌드해야 한다.

## 작업트리 상태

공유 작업트리에 다른 세션의 대규모 미커밋 변경이 있으므로 stage/commit하지 않았다.
이번 결과는 관련 파일만 추가·수정했으며 다른 담당 변경을 되돌리지 않았다.
