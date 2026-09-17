# 세이튼 카드·트럼펫·공과 돌진 잔상 구현 결과

## 현재 상태 — 최종 저장본 등록 및 Publish 완료

사용자가 모든 편집을 저장하고 Client를 종료했다고 확인한 뒤, 최신 Composition1223을 기준으로
19개 파일을 병합·설치했다. 현재 Composition1224, WorldSequence2035이며 정식 KoukuSaydon
owner Publish의 네 domain이 모두 통과했다. G07~G16의 등록 대기는 당시 기록이고, 최종 등록 증거와
남은 범위는 G17을 따른다. 최신 Client·Server 제품 빌드와 사용자 화면 확인은 수행하지 않았다.

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
