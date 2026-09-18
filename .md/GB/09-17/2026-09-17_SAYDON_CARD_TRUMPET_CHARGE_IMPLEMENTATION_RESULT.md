# 세이튼 카드·트럼펫·공과 돌진 잔상 구현 결과

## 현재 상태 — 후속 수정 등록 및 Publish 완료

최초19파일 등록1224 이후 사용자가 저장한 최신1232를 기준으로 후속12파일을 병합·설치했다.
현재 Composition1233, WorldSequence2036이며 정식 KoukuSaydon owner Publish의 네 domain이
모두 통과했다. 카드 출력의 축 회전, 주사위 크기·연속 궤적, 낙하 공·상단광, 서버 추적 카드,
전체 공 분열 그룹과 오른손 지팡이 트레일을 반영했다. 피자는 추가 원형을 제거했으나 사용자
이미지의 내부 세 공백 해소는 확정하지 않았다. 후속 검증·등록 증거는 G18~G21을 따른다.
G07~G16의 등록 대기는 당시 기록이다. Client·Server 제품 빌드와 사용자 화면 확인은 수행하지 않았다.

## 이전 재검토 경위 — 사용자 V1 오류 후 정정

아래 G00~G06은 첫 구현·검증 시점의 기록이다. 새 파일 등록과 CPU codec 성공을 실제 Effect V1 Play All 성공으로 확대 해석하면 안 된다. 사용자 실행에서 트럼펫의 projection 거절, 회전 카드 통합 emitter 부재, 잔상의 V1 항목 부재, 요소 삭제 시 stale preview ID가 확인됐다. 셰이더 빌드 최적화는 사용자 우선순위 변경으로 중단했다.

후속 source 수정과 out 후보는 아래 G07 이후에 기록했다. 당시 사용자가 Effect V1·쿠크 패턴에 미저장 편집이 있다고 명시하여 live 등록을 보류했다. 이후 저장·종료 확인을 받아 최신 revision에서 필요한 항목만 병합했다. 실행 중인 Client에는 소스 변경이 자동 반영되지 않으며 사용자 빌드·화면 확인은 여전히 별도다.

## G00. 반영 범위와 보존

기준은 `bddacace2296c1c38e016146a13fcd767cdf82da`와 기존 dirty 작업 폴더다. 기존 shader 분리 작업에 필요한 줄만 병합했고 사용자 수정 `sk_06_7`, `sk_13_1_loc_int`, `sk_13_3_loc_int`의 diff는 세 파일 모두 세션 시작 baseline과 바이트 단위로 같다. 기존 World object/template/instance payload는 보존하고 새 항목만 추가했다. 최초 상태는 `out/sayton-pattern-20260917/session-baseline.patch`와 `session-baseline-status.txt`에 남겼다. 다른 변경을 stage/commit하거나 되돌리지 않았다.

첨부 이미지 다섯 장을 열람했다. 상단 다이아, 네 카드 무늬, 접촉 폭발, 흰 몸체 잔상, 8방향 레이저를 진단 입력으로 사용했다. 원작 데이터 수치, 사용자가 지정한 타이밍·무한 수명, 프로젝트 표현값을 구분한다. 에이전트의 Client/UI 실행·조작·캡처와 visual PASS는 없다.

## G01. 이름과 패턴 타이밍

새 V1 Effect 12개, 총 221 elements를 Authored·catalog·tree·Composition·프로젝트에 등록했다. 카드 짝 맞추기에는 `주사위 다이아 폭발`, `하트`, `클로버`, `다이아`, `스페이드`, `카드출력 이펙트`, `카드폭발`을, 회전 카드에는 독립 무늬 네 종을, 트럼펫에는 `트럼펫_카드장판`을 등록했다. 정의 목록은 `out/sayton-pattern-20260917/effects/installation.json`이다.

| 실제 패턴 | 연결한 동작 |
|---|---|
| P78 `세이튼_카드맞추기_주사위패턴` | stage_4 기반 주사위·다이아·폭발 통합, 6114ms 입 출력과 카드 4개 생성 |
| P48 `세이튼_빙글빙글돌며카드던지기` | 3533/3833/4133/4433ms에 3개씩 총 12개, 4종 무늬 pool, 원작 속도와 사용자 요청의 랜덤 발사 방향 |
| P47 `세이튼_트럼펫장판소환` | 4184ms 8방향 레이저와 sk_06_9 시작, 6637ms 바닥 구간 종료와 sk_06_2 폭발 |
| P81 `세이튼_공굴리기_카운터` | 1696ms부터 기존 Mario 공 기하를 이용한 World object 생성·회전 |
| P79 `세이튼_공먹고휘리릭_노란장판펑펑` | 1172~2172ms 작은 공의 실제 b_wp_2 부착, 3596ms 지팡이 회전 Effect |
| P80 `세이튼_돌진_카운터` | 원작 22_01/22_04 TrailGhost notify 이후 Server 재생에 실제 골격 자세 잔상 |
| `VALTAN_DASH_CHARGE` | 저장 이름 `3회 땅 치기 후 돌진`, Server 돌진 active 구간의 몸체 골격 잔상 |

원작 추적 카드의 속도 30m/s, 접촉 반경 .5m와 회전 카드의 15m/s, 반경 .4m, 3turn/s를 사용했다. 원작 추적 수명 10~20초를 사용자 요청에 따라 0(무한)으로 바꿨다. 회전 카드는 현재 사용자가 저장한 1회 spin 체인을 유지해 12발이고 각 발의 수명은 1000ms다. 원래 다른 spin/후속 clip을 임의로 추가하지 않았다.

주사위 통합 Effect의 원작 anchor clip은 `rpct00_att_battle_11_02` 0~5167ms이며 손 notify900ms, 상승2423.504ms, 낙하3900ms, 착지와 위쪽 심볼4161.037ms, 폭발5161.416ms로 연결했다. 사용자가 말한 “다이아 mesh”는 확인한 원작에서 `usemesh=false`인 `fx_d_symbol_109` sprite 세 장이다. 원작대로 그 carrier를 사용하며 별도 diamond mesh를 임의로 만들지 않았다. 심볼 위치(242,0,130)cm는 dice 위치(240,0,35)cm보다 위쪽이다.

회전 카드의 `fx_l_symbol_45` 2×2 atlas는 클로버0·다이아1·하트2·스페이드3이다. 고정 재생 seed와 원작 random subimage의 조합이 같은 타일을 반복하므로 네 파생 문서에서 각 타일을 명시했다. 사용자가 수정한 원본 emitter의 위치와 원래 남아 있던 elements는 보존했다.

P47의 시작 기준 presentationOccurrences가 비어 있어 패턴 Play에서 기존 8방향 asset을 발생시키는 연결이 없었다. 이번에는 원래 native 40 elements의 emitter/socket/rotation/scale을 보존하고 4184ms 발생을 연결했다. 실제 CPU 재생의 4.3초에서 공통 중심 약(-.000136,6.11539,.452431)m와 45도 간격8방향·32개의 native trail 층을 확인했다. 이는 standalone에서 안 보인다는 증상의 모든 GPU 원인을 확정한 결과는 아니다.

원래 floor particle 수명1초로는 6637ms까지 유지되지 않아 요청한 2453ms 구간에 맞춰 sourceScale.lifeTime을2.453배로 조정했다. 실제 6.6초에 floor15 particles,6.7초에 floor/laser0·impact150을 확인했다. standalone 통합 Effect의 tail 끝은10637ms지만 기존 P47 clip chain은9233ms다. 현재 제품 Pattern은 그 경계에서 종료되어 impact를2596ms 재생하며 마지막 source tail1404ms 전체를 보존하지 않는다. 기존 clip chain을 임의로 연장하지 않았다.

## G02. 실제 소비자 연결

Composition의 `DURATION / PURSUIT_PROJECTILES`를 Client parser·validator·serializer·Logic 편집기에 연결했다. Effect resource 1~4개와 접촉 Effect, 속도·접촉 반경·생성 반경·발사 간격·개수·수명·homing을 저장한다. active/contact는 실제 `V1_EFFECT` resource만 허용한다. parse·validation·atomic save 실패에서는 이전 문서를 보존한다.

Server는 기존 CombatObject 경로에서 pattern aggro 대상 플레이어의 위치를 매 tick 추적하고 Shared XZ swept circle로 접촉을 결정한다. 카드는 자연 animation 종료 뒤에도 남으며 접촉·대상 사망/퇴장·owner 소멸·명시 Stop에서 정리된다. 피해량은 추가하지 않았다. 자세한 Server 계약과 수치 검사는 `2026-09-17_KOUKU_PURSUIT_SERVER_RESULT.md`가 소유한다.

Client는 pinned targeted visual ID의 반복 Effect를 Server spawn/snapshot 위치와 yaw로 재생한다. 접촉은 reliable HIT_PULSE의 검증된 위치에서 공유 카드폭발을 재생하고, 뒤따르는 카드 despawn과 폭발의 자연 수명을 분리한다. 준비 중에는 복사한 event·정의가 살아 있으며 중복 burst를 차단한다. 기존 우선 준비 queue로 active/contact 자원을 미리 준비한다. Showtime의 경로와 packet 종류는 유지했다. Client가 이동 또는 충돌 정답을 만들지 않는다.

공 두 개는 기존 WorldSequences 소비자를 사용한다. 굴리는 공은 source static mesh의 1.05turn/s를 -378도/초로 연결하고 원래 Mario 모델의 위로 치우친 pivot을 보정했다. 설치 WModel의 bone basis 1.7을 실측해 rolling scale1.87, 작은 공 scale1.36을 사용했다. 굴림은 원작 static particle의 회전이며 존재하지 않는 skeletal rolling clip을 추가한 것으로 기록하지 않는다.

## G03. 돌진 잔상과 복원 경계

새 `CSkeletalAfterimage`는 기존 `CModel`의 실제 world와 skin palette를 복사해 최대6개, 50ms 간격, 250ms 수명으로 제출한다. 세이튼 body와 skinned 지팡이, 발탄 body를 사용하고 live world/palette를 복원한다. 숨김·model 교체·teleport·긴 시간 불연속과 종료에서 history를 정리한다. 새 H/CPP를 기존 프로젝트/filter에 등록했다.

원작 TrailGhost notify 시각은 확인했으나 원본 native TrailGhost shader ABI 전체는 복구하지 않았다. 흰색·alpha·fade·rim 값은 프로젝트 표현이며 source-exact Full Restore 완료가 아니다. 발탄의 별도 static 도끼·armor는 이번 골격 잔상에 포함하지 않았다. 상세 근거와 시험 범위는 `2026-09-17_BOSS_CHARGE_AFTERIMAGE_IMPLEMENTATION_RESULT.md`에 있다.

굴리는 공은 기존 Mario 모델과 원작 공의 기하가 일치하지만 현재 Mario의 MN_PPCC 재질을 재사용한다. 원작 WP_MN_PPCT 재질과 같다고 판단하지 않았다. 작은 공은 기존 World 모델의 원작 RH 재질을 사용한다. 트럼펫·카드·다이아의 화면 표시와 밀도·크기·색은 사용자 육안 확인이 남아 있다.

## G04. 실행한 검증

| 검사 | 결과와 증거 |
|---|---|
| 실제 Server/Shared 컴파일·링크와 동작 검사 | 138 checks, failures0; `out/KoukuPursuit20260917/server_result.log` |
| 신규 projector/PowerShell 및 Showtime 회귀 | 6개 PASS; Server RESULT의 범위 참조 |
| 실제 Composition codec | 최신 저장 데이터 Parse, exact roundtrip, 무한/유한 수명, 잘못된 resource·수치 거절, parse rollback, atomic save와 외부 수정 보존 등 16 checks, failures0; `out/sayton-pattern-20260917/composition-probe.log` |
| 실제 Effect codec와 파티클 CPU 재생 | 새 12개 Load/Validate_Drawable/Serialize+Parse PASS; 회전 네 종을 Stage_Document·Seek(.5초)하여 native2999 particle의 atlas0/1/2/3 확인; `out/sayton-pattern-20260917/effect-probe.log` |
| 트럼펫 실제 CPU 시간 샘플 | 4184ms 이전 무생성,8방향 native trail,6637ms 전 floor 유지·후 impact 교체 PASS; `out/sayton-pattern-20260917/effect-timing-probe.log`의 GPU83은 CPU GpuOccurrences 기술자 수이며 GPU draw 증거가 아니다 |
| WorldSequences 정식 배포 | 동일 Scope WorldSequences Validate 및 Publish PASS; 출력 `Client/Bin/DataFiles/Map/LV_LUT_MIDNIGHTC_ED.worldsequences.json` SHA256 `f10e332392e370771618ee96d2664d98796e7a98e2919762ef53b58115628a42` |
| 골격 잔상 focused native probe | 317 checks, failures0; 실제 helper의 queue·copy·정리·실패 복원 검사이며 CModel/GPU 경계는 stub; 개별 RESULT 참조 |
| 프로젝트 XML 및 whitespace | Client.vcxproj와 filters XML parse, 전체 git diff --check PASS |

Effect CPU probe는 현재 header와 실제 34개 codec/playback 의존 TU를 같은 설정으로 새로 컴파일했다. 초기 검사에서 오래된 object와 새 header를 혼합해 생긴 runtimeExtensions 오검출은 성공 근거에서 제외했다. 게시 후 변경 JSON22개 syntax parse와 전체 diff check도 통과했다.

첫 정식 Kouku publish에서는 저장된 정상 Pattern 수가 기존64 상한을 넘어 거절됐다. 원래 Shared의 pattern flow slot 계약255에 맞춰 Python projector, Gameplay publisher, Client Boss Tool의 상한 세 곳을 일치시켰다. 기존 Pattern을 삭제하거나 검증을 우회하지 않았다. 64·65·255 수용과256 거절의 boundary 검사를 통과했다.

수정 후 실제 full prepare는 sourceRevision1186에서 saved83 → ready Product69를 만들었고 P47/P48/P78/P79/P81이 모두 포함됐다. 기존 P32는 stages가 없는 미완성 상태 그대로 별도 unavailable이다. read-only 검사에서 source bytes 보존을 확인했다. 정식 Kouku publish도69 patterns·408 stages·8 bundles·2 outputs로 통과했다. 증거는 `out/KoukuPursuit20260917/publication-readiness.json`이다. 출력 정본은 `Data/Encounters/KoukuSaydon/KoukuSaydonEncounter.json`과 Client가 CProjectDataRoot로 직접 읽는 `Data/Animation/Authored/KoukuSaydon/KoukuSaydon.patternbindings.json`이다.

이후 `Publish-GameplayBalance.ps1 -Mode Publish`도 성공했다. 생성한 `Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap`에서 추적2행→Client visual8개·공유 폭발의 정확한 join과 World2행(P79 1172/1000ms, P81 1696/7568ms)을 직접 확인했다. SHA256은 `3bc377bd44ebb2fee9f647f82c56b33c115d0b080a5c20dc427f9a74e83fa5f2`이며 `out/sayton-pattern-20260917/data-final-validation.json`에 기록했다. 데이터 게시의 미완료 단계는 없다.

첫 정상 증분 Product Build는 Engine·Shared·Server를 통과하고 Client의 기존 dirty shader 분리 경로에서 FXC의 macro include 미확장(X1500) 오류가 발생했다. 연쇄 Has_EffectArtistNativeProfile 미정의는 재질 함수 자체의 결함으로 판단하지 않았다. 기존 loader와 generator를 wrapper의 literal include 방식으로 맞추고 두 carrier의 중복 common 입력만 guard했다. 기존 cohort 격리 테스트6개를 수정 없이 통과했고 material/ABI/selected-group 파일30개는 이전 생성 결과와 같다. MeshArtist448·ParticleKouku2304의 실제 FXC가 통과했다. 근거는 `out/BossChargeAfterimage20260917/artist-fxc-baseline/fix-scope.json`이다.

수정 후 `Invoke-BuildAndRegression.ps1 -Configuration Debug -BuildLogDirectory out/sayton-pattern-20260917/product-rebuild`가 **Product PASS**로 끝났다. Engine·Shared·Server·Client 전부 PASS이며 compile/link 오류는 없다. 공식 결과는 `out/BuildPipeline/runs/20260916T180811842Z-debug-product.json`, 로그는 `out/sayton-pattern-20260917/product-rebuild.log`다. 기존 native shader의 X4000/X4008와 C++ C4819 경고는 남아 있다. 이전 실패 로그는 product-build.log에 보존했다. Clean/Rebuild 또는 중간 산출물 삭제는 하지 않았다.

마지막 빌드 중 VS의 별도 Framework.sln 빌드도 관찰했다. 다른 VS process를 중단하거나 조작하지 않았다. Product 결과의 tracking identity changed 표시는 그대로 보존하고 위 에이전트 빌드의 PASS와 실제 파일 갱신을 별도로 확인했다. 산출물 개수는 공유 폴더 관측치이므로 에이전트만 생성한 개수라고 주장하지 않는다.

## G05. 사용자 실행 경로

세이튼은 `Lobby → KoukuSaydon → F1 → Open KoukuSaydon Boss Tool → Gate: 1관문 → All Patterns`에서 위 표의 패턴을 선택하고 `Play Isolated`를 누른다. 중지는 같은 창 `Pattern Flow → Stop Playback`이다. 생성된 카드 추적·접촉은 이 Server Play에서 확인한다.

발탄은 `Lobby → Valtan → F1 → Open Valtan Boss Tool → Boss Verification → All Patterns → 3회 땅 치기 후 돌진 → Play Selected Pattern (Keep Arena)`이다. 반복 중지는 `Stop After Current`다. 원래 요청의 “3연 구르기”와 저장된 패턴 표시명을 구분했다.

세션 시작 LAN sync는 server-host·방화벽 준비 완료·192.168.0.14:7777 not-listening이었다. 03:08 KST 최종 확인에서도 Client와 Server는 실행 중이지 않고7777 연결은 false였다. 에이전트는 Client/UI를 자율 실행하지 않았다. 새 gameplay가 적용되려면 사용자가 Server를 실행한 뒤 Client로 입장해야 한다.

실행 파일은 `Server/Bin/Debug/Server.exe`(02:38:48 KST, SHA256 `58d04c9bab1e37fcc45884d19a8c267ed522c8a519f68289ccb071a71183aaf4`)와 `Client/Bin/Debug/Client.exe`(03:08:08 KST, SHA256 `e3786f1ac59e0095f62d469d723582de7c631084d01d19a24475b33f69a4b01d`)다. 설치된 기본 animated shader에도 새 pass14가 포함된 컴파일 출력이 있으며 파일 시각은02:52:56 KST다. 무빌드 실행은 해당 exe를 직접 실행하는 경로이고 F5/Ctrl+F5를 무빌드 실행으로 설명하지 않는다.

