# 캐릭터·아바타 재질 연결과 고난도 이펙트 2차 복구 예비 견적

## 현재 상태와 이번 문서의 범위

2026-09-08 현재 working copy를 읽어 정리했다. 조사·부분 구현·중단 검토 이후 사용자가
Q 복구본의 All Effects/Play 연결, 모코코·차원술사·양 클래스 무기 복구와 CS→쿠크 진입
종료 오류 수정을 승인했다. 현재 구현을 재개하며 최신 범위는 G13이다. 기존 Product Q 호출은
유지한다. 앞선 빌드 대기를 유지하고 Client/UI를 자율 실행하지 않는다.
실제 반영 상태와 검증 경계는 대응 `2026-09-08_CHARACTER_MATERIAL_AND_EFFECT_ROUND2_ESTIMATE_RESULT.md`를 따른다.
G01~G10의 현재 상태는 최초 조사 시점의 기준선이다. 재개 후 실제 변경과 검증 상태는 RESULT를
우선하며, 아래 최초 조사나 중단 당시 문장을 최신 코드의 미구현 판정으로 사용하지 않는다.
이 문서의 공수는 개발자 1명이 집중 작업하는 개발일 기준의 설계 추정이며 실제 소요시간을
측정한 확정 일정, Codex 응답시간, 비용 견적이 아니다. 기존 코드·원본 package/cache가 사용
가능하고 사용자가 비교 화면과 재생 결과를 확인할 수 있다는 조건을 둔다.

## G01. 렌더링 연결의 실제 완료 범위

엔진 전체에 재질 계산이 없는 상태는 아니다. 기존 D/N/S/E, 조명, 그림자, HDR, Bloom,
톤매핑 및 투명 합성 기반이 있고 일부 원본 material family를 추가로 연결했다.
핵심 미완성은 원본 재질의 선택·채널 의미·파라미터·계산을 모든 소비자에게 전달하는 부분이다.

| 대상 | 현재 연결 | 남은 범위 |
|---|---|---|
| 복구한 맵 재질 | source specular, PBR opaque/seamless, 실제 normal/ORM 식, 선택적 UV1 lightmap·환경반사·발광 | 다른 모델/재질은 별도 identity 및 family 연결 필요 |
| 일반 캐릭터·장비 | D/N/S/E, 일부 WMA3 염색, body/equipment/socket·avatar 가시성 | source family와 S.A/ORM/피부·머리·눈·환경반사 계산의 완전한 연결 |
| 일반 이펙트 | particle/mesh/trail/decal, UV·mask·dissolve·blend·distortion 및 source 식 일부 | 이펙트별 다른 입력/변형/장면 정보·출력 pass를 현재 renderer와 맞추는 일 |

색감 차이를 전부 PBR 누락 하나로 환산하지 않는다. 잘못된 material override, 누락되거나 잘못
해석된 채널, 색 공간, 환경광, mip/샘플러, alpha/blend와 최종 톤매핑은 서로 다른 원인이다.
모든 이펙트에 metallic/roughness를 추가하는 것도 해법이 아니다. 먹물·불꽃·빛 카드는
원본의 색·coverage·합성 식을 먼저 맞춘다.

실측 캐릭터 범위는 여섯 class의 실제 모델 45개, material slot103개다. 90개 slot은 원본 재질
정합 자료가 있으며 이것이 원본 shader permutation과 최종 화면까지 검증했다는 뜻은 아니다.
기존 WMA2/3 reader는 텍스처와 염색 입력을 보존하지만 원본 shader graph 전체를 저장하지 않는다.
현재 animated shader는 S.rgb를 회색 specular mask로 줄이며 S.a와 ORM을 소비하지 않는다.
차원술사 실제 모델에 ORM13슬롯이 있어도 이 경로에서는 PBR 계산에 들어가지 않는다.

모코코 원본에서는 S.RGB, S.A, 색상/피부 마스크의 RGB/A가 서로 다른 계산에 쓰인다.
S.A는 거칠기 식, 마스크 A는 피부와 일반 표면의 파라미터 선택에 들어간다.
BRDF LUT는 meshUV에 붙이는 무늬가 아니라 시선·광원·광택 값으로 조회하는 계산표다.
따라서 파일 이름 `_s`, `ORM`, 알파라는 이유만으로 의미를 일괄 결정하지 않는다.
움직이는 캐릭터의 간접광은 바닥의 UV1 lightmap을 그대로 복사해서 해결하지 않는다.

현재 구현 근거:

- [material 입력 구조](C:/Users/user/Desktop/LostArk/Engine/Public/BinaryAsset/ModelAssetData.h)
- [WMaterialReader](C:/Users/user/Desktop/LostArk/Engine/Private/BinaryAsset/Winters/WMaterialReader.cpp)
- [캐릭터 모델 생성](C:/Users/user/Desktop/LostArk/Client/Private/PlayableCharacterAssetService.cpp)
- [캐릭터 재질 바인딩](C:/Users/user/Desktop/LostArk/Client/Private/DeferredMaterialRenderUtils.cpp)
- [실제 animated shader](C:/Users/user/Desktop/LostArk/Client/Bin/ShaderFiles/Shader_VtxAnimMeshBinary.hlsl)
- [맵 재질·조명 바인딩](C:/Users/user/Desktop/LostArk/Client/Private/MapAssetRenderUtils.cpp)

## G02. 향후 아바타가 공통으로 사용할 재질 연결

기존 Character.cpp의 body/equipment/weapon, avatar head/armor 가시성 및 몸체 가림 규칙은
재사용한다. 현재 클래스별 모델 목록·prototype 개수는 고정된 부분이 있어 임의 아바타 catalog
전체를 교체하는 일반 시스템이 완성됐다고 보지는 않는다. 착용/인벤토리/서버 상태와 표면을
그리는 계산은 별도 기능이며 이번 재질 공수에 새 상품·인벤토리 시스템은 포함하지 않는다.

현재 CActorCatalog → CPlayableCharacterAssetService → CModel 생성 단계에 named material
입력을 연결하고, 기존 MODEL_MATERIAL_OVERRIDE → CMaterial의 단일 경로를 확장한다.
이름으로 고정한 모델/재질 key, source family, texture 역할·색 공간·sampler, typed parameter를
데이터로 전달한다. 같은 family의 새 아바타는 이미지·값을 추가하고, 새 계산 계열일 때만 shader를
확장한다. 아바타 이름별 shader 분기를 늘리지 않는다.

대표 순서는 원본 프로그램이 확보된 모코코 3재질 → 일반 염색 장비의 천/금속 → 신형 realpbr
→ 피부·눈·머리의 특수 표현이다. 각 단계에서 로드뿐 아니라 skinned/socket 두 소비자와
G-buffer/조명 결과까지 연결한다. Rendering Tool은 구현된 입력의 비교·튜닝을 맡고,
누락된 셰이더 계산을 자동으로 생성하는 도구로 취급하지 않는다.

초기 **5~10 개발일**은 공통 material 연결과 모코코 대표 family의 첫 구현을 가정한 수치였다.
사용자 요청의 원작 비교·반복 수정까지 포함하는 현재 작업 예산은 G08로 갱신한다.
모든 class의 피부·눈·머리, 모든 아바타와 인벤토리 확장은 포함하지 않는다.

## G03. 도화가 F에서 이미 복구한 원리

도화가 F(31470)는 texture만 추가한 사례가 아니다. 원본 emitters의 geometry, 좌표계,
크기·생성 수·수명, color/alpha over life, dynamic parameter, SubUV, 무기 ribbon과
재질별 texture/channel 식을 함께 연결했다. 예를 들어 흰 붓은 색상 곡선 손실,
굵고 불투명한 ribbon은 mask red 대신 DDS container alpha를 쓴 문제였고 각각 교정했다.
원본 compiled shader를 읽어 필요한 식을 현재 HLSL에서 재현한 경로가 있으며,
원본 엔진의 raw DXBC/pass 전체를 그대로 실행한 것과 구분한다.

현재 authored 문서는 v13의 17개 활성 element(particle14/trail1/decal2), sourceRecipe14개다.
예전 Core33/35와 억제 행 개수를 현재 제품 문서의 결함 분모로 사용하지 않는다.
과거 미복구로 기록된 #20/#21은 후속 Track C에서 six-SRV localDecal opcode14로 열렸고,
실제 Render_Decal → Depth/Normal binding → six-SRV shader 경로가 존재한다.
다만 원본의 모든 parallax/orientation fade/fog/custom-light/MRT 계약을 재현한 것은 아니다.
현재 문서에 없는 과거 효과를 사용자 동의 없이 복원 대상에 다시 넣지 않는다.

재사용할 것은 source particle 실행·좌표/단위 처리, texture/channel 바인딩,
decal/trail/mesh renderer와 필요한 식의 재현이다. 새 이펙트에서는 해당 원본 family·입력과
현재 frame/particle 값을 다시 연결해야 한다. 도화가용 수치를 다른 스킬에 복사하지 않는다.

현재 근거:

- [Artist F authored](C:/Users/user/Desktop/LostArk/Data/Effects/Authored/effect.artist.skill.31470.unified.effect.json)
- [Track C 결과](C:/Users/user/Desktop/LostArk/.md/GB/08-14/2026-08-14_TRACK_C_UNIFIED_EFFECT_AUTHORING_RESULT.md)
- [Effect renderer](C:/Users/user/Desktop/LostArk/Client/Private/Effect_DocumentRenderer.cpp)
- [일반 V2 재질 식](C:/Users/user/Desktop/LostArk/Client/Bin/ShaderFiles/Shader_EffectV2_Common.hlsli)

## G04. 고난도 이펙트의 첫 연결 범위와 초기 추정 이력

여기서 2차는 복구 작업의 두 번째 묶음이며 특정 레이드 관문을 뜻하지 않는다.
현재 사용자 저장본을 기준으로 Artist F를 회귀 기준으로 두고 차원술사 Q/F를 대표 대상으로 삼는다.

| 작업 | 실제 범위 | 예비 공수 |
|---|---|---:|
| 대상·비교 기준 고정 | 현재 선택된 cue/element, 카메라·조명, 원작에서 맞출 핵심 형태와 타이밍 고정 | 1~2일 |
| 차원술사 Q 2050100 | 현재 cube mesh+slice sprite 2element 중 cubesample 계열의 grouped 재질 입력·식 연결 | 2~4일 |
| 차원술사 F 2050230 | 현재 5cue 합성의 유리·water 후보 형태, 방향·크기·움직임·coverage 개선 | 3~6일 |
| 공통 연결·통합 확인 | 필요한 family의 현재 scene depth/color·distortion 소비, 기존 Artist F 회귀, 저장/재생/실패 보존 | 3~5일 |
| 합계 | 위 대표 범위의 첫 복구 묶음 | **9~17 개발일** |

위 **9~17 개발일은 현재 후보의 첫 연결·개선에 대한 초기 추정**이다. 사용자 요청에 따른
원작 유사도 비교·반복 수정과 ALT_V까지의 현재 작업 예산은 G08을 우선한다.
초기 수치를 최종 시각 품질의 완료 일정으로 사용하지 않는다.

F는 새 이펙트가 전혀 없는 상태가 아니다. 현재 기본 F0ms, W clip3 450ms,
W clip2 590ms, water-burst700ms, single-glass700ms의 5cue가 연결돼 있다.
견적은 그 합성과 후보를 개선하는 범위다. 파편마다 독립 운동·회전·충돌을 만들거나 입자마다
별도 ribbon history를 생성하는 확장은 이 범위에 포함하지 않고 추가 견적을 낸다.
새로운 refraction pass나 원본 pass 수준의 decal 정합이 필요해질 때도 먼저 한 대표 입력을
확인해 확장 범위를 결정한다. 이미 있는 Artist F six-SRV decal의 재개발 비용은 더하지 않는다.

창술사 F34150 같은 다음 후보는 공통 family의 재사용 결과를 보고 추가한다. 건슬링어·슬레이어·
워로드·보스 전체와 모든 원본 occurrence의 복구는 이번 합계의 분모가 아니다.

## G05. ALT+V와 최종 품질 목표의 실현 가능성

현재 코드·데이터와 Artist F 복구 사례는 원작에 가까운 표현을 목표로 진행할 기술적 근거가 된다.
다만 원작의 모든 화면과 성능을 동일하게 재현할 수 있다고 확정한 것은 아니다. 정지 화면의
색감뿐 아니라 전체 시간의 형태·움직임·경계·합성·카메라와 실제 플레이 중 성능을 함께 확인한다.
미복구 입력이나 원본 장면 정보가 없으면 해당 부분의 근사 범위도 분리해 기록한다.

2026-09-08 현재 PlayerSkills 및 실제 skillbindings/animevents/EffectCatalog 기준이다.
과거 V/ALT_V 번호가 다른 조사 문서를 현재 슬롯 정본으로 사용하지 않는다.

| 클래스 | ALT_V skill ID | 현재 데이터 연결 |
|---|---:|---|
| 창술사 | 34630 | 애니메이션 4클립에서 4개 제품 Effect 문서로 연결 |
| 건슬링어 | 38320 | 애니메이션 2클립 연결. 해당 클립의 제품 Effect asset cue는 이번 조사에서 미확인 |
| 슬레이어 | 45820 | 3종 애니메이션 클립을 반복하는 체인. 제품 Effect asset cue는 이번 조사에서 미확인 |
| 워로드 | 17250 | 애니메이션 2클립에서 2개 제품 Effect 문서로 연결 |
| 도화가 | 31930 | 애니메이션 2클립에서 2개 제품 Effect 문서로 연결, 합계 97개 element |
| 차원술사 | 2050540 | 애니메이션 1클립에서 제품 Effect 문서로 연결, 123개 element |

이 연결은 실제 draw·최종 색감·원작 전체 구성의 일치 증거가 아니다. 네 클래스의 연결된 문서만으로
별도 ModelCue·ScreenPost 및 원작 카메라 연출까지 완료됐다고 결론 내릴 수 없다.
건슬링어·슬레이어도 위 경로의 cue 미확인을 프로젝트 전체 이펙트 부재로 확대하지 않는다.

첫 ALT_V는 차원술사 2050540을 제안한다. Q/F에서 연결한 재질·입자 경로를 같은 클래스의
복합 연출로 확장하고, 현재 한 개 클립에서 연결되는 전체 재생을 비교하기 좋다.
기존 123개 element를 무조건 늘리기보다 실제 화면에 필요한 원본 구성과 시간별 출력을
대조하고, 색·형태·움직임·공간 배치·깊이 합성·필요한 카메라/화면 연출 순으로 닫는다.
이때 발견된 새 재질 계열과 입력의 개수로 남은 ALT_V의 공수를 다시 산정한다.

**G04의 9~17 개발일에는 ALT_V 한 개 또는 여섯 클래스 전체 복구가 포함되지 않는다.**
지금 단계에서 전체 ALT_V의 완료 날짜를 같은 숫자로 약속하지 않는다. 재사용 가능한 부분이
크더라도 새로운 특수 재질, 모델 변형, 카메라와 화면 합성은 별도 작업량이 될 수 있다.

실행 순서는 캐릭터 대표 재질과 비교 조명 고정 → 차원술사 Q/F → 차원술사 ALT_V 전체 재생
→ 같은 계열을 사용하는 다른 클래스 확장이다. 비교 조명을 고정한다는 것은 임의 밝기 튜닝으로
재질 결함을 가리는 대신 같은 조건에서 복구 전후의 원인을 구분한다는 뜻이다.
작업 완료의 품질 기준은 사용자와 비교 가능한 목표 해상도·카메라·대표 장면을 먼저 정하고,
시각적 일치와 프레임 비용을 함께 확인하는 것으로 잡는다.

현재 근거:

- [현재 스킬·입력 슬롯](C:/Users/user/Desktop/LostArk/Data/Balance/PlayerSkills.json)
- [현재 Effect catalog](C:/Users/user/Desktop/LostArk/Data/Effects/EffectCatalog.json)
- [차원술사 애니메이션 이벤트](C:/Users/user/Desktop/LostArk/Data/Animation/Authored/DimensionMaster/DimensionMaster.animevents)
- [차원술사 ALT_V 문서](C:/Users/user/Desktop/LostArk/Data/Effects/Authored/effect.dimensionmaster.skill.2050540.unified.effect.json)

## G06. 구현 시 완료 기준

1. 현재 저작본의 실제 선택 cue/element가 원하는 model/texture/material/pass를 소비한다.
2. 전체 재생 시간에서 transform과 particle 수·크기·수명이 유한하며 필요한 draw가 제출된다.
3. 채널·색 공간·UV·blend·깊이/화면색의 사용이 회수한 원본 식 또는 명시한 프로젝트 근사와 맞는다.
4. 수정 family의 focused shader/CPU 검사와 최소 컴파일을 하고 실제 저장/재로드 및 실패 보존을 확인한다.
5. 사용자가 고정 조건에서 색·형태·경계·타이밍을 판정한다. 자동 수치 검사는 이 판정을 대신하지 않는다.

자료가 필요한 곳부터 ABI의 texture/sampler/constant/vertex/output 계약을 조사한다.
원본 엔진 전체 ABI 복구, 새 광역 oracle/manifest 시스템, 사용자 편집본을 과거 추출본으로
일괄 되돌리는 작업은 완료 조건에 넣지 않는다. 이번 문서 작성에서는 위 구현·빌드를 실행하지 않았다.

