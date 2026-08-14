# 2026-08-14 Track C authored material·portable particle execution 구현 결과

기준일: 2026-08-14 KST

## 결론

Artist F의 Track A material과 Particle 실행 정보를 하나의 ordinary authored v13
`.effect.json`에 저장하고 `CEffectObject::Stage_Document` 경로가 직접 소비하도록 연결했다.
Track A sidecar와 reconstructed preparation은 import·복구 근거이며 저장 후 Effect Tool 재생의
runtime 권위가 아니다.

물리 `Data/Effects/Authored/effect.artist.skill.31470.unified.effect.json`에는 사용자가 편집한
Element ID, Visible, Transform, DDS binding과 LocalDecal을 보존한 채 typed material packet과
Mesh/Sprite 29개의 portable `SourceRecipe`가 실제 저장돼 있다. 별도 Upgrade나 하네스 없이 unified
문서를 Load한 뒤 Play All/Family/Solo로 재생한다.

현재 자동 검증 기준 Artist F Particle 실행을 막는 P0는 발견되지 않았다. 다만 Client 화면을
에이전트가 실행하지 않았으므로 원본 대비 visual fidelity와 Play All 최종 판정은 사용자의 수동
검증으로 남긴다.

## 현재 authored 정본

| 항목 | 현재 값 |
|---|---:|
| schema | ordinary authored v13 |
| canonical file SHA-256 | `cfd83757e76529bfb82cc48d90724fd7eba32db2ed7a9954fb4a293462fdb94d` |
| 파일 크기 | `1,367,152` bytes |
| Elements | `33` = Particle `29` + Trail `1` + Decal `3` |
| visible / hidden | `28 / 5` |
| portable Particle recipes | `29` |
| portable modules | `350` |
| portable distributions | `564` |
| portable literals | `1,415` |
| typed material | `28` = ArtistVisualV4 `10` + RuntimeMaterialV2 `16` + LocalDecal `2` |
| FiniteCommon / persistent fail-closed | `1 / 4` |
| fixed burst | `26` emitters / total `167` |
| Mesh WModel pre-scale | `13/13`, `0.01` |
| transform basis | root snapshot `28`, follow emit-start bake `5` |
| seeded modules | `14/14` seed-policy identity 일치 |

Persistent fail-closed는 `#1/#16/#26/#33`이다. 이 네 행은 source recipe를 보존하지만 native
SceneColor/depth/fog/MRT 계약이 닫히지 않아 Material submission을 억제한다. `#20/#21`은 six-SRV
LocalDecal typed packet으로 열렸고 `#22`는 typed data를 보존한 hidden Decal이다.

이 문서의 `#5`, `#13`, `#20` 같은 번호는 JSON 배열 index가 아니라 Track A source emitter order다.

## 이번 세션에서 확인·수정한 버그

