# 도화가·창술사·워로드와 전체 맵 복원 범위·견적

## G00. 이번 견적의 기준

2026-09-08 21시 전후 KST의 현재 working copy와 `렌더링 색감 및 맵 점검` 작업
(`01a07adf-f961-7d00-82e6-c104525ccfc7`) 기록을 대조했다. 사용자는 우선 현재 상황을
전수 조사하고 견적을 요청했다. 이 문서는 구현 범위와 작업량의 추정이며 제품 변경 결과가 아니다.

현재 branch는 `codex/kouku-ball-motion-effects`, HEAD는
`f92178f054a759c128a61cebd8e2ff4b8712eb12`이고 fetch 후 HEAD와 origin/main은 같았다.
실제 작업 폴더에는 다른 작업의 대규모 미커밋 변경이 있다. 아래 수량은 HEAD만의 상태가
아니라 그 변경을 포함한 관찰값이다. 기존 차원술사 작업의 PLAN/RESULT는 계속 그 작업이 소유한다.

현재 조사 결과와 전체 57개 스킬 표는
[대응 RESULT](2026-09-08_THREE_CLASS_AND_MAP_RESTORATION_ESTIMATE_RESULT.md)에 둔다.
공통 복원 원리의 정본은 [렌더링이펙트복원V2.md](../렌더링이펙트복원V2.md)다.
차원술사 구현 이력은 [기존 RESULT](2026-09-08_CHARACTER_MATERIAL_AND_EFFECT_ROUND2_ESTIMATE_RESULT.md),
[기존 PLAN](2026-09-08_CHARACTER_MATERIAL_AND_EFFECT_ROUND2_ESTIMATE_PLAN.md)을 연결한다.

전수조사의 분모는 세 직업의 **현재 57개 skill definition, 83개 catalog 등록 authored 문서,
1,021개 element**, 네 맵의 현재 catalog/placement와 선택 material/light 입력이다.
원본 게임의 모든 package·shader permutation을 새로 역분석하거나 모든 프레임을 재생한 조사는 아니다.
큰 세 맵의 원본 프로그램 전수 대응 미완료도 아래 견적의 실제 작업 항목이다.

## G01. 재사용할 공통 경로와 먼저 보완할 부분

이펙트의 현재 제품 입력은 `Data/Effects/EffectCatalog.json`과 `Authored/*.effect.json`이다.
Animation의 `effectref=asset` → CEffectPresentationService → CEffectObject →
CEffectDocumentRenderer/CEffectPlayback의 기존 경로를 확장한다. `.restore` 편집기 복구본을
추가한 것과 실제 스킬 cue를 교체한 것은 별개다. 과거 sealed Effect runtime/publisher를 되살리지 않는다.

같은 프로그램·입력이면 현재 식과 renderer를 재사용한다. 원본 parent 이름이 같다는 이유로
모든 분기가 동일하다고 판단하지 않는다. 저장된 grouped profile도 현재 strict identity selector를
거쳐 구체적인 식을 실행할 수 있으므로 `grouped=미복원`, `execution=0=렌더링 없음`으로 세지 않는다.

현재 재발 가능성이 가장 큰 공통 결함은 importer의 기본값 상속이다.
`Tools/LevelPlacementExtractor/build_imported_effect_documents.py`의 SourceIndex는 raw properties를
복사하며 일반적인 CDO → archetype → instance 필드 병합을 수행하지 않는다.
Q JSON의 8개 분포를 교정한 결과는 다른 스킬 재생성에 자동 적용되지 않는다.

보완 범위는 기존 SourceIndex의 명시 원본 상속 입력, distribution/duration 소비,
단위·기준축 적용 위치다. 명시 0/false/null과 LUT를 보존하고 배열은 원본 의미대로 교체한다.
sourceRecipe를 꺼서 처리하지 못한 움직임을 없애거나 모든 default를 1로 채우지 않는다.
Q의 회전·radial velocity 교정을 작은 비교 기준으로 재사용하고, 새 원본 입력의 오류는 해당 항목에 남긴다.

주요 기존 파일:

| 위치 | 이 작업에서 맡는 책임 |
|---|---|
| `Tools/LevelPlacementExtractor/build_imported_effect_documents.py` | 원본 상속·분포를 기존 변환으로 전달 |
| `Client/Private/Effect_Playback.cpp` | 입자 시간·위치·회전·동적 값 소비 |
| `Client/Public/Effect_MaterialTemplate.h` | 실제 source identity로 기존 계산 선택 |
| `Client/Private/Effect_DocumentRenderer.cpp` | 필수 texture/uniform/geometry/scene 입력과 실제 pass 연결 |
| `Client/Bin/ShaderFiles/Shader_EffectUe3MaterialFamilies.hlsli`와 기존 전용 HLSLI | 확인된 식 재사용, 다른 계산만 추가 |
| `Client/Private/EffectAuthoringSequencer.cpp` 및 Camera 분리 구현 | 전체 재생 시계·구성·seek·종료 복귀 |
| `Client/Private/Character.cpp`, `AnimationEffectCueDocument.cpp` | 실제 action/clip/cue와 카메라 shake 소비 |

Camera row·SceneColor·Depth의 공통 기능은 차원술사 작업이 현재 확장 중이다. 완료된 계약을
받아 사용하는 것이 기본이며 이 견적에서 같은 기능을 다시 만드는 비용을 직업별로 넣지 않는다.
이후 새 C++ 파일을 추가하게 되면 같은 변경에서 해당 `.vcxproj`와 `.vcxproj.filters`에 등록한다.
이 문서 단계에서는 신규 C++ 파일을 제안하거나 생성하지 않았다.

## G02. 도화가 복원 단위

F 한획긋기 17개 요소는 사용자가 손조정해 만족한 비교 기준으로 보존한다.
다른 스킬도 원본의 전체 발생 항목과 현재 손조정본을 대조한 복구본으로 시작한다.
과거 문서보다 행 수가 작다는 이유만으로 삭제된 행을 전부 되살리지 않는다.

| 순서 | 범위 | 실제 남은 연결 |
|---|---|---|
| D 31490 범가르기 | 68개 요소, 기존 stroke 12행 | 호랑이 먹선의 계산·coverage와 나머지 구성의 발생 위치·시간 |
| T 31950 미르 새김 | 23행 중 visible 11, 15행 실행 억제 | visible helix 3행을 포함한 MakeFlow/리본·굴절·용 형상. 원본이 별도 모델을 요구하는지 먼저 확인 |
| V 31910 몽유도원 | 46행, 기존 집결 보정 | 원본 장면색·빛·합성, PROJECT_TUNED attractor와 원본 운동의 차이 |
| Alt+V 31930 몽중백화원 | 2클립, 97행 | FlowerGarden 등 미대응 재질, 전체 시간 구성·화면 연출 |
| 일반 슬롯·BA·Z·X·Space | 기존 12개 정의 | 정상 손조정본 보존, cue 없는 X/Space의 원본 역할 확인·필요 연결 |

D/T/V/Alt+V의 카메라 SHAKE는 이미 Character action timeline에서 소비한다.
기존 shake를 다시 구현할 필요는 없다. 전체 cinematic, Light/ScreenPost와 그 수명·취소는
별도 원본 입력이 필요하므로 각 스킬의 실제 구성과 대조한다.

## G03. 창술사 복원 단위

긴 창·짧은 창을 모두 포함한 24개 정의가 현재 분모다. 자세가 다르면 같은 Q 키도 다른
skillId를 사용한다. Controller에 ID를 추가 하드코딩하지 않고 현재 catalog resolve를 유지한다.

| 범위 | 현재 저장본 | 복원 초점 |
|---|---|---|
| D 반월섬 34110 | 2개 요소 | 원본 검격·파편과 현행 손조정본 대응 |
| F 맹룡열파 34150 | 3개 요소 | 용 flamesurface, glow/trail과 실제 궤적 |
| T 적룡필살 34650 | 2문서/10개 | 클립별 발생 시점·합성 |
| V 적룡질풍격 34610 | 3문서/9개 | 연속 용/검격의 재질과 전체 시간 구성 |
| Alt+V 마룡합일섬 34630 | 4문서/210개 | 전체 41 parent 계열 중 기존 dragon opcode19 10행 재사용, 나머지 식·입력 대응 |
| 양 자세의 일반 스킬·BA·Space | 나머지 정의 | 검격·찌르기·trail 재사용, root/bone/action 방향 및 전환 시계 |

