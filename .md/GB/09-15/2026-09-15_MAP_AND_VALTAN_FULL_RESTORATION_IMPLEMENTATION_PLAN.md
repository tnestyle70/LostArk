# 맵과 발탄 원본 입력·표현 복원 구현 계획

## G24-V. Full Restore 재생 경로와 실제 클립 연결 수정

2026-09-16 후속 요청은 발탄 복원본 전체를 먼저 재생 가능하게 하고, 패턴에서 연 복원본은
실제 애니메이션과 함께 재생하는 것이다. `.restore` suffix만으로 player recovery에 진입하는
공통 분기를 발탄과 구분한다. 이 수정은 Play, Restart, Solo, 필터 전환에 함께 적용한다.

`ValtanFullRestoreAnimations.json`에 설치된 35개 복원본의 원본 receipt action/stage/clip/time을
등록하고 실제 클립으로 색인한다. legacy actionbindings는 대조 자료로만 사용한다. 현재 패턴의 SourceActionIds와 ClipOccurrences가 모두 일치하는
복원본만 PRODUCT 아래에 표시한다. 이전 G23-V의 action만으로 모은 목록과 standalone 재생은
이 단계로 대체한다. 2페이즈 4방향 공격은 420624 stage007, mesh_att_battle_19_01 하나다.

Effect_Tool.h는 source clip 색인과 새로고침 상태, Effect_Tool_Valtan.cpp는 parse/validate와
정확한 clip occurrence 선택을 소유한다. 기존 Build_ValtanProductPreview/Play_ValtanProductCue를
메모리 내 편집 미리보기에 재사용한다. sourceStart=0, root TRS identity, ARENA_ABSOLUTE 1을
유지하고 notify의 지연과 -90도 보정을 중복 적용하지 않는다. 같은 active 문서를 다시 열 때는
미저장 편집을 보존한다. Product cue나 presentation JSON에 승격하지 않는다.

Loader 20/3760은 source BG의 subspecular-only 입력을 Engine 모델 검증이 거부하는지 실제
CModel로 재현하고, catalog/shader가 지원하는 계약과 일치시킨다. 실패 상태에는 asset/path를
남긴다. 기존 dirty 파일과 프로세스를 보존하며 최소 컴파일과 비UI 입력 검증을 수행한다.
새 C++ 파일 없이 기존 호출 경로만 확장한다. 새 metadata JSON은 프로젝트 None/96.DataFiles에
등록하며 기존 Effect 문서는 다시 생성하지 않는다. builder도 metadata 설치를 함께 유지한다.

## G23-V. All Effects의 PRODUCT 아래 Full Restore 진입점

2026-09-16 요청은 별도 검증용 복원본을 패턴의 PRODUCT 목록 바로 아래에서 여는 것이다.
현재 Authored/Catalog에는 `effect.valtan.action.<sourceActionId>.stageNNN.full.restore`
35개가 있고, 제품 presentation의 해당 cue는 0개다. 기존 G12-V의 승격 보류를 유지한다.

`Client/Private/Effect_Tool_Valtan.cpp`의 `Render_ValtanPatternNode`에서 Saved Pattern
Effects 다음, Unsaved Pattern Draft 전에 `[FULL RESTORE]` 행을 둔다. 기존 exact authored
index와 Pattern의 `SourceActionIds`로 관련 원본 action의 복원본을 모은다. source stage는
제품 stage와 다르므로 행에 원본 stage 문서 ID를 표시하며 동일 action의 여러 variant가
있으면 각각 연다. 제품 cue나 새 source-owner 계약은 만들지 않는다.

같은 CPP의 파일 내부 판별 함수를 목록과 `Matches_ValtanPatternSearch`가 함께 사용한다.
`Open Editor`와 `Play Effect`는 기존 exact-path 재확인과 Standalone Effect 경로를 사용한다.
복원본 선택은 이전 Product unlink 선택을 해제한다. dirty draft 확인과 pending load,
Load/Stage 실패 시 기존 문서 보존은 기존 도구가 소유한다. 열지 않은 행은 Effect JSON을
parse하지 않는다. 신규 H/CPP가 없어 vcxproj/filters 등록은 필요 없다.

수정 CPP의 UTF-8 무BOM·CRLF를 유지하고 최소 Debug 컴파일, 기존 All Effects contract,
35개 Catalog/path와 현재 pattern source action 대응, `git diff --check`를 확인한다.
실행 중 Client/Server와 편집 상태를 보존하며 제품 링크와 최종 화면 확인은 별도 상태로 기록한다.

## G00. 목표와 확인한 입력

사용자가 카메라 복구 성공을 확인했다. 다음 우선순위는 발탄 전투 공간과 발탄 본체,
Character Select, 베른과 발탄 이펙트, 쿠크세이튼이다. 실제 참고 폴더
`C:/Users/user/Desktop/로스트아크_렌더링`의 PNG 13장을 전부 열람했다.
Character Select는 중앙 장식과 외곽 보행 면의 표현이 다르고, 발탄은 서로 다른 바닥 재질의
연결 범위가 불균일하며, 유령의 마스킹·안개·오라가 추가 조사 대상이다. 다른 화면 비율과
플레이어 위치의 스크린샷만으로 전역 맵 배율이나 원본 캐릭터 배율을 결정하지 않는다.

