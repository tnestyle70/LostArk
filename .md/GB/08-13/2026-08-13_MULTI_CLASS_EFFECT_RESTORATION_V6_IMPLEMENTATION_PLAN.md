# 2026-08-13 멀티 클래스 이펙트 복원 V6 구현 계획

## 2026-08-14 Authoring-first 최종 전환

이 계획의 최종 제품 정본은 사용자가 직접 조합하고 튜닝해 저장한 authored `.effect.json`이다.
Track A에서 회수한 ShaderCache, source occurrence, renderer family, material lane과 exact slot 정보는
이 문서를 만들기 위한 **read-only preset/evidence provider**로만 사용한다. Track A artifact나 native
packet을 authored 결과의 권위로 두거나, 새 Effect마다 native VF/MRT와 shader permutation을 끝까지
복원하는 것을 완료 조건으로 삼지 않는다.

최종 저작 흐름은 다음 하나로 고정한다.

`Domain(DimensionMaster / Artist / LanceMaster / Valtan / Warlord)`
→ `Family(Mesh / Sprite / MeshParticle / SpriteParticle / LocalDecal / TrailRibbon)`
→ `WModel 및 DDS named slot 선택`
→ `Create Effect 또는 Add Element`
→ `Effect / Family / stable Element tree`
→ `Element 선택 및 명시적 Solo`
→ `Effect Details 전체 편집`
→ `Save / Reload`

한 effect는 여러 family의 여러 stable element를 소유한다. element 선택은 편집 대상만 바꾸며 합성
재생을 유지하고, `Solo`를 명시했을 때만 해당 element 하나를 격리한다. Effect Details는 위치·회전·
스케일·속도·색·alpha clip·emissive·distortion·dissolve·UV·sequence·timing과 family별 particle,
decal, trail 값을 편집한다. 각 named slot에는 현재 바인딩된 Resources-relative WModel/DDS asset ID를
정확히 표시하고, Track A evidence가 제공하는 role/register/channel/sampler/hash는 read-only 근거로
함께 보여준다.

LocalDecal과 TrailRibbon의 authored element는 기존 공용 projector/trail carrier로 실행할 수 있으면
충분하다. native FLocalDecal MRT/VF나 CascadeRibbon exact semantics의 완전한 재현은 필수 acceptance가
아니다. preset 적용은 공용으로 표현 가능한 resource binding, Detail, material 초기값만 복사하며 source
occurrence identity, native renderer packet과 adapter token을 authored 정본에 섞지 않는다. 클래스명,
skill ID, order, material path 하드코딩으로 이 흐름을 분기하지 않는다.

### Acceptance

- 다섯 Domain에서 WModel/DDS를 물리 `Resources/Effect` catalog 기준으로 선택할 수 있다.
- 여섯 Family 각각에 대해 필수 named slot을 검증하고 새 effect의 첫 element 또는 활성 effect의 추가
  element를 생성할 수 있다.
- 하나의 authored `.effect.json`에 서로 다른 family의 element를 조합하고 stable ID로 재선택한다.
- Effect / Family / Element tree에서 선택과 Solo를 분리하고 전체 합성 재생으로 복귀할 수 있다.
- 선택한 element의 전체 Effect Details와 named slot을 편집한 뒤 Save하고, Reload 후 동일한 ID, binding,
  세부 수치와 element 조합이 복원된다.
- 잘못된 경로·resource 종류·중복 ID·필수 slot 누락·stale writer·preview stage 실패는 기존 문서와
  preview를 보존하고 부분 저장하지 않는다.
- Track A preset/evidence를 사용하지 않아도 수동 저작이 가능하며, 사용하더라도 원본 artifact와 native
  packet은 변하지 않는다.
- Debug/Release focused harness, Effects publisher validation, Client build를 통과한 뒤 사용자가 Effect
  Tool에서 직접 생성·Solo·튜닝·Save·Reload를 육안 검증한다. 사용자 확인 전 visual PASS로 기록하지
  않는다.

