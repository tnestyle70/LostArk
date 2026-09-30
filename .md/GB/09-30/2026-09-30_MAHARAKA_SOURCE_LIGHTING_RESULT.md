# 마하라카 원본 환경·조명 연결 결과

## G00. 실제 적용 상태

마하라카 전용 `scene.maharaka.source-day.v1`을 추가하고 기존 개발용 neutral profile
참조를 교체했다. 원본 전역/섬 volume의 태양광·안개·후처리·LUT, local33광원의
RNM 중복 수신 방지와 원본 스포트라이트 방향을 저작 정본·게시본에 반영했다.
다른27 profile과 globalQuality는 변경 전 값과 같고 RenderingProfiles revision은85→86이다.

소스·데이터 연결과 실제 실행 파일의 상태는 다르다. 당시 Product 빌드는 실행 중인
Debug Client PID61832/Server PID84960 때문에 preflight에서 중단됐으며 컴파일은
에이전트가 실행하지 않았다. Client/UI를 자율 실행하거나 화면 확인하지 않았다.
이후 사용자가 직접 빌드와 검토를 진행 중이라고 알렸다. 빌드 성공 및 화면 판정 결과는
아직 전달받지 않았으며, 에이전트가 별도 빌드나 프로세스 종료를 수행하지 않았다.

## G01. 원본 재조사로 확정한 입력

원본은 `C:/ProgramData/Smilegate/Games/LOSTARK/EFGame/ReleasePC`다.
PS=`423OPDI3SR2RIOJH3DBCW3IWH.upk`, SL01=`645QRFK5UT4TKQLJ5FDEY5KJ63A.upk`,
LAND01=`867STHM7WV6VMSNL7HFG07M83MO5C.upk`를 읽었다. Engine/EFGame의
CDO와 `efpostprocess.postprocesschain.defaultscenepostprocess`도 함께 합성했다.

| 입력 | 실제 연결 |
|---|---|
| PS45 directional component | property stream byte835592~836217, 전역 밝기2.2·색255/254/203 |
| PS44 방향 | UE rotator를 기존(x,z,-y) basis로 변환, 방향(-.5491882,-.7776123,.3061231) |
| PS107 전역 안개 | density.1, heightFalloff.7, maxOpacity.2, start16m, height19.064862m |
| PS54 섬 volume→PS53 environment | DDL1.4, fog density.1/heightFalloff3, 원본 convex6면·8정점 |
| 섬 volume 산란 밝기 | EFGame EnvironmentInfoData CDO의1, 전역 component의.5를 상속하지 않음 |
| 전역 후처리 | bloom threshold.4/intensity1, tint255/178/159, colorize(1,1,1.15), SSAO power1.2 |
| 섬 후처리 | threshold.5, midtone(.9,1,1), override=false인 tint·LUT·colorize는 전역 값 유지 |
| LUT | `Map/Lighting/Maharaka/lv_ocn_dookyis_lut.dds`, 원본 override=true |
| 셀피 환경 | PS68의 `selfiemodeenvironmentinfo`→PS101. 일반 환경에 적용하지 않음 |

원본 volume 정점/plane의 최대 오차는4.2e-6m다. 기존G05 보고서가 읽지 못했던
directional component의 native prefix 이후 property와 SkyLight 상단 밝기.3·색255/230/221를
실제로 회수했다. 기존G05의 상단 하늘광 미직렬화 추정은 이번 원본 실측으로 교정한다.

WorldInfo의 toneToe override=true와 CDO toe1이 실제 postprocess chain의 toe.5를 덮는다.
toneScale1·range8도 기본값 근거를 확인했다. sourcePP가 없는 프로젝트의 FXAA·exposure·
gamma·SSAO/bloom enable 등 무관한 설정은 현재 저장값을 유지한다.

재현기와 원본 dump/근거는 `out/MaharakaEnvironment20260930/source-scene/`의
`extract_environment.py`, `build_scene_candidate.py`, `source-provenance-result.txt`에 있다.
원본 package는 수정하지 않았다.

## G02. 환경광과 local light의 소비 경계

SL01 StaticMeshComponent3823개 중 RNM3752, NoLOD71이며 공용 추출기 실패/미지원은0이다.
점광원32개·스포트라이트1개의 lightmapGuid는 RNM에 실제 포함돼 있다. 따라서 기존33행의
stable ID·위치·색·밝기·반경·falloff·cone을 유지하고 receiver만 UNBAKED로 바꿨다.
스포트라이트3634의 cachedParentToWorld는 runtime 방향(0,-1,0)을 가리키므로 그 행만
rotationDegrees를(0,0,0)→(90,0,0)으로 교정했다.

