# 쿠크 첫 진입 이펙트 준비와 Deploy 로딩 경계 구현 결과

기준일: 2026-09-11. 대응 계획은 `2026-09-11_KOUKU_ENTRY_STUTTER_IMPLEMENTATION_PLAN.md`다.

## G00. 완료 범위와 판단

쿠크 입장 후 메인 프레임에서 선택 캐릭터 이펙트를 처음 준비하던 누락과, Level activation에서
컷씬용 Deploy 모델을 동기 생성하던 누락을 수정했다. 기존 Loader/Effect worker를 연결했으며 새
runtime·thread·파일 형식은 추가하지 않았다.

사용자가 말한 약 10초 프레임 드랍과 부합하는 두 실행 경로를 코드와 실제 입력 크기로 확인했다.
쿠크 첫 입장 직후 CPU/GPU 프레임 캡처는 현재 증거에 없으므로 '10초 드랍 해결'이나 FPS 개선을
실측 완료했다고 판정하지 않는다. Client/UI 실행·조작·캡처는 수행하지 않았다.

## G01. 실제 원인 경로와 변경

| 구간 | 수정 전 | 수정 후 |
|---|---|---|
| 입장 후 여러 프레임의 Effect 준비 | `CCharacter::Load_EffectCues`가 선택 class cue를 등록한 뒤 `MainApp::Advance_ProductCuePreparation`이 메인에서 target 하나씩 동기 읽기·준비 | `Level_Loading`이 쿠크 선택 class cue를 먼저 등록하고 Loader worker stage, main commit과 terminal readiness가 끝나야 activation |
| Loading 종료와 첫 arena Render 사이 | `Level_KakulSaydonArena::Initialize`가 4개 Deploy 모델과 재질을 main에서 admission | `Loader::Ready_For_KakulSaydonArena`의 rollback scope 안에서 미리 admission, Level에서는 기존 `m_DeployRuntime.Load_Area` Clone |

`Client/Private/Level_Loading.cpp:130`의 job 생성 대상, `:179`의 activation 준비 gate, `:532`의 Debug
진행 표시, `:569`의 target preparation 대상에 `KAKULSAYDON_ARENA`를 포함했다. 선택 class의
`Load_ForProductPrewarm`은 `:634`, 실제 job 등록은 `:760`, main 준비 결과 소비는 `:254`다.
Valtan 전용 boss/map Effect는 기존 `bValtanArena` 분기 안에 남는다.

`Client/Private/Loader.cpp:805`에서 `Ready_DeployPropArea`를 호출하고 성공한 뒤에만 완료 상태와
`rollback.Commit()`에 도달한다. 기존 helper `:1161`는 취소를 callback으로 전달하고 실패를 보고한다.
`Client/Private/Level_KakulSaydonArena.cpp:801`에서는 중복 admission 블록을 제거하고 `:804`의 Clone를
유지했다. `Ensure_AreaPrototypes`는 idempotent 함수가 아니라 매번 `CModel::Create/Add_Prototype`을
수행하므로 양쪽에 호출을 남기면 중복 Prototype 거절로 진입이 실패한다.

보존된 실제 소비자와 불변식:

- `Character.cpp:330`의 cue document, catalog target ID, runtime playback 경로는 변경하지 않았다.
- `Effect_PresentationService.cpp:2798`의 exact epoch/catalog revision, queue ownership, worker ACK,
  stale result 거부, isolated terminal 처리와 bounded join을 재사용한다.
- `Effect_PresentationService.cpp:3197`의 일반 incremental 경로도 유지한다. 같은 revision에 이미
  준비한 선택 class target은 `CEffectProductPrewarmQueue::Enqueue`가 다시 pending에 넣지 않는다.
- Deploy material은 현재 `CActorCatalog::Build_ModelLoadDescription → CModel → CMaterial` 그대로다.
  재질 입력, animation, placement, cutscene visibility, Server authority를 변경하지 않았다.

