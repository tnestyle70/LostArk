# 차원술사 이펙트 추출 결과

## 결론

차원술사 1차 source-intake 범위는 원본 패키지 식별, 선택된 파티클 graph-class export와 tagged-property envelope 복구, core/context 직접 ImportTable 수집, Alt+V 카메라 패키지 식별까지 진행했다. 이것은 `현재 Effect Tool에서 1:1 재생 완료`, 완전한 재귀 dependency closure, Cascade 의미 디코딩 완료를 뜻하지 않는다.

차원술사의 내부 클래스 코드는 `SWP`이며 코어 패키지는 `FX_PC_SWP_00~05`다. emitter 누락을 교정한 뒤 여섯 패키지에서 38,104개 graph-class export, 6,046개 `ParticleSpriteEmitter` 참조, 459개 `ParticleSystem` 후보를 읽었다. 모든 emitter 참조 target이 graph에 존재하고 tagged-property stream 종단 파싱 오류는 0개다. 다만 459개에는 old/test/camera/helper가 섞였으므로 제품 effect 수나 스킬 수가 아니다.

## 2026-08-03 런타임 폴더 정리와 admission 갱신

원본 UPK와 분석 보고서는 외부 `Resource_LostArk`에 유지하고, 런타임이 직접 읽는
선별 결과만 다음 위치에 정리했다.

- `Client/Bin/Resources/Effect/Dimensionist/Textures`: 693 DDS
- `Client/Bin/Resources/Effect/Dimensionist/Meshes`: 139 `.wmodel`
- 합계 832파일, 90,608,380 bytes, UModel/cook 실패 0
- `Data/Effects/Authored/Dimensionist/Candidates`: 459 `.effect`
- `Data/Effects/SourceCatalog/dimensionist_candidates.json`: 459개 선택 catalog
- `Data/Effects/SourceCatalog/dimensionist_admission.json`: 재생 허용 여부와 누락 정본

이 459개는 `provenance=DERIVED_CONVERSION`인 Effect Tool 후보이며 원본 1:1 effect가
아니다. 기존 recipe 추출기가 LOD의 음수 package ref를 조용히 버리던 문제를 수정해
외부 module 참조를 명시적으로 기록했다. 전체 459개 중 403개가 65,626개의 외부
Cascade module ref에 영향을 받는다. LOD0 기준 4,885 emitter는 dependency module을
materialize하기 전까지 비활성화했고, LOD payload 자체가 없는 158 emitter도 비활성화했다.
따라서 6,046 emitter 중 현재 활성 candidate는 1,003개다. 누락 module을 감추기 위해
Spawn/Lifetime을 발명하지 않으며, 현재 변환에서 발명된 값은 각각 0개/3개다.

material은 1,050개 직접 참조 중 17개가 미해결이며 91개 ParticleSystem, 236개
emitter/LOD occurrence에 영향을 준다. 5,888 REQUIRED module 중 4,372개만 runtime
texture가 연결됐다. 이 수치는 resource export 성공과 effect admission을 분리한다.
즉 693 texture와 139 mesh의 현재 로컬-export closure 추출은 완료됐지만, 외부 Cascade
module 65,626개를 dependency package에서 materialize하고 미지원 module을 구현하기
전까지 전체 차원술사를 제품 재생 완료로 판정하지 않는다.

## 스킬·연출 패키지 구성

| 패키지 | ParticleSystem | 확인된 대표 계열 |
|---|---:|---|
| `FX_PC_SWP_00` | 73 | Dimension Thrust, Nail Strike, Other Timeline, Time Collapse, Willow Rend, 기본 공격 |
| `FX_PC_SWP_01` | 65 | Dimension Gate/Move/Prison, Flicker Thrust, Overslash |
| `FX_PC_SWP_02` | 68 | Dimension Break, Momentary Rift, Riposte, Time Wave |
| `FX_PC_SWP_03` | 69 | Chrono, Fold Cut, Instance 계열 카메라·배경·시간 정지 연출 |
| `FX_PC_SWP_04` | 129 | Gravity Point(`gp`), Tearing Sword(`ts`), Time Wave/초월 연출(`tw`), Void Trigger(`vt`) |
| `FX_PC_SWP_05` | 55 | Dimension Detonate, weak-point hit, Phantom Thrust, Rift Slash, Void Curse, Willow Rend |