기존 09-14 카메라·렌더링 PLAN/RESULT와 09-11 발탄 재질 복원을 이어서 확장한다.
`pattern-3`의 기존 미커밋 변경을 보존한다. Client 실행과 화면 판정은 사용자가 수행한다.

## G01. 맵별 FOV와 캐릭터 크기

`ArenaCameraProfile.h/.cpp`의 기존 v1 JSON에 optional `characterSizeMultiplier`를 추가한다.
기존 8필드 문서는 1을 사용하고 새 저장은 9필드를 쓴다. 유한한 0.25~4 범위만 허용하며
잘못된 입력과 외부 수정 시 기존 profile·파일을 보존한다.

`Character.h/.cpp`는 기존 catalog presentation scale에 곱하는 화면 표현 배율을 소유한다.
body/equipment/socket이 사용하는 동일 presentation root에 한 번 적용한다. Server 위치,
충돌, 공격 반경은 기존 권위를 유지한다. 네 Level의 profile 적용과 캐릭터 바인딩이 이를
소비하므로 class 교체·재입장에도 저장값을 적용한다. `MainApp.cpp`는 FOV 바로 아래에
Character size 입력과 catalog 기준 복귀를 제공한다. 카메라 preset은 크기 입력을 보존한다.
사용자가 확인한 Bern 수평 FOV 55도를 저장하며 원본 CDO 50도 preset은 출처와 함께 남긴다.
캐릭터 원본 크기는 설치 모델·cook 단위·catalog·원본 actor scale을 대조한 경우에만 수정한다.

## G02. 발탄 맵의 실제 재질 소비자

기존 StaticMesh의 원본 component effective MIC, static parameter set, vertex COLOR,
UV1과 RNM을 join한다. 기존 source family와 같은 static set인 배치는 같은 material program을
재사용하고, 다른 옵션은 해당 원본 shader 근거를 먼저 확보한다. Authoring mapmaterials,
mapplacements와 Imported mapassets를 갱신하고 기존 publisher로 해당 Area를 설치한다.
중앙 Deploy 바닥은 별도 기존 CDeployPropObject 소비자를 조사하여 CModel/CMaterial 경로에
원본 재질을 연결한다. 밝기 보정만으로 diffuse-only 입력을 복원 완료 처리하지 않는다.

정적 Deploy의 `BossCatalog.modelMaterialOverrides`를 기존 map material parser와 bind helper에
연결한다. `CMapAssetCatalog::Parse_ModelSurface`는 단일 row를 같은 parser로 stage하고 성공한
결과만 반환한다. actor 경로는 per-placement bakedLighting과 비지원 blend/cull을 거부한다.
일반 map parser를 복제하거나 새 모델 런타임을 만들지 않는다.

## G03. 발탄 본체와 이펙트

`CBody_Valtan`과 기존 source skeletal shader에서 유령 program 84의 varying, blend, depth
계약을 확인한다. 기존 source translucent forward pass를 재사용해 ordered coverage에 의한
픽셀 소실을 수정한다. normal/ghost/armor/axe의 서로 다른 cook 단위는 실제 정점·preScale·
socket basis를 대조한다. source material의 내용과 actor 크기를 별개로 검증한다.

현재 패턴·진입·2페이즈·whirlwind·무기·오라·푸른 에테르의 원본 occurrence와 실제 cue/asset
소비자를 연결한다. 기존 Effect_PresentationService와 material ABI를 확장하며, 별도 런타임과
원본 근거 없는 전역 회전·배율은 추가하지 않는다.

LookInfo의 기본 particle은 action notify와 별개로 `defaultParticles`에서 실제 본을 참조한다.
기존 Effect_Playback의 owner-sustained 옵션은 Valtan 기본 오라의 source infinite emitter만
살려 둔다. 유한 emitter는 기존 종료 정책을 유지하며 owner 종료·모델 교체 때 handle을 정리한다.

## G04. 맵별 원본 후처리와 환경

Valtan, Character Select, Bern, Kouku의 WorldInfo/CDO/chain/volume와 LUT 본체를 추출한다.
native ToneScale/Range/Toe와 shader 상수 packing은 원본 CPU 함수·상수 근거로 복원한다.
활성 override와 비활성 속성을 구분하고 기존 RenderingProfileService/Benchmark의
Before/Restored/Return 흐름을 확장한다. 입력만 연결한 adapter와 native 식 복원을 구분한다.
안개·스카이·region·맵 재질과 환경 occurrence는 같은 맵의 원본 참조를 따른다.

기존 Engine quality와 Shader_Deferred에 optional sourcePostProcess 및 LUT bake pass를 추가한다.
원본 UE3 customizable curve와 256×16, 16³ 색 보정 LUT를 사용한다. 기존 pass index는 유지하고
별도 선택이 없는 profile의 Hable 출력은 보존한다. 새 GPU 리소스 생성은 stage 후 교체하며
실패 시 기존 quality와 LUT를 보존한다.

Character Select 중앙 문양은 원본 mesh/UV/MIC/배치 누락 여부부터 확인한다. 베른의 물·나무·
폭포·창문 빛, 쿠크의 렌더링과 맵 크기는 실제 원본 배치·카메라 영역·shader로 대조한다.

