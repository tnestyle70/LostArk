# Retail 수치와 실행 중 쿨타임 정책 결과

## G01. 구현한 계약

PR #454 병합 `d9361f98f` 이후 room 단위 `COOLDOWN_MODE`를 연결했다. 새 room 기본값은
`DEBUG_THREE_SECONDS`이며 기존 0초 스킬은 0초를 유지한다. `RELEASE_AUTHORED`는 게시된
Retail 쿨타임을 사용한다. 이 이름은 Client/Server의 Debug/Release 빌드 설정과 독립적이다.

`F1 -> Balance Test`의 두 버튼은 `IPlayerCommandSink -> NetworkPlayerCommandSink ->
NetworkManager -> Shared -> ServerApp -> RoomCommand -> GameRoom`을 사용한다. Server는
session/world/sequence를 확인하고 현재 room의 모든 플레이어 정책을 변경한다. 승인된 새 스킬과
pending 스킬 양쪽에 정책을 전달한다. 아직 진행 중인 cooldown은 최초 사용 시점을 유지한 채
길이를 재계산하고 이미 끝난 cooldown과 기믹/차량 전용 타이머는 유지한다. 다른 room이나 전역
GameplayCatalog는 변경하지 않는다.

protocol 109의 player snapshot은 mode와 각 cooldown의 실제 duration ticks를 보낸다.
Client `CombatHUDViewModel`은 Server duration을 HUD 분모로 사용한다. MainApp의 특수/일반 슬롯
분모도 같은 필드를 소비하도록 연결했다. 서버 종료 tick과 전체 duration이 같은
정책을 사용하므로 로컬 기본 3초 catalog가 Retail 300초 원형 표시를 왜곡하지 않는다.

Kill Current Gate Boss는 Release 거부 분기만 제거했다. 현재 room/gate의 primary boss 선택,
stale HUD/sequence 검증과 기존 death/clear/Encore 소비자는 유지한다. 최종 clear flag를 직접
변경하지 않는다. `Apply_SkillBuffs`의 ENEMY 후보에 MONSTER를 포함하여 기존 stun runtime과
MonsterBrain deadline 소비자가 실제 제품 호출에서 연결되었다.

## G02. Retail 수치 편집과 게시

공용 panel은 base JSON row와 Retail override를 합친 수치를 읽고 각 field의 소유 문서를
기억한다. HP/AP/health bars/cooldown/resource/stagger/part 및 critical/coefficient/addend/spread의
Retail field는 Retail profile에, 덮지 않는 이동/충돌/timing 등의 field는 base JSON에 저장한다.
프로필 coefficient/addend가 활성화된 damage에서 소비되지 않는 base rate는 숨긴다.

`Save-BalanceTestDraft.ps1`은 profile domain과 stable ID를 포함한 field patch를 병합한다.
최신 base/profile 전체 input bytes를 확인하고, 같은 field 충돌뿐 아니라 Retail이 새로 소유한
field에 대한 오래된 base draft도 거부한다. 무관 field와 미지원 schema를 보존하며 candidate
Retail validation 뒤 원자 교체/자기 변경 rollback을 사용한다. 저장은 실행 중 Server나 다른
도구 draft를 Reload하지 않는다.

`Publish-BalanceRuntimeSet.ps1` 기본 프로필 및 F1의 명시 인자는 Retail이다. Gameplay와 World의
Validate/Publish 양쪽에 전달한다. 공식 Gameplay/World publisher도 기본 프로필을 Retail로
변경하여 Kouku/Valtan/Build domain의 직접 호출이 전투 수치를 base로 되돌리지 않는다.
명시적인 빈 프로필은 원본 비교용 opt-out으로만 유지한다.
Retail profile을 Client 프로젝트와 filters의 기존 `96.DataFiles/Balance` None 항목에 등록했다.
새 C++ 파일은 추가하지 않았다.

## G03. 실행한 검증과 현재 상태

| 검증 | 상태 |
|---|---|
| PowerShell 저장 transaction | PASS, 10 tests |
| 기존 동일 field 충돌/unknown field 보존/외부 저장/rollback | PASS |
| Retail + base 동시 저장과 profile unknown field 보존 | PASS |
| Retail 동일 field 충돌 시 모든 원본 bytes 보존 | PASS |
| Retail에 가려진 stale base draft 거부 | PASS |
| 검증 중 외부 Retail profile 수정 보존 | PASS |
| Save/통합 publisher PowerShell AST | PASS |
| Client project/filter XML parse | PASS |
| 담당 source diff check | PASS |
| Product Debug / Release 최종 빌드 | 양 구성 PASS |
| Server Debug debug-teleport 전체 그룹 | PASS, 8,109 PASS / `failures : 0`, 담당 cooldown/Kill 15개 assertion 및 복귀 후 trigger 정책 검증 포함 |
| Server Release debug-teleport 전체 그룹 | PASS, 7,576 PASS / `failures : 0` |
| NetworkProtocolHarness Debug build/run | PASS, 1,269 PASS / `failures : 0` |
| NetworkProtocolHarness Release build/run | PASS, 1,269 PASS / `failures : 0` |
| Kouku support-surface / object-overlap 카탈로그 재로드 회귀 | 양 구성 PASS / `failures : 0` |
| Retail Gameplay + World + Items 최종 통합 게시 | PASS |

