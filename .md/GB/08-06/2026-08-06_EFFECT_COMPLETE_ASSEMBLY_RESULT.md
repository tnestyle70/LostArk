# Effect Complete Assembly 구현 결과

## 0. 2026-08-06 Particle semantic/runtime data view 추가 결과

이번 변경은 Character pivot과 Light/Camera/Screen Post/Sound typed presentation을
포함하지 않는다. 사용자와 합의한 대로 다음 단계로 미뤘다.

완료한 실행 경계는 다음과 같다.

- UE `FRandomStream` LCG와 seeded module별 stream, vector 축별 random 소비 순서,
  locked axes를 실행한다.
- local/world particle root를 분리하고 world particle은 spawn root를 유지한다.
- bone/socket module은 현재 Character animation의 live anchor matrix를 소비한다.
- spawn event generator/receiver는 source event name/type/count로 연결된다.
- local vector field 4개를 `.wvectorfield`로 복원하고 24 occurrence에서 실제 volume을
  trilinear sample한다. 누락/손상 volume은 stage 전체를 rollback한다.
- source mesh particle size는 UE 단위에 프로젝트 `0.01`을 한 번만 적용한다.
  하네스의 `100/200/300 -> 1/2/3` world scale 검증이 통과했다.
- 원본 sphere direction의 positive/negative 축 random 선택 규칙을 복원했다.

데이터 및 Tool 연결 결과는 다음과 같다.

- Imported 11개, Authored 기본 스킬 11개, BA stage 포함 runtime Effect 15개를 생성했다.
- Assembly 15개, WFX Component 265개, emitter 1,758개의 compile identity가 일치한다.
- stale generated WFX 6개가 stable component ID를 중복시키던 문제를 수정했다.
  생성기는 같은 source Effect가 소유한 오래된 파일만 제거하며 다른 source 파일은
  건드리지 않는다.
- All Effects의 11개 DimensionMaster Authored 문서를 모두 drawable/playback stage했다.
- Runtime Catalog의 15개 compiled Assembly 문서를 모두 playback stage했다.
- Data Files에 `[Assembly]`와 `[WFX Component]` runtime row를 추가했다. Assembly 전체와
  Component 하나를 같은 world preview stage 경로로 audition할 수 있다.
- runtime row는 생성물을 직접 덮어쓰지 않는 audition copy다. 직접 Save는 거부하고
  Authored override가 필요할 때만 Save As를 사용한다.

자동 검증 증거:

- extractor/generator unit tests: 20/20 PASS
- Effect executor harness: 26/26 PASS
- Effect runtime/data-view harness: 3/3 PASS
- WFX existing compile verification: 15 Effects / 265 Components /
  1,758 emitters / 501 source Action cues, identity complete
- Debug Client build: errors 0
- Debug Client `Client/Default` startup smoke: 12초 생존 PASS
- 관련 파일 `git diff --check`: PASS

전체 ProjectAudit은 실행했으나 2개 legacy/global 계약 때문에 FAIL했다.

- `projects.data-source-visibility`: dirty worktree의 전체 Data 552개 중 project/filter 등록이
  225개라는 전역 등록 문제다. 이번 runtime stage 결과와는 별도이며 해결하지 않았다.
- `effect.g09-cross-document-contract`: audit가 아직 v8 Particle System/v7 Model Cue 문자열을
  요구한다. 현재 정본은 v10 source recipe/Assembly/WFX이므로 audit 계약 교정이 필요하다.

따라서 ProjectAudit PASS는 이번 결과의 완료 증거로 기록하지 않는다.

현재 global extraction receipt의 `runtimeExecutionComplete`는 여전히 `false`다.
이는 이번에 닫은 seeded/world-local/bone/event/vector/size 경계와 별개로 Material Instance,
procedural material, presentation cue, 그리고 아직 EXACT로 승격하지 않은 다른 module
class가 남아 있기 때문이다. 따라서 이번 결과는 "차원술사 전체 픽셀 복원 완료"가 아니라
"요청한 particle semantic 경계와 문서/Component runtime 연결 완료"로 판정한다.

