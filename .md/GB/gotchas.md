# LostArk merge 회귀 방지 정본

## 0. 모든 세션의 사용자 전용 화면 검증 경계

- 세션 시작 시 `AGENTS.md`, `CLAUDE.md`, 이 문서, 있으면 `gotchas.local.md`,
  `.md/TEAM/README.md`, 대응 PLAN/RESULT를 먼저 읽는다.
- Artist F, Effect Tool, Character Select와 Client 시각 결과는 사용자만 직접 조작하고 최종 visual fidelity를 판정한다.
- 에이전트는 Client나 UI를 자율적으로 실행·조작하지 않고 화면 캡처·스크린샷 생성을 하지 않으며,
  visual fidelity를 대신 판정하지 않는다.
- 사용자가 대화에 첨부한 스크린샷이나 이미지 분석을 요청하면 에이전트는 반드시 열람·분석해
  관찰된 결함과 가능한 occurrence 진단을 보고한다.
- 에이전트는 빌드와 구조화된 로그·수치 진단, 실행 준비까지만 수행하고 사용자가 직접 누를 경로를
  보고한 뒤 멈춘다. 사용자의 서면 판정 전에는 first pixel, eye smoke, visual PASS를 기록하지 않는다.
- 일반적인 완성·복원 요청과 기존 캡처 파일의 존재는 Client/UI 자율 실행·조작이나 화면 캡처를 허가하지 않는다.

이 문서는 merge, pull, rebase와 충돌 해결에서 이미 닫힌 다른 작성자의 계약을 되살리거나
지우는 회귀를 막는 공용 체크리스트다. 날짜별 실패 로그는 대응 RESULT에 기록한다.

## 1. 동기화 전 상태 고정

다음 증거를 먼저 남긴다.

```text
git status --short
git diff --name-only --diff-filter=U
git branch --show-current
git rev-parse HEAD
git fetch --prune
git rev-list --left-right --count HEAD...origin/main
```

- dirty worktree를 정리한다는 이유로 다른 작성자의 변경을 reset, checkout, clean하지 않는다.
- 동기화가 필요하면 추적·미추적 파일을 포함한 이름 있는 safety stash를 만들고, 원격 반영 후
  같은 stash를 복원한다. stash 적용 충돌도 아래 파일 역할 기준으로 다시 해결한다.
- `ours` 또는 `theirs`를 파일 전체에 일괄 적용하지 않는다. 실제 소유자, 호출자, 데이터 정본,
  실패 소비자와 대응 PLAN/RESULT를 읽고 계약별로 합친다.

## 2. 충돌 해결 불변식

### DimensionMaster 이름과 런타임 계약

- 활성 코드·공유 enum·catalog·Loader·HUD·Server profile·spawn·Animation Tool의 class 이름은
  `DimensionMaster`/`DIMENSIONMASTER` 계약을 유지한다.
- 이전 `Dimensionist` 이름은 명시적으로 보존한 역사 문서나 외부 원본 식별자가 아니면 되살리지 않는다.
- 이름 불일치를 임시 fallback으로 숨기지 않는다. 정의와 실제 소비 경로를 같은 변경에서 맞춘다.
- DimensionMaster의 Server skill 계약은 `Q W E R A S D F T V ALT_V`와 LMB `2050010` automatic
  3-stage(`1500/1067/1700ms`)로 닫혔고 `ALT_V`는 `2050540`이다. merge에서 LMB 4단 수동
  window, 이전 candidate-only `2050550` 또는 Z 슬롯을 되살리지 않는다.
  candidate-only Effect는 별도의 admitted effect 계약 없이 제품 런타임에 활성화하지 않는다.

### 공용 Character Preview

- Model Preview, Animation Tool, Effect Tool이 공유하는 Character Preview Panel 계약을 유지한다.
- 충돌 해결 과정에서 툴별 두 번째 character loader, pivot owner, preview runtime을 만들지 않는다.
- project와 filters 등록, 실제 include/caller, 모델·무기 part 경로를 함께 확인한다.

### Effect Tool 재작성 경계

- Effect Tool reboot의 정본은 G0/G1에서 승인한 `Effect_AuthoringDocument`와 최소 ImGui document
  경계다. 현재 G 계획이 삭제한 레거시 `Effect_AssetIO`, `Effect_Runtime`,
  `Effect_ParticleSimulator`, `Effect_ResourceCatalog`, `Effect_Types`, 전용 Effect shader와
  생성된 `.effect` 후보 파일을 merge가 다시 살리지 않게 한다.
- 추출 원본과 증거 자료는 저작 데이터와 구분한다. Source Catalog/Extracted/참고 PNG처럼 계획이
  보존하기로 한 원본은 레거시 런타임 삭제와 함께 지우지 않는다.
- G1 Active Document는 메모리 저작 단위이며, Element 종류 radio 선택만으로 Document를 변경하지 않는다.
  G2의 Add Element 이후에만 Element가 Document에 들어간다.
- Effect asset ID와 resource ID는 `Client/Bin/Resources` 기준 상대 안정 ID다. 절대 경로,
  drive-qualified 경로, `..` 탈출 경로를 저장 계약으로 되살리지 않는다.
- 제품 Effect는 `Data/Effects/EffectCatalog.json`과 `Data/Effects/Authored/*.effect.json`만 직접 읽는다.
  `Client/Bin/DataFiles/Effect`, hash seal, VisualPrograms sidecar, Effect publisher를 merge나 복구 과정에서
  다시 만들지 않는다. Editor Save는 파일 저장과 다음-spawn Product activation을 한 transaction으로 처리하고,
  activation 실패 시 compare-and-swap으로 이전 파일과 prepared target을 모두 보존한다.

### Bone/socket scale은 transform 계층별로 검증

- model prototype admission scale을 `CModel::Get_BoneMatrix()`의 combined socket scale로 간주하지 않는다.
  Artist는 admission `0.0001`과 rig `sdm` root `100`이 합성되어 `b_wp_1` combined basis가 `0.01`이다.
- Artist Effect anchor는 combined basis `0.01`을 exact tolerance로 검사한 뒤 3x3에만 `x100`을 적용한다.
  translation, animation rotation, asset orientation과 owner world는 보존한다.
- 관찰한 임의 scale 자동 정규화, `0.0001 또는 0.01` 동시 허용, raw fallback은 금지한다. 다른 class/asset에
  확대할 때는 `prototypeAdmissionScale`, `rigRootScale`, `combinedAnchorScale`, reciprocal을 manifest가 소유한다.
- 회귀는 synthetic matrix만으로 닫지 않는다. 실제 model을 제품 pretransform으로 로드해 bind pose와 대상
  animation pose의 named bone combined scale, 정규화 결과, 잘못된 scale fail-close를 Debug·Release에서 검사한다.

### ObjectManager layer-map AV는 Effect 오류와 분리

- `std::map<..., shared_ptr<CLayer>>::_Find_lower_bound` 내부 AV만으로 missing layer tag, map insertion,
  Effect clone 또는 shader를 원인으로 확정하지 않는다. key 비교 전 map tree-state 주소에서 fault면 manager/map
  lifetime, ABI, out-of-bounds 또는 선행 메모리 손상을 full dump로 구분한다.
- authored animation Effect cue와 balance `effectId` fallback은 별도 경로다. `effectId=""`만 보고 cue가 없다고
  결론내리지 말고 실제 `.animevents`의 published `effectref=asset` row와 runtime caller를 함께 확인한다.
- 위험 경로를 queue로 옮긴 뒤 같은 RVA가 재현되면 그 변경을 root fix로 기록하지 않는다. Client 전용 full
  LocalDump의 전체 stack과 matching EXE/DLL/PDB identity가 확보되기 전에는 `OPEN`을 유지한다.

### Effect 준비 성공과 첫 GPU draw 성공은 별도 gate