피격 이펙트는 실제 에셋으로 존재한다. 예를 들어 `dimensionthrust_impact`, `willowrend_swinghit`, `dimensionmaster_weakpoint_hit`, `vt_exp_hit`가 독립 ParticleSystem으로 확인됐다.

노이즈도 전부 하나의 셰이더 효과로 뭉쳐 있지 않다. `rgbnoise`, `rgbsplit`, `spherergb`, `zoomblur`, camera glass/transition/background가 ParticleSystem과 그 Material/Texture 참조로 존재한다. 다만 화면 전체 RGB split, zoom blur, desaturation, 장면 전환 합성은 파티클 메시만으로 동일하게 재현할 수 없으므로 renderer post-process pass가 필요하다.

## Alt+V 초월기 컷씬

정확한 후보 패키지는 `STANDARD_SKILLCAM_DIMENSIONMASTER`다. 직접 해석한 ExportTable에는 다음이 들어 있다.

- `CameraActor` 15개
- `EFSeqAct_Matinee` 3개
- `EFInterpTrackUltimateSkillCinematicControl` 3개
- `InterpData` 3개
- Director/Move/Anim/Event/Float 트랙
- 카메라 타깃 전환과 캐릭터 애니메이션 참조
- RemoteEvent `skillcam_dimensionmaster_01/02`에서 Attach, SetCameraTarget, Matinee로 이어지는 연결
- 길이 3.7065/3.70/3.75초, 활성 Move track의 transform key와 Director target `cm01`
- AnimControl `sk_super_instance`, `sk_super_timewave`
- 이 패키지의 Texture/Movie/Media/Bink 계열 import 0개

따라서 Alt+V 핵심 연출은 PNG 프레임 시퀀스를 RenderTarget에 재생한 것이 아니라
캐릭터/이펙트를 실시간 렌더링하면서 Matinee가 카메라, cut, 애니메이션을 약 3.7초 동안
제어하는 3D 시퀀스라고 판정한다. RenderTarget은 이 실시간 장면의 후처리나 합성에 쓸 수
있고 외부 ParticleSystem/material overlay도 공존할 수 있으므로, 전체 연출에 이미지 overlay가
전혀 없다고 확대 해석하지는 않는다. 현재 추출은 원본 timeline 증거와 주요 key 해석까지이며
Client용 cinematic sequencer 데이터로 변환·재생한 상태는 아니다.

## 외부 소스 팩

외부 분석 소스 묶음:

`C:/Users/user/Desktop/Resource_LostArk/00_SourcePackages/Effect_DIMENSIONMASTER_20260803_v3`

구성:

- 코어 6개: `FX_PC_SWP_00~05`
- 문맥 4개: `STANDARD_SKILLCAM_DIMENSIONMASTER`, `PC_DIMENSIONMASTER_M`, `FX_J_W_01`, `FX_K_W_01`
- 중복 제거 직접 의존성 611개
- 총 621개 패키지, 778,332,058 bytes
- machine report 19개를 팩 내부 `Reports`에 포함
- 각 파일의 logical/physical name, actual size, inventory size, SHA-256를 `source_pack_manifest.json`에 기록
- payload aggregate SHA-256: `8fefaef0842d0c74105344eaf95a2aab3e36614d7ab0781702ccf6dfc2dcedac`
- manifest 자체 SHA-256 sidecar 포함

그래프와 분석 보고서:

`C:/Users/user/Desktop/Resource_LostArk/05_Reports/EffectExtraction/DIMENSIONMASTER`

- `particle_graphs`: 코어 여섯 패키지의 파티클 그래프 JSON
- `dependencies`: Import/ExportTable 및 class count JSON
- `dimensionmaster_fx_name_matches.json`: 전체 FX 3,058개 패키지 NameTable 검색 결과
- `dimensionmaster_particle_source_manifest_v2.json`: 72,910개 tagged-property object reference와 10개 core/context ImportTable report를 합친 직접 의존성 manifest

