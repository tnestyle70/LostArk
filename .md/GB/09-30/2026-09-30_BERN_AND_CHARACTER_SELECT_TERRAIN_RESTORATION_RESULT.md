# 베른·캐릭터 선택 지형 원본 복원 결과

## G00. 현재 상태

[구현 계획](2026-09-30_BERN_AND_CHARACTER_SELECT_TERRAIN_RESTORATION_IMPLEMENTATION_PLAN.md)의 가시성·geometry·도어 복원과 게시·GBResources 전달을 완료했다. Bern66개(지형42 + 기존 재질 가시성 override22 + 추가 도어2), Character Select7개(지면6 + spotlight1)를 복원했다. 원본 초기 Kismet이 숨기는 SCENE03E45개와 기본 스트리밍에 연결되지 않은 EVENT01 1,127개는 보존했다. 이어 `(250,15,-168)` Landscape 흐림을 원본 UV·반복 텍스처·레이어 혼합으로 수정하고42개 지형에 연결·설치·게시했다. 최종 Bern 배치 수는50,021개로 유지되며 이번 Bern·Character Select 복원의 GBResources 의존성은 중복 제거371개·85,445,734B다. 격리 컴파일·GPU 수치 검증과 설치본 hash 검증은 통과했다. 사용자 요청에 따라 여기까지 반영하고 변경을 마쳤다. 제품 전체 빌드·Client/UI 실행·화면 확인은 수행하지 않았으며 사용자 또는 다른 세션의 빌드 후 사용자 화면 확인이 남아 있다.

## G01. 원본과 현재 상태 대조

Bern 16패키지를 현재 설치 원본 UPK와 `.u` CDO로 재추출했다. StaticMesh 배치 32,324개와 Character Select 803개 모두 현재 authoring에 대응 source ID가 있다. 추출 property error와 unresolved placement는 0이다. Bern 정본 배치 수는 50,019개, Character Select는 804개다. 과거 receipt의 50,017/50,018 수치로 현재 저장본을 덮어쓰지 않았다.

원본 Landscape 42개는 visible이며 설치된 구형 WModel은 tile-local Z가 원본과 반대였다. 기존 hidden 상태는 이 지형 전체를 가렸다. 현행 추출 결과는 원본 166,698개 정점·321,970개 삼각형·463개 hole quad를 보존한다. 최대 위치 오차는 0.0000024414062523m, normal 오차는 0.00000002978583이며 인접 tile edge 70개·표본 4,410개 모두 일치한다. authoring 42개 placement는 새 추출 결과와 stable ID·source ID·asset ID·TRS가 같다.

두 문제 좌표를 현재 설치 WModel 삼각형과 직접 교차했다. `(163,49,-111)`에는 SL09:4655 정적 바닥이 y49.207916에 있고 원본 LAND02:765도 y49.16·hole=false다. `(192,49.19,-136)`에는 SL09:4660/4663/4661 정적 표면이 y49.187045/49.137274/49.117294에 있다. 그 아래 LAND02:766의 원본 quad는 hole=true다. 한 점의 표면 존재나 원본 hole만으로 첨부 화면 전체가 정상이라고 단정하지 않는다. 주위 지형 복원과 사용자 화면 판정을 구분한다. 해당 BG 재질의 sourceFlags 141/269/525에는 alpha-clip bit 64가 없으며 현재 mirrored culling 소비 경로가 있다.

PS의 BSP Model들은 대부분 숨김 volume이고 모든 조사 패키지의 ModelComponent는 0개다. 원본 숨김 volume을 바닥으로 표시하지 않았다. 자세한 source identity와 bounds는 `out/BernTerrainRestore_20260930/source-support/source-audit.json`에 있다.

## G02. 초기 시퀀스 가시성 보존

SCENE03E는 PS export 147의 `EFLevelStreamingAlwaysLoaded`와 actor/component visible만 보면 복원 후보처럼 보이지만, 실제 초기 Kismet이 45개 전부를 숨긴다. `LevelLoaded1197 -> ToggleHidden1191.Hide`는 InterpActor286 1개, `LevelLoaded1200 -> ToggleHidden1194.Hide`는 InterpActor287~330 44개를 대상으로 한다. UnHide는 별도 RemoteEvent/Touch 조건이다. 따라서 SCENE03E 45개와 importer 기본 숨김을 유지했다. 조사 중 생성한 109개 표시 후보와 importer 변경은 철회했고 최종 후보는 64개다.

