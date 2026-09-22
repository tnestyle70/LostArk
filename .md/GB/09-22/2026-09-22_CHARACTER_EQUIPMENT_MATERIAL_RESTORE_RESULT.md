# 기본 캐릭터·선택 의상·무기 재질 연결 결과

## G04. 현재 반영 범위

사용자가 첨부한 첫 번째 Slayer, 두 번째 GunSlinger의 기존 기본 외형 경로를 유지하면서
CharacterCatalog의 두 entry에 native material descriptor를 연결했다.

| 대상 | 설치 기본 모델 | native material slot |
|---|---:|---:|
| GunSlinger | 7 | 14 |
| Slayer | 7 | 17 |

모델별 범위는 본체, Upper, Lower, Arm, Shoulder, Helmet과 기존 기본 weapon이다.
다른 캐릭터와 두 entry의 body/equipment/weapon/AnimSet 목록 및 presentationScale은 보존했다.
기존 계획 G01의 선택 의상 전체332모델 작업을 이번 기본14모델 결과로 완료 처리하지 않는다.

## G05. 원본 입력과 런타임 연결

실제 본체는 `PC_GN_F_00.mesh.pc_gn_f_00_sk`, `PC_WR_F_00.mesh.pc_wr_f_00_sk`다.
GunSlinger 얼굴은 `PC_DL_00_FACE`, Slayer 얼굴은 `PC_WR_F_FACE`의 실제 mesh 참조를 따른다.
Slayer와 같은 material basename을 가진 `PC_WBK_F_00.mat_high`의 다른 MIC를 선택하지 않았다.
원본 skin/variation/head/eye/eyelash/hair/weapon native shader와 uniform packing을 대조하여
기존 program1/2/4/5/6/7/14/90/91/99를 재사용한다.

GunSlinger Arm은 원본 Base `0b3be796bcf5b144a1761c47690de70a`,
Light `c5df3fee8e45da48bd2c274cccd1ca3a`의 별도 masked permutation이다.
기존 translator로 program112, `source.character.classic-armor-skin-masked.v1`을 생성했다.
기존 SourceCharacter cohort84, Model admission, Shader의 program 선택 범위를112까지 연결했다.
기존 program110/111의 병행 수정은 보존했다. 새 C++ 파일이나 프로젝트 등록은 없다.

두 WModel에 원본 눈 UV1/UV2와 머리 UV1을 추가했다. 원본과 설치 모델의 position/UV0 전체
join은 GunSlinger12,565정점, Slayer14,130정점에서 누락0이다. Slayer 머리의8개 seam 정점은
동일 position/UV0만으로 구분하지 않고 원본 triangle corner로 UV1을 확정했다.
기존 geometry·skin weights·skeleton 및 GunSlinger187개/Slayer208개 animation section은
byte 그대로 보존했다. source texture106개를 native expression index와 sRGB/linear flag에 연결했다.
기존 공유 TGA가 압축 형식만 다른 경우 decode한 전체 RGBA가 동일함을 확인하고 기존 bytes를 유지했다.

MIC 추출기의 expression-count 검사는 실제 중복 참조를 거절하고 있었다.
12개 texture expression이10개 reference를 재사용하는 얼굴을 위해 개수 비교를 제거하고,
fallback이 필요한 각 referencedTextureIndex의 범위를 검사하도록 고쳤다.

## G06. 설치와 전달

`Data/Actors/CharacterCatalog.json`의 최신 디스크 두 entry만 stable model/material key로 병합했다.
두 body의 candidate에서 복구한 legacy bytes가 교체 직전 디스크와 같은지 확인하고,
백업·hash 재확인·원자 교체로 설치했다. 실패 시 자기 변경만 복구하는 절차를 사용했다.

Resources의106 texture와2 WModel, 총108파일322,360,337bytes를
`C:/Users/user/Desktop/GBResources`의 같은 상대 경로에도 복사했다. 기존 값이 다른 전달 파일은
별도 백업했다. Drive 업로드·실행 중 Client의 메모리 reload를 수행한 것은 아니다.
리소스와 컴파일 산출물은 소스 Git 변경에 넣지 않는다.
사용자의 GBResources 전달 요청 후 설치 Resources와 전달 폴더의108파일을 다시 읽어,
모든 SHA-256이 서로 같고 설치 당시 receipt와도 같은 것을 확인했다.
재확인 시각·상대경로·개별 hash·bytes는 `delivery-verified.json`에 기록했다.