### 구현 checkpoint와 남은 완료 경계

Authoring-first 코드 경계는 구현됐다. `Resources/Effect`의 다섯 resource domain
(`DimensionMaster`, `Artist`, `LanceMaster`, `Warlord`, `Valtan`)에서 실제 browser 지원 입력
`445 WModel + 2,234 DDS`를 선택한다. 수동 생성 family는 `Mesh`, `Sprite`, `MeshParticle`,
`SpriteParticle`, `LocalDecal`, `Trail/Ribbon` 여섯 가지다. `Create Effect`가 첫 stable element를 만들고,
`Add Element`가 같은 effect에 다른 family element를 더한다. All Effects는
`Active Effect -> Family -> Element` 트리로 열리며 row 선택은 Effect Details의 편집 대상만 바꾸고,
`Solo`, `Play Group`, `Play All`이 재생 격리를 별도로 소유한다.

선택 element는 위치·회전·스케일·속도·색·emissive·distortion·dissolve·UV·timing과 family별 값을
Effect Details에서 편집하고 stable authored `.effect.json`으로 Save/Reload한다. 현재 editable named slot의
WModel/DDS와 별도로, Track A `SourceMaterial` evidence의 role/register/channel/sampler/hash를 exact source
DDS inspector에 read-only로 표시한다. preset/generic starting copy는 WModel, DDS, Material, Detail을
보존하되 source occurrence, native renderer packet, attachment, adapter token과 source rotation lane은
지워 authored 문서가 native packet 보존을 허위 주장하지 않게 한다.

자동 checkpoint는 Debug/Release Client build PASS, 양 구성 authoring-fast `failures 0`, runtime-fast
`23/23`, occurrence tuning `10/10`, reconstructed-material `24/24`이며 Python visual-runtime `12/12`,
visual-program corpus `10/10`, occurrence tuning `5/5`다. 따라서 남은 acceptance는 사용자가 직접
Create/Add, tree 선택, Solo/합성 복귀, Details 편집, Save/Reload와 실제 화면 fidelity를 판정하는 단계다.
사용자 서면 판정 전에는 visual PASS로 기록하지 않는다. `Warlord`와 `Valtan`은 resource authoring domain만
열렸으며 exact restored product content가 아니다. shared dirty worktree에서 ProjectAudit 도구가 기존부터
삭제된 상태라 실행하지 않았고 복원하거나 PASS로 대체하지 않는다.

binary artifact는 authored `.effect.json`이 확정된 뒤의 배포·로딩·draw submission 최적화다. binary
packing, resource sharing, instance/buffer reuse와 prewarm은 이 저작 계약을 대체하지 않으며 후속 성능
단계에서 측정값과 함께 닫는다.

## 2026-08-14 최종 범위 동결

이 계획의 주 개발축은 Track A의 native 복원 증명 확장이 아니라 stable Element authoring이다.
Track A 결과는 DDS/WModel resource role, source module 수치, material lane과 family adapter의 초기
레시피로 소비한다. All Effects의 family 아래 개별 stable Element를 선택해 기존 Effect Details와
Resource Library에서 편집하고, immutable import를 덮지 않은 Authored Copy 하나로 저장·재로드하는
경로를 완료 조건으로 삼는다. runtime binary는 이 authored document를 배포·최적화하는 산출물이다.

MeshParticle, SpriteParticle, LocalDecal, CascadeRibbon, AnimationTrail은 같은 저장/선택/UI 계약을
공유하지만 서로 다른 carrier adapter를 유지한다. 완전한 native shader/VF/MRT 추적은 이 계획의
완료 조건이 아니다. 클래스명, skill ID, order, material path switch를 generic consumer에 추가하지
않는다.

### 2026-08-14 구현 checkpoint

이 계획에서 실제 구현·게시한 범위는 visual program `13`개(세 캐릭터 BA 12단계 + 도화가 F adapter),
renderer row `135`개와 supplemental row `5`개다. canonical renderer 분모는 MeshParticle `41`,
SpriteParticle `69`, CascadeRibbon `4`, LocalDecal `4`, LightParticle `15`, ScreenPost `2`이며,
supplemental은 AnimationTrail `4`와 도화가 F CascadeRibbon `1`이다.