EVENT01 1,127개는 원본 PS 스트리밍 객체·name table에 연결되지 않는다. 원본의 이벤트 활성 상태가 입증되지 않은 항목을 현재 기본 맵에 일괄 표시하지 않았다. 근거는 `source-support/ps-level-streaming.json`, `source-support/scene-event-control-audit.json`이다.

반복 방지 원리: source actor/component/CDO visible은 가시성의 한 단계다. streaming과 초기 Kismet을 함께 확인해야 하며, AlwaysLoaded와 화면 visible을 같은 의미로 취급하지 않는다. 이름이나 nav 참여만으로 숨기거나 복원하지 않는다.

## G03. 재질 및 리소스 후보

Character Select 315/322/326/327/328/376의 6개 지면은 기존 원본 PBR/overlay와 RNM이 이미 연결되어 있었다. visible 토큰만 변경하며 기존 2mm 깊이 분리 등 TRS는 유지했다. 563의 spotlight는 원본 MI와 기존 program 33이 사용하는 Midnight MI의 부모가 동일함을 재추출로 확인했다. 두 leaf의 native tail은 0이고 원본 additive·two-sided 설정과 4개 parameter를 연결했다. 기존 texture0를 재사용하며 새로운 셰이더나 임의 기본값을 추가하지 않았다.

409는 원본 static mesh의 기본 `enginematerials.defaultmaterial` import가 해결되지 않는 상공 평면이다. WModel은 dummy material만 가진다. 원본 재질이 복구된 것으로 표시하거나 임의 평면으로 대체하지 않고 기존 숨김을 유지했다.

Bern 22개는 구형 `visibilityOverrides`로 숨겨졌으나 현재 source BG/Water39·40/Translucent47·53·63 재질이 연결되어 있다. 실제 WModel material slot 이름과 texture 의존성을 대조하고 해당 stable source ID의 visible 및 importer override만 변경했다. Bern22와 CS7 및 격리한409를 원본 Kismet과 독립 대조해 초기 Hide 경로가 없음을 확인했다. Bern PS 연결 34개 스트리밍 패키지와 PS/LobbyPS/CS00 총 37패키지의 Kismet 파싱 오류는 0이고, import table에서도 후보30개 owner/component에 대한 외부 정확 참조는 0이다. 최종 판정은 `source-support/visibility-initialization-decision.json`, 상세 근거는 `source-support/visibility-candidate-control-audit.json`, `source-support/candidate-external-reference-check.json`이다.

1차 Landscape 후보는 WModel 42개, baked PNG 84개, cliff DDS 82개, 총 208개였다. flat/cliff 분리는 41개, flat-only는 1개다. 원본 cliff DDS의10개 mip은 별도 overlay 후보에서 공급했다. 현재 WModel·재질은 G08의3차 설치본이 정본이며 이1차 후보를 다시 덮어쓰지 않는다.

1차 설치는 기존 `layercliff` 투영·결정적 bake 경로이며 원본 UE3 layered material graph·BRDF·specular/reflection 전체의 동일성을 주장하지 않는다. 새 UV1을 만들지 않았으며 1차 설치 당시 Bern mapmaterials에 Landscape override가 없어 WModel embedded diffuse/normal 경로가 소비한다.

## G04. 후보 검증 증거

| 증거 | 결과 |
|---|---|
| `LV_BER_BERNCASTLE.visibility-audit.json`, `LV_LOBBY_CLASSSELECT_SL00.visibility-audit.json` | 기존 Bern16패키지와 CS00 범위 source ID 누락 0, source/CDO 가시성 전수 기록 |
| `bern-final-candidate-receipt.json` | Bern 64 visible 필드만 변경, 50,019 stable ID/TRS/나머지 byte 보존 |
| `character-select-candidate-receipt.json` | 지면 6개 가시성·resource closure·역변환 검증 |
| `character-select-spotlight-candidate-receipt.json` | 최종 7개 visible + 재질 1행 + catalog 2필드, 동일 base 재질 근거 |
| `source-support/full-cook-candidate-manifest.json` | 지형 42개 topology/winding/463hole·정점·normal 비교, 208리소스 hash |
| `target-installed-static-surfaces.json` | 두 사용자 좌표 및 기존 조사 위치의 설치 삼각형 교차, model read error 0 |
| `restored-resource-delivery-manifest.json` | 복원71배치의 모델·embedded texture·mapmaterials·RNM·shadow 의존성272개, 74,825,718B, 누락0 |

