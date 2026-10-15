# 쿠크 전체 흐름 검증과 현재 입력 기반 테스트 보정 결과

이 문서는 [대응 PLAN](C:/Users/tnest/Desktop/LostArk/.md/GB/10-14/2026-10-14_KOUKU_FULL_FLOW_VERIFICATION_PLAN.md)의 실행 근거를 기록한다. 초기 저작·게시 데이터 및 직접 리소스 감사, Python 검사 10개와 문서 정정, 이후의 native fixture 보정과 최종 Product 빌드·Server 실행 근거를 구분한다. 실제 Client 화면 확인과 미실행 검사는 아래 갱신 구간에서 별도로 판정한다.

## G00. 최신 완료 상태를 갱신하는 구간

<!-- KOUKU_FINAL_VERIFICATION_BEGIN -->

최종 검증 상태: **23:07까지의 빌드 PASS, 이후 사용자 추가 파일을 포함한 최신 전체 트리는 미검증**. Release·Debug Product는 각 실행 당시 소스 안정성까지 PASS였다. 이후 사용자 작업 파일 두 개의 추가·내용 저장·프로젝트 등록이 확인됐다. 컴파일 입력도 달라졌으므로 새 안정 Product 빌드 전에는 최신 전체 트리 PASS로 표시하지 않는다. 아래 native 검사는 저장된 EXE의 Server 계약 시뮬레이션이며 실제 Client 플레이 완료로 바꾸어 기록하지 않는다.

| 완료 조건 | 현재 판정 | 근거와 경계 |
|---|---|---|
| Release 빌드 | **기록된 입력 PASS / 최신 트리 미검증** | `20261014T140648402Z-release-product.json` 실행 당시 Engine·Shared·Server·Client 및 sourceStable PASS; 이후 사용자 추가 파일은 G11 참조 |
| Debug 빌드 | **기록된 입력 PASS / 최신 트리 미검증** | `20261014T140709351Z-debug-product.json` 실행 당시 Engine·Shared·Server·Client 및 sourceStable PASS; 이후 사용자 추가 파일은 G11 참조 |
| 최종 Server의 집중 native 검사 | 부분 완료; 대기: product, bundle, cardmaze, bingo, showtime, overlap, support | G08의 실제 종료 결과와 EXE·데이터 무변경 검사로 판정 |
| Debug bundle 검사 본문 | 미완료 | Release bootstrap 2줄을 `_DEBUG` 본문 통과로 세지 않는다. |
| Release·Debug compiled shader closure | **두 구성 PASS** | Product shader closure와 별도 WARP shader 생성 검사; 실제 아레나 화면은 아님 |
| 실제 Server·Client 연결 및 쿠크 입장 로드 | 미확인 | 이 감사에서 Client·UI·listener를 실행하지 않음 |
| 입장 연출부터 G1·G2·G3·미니게임·빙고·종료까지 실제 표시 | 미확인 | 사용자 실제 화면의 효과·소리·카메라·HUD·조작 확인이 남음 |
| 전체 도구·로드 검토 | 부분 확인 | G09 parser 최종 재실행 미완료. G10 P65 게시 투영 결함은 재현했으며 제품 수정은 미반영 |

빌드 receipt와 source-stability 및 Server SHA는 [최종 입력 증거](C:/Users/tnest/Desktop/LostArk/out/KoukuArenaReview20261014/verified-final-native-inputs.json)에 연결했다. 실행 중이거나 메모리 때문에 시작하지 못한 검사는 PASS/FAIL로 확정하지 않는다.

<!-- KOUKU_FINAL_VERIFICATION_END -->

## G01. 현재 저작 데이터와 게시 참조 대조

Action revision **2500**, Sequence revision **183**을 기준으로 source Flow와 게시 Encounter의 `raidGates` 투영이 일치했다. Flow의 대상 패턴뿐 아니라 후속 패턴을 따라가 Server stage의 action ID와 Client binding, 대상 boss ID의 연결을 대조했으며 이 검사 범위에서 누락은 0건이었다.

| 구간 | Flow 항목 | 그룹 | 후속 의존을 포함한 패턴 | 입장/전환 Sequence 길이 |
|---|---:|---:|---:|---:|
| G1 | 166 | 13 | 20 | 61,662 ms |
| G2 | 85 | 13 | 24 | 27,000 ms |
| G3 | 72 | 11 | 23 | 35,368 ms |
| Bingo | 35 | 0 | 14 | Encore 21,322 ms |

각 관문의 의존 수는 해당 관문 기준이며 중복 없는 전체 패턴 수로 합산하지 않는다. G1·G2는 실제 publisher의 의존 추적을 사용했고, G3·Bingo는 중첩된 후속 `patternId`와 `bingoSpecialPatternId`도 포함했다. 패턴 참조와 stage/binding의 일치가 실제 화면의 효과·음향·피격 시각 성공을 증명하지는 않는다.