- Catalog parse, typed Program 검사, DDS/SRV/sampler 준비와 prepared-cache commit이 PASS해도 실제
  `Bind -> Begin -> Draw`는 아직 한 번도 실행하지 않았을 수 있다. Effect 종료 회귀는 제품과 같은
  `CEffectObject`를 layer에 넣고 실제 fixed-step occurrence가 활성화된 시점까지 진행한 뒤 첫 draw의
  `attempted/submitted/suppressed/failed/committed`를 검사한다.
- native-v14 Artist 문서를 format-13 runtime drawable로 낮출 때 `Renderer.eType`과
  `Renderer.eSourceSpace`를 지우지 않는다. 35개 element는 stable element/emitter ID로 Program의
  renderer/source-space와 다시 exact join하고, aggregate family count만 맞는 것은 증거로 쓰지 않는다.
- 반대로 기존 v3~v13 문서에는 `SourceRecipe.bEnabled=true`이면서 `Renderer==END`인 정상 저작 문서가
  존재한다. 이 경로는 kind와 geometry binding으로 legacy family를 결정한다. typed Artist 규칙을
  legacy에 역적용하거나 `SourceRecipe.bEnabled`를 이유로 GPU occurrence를 제거하지 않는다. 2026-08-12
  기준 회귀 분모는 authored 18문서, particle/decal GPU occurrence 1,300개다.
- Effect 하나의 frozen input, packet denominator, profile, local resource identity 위반은 명시적인
  `LOCAL_CONTRACT`로 기록하고 해당 object만 격리한다. D3D Map/draw, device removal, OOM, global render
  target과 presentation capacity 실패는 `GLOBAL_RUNTIME`으로 전파한다. `E_FAIL` 값만 보고 둘을 추론하지 않는다.
- 첫 draw 회귀는 최소 Artist Full35와 legacy Lance BA1을 함께 태운다. Artist만 검사하면 legacy
  renderer fallback 퇴행을, Lance만 검사하면 35행 typed renderer join 퇴행을 놓친다. 화면 모양은 이
  자동 gate가 녹색인 뒤 사용자가 별도로 판정한다.

### Artist F 수동 검증 경계

- 화면의 최종 판정자는 사용자다. 에이전트는 Client HWND나 Effect Tool을 자율적으로 실행·조작하거나
  직접 캡처하지 않는다.
- 사용자가 첨부한 실행 화면이나 이미지를 분석해 달라고 요청하면 반드시 열람·분석한다. 분석 결과는
  occurrence별 진단·리뷰 입력으로 사용하되 최종 visual PASS나 단독 admission 증거로 승격하지 않는다.
- 자동 증거는 compile, structured diagnostic, resource/shader/draw 수치에 한정한다. 에이전트가 직접 만든
  캡처나 자동 클릭 결과를 구현·리뷰·완료 증거로 사용하지 않는다.
- 에이전트는 Server CMD와 Client 준비 상태, 정확한 수동 클릭 경로만 전달한다. 사용자의 관찰 결과를
  받은 뒤에만 occurrence별 결함과 튜닝 작업을 이어간다.

### Debug Client `abort()` 팝업

- `Client/Bin/Debug/Client.exe` 실행 직후 Microsoft Visual C++ Runtime Library의
  `abort() has been called`가 발생하면 창 핸들이나 흰 배경의 Win32 창이 존재한다는 이유로
  시작 성공으로 처리하지 않는다. Lobby 첫 렌더 프레임이 보여야 시작 smoke 성공이다.
- 확인된 사례에서는 `CMainApp::Ready_Fonts()`가
  `Client/Bin/Resources/Fonts/161ex.spritefont`를 요구했지만 실제 파일이
  `Client/Bin/Resources/Fonts/Fonts/161ex.spritefont`에 있어 한 단계 중첩돼 있었다.
  DirectXTK `BinaryReader`가 `0x80070002` 파일 없음 오류를 기록한 뒤 `SpriteFont` 생성자에서
  C++ 예외 `0xE06D7363`을 던졌고, 처리되지 않은 예외가 `std::terminate()`와 `abort()`로 끝났다.
- `CCustomFont::Initialize()`의 `make_unique<SpriteFont>()`는 실패 시 `HRESULT`를 반환하기 전에
  예외를 던질 수 있다. 이 경계는 2026-08-05에 `std::exception`과 알 수 없는 예외를 포착하고
  생성 중 객체를 정리한 뒤 `E_FAIL`을 반환하도록 수정됐다. merge에서 이 catch를 제거하면
  `CCustomFont::Create()`의 `FAILED(Initialize())` 경계를 건너뛰고 `abort()`가 재발한다.
- 복구 전에는 `Fonts/Fonts` 중첩 여부와 다음 필수 파일이 `Resources/Fonts` 바로 아래에 있는지
  확인한다: `161ex.spritefont`, `YG760.spritefont`, `YG330.spritefont`,
  `YoonGasiIIM.spritefont`, `BMKkubulim.spritefont`. 임의 fallback 경로를 추가하지 않고
  immutable resource pack의 올바른 구조로 Hydrate한다.
- 제품 오류 표시는 font 생성 예외를 파일별 로드 경계에서 포착해 실패 경로를 보존하고 기존
  Client 초기화 실패 메시지 경계로 전달한다. `catch (...)`는 C++ 표준 예외가 아닌 DirectX 경계도
  process abort로 빠지지 않게 하는 마지막 변환이며 성공으로 삼지 않고 반드시 `E_FAIL`을 반환한다.
  기본 폰트나 다른 디렉터리로 자동 fallback해 정상 시작으로 위장하지 않는다.
- 수정 후에는 Debug Client를 다시 빌드하고 사용자가 아무 입력 없이 실행해 Lobby 렌더와 `abort()` 부재를
  눈으로 확인한다. 에이전트는 공유된 결과와 종료 후 잔류 Client process 부재, resource pack의 `Verify`
  결과를 별도로 확인한다.

### 프로젝트·데이터 등록

- 물리 C++ 파일, `.vcxproj`, `.vcxproj.filters`의 등록을 세트로 비교한다.
- 삭제한 manifest나 생성물을 project가 계속 등록하지 않는지 확인한다.
- Git 관리 `Data` 원본은 Client 프로젝트의 `96.DataFiles` 아래 `None`으로만 노출한다.
- `Client/Bin/Resources`는 팀장이 관리하는 runtime 입력이다. 존재하지 않는 asset pack lock이나 immutable manifest를 새 완료 조건으로 만들지 않는다.

## 3. 병합 후 필수 감사

실행하지 않은 항목을 PASS로 쓰지 않는다.

```text
1. conflict marker와 unmerged path 0개
2. Dimensionist/DimensionMaster 잔류를 활성 코드·데이터와 역사/원본 자료로 분류
3. Effect 레거시 파일·symbol·project 등록 0개, G1 파일·등록 존재
4. Character Preview 공용 경로와 project/filter 등록 확인
5. 변경 JSON과 XML parse
6. 관련 focused harness와 Debug build
7. 사용자가 직접 수행한 Character Select 재진입 또는 Effect Tool 수동 smoke의 서면 결과
8. 변경 domain publisher의 `Validate`/`Check`와 실행형 focused harness
9. git diff --check
10. 잔류 Client/Server process와 listener 확인
```

개인 PC 경로, 실행 중인 세션 메모, 임시 예외는 `.md/GB/gotchas.local.md`에만 기록하고
Git에 커밋하지 않는다.

## 12. Effect 복원에서 비싸게 배운 것 (2026-08-17 실측)

### 12.1 데이터가 맞아도 담을 그릇이 없으면 화면에 안 나온다

Track A는 원본 추출과 매핑에 성공했다. 실패는 그 다음에 났다.
`Data/Effects/Authored` 3,400 element 실측이다.