실제 GPU 창에서 All Effects/Data Files 행을 클릭한 결과와 원작 PNG 픽셀 A/B는 자동
PASS로 기록하지 않았다. Client startup과 모든 playback stage는 통과했지만, 가시 화면의
크기·피벗·material 결과는 다음 수동 runtime 확인에서 판정한다.

## 1. 결론

추출 결과와 완성 Effect 사이의 연결을 기존 Effect runtime 안에서 닫았다. 새 runtime이나 별도
preview 객체 계층은 만들지 않았다.

- DimensionMaster F `2050500 / 업의 경계`
  - Cascade 97 Elements
  - `DimensionMaster_DimensionSummon.wmodel`
  - `sk_swp_dms_00_sk_sk_dimensionprison` 5.45833333초 Model Cue
- DimensionMaster T `2050510 / 일념`
  - Cascade 111 Elements
  - Summon Model Cue 없음
- All Effects의 스킬 행과 Data Files의 Authored 문서는 같은 `EFFECT_DOCUMENT_DESC`를 연다.
- All Effects의 스킬 행을 클릭하면 Authored 문서를 load하고 Complete Effect를 0초부터 다시 재생한다.
- Data Files는 파일 관점의 Load/New/Save/Save As/Reload/Discard 진입점이며 별도 Effect 의미를
  만들지 않는다.
- 실행 가능한 Imported 초안은 Data Files에서 먼저 검수한 뒤 `Promote to Authored Skill`로 같은
  skill `effectId`의 Authored 문서에 원자적으로 승격할 수 있다.

원작과 같은 밝기, 색, 크기, 피벗의 A/B 튜닝과 현재 문서가 표현하지 못하는 Light/Post Effect는
이번 범위에 포함하지 않았다. Unsupported emitter는 영수증에 보존했고 정상 Element로 가장하지 않았다.

## 2. 최종 사용자 흐름

```text
All Effects
  Dimension Master
    F | 업의 경계
      Model Cues
        dimension_summon
      Particle / Decal / ...
        Element

스킬 행 클릭
  -> Data/Effects/Authored의 같은 effectId 문서 load
  -> Complete Effect 즉시 restart
  -> F: Summon animation + 97 Elements
  -> T: 111 Elements

Element 클릭
  -> 같은 active Document 안의 Element 선택
  -> Mesh Shape / Base / Noise / Mask / Emissive / Dissolve
  -> Detail Apply/Revert
  -> Complete / Solo Selected / Mute Selected

Data Files
  -> 같은 문서를 파일 관점에서 load/save/reload
  -> Imported 실행 초안은 검수 후 Promote to Authored Skill
```

Resource Library는 Element 선택 여부와 무관하게 `Valtan / DimensionMaster / LanceMaster /
Artist / Warlord` domain을 먼저 선택하고, 그 안에서 `Meshes / Textures`, 폴더, 검색 조건으로 실제
WModel/DDS thumbnail을 탐색한다. 선택한 slot과 파일 종류가 맞을 때만 Bind가 활성화된다.
Particle도 `meshModel`을 가지면 Mesh Shape 카드를 표시한다.

## 3. 구현 결과

### 3.1 Effect authoring v7 Model Cue

`EFFECT_DOCUMENT_DESC`에 `modelCues`를 추가하고 authoring version을 7로 올렸다. v3~v6 문서는
빈 cue 목록으로 계속 읽는다.

각 cue는 stable cue ID, `Character/...wmodel` asset ID, 실제 animation clip 이름, 시작 시각,
재생 길이, Effect root 기준 local transform과 asset pre-transform을 소유한다. codec과 publisher는
version, ID, 중복, 안전한 Resources-relative path, 유한한 시간/transform, 양수 scale/duration,
최대 개수를 검증한다. 실패하면 기존 staged/committed 문서를 유지한다.

renderer는 기존 `CModel -> CMaterial`, `Shader_VtxAnimMeshBinary.hlsl`, bone matrix 경로로 cue 모델을
stage하고 Effect sample time에 맞춰 clip track을 seek/render한다. `CEffectPlayback`의 complete duration은
Element tail과 Model Cue end 중 큰 값을 사용한다. 별도 GameObject, shader 계열, catalog를 추가하지 않았다.