Sky GUID는 RNM3752개 전부에 포함되며 Dynamic/CompositeDynamic channel은false다.
정적 하늘광을 별도의 동적광으로 다시 합산하지 않는다. dominant directional의 RNM GUID는
0회여서 해당 방향광은 ALL receiver를 유지한다. 기존 Deferred와 forward light upload가
UNBAKED 표면 판정을 처리하므로 새 shader나 renderer는 추가하지 않았다.

현재 ambient는 원본 Lightmass EnvironmentColor(242,244,255)×Intensity1×
CharacterLitIndirectBrightness1을 기존 Bern/Kouku의 scene float4 방식으로 변환했다.
값은(.9490196,.9568627,1,1)이다. RNM을 가진 source BG/stone/PBR 표면은 기존 shader가
scene ambient를0으로 차단하고, 굽지 않은 표면/캐릭터만 이 값을 받는다.
이는 프로젝트 adapter이며 원본 dynamic LightEnvironment SH·상하반구·대비 계산의
동일성을 복원한 것으로 기록하지 않는다.

## G03. 원본 정적 그림자

SL01 ShadowMap2D3027개와 atlas52개, LAND01 ShadowMap2D10152개와 atlas1113개의
원본 참조를 조사했다. 현재 RNM/UV1 연결이 있는2822배치 중 ShadowMap2D가 있는
2267배치를 원본 atlas·GUID·UV scale/bias로 연결했다. shadow가 없는555배치는 보존했다.

모든 shadow의 lightGuid는 `5d9093ae8f7f5f438eae4d867e63b073`이며 원본 dominant
directional과 같다. 기존 main directional의 static lightChannel1에 연결하고 직접광만
감쇠한다. 원본 G8 DDS48개의 전체 mip을 복원했다. 원본 CDO shadowExponent2를 사용하며
penumbraWidth.05는 기존 Bern SDF transfer의 명시적인 PROJECT_ADAPTER다.
실시간 shadow.enabled는 현재false를 유지하고 정적 shadow channel과 구분한다.

같은 asset의 다른 atlas와 shadow가 없는 소비자를 보존하기 위해 variant290개를 추가했다.
catalog871→1161, material963→1319(그중 shadow1062, RNM1287), 배치4671개 중
1159개는 assetId만 바뀌었다. 모든 기존 catalog행·재질 물성·RNM 계수/UV·배치 ID/TRS/
visibility를 보존했다. variant는 같은 WModel을 공유하므로 geometry를 다시 cook하지 않았다.
Imported placement도 대응 assetId로 맞췄고 MapCatalog의 오래된 개수 메타데이터를 교정했다.

독립 검토에서2267개 원본 ShadowMap2D export를 다시 파싱해 texture/GUID/UV 전부 일치했고
48 DDS의 header·mip별 hash·설치본이 일치했다. 근거는
`out/MaharakaEnvironment20260930/source-scene/shadow-independent-review.json`이다.
짧은 격리 검증 경로에서 Area Validate도 PASS했다. 첫 긴 검증 경로의 reflection 누락은
MAX_PATH에 따른 File.Exists 실패였으며 기존 texture를 교체하지 않았다.

원본 package 전체의13179개 shadow나 미설치 레이싱 트랙을 모두 runtime에 넣은 것은 아니다.
현재 native Landscape 조명 미연결과 미지원 재질의 복원 범위는 확장하지 않았다.

## G04. 리소스 전달

`C:/Users/user/Desktop/GBResources`에 Resources 상대 경로 그대로 DDS204개,
23,096,500 bytes를 추가했다. 기존 조명137개(RNM136+LUT1), 재질 반사19개,
새 정적 shadow48개(9,448,788 bytes)다.
기존 파일 교체·삭제0, 설치본과 전달본의 크기·SHA-256 일치204/204다.
DDS magic·형식·전체 mip payload 검사와 현재 runtime texture 참조 누락0을 확인했다.

실제 참조 RNM92개는6~11단 mip chain이다. 미참조 RNM44개는 기존 top mip만 있는 파일이며
이번 복사를 해당44개 원본 mip 재복구 완료로 기록하지 않는다. 최종 resource receipt는
`out/MaharakaEnvironmentRestore_20260930/resource-delivery.json`, 최초 추가 증거는
`resource-delivery.initial.json`, shadow 추가 증거는 `resource-delivery.shadow-install.json`이다.
JSON·CSO·실행 파일은 GBResources에 넣지 않았다.

## G05. 실행한 검증과 미완료