## G02. 실제 입력과 CPU 측정

`Data/Animation/Authored/<Class>/<Class>.animevents`의 명시적인 `effectref=asset` row를 stable Effect ID로
중복 제거하고 `Data/Effects/EffectCatalog.json`과 exact join했다. 다음 4개 복원 class의 ID는 모두
catalog에 존재한다. 이 표는 해당 명시적 asset cue 집합이며 전체 게임 class/원본 notify 목록이 아니다.

| class | unique target | authoring JSON bytes | Python read+JSON parse 3회(ms) |
|---|---:|---:|---|
| Artist | 18 | 23,679,475 | 223.269 / 214.484 / 205.846 |
| DimensionMaster | 15 | 29,396,495 | 299.115 / 305.937 / 257.064 |
| LanceMaster | 46 | 64,855,350 | 574.456 / 552.556 / 580.372 |
| Warlord | 26 | 36,655,287 | 369.412 / 318.585 / 325.492 |

CPU 측정은 현재 PC의 Python `time.perf_counter`로 실제 파일을 `read_bytes → json.loads`한 합계다.
따뜻한 OS file cache일 수 있고 제품 C++ validation, renderer resource 생성, D3D, texture decode, FPS는
포함하지 않는다. 이 숫자를 실제 입장 후 프레임 시간이나 10초의 증명으로 바꾸어 해석하지 않는다.

runtime `Client/Bin/DataFiles/Map/LV_LUT_MIDNIGHTC_ED.deployassets`의 4개 wmodel은 다음과 같다.

| Deploy | wmodel bytes |
|---|---:|
| `DEPLOY_ITR_02283` | 318,144 |
| `DEPLOY_BG_RAD_KOUKUSATON_PAPERSTAGE` | 75,672 |
| `DEPLOY_BOSS_MN_RPCT_00` | 189,894,768 |
| `DEPLOY_CINE_KOUKU_BOOK` | 471,524 |
| 합계 | 190,760,108 |

이 bytes는 파일 크기이며 GPU VRAM이나 로드 시간 추정치가 아니다. 큰 세이튼 모델을 포함한 admission이
activation main에서 실제 호출되던 사실과, 동일 호출을 worker로 옮겼다는 것을 함께 확인했다.

구조화 증거는 Git 제외 `out/KoukuEntryStutter20260911/resource-survey.json`에 있다.
ID 목록, 입력 bytes와 3회 측정치를 보존했다. 반복 방법은 동일한 animevents에서
`payload="([^"]+)"[^\n]*effectref=asset`를 추출하고 catalog `authoringPath`를 `Data`에 resolve한 뒤,
class별 전체 문서의 `read_bytes → json.loads`를 3회 측정하는 것이다.

## G03. 다른 구간과 미확정 경계

`Client/Bin/ProfilerCaptures/profiler_20260911_044109_437_frame11498.json`의 1,200프레임은
mapPlacements=0인 1,119프레임과 Bern 50,017개인 81프레임이다. 쿠크 입장 직후 10초의 직접 증거로
사용하지 않았다. `client-session-*.jsonl`은 접속/Server 승인·단절 진단이므로 그 시간 차이를 렌더
프레임 시간으로 사용하지 않았다.

첫 activation에는 여전히 map placement Clone, worldsequences/maplight/self-motion 문서 검증, UI와
replication 준비가 있다. 첫 Kouku presentation update는 `Reload_Product`로 patternbindings를 읽는다.
이는 상시 매 프레임 재읽기와 구분하며 이번 두 병목 수정의 효과만큼 해결됐다고 주장하지 않는다.

`MainApp::Apply_LevelRequest`는 Update 끝에서 Level을 생성하고 같은 frame에서 Render할 수 있다.
`Timer_60`의 다음 delta에는 긴 activation 시간이 들어갈 수 있다. delta를 clamp해서 비용을 숨기는
변경은 하지 않았다. 원래 cutscene/action timeline의 clock 정책도 변경하지 않았다.

