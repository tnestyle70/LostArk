# Debug Effect 로딩과 KoukuSaydon·Valtan Publish 비교 검토

2026-10-04 현재 코드의 읽기 전용 조사 결과다. 이 문서는 구현 계획서가 아니다. 아래의 Effect 준비 캐시 제안은 구현되지 않았으며, 조사 중 publisher·Client·Server를 실행하거나 authored/runtime 데이터를 교체하지 않았다.

## Save와 Publish, 디스크와 실행 메모리

| 동작 | 현재 바뀌는 상태 | 그 동작만으로 완료되지 않는 일 |
|---|---|---|
| Sequencer 편집 | 도구가 보유한 메모리 draft | 디스크 저장, 제품 데이터 생성, Server 적용 |
| ActionWorkbench의 Save 단독 | 해당 domain의 authored 정본과 저장 revision | 제품 재생 데이터 생성, 실행 중 Server revision 변경 |
| Raid Publish | 저장된 입력에서 검증·projection한 제품 파일과 receipt/generation | 실행 중 Client/GPU 객체 준비와 실행 중 Server의 승인된 적용 |
| 런타임 stage → commit | 현재 process의 catalog·prepared resource·활성 revision | authored 원본 수정이나 publish 파일 생성 |

Kouku Save는 `m_Document.Save_Atomic` 성공 후 LastGood를 draft로 받아 저장 상태를 갱신한다. 이어지는 `Publish_AllPatterns`는 dirty/freshness를 확인하고 저장 revision을 넘겨 별도 domain publisher를 실행한다. 따라서 Save와 Publish는 실제 호출도 구분된다. [KoukuSaydonActionWorkbench.cpp:1823](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonActionWorkbench.cpp:1823), [Publish_AllPatterns:1846](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonActionWorkbench.cpp:1846), [publisher command:1931](C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonActionWorkbench.cpp:1931).

Valtan은 버튼과 함수 이름을 구분해야 한다. ActionWorkbench의 **Save 단독**은 저장 validation·owner baseline·atomic commit을 수행하고, Product/native resource 조인은 Publish에 맡긴다. 같은 도구의 **Save & Publish**는 저장을 끝낸 뒤 그 정확한 revision receipt로 `Begin_ValtanProductPublishRetry`를 이어 간다. 반면 Balance Tool의 **Save & Apply**인 `Save_ValtanProduct`는 canonical 저장, `Publish_ValtanCandidate`, Server 적용 준비까지 묶은 합성 명령이다. 따라서 함수 이름에 Save가 있다고 언제나 authored 파일만 바꾼다고 설명하면 잘못이다. [ValtanActionWorkbench.cpp:6715](C:/Users/tnest/Desktop/LostArk/Client/Private/ValtanActionWorkbench.cpp:6715), [storage validation:6810](C:/Users/tnest/Desktop/LostArk/Client/Private/ValtanActionWorkbench.cpp:6810), [저장 요청:6884](C:/Users/tnest/Desktop/LostArk/Client/Private/ValtanActionWorkbench.cpp:6884), [Publish_AfterSave:7033](C:/Users/tnest/Desktop/LostArk/Client/Private/ValtanActionWorkbench.cpp:7033), [Balance Save & Apply:6483](C:/Users/tnest/Desktop/LostArk/Client/Private/BalanceTool.cpp:6483).

Valtan의 `Publish_ServerRuntimeSet`은 정확한 저장 revision SHA-256를 확인한 다음 `Run-FullPipeline.ps1 -DataOnly`를 실행한다. immutable candidate의 `ApplyCandidate`는 Server 연결·world·transaction identity를 확인하며, Server의 같은 candidate revision 응답을 보기 전까지 적용 성공을 선언하지 않는다. 디스크에 게시된 candidate와 Server가 현재 실행하는 revision은 별개다. [BalanceTool.cpp:5629](C:/Users/tnest/Desktop/LostArk/Client/Private/BalanceTool.cpp:5629), [ValtanTuningCommandService.cpp:177](C:/Users/tnest/Desktop/LostArk/Client/Private/ValtanTuningCommandService.cpp:177), [응답 확인:314](C:/Users/tnest/Desktop/LostArk/Client/Private/ValtanTuningCommandService.cpp:314).

이 표는 raid 도구 흐름을 설명한다. Effect Tool의 Save는 authored 파일 원자 교체와 다음 spawn target 활성화를 한 transaction으로 묶는 별도 계약이 있다. 그렇더라도 raid gameplay publish나 Server simulation 적용과 같지는 않다. [UNIFIED_DATA_MANAGEMENT_ARCHITECTURE.md:498](C:/Users/tnest/Desktop/LostArk/.md/TEAM/UNIFIED_DATA_MANAGEMENT_ARCHITECTURE.md:498).

## KoukuSaydon Publish가 이미 처리하는 일