근거: [G1·G2 감사](C:/Users/tnest/Desktop/LostArk/out/KoukuArenaReview20261014/gates12/current-data-audit.json), [G3·Bingo 감사](C:/Users/tnest/Desktop/LostArk/out/KoukuArenaReview20261014/gate3/current-data-audit.json), [G3 lifecycle 감사](C:/Users/tnest/Desktop/LostArk/out/KoukuArenaReview20261014/gate3/gate3-lifecycle-audit.json). 감사 스크립트는 [G1·G2](C:/Users/tnest/Desktop/LostArk/out/KoukuArenaReview20261014/gates12/audit_current.py)와 [G3·Bingo](C:/Users/tnest/Desktop/LostArk/out/KoukuArenaReview20261014/gate3/audit_current.py)에 보관했다.

G1·G2 감사 파일은 2026-10-14 21:29:17 KST에 작성했고, 21:37:34 KST에 아래 네 입력의 SHA256이 그대로임을 재확인했다. 정적 C++ 검토에는 고정 SHA가 없으므로 이 데이터 감사 결과를 동시에 변경되던 C++ 소스의 최종 빌드 증거로 사용하지 않는다.

| 입력 | SHA256 |
|---|---|
| `Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json` | `427575c883f3b56969b71c83cfc6f03e4dace0733faa8c6d034609b3ec2ef70d` |
| `Data/Encounters/KoukuSaydon/KoukuSaydonEncounter.json` | `b856d7a05547960c0ff2c3712f6f140ea8edf87b8cfe526d15306d8a80dbbe72` |
| `Data/Animation/Authored/KoukuSaydon/KoukuSaydon.patternbindings.json` | `7fb9ed6888043d3f7e811f4a0f6b58c15a0f7667852530358e0e1e9b34ccba0c` |
| `Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json` | `0c61ffd08c468a236403bfc22994e7d21c50430cc0d91d4363e0dc5968f9e1e0` |

재확인 시각과 입력 SHA는 [설치 검증 기록](C:/Users/tnest/Desktop/LostArk/out/KoukuArenaReview20261014/test-maintenance/verification.json)에 함께 기록했다.

## G02. G1·G2 직접 리소스 참조

G1·G2 Flow의 의존 패턴과 두 입장 Sequence에서 참조하는 V1/V2 이펙트 JSON 및 Sound event를 따라 직접 파일의 존재를 확인했다. V1 asset 73개, V2 asset 55개, JSON 문서 128개, Sound event 215개가 검사 대상이며 직접 리소스 파일 **910개에서 누락 0건**이었다.

이 910개 검사는 WModel 내부 material/texture, 전체 WORLD Sequence의 모든 의존 파일, native parser의 실제 승인, GPU 생성·표시를 포함하지 않는다. 따라서 전체 쿠크 리소스나 모든 도구의 로드 성공으로 판정하지 않는다.

근거: [직접 리소스 감사](C:/Users/tnest/Desktop/LostArk/out/KoukuArenaReview20261014/gates12/direct-resource-audit.json), [감사 코드](C:/Users/tnest/Desktop/LostArk/out/KoukuArenaReview20261014/gates12/audit_direct_resources.py).

## G03. 실제 반영한 테스트와 문서 수정

이 G03은 아래 Python 검사 두 파일과 준비 제한시간 설명을 먼저 보정한 당시 범위다. 이 단계에서는 제품 C++·저작 수치·게시 데이터·프로젝트 등록·렌더링 옵션을 변경하지 않았다. 이후 추가한 native 검사 C++ 두 파일과 shader 검사 도구의 보정은 G06~G08에 별도로 기록한다. 이번 쿠크 검증에서 gameplay runtime·저작 데이터·게시 데이터·렌더링 옵션은 변경하지 않았다.

| 파일 | 확인한 원인과 수정 | 설치 후 결과 |
|---|---|---|
| [test_dice_card_contract.py](C:/Users/tnest/Desktop/LostArk/Tools/KoukuSaydonPipeline/test_dice_card_contract.py) | 기존 검사가 현재 사용하지 않는 logic 73을 수정하고 한 행에 네 문양을 기대했다. 활성 P78 occurrence가 참조하는 logic 125~128 네 행을 따라가 문양·시간·피해·출력 순서를 확인한다. | 4개 PASS |
| [test_kouku_saydon_runtime_inputs.py](C:/Users/tnest/Desktop/LostArk/Tools/KoukuSaydonPipeline/test_kouku_saydon_runtime_inputs.py) | 이동한 publisher 함수 위치와 Parent validator 의존, 현재 장비·배치·world 입력에 맞춰 검사를 연결했다. | 6개 PASS |
| [CLAUDE.md](C:/Users/tnest/Desktop/LostArk/CLAUDE.md:1027) | 기존 PREPARING 설명의 `10초 timeout` 한 곳을 실제 `preparationTicks = 20u * 60u * 30u`에 맞는 `20분 timeout`으로 정정했다. | 현재 코드와 대조, 동작 변경 없음 |