최종 Bern 후보 SHA256은 `04c3efc86579fac6358f29e8b276afea880dd52741b084376d2b58252ee37d87`이다. source SHA256은 `a4f718954dd79d83ee99fd61d991ffb774048dd23acb873d9c3c7027f17c6e22`다. Character Select 최종 placement 후보 SHA256은 `7b66335fab8e9b60702c74616803eee4d98c06df8e4b562772e22ce82dc58260`이다. importer source와 최종 candidate는 SHA256 `ab31b5316baf3d85ae2ba88d1e638e2c4e6f229f9285996e2a768a9e264c3131`로 동일하다. 새 문서의 `git diff --check`와 후보 JSON parse를 통과했다.

의존성 manifest는 208개 Landscape 최종 candidate와 기존64개 리소스를 합친다. WMaterial의 legacy `Resource/` 경로는 `WMaterialReader::ResolveBelowAssetRoot`와 동일하게 Resources 루트 상대경로로 정규화했다. Character Select의 기존 fallback PNG도 실제 파일이 존재하며 누락으로 처리하지 않는다.

## G05. 설치·게시·GBResources 전달

공식 Area publisher를 격리된 `out/BernStage`에서 실행했다. Bern은 50,019배치·24shard·53파일, Character Select는 804배치·5파일을 정상 생성했다. 근거는 `bern-candidate-publish.log`, `character-select-candidate-publish.log`다. 게시 결과와 최신 live 저장본을 비교한 뒤 source hash CAS·백업·원자적 교체로 승인된 변경만 반영했다. 별도의 liveCheck 재실행을 수행한 것으로 기록하지 않는다.

`approved-install-manifest.json`과 `installed-data-and-resources.json`에서 설치 상태 `installed`를 확인했다. 총219파일이 변경됐다. 이 중 Resources205개, Data14개(저작·imported5 + 게시9)다. Landscape208개 중 이미 동일한3개는 교체하지 않았다. 비대상 Bern `.mapeffects.json`과 `.mapmaterials.json`은 publisher와 의미상 같고 서식만 달라 최신 live byte를 그대로 보존했다. 원본 조건부 장면·바다·navigation·조명·사용자 튜닝에 대한 추가 변경은 없다.

`resource-delivery.json`에 따라 `C:/Users/user/Desktop/GBResources`에 실제 의존성272개, 74,825,718바이트를 새로 전달했다. 기존 GBResources 파일 교체는0개이며 설치본과 전달본의 SHA256 일치를 확인한 receipt를 남겼다. 이 전달에는 Landscape208개와 복원 배치에 필요한 기존 모델·재질·RNM·그림자 의존성64개가 포함된다.

## G06. 실행·화면 경계

사용자는 직접 빌드하며 현재 프로세스를 유지하기로 했다. 제품 전체 빌드·실행·Reload·UI 조작은 하지 않았다. G08은 실행 파일과 제품 CSO를 교체하지 않는 별도 C++/shader 검사와 WARP 수치 검증만 수행했다. 수치·파일·publisher 검증을 사용자 화면 확인으로 대신 기록하지 않는다. live 반영, publish, GBResources 전송이 완료되어도 실행 중 메모리 draft나 Server가 자동 갱신됐다고 설명하지 않는다.

## G07. 추가 스트리밍 감사와 독립 도어 후보

원본 PS 스트리밍34개 중 기존 추출 범위 밖20개를 추가 감사했다. 13개는 StaticMeshComponent0개이고 나머지7개에 49개가 있다. `source-support/supplemental-initial-visibility-conclusion.json`은 이49개를 원본 초기 숨김24개, 독립 도어2개, owner 부착·연출 제어23개로 판정한다. 신규7패키지 schema3 추출은 `supplemental-source-placements`에 있으며 property error/unresolved는0이다.

