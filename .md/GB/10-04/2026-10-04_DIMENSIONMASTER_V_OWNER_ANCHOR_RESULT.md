# 차원술사 V 시전자 앵커와 관전자 화면 범위 결과

## G00. 판정 기준과 원인

2026-10-04 사용자 요청은 원본 카메라 부착의 복원이 아니라 **차원술사 시전자 앵커,
local space OFF**다. 다른 가디언 나이트 화면에 V가 붙는 현재 프로젝트 화면을 문제 입력으로
받았다. 첫 시전 장면이라는 설명을 두 번째 시전에서는 정상이라는 캐시 결함으로 단정하지 않았다.

현재 `(DIMENSIONMASTER,V) → 2050520 → pc_sp_m_00_sk_sk_timewave →
effect.dimensionmaster.skill.2050520.full.restore` 연결을 확인했다. 실제 animevent는 이미
root/snapshot/action_facing이다. 그러나 43개 저작 요소 중 17개는 별도의
`camera_view/follow=true/localSpace=true`를 사용했다. `Resolve_SourceAnchors`가 현재 PC의
inverse VIEW를 그 17개의 부모로 제공해 실제 시전자 대신 관전자 카메라를 소비하는 것이
이번 사용자 정책과 충돌하는 직접 원인이다.

`CCharacter::Update_EffectCues`는 자기 `shared_from_this()`를 `Desc.pOwner`로 전달하고
`Spawn_Immediate`는 그 weak owner를 resolve한다. `Resolve_Anchor(root)`는 해당 Character의
world transform을 사용한다. 따라서 remote 차원술사 cue도 로컬 가디언을 다시 선택하지 않는다.

별도 읽기 조사에서 prewarm 준비, native texture staging, 매 draw의 material packet/SRV
재바인딩, native69의 occurrence별 SceneHDR snapshot 갱신을 확인했다. 확정된 초기화 누락이나
이전 재질 잔존 근거는 찾지 못했다. native69의 미사용 초반 레지스터 계산을 출력 결함으로
판정하지 않았다. 색·가림·원본 UV의 완전 복원을 주장하지 않는다.

## G01. 시전자 snapshot 데이터 반영 완료

`Data/Effects/Authored/effect.dimensionmaster.skill.2050520.full.restore.effect.json`의 PLAN에
열거한 17개 stable ID에서 다음 네 값만 바꿨다.

| 필드 | 현재 반영 값 |
|---|---|
| `actionCueAttachment.follow` | `false` |
| `actionCueAttachment.orientation` | `bone` |
| `actionCueAttachment.runtimeAnchorSlotId` | `root` |
| `detail.particle.localSpace` | `false` |

변경은 68개 leaf 값, Git diff 34줄 추가·34줄 제거다. 기존 26개 요소, 43개 전체 구성,
native program과 DDS/WModel, 모든 크기·시간·색·배율, bloom0과 native66 세 유리의
`fresnel_pow=0.5`를 보존했다. 원본 source recipe의 `buselocalspace=true`, source anchor ID와
socket TRS도 출처로 남겼다. snapshot 경로는 이 socket TRS를 사용하지 않으므로 카메라의
0.5m/-90도 보정을 시전자에게 새로 적용하지 않는다. 현재 43개 요소 모두 localSpace=false이며
camera_view attachment는 0개다.

최신 디스크 SHA 확인 후 `ReplaceFileW`로 교체했고 out에 원본을 백업했다. 설치된 파일이
후보와 byte 동일함을 확인했다.

```text
before SHA256: c45d71b521f31b1bd63c6dc49d07fac1f0ec1d2508c0c21b0e43bd5fa66e3652
after  SHA256: e273b145f83deb93c7b8c3d6d4241e83e50f045bfde2f99b9afc4bae05165d71
```

반영 영수증은 `out/DimensionMasterV20261004/install-receipt.json`, 구조·보존 검증은
`final-receipt.json`, 원본은 `atomic-backup.effect.json`이다.

