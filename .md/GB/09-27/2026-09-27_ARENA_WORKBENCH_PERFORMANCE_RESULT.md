# 아레나·베른·Workbench 공통 성능 결과

## G00. 캡처가 보여 준 원인

9월 27일 저장 JSON을 비교했다. 실제 Client/UI 실행과 수정 후 게임 FPS 측정은 하지 않았다. 아래 CPU 시간은 inclusive 구간이며 부모·자식 또는 GPU elapsed와 합산하지 않는다. GPU timestamp에는 CPU 명령 공급 공백과 제출 대기가 포함될 수 있다.

| 캡처 | 유효 interval 평균 | 주요 CPU 비용 |
|---|---:|---|
| 발탄 23:06:41, 120frame | 36.24ms | NonBlend 14.90ms, secondary Present 6.25ms, shadow admission 2.42ms |
| 발탄 23:07:27, 28frame/유효 interval27 | 47.60ms | NonBlend 17.40ms(Deploy 5.56ms 포함), secondary Present 10.95ms, trail 재구성 포함 FrameRebuild 3.35ms |
| 쿠크 23:13:02, 120frame | 36.14ms | secondary Present 10.57ms, Workbench Patterns 2.47ms/Timeline 2.75ms |
| 베른 05:10:04 / 05:11:12 / 05:12:37 | 50.81 / 44.86 / 37.07ms | NonBlend 23.72 / 19.34 / 12.87ms, 공통 map fallback 1,221개 |

23:10:19의 이름이 발탄인 파일은 LOADING, map/animation 0이므로 아레나 비교에서 제외했다. 모든 위 캡처는 Debug/iterator debug 2/D3D debug layer ON이다. baseline·베른은 상세 scope OFF, 아레나 상세는 ON이므로 계측 비용과 서로 다른 카메라·설정·창 수를 통제한 A/B가 아니다. 분석 자료는 `out/KoukuWorkbenchPerf20260928/capture-analysis.json`이다.

발탄은 visible map instances 581~662, 전체 제출 indices 약 500만~577만이며 쿠크는 각각27, 약33만이다. 설치된 발탄 Deploy12종×145배치의 원본 index 상한은 약364만이다. 이는 cull/상태 적용 전의 상한이다. 반복 벽3종만 각각32/27/31개 배치돼 있다. map LOD counter가0이라는 이유로 모든 asset에 최고 품질 옵션이 강제됐다고 결론내리지 않았다. 베른 돌의 외관 품질과 geometry 비용도 같은 사실이 아니다. 화질·geometry·재질 옵션을 변경하지 않았다.

## G01. ImGui 제출과 계측

secondary swapchain은 `DXGI_PRESENT_DO_NOT_WAIT`를 사용하고 busy이면 다음 frame의 최신 UI를 제출한다. 순서를 매 frame 순환해 후순위 창만 지속적으로 밀리는 것을 피한다. main swapchain, DPI, zoom, dock/undock 정책은 유지했다. 이 수정의 실제 DWM/driver 대기 감소량은 새 사용자 캡처로 확인해야 한다.

Profiler에는 bounded32개/frame의 viewport ID·화면 위치/크기·CPU duration·flags·HRESULT와 overflow, attempts/busy/failure/occluded counter를 추가했다. 기존 v3 JSON의 additive 필드이며 worker·capture off의 기록은 거부한다. actual CProfiler와 exporter의 비UI WARP 검사에서32표본+3overflow, 결과 구분, frame 초기화, capture off, main-thread 제한, invalid 저장의 기존 파일 보존과 strict JSON parse를 통과했다. 실제 scheduler 함수 body의1~32창/최소화/창 수 변화/queue1slot 검사는 main창 제외, 중복 없음, 공정한 제출 순서를 확인했다. 근거는 `out/ArenaPerformance20260927`이다.

