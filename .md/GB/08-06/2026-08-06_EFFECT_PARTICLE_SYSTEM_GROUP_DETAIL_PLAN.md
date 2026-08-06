# Effect Particle System Group 상세 코드 계획

## 1. 구현 목표

원본에서 변환된 95/110 Particle layer의 timing, material, resource, random seed를 보존하면서 사용자가
완성 Particle 묶음 하나를 선택해 전체 scale, yaw, 방출 방향, 초기 속력만 비파괴적으로 조절하게 한다.
Winters의 `WFX asset -> emitter list -> selected emitter detail` 구조를 LostArk의 대규모 변환 문서에
맞게 `Effect Document -> Particle System -> source layers -> selected layer detail`로 확장한다.

완료 화면은 다음 두 편집 수준을 명확히 구분한다.

```text
Particle System 선택: 완성 묶음의 상대 배치와 방출 전체 보정
Particle Layer 선택 : 문제가 있는 한 layer의 timing/material/shape 정밀 보정
```

## 2. 데이터 계약

### G25-1. `Client/Public/Effect_AuthoringDocument.h`

포맷 상수를 8로 올리고 아래 타입과 필드를 추가한다.

```cpp
inline constexpr uint32_t EFFECT_AUTHORING_FORMAT_VERSION = 8u;

struct EFFECT_PARTICLE_SYSTEM_DESC final
{
    f32_t fUniformScaleMultiplier = 1.f;
    f32_t fYawOffsetDegrees = 0.f;
    f32_t fDirectionYawDegrees = 0.f;
    f32_t fInitialSpeedMultiplier = 1.f;
};

struct EFFECT_DOCUMENT_DESC final
{
    uint32_t iFormatVersion = EFFECT_AUTHORING_FORMAT_VERSION;
    std::string strEffectAssetId;
    std::string strDisplayName;
    EFFECT_PARTICLE_SYSTEM_DESC ParticleSystem;
    std::vector<EFFECT_MODEL_CUE_DESC> ModelCues;
    std::vector<EFFECT_ELEMENT_DESC> Elements;
};
```

JSON 저장 순서는 document identity 다음, `modelCues` 전이다.

```json
"particleSystem": {
  "uniformScaleMultiplier": 1.0,
  "yawOffsetDegrees": 0.0,
  "directionYawDegrees": 0.0,
  "initialSpeedMultiplier": 1.0
}
```

### G25-2. `Client/Private/Effect_DocumentCodec.cpp`

`Parse_Value`는 source version이 8 이상이면 네 필드를 모두 읽는다. v3~v7은 struct 기본값을 사용한다.
`Validate`는 모든 값의 finite 여부와 scale/speed 범위를 검사한다. `Serialize`는 항상 v8 object를 쓴다.
파싱은 기존처럼 staged document를 완성한 뒤 한 번만 validate/commit한다.

## 3. 재생 수학

### G25-3. `Client/Public/Effect_Playback.h`

아래 private helper를 선언한다.

```cpp
float4x4_t Evaluate_ElementWorld(
    const EFFECT_ELEMENT_DESC& Element,
    f32_t fSampleTimeSeconds,
    const float4x4_t& RootWorld) const;
float3_t Apply_ParticleEmissionModifier(const float3_t& Velocity) const;
```

### G25-4. `Client/Private/Effect_Playback.cpp`

Particle Element에만 system matrix를 삽입한다.

```cpp
matrix_t Parent = XMLoadFloat4x4(&RootWorld);
if (EFFECT_ELEMENT_KIND::PARTICLE == Element.eKind)
{
    const auto& System = m_Document.ParticleSystem;
    Parent = XMMatrixScaling(
        System.fUniformScaleMultiplier,
        System.fUniformScaleMultiplier,
        System.fUniformScaleMultiplier) *
        XMMatrixRotationY(XMConvertToRadians(System.fYawOffsetDegrees)) *
        Parent;
}
XMStoreFloat4x4(&Result, Local * Parent);
```

`Spawn_Particles`는 기존 random draw 순서를 바꾸지 않고 샘플링이 끝난 벡터에만 modifier를 적용한다.

```cpp
Particle.vVelocity = Apply_ParticleEmissionModifier(Particle.vVelocity);
```

`Apply_ParticleEmissionModifier`는 Y축 회전 뒤 speed multiplier를 곱한다. 이로써 같은 document/seed는
같은 sample을 만들고 modifier만 결과를 결정한다. acceleration과 lifetime은 변하지 않는다.

