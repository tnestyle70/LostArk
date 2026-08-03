# 차원술사 이펙트 추출 결과

## 결론

차원술사 1차 source-intake 범위는 원본 패키지 식별, 선택된 파티클 graph-class export와 tagged-property envelope 복구, core/context 직접 ImportTable 수집, Alt+V 카메라 패키지 식별까지 진행했다. 이것은 `현재 Effect Tool에서 1:1 재생 완료`, 완전한 재귀 dependency closure, Cascade 의미 디코딩 완료를 뜻하지 않는다.

차원술사의 내부 클래스 코드는 `SWP`이며 코어 패키지는 `FX_PC_SWP_00~05`다. emitter 누락을 교정한 뒤 여섯 패키지에서 38,104개 graph-class export, 6,046개 `ParticleSpriteEmitter` 참조, 459개 `ParticleSystem` 후보를 읽었다. 모든 emitter 참조 target이 graph에 존재하고 tagged-property stream 종단 파싱 오류는 0개다. 다만 459개에는 old/test/camera/helper가 섞였으므로 제품 effect 수나 스킬 수가 아니다.

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

따라서 Alt+V 연출이 단일 카메라 애니메이션이나 단일 파티클은 아니라는 근거는 충분하다. 그러나 현재 extractor는 카메라 transform/FOV/duration/cut/move/anim/event key의 serial payload를 의미 디코딩하지 않는다. 정확한 패키지와 class inventory를 발견한 상태이며 Alt+V timeline을 추출하거나 재생 데이터로 변환한 상태는 아니다.

## 외부 소스 팩

불변 소스 팩:

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

현재 `Effect_Tool.cpp`, `MainApp`, Level 전환 및 프로젝트 파일에 다른 작업의 미검증 변경이 겹쳐 있으므로 이번 추출 변경에서는 해당 런타임 파일을 수정하지 않았다.