runtime catalog의 dependency 검증은 기존 `Effect/...` 리소스와 명시적인 Model Cue의
`Character/...wmodel`만 구분해 허용한다. Published 개념은 저작 UI에 다시 만들지 않았고 실제 소비자인
`CEffectCatalog` lookup은 유지했다.

### 3.2 F/T 복원 데이터

| 항목 | F 2050500 | T 2050510 |
|---|---:|---:|
| authoring version | 7 | 7 |
| emitter partition | 88 | 101 |
| 변환 emitter | 83 | 78 |
| unsupported emitter | 5 | 23 |
| 최종 Element | 97 | 111 |
| particle budget | 886 | 1,250 |
| 외부 Module 참조 | 658/658 | 753/753 |
| closure property error | 0 | 0 |
| runtime dependency | 143 | 110 |
| Model Cue | 1 | 0 |

F/T Imported와 canonical Authored 문서를 v7으로 생성했다. F Authored는 97 Elements와 근거가 확인된
Summon main clip 하나를 결합한다. 근거가 확정되지 않은 `_1` clip은 임의로 연결하지 않았다. T Authored는
111 Elements만 가지며 Summon을 섞지 않는다.

Imported Element가 원본 global timeline의 `startDelaySeconds`를 이미 소유하므로 F/T admitted animation
Effect cue는 0ms에서 한 번만 시작하도록 맞췄다. 실제 스킬 표현은 기존
`CEffectPresentationService -> CEffectObject -> Layer_Effect` 경로를 그대로 사용한다.

### 3.3 Tool UI와 승격

- Effect Tool 기본/최소 크기를 `620x760 / 560x680`으로 확대했다.
- All Effects 기본/최소 크기를 `620x560 / 540x500`으로 확대했다.
- All Effects skill click은 별도 Load 버튼 없이 complete load/restart한다.
- All Effects에 Model Cue branch와 clip/duration을 표시한다.
- Effect Tool 상단에 domain Resource Library를 항상 표시한다.
- Mesh와 Texture library를 분리하고 실제 WModel/DDS thumbnail을 사용한다.
- 선택 slot 종류와 library 파일 종류가 맞을 때만 Bind를 허용한다.
- Particle의 Mesh Shape 카드와 기존 5개 Material input을 함께 유지한다.
- Data Files active 상태에 Element/Model Cue 수를 표시한다.
- 실행 가능한 Imported 문서에 확인 modal이 있는 `Promote to Authored Skill`을 추가했다.
- 승격은 PlayerSkills에 존재하는 target ID만 허용하고 stage 검증, atomic replace, 실패 rollback을 수행한다.
- 기존 Authored에 Model Cue가 있고 Imported에 없다면 승격 시 cue를 보존한다.
- Solo Selected는 선택 Element만 남기므로 Model Cue를 제외하고, Complete/Mute는 Model Cue를 유지한다.

### 3.4 문서 전환, sequence, audition

- dirty active Document에서 다른 All Effects/Data Files 문서를 고르면 공통
  `Save & Load / Discard & Load / Cancel` modal을 연다.
- Save는 Detail draft를 먼저 Apply/Revert하도록 요구하고, Discard도 target parse/validate/stage가
  성공한 뒤에만 현재 Document를 교체한다. 실패하면 현재 Document와 preview를 보존한다.
- skillbinding의 body clip 목록 전체를 non-loop 순서로 재생하고 마지막 뒤 첫 clip로 돌아간다.
  target generation이 바뀌거나 Animation combo에서 수동 clip을 선택하면 sequence를 해제한다.
- `Audition Selected`는 선택 Element를 Solo로 전환하고 해당 `startDelaySeconds`로 seek한다. pending
  Detail draft는 preview 문서에만 합성한다.
- Detail Apply 성공은 `Applied to active Document memory; Save required to persist.`로 표시해 memory
  commit과 파일 Save를 구분한다.
- All Effects와 Data Files에 `Particle Layers / Mesh-backed / Budget`를 표시한다. F는
  `95 / 31 / 886`, T는 `110 / 60 / 1,250`이다.

### 3.5 저장 profiler capture 분석