남은 것은 사용자의 실제 화면 판정이다. 카드4종의 노출·무한 추적과 접촉 폭발, 주사위 착지 뒤 상단 다이아, 트럼펫 8방향과 바닥→폭발 전환, 공 회전·작은 공 부착, 세이튼/발탄의 흰 잔상을 위 경로에서 확인한다. G01/G03의 원작·프로젝트 표현·Pattern tail 경계를 유지하며 source-exact Full Restore 또는 visual PASS로 승격하지 않는다.

## G06. 사용자 입장 실패 재현과 잔상 pass 등록 수정

위 G04의 Product PASS 뒤 사용자가 실제 KoukuSaydon 입장에서 `loading.target-resource-load / [Loader] KoukuSaydon: server-approved character rendering` 오류를 보고했다. 첨부 화면을 열람했고 실제 설치된 CSO와 Engine.dll을 사용한 기존 headless WARP probe에서 `CShader::Create` 실패를 재현했다. 이전 FXC·Product 성공은 이 기본/파생 shader admission 검사를 포함하지 않았으며 실제 입장 성공 증거가 아니었다.

실패 경로는 `CLoader::Ready_Character_Rendering → Ready_AnimatedMeshShader → CShader::Create → Stage_ProgramVariants`다. 추가한 ChargeAfterimage pass14를 기본과 여섯 SourceGroup 모두 BASE(1)로 등록했지만 기존 검증은 기본 BASE의 파생 counterpart가 UNAVAILABLE(2)인 계약을 요구한다. 실제 FX reflection에서 pass 수15·이름·IA signature336 bytes는 일치했고 여섯 group의 pass14 정책만 불일치했다. 서버 승인 이후 공용 캐릭터 rendering prototype을 준비하다가 실패한 것으로, 서버 연결을 우회할 문제가 아니다.

pass14를 기존 `BINARY_ANIMATED_NATIVE_PASS_POLICY`에 연결하고 기본 FX에만 afterimage PS를 컴파일하며 파생 PS는 NULL로 유지했다. 기존 엄격 validator, source 재질 수식, 잔상의 색·fade·기하와 카드/패턴 데이터는 변경하지 않았다. 기존 `SourceCharacterShaderVariantProbe.cpp`의 미커밋 내용을 보존하면서 파생 pass14 직접 실행 거절과 stale program1/9/18/88 아래 shared clone의 base PS·color/fade·bone 상수 유지 검사를 추가했다.

실패 증거는 `out/EntryCharacterShader20260917/probe.log`와 `reflect.before.log`다. 새 파생6개와 이전 base(동일 base 정책/PS)의 중간 확인은 실제 CShader 생성과126 programs·6 clones·9 failure cases·504 light passes·144 native passes·4 afterimage passes·10 unavailable passes를 통과했다(`after-group-fix/probe.log`). 이 중간 결과는 새 base까지 배포된 최종 Product 검사와 구분한다. window0·draw0이며 Client/UI 실행·조작이나 화면 캡처는 없다.

사용자가 VS 빌드 중이며 빌드 오류와 실제 진입은 직접 검증하겠다고 명시했다. 중복 컴파일을 줄이기 위해 부모 chain이 이번 Product인 agent 소유 FXC만 확인 후 중단했고 사용자 VS의 FXC·Tracker·MSBuild는 조작하지 않았다. 이번 Product 로그의 MSB6006/exit-1은 이 의도적인 중단 결과이며 새 소스의 컴파일 오류로 기록하지 않는다. `out/KoukuEntryAfterimage20260917/product-build.log`와 `out/BuildPipeline/runs/20260916T182858317Z-debug-product.json`에 남아 있다. 이 실행의 출력은 파생 CSO6개·OBJ0·binary0이며 최종 Product PASS를 주장하지 않는다.

최종 base CSO 빌드·Client 재실행·Lobby → KoukuSaydon 진입과 잔상 화면은 사용자에게 인계했다. 위 중간 WARP 성공은 실제 실패한 shader admission이 회복되는 것을 확인한 범위다. 사용자 빌드 중 shader와 프로젝트 입력을 추가 수정하지 않는다. 빌드 시간의 별도 최적화는 현재 로그와 기본 animated shader의 native program 포함 범위를 읽기 전용으로 조사하며 이번 입장 수정의 완료 범위에 섞지 않는다.

## G07. 트럼펫 projection과 선택 삭제·앵커 편집 수정

트럼펫의 `Document-owned runtime carrier kind is unsupported`는 CASCADE_BEAM_V1을 이미 존재하는 직접 Playback 경로 대신 supplemental projection으로 보내던 분기에서 발생했다. Beam만 있는 문서는 직접 소비하고, Beam+Ribbon/baked 문서는 Beam 원문을 유지하면서 변환이 필요한 요소만 projection한다. 기존 TypeData/target/carrier 검증과 실패 rollback은 유지했다. 설치된 실제 트럼펫으로 기존 오류를 재현한 후 실제 codec/projection/playback의42검사,32 Beam/8방향, 세 혼합 문서와 invalid 입력 거절을 통과했다. 증거는 `out/sayton-pattern-20260917/beam-projection.log`다.

사용자 첨부 오류의 `saydon.410cec11edc8eea226f28eb9`는 다이아의 Mesh Particle ID다. 삭제 문서는 이미 그 요소를 제거했지만 활성 Sequencer transient의 previewElementIds는 이전 선택을 유지해 restage가 거절됐다. staged transient만 현재 요소 ID와 교차하고 남은 선택이 없으면 남은 전체 Effect로 전환한다. legacy isolation 목록도 정리하고, 선택·marks는 commit 성공 뒤에만 비운다. 재생 stage/sample 실패 시 문서와 양쪽 preview 선택이 유지된다.

Following bone 그룹에 `Anchor Position (bone-local m)`와 `Anchor Rotation (deg)`를 추가했다. 전체 요소의 공유 SocketLocalTransform을 수정하므로 소켓 원점에서 모든 요소를 함께 돌리고, 각 Element의 위치·회전·native recipe는 그대로 보존한다. 기존 평균 중심 회전은 `Element Group Rotation (about center)`로 구분한다. 같은 runtime slot을 별도 그룹이 공유하는 충돌은 부분 변경 전에 거절한다. 현재 사용자 카드출력의 저장 offset/rotation은 변경하지 않았다. 실제 입 위치의 최종 판정은 사용자 화면 확인 범위다.

변경 Sequencer/Playback/Editing/Helpers/Detail 5개 실제 TU는 out 격리 Debug 컴파일을 통과했다. 현재 source 함수로 만든29검사는 삭제 성공/전체선택 소진/실패 rollback/미적용 detail 보존, 앵커 전체 회전과 요소 상대값 보존, NaN/source-contract/공유 slot 충돌 거절을 확인했다. GPU staging은 stub이고 UI 실행 검사가 아니다. 로그는 `out/EffectV1Review20260917/editing_probe.log`, source hash는 `editing_probe.sources.json`이다. 전체 Product 빌드·Client/UI 실행은 수행하지 않았다.

## G08. 회전 Sprite emitter 후보와 원본 속도 정정

새 후보는 `effect.kouku.card.spinning.emitter` / `회전 카드 발사`다. 기존 sk_13_1의 native2999 symbol Sprite 1개와 공용 카드폭발 death-event receiver 7개만 사용하며 새 상위 emitter runtime을 만들지 않았다. 원본 4칸 random SubUV와3회전/초를 유지하고 기존 velocity cone으로 수평 무작위 방향을 만든다. 현재 사용자 clip의2833~7633ms 회전 구간에서3533~7433ms 300ms 간격·wave당3장, 총42장을 낸다. 원작 notify는700/1000/1300/1600ms의12장이고 이후30장은 사용자가 늘린 clip 창에 맞춘 프로젝트 연장이다.

실제 최신 codec/Playback Stage→Update에서42장·4문양·39개 방향·수평8m/s·최대21장 동시생존·death event42×29=1218폭발 particles·10.208초 이후 전부 정리를 확인했다. 원작3turn/s도 실제 sample90→2106도로 확인했다.15개 resource가 설치되어 있다. 후보는 `out/EffectV1Review20260917/candidate`, 로그는 `cpu/spinning-probe.log`다. catalog/tree 등록 행은 spinning-emitter-schedule.json에 있으며 installed=false다. 이 standalone preview의 폭발은 거리15m에 해당하는 입자 수명 종료를 표현하고 플레이어 접촉 판정은 Server가 소유한다.

원본 projectile class reflection의 필드 순서는 ResScale / CollisionSize / Height / Speed / MaxSpeed / Lifetime / MaxDistance다. 이전 G01의15m/s·1초 표기는 정확하지 않았다. 회전 카드421981901은 Speed800cm/s·MaxSpeed1500cm/s·Lifetime5초·MaxDistance1500cm이며, 원본 가속식은 확인되지 않아8m/s 정속·15m 도달 종료로 소비한다. 추적 카드421980901~904도 Speed300cm/s이므로 기존30m/s는3m/s로 정정할 후보를 만들었다. 사용자 지정 무한 수명은0/거리cap0으로 유지한다.

Server/Client/publisher의 optional maxDistanceM 계약과 정확한 거리 종료·공유폭발 연결은 source 반영됐다. 실제 Server147checks, Client codec23checks, Python/PowerShell3tests를 통과했다. 현재 편집 중인 최신 revision을 기준으로 `out/KoukuPursuit20260917/card-motion.patch.json`에 expected/proposed 변경만 남기고 live Composition은 쓰지 않았다. 상세는 같은 날짜 Kouku pursuit RESULT에 기록한다.

카드출력의 원본 FX_Prj_01은 실제 socket26 / b_wp_1 / [32,0,-15]cm / 회전270도다. 별도 head socket FX_Prj_02와 구분되며 현재 변환은 원본값과 일치한다. 다만21_02 clip에서 시각적으로 입 중심에 오는지는 아직 확인되지 않았다. 원본 socket을 임의 교체하거나 사용자가 현재 조정한 Element offset을 덮어쓰지 않고 G07의 공유 앵커 편집을 제공한다.
## G09. 잔상 V1 후보와 최종 검증·적용 대기

Model Cue의 optional afterimage를 기존 CModel/CSkeletalAfterimage에 연결했다. V1은 현재 본체를 source preview에서 그리고 cue는 과거 skin palette/world만 그린다. 샘플 간격·수명·개수·notify 시작 창을 저장하며, Stop/숨김/명시 seek는 history를 지우고 재생 후 다시 쌓는다. 모델마다 다른 root suppression axis/scale를 저장하고 발탄의 별도 animationSetAssetId를 기존 CModel::Attach_AnimationSet으로 연결했다. source notify 외 색·fade·표본 정책은 PROJECT_AUTHORED다. V1 후보는 몸체만 포함하며 Server의 기존 몸체/무기 잔상 경로와 구분한다.

후보는 `out/EffectV1Afterimage20260917/candidates`의 세이튼 돌진카운터·발탄420604 stage003 두 문서다. EffectCatalog/EffectResourceTree와 발탄 FullRestoreAnimations의 등록 행도 함께 준비했다. 설치 실제 CModel로 세이튼 두 clip·발탄 body+animation donor를 만들고 원본 notify 시각의 총1,941 skin palette 행렬 finite를 확인했다. 설치 CShader pass14와 stale selector89의 admission도 통과했으며 창·픽셀 검사는 하지 않았다. `model_probe.run.log`가 증거다. helper333검사, 최종 descriptor ABI로 다시 컴파일한 codec/Playback24검사와 변경 renderer/helper/UI TU 컴파일도 통과했다.

마지막 공통34TU를 현재 animationSetAssetId 포함 header로 새로 컴파일한 뒤 회전카드42발·4문양·폭발, Beam42검사, 잔상24검사를 다시 통과했다. 삭제/앵커 독립 검토에서 추가 blocker는 없었다. 앵커 UI의 ImGui ID는 socket 값 대신 stable Element ID를 사용해 드래그 중 값 변경으로 조작이 끊기지 않도록 했다.

현재 상태는 **source 수정·out 후보·격리 검증 완료, live 등록·최신 Product 빌드·사용자 화면 확인 대기**다. 사용자 미저장 편집이 있고 현재 Client/Server가 실행 중이므로 에이전트가 종료·Reload·Data 덮어쓰기를 하지 않았다. 저장·Client 종료가 확인되면 최신 Composition revision/hash를 읽어 회전카드/Logic 후보를 재생성하고, 신규3효과의 catalog/tree/프로젝트 None 등록 및 발탄 animation sidecar, Logic73/74의 semantic patch만 병합한다. 그 뒤 정식 Kouku/Gameplay publisher를 실행하며 이전 snapshot 전체 파일로 덮어쓰지 않는다. 빌드와 최종 시각 판정은 사용자가 직접 한다.


## G10. 공먹기 타깃 착지와 훌라후프 초기 높이

소스 연결과 격리 검증을 완료했다. 기존 ALBION_AIRBORNE의 여섯 phase와 25+7열 bootstrap 계약을 그대로 사용한다. JUMP의 0 ms만 즉시 초기 높이로 정의했으며 양수 상승은 기존 보간을 유지한다. Client의 특정 Albion clip명 제한은 실제 native pose/남은 하강 검증으로 대체했다. SLAM은 착지 source Stage에서만 XZ를 고정하고 Y 하강을 소비하며 다음 Stage의 원본 root 이동·상승을 재개한다. 새 packet이나 별도 이동 runtime은 없다.

- **P79**: logic75(7148 ms)는 기존 Albion 13.6788133052 m를 200 ms에 상승한다. logic76(8937 ms)의 출현은 같은 시각의 기존 logic53 선택과 logic70 착지를 합성해 한 플레이어 ID/위치를 사용한다. stage5 시작7200 ms에 animation startOffsetMs3099를 더한 실제17_05 재생창은10299~12966 ms다. 그 전에는 높이를 유지하고 남은18.6698725762 m 하강을 현재 높이에서 지면까지 정규화한다. 원본 lateral root 약1.02725 m는 착지 타깃 XZ를 옮기지 않는다.
- **P84**: logic77의 실제 저장 이름은 `쿠크_훌라후프_상단시작`이다. 0 ms부터11.1910782640 m 높이에서 시작해 stage10 `13_start`의 하강을 소비한다. 사용자 지칭 stage11 `13_loop`의 원본 하강은약0.005624 m이며 큰 하강의 소유자는 stage10이다. stage12 `13_end`의 원본16.7441587758 m 상승은 유지한다.
- **편집 보존**: Composition1206/SHA256 `edcf294d7424b17c27f0e276270c40de4898cefcba396bd3a7516a91df6f890e`를 기준으로 out 후보만 생성했다. P79의 빈 stage4 33 ms는 후보에서 이전 stage hold에 흡수하며 기존 animation/effect/world/logic의 전역시각과 전체길이가 같음을 assertion했다. live Data는 쓰지 않았다.

후보 재생성은 `python out/SaydonAirborne20260917/build_candidate.py`다. `airborne.patch.json`에는 source revision/SHA와 stable ID별 expected/proposed 값이 있고, `composition.candidate.json`은 검토용 전체 후보다. 사용자의 최신 저장본에 대한 expected 비교 없이 전체 후보를 덮어쓰면 안 된다. 원본 native 측정은 같은 폴더 `source-motion.json`, projection은 `root-motion.json`과 `projected/`에 있다.

실행한 검증은 다음과 같다.

| 검증 | 실제 결과 |
|---|---|
| 기존 Server GameRoom commit, navigation/root sweep, bootstrap reader | 151 checks, 0 failures; `server_result.log` |
| 실제 Client codec Parse/Validate/Serialize/Save + 설치 WModel의 PreviewRootMotion | 438 checks, 0 failures; `client_result.log` |
| 기존 Albion 원본 candidate/native model 회귀, 역방향 seek, 마지막 착지 | 3162 checks, 0 failures; `albion_result.log` |
| 두 후보 Pattern의 정식 Python validate/projected_outputs | 2 closures, 2 outputs PASS |
| 실제 PowerShell airborne row emitter/범위 거부 | 8 checks, 0 failures; `publisher_result.log` |
| 변경 Client Document/PreviewRootMotion/Workbench 및 Server 변경 TU | out 격리 컴파일 PASS; root의 Workbench model-cue duration 1줄도 포함 |
| JSON/Python parse, scoped git diff --check | PASS |

제품 빌드·공유 publish·Client/UI 실행·화면 판정은 수행하지 않았다. 현재 사용자 미저장 편집을 보존하기 위해 데이터 등록은 대기한다. 소유 소스는 Client의 CompositionDocument/ActionWorkbench/PreviewRootMotion, Server의 KoukuSaydonBrain/GameRoom_BossSimulation/기존SupportSurface 테스트, Python projector, Gameplay publisher의 해당 airborne 구간이다.


G10 후보는 이후 최신 저장 revision1214(SHA256 `70e13e94eadf20b507b83a085fd1c99095a85853835ddef42c1b4649cdc01b9a`)로 out에서 다시 생성했다. candidate revision1215의 SHA256은 `9ed52ca4c62a7d45847744d3b2db957d9d248cb8e887c79e3508ab72a91db977`다. 8개 expected/proposed operation만 적용한 값이 후보와 완전히 일치하여 다른 사용자 저장값 보존을 확인했다. 동일 실제 Codec/native model438검사0실패와 P79/P84 publisher closure/출력2개를 재확인했다. `out/SaydonAirborne20260917/freshness-manifest.json`에 실행 파일·후보 hash를 기록했으며 Server 재컴파일과 live Data 쓰기는 하지 않았다.

## G11. 원본 영문 이펙트의 Catalog 누락과 Append 준비

원본 훌라후프5개가 Authored 파일/EffectResourceTree에는 있고 EffectCatalog에는 없어,
실제 CEffectCatalog::Find에서 `direct authored runtime metadata is absent`가 발생했다.
같은 Kouku 누락은521개다. `sync_kouku_effect_tree.py`는 이제 기존 분류와 문서를 유지하며
Tree와 정확한 DIRECT_AUTHORED_DOCUMENT metadata를 함께 stage한다. Catalog를 먼저
교체하고 두 번째 쓰기 실패는 이번 transaction의 bytes가 그대로인 경우에만 rollback한다.
source input SHA·기존 Catalog/Tree bytes가 변하면 기존 편집을 보존하고 거부한다.
payload의 실제 codec/지원 module 검증은 기존 첫 사용 경로가 계속 담당한다.

