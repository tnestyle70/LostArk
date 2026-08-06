# Effect Tool 기능 완성 계획

## 현재 실제 반영 상태와 이번 경계

현재 working tree에는 G07~G09의 `Effect Document -> Playback -> Renderer ->
CEffectObject -> CEffectCatalog -> Character presentation` 수직 경로가 있다. DDS
thumbnail, Detail Apply/Revert, 원자적 authoring save/load, Model View와 차원술사
11개 runtime wiring도 이미 존재한다.

이번 변경은 `.md/GB/이펙트툴.md` 3장, 12장, 13.1과 사용자가 다시 전달한
All Effects/Element/resource-slot 계약을 기준으로 다음 기능 차이를 닫는다.

- 저작 UI의 Published 구역과 중복 Load 명령, G단계 문구를 제거한다.
- `PlayerSkills.json + Data/Effects/Authored`를 stage해 class -> input slot/완성
  skill -> Kind -> Element 트리를 만든다.
- 선택 Element의 Mesh Shape와 Material Input 카드를 분리하고 DDS와 WModel을
  실제 thumbnail로 표시한다.
- Element 선택 변경과 구조 변경에서 미적용 Detail draft를 보존한다.
- 완성 Effect, 선택 Element Solo, 선택 Element Mute preview를 같은
  `CEffectObject` stage 경로로 재생한다.
- Data Files를 New/Save/Save As/Reload/Discard 명령으로 정리하고 Imported
  draft가 도착하면 별도 runtime onboarding 없이 목록과 load source로 표시한다.
- 표준 다섯 Material input은 유지하면서 authoring v6에 Material Template ID와
  stable slot ID를 저장한다. 현재 등록 Template은 `effect.standard` 하나뿐이며
  근거 없는 custom slot과 shader는 허용하지 않는다.

렌더링 품질 고도화와 원작 시각 튜닝, 원본 package 추출은 이번 범위가 아니다.
런타임 `CEffectCatalog` lookup과 기존 renderer/playback/object는 유지한다.

## 수정·추가 파일과 위치

| 구분 | 절대 경로 | 역할 |
|---|---|---|
| 추가 | `C:/Users/user/.codex/worktrees/543f/LostArk/Client/Public/Effect_MaterialTemplate.h` | 표준 Template의 stable slot/semantic/display/HLSL/resource-kind 정본 |
| 수정 | `C:/Users/user/.codex/worktrees/543f/LostArk/Client/Public/Effect_AuthoringDocument.h` | v6 Element metadata, visible, Template ID, stable binding slot ID |
| 수정 | `C:/Users/user/.codex/worktrees/543f/LostArk/Client/Public/Effect_DocumentCodec.h` | v5 호환 load와 v6 save/validation 계약 |
| 수정 | `C:/Users/user/.codex/worktrees/543f/LostArk/Client/Private/Effect_DocumentCodec.cpp` | v5 -> v6 default stage, v6 parse/serialize, Template/slot 검증 |
| 수정 | `C:/Users/user/.codex/worktrees/543f/LostArk/Client/Public/Effect_Tool.h` | All Effects index, Data Files source, preview filter, stable selected slot 상태 |
| 수정 | `C:/Users/user/.codex/worktrees/543f/LostArk/Client/Private/Effect_Tool.cpp` | 최종 ImGui 작업 흐름과 명령/rollback 구현 |
| 수정 | `C:/Users/user/.codex/worktrees/543f/LostArk/Client/Public/Effect_ThumbnailCache.h` | Texture/Model thumbnail 요청 계약 |
| 수정 | `C:/Users/user/.codex/worktrees/543f/LostArk/Client/Private/Effect_ThumbnailCache.cpp` | 기존 CModel과 Mesh Preview shader를 사용한 bounded WModel offscreen thumbnail |
| 수정 | `C:/Users/user/.codex/worktrees/543f/LostArk/Client/Private/Effect_DocumentRenderer.cpp` | stable slot ID를 표준 runtime semantic으로 해소 |
| 수정 | `C:/Users/user/.codex/worktrees/543f/LostArk/Client/Private/Effect_Playback.cpp` | visible=false Element의 simulation/draw packet 격리 |
| 수정 | `C:/Users/user/.codex/worktrees/543f/LostArk/Tools/EffectPipeline/Publish-Effects.ps1` | v5/v6 authoring, Template/slot/metadata, drawable gate와 rollback 검증 |
| 수정 | `C:/Users/user/.codex/worktrees/543f/LostArk/Tools/EffectPipeline/Test-EffectPipeline.ps1` | v6 정상/잘못된 Template·slot과 기존 rollback 회귀 |
| 수정 | `C:/Users/user/.codex/worktrees/543f/LostArk/Tools/ClientFrontendHarness/Private/ClientFrontendHarness.cpp` | v6 metadata/Template/stable slot atomic save-reload |
| 수정 | `C:/Users/user/.codex/worktrees/543f/LostArk/Tools/ProjectAudit/Test-EffectToolFinal.ps1` | Published UI 부재와 최종 트리/thumbnail/Data Files 계약 검사 |
| 수정 | `C:/Users/user/.codex/worktrees/543f/LostArk/Client/Default/Client.vcxproj` | 새 data-only header 등록 |
| 수정 | `C:/Users/user/.codex/worktrees/543f/LostArk/Client/Default/Client.vcxproj.filters` | 기존 Effect header filter에 등록 |