`Client/Bin/ProfilerCaptures/profiler_20260806_125134_121_frame110.json`은
`LostArkProfilerCapture.v1` 정상 JSON이며 frame 73~110의 38 samples, dropped CPU/GPU 0이다.

| 항목 | 실측 |
|---|---:|
| CPU frame 평균 | 56.8949 ms / 17.58 FPS |
| CPU frame 중앙값 | 56.9080 ms |
| CPU frame 최소~최대 | 52.6110~62.8211 ms |
| 유효 GPU frame 평균 | 57.0849 ms / 17.52 FPS |
| `Client.Update` CPU 평균 | 4.0272 ms |
| `Client.Render` CPU 평균 | 52.8306 ms |
| `EffectTool.Render` CPU 평균 | 46.1566 ms |
| `EffectTool.ResourceGrid` CPU 평균 | 0.0670 ms |

기존 capture는 `EffectTool.Render` 내부 창별 비용을 나누지 않는다. 이번 변경에서
`AuthoringWindow / ModelViewWindow / DetailWindow / AllEffectsWindow / DataFilesWindow /
ThumbnailTrim` scope를 추가했다. 성능 코드를 추측으로 제거하지 않았으며, 정확한 46 ms 소유자는 새
빌드로 capture를 한 번 더 저장한 뒤 판단한다.

표준 Base/Noise/Mask/Emissive/Dissolve slot은 유지했다. Material Template가 가변 slot 근거를 제공할 수
있는 데이터 경계는 그대로 두었지만, 추출 근거 없는 custom shader나 임의 slot은 만들지 않았다.

## 4. 자동 검증 증거

### 4.1 데이터와 pipeline

- Python extraction/conversion 전체: 64 tests PASS
- Effect pipeline 전용 테스트: PASS
  - v7 Model Cue 정상 publish
  - v6 호환
  - version/path/kind/duplicate/resource/hash/budget/binding 검증
  - promote 거부와 replace 실패 rollback
- 실제 Effect publish: 11 Effects PASS
- F runtime catalog: version 7, 97 Elements, 1 Model Cue, 143 dependencies
- T runtime catalog: version 7, 111 Elements, 0 Model Cue, 110 dependencies
- 관련 JSON 7개와 Client project/filter XML parse: PASS
- UTF-8 직접 파싱: F `업의 경계`, T `일념` 정상

### 4.2 harness와 audit

- ClientFrontendHarness x64 Debug: `failures : 0`
- ClientFrontendHarness x64 Release: `failures : 0`
- F 97 Elements/95 Particle Layers/31 mesh-backed/Budget 886 계약: PASS
- T 111 Elements/110 Particle Layers/60 mesh-backed/Budget 1,250 계약: PASS
- DimensionMaster T `_01 -> _02` binding 순서 계약: PASS
- F Model Cue codec round-trip/dependency/duration: PASS
- duplicate Model Cue stage 실패 시 기존 상태 보존: PASS
- All Effects의 모든 PlayerSkills effectId와 Authored join: PASS
- Effect Tool final audit: PASS
  - `code=50`, `documents=11`, `resources=217`, `palette=2662`, `cues=14`
- 전체 ProjectAudit: 74 checks PASS

ProjectAudit의 Data 노출 검사는 이미 Git에 있던 SkillWindow JSON 두 개가 Client project/filter에 빠진 것을
검출했다. 해당 JSON 내용은 수정하지 않고 `96.DataFiles\UI`의 `None` 항목으로만 등록했다. Effect G09
검사는 v7 정본을 보도록 한 줄만 갱신했고 같은 파일의 Valtan 관련 병행 변경은 보존했다.

### 4.3 정본 build와 실행 생존

- `Invoke-BuildAndRegression.ps1 -Configuration Debug`: PASS
- `Invoke-BuildAndRegression.ps1 -Configuration Release`: PASS
- 두 구성 모두 Engine, Shared, Server, Client, NetworkProtocolHarness, ClientFrontendHarness,
  Server contract, Effect final audit, ProjectAudit를 통과했다.
- Debug Client: `Client/Default` working directory에서 10초 생존 PASS
- Release Client: `Client/Default` working directory에서 10초 생존 PASS