증거는 `out/DefaultOutfits20260922/`의 `target-materials.json`, `uv-receipts.json`,
`texture-install.json`, `install.json`, `delivery.json`과 `before-install/`에 보존했다.

## G07. 검증과 남은 확인

- native program112의 Base667행·Light737행·CPU packing을 원본 dump에서 재생성해 byte 일치를 확인했다.
- 18:46 공용 parameter header 갱신 후112만 다시 원본 dump에서 메모리로 생성했다. 16:58 보관본,
  현재 Engine/Client 함수와 최신 header의 CPU packing이 모두 같은 SHA-256이며 원본 PS ID와
  named-vector 배치도 일치한다. 실제 파일의 bytes/mtime는 변경하지 않았다.
  증거는 `program112-current-readonly.json`과 `.log`다.
- 기존 source shader partition 검사3개를 통과했다. 미등록 번호를 검사하는 fixture는 이전에 이미
  등록된110을 사용하고 있었으므로 현재 마지막 등록 번호 다음 값을 검사하도록 고쳤다.
- 31개 descriptor의 실제 C++ parameter Configure, texture expression mask와 설치 파일을 검사했다.
- 설치 전 구 Engine으로 GunSlinger 본체7재질의 CModel 생성·clone·잘못된 material slot 거절과
  기존 clone 보존을 확인했다. Arm112는 구 Engine의111 상한으로 예상대로 거절됐다.
- static mesh cohort84의 독립 FXC는 성공했다. Animated/Deferred의 독립 컴파일은 Product 빌드와
  중복되어 중단했고, 이를 성공으로 기록하지 않는다.
- catalog의 무관한 캐릭터·필드 보존, JSON parse, Engine/Client source shader4파일 byte동일,
  변경 파일 `git diff --check`를 확인했다.

첫 Product 시도는 Engine/Shared/Server까지 성공했으나, 병행 작업의 MapStaticBatchObject가
이전 EngineSDK의 Profiler enum을 읽는 시점 불일치로 Client 컴파일에서 실패했다.
해당 로그는 `out/CharacterSizeSave20260922/product-build.log`다.
동시에 다른 재질 작업이160~210 cohort를 공통 소스에 추가했다. 본 작업의112·084와
설치 catalog hash는 유지됐으며112 재생성·4파일 mirror 재검사는 통과했다.
추가 cohort 직후 공통 partition3검사는2개 실패(no-op 재작성)였다. 앞선3개 통과와
구분하며, 타 작업을 되돌리거나 shader를 재생성하지 않았다.
새 Engine을 사용하는 화면 없는 WARP consumer에서 실제 ActorCatalog의
Build_ModelLoadDescription과 설치된 Resources를 읽어14모델·31재질의 CModel 생성·clone,
14개 잘못된 material slot 거절과 기존 clone 보존을 통과했다. 로그는
`out/DefaultOutfits20260922/material-model-final.log`다. 이 실행은 shader binding0이며
GPU 표시나 Base/Light 연결 성공으로 대신 기록하지 않는다.

공통 shader factory의 실패는 FX11 reflection으로 따로 조사했다. 먼저 Deferred 본체에만
`g_MapPBRContributionScale`이 있고10개 분할 CSO에 없는 세대 불일치를 확인했다.
18:19 Engine 재빌드 후 Deferred의10개 ABI 비교는 모두 통과했다. 이어 Client 본체와
아직 재빌드 중인 static mesh 분할 CSO에서 두 Map PBR uniform의 같은 불일치가 관측됐다.
기존 Stage_ProgramVariants의 uniform/pass ABI 거절을 완화하지 않고 정식 빌드 결과를 검증한다.
진단 코드는 `shader_abi_probe.cpp`, 결과는 `shader-abi-current.log`에 있다.

