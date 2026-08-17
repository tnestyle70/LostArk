# 2026-08-17 Product Effect prewarm 정지 구간 진단과 선택지

이 문서는 "Character Select 진입 후 좌클릭 시 1FPS로 멈춘다"는 관찰의 원인을 코드와 데이터
실측으로 확정하고, 수정 방향 두 가지를 비교한다. 이번 회차에 코드는 바꾸지 않았다.

## 1. 결론

관찰된 정지는 좌클릭이 유발하지 않는다. 좌클릭 spawn 경로에는 파싱이 없다.
정지 구간은 **Character spawn 시점에 시작되어 41 프레임 동안 지속**되며,
그 41 프레임이 약 60 MB의 JSON parse와 GPU prepare를 나눠 처리한다.

## 2. 좌클릭 경로에 파싱이 없다는 근거

`CEffectPresentationService`의 spawn 진입점은 cache-only다.

```text
Client/Private/Effect_PresentationService.cpp:2472
    if (!g_ProductPrewarmQueue.Is_Prepared(Desc.strEffectAssetId))
        "Effect spawn rejected because its Product target is not prepared."

Client/Private/Effect_PresentationService.cpp:2480
    CEffectCatalog::Find_Loaded(Desc.strEffectAssetId)
```

`Find_Loaded`는 `g_Effects` 조회만 하고 miss여도 로드하지 않는다.
lazy parse를 하는 `Find`는 prewarm 경로에서만 쓴다.

```text
Effect_Catalog.cpp:4841  Find_Loaded   맵 조회만, 없으면 nullptr
Effect_Catalog.cpp:4821  Find          miss면 Load_DirectAuthoredRuntimeDocument 실행
```

`Reprepare_ProductTargets`(전체 재준비)는 Effect Tool의 publish/reload에서만 호출되고
gameplay 경로에는 없다. Character Select의 `requiresValtanPrewarm`은 Valtan presentation
prototype 준비이며 `Spawn Selected` 버튼 전용이고 좌클릭과 무관하다.

## 3. 정지 구간의 실제 소유자

```text
CCharacter::Initialize
  Character.cpp:113   Load_EffectCues()
    Character.cpp:230   CAnimationEffectCueDocument::Load(assetName, clips, ...)
    Character.cpp:308   CEffectPresentationService::Queue_ProductCues(staged.Cues)
                          -> g_ProductPrewarmQueue.Enqueue(targets)

CMainApp::Update
  MainApp.cpp:316     CEffectPresentationService::Advance_ProductCuePreparation()
                          -> Begin_Frame()  프레임당 정확히 1개
                          -> CEffectCatalog::Find(EffectId)          JSON parse
                          -> Estimate_DocumentBudget(...)
                          -> CEffectDocumentRenderer::Prepare_VisualProgramTarget(...)  GPU
```

`Enqueue`는 등록 프레임에 `m_bYieldNextFrame = true`를 세우고 다음 프레임부터 1개씩 처리한다.
따라서 등록 프레임은 비고, 그 다음 41 프레임이 각각 문서 하나를 통째로 처리한다.

즉 정지는 **좌클릭이 아니라 Character가 생기는 순간부터** 시작된다.
사용자가 좌클릭한 시점이 그 41 프레임 구간 안에 있었던 것이다.

## 4. 데이터 실측 — 이번 진단의 핵심

`Data/Animation/Authored/<Asset>/<Asset>.animevents`의
`payload="..." effectref=asset` 행이 prewarm target 정본이다.

```text
class             cue행   고유 prewarm target
LanceMaster        41            41
Warlord            24            24
DimensionMaster    19            19
Artist             15            15
GunSlinger          0             0
Slayer              0             0
합계               99            99
```

LanceMaster 41개 target의 `Data/Effects/Authored` 문서 실물 크기다.

```text
문서 발견        41 / 41
총 크기          61,506 KB   (약 60 MB)
최대 단일 문서    6,363 KB   (약 6.2 MB)
평균             약 1.5 MB
```

**한 프레임이 평균 1.5 MB, 최악 6.2 MB의 JSON을 파싱하고 그 위에 GPU prepare까지 한다.**
1 FPS는 이 값으로 충분히 설명된다. 41 프레임이면 체감 수십 초다.

