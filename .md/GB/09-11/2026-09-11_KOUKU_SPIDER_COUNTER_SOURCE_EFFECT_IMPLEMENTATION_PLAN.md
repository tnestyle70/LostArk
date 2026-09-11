# 쿠크 거미카운터 원본 바닥·돌진 이펙트 연결 구현 계획

## G00. 실제 source clip과 바닥 색

대상은 GATE2 `KAKULSAYDON_G1_PATTERN_15`, actor `MN_RPCZ_00`, 원본 action4219776이다.
현재 Animation은 Att_Battle_6_01 → 02 → 02 → 03 → 04를3회 반복한다.
원본 stage1의Dust·EyeLight와 stage2의Dash_Trail·Dash_Ground만 Particle notify를 가진다.
01/04에는 같은 Particle notify가 없다. 활성 firstLOD는12개이며 두 eye socket 복제로14개다.

검정+빨강 바닥은 stage2 0.19218초의 `Par_U_RPCZ_Dash_Ground_01_LOC_INT`다.
worldoffset_02_2_tr의 검정[0,0,0]과 적색[0.2,0.005,0.005] 두 줄 및 붉은CD02 세 줄이다.
EPAL_Z 바닥고정, 길이600cm의 source size,0.95/1초 source lifetime을 보존한다.
현재 패턴의 실제 돌진 시작4500/11667/18834ms에 해당 local time을 더해4692/11859/19026ms에 발생한다.
원본 action 전체 합산시각을 그대로 넣어 사용자의추가02 loop1초를 빼먹지 않는다.

## G01. 원본 재질과 기존 carrier

`build_kouku_spider_counter_restore.py`는 기존 Gate1 source graph/CDO/분포/attachment projector를
재사용한다. Shared builder의 Slam 전용 baked trail history만 해당action존재조건으로 제한한다.
source stage1/stage2를 별도V1문서로 저장하고 source clip별기존 pattern presentation에 연결한다.
Eye 두 socket은 원본props의bip001-head와±0.0641881m offset, FX_State_01은bip001-spine1의
원본rotation을 사용한다. root snapshot과FOLLOW출생위치의 차이는 그대로 유지한다.

MIC9개의 원본static set/VF/PS/VS와uniform/texture를 비교한다. 같은 원본식은 현재native식을
재사용하고새sourceMaterial ID를 정확한기존 interpreter table에 추가한다. 새임의shader나색칠은 없다.
실제Ribbon1개는 source TypeData로 분류하고v15 cascadeRibbonV1/typeDataModuleStableId로
기존Trail carrier를 사용한다. 소스 material과carrier계약이 닫히지 않은 요소는 근사로 바꾸지 않는다.

## G02. 저장·호출·등록과 검증

새stage effect2개는 EffectCatalog와Client96.DataFiles None항목에 등록한다. 기존Effect_ShaderFamily의
Kouku2304 범위안에 필요한native program만 추가하며 기존프로그램은 보존한다. 새C++파일은 없다.
Composition은 최신bytes와revision을 읽고pattern15의presentation에만 source stage연결을 추가한다.
저장직전bytes를 재검증하고충돌시 기존자료를 덮지 않는다. 기존counter/fear/charge/다른pattern은 보존한다.
반복실행은 이미연결된source stage ID와asset를 유지한다. 사용자튜닝도 덮지 않는다.

변경JSON/XML parse,원본source occurrence/필요Resources 존재,실제codec/stage/playback,
원본shader입력/출력과필요최소컴파일,diff check를 검사한다. publisher와제품빌드는root가 통합한다.
Client/UI 실행·조작·캡처는 하지 않는다. 사용자가F1 Action Workbench →쿠크거미카운터 패턴을
직접재생해검정/빨강바닥과돌진·눈·먼지의시각결과를 판정한다.

## G03. 실제 소비자까지 닫는 범위

`Effect_ArtistMaterial.h::Has_ArtistMaterialContract`에 Kouku 원본 `ribbon`의
`TRAIL + sourceRecipe + cascadeRibbonV1 + bounded` 조합만 추가한다.
기존 animationTrail, decal, 다른 캐릭터 재질 판정은 유지한다.
Kouku native 그룹은 이미 등록된 2304~2367 범위를 사용하며 이번 추가 ID는 2342~2350이다.
Mesh/Particle/Trail/Decal shader와 Renderer의 기존 Kouku 선택 상한을 이 그룹 범위에 맞춘다.

`Effect_Playback.cpp::Sample_Trail`은 2346의 결정적 Color/ColorScale, StartSize/SizeScaleByTime,
DynamicParameter를 기존 FlowRibbon01 계산으로 평가한다. 다른 재질의 색 전달은 유지한다.
`Effect_DocumentRenderer.cpp::Render_Trails`는 이 재질의 원본 RGBA와 실제 폭을 전달하고,
색·Dynamic mask 0xF, 유한한 좌표·수명·폭과 source tiling/tessellation을 검사한다.
이 연결이 없으면 기존 generic Trail의 흰색·선형 투명도가 원본 붉은 색을 대체한다.

2349 WorldOffset02의 원본 PS는 opacity를 `CB0[0].w`, 월드 UV 방향을 `CB0[1..3]`에서 읽는다.
기존 generic generator의 opacity X만 1인 prefix는 이 두 바닥 emitter의 최종 alpha를 0으로 만든다.
`patch_kouku_spider_native_programs.py`는 정확한 PS/VS/static-map을 검증한 뒤 2349의 prefix와
world-position varying만 수정한다. `Render_Particles`는 기존 DimensionMaster 341/361의
`SourceEmitterWorld` 역행렬 계산을 재사용해 `g_ArtistSourceWorldToLocal[3]`에 바인딩한다.
소스 material vertex uniform과 texture sample이 없으므로 임의 WPO 변위를 추가하지 않는다.

추가 H/CPP 파일은 없다. 두 authored JSON만 Client 프로젝트와 filters의 96.DataFiles/None에
등록한다. native 표 설치는 전달받은 ID만 소유하며 기존 프로그램을 지우지 않는다.
shader 설치의 `--append-source-dir`는 기존 38개 함수와 새 9개를 합쳐 같은 그룹에 설치한다.
