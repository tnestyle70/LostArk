# 발탄 장판 수신 표면·패턴 동기화 수정 결과

## G00. 저장본 보존과 실제 변경

작업 브랜치는 codex/valtan-all-effects-editing이며 기존 대규모 dirty 변경을 보존했다.
사용자가 현재 저장본 반영·게시를 명시 승인한 상태에서 canonical writer lock 아래 최신 파일을
읽고 stable ID로68개 대상 필드와 정확한 timing receipt2개만 병합했다. 교체 전 원문 백업,
hash 재확인, 원자 교체와 자기 변경 rollback을 적용했다. 새 rendering option 변경은 없다.
원문 및 영수증은 `out/ValtanEffects20260929/backup`과 `source-install-receipt.json`에 있다.
후속 공중 착지 요구는 추가11개 occurrence 필드를 같은 절차로 병합했다.
`landing-install-receipt.json`의 파일은 실제 Playback에서 검증한 후보와 의미가 동일하다.

추가 요청의 `3회 구르기 후 돌진`은 VALTAN_DASH_CHARGE(현재 표시명 `3회 땅 치기 후 돌진`)다.
CHARGE cue의 source dash Effect는 최신 저장103149bytes, SHA256
`456d1b3911d9541e919c148969c23e29ad01936f8e7e033e7c966fb7eae2cf76`를 그대로 보존했다.
현재 presentation generation의172개 artifact 안에 같은 bytes가 포함되는 것을 확인했다.

## G01. 공통 데칼 receiver

기존 V1 필터는10개 재질 번호에만 actor 제외를 적용했다. 진입 연출3852/3856과 피자
5184/5185/5198이 빠져 있었다. 실제 진입 연출 source burst·수명도18~19초와 겹친다.
V1/V2는 새 공용 Shader_EffectDecalReceiver.hlsli에서 기존 Depth marker0/5와
RGBA32_FLOAT PickPos.W의 skinned bit8·actor program을 정확히 Load한다. V2 CPP도
Target_PickPos를 바인딩한다. 정적 Map과 원본 투영 volume·색·alpha, sprite/mesh/trail은 유지한다.
신규 HLSLI의 프로젝트·filters None 등록도 포함했다.

V1/V2 FXC fx_5_0/O1 compile PASS. WARP 실제 packed receiver75,264조건 PASS,
새 actor 제외5951조건 외 기존 보호 재질·Map 판정 차이0. V2 전체 CPP /Zs PASS.
증거는 `out/ValtanGroundReceiver20260929/validation.receipt.json`이다.

## G02. 피자와 부채꼴의 실제 축

source-warnings의18개 ground 요소는 root가 돌아도 snapshot attachment가11초 방향을
고정했다. source basis를 detail yaw에 한 번 합치고 snapshot attachment를 꺼 mutable
root를 소비한다. 최초 방향과 원본 재질·크기·시간은 유지한다. stage001 cue도
arena.center.target-follow를 사용하며 local yaw90으로 이전 body/source basis를 보존한다.
V2 binding 자체의 방향과 시간은 변경하지 않았다.

stage004 전체13개 중 B_Root/b_root의 부채꼴3개만 socket roll+90으로 수정했다.
설치 MN_RPBF_01 골격에서 local Z가 world -Y이므로 이것이 월드 반시계90도다.
무기본 요소는 유지한다. 같은 asset을 쓰는 피자와3시·9시 지형 파괴에 함께 적용된다.

실제 모델12_06/12_07/12_10 clip과 CEffectDocumentCodec/CEffectPlayback 검증:
fan27표본의 CCW90 오차 최대0.00001365도, normal 변화0, 축 크기 차이0.000006883 이하,
무관33 packet 보존. 설치 정본의59개 대상 필드도 후보값과 일치했다.
420 forward tick에서0→47도 추적 시 기존 경고 오차47도, 수정 후0도.
처음 비교한 red warning/stage001 projector 축 오차는0.00001761도 이하였으나, 이는
붉은 발광 입자 자체의 방향을 증명하지 못했다. 후속 감사에서 실제 shader5213 장판의
전방은 local -Z, 붉은 notify014의 emission 전방은 source +X여서 별도90도 차이를 찾았다.
해당11개 요소의 snapshot source basis만-90→0으로 보정했다. 실제 Playback999개
matrix에서 붉은 emission/sector 방향 오차는90도→0.00001070도 이하, 중심 offset 방향
오차0.00012255도 이하, 높이 변화0, 반경·크기 변화0.000006m 미만이다. 무관한96개
matrix와 본체 충격 notify013·V2 landing·재질·시각을 보존했다.
GPU 화면 판정은 아니며 근거는 `out/ValtanEffectAlignment20260929/verification.json`과
`landing-verification.json`이다.

