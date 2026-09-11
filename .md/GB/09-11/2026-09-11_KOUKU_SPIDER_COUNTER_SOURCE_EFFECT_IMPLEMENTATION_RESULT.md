# 쿠크 거미카운터 원본 이펙트 연결 결과

## G00. 구현 상태

대상은 기존 GATE2 `KAKULSAYDON_G1_PATTERN_15`, 표시 이름 `쿠크_거미카운터`다.
원본 `MN_RPCZ_00` action 4219776의 실제 연결 clip을 기준으로 2개 V1 Effect를 추가했다.
현재 패턴의 01→02→02→03→04 반복과 기존 카운터·공포·돌진 로직을 유지했다.
비어 있는 pattern 16에는 애니메이션이나 이펙트를 임의로 넣지 않았다.

| 제품 문서 | 요소 | source notify | 현재 패턴의 시작 시각 |
|---|---:|---|---|
| `effect.kouku.gate2.4219776.stage1.full.restore` | 5 | Dust, 두 EyeLight socket | 2500, 3500, 9667, 10667, 16834, 17834 ms |
| `effect.kouku.gate2.4219776.stage2.full.restore` | 9 | Dash Trail 4개, Dash Ground 5개 | 4500, 11667, 18834 ms |

검정·빨강 바닥은 원본 `Par_U_RPCZ_Dash_Ground_01_LOC_INT`의 EPAL_Z sprite 5개다.
실제 decal TypeData가 아니다. 첫 두 emitter는 검정 `[0,0,0]`과 적색 `[0.2,0.005,0.005]`,
나머지 세 emitter는 원본 붉은 CD02 색 변화다. source 크기, ±X 부호, 600 cm 길이,
0.95/1초 particle lifetime을 보존했다. 바닥의 실제 notify 시각은 4692/11859/19026 ms다.

첫 설치는 최신 Composition revision 326을 읽고 bytes를 재검증한 뒤 327로 저장했다.
presentation resource 55/56과 pattern 15 occurrence 7~15만 추가했다. 이후 재생성은
현재 문서를 다시 읽어 다른 작업의 logic 변경까지 보존했고, 멱등성 실행의 변경 파일은 0개다.
EffectCatalog와 Client 프로젝트/filters의 96.DataFiles/None 등록을 완료했다.

## G01. 원본 재질과 렌더링 원인

원본 first LOD는 12 emitter이며 두 eye socket 복제로 runtime 14개다.
source socket props에서 Eye 01/02는 `bip001-head`, Dash Trail은 `FX_State_01`의
`bip001-spine1`을 확인했다. root snapshot과 FOLLOW attachment, source local-space 여부를 유지했다.

9개 원본 재질을 기존 native interpreter의 2342~2350에 연결했다. 원본 PS 9개와 서로 다른
VS 8개를 회수했으며 deferred program은 0개다. 기존 2304~2341의 함수 본문 38개와
재질 표의 기존 305개 행을 보존했다.

2349 WorldOffset02에는 실제 미표시 원인이 있었다. 원본 PS는 `CB0[0].w`를 opacity로
읽지만 generic lowering은 X만 1로 두어 W가 0이었다. 월드 UV basis와 원본 VS의 world-position
varying도 필요했다. 새 source-qualified patch는 opacity W=1, emitter WorldToLocal 3행,
기존 carrier의 source-centimetre world position을 원본식에 공급한다. Renderer는 이미 사용 중인
차원술사 341/361의 emitter inverse 계산을 재사용한다. 다른 Artist 재질 수식은 수정하지 않았다.

2346은 실제 Ribbon이며 sprite로 분류하면 안 된다. `cascadeRibbonV1` bounded 계약으로
기존 Trail에 연결하고, CPU의 원본 width/RGBA/Dynamic 계산부터 Renderer의 mask·finite 검사와
vertex 전달까지 연결했다. 최대 50 point, 0.6초 point lifetime, source 간격 75 cm,
tiling 5 m, tessellation 0.05 m를 사용한다. 폭은 원본 SizeScaleByTime에 따라 출생 시 0에서 증가한다.

