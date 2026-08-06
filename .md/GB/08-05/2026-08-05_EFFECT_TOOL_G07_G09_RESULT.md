# Effect Tool G07~G09 구현 결과

## 1. 결론

G07~G09의 기능 중심 Effect Tool 수직 슬라이스를 구현하고 `origin/main` 최신 커밋 `32fab821d4be3da5c5c54fb586522b960b4a2440`에 fast-forward한 작업 트리에서 Debug/Release 정본 회귀를 통과했다.

완료 범위는 다음과 같다.

- Mesh, Sprite, Particle, Decal, Trail 다섯 Element kind 선택과 실제 렌더 경로
- Mesh Model, Base, Noise, Mask, Emissive, Dissolve typed slot과 DDS thumbnail
- Resources/Effect catalog 검색, slot bind/clear, Element 추가·삭제·합성
- Effect Detail의 Transform/Lerp/Color/Distortion/UV/Timing/Particle/Trail/Decal/Pass 편집
- Data Files의 원자적 저장·로드와 All Effects의 publish catalog 재로드·선택 로드
- Animation Tool과 공유하는 Model View의 캐릭터·무기·pivot·animation session
- Animation cue에서 runtime Effect spawn, bone FOLLOW, 월드 Tick/LateTick/Render
- 차원술사 11개 skill ID의 Effect catalog/balance/cue/runtime 연결

후반 렌더링 품질 개선과 Winters WFX의 스킬별 미관을 완전히 복제하는 작업은 이번 완료 범위에서 제외했다.

## 2. 실제 반영 상태

### 2.1 Effect 저작 작업공간

`CEffect_Tool`은 다섯 개의 독립 ImGui 창을 제공한다.

| 창 | 실제 책임 |
|---|---|
| `Effect Tool` | kind 선택, `CreateEffect`, resource slot, thumbnail/grid bind |
| `Effect Detail` | 선택 Element의 수치 편집, Apply/Revert |
| `All Effects` | 현재 Element stack, publish catalog reload/load, preview/reset/delete |
| `Data Files` | Effect document create/save/load/discard |
| `Model View` | 캐릭터·무기·surface/bone pivot·animation·cue 전달 |

저장 가능한 저작 초안은 아직 resource가 없는 Element를 허용한다. GPU preview와 runtime publish는 `Validate_Drawable`을 통과한 완성 Element만 허용하며, stage 실패 시 기존 active document와 preview를 보존한다.

### 2.2 렌더링과 재생

- Mesh는 `CModel`, Sprite는 `CVIBuffer_Rect`, Particle은 `CVIBuffer_ParticleRect`, Trail은 `CVIBuffer_DynamicTrail`, Decal은 depth reconstruction shader를 사용한다.
- fixed-step playback은 30/60/144 FPS 입력에서 같은 결과를 만들고 scrub `Seek`와 순차 재생 결과가 일치한다.
- renderer는 `Document.Elements` 순서로 Element별 AfterImage, 현재 draw, Particle, Trail을 제출한다. pointer hash 순회로 alpha 순서가 바뀌던 경로를 제거했다.
- Decal은 shader에 없는 forward World/View/Projection을 바인딩하지 않는다. material/texture와 inverse view/projection/decal matrix만 바인딩한다.
- runtime FOLLOW anchor는 Character animation update 뒤, render 전에 다시 샘플한다. anchor가 사라지면 이미 render queue에 들어간 객체를 즉시 제거하지 않고 숨긴 뒤 다음 Update에서 안전하게 정리한다.

### 2.3 차원술사 runtime 연결

| 항목 | 수량 |
|---|---:|
| Effect document/catalog entry | 11 |
| 전체 authored Element | 13 |
| Mesh/Sprite/Particle/Decal/Trail | 2 / 2 / 5 / 2 / 2 |
| unique physical Effect resource | 4 |
| animation Effect cue | 14 |

11/11 skill ID의 runtime wiring은 닫혔다. LMB는 4단 combo clip에 cue 4개, 나머지 10개 skill은 cue 1개씩 연결한다.

다만 현재 11개 문서는 공용 ring/glow/noise/trail resource 네 개를 조합한 기능 검증용 저작물이다. Winters WFX와 같은 스킬별 고유 mesh/texture, timing, 색, distortion, decal 모양을 완성했다는 뜻은 아니다.

## 3. `main` 동기화와 병합 정리

- 현재 branch: `codex/effect-tool-reboot`
- 현재 HEAD: `32fab821d4be3da5c5c54fb586522b960b4a2440`
- `origin/main`에 `--ff-only`로 동기화했다.
- stash apply 충돌은 Map Editor PLAN/RESULT, project filter, MapAssetCatalog, PlayerSkillCatalog, balance receipt를 현재 main 계약과 Effect 변경을 함께 보존하도록 해결했다.
- 안전 stash `safety/2026-08-05-pre-origin-main-effect-final`은 복구용으로 유지했다.
- 공유 dirty worktree에 다른 기능 변경이 많아 자동 stage/commit/push는 하지 않았다.

main 병합 후 추가된 LanceMaster stance/movement quick skill 네 개는 의도적으로 damage profile이 없다. Server 계약 테스트의 오래된 “모든 quick skill은 damage가 있다” 가정을 `empty profile -> 0`, `attack profile -> nonzero` 정책으로 교정했다.

## 4. Resources 관리 규칙 정리