renderer row의 실제 admission은 `72/135`, fail-closed는 `63/135`다. 세 BA만 보면 admitted `70/133`,
fail-closed `63/133`이며 family별 admitted/total은 Mesh `30/41`, Sprite `36/69`, LocalDecal `2/4`,
CascadeRibbon `4/4`, Light `0/15`, ScreenPost `0/2`다. supplemental `5/5`는 Lance AnimationTrail `4`와
도화가 F CascadeRibbon `1`이다.

세 BA의 admitted `SOURCE_RECIPE_OVERLAY_V1` row는 기존 full Effect Details/Resource Library로 열리고
immutable projection을 `Save As Authored Copy`로 authored document에 저장한다. 도화가 F
`ADAPTER_PACKET_V1`은 `Play Core F`가 exact projection을 stage한 뒤 LocalDecal `#20/#21`과 Ribbon `#3`을
선택할 수 있다. exact adapter packet은 full Details에서 읽기 전용이고 direct Save As를 금지한다.
대신 선택한 Decal/Trail 하나를 **generic Authored starting copy**로 명시적으로 낮출 수 있다. 이 copy는
Detail/Material/ResourceBindings를 보존하지만 native projector/VF/6-SRV packet 보존을 주장하지 않으며,
저장 뒤 기존 Effect Tool의 편집·Save·Reload를 사용한다. exact runtime 위치·회전·스케일은 stable
occurrence tuning Save/Reload를 계속 사용한다.

catalog와 visual sidecar도 한 transaction으로 tracked 제품 위치에 게시했다. 현재 제품 범위는 세 캐릭터
BA와 도화가 F다. Warlord/Valtan은 fail-closed extension canary이고, 전체 캐릭터 content admission 완료가
아니다. 도화가 F Mesh `#4`, crack `#22`와 나머지 Core row는 이 adapter full-details bridge의 대상이
아니며 기존 occurrence transform/runtime 경계를 유지한다.

Track A 시각 경계는 자동 완료로 과장하지 않는다. Lance BA1 focused CPU 검증에서
CascadeRibbon/AnimationTrail의 두 점 이상 strip은 통과했지만 WARP probe는 draw 전 Engine 초기화
`HRESULT=0x80004005`로 중단됐고 CascadeRibbon material/resource도 `fallback-blocked`다. LocalDecal은
`native=false`인 bounded projector/six-SRV adapter다. 둘 다 사용자 visual 판정 전에는 PASS가 아니다.

## 2026-08-13 최종 인터뷰 결정

V6의 중심은 원본 ShaderMap을 끝없이 추적하는 일이 아니다. Track A에서 얻은 source family,
module, material, transform 근거를 기존 Effect Tool의 개별 element authoring 흐름에 연결해 하나의
완성 이펙트를 만드는 것이 중심이다.

- **Track A 종료선**: 도화가 F `CascadeRibbon #3`, 창술사 BA의 실제
  `ParticleModuleTypeDataRibbon`과 별도 animation `Trails` notify, 공용 bounded LocalDecal
  `#20/#21`, 세 캐릭터 BA 12단계를 자동 검증 뒤 사용자가 직접 눈으로 판정할 수 있게 만든다.
- **Track B 중심 계약**: All Effects에서 원본 typed data로 판정한 family 이름을 그대로 표시하고,
  family 아래 stable 개별 element를 선택·명시적 Solo·Effect Details 편집·Save·Load할 수 있게 한다.
- **Track A 이후**: 위 사용자 판정이 끝나면 Track A는 중단이 아니라 보류한다. source evidence는
  이후 element의 기본값과 adapter 입력으로 계속 사용하되, native VF/MRT를 증명하기 위해 몇 시간씩
  확장 조사를 반복하지 않는다.