독립 읽기 검토도 live/candidate SHA 일치와 정확 68값 diff, 26개 무변경 요소, source/socket 및
튜닝 보존을 재확인했다. 일반 DIRECT parser의 DetailIo는 `localSpace`를 직접 읽고 Playback은
그 값을 사용한다. 보존한 원본 Required의 `buselocalspace=true`가 이 DIRECT 저작값을 다시
덮어쓰는 경로는 없다. reconstructed/Artist31470 전용 처리를 V에 적용하지 않았다.

## G02. 화면 후처리의 관전 대상 범위 반영 완료

신규 `Data/Effects/Sequences/effect.dimensionmaster.skill.2050520.full.restore.effectsequence.json`에
PLAN의 RGBNoise/ZoomBlur 네 ID만 `localOnlyElementIds`로 등록했다. `cameras=[]`이며
DimensionMaster/skill.2050520의 root occurrence와 `productNaturalDuration`, `productSnapshot`,
`productActionFacing`을 true로 명시해 기존 animevent 정책을 유지한다. duration5800ms는
기존 source emitter+particle tail의 상한을 적은 metadata이며 제품 재생 clock을 바꾸지 않는다.

여기서 local은 입력 권한만을 뜻하지 않고 **현재 카메라가 관전하는 Character**다. 차원술사를
보는 화면은 기존 43개를 유지하고, 가디언 등 다른 대상을 보는 화면은 39개 월드 요소를 유지한
채 네 screenPost만 제외한다. 시전자를 관전하는 상태로 전환하면 네 요소가 다시 허용된다.
첨부의 차원술사 용병도 `COLOSSEUM_MERCENARY_AI → ClientReplication::Create_Character →
CCharacter` 경로를 사용해 기존 Character 필터에 들어간다. CNpc용 새 정책은 추가하지 않았다.

실제 RecoveryCamera Load/Parse, CDataJson, CProjectDataRoot TU와 Service의 Stage/Prepare cache,
Spawn mask, Is_CameraPresentationOwner, Prepare_FrameCamera, Object/Renderer의 제출 필터 본문을
사용한 **70개 검사, 실패0**이다. 이전 remote 39월드+4post 누출을 재현하고 후보에서 39+0을
확인했다. caster 관전43 → 다른 subject39, camera override Begin/Pose 0회, duplicate/잘못된
ID type parse rollback, 없는 ID admission 실패 때 clone 제거, external preview와 다른 스킬
정책 보존을 검사했다. 실제 게임 actor 실행과 GPU 화면 검사가 아닌 owner/camera 경계 모형이다.

신규 경로의 부재를 다시 확인하고 `MoveFileExW WRITE_THROUGH`의 no-replace로 원자 추가했다.
설치 SHA256은 `f52b76dd4b4f1c727500d85da37883fce18ab68d75af100cd0abd54d8162bac8`이다.
증거는 `out/DimensionMasterV20261004/local-screen/`의 `probe-run.log`, `probe-receipt.json`,
`candidate-receipt.json`, `installation-receipt.json`이다. root가 신규 파일 하나를 Client
project/filter의 `96.DataFiles\Effects\Sequences`에 `None`으로 등록하고 XML parse를 확인했다.

## G03. 자동 검증과 배포 경계

생산 `Resolve_Anchor`, `CAnimationEffectCueDocument::Try_ComposeRootTransform`과 그 validator,
`CEffectPlayback::Evaluate_ElementWorld` 전체 본문, 실제 Step snapshot capture와
SpawnRootWorld 저장·local/world root 선택 문장을 추출한 CPU 검사 **1905개, 실패0**이다.
현재 17개 후보의 transform/attachment/timing 값과 실제 DirectXMath, MSVC14.44를 사용했다.
두 차원술사의 서로 다른 위치·action yaw, 별도 가디언 관전자, 카메라 이동과 시전자 이동을
분리했다. 생성 시 시전자 위치/yaw를 사용하고 생성 후 snapshot과 world-space particle root가
유지되는 것을 확인했다. follow/local-space 대조군은 실제로 이동하여 음성 대조도 확인했다.

이 검사는 Owner getter에 독립 Character 행렬을 공급한 함수 경계 검사다. 실제 모델 골격,
전체 particle distribution/fixed-step 재생, GPU draw·색·가림·사용자 화면의 검증이 아니다.
증거는 `probe-provenance.json`, `probe-compile.log`, `probe-run.log`에 있다.

