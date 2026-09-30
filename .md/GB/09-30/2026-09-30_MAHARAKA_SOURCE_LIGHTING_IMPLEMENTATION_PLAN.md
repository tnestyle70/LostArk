# 마하라카 원본 환경·조명 연결 구현 계획

## G00. 현재 연결과 작업 경계

요청은 기존 마하라카 맵의 원본 환경광·조명을 복원하고 필요한 조명 리소스를
`C:/Users/user/Desktop/GBResources`에 전달하는 것이다. 기준은 현재 저장된
`LV_OCN_EVENTIS_MHP`와 실제 원본 package다. 기존09-26 G05 scene 입력 RESULT와
09-27 RNM 적용 RESULT를 이어서 실제 소비자까지 연결한다.

현재 `Client/Private/LevelRegistry.cpp`의 마하라카 descriptor는
`scene.development.neutral.v1`을 사용한다. RenderingProfiles revision85에는
마하라카 전용 profile이 없다. 맵의 RNM과 local maplights는 이미 존재한다.
광원 receiver를 생략한 기존33행은 ALL로 해석되므로 원본 baked flag와 기존
UNBAKED 소비 계약을 대조한다.

작업 폴더에는 다른 기능의 변경이 다수 있다. 해당 변경과 다른 맵의 렌더링 튜닝을
보존하고 자동 stage/commit하지 않는다. Client/UI 실행·조작·화면 판정은 하지 않는다.

## G01. 원본 환경 입력과 전용 profile

`LV_OCN_EVENTIS_MHP_PS`의 환경 override, directional light, fog와 post-process,
`SL01`의 sky/local light를 현재 원본에서 대조한다. 회수한 값과 단위 변환,
원본에서 확인하지 못한 class default·카메라 적용 조건을 구분해 결과에 남긴다.

`out/MaharakaEnvironmentRestore_20260930`에 profile 후보와 근거를 준비한다.
현재 development profile의 프로젝트 전용 필드를 보존하면서 원본에서 확인된
조명·안개·색 보정을 마하라카 전용 profile에 연결한다. 다른 profile과 globalQuality,
무관한 FXAA·노출·감마 설정은 변경하지 않는다.

`Data/Rendering/Authored/RenderingProfiles.json`에 전용 profile을 추가하고
`Tools/RenderingPipeline/Publish-RenderingProfiles.ps1`로 Validate/Publish한다.
LevelRegistry의 마하라카 descriptor만 같은 profile ID로 바꾼다. 기존
RenderingProfileService의 parse/validate/stage/commit 소비 경로를 유지한다.
새 profile은 RenderingProfileService의 삭제 보호 목록과 publisher의 필수 profile
목록에도 등록한다. 새 C++ 파일이나 project/filter 항목은 필요 없다.

## G02. 구운 조명과 local light의 수신 범위

원본 lightmap에 포함된 광원의 위치·색·반경·falloff·cone을 보존하고, 같은 빛이
RNM 표면에 다시 더해지지 않도록 기존 UNBAKED receiver를 사용한다.
`Data/Maps/Authoring/LV_OCN_EVENTIS_MHP`의 maplights만 좁게 수정하며
`Publish-MapAuthoring.ps1 -AreaId LV_OCN_EVENTIS_MHP`로 해당 Area를 검증·게시한다.
원본 spotlight의 cachedParentToWorld를 읽어 기존 수평 방향을 아래 방향으로 교정한다.
다른 Area는 이 변경에 포함하지 않는다.

## G03. 원본 정적 그림자 atlas 연결

원본 SL01 component의 ShadowMap2D와 dominant directional light의 light GUID를
조인한다. RNM의 lightmap GUID와 정적 shadow의 light GUID를 혼동하지 않는다.
원본 G8 texture의 전체 mip payload를 추출하고 기존 staticShadow 채널1과
배치별 shadowCoordinateScale/Bias로 직접광에만 적용한다.

같은 asset이 서로 다른 shadow atlas를 쓰는 배치에는 필요한 최소 material/catalog
variant를 만든다. 같은 WModel geometry를 공유하고 stable placement ID·Transform·
visibility와 기존 RNM 입력을 유지한다. 직접광에만 기존 SDF transfer를 사용하는
project adapter이며 원본 CPU의 SH·모든 shader 계산과의 동일성을 주장하지 않는다.
후보 보존 검사와 Area publisher Validate 후 해당 Area 정본·게시본을 함께 갱신한다.