- **확장**: 같은 schema와 adapter를 4캐릭터와 Valtan에 적용한다. class 이름, skill ID, order,
  material path switch를 공용 consumer에 넣지 않는다.
- **최적화**: binary는 authoring 결과의 배포 최적화다. 최종 authoring 경로가 닫힌 뒤 draw submission,
  resource sharing, instance/buffer reuse, prewarm과 binary packing을 별도 성능 단계에서 닫는다.

선택과 Solo는 분리한다. stable element click은 Effect Details의 편집 대상만 바꾸고 현재 All/Family
합성 재생을 유지한다. 사용자가 명시적으로 `Solo`를 눌렀을 때만 해당 element 하나를 재생하며,
`Return to Family`와 `Return to All`로 합성 범위를 복원한다.

## 목표

V6는 도화가 31470 F V5에서 회수한 source occurrence, ShaderCache, VF, material wire,
attachment basis 계약을 한 캐릭터의 C++ order switch에 복제하지 않고 다음 네 소비자로
승격한다.

1. MeshParticle의 source 전방축과 carrier geometry 축을 분리하고 occurrence data가 소유하는
   회전을 정확히 한 번 적용한다.
2. Effect Tool의 All Effects/family/stable occurrence 선택에서 위치, 회전, 스케일 override를
   편집하고 authoring JSON에 저장·재로드한다.
3. LocalDecal #20/#21의 projector/VF/CB/6-SRV/pass 계약을 공용 typed LocalDecal carrier로
   연결한다.
4. 차원술사 BA, 도화가 BA, 창술사 BA를 공용 visual-program artifact와 클래스별 source data로
   publish한다. 창술사 BA의 Mesh, CascadeRibbon, animation Trails notify를 서로 다른 element로
   보존한다.

사용자 첨부 이미지는 결함 진단 입력일 뿐 자동 admission이나 visual PASS가 아니다. Client/UI는
사용자가 직접 조작하며 에이전트는 빌드와 headless 수치 검증까지만 수행한다.

## V5 고정 기준선

- Debug ClientFrontendHarness 빌드: PASS
- reconstructed material: 21/21, failures 0
- WARP reconstructed GPU material: failures 0
- catalog: 27,147,692 bytes,
  SHA-256 `1029365468dfb9fb4e17c166a2f2fd5d4870ec87121267d1bc74cf8b17f64460`
- Core33: draw 27, fail-closed 6, non-consumer 2
- #16, #20, #21은 evidence를 보존하되 draw 전에 occurrence 단위로 억제한다.

V6의 각 수직 슬라이스는 이 기준선을 유지하고 자체 정상/변조/rollback 검증을 추가한다.

## G00. 공용 visual-program artifact

### 계약

- stable key는 effect asset ID + occurrence ID이며 pointer, vector index, UI row는 저장 ID가 아니다.
- program admission은 occurrence, emitter, recipe, family, renderer/VF, static set, shader identity,
  resource role을 모두 결합한다.
- class/import basis, carrier axis, material packet은 artifact data가 소유하고 generic renderer는
  이를 소비한다.
- Artist 31470 전용 registry와 DocumentRenderer order switch를 다른 클래스에 복사하지 않는다.
- parse -> validate -> stage -> commit을 따르고 하나라도 실패하면 이전 catalog와 preview를 유지한다.
- family 판정은 object/material 이름이 아니라 typed data가 우선한다.
  `ParticleModuleTypeDataMesh -> MeshParticle`, `ParticleModuleTypeDataRibbon -> CascadeRibbon`으로
  판정하며, animation notify의 `Trails`는 별도 `AnimationTrail` family다.
- UI canonical family 이름은 `MeshParticle`, `SpriteParticle`, `LocalDecal`, `CascadeRibbon`,
  `AnimationTrail`, `LightParticle`, `ScreenPost`다. source가 증명하지 않은 family로 화면 인상만 보고
  승격하지 않는다.

### 완료 조건

- 0도와 비0도 import basis fixture가 같은 consumer를 사용한다.
- 다른 캐릭터가 같은 material path를 사용해도 occurrence admission 없이 opcode/transform이
  전파되지 않는다.