Dice 검사는 각 행의 피해를 메모리에서 50·51·52·53으로 다르게 바꿔 실제 입력 소비를 구분한다. 잘못된 문양·개수의 8가지 입력이 기존 projection에서 거절되는지, 문양 생략 시 일반 pursuit 호환이 유지되는지, 각 occurrence의 parent/card/hit bootstrap 행과 canonical 네 파일의 바이트가 보존되는지도 검사한다. 제품 validator를 완화하지 않았다.

Runtime inputs 검사는 현재 `KoukuBootstrapRows.ps1`의 source-action 함수와 실제 `Assert-KoukuParentPatternSequence`를 가져온다. 설치된 Albion visual, G1 Saydon staff, disabled G1 boss template과 위치를 검사하며 world revision은 과거 고정 숫자 대신 양수 정수 계약을 검사한다.

준비 제한시간은 [GameRoom_KoukuRaidFlow.cpp](C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom_KoukuRaidFlow.cpp:205), [09-26 준비 진단 RESULT](C:/Users/tnest/Desktop/LostArk/.md/GB/09-26/2026-09-26_KOUKU_RAID_PREPARATION_DIAGNOSTICS_RESULT.md), [09-29 Release Sequence 준비 RESULT](C:/Users/tnest/Desktop/LostArk/.md/GB/09-29/2026-09-29_KOUKU_RELEASE_SEQUENCE_READY_RESULT.md)를 대조했다. 다른 Lobby·Loader·요청 timeout은 변경하지 않았다.

설치 직전 SHA를 대조하고 원본을 out에 보관한 뒤 원자적 파일 교체를 수행했다. 설치 후 집중 검사는 **10개 PASS / 실패 0 / 오류 0 / skip 0 / 33.845초 / exit 0**이었다. 이전 감사에서 기록한 43개·46개 묶음에는 같은 검사가 중복되므로 여기의 10개와 합산하지 않는다.

```powershell
$env:PYTHONIOENCODING='utf-8'
$env:PATH='C:\Users\tnest\.cache\codex-runtimes\codex-primary-runtime\dependencies\python;' + $env:PATH
$env:PYTHONPATH=(Get-Location).Path + ';' + (Join-Path (Get-Location).Path 'Tools/KoukuSaydonPipeline')
python -m unittest test_dice_card_contract test_kouku_saydon_runtime_inputs -v
```

Python parse, 후보 코드와 설치 코드의 일치, 해당 변경의 `git diff --check`도 통과했다. CLAUDE의 기존 혼합 줄바꿈에 대한 Git 안내만 있었고 diff 오류는 없었다. Dice 후보는 별도 에이전트가 정적 검토했으며 추가 실행 수로 세지 않는다.

| 설치 파일 | 검증한 SHA256 |
|---|---|
| `test_dice_card_contract.py` | `5ea111773dbd9e7aaa8b2524e525095ba765e8f421d6ee58a9e0500ad2acc21b` |
| `test_kouku_saydon_runtime_inputs.py` | `6eb302e7e99c061866e2db49c6dafa60a5ec5626aa724995bfd1acb400b1f43c` |
| `CLAUDE.md` | `f122f97e7551cf9448faf31b8a88f2602b87519cb10907f37dc38c1280c0658b` |

근거: [적용 기록](C:/Users/tnest/Desktop/LostArk/out/KoukuArenaReview20261014/test-maintenance/applied.json), [설치 테스트 로그](C:/Users/tnest/Desktop/LostArk/out/KoukuArenaReview20261014/test-maintenance/installed-tests.log), [검증 요약](C:/Users/tnest/Desktop/LostArk/out/KoukuArenaReview20261014/test-maintenance/verification.json), [diff 검사](C:/Users/tnest/Desktop/LostArk/out/KoukuArenaReview20261014/test-maintenance/diff-check.log). 해당 검증 요약의 PLAN SHA는 이 RESULT 연결과 G05 추가 전 버전의 SHA다.

## G04. 갱신 전 Release Server 집중 검사

`release-current-001`은 2026-10-14 21:27:27~21:29:24 KST에 격리한 EXE와 DataFiles로 실행한 기존 no-listener 계약 검사다. 이 실행의 Server SHA256은 **`572a6e3b1a9d62d60e8d42302291438b2119009ced6e931917c51726f4908947`**이며, 최종 소스 변경을 다시 반영하기 전 EXE의 증거다. 현재 새 Server 빌드와 같은 것으로 취급하지 않는다.