이번 문서의 후행 공백 검사는 0건이다. 전체 working copy의 `git diff --check`는
다른 작업 파일인 `Client/Private/ArenaCameraProfile.cpp:53`의 후행 공백으로 실패했다.
이 조사에서 해당 파일을 수정하지 않았으며 다른 작업의 변경도 정리하지 않았다.

## G07. 기존 Effect Tool에 복구 작업을 연결하는 실제 방향

사용자는 모코코 → 차원술사 Q/F → 차원술사 ALT_V → 타 클래스 확장 순서를 선택했다.
현재 요청은 어제 변경한 Tool의 상태 확인과 현실적인 견적·구조 결정이다. 이번 조사에서
새 Effect 문서, Catalog 행, 애니메이션 cue 또는 제품 코드를 생성·교체하지 않았다.

### 현재 화면과 문서 소유권

어제 작업은 기존 F1 Effect Tool의 V1 편집기와 V2 편집기를 공통 Resources/Model View/
Sequencer 화면에 연결했다. V1 문서를 V2로 일괄 변환하거나 기존 V1 renderer를 폐기하지 않았다.
현재 MainApp은 EFFECT_V2/EFFECT_COMPOSITION 진입을 공통 EFFECT로 정규화하고
`Configure_AuthoringWorkspace`로 V2 pane와 Sequencer를 연결한다.

| 화면·명령 | 현재 실제 역할 | 자동으로 하지 않는 것 |
|---|---|---|
| Effect Resources | V1/V2 저장 문서, 한글 이름·Parent/category, Create Effect | 새 ID의 Player Product catalog 등록 |
| Current Effect / Effect Detail | 선택한 V1 또는 V2 native 문서 편집 | V1 원본 recipe를 V2로 변환 |
| Model View | 기존 캐릭터·클립·bone/root 참고 재생 | 새 캐릭터 로더 또는 Server 판정 생성 |
| Effect Sequencer | 같은 시간에 여러 V1/V2 occurrence를 Preview/Append | 실제 캐릭터 스킬의 Effect 호출 교체 |
| Save Sequence | `Data/Effects/Sequences/<id>.effectsequence.json` 저장 | `.animevents` 저장 |
| Animation Tool | admitted Effect ID의 timing/anchor/follow/stop과 clip cue 저장 | Effect 내부 입자·재질 편집 |

`Render()`는 Workspace가 있으면 옛 All Effects 창을 그리지 않는다. Data Files도 새
Effect Resources로 대체된다. 따라서 과거 `All Effects -> V1` 안내를 현재 UI의 그대로인
클릭 경로로 사용하면 안 된다. 스킬별 제품 호출 조회가 필요하면 기존 조회 기능을 같은 툴에
노출하고 데이터와 renderer를 재사용한다.

현재 dirty diff에는 V2 본문보다 공통 Append 명령을 먼저 그리는 수정, 선택한 V2 duration을
Preview/Append에 전달하는 수정, saved resource의 Preview/Append 버튼 추가가 있다.
사용자는 V2 담당자가 표시 문제를 수정했다고 설명했다. 여기서는 코드 연결을 확인했으며
그 수정의 실제 UI 실행 성공이나 빌드 포함 여부를 독립 판정하지 않았다.

### 확인된 복사·저장 기능의 빈틈

New V1은 빈 문서를 만들고 한글 표시명과 별도 ASCII ID를 발급한다. 현재 Q를 복사하지 않는다.
Duplicate Selected/Marked는 같은 문서 안의 Element 복제이며 전체 Effect 복제가 아니다.

전체 복사 함수 `Try_SaveDocumentAs`는 존재한다. 하지만 일반 AUTHORED Q에서 사용할
Save As 버튼은 옛 Data Files의 Advanced 영역에 있고, Workspace가 그 전에 return한다.
다른 Save As 버튼은 migration/visual-program 분기이므로 현재 Q의 대체 경로가 아니다.
이 함수는 표시명을 원본 그대로 유지하고 Catalog 등록도 하지 않는다.

최소 보강은 기존 ResourceTree/Workspace 명령에 V1 전체 `복구본 만들기`를 연결하고,
기존 Save As의 새 ID·새 표시명·Parent 연결을 함께 처리하는 것이다. 문서 전체를 복사해
sourceRecipe·material sourceProfile/execution·곡선·resources·attachment를 보존한다.
일반 codec이 담지 못하는 exact adapter projector/VF packet의 복사는 기존처럼 거부한다.
sourceRecipe를 끄거나 모든 element를 일반 sprite로 변환하는 방법을 사용하지 않는다.

변경 시작점은 `EffectAuthoringResourceTree.{h,cpp}`의 typed command,
`Effect_Tool_Workspace.cpp`의 명령 소비, `Effect_Tool.{h,cpp}`의 저장·트리 연결이다.
기존 파일을 확장하므로 이 단계에는 새 C++ 파일이나 project/filter 등록을 계획하지 않는다.
새 shader 파일이 실제 필요해지는 family 구현 단계에서는 Client/Engine 소유권과 기존 CSO
producer 등록을 확인해 해당 프로젝트 및 filters 등록까지 같은 변경에 포함한다.

### 차원술사 Q 복구본의 구체적인 흐름

| 항목 | 계획 값 |
|---|---|
| 원본 유지 | `effect.dimensionmaster.skill.2050100.unified` |
| 새 stable ID | `effect.dimensionmaster.skill.2050100.restore` — 이번 조사 시 동일 파일 없음 |
| 새 표시명 | `이펙트_차원술사Q` |
| 보관 위치 | Effect Resources의 V1 아래 사용자 Parent `복구 작업` |
| 새 파일 | `Data/Effects/Authored/effect.dimensionmaster.skill.2050100.restore.effect.json` |
| 문서 등록 | 기존 `Data/Effects/EffectCatalog.json`의 DIRECT_AUTHORED 경로. 등록과 실제 스킬 호출 교체는 별개 |
| 이번 연결 목표 | All Effects에서 독립 복구본 선택 → 모델/애니메이션/anchor → Play All. 기존 Product Q의 0ms cue 유지 |

원본 문서는 보존하고 복구본만 편집한다. 먼저 원본과 복구본을 같은 Model View/Sequencer
조건에서 각각 재생한다. 공통 shader 코드를 바꾸면 두 문서에 모두 영향을 줄 수 있으므로,
새 식은 복구본이 명시적으로 선택하는 재질 family/profile로 연결하고 기존 Artist F/Q를 회귀 확인한다.
Effect 파일 이름을 shader 선택 조건으로 사용하지 않는다.

최신 사용자 요청은 현재 Product Q와 분리해 도구에서 복구본을 비교하는 것이다.
Catalog 등록이 필요하더라도 기존 animevents의 Q 호출은 `effect.dimensionmaster.skill.2050100.unified`로
유지한다. 실제 스킬 호출을 복구본으로 교체하는 제품 활성화는 이번 범위에 포함하지 않는다.
원본 reference notify와 gameplay skill/damage도 유지한다. All Effects 독립 항목과 해당
복구본·모델의 Play All 연결은 중단 시점에 아직 구현하지 않았다.

### 별도 복구 전용 툴을 만들지 않는 이유

시각 품질을 결정하는 것은 원본 입력·입자/정점 처리·재질 식·장면 합성이다. 새 창 하나가
그 계산을 늘려주지는 않는다. 현재 공통 툴에 복구 작업 Parent, 원본/복구본 선택, source 채널과
현재 식의 확인, 실제 스킬 연결 상태를 보강하는 쪽을 선택한다. 필요하면 같은 툴 안에 복구용
작업 화면을 두되 파일 저장과 preview/product renderer를 복제하지 않는다.

V1은 현재 원본 recipe와 Artist F/차원술사 Effect 문서를 유지하는 데 적합하다. V2의 기존
leaf/group 저작은 그 용도로 계속 사용한다. 번호가 2라는 이유만으로 원작 재현의 상위 호환이라고
판단하지 않는다. V1 문서가 사용하는 `RuntimeMaterialV2` 재질 실행 backend도 V2 Effect 문서와
별도 개념이므로 이름만 보고 문서 변환을 결정하지 않는다.

근거: [Workspace](C:/Users/user/Desktop/LostArk/Client/Private/Effect_Tool_Workspace.cpp),
[현재 Save As와 UI 분기](C:/Users/user/Desktop/LostArk/Client/Private/Effect_Tool.cpp:19816),
[Sequencer 저장](C:/Users/user/Desktop/LostArk/Client/Private/EffectAuthoringSequencer.cpp:847),
[Animation cue 선택](C:/Users/user/Desktop/LostArk/Client/Private/Animation_Tool.cpp:14133).

## G08. 원작 유사도 반복까지 포함한 보수적인 작업 예산

100개 커밋을 일수나 완료율로 환산하지 않는다. Artist F 주변 이력에는 식의 소비 분기 누락,
변환 과정의 색/방향/크기 손실, 잘못된 중복 제거 후 되돌리기, 저장본과 제품 사본 불일치,
준비 성공과 실제 draw/사용자 시각 결과의 혼동이 있었다. 현재 direct-authored와 sourceScale,
기존 family/particle 실행기를 재사용하므로 그 기반을 처음부터 만들 비용은 제외한다.

반면 새 원본 식과 장면 입력, 입자/정점 변형, 전체 시간 합성, 사용자 비교 후 수정은 남는다.
09-07 Tool 결과는 새 Glass/Energy/Slash material을 명시적으로 제외했다. 도구 완성과 재질
복구의 공수를 같은 완료 상태로 합치지 않는다.

아래는 실측 생산성이나 확정 납기가 아닌 엔지니어링 판단에 따른 **1인 집중 개발일 예산**이다.
현재 원본 자료 접근, 비교할 해상도/카메라/장면 고정, 단계별 2~3회 비교 수정이 가능하다고 가정한다.
일반 회귀·최소 컴파일과 해당 단계의 성능 확인은 각 행에 포함하며 별도 중복 가산하지 않는다.
사용자 응답 대기, 다른 세션과의 일정 충돌은 개발일 외의 달력 지연이 될 수 있다.

| 단계 | 첫 연결의 낙관 범위 | 원작 유사도 반복을 포함한 작업 예산 | 신뢰·주요 변수 |
|---|---:|---:|---|
| 복구본 생성·등록 흐름 보강 | 1~2일 | 2~4일 | 기존 Save As 재사용, UI/저장/호출 경계 확인 |
| 모코코 대표 3재질 | 7~10일 | 12~18일 | native 식 확보. skinned/장비·염색·광원 결과의 연결과 비교 |
| 차원술사 Q | 3~5일 | 6~10일 | 현재 2요소의 cubesample/slice 원본 식과 시각 대조 |
| 차원술사 F | 7~10일 | 14~20일 | 5cue/29요소 합성, 기존 물·유리 근사를 대체할 입력과 운동 |
| 이어서 차원술사 ALT_V | 15~25일 | 30~45일 | 신뢰 낮음. 123요소의 원본 계열·장면 입력·전체 연출 대조 |
| 합계 | 33~52일 | **64~97 개발일, 약 13~20주 작업량** | 타 클래스 전체와 대규모 새 렌더러 교체 제외 |

첫 연결이 잘 맞으면 낙관 범위에 가까워질 수 있다. 원본 입력 부재, 새 굴절/정점 변형 또는
프레임 비용 문제가 크면 위 범위를 넘을 수 있고 현재 상한을 보장할 근거는 없다. 이 숫자를
Codex의 실제 응답시간, 완료 날짜 또는 원작 100% 일치 약속으로 사용하지 않는다.

사용자가 승인한 순서를 유지하되 모코코의 첫 재질과 Q의 큐브 재질을 화면 비교까지 닫은 시점에
남은 공수를 재산정한다. 모든 단계를 한 번에 개발하지 않는다. 같은 결함이 반복되면 원본 입력,
문서 보존, 실제 shader/pass 소비, 최종 합성 중 어느 경계인지 좁힌 뒤 변경한다.

### 수천 개의 수작업 shader가 필요한가

원본 shader reference 개수는 유지할 독립 HLSL 수가 아니다. 조사한 모코코 3재질은 같은
Base PS/Directional PS를 공유한다. 모코코 3재질과 창술사 기본 상의의 선택 프로그램은
공통 VS1개와 PS4개였다. 조명·vertex factory·shadow·기능 스위치에 따른 compiled variant와
독립적인 재질 계산을 분리해야 한다.

현재 차원술사 F의 유체 2요소는 같은 opcode17 계산기를 공유한다. ALT_V의 source parent 경로는
30개지만 이를 독립 수식 30개 또는 현재 profile2개만으로 완전 복구됐다고 해석할 수 없다.
정확한 신규 family 수는 대표 native 정합 뒤에 확정한다. 수천 개 수작업 코드의 필요성이나
반대로 몇 개의 공통 shader만으로 전 효과 완료를 보장할 근거는 모두 없다.

근거: [모코코 선택 프로그램](C:/Users/user/Desktop/LostArk/out/CharacterMaterialReview20260908/source_program_index.json),
[현재 공통 재질 식](C:/Users/user/Desktop/LostArk/Client/Bin/ShaderFiles/Shader_EffectUe3MaterialFamilies.hlsli),
[Artist F 실제 재작업 기록](C:/Users/user/Desktop/LostArk/.md/GB/08-14/2026-08-14_TRACK_C_UNIFIED_EFFECT_AUTHORING_RESULT.md:51),
[중복 제거 교정](C:/Users/user/Desktop/LostArk/.md/GB/08-17/2026-08-17_EFFECT_SOURCE_TRIM_AND_DEDUP_CORRECTION_RESULT.md:29).

## G09. 현재 Drive 공유 대상과 이후 보고 방식

이번 조사·계획 갱신에서는 Resources를 추가·교체하지 않았다. 최근 링·캐릭터 축소 품질
작업에서 설치한 공유 대상은 아래와 같다. ZIP을 만들거나 Drive에 업로드한 상태는 아니다.

| Resources 기준 공유 단위 | 변경 | 이유 |
|---|---|---|
| `Resources/Map/CHARACTERSELECTMAP/` | 최근 링의 새 하위 폴더 포함 | 현재 CS 전체 공유 시 이 폴더로 묶으면 링과 기존 조명 의존성 포함 |
| `Resources/Character/LanceMaster/LanceMaster.wmodel` | 기존 파일 교체 | 눈/머리 쪽 잘못된 BRDF texture 경로 3개 제거 |
| `Resources/Character/WP_WSWP_M_06/textures/` | 기존 DDS12개 교체 | 차원술사 무기 diffuse/normal/emissive/ORM에 mip11단계 설치 |

새 링만 추가 전달할 정확한 하위 폴더는
`Resources/Map/CHARACTERSELECTMAP/MAP_D6DB636BED17_BG_ELG_ARYANORB_FLOOR13H_SM_OVR_E42AC6730343/`다.
WModel1개와 textures 하위 DDS6개가 있다. 이전 CS lighting과 다른 CS 입력이 설치되어 있는
상태를 전제로 하므로 첫 배포라면 위의 CHARACTERSELECTMAP 전체를 공유한다.
TGA247개의 mip 생성은 코드에서 처리하므로 해당 TGA를 새 파일로 배포한 것은 아니다.

앞으로 각 반영 보고에는 **Drive 공유 필요/불필요**, **추가/교체**, **정확한 Resources 상대
폴더·파일**, **이미 로컬 설치했는지**를 기록한다. JSON/HLSL/C++만 바뀌면 Git 변경으로
명시하고 리소스 공유가 필요하다고 뭉뚱그리지 않는다. 별도 Resource manifest/hash publish를
만들지 않는다. 원본 조사 중 out에 만든 참고 파일을 runtime 공유 대상으로 섞지 않는다.

## G10. 바로 시작할 모코코·무기·Q의 작업 단위

모코코와 무기는 Effect 문서가 아니라 CModel/CMaterial이 그리는 캐릭터 표면이다.
Rendering Tool은 연결된 값과 조명을 비교·튜닝하는 데 사용하고, source material 전달과
shader 소비는 코드·데이터에서 먼저 완성한다. Effect Tool에 모코코나 무기의 재질을 별도
Effect로 만들어 착용시키지 않는다.

| 순서 | 바로 할 일 | 끝났다고 판단할 범위 |
|---|---|---|
| 모코코 | 확보된 native 3재질의 S.RGBA/색상·피부 마스크/BRDF LUT를 기존 캐릭터 material override와 skinned·equipment 소비자로 연결 | 같은 조명에서 머리·상의의 재질 반응, 염색, 근거리/원거리 안정성을 사용자 확인 |
| 무기 | 우선 현재 차원술사 `WP_WSWP_M_06` 무기에서 D/N/E/ORM의 원본 채널 의미와 재질 분기를 확인해 동일한 캐릭터 경로로 연결 | 무기 표면 반사와 거칠기·발광이 해당 원본 식을 소비. mip 개선과 별도 확인 |
| Q 작업 준비 | 기존 V1 전체 복사 함수를 현재 UI에 연결하고 새 표시명·새 ID·Parent를 함께 저장 | `이펙트_차원술사Q`가 V1 저장 목록에 독립 문서로 나타나며 원본 문서 보존 |
| Q 재질 | 현재 2요소의 cube/slice를 보존하고 큐브 표면의 원본 입력·식을 복구본 전용 family/profile로 연결 | 실제 preview draw와 고정 시각·조건의 전후 차이 확인 |
| Q 도구 연결 | All Effects에 독립 복구본을 노출하고 모델·Q 애니메이션·anchor와 Play All 연결 | 기존 Product Q의 cue를 유지한 채 도구에서 두 문서를 각각 비교 |