STANDARD_TRACK19개는 CameraActor Base 부착과 native MatineeIndex 배우 연결이 있다. SCENE07B transporter2개도 skeletal owner와 함께 Touch→Matinee loop Play/Stop·ToggleUnHide를 소비한다. SCENE04B760은 CameraActor 부착 소품이며 부모 카메라의 초기 Hide가 있다. SCENE07E1410은 RemoteEvent 연출 전환용 collision 없는 배경판이다. 이23개의 부착·재생 소비자가 복원됐다고 주장하지 않으며 source-visible을 강제로 false로 바꾸거나 독립 고정 지형으로 표시하지 않았다. 근거는 같은 conclusion 및 `source-support/standard-track-independent-audit.json`이다.

SCENE07B355/356은 독립 도어이며 초기 Hide·Base·직접 또는 외부 Matinee/Toggle 연결이 없다. 현재50,019개 전체에서 source ID와 stable ID 중복이0이고 같은 원본 door01 WModel의 기존 두 배치는 최소174.89m 떨어져 있다. 원본 위치는 각각 `(229.6727734375,51.21486328125,-118.27634765625)`, `(232.26083984375,51.21486328125,-115.6883203125)`다. 첫 문은 원본 음수X scale을 유지한다. `phase2-door/duplicate-and-trs-audit.json`에 원본→runtime 변환을 기록했다.

`SCENE07B.component-lighting.json`은 두 문을 포함한4개 component 전부 `NO_LOD_LIGHTING_DATA`, 오류0으로 기록한다. 도어 원본 default 재질은 기존 SL04 override와 specular 값이 다르므로 RNM 없는 WModel 공유 variant1개를 별도로 준비했다. 정식 parameter extractor가 원본 native3260바이트의 static set을 해석했고 공용 `source_map_surface.build_surface`로 flags205/specular3/power60인1행을 생성했다. D/N/S의 원본/runtime mip 수는11/11/10이며 D/S는 원본 CDO의 SRGB=true, N은 instance의 false다. fresh UModel mip0의 압축 블록과 RGBA도 기존 DDS와 일치했다. 이 검사를 하위 모든 mip의 원본 재디코드 비교로 확대해 기록하지 않는다.

`phase2-door/candidate-receipt.json`의 5개 후보는 1차 설치 이후 source hash에 고정했다. Authoring은50,019→50,021배치, Imported MaterialExtra00은 catalog8→9 및 baseline0→2이며 mapset의 해당 count와 mapmaterials 신규1행만 추가한다. 기존50,019개 행과 모든 material/placementLighting은 보존됐고 신규 stable ID 충돌0, 원본 TRS 최대 직렬화 차이0.000001m 미만, 실제 WModel slot0 이름 일치, resource 존재/hash 검증을 통과했다. 모델과 native/embedded texture 의존성7개·5,220,380B는 기존 설치본이며 새로운 RNM/static shadow나 Resources 파일을 만들지 않았다. 후보 placement SHA256은 `fdba76562ca82e6d84f79d8b98b8559f3af900bf91b9a2dc513c8d44d0fa512c`다.

2차 도어도 설치·게시를 완료했다. `doors-phase2/candidate-publish.log`의 공식 격리 Area Publish는50,021배치·24shard·53파일 PASS이며 `installed-data.json`은9파일(정본5 + 게시4) 변경을 기록한다. `final-verification.json`에서 stage112파일 비교, 기존50,019배치 보존, 신규2배치, variant1개, diff-check PASS를 확인했다. live 전체 Area Check를 반복 실행한 것으로 기록하지 않는다. `doors-phase2/resource-delivery.json`은 GBResources7개·5,220,380B 추가와 SHA 일치를 기록한다. 1·2차 누계는 Bern66/CS7, GBResources279개·80,046,098B다.

두 source ID는 Authoring뿐 아니라 Imported baseline에도 저장해 반복 publish에서 유지한다. 구형13shard 전체 재생성은 현24shard 정본의 갱신 절차가 아니며 전체 재추출 시 기존 baseline·Authoring stable ID를 병합해야 한다. SCENE07B 전체 include나 importer·runtime 코드 변경은 없다.

## G08. 사용자 화면의 Landscape 흐림과 원본 painted 재질 수정