| 인자 | 실제 출력 결과 | 확인 범위 |
|---|---|---|
| `--kouku-raid-contract-test` | PASS 2,066줄, failures 0, exit 0 | CGameRoom의 관문 상태 전환, 준비·roster, G1/G2/G3/Encore/Bingo 종료 관련 고정 입력 |
| `--kouku-product-contract-test` | PASS 96줄, failures 0, exit 0 | 게시 catalog·product/draft 승인·logic·Mario 진입 접촉 고정 입력 |
| `--card-maze-contract-test` | PASS 201줄, failures 0, exit 0 | CardMaze 상태·역할·수집·복귀 고정 입력 |
| `--bingo-contract-test` | PASS 51줄, failures 0, exit 0 | Bingo board·hammer·bomb·hook 관련 고정 입력 |
| `--kouku-bundle-contract-test` | bootstrap PASS 2줄, failures 0, exit 0 | **bundle 본문 미실행**. Release 결과를 bundle 완료로 세지 않음 |

위 수치는 로그의 PASS assertion 줄 수다. 실제 전투 패턴 실행 횟수나 서로 독립인 전체 시나리오 수로 합산하지 않는다. 특히 `Run_KoukuBundles` 본문은 [ServerGameplayContractTests_KoukuBundles.cpp](C:/Users/tnest/Desktop/LostArk/Server/Private/ServerGameplayContractTests_KoukuBundles.cpp:42)의 `_DEBUG` 조건 아래 있으므로 Release summary의 `passed: true`만 보고 본문을 통과했다고 해석하지 않는다.

이 검사에는 내부 시계 이동·직접 보스 처치 등 계약 검사용 입력이 있으며 Client GPU·화면·음향·HUD·카메라와 사람이 처음부터 끝까지 수행한 플레이를 검증하지 않는다. 실행 snapshot의 데이터·EXE 불변 검사는 통과했지만, 이후 소스 변경의 현재 EXE 일치 검사를 대신하지 않는다.

근거: [실행 요약과 snapshot SHA](C:/Users/tnest/Desktop/LostArk/out/KoukuArenaReview20261014/runs/release-current-001/summary.json), [raid 로그](C:/Users/tnest/Desktop/LostArk/out/KoukuArenaReview20261014/runs/release-current-001/raid/stdout.log), [product 로그](C:/Users/tnest/Desktop/LostArk/out/KoukuArenaReview20261014/runs/release-current-001/product/stdout.log), [cardmaze 로그](C:/Users/tnest/Desktop/LostArk/out/KoukuArenaReview20261014/runs/release-current-001/cardmaze/stdout.log), [bingo 로그](C:/Users/tnest/Desktop/LostArk/out/KoukuArenaReview20261014/runs/release-current-001/bingo/stdout.log), [bundle 로그](C:/Users/tnest/Desktop/LostArk/out/KoukuArenaReview20261014/runs/release-current-001/bundle/stdout.log).

## G05. 현재 판정의 의미

G01~G04의 초기 단계에서 확인한 문제는 현재 저작 입력과 맞지 않던 Python 테스트 두 파일 및 준비 제한시간 설명이었다. 이후 실제 native 실행에서 기존 피해 변동·무력화 입력 정책과 어긋난 검사 fixture를 확인해 C++ 검사 두 파일도 보정했다. 이는 gameplay runtime의 피해·무력화 규칙을 변경한 것이 아니다. 데이터 참조 연결, 검사 입력 통과와 실제 아레나의 모든 패턴·표시가 끝까지 정상 동작한다는 판정은 별도다.

최종 완료는 G00 marker 구간에서 빌드, 최신 Server, 실제 Client 및 도구 확인의 증거가 채워졌을 때만 판단한다. 미확인 범위는 남겨 두고, 추가 결함이 발견되면 대응 PLAN을 먼저 갱신한 뒤 정확한 변경과 재검사 결과를 이 문서에 기록한다.

<!-- KOUKU_NATIVE_EVIDENCE_BEGIN -->

## G06. 빌드 검증 도구와 최종 Product 빌드

`Tools/Build/Test-CompiledShaderClosure.ps1`의 Visual Studio toolchain 검색을 현재 설치된 VS 18 Insiders에서도 동작하도록 보정했다. 이 변경은 검사 도구이며 shader 내용이나 gameplay runtime을 변경하지 않는다. 적용 근거는 [shader 도구 적용 기록](C:/Users/tnest/Desktop/LostArk/out/KoukuArenaReview20261014/shader-closure-toolchain/application.json)에 남겼다.