무기 텍스처에 ORM이라는 이름이 있다는 이유만으로 모든 무기를 동일 채널·동일 shader로
처리하지 않는다. 차원술사 무기의 12개 DDS mip 교체는 이미 설치됐지만 그 사실이 새 PBR
계산의 완료를 뜻하지 않는다. 모코코 입력과 다른 원본 계열이면 별도 family로 확장한다.

현재 가능한 사용자 경로는 F1 → Effect Tool → Effect Resources → V1 → 저장 Effect 선택이다.
Create Effect는 빈 문서이므로 기존 Q 복구의 시작점으로 안내하지 않는다. 전체 복사 보강 후에는
기존 `effect.dimensionmaster.skill.2050100.unified` 선택 → 복구본 만들기 → 표시명
`이펙트_차원술사Q` → 독립 편집 → Save → All Effects 독립 선택/Play All 순서로 사용한다.
여기서 `복구본 만들기`는 **계획한 버튼이며 현재 구현됐다고 보고하지 않는다.**

이 G10과 공수에서의 무기 대표 연결 범위는 모코코 공통 경로와 재사용 가능한 부분을 공유하되,
차원술사 무기의 새 원본 family가 요구되면 해당 차이를 먼저 확인하고 추가 공수를 재산정한다.
모코코의 예산에 모든 클래스 무기 재질 전체 복구가 포함됐다고 확대하지 않는다.

## G11. 사용자 승인한 V1 확장·모코코·창술사 무기·Q 구현

사용자는 V1의 애니메이션 선택/로드/anchor와 Sequencer 두 ImGui 패널을 확장하고,
`이펙트_차원술사Q`, 모코코, **창술사 무기** 복구를 진행하도록 요청했다. G10의 첫 무기를
차원술사로 제안했던 부분은 이 요청으로 대체한다. 원본 Q·도화가 F 저작본과 다른 작업의
V2 수정, 중앙 링, mip, MirrorU, 보스 로직 변경을 보존한다.

### V1의 두 패널과 V2 보존

기존 `EffectAuthoringSequencer::Render_ModelView`와 `Render_Sequencer`를 재사용한다.
V1 선택에서 기존 All Effects를 다시 노출하고 V1/V2 편집 owner를 명시적으로 선택하게 한다.
모델 선택은 기존 CharacterPreviewPanel/AnimationTargetService, 클립은 실제 모델과 저장
skillbindings를 사용한다. 선택 anchor는 실제 이름으로 검증해 재생/Seek/Save/Load까지 연결한다.
SourceRecipe 내부의 원본 bone 부착과 바깥 occurrence anchor가 이중 적용되지 않게 한다.

ResourceTree의 새 복구본 생성 명령은 기존 V1 whole-document Save As를 재사용한다.
새 표시명·ID·Parent의 저장이 성공한 뒤에만 편집 중인 문서를 교체한다. 기존 파일 덮어쓰기를
거부하고 실패 시 원본과 편집 draft를 유지한다. 불변 raw adapter packet은 일반 문서로 변환하지 않는다.

대상 파일은 `Effect_Tool.{h,cpp}`, `Effect_Tool_Workspace.cpp`,
`EffectAuthoringResourceTree.{h,cpp}`, `EffectAuthoringSequencer.{h,cpp}`다.
MainApp의 기존 Configure/Update 연결을 먼저 사용하며 V2 Runtime/Object의 다른 작업은 수정하지 않는다.

### 차원술사 Q

기존 2요소의 전체 recipe/sourceProfile과 사용자 편집을 유지한 별도 restore 문서를 만든다.
cubesample parent/MIC의 native shader와 입력을 먼저 확인한 뒤 명시적인 재질 분기로 연결한다.
기존 grouped profile을 전역 변경하지 않는다. 복구본을 도구에서 독립 선택·재생하도록
연결하고 기존 Product Q0ms 호출은 유지한다. 새 Data 파일은 기존 `96.DataFiles` 등록 규칙을 따른다.

### 모코코·창술사 무기

확보된 모코코 native3재질과 실제 창술사 장·단창의 source material을 기준으로 입력을 확정한다.
기존 CharacterCatalog/PlayableCharacterAssetService에서 materialOverrides를 전달하고,
CModel/CMaterial → skinned/socket equipment → shader/조명의 실제 소비까지 연결한다.
캐릭터 재질은 기존 맵의 lightmap에 의존하지 않고 움직이는 표면의 원본 입력을 사용한다.
새 Data·shader 파일이 필요하면 기존 project/filter와 shader CSO producer 등록을 함께 처리한다.

### 이번 반영의 검증 경계

사용자의 앞선 빌드 대기 요청을 유지한다. 코드·JSON·리소스 구조와 유한값·정합 검사는
진행하지만 C++/shader 컴파일, Product 빌드와 Client/UI 실행은 이번 단계에서 수행하지 않는다.
따라서 코드 연결과 빌드/실행 완료를 구분해 보고한다. 필요한 Resources 추가·교체는 즉시
정확한 상대 경로로 알리고 RESULT에 남긴다. 원작 시각 품질은 사용자의 실제 확인으로 판정한다.

## G12. 중단 후 반영 전 검토에서 확정할 사항

중단 당시 검토는 기존 element의 보존된 원본 정보와 원본 재질·shader를
대조한다. shader 경로는 원본 식을 찾는 식별자다. 실제 복구는 해당 재질의 부모·static 분기를
선택하고 텍스처 RGBA, 상수, vertex/particle 값, 좌표계, 화면 입력과 출력 합성을 기존 renderer의
실제 소비자에 연결하는 작업이다. 원본 바이너리를 경로만 지정해 실행하는 방식이 아니다.
현재 저작본의 element 전체가 원작의 모든 element를 포함한다는 전제를 두지 않는다.

Q cube는 원본 MIC와 `use_emissiveclamp=false` 분기에 해당하는 shader map을 찾았다.
원본 PS는 화면 좌표를 변형해 t1을 읽고 시선 가중·색의 5차곱·caustic·UV 모서리 식을
합성한다. t0의 native fallback은 `fx_tex_06.fx_j_caustic_tile_02`이고 기존 DDS가 있다.
화면색의 정확한 공급 시점, engine 상수와 fog/보조 MRT, 생략된 sRGB/sampler 기본값은
추가 확인이 필요하다. 별도 화면색 snapshot은 이 입력을 현재 renderer에 공급하는 구현안이며
중단 당시에는 Renderer.h 선언만 있었고 복사·바인딩·Q shader 소비는 구현하지 않았다.
G13 재개에서 이를 실제 호출·복사·바인딩으로 연결한다.

모코코 3재질은 공통 native Base/Directional 프로그램을 사용한다. 창술사 장·단창은
같은 pbr_base_msk 부모 계열이지만 color/mask variation 활성과 skin 비활성 분기를 사용한다.
같은 부모라는 이유로 모코코의 식을 그대로 적용하지 않는다. 원본 재질별 조명 입력을 보존할
기존 deferred 경로의 확장 방식과 추가 light draw 비용을 반영 전에 비교한다.

복구 순서는 입력과 좌표·채널 → 원본 재질 계산 → 합성·장면 입력 → 같은 조건의 사용자
재생 비교 → 최종 조명/노출 튜닝이다. 원본에서 확인한 부분과 프로젝트에서 선택한 근사는
분리해 기록한다. 조사 산출물은 out에만 있고 runtime Resource 배포 대상이 아니다.

## G13. 재개: Q 독립 재생·캐릭터 재질·레벨 진입 종료 오류

사용자는 기존 Product 아래 All Effects에서 `이펙트_차원술사Q`를 선택하고 Play로 확인할
수 있도록 복구를 재승인했다. 큐브 원본 계산은 독립 HLSLI로 분리하되 기존 mesh renderer가
명시적인 원본 재질 profile로 선택한다. Q 원본 문서와 animevents의 제품 호출을 유지한다.
모코코, 차원술사 body, 차원술사 무기, 창술사 장·단창이 현재 캐릭터 재질 범위다.

| 작업 단위 | 변경 위치와 실제 소비 | 종료 확인 |
|---|---|---|
| 진입 종료 오류 | 기존 로그, CS 종료·쿠크 로드·map lightmap binding/수명 경로에서 원인을 확인한 뒤 해당 owner 수정 | 원인 조건과 수정 전후 코드/로그 근거, 실제 진입은 사용자 확인 |
| Q CubeSample·Slice | 기존 Effect_MaterialTemplate/DocumentRenderer, MeshPreview·Particle와 전용 HLSLI, restore authored | 원본 recipe 보존, caustic·상수·카메라·SceneColor·SceneDepth·DynamicParameter 전달, 실제 draw 경로 선택 |
| 화면색 공급 | Engine Renderer/GameInstance, 기존 TargetManager | 요청 프레임에 투명 합성 전 별도 HDR snapshot 생성·바인딩, 동일 resource 읽기/쓰기 충돌 방지 |
| All Effects 복구본 | 기존 SourceIndex/Effect_Tool/Workspace와 AuthoringSequencer | 정상 Product source와 연결된 편집기 전용 .restore, Catalog 보존, Product 아래 독립 항목, 모델·clip·anchor·Play/Seek·편집 연결 |
| 캐릭터 표면 | CharacterCatalog/PlayableCharacterAssetService → Model/Material → animated/equipment shader와 기존 lighting | source identity와 채널·분기·LUT/환경 입력 전달, 원본 불명값과 근사 경계 기록 |

새 HLSLI와 authored JSON은 기존 Client project/filter의 해당 항목에 등록한다. 프로젝트
구조를 재배치하지 않고 실제 사용 파일만 추가한다. 다른 작업의 변경은 수정 직전 diff와
백업으로 구분하며 Renderer와 Material/Model의 교차 편집은 순서대로 조율한다.
별도 툴·renderer·admission framework를 만들지 않는다. JSON/정적 계약과 diff를 확인하고,
컴파일·FXC·Product 빌드·Client 실행은 앞선 대기 조건을 유지해 미실행으로 보고한다.
새 Resources가 필요하면 설치 직후 Resources 기준 정확한 파일·폴더와 Drive 공유 필요를 알린다.

차원술사 눈의 원본 VS는 UV1/UV2, 머리카락은 UV1을 실제 소비한다. 기존 skinned geometry의
UV0 복제로 대신하지 않고 원본 vertex와 정합한 추가 채널을 기존 WModel/WMesh reader와
CMesh에 보존한다. 기존 모델의 submesh, UV0, skin weights, animation은 유지하며 새 버전
추가 시 이전 포맷 읽기와 형식 검증을 함께 유지한다. Resource 교체는 별도 정확한 경로로 보고한다.

## G14. 최신 우선순위: 차원술사 Q 먼저

사용자는 차원술사 Q를 최우선으로 두고 나머지 캐릭터 재질은 후순위로 미뤘다.
Q의 큐브·베기 무늬 전용 계산, 독립 문서와 기존 V1 Play 경로를 먼저 검증한다.
기존 Product Q 호출은 계속 유지한다. 빌드 재개 승인은 아직 없으므로 컴파일과 실행은 대기한다.

캐릭터 source material override 22행은 out의 pending 문서로 보존하고 CharacterCatalog의
활성 연결은 이번 작업 전 상태로 유지한다. 추가 UV 모델 후보도 out에만 보존하며
Resources의 기존 DimensionMaster 모델을 교체하지 않는다. 이미 작성한 공통 코드·shader
초안은 별도 checkpoint로 남기고 추가 복구 작업을 중단한다. Q 미리보기가 이 후보 설치를
전제로 요구하지 않도록 한다. 캐릭터 작업의 미컴파일 상태를 Q 시각 복구 완료로 합산하지 않는다.

## G15. Q 우선 Debug 빌드와 실행 준비

사용자가 EXE 종료를 알리고 새 실행 파일 준비를 요청하여 앞선 빌드 대기를 해제했다.
정본 Product 명령으로 Engine → Shared → Server → Client를 빌드하고 정상 SDK·CSO·DLL
배포를 확인한다. 캐릭터 새 override와 UV 모델 교체는 계속 보류한다. Client/UI를 실행하지 않는다.

현재 빌드에서 드러난 기존 작성분의 최소 오류를 수정한다. SourceCharacterAppend의
unroll 미선택 분기가 음수 인덱스를 uint로 계산하지 않도록 유효 count별 float4 조합을 사용한다.
Client.vcxproj의 PrepareEngineSdk는 BeforeTargets="FxExport;FxCompile;ClCompile"로
지정하여 Engine 정본 HLSLI가 Client FXC보다 먼저 배포되게 한다. 별도 복사 경로는 만들지 않는다.

종료 증거는 Product compile/deploy 결과, Q 셰이더 산출물과 공유 배포본 일치,
변경 XML/JSON parse 및 해당 diff 확인이다. Play/Play All과 실제 시각 결과는 사용자가 확인한다.

## G16. 차원술사 Q·V·Alt+V의 원본 전체 구성 복구

사용자는 기존 Q의 두 element 복구가 성공했다고 확인하고 세 스킬의 원본 전체 구성을
복구하도록 요청했다. 범위는 PlayerSkills의 Q=2050100, V=2050520, ALT_V=2050540이다.
기존 제품 unified와 성공한 Q `.restore`를 보존하고 각 스킬의 `.full.restore` 문서를
별도로 만든다. 표시명은 `이펙트_차원술사Q_전체`, `이펙트_차원술사V_전체`,
`이펙트_차원술사AltV_전체`이며 같은 Product 노드 아래에서 비교·재생한다.

보존된 Imported/Converted만 전체 분모로 삼지 않는다. 원본 action cue와 particle system의
emitter·외부 module·camera particle·child 생성 참조를 끝까지 대조하고 실제 occurrence의
시간·모델·재질·부착·파티클 곡선을 연결한다. 삭제된 레거시 Data 폴더를 runtime에 되살리지
않고 보존 worktree와 Git 원본은 읽기 전용 조사 입력으로만 사용한다.

SourceIndex는 `.restore`와 `.full.restore`를 각각 정확한 `.unified` source에 join하며
Catalog admission을 바꾸지 않는다. All Effects는 두 종류의 복구본을 열거하고 기존 V1
CharacterPreviewPanel/Sequencer/CEffectObject를 소비한다. 같은 역할의 툴·런타임을 추가하지 않는다.
성공한 Q의 두 요소는 transform·timing·수식을 유지한 채 전체본에 포함한다.

각 재질의 원본 parent/static shader와 RGBA·scalar·DynamicParameter·sampler 배선을
기존 carrier에 연결한다. 같은 프로그램이면 descriptor를 재사용하고 다른 식이면 HLSLI를
추가한다. 원본 scene 입력과 별도 모델·화면 연출은 실제 소비자까지 구현하고, 연결되지 않은
항목을 generic으로 치환하여 전체 복구 완료라고 보고하지 않는다.

새 Resources는 out에 준비한 뒤 필요한 Resources 상대 위치에 설치하고 추가·교체 경로를
즉시 보고한다. C++/shader 변경은 정상 Product 빌드와 배포로 검증한다. 실행 파일 사용 중에는
교체하지 않으며, 사용자가 직접 Play/Play All로 최종 시각 결과를 확인한다.

## G17. 구성 row 확장과 Q 실행 확인 우선

사용자는 Action Benchmark처럼 기존 Effect Sequencer의 occurrence 아래 구성 row를
펼치는 확장을 요청했다. 실제 document snapshot의 stable element/modelCue/anchor ID를
사용하며 모델과 자식 bone effect, 카메라 부착, light/post의 시작·수명·offset을 같은 시계로
편집·Play·Seek하도록 한다. sequence 저장 override는 문서 복사본에 적용하여 validate/stage
후 교체하고, 실패하면 이전 occurrence를 보존한다. 원본 Product/성공 Q2 문서를 변경하지 않는다.

단순 Timing 값 변경만으로 SourceRecipe의 emitter span이나 light/post lifetime이 바뀌지
않는 현재 소비 경로를 고려해 emission span과 particle tail을 구분한다. 이 row 확장은 아직
구현 전이다. 19:14 사용자 요청에 따라 먼저 Q 실행 확인을 마치도록 제품 소스 추가 편집을
중단한다. 빌드 C2664를 고친 뒤 사용자의 자체 빌드 결과를 정본으로 확인하며 추가 빌드는
중복 실행하지 않는다. 현재 구현·검증 상태는 RESULT에 기록한다.

## G18. 사용자 Q 화면 피드백: 크기·스프라이트 입력·첫 목록 표시

사용자는 전체 Q의 거대한 유리와 노란 사각형을 원본 화면과 비교하여 재수정을 요청했다.
Q에 범위를 집중하고 V/Alt+V·구성 row 확장은 계속 보류한다. 기존 두 요소 `.restore`는
비교본으로 보존하고 `.full.restore`의 10개 원본 occurrence를 교정한다.

