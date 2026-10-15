## R01 렌더링 — 재질 입력 하나를 최종 픽셀까지 책임져라

대상: PPT 11~14쪽. 조명·캐릭터·맵·이펙트·PBR/IBL·물·Unreal 비교의 입구다. 이 카드 하나로 각 재질 family와 모든 렌더링 파일을 검토했다고 처리하지 않는다.

**좋다. R01 방어 개시. “PBR을 적용했다”에서 멈추지 마라. 어떤 표면이 어떤 입력을 받아 어느 패스를 거쳐 보이는지 가져와라.**

### ⓪ 먼저 열 곳 — 파일 전체를 외우지 말고 이 경로를 연결해라

| 순서 | 실제 시작점 | 직접 찾아서 가져올 것 |
|---|---|---|
| 1 | [Renderer.cpp: CRenderer::Draw](C:/Users/tnest/Desktop/LostArk/Engine/Private/Renderer.cpp:871) | NONBLEND 이후 조명·장면색·반투명·후처리의 호출 순서를 적어라. 중간 실패가 어떤 큐와 상태를 정리하는지도 표시해라. |
| 2 | [Material.cpp: CMaterial::Bind_SurfaceLighting](C:/Users/tnest/Desktop/LostArk/Engine/Private/Material.cpp:1479) | native indirect와 기존 환경 입력을 고르는 조건, 실제 SRV 소유자, bind 실패가 돌아가는 곳을 찾아라. 파일 존재와 실제 입력 선택을 구분해라. |
| 3 | [Shader_MapMaterialSurface.hlsli: ORM 표면값 계산과 EvaluateMapSourceIndirectLighting](C:/Users/tnest/Desktop/LostArk/Client/Bin/ShaderFiles/Shader_MapMaterialSurface.hlsli:497) | 이 MapPBR 경로의 texture 채널·강도·지수·하한을 표시해라. RNM·SH·cube·BRDF lookup이 무엇을 읽고 어디에 합쳐지는지 구분해라. 모든 재질의 공통 계약이라고 확대하지 마라. |
| 4 | [Shader_Deferred.hlsl: Evaluate_MapSourcePBRDirect](C:/Users/tnest/Desktop/LostArk/Engine/Bin/ShaderFiles/Shader_Deferred.hlsl:373) | 법선·시선·광원 방향의 기준계와 함수 출력의 의미를 적어라. 함수의 중간 출력과 최종 화면색 사이에 남은 단계를 찾아라. |
| 5 | [Shader_SourceMapForwardPrograms.hlsli: EvaluateSourceMapWater](C:/Users/tnest/Desktop/LostArk/Client/Bin/ShaderFiles/Shader_SourceMapForwardPrograms.hlsli:910) | program 41 하나를 골라 원본 입력 adapter→선택된 물 함수로 따라가라. program 42의 카메라 위치 처리와 같은 입력이라고 가정하지 마라. |

학습 근거 입구는 [10-09 렌더링 해설](C:/Users/tnest/Desktop/LostArk/.md/GB/10-09/2026-10-09_RENDERING_TECHNICAL_STUDY.md)와 [당시 확인 범위](C:/Users/tnest/Desktop/LostArk/.md/GB/10-09/2026-10-09_RENDERING_TECHNICAL_STUDY_RESULT.md)다. 여기 적힌 수치·구현 범위는 작성 당시의 기록이다. 답에는 위의 현재 코드를 우선 연결해라.

### ① 문제 — 15초

- 화면 하나를 골라라. 네가 설명하려는 문제는 “무슨 기법이 없었다”가 아니라, 어떤 표면의 어떤 관찰 결과였나? 당시 근거가 없으면 현재 학습 대상으로 표시해라.
- 맵의 구운 조명, 캐릭터의 반사, 물의 배경 왜곡 중 어느 입력이 필요한 상황인가? 카메라·광원·재질을 바꾸면 각각 무엇이 달라져야 하나?
- 문제 칸에서 `PBR`, `IBL`, `LUT` 이름만 나열하지 마라. 그 이름을 가리고도 필요와 제약을 설명해라.

