# 발탄 발악 Preview·Object 편집 통합 결과

## G00. 반영 범위

작업 브랜치는 `codex/valtan-authoring-and-entry-20260930`이며 기준 HEAD는 `4fce79521`이다.
사용자가 EXE 종료 후 전체 반영을 요청했다. 기존 STEP_11 위치 튜닝과 Atk08_04 emitter13 삭제,
쿠크 렌더링 옵션, 관계없는 backup/retired 파일은 보존했다. Client/UI는 실행하지 않았다.

## G01. Preview와 Object 편집

발악 Preview는 ARENA_CENTER rock volley를 사용하지만 LEAP serverMotion이 없어 중심을 stage하지
못했다. `CValtan::Stage_LocalPatternAuthoringPreview`가 실제 effect/volley/aim의 중심 요구를 확인해
동일 `boss.valtan.center`를 읽고 transaction 검증 전에 등록한다. 잘못된 anchor는 계속 거부한다.

Composition Actions의 Object에서 Boss=Valtan을 선택하면 기존 Valtan workbench를 사용한다.
MainApp의 Resources/transport/AnimationTool 입력 소유권도 같은 세션으로 연결하고, Kouku
WorldObject 세션이 입력을 가져가지 않도록 했다. 실제 Pattern/Stage/object stable ID로 선택한다.

돌의 피해·넉백은 기존 Server combat-object hit에 이미 있었다. Summon Detail의 조기 반환으로
가려졌던 CIRCLE/RING collider를 같은 화면에 노출했다. 폭발 시간, 반경, owner-hit 연쇄 지연,
넉백 거리/시간, 다운 여부/시간은 `SET_COMBAT_OBJECT_IMPACT`로 원본에 저장한다.
동일 archetype을 여러 occurrence에서 사용해도 hit별 patch는 한 번만 생성한다.
Preview wire는 실제 돌 위치·lifetime·폭발 시계를 사용하며 gameplay 피해를 만들지 않는다.

`itr-02326.off.full`의 주요 폭발 element는 원본 약1.82초에 시작한다. 사용자 확인에 따라
이펙트를 폭발 프레임부터 seek하는 초기 후보는 폐기했다. 준비 동작 전체를 재생하고 실제 파편
시점에 피해·넉백·Sound가 나오게 한다. owner-hit chain의 direct/외부 돌 준비 시작 차이0/1500ms는
유지하며 각 준비 시작에1820ms를 더해 hit한다. fixed 발악 돌은 준비4133ms, hit5953ms,
lifetime7153ms다. terminal Effect는 한 번만 시작하며 hit에서 재시작하지 않는다.
고정 hit 시계 편집은 준비 event도 같은 delta로 이동하여 원본 준비 길이를 보존한다.

Object Sound의 event와 원본 playbackOffsetMs도 같은 Detail에서 편집한다. 미저장 Sound는 기존
AnimationTool transport에 실제 object hit clock으로 합류하므로 pause/seek/속도/stop을 공유한다.
Source Save는 Combat Object Sound baseline/candidate를 기존 canonical CAS·rollback transaction으로
전달하며 별도 저장 선로를 만들지 않는다. runtime은 기존 hit/presentation event 경계를 유지한다.

## G02. 함께 반영한 기능

- 첫 성공 파괴 폭탄의 즉시 전체 갑옷 파괴와 집결·10초·전원 입장 투표는
  `2026-09-30_VALTAN_ASSEMBLY_AND_FIRST_ARMOR_BREAK_RESULT.md`에 기록한다.
- 피자 검격 중심 pivot, 뒤잡기 yaw90, 원작 상공 블랙홀 복원은
  `2026-09-30_VALTAN_EFFECT_CENTER_AND_SKY_RESULT.md`에 기록한다.
- 실리안/바훈투르 초상화와 전용 무력화·부위파괴 PNG 및 Sound CPU 검증은
  `2026-09-30_VALTAN_STAGGER_ARMOR_UI_RESULT.md`에 기록한다.
- 발악 STEP_12의 종료는 별도 23초 사망 패턴을 건너뛰고 망령 부활 패턴으로 직접 연결했다.
  원작 clip 연결과 현재 gameplay branch를 대조했으며 별도 사망 패턴 자체는 보존했다.
- 쿠크 피자 마지막 노란 원 공격은 기존 피해·넉백을 MAP 고정 collider로 연결했다.
  두번내려치기의 최대 HP 10% 피해와 10m/1500ms 넉백을 유지하며 노란 예고 종료를
  16.997초 타격 시작에 맞췄다. `2026-09-30_KOUKU_PIZZA_FINAL_CONTACT_RESULT.md`에 기록한다.

