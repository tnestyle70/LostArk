# 가디언나이트 클래스 선택 연출 조사·구현 결과

## G00. 현재 반영 상태

사용자의 “일단 재생 가능하게 마무리” 요청에 따라 **카메라·배우·용·활성 FX·SL10 배경을
실제 Data/Resources에 설치하고, WorldSequence와 배경 Map을 정식 게시했다.**
Action Workbench의 `WORLD → Character Select → Guardian Knight / Intro + Loop`에
Play/Restart intro, Pause/Resume, Stop과 시간 탐색을 연결했다. 제품 class 선택과 Workbench는
동일한 `CClassSelectionPresentation`을 호출한다. Debug Product 빌드·배포와 설치된22개
리소스의 실제 WARP CModel 로딩·애니메이션 결합 검증은 통과했지만, 사용자가 EXE에서
Play 실패를 보고했다. **이 검사를 연출 전체의 재생 가능 판정으로 사용했던 결론은 철회한다.**
현재는 실제 초기화·Play 경계를 추가 검증 중이며 최신 상태는 G10에 기록한다.

현재 재생 대상은 설치된 부분이다. 원작 동적 소품18개, 일부 배경 입력, 카메라 shake/DOF/Bloom,
음향이 남아 있으므로 원작 전체 복원 완료로 표시하지 않는다. 기존 Character Select의 Server
입장·class 변경 권한과 사용자가 옮긴11개 바닥의 배치는 유지한다.

현재 일반 Guardian `CharacterCatalog`는 PCPreview702에서 확인한 `class_select_hr00` 의상과
`WP_WDDK04-05` 계열 무기를 연결한다. 원본 컷신은 별도 배우 assembly로, 얼굴
`pc_dk_02_face_sk`, 머리 `pc_dl_56_hair_sk`, 뿔 `cc_dk_prop_06_sk`, HR00 의상과
별도 부착 `wp_ddk_hr_00_sk`를 사용한다. 일반 모델의 머리/얼굴과 컷신 DK02/DL56을 같은
외형으로 간주하지 않으며, 사진과 완전히 일치한다는 화면 판정도 아직 없다.

## G01. 검은11개 바닥과 원본 SL10 컷신

- SL00의11개 floor/star 쌍은 정지 캐릭터 미리보기 자리다. 같은 geometry와 서로 다른
  배치별 RNM을 사용하며 원본 diffuse brightness는 바닥·별 모두0이다. 사용자가 제시한
  차원술사 화면의 검은 팔각 바닥을 누락된 diffuse로 취급하지 않는다.
- 현재 Guardian floor1057/star1058과 중앙336은 수평 약75.31m 떨어져 있다. 약1km 밖에
  있던 바닥을 사용자가 비교 목적으로 옮긴 상태이며, 그 위치를 원본 위치로 기록하지 않는다.
- 원본 `LV_LOBBY_CLASSSELECT_SCENE01`의 `ChangeClass9`는 Matinee75(export702)의
 31.505365초 등장 연출을 시작하고, 완료 뒤 Matinee74(export701)의23.002918초 반복으로
  이어진다. 소개 배경은 별도 Area `LV_LOBBY_CLASSSELECT_SL10`이다.
- SL00 GuardianDragonHuman의 psMatinee103은 약0.995초 customization zoom이다.
  이 카메라와 SCENE01의 클래스 소개 연출은 서로 다른 원본 연결이다.
- SL10 원본 중심은 UE cm `(-135002.609375,169562.203125,-4459.216309)`, 런타임으로
  약 `(-1350.0261,-44.5922,-1695.6220)m`다. 배경과 배우·카메라·FX는 이 WORLD 좌표를 쓴다.

원본 package SHA256은
`69538e81c4a1566795799fc8a5348d1bb0561217db935f0ac76b1d4bab0cc748`이다.
근거는 `out/GuardianCharacterSelectResearch20260922/Source/`와
`current-floor-comparison.json`에 있다. export 번호는0-based다.

## G02. 실제 소비 경로

`CClassSelectionPresentation`은 `Data/Camera/ClassSelection.cinematics.json`의 phase를
parse/validate하고 기존 WorldSequence player로 배우와 애니메이션을 재생한다. WORLD camera
cut/HFOV/roll은 `CEffectRecoveryCamera`와 camera presentation owner를 사용한다.
`clockKeys`, `materialTracks`, `lights`, `effects`는 동일 phase source time으로 평가된다.