`(250,15,-168)`의 바닥은 LAND02 LC762, asset `MAP_110E3C287A41_LAND02_LC_00762`다. 원본 높이15.76m와 지형은 일치하지만 39.68m 타일을256px로 굽는 파생 재질은15.5cm/texel로 세부 무늬를 잃었다. 원본 UV는 grid×0.1을 중심 회전(source scalar×3.1400001049)한 뒤 tiling을 적용한다. 기존 /62·degree 경로는 layer06의 반복22.32회를3.6회로 확대했다. 이 원인을 단순 upsample이나 전역 mip bias로 덮지 않았다.

LAND01/02 ShaderCache와 component material의 native static key를42개 모두 맞췄다. 최대6개 painted layer와2개 weightmap을 사용하며, 원본 PS에 layercliff 샘플은0개다. height와 paint 정규화는 diffuse/specular와 normal에 서로 다르게 적용된다. 원본 sample normal RG의 UV 회전을 다시 normal XY에 적용하지 않고 Heightmap BA로 pixel basis를 복원한다. specular는 diffuse tint·brightness 적용 전의 desaturated D와 원본 specular 색/강도를 사용한다. 직접광 marker14는 saturate(N·H)와 RGB specular uncapped branch를 사용하며 다른 family의 기존 계산은 보존한다.

`CModel -> CMaterial`의 map surface에 `SOURCE_LANDSCAPE_OPAQUE` family14를 추가했다. typed 입력은6층/2weight/1height로 제한하며 JSON/publisher/CModel에서 범위·경로·중복·unused weightmap·미지원 입력을 거부한다. 기존 load/stage/commit을 재사용하며 새 renderer나 별도 모델 타입은 없다. Engine/Client 기존 H·CPP7개, Client 프로젝트·filters2개, 기존 mesh/Deferred shader와 신규 Client HLSLI가 변경 대상이다. normal basis는 ordinary/instanced VS의 실제 inverse-transpose 축으로 전달하며 양쪽 draw를 지원한다.

정본 builder는 `Tools/LandscapeExtractor/build_source_landscape_candidate.py`, 원본 해독 계약은 `Tools/LandscapeExtractor/SourceContracts/BernSourceLandscape.v1.json`이다. SourceRaw와 source contract 및 원본 mip closure 또는 설치된 package/UModel로 후보를 재생성한다. LC762 단독·42개 일괄·LC762 원본 재추출 결과는 동일 리소스/row bytes다. geometry는 painted1슬롯으로 재생성하며 기존321,970삼각형·463hole·winding·공간 위치를 보존했다. 최대 위치 차이2.4414063e-6m이고 모든 면의 UV0=component grid/62를 확인했다. 배치 ID/TRS 변경은0이다.

후보는 WModel42개와 DDS92개(공유D/N8 + weight42 + height42), 총134개·19,862,100B다. WModel41개는 UV/slot 구성이 바뀌고 flat1개는 byte 동일하다. DDS는 원본 압축 또는 BGRA payload와 각 source mip chain을 보존했다(weight7mip, height5mip). 기존 embedded PNG84개는 unchanged dependency로 유지하고 typed 재질은 새 원본 레이어 DDS를 소비한다. 이 구현은 원본의 전체 RNM/SH/environment 연결까지 완료했다는 뜻은 아니다.

| 검증 | 실제 결과와 경계 |
|---|---|
| C++ 격리 검사 | 최종 Debug4개 translation unit `/Zs` PASS, SDK·제품 EXE/DLL 변경0. Release 초기 검사도 PASS |
| Shader 격리 검사 | ordinary/instanced VS/PS, Deferred directional/point 총6 entry FXC PASS. 신규 helper warning0, 기존 경고는 수정 전과 동일 |
| 원본 DXBC 대조 | LC762 원본 PS와 실제 product helper를 D3D11 WARP에서40개 결정적 texture sample로 비교, 색/normal/specular/power 최대절대오차2.3841858e-6 |
| GPU 수치 대조의 한계 | constant sample로 혼합 계산을 확인했으며 공간 UV·mip filtering·실제 Client 화면 PASS를 뜻하지 않음 |
| 원본 LOD0 좌표·경계 | LC762 UV register168개 float32 비교 오차0,42개 subsection weight/height 좌표2,352개 오차0. Height42+active weight55의 원본 경계 bytes 일치. 거리 LOD morph·mip derivative·원본 VS 전체 실행 대조는 미수행 |
| 원본 리소스·형상 | 독립134개 hash·원본 BGRA/압축 mip·321,970tri/463hole/UV0/winding 대조 PASS |
| Publisher focused test | 실제 single/sharded Area 게시, 잘못된 입력 거부 및 기존 runtime snapshot 보존3test PASS |