## G03. 실행한 검증

- `test_valtan_object_impact_authoring.py`: 실제 source patch/project, owner-hit delay,
  범위·identity·NaN·clock 실패 보존 3 tests PASS.
- `test_valtan_combat_object_sound_save.py`: async PowerShell 인자 전달, source CAS 거절,
  injected rollback, 정확한 단일 owner 변경, 잘못된 identity/clock 거절 2 tests PASS.
- 실제 함수 CPU probe: HUD81 + Sound document16 + Sound transport20 + preparation clock19,
  총136 checks PASS.
- World publisher Publish VALTAN_ARENA:158 placements,3 spawn groups,6 encounter props 출력 반영.
- PublishV2, Kouku projector(revision2497), 최종 Gameplay Publish PASS.
  `publish-gameplay-final.log`의 source manifest는
  `2073a8072bc3c77a3821dd9af63f8b8cb2d0fe9f4b3e0fbe6576038897c92c66`이며
  44 managed patterns/17 combat objects를 검증했다. bootstrap109863행/32324771bytes.
- Debug/Release Product Build·배포 PASS. Engine/Shared/Server/Client 정상 Build이며 Client를 실행하지 않았다.
  증거는 `out/BuildPipeline/runs/20260930T071816058Z-debug-product.json`과
  `20260930T072322910Z-release-product.json`이다.
- 최종 변경·신규 JSON/XML23개 parse PASS, diff whitespace 검증 PASS.
- 첫 native battle-items 실행에서 첫 폭탄·집결·시간 제한·이탈 취소는 PASS했지만 최종4인 이동
  테스트1개가 실패했다. 다른 두 도착 슬롯의 전방 벽 충돌을 수정한 후 Debug/Release 모두
  failures0 PASS. 사용자 지정 두 좌표와 pair 간격을 보존했다.
- Debug/Release `--valtan-presentation-contract-test` 모두 failures0 PASS. 실제 4종 바위의
  준비 중 HP 보존과 1820ms 뒤 최초 30Hz tick의 단일 피해·sound-driving hit pulse를 확인했다.
  로그는 `out/ValtanAuthoring20260930/valtan-presentation-final.log`,
  `valtan-presentation-release.log`, `battle-items-gameplay-corrected.log`,
  `battle-items-release-final.log`이다.
- lifecycle 검증은 폐기된 G 입장 fixture를 새 집결·전원 투표·4개 슬롯으로 수정했다.
  제품 코드는 추가 변경하지 않았다. 사용자가 Debug를 실행한 뒤 전체 runner의 재빌드는
  output guard에서 중단되었으며 실행 중인 Client/Server는 종료하지 않았다.
  점유되지 않은 Release Server에서 fixture 한 CPP를 증분 컴파일하고
  `--valtan-lifecycle-contract-test`를 실행해 failures0 PASS를 확인했다.
  4인 연속 레이드의 7개 체력 기믹, 발악→망령 부활 직결, 최종 처치·4인 보상까지 통과했다.
  로그는 `out/ValtanAuthoring20260930/build-release-lifecycle.log`와
  `valtan-lifecycle-release-final.log`다. Debug 제품 빌드 뒤 추가된 변경은 이 테스트 fixture뿐이며
  사용 중인 Debug EXE는 다시 링크하지 않았다.

## G04. 사용자 확인 경계

발탄 arena에서 Composition Actions → Object → Boss=Valtan으로 돌을 선택하면 Explosion / Damage
Collider와 Sound를 편집한다. Play Preview로 실제 표시·소리를 확인하고 Save/Publish한다.
F1 → Health bar positions에서 Valtan Stagger와 Valtan Armor Break PNG를 각각 Show debug로 표시해
위치·크기를 튜닝한다. 패턴 재생은 필요하지 않지만 살아 있는 발탄의 HUD 대상이 화면에 존재해야 한다.
GPU 화면, 소리 체감 타이밍, 실제 다인 네트워크 UI는 사용자 확인 대상이다.

## G05. 저장 확인과 자동 입장 후속 요청

