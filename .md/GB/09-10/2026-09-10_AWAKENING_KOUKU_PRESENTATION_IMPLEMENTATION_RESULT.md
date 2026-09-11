# 각성기·쿠크 연출 후속 구현 결과

## 현재 상태

작업 브랜치는 `codex/awakening-kouku-presentation-fixes`, 시작 HEAD는
`e26cd2b282293bd4cb433338bdbc6a7315dd3f84`다. 사용자의 튜닝 중에는 후보만 검증했다.
사용자가 정리했다고 알린 뒤 Client/Server 모두 종료 및 디스크 약22GB 여유를 확인했다.
최신 저장본과 source SHA를 전부 대조한 뒤25개 저작 파일에 복구 후보를 적용했다.
`out/AwakeningKouku20260910/activation_receipt.json`에 적용 전후와 정확한 이전 byte 백업을
남겼다. 공식 Kouku/Gameplay/Map WorldSequences publish와 Engine/Shared/Server/Client
Debug 제품 전체 빌드·실행 폴더 배포가 모두 성공했다.
이 소스·수치 검증을 사용자 visual PASS로 취급하지 않는다.
팡파레와 다른26개 패턴, 모든 presentationResources 및 Warlord17240 사용자 변경을 보존했다.

## 원인과 구현

| 기능 | 확인한 근거와 수정 | 데이터 상태 |
|---|---|---|
| Artist31930 반복 | 원본 emitter500초가 약506초 full 수명을 만들어 재사용 mesh예약3964가 owner2048 초과. 6초 action cue_end 후보와 이전 action 예약의 지연 제거 연결 | 정본 적용 |
| Artist/Lance/Warlord 배경 | 원본 opaque static mesh를 sceneBackdrop으로 구분. 실제 로컬 활성 occurrence 동안 map/sky/shadow를 생략하고 NONLIGHT에 배경 제출, character depth와 이후 FX 유지 | Artist2/Lance19+19/Warlord44 opaque 적용 |
| Lance34630 카메라 | 원본 Matinee 이동·시선·FOV·속도·종료를 기존 RecoveryCamera sidecar에 연결 | 새 sidecar2개, 배경 localOnly 적용 |
| Lance V/T 크기 | 원본/cooked 용 geometry 길이 약3.63m 일치. source bone basis와 playable pre-scale 조합에서0.01 중복 축척 실측. 해당 attachment basis만 정규화 | 소스 수정 |
| Warlord17250 | 카메라2개, camera particle10개, static notify10개/48section 누락 복구. 기존340개 요소 보존, 최종218/180개 후보 | 정본 적용 |
| DM F2050230 | 현재43/원본69 차이와 외부 emissive 중복 배율, 일부 UniformRange 축소 확인. 원본 PS/VS·MIC·world좌표·provider 경로 복구 | 원본 순서69개 정본 적용, codec/playback/GPU 검증 |
| DM V2050520 | StartSize2행의 네 벡터가 단일 min/max로 합쳐짐. 원본 op4 payload 복구. SceneColor sprite가 오래된 화면으로 이전 FX를 사각형으로 덮던 시점 수정 | 크기 정본 적용, 렌더 소스 수정 |
| DM D2050240 | 현재25/원본51 차이. 메인 검격 RT0 연산 일치와 별도로 누락26개 재질·입력·왜곡 복구 | 원본 순서51개 정본 적용, codec/playback/GPU 검증 |
| DM BA2050010 | 원본 첫 clip 자체가0.2/0.4초 두 찌르기. 제품3단계 clip01→03→04,1400/1067/1700ms와 입력·root motion 동시 후보. 효과59→75, 미연결/근사 native와 크기 분포 복구 | 정본 적용 |
| 거미 얼굴 | HDR 이미지가 scene exposure를 함께 받음. optional displaySpace로 final tone mapping 이후/UI 이전 합성 | 소스 수정, leaf 1개 정본 적용/공식 validator 통과 |
| 쿠크 공 | 사용자 box1708ms 시작에 맞춰 interval180ms,6×4m 직사각형 생성, 기존 bounce/smoke 유지 | 정본 적용 |
| 휠윈드 돌진 | 기존 spin3204ms에 Server target-facing charge10m 연결. 평균3.121m/s로 거미약6m/s보다 느림 | 정본 적용 |
| 패턴 트리 | Patterns by Gate child 높이240→480 | 소스 수정 |
| Reverse Sector/타원 | X/Z 독립 축척, safe sector의 보완 영역. UI/codec/publisher/Server/Client wire 연결 | 소스 수정, 사용자 저작 유지 |
| 레이저 밀림 | 기존7개 Collider의 ENTER_AREA OnSuccess가 비어 피해가 실행되지 않음. 휠윈드10%/2m/242ms를 복제하고 BOSS_FORWARD 지정 | 정본 적용 |
| 단순 피해 편집 | Box Detail에서 피해·반복·밀림 직접 편집. 기존 ENTER_AREA/RESULT 원자 연결 재사용, 다른 occurrence가 쓰는 정의는 직접 변경하지 않음 | 소스 수정 |