```text
source 소유 1,909 element 의 Detail 적용률
  scale / maxParticles / particleLife / startSize / timingLife   100%
  anchor 48%   position 39%
  rotation 1%   color 0%   velocity 0%
```

정적인 축은 전부 정확히 들어갔다. 안 들어간 것은 **시간에 따라 변하는 축과 방향 축**이고,
이유는 추출 실패가 아니라 `Detail` 스키마에 그 개념이 없어서다.

`sourceRecipe.modules` 인구조사가 어디에 정보가 남았는지 말해준다.

```text
particlemodulesizemultiplylife     2259   수명에 따른 크기
particlemodulecolorscaleoverlife   1982   수명에 따른 색·알파
particlemodulelocation             1343   스폰 형태
particlemoduleparameterdynamic     1361
particlemodulecolor                1324
particlemodulevelocity              580   초기 속도
```

**교훈**: 추출이 끝났다고 복원이 끝난 것이 아니다. 저작 스키마가 그 축을 표현할 수 있는지를
추출 전에 확인한다. 표현할 수 없으면 추출한 값은 문서에 남아도 화면에 오지 않는다.

### 12.2 재생 소유권이 저작 수치를 무효로 만든다

`sourceRecipe.enabled`가 true면 재생이 원본 모듈을 따라가고 저작 `Detail`은 무시된다.
`Effect_Playback.cpp:669` 외 6곳이 그 게이트다.

```text
Detail.Transform    게이트 없음   -> 크기·위치 튜닝은 먹는다
Detail.Particle     게이트 있음   -> particle 수 튜닝은 안 먹는다
```

"어떤 스킬은 되고 어떤 스킬은 안 된다"의 정체가 이것이다. 스킬 차이가 아니라 **건드린 축의 차이**다.
튜닝이 안 먹으면 먼저 이 플래그를 본다.

### 12.3 제품이 보는 문서와 저작할 수 있는 문서가 달랐다

같은 스킬에 문서가 네 갈래였다.

```text
.unified                 sourceRecipe 소유   <- runtime catalog 99개 중 98개가 이것
.effect.json             저작 소유           <- catalog 에 없다
.authored-baseline       저작 소유           <- catalog 에 없다
.restoration-candidate   저작 소유           <- catalog 에 없다
```

저작 가능한 문서는 화면에 나오지 않고, 화면에 나오는 문서는 저작이 무시됐다.
**저작 전에 그 문서가 runtime catalog 에 실려 있는지 확인한다.**

### 12.4 중복 판정의 기준을 바꾸면 답이 뒤집힌다 (2026-08-17 재실측으로 교정)

이 절은 원래 "Track A import 가 source occurrence 당 element 를 만들어 같은 시각 요소가
5~6번씩 들어갔다"고 기록했고 `element 7,861 -> 3,042`, `210.9 MB -> 84.9 MB`를 근거로 삼았다.
**그 수치는 잘못된 signature 의 산물이며 중복은 실재하지 않았다.** 되돌리기 `413e4e36`
이후 같은 corpus 8,219 element 를 세 기준으로 다시 셌다.

```text
binding (slotId, assetId) 만          4,759   57.9%
binding + transform                   4,003   48.7%
element 전체 (id/displayName 제외)        6    0.1%
```

되돌린 규칙이 합쳤을 4,471쌍을 열어보면 detail.particle 11,855, sourceNode 4,449,
detail.timing 2,847, detail.transform 1,386, 심지어 kind 109 가 서로 다르다. 한 텍스처를
여러 위치·크기·입자수로 배치하는 것이 이펙트 구성 방식이므로 binding 기반 판정은 공간
구조를 파괴한다. 워로드 17030 이 21 -> 9 로 줄고 손튜닝 하나가 사라진 것이 그 결과다.

`NO_RESOURCES` 일괄 삭제도 틀렸다. light 44 중 28, screenPost 56 중 39 는 텍스처를
바인딩하지 않는 것이 정상이다.

**교훈**: 일괄 삭제 전에 무엇을 동일성의 기준으로 삼았는지 먼저 쓰고, 그 기준을 한 단계
엄격하게 바꿨을 때 답이 얼마나 달라지는지 재본다. 두 수치의 차이가 크면 기준이 틀린 것이다.
남은 진짜 중복 6개도 additive 로 겹쳐 그려지므로 지우면 그 element 밝기가 절반이 된다.

### 12.4.1 소유권 flip 은 축별 게이트가 아니다

`sourceRecipe.enabled = false` 는 `Effect_Playback.cpp` 28개 지점에서 시뮬레이터 전체를
갈아탄다. 따라서 이식 도구가 "해석 못 하는 모듈은 건드리지 않는다"고 해도 flip 이후에는
그 모듈이 실행되지 않으므로 보존이 아니라 삭제다.

```text
source 소유 particle 4,609
   모든 모듈이 저작 스키마로 표현 가능      109   2.4%
   최소 한 축을 잃음                     4,500  97.6%
   주요 손실: parameterdynamic 3,058, cameraoffset 1,713, rotation 1,614,
              meshrotation 1,161, orientationaxislock 1,023, subuv, orbit, acceleration
```

그리고 `Detail.Color.multiply` 와 `Detail.Transform` 은 source 소유 element 에서도 이미
합성되어 먹는다(`Effect_Playback.cpp` ~4996 의 `ElementColor * Particle.vColor`). 막혀 있던
것은 `Detail.Particle` 축뿐이다. 그래서 소유권을 내리는 대신 저작 배율을 원본 결과 위에
곱하는 `Detail.Particle.sourceScale` 을 넣었다. 축이 더 필요하면 flip 이 아니라 같은 방식으로
하나씩 추가한다. 상세는
`.md/GB/08-17/2026-08-17_EFFECT_SOURCE_TRIM_AND_DEDUP_CORRECTION_RESULT.md`.

### 12.5 원본 동일의 비용 단위는 스킬 수가 아니라 exact program과 ABI다

도화가 F가 가장 높은 화면 완성도를 낸 것은 generic profile 하나에 맡긴 결과가 아니라, stable
occurrence와 resource 역할을 고정하고 다음 translated/typed 셰이더들을 실제 carrier에 연결했기 때문이다.

```text
Shader_Artist31470RuntimeMaterial.hlsli         34 sample
Shader_Artist31470Active003RibbonMaterial.hlsli  2
Shader_Artist31470Active011OuterMaterial.hlsli   4
Shader_Artist31470Active022DecalMaterial.hlsli   1
Shader_Artist31470Diagnostic.hlsli               6
```

`g_SourceTexture0..6`을 실제로 샘플링하는 것은 이 파일들과 decal adapter 뿐이고,
표준 경로 `Shader_EffectCommon.hlsli`는 이름 있는 5개만 샘플링한다.

**교훈**: "원본과 동일"은 element나 스킬마다 셰이더 한 벌을 요구하지 않는다. equation이 같은
occurrence는 translated HLSL program과 renderer adapter를 재사용하고 texture·CB·sampler 차이는
exact descriptor가 소유한다. equation이 다르면 새 program, VF/pass/scene/RT topology가 다르면 새
adapter가 필요하다. 도화가 F에서 사람이 쓴 전용 파일은 이 경계를 처음 증명한 선례이지,
스킬별 renderer 복제를 정본으로 만든 근거가 아니다. 현재 공정은
[`EFFECT_FAMILY_RUNTIME_ABI_RESTORATION_GUIDE.md`](../TEAM/EFFECT_FAMILY_RUNTIME_ABI_RESTORATION_GUIDE.md)를
따른다.

### 12.6 판정자 없는 목표를 세우지 않는다

`100% 복원`, `원본과 동일`은 판정할 oracle 이 없으면 완료를 선언할 수 없는 목표다.
그런 목표는 진척감을 없애고, 그 자리를 커밋 수와 문서 수 같은 대체 지표가 채운다.

