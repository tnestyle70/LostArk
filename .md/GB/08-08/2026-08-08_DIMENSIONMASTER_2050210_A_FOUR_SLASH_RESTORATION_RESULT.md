# 2026-08-08 차원술사 A 2050210 원본 복구 결과

## 1. 결론

차원술사 A의 정본을 수동 16-layer candidate가 아니라 원본 추출 계약 기반 117 Elements로
되돌렸다. 수동 `body/rim/highlight/afterimage` 문서는 삭제하지 않았지만 시각 참고용이며 제품
정본이 아니다.

이번 결과는 원본 Geometry, Cascade module, event time, anchor, named Material input을 보존하고
실행하는 단계까지 완료했다. cooked package에서 소실된 Material graph 연산은 근사 profile이므로
원작 픽셀 완료 또는 `RUNTIME_EXACT`로 승격하지 않았다.

## 2. 반영된 원본 계약

| 항목 | 결과 |
|---|---:|
| canonical Elements | 117 |
| Particle container | 100 |
| Mesh-backed Particle | 48 |
| Sprite | 52 |
| Light | 4 |
| Screen Post | 13 |
| LinearFlow occurrence | 12 |
| LinearFlow required named inputs | 12 occurrences 모두 7/7 |
| Material `RUNTIME_EXACT` | 0 |

원본 네 타격의 source event는 다음과 같다.

| Hit | Source time | Root-relative position |
|---:|---:|---|
| 1 | `0.25 s` | `(0.5, -0.5, -0.9)` |
| 2 | `0.60 s` | `(0.5, -0.5, 0.8)` |
| 3 | `0.90 s` | `(0.5, 0.3, -0.9)` |
| 4 | `1.30 s` | `(0.5, 0.6, -0.8)` |

각 타격의 핵심 LinearFlow Mesh는 `fm_m_trail_002`, `fm_h_swing_02`, `fm_m_trail_01`이고,
각 occurrence가 원본 부모의 named texture 입력을 직접 보존한다.

## 3. Material 복구

원본 package `ZHJ4TC4PCK4PR4J22HIXEYUXBU.upk`에서 부모 Material 증거를 직접 추출했다.
`fx_m.fx_j_pa_linearflow_02_tr`의 실측은 다음과 같다.

```text
Expression entries       344
Non-null                 110
Null                     234
Named textures             7
Unresolved input edges     45
Topology status          COOKED_PARTIAL
Runtime exact eligible   false
```

문서와 런타임은 `diff_tex`, `diff_noise_tex`, `a_mask_tex`, `a_noise_01_tex`, `b_mask_tex`,
`b_noise_01_tex`, `dissolve_tex`를 이름 기준으로 stage한다. 누락되거나 잘못된 리소스는 숨김과
명시적 오류로 처리하며 white fallback을 사용하지 않는다.

Publisher가 이 texture들을 manifest dependency로 게시하는 것과 C++ runtime이 compiled
Assembly에서 수집하는 dependency 집합도 동일하게 맞췄다. 이 불일치가 있으면 런타임이 최신
Catalog 전체를 거부하던 버그도 함께 해결했다.

## 4. Animation과 Effect 시간축

A animation clip `pc_sp_m_00_sk_sk_willowrend`의 authored `playRate`를 Effect playback rate에도
전달한다. 따라서 Animation 담당자가 일정 배속을 바꾸더라도 Effect 문서의 source event 시간을
다시 수동 환산하지 않는다.

```text
effect source time = animation source time
wall-clock time    = source time / playRate
```

일반적인 A layer는 각 event 프레임의 Character root를 snapshot한 뒤 재생한다. 원본에서
`follow=true`인 layer만 현재 anchor를 계속 추적한다.

## 5. 생성물과 검증

- canonical A SHA-256:
  `D8D5C8AB6160DE820A56019D6BC36C7DC95037391EED3821F4635B1C7B06CC76`
- Assembly/WFX: 16 Effects / 182 Components / 947 Emitters
- Runtime Catalog publish: PASS
- Python: 34 tests PASS
- Effect pipeline: PASS
- Client x64 Debug build: 오류 0
- ClientFrontendHarness: `failures 0`
- Effect Tool final audit: PASS
- JSON parse와 `git diff --check`: PASS
- ProjectAudit: FAIL 1
  - `projects.data-source-visibility: expected=550 project=547 filters=547`
  - 공유 작업트리의 DataFiles 프로젝트 등록 불일치이며 A 문서·Assembly·Runtime stage는 통과했다.

