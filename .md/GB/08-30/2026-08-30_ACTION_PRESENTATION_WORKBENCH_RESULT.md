# Action Presentation Workbench 통합 구현 결과

## 1. 결과

`codex/action-presentation-integrated-tool` 브랜치에서 Debug Developer Tools를 실제
`VALTAN_ARENA`의 Server authority 위에 조율하는 `Action Presentation Workbench` 수직 슬라이스를
구현했다.

- 전용 worktree: `C:/Users/user/Desktop/CodexWorkTree/LostArk-action-presentation-tool`
- 기준 commit: `origin/main@68cabd25`
- 원본 물리 폴더: `C:/Users/user/Desktop/LostArk`
- 원본 물리 폴더에는 이 작업을 merge하거나 stage하지 않았다.

원본은 2026-08-30 최종 확인 시 `codex/release-runtime-finalization` 브랜치였고
`origin/main`보다 2 commit 뒤에 있었다. 또한 status 2,269개, 삭제 표시 2,170개, untracked 22개가
동시에 존재했다. 다른 세션의 소유권이 섞인 이 상태에서 자동 merge하면 Effect 정리와 Kakul intake를
손상할 수 있으므로 전용 worktree만 빌드·검증했다.

## 2. 실제로 연결한 Workbench

Workbench는 새 테스트 레벨이나 두 번째 Valtan runtime을 만들지 않는다. 기존 Debug 도구와 실제
제품 경로를 다음처럼 조율한다.

```text
Action Presentation Workbench
  -> Clip Preview: 실제 model clip 단독 확인
  -> Pattern Offline: Product 패턴과 presentation join 확인
  -> Server Replay / Live: 실제 Valtan Arena Server audition request
       -> Server pattern/action/combat object
       -> reliable semantic occurrence event
       -> Client animation/effect/sound/camera presentation
```

중앙 joined timeline에는 다음 lane을 한 stage 기준으로 표시한다.

- Server Stage와 Hit
- Animation clip sequence와 anchor
- Effect point/window
- Sound asset과 combat-object impact sound
- Camera invocation과 Shake occurrence
- World Event trigger
- Combat Object

Camera invocation의 stable invocation ID, cue ID, trigger, 시작 시각과 duration policy를 표시하며
`Open Camera Tool`은 기존 Camera Tool에 one-shot 요청을 보낸다. 두 번째 Camera runtime이나 복제
저장 경로는 만들지 않았다. World Event lane은 trigger reference를 표시하며 UI는 Server replicated
state를 관찰할 뿐 별도 UI command 경로를 저작하지 않는다는 권위 경계도 화면에 명시했다.

사용자에게 노출되는 저장 명령은 `Save` 하나다. Valtan stage duration과 high-jump 도끼 패턴의
공백 시간을 수정해 저장하고, 저장된 stable pattern ID를 `Server Replay / Live`로 다시 제출할 수
있다. `Publish Candidate` 같은 구현 용어는 UI에서 제거했다.

## 3. Arena preset과 실제 패턴 재생

`VALTAN_ARENA_PRESET`은 다음 다섯 상태를 stable enum으로 사용한다.

1. `Fresh`
2. `Circle Walls Gone`
3. `3 o'clock Broken`
4. `9 o'clock Broken`
5. `Both Sides Broken`

예전 phase 증가 방식처럼 이전 상태에 누적하지 않는다. Server가 destruction, collision,
navigation과 boss hold를 preflight한 뒤 reset된 arena에 선택 상태를 원자적으로 적용한다. 실패하면
부분 파괴 상태를 남기지 않는다. Boss Tool, Animation Workbench와 Effect Tool V2는 같은
`CValtanPatternAuditionService`를 통해 선택한 실제 패턴을 재생한다.

## 4. 도끼 바닥 충돌음 누락의 구조적 수정

바닥 충돌음을 animation clip의 추측 시각에 붙이지 않았다. Server의 timed combat-object hit가
실제로 commit된 tick에 `S2C_COMBAT_OBJECT_PRESENTATION_EVENT`를 reliable하게 전송한다.

정확한 join key는 다음과 같다.

```text
combatObjectArchetypeId = combatobject.valtan.high-jump.target-axe
hitId                    = hit.valtan.high-jump.target-axe.01
soundBank                = S_Mob_G_Voltan2
soundEvent               = G_Voltan2_Attack09_ProjExp1
```