목표를 쓰기 전에 **무엇을 보면 끝났다고 판정하는가**를 한 줄로 먼저 쓴다.
그 판정자가 없으면 목표를 바꾼다. 화면에 변화 없는 작업이 이틀 연속이면 경보로 취급한다.
## Valtan source carrier를 system-wide mesh로 합치면 100배 WModel과 carrier 붕괴가 함께 난다

- 발탄 ParticleSystem 하나에 mesh emitter가 있다는 이유로 같은 system의 모든 emitter에
  `meshModel`을 복사하면 안 된다. emitter별 원본 carrier가 정본이며 Sprite, Mesh, Decal,
  Light를 각각 보존해야 한다.
- `build_valtan_stage_effects.py` 계열의 clip aggregate는 source 감사 자료일 뿐 V1 Product 입력이
  아니다. reviewed occurrence와 `carrierKey + sourceOrder + rendererShape`가 exact join된 행만
  candidate element가 될 수 있다.
- `Effect/Valtan/Meshes/**/*.wmodel`을 사용하는 exact Mesh carrier는
  `detail.mesh.modelPreScale=0.01`을 반드시 가진다. 런타임 기본값 `1.0`을 쓰면 같은 WModel이
  100배로 렌더링된다.
- Sprite/Decal/Light carrier에는 `meshModel`과 `modelPreScale`을 넣지 않는다. exact resource와
  portable runtime closure가 닫힌 Sprite/Mesh/Decal은 source material identity를 보존한 채
  `effect.standard + alpha_two_sided_depth_read` 공통 RT0로 손튜닝 시작점을 만들 수 있다. Light,
  ScreenPost, resource/adapter 미해석 행은 임의 quad/mesh로 위장하지 않고 `BLOCKED_REQUIRED`로
  남긴다. 공통 RT0 승격은 family 복원이나 `V1_COMPLETE`를 뜻하지 않는다.
- 회귀 검증은 후보 전체에 대해 `rendererShape=mesh <=> meshModel 1개 + modelPreScale 0.01`과
  `rendererShape!=mesh => meshModel 0개`를 함께 검사해야 한다.
- 발탄 materialization receipt가 전체 `EffectCatalog.json` 해시를 봉인하면 다른 캐릭터가 catalog
  행 하나를 추가하는 것만으로 발탄 검증이 실패한다. receipt는 `effect.valtan.` slice와 catalog
  formatVersion만 봉인하고, 전체 catalog 보존은 publisher가 담당해야 한다. 그래야 병렬 캐릭터
  복원과 발탄 exact carrier 검증이 서로의 Product를 지우거나 재봉인하지 않는다.

## Valtan Product presentationScale 정본은 1.0이고 10배 대상은 본체 HP다

- 2026-08-28 사용자 정정에 따라 `Data/Actors/BossCatalog.json`의
  `BOSS_VALTAN.presentationScale`은 `1.0`이 정본이다. `10.0`은 HP 10배 요청을 시각 배율에 잘못 적용한 값이므로
  다시 복원하지 않는다.
- Server 권위 정본 `Data/Balance/BossProfiles.json`의 일반 `BOSS_VALTAN.maximumHp`는 `600000`,
  `maximumHealthBars`는 `160`이다. 종속 `BOSS_VALTAN_GHOST.maximumHp`는 `60000`을 유지한다.
- Server `collisionRadius: 1.4`와 공격 hit geometry는 scale-one gameplay 기준을 유지한다.
- `GAMEPLAY_FOOTPRINT`는 owner basis scale을 제거한 뒤 authored world scale을 적용하므로,
  Effect footprint 보정이나 미세 scale drift 허용을 이유로 boss presentationScale을 바꾸지 않는다.
- `test_valtan_model_view_composition.py`가 `1.0`과 Server body radius `1.4`를 함께 고정하고,
  gameplay balance/Server 계약 테스트가 본체 HP `600000`을 고정한다.

## Valtan strict join과 Effect Tool에서 재발시키지 않을 경계

### Boss Pattern/All Effects/Composition 목록이 함께 사라지면 공용 graph admission부터 본다

Boss Tool, Effect Tool의 `All Effects -> Valtan`, Composition Patterns는 서로 별개의
패턴 catalog를 갖지 않는다. 셋 모두 `Valtan.gameplay.json`,
`Valtan.presentation.json`, generated `ValtanEncounter.json`, rotation, Effect binding,
BossCatalog/combat-object를 strict join한 동일한 canonical Pattern tree에서 목록을 투영한다.
따라서 아래와 같은 화면은 Effect resource나 ImGui category 자체의 문제가 아니라 공용 graph
admission 실패의 연쇄 증상일 가능성이 가장 높다.

```text
No Valtan pattern inventory was staged
0 canonical patterns
canonical Valtan graph did not load
live only; outside All Effects list / UNKNOWN ACTION
Save/Restart button disabled
```

Server는 이미 publish된 bootstrap으로 계속 패턴을 실행할 수 있지만 Client Tool의 새 graph만
거부될 수 있다. 이때 `live only`와 `UNKNOWN ACTION`은 “Server에 패턴이 없다”는 뜻이 아니라,
현재 Server snapshot의 `patternId/actionId`를 Client의 admitted tree에서 resolve하지 못했다는
뜻이다. 화면의 마지막 증상부터 고치지 말고 `Graph reload failed:` 뒤 최초 strict-join 오류를
먼저 고친다.

한 field가 세 화면을 모두 비운 구조적 이유도 함께 기억한다.

- split parser는 모든 managed Pattern을 한 transaction으로 join하고 한 Stage 오류에서 전체 candidate를
  rollback한다. partial authoring tree를 정상 Product처럼 보여 주지 않는 것은 맞지만, 기존 구현은
  generated Product 표시보다 strict split join을 먼저 실행했다.
- last-good snapshot 보존은 같은 process에서 한 번 이상 성공한 뒤에만 가능했다. process restart 직후
  첫 load가 실패하면 Boss Tool, All Effects, Composition 모두 보존할 snapshot이 없어 0개가 됐다.
- 세 Tool이 한 process-level snapshot을 공유하지 않고 각각 같은 tree를 reload했다. 그래서 하나의
  source 오류가 세 곳에서 서로 다른 빈 화면과 버튼 비활성화로 반복 노출됐다.
- Boss Tool과 Effect Tool은 첫 자동 load 실패 뒤 `attempted` 상태가 남는다. 파일을 고친 뒤에도 explicit
  `Refresh/Retry Graph Load` 전에는 다시 시도하지 않아 “수정했는데 여전히 0개”처럼 보일 수 있다.
- reference/legacy/live-only Pattern 하나가 Complete Play 목록에 없는 것은 정상 필터다. 이 경우 graph
  자체는 loaded이고 해당 row만 `[live only; outside All Effects list]`다. 이번처럼 graph 전체가
  `did not load`인 경우와 혼동하지 않는다.

이번 작업과 인접 세션에서 실제 확인한 원인은 다음과 같다.