현재 산출물은 `out/EffectV1Review20260917/library-registration/`의 Catalog/Tree 후보와
521개 추가 entries다. 설치하지 않았다. sync의 기존688개 Kouku 문서 분류 과정에서 별도
미등록 Tree 참조1개(기존 저장된 doll.flame.object-sustain15)도 추가 후보로 기록됐다.
`KoukuSaydonActionWorkbench::Refresh_V1ResourceDuration`는 기존 model cue helper로
잔상 tail을 포함한다. 새 C++ 파일은 없다. 기존 원본 문서521개가 VS Data 목록에도
빠져 있어 vcxproj/filters의 None/96.DataFiles 등록 후보를 함께 준비했다. 실제 프로젝트는
편집하지 않았고, 후보 XML parse와 기존 None 항목 보존·추가521개 필터를 확인했다.

검증은 실제 Catalog/codec/Playback를 링크한 `catalog_library_probe`와 격리 Data를
사용했다. 훌라후프5개는 등록 전 같은 metadata 오류를 재현했고, 등록 후 전부 Load/Find/
drawable/Stage_Document 및 유한 duration(2.2/21/25/9/2.2초)을 통과했다.
521개 전체는493 PASS/28 FAIL이었다. 남은28개는 원본 lifetime/range, Orbit 옵션,
module cardinality, event closure 등의 별도 문제이며 등록만으로 고쳐졌다고 기록하지 않는다.
사용자가 추가28개 수정을 중단하고 나머지 패턴에 집중하라고 지시해 이 범위는 보류했다.
무한 lifetime이나 미지원 module validator를 임의 완화하지 않았다.

`test_sync_kouku_effect_tree.py`의7검사는 ID/기존 Catalog 충돌, 반복 등록,
source/editor 동시변경 거절, 두 번째 쓰기 실패 rollback, 성공 등록을 통과했다.
증거는 같은 out의 `cpu/hula-before.log`, `cpu/catalog-library-probe.log`,
`library-registration/catalog-runtime-validation.json`이다. 이 수치는 화면 판정이 아니다.

## G12. World 그룹 목록과 단일 Append 연결

공5회 분열의 전체 결과는 별도 `2026-09-17_SAYDON_CIRCUS_BALL_WORLD_RESULT.md`
에 기록한다. Root의 목록/Append 변경은 MainApp.cpp, KoukuSaydonActionWorkbench.cpp/H다.
그룹 stable ID를 한 World resource로 노출하고, 각 세대의 효과 tail까지 포함한11500ms를
사용한다. Append가 짧은 기존 Pattern 길이로 그룹을 잘라내지 않도록 필요한 lifetime을
확장하며 단품 Object의 기존 길이 동작은 유지한다. 그룹 편집은 Object Tool로 보낸다.

변경 header를 포함하는 MainApp/Workbench/MainApp_WorldLevel 3개 TU는 out 격리 Debug
컴파일을 통과했다. 실제 MainApp 목록 수집 본문을 추출한 CPU 검사는 split/shot 각63개,
11500ms,2그룹, disabled member 제외를 확인했다. 기존 map placement 조회만 stub이며
Client/UI를 실행하지 않았다. 로그는 `out/SaydonCircusWorld20260917/ui/`에 있다.
등록·shared publish·제품 EXE 빌드·사용자 화면 판정은 계속 대기 상태다.

## G13. 쿠크 거미카운터의 잘못된 링 연결 교체 후보

최신 저장 revision1214에서 `KAKULSAYDON_G1_PATTERN_15`(`쿠크_거미카운터`,
MN_RPCZ_00/action4219776)의 presentation4/5/6은 resource42를 사용했다. 이는
`effect.valtan.project-tuned.sequence.counter`의 cyan decal
`requested.20260827.counter.cyan-roar-ring` V1_ELEMENT다. 사용자가 선택한 B는
다른 문서 `effect.valtan.project-tuned.sequence.trash`의 파란 `mesh_particle_11`이다.

공통 링 후보 `effect.kouku.common.counter.ring` / `kakulsaydon.effect.counter.ring`으로
세 occurrence의 resourceId만 교체했다. 시작1600/8767/15934ms, duration1680ms,
scale[0.2,0.2,0.2], BOSS follow, debugRenderfalse 및 모든 다른 authored 값은 동일하다.
원본 resource42는 남겨두며 다른 Pattern, Effect, 카운터 판정, source clip을 변경하지 않는다.
공통 링의 particle 수명600ms를 1680ms로 늘리거나 반복하지 않는다. 실제 Playback의 1/60초 출생 지연을 포함한617ms를 공유 resource.durationMs로 사용하며, P15의 기존 occurrence1680ms는 보존했다.

원본 action4219776의 CounterAttack은 stage000의1497.727~2500ms, stage001의0~1000ms,
stage002의0~301.864ms다. 이 notify에는 파란 링의 명시적 asset 참조가 없다.
현재1600ms는 저장된 사용자 시각이며 원작 notify 시각과 동일하다고 기록하지 않는다.

`out/KoukuSpiderCounter20260917/spider-counter.patch.json`은 source SHA
`70e13e94eadf20b507b83a085fd1c99095a85853835ddef42c1b4649cdc01b9a`와
새 resource1행, stable occurrence별 expected/proposed3필드 및 expectedRecord를 담는다.
`build_candidate.py`로 현재 저장 상태에서 다시 생성할 수 있다. 전체 candidate를 설치하지 않고
사용자 미저장 편집을 보존한 뒤 해당 필드만 병합해야 한다. live Data는 변경하지 않았다.

실제 최신 CompositionDocument.cpp를 out에 격리 컴파일한 Codec 검사20개가 통과했다.
Parse/Validate/Serialize roundtrip, 새 링3개, 기존 시각·크기·anchor 및 Save_Atomic 재로드를
검사했다(`codec-result.log`). 실제 publisher의 P15→P22 의존 closure와
validate_document/validate_publishable/projected_outputs도 통과하여 Encounter/PatternBindings
두 후보에 새 링3개가 정확히 연결됐다. publisher 설치는 호출하지 않았다.
원본 링 복사·Effect Codec·Playback 자체 검증은
`out/EffectV1ChargeCounter20260917/candidate/counter-ring.receipt.json`과 해당 담당 결과를 따른다.
Client/UI 실행·화면 판정·제품 빌드 및 live 등록은 수행하지 않았다.

같은 공통 링과 P80 FX-only 후보의 Composition 연결도 out에서 검증했다. receipt의
이름형 occurrence ID는 저장 계약에 맞춰 P80 ring=`presentation.1`, FX=`presentation.2`,
P81 ring=`presentation.2`로 배정하고 두 Pattern의 nextPresentationOccurrenceOrdinal을
현재 값에서3으로 증가시킨 expected/proposed patch를 만들었다. P80 FX는0~4366ms,
링은 P80의0~617ms/P81의300~917ms이며 fit/loop는false다. 기존 P81 불뿜기와
양쪽 clip/Logic/World는 보존했다. FX-only 문서의 modelCues0/원본elements21개를 확인하여
기존 live CNpc 잔상과 중복된 model cue를 Pattern에 붙이지 않는다. actual Codec18검사와
Save_Atomic, P80/P81 publisher closure/출력2개가 통과했다. source revision1214의
`out/EffectV1ChargeCounter20260917/composition-validation/pattern-counter.patch.json` 및
`verified.json`이 정확한 numeric ID 연결 후보·검증 증거다. 공통 링 resource는 P15
patch와 같은617ms 행을 한 번만 병합하며 full candidate는 설치하지 않는다.


## G14. 종이비둘기·피자 후보와 Orbit 종료 위치 검증

소스와 out 후보 작성 및 격리 검사를 완료했다. `build_kouku_dove_pizza_candidates.py`가 만든
`effect.kouku.magic.paper.dove.group`은 원본 비둘기4개와 원본 폭발19개를 한 문서로 묶는다.
velocity/orbit/death-event를 재사용하고, frame과 event는 `Evaluate_SourceOrbitOffset`의 같은
계산을 사용한다. PortableRuntime은 해당 Orbit-offset event boolean만 허용했다. 중단 지시를
받은 영문 Append28개 추가 복구는 포함하지 않았으며 public layout 변경도 없다.

실제 CPU 재생은4마리, 반경1m, 주기3초, 중심속도8m/s, 첫 종료 폭발3.03333초와 총216개
impact birth,6초 내 정리를 확인했다.8m/s는 원본 초기속도이고3초·반경1m·회전주기는 저작값이다.
단일/연결 Orbit의 frame-event 위치 오차는 각각 약0.000021/0.000023m다. root yaw90도와37도
비교에서 궤적과 최종 폭발이 함께 회전했고 impact 위치 최대오차는0.000003815m였다.
원본 projectile의15m 이동 상한과 같은 동작이라고 주장하지 않는다.

`effect.kouku.pizza.explosion.group`은 원본02/03의24개 요소와 native material/mesh를 유지하고,
Distribution=None 때문에 비어 있던 CDO radius/velocityscale5필드만 복구했다. PSA_Velocity
sprite의 실제 이동은 수평이며 최소 방사속도0.01m/s, 수직속도0이었다. 원본 mesh는 이미 바닥
방향임을 설치 정점과 Playback 행렬로 확인했다.120도/90도 sector mesh의 원본 yaw 합집합은
약85.2도의 안전 간격이므로 **정확90도 mask는 미구현**이다. 전역90도 회전·범용 shader 수정은 없다.

증거는 `out/EffectV1DovePizza20260917/verified.json`, `cpu-probe.log`,
`root-rotation-probe.log`, `pizza-mesh-floor-proof.json`이다. candidate 폴더의 두 Effect와 receipt에
catalog/tree 등록 재료가 있다. Load/Drawable/Serialize-Parse/Stage와 관련 격리 컴파일은 통과했고,
이는 GPU 표시나 사용자 화면 승인 증거가 아니다. live Data·Catalog·Tree 등록, 최신 제품 빌드 및
사용자 최종 시각 확인은 대기한다.

## G15. 공통 파란 링과 돌진 V1/Pattern 후보 검증

`build_kouku_counter_ring_candidate.py`는 사용자 선택 B의 `mesh_particle_11`을 복사했다.
원본의 실제 저장명은 `Project Tuned / Trash / Floor And Hand Composite`이며 Trash는 이름의
일부다. 삭제함 상태라는 근거가 없고 기존 문서를 삭제·이동하지 않았다. 새 asset은
`effect.kouku.common.counter.ring`, 표시 경로는 `세이튼_카운터 / 카운터 이펙트`다.
ID/groupId/displayName 외의 visual payload 전체가 원본과 동일함을 비교했고,600ms particle 수명과
파란색·크기·위치·재질을 보존했다. 실제 Playback616.667ms를 올림한617ms가 resource 길이다.
새 P80은0~617ms, P81은300~917ms이며 G13의 P15 occurrence1680ms는 그대로다.

`build_kouku_charge_counter_group.py`의 `effect.kouku.gate1.charge_counter.group`은 V1 PlayAll용
몸체 잔상 cue2개와 원본 dust3/양팔 trail12/light1/dash streak5개를 포함한다. 기존 afterimage
후보는 보존했다. 몸체 잔상의 색·수명 등은 PROJECT_AUTHORED이며 원작 TrailGhost 재질 전체
복원이라고 기록하지 않는다. P80은 이미 `Charge_AfterimageActive → CNpc`의 몸체·무기 helper를
사용하므로 별도 `effect.kouku.gate1.charge_counter.fx`만 연결한다. 이 후보는 동일한 원본 FX21개와
source attachment를 보존하고 modelCues는0개다. 새 몸체 렌더나 두 번째 잔상 owner가 없다.

Effect codec/Drawable/roundtrip/Stage를 통과했고 설치 CModel의 실제 b_effectroot/b_root/좌우
upperarm 네 본을 샘플링해 basis1.7, FX-only 최대99입자/6trail과 유한 종료를 확인했다.
실제 윈도우·draw는0이다. 단독 FX의5199ms tail과 P80 saved clip 종료4366ms를 구분했으며,
패턴 종료에서 남은 tail은 기존 ownerStop에 따라 잘린다. 시간 늘림·반복·전체 tail 완료를 주장하지 않는다.

asset·anchor 검증은 `out/EffectV1ChargeCounter20260917/verified.json`,
`charge-fx-codec-probe.log`, `charge-fx-model-probe.log`에 있다. 정확한 P80/P81 등록 후보는
G13에서 검증한 `composition-validation/pattern-counter.patch.json`이다. 숫자 occurrence ID와
next ordinal을 포함한7개 guarded operation이며 actual Codec18검사와 publisher2출력이 통과했다.
ring/charge receipt의 이름형 ID는 semantic template이고 실제 저장에는 이 numeric patch를 사용한다.
P15와 공통617ms resource는 한 번만 병합한다. 소스·후보·격리 검사는 완료했지만 live 등록·shared
publish·최신 제품 빌드·사용자 화면 판정은 대기한다. 기존 사용자 원본과 미저장 draft는 변경하지 않았다.

G14/G15 검증 시점의 Composition은 revision1214, SHA256
`70e13e94eadf20b507b83a085fd1c99095a85853835ddef42c1b4649cdc01b9a`였다. 이후 root가 관측한
최신 저장 hash는 `718d8d77669750d19f482f94d8368e079ce0ae9a228fca5a7389fb7a83bb28ef`로 달라졌다.
사용자가 계속 편집 중이므로 후보를 다시 생성하거나 설치하지 않았다. 최종 저장 뒤 semantic patch의
expected record·numeric ordinal을 새 기준에서 재생성하고 검증해야 한다. 전체30개 JSON 후보 목록은
`out/EffectV1Review20260917/all-work-pending-registration.json`이며, 등록 완료 목록이 아니다.

현재 live 저장 revision은1218이며 위 검증 기준1214와 다르다.1218 기준의 적용 검증은 아직 수행하지 않았다.

## G16 보충. P79 SELECT 지면 고정 및 그룹 연결 최종 검증

사용자 최종 저장 revision1223을 기준으로 G10 후보를 갱신했다. 별도 Logic80 SELECT_PLAYER를0ms에 추가하고
optional SELECT 위치 정책으로 실제 player navigation ground XYZ를 한 번 저장한다. 공유 Logic53과 기존 Albion
APPEAR 시점 재조회는 유지한다. P79 Logic75는7148ms부터13.678813m/200ms 상승, Logic76은8883ms에
선택한 지면 위로 등장하고 같은 시각 SLAM이 원본17_05 하강을 소비해 약10566ms에 지면에 도달한다(10662ms 원형 폭발보다 약96ms 앞서며,12966ms는17_05 클립 종료 시각이다).
P84 즉시 초기 높이11.191078m와 후속13_end의 원본16.744m 상승도 유지했다.

사용자가 만든 group9의 네 Effect는8907/10662/11423/11978ms를 그대로 사용한다. 공통 MAP 원점
[6.4299998283,1.2999999523,730]을 XYZ 모두 빼서 local0으로 만들고 기존 fixed targeted visual 하나로
투영한다. Server SELECT가 지면 capture와 단일14478ms presentation 객체를 함께 commit하며 대상 이동·사망 뒤에도
APPEAR는 같은 점을 사용한다. 일반 presentation에서 네 멤버를 제외해 중복을 막는다. 기존 Stop/source 정리가
객체를 제거한다. source 파일과 out 후보만 수정했으며 live 등록은 root의 최종 transaction이 별도로 소유한다.

실제 Client Document/PreviewRootMotion/Workbench isolated compile, 실제 설치 WModel과 Codec/Save/착지
442검사, 실제 Server SupportSurface162검사, Python airborne4검사, 실제 PowerShell row writer14검사가 통과했다.
root의 최종19파일 합성 Composition revision1224도 실제 Parse/Validate/Serialize/Parse3검사를 통과했다.
legacy7열과 SELECT10열의 catalog admission·잘못된 phase/정책/수명/짝필드 거절도 확인했다.
Client Presentation/LogicPreview의 별도3TU 컴파일과 source-extracted38검사는 effect_data가 수행했다.
이38검사의 ground/effect Sample은 stub이며 실제 draw가 아니다. 제품 코드는 CCharacter::Try_SampleTargetGround를 호출한다.

증거는 `out/SaydonAirborne20260917/freshness-manifest.json`, `server_result.log`, `airborne.bootstrap.rows`,
`projected/Data/Animation/Authored/KoukuSaydon/KoukuSaydon.patternbindings.json`에 있다. 입력 경로를 받는
`client_probe.exe <Composition.json>`은 실제 document codec을 재사용한다. 최신 제품 빌드·화면 판정은 이 검사에 포함하지 않았다.

## G17. 최신 저장본 실제 등록·정식 Publish 결과

사용자의 “모두 저장했고 Client도 종료했음” 확인 뒤 Client process 부재와 최신 저장본을 확인했다.
Composition1223의 사용자 변경을 기준으로 stable ID와 expected/proposed record를 다시 검증했다.
710개 입력 hash와 각 대상의 before byte를 확인하고 새 Authored12개, WorldSequence, Valtan sidecar,
Catalog, Tree, Composition, vcxproj/filters의 총19개 파일을 설치했다. 기존 카드·트럼펫12개 Effect는
byte-identical이며 사용자 카드 rotation, P79 작은 공511ms·scale0.5, group9와 네 재생 시각을 보존했다.
원본 백업은 `out/EffectV1Final20260917/before`에 있다.

| 등록 항목 | 실제 연결 상태 |
|---|---|
| 새 Effect12개 | 회전 카드 emitter, 공통 파란 링, 세이튼 돌진3종, 발탄 full restore 잔상, 쓰리투원4종, 비둘기 묶음, 피자 묶음을 Catalog·Tree·프로젝트에 등록 |
| 공 5회 분열 | 낙하/발사 World group2개, 각각63개 공을 기존 WorldSequence 경로에 등록. P83의 비어 있던 타임라인에는 임의 시각을 추가하지 않았으며 사용자가 World 항목을 Append하여 배치하는 리소스다 |
| 카운터 | P15 거미의3개 occurrence와 P80 돌진/P81 공굴리기에 공통 링 연결, P80에는 몸체 cue 중복 없는 FX-only 연결 |
| P79 공먹기 | Play0 SELECT 지면 좌표 하나를 착지와 warning/circle/innerdonut/outerdonut에 공유. 네 시각8907/10662/11423/11978ms 유지 |
| P84 훌라후프 | 원본 하강량 기준 초기 높이와 SLAM 연결 |
| 영문 source 항목 | 누락 metadata521개 등록. 이전 payload 검사493개 통과/28개 실패를 모두 해결한 것으로 확대하지 않음 |

