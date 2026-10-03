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