| 최초 오류/증상 | 실제 원인 | 재발 방지 |
|---|---|---|
| `split gameplay defaultNextActionId drifted: VALTAN_CATCH_BREATH/STEP_02` | Catch 성공/실패를 `ANY_PLAYER_GRABBED -> STEP_03`, `TIMEOUT -> terminal`로 바꾸면서 author script와 generated Product는 갱신했지만 split gameplay의 호환 필드 `defaultNextActionId=STEP_03`이 남았다. strict reader는 명시적 TIMEOUT target인 `null`을 기본 edge로 요구하므로 전체 graph를 거부했다. | branch를 바꾸는 writer는 `defaultNextActionId`를 독립 입력으로 받지 않고 `TIMEOUT.nextActionId`, ordered fallthrough, terminal 순으로 derive한다. `VALTAN_CATCH_BREATH/STEP_02`의 source는 반드시 `defaultNextActionId: null`이어야 한다. author script, split source, generated Encounter를 같은 transaction/revision으로 닫고 negative drift fixture를 실행한다. |
| `split gameplay SET_PLAYER_BIND event is invalid` | Bind event의 exact typed 계약과 source가 달랐다. ENTER는 `heightM=5`, `durationMs=stage duration`, EXIT는 `heightM=0`, `durationMs=0`이어야 하며 unknown/extra field도 거부한다. 한 event의 실패가 split master 전체를 거부했다. | event schema, author script, split source, projection과 runtime consumer를 같은 변경으로 수정한다. parser를 느슨하게 하거나 legacy event로 fallback하지 않는다. |
| `split gameplay SET_PLAYER_SILENCE event is invalid`와 `Encounter stage action lifetime is not closed: VALTAN_SILENCE_SLOT` | `SILENCE_APPLY`는 100ms Stage에서 침묵 5000ms를 한 번 설정하는 deadline-latched 계약이다. Publisher와 Server는 `ENTER only`, `duration >= Stage`, pattern 밖 deadline 만료를 정본으로 사용했지만 Client strict reader는 `duration == Stage`를, Product reference reader는 paired `EXIT`를 요구했다. strict와 fallback이 동시에 실패해 All Effects에는 `EXISTING AUTHORED EFFECTS`와 별도 Area `INDEPENDENT EFFECT`만 남고, Composition/Boss 목록과 Arena presentation admission까지 연쇄 차단됐다. | Client strict source reader와 Product fallback reader가 각각 deadline-latched Silence를 승인하게 하고, Publisher/Server와 동일한 truth table을 focused/native parity 회귀로 고정한다. Silence는 lifetime closure set에 넣지 않는다. actual current JSON을 compiled `ValtanPatternAuditionServiceHarness`로 로드해 ENTER 5000ms 승인, EXIT/value 0/duration < Stage 거부, rollback을 검사한다. Python publisher PASS만으로 Client admission PASS라고 결론내리지 않는다. |
| `master independent Effect did not resolve to one Product owner/document` | V2 도끼를 보이게 하려는 변경에서 independent Effect master row를 추가했지만 exact Product cue/combat-object owner 또는 authored Effect 문서가 정확히 하나로 join되지 않았다. runtime binding 변경과 authoring ownership 변경을 섞어 전체 tree를 깨뜨린 사례다. | 단순 runtime visual 교체는 runtime binding만 바꾼다. independent row는 실제 Product owner, cue timing, authored document가 모두 존재할 때만 추가한다. V1/V2 편집 도구의 목록 소유권을 Product pattern owner로 위조하지 않는다. |
| `BOSS_VALTAN combat-object visual identity is invalid or duplicated` | BossCatalog의 `combatObjectVisuals`에 같은 stable `combatObjectArchetypeId`가 중복되었거나 빈 `clientVisualId/effectAssetId`가 들어갔다. | catalog/projection은 archetype당 visual row 정확히 하나를 보장한다. 기존 row를 교체할 때 append하지 말고 stable ID로 replace하며 combat-object source와 exact join한다. |
| `Valtan scripted-sequence Product parity drifted` 또는 `0 canonical patterns` | saved Boss audition Flow의 occurrence order와 automatic Product rotation order를 하나의 동일 order로 오인해 exact-equal 비교했다. 서로 다른 소유자를 한 revision처럼 묶으면서 전체 tree가 fail-close했다. | saved Flow reference는 Boss Tool audition order를, Product rotation은 자동 전투 order를 각각 소유한다. schema/ID 존재는 함께 검증하되 두 order의 equality를 요구하지 않는다. legacy inline sequence만 기존 parity를 유지한다. |
| old Client에서 새 motion/schema를 연 뒤 모든 목록과 Play가 차단됨 | Data는 새 `{kind, retargetDelayMs, speedMps, distanceM}` 계약인데 실행 중 EXE는 구 parser였다. build가 실행 중 Server/Client의 출력 잠금에서 멈췄는데도 새 EXE로 오인했다. | build 호출 성공 여부가 아니라 Client EXE timestamp/receipt와 실제 strict graph load를 확인한다. 실행 중 EXE의 `LNK1104/MSB302x`는 compile과 link를 분리해 보고하며 old EXE로 새 Data를 검증하지 않는다. |
| Save Flow validation 실패 후 Restart 비활성화 | Save adapter가 합법적인 cross-pattern `COUNTER_HIT -> GROGGY`를 구형 same-pattern/local-action 규칙으로 거부하거나, 첫 candidate가 pending인 동안 두 번째 Save를 유실했다. | Counter success는 local action 또는 cross-pattern target 중 정확히 하나를 허용하고 TIMEOUT은 local failure edge로 검증한다. pending 중 두 번째 Save는 latest deferred candidate로 보존하고 첫 exact terminal 뒤 제출한다. 저장 성공, Server-active revision, 현재 실행 revision을 별도 상태로 표시한다. |
| 첫 Save는 되지만 두 번째 Save/Restart가 계속 막힘 | Apply A 결과 packet을 놓치면 Server가 이미 A를 active로 사용해도 Client transaction이 `UNCONFIRMED`에 남아 deferred B를 영구 대기시켰다. | 성공 packet을 추측해 `COMMITTED`로 만들지 않는다. 현재 연결/월드가 명시한 `ServerActiveRevision == immutable A`일 때만 `ALREADY_ACTIVE`로 reconcile하고, 그 exact A를 base로 queued B를 제출한다. 다른 revision, 다른 world의 관측, concurrent transaction에서는 계속 fail-close한다. |
| source commit 뒤 `COMMIT_SUCCEEDED_REOPEN_FAILED`, Flow는 clean인데 Save/Restart 재개 버튼 없음 | source CAS는 이미 성공했지만 editor reopen/Product publish/apply가 뒤에서 실패했다. Flow dirty flag는 clean이므로 Save 버튼은 비활성이고, 같은 source를 다시 쓰지 않고 post-commit 단계만 재시도할 typed 경로가 없었다. | durable committed revision과 당시 draft generation을 별도 보존한다. `Retry Product Publish / Apply`는 newer edit가 없을 때 그 exact revision을 reopen하고 Product publish/apply만 계속하며 source writer를 다시 호출하지 않는다. 새로운 edit가 있으면 retry가 이를 버리지 않고 거부한다. |
| Save Flow 직후 Lobby fallback과 `Server entry failed` | graph와 별개인 Server process 사망이었다. candidate artifact SHA-256 함수가 1 MiB local stack buffer를 만들었고 1 MiB Server thread stack의 함수 진입에서 `0xC00000FD` stack overflow가 발생했다. Client의 Lobby 문구는 그 뒤 연결 대상 Server가 사라진 후속 증상이다. | 해시 chunk는 bounded size를 유지하되 heap storage를 사용한다. 사용자가 수동 종료한 경우와 Server crash/fallback을 process exit code, dump/structured recovery state로 분리한다. `Server entry failed`만으로 graph 오류라고 결론내리지 않는다. |
| `Canonical Save validation failed; every source/Product owner was preserved` | 편집 source 한 곳만 바뀌고 이를 참조하는 generated Product, owner join 또는 publisher receipt가 아직 이전 revision이었다. validator가 reject한 것은 정상 rollback이며 파일이 저장되지 않았다는 뜻일 수 있다. | Save는 `parse -> validate -> stage -> source/Product CAS -> project -> post-validate -> commit` 한 transaction으로 수행한다. 생성물을 직접 고치거나 validator를 우회하지 않고 첫 stable ID/field 오류를 해결한다. |

현재 `author_valtan_phase_two_mechanics.py --mode Validate`와
`Project-ValtanPatternMaster.ps1 -Mode Validate`가 branch target 존재만 검사하면
`defaultNextActionId` drift를 놓치는 false negative가 생길 수 있었다. authoring helper와 focused
fixture에서 다음 불변식을 직접 검사한다.