| 증상 | 원인 | 현재 반영 |
|---|---|---|
| Load/Play All/Solo 뒤 약 2초 정지하고 무재생 | UI thread의 reconstructed full prewarm 뒤 character 전용 full-row SHA가 provenance-only 재생성과 불일치하여 cache commit 전 실패 | 현재 authority 세대로 stale row를 동기화했고, unified Load/Play는 ordinary authored 경로로 source Acquire/prewarm을 우회한다. source preview의 literal row pin 자체는 P1로 남긴다. |
| ArtistVisualV4 Mesh/Sprite가 generic shader로 하강 | authored material은 opcode를 저장했지만 HLSL이 reconstructed evaluator flag만 검사 | Mesh/Particle shader가 `g_ArtistVisualV4Opcode != 0`도 직접 소비한다. |
| Mesh Particle이 순백 붓으로 출력 | WModel/DDS는 존재했지만 Track A StartColor/StartAlpha/ColorOverLife/ColorScaleOverLife가 authored 변환에서 삭제되어 identity white가 shader에 전달 | 29 Particle의 portable source module을 저장하고 particle color/alpha ABI를 opcode별 실제 소비 mask와 맞췄다. |
| Mesh Particle이 너무 작거나 보이지 않음 | 이미 단위 변환된 Particle size와 WModel `modelPreScale=0.01`이 이중 축소 | Mesh 13개의 dimensionless Start/End Size를 source 값으로 복원하고 model pre-scale은 정확히 한 번만 적용한다. |
| Sprite가 1px 수평선 또는 거의 zero coverage | CAMERA_SQUARE/velocity/axis alignment, per-axis size와 DynamicParameter가 generic rectangle/상수로 축약 | Required alignment, AxisLock, size/size-over-life, pivot/flip, DynamicParameter source distribution을 portable recipe로 복원했다. |
| Sprite 흰 사각 tile 또는 잘못된 frame | SubUV atlas 크기만 남고 interpolation/random/flip 계약을 SourceMaterial 유무에 묶음 | Required literal의 grid, `psuvim_linear_blend`/`psuvim_random`, flip을 Renderer와 runtime probe가 직접 소비한다. |
| Flow02 Mesh가 시작 약 1.38초에 Effect 전체를 격리 | opcode `7`이 요구하는 Dynamic `(1, 0..2, 1, 1)`이 authored에서 zero | source ParameterDynamic 4-lane curve를 보존해 renderer predicate를 만족시킨다. |
| Ribbon 두 번째 point부터 Play All 전체가 사라짐 | generic authored Ribbon opcode `9` point가 stored color/dynamic consumed mask 대신 source-only exact mask 검사를 통과해야 했음 | generic point는 저장된 alpha/dynamic mask를 사용하고 source-backed point만 기존 exact mask를 검사한다. |
| portable 재생 시작 시간이 늦어짐 | schedule+emitter delay가 Detail start에 이미 bake된 뒤 SourceRecipe emitter delay를 다시 적용 | portable recipe의 emitter delay를 `0`으로 평탄화한다. |
| HDR alpha/green tint가 과증폭 | 이전 bounded alpha-50/green Detail fallback과 복원된 source color curve가 이중 곱 | 이전 자동 생성 패턴과 정확히 일치할 때만 Detail color를 identity로 정규화하고 사용자 색 편집은 보존한다. |
| seeded module 분포가 실행마다 달라짐 | source literal이 nested seed array 또는 UE3 32-byte body인데 Playback은 canonical seed array만 조회 | 두 source 표현을 정식 seed policy로 해석하며 현재 F `14/14`가 Program과 일치한다. |
| Artist F tree를 펼치면 7 FPS, ImGui 클릭 불가 | Render 함수가 full material preparation과 DDS/WModel prewarm을 동기 실행하고 실패를 매 frame 재시도 | Render는 metadata/cache만 읽고 준비는 explicit action에서 한 번 수행한다. 실패도 runtime revision별로 latch하며 Refresh 때만 재시도한다. |
| Open for Editing 뒤 Effect Tool CPU 시간이 크게 증가 | unified drawable 검증과 33개 Family/Element tree를 매 frame 반복 | drawable readiness를 파일 revision에 캐시하고 Family/seed tree 기본 펼침을 제거했다. |
| schema 추가 뒤 Effect Catalog가 `typed-codec SHA is stale`로 로드 실패 | 기본값 DynamicParameter/model pre-scale을 legacy projected document에도 serialize | optional/non-default 값만 기록해 기존 typed-codec byte identity를 유지했다. |
| LocalDecal `#20/#21` material preparation 실패 | `fx_d_normal_078.dds`의 실제 ATI2/BC5를 BC3 및 BA channel로 해석 | renderer/parser/generator는 BC5, shader와 typed lane은 RG를 사용한다. |
| 사용자가 숨은 Upgrade를 찾아야 한다는 잘못된 안내 | migration 코드와 실제 authored artifact 적용을 혼동 | materializer가 사용자 Transform/Visible/DDS 편집을 보존해 JSON을 갱신하며, normal UI는 별도 Upgrade 없이 Load/Play한다. |

## 복원된 Track A Particle 정보

원본 native-v14 source의 Particle 29개는 `350 modules / 564 distributions / 1,415 literals`다.
현재 authored 문서의 portable projection도 같은 분모이며 executable 의미 payload가 일치한다.