기존 World template256개가 상한에 도달해 추가 문서의 actual codec load가 실패했다. 컨테이너가
vector/stable ID이고8-bit count나 고정 배열 소비자가 없음을 확인한 뒤 Client codec·Python·PowerShell·
생성기의 같은 상한을512로 올렸다. 최종268 templates/456 resources/324 instances의 load/save/reload가
통과했고512 허용·513 거절 및 실패 시 기존 문서 보존을 검증했다. 기존 row payload는 변경하지 않았다.

정식 `Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon -ExpectedKoukuSaydonSourceRevision 1224`를
실행하여 koukusaydon.product, map.kakulsaydon, world.gameplay, gameplay.balance 네 domain이 PASS,
process exit0이었다. 저장84개 중 기존 publish 가능한71개 패턴/413 stages/8 bundles를 투영했다.
불완전한 다른 draft13개를 임의 수정하지 않았다. 이번 대상 P15/P47/P48/P78/P79/P80/P81/P83/P84의
unavailableReason은 모두 비어 있다. Composition·Encounter·PatternBindings revision1224,
authoring/runtime WorldSequence revision2035와 문서 동등성을 확인했다.

설치된 Catalog의 actual native probe는 새 Effect12개 모두 Find/Drawable/Stage/duration을 통과했다.
1159개 전체 source 문서는 metadata를 읽었을 뿐 모든 payload를 검사한 결과가 아니다. P79의 네 요소가
일반 MAP lane에서 제외되고 local XYZ0 targeted visual 하나에 들어갔으며 Server bootstrap의 SELECT10열과
14478ms 수명이 해당 visual ID를 가리키는 것까지 확인했다. 설치19개 파일 hash는 publish 후에도 동일하다.
변경 JSON/XML parse와 관련 source/data의 `git diff --check`도 통과했다.

최종 증거는 `out/EffectV1Final20260917/installed-registration.json`, `publish.log`,
`final-publication-verification.json`, `catalog-installed-effects.log`, `world-codec/result.log`다.
이전 pending manifest3개는 이 설치 receipt로 superseded 처리했으며 옛 candidate hash를 최신 설치값으로
바꿔 쓰지 않았다. 이 단계는 데이터 등록·publish와 격리 compile/CPU 검증이며 제품 실행·GPU 시각 PASS가 아니다.

피자는 원본 exp_03의 실제120도 sector mesh2개와90도 sector mesh1개를 원본 회전대로 조합한다.
세 mesh의 빈 각도는 약85.2도이고 exp_02의 무지개 halfcylinder와 sprite는 별도 요소다. 기존 material의
flow/opacity/alpha/noise/dissolve mask는 무늬·투명도용이다. 모든 요소를 공통90도로 자르는 angular mask는
추가하지 않았으므로 모든 입자에 정확90도 안전 영역이 적용됐다고 기록하지 않는다.

최신 Client·Server 빌드와 아레나/Effect V1 실제 화면 확인은 사용자 담당으로 남는다. 당시 확인한
로컬 Client·Server process는 모두 없으며 에이전트가 실행하지 않았다. 사용자 요청으로 중단했던
영문 원본 payload28개 추가 복구와 shader build 최적화도 이번 등록 완료에 포함하지 않는다.

## G20. 피자 source 분리 후보와 오른손 지팡이 끝 트레일

사용자 첨부 피자 화면의 검정·무지개 내부 공백을 열람했다. 기존 그룹은 원본 exp02의 원형
충격파8개와 exp03의 피자16개를 동시에 재생했다. 원본 Action4219774는 stage4의
1.162550초에 exp01/02를 호출하고 다음 stage5의0초부터 exp03을 호출한다. 새 피자 후보는
사용자 허용대로 exp02만 제외하고 현재 저장된 exp03의16개 ID·모듈·재질·TRS를 전부 보존한다.
후보 asset ID는 기존 `effect.kouku.pizza.explosion.group`이며 라이브 문서를 덮어쓰지 않았다.

피자 메시를 개수 부족으로 판정하지 않았다. 설치 WModel의 실제63/63/49정점,
80/80/60삼각형과 실제 CEffectPlayback0.2초 행렬을 대조했다. 원본 rotation .45/.88/.65turn과
120/120/90도 mesh는 반경0.3/1/3/5/6m에서 모두 한 구간 약85.1도만 기하학적으로 비어 있다.
원본 native3241의 UV noise·opacity·dissolve와 보조 sprite를 임의 수정하지 않았다. 따라서
이 후보는 잘못 합친 원형 pulse의 제거까지 완료이며, 사용자 이미지의 내부3공백이 원본처럼
보이게 해결됐다는 판단은 미완료다. geometry coverage를 최종 pixel coverage로 대신하지 않는다.

새 비교 asset은 `effect.kouku.gate3.ritual.staff.tip.trail.full.restore`, 표시 이름은
`저주의식 | 오른손 지팡이 끝 트레일`이다. 기존 왼손 문서는 byte 보존한다. 원본4219911/001의
startcontrol→b_wp_1, X75cm와 실제 WP_MN_RPCT_05의 +X끝70.236702cm를 사용했다.
socket은 실제 끝보다4.763298cm 바깥이며 설치 actor배율1.7에서 약0.081m다. 측정된
socket Rx(-90)와 body preScale.017을 적용한다. white ribbon의 원본 +100cm 추가 offset을
0으로, clip-source-segment를 false로 둔 것은 요청한 tip 연결용 USER_REQUESTED 조정이다.
원본 source leaf와 기존 왼손의 현재100cm 편집값은 변경하지 않는다.

현재 source-anchor helper를 소스에서 추출해 out에 격리 컴파일·링크하고 실제 CModel27_01
clip으로676개 anchor표본,2개 ribbon+sprite, finite속성, 되감기 동일성을 통과했다. Ribbon
최대 point30/20, alpha1, 폭0.339592/1.36m를 관찰했다. 두 후보 모두 실제 Codec Load,
Validate_Drawable, Serialize/Parse, Stage/15초 Update 종료를 통과했다. 피자 duration2.4초,
지팡이4.174864초이며 source preview clip4667ms는 별도 애니메이션 문맥이다. Python compile과
관련 diff check도 통과했다. 제품 빌드·Client/UI실행·GPU시각 검사는 하지 않았다.

피자 후보·입력hash·요소별 제거조건은 `out/EffectV1PizzaFan20260917/installation.json`,
source notify는 `original-stage-notifies.json`, 삼각형은 `triangle-coverage.json`, 공통 CPU
검증은 `codec-playback.log`다. 지팡이 Catalog/Tree entry와 Data None 경로는
`out/KoukuStaffTipTrail20260917/installation.json`, 실제 모델 검증은 `cpu/result.json`이다.
수정한 소스는 기존 두 Python builder뿐이고 새 H/CPP/project 소스 등록은 없다. 실제 등록은
최신 사용자 저장본과 manifest guard를 확인한 별도 transaction에 남아 있다.

## G18. 주사위 크기·비행 연속성 후보

사용자가 확인한 손/비행/착지 주사위 크기 차이와 정지·순간 이동을 실제 설치 CModel로 재현했다.
원본 상승·낙하 LocationDirect의 ScaleFactor에는 Distribution=None만 저장되어 있고, Engine CDO는
단위 배율1을 가진다. 기존 leaf의 빈 cooked table은 이를0으로 평가해 위치 곡선을 소거했다.
손·상승은 본 basis1.7을 받아 mesh basis12.75이고 낙하·착지는7.5였다. CDO1만 복구하면 상승은
기울어진 손 birth basis를 따라가 다음 root 기준 낙하와 끊긴다. 미해독 proprietary notify 방향
플래그를 복원했다고 주장하지 않고 이 부분은 `PROJECT_AUTHORED_CONTINUITY`로 구분했다.

`build_saydon_card_pattern_groups.py --dice-only`는 최신 주사위 문서를 읽어53요소 중5개만 바꾼다.
손 mesh 크기를 설치 본 basis로 정규화하고, 원본 상승7개·낙하5개 곡선 표본을 notify 창에
재시각화한다. 실제 손 해제점에서 원본 apex(40,0,600)cm를 거쳐 착지(240,0,35)cm로 이어지며,
원본 낙하 끝50cm와 착지35cm의 차이도 끝에서 해소한다. 기존 SourceTransformTrack의 absolute
alpha cutoff로 다음 carrier가 시작될 때 이전 carrier를 끈다. 원본 native 재질·mesh·회전 속도와
다이아·폭발 요소는 보존했고, 원본 입력 전체는 receipt에 남겼다. 공용 runtime/ABI 변경은 없다.

실제 CModel(preScale.017), 설치 dice mesh(preScale.01), 원본11_02 clip과 Playback으로 검사했다.
네 carrier의 world basis는7.5(최대오차.000137), mesh 축별 치수는 약.989×.871×1.067m다.
해제2.433333초/낙하3.9초/착지4.166667초의 같은 frame 중심 오차는 최대1.2e-7m이며,
root yaw37도 공변 오차는1.9e-7m다. 상승·하강은 각각 단조이고 보이는 주사위는 매frame1개다.
Codec/Drawable/roundtrip/Stage와15초 유한 재생, builder py_compile, 관련 diff-check가 통과했다.
G19 최신 Playback OBJ로 재링크한409개 frame 행도 이전 수치와 byte 단위로 동일했다.

후보와 검증은 `out/EffectV1Dice20260917/candidate/`, `verified.json`, `dice-continuity.patch.json`,
`dice_probe.cpp/.cmd/.link.rsp`, `candidate.csv`, `rotated.csv`, `final.csv`, `mesh-bounds.txt`다.
후보SHA256은 `7919c8d46ca94acb3335198ef03e654ca8bb45ad011f854ee6ba51c68ebfa917`이며
baseline `6257aa644baea2382f59d0aca17204c7c53643584df3d5782980245d2dccec6a`의 bytes는 불변이다.
71개 필드별 expected/proposed와 전체 baseline hash를 함께 guard한다. live 설치·제품 빌드·화면
판정은 아직 하지 않았다. 수치상 연속성과 원본 엔진의 미해독 부착 정책·시각 일치는 별개다.

## G19. 카드 출력 회전축과 추적 카드 표시 정리

원본 카드 출력5개 sprite의 EPAL_Rotate_Z는 Client ROTATE_Y로 변환되지만 기존 renderer는
ROTATE_X/Y를 일반 camera billboard로 처리했다. Particle Billboard를 끄면 source facing과
pivot 경로도 건너뛰기 때문에 raw 단면 quad가 사라져 보일 수 있다. Local Space는 birth/current
root 선택이며 이 방향 결함을 해결하는 옵션이 아니다.

GeometryHelpers는 sourceRecipe와 followEmitterAxisRotation이 모두 켜진 ROTATE_X/Y/Z에만
변환된 emitter 축을 적용한다. 카메라 방향을 축의 수직 평면에 투영해 rect front normal을 카메라로
유지하고, view가 축과 평행하거나 camera와 입자가 겹쳐도 finite한 대체 basis를 사용한다.
opt-off의 기존 X/Y camera billboard와 Z 계산은 그대로다. Playback은 local-space 현재 root와
world-space 출생 root를 전달한다. MaterialDetail의 기존 Axis lock follows emitter rotation 옵션을
회전축에도 노출하고 활성화할 때 particle billboard를 켠다. public field·shader ABI는 추가하지 않았다.

카드출력 후보는 사용자 TRS·revolution·source 모듈·재질·타이밍·숨긴 spark를 보존하면서
6개 particle billboard/localSpace와5개 axis-follow의15개 필드만 수정한다. 실제 GeometryHelpers,
MaterialHelpers, 현재 Playback과 Codec을 링크한 sprite_axis_probe에서888검사/36축·카메라
조합을 통과했다. 180도 반전, 기울기, pivot·크기, 평행/겹친 camera, local/current 및 world/birth,
실제 저작5개 sprite의30frame을 검사했다. 변경3TU 격리 컴파일과 독립 코드검토도 통과했다.
증거는 out/EffectV1Corrections20260917/sprite-axis-result.log와card-emission.patch.json이다.

P78 logic73은6114ms에 서로 다른4종 Card Effect를 생성하며 속도3m/s·contactRadius0.5m·
lifeTime0·homing을 실제 Server combat object에 전달한다. root의 일반 Effect lane에 별도로
놓여 있던 카드4행과 고정 폭발1행은 gameplay 추적 카드와 다른 표시이므로 이5개 occurrence만
제거하는 guarded patch를 만들었다. 원본5개 행은 patch와before 백업에 보존하며 effect 정의와
logic73, 주사위 및 카드출력 occurrence는 유지한다. 실제 projector에서 ordinary2개와Server card
visual4개, loop=true 및 공통contact explosion 연결을 확인했다.

현재 Server focused harness162검사/0실패:601초 무한 생존, 현재 target로 방향 전환, 자연 완료
뒤 생존, target 사망 정리, swept contact의1회 폭발·despawn, source 취소 정리를 확인했다.
무한 추적 요구에 따라 자연 패턴 완료로 카드를 제거하지 않는다. 단, UI Stop/restart는 활성 run
중에만 수락되며 자연 완료 뒤에는 stale run으로 거절된다. 이때 카드는 접촉 또는 target/boss
무효화로 종료된다. 증거는 pursuit-server-verification/server-result.log와verified.json을 따른다.

이 절의 CPU 검사와 격리 컴파일은 사용자 제품 빌드·화면 확인을 대신하지 않는다. 실제 데이터
설치와 정식 Publish 결과는 후속 통합 등록 절에 별도로 기록한다.

### G20 후속 — 원본 native3241 투명 영역 대조

원본 cooked pixel shader `8989ff46dbefa54a848aa35f3480555c.dxbc`와 설치된
`Shader_VtxEffectMeshKouku3200.cso`의 native3241을 headless WARP에서 비교했다.
실제 Playback의 .05/.1/.2/.4/.7초 Color·DynamicParameter, 원본 material parameter와
DDS 네 장을 동일하게 공급했다. 원본 PS의 register별 입력과 texture/sampler binding을
맞춘 UV0 64×64 사각형에서 blending을 끈 RT0 alpha를 수치로 읽었다. Client·장면 실행이나
화면 캡처는 하지 않았고 shader source와 제품 CSO도 변경하지 않았다.

총20,480표본의 alpha=0 mask와 abs(alpha)>1e-5 mask는 원본/설치 간 차이0이다.
시각별 nonzero alpha 표본은1629/2202/2878/3215/3153으로 각각 일치했다. 최대 alpha
절대 오차는0.001629미만, 시각별 평균 오차는8.43e-6미만이다. 실제 carrier는 UV0와
ParticleColor·DynamicParameter4개를 전달하며 원본도 이 경로를 소비한다. 이 입력 범위에서
native3241 번역 또는 입력 누락이 추가 투명 공백을 만든다는 가설은 재현되지 않았다.
원본 PS 자체도 같은 UV 영역을 투명하게 만든다는 근거이지, 사용자 이미지의 모든3공백이
원본 의도라는 증명은 아니다. 원본 화면의 정확한 시각·장면 합성과 보조 sprite의 결과는 이
검사 범위에 포함되지 않는다. 추가 mesh 복제나 원본 alpha 변조를 정당화할 근거는 없다.

증거는 `out/EffectV1PizzaFan20260917/native-comparison-summary.json`,
`native-installed.json`, `native-original.json`, `native_inputs.txt`,
`cpu/material-samples.csv`, `native_compare_probe.cpp`와 `compile_compare.log`에 있다.
요약 JSON에 원본 DXBC와 설치 CSO의 SHA256을 고정했다. exp02 분리 후보 이후 추가
source/Data 수정은 없으며 root의 설치 transaction과 이 원인 조사를 구분한다.

오른손 지팡이 검증도 G19 최신 `out/EffectV1Corrections20260917/Effect_Playback.obj`로
재링크하여 실제 CModel676표본과2개 ribbon·sprite 검사를 다시 통과했다. 증거는
`out/KoukuStaffTipTrail20260917/cpu/result.latest-playback.json`이다. 최종 Catalog용
duration은 실제 Effect 문서4.174864초를 올림한4175ms이며, source preview clip4667ms와
구분한다. manifest의4175ms는 변경할 필요가 없다.

## G21. 최신 저장본1232에 후속 수정 실제 등록·발행

앞선 전체 등록 승인을 유지한 상태에서 이번 최신 수정 중에도 Client/Server 프로세스가 없음을
재확인했다. 새 보존 질문에는 답이 오지 않았으며 이를 새 확인으로 간주하지 않았다. 실제 저장된
Composition1232/World2035와 여섯 Effect의 bytes를 기준으로 후보를 작성하고, commit 직전에도
Client 종료 상태와 모든 대상·후보·참조 입력 해시를 다시 검사했다. 미저장 editor를 종료하거나
사용자 draft를 덮어쓰는 작업은 하지 않았다.

12개 파일을 백업·설치했다. Composition1233, WorldSequence2036이며 기존 user occurrence의
배치·속도·TRS와 관련 없는 모든 logics/world/resource는 보존했다. 변경 목록은 다음과 같다.

- 카드 출력15개 설정 필드: source billboard/localSpace 유지와5개 emitter axis-follow 활성화.
- 주사위71개 guarded 필드: CDO 이동 기본값 및 실제 손 basis를 반영한 크기·연속 궤적 보정.
- 낙하 공/상단광2개 Effect와World12개 template: 같은 공 중심·상단 offset·시간을 소비하고 별도 낙하 drift 제거.
- P78: 서버 카드와 중복되던 ordinary 카드4행·고정 폭발1행만 제거. logic73·6.114초 발생·4종 무한 추적 유지.
- P83: world.7을 내부 g0 모델에서 전체 group world30으로 교체하고7479→11500ms. 배치·시작 시각 보존.
- 피자: 추가 exp02 원형8개 제거, 기존 exp03 16개 완전 보존. 사용자 이미지의 내부3공백 해소는 확정하지 않음.
- 오른손 지팡이 트레일 신규1개: Catalog/Tree/Composition4175ms/프로젝트None·filter 등록. 기존 왼손 bytes 보존.

독립 검토에서12파일과20개 참조 입력의 해시 및 before에서 재구성한 전체 JSON 동등성을 확인했다.
World456 objects/324 instances와Composition의전체logics/worlds를 보존했으며 split/shot 양쪽 모두
6개 motion,1+2+4+8+16+32=63개,최종FX종료11500ms다. MainApp/Workbench Append는 내부 donor를
고유한 전체 group으로 해석하고 중복 목록을 제외한다. 여러 owner가 있는 경우 임의 선택하지 않는다.