Server packet은 asset 경로를 알지 않는다. room-monotonic event sequence, server tick, stable
combat-object/pattern/stage/hit ID, repeat index와 실제 world transform만 보낸다. Client가 sound
binding과 Sound Catalog를 exact join해 Resources-relative WAV variant를 선택한다.

새 저작 문서는
`Data/Animation/Authored/Valtan/Valtan.combatobjectsoundcues.json`이며 exact property, duplicate source
tuple, Product hit 존재와 Sound Catalog event 존재를 검증한다. 로드는
`parse -> validate -> stage -> commit`이고 저장은 임시 파일과 atomic replace를 사용한다.

## 5. 데이터 정본과 물리 리소스 경계

| 영역 | 저작 정본 | Product/Runtime 소비 |
|---|---|---|
| Valtan stage/action/duration | `Data/Valtan/Valtan.presentation.json` | Valtan encounter bootstrap과 Server |
| Valtan animation sequence | `Data/Animation/Authored/Valtan/Valtan.patternbindings.json` | `CValtan` presentation |
| clip sound | `Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json` | action timeline |
| combat-object impact sound | `Data/Animation/Authored/Valtan/Valtan.combatobjectsoundcues.json` | semantic event와 `CValtan` |
| shake | `Data/Animation/Authored/Valtan/Valtan.patternshakecues.json` | joined timeline과 presentation |
| combat object hit | `Data/Valtan/Valtan.combatobjects.json` | `Data/Encounters/Valtan/ValtanCombatObjects.json`과 Server |
| sound event/variant | `Data/Sound/CharacterSoundCatalog.json` | `Client/Bin/Resources/Sound/...` |
| Effect V2 | `Data/Effects/V2`와 현재 V2 binding | Effect Tool V2와 Product Effect runtime |
| physical runtime asset | `Client/Bin/Resources/<root>/...` | Client runtime |

현재 `Data/Animation/Authored`에는 Artist, DimensionMaster, GunSlinger, LanceMaster, Slayer,
Warlord와 Valtan의 실제 저작 폴더가 존재한다. Valtan은 animation, effect, sound, shake,
preview와 combat-object sound 문서를 stable ID로 분리해 소유한다. Effect V1/V2는 서로의 파일을
복사하지 않고 각 domain 정본을 유지하며 Workbench가 pattern/stage ID로 join한다.

Effect V2 validator의 현재 결과는 authored 75개, binding 75개, texture dependency 38개다.
`Client/Bin/Resources`의 물리 최상위 폴더는 `Fonts, Character, Deploy, Effect, Map, Sound, UI`
일곱 개다.

## 6. 빌드와 프로젝트 병목 정리

### 제품 프로젝트

`Framework.sln`에는 12개 프로젝트가 있지만 기본 Build는 다음 네 제품 프로젝트만 선택한다.

1. Engine
2. Shared
3. Server
4. Client

solution의 16개 `Build.0` mapping은 네 configuration 각각에서 위 네 제품을 선택한 결과다.
`Invoke-BuildAndRegression.ps1`은 `Product`, `Core`, `FullDiagnostic` profile을 명시하며 기본은
`Core`다. Product build는 project reference의 암묵적 재빌드를 막고 정해진 제품 순서로 한 번씩만
빌드한다.

### 진단 프로젝트

solution의 opt-in 진단 프로젝트는 NetworkProtocol, CharacterSelectIsolation, Physics, WModel,
PointLight, ActionPresentationTimeline, ValtanPatternAuditionService, MapFrustum 여덟 개다.
물리 폴더의 나머지 22개 vcxproj는 vendored ImGui example이며 solution에 들어 있지 않다.

현재 물리 vcxproj는 총 35개다.

- 제품 4개
- solution 진단 8개
- optional Effect shader probe 1개
- vendored ImGui example 22개

solution 활성 프로젝트 기준 CPP entry는 300개, unique CPP는 270개, 중복 compile entry는 30개,
중복 대상은 26개다. 이전 329/271/58/43보다 줄었고, 남은 중복은 일상 Product build가 아니라
명시적 FullDiagnostic에서만 발생한다.

### 제거 또는 축소한 병목

- Client 프로젝트의 Effect data `<None>` 2,405개와 생성 filter를 제거했다.
- `Tools/EffectPipeline/Sync-EffectDataProject.ps1`을 삭제했다.
- 깨진 registration smoke인 `Test-EffectDataProjectRegistration.ps1`을 삭제했다.
- `ValtanFourPlayerHarness` project/source/wrapper를 삭제했다.
- 8인 raid를 `ValtanFourPlayer`라는 이름과 결합하지 않고 protocol의 8 actor snapshot round-trip으로
  고정했다. 네 Server AI actor의 판단·스킬·생존은 아직 구현하지 않았다.
