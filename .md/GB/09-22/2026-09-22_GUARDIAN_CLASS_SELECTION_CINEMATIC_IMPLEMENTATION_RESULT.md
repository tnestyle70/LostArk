# 가디언나이트 클래스 선택 연출 조사·구현 결과

## G00. 현재 반영 상태

현재 F1 `Character Select Movie`와 Action Composition의 `WORLD → Character Select`는
같은 11개 바닥 category 선택을 사용한다. 기본 선택은 현재 입장 class에 대응하는 category이며,
Play는 그때 선택한 category의 movie를 요청한다. Server의 실제 player class가 Guardian Knight일
필요가 없고, movie 선택이나 Play가 Server class를 변경하지 않는다. 현재 설치된 movie는
Guardian Knight 하나이며 `10 Dragon Human`에서 선택한다.

이번 Debug Product 빌드와 실제 SL00·SL10 모델/배치 stage 검사는 통과했다. 실제 cinematic
소비자도 Initialize → Play → 90초 Update → Loop 2회 경계 → Stop → 재Play를 통과했다.
별도 선택·WORLD transport 검사는 실제 함수와 대역을 사용했다. **창 없는 native 실행 검증이며
Client/UI·GPU draw·사용자의 화면 확인은 수행하지 않았다.** 네 class의 추가 원본 설치는 별도 작업이다.
moving PSC와 색상 입력의 실제 소비자 검증·완료 경계는 G15/G16을 따른다.
현재 선택 정책과 사용 경로는
[G13](#g13-2026-09-25-선택-category-play와-world-연결), 추가한 scene별 배경 실패 격리와 최신 대역 검사 결과는 [G14](#g14-2026-09-25-scene별-배경-준비와-실패-격리), moving PSC·음성 Pause·조명 반경과 실제 native 증거는 [G15](#g15-2026-09-25-moving-psc음성-pause조명-반경-연결)다. 이전 검사를 연출 전체의 재생 가능
판정으로 사용했던 결론의 철회는 유지하며, G11/G12의 기본 Guardian·미준비 Play 비활성 안내는
G13의 선택 category 정책으로 대체한다.

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

아래는 당시 구현·확인 기록이다. 기본 Guardian 선택, 7-class combo와 미준비 Play 비활성 조건은
[G13](#g13-2026-09-25-선택-category-play와-world-연결)의 11-category 선택·요청 검증 정책으로 대체됐다.

F1의 `Character Select Movie` 구역에서 Guardian Knight를 기본 선택하고 기존 7 class를 선택할 수 있도록 `CLevel_CharacterSelect::Render_ClassSelectMovieControls`를 추가했다. 현재 Level의 동일 presentation owner에 Play/Restart Intro·Stop을 전달하고 Intro/Loop, 일시정지 여부, 현재 시간/길이와 기존 준비 실패 상태를 표시한다. MainApp 공통 F1의 DefaultOpen `Character Select Movie` 섹션에서 이 함수를 호출하는 연결도 확인했다.

Character Select 밖에서는 `Enter Character Select to preview this movie`를 표시하고 player나 리소스를 생성하지 않는다. 기존 Server 승인·customizing·create character·raid-entry preview·Level 전환 조건을 그대로 검사한다. class combo는 movie 대상 선택일 뿐 Server class-change를 요청하지 않는다. 다른 UI가 시작한 현재 연출도 명시적인 Stop으로 동일 owner에서 종료할 수 있으며 기존 camera·배우·FX 정리를 사용한다. 기존 음향·미복원 영역을 이번 UI 추가로 복원했다고 기록하지 않는다.

현재 설치 manifest에는 GUARDIANKNIGHT 한 scene만 있다. 나머지 class는 준비된 movie가 없다고 표시하고 Play를 비활성화한다. Guardian intro는 wall 31,735.171ms/source31,505ms, loop는23,003ms이며 각 phase22개 instance와 intro5/loop4 camera cut을 사용한다. 신규 미디어나 Resources·Data·게시 파일 변경은 없다.

읽기 전용 설치 참조 검사는 PASS다. manifest, source/runtime WorldSequence, EffectCatalog,4개 effect문서, SL10 material문서의9개 JSON을 중복 key·비유한 값 거부 파서로 읽었다.44개 instance의 enabled·template duration·binding을 대조하고22 actor resource,25 model/donor 경로,54 FX 참조,679 배경 참조의 합집합758파일이 모두 존재하며 비어 있지 않음을 확인했다. source/runtime WorldSequence JSON 객체도 일치한다. 증거는 `out/CharacterSelectMovie20260924/installed-resource-check.json`이다. 이 검사는 현재 파일 존재·참조 정합성 검사이며 CModel·GPU·영상 재생 성공을 의미하지 않는다.

Level cpp/header의 UTF-8 BOM 없음과 CRLF를 유지했고 담당 `git diff --check`는 PASS다. 기존 두 파일만 확장하여 project/filter 추가는 없다. root가 수행한 최종 Debug/Release Product 빌드는 모두 PASS이며 두 receipt의 skippedBuild=false도 확인했다. 증거는 `out/BuildPipeline/runs/20260923T234847724Z-debug-product.json`, `20260923T234951599Z-release-product.json`다. Client·영상·UI 실행과 프로세스 종료는 수행하지 않았다.

## G12. 2026-09-24 현재 관람 경로와 Stop·재Play 확인

아래는 당시 구현·확인 기록이다. 기본 Guardian 선택, 7-class combo와 미준비 Play 비활성 조건은
[G13](#g13-2026-09-25-선택-category-play와-world-연결)의 11-category 선택·요청 검증 정책으로 대체됐다.

이번 확인은 현재 디스크와 현재 소스에 대한 검사다. G04/G08~G11의 과거 WARP·Product 빌드
결과를 이번 PC의 실제 화면 재생 성공으로 재사용하지 않는다. Client/UI를 실행하거나
사용자 프로세스를 종료하지 않았고, 컷신 원본·게시 데이터도 변경하지 않았다.

### 현재 파일과 초기화 조건

`python out/GuardianMovieValidation20260924/check_installed.py`는 exit 0, 실패 0건이다.
동일 폴더의 `installed-resource-check.json`에 검사 당시 문서 SHA256과 수량을 기록했다.

| 검사 대상 | 현재 결과 |
|---|---|
| Guardian manifest / WorldSequence | Intro·Loop 각각 22 instance, 총 44개, 배우 resource 22개. 원본과 게시 WorldSequence JSON 동일 |
| 카메라·시간 | Intro 5컷, Loop 4컷이 각 phase 전체를 덮음. Intro wall 31.735171초, Loop 23.003초 |
| Effect | 4개 문서·25 emitter를 공식 validator의 관련 material/module/attachment/native sprite/v15/resource 검사로 검증 |
| SL10 배경 | catalog 574개, 배치 1,356개, material 963개, lighting binding 1,349개. 원본 catalog v4→게시 v5 헤더 변환을 제외한 행 일치 |
| 설치 참조 | 배우 model/donor/texture, FX, 배경 합집합 848파일이 존재하며 0 byte 파일 없음 |

848파일 검사는 CModel 생성·GPU draw·화면 재생 성공을 뜻하지 않는다. 실제 Play 활성화는
`CClassSelectionPresentation::Initialize`가 SL10 배경 준비 이후 실행되고, SL00의 staged
WorldSequence 소비·44 instance 계약·prototype 등록·사전 object clone 준비까지 성공해야 한다.
그 뒤에만 scene이 등록되고 `Class selection cinematics ready.`가 표시된다. F1은 이 등록
결과와 Server Arena 상태, 생성/커스터마이징/레이드 입장창 및 Level 전환 여부를 검사한다.

별도 진입 제약도 발견했다. 검사 시 `Shared/Public/GameplayDataRevision.h`의 기대 버전은
37인 반면 `Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap` 헤더는 36/40699다.
현재 소스로 빌드한 Server는 버전 36을 `Gameplay bootstrap header is invalid`로 거부한다.
진행 중인 Madness 원본 반영과 Gameplay Publish가 아직 남아 있으므로, 새 빌드 기준의
Character Select 실제 입장 완료를 선언하지 않는다. 실행 파일의 시각만으로 그 바이너리에
포함된 버전을 확정하지도 않는다. 이 문제는 Guardian의 WorldSequence 재게시와 별개다.

후속 복구: 사용자의 반영 요청 후 Madness 저장본 병합과 공식 Gameplay Publish를 완료해
게시본이 v37/40699행으로 갱신됐다. 같은 Debug Server.exe의 실제 headless 초기화·Listening·
자동 종료와 Kouku draft 계약 29항목이 exit 0으로 통과했다. 위 버전 불일치 진입 제약은
해소됐으며, 자세한 반영·백업·검증은
[Madness 결과 G06](../09-24/2026-09-24_KOUKU_MADNESS_TUNING_RESULT.md#g06-승인-후-v37-게시와-실제-server-초기화-복구)을 따른다.
이 검사는 Guardian의 실제 Client 화면 재생 판정을 대신하지 않는다.

### 현재 재생 수명 검사

`out/CharacterSelectMovie20260924/native-lifecycle/build_lifecycle.py`로 임시 console fixture를
빌드·실행했고 48항목, 실패 0건, exit 0이다. `run.log`와 `source-manifest.json`에 실제 실행
기록과 함수별 원본 hash를 남겼으며, root가 실행 후 11개 원본 파일의 hash 일치를 재확인했다.
제품 C++ 수정이나 제품 EXE 빌드는 이 확인에서 필요하지 않았다.

실제 Guardian JSON과 현재 production의 Play/Start_Phase/Update/Stop/Fail, camera/clock/FX/
material/light sampling 본문 및 Engine 카메라 Begin/ApplyWithUp/End 본문을 실행했다.
첫 Play와 첫 Update의 정확한 0초, Intro→Loop, 반복 시 actor player 유지, Restart의 0초,
Stop의 상태·FX·대기 중 light payload 정리, 이전 camera world/FOV의 정확한 복원,
재Play의 새 camera snapshot, 경쟁 camera owner 거부, 6종 시작 실패와 Loop 전환/FX 실패,
NaN 시간 입력 뒤 정리·재시도, 실패한 staged Restart의 기존 재생 보존을 검사했다.

WorldSequence 객체 생성, material 설정, FX handle 생성, light 제출, render pipeline sink는
실패 주입 가능한 테스트 대역이다. `Initialize`, 실제 CModel/WorldSequence 배우 초기화,
CCamera_Free의 Follow 갱신, Server 접속, GPU draw, 제품 UI는 이 fixture에서 실행하지 않았다.
따라서 이는 현재 production 재생 제어 함수의 검사이며 실제 화면 재생 완료 판정이 아니다.
Follow 복귀와 HUD·입력·SL10 가시성 복구는 현재 Level/Camera/UI 소비자 코드를 별도로 추적했다.

### 직접 확인할 순서

1. 해당 빌드와 Gameplay 게시 버전이 맞고 팀 Server에 연결되는 상태에서 Lobby의
   `Character Select`로 진입한다. Server 승인 캐릭터가 나타나고 이동 가능한 상태까지 기다린다.
2. 플레이어를 따라가는 Follow 카메라에서 F1 → `Character Select Movie` → `Guardian Knight`를
   선택한다. `Class selection cinematics ready.`와 활성화된 Play가 준비 완료의 확인점이다.
   movie 선택은 Server의 플레이어 class를 변경하지 않는다.
3. Play를 누르면 `Intro`와 0초부터 시작하는 시간이 표시된다. F1을 닫아도 연출은 계속된다.
   11개 발판 중 하나를 추적하는 동작이 아니라 SL10의 저장된 WORLD 카메라와 연출로 전환한다.
   약 31.735초 뒤에는 `Loop`로 바뀌고 23.003초 구간을 반복한다. 자동 플레이어 복귀는 없다.
4. F1의 Stop을 누른다. `Class selection cinematic stopped.`가 나오고 버튼이 Play로 돌아온다.
   Stop은 배우·FX·조명과 재생 상태를 정리하고 저장한 카메라/FOV를 복구한다. 다음 Update에서
   SL10을 숨기고 HUD·입력을 복구하며, Follow 카메라는 현재 플레이어 위치를 다시 따라간다.
5. 다시 Play를 누르면 Intro 0초부터 시작한다. Stop은 준비한 scene와 object pool을 보존하므로
   정상 반복 관람에 Reload·Publish·재입장은 필요 없다. 재생 중 Restart Intro도 처음부터 시작한다.

Free 카메라에서 시작했다면 Stop은 Free 상태로 복귀한다. F6는 연출 중지와 Follow/Free 전환을
함께 수행할 수 있으므로 위 복귀 확인에는 F1 Stop을 사용한다. 재생 불가 시 F1의 준비 실패
문구를 확인한다. 배경 실패, WorldSequence 준비 실패, 다른 presentation의 카메라 점유는
각각 별도 상태로 전달된다. 직접 화면에서의 모델·이펙트 외형과 정상 복귀 확인은 아직 남아 있다.

## G13. 2026-09-25 선택 category Play와 WORLD 연결

### 현재 선택과 재생 계약

G11/G12의 고정 Guardian 기본 선택과 미준비 movie의 Play 비활성 처리를 교체했다.
`Data/Rendering/Authored/CharacterSelectFloorSwap.json`의 11개 stable category ID·label·원본
바닥 placement ID를 읽고, 현재 SL00의 해당 배치가 하나씩 존재하는지 확인한 뒤 Level의
movie option으로 commit한다. 초기 선택은 현재 입장 class의 category다. 제품 category 클릭,
F1 combo와 Action Composition WORLD가 같은 Level 선택을 갱신하며 Server 승인 player class와
독립적이다. 제품의 subclass thumbnail이 보내는 기존 typed class-change는 별도 계약으로 유지한다.

| 바닥 category | 연결 class ID | 현재 movie |
|---|---|---|
| `01 Hunter` / `class.hunter` | 미연결 | 없음 |
| `02 Warrior` / `class.warrior` | `WARLORD` | 없음 |
| `03 Magician` / `class.magician` | 미연결 | 없음 |
| `04 Delain` / `class.delain` | 미연결 | 없음 |
| `05 Hunter Female` / `class.hunter_female` | `GUNSLINGER` | 없음 |
| `06 Specialist` / `class.specialist` | `ARTIST` | 없음 |
| `07 Warrior Female` / `class.warrior_female` | `SLAYER` | 없음 |
| `08 Fighter` / `class.fighter` | `LANCE_MASTER` | 없음 |
| `09 Fighter Male` / `class.fighter_male` | 미연결 | 없음 |
| `10 Dragon Human` / `class.dragon_human` | `GUARDIANKNIGHT` | Intro + Loop 연결 |
| `11 Specialist Male` / `class.specialist_male` | `DIMENSIONMASTER` | 없음 |

7 category는 playable class ID에 연결되고 나머지 4 category는 명시적인 미연결 상태다.
현재 manifest에는 Guardian movie 하나만 있다. 다른 category의 Play가 Guardian을 대신
재생하지 않으며 선택 category·class와 준비 실패 이유를 반환한다. Play 버튼은 기존 Server
Arena·customizing·캐릭터 생성창·레이드 preview·Level 전환 gate로만 제한한다. 버튼을 누를 수
있다는 사실과 배경·scene admission 성공은 구분하며, 배경과 presentation Initialize 실패는
준비 실패 상태에 보존한다. 준비 실패를 ready로 바꾸거나 prerequisite를 제거하지 않았다.

F1과 WORLD의 Play는 `CLevel_CharacterSelect::Play_ClassCinematic`을 사용한다. WORLD의
timeline·Pause·Stop·Seek는 현재 active movie를 제어하고, 다음 Play 대상으로 고른 category와
active movie의 ID·phase 길이를 구분한다. 같은 movie 재시작에만 Restart 문구를 표시한다.
미연결 movie 요청과 실패한 재생 요청은 이미 재생 중인 다른 movie를 중지하지 않는다.
제품 category의 기존 자동 재생/중지 동작은 준비된 movie 경로에서 유지한다.

이번 위치 방식은 원본 전체 WORLD 무대다. 이동해 둔 11개 SL00 바닥은 category 선택 근거와
현재 배치를 보존하고, Guardian의 SL10 배경·배우·카메라·FX는 함께 기존 원본 WORLD 좌표를
사용한다. 바닥에 맞춰 일부 actor나 camera만 따로 offset하지 않았고 Server player 위치도
변경하지 않았다. 이번 변경에서 원본 movie 데이터나 Resources를 새로 생성·교체하지 않았다.

### 이번 실행 증거와 검사 범위

| 검증 | 결과 | 실제 실행 범위와 남은 경계 |
|---|---|---|
| Level 선택·실패 처리 | 198/198 PASS (G14 반영 후 재실행) | 현재 Level 함수 본문과 실제 `CDataJson`을 추출·컴파일했다. 실제 11-category JSON과 게시 SL00 804 placement의 ID를 사용했다. map/presentation은 대역이며 실제 Level Initialize, Server, Client/UI/GPU는 실행하지 않았다. |
| WORLD 선택·transport | 27/27 PASS (G14 반영 후 재실행) | 현재 MainApp callbacks·workbench session 본문과 Level 함수 본문을 연결했다. 미지원 선택 뒤 active Guardian 보존, active ID/길이의 Seek, 다른 admitted class, F1/WORLD token ownership을 검사했다. ImGui·scene·map·Level host는 대역이며 실제 제품 UI/GPU 재생은 실행하지 않았다. |
| 실제 SL00 map stage | PASS | 필수 모델 174개, 배치 804개, static batch 객체 213개, 개별 fallback 객체 4개 |
| 실제 SL10 map stage | PASS | 필수 모델 574개, 배치 1,356개, static batch 객체 400개, 개별 fallback 객체 302개 |
| Debug Product Build | PASS, `skippedBuild=false` | 이번 소스의 정식 Debug Product 빌드. 이번 변경의 Release 빌드는 미실행 |

Level 증거는 `out/SelectedClassMovie20260925/result.json`과 `source-manifest.json`,
WORLD 증거는 그 아래 `world_transport/result.json`과 `source-manifest.json`이다.
두 fixture의 성공을 실제 모델 admission이나 draw 성공으로 기록하지 않는다.

실제 map 증거는 `out/MovieMapFailure20260924/result.json`이다. Loader와 같이 COM을
초기화한 창 없는 D3D11 WARP 검사에서 설치된 Engine의 CModel/Material/Shader factory와
현재 MapPlacementRuntime 객체를 사용했다. 같은 Level index에 SL00 다음 SL10 prototype을
등록하고 두 Area를 함께 유지한 stage가 exit 0으로 통과했다. 이 검사는 Loader의 병렬 배치와
optional failure cache, `CClassSelectionPresentation::Initialize/Play`, WorldSequence/FX/camera,
GPU draw·Client/UI·Server 연결을 실행하지 않았다. 초기 probe의 COM 초기화 누락으로 발생한
14개 PNG 로드 실패는 probe 설정을 맞춘 뒤 해소됐으며 제품 결함으로 기록하지 않는다.
현재 데이터에서 map stage가 통과했다는 근거만으로 이전 screenshot의 일반 rollback 문구를
어느 historical asset의 실패로 확정하지 않는다.

정식 Debug 빌드 증거는
`out/BuildPipeline/runs/20260924T212317091Z-debug-product.json`이며 `result=PASS`,
`configuration=Debug`, `skippedBuild=false`를 확인했다. 이어서 G14의 실제 전체 cinematic
소비자 검사까지 완료했다. 창 없는 실행 검증과 사용자 화면 확인은 구분한다. 원작의 동적 소품18개, 일부 배경 입력, shake/DOF/Bloom·음향과
사용자의 최종 visual 판정은 별도로 남아 있다.

### 사용자가 직접 확인할 경로

1. 팀 Server에 승인된 Character Select로 진입한다. 실제 player class는 어느 지원 class여도 된다.
2. F1 → `Character Select Movie` → Category `10 Dragon Human` → `Play`, 또는
   Action Composition → `WORLD` → `Character Select` → Category `10 Dragon Human` → `Play`를 누른다.
3. Guardian 원본 SL10 무대의 Intro와 이후 Loop, Stop 뒤 기존 camera 복귀를 직접 확인한다.
   다른 category를 선택하면 그 class의 movie만 요청하며 미연결 상태는 해당 선택의 이유로 표시한다.

Client/UI 자동 실행·조작·캡처나 사용자 프로세스 종료는 수행하지 않았다.

## G14. 2026-09-25 scene별 배경 준비와 실패 격리

Level은 단일 SL10 runtime을 scene별 고유 Area runtime 목록으로 교체했다.
`Load_BackgroundAreas`가 전체 manifest를 검증해 반환한 Area를 동일
`PresentationMapLoadScope`로 준비한다. scene의 optional `backgroundAreaId`가 없으면
기존 registry fallback을 사용하며 현재 Guardian은 계속 SL10에 연결된다. 현재 읽은 manifest는
Guardian 한 scene이고, 다른 class의 데이터·리소스가 설치됐다고 기록하지 않는다.

각 Area는 기존 CMapPlacementRuntime owner와 그 Area의 실패 문자열·가시성만 소유한다.
한 Area의 Load_Area 실패는 common preparation failure에 기록하지 않고 나머지 Area를
계속 준비한다. 공통 manifest 검증이 성공하면 개별 배경 실패와 독립적으로 presentation을
Initialize한다. manifest·scene·presentation 공통 초기화 실패는 기존 전역 실패로 유지한다.
Play는 선택 class가 admitted됐는지 확인한 다음 그 class의 resolved Area 준비만 검사한다.
따라서 다른 class 배경이 없다는 이유로 정상 Guardian 배경의 Play를 차단하지 않는다.

성공한 Play에서는 활성 scene의 배경만 표시한다. 다른 Area의 scene으로 바뀌면 이전 배경을
숨기고 새 배경을 표시하며 같은 Area를 쓰는 class는 runtime을 공유한다. Stop 뒤 다음 Level
Update는 모든 cinematic 배경을 숨긴다. Level 종료는 presentation owner를 먼저 Clear한 뒤
각 배경 runtime을 정리한다. 배경 Area가 primary SL00와 같으면 기존 m_MapRuntime을
그대로 사용하고 중복 로드·배치 clone·cinematic suppression 대상에 포함하지 않는다.
primary gameplay map은 Stop 때도 표시 상태를 유지한다.

추가된 실제 Level 함수 본문을 포함해 `build_selection.py`를 다시 컴파일·실행한 결과
198/198, WORLD `build_world_transport.py`는 27/27, 모두 실패 0건이다. 최신 결과는 각각
`out/SelectedClassMovie20260925/result.json`,
`out/SelectedClassMovie20260925/world_transport/result.json`이다. G13의 173/26 당시 검사에
배경 scope 검사를 추가했으며 위 결과표와 같은 경로의 source manifest도 최신 실행으로 갱신했다.

이번 추가 검사는 optional fallback, 실패한 다른 Area와 정상 Guardian의 공존, 선택한 Area의
정확한 실패 이유, 실패 요청 시 기존 재생·배경 보존, 새 Area/같은 Area 전환, SL00 공유와
Stop 가시성, global manifest 실패 보존을 확인했다. production의 Level 함수 본문과 실제
CDataJson은 실행했지만 `Load_BackgroundAreas`의 manifest 파서, map Load_Area·scene playback·
visibility 대상은 대역이다. 따라서 실제 새 Area의 모델·배치 admission, 전체 연출 Initialize/Play,
GPU draw와 사용자 화면 판정의 성공 근거로 쓰지 않는다.

CPP/H의 UTF-8 BOM 없음·CRLF와 담당 `git diff --check`를 확인했다. G14를 포함한 정식
Debug Product Build는 PASS(`skippedBuild=false`)다. 증거는
`out/BuildPipeline/runs/20260924T213517310Z-debug-product.json`이며 이번 Release 빌드는 미실행이다.

추가한 parser/metadata 소비자의 실제 검사는 30건, 실패 0건이다. Guardian의 필드 생략,
SL01/03/08/12 명시, stable-order 중복 제거, fallback 공유, 잘못된 타입·경로·길이 거부와
늦은 실패 시 기존 출력 보존을 확인했다. 입력 후보는 out에만 생성했고 제품 데이터를 바꾸지 않았다.
증거는 `out/ClassSelectionNativePlay20260925/background_result.log`다.

같은 최신 Debug OBJ로 actual native cinematic probe를 다시 연결해 Initialize ready → Play →
360회×0.25초(90초) Update → Intro에서 Loop 전환·2회 loop 경계 → Stop → 재Play를 통과했다.
두 번째 Play의 0초와 새 token, 카메라 소유권 해제, 실제 배우 44개·최대 visible 21개와
7,560개 visible actor transform의 finite 값을 검사했다. 실제 WorldSequence 44개 자원,
Effect worker GPU 준비 4개 모두 실패 0건이다. CClassSelectionPresentation, WorldSequence,
CWorldSequenceObject, CModel/CMaterial, CCamera_Free, Effect service는 대역 없이 실행했다.
증거는 `out/ClassSelectionNativePlay20260925/result.json`, `run.log`, `object-manifest.json`이며
링크한 OBJ 335개가 그 시점 Product OBJ와 전부 동일했다.

이 fixture의 Map 대상은 실제 catalog/placement 메타데이터다. 실제 Map 객체 stage는 G13의
별도 소비자 검사가 담당한다. Window·Client·UI·draw·Server 접속은 실행하지 않았으므로
GPU 외형과 사용자 화면 판정은 남아 있다. 새 네 Area의 데이터 설치·생성 성공도 이번
Guardian 검사로 대신하지 않는다.

Loader는 고유 background Area에 Ready_MapArea만 실행하고 primary SL00 WorldSequence 준비는
한 번 유지한다. 준비 시작 시 해당 Area의 이전 load cache를 제거하여 optional 준비 실패 뒤
이전 방문의 catalog/placement를 소비하지 않게 했다. 전체 Level prototype을 삭제하지 않으며
실패한 배경의 asset/source placement와 stage 이유는 선택 class의 Play 오류에 전달한다.

## G15. 2026-09-25 moving PSC·음성 Pause·조명 반경 연결

선택 정책과 원본 WORLD 무대는 유지하고 추가 class에서 필요한 소비자를 연결했다.
`effects[].rootKeys`는 optional phase source-time WORLD meter TRS이며 position/scale은 선형,
정규화 quaternion은 shortest slerp다. 2~65,536개 키가 0부터 phase.durationMs를 덮어야 하며
비유한 값·퇴화 quaternion/scale·축 scale의 0 통과를 거부한다. 생략하면 기존 rootWorld다.
`loopAgeDeltaMs`는 생략 시 기존 clock span, 양수는 span과 0.001ms 이내 일치해야 한다.
0과 일정 age는 held particles, 0과 증가 clock은 loop마다 history를 다시 만드는 epoch다.
원본 phase에 39 PSC가 있어 occurrence 상한을32에서128로 조정했다.

과거 fixed-step 입자 생성 위치는 같은 PSC의 intro/loop clock을 역으로 조회한다. plateau에서는
age에 처음 도달한 위치를 사용하고, 현재 표시 위치는 별도로 현재 phase root를 전달한다.
기존 `Seek_WorldRoot`의 마지막 optional root는 pending/active에 값으로 복사되고 history commit
성공 뒤 `Set_RootWorld`의 Update(0) frame 갱신으로 적용된다. 기존 caller는 옵션을 생략하여
동작을 유지하며 새 handle 재생기를 만들지 않았다. source anchor 자체가 필요한 Effect는
rootKeys만으로 복원됐다고 간주하지 않는다.

음성은 기존 WorldSequence template.soundTracks의 한 carrier instance가 소유한다. staged player는
첫 Play 전에 pause하여 실패 또는 scrub 준비 중0초 cue가 새어나가지 않게 했다. 성공 commit 뒤
Play는 재생, Seek는 pause를 적용하고 Set_Paused가 실제 WORLD sound handle까지 전달된다.
비선형 phase clock에서도 PCM rate는 기존 instance rate이고 phase 전환 때 남은 WAV tail을
자른다. 이 경계를 넘어서는 원본 음성 복원·청취는 별도다. 제품 WAV를 이번 코드 작업에서
새로 설치하거나 실제 오디오 장치를 실행하지 않았다.

`lights[].keys[].radiusMeters`는 optional이며 생략 시 track.radiusMeters를 상속한다.
finite positive 값으로 기존 반경 상한을 적용하고 brightness/color와 같은 fraction으로
선형 보간하여 기존 transient point-light fRange에 전달한다. 다른 rendering option은 바꾸지 않았다.

### 검사 증거

- 정규 Debug Product Build는 PASS, skippedBuild=false다.
  `out/BuildPipeline/runs/20260924T214936812Z-debug-product.json`과
  `out/CharacterMoviePlayFix20260924/product-moving-root-debug-20260925.log`를 기록했다.
- 실제 ClassSelection 함수 본문을 추출한 lifecycle fixture는104 checks, 실패0이다.
  TRS/plateau/intro history/정확한 loop 경계/reset epoch/held root, staged audio pause와
  실패 시 기존 재생 보존, light radius를 포함한다. WorldSequence·Effect sink 등은 대역이다.
  증거는 `out/ClassMovieRootKeys20260925/native-lifecycle/run.log`다.
- 실제 Seek_WorldRoot/Commit_ExternalTransformHistorySample/affine 검사 본문을 실행한
  service fixture는14 checks, 실패0이다. pending/active 값복사, null 기본호출 유지,
  commit 실패 시 final root 미적용을 확인했다. 이 fixture의 EffectObject는 대역이다.
  증거는 같은 폴더의 `service-run.log`다.
- 최신 Product OBJ로 원본 Guardian 실제 native 경로를 재실행했다. 이번에는 이전 검사의
  Spawn/Seek queue·Commit뿐 아니라 EffectService.Update의 최종 history 소비까지 실행했다.
  90초(360×0.25초), 배우44개와 visible finite샘플7,560개, 최종 Effect frame1,370회,
  peak active5개, finite particle WORLD 샘플122,294개, GPU occurrence descriptor9,225개를
  확인하고 Intro→Loop2회 경계→Stop→재Play가 통과했다. GPU descriptor 생성과 draw는 구분한다.
- out에만 만든 moving PSC8370 후보는 실제 설치 Effect asset/clock을 유지했다. 같은 EffectObject로
  90초를 진행하고 최종 root347회(오차0.002m), loop held age0.262369156초를238회 비교했다.
  Pause, loop1000/18000ms Seek, intro5000ms Seek, Stop layer 제거와 재Play도 통과했다.
  제품 Data/Resources는 변경하지 않았다. 증거는
  `out/ClassSelectionNativePlay20260925`의 최신 결과와 실행 로그를 따른다.
- 실제 parser 추가13개 사례는39개 Effect 허용/129개 거부, root endpoint/time/quaternion/scale와
  scale sign crossing 거부, loop delta 불일치 거부와0 허용, radius 상속/명시/음수 거부 및
  실패 output 보존을 확인했다. 같은 폴더의 result.json에 기록했다.

창·Client/UI·GPU draw·실제 오디오 출력은 실행하지 않았다. 위 검사는 새 네 class의 모델·Effect
리소스 설치와 실제 최종 화면 판정을 대체하지 않는다. Fighter의 가변 particle color/alpha는
원본 module/property binding을 확인해 G16의 typed fixed-step 입력으로 연결했다.
기존 distribution.keys에 phase 곡선을 단순 복사하거나 material tint로 대신하지 않는다.


## G16. 2026-09-25 PSC별 가변 color·alpha 소비자

원본 Fighter binding43행은 color35·alpha8이며 실제 대상은8 system의38 emitter LOD다.
Sprite29·Mesh9이고 Trail/Beam/Decal은 없다. source row와 CDO 근거는
`out/ClassMovies20260925/lance-warlord/fighter-parameter-bindings.json`,
`fighter-parameter-renderers.json`, `particle-parameter-cdo.json`에 있다. mode가 생략된5행은
CDO의 NORMAL을 따른다. DIRECT로 임의 분류했던 초안은 실제 mode 근거로 교정했다.

ClassSelection `effects[].parameterTracks`는 parameterName, componentCount1/3과 keys를 읽는다.
각 key는 소수 timeMs와 같은 성분 수의 value/arriveTangent/leaveTangent, 명시적
CONSTANT/LINEAR/CUBIC을 갖는다.1개 상수 key, 구간 밖 hold와 원본 value/sec tangent를
지원한다. curve를 먼저 보간하고 Effect distribution mapping을 뒤에 적용한다.

source distribution의 JSON binding은 기존 naming convention에 맞는 `worldSample`이며
C++ enum만 WORLD_SAMPLE이다. `parameterMapping`의 modes는 DIRECT/NORMAL, 입력/출력
최소·최대 배열은 componentCount와 일치한다. source constant와 baked payload 충돌,
누락된 이름·타입·mode와 ABS는 명시 실패한다. 기존 none/actionCue는 유지한다.
PSC별 variant Effect만 dynamic binding을 갖고 모든 phase의 raw input을 공급하는 데이터
계약이다. 같은 source system을 쓰는 다른 PSC까지 가변 입력을 강제하지 않는다.

기존 fixed-step sample의 ParticleParameters는 instance별 상태다. history의 root와 raw input은
같은 intro/loop phase 역조회를 사용한다. 전체 sample 검증 뒤 Step에 입력을 설치하며 현재
표시 입력은 별도로 검증해 history commit 뒤 전달한다. held-age color는 마지막 simulation
step의 spawn base와 color module 순서를 보존하여 frame에 투영한다. shared definition은
변경하지 않고 기존 world-root handle·Stop·Seek·loop lifecycle을 사용한다.

현재 코드/독립검증과 제품 검증을 구분한다. helper·실제 codec read/write·DataJson을 사용한
최종128개 검사는 전부 통과했다(`out/WorldParameterCodec20260925/result.json`). legacy codec,
HDR/alpha>1 보존, DIRECT/NORMAL 순서, 이름 중복·타입·mapping·비유한 값·충돌 거부와
실패 output 보존을 포함한다. 이 결과만으로 새 네 class 리소스 설치나 화면 복원을 완료로
판정하지 않는다. 최신 Product·실제 Playback 소비자 검증 결과는 아래에 추가한다.


정규 Debug Product는 PASS, skippedBuild=false다.
`out/BuildPipeline/runs/20260924T221039597Z-debug-product.json`과
`out/CharacterMoviePlayFix20260924/product-particle-parameters-debug-20260925.log`를 기록했다.
Client OBJ157개와 실행 파일을 갱신했고 C++ source encoding과 scoped git diff --check를 확인했다.
빌드가 실행 데이터 publish나 화면·음향 확인을 수행한 것으로 간주하지 않는다.


실제 함수 본문을 추출한 계약 검사는 기존 baseline을 보존하며 확장했다.
`out/ClassMovieParameters20260925/native-contracts/run.log`의 lifecycle/curve/parser134개와
`service-run.log`의 Service/Object24개가 통과했다. cubic seconds, CONSTANT exact key,
구간 밖 hold, raw 보간 뒤 NORMAL, intro/loop shared history, parser 실패 out 보존,
final 값 복사와 historical mutation 전 검증을 포함한다. 이 검사에서는 WorldSequence/Playback
주변 의존성을 대역으로 사용했으며 실제 particle 최종 소비자 검사는 별도다.


Playback 실제 함수 본문 fixture22개도 통과했다.
`out/ClassMovieParticleParameters20260925/native/run.log`는 fixed-step 사전검증, 매Step 입력 설치,
held-age frame color의 spawn base/원본 module 순서·난수 보존, instance 분리, unsupported carrier
명시실패와 Reset을 확인한다. 마지막 검토에서 WORLD_SAMPLE 문서 뒤 reconstructed program으로
같은 Playback을 재사용하는 경로의 별도 clear가 필요함을 확인했다. 해당 stage의 성공 commit에서
input·resolved value·distribution pointer·contract error를 함께 비우고 실제 함수 재사용 fixture로
검증했다. 이4줄을 포함한 최종 증분 Debug Product도 PASS이며
`out/BuildPipeline/runs/20260924T221322781Z-debug-product.json`과
`out/CharacterMoviePlayFix20260924/product-particle-reset-debug-20260925.log`에 기록했다.
OBJ1개 재컴파일 및 Client 링크·배포 성공, skippedBuild=false다.


실제 EffectObject 검사에서 개별 helper 검사로는 잡히지 않은 format13 경계를 확인했다.
movie의 direct-authored source recipe는 native-v14가 아니므로 기존 reader·전체 문서 validation이
parameter binding을 native 증거로 오인해 거부했고 writer도 name/binding을 누락했다.
원본 실패 로그는 `out/ClassSelectionNativePlay20260925/particle-parameters/dynamic-color-before-v13-codec-fix.log`에 보존한다.
format14로 바꾸거나 검증을 우회하지 않고 format13 enabled source ParticleParameter의
explicit worldSample/name/mapping만 reader·validation·writer가 일치하여 소비하도록 교정했다.
기존 native reference/admission 증거와 legacy actionCue는 여전히 거부한다.


이 format13 교정을 포함한 Debug Product 역시 PASS, skippedBuild=false다.
`out/BuildPipeline/runs/20260924T222029832Z-debug-product.json`과
`out/CharacterMoviePlayFix20260924/product-particle-v13-debug-20260925.log`가 최종 빌드 증거다.
codec3 OBJ와 Client를 갱신했다. 원본 native-v14 증거 gate는 유지했다.


이어 실제 v13 ordinary sourceRecipe도 공통 portable admission을 소비함을 확인했다.
`Validate_AuthoredRuntimeExtensions`의 v15 분기만 보고 이 소비자를 제외할 수 없다.
두 번째 실패는 `out/ClassSelectionNativePlay20260925/particle-parameters/dynamic-color-before-portable-gate-fix.log`에 보존했다.
공통 admission에서 enabled sourceRecipe의 particle sprite/mesh이며 ColorOverLife 또는
ColorScaleOverLife의 color3·alpha1인 검증된 worldSample만 허용했다. native 증거와 중복 property
검증은 유지하고 source binding을 지우는 projection 경로는 계속 명시 거부한다.
이 수정의 Product PASS 증거는 `out/BuildPipeline/runs/20260924T222335251Z-debug-product.json`과
`out/CharacterMoviePlayFix20260924/product-particle-portable-debug-20260925.log`다.
OBJ1개와 Client를 갱신했다. authored-v15 runtimeCarrier schema나 native projection 자체는 바꾸지 않았다.


### G16 실제 소비자 최종 증거와 완료 경계

최종 Product OBJ를 사용한 실제 `CEffectDocumentCodec::Load/Serialize/Parse`와
`CEffectObject/CEffectPlayback` 검사는29개 전부 통과했다. 증거는
`out/ClassSelectionNativePlay20260925/particle-parameters/dynamic-color-result.json`과
`out/ClassSelectionNativePlay20260925/dynamic_color.log`다. Guardian의 원본 mesh/material 및
GPU resource 준비를 사용하는 out 전용1-element 후보에 typed color/alpha distribution을 넣었다.
particle age0.25초의 history RGBA는(1.25,6,0.25,0.4), 같은 age를 유지한 현재 입력은
(2,2/6/10,0.75,0.8), 역방향0.1초 Seek는(1.1,6,0.25,0.4)로 실제 최종 Particles.Color에서
확인했다. 누락된 alpha와 실패한 history provider는 이전 clock/root/color를 보존했다.

원본 Guardian의90초 Intro→Loop2회→Stop→재Play는 feature Product221039 기준으로 다시
통과했다. 이후 reset4줄과 codec/admission 변경은 해당 함수·문서·실제 finalColor 범위로
검증했으며90초 검사를 같은 내용으로 반복하지 않았다. 선택/WORLD 검증과 실제 map stage,
실제 movie owner, typed handoff 대역검사, 실제 EffectObject 출력의 증거를 서로 구분한다.

C++ 소비자 구현·최소 Debug 빌드·자동 검증은 완료했다. 네 class 추가 Data/Resources의 설치와
최종 scene 검증은 해당 데이터 담당 작업이 이어간다. 이 작업의 native 검사는 제품 문서나
리소스를 바꾸지 않았고 Client/UI·GPU draw·오디오 청취·사용자의 최종 화면 판정은 미실행이다.