## Authoring v6 저장 계약

v5 문서는 parse 후 아래 default를 local stage에 채우고 메모리에서는 v6로 다룬다.
기존 파일은 열기만 해서는 바꾸지 않으며 다음 Save에서 v6가 된다.

```json
{
  "id": "q_impact_particle",
  "displayName": "Impact Particle",
  "groupId": "impact",
  "sourceNode": "",
  "visible": true,
  "kind": "particle",
  "resources": [
    { "slotId": "base", "assetId": "Effect/.../base.dds" }
  ],
  "material": {
    "templateId": "effect.standard",
    "renderProfile": "additive_two_sided_depth_read"
  }
}
```

`effect.standard`는 Base/Noise/Mask/Emissive/Dissolve 다섯 input을 정의한다.
각 input은 stable slot ID, semantic role, 표시 이름, 현재 HLSL binding 이름,
허용 resource kind와 현재 runtime semantic을 가진다. Mesh Shape의 `meshModel`은
Material input이 아니라 별도 geometry slot이다. 등록되지 않은 Template/slot은 load,
save, publish에서 거부한다.

## All Effects와 선택 Element 흐름

```text
CPlayerSkillCatalog::Load
-> effectId가 있는 PlayerSkills row 수집
-> Data/Effects/Authored/<effectId>.effect.json parse/validate
-> 모든 문서가 성공하면 index commit
-> class combo와 search로 view filter
-> input slot/완성 skill -> kind -> optional group -> element 표시
-> skill row 선택은 같은 authored document를 load
-> element row 선택은 Detail dirty가 없을 때만 active selection commit
```

한 파일이라도 version, ID, 중복, path 검증에 실패하면 기존 All Effects index와
현재 Document/Preview를 유지한다. `CEffectCatalog`는 이 저작 index와 별개로 runtime
consumer를 위해 남긴다.

## Preview와 Data Files 명령

Solo/Mute는 active Document를 수정하지 않는다. 같은 Document copy에서 Element를
filter한 뒤 기존 `CEffectObject::Stage_Document`에 넣는다. stage 실패 시 이전 preview를
유지한다. visible=false Element도 runtime playback에서 emit/draw하지 않는다.

Data Files 명령은 다음 하나의 source 상태를 사용한다.

```text
New      새 v6 memory Document를 만들고 DIRTY로 표시
Save     현재 asset ID로 Data/Effects/Authored에 원자 저장
Save As  새 stable asset ID의 미존재 경로에 저장하고 active identity commit
Reload   현재 Authored/Imported source를 parse -> validate -> preview stage -> commit
Discard  확인 뒤 memory Document와 preview만 해제
```

Imported 문서는 `Data/Effects/Imported`가 실제로 존재할 때만 목록에 표시한다. load한
Imported draft의 Save/Save As 대상은 Authored이며 Warlord class/runtime 계약은 추가하지 않는다.

## 적용 순서와 검증

1. v6 data-only header와 codec 호환 load/save를 반영한다.
2. renderer/playback을 stable slot/visible 계약에 연결한다.
3. Model thumbnail cache를 기존 CModel/Shader 경로로 구현한다.
4. All Effects, Selected Element/Resource Browser, Preview, Data Files UI를 교체한다.
5. publisher, harness, project/filter와 RESULT를 동기화한다.
6. `ClientFrontendHarness`, Effect pipeline validate/publish/rollback,
   `Test-EffectToolFinal.ps1`, Client x64 Debug와 ProjectAudit를 실행한다.
7. `Client/Default`에서 가능한 범위의 F1 save/reload와 실제 world Effect smoke를
   수행하고, 수행하지 못한 육안 항목은 RESULT에 PASS로 기록하지 않는다.

## 2026-08-06 실제 화면 실측 후속 수정

실제 F1 화면에서 All Effects tree가 112 px로 고정되어 목록이 지나치게 작고,
`Load Selected`가 drawable 문서를 stage한 뒤에도 0초 정지와 world 원점 pivot을
유지하는 문제가 확인됐다. 또한 공유 Model View에 남아 있던 Dimension Core의
`sk_super_instance` clip이 새 DimensionMaster 스킬 Effect를 로드한 뒤에도 유지됐다.