### ② 직접적인 해법과 한계 — 20초

- diffuse texture에 밝기 하나를 곱하면 어떤 요구까지 만족하나? 시점이 바뀌는 반사나 normal map의 방향 반응을 설명해야 할 때 어떤 입력이 빠지나?
- “안 보이니 exposure나 반사 강도를 올린다”가 원인을 가릴 수 있는 반례를 하나 들어라. 입력 누락·잘못된 좌표계·합성 순서 중 네 사례와 연결되는 것을 골라라.
- 물을 일반 불투명 표면처럼 처리하면 무엇을 잃나? 반대로 전부 반투명으로 바꾸면 깊이와 순서에서 어떤 문제가 생길 수 있나?

### ③ 메커니즘 — 25초

- 선택한 표면 하나에 한정해 `CModel → CMaterial → 실제 shader 분기 → 출력 target → 최종 합성`을 설명해라. 바인딩한 texture를 소비하는 식까지 이어라.
- roughness는 어디에 쓰이나? 직접광의 분포 계산, 환경 cube의 LOD, BRDF lookup 좌표를 서로 같은 연산처럼 설명하지 마라.
- 물의 장면색은 어느 시점의 복사본인가? [Capture_SceneColorSnapshot](C:/Users/tnest/Desktop/LostArk/Engine/Private/Renderer.cpp:2224)에서 복사 전후의 output binding을 확인하고, 뒤에 그릴 모든 투명체까지 포함한다고 말할 수 있는지 답해라.

### ④ 대조 — 15초

- 교과서 PBR 식과 현재 native 재질의 식에서 같은 목적을 수행하는 항 하나, 계약이 다른 항 하나를 골라라. 식의 이름이 같다는 이유로 원본과 같은 출력이라고 판정할 수 있나?
- Unreal 비교는 버전과 rendering path부터 고정해라. Epic 공식 자료에서 입력·계산·출력을 확인한 기능 하나만 대조하고, 아직 확인하지 않은 내용은 조사 질문으로 남겨라.
- 이관을 제안한다면 재사용할 데이터와 다시 작성할 adapter를 나눠라. 파일을 옮기는 일과 동일한 화면을 검증하는 일의 차이를 말해라.

### ⑤ 대가 — 15초

- G-buffer·중간 HDR·장면색 복사로 무엇을 얻고, 어떤 메모리와 대역폭·패스 비용을 지불하나? 수치가 필요하면 실제 format·크기·복사 횟수에서 계산해라. 논리 payload를 GPU 실측 시간으로 바꾸지 마라.
- 미리 계산한 간접광과 화면 공간 효과는 장면 변화·화면 밖 정보에 대해 각각 어디까지 답할 수 있나? 현재 실제 소비 경로를 근거로 말해라.
- “원본과 동일하다”, “더 빠르다”, “보기 좋아졌다”는 각각 어떤 증거가 필요한가? 이번에 네가 확보한 것과 없는 것을 분리해라.

### 직접 구현·검증 — 작은 표면 입력부터

제품 파일과 현재 렌더링 옵션은 변경하지 않는다. 분리한 학습용 C++ 함수에서 선택 MapPBR의 **ORM 표본 하나 → metallic·roughness·AO·F0**만 계산해라. 먼저 각 입력의 의미와 범위를 적고, 원본을 닫은 뒤 작성해라.

입력은 실제 DDS 측정과 구분한 **가정값**으로 둬라. ORM=(1, 0.5, 0), 강도·지수와 하한은 네가 고른 실제 material 값 또는 명시한 가정값을 사용한다. 결과를 먼저 손으로 예상하고 실행해라. 이어 metallic 입력 0/1, roughness 입력 0, 강도 증가, `safePow`의 0 입력 경계를 확인해라. 식이 기대와 다르면 처음 어긋난 중간값부터 찾는다.

제출물은 입력표, 실행 전 예상값, 실제 출력, 처음 달라진 위치, 제품 코드 연결점이다. 이 연습은 표면값 계산의 증거다. GPU 바인딩·최종 픽셀·원작 화면·실제 성능을 검증한 것으로 기록하지 않는다.