- RenderingProfiles 후보 Validate 및 정본 Publish PASS. 저작본/게시본 의미 동일.
- MapAuthoring `-AreaId LV_OCN_EVENTIS_MHP -Scope Lights` Publish/Check PASS.
- 정적 shadow 포함 Area Validate/Publish/Check PASS:4671배치,7개 게시 출력.
  로그는 `out/MaharakaEnvironmentRestore_20260930/area-publish.log`, `area-check.log`다.
  정본 mapassets v4→게시 v5의 material binding 추가를 확인했고 material/light/placement
  내용은 대응 정본과 일치한다. shadow 게시로 바뀐 내용은 catalog/material/placement3개이며
  camera/worldsequence2개는 publisher의 CRLF→LF 정규화만 있어 Git diff는 없다.
- 기존27profile/globalQuality 동일, local33개 중 변경 필드는 receiver와 spot1개의 회전뿐.
- Rendering publisher의 round trip·환경 오류 rollback·quality override 오류 rollback3개 PASS.
- 변경 코드/데이터 `git diff --check` PASS. LF→CRLF 안내는 내용 오류가 아니다.
- Debug Product: `out/BuildPipeline/runs/20260930T120957565Z-debug-product.json`에
  실행 파일 점유로 preflight FAIL. compile/link/shader 컴파일 실행0. 사용자 요청으로 후속 빌드는 보류.
- 사용자 화면 확인 미실행. 원본 dynamic SH·DOF·motion blur·셀피 전용 기능의 신규 renderer는
  이번 연결에 포함하지 않았다.

core 적용 백업·해시·검증 기록은 `out/MaharakaEnvironmentRestore_20260930/backup`,
`apply-receipt.json`, `core-verification.json`에 있다. 최신 디스크 내용을 다시 읽고
hash 재확인·원자적 파일 교체로 변경했으며 무관한 미커밋 변경은 보존했다.
dirty 작업 폴더에서 자동 commit/push하지 않았다.

## G06. 사용자의 다음 실행

현재 실행 중인 Client/Server는 유지했다. 사용자가 편집 내용을 저장하고 직접 종료한 뒤
`powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug`
로 정상 증분 빌드하면 새 LevelRegistry 연결을 실행 파일에 반영할 수 있다.
기존 실행 파일은 아직 개발용 neutral profile을 참조하므로 데이터 게시만으로 실행 중
화면에 새 scene이 자동 적용됐다고 판단하지 않는다. 빌드 후 Server/Client를 사용자가 시작하고
Debug Lobby의 Maharaka로 진입해 섬 조명·안개·색·정적 그림자를 확인한다.

## G07. 후속 반사 lookup mip 감사: 수정 전 누락 확인

사용자가 문의한 TGA full mip 미연결에 따른 거울 같은 반짝임을 현재 저작본·게시본·
설치 texture·실제 shader 소비자까지 읽기 전용으로 대조했다. 이번 조명 복원은 기존963개
material의 staticShadow 외 모든 필드를 보존했으므로 반사 texture나 거칠기 수정은 포함하지
않았다. 일반 BG의 reflectionTexture8종은8~11단 전체 mip가 있지만 이것만으로 맵의 모든
표면에서 같은 결함이 없다고 판단할 수 없다.

WorldSequences의 `world.object.maharaka.waterpang.source.intro15.mokomoko`와
`world.object.maharaka.waterpang.source.intro15.cannon`은 모두
`source.character.maharaka-ismp-2.v1`을 사용한다. 두 object의 expression3은
`Character/SourceMaterials/efmaster_material_prologue/hdr07_1.dds`(srgb), expression6은
동일 폴더의 `brdf_beckmann_spec.dds`(linear)를 여전히 참조한다. 두 DDS는512×512에
실제 mip1개이며, 기존 정상 TGA의 decoded mip0 RGBA와 각각 byte-exact equal이다.
기존 CMaterial TGA loader는 이 크기에서10단 mip를 생성한다.

family의 native program1527은 roughness_power3과 ibl_reflect_lodbias80 및 화면 미분으로
LOD를 계산한다. `Shader_SourceCharacterBaseGroup1472.hlsli`의 해당 program은 expression3
HDR을 실제 `SampleLevel`로 읽으므로 양수 LOD가 단일 mip DDS의 mip0으로 고정되는
기존 Movie 결함과 같은 입력 누락이 남아 있다. 두 object의 네 texture 참조를 정상 TGA로
연결하는 수정은 아직 적용하지 않았다. 수치/파일 감사이며 사용자 화면 판정은 아니다.
두 object의 specular alpha는 전 픽셀71/255이며1080p 내부 quad 기준 식 계산은 요청 LOD9,
현재 DDS의 유효 mip0, 기존 TGA의 유효 mip9다. 실GPU 측정으로 기록하지 않는다.
해당 Character lookup 두 TGA는 설치 Resources에는 있지만 이번 GBResources 전달에는 없다.