`sceneBackdrop`은 배경이 활성인 프레임의 전체 맵 교체이며 부분 화면 stencil mask가 아니다.
원본 alpha 장식은 normal 합성을 유지한다. sidecar localOnly에는 opaque와 alpha 배경 및
camera-view 요소를 모두 포함한다. Solo에서 제외되거나 remote에 숨겨진 배경은 맵 교체를
요청하지 않는다. Renderer 요청은 성공/실패 프레임 종료에서 초기화한다.
단일 MapAssetObject와 실제 deferred map batch인 MapStaticBatchObject의 Render/Shadow를
모두 처리한다. resource가 없거나 fallback/visual suppression인 배경은 환경 숨김을 요청하지 않는다.

사용자가 추가한 원작 BA 이미지 두 장을 직접 열람했다. BA1/2는 길게 뻗은 반투명 보라색
찌르기와 날카로운 흰 끝, BA3/4는 넓은 곡선 잔상과 얇고 밝은 외곽선이 기준이다.
이 원작 이미지를 현재 후보가 실제 게임에서 일치했다는 증거로 사용하지 않는다.
BA 원본75개 occurrence는 기존59개의 상대 순서까지 유지하며 복구했다. 신규10개 native PS의
정밀 상수 감사에서5개 함수의25개 상수/24개 명령이 원본 DXBC 비트와 달랐다.
특히 SD390의 `lt ..., 2.5e-7`이6자리 disassembly 텍스트에서0이 되어 log/pow 뒤
0-mask를 선택하는 원래 경계 분기가 사라졌다. 원본 IEEE 비트로 교정하고 별도 검증했다.
후속 감사는 연결된 SD49개 전체로 마감했다. 잘못0으로 줄어든 임계값은
323/342/343/349/365/372/390 총7개 프로그램이며 각도 계수도 원본 비트로 교정했다.
D361은 호스트의 WorldToLocal 전달만 존재하고 함수의 source[1..3] 대입이 없던 누락을
연결했다. 실제 원본 PS 비교1105920 pixels에서 비정상 실수·상대오차1e-3 초과0이다.

대형 세이튼 선행 이펙트는 설계 설명 요청 범위다. 현재 제품 stage는 애니메이션1개와
startOffset0을 요구하고 bundle 공통 presentation은 CAMERA만 지원한다. 기존 UI에서
임의 공백 stage나 bundle effect pre-roll이 지원된다고 설명하지 않는다. 독립 선행 연출
시간을 세이튼 패턴이 소유하도록 확장하면 이전 쿠크 패턴 종료 지점에 억지로 결합하지
않아도 된다. 이 일반 선행 구간 기능 자체는 이번 소스에서 아직 구현하지 않았다.

## 자동 검증

실행한 검사만 기록한다. 아래 GPU 검사는 WARP 수치 출력이며 Client/UI를 실행하거나
이미지·스크린샷을 생성하지 않았다.