도구가 호출하는 `Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon`은 `koukusaydon.product`, `map.kakulsaydon`, `world.gameplay`, `gameplay.balance` 순서의 domain을 처리한다. world publisher는 같은 구현을 `KAKULSAYDON_ARENA` 출력 범위로 좁혀 사용한다. [Invoke-BuildDomainOwner.ps1:63](C:/Users/tnest/Desktop/LostArk/Tools/Build/Invoke-BuildDomainOwner.ps1:63).

Composition projection은 다음 두 파일을 만든다.

- `Data/Encounters/KoukuSaydon/KoukuSaydonEncounter.json`: 전투 실행용 pattern·stage·logic·raid flow 데이터.
- `Data/Animation/Authored/KoukuSaydon/KoukuSaydon.patternbindings.json`: Client presentation 연결 데이터.

그 뒤 Map/World domain은 `Client/Bin/DataFiles/Map`의 맵 배치·WorldSequence·Camera와 `Client/Bin/DataFiles/World`의 NPC presentation·stage marker 등을 만든다. Server에는 `Server/Bin/DataFiles/World/KAKULSAYDON_ARENA.*bootstrap`과 `Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap`이 게시된다. 한 버튼이 여러 종류의 디스크 제품을 만든다. [BuildDomains.json:45](C:/Users/tnest/Desktop/LostArk/Tools/Build/BuildDomains.json:45), [World 출력:92](C:/Users/tnest/Desktop/LostArk/Tools/Build/Invoke-BuildDomainOwner.ps1:92), [Gameplay 출력:430](C:/Users/tnest/Desktop/LostArk/Tools/Build/BuildDomains.json:430).

비싼 계산을 실행 전에 옮기는 구조는 이미 있다. Parent의 반복·stage expansion, 설치 WModel의 애니메이션과 bone을 사용한 Collider 궤적, World object Collider의 시간별 transform을 publisher가 계산한다. Server가 Client 모델을 직접 재생하지 않고 게시된 판정 데이터를 소비할 수 있는 이유다. [Parent expansion:1454](C:/Users/tnest/Desktop/LostArk/Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py:1454), [Bone bake:4417](C:/Users/tnest/Desktop/LostArk/Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py:4417), [World sample:4624](C:/Users/tnest/Desktop/LostArk/Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py:4624), [최종 출력:6212](C:/Users/tnest/Desktop/LostArk/Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py:6212).

`_try_validation_certificate`는 도구·Python·입력·출력의 exact hash를 확인하여 publisher 검증을 재사용한다. 같은 크기와 수정 시각만으로 승인하지 않는다. 이 certificate는 publisher의 재검증 캐시이며 Client의 Effect 로딩 캐시는 아니다. [project_kouku_saydon_composition.py:6395](C:/Users/tnest/Desktop/LostArk/Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py:6395).

## Valtan Publish가 이미 처리하는 일

`project_v2_products`는 Valtan authoring에서 Encounter·rotation·combat object·World event·animation binding·Effect cue와 provenance 등을 투영한다. candidate 게시 경로는 저장 원본과 candidate overlay를 검증하고, Server가 읽는 `Gameplay.bootstrap`과 Client presentation generation을 함께 봉인한다. [제품 목록:133](C:/Users/tnest/Desktop/LostArk/Tools/ValtanPipeline/valtan_tuning_pipeline.py:133), [project_v2_products:7470](C:/Users/tnest/Desktop/LostArk/Tools/ValtanPipeline/valtan_tuning_pipeline.py:7470), [candidate stage:14966](C:/Users/tnest/Desktop/LostArk/Tools/ValtanPipeline/valtan_tuning_pipeline.py:14966), [Server bootstrap 생성:13107](C:/Users/tnest/Desktop/LostArk/Tools/ValtanPipeline/valtan_tuning_pipeline.py:13107).

Valtan의 presentation generation은 Effect cue에서 EffectCatalog를 따라 실제 `Data/Effects/Authored/*.effect.json`까지 수집한다. `_stage_presentation_generation_closure`는 해당 원문 bytes를 candidate에 보존하고 generation hash가 바뀌지 않았는지 확인한다. 따라서 Effect가 게시 검증과 전혀 관계없다는 설명도 정확하지 않다. 다만 이 단계는 원문 파일과 hash를 봉인하며, Effect JSON을 런타임 typed document로 바꿔 저장하는 cook 단계는 아니다. [Effect closure:199](C:/Users/tnest/Desktop/LostArk/Tools/GameplayPipeline/valtan_presentation_generation.py:199), [generation hash:562](C:/Users/tnest/Desktop/LostArk/Tools/GameplayPipeline/valtan_presentation_generation.py:562), [closure stage:13311](C:/Users/tnest/Desktop/LostArk/Tools/ValtanPipeline/valtan_tuning_pipeline.py:13311).

## 지금도 Client 실행 때 남는 Effect 작업