- 예전 `EffectRenderContractHarness`의 Client CPP 28개 직접 재컴파일과 Imported Artist corpus 결합,
  solution/central runner 등록을 제거했다.

마지막 항목의 물리 폴더 이름은 아직 `EffectRenderContractHarness`지만 예전 광역 하네스는 아니다.
제품 CPP와 ProjectReference가 0개인 단일 C++ optional WARP probe만 남겨 V1/V2 Product CSO의
draw/readback과 runtime asset-root 경계를 확인한다. 이 프로브는 Product/Core/FullDiagnostic에
자동 포함되지 않아 제품 빌드 병목이 아니다.

## 7. 빌드 시간과 자동 검증

### 제품 빌드

| 검증 | 결과 | 시간 |
|---|---:|---:|
| Release Product 실제 clean build | PASS | 467.40초 |
| Debug Product 최종 증분 build | PASS | 32.80초 |
| Release Product 최종 증분 build | PASS | 39.72초 |
| Debug Core | PASS | 약 62초 |
| Release Core | PASS | 약 64초 |

clean Release의 주된 비용은 active HLSL/FxCompile producer 23개와 Client consumer 22개의 최초
CSO 생성이다. 제품 CPP 2,405개를 각각 처리해서 30분이 걸리는 구조는 제거됐다. 최종 Release
Product는 Client link, Product CSO closure와 runtime DLL 배포까지 통과했다. DirectXTK PDB가 없는
기존 `LNK4099` warning은 있었지만 link 실패는 없었다.

### 계약 검증

- Workbench static contract: 7 PASS
- build profile contract: 6 PASS
- Release Client surface contract: 3 PASS
- Animation Tool Valtan master: 12 PASS
- Effect source contract: 33 PASS
- Effect Tool saved rows: 35 PASS, 7 intentional skip
- Effect V2: 5 PASS, CLI closure authored 75 / binding 75 / texture 38
- focused Effect render static contract: 3 PASS
- NetworkProtocol Debug/Release: PASS, 8 raid participant snapshot 포함
- Server gameplay contract Debug/Release: failures 0
- ActionPresentationTimeline native harness: PASS
- ValtanPatternAuditionService native harness: PASS
- Character Select Core, Party2, Party4: PASS
- PointLight, Physics, WModel native harness: PASS
- optional Effect WARP probe Debug/Release: PASS, V1/V2 모두 `litPixels=1352`
- tracked/current JSON 2,260개 parse: PASS
- vcxproj/XML 70개 parse와 project/filter parity: PASS
- Client Effect `<None>` registration: 0

Party2와 Party4를 같은 port에서 동시에 실행한 최초 진단은 포트 충돌이었으며 제품 실패가 아니다.
각각 직렬 재실행해 PASS를 확인했다.

## 8. FullDiagnostic에서 발견한 실제 물리 리소스 blocker

Release FullDiagnostic는 compile과 Valtan crash/CAS/journal 73개 테스트를 279.054초 동안 통과한 뒤
422.73초 시점에 `test_valtan_model_view_composition.py`의 물리 리소스 gate에서 중단됐다. gate는
우회하거나 약화하지 않았다.

원본 `Client/Bin/Resources/Character/Valtan/Ghost` 폴더가 비어 있으며 최소 다음 파일을 복원해야 한다.

```text
MN_RPBF_02.wmodel
MN_RPBF_02_AnimSet.wmodel
mn_rpbf_01_ghost_d.dds
mn_rpbf_01_n.dds
mn_rpbf_01-1_d_loc_int.dds
mn_rpbf_01-1_n_loc_int.dds
mn_rpbf_01-2_d_loc_int.dds
mn_rpbf_01-2_n_loc_int.dds
```

body 복원 뒤 `Tools/ModelAssetConverter/bake_ghost_valtan_animset.py`로 AnimSet을 다시 생성한다.
과거 AnimSet의 확인값은 46,540,308 bytes,
SHA-256 `cb21ca2cb57a439966789bfee4a0ee9d91ee9ce521f7f9c006fd4cc2824a3998`이다.
body와 texture의 근거는
`.md/GB/08-22/2026-08-22_VALTAN_GHOST_MN_RPBF_02_RESOURCE_EXTRACTION_RESULT.md`와
`.md/GB/08-26/2026-08-26_GHOST_VALTAN_CUSTOM_CHAIN_AND_WEAPON_RESULT.md`에 남아 있다.