저장된 Product 빌드 검사는 [Release receipt](C:/Users/tnest/Desktop/LostArk/out/BuildPipeline/runs/20261014T140648402Z-release-product.json)와 [Debug receipt](C:/Users/tnest/Desktop/LostArk/out/BuildPipeline/runs/20261014T140709351Z-debug-product.json) 모두 PASS다. 각 구성의 Engine·Shared·Server·Client를 검증했고, [Release 소스 안정성](C:/Users/tnest/Desktop/LostArk/out/BernGeometryProxy20261014/product-integration-applied/verified-release/source-stability.json)과 [Debug 소스 안정성](C:/Users/tnest/Desktop/LostArk/out/BernGeometryProxy20261014/product-integration-applied/verified-debug/source-stability.json)은 모두 `buildExit: 0`, `sourceStable: true`다. 앞선 전체 컴파일 뒤 두 native fixture 수정까지 반영한 최종 증분 Product 검사이며, 모든 파일을 다시 컴파일했다는 뜻은 아니다. 이 PASS는 실행 당시 입력의 증거다. 이후 사용자 추가 파일로 달라진 최신 전체 트리의 freshness 경계는 G11에 기록했다.

별도 compiled shader closure도 [Release PASS](C:/Users/tnest/Desktop/LostArk/out/KoukuArenaReview20261014/shader-closure-toolchain/runs/20261014T135121Z-release/result.json)와 [Debug PASS](C:/Users/tnest/Desktop/LostArk/out/KoukuArenaReview20261014/shader-closure-toolchain/runs/20261014T141947Z-debug/result.json)로 확인했다. 두 실행 모두 exit 0 및 입력 불변이다. Release shader 검증 뒤의 변경은 native fixture였고 shader 내용은 변경하지 않았다. WARP shader 검증을 실제 쿠크 화면 표시 성공으로 해석하지 않는다.

## G07. Native 검사 fixture의 실제 반영

| 파일 | 확인한 원인 | 보정한 계약 |
|---|---|---|
| `Server/Private/ServerGameplayContractTests_KoukuRaid.cpp` | Dice fixture가 기존 정수 피해 ±10% 규칙을 무시하고 정확한 HP 잔량을 기대했다. | 각 hit의 허용 최솟값/최댓값, 대상별 event 수, 전체 event 수, event 합과 HP 감소 일치, serial 증가 및 면역·원거리 0 hit를 검사한다. 고정 tick 중복/간격 검사도 유지한다. |
| `Server/Private/ServerGameplayContractTests_KoukuSupportSurface.cpp` | HP 직접 차감을 무력화 credit로 오해했고, 후반 일반 incoming hit에서도 정확히 HP 90을 기대했다. | 공통 무력화 최대값과 credit 경계, HP 차감만으로는 무력화되지 않음, 실제 whirlwind의 `Apply_PlayerToWorld` 입력 3회, 0 HP 피해·event credit·단발 성공·owner cleanup·P42 후속을 검사한다. 후반 피해도 per-hit 범위/event/HP 합/serial로 검사한다. |

피해 변동은 2026-09-30부터 존재한 규칙이며, 현재 테스트를 통과시키려고 관측 피해값을 기대값에 복사하거나 제품 피해·방어·HP·무력화 수치를 바꾸지 않았다. 기존 무력화 정책을 설명한 09-30 RESULT와 현재 published 데이터 및 실제 `ServerCombatHitRuntime` 소비 경로를 대조했다.

Dice는 [독립 리뷰](C:/Users/tnest/Desktop/LostArk/out/KoukuArenaReview20261014/dice-variation-candidate/independent-review.json) 후 [CAS 적용](C:/Users/tnest/Desktop/LostArk/out/KoukuArenaReview20261014/dice-variation-candidate/application.json), Support는 [통합 리뷰](C:/Users/tnest/Desktop/LostArk/out/KoukuArenaReview20261014/support-combined-candidate/independent-review.json) 후 [CAS 적용](C:/Users/tnest/Desktop/LostArk/out/KoukuArenaReview20261014/support-combined-candidate/application.json)했다. 원본 백업과 교체 직전 SHA 비교를 수행했고 두 파일의 UTF-8 BOM 없음·CRLF를 유지했다. 새 파일/프로젝트 등록은 없다.

## G08. 실제 Server native 실행과 미실행 경계

최종 Release Server SHA는 `ad3cde7fa81f48f3ff31ae669eecc7a0435c4343f37a062de7fdb861343276a8`, Debug는 `66b27dd2e458d25bbe93379c90684bc63fcd17a9371824189c115635dae57789`다. 아래 결과는 로그의 PASS assertion 줄 수이며 독립 패턴 개수나 사람이 플레이한 횟수가 아니다.