팀장은 `Client/Bin/Resources` 물리 폴더를 직접 관리한다. 전역 immutable ZIP/hash/lock/manifest/Snapshot/Publish/Hydrate/Verify 체계와 완료 게이트는 제거했다.

삭제한 항목:

- `Data/AssetPacks.lock.json`
- `Data/AssetManifests/*.manifest.json` 네 개
- `Tools/AssetPipeline/Manage-ResourcePack.ps1`
- `Tools/AssetPipeline/README.md`
- 관련 Client project/filter 등록과 ProjectAudit deep hash/inventory 검사

유지한 경계:

- JSON에는 `Client/Bin/Resources` 상대 asset ID만 저장한다.
- 절대 경로, drive-qualified 경로, `..` 탈출은 거부한다.
- 실제 기능에 필요한 model/texture 존재 여부와 Effect drawable binding은 검사한다.
- `Tools/EffectPipeline/Publish-Effects.ps1`은 리소스 ZIP 배포기가 아니라 Effect 저작 JSON을 runtime catalog로 변환하는 기능이므로 유지한다.

## 5. 자동 검증

| 검증 | 결과 |
|---|---|
| Effect publish `-Mode Validate` | PASS, 11 Effects |
| Effect publish `-Mode Publish` | PASS, 11 Effects |
| Effect pipeline failure/rollback harness | PASS |
| final code bundle/source normalized SHA | PASS, code 49/document 11/resource 4/cue 14 |
| HLSL FXC `fx_5_0` | PASS, Mesh/Rect/Particle/Trail/Decal 5개 |
| gameplay balance provenance sync | 11 `effectId` field만 `PROJECT_TUNED`로 동기화 |
| gameplay balance Validate | PASS, 5 profiles/88 skills/63 damage/1 boss |
| ProjectAudit | PASS, 69 checks |
| canonical Debug build/regression | PASS, `Regression completed: Debug` |
| canonical Release build/regression | PASS, `Regression completed: Release` |
| Server gameplay contract | PASS, failures 0 |
| Debug Client launch smoke | PASS, `Client/Default`에서 10~12초 생존, 즉시 abort 없음 |
| Release Client launch smoke | PASS, `Client/Default`에서 10초 생존, 즉시 abort 없음 |
| `git diff --check` | PASS, line-ending conversion warning만 존재 |

Release 첫 relink에는 third-party DirectXTK/Effects11 PDB `LNK4099` 경고가 있었다. 일부 project post-build가 `pwsh.exe` 부재를 출력하지만 정본 회귀는 Windows PowerShell로 publisher와 harness를 다시 실행해 통과했다. 이 경고 자체를 검증 성공 근거로 사용하지 않는다.

## 6. 수동 F1 검증

자동 검증은 화면의 미관이나 직접 조작 감각을 증명하지 않는다. Debug Client에서 다음을 사람이 확인해야 한다.

1. `Client/Default`에서 Debug `Client.exe`를 실행하고 F1로 Developer Tools를 연다.
2. Effect Tool에서 Mesh/Sprite/Particle/Decal/Trail을 차례로 선택한다.
3. Mesh Model/Base/Noise/Mask/Emissive/Dissolve slot card와 DDS thumbnail을 확인한다.
4. resource grid 선택이 bound slot에 강조되고 `CreateEffect`가 Element를 추가하는지 확인한다.
5. Effect Detail의 Lerp, Color, Distortion, UV sequence/loop/tile, lifetime/delay/afterimage, Particle, Trail, Decal, Pass를 바꿔 Apply한다.
6. Data Files save/load, All Effects reload/load, 여러 Element 합성 후 재로드를 확인한다.
7. Model View에서 class/weapon/animation/pivot을 선택하고 bone/surface FOLLOW를 확인한다.
8. 월드에서 Mesh/Sprite/Particle/Decal/Trail과 차원술사 11개 skill cue의 위치·크기·색·수명을 육안 확인한다.

Release에는 F1 저작 툴이 의도적으로 포함되지 않는다.

## 7. 남은 경계

기능 중심 Effect Tool은 완료했다. 다음은 별도 시각 저작·UX 개선 범위다.

- Winters reference의 DockSpace/DockBuilder 기반 한 창 레이아웃과 정확한 panel 배치
- 폴더명에서 자동 파생하는 resource category combo와 reference 수준의 thumbnail 탐색 UX
- 차원술사 11개 스킬별 고유 resource 발굴·합성·timing·pivot·animation 육안 튜닝
- bloom/distortion/decal/alpha ordering의 장면별 최종 품질 조정
- Debug F1의 실제 다섯 kind × 각 slot 수동 화면 증거

따라서 현재 상태는 `11/11 runtime wiring + 다섯 kind 기능 완료`이며, `Winters WFX 시각 동일성 + 11개 스킬 미관 완료`는 아니다.

## 8. 최종 코드 정본

읽기용 전체 코드 묶음은 [`EffectTool_G07_G09_FinalCode/README.md`](EffectTool_G07_G09_FinalCode/README.md)를 시작점으로 사용한다. 실제 빌드 대상은 같은 상대 경로의 `Engine`, `Client`, `Data`, `Tools` 파일이다.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/ProjectAudit/Test-EffectToolFinal.ps1 `
  -BundleRoot .md/GB/08-05/EffectTool_G07_G09_FinalCode `
  -ResourceRoot Client/Bin/Resources `
  -SourceRoot .
```