| 검사 | 결과와 증거 |
|---|---|
| Engine Debug ClCompile | 성공, `out/AwakeningKouku20260910/engine-compile.log`; 기존 인코딩/변환 warning 남음 |
| Shared Debug ClCompile | 성공, `out/AwakeningKouku20260910/shared-compile.log` |
| Server Debug ClCompile 및 격리 링크 | 성공, `server-compile.log`, `shared-probe-build.log`, `server-probe-build.log`; 실행 파일은 out에만 배치 |
| Client Debug ClCompile | 성공, `client-compile.log`; 최종 제품 링크·배포도 아래 별도 성공 |
| 표준 Debug 제품 빌드·배포 |Engine/Shared/Server/Client 모두 성공, 필수 runtime 입력 누락0. `product-build.log`, `out/BuildPipeline/runs/20260910T143053354Z-debug-product.json` |
| 얼굴 실제 Deferred pass17 |3456 checks, exposure0.1/1/4, alpha0/.5/1, 최대오차2.38419e-7. `overlay-probe-run.log` |
| 실제 SceneColor Capture 함수 |1549 checks, stale 손실0.8→0, MRT/DSV 복원, D3D warning0. `scene_color_refresh/result.log` |
| Warlord 실제 RecoveryCamera |20130 samples/40274 checks, 최대 eye9.424e-5m/look9.449e-5m/FOV.005806도. `out/WarlordAltVRecovery20260910/camera-probe-run.log` |
| Artist 실제 retire/예산 함수+호스트 fixture |1540 checks, 기존 action만 예약 해제, pending 삭제, active container 지연 제거, owner한도 유지. `artist_repeat/result.log` |
| 타원 Sector 실제 Shared C++ |72150 checks: 독립 arc polygon 근사오차 bound oracle12000, 기존 원형/역영역 호환60000, 접선·원외·회전·0/360·잘못된 값. `ellipse/result.log` |
| 기존 쿠크 편집기 harness 빌드 |성공, Client 실제 draft 메서드 사용, Engine delay-load, live UI 호출 시 fixture가 실패하도록 유지. `kouku-editor-build.log` |
| 쿠크 Preview Transport |독립 X/Z Sector/Reverse의 preview→Save→Reload→Logic/damage 연결과 기존 CAS/원본 보존 통과. `kouku-preview-contract.log` |
| 쿠크 Composition Editor |damage/push/repeat 편집, 공유 정의 보호, 중복 재사용, 잘못된 입력 보존 및 기존 편집 계약 통과. `kouku-editor-contract.log` |
| 실제 Server 쿠크 overlap/bundle |`--kouku-object-overlap-contract-test`, `--kouku-bundle-contract-test` 모두 failures0. 타원/Reverse와 boss yaw4방향 밀림, legacy/new bootstrap loader 포함. `kouku-overlap-contract.log`, `kouku-bundle-contract.log` |
| 최종 제품 Server 쿠크 검사 |실제 `Server/Bin/Debug/Server.exe`로 두 계약 재확인, 모두 failures0. `kouku-product-overlap.log`, `kouku-product-bundle.log` |
| DM F/D/V 실제 codec/playback |21005 checks, 실패0, 원본 UniformRange20480개 비교 최대1ULP. `out/LanceVisualRepair20260910/fdv_runtime_probe_final.log` |
| ALT 배경 실제 codec/stage/재저장 |5개 문서20 checks, 실패0. `out/LanceVisualRepair20260910/backdrop_runtime_probe.log` |
| DM BA 실제 codec/playback |75개 모두 활성 sampling,219 checks, 실패0/비정상 실수0. Python7개 통과. `out/DimensionMasterCombo20260910/ba_native/verification_receipt.json` |
| 적용된 BA 저작 정본 검사 |실제 Data 기준 Python7개 통과. `ba-product-data-tests.log` |
| 적용된 DM7개 문서 실제 C++ 검사 |F/D/V/BA4의 실제 Data 및 Resources를 기존 codec/playback에 입력. 21224 checks 실패0, 비정상 실수0. 처음 DLL search path 누락으로 시작 실패 후 Client Debug 의존성 경로를 명시해 정상 실행. `dimensionmaster-applied-runtime-check.log` |
| BA390 원본 PS/현재 PS 비교 |1140750 pixels RGBA 오차0, 중심·끝 UV 포함. `out/DimensionMasterCombo20260910/ba_native/f390_native_diff/verification_receipt.json` |
| BA 주요 검격 geometry |원본 cone/plane/bendplane/swing01/02/03과 실제 WModel957정점/3369인덱스의 위치·법선·UV0 오차0와 winding 일치. 추가 임의 크기 보정 없음. `out/DimensionMasterCombo20260910/ba_native/geometry_verification_receipt.json` |
| F341 원본 PS/현재 PS 비교 |270조건×4096=1105920 pixels. 실제 DDS5개,3종 WorldToLocal, 비정상 실수0/상대오차1e-3 초과0. `out/LanceVisualRepair20260910/f341_native_diff/verification_receipt.json` |
| OneLayer374 원본 PS/현재 PS 비교 |12조건50700 pixels, RGBA bit exact, 비정상 실수0. 실제 FX11 pass depth-read 유지/ONE-ZERO/RT1 비활성 확인. `out/DimensionMasterDMissing20260910/one_layer/verification_receipt.json` |
| 최종 particle/mesh FXC |SD49개 최종 상수 및 D361 연결 수정 뒤 두 shader fx_5_0 컴파일 성공. `out/LanceVisualRepair20260910/final_integration_receipt.json` |
| 공식 gameplay publisher 격리 실행 |skip 없이 성공,6profile/230skill/109damage/65boss pattern. DM3단계와 원본 이동128samples 일치. 실제 Data/제품 출력 수정0. `out/DimensionMasterCombo20260910/publisher_isolation_receipt.json` |
| 실제 Kouku/Gameplay 배포 |정본322에서24개 실행 가능 pattern/195stage/7bundle 게시. Gameplay 정상 배포, 기존 partial-hitshape 경고 유지. `gameplay-product-publish.log` |
| 실제 Map WorldSequences 배포 |공식 scoped publisher 성공, 공 생성 구간 반영. `map-product-publish.log` |
| Engine 격리 전체 링크 |성공. `engine-probe-build.log`; 이후 Client 전체 링크는 C드라이브 공간 부족으로 중단되어 성공으로 기록하지 않음. 사용자 정리 후 표준 제품 빌드로 재개 |
| 실제 Server 전체 contract 후보 실행 |DM 새3개 수동 콤보 검사 모두 PASS. 전체는 failures13. 아래 기존 검사 전제 불일치가 있어 전체 PASS가 아님. `server-gameplay-candidate-contract.log`, `server_contract_failure_audit.json` |
| JSON/XML 및 diff whitespace |최종 JSON28/XML2 parse 성공, `final_parse_receipt.json`; 적용·publisher 뒤 `git diff --check` 성공 |