근거는 `source-support/landscape-runtime-final-review.json`, `landscape-shader/native-warp-receipt.json`, `landscape-shader/native-candidate-independent-review.json`, `source-support/native-bern42-candidate/candidate-manifest.json`이다. 첫 focused 실행에서 테스트 클래스를 import해 기존 material test도 수집됐고 duplicateMaterialName 한 케이스가 실패했다. 수정 전 publisher로도 같은 실패가 재현되어 이번 family14 회귀가 아님을 `landscape-native/existing-test-baseline.log`에 남겼다. focused suite의 import 범위를 바로잡은 최종3test는 통과했으며 기존 타 family의 중복 정책은 바꾸지 않았다.

weight/height sampler에는 subsection31/32의 중복 texel 경계가 잘못된 mip derivative를 만들지 않도록 연속 logical grid의 기울기를 `SampleGrad`에 전달했다. 마지막 수정 후 ordinary/instanced PS와 helper를 FXC로 다시 검사하고 WARP40건도 재실행해 같은 최대 오차로 통과했다. `landscape-shader/final-source-shader-review.json`에 최종 hash·컴파일·대조 결과를 고정했으며 Engine/Client Deferred 파일은 byte 동일하다. 공간 mip filtering과 실제 Client 화면을 검증했다는 뜻은 아니다.

공식 Area publisher의 첫 격리 실행은 stage에 read-only Effect catalog 연결이 없어 중단됐고 제품 파일은 바뀌지 않았다. 기존 도어 stage와 동일한 Actors/Effects/Worlds/Navigation 읽기 연결을 준비한 뒤 공식 Area Publish를 재실행해 PASS했다. `landscape-native/candidate-publish.log`는50,021배치/24shard/53파일을 기록하며 최종 mapset SHA256은 `d11bb2050ea951ceb7d46a4b8665243e1aa20141b8be12219e3d4ed34b2d6862`다.

`landscape-native/installed-data-and-resources.json`에 따라 최신 저장본 hash CAS·백업·원자적 교체를 완료했다. 제품 변경은135파일(WModel41 + DDS92 + 저작·게시 mapmaterials2)이며 동일 WModel1개는 보존했다. mapmaterials는 기존23,158행을 유지하고 typed 지형42행만 추가해23,200행이다. 의미상 동일한 비대상 mapeffects의 최신 byte는 유지했다. 배치·TRS·렌더링 옵션의 이번 변경은0이다.

`landscape-native/resource-delivery.json`은 GBResources의 신규 DDS92개, 변경 WModel41개, 동일 WModel1개를 기록한다. `landscape-native/final-verification.json`은 stage112파일, 신규 material42행,134개 리소스와 기존 PNG84개, Bern·Character Select 누계371개·85,445,734B의 실제 설치본/전달본 hash 일치를 모두 PASS로 기록한다. 이 누계는 GBResources 폴더 전체나 Maharaka 전달분을 포함하는 수치가 아니다.

빌드 인계: 현재 작업본의 Engine·Client와 관련 셰이더를 함께 빌드해야 새 family14 재질을 소비한다. 이 세션은 전체 빌드·링크·SDK 또는 제품 CSO 배포를 수행하지 않았다. 이미 실행 중인 Client는 새 코드를 자동으로 갖지 않는다. 사용자 화면 확인 대상은 `(250,15,-168)`의 반복 무늬·흐림, 앞선 두 좌표의 지형, Height Fog 토글과 용 탑승 시 조명이다. 안개·탑승 변경의 별도 근거는 [해당 RESULT](2026-09-30_BERN_FOG_AND_MOUNT_LIGHTING_RESULT.md)다. 화면 PASS와 전체 조명 복원 완료를 이번 수치 검사로 대신하지 않는다.