## G04. 설치·전달·검증

후보 검증 후 최신 저장본을 다시 읽어 대상 필드만 병합한다. 편집 중인 데이터의
최종 반영은 저장 여부/현재 디스크 기준 승인을 한 번 확인한다. 교체 전 해시,
백업, 원자적 교체와 실패 시 자기 변경 rollback을 유지한다.

마하라카 lighting DDS와 실제 조명·환경에서 참조하는 공통 입력은 Resources 상대
경로를 유지해 GBResources에 추가한다. 기존 파일을 삭제하지 않고 동일 파일은
다시 쓰지 않는다. JSON·셰이더·실행 파일은 이 리소스 폴더에 넣지 않는다.
원본 설치 위치·전달 위치·파일 수·누락 및 SHA-256 비교 결과를 기록한다.

검증은 변경 JSON parse, 두 domain publisher, runtime/authoring 대응,
RNM 수신 분기와 Level profile 참조, `git diff --check`다. C++ 변경은 다른 빌드와
겹치지 않는 정상 증분 Product Build로 검증한다. 실제 EXE/DLL 잠금 때문에
링크가 실패하면 그때 사용자 종료가 필요함을 알린다. 데이터 게시와 프로세스 종료를
같은 조건으로 묶지 않는다. 화면상 밝기·안개·색의 최종 확인은 사용자가 직접 한다.

## G05. 누락된 흐린 반사 입력의 후속 반영

후속 감사에서 워터팡의 mokomoko/cannon 두 World Object가 기존 정상 TGA 대신 단일 mip
DDS의 HDR/BRDF lookup을 사용함을 확인했다. 사용자는 해당 누락을 전부 반영하도록 승인했다.
최신 디스크 저장본을 기준으로 stable objectId와 expressionIndex를 찾아 네 assetId를
동일한 mip0·색공간의 기존 TGA로 연결하고 revision을 한 번 올린다. Object의 scalar,
거칠기·specular 강도·모델·클립·카메라·타임라인과 모든 렌더링 옵션은 보존한다.

대상 정본은 `Data/Maps/Authoring/LV_OCN_EVENTIS_MHP/LV_OCN_EVENTIS_MHP.worldsequences.json`이다.
현재 WorldSequenceDocument→CMaterial TGA loader→native program1527의 기존 경로를 사용한다.
새 C++/shader나 project/filter 항목은 필요 없다. 현재 마하라카의 다른 World Object,
실제 NPC model override, 일반 배경 및 물/바위 반사도 같은 입력 누락이 있는지 확인하고,
원본 근거가 확인된 대상만 추가한다. 단일 mip라는 이유만으로 모든 물 반사를 임의로 흐리게
만들지 않고 원본 native mip와 sampler 정책을 대조한다.

후보에서 decoded mip0·색공간 동일성, 허용 필드 밖 변경 없음과 공식 WorldSequences
publisher Validate/Publish/Check를 확인한다. 최신 정본·게시본 hash를 재확인하고 백업 및
원자적 교체를 유지한다. 사용자의 이번 반영 승인을 재사용하며 Client/Server 종료나 Reload를
자동 수행하지 않는다. 정상 TGA 두 개와 추가로 복구한 반사 리소스는 Resources 상대 경로로
GBResources에 전달하고 SHA-256을 비교한다. Product 빌드는 사용자의 기존 보류 요청을 따른다.

확장 조사에서 `Data/Actors/NpcCatalog.json`의 MN_ISMP_00/MN_ISMP_00-1 두 모델,
실제 슬롯0·1·2의 총12개 lookup 참조도 같은 결함으로 확인했다. World Object4개와 같은
TGA로 연결하며 NpcCatalog는 Client가 Data 정본을 직접 읽으므로 별도 게시본을 만들거나
Server 수치를 다시 게시하지 않는다.

원본 ambientreflection_01/10/10a는128²8mip, floorcloud_01_d는512²10mip지만 설치된
Maharaka 전용16경로에는 top mip만 남아 있다. 원본 BC payload의 전체 chain을 새 hash
DDS로 만들고 mapmaterials의 native texture 표현식과 mapwater의 동일 참조를 함께 바꾼다.
mapmaterials/mapwater/worldsequences를 기존 Area publisher로 묶어 생성·검증하며 기존
RNM·staticShadow·물의 속도·색공간·반사 강도·배치는 유지한다. 일반 BG8종과 이미 정상인
floorcloud_02_d의 전체 mip는 변경하지 않는다.