## 6. 수동 GPU 검증

수동 GPU A/B는 아직 PASS 처리하지 않았다. 다음 실행에서 Effect Tool의 수동 candidate가 아니라
canonical `effect.dimensionmaster.skill.2050210`을 선택한다.

1. DimensionMaster의 A animation을 재생한다.
2. Screen Post를 끄고 `0.25 / 0.60 / 0.90 / 1.30초`를 차례로 확인한다.
3. 첫 타격에서 emitter 14, 15, 20을 Solo한다.
   - emitter 14: `fm_m_trail_002`
   - emitter 15: `fm_h_swing_02`
   - emitter 20: `fm_m_trail_01`
4. opaque card가 없는지, Mesh 방향과 root snapshot이 맞는지, Mask/Dissolve/Flow가 동작하는지
   확인한다.
5. Geometry와 Material을 확인한 뒤 Screen Post를 켜고 네 타격의 누적과 마지막 잔상을 본다.

캡처에는 Active Effect ID, sample time, selected emitter ID, Screen Post ON/OFF, camera/FOV를
함께 남긴다.

## 7. 남은 경계

- `RUNTIME_EXACT=0`이다. 45개 unresolved graph edge가 복구되거나 GPU A/B로 finite 식이 검증되기
  전에는 exact라고 부르지 않는다.
- 현재 LinearFlow HLSL은 원본 7개 texture와 MI scalar/vector를 소비하는 source-driven
  reconstruction이다. 원본 parent graph의 모든 Multiply/Lerp/Panner 연결과 동일하다는 증거는 없다.
- A 수동 candidate는 비교 자료일 뿐 canonical publish에 사용하지 않는다.
- 향후 base11 자동 promotion은 canonical A SHA가 기존 receipt와 다르면 의도된 수동 override로
  거부한다. A를 덮기 전에 이번 canonical을 명시적으로 보존해야 한다.

## 8. main 병합 후 수동 후보와 검증 성능 복구

main 병합에서 canonical과 혼동하지 않도록 제외했던 별도 수동 후보를 다시 Git 관리 대상으로
복구했다. 이 후보는 canonical을 덮지 않으며, 원본 실행 복구 전후를 비교하는 검증 기준이다.

```text
effect.dimensionmaster.skill.2050210.a-restoration-candidate
16 Mesh Elements
manual.a.hit01~04 × body/rim/highlight/afterimage
```

`Data Files`에서 이 문서를 로드하면 `Play Group`과 Element별 `Solo`가 다시 표시된다. 후보는
deterministic seed와 동일하며 7개 Python 회귀 테스트로 확인했다.

또한 문서 로드가 117-layer canonical 전체를 즉시 재생해 프레임을 떨어뜨리던 동작을 제거했다.
이제 Load는 문서와 GPU 리소스를 stage한 뒤 Preview를 숨기고 정지한다. 다음 명령 중 하나를
눌렀을 때만 Character root pivot과 연결된 A animation source time으로 재생한다.

```text
Play Complete Effect
Play Group
Solo
```

자동 검증 결과:

- A candidate Python: 7 tests PASS
- Effect Tool final audit: PASS
- Effect data project registration: 435 files / 47 filters PASS
- Client x64 Debug build: 오류 0

수동 런타임 FPS와 네 타격의 실제 Animation/Effect 정합은 사용자 GPU 검증 전이므로 PASS로
기록하지 않는다.

## 9. canonical A 6 FPS 검증 병목 교정

정본 A를 로드한 뒤 명시적 재생 전에도 프레임이 떨어지는 원인을 코드로 재현했다.

- Load가 EffectObject를 숨겨도 다음 Tool Update가 정상 Root를 확인하며 다시 표시했다.
- 정지 상태에서도 `Set_RootWorld`가 117 Elements의 evaluated frame을 매 프레임 재구축했다.
- 재생 중에는 Root 갱신과 시간 진행이 각각 frame rebuild를 호출해 같은 프레임을 두 번 평가했다.
- Renderer는 authored Element 117개마다 전체 Particle/Trail/AfterImage 목록을 다시 순회했다.

교정 결과:

- Load와 Hide는 명시적 preview visibility 상태를 유지하며 Play/Solo 전에는 평가·제출하지 않는다.
- Tool preview는 Root와 sequential time을 한 번의 `Advance_Preview` 호출로 평가한다.
  제품 Follow anchor의 같은 프레임 계약은 변경하지 않았다.
- Renderer는 기존 authored stack 순서를 유지하면서 evaluated 목록의 contiguous range를 한 번만
  순회한다.