V1 Product Effect는 `Data/Effects/Authored/<EffectAssetId>.effect.json`을 직접 읽는다. 별도 `Client/Bin/DataFiles/Effect` 복사본이나 Effect publisher는 현재 없다. 위 raid publisher가 실행되었다는 이유로 Effect 본문의 parse 비용이 사라지지는 않는다. [현재 계약:478](C:/Users/tnest/Desktop/LostArk/.md/TEAM/UNIFIED_DATA_MANAGEMENT_ARCHITECTURE.md:478), [direct load:932](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_Catalog.cpp:932).

현재 경로는 `CEffectDocumentCodec::Load → CDataJson::Parse → Parse_Value → Validate`다. JSON의 `sourceRecipe` 등 필요한 실행 정보를 typed descriptor로 읽고 검증한다. v15 runtime carrier 중 adapter가 필요한 경우 document-owned projection도 준비한다. 이 동작은 Debug에만 있는 검증이 아니다. [Load:1294](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_DocumentCodec.cpp:1294), [Parse:252](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_DocumentCodec.cpp:252), [sourceRecipe:817](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_DocumentCodec.cpp:817), [typed validation:870](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_DocumentCodec.cpp:870), [projection 조건:83](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_DocumentCodec.cpp:83), [projection 생성:1310](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_Catalog.cpp:1310).

renderer 준비는 추가로 `Validate_Drawable`, particle resources와 model cue 준비를 수행한다. 여기에는 현재 `ID3D11Device`·context identity·catalog revision과 실제 CModel·texture 객체가 필요하다. 이 객체들을 디스크 Publish 결과로 그대로 저장할 수는 없다. [renderer stage:311](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_DocumentRenderer_Catalog.cpp:311), [drawable 및 resource 준비:87](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_DocumentRenderer_PreparedDocument.cpp:87), [CModel 생성:2704](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_DocumentRenderer_ResourceStaging.cpp:2704), [texture 생성:501](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_DocumentRenderer_CacheHelpers.cpp:501).

준비가 끝나도 catalog와 renderer를 현재 process에 commit하는 단계는 필요하다. 현재 구현은 catalog commit 뒤 renderer commit이 실패하면 catalog를 rollback한다. 이것은 디스크 게시를 뜻하는 Publish와 다른 메모리 transaction이다. [Effect_PresentationService.cpp:3536](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_PresentationService.cpp:3536).

## 검토 결론과 미구현 제안

KoukuSaydon·Valtan Publish에서 Effect의 정적 준비 작업까지 앞당기는 것은 가능한 방향이다. 현재도 raid projection과 dependency closure를 게시 전에 계산하므로, 해당 입력 closure에 공통 Effect 준비 단계를 결합할 자리가 있다. 현재 publish만 다시 실행해서 Client의 JSON parse가 자동으로 없어지는 구조는 아니다.

미구현 제안은 authored 원문을 유일 정본으로 유지하면서, 같은 `EFFECT_DOCUMENT_DESC`와 renderer가 소비할 수 있는 버전이 있는 compact 준비 산출물을 파생시키는 것이다. JSON 파일 복사나 공백 제거만으로는 DOM→typed 변환 비용이 없어지지 않는다. 산출물에는 source hash·schema/cooker version과 실제 사용한 의존 데이터의 hash가 연결되어야 한다. 새 산출물을 읽을 때도 byte/범위/identity 검증과 실패 시 기존 target 보존은 필요하다.

여기서 준비 파일은 C++ struct의 메모리를 그대로 저장하는 파일이 아니다. `vector`, `string`, pointer는 현재 process의 주소와 라이브러리 배치를 포함한다. 길이·숫자·문자열 등의 명시적 저장 형식으로 기록하고 기존 typed descriptor로 읽어야 Debug/Release와 다음 실행에서도 사용할 수 있다. 또한 Publish process가 JSON을 읽기만 하고 종료하면 그 메모리는 사라지므로, 지속 산출물과 Client 소비자 연결이 모두 있어야 한다.

입력 hash가 같은 정적 JSON 해석·정적 validation·resource 목록·고정된 projection 일부는 이 단계로 이전할 후보다. 실제 GPU 객체 생성, 현재 모델의 bone·owner·재생 시간에 따른 계산, 실행 중 catalog/renderer commit은 Client에 남는다. 저장하지 않은 Preview draft는 아직 Publish 산출물이 없으므로 편집 메모리와 같은 runtime consumer로 연결되어야 한다. 새 파일 형식의 도입 여부와 정확한 캐시 경계는 이번 조사에서 구현하거나 승인된 것으로 취급하지 않았다.

따라서 이번 DataJson 내부 이동 비용 개선은 현재 Save·Preview·일반 JSON 소비자에도 효과를 낼 수 있는 직접 최적화이고, raid Publish에 정적 준비를 옮기는 작업은 첫 로딩에서 해야 할 일의 양을 줄이는 별도 확장이다. 둘은 함께 적용할 수 있으며, 실제 이득은 CPU document 준비와 GPU resource 준비 시간을 나누어 측정해야 한다.
