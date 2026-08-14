# 2026-08-13 도화가 31470 F V4 복원 결과

## 결론

이번 변경은 V3처럼 문제 행을 숨긴 결과가 아니다. 35개 stable occurrence를 단일 registry로 고정하고,
Core33 각각을 `EXACT_CACHE_DXBC_SEMANTIC_REPLAY`, `BOUNDED_EXPLICIT`,
`UNRESOLVED_FAIL_CLOSED`, `NON_CORE_FORBIDDEN` 중 하나로 분류했다. 근거 없는 generic numeric
evaluator, white/black source fallback, 불투명 DDS alpha의 자동 coverage 사용은 Artist F 제품 draw
선택에서 제거했다.

현재 Core33 결과는 다음과 같다.

| 분류 | 수 | 의미 |
|---|---:|---|
| exact semantic replay | 7 | official cache의 static-set/PS/DXBC 식을 현재 HLSL로 재생한다. raw DXBC/native pass 직접 실행은 아니다. |
| bounded explicit | 21 | occurrence별 typed texture/channel/CB/output 식을 명시적으로 구현했지만 일부 native VF/pass/fog/aux MRT 근거는 닫히지 않았다. |
| unresolved fail-closed | 5 | 근거가 부족해 material·pass·draw 전에 occurrence만 억제한다. |
| non-core forbidden | 2 | `#32` ScreenPost와 `#34` PointLight. Core evaluator/consumer에 진입하지 않는다. |

따라서 Core33 중 28개가 registry가 승인한 식으로 draw되고, 5개는 잘못된 판을 그리는 대신
fail-closed 상태다. Product admission과 native selection admission은 계속 `false`다. 사용자 원본
이미지와의 최종 visual PASS도 아직 기록하지 않는다.

## V3 대비 실제 변경

- V3는 `#9/#10/#11` 세 행만 보이게 하고 나머지를 숨겼다.
- 기존 Core33 변경은 새 source shader 없이 scope만 3→33으로 넓혀 잘못된 generic carrier를 모두
  노출했다.
- V4는 stable occurrence + recipe + family + static-set identity를 단일 registry로 검증하고,
  occurrence별 source texture/channel/output 역할을 renderer가 fail-closed로 소비한다.
- Mesh 전체 0.01 재축소는 하지 않았다. geometry preScale 0.01과 dimensionless Mesh StartSize 계약은
  유지했다.
- `#12/#15` make-flow의 internal UV warp가 RT1 screen distortion으로 새지 않도록 RT0-only로
  고정했다.
- `#16`은 DDS container alpha를 coverage로 쓰지 않고 선택된 mask red만 소비한다.
- `#25/#29`는 map-D 내부 warp, map-alpha/map-A coverage, spec/color 역할을 분리했다.
- `#27`은 6개 exact DDS lane과 sampler scope를 연결하고 RT0-only bounded program으로 만들었다.
- `#0`은 recovered skull/radial/dissolve 식과 세 DDS의 red channel을 occurrence 전용 V4 opcode로
  연결했다.
- `#33`은 blue RT1 distortion이 아니라 SceneColor를 읽어 RT0에 합성하는 refraction PS임을
  확인했다. cb0[7], fog, actual VF/pass/state가 미확정이므로 잘못 구현하지 않고 억제했다.

## 런타임 구조

- registry 분모: exact 7 / bounded 21 / unresolved 5 / forbidden 2 / draw 28
- backend: Artist V4 10 / Runtime Material V2 17 / Finite Common 1 / None 7
- renderer 준비 분모: evaluator 27 / Runtime V2 17 / Artist V4 10 / suppressed 5
- family draw/suppress:
  - MeshParticle 12 / 1
  - SpriteParticle 14 / 2
  - DecalParticle 1 / 2
  - CascadeRibbon 1 / 0
- 28개 draw program은 모두 SceneColor RT0만 소유하며 RT1은 쓰지 않는다.
- actual native VF/pass admission은 0/35다. `exact`라는 이름은 raw DXBC bind가 아니라 recovered
  equation identity를 뜻한다.

## 남은 fail-closed 행

| occurrence | 이유 |
|---:|---|
| `#1` | exact program은 회수됐지만 source SubUV/dynamic particle VF와 live view/spec/fog semantic을 현재 carrier가 완전히 제공하지 못한다. |
| `#20/#21` | LocalDecal VF의 projector slab/fade/fog와 6 texture wire, actual pass admission이 필요하다. 현재 fullscreen decal adapter로 대체하지 않는다. |
| `#26` | source lit masked LocalVF, SV_Coverage/aux MRT/lighting과 ground-placement provider가 미완료다. |
| `#33` | exact SceneColor feedback PS의 cb0[7], fog factor, sampler/state, actual particle VF/pass가 미확정이다. RT1 fallback은 금지한다. |

이 다섯 행은 active/evaluated 시간축과 transform을 유지하지만 material/SRV/pass/VF/draw/submitted는
0이어야 한다.

## 자동 검증

실행한 검증만 기록한다.

