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