## G05. 검증과 인계

새 C++ 런타임 파일은 현재 계획에 없다. 필요 시 실제 소비자와 프로젝트/필터 등록을 함께 추가한다.
변경 JSON/XML parse와 입력 참조, 실제 parser의 저장·실패 보존, 원본 단위·투영·shader 수치,
변경 기능의 최소 컴파일과 정상 Debug Product build, `git diff --check`를 확인한다.
이미 통과한 검사를 근거 없이 반복하거나 광역 하네스를 추가하지 않는다.
RESULT에 실제 반영·원본 근거·검증·남은 기능과 사용자 화면 확인을 분리해 기록한다.

## G06. 활성 맵의 밝기 회귀 복구

사용자가 Character Select부터 쿠크까지 전체 화면의 과도한 밝기·번짐을 보고하고 빠른
복구를 요청했다. 네 기본 profile만 `579d9b90^`의 quality·조명·안개·환경영역 객체로 돌린다.
기존 globalQuality, 나머지 scene profile과 추가 before/source 비교 profile은 유지한다.
메시·재질·카메라 복원 및 source shader 구현을 되돌리지 않는다. revision을 올리고 기존
Rendering publisher로 검증·게시한 뒤 사용자 Reload Runtime으로 활성 화면을 재적용한다.

## G07. 쿠크 3관문 직접 반사광 입력 연결

3관문 FLOOR08/FLOOR08A의 geometry·재질·D/N/S/reflection은 기존 복구본과 같지만,
광원 제출은 specular RGB를 0으로 만들고 선택 바닥의 marker1 경로는 이를 곱한다.
Engine `Shader_Deferred.hlsl`의 선택 바닥 직접 반사만 광원의 diffuse radiance를 소비하도록
연결한다. 기존 Phong lobe·재질 RGB·감쇠·그림자는 유지하고 marker0/2의 legacy specular
설정과 다른 native family는 보존한다. 이 변경은 직접 반사 입력 단절의 교정이며 원본
Blinn 조명식이나 바닥 전체 검정 증상의 최종 시각 복원으로 설명하지 않는다.

기존 PointLightFalloffContractHarness에서 실제 제품 CSO의 Directional/Point/Spot을
marker0/1/2와 specular 0/양수 입력으로 draw/readback한다. 수정 전 marker1 반사 0을
재현하고 수정 후 유한 RGB 및 legacy 보존을 검사한다. 정상 Debug Product Build로
동일 CSO를 배포하며 사용자 저장 RenderingProfiles·maplights·Resources는 바꾸지 않는다.
새 프로젝트·파일·public 데이터 계약은 추가하지 않는다. 최종 화면은 사용자 확인 항목이다.

## G08. 전체 발탄·Character Select·베른으로 복원 범위 확장

사용자 재개 요청으로 전투 범위 밖을 포함한 발탄, Character Select, 베른 전체 배치의 실제
material slot과 원본 effective MIC를 대조한다. 이미 복구한 재질과 geometry를 유지하면서
누락된 source family, 추가 UV·정점색, 배치별 RNM과 환경 particle을 기존 소비자에 연결한다.
발탄 중앙 아래층 `LV_LUT_HEARTRB_ED_SL00:export:1196`의 두 slot은 원본 component
override와 native mesh material array를 함께 사용해 복구한다. 원본과 다른 사용자 저작
Effect는 보존하고 별도 full restore 문서에 원본 발생·재질·본·시간을 연결한다.

Character Select의 803개 component와 55개 원본 mesh, 92개 사용 slot을 출발점으로
전체 source material을 조사한다. 현재 선택 중앙 재질 9행을 전체 맵 복구로 세지 않는다.
베른의 기존 정적 재질 23,153행은 다시 생성하기 전에 현재 데이터를 확인하고, 미연결 환경
particle·광원 함수·바람 등 실제 남은 입력을 구분한다. Source 후처리 일괄 활성화로
앞서 사용자가 되돌린 기본 밝기를 다시 바꾸지 않는다.

발탄 3연속·4방향·십자 돌을 우선해 전체 full restore 발생으로 확장한다. 기존
`CModel -> CMaterial`, Effect v15, MapEffectPresentationRuntime 및 Area publisher를
사용한다. 새 원본 재질 program은 기존 shader family에 추가하며 새로운 모델 런타임이나
pattern별 하드코딩 렌더 경로를 만들지 않는다.

## G09. UE4 전환 가설과 추출 경로 검토

설치 원본의 패키지 헤더와 import/export를 새로 읽어 엔진 형식을 먼저 판별한다.
Character Select·베른·중앙 MIC 패키지는 version868/licensee16/engine12097인 현재 UE3
패키지다. PBR·ORM·새 MIC의 존재는 UE4 전환 증거로 사용하지 않는다. 신규 재질/배치가
추가됐을 가능성은 현재 effective MIC, parent와 shader-cache의 실제 입력에서 확인한다.

실제 UE4/5 컨테이너가 확보되는 경우에는 해당 버전의 UModel 또는 FModel/CUE4Parse로
mesh·material instance·texture·skeleton/animation을 함께 추출한다. 설치 원본을 변경하지
않고 추출은 out에 stage하며 기존 geometry cooker와 native material 연결 검증 뒤 설치한다.
현재 설치에 없는 UE4 자산을 임의 생성하거나 UE4 도구 전환만으로 복원됐다고 기록하지 않는다.


