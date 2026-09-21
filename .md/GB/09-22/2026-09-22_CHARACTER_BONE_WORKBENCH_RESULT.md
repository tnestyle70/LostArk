# Character·Bone·Valtan Workbench 반영 결과

## 구현

- Character class 목록을 현재 7종(GuardianKnight 포함)으로 연결했다. 기존 PlayerSkills/skillbinding/combat owner를 유지하고 공유 Composition Resources tree를 사용한다.
- 광기·Mario·카드미로 광대와 VehicleCatalog 탈것은 CharacterModelWorkbench adapter에서 기존 EffectAuthoringSequencer stage/clip/effect/sound/logic row와 box 편집, seek/play, 저장을 사용한다. 광대는 interaction mode+slot, 탈것은 vehicleId+skillId/source action stable ID다. local combat 판정은 추가하지 않았다.
- generic sequence Save와 Product binding Save는 구분된다. Product는 최신 선택 subtree CAS 병합으로 저장하며 다른 skill/optional shake/root sound 필드를 보존한다. mount idle/run의 상시 cue는 외부 owner 표시이며 별도 Mount effects action에서 ambient/mount cue 배열을 저장한다.
- BoneAnimationDocument/Workbench는 native clip segment와 bone-local TRS key를 JSON으로 저장한다. 실제 model skeleton과 bone을 사용하고 quaternion normalize/slerp, source dependency DAG, time/finite/count를 검증한다. CModel의 기존 animation channel 재생/seek를 확장했고 native clips는 불변이다.
- playable/vehicle prototype과 Character body admission에 authored clip loader를 연결했다. 따라서 탑승 skill의 authored 이름도 기존 mounted model animation consumer가 재생한다. 실제 바다 WModel과 본으로 만든 flight/glide/ascent 3개는 원본이 아닌 **authored 예시**다.
- Valtan Box Detail에 본/무기 editor를 연결했다. 도끼 attachment bone `b_wp_r_01`의 rotation key를 저장할 수 있다. source bone Save와 기존 Pattern PublishV2를 분리하고 validated Product를 `Data/Valtan/Published/Valtan.boneclips.json`으로 원자 게시한다. ANIMATION generation closure와 실제 Valtan prototype load에 포함했다.
- Valtan source clip 앞/뒤 trim을 rate 기반 source milliseconds로 연결했다. 순차 clip 앞 trim은 후속 clip을 당기며 server stage duration을 바꾸지 않는다. 짧아진 EXACT는 HOLD_LAST_POSE, 증가로 넘는 구간은 기존 admission에 따라 거부한다. Box Detail Save와 Ctrl+S, Ctrl+D/Delete는 기존 owner 명령을 사용하며 gesture 중 revision 변경/Escape는 취소한다.
- Saydon의 row도 saved occurrence에서 계산하는 표시 row다. 별도 영구 빈 row schema를 만들지 않았다. Valtan/Ghost preview donor를 BossCatalog Cinematic donor와 맞춰 177/290 clip으로 표시한다.

## 검증

- Engine Debug x64 build, Client ClCompile 통과(최초 integrated Bone/Character slice).
- 후속 변경 파일은 MSVC C++20 syntax 검사 통과: BoneAnimationDocument/Workbench, ValtanActionWorkbench/Timeline, ValtanPresentationAssetService/GenerationAdmission, Animation_Tool_DocumentIo, CharacterPreviewPanel, CharacterModelWorkbench, EffectAuthoringSequencer/Resources, CharacterActionWorkbench, Model.
- WARP console + 실제 AncientMyth CModel 31검사 PASS: load/source pose/bake/seek/replay/native보존/invalid rollback/CAS.
- 실제 AncientSea WModel 41검사 PASS: native9clip/91bones, 실 wing/root basis, authored3예시 포함.
- 실제 Valtan body+Cinematic donor CModel 16검사 PASS: native177, authored 추가, source Save만으로 Product 미변경, Published load/pose동일성/기존seek재생, corrupt Product의 기존모델 보존.
- shared trim helper 13검사 PASS. Valtan source Save 5검사 PASS: CAS/무관 owner/no-op/sidecar rollback/async wrapper.
- `test_valtan_authored_bone_publish.ValtanAuthoredBonePublishTests` PASS(19조건): 실 WSKL/도끼 본, malformed source/key/cycle 거부, CAS read set, 기존 durable publish와 generation closure, 실제 FOUR_SLASH에 authored clip 참조 후 게시, stale source commit 거부/이전 Product 보존.
- 추가 구 canonical typed-patch regression 1건은 commit 자체 성공 후 `changedCount` 고정 기대6/현재Data7 차이로 assertion 실패했다. 본 기능의 optional bone 파일이 없는 fixture이며 오래된 총 파일수 assertion은 수정하지 않았다. 신규 bone publisher 검사와 분리한다.
- 검증 로그: `out/CharacterWorkbench20260922/*result.log`, `*.final-syntax.log`.