재질 곡선은 기존 CModel/CMaterial override, point light는 기존 transient light queue로 전달한다.
원본 eyeAO/hair의 two-sided translucent 재질은 기존 pass9를 소비한다. PSC5개는 static WORLD
root이며 기존 `CEffectPresentationService::Spawn_LevelPlacement/Seek_WorldRoot`를 쓴다.
카메라가 반복돼도 stable PSC ID에 해당하는 handle과 particle history를 유지한다.

Loader는 실제 연출 설정이 있을 때 SL10을 기존 CMapPlacementRuntime의 별도 presentation
Area로 준비한다. FX도 기존 Loading worker preparation queue에 합류한다. 연출 중 gameplay
command와 왼쪽 상세/전투 HUD를 억제하지만 Server state와 socket 권한은 바꾸지 않는다.
Stop/class 전환/Level 이탈은 배우·FX·light와 camera owner를 정리한다.

원본 baked transform이256key를 넘는 행을 가지므로 WorldSequence와 Map publisher의 행 상한을
4096으로 맞췄다. 문서16MiB, track64 등의 제한은 유지했다. Publisher의 native 재질 검증은
고정 program21 대신 실제 generated Client packer의 parameter 이름과 base/light texture mask를
읽는다. Effect runtime은 authored JSON을 직접 읽으며, 폐기된 Effect publisher를 다시 만들지 않았다.

## G03. 설치·게시된 데이터와 리소스

| 대상 | 실제 상태 | 설치·게시 증거 |
|---|---|---|
| 컷신 배우 geometry/재질 |9 source actors, 최종 WS22 split resources; body285본, hair303본, sword11본, dragon115본 |초기 배우 설치 Runtime61/GBResources63 신규, 기존 교체0; actor receipt 별도 |
| 애니메이션 donor |body/hair/wing/dragon4종, 각각 `guardian.select.intro/loop` |Runtime와 GBResources 양쪽 설치 |
| 카메라·시간·재질·light |intro5/loop4 camera cuts, phase당7 point lights 및 지원된 face/hair 곡선 |`Data/Camera/ClassSelection.cinematics.json` 설치 |
| WorldSequence |22resources,44templates,44instances |SL00 authoring 설치, 공식 WorldSequences Validate/Publish/Check PASS |
| 활성 FX |4문서/25elements, intro5·loop4 PSC occurrences; 새 native4576~4585와 기존7재사용 |EffectCatalog/Authored 등록,54개 참조 리소스 존재 |
| SL10 배경 |574 catalog entries,1,356placements,963재질 rows,1,349RNM bindings |Resources1,566파일을 두 root에 설치(3,132신규·기존 교체0), Area Validate/Publish/Check PASS |

SL10은81WModel,1,425DDS,60TGA와 필요한 원본 lighting texture를 포함한다. 배경은 별도
CMapPlacementRuntime가 소비하며1,356개 WorldSequence 객체로 복제하지 않는다.
사용자의 SL00 placement 문서는 이번 배경 설치에서 수정하지 않았다.

설치는 최신 저장본의 stable ID를 병합하고 hash freshness, backup, atomic replacement,
실패 rollback을 적용했다. 연출 데이터/애니메이션/FX mirror transaction은41파일 변경·305동일,
동일 입력 재계획은 변경0이었다. `out`의 후보와 receipt는 근거이며, 제품 소비 파일은 Data와
게시된 `Client/Bin/DataFiles/Map` 및 Resources에 있다.

주요 영수증:

- `out/GuardianCharacterSelectResearch20260922/Installation/final-result.json`, `receipt.json`, `effect-validation.json`
- `out/GuardianClassSelectActors20260922/ACTOR_RESOURCE_RESULT.md`, `resource-install-receipt.json`
- `out/GuardianClassSelectBackground20260922/replay-install-receipt.json`, `replay-publish*.log`

## G04. 검증 완료와 실제 화면 확인 경계

- 최종 설치된 입력의 실제 C++ parser/camera/WorldSequence CPU 검증은 **320,029 checks, exit0**이다.
  phase/particle clock, camera cut 경계, JSON 실패 rollback, 실제 material slot/name/program/
  textureMask/UVMask와 native 상수 finite 검사가 통과했다.