## G11. Character Select의 명시 PBR 입력과 선택적 환경 Effect 진입

사용자 실행의 `client-session-87156.jsonl`에서 Character Select 두 입장이
`Map: model prototypes 23/127`로 실패했다. 현재 scope와 catalog를 대조한 24번째는
`MAP_94E1EF196C1A_LV_MODULE_MESH03_512_OVR_75EF6B0F204F_FR_E0FED3CFC174`다.
설치 WModel의 UV1은 보존돼 있으나 dummy 기본 material의 normalPath는 비어 있다.
원본 PBR descriptor의 surfaceNormalPath는 존재한다. Model override와 Material 생성은
PBR의 실제 명시 입력을 검사하고, 기존 기본 D/N/S를 소비하는 재질의 검사는 유지한다.

Character Select 진입은 기존 Loader의 optional ambient 문서 계약과 일치시킨다.
게시 파일이 없으면 환경 Effect 없이 진입하고, 경로 조회 오류나 존재하는 문서의
손상은 실패시킨다. Bern과 Valtan의 기존 필수 Load 호출은 변경하지 않는다.
새 C++ 파일이나 프로젝트 등록은 없다.

현재 참조 텍스처 5개는 TGA 본문을 .dds 이름으로 복사한 설치 오류다. 원본
RGBA 픽셀과 크기를 유지해 out에서 무손실 DDS로 변환하고, 원본 백업·전후
픽셀 일치·DDS 헤더를 검사한 뒤 해당 Resources 파일만 교체한다. 텍스처 ID와
재질 값은 유지한다. 현재 scope 127개 실제 모델 로딩을 다시 확인한다.


검증은 현재 설치 모델과 명시 material을 실제 CModel로 준비하는 headless probe,
관련 최소 컴파일 및 Product Build, JSON parse와 diff check로 수행한다. Client 실행과
화면 판정은 사용자가 담당한다. 문양 후보 11개 발견은 중앙 별·물방울 장식 식별 또는
배치 적용 완료와 구분해 RESULT에 기록한다.

## G12-V. 사용자 검증 전 발탄 Effect 연결 보류와 시퀀서·카메라 조절

사용자는 신규 full restore를 육안 검증한 뒤 항목별 제품 승격을 지시한다.
이번 두 canonical transaction만 역순으로 조건부 되돌려 기존 cue와 V2 binding을 복귀한다.
별도 full restore 문서·catalog와 native 재질은 검증 자료로 보존한다. 새 문서의 schema,
source 시간 창, collision freeze 및 재질 지원은 기존 consumer에서 검증하고 제품 cue에
추가하지 않는다. 기존 사용자 변경을 일괄 baseline으로 덮어쓰지 않는다.

Sequence의 Boss 목록에 Valtan을 허용하고 기존 CValtanActionWorkbench의 canonical
Pattern/Stage 및 Animation·Effect·Sound·Logic·Collider·Camera·World 행을 재사용한다.
새 gameplay 정본이나 별도 Valtan 모델 runtime을 만들지 않는다. input owner와 preview
회수도 Sequence target을 포함한다. 원본 진입 컷씬 Camera/Effect는 source occurrence를
확인해 검증 자료로 준비하며 기존 영상 참고 카메라를 원본 native 복구로 세지 않는다.

Kouku를 포함한 Follow Camera에 주시점을 유지하는 거리·pitch·yaw 조절을 노출한다.
거리 변경은 focusDistance만 바꾸지 않고 실제 eye offset을 같은 focus 주위에서 재계산한다.
기존 JSON pose/lens와 Save freshness를 사용하고 source baseline 및 사용자 저장값은
임의로 교체하지 않는다. 현재 1관문 원본 19m/수평50도와 2·3관문 미확정 경계를 유지한다.

새 C++ 파일은 없다. 변경 TU와 실제 shader consumer를 out에서 컴파일하고 실제
profile 함수·session 라우팅·Effect codec/playback의 수치 검증을 수행한다. 실행 중인
Client/Server 및 설치 EXE/DLL은 교체하지 않고 사용자 화면 판정을 기다린다.


## G12. 식별한 중앙 별 조각을 기존 배치 경로로 추가

원격 원판1036/별1040의 실제 mesh·UV·diffuse에서 원본 중앙 별 장식을 식별했다.
원격 원판에서 중앙 흰 원판336으로 가는 group 변환을 별에 적용해 원판 대비 크기와
방향을 보존한다. scale은2.25428036849, quaternion은(-.5,.5,-.5,.5)다. 현재 다리 표면에
묻히지 않도록 실제 앞면 삼각형의 높이 교차를 계산한 추가Y3.0731624cm를 적용한다.
최종 pivot은(-772.022176277,-142.886223812,197.538195165)이며 앞면은 현재 표면과
최소1cm 간격이다. 이 높이 보정은 현재 조립을 위한 값이며 원본 target의 저작값이 아니다.