실행 명령은 `python -B Tools/GameplayPipeline/test_balance_test_transaction.py`다. 임시 저장소에서
실제 transaction과 writer admission/CAS/File.Replace를 실행했고 제품 데이터는 변경하지 않았다.
native 테스트에는 room 기본값, 4인 정책 변경, 3초/300초/0초 계산, expired/gimmick timer 보존,
replay/world/session 거부, wire enum/round-trip/실제 payload size 및 양 구성의 Kill 범위를 추가했다.

최종 Product receipt는 `out/BuildPipeline/runs/20260923T224833539Z-debug-product.json`과
`20260923T224850148Z-release-product.json`이며 둘 다 `result: PASS`다.

native 로그는 `out/ReleaseRaidTools20260924/debug-debug-teleport-verified.log`와
`release-debug-teleport.log`다. room 기본 정책, 실제 Retail catalog의 300초/0초, 4인 정책 변경,
expired/gimmick timer 보존, replay/world/session 거부, Kill의 현재 gate/primary/stale HUD 계약과
G05의 아레나 복귀 검증까지 양 구성 모두 `failures : 0`으로 종료했다. protocol build/run 로그는
같은 경로의 `protocol-build-debug.log`, `protocol-debug.log`, `protocol-build-release.log`,
`protocol-release.log`다.

카탈로그 공통 재로드 회귀도 `debug-kouku-support-surface-verified.log` 307 PASS,
`release-kouku-support-surface.log` 28 PASS, `debug-kouku-object-overlap-final.log` 660 PASS,
`release-kouku-object-overlap.log` 628 PASS로 확인했다. 모든 로그의 최종 실패 수는 0이다.
최종 Retail 게시 로그는 `publish-retail-runtime-set.log`이며 gameplay + 4 worlds + items의
통합 게시 성공과 revision `d21992b07d3b9118cf9b1ba88c858538eaec54b1f63c6762338c184a91299d6b`를
확인했다. 이 통합 빌드·실행·게시는 root가 수행했고, 이 문서 갱신에서는 소스를 변경하지 않았다.

## G04. 남은 경계

- 4인 F1 모드 변경, 쿨타임 원형/초, 수치 Save/Publish 후 재시작 및 Kill 이후 레이드 연출 화면은
  사용자가 직접 확인한다. Client/UI는 실행하지 않았다.
- Retail의 boss별 `staggerGaugeMaximum`은 여전히 미소비 field다. 저작
  `SET_STAGGER_GAUGE`와 #454의 전역 scale 경로를 유지하며 미소비 field는 공용 editor에서 숨긴다.
- `attackSpeedPercent`는 행동시간/animation에 아직 소비되지 않는다. 이번 변경이 그 효과까지
  완성했다고 기록하지 않는다.
- 일반 damage tooltip의 로컬 AP/rate 계산은 기존 경로다. 이번 HUD 수정은 실제 cooldown
  duration/end tick 일치에 한정한다.

## G05. Release 아레나 복귀 후속 진단

Release `debug-teleport`의 네 실패는 fixture가 `CServerTriggerSystem::Debug_Activate`를
직접 호출한 결과였다. 이 일반 WORLD 저작 API는 Release에서 DISABLED를 반환하는 기존
정책이며 제품 오버랩은 `Evaluate_Entries`로 진입한다. 테스트를 실제 게시 trigger의 OBB
중심과 Product overlap callback으로 교체했다. once latch 소비, reset 후 재활성화, duplicate
요청 보존은 계속 검증하고 generic WORLD 명령의 Release 권한은 변경하지 않았다.

이 조사에서 `Apply_DebugReturnToKoukuStart`가 새 기본 trigger system으로 교체하면서 room의
world ID, ground sampler, fire log와 Debug wave suppression 설정을 잃는 제품 결함도 확인했다.
기존 system을 candidate로 복사하고 player contact/debounce만 비운 뒤 Initialize하여 설정을
보존한다. 검증 실패 전에는 기존 system을 바꾸지 않으며 성공한 이동/보스 정리 뒤에만 교체한다.
게시된 Book1 trigger를 사용한 Release 자동 entry와 Debug suppression 보존도 실제 native
실행에서 PASS했다. `release-debug-teleport.log`의 `Arena reset preserves Release Kouku Book
auto-entry world rule`, `debug-debug-teleport-verified.log`의 `Arena reset preserves Debug F1
wave suppression`으로 확인한다. 양 구성에서 fixture를 다시 초기화하기 전 실제 복귀 함수가
소비된 once sequence를 Product overlap으로 재활성화하는 검사도 PASS했다.

변경 파일은 `GameRoom_KoukuPlayerCommands.cpp`, `ServerGameplayContractTests_DebugTeleport.cpp`다.
source diff 검사와 이 후속 수정을 포함한 최종 Product 빌드 및 native 재실행은 양 구성 모두
PASS다. 최종 실패 수와 실행 증거는 G03에 기록했다.