| 확인된 원인 | 구현 위치와 변경 | 검증 |
|---|---|---|
| 새 메시 44·50의 native/cooked vertex는 cm인데 modelPreScale 누락 | full Q의 두 mesh에만 modelPreScale=0.01; 기존 cube는 그대로 | native positionstream/WModel 실측과 변환 후 크기 비교 |
| Q45·49에 Dynamic 모듈이 없는데 raw 입력이 0 | 기존 Playback source spawn에서 이 두 프로그램의 모듈 부재에만 Null Dynamic 값 (1,1,1,1) 공급 | 원본 VS→PS varying·기본 stream 근거, UV/opacity 식 및 최소 컴파일 |
| 전체본에 복사된 Q2 시험 배율과 잘못된 cue 좌표 축 | full Q의 SourceScale을 원본 1로, cue 시간·위치·회전·cube delay를 원본 파서 기준으로 교정 | 원본 활성 notify006/007 및 113개 source module 대조, 비교본 해시 보존 |
| 최초 목록이 Catalog만 읽어 독립 복구본 누락 | All Effects 첫 표시 때 Product/authoring metadata를 한 번 준비; 수동 Refresh는 이후 재검색 | 기존 lazy metadata·revision 계약 확인, Open/Play 전 문서/GPU 로드 금지 유지 |

새 C++/shader 파일과 Resources 추가는 필요 없다. 기존 재질 수식과 텍스처는 유지한다.
변경 JSON과 최소 C++ 컴파일, 해당 diff를 확인한다. 사용자 빌드와 중복 Product 빌드를
실행하지 않으며 새 실행 파일의 Play/Play All 결과는 사용자가 판정한다.

## G19. Q 전체 재생과 원본 공간 입력 재검토

사용자는 자동 목록 표시 성공을 확인했고 Family별 sprite/mesh 화면을 제공했다.
Play All의 일시 정지 후 미표시, Sequencer Play 미재생, 위치·방향·수량 차이를 수정한다.
기존 Q2 비교본과 원본 재질 계산을 유지하며 새 범용 툴이나 런타임을 추가하지 않는다.

- EffectAuthoringSequencer의 Play는 현재 transient preview도 재생 대상으로 유지한다.
  준비·샘플 성공 후 기존 occurrence를 교체하며 실패하면 이전 재생 데이터를 보존한다.
- Play/Preview 성공은 기존 interaction 요청을 통해 Effect Tool의 입력 소유권을 알린다.
  리소스 준비에 걸린 시간을 첫 재생 delta로 소비하지 않아 시작 구간이 건너뛰지 않게 한다.
- Effect_Tool의 무리소스 표시와 Q47의 실제 텍스처 없는 계산을 대조한다. 검증된 Q47에만
  procedural glow 표시를 제공하고 실제 누락 리소스를 정상으로 표시하지 않는다.
- 원본 Q10의 action cue/socket, location/velocity/rotation/size/burst를 현재 문서와
  Family·Sequencer 경로 모두에 대조한다. native/cooked vertex 축과 character-facing
  변환이 소비되는 지점을 확인한 뒤 근거가 있는 오류만 교정한다. 임의 90도 회전이나
  이미지 한 장으로 추정한 수량을 원본값으로 기록하지 않는다.

검증은 실제 상태 전환의 CPU 확인, 변경 C++ 최소 컴파일, 해당 JSON/공백 검사로 한다.
실제 Client/Server 실행과 Play/앵커의 화면 비교는 사용자에게 맡긴다. 새 Resource가
필요하면 설치 시 정확한 Resources 상대 경로와 Drive 공유 여부를 별도로 알린다.

## G20. 누적 복원 노트와 기존 입력을 재사용하는 확장

사용자는 Q를 우선 검증하면서 캐릭터·무기·Character Select 남은 재질과 향후 모든 클래스의
V/Alt+V, Valtan·Bern·쿠크 맵으로 확대하도록 요청했다. `.md/GB/렌더링이펙트복원V2.md`를
공통 재개 노트로 만들고 AGENTS/gotchas에서 연결한다. 증상·확인된 원인·실제 소비자·검증
경계·Resources 전달 필요를 갱신하며, 일별 실행 기록은 이 RESULT에 계속 둔다.

Q의 이번 공간 교정은 native class-default가 빠진 8개 분포, snapshot root basis -90도,
Cylinder의 명시적 radialvelocity 소비를 한 단위로 적용한다. 기존 LUT/곡선의 명시값과
성공한 Q2 비교본은 보존한다. 모든 생략값을 1로 채우는 전역 추측 보정은 하지 않는다.

Character Select는 현재 복구 배치와 mesh·ordered MIC·atlas pair·환경 입력이 같은
BRIDGE 442/444/458/474와 FLOOR13H 507을 먼저 연결한다. 기존 variant를 재사용하고
원본 배치별 atlas 좌표·decode 값을 각각 넣는다. 기존 가시성·다른 배치를 유지하고
저작 정본 수정 후 기존 Map publisher의 해당 맵 경로로 검증·배포한다. 후보가 실제 입력과
다르면 적용하지 않고 원인을 RESULT에 기록한다. 새 runtime/model 경로는 만들지 않는다.

캐릭터의 보류 22개 override와 추가 UV WModel은 실제 CModel admission·필수 texture·
source shader 소비를 재검토한 뒤 연결한다. Resources 교체가 있으면 경로를 즉시 보고한다.
전체 클래스/맵의 남은 식과 특수 입력은 누적 노트에서 별도 미완료 범위로 관리하며,
Q에서 보였다는 이유로 모든 같은 계열의 원본 일치 판정을 승계하지 않는다.

## G21. 차원술사 전체 restore와 Alt+V Camera row

사용자가 `로스트아크이펙트이미지/차원술사`의 레퍼런스와 초각성기 화면을 제공하고
Q/W/E/R/T/A/S/D/F/V/Alt+V 전체 복구로 범위를 확대했다. 원본 active cue와 그 자식
발생 항목을 스킬별로 다시 대조한다. 이미 연결한 native 프로그램을 동일 shader identity에
재사용하고 새 검격·유리·sprite 식은 실제 선택 프로그램과 입력 계약으로 추가한다.
조사 후보를 무조건 활성화하거나 이미지의 비슷한 색을 근거로 다른 MIC를 대체하지 않는다.

EffectAuthoringSequencer에 Camera row를 추가한다. saved sequence format3의 cameras는
stable ID, source 설명, 시작/수명, WORLD 또는 MODEL_ROOT 공간, mute와 시간별
eye/lookAt/FOV key를 저장한다. 기존 Valtan/Area의 camera cue sampler를 재사용하며
별도 시간 진행기나 보간 runtime을 만들지 않는다. v1/v2 sequence는 카메라 없음으로 읽는다.
중복 ID·잘못된 시간/pose·활성 카메라 구간 충돌을 거부하고 실패 시 이전 timeline을 보존한다.

Level이 소유한 camera는 MainApp의 기존 typed Get_DebugCamera 경계를 통해 Tool과
Sequencer에 전달한다. AUTHORING_PREVIEW 우선순위의 기존 CCamera override를 사용하고
더 높은 cinematic이 선점하면 멈춘다. Stop·구간 종료·도구 비활성·level 교체·실패에서
기존 End_PresentationOverride로 복귀한다. 카메라를 먼저 샘플하고 같은 시각의 camera
attachment 이펙트를 샘플한다. 모델/이펙트의 기존 dt 교정과 사용자 손 조정본을 유지한다.

복구본의 선택적 preset은 같은 기존 sequence 형식으로
`Data/Effects/Sequences/<effectAssetId>.effectsequence.json`에 둔다. 복구 Preview는
현재 클래스·스킬·effect ID가 일치하는 preset의 camera rows를 임시 occurrence에 연결한다.
일반 saved sequence와 임시 preview를 구분하고, 사용자 선택으로 preview+camera rows를
saved timeline에 추가한 뒤 기존 Save/Load를 사용한다. UI는 row 시간·mute·key pose를 편집한다.
원본 camera 곡선과 사진 기준 수동 보정을 구분하며 미확정 수치를 원본이라고 기록하지 않는다.

카메라 멤버 구현을 분리하는 `Client/Private/EffectAuthoringSequencer_Camera.cpp`는
현재 Client.vcxproj와 같은 기존 filters 위치에 등록한다. 실제 caller/codec/소비자까지 연결하고
변경된 sequence JSON/XML parse, 카메라 sampling·우선순위·복귀와 최소 컴파일을 확인한다.
각 스킬 문서/Resource 활성은 해당 입력 검증 후 수행하고 새로운 공유 파일을 즉시 알린다.

## G22. 차원술사 BA 네 단계와 단계별 입력·이펙트

사용자는 현재 LMB에서 한 번에 이어지는 BA를 0/1/2/3 애니메이션과 Effect로 나누도록 요청했다.
원본 clip `_01/_02/_03/_04`를 확인했고 현재 Product는 두 번째 clip이 빠진 3단계다.
첫 clip의 3000ms/rate2는 이전 프로젝트 튜닝이다. 원본 강제종료 notify를 근거로 `_01`의
1400ms 창을 쓰고 `_02/_03/_04`는 각각 1500/1067/1700ms의 자연 길이를 연결한다.

현재 Server는 window 0/0과 advance=duration인 nonfinal COMBO를 자동 연결로 취급한다.
기존 PlayerController의 press/hold 반복과 Server의 buffered input을 그대로 소비하여
추가 입력이 없으면 한 단계에서 끝나고, 추가 클릭/hold가 있으면 다음 단계로 진행하게 한다.
BA0의 100..510ms와 BA1의 0..410ms 창은 원본 입력 notify를 사용하며 BA2의 93..494ms는
이전 4단계 프로젝트 튜닝으로 명시한다. 단계별 full boundary를 유지하고 입력·이동·다른
스킬의 우선순위와 실제 iComboStage/actionStartTick 권위는 Server에 둔다.

변경은 PlayerSkills 2050010, 해당 skillbindings와 RootMotion, animevents·EffectCatalog의
BA0/1/2/3 참조, 4개의 full.restore 문서와 해당 provenance로 묶는다. 원본 활성 75개 발생
항목을 18/14/9/34개로 분리하고 terrain 대체 발생은 동시에 겹쳐 그리지 않는다.
damage 수치를 이펙트 발생 수로 증폭하지 않는다. 새 source material 소비가 준비되기 전에는
Product 참조만 먼저 교체하지 않고 후보를 out에 둔다. 기존 authored 비교 파일은 보존한다.

Effect Sequencer는 기존 skill 전체 sequence를 유지하고 COMBO의 stage별 sequence도 제공한다.
각 stage의 원래 clip ID·source window·rate를 보존하고 선택한 stage의 시작만 0으로 둔다.
Effect Tool은 Catalog/animation binding의 실제 stage를 전달하며 파일 이름에서 단계 번호를
추측하지 않는다. 단계별 미리보기와 실제 서버 콤보를 같은 시간 진행기로 혼합하지 않는다.

검증은 변경 JSON·기존 publisher·관련 기존 테스트를 사용한다. 단발 입력의 한 단계 종료,
buffered 입력의 다음 한 단계 진행, hold의 네 단계, 이동·다른 스킬 전환을 확인하고 필요한
최소 컴파일을 한다. UI/실제 Character Select 입력 및 BA 외형 확인은 사용자가 수행한다.

## G23. Q·W·R 우선과 native sprite velocity basis

사용자는 Q·W·R 원본 이미지를 제공하고 오류가 있는 이펙트를 먼저 복구하며 카메라를
뒤로 미루도록 요청했다. 기존 카메라 소스와 out 후보는 보존하되 새로운 연출 활성이나
Alt+V 후처리 연결을 현재 Q 실행 준비의 선행 조건으로 삼지 않는다. BA는 source 후보를
보존하고 실제 material 소비와 함께 활성화할 다음 단위로 둔다.

원본 Q의 offset-center VS와 현재 ParticleRect의 UV·signed corner·pivot를 대조했다.
확인된 Q native profile과 명시 PSA_Velocity에만 Make_ParticleSpriteWorld가 세로축을
3D motion에 맞추도록 한다. 실제 prepared profile을 caller가 전달하고 원본 감쇠식·
색·크기·pivot·Q2/Q10 JSON은 변경하지 않는다. zero/view-parallel 대응과 legacy 경로를
보존한다. 원본 GPU 분기와 serialized alignment에 근거한 adapter이며 stripped CPU의
selector CB 실행값을 확보했다고 보고하지 않는다.

W95/R33은 기존 Converted의 module 이름을 그대로 신뢰하지 않고 현재 원본 LOD의
ordered refs를 다시 읽는다. CDO·archetype·instance와 occurrence parameter를 투영하고,
native/cooked P·UV를 대조한 뒤 mesh .01 단위와 cue 기준축을 설정한다. 같은 selected
PS·VF·uniform-expression 계약은 기존 Q/V/Alt shader를 재사용하고 새 계산만 추가한다.
검증된 source 문서와 실제 material admission을 묶어 full.restore로 노출한다.

최소 종료 증거는 Q 새 sprite basis의 실제 함수 수치 대조, Q 재생 제어 회귀, Product 빌드와
배포, 변경 JSON/XML parse·관련 diff 확인이다. 이후 사용자가 All Effects의 Q 전체를
Play All로 확인한다. W/R 후보만 준비된 동안 제품에 반영됐다고 표시하지 않는다.

## G24. 성공 재료를 재사용하는 BA·Q·W·E·R·A 손튜닝

사용자는 맵의 현재 외형에 만족하며, 이펙트는 원본 동일화보다 성공한 사각 유리·유리 파편·
어두운 중심의 검격을 조립하는 방향으로 확정했다. 이번 구현은 BA0~3, Q/W/E/R/A다.
S는 다음 작업이며 F/V의 하늘색 유리와 Alt+V 카메라는 후속 구성으로 둔다.
첨부 이미지3은 성공 Q2, 이미지1/2는 현재 Q10이다. 기존 두 파일과 맵 데이터는 보존한다.

Q 원본10발생의 현재 단위·기본값·좌표 교정을 마지막으로 대조하고, 새 Q는 성공 Q2의
두 표현과 사각형 중심의 방사형 유리 파편으로 만든다. 단위 .01은 유지하고 파편 size5,
수량16·반경0.6m·방사속도1.2m/s를 손튜닝 시작값으로 쓴다. 성공 중앙 유리 배율은 보존한다.

| 별도 복구본 | 구성과 실제 시계 |
|---|---|
| Q `2050100.tuning.restore` | 성공 Q2 + 앞쪽 사각 유리 중심의 360도 파편 |
| W `2050120.clip2.tuning.restore` | 검은 세로 검격, size5 노란 유리5개가 위에서 아래로 순차 발생한 뒤 아래로 내려옴 |
| E `2050160.clip4.tuning.restore` | 최종 화면 확인 단계에서는 핵심6개 유지. 원본67개 구성은 별도 full restore로 비교 |
| R `2050180.tuning.restore` | 보라 검격과 보라 유리 파편을 한 묶음으로 정확히3회 재생 |
| A `2050210.a1.tuning.restore` | 기존 네 타격 시점의 보라 초승달. 사용자 최종 첨부의 밝은 안쪽 띠·짙은 보라 바깥 윤곽 기준 |
| BA0~3 `2050010.ba0..3.restore` | 보라 찌르기와 실제 swing 메시/마스크의 보라 초승달, G22의4단계 입력 계약 |

표의 모든 ID 앞에는 `effect.dimensionmaster.skill.`이 붙고 파일은 기존
`Data/Effects/Authored/<ID>.effect.json`에 둔다. W/E/A의 clip suffix는 기존 Product source에
정확히 연결하는 편집 소유권이다. 새 튜닝 문서의 시간은 전체 스킬 미리보기의 action 시간이다.
Q/W/E/R/A는 Recovery Effect에서 비교·편집하고 기존 실제 스킬 Effect는 유지한다.
BA는 누락된 두 번째 단계 복귀 때문에 PlayerSkills·skillbindings·RootMotion·animevents와
EffectCatalog를 함께 연결한다. 기존 비교 Effect 문서는 삭제하지 않는다.

공통 편집 경로는 `EFFECT_CASCADE_RECIPE_DESC::bAuthoredModuleOverrides`와 JSON의
optional `sourceRecipe.authoredModuleOverrides`를 추가한다. 기본 false는 기존 원본 모듈 잠금을
유지한다. 새 튜닝 문서에서만 true로 설정하고 기존 Module/Distribution 편집 UI를 사용한다.
`sourceRecipe.enabled`와 원본 재질·geometry·Dynamic 소비는 유지한다. 이 flag는 module 값의
저작 소유권이며 native-v14 증거나 원본 동일성 승인이 아니다. 잘못된 타입과 native 문서의
혼용은 거부하고 기존 codec의 값 검증·stage/commit·저장·재로드를 그대로 소비한다.

`Effect_DirectAuthoredSourceIndex.cpp`는 `.tuning.restore`를 `.full.restore`와 마찬가지로
정확한 `.unified` sibling에 연결하고, Effect Tool의 Recovery 목록도 세 suffix를 열거한다.
기존 캐시·CharacterPreviewPanel·Sequencer·CEffectObject를 사용한다. 새 C++ 파일이나
새 runtime은 필요 없으며 현행 project 등록에 신규 JSON만 필요한 범위로 추가한다.