```text
explicit TIMEOUT exists  -> defaultNextActionId == TIMEOUT.nextActionId
no TIMEOUT, next stage   -> defaultNextActionId == ordered next action
no TIMEOUT, terminal     -> defaultNextActionId == null/absent
```

목록 표시와 mutation admission도 분리한다. generated Product가 정상이라면 fresh launch에서도
Product pattern/action 목록은 `PRODUCT_ONLY / READ ONLY`로 표시한다. split authoring strict join이
실패하면 Save, Complete Play, Restart, Repeat, Next와 source mutation은 계속 fail-close하고, 화면에는
실패한 `patternId/stageId/field`를 그대로 남긴다. reload 실패 시 last-good display snapshot을 지우지
않는다. Product-only 표시를 authoring 성공으로 승격하거나 서로 다른 generation을 섞지 않는다.

첫 load가 source/Product writer의 짧은 lock 구간과 겹친 경우는 semantic invalid와 다르게 처리한다.
`Create/Project transaction is active`, Win32 sharing violation, admission 뒤 generation change는
transient failure다. All Effects와 Boss Tool은 last-good 또는 read-only Product fallback을 유지한 채
0.25초 간격으로 다시 admission을 시도한다. `attempted=true`만 남겨 fresh process를 영구 0 rows로
고정하지 않는다. 반대로 stable ID/schema/owner/join 오류는 자동 무한 재시도하지 않고 최초 오류를
그대로 표시한다.

Effect V2 closure에는 owner lane이 둘이다. animation의
`Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json`에서 reachable한 group/leaf뿐 아니라,
`BossCatalog.json`의 `BOSS_VALTAN.combatObjectVisuals[].effectV2Group`도 포함한다. 점프 도끼의
`boss.valtan.axe`는 후자다. native receipt test가 bindings만 expected closure로 계산하면 정상 도끼
group을 extra artifact로 오판한다. runtime loader와 회귀 fixture가 두 owner lane을 같은 집합으로
계산해야 한다. `SET_PLAYER_SILENCE` stage-action 오류를 Effect 파일 오류로 오인해 V2 binding을
삭제하거나 Arena admission을 느슨하게 만들지 않는다.

Effect V2 validator를 독립 fixture에서도 재사용할 때 `Data/Actors/BossCatalog.json`은 optional
owner lane이다. 실제 제품 저장소처럼 문서가 존재하면 schema/version/Valtan visual owner를 끝까지
strict 검증하지만, Effect V2만 만든 격리 fixture에 문서가 없으면 빈 owner 집합으로 처리한다.
파일을 무조건 열면 제품 Effect는 정상인데 모든 Effect V2 단위 테스트가 `FileNotFoundError`로
무너져 Core가 compile 전에 중단된다. 반대로 제품 저장소의 손상된 BossCatalog를 optional이라는
이유로 건너뛰면 안 된다. 부재 허용, 정상 owner admission, 존재하지만 invalid인 문서의 fail-close를
세 개의 회귀 경로로 유지한다.

canonical writer-lock 회귀와 Debug Core는 같은 checkout에서 병렬 실행하지 않는다.
`test_valtan_pattern_master_v2`의 transaction fixture가
`out/ValtanPatternTransactions/create-pattern.lock`을 소유하는 동안 Core의 native reader가
`Create/Project transaction is active (Win32 33)`을 반환하는 것은 정상 transient contention이다.
이를 source/Product 손상으로 기록하거나 stale 파일을 삭제하지 말고 장기 writer 회귀를 종료한 뒤
Core를 직렬 재실행한다. 제품 UI는 같은 상태에서 last-good을 보존하고 자동 재시도만 수행한다.

### Effect 세대를 한 화면에 합칠 때 backend catalog를 다시 직접 순회하지 않는다

V1 authored 문서는 `elements[]` 전체가 하나의 원자적 composition이고, V2는 leaf와 ordered group을
분리해 저장한다. 두 저장 형식을 하나로 보이게 한다는 이유로 V1 element를 V2 leaf처럼 펼치거나,
같은 V1 문서를 배치 수만큼 복제하지 않는다. 공용 authoring resource 계약은
`EFFECT_RESOURCE_KEY { ownerKind, stableId }`와 immutable `CEffectResourceCatalog` snapshot이며,
`V1_DOCUMENT`와 `V2_GROUP`은 같은 `Groups`, `V2_LEAF`는 `Leaves`에 표시한다. 이는 무손실 group
승격이지 V1 JSON을 불완전한 V2 field로 변환하는 migration이 아니다.

- All Effects는 `CEffectResourceCatalog` facade의 owner-kind과 stable ID를 소비해 V1/V2를 한
  화면에 표시한다. Action Composition Workbench는 현재 `V1 Pattern Effects`,
  `V2 Authored Effects`, `V2 Effect Groups`를 각각 명시적 owner lane으로 유지한다. 단일 writer가
  없는데 facade snapshot을 공유한다고 기록하거나 backend을 혼합하지 않는다.
- 선택 identity는 해당 backend의 owner kind와 stable ID를 함께 보존한다. V1 document append는
  기존 exact clip cue writer로, V2 leaf/group append는 typed stage binding writer로 dispatch한다.
  현재 코드에는 V1/V2/Camera/Catalog를 하나로 묶는 canonical mutation coordinator가 없으므로,
  이를 기존 저장 경로의 완료 계약으로 가정하지 않는다.
- owner refresh 또는 facade join이 실패하면 이전 snapshot은 표시용으로 유지하되 append/save는
  `STALE PRESERVED / READ ONLY`로 막는다. 실패한 새 owner와 이전 다른 owner를 섞어 새 snapshot처럼
  표시하지 않는다.
- Ground Roar 4방향 배치는 V1 active/explode 문서를 24/4 element로 복사하는 문제가 아니다. active
  6개와 explode 1개인 원본 atomic group을 유지하고 Server combat-object volley가 boss-relative
  `radiusM=4.9497475`, `startAngleDegrees=45`, `angleStepDegrees=90`의 root 네 개를 만든다.
  boss yaw 0도 기준 각 root는 X/Z `(3.5,3.5)`, `(3.5,-3.5)`, `(-3.5,-3.5)`,
  `(-3.5,3.5)`이며 boss yaw를 따라 함께 회전한다. element 복제와 root instancing을 동시에 적용하면
  16배 occurrence가 생기므로 회귀가 두 계약을 함께 검사해야 한다.

Save/Restart 진단에서는 한 문장인 `SAVED`를 다음 상태로 나눠 확인한다.

```text
SOURCE_COMMITTED -> EDITOR_REOPENED -> CANDIDATE_PUBLISHED
                 -> APPLY_PENDING -> SERVER_ACTIVE -> FLOW_RESTART_ADMITTED
```

- 앞 단계 성공은 뒤 단계 성공을 뜻하지 않는다. source commit 성공 뒤 reopen 실패라면 source를 다시
  쓰지 않고 post-commit retry를 제공한다.
- `UNCONFIRMED`는 실패도 성공도 아니다. exact Server-active revision 관측 전에는 다음 revision을
  제출하지 않는다.
- Restart는 saved Flow content, latest candidate, Server-active Product revision, presentation generation을
  각각 exact 비교한다. 버튼을 억지로 활성화하거나 이전 candidate로 fallback하지 않는다.

`Restart Pattern`과 `Restart Flow`는 같은 명령이 아니다. 과거 Boss Verification의
`Restart Saved Pattern (Fresh Arena)`는 내부에서 `Restart_SavedFlow(true)`를 호출했기 때문에,
saved slot이 하나일 때는 실제로 `Restart Flow`와 완전히 같은 Flow packet과 arena reset을 사용했다.
이 one-slot alias가 두 기능을 같은 것으로 보이게 만든 원인이므로 다시 만들지 않는다.