Debug 회귀 중 기존 NetworkProtocolHarness post-build에서 `pwsh.exe` 미탐색 문구가 한 번 출력됐지만
실행 harness와 전체 회귀는 exit code 0으로 완료됐다.

## 5. Particle authoring 해석과 남은 UI 경계

F의 `97 Elements`는 97개 독립 ParticleSystem이나 97개의 수동 튜닝 체크리스트가 아니다.

| 원본/변환 계층 | 수 |
|---|---:|
| source ParticleSystem group | 7 |
| converted emitter | 83 |
| 최종 runtime layer | 97 |
| 1 layer emitter | 73 |
| 2 layer emitter | 8 |
| 3 layer emitter | 1 |
| 5 layer emitter | 1 |

원본 Burst를 현재 deterministic playback 단위로 나누면서 97 layer가 됐다. 올바른 작업은 Imported
baseline을 complete로 먼저 재생한 뒤 문제가 보이는 emitter/layer만 audition하고 수정하는 것이다.
추출을 버리고 레퍼런스처럼 전부 수동 재제작할 근거는 없다.

다만 현재 All Effects가 Kind 아래 95 Particle layer를 평면으로 표시하는 것은 저작 탐색으로는 부족하다.
다음 구현 경계는 기존 `groupId/sourceNode` provenance를 이용한
`Source System -> Emitter -> Burst Layer` tree와 group audition/filter다. 서로 다른 source 값을 가진
layer에 같은 Detail 숫자를 무조건 덮는 bulk edit은 absolute/relative field 의미를 먼저 정한 뒤 추가한다.
이 항목은 이번 G22~G24 구현에 포함되지 않았으며 완료로 기록하지 않는다.

DimensionMaster basic attack은 재추출 누락이 아니다. 실제 `DimensionMaster_Character.wmodel`은 154
animations를 가지며 `pc_sp_m_00_sk_att_battle_1_01/_02/_03/_04` 네 section이 모두 존재한다.
`DimensionMaster.skillbindings.json`도 네 clip을 순서대로 소유한다. 한 clip만 보이던 현상은 기존
Effect Tool의 first-clip-only preview 경로였고 이번 sequence 구현은 전체 binding 목록을 소비한다.

## 6. 실행하지 않은 수동 검증

다음 항목은 GPU 화면과 실제 입력이 필요하므로 자동 PASS로 기록하지 않는다.

1. Debug Client F1에서 All Effects의 F/T 스킬 행 클릭 후 실제 GPU 화면 확인
2. F에서 Summon animation과 97 Elements가 같은 complete timeline으로 보이는지 확인
3. T에서 111 Elements가 재생되고 Summon이 섞이지 않는지 확인
4. Complete/Solo Selected/Mute Selected의 육안 비교
5. Resource Library thumbnail, slot binding/clear, Detail Apply/Revert의 실제 조작
6. Data Files Imported load, Promote 확인 modal, Save/Reload/Discard의 실제 조작
7. Server Arena에서 실제 F/T 스킬 입력 시 admitted animation cue와 world Effect 동시 재생 확인
8. 새 세부 scope가 포함된 profiler JSON을 다시 저장해 `EffectTool.Render` 46 ms의 창별 소유자 확인
9. All Effects LMB에서 basic attack `_01 -> _02 -> _03 -> _04` sequence 육안 확인

위 항목은 기능 계약의 미구현 목록이 아니라 최종 화면·입력 수동 smoke 목록이다. 원작과의 밝기·색·크기,
procedural material fallback, F의 Light 2개/Post Effect 3개, T의 unsupported 23개는 별도 시각 튜닝 및
runtime 표현 확장 경계로 남는다.

## 6.1 G25 Particle System 상위 튜닝 계층 완료

평면 95/110 Particle layer 위에 하나의 선택 가능한 `Particle System` 저작 계층을 추가했다. 원본
Element는 병합하거나 다시 쓰지 않았고 다음 document-level modifier만 v8 계약으로 추가했다.

| 필드 | 적용 범위 |
|---|---|
| `uniformScaleMultiplier` | 모든 Particle layer의 공간 배치와 표시 크기 |
| `yawOffsetDegrees` | 모든 Particle layer의 root 기준 Y 회전 |
| `directionYawDegrees` | deterministic sampling 뒤 초기 방출 속도 방향 |
| `initialSpeedMultiplier` | 초기 방출 속력 |