기존 원격11쌍은 보존하고 MapTool의 editor placement ID 계약으로 별 인스턴스1개를
추가한다. 기존 physical WModel과 검증된 D/N texture를 재사용하며, 원본 MIC의
bg_base_pbr_opa variant를 Imported catalog와 Authoring mapmaterials에 연결한다.
미설치 reflection texture1개를 원본hash 확인 후 Resources에 설치한다. 기존 배치의
재질/위치와 사용자의 다른 변경은 보존한다. 새 C++·모델 런타임은 추가하지 않는다.

Authoring을 수정한 뒤 동일 Area publisher로 Validate/Publish/Check하고, 증가한 scope의
실제 CModel 생성과 placement parser를 검사한다. 모델/표면 교차 수치와 사용자 최종
화면 확인은 분리한다. 물방울·곡선·외곽 장식은 식별된 원본부터 같은 절차로 추가한다.

## G15. 발탄 검증용 native Modulate와 충돌 Event 소비자

- 제품 승격은 보류한다. 신규 full restore cue는 0개를 유지하고 기존 V2 102개 binding과 사용자 저작을 보존한다.
- 원본 `fx_a_aura_01_1_mo` / native 2553은 sprite, 단면, `blend_modulate`, RT0만 쓰는 PS다. 기존 render profile 번호와 문자열을 보존하며 마지막에 단면 Multiply profile과 particle pass 5만 추가한다.
- 실제 소비자는 `CEffectDocumentCodec::Validate -> Has_ArtistMaterialContract -> CEffectDocumentRenderer::Select_Pass -> native particle carrier`다. Sprite 원본 재질 계약이 없는 Multiply 문서는 거부한다.
- RT0 RGB에는 `Dst * Src`를 적용하고 alpha는 보존한다. RT1 distortion과 RT2 bloom은 원본 PS 출력이 없으므로 쓰지 않는다. 원본 factor에 emissive 배율과 bloom emission 변환을 적용하지 않는다. 기존 Alpha/Add와 V2 Multiply는 수정하지 않는다.
- native installer의 원본 blend admission과 case 생성도 같은 계약으로 확장한다. 새 H/CPP는 없으며 기존 shader group 2496을 사용한다.
- 실제 D3D11 WARP draw/readback으로 원본 함수, source alpha 0/1, HDR 목적지, 보조 RT 보존, 기존 Alpha/Add control을 검사한다. Codec와 drawable admission은 실제 현재 C++ OBJ로 확인한다. Client 시각 결과는 사용자 확인 전까지 미확인이다.
- 충돌 completion은 실제 PhysX 접촉으로 Kill/FreezeMovement를 검증한다. FreezeMovement는 위치·회전만 멈추고 색·크기·수명은 계속 평가한다. Collision Event는 기존 bounded event queue를 사용하며 source occurrence별 이름으로 다른 팔의 receiver와 분리한다.


## G13-CS. Character Select 전체 Area 배치 로딩

후속 요청은 추가 대상의 원본 재질·구운 조명·환경 반사까지 복원하는 것이다. 사용자
16:08 빌드에 이미 포함된 전체 범위는 제품 descriptor에서 유지한다. 재질 검증을 기다리며
중앙 범위로 되돌려 다음 빌드에서 기존 기능이 사라지게 하지 않는다. 재질의 실제 준비와
미게시 상태는 RESULT G14-CS를 따른다.

사용자는 원격 원판과 장식을 일반 Character Select에서도 확인하도록 전체 반영을 요청했다.
현재 LevelRegistry의 중앙 X/Z 범위는 전체 804개 배치 중 22개(원판11/별11)를 제외한다.
Character Select descriptor를 기존 MakeFullMapScope()로 바꾸어 동일 SL00 Area의 모든
배치를 Loader 모델 준비와 Level placement 생성에 함께 전달한다. 별도 runtime 경로나
Test 전용 복사본은 만들지 않는다. 기존 배치 transform, 재질, placementLighting은 보존한다.

수정 파일은 Client/Private/LevelRegistry.cpp이며 기존 파일 한 줄 교체다. 새 H/CPP,
project/filter 등록과 JSON 변경은 없다. Bern/Valtan/Kouku의 현재 full scope는 유지한다.
전체 배치의 사용 모델을 실제 CModel/Create_MaterialVariant로 검증하고 변경 TU를
별도 out에 컴파일한다. 실행 중 Client/Server는 유지하며 EXE 링크·배포와 사용자 화면
확인은 별도로 기록한다. 전체 로드는 미식별 원작 장식의 식별·복원 완료를 뜻하지 않는다.

## G18-CS. 원격 11쌍의 중앙 인근 배치와 누락 재질 입력 연결

사용자가 원격 바닥을 직접 찾았고, 실제 Bern이 아니라 Character Select 중앙 섬 근처로
11쌍을 서로 겹치지 않게 옮기도록 확정했다. 원판과 별의 상대 transform, stable ID,
개별 RNM tile을 보존하고 공통 평행이동만 적용한다. 기존 중앙 별1개는 이동 대상에서
제외한다. 기존 기하와 조립체의 실제 bounds를 대조해 가까운 전시 배열을 선택한다.