### 후속 방어 ID — 렌더링 전체 범위의 남은 질문

| ID | 범위 | 다음에 방어할 질문과 출발점 |
|---|---|---|
| R02 | PBR·IBL·SH·RNM | diffuse 간접광과 specular 환경 반사를 왜 같은 자료로 처리하지 않는가? [MapSourceDirectionalLightmapWeight](C:/Users/tnest/Desktop/LostArk/Client/Bin/ShaderFiles/Shader_MapMaterialSurface.hlsli:526)와 [MapSourcePBRSHColor](C:/Users/tnest/Desktop/LostArk/Client/Bin/ShaderFiles/Shader_MapMaterialSurface.hlsli:572)에서 기저·공간·계수를 손추적해라. |
| R03 | 캐릭터 재질·헤어 | 같은 geometry와 texture라도 draw 경로가 다르면 무엇이 달라지나? [Render_SourceHairMaskedMesh](C:/Users/tnest/Desktop/LostArk/Client/Private/DeferredMaterialRenderUtils.cpp:276)의 깊이 처리와 임시 상수 복원·실패를 설명해라. |
| R04 | 물 | 물 program 41의 깊이·카메라·scene sample 입력을 추적해라. 42까지 같은 식으로 설명할 수 있나? [물 adapter](C:/Users/tnest/Desktop/LostArk/Client/Bin/ShaderFiles/Shader_SourceMapForwardPrograms.hlsli:910)와 [MapAssetObject의 snapshot 요청](C:/Users/tnest/Desktop/LostArk/Client/Private/MapAssetObject.cpp:179)을 연결해라. |
| R05 | AO·후처리·화면 공간 효과 | SSAO가 어떤 수신 재질과 조명 성분에 적용되는가? bloom·tone mapping·색 보정 LUT·BRDF LUT를 구분해라. [Draw](C:/Users/tnest/Desktop/LostArk/Engine/Private/Renderer.cpp:871)에서 실행 조건과 실제 shader consumer를 따라가라. |
| R06 | Unreal 비교·이관 | 공식 문서의 동일 기능을 한 개 고른 뒤 현재 구현의 입력·pass·출력과 비교해라. 원본 UE3 cooked 자료 복원, 현재 Unreal 기능 조사, 실제 마이그레이션 완료를 분리해라. 이 카드는 공식 출처 확인 전 기술 단정을 하지 않는다. |

## E01 이펙트와 툴 — 저장한 정의와 살아 있는 발생을 구분해라

대상: PPT 15~16쪽. 첫 방어는 **정의→준비→발생→편집 저장→다음 발생**이다. particle·trail·decal·mesh·screen의 모든 내부 구현을 한 번에 검토한 것으로 처리하지 않는다.

**좋다. E01 방어 개시. “이펙트 툴에서 수정했다”로 끝내지 마라. 저장 직전 재생 중이던 효과와 저장 직후 새로 발생한 효과가 무엇을 읽는지 가져와라.**

### ⓪ 먼저 열 곳

| 순서 | 실제 시작점 | 직접 찾아서 가져올 것 |
|---|---|---|
| 1 | [Effect_AuthoringDocument.h: EFFECT_ELEMENT_DESC](C:/Users/tnest/Desktop/LostArk/Client/Public/Effect_AuthoringDocument.h:1515), [EFFECT_DOCUMENT_DESC](C:/Users/tnest/Desktop/LostArk/Client/Public/Effect_AuthoringDocument.h:2074) | stable ID·시간·carrier·재질·resource·부착을 구분해라. 정의에 있는 값과 실행 중 따로 필요한 상태를 적어라. |
| 2 | [Effect_DocumentRenderer_PreparedDocument.cpp: Build_PreparedDocument](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_DocumentRenderer_PreparedDocument.cpp:29) | document identity와 revision을 왜 같이 검사하는가? resource 준비 중 실패하면 `OutPrepared`가 언제 바뀌는지 찾아라. 긴 family 분기는 우선 선택한 한 종류만 읽어라. |
| 3 | [Effect_PresentationService.cpp: Spawn](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_PresentationService.cpp:5068), [Reload_SelectedProductEffect](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_PresentationService.cpp:4194) | 제품 발생과 preview의 문서 선택을 비교해라. Catalog 교체와 GPU target 교체의 실제 순서·복원 경로를 적어라. |
| 4 | [Effect_Tool_DocumentIo.cpp: Try_SaveDocument](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_Tool_DocumentIo.cpp:173) | 메모리 draft·디스크 baseline·Product target을 따로 적어라. 저장 성공 후 activation 실패와 rollback 실패의 상태를 구분해라. |
| 5 | [Effect_DocumentCodec.cpp: Save_EffectDocumentAtomic](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_DocumentCodec.cpp:1370) | 임시 기록·round trip·freshness 비교·백업·교체·실패 복원을 따라가라. 함수 이름만으로 모든 동시 writer가 보호된다고 결론 내리지 마라. |