실제 Catalog first-use 검사에서 변경·추가6개 전부 Find/Drawable/Stage/duration을 통과했다.
결과 duration은 카드출력20.15초,주사위11.661초,낙하3.31667초,World상단2.5초,
피자2.4초,오른손지팡이4.17486초다. 관련6개 TU 격리 컴파일, Server162검사, sprite888검사,
공FX891검사,World18494검사,실제CModel지팡이676표본 및 주사위 연속성 검사를 통과했다.

정식 Invoke-BuildDomainOwner -Owner KoukuSaydon -ExpectedKoukuSaydonSourceRevision 1233의
koukusaydon.product / map.kakulsaydon / world.gameplay / gameplay.balance 네 domain이 통과했다.
발행본을 다시 읽어 sourceRevision1233,World2036동등,P78ordinary2개/loop카드4종/contact폭발,
P83전체group11500ms와사용자배치,지팡이등록1개를 확인했다. 이전에 사용가능하던
P15/47/48/78/79/80/81/83/84도 사용가능 상태다.

백업은 out/EffectV1Corrections20260917/before,실제 설치 증거는 installed-registration.json,
최종 연결·Publish 증거는 final-publication-verification.json과publish-owner.log다.
기존 회전 카드 emitter는 패턴의Logic 소유이며 V1통합 `effect.kouku.card.spinning.emitter`도 유지한다.
이번 작업에서 제품 셰이더 전체 빌드·Client/Server 제품 빌드·실행·화면 캡처는 하지 않았다.
현재 프로세스가 없는 상태이며 최신 제품 빌드와 Server 시작 및 Client의Effect V1/패턴 Play
화면 판정은 사용자가 수행한다. 셰이더 빌드 최적화와 기존source28개 미지원 범위는 중단 상태를 유지한다.

### G20 후속 — 검정 sprite 원본과 원통 반원 제한

추가 저장본은19요소이며 SHA256은 `4074dd36418efaa3a2b6648186c8bf06c067b6ac75db49653bb50b750d42f116`이다.
사용자가 복제·배치한 4~8번 dark-aura와9/10번 flow-mask를 포함하여 문서 byte 전체를
`out/PizzaBlackLayer20260917/live-before.effect.json`에 보관했다. 이 작업은 live Data,
Catalog, Tree와 사용자 복제본의 위치·회전·크기를 변경하지 않았다.

원본 자료는 존재한다. exp03 dark-aura emitter19/33(native3171)는 EPAL_Z와 pivot(.5,1),
StartSize(800,800,900)cm 및 수명 배율(3,2,1.021421)을 갖는다. 실제0.2초의 rect는
13.858187×10.929092m다. 원본 PS `c32c8cb20b653f4194a5f1f195a26bdc`의 alpha는
UV falloff·ParticleAlpha 수식이며 두 noise texture는 RGB 경로에 사용된다. 이 개별 검정
레이어는 원형 world mask가 아니다. flow-mask emitter1/31(native3280)는 signed 크기
(-200,-650,1)/(200,-700,1)cm와 PSA_Velocity, pivot(.5,.8)을 사용한다. fx_d_noise_002는
flow 입력이고, 실제 경계는 UV의 radial-distance fade와 fx_m_ring_001_cl opacity texture가
함께 만든다. world-offset emitter0(native3281)는 단일 StartSizeX1200cm를 가진 EPAL_Z
sprite이며 실제0.2초 rect12.733333×12.733333m로 양 축이 같다. 이를 색깔별 공통 원형
mask 하나로 설명하거나 개별 sprite의 타원을 곧바로 복원 오류로 판단하지 않는다.

flow emitter1의 원본 FX_MN_RPCZ_00_U export4974에는 negative_x=false, radius80~90cm,
surfaceOnly와 radialVelocity가 명시돼 있다. emitter31의 export144는 halfMode=true와
CDO splitCircleCount6을 사용하고, sparse Distribution=None 아래의 CDO radius는1cm다.
원본 burst는 각각4/10개다. 현재 사용자 문서에는 원본 emitter1 행이 없고 두 flow 행 모두
emitter31 복제본이므로 원본16요소와 별도 저작 구성이다. 이 사실을 사용자 편집 삭제·복구의
근거로 사용하지 않았다. `source-reference.effect.json`은 기존 원본16요소와 이미 검증한
CDO 값을 보관한 비교용 out 문서이며 설치 후보로 승인하거나 live에 덮지 않는다.

실제 코드 결함은 `Effect_Playback.cpp`의 LOCATION_PRIMITIVE_CYLINDER가 source의
positive/negative XYZ 제한을 읽지 않던 부분이다. 원본 emitter1의 첫4입자 중2개가 금지된
음수X(-0.610443/-0.748921m)에 생기는 것을 현재 Playback으로 재현했다. 해당 분기에만
17줄을 추가해 height-axis 배치 뒤 source 좌표의 부호 제한을 적용하고, 기존 난수 소비와
제한 없는 동작을 유지했다. 새 shader나 source size·alpha 수식은 추가하지 않았다.

원본4입자의 반원 위반은2→0이며, height축3×제한축3×부호조합4의36조건을 실제 Playback으로
검사한9,216행에서 제한 위반은4,608→0이다. 제한 없는2,304행과 다른 검정 레이어156행은
수정 전후 byte 동일하다. Playback·GeometryHelpers와 전용 CPU probe 격리 컴파일/링크,
JSON parse와 diff-check를 통과했고 C++ UTF-8/noBOM/CRLF를 유지했다. 다른 에이전트의
Step/SpawnPerUnit 변경과 기존 axis 변경을 보존했다. 이 수정은 현재 두 emitter31 복제본의
타원을 직접 원형으로 바꾸는 수정이 아니며, 최종 합성 외곽의 화면 일치는 아직 미확인이다.

원본 module export·CDO·native PS/VS와 수치 근거는 같은 out의 `evidence.json`,
`raw-source-exports.json`, `raw-class-defaults.json`, `native-material-evidence.json`,
`layer-before.csv`, `layer-after.csv`, `cylinder-verification.json`에 있다. 검증36조건은
`cylinder-axis-fixture.effect.json`과 `cylinder-axis-cases.json`으로 재현한다. 원본 EFGame
CircleSurface의 각도 선택 native 구현을 새로 확정한 것은 아니므로 그 분기는 수정하지 않았다.

### G18 후속 — 주사위·상단 다이아 현재 크기의 1.5배 후보

최신 주사위 문서 SHA256 `7919c8d46ca94acb3335198ef03e654ca8bb45ad011f854ee6ba51c68ebfa917`을
out/CardDiceScale20260917/dice/before.effect.json에 byte 보관했다. 53요소 중 손·상승·하강·착지의
`fm_l_dice_01_sm.wmodel` mesh 4개와 상단 symbol_109 sprite 3개(emitter23/24/25)의
`detail.particle.sourceScale.size`만 현재값의 1.5배로 만든 후보를 준비했다. 손 항목은
0.5882353782620658→0.8823530673930988이고 나머지6개는1→1.5다. 별도 modelCue는0개다.
상단 다이아는 native2987 alpha sprite2개와 native2988 additive sprite1개의 합성이며,
함께 사용하는 symbol_109 mask의 크기를 일관되게 늘렸다. 다른46요소와 보스 preview,
본 부착, 사용자 TRS, source 궤적·회전·타이밍·alpha는 변경하지 않았다.

실제 CModel MN_RPCT_05와 rpct00_att_battle_11_02, bip001-l-hand 및 주사위 WModel을 로드한
headless WARP/Playback에서 yaw0/180 각각916개, 총1,832개 표본을 비교했다. 모든 입자 중심,
alpha와 손 본 위치·배율 witness가 전후 정확히 같고, mesh XYZ 및 sprite XY 표시 크기는
1.5배다(최대 비율 오차1.91e-7). sprite의 단위 법선 Z는 그대로다. 주사위 월드 행렬 축 배율은
약7.5→11.25이며 상단 다이아 첫 표본은1.21→1.815m,1.1→1.65m다. 기존 손→비행→착지의
좌표 표본과 출생·소멸 시각을 그대로 보존했다. Codec Load/Drawable/Serialize/Parse/Stage와
전용 probe 컴파일·링크 및 JSON/diff 검사를 통과했다. 픽셀·화면 검증은 수행하지 않았다.

후보 SHA256은 `c617fdcbab90e174ff444f1bcf263677caca452245d76888fd14b6398180767e`다.
같은 out의 installation.json에 현재값/제안값7개와 baseline guard를, verification.json에
검증 결과를 기록했다. 실제 Data, Catalog/Tree와 builder는 수정하지 않았다. 사용자 편집과
live hash는 보존했고 실제 설치는 root의 등록 단계, 화면 판정은 사용자 단계로 남겨 두었다.

### G23 후속 — 비둘기 일렬 비행·반원 선회 후보와 Python 검증

현재 V1의 이동은 속도8m/s 직선 중심에 반경1m/주기3초/위상0·90·180·270도의 Orbit를
더한 형태다. 네 마리가 같은 경로를 뒤따르는 사용자 정정과 달라, 기존 native2893 mesh의
local-space와 sourceTransformTrack으로 후보를 만들었다. 각 새의 synthetic velocity/orbit
두 모듈만 제거하고 위치·접선 yaw 키를 추가했다. 날갯짓 SizeLife와 재질, 사용자 TRS/크기,
bloom,3초 수명·출생과 기존19개 폭발 요소 및 death event를 보존했다.

원본 Action4219802의 실제 바이너리와 SkillEffect DB,Projectile421980201~211을 읽었다.
stage2의1.1초4개/1.6초4개/2.1초7개 발사와 Missile의초기800cm/s·최대1500cm/s·거리1500cm,
수명3/3.5/5초를 구분했다. 독립4마리·3초의 반원 경로를 원본 전체 Action과 같다고 하지
않는다. 간격0.8m,선두0.7초 직진+2초 반원+0.3초 복귀 직진,후속0.1초씩 지연은
USER_REQUESTED/PROJECT_AUTHORED다. 반경은16/pi=5.092958m이며 네 마리 모두3초까지
180도 선회를 마친다. 기존 독립 effect와 같은8m/s×3초=24m를 유지하므로 원본15m cap을
복원한 projectile simulation으로 분류하지 않는다.

`build_kouku_dove_pizza_candidates.py`에 track/helper와 --dove-only를 추가했다. 최신 saved
문서를 먼저 읽어 다른 필드를 유지하며, 별도로 편집한 track/수명/불완전한 이동 모듈은
덮지 않고 거부한다. 같은 후보 재입력은 idempotent다. 현재 baseline SHA02137ef4fc3a4…와
후보 SHA9363c39c8b181… 및7필드 주사위 후보는 별도 유지한다. 원본 조사, 후보,변경명세는
out/CardDiceScale20260917/dove의 DESIGN.md,installation.json,source-action4219802.json,
source-skill-effects.json에 있다. live Data/Catalog/Tree는 이 작업에서 수정하지 않았다.

Python 검사에서23요소/19폭발의 보존,실제27리소스 존재,새 builder 결과의 JSON 완전 동등성,
재입력과3종 거부 시 입력 보존을 확인했다. 60Hz×181키×4새를 yaw0/90/180으로 평가한
2,172개 위치는 finite하며,같은 경로를0.1초 지연 추적하는 최대 오차는3.98e-15m다.
반원 선형 키 보간의 원곡선 최대 오차는0.437mm다. 끝위치(yaw0)는 각x3.2/4/4.8/5.6m,
y1.79999995m,z-10.18591636m이며3초 이후 키 clamp에도 변하지 않는다.

실제 C++의 Update_Particles는 현재 sample의 ElementWorld를 평가하고 terminal death를
제거 전에 queue한다. local-space event는 그 행렬로 world 위치를 만들며,폭발 receiver는
event의 world 위치를 자기 emitter의 inverse 행렬로 변환한다. 기존 receiver19개는
busepsyslocation=false,binheritvelocity=false이고3초에 활성 상태라 동일 event4개/기대
216birth 계약을 유지한다. 이는 코드 읽기와 Python 수치 검증이며 실제 C++ 실행 증거가 아니다.
사용자의 abort 창 보고 후 모든 새 probe EXE·Client/UI·제품 빌드를 실행하지 않았다.
actual Codec/Playback 재실행과 사용자 화면 판정은 미수행이다. Python syntax/JSON과
변경 source diff-check만 통과했다.

이후 root가 Windows/CRT/Engine 비대화형 오류 처리와 catch/terminate 경로를 읽고 승인한
단일 실제 C++ 검사를 수행했다. out의 dove_path_probe.cpp를 격리 컴파일·링크하고,
CREATE_NO_WINDOW/30초 제한으로 PID2124를16:40:16.787~16:40:18.464 KST에 한 번 실행했다.
종료코드는0이고 재시도하지 않았다. Codec Load/Drawable/Serialize/Parse/Stage,
실제 Particle.World 메시 접선 및 SourceEmitterWorld,위치,37도 회전+이동 root 공변성,
Seek/Reset/자연 종료를 포함한22,853 checks를 통과했다. 최대 위치 오차1.71926e-6m,
실제 메시·emitter 접선 오차2.98023e-7,root 오차2.24924e-6m다.

원본19개 폭발 수신기가 실제216입자를 생성했다. 별도 out 전용1:1 event marker4개도
3.03333초에 각 terminal 위치에서 생성됐고 최대 오차는4.76837e-7m다. marker 문서는
제품 effect에 추가하지 않았다. stderr에는 CRT 종료 시 memory-leak dump가 남았으며
assert/abort/exception은 없고 정상 종료했다. 누수 원인은 이번 검사로 확정하지 않았다.
실행 영수증과 증거는 같은 out의 actual-execution.json,actual-probe.log,
actual-probe.stderr.log,actual-verification.json에 있다. Live 설치,제품 빌드와 화면 검증은
수행하지 않았고 후보 SHA9363c39c8b181…를 유지한다.

### G22 추가 — 회전 카드 방출 수 2배 후보와 실제 소비 검증

Composition1237의 P48은 Logic74가 3533~7633ms 구간에서300ms마다3장씩14회, 총42장을
생성한다. countPerWave만3→6으로 변경한 후보는 같은14회에84장을 생성한다. source 초기
속도8m/s, 최대 거리15m, 수명5초, 비추적 정책, 문양4개와 접촉 폭발, 현재 클립6회 및 모든
ordinary presentation은 유지했다. 1236→1237 사용자 저장 차이는 P82 presentation 추가였고
P48/74에 영향이 없음을 확인한 뒤 최신1237 해시를 기준으로 후보를 다시 만들었다.

독립 `effect.kouku.card.spinning.emitter`는 주 심볼의 sourceScale.count만1→2로 변경한다.
원본3발 burst와 발사 시각, sourceRecipe·sourceModelPreview·생존 시간은 바꾸지 않았고,
폭발 receiver7개는 전체 행이 기존과 같다. 추가 카드가 각자의 death event를 발행하므로
폭발당 입자 수나 receiver 용량을 다시2배 하지 않는다. 두 데이터의 guarded patch는
`out/CardDiceScale20260917/count/semantic-patch.json`의2필드다. 공통224요소 연출의 방출
개수는 변경하지 않았다. standalone을 P48에 별도 추가하지 않아 같은 방출 경로를 중복하지 않는다.

`build_saydon_spinning_emitter_preview.py`는 명시적 `--count-multiplier {1,2}`(기본2)와
`--output`을 받으며 원작확장42발과 사용자84발 schedule을 분리 기록한다. 생성 마지막에
공통 카드 크기 helper를1회 호출해1.5배 조정도 유지한다. 실제 publisher projection에서
4개 visual template은 전후 동일하고 count3→6만 바뀌었다. C++·Shared 계약 변경은 없다.

root가 비대화형 오류 처리를 검토한 뒤 실행한 실제 Codec Load/Drawable/roundtrip/Stage와
Playback 검사는303,829 checks/0 failures, exit0이다. baseline42→후보84, 재생성84이며
최대 동시 카드는21→42다. 폭발7개 emitter의 총 출생 수는 각각 정확히2배이고 최대 동시
폭발 입자는360개로 기존 최대 용량1134를 넘지 않는다. 세 문서 모두10.208초 자연 종료와
빈 최종 frame, 같은14개 발사 시각·8m/s를 확인했다. CPU의 고정 root count 검증이며 실제
보스 부착·GPU 표현·시각 품질 검증으로 확대하지 않는다. Debug CRT의 종료 시 static
할당 dump가 남으므로 이 검사를 메모리 누수 없음의 증거로 사용하지 않는다.

최초 out 후보에서 불필요하게 receiver 용량1134→2268을 적용해 Codec 상한2048을 넘겼다.
이 Load 실패 뒤 기존 진단기가 계속 실행해 map.at 예외가 처리되지 않으면서 abort 창을
발생시켰다. 이는 제품 Client 오류가 아니라 후보·검사 프로그램의 결함이다. receiver 변경을
모두 제거했고 진단기는 첫 실패 즉시 중단하며 CRT assertion/abort, Windows 오류 및 Engine
오류를 비대화형 로그로 처리하도록 수정했다. root의 통제 이전에는 검사를 중단했고, 수정판
실행은 root가 수행했다. Client/Server 프로세스와 live Data는 이 하위 작업에서 변경하지 않았다.

입력 bytes/hash, 후보, 재생성 proof, publisher-projection.json과 guarded-count-final.log는
같은 count 폴더에 있으며 최종 상태는 verified.json에 있다. 실제 설치·publish는 root의
전체 등록 단계에서 별도로 수행한다. 사용자 화면 판정은 미실시다.

## G24. 상단 무지개 Solo 공급원 유지 — 소스 반영

Build_ElementsPreviewDocument가 particlemodulelocationemitterdirect 및 EF alias를 수집하지
않아 선택 sprite의 공 위치 공급원을 제거했다. 네 class의 활성 참조와 transform master를
재귀 수집하고 기존 provider validation을 전후 유지했다. 공급원은 simulation하며 기존
Set_SubmissionElementSet으로 선택한 sprite만 제출한다. Sequencer factory와 일반
Solo/Family/Group이 같은 함수를 사용한다. stale single/named group도 전체 재생으로 바뀌지 않는다.