Q42 유리의 몸체는 native material `color`, 검격51의 테두리는 `aura_color`를 조절한다.
원본 particle RGB가 덮어 곱하지 않도록 새 튜닝본의 색 모듈을 함께 확인하고 alpha와 Dynamic
곡선을 보존한다. 먼지 텍스처만으로 초승달이 생긴다고 가정하지 않고 실제 swing 메시를 사용한다.

검증은 새 문서의 실제 C++ codec load/save/reload, 기존 source carrier의 위치·속도·크기
수치, 네 BA 단계의 Server 입력/곡선 대응과 기존 관련 검사, 최소 컴파일/정상 배포로 한다.
외형·크기·색·밀도·Play/Seek/Loop와 실제 조작감은 사용자가 직접 확인한다. Resources는
기존 파일 재사용을 우선하며 추가/교체가 생기면 정확한 상대 경로를 기록한다.

## G25. 전체 원본 구성과 손튜닝을 모든 차원술사 스킬에서 병행

G24 진행 중 사용자가 범위를 BA와 Q/W/E/R/T/A/S/D/F/V/Alt+V 전체로 확대했다.
S 보류와 카메라 후순위는 이 요청으로 해제한다. 두 트랙은 같은 계산과 재료를 사용하되
독립 파일로 유지한다. `_전체`는 원본 활성 cue·첫 LOD의 발생 구성을 복구하고, `_튜닝`은
사용자가 선호한 사각 유리·파편·검격으로 크기·방향·색·시간을 조립한다.

현재 Q2와 Q10은 보존한다. 사용자 설명에 따라 image1/2는 Q10, image2의 중심 재료는
dustparticle 검격, image3은 성공 Q2의 사각 유리다. 정상 단위로 줄인 파편도 Q10에 존재한다.
새 파편을 빠진 리소스라고 오인하여 재추출하거나 단위 오류를 재도입하지 않는다.

T는 현재 잘 나온 mesh의 데이터와 소비를 유지하며 sprite particle을 중심으로 확장한다.
F/V 튜닝은 하늘색 유리 파편을 사용한다. Alt+V는 전방 사각형, 뒤로 당기는 카메라,
이어지는 사각형을 같은 Sequencer 시계로 조립한다. 기존 원본 camera preset은 전체본에,
튜닝 camera preset은 튜닝본에 연결하고 정지·구간 종료·도구 비활성 때 기존 시점으로 복귀한다.

full 문서는 원본 발생 수와 실제 프로그램/입력 연결 범위를 별도로 기록한다. 파일 생성만으로
원작 전체 재질 복원이나 visual 완료를 표시하지 않는다. 기존 Q/V/Alt native 계산과 grouped
selector를 실제 identity에 따라 재사용하고, 빠진 필수 입력·특수 geometry가 있으면 해당
발생의 미연결 이유를 남긴다. 정상 발생까지 제거하거나 원본 전체를 튜닝본 복제로 대체하지 않는다.

공유 편집 flag/목록/codec는 root, Q와 full 구성 트랙은 원본 담당, W~AltV 튜닝은 튜닝 담당,
BA4단과 full/tuned8문서는 BA 담당으로 나눈다. 공통 renderer와 shader는 동시에 덮어쓰지 않는다.
카메라·project/catalog 연결·최종 검증/배포는 root가 통합한다. 날짜별 실제 상태와 미완료
프로그램은 RESULT에 갱신하고 사용자 화면 검증은 기존 경계를 유지한다.

## G26. 이번 반영 범위 확정: BA·Q·W·E·R·A

최종 사용자 결정에 따라 이번 구현 범위는 G24의 BA0~3와 Q/W/E/R/A로 고정한다.
G25의 S/D/T/F/V/Alt+V 확장은 후속 작업으로 미룬다. 그 과정에서 만든 신규 후보는
out/DimensionMasterHandTuning20260908/deferred에 보관하며 이번 제품 목록에 등록하지 않는다.
새 S322 제품 연결도 보관하고 제거한다. 이미 존재하던 다른 작업의 소스와 카메라는 유지한다.

full restore는 원본 전체 구성을 먼저 비교하여 재사용할 표현을 고르는 자료다. 성능을 이유로
원본 발생을 삭제하거나 튜닝본의 핵심만 복제해 전체본으로 표시하지 않는다. W/R에서 기존
계산과 정확히 일치한 8종/22발생은 재사용하고, 신규 native 계산은 실제 texture/parameter/
geometry/scene 입력을 확인하여 연결한다. 부족한 입력은 해당 발생의 보류 사유로 기록한다.
신규 WR header와 hlsli는 Client 프로젝트 및 filters에 등록하고 실제 기존 렌더러에 연결한다.

실사용 손튜닝은 G24의 색·크기·방향·시점을 따른다. Q는 성공본2개+방사형 유리1개다.
같은 재질의 반복 발생은 기존 emitter burst와 발생 시각 곡선으로 표현할 수 있을 때 합친다.
element 개수만 줄이기 위해 서로 다른 위치와 공격 시점을 잃지 않는다. 파편 크기는 정상 단위에
연출 배율5를 적용하며 원본100배 단위 오류를 다시 넣지 않는다. element 수, 실제 particle 수,
화면 중첩과 셰이더 컴파일량은 서로 다른 비용으로 구분하여 기록한다.

최종 연출 명세는 위 G24 표다. full은 원본 구성·수치, 손튜닝은 사용자 명시 구성을 소유한다.
E는 최종 사용자 결정에 따라 튜닝6개를 유지하고 원본67 발생은 full에서 비교한다.
A의 이전 초록/직선 및 R의 원본 시점4회 해석은
현재 명세가 아니다. A는 첨부 `codex-clipboard-e7c08b0f-cb32-4d85-a7b8-5782dc53d7d7.png`의
보라 초승달로, R은 보라 검격+보라 파편3회로 구현한다. 원본 재질 조사 결과를 이유로
사용자가 지정한 손튜닝의 색·형태·횟수를 자동 변경하지 않는다.

사용자는 이 상태를 먼저 눈으로 확인하기로 했다. E 잔여13 MIC/12 PS의45발생용 새 입력과
계산식은 out 후속 후보로 보관하며 이번 제품 연결/컴파일에 추가하지 않는다. 이번 full E는
원본67 발생을 보존하되 실제 활성22/미연결45를 구분해 보고한다.

종료는 위 범위의 문서 로드·저장·재로드, BA4단 데이터 배포, 필요한 C++/셰이더 빌드와
JSON/XML parse 및 변경 diff 확인까지다. 원본 미연결 재질과 사용자 화면 확인은 별도로 남긴다.

기존 BA `.unified` 3개는 파일과 내용을 보존하고 기존 EffectAuditionCatalog로 분류한다.
이전 순서인 ba1→새ba3, ba2→새ba0, ba3→새ba2의 실제 clip 소유권을 명시한다.
Product Catalog에는 새 BA0~3만 연결하며, 비교 문서는 새 Product 원본의 현재 hash를 가진
기존 audition 계약을 사용한다. 회복본 파일의 원본 소유권 판정은 기존 source validator에도
반영하여 `.restore/.full.restore/.tuning.restore`의 정확한 player sibling만 허용한다.

## G27. F·V·Alt V 전체본과 핵심 손튜닝 및 기존 Sequencer 카메라 연결

사용자의 후속 요청으로 F(2050230), V(2050520), Alt V(2050540)를 추가한다. BA/Q/W/E/R/A의
확정 결과는 유지하며 S/D/T와 미연결 E 신규 프로그램은 이번 범위에 포함하지 않는다.
full은 원본 F69, V42, Alt V344 발생과 Alt V model cue1의 구성을 보존한다. 현재 실제
shader/parameter/texture/geometry identity와 일치하는 프로그램을 먼저 재사용하고, 미연결
계산과 module/geometry 입력은 발생별로 보류한다. full의 생성·활성·원본 동일성은 구분한다.

손튜닝은 기존 Q42 유리 계산의 몸체 색과 출생시각별 distribution/burst를 사용한다. F는
.7/1.4초의 중앙 mesh와 하늘색 주변 파편, V는 .6909초의 중앙 mesh와 하늘색 확산 파편을
각각2 element부터 구성한다. Alt V는 .7초 보라 큐브1개와 2/2.25/2.5/2.75/3/3.25초 하늘색
큐브6개, 3.95초 하늘색 마무리 파편을2 element로 조립한다. 정상 단위를 유지하고 성공 Q2
크기와 size5 파편을 연출 값으로 사용한다. 색·크기·밀도는 사용자 화면 확인 후 조절한다.

카메라는 기존 EffectAuthoringSequencer의 format3 MODEL_ROOT camera track, 같은 preview
시계와 소유권/해제 경로를 재사용한다. 기존 Alt V full 원본 camera preset은 보존하고 새
tuning preset은 보라 큐브를 보여준 뒤2~3.25초 순차 큐브 때 줌아웃하도록 연결한다.
5.1초 실제 animation clip과 effect/camera duration을 대조하고 Stop/종료 때 원래 시점으로
돌아가는 기존 경계를 확인한다. 별도 camera runtime이나 자동 Client 실행은 추가하지 않는다.
카메라 연결 자체가 막히면 해당 preset만 후속으로 남기고6개 effect 문서의 준비를 막지 않는다.

F/V full, F/V/Alt V tuning, Alt V full은 각각 분담하고 camera/project 등록과 통합 확인은
root가 맡는다. 새 JSON과 Alt V tuning sequence는 기존96.DataFiles에 필요한 항목만 등록한다.
기존 Product skill effect는 바꾸지 않고 Recovery Effect에서 full/tuning을 비교한다. 필요한
Alt V cube geometry는 기존 CModel과 변환 경로를 사용하고 물리 Resource 추가와 정확 재질
override 복원 상태를 분리한다. 미참조 리소스 팩이나 새 manifest는 만들지 않는다.

현재 사용자가 실행한 빌드는 중지하지 않으며 에이전트가 Product 빌드를 중복 실행하지 않는다.
실제 C++ codec의 load/save/reload, source sibling, burst/색/위치/속도, camera schema/시계,
참조 Resource 존재, JSON/XML 등록과 변경 diff를 확인한다. 새 compiled source가 꼭 필요하면
현재 빌드와 충돌하지 않게 처리하고 정확한 미컴파일 범위를 인계한다. Client 화면과 카메라
외형의 최종 판단은 사용자가 직접 한다.

G27 진행 중 사용자는 시작 큐브 면에 맵과 캐릭터가 담긴 스크린샷을 제공하고 가능하면
캡처 연출도 추가하도록 요청했다. 원본 SceneCapture notify와 큐브 재질의 screencapture
입력을 조사한다. 일반 Q42의 현재 HDR 굴절을 원본의 저장된 캡처로 표시하지 않는다.
실제 캡처 view/투영 및 texture lifetime 소비까지 연결된 경우에만 반영 상태로 기록하며,
해당 연결이 병목이면 사용자의 카메라 병목 허용에 따라 이번6문서 준비와 분리한다.

## G28. 사용자 저장본의 검격·crack 입력 수정과 Effect Tool 열기·저장 지연 개선

9월9일 사용자는 full restore에서 필요 없는 element를 삭제하고 원본 sprite를 붙여 넣어
손으로 편집하는 방식을 확정했다. 현재 R full30/A full104와 W에서 저장한 transform을
그대로 보존한다. 이전33/117행을 다시 채우거나 R/A의 위치·시간을 새 손튜닝으로 조립하지 않는다.
중간에 제안된 R ±30도 재배치는 최종 설명에 따라 구현 대상에서 제외한다.

핵심 대상은 A의 원본 초승달과 R의 검은 중심·보라 경계 sprite 검격이다. Q에서는 작아서
덜 보였던 깨짐이 확대 시 두드러진다는 사용자 관찰을 기준으로 실제 원본 선택 shader,
texture 채널, UV/alpha/Dynamic 입력과 mesh varying을 추적한다. A의 현재 particle10은
보라 유리, sprite22는 검은 noise라는 사용자 구분을 occurrence 진단에 사용하고, 파일의
현재 stable element ID로 대응시킨다. native 식/입력의 잘못된 연결을 고치며 외형 PASS는
사용자 판단으로 남긴다. 같은 문제의 BA sprite는 exact 기존 native9 MIC/19발생을 먼저
재사용한다. alpha가255인 source texture를 일반 alpha sprite로 표시해 사각형이 되는 경우
원본 채널·절차 mask를 연결한다. BA0의 사용자 확인값과 실제4단 timing을 보존한다.

Q의 주변 파편은 중앙 성공 큐브와 분리한다. 실제 Q50 `fm_d_crack_032.wmodel`과
local-crack 재질을 사용하고 modelPreScale.01 및 원본 StartSize1 위의 size2로 교체한다.
W/F/V와 Alt V의 마무리 파편도 cube로 잘못 연결한 부분만 같은 crack 재료로 교체한다.
Q50은 in_color/out_color/refle_color와 필수 Dynamic 입력을 소비하므로 Q42 color 필드를
그대로 복사하지 않는다. W 사용자 transform과 기존 출생·속도·색 의도는 보존한다.
Alt V의 중앙 보라/순차 하늘색 큐브는 유지하며 마지막 파편은 geometry가 다르므로 별도
emitter로 분리한다. element 수를2개로 유지하기 위해 파편을 큐브로 바꾸지 않는다.

BA2/3 열기 실패는 현재8문서 codec와 audition source hash에서는 재현되지 않았다.
실제 Open/선택 source/COMBO 소유 경로와 read-only 로그로 원인을 좁힌 뒤 해당 소비자를
수정한다. parse 통과만으로 Open 문제를 해결했다고 보고하지 않는다. 전체 source 재사용
수정은 BA0/1/2/3 full75의 실제 재질 연결만 대상으로 하고 손튜닝BA0와4단 입력은 유지한다.

Effect Tool 열기와 저장은 별도의 C++ 작업이다. 현재 Saved Effects의 Read_V1Inventory는
파일명 목록에97MB 이상의206개 authored JSON 전체를 parse한다. Attach_Saved가 저장마다
Reload를 다시 호출해 같은 전체 parse와 V2 inventory를 반복한다. 목록은 stable ID/이름
metadata만 읽고 실제 문서 validation은 기존 Open/Play owner가 수행하도록 범위를 좁힌다.
저장 성공 후에는 바뀐 row만 갱신한다. metadata의 오류 표시와 사라진 파일, source identity
검사는 해당 입력에서 보존한다. 일반 Save의 Save_AtomicIfUnchanged와 원본 충돌/rollback
검사는 유지한다. 현재 Save는 Sequencer 복구본에도 별도 WorldPreview를 다시 만들므로
실제 preview owner에만 적용하도록 연결을 확인하고 중복 GPU resource stage를 제거한다.

분담은 root가 Tool/ResourceTree 및 저장 성능, 원본 검격 담당이 R/A JSON과 native 식,
crack 담당이 Q/W/F/V/Alt V 파편 JSON, BA 담당이 BA data/Open 진단이다. 공통 Renderer,
Codec, MaterialTemplate와 shader entry 변경은 root와 조정한다. 현재 파일 bytes를 먼저
out/DimensionMasterSlashFix20260909/before에 보존하고 사용자 재저장이 있으면 다시 대조한다.
새 C++ 파일·별도 runtime은 우선 필요 없으며 새 JSON이 생길 때만96.DataFiles 등록을 추가한다.

검증은 실제 codec load/save/reload와 source/material 입력, 단위·색·burst 수치 및 BA Open
실패 경로, 목록/Save의 좁은 CPU 시간 비교로 한다. 필요한 최소 C++ 컴파일과 shader 검사를
수행하되 사용자의 Product 빌드·Client/Server process는 조작하지 않는다. 사용자 제공 이미지
열람은 수행하며 Client/UI 실행·화면 캡처·visual PASS는 수행하지 않는다.


## G29. 사용자 선별본 보존, 원본 크기 파편과 Alt V box 중심 연출

사용자가 09-09 추가로 확정한 순서는 full restore에서 원본 계산과 원본 발생을 복구한 뒤
개별 재생으로 확인한 핵심만 남기는 것이다. W full20행, E full64행 및 Alt V full340행의
Screen Post4행 삭제는 사용자 저장 결과이므로 되돌리지 않는다. 새 크기 요청은 Q 튜닝
파편에 full Q crack의 StartSize와 SizeMultiplyLife 원본 분포를 동일하게 쓰는 것이다.
이전 2배·5배 배율과 큐브에서 가져온 크기 커브를 유지하지 않는다. 발생 위치·방향은 보존한다.

Alt V 튜닝은 source mesh particle67의 굴절 box,70의 edge box, 원본 캡처 box 및
하늘색 순차 box/crack를 좁은 구성으로 사용한다. camera-facing box는 원본 cm geometry의
modelPreScale=.01을 한 번 적용한다. 기존448키 카메라를 이 튜닝 자원 ID에 연결한다.
Scene Capture는 현재 엔진이 완료한 연출 직전 장면의 GPU HDR 복사본을 occurrence가
보관하며 같은 box의 원본 RT0 색·왜곡 계산으로 읽는다. 원본 미해석 engine CB0[0..5]의
동일 복원이라고 하지 않고 프로젝트의 box-local 투영 adapter임을 결과에 명시한다.
사용자 화면 캡처·UI 자동 조작은 하지 않는다. 실패하면 기존 occurrence를 보존한다.