현행 source validator의 element 이름·material color-space·native sprite option·module override·
attachment orientation 다섯 검사와 해당 문서 **44개 DDS/WModel dependency closure**가 통과했다.
catalog의 DIRECT_AUTHORED_DOCUMENT identity와 설치 JSON parse, scoped `git diff --check`도
통과했다. `candidate-validation.json`이 그 범위를 기록한다.

전체 `Tools/EffectPipeline/Validate-EffectSources.ps1`은 이번 V 교체 전부터 기존
`effect.dimensionmaster.skill.2050230.mirror-particle-canary.unified`의 source hash가 현재
`effect.dimensionmaster.skill.2050230.unified`와 달라 실패했다. 로그는
`source-validation-before.log`이며 이 다른 스킬과 audition row는 변경하지 않았다.
전체 validator PASS로 기록하지 않는다.

Effect는 `Data/Effects/EffectCatalog.json`과 Authored JSON을 제품이 직접 읽는다. Effect
publisher나 `Client/Bin/DataFiles/Effect` 복사본은 없다. C++/HLSL 수정과 제품 재빌드는 없으며,
신규 sequence의 `None` project/filter 등록은 root 담당 변경이다. 파일 설치를 실행 중
immutable cache의 자동 갱신으로 설명하지 않는다. 이미 준비된 sequence mask도 파일 추가만으로
바뀌지 않으며 기존 명시적 `Reload_ProductCamera`, 새 resource 준비 또는 새 프로세스가 필요하다.
도구의 재준비/Reload와 다음 실행은
사용자가 선택하며, Client 실행·조작·화면 캡처는 수행하지 않았다.

최종 화면 판정은 미실행이다. 사용자가 차원술사 V를 직접 시전하고 다른 직업 관전자 화면에서
월드 유리·파편은 시전자 위치에 남고 화면 후처리는 관전자에게 적용되지 않는지 확인해야 한다.
원본 게임과의 최종 시각 동등성이나 모든 V carrier의 원본 복원이 완료됐다는 판정은 아니다.

## G04. 새 설치 원본 재대조

기준은 `2645d7cce`와 새 설치 `C:/ProgramData/Smilegate/Games/LOSTARK`다. 현재43개 요소의 원본 particle/material과711개 distribution을 대조했으며 **새로 수정해야 한다고 입증된 source 값 불일치는 찾지 못했다.** 따라서 Data, Resources, C++와 shader를 변경하지 않았다. 이것은 사용자 문제 화면의 해결 또는 원작 화면 전체 일치 판정이 아니다.

10-04의 시전자 root snapshot·Local Space OFF·관전 대상 ScreenPost 정책은 그대로다. 이 원본 재대조는 위 G00~G03의 owner-anchor 결과를 대체하지 않는다.

### 원본과43요소의 연결

기존 LPK reader로 `data3.lpk/Action/DimensionMaster.loa`, `data1.lpk/Projectile/20505200.loa`, `data2.lpk/TableData/EFTable_SkillEffect.db`를 읽었다. 원본 Main25개 notify, V2050520의1.549999952초 Effect20505200 호출, SkillEffect의 fixed-area projectile20505200, spawn offset25cm,3.1초 projectile,4개 즉시 CreateFX와7개 damage-only timer를 기존 decoder로 검증했다. 새 dummy 호출이나 효과 ID를 만들지 않았다.

원본11개 ParticleSystem에서 첫 LOD62개 emitter의 module·archetype·CDO를 해석했다. 현재43요소의 emitter/full material identity43개가 모두 일치한다.711개 저장 distribution은 원본 notify/projectile parameter override까지 적용한 뒤 float32 기준 전부 일치한다. JSON의9자리 소수 표기와 원본 float32의 긴 decimal 표기를 숫자 변경으로 오인하지 않았다. package index 등 소비하지 않는 provenance 숫자나 CDO 기본 literal을 제품 값 오류로 취급하지 않았다.

