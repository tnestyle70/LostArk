# 쿠크 이펙트·본 부착 복구 결과

## G00. 완료와 적용 경계

이 문서는 2026-09-25 쿠크 복구의 effect/presentation 분담 결과다. 기준 branch는 `codex/kouku-timeline-local-preview`, 시작 HEAD는 `1ddbcffc016144edb8c8120c4164f68668fdec40`이다. 기존 미커밋 변경을 유지했다. Client와 UI를 실행하거나 화면을 캡처하지 않았다. 첨부된 사용자 이미지와 실제 원본·설치 모델·headless 소비자를 대조했다. 렌더링 옵션과 Resources binary는 이 분담에서 변경하지 않았다.

C++와 shader source는 공유 작업 트리에 반영했다. authored JSON은 `out/KoukuEffectRecovery20260925`에 후보만 만들고 상위 통합 담당자가 최신 저장본 기준 stable ID/필드 병합, 백업, freshness 확인과 원자적 교체를 담당한다. 통합 Product 빌드와 domain publish 결과는 상위 RESULT를 따른다. 이 문서의 native PASS는 사용자의 화면 승인과 별개다.

## G01. 회전카드와 주사위

네 `effect.kouku.card.spinning.{heart,spade,clover,diamond}`의 원본 `par_l_rpct_05_sk_13_1_loc_int.particlespriteemitter_0`는 이미 존재했지만 지나간 위치의 보라 잔상 수와 투명도 조정이 필요했다. native2999, world space, 원본 SpawnPerUnit 30 cm를 보존하고 lifeTime 배율을 1.125로, 최종 `detail.color.multiply.a`를 0.5로 설정했다. 이는 사용자가 요청한 외관 조정이다. 8 m/s 실제 Playback transform history에서 60 Hz, 2초 동안 peak 5개, 과거 위치 birth, 정지 0개와 alpha ≤ 0.351을 확인했다. sourceScale.alpha만 줄이면 원본 ColorOverLife가 덮으므로 최종 color multiply를 사용한다.

주사위 카드 배출은 `effect.kouku.card.match.emit.full.restore`의 원본 고정 축 LockAxis를 가진 8개 요소만 `followEmitterAxisRotation=true`로 설정하고 P78의 해당 occurrence yaw를 180도 돌리는 후보를 만들었다. 다른 sprite·mesh와 global basis에는 보정을 전파하지 않았다. 이 yaw 후보는 초기 저장본에 한 번 적용하는 delta이며 두 번 누적하면 안 된다.

P78에서 `logic.73` occurrence를 실제 삭제하고 `logic.86` 속박을 유지한다. suit trigger125~128을 PURSUIT_PROJECTILES로 연결해 각각 HEART/SPADE/CLUB/DIAMOND visual 하나를 소유한다. 이미 설치된 Server combatobject.kouku.pursuit → Shared spawn/snapshot/contact → Client presentation 소비자를 사용한다. 50% 접촉 damage, speed 3, spawnRadius 3, homing, 600000 ms lifetime은 기존 카드 logic 입력을 복사한다. 클라이언트가 damage나 추적 결과를 판정하는 경로는 추가하지 않았다.

주사위 접촉 폭발은 공유 `effect.kouku.card.match.explosion`을 바꾸지 않고 새 `effect.kouku.card.match.contact.recovery`로 분리했다. 크기 ×1.5와 요소 Y +0.5를 적용하며 4개 suit의 contactVisualId를 이 resource로 연결한다. EffectCatalog의 DIRECT_AUTHORED_DOCUMENT, EffectResourceTree와 Client `96.DataFiles` None 항목을 함께 준비했다.

## G02. 메두사와 블랙홀

P94는 현재 attack Effect만 재생하고 원본에서 복원된 별도 `effect.kouku.bingo.medusa.face.full.restore` resource는 소비하지 않았다. 이 face에는 붉은 바깥 ring emitter23/36과 red glow29/30/32가 들어 있다. 기존 resource를 gaze 시점 3167 ms부터 3261 ms 동안 같은 occurrence TRS로 추가했다. occurrenceId는 P94의 nextPresentationOccurrenceOrdinal에서 정수 ordinal로 발급하고 counter를 증가시킨다. 임의 suffix ID를 넣으면 실제 publisher가 거절한다.