- 중복 stable ID, 모르는 VF/pass/resource role, 비유한 수치, SHA 변조를 transactionally 거부한다.

## G01. MeshParticle carrier 전방축

### 구현 경계

- 이미 교정된 snapshot-root 위치 basis와 Sprite/Decal transform은 변경하지 않는다.
- source emitter rotation, particle mesh rotation, WModel bind/import basis, geometry principal axis,
  renderer world 행렬을 각각 측정한다.
- 필요한 회전은 generic typed carrier-axis transform으로 표현하고 occurrence data가 값을 소유한다.
- global particle yaw, material path 분기, Artist order 번호 분기, WModel vertex 영구 회전은 금지한다.

### 완료 조건

- 도화가 F mesh 중심 위치는 V5와 동일하고 진행축만 source 전방과 일치한다.
- 사용자 canary는 `source-active-004` / `Par_V_SMD_OneStroke_Weapon_01.particlespriteemitter_6`
  하나를 Solo한다. cooked `ParticleModuleTypeDataMesh`의 stable module
  `FX_PC_SDM_07:export:1289@ref:9`에는 literal `yaw=90.0`이 있으며,
  `fm_v_wp_wsdm_base_01.wmodel` carrier에만 `[roll,pitch,yaw]=[0,0,90]`으로 적용한다.
- Sprite, Decal, Ribbon의 world position/orientation canary가 bit/epsilon 범위에서 불변이다.
- 0도/90도/잘못된 비유한 carrier-axis fixture와 행렬 합성 1회성을 harness가 검증한다.

## G02. Effect Details stable occurrence override

### authoring 계약

- All Effects의 ALL/family 선택과 명시적 element Solo는 preview 범위이고 저장 owner가 아니다.
- 편집 owner는 effect asset ID + stable occurrence ID이다.
- override는 position, rotationDegrees, scale을 각각 optional field로 저장하며 단위와 좌표계가
  schema에 명시된다.
- 원본 imported/candidate와 published runtime catalog를 직접 편집하지 않는다.
- 전용 authoring JSON을 publisher가 source transform 뒤의 명시적 project-tuned 단계로 합성한다.

### Tool 흐름

1. stable occurrence를 선택한다.
2. `Source`(읽기 전용), `Override / PROJECT_TUNED`(선택적 편집), `Effective`(읽기 전용)를
   구분해 표시한다.
3. Apply는 메모리 stage/preview만 변경한다.
4. Save는 임시 파일 parse/validate 후 원자적으로 authoring JSON을 교체한다.
5. Reload 실패는 기존 active document/preview를 유지한다.
6. Reset은 해당 occurrence override만 제거한다.
7. element click은 편집 대상만 바꾸며 암묵적 Solo를 하지 않는다. 별도 Solo 명령이 현재 preview
   범위를 바꾼다.

### 완료 조건

- ALL/family/occurrence 전환 뒤에도 stable ID로 같은 값을 편집한다.
- family 아래 모든 element가 원본 family 이름과 stable occurrence ID로 보이며 개별 Solo와 합성 복귀가
  가능하다.
- 저장·재로드·publisher·runtime preview가 동일한 행렬을 만든다.
- 모르는 effect/occurrence, 중복 row, 비유한 값, 0 scale, 부분 write 실패를 거부하고 rollback한다.

## G03. LocalDecal #20/#21

### 회수된 정본

- FLocalDecal VS `5d79421dc8571c45aa49790f50274f51`,
  DXBC `94072a22ef44ce0319bc9bd7915bded5f7ce94d9a5badde32def4fc01a4bff4d`
- PS `ef68ae7aec8f94458ef2cbb3c6bafd2d`,
  DXBC `d5a1d55021ff7e2a06e4de978e6850da56b9ff3ba6c7f68321f04852bd28ff1c`
- SRV: t0 height, t1 diffuse, t2 fixed dissolve/decal, t3 normal, t4 spec, t5 emissive
- native outputs: Target0, Target2, Target3, Target4, Target5