현재 사라진21개 emitter는 이번 설치의 누락이 아니다. `3fc237507`의64요소가 `160b7a2af`에서43개로 저장되었고, 그43개 stable ID 집합은 현재까지 같다. 과거 편집을 원본 복구 명목으로 자동 되돌리지 않았다.

원본 native66 MIC의 fresnel_pow는0.20000000298이며 현재 세 occurrence의0.5는09-21 사용자 clarity 튜닝이다. 나머지 확인한 numeric/texture identity와 native66/69의 translucent BlendMode를 유지했다. 이름의 `_ad`를 근거로 additive로 바꾸지 않았다. 기본 유리 PS와 별도 distortion pass의 구분도 유지했다.

### 첫 native69의 공간 입력 수치 검증

설치 RefShaderCache에서 실제 material static set로 선택한 PS `5dfee80075c74444bffc1d810344820c`를 다시 추출했다. DXBC SHA256은 `477086f17abc547d7e6858e3d256f3558d067d19279de5c2136770761ff1bd73`이다.

현재 `Shader_EffectDimensionMasterVNative.hlsli::VNative69` 함수 본문, `VNativeSample0`, 주기·append helper와 SceneColorBias 읽기를 그대로 추출해 독립 PS로 컴파일했다. 기존 WARP replay를 재사용했으며 설치 DDS `fx_d_atypical_045.dds`를 명시된 sRGB로 해석하고, 공간적으로 변화하는 SceneColor, linear wrap/clamp, UV4종·화면 위치3종·alpha3종·time2종을 비교했다.72조건 모두 finite이며 원본 DXBC 대비 최대 절대 오차0이었다. alpha0도 유지했다.

이는09-15의 constant SceneColor 검증이 다루지 못했던 공간 sampling 일부를 확인한 것이다. 실제 camera/mesh/billboard·깊이·톤매핑·여러 이펙트의 합성 화면을 재현한 검사가 아니다. native66의 원본 MIC/static map과 PS를 이번 설치에서 다시 확인했으나 native66 전체 화면 GPU 재검증을 실행한 것으로 기록하지 않는다.

### 실제 문서 로드와 Playback

root가 제공한 방식대로 기존 Debug Product OBJ에 이 조사 전용 console entry를 격리 링크했다. Client MainApp, 창, GPU viewport와 게임 actor를 시작하지 않았다. 실제 `CEffectDocumentCodec::Load`가43요소를 읽고 `Stage_Document`에 성공했으며, identity root와 명시된 anchor map으로6초간60Hz Update를 실행했다.43/43요소가 particle/light/ScreenPost frame에 모두 관측되었고 모든 검사 대상 particle 위치와 색은 finite였다. 첫 native69 두 요소의 첫 관측은0.016667초, 후속 묶음은0.716667초, 마지막 camera-origin glass는1.583333초, projectile은1.55초였다.

수치 root/anchor를 사용한 실제 Playback 검사다. 실제 DimensionMaster WModel 본 부착, live 캐시 갱신, 사용자 screenshot 시점의 draw 가시성까지 검증한 것으로 대신하지 않는다. 현재 전체43요소 보존 때문에 source62 emitter 전부의 재생 성공을 주장하지 않는다.

### 산출물과 남은 작업

진단은 `out/DimensionMasterVOriginal20261004`의 `audit_receipt.json`, `recipe_comparison.json`, `historical_removed_21.json`, `projectile_source_contract.json`, `native69_spatial_receipt.json`, `playback.log`, `materials`와`closure`에 보관했다. 독립 compile/link 및 native69 PS/WARP는 성공했고, 제품 빌드·게시·UI Reload·사용자 화면 확인은 실행하지 않았다. 변경 코드·데이터가 없어 추가 제품 빌드나 project/filter 등록은 필요하지 않다.

현재 확인한 source 값과 pixel 식을 이유 없이 바꾸는 후보는 만들지 않았다. 사용자가 owner-anchor/관전자 필터 반영 후에도 같은 문제를 확인하면 그 **현재** element·시간·시점의 geometry/depth/composition을 좁혀 확인해야 한다. 원본 camera attachment 복귀나21개 요소 재추가는 자동 해결책이 아니다.
