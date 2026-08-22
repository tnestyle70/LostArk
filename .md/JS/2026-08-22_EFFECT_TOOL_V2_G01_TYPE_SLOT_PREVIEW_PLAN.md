# Effect Tool v2 G01 — Effect Type 선택 · Mesh/Base/Noise/Mask/Emissive/Dissolve 슬롯 · 리소스 선택 · 미리보기

작성 2026-08-22. 브랜치 `feature/effect-tool-v2`. 기존 `CEffect_Tool`(팀장 소유)과 완전히 분리된
새 F1 툴이다. UE3 Cascade를 참고하는 툴의 첫 슬라이스로, 저장/문서/런타임 재생은 아직 없다.

## G01 범위 (사용자 확인 완료 2026-08-22)

- 슬롯별 라이브러리 필터: `Data/Effects/V2/TextureSlotUsage.v1.json`(`Tools/EffectToolV2/build_texture_slot_usage.py`가
  Imported/Authored 392개 문서에서 생성, 텍스처별 base/noise/mask/emissive/dissolve 사용 횟수 + 원본 파라미터명)을
  로드해 선택 슬롯으로 쓰인 적 있는 텍스처만 횟수순 표시. 여러 슬롯에 쓰인 텍스처는 각 슬롯에 모두 노출.
  `Show all`로 전체. 타일 뱃지 `B.. N.. M.. E.. D..`, 툴팁에 원본 파라미터명.

- F1 Developer Tools `Effect Tool v2` 버튼 → `Effect Tool v2` 창.
- Effect Type 라디오 `Mesh / Texture / Particle / Decal / Trail`.
- 슬롯 카드 `Mesh`(Mesh/Particle 타입에서만) + `Base / Noise / Mask / Emissive / Dissolve`. Type별 바인딩 독립.
- Resource Library: `Resources/Effect/**` DDS(텍스처 슬롯) / WModel(Mesh 슬롯) 스캔. 도메인은
  `Meshes/Textures/Models/Animations/VectorFields` 폴더 앞까지의 경로(`Esther/Wei` 등으로 세분화).
- 카드 클릭 → 슬롯 선택, 라이브러리 썸네일 클릭 → 바인드, 카드와 Preview 패널에 표시. `Clear`로 해제.
- DDS 미리보기: `DirectXTK CreateDDSTextureFromFile`(최대 512px), 프레임당 2개.
- WModel 썸네일: Engine `CModel`을 `NONANIM`→실패 시 `ANIM`으로 로드, 128px RT에 렌더.
  스태틱은 `Shader_VtxMeshPreview.hlsl`, 스킨은 v2 소유 `Shader_VtxAnimMeshPreview_V2.hlsl` +
  `Bind_BoneMatrices`. 스킨 모델은 Engine이 local bounds를 계산하지 않으므로(`Model.cpp:914`)
  `CModelDecoderRegistry`로 다시 디코드해 `정점 × inverseBind × combined`로 bounds를 구한다
  (원시 정점 bounds는 본 오프셋 공간이라 카메라가 빗나갔던 문제의 수정).
- Preview 패널에 모델 진단(skinned/static, 메시·본·애니·머티리얼 수, bounds, 메시별 diffuse 파일명) 표시.


## G03 범위 — Create Effect 월드 preview + 실시간 튜닝 (사용자 확인 완료 2026-08-22)

- `CEffectPreviewV2`(`Effect_Preview_V2.h/.cpp`, `CGameObject` 파생): `Prototype_GameObject_EffectPreviewV2`를
  `LEVEL::STATIC`에 1회 등록하고 현재 레벨의 `Layer_EffectPreviewV2`에 Clone. 레벨 전환 시 자동 정리(툴은 `weak_ptr`).
- Shape: Mesh 타입 → WModel(NONANIM, 실패 시 ANIM + `Bind_BoneMatrices`), Texture 타입 → `CVIBuffer_Rect` 빌보드.
- v2 소유 셰이더: `Shader_EffectV2_Common.hlsli`(공용 PS: Base(UV scale/pan) + Noise 왜곡 × Mask.r × Dissolve
  smoothstep × Tint + Emissive×Intensity, 패스 4개 = Alpha/Additive × DepthTest on/off),
  `Shader_EffectMeshV2.hlsl`, `Shader_EffectAnimMeshV2.hlsl`, `Shader_EffectRectV2.hlsl`. BLEND 그룹 제출.
- 툴: `Create Effect`(Base 필수, Mesh/Particle 타입은 Mesh 필수; Particle/Decal/Trail은 비활성) → 카메라 전방 3 m.
  Tuning: Restart/Visible/Bring To Camera, Position/Rotation/Scale, Tint/Blend/DepthTest/Billboard, UV Scale·Base Pan,
  Noise Strength/Scale/Pan, Emissive Intensity, Dissolve Amount/Softness, Lifetime/Loop/Play Rate. 실패 사유는
  `CEffectPreviewV2::Last_Error()`로 상태줄에 노출.
- 카메라 없는 Lobby에서는 안 보임(Character Select/Bern/Valtan/Test에서 사용).
- AGENTS '단일 Effect 런타임 경로' 규칙 대비: 이 경로는 Debug 툴 preview 전용이며 제품 재생 경로가 아니다.

## G04 범위 — Tuning 옵션 재구성 (pivot 상대 transform · Lerp 트랙 · velocity · color · bloom/distortion · UV · dissolve start)

사용자 메모(2026-08-22)를 그대로 옮긴 것이며 아직 구현·검증 전이다. 새 파일은 없다. `Effect_Tool_V2.h`, 다른 셰이더 3개, 프로젝트/필터는 바꾸지 않는다.

### 데이터 모델 (`CEffectPreviewV2`)

- `LERP_FLOAT3 { vStart, vEnd, bLerp; Evaluate(fLifeRatio) }`: `bLerp`가 꺼지면 항상 `vStart`, 켜지면
  `lerp(vStart, vEnd, fLifeRatio)`. `fLifeRatio = saturate(time / lifetime)`이며 lifetime 0(무한)이면 0으로 고정한다.
  사용자 메모의 "잔여 시간 비율"은 `1 - fLifeRatio`와 같은 축이므로 값은 start→end로 흐른다.
- `PARAMS`에 `Position / Rotation(deg) / Scale / Velocity(m/s)` 네 `LERP_FLOAT3` 트랙. 이전 `DESC.vPosition/vRotationDegrees/vScale`과
  `Position()/RotationDegrees()/Scale()` 접근자는 제거하고 `Params().Position.vStart` 등으로 일원화.
- **Pivot**: `DESC.PivotWorld`(float4x4, Create 시 카메라 전방 3 m translation) → `m_PivotWorld`. 네 트랙은 전부 pivot 로컬이며
  `World = S * R * T(position + displacement) * Pivot`. Billboard sprite는 pivot으로 변환한 위치만 쓰고 orientation은 카메라 basis로 교체.
  지금은 pivot이 월드 고정점이지만 나중에 부모(본/캐릭터) 행렬을 그대로 넣는 자리다. `PivotWorld()` 접근자, Tuning `Pivot (world)`로 이동.
- **Velocity**: `Update`에서 `m_vDisplacement += Velocity.Evaluate(ratio) * dt * playRate`. `Restart`와 loop wrap에서 0으로 리셋.
  Lerp를 켜면 속도 자체가 start→end로 변한다.
- **Color**: `vColorMul`(이전 Tint 대체), `vColorOffset`, `eColorClipChannel{RGB, ALPHA}` + `fColorClip`.
  셰이더: `rgb = max(base.rgb*mul.rgb+offset.rgb, 0)`, `a = saturate(base.a*mask*dissolve*mul.a+offset.a)`,
  `a<=0.001 → discard`, `fColorClip>0`이면 `max(r,g,b)` 또는 `a`가 그 이하일 때 discard. clip은 emissive 가산 전에 적용.
- **Bloom Intensity**(이전 Emissive Intensity 대체): `rgb += emissive.rgb * bloom`. Emissive 텍스처 미바인드면 셰이더가 무시하고 UI도 비활성.
  Renderer bloom threshold가 1.0이므로 1 이하면 번지지 않는다.
- **Distortion Intensity**: PS 출력이 `SV_TARGET0(SceneHDR) + SV_TARGET1(Target_Distortion)` 두 개가 되며
  `distortion = (noise.rg*2-1) * intensity * a`, noise 미바인드면 0. 단위는 UV offset이고 `Shader_Deferred` SCENE_RESOLVE가 ±0.05로 clamp하므로
  UI 범위 0~0.1. 블렌드 스테이트는 제품 `BS_EffectAlpha/BS_EffectAdditive`와 같은 형식(RT1 One/One, write mask RG)의
  `BS_EffectV2Alpha/BS_EffectV2Additive`를 v2 hlsli 안에 둔다. Noise Strength/Scale/Pan은 그대로 두고(noise sample을 공유) noise 미바인드면 함께 비활성.
- **UV**: `vUVStart`, `vUVSpeed`(이전 Base Pan), `vUVTileCount`(이전 UV Scale). `uv = texcoord * tile + start + speed * time`.
- **Dissolve Start**(이전 Dissolve Amount 슬라이더 대체): `amount = saturate((ratio - start) / (1 - start))`, `start>=1`이면 0.
  CPU에서 계산해 `g_DissolveAmount`로 바인드하므로 셰이더의 dissolve 식은 그대로. lifetime 0이면 진행하지 않는다(UI 경고).
- 유지: Blend/Depth Test/Billboard, Lifetime/Loop/Play Rate, Mesh Pre-Scale, Dissolve Softness.

### Tuning 창 레이아웃

```text
[Mesh|Sprite] time | life ratio | status      Restart  Visible  Bring To Camera
(경고) Lifetime is 0: Lerp tracks and Dissolve Start stay at their start values.
Transform (relative to pivot)
  Pivot (world)  x y z
  [ ] Position      x y z        ← 체크 시 아래에 End x y z
  [ ] Rotation (deg)
  [ ] Scale
  [ ] Velocity (m/s)
  Mesh Pre-Scale (Mesh만)
Color
  Color Mul (ColorEdit4) · Color Offset (-1~1) · [RGB|Alpha] Color Clip 0~1
Bloom / Distortion
  Bloom Intensity (Emissive 없으면 disabled) · Distortion Intensity · Noise Strength/Scale/Pan (Noise 없으면 disabled)
UV
  UV Start · UV Speed (uv/s) · UV TileCount
Dissolve
  Dissolve Start (life 0-1) · Dissolve Softness · "Dissolve amount now x.xx" (Dissolve 없으면 disabled)
Blend
  Blend · Depth Test · Billboard(Sprite)
Playback
  Lifetime · Loop · Play Rate
```

### 바뀌는 셰이더 상수 (`Shader_EffectV2_Common.hlsli`)

| 제거 | 추가 |
|---|---|
| `g_Tint`, `g_UVScale`, `g_BasePan`, `g_EmissiveIntensity` | `g_ColorMul`, `g_ColorOffset`, `g_ColorClip`, `g_ColorClipChannel(uint)`, `g_BloomIntensity`, `g_DistortionIntensity`, `g_UVStart`, `g_UVSpeed`, `g_UVTileCount` |

`PS_EFFECT_V2` 반환형이 `float4`에서 `PS_EFFECT_OUT{SV_TARGET0, SV_TARGET1}`로 바뀐다. Mesh/AnimMesh/Rect 셰이더는 include만 하므로 수정 없음.

### `Effect_Tool_V2.cpp` 변경 범위

- `Try_CreatePreview`: `Desc.vPosition/vScale` → `XMStoreFloat4x4(&Desc.PivotWorld, XMMatrixTranslationFromVector(Spawn))`.
- `Try_CreatePreview` 정의 바로 뒤, `Render_TuningPanel` 바로 앞에 파일 로컬 `namespace { void Draw_LerpTrack(...) }` 추가
  (체크박스 + Start DragFloat3 + 체크 시 End DragFloat3).
- `Render_TuningPanel` 본문 전체 교체(위 레이아웃). `Bring To Camera`는 `PivotWorld()`를 옮긴다.
- 나머지 함수와 `Effect_Tool_V2.h`는 그대로.

## G05 범위 — 스킨드 Mesh 이펙트의 클립 재생 (도철 `FX_CUDC_00_SK` 등)

배경: `Effect/Esther/Wei/Models/Dochul/FX_CUDC_00_SK.wmodel`은 클립 52개를 내장한다(`dochul_idle_normal_1`, `dochul_att_battle_1_01~10_01`,
`dochul_sk_dochul_01~03`, `dochul_sk_esthereffect_2`, …). v2 preview는 이미 `MODEL::ANIM`+`Bind_BoneMatrices`로 그리지만
`Play_Animation`을 부르지 않아 바인드 포즈로 멈춰 있었다. 인하우스 툴은 Cascade와 달리 "Skinned Mesh 엘리먼트 = 모델 + 클립 + 머티리얼"을
한 요소로 다루므로, 클립 재생을 preview 안에 넣고 Color/Bloom/Dissolve를 재생 중에 튜닝한다. Animation Tool/Character Preview 경로는
캐릭터 프리뷰용이라 여기서 쓰지 않는다(다른 이펙트를 도철 본에 붙이는 G06에서 pivot 소스 후보로만 검토).

### `CEffectPreviewV2`

- `PARAMS`에 `uint32_t iAnimationIndex = 0`, `bool_t bAnimationLoop = true` 추가. 재생 속도는 기존 `fPlayRate` 공유.
- public: `Is_Skinned()`, `Animation_Count()`(skinned가 아니면 0), `Animation_Name(i)`, `Animation_DurationSeconds(i)`
  (`Get_AnimationProgress`의 tick duration ÷ `Get_AnimationTickPerSecond`), `Animation_Progress(outSeconds, outDurationSeconds)`.
- private `Sync_Animation(bRestart)`: 클립 수 0이면 무시. `iAnimationIndex`를 범위로 clamp. `bRestart`이거나 인덱스가
  `m_iAppliedAnimationIndex`와 다르면 `Start_Animation(index, loop)`(트랙 0초로 되감기), 아니면 `Set_Animation(index, loop)`로 loop 플래그만 반영.
- 호출 시점: `Initialize` 텍스처 로드 후(`"Ready"` 직전), `Restart`, `Update`의 매 프레임(`Sync_Animation(false)` 뒤 `Play_Animation(fStep)`),
  lifetime loop wrap 시 `Sync_Animation(true)`. 따라서 `Restart`/loop wrap에서 이펙트 시간 0초와 클립 0초가 함께 출발한다.
- `m_bFinished`면 Play_Animation도 멈춘다(화면에서 사라지므로).
- Mesh Pre-Scale 기본값: 정적 FX 메시는 0.01(cm→m), 스킨드 모델은 `Initialize`에서 로드 직후 **0.0001**로 덮어쓴다.
  NPC 파이프라인(`buildScript/build_npc.py` + `cook_npc.py`) cook은 정점 cm + 아마추어 노드 scale 100이라 스킨 결과가 cm×100이고
  제품 `CNpcPresentationAssetService`도 `0.0001 * RotationY(-90)`로 admission한다. Tuning에 `1 / 0.01 / 0.0001` 프리셋 버튼.
  (이전 기록 "1.0이 맞음"은 다른 사람이 구운 구 도철 파일 기준이었고 그 파일은 아래 G06 note대로 폐기.)

### 도철 `FX_CUDC_00_SK` 재쿠킹 (2026-08-22)

- 구 파일(다른 팀원 추출)은 스켈레톤 bind translation cm, 애니 position key m(1/100)으로 불일치 → 클립 재생 시 본이 루트로 뭉쳐
  머리·몸통이 겹쳤다. 아마추어 노드 `dochul` scale 0.01(→ bind 결과 m, pre-scale 1.0에서 정상 크기)였으나 키만 m.
- LookInfo `EFDLChar_MN_CUDC_00.MN_CUDC_00.loa`: 메시 `MN_CUDC_00.Mesh.MN_CUDC_00_SK`, 애니셋 `MN_BSTRH_00.Ani.MN_BSTRH_00_Ani`(52클립,
  `sk_esthereffect_2` 포함). `FX_CUDC_00` 패키지는 없고 `MN_CUDC_00` 안에 `fx_cudc_00_sk.psk`(= `mn_cudc_00_sk.psk`와 재질명만 다름, 동일 크기)가 있다.