Effect_Tool_Playback.cpp/Effect_Tool_Workspace.cpp 격리 컴파일 PASS. exact production 함수와
실제 Codec/Playback 및 renderer setter/predicate를 사용하는879검사0실패. actual rainbow
49프레임 Solo/full World·clock 일치, recursive6요소 closure와 disabled/stale/rollback 검사 통과.
진단은 Windows/CRT/Engine 비대화형 가드 후1회 종료0, Client/UI 실행 없음. 검증 정본은
out/CardDiceScale20260917/solo/verified.json이다. 제품 exe 교체와 사용자 visual 판정은 미실시.

## G25. 알비온 출현 시 대상 선택 — 소스 반영·현재 P79 검사 통과

현재 저장본 r1245의 P79는 JUMP(logic75,7148ms), APPEAR_PLAYER(logic76,8883ms),
SLAM(logic70,8883ms)을 갖고 SELECT_PLAYER는 없다. 새 사라지기 presentation.10은 순수
V1 Effect이며 airborne 수집기는 LogicOccurrences만 읽으므로 이펙트가 다른 패턴의 Logic을
상속한 것은 아니다. 실제 Server는 APPEAR에서 이전 선택이 없으면 살아 있는 대상을 고른다.
미리보기만 SELECT 선행을 강제해 같은 저장본을 거절한 것이 확인된 차이다.

PreviewRootMotion의 SELECT 필수 조건을 제거하고 JUMP 선행·native pose·남은 하강 검증은
보존했다. PresentationPlayer는 첫 APPEAR의 occurrence ID로 대상 identity를 기록하고 다음
APPEAR에서 현 위치를 읽으며, 대상이 사라졌을 때만 재선택한다. 되감기와 read-only sampling은
이미 기록한 위치를 유지한다. SELECT_PLAYER의 SELECT 정책만 기존 navigation ground를
고정한다. 삭제된 Logic, 사라지기 위치·크기·시각과 기타 live Data는 수정하지 않았다.

두 변경 TU와 최신 CompositionDocument 및 probe의 격리 컴파일·링크는 exit0이다.
Windows/CRT/Engine 오류창 차단과 catch/terminate를 root가 검토한 후1회 실행했으며
PID21056,16:56:00~16:56:02 KST,exit0,301검사를 통과했다. 실제 설치 MN_RPCT_05 CModel과
17_* clip은 현재3단계를 승인하고 목표(10,14.6788,10)에서 지면(10,1,10)으로 착지한다.
production 선택 블록의 noSELECT·기존 APPEAR·SELECT ground·이탈·재선택·되감기·빈 대상과
JUMP 누락/하강 없는 입력 거절 및 이전 phase 보존을 확인했다. 지면 sampler만 stub이고
모델·animation/height sampling은 실제 구현이다. stderr에는 CRT 종료 memory-leak dump가
남았으며 원인은 이번 범위에서 조사하지 않았다. assertion·abort·예외는 없었다.

근거·입력 hash·frozen Composition·컴파일/실행 로그와 검증 정본은
out/CardDiceScale20260917/albion/verification.json에 있다. 제품 빌드·Client/UI 실행·라이브
Data 등록과 사용자 화면 판정은 이 하위 작업에서 수행하지 않았다.

## G22~G25 통합 후보 상태 — Client 최종 저장·종료 확인 대기

카드/주사위 크기·42→84발의8문서, 비둘기1문서, 상단광1문서, 분열 child upper 제외
World1문서로 총11파일103변경 후보를 준비했다. 최신 Composition1245를 보존한1246,
World2036→2037 후보이며 live Data 쓰기는0회다. 카드 수 검증 당시1237과1245의 P48·Logic74가
동일한 것을 재확인해 unrelated P79 사용자 편집을 보존했다. source크기·수 후보, 비둘기actual
22,853검사, Solo879검사, 상단광1,283검사, Albion301검사와 변경TU 최소 컴파일을 통과했다.
전체World C++ Codec의268templates/456objects/324instances 및 실제 publisher reader를 검증했고,
정식 per-row publication closure의 P48/P78/P83은 ready다. 기존 P32는 stage없는 보관 패턴으로
격리되는 기존 정책 그대로이며 raw validate_document 전체 호출 실패를 제품 전체 실패로 간주하지 않았다.

동시에 추가된 builder의 별도 카드 조정/상승공 helper는 보존했다. 현재 helper로11후보를
다시 구성했을 때 기존 검증 후보의 모든 byte hash가 같음을 확인하고 source guard를 최신으로
고정했다. out/CardDiceScale20260917/registration-transaction.json은 readyForCommit=false,
installed=false다. 사용자 미저장 편집 보호에 필요한 마지막 Save+Client 종료 답변이 아직
없으므로 등록 script와 publisher는 실행하지 않았다. 제품 Client/Server 빌드·재시작 및
최종 화면 판정은 사용자 단계다. 서버 카드미로 목표1개 코드는 별도로 반영·최소 컴파일했다.


## G26. 카드 짝 맞추기 네 무늬 정렬·반복과 회전 카드 잔상 조정

### 원본 근거와 수정 범위

대상은 `effect.kouku.card.match.{heart,clover,diamond,spade}`, 회전 카드 네 무늬,
`effect.kouku.card.spinning.emitter`, `effect.kouku.common.spinning.card.throw`의 10문서다.
`Tools/EffectPipeline/build_saydon_card_pattern_groups.py`의 현재 저장 문서 patch 경로로
후보를 만들었다. 사용자가 지운 회전 카드 mesh·emitter를 원본 leaf 재생성으로 되살리지 않았다.
변경은 loop count 40개, 앞면 RGB 세기 4개, 고정축 owner 회전 flag 25개,
다이아 mesh 위치 성분 1개, 원본 잔상 occurrence visible 24개로 총 94필드다.

다이아의 mesh detail 위치 Y는 2.37016678, symbol_050은 0.680166662로 1.69m 차이가
있었다. 원본 StartLocation은 둘 다 UE (10, 0, 0)cm이므로 네 무늬 각각의 현재 앞면 배치를
기준으로 mesh 중심을 맞췄다. 나머지 세 무늬는 원래 같은 detail 위치였다. 네 무늬에 공통인
회전 불일치는 별도 원인이다. sourceTransformTrack이 없는 local-space EPAL_Z sprite가
owner 회전을 소비하지 않아 mesh만 회전했다. 기존 `followEmitterAxisRotation` 경로를
symbol_050·holomatrix·symbol_109 세 장에 연결했다. 회전 카드 파생 문서에도 같은 원리로
sk13_1의 해당 sprite만 연결했으며 독립 emitter의 폭발 레이어에는 전파하지 않았다.

원본 Required의 emitterloops는 무늬마다 12요소 중 2요소에만 1이 명시돼 있고 나머지
10요소에는 없다. Required→ParticleModule→Object까지 확인한 native 생략값 0을 복구해
네 무늬의 40개만 1→0으로 바꾸고 명시적인 1은 8개 모두 유지했다. 원본 mesh·050의
입자 수명 1초와 symbol_109의 2초는 그대로다. 기존 tracking occurrence의
`loopEffectToDuration`은 `CEffectPlayback::Set_SourceLoopEndSeconds`를 호출하므로
실제 반복 recipe가 있어야 admission과 지정 구간 반복이 성립한다. 입자 수명을 모두 늘리거나
whole-effect 무한 재생을 추가한 것이 아니다.

회색 앞면은 원본 native2967의 shader 식과 현재 경로가 일치하고 원본 DDS 자체에도 회색
영역이 있어, shader 누락만으로 원인을 확정할 근거가 없다. 이번 앞면 emissiveIntensity
1→4는 `PROJECT_AUTHORED_RGB_EXPOSURE_4`로 분류한 요청 범위의 RGB 보정이다.
원본 texture·native shader·masked alpha threshold는 유지했다. 원작의 최종 노출·후처리까지
완전히 복원했다거나 화면의 흰색이 원작과 일치한다고 판정하지 않는다.

회전 카드 원본에는 실제 world-space `sk_13_1_loc_int.particlespriteemitter_0`이 있으며
native2999/symbol_45, 수명 0.15초, SpawnPerUnit 30cm 간격의 잔상 방출이다.
사용자 요청에 따라 common 투척 문서의 이 occurrence 24개만 visible=false로 억제했다.
원본에 잔상이 없었다고 기록하지 않으며 smoke·ribbon은 보존했다. 이미 해당 emitter를 지운
사용자 저장 leaf와 네 무늬 문서에는 emitter를 다시 추가하지 않았다.

### 후보 자동 검증과 독립 수치 검증

`out/CardMatchFix20260917/static-verification.json`은 10문서/94필드 변경,
재적용 idempotent, 검증 시점 live 기준본 미변경을 기록한다. 실제 DocumentCodec의
Load→Validate_Drawable→Serialize→Parse→동일 재직렬화와 CEffectPlayback 및
Make_ParticleSpriteWorld CPU 경로를 연결한 비대화형 probe는 10문서 10,509,062검사,
failures=0이다. 최종 standalone은 sk13_1만 보정한 `spinning-emitter-final` 입력이다.
실행 증거는 `runtime-verification.json`, 개별 `*.execution.json`, stdout/stderr에 둔다.

창과 GPU draw 없이 root identity, Y축 90도 회전+이동, 복합 기울기+이동의 세 조건에서
두 번씩 Seek(0) 후 전체 구간을 재생했다. match는 실제 bounded loop API에 4.5초를 주고
1/120초 update를 입력했다. 이는 static root에서 기존 60Hz simulation의 transform·수명
검증이며 움직이는 owner의 화면·GPU·게임 전투 재현은 아니다. stderr의 CRT 종료 시
static allocation dump는 보존했고 leak-free 판정으로 바꾸지 않았다.

독립 분석은 설치 WModel의 실제 164정점과 CSV의 m00~m33를 사용했다. 이 mesh의
실제 카드 평면은 XZ, 법선은 local Y, 장축은 local Z다. 일반 quad용 normal 출력값을
mesh의 법선으로 오인하지 않았다. 근거와 입력 SHA는 `geometry-verification.json`에
보존했으며 status=NUMERICAL_PASS, failures=[]다.

| 검증 항목 | 실측 결과 |
|---|---|
| 다이아 mesh/050 particle anchor 거리 | 수정 전 1.68999978~1.69000012m, 수정 후 네 무늬 모두 0m |
| 실제 mesh geometry 중심과 050 draw 중심 | 수정 전 1.68844~1.68872m, 수정 후 17.9664~17.9668mm |
| 수정 후 평면·장축 동행 | 세 root/두 회차에서 `1-abs(dot)` 최대 각각 8.993e-15, 2.565e-14 |
| 원본 050 pivot 차이 | 18mm 편심 유지, 예상 pivot vector 대비 최대 오차 0.342μm |
| 원본 symbol_109 상대 위치 | (0.03, 0.02, 0)m 유지, 최대 오차 1.08μm |
| 원본 camera offset | 20mm 계층 차이 유지, sprite 계산 오차 최대 0.445μm |
| 회전 카드 네 무늬 | 각 366개 중복 제거 sample, root 회전에 따른 축 변환 최대 오차 3.159e-7 |
| common 카드 잔상 | source occurrence 24/24 숨김, CSV 38,040행 중 해당 emitter 방출 0행 |

mesh CSV에는 camera offset을 더하지 않은 Particle.World가 기록되므로 위 geometry 중심
수치는 그 범위를 명시한다. 별도로 native mesh -20mm camera offset을 해석적으로 더한
mesh/050 draw 중심 거리는 12.75~27.41mm다. 이 값은 원본 pivot과 layer offset의 결과이며
중심을 무조건 0으로 만들기 위한 보정 대상이 아니다.

mesh+050+symbol_109 세 장의 필수 5요소는 각 root·회차의 0<t<4.5초 구간에서
269개 fixed-step 시각 모두 존재했다. 1초·2초 경계 이후를 포함해 누락 시각은 0개다.
CSV가 같은 simulation frame을 두 번 기록하는 중복은 제거했다. 다만 원본 loop의 수명
경계에는 구입자와 새 입자가 1~2 fixed tick 공존하며 요소당 count 1 또는 2를 관측했다.
무늬마다 세 root·두 회차 합계 36개 경계 sample에 중첩이 있으므로 단일 입자 유지나
화면 무중첩을 통과한 것으로 기록하지 않는다. holomatrix는 이 필수 5요소 CSV 선택에
포함되지 않았으며 해당 요소의 연속성 수치로 확대하지 않는다.

### 저장 기준본 반영과 남은 경계

위 검증은 처음에 live Data 미설치 후보로 완료했으며 미저장 draft 보호를 위해 외부 등록을
보류했다. 이후 사용자가 “오케이 지금 기준으로 반영해도 돼”라고 현재 저장 기준본 적용을
승인했다. 최신 baseline SHA를 재확인한 뒤 카드 10문서와 별도 공 1문서·등록 5문서의
16파일 설치가 완료됐고 기존 15파일은 백업했다. Composition은 1246→1247이다.
현재 설치 증거는 `out/CardMatchFix20260917/installation.json`의 installed=true이며,
기존 `stage.json`의 installed=false는 후보 생성 당시 상태다. 이 절의 검증 범위는 카드
10문서이고 별도 공·등록·제품 publisher의 완료 증거는 담당 후속 기록과 구분한다.

제품 publisher 실행·Client의 현재 draft reload·최종 화면 확인을 이 수치 검증으로
대체하지 않는다. Client/UI 자율 실행·캡처와 visual PASS는 없으며 네 무늬의 흰 앞면,
정렬, 1초·2초 경계, 회전 투척의 잔상 여부는 사용자가 최종 판정한다.

## G27. 쓰리투원투하 공 발사 설치와 검증

사용자 첨부 화면의 상승 공과 하단 무지개·별을 기준으로
`effect.kouku.gate1.circus.ball.launch`를 설치했다. 표시 이름은
`세이튼 / 쓰리투원투하 | 공 발사`이며 V1 트리의
`KoukuSaydon / 1관문 / 패턴 / 세이튼 / 쓰리투원투하`에 등록했다.
Catalog·Tree·Composition resource와 Client 프로젝트의 `96.DataFiles` None을 연결했다.
기존 패턴에 임의 occurrence를 추가하지 않았으며 현재 saved pattern·World 배치는 보존했다.

기존 `rainbow.drop`의 저장된 ball_04 네 요소만 사용했다. 공 mesh5, 무지개30,
별 mesh33·sprite34의 기존 native 재질·texture·입자 수명·사용자 sourceScale.size=0.7을
보존했다. 사용자가 삭제한 sprite35와 낙하 도넛 폭발은 추가하지 않았다. 설치 공 메시의
290정점과 geometryPreScale을 적용한 지름은 약0.659127m로 기존 낙하 공과 같다.
원본 LocationDirect의 UE cm `[0,0,1000] → [0,0,-50]`을 뒤집어0.8초에10.5m 상승시킨다.
시작 공 하단은 effect root 높이이며 이 역궤적은 사용자 요청의 PROJECT_AUTHORED 변형이다.

무지개는 같은 공의 LocationEmitterDirect를 유지하고 공 하단 -0.329563318m의 Orbit
offset과 Required.offsetcentery=0을 사용해 아래로 뻗는다. 기존 +Y velocity는 방향 입력이고
공 위치는 Direct provider가 소유한다. 두 별은 같은 mesh stable ID에서 공 하단 위치로
spawn한 뒤 원본 world-space 운동·중력·fade를 유지한다. 잔여 별 수명을 포함한 등록 길이는
2200ms다. 원래 낙하 문서의 SHA256은 후보 생성 전후와 설치 후 모두 동일하다.

`Tools/EffectPipeline/build_saydon_circus_world.py --launch-only`에 별도 out 후보 생성을
추가했다. 실제 Codec/Drawable/typed roundtrip/Playback/Geometry 검사20,297개가 failures=0으로
끝났다. 공·무지개98개 시간 표본에서10.5m 상승, sourceScale0.7, 동시 생존·종료를 확인했고
하단 부착 오차는0.298μm, 실제 quad 상단 오차는0.477μm 이하다. 두 별 carrier의 최대 활성
입자는34/39이며4초 검사 종료 시 모든 입자가 정리됐다. 창·GPU draw·Client 실행은0회다.
독립 검토도 설치본과 검사 후보 SHA, provider3개, 실제 리소스14개와 등록 중복 없음에 일치했다.

증거는 `out/CardMatchFix20260917/ball-launch/launch.execution.json`, `launch.stdout.log`,
`launch.csv`, `candidate.receipt.json`, `registration-stage.json`, `installation.json`이다.
후보 receipt의 pending/installed=false는 생성 시점 상태이며 현재 설치 상태는 마지막
installation receipt를 따른다. 임의 재질·리소스 신규 제작이나 제품 C++·shader 수정은 없다.
최종 무지개 색·별 모양·발사 화면의 일치는 사용자 육안 확인 전이다.

사용자 확인 경로는 F1 → `Effect Tool V1` → `Refresh Resources` → 위 분류의
`세이튼 / 쓰리투원투하 | 공 발사`다. 이미 열려 있는 카드는 `Load Saved`로 설치본을 다시
읽는다. Action Workbench에서는 Boss `KoukuSaydon`의 `Reload`와 `Refresh Resources`로
Composition1247의 등록을 읽는다. 현재 Client31396·Server34628은 계속 실행 중이며
에이전트가 종료·재시작·화면 조작하지 않았다. 이번 데이터 변경만을 위한 제품 재빌드는 없다.

### G26·G27 최종 반영 확인

사용자 승인 후 `Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon
-ExpectedKoukuSaydonSourceRevision 1247`이 exit0으로 끝났다. `koukusaydon.product`,
`world.gameplay`, `gameplay.balance`는 PASS, `map.kakulsaydon`은 기존 산출물 REUSED다.
설치16파일의 SHA는 publisher 종료 뒤에도 검증한 후보와 모두 일치했다. 설치 JSON/XML16개,
생성 Encounter·patternbindings JSON, 생성기4개의 Python AST와 변경 범위 `git diff --check`를
확인했다. 제품 revision1247의 생성 결과는 saved84/product71패턴·product413stage이며,
이는 기존 보관 패턴 격리 정책을 포함하는 publisher 결과다.

최종 증거는 `out/CardMatchFix20260917/installation.json`의
`installed=true`, `runtimePublication=PASS`, `liveCandidateHashesVerifiedAfterPublish=true`와
`publish.log`다. 현재 실행 중인 Server의 다음 패턴 admission·Client의 문서 Reload·사용자
최종 화면 판정은 실행하지 않았다. 원본 카드의 정확한 최종 노출값은 미확정이며 앞면의4배
RGB 보정과 원본 카드 잔상 억제는 계속 사용자 요청의 PROJECT_AUTHORED 변경으로 구분한다.