18:26 공통 partition 재검사는3개 중2개 실패했다. 병행 작업의 신규160~210 생성기가
정본 partition writer와 다른 comment/dispatcher 형식을 써서 no-op에 해당 파일을 다시 쓴다.
임시 복사본에서 expanded Base/Light source가 완전히 같음을 확인했으며 program112의 수식
회귀는 아니다. 같은 임시 복사본을 한 번 정규화하면 기존 검사3/3이 통과하고 다음 no-op은
bytes와 mtime를 보존한다. 실제 shader 파일들의 bytes/mtime가 조사 전후 같은 것도 확인했다.
증거는 `partition-byte-diagnosis.json`과 `partition-normalized-copy-tests.log`다.
진행 중인 Product의 shader 파일이나 타 작업의 생성기를 다시 쓰지 않았다.
최종 제품 빌드와 새 CSO의 Base/Light 실제 연결 결과는 완료 후 별도로 기록한다.
Client/UI 실행·조작·화면 캡처는 수행하지 않았다. 원작의 모든 scene 환경 입력·최종 색과 광택의
동등성 및 사용자의 화면 판정은 이 source 연결·컴파일·수치 검증과 구분한다.

## G08. 최신 제품 셰이더의 실제 소비 — 19:02

정식 Product의 FxCompile로 ABI가 불일치한4개 CSO를 재생성한 뒤 실제 설치된 Resources와
현재 ActorCatalog/parameter 코드를 사용하는 WARP consumer를 실행했다. 최신18:46 header로
probe도 다시 컴파일했다. 결과는14모델·31재질·Base/Light binding62건·잘못된 material slot의
rollback14건 모두 PASS, failures0이다. 기존 CModel 생성·clone·Bind_SourceCharacter·
Bind_SourceCharacterForwardLight·CShader::Begin 경로를 사용했으며 테스트용 모델 런타임은 없다.

Animated/Static/Deferred base3개와 각각10개 source cohort, 총33개 설치 CSO의
pass 수·이름·input signature·ProgramVariantPass와 uniform/resource ABI도 failures0이다.
실제 CShader factory/clone이 성공한 뒤 각 모델의 재질을 바인딩했으므로 reflection 결과만으로
factory 성공을 추정한 것이 아니다. 프로그램은 기본 stack 크기를 사용하는 headless 실행이다.

증거는 `out/DefaultOutfits20260922/material-probe-final.log`, `shader-abi-final.log`,
검사한 Engine DLL·CSO·probe·catalog·parameter header의 SHA-256은 `material-consumer-final.json`이다.
이 검사는 UI 진입이나 최종 GPU pixel/원작 화면 동등성 검사가 아니다. Product 실행 파일의
최종 링크 결과는 Character Composition 통합 RESULT의 G06에서 별도로 기록한다.

## G09. 후속 공용 family 확장의 C1061 방지

병행 작업에서 native237까지 추가된19:02~19:08 header는135개 family의 긴 else-if chain으로
MSVC C1061 중첩 한계를 넘었다.19:04 Product의 ActorCatalog object는 그 이전 header로
컴파일됐으므로 앞선 PASS와 구분한다. 백업과 교체 직전 hash 검사 후 모든 family가 유일한
nonzero program을 첫 문장에서 설정함을 확인하고, `staged.program == 0u`를 조건에 포함한
sibling if로 바꿨다. 앞에서 일치한 family 뒤의 문자열 비교는 건너뛰며 unknown family는
마지막 zero-program guard에서 거절한다.135개 packing body와 공통 보정·validation·commit
tail의 bytes는 그대로 보존했다.

정본 VehiclePipeline의 emit/install/verify/named-vector reader와 customizing face parameter
reader도 두 분기 형식을 처리한다. Debug header syntax PASS, 실제 Configure11검사 PASS,
Python7검사와 face reader 교차4fixture PASS다. 원본112의 Base667/Light737행과 CPU packing은
조건문 표기만 제외하고 동일하다. unknown family·필수값 누락·추가값은 실패하고 기존 result를
보존한다. HLSL·CSO·project는 이 분기 수정에서 변경하지 않았다.

증거는 `out/DefaultOutfits20260922/configure-dispatch-depth/`의 `header-migration.json`,
`header_syntax.log`, `guard_runtime.run.log`, `program112-after-flatten.json`이다.
수정한 header로19:15 probe를 다시 컴파일해 앞서 통과한19:04 runtime snapshot에 연결한
14모델·31재질·62바인딩·14rollback 검사도 통과했다. 새14개 cohort의 정식 Product는 별도
진행 중이며 이 보존 snapshot 검사를 해당 최신 Engine/CSO 검증으로 기록하지 않는다.

## G10. 사용자 요청에 따른 마무리 상태 — 19:22