Microsoft의 [Present flag 계약](https://learn.microsoft.com/en-us/windows/win32/direct3ddxgi/dxgi-present)과 [동일 device 다중 swapchain의 queue 계약](https://learn.microsoft.com/en-us/windows/win32/direct3ddxgi/dxgi-multiple-swap-chains)을 대조했다. nonblocking UI 제출은 게임 simulation이나 timeline 시간을 생략하는 정책이 아니다.

## G02. 공통 맵과 그림자

MapAssetObject는 최종 camera revision과 transform이 같고 reject hysteresis가 끝난 경우 frustum 결과를 재사용한다. static shadow의 immutable material/profile admission은 모델 준비 때 계산하며 morph·texture override·visibility·transform·source option·opacity/vortex는 계속 확인한다. MapStaticBatchObject에도 같은 원칙을 적용했다. camera 확정 전 조기 cull이나 Level 이름별 예외는 추가하지 않았다.

실제 old/new production 함수와 기존 predicate를 추출한 Debug 비UI 검사는970,429 assertion을 통과했다. 모델/게임 holder는 fixture이므로 GPU 실행 검사가 아니다.19,200 frustum 평가가990회로 줄면서 visible/reject/hysteresis·진단 건수는 동일했다.12,000 material/model 조합×20frame의 shadow admission/revision도 동일했다.3mesh×200,000호출 microbenchmark는201.537→133.729ms, surface 조회600,000→0이다. 게임 FPS로 환산하지 않는다. 두 실제 CPP 최소 컴파일도 통과했다. 근거는 `out/ValtanPerformance20260927/map`이다.

베른의1,221 fallback도 같은 클래스를 소비한다. 베른 캡처에는 shadow admission scope가 없어 그곳의 그림자 절감 시간을 측정값으로 주장하지 않는다. Deploy5.56ms는 실제 draw이며 이번 culling/admission 변경으로 모두 사라진다고 기록하지 않는다.

## G03. Baked trail 반복 조회

Effect_Playback은 baked edge마다 반복하던 module variant·distribution·random identity·dynamic clock flag 조회를 호출당 한 번으로 옮겼다. per-edge 시간, random, WORLD_SAMPLE, 분포 평가·geometry는 유지했다. 실제 설치 효과의 CPU 검사300회 평균 FrameRebuild2.518→0.497ms, trail3개 합2.431→0.329ms였다.8시각×3world변환×3trail×8source변형의 geometry/payload7,826,787bytes가 비트 동일했다. `effect-perf-result.json`과 `trail-variants-equivalence.json`은 `out/ValtanPreviewCrash20260927/native`에 있다.

기본 휠윈드에서 아래 G05의 중복 연결을 제거한 뒤, 실제 첫 Product로 Play→1초 Pause→0.1초 Resume→역방향 Seek→Reset 검사를 다시 통과했다. source age0.0223562 유지 후0.0667728로 진행했고 Reset이 이펙트를 모두 정리했다. 실제 패턴 속도0.4441667을 반영한 값이며 game FPS 측정이 아니다.

## G04. Workbench와 최종 빌드

Kouku Workbench의 draft generation별 pattern tree/표시 label과 presentation ID index, leaf clipping을 반영했다. Presentation은 이미 받은 boss view를 재사용한다. 새로운 timeline layout/draw 및 presentation 세부 scope를 소비 지점에 연결했다. 두 CPP 최소 컴파일을 통과했고 실제 두 문서·ImGui core로 190개 표시 ID/좌표/스크롤/캐시 갱신 조건의 old/new 동등성을 확인했다. 폰트 1/1.5/2배, 다섯 scroll 위치, fold/picker/editor/sequence gate/popup/누락·여러 줄 label을 포함한다. Client/UI를 실행한 검사는 아니다.

Debug /Od의 실제 production 함수, 20 warmup+50측정씩 ABBA 200 frame에서 메인 121패턴/1,139리소스 트리 CPU는 1.785630→0.035202ms, sequence 문서는 0.040732→0.007540ms였다. 25,120회 ID 조회는 120.8059→3.4053ms였다. 이 격리 시간은 게임 FPS나 전체 Workbench 절감량이 아니다. 근거는 `out/KoukuWorkbenchPerf20260928/workbench-result.md`와 `check-result.log`다. Present/Profiler 독립 리뷰에서도 수정 필수 회귀는 발견되지 않았으나 GPU 제출 대기가 다른 구간으로 이동할 가능성은 새 캡처로 확인해야 한다.

2026-09-28 00:22 KST 정상 Product Debug 빌드(Engine→Shared→Server→Client)가 성공했다. Client175개 OBJ를 갱신하고 EXE/DLL을 링크·배포했다. 앞선 timeline·floor·healthbar 수정도 포함했다. Clean/Rebuild, shader skip 또는 전체 tlog 초기화 없이 기존 MSBuild 경로를 사용했다. 빌드 receipt는 `out/BuildPipeline/runs/20260927T152226848Z-debug-product.json`이며 로그는 `out/ArenaPerformance20260927/final-build.log`다. C4819 및 외부 DirectXTK PDB LNK4099 경고는 남아 있지만 컴파일·링크 오류는 없다. Client/UI와 수정 후 실제 FPS는 실행·측정하지 않았다. 최종 Gameplay 게시 결과는 아래에 별도 기록한다.

## G05. 휠윈드 잔상

원인은 `VALTAN_WHIRLWIND/SPIN`의 두 Product 동시 호출이다. 첫 번째 저작 carrier는 visible mesh particle2개(원래 trail1개 hidden)이며 decal이 아니다. 추가 `effect.valtan.pattern.420633.active`에는 visible baked trail3개/particle5개/light1개가 있었다. 사용자 설명의 첫 Product 외형을 유지하기 위해 `cue.valtan.whirlwind.active` 연결1개만 공식 unlink의 writer lock·CAS·backup/rollback·PublishV2로 제거했다. source와 generated cue가 일치하고, 공유 effect asset/다른 패턴 연결은 보존했다. validation, clip template parity, hit/presentation alignment PASS. 증거는 `out/ArenaPerformance20260927/whirlwind-unlink-{validate,apply}.log`다.

## G06. 피자·외곽 돌 추가 요청

`VALTAN_SIX_PIZZA_106`의 외곽4개 돌은 별도 패턴 소유 `effect.valtan.six-pizza.rock.active`와 Server combat-object 역할을 유지한다. 첫 적용에서 mesh scale을 `0.7200000288`에서 `1.4400000576`으로 정확히2배 늘리고 이름을 `피자 패턴 | 외곽 4개 돌`로 바꿨다. 실제 설치 WModel·재질을 준비한 비UI WARP 검사에서5시각의9개 basis 성분이2배이고 world origin은 동일했다. 이는 사용자 요청 배율이며 원본 게임 크기 복원값이라고 주장하지 않는다.

그 뒤 최신 저장본의 고정19.5초 폭발을 owner-hit 연쇄 시점과 분리하기 위해 기존 저작 element를 다음처럼 나눴다. geometry·TRS·색·material·particle 속도와 수명은 원래 저작값을 보존했으며, 옮긴5개 element는 시작 시각만 해당 그룹의0초로 이동했다.

| 편집 항목 | asset ID의 `effect.valtan.six-pizza.rock.` 뒤 부분 | 구성·시점 |
|---|---|---|
| 외곽4개 돌 | `active` | mesh1개, 기존2배 TRS 유지, 수명32초 |
| 돌 폭발 전조 | `telegraph` | 기존 inner grow·sprite·outer red3개,0초 시작 |
| 돌 폭발 | `explode` | 기존 debris+impact wave2개,0초 및0.2초 |
| 돌 폭발 전체 | `explosion.full` | 위 전조3개와 폭발3개의 저작 조합,1.5초 후 폭발 |

`EffectCatalog.json`과 기존 Valtan 아래 `피자` category에 등록했다. 전체는 기존 Kouku V1 조합과 같은 element 복사 문서이며, 개별 그룹의 나중 편집이 전체 복사본에 자동 전파되는 live reference는 아니다. active의 `detail.timing/particle`뿐 아니라 실제 소비되는 sourceRecipe의 emitter duration, Required literal, Lifetime distribution도32초로 맞췄다. detail만 연장한 첫 후보는20초 sample에서 사라졌고, 최종 후보는30.2초에도 돌1개만 유지한다. 따라서 예전18.5~19.7초 전조·폭발이 active에서 따로 재생되지 않는다.

현재 데이터의 `ownerHitChain`은 `valtan.sequence.center-six-pizza-charge.step-11`의 owner cone hit와1500ms 지연, `event.valtan.six-pizza.rock-pillar.armed`를 사용한다. BossCatalog는 armed를 `telegraph`, hit를 기존 `explode`로 연결하고 `stopActiveOnHit=true`다. 실제 Server 판정과 local preview의 같은 global clock 소비, chain 완료 뒤 정리는 각 런타임 검증과 구분한다. 여기의 문서 단독 playback 성공을 Server/Client 연쇄 성공으로 대신 기록하지 않는다. 피자·6방향 source 연결과 local target-follow의 별도 검증은 후속 결과를 따른다.

**위 돌3개 편집 그룹은 PROJECT_AUTHORED이며 원본 FullRestore가 아니다.** 원본 action420629의 `Par_O_RPBF_Atk_07_01/02`, Projectile42061915/17의 `Atk_07_06/07` 호출과 실제 UPK 첫 LOD·module·material은 확보했다. 그러나07_06은 crackline mesh3개 등을 포함한14emitter,07_07은 decal·ring·파편 등16emitter이며,4개 고정 돌 소유라는 연결은 확정되지 않았다. RPBF 참조 Projectile88개와 관련 LookInfo 조사만으로 원본 stationary-rock 소유/호출 순서까지 복원했다고 주장하지 않는다. 비슷한 효과로 대체하거나 스크린샷만 보고 크기·색을 만들지 않았고, 사용자가 요청한 청백색/핑크 원본 전조·폭발 외관 복원은 미완료다. 원본 Server 피격/연쇄 권위를 효과 자료로 추정하지 않았다.

검증은 실제 `CEffectDocumentCodec → Prepare_AuthoringDocument → CEffectPlayback`의4문서 준비와7시각 sample을 통과했다. 전조는3particle, 폭발 초기에는 debris16+impact16, 전체는1.49초까지 폭발0개이고1.6초에 폭발32개였다. Client/UI 및 arena 화면은 실행하지 않았다. `split/semantic-validation.json`은 분리 element의 시작 시각 외 필드 동일성, 기존 debris 보존,2배 TRS 유지를 확인한다. installer fixture는 무관한 catalog 저장 보존, 같은 element 충돌 거절, 부분 적용 rollback과6파일 원자 교체를 통과했다.

두 적용은 canonical writer admission 아래 최신 stable ID 병합·직전 CAS·백업·원자 교체로 완료했다. 첫2배 적용 receipt는 `out/ValtanPizzaRocks20260927/transactions/ec81ff0372274e74b4aa0c008f917bb9/receipt.json`, 분리 적용 receipt는 `split/transactions/febbd47d41864e2aa4573f0b3c9b8c8d/receipt.json`이다. 두 번째 경로의 기준은 같은 `out/ValtanPizzaRocks20260927`이다. Catalog/ResourceTree의 무관한 기존 row bytes는 유지했다. receipt 상태는 `APPLIED_PENDING_PUBLISH`이며 이 적용기가 publisher를 실행하지는 않았다. 원본 비교표·남은 경계는 같은 폴더의 `RESTORATION_BOUNDARY.md`, 실제 native 로그는 `out/ValtanPreviewCrash20260927/native/pizza-split.log`에 있다.

## G07. 피자 Full Restore와 실제 재생 연결

기존 sector composite 12개 요소를 유지하고 STEP01/03/04/10에 원본 Full Restore 네 문서를 연결했다. STEP01은 420629.stage006 점프7개, STEP03은 420629.stage008 착지14개, STEP04는 420619.stage003 6방향19개, STEP10은 420620.stage004 후반 모아치기13개 요소다. 새 STEP04는 Att_Battle_12_04의 Particle notify8개와 원본 UPK/CDO/TypeData·socket·TRS·시간을 대조했다. STEP03 임시 V2 landing과 STEP04 임시 V2 stomp 두 연결은 중복 재생을 피하도록 제거했다. 전체 효과나 다른 패턴의 공유 연결을 지우지 않았다.

실제 설치 MN_RPBF_01 모델·AnimSet·본, 모델의0.01 정규화와 네 효과의 Codec/Prepare/Playback을 비UI WARP로 검사했다. 네 문서 모두 유효한 particle/light 또는 post 표본을 생성했다. source와 제품 projection의 target-follow 계약7개, CValtan/PatternTree 두 실제 CPP 최소 컴파일을 통과했다. local Preview의 typed target lifetime/교체와 동일 cursor의 회전 유지, 진행·역방향 seek 초기화를 확인했다. 근거는 `out/ValtanPizzaFullRestore20260927/{inspection.json,native/prepare.log,compile.log,chain-result.log,follow-result.log}`이다.

후보를 최신 저장본에 직렬 적용한 backup은 `out/ValtanPizzaFullRestore20260927/backup-26e0fc55e80e44c48a3fa2fa74ab022c`이다. 새 효과와 돌 telegraph/explosion.full 두 문서는 Client 프로젝트의96.DataFiles None 및 filters에 등록했다. 공식 PublishV2가 제품4파일을 갱신했고9개 산출물을 확인했다. 로그는 `out/ArenaPerformance20260927/final-valtan-project.log`다.

## G08. 돌 연쇄 폭발의 Server·Client 계약

실제 모아치기 cone과 돌 cover circle의 교차는 Shared 순수 함수로 분류한다. Server는 source entity/pattern sequence/archetype/spawn tick이 같은 그룹에서 먼저 맞은 돌을 즉시 폭발·despawn하고 나머지는1500ms 뒤 폭발시킨다. 겹치는 돌이 없으면 arm하지 않으며 반복 타격은 시간을 덮어쓰지 않는다. 사자후·발악에는 정책을 추가하지 않았다. 원본 서버 코드를 확보한 것이 아니고1500ms는 사용자 요청값이다.

실제 CombatObjectRuntime19개 검사는 즉시 폭발,30Hz의44tick 미폭발/45tick 폭발, 피해·cover·중복방지·source 격리·cancel·늦은 입장 시 이미 터진 돌 미재등장을 통과했다. 문서 계약4개도 통과했다. Client ActorCatalog9개 파싱/실패보존 검사와 ProjectionRuntime의 완료 뒤188개 snapshot 미재생성, 중복 event/despawn, 기존 일반 lifecycle 검사를 통과했다. Level은 armed Effect를 준비하고 Effect Tool은 같은 armed asset을 combat-object Product에 연결한다. 증거는 `out/ValtanRockChain20260927` 및 `out/ValtanRockChain20260928/runtime_probe.log`, `result.md`다.

wire의 기존 Spawn→PresentationEvents→Despawn 순서는 보존한다. Client는 정확한 armed ID에 전조, hit에 폭발을 선택하고 active visual을 완료시켜 다음 snapshot이 돌을 되살리지 못하게 한다. despawn만 도착하는 종료에서도 active visual을 정리한다. 추가 bootstrap row는 optional BOSSCOMBATOBJECTOWNERHITCHAIN이며 서버 binary와 게시 데이터가 함께 필요하다. 실행 중 Server는 자동 갱신되지 않으므로 새 빌드·게시 뒤 재시작해야 한다.

최종 caller 검토에서 실제 Play master가29.9초에 끝나30.25초의 잔여 폭발을 샘플링하지 못하는 결함을 발견했다. ownerHitChain이 있는 객체에 한해서 spawn+수명까지 Preview 마지막 item을33초로 늘렸다. Server stage 합29.9초는 유지한다. 실제 Build/Advance/Seek/Activate/Apply 다섯 함수와 ActionPresentationTimeline.cpp를 사용한9개 검사는30.25초 전달, 마지막 finite clip의1.3초 pose hold,32.9초 pause/seek,33초 종료, 정책 없는 기존29.9초를 확인했다. 입력 모델/대상은 fixture이며 Client 화면 검사가 아니다. 로그는 `out/ValtanRockChain20260928/tail-result.log`다.

Full Restore 연결 뒤 clip parity는 기존 generic V2 impact 두 개를 여전히 요구해 첫 Gameplay publish를 거부했다. `Valtan.cliptemplates.json`의 STEP03/04에만 `allowlist.effectReplacement`를 추가했다. waiver 수는23개 그대로이며, exact template effect·cue/asset ID·native action/stage clip metadata·sourceStart·root·단위 world scale·follow/natural과 구 V2 중복 부재를 검사한다. 공통 template나 다른 occurrence는 바꾸지 않았다. 잘못된 asset/time/root/metadata 및 중복 V2 연결을 거부하는7개 검사, target-follow7개, hit alignment와 standalone parity(13templates/33occurrences/28effects/2exact replacements)를 통과했다. backup은 `out/ValtanPizzaFullRestore20260927/template-backup-0be9acf17d72490fa014ed2c57dc2fb0`이다.

## G09. 최종 게시·검증 상태

2026-09-28 00:23 KST 공식 Gameplay Publish가 성공했다. Kouku composition 검증, Valtan split/clip parity/hit alignment와 player hit-shape95/95를 통과하고 Retail profile을 유지한 bootstrap108,878행/32,164,616bytes를 게시했다. 실제 `Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap`에 피자 STEP11/1500ms/armed ID의 BOSSCOMBATOBJECTOWNERHITCHAIN 행이 있는 것을 확인했다. 로그는 `out/ArenaPerformance20260927/final-gameplay-publish-retry.log`다. 앞선 개별 apply receipt의 PENDING_PUBLISH는 적용 시점 기록이며 최종 게시 완료는 이 로그가 증명한다.

최종 실행 파일은 `Client/Bin/Debug/Client.exe`(00:22:24 KST)와 `Server/Bin/Debug/Server.exe`(00:19:38 KST)다. 제품 build receipt는 PASS이며 runtime required files/catalog/navigation 검사도 통과했다. 변경 JSON/XML parse와 git diff --check를 통과했다. Client와 arena는 실행하지 않았다. 사용자가 Server와 Client를 새로 실행해 Preview 화면·healthbar·FPS를 확인해야 한다. 원본 돌 폭발 외관 복원과 반복 Deploy draw 자체의 추가 instancing은 완료 범위가 아니며, 60fps 달성은 새 캡처 전에는 확정하지 않는다. 사용자 요청에 따라 외곽 돌 재질 복원 조사는 중단했다.

## G10. 09-28 V1 Effect Tool attachment 그룹 비용

새 컷신 프레임드랍 캡처의 CPU scope가 완전한79frame에서 `EffectTool.Render` 평균은38.392ms였다. AuthoringWindow19.887ms와 DetailWindow13.972ms에는 같은 전체 attachment 그룹 재구축이 각각 있었다. 저장·문서 parse가 매 frame 반복된다는 증거는 없으며 이 캡처의 DocumentLoad scope 호출도0이다. 캡처에는 선택 asset ID가 없으므로 아래 fixture를 캡처의 실제 선택 문서로 단정하지 않는다.

`Effect_Tool_Helpers.cpp`는 한 호출 안의 typed attachment/manual group/inheritance identity로 이미 만든 그룹을 찾고, 같은16float의 hexfloat 문자열은 그룹이 처음 등장했을 때만 만든다. 기존 key 문자열, 그룹·member 순서, center, 회전 및 편집 가능 여부와 이유는 보존한다. float bit를 사용해 양수0과 음수0도 구분한다. 두 번의 member 전체 검색은 해당 호출에서 모은 const pointer view로 대체했다. 그룹 검색은 O(N log G), member 집계는 O(N)이며 N은 Element 수, G는 그룹 수다. `Effect_Tool_Detail.cpp`의 authoring 그룹 행도 호출 안의 stable ID 인덱스를 재사용한다. 같은 active 문서의 선택·Solo는 문서를 교체하지 않으며, 기존 그룹 변형 commit은 행 제출이 끝난 뒤에 실행된다.

cross-frame cache, clipper, 저장·publish·preview 시계는 변경하지 않았다. source recipe의 중첩 데이터와 사용자 미저장 draft를 복사하거나 저장하지 않는다. Data·Resources 파일을 교체하지 않았으며 실제 CPP 두 파일은 UTF-8/CRLF를 유지했다.

실제 수정 전·후 production helper를 추출하고 현재 문서에서 소비되는 필드를 그대로 투영한 Debug `/MDd /Od` 격리 검사는 아래 결과였다. 각 버전60회씩 ABBA, 버전당120회 측정이며 Client/UI/GPU 검사가 아니다. source recipe의 미사용 대형 필드는 fixture에 들어 있지 않다.

| 문서 | Element / 그룹 | 이전 ms | 이후 ms |
|---|---:|---:|---:|
| 발탄 cinematic finale actor35 | 9 / 1 | 0.045656 | 0.024635 |
| 발탄 cinematic phase2 actor75 | 8 / 1 | 0.040632 | 0.022053 |
| 발탄 action420624 stage007 full restore | 248 / 1 | 11.076681 | 0.434012 |
| 쿠크 sequence scene04a matinee1 | 308 / 1 | 2.398064 | 0.455480 |
| 쿠크 common flame wave full | 392 / 14 | 3.826116 | 1.130898 |

5개 실제 문서의 전체 그룹과965개 selected-only 출력의 모든 필드를 대조했다. 추가28개 변형에서 attachment의 enabled/follow/orientation/socket/yaw/model/bone/slot, manual 그룹, inheritance, source read-only, mixed carrier, animated rotation, 양수·음수0와 문서 교체를 확인했다. 행 인덱스는 기존 선형 검색과 동일한 Element를 반환했다. member 검색만 제거한 첫 격리 후보는 발탄248개 문서에서 개선되지 않았으며 최종 개선은 동일 attachment 문자열 포맷 반복도 제거한 결과다.

두 실제 CPP Debug `/Zs` 최소 컴파일과 변경 파일 diff check를 통과했다. 근거는 `out/EffectToolPerf20260928/check_groups.py`, `result.log`, `source-tu-compile.log`, `fixture-receipts.json`, `actual-delta.diff`다. 수정 직전 dirty 원본은 같은 폴더의 `*.cpp.before`와 `baseline-receipts.json`에 보존했다. baseline SHA256은 Helpers `39b20edc52859574dd2561091bc6ec38dc77ba2b2b4e14ad3ffe96ad14d40412`, Detail `faabb1bc047393d71cfbce6160c52f09dcfe94aaeb77c37e8c81f11ddce0e5eb`다. 제품 전체 빌드와 사용자 화면/FPS는 별도 최종 검증이며 이 국소 개선을 전체38ms의 회복이나60fps 달성으로 기록하지 않는다.


## G13. 명시 Composition seek의 V1 과거 입자 표시 수정

`out/ValtanPizzaVisibility20260928/product/before-g13.log`의 실제 AnimationTool → 설치 Valtan clone → EffectPresentationService probe에서 전진 29.346초는 후반 red sector 1개(alpha 0.254336)였지만 명시 seek는 11초 yellow/red 2개(alpha 0.908842)였다. 12/18.1/24초 seek에서는 sector가 없었다. 컷신 suppression은 0이며 같은 current document의 단독 Playback.Seek는 정상이다. 원인은 앞 stage 끝 재구성의 큰 delta를 실시간 Advance에 전달해 60 fixed step 예산만 실행하면서 서비스 elapsed clock을 목적 시각으로 갱신한 것이었다. 사용자의 Play Preview 전체 GPU 비표시 자체를 이 CPU probe에서 재현한 것은 아니며, scrub와 재생 결과가 달라지는 구체적인 결함을 재현·수정했다.

`Animation_Tool.h/Animation_Tool_ValtanPlayback.cpp`, `Valtan.h/.cpp`, `Effect_PresentationService.h/.cpp`에 기본 false인 `bRebuildEffectHistory` 전달을 추가했다. 명시 reset/seek의 이전 stage 및 목적 stage 재구성에만 true를 전달하고, 서비스가 기존 bPendingInitialSeek를 설정해 정확한 Playback.Seek를 소비한다. 일반 forward·자연 stage 경계·같은 cursor의 pause/resume는 기존 증분 경로를 유지한다. 데이터·source particle·shader·재생 길이와 Server aim 정책은 변경하지 않았다.

실제 current authored 8개 resource(composite/FullRestore 4개/rock active·armed·terminal)를 Prepare하고 local boss에서 11.1/12/18.1/19.8/23.1/24/28.6/29.346/30.2초를 전진·명시 seek로 비교했다. 9개 시점의 sector count/ID/world/color가 모두 정확히 일치(maxdiff 0), target root 1개와 cinematic suppression 0을 유지했다. STEP_06의 target을 동쪽에서 북쪽으로 옮기자 yellow sector의 행 벡터가 (0,0,37.5)에서 (-37.5,0,0)으로 회전했다. 실제 installed model/animation/Effect 준비를 사용했으며 화면·GPU draw는 실행하지 않았다. source/runtime Server는 LOCK_RANDOM_ALIVE_ON_START + TRACK_TARGET_EACH_TICK 정책을 유지하고 ValtanBrain::FacePatternTarget은 landing center에서 방향을 계산한다. Server snapshot → current yaw → 동일 WorldRoot 소비는 코드 대조이며 이번 probe에서 Server process를 실행하지 않았다.

최소 컴파일은 Animation_Tool.cpp, Animation_Tool_ValtanPlayback.cpp, Valtan.cpp, Effect_PresentationService.cpp, Effect_Tool_V2.cpp와 native fixture link를 통과했다. probe의 client archive는 07:22:43에 생성되어 07:21:49 ValTan.obj의 G09 fixed RootHistory 변경도 포함한다. 따라서 실제 전진·seek 검증은 G09와 함께 수행됐다. 별도 직접 Playback에서는 rock.active의 15시점 × 16.7/80/117.609 ms frame 비교가 모두 world/color maxdiff 0이었다. full product build와 canonical publish는 root의 최종 결과를 따르며 사용자 arena GPU 표시·최종 FPS는 미검증이다. delta/백업은 `out/ValtanPizzaVisibility20260928/g13`, 실행 근거는 `product/visibility-compile.log`, `visibility-link.log`, `visibility.log`, 단독 소비 근거는 상위 `visibility.log`다.

G13 추가 검증: 30.2초 paused 상태에서6회80 ms update 동안8개 실제 effect instance와 각 clock이 모두 그대로였고, 같은 cursor Resume 뒤 동일 instance의 clock이 전진했다. `product/verification.json`은9시점 exact 동등성과 pause/resume 결과를 요약한다. 변경6파일과 갱신 문서의 `git diff --check`를 통과했다.

G13 독립 검토는6파일 delta와 explicit seek → Apply/Activate → Valtan → Sample → Commit 전달, 같은 sample의 force 재구성, pending/active owner 필터와 정상 Advance false 경로를 확인했고 actionable defect를 발견하지 않았다. 비UI probe 종료 후 잔류 probe process는 없다.

## G11. 다섯 번 발자국과 standalone 애니메이션 반영

사용자 전체 저장·반영 승인 후 최신 `effect.valtan.action.420624.stage001.full.restore`에 발자국 21개를 추가했다. 기존 dash 5개와 발자국 14개를 변경하지 않고 좌/우/좌 7개씩 500/650/1000 ms에 복제했다. 다섯 발생 시각은 0/150/500/650/1000 ms, 합계 40 Element다. 기존 19개 Element의 사용자 위치·회전·크기·재질·recipe·timing을 모두 보존했고 새 Element는 portable authored-copy origin과 별도 stable ID를 사용한다.

원본 animationClips의 `mesh_att_battle_18_02`, 500 ms, loop false, source stage 400 ms는 그대로다. optional authoredPreview(PROJECT_AUTHORED, loop true, 1140 ms)는 standalone FullRestore Open에서만 선택한다. 원본 pattern matching cache에는 override를 저장하지 않는다. 실제 소스 500 ms를 반복하여 1000 ms 접지까지 소비하고 1150 ms의 여섯 번째 전에 hold하며 자연스러운 이펙트 잔여 재생을 허용한다. 원본 재생성 builder는 유효한 optional override를 보존하고 잘못된 표식·타입·범위를 거절한다.

검증은 `out/ValtanFootsteps20260928`에 있다. native probe는 후보 loader 함수 원문과 실제 DataJson/EffectDocumentCodec/CEffectPlayback/ActionPresentationTimeline을 링크했고, 설치된 Valtan WModel와 CinematicAnimSet을 headless WARP device로 로드해 실제 clip 500 ms와 b_effectroot 표본을 사용했다. 40 Element parse/stage, 발생 전 미생성 및 다섯 발생 직후 해당 cloud 입자 생성, 1140 ms 종료 뒤 140 ms 직전 pose 유지와 1.6초 잔여 입자 재생을 통과했다. 각 발생 25 ms 뒤 전체 입자는 27/58/97/118/131개였다. 원본/override loader 분리, 나머지 45개 mapping 동일, 잘못된 override의 이전 index 보존도 PASS다. 이것은 실제 timeline/입자 소비 검증이며 제품 화면의 최종 위치·크기·접지 시각을 판정한 것은 아니다.

metadata 회귀 17개, 변경 두 CPP 최소 syntax compile, native fixture compile/link/run, JSON parse와 범위 diff check를 통과했다. 기존 UTF-8 무BOM/CRLF C++ 인코딩을 유지했다. 7개 파일은 전체 사전 hash와 각 교체 직전 bytes를 재확인하고 백업·원자 교체·실패 시 자기 변경 rollback 절차로 설치했다. `installation-receipt.json`의 effect SHA는 `2df5ebbe790c9bff26289e5012061f76dbab546f492b4e677ac5ebc2a619ecf3`, metadata SHA는 `728ba8eea4140c5451e6250904555ef4abf68ec59f40e69658fcb329937795a4`다. 최종 Product build/publish 결과는 root의 통합 결과를 따른다.

Product에서는 기존 VALTAN_WARP/STEP_02의 1600 ms stage, LOOP_TO_STAGE_END와 처음 300 ms body hidden을 보존했다. exact cue `cue.valtan.composition.valtan_warp.step_02.01`만 307~1831 ms에서 500~2024 ms로 평행 이동해 사용자 box 길이 1524 ms를 유지했다. 보이는 모델 접지와 Effect 발생이 모두 stage 500/650/1000/1150/1500 ms다. canonical JSON의 나머지 필드는 semantic 동일하며 이 추가 두 파일도 최신 baseline 백업·CAS·원자 교체로 반영했다.

Product reload는 native 500 ms 종료 경계의 시작을 무조건 거절했지만 기존 live scan은 composition+loop+ONCE를 unwrapped 시계로 지원했다. `Valtan.cpp`의 해당 admission만 기존 live scan과 일치시켰다. stage 밖 시작과 일반 native/each_loop source 범위의 거절은 유지하며 G09/G13 변경도 보존했다. Product 추가 설치와 native 근거는 `out/ValtanFootsteps20260928/product`에 있다.

실제 native Product probe는 최신 ValTan.cpp를 compile/link하여 현재 게시 데이터의 `Reload_PatternPresentationAuthoring` 전체 통과와 cached cue 500/2024를 확인했다. 실제 local Valtan과 Effect service를 10 ms씩 전진하며 각 발자국 cloud가 발생 20 ms 전에는 0개, 25 ms 뒤에는 1개인 것을 다섯 번 확인했다. 같은 시점 실제 body model clip 표본은 0.0250004/0.175/0.0250005/0.175001/0.0250005초로 반복 위상이 일치했다. owning stage 종료 후 pattern 3.7초에도 마지막 발자국의 tail이 살아 있었다. 자동 Client/UI/GPU draw는 하지 않았으며 실제 전체 화면 판정은 사용자에게 남아 있다. 추가 native compile/link/run 및 범위 diff check는 PASS다.


## G12. 저장본 보존과 게시 소비자 보완

사용자 저장·종료 승인 직후 Data/Client·Server DataFiles 2,629개(약1.71GB)를 out/FinalApply20260928/saved-before에 백업하고 경로별 SHA-256을 기록했다. 마지막 Save job39는 canonicalCommitted=true/ok=true다. job34의 파일 교체 Access Denied 실패 뒤 job35가 동일 이전 revision에서 성공했고 이후39까지 저장이 이어졌다. running은 비동기 Save 작업번호이며 Stage 번호가 아니다. 별도의 동일 frame stale Detail 덮어쓰기는 09-09 Composition RESULT G30에서 수정/검증했다.

실제 게시 실패 원인은 native에서 이미 지원하는 Sound playbackOffsetMs/Duration와 Effect playbackOffsetMs를 Composition validator가 거절한 것이었다. 해당 native 타입·범위와 once cue_end 잔여 수명을 맞추고, World Sequence의 저장된 soundTracks도 native 계약대로 검증·전달했다. 사용자 WINDUP의 최신2450ms 반복값을 예전600ms로 강제하던 test는 source payload 보존과 연속 clock 검증으로 교정했다.

사용자가 삭제한 BIND_SLOT STEP01 clip05를 가리키던 V2 binding1개와 그 clip의 낡은 template waiver1개를 정리했다. RECOVERY Shot7은 사용자 저장100ms를 그대로 유지하고 occurrence별 SOUND waiver로 명시했다. source/publish projection 불일치는 공식 PublishV2로9개artifact 중5개를 갱신했다. 필드 보존과 최종게시 검증은 아래 최종 상태와 out/FinalApply20260928에 기록한다.

G12 추가 정규화 보완: source owner 검증을 통과한 ONCE/CUE_END도 기존 `_resolve_clip_source_ms`가 종료값을 stage 끝으로 잘랐고 payload에서 sourceEndMs를 제외해 저작 종료값을 잃었다. sourceClock.endMs에 원값을 보존하고 attached endMs는 occurrence 누적 시작 + (sourceEndMs − sourceStartMs) / playRate의 기존 half-up 결과를 그대로 전달한다. detached cue는 source 기준 endMs를 함께 보존한다. stage 초과 예외는 EFFECT_V1/ONCE/CUE_END에만 적용했고 시작 범위 및 다른 kind/repeat/stop 검증은 그대로다.

2배속·앞 occurrence 200 ms·source 시작 1000 ms인 fixture는 effect 시작 250 ms, source 종료 4000 ms를 stage 종료 1000 ms로 자르지 않고 1700 ms로 전달한다. 0.5배속·half-up 경계·natural·bounded EACH_LOOP·detached·입력 불변·다른 정책과 잘못된 시작 거절을 독립 회귀 5개로 확인했다. 변경 전 동일 회귀는 8개 subtest 실패와 sourceClock.endMs 누락 1개 오류, 변경 후 모두 통과했다. 두 Python 파일 py_compile과 범위 diff check도 통과했다. 증거와 dirty baseline은 `out/CompositionTailNormalization20260928/{baseline.json,current.json,regression.json,before-tests.log,after-tests.log,actual-delta.diff}`에 있다. 이 검증은 게시 정규화 계약이며 실제 Client 실행/화면이나 최종 재게시 성공을 대신하지 않는다.

### G09 추가 — 고정 돌의 시간 재적분 제거

컷씬_프레임드랍_20260928_071303 캡처120frame의 interval평균은88.185ms다. CPU incomplete41개를 attribution에서 제외한79frame에서 Effect Tool Render38.392ms가 확인됐고 G10이 그 그룹 작업 비용을 줄인다. 별도로 외곽 고정 돌4개가 매frame 시작부터 현재age까지 Seek하는 경로를 확인했다. 고정 Root를 기존 typed fixed-step history provider로 전달하여 첫재생/되감기/큰점프는 Seek, 일반전진은 Advance, hold는 재적분하지 않게 했다. 움직이는 root의 역사를 상수로 치환하지 않았다.

실제 Client Valtan.cpp 선택컴파일과 production history helper 조건검증(첫표본/hold20회/전진10회/되감기/큰점프/tail/invalidate/실패보존)이 PASS다. G13의 실제 native localboss/service fixture는 이 변경도 포함한다. 근거는 out/CutsceneFrame20260928의 capture-analysis.json, external-history-probe.log, Valtan-compile.log다. 베른의 이전 캡처에는 NonBlend 제출 비용이 남아 있지만 상세scope가 꺼져 있어 추가 원인을 단정하지 않는다. 이번 변경 뒤 실제 게임 FPS·GPU 표시의 사용자 확인은 아직 수행되지 않았다.

G11 admission 경계 회귀는 실제 수정 블록 원문과 ActionPresentationTimeline을 native로 컴파일하여 7개를 통과했다. composition ONCE의 500 ms loop 경계와 1599 ms 후속 epoch는 수용하고, 1600 ms stage end·each_loop/native/non-loop의 source end 500 ms는 거절하며 native 내부 499 ms는 계속 수용한다. 근거는 Product probe 폴더의 `start-gate.cpp`, `start-gate-compile.log`, `start-gate.log`다.


## G14. 최신 저장본 V2·Sound 역할/시각 정합성

최종 저장본은 Library의 `boss.valtan.six.sonic.after`를 FLOOR_WIPE/FIRST_SMASH 224 ms에 새로 연결했지만 EffectRoles에 이 group의 역할이 없어 gate가 실패했다. 삭제된 그룹이나 피자 V1 누락이 원인은 아니다. 0 ms의 기존 gameplay hit 뒤에 오는 ripple·smoke·파편·fade decal·screen blur 후속 표현을 STATE/NONE 한 행으로 등록했다. 기존 주 six.sonic의 ATTACK 역할과 18개 기존 역할을 보존했다. 이 분류는 현재 저작된 후속 표현에 대한 검토이며 원본 Server 동작 또는 GPU 표시를 입증하지 않는다.

SILENCE_SLOT/STEP_01 786 ms의 사용자 `shout.burst` 연결은 피해 없는 PROJECT_AUTHORED stage의 연출이다. shared resource의 ATTACK/CLIP_TEMPLATE는 유지하고 exact binding/scope/resource/STAGE ONCE clock에만 `PROJECT_AUTHORED_PRESENTATION_ONLY` receipt를 적용했다. 실제 stage에 hit·grab damage가 생기거나 source clip mapping이 바뀌면 거절한다.

현재 saved Sound가 contact와 다른 5개 scope(WHIRLWIND/SPIN, FOUR_SLASH/SLASHES, FOUR_SLASH/SPIN, HIGH_JUMP/LAND, BIND_SLOT/RECOVERY)는 `PROJECT_AUTHORED_SOUND_TIMING` receipt에 전체 hit offsets와 현재 sound cue payload, stage animation 전체를 고정했다. 기존 <=100 ms 고빈도 track 예외는 유지했다. 독립 검토에서 cue 원문만 pin하면 playRate/앞 clip cursor가 달라져도 wall time drift를 허용하는 문제가 발견돼 expectedAnimation을 추가했다. cue의 identity/event/start/optional 재생창, animation source trim/rate/clip 순서·길이 또는 hit가 바뀌면 다시 실패한다. 새 rule이 없는 공격·소리의 기존 정렬 검사는 그대로다.

검증은 `out/ValtanAlignment20260928`의 dirty baseline 4개와 `actual-delta.diff`, `install-receipt.json`, `focused-tests.log`, `validation.json`에 기록했다. hash/CAS/백업/원자적 파일 교체 뒤 기존 V2 binding·Sound·gameplay·presentation 4개 source raw hash가 그대로임을 확인했다. 기존 30개 allowlist row도 동일하다. 실제 repository alignment는 19 roles, 42/42 ATTACK bindings, 16 clip-template 위임, 1 presentation-only, 5 authored Sound receipt, 472 stage hit points, 94 개별 Sound 정렬, 22 고빈도 예외, 8 external binding, 9 combat-object hit로 PASS했다. 15개 focused test(회귀 subcase 포함), Python compile, 변경 JSON parse, diff check를 통과했다. 과거 테스트가 제거된 pizza V2 stomp ID를 찾던 한 곳은 현재 존재하는 impact binding의 동일 timing 실패 검사로 교체했다.

새 C++나 runtime 데이터 값을 바꾸지 않았으므로 G14 자체 TU compile은 없다. 최종 공식 projection/publish와 Product 빌드는 root 작업이며 Client/UI를 실행하거나 조작하지 않았다.

G14 독립 재검토는 실제 FOUR_SLASH occurrence playRate 1→2 변경을 재현했고, 보완 전 PASS였던 입력이 보완 후 authored sound animation receipt drift로 거절되는 것을 확인했다. 15개 focused test와 4파일 diff check도 별도로 통과했으며 추가 actionable defect는 없었다. 검토 증거는 out/CompositionTailNormalization20260928/g14-review-before.json 및 g14-review-after.json이다.

G12 root-motion 최종 파생 동기화: Gameplay Publish는 사용자 편집으로 VALTAN_BIND_SLOT/STEP_01의 클립이 5개에서 4개가 되고 stage가 4107 ms가 된 뒤에도 남은 5000 ms root-motion 행을 거절했다. 공식 `Tools/ValtanActionExtractor/build_valtan_rootmotion.py --out ...`로 최신 Encounter/PatternBindings와 설치 AnimSet b_root를 재생성했다. 해당 4개 source slice는 1400/900/900/900 ms, rate 1, 비반복이며 4100 ms 뒤 사용자 gap 7 ms 동안 terminal displacement를 유지한다. stale duration 5000→4107 ms, samples 153→126, 마지막 forward 0.1962→-0.1003 m로 고쳤다. 나머지 119개 stage는 모든 sample까지 semantic 동일하다. 사용자 gameplay/presentation/Encounter/bindings는 bytes 그대로 보존했다.

전체 50 patterns/120 stages/7755 samples를 대조했고, 실제 Gameplay Publisher root-motion 블록과 공식 sample packing 함수를 분리 실행하여 120개 stable stage/112개 runtime row 검증을 통과했다. typed PORTAL_TARGET_RUSH 8개는 공식 계약대로 packing을 생략했다. 공식 builder `--check`, 해당 회귀 5개, JSON parse와 diff check도 PASS다. `out/ValtanRootMotion20260928`의 candidate/backup/installation receipt를 남기고 최신 입력 hash 재확인·CAS·원자 교체로 rootmotion 파생 파일 하나만 설치했다. 최종 SHA는 `d982fdcfcd4773b303f5cf416d23f9ab221d29810013779322bcee01133d22b3`이다. 전체 Gameplay Publish 성공 여부는 이후 root의 통합 결과를 따른다.

### G12 최종 게시·제품 빌드·저장본 보존 확인

사용자의 저장·종료 및 전체 반영 승인에 따라 G07/G09/G10/G11/G12/G13/G14/G30 변경을 설치하고 공식 Valtan PublishV2, Composition Publish, Gameplay Publish를 모두 완료했다. 최종 Gameplay generation은 `4c690697d4356ee0c1446dade174a693464d239c6cd8b90755ebe6dba9f56a47`이다. 생성 문서의 SHA-256이 generation ID와 일치하고, 포함된 161개 artifact의 크기·SHA-256과 Gameplay.bootstrap의 같은 generation 참조가 모두 일치한다. 근거는 `out/FinalApply20260928/final-generation-verification.json`, `valtan-final-project-publish.log`, `composition-final-publish.log`, `gameplay-final-publish.log`다. 기존 root-motion 불일치로 실패했던 최초 게시 로그와 최종 성공 로그를 구분한다.

Engine/Shared/Server/Client를 포함한 Debug 제품 빌드와 설치를 통과했다. 최종 receipt는 `out/BuildPipeline/runs/20260927T224848583Z-debug-product.json`이며 result PASS, skippedBuild false, missingRuntimeInputs/invalidRuntimeInputs 빈 배열이다. 전체 빌드 뒤 Product 발자국 admission 수정의 Valtan.cpp를 다시 컴파일·링크했다. 기존 C4828 인코딩 경고와 DirectXTK.pdb LNK4099 경고는 남으며 오류는 없다. Release는 이번 작업에서 재빌드하지 않았다.

Composition 최종 회귀 71개, FullRestore metadata 17개, Product admission 7개, alignment 15개, root-motion builder 5개가 모두 통과했다. 실제 native 피자 probe의 9시점 전진/seek 일치, STEP_06 target 90도 회전, pause/resume 상태 유지와 실제 Product 발자국 다섯 번의 접지 위상도 통과했다. G30 Save 상세값 fixture는 52조건을 통과했다. 별도로 JSON 2179개/XML 12개 parse와 최종 추가 generation JSON parse를 확인했다. 각 세부 근거와 검증 경계는 위 해당 G의 결과를 따른다.

승인 직후 백업과 최종 파일을 비교한 결과 2611개는 bytes 그대로, 18개는 요청 수정 또는 공식 파생 게시 결과, 새 파일은 위 generation manifest 1개다. 사용자 gameplay/combatobjects/patternsoundcues/RenderingProfiles 원본은 bytes 그대로다. presentation은 발자국 cue의 307→500 ms 시작 및 같은 1524 ms 길이를 유지한 종료 조정만 다르다. 기존 effect Element 19개는 모두 그대로이고 21개만 추가했다. 삭제된 clip05의 V2 orphan 1개 정리 외 기존 V2 연결은 보존했다. 게시 Sound payload는 현재 saved source와 일치하고 Composition bytes도 새 projection과 일치한다. 근거는 `out/FinalApply20260928/preservation-audit.json`, `preservation-audit.log`, `audit_saved.py`다.

Client/UI/Server를 자동 실행하지 않았다. 따라서 파일 설치·게시·Debug 빌드 완료와 실제 재실행 후 화면 판정은 구분한다. 실제 아레나의 최종 GPU 표시와 컷신 FPS는 아직 확인하지 않았으며, Bern 캡처의 남은 NonBlend 비용 전체가 해결됐다고 판정하지 않는다. 사용자 전역 렌더링 옵션·카메라 편집값은 보존했다. 다음 Server/Client 실행은 이번에 설치·게시한 결과를 읽는다.