별의 누락된 원본 MIC/RNM12행과 6개 atlas variant를 기존 CModel/CMaterial 경로에
연결한다. 원판11개와 별12개의 12개 material variant에는 원본 HDR02 cube, component
RGB와 minimumRoughness0.05를 연결한다. `_02`의 diffuseBrightness0은 원본값이므로
밝기를 맞추려고 변경하지 않는다. 기존 프로젝트 BRDF와 현재 PBR shader를 사용한다.
120도는 현재 프로젝트의 sin/cos 규약으로 변환하며 native binder의 부호는 미확정이다.
component alpha1을 additive floor로 오인하지 않고 기존 프로젝트 floor0을 유지한다.

이 변경은 확인한 입력의 누락을 고치는 단계다. 원본 BRDF payload, SH packing,
hemisphere/ambient까지 완전히 복원했다고 기록하지 않는다. 이동한 RNM은 원래 정적
배치의 조명이며 새 위치에서 구운 그림자가 아니다. 원본 기준과 미확정 항목은 RESULT에
구분한다. source hash를 재확인해 다른 편집을 보존하고 Area Validate/Publish/Check,
실제 CModel material 생성 및 배치의 간격을 검사한다. 새 C++·HLSL·프로젝트 등록은
없다. Client/Server는 조작하지 않으며 EXE 빌드와 최종 화면 확인은 사용자가 수행한다.

## G19-CS. F1 바닥 교체와 접기·펼치기 UI

F1의 Arena Camera / Player 아래 Character Select Floor Swap을 추가한다. 사용자는
원본 직업 무대11쌍을 골라 중앙에 원판과 별을 함께 맞추고, offset·yaw와 환경 입력을
비교할 수 있다. 기존11쌍 전시 배치는 유지하고 Original로 되돌리면 원래 중앙 배치가
복구된다. 이번 선택은 맵 방문 중 시각 비교 상태이며 MapTool 저장·Server navigation을
수정하지 않는다. 목록·stable source ID·기본 offset 정본은
Data/Rendering/Authored/CharacterSelectFloorSwap.json이다.

Level_CharacterSelect가 문서를 parse/validate하고 실제 로드된 floor/star record,
배치별 RNM 및 중앙 기준 transform을 resolve한다. source floor→중앙 floor의 조립체
변환을 별에도 적용한다. 기본Y offset0.2m는 현재 bridge/brown floor의 위쪽 기하와
31,102표본을 대조한 최소 필요값0.19155m에서 정했다. 사용자는 offset을 조절할 수 있다.

MapPlacementRuntime의 Debug preview transaction이 기존 CMapAssetObject/CModel/
CMaterial 경로로2개를 먼저 stage한 뒤 기존 중앙 원판·별의 visibility를 바꾼다. 실패하면
새 객체를 회수하고 이전 선택·visibility를 보존한다. Preview의 material override는
CModel::Create_MaterialVariant로 분리해 원래11쌍과 공유 prototype을 바꾸지 않는다.
Source Stage는 원본 HDR02 입력과 조립체 회전을 사용하고 Center Environment는 중앙
원판의 environment 입력을 재사용한다. 두 모드 모두 원본 RNM tile을 새 위치에서 구운
그림자로 부르지 않는다. scene direct light는 실제 새 world position을 소비한다.

MainApp는 Level의 typed 선택 API만 호출한다. Arena Camera / Player, Player Follow
Camera, Kouku UI Preview를 독립 CollapsingHeader로 정리한다. MainApp의 UTF-8 no-BOM
소스가 ANSI로 해석될 수 있는 한글 UI literal은 기존 파일 인코딩을 유지하며 명확한
ASCII 라벨로 바꾼다. 새 JSON은 Client.vcxproj와 filters의96.DataFiles에만 등록한다.

모자이크 진단에서는 원본 DDS의 mip chain과 실제 sampler/CSO를 대조한다. diffuse
brightness나 shared roughness를 임의로 올리지 않는다. source mip을 확보한 파일만
동일mip0·색공간·압축 계약을 확인해 복구한다. 변경 C++의 최소 out 컴파일, 실제 모델
생성, 선택·실패·원복 transaction, JSON/XML parse와 diff 검사를 실행한다. EXE 빌드와
Client 화면 조작은 사용자가 수행한다.

## G20-CS. 사용자 화면의 검은 표면과 원판·별 높이 분리

G19 뒤 사용자 첨부에서 넓은 원판과 별의 검정이 계속되고 큰 장식 링이 사라진 상태를
확인했다. 사용자 additional Y=-0.17은 preset+0.20과 합쳐 실제+0.03이다. 같은 XZ의
실제 삼각형 높이 비교에서 이 높이는 큰 링482의 상향표면6,284표본을 전부 가리고,
원래 높이0은74.4%를 남긴다. 별은 원래 높이에서 bridge에 가려져 별도 보정이 필요하다.
따라서 원판 preset offset은0으로 되돌리고 별에만 기존 중앙 별의 검증된0.0307316239m
높이 보정을 적용한다. JSON에 starOffsetMeters를 명시하고 Level의 기존 parse/validate와
조립체 변환 뒤 해당 offset을 별에만 적용한다. 추가 XYZ/yaw는 계속 조립체 전체에 적용한다.
원본 배치·preview transaction·원판과 별의 scale 및 RNM은 보존한다.