- `Mesh Emitters`와 `Sprite Emitters` preview scope를 추가했다. Data Files의
  `Play Mesh Emitters`는 48개 mesh-backed layer만 재생하며 Screen Post와 Sprite는 제외한다.

자동 검증:

- Client x64 Debug build: 오류 0
- ClientFrontendHarness x64 Debug build/run: `failures 0`
- Effect Tool final audit: PASS
- ProjectAudit: 77 checks PASS
- `git diff --check`: PASS

수동 GPU FPS는 아직 측정 전이다. 따라서 6 FPS 해결 완료가 아니라, 원인이었던 숨김 해제·이중 평가·
제곱 순회를 제거하고 검증 가능한 Mesh 전용 범위를 제공한 상태다.

## 10. canonical A Load의 동기 GPU stage 제거

추가 재현에서 숨김과 자동 재생을 막아도 `Load Document` 자체가 117 Elements의 GPU resource를
동기 stage한다는 병목이 남아 있음을 확인했다. canonical A에는 701개 resource reference가 있고,
stable asset ID 기준 고유 resource는 74개다. 기존 Renderer는 Element마다 같은 WModel과 DDS도
다시 열기 때문에 Preview를 숨기기 전에 Load 호출이 장시간 막혔다.

`Try_LoadDocumentPathStaged`의 Load 계약을 다음처럼 교정했다.

```text
Load Document
  parse -> validate -> Active Document commit
  이전 Preview Object와 GPU resource release
  새 GPU stage 없음

Play Complete / Mesh / Sprite / Group / Solo
  Build_PreviewDocument로 선택 범위 생성
  해당 범위만 GPU stage
  Character root와 Animation source time 연결 후 재생
```

정적 계약 검증에서 Load 함수 안의 `Stage_WorldPreview` 호출이 0건이고,
`Release_WorldPreview(true)`와 deferred GPU 상태 문구가 존재함을 확인했다. Client x64 Debug 전체
컴파일과 정본 `Client.exe` 링크는 PASS했다. 하네스는 사용자 요청에 따라 실행하지 않았다.

전체 Effect Tool audit는 같은 파일의 초기 인덱스 구간을 수정 중인 병렬 세션의 미완료 계약 때문에
`Valid partial drafts must reload...`에서 중단됐다. A Load 독립 검사는 통과했으며 이 실패를 A Load
회귀로 기록하지 않는다. 실제 Load 응답 시간, emitter Solo FPS, Mesh 48-layer FPS는 최신 Client
수동 GPU 검증 전이므로 아직 PASS로 기록하지 않는다.

## 11. A 1검격 Solo 검증으로 확정된 복구

0.25초 전후의 원본 emitter를 개별 Solo한 결과로 다음 결함과 정상 레이어를 구분했다.

- `particlespriteemitter_14`, `particlespriteemitter_20`: 빈 UE3 Alpha distribution이 0으로 평가되어
  보이지 않았다. 빈 `StartAlpha`, `AlphaOverLife`, `AlphaScaleOverLife`는 곱셈 항등값 1로 평가하되
  명시된 0 또는 0.5 같은 값은 그대로 보존하도록 playback을 수정했다.
- `particlespriteemitter_3`: blackline의 Dynamic Parameter를 clamp texture UV에 절대 좌표처럼 더해
  mask A/B가 범위를 벗어났다. 이름 기반 `mask_a_pan`, `flow_strength`, `mask_b_pan`,
  `diffuse_pan`을 각 pan 강도로 소비하는 유한식으로 교정했다. 원본 그래프가 없으므로
  `RUNTIME_EXACT=0`은 유지한다.
- `dust_00_1.particlespriteemitter_42`: aura DDS의 불투명 alpha와 잘못 상속된 SubUV atlas가
  흰 판을 만들었다. RGB luminance와 radial feather를 mask로 사용하고 이 profile의 SubUV를
  `none`으로 고정했다.
- `swinghit_00_1.particlespriteemitter_30`: 불투명 Voronoi carrier를 generic translucent로 그려
  세로 흰 판이 됐다. `effect.ue3.slice.v1` 전용 유한 profile로 blade envelope와 flow mask를
  계산하도록 분리했다.
- `swinghit_00_1.particlespriteemitter_25`: `fx_i_atypical_02`를 쓰는 additive impact glint다.
  Alpha 3에서 0으로 감쇠하는 정상 후반 emissive/highlight 레이어이며 distortion 채널은 없다.
  원본 A 후반 프레임의 흰색 bloom과 대응하므로 수정하거나 제거하지 않는다.