- spawn rate, fixed burst, lifetime, emitter duration/loop
- location, direct/cylinder/cylinder-spin/sphere, ground offset
- velocity, radial velocity, acceleration, camera offset
- size, size-over-life
- particle rotation과 Mesh rotation/rate/over-life
- start color/alpha, color-over-life, color-scale-over-life
- DynamicParameter 4개 lane
- square/rectangle/velocity alignment, axis lock, pivot, image flip
- SubUV grid, frame distribution, linear-blend/random mode
- SpawnPerUnit과 deterministic seeded RNG

Material 실행은 SourceRecipe와 별도다. `Material.Execution`이 texture semantic lane, asset ID, `t#`,
`s#`, channel, color space, sampler, scalar/vector/color constants, pass와 render state를 소유한다.
enabled typed execution과 legacy SourceMaterial을 동시에 실행하지 않는다.

## 의도적으로 authored 문서에 넣지 않은 정보

portable seam은 실행 값과 source 증거를 분리한다. 다음 native-v14 필드는 authored runtime에 복사하지
않는다.

- source-contract/profile/graph/closure SHA
- compiler evidence와 execution/material admission receipt
- geometry authority와 local reference closure
- distribution reference/occurrence/fidelity/admission row
- ActionCue parameter binding. 값 없이 binding만 지우지 않고 해당 carrier를 거부한다.

원본 core 33의 non-particle graph는 아직 portable 저장 범위 밖이다.

| Family | source graph | 현재 authored portable graph |
|---|---:|---:|
| Particle 29 | `350 modules / 564 distributions / 1,415 literals` | 전부 보존 |
| Ribbon 1 | `8 / 13 / 43` | 미저장 |
| Decal 3 | `24 / 27 / 64` | 미저장 |

ScreenPost `#32`와 Light `#34`는 source 35에는 있지만 editable Core33 authored 문서에는 없다.
ModelCue는 source와 authored 모두 0이므로 유실 항목이 아니다. root 28과 follow 5는 emit-start pose로
bake되어 live weapon/bone follow는 복원되지 않았다.

## 실제 코드 연결

- `Client/Public/Effect_DocumentCodec.h`
  - portable recipe/module/distribution 통계와 carrier 이식 public 계약.
- `Client/Private/Effect_DocumentCodec.cpp`
  - 29-class/property capability manifest, transactional carrier 이식, Artist F upgrade/readiness 분모,
    ArtistVisualV4 particle color ABI, legacy fallback 정규화.
- `Client/Private/Effect_Playback.cpp`
  - effect ID 비의존 portable carrier 판별, source particle module 실행, SpawnPerUnit/LocationOnGround,
    alignment/pivot/flip, SubUV probe와 seed body 해석.
- `Client/Private/Effect_DocumentRenderer.cpp`
  - ordinary authored material staging, Required-based SubUV, typed execution GPU bind.
- `Client/Bin/ShaderFiles/Shader_VtxEffectMeshPreview.hlsl`
- `Client/Bin/ShaderFiles/Shader_VtxEffectParticle.hlsl`
  - authored ArtistVisualV4 opcode 진입.
- `Client/Private/Effect_Tool.cpp`
  - unified cache parse/validate, authored Load/Play All/Family/Solo와 별도 source preview 경계.
- `Tools/EffectPipeline/materialize_artist_31470_portable_particle_carriers.py`
  - 기존 authored 문서를 기준으로 사용자 Decal/Transform/Visible/DDS를 보존하고 29 Particle recipe만
    원자 교체하는 F 전용 offline publisher/materializer.

## 다른 캐릭터와 공유되는 범위

v13 SourceRecipe schema, portable codec helper, validator, Playback module interpreter, Required 기반 SubUV,
ordinary material staging은 effect/character ID를 보지 않는 공통 runtime이다. 같은 계약으로 JSON이
materialize된 다른 캐릭터는 이 경로를 그대로 소비할 수 있다.

그러나 현재 importer/materializer 전체가 범용화된 것은 아니다.

- F stable-ID join, Core33 분모, basis, `0.01`, shader registry/opcode와 fail-closed order는 F 전용이다.
- imported corpus의 normalized module class는 `51`종이고 현재 portable manifest는 F에서 증명한 `29`종만
  허용한다. 나머지 `22`종은 잘못 근사하지 않고 거부한다.