회색 빈 곳은 기존804배치의 실제 삼각형과 visibility, 원본 component를 대조한다.
큰 bounds나 이름만으로 바닥 후보를 활성화하지 않는다. 검은 표면은 원본 static MIC의
diffuseBrightness0와 실제 uniform expression/원본 DXBC를 대조한다. Source/Center
환경 선택이 원본의 바탕색0을 변경하지 않는다는 점을 분리한다. 원작 실행 중 material
변경값이 확인되지 않으면 임의의 brightness를 원본 복원값으로 게시하지 않는다.

새 C++ 파일과 project 등록은 없다. 수정 Level의 최소 컴파일과 기존 실제 parser/변환
probe, 변경 JSON parse 및 diff 검사를 수행한다. 원본 source·Resources와 실행 중
Client는 유지하며 EXE 빌드와 화면 판정은 사용자가 수행한다.

후속 실측에서 source488의 폭20.406m 바닥면을 확인했다. 이 면의 현재 숨김은 원본
actor/component의 bHidden 설정이 아니라 importer의 LV_MODULE 전체 module-proxy
분류다. 실제 source MIC·RNM과 기존 CModel 표면 지원을 확인한 이 한 배치만 Authoring
visible=true로 복구하고 Area Validate/Publish/Check로 게시한다. 원본 transform·asset ID·
조명은 보존하며 다른 module proxy나 water를 일괄 켜지 않는다. 원판 바깥의 실물 기하
coverage와 실제 material loading은 사용자 최종 화면의 동일성과 구분한다.

사용자가 승인한 F1 brightness 비교는 기본 OFF로 원본 값을 유지하고 ON 적용 시
임시 원판·별의 material variant에만 diffuseBrightness=1을 쓴다. 원본 catalog와 11쌍은
변경하지 않는다. Reset은 OFF이며 실패한 Apply는 이전 설정과 재질을 유지한다.
이는 shader 기본값의 진단 비교이며 원작 실행 중 MIC 값 복원 완료를 뜻하지 않는다.

## G21. 팀 공통 맵 추출에서 가시성·기하·재질 근거 보존

사용자는 이번 누락의 재발 방지를 공통 추출 도구와 팀 절차까지 요청했다. 기존
extract_ue3_placements → build_map_material_variants → build_maptool_scene → Area
publisher와 CModel/CMaterial 소비자를 확장한다. 새 모델 런타임을 만들지 않는다.

1. placement schema3에 actor/component와 archetype/CDO의 실제 visibility 근거를
   보존한다. LV_MODULE, nav, water, FX 이름 분류는 진단용이며 visible을 결정하지 않는다.
   navigation 참여와 렌더 visibility를 분리한다. 구버전 입력의 근거 부재와 미지원 재질은
   명시적으로 보고하고 불완전 geometry preview 승인을 제품 복원 완료로 기록하지 않는다.
2. Bern 공통 cook과 map variant cook에 기존 WModel geometry contract를 연결해 실제
   glTF의 추가 UV, tangent, COLOR0를 보존한다. 없는 source channel을 만들어내지 않으며
   원본 package→glTF 손실 여부와 runtime 연결 증거를 각각 남긴다.
3. 개별 out 작업에서 검증한 source surface 매핑을 공유 도구로 옮기고 실제 CLI 소비자에
   연결한다. full source material path, terminal, child-wins effective scalar/vector/static
   switch, texture hash·색공간, source slot과 배치별 RNM/environment 입력을 함께 검증한다.
   명시적 0은 유지하고 미지원 graph/active branch는 실패 목록으로 보존한다. 환경의
   프로젝트 근삿값과 원작 동적 uniform 미확정 상태를 원본 값과 구분한다.
4. 원본 mip 회수 도구를 공유 Tools에 둔다. BC1/BC3/BC5와 redirect를 지원하고 원본 package는
   읽기 전용으로 유지한다. mip0 일치와 전체 chain 검증 후 출력만 교체한다. 색공간 변경이나
   재압축으로 원본 회수를 대체하지 않는다.
5. scene consumer가 실제 material/runtime coverage와 file hash closure를 확인하도록 한다.
   source visibility와 로드 준비 실패를 섞어 숨기지 않는다. authoring 갱신 후 배포는 기존
   Area Validate/Publish/Check만 사용한다.

도구별 회귀 테스트와 실제 SL00 자료 재처리, 변경 Python 문법·JSON/XML parse 및 diff 검사를
실행한다. 기존 재질 family의 실제 지원 범위를 넘는 원본 shader 복원이나 Client 화면 PASS를
주장하지 않는다. 공통 README와 팀 material binding 계약·반복 결함 문서를 같은 변경에서
갱신한다. 사용자 빌드 전 실행 준비 상태를 보고하며 Client 실행·화면 검증은 사용자가 한다.

## G22. 큰 바닥의 보행 범위, 누락 장식과 마지막 팔각별 보정

사용자 새 화면에는 source488의 넓은 바닥이 표시되고 중앙 ARCH01A 별은 여전히 검다.
488은 원래 transform의 한 배치이며 원격11쌍을 중앙에 중첩한 결과가 아니다. 링 위 사각
부조가 왼쪽 아래에서 빠지고 외곽 세로 첨탑의 재질이 미복원된 증상도 별도로 조사한다.
사용자는 별을 이번에 마지막으로 수정하고 다음 확인에서도 검으면 제거하도록 지시했다.
이번 변경에서 별을 먼저 삭제하거나 사용자 화면 성공을 가정하지 않는다.