사용자 Save 결과는 2026-09-30 16:38:21 KST의 Data/UI/KoukuSaydon/KoukuHudModes.json이다.
Valtan Stagger는 X0/Y98/width0.3333333432674408/thickness1, Armor Break PNG는
X-6/Y-85/width1/height1이다. 두 직전 백업과 비교해 위치 변경만 확인했고 JSON/schema/범위와
MainApp 시작 Reload 및 두 HUD 소비 연결을 확인했다. 현재 hash는
EA5721200A0B4EBC4BA9FF094594955ED364675502F6DC824CD31BA16B8AE65F이며 이 파일은 수정하지 않았다.
첫 파괴 폭탄의 즉시 갑옷 파괴는 사용자가 실제 화면에서 확인했다.

후속 변경은 기존 집결·투표 완료를 Server 300tick 뒤 자동 입장으로 바꾼다. 발탄의 전원 집결과
쿠크의 공대장 오라 조건을 유지하며 입장 확인·수락 popup을 제거한다. 실패 시 같은 점유에서
재시도하지 않고 재집결을 기다린다. 다른 관문·재시작·퇴장 및 Bern 파티 투표는 유지한다.
두 Client는 CRaidGateProgressView::Render_AssemblyCountdown으로 쿠크의 기존 흰 안내
‘잠시 후 다음 지점으로 이동됩니다.’(화면72%, 배율0.625)와 노란 N초(75.5%,0.7)를 공유한다.
Client는 입장 명령을 보내지 않고 Server 자동 완료를 표시한다. 데이터 publisher 변경은 없다.

Client Release 증분 Build와 Debug ClCompile은 PASS했다. 사용 중인 Debug Client는 링크하지
않았고 Server 수정 및 회귀 검증은 후속 결과를 따른다. 로그는
out/ValtanAuthoring20260930/build-release-autoentry-client.log와
compile-debug-autoentry-client.log다. Client와 Server는 에이전트가 종료하거나 UI 조작하지 않았다.


## G06. 자동 입장·공중 속박 최종 native 검증

Release의 --battle-items-contract-test, --kouku-raid-contract-test,
--valtan-lifecycle-contract-test, --valtan-presentation-contract-test 모두 failures0/exit0이다.
첫 두 검증은 실제4인 목적지·입장 cinematic 및 쿠크1~4인 자동입장,299/300tick 경계,
실패 보존·재시도 latch와 일반 투표 보존을 포함한다. lifecycle는 자동 집결 입장 뒤 연속 raid의
발악→부활·최종 처치·4인 보상까지 통과했다. 대응 로그는 out/ValtanAuthoring20260930의
battle-items-autoentry-release.log, kouku-autoentry-release.log,
valtan-autoentry-lifecycle-release.log, BindAirborne/test-after.log다.

속박 실패는 실제 Debug/Release session 로그에서 같은 restore-pose guard로 확인됐다.
원본 FOUR_SLASH contact6개의 실제 Arm/Advance를 사용한 A/B native 재현은 구코드 exit1,
수정본 exit0이다. 공중 현재Y와 복귀 지면Y를 분리하여 기존1.5m 지면 검사는 유지하고,
속박 진입·flight 종료·EXIT 지면 복원·후속 반응을 확인했다. 실플레이에는 XYZ 로그가 없어
재현 좌표를 그 프레임의 실측으로 주장하지 않는다. 자세한 근거는
2026-09-30_VALTAN_COUNTER_LOOP_AND_STRUGGLING_RESULT.md G14에 기록했다.

Server Release Build와 Debug ClCompile도 PASS했다. build-after.log와
compile-debug-autoentry-bind-server.log가 최초 근거다. 이후 EXE 점유가 해제된 것을 확인하고
Guardian 병합 및 속박 상태 문구까지 포함하여 Debug/Release Product 컴파일·링크를 완료했다.
최종 receipt는 out/BuildPipeline/runs/20260930T091228557Z-debug-product.json과
20260930T091359049Z-release-product.json이며 둘 다 PASS다.
저장 HUD hash는 검증 후에도 EA5721200A0B4EBC4BA9FF094594955ED364675502F6DC824CD31BA16B8AE65F로
유지됐다. 사용자 확인: 첫 폭탄 갑옷 파괴 및 쿠크 피자 collider 정상 표시.

## G07. 마지막 착지·실제 무력화바 수정과 최종 제품 빌드

마력구의 저작 channel은 독립 stagger current/maximum을 Server가 보내지만 Client가
legacy response threshold0으로 덮어써 실제 bar를 숨겼다. 저작 channel은 독립 stagger를,
legacy window만 유효한 response 수치를 사용하도록 수정했다. 실제 Apply_Boss native
fixture는 수정 전9 PASS/7 FAIL에서 수정 후16 PASS/0 FAIL/exit0이며 사용자 HUD 저장값은
변경하지 않았다. 상세 근거는 STAGGER_ARMOR_UI RESULT G07에 있다.