F/V는 현재 full의 실제 source native 연결을 재점검하고, S30/D51 원본 발생 문서는
기존 추출 정본에서 생성한다. 기존 Q/V/ALT/WR과 원본 PS/VF가 같은 재질은 재사용하며
이미 조사된 S native320–323의4계산을 실제 codec/renderer/particle shader에 연결한다.
미지원 재질·모듈은 원본 데이터를 지우지 않고 해당 행만 사유를 남겨 실행을 보류한다.
새 데이터는 audition/source index·프로젝트 Data None 등록에 연결하고 Product skill은
기존 authoring 소비 경로를 유지한다. 새 C++헤더/셰이더 include는 vcxproj와 filters에 등록한다.

G28 목록 최적화는 전체206문서 이름 일치, 범위제한 metadata읽기, 저장 시 해당 행 갱신,
외부 변경 충돌 보존을 실제 C++ CPU probe로 검증한다. Sequencer Save는 기존 모델/카메라
소유자를 바꾸지 않고 저장한 resource의 occurrence만 현재 시점에 갱신한다. 동기 저장 시간은
다음 playback dt에 넣지 않는다. 끝에는 최소 C++컴파일·FXC·actual codec와 Drawable·
JSON/XML parse·이번 범위 diff-check를 확인하고 Product 실행과 화면 판정은 사용자에게 남긴다.

### G29 사용자 추가 확정: 반복 박스와 shader build 중복 제거

Alt V는 원본 camera-space 박스와 source model cue의 큐브 반복 애니메이션을 메인으로 한다.
원본 mesh ordinal 1~4,16~19,67,69,70,72,77,78,96~106의25행, 마무리 crack1행과
원본 CModel cue1개를 tuning 문서에 둔다. 원본 sourceRecipe/attachment/timing을 유지하며
body 투명도·파란 경계·HDR 발광과 cm geometry 단위만 저작 조정한다. full340은 보존한다.
기존 frame HDR 굴절과 frozen starting-scene capture는 구분하고, native skinned model-cue
capture override는 미연결 경계로 남긴다. 초기2개 capture mesh에는 원본116 RT0 식과
프로젝트 cube-face UV 투영, occurrence GPU 복사본을 연결한다. Save refresh는 이전
occurrence의 frozen capture를 공유하며 다른 resource/occurrence의 캡처로 바꾸지 않는다.

사용자의 긴 셰이더 빌드 문의에 따라 기존 결과의 SHEX를 비교한다. 동일 VS/PS가 메시7벌,
파티클5벌 존재하는 경우 두 entry HLSL에서 shader object를 각각 한 번 compile하고
동일 순서의 render-state pass에서 공유한다. pass index/name, blend, cull, depth와 함수
본문은 이 최적화에서 변경하지 않는다. 실제 Debug /Od /Zi로 최소 FXC 두 파일을 확인하고
CSO shader 개수·크기와 실측 시간을 기록한다. 일반 JSON 손튜닝에는 FXC를 요구하지 않는다.


## G30. 제품 증분 빌드와 ImGui 복구 목록 병목

사용자가 Debug 제품 빌드를 승인했다. 정본 Product runner로 Engine→Shared→Server→Client를
실행하고 단계별 시간을 확인한다. 같은 작업 폴더의 S 세션이 소유한 renderer V63 수정은
읽기만 하며 이 G에서는 셰이더 식과 renderer를 추가 수정하지 않는다.

기존 Effect_Tool.cpp의 Render_RecoveryEffectForProduct는 스킬 트리를 그리는 것만으로
형제 restore/full/tuning 문서를 Refresh_UnifiedEffectCache로 모두 parse/Drawable 검증한다.
목록은 기존 source index의 ID를 사용하고 Play는 Try_PlaySavedUnifiedEffect, Open Editor는
Try_LoadDocument의 기존 검증 경로를 사용한다. 요소 열람은 해당 행 Show Elements 명령에서만
정확한 cache를 준비하고 실패 이유와 재시도 버튼을 보존한다. 명령 중 index가 갱신될 수 있어
선택 Binding을 값으로 보관하고 cache iterator를 재탐색한다.

Render_UnifiedEffectTree와 Render_ActiveAuthoredEffectTree의 열린 Family는 전체 원본 순서의
Element 포인터 목록을 만든 뒤 ImGuiListClipper로 보이는 고정 높이 행만 그린다. ordinal은
전체 Family index+1이고 선택/마킹/Solo는 기존 stable Element ID로 수행한다. 숨겨진 행을
runtime에서 제거하거나 timeline 재생을 바꾸지 않는다. 새로운 저장 schema/파일은 없다.

수정 뒤 Product 증분 빌드로 실제 EXE까지 연결하고 셰이더 재컴파일 여부를 로그로 확인한다.
이미 통과한 FXC를 임의 복사하거나 tlog를 바꾸지 않는다. JSON/XML과 변경 delta 공백을
확인하며 실제 UI 스크롤·선택·Solo와 체감 시간은 사용자가 직접 확인한다.

## G31. S Shine V63의 원본 속도 정렬 소비 연결

S 원본 조사에서 Shine `fx_e_pa_ht_18_4_tr`의 offset-center dynamic VS는 Q에서 복구한
`2dd6d96a7e6c974fac82106409a5b9b8`와 ID·보존 ASM bytes가 같다. S4개와 V2개의 현재
V63 발생은 모두 Required의 `PSA_Velocity`를 명시한다. 그러나 renderer의 native basis
적용 목록에는63이 없어 camera-plane atan2를 선택한다. 원본 계산의 정확 consumer만 확장한다.

수정 파일은 기존 `Client/Private/Effect_DocumentRenderer.cpp`다. `Make_ParticleSpriteWorld`
안 `CAMERA_VELOCITY`의 `bNativeVelocityBasis` 조건에 profile63을 추가한다. 기존
`SourceRecipe.bEnabled`와 element 존재 조건, zero/view-parallel fallback, finite 검사를 유지한다.
직접 호출자는 particle instance를 준비하는 `Render_Particles` 경로이며 실패하면 기존
`Fail_RenderOperation`으로 원인을 보존한다. H 계약·저장 schema·shader 수식은 변경하지 않는다.

부호와 pivot은 원본 literal .9/.1을 보존한다. 음수 size에 대한 원본 CPU→VS 입력은 별도
대조 중이므로 이번 basis 연결에서 pivot을 임의 반전하거나 particle 데이터 값을 바꾸지 않는다.
단순 화면 평면 회전에 상수90도를 더하는 방식도 사용하지 않는다. 새 C++/HLSL 파일이 없어
vcxproj/filters 신규 등록은 없다.

검증은 기존 Q의 실제 `Make_ParticleSpriteWorld` 추출 CPU 검사 방식을 V63에 적용하여
원본 VS의 U/V 식과 네 모서리 위치를 비교한다. 비대상 profile·비속도 정렬·퇴화 방향·실패 시
출력 보존도 확인한다. 현재 변경 translation unit만 별도 중간 출력 경로에서 최소 컴파일한다.
다른 세션의 제품 빌드와 같은 IntDir/OutDir를 쓰지 않으며 Client/UI 실행·화면 판정은 하지 않는다.
최종 source diff·실행 결과·아직 제품 EXE에 포함되지 않은 경계는
`09-09/2026-09-09_DIMENSIONMASTER_S_MATERIAL_FORENSIC_RESULT.md`에 기록한다.

## G32. 09-09 재개: Solo·운동·검격·반구·박스 복원 2차와 수명 정리

사용자가 최신 main pull, 제품 빌드, 메모리 누수 교정과 S/R/A 검격, D/V 반구·crack,
V 파편, Alt V box의 두 번째 복원 및 PR/merge를 요청했다. 기준은 PR344를 포함한
`94e90fd9`이며 `codex/dimensionmaster-effect-round2`에서 구현한다. 기존 full에서 사용자가
선별한 행·Transform과 제품 `.unified`를 보존하며, 원본 입력이 확인된 소비 누락만 고친다.

`CEffectTool::Build_PreviewDocument`의 Element/Group Solo는 선택 요소가 참조하는
model cue까지 지워 Alt V child의 codec 검증을 실패시킨다. 필요한 model cue를 숨긴 pose
공급자로 유지하고 실제로 stage한 preview 문서를 Update/Seek의 anchor 수집에도 사용한다.
리소스 준비 직후 첫 delta를 건너뛰어 짧은 검격의 수명이 준비 시간으로 소모되지 않게 한다.
V1/V2의 F1 entry·focus·draft·typed open owner를 기존 호출자와 focused 검사로 다시 확인한다.

`CEffectPlayback`은 실제 source 모듈과 CDO를 대조해 cylinder spin의 축/방향 선택,
vector field의 원본 XYZ와 Client XZ 좌표 변환, 보류 Orbit 옵션을 단계별로 연결한다.
`CEffectDocumentRenderer`와 native material descriptor는 선택 MIC의 two-sided override와
실제 draw 입력을 함께 검사한다. V70 반구의 원본 `overridedtwosided=true`는 parent만 읽은
기존 one-sided 상태를 교정하는 근거다. 재질·geometry·수명 검증을 우회해 hidden 행을
일괄 활성화하지 않는다. 필요한 데이터 수정은 exact 발생별로 기존 문서에 반영한다.

`CEffectV2Runtime::Release_Resources`는 shutdown owner다. Loader와 Level의 객체가
해제된 뒤 target/group callback·snapshot·document를 비우고 마지막에 기존 V2 GPU cache와
particle buffer pool을 정리한다. `CMainApp::Free`에서 호출하며 tool close나 world transfer의
일반 Reset과 구분한다. 종료 시 의도치 않은 cache 잔류와 실행 중 지속 증가 누수를 별도로
검증·보고한다. 새 C++ 파일과 프로젝트/filter 등록은 현재 변경안에 필요하지 않다.

검증은 Debug 제품 빌드, 실제 codec Load/Save/Reload와 선택 발생의 fixed-step·finite·
draw 입력, 기존 focused tool 계약, shutdown의 CPU/COM 수명 검사와 변경 JSON/XML parse,
diff 공백 검사다. Release도 제품 빌드를 수행한다. Client/UI는 실행·조작·캡처하지 않고
사용자가 F1에서 직접 재생할 경로와 미완료 원본 입력을 RESULT에 남긴다. 검증된 변경을
기능 PR로 올리고 해당 head를 확인한 뒤 merge한다. 화면 일치와 누수 전체 부재를 자동
검증 성공만으로 완료 처리하지 않는다.

### G32 추가 연결: D crack의 원본 재질 슬롯 두 개

D full의 `7508b2684fd272292d7b`는 `fm_d_crack_037`의 source materialIndex0/1에
LocalCrack과 Ice를 각각 적용한다. 실제 모델도 두 슬롯으로 나뉘므로 한 재질을 전체 mesh에
덮는 방식으로 활성화하지 않는다. 기존 `detail.mesh`에 선택적
`sourceMaterialSlots[{sourceMaterialIndex, material}]`을 추가한다. index는 WModel에 보존된
원본 재질 슬롯 번호이며 현재 mesh vector의 위치나 새로운 placement 저장 ID가 아니다.

각 슬롯의 material은 기존 `EFFECT_MATERIAL_DESC` parse·validation·resource 준비 경로를
재사용한다. `CModel`은 특정 mesh의 원본 materialIndex를 읽기 전용으로 제공한다.
Renderer는 같은 CModel과 한 번 평가한 particle/clock/transform을 공유하고, 슬롯별 준비
자원을 기존 `Render_Mesh`에서 해당 source materialIndex에만 적용한다. 중복·없는 슬롯,
원본 MeshMaterial ObjectPath 불일치, 미지원 재질과 리소스 누락은 이유를 보존한다.
실제 모델의 모든 슬롯을 검증·준비한 뒤에만 commit하며 실패하면 기존 preview를 유지한다.

slot0은 기존 V66 LocalCrack을 사용한다. slot1의 Ice는 선택 PS
`046f090de8eb2f408bbb8debf92fd28f`, flocal VS `8562847977cf324b900feff85799f43a`의
확보한 식을 기존 SD native324에 추가한다. 일반 2D sampler3개와 tangentView를 사용하며
다른 유리의 SceneColor/Depth나 임의 donor 식을 끼우지 않는다. 기존 SD descriptor/HLSLI와
mesh shader entry를 확장하므로 새 C++/HLSL 파일과 project/filter 등록은 없다.

실제 D 모델의 두 슬롯·형상 보존, source uniforms/texture 경로·원본 PS 수치, 두 재질의
분리 draw, codec 왕복·잘못된 슬롯의 실패 보존을 확인한다. Engine/Client와 변경 shader를
다시 빌드하고 새 Resources 설치 여부를 RESULT에 기록한다. 최종 하늘색 crack/Ice 모양은
사용자의 실제 Solo/Play 관찰로 판단한다.


## G33. 사용자 화면 피드백 이후 선택·저장 병목과 R/S/F 재생 연결

기준 HEAD는 PR345 병합 `591012db`이며 새 branch는 `codex/dimensionmaster-tool-round3`이다.
시작 시 타 작업 Kouku 변경 및 사용자가 저장한 Q/R/S/F 등의26개 dirty 파일을
`out/DimensionMasterRound3_20260909/initial_worktree/`에 원문 보존했다. 타 작업 파일은 수정하거나
자동 stage하지 않는다. 현재 사용자 튜닝을 데이터 기준으로 삼고 삭제한 occurrence를 무차별 복구하지 않는다.

첨부 R 원본은 이어진 넓은 검은 중심과 보라색 발광이고 현재 R tuning은 여러 검은 선과 점무늬다.
A 초승달과 S 바닥/유리/주변 효과는 사용자 관찰상 합격이며 현재 상태를 보존한다.
48 mesh와26/38번은 Q가 아닌 A full에 있어 4회 편성의 대상은 사용자 질문 응답을 따른다.
마지막 정정대로 Q 유리는 S mesh01/02의 fm_d_crack_037을 금색으로 재사용한다.

### Effect_Tool.cpp / Effect_Tool.h

기존 `Render_DataFilesWindow`는 authoring resource tree 분기의 조기 return 때문에 그 아래
category/search/element/family/Add Element UI에 도달하지 않는다. 같은 창에서 기존 typed facade를
유지하면서 Data Files 선택지를 다시 노출하고, 기존 복사·stable ID 생성·의존 cue remap 경로를 사용한다.
Shift 선택의 표시 상태 변경은 문서 내용 mutation과 분리한다. 선택만으로 전 문서 validation,
prepared resource 교체 또는 전체 JSON serialize를 반복하지 않게 현재 호출자/무효화 지점을 확인해 고친다.
Save는 현재 bytes 비교와 validate→stage→commit 및 실패 시 이전 파일/preview 보존 계약을 유지하며
중복 serialize·전역 색인 재구축·동일 자원 재준비를 줄인다. 마지막 명시 검증 문구를 실제 화면/모든
Element의 실행 성공처럼 표시하지 않는다. Life x는 UI와 codec 모두 현재16배 상한이며 초가 아니다.

### Q / R tuning authored JSON

`2050100.tuning`의 기존 cube/검격/튜닝을 보존한다. 기존 glass-ring 위치에 S crack01 복사본을
연결하고 crack02를 별도 stable ID로 추가한다. 두 행은 기존 mesh/material/source module 경로를
재사용하며 금색 in/out/reflection 재질 파라미터와 중심에서 원형으로 퍼지는 cylinder/radial velocity
저작 override를 사용한다. 원본 S 복사 제공자는 이 변경으로 수정하지 않는다.

`2050180.tuning`의 세 slash와 cube는 사용자 위치·타이밍을 기준으로 삼는다. 대응 full의 WR260
swing mesh 세 행을 각 slash 시작 시각에 복사하고, S crack 두 행을 보라색으로 각 검격 양옆에
연결한다. 기존 cube도 같은 시간의 방출을 유지하며 검격 주변으로 퍼지는 속도를 준다.
복사 시 stable ID와 group을 새로 부여하고 sourceNode는 복사 원본을 기록한다. raw 추출 정본 대신
저작 override라는 상태를 명시한다. A 합격 재질은 수정하지 않는다.

### Playback / Renderer / source shader 연결

현재 F mesh15개 중14개는 저장된 failClosed hold이며12개에는 native profile ID가 없다.
BA의 ready 문구나 grouped proxy 통과를 원작 재생 성공으로 보지 않는다. 각 막힌 행에서 필요한
source module과 material/geometry를 확인하고, 같은 MIC의 이미 연결된 runtime은 재사용한다.
S sprite24 중 ALT129 한 행은 burst500/local vector field를 사용한다. 기존 실제 Playback의
focused 수치 측정으로 비용을 분리한 뒤, 동일 입력을 입자마다 다시 해석하거나 과거부터 반복 적분하는
비용을 줄인다. 입력/Seek/loop 결과와 자원 stage 실패 보존은 유지한다.
R 줄무늬와 S 메인검격/screw의 실제 source→geometry→shader 호출을 대조한 뒤 구체 변경을
이 G에 추가하고 구현한다. shader/재질 연결 누락을 운동 문제로 가정해 hold만 해제하지 않는다.