| 구성 | 모드 | 실제 결과 | 경과 시간 | 실행 증거 |
|---|---|---|---:|---|
| Release | `raid` | 2,066 PASS / failures 0 / exit 0 | 79.203초 | [release-confirmed-final-001](C:/Users/tnest/Desktop/LostArk/out/KoukuArenaReview20261014/runs/release-confirmed-final-001/summary.json) |
| Release | `product` | 96 PASS / failures 0 / exit 0 | 8.015초 | [release-confirmed-final-001](C:/Users/tnest/Desktop/LostArk/out/KoukuArenaReview20261014/runs/release-confirmed-final-001/summary.json) |
| Release | `bundle` | 2 PASS / failures 0 / exit 0 | 1.016초 | [release-confirmed-final-001](C:/Users/tnest/Desktop/LostArk/out/KoukuArenaReview20261014/runs/release-confirmed-final-001/summary.json) |
| Release | `cardmaze` | 201 PASS / failures 0 / exit 0 | 9.531초 | [release-confirmed-final-001](C:/Users/tnest/Desktop/LostArk/out/KoukuArenaReview20261014/runs/release-confirmed-final-001/summary.json) |
| Release | `bingo` | 51 PASS / failures 0 / exit 0 | 0.516초 | [release-confirmed-final-001](C:/Users/tnest/Desktop/LostArk/out/KoukuArenaReview20261014/runs/release-confirmed-final-001/summary.json) |
| Release | `showtime` | 25 PASS / failures 0 / exit 0 | 3.531초 | [release-confirmed-final-001-optional](C:/Users/tnest/Desktop/LostArk/out/KoukuArenaReview20261014/runs/release-confirmed-final-001-optional/summary.json) |
| Release | `overlap` | 1,102 PASS / failures 0 / exit 0 | 7.531초 | [release-confirmed-final-001-optional](C:/Users/tnest/Desktop/LostArk/out/KoukuArenaReview20261014/runs/release-confirmed-final-001-optional/summary.json) |
| Release | `dice` | 182 PASS / failures 0 / exit 0 | 3.640초 | [release-verified-final-dice-001](C:/Users/tnest/Desktop/LostArk/out/KoukuArenaReview20261014/runs/release-verified-final-dice-001/summary.json) |
| Debug | `dice` | 182 PASS / failures 0 / exit 0 | 27.906초 | [debug-verified-final-runtime-probe-001](C:/Users/tnest/Desktop/LostArk/out/KoukuArenaReview20261014/runs/debug-verified-final-runtime-probe-001/summary.json) |
| Debug | `raid` | 2,071 PASS / failures 0 / exit 0 | 709.782초 | [debug-verified-final-002](C:/Users/tnest/Desktop/LostArk/out/KoukuArenaReview20261014/runs/debug-verified-final-002/summary.json) |
| Debug | `product` | 미실행 — 시작 메모리 기준 미달 | — | [debug-verified-final-002](C:/Users/tnest/Desktop/LostArk/out/KoukuArenaReview20261014/runs/debug-verified-final-002/summary.json) |

`release-confirmed-final-001` 계열은 fixture 보정 전 EXE `af445779082f42da71e125d1a694f4e3b271e98a5ee2f303606c62ac73ce9bc7`의 결과다. 해당 결과를 최종 EXE에서 재실행했다고 하지 않는다. [런타임 동등성 비교](C:/Users/tnest/Desktop/LostArk/out/KoukuArenaReview20261014/runtime-source-equivalence-after-fixture-fixes.json)에서 두 검사 파일을 제외한 Server/Shared 180개 입력과 게시 데이터가 동일함을 확인하여, 변경하지 않은 Release 모드를 재사용했다. 수정한 Dice는 최종 Release·Debug에서 각각 다시 실행했다. 변경한 Support의 최종 Debug 검사는 아직 실행하지 못했다.

Debug 첫 최종 시도 `debug-verified-final-001`은 `0xC0000135`로 assertion 이전에 종료되어 검사 결과로 세지 않았다. out 격리 폴더에 실제 v143 14.44 Debug CRT 3개와 Windows SDK 10.0.26100 UCRT 1개를 정확한 설치 경로에서 복사한 뒤 Dice가 완료됐다. 원본 DLL 경로·SHA와 복사 SHA는 각 summary에 기록했다. 시스템 DLL 부재나 특정 버전의 비호환을 확정한 것은 아니며 PATH·전역 설치·Product Bin은 변경하지 않았다. Client는 실행하지 않았으므로 이 증거가 Client 로더 실패 또는 성공을 뜻하지 않는다.

오래된 Debug Support 재현은 322 PASS 줄·12 FAIL 줄까지만 관측한 뒤 host free commit 가드로 중단됐다. 완주 footer가 없으므로 전체 검사 개수나 최종 Debug 실패로 합산하지 않는다. 그 재현에서 확인한 stale fixture 원인을 G07에서 보정했고, 최종 결과와 별도로 보관했다.