| Tool 명령 | wire/runtime | reset 범위 | 재생 범위 |
|---|---|---|---|
| `Play Selected Pattern (Keep Arena)` | `PLAY_PATTERN_ID` | Valtan boss-only reset. 현재 wall/floor/prop/collision/Nav를 유지하고 교체되는 boss-source combat object만 취소하며 player-source object는 유지 | 선택한 Pattern 하나를 첫 Stage부터 재생. saved Flow, Next, Wait는 소비하지 않음 |
| `Restart Active Pattern (Keep Arena)` | `RESTART_PATTERN_ID` exact predecessor CAS | 같은 boss-only reset. 현재 arena를 유지하고 교체되는 boss-source combat object만 취소 | 이 Tool이 소유한 exact ACTIVE/COMPLETED Pattern occurrence 하나를 첫 Stage부터 교체 재생 |
| `Restart Saved Flow (Fresh Arena)` | `C2S_DEBUG_VALTAN_PATTERN_FLOW_START` | world destruction과 encounter prop을 포함한 authoritative arena reset | disk의 전체 saved `scriptedSequence`를 Pattern 01부터 시작하고 saved order/Next/Wait를 끝까지 소비 |

따라서 single Pattern 버튼에 `Fresh Arena`를 쓰거나, Flow 버튼을 `Pattern Restart`라고 부르지 않는다.
Server 회귀에서는 Pattern ID branch가 `Reset_ValtanBossOnlyAuditionState`만 호출하고
`Reset_ValtanAuditionState`를 호출하지 않는지, Flow start branch는 destruction/prop preflight 뒤
`Reset_ValtanAuditionState`를 호출하는지를 함께 고정한다.

변경 후 최소 검증은 한 도구의 목록 개수만 보는 것으로 끝내지 않는다.

```text
python -m unittest Tools.ValtanPipeline.test_valtan_pattern_master_v2
python -m unittest Tools.ValtanPipeline.test_valtan_pattern_tree_contract
python -m unittest Tools.EffectPipeline.test_effect_tool_valtan_all_effects_contract
powershell -ExecutionPolicy Bypass -File Tools/ValtanPipeline/Project-ValtanPatternMaster.ps1 -Mode Validate
powershell -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-GameplayBalance.ps1 -Mode Validate
Tools/ValtanPatternAuditionServiceHarness/Bin/Debug/ValtanPatternAuditionServiceHarness.exe
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Core
```

특히 목록/Save/Restart/Arena admission을 바꾼 작업은 Python 정적 검사와 `Product` compile만으로
완료 처리하지 않는다. current source와 current generated Product를 실제 Client C++ reader로 여는
native harness가 PASS해야 한다. 과거 Debug harness timestamp가 수정된 C++보다 오래되면 그 결과도
증거가 아니므로 먼저 다시 build한다.

그 뒤 새 Debug EXE에서 사용자가 `Boss Tool`, `All Effects -> Valtan`, `Composition Patterns`를
각각 열어 pattern count, category, live action resolve를 확인한다. 한 화면만 복구됐으면 공용 snapshot
경계가 아직 분리된 것이므로 완료가 아니다.

### `serverMotion`의 takeoff stage는 이름이 아니라 ordered entry다

- `takeoffStartMs/takeoffEndMs`의 소유자는 `stageId == "TAKEOFF"`가 아니라 `entryActionId`와 일치하는
  첫 ordered stage다. `travelStageId`는 그보다 뒤의 고유 stable stage여야 한다.
- `VALTAN_SIX_PIZZA_106`의 첫 stage는 정본 `STEP_01`이다. validator를 통과시키려고 이를 `TAKEOFF`로
  개명하면 Effect, Camera, Product occurrence join을 함께 깨뜨린다.
- 한 pattern의 strict join 실패로 전체 All Effects inventory가 rollback되는 것은 정상 fail-close다. partial
  inventory나 legacy fallback으로 숨기지 말고 오류에 `patternId`와 실제 stage/action identity를 남긴다.

### Map Effect catalog 등록과 entry-required Product 준비는 다른 gate다

- catalog 행, authored Effect 파일, `.mapeffects.json` world row가 모두 있어도 portable `sourceRecipe` codec
  admission 또는 prepared Product commit이 실패할 수 있다. catalog 재등록만 반복하지 않는다.
- entry-required Map Effect에 실패도 `settled`로 보는 optional prewarm 정책을 적용하면 Level activation 뒤
  `Map Effect world target is absent...`라는 후속 증상만 남는다. 최초 `incremental prewarm failed`의 asset ID와
  codec 오류를 Level 진입 실패 원인으로 보존한다.
- 일반 Particle source recipe는 Required 1개, Lifetime 1개 이상, Spawn 1개를 요구한다.
  `particlemodulecolorscaleoverlife`는 color와 alpha distribution을 모두 가져야 한다. validator 약화나
  `sourceRecipe.enabled=false`로 입장을 통과시키지 않는다.

### 선언/정의가 일치하는 `LNK2019`는 실제 provider obj를 확인한다

1. 같은 working tree의 다른 MSBuild, CL, FXC, linker와 publisher를 먼저 멈춘다.
2. 선택한 `Configuration|Platform`의 evaluated `IntDir/OutDir`를 확인한다.
3. 참조하는 consumer obj가 아니라 provider obj에 `dumpbin /symbols`로 정의 심볼이 있는지 검사한다.
4. 심볼이 없으면 해당 translation unit만 강제 재컴파일한 뒤 최종 link를 한 번 수행한다. broad output 삭제나
   두 번째 전체 빌드를 겹치면 `.tlog` 잠금과 서로 다른 시점의 obj 혼합을 만든다.
5. `LNK1104`, `MSB3021`, `MSB3027`이 EXE/DLL을 가리키면 실행 중 출력물 잠금이다. compile 성공과 link
   차단을 분리하고, 현재 실행 중인 EXE는 새 obj가 반영되지 않은 이전 바이너리라고 보고한다.

### publisher의 `exit code 1`은 최초 오류가 아니다

- MSBuild가 출력한 마지막 `명령이 종료되었습니다(코드: 1)`은 wrapper 결과다. 그 앞의 첫 terminating
  error에서 domain, phase, stable ID와 path를 확보한다.
- `Publish-GameplayBalance -Mode Publish`는 destination 교체 전에 Valtan strict validation도 실행한다.
  먼저 `-Mode Validate`로 source/schema/join 실패를 분리하고 통과한 뒤에만 destination lock, promotion,
  rollback을 조사한다.
- Server contract-test의 기대값 실패는 pre-build publisher 실패와 별도 단계다. 생성 bootstrap 직접 수정,
  validation skip, 실행 중 Server/Client 위에 publisher/build 반복 실행으로 숨기지 않는다.

### All Effects의 `Delete Effect`는 소유권에 따라 의미가 다르다

- `[PRODUCT]` 행 삭제는 선택한 Pattern의 exact cue 연결만 split `Valtan.presentation.json`에서 제거하고
  candidate source CAS 뒤 Product를 투영한 다음 단일 `Validate` postcondition을 실행한다. 이전 Product parity를
  source Save 전에 요구하면 새 source와 이전 Product의 정상 drift를 실패로 오인한다. 공유 `EffectCatalog` 행과 authored Effect 파일, 다른 Pattern
  연결은 보존한다.
- `DRAFT_ATTACHED`만 sidecar row와 deterministic `Effects/Authored/<effectId>.effect.json` 파일을 함께
  삭제할 수 있다. Product catalog/cue 참조가 있으면 파일 삭제를 거부한다.