## G03. 전멸 피해와 돌 파괴 순서

VALTAN_FLOOR_WIPE_130 SECOND_SMASH hit offset0→500ms, stage500→1000ms,
animation EXACT→HOLD_LAST_POSE로 변경했다. 기존500ms clip, 두 cue99/101ms와
transform,1500ms recovery는 보존했다. SIX_PIZZA 착지 피해250ms는 그대로다.
실제 Server 코드31검사 PASS: SECOND 시작4.600s, 피해5.100s, 회복5.600s, 완료7.100s.
기존0ms hit는 다음 tick4.633s에 실행돼 실측 차이는466.667ms다. 저작 offset과 전체 완료
지연은500ms다. 마지막 플레이어가 죽어도 나머지 동작·회복을 완주하며 hit 이전 사망은
기존 ABORTED를 유지한다. 근거는 `out/ValtanEffects20260929/wipe-native/RESULT.json`이다.
실제 최종 게시32,222,580bytes bootstrap을 다시 읽은 후속36개 검사는 전부 PASS했다.
이 검사는 in-memory timing 교체 없이 게시 catalog의1000/500ms와 root-motion 끝1000ms를
직접 확인하고 같은 실제 Brain tick과 마지막 사망 후 완주를 재확인했다.
근거는 `wipe-native/published-RESULT.json`, `published_runtime_probe.log`다.
단일 explicit playMs를 무시하던 root-motion generator는 기존 segment sampler로 해당
source window 끝을 유지한다. 정본 SECOND_SMASH만1000ms로 병합하고 원래0~500ms의
17개 sample을 완전히 보존했다. 이후500ms는 마지막 forward -0.556m/lateral0으로
유지하며 총33개 sample이다. 다른 Stage bytes는 보존했다. 관련 새 회귀2개와 기존4개
PASS, 기존 전체 명시 chain 목록 테스트1개는 수정 전부터 저장된4개 chain 추가로 불일치하여
이번 범위에서 목록을 바꾸지 않았다. `rootmotion-hold/install-receipt.json`을 따른다.

돌은 Server ownerHitChain이 이미 collider 내부0ms/외부1500ms를 구분했다. 같은 Off asset을
ARMED/HIT에서 동시에 시작하는 Client 표현이 원인이었다. 피자·3시·9시의 두 armed flag를
false로 저장하고 같은 asset인 이 계약에서 HIT까지 idle을 유지한다. Product와 Preview가
실제 HIT 때 Off를 시작한다. Preview cone은 같은 보간·고정 yaw를 사용한다.
원본 Off 내부 burst 지연1.82s와 Server 피해 시각은 보존했다.
실제 함수·Shared geometry native144검사 및 Python6검사 PASS. Valtan.cpp 전체 Debug x64
/Zs도 PASS. 근거는 `out/ValtanStoneSequence20260929/RESULT.json`, `valtan-tuc.json`이다.

## G04. Save & Publish 실패 원인과 검증

PublishCandidate는 추가 draft가 없으면 최신 projection을 이전 Encounter로 다시 교체했다.
9시 COMBO_STEP_11(index12)의 저장7870ms와 이전 게시2130ms 차이로 receipt가 맞지 않아 실패했다.
이 무조건 교체를 제거하고 의미가 동일한 Product만 byte 재사용하며 최종 값으로 receipt를
생성한다. 새 저장 Stage와 receipt·실패 rollback을 실제 candidate 경로에서 검사했다.
사용자 저장본의3시·9시 COMBO_STEP_11도 이전 root-motion 길이2130ms가 남아 있었다.
전체143개 root-motion Stage와328개 Encounter Stage를 대조해 이2개만 최신9670/7870ms로
정상 generator에서 재생성했다. 각각 기존66개 sample을 보존하고 clip 끝의-0.0869m
위치를 유지한다. 다른 Stage bytes는 동일하며 duration 불일치는0개다.
`rootmotion-saved-durations/RESULT.json`을 따른다.

별도 실제 게시 시간초과의 진단은180초 제한을 유지하며 baseline/candidate 구분과
양 stream의 마지막2000자를 남긴다. 부분 출력·UTF-8·실패 rollback의 새3개 회귀 PASS.
receipt 전체4205행 검사4.539초였으며 serializer가 시간초과 원인이라는 근거는 없었다.

저장된 피자 V2 landing479ms와 gameplay/audio250ms는 그대로다. 기존 exact timing receipt에
CLIP_OCCURRENCE clock을 정확히 검증하는 기능을 추가했다. 지연 전멸도 기존1ms sound와
hit500ms·animation 전체를 기존 exact sound receipt로 기록했다. 범용 검사를 완화하지 않았다.