모든 runner는 EXE·DLL·DataFiles를 out으로 복사하고 listener가 시작되기 전 종료하는 기존 contract 인자만 실행한다. 시작 free commit 3 GiB, 실행 중 1.5 GiB, 자기 private memory 3 GiB, mode timeout 1,200초를 적용한다. `debug-verified-final-002`의 raid는 정상 종료했지만 다음 product 시작 직전 메모리 기준이 미달해 나머지는 시작하지 않았다. 이는 논리 실패가 아니다. 실행 후 source/copied EXE·DLL·data의 무변경 여부는 각 summary에서 확인한다.

Release bundle의 bootstrap 2줄은 `_DEBUG` 본문을 실행하지 않는다. Debug Bingo에는 Release에서 제외된 hook 착지 fixture가 있고, overlap은 설치된 hook 66개 carrier·51개 상승 종료와 만료·사망·취소·잘못된 바닥 착지 시 기존 상태 보존 등을 검사한다. Support에는 Debug 전용 room 검사가 다수 포함된다. 이 시뮬레이션은 실제 Client의 모델·clip·효과·음향·카메라·HUD·GPU 생성이나 사용자의 처음부터 끝까지 실제 조작을 검증하지 않는다.

## G09. Native Client parser의 현재 경계

최종 Release Client OBJ/DLL을 링크한 첫 parser probe는 [첫 실행 receipt](C:/Users/tnest/Desktop/LostArk/out/KoukuArenaReview20261014/gate3/native-parser/runs/20261014T141649Z/receipt.json)에서 exit 1이다. 13개 실패는 저작 목록 121 patterns/11 bundles와 게시 런타임 목록 114 patterns/9 bundles를 동일하다고 기대한 out probe의 범위 차이와 대조했다. 제품 파서나 게시 데이터를 통과시키도록 변경하지 않고 out probe 후보만 검토·반영했다. [out probe 적용 기록](C:/Users/tnest/Desktop/LostArk/out/KoukuArenaReview20261014/gate3/native-parser/Probe.inventory.application.json)은 최종 재실행 PASS를 대신하지 않는다. 수정 후 재실행 결과를 확인하기 전까지 전체 native parser 검증 완료로 표시하지 않는다.

<!-- KOUKU_NATIVE_EVIDENCE_END -->

## G10. P65의 실제 게시 투영 결함과 활성 Flow의 범위

[P65 순수 함수 재현](C:/Users/tnest/Desktop/LostArk/out/KoukuArenaReview20261014/p65-publication-boundary-audit/reproduction.json)과 [상세 코드·데이터 검토](C:/Users/tnest/Desktop/LostArk/out/KoukuArenaReview20261014/p65-publication-boundary-audit/REVIEW.txt)에서 **저작 원본의 strict cross-direction 검사는 PASS하지만 게시 expansion 뒤에는 FAIL**임을 확인했다. 실제 현재 함수로 재현했으며 제품 publisher 전체 실행이나 live publish는 하지 않았다.

P65는 Summon5의 `CROSS_DIRECTION_CLONES`로 P71/P72/P63/P64를 참조하며 cutoff는 `cloneEndStageId=STAGE_18`이다. P71/P72/P64의 원본 Stage는 `STAGE_18=1600 ms` 뒤에 animation이 없는 `STAGE_2=1 ms`가 남아 있다. `project_kouku_saydon_composition.py`의 `_expand_parent_patterns -> _coalesce_trailing_pose_holds`가 이 둘을 `STAGE_18=1601 ms` 하나로 합치면, 다른 정의가 참조하는 cutoff Stage가 마지막 Stage가 된다. 이어 `_validate_cross_direction`의 “clone cutoff는 전체 종료보다 앞서야 함” 계약에 걸린다. Summon 6,665 ms보다 자식의 수명이 길어 발생한 단순 기간 초과가 아니다.

현재 네 활성 Flow는 projector의 실제 공유 `_pattern_dependencies`를 재귀 적용하여 다시 확인했다.

| Flow | 직접 entry | 전체 의존 closure | 격리 7패턴과의 교집합 |
|---|---:|---:|---:|
| G1 | 166 | 20 | 0 |
| G2 | 85 | 24 | 0 |
| G3 | 72 | 27 | 0 |
| Bingo | 35 | 14 | 0 |

이 최신 closure는 Parent·분신·RESULT follow-up 등 공유 의존 추적을 포함한다. G01에 보존한 초기 G3 감사의 23개보다 넓은 범위를 다시 조사한 결과이며, 초기 로그를 재작성하지 않았다. **P65 게시 결함은 현재 네 활성 Flow의 누락 의존성이 아니다.** 그렇다고 모든 저장 패턴이 정상이라는 뜻도 아니다.