- WorldSequence Load→Validate→Save→Reload 통과. 무기 donor의 빈 optional 필드는 producer에서
  생략하도록 고쳤고 effect start/end는 원본 소수 밀리초를 유지했다.
- FX4문서25요소는 실제 codec Load·Drawable validation·serialize round-trip 통과.
  native group4544 mesh/particle FXC, actor group208/235 animated/static/deferred6개 FXC,
  배경9개 shader FXC 및30개 native configuration 검사가 통과했다.
- 이번 설치 후 SL00 WorldSequences의 공식 Validate/Publish/Check가 통과했다. 카메라/WS
  authoring은 검증된 candidate와 일치하고 게시된22resources/44templates/44instances에
  누락된 WModel/DDS 참조가 없다.
- 새 FX4종은 공식 scoped material/module/orientation 검사와 ClassSelection→Catalog→문서
  연결을 통과했다. FX54resources,3,657,896bytes의 실제 존재를 확인했다. **전체 Effect validator는
  기존 `effect.kouku.gate1.blade-dance.circle.impact`의 v15 runtime carriers 없음으로 실패**한다.
  별도 전체 reachability 검사에서도 기존 Artist unified orphan rows가 나온다. 이 파일들은
  이번 작업에서 고치지 않았으며 새4종 scoped 성공과 전체 검사 실패를 구분한다.
- 설치 경로를 읽는 실제 CMapAssetCatalog::Load_Area/MapPlacementDocument는
  SL10의574 catalog·1,356배치를 수용했다(exit0). 전체598 source DDS의 payload/mip 검증도
  완료했다. 근거는 `replay-installed-catalog-probe.log`다.
- 기존 단계의 Client12개/Engine3개 최소 컴파일, project/filter XML, publisher PowerShell 구문,
  scoped `git diff --check`가 통과했다. 최종 Debug Product는 Engine→Shared→Server→Client 빌드와
  실행 파일·DLL·shader 배포를 통과했다. receipt는 `out/BuildPipeline/runs/20260922T112641140Z-debug-product.json`이다.
- 설치된22개 resource의 WARP CModel 생성,21개 donor 결합,4개 donor,42개 clip window,
  복제 후11,529개 bone pose와43,928,784개 float의 finite 검사가 통과했다(exit0, 약4.5초).
  이는 실제 제품 모델 로더 검사이며, 장면 전체 WorldSequence/FX 렌더링이나 화면 비교를 대신하지 않는다.
  증거는 `InstalledModelProbe/run.log`, `run.json`과 G08의 수정 영수증이다.

최종 CPU 입력과 실행 경계는 `ContractSut/source-boundary.json`, `installed-contract-receipt.json`,
`contract-installed-run.log`에 있다. long-loop 검사는 PSC의 source age 연속성과 float 수명 horizon을
검사했으며 실제 장시간 GPU 실행을 한 것은 아니다.
실제 제품 Client/UI 실행과 장면 전체 GPU 표시, 사용자 원작 화면 비교는 아직 미실시다.

## G05. 남은 원작 동등성

1. 원본 동적 소품18개(crack7, fire plane6, swing1, filmnoise2, zoomblur1, camera dust1)의
   geometry/MIC와 material 곡선이 미연결이다. phase당84 material target applications를
   원본 자료에 보존했으며, 활성 파티클25개를 이 소품 전체의 복원으로 대신하지 않는다.
2. SL10 vertex lightmap6개, foliage wind, scene environment cube/SH/BRDF는 미복원이다.
   해당 원본 배치는 숨기지 않고 유지했다.
3. `pc_common_create.pc_create_dragonknight1` 음원/event 근거는 확보했지만 오디오 재생은
   미연결이다. Camera shake와 DOF/Bloom source tracks도 보존만 했고 아직 소비하지 않는다.
4. 실제 화면에서 유리 물방울·검기·카메라·반복 상태를 비교해야 한다.
   장시간 GPU playback과 사용자의 최종 판정은 수치/codec 검사로 대체하지 않는다.

무관한 미커밋 변경을 정리하거나 일괄 commit/push하지 않았다.

## G06. WORLD 재생·배치·반복 재검토