검증은 변경 기능 최소 컴파일, 현재 JSON의 Load/Save/Reload, Solo/시간 sweep의 유한수·발생수,
선택/저장/Playback 전후 CPU 비용과 실패 시 기존 항목 보존, diff-check다. 새로운 광역 admission
체계는 만들지 않는다. Client/UI는 실행하지 않으며 최종 화면은 사용자가 같은 F1 V1 경로에서 판단한다.


### G33 R 중심 면과 S 기존 저작 연결 확정

R tuning은 Q51 glass-hole PS의 원형/polar 마스크를 긴 sprite로 늘려 사용한다. 원본과 비교한
31개 named parameter의 차이는 aura 색뿐이며, alpha 입력 texture의 빈 구간이 긴 선으로 보인다.
Q51 native 식을 바꾸지 않고 R에만 `effect.project-tuned.dimensionmaster-r-glasshole-solid-core.v1`
식별자를 연결한다. 기존 Q51 descriptor/texture/packet을 재사용하는 alias이며 여유 parameter11.x/y에
`blackCoreWidth/blackCoreSoftness`를 바인딩한다. wrapper는 중심 alpha를 native alpha와 합성하고
premultiplied 기여를 나누어 기존 보라색 aura를 보존한다. 기존 Q alias parameter0은 정확히 이전 경로다.
원본 shader 누락 복구로 표기하지 않고 사용자 요청의 R 전용 저작 조정으로 기록한다.

S 현재 full의 source occurrence는 crack mesh2개뿐이다. unified에 이미 저장된 사용자의 cone003
하나와 helix036 둘은 full에 연결돼 있지 않다. 해당 standard material·texture·transform·particle
계약을 기존 CModel renderer로 복사하고, 새 stable ID/sourceNode와 명확한 main/screw 표시 이름을 둔다.
현재 S hit의1.09361696초에 맞춰 full의 기존27행을 유지하며3행을 추가한다. burst 입자 수명과
scale lerp를 실제 Playback으로 확인한다. 이3행은 원본 native 증거가 아닌 기존 저작 복사임을 명시한다.

S Playback의 ElementWorld는 같은 fixed tick/element 안에서 입자마다 반복 계산되고 있다.
입자 의존 입력이 없는 계산만 element loop로 올리고 실제 이전/이후 frame 값과 CPU 시간을 대조한다.
R의 Life16은 duration과 particle lifetime/normalized curve에 이미 반영되므로 별도 early-expiry 수정을
추측으로 추가하지 않는다.


### G33 F crack와 broken의 기존 runtime 재사용

F의 `93c317e4f8a6ce1fb9c3/68431dd9f54ea0fd1110`은 D `7508b2684fd272292d7b`와 같은
fm_d_crack_037 및 동일 MeshMaterial_2 원본 슬롯을 참조한다. 두 행에 D의 V66/SD324
`detail.mesh.sourceMaterialSlots`를 복사해 실제 materialIndex0/1을 함께 연결하고 visible을 켠다.
원본 primary hold와 F의 각 time/source recipe는 보존한다. `6bc633623ba8718a150d/ab69f8cb4c04d30fe391`
broken 둘은 현재 R의 동일 MIC WR263 native material을 재사용해 hold를 해제한다. 이 네 행은 새
shader나 우회 renderer 없이 기존 stage/codec/Playback 경로로 검사한다. 나머지 미연결 profile을
이 네 행의 성공과 합쳐 전체 F 복원 완료라고 기록하지 않는다.


### G33 실제 재현된 저장 중복과 1ms burst 누락

Save는 새 문서를 Tool과 Codec이 각각 serialize하고, 저장 기준본도 이미 canonical인 디스크 bytes를
다시 Parse→Serialize한다. Codec은 성공 시 실제 기록한 canonical bytes를 optional output으로
돌려 Tool의 baseline/rollback 키로 재사용한다. 기준본 bytes가 기대 canonical과 같으면 바로 통과하고,
다르면 기존 Parse→Serialize 비교를 수행한다. 임시 파일 왕복 검증과 외부 변경 거부는 유지한다.

S main-slash는 시작1.09361696초/방출창1ms여서 다음 fixed tick1.1초가 창 끝을 지나 실제 입자0을
만든다. 이미 존재하는 `Is_ManualBurstOnlyParticle/Calculate_ManualBurstFirstSpawnStep`을 Step에서도
사용해 수동 burst가 최초 해당 tick에 정확히 한 번 태어나게 한다. 이후 중복 방출은 막고 emitter
창과 실제 particle life.3초를 구분한다. 1ms 데이터를 임의로 늘려 근본 원인을 숨기지 않는다.


S500 파편은 실제 Debug CPU fixed-tick 검사에서 평균26ms 수준이며 단순 ElementWorld hoist만으로는
차이가 작다. LocalVectorField의 동일 module literal, field lookup, field transform/inverse는
같은 element/tick에서 공통이므로 tick-local context로 한 번 계산해 기존 입자 update에 전달한다.
입자별 field sample, age curve, RNG와 속도/위치 결과는 유지한다. 영구 cache나 두 번째 simulation을
만들지 않고 이전 frame hash/유한수/방출수를 비교해 검증한다.


### G33 F의 남은10 mesh /8 native material 연결 확정

남은8 MIC의 실제 selected shader map/flocal VS·PS를 기존 추출 도구로 읽었다.10행 모두 선택이
유일하고 PS는8개(31~88 RT0명령), 공통 VS는 D324와 같은 `8562847977cf324b900feff85799f43a`다.
SceneColor/Depth sample은0이며 named texture21개는 모두 현재 Resources에 존재한다.

기존 `Effect_DimensionMasterSDMaterial.h`와 SDNative HLSLI에 SD325~332를 추가하고 기존32float4
packet, shader mesh dispatch, renderer material classification만 범위를 확장한다. 원본의 named
texture/CB0식·sampler·tangentView·particle color를 사용한다. MIC의 two-sided override를 parent보다
우선해8개 모두 양면 상태를 사용한다. 별도 runtime, shader파일, 프로젝트 등록, Resources payload는
추가하지 않는다. 각 F행에는 exact source MIC에 대응하는 descriptor/material을 연결하고 남아 있는
module/lifetime/geometry gate를 실제 codec/Playback으로 확인한다. 실패한 행을 generic fallback으로
활성화하지 않는다. 원본 RT0 수식과 현재 PS 최소컴파일, 유한 출력/기존D320~324 보존을 검사한다.


#### G33 F disabled CircleSurface와 effective slot 복사 경계

F의 두 CircleSurface module은 원본에서 `benabled=false`이고 Playback도 해당 모듈을 건너뛴다. portable module validator가 유효한 false까지 거부하던 조건만 제거하고 boolean type, axis, split, geometry 검사는 유지한다. SD325–332는 공통 mesh VS의 UV1 evidence를 전달하되 실제 PS에 scene depth sample이 없으므로 scene-depth 필수 조건은 확장하지 않는다.

Data Files의 Add Element는 실제 mesh material slot들이 실행 가능한 경우에도 사용하지 않는 primary material의 hold를 보고 거부하던 단일 gate를 `Is_EffectElementAuthoringExecutionTarget`으로 통일한다. 복사된 Detail과 SourceRecipe 보존, FOLLOW/Trail/history 차단, 최종 canonical parse/validation은 유지한다. F 두 slot crack의 복사 성공 및 어느 slot 하나라도 hold이면 거부되는지 기존 focused probe에서 확인한다.

S full의 mesh 01/02도 사용자 최종 요청에 맞춰 Q tuning에 복사한 동일 crack의 금색 `refle_color/in_color/out_color`로 맞춘다. S의 원래 양방향 source velocity, 개수, 시점은 보존하며, R에 복사된 여섯 crack은 별도의 보라색 tuning을 유지한다.


#### G33 저장 잔여 병목: JSON DOM 배열 성장

Debug MSVC에서 DATA_JSON_VALUE의 map 멤버 때문에 move가 noexcept가 아니므로 vector의 자동 재할당은 자식 DOM을 깊게 복사한다. A104 문서에서 generic parse 2.18초/1,074만 allocation을 재현했다. Client/Private/DataJson.cpp ReadArray의 배열 성장만 별도 vector에 reserve 후 기존 node를 명시 move하고 swap하도록 바꾸어 공개 타입/ABI와 파서 문법, 오류, limit, 외부 out commit 정책을 보존한다. out-only 비교에서 0.84초/370만 allocation으로 감소했다. 전체 JSON 차등 및 기존 Save 성공·외부변경 거절·원자 rollback 검사를 거쳐 실제 값으로 RESULT를 갱신한다.


#### G33 사용자 확인: Q에 A 검격을 가져와 4회 재생

사용자가 A 자체 수정이 아니라 Q에 검격을 가져오는 쪽으로 확인했다. Q tuning 첫 타격 0.27초를 기준으로 A WR279/280 두 검격의 네 타격 간격을 보존하여 0.27/0.62/0.92/1.32초에 각각 복사한다. Q 금색 crack 2개도 각 타격마다 독립 occurrence로 복사해 같은 group ID로 묶는다. 기존 Q 사용자 cube와 slice 4행은 보존하고 A 원본은 바꾸지 않는다. 새 stable ID와 authored-copy sourceNode로 출처를 남기며 기존 CModel, source material, SourceRecipe 운동을 재사용한다.

R 양옆 crack은 S 원본 02의 반전 yaw와 음의 source velocity가 서로 상쇄하므로 그대로 복사하면 같은 방향이 된다. R에서는 두 donor 모두 R 검격과 같은 snapshot yaw0/rotation0을 사용하고 02의 sphere X 반구도 음의 방향으로 바꾼다. +X/-X source velocity와 반구 시작 분산이 검격 중심 양쪽으로 일치한다. S는 기존 forward 운동을 보존하므로 대칭이라고 보고하지 않는다.


#### G33 F LocationEmitter의 독립 저작 복원

F SD329 sphere는 실제로 `ppp`라는 다른 emitter의 위치를 읽는 enabled LocationEmitter이며 원본 owner는 회전 Orbit 입자다. 이 의존성을 무시하거나 원본 운동 복원 완료라고 표시하지 않는다. 현재 요청의 Solo 저작 복원을 위해 해당 한 행을 명시적인 `authored-copy` + `F orbit-path tuning`으로 바꾸고, LocationEmitter만 기존 `particlemodulelocation/startlocation` 곡선으로 치환한다. owner의 실제 lookup 해석은 offset(-240,0,0)cm, yaw rate -4.5turn/s이다. target source delay0.05초를 더한 owner age에서 원형 경로를 600Hz/121개 점으로 저작하며 선형 보간 최대 편차는0.067cm다. 원본 cross-emitter 위치/RNG/rotation 상속과 동등하다고 주장하지 않는다. 재질·시점·생성 rate50·크기·수명 입력은 보존하고 원본은 기존 Git/초기 snapshot에 남는다. 새 runtime 경로나 source-particle 참조 프로토콜을 만들지 않고 기존 curve evaluator를 사용한다.


#### G33 Play All 렌더 제출의 반복 계산

실제 S ALT129의500 sprite는 기존 instancing으로 하나의 draw를 쓴다. source SubUV의 immutable module/literal 설정을500회 재검색하던 CPU 비용은 Debug 약3.0ms, Release0.068ms다. Render_Particles batch에서 설정을 한 번 resolve하고 입자별 frame/flip만 계산하도록 hoist한다. 입자 수·순서·atlas 보간·flip·운동은 유지한다. FX11 변수 lookup의24draw 비용은0.5ms 미만이므로 광역 Engine shader 캐시 수정으로 범위를 넓히지 않는다. GPU 비용과 실제 Client FPS를 이 부분 측정만으로 확정하지 않는다.

Add Element의 portable carrier 복사 helper도 이미 문서 validator가 허용하는 mesh material slot 모듈을 같은 조건(PARTICLE/mesh/nonempty sourceMaterialSlots/exact module class)에서 보존한다. 현재 validator와 copy helper 사이의 whitelist 불일치만 고치며 최종 전체 material slot 계약 검사는 유지한다.


#### G33 최종 사용자 정정 — Q는 한 번, A는 네 번

앞의 Q4회 복사안은 사용자 최신 정정으로 폐기한다. Q tuning에 잘못 추가했던 A 검격8행과 반복 crack6행 및 두 번째 crack1행을 제거해, 기존 Q cube/slice4행과 금색 crack1행만 남긴다. crack은0.27초 burst1회이며 하나의 element가 원형 전체를 덮도록 cylinder의 positive_x/negative_x를 모두 true로 한다. A full의 기존 WR279/280 검격은0.25/0.60/0.90/1.30초 네 발생을 그대로 보존하고, 각 타격에 이미 있는 q-local-crack와WR238 유리 두 재질 층의 색3벡터만 보라색으로 맞춘다. A의 검격 shape/운동/시간/개수는 바꾸지 않는다. 사용자 빌드 중이므로 C++/shader 소스는 그대로 두며 데이터만 수정한다.

#### G33 A 검은 dustparticle 보강 누락 수정

A full의 Q51 glass-hole sprite 네 행에는 R에서 추가한 검은 중심 면 보강이 빠져 있었다.
동일한 MIC를 쓰는 6449356759fd8f684f81, f23886dd0742331c9546,
99d30eff4c2c88e26aaf, a91256889932fbe55fef 행에 기존 solid-core runtime alias와
blackCoreWidth=0.38, blackCoreSoftness=0.12만 연결한다. 기존 native 입력과 source module,
초승달 mesh, 네 발생 시점은 보존한다. alias 이름의 R은 최초 적용 대상을 뜻하며 실제 계약과
shader dispatch에 스킬 제한은 없다. 현재 빌드에 이미 포함된 재질을 재사용하므로 추가 C++ 또는
shader 컴파일은 하지 않는다. JSON 변경 범위와 기존 Codec/Playback 실행 파일의 네 occurrence
생성·유한수를 검사하며, 원본 PS의 정확 복원이나 사용자 화면 확인 완료로 기록하지 않는다.


#### G33 Saved Skill Effects 목록 정리

Saved Skill Effects 목록의 고정190px 높이를 창의 남은 공간에 맞춘 가용 높이로 바꾸고,
작업 버튼·Reference Files 접기 영역·상태 한 줄의 공간을 남긴다. Data Files의 상시 장문
안내와 반복되는 Subtypes/Cascade 진단 요약을 제거하고 현재 Effect/개수 표시는 간결하게
유지한다. 저장/복사 실패 메시지는 제거하지 않고 첫 줄을 표시하며 전체 메시지는 툴팁으로
확인하게 한다. 검색, Add Element, Load, Unload, Refresh와 기존 typed 소유권·저장/실패
처리는 바꾸지 않는다. 해당 렌더 함수만 수정하고 기존 Data Files/clone 검사를 실행한다.

#### G33 S 원본 Shine 입력과 출력 추적 재개

손튜닝 cone/helix 연결을 원본 복원 결과로 취급하지 않는다. 현재 S2050220의 enabled notify와
첫 LOD에 연결된 Shine4행 및 주변 선의 원본 VS/PS와 현재 sourceRecipe→Playback frame→
renderer packet→shader 출력 경로를 비교한다. 기존 원본 조사와 수치 검사를 재사용하고,
불일치를 재현한 입력/수식만 기존 소비자에서 수정한다. 원본 선택 프로그램이 이미 있는
상태에서 근거 없는 새 shader나 형상 대체를 추가하지 않는다. 단독/전체 재생의 실제 입자
입력·유한수·shader 출력을 확인하고 제품 빌드는 다른 compile/link와 겹치지 않게 순차 실행한다.


#### G33 R 검은 중심 무늬 분리와 full restore 미지원 요소 정리

사용자가 제공한 현재 R 이미지에는 검은 중심 면 위에 밝은 점과 띠가 남는다. 기존 native Q51과
A/기존 solid-core alias는 보존하고 R의 마지막 dust 저작 조정을 위해 동일34params/profile51의
clean-core alias를 추가한다. 새 alias만 core packet row11.z=1을 설정하며 기존xy bounds를
공유한다. Fill helper의 cleanSurface 기본값0은 기존 계산을 유지한다. 새 모드에서는 coverage
내부의 RGB 기여만 (1-saturate(coverage*cleanSurface))로 줄여 검은 면을 만들고 외곽 빛과
깊이·입자 fade를 유지한다. 이 조정은 원본 PS 복원으로 기록하지 않는다. 사용자가 직접 복원한
voronoi 검격은 변경하지 않는다.

사용자는 Solo 미재생 제거 범위를 차원술사 full restore 전체로 확정했다. 현재 Tool의 유효
material slot·visible·presentation gate로 명확한 미지원 항목을 추리고 기존 저장 내용을 백업한
뒤 해당 element만 제거한다. 정상 요소가 참조하는 owner와 model cue는 보존한다. 각 문서의
최신 hash를 적용 직전 확인하고 다른 편집이 있으면 최신 내용을 다시 읽어 제거 목록을 검토한다.
생성 직후 크기/alpha0이나 본 준비가 필요한 정상 요소를 삭제 근거로 삼지 않는다.