Decal, Summon Model Cue, Sprite/Mesh/Trail, material, Burst time, lifetime은 이 modifier의 영향을 받지
않는다. F/T Authored와 executable Imported는 v8 identity `1/0/0/1`로 승격했기 때문에 이번 변경만으로
기존 외형이 달라지지 않는다. v3~v7 문서는 load 시 identity Particle System으로 호환 승격한다.

All Effects에는 다음 부모가 표시된다.

```text
Particle System | Source Systems 7 | Emitters 83 | Layers 95 |
Mesh-backed 31 | Budget 886
└─ Layers (95)
   └─ 기존 개별 Element
```

부모를 선택하면 Effect Detail에서 전체 Scale, System Yaw, Emission Direction Yaw, Initial Speed를
live preview하고 `Apply Particle System / Revert Particle System / Audition Particle System`을 사용할 수
있다. Apply는 active Document memory만 바꾸고 Save가 JSON 영구 저장을 담당한다. unapplied system
draft는 기존 Element draft와 같은 Save/Load/selection dirty 보호를 받는다. 새 Effect를 로드했을 때
Particle layer가 있으면 첫 개별 Element 대신 이 부모가 기본 선택된다.

### G25 자동 검증

- extractor unit tests 3개: PASS, v8 identity 생성과 v7 promote 승격 포함
- Effect pipeline: PASS, v8 publish/범위 오류 rollback/v7·v6 호환 포함
- actual Effect publish: 11 Effects PASS
- Debug Client build: PASS
- Debug 정본 regression `-SkipBuild`: PASS
  - ClientFrontendHarness failures 0
  - F 97/T 111, F Summon cue, All Effects join PASS
  - Particle layout scale/yaw + emission direction/speed 수학 PASS
  - non-Particle 불변, invalid stage rollback, v7 identity 승격 PASS
  - Effect Tool final audit PASS, ProjectAudit 74 checks PASS
- Release Client와 Release ClientFrontendHarness 직접 build: PASS
- Release ClientFrontendHarness: failures 0
- Debug/Release Client 10초 startup smoke: PASS
- `git diff --check`: PASS

Release 전체 정본 회귀는 이번 Effect 변경이 아니라 기존 병행 Valtan dirty 변경에서
`BOSS_PATTERN_DEFINITION` 선언과 `GameplayCatalog.cpp`/`ValtanBrain.cpp` 소비 필드가 불일치해 Server
compile에서 중단됐다. 해당 Server 파일은 다른 작업 범위이므로 수정하지 않았다. Effect가 포함된 Release
Client와 Release harness는 별도로 성공했다.

### G25 남은 수동 화면 확인

F1에서 F/T를 선택한 뒤 다음 조작은 실제 GPU 화면과 마우스 입력이 필요해 자동 PASS로 기록하지 않았다.

1. Particle System 부모가 기본 선택되고 하위 `Layers`가 펼쳐지는지 확인
2. 네 slider의 live preview와 Apply/Revert/Save/Reload 확인
3. `Particles Only`와 `Audition Particle System`이 Summon/Decal을 숨기는지 확인
4. 하위 layer를 다시 선택해 기존 Element Solo/Mute/Detail이 유지되는지 확인
5. identity 상태에서 변경 전 F/T 화면과 같은지 육안 비교

PNG/영상 레퍼런스는 자동 복원 데이터를 다시 손으로 만드는 입력이 아니라 이 마지막 육안 차이 검증과
미지원 표현 보정에 사용한다. 기본 작업 순서는 `원본 변환 complete -> Particle System 전체 정렬 ->
차이 나는 layer만 정밀 보정 -> 레퍼런스 A/B`로 확정한다.

## 7. 병행 작업 보존

작업 시작부터 존재했거나 도중 추가된 Valtan balance/pattern/HUD, Server gameplay/world bootstrap,
Gameplay/World publisher 변경은 되돌리거나 정리하지 않았다. 공유 파일에서는 현재 내용을 기준으로 필요한
Effect 관련 줄만 좁게 수정했다. 전체 stage/commit은 수행하지 않았다.