## 사용 및 남은 확인

Bone Clips Save 후 Valtan Pattern source revision Reload와 Composition Resources Refresh를 거쳐 authored clip을 넣는다. Pattern Save는 source, Publish는 Product 교체이며 arena 재입장이 model prototype reload 경계다. dirty draft는 자동 폐기하지 않는다.

Client·아레나 UI를 자율 실행하지 않았다. 실제 화면, HUD 배치, 도끼 자세 및 dragon flight/glide/ascent 연출 판정은 사용자 확인 대상이다. 새 본 애니메이션은 시각 animation이며 탈것의 Server 비행 이동/충돌 권위를 새로 만들지 않는다. 원본에 없는 Sea 5 clip 참조는 synthetic native clip으로 만들지 않는다. Source stop-only Wwise4event는 fake WAV를 만들지 않는다.

새 C++6파일은 Client.vcxproj/.filters 기존03Tools05Sequencer에 등록했다. 신규 Python2파일은 C++ 프로젝트 등록 대상이 아니다. 설치·양쪽 최종 Product build·GBResources 전달 상태와 기능 PR은 통합 RESULT 및 세션 최종 전달을 따른다.

## Guardian·Sea 원본 TrailGhost와 현재 owner의 자세

`guardian_afterimage_projection.py`는 EFGame reflection의 22개 필드 순서와 원본 payload 길이/SHA256을 검사하여 Guardian 10 stage의 12 notify와 Sea 대시 1 notify를 기존 Model Cue Afterimage로 투영한다. 발생 시작/끝, 주기, 자식 수명, 첫 alpha, source diffuse intensity, 시작/끝 rim 색상, part enum, only-local flag를 보존한다. source material/pivot reference가 있거나 미구현 scale/view offset/force-remove 의미가 있으면 투영을 거부한다.

`CEffectPresentationService`는 실제 `CCharacter` weak owner callback을 제공한다. provider가 body·현재 장비·실제 무기 socket world, 해당 장비의 own/shared bone palette, visibility/hidden mesh mask를 읽는다. 탑승 중에는 rider 몸체 대신 `CPart_Vehicle`이 제공한다. CharacterPreviewPanel의 generic mount/광대 모델은 AnimationTargetService에 weak part provider를 게시한다. generic target이 선택된 동안 scene Character로 잔상 대상이 새지 않는다. 원본 clip을 별도 복제한 고정 갑옷 잔상은 만들지 않았다.

`CSkeletalAfterimage`는 기존 history carrier를 확장했다. emission이 period보다 짧아도 첫 실제 pose가 기록된다. 키 프레임 사이의 미관측 자세는 합성하지 않는다. 각 child는 실제 palette/world/mesh mask를 보관하며 pause·expiry·seek·outfit 변경 때 정리한다. static socketed weapon은 기존 mesh shader에 추가한 pass23, skinned outfit은 기존 animated pass14를 사용한다. GPU에 history를 bind한 뒤 현재 palette/world/normal matrix를 복원하며 CModel pose/cursor를 쓰지 않는다. 새 pass도 기존 source-program group의 BASE/UNAVAILABLE 정책을 따른다.

원본 enum 이름은 `EFTG_NONE=0`, `EFTG_WP=1`, `EFTG_ALL=2`다. 0을 현재 base outfit, 1을 socketed weapon, 2를 전체 outfit+weapon으로 표시하는 정책은 프로젝트 해석이다. **원본 rim exponent/fade/material ABI는 복구되지 않았고 appearanceBasis는 PROJECT_AUTHORED다.** 원본 parameter의 소비, 실제 owner의 자세, 원본 shader 동일성은 별개다.

추가 검증:

- 변경 Client C++ 12 TU의 MSVC C++20 `/Zs` 통과. mesh/animated shader base의 `fx_5_0` 컴파일 통과.
- 실제 설치 Guardian body+장비6개+무기 및 Sea dash CModel로 기존 `CSkeletalAfterimage::Render`를 실행하는 WARP console 검사 **97/97 PASS**. Guardian 실제 `b_wp_1` socket을 확인했고 8부위 각각 colored RGB가 양수·nonfinite0이다. Sea `npc_sk_dash`도 colored190pixel·nonfinite0이다. 실제 palette bitwise 불변, 첫 샘플·pause·expiry·seek/reset을 검사했다.
- WARP shader fixture는 current base bytecode에 별도 논리 파일명을 써서 무관 source group 로드를 생략했다. 전체 source group pass/variable 호환은 통합 Product build의 검사 범위다. Client/window/level은 실행하지 않았다.
- 원본 payload3종의 typed decoder 및 거부 경계 5 unittest PASS: WP/local-only/짧은발생, SHA 손상, invalid enum/flag/NaN, 미지원 scale, stable notify identity, 빈 stopped emission.
- Root의 `inheritParentRotation=false` 변경은 actual `Effect_Playback.cpp`를 새로 컴파일한 matrix probe **8/8 PASS**. own rotation과 scale, world attachment 위치, follow anchor를 확인했고 nonidentity SourceTransformTrack을 합성한 경우도 통과했다. production C++는 이 검사에서 수정하지 않았다.
- 증거: `out/CharacterWorkbench20260922/afterimage-result.log`, `parent-rotation-result.log`, shader `*.afterimage-fxc.log`, `guardian-afterimage-receipt.json`, `sea-afterimage-receipt.json`. 최종 Cue Codec/전체 Product 검증은 통합 및 Guardian owner 결과와 함께 기록한다.


## 최종 보조 소비자 검사

- static native4530/4531에서 쓰는 실제 원본 mesh3종(`sky_seamless_sm`, `bg_chs_stone_foothold01_sm_lkj`, `bg_chs_stone_foothold02_sm_lkj`)의 CModel을 원래 mesh preScale .01로 로드했다. Render_Mesh가 native source texture와 별개로 요구하는 기본 diffuse 바인딩이 세 모델 모두 S_OK다. local bounds와 정점 draw가 유효하고 기존 static geometry shader 경로에서 colored6560/3606/2455pixel, nonfinite0으로 확인했다. 이 검사는 **geometry/material dependency 검사**이며 native4530/31 pixel shader 복원 동일성은 통합 owner의 별도 original-DXBC replay 결과를 따른다. Afterimage 및 이 검사 합계는 `afterimage-result.log`의 **115/115 PASS**다.
- 가디언 S 두 stage의 원본 Lifetime0을 current Codec/Playback로 실행했다. clip0 source start .168041에서 +.05/.2, clip1 start0에서 +.05/.2 모두 active1/normalizedLife0/alpha1이다. 실제 Guardian `bip001-l-hand`와 skillbinding `ddk_sk_flame_scale_01/02`의 시간을 fixed-step TransformHistory provider에서 샘플했다. source0을 manual lifetime1로 대체하지 않은 문서를 그대로 사용했다.
- 첫 실행에서는 clip0 원본 occurrence의 끝 .8초 이후에도 입자가 남는 결함을 발견했다. Guardian owner가 기존 `Update_Particles`에 occurrence/owner-sustained lifetime 종료를 연결하고, sourceRecipe에만 manual lifetime0을 허용하도록 검증 경계를 수정했다. fresh Playback+Codec Validation 객체로 재검사한 결과 clip0 .9초, clip1 .398021초에서 active0이며 owner Reset도 빈 frame을 만든다. **28/28 PASS**, `zero-lifetime-result.log`. 이 보조 검사는 production 코드를 직접 변경하지 않았다.

## Guardian 원본 visibility·궁극기 camera 마감