현재 F/V의 `fx_j_me_flamesurface_01_ma`는 Effect 전용 source classifier에서 명시 소비가
확인되지 않아 새 계산 또는 기존 식 재사용 대조가 필요한 우선 대상이다.
모든 클립이 자기 Effect 문서를 가져야 하는 것은 아니다. 원본 cue가 없는 clip에 임의 cue를
붙이거나 이미 통합된 구성에 중복 생성하지 않는다. Z 무기 전환은 현재 원본 EFFECT의 nonempty
payload 자체가 확인되지 않으므로 문서 0개를 곧바로 복원 결손으로 세지 않는다.

## G04. 워로드 복원 단위

| 범위 | 현재 저장본 | 복원 초점 |
|---|---|---|
| Z 방어 태세 17800/17810 | Effect 문서 0 | 기존 서버 자세와 별개로 시작 FilmNoise/ApShield, 유지·해제·사망·전환 시 정리 |
| F 가디언의 낙뢰 17140 | 4개 요소 | 원본 낙뢰 형상·WPO/시간식, 기존 전기 메시 손조정본과 대조 |
| T 풀배럴 캐넌 17240 | 3문서/17개 | 중간 clip03/04 원본 cue가 hold 문서에 합쳐졌는지 확인 |
| V 가디언의 수호 17170 | 3문서/9개 | 보호·타격·종료의 전체 구성 |
| Alt+V 수호의 맹세 17250 | 2문서/262개 | RealPBR weapon, rock, worldoffset, decal, FilmNoise/ZoomBlur 등 전체 연출 |
| 일반 슬롯·BA·X·Space | 나머지 정의 | 기본 방패·불꽃·창 표현과 입력·hit/종료 동기화 |

Z는 `src=orig` 텍스트만 있는 원본 cue가 현재 제품 Effect로 자동 재생되지 않는 구체적인 연결
공백이다. 다만 자세 자체의 idle/run과 서버 권위 동작까지 없다는 뜻은 아니다.
F opcode22 C++/HLSL의 존재도 현재 F 문서가 그 경로를 선택한다는 증거가 아니다.
현재 문서·실제 selector·renderer를 한 줄로 연결해 확인해야 한다.

## G05. 맵과 캐릭터 재질의 확대

Bern·Valtan·Kouku는 현재 descriptor가 full map scope다. CS는 선택 아레나 범위와 배경을
로드한다. 전체 화면 복원은 기존 geometry를 무작정 더 추출하는 작업과 다르며 아래 단위로 나눈다.

| 대상 | 현재 규모 | 다음 연결 |
|---|---|---|
| CS | 803배치, source mesh55, 현재 catalog63 | 기존 9재질/29배치 조명에서 나머지 원본 표면으로 확대 |
| Valtan | catalog272, 13,184배치 | 재질 override, 베이크 조명·환경, 눈에 보이는 불꽃/발광 표면 |
| Kouku | catalog323, 3,231배치 | 복구한 2표면+Mirror sampler3행 이후 관문 바닥·장식·조명·화면 연출 확대 |
| Bern | 13shard, 고유 asset1,003, 50,017배치 | 실제 재질 그룹화, 석재·landscape·foliage·water와 장소별 조명 |

CS는 75 source material / 14 terminal family / 179 render-input 조합까지 기존 원본 조사 자료가
있다. 실제 활성 source material 6종과 baked/base/vertex/light 프로그램이 같은 추가 4종,
27배치는 먼저 기존 식의 재사용 대상으로 확인할 수 있다. 동일 shader라도 UV1, atlas scale/bias,
environment, shadow·visibility를 배치별로 다시 연결한다. 27배치는 아직 적용 완료가 아니다.

큰 세 맵은 기존 binary metadata상 used material row가 Bern1,453/Valtan347/Kouku399다.
이 2,199행은 새 shader 2,199개라는 뜻이 아니다. 같은 material·parent·선택 program을 묶는
조사가 선행하며 이 결과가 큰 맵 견적의 범위를 줄인다. 전체 원본 프로그램 대응은 아직 닫히지 않았다.

캐릭터는 45개 실제 body/equipment/weapon 모델, material slot103개를 조사한 기존 자료를
재사용한다. mip/filter 개선과 원본 표면식 활성은 분리한다. 이번 검토 중 CharacterCatalog에
창술사5/차원술사17 override가 실제 추가됐다. 최신 코드/JSON 추가를 반영하되 새 CModel 로딩,
최종 EXE·화면 검증까지 완료된 것으로 승계하지 않는다. 도화가·워로드 override는 관찰 시0이다.