## G28. 최신 저장본 병합 설치와 실행 중 데이터 반영 절차 교정

사용자는 다른 세션 종료 후 데이터 반영을 승인했고 다시 교체를 명시했다. 이전에 요구한
Client 종료는 데이터 파일 교체의 필수 조건이 아니었다. out 설치기의 프로세스 실행 여부
차단을 제거하고 명시적 적용 승인, 최신 입력 hash·백업 일치, 경로 범위, JSON parse,
교체 직전 동시 저장 검사와 파일별 원자적 교체·자기 변경 rollback을 유지했다. 이전 절의
종료 대기 기록은 당시 상태이며 현재 절차·설치 상태는 이 절을 따른다. AGENTS를 정본으로
CLAUDE·팀 사용서·gotchas·복원 문서·진행 PLAN도 같은 경계로 교정했다.

최신 Composition1247을 기준으로 11개 저장 문서의 103개 변경만 병합해 실제 설치했다.
Composition은1248, WorldSequence는2037이다. 다른 세션의 카드 local 축 회전·무한 반복,
앞면 밝기, 숨긴 원본 잔상24개, 공 발사 등록과 presentationResources260개를 보존했다.
역방향 차이 비교로 나머지 필드가 현재 저장본과 동일함을 확인했다.

- 회전 카드의 본체·잔상 크기1.5배와 P48 방출42→84발, standalone emitter 동시 반영.
  다른 세션에서 숨긴 잔상은 크기만 변경하고 숨김을 유지한다.
- 주사위 네 단계와 위 다이아 세 요소 크기1.5배.
- 종이 비둘기 네 마리는 같은 직선→반원 경로를 간격을 두고 따르며 기존 종료 폭발19요소 유지.
  반경·간격은 사용자 요청의 저작 궤적이며 원본 확정값으로 간주하지 않는다.
- 낙하 공 상단 무지개의 velocity-facing 입력과 상단 표시 pivot 반영.
- 공 분열g1~g5의 상단 무지개만 제거하고 g0·shot과 전체 공 운동·충돌 폭발을 보존.

최종 합성 문서의 실제 Playback count 검사는182,319개·실패0·exit0이다. P48/P78/P79/P83은
정식 saved inventory와 dependency closure를 거친 publication 검사에 모두 통과했다.
이는 GPU 화면 판정이 아니다. 근거는 `out/CardDiceScale20260917/merge/verification.json`,
`merge/count-final-verification.json`, `ball/world-validation/selected-publication-readiness.json`,
`installed-registration.json`이다. 최신 receipt의 installed=true가 현재 설치 상태이며,
이전 후보의 installed=false를 설치 미완료로 해석하지 않는다.

기존 Albion preview·Solo provider·카드미로1개 수정은 소스와 격리 컴파일 검증 상태를 유지한다.
이번 데이터 설치를 위해 추가 C++·shader 빌드나 Client 종료·재시작·UI 조작을 수행하지 않았다.
실행 중 도구의 draft Reload와 Server가 소비한 상태, 최종 화면 확인은 설치와 구분한다.


G28 runtime publish 완료: `Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon
-ExpectedKoukuSaydonSourceRevision 1248`은 exit0이며 product/map/world/balance 네 domain이
모두 PASS다. 생성 Encounter·patternbindings의 sourceRevision1248 및 런타임 WorldSequence
revision2037을 읽어 확인했고, 설치11문서의 SHA는 게시 후에도 후보와 일치한다. 설치 JSON
parse와 `git diff --check`도 통과했다. 최종 receipt의 runtimePublication=PASS를 따른다.

피자 추가 확인: 원본 exp03 문서는16요소, 현재 통합본은 사용자 편집19요소이며 기존 G20
보존 SHA4074dd36…과 동일하다. 이번11파일 교체에 피자는 포함되지 않았다. 검정 위치·회전
보정이 완료됐다는 뜻이 아니며 기존 사용자 배치를 보존한 상태다. 원본 바닥 축 잠금에는
추가90도 회전 근거가 없고, fixed-axis sprite의 일반 Transform 회전과 실제 quad 회전은
followEmitterAxisRotation/roll 소비에 따라 구분해야 한다.


피자 연결 상태 추가 확인: Composition1248의 새 통합 피자 resource
`kakulsaydon.effect.8c73c44bdddbf9001b0a`는 등록돼 있지만 presentation occurrence 참조는0개다.
P25의 현재 타임라인은 기존 `boss.kouku.pizza.*` 리소스를 사용한다. 따라서 새 통합 피자의
등록 존재, 사용자19요소 편집 보존, 실제 패턴 연결과 검정 배치 보정 완료를 구분한다.
원본16요소를 현재19요소 위에 덮거나 검증 없이 기존 패턴 연결을 교체하지 않았다.

## G29. 피자 원본 조합 복원본과 저작 원형 경계

### 원본 재질과 검정 외곽의 원인

새 후보는 `effect.kouku.pizza.explosion.group.source-restored`, 표시 이름은
`쿠크_피자 | 피자 부채꼴 - 원본 검정 무지개 경계 폭발 2`다. 기존 사용자19요소 문서와
원본 exp03 leaf는 수정하지 않고 원본16요소의 별도 문서를 만들었다. 원본의 두 native3171
dark-aura, 서로 다른 두 flow emitter, 원본 native MIC·DDS·색상·TRS를 사용한다. 이미 원본
바이너리/CDO로 확인한 빈 Cylinder radius·velocity distribution 보완만 함께 적용했다.

원본3171 PS의 alpha는 UV analytic falloff와 ParticleColor.a로 결정된다. 두 noise texture는
RGB에 참여하고 전체 효과의 원형 world clip을 만들지 않는다. 원본 sparse ColorScale의
생략값0에 따른 검정색도 유지했다. source3280의 ring opacity와 radial UV fade는 해당 flow의
재질 경계이며 별도3171 sprite 전체를 잘라 주는 stencil 또는 공통 mask가 아니다.

설치 sphere002/003의 실제 WModel 정점, geometryPreScale .01, .2초 Playback 행렬로 측정한
무지개 외곽 반경은6.599988~6.600002m다. `dark-world-coverage.json`은 기존 실제 Playback·
sprite draw 행렬과 원본3171 alpha식을 결합한 측정이다. 원본 두 dark layer로 돌아가도 .2초
alpha>=.1 영역의 최대반경은8.501m이고 해당 sample의24.31%가 반경6.6m 밖이다. .4초의
최대반경은10.033m다. 사용자 다섯 복제본은 .2초 최대반경13.359m, 바깥 비율50.65%다.
따라서 원본16요소를 다시 복사하는 것만으로 요청된 원형 경계를 충족한다고 처리하지 않았다.

### Document에서 기존 particle shader까지 연결한 범위

추가 원형 제한은 원본에서 회수한 shader 기능이 아니라 요청된 `PROJECT_AUTHORED`
coverage다. 새 복원본의 두3171 요소 `saydon.eae7033888f6af67ecba53ca`,
`saydon.f6bff2e4b02def707b4fce98`에만 반경6.6m와 안쪽 feather .05m를 저장했다.
source leaf와 기존 사용자 문서는 이 옵션을 가지지 않는다.

| 파일 | 실제 변경 책임 |
|---|---|
| `Effect_AuthoringDocument.h` | sprite의 `OwnerRadialMask` 값과 기본값·범위·지원 carrier 계약 |
| `Effect_DocumentCodec_DetailIo.cpp`, `Effect_DocumentCodec_Validation.cpp` | optional JSON 읽기·저장과 잘못된 값·미지원 carrier 거부 |
| `Effect_Tool_MaterialDetail.cpp` | 지원 source3171 sprite의 원형 경계 편집과 비활성화 시 기본값 복원 |
| `Effect_DocumentRenderer_Particles.cpp` | Effect origin 역행렬 생성·검증 및 매 draw의 enabled·역행렬·반경 바인딩 |
| `Shader_EffectParticleFamilyCarrier.hlsli` | 기존 group3136의 opt-in coverage를 원본 PS 뒤, bloom 출력 전에 적용 |
| `build_kouku_dove_pizza_candidates.py` | `--pizza-source-restored`의 독립16요소 후보·근거 receipt 생성 |

JSON은 `detail.sprite.ownerRadialMask`다. disabled/default인 기존 문서는 이 필드를 저장하지
않는다. 반경은 유한한0초과10000이하, feather는0이상반경이하, centerXZ는 finite여야 한다.
지원 범위는 source recipe를 가진 native3171 sprite particle의 기존 alpha/depth-read carrier다.
다른 재질·mesh·trail·compiled material adapter로 활성화하는 문서는 validation이 거부한다.
새 C++ 파일·family·runtime은 추가하지 않았으므로 프로젝트 소스 등록 추가는 없다.

`OwnerRadialMask`의 좌표는 개별 sprite 중심이 아닌 Effect origin XZ다. mask-to-world는
ParticleSystem uniform scale·yaw 뒤에 Frame.RootWorld를 곱한 행렬이다. owner/cue root scale과
전역 particle scale은 각각 한 번 소비하고 StartSize·개별 element scale은 공통 경계를 키우지
않는다. 비가역·non-finite transform은 실패로 분리한다. 원본3171 PS 자체는 보존하고 결과의
alpha·distortion에 coverage를 적용한다. RT0 RGB는 straight alpha 그대로이며 RT2 bloom도
coverage 적용 후 alpha를 받는다. 기본 비활성 경로는 기존 출력과 같다.

### 실행한 자동 검증

후보 JSON parse, 생성기 Python compile, 변경 범위 `git diff --check`를 통과했다. 새 struct의
ABI를 사용하는 Codec·Playback·geometry helper closure36개 TU와 변경 particle renderer·
MaterialDetail·probe를 현재 header로 다시 컴파일했다. production group3136 HLSL의 fxc
compile도 성공했다. 기존 native dispatch의 X4000 경고는 compile log에 보존했으며 경고가
없었다고 기록하지 않는다. 산출물과 source SHA는 `out/PizzaMaskRestoration20260917`에 둔다.

root가 검토한 `Run-GuardedMaskProbe.ps1`로 CPU와 GPU를 각각 한 번 실행했다. 최종 runId는
`3d55489c-61b9-482b-9e2c-1bc7f78b8482`이며 둘 다 exit0, timeout=false다. CPU는881ms,
GPU는120ms였다. 45초 제한, 숨김·no-window 실행, parent/child error dialog 억제,
명시적 Data/Resources root, 입력 SHA 전후 일치와 단일 실행 receipt를 확인했다. CPU50개,
GPU19개 입력 hash가 실행 전후 일치했다.

| 검증 경로 | 실제 결과 |
|---|---|
| 실제 Codec Load→Drawable 검증→직렬화→재파싱 | 후보16요소·mask2개 유지, 재직렬화 일치, 기존 문서의 disabled mask 생략 |
| 실패 입력 | 반경0·음수·NaN·상한초과, 잘못된 feather, 미지원 carrier, disabled 비기본값 거부 |
| Effect origin 역변환 | identity·이동·XYZ회전·비등방·음수 축 root와 전역 scale .125/1/3의15조합, 최대 복원 오차3.41197e-05m |
| 실제 Playback와 sprite draw transform | .05/.2/.4초의 원본 dark particle6개, 바깥54,143 sample의 최종 alpha 모두0 |
| CPU 종합 | checks=226, maskParticles=6, outsideAlpha=0 |
| WARP offscreen8조건 | 비활성·중심·안쪽·feather·경계·바깥·scale/이동 조건 통과, alpha 최대 오차6.55651093e-06 |
| GPU MRT | RT0와 RT2의 경계·바깥 alpha 정확히0, feather alpha1.1419524, 중심2.28390479, RT1은 원본3171의0 유지 |

GPU probe는 실제 두 source DDS와 native3171 PS, production `PS_COVERED_MATERIAL`,
`PS_MAIN`, bloom 함수의 추출 코드를 사용했다. 1×1 offscreen MRT의 raw shader 출력을 읽었고
비활성·중심 출력 byte 일치도 확인했다. 이것은 실제 Client의 draw batching·blend·depth·scene
후처리 전체나 카메라 시각 일치 검증이 아니다. raw alpha가1을 넘는 것은 원본 particle alpha
입력 결과이며 최종 scene alpha 또는 화면 밝기로 해석하지 않는다.

첫 runId `07b38993-9f7a-4cdb-b032-6acfcef21d6d`의 두 실행은 native main 전에
STATUS_ENTRYPOINT_NOT_FOUND(-1073741511)로 끝났다. EXE가 import한
`Set_NonInteractiveErrorMode`가 구형 `Client/Bin/Engine.dll`에는 없고 Debug DLL에는 있음을
dumpbin으로 확인했다. PATH의 Debug 우선순위와 실제 Engine·직접 dependency DLL SHA를
수정해 새 runId로 검증했으며 이전 실패 receipt·manifest를 보존했다. 이 첫 실패는 데이터나
mask 검사 실패로 분류하지 않는다. 최종 두 stderr의 CRT 종료 allocation dump도 보존했고
leak-free 판정은 하지 않았다.

### 후보 보존과 남은 확인

위 검증 시점의 후보 SHA는 `284565965e92dba11c467da2179538ed197f9c8110f7867b759feb339bec98bd`다.
기존 사용자19요소 SHA4074dd36…와 source leaf SHA2cf2a74d…는 검증 전후 유지됐다.
이 절은 후보·코드·자동 수치 검증의 완료 증거이며 최종 등록·설치·domain publish는 후속
반영 기록으로 구분한다. 기존 패턴 occurrence를 자동 교체하지 않았고 취소된 비둘기 추가
요청은 이번 후보 범위에 포함하지 않았다. Client/UI 실행·캡처·자동 Reload는 수행하지 않았으며
실제 원형 검정 경계와 원작 화면의 최종 시각 판정은 사용자가 직접 확인한다.

### G29 최종 저장본 반영·게시

2026-09-17 19:15:42 KST에 최종 디스크 Composition revision1304를 다시 읽어, 새 피자
resource와 모자 WORLD를 병합한 revision1306을 설치했다. WorldSequence는2051이다.
사용자가 작업 중 저장한 앞선 revision을 덮어쓰지 않았으며, 사용자19요소 피자는 원래
SHA4074dd36… 그대로다. 최종 반영 뒤 설치7파일 SHA가 후보와 같고 독립 검토에서도
기존 항목 보존을 확인했다. Catalog·Tree·Composition resource·Client project/filter에는
새 복원본만 추가했다. 기존 피자 패턴 occurrence의 자동 교체는 하지 않았다.

후보 검증은 실제 P83 publisher의 source/product admission과 두 projection, 실제 전체
WORLD reader를 사용했다. PS5의 41,922 key 검증이60초를 넘긴 첫 시도는 timeout이며,
조건을 바꾸지 않고180초 제한에서 통과했다. 이후 Composition만 바뀐 경우에는 같은
World bytes·원본 검증4함수·실제 외부입력의 fingerprint를 대조해 그 PASS를 재사용하고,
P83 검증은 새 저장본마다 다시 실행했다. cache의 초기 근거는 완료된 실제 PASS와 동일
World/함수, reader 준비 전 외부입력 mtime 및 두 post-run fingerprint다. 이를 소급한
pre-start content hash라고 기록하지 않는다.

`Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon -ExpectedKoukuSaydonSourceRevision 1306`
은 exit0으로 끝났고 product/map/world.gameplay/gameplay.balance 네 domain이 모두 PASS다.
실제 게시 P83와 검증 후보가 같고, runtime World의 모자 resource/template/instance도
후보와 일치한다. WORLD cue는 patternbindings의 일반 presentationOccurrences가 아니라
Encounter의 patterns[].worldSequences에서 소비된다는 계약에 따라 최종 검증했다.
기록은 `out/PizzaTrailHatIntegration20260917/{installation,installed-verification}.json`과
`publish-1306.log`다.

Client.exe·Engine.dll·설치 Trail CSO의 시작/종료 SHA는 같다. 격리 컴파일·수치 검증과
저장/게시 완료를 제품 전체 빌드 또는 실행 중 메모리 갱신으로 취급하지 않는다. Client와
Server 종료·재시작·Reload·화면 조작은 하지 않았다. 새 C++와 셰이더는 다음 제품 빌드,
게시된 Server 데이터는 다음 Server 재시작 뒤 소비한다.


## G32. 추적 중심·비둘기 거리·거미 장판 시작점 회전 (2026-09-18)

### 현재 완료 경계

소스 코드 수정, 데이터 후보 생성, 변경 TU 격리 컴파일, 실제 Codec/Playback/quad 수치 검증은
완료했다. 이 절 작성 시점에는 편집 중인 사용자에게 최종 저장본 반영 승인을 요청한 상태이며
Authored/Composition 교체·domain publish·Product EXE 링크는 아직 하지 않았다. 최종 설치
상태는 아래 후속 반영 기록만 기준으로 한다. Client/UI 실행·화면 캡처·육안 판정은 하지 않았다.

시작 브랜치는 codex/shader-build-isolation-20260917, HEAD는
`a198c9406db46b92f8b8aeaf087a538733f259ea`다. 기존 pursuit Preview/Server, 카드4문양,
Composition, project/filter와 다른 세션의 발탄 변경을 보존했다. 소유권이 섞인 작업트리라
자동 stage/commit/push하지 않았다. LAN 스크립트는 server-host/reachable/방화벽 ready였다.

### G32-01. GameRoom_BossSimulation.cpp와 P79 선택 지점

`Commit_KoukuAlbionAirborne`가 플레이어 선택·지면 좌표·보스 Transform을 소유한다.
APPEAR_PLAYER에서 기존 SelectedGround가 있으면 그 XZ를, 없으면 살아 있는 선택 대상의
현재 XZ를 읽는다. navigation의 exact walkability와 지면 Y, boss body collision을 검증한다.
높이는 지면에 airborneHeightM를 더한다. stage root origin과 ground origin도 같은 delta로
옮겨 다음 root-motion sample이 teleport를 되돌리지 않게 하고 staged boss를 commit한다.
실패 시 그 호출의 원래 boss pose를 유지한다. Server transform은 snapshot으로 Client에 전달된다.

이전 Claude 설명의 “selectedEffectGroupId는 Preview 전용”은 현재 코드와 다르다.
`project_kouku_saydon_composition.py::_project_airborne_selected_effects`가 그룹의 네 행을
하나의 `selectedEffectVisualId` template으로 변환한다. Server의 단일 visual ID는 한 개의
이펙트가 아니라 네 occurrence 전체를 식별한다. `combatobject.kouku.showtime.fixed`의
기존 고정 지점 표현을 사용하며, 그 이름이 쇼타임이어도 신규 별도 runtime은 필요하지 않다.