배경은 [Effect family/runtime ABI 가이드](C:/Users/tnest/Desktop/LostArk/.md/TEAM/EFFECT_FAMILY_RUNTIME_ABI_RESTORATION_GUIDE.md)와 [Save 단일 정본 전환 결과](C:/Users/tnest/Desktop/LostArk/.md/GB/08-25/2026-08-25_EFFECT_SAVE_IS_RUNTIME_CANONICAL_IMPLEMENTATION_RESULT.md)다. 08-25 결과의 당시 수량과 Resources Git 전달 방식은 현재 전체 상태의 증거로 쓰지 않는다. 현재 Resources 전달은 AGENTS/TEAM의 Drive 관리 계약을 따른다.

### ① 문제 — 15초

- 같은 효과를 두 번 발생시키고, 첫 번째가 살아 있는 동안 정의를 수정한다고 가정해라. 두 실행이 서로 침범하면 안 되는 상태는 무엇인가?
- “Save 버튼이 성공했다”는 말은 파일 저장·미리보기·다음 제품 발생·현재 발생 중 무엇을 보장해야 하나? 네가 말할 범위를 고정해라.
- 효과가 보이지 않는 상황에서 문서 준비 실패와 실제 draw 실패를 무엇으로 구분할 것인가? particle 개수가 양수라는 사실만으로 화면 출력을 확정하지 마라.

### ② 직접적인 해법과 한계 — 20초

- 모든 발생이 하나의 수정 가능한 정의를 그대로 참조하면 중간 편집 때 무엇이 깨질 수 있나? 수명·요소 삭제·재질 교체 중 하나로 구체적인 반례를 들어라.
- JSON 파일부터 덮어쓰고 나중에 resource를 준비하면 어떤 실패 상태가 남나? 저장 성공과 활성화 성공을 분리해야 하는 이유를 말해라.
- 모든 occurrence에 새 shader 파일을 만들거나, 모든 family에 같은 texture 슬롯을 강제하면 어떤 중복 또는 입력 오류가 생기나? 같은 program 재사용과 같은 입력 계약을 구분해라.

### ③ 메커니즘 — 25초

- effect asset ID, element ID, occurrence ID가 각각 무엇을 식별하나? 같은 정의를 공유하는 두 실행의 시간과 부착 대상은 누가 소유하나?
- 현재 Save에서 catalog candidate 준비, catalog commit, prepared target 교체가 실제로 어떤 순서인가? GPU 준비 실패 때 어떤 상태를 되돌리고, 돌아가지 못하면 어떻게 드러내나?
- carrier·material program·descriptor·renderer adapter를 하나씩 골라 연결해라. HLSL의 식이 맞아도 입력 UV·DynamicParameter·RT 계약이 다르면 왜 틀릴 수 있나?

### ④ 대조 — 15초

- 정의 전체 복사와 불변 정의 공유는 어떤 비용과 수명 조건이 다른가? 포인터를 공유한다는 사실만으로 수정 안전성을 확보한 것인가?
- Effect의 현재 직접 authored 소비와 Map/Gameplay의 publisher 경로를 비교해라. 모든 domain을 한 저장 방식으로 통일하면 잃는 계약은 무엇인가?
- baseline 비교와 OS 수준의 동시 writer 직렬화는 같은 보장인가? 두 writer가 비교와 교체 사이에 끼는 가정 상황을 그려라. 현재 코드가 보장하지 않는 범위를 말해라.