### 최종 선택과 구현 경계

- 기존 fullscreen Rect decal을 native LocalDecal로 오인하지 않는다.
- V6 Track A 종료선은 공용 projector와 6-SRV를 소비하는 **bounded RT0 semantic replay**다.
  native FLocalDecal VF, fog/custom-light CB, tangent parallax, Target2~5 MRT exact replay는 이후
  Track A backlog로 보류하고 V6 완료 조건으로 두지 않는다.
- #22 crack은 별도 occurrence와 exact DDS를 그대로 유지한다.
- parent/fixed DDS는 Artist-owned runtime asset ID와 exact payload receipt로 publish한다.

### 완료 조건

- #20/#21만 stable occurrence admission으로 보이며 #22 결과는 불변이다.
- projector outside/depth slab/fade/lifetime/opacity와 six-SRV channel role을 headless harness로 검증한다.
- state, CB, VF, MRT 중 하나라도 불명확하면 native/exact로 표현하지 않고 occurrence를 계속 억제한다.

## G04. CascadeRibbon과 AnimationTrail

### 정정된 source truth

기존 V6 corpus의 `Trail/Ribbon 0` 집계는 잘못됐다. 창술사 BA의
`FX_PC_FLM_01.Par_M_FLM_Ribbon_02.particlespriteemitter_0`에는 실제
`ParticleModuleTypeDataRibbon`이 있으며 BA1~BA4에 한 번씩 schedule된다. importer가 legacy
`rendererShape=sprite`를 우선해 네 행을 Sprite로 오분류했다.

창술사 BA의 화면 구성은 다음 세 element 층이다.

1. Main Mesh: `Par_M_FLM_PyungMTrail_01.particlespriteemitter_4`,
   `ParticleModuleTypeDataMesh`, `fm_m_ring_001.wmodel`, trail/dissolve material.
2. CascadeRibbon: `Par_M_FLM_Ribbon_02.particlespriteemitter_0`,
   `ParticleModuleTypeDataRibbon`.
3. AnimationTrail: `Par_M_FLM_Trail_03` 또는 `_2`를 참조하는 animation `Trails` notify.

현재 135 renderer-row family 분모는 `Mesh 41 / Sprite 69 / CascadeRibbon 4 / Decal 4 /
Light 15 / ScreenPost 2`로 정정한다. 별도 AnimationTrail notify 4건은 renderer row와 섞지 않고
schedule denominator로 따로 관리하며 adapter 구현 뒤 제품 denominator를 확정한다.

차원술사와 도화가 BA에는 현재 TypeDataRibbon 근거가 없다. material 이름의 `trail`이나 화면의
호 형태만으로 Ribbon으로 승격하지 않는다.

### 완료 조건

- 도화가 F `#3`과 창술사 BA1~BA4 TypeDataRibbon이 같은 공용 CascadeRibbon adapter를 소비한다.
- 창술사 BA의 AnimationTrail notify 네 건은 TypeDataRibbon과 별도 stable element·schedule로 실행된다.
- Main Mesh, CascadeRibbon, AnimationTrail을 각각 Solo하고 Combined로 복귀할 수 있다.
- 초기 1점 suppression은 허용하지만 움직이는 anchor에서 2점 이상 strip submission을 만들고,
  불투명 검은 폐곡선·white card·NaN spike가 없어야 한다.
- material은 bounded reconstruction으로 표기하며 사용자가 원본과 비교해 `APPROVED / RETUNE /
  BLOCKED`를 결정한다.

## G05. 차원술사 BA, 도화가 BA, 창술사 BA

### 데이터 분리

- 애니메이션 연결은 authored skillbindings와 BA stage/notify가 소유한다.
- effect program은 source occurrence, attachment, renderer family, material/resource packet을 소유한다.
- class/skill/stage별 수치는 artifact row로 publish하고 generic consumer는 class 이름이나 skill ID를
  switch하지 않는다.