자동 검증 결과는 material contract 관련 Python 43건 PASS, ClientFrontendHarness `failures 0`,
Effect Tool final audit PASS, HLSL `ps_5_0` 컴파일 PASS, Client x64 Debug/Release 빌드 PASS다.
최종 GPU 판정은 새 Debug 실행에서 Screen Post OFF로 위 emitter들을 Solo한 뒤 그룹과 Complete를
확인하고, 마지막에만 Screen Post를 ON 하는 수동 검증으로 남긴다.

Authored 2050210 변경 뒤 기존 WFX compile identity가 불일치한 상태도 확인했다. 검증 시에는
Assembly 행을 선택하지 않지만 실제 인게임 소비를 위해 `build_effect_components.py`로 Authored를
다시 compile했다. 결과는 effect 16개, component 182개, emitter 947개이며 compile identity가
완전하게 일치한다. 새 texture sampling evidence JSON도 Client 프로젝트의 `96.DataFiles` None 항목에
등록했다.

---

## 12. 2026-08-09 Track B 4타 실제 A audition 수직 슬라이스

### 12.1 동기화와 실제 입력 경로

작업 브랜치와 `origin/main`이 모두 MapTool PR #72 merge commit
`5c98aa62117b582d181878dc7134b99f6e0e4919`를 가리키는 상태에서 이어서 구현했다. 과거 physics
worktree EXE는 사용하지 않았다.

실제 A 입력은 socketless Character Select Preview가 아니다. DimensionMaster 선택 뒤
`Server Play (Lobby-approved)`로 Server Arena에 진입하고 F6 follow 상태에서 A를 누른다. F6 free
camera에서는 gameplay command가 차단된다.

2050210 이동은 새로 구현하지 않았다. 기존 Server bootstrap의 84-sample root-motion, 최종 전진
약 2.3053m를 Server가 aim 방향으로 적용하며 Client는 snapshot을 소비한다. 따라서 Track B local
position에는 누적 대시 이동량을 중복 bake하지 않았다.

### 12.2 최종 Authored Track B 데이터

일회성 correction을 version 2로 올리고 다음 6 layer를 네 hit에 각각 materialize했다.

```text
white-echo _2
flow       _14
body       _15
afterimage _20
rim        _3
sprite     _9
```

결과는 정확히 Mesh 20 / Sprite 4 / Particle 0, 총 24 Element다. 네 hit의 공통 snapshot 시점과
초기 root-relative seed는 다음과 같다.

| hit | delay | position |
|---|---:|---|
| hit01 | 0.25s | `[-0.55, 0.15, 0.95]` |
| hit02 | 0.60s | `[-0.60, 0.15, 1.05]` |
| hit03 | 0.90s | `[-0.65, 0.20, 1.15]` |
| hit04 | 1.30s | `[-0.70, 0.25, 1.25]` |

각 hit의 여섯 Element는 `root`, `follow=false`로 같은 시점의 이동된 Player root/facing을 캡처한다.
로컬 X 음수는 Player 왼쪽, Z 양수는 Player 앞이다. 모든 `sourceRecipe`와 `sourcePresentation`은
비활성이고 Imported/canonical SourceRecipe는 바뀌지 않았다. 24개 모두 선택한 원본 emitter의
`sourceMaterialPath`, `sourceProfile`, texture slot을 보존하며 명시적 emissive slot을 추가했다.
네 Sprite만 billboard roll `-90°`이고 전역 Sprite나 Cascade Mesh Particle에는 적용하지 않았다.

materializer는 기존 Authored 결과를 덮어쓰지 않으며, seed 생성 뒤 수동 정본은 다음 한 파일이다.

```text
Data/Effects/Authored/
  effect.dimensionmaster.skill.2050210.authored-baseline.effect.json
```

### 12.3 탑뷰, 캐릭터 백색 emissive, 반복 audition

Server Arena follow camera를 높이 7.5m, 뒤 거리 4.5m의 높은 대각 탑뷰로 조정했다. Preview의 근접
카메라는 유지한다.

DimensionMaster `CharacterSpec`에 2050210의 white full-surface emissive 한 행을 추가했다. Server
snapshot의 SKILL action 동안 Body, skinned equipment, 네 socket weapon이 같은 transient override를
사용한다. animated/static deferred shader는 기존 emissive map을 보존하면서 diffuse detail/alpha를
마스크로 HDR emissive를 더한다. NONE, DEAD, 다른 action/skill snapshot에서 즉시 0으로 초기화되며
로컬과 원격 캐릭터가 같은 경로를 사용한다. shared material은 변경하지 않았다.