WORLD 탭의 transport는 새 `CClassSelectionWorkbenchSession`이 소유하고 MainApp의 callback으로
현재 Character Select Level의 동일 presentation을 호출한다. Workbench가 시작한 playback token을
기억하므로 탭을 닫을 때 다른 UI가 새로 시작한 연출까지 잘못 정지시키지 않는다. WORLD는 현재
재생·일시정지·시간 탐색을 제공하며 컷신 전체 배치의 저작/이동 기능을 추가한 상태는 아니다.

11개 미리보기 바닥 이동은 원본 SL10 cinematic의 WORLD 좌표를 자동 이동시키지 않는다.
배우 한 객체나 바닥만 이동해도 카메라·FX·light·SL10 배경은 각각의 원본 좌표에 남는다.
전체 스테이지 이동에는 이 입력 모두를 같은 변환으로 처리하는 별도 저작 계약이 필요하다.

반복 검토에서23초마다 player/clone을 다시 만드는 경로,0.5초 초과 stall의 particle history
재시뮬레이션, 긴 실행 뒤 float 경계가 Lifetime0 물방울을 조기에 지울 수 있는 경로를 확인했다.
현재 코드는 초기화에서 객체 pool을 prewarm하고 첫 intro→loop 뒤에는 같은 player/배우를 seek한다.
visual delta는 프레임당 최대0.25초로 제한하며 source 종료 창에는 최소1초 또는 다음 loop분의
여유를 둔다. 이는 visual preview clock만 조정하며 Server simulation time은 바꾸지 않는다.

정상 진행에서는 PSC stable ID로 같은 handle을 유지한다. 명시적인 Seek만 해당 scene의 particle
history를 다시 구성하고 paused 상태로 둔다. Body285본과 visibility/material의 intro끝→loop첫
상태는 원본 자료로 대조했다. Dragon과 일부 light의 경계 위치 차이는 원본의 서로 다른
camera shot 배치에도 존재하며, 임의로 한 좌표에 고정하지 않았다. 이 검토는 실제 수 시간
GPU 실행의 성능·안정성 보증은 아니다.

## G07. 현재 재생 계약과 원본 속도

Server 승인을 받아 Character Select에 들어간 뒤 Action Workbench `WORLD`에서
`Guardian Knight / Intro + Loop`의 **Play**를 누른다. 재생 중에는 **Restart intro**,
**Pause/Resume**, **Stop**을 제공한다. 타임라인 드래그는 놓는 시점에 해당 phase의 시간을
탐색하고 일시정지한다. **Intro start / Loop start**로 구간 시작을 확인할 수 있다.
Stop은 카메라 소유권과 연출 객체를 정리한다. 설치된 새 바이너리의 최종 검증은 G04와 같다.

Intro source31.505초는 원본 global Slomo를 적분한 실제 약**31.735초** 동안 재생되고,
그 뒤 source/wall **23.003초** loop를 반복한다. 단일 전체 감속값으로 모든 요소를 늦추지 않는다.

| 원본 제어 | source time과 값 | 설치된 처리 |
|---|---|---|
| Global Slomo |3.832359초=1 →4.100511초=0.55 →4.496261초=1 |원본 곡선을 적분한 wall→source `clockKeys` |
| Body B animation |1.478359초 flying1.5배,4.343237초 guillotinespin0.4배·endOffset1.95초 |slot/SkelControl 포함 bone bake, 약4.634903초부터 끝 포즈 유지 |
| Body A animation/blend |3.626868초 시작·startOffset0.3초·rate0.5,4.343237초 blend1 →4.429548초0 |B슬롯으로 넘어가는 원본 혼합 유지; C슬롯은 항상 blend0 |
| Dragon animation |3.990942초 시작·rate5, clip3.033333초 |약4.597609초부터 끝 포즈 유지 |
| Swing particle |4.360836초 활성,4.885574초에 particle rate0 |최종 particle age0.262369초에서 고정 |
| Ash particle |particle rate0.5;4.568159초부터 표시 |숨김 구간의 age도 반영, intro끝15.7525초, loop마다11.5015초 증가 |
| Water particle2 PSC |4.568159초 활성·rate1; burst2/3/5·Lifetime0 |intro끝 age26.936841초, loop마다23.003초 증가; 같은 입자 유지 |