GunSlinger와 Slayer가 0인 것도 일치한다. 이 두 class로 진입하면 정지가 없어야 한다.
이 예측은 사용자가 실제로 확인할 수 있는 검증 항목이다.

## 5. 선택지

### 5.1 A안 — 스케줄 이동 (사용자가 제안한 방향)

Loader/로딩 화면 구간으로 prewarm을 옮긴다.

```text
Loader는 이미 선택 class를 안다.
  Loader.cpp:256  CCharacterSelectionState::Try_Get_SelectedClass(initialClass)
  Loader.cpp:262  CPlayableCharacterAssetService::Ensure_Prototypes(...)

CLevel_Loading::Update(main thread)에서 한 번
  선택 class의 animevents에서 effectref=asset ID 집합을 수집해 queue에 enqueue한다.
  MainApp의 Advance_ProductCuePreparation은 LOADING 프레임에서도 이미 매 프레임 돌므로
  로딩 화면 뒤에서 그대로 소진된다.
  Character spawn 시점의 Queue_ProductCues는 Enqueue가 prepared를 건너뛰므로
  0개를 큐잉하는 안전망으로 남는다 (Effect_ProductPrewarmQueue.cpp:34).

필요한 추가
  CAnimationEffectCueDocument에 clip 필터 없는 target ID 수집 진입점
  CEffectPresentationService에 ID 목록 전용 enqueue 진입점
  CLevel_Loading에 1회 호출

장점  gameplay 정지가 사라진다. 스레드 위험이 없다(전부 main thread).
단점  로딩 시간이 그만큼 길어진다. 60 MB 파싱 총량은 그대로다.
```

worker thread에서 파싱하는 변형도 가능하지만 `CEffectCatalog`의 전역 맵과
`Prepare_VisualProgramTarget`의 D3D context 때문에 동기화 설계가 따로 필요하다.
이번 범위로 권하지 않는다.

### 5.2 B안 — 문서 크기 축소

target 하나가 6.2 MB인 것 자체가 비정상이다. 한 스킬의 시각 효과 문서가 6 MB일 이유가 없다.
A안은 이 비용을 로딩 화면 뒤로 숨기고, B안은 비용 자체를 없앤다.

```text
확인해야 할 것
  6.2 MB 문서가 무엇이고 어떤 element가 그 크기를 만드는가
  4직업 복원 때 들어간 source occurrence 원본 값이 그대로 실려 있는지
  runtime이 실제로 소비하지 않는 저작/진단 필드가 포함돼 있는지

이미 관련 선례가 있다
  be5a4059 perf(effect): prune invisible authored elements
  .md/GB/08-16/2026-08-16_EFFECT_MANUAL_AUTHORING_PRUNE_RESULT.md
```

### 5.3 권고

A안과 B안은 배타적이지 않다. 다만 순서는 **B안 조사 먼저**를 권한다.
A안만 하면 로딩이 수십 초 길어진 채로 60 MB가 남고, 나중에 발탄 31 패턴을 붙이면
같은 문제가 더 큰 규모로 돌아온다. B안으로 문서가 절반이 되면 A안의 비용도 절반이 된다.

B안 조사는 파일 크기 분해 한 번이면 끝나므로 A안 구현보다 짧다.

## 6. 사용자가 확인할 수 있는 예측

이 진단이 맞으면 다음이 성립한다. 코드 변경 없이 지금 확인 가능하다.

```text
1. 좌클릭을 하지 않고 Character Select에 가만히 있어도 진입 직후 수십 초간 FPS가 떨어진다.
2. 그 구간이 끝난 뒤에는 좌클릭을 해도 멈추지 않는다.
3. GunSlinger 또는 Slayer로 진입하면 target이 0이라 정지 구간이 없다.
4. 같은 class로 재진입하면 prepared가 유지되므로 두 번째부터는 정지가 없다.
```

3번이 특히 결정적이다. 좌클릭이 원인이라면 class와 무관해야 한다.

## 7. 이번 회차에 하지 않은 것

```text
코드 변경 없음. 빌드 없음.
런타임 계측 없음 — 진단은 정적 실측(코드 경로 + 문서 크기)으로만 했다.
프레임당 parse/GPU 비중 분해는 하지 않았다. A안과 B안 선택에는 필요하지 않다.
```