- all-core cache extractor focused unit: 5/5 PASS
- missing-texture acquisition focused unit: 5/5 PASS
- Debug x64 ClientFrontendHarness build/link: PASS
- Release x64 ClientFrontendHarness build/link: PASS
- Debug `--effect-reconstructed-runtime-program-fast`: PASS, failures 0
- Debug WARP `--effect-reconstructed-gpu-material`: PASS, failures 0
- Release WARP `--effect-reconstructed-gpu-material`: PASS, failures 0
- Debug x64 Client build/link: PASS
- Release x64 Client build/link: PASS
- `UpdateLib.bat Debug`, `UpdateLib.bat Release`: PASS
- particle PS `fxc /T ps_5_0`: PASS (기존 X4000 경고만 존재)
- scoped `git diff --check`: PASS (기존 LF/CRLF 경고만 존재)

ProjectAudit는 사용자가 이번 작업에서 퇴역·삭제한 상태라 실행하거나 완료 근거로 기록하지 않았다.
Client/UI는 에이전트가 실행하지 않았다.

## 사용자 수동 검증

1. `Client/Default`를 working directory로 Debug Client를 직접 실행한다.
2. F1 `Debug Developer Tools` → `Effect Tool` → `All Effects` → `Artist` →
   `Skill | F | Core F (33)`을 연다.
3. `Play Core F (33)`으로 전체 합성을 확인한다.
4. 같은 패널의 family isolation과 stable occurrence isolation을 사용해 원본
   `R_00.png`, `R_01.png`와 비교한다.
5. 특히 다음을 확인한다.
   - 전기 파랑/네온 초록/흰·회색 사각 carrier가 사라졌는가
   - 원본의 dark navy/black brush와 ink splatter silhouette가 유지되는가
   - `#12/#15/#16/#25/#27/#28/#0` 단독 재생에서 full quad coverage가 없는가
   - 크기와 위치가 원본과 맞는가

사용자의 서면 관찰 전에는 manual first pixel 또는 visual fidelity PASS로 승격하지 않는다.

## 2026-08-14 portable authored particle carrier 보강

사용자의 Mesh 흰색 출력과 Sprite 수평선 출력은 WModel/DDS 누락이 아니라, Track A를 ordinary
authored Effect로 낮추는 과정에서 `SourceRecipe`의 particle 실행 carrier가 제거된 결과로 확정했다.
Mesh는 source color/alpha over-life가 identity white로 바뀌었고, Sprite는 square/velocity/axis
alignment, per-axis size, color/dynamic curve와 SubUV 모드가 generic rectangle 근사로 축약됐다.

이번 보강은 source-contract의 admission/evidence/hash를 authored 문서로 복사하지 않고, 기존 v13
`SourceRecipe`에서 Playback이 실제 실행하는 수치 carrier만 portable 형태로 materialize한다. 현재
Artist F authored JSON에는 Mesh/Sprite 29개 recipe, 350개 module, 564개 distribution이 저장됐다.
ArtistVisualV4 particle color ABI는 opcode별 소비 mask와 일치시키고, 기존 alpha-50/green fallback은
source color curve와 이중 곱되지 않도록 identity로 정규화했다. Required의 SubUV grid/interpolation/
flip은 SourceMaterial 유무와 무관하게 Renderer가 소비하며, SpawnPerUnit과 LocationOnGround도 portable
carrier에서 실행한다. 14개 seeded module은 canonical seed array와 UE3 32-byte seed body를 같은
정식 seed policy로 해석한다.

기존 authored 문서를 기준으로 recipe만 transactionally 교체하므로 사용자가 저장한 Decal DDS,
Transform, Visible과 나머지 Element는 보존된다. materializer는 Client나 UI, ClientFrontendHarness를
실행하지 않는 offline publisher이며 정상 Client runtime은 저장된 authored JSON을 직접 소비한다.

실행한 검증은 다음과 같다.

- portable materializer `--check`: PASS (`29 recipes / 350 modules / 564 distributions / ArtistV4 10`)
- authored JSON canonical SHA-256:
  `cfd83757e76529bfb82cc48d90724fd7eba32db2ed7a9954fb4a293462fdb94d`
- portable seed policy identity: PASS (`14/14`, missing/extra/mismatch 0)
- `Publish-Effects.ps1 -Mode Validate`: PASS (`102` catalog entries, visual programs `13/135`)
- Debug x64 Client build/link: PASS
- scoped `git diff --check`: PASS (기존 LF/CRLF 경고만 존재)

Client/UI와 하네스는 실행하지 않았다. 원본 대비 dark navy/black Mesh polarity, Sprite의 compact square
shape·먹 번짐·연기 밀도와 전체 Play All은 사용자가 새 Debug Client에서 직접 판정해야 한다.

남은 일반화 경계는 P1이다. live weapon-follow는 현재 snapshot transform으로 bake되어 있고, 다른
캐릭터에서 portable capability manifest에 없는 module/literal은 silent approximation하지 않고
fail-closed해야 한다. carrier-bearing Element를 일반 Create/Merge 흐름으로 확대하기 전에는 public
helper의 standalone distribution/burst 검증과 literal capability를 같은 공용 validator로 닫아야 한다.