후보는 Logic76의 기존 8883ms 추적 시각에 SELECT_PLAYER 트리거를 하나 추가한다.
논리 ID와 occurrence ID는 최종 병합 시 최신 next ordinal에서 할당한다. 핵심 저작값은:

```json
{
  "logicType": "TRIGGER",
  "triggerKind": "ALBION_AIRBORNE",
  "airbornePhase": "SELECT_PLAYER",
  "airborneHeightM": 0,
  "airborneDurationMs": 0,
  "teleportPosition": [0, 0, 0],
  "airborneTargetPositionPolicy": "SELECT",
  "selectedEffectGroupId": "KAKULSAYDON_G1_PATTERN_79.effectgroup.11"
}
```

Server와 `KoukuSaydonPreviewRootMotion::Prepare_Airborne` 모두 같은 시각의 SELECT를
APPEAR보다 먼저 처리한다. 따라서 별개의 대상 추정 없이 같은 캡처 지점을 공유한다.
Server SELECT는 navigation ground를 저장하고 fixed combat object를 생성한다.
APPEAR는 그 ground 위 높이13.6788133052m에 보스를 배치한다. 장판은 공중 보스 높이를
사용하지 않고 ground에 남는다. 이후 플레이어·보스의 이동이 장판을 끌고 가지 않는다.
각 트리거는 별도 transaction이므로 SELECT 성공 뒤 APPEAR의 body 검사만 실패하면
장판은 생성됐지만 보스 이동은 거절될 수 있다. 한 transaction으로 바꿨다고 주장하지 않는다.

Preview 경로는 `airborneSelections`의 같은 occurrence ID →
`Sample_LogicPreview` → `Build_SelectedAirbornePresentation` → 기존 `Sample`이다.
Server Play는 publisher template → Server fixed object spawn →
`Read_TargetedCombatVisuals` → 같은 `Sample`이다. 원래 MAP 행은 소유된 template으로
분리되어 일반 presentation에서 중복 재생되지 않는다.

네 행 presentation5/6/7/8의 시간·길이·TRS는 보존한다. 첫 행의 원래 XYZ를 공통 기준으로
빼고 캡처 ground를 더한다. 현재 네 위치가 같아 상대 위치는 모두 [0,0,0]이며 Logic 대비
지연은24/1779/2540/3095ms, 전체 template 수명은4083ms다. Server의 실제 시작은30Hz
tick으로 양자화된다. 단순 BOSS anchor는 각 행의 다른 startMs와 공중 Y를 샘플하므로
그 대안은 적용하지 않았다.

### G32-02. build_kouku_dove_pizza_candidates.py의 실제 거리

`dove_single_file_track`이 네 local-space 새의 sourceTransformTrack을 만든다.
`travel_time = time - lane * spacing / speed`로0.1초씩 같은 경로를 뒤따른다.
source 좌표는 cm, Client 변환은 기존 Track sampler가 한 번 수행한다.

이전 builder는2/3 속도의 후보를 만들지만 실제 Data는 아직8m/s×0.7초=5.6m인 이전
track이었다. 사용자 최종 표현인 “절반”을 실제 저장본 기준으로 적용했다.

| 선두 구간 | 기존 저장본 | 이번 후보 |
|---|---:|---:|
| 초기 직선 | 0.7초,5.6m | 0.35초,2.8m |
| 반원 선회 | 2초,반경16/πm | 동일 |
| 선회 뒤 직선 | 0.3초,2.4m | 0.65초,5.2m |
| 속도·새 수명 | 8m/s,3초 | 동일 |

코드는0.5 정책일 때 `straight *= straight_scale`로 직선 시간을 줄이고
`straight_speed = speed`를 유지한다. 옛1 및2/3 track은 안전한 migration 인식용으로만
받는다. 독립 편집한 모르는 track은 거절한다. 과거 builder가 매번 밝기를4로 되돌리던 부분도
제거해 현재 저장된 밝기1을 유지한다. 경로 요청 때문에 다른 저작값을 다시 적용하지 않는다.
재질·resource·날갯짓·출생·수명·기존19개 폭발 요소·death event는 바꾸지 않는다.

`CEffectPlayback::Evaluate_ElementWorld`가 Track을 local Transform과 합성한다.
죽는 순간 기존 death event가 최종 world 위치를 전달하고19개 receiver가 그 위치에서
폭발한다. 폭발의 별도 고정 위치나 신규 projectile simulation을 만들지 않는다.

### G32-03. Effect_Tool.h / Internal.h / Helpers.cpp / Detail.cpp

사용자가 고른 회전축은 “쿠크 쪽 시작 끝점”이다. 거미 stage2 FullRestore의 바닥은
notify008의5요소이며 body-follow notify007의4요소와 다른 anchor group이다.
바닥은 follow=false captured root, snapshotRootSourceBasisYawDegrees=-90을 사용한다.
주 검은 띠2개(emitters19/25)는 Required offsetCenter=[0.5,1]이어서 시작 끝점이 emitter의
XZ 원점에 놓인다. source StartLocation의5cm는 지면 위 높이이며 yaw 회전의 XZ 축을 옮기지 않는다.

기존 Group Center는 Detail.Transform.position의 산술평균일 뿐 texture의 시각적 중심이나
끝점을 계산하지 않는다. 기존 group helper는 captured root를 편집 불가로 두었다.
또한 원본 EPAL_Z → Client 고정 +Y sprite는 opt-in 없이 emitter 방향을 소비하지 않는다.
이는 각 sprite의 내부 회전/roll과 effect 배치 방향을 혼동하게 하는 두 가지 별도 원인이다.

추가된 선언·상태는 다음 역할이다.

- `Rotate_AttachmentElementGroup(..., const float3_t* pivot, const std::string& elementId)`:
  pivot=null이면 기존 그룹 중심, 지정하면 같은 local m 좌표를 축으로 사용한다.
  elementId가 비었으면 전체 그룹, 있으면 그 멤버 하나만 선택한다.
- `GROUP_ROTATION_EDIT_STATE`: pivotMode(중심/anchor원점/custom), customPivot(m),
  elementId를 도구 세션에서 소유한다. `m_GroupRotationEdits`는 effect ID와 stable 첫 멤버 ID로
  구분한다. runtime 데이터나 별도 누적 rotation owner가 아니다.
- `Try_RotateAttachmentGroup`: 미적용 Detail draft를 보호하고 복사한 문서를 helper에 전달한 뒤
  기존 Try_CommitDocument로 한 번 교체한다. 실패하면 기존 문서·preview를 보존한다.

회전 helper의 실질 계산은 `delta = inverse(oldRotation) * newRotation`,
`p' = pivot + delta(p - pivot)`이다. 멤버의 방향뿐 아니라 위치, velocity,
position/velocity lerp 끝점도 함께 바꾼다. 전체 batch의 finite/range 검증 후 commit한다.
단일 멤버일 때는 그 멤버의 기존 orientation을 delta 기준으로 사용한다.
source track·runtime carrier·transform inheritance 및 animated rotation owner 제한은 유지한다.

바닥5개에만 `detail.sprite.followEmitterAxisRotation=true`를 연결한다. Playback이 local
입자는 현재 emitter, world 입자는 birth emitter 행렬을 전달하고
`Effect_DocumentRenderer_GeometryHelpers::Make_ParticleSpriteWorld`가 고정 sprite 축에
회전을 합성한다. 입자 내부 roll, 원본 pivot, snapshot source basis, body-follow4개는 보존한다.
Source basis까지 올바로 소비하므로 opt-off였던 바닥의 기본 방향 자체도 기존 결과와 달라질 수 있다.
이는 전역 -90도 추가 보정이 아니라 원래 누락된 해당5개 carrier의 basis 소비다.

사용 순서는 F1 → Open Effect Tool V1 → 해당 거미 stage2 Current Effect → Group by anchor →
Captured root 그룹 → Rotation target=Whole group → Rotation pivot=Anchor origin →
Rotation about pivot의 Y(deg) 편집 → Save Changes다. 같은 입력에서 target을 element ID로
바꾸면 한 요소만 쿠크 쪽 원점 주위로 돌린다. Custom point는 정확한 local 좌표를 지정할 때 쓴다.
입자 내부 roll은 전체 배치 회전이 아니므로 이 경로와 구분한다.

Pivot 선택은 세션 상태이며 Save Changes는 결과 Element TRS를 저장한다. 재로드 후 결과 방향은
유지되지만 편집 pivot 모드는 저장되지 않는다. JSON schema와 공용 Effect ABI는 변경하지 않았다.
새 파일·vcxproj/filter 등록도 없다. 사용자 각도를 지정받지 않았으므로 임의의90도 회전을
저장본에 넣지 않고 직접 조절할 기능과 올바른 회전 소비를 연결했다.

### 자동 검증과 재현 경로

- P79 후보의 publisher validate_document/validate_publishable와 실제 template 투영 통과.
- 기존 selected-group 보존·잘못된 소유자 거절 focused unittest2개 통과.
- 비둘기 실제 Codec/Playback/종료 receiver:22,853 checks, 원래19폭발216입자 및
  검사용1:1 marker4개. 위치 최대오차1.43051e-6m, 종료 위치2.38419e-7m.
- 선두/후속720개의60Hz 구간 속도는7.999~8.001m/s 이내. 모든 non-track 값과19폭발 요소
  동일, builder 재입력은 변경0이다.
- 거미 실제 helper/Codec/save/reload/Playback/최종 sprite quad:2,187 checks,
  원점과 custom pivot에서 각각299개 quad 검사, 최대 행렬오차9.53674e-7.
  단일 요소 외 항목 보존, 잘못된 pivot/ID 거절 시 문서 보존도 통과.
- 변경 Effect_Tool_Helpers.cpp와 Effect_Tool_Detail.cpp의 격리 Debug TU 컴파일 통과.
- probe는 최신 Effect_AuthoringDocument.h ABI 이후의 Product/기존 격리 OBJ와 링크했다.
  Windows/CRT 오류창을 막고30초 제한의 CREATE_NO_WINDOW로 실행했다. Client/UI/GPU는
  실행하지 않았다. DirectXTK PDB 부재의 linker warning은 남는다.
- 로그·백업·최신 필드 병합 도구: `out/KoukuPatternPivots20260918/`.
  `merge_data.py` 기본 실행은 후보만 만든다. 승인 후에만 --apply로 exact-input 비교,
  백업, 기존 commit_library의 원자 교체·자기 변경 rollback을 수행한다.

Product 링크·최종 JSON/XML/diff 확인·publish는 최종 반영 기록에서 따로 판정한다.
실제 Preview Play/Server Play 화면과 거미 장판 시작점의 육안 판정은 사용자 확인 대기다.


### G32 최종 보류 상태 — 사용자 편집 계속 / 직접 빌드

사용자는 “아직 편집중이어서 내가 직접 다시 빌드 돌린 다음에 결과 알려줄게”라고 답했다.
이 답변은 현재 디스크 저장본 교체 승인으로 해석하지 않았다. 이번 작업은 Product 빌드·실행과
외부 데이터 교체·publish를 보류하고 소스 변경과 검증된 후보를 보존했다.

현재 사용자가 빌드하면 적용되는 것은 Pivot/target UI와 captured-root 편집 기능이다.
P79 캡처 Logic 추가, 비둘기4개 Track, 거미5개 emitter-axis 옵션은 아직 live에 없다.
따라서 재빌드만으로 세 데이터 변경까지 확인할 수 있다고 안내하지 않았다. 저장 완료 및
반영 의사를 받은 다음 최신 stable ID/field 기준 병합과 publish를 이어가야 한다.
후보의 마지막 dry-run은 Composition1527→1528이며 source는 이후 더 바뀔 수 있으므로
이 전체 파일을 그대로 덮어쓰지 않는다. merge_data.py가 최신 ordinal/revision을 재할당하고
P79 대상 행의 실제 충돌은 중단한다. 후보는 out/KoukuPatternPivots20260918/latest-merge다.

마지막 Python syntax, 후보3개 JSON parse, 현재 project/filter XML parse 및 git diff --check는
통과했다. Product EXE 링크·게시·사용자 시각 확인은 미실행이다. 커밋·푸시는 하지 않았다.

### G32 실제 반영 — 거미 바닥의 quad·native 좌표 회전 일치 (2026-09-18)

사용자가 새 EXE에서 무늬만 회전한다고 재보고하고 재수정·반영을 요청했다. 현재 실행은
Client PID42028, 12:36:56 시작이며 EXE12:36:55, Tool Detail/Helpers OBJ12:36:45~46이다.
이전 Pivot UI는 이 빌드에 포함되었지만 G32의 데이터 후보는 계속 미설치였음을 확인했다.

실제 원인은 native2349가 SourceEmitterWorld 역행렬로 내부 noise 좌표를 계산하는 반면,
followEmitterAxisRotation이 꺼진 fixed-axis quad는 같은 emitter 방향을 무시하는 것이다.
따라서 내부 이미지만 회전하는 관찰과 일치한다. fx_m_flow_04_n.dds를 사용하는 두 주 요소는
notify008의 emitter19/25이며, 해당5개 floor 요소를 함께 켜야 같은 방향을 유지한다.

최신 저장본에 다음 두 문서의5개씩, 총10개 followEmitterAxisRotation=true만 반영했다.
Transform·timing·재질·sourceRecipe·원본 offsetCenter[.5,1]·body-follow는 그대로다.

- effect.kouku.gate2.4219776.stage2.full.restore: 패턴 presentation56의 실제 자산.
- effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_dash_ground_01_loc_int: Effect Tool 원본 항목.

build_kouku_spider_counter_restore.py도 stage2의 해당 source-system만 같은 옵션을 생성한다.
추가 shader/C++ 변경은 없다. 기존 opt-in 소비 코드가 현재 EXE에 이미 들어 있다.
미반영 상태인 P79/비둘기 후보는 이 두 파일 교체에 섞지 않았다.

후보·이전 파일·설치 hash receipt는 out/SpiderRotationApply20260918에 있다. 최신 bytes를
백업하고 검증된 후보와 교체 직전 일치를 재확인한 뒤 commit_library의 원자 교체와 소유한
변경만 rollback하는 경로를 사용했다. 설치 뒤 두 파일이 검증 후보와 바이트 동일함을 확인했다.

실제 Codec/Playback/최종 sprite quad 검사는 두 문서 각각4,363 checks를 통과했다.
각 문서에서 원점/custom pivot, root yaw0/37도+이동된 root의4조합마다299개 quad를
비교했다. 최대 행렬오차는 stage2 1.43051e-6, leaf1.19209e-6이다. 동일 helper의 선택 요소
외 항목 보존, save/reload, 잘못된 입력 보존도 통과했다. CPU 프로브는 종료0이며
CRT의 FBX/라이브러리 정적 할당 leak dump가 stderr에 남아 있어 leak-free 검증으로 기록하지 않는다.
Python syntax, 변경 JSON parse, 설치 hash, git diff --check도 통과했다.

실행 중 문서 갱신은 별도다. Effect Tool Current Effect → Editing Session → Load Saved가
디스크를 다시 읽는다. Restart Preview만으로는 외부 변경을 읽지 않는다. Pattern Workbench의
Refresh Resources도 목록만 갱신한다. Load Saved 후 실제 Rotation 편집을 Save하면
Reload_SelectedProductEffect가 다음 product spawn을 갱신한다. 또는 사용자가 Client를
재시작하면 catalog를 새로 준비한다. 이 데이터 반영에 재빌드는 필요하지 않다.
미저장 draft를 버리는 Load Saved/종료는 자동 실행하지 않았다. 화면 결과는 사용자 검증 대기다.

### G32 실제 반영 — 비둘기 비행 경로 (2026-09-18)

사용자는 비둘기5.6→2.8m/선회 뒤2.4→5.2m도 미반영이라고 재보고하고 반영을 요청했다.
직전 작업에서는 거미·발탄만 설치했고 이 비둘기 후보는 아직 설치하지 않았음을 재확인했다.
현재 Composition의 kakulsaydon.effect.285a0dae002c0367563c와 P82.presentation.1,
제품 patternbindings, EffectCatalog는 모두 effect.kouku.magic.paper.dove.group의 동일
DIRECT_AUTHORED_DOCUMENT를 참조한다. 별도 복사본이나 Gameplay publish는 필요없다.

최신 Authored 저장본을 읽고 기존4마리의 sourceTransformTrack만 병합했다.
선두 직선0.7초/5.6m→0.35초/2.8m, 반원2초 유지, 이후 직선0.3초/2.4m→0.65초/5.2m다.
8m/s,0.8m 후속 간격,3초 수명과 death event, 기존19폭발, 밝기1·재질·Transform·기타
모든 non-track 값은 유지했다. builder 재입력은 변경0이며 독립 편집 track은 거부한다.

out/DovePathApply20260918에 before/candidate와 설치 SHA receipt를 기록했다.
후보 검사 후 교체 직전 raw bytes가 같음을 재확인하고 기존 commit_library의 원자 교체와
자기 변경 rollback으로 실제 Authored에 설치했다. 다른 Composition/P79 후보는 건드리지 않았다.
설치 뒤 현재 파일을 다시 로드하여 후보와 hash 및 실제 CPU 재생 출력이 동일함을 확인했다.

기존 실제 Codec/Playback/원본 explosion+terminal marker 검사는22,853 checks를 통과했다.
4마리, 원래19 receiver의216폭발 입자, 기존3초 수명 뒤fixed-step의3.03333초 발동을
유지한다.720개60Hz 이동 구간 속도는7.9997715~8.0000000m/s였다. 종료 위치 최대오차
2.38419e-7m, 경로 최대오차1.43051e-6m, 회전/이동 root 합성 최대오차2.38419e-6m다.
JSON parse·설치 hash·git diff --check를 확인했다. CPU만 실행했고 Client/UI/GPU는 실행하지 않았다.

현재 EXE의 기존 Track/Death event 소비 경로로 적용되므로 재빌드는 필요없다.
Effect Tool은 해당 비둘기 Current Effect의 Editing Session → Load Saved → Play All로
새 파일을 읽는다. Pattern Play Preview는 기존 immutable catalog를 잡고 있을 수 있으므로
Client 재시작 또는 명시적인 Tool Save의 runtime activation 뒤 확인해야 한다.
사용자의 미저장 편집을 버리는 Reload/종료는 자동 수행하지 않았다. 화면 판정은 사용자 확인 대기다.