### 재질 정본과 기존 소비자

`Data/Maps/Authoring/LV_LOBBY_CLASSSELECT_SL00/LV_LOBBY_CLASSSELECT_SL00.mapmaterials.json`의
정확한 ARCH01A _02 재질6행만 diffuseBrightness=1로 저작한다. 중앙 clone과 원격11별이
이6개 asset을 공유한다. 원본 MIC0은 추출 근거에 보존하고 밝기1은 원본 shader 기본값을
사용한 프로젝트 보정으로 기록한다. 다른 표면 값·sourceMaterial·RNM·환경은 보존한다.
`CMapAssetCatalog -> CModel/CMaterial`과 기존 F1 preview 복사 경로가 동일한 값을 소비한다.
`CMainApp::RenderCharacterSelectFloorSwapControls`의 OFF/적용값 설명은 source가 아닌
authored material setting으로 교정한다. 체크 ON은 계속 원판·별에1을 적용한다.
새 C++ 파일이나 별도 renderer, 추가 project/filter 등록은 필요하지 않다.

### 장식과 외곽 재질

첨부 사각 부조를 실제 설치 geometry와 원본 asset/slot에 대조한다. 독립 배치인지 링의
일부인지 먼저 확인하고 위치·회전·가림을 검사한다. 원본 복구가 가능한 경우 빠진 왼쪽
아래를 채우며, 불가능할 때만 사용자가 허용한 나머지3개 제거 범위를 정확히 특정한다.
외곽 첨탑은 실제 source MIC·native shader 분기·texture·RNM과 현재 legacy 소비자의
누락을 대조하고 확인된 입력을 기존 material 경로에 연결한다. 미지원 분기를 기본값으로
우회하거나 같은 이름의 다른 material에 일괄 보정을 적용하지 않는다.

### 네비게이션과 종료 검증

`Data/Navigation`의 현행 CS navsource/navpaint와 실제 `CNavGridBaker`를 사용해 새로 표시된
488을 포함한 중앙 보행 표면을 재베이크한다. 기존 차단 paint의 근거를 검토하고 실제 삼각형
높이·연결성·최대step·낭떠러지를 확인한다. 공식 Server navigation publisher가 같은 결과를
Client/Server에 원자 게시한다. 시각 장식과 멀리 둔11쌍을 새로운 보행 영역으로 가정하지 않는다.

변경 material/placement는 Area Validate/Publish/Check, 실제 CModel 생성, 대상 표면 입력
수치 비교를 수행한다. 수정 CPP 최소 컴파일, JSON/XML parse와 scoped diff 검사를 실행한다.
Server 재시작 필요와 제품 EXE 미링크를 명시하고 사용자가 새 빌드·화면·이동을 확인한다.

## G25-V. 발탄 주요 패턴의 원본 재질·셰이더·크기 재대조

사용자가 제공한 네 장면의 도끼 전기, 개별 생성돌, 4방향 물보라와 포탈·돌진 바닥을
쿠크의 원본 MIC → MaterialMap/VF/VS/PS → native material/shader → occurrence 경로로
검토한다. 기존 재생 연결과 원작 화면 일치는 별개이며 사용자 화면 승인을 대신하지 않는다.

- `build_kouku_pattern_native.py`로 물보라의 네 원본 PS, 60 emitter occurrence를 새로
  추출하고 기존 25 native program의 식·텍스처·CB와 원본 cm/notify scale을 대조한다.
  기존 ID와 정상 방향을 보존하며 차이가 확인된 입력만 교정한다.
- 누락된 6방향 충전 Atk_08_05와 포탈 entry/exit, SkillDecal 2002를 source action/stage로
  연결한다. GroundEffect의 LocalDecal VS/PS와 engine-owned CB0 prefix를 확인한 후
  기존 native decal adapter에 정확한 PS를 추가한다. 새 renderer는 만들지 않는다.
- 개별 생성돌에서 명시된 원본 mesh/MIC와 동일한 source occurrence의 texture/native
  profile을 연결한다. 원본 stationary owner를 확인하지 못한 수명·배치는 프로젝트 값으로
  남겨 두고 원작 복원 완료로 표기하지 않는다.
- Tool의 본 follow는 실제 무기/몸의 owner basis를 유지하고, snapshot 바닥은 arena 크기를
  유지한다. 원본 Anim playMs와 source stage의 previewWallMs를 분리하여 loop의 조기 정지를
  수정한다. 조건부 전환의 시간은 preview 범위라는 근거를 명시한다.
- 기존 `ValtanFullRestoreAnimations.json`, EffectCatalog와 96.DataFiles 등록을 확장한다.
  Full Restore는 기존 검증용 목록이며 Product cue의 정본을 임의 승격하지 않는다.

수정한 CPP 최소 컴파일, 해당 shader carrier 컴파일, source-native program/texture closure,
실제 설치 WModel의 본·배율 및 시간 수치, 변경 JSON/XML parse와 scoped diff 검사를 수행한다.
Client/Server 실행 중 EXE/DLL을 교체하지 않는다. 사용자 화면 조작·최종 판정은 별도이며,
미확정 원본 입력·제품 빌드 상태와 실제 완료 증거는 대응 RESULT에 분리한다.