사용자가 “일단 정리하고 마무리”를 요청해 추가 대기 검사를 종료했다. 이 작업이 만든
`wait-and-probe.ps1` 대기 process만 중단했으며 다른 작업의 정식 Product/MSBuild는 건드리지
않았다. 마지막 완료된 제품 checkpoint는19:04 PASS이고,19:06에 시작한 후속 Product는
19:13 Engine/Shared/Server 이후 Client shader 단계에 있다. 따라서 최신14개 cohort 전체의
제품 링크·45개 CSO consumer 검사는 이번 마무리 시점에 미완료다. Client/UI 실행은 하지 않았다.

19:08 최종 전달 검증에서 승인 Camera4개의 hash와 설치/GBResources108파일의 hash·크기가
모두 일치했다. `delivery-final-verified.json`에 기록했다. 별도 작업의19:06 partition 정규화
검사는3개 PASS였지만, 이후 공용 소스 변경 뒤 실행한19:22 최종 검사는 no-op/단일 leaf 변경
검사2개가 다시 실패했다. `partition-final.log`에 현재 실패 내용을 보존했다. 이를 최종 PASS로
기록하지 않으며 현재 출력 형식의 재정규화는 미완료다. 분기 수정의 Source header/생성기 검증은
G09처럼 완료다.

## G08. 선택 장비와 누락된 본체 재질의 실제 반영

커스터마이징 의상·머리·무기 및 기존 기본 캐릭터의 실제 설치 WModel을 조사하고,
CharacterCatalog의 최종545개 model/material descriptor를337개 고유 모델에 연결했다.
545는 descriptor 수이며, 여러 submesh가 같은 재질을 공유하므로 실제 native mesh slot은549개다.
root `modelMaterialOverrides`는428행, 가디언나이트의 본체·새 기본 장비·Wing은19행이다.
기존 건슬링어·슬레이어 기본 복원과 다른 세션의110/111/112 program 수정도 유지했다.

`CActorCatalog::Build_ModelLoadDescription`은 선택 장비의 source descriptor를 해석하며,
`CEquipmentPresentationService`가 같은 descriptor로 기존 `CModel -> CMaterial` 경로를 만든다.
정확한 원본 MIC/ShaderMap에 맞춘 신규 program160~200을 Base/Light cohort160/176/192에
추가했다. 실제 사용 중인 신규 program은39개이며 생성 후보166/200은 현재 descriptor에 없다.
기존 source family가 일치하는 얼굴·눈·눈꺼풀·머리 등은 기존 program을 재사용했다.
Character Select의208/210도 같은 공통 등록에 포함했다. 209의 별도 map forward 경로와
45배치 복원은 Character Select 결과 문서에서 설명한다.

투명·fur 재질은 실제 source permutation을 분류해 기존 forward 조명으로 소비한다.
본 부착형 static 장비에도 같은 native forward 입력을 연결하고 static pass24를 추가했다.
skinned 장비의 master palette, socket, preTransform은 유지한다. shader의 원본 varying,
texture expression index, sRGB/linear, 상수와 실제 front-face 위치를 각각 연결했다.
새 shader/header/wrapper는 Engine/Client 프로젝트와 filters에 등록했고 기존 filter는 이동하지 않았다.

## G09. 원본 UV와 NULL material의 구분

181개 선택 장비와 가디언나이트·창술사 본체2개에 원본 추가 UV를 복구했다.
position/UV0만 같은 seam은 원본 triangle corner와 normal까지 대조하여 유일하게 조인했다.
가디언나이트 본체는 `pc_dk_00.mesh.pc_dk_00_sk`의 전체6 submesh와 일치한다.
14,703정점·20,789삼각형을 대조해 hair UV1, eye UV1/UV2를 복구했으며285bone·165animation을
보존했다. 창술사 본체의 hair7은 `pc_ft_08_hair_sk`의1,239정점·1,595삼각형과 일치하며,
224bone·224animation과 기존 eye UV를 보존했다. WUVS 추가 외 legacy payload는 byte 동일하다.

워로드 `pc_wr_05_hair`의 원본 mesh에는 UV1이 없다. 실제 source program7에서 UV1은
두 톤 혼합에만 영향을 주고 설치 MIC의 Base/Light two-tone 값이 모두0이다.
이 조건과 유효한 양쪽 분모를 검사해 원본 단일 UV를 허용한다. UV1 없는 모델에 runtime
override나 material variant가 두 톤을 켜려 하면 해당 변경을 거절하고 기존 clone을 유지한다.
UV0 복제나 임의 UV 값은 만들지 않았다.

