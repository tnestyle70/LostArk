# 쿠크 첫 진입 이펙트 준비와 Deploy 로딩 경계 구현 계획

기준일: 2026-09-11. 기능 브랜치 `codex/kouku-full-material-lighting-restoration`의 기존 미커밋 변경을 보존한다.
이 문서는 사용자가 승인한 쿠크 첫 입장 뒤 약 10초 프레임 드랍 조사와 최소 수정을 다룬다.

## G00. 현재 실측과 수정 목표

현재 코드는 서로 다른 두 비용을 메인 스레드에서 수행한다.

| 구간 | 실제 호출 | 현재 결함 | 수정 목표 |
|---|---|---|---|
| 로딩 종료와 첫 렌더 사이 | `CMainApp::Apply_LevelRequest → Create_Level → CLevel_KakulSaydonArena::Initialize → CDeployPropRuntime::Ensure_AreaPrototypes` | Loader가 빠뜨린 Deploy 모델·재질을 메인 스레드에서 생성 | 기존 Loader worker의 Area admission으로 옮기고 Level은 준비된 Prototype을 Clone |
| 실제 입장 뒤 여러 프레임 | `CCharacter::Load_EffectCues → Queue_ProductCues → MainApp::Advance_ProductCuePreparation` | 쿠크는 Loading Effect job을 만들지 않아 선택 class의 cue 문서·renderer·GPU 준비가 입장 뒤 메인 프레임에 남음 | 기존 Character Select/Valtan의 worker stage와 bounded main commit에 쿠크의 선택 class를 연결 |

`Advance_ProductCuePreparation`의 한 프레임 한 target 정책은 target 내부 비용을 시간 예산으로 쪼개지 않는다.
그 내부는 `CEffectCatalog::Find`의 동기 문서 읽기와 `Prepare_ProductTarget`이다. 첫 입장만 무겁고 이후
prepared cache를 재사용하는 현재 경로는 사용자의 증상과 부합한다. 다만 현재 보관된 최신 profiler는
Lobby/Loading과 Bern 프레임이고, 쿠크 입장 직후 10초의 CPU/GPU 구간을 직접 기록한 자료는 아니다.
약 10초 전체를 단일 원인으로 확정하거나 FPS 개선값을 계획 단계에서 만들지 않는다.

관련 선행 구현은 `08-27/2026-08-27_RELEASE_EFFECT_LOADING_RESPONSIVENESS_IMPLEMENTATION_RESULT.md`다.
당시 worker 경로의 제품 범위가 Character Select/Valtan임을 명시했고, 현재 코드에도 그 범위가 그대로 남아 있다.

## G01. Level_Loading.cpp — 선택 class 이펙트를 입장 전에 준비

수정 파일은 `Client/Private/Level_Loading.cpp`다. 새 H 계약, 파일, runtime, thread를 만들지 않는다.

1. `Initialize`의 `bUsesEffectLoadJob`에 `KAKULSAYDON_ARENA`를 포함한다. 기존 catalog revision,
   epoch, Loader job을 그대로 사용한다.
2. `Update`의 target preparation gate와 Debug progress 표시에도 같은 Level을 포함한다.
3. `Advance_TargetEffectPreparation`은 쿠크를 유효 대상으로 처리하고 상태에 `KOUKUSAYDON ARENA`를 표시한다.
4. 기존 선택 class의 `Load_ForProductPrewarm → Queue_ProductCues_Priority → Begin_LoadingProductCuePreparation`
   경로를 재사용한다. Valtan 전용 cue와 map effect를 쿠크에 섞지 않는다.
5. current revision의 선택 target이 prepared 또는 isolated terminal이 되고 Loader job이 끝난 뒤 activation한다.
   registration 실패, worker cancellation, stale revision 거부, bounded join, rollback 계약은 유지한다.

이 수정은 선택 플레이어 class의 V1 cue를 준비한다. 아직 재생되지 않은 쿠크 Composition V1/V2 전체나
이후 다른 class의 첫 등장까지 무조건 미리 읽는 기능은 아니다.

## G02. Loader.cpp와 Level_KakulSaydonArena.cpp — Deploy 모델 admission을 worker로 이동

`Client/Private/Loader.cpp`의 `Ready_For_KakulSaydonArena`에서 기존 map, 선택 class, Server boss 준비 뒤
`Ready_DeployPropArea(KAKULSAYDON_ARENA, pEntry->pMapAreaId)`를 호출한다. 실패는 기존 Level rollback scope에
포함하고 취소는 기존 helper가 감지한다. 완료 상태와 `rollback.Commit()`은 Deploy 준비 뒤에 둔다.

`Client/Private/Level_KakulSaydonArena.cpp`의 `Initialize`에서는 기존 `Ensure_AreaPrototypes` 블록을 제거한다.
이 함수는 실제로 매번 `CModel::Create`와 `Add_Prototype`을 호출하므로 중복 호출을 남기면 Prototype 중복
거절로 진입이 실패한다. `m_DeployRuntime.Load_Area`의 Clone와 실패 보고는 그대로 둔다.

runtime Deploy catalog의 4개 animated 모델(레버, 종이 무대, `MN_RPCT_00`, 팝업북)을 같은
`CModel → CMaterial` 경로로 로드한다. source character material overrides는 현재
`CActorCatalog::Build_ModelLoadDescription`을 통해 그대로 적용된다.

## G03. 검증과 종료 조건

기존 `Tools/KoukuSaydonPipeline/test_kouku_saydon_client_product_level_contract.py`의 쿠크 Loader 계약은
`Ready_DeployPropArea`를 금지한다. 지금의 실제 Deploy 소비자와 새 worker 소유권에 맞게 그 검사를
교정하고, Loader에 admission이 있으며 Level에서 중복하지 않는지 확인한다. 별도 하네스는 만들지 않는다.

현재 파일 인코딩·CRLF를 유지한다. 변경 파일 diff와 기존 사용자 diff가 섞이지 않았는지 확인한다.
기존 Python 계약 테스트, JSON/XML parse, `git diff --check`를 실행하고 결과를 RESULT에 기록한다.
최소 C++ compile 대상은 `Level_Loading.cpp`, `Loader.cpp`, `Level_KakulSaydonArena.cpp`이며 root가
전체 작업의 빌드를 단일 조율한다. 이 작업에서 publisher를 실행할 데이터 변경은 없다.

Client/UI를 실행·조작하거나 화면을 캡처하지 않는다. root의 빌드 뒤 사용자가 Server + Client profile로
Lobby → KoukuSaydon 첫 입장과 재입장을 직접 비교한다. 수동 FPS/응답성 확인은 자동 검증과 분리한다.