- 일반 `Build_GenericAuthoredElementStartingCopy`는 SourceRecipe를 지우고 일반 Bake/Merge/Create는
  carrier를 보존하지 않거나 거부한다.
- F materializer는 아직 `Publish-Effects.ps1`의 공통 캐릭터 compiler로 연결되지 않았다.

따라서 이번 변경은 공통 runtime 기반과 F의 첫 materialized vertical slice까지 완료한 상태이며,
다른 캐릭터의 `Create -> Bake -> Merge -> Save -> Reload -> Play` 전체 자동 변환 완료를 뜻하지 않는다.

## 자동 검증

현재 successor-owned WIP에 대해 후속 통합 세션이 기록한 자동 검증은 다음과 같다. 이 문서 동기화
세션은 인계 뒤 이를 다시 실행하지 않았다.

- `Client/Default/Client.vcxproj`, `Debug|x64`: compile/link PASS.
- portable materializer `--check`: PASS
  (`29 recipes / 350 modules / 564 distributions / ArtistVisualV4 10`).
- authored JSON SHA-256:
  `cfd83757e76529bfb82cc48d90724fd7eba32db2ed7a9954fb4a293462fdb94d`.
- portable seed policy identity: PASS (`14/14`, missing/extra/mismatch `0`).
- `Publish-Effects.ps1 -Mode Validate`: PASS
  (`102` Effect catalog entries, visual programs `13/135`).
- scoped `git diff --check`: PASS. 기존 LF/CRLF 안내만 존재한다.
- Client/UI와 ClientFrontendHarness는 이번 최종 검증에서 실행하지 않았다.
- 현재 Debug Harness는 `16:54:55 KST` 산출물로 최신 Playback/Renderer/Codec 수정 전이므로 현 WIP의
  실행 PASS 증거로 사용하지 않는다.

이전 Track C checkpoint에서 수행한 focused harness/build 결과는 당시 증거로 남기되, 이번 결과의 완료
근거로 다시 실행했다고 기록하지 않는다.

## 세션 종료와 shared worktree 기준

이 문서 동기화 세션이 마지막으로 독립 실행한 Ribbon P0 검증 기준은 material-only JSON
`B692AF906CF87E580EDE1538A471349A9A1541CF0C9D787B1736B66E2D056900`과 16:57 Debug
Client였다. 해당 기준에서 Client/Harness Debug build, source visual `8/8`, runtime-fast와 codec round-trip이
PASS했다.

그 뒤 별도 통합 세션이 4직업, Valtan Whirlwind와 성능 구현을 시작해 shared worktree를 갱신했다. 이
문서 종료 시점의 현재 물리 기준은 다음과 같으며, 이후 검증·수정 소유자는 새 통합 세션이다.

- authored JSON SHA-256: `CFD83757E76529BFB82CC48D90724FD7EBA32DB2ED7A9954FB4A293462FDB94D`
- authored JSON mtime/size: `2026-08-14 17:24:40 KST`, `1,367,152` bytes
- 현재 JSON 분모: Element `33`, typed `28`, FiniteCommon `1`, fail-closed `4`, portable Particle
  SourceRecipe `29`, burst `26/167`, Mesh pre-scale `13/13`
- Debug Client SHA-256: `A1E13167BD7159217951032FEA265F133893CC27C72CA4570B5E0BA989FEE3BA`
- Debug Client mtime: `2026-08-14 18:41:17 KST`

현재 snapshot은 후속 세션이 계속 변경할 수 있다. 따라서 다른 세션은 위 SHA가 달라지면 이 문서의
수치를 자동 승계하지 말고, 자신의 최종 JSON parse, publisher validation, Debug build와 사용자 수동
gate를 다시 기록한다. 이 세션은 인계 뒤 C++/JSON/빌드를 더 수정하지 않았다.

## Legacy 보존과 product 전환

- 현재 `.unified` 문서는 아직 gameplay product mapping이 아니다. 기존 Product Effect와 effectref는
  visual acceptance 전까지 그대로 유지한다.
- 4직업 확대는 기존 JSON을 덮어쓰거나 먼저 이동하지 않는다. skill/stage별 새 authored Effect ID를
  만들고 Track A/imported evidence를 materialize한 뒤 Effect Tool에서 편집한다.