표에서 별도 root가 없는 로그는 `out/AwakeningKouku20260910/` 기준이다.
후보 상태의 검사를 실행 중 제품 검증으로 기록하지 않는다.

전체 Server 검사 실패13개는 별도 코드 조사로 구분했다. World fixture가 폐기된 v7을
생성하여 첫 load와 후속 rollback 조건8개가 실패한다. 쿠크5개는 Gate1 전용 패턴을
Gate3 actor에 요청한2개, 사용자가 바꾼 gaze 좌표를 이전 상수로 검사한1개, 실제 fixed
tick의 pending-member 준비를 건너뛴 lifecycle2개다. 이번 요청에 맞추려고 원래 target
제약을 완화하거나 사용자 튜닝을 이전 값으로 되돌리지 않았다. 진단 receipt에 로그와
실제 소비자 위치를 기록했으며 이 무관한 테스트들의 수정·재실행은 하지 않았다.

## 원본 해석 경계

D/BA OneLayer 원본 PS의 CB0[10].x는 material uniform 밖의 LostArk 엔진 추가 binding이다.
shader의 `SceneColor*(1-x)+Emission` 계산과 원본 OneLayer의 ONE/ZERO overwrite 계약은
확인했지만 engine field 이름과 원작 default는 아직 증명하지 못했다. 프로젝트에는 해당
전역 scene attenuation 기능이 없으므로 배경 보존의 중립0을 명시 adapter 값으로 사용한다.
이는 원작 default를 복구했다는 의미가 아니다. 원본 PS에 같은 adapter 입력을 제공한 수치
일치와 원작 전체 엔진 의미의 일치를 구분한다. overwrite에도 원래 투명 효과의 depth-read를
유지하고 opaque depth-write로 바꾸지 않는다.

새 물리 입력 `Effect/DimensionMaster/Textures/FX_TEX_05/fx_m_elecircuit_001.dds`는
원본에서 추출해 Resources에 준비했다. 다른 PC에 전달할 때 팀장의 Drive 관리 대상이며
Git에 강제 추가하지 않는다. DM 평타 damage hitShape는 이번 화면 복원과 별개의 기존
publisher 경고(maximumRange fallback)가 남으며 원본 피해 범위를 복원했다고 기록하지 않는다.

## 사용자 실행과 남은 확인

저작 병합·공식 배포·제품 빌드와 필요한 자동 검사는 완료했다. Server CMD와 Client는
실행하지 않았으며 둘 다 종료 상태다. 자동 stage/commit은 하지 않았다.

1. 이 PC는 LAN `server-host`이므로 VS의 `Server + Client` profile을 Ctrl+F5로 시작한다.
2. Lobby → Character Select → 차원술사에서 LMB1회의 두 찌르기와 추가 입력이 없을 때 종료,
   다음 입력의 BA3·BA4를 확인한다. F/V/D도 사용자 원작 이미지와 비교한다.
3. 도화가 ALT+V를 다시 사용할 때 카메라·이펙트가 재생되는지 확인한다. 도화가/워로드/창술사는
   원본 배경이 활성인 동안 기존 맵·sky가 가려지고 캐릭터·연출 이펙트가 보이는지 확인한다.
   창술사 V/T 용 크기도 확인한다.
4. Lobby → KoukuSaydon → F1 → Action Workbench에서 Patterns by Gate 높이, 거미 얼굴 밝기,
   공의 빠른 직사각형 생성, 휠윈드의 플레이어 방향 돌진과 레이저의 보스 전방 밀림을 확인한다.
5. Collider Box Detail → Collider purpose → Damage collider에서 Damage (% max HP), 반복,
   밀림을 편집하고 Apply → Save한다. General region으로 되돌리면 해당 box의 피해 연결만
   해제한다. Sector/Reverse Sector는 X/Z를 독립적으로 바꿔 얇고 긴 영역을 저작할 수 있다.

화면의 최종 일치는 사용자 확인 전이다. 대형 세이튼의 독립 선행 연출 구간은 앞서 적은
설계 방향이며 이번 구현에 포함하지 않았다. 원본 OneLayer 엔진 binding 기본값과 기존 DM
평타 damage hitShape fallback도 위 경계를 유지한다.