## G02. Resources 경계

새 Resources 바이너리는 없다. 원본 DDS 17개와 아래 기존 mesh 1개를 사용한다.

- `Effect/KoukuSaydon/Textures/<원본 package>/<원본 object>.dds`
- `Effect/KoukuSaydon/FullRestore/Meshes/fm_e_plan_001.wmodel`

실제 18개 입력 파일 3,428,652 bytes의 로컬 resource closure를 확인했다.
정확한 texture별 sourceObjectPath/assetId는 두 authored 문서와
`out/KoukuSpiderSource20260911/native/texture_resource_receipt.json`에 기록했다.
Resources는 Git 제외 입력이며 index에 추가하지 않았다.

## G03. 실행한 검증

- 두 authored JSON의 공식 scoped material/color-space, module override, attachment,
  native sprite option, v15 runtime extension 검사와 resource closure 통과.
- Python 도구 6개의 구문, 변경 프로젝트 XML 2개 parse, scoped `git diff --check` 통과.
- `build_kouku_spider_counter_restore.py` no-write 반복 실행에서 변경 파일 0개.
- 원본 PS 9개 × 2 seed의 DXBC→HLSL 모든 RT 비교 통과. 최대 차이는 2.98e-8이다.
  이 검사는 원본식 번역 검사이며 제품 화면 판정이 아니다.
- 실제 제품 FXC `fx_5_0` ParticleKouku2304, MeshKouku2304, Trail, Decal 컴파일 통과.
  현재 MSBuild 설정과 같은 `/Zi /O1` 또는 `/Zi /Od`를 사용했다. 샘플러 X4000와
  Effects deprecation X4717 경고는 남았으며 오류는 0개다.
- 위 4개 CSO를 `Client/Bin/Debug`와 현재 producer의 `Linked` 폴더에 같은 SHA로 배포했다.
- 최종 C++ Parse/Stage/60 Hz sweep은 두 문서 14개 요소의 admission을 모두 통과했다.
  필수 anchor 누락과 역방향 재생도 검사했다. 10 m/s 합성 anchor 이동에서 14개 모두 실제
  particle 또는 Ribbon point를 생성했다. SpawnPerUnit인 2345/2347/2348은 각각 peak 2 particle,
  2346은 peak 8 point였다. Ribbon payload 432개 표본의 color/dynamic mask가 모두 0xF이며
  invalid 0개, source width 0~1 m를 확인했다.
- 먼저 검사한 0.1 m/s 이동에서는 바닥·눈·먼지는 생성됐지만 SpawnPerUnit 3개는 source 거리
  간격에 도달하지 않아 0개였다. 이는 고속 검사와 구분한다. `cpu_probe_result.json`은 최종
  10 m/s 검사로 갱신됐다. 사용자 화면을 대신 재생한 결과가 아니라 CPU 입력 검사다.
- 통합 publisher와 Debug 제품 빌드는 root가 수행하며 해당 최종 로그를 후속 기록한다.

검증 증거는 `out/KoukuSpiderSource20260911/scoped_validation.json`,
`idempotence/installation.json`, `native/product_compile/result.json`, `deployment.json`,
`native/verification_summary.json`, `cpu/cpu_probe_result.json`에 있다.

## G04. 사용자 화면 확인

Client/UI를 실행·조작하거나 화면을 캡처하지 않았다. 최종 visual fidelity는 미확인이다.
사용자가 Client의 F1 Action Workbench에서 GATE2 `쿠크_거미카운터`를 재생하여
세 번의 돌진마다 검정·빨강 바닥, 붉은 trail, 두 눈과 먼지를 확인한다.
이 문서는 자동 검증을 수동 화면 PASS로 대체하지 않는다.

## G05. 패턴 길이와 실제 Product 연결 보완