맵 표면의 소비 경로는 MapCatalog/Authoring → Publish-MapAuthoring → runtime →
CModel/CMaterial → static/instanced shader → deferred 조명이다. geometry/lightmap 자료는
기존 WModel reader와 material variant 경로로 운반한다. 플레이 가능 영역을 확대할 때는
Data/Navigation → publisher → Server navgrid/collision/trigger까지 같은 기능으로 연결한다.

## G06. 작업량 견적

**단위는 숙련 개발자 1명이 8시간 집중하는 개발일이다. Codex 응답시간·연속 실행시간이나
완료 날짜를 측정한 값이 아니다.** 원본 package/cache와 현재 Resources가 사용 가능하고,
사용자의 비교 피드백을 받을 수 있다는 가정의 공학적 추정이다. 사용자 확인 대기시간과 팀 작업
충돌 대기는 별도다. 이미 끝난 Q 유리·Artist F·선택 맵 표면을 처음부터 만드는 공수는 제외했다.

### 세 직업 전체

| 작업 | 개발일 | 산정 내용 |
|---|---:|---|
| 공통 importer 기본값·좌표·재생 계약 보완 | 4–8 | 기존 변환·Playback 보완. 차원술사의 완료된 공통 기능은 재사용 |
| 도화가 D | 2–4 | 기존 stroke와 남은 구성 |
| 도화가 T 미르 새김 | 4–8 | 차단된 helix·리본·용 형상·굴절 |
| 도화가 V | 3–6 | 집결·장면 입력·전체 시간 |
| 도화가 Alt+V | 5–10 | 두 clip과 FlowerGarden 특수 구성 |
| 도화가 나머지 슬롯 및 통합 | 6–11 | 일반 스킬3–6 + 제품/저장/비교3–5 |
| 창술사 일반/원본 대응/제품 통합 | 6–12 | 두 클래스 공통 대응·일반 스킬·통합 공수를 절반 배분 |
| 창술사 D/F/T/V | 6–12 | 용·검격·trail 특수 입력 |
| 창술사 Alt+V | 6–12 | 기존 dragon 이외의 전체 구성 |
| 워로드 일반/원본 대응/제품 통합 | 6–12 | 위 공통 공수의 나머지 절반 |
| 워로드 Z/F | 4–8 | 지속 방어 표현·화면 효과·낙뢰/WPO |
| 워로드 Alt+V | 8–15 | weapon/rock/worldoffset·화면 연출 |
| **세 직업 합계** | **60–118** | 도화가20–39 + 창술사18–36 + 워로드18–35 + 공통4–8 |

이 합계는 위 57개 현재 정의를 대상으로 원본 대조, 필요한 식·입력·구성, 실제 스킬 연결과
사용자 비교 수정까지 진행하는 **작업 예산**이다. generic 근사만 연결한 첫 화면의 견적이 아니다.
다만 원본의 모든 CPU 난수·엔진 상수·모든 프레임의 수학적 동일성을 보증하는 상한은 아니다.
Space/identity의 gameplay 기능 추가가 필요한 경우에는 단순 이펙트 연결 공수와 구별해 재산정한다.

처음 실행할 D→T→워로드Z/F 묶음은 공통4–8 + D2–4 + T4–8 + Z/F4–8로
**14–28 개발일**의 1차 예산이다. 이것은 세 직업 전체 합계에 포함되며 추가 합산하지 않는다.
이 묶음의 실제 작업시간과 사용자 수정 횟수로 나머지 추정을 보정한다.

### 맵·캐릭터·조작감까지 포함하는 확장