used placeholder3개는 원본 UPK USkeletalMesh material array에도 NULL이다.
Artist `default_variant_01_lower/lower.wmodel`의 sub1은22정점/60index,
`pc_sp_32_hair/head.wmodel` sub1은239정점/1,014index,
`pc_sp_49_hair/head.wmodel` sub1은288정점/1,362index다.
각 원본 배열은 `[295,0]`, `[3,0]`, `[-4,0]`이며 두 번째0이 실제 NULL object reference다.
이3개와 미사용 빈 WMAT270개를 누락 MIC로 계산하거나 새 재질로 추정하지 않았다.
증거는 `out/CharacterMaterials20260922/null-material-audit.json`과 body/장비 UV receipt에 있다.

## G10. 첫 이미지의 가디언나이트 기본 외형과 커스터마이징

원본 PCPreview702와 LookInfo의 HR00 upper/lower, PC_DDK_02-5 arm/shoulder/helmet,
WP_WDDK_04-5 무기를 추적했다. 무기 LookInfo는 `WP_WDDK_04.Mesh.WP_WDDK_04_SK`와
`WP_WDDK_04.Mat.WP_WDDK_04-05_MI`를 명시한다. 모델04와 재질04-05의 차이를 이름으로
추정하지 않고 이 참조를 따랐다. source socket `WP_DDK_R`는 설치 master의 `b_wp_1` index152로 연결한다.

기본 의상5부위와 무기는 `Character/GuardianKnight/Equipment/class_select_hr00/`의
upper/lower/arm/shoulder/helmet/weapon WModel로 변경했다. 기존 기본6부위는
`original_00/`로 보존해 다시 선택할 수 있다. 본체·Wing·animation 경로는 유지했다.
EquipmentPresentationCatalog는235 visualSets, 가디언나이트는14세트다.
부위별12세트와 의상5부위를 한 번에 교체하는 bundle2개를 제공한다.
커스터마이징에는 새 의상·기존 의상·새 무기·기존 무기의4선택과 대응 icon을 연결했다.
실제 `CEquipmentPresentationCatalog`, costume/icon document reader가235세트와
가디언나이트14세트, 여섯 부위 및4개의 선택 ID/이미지 연결을 검증했다.

## G11. 설치·전달과 현재 검증 증거

최신 디스크의 stable ID/변경 필드만 병합하여 CharacterCatalog, EquipmentPresentationCatalog,
CustomizingCostumes, CustomizingIcons의4 JSON을 반영했다. 교체 직전 hash 재검사,
writer lock, 백업과 원자 교체를 수행했고 실패 시 자기 변경만 rollback하는 경계를 유지했다.
실제 교체 receipt는 `out/CharacterMaterials20260922/catalog-install-receipt.json`이다.

이 선택 장비 작업의 전달 목록 합집합은 root당1,201개 고유 파일,1,156,172,756bytes다.
원본 texture, UV를 복구한183 WModel, 새/기존 가디언나이트 선택 리소스와 icon을 포함한다.
`Client/Bin/Resources`와 `C:/Users/user/Desktop/GBResources`에서 전체 파일을 다시 읽어
SHA-256 일치를 검증했다. 별도 작업의108파일 집계와 중복될 수 있으므로 수를 단순 합산하지 않는다.
같은 이름의 기존 texture bytes가 다른3개는 원본 hash를 넣은 별도 파일명으로 저장하고 참조도
그 경로로 연결하여 기존 리소스를 덮어쓰지 않았다. 증거는 `delivery-final-receipt.json`이다.

화면 없는 실제 WARP CModel 소비자에서337모델·549 native mesh slot의 생성/clone과
잘못된 material slot337건 거절, 기존 clone 보존을 검증했다. 원본 NULL3개는 별도로 기록했다.
최초 전체 실행 중 가디언나이트 UV 교체가 완료되기 전 발생한1실패는 본체2모델의 재검사로
닫았다. 이후18:46 Engine과 현재 설치 데이터로337모델 전체를 다시 한 번 실행해
`models=337 materials=549 bindings=0 negative=337 unbound=3 failures=0`을 확인했다.
`material-model-installed-final.log`와 `material-model-coverage.json`이 이 단일 전체 실행의 증거다.
워로드 단일 UV의 inactive 허용과 active override/variant 거절도 같은 새 Engine으로 검증했다.
이는 실제 shader binding 검사의 완료를 대신하지 않는다. 최종 canonical CSO의 Base/Light
연결 결과와 제품 빌드는 아래 후속 검증에 따로 기록한다. Client/UI 실행은 수행하지 않았다.