반사 전달19개 중 물/바위용11경로의 단일 mip 여부는 별도로 확인했지만, 원본 mip 정책과
해당 sampler 소비를 모두 확정하지 않았으므로11개 전부를 동일 결함으로 단정하지 않는다.
리소스 전달의 DDS payload 검사 PASS도 원본의 모든 mip 복원이나 반짝임 해소 증거가 아니다.
감사 근거는 `out/MaharakaEnvironmentRestore_20260930/reflection-lookup-resource-audit.json`에
있다. 이 후속 감사에서 runtime 데이터·Resources·shader 변경과 빌드/Client 실행은0이다.

## G08. 사용자 승인 후 반사 입력 전체 반영

사용자의 후속 `그래 다시 전부 반영해보자` 요청에 따라 G07의 누락을 수정했다. 범위를
현재4671배치·431개 사용 WModel·NPC22모델·379개 native texture 소비자까지 확장해
확인했다. World Object2개의4참조 외에 NpcCatalog의 MN_ISMP_00/MN_ISMP_00-1,
실제 슬롯0·1·2의6개 재질에서도12개 lookup 참조가 추가로 누락돼 있었다.
총16개 assetId를 기존 정상 HDR/BRDF TGA로 연결했다. WorldSequences revision은9→10이며
NpcCatalog에는 revision 필드가 없다. 두 TGA의 mip0 RGBA·색공간은 기존 DDS와 동일하고,
기존 loader에서10단 mip를 만든다. 이 경로의 program26/1528/1527은 기존 shader 그대로다.

물/바위의 ambientreflection_01/10/10a와 floorcloud_01_d를 원본 package에서 다시 읽었다.
128² DXT1 세 종류는8단,512² DXT5 구름은10단의 압축 mip가 실제로 존재한다. 기존 mip0와
전체 원본 BC payload를 한 바이트도 바꾸지 않고 DDS16경로·2,547,336 bytes로 복원했다.
처음 조사한 반사11경로 외에 native texture_sky에서 쓰는 구름5경로도 포함한다. 신규 SHA12
파일명으로 설치하고 mapmaterials25참조·mapwater10참조만 새 asset ID로 연결했다.
mapwater revision은2→3이다. 정상 BG8종과 floorcloud_02_d는 유지했다. 기존 단일 mip 파일,
공유 DDS·shader, 재질의 색공간·거칠기·반사 강도, RNM·staticShadow·배치는 변경하지 않았다.

정본4개(NpcCatalog·mapmaterials·mapwater·worldsequences)와 공식 Area publisher가 생성한
runtime3개를 최신 hash 재확인·백업·원자적 교체로 반영했다. NpcCatalog는 Client가 Data
정본을 직접 읽으므로 별도 게시 복사본이나 Server bootstrap 갱신은 없다. 이번 보완의
GBResources 추가는 DDS16+TGA2=18개·3,790,904 bytes이며 설치본 SHA와 모두 동일하다.
기존 GBResources 파일 교체·삭제는0이다.

실행 검증은 WorldSequences 후보 Validate, 전체 Area 후보 Publish, 실설치 Area Check,
정본/게시본 JSON 의미 일치3쌍, source 압축 mip 독립 재추출 대조와 허용 필드 외 보존 검사다.
모두 PASS이며 설치 후 조사한27개 반사/lookup 경로에서 단일 mip 누락은0이다. 정상 TGA는
파일 내 mip가 아니라 기존 CMaterial loader가 생성하는 full mip 입력으로 집계했다.
검토한 mapslots158개에 새 native 재질을 복원했다거나 임의의 미지원 원본 재질까지 모두
완료했다고 확대하지 않는다. 이 특정 반사 입력 범위의 복구이며 최종 화면 판정은 남아 있다.

근거는 `out/MaharakaReflectionFix_20260930`의 `installed.json`, `final-verification.json`,
`resource-delivery.json`, `live-area-check.log`, `water-independent-source-review.json`과
`audit/reflection-input-audit.after.json`이다. 이전 G07의 미수정 상태는 이 G08에서 해소됐다.
이 보완은 데이터·리소스 변경만으로 C++/shader 재컴파일이 필요 없지만, 앞선 환경 profile
연결의 제품 빌드는 사용자가 직접 검토 중이며 성공 결과는 아직 확인하지 않았다. 실행 중 Client/Server의 메모리
캐시와 미저장 draft는 자동 Reload하지 않았으며 화면에 즉시 적용됐다고 기록하지 않는다.