## 4. Effect Tool 선택과 draft transaction

### G25-5. `Client/Public/Effect_Tool.h`

```cpp
enum class EFFECT_DETAIL_SELECTION : uint8_t
{
    NONE,
    PARTICLE_SYSTEM,
    ELEMENT,
    END
};

enum class EFFECT_PREVIEW_FILTER : uint8_t
{
    COMPLETE,
    SOLO_PARTICLE_SYSTEM,
    SOLO_SELECTED,
    MUTE_SELECTED,
    END
};
```

추가 메서드와 상태는 다음과 같다.

```cpp
void Render_ParticleSystemDetail();
bool_t Try_SelectParticleSystem(const std::string& strEffectAssetId);
bool_t Try_AuditionParticleSystem();
bool_t Stage_ParticleSystemDraftPreview();
bool_t Apply_ParticleSystemDraft(EFFECT_DOCUMENT_DESC& Document) const;
void Reset_ParticleSystemDraft();
bool_t Has_UnappliedDetailDraft() const;

optional<EFFECT_PARTICLE_SYSTEM_DESC> m_ParticleSystemDraft;
EFFECT_DETAIL_SELECTION m_eDetailSelection = EFFECT_DETAIL_SELECTION::NONE;
bool_t m_bParticleSystemDraftDirty = false;
```

selection 전환은 어느 draft든 unapplied이면 거부한다. document load/discard는 두 draft를 모두 초기화한다.
저장, Save As, promote, pending Save & Load도 `Has_UnappliedDetailDraft()`를 사용한다.

### G25-6. `Client/Private/Effect_Tool.cpp`

`PARTICLE_LAYER_SUMMARY`에 source system/emitter count를 추가한다. system은 non-empty `groupId` unique
count, emitter는 `sourceNode`의 `|burst:` suffix를 제거한 unique count다.

Particle kind branch는 summary 부모를 selectable하게 만들고, 그 아래 `Layers (N)` tree에서 기존
Element를 표시한다. 부모 클릭은 다른 asset이면 guarded load, 같은 asset이면 selection만 변경한다.

`Render_EffectDetailWindow`는 selection kind로 분기한다.

```text
PARTICLE_SYSTEM -> Render_ParticleSystemDetail
ELEMENT         -> 기존 Element draft editor
NONE            -> 선택 안내
```

system editor의 drag는 `Stage_ParticleSystemDraftPreview`, Apply는 `Try_CommitDocument`, Revert는 active
document 값으로 복구한다. status는 live preview / memory applied / save required를 구분한다.

`Build_PreviewDocument`의 `SOLO_PARTICLE_SYSTEM`은 `PARTICLE`가 아닌 Element와 모든 Model Cue를 제거한다.
Complete/Solo/Mute radio 변경과 preview restart는 존재하는 system/element draft를 임시 document에 합성한다.

## 5. 변환기·publisher·데이터

### G25-7. 변환기

`build_imported_effect_documents.py`는 v8과 identity `particleSystem`을 출력하고 promote도 v8로 고정한다.
Python tests는 필드와 값까지 검사한다.

### G25-8. publisher와 정본 데이터

`Publish-Effects.ps1`과 ProjectAudit의 허용 version을 v8까지 확장하고 v8 object의 required property,
finite/range를 검사한다. F/T Authored와 executable Imported 문서는 identity modifier를 명시한 v8로
승격한다. 다른 v5~v7 Authored는 C++ codec과 publisher의 호환 입력으로 유지할 수 있다.

## 6. 자동·수동 검증

1. extractor unit tests: v8 identity emission/promote
2. Effect pipeline: v8 정상, v7 호환, 누락/범위 오류 거부, publish rollback
3. ClientFrontendHarness: codec round-trip과 runtime scale/yaw/direction/speed 수학
4. F/T 회귀: Element/layer/mesh-backed/budget/model cue 수 불변
5. Effect Tool final audit와 전체 ProjectAudit
6. Client Debug/Release build와 startup smoke
7. F1 수동: F/T system 자동 선택, 전체 scale/yaw/speed live preview, Apply/Revert/Save/Reload,
   Particle-only audition, 하위 layer 재선택

Summon과 Decal 값이 이 변경 전후 동일한지 frame 데이터와 문서 diff로 확인한다. GPU 화면 검증을 자동
PASS로 대신하지 않으며 조작이 불가능한 항목은 RESULT의 수동 미검증 목록에 남긴다.