이 리소스가 없으므로 FullDiagnostic 전체를 PASS라고 기록하지 않는다. gate 뒤에 있는 각 native
diagnostic은 별도 실행해 모두 PASS했지만, 물리 closure가 복원된 뒤 정본 FullDiagnostic를 다시
한 번 끝까지 실행해야 한다.

## 9. 아직 구현 완료로 처리하지 않은 경계

### KakulSaydon

현재 다른 물리 세션의
`Data/ResourceIntake/LV_LUT_MIDNIGHTC_ED.resource-intake.json`은 alias가 KakulSaydon인 추출 intake다.
문서 자체가 `productLevelAdmitted=false`, `sourceComplete=false`, `debugAuthoringReady=false`를
선언한다. Workbench branch에는 Kakul Product world, encounter, Server room/runtime, Client level
descriptor가 없으므로 가짜 Test Kakul level이나 부분 resource copy를 만들지 않았다.

### Save 원자성

현재 하나의 visible `Save`가 Product pattern save와 combat-object sound document save를 순서대로
실행한다. 각 파일은 개별적으로 atomic replace하지만 두 파일을 하나의 filesystem transaction으로
묶지는 못한다. Product save 성공 뒤 Sound replace가 실패하면 partial result를 명시적으로 보고하며
조용히 성공 처리하지 않는다.

### 아직 남은 Workbench 범위

- Sound는 WAV preview를 제공하지만 waveform raster lane은 아직 없다.
- Camera/Shake/World Event는 join·표시·deep-link까지이며 Camera Tool의 모든 저장을 같은 cross-domain
  transaction으로 합치지 않았다.
- 네 명의 Server AI raid actor는 아직 없다. protocol wire의 8 actor 보존만 검증했다.
- 사용자 화면과 실제 음량·음색·Effect fidelity는 자동 PASS로 판정하지 않았다.

## 10. Debug, Release와 ImGui

- Release Lobby는 `Test`, `Character Select`, `Valtan`, `Bern` 네 제품 버튼만 표시한다.
- Release Lobby에는 Debug ImGui와 F1 Developer Tools가 없다.
- Debug Lobby도 같은 네 제품 버튼을 사용하며 F1 Debug Developer Tools가 추가된다.
- Workbench, Boss Tool, Effect Tool V2와 Camera Tool 연결은 Debug 전용이다.

`test_release_client_surface_contract.py` 3개 계약으로 위 분기를 확인했다. Client를 에이전트가 직접
실행하거나 화면을 대신 판정하지는 않았다.

## 11. Git ignore와 Intermediate

공유 `.gitignore`는 `/Client/Bin/Resources/` 전체를 제외한다. 따라서 `.gitattributes`의 DDS/WAV
LFS 규칙이 있더라도 exact dependency closure를 force-add하지 않는 한 물리 리소스는 Git 대상이
아니다. `git check-ignore -v`로 Effect DDS와 Sound WAV가 이 규칙에 걸리는 것을 확인했다.

`**/[Ii]ntermediate/`도 이제 로컬 exclude가 아니라 공유 `.gitignore`에 있다. Intermediate는
configuration별 compiler object, PDB, incremental link, tool scratch 같은 재생성 산출물이다.
빌드가 실행 중이지 않을 때 삭제해도 source나 authoring 정본은 사라지지 않으며 다음 build가 다시
생성한다.

## 12. 사용자 수동 검증 순서

1. LAN endpoint `192.168.0.14:7777`의 Server를 시작한다.
2. `Client/Default` working directory로 Debug Client를 실행한다.
3. Lobby의 `Valtan`으로 Server 승인 입장한다.
4. F1에서 `Action Presentation Workbench`를 연다.
5. high-jump/target-axe pattern을 선택하고 blank/stage duration을 늘린 뒤 `Save`한다.
6. Sound와 Combat Object lane에서 exact hit binding과 WAV variant를 확인한다.
7. `Server Replay / Live`를 실행해 실제 도끼 impact occurrence에서 바닥음을 듣는다.
8. 다섯 arena preset을 각각 선택해 wall, collision과 navigation 상태를 육안 확인한다.
9. Effect Tool V2에서 같은 pattern의 `Play Server / Arena`를 실행한다.
10. Camera lane의 `Open Camera Tool`로 실제 sequence를 확인한다.
11. Release Client에서 Lobby 네 버튼만 존재하고 ImGui가 나오지 않는지 확인한다.

이 수동 확인 전에는 시각·청각 fidelity를 완료 처리하지 않는다.