### 구현 순서

1. 세 BA의 skill ID, authored clip, stage/notify, source effect, stable occurrence inventory를 만든다.
2. 기존 runtime에서 실제 연결된 row와 legacy reference-only row를 구분한다.
3. typed data 우선 family resolver로 창술사 Ribbon 오분류를 먼저 교정한다.
4. 공용 carrier로 처리 가능한 Mesh/Sprite/CascadeRibbon/AnimationTrail/LocalDecal을 publish한다.
5. V6 종료에 꼭 필요한 누락 resource/material packet만 제한적으로 획득한다. native exact 추적은
   종료 조건으로 확장하지 않는다.
6. 불완전 occurrence는 정확한 blocker와 evidence를 보존한 채 개별 fail-closed한다.
7. 각 BA의 zero-time, active-time, next-fixed-step family draw와 action timing을 검증한다.

### 완료 조건

- 세 BA 모두 Server 승인 action -> authored animation -> notify -> effect program -> renderer의 한 경로를 쓴다.
- Artist 전용 C++ registry/order packet을 복사하지 않는다.
- 클래스별 occurrence의 위치·회전·스케일·material은 source data 또는 명시적 PROJECT_TUNED override로
  추적 가능하다.

## G06. 세션 분할과 Captain 계약

### Captain / integration

- PLAN/RESULT와 분모를 동결하고 Catalog, sidecar, Presentation, publisher의 단일 transaction을 소유한다.
- 각 구현 세션의 public token과 rollback harness만 결합하며 class/order switch를 허용하지 않는다.

### Ribbon·LocalDecal

- typed-data family resolver, 공용 CascadeRibbon, AnimationTrail, bounded LocalDecal 6-SRV adapter를
  소유한다.
- Artist F와 창술사 BA를 golden fixture로 사용하고 native exact 탐사를 V6 범위 밖으로 유지한다.

### BA runtime

- 3개 BA 12단계의 authored animation -> notify -> visual program -> common Playback/Object/Renderer를
  소유한다.
- Main Mesh, Ribbon, AnimationTrail target closure와 fail-closed rollback을 검증한다.

### Effect Tool

- 원본 family 트리, stable element 선택, 명시적 Solo, Source/Override/Effective, Apply/Reset/Save/Load를
  소유한다.
- family/order/vector index가 아니라 stable effect asset ID + occurrence ID + source row SHA를 저장한다.

## Track A 종료와 이후 순서

1. Artist F Ribbon, 창술사 BA Ribbon/AnimationTrail, LocalDecal, 3 BA 12단계를 자동 검증한다.
2. 사용자가 Artist F Mesh `#4` forward-axis, Ribbon `#3`, LocalDecal `#20/#21`, crack `#22`,
   창술사 BA1~BA4의 Main Mesh/CascadeRibbon/AnimationTrail/Combined를 직접 눈으로 판정한다.
3. 승인 또는 명시적 PROJECT_TUNED 값이 저장되면 Track A를 보류 상태로 전환한다.
4. Track B element authoring을 4캐릭터와 Valtan corpus로 확장한다.
5. authoring 계약이 안정된 뒤 binary packing과 제품 draw/resource 최적화를 수행한다.

## 검증

1. 관련 Python builder/unit과 JSON parse/check
2. Debug/Release ClientFrontendHarness 독립 rebuild
3. source/runtime program fast, executor, material gate
4. GPU/WARP는 사용자에게 Debug assertion을 발생시킨 경로를 자동 재실행하지 않는다. 이번 최종
   checkpoint는 CPU/headless Debug/Release gate만 실행하고 시각 판정은 사용자가 수행한다.
5. 관련 HLSL fxc
6. `git diff --check`
7. 사용자의 Debug Client 수동 eye test

ProjectAudit 파일은 shared dirty worktree에서 기존부터 삭제된 상태이므로 V6 완료 증거로 실행하거나
복원했다고 주장하지 않는다. 자동 검증과 사용자 visual 판정은 RESULT에서 분리한다.