카메라는 intro5cuts/loop4cuts를 계속 따라간다. 그래서 캐릭터·용·검기가 멈춘 뒤에도 카메라가
움직이고 재/물방울의 별도 시간이 진행되는 원작 구조가 유지된다. 유리 물방울은 원본 native
material/distortion 경로로 등록했으나, 현재 화면에서 원작처럼 보이는지의 판정은 아직 하지 않았다.

## G08. 실제 모델 로더에서 발견·수정한 재생 차단 원인

1. Dense60Hz bone bake가 Engine의 clip당1,000,000 TRS key 상한을 초과했다. Body intro
   1,617,660개를213,146개, loop1,181,610개를1,710개로 줄였다. 동일 packed-float 값을 가진
   연속 구간 내부만 제거해 원본 키와 모든 중간 보간 결과를 보존한다.4donor/8clip 전체
   9,704,136개 원본 키를 독립 검증했으며 Engine 안전 상한은 바꾸지 않았다.
2. a744 무기는 skinned11bone 모델이지만 원본 AnimControl 없이 b_wp_1을 따라간다.
   producer가 잘못 추가한 intro/loop animationTrack2개만 제거하고 WORLD 위치·회전 키
   intro446개/loop2개와 rest palette를 유지했다. 공식 WorldSequence 재게시/Check 통과.
3. Dragon donor 복사 후 기존 textures/상대 경로4개가 새 폴더 기준으로 해석돼 재질 생성이
   실패했다. 원본 Resources 상대 경로로 재기록하고 실제 파일 존재와 Runtime/GBResources
   동일성을 검사했다. 모델의 재질 슬롯·geometry·skeleton·animation은 보존했다.

세 수정은 baker/projector에도 반영해 재생성 때 재발하지 않게 했다. 최종 WARP22/22 통과 전에
실행 가능하다고 안내했던 판단은 실제 로더 검증보다 빨랐으며, 최종 완료 근거는 G04다.
증거는 AnimationKeyRepair의 receipt.json, independent-audit.json, weapon-track-repair.json,
weapon-worldsequence-publish.json, dragon-material-repair.json이다.

이 단계의 설치 모델 검사는 장면 전체 Play 성공을 보증하지 않았다. 후속 사용자 실패와
수정은 G09/G10을 따른다. 에이전트가 Client 실행·Reload·사용자 편집 폐기를 수행하지는
않았다. crack7은 별도 masked/unlit carrier이므로
현재 watersplash의 native distortion 성공과 그 소품 복원을 혼동하지 않는다.

## G09. 사용자 화면의 Play 비활성 수정

사용자 화면에서 Play가 비활성이며 Class selection cinematics are not loaded가 표시됐다.
배우22개 admission 통과와 별개로 SL10 배경 준비가 실패하여 Level의 연출 Initialize가 호출되지
않았다. optional 실패가 기존 아레나 입장을 보존했으므로 정상 아레나 표시는 이 실패를 가렸다.

실제 Loader와 같은 CModel 입력으로 첫 SL10 모델 실패를 재현했다. CModel/MapAssetCatalog 및
Bind_SourceCharacterInputs는 native214..234/237 RNM을 허용했지만 CMaterial::Initialize의
supportsBaked만 누락돼 E_FAIL이었다. Engine/Private/Material.cpp의 동일 program 허용식을
맞췄으며 RNM을 제거하거나 배치를 숨기지 않았다.

수정된 Engine으로574/574 = geometry81 + material variants493, clone574, diffuse와 localbounds가
모두 통과했다. UI/Client를 실행·조작하지 않은 WARP 소비자 검사이며 소요 약3.5초다.
최종 Engine SHA256은01f9e37f77c33af660c100c866e0aa6f9afa7cc6ed43b2fc4ec8b3c78caa9c49이다.
후속 확인에서 Client/Bin/Debug/Engine.dll과 Engine/Bin/Debug/Engine.dll이 위 SHA256으로
일치하며21:27:49에 시작한 사용자 Client가 설치 DLL을 사용함을 확인했다. 그런데도 사용자는
Play 실패를 보고했으므로 기존 DLL만의 문제로 설명하지 않는다.

## G10. optional Deploy 검사와 실제 초기화 경계