버러지들 일부 포획의 생존 해제는 원본 내려찍기 source1500ms의 실제 왼손 위치를 기준으로
Server가 같은 층의 이동 가능하고 충돌 없는 지면에 확정한다. 원본 root rotation180도를
무시한 포획 offset 복귀를 제거했다. 실제8방향 Room A/B는 수정 전 착지8 FAIL에서 수정 후
16 PASS로 바뀌었고, 추가 착지·실제 이동·원자성63개 검사도 통과했다. 전원 처형과 일반
Release_PlayerAttachment는 보존했다. COUNTER_LOOP_AND_STRUGGLING RESULT G17/G18을 따른다.

가디언 나이트49150 clip1은 최초 병합 이후 사용자가18:43:52에 다시 저장한
58e0a03c977ed2230875c96b8ef8ae4d2d175178638be53eb7cd0c6037084387을 보존했다.
최신 native Codec 및 resource boundary 검사도 통과했다. 예전 후보를 재적용하지 않았고,
최신 위치·회전·복제·삭제를 그대로 최종 포장 입력으로 삼는다. 구체적인 차이는
out/FinalRaidGuardian20260930/guardian-latest-edit-validation.json에 기록했다.

최종 Debug/Release Product receipt는 out/BuildPipeline/runs의
20260930T100413252Z-debug-product.json과20260930T100434924Z-release-product.json이다.
둘 다 전체 Product 경로를 SkipBuild=false로 실행해 PASS했다. 기존 인코딩·DirectXTK PDB
경고가 남아 있으므로 warning0으로 표현하지 않는다. 최종 Release lifecycle는152 PASS/0 FAIL,
직전 동일 제품 코드의 presentation43/battle-items183/Kouku2023도 모두 통과했다.
중간 fixture 전제 오류의 실패 로그와 수정 후 결과는 별도로 보존했다.

Server10/Client5 domain owner publish는 앞서 모두 PASS했고 현재 source manifest는
2073a8072bc3c77a3821dd9af63f8b8cb2d0fe9f4b3e0fbe6576038897c92c66으로 같다.
마지막 두 버그는 C++ 변경이며 Guardian DIRECT_AUTHORED_DOCUMENT는 최신 Data를 직접
소비하므로 이 후속 변경을 위한 domain 재게시 항목은 없다. NumericSourceBindings는 최신
원본으로587필드를 재생성했고 공식 package-tools20개 검사와 payload plan이 통과했다.
공식 portable 포장 결과와 최신 Guardian 원본/ZIP 해시 대조는 최종 delivery-evidence.json에 기록한다.

## G08. 테스트 배포 ZIP 완료

공식 build_portable.py로 C:/Users/user/Desktop/LostArk-Verification-20260930.zip을 생성했다.
크기는168004971bytes이며 SHA256은
4cd3a680c5b3a8c2cad641853ee42a4344d173103ab0bf14f599e4724c02b8cd다.
같은 이름의 이전 ZIP은 backup-20260930-190753-915148.zip으로 보존했다.

Release Client/Server, app-local VC 런타임, compiled shader256개, canonical Data2149개와
DataFiles 및 실행기를 포함한다. Resources는 기존 배포 계약대로 포함하지 않는다.
원작 상공 효과의24개 리소스는 별도 GBResources에 모두 있고 원본과 해시가 일치하며,
누락19개만 추가했다. 외부 Drive 업로드 완료로 기록하지 않는다.

포장 도구가 source CAS, 전체 ZIP CRC, 모든 manifest entry SHA256, 중복·금지 항목 및
LostArk.exe --check 사전 검사를 통과했다. Client/Server 제품 실행은 하지 않았다.
추가 저장된 최신 용 효과를 포함해 Guardian13파일의 현재 원본과 ZIP SHA가 모두 일치한다.
Debug·Release receipt 및 실제 Release 바이너리 pin도 ZIP과 일치한다.
최종 변경 JSON33/XML2 parse와 git diff --check는 PASS이며 HUD 저장 해시도 유지됐다.

증거는 out/ReleasePackaging/portable-delivery.receipt.json,
preflight-20260930-valtan-guardian-final.json,
out/FinalRaidGuardian20260930/delivery-evidence.json 및 delivery-source-validation.json이다.
실제 화면·음향·다인 LAN 플레이는 사용자가 이 ZIP과 최신 외부 Resources로 확인한다.