| 작업 | 개발일 | 산정 내용 |
|---|---:|---|
| CS 동일 프로그램27배치 | 2–4 | 새 식보다 geometry/atlas/환경 입력과 variant/publish |
| CS 나머지 주요 재질·family | 12–25 | 75재질/14family 조사 활용, 전체 배치 association |
| Bern/Valtan/Kouku 원본 재질·program 대응 | 8–18 | 2,199 used material row 그룹화와 실제 배치 override 추적 |
| Valtan 재질·환경 표현 | 10–22 | 원본 표면·lightmap·fire/slot·환경, 기존 파괴와 회귀 |
| Kouku 재질·관문 환경 | 12–28 | 바닥·장식·조명/화면 입력, 현재 sampler 보존 |
| Bern 전체 표면·환경 | 20–45 | 13shard의 landscape/foliage/water와 배치별 조명 |
| 성능·navigation·카메라·입력 감각의 측정/수정 | 10–20 | 실제 병목·이동 영역·hit/효과/소리의 시간 대조 |
| **맵과 통합 합계** | **74–162** | 큰 세 맵의 program 대응 후 재견적할 예비 범위 |
| 현재 캐릭터·무기 재질 확장 별도 | 8–16 | 기존22 override 작업 재사용, 도화가/워로드 등 미연결 표면. 모든 신규 아바타 제작은 제외 |
| **세 직업 + 네 맵 + 현재 캐릭터의 계획 예산** | **142–296** | 60–118 + 74–162 + 8–16 |

사용자에게는 마지막 합계를 **약140–300 개발일의 넓은 예산 범위**로 전달한다.
큰 맵 원본 대응과 실제 성능 측정이 덜 끝났으므로 이 숫자를 확정 달력 일정으로 바꾸지 않는다.
여러 에이전트가 자료 조사·개별 HLSLI·직업별 데이터를 병렬로 다룰 수 있지만,
공유 Renderer/Playback/Importer, 동일 worktree의 build/publish와 사용자 화면 비교는 순서가 필요하다.
따라서 에이전트 수로 단순 나눈 기간도 제시하지 않는다.

상한을 늘리는 구체적인 조건은 새로운 geometry/vertex deformation, screen pass/MRT,
누락된 원본 lightmap/environment 상수, 원본과 다른 map composition, 새 보행 지역의 gameplay다.
기존 프로그램과 입력이 정확히 재사용되면 줄어든다. 그 판단을 parent 이름이나 element 개수가
아닌 실제 식·입력·소비자 대조로 한다. 원작 전체의 최종 동일성은 단계별 비교 관찰로 판단한다.

## G07. 구현 때의 순서와 종료 증거

첫 구현은 공통 importer에서 재현된 누락을 필요한 범위로 고치고, D 범가르기 → T 미르 새김 →
워로드 Z/F → 창술사 F/V → 세 직업 Alt+V → 나머지 슬롯 순으로 진행한다.
CS의 동일 program27배치 조사는 병렬 가능하지만 차원술사와 같은 Renderer를 동시에 편집하지 않는다.

각 스킬은 원본 발생 항목 → 현재 손조정본 → 복구본 → 실제 소비 순서로 맞춘다.
형태·색·발생 위치·수량·수명·합성·카메라/사운드 시점을 같은 action clock에서 대조한다.
프로그램 공유는 허용하지만 서로 다른 stable occurrence를 재질이 같다는 이유로 합치지 않는다.

자동 확인은 변경한 JSON/XML parse, 기존 domain 검사, 필수 입력/실패 보존, 실제 선택 pass와
zero draw·finite transform/lifetime에 필요한 focused 확인, 최소 컴파일과 diff 공백 검사다.
새 receipt/admission framework나 스킬별 별도 하네스를 기본 산출물로 추가하지 않는다.
Engine public 구조를 바꾸면 정상 Product 빌드로 SDK와 Client까지 반영한다.

사용자 확인은 `Server + Client`를 Ctrl+F5로 시작한 뒤 Character Select에서 해당 class를 선택하고,
F1 → Effect Tool → All Effects에서 복구본의 Play/Play All·Seek/Loop/저장·재로드를 비교한다.
그 뒤 실제 키 입력으로 서버 snapshot → 애니메이션 → 이펙트와 취소·전환·재입장을 확인한다.
편집기 복구본이 보이는 것만으로 실제 키 입력 연결을 완료로 기록하지 않는다.

Resources는 Git 밖의 `Client/Bin/Resources`를 사용한다. 실제 추가/교체 때 class별
`Effect/Artist`, `Effect/LanceMaster`, `Effect/Warlord`, `Character/...`, 해당 `Map/...`의 정확한
상대 ID와 물리 위치·Drive 전달 여부를 기록한다. 이번 조사에서 binary를 추가/교체하지 않았다.