- 원본 `EFActionNotify_IdentityParts` 9개와 `HidePawn` 3개의 serialized payload를 SHA/길이 검증 후 원본 reflection 순서로 해석했다. identity target12, whole pawn target0, weapon target9만 기존 ownerControls로 전달했다. 실제 source9개는 MakeParts=false이며 임시 identity 표시 억제로 연결한다. Server stance나 기존 사용자 part visibility를 변경하지 않는다. 공통 enabled는 기존 PlayParticleEffect decoder와 동일한 signature+12 계약이다. final notify 뒤의 stage footer를 HidePawn 추가 flag로 해석하지 않는다.
- `CPart_Equipment`에 weapon/identity 역할과 독립 suppression을 추가했다. `CCharacter::Set_PresentationVisibilityControls`가 기존 live owner overlay 합계를 body/equipment/vehicle preview 및 afterimage 제출에 반영한다. 기존 visibility와 승인 stance를 보존하므로 overlay가 끝나면 현재 상태로 복원된다. EffectObject의 Product action token/preview clock 집계와 codec/40행 설치는 rendering owner 결과를 따른다.
- 원본 `UltimateSkillCameraControl` PLAY0/STOP3800ms 2개가 가리킨 `SkillCam_DragonKnight_02`를 실제 `STANDARD_SKILLCAM_DRAGONKNIGHT` package에서 찾았다. RemoteEvent123→Attach110→Matinee26/Data31의 parent Move100, CM02 Move102, FOV81을 사용했고 disabled Move99를 제외했다. Engine의 AttachToActor CDO hard-attach/zero relative 기본값까지 조회했다. 저장 Hermite tangent를 120Hz 및 원본 knot 시간으로 샘플해 기존 recovery sequence에 **458키**를 저장했다.
- 기존 All Effects `Read_RecoveryCameras`와 Product `Prepare_FrameCamera`가 같은 effect stable ID의 camera sequence를 소비한다. Character `Stage_CharacterAction`도 공통 Camera row에 가져오며 앞/뒤 source trim과 rate를 같은 시간으로 환산한다. 앞trim이 start0 FXcue를 제외하더라도 원본 camera import는 유지한다. F6 free camera에서는 authoring override를 해제하고 기존 priority/다른 owner 선점/Stop/owner reset 해제 흐름을 재사용한다. Camera 저장은 기존 Effect Sequence owner이며 skillbinding 저장과 분리한다.
- 원본 곡선과 1ms 간격 **3801샘플**을 대조했다. 최대 eye 축 오차 **0.001152m**, lookAt **0.001217m**, up **0.000125**, horizontal FOV **0.011119도**다. UE cm→Client meter와 parent TRS를 한 번 적용했고 live MODEL_ROOT scale/rotation/translation 및 viewport horizontal→vertical FOV 변환을 수치로 확인했다. 실제 `CEffectRecoveryCamera.cpp` Load/Parse/Sample, 원문 CValtan camera sampler, 원문 Stage_CharacterAction clip-match/camera-import/trim-filter block을 사용하는 console 검사 **53/53 PASS**다. trim0/500ms×rate0.5/1/2, start0 effect 제외와 camera 유지, exclusive STOP 및 inactive sample의 기존 pose 보존을 확인했다. 이는 console 소비자 검사이며 Client 화면이나 실제 사용자 F6 조작을 실행한 증거는 아니다.
- 원본14 payload/12 visibility/458camera key 및 SHA·invalid bool·disabled notify·unknown recipient·변경된 camera graph 거부 테스트 **7개 PASS**. 최종 Character, Part_Equipment, EquipmentPresentationService, EffectAuthoringSequencer, EffectAuthoringSequencer_Camera **5TU MSVC syntax EXIT0**다. 최종 Product/원본 Desktop 적용/리소스 배포 상태는 통합 RESULT의 최종 기록을 따른다.
- `HidePawn.hideStatusEffectFX` 2개 및 Ultimate PLAY의 `hideIdentityBuffParticle`는 현재 Guardian에 독립된 status/buff FX presentation owner가 없으므로 source receipt에 남겼다. active skill FX 전체를 대신 숨기지 않는다. visibility는 명시적 `PROJECT_ADAPTER`이며 원본 native visibility/Light/material 시간 ABI 전체가 닫혔다고 기록하지 않는다.
- 증거: `out/CharacterWorkbench20260922/guardian-visibility-controls12.receipt.json`, `guardian-ultimate-controls2.receipt.json`, `guardian-camera-source.json`, `inspect_camera_defaults.json`, `guardian-camera-projection.receipt.json`, `camera-stage-exact-source.receipt.json`, `camera-result.log`, `owner-controls-syntax.json`. PLAN은 현재 파일 **50개 전체 코드**로 갱신했고 SHA 목록은 `final-plan-source-snapshots.json`이다.