### ⑤ 대가 — 15초

- 진행 중 occurrence가 이전 정의를 유지하는 정책으로 무엇을 얻고, 메모리·준비 시간·편집 반영 시점에서 무엇을 지불하나?
- 문서 하나의 누락 resource 때문에 준비를 거절하는 정책은 왜 필요할 수 있나? 정상 항목 유지와 오류 격리의 단위를 어디로 정했는지 설명해라.
- 문서 parse, 준비 성공, 실제 GPU draw, 사용자 화면 판정은 무엇을 각각 증명하나? 네가 확보한 증거보다 큰 복원 주장을 하지 마라.

### 직접 구현·검증 — 두 발생과 한 번의 정의 교체

제품 파일을 수정하지 않는 학습용 C++ 코드에서 효과 정의 한 개와 실행 A/B만 구현해라. 어떤 상태를 정의에 두고 어떤 상태를 각 실행이 가져야 하는지 먼저 적어라. 이 연습은 실제 파일·GPU 준비 대신 성공 또는 실패를 입력받는 작은 준비 함수를 써도 된다. 생략한 제품 제약을 반드시 적는다.

**가정 입력:** 첫 정의의 수명 1,000ms로 A를 시작한다. A의 나이가 300ms일 때 수명 2,000ms인 후보를 적용하고 B를 시작한다. A/B 각각 종료 시점과 참조 정의를 실행 전에 예상해라. 이어 같은 교체를 준비 실패 조건으로 반복하고, 기존 정의와 A가 보존되는지 확인해라. 다음에는 같은 이름의 요소 두 개 또는 잘못된 수명을 넣어 실패 위치를 확인해라.

제출물은 정의·실행 상태 표, 실행 전 예상, 실제 시간별 로그, 실패 후 보존 상태, 제품에 다시 연결한 함수다. 작은 메모리 실험을 실제 Effect 파일 원자 저장·GPU 준비·화면 검증 성공이라고 기록하지 않는다.

### 후속 방어 ID — 이펙트 전체 범위의 남은 질문

| ID | 범위 | 다음에 방어할 질문과 출발점 |
|---|---|---|
| E02 | 시간·발생·수명 | 같은 시각 재평가, 큰 delta, seek, loop, 자연 종료와 강제 종료를 구분해라. [CEffectPlayback::Update](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_Playback.cpp:4306)에서 실제 상태를 찾아라. |
| E03 | 부착·좌표계·크기 | owner가 움직였을 때 snapshot과 follow는 무엇이 달라지나? [Requires_SourceBoneImportScaleNormalization](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_PresentationService.cpp:4256)에서 적용 대상과 실제 본 basis를 대조하고, 전체 effect를 100배 키우는 보정과 구분해라. |
| E04 | shader 입력 ABI | Dynamic 모듈이 없는 경우의 기본 입력은 무엇을 근거로 결정하나? [Effect_Playback.cpp의 Null Dynamic 준비](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_Playback.cpp:2792)에서 선택 VF와 실제 particle 소비를 연결해라. |
| E05 | Trail·Decal·Mesh·Screen | sprite와 같은 geometry·좌표·blend를 써도 되는가? 한 carrier를 선택해 생성 입력·vertex stream·material·render target의 계약을 추적해라. [ABI 가이드의 네 축](C:/Users/tnest/Desktop/LostArk/.md/TEAM/EFFECT_FAMILY_RUNTIME_ABI_RESTORATION_GUIDE.md:124)을 입구로 쓴다. |
| E06 | 편집·저장·반영 실패 | 편집 draft, 디스크, catalog, prepared target, 기존 발생, 다음 발생을 여섯 칸으로 두고 정상 저장·외부 변경·GPU 준비 실패·rollback 실패를 각각 추적해라. [Try_SaveDocument](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_Tool_DocumentIo.cpp:173)에서 시작한다. |