후속 수정은 다음 실행 계약으로 닫는다.

1. All Effects 창은 최소 높이를 보장하고 tree child가 상태 문구를 제외한 남은 높이를
   사용한다. 112 px 고정값은 제거한다.
2. `Load Selected`와 All Effects의 완성 스킬 로드는 기존 Authored 문서를 여는 명령이다.
   별도 `CreateEffect`를 요구하지 않으며 로드 즉시 Complete preview를 0초부터 loop 재생한다.
3. drawable 문서는 현재 animation target의 player root를 기본 pivot으로 사용한다.
   root가 없으면 숨김 이유를 유지하고 임의 world 원점 성공으로 처리하지 않는다.
4. Effect의 `PlayerSkills.json` 소유 skill을 찾아 playable class target으로 전환하고,
   `<Class>.skillbindings.json`의 첫 clip을 재생한다. 남아 있던 Dimension Core/Summon
   preview-only target은 해당 playable class body로 교체한다.
5. Timeline은 PLAYING/PAUSED 상태를 명시하고 `Restart + Play`를 제공한다.
6. source audit, Client Debug build, Effect harness, 정본 Debug/Release 회귀를 다시 실행한다.

## 2026-08-06 수동 검증 후속: FPS, All Effects 명령, Live Detail 튜닝

실제 F1 화면에서 frame pacing 저하를 계측할 숫자가 즉시 보이지 않았고, All Effects의
완성 스킬 행이 현재 active 문서와 같을 때는 클릭해도 Complete Effect 재생을 다시 시작하지
않아 탐색 전용 목록처럼 보였다. Effect Detail도 숫자 입력 뒤 Apply 전까지 world preview에
반영되지 않아 연속 튜닝 흐름이 끊겼다.

### G10. Developer Tools FPS 계측

- F1 `LostArk Developer Tools`의 Diagnostics에 profiler 활성화와 무관한 ImGui frame rate와
  frame time을 항상 표시한다.
- 기존 Profiler 체크박스와 CPU/GPU overlay는 상세 진단 경로로 유지한다.

검증: Debug Client compile과 Effect final audit에서 FPS/frame-time widget 존재를 확인한다.

### G11. All Effects 완성 스킬 선택 계약

- All Effects 스킬 행은 active 여부를 선택 강조로 표시한다.
- 다른 스킬 행은 같은 Authored load 경로로 전체 Document를 열고 즉시 Complete Effect를
  재생한다.
- 이미 active인 스킬 행도 현재 committed Document를 Complete 모드로 restage하고 0초부터
  재생한다. Element 행은 파츠 편집 선택만 바꾸며 기본 Complete preview를 유지한다.
- All Effects는 여러 Effect Document를 중첩하는 composer가 아니다. 한 스킬의 Mesh,
  Sprite, Particle, Decal, Trail은 동일한 Effect Document의 여러 Element다.

검증: 같은/다른 skill 행 선택, Detail dirty 보호, complete filter와 restart 호출을 harness로
검사한다.

### G12. Effect Detail drag와 live preview

- Transform, Velocity, Color, UV, Timing, Lerp와 Type별 연속 수치를 의미에 맞는 bounded
  drag control로 바꾼다.
- drag 중에는 active Document를 바꾸지 않고 Detail draft를 복사한 임시 Document만 기존
  `CEffectObject::Stage_Document` 경로에 live-stage한다.
- Apply는 draft를 active Document에 commit하고, Revert는 active Document를 다시 stage한다.
  잘못된 범위나 stage 실패는 기존 preview와 active Document를 유지한다.

검증: live-stage/Apply/Revert 경계 audit, Client Debug/Release build, Effect harness,
ProjectAudit와 `git diff --check`를 실행한다.

### G13. Resource Browser frame pacing

- Resource catalog refresh 때 asset category를 한 번 계산하고 file kind별 category 목록을
  commit한다.
- Resource Browser의 visible index는 catalog revision, 선택 slot file kind, filter, category가
  바뀔 때만 다시 계산한다. 평상시 frame에는 전체 catalog와 filesystem path를 재순회하지 않는다.
- 실제 DDS/WModel thumbnail은 유지하되 한 frame의 동기 생성 수와 생성 간격을 제한하고 DDS는
  thumbnail 용도에 맞는 최대 dimension으로 로드한다.
- Effect Tool 창 자체에 FPS/frame ms를 표시하고 Profiler capture에 `EffectTool.Render`와
  `EffectTool.ResourceGrid` CPU scope를 남긴다.

검증: cache revision/frame budget 정적 audit, Client Debug build, Effect harness,
ProjectAudit와 실제 F1 전후 FPS/CPU/GPU capture 비교를 수행한다.