2026-07-28 인벤토리 이후 게임 패키지가 갱신돼 현재 파일 크기와 다른 항목이 411개였다. 오래된 byte size를 정답으로 강제하지 않고 현재 원본을 다시 SHA-256해 actual size와 이전 inventory size를 모두 영수증에 남겼다. 다만 logical-to-physical 매핑 자체의 최신 정본성은 현재 inventory 재생성 전까지 별도 한계다.

직접 의존성 이름 624개 중 619개가 인벤토리에서 해소됐다. 미해소 항목은 `engine_materialfunctions03`, `enginematerials`, `fx_mn_cnbg_00_x`, `fx_mn_lpda_01_t`, `fx_mn_lpda_02_t`다. 마지막 3개는 실제 Dimension Prison/Instance/Tearing Sword/Void Trigger/Dimension Detonate graph 참조에 걸리므로 무시할 수 없다. 해당 effect는 dependency가 해소될 때까지 admission 금지 대상이다. 이 수치는 direct intake이며 dependencies 자체의 재귀 ImportTable fixed point를 증명한 것은 아니다.

169,710개 tagged property 중 27,392개(16.14%)는 아직 raw payload다. 대부분 `RawDistributionFloat` 15,160개와 `RawDistributionVector` 11,242개다. `propertyError=0`은 property stream을 끝까지 읽었다는 뜻이며 이 curve/distribution의 의미 복원이 끝났다는 뜻은 아니다.

## 현재 Effect Tool 수용 경계

바로 변환 경로를 설계할 수 있는 범위:

- Sprite, Mesh, Ribbon 계열
- SubUV
- base/opacity/dissolve/distortion texture
- emissive scalar와 HDR/bloom 출력
- dynamic parameter 기반 emissive/dissolve/distortion

추가 구현이 필요한 원본 모듈:

- UE3 Decal
- local VectorField 및 회전/회전율
- CameraOffset
- 원본 Material expression을 현재 Effect Material schema로 낮추는 변환 규칙
- emissive 전용 texture, radial UV, afterimage
- fullscreen RGB split/zoom blur/desaturation/transition pass
- Alt+V Matinee/ultimate cinematic sequencer

원본 그래프에는 `ParticleModuleTypeDataMesh` 1,390개, camera offset 181개, local vector field 31개 등 현재 툴 스키마 밖의 노드가 실제로 존재한다. 따라서 자동 변환기는 지원 노드를 stage하고 미지원 노드가 있으면 조용히 버리지 말고 effect별 admission report를 내야 한다.

## 검증

- 전체 FX 패키지 3,058개 NameTable 스캔: 오류 0, 차원술사 관련 61개 패키지 탐지
- 코어 6개 dependency table 해석 완료
- graph-class export 38,104개, ParticleSpriteEmitter 6,046개, ParticleSystem 후보 459개, property stream error 0
- ParticleSystem emitter 참조 6,046개 target 존재 검증, 누락 0
- 직접 dependency 624개 중 619개 해소
- 소스 팩 621개 파일과 report 19개를 원본/복사본 SHA-256 및 aggregate hash로 검증
- Python 추출 도구 compile 및 회귀 테스트 대상 추가

## 다음 구현 단위

1. `Dimension Thrust impact` 하나를 파일럿으로 골라 `ParticleSystem -> Emitter -> LOD -> Module -> Material/Mesh/Texture` effect별 reachability를 증명
2. raw distribution/curve를 의미 디코딩하고 unresolved dependency가 있으면 admission 실패
3. 지원 UE3 module을 Effect schema v5로 변환하는 importer와 unsupported module report 구현
4. 459개 후보를 product/old/test/camera/helper로 분류하고 실제 호출자와 stable ID 연결
5. 작은 대표 세트로 순차 확인: 기본 hit -> Dimension Thrust -> Gravity Point -> Time Wave/초월기
6. render-target inspector에서 SceneHDR/Bloom/Distortion 결과 검증
7. Alt+V Matinee key extractor와 fullscreen post-process를 별도 수직 슬라이스로 구현