쿠크 Composition에서 나중에 처음 쓰는 V1/V2, 다른 class player의 첫 등장, driver 최초 draw 비용은
이번 선택 class prewarm의 보장 범위 밖이다. 새 두 패턴의 source Effect 복원은 별도 구현 결과가 소유한다.

## G04. 검증 상태

| 검증 | 실제 결과 |
|---|---|
| 변경 3개 C++ UTF-8(BOM 없음), CRLF 보존 | PASS |
| 기존 dirty baseline 대비 이 작업 diff | `out/KoukuEntryStutter20260911/implementation.diff`, 3 C++와 기존 test만 변경 |
| Client vcxproj/filter XML parse 및 각 TU 1회 등록 | PASS, project/filter 변경 없음 |
| 수정된 쿠크 Loader admission source 계약 | PASS |
| 전체 해당 쿠크 contract module | 18개 중 15 PASS, 수정 전 dirty baseline과 동일한 기존 3개 실패 |
| Loading Release product UI 경계 test | PASS, 1개 |
| `git diff --check` 변경 파일 | PASS |
| C++ compile/link/deploy | 최종 Debug Product 통합 빌드 PASS. `out/BuildPipeline/runs/20260910T215447751Z-debug-product.json`의 Engine/Shared/Server/Client 모두 PASS, missingRuntimeInputs 없음 |
| Client/UI 실행·FPS·시각 검증 | 미실행, 사용자 직접 확인 |

쿠크 module의 기존 실패는 `Render_Details`의 제거된 `requestedAuthoringStatus` 문자열,
Workbench의 이전 `Publish All PRODUCT` UI label, 이전 `Consume_PreviewTransportRequest` 호출명이다.
baseline과 현재를 동일한 나머지 소스 입력으로 실행해 실패 ID가 같은 것을 확인했다.
`out/KoukuEntryStutter20260911/focused-contract-results.json`에 결과를 보존했다.
이 작업과 관계없는 UI 테스트를 완화하거나 이전 UI를 되살리지 않았다.

전체 Release UI module은 4개 중 Loading 포함 3개 PASS, Character Select의 이전
`m_hasCreateCharacterButtonClick` 문자열 assertion 1개 실패였다. 영향받는 Loading test를 별도 실행해 PASS를 확인했다.

재현 가능한 테스트 명령:

```powershell
python -m unittest Tools.KoukuSaydonPipeline.test_kouku_saydon_client_product_level_contract
python -m unittest Tools.Build.test_release_client_surface_contract.ReleaseClientSurfaceContractTests.test_loading_release_has_product_recovery_without_visible_imgui_windows
git diff --check -- Client/Private/Level_Loading.cpp Client/Private/Loader.cpp Client/Private/Level_KakulSaydonArena.cpp Tools/KoukuSaydonPipeline/test_kouku_saydon_client_product_level_contract.py
```

## G05. 사용자 실행 준비

이 PC의 LAN 설정은 server-host, `192.168.0.14:7777`, TCP 7777 LocalSubnet 규칙 확인 완료다.
세션 시작 당시 endpoint는 not-listening이었다. 최종 실행 파일과 Server 상태는 root의 통합 빌드 결과를 따른다.

사용자가 빌드 후 Server + Client profile로 시작하고 Lobby → KoukuSaydon에 처음 입장한다.
Loading 중 Effect 준비가 끝나는지, 입장 뒤 첫 10초와 이후 구간의 입력·프레임이 어떤지 비교한다.
숫자가 필요하면 사용자가 F1 → Profiler에서 `Capture`를 켠 뒤 입장하고 `Save JSON`을 누른다.
프로파일러의 `Loader.LevelLoad`, `Loader.EffectPreparation`, `Effect.Prewarm.Advance`와
main `Client.Update/Client.Render`를 구분해 후속 판정한다. 에이전트가 캡처나 시각 PASS를 대신하지 않는다.