Effect Tool에는 exact clean Authored Track B에서만 활성화되는 Debug 버튼을 추가했다.

```text
Save Authored
  -> Publish + Reload A Test
  -> existing component builder
  -> existing publisher
  -> transactional CEffectCatalog::Load
  -> F1 close / F6 follow / A
```

실패하면 기존 Runtime Catalog를 유지하며 Authored 문서를 EffectObject에 직접 넣는 두 번째 런타임
경로는 없다. 다음 A spawn부터 새 revision을 사용한다.

정식 2050210 cooldown 55초와 resource cost 839가 반복 검증을 막으므로 Debug
`CHARACTER_SELECT_ARENA`에서만 Server가 요청 skill의 cooldown entry와 resource를 초기화한다.
Release와 다른 world의 balance는 그대로다. action-running, sequence, class, aim, root-motion,
snapshot은 계속 기존 Server `Try_Start` 경로가 승인한다.

### 12.4 자동 검증 결과

- authored materializer: 13/13 PASS
- component/base builder unit test: 14/14 PASS
- generated authored document: 24 Element, Mesh 20 / Sprite 4 / Particle 0,
  sourceProfile 24, recipe/presentation disabled 24 PASS
- component build: Effect 17 / Component 186 / Emitter 971,
  `compileIdentityComplete=true`
- `Publish-Effects.ps1 -Mode Validate`: 17 Effects PASS
- `Publish-Effects.ps1 -Mode Publish`: 17 Effects / 186 Components PASS
- Effect Tool final audit: PASS
- Render Quality Workbench audit: PASS
- Client x64 Debug + HLSL compile/link: PASS
- Server x64 Debug build: PASS
- `Server.exe --contract-test`: `failures : 0`
- ProjectAudit: 83 checks PASS
- scoped `git diff --check`: 최종 실행 전 재확인 대상

### 12.5 아직 수동 판정이 필요한 항목

자동 검증과 빌드는 완료했지만 원작 PNG와의 GPU 시각 일치는 아직 PASS로 기록하지 않는다. 최신
Server와 Client를 실행한 뒤 다음 순서로 확인해야 한다.

1. Lobby → Character Select → DimensionMaster → Server Play.
2. Server Arena active와 F6 follow 상태를 확인하고 A를 누른다.
3. 전진 중 0.25/0.60/0.90/1.30초의 네 원호가 Player 왼쪽에서 앞 방향으로 생성되는지 확인한다.
4. 몸·장비·무기가 하얗게 발광하고 action 종료 뒤 원복되는지 확인한다.
5. F1 Effect Tool에서 authored-baseline을 열어 한 layer의 position 또는 UV tiling을 바꾼다.
6. Save Authored → Publish + Reload A Test → F1 닫기 → A로 다음 spawn 반영을 확인한다.
7. Shape/anchor 합격 뒤에만 emissive/bloom/material을 원작 PNG 기준으로 미세 조정한다.

검증용 최신 프로세스도 실행했다. Debug Server는 `0.0.0.0:7777` listener를 확인했고, Debug Client는
`Client/Default` working directory와 process-local `LOSTARK_SERVER_HOST=127.0.0.1`로 실행했다.
프로세스 실행은 수동 GPU 합격을 의미하지 않으며 위 일곱 단계의 화면 판정이 남아 있다.

최초 실행은 Windows loader가 `PhysX_64.dll` 누락으로 거부했다. 원인은 정식 `UpdateLib.bat`에는
PhysX runtime 3종 복사가 있었지만 직접 `Client.vcxproj` 빌드의 `AfterTargets=Build` 배포 목록에는
Assimp/FMOD/Engine만 있고 PhysX가 빠져 있었기 때문이다. Client project에 configuration별
`PhysXRuntimeRoot`, 세 DLL 존재 검사, 다음 runtime 복사를 추가했다.

```text
PhysX_64.dll
PhysXCommon_64.dll
PhysXFoundation_64.dll
```

Client x64 Debug 재빌드가 세 파일을 `Client/Bin/Debug`에 배포했고 source/destination SHA-256 일치를
확인했다. 재실행한 Client는 loader dialog 없이 2.5초 이상 정상 생존했다. ProjectAudit에도 직접
Client 빌드와 UpdateLib 양쪽의 3-DLL 배포 계약을 추가했다.