## Effect 저장 형식 계약

Effect Tool의 편집 정본은 JSON이나 `.wfx`가 아니라 UTF-8 텍스트 `.effect`다. 첫 줄은
`LOSTARK_EFFECT 5`이며 emitter와 module 값을 사람이 diff 가능한 형태로 저장한다.
`Data/Effects/Authored/Dimensionist/Candidates`의 459개 후보가 이 형식이므로 Effect Tool의
`Authored` 목록에서 바로 열 수 있다.

런타임 조리 형식은 `.weffect`다. 16-byte header에 magic, schema version, payload size,
payload hash를 기록한 뒤 같은 effect payload를 직렬화한다. `.wfx` 경로는 만들지 않는다.
JSON은 source catalog, admission, provenance 같은 목록·검증 데이터에만 사용한다. 현재
459개 후보는 미지원 Cascade module이 남은 파생 후보이므로 일괄 `.weffect` 제품 배포하지
않고, Effect Tool에서 검토·수정한 stable asset만 Cook한다. `.weffect`의 Save/Load와
round-trip은 구현됐지만 스킬 stable ID에서 cooked effect를 생성하는 제품 runtime
consumer는 아직 연결하지 않았다.

대표 후보 `par-j-swp-dimensionthrust-impact-01-01.effect`를 Debug Client의 headless
round-trip으로 검증했다. `Load -> Save -> Load -> Cook .weffect -> Binary Load` 결과가
동일한 1,742-byte authoring payload를 복원했고 결과는 PASS였다. 활성 후보
`old-par-m-swp-vt-exp-pierce-01.effect`도 headless simulator에서 초기화 가능한 emitter를
재생해 peak alive 94 이상과 HDR color 값 보존을 확인했다. 이는 직렬화와 CPU particle
simulation 검증이며 최종 화면 품질 검증을 대신하지 않는다.

## Dimensionist 캐릭터·초월기 애니메이션

차원술사 애니메이션은 원본 ActorX `PSK + PSA`를 별도 런타임으로 만들지 않고 다음 기존
경로로 조리했다.

```text
PSK + PSA
-> Blender ActorX import / all-actions FBX export
-> ModelAssetConverter
-> CModel -> CMaterial WModel
```

원본 `PC_SWP_M_00`에는 차원술사 PSA가 있지만 함께 들어 있던 `pc_swp_fx01_sk`는 26-bone
FX rig라 캐릭터 본체 mesh로 쓸 수 없었다. 실제 222-bone Specialist 남성 본체
`PC_SP_M_00/pc_sp_m_00_sk`와 차원술사 PSA의 bone 집합을 대조한 뒤 결합했다. 이 구분을
하지 않은 것이 이전 추출에서 캐릭터 본체가 빠질 수 있었던 핵심 함정이다.

런타임 결과는 `Client/Bin/Resources/Character/Dimensionist`에 정리했다.

| 파일 | ActorX bones | cooked bones/meshes | animation |
|---|---:|---:|---:|
| `Dimensionist_Character.wmodel` | 222 | 225 / 10 | 154 |
| `Dimensionist_DimensionCore.wmodel` | 26 | 29 / 2 | 1 |
| `Dimensionist_DimensionSummon.wmodel` | 20 | 23 / 4 | 2 |

본체 154개에는 Alt+V 연출에서 참조하는 `sk_super_instance`, `sk_super_timewave`가 모두
포함된다. 코어는 `sk_super_instance`, 소환체는 `sk_dimensionprison`과
`sk_dimensionprison_1`을 포함한다. 3개 WModel과 71개 texture의 총 크기는
146,400,423 bytes다. 자동 texture 이름 추측은 사용하지 않고 diffuse, normal, emissive,
ORM을 material별로 명시 remap했다.