P94의 기존 animationRootVerticalScale을 0으로 바꿔 애니메이션 root Y 하강을 억제한다. 연쇄 parent와 child의 동일 root-scale 계약은 통합 담당자가 P107 전체에 반영한다. actor 자체의 임의 Y 이동으로 시각 오류를 숨기지 않았다.

블랙홀은 상위의 prep → P94 → hold/explosion 연쇄에 맞춰 두 Effect를 hold child의 local 0 기준으로 다시 맞췄다. 원본 stage003 ray pulse는 emitter를 13초 window 동안 반복하고 중앙 ball의 10초 particle life를 ×1.3으로 맞춘다. stage004 폭발은 정확히 local13000 ms, stage006 recovery는 13200 ms 이후다. native Playback에서 12.9초 body52/center47 visible hold particles와 13.1초 center71 explosion particles를 확인했다. body의 stage004 항목은 별도 light pulse라 13.1초 particle-count를 중앙 폭발 판정으로 사용하지 않는다. 숫자 검증의 source anchor는 identity이므로 실제 boss 본 부착 검증을 대신하지 않는다.

source preview 메타데이터는 prep12_02를 제거하고 hold12_04를 13초 LOOP_TO_WINDOW로 연결했다. 제품 Composition의 native clip source0..1042 crop은 상위 후보가 소유한다. Effect sourceModelPreview schema의 full-clip loop를 제품 Composition crop과 동일하다고 설명하지 않는다. 원본 prep stage002에는 이 두 Effect에 포함된 발생 요소가 없었다.

P11의 세 blue safezone occurrence 종료를 red gaze end 20001 ms와 맞췄다. Bingo floor white source는 RGB1/2, red source는 RGB3/.2/.1이다. Server의 empty→red transition에서도 Client는 redFlipId를 먼저 재생하고 redId NEXT를 소비하도록 수정했다. white→empty는 기존 인스턴스 정리로 즉시 지운다.

## G03. BONE Collider와 노란시선 모델 준비

`KoukuSaydonPresentationPlayer.cpp`는 following BOSS named bone Collider와 following WORLD BODY named bone Collider의 BONE 계약을 검증한다. 실제 bone/socket 전체 basis에 authoring local scale·XYZ rotation·position을 먼저 곱하고, 마지막 forward를 XZ yaw로 투영한다. 본 +Z를 먼저 평탄화하면 원본 fire axis가 -Y인 입 본에서는 잘못된 방향이 되므로 순서를 바꾸었다. 저장된 sampledBasis와 편집 중 resource 재배치도 같은 전체 bone 기준을 유지한다. WORLD의 실제 bone pivot 제공, publisher bake와 Server region은 logic_tuning 분담이 같은 계약으로 연결한다.

백스텝은 실제 설치 MN_RPCT_05 WModel과 원본 34_04/14_01/14_02 clip에서 788개 matrix를 샘플했다. P102 .23/.24/.25는 각각 b_wp_1/bip001-spine2/bip001-head에 연결된다. 원본 FX_Prj01/03/02의 실제 socket·element TRS와 각 box 위치가 대응한다. 사용자 box가 이미 맞춰진 2913/2886/2858 ms의 actor-space TRS를 해당 시점 실제 normalized bone inverse로 환산하여 local TRS 후보를 만들었다. 재합성 최대 오차는 2.44e-7 이하이며 window 내 yaw 범위는 hand 약83도, spine 약69도, head 약66도다. receipt는 `backstep-bone-fields.json`이다. common tuning이 start를 1555 ms로 앞당겨도 위 calibration 시점의 사용자 위치 보존은 유지되지만 새 birth 시점의 모양은 실제 bone animation에 따라 달라진다. 같은 clip의 P43/P114 복제는 통합 담당자가 처리한다.

공굴리기 추가 조사에서는 0.740/1.5/2.99/3.266초 b_root의 normalized X=(0,0,1), Y=(-1,0,0), Z=(0,-1,0)을 확인했다. 본 +Z가 수직이므로 이 본의 raw +Z를 planar forward로 삼으면 안 된다. 공 회전축과 Saydon model +X 앞 방향의 정렬 조사 결과를 logic_tuning 담당자에게 전달했다. 최종 placement 후보는 그 분담의 RESULT를 따른다.