- Draft 삭제는 sidecar CAS를 먼저 commit하고 파일을 같은 handle에서 canonical compare-delete한다. 파일
  삭제가 실패하면 sidecar를 CAS rollback한다. unsaved 편집이 있거나 선택 이후 cue/baseline이 바뀌면 삭제하지 않는다.
- generated `Valtan.patterneffectcues.json`을 직접 편집하지 않는다. Product 연결 변경의 정본은 split
  presentation이고 Server 재생 판정은 publish 후 Server 재시작·Arena 재진입 뒤에 한다.

#### 삭제 직전에는 캐시가 아니라 정본을 다시 잠그고 읽는다

- `CEffectCatalog::Find()`나 현재 화면의 Pattern tree는 표시용 snapshot이다. 사용자가 확인 modal을 보는 동안
  다른 publisher가 Effect를 Product에 등록할 수 있으므로, 이 캐시만 보고 authored 파일을 삭제하면 안 된다.
- Draft 생성·삭제의 destructive preflight는 Effect catalog, split gameplay/presentation, Encounter/rotation,
  animation binding/cue/alias/stage-Effect, BossCatalog/combat-object까지 `CValtanPatternTree`가 소비하는 complete
  source read set을 read handle로 열어 concurrent write/delete를 막은 상태에서 catalog와 Product graph를 새로
  parse한다. 어느 하나라도 parse/lock에 실패하면 파일을 보존하고 Refresh를 요구한다.
- modal을 열 때 복사한 `kind + patternId + effectAssetId + cueIds + alias`와 확인 순간의 선택이 하나라도 다르면
  삭제하지 않는다. render-frame vector pointer가 아니라 stable identity만 확인 대상으로 보존한다.

#### publisher를 소유한 child process는 timeout으로 죽이지 않는다

- Product unlink는 source commit 뒤 원자적 Product projection + `Validate` postcondition과 실패 시 source/Product rollback을
  수행한다. managed cue scale-policy migration 표는 현재 cue의 허용 정책 ledger이며 live cue 전체 개수를
  고정하지 않는다. sealed legacy Effect cue도 전역 배열 ordinal이 아니라 stable `bindingId`로 검증한다. 이 child를 180초
  timeout에서 `TerminateProcess`하면 PowerShell의 catch/finally가 실행되지 않아 source/Product가 반쪽 상태로
  남을 수 있다.
- Effect Tool은 unlink process를 비동기로 시작하고 매 frame exit만 poll한다. 180초는 경고 기준일 뿐 종료 기준이
  아니다. 작업 중에는 All Effects 편집을 잠그고, Client가 닫혀도 process handle만 닫아 child가 commit 또는
  rollback을 끝내게 한다.
- exit 0에서만 unlink 성공으로 표시한다. nonzero나 process observation 실패에서는 disk를 다시 읽되 보존 여부를
  추측하지 않고 최초 publisher 오류를 확인하게 한다. child가 아직 실행 중일 수 있는 observation failure에서는
  All Effects를 다시 열어 두지 않는다. 잠금을 유지해 두 번째 publisher transaction이 겹치는 것을 막는다.

#### atomic replace backup은 post-commit 검증 뒤에만 지운다

- `File.Replace`가 성공한 직후 실행되는 byte verification도 실패할 수 있다. replace 완료 flag를 verification 뒤에
  세우거나 backup을 `finally`에서 무조건 지우면 이미 바뀐 source를 baseline으로 복구할 수 없다.
- replace 직후 commit flag와 recovery backup path를 먼저 기록하고, replacement bytes를 다시 확인한 뒤에만 backup을
  삭제한다. post-replace 실패는 보존한 backup/CAS rollback으로 baseline을 복원하며, 복구까지 실패하면 backup의
  정확한 경로를 오류에 남긴다.

### Model View target 교체와 Create auto-open은 저장 transaction과 분리한다

- Character Select의 Model View가 같은 `Valtan` asset을 다시 publish해도 target generation은 바뀔 수 있다.
  asset 이름과 포인터만 비교하면 synchronized sequence가 generic update에서 비워진 뒤 다시 stage되지 않는다.
- Pattern Draft는 매 frame generic synchronized update보다 먼저 generation mismatch를 확인하고, 동일 Pattern의
  ordered timeline을 새 generation에 다시 stage한다. target 교체를 문서 unload나 빈 sequence의 정상 종료로
  처리하지 않는다.
- `Create Effect`의 durable 성공 경계는 authored Effect 파일과 `DRAFT_ATTACHED` sidecar가 모두 CAS commit된
  시점이다. 그 뒤 Model View 준비나 auto-open이 실패해도 두 파일을 rollback하지 않는다. `files remain
  committed`와 preview 실패 원인을 분리해 보고하고, 사용자가 `Open Editor`로 재시도할 수 있게 한다.
- 실행 중 `Client.exe`는 이 새 source를 반영하지 않은 이전 바이너리다. 사용자의 tuning 세션을 강제 종료하거나
  그 위에 link하지 말고, 종료 신호 뒤 한 번 재빌드한 새 EXE에서 target replacement와 Delete UI를 확인한다.

### 오래된 worktree의 PatternTree 전체 파일로 All Effects를 덮어쓰지 않는다

- `피자 패턴 바닥 이펙트 가이드` 작업에서 확인된 회귀처럼, 최신 strict join 위에 오래된 worktree 파일을
  통째로 덮으면 개별 Effect 문제가 아니라 Valtan tree 전체 admission 실패로 나타난다.
- `Boss Tool`이나 Pattern Flow를 추가할 때 과거 계획서·stash·별도 worktree의
  `ValtanPatternTree.h/.cpp` 또는 `Effect_Tool.cpp` All Effects 본문을 전체 복사하지 않는다. 현재 working copy의
  strict join은 누적 계약이며 `partDamagePolicy`, `counterProxy`와 transactional previous-tree 보존 중 하나라도
  사라지면 `F1 -> Effect Tool -> All Effects -> Valtan` tree 전체가 fail-close로 비어 보일 수 있다.
- 작업 전후 no-touch diff를 비교하고 다음 focused contract를 함께 실행한다.
  `python -m unittest Tools.ValtanPipeline.test_valtan_pattern_tree_contract Tools.EffectPipeline.test_effect_tool_valtan_all_effects_contract`
- reload 실패는 기존 admitted tree를 지우지 말고 exact parse/join 오류를 표시한다. 자동 검증 뒤에도 사용자가 새
  Client에서 All Effects의 Valtan 28개 Pattern과 Stage tree가 실제로 열리는지 확인해야 visual PASS다.

### gameplay source가 없는 encounter에 공용 Dataset/runtime부터 만들지 않는다

- 모델, animation chain, 추출 asset 폴더가 존재하는 것과 Server-authoritative encounter source/Product가
  존재하는 것은 다른 계약이다. 현재 Valtan은 기존 canonical source/Product 경로와
  Tool/Server의 직접 reader를 소비하며, 별도 `ENCOUNTER_DATASET` registry나 공용
  `BossPatternGraphRuntime`을 완료 계약으로 두지 않는다.
- `KAKULSAYDON`은 public logical ID이고
  `KoukuSaton`은 실제 animation/resource 저장 alias이므로 spelling을 통일한다는 이유로 raw asset을 바꾸거나,
  존재하지 않는 Kakul gameplay source/Product 경로를 catalog에 추가하지 않는다.
- 향후 공용 Dataset/runtime을 도입하려면 새 encounter source/Product를 먼저 publish한 뒤
  실제 두 소비자 이상을 한 변경 단위에 이전한다. invalid absolute/`..` path, identity mismatch,
  missing/duplicate stage, action+pattern dual branch target, follow-up depth 32 경계를 같은 변경 단위의 native
  contract와 structural oracle로 닫는다. registry/helper만 먼저 만들면 Tool 목록은 생겨도 Save/Restart가 별도 정본을
  참조하는 두 번째 경로가 다시 만들어진다.
