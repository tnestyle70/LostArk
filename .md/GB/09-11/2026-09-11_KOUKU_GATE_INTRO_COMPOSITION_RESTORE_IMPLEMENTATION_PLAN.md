# 쿠크 진입 컷신 Composition 복구 구현 계획

## G00. 현재 정본과 작업 범위

현재 브랜치 `codex/kouku-gate1-sequence-playback`의 미커밋 Sequence·Effect 작업을 보존한다.
`KAKULSAYDON_G1_PATTERN_3`은 사용자가 만든 `2관문_진입컷씬`이며 타임라인이 비어 있다.
기존 `연출_팝업북`과 같은 Composition Resources/WorldSequence/CModel 경로로 원본
SCENE04A의 카메라·맵·보스·카드 분출을 연결한다. 별도 모델 런타임을 만들지 않는다.
카메라 저작과 카드미로는 각 기능 담당 PLAN에서 실제 파일·저장·Server 경계를 기록한다.

## G01. 펼쳐지는 맵의 원본 광원 수광

실측한 쿠크 mapmaterials는 1,732행이고 maplights는 119개다. 그중 원본 구운 광원 84개는
`SOURCE_CHARACTER`만 비춘다. 팝업북의 움직이는 맵은 기존 BG8 재질을 사용하지만 구운 RNM이
없으므로 이 84개 광원을 받지 못한다. 정적 맵의 RNM을 움직이는 물체에 복사하지 않고,
동일 광원이 구운 조명 없는 표면을 비추도록 `LIGHT_RECEIVER::UNBAKED`를 추가한다.

`Engine_Struct.h`의 값 2는 구운 조명이 없는 표면 수광을 뜻한다. 기존 ALL=0,
SOURCE_CHARACTER=1 의미는 유지한다. `Light`, `Light_Manager`, `Presentation_Manager`는
새 값을 검증한다. 일반 Deferred pass는 기존 GBuffer의 baked bit로 구운 map pixel을 제외하고,
source character pass는 native map monster의 baked 표식을 별도로 검사한다. Forward map도
같은 receiver를 적용한다. 새 상태는 기존 LIGHT_DESC가 소유하며 별도 session state는 없다.

`MapLightDocument`의 parse/serialize, `light_resources_pipeline.py`의 publisher 검증과 쿠크
maplights의 해당 84행을 함께 바꾼다. 광원 위치·색·강도·감쇠와 기존 승인 재질은 유지한다.
잘못된 receiver는 기존 문서를 보존하며 실패한다. 기존 파일 수정이므로 project/filter 등록은 없다.

## G02. 원본 컷신과 저작 도구 통합

2관문 리소스 담당은 원본 전체 track/clip을 조사한 뒤 필요한 누락 모델을 기존 cooker로 설치하고,
WORLD/CAMERA/Effect resource 및 사용자의 빈 Sequence만 갱신한다. reverse/start offset 등
실제 컷신이 요구하는 animation 입력은 원본 A/B track을 오프라인 표본화한 단일 WANM clip으로
보존하고 기존 WorldSequence parser/publisher/sample이 소비한다.
Composition Camera는 이름 있는 camera action에 시간·position·rotation을 저장하고 같은
카메라 controller로 재생한다. 카드미로의 네 숨김과 마지막 진입은 Server timeline이 처리한다.

### G02-1. 생성형 컷신 모델의 재질 소유권

World Object의 optional `materialSourceModelAssetId`는 새로 구운 actor 모델이 기존
ActorCatalog의 승인 재질을 쓰게 한다. `mapMaterialBindings`는 실제 대상 materialName을
같은 Area의 sourceAssetId/sourceMaterialName에 연결한다. 선택한 원본 diffuse만 다른
경우 optional diffuseTextureAssetId를 쓸 수 있다. 저장·동등성 비교·publisher와 실제
CModel 로더가 모두 이 필드를 소비한다. 누락 모델·slot·BG8 정본은 실패하며 기존 객체를 보존한다.

움직이는 모델에는 정적 배치의 RNM/static shadow를 복사하지 않는다. 기존 skinned shader에
공유 `Shader_MapMaterialSurface.hlsli`의 BG8 평가를 연결하고 동일 Deferred GBuffer와
UNBAKED 광원 경로를 사용한다. WorldSequence의 localMs를 표면 발광 시간으로 전달한다.
기존 파일 수정이며 project/filter 추가는 없다.

### G02-2. 원본 이동 광원

SCENE04A의 l1/l2/l3 위치·밝기·색·반경 track을 기존 V1 typed point-light에 투영한다.
위치 곡선은 오차 0.0005m 이하의 서로 겹치지 않는 선형 구간으로 나누며 RGB는 원본 Hermite를 cubic distribution에 보존한다.
기존 transform lerp와 color-over-life distribution을 그대로 사용한다. 동시에 켜지는 광원은
최대 세 개다. source channel mask와 dynamic shadow map은 이 기존 carrier의 지원 범위와
구분해 결과에 기록한다. 암전은 기존 V2 ScreenPost 소비자가 시간별 intensity key를 사용한다.

## G03. 검증과 사용자 실행

변경 JSON/XML parse, 해당 Area Publish/Check와 Composition publisher를 실행한다.
receiver의 baked/비baked 선택과 잘못된 입력 보존, 카메라 저장·재로드 및 보간,
카드미로 hide/entry/실패 복구를 기존 기능 검사에서 검증한다. Engine public 변경이 있으므로
정본 Debug Product 빌드로 Engine SDK·Shared·Server·Client까지 확인한다.
Client/UI 자율 실행·캡처는 하지 않는다. 새 바이너리와 리소스 준비 후 사용자가
Lobby → KoukuSaydon → F1 → Open Sequencer Benchmark에서 해당 Sequence를 Play한다.
실행한 자동 검사와 사용자 화면 미검증은 RESULT에서 분리한다.