`EffectCompositionModelPreview::Select_SourceEffect`는 standalone source effect에서 Area가 비어 있던 문제를 고쳤다. 이미 Kouku Gate/actor/placement 검증을 통과한 source에 `LV_LUT_MIDNIGHTC_ED`를 설정해 Begin_BundlePreview가 빈 Area의 Gameplay.world.json을 찾지 않게 한다. 실제 G2 BIG_SAYDON CModel, material descriptor, idle, yellow-eye clip, b_wp_1과 weapon을 headless WARP로 생성하고 84 bones 및 source Area를 확인했다. 실제 arena CNpc clone/화면 판정은 사용자 확인 범위다.

## G04. 바주카 원본 MODEL 재질

LaserCannon의 유일한 named material slot은 `wp_mn_rhkp_06_mi_dead`다. 원본은 `wp_mn_rhkp_06.mat.wp_mn_rhkp_06_mi_dead`, PBR Base MSK dead+emissive이며 Showtime 총 program26과 다르다. 09-12에 조사만 하고 중단했던 동일 누락을 재개했다.

설치 retail RefShaderCache에서 GPU-skin BasePass `231a7f149fd8054589dc4baf0825beb0`와 directional Light `da7621e9468b8d4d9d69090a334b5828`을 다시 추출했다. [Gildor 공식 Lost Ark 배포 안내](https://www.gildor.org/smf/index.php/topic,3055.msg41220.html)에서 제공한 v7를 out 전용으로 사용했다. 게임 설치의 SDL dependency만 out에 복사했으며 시스템 설치·Client 실행은 하지 않았다.

`build_vehicle_source_material.py`의 기존 DXBC 생성/packing 경로로 MODEL238 `source.character.kouku-bazooka-dead.v1`을 등록했다. Engine CModel cap238, CShader group237..238, 동일 Python registry, Engine/Client Base/LightGroup237과 dispatch, SourceCharacterMaterialParameters를 연결했다. 기존 group 파일을 사용하므로 새 shader project 항목은 없다. generator의 install anchor는 이전 program의 baked/dynamic 복합 case와 현재 partition blank-line을 허용하도록 국소 수정했다. 기존 shader group의 자동 재정렬은 원상태와 token 일치 확인 후 제거했다.

후보 `bazooka-override.json`은 BossCatalog.modelMaterialOverrides에 정확한 LaserCannon model ID와 materialName으로 들어갈 1행이다. baseTextureMask255/lightTextureMask479, 합집합511의 원본 texture9개를 기존 Resources에서 사용한다. diffuse/normal/specular/emissive, hdr07_1, flat_black, statefx_default, fx_a_ice_003, brdf_beckmann_spec의 원본 expression index와 색 공간을 보존한다. 새 texture나 model binary를 만들지 않았다.

## G05. 실행한 검증

| 검증 | 실제 결과 |
|---|---|
| 후보8문서 native Codec + Stage + Playback | PASS. 네 suit peak5, 정지0, alpha 상한; finite particle sample18812 |
| 블랙홀 hold/explosion | PASS. 12.9초 hold body52/center47, 13.1초 중앙 explosion71 |
| 실제 G2 CModel source preview 준비 | PASS. body/material/idle/yellow clip/bone/weapon/Area, 84 bones |
| 실제 Saydon05 bone 샘플과 역환산 | PASS. 788 matrices, 백스텝 시작 보존 오차≤2.44e-7, rolling 추가16 matrices |
| 바주카 원본 재생성 | PASS. Base EXACT684 lines, Light EXACT625 lines, configure EXACT |
| 현재 SourceCharacter packing 컴파일/소비 | PASS. native238, texture9개, finite constants, required mask511 |
| C++·JSON/project 형식 | 후보 JSON parse, 기존 C++ 인코딩/CRLF 유지, 소유 변경 git diff --check PASS |
| 통합 Engine/Client/Product·shader compile | 상위 통합 담당자 실행. Engine·Server PASS를 통합 담당자에게 전달받았고 Client 최종 빌드 진행 중 |
| 실제 LaserCannon CModel/CMaterial238 | PASS. 최신 Engine Debug DLL + candidate descriptor에서 실제 mesh1 전부 program238 소비, bazooka-model-result.log |
| domain publish, Reload, 사용자 화면 | 상위 통합/사용자 확인 영역. 수행 전 완료로 기록하지 않음 |

검증 source와 로그는 out 하위 native_probe.cpp/native_result.log, rig_probe.cpp/rig_result.log, rig-bones.csv, rolling-bones.csv, bazooka-generated, bazooka-native, bazooka_probe.cpp/bazooka_result.log에 남겼다. 실행·중간 산출물은 소스 commit 대상이 아니다.

## G06. 최종 게시본의 Client admission 검증

상위 통합 담당자의 최종 Owner Publish 완료 뒤 최신 Debug `ValtanPatternAuditionServiceHarness --presentation-generation-admission-contract`를 실행했다. 게시본은 revision2345, 82441행, 23883405 bytes다. 최초 실행은 exit4였지만 baseline acquisition, valid receipt, currentness, Server revision reopen와 exact receipt는 통과했다. 실패4개는 구 binding 경로를 기대한 closure 비교와 semantic drift/missing/duplicate fixture 준비였다.

실제 제품 `ValtanPresentationGenerationAdmission.cpp`와 `EffectV2_Catalog.cpp`, Valtan publisher는 `Data/Valtan/Published/BOSS_VALTAN.effectv2bindings.json`을 사용한다. 하네스만 `Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json`을 기대하고 있었다. 두 파일의 현재 SHA256는 `D844806C775321743501583DA8472630B30236C7EF5DCC27997C639BF1D23BC5`로 동일했다. 내용이나 새 Kouku effect catalog row의 결함이 아니라 구경로 fixture였다.

`ValtanPresentationGenerationAdmissionContractTests.cpp`의 경로 literal 5곳만 현재 typed 게시 정본으로 교정했다. exact closure 비교, semantic drift, missing artifact, duplicate binding 거부와 정확한 진단 검사는 모두 유지했다. 제품 코드와 정본 Data를 완화하지 않았으며 원래 파일의 바이트 인코딩과 줄바꿈을 보존했다.

정상 Debug 증분 Build 및 동일 admission contract 재실행은 모두 exit0/PASS다. 최초 실패 로그 `out/KoukuAuthoring20260925/admission-final.log`는 보존했고 재빌드와 성공 로그는 각각 `admission-retry-build.log`, `admission-final-fixed.log`다. `git diff --check`도 통과했다. Client/UI 실행 없이 실제 게시 bootstrap과 typed presentation closure를 소비했고 fixture 변경은 임시 복사본에만 수행했다. 별도 `--kouku-fixed-damage-contract`는 반복하지 않았다.
## G07. 광역 Server 검사에서 분리한 Valtan fixture 전제

`out/KoukuPattern20260925/server-contract-final.log`의 Counter 4건, 잡기 해제 1건, Ghost sequence 1건을 ABSORB/encounter-wipe diff와 대조했다. Counter fixture의 기본 resource100은 실제 게시 SKILL34580 cost410보다 작아서 Try_Start에서 거절된다. release는 현재 push-only KNOCKDOWN을 NONE으로 기대했다. Ghost는 현재 저장 flow의 Respawn index60/Death index69(마지막)와 달리 Death 바로 다음 Respawn을 필수로 강제했다. 전멸 flag는 기본 false이며 제품의 true 설정은 Kouku Bingo 실패 경로만 소유한다. Counter proxy는 Apply_WorldToPlayer를 거치지 않고 Try_Counter/Apply_PlayerHit를 직접 호출하고, hit-driven Counter도 일반 damage 전에 처리한다. 이번 ABSORB/wipe 변경으로 해당 실패가 생긴 근거는 없었다.

상위 승인에 따라 두 기존 fixture 파일만 교정했다. `ServerGameplayContractTests_ValtanPinnedGeneration.cpp`는 실제 Lance Master runtime profile의 최대 자원을 사용한다. `ServerGameplayContractTests_ValtanLifecycle.cpp`는 실제 arena 중심 부근에서 KNOCKDOWN+pushOnly, 이동/스킬 거절, room tick recovery와 완료 뒤 skill 허용을 검증한다. Ghost는 실제 저장된 다음 cursor/terminal idle 경계를 검사하고, 별도 메모리의 Death→Respawn sequence로 이전의 delay suppression, Debug hold 보존, next-tick Respawn과 phase3 전환 검사를 유지한다. 제품 코드와 정본 Data는 변경하지 않았다. C++ 바이트 인코딩/줄바꿈 보존 및 diff --check PASS이며, 통합 컴파일과 focused 재실행 결과는 상위 검증 완료 뒤 확정한다.

## G08. Bingo independent-flow fixture correction

The final Raid log exposed five stale whole-Bingo Parent assumptions. The first published entry is now the independent P129 board initializer, and the saved gate contains 35 entries with the loop beginning at stable entry `kakulsaydon.flow.bingo.independent.30`. P107 remains the small three-child special Parent. Product code and authored data were not changed for this fixture correction.

`ServerGameplayContractTests_KoukuRaid.cpp` now derives the exact stable-entry completion sequence from all saved entries plus one full repeated tail. It verifies terminal completion receipts, request targets, raid epoch and encounter-owned board identity. The four-player case also advances the actual first-mark and repeat clocks through marks three and six, checks the published flattened P107 stage clocks and one detonation, and verifies replay of the interrupted normal entry after each special. Ordinary hit/hammer attrition and boss HP depletion are isolated in the scheduler fixture; failed Bingo judgement still uses its production encounter-wipe path and fails the test. Fake connection queues are drained because the fixture has no transport send worker.

The shared-file standalone vote changes from logic_tuning were retained. Byte encoding and CRLF were preserved; `git diff --check` passed. Product9 compilation and the focused Raid execution remain pending. The later user request to manage the normal flow and P107 under a common authoring bundle is a separate product/data change; the order, loop and special-interruption checks must remain when that ownership representation is updated.


P107 projection was re-measured before the Product9 build. Its authoring Parent has `playChildrenSequentially=false`; the runtime has no `ParentChildren`. The ten published stage durations are 5400, 867, 2000, 2167, 1394, 13000, 200, 1167, 5100 and 1333 ms. Their total is 32628 ms; the optional runtime tail duration may remain zero under the existing stage-sum contract. Medusa gaze starts at 8567 ms, the 13-second hold starts at 11828 ms, and detonation starts at 24828 ms. The whole-pattern line window requires three completed lines and grants 30000 ms player invulnerability.

The fixture now resolves `gate.strBingoSpecialPatternId`, validates those actual projected definitions, and observes all ten runtime stage boundaries against the single fixed timeline. Each special must emit exactly one 13-bar outgoing boss damage event on tick 745 and finish on tick 979 relative to its actual start. It then checks return to the interrupted stable flow entry. This replaces the incorrect runtime-three-children assertion without weakening the source order or special-clock checks. The new explicit special reference is supplied by the separate product/publisher change. Compilation and native execution remain pending.


## G09. Product10 focused Server validation

After the root coordinator confirmed revision2346 publication (82442 rows, 23883456 bytes), the Product10 Debug Server ran `--valtan-lifecycle-contract-test` and `--skill-stages-contract-test` concurrently against `Server/Bin/DataFiles`. Both exited zero with `failures : 0`. Original outputs are `out/KoukuPattern20260925/server-valtan-focused-final.log` and `server-skill-stages-final.log`; sibling `.exitcode` files record zero. The Valtan selector includes both lifecycle and pinned-generation contracts, so all four Counter paths, captured-player release/recovery and both Ghost sequence tests executed successfully. Its two expected `RoomRuntimeFailure` diagnostics belong to rollback rejection tests and are followed by PASS assertions.

Product9 first caught an invalid direct dereference of the fixture's weak session pointer. Root corrected the outbound drain to lock the weak pointer; Product10 compilation passed. No Client or UI was launched. These focused passes do not replace or erase the earlier broad-suite failure logs. The separately running Bingo Raid special-insertion failure remains under investigation and is not reported as a pass here.


## G10. Bingo third-mark admission regression and diagnostic coverage

The revision2346 Raid run passed the full independent prefix/repeated tail and the flattened special-definition check, then failed the special-execution assertion. The logged zero marks were observed after cleanup; they did not prove the third mark had never occurred. Code review found that `Start_KoukuBingoSpecialPattern` passed the actual running world entities as a staged preflight. `Evaluate_KoukuSaydonPatternAudition` rejects a staged boss that still owns a nonempty pattern, so insertion into an active normal occurrence could return BUSY and abort the Raid. Root confirmed this finding and delegated the product fix to server_patterns: preflight a detached copy of the exact owned target after the existing abort operation, preserving the live world until successful admission.

The Raid fixture now reports the retained Raid reason, phase, failed tick, advance failure, first special condition failure, stage/detonation counts and ownership/request flags. It also deliberately supplies a mismatched source revision while a normal entry is active and verifies rejected special admission preserves all live actor transforms, HP, actions and pattern clocks; flow cursor; audition epoch/request/lifecycle receipt; board masks/owner/epoch/bomb clock/count; and next audition epoch. The actual third/sixth mark, exact flattened timing, single explosion and resumed-entry checks remain unchanged. Source diff checking passed; Product11 compilation and focused Raid re-execution are pending under root coordination. No authored or published data was changed by this fixture addition.