#### G33 S splitline의 Null Dynamic 입력 누락 수정

S e49(V69)는 실제39개 재생 입력의 Dynamic이 전부0이며, 원본 PS의 floor(textureR+Dynamic.z)
때문에972조건 중 해당117조건 모두alpha0이었다. 원본의 선택 dynamic VF/VS는 Q45/49와 같고,
이미 확인한 Null Dynamic stream 계약은 모듈 부재 시(1,1,1,1)이다. Apply_SourceSpawnModules의
기존 no-enabled-Dynamic 기본값 분기에 V69만 추가한다. 원본 PS와 JSON 모듈을 바꾸지 않고
실제 Dynamic 모듈이 있는 항목의 값은 보존한다. 같은 전체 재생에서 변경 전후 입자 입력을
비교하고 실제 원본 PS가 양의 alpha를 만드는지 WARP 수치로 재검사한다.


### G34 R 원본 검격과 Q에서 가져온 dust의 입력·출력 대조

사용자는 추가적인 clean-core 보강 대신 원본 경로 복원을 요구했다. R 튜닝의 세 dust 행은
Q nailstrike emitter13의 Q51 glasshole을 복사한 저작 입력이다. 원본 Q의 PSA_Velocity,
offsetCenterY=.9, StartSize20×130cm가 R에서는 PSA_Rectangle, offset 해제,260×35cm로
바뀌었다. 따라서 이 행의 모양을 R 원본 셰이더 복원 결과로 취급하지 않는다. 새로 시도한
superellipse/.46/.08 강화는 적용 전 값과 byte 단위로 동일하게 되돌렸다. 기존 사용자
Voronoi와 다른 occurrence는 보존한다.

원본 R full의 foldcut WR259 sprite3행·WR260 mesh3행은 별도로 추적한다. 현재 저장된
원본 selected 재질과 named parameter25/40개의 일치, 실제 Dynamic module 및 sampler
mapping을 확인했다. 임의 검은 면을 넣기 전에 현재 실제 frame 입력과 원본 PS 출력을 대조한다.
Q51은 보존된 원본 DXBC와 변환 QNative51에 동일 재질·UV·color·dynamic·scene 입력을
공급해 수치 차이를 찾는다. R은 원본 six occurrence의 시간별 실제 playback 평가와 원본
WR259/260 PS 출력의 alpha·finite 상태를 확인한다. synthetic depth/fog fixture는 원작 장면
확보와 구분하며 이 검사를 visual PASS로 기록하지 않는다.

확정된 입력/계산 결함만 기존 native owner에 수정한다. 차이가 재현되지 않은 PS를 새로운
모양으로 바꾸거나 불가능한 element로 분류하지 않는다. source·native RT0·engine adapter·
실제 장면 소비 범위를 RESULT에 나누어 기록한다. out-only 비교 코드는 Resources나 별도
runtime에 추가하지 않으며 compile/link는 Product 작업과 순차 실행한다.

### G35 A 네 타격에 보라색 cube와 기존 crack 동반 배치

사용자의 원본 관찰에서 가로로 깨지는 경계는 정상 표현이다. 검은 중심 면과 보라색 외곽의
시간별 형성·소멸을 복원 기준으로 두며, 정지 이미지로 실제 element 수나 발생 순서를 확정하지
않는다. 이번 마무리 변경은 A의 cube 동반 배치에 한정한다. 검은 면/외곽 셰이더의 추가 보정은 없다.

A full의 crack emitter4/24는 이미 각각 네 타격에 보라색으로 연결돼 있다. 이8행과 기존62행을
보존하고 R tuning의 purple-glass-sequence에서 cube mesh/재질/크기/수명을 복사한4행만 추가한다.
각각 A의0.25/0.60/0.90/1.30초 occurrence와 crack의 emitter delay0.1초를 사용한다.
R의 세 burst를 각 행의 단발6개로 바꿔12회 중복 발생을 피하며 maxParticles도6으로 맞춘다.
각 A crack emitter24의 root attachment·transform·localSpace와 sphere/cylinder-spin 위치 모듈을
사용해 초승달 주변에 놓는다. A의 생성 위치·초기 속도를 차용하며 이후 감속·가속 전체를
동일하게 복제하는 변경은 아니다. R donor 자체와 Q, 기존 검격·crack의 다른 값은 수정하지 않는다.

기존 문서 bytes를 보존하여4행을 append하고 쓰기 직전 내용 일치를 확인한다. JSON parse,
기존62행의 구조 동일성, stable ID와 네 timing, 기존 실제 Codec/Playback의 전체 A Load/Stage 및
새 cube/기존 crack의 네 발생 구간·유한 운동을 확인한다. 이번 조합은 사용자 요청의 저작 배치이며
원본 A cube emitter를 특정해 완전히 복원했다는 판정과 구분한다. 최종 화면은 사용자가 확인한다.

### G36 R/A dust 경계와 S 메인 검격의 원본 입력 재검증

사용자는 R·S 원본 이미지와 A의 시간별 깨짐 관찰을 제공했다. 정상 유지 구간의 검은 중심과
보라 경계까지 잘게 갈라지는 결함을 특정 시점의 의도된 분열과 구분한다. 기존 clean-core/solid-core
면 보강을 원본 계산으로 간주하지 않는다. S는 현재 설치 LOA의 활성 발생과 V63의 실제 입력을
다시 대조하며, 기존 cone/helix 손작업을 원본 스크류로 간주하지 않는다.

먼저 확정한 변경은 `Shader_EffectDimensionMasterQNative.hlsli`의 `QNative51` 내부10개
instruction에 한정한다. 원본 DXBC immediate32와 다른11개 상수 lane을 원본 bit 값으로 복구한다.
극좌표 상수, 반경의0에 가까운 값 분기 및 두 depth fade 상수가 대상이며 기존 native 함수,
입력 packet,7개 texture sampler,geometry,draw pass와 R/A 저작 보강은 유지한다. 새 파일·프로젝트
등록·두 번째 runtime은 필요 없다. 현재 dirty 파일 전체를 후보로 덮지 않고 함수 내부 차이만
대조한 뒤 반영한다.

원본 PS와 제품 함수에 같은 material/UV/color/Dynamic/depth를 공급하는 기존 headless 검사를
재사용한다. 원본 literal 후보는120조건/491520픽셀에서 기존 오차 기준 초과2460픽셀을0으로
줄였다. 제품 반영 후 같은 검사를 다시 실행해 후보와 제품의 일치를 확인한다. 이 증거는
통제된 RT0 수치 비교이며 정상 구간의 전체 모양이나 원작 화면 일치 판정이 아니다. S 입력과
R/A texture·시간·occurrence 차이는 실제 근거를 확보한 항목만 후속 수정한다.

Q51의 `cracknormal_tex=fx_j_normal_bc5_09`는 현재 설치 UPK에10개 mip이 있지만 Resources의
BC5 DDS에는 mip0 하나만 있다. 두 `SampleBias(0)`가 축소 시에도 고해상도 noise를 소비하는
입력 차이다. 기존 파일을 보존한 뒤 원본의 mip1~9를 직접 회수해 같은 Resources 상대 asset ID에
연결한다. mip0은 byte 단위로 유지하고 새 축소 이미지를 임의 생성하지 않는다. 기존
`Load_SourceTexture`의 DDS loader는 파일에 든 mip을 모두 소비하므로 별도 runtime이나
source-profile 변경은 하지 않는다. 원본 payload·DDS layout·실제 GPU mip 수와 축소 sampling을
검사하고 UI의 최종 자글거림 감소 판정은 사용자에게 남긴다.

같은 조사에서 S V63의 `fx_d_noise_009/014/021`도 원본은128→1의8개 mip, 현재 DDS는 mip0
하나임을 확인했다. 원본 PS 세 texture instruction 모두 `SampleBias(0)`이므로 이3개도 원본
BC1 mip chain을 회수한다. Q51 normal과 마찬가지로 mip0을 보존하고 current Resources 상대
경로에서 교체한다. S V63의 native 수식·색·정렬·발생 수를 임의 수정하지 않고 원본과 같은
입력의120조건 RT0 비교 및 실제 DDS mip 수로 검증한다.

Q51 sprite의 원본 VS는 U·V·normal에 대한 시선 내적을 넘긴다. 현재 rect는 localY=.5-v이므로
원본 V와 normal은 각각 current world1/world2의 반대다. `Shader_VtxEffectParticle.hlsl`의
기존 tangentView 계산 직후 profile51에만 yz 부호를 교정한다. source camera-facing normal
기본 분기를 대상으로 하며 이 값은 Q51의 환경반사 UV에 소비된다. geometry/UV/alpha/dynamic을
바꾸지 않는다. 실제 source VS 연산과 camera·회전·크기가 다른 수치 입력으로 비교하고 제품
VS_MAIN을 최소 컴파일한다. 이 변경을 마스크 자글거림 해결의 단독 근거로 사용하지 않는다.

사용자의 전체 반영 승인에 따라 R/S의 실제 animevents asset 참조를 각각 현재 full.restore로
연결하고 EffectCatalog에 그 두 문서를 등록한다. 기존 unified/tuning 문서는 보존한다. 원본 R
full의 WR260 세 행에서만 SourceScale.lifeTime16→1, 마지막 행의size2→1로 원본 진행을 회복한다.
다른 저작 위치·색·배치와 R tuning의 Q donor는 이번 연결 변경으로 덮지 않는다. 문서 identity,
stable element ID·source module·Resources 상대 참조 및 실제 Codec/Playback 준비를 검증한다.

원본 R의 WR208/259/260 핵심9행에서도 SampleBias용8개 texture에 하위66mip이 빠져 있음을
확인했다. caustic05/electric008/noise03/environment02/cloud021/noise030/auratile02/atypical03_1의
원본 mip을 같은 방식으로 회수하고 mip0을 보존한다. WR260 main trail007은 명시적 LOD -1이므로
이번 축소 입력 결함과 구분한다. 새로 설치한 mip에 대해 실제 DDS/SRV의 개수, sRGB 형식과
공유 asset 소비 목록을 확인한다. 원본 R 핵심 PS의 immediate32도 실제 DXBC와 대조하되,
현재 수식에서 차이가 없는 부분은 다시 쓰지 않는다.

WR259 Required의 `psortmode_viewprojdepth`는 원본 sprite가 translucent일 때 clip-Z 큰 값부터
그리는 계약이다. 현 renderer는 이를 읽지 않고 생성 순서로 업로드한다. 기존 Render_Particles의
한 element span 안에서 원본 Particle.Location에 해당하는 Particle.World translation을 현재
View×Projection으로 변환해 정렬하고 기존 sprite instance 업로드에 연결한다. billboard pivot,
camera offset과 quad 크기를 sort key에 섞지 않는다. 같은 깊이는 입력 순서를 유지하는 엄격한
비교 규칙을 사용하고 비유한 값은 draw 실패 이유로 보존한다. mesh·additive·sortmode 미지정
행과 문서의 element 순서는 바꾸지 않는다. 새 공개 인터페이스/파일/project 등록은 필요 없다.
원본과 현재 actual frame의 순서 차이, helper 수치 검사, 현재 renderer 최소 컴파일을 확인하고
진행 중 Product가 이 C++ 변경을 놓친 경우 shader 빌드 종료 후 Client compile/link를 이어간다.


WR260 actual61 frame의 원본 CPU uniform을 양쪽 PS에 동일 공급하면 RGBA가 일치하지만,
현재 GPU uniform prefix는 dissolvetex_rotator의 sin/cos6lane에서 차이가 났다. 편집 가능한
rotator를 원본과 같은 float 연산 순서로 CPU에서 평가해 기존 native 파라미터 전송에 연결한다.
고정된 원본 기본값을 HLSL에 넣는 방식은 쓰지 않는다. 기존 파라미터 미사용 lane과 소비 경계를
확인한 뒤 해당 prefix만 연결하며 원본 raw PS 수식은 보존한다. actual61 frame과 rotator 변경
입력으로 비교하고 최종 Product는 이 변경과 source depth 정렬을 포함해 새로 컴파일한다.

### G37. R 원본 보라 경계, D 세로 잘림, S 검격 가독성 후속 복구

2026-09-10 사용자는 R full의 원본 보라 경계가 아직 없다고 관찰했고, F1의 D full은 검격이
세로로 잘리며 S 검격도 다시 확인하도록 요청했다. 실제 R/S binding과 F1 D full을 각각 소비
정본으로 삼는다. R tuning의 Q51 보라 경계 보강을 R full의 원본 복원으로 대체하지 않는다.

R의 WR208/259/260 핵심9행과 빠진 body109/110·RGBNoise·ZoomBlur·Light의 source occurrence를
대조한다. 같은 MIC·PS·VS·VF가 이미 지원된 native 재질이면 기존 descriptor와 renderer를 재사용해
재연결하고, 각 요소의 실제 역할·경계 기여·제외 이유·남은 입력을 RESULT에 표로 기록한다.
부재 원본 요소를 임의 이름이나 generic 후처리로 대신하지 않는다.

D는 현재 full25행의 실제 Q51/WR259 occurrence를 추적한다. 원본 UV panning과 generic UVOffset의
중복, clamp alpha mask, billboard velocity basis와 source texture 입력을 분리해서 대조한다.
원인이 재현된 full 문서의 행만 교정하고 사용자가 별도로 편집한 unified와 실제 D binding은 보존한다.

S는 중심 V63과 시작부 SD320~323, 원본 size 누적·cm→m·pivot·velocity alignment 및 shader alpha
coverage를 실제 Playback의 World와 함께 측정한다. 복원 오류를 먼저 교정하고, 크기 확대가 필요한
경우에는 중심 검격의 SourceScale.size만 명시적인 사용자 가독성 보정으로 조정한다. 원본
SourceRecipe와 PS·발생 시간은 보존한다. 큰 원판·주변 요소를 함께 확대해 성능 문제를 키우지 않는다.

기존 out 수치 probe를 재사용해 변경 전후 실제 Codec/Playback, 원본 shader 입력과 UV·alpha
coverage, 필요한 최소 C++/FX를 확인한다. 새 C++ 파일은 제안하지 않는다. Client/UI 실행·캡처는
수행하지 않으며 현재 정식 EXE 잠금과 별도 링크 산출물을 구분한다. Alt V318요소의 재생 성능은
09-09 Effect Shader Build and Play All Performance PLAN/RESULT의 G06이 소유한다.

D 원본 startsize의 비베이크 DistributionVectorUniformRange는 두 개의 분리된 범위 중 하나를
먼저 고르고 XYZ 난수 세 개를 소비한다. 기존2개 min/max로 축약하면 빈 구간까지 생성하므로
CEffectDistribution operation4에 원본4벡터를 담아 같은 Playback 경로에서 평가한다. Codec의
기존 Validate 위임으로 형태·component·시간·유한값을 거부하고, alpha의 비베이크 cubic keys도
실제 원본에서 회수한다. RNG 순서·signed flip·birth geometry와 기존 operation0~3를 확인한다.

### G38. BA 전체 목록과 제품 연결, A 발생 높이 — 2026-09-10

기존 BA1~4 full 문서(내부 ID ba0~ba3)를 그대로 사용한다. 현재 Codec Load와 실제 GPU Stage를
먼저 실행해 문서 오류와 목록 누락을 구분한다. EffectCatalog의 제품 등록 및 EffectResourceTree의
명시적 목록 참조에서 빠진 요청 문서만 추가한다. 다른 평타나 A를 복제해 BA 모양을 추정하지 않는다.

A full의 0.6초 두 번째 타격은 Y=0.8, 나머지는 Y=-0.9/-0.8로 저장돼 있다. 같은 검격·cube
발생에 속한 원래 요소의 Y만 두 번째 값으로 맞추고 X/Z·회전·크기·원본 module을 유지한다.
sprite 원본 carrier의 detail timing start delay에는 각각 0.2초를 더한다. mesh cube의 timing은 유지한다.

제품 animevents는 BA 네 단계 full, Q tuning, W/E full, R tuning, A/S/F/D/V/AltV full로 연결한다.
full 문서 안에 들어 있는 발생을 animevent에서 중복 생성하지 않도록 실제 원래 clip/time 연결을
확인한다. 기존 skillbindings와 Server skill/stage 이름 및 T는 유지한다. R tuning의 사용자 저장은
편집하지 않는다. header row count는 변경 후 실제 행 수로 계산한다.

변경 전 bytes를 out에 보관하며 사용자 Save가 겹치면 원본을 덮어쓰지 않는다. JSON parse,
기존 BA 단계 회귀 검사, 실제 Codec/Playback/GPU 준비와 offscreen draw 수치만 확인한다.
새 제품 C++ 파일·프로젝트 등록·전체 제품 빌드·Client/UI 실행·화면 캡처는 수행하지 않는다.

G38 조사에서 legacy BA audition의 저장된 source hash는 LF 원문과 일치하나 현재 checkout은
CRLF인 문제가 확인됐다. 내용 수정이나 hash 검증 완화 없이 해당 donor 3개의 LF bytes를
복구하고 `.gitattributes`의 BA restore 문서 한 패턴에 `text eol=lf`를 고정한다. 실제 source index
freshness와 Open에서 사용하는 catalog provenance 검사를 그대로 재실행한다.