첫 revision327의 publisher는 성공했지만, 마지막 두 새 occurrence가 기존21501ms 패턴 길이를
넘어 pattern15가 unavailable로 격리됐다. 성공 exit나 전체 개수만으로 연결 완료를 판정하지 않고
생성된 patternbindings에서 실제 asset ID를 찾는 과정에서 발견했다.

builder가 기존 패턴의 남은 시간 안에서 새 occurrence의 종료를 정하도록 수정하고 revision328로
저장했다. 마지막 준비 occurrence14는17834+3880 대신17834+3667ms, 마지막 돌진 occurrence15는
18834+3167 대신18834+2667ms로 닫는다. 원본 Effect 내부 시간, 앞7개 occurrence, 기존 animation,
logic과 패턴 전체 길이는 보존했다. projector에서 pattern15와15개 source clip·새FX9개 연결이
복귀하고 Product pattern26/stage200을 확인했다. bundle6의 기존 빈 pattern16 문제는 별도이며
이번 기능 때문에 만든 새 실패로 기록하지 않는다.

원본 문서 displayName은64-byte 계약 안으로 줄였고, native Ribbon을 정확한 bounded carrier로
승인하도록 보완한 뒤 최종14/14 C++ 검사를 수행했다. parser의 기존 경계를 완화하지 않았다.

최종 revision328 공식 owner publisher의 product/map/world/gameplay.balance4개 domain이 모두
통과했다(`out/KoukuSpiderSource20260911/owner_publish_328_final.log`). 중간의 source closure 변경
감지 실패는 기존 출력 rollback 뒤 재실행했고 성공으로 세지 않는다. 최종 generated
patternbindings와 Encounter의 실제 Product 목록에서15/29/30 세 패턴을 찾고, 새 거미FX9개와
기존 내려찍기·불뿜기2개가 모두 유효한 시간 창 안에서 연결되는 것을 확인했다.
`final_product_links.json`에 revision328, 세 패턴과11개 occurrence의 정확한 시간/asset ID를 기록했다.

최종 Debug Product compile·link·deploy는 Engine/Shared/Server/Client 모두 PASS다.
`out/BuildPipeline/runs/20260911T073907619Z-debug-product.json`에 결과가 있으며
missingRuntimeInputs는0개다. Client.exe는2026-09-11 16:39:07 KST 빌드다.
빌드와 publisher 완료를 실제 입력·화면 검증 완료로 확대하지 않는다.


## 돌진 stage2 전체 미표시의 v15 준비 실패 수정

stage1 연기는 표시되지만 stage2 바닥이 표시되지 않는 추가 관찰에 대해, stage2 v15 문서의 실제 renderer resource prepare가 `Visual-program adapter denominator did not map to prepared elements.`로 실패함을 재현했다. 문서의 bounded cascade ribbon은 supplemental-only projection인데 준비 검증이 LocalDecal adapter 수 1개 이상을 잘못 요구했다. `Effect_DocumentRenderer.cpp`에서 유효한 supplemental-only projection을 허용하고 실제 typed adapter 수 일치 검사를 유지했다. 바닥 크기·색·배치 JSON과 shader는 이번에 변경하지 않았다.

같은 실제 C++ WARP preparation/renderer attach/`CEffectObject::Clone` 검사는 수정 전 stage2 실패에서 수정 후 성공으로 바뀌었다. G1 내려찍기 v15도 동일하게 성공했다. 증거는 `out/KoukuDecalAnchor20260911/renderer_stage_probe_run.log`와 `renderer_stage_probe_fixed.log`이며 후자는 exit0이다. 이 검사는 설치 리소스를 사용하되 Client/UI/창/swapchain/draw/캡처는 생성하지 않았다. 실제 돌진 위치의 바닥과 잔상 표시 판정은 사용자 확인 대기이며, 세부 원인과 최소 컴파일은 [내려찍기 결과](2026-09-11_KOUKU_GATE1_TWO_PATTERN_FULL_RESTORE_IMPLEMENTATION_RESULT.md)의 추가 수정 절을 따른다.