- 재현: `umodel_lostark_v7 -export -psk -uncook -groups MN_CUDC_00 / MN_BSTRH_00` → `_export_esther_wei_g`,
  `npc_lookinfo.py … 175320`(불사귀 도철) → `esther_wei_dochul.json`, `build_npc.py`(body 21,816 verts·46본·52 actions, 30fps bake) →
  `cook_npc.py Npc_175320=175320` → `Npc_175320.wmodel`(meshes 2, animations 52, 8,308,258 B). bind/anim translation 일치 확인
  (`b_effectname` -114.72 둘 다). 구 파일은 scratch `dochul_backup/`에 보관, `FX_CUDC_00_SK.wmodel`·`FX_CUDC_00_PRL_SK.wmodel` 둘 다 새 cook으로 교체
  (PRL psk는 재질만 다르고 geometry 동일). 클립 이름은 `npc_*`(`npc_sk_esthereffect_2`), 파트 `mn_cudc_00_mi`(#0 머리 `mn_cudc_00_d_loc_int`)·
  `mn_cudc_00-1_mi`(#1 몸 `mn_cudc_00-1_d`). 제품 orientation을 맞추려면 Rotation Y -90.
- 교체 직후 "WModel load failed": 새 cook은 `textures/*.tga`를 참조하는데 폴더엔 구 파일용 `.dds`만 있어 `CMaterial`이 fail-closed.
  cook 산출 tga 4장을 `Dochul/textures/`에 추가해 해결. 툴 썸네일/Create 실패 문구에 `CModelDecoderRegistry::Get_LastReport().error`를
  붙여 디코더 거부 사유가 보이게 했다(디코더 통과 후 `Ready_Materials` 실패는 report가 비어 있으므로 그때는 텍스처 파일 존재부터 확인).
- 2026-08-22 사용자 확인: 새 cook 정상 표시. 모델 내장 머티리얼 자동 바인딩·노말맵 Lit 모드(G07 후보)는 사용자 결정으로 보류.

### Tuning 창 `Animation` 섹션 (skinned일 때만, Transform 바로 아래)

```text
Clip      [idx] name            ← combo, 항목마다 "(x.xx s)" 길이 표시
[ ] Clip Loop   [Lifetime = Clip Length]
Clip 1.23 / 2.40 s (Play Rate applies)
```

클립이 0개인 skinned 모델은 "Skinned mesh without clips (bind pose only)."만 표시.

### 변경 파일

`Effect_Preview_V2.h/.cpp`, `Effect_Tool_V2.cpp`(`Render_TuningPanel`의 Mesh Pre-Scale 블록 바로 뒤에 섹션 추가). 셰이더·프로젝트 변경 없음.

## G06 범위 — Opaque 패스 + 서브메시(Part)별 Base/Visible

배경(2026-08-22 사용자 관찰): 도철 `FX_CUDC_00_SK`에 Base 한 장을 입히면 모델이 깨져 보였다. 실측 원인 두 가지.
(1) wmodel은 서브메시 2개(#0 정점 18,845 `fx_cudc_00-1_mi`, #1 정점 6,430 `lfx_cudc_00_mi`)이고 텍스처 경로가 박혀 있지 않아
Base 한 장이 두 파트 모두에 입혀진다. 짝은 옆 `textures/`의 `mn_cudc_00-1_d.dds`(#0), `mn_cudc_00_d_loc_int.dds`(#1).
(2) v2 패스 4개는 모두 depth write 없음 + CullMode None + 블렌드라 속이 있는 몸체는 뒷면·내부가 앞면 위로 겹친다.

### 셰이더 (`Shader_EffectV2_Common.hlsli`)

- `BS_EffectV2Opaque`(제품 `BS_EffectOpaque` 형식: RT0 블렌드 off, RT1 One/One write mask RG) 추가.
- 5번째 pass `Opaque`: `RS_Default`(back-face cull) + `DSS_Default`(depth write) + `BS_EffectV2Opaque`. PS는 동일(discard·Bloom·Dissolve 그대로).

### `CEffectPreviewV2`

- `BLEND_MODE`에 `SOLID` 추가(`OPAQUE`는 wingdi 매크로와 충돌). `Render`의 pass 선택: SOLID → 4, 아니면 기존 `blend + (depthTest ? 0 : 2)`.
- `struct PART { bVisible, strBaseAssetId, pBaseView }`, `std::vector<PART> m_Parts`를 Mesh 로드 직후 `Get_NumMeshes()` 크기로 생성.
- public `Part_Count / Part_Name(i)`(= `CModel::Get_MaterialName`) `/ Part_Visible(i)& / Part_BaseAssetId(i) / Set_PartBase(i, assetId)`
  (빈 문자열이면 오버라이드 해제, 로드 실패 시 이전 상태 유지하고 `E_FAIL`).
- Mesh 루프: `bVisible` false면 skip. 메시마다 Base SRV를 `PART.pBaseView ?: 공용 Base 슬롯`으로 골라 `g_HasBase/g_BaseTexture`를 다시 바인드.
  Noise/Mask/Emissive/Dissolve는 공용 유지.

### Tuning 창

- `Parts` 섹션(Mesh shape, 파트 1개 이상): "Base slot now: <파일명>" 안내 + 파트별 행
  `[v] #i <머티리얼명>  (shared Base | 파일명)  [<- Base slot] [Clear]`.
  사용 흐름: 라이브러리에서 텍스처를 클릭해 Base 카드를 바꾼 뒤(라이브 preview는 안 바뀜) 해당 파트의 `<- Base slot`으로 복사.
- Blend combo `Alpha / Additive / Opaque`. Opaque(`SOLID`)면 Depth Test 체크박스 비활성(항상 depth write).

### 변경 파일

`Shader_EffectV2_Common.hlsli`, `Effect_Preview_V2.h/.cpp`, `Effect_Tool_V2.cpp`(`Render_TuningPanel`의 Blend 섹션 앞에 Parts, Blend combo 항목). 프로젝트 변경 없음.

## G07-lite 범위 — Rim(프레넬) 외곽 발광 + Ghost Alpha

배경: 원본 도철 머티리얼 부모 `preset_ghost_msk`의 흰 외곽선은 프레넬/림 항이다. 라이팅·노말맵 없이 실루엣 발광만 넣는다
(모델 머티리얼 자동 바인딩·Lit 모드 G07은 사용자 결정으로 보류).

### 셰이더

- `PS_EFFECT_IN`에 `float3 vWorldNormal : NORMAL`, `float3 vWorldPosition : TEXCOORD1` 추가. 상수 `g_vCamPosition`, `g_RimColor`,
  `g_RimPower`, `g_RimIntensity`, `g_GhostAlpha`.
- Mesh VS: 월드 노말 `normalize(mul(N, (float3x3)World))`. AnimMesh VS: `mul(float4(N,0), boneMatrix)` 후 월드. Rect VS: 노말 0 → PS가 rim 생략.
- PS: `fresnel = pow(1 - saturate(abs(dot(N, V))), RimPower)`(CullMode None이라 뒷면도 `abs`), `a *= lerp(1, fresnel, GhostAlpha)`를
  discard 판정 전에, `rgb += RimColor * fresnel * RimIntensity`를 emissive 가산 직전에 적용.

### `CEffectPreviewV2` / Tuning

- `PARAMS`: `vRimColor(1,1,1,1)`, `fRimPower 3`, `fRimIntensity 0`, `fGhostAlpha 0`. `Bind_Common`이 `Get_CamPosition()`과 함께 바인드.
- Tuning Mesh shape에 `Rim (Fresnel)` 섹션: Rim Color(ColorEdit3) · Rim Power 0.1~16 · Rim Intensity 0~8 · Ghost Alpha 0~1.

### 변경 파일

`Shader_EffectV2_Common.hlsli`, `Shader_EffectRectV2.hlsl`, `Shader_EffectMeshV2.hlsl`, `Shader_EffectAnimMeshV2.hlsl`,
`Effect_Preview_V2.h/.cpp`, `Effect_Tool_V2.cpp`. 프로젝트 변경 없음.

## G09 범위 — Effect v2 문서 Save / Load

결정(2026-08-22, 사용자): 인게임 재생은 **B안** — v2 전용 문서 + v2 전용 런타임 오브젝트 + v2 바인딩 문서. "v2로 저작한 에셋만 v2 런타임"으로
범위를 한정하고 팀장 합의는 별도. G09는 그 첫 단계인 저작 문서 저장/로드만 다룬다.

### 문서 계약 `Data/Effects/V2/Authored/<effectId>.effectv2.json`

```json
{
  "schema": "lostark.effect-v2", "formatVersion": 1,
  "effectId": "esther.wei.dochul",
  "effectType": "Mesh",
  "slots": { "mesh": "Effect/Esther/Wei/Models/Dochul/FX_CUDC_00_SK.wmodel", "base": "...", "noise": "", "mask": "", "emissive": "", "dissolve": "" },
  "params": { "position": {"start":[0,0,0],"end":[0,0,0],"lerp":false}, "rotation": …, "scale": …, "velocity": …,
              "colorOffset":[0,0,0,0], "colorMul":[1,1,1,1], "colorClipChannel":"Alpha", "colorClip":0,
              "rimColor":[1,1,1,1], "rimPower":3, "rimIntensity":0, "ghostAlpha":0,
              "bloomIntensity":1, "distortionIntensity":0, "uvStart":[0,0], "uvSpeed":[0,0], "uvTileCount":[1,1],
              "noiseStrength":0, "noiseScale":1, "noisePan":[0,0], "dissolveStart":0, "dissolveSoftness":0.1,
              "blend":"Opaque", "billboard":true, "depthTest":true, "lifetime":0, "loop":true, "playRate":1,
              "meshPreScale":0.0001, "animationClip":"npc_sk_esthereffect_2", "animationLoop":true },
  "parts": [ { "index":0, "material":"mn_cudc_00_mi", "visible":true, "base":"…/mn_cudc_00_d_loc_int.dds" } ]
}
```

- `effectId` = 파일명 stem, `[A-Za-z0-9._-]{1,80}`. 클립은 인덱스가 아니라 **이름**으로 저장(모델 재쿠킹에 안전).
- 로드 검증(fail-closed): schema/formatVersion, `effectType` enum, `slots` 문자열 + 자산 실존(`CRuntimeAssetRoot::Resolve`), Mesh면 `slots.mesh` 필수,
  `params` 각 키 타입(누락 키는 기본값), enum 문자열, `meshPreScale>0`/`lifetime>=0`/`playRate>=0`, `parts[].index` 0~255·`base` 실존.
- 저장은 `.tmp`에 쓴 뒤 rename(원자). 파서는 `CDataJson::Parse`, 쓰기는 `CDataJson::Escape` 기반 수동 직렬화(`%.7g`).

### 코드

- `CEffectPreviewV2`: `DESC.bParamsAuthored`(true면 스킨드 pre-scale 자동값으로 덮지 않음), `m_CreationDesc`/`Creation_Desc()` — Save가 생성 당시 슬롯을 읽는다.
- `CEffect_Tool_V2`: `Effect_Tool_V2.h`가 `Effect_Preview_V2.h`를 include(전방선언 → 실체). `PART_OVERRIDE`, `DOCUMENT_STAGE`,
  `Spawn_Preview(Desc, Parts, clip)`(Try_CreatePreview와 Load의 공용 스폰: 프로토타입 등록·이전 preview 숨김·카메라 pivot·파트 오버라이드·클립 이름→인덱스),
  `Document_Directory()`, `Scan_Documents()`, `Save_Document()`, `Parse_Document()`(static, parse→validate→stage), `Load_Document()`(commit: type·슬롯 바인딩·스폰, 실패 시 이전 type/바인딩 복원).
  멤버 `m_ePreviewType`, `m_szEffectId[96]`, `m_Documents`, `m_bDocumentsScanned`, `m_strDocumentStatus`.
- UI: Create 패널 아래 `Document` 패널 — Effect ID 입력 · Save(라이브 preview 필요) · Rescan · 문서 목록(더블클릭 로드) · Load · 상태줄.

## G10 범위 — Attach: 타깃 NPC 소환 · 클립 재생/스크럽 · 본 pivot · 바인딩 문서

배경: 피벗을 붙이려면 소환 대상 모델을 지형 위에 띄우고 애니메이션을 재생/정지한 채 튜닝해야 한다. 공용 Character Preview는 NPC 애니셋을 붙이지 않아
웨이 클립을 못 돌리므로, **제품 NPC 표현 경로**(`CActorCatalog` + `CNpcPresentationAssetService::Ensure_Prototypes` + `Prototype_GameObject_Npc`)로 실제 CNpc를
`Layer_EffectPreviewV2Target`에 소환한다(ClientReplication과 같은 레시피, 두 번째 로더 없음). 위치는 **씬 캐릭터 옆 2.5 m 지면**(캐릭터 없으면 원점).

### 툴 (`Effect Attach v2` 창, Create 패널 `Attach...`)

- Target: supported NPC archetype 콤보 → `Spawn Target`/`Despawn`, Target Position/Yaw(DragFloat, `CNpc::Apply_NetworkState`), `Beside Character`.
  웨이 `NPC_58700`은 `esther.strike` 첫 클립(`npc_sk_dochul`)으로 시작. 본 이름은 모델 자산을 `CModelDecoderRegistry`로 디코드해 `Has_Bone` 필터.
- Target Playback: 클립 콤보(애니셋 포함) · 프레임 스크럽(`Set_AnimPaused`+`Set_AnimTrackPosition`) · Play/Pause · `< Frame`/`Frame >` · Restart Clip(이펙트도 Restart) · Loop.
- Effect Pivot: `Pivot Mode World / Target Bone` + Pivot Bone 콤보. Target Bone이면 매 프레임 `PivotWorld = Get_BoneMatrix(bone) × RotY(yaw)·T(pos)`
  (`Update_Attach`), Tuning의 Pivot(world) 입력은 비활성, Bring To Camera는 World 모드로 되돌림.
  **본 행렬의 스케일은 버린다**(축 정규화, 위치·회전만): NPC 본에는 admission 0.0001×아마추어 100=0.01이 합성돼 있어 그대로 쓰면 이펙트가 100분의 1로
  줄어 사라진다(2026-08-22 사용자 관찰 "Target Bone 후 도철 안 보임"의 원인).
  `Pivot Rotation` = `Bone`(본 회전 상속) / `Target Yaw`(본 위치 + 타깃 yaw, **기본**) / `World`(본 위치만). 본 회전을 상속하면 Tuning의 오일러
  Rotation(Z→X→Y 순, 피벗 축 기준)이 기울어진 본 축 위에서 돌아 직관과 어긋나므로 기본은 Target Yaw. 바인딩 행에 `"rotation": "Bone|TargetYaw|World"` 저장. `Spawn Frame`(30 fps) — 타깃 클립이 그 시각을 지나면 preview Restart.
- Bindings: `Data/Effects/V2/Bindings/<archetypeId>.effectv2bindings.json`
  `{ "schema":"lostark.effect-v2-bindings","formatVersion":1,"archetypeId":"NPC_58700",
     "bindings":[{"effectId":"esther.wei.dochul","clip":"npc_sk_dochul","startMs":0,"bone":"b_effectroot","followBone":true,"stopWithClip":false}] }`
  `Add / Update Binding`(effectId+clip 키), 행 클릭 → effectId/frame/pivot/clip 복원, `Stop w/ clip`, Remove, `Save Bindings`(.tmp→rename), `Reload`.
  로드 검증: schema/version/archetypeId 일치, effectId 형식, clip 비어있지 않음, startMs 0~600000, 중복(effectId+clip) 거부.
- 로컬 오프셋은 바인딩이 아니라 이펙트 문서의 Position/Rotation/Scale 트랙이 소유한다.

### 코드

`Effect_Tool_V2.h/.cpp`: `PIVOT_MODE`, `EFFECT_BINDING`, `Render_AttachWindow/Spawn_Target/Despawn_Target/Move_Target/Update_Attach/
Binding_Directory/Save_Bindings/Load_Bindings/Parse_Bindings/Collect_BoneNames`, 멤버 `m_pTarget(weak CNpc)`, `m_strTargetArchetypeId`, 위치/yaw, 본 목록,
`m_ePivotMode/m_strPivotBone/m_iSpawnFrame/m_Bindings`. include `ActorCatalog.h, AnimationTargetService.h, Character.h, Npc.h, NpcPresentationAssetService.h`.
`Render()`가 `Update_Attach` → Tuning → Attach 창 순으로 호출. 프로젝트 변경 없음.

## G11 범위 — v2 런타임: NPC 클립 시작 → 바인딩 스폰 (B안, 팀장 합의 전제)

목표: 게임에서 웨이(`NPC_58700`)가 `npc_sk_dochul`을 시작하면 `NPC_58700.effectv2bindings.json`의 세 행대로 1733/2966/4700 ms에
`esther.wei.dochul_1..3`이 `b_effectroot`(위치 + 타깃 yaw)에 스폰되고 lifetime 뒤 제거된다. Server·Shared 변경 없음(표현 전용).
이 경로는 AGENTS "단일 Effect 런타임" 규칙의 예외이며 **v2 문서(`.effectv2.json`)로 저작한 에셋만** 태운다 — 팀장 합의 항목으로 RESULT에 남긴다.

### 파일

| 파일 | 역할 |
|---|---|
| `Client/Public/EffectV2_Object.h`, `Client/Private/EffectV2_Object.cpp` (rename: `Effect_Preview_V2.*`) | `CEffectV2Object`(구 `CEffectPreviewV2`) — 툴 preview와 런타임이 같은 오브젝트. `Set_FollowTarget(weak CNpc, bone, rotation)`가 매 Update 피벗을 갱신(본 스케일 제거, Target Yaw/Bone/World) |
| `Client/Public/EffectV2_Document.h`, `Client/Private/EffectV2_Document.cpp` (새) | `EFFECT_V2_DOCUMENT`/`EFFECT_V2_BINDING` + `Parse_EffectV2Document/Bindings`, `Serialize_…`, 경로 헬퍼. 툴과 런타임이 같은 파서를 쓴다 |
| `Client/Public/EffectV2_Runtime.h`, `Client/Private/EffectV2_Runtime.cpp` (새) | `CEffectV2Runtime` static 서비스: archetype별 바인딩·문서 lazy 로드(fail-closed, archetype 격리), `Notify_NpcClip`, `Tick_Npc`, 스폰/제거 |
| `Client/Public/Npc.h`, `Client/Private/Npc.cpp` | `Get_Transform()` 추가, `Set_Animation` 성공 시 `CEffectV2Runtime::Notify_NpcClip(shared_from_this, clip)`, `Update` 끝에 `Tick_Npc(shared_from_this)` |
| `Effect_Tool_V2.h/.cpp` | 문서/바인딩 파서·직렬화를 `EffectV2_Document`로 이관, preview 피벗 추종을 `Set_FollowTarget`로 일원화 |
| `Client.vcxproj`, `.filters` | rename 항목 교체 + 새 4파일 등록(필터 `03. Tools\06. Effect V2`) |

### 런타임 흐름

```text
CNpc::Set_Animation(clip) 성공
  → CEffectV2Runtime::Notify_NpcClip(npc, clip)
      archetype = 모델 태그 역매핑(CActorCatalog::Get_Npcs × Get_ModelPrototypeTag) — NPC_DESC는 건드리지 않음
      bindings[archetype] lazy load (없으면 no-op, 파싱 실패면 그 archetype만 격리 + OutputDebugString 1회)
      clip이 일치하는 행마다 PENDING{weak npc, binding, bSpawned=false} 등록, 기존 stopWithClip 이펙트는 Finish
CNpc::Update 끝 → Tick_Npc(npc)
      npc 모델의 현재 클립 진행(초) 읽기; 클립이 바뀌었거나 시간이 되돌아가면 pending 리셋(loop 재생)
      진행 ≥ startMs/1000 이고 미스폰이면: 문서 lazy load → CEffectV2Object를 현재 레벨 Layer_EffectV2에 Clone(DESC=문서, bParamsAuthored)
        → Set_FollowTarget(npc, bone, rotation) → 파트/클립 적용
      스폰된 오브젝트 중 Is_Finished 또는 npc 만료 → Remove_GameObject_from_Layer
프로토타입 Prototype_GameObject_EffectV2는 STATIC에 1회 등록(툴과 공유)
```

- 레벨 전환: 레이어가 함께 정리되고 weak_ptr가 만료되므로 서비스는 pending/spawned를 `Tick`에서 정리한다. 문서·바인딩 캐시는 immutable로 프로세스 수명 유지(툴 Save 후 새 값을 보려면 Reload 명령 — `CEffectV2Runtime::Invalidate()`를 툴 Save가 호출).
- Esther 웨이: Server 스폰 → ClientReplication이 `npc->Set_Animation(chain.front())` → 위 흐름. Character(플레이어) 훅은 G12.

### 구현·검증 상태 (2026-08-23)

- 구현 완료: 위 파일 전부. 툴 Attach 창에 `Runtime spawns on target` 체크(기본 off = 툴 타깃은 런타임 무시, 켜면 저장된 바인딩으로 런타임이 타깃에 스폰 → 인게임 동작 확인용).
  툴 `Save`(문서·바인딩)는 `CEffectV2Runtime::Invalidate_Caches()`를 불러 다음 스폰부터 새 파일을 읽는다.
- 자동(실행): Client Debug x64 MSBuild exit 0(신규 경고 0), `git diff --check` 통과. vcxproj/filters에 `EffectV2_Document/Runtime` 등록, `Effect_Preview_V2.*` → `EffectV2_Object.*` rename.
- 수동(사용자 확인 2026-08-23): ① 툴 Attach 웨이 타깃 + `Runtime spawns on target`에서 1733/2966/4700 ms 도철 3종 스폰·소멸 확인,
  ② Valtan 맵에서 웨이 에스더 소환(Server 경로) 시 같은 동작 확인. 예열 후 스폰 히치 없음.
- 경계: CNpc 훅 2줄 + `Get_Transform/Get_ModelTag` 접근자는 팀 파일 변경(표현 전용, 동작 변화 없음). v2 런타임 경로 자체는 팀장 합의 항목.
- 예열(2026-08-23, 사용자 제안): 스폰 순간 `CModel::Create`(8 MB 디코드 ×2)+셰이더 컴파일+DDS 로드가 동기로 돌아 히치가 났다.
  `CEffectV2Object`에 정적 리소스 캐시(모델 프로토타입+스킨 여부 / 셰이더 / 텍스처 SRV)와 `Prewarm(device, ctx, DESC)`를 두고 스폰은 `CModel::Clone`만 한다.
  `CEffectV2Runtime::Prewarm_Archetype`가 바인딩·문서를 읽어 각 문서를 Prewarm + 이펙트 프로토타입 등록. 호출 seam은
  `CNpcPresentationAssetService::Ensure_Prototypes` 성공 끝 한 줄(팀 파일, NPC 프로토타입 준비 = Valtan 로더 에스더 선로드 시점) — 툴 Spawn_Target도 같은 경로.
  자산 캐시는 프로세스 수명(immutable), 문서/바인딩 캐시만 툴 Save에서 Invalidate.

### 다음 단계(미구현)

플레이어 캐릭터 타깃(`CAnimationTargetService` 경로·`CCharacter` 훅)은 G12.

## G08 — 인버티드 헐 Outline (추가 후 제거, 2026-08-22)

pass 5 `Outline`(CullFront, 노말 방향 월드 m 밀어내기, 단색) + Tuning `Outline` 섹션 + 파트별 제외를 넣어 확인했으나 사용자 판정으로 제거.
사유: 디졸브로 사라지는 연출과 겹치면 헐이 모델 크기만큼 남아 보이고, 윤곽은 툴 파라미터가 아니라 나중에 전용 셰이더(스크린 스페이스 등)에서
처리할 문제. 코드·셰이더는 G07-lite 상태로 되돌렸고 Rim 기능만 남긴다.

## 파일

| 파일 | 역할 |
|---|---|
| `Client/Public/Effect_Tool_V2.h` (새) | `CEffect_Tool_V2` 선언 |
| `Client/Private/Effect_Tool_V2.cpp` (새) | 스캔/미리보기/썸네일/ImGui |
| `Client/Bin/ShaderFiles/Shader_VtxAnimMeshPreview_V2.hlsl` (새) | 스킨 메시 썸네일 셰이더 |
| `Client/Public/Effect_Preview_V2.h`, `Client/Private/Effect_Preview_V2.cpp` (새, G04에서 PARAMS/pivot/lerp/velocity 재구성) | 월드 preview GameObject |
| `Client/Bin/ShaderFiles/Shader_EffectV2_Common.hlsli` (새, G04에서 상수·PS 출력·블렌드 교체), `Shader_EffectMeshV2.hlsl`, `Shader_EffectAnimMeshV2.hlsl`, `Shader_EffectRectV2.hlsl` (새) | preview 셰이더 |
| `Tools/EffectToolV2/build_texture_slot_usage.py` (새) | 슬롯 사용 사이드카 생성기 |
| `Data/Effects/V2/TextureSlotUsage.v1.json` (생성물, Git 추적) | 텍스처→슬롯 사용 횟수 |
| `Client/Public/MainApp.h`, `Client/Private/MainApp.cpp` | `DEBUG_TOOL::EFFECT_V2`, 멤버, 버튼/생성/Render/Free |
| `Client/Default/Client.vcxproj`, `.filters` | 등록, 필터 `03. Tools\06. Effect V2` |

의존: `imgui.h`, `RuntimeAssetRoot.h`, `Model.h`, `Shader.h`, `BinaryAsset/ModelDecoderRegistry.h`,
`DirectXTK/DDSTextureLoader.h`. 기존 Effect 코드(`Effect_AuthoringDocument.h`, `CEffectThumbnailCache`,
`CEffectCatalog`)는 include하지 않는다.

## 검증

- Client Debug x64 빌드 성공, `Effect_Tool_V2.cpp` 경고 0, `git diff --check` 통과.
- 사용자 확인(2026-08-22): Type/슬롯/라이브러리/미리보기 동작, `Esther/Balthorr·Thirain·Wei` 도메인 분리,
  `Esther/Wei/Models/Dochul/FX_CUDC_00_SK.wmodel` 스킨 썸네일 표시.
- G04 자동(2026-08-22 실행): Client Debug x64 MSBuild exit 0(신규 경고 0, 기존 `GameInstance.h` C4819만),
  `fxc /T fx_5_0`로 Rect/Mesh/AnimMesh V2 셰이더 3개 컴파일 성공(X4717 deprecated 경고만), `git diff --check` 통과.
  사용자 수동(미실행): Create → Pivot 이동과 Position 분리, 각 트랙 Lerp 체크 시 End 칸 표시·lifetime 동안 보간, Velocity로 이동·Restart 리셋,
  Color Mul/Offset/Clip(RGB·Alpha), Emissive 바인드 시 Bloom Intensity > 1에서 번짐, Noise 바인드 시 Distortion Intensity로 배경 굴절,
  UV Start/Speed/TileCount, Dissolve 바인드 + lifetime > 0에서 Dissolve Start 이후 소멸, lifetime 0 경고 표시.
- G05 자동(2026-08-22 실행): Client Debug x64 MSBuild exit 0(신규 경고 0), `git diff --check` 통과.
  사용자 수동(미실행): Mesh 타입에 `FX_CUDC_00_SK.wmodel` 바인드 → Create → Animation 섹션 표시, Clip combo 52개·길이 표시,
  `dochul_sk_esthereffect_2` 선택 시 재생, Clip Loop, Restart 시 클립 0초 동기, `Lifetime = Clip Length` 뒤 lifetime loop에서 클립도 되감김,
  Play Rate가 클립 속도에 반영, 재생 중 Color/Bloom/Dissolve 튜닝 반영.
- G06 자동(2026-08-22 실행): Client Debug x64 MSBuild exit 0(`OPAQUE` 매크로 충돌로 1회 실패 후 `SOLID`로 rename), fxc fx_5_0 3개 셰이더
  컴파일 성공(Opaque pass 포함), `git diff --check` 통과.
  사용자 수동(미실행): 도철 Create → Blend `Opaque`에서 뒷면/내부 겹침 사라짐, Parts 섹션에 #0 `fx_cudc_00-1_mi`/#1 `lfx_cudc_00_mi` 2행,
  라이브러리에서 `mn_cudc_00_d_loc_int` 선택 후 #1 `<- Base slot`으로 파트 텍스처 분리, Visible 체크로 파트 숨김, Clear로 공용 Base 복귀.

## 전체 코드

### Client/Public/Effect_Tool_V2.h

```cpp
#pragma once

#include "Client_Defines.h"
#include "EffectV2_Document.h"
#include "EffectV2_Object.h"
#include "Engine_Defines.h"

#include <array>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

NS_BEGIN(Engine)
class CModel;
class CShader;
NS_END

NS_BEGIN(Client)

class CNpc;

class CEffect_Tool_V2 final
{
public:
	using EFFECT_TYPE = EFFECT_V2_TYPE;

	enum class RESOURCE_SLOT : int32_t
	{
		MESH,
		BASE,
		NOISE,
		MASK,
		EMISSIVE,
		DISSOLVE,
		END
	};

	enum class RESOURCE_KIND : int32_t
	{
		TEXTURE,
		MODEL,
		END
	};

private:
	struct TEXTURE_USAGE final
	{
		std::array<uint32_t, static_cast<size_t>(RESOURCE_SLOT::END)> Counts{};
		std::vector<std::string> Params;
	};

	struct RESOURCE_ENTRY final
	{
		std::string strAssetId;
		std::string strDomain;
		std::string strFileName;
		RESOURCE_KIND eKind = RESOURCE_KIND::TEXTURE;
		const TEXTURE_USAGE* pUsage = nullptr;
	};

	struct PREVIEW_ENTRY final
	{
		ComPtr<ID3D11ShaderResourceView> pTextureView;
		std::string strError;
		std::string strInfo;
		uint32_t iWidth = 0u;
		uint32_t iHeight = 0u;
	};

	using SLOT_BINDINGS =
		std::array<std::string, static_cast<size_t>(RESOURCE_SLOT::END)>;

	using PART_OVERRIDE = EFFECT_V2_PART_OVERRIDE;
	using PIVOT_ROTATION = CEffectV2Object::PIVOT_ROTATION;
	using EFFECT_BINDING = EFFECT_V2_BINDING;

	enum class PIVOT_MODE : int32_t
	{
		WORLD,
		TARGET_BONE,
		END
	};

public:
	CEffect_Tool_V2(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	~CEffect_Tool_V2();

	void Render();

private:
	void Scan_Resources();
	void Load_TextureUsage();
	void Rebuild_VisibleResources();
	const PREVIEW_ENTRY* Request_Preview(
		const std::string& strAssetId, RESOURCE_KIND eKind);
	bool_t Create_ModelThumbnail(
		const std::filesystem::path& Path,
		ComPtr<ID3D11ShaderResourceView>& OutTextureView,
		std::string& strOutError,
		std::string& strOutInfo);
	static bool_t Compute_SkinnedBounds(
		const std::filesystem::path& Path,
		const Engine::CModel& Model,
		float3_t& OutMinimum,
		float3_t& OutMaximum,
		std::string& strOutInfo);

	void Render_TypeSelector();
	void Render_SlotCards();
	void Render_ResourceBrowser();
	void Render_PreviewPanel();
	void Render_CreatePanel();
	void Render_DocumentPanel();
	void Render_TuningPanel();
	bool_t Try_CreatePreview();
	bool_t Spawn_Preview(
		const CEffectV2Object::DESC& Desc,
		const std::vector<PART_OVERRIDE>& Parts,
		const std::string& strAnimationClip);
	void Scan_Documents();
	bool_t Save_Document();
	bool_t Load_Document(const std::string& strEffectId);

	void Render_AttachWindow();
	bool_t Spawn_Target(const std::string& strArchetypeId);
	void Despawn_Target();
	void Move_Target(const float3_t& vPosition, f32_t fYawDegrees);
	void Update_Attach(f32_t fTimeDelta);
	bool_t Save_Bindings();
	bool_t Load_Bindings(const std::string& strArchetypeId);
	static bool_t Collect_BoneNames(
		const std::string& strModelAssetId,
		std::vector<std::string>& OutNames);

	SLOT_BINDINGS& Current_Bindings();
	std::string& Current_SlotAssetId();
	bool_t Slot_VisibleForType(RESOURCE_SLOT eSlot) const;

	static RESOURCE_KIND Slot_Kind(RESOURCE_SLOT eSlot);
	static std::string Domain_FromRelativePath(
		const std::filesystem::path& EffectRelative);
	static const char* Type_Label(EFFECT_TYPE eType);
	static const char* Slot_Label(RESOURCE_SLOT eSlot);
	static const char* Slot_Description(RESOURCE_SLOT eSlot);

private:
	ComPtr<ID3D11Device> m_pDevice;
	ComPtr<ID3D11DeviceContext> m_pContext;
	shared_ptr<Engine::CShader> m_pModelShader;
	shared_ptr<Engine::CShader> m_pAnimModelShader;

	EFFECT_TYPE m_eType = EFFECT_TYPE::MESH;
	RESOURCE_SLOT m_eSelectedSlot = RESOURCE_SLOT::BASE;
	std::array<SLOT_BINDINGS, static_cast<size_t>(EFFECT_TYPE::END)> m_SlotBindings;

	bool_t m_bScanned = false;
	std::vector<RESOURCE_ENTRY> m_Resources;
	std::vector<std::string> m_Domains;
	std::vector<size_t> m_VisibleResources;
	RESOURCE_KIND m_eVisibleKind = RESOURCE_KIND::END;
	std::string m_strDomainFilter;
	char m_szNameFilter[128] = {};
	bool_t m_bVisibleDirty = true;
	std::unordered_map<std::string, TEXTURE_USAGE> m_TextureUsage;
	std::string m_strUsageStatus;

	std::unordered_map<std::string, PREVIEW_ENTRY> m_Previews;
	uint32_t m_iLoadsThisFrame = 0u;

	std::weak_ptr<CEffectV2Object> m_pPreview;
	EFFECT_TYPE m_ePreviewType = EFFECT_TYPE::MESH;
	bool_t m_bPreviewPrototypeRegistered = false;
	bool_t m_bTuningWindowOpen = false;
	std::string m_strPreviewStatus;

	char m_szEffectId[96] = {};
	std::vector<std::string> m_Documents;
	bool_t m_bDocumentsScanned = false;
	std::string m_strDocumentStatus;

	bool_t m_bAttachWindowOpen = false;
	std::weak_ptr<CNpc> m_pTarget;
	std::string m_strTargetArchetypeId;
	int32_t m_iTargetArchetypeSelection = -1;
	float3_t m_vTargetPosition = { 0.f, 0.f, 0.f };
	f32_t m_fTargetYawDegrees = 0.f;
	std::vector<std::string> m_TargetBoneNames;
	bool_t m_bTargetClipLoop = true;
	bool_t m_bRuntimeOnTarget = false;
	f32_t m_fTargetLastClipSeconds = -1.f;
	PIVOT_MODE m_ePivotMode = PIVOT_MODE::WORLD;
	PIVOT_ROTATION m_ePivotRotation = PIVOT_ROTATION::TARGET_YAW;
	std::string m_strPivotBone;
	int32_t m_iSpawnFrame = 0;
	std::vector<EFFECT_BINDING> m_Bindings;
	std::string m_strAttachStatus;

	std::string m_strStatus;
};

NS_END
```

### Client/Private/Effect_Tool_V2.cpp

```cpp
#include "imgui.h"

#include "Effect_Tool_V2.h"
#include "ActorCatalog.h"
#include "AnimationTargetService.h"
#include "BinaryAsset/ModelAssetData.h"
#include "BinaryAsset/ModelDecoderRegistry.h"
#include "Character.h"
#include "DataJson.h"
#include "EffectV2_Object.h"
#include "EffectV2_Runtime.h"
#include "GameInstance.h"
#include "Model.h"
#include "Npc.h"
#include "NpcPresentationAssetService.h"
#include "ProjectDataRoot.h"
#include "RuntimeAssetRoot.h"
#include "Shader.h"

#include "DirectXTK/DDSTextureLoader.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iterator>
#include <limits>
#include <set>

namespace
{
	constexpr uint32_t MAX_PREVIEW_LOADS_PER_FRAME = 2u;
	constexpr size_t MAX_PREVIEW_DIMENSION = 512u;
	constexpr uint32_t MODEL_THUMBNAIL_SIZE = 128u;
	constexpr float SLOT_CARD_SIZE = 96.f;
	constexpr float BROWSER_TILE_SIZE = 80.f;
	constexpr float PREVIEW_PANEL_SIZE = 256.f;

	constexpr const char* ASSET_KIND_FOLDERS[] = {
		"meshes", "textures", "models", "animations", "vectorfields"
	};

	std::string To_Lower(std::string Value)
	{
		std::transform(Value.begin(), Value.end(), Value.begin(),
			[](const char Character)
			{
				return static_cast<char>(std::tolower(
					static_cast<unsigned char>(Character)));
			});
		return Value;
	}

	bool_t Is_AssetKindFolder(const std::string& strName)
	{
		const std::string Lower = To_Lower(strName);
		for (const char* pFolder : ASSET_KIND_FOLDERS)
		{
			if (Lower == pFolder)
				return true;
		}
		return false;
	}
}

Client::CEffect_Tool_V2::CEffect_Tool_V2(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: m_pDevice(std::move(pDevice)),
	  m_pContext(std::move(pContext))
{
}

Client::CEffect_Tool_V2::~CEffect_Tool_V2() = default;

void Client::CEffect_Tool_V2::Render()
{
	m_iLoadsThisFrame = 0u;
	if (!m_bScanned)
		Scan_Resources();
	if (!Slot_VisibleForType(m_eSelectedSlot))
		m_eSelectedSlot = RESOURCE_SLOT::BASE;

	if (!ImGui::Begin("Effect Tool v2"))
	{
		ImGui::End();
		return;
	}

	Render_TypeSelector();
	ImGui::Separator();
	Render_SlotCards();
	ImGui::Separator();

	if (ImGui::BeginTable("EffectToolV2Body", 2,
		ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV))
	{
		ImGui::TableSetupColumn("Resources",
			ImGuiTableColumnFlags_WidthStretch, 0.62f);
		ImGui::TableSetupColumn("Preview",
			ImGuiTableColumnFlags_WidthStretch, 0.38f);
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		Render_ResourceBrowser();
		ImGui::TableSetColumnIndex(1);
		Render_PreviewPanel();
		ImGui::Separator();
		Render_CreatePanel();
		ImGui::Separator();
		Render_DocumentPanel();
		ImGui::EndTable();
	}

	if (!m_strStatus.empty())
		ImGui::TextWrapped("%s", m_strStatus.c_str());
	ImGui::End();

	Update_Attach(ImGui::GetIO().DeltaTime);
	Render_TuningPanel();
	Render_AttachWindow();
}

void Client::CEffect_Tool_V2::Scan_Resources()
{
	m_bScanned = true;
	m_Resources.clear();
	m_Domains.clear();
	m_bVisibleDirty = true;

	std::error_code Error;
	const std::filesystem::path Root = CRuntimeAssetRoot::Get();
	const std::filesystem::path EffectRoot = Root / "Effect";
	if (!std::filesystem::is_directory(EffectRoot, Error) || Error)
	{
		m_strStatus = "Resources/Effect is missing: " + EffectRoot.string();
		return;
	}

	std::set<std::string> Domains;
	size_t iTextures = 0u;
	size_t iModels = 0u;
	for (std::filesystem::recursive_directory_iterator Iterator(
		EffectRoot,
		std::filesystem::directory_options::skip_permission_denied,
		Error), End; Iterator != End; Iterator.increment(Error))
	{
		if (Error)
		{
			Error.clear();
			continue;
		}
		if (!Iterator->is_regular_file())
			continue;
		const std::string Extension =
			To_Lower(Iterator->path().extension().string());
		RESOURCE_KIND eKind = RESOURCE_KIND::END;
		if (".dds" == Extension)
			eKind = RESOURCE_KIND::TEXTURE;
		else if (".wmodel" == Extension)
			eKind = RESOURCE_KIND::MODEL;
		else
			continue;
		const std::filesystem::path EffectRelative =
			Iterator->path().lexically_relative(EffectRoot);
		if (EffectRelative.empty() || !EffectRelative.has_parent_path())
			continue;
		const std::string Domain = Domain_FromRelativePath(EffectRelative);
		if (Domain.empty())
			continue;
		Domains.insert(Domain);
		(RESOURCE_KIND::TEXTURE == eKind ? iTextures : iModels)++;
		m_Resources.push_back({
			Iterator->path().lexically_relative(Root).generic_string(),
			Domain,
			Iterator->path().filename().string(),
			eKind });
	}
	std::sort(m_Resources.begin(), m_Resources.end(),
		[](const RESOURCE_ENTRY& Left, const RESOURCE_ENTRY& Right)
		{
			return Left.strAssetId < Right.strAssetId;
		});
	m_Domains.assign(Domains.begin(), Domains.end());
	Load_TextureUsage();
	size_t iWithUsage = 0u;
	for (RESOURCE_ENTRY& Entry : m_Resources)
	{
		if (RESOURCE_KIND::TEXTURE != Entry.eKind)
			continue;
		const auto Iterator = m_TextureUsage.find(To_Lower(Entry.strFileName));
		Entry.pUsage = m_TextureUsage.end() == Iterator ? nullptr : &Iterator->second;
		iWithUsage += nullptr != Entry.pUsage ? 1u : 0u;
	}
	m_strStatus = "Scanned " + std::to_string(iTextures) + " DDS (" +
		std::to_string(iWithUsage) + " with slot usage), " +
		std::to_string(iModels) + " WModel in " +
		std::to_string(m_Domains.size()) + " domains. " + m_strUsageStatus;
}

void Client::CEffect_Tool_V2::Load_TextureUsage()
{
	m_TextureUsage.clear();
	const std::filesystem::path Path =
		CProjectDataRoot::Resolve(L"Effects/V2/TextureSlotUsage.v1.json");
	std::ifstream File(Path, std::ios::binary);
	if (!File)
	{
		m_strUsageStatus = "TextureSlotUsage.v1.json not found.";
		return;
	}
	const std::string Text{
		std::istreambuf_iterator<char>(File), std::istreambuf_iterator<char>() };
	DATA_JSON_VALUE Root;
	std::string Error;
	if (!CDataJson::Parse(Text, Root, Error) || !Root.Is_Object())
	{
		m_strUsageStatus = "TextureSlotUsage parse failed: " + Error;
		return;
	}
	const DATA_JSON_VALUE* pVersion = Root.Find("formatVersion");
	const DATA_JSON_VALUE* pTextures = Root.Find("textures");
	if (nullptr == pVersion || !pVersion->Is_Number() ||
		1 != static_cast<int32_t>(pVersion->Get_Number()) ||
		nullptr == pTextures || !pTextures->Is_Object())
	{
		m_strUsageStatus = "TextureSlotUsage has an unsupported format.";
		return;
	}
	for (const std::string& Name : pTextures->Get_ObjectInsertionOrder())
	{
		const DATA_JSON_VALUE* pEntry = pTextures->Find(Name);
		if (nullptr == pEntry || !pEntry->Is_Object())
			continue;
		TEXTURE_USAGE Usage;
		for (int32_t iSlot = static_cast<int32_t>(RESOURCE_SLOT::BASE);
			iSlot < static_cast<int32_t>(RESOURCE_SLOT::END); ++iSlot)
		{
			const DATA_JSON_VALUE* pCount =
				pEntry->Find(To_Lower(Slot_Label(static_cast<RESOURCE_SLOT>(iSlot))));
			if (nullptr != pCount && pCount->Is_Number())
				Usage.Counts[static_cast<size_t>(iSlot)] =
					static_cast<uint32_t>((std::max)(0.0, pCount->Get_Number()));
		}
		if (const DATA_JSON_VALUE* pParams = pEntry->Find("params");
			nullptr != pParams && pParams->Is_Array())
		{
			for (const DATA_JSON_VALUE& Param : pParams->Get_Array())
			{
				if (Param.Is_String())
					Usage.Params.push_back(Param.Get_String());
			}
		}
		m_TextureUsage.emplace(To_Lower(Name), std::move(Usage));
	}
	m_strUsageStatus = "Slot usage: " + std::to_string(m_TextureUsage.size()) +
		" textures.";
}

std::string Client::CEffect_Tool_V2::Domain_FromRelativePath(
	const std::filesystem::path& EffectRelative)
{
	std::string Domain;
	const std::filesystem::path Parent = EffectRelative.parent_path();
	for (const std::filesystem::path& Component : Parent)
	{
		const std::string Name = Component.string();
		if (Is_AssetKindFolder(Name))
			break;
		if (!Domain.empty())
			Domain += '/';
		Domain += Name;
	}
	if (Domain.empty() && Parent.begin() != Parent.end())
		Domain = Parent.begin()->string();
	return Domain;
}

void Client::CEffect_Tool_V2::Rebuild_VisibleResources()
{
	m_bVisibleDirty = false;
	m_VisibleResources.clear();
	const RESOURCE_KIND eKind = Slot_Kind(m_eSelectedSlot);
	m_eVisibleKind = eKind;
	const std::string Filter = To_Lower(m_szNameFilter);
	const bool_t bDedupeByName = m_strDomainFilter.empty();
	std::set<std::string> SeenNames;
	for (size_t iEntry = 0u; iEntry < m_Resources.size(); ++iEntry)
	{
		const RESOURCE_ENTRY& Entry = m_Resources[iEntry];
		if (Entry.eKind != eKind)
			continue;
		if (!m_strDomainFilter.empty() && Entry.strDomain != m_strDomainFilter)
			continue;
		const std::string LowerName = To_Lower(Entry.strFileName);
		if (!Filter.empty() && std::string::npos == LowerName.find(Filter))
			continue;
		if (bDedupeByName && !SeenNames.insert(LowerName).second)
			continue;
		m_VisibleResources.push_back(iEntry);
	}
}

const Client::CEffect_Tool_V2::PREVIEW_ENTRY*
Client::CEffect_Tool_V2::Request_Preview(
	const std::string& strAssetId, const RESOURCE_KIND eKind)
{
	if (strAssetId.empty())
		return nullptr;
	auto Iterator = m_Previews.find(strAssetId);
	if (Iterator != m_Previews.end())
		return &Iterator->second;
	if (m_iLoadsThisFrame >= MAX_PREVIEW_LOADS_PER_FRAME)
		return nullptr;
	m_iLoadsThisFrame += RESOURCE_KIND::MODEL == eKind ?
		MAX_PREVIEW_LOADS_PER_FRAME : 1u;

	PREVIEW_ENTRY Staged;
	const std::filesystem::path Path =
		CRuntimeAssetRoot::Resolve(std::filesystem::path(strAssetId));
	std::error_code Error;
	if (Path.empty() || !std::filesystem::is_regular_file(Path, Error))
	{
		Staged.strError = "Missing: " + strAssetId;
	}
	else if (RESOURCE_KIND::MODEL == eKind)
	{
		if (Create_ModelThumbnail(Path, Staged.pTextureView, Staged.strError,
			Staged.strInfo))
		{
			Staged.iWidth = MODEL_THUMBNAIL_SIZE;
			Staged.iHeight = MODEL_THUMBNAIL_SIZE;
		}
	}
	else
	{
		ComPtr<ID3D11Resource> pResource;
		if (FAILED(DirectX::CreateDDSTextureFromFile(
			m_pDevice.Get(), Path.c_str(), &pResource,
			&Staged.pTextureView, MAX_PREVIEW_DIMENSION)))
		{
			Staged.strError = "DDS load failed: " + strAssetId;
		}
		else
		{
			ComPtr<ID3D11Texture2D> pTexture;
			if (SUCCEEDED(pResource.As(&pTexture)))
			{
				D3D11_TEXTURE2D_DESC Desc{};
				pTexture->GetDesc(&Desc);
				Staged.iWidth = Desc.Width;
				Staged.iHeight = Desc.Height;
			}
		}
	}
	return &m_Previews.emplace(strAssetId, std::move(Staged)).first->second;
}

bool_t Client::CEffect_Tool_V2::Create_ModelThumbnail(
	const std::filesystem::path& Path,
	ComPtr<ID3D11ShaderResourceView>& OutTextureView,
	std::string& strOutError,
	std::string& strOutInfo)
{
	unique_ptr<Engine::CModel> Model = Engine::CModel::Create(
		m_pDevice, m_pContext, MODEL::NONANIM,
		Path.string().c_str(), XMMatrixIdentity());
	bool_t bSkinned = false;
	if (nullptr == Model)
	{
		Model = Engine::CModel::Create(
			m_pDevice, m_pContext, MODEL::ANIM,
			Path.string().c_str(), XMMatrixIdentity());
		bSkinned = nullptr != Model;
	}
	if (nullptr == Model)
	{
		strOutError = "WModel load failed: " + Path.string() + "\n" +
			CModelDecoderRegistry::Get().Get_LastReport().error;
		return false;
	}
	float3_t Minimum{};
	float3_t Maximum{};
	if (Model->Has_LocalBounds())
	{
		Minimum = Model->Get_LocalBoundsMin();
		Maximum = Model->Get_LocalBoundsMax();
	}
	else if (!Compute_SkinnedBounds(Path, *Model, Minimum, Maximum, strOutInfo))
	{
		strOutError = "WModel bounds failed: " + Path.string() + "\n" +
			CModelDecoderRegistry::Get().Get_LastReport().error;
		return false;
	}
	{
		char szBounds[192] = {};
		std::snprintf(szBounds, sizeof(szBounds),
			"%s | meshes=%u | min(%.2f %.2f %.2f) max(%.2f %.2f %.2f)",
			bSkinned ? "skinned" : "static", Model->Get_NumMeshes(),
			Minimum.x, Minimum.y, Minimum.z, Maximum.x, Maximum.y, Maximum.z);
		strOutInfo = strOutInfo.empty() ? szBounds : szBounds + ("\n" + strOutInfo);
	}

	shared_ptr<Engine::CShader>& pShader =
		bSkinned ? m_pAnimModelShader : m_pModelShader;
	if (nullptr == pShader)
	{
		unique_ptr<Engine::CShader> Shader = bSkinned ?
			Engine::CShader::Create(m_pDevice, m_pContext,
				TEXT("../Bin/ShaderFiles/Shader_VtxAnimMeshPreview_V2.hlsl"),
				VTXANIMMESH::Elements, VTXANIMMESH::iNumElements) :
			Engine::CShader::Create(m_pDevice, m_pContext,
				TEXT("../Bin/ShaderFiles/Shader_VtxMeshPreview.hlsl"),
				VTXMESH::Elements, VTXMESH::iNumElements);
		if (nullptr == Shader)
		{
			strOutError = bSkinned ?
				"Skinned mesh preview shader creation failed." :
				"Mesh preview shader creation failed.";
			return false;
		}
		pShader = std::move(Shader);
	}

	D3D11_TEXTURE2D_DESC ColorDesc{};
	ColorDesc.Width = MODEL_THUMBNAIL_SIZE;
	ColorDesc.Height = MODEL_THUMBNAIL_SIZE;
	ColorDesc.MipLevels = 1u;
	ColorDesc.ArraySize = 1u;
	ColorDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	ColorDesc.SampleDesc.Count = 1u;
	ColorDesc.Usage = D3D11_USAGE_DEFAULT;
	ColorDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	ComPtr<ID3D11Texture2D> ColorTexture;
	ComPtr<ID3D11RenderTargetView> ColorRTV;
	ComPtr<ID3D11ShaderResourceView> ColorSRV;
	if (FAILED(m_pDevice->CreateTexture2D(&ColorDesc, nullptr, &ColorTexture)) ||
		FAILED(m_pDevice->CreateRenderTargetView(
			ColorTexture.Get(), nullptr, &ColorRTV)) ||
		FAILED(m_pDevice->CreateShaderResourceView(
			ColorTexture.Get(), nullptr, &ColorSRV)))
	{
		strOutError = "Mesh preview color target creation failed.";
		return false;
	}

	D3D11_TEXTURE2D_DESC DepthDesc = ColorDesc;
	DepthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	DepthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	ComPtr<ID3D11Texture2D> DepthTexture;
	ComPtr<ID3D11DepthStencilView> DepthDSV;
	if (FAILED(m_pDevice->CreateTexture2D(&DepthDesc, nullptr, &DepthTexture)) ||
		FAILED(m_pDevice->CreateDepthStencilView(
			DepthTexture.Get(), nullptr, &DepthDSV)))
	{
		strOutError = "Mesh preview depth target creation failed.";
		return false;
	}

	const uint32_t WhitePixel = 0xffffffffu;
	D3D11_TEXTURE2D_DESC WhiteDesc{};
	WhiteDesc.Width = 1u;
	WhiteDesc.Height = 1u;
	WhiteDesc.MipLevels = 1u;
	WhiteDesc.ArraySize = 1u;
	WhiteDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	WhiteDesc.SampleDesc.Count = 1u;
	WhiteDesc.Usage = D3D11_USAGE_IMMUTABLE;
	WhiteDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	D3D11_SUBRESOURCE_DATA WhiteData{};
	WhiteData.pSysMem = &WhitePixel;
	WhiteData.SysMemPitch = sizeof(WhitePixel);
	ComPtr<ID3D11Texture2D> WhiteTexture;
	ComPtr<ID3D11ShaderResourceView> WhiteSRV;
	if (FAILED(m_pDevice->CreateTexture2D(&WhiteDesc, &WhiteData, &WhiteTexture)) ||
		FAILED(m_pDevice->CreateShaderResourceView(
			WhiteTexture.Get(), nullptr, &WhiteSRV)))
	{
		strOutError = "Mesh preview fallback texture creation failed.";
		return false;
	}

	const float3_t Center(
		(Minimum.x + Maximum.x) * 0.5f,
		(Minimum.y + Maximum.y) * 0.5f,
		(Minimum.z + Maximum.z) * 0.5f);
	const float3_t HalfExtent(
		(Maximum.x - Minimum.x) * 0.5f,
		(Maximum.y - Minimum.y) * 0.5f,
		(Maximum.z - Minimum.z) * 0.5f);
	const f32_t Radius = std::sqrt(
		HalfExtent.x * HalfExtent.x +
		HalfExtent.y * HalfExtent.y +
		HalfExtent.z * HalfExtent.z);
	if (!std::isfinite(Radius) || Radius <= 0.0001f)
	{
		strOutError = "Mesh preview bounds are invalid.";
		return false;
	}

	ComPtr<ID3D11RenderTargetView> PreviousRTV;
	ComPtr<ID3D11DepthStencilView> PreviousDSV;
	m_pContext->OMGetRenderTargets(1u, &PreviousRTV, &PreviousDSV);
	std::array<D3D11_VIEWPORT,
		D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE>
		PreviousViewports{};
	uint32_t iPreviousViewportCount =
		static_cast<uint32_t>(PreviousViewports.size());
	m_pContext->RSGetViewports(&iPreviousViewportCount, PreviousViewports.data());

	ID3D11RenderTargetView* pTarget = ColorRTV.Get();
	m_pContext->OMSetRenderTargets(1u, &pTarget, DepthDSV.Get());
	const float ClearColor[4] = { 0.035f, 0.045f, 0.06f, 1.f };
	m_pContext->ClearRenderTargetView(ColorRTV.Get(), ClearColor);
	m_pContext->ClearDepthStencilView(
		DepthDSV.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0u);
	D3D11_VIEWPORT Viewport{};
	Viewport.Width = static_cast<f32_t>(MODEL_THUMBNAIL_SIZE);
	Viewport.Height = static_cast<f32_t>(MODEL_THUMBNAIL_SIZE);
	Viewport.MinDepth = 0.f;
	Viewport.MaxDepth = 1.f;
	m_pContext->RSSetViewports(1u, &Viewport);

	const f32_t Distance = Radius / std::sin(XMConvertToRadians(22.5f));
	const matrix_t WorldMatrix =
		XMMatrixTranslation(-Center.x, -Center.y, -Center.z) *
		XMMatrixRotationRollPitchYaw(
			XMConvertToRadians(18.f), XMConvertToRadians(-32.f), 0.f);
	const vector_t Eye = XMVectorSet(0.f, 0.f, -Distance, 1.f);
	const matrix_t ViewMatrix = XMMatrixLookAtLH(
		Eye, XMVectorZero(), XMVectorSet(0.f, 1.f, 0.f, 0.f));
	const matrix_t ProjectionMatrix = XMMatrixPerspectiveFovLH(
		XMConvertToRadians(45.f), 1.f,
		(std::max)(0.001f, Distance - Radius * 1.25f),
		Distance + Radius * 3.f);
	float4x4_t World{};
	float4x4_t View{};
	float4x4_t Projection{};
	XMStoreFloat4x4(&World, WorldMatrix);
	XMStoreFloat4x4(&View, ViewMatrix);
	XMStoreFloat4x4(&Projection, ProjectionMatrix);
	float3_t CameraPosition{};
	XMStoreFloat3(&CameraPosition, Eye);
	const float3_t LightDirection(-0.45f, -0.75f, 0.35f);

	bool_t bRendered =
		SUCCEEDED(pShader->Bind_Matrix("g_WorldMatrix", &World)) &&
		SUCCEEDED(pShader->Bind_Matrix("g_ViewMatrix", &View)) &&
		SUCCEEDED(pShader->Bind_Matrix("g_ProjMatrix", &Projection)) &&
		SUCCEEDED(pShader->Bind_RawValue(
			"g_CameraPosition", &CameraPosition, sizeof(CameraPosition))) &&
		SUCCEEDED(pShader->Bind_RawValue(
			"g_LightDirection", &LightDirection, sizeof(LightDirection)));
	for (uint32_t iMesh = 0u; bRendered && iMesh < Model->Get_NumMeshes(); ++iMesh)
	{
		const uint32_t iHasNormal =
			Model->Has_MaterialTexture(iMesh, aiTextureType_NORMALS) ? 1u : 0u;
		bRendered = SUCCEEDED(pShader->Bind_RawValue(
			"g_HasNormalTexture", &iHasNormal, sizeof(iHasNormal)));
		if (bRendered && bSkinned)
			bRendered = SUCCEEDED(Model->Bind_BoneMatrices(
				pShader, "g_BoneMatrices", iMesh));
		if (bRendered && Model->Has_MaterialTexture(iMesh, aiTextureType_DIFFUSE))
			bRendered = SUCCEEDED(Model->Bind_Material(
				pShader, "g_DiffuseTexture", iMesh, aiTextureType_DIFFUSE));
		else if (bRendered)
			bRendered = SUCCEEDED(pShader->Bind_Texture(
				"g_DiffuseTexture", WhiteSRV));
		if (bRendered && 0u != iHasNormal)
			bRendered = SUCCEEDED(Model->Bind_Material(
				pShader, "g_NormalTexture", iMesh, aiTextureType_NORMALS));
		bRendered = bRendered &&
			SUCCEEDED(pShader->Begin(0u)) &&
			SUCCEEDED(Model->Render(iMesh));
	}

	ID3D11RenderTargetView* pPreviousTarget = PreviousRTV.Get();
	m_pContext->OMSetRenderTargets(1u, &pPreviousTarget, PreviousDSV.Get());
	if (0u != iPreviousViewportCount)
		m_pContext->RSSetViewports(iPreviousViewportCount, PreviousViewports.data());
	if (!bRendered)
	{
		strOutError = "Mesh preview render failed: " + Path.string();
		return false;
	}
	OutTextureView = std::move(ColorSRV);
	strOutError.clear();
	return true;
}

bool_t Client::CEffect_Tool_V2::Compute_SkinnedBounds(
	const std::filesystem::path& Path,
	const Engine::CModel& Model,
	float3_t& OutMinimum,
	float3_t& OutMaximum,
	std::string& strOutInfo)
{
	MODEL_ASSET_LOAD_DESC Desc{};
	Desc.meshPath = Path.lexically_normal();
	std::error_code Error;
	const std::filesystem::path Absolute =
		std::filesystem::absolute(Desc.meshPath, Error).lexically_normal();
	for (std::filesystem::path Current = Absolute.parent_path();
		!Current.empty() && Current != Current.parent_path();
		Current = Current.parent_path())
	{
		if (L"Resources" == Current.filename())
		{
			Desc.assetRoot = Current;
			break;
		}
	}
	MODEL_ASSET_DATA Asset{};
	if (!CModelDecoderRegistry::Get().Decode(Desc, Asset) || Asset.meshes.empty())
		return false;

	const f32_t fMax = (std::numeric_limits<f32_t>::max)();
	float3_t Minimum(fMax, fMax, fMax);
	float3_t Maximum(-fMax, -fMax, -fMax);
	bool_t bAny = false;
	const auto Include = [&](const float3_t& Position)
	{
		if (!std::isfinite(Position.x) || !std::isfinite(Position.y) ||
			!std::isfinite(Position.z))
			return;
		Minimum.x = (std::min)(Minimum.x, Position.x);
		Minimum.y = (std::min)(Minimum.y, Position.y);
		Minimum.z = (std::min)(Minimum.z, Position.z);
		Maximum.x = (std::max)(Maximum.x, Position.x);
		Maximum.y = (std::max)(Maximum.y, Position.y);
		Maximum.z = (std::max)(Maximum.z, Position.z);
		bAny = true;
	};
	const size_t iBoneCount = Asset.skeleton.bones.size();
	std::vector<matrix_t> SkinMatrices(iBoneCount, XMMatrixIdentity());
	for (size_t iBone = 0u; iBone < iBoneCount; ++iBone)
	{
		matrix_t Combined = XMMatrixIdentity();
		if (!Model.Get_BoneCombinedMatrix(static_cast<uint32_t>(iBone), Combined))
			return false;
		SkinMatrices[iBone] =
			XMLoadFloat4x4(&Asset.skeleton.bones[iBone].inverseBind) * Combined;
	}
	for (const MODEL_MESH_DATA& Mesh : Asset.meshes)
	{
		for (const VTXANIMMESH& Vertex : Mesh.skinnedVertices)
		{
			const uint32_t Indices[4] = {
				Vertex.vBlendIndices.x, Vertex.vBlendIndices.y,
				Vertex.vBlendIndices.z, Vertex.vBlendIndices.w };
			const f32_t Weights[4] = {
				Vertex.vBlendWeights.x, Vertex.vBlendWeights.y,
				Vertex.vBlendWeights.z, Vertex.vBlendWeights.w };
			matrix_t Skin = XMMatrixSet(
				0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f,
				0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f);
			f32_t fTotalWeight = 0.f;
			for (uint32_t iInfluence = 0u; iInfluence < 4u; ++iInfluence)
			{
				if (Weights[iInfluence] <= 0.f || Indices[iInfluence] >= iBoneCount)
					continue;
				Skin += SkinMatrices[Indices[iInfluence]] * Weights[iInfluence];
				fTotalWeight += Weights[iInfluence];
			}
			if (fTotalWeight <= 0.f)
			{
				Include(Vertex.vPosition);
				continue;
			}
			float3_t Position{};
			XMStoreFloat3(&Position, XMVector3TransformCoord(
				XMLoadFloat3(&Vertex.vPosition), Skin));
			Include(Position);
		}
		for (const VTXMESH& Vertex : Mesh.vertices)
			Include(Vertex.vPosition);
	}
	if (!bAny)
		return false;
	OutMinimum = Minimum;
	OutMaximum = Maximum;

	strOutInfo = "bones=" + std::to_string(Asset.skeleton.bones.size()) +
		" anims=" + std::to_string(Asset.animations.size()) +
		" materials=" + std::to_string(Asset.materials.size());
	for (size_t iMesh = 0u; iMesh < Asset.meshes.size(); ++iMesh)
	{
		const MODEL_MESH_DATA& Mesh = Asset.meshes[iMesh];
		strOutInfo += "\n[" + std::to_string(iMesh) + "] " + Mesh.name +
			" v=" + std::to_string(Mesh.skinnedVertices.size() + Mesh.vertices.size()) +
			" mat=" + std::to_string(Mesh.materialIndex);
		if (Mesh.materialIndex < Asset.materials.size())
		{
			strOutInfo += " diffuse=" +
				Asset.materials[Mesh.materialIndex].diffusePath.filename().string();
		}
	}
	return true;
}

void Client::CEffect_Tool_V2::Render_TypeSelector()
{
	ImGui::TextUnformatted("Effect Type");
	for (int32_t iType = 0; iType < static_cast<int32_t>(EFFECT_TYPE::END);
		++iType)
	{
		if (0 != iType)
			ImGui::SameLine();
		const EFFECT_TYPE eType = static_cast<EFFECT_TYPE>(iType);
		if (ImGui::RadioButton(Type_Label(eType), m_eType == eType))
		{
			m_eType = eType;
			m_bVisibleDirty = true;
		}
	}
}

void Client::CEffect_Tool_V2::Render_SlotCards()
{
	ImGui::Text("%s Slots", Type_Label(m_eType));
	SLOT_BINDINGS& Bindings = Current_Bindings();
	bool_t bFirst = true;
	for (int32_t iSlot = 0; iSlot < static_cast<int32_t>(RESOURCE_SLOT::END);
		++iSlot)
	{
		const RESOURCE_SLOT eSlot = static_cast<RESOURCE_SLOT>(iSlot);
		if (!Slot_VisibleForType(eSlot))
			continue;
		std::string& strAssetId = Bindings[static_cast<size_t>(iSlot)];
		if (!bFirst)
			ImGui::SameLine();
		bFirst = false;
		ImGui::PushID(iSlot);
		ImGui::BeginGroup();

		const PREVIEW_ENTRY* pPreview =
			Request_Preview(strAssetId, Slot_Kind(eSlot));
		bool_t bClicked = false;
		if (nullptr != pPreview && nullptr != pPreview->pTextureView)
		{
			bClicked = ImGui::ImageButton("slot",
				pPreview->pTextureView.Get(),
				ImVec2(SLOT_CARD_SIZE, SLOT_CARD_SIZE));
		}
		else
		{
			const char* pLabel = strAssetId.empty() ? "Empty" :
				(nullptr == pPreview ? "Loading" : "Error");
			bClicked = ImGui::Button(pLabel,
				ImVec2(SLOT_CARD_SIZE + 8.f, SLOT_CARD_SIZE + 8.f));
			if (nullptr != pPreview && !pPreview->strError.empty() &&
				ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("%s", pPreview->strError.c_str());
			}
		}
		if (m_eSelectedSlot == eSlot)
		{
			ImGui::GetWindowDrawList()->AddRect(
				ImGui::GetItemRectMin(), ImGui::GetItemRectMax(),
				ImGui::GetColorU32(ImGuiCol_HeaderActive), 2.f, 0, 3.f);
		}
		if (bClicked && m_eSelectedSlot != eSlot)
		{
			m_eSelectedSlot = eSlot;
			m_bVisibleDirty = true;
		}

		ImGui::TextUnformatted(Slot_Label(eSlot));
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("%s", Slot_Description(eSlot));
		if (strAssetId.empty())
		{
			ImGui::TextDisabled("(none)");
		}
		else
		{
			std::string Name =
				std::filesystem::path(strAssetId).filename().string();
			if (Name.size() > 12u)
				Name = Name.substr(0u, 10u) + "..";
			ImGui::TextDisabled("%s", Name.c_str());
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("%s", strAssetId.c_str());
			if (ImGui::SmallButton("Clear"))
				strAssetId.clear();
		}
		ImGui::EndGroup();
		ImGui::PopID();
	}
}

void Client::CEffect_Tool_V2::Render_ResourceBrowser()
{
	const RESOURCE_KIND eKind = Slot_Kind(m_eSelectedSlot);
	ImGui::Text("Resource Library (%s) -> %s / %s",
		RESOURCE_KIND::MODEL == eKind ? "WModel" : "DDS",
		Type_Label(m_eType), Slot_Label(m_eSelectedSlot));

	if (ImGui::Button("Rescan"))
		Scan_Resources();
	ImGui::SameLine();
	ImGui::SetNextItemWidth(180.f);
	if (ImGui::BeginCombo("Domain",
		m_strDomainFilter.empty() ? "All" : m_strDomainFilter.c_str()))
	{
		if (ImGui::Selectable("All", m_strDomainFilter.empty()))
		{
			m_strDomainFilter.clear();
			m_bVisibleDirty = true;
		}
		for (const std::string& Domain : m_Domains)
		{
			if (ImGui::Selectable(Domain.c_str(), m_strDomainFilter == Domain))
			{
				m_strDomainFilter = Domain;
				m_bVisibleDirty = true;
			}
		}
		ImGui::EndCombo();
	}
	ImGui::SameLine();
	ImGui::SetNextItemWidth(200.f);
	if (ImGui::InputTextWithHint("##NameFilter", "filename filter",
		m_szNameFilter, sizeof(m_szNameFilter)))
	{
		m_bVisibleDirty = true;
	}

	if (m_bVisibleDirty || m_eVisibleKind != eKind)
		Rebuild_VisibleResources();
	ImGui::TextDisabled("%zu shown", m_VisibleResources.size());

	if (!ImGui::BeginChild("ResourceGrid", ImVec2(0.f, 0.f),
		ImGuiChildFlags_Borders))
	{
		ImGui::EndChild();
		return;
	}
	const float fTileWidth = BROWSER_TILE_SIZE + 12.f;
	const int32_t iColumns = (std::max)(1,
		static_cast<int32_t>(ImGui::GetContentRegionAvail().x / fTileWidth));
	const int32_t iRows = static_cast<int32_t>(
		(m_VisibleResources.size() + iColumns - 1u) / iColumns);
	std::string& strBoundAssetId = Current_SlotAssetId();

	ImGuiListClipper Clipper;
	Clipper.Begin(iRows, BROWSER_TILE_SIZE + 56.f);
	while (Clipper.Step())
	{
		for (int32_t iRow = Clipper.DisplayStart; iRow < Clipper.DisplayEnd;
			++iRow)
		{
			for (int32_t iColumn = 0; iColumn < iColumns; ++iColumn)
			{
				const size_t iVisible = static_cast<size_t>(
					iRow * iColumns + iColumn);
				if (iVisible >= m_VisibleResources.size())
					break;
				const RESOURCE_ENTRY& Entry =
					m_Resources[m_VisibleResources[iVisible]];
				if (0 != iColumn)
					ImGui::SameLine();
				ImGui::PushID(Entry.strAssetId.c_str());
				ImGui::BeginGroup();

				const PREVIEW_ENTRY* pPreview =
					Request_Preview(Entry.strAssetId, Entry.eKind);
				bool_t bClicked = false;
				if (nullptr != pPreview && nullptr != pPreview->pTextureView)
				{
					bClicked = ImGui::ImageButton("tile",
						pPreview->pTextureView.Get(),
						ImVec2(BROWSER_TILE_SIZE, BROWSER_TILE_SIZE));
				}
				else
				{
					bClicked = ImGui::Button(
						nullptr == pPreview ? "..." :
							(RESOURCE_KIND::MODEL == Entry.eKind ? "Mesh" : "DDS"),
						ImVec2(BROWSER_TILE_SIZE + 8.f, BROWSER_TILE_SIZE + 8.f));
					if (nullptr != pPreview && !pPreview->strError.empty() &&
						ImGui::IsItemHovered())
					{
						ImGui::SetTooltip("%s", pPreview->strError.c_str());
					}
				}
				if (Entry.strAssetId == strBoundAssetId)
				{
					ImGui::GetWindowDrawList()->AddRect(
						ImGui::GetItemRectMin(), ImGui::GetItemRectMax(),
						ImGui::GetColorU32(ImGuiCol_HeaderActive), 2.f, 0, 3.f);
				}
				std::string Name = Entry.strFileName;
				if (Name.size() > 12u)
					Name = Name.substr(0u, 10u) + "..";
				ImGui::TextUnformatted(Name.c_str());
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("%s", Entry.strAssetId.c_str());
				if (nullptr != Entry.pUsage)
				{
					std::string Badge;
					for (int32_t iSlot = static_cast<int32_t>(RESOURCE_SLOT::BASE);
						iSlot < static_cast<int32_t>(RESOURCE_SLOT::END); ++iSlot)
					{
						const uint32_t iCount =
							Entry.pUsage->Counts[static_cast<size_t>(iSlot)];
						if (0u == iCount)
							continue;
						if (!Badge.empty())
							Badge += ' ';
						Badge += Slot_Label(static_cast<RESOURCE_SLOT>(iSlot))[0];
						Badge += std::to_string(iCount);
					}
					ImGui::TextDisabled("%s", Badge.c_str());
					if (ImGui::IsItemHovered())
					{
						std::string Tip = "Source usage (B=Base N=Noise M=Mask E=Emissive D=Dissolve)";
						for (const std::string& Param : Entry.pUsage->Params)
							Tip += "\n  " + Param;
						ImGui::SetTooltip("%s", Tip.c_str());
					}
				}
				ImGui::EndGroup();
				if (bClicked)
				{
					strBoundAssetId = Entry.strAssetId;
					m_strStatus = std::string("Bound ") + Slot_Label(m_eSelectedSlot) +
						" <- " + Entry.strAssetId;
				}
				ImGui::PopID();
			}
		}
	}
	ImGui::EndChild();
}

void Client::CEffect_Tool_V2::Render_PreviewPanel()
{
	ImGui::Text("Preview: %s / %s",
		Type_Label(m_eType), Slot_Label(m_eSelectedSlot));
	const std::string& strAssetId = Current_SlotAssetId();
	if (strAssetId.empty())
	{
		ImGui::TextDisabled("Select a slot card, then click a resource in the library.");
		return;
	}
	const PREVIEW_ENTRY* pPreview =
		Request_Preview(strAssetId, Slot_Kind(m_eSelectedSlot));
	if (nullptr == pPreview)
	{
		ImGui::TextDisabled("Loading...");
		return;
	}
	if (nullptr == pPreview->pTextureView)
	{
		ImGui::TextColored(ImVec4(1.f, 0.4f, 0.4f, 1.f), "%s",
			pPreview->strError.c_str());
		return;
	}
	const float fAvail = (std::max)(64.f,
		(std::min)(PREVIEW_PANEL_SIZE, ImGui::GetContentRegionAvail().x));
	float fWidth = fAvail;
	float fHeight = fAvail;
	if (pPreview->iWidth > 0u && pPreview->iHeight > 0u)
	{
		const float fAspect = static_cast<float>(pPreview->iWidth) /
			static_cast<float>(pPreview->iHeight);
		if (fAspect >= 1.f)
			fHeight = fAvail / fAspect;
		else
			fWidth = fAvail * fAspect;
	}
	ImGui::Image(pPreview->pTextureView.Get(), ImVec2(fWidth, fHeight));
	ImGui::TextWrapped("%s", strAssetId.c_str());
	if (RESOURCE_KIND::MODEL == Slot_Kind(m_eSelectedSlot))
		ImGui::TextDisabled("WModel thumbnail %u px", pPreview->iWidth);
	else
		ImGui::TextDisabled("%u x %u", pPreview->iWidth, pPreview->iHeight);
	if (!pPreview->strInfo.empty())
		ImGui::TextWrapped("%s", pPreview->strInfo.c_str());
}

void Client::CEffect_Tool_V2::Render_CreatePanel()
{
	ImGui::TextUnformatted("World Preview");
	const SLOT_BINDINGS& Bindings = Current_Bindings();
	const bool_t bMeshType =
		EFFECT_TYPE::MESH == m_eType || EFFECT_TYPE::PARTICLE == m_eType;
	const bool_t bSupportedType =
		EFFECT_TYPE::MESH == m_eType || EFFECT_TYPE::TEXTURE == m_eType;
	const bool_t bHasBase =
		!Bindings[static_cast<size_t>(RESOURCE_SLOT::BASE)].empty();
	const bool_t bHasMesh =
		!Bindings[static_cast<size_t>(RESOURCE_SLOT::MESH)].empty();
	const bool_t bCanCreate =
		bSupportedType && bHasBase && (!bMeshType || bHasMesh);
	ImGui::BeginDisabled(!bCanCreate);
	if (ImGui::Button("Create Effect"))
		Try_CreatePreview();
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(m_pPreview.expired());
	if (ImGui::Button("Open Tuning"))
		m_bTuningWindowOpen = true;
	ImGui::EndDisabled();
	ImGui::SameLine();
	if (ImGui::Button("Attach..."))
		m_bAttachWindowOpen = true;
	if (!bSupportedType)
		ImGui::TextDisabled("Only Mesh and Texture types can be previewed yet.");
	else if (!bHasBase)
		ImGui::TextDisabled("Bind a Base texture first.");
	else if (bMeshType && !bHasMesh)
		ImGui::TextDisabled("Bind a Mesh first.");
	if (!m_strPreviewStatus.empty())
		ImGui::TextWrapped("%s", m_strPreviewStatus.c_str());
}

bool_t Client::CEffect_Tool_V2::Try_CreatePreview()
{
	const SLOT_BINDINGS& Bindings = Current_Bindings();
	CEffectV2Object::DESC Desc{};
	Desc.eShape = EFFECT_TYPE::MESH == m_eType ?
		CEffectV2Object::SHAPE::MESH : CEffectV2Object::SHAPE::SPRITE;
	Desc.strMeshAssetId = Bindings[static_cast<size_t>(RESOURCE_SLOT::MESH)];
	for (int32_t iSlot = static_cast<int32_t>(RESOURCE_SLOT::BASE);
		iSlot < static_cast<int32_t>(RESOURCE_SLOT::END); ++iSlot)
	{
		Desc.TextureAssetIds[static_cast<size_t>(
			iSlot - static_cast<int32_t>(RESOURCE_SLOT::BASE))] =
			Bindings[static_cast<size_t>(iSlot)];
	}
	return Spawn_Preview(Desc, {}, std::string());
}

bool_t Client::CEffect_Tool_V2::Spawn_Preview(
	const CEffectV2Object::DESC& SourceDesc,
	const std::vector<PART_OVERRIDE>& Parts,
	const std::string& strAnimationClip)
{
	CGameInstance& GameInstance = CGameInstance::Get();
	if (!m_bPreviewPrototypeRegistered)
	{
		unique_ptr<CEffectV2Object> pPrototype =
			CEffectV2Object::Create(m_pDevice, m_pContext);
		if (nullptr == pPrototype)
		{
			m_strPreviewStatus = "Preview prototype creation failed.";
			return false;
		}
		const HRESULT hResult = GameInstance.Add_Prototype(
			ETOUI(LEVEL::STATIC), L"Prototype_GameObject_EffectPreviewV2",
			std::move(pPrototype));
		if (FAILED(hResult))
		{
			m_strPreviewStatus =
				"Prototype registration returned failure (already registered or STATIC level unavailable); continuing.";
		}
		m_bPreviewPrototypeRegistered = true;
	}
	if (const std::shared_ptr<CEffectV2Object> pPrevious = m_pPreview.lock())
		pPrevious->Set_Hidden(true);

	CEffectV2Object::DESC Desc = SourceDesc;
	const float4_t* pCameraPosition = GameInstance.Get_CamPosition();
	const float4x4_t* pCameraWorld = GameInstance.Get_InverseTransform(D3DTS::VIEW);
	if (nullptr == pCameraPosition || nullptr == pCameraWorld)
	{
		m_strPreviewStatus = "Camera is not available in this level.";
		return false;
	}
	const vector_t Look = XMVector3Normalize(XMLoadFloat4x4(pCameraWorld).r[2]);
	const vector_t Spawn = XMLoadFloat4(pCameraPosition) + Look * 3.f;
	XMStoreFloat4x4(&Desc.PivotWorld, XMMatrixTranslationFromVector(Spawn));

	std::shared_ptr<CGameObject> pGameObject;
	if (FAILED(GameInstance.Add_GameObject_to_Layer(
		ETOUI(LEVEL::STATIC), L"Prototype_GameObject_EffectPreviewV2",
		GameInstance.Get_CurrentLevelID(), L"Layer_EffectPreviewV2",
		&Desc, &pGameObject)))
	{
		m_strPreviewStatus = "Create failed: " +
			(CEffectV2Object::Last_Error().empty() ?
				std::string("prototype clone or layer add failed.") :
				CEffectV2Object::Last_Error());
		return false;
	}
	const std::shared_ptr<CEffectV2Object> pPreview =
		std::dynamic_pointer_cast<CEffectV2Object>(pGameObject);
	if (nullptr == pPreview)
	{
		m_strPreviewStatus = "Create failed: unexpected object type.";
		return false;
	}
	for (uint32_t iPart = 0u;
		iPart < Parts.size() && iPart < pPreview->Part_Count(); ++iPart)
	{
		pPreview->Part_Visible(iPart) = Parts[iPart].bVisible;
		if (!Parts[iPart].strBaseAssetId.empty() &&
			FAILED(pPreview->Set_PartBase(iPart, Parts[iPart].strBaseAssetId)))
		{
			m_strPreviewStatus = "Part texture load failed: " + Parts[iPart].strBaseAssetId;
		}
	}
	if (!strAnimationClip.empty())
	{
		bool_t bFound = false;
		for (uint32_t iClip = 0u; iClip < pPreview->Animation_Count(); ++iClip)
		{
			const char_t* pName = pPreview->Animation_Name(iClip);
			if (nullptr != pName && strAnimationClip == pName)
			{
				pPreview->Params().iAnimationIndex = iClip;
				bFound = true;
				break;
			}
		}
		if (!bFound)
			m_strPreviewStatus = "Animation clip not found in model: " + strAnimationClip;
	}
	m_pPreview = pPreview;
	m_ePreviewType = m_eType;
	m_bTuningWindowOpen = true;
	if (m_strPreviewStatus.empty() || 0 == m_strPreviewStatus.rfind("Pivot", 0))
		m_strPreviewStatus = "Pivot spawned at camera forward 3 m.";
	return true;
}

void Client::CEffect_Tool_V2::Scan_Documents()
{
	m_bDocumentsScanned = true;
	m_Documents.clear();
	std::error_code Error;
	const std::filesystem::path Directory = CEffectV2Document::Document_Directory();
	if (Directory.empty() || !std::filesystem::is_directory(Directory, Error))
		return;
	for (const std::filesystem::directory_entry& Entry :
		std::filesystem::directory_iterator(Directory, Error))
	{
		if (!Entry.is_regular_file(Error))
			continue;
		const std::string strName = Entry.path().filename().string();
		constexpr const char* SUFFIX = ".effectv2.json";
		const size_t iSuffix = std::strlen(SUFFIX);
		if (strName.size() <= iSuffix ||
			strName.compare(strName.size() - iSuffix, iSuffix, SUFFIX) != 0)
			continue;
		m_Documents.push_back(strName.substr(0u, strName.size() - iSuffix));
	}
	std::sort(m_Documents.begin(), m_Documents.end());
}

bool_t Client::CEffect_Tool_V2::Save_Document()
{
	const std::shared_ptr<CEffectV2Object> pPreview = m_pPreview.lock();
	const std::string strEffectId = m_szEffectId;
	if (nullptr == pPreview)
	{
		m_strDocumentStatus = "Create Effect first; Save writes the live preview.";
		return false;
	}
	if (!CEffectV2Document::Is_ValidEffectId(strEffectId))
	{
		m_strDocumentStatus = "Effect ID must be 1-80 chars of [A-Za-z0-9._-].";
		return false;
	}
	EFFECT_V2_DOCUMENT Document;
	Document.strEffectId = strEffectId;
	Document.eType = m_ePreviewType;
	Document.Desc = pPreview->Creation_Desc();
	Document.Desc.Params = pPreview->Params();
	Document.Desc.bParamsAuthored = true;
	const char_t* pClip = pPreview->Animation_Name(pPreview->Params().iAnimationIndex);
	Document.strAnimationClip = nullptr != pClip ? pClip : "";
	for (uint32_t iPart = 0u; iPart < pPreview->Part_Count(); ++iPart)
	{
		EFFECT_V2_PART_OVERRIDE Part;
		Part.bVisible = pPreview->Part_Visible(iPart);
		Part.strBaseAssetId = pPreview->Part_BaseAssetId(iPart);
		Document.Parts.push_back(std::move(Part));
	}
	std::string strError;
	if (!CEffectV2Document::Write_AtomicFile(
		CEffectV2Document::Document_Path(strEffectId),
		CEffectV2Document::Serialize_Document(Document), strError))
	{
		m_strDocumentStatus = strError;
		return false;
	}
	CEffectV2Runtime::Invalidate_Caches();
	m_bDocumentsScanned = false;
	m_strDocumentStatus = "Saved " + strEffectId + ".effectv2.json";
	return true;
}

bool_t Client::CEffect_Tool_V2::Load_Document(const std::string& strEffectId)
{
	EFFECT_V2_DOCUMENT Document;
	std::string strError;
	if (!CEffectV2Document::Load_DocumentFile(strEffectId, Document, strError))
	{
		m_strDocumentStatus = "Load rejected (" + strEffectId + "): " + strError;
		return false;
	}
	const EFFECT_TYPE ePreviousType = m_eType;
	const SLOT_BINDINGS PreviousBindings = m_SlotBindings[static_cast<size_t>(Document.eType)];
	m_eType = Document.eType;
	SLOT_BINDINGS& Bindings = Current_Bindings();
	Bindings[static_cast<size_t>(RESOURCE_SLOT::MESH)] = Document.Desc.strMeshAssetId;
	for (size_t iInput = 0u; iInput < Document.Desc.TextureAssetIds.size(); ++iInput)
	{
		Bindings[static_cast<size_t>(RESOURCE_SLOT::BASE) + iInput] =
			Document.Desc.TextureAssetIds[iInput];
	}
	if (!Spawn_Preview(Document.Desc, Document.Parts, Document.strAnimationClip))
	{
		m_SlotBindings[static_cast<size_t>(Document.eType)] = PreviousBindings;
		m_eType = ePreviousType;
		m_strDocumentStatus = "Load failed (" + strEffectId + "): " + m_strPreviewStatus;
		return false;
	}
	std::snprintf(m_szEffectId, sizeof(m_szEffectId), "%s", strEffectId.c_str());
	m_bVisibleDirty = true;
	m_strDocumentStatus = "Loaded " + strEffectId;
	return true;
}

void Client::CEffect_Tool_V2::Render_DocumentPanel()
{
	if (!m_bDocumentsScanned)
		Scan_Documents();
	ImGui::TextUnformatted("Document (Data/Effects/V2/Authored)");
	ImGui::SetNextItemWidth(-1.f);
	ImGui::InputTextWithHint("##EffectId", "Effect ID (e.g. esther.wei.dochul)",
		m_szEffectId, sizeof(m_szEffectId));
	ImGui::BeginDisabled(m_pPreview.expired() || '\0' == m_szEffectId[0]);
	if (ImGui::Button("Save"))
		Save_Document();
	ImGui::EndDisabled();
	if (m_pPreview.expired())
	{
		ImGui::SameLine();
		ImGui::TextDisabled("(Create Effect first)");
	}
	ImGui::SameLine();
	if (ImGui::Button("Rescan"))
		Scan_Documents();
	if (m_Documents.empty())
		ImGui::TextDisabled("No saved documents.");
	else if (ImGui::BeginListBox("##Documents", ImVec2(-1.f, 88.f)))
	{
		for (const std::string& strDocument : m_Documents)
		{
			const bool_t bSelected = strDocument == m_szEffectId;
			if (ImGui::Selectable(strDocument.c_str(), bSelected))
				std::snprintf(m_szEffectId, sizeof(m_szEffectId), "%s", strDocument.c_str());
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				Load_Document(strDocument);
		}
		ImGui::EndListBox();
	}
	ImGui::BeginDisabled('\0' == m_szEffectId[0]);
	if (ImGui::Button("Load"))
		Load_Document(m_szEffectId);
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::TextDisabled("(double-click a row to load)");
	if (!m_strDocumentStatus.empty())
		ImGui::TextWrapped("%s", m_strDocumentStatus.c_str());
}

namespace
{
	constexpr const wchar_t* TARGET_LAYER_TAG = L"Layer_EffectPreviewV2Target";
	constexpr f32_t BINDING_FRAME_RATE = 30.f;
}

bool_t Client::CEffect_Tool_V2::Collect_BoneNames(
	const std::string& strModelAssetId,
	std::vector<std::string>& OutNames)
{
	OutNames.clear();
	MODEL_ASSET_LOAD_DESC Desc{};
	Desc.meshPath = CRuntimeAssetRoot::Resolve(std::filesystem::path(strModelAssetId));
	if (Desc.meshPath.empty())
		return false;
	Desc.assetRoot = CRuntimeAssetRoot::Get();
	MODEL_ASSET_DATA Asset{};
	if (!CModelDecoderRegistry::Get().Decode(Desc, Asset))
		return false;
	for (const MODEL_BONE_DATA& Bone : Asset.skeleton.bones)
		OutNames.push_back(Bone.name);
	return !OutNames.empty();
}

bool_t Client::CEffect_Tool_V2::Spawn_Target(const std::string& strArchetypeId)
{
	Despawn_Target();
	CGameInstance& GameInstance = CGameInstance::Get();
	const uint32_t iLevel = GameInstance.Get_CurrentLevelID();
	const NPC_ACTOR_ENTRY* pActor = CActorCatalog::Find_Npc(strArchetypeId);
	const wstring_t strModelTag =
		CNpcPresentationAssetService::Get_ModelPrototypeTag(strArchetypeId);
	if (nullptr == pActor || strModelTag.empty())
	{
		m_strAttachStatus = "Unknown NPC archetype: " + strArchetypeId;
		return false;
	}
	if (FAILED(CNpcPresentationAssetService::Ensure_Prototypes(
		m_pDevice, m_pContext, iLevel, strArchetypeId)))
	{
		m_strAttachStatus = "NPC presentation prototypes failed: " + strArchetypeId;
		return false;
	}

	float3_t vPosition{ 0.f, 0.f, 0.f };
	if (const std::shared_ptr<CCharacter> pCharacter =
		CAnimationTargetService::Resolve_SceneCharacter();
		nullptr != pCharacter && nullptr != pCharacter->Get_Transform())
	{
		XMStoreFloat3(&vPosition,
			pCharacter->Get_Transform()->Get_State(STATE::POSITION) +
			XMVectorSet(2.5f, 0.f, 0.f, 0.f));
	}

	CNpc::NPC_DESC Desc{};
	Desc.iPrototypeLevelIndex = iLevel;
	Desc.strModelTag = strModelTag;
	Desc.strShaderTag = pActor->shaderProfile == "esther" ?
		TEXT("Prototype_Component_Shader_VtxEstherNpc") :
		TEXT("Prototype_Component_Shader_VtxAnimMeshBinary");
	Desc.pIdleClip = pActor->idleClip.c_str();
	Desc.isLoop = true;
	Desc.vPosition = vPosition;
	Desc.fYawDegree = 0.f;
	Desc.fCollisionRadius = 0.f;
	std::shared_ptr<CGameObject> pGameObject;
	if (FAILED(GameInstance.Add_GameObject_to_Layer(
		iLevel, TEXT("Prototype_GameObject_Npc"), iLevel, TARGET_LAYER_TAG,
		&Desc, &pGameObject)))
	{
		m_strAttachStatus = "Target spawn failed: " + strArchetypeId;
		return false;
	}
	const std::shared_ptr<CNpc> pNpc = std::dynamic_pointer_cast<CNpc>(pGameObject);
	if (nullptr == pNpc || nullptr == pNpc->Get_Model())
	{
		GameInstance.Remove_GameObject_from_Layer(iLevel, TARGET_LAYER_TAG, pGameObject);
		m_strAttachStatus = "Target spawn returned an unexpected object.";
		return false;
	}
	CEffectV2Runtime::Set_Ignored(pNpc, !m_bRuntimeOnTarget);
	m_pTarget = pNpc;
	m_strTargetArchetypeId = strArchetypeId;
	m_vTargetPosition = vPosition;
	m_fTargetYawDegrees = 0.f;
	m_fTargetLastClipSeconds = -1.f;

	std::vector<std::string> BoneNames;
	Collect_BoneNames(pActor->modelAssetId, BoneNames);
	m_TargetBoneNames.clear();
	for (const std::string& strBone : BoneNames)
	{
		if (pNpc->Get_Model()->Has_Bone(strBone.c_str()))
			m_TargetBoneNames.push_back(strBone);
	}
	if (m_strPivotBone.empty() || !pNpc->Get_Model()->Has_Bone(m_strPivotBone.c_str()))
	{
		m_strPivotBone = pNpc->Get_Model()->Has_Bone("b_effectroot") ? "b_effectroot" :
			(m_TargetBoneNames.empty() ? std::string() : m_TargetBoneNames.front());
	}

	const auto Strike = pActor->actionClips.find("esther.strike");
	if (Strike != pActor->actionClips.end() && !Strike->second.empty())
		pNpc->Set_Animation(Strike->second.front().c_str(), m_bTargetClipLoop);

	Load_Bindings(strArchetypeId);
	m_strAttachStatus = "Target " + strArchetypeId + " spawned beside the scene character.";
	return true;
}

void Client::CEffect_Tool_V2::Despawn_Target()
{
	if (const std::shared_ptr<CEffectV2Object> pPreview = m_pPreview.lock())
		pPreview->Clear_FollowTarget();
	m_ePivotMode = PIVOT_MODE::WORLD;
	if (const std::shared_ptr<CNpc> pNpc = m_pTarget.lock())
	{
		CEffectV2Runtime::Set_Ignored(pNpc, false);
		CGameInstance::Get().Remove_GameObject_from_Layer(
			CGameInstance::Get().Get_CurrentLevelID(), TARGET_LAYER_TAG, pNpc);
	}
	m_pTarget.reset();
	m_TargetBoneNames.clear();
}

void Client::CEffect_Tool_V2::Move_Target(const float3_t& vPosition, const f32_t fYawDegrees)
{
	const std::shared_ptr<CNpc> pNpc = m_pTarget.lock();
	if (nullptr == pNpc || !pNpc->Apply_NetworkState(vPosition, fYawDegrees))
		return;
	m_vTargetPosition = vPosition;
	m_fTargetYawDegrees = fYawDegrees;
}

void Client::CEffect_Tool_V2::Update_Attach(const f32_t fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);
	const std::shared_ptr<CNpc> pNpc = m_pTarget.lock();
	const std::shared_ptr<CEffectV2Object> pPreview = m_pPreview.lock();
	if (nullptr != pPreview)
	{
		if (PIVOT_MODE::TARGET_BONE == m_ePivotMode && nullptr != pNpc &&
			!m_strPivotBone.empty())
		{
			pPreview->Set_FollowTarget(pNpc, m_strPivotBone, m_ePivotRotation);
		}
		else if (pPreview->Has_FollowTarget())
			pPreview->Clear_FollowTarget();
	}
	if (nullptr == pNpc || nullptr == pNpc->Get_Model())
		return;
	const std::shared_ptr<Engine::CModel> pModel = pNpc->Get_Model();

	f32_t fPosition = 0.f;
	f32_t fDuration = 0.f;
	const uint32_t iClip = pModel->Get_CurrentAnimIndex();
	const f32_t fTickPerSecond = pModel->Get_AnimationTickPerSecond(iClip);
	if (fTickPerSecond <= 0.f || !pModel->Get_AnimationProgress(iClip, fPosition, fDuration))
		return;
	const f32_t fSeconds = fPosition / fTickPerSecond;
	const f32_t fSpawnSeconds = static_cast<f32_t>(m_iSpawnFrame) / BINDING_FRAME_RATE;
	if (nullptr != pPreview && !pModel->Is_AnimPaused() &&
		m_fTargetLastClipSeconds >= 0.f && m_fTargetLastClipSeconds < fSpawnSeconds &&
		fSeconds >= fSpawnSeconds)
	{
		pPreview->Restart();
	}
	if (fSeconds < m_fTargetLastClipSeconds && nullptr != pPreview && 0 == m_iSpawnFrame)
		pPreview->Restart();
	m_fTargetLastClipSeconds = fSeconds;
}

bool_t Client::CEffect_Tool_V2::Load_Bindings(const std::string& strArchetypeId)
{
	m_Bindings.clear();
	std::error_code Error;
	const std::filesystem::path Target = CEffectV2Document::Binding_Path(strArchetypeId);
	if (Target.empty() || !std::filesystem::is_regular_file(Target, Error))
	{
		m_strAttachStatus = "No bindings yet for " + strArchetypeId + ".";
		return true;
	}
	std::string strError;
	std::vector<EFFECT_BINDING> Bindings;
	if (!CEffectV2Document::Load_BindingsFile(strArchetypeId, Bindings, strError))
	{
		m_strAttachStatus = "Bindings rejected (" + strArchetypeId + "): " + strError;
		return false;
	}
	m_Bindings = std::move(Bindings);
	m_strAttachStatus = "Loaded " + std::to_string(m_Bindings.size()) +
		" binding(s) for " + strArchetypeId + ".";
	return true;
}

bool_t Client::CEffect_Tool_V2::Save_Bindings()
{
	if (m_strTargetArchetypeId.empty())
	{
		m_strAttachStatus = "Spawn a target first.";
		return false;
	}
	std::string strError;
	if (!CEffectV2Document::Write_AtomicFile(
		CEffectV2Document::Binding_Path(m_strTargetArchetypeId),
		CEffectV2Document::Serialize_Bindings(m_strTargetArchetypeId, m_Bindings), strError))
	{
		m_strAttachStatus = strError;
		return false;
	}
	CEffectV2Runtime::Invalidate_Caches();
	m_strAttachStatus = "Saved " + m_strTargetArchetypeId + ".effectv2bindings.json";
	return true;
}

void Client::CEffect_Tool_V2::Render_AttachWindow()
{
	if (!m_bAttachWindowOpen)
		return;
	ImGui::SetNextWindowSize(ImVec2(460.f, 640.f), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("Effect Attach v2", &m_bAttachWindowOpen))
	{
		ImGui::End();
		return;
	}
	const std::shared_ptr<CNpc> pNpc = m_pTarget.lock();
	const std::shared_ptr<Engine::CModel> pModel =
		nullptr != pNpc ? pNpc->Get_Model() : nullptr;

	ImGui::SeparatorText("Target (NPC archetype)");
	const std::vector<NPC_ACTOR_ENTRY>& Npcs = CActorCatalog::Get_Npcs();
	const char* pSelectedLabel =
		m_iTargetArchetypeSelection >= 0 &&
		m_iTargetArchetypeSelection < static_cast<int32_t>(Npcs.size()) ?
		Npcs[static_cast<size_t>(m_iTargetArchetypeSelection)].archetypeId.c_str() : "(select)";
	if (ImGui::BeginCombo("Archetype", pSelectedLabel))
	{
		for (int32_t iIndex = 0; iIndex < static_cast<int32_t>(Npcs.size()); ++iIndex)
		{
			const NPC_ACTOR_ENTRY& Entry = Npcs[static_cast<size_t>(iIndex)];
			if (Entry.runtimeStatus != "supported")
				continue;
			const std::string strLabel = Entry.archetypeId + "  (" +
				std::filesystem::path(Entry.modelAssetId).stem().string() + ")";
			if (ImGui::Selectable(strLabel.c_str(), iIndex == m_iTargetArchetypeSelection))
				m_iTargetArchetypeSelection = iIndex;
		}
		ImGui::EndCombo();
	}
	ImGui::BeginDisabled(m_iTargetArchetypeSelection < 0);
	if (ImGui::Button("Spawn Target"))
		Spawn_Target(Npcs[static_cast<size_t>(m_iTargetArchetypeSelection)].archetypeId);
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(nullptr == pNpc);
	if (ImGui::Button("Despawn"))
		Despawn_Target();
	ImGui::EndDisabled();
	if (nullptr != pNpc)
	{
		ImGui::Text("Live: %s", m_strTargetArchetypeId.c_str());
		if (ImGui::Checkbox("Runtime spawns on target", &m_bRuntimeOnTarget))
		{
			CEffectV2Runtime::Set_Ignored(pNpc, !m_bRuntimeOnTarget);
			if (m_bRuntimeOnTarget && nullptr != pModel)
			{
				const char_t* pClip = pModel->Get_AnimationName(pModel->Get_CurrentAnimIndex());
				if (nullptr != pClip)
					CEffectV2Runtime::Notify_NpcClip(pNpc, pClip);
			}
		}
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Let CEffectV2Runtime apply the saved bindings to this tool target (in-game behaviour check). Hide the preview to avoid doubles.");
		float3_t vPosition = m_vTargetPosition;
		f32_t fYaw = m_fTargetYawDegrees;
		if (ImGui::DragFloat3("Target Position", &vPosition.x, 0.05f))
			Move_Target(vPosition, fYaw);
		if (ImGui::DragFloat("Target Yaw (deg)", &fYaw, 1.f, -360.f, 360.f))
			Move_Target(vPosition, fYaw);
		if (ImGui::Button("Beside Character"))
		{
			if (const std::shared_ptr<CCharacter> pCharacter =
				CAnimationTargetService::Resolve_SceneCharacter();
				nullptr != pCharacter && nullptr != pCharacter->Get_Transform())
			{
				XMStoreFloat3(&vPosition,
					pCharacter->Get_Transform()->Get_State(STATE::POSITION) +
					XMVectorSet(2.5f, 0.f, 0.f, 0.f));
				Move_Target(vPosition, fYaw);
			}
		}
	}

	if (nullptr != pModel && 0u < pModel->Get_NumAnimations())
	{
		ImGui::SeparatorText("Target Playback");
		const uint32_t iCurrent = pModel->Get_CurrentAnimIndex();
		const char_t* pCurrentName = pModel->Get_AnimationName(iCurrent);
		if (ImGui::BeginCombo("Target Clip", nullptr != pCurrentName ? pCurrentName : "(none)"))
		{
			for (uint32_t iClip = 0u; iClip < pModel->Get_NumAnimations(); ++iClip)
			{
				const char_t* pName = pModel->Get_AnimationName(iClip);
				if (nullptr == pName)
					continue;
				if (ImGui::Selectable(pName, iClip == iCurrent))
				{
					pNpc->Set_Animation(pName, m_bTargetClipLoop);
					pModel->Set_AnimTrackPosition(iClip, 0.f);
					m_fTargetLastClipSeconds = -1.f;
				}
			}
			ImGui::EndCombo();
		}
		f32_t fPosition = 0.f;
		f32_t fDuration = 0.f;
		const bool_t bHasTrack =
			pModel->Get_AnimationProgress(iCurrent, fPosition, fDuration) && fDuration > 0.f;
		const f32_t fTickPerSecond = pModel->Get_AnimationTickPerSecond(iCurrent);
		if (bHasTrack)
		{
			f32_t fScrub = fPosition;
			char_t szFormat[64]{};
			std::snprintf(szFormat, sizeof(szFormat), "frame %%.1f / %.0f", fDuration);
			ImGui::SetNextItemWidth(-1.f);
			if (ImGui::SliderFloat("##TargetScrub", &fScrub, 0.f, fDuration, szFormat))
			{
				pModel->Set_AnimPaused(true);
				pModel->Set_AnimTrackPosition(iCurrent, fScrub);
			}
			if (fTickPerSecond > 0.f)
				ImGui::TextDisabled("%.2f / %.2f s", fPosition / fTickPerSecond, fDuration / fTickPerSecond);
		}
		const bool_t bPaused = pModel->Is_AnimPaused();
		if (ImGui::Button(bPaused ? "Play" : "Pause"))
			pModel->Set_AnimPaused(!bPaused);
		ImGui::SameLine();
		ImGui::BeginDisabled(!bHasTrack);
		if (ImGui::Button("< Frame"))
		{
			pModel->Set_AnimPaused(true);
			pModel->Set_AnimTrackPosition(iCurrent, (std::max)(0.f, fPosition - 1.f));
		}
		ImGui::SameLine();
		if (ImGui::Button("Frame >"))
		{
			pModel->Set_AnimPaused(true);
			pModel->Set_AnimTrackPosition(iCurrent, (std::min)(fDuration, fPosition + 1.f));
		}
		ImGui::EndDisabled();
		ImGui::SameLine();
		if (ImGui::Button("Restart Clip"))
		{
			pModel->Set_AnimTrackPosition(iCurrent, 0.f);
			m_fTargetLastClipSeconds = -1.f;
			if (const std::shared_ptr<CEffectV2Object> pPreview = m_pPreview.lock())
				pPreview->Restart();
		}
		ImGui::SameLine();
		if (ImGui::Checkbox("Loop", &m_bTargetClipLoop))
			pModel->Set_Animation(iCurrent, m_bTargetClipLoop);
	}

	ImGui::SeparatorText("Effect Pivot");
	int32_t iMode = static_cast<int32_t>(m_ePivotMode);
	if (ImGui::Combo("Pivot Mode", &iMode, "World\0Target Bone\0"))
		m_ePivotMode = static_cast<PIVOT_MODE>(iMode);
	ImGui::BeginDisabled(m_TargetBoneNames.empty());
	if (ImGui::BeginCombo("Pivot Bone", m_strPivotBone.empty() ? "(none)" : m_strPivotBone.c_str()))
	{
		for (const std::string& strBone : m_TargetBoneNames)
		{
			if (ImGui::Selectable(strBone.c_str(), strBone == m_strPivotBone))
				m_strPivotBone = strBone;
		}
		ImGui::EndCombo();
	}
	ImGui::EndDisabled();
	int32_t iRotation = static_cast<int32_t>(m_ePivotRotation);
	if (ImGui::Combo("Pivot Rotation", &iRotation, "Bone\0Target Yaw\0World\0"))
		m_ePivotRotation = static_cast<PIVOT_ROTATION>(iRotation);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Bone: inherit bone orientation. Target Yaw: bone position + target facing (default). World: bone position only.");
	if (PIVOT_MODE::TARGET_BONE == m_ePivotMode && nullptr == pNpc)
		ImGui::TextDisabled("Spawn a target to follow a bone.");
	ImGui::InputInt("Spawn Frame (30 fps)", &m_iSpawnFrame);
	m_iSpawnFrame = (std::max)(0, (std::min)(m_iSpawnFrame, 18000));
	ImGui::TextDisabled("Effect restarts when the target clip passes the spawn frame (= %u ms).",
		static_cast<uint32_t>(static_cast<f32_t>(m_iSpawnFrame) * 1000.f / BINDING_FRAME_RATE));

	ImGui::SeparatorText("Bindings (Data/Effects/V2/Bindings)");
	const char_t* pClipForBinding =
		nullptr != pModel ? pModel->Get_AnimationName(pModel->Get_CurrentAnimIndex()) : nullptr;
	ImGui::Text("Effect: %s | Clip: %s | Bone: %s",
		'\0' == m_szEffectId[0] ? "(none)" : m_szEffectId,
		nullptr != pClipForBinding ? pClipForBinding : "(none)",
		m_strPivotBone.empty() ? "(none)" : m_strPivotBone.c_str());
	ImGui::BeginDisabled('\0' == m_szEffectId[0] || nullptr == pClipForBinding || nullptr == pNpc);
	if (ImGui::Button("Add / Update Binding"))
	{
		EFFECT_BINDING Binding;
		Binding.strEffectId = m_szEffectId;
		Binding.strClip = pClipForBinding;
		Binding.iStartMs = static_cast<uint32_t>(
			static_cast<f32_t>(m_iSpawnFrame) * 1000.f / BINDING_FRAME_RATE);
		Binding.strBone = PIVOT_MODE::TARGET_BONE == m_ePivotMode ? m_strPivotBone : std::string();
		Binding.bFollowBone = PIVOT_MODE::TARGET_BONE == m_ePivotMode;
		Binding.eRotation = m_ePivotRotation;
		bool_t bReplaced = false;
		for (EFFECT_BINDING& Existing : m_Bindings)
		{
			if (Existing.strEffectId == Binding.strEffectId && Existing.strClip == Binding.strClip)
			{
				Binding.bStopWithClip = Existing.bStopWithClip;
				Existing = Binding;
				bReplaced = true;
				break;
			}
		}
		if (!bReplaced)
			m_Bindings.push_back(Binding);
		m_strAttachStatus = bReplaced ? "Binding updated (unsaved)." : "Binding added (unsaved).";
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(m_strTargetArchetypeId.empty());
	if (ImGui::Button("Save Bindings"))
		Save_Bindings();
	ImGui::SameLine();
	if (ImGui::Button("Reload"))
		Load_Bindings(m_strTargetArchetypeId);
	ImGui::EndDisabled();
	if (m_Bindings.empty())
		ImGui::TextDisabled("No bindings.");
	for (size_t iIndex = 0u; iIndex < m_Bindings.size(); ++iIndex)
	{
		EFFECT_BINDING& Binding = m_Bindings[iIndex];
		ImGui::PushID(static_cast<int32_t>(iIndex));
		char_t szRow[256]{};
		std::snprintf(szRow, sizeof(szRow), "%s @ %s +%ums %s%s",
			Binding.strEffectId.c_str(), Binding.strClip.c_str(), Binding.iStartMs,
			Binding.strBone.empty() ? "(world)" : Binding.strBone.c_str(),
			Binding.bFollowBone ? "" : " [no follow]");
		if (ImGui::Selectable(szRow, false))
		{
			std::snprintf(m_szEffectId, sizeof(m_szEffectId), "%s", Binding.strEffectId.c_str());
			m_iSpawnFrame = static_cast<int32_t>(
				static_cast<f32_t>(Binding.iStartMs) * BINDING_FRAME_RATE / 1000.f + 0.5f);
			m_ePivotMode = Binding.strBone.empty() ? PIVOT_MODE::WORLD : PIVOT_MODE::TARGET_BONE;
			m_ePivotRotation = Binding.eRotation;
			if (!Binding.strBone.empty())
				m_strPivotBone = Binding.strBone;
			if (nullptr != pNpc)
				pNpc->Set_Animation(Binding.strClip.c_str(), m_bTargetClipLoop);
		}
		ImGui::SameLine();
		ImGui::Checkbox("Stop w/ clip", &Binding.bStopWithClip);
		ImGui::SameLine();
		if (ImGui::SmallButton("Remove"))
		{
			m_Bindings.erase(m_Bindings.begin() + static_cast<std::ptrdiff_t>(iIndex));
			ImGui::PopID();
			break;
		}
		ImGui::PopID();
	}
	if (!m_strAttachStatus.empty())
		ImGui::TextWrapped("%s", m_strAttachStatus.c_str());
	ImGui::End();
}

namespace
{
	void Draw_LerpTrack(
		const char* szLabel,
		Client::CEffectV2Object::LERP_FLOAT3& Track,
		const float fSpeed,
		const float fMin,
		const float fMax)
	{
		ImGui::PushID(szLabel);
		ImGui::Checkbox("##Lerp", &Track.bLerp);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Interpolate Start -> End over Lifetime.");
		ImGui::SameLine();
		ImGui::DragFloat3(szLabel, &Track.vStart.x, fSpeed, fMin, fMax);
		if (Track.bLerp)
		{
			ImGui::Indent(24.f);
			ImGui::DragFloat3("End", &Track.vEnd.x, fSpeed, fMin, fMax);
			ImGui::Unindent(24.f);
		}
		ImGui::PopID();
	}
}

void Client::CEffect_Tool_V2::Render_TuningPanel()
{
	if (!m_bTuningWindowOpen)
		return;
	ImGui::SetNextWindowSize(ImVec2(420.f, 720.f), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("Effect Tuning v2", &m_bTuningWindowOpen))
	{
		ImGui::End();
		return;
	}
	const std::shared_ptr<CEffectV2Object> pPreview = m_pPreview.lock();
	if (nullptr == pPreview)
	{
		ImGui::TextDisabled("No live preview. Create Effect to spawn one.");
		ImGui::End();
		return;
	}
	CEffectV2Object::PARAMS& P = pPreview->Params();
	ImGui::Text("%s | %.2fs | life %.2f | %s",
		CEffectV2Object::SHAPE::MESH == pPreview->Shape() ? "Mesh" : "Sprite",
		pPreview->Time(), pPreview->Life_Ratio(), pPreview->Status().c_str());
	if (ImGui::Button("Restart"))
		pPreview->Restart();
	ImGui::SameLine();
	bool_t bVisible = !pPreview->Is_Hidden();
	if (ImGui::Checkbox("Visible", &bVisible))
		pPreview->Set_Hidden(!bVisible);
	ImGui::SameLine();
	if (ImGui::Button("Bring To Camera"))
	{
		CGameInstance& GameInstance = CGameInstance::Get();
		const float4_t* pCameraPosition = GameInstance.Get_CamPosition();
		const float4x4_t* pCameraWorld = GameInstance.Get_InverseTransform(D3DTS::VIEW);
		m_ePivotMode = PIVOT_MODE::WORLD;
		if (nullptr != pCameraPosition && nullptr != pCameraWorld)
		{
			const vector_t Look = XMVector3Normalize(XMLoadFloat4x4(pCameraWorld).r[2]);
			XMStoreFloat4x4(&pPreview->PivotWorld(), XMMatrixTranslationFromVector(
				XMLoadFloat4(pCameraPosition) + Look * 3.f));
		}
	}
	const bool_t bLifetimeKnown = P.fLifetime > 0.f;
	const bool_t bAnyLerp =
		P.Position.bLerp || P.Rotation.bLerp || P.Scale.bLerp || P.Velocity.bLerp;
	if (!bLifetimeKnown && (bAnyLerp || pPreview->Has_Texture(CEffectV2Object::TEXTURE_INPUT::DISSOLVE)))
		ImGui::TextColored(ImVec4(1.f, 0.8f, 0.3f, 1.f),
			"Lifetime is 0: Lerp tracks and Dissolve Start stay at their start values.");

	ImGui::SeparatorText("Transform (relative to pivot)");
	if (PIVOT_MODE::TARGET_BONE == m_ePivotMode)
	{
		ImGui::TextDisabled("Pivot follows target bone '%s' (Attach window). World pivot is read-only.",
			m_strPivotBone.c_str());
	}
	ImGui::BeginDisabled(PIVOT_MODE::TARGET_BONE == m_ePivotMode);
	ImGui::DragFloat3("Pivot (world)", &pPreview->PivotWorld()._41, 0.05f);
	ImGui::EndDisabled();
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Parent pivot. Position/Rotation/Scale/Velocity below are relative to it.");
	Draw_LerpTrack("Position", P.Position, 0.05f, -1000.f, 1000.f);
	Draw_LerpTrack("Rotation (deg)", P.Rotation, 1.f, -3600.f, 3600.f);
	Draw_LerpTrack("Scale", P.Scale, 0.01f, 0.001f, 1000.f);
	Draw_LerpTrack("Velocity (m/s)", P.Velocity, 0.05f, -1000.f, 1000.f);
	if (CEffectV2Object::SHAPE::MESH == pPreview->Shape())
	{
		ImGui::DragFloat("Mesh Pre-Scale", &P.fMeshPreScale, 0.00005f, 0.00001f, 10.f, "%.5f");
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("WModel unit conversion applied before Scale. Static FX meshes are cm (0.01 = metres); NPC-pipeline skinned cooks carry a x100 armature node, so 0.0001 = metres.");
		ImGui::SameLine();
		if (ImGui::SmallButton("1"))
			P.fMeshPreScale = 1.f;
		ImGui::SameLine();
		if (ImGui::SmallButton("0.01"))
			P.fMeshPreScale = 0.01f;
		ImGui::SameLine();
		if (ImGui::SmallButton("0.0001"))
			P.fMeshPreScale = 0.0001f;
	}

	if (pPreview->Is_Skinned())
	{
		ImGui::SeparatorText("Animation");
		const uint32_t iClipCount = pPreview->Animation_Count();
		if (0u == iClipCount)
			ImGui::TextDisabled("Skinned mesh without clips (bind pose only).");
		else
		{
			const char_t* pCurrentName = pPreview->Animation_Name(P.iAnimationIndex);
			char_t szCurrent[160]{};
			snprintf(szCurrent, sizeof(szCurrent), "[%u] %s",
				P.iAnimationIndex, nullptr != pCurrentName ? pCurrentName : "(none)");
			if (ImGui::BeginCombo("Clip", szCurrent))
			{
				for (uint32_t iClip = 0u; iClip < iClipCount; ++iClip)
				{
					const char_t* pName = pPreview->Animation_Name(iClip);
					char_t szItem[160]{};
					snprintf(szItem, sizeof(szItem), "[%u] %s (%.2fs)",
						iClip, nullptr != pName ? pName : "(none)",
						pPreview->Animation_DurationSeconds(iClip));
					const bool_t bSelected = iClip == P.iAnimationIndex;
					if (ImGui::Selectable(szItem, bSelected))
						P.iAnimationIndex = iClip;
					if (bSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			ImGui::Checkbox("Clip Loop", &P.bAnimationLoop);
			ImGui::SameLine();
			if (ImGui::Button("Lifetime = Clip Length"))
				P.fLifetime = pPreview->Animation_DurationSeconds(P.iAnimationIndex);
			f32_t fClipSeconds = 0.f;
			f32_t fClipDuration = 0.f;
			if (pPreview->Animation_Progress(fClipSeconds, fClipDuration))
				ImGui::TextDisabled("Clip %.2f / %.2f s (Play Rate applies)", fClipSeconds, fClipDuration);
		}
	}

	ImGui::SeparatorText("Color");
	ImGui::ColorEdit4("Color Mul", &P.vColorMul.x, ImGuiColorEditFlags_Float);
	ImGui::DragFloat4("Color Offset", &P.vColorOffset.x, 0.01f, -1.f, 1.f);
	int32_t iClipChannel = static_cast<int32_t>(P.eColorClipChannel);
	ImGui::SetNextItemWidth(90.f);
	if (ImGui::Combo("##ClipChannel", &iClipChannel, "RGB\0Alpha\0"))
		P.eColorClipChannel = static_cast<CEffectV2Object::COLOR_CLIP_CHANNEL>(iClipChannel);
	ImGui::SameLine();
	ImGui::SliderFloat("Color Clip", &P.fColorClip, 0.f, 1.f);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Pixels whose max(RGB) or A is <= this value are discarded. 0 = off.");

	if (CEffectV2Object::SHAPE::MESH == pPreview->Shape())
	{
		ImGui::SeparatorText("Rim (Fresnel)");
		ImGui::ColorEdit3("Rim Color", &P.vRimColor.x, ImGuiColorEditFlags_Float);
		ImGui::DragFloat("Rim Power", &P.fRimPower, 0.05f, 0.1f, 16.f);
		ImGui::DragFloat("Rim Intensity", &P.fRimIntensity, 0.05f, 0.f, 8.f);
		ImGui::SliderFloat("Ghost Alpha", &P.fGhostAlpha, 0.f, 1.f);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("0 = alpha untouched, 1 = alpha multiplied by the fresnel term (only silhouettes remain).");
	}

	ImGui::SeparatorText("Bloom / Distortion");
	ImGui::BeginDisabled(!pPreview->Has_Texture(CEffectV2Object::TEXTURE_INPUT::EMISSIVE));
	ImGui::DragFloat("Bloom Intensity", &P.fBloomIntensity, 0.05f, 0.f, 32.f);
	ImGui::EndDisabled();
	if (!pPreview->Has_Texture(CEffectV2Object::TEXTURE_INPUT::EMISSIVE))
		ImGui::TextDisabled("Bind an Emissive texture to use Bloom Intensity.");
	ImGui::BeginDisabled(!pPreview->Has_Texture(CEffectV2Object::TEXTURE_INPUT::NOISE));
	ImGui::DragFloat("Distortion Intensity", &P.fDistortionIntensity, 0.001f, 0.f, 0.1f, "%.3f");
	ImGui::DragFloat("Noise Strength", &P.fNoiseStrength, 0.005f, 0.f, 2.f);
	ImGui::DragFloat("Noise Scale", &P.fNoiseScale, 0.01f, 0.01f, 64.f);
	ImGui::DragFloat2("Noise Pan (uv/s)", &P.vNoisePan.x, 0.01f, -10.f, 10.f);
	ImGui::EndDisabled();
	if (!pPreview->Has_Texture(CEffectV2Object::TEXTURE_INPUT::NOISE))
		ImGui::TextDisabled("Bind a Noise texture to use Distortion and Noise.");

	ImGui::SeparatorText("UV");
	ImGui::DragFloat2("UV Start", &P.vUVStart.x, 0.01f, -10.f, 10.f);
	ImGui::DragFloat2("UV Speed (uv/s)", &P.vUVSpeed.x, 0.01f, -10.f, 10.f);
	ImGui::DragFloat2("UV TileCount", &P.vUVTileCount.x, 0.01f, 0.01f, 64.f);

	ImGui::SeparatorText("Dissolve");
	ImGui::BeginDisabled(!pPreview->Has_Texture(CEffectV2Object::TEXTURE_INPUT::DISSOLVE));
	ImGui::SliderFloat("Dissolve Start (life 0-1)", &P.fDissolveStart, 0.f, 1.f);
	ImGui::SliderFloat("Dissolve Softness", &P.fDissolveSoftness, 0.f, 0.5f);
	ImGui::EndDisabled();
	if (pPreview->Has_Texture(CEffectV2Object::TEXTURE_INPUT::DISSOLVE))
		ImGui::TextDisabled("Dissolve amount now %.2f", pPreview->Dissolve_Amount());
	else
		ImGui::TextDisabled("Bind a Dissolve texture to use Dissolve Start.");

	if (CEffectV2Object::SHAPE::MESH == pPreview->Shape() && 0u < pPreview->Part_Count())
	{
		ImGui::SeparatorText("Parts");
		ImGui::TextDisabled("Base slot now: %s",
			Current_Bindings()[static_cast<size_t>(RESOURCE_SLOT::BASE)].empty() ?
			"(none)" :
			std::filesystem::path(Current_Bindings()[static_cast<size_t>(RESOURCE_SLOT::BASE)])
				.filename().string().c_str());
		for (uint32_t iPart = 0u; iPart < pPreview->Part_Count(); ++iPart)
		{
			ImGui::PushID(static_cast<int32_t>(iPart));
			ImGui::Checkbox("##Visible", &pPreview->Part_Visible(iPart));
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Visible");
			ImGui::SameLine();
			const std::string& strOverride = pPreview->Part_BaseAssetId(iPart);
			ImGui::Text("#%u %s", iPart, pPreview->Part_Name(iPart).c_str());
			ImGui::SameLine();
			ImGui::TextDisabled("%s", strOverride.empty() ? "(shared Base)" :
				std::filesystem::path(strOverride).filename().string().c_str());
			ImGui::SameLine();
			ImGui::BeginDisabled(
				Current_Bindings()[static_cast<size_t>(RESOURCE_SLOT::BASE)].empty());
			if (ImGui::SmallButton("<- Base slot"))
			{
				if (FAILED(pPreview->Set_PartBase(iPart,
					Current_Bindings()[static_cast<size_t>(RESOURCE_SLOT::BASE)])))
				{
					m_strPreviewStatus = "Part texture load failed.";
				}
			}
			ImGui::EndDisabled();
			if (!strOverride.empty())
			{
				ImGui::SameLine();
				if (ImGui::SmallButton("Clear"))
					pPreview->Set_PartBase(iPart, std::string());
			}
			ImGui::PopID();
		}
	}

	ImGui::SeparatorText("Blend");
	int32_t iBlend = static_cast<int32_t>(P.eBlend);
	if (ImGui::Combo("Blend", &iBlend, "Alpha\0Additive\0Opaque\0"))
		P.eBlend = static_cast<CEffectV2Object::BLEND_MODE>(iBlend);
	ImGui::BeginDisabled(CEffectV2Object::BLEND_MODE::SOLID == P.eBlend);
	ImGui::Checkbox("Depth Test", &P.bDepthTest);
	ImGui::EndDisabled();
	if (CEffectV2Object::SHAPE::SPRITE == pPreview->Shape())
	{
		ImGui::SameLine();
		ImGui::Checkbox("Billboard", &P.bBillboard);
	}

	ImGui::SeparatorText("Playback");
	ImGui::DragFloat("Lifetime (s, 0 = infinite)", &P.fLifetime, 0.05f, 0.f, 600.f);
	ImGui::Checkbox("Loop", &P.bLoop);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(120.f);
	ImGui::DragFloat("Play Rate", &P.fPlayRate, 0.01f, 0.f, 16.f);
	if (pPreview->Is_Finished())
		ImGui::TextDisabled("Finished (lifetime reached). Restart to replay.");
	ImGui::End();
}

Client::CEffect_Tool_V2::SLOT_BINDINGS& Client::CEffect_Tool_V2::Current_Bindings()
{
	return m_SlotBindings[static_cast<size_t>(m_eType)];
}

std::string& Client::CEffect_Tool_V2::Current_SlotAssetId()
{
	return Current_Bindings()[static_cast<size_t>(m_eSelectedSlot)];
}

bool_t Client::CEffect_Tool_V2::Slot_VisibleForType(const RESOURCE_SLOT eSlot) const
{
	if (RESOURCE_SLOT::MESH != eSlot)
		return true;
	return EFFECT_TYPE::MESH == m_eType || EFFECT_TYPE::PARTICLE == m_eType;
}

Client::CEffect_Tool_V2::RESOURCE_KIND Client::CEffect_Tool_V2::Slot_Kind(
	const RESOURCE_SLOT eSlot)
{
	return RESOURCE_SLOT::MESH == eSlot ?
		RESOURCE_KIND::MODEL : RESOURCE_KIND::TEXTURE;
}

const char* Client::CEffect_Tool_V2::Type_Label(const EFFECT_TYPE eType)
{
	switch (eType)
	{
	case EFFECT_TYPE::MESH: return "Mesh";
	case EFFECT_TYPE::TEXTURE: return "Texture";
	case EFFECT_TYPE::PARTICLE: return "Particle";
	case EFFECT_TYPE::DECAL: return "Decal";
	case EFFECT_TYPE::TRAIL: return "Trail";
	default: return "Unknown";
	}
}

const char* Client::CEffect_Tool_V2::Slot_Label(const RESOURCE_SLOT eSlot)
{
	switch (eSlot)
	{
	case RESOURCE_SLOT::MESH: return "Mesh";
	case RESOURCE_SLOT::BASE: return "Base";
	case RESOURCE_SLOT::NOISE: return "Noise";
	case RESOURCE_SLOT::MASK: return "Mask";
	case RESOURCE_SLOT::EMISSIVE: return "Emissive";
	case RESOURCE_SLOT::DISSOLVE: return "Dissolve";
	default: return "Unknown";
	}
}

const char* Client::CEffect_Tool_V2::Slot_Description(const RESOURCE_SLOT eSlot)
{
	switch (eSlot)
	{
	case RESOURCE_SLOT::MESH: return "Mesh: one WModel carrier shape.";
	case RESOURCE_SLOT::BASE: return "Base: RGB color, A opacity.";
	case RESOURCE_SLOT::NOISE: return "Noise: UV distortion source.";
	case RESOURCE_SLOT::MASK: return "Mask: R channel multiplies opacity.";
	case RESOURCE_SLOT::EMISSIVE: return "Emissive: RGB added as glow.";
	case RESOURCE_SLOT::DISSOLVE: return "Dissolve: R channel threshold over lifetime.";
	default: return "";
	}
}
```

### Client/Public/EffectV2_Object.h

```cpp
#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

NS_BEGIN(Engine)
class CModel;
class CShader;
class CVIBuffer_Rect;
NS_END

NS_BEGIN(Client)

class CNpc;

class CEffectV2Object final : public CGameObject
{
public:
	enum class PIVOT_ROTATION : int32_t
	{
		BONE,
		TARGET_YAW,
		WORLD,
		END
	};

	enum class SHAPE : int32_t
	{
		MESH,
		SPRITE,
		END
	};

	enum class BLEND_MODE : int32_t
	{
		ALPHA,
		ADDITIVE,
		SOLID,
		END
	};

	enum class TEXTURE_INPUT : int32_t
	{
		BASE,
		NOISE,
		MASK,
		EMISSIVE,
		DISSOLVE,
		END
	};

	enum class COLOR_CLIP_CHANNEL : int32_t
	{
		RGB,
		ALPHA,
		END
	};

	struct LERP_FLOAT3 final
	{
		float3_t vStart = { 0.f, 0.f, 0.f };
		float3_t vEnd = { 0.f, 0.f, 0.f };
		bool_t bLerp = false;
		float3_t Evaluate(f32_t fLifeRatio) const;
	};

	struct PARAMS final
	{
		LERP_FLOAT3 Position;
		LERP_FLOAT3 Rotation;
		LERP_FLOAT3 Scale = { { 1.f, 1.f, 1.f }, { 1.f, 1.f, 1.f }, false };
		LERP_FLOAT3 Velocity;
		float4_t vColorOffset = { 0.f, 0.f, 0.f, 0.f };
		float4_t vColorMul = { 1.f, 1.f, 1.f, 1.f };
		COLOR_CLIP_CHANNEL eColorClipChannel = COLOR_CLIP_CHANNEL::ALPHA;
		f32_t fColorClip = 0.f;
		float4_t vRimColor = { 1.f, 1.f, 1.f, 1.f };
		f32_t fRimPower = 3.f;
		f32_t fRimIntensity = 0.f;
		f32_t fGhostAlpha = 0.f;
		f32_t fBloomIntensity = 1.f;
		f32_t fDistortionIntensity = 0.f;
		float2_t vUVStart = { 0.f, 0.f };
		float2_t vUVSpeed = { 0.f, 0.f };
		float2_t vUVTileCount = { 1.f, 1.f };
		f32_t fNoiseStrength = 0.f;
		f32_t fNoiseScale = 1.f;
		float2_t vNoisePan = { 0.f, 0.f };
		f32_t fDissolveStart = 0.f;
		f32_t fDissolveSoftness = 0.1f;
		BLEND_MODE eBlend = BLEND_MODE::ADDITIVE;
		bool_t bBillboard = true;
		bool_t bDepthTest = true;
		f32_t fLifetime = 0.f;
		bool_t bLoop = true;
		f32_t fPlayRate = 1.f;
		f32_t fMeshPreScale = 0.01f;
		uint32_t iAnimationIndex = 0u;
		bool_t bAnimationLoop = true;
	};

	struct PART final
	{
		bool_t bVisible = true;
		std::string strBaseAssetId;
		ComPtr<ID3D11ShaderResourceView> pBaseView;
	};

	struct DESC final : public GAMEOBJECT_DESC
	{
		SHAPE eShape = SHAPE::SPRITE;
		std::string strMeshAssetId;
		std::array<std::string, static_cast<size_t>(TEXTURE_INPUT::END)> TextureAssetIds;
		float4x4_t PivotWorld = {
			1.f, 0.f, 0.f, 0.f,
			0.f, 1.f, 0.f, 0.f,
			0.f, 0.f, 1.f, 0.f,
			0.f, 0.f, 0.f, 1.f };
		PARAMS Params;
		bool_t bParamsAuthored = false;
	};

private:
	CEffectV2Object(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CEffectV2Object();

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual void Late_Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

	PARAMS& Params() { return m_Params; }
	const DESC& Creation_Desc() const { return m_CreationDesc; }
	float4x4_t& PivotWorld() { return m_PivotWorld; }
	const std::string& Status() const { return m_strStatus; }
	SHAPE Shape() const { return m_eShape; }
	f32_t Time() const { return m_fTime; }
	f32_t Life_Ratio() const;
	f32_t Dissolve_Amount() const;
	bool_t Has_Texture(const TEXTURE_INPUT eInput) const
	{
		return nullptr != m_Textures[static_cast<size_t>(eInput)];
	}
	uint32_t Part_Count() const { return static_cast<uint32_t>(m_Parts.size()); }
	const std::string& Part_Name(uint32_t iIndex) const;
	bool_t& Part_Visible(const uint32_t iIndex) { return m_Parts[iIndex].bVisible; }
	const std::string& Part_BaseAssetId(const uint32_t iIndex) const
	{
		return m_Parts[iIndex].strBaseAssetId;
	}
	HRESULT Set_PartBase(uint32_t iIndex, const std::string& strAssetId);
	bool_t Is_Skinned() const { return m_bSkinned; }
	uint32_t Animation_Count() const;
	const char_t* Animation_Name(uint32_t iIndex) const;
	f32_t Animation_DurationSeconds(uint32_t iIndex) const;
	bool_t Animation_Progress(f32_t& fOutSeconds, f32_t& fOutDurationSeconds) const;
	bool_t Is_Finished() const { return m_bFinished; }
	void Finish() { m_bFinished = true; }
	bool_t Is_Hidden() const { return m_bHidden; }
	void Set_Hidden(const bool_t bHidden) { m_bHidden = bHidden; }
	void Restart();
	static HRESULT Prewarm(
		const ComPtr<ID3D11Device>& pDevice,
		const ComPtr<ID3D11DeviceContext>& pContext,
		const DESC& Desc,
		std::string& strOutError);
	static void Clear_ResourceCache();
	void Set_FollowTarget(
		const std::weak_ptr<CNpc>& pTarget,
		std::string strBone,
		PIVOT_ROTATION eRotation);
	void Clear_FollowTarget();
	bool_t Has_FollowTarget() const { return m_bFollowTarget; }
	static bool_t Resolve_TargetPivot(
		const CNpc& Npc,
		const std::string& strBone,
		PIVOT_ROTATION eRotation,
		float4x4_t& OutPivot);
	static const std::string& Last_Error() { return s_strLastError; }

private:
	HRESULT Load_Texture(
		const std::string& strAssetId, ComPtr<ID3D11ShaderResourceView>& OutView);
	static HRESULT Acquire_Model(
		const ComPtr<ID3D11Device>& pDevice,
		const ComPtr<ID3D11DeviceContext>& pContext,
		const std::string& strAssetId,
		shared_ptr<Engine::CModel>& OutModel,
		bool_t& bOutSkinned,
		std::string& strOutError);
	static HRESULT Acquire_Shader(
		const ComPtr<ID3D11Device>& pDevice,
		const ComPtr<ID3D11DeviceContext>& pContext,
		const wstring_t& strFilePath,
		const D3D11_INPUT_ELEMENT_DESC* pElements,
		uint32_t iNumElements,
		shared_ptr<Engine::CShader>& OutShader);
	static HRESULT Acquire_Texture(
		const ComPtr<ID3D11Device>& pDevice,
		const std::string& strAssetId,
		ComPtr<ID3D11ShaderResourceView>& OutView);
	void Apply_Transform();
	void Sync_Animation(bool_t bRestart);
	HRESULT Bind_Common(const shared_ptr<Engine::CShader>& pShader);

private:
	SHAPE m_eShape = SHAPE::SPRITE;
	PARAMS m_Params;
	DESC m_CreationDesc;
	float4x4_t m_PivotWorld;
	float3_t m_vDisplacement = { 0.f, 0.f, 0.f };
	uint32_t m_iAppliedAnimationIndex = UINT32_MAX;
	std::vector<PART> m_Parts;
	bool_t m_bFollowTarget = false;
	std::weak_ptr<CNpc> m_pFollowTarget;
	std::string m_strFollowBone;
	PIVOT_ROTATION m_eFollowRotation = PIVOT_ROTATION::TARGET_YAW;
	f32_t m_fTime = 0.f;
	bool_t m_bFinished = false;
	bool_t m_bHidden = false;
	bool_t m_bSkinned = false;
	std::string m_strStatus;

	shared_ptr<Engine::CShader> m_pShader;
	shared_ptr<Engine::CModel> m_pModel;
	shared_ptr<Engine::CVIBuffer_Rect> m_pRect;
	std::array<ComPtr<ID3D11ShaderResourceView>,
		static_cast<size_t>(TEXTURE_INPUT::END)> m_Textures;
	static std::string s_strLastError;

public:
	static unique_ptr<CEffectV2Object> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
```

### Client/Private/EffectV2_Object.cpp

```cpp
#include "EffectV2_Object.h"
#include "BinaryAsset/ModelDecoderRegistry.h"
#include "GameInstance.h"
#include "Model.h"
#include "Npc.h"
#include "RuntimeAssetRoot.h"
#include "Shader.h"
#include "VIBuffer_Rect.h"

#include "DirectXTK/DDSTextureLoader.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <map>
#include <unordered_map>

namespace
{
	constexpr const char* TEXTURE_CONSTANTS[] = {
		"g_BaseTexture", "g_NoiseTexture", "g_MaskTexture",
		"g_EmissiveTexture", "g_DissolveTexture"
	};
	constexpr const char* TEXTURE_FLAG_CONSTANTS[] = {
		"g_HasBase", "g_HasNoise", "g_HasMask", "g_HasEmissive", "g_HasDissolve"
	};

	f32_t Saturate(const f32_t fValue)
	{
		return (std::min)(1.f, (std::max)(0.f, fValue));
	}
}

float3_t Client::CEffectV2Object::LERP_FLOAT3::Evaluate(const f32_t fLifeRatio) const
{
	if (!bLerp)
		return vStart;
	float3_t vResult;
	XMStoreFloat3(&vResult, XMVectorLerp(
		XMLoadFloat3(&vStart), XMLoadFloat3(&vEnd), fLifeRatio));
	return vResult;
}

Client::CEffectV2Object::CEffectV2Object(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CGameObject(std::move(pDevice), std::move(pContext))
{
	XMStoreFloat4x4(&m_PivotWorld, XMMatrixIdentity());
}

Client::CEffectV2Object::~CEffectV2Object() = default;

HRESULT Client::CEffectV2Object::Initialize_Prototype()
{
	return S_OK;
}

std::string Client::CEffectV2Object::s_strLastError;

HRESULT Client::CEffectV2Object::Initialize(void* pArg)
{
	const auto Fail = [this](std::string strReason)
	{
		m_strStatus = std::move(strReason);
		s_strLastError = m_strStatus;
		return E_FAIL;
	};
	s_strLastError.clear();
	if (nullptr == pArg)
		return Fail("Preview desc is null.");
	if (FAILED(__super::Initialize(pArg)))
		return Fail("Transform component creation failed.");
	const DESC& Desc = *static_cast<const DESC*>(pArg);
	m_CreationDesc = Desc;
	m_eShape = Desc.eShape;
	m_Params = Desc.Params;
	m_PivotWorld = Desc.PivotWorld;

	if (SHAPE::MESH == m_eShape)
	{
		std::string strError;
		if (FAILED(Acquire_Model(m_pDevice, m_pContext, Desc.strMeshAssetId,
			m_pModel, m_bSkinned, strError)))
			return Fail(strError);
		m_Parts.assign(m_pModel->Get_NumMeshes(), PART{});
		if (m_bSkinned && !Desc.bParamsAuthored)
			m_Params.fMeshPreScale = 0.0001f;
		const HRESULT hShader = m_bSkinned ?
			Acquire_Shader(m_pDevice, m_pContext,
				TEXT("../Bin/ShaderFiles/Shader_EffectAnimMeshV2.hlsl"),
				VTXANIMMESH::Elements, VTXANIMMESH::iNumElements, m_pShader) :
			Acquire_Shader(m_pDevice, m_pContext,
				TEXT("../Bin/ShaderFiles/Shader_EffectMeshV2.hlsl"),
				VTXMESH::Elements, VTXMESH::iNumElements, m_pShader);
		if (FAILED(hShader))
		{
			return Fail(m_bSkinned ?
				"Shader_EffectAnimMeshV2.hlsl compile failed." :
				"Shader_EffectMeshV2.hlsl compile failed.");
		}
	}
	else
	{
		unique_ptr<Engine::CVIBuffer_Rect> Rect =
			Engine::CVIBuffer_Rect::Create(m_pDevice, m_pContext);
		if (nullptr == Rect)
			return Fail("Rect buffer creation failed.");
		m_pRect = std::move(Rect);
		if (FAILED(Acquire_Shader(m_pDevice, m_pContext,
			TEXT("../Bin/ShaderFiles/Shader_EffectRectV2.hlsl"),
			VTXTEX::Elements, VTXTEX::iNumElements, m_pShader)))
			return Fail("Shader_EffectRectV2.hlsl compile failed.");
	}

	for (size_t iInput = 0u; iInput < m_Textures.size(); ++iInput)
	{
		const std::string& strAssetId = Desc.TextureAssetIds[iInput];
		if (strAssetId.empty())
			continue;
		if (FAILED(Load_Texture(strAssetId, m_Textures[iInput])))
			return Fail("Texture load failed: " + strAssetId);
	}
	Sync_Animation(true);
	m_strStatus = "Ready";
	Apply_Transform();
	return S_OK;
}

HRESULT Client::CEffectV2Object::Load_Texture(
	const std::string& strAssetId,
	ComPtr<ID3D11ShaderResourceView>& OutView)
{
	return Acquire_Texture(m_pDevice, strAssetId, OutView);
}

namespace
{
	struct MODEL_CACHE_ENTRY final
	{
		shared_ptr<Engine::CModel> pPrototype;
		bool_t bSkinned = false;
	};
	std::unordered_map<std::string, MODEL_CACHE_ENTRY> g_ModelCache;
	std::map<wstring_t, shared_ptr<Engine::CShader>> g_ShaderCache;
	std::unordered_map<std::string, ComPtr<ID3D11ShaderResourceView>> g_TextureCache;
}

HRESULT Client::CEffectV2Object::Acquire_Model(
	const ComPtr<ID3D11Device>& pDevice,
	const ComPtr<ID3D11DeviceContext>& pContext,
	const std::string& strAssetId,
	shared_ptr<Engine::CModel>& OutModel,
	bool_t& bOutSkinned,
	std::string& strOutError)
{
	auto Found = g_ModelCache.find(strAssetId);
	if (Found == g_ModelCache.end())
	{
		const std::filesystem::path MeshPath =
			CRuntimeAssetRoot::Resolve(std::filesystem::path(strAssetId));
		if (MeshPath.empty() || !std::filesystem::is_regular_file(MeshPath))
		{
			strOutError = "Mesh asset is missing: " + strAssetId;
			return E_FAIL;
		}
		MODEL_CACHE_ENTRY Entry;
		unique_ptr<Engine::CModel> Model = Engine::CModel::Create(
			pDevice, pContext, MODEL::NONANIM,
			MeshPath.string().c_str(), XMMatrixIdentity());
		if (nullptr == Model)
		{
			Model = Engine::CModel::Create(
				pDevice, pContext, MODEL::ANIM,
				MeshPath.string().c_str(), XMMatrixIdentity());
			Entry.bSkinned = nullptr != Model;
		}
		if (nullptr == Model)
		{
			strOutError = "Mesh load failed: " + strAssetId + " | " +
				CModelDecoderRegistry::Get().Get_LastReport().error;
			return E_FAIL;
		}
		Entry.pPrototype = std::move(Model);
		Found = g_ModelCache.emplace(strAssetId, std::move(Entry)).first;
	}
	const shared_ptr<Engine::CModel> pClone =
		std::static_pointer_cast<Engine::CModel>(Found->second.pPrototype->Clone(nullptr));
	if (nullptr == pClone)
	{
		strOutError = "Mesh clone failed: " + strAssetId;
		return E_FAIL;
	}
	OutModel = pClone;
	bOutSkinned = Found->second.bSkinned;
	return S_OK;
}

HRESULT Client::CEffectV2Object::Acquire_Shader(
	const ComPtr<ID3D11Device>& pDevice,
	const ComPtr<ID3D11DeviceContext>& pContext,
	const wstring_t& strFilePath,
	const D3D11_INPUT_ELEMENT_DESC* pElements,
	const uint32_t iNumElements,
	shared_ptr<Engine::CShader>& OutShader)
{
	auto Found = g_ShaderCache.find(strFilePath);
	if (Found == g_ShaderCache.end())
	{
		unique_ptr<Engine::CShader> Shader = Engine::CShader::Create(
			pDevice, pContext, strFilePath.c_str(), pElements, iNumElements);
		if (nullptr == Shader)
			return E_FAIL;
		Found = g_ShaderCache.emplace(strFilePath, std::move(Shader)).first;
	}
	OutShader = Found->second;
	return S_OK;
}

HRESULT Client::CEffectV2Object::Acquire_Texture(
	const ComPtr<ID3D11Device>& pDevice,
	const std::string& strAssetId,
	ComPtr<ID3D11ShaderResourceView>& OutView)
{
	auto Found = g_TextureCache.find(strAssetId);
	if (Found == g_TextureCache.end())
	{
		const std::filesystem::path Path =
			CRuntimeAssetRoot::Resolve(std::filesystem::path(strAssetId));
		if (Path.empty() || !std::filesystem::is_regular_file(Path))
			return E_FAIL;
		ComPtr<ID3D11ShaderResourceView> pView;
		if (FAILED(DirectX::CreateDDSTextureFromFile(
			pDevice.Get(), Path.c_str(), nullptr, &pView)))
			return E_FAIL;
		Found = g_TextureCache.emplace(strAssetId, std::move(pView)).first;
	}
	OutView = Found->second;
	return S_OK;
}

HRESULT Client::CEffectV2Object::Prewarm(
	const ComPtr<ID3D11Device>& pDevice,
	const ComPtr<ID3D11DeviceContext>& pContext,
	const DESC& Desc,
	std::string& strOutError)
{
	if (SHAPE::MESH == Desc.eShape)
	{
		shared_ptr<Engine::CModel> pModel;
		bool_t bSkinned = false;
		if (FAILED(Acquire_Model(pDevice, pContext, Desc.strMeshAssetId, pModel, bSkinned, strOutError)))
			return E_FAIL;
		shared_ptr<Engine::CShader> pShader;
		const HRESULT hShader = bSkinned ?
			Acquire_Shader(pDevice, pContext,
				TEXT("../Bin/ShaderFiles/Shader_EffectAnimMeshV2.hlsl"),
				VTXANIMMESH::Elements, VTXANIMMESH::iNumElements, pShader) :
			Acquire_Shader(pDevice, pContext,
				TEXT("../Bin/ShaderFiles/Shader_EffectMeshV2.hlsl"),
				VTXMESH::Elements, VTXMESH::iNumElements, pShader);
		if (FAILED(hShader))
		{
			strOutError = "Effect mesh shader compile failed.";
			return E_FAIL;
		}
	}
	else
	{
		shared_ptr<Engine::CShader> pShader;
		if (FAILED(Acquire_Shader(pDevice, pContext,
			TEXT("../Bin/ShaderFiles/Shader_EffectRectV2.hlsl"),
			VTXTEX::Elements, VTXTEX::iNumElements, pShader)))
		{
			strOutError = "Shader_EffectRectV2.hlsl compile failed.";
			return E_FAIL;
		}
	}
	for (const std::string& strAssetId : Desc.TextureAssetIds)
	{
		if (strAssetId.empty())
			continue;
		ComPtr<ID3D11ShaderResourceView> pView;
		if (FAILED(Acquire_Texture(pDevice, strAssetId, pView)))
		{
			strOutError = "Texture load failed: " + strAssetId;
			return E_FAIL;
		}
	}
	return S_OK;
}

void Client::CEffectV2Object::Clear_ResourceCache()
{
	g_ModelCache.clear();
	g_ShaderCache.clear();
	g_TextureCache.clear();
}

const std::string& Client::CEffectV2Object::Part_Name(const uint32_t iIndex) const
{
	static const std::string strEmpty;
	if (nullptr == m_pModel || iIndex >= m_Parts.size())
		return strEmpty;
	return m_pModel->Get_MaterialName(iIndex);
}

HRESULT Client::CEffectV2Object::Set_PartBase(
	const uint32_t iIndex, const std::string& strAssetId)
{
	if (iIndex >= m_Parts.size())
		return E_INVALIDARG;
	PART& Part = m_Parts[iIndex];
	if (strAssetId.empty())
	{
		Part.strBaseAssetId.clear();
		Part.pBaseView.Reset();
		return S_OK;
	}
	ComPtr<ID3D11ShaderResourceView> pView;
	if (FAILED(Load_Texture(strAssetId, pView)))
		return E_FAIL;
	Part.strBaseAssetId = strAssetId;
	Part.pBaseView = std::move(pView);
	return S_OK;
}

void Client::CEffectV2Object::Restart()
{
	m_fTime = 0.f;
	m_vDisplacement = { 0.f, 0.f, 0.f };
	m_bFinished = false;
	Sync_Animation(true);
}

uint32_t Client::CEffectV2Object::Animation_Count() const
{
	if (!m_bSkinned || nullptr == m_pModel)
		return 0u;
	return m_pModel->Get_NumAnimations();
}

const char_t* Client::CEffectV2Object::Animation_Name(const uint32_t iIndex) const
{
	if (iIndex >= Animation_Count())
		return nullptr;
	return m_pModel->Get_AnimationName(iIndex);
}

f32_t Client::CEffectV2Object::Animation_DurationSeconds(const uint32_t iIndex) const
{
	if (iIndex >= Animation_Count())
		return 0.f;
	f32_t fPosition = 0.f;
	f32_t fDuration = 0.f;
	const f32_t fTickPerSecond = m_pModel->Get_AnimationTickPerSecond(iIndex);
	if (fTickPerSecond <= 0.f ||
		!m_pModel->Get_AnimationProgress(iIndex, fPosition, fDuration))
		return 0.f;
	return fDuration / fTickPerSecond;
}

bool_t Client::CEffectV2Object::Animation_Progress(
	f32_t& fOutSeconds, f32_t& fOutDurationSeconds) const
{
	const uint32_t iIndex = m_Params.iAnimationIndex;
	if (iIndex >= Animation_Count())
		return false;
	f32_t fPosition = 0.f;
	f32_t fDuration = 0.f;
	const f32_t fTickPerSecond = m_pModel->Get_AnimationTickPerSecond(iIndex);
	if (fTickPerSecond <= 0.f ||
		!m_pModel->Get_AnimationProgress(iIndex, fPosition, fDuration))
		return false;
	fOutSeconds = fPosition / fTickPerSecond;
	fOutDurationSeconds = fDuration / fTickPerSecond;
	return true;
}

void Client::CEffectV2Object::Sync_Animation(const bool_t bRestart)
{
	const uint32_t iCount = Animation_Count();
	if (0u == iCount)
		return;
	if (m_Params.iAnimationIndex >= iCount)
		m_Params.iAnimationIndex = iCount - 1u;
	const uint32_t iIndex = m_Params.iAnimationIndex;
	if (bRestart || iIndex != m_iAppliedAnimationIndex)
	{
		m_pModel->Start_Animation(iIndex, m_Params.bAnimationLoop);
		m_iAppliedAnimationIndex = iIndex;
		return;
	}
	m_pModel->Set_Animation(iIndex, m_Params.bAnimationLoop);
}

f32_t Client::CEffectV2Object::Life_Ratio() const
{
	if (m_Params.fLifetime <= 0.f)
		return 0.f;
	return Saturate(m_fTime / m_Params.fLifetime);
}

f32_t Client::CEffectV2Object::Dissolve_Amount() const
{
	const f32_t fStart = Saturate(m_Params.fDissolveStart);
	if (fStart >= 1.f)
		return 0.f;
	return Saturate((Life_Ratio() - fStart) / (1.f - fStart));
}

void Client::CEffectV2Object::Set_FollowTarget(
	const std::weak_ptr<CNpc>& pTarget,
	std::string strBone,
	const PIVOT_ROTATION eRotation)
{
	m_pFollowTarget = pTarget;
	m_strFollowBone = std::move(strBone);
	m_eFollowRotation = eRotation;
	m_bFollowTarget = !m_pFollowTarget.expired();
}

void Client::CEffectV2Object::Clear_FollowTarget()
{
	m_bFollowTarget = false;
	m_pFollowTarget.reset();
	m_strFollowBone.clear();
}

bool_t Client::CEffectV2Object::Resolve_TargetPivot(
	const CNpc& Npc,
	const std::string& strBone,
	const PIVOT_ROTATION eRotation,
	float4x4_t& OutPivot)
{
	const shared_ptr<Engine::CModel> pModel = Npc.Get_Model();
	const shared_ptr<Engine::CTransform> pTransform = Npc.Get_Transform();
	if (nullptr == pModel || nullptr == pTransform)
		return false;
	const matrix_t TargetWorld = XMLoadFloat4x4(pTransform->Get_WorldMatrixPtr());
	matrix_t Pivot = TargetWorld;
	if (!strBone.empty())
	{
		if (!pModel->Has_Bone(strBone.c_str()))
			return false;
		Pivot = pModel->Get_BoneMatrix(strBone.c_str()) * TargetWorld;
	}
	const vector_t Translation = XMVectorSetW(Pivot.r[3], 1.f);
	if (PIVOT_ROTATION::BONE == eRotation)
	{
		const vector_t Right = XMVector3Normalize(Pivot.r[0]);
		const vector_t Up = XMVector3Normalize(Pivot.r[1]);
		const vector_t Look = XMVector3Normalize(Pivot.r[2]);
		if (XMVectorGetX(XMVector3LengthSq(Right)) > 0.f &&
			XMVectorGetX(XMVector3LengthSq(Up)) > 0.f &&
			XMVectorGetX(XMVector3LengthSq(Look)) > 0.f)
		{
			Pivot.r[0] = Right;
			Pivot.r[1] = Up;
			Pivot.r[2] = Look;
		}
		else
			Pivot = XMMatrixIdentity();
	}
	else if (PIVOT_ROTATION::TARGET_YAW == eRotation)
	{
		const vector_t Right = XMVector3Normalize(TargetWorld.r[0]);
		const vector_t Up = XMVector3Normalize(TargetWorld.r[1]);
		const vector_t Look = XMVector3Normalize(TargetWorld.r[2]);
		Pivot.r[0] = Right;
		Pivot.r[1] = Up;
		Pivot.r[2] = Look;
	}
	else
		Pivot = XMMatrixIdentity();
	Pivot.r[3] = Translation;
	XMStoreFloat4x4(&OutPivot, Pivot);
	return true;
}

void Client::CEffectV2Object::Update(const f32_t fTimeDelta)
{
	if (m_bFollowTarget)
	{
		const std::shared_ptr<CNpc> pTarget = m_pFollowTarget.lock();
		if (nullptr == pTarget ||
			!Resolve_TargetPivot(*pTarget, m_strFollowBone, m_eFollowRotation, m_PivotWorld))
		{
			m_bFinished = true;
		}
	}
	if (!m_bFinished)
	{
		const f32_t fStep = fTimeDelta * m_Params.fPlayRate;
		const float3_t vVelocity = m_Params.Velocity.Evaluate(Life_Ratio());
		m_vDisplacement.x += vVelocity.x * fStep;
		m_vDisplacement.y += vVelocity.y * fStep;
		m_vDisplacement.z += vVelocity.z * fStep;
		m_fTime += fStep;
		Sync_Animation(false);
		if (0u != Animation_Count())
			m_pModel->Play_Animation(fStep);
		if (m_Params.fLifetime > 0.f && m_fTime >= m_Params.fLifetime)
		{
			if (m_Params.bLoop)
			{
				m_fTime = std::fmod(m_fTime, m_Params.fLifetime);
				m_vDisplacement = { 0.f, 0.f, 0.f };
				Sync_Animation(true);
			}
			else
				m_bFinished = true;
		}
	}
	Apply_Transform();
}

void Client::CEffectV2Object::Apply_Transform()
{
	const f32_t fRatio = Life_Ratio();
	const float3_t vPosition = m_Params.Position.Evaluate(fRatio);
	const float3_t vRotation = m_Params.Rotation.Evaluate(fRatio);
	const float3_t vScale = m_Params.Scale.Evaluate(fRatio);
	const f32_t fPreScale =
		SHAPE::MESH == m_eShape ? (std::max)(0.0001f, m_Params.fMeshPreScale) : 1.f;
	const matrix_t Scale = XMMatrixScaling(
		vScale.x * fPreScale, vScale.y * fPreScale, vScale.z * fPreScale);
	const matrix_t Rotation = XMMatrixRotationRollPitchYaw(
		XMConvertToRadians(vRotation.x),
		XMConvertToRadians(vRotation.y),
		XMConvertToRadians(vRotation.z));
	const matrix_t LocalTranslation = XMMatrixTranslation(
		vPosition.x + m_vDisplacement.x,
		vPosition.y + m_vDisplacement.y,
		vPosition.z + m_vDisplacement.z);
	const matrix_t Pivot = XMLoadFloat4x4(&m_PivotWorld);
	matrix_t World = Scale * Rotation * LocalTranslation * Pivot;
	if (SHAPE::SPRITE == m_eShape && m_Params.bBillboard)
	{
		const float4x4_t* pCameraWorld =
			CGameInstance::Get().Get_InverseTransform(D3DTS::VIEW);
		if (nullptr != pCameraWorld)
		{
			matrix_t CameraWorld = XMLoadFloat4x4(pCameraWorld);
			CameraWorld.r[0] = XMVector3Normalize(CameraWorld.r[0]);
			CameraWorld.r[1] = XMVector3Normalize(CameraWorld.r[1]);
			CameraWorld.r[2] = XMVector3Normalize(CameraWorld.r[2]);
			CameraWorld.r[3] = XMVectorSet(0.f, 0.f, 0.f, 1.f);
			World = Scale * Rotation * CameraWorld *
				XMMatrixTranslationFromVector(World.r[3]);
		}
	}
	m_pTransformCom->Set_State(STATE::RIGHT, World.r[0]);
	m_pTransformCom->Set_State(STATE::UP, World.r[1]);
	m_pTransformCom->Set_State(STATE::LOOK, World.r[2]);
	m_pTransformCom->Set_State(STATE::POSITION, World.r[3]);
}

void Client::CEffectV2Object::Late_Update(const f32_t fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);
	if (m_bHidden || m_bFinished || nullptr == m_pShader)
		return;
	CGameInstance::Get().Add_RenderObject(
		RENDERGROUP::BLEND,
		static_pointer_cast<CGameObject>(shared_from_this()));
}

HRESULT Client::CEffectV2Object::Bind_Common(
	const shared_ptr<Engine::CShader>& pShader)
{
	CGameInstance& GameInstance = CGameInstance::Get();
	if (FAILED(m_pTransformCom->Bind_ShaderResource(pShader, "g_WorldMatrix")) ||
		FAILED(GameInstance.Bind_Transform(pShader, "g_ViewMatrix", D3DTS::VIEW)) ||
		FAILED(GameInstance.Bind_Transform(pShader, "g_ProjMatrix", D3DTS::PROJ)))
	{
		return E_FAIL;
	}
	const PARAMS& P = m_Params;
	const uint32_t iColorClipChannel = static_cast<uint32_t>(P.eColorClipChannel);
	const f32_t fDissolveAmount = Dissolve_Amount();
	const float4_t* pCameraPosition = GameInstance.Get_CamPosition();
	const float4_t vCameraPosition =
		nullptr != pCameraPosition ? *pCameraPosition : float4_t(0.f, 0.f, 0.f, 1.f);
	if (FAILED(pShader->Bind_RawValue("g_vCamPosition", &vCameraPosition, sizeof(vCameraPosition))) ||
		FAILED(pShader->Bind_RawValue("g_RimColor", &P.vRimColor, sizeof(P.vRimColor))) ||
		FAILED(pShader->Bind_RawValue("g_RimPower", &P.fRimPower, sizeof(f32_t))) ||
		FAILED(pShader->Bind_RawValue("g_RimIntensity", &P.fRimIntensity, sizeof(f32_t))) ||
		FAILED(pShader->Bind_RawValue("g_GhostAlpha", &P.fGhostAlpha, sizeof(f32_t))) ||
		FAILED(pShader->Bind_RawValue("g_Time", &m_fTime, sizeof(m_fTime))) ||
		FAILED(pShader->Bind_RawValue("g_ColorMul", &P.vColorMul, sizeof(P.vColorMul))) ||
		FAILED(pShader->Bind_RawValue("g_ColorOffset", &P.vColorOffset, sizeof(P.vColorOffset))) ||
		FAILED(pShader->Bind_RawValue("g_ColorClip", &P.fColorClip, sizeof(f32_t))) ||
		FAILED(pShader->Bind_RawValue("g_ColorClipChannel", &iColorClipChannel, sizeof(iColorClipChannel))) ||
		FAILED(pShader->Bind_RawValue("g_BloomIntensity", &P.fBloomIntensity, sizeof(f32_t))) ||
		FAILED(pShader->Bind_RawValue("g_DistortionIntensity", &P.fDistortionIntensity, sizeof(f32_t))) ||
		FAILED(pShader->Bind_RawValue("g_UVStart", &P.vUVStart, sizeof(P.vUVStart))) ||
		FAILED(pShader->Bind_RawValue("g_UVSpeed", &P.vUVSpeed, sizeof(P.vUVSpeed))) ||
		FAILED(pShader->Bind_RawValue("g_UVTileCount", &P.vUVTileCount, sizeof(P.vUVTileCount))) ||
		FAILED(pShader->Bind_RawValue("g_NoiseStrength", &P.fNoiseStrength, sizeof(f32_t))) ||
		FAILED(pShader->Bind_RawValue("g_NoiseScale", &P.fNoiseScale, sizeof(f32_t))) ||
		FAILED(pShader->Bind_RawValue("g_NoisePan", &P.vNoisePan, sizeof(P.vNoisePan))) ||
		FAILED(pShader->Bind_RawValue("g_DissolveAmount", &fDissolveAmount, sizeof(f32_t))) ||
		FAILED(pShader->Bind_RawValue("g_DissolveSoftness", &P.fDissolveSoftness, sizeof(f32_t))))
	{
		return E_FAIL;
	}
	for (size_t iInput = 0u; iInput < m_Textures.size(); ++iInput)
	{
		const uint32_t iHas = nullptr != m_Textures[iInput] ? 1u : 0u;
		if (FAILED(pShader->Bind_RawValue(
			TEXTURE_FLAG_CONSTANTS[iInput], &iHas, sizeof(iHas))))
			return E_FAIL;
		if (0u != iHas &&
			FAILED(pShader->Bind_Texture(TEXTURE_CONSTANTS[iInput], m_Textures[iInput])))
			return E_FAIL;
	}
	return S_OK;
}

HRESULT Client::CEffectV2Object::Render()
{
	if (FAILED(Bind_Common(m_pShader)))
	{
		m_strStatus = "Shader bind failed.";
		return E_FAIL;
	}
	const uint32_t iPass = BLEND_MODE::SOLID == m_Params.eBlend ? 4u :
		static_cast<uint32_t>(m_Params.eBlend) + (m_Params.bDepthTest ? 0u : 2u);
	if (SHAPE::MESH == m_eShape)
	{
		for (uint32_t iMesh = 0u; iMesh < m_pModel->Get_NumMeshes(); ++iMesh)
		{
			if (iMesh < m_Parts.size() && !m_Parts[iMesh].bVisible)
				continue;
			const ComPtr<ID3D11ShaderResourceView>& pBase =
				(iMesh < m_Parts.size() && nullptr != m_Parts[iMesh].pBaseView) ?
				m_Parts[iMesh].pBaseView :
				m_Textures[static_cast<size_t>(TEXTURE_INPUT::BASE)];
			const uint32_t iHasBase = nullptr != pBase ? 1u : 0u;
			if (FAILED(m_pShader->Bind_RawValue("g_HasBase", &iHasBase, sizeof(iHasBase))) ||
				(0u != iHasBase && FAILED(m_pShader->Bind_Texture("g_BaseTexture", pBase))))
			{
				m_strStatus = "Part base bind failed.";
				return E_FAIL;
			}
			if (m_bSkinned && FAILED(m_pModel->Bind_BoneMatrices(
				m_pShader, "g_BoneMatrices", iMesh)))
			{
				m_strStatus = "Bone matrix bind failed.";
				return E_FAIL;
			}
			if (FAILED(m_pShader->Begin(iPass)) || FAILED(m_pModel->Render(iMesh)))
			{
				m_strStatus = "Mesh draw failed.";
				return E_FAIL;
			}
		}
		return S_OK;
	}
	if (FAILED(m_pShader->Begin(iPass)) || FAILED(m_pRect->Render()))
	{
		m_strStatus = "Sprite draw failed.";
		return E_FAIL;
	}
	return S_OK;
}

unique_ptr<Client::CEffectV2Object> Client::CEffectV2Object::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	unique_ptr<CEffectV2Object> Instance(new CEffectV2Object(
		std::move(pDevice), std::move(pContext)));
	if (FAILED(Instance->Initialize_Prototype()))
		return nullptr;
	return Instance;
}

shared_ptr<CPrototype> Client::CEffectV2Object::Clone(void* pArg)
{
	shared_ptr<CEffectV2Object> Instance(new CEffectV2Object(m_pDevice, m_pContext));
	if (FAILED(Instance->Initialize(pArg)))
		return nullptr;
	return Instance;
}
```

### Client/Bin/ShaderFiles/Shader_EffectV2_Common.hlsli

```hlsl
#include "Engine_Shader_Defines.hlsli"

float4x4 g_WorldMatrix;
float4x4 g_ViewMatrix;
float4x4 g_ProjMatrix;

float g_Time;
float4 g_vCamPosition = float4(0.f, 0.f, 0.f, 1.f);
float4 g_RimColor = float4(1.f, 1.f, 1.f, 1.f);
float g_RimPower = 3.f;
float g_RimIntensity = 0.f;
float g_GhostAlpha = 0.f;
float4 g_ColorMul = float4(1.f, 1.f, 1.f, 1.f);
float4 g_ColorOffset = float4(0.f, 0.f, 0.f, 0.f);
float g_ColorClip = 0.f;
uint g_ColorClipChannel = 1;
float g_BloomIntensity = 1.f;
float g_DistortionIntensity = 0.f;
float2 g_UVStart = float2(0.f, 0.f);
float2 g_UVSpeed = float2(0.f, 0.f);
float2 g_UVTileCount = float2(1.f, 1.f);
float g_NoiseStrength = 0.f;
float g_NoiseScale = 1.f;
float2 g_NoisePan = float2(0.f, 0.f);
float g_DissolveAmount = 0.f;
float g_DissolveSoftness = 0.1f;

Texture2D g_BaseTexture;
Texture2D g_NoiseTexture;
Texture2D g_MaskTexture;
Texture2D g_EmissiveTexture;
Texture2D g_DissolveTexture;
uint g_HasBase = 0;
uint g_HasNoise = 0;
uint g_HasMask = 0;
uint g_HasEmissive = 0;
uint g_HasDissolve = 0;

RasterizerState RS_EffectV2
{
	FillMode = Solid;
	CullMode = None;
	FrontCounterClockwise = false;
};

BlendState BS_EffectV2Alpha
{
	BlendEnable[0] = true;
	BlendEnable[1] = true;
	SrcBlend[0] = Src_Alpha;
	DestBlend[0] = Inv_Src_Alpha;
	BlendOp[0] = Add;
	SrcBlendAlpha[0] = One;
	DestBlendAlpha[0] = Inv_Src_Alpha;
	BlendOpAlpha[0] = Add;
	SrcBlend[1] = One;
	DestBlend[1] = One;
	BlendOp[1] = Add;
	SrcBlendAlpha[1] = One;
	DestBlendAlpha[1] = One;
	BlendOpAlpha[1] = Add;
	RenderTargetWriteMask[1] = 0x03;
};

BlendState BS_EffectV2Additive
{
	BlendEnable[0] = true;
	BlendEnable[1] = true;
	SrcBlend[0] = Src_Alpha;
	DestBlend[0] = One;
	BlendOp[0] = Add;
	SrcBlendAlpha[0] = One;
	DestBlendAlpha[0] = One;
	BlendOpAlpha[0] = Add;
	SrcBlend[1] = One;
	DestBlend[1] = One;
	BlendOp[1] = Add;
	SrcBlendAlpha[1] = One;
	DestBlendAlpha[1] = One;
	BlendOpAlpha[1] = Add;
	RenderTargetWriteMask[1] = 0x03;
};

BlendState BS_EffectV2Opaque
{
	BlendEnable[0] = false;
	BlendEnable[1] = true;
	SrcBlend[1] = One;
	DestBlend[1] = One;
	BlendOp[1] = Add;
	SrcBlendAlpha[1] = One;
	DestBlendAlpha[1] = One;
	BlendOpAlpha[1] = Add;
	RenderTargetWriteMask[1] = 0x03;
};

struct PS_EFFECT_IN
{
	float4 vPosition : SV_POSITION;
	float2 vTexcoord : TEXCOORD0;
	float3 vWorldNormal : NORMAL;
	float3 vWorldPosition : TEXCOORD1;
};

struct PS_EFFECT_OUT
{
	float4 vSceneColor : SV_TARGET0;
	float4 vDistortion : SV_TARGET1;
};

PS_EFFECT_OUT PS_EFFECT_V2(PS_EFFECT_IN input)
{
	PS_EFFECT_OUT output;
	const float2 uv = input.vTexcoord * g_UVTileCount + g_UVStart + g_UVSpeed * g_Time;

	float fresnel = 0.f;
	if (dot(input.vWorldNormal, input.vWorldNormal) > 0.f)
	{
		const float3 N = normalize(input.vWorldNormal);
		const float3 V = normalize(g_vCamPosition.xyz - input.vWorldPosition);
		fresnel = pow(1.f - saturate(abs(dot(N, V))), max(g_RimPower, 0.01f));
	}

	float2 noise = float2(0.5f, 0.5f);
	float2 warp = float2(0.f, 0.f);
	if (0 != g_HasNoise)
	{
		const float2 noiseUV = uv * g_NoiseScale + g_NoisePan * g_Time;
		noise = g_NoiseTexture.Sample(LinearSampler, noiseUV).rg;
		warp = (noise * 2.f - 1.f) * g_NoiseStrength;
	}
	const float2 baseUV = uv + warp;

	float4 base = float4(1.f, 1.f, 1.f, 1.f);
	if (0 != g_HasBase)
		base = g_BaseTexture.Sample(LinearSampler, baseUV);

	float mask = 1.f;
	if (0 != g_HasMask)
		mask = g_MaskTexture.Sample(LinearSampler, uv).r;

	float dissolve = 1.f;
	if (0 != g_HasDissolve)
	{
		const float threshold = g_DissolveTexture.Sample(LinearSampler, uv).r;
		dissolve = smoothstep(
			g_DissolveAmount - g_DissolveSoftness,
			g_DissolveAmount + g_DissolveSoftness,
			threshold);
	}

	float4 color;
	color.rgb = max(base.rgb * g_ColorMul.rgb + g_ColorOffset.rgb, float3(0.f, 0.f, 0.f));
	color.a = saturate(base.a * mask * dissolve * g_ColorMul.a + g_ColorOffset.a);
	color.a *= lerp(1.f, fresnel, saturate(g_GhostAlpha));
	if (color.a <= 0.001f)
		discard;
	const float clipValue = (0 == g_ColorClipChannel) ?
		max(color.r, max(color.g, color.b)) : color.a;
	if (g_ColorClip > 0.f && clipValue <= g_ColorClip)
		discard;

	color.rgb += g_RimColor.rgb * fresnel * g_RimIntensity;
	if (0 != g_HasEmissive)
		color.rgb += g_EmissiveTexture.Sample(LinearSampler, baseUV).rgb * g_BloomIntensity;

	const float2 distortion = (0 != g_HasNoise) ?
		(noise * 2.f - 1.f) * g_DistortionIntensity * color.a : float2(0.f, 0.f);

	output.vSceneColor = color;
	output.vDistortion = float4(distortion, 0.f, 0.f);
	return output;
}

#define EFFECT_V2_PASSES(VS_FUNC) \
	pass AlphaDepth \
	{ \
		SetRasterizerState(RS_EffectV2); \
		SetDepthStencilState(DSS_ReadOnly, 0); \
		SetBlendState(BS_EffectV2Alpha, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff); \
		VertexShader = compile vs_5_0 VS_FUNC(); \
		GeometryShader = NULL; \
		PixelShader = compile ps_5_0 PS_EFFECT_V2(); \
	} \
	pass AdditiveDepth \
	{ \
		SetRasterizerState(RS_EffectV2); \
		SetDepthStencilState(DSS_ReadOnly, 0); \
		SetBlendState(BS_EffectV2Additive, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff); \
		VertexShader = compile vs_5_0 VS_FUNC(); \
		GeometryShader = NULL; \
		PixelShader = compile ps_5_0 PS_EFFECT_V2(); \
	} \
	pass AlphaNoDepth \
	{ \
		SetRasterizerState(RS_EffectV2); \
		SetDepthStencilState(DSS_ZNone, 0); \
		SetBlendState(BS_EffectV2Alpha, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff); \
		VertexShader = compile vs_5_0 VS_FUNC(); \
		GeometryShader = NULL; \
		PixelShader = compile ps_5_0 PS_EFFECT_V2(); \
	} \
	pass AdditiveNoDepth \
	{ \
		SetRasterizerState(RS_EffectV2); \
		SetDepthStencilState(DSS_ZNone, 0); \
		SetBlendState(BS_EffectV2Additive, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff); \
		VertexShader = compile vs_5_0 VS_FUNC(); \
		GeometryShader = NULL; \
		PixelShader = compile ps_5_0 PS_EFFECT_V2(); \
	} \
	pass Opaque \
	{ \
		SetRasterizerState(RS_Default); \
		SetDepthStencilState(DSS_Default, 0); \
		SetBlendState(BS_EffectV2Opaque, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff); \
		VertexShader = compile vs_5_0 VS_FUNC(); \
		GeometryShader = NULL; \
		PixelShader = compile ps_5_0 PS_EFFECT_V2(); \
	}
```

### Client/Bin/ShaderFiles/Shader_EffectMeshV2.hlsl

```hlsl
#include "Shader_EffectV2_Common.hlsli"

struct VS_IN
{
	float3 vPosition : POSITION;
	float3 vNormal : NORMAL;
	float3 vTangent : TANGENT;
	float3 vBinormal : BINORMAL;
	float2 vTexcoord : TEXCOORD0;
};

PS_EFFECT_IN VS_MAIN(VS_IN input)
{
	PS_EFFECT_IN output;
	const float4 worldPosition = mul(float4(input.vPosition, 1.f), g_WorldMatrix);
	output.vPosition = mul(mul(worldPosition, g_ViewMatrix), g_ProjMatrix);
	output.vTexcoord = input.vTexcoord;
	output.vWorldNormal = normalize(mul(input.vNormal, (float3x3)g_WorldMatrix));
	output.vWorldPosition = worldPosition.xyz;
	return output;
}

technique11 DefaultTechnique
{
	EFFECT_V2_PASSES(VS_MAIN)
}
```

### Client/Bin/ShaderFiles/Shader_EffectAnimMeshV2.hlsl

```hlsl
#include "Shader_EffectV2_Common.hlsli"

matrix g_BoneMatrices[512];

struct VS_IN
{
	float3 vPosition : POSITION;
	float3 vNormal : NORMAL;
	float3 vTangent : TANGENT;
	float3 vBinormal : BINORMAL;
	float2 vTexcoord : TEXCOORD0;
	uint4 vBlendIndices : BLENDINDEX;
	float4 vBlendWeights : BLENDWEIGHT;
};

PS_EFFECT_IN VS_MAIN(VS_IN input)
{
	PS_EFFECT_IN output;
	const matrix boneMatrix =
		g_BoneMatrices[input.vBlendIndices.x] * input.vBlendWeights.x +
		g_BoneMatrices[input.vBlendIndices.y] * input.vBlendWeights.y +
		g_BoneMatrices[input.vBlendIndices.z] * input.vBlendWeights.z +
		g_BoneMatrices[input.vBlendIndices.w] * input.vBlendWeights.w;
	const float4 skinnedPosition = mul(float4(input.vPosition, 1.f), boneMatrix);
	const float3 skinnedNormal = mul(float4(input.vNormal, 0.f), boneMatrix).xyz;
	const float4 worldPosition = mul(skinnedPosition, g_WorldMatrix);
	output.vPosition = mul(mul(worldPosition, g_ViewMatrix), g_ProjMatrix);
	output.vTexcoord = input.vTexcoord;
	output.vWorldNormal = normalize(mul(skinnedNormal, (float3x3)g_WorldMatrix));
	output.vWorldPosition = worldPosition.xyz;
	return output;
}

technique11 DefaultTechnique
{
	EFFECT_V2_PASSES(VS_MAIN)
}
```

### Client/Bin/ShaderFiles/Shader_EffectRectV2.hlsl

```hlsl
#include "Shader_EffectV2_Common.hlsli"

struct VS_IN
{
	float3 vPosition : POSITION;
	float2 vTexcoord : TEXCOORD0;
};

PS_EFFECT_IN VS_MAIN(VS_IN input)
{
	PS_EFFECT_IN output;
	const float4 worldPosition = mul(float4(input.vPosition, 1.f), g_WorldMatrix);
	output.vPosition = mul(mul(worldPosition, g_ViewMatrix), g_ProjMatrix);
	output.vTexcoord = input.vTexcoord;
	output.vWorldNormal = float3(0.f, 0.f, 0.f);
	output.vWorldPosition = worldPosition.xyz;
	return output;
}

technique11 DefaultTechnique
{
	EFFECT_V2_PASSES(VS_MAIN)
}
```

### Client/Bin/ShaderFiles/Shader_VtxAnimMeshPreview_V2.hlsl

```hlsl
#include "Engine_Shader_Defines.hlsli"

float4x4 g_WorldMatrix;
float4x4 g_ViewMatrix;
float4x4 g_ProjMatrix;
matrix g_BoneMatrices[512];

float3 g_CameraPosition;
float3 g_LightDirection;

Texture2D g_DiffuseTexture;
Texture2D g_NormalTexture;
uint g_HasNormalTexture = 0;

RasterizerState RS_Preview
{
	FillMode = Solid;
	CullMode = None;
	FrontCounterClockwise = false;
};

struct VS_IN
{
	float3 vPosition : POSITION;
	float3 vNormal : NORMAL;
	float3 vTangent : TANGENT;
	float3 vBinormal : BINORMAL;
	float2 vTexcoord : TEXCOORD0;
	uint4 vBlendIndices : BLENDINDEX;
	float4 vBlendWeights : BLENDWEIGHT;
};

struct VS_OUT
{
	float4 vPosition : SV_POSITION;
	float3 vNormal : NORMAL;
	float3 vTangent : TANGENT;
	float3 vBinormal : BINORMAL;
	float2 vTexcoord : TEXCOORD0;
	float3 vWorldPosition : TEXCOORD1;
};

VS_OUT VS_MAIN(VS_IN input)
{
	VS_OUT output;
	const matrix boneMatrix =
		g_BoneMatrices[input.vBlendIndices.x] * input.vBlendWeights.x +
		g_BoneMatrices[input.vBlendIndices.y] * input.vBlendWeights.y +
		g_BoneMatrices[input.vBlendIndices.z] * input.vBlendWeights.z +
		g_BoneMatrices[input.vBlendIndices.w] * input.vBlendWeights.w;
	const float4 skinnedPosition =
		mul(float4(input.vPosition, 1.f), boneMatrix);
	const float3 skinnedNormal =
		mul(float4(input.vNormal, 0.f), boneMatrix).xyz;
	const float3 skinnedTangent =
		mul(float4(input.vTangent, 0.f), boneMatrix).xyz;
	const float3 skinnedBinormal =
		mul(float4(input.vBinormal, 0.f), boneMatrix).xyz;

	const matrix worldView = mul(g_WorldMatrix, g_ViewMatrix);
	const matrix worldViewProjection = mul(worldView, g_ProjMatrix);

	output.vPosition = mul(skinnedPosition, worldViewProjection);
	output.vNormal = normalize(mul(float4(skinnedNormal, 0.f), g_WorldMatrix).xyz);
	output.vTangent = normalize(mul(float4(skinnedTangent, 0.f), g_WorldMatrix).xyz);
	output.vBinormal = normalize(mul(float4(skinnedBinormal, 0.f), g_WorldMatrix).xyz);
	output.vTexcoord = input.vTexcoord;
	output.vWorldPosition = mul(skinnedPosition, g_WorldMatrix).xyz;
	return output;
}

float4 PS_MAIN(VS_OUT input) : SV_TARGET0
{
	const float4 albedo = g_DiffuseTexture.Sample(LinearSampler, input.vTexcoord);
	if (albedo.a < 0.3f)
		discard;

	float3 normal = normalize(input.vNormal);
	if (0 != g_HasNormalTexture)
	{
		const float3 tangentNormal =
			g_NormalTexture.Sample(LinearSampler, input.vTexcoord).xyz * 2.f - 1.f;
		const float3x3 tangentToWorld = float3x3(
			normalize(input.vTangent),
			normalize(input.vBinormal) * -1.f,
			normal);
		normal = normalize(mul(tangentNormal, tangentToWorld));
	}

	const float3 light = normalize(-g_LightDirection);
	const float diffuseLight = saturate(dot(normal, light));
	const float hemisphere = 0.38f + saturate(normal.y) * 0.18f;
	const float3 viewDirection = normalize(g_CameraPosition - input.vWorldPosition);
	const float rim = pow(1.f - saturate(dot(normal, viewDirection)), 3.f) * 0.12f;

	const float3 color = albedo.rgb * (hemisphere + diffuseLight * 0.72f) + rim;
	return float4(color, albedo.a);
}

technique11 DefaultTechnique
{
	pass PreviewPass
	{
		SetRasterizerState(RS_Preview);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_MAIN();
	}
}
```

### Tools/EffectToolV2/build_texture_slot_usage.py

```python
"""Build Data/Effects/V2/TextureSlotUsage.v1.json for Effect Tool v2.

Scans every imported/authored Effect document and records, per texture file name,
how many times it was bound to each material slot (base/noise/mask/emissive/dissolve)
and which original material parameter names referenced it.

Usage (Blender bundled python is fine):
    python Tools/EffectToolV2/build_texture_slot_usage.py
"""
import collections
import glob
import json
import os
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
OUTPUT = os.path.join(ROOT, "Data", "Effects", "V2", "TextureSlotUsage.v1.json")
SOURCES = [
    "Data/Effects/Imported/*/Converted/*.imported.effect.json",
    "Data/Effects/Authored/*.json",
]
SLOT_ALIASES = {
    "base": "base", "base2": "base",
    "noise": "noise", "noise2": "noise",
    "mask": "mask", "mask2": "mask",
    "emissive": "emissive",
    "dissolve": "dissolve",
}


def walk(node, slots, params):
    if isinstance(node, dict):
        asset = node.get("assetId")
        if isinstance(asset, str) and asset.lower().endswith(".dds"):
            name = os.path.basename(asset).lower()
            slot = SLOT_ALIASES.get(str(node.get("slotId", "")).lower())
            if slot:
                slots[name][slot] += 1
            param = node.get("name")
            if isinstance(param, str) and "sourceObjectPath" in node:
                params[name][param.lower()] += 1
        for value in node.values():
            walk(value, slots, params)
    elif isinstance(node, list):
        for value in node:
            walk(value, slots, params)


def main():
    slots = collections.defaultdict(collections.Counter)
    params = collections.defaultdict(collections.Counter)
    files = []
    for pattern in SOURCES:
        files.extend(sorted(glob.glob(os.path.join(ROOT, pattern))))
    parsed = 0
    for path in files:
        try:
            with open(path, encoding="utf-8") as handle:
                walk(json.load(handle), slots, params)
            parsed += 1
        except (OSError, ValueError) as error:
            print(f"skip {path}: {error}", file=sys.stderr)

    names = sorted(set(slots) | set(params))
    textures = {}
    for name in names:
        entry = {slot: count for slot, count in sorted(slots[name].items())}
        if params[name]:
            entry["params"] = [p for p, _ in params[name].most_common()]
        textures[name] = entry

    document = {
        "schema": "lostark.effect-tool-v2.texture-slot-usage",
        "formatVersion": 1,
        "sourcePatterns": SOURCES,
        "sourceDocumentCount": parsed,
        "slots": ["base", "noise", "mask", "emissive", "dissolve"],
        "textures": textures,
    }
    os.makedirs(os.path.dirname(OUTPUT), exist_ok=True)
    with open(OUTPUT, "w", encoding="utf-8", newline="\n") as handle:
        json.dump(document, handle, ensure_ascii=False, indent=1)
        handle.write("\n")
    print(f"wrote {OUTPUT}: {len(textures)} textures from {parsed} documents")


if __name__ == "__main__":
    main()
```

### Client/Public/EffectV2_Document.h

```cpp
#pragma once

#include "Client_Defines.h"
#include "EffectV2_Object.h"

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

NS_BEGIN(Client)

enum class EFFECT_V2_TYPE : int32_t
{
	MESH,
	TEXTURE,
	PARTICLE,
	DECAL,
	TRAIL,
	END
};

struct EFFECT_V2_PART_OVERRIDE final
{
	bool_t bVisible = true;
	std::string strBaseAssetId;
};

struct EFFECT_V2_DOCUMENT final
{
	std::string strEffectId;
	EFFECT_V2_TYPE eType = EFFECT_V2_TYPE::MESH;
	CEffectV2Object::DESC Desc;
	std::vector<EFFECT_V2_PART_OVERRIDE> Parts;
	std::string strAnimationClip;
};

struct EFFECT_V2_BINDING final
{
	std::string strEffectId;
	std::string strClip;
	uint32_t iStartMs = 0u;
	std::string strBone;
	bool_t bFollowBone = true;
	CEffectV2Object::PIVOT_ROTATION eRotation = CEffectV2Object::PIVOT_ROTATION::TARGET_YAW;
	bool_t bStopWithClip = false;
};

class CEffectV2Document final
{
public:
	static std::filesystem::path Document_Directory();
	static std::filesystem::path Binding_Directory();
	static std::filesystem::path Document_Path(const std::string& strEffectId);
	static std::filesystem::path Binding_Path(const std::string& strArchetypeId);
	static bool_t Is_ValidEffectId(const std::string& strEffectId);

	static bool_t Parse_Document(
		const std::string& strText,
		EFFECT_V2_DOCUMENT& OutDocument,
		std::string& strOutError);
	static bool_t Parse_Bindings(
		const std::string& strText,
		const std::string& strExpectedArchetypeId,
		std::vector<EFFECT_V2_BINDING>& OutBindings,
		std::string& strOutError);
	static std::string Serialize_Document(const EFFECT_V2_DOCUMENT& Document);
	static std::string Serialize_Bindings(
		const std::string& strArchetypeId,
		const std::vector<EFFECT_V2_BINDING>& Bindings);

	static bool_t Load_DocumentFile(
		const std::string& strEffectId,
		EFFECT_V2_DOCUMENT& OutDocument,
		std::string& strOutError);
	static bool_t Load_BindingsFile(
		const std::string& strArchetypeId,
		std::vector<EFFECT_V2_BINDING>& OutBindings,
		std::string& strOutError);
	static bool_t Write_AtomicFile(
		const std::filesystem::path& Target,
		const std::string& strText,
		std::string& strOutError);

	static const char* Type_Key(EFFECT_V2_TYPE eType);
	static const char* Rotation_Key(CEffectV2Object::PIVOT_ROTATION eRotation);
};

NS_END
```

### Client/Private/EffectV2_Document.cpp

```cpp
#include "EffectV2_Document.h"
#include "DataJson.h"
#include "ProjectDataRoot.h"
#include "RuntimeAssetRoot.h"

#include <cctype>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iterator>

namespace
{
	const char* EFFECT_TYPE_KEYS[] = { "Mesh", "Texture", "Particle", "Decal", "Trail" };
	const char* BLEND_KEYS[] = { "Alpha", "Additive", "Opaque" };
	const char* CLIP_CHANNEL_KEYS[] = { "RGB", "Alpha" };
	const char* PIVOT_ROTATION_KEYS[] = { "Bone", "TargetYaw", "World" };
	const char* SLOT_KEYS[] = { "mesh", "base", "noise", "mask", "emissive", "dissolve" };

	std::string Json_String(const std::string& strValue)
	{
		return "\"" + Client::CDataJson::Escape(strValue) + "\"";
	}

	std::string Json_Number(const f32_t fValue)
	{
		char szBuffer[48]{};
		std::snprintf(szBuffer, sizeof(szBuffer), "%.7g",
			std::isfinite(fValue) ? static_cast<double>(fValue) : 0.0);
		std::string strText = szBuffer;
		if (std::string::npos == strText.find_first_of(".eE"))
			strText += ".0";
		return strText;
	}

	std::string Json_Float2(const float2_t& vValue)
	{
		return "[" + Json_Number(vValue.x) + ", " + Json_Number(vValue.y) + "]";
	}

	std::string Json_Float3(const float3_t& vValue)
	{
		return "[" + Json_Number(vValue.x) + ", " + Json_Number(vValue.y) + ", " +
			Json_Number(vValue.z) + "]";
	}

	std::string Json_Float4(const float4_t& vValue)
	{
		return "[" + Json_Number(vValue.x) + ", " + Json_Number(vValue.y) + ", " +
			Json_Number(vValue.z) + ", " + Json_Number(vValue.w) + "]";
	}

	std::string Json_Lerp(const Client::CEffectV2Object::LERP_FLOAT3& Track)
	{
		return "{ \"start\": " + Json_Float3(Track.vStart) +
			", \"end\": " + Json_Float3(Track.vEnd) +
			", \"lerp\": " + (Track.bLerp ? "true" : "false") + " }";
	}

	const char* Json_Bool(const bool_t bValue)
	{
		return bValue ? "true" : "false";
	}

	bool_t Read_Number(const Client::DATA_JSON_VALUE& Object,
		const char* pKey, f32_t& fOut, std::string& strError)
	{
		const Client::DATA_JSON_VALUE* pValue = Object.Find(pKey);
		if (nullptr == pValue)
			return true;
		if (!pValue->Is_Number() || !std::isfinite(pValue->Get_Number()))
		{
			strError = std::string("params.") + pKey + " must be a finite number.";
			return false;
		}
		fOut = static_cast<f32_t>(pValue->Get_Number());
		return true;
	}

	bool_t Read_Bool(const Client::DATA_JSON_VALUE& Object,
		const char* pKey, bool_t& bOut, std::string& strError)
	{
		const Client::DATA_JSON_VALUE* pValue = Object.Find(pKey);
		if (nullptr == pValue)
			return true;
		if (!pValue->Is_Boolean())
		{
			strError = std::string(pKey) + " must be a boolean.";
			return false;
		}
		bOut = pValue->Get_Boolean();
		return true;
	}

	bool_t Read_FloatArray(const Client::DATA_JSON_VALUE& Object,
		const char* pKey, f32_t* pOut, const size_t iCount, std::string& strError)
	{
		const Client::DATA_JSON_VALUE* pValue = Object.Find(pKey);
		if (nullptr == pValue)
			return true;
		if (!pValue->Is_Array() || pValue->Get_Array().size() != iCount)
		{
			strError = std::string("params.") + pKey + " must be an array of " +
				std::to_string(iCount) + " numbers.";
			return false;
		}
		for (size_t iIndex = 0u; iIndex < iCount; ++iIndex)
		{
			const Client::DATA_JSON_VALUE& Element = pValue->Get_Array()[iIndex];
			if (!Element.Is_Number() || !std::isfinite(Element.Get_Number()))
			{
				strError = std::string("params.") + pKey + " contains a non-finite value.";
				return false;
			}
			pOut[iIndex] = static_cast<f32_t>(Element.Get_Number());
		}
		return true;
	}

	bool_t Read_Lerp(const Client::DATA_JSON_VALUE& Object,
		const char* pKey, Client::CEffectV2Object::LERP_FLOAT3& Track, std::string& strError)
	{
		const Client::DATA_JSON_VALUE* pValue = Object.Find(pKey);
		if (nullptr == pValue)
			return true;
		if (!pValue->Is_Object())
		{
			strError = std::string("params.") + pKey + " must be an object.";
			return false;
		}
		return Read_FloatArray(*pValue, "start", &Track.vStart.x, 3u, strError) &&
			Read_FloatArray(*pValue, "end", &Track.vEnd.x, 3u, strError) &&
			Read_Bool(*pValue, "lerp", Track.bLerp, strError);
	}

	bool_t Read_Enum(const Client::DATA_JSON_VALUE& Object, const char* pKey,
		const char* const* pKeys, const size_t iKeyCount, int32_t& iOut, std::string& strError)
	{
		const Client::DATA_JSON_VALUE* pValue = Object.Find(pKey);
		if (nullptr == pValue)
			return true;
		if (pValue->Is_String())
		{
			for (size_t iIndex = 0u; iIndex < iKeyCount; ++iIndex)
			{
				if (pValue->Get_String() == pKeys[iIndex])
				{
					iOut = static_cast<int32_t>(iIndex);
					return true;
				}
			}
		}
		strError = std::string(pKey) + " has an unknown value.";
		return false;
	}

	bool_t Asset_Exists(const std::string& strAssetId)
	{
		const std::filesystem::path Resolved =
			Client::CRuntimeAssetRoot::Resolve(std::filesystem::path(strAssetId));
		return !Resolved.empty() && std::filesystem::is_regular_file(Resolved);
	}

	bool_t Read_TextFile(const std::filesystem::path& Path, std::string& OutText)
	{
		std::ifstream Stream(Path, std::ios::binary);
		if (!Stream.is_open())
			return false;
		OutText.assign((std::istreambuf_iterator<char>(Stream)),
			std::istreambuf_iterator<char>());
		return true;
	}
}

std::filesystem::path Client::CEffectV2Document::Document_Directory()
{
	return CProjectDataRoot::Resolve(L"Effects/V2/Authored");
}

std::filesystem::path Client::CEffectV2Document::Binding_Directory()
{
	return CProjectDataRoot::Resolve(L"Effects/V2/Bindings");
}

std::filesystem::path Client::CEffectV2Document::Document_Path(const std::string& strEffectId)
{
	return Document_Directory() / (strEffectId + ".effectv2.json");
}

std::filesystem::path Client::CEffectV2Document::Binding_Path(const std::string& strArchetypeId)
{
	return Binding_Directory() / (strArchetypeId + ".effectv2bindings.json");
}

bool_t Client::CEffectV2Document::Is_ValidEffectId(const std::string& strEffectId)
{
	if (strEffectId.empty() || strEffectId.size() > 80u)
		return false;
	for (const char Character : strEffectId)
	{
		if (!std::isalnum(static_cast<unsigned char>(Character)) &&
			'.' != Character && '_' != Character && '-' != Character)
			return false;
	}
	return true;
}

const char* Client::CEffectV2Document::Type_Key(const EFFECT_V2_TYPE eType)
{
	const size_t iIndex = static_cast<size_t>(eType);
	return iIndex < _countof(EFFECT_TYPE_KEYS) ? EFFECT_TYPE_KEYS[iIndex] : "Mesh";
}

const char* Client::CEffectV2Document::Rotation_Key(const CEffectV2Object::PIVOT_ROTATION eRotation)
{
	const size_t iIndex = static_cast<size_t>(eRotation);
	return iIndex < _countof(PIVOT_ROTATION_KEYS) ? PIVOT_ROTATION_KEYS[iIndex] : "TargetYaw";
}

bool_t Client::CEffectV2Document::Parse_Document(
	const std::string& strText,
	EFFECT_V2_DOCUMENT& OutDocument,
	std::string& strOutError)
{
	DATA_JSON_VALUE Root;
	if (!CDataJson::Parse(strText, Root, strOutError) || !Root.Is_Object())
	{
		if (strOutError.empty())
			strOutError = "Document root is not an object.";
		return false;
	}
	const DATA_JSON_VALUE* pSchema = Root.Find("schema");
	const DATA_JSON_VALUE* pVersion = Root.Find("formatVersion");
	const DATA_JSON_VALUE* pEffectId = Root.Find("effectId");
	if (nullptr == pSchema || !pSchema->Is_String() || pSchema->Get_String() != "lostark.effect-v2" ||
		nullptr == pVersion || !pVersion->Is_Number() || pVersion->Get_Number() != 1.0)
	{
		strOutError = "schema must be lostark.effect-v2 formatVersion 1.";
		return false;
	}
	if (nullptr == pEffectId || !pEffectId->Is_String() || !Is_ValidEffectId(pEffectId->Get_String()))
	{
		strOutError = "effectId is missing or invalid.";
		return false;
	}
	if (nullptr == Root.Find("effectType"))
	{
		strOutError = "effectType is required.";
		return false;
	}
	int32_t iType = 0;
	if (!Read_Enum(Root, "effectType", EFFECT_TYPE_KEYS,
		_countof(EFFECT_TYPE_KEYS), iType, strOutError))
		return false;

	EFFECT_V2_DOCUMENT Document{};
	Document.strEffectId = pEffectId->Get_String();
	Document.eType = static_cast<EFFECT_V2_TYPE>(iType);
	Document.Desc.eShape = EFFECT_V2_TYPE::MESH == Document.eType ?
		CEffectV2Object::SHAPE::MESH : CEffectV2Object::SHAPE::SPRITE;

	const DATA_JSON_VALUE* pSlots = Root.Find("slots");
	if (nullptr == pSlots || !pSlots->Is_Object())
	{
		strOutError = "slots object is required.";
		return false;
	}
	for (size_t iKey = 0u; iKey < _countof(SLOT_KEYS); ++iKey)
	{
		const DATA_JSON_VALUE* pValue = pSlots->Find(SLOT_KEYS[iKey]);
		if (nullptr == pValue)
			continue;
		if (!pValue->Is_String())
		{
			strOutError = std::string("slots.") + SLOT_KEYS[iKey] + " must be a string.";
			return false;
		}
		const std::string& strAssetId = pValue->Get_String();
		if (!strAssetId.empty() && !Asset_Exists(strAssetId))
		{
			strOutError = std::string("slots.") + SLOT_KEYS[iKey] + " asset is missing: " + strAssetId;
			return false;
		}
		if (0u == iKey)
			Document.Desc.strMeshAssetId = strAssetId;
		else
			Document.Desc.TextureAssetIds[iKey - 1u] = strAssetId;
	}
	if (CEffectV2Object::SHAPE::MESH == Document.Desc.eShape && Document.Desc.strMeshAssetId.empty())
	{
		strOutError = "Mesh effect requires slots.mesh.";
		return false;
	}

	const DATA_JSON_VALUE* pParams = Root.Find("params");
	if (nullptr == pParams || !pParams->Is_Object())
	{
		strOutError = "params object is required.";
		return false;
	}
	CEffectV2Object::PARAMS& P = Document.Desc.Params;
	int32_t iClipChannel = static_cast<int32_t>(P.eColorClipChannel);
	int32_t iBlend = static_cast<int32_t>(P.eBlend);
	if (!Read_Lerp(*pParams, "position", P.Position, strOutError) ||
		!Read_Lerp(*pParams, "rotation", P.Rotation, strOutError) ||
		!Read_Lerp(*pParams, "scale", P.Scale, strOutError) ||
		!Read_Lerp(*pParams, "velocity", P.Velocity, strOutError) ||
		!Read_FloatArray(*pParams, "colorOffset", &P.vColorOffset.x, 4u, strOutError) ||
		!Read_FloatArray(*pParams, "colorMul", &P.vColorMul.x, 4u, strOutError) ||
		!Read_Enum(*pParams, "colorClipChannel", CLIP_CHANNEL_KEYS,
			_countof(CLIP_CHANNEL_KEYS), iClipChannel, strOutError) ||
		!Read_Number(*pParams, "colorClip", P.fColorClip, strOutError) ||
		!Read_FloatArray(*pParams, "rimColor", &P.vRimColor.x, 4u, strOutError) ||
		!Read_Number(*pParams, "rimPower", P.fRimPower, strOutError) ||
		!Read_Number(*pParams, "rimIntensity", P.fRimIntensity, strOutError) ||
		!Read_Number(*pParams, "ghostAlpha", P.fGhostAlpha, strOutError) ||
		!Read_Number(*pParams, "bloomIntensity", P.fBloomIntensity, strOutError) ||
		!Read_Number(*pParams, "distortionIntensity", P.fDistortionIntensity, strOutError) ||
		!Read_FloatArray(*pParams, "uvStart", &P.vUVStart.x, 2u, strOutError) ||
		!Read_FloatArray(*pParams, "uvSpeed", &P.vUVSpeed.x, 2u, strOutError) ||
		!Read_FloatArray(*pParams, "uvTileCount", &P.vUVTileCount.x, 2u, strOutError) ||
		!Read_Number(*pParams, "noiseStrength", P.fNoiseStrength, strOutError) ||
		!Read_Number(*pParams, "noiseScale", P.fNoiseScale, strOutError) ||
		!Read_FloatArray(*pParams, "noisePan", &P.vNoisePan.x, 2u, strOutError) ||
		!Read_Number(*pParams, "dissolveStart", P.fDissolveStart, strOutError) ||
		!Read_Number(*pParams, "dissolveSoftness", P.fDissolveSoftness, strOutError) ||
		!Read_Enum(*pParams, "blend", BLEND_KEYS, _countof(BLEND_KEYS), iBlend, strOutError) ||
		!Read_Bool(*pParams, "billboard", P.bBillboard, strOutError) ||
		!Read_Bool(*pParams, "depthTest", P.bDepthTest, strOutError) ||
		!Read_Number(*pParams, "lifetime", P.fLifetime, strOutError) ||
		!Read_Bool(*pParams, "loop", P.bLoop, strOutError) ||
		!Read_Number(*pParams, "playRate", P.fPlayRate, strOutError) ||
		!Read_Number(*pParams, "meshPreScale", P.fMeshPreScale, strOutError) ||
		!Read_Bool(*pParams, "animationLoop", P.bAnimationLoop, strOutError))
	{
		return false;
	}
	P.eColorClipChannel = static_cast<CEffectV2Object::COLOR_CLIP_CHANNEL>(iClipChannel);
	P.eBlend = static_cast<CEffectV2Object::BLEND_MODE>(iBlend);
	if (P.fMeshPreScale <= 0.f || P.fLifetime < 0.f || P.fPlayRate < 0.f)
	{
		strOutError = "params.meshPreScale/lifetime/playRate out of range.";
		return false;
	}
	if (const DATA_JSON_VALUE* pClip = pParams->Find("animationClip"))
	{
		if (!pClip->Is_String())
		{
			strOutError = "params.animationClip must be a string.";
			return false;
		}
		Document.strAnimationClip = pClip->Get_String();
	}
	Document.Desc.bParamsAuthored = true;

	if (const DATA_JSON_VALUE* pParts = Root.Find("parts"))
	{
		if (!pParts->Is_Array())
		{
			strOutError = "parts must be an array.";
			return false;
		}
		for (const DATA_JSON_VALUE& Part : pParts->Get_Array())
		{
			const DATA_JSON_VALUE* pIndex = Part.Is_Object() ? Part.Find("index") : nullptr;
			if (nullptr == pIndex || !pIndex->Is_Number() || pIndex->Get_Number() < 0.0 ||
				pIndex->Get_Number() > 255.0)
			{
				strOutError = "parts[].index must be a number in [0, 255].";
				return false;
			}
			const size_t iIndex = static_cast<size_t>(pIndex->Get_Number());
			if (Document.Parts.size() <= iIndex)
				Document.Parts.resize(iIndex + 1u);
			EFFECT_V2_PART_OVERRIDE& Override = Document.Parts[iIndex];
			if (!Read_Bool(Part, "visible", Override.bVisible, strOutError))
				return false;
			if (const DATA_JSON_VALUE* pBase = Part.Find("base"))
			{
				if (!pBase->Is_String())
				{
					strOutError = "parts[].base must be a string.";
					return false;
				}
				Override.strBaseAssetId = pBase->Get_String();
				if (!Override.strBaseAssetId.empty() && !Asset_Exists(Override.strBaseAssetId))
				{
					strOutError = "parts[].base asset is missing: " + Override.strBaseAssetId;
					return false;
				}
			}
		}
	}
	OutDocument = std::move(Document);
	return true;
}

bool_t Client::CEffectV2Document::Parse_Bindings(
	const std::string& strText,
	const std::string& strExpectedArchetypeId,
	std::vector<EFFECT_V2_BINDING>& OutBindings,
	std::string& strOutError)
{
	DATA_JSON_VALUE Root;
	if (!CDataJson::Parse(strText, Root, strOutError) || !Root.Is_Object())
	{
		if (strOutError.empty())
			strOutError = "Bindings root is not an object.";
		return false;
	}
	const DATA_JSON_VALUE* pSchema = Root.Find("schema");
	const DATA_JSON_VALUE* pVersion = Root.Find("formatVersion");
	const DATA_JSON_VALUE* pArchetype = Root.Find("archetypeId");
	if (nullptr == pSchema || !pSchema->Is_String() ||
		pSchema->Get_String() != "lostark.effect-v2-bindings" ||
		nullptr == pVersion || !pVersion->Is_Number() || pVersion->Get_Number() != 1.0 ||
		nullptr == pArchetype || !pArchetype->Is_String() ||
		pArchetype->Get_String() != strExpectedArchetypeId)
	{
		strOutError = "schema/formatVersion/archetypeId mismatch.";
		return false;
	}
	const DATA_JSON_VALUE* pRows = Root.Find("bindings");
	if (nullptr == pRows || !pRows->Is_Array())
	{
		strOutError = "bindings must be an array.";
		return false;
	}
	std::vector<EFFECT_V2_BINDING> Staged;
	for (const DATA_JSON_VALUE& Row : pRows->Get_Array())
	{
		if (!Row.Is_Object())
		{
			strOutError = "bindings[] entries must be objects.";
			return false;
		}
		EFFECT_V2_BINDING Binding;
		const DATA_JSON_VALUE* pEffect = Row.Find("effectId");
		const DATA_JSON_VALUE* pClip = Row.Find("clip");
		const DATA_JSON_VALUE* pStart = Row.Find("startMs");
		const DATA_JSON_VALUE* pBone = Row.Find("bone");
		if (nullptr == pEffect || !pEffect->Is_String() || !Is_ValidEffectId(pEffect->Get_String()) ||
			nullptr == pClip || !pClip->Is_String() || pClip->Get_String().empty() ||
			nullptr == pStart || !pStart->Is_Number() || pStart->Get_Number() < 0.0 ||
			pStart->Get_Number() > 600000.0 ||
			nullptr == pBone || !pBone->Is_String())
		{
			strOutError = "bindings[] requires effectId, clip, startMs (0-600000), bone.";
			return false;
		}
		Binding.strEffectId = pEffect->Get_String();
		Binding.strClip = pClip->Get_String();
		Binding.iStartMs = static_cast<uint32_t>(pStart->Get_Number());
		Binding.strBone = pBone->Get_String();
		int32_t iRotation = static_cast<int32_t>(Binding.eRotation);
		if (!Read_Bool(Row, "followBone", Binding.bFollowBone, strOutError) ||
			!Read_Enum(Row, "rotation", PIVOT_ROTATION_KEYS,
				_countof(PIVOT_ROTATION_KEYS), iRotation, strOutError) ||
			!Read_Bool(Row, "stopWithClip", Binding.bStopWithClip, strOutError))
			return false;
		Binding.eRotation = static_cast<CEffectV2Object::PIVOT_ROTATION>(iRotation);
		for (const EFFECT_V2_BINDING& Existing : Staged)
		{
			if (Existing.strEffectId == Binding.strEffectId && Existing.strClip == Binding.strClip)
			{
				strOutError = "duplicate binding: " + Binding.strEffectId + " / " + Binding.strClip;
				return false;
			}
		}
		Staged.push_back(std::move(Binding));
	}
	OutBindings = std::move(Staged);
	return true;
}

std::string Client::CEffectV2Document::Serialize_Document(const EFFECT_V2_DOCUMENT& Document)
{
	const CEffectV2Object::DESC& Desc = Document.Desc;
	const CEffectV2Object::PARAMS& P = Desc.Params;
	std::string Text;
	Text += "{\n";
	Text += "  \"schema\": \"lostark.effect-v2\",\n";
	Text += "  \"formatVersion\": 1,\n";
	Text += "  \"effectId\": " + Json_String(Document.strEffectId) + ",\n";
	Text += "  \"effectType\": " + Json_String(Type_Key(Document.eType)) + ",\n";
	Text += "  \"slots\": {\n";
	Text += "    \"mesh\": " + Json_String(Desc.strMeshAssetId) + ",\n";
	for (size_t iInput = 0u; iInput < Desc.TextureAssetIds.size(); ++iInput)
	{
		Text += std::string("    \"") + SLOT_KEYS[iInput + 1u] + "\": " +
			Json_String(Desc.TextureAssetIds[iInput]) +
			(iInput + 1u < Desc.TextureAssetIds.size() ? ",\n" : "\n");
	}
	Text += "  },\n";
	Text += "  \"params\": {\n";
	Text += "    \"position\": " + Json_Lerp(P.Position) + ",\n";
	Text += "    \"rotation\": " + Json_Lerp(P.Rotation) + ",\n";
	Text += "    \"scale\": " + Json_Lerp(P.Scale) + ",\n";
	Text += "    \"velocity\": " + Json_Lerp(P.Velocity) + ",\n";
	Text += "    \"colorOffset\": " + Json_Float4(P.vColorOffset) + ",\n";
	Text += "    \"colorMul\": " + Json_Float4(P.vColorMul) + ",\n";
	Text += "    \"colorClipChannel\": " + Json_String(
		CLIP_CHANNEL_KEYS[static_cast<size_t>(P.eColorClipChannel)]) + ",\n";
	Text += "    \"colorClip\": " + Json_Number(P.fColorClip) + ",\n";
	Text += "    \"rimColor\": " + Json_Float4(P.vRimColor) + ",\n";
	Text += "    \"rimPower\": " + Json_Number(P.fRimPower) + ",\n";
	Text += "    \"rimIntensity\": " + Json_Number(P.fRimIntensity) + ",\n";
	Text += "    \"ghostAlpha\": " + Json_Number(P.fGhostAlpha) + ",\n";
	Text += "    \"bloomIntensity\": " + Json_Number(P.fBloomIntensity) + ",\n";
	Text += "    \"distortionIntensity\": " + Json_Number(P.fDistortionIntensity) + ",\n";
	Text += "    \"uvStart\": " + Json_Float2(P.vUVStart) + ",\n";
	Text += "    \"uvSpeed\": " + Json_Float2(P.vUVSpeed) + ",\n";
	Text += "    \"uvTileCount\": " + Json_Float2(P.vUVTileCount) + ",\n";
	Text += "    \"noiseStrength\": " + Json_Number(P.fNoiseStrength) + ",\n";
	Text += "    \"noiseScale\": " + Json_Number(P.fNoiseScale) + ",\n";
	Text += "    \"noisePan\": " + Json_Float2(P.vNoisePan) + ",\n";
	Text += "    \"dissolveStart\": " + Json_Number(P.fDissolveStart) + ",\n";
	Text += "    \"dissolveSoftness\": " + Json_Number(P.fDissolveSoftness) + ",\n";
	Text += "    \"blend\": " + Json_String(BLEND_KEYS[static_cast<size_t>(P.eBlend)]) + ",\n";
	Text += std::string("    \"billboard\": ") + Json_Bool(P.bBillboard) + ",\n";
	Text += std::string("    \"depthTest\": ") + Json_Bool(P.bDepthTest) + ",\n";
	Text += "    \"lifetime\": " + Json_Number(P.fLifetime) + ",\n";
	Text += std::string("    \"loop\": ") + Json_Bool(P.bLoop) + ",\n";
	Text += "    \"playRate\": " + Json_Number(P.fPlayRate) + ",\n";
	Text += "    \"meshPreScale\": " + Json_Number(P.fMeshPreScale) + ",\n";
	Text += "    \"animationClip\": " + Json_String(Document.strAnimationClip) + ",\n";
	Text += std::string("    \"animationLoop\": ") + Json_Bool(P.bAnimationLoop) + "\n";
	Text += "  },\n";
	Text += "  \"parts\": [\n";
	for (size_t iPart = 0u; iPart < Document.Parts.size(); ++iPart)
	{
		const EFFECT_V2_PART_OVERRIDE& Part = Document.Parts[iPart];
		Text += "    { \"index\": " + std::to_string(iPart) +
			", \"visible\": " + Json_Bool(Part.bVisible) +
			", \"base\": " + Json_String(Part.strBaseAssetId) + " }" +
			(iPart + 1u < Document.Parts.size() ? ",\n" : "\n");
	}
	Text += "  ]\n";
	Text += "}\n";
	return Text;
}

std::string Client::CEffectV2Document::Serialize_Bindings(
	const std::string& strArchetypeId,
	const std::vector<EFFECT_V2_BINDING>& Bindings)
{
	std::string Text;
	Text += "{\n";
	Text += "  \"schema\": \"lostark.effect-v2-bindings\",\n";
	Text += "  \"formatVersion\": 1,\n";
	Text += "  \"archetypeId\": " + Json_String(strArchetypeId) + ",\n";
	Text += "  \"bindings\": [\n";
	for (size_t iIndex = 0u; iIndex < Bindings.size(); ++iIndex)
	{
		const EFFECT_V2_BINDING& Binding = Bindings[iIndex];
		Text += "    { \"effectId\": " + Json_String(Binding.strEffectId) +
			", \"clip\": " + Json_String(Binding.strClip) +
			", \"startMs\": " + std::to_string(Binding.iStartMs) +
			", \"bone\": " + Json_String(Binding.strBone) +
			", \"followBone\": " + Json_Bool(Binding.bFollowBone) +
			", \"rotation\": " + Json_String(Rotation_Key(Binding.eRotation)) +
			", \"stopWithClip\": " + Json_Bool(Binding.bStopWithClip) + " }" +
			(iIndex + 1u < Bindings.size() ? ",\n" : "\n");
	}
	Text += "  ]\n";
	Text += "}\n";
	return Text;
}

bool_t Client::CEffectV2Document::Load_DocumentFile(
	const std::string& strEffectId,
	EFFECT_V2_DOCUMENT& OutDocument,
	std::string& strOutError)
{
	if (!Is_ValidEffectId(strEffectId))
	{
		strOutError = "Invalid effect ID.";
		return false;
	}
	std::string Text;
	const std::filesystem::path Path = Document_Path(strEffectId);
	if (!Read_TextFile(Path, Text))
	{
		strOutError = "Cannot open: " + Path.string();
		return false;
	}
	if (!Parse_Document(Text, OutDocument, strOutError))
		return false;
	if (OutDocument.strEffectId != strEffectId)
	{
		strOutError = "effectId does not match the file name.";
		return false;
	}
	return true;
}

bool_t Client::CEffectV2Document::Load_BindingsFile(
	const std::string& strArchetypeId,
	std::vector<EFFECT_V2_BINDING>& OutBindings,
	std::string& strOutError)
{
	std::string Text;
	const std::filesystem::path Path = Binding_Path(strArchetypeId);
	if (!Read_TextFile(Path, Text))
	{
		strOutError = "Cannot open: " + Path.string();
		return false;
	}
	return Parse_Bindings(Text, strArchetypeId, OutBindings, strOutError);
}

bool_t Client::CEffectV2Document::Write_AtomicFile(
	const std::filesystem::path& Target,
	const std::string& strText,
	std::string& strOutError)
{
	std::error_code Error;
	if (Target.empty() || Target.parent_path().empty())
	{
		strOutError = "Target path is empty.";
		return false;
	}
	std::filesystem::create_directories(Target.parent_path(), Error);
	const std::filesystem::path Temporary = Target.string() + ".tmp";
	{
		std::ofstream Stream(Temporary, std::ios::binary | std::ios::trunc);
		if (!Stream.is_open())
		{
			strOutError = "Cannot open for write: " + Temporary.string();
			return false;
		}
		Stream << strText;
		if (!Stream.good())
		{
			Stream.close();
			std::filesystem::remove(Temporary, Error);
			strOutError = "Write failed: " + Temporary.string();
			return false;
		}
	}
	std::filesystem::rename(Temporary, Target, Error);
	if (Error)
	{
		std::filesystem::remove(Temporary, Error);
		strOutError = "Rename failed: " + Target.string();
		return false;
	}
	return true;
}
```

### Client/Public/EffectV2_Runtime.h

```cpp
#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <memory>
#include <string>

NS_BEGIN(Client)

class CNpc;

class CEffectV2Runtime final
{
public:
	static void Notify_NpcClip(
		const std::shared_ptr<CNpc>& pNpc,
		const char_t* pClipName);
	static void Tick_Npc(
		const std::shared_ptr<CNpc>& pNpc,
		const ComPtr<ID3D11Device>& pDevice,
		const ComPtr<ID3D11DeviceContext>& pContext);
	static void Prewarm_Archetype(
		const ComPtr<ID3D11Device>& pDevice,
		const ComPtr<ID3D11DeviceContext>& pContext,
		const std::string& strArchetypeId);
	static void Set_Ignored(const std::shared_ptr<CNpc>& pNpc, bool_t bIgnored);
	static void Invalidate_Caches();
	static const std::string& Last_Error();
};

NS_END
```

### Client/Private/EffectV2_Runtime.cpp

```cpp
#include "EffectV2_Runtime.h"
#include "ActorCatalog.h"
#include "EffectV2_Document.h"
#include "EffectV2_Object.h"
#include "GameInstance.h"
#include "Model.h"
#include "Npc.h"
#include "NpcPresentationAssetService.h"

#include <map>
#include <set>
#include <unordered_map>
#include <vector>

namespace
{
	constexpr const wchar_t* EFFECT_LAYER_TAG = L"Layer_EffectV2";
	constexpr const wchar_t* EFFECT_PROTOTYPE_TAG = L"Prototype_GameObject_EffectV2";

	struct BINDING_SET final
	{
		bool_t bLoaded = false;
		bool_t bFailed = false;
		std::vector<Client::EFFECT_V2_BINDING> Bindings;
	};

	struct DOCUMENT_ENTRY final
	{
		bool_t bLoaded = false;
		bool_t bFailed = false;
		Client::EFFECT_V2_DOCUMENT Document;
	};

	struct PENDING_SPAWN final
	{
		Client::EFFECT_V2_BINDING Binding;
		bool_t bSpawned = false;
	};

	struct SPAWNED_EFFECT final
	{
		std::weak_ptr<Client::CEffectV2Object> pObject;
		bool_t bStopWithClip = false;
	};

	struct NPC_STATE final
	{
		std::weak_ptr<Client::CNpc> pNpc;
		std::string strArchetypeId;
		std::string strClip;
		f32_t fLastSeconds = -1.f;
		std::vector<PENDING_SPAWN> Pending;
		std::vector<SPAWNED_EFFECT> Spawned;
	};

	std::unordered_map<std::string, BINDING_SET> g_BindingSets;
	std::unordered_map<std::string, DOCUMENT_ENTRY> g_Documents;
	std::map<std::wstring, std::string> g_ModelTagToArchetype;
	bool_t g_bModelTagMapBuilt = false;
	std::map<const Client::CNpc*, NPC_STATE> g_NpcStates;
	std::set<const Client::CNpc*> g_IgnoredNpcs;
	bool_t g_bPrototypeRegistered = false;
	std::string g_strLastError;

	void Report(const std::string& strMessage)
	{
		g_strLastError = strMessage;
		OutputDebugStringA(("[EffectV2Runtime] " + strMessage + "\n").c_str());
	}

	const std::string* Resolve_Archetype(const Client::CNpc& Npc)
	{
		if (!g_bModelTagMapBuilt)
		{
			g_bModelTagMapBuilt = true;
			for (const Client::NPC_ACTOR_ENTRY& Entry : Client::CActorCatalog::Get_Npcs())
			{
				const wstring_t strTag =
					Client::CNpcPresentationAssetService::Get_ModelPrototypeTag(Entry.archetypeId);
				if (!strTag.empty() && !g_ModelTagToArchetype.contains(strTag))
					g_ModelTagToArchetype.emplace(strTag, Entry.archetypeId);
			}
		}
		const auto Found = g_ModelTagToArchetype.find(Npc.Get_ModelTag());
		return Found != g_ModelTagToArchetype.end() ? &Found->second : nullptr;
	}

	const BINDING_SET& Ensure_Bindings(const std::string& strArchetypeId)
	{
		BINDING_SET& Set = g_BindingSets[strArchetypeId];
		if (Set.bLoaded)
			return Set;
		Set.bLoaded = true;
		std::error_code Error;
		const std::filesystem::path Path =
			Client::CEffectV2Document::Binding_Path(strArchetypeId);
		if (Path.empty() || !std::filesystem::is_regular_file(Path, Error))
			return Set;
		std::string strError;
		if (!Client::CEffectV2Document::Load_BindingsFile(strArchetypeId, Set.Bindings, strError))
		{
			Set.bFailed = true;
			Set.Bindings.clear();
			Report("bindings rejected for " + strArchetypeId + ": " + strError);
		}
		return Set;
	}

	const DOCUMENT_ENTRY& Ensure_Document(const std::string& strEffectId)
	{
		DOCUMENT_ENTRY& Entry = g_Documents[strEffectId];
		if (Entry.bLoaded)
			return Entry;
		Entry.bLoaded = true;
		std::string strError;
		if (!Client::CEffectV2Document::Load_DocumentFile(strEffectId, Entry.Document, strError))
		{
			Entry.bFailed = true;
			Report("document rejected " + strEffectId + ": " + strError);
		}
		return Entry;
	}

	bool_t Ensure_Prototype(
		const ComPtr<ID3D11Device>& pDevice,
		const ComPtr<ID3D11DeviceContext>& pContext)
	{
		if (g_bPrototypeRegistered)
			return true;
		unique_ptr<Client::CEffectV2Object> pPrototype =
			Client::CEffectV2Object::Create(pDevice, pContext);
		if (nullptr == pPrototype)
		{
			Report("effect prototype creation failed.");
			return false;
		}
		(void)CGameInstance::Get().Add_Prototype(
			ETOUI(LEVEL::STATIC), EFFECT_PROTOTYPE_TAG, std::move(pPrototype));
		g_bPrototypeRegistered = true;
		return true;
	}

	void Spawn(
		NPC_STATE& State,
		PENDING_SPAWN& Pending,
		const std::shared_ptr<Client::CNpc>& pNpc,
		const ComPtr<ID3D11Device>& pDevice,
		const ComPtr<ID3D11DeviceContext>& pContext)
	{
		Pending.bSpawned = true;
		const DOCUMENT_ENTRY& Entry = Ensure_Document(Pending.Binding.strEffectId);
		if (Entry.bFailed || !Ensure_Prototype(pDevice, pContext))
			return;
		CGameInstance& GameInstance = CGameInstance::Get();
		Client::CEffectV2Object::DESC Desc = Entry.Document.Desc;
		if (!Client::CEffectV2Object::Resolve_TargetPivot(
			*pNpc,
			Pending.Binding.bFollowBone ? Pending.Binding.strBone : std::string(),
			Pending.Binding.eRotation,
			Desc.PivotWorld))
		{
			Report("pivot bone not found: " + Pending.Binding.strBone +
				" for " + Pending.Binding.strEffectId);
			return;
		}
		std::shared_ptr<CGameObject> pGameObject;
		if (FAILED(GameInstance.Add_GameObject_to_Layer(
			ETOUI(LEVEL::STATIC), EFFECT_PROTOTYPE_TAG,
			GameInstance.Get_CurrentLevelID(), EFFECT_LAYER_TAG,
			&Desc, &pGameObject)))
		{
			Report("spawn failed for " + Pending.Binding.strEffectId + ": " +
				Client::CEffectV2Object::Last_Error());
			return;
		}
		const std::shared_ptr<Client::CEffectV2Object> pObject =
			std::dynamic_pointer_cast<Client::CEffectV2Object>(pGameObject);
		if (nullptr == pObject)
			return;
		for (uint32_t iPart = 0u;
			iPart < Entry.Document.Parts.size() && iPart < pObject->Part_Count(); ++iPart)
		{
			pObject->Part_Visible(iPart) = Entry.Document.Parts[iPart].bVisible;
			if (!Entry.Document.Parts[iPart].strBaseAssetId.empty())
				(void)pObject->Set_PartBase(iPart, Entry.Document.Parts[iPart].strBaseAssetId);
		}
		if (!Entry.Document.strAnimationClip.empty())
		{
			for (uint32_t iClip = 0u; iClip < pObject->Animation_Count(); ++iClip)
			{
				const char_t* pName = pObject->Animation_Name(iClip);
				if (nullptr != pName && Entry.Document.strAnimationClip == pName)
				{
					pObject->Params().iAnimationIndex = iClip;
					break;
				}
			}
		}
		if (Pending.Binding.bFollowBone)
		{
			pObject->Set_FollowTarget(
				pNpc, Pending.Binding.strBone, Pending.Binding.eRotation);
		}
		State.Spawned.push_back({ pObject, Pending.Binding.bStopWithClip });
	}

	void Prune_Spawned(NPC_STATE& State, const bool_t bStopClipBound)
	{
		CGameInstance& GameInstance = CGameInstance::Get();
		for (auto Iterator = State.Spawned.begin(); Iterator != State.Spawned.end();)
		{
			const std::shared_ptr<Client::CEffectV2Object> pObject = Iterator->pObject.lock();
			if (nullptr != pObject && bStopClipBound && Iterator->bStopWithClip)
				pObject->Finish();
			if (nullptr == pObject || pObject->Is_Finished())
			{
				if (nullptr != pObject)
				{
					GameInstance.Remove_GameObject_from_Layer(
						GameInstance.Get_CurrentLevelID(), EFFECT_LAYER_TAG, pObject);
				}
				Iterator = State.Spawned.erase(Iterator);
				continue;
			}
			++Iterator;
		}
	}
}

void Client::CEffectV2Runtime::Prewarm_Archetype(
	const ComPtr<ID3D11Device>& pDevice,
	const ComPtr<ID3D11DeviceContext>& pContext,
	const std::string& strArchetypeId)
{
	const BINDING_SET& Set = Ensure_Bindings(strArchetypeId);
	if (Set.bFailed)
		return;
	for (const EFFECT_V2_BINDING& Binding : Set.Bindings)
	{
		const DOCUMENT_ENTRY& Entry = Ensure_Document(Binding.strEffectId);
		if (Entry.bFailed)
			continue;
		std::string strError;
		if (FAILED(CEffectV2Object::Prewarm(pDevice, pContext, Entry.Document.Desc, strError)))
			Report("prewarm failed " + Binding.strEffectId + ": " + strError);
	}
	Ensure_Prototype(pDevice, pContext);
}

void Client::CEffectV2Runtime::Set_Ignored(const std::shared_ptr<CNpc>& pNpc, const bool_t bIgnored)
{
	if (nullptr == pNpc)
		return;
	if (bIgnored)
	{
		g_IgnoredNpcs.insert(pNpc.get());
		const auto Found = g_NpcStates.find(pNpc.get());
		if (Found != g_NpcStates.end())
		{
			Prune_Spawned(Found->second, true);
			g_NpcStates.erase(Found);
		}
	}
	else
		g_IgnoredNpcs.erase(pNpc.get());
}

void Client::CEffectV2Runtime::Invalidate_Caches()
{
	g_BindingSets.clear();
	g_Documents.clear();
	g_ModelTagToArchetype.clear();
	g_bModelTagMapBuilt = false;
}

const std::string& Client::CEffectV2Runtime::Last_Error()
{
	return g_strLastError;
}

void Client::CEffectV2Runtime::Notify_NpcClip(
	const std::shared_ptr<CNpc>& pNpc,
	const char_t* pClipName)
{
	if (nullptr == pNpc || nullptr == pClipName || g_IgnoredNpcs.contains(pNpc.get()))
		return;
	const std::string* pArchetypeId = Resolve_Archetype(*pNpc);
	if (nullptr == pArchetypeId)
		return;
	const BINDING_SET& Set = Ensure_Bindings(*pArchetypeId);
	NPC_STATE& State = g_NpcStates[pNpc.get()];
	State.pNpc = pNpc;
	State.strArchetypeId = *pArchetypeId;
	State.strClip = pClipName;
	State.fLastSeconds = -1.f;
	Prune_Spawned(State, true);
	State.Pending.clear();
	if (Set.bFailed)
		return;
	for (const EFFECT_V2_BINDING& Binding : Set.Bindings)
	{
		if (Binding.strClip == State.strClip)
			State.Pending.push_back({ Binding, false });
	}
}

void Client::CEffectV2Runtime::Tick_Npc(
	const std::shared_ptr<CNpc>& pNpc,
	const ComPtr<ID3D11Device>& pDevice,
	const ComPtr<ID3D11DeviceContext>& pContext)
{
	for (auto Iterator = g_NpcStates.begin(); Iterator != g_NpcStates.end();)
	{
		if (Iterator->second.pNpc.expired())
		{
			Prune_Spawned(Iterator->second, true);
			Iterator = g_NpcStates.erase(Iterator);
			continue;
		}
		++Iterator;
	}
	if (nullptr == pNpc)
		return;
	const auto Found = g_NpcStates.find(pNpc.get());
	if (Found == g_NpcStates.end())
		return;
	NPC_STATE& State = Found->second;
	const shared_ptr<Engine::CModel> pModel = pNpc->Get_Model();
	if (nullptr == pModel)
		return;
	const uint32_t iClip = pModel->Get_CurrentAnimIndex();
	const char_t* pCurrentClip = pModel->Get_AnimationName(iClip);
	if (nullptr == pCurrentClip || State.strClip != pCurrentClip)
	{
		Prune_Spawned(State, true);
		State.Pending.clear();
		return;
	}
	f32_t fPosition = 0.f;
	f32_t fDuration = 0.f;
	const f32_t fTickPerSecond = pModel->Get_AnimationTickPerSecond(iClip);
	if (fTickPerSecond <= 0.f || !pModel->Get_AnimationProgress(iClip, fPosition, fDuration))
		return;
	const f32_t fSeconds = fPosition / fTickPerSecond;
	if (fSeconds < State.fLastSeconds)
	{
		for (PENDING_SPAWN& Pending : State.Pending)
			Pending.bSpawned = false;
	}
	State.fLastSeconds = fSeconds;
	for (PENDING_SPAWN& Pending : State.Pending)
	{
		if (Pending.bSpawned)
			continue;
		if (fSeconds >= static_cast<f32_t>(Pending.Binding.iStartMs) / 1000.f)
			Spawn(State, Pending, pNpc, pDevice, pContext);
	}
	Prune_Spawned(State, false);
}
```

### Client/Public/Npc.h

```cpp
#pragma once

#include "Client_Defines.h"
#include "DeferredMaterialRenderUtils.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CCollider;
NS_END

NS_BEGIN(Client)

/* A town NPC: one skinned model that stands where it is put and plays a clip.

Deliberately not a CCharacter. That type assembles equipment parts, weapon
sockets and a class logic from a CHARACTER_SPEC, none of which an NPC has -- the
cook already merges an NPC's body and head into a single mesh, so there is one
model and nothing to assemble.

Everything an instance differs by is in NPC_DESC, so the placement tool can spawn
the same prototype many times with different positions and clips. */
class CNpc final : public CGameObject
{
public:
	typedef struct tagNpcDesc : public CGameObject::GAMEOBJECT_DESC
	{
		uint32_t iPrototypeLevelIndex = {};
		wstring_t strModelTag;
		wstring_t strShaderTag;

		/* Clip to stand in. Every NPC is cooked under the same "npc" armature
		name, so the clip names all carry that prefix -- "npc_idle_normal_1",
		"npc_sc_talk_1" -- and one name works across every NPC that shares an
		archetype. An unknown name falls back to the model's first clip. */
		const char_t* pIdleClip = { nullptr };
		bool_t isLoop = { true };

		float3_t vPosition = {};
		/* Degrees about Y. Town NPCs face doors and counters, not always north. */
		f32_t fYawDegree = {};
		/* Zero for non-combat NPCs; Server-replicated radius for monsters. */
		f32_t fCollisionRadius = {};
	} NPC_DESC;

private:
	CNpc(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CNpc();

public:
	shared_ptr<Engine::CModel> Get_Model() const {
		return m_pModelCom;
	}
	shared_ptr<Engine::CTransform> Get_Transform() const {
		return m_pTransformCom;
	}
	const wstring_t& Get_ModelTag() const {
		return m_strModelTag;
	}
	bool_t Set_Animation(const char_t* pClipName, bool_t isLoop);
	bool_t Apply_NetworkState(
		const float3_t& position,
		f32_t yawDegrees);
	void Trigger_HitFlash();
#ifdef _DEBUG
	void Set_CombatColliderDebugVisible(bool_t isVisible) {
		m_isCombatColliderDebugVisible = isVisible;
	}
#endif

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(f32_t fTimeDelta) override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual void Late_Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	shared_ptr<Engine::CShader> m_pShaderCom = { nullptr };
	shared_ptr<Engine::CModel> m_pModelCom = { nullptr };
	wstring_t m_strModelTag;
	shared_ptr<Engine::CCollider> m_pColliderCom = { nullptr };
	DEFERRED_EMISSIVE_OVERRIDE m_HitFlash;
	f32_t m_fHitFlashRemainingSeconds = { 0.f };
#ifdef _DEBUG
	bool_t m_isCombatColliderDebugVisible = { false };
#endif

private:
	HRESULT Ready_Components(const NPC_DESC* pDesc);
	HRESULT Bind_ShaderResources();

public:
	static unique_ptr<CNpc> Create(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
```

### Client/Private/Npc.cpp

```cpp
#include "Npc.h"
#include "EffectV2_Runtime.h"

#include "Collider.h"
#include "DeferredMaterialRenderUtils.h"
#include "GameInstance.h"
#include "Model.h"
#include "Shader.h"
#include "Transform.h"

#include <cmath>

namespace
{
	constexpr f32_t HIT_FLASH_DURATION_SECONDS = 0.12f;
	constexpr f32_t HIT_FLASH_PEAK_INTENSITY = 4.f;
}

CNpc::CNpc(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CGameObject{ pDevice, pContext }
{
}

CNpc::~CNpc()
{
}

HRESULT CNpc::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CNpc::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	const NPC_DESC* pDesc = static_cast<const NPC_DESC*>(pArg);
	if (!std::isfinite(pDesc->fCollisionRadius) ||
		pDesc->fCollisionRadius < 0.f)
	{
		return E_INVALIDARG;
	}

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components(pDesc)))
		return E_FAIL;

	m_pTransformCom->Set_State(STATE::POSITION,
		XMVectorSet(pDesc->vPosition.x, pDesc->vPosition.y, pDesc->vPosition.z, 1.f));
	if (0.f != pDesc->fYawDegree)
		m_pTransformCom->Rotation(0.f, pDesc->fYawDegree, 0.f);
	if (nullptr != m_pColliderCom)
	{
		m_pColliderCom->Update(XMMatrixTranslationFromVector(
			m_pTransformCom->Get_State(STATE::POSITION)));
	}

	/* With no animation set the bone palette is never filled, so every vertex
	collapses onto the origin and the NPC simply vanishes -- a wrong clip name
	looks exactly like a failed load. Fall back to the model's first clip so a
	bad name is visible as a wrong pose instead of nothing at all. */
	if (nullptr == pDesc->pIdleClip ||
		!m_pModelCom->Set_Animation(pDesc->pIdleClip, pDesc->isLoop))
	{
		if (0 == m_pModelCom->Get_NumAnimations())
			return E_FAIL;
		m_pModelCom->Set_Animation(0u, pDesc->isLoop);
	}

	return S_OK;
}

bool_t CNpc::Set_Animation(const char_t* pClipName, bool_t isLoop)
{
	if (nullptr == pClipName || nullptr == m_pModelCom)
		return false;
	if (!m_pModelCom->Set_Animation(pClipName, isLoop))
		return false;
	CEffectV2Runtime::Notify_NpcClip(
		static_pointer_cast<CNpc>(shared_from_this()), pClipName);
	return true;
}

bool_t CNpc::Apply_NetworkState(
	const float3_t& position,
	const f32_t yawDegrees)
{
	if (nullptr == m_pTransformCom ||
		!std::isfinite(position.x) ||
		!std::isfinite(position.y) ||
		!std::isfinite(position.z) ||
		!std::isfinite(yawDegrees))
	{
		return false;
	}
	m_pTransformCom->Set_State(
		STATE::POSITION,
		XMVectorSet(position.x, position.y, position.z, 1.f));
	m_pTransformCom->Rotation(0.f, yawDegrees, 0.f);
	if (nullptr != m_pColliderCom)
	{
		m_pColliderCom->Update(XMMatrixTranslationFromVector(
			m_pTransformCom->Get_State(STATE::POSITION)));
	}
	return true;
}

void CNpc::Trigger_HitFlash()
{
	m_fHitFlashRemainingSeconds = HIT_FLASH_DURATION_SECONDS;
	m_HitFlash.isEnabled = true;
	m_HitFlash.vColor = float4_t(1.f, 1.f, 1.f, 1.f);
	m_HitFlash.fIntensity = HIT_FLASH_PEAK_INTENSITY;
	m_HitFlash.usesSurfaceDetailMask = true;
}

void CNpc::Priority_Update(f32_t fTimeDelta)
{
}

void CNpc::Update(f32_t fTimeDelta)
{
	m_pModelCom->Play_Animation(fTimeDelta);
	CEffectV2Runtime::Tick_Npc(
		static_pointer_cast<CNpc>(shared_from_this()), m_pDevice, m_pContext);
	if (m_fHitFlashRemainingSeconds > 0.f)
	{
		m_fHitFlashRemainingSeconds -= fTimeDelta;
		if (m_fHitFlashRemainingSeconds <= 0.f)
		{
			m_fHitFlashRemainingSeconds = 0.f;
			m_HitFlash = {};
		}
		else
		{
			m_HitFlash.fIntensity = HIT_FLASH_PEAK_INTENSITY *
				(m_fHitFlashRemainingSeconds / HIT_FLASH_DURATION_SECONDS);
		}
	}
}

void CNpc::Late_Update(f32_t fTimeDelta)
{
	CGameInstance::Get().Add_RenderObject(
		RENDERGROUP::NONBLEND,
		static_pointer_cast<CGameObject>(shared_from_this()));
#ifdef _DEBUG
	if (m_isCombatColliderDebugVisible && nullptr != m_pColliderCom)
		CGameInstance::Get().Add_DebugComponent(m_pColliderCom);
#endif
}

HRESULT CNpc::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	const uint32_t iNumMeshes = m_pModelCom->Get_NumMeshes();
	for (uint32_t i = 0; i < iNumMeshes; ++i)
	{
		if (FAILED(Bind_DeferredMaterialInputs(
				*m_pModelCom, m_pShaderCom, i, {}, &m_HitFlash)) ||
			FAILED(m_pModelCom->Bind_BoneMatrices(
				m_pShaderCom, "g_BoneMatrices", i)) ||
			FAILED(m_pShaderCom->Begin(0)) ||
			FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}
	return S_OK;
}

HRESULT CNpc::Ready_Components(const NPC_DESC* pDesc)
{
	if (FAILED(__super::Add_Component(
		pDesc->iPrototypeLevelIndex,
		pDesc->strShaderTag,
		TEXT("Com_Shader"),
		m_pShaderCom)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(
		pDesc->iPrototypeLevelIndex,
		pDesc->strModelTag,
		TEXT("Com_Model"),
		m_pModelCom)))
		return E_FAIL;
	m_strModelTag = pDesc->strModelTag;

	if (pDesc->fCollisionRadius > 0.f)
	{
		Engine::CBounding_Sphere::BOUNDING_SPHERE_DESC colliderDesc{};
		colliderDesc.vCenter = float3_t(
			0.f, pDesc->fCollisionRadius, 0.f);
		colliderDesc.fRadius = pDesc->fCollisionRadius;
		if (FAILED(__super::Add_Component(
			pDesc->iPrototypeLevelIndex,
			TEXT("Prototype_Component_Collider_WorldEntity"),
			TEXT("Com_CombatCollider"),
			m_pColliderCom,
			&colliderDesc)))
		{
			return E_FAIL;
		}
	}

	return S_OK;
}

HRESULT CNpc::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")) ||
		FAILED(CGameInstance::Get().Bind_Transform(
			m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW)) ||
		FAILED(CGameInstance::Get().Bind_Transform(
			m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ)))
		return E_FAIL;
	return S_OK;
}

unique_ptr<CNpc> CNpc::Create(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto pInstance = unique_ptr<CNpc>(new CNpc(pDevice, pContext));

	if (FAILED(pInstance->Initialize_Prototype()))
		MSG_BOX("Failed to Created : CNpc");

	return move(pInstance);
}

shared_ptr<CPrototype> CNpc::Clone(void* pArg)
{
	auto pInstance = shared_ptr<CNpc>(new CNpc(*this));

	if (FAILED(pInstance->Initialize(pArg)))
		MSG_BOX("Failed to Cloned : CNpc");

	return pInstance;
}
```

### 기존 파일 변경 (MainApp.h / MainApp.cpp / Client.vcxproj / Client.vcxproj.filters, 누적 diff vs HEAD)

```diff
diff --git a/Client/Default/Client.vcxproj b/Client/Default/Client.vcxproj
index 8a692e5c..b0b09706 100644
--- a/Client/Default/Client.vcxproj
+++ b/Client/Default/Client.vcxproj
@@ -121,6 +121,7 @@
     <ClInclude Include="..\Public\Client_Defines.h" />
     <ClInclude Include="..\public\Effect_Tool.h" />
     <ClInclude Include="..\Public\Effect_Tool_V2.h" />
+    <ClInclude Include="..\Public\Effect_Preview_V2.h" />
     <ClInclude Include="..\Public\HUDLayoutTool.h" />
     <ClInclude Include="..\Public\Level_Loading.h" />
     <ClInclude Include="..\public\Loader.h" />
@@ -250,6 +251,7 @@
       <AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>
     </ClCompile>
     <ClCompile Include="..\Private\Effect_Tool_V2.cpp" />
+    <ClCompile Include="..\Private\Effect_Preview_V2.cpp" />
     <ClCompile Include="..\Private\HUDLayoutTool.cpp" />
     <ClCompile Include="..\Private\Level_Loading.cpp" />
     <ClCompile Include="..\private\Loader.cpp" />
diff --git a/Client/Default/Client.vcxproj.filters b/Client/Default/Client.vcxproj.filters
index d40493c0..a01b058e 100644
--- a/Client/Default/Client.vcxproj.filters
+++ b/Client/Default/Client.vcxproj.filters
@@ -381,6 +381,9 @@
     <ClCompile Include="..\Private\Effect_Tool_V2.cpp">
       <Filter>03. Tools\06. Effect V2</Filter>
     </ClCompile>
+    <ClCompile Include="..\Private\Effect_Preview_V2.cpp">
+      <Filter>03. Tools\06. Effect V2</Filter>
+    </ClCompile>
     <ClCompile Include="..\Private\Valtan.cpp">
       <Filter>02.GameObjects\01. Boss</Filter>
     </ClCompile>
@@ -715,6 +718,9 @@
     <ClInclude Include="..\Public\Effect_Tool_V2.h">
       <Filter>03. Tools\06. Effect V2</Filter>
     </ClInclude>
+    <ClInclude Include="..\Public\Effect_Preview_V2.h">
+      <Filter>03. Tools\06. Effect V2</Filter>
+    </ClInclude>
     <ClInclude Include="..\Public\Valtan.h">
       <Filter>02.GameObjects\01. Boss</Filter>
     </ClInclude>
```