- `Play All -> Family -> Solo -> 실제 animation anchor` 사용자 승인을 받은 skill/stage만 catalog와
  animevent effectref를 원자 전환한다.
- 전환 뒤 기존 ID/JSON은 Normal UI에서 숨긴 read-only Legacy/Rollback Reference로 보존한다.
  zero-reference와 rollback 검증 전에는 삭제하지 않는다.
- 현재 Track A 공용 입력은 DimensionMaster `2050010` BA1~BA4, Artist `31000` BA1~BA4,
  LanceMaster `34010` BA1~BA4와 Artist F다. Warlord는 동급 Visual Program이 없어 대표 스킬부터
  data-driven importer를 새로 닫아야 한다.
- Valtan은 keyboard skill이 아니라 `actionId -> phase/stage -> clip -> effectAssetId` 계약이 먼저다.
  현재 pattern binding을 곧바로 product Effect로 간주하지 않는다.

위 catalog/animevent 원자 전환, rollback receipt와 Legacy UI 격리는 아직 구현 완료가 아니라 rollout
정책이다. 현재 `.unified` 파일이 존재한다는 이유로 product 전환 완료를 주장하지 않는다.

## 사용자 수동 gate

1. Debug Client에서 `F1 -> Effect Tool -> All Effects -> Artist -> F`를 연다.
2. 별도 Upgrade 없이 `Ribbon -> Play Family`를 먼저 확인한다.
3. `Mesh Particle -> Play Family`, `Sprite Particle -> Play Family`, LocalDecal occurrence Solo를 확인한다.
4. 마지막에 `Play All`을 실행해 object-local isolation 없이 함께 재생되는지 확인한다.
5. Mesh가 흰 실루엣이 아니라 원본의 dark navy/black brush와 반투명 가장자리를 갖는지 확인한다.
6. Sprite가 수평선이 아니라 compact square/ink/smoke/spark 분포로 재생되는지 확인한다.
7. 사용자 Decal 위치·크기·DDS·Visible이 Save/Reload 뒤 유지되는지 확인한다.
8. 실패하면 exact family/occurrence, sample time과 render-isolation status를 보존한다.

사용자의 서면 관찰 전에는 manual first pixel, Play All visual PASS 또는 Artist F 원본 복원 완료로
승격하지 않는다.

## 남은 P0/P1

현재 저장된 Artist F authored 문서의 Particle 실행에는 자동 검증상 알려진 P0가 없다.

다른 캐릭터 공통 확장 기준 P0는 다음 하나다.

- generic Tool/compiler가 portable carrier를 `Create -> Bake -> Merge -> Save -> Reload` 전 구간에서
  보존하고 공통 publisher가 character별 stable identity/material capability를 data-driven으로 join해야 한다.

4직업/Valtan product rollout 성능 기준 P0는 다음과 같다.

- spawn hot path는 prepared handle을 사용하고 global cache mutex 안에서 whole-document Serialize/복사와
  DDS/WModel 생성이 일어나지 않아야 한다.
- MeshParticle의 particle × submesh × pass draw 폭증을 weighted budget/instancing/culling으로 제한하고,
  scene-wide active Effect/particle/trail/transparent draw budget을 둔다.
- 한 Effect first pixel이 아니라 4인 동시 스킬과 Valtan pattern의 CPU/GPU/frame-time stress gate를 통과해야 한다.

P1은 다음과 같다.

- Ribbon/Decal source module graph portable화.
- live weapon/bone follow와 history geometry.
- imported module class `51`종 중 아직 미지원인 `22`종과 literal capability manifest.
- ActionCue parameter-bound distribution.
- source overlay supplemental Decal/Visible과 strict `35/33` preview validator의 충돌 해소.
- reconstructed source preview에 남은 character 전용 full-row SHA literal gate 제거.
- product catalog/animevent mapping, Release/runtime/GPU/4인 성능 검증.
- 사용자 visual A/B와 서면 판정.
- Warlord/Valtan은 현재 executable Track A-ready가 아니다. 기존 authored 파일 또는 pattern binding 존재만으로
  source material/particle compiler와 product effect mapping이 닫혔다고 보지 않는다.