최종 원본에서 split/Product Validate, clip parity13 templates/35 occurrences,
attack binding50/50과496개 hit point 정합성 검사 PASS. 관련 Python38검사 PASS.
소스 JSON7개 parse와 수정 C++/HLSL·project/filter diff check PASS.
게시·최종 Product 빌드 상태는 아래 최종 기록에서 구분한다.

## G05. 최종 게시·설치와 사용자 확인 경계

V2 Product projection은9개 artifact 중5개를 최신 저장본으로 게시했다.
Gameplay 게시 PASS(109121행/32222580bytes), Composition 게시 PASS
(sourceManifestId `7c7864bc8dcccf4bbd7002fceccf60ce01108e762b8d733332142d1f884b1c6e`).
실제 PublishCandidate도 PASS. revision은
`dc07dd0d9af95e91bf8430f0fe2e31643c34d07ff849d3ffc73bb4171e7c4cde`이고 sourceManifestId는
`002a0b5f8ee0f15642548e66bae99906f7dcdc15c3ba89a4c9e03ac4c75f59fb`다.
정본과 candidate의172개 presentation artifact·서버 bootstrap generation이 모두
`1d8f4b49dc0163f805364e4255c182f211d181d9f55c54c8fffa0aa1da78aadd`로 일치한다.
사용자 최신 dash 저장본의 동일 SHA 포함, 원본79개 변경 필드·정확한 timing receipt2개,
프로젝트/filter 등록2개를 최종 재검사했다. `final-publish-verification.json`과
`final-source-field-verification.json` 및 `publish-candidate-complete.log`를 따른다.
최종 publisher 관련6개 회귀도 writer 점유가 없는 상태에서 PASS했다.
사용자 추가 HUD·엔딩 유령 발탄 경로는 아래 후속 기록에서 구분한다.
첫 Debug Product runner는 실행 중 Client16832/Server42136의 표준 출력 점유를 확인해
컴파일 시작 전에 중단했다. 사용자 종료 확인 후 실행한 Product runner는114초에 PASS,
Server 테스트OBJ1개와 Client OBJ98개·CSO2개를 컴파일하고 EXE와 런타임 배포를 완료했다.
로그는 `product-build-complete.log`, 정식 receipt는
`out/BuildPipeline/runs/20260928T163923108Z-debug-product.json`이다.
공통 receiver의 FXC early-return 경고는 판정이 동일한 단일 반환식으로 제거했다.
V1/V2 FXC와 WARP75264조건을 다시 통과하고 정식 Product 증분 빌드도 PASS했다.
추가 CSO2개 배포 receipt는 `out/BuildPipeline/runs/20260928T164556250Z-debug-product.json`,
로그는 `product-build-final-installed.log`다. 기존 native adapter 경고와 인코딩 경고는 남아 있다.
실행 중 앱을 자동 종료하거나 Reload하지 않았고 빌드 후 실행도 하지 않았다.
최종 화면 확인은 사용자에게 남기며 이 결과를 visual PASS로 기록하지 않는다.

## G06. 입장·사망 엔딩의 HUD 숨김

MainApp의 쿠크 전용 Sync_KoukuCinematicUI를 Sync_CinematicUI로 확장하고 현재 Level의
발탄 entrance/finale 상태를 기존 CUIInputRouter suppression에 연결했다. 보스 체력바의
CUI_Sprite와 HUD·HP text·창 입력이 같은 gate를 따른다. Level에서 직접 그리던 이름표,
말풍선, 상태 text, interact/party/gate/MVP text에도 동일한 조건을 적용했다.

camera owner가 입장·엔딩 숨김 여부를 보관해 source 종료 뒤 카메라 복귀까지 유지하며,
End_CinematicCameraOverride에서 해제한다. source만 남거나 camera tail만 남은 경우 모두
숨김을 유지한다. 일반 전투 카메라가 시작되면 flag를 다시 계산한다. 저장된 visibility는
수정하지 않아 완료·Stop·실패·레벨 전환에서 기존 UI로 복구된다. 자막·FPS·ImGui 개발
도구는 기존 별도 경로를 유지한다.