재현 도구는 `Tools/CharacterAnimationIntake/build_actorx_fbx.py`다. Blender에서 PSK/PSA
bone subset을 먼저 검사하고 모든 PSA sequence가 Blender action이 됐을 때만 FBX와 JSON
report를 만든다. 중간 ActorX/FBX/Blend/report는 외부
`Resource_LostArk/01_Extracted/Character/Dimensionist`에 보존했다.

Debug Animation Tool에는 `Scene Character`, `Dimensionist Character`, `Dimension Core`,
`Dimension Summon` 선택을 추가했다. Character Select 또는 Development 진입 때 존재하는
WModel만 optional prototype으로 등록하며, 선택하면 기존 `CPart_Body`와 anim shader로
`Layer_AnimationPreview`에 배치한다. 즉 별도 animation runtime이나 플레이 가능한 class
ID를 만들지 않는다. 리소스가 없으면 레벨 로드는 계속되고 선택 시 admission 오류를
보여 주며, 파일이 존재하지만 decode가 실패하면 해당 레벨 로드를 실패시킨다.
Tool은 preview를 weak reference로만 기억하므로 F1 창을 닫은 채 레벨을 전환해도 이전
레벨의 Body/Model 수명을 붙잡지 않는다.

사용 순서는 Debug Client 실행, Lobby에서 Character Select 또는 Development 진입,
`F1 -> Animation Tool`, Target에서 Dimensionist 항목 선택이다. 모델은 scene character
기준 오른쪽 2.5 m에 나타나고 기존 clip list와 playback UI가 선택한 CModel을 직접
제어한다. Animation Tool 버튼만 Character Select와 Development에서 활성화하고,
Map/Effect/HUD authoring은 기존대로 Development 전용으로 유지했다.

이 preview는 애니메이션·skeleton 확인 경로다. `CPart_Body`는 diffuse/normal/bone을
opaque pass로 그리므로 코어·소환체의 emissive, ORM, transparency를 원본과 같은 최종
재질로 증명하지 않는다. 해당 품질은 Effect Runtime/전용 재질 연결 후 수동 smoke로
판정해야 한다.

## ResourcePack 2026.08.03.4 처리

사용자 결정에 따라 `2026.08.03.4` immutable copy, ZIP, ZIP SHA-256,
Hydrate/Verify 인계 작업은 폐기했다. 해당 버전의 manifest를 정본으로 갱신하거나 새
snapshot을 만들지 않았다. 이미 Git에 들어가 있던 `.4` manifest는 제거하고
`Data/AssetPacks.lock.json`은 직전 `.3`으로 되돌렸다. 원본 `Client/Bin/Resources`를
교체·삭제하지도 않았다. 따라서
현재 Resources inventory가 기존 `Data/AssetPacks.lock.json`과 다른 것은 의도된 로컬
작업 상태이며 ProjectAudit의 `asset-lock.inventory`는 이 결정이 유지되는 동안 PASS로
기록할 수 없다. clean `.3` Hydrate 환경에는 이번 두 맵과 Dimensionist 리소스가 없으므로
이 로컬 payload 없이 동일 장면을 실행할 수 없다. 새 immutable pack 배포 완료로 표현하지
않는다.

## 이번 통합 검증

- `ModelAssetConverter info`: 본체 157 sections/154 animations/skeleton yes, 코어
  4/1/yes, 소환체 5/2/yes
- 본체의 `sk_super_instance`, `sk_super_timewave` 및 코어·소환체 clip 이름 확인
- Debug/Release x64 Client 전체 빌드 및 링크 PASS
- `.effect -> .weffect -> .effect` headless round-trip PASS
- 활성 후보 headless particle simulation PASS
- ProjectAudit의 map, project XML/Data visibility, gameplay/navigation 검증 PASS
- 새 두 맵의 runtime directory/manifest/asset ID/model 집합 누락 검출 PASS
- ProjectAudit 전체 결과는 폐기 결정에 따른 `asset-lock.inventory` 한 항목만 FAIL

창을 실제로 열어 모델 외형, 카메라 프레이밍, 모든 157개 clip의 시각 결과를 순회하는
수동 smoke는 남아 있다. 자동 검증을 화면 품질 확인으로 확대 기록하지 않는다.