다른 격리 패턴 P3/P20/P45/P51/P110/P111은 현재 Stage와 실행 lane이 비어 있으며 bundle8(P20/P21), bundle11(P51)도 이 미완성 정의 때문에 제외된다. 현재 Flow가 이들을 참조하지 않는다는 사실과 사용자가 폐기를 승인했다는 판단을 구분한다. 저장 상태 `DRAFT`만으로 원래 미게시 의도라고 단정하지 않는다. 특히 P65는 과거 revision1051 공식 게시 기록이 있어 “원래 사용하지 않는 빈 draft”로 설명할 수 없다. 결함이 처음 생긴 commit·저작 변경과 이번 베른 렌더 수정 사이의 인과는 확정하지 않았다.

**제품 수정 없음 / 완성된 수정 후보 없음.** 참조된 cutoff의 Stage ID·종료 1,600 ms와 전체 lifetime을 보존해야 하지만, coalesce만 생략하면 downstream `_project_pattern_root_motion`의 Stage별 animation 1개 계약에서 다시 실패한다. 따라서 단순 guard, authored 기간 연장, strict validator 완화나 ID별 예외를 적용하지 않았다. 기존 `HOLD_LAST_POSE`가 중간 포즈 정지와 root-motion 정지를 정확하게 표현하는지, 정상 Showtime 6,443 ms 병합을 보존하는지까지 publisher·native·Client 소비자 전체를 별도 검증해야 한다. 이번 재현은 입력 SHA와 전후 불변을 확인한 가벼운 Python JSON/순수 함수 검사이며 모델 sampling·native·빌드·Client 실행은 하지 않았다.

## G11. 빌드 이후 사용자 작업 파일과 최신 트리 freshness

23:42 KST 이후 읽은 [사용자 저장 후 Release source snapshot](C:/Users/tnest/Desktop/LostArk/out/BernGeometryProxy20261014/product-integration-applied/current-source-after-user-save.json)을 [PASS 당시 Release source snapshot](C:/Users/tnest/Desktop/LostArk/out/BernGeometryProxy20261014/product-integration-applied/verified-release/source-after.json)과 대조했다. Release 전체 입력 fingerprint는 `8fb07abf283151a405544b770ae167d15c20edf853eca9cef8a8f6c8fc97974e`에서 `68301416551db7facea12b1ebdd69be39568f570b01ffe7575b447a2403f9a83`로 바뀌었다. 정의 fingerprint는 같고 기존 입력의 SHA 변경·삭제는 0개이며, 아래 두 파일만 새로 추가됐다.

| 사용자 추가 파일 | 23:42 snapshot의 디스크 크기 | 23:42 당시 프로젝트 등록 |
|---|---:|---|
| `Client/Private/UISoundPlayer.cpp` | 0 bytes | `.vcxproj`·`.filters` 미등록, Git untracked |
| `Client/Public/UISoundPlayer.h` | 14 bytes | `.vcxproj`·`.filters` 미등록, Git untracked |

이 23:42 대조만으로는 기존 컴파일 입력·저장 EXE가 바뀌었다고 할 수 없지만, 당시에도 전체 트리 fingerprint의 freshness gate는 달라진 입력을 그대로 거절했다. 이 미등록 상태와 기존 입력 불변은 23:42 snapshot의 사실이며 이후 상태에 일반화하지 않는다. 따라서 이전 Release·Debug PASS와 sourceStable 결과는 당시 입력에 한정하여 보존하고 **사용자 추가 파일을 포함한 최신 트리의 빌드/검증 완료를 주장하지 않는다.** 사용자 디스크 파일과 편집 중 미저장 내용은 수정·등록·삭제·복구하지 않았고, freshness 검사를 우회하지 않았다. 이 문서 보정에서 새 빌드나 native/Client 프로세스도 실행하지 않았다.

23:49 KST에는 사용자가 `UISoundPlayer.cpp`와 `.h`를 실제 코드로 저장하고 Client 프로젝트에도 등록한 추가 변경을 확인했다. 이어 읽기 전용으로 남긴 [4개 파일 snapshot](C:/Users/tnest/Desktop/LostArk/out/KoukuArenaReview20261014/result-updates/20261014T145102338192Z/user-edit-inputs.json)에서 CPP 1,967 bytes, H 550 bytes이며, `Client.vcxproj`와 `.filters` 양쪽에 CPP/H 등록이 존재한다. 따라서 **현재 작업 경계는 더 이상 미등록 빈 파일 두 개가 아니다. 컴파일 입력이 추가·변경되었으므로 새 Release·Debug Product 빌드와 소스 안정성 검사가 필요하다.** 기존 PASS는 여전히 해당 빌드 당시 입력의 기록으로만 사용한다. 사용자는 계속 편집할 수 있으며 이 snapshot 이후 저장본의 동일성까지 주장하지 않는다. 이 확인에서는 해당 네 파일을 읽었을 뿐 사용자 코드를 수정하거나 새 빌드·native를 실행하지 않았다.