MainApp h/cpp와 Level_ValtanArena h/cpp4개는 UTF-8 BOM없음·원래 CRLF를 유지했다.
실제 함수에서 구성한 native lifecycle/routing26검사 PASS. 입장·사망, 양 owner tail,
skip/cancel/failure, 다음 전투 카메라, Level 전환, 기존 쿠크 경로를 포함한다.
관련 증거는 `out/ValtanCinematicHud20260929/validation.json`에 있다. 변경 CPP2개의 전체
Debug x64 /Zs도 PASS했다. 마지막 정식 Product 빌드는43개 OBJ와 Client EXE를 새로
생성·배포하고 PASS했다. 기존 Server·Engine 및 셰이더도 최신으로 확인했다.
최종 receipt는 `out/BuildPipeline/runs/20260928T165549919Z-debug-product.json`,
로그는 `out/ValtanEffects20260929/product-build-hud-final.log`다. 기존 인코딩·DirectXTK PDB
경고는 남아 있으며 컴파일/링크 오류는 없다. 이 빌드 뒤 데이터 generation과79개 필드·
사용자 dash 저장본 hash를 다시 확인했고 모두 PASS했다.

## G07. 사망 엔딩의 복원 유령 발탄 확인

boss death/VALTAN_GHOST_DEATH_AUDITION에서 source-preview.finale의 게시 world object가
BOSS_VALTAN_GHOST와 valtan.cinematic.finale를 선택한다. authoring과 게시 actor/clip은
일치하며 설치 smooth WModel은 기존 복원본 hash `c196b093…edcfba`와 일치했다.
현재3개 native84 재질과 피부 specular RGB0, NONLIGHT/pass16 경로도 유지된다.
따라서 코드·Data·Resources를 추가로 바꾸지 않고 이미 복구된 경로를 최종 빌드에 포함했다.

현재 CSO 전체/PS bytes는 다른 source family의 추가로 과거 검증본과 다르지만, 현재 ghost
모델·현재3재질·현재 pass16으로 실행한 WARP의 RGBA와 depth 전체 buffer는 과거 복원
검증 pass와 byte-identical이었다. 3draw/14472triangles, covered=colored=depth12397pixels,
alpha1, nonfinite0, RGBsum4331.743753647432이며 blend OFF/depth write ON이다.

GPU 검증은 이전 archived WorldSequenceObject의 no-window fixture에 현재 ghost 행만
갱신하여 실행했다. 이전 fixture가 최신 전체 catalog의 무관한 schema를 읽지 못한 실패는
전체 Client 검증으로 대체하지 않았다. 최신 전체 Client는 위 Product 빌드로 컴파일했고,
실제 Client/UI 실행과 엔딩 화면 확인은 수행하지 않았다. 근거는
`out/ValtanEffects20260929/finale-ghost-audit.json`, `ghost-gpu/comparison.json`,
`ghost-gpu/buffer-comparison.json`, `ghost-gpu/installed.log`다.

## G08. 최종 완료와 남은 확인

요청된 소스·저장본 병합, Valtan/Gameplay/Composition 게시와 실제 PublishCandidate,
Debug Engine/Server/Client 및 셰이더 빌드·배포를 완료했다. 추가 입장/엔딩 HUD 변경도
마지막 Client EXE에 포함했다. 사용자 최신 돌진 effect bytes와 렌더링 option을 보존했다.
Client와 Server는 종료된 상태를 유지했다. 사용자가 Server→Client 순서로 재실행하면
새 게시본과 실행 파일을 사용한다. 실제 컷씬·피자·돌 파괴의 화면 판정은 사용자 확인 범위다.

## G09. GBResources 추가 전달

후속 요청에 따라 `C:/Users/user/Desktop/GBResources`에 빠져 있던 최근 발탄 리소스3개를
현재 설치 Resources에서 같은 상대 경로로 복사했다. 기존에는 이3개가 GBResources2에만
있었다. 유령 발탄 복원 모델 `Character/Valtan/Ghost/MN_RPBF_02.wmodel`과 에테르의
`Effect/KoukuSaydon/FullRestore/Textures/fx_tex_00/fx_a_line_002.dds`,
`Effect/KoukuSaydon/FullRestore/Textures/fx_tex_high_00/fx_c_atypical_003.dds`다.

기존37개를 byte 그대로 보존하고3개42602616bytes를 추가해 총40개69575627bytes다.
전체40개의 길이·SHA-256이 현재 Resources와 일치했다. README와 resources-manifest.json의
목록·크기·출처도 갱신했고 이전 metadata는 out에 백업했다. 정본 Resources나 실행 Data는
변경하지 않았으므로 재빌드·재게시가 필요한 변경은 없다.

이 폴더는 기존 리소스 팩 위에 적용하는 추가·교체본이다. ghost의 기존 donor·weapon·texture
의존성은 감사했고 새3개와 구분했다. Data의 재질 override·shader·EXE는 섞지 않았다.
근거는 `out/ValtanEffects20260929/resource-delivery/verification.json`과
`resource-delivery-ghost-audit.json`이다.