실제 설치 Engine DLL, 실제 Prototype/Object manager, CMapPlacementRuntime와 map shader로
SL10 배치1,356개 생성이 통과했다.400개 batch 객체에1,054개 배치, 개별302개 객체를 생성했다.
증거는 `out/GuardianCharacterSelectResearch20260922/PlacementProbe/result.json`이다.
창·Client·draw를 실행하지 않은 실제 Object/Layer 검사이며 화면 확인을 뜻하지 않는다.

추가 확정 원인은 `CWorldSequencePlayer::Prepare_AreaLoad`의 무조건적인 Deploy catalog
로드다. SL00은 `.deployassets/.deployplacements`가 모두 없는 정상 optional Area인데 이 검사가
실패 stage를 저장하여 ClassSelectionPresentation.Initialize까지 실패시켰다. 두 파일 모두
없을 때에만 빈 Deploy 대상으로 처리하고 한쪽 누락·손상·존재하지 않는 Deploy binding은
계속 실패시키도록 수정했다. Level 배경/manifest 실패도 Workbench와 F1에 직접 전달한다.

수정 파일의 최소 컴파일은 통과했다. 후보 EXE 빌드와 실제 ClassSelectionPresentation
Initialize/Play 연결 검사는 진행 중이며, 이 시점에는 사용자 재생 성공을 기록하지 않는다.

## G11. 2026-09-24 F1 Character Select Movie 진입

F1의 `Character Select Movie` 구역에서 Guardian Knight를 기본 선택하고 기존 7 class를 선택할 수 있도록 `CLevel_CharacterSelect::Render_ClassSelectMovieControls`를 추가했다. 현재 Level의 동일 presentation owner에 Play/Restart Intro·Stop을 전달하고 Intro/Loop, 일시정지 여부, 현재 시간/길이와 기존 준비 실패 상태를 표시한다. MainApp 공통 F1의 DefaultOpen `Character Select Movie` 섹션에서 이 함수를 호출하는 연결도 확인했다.

Character Select 밖에서는 `Enter Character Select to preview this movie`를 표시하고 player나 리소스를 생성하지 않는다. 기존 Server 승인·customizing·create character·raid-entry preview·Level 전환 조건을 그대로 검사한다. class combo는 movie 대상 선택일 뿐 Server class-change를 요청하지 않는다. 다른 UI가 시작한 현재 연출도 명시적인 Stop으로 동일 owner에서 종료할 수 있으며 기존 camera·배우·FX 정리를 사용한다. 기존 음향·미복원 영역을 이번 UI 추가로 복원했다고 기록하지 않는다.

현재 설치 manifest에는 GUARDIANKNIGHT 한 scene만 있다. 나머지 class는 준비된 movie가 없다고 표시하고 Play를 비활성화한다. Guardian intro는 wall 31,735.171ms/source31,505ms, loop는23,003ms이며 각 phase22개 instance와 intro5/loop4 camera cut을 사용한다. 신규 미디어나 Resources·Data·게시 파일 변경은 없다.

읽기 전용 설치 참조 검사는 PASS다. manifest, source/runtime WorldSequence, EffectCatalog,4개 effect문서, SL10 material문서의9개 JSON을 중복 key·비유한 값 거부 파서로 읽었다.44개 instance의 enabled·template duration·binding을 대조하고22 actor resource,25 model/donor 경로,54 FX 참조,679 배경 참조의 합집합758파일이 모두 존재하며 비어 있지 않음을 확인했다. source/runtime WorldSequence JSON 객체도 일치한다. 증거는 `out/CharacterSelectMovie20260924/installed-resource-check.json`이다. 이 검사는 현재 파일 존재·참조 정합성 검사이며 CModel·GPU·영상 재생 성공을 의미하지 않는다.

Level cpp/header의 UTF-8 BOM 없음과 CRLF를 유지했고 담당 `git diff --check`는 PASS다. 기존 두 파일만 확장하여 project/filter 추가는 없다. root가 수행한 최종 Debug/Release Product 빌드는 모두 PASS이며 두 receipt의 skippedBuild=false도 확인했다. 증거는 `out/BuildPipeline/runs/20260923T234847724Z-debug-product.json`, `20260923T234951599Z-release-product.json`다. Client·영상·UI 실행과 프로세스 종료는 수행하지 않았다.