## G12. native 분할 생성의 재현성 복구

선택 장비 설치기가 기존 canonical partition writer와 다른 comment 경계·case guard 형식으로
cohort를 추가하여 공통 no-op 검사2개가 실패했다. 재질 수식의 차이는 아니었으나 기존 재질을
한 번 편집할 때 무관한 leaf까지 재작성하므로 설치기를 정본 writer 호출로 수정했다.

활성 Product의 shader 컴파일이 끝난 뒤 최신 디스크에서 후보를 재생성했다.
동시 작업의211~213과235 cohort 추가도 보존했다. 구 후보의 hash 불일치 시도는 쓰기 전에
거절됐고, 새 후보는 백업·직전 hash 재확인·atomic 교체로 Engine/Client28 HLSL에 반영했다.
24개였던 초기 범위에 새235 Base/Light의 양쪽 mirror4파일이 더해진 것이다.
내용이 바뀐 파일의 mtime를 보존하지 않았다.

설치 전후 전체 expanded Base/Light의 bytes와 SHA-256은 정확히 같다. 또한 각 stage에서
11개 cohort와 미지정 전체 선택을 따로 펼친24개의 guard 선택 모두 HLSL token이 같다.
설치 후 실제 공통 partition 검사3/3이 통과했고, Engine/Client mirror 일치도 확인했다.
증거는 `partition-normalization.json`, `partition-cohort-equivalence.json`,
`partition-installed-tests.log`다. 이후 정상 Product 증분 빌드는 통합 작업이 수행한다.

## G13. 사용자 마무리 시점의 shader 검사 경계

고정한33 CSO snapshot에서 animated/static/Deferred 본체3개와 source variant30cohort를
실제 `CShader`로 모두 로드했다. 처음 두 shader factory 실패는 컴파일 진행 중인 서로 다른
CSO 세대를 복사한 검사에서 발생했고, 완성된 고정 snapshot에서는 재현되지 않았다.
이 snapshot은 후속235 cohort를 포함한 최종 Product 검사를 대신하지 않는다.

전체 native binding의 첫 실행은 검사기가549재질을 한 프레임에 계속 누적하여 기존
256-row 상한에서 E_BOUNDS로 멈췄다. 제품 Renderer는 매프레임 material frame을 초기화한다.
검사기의 직접 Reset 호출은 DLL 내부 경계라 사용하지 않았고 제품 export도 추가하지 않았다.
대신 모델별로 순차 join하는 worker에서 thread-local material frame 수명을 분리했다.
D3D11 device Flags=0으로 SINGLETHREADED를 사용하지 않는다. 이는 모델별 격리 TLS의
Base/Light/forward binding 검사이며 실제 Renderer Reset 호출을 검증한 것은 아니다.

재컴파일 중 최신 native family 추가 이전의 긴 else-if header에서 C1061이 관측됐다.
외부 작업이135family를 flat-if/lambda로 바꾸고 generator의 emit_configure 및
guarded_configure_block도 갱신했다. 이를 보존한 최신 검사기 컴파일·링크는 성공했다.
본 작업은 이 문제 때문에 제품 코드를 추가로 수정하지 않았다.

사용자가 정리 후 마무리를 요청하여 진행 중인 본 작업의 headless probe PID35812만 종료했다.
전체337모델의 Base/Light/forward binding 완료 결과는 아직 없다. source30cohort 로드와
337모델·549slot의 생성/clone/337rollback 성공을 전체 GPU binding 완료로 확대하지 않는다.
현재 상태와 마지막 binding 실행의 완료 모델 수는 `native-binding-wrapup.json`,
진행 로그는 `material-shader-installed-final.log`에 기록했다. Root의 Product 빌드는 건드리지 않았다.
마지막으로4개 설치 JSON을 다시 parse하고 설치 receipt와 hash가 그대로 같은 것을 확인했다.
Client/UI 실행·화면 판정과 최신 통합 Product 최종 결과는 별도 확인 범위다.
