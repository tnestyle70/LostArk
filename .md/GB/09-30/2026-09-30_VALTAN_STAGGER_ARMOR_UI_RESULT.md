# 발탄 무력화·부위 파괴 HUD와 에스더 초상화 결과

## G00. 실제 반영

MainApp의 발탄 에스더 초상화 override를 Ctrl+Z 실리안, Ctrl+X 웨이, Ctrl+C 바훈투르로
교정했다. 기존 Server roster, PlayerController의 Z/X/C slot 순서, EstherUI.json과 일치한다.

F1 `Health bar positions`에서 쿠크 무력화와 별도로 `Valtan Stagger`의 머리 X/Y·너비·두께,
`Valtan Armor Break PNG`의 머리 X/Y·너비·높이를 조절한다. 두 그룹의 `Show debug`는
각각 독립이고 저장되지 않는다. debug는 실제 살아 있는 발탄의 표시용 머리를 사용하며
Server combat state를 변경하지 않는다. 기존 화면 숨김·투영 실패·사망 경계를 유지한다.

발탄과 망령 발탄은 전용 값을 소비하고 PNG는 최초 layout rect를 보관해 매 프레임 크기
배율이 누적되지 않는다. Save/Reload는 기존 최신 필드 병합·필드별 CAS·writer lock·
백업·원자 교체와 충돌 복구를 확장했다. 잘못된 신규 필드도 전체 로드를 실패시켜 draft를 보존한다.

## G01. 저장본 반영

`Data/UI/KoukuSaydon/KoukuHudModes.json`의 `healthBarPositions`에 8개 optional 필드만
추가했다. 최초 발탄 무력화는 현재 쿠크와 동일한 X0/Y196/너비0.3333333432674408/두께1,
PNG는 기존 X-24/Y-40/너비1/높이1이다. 다른 JSON 필드·기존 값과 원문 형식을 보존했다.
공유 madness-position lock 안에서 최신 bytes를 읽고 후보 parse·교체 직전 freshness·
ReplaceFileW 백업 내용·교체 후 bytes를 확인했다. 추가 필드를 제거한 parsed 결과는 이전과 같다.

근거는 `out/ValtanHud20260930/hud-install-receipt.json`과 같은 폴더의 원문 백업이다.
이 UI 설정은 ProjectDataRoot의 기존 직접 읽기 경로를 사용하며 별도 publisher가 없다.

## G02. 실행한 검증

- 실제 MainApp의 8개 함수, WorldHealthBarView의 생성·setter·Update, DataJson, 실제
  DirectXMath 투영과 Win32 파일 교체를 사용하는 CPU probe: `/W4 /WX` 컴파일 성공,
  81 checks, failures0. `out/ValtanHud20260930/hud-probe.log`, `source-manifest.json`.
- 기존 머리 추종/대상 교체/숨김/legacy 보존에 더해 발탄·망령 독립값, 쿠크 수정 격리,
  Show debug의 기믹 없는 표시·Server 상태 보존·UI/사망/다른 Level 격리, PNG 실제 소비와
  비누적 크기, 잘못된 값 거절, 신규 8필드 저장/재로드·서로 다른 필드 병합·같은 필드 충돌을 확인했다.
- 4개 기존 C++ 파일 UTF-8 BOM 없음·CRLF 보존, 등록된 project/filter 항목 확인,
  변경 파일 `git diff --check` 통과. 신규 C++ 파일은 없다.

probe의 actor/head/layout/HUD/scene은 통제된 협력 객체다. 실제 애니메이션 본·GPU·
네트워크·Client 화면 성공을 대신하지 않는다. Product Debug/Release Build는 root가
전체 요청과 함께 조율한다. Client/UI는 실행하지 않았고 최종 위치·크기는 사용자가 튜닝한다.

## G03. public 계약 통합 위치

TEAM_GAMEPLAY_INTERFACE_HANDBOOK의 Health bar positions 절에서 발탄 무력화가 공통
X/Y를 소비한다는 설명을 전용 필드로 바꾼다. 신규 key는
`valtanStaggerHeadOffsetX/Y`, `valtanStaggerWidthScale/HeightScale`,
`valtanArmorBreakOffsetX/Y`, `valtanArmorBreakWidthScale/HeightScale`이다.
각 X/Y와 각 배율은 독립 key이며 값 경계는 기존 offset ±1280/scale0.1..3과 같다.

## G04. World Object 폭발 Sound 편집·저장

`ValtanCombatObjectSoundCueDocument`는 원문 baseline과 draft generation을 보유하고 기존
bindingId의 soundEvent/playbackOffsetMs만 stage한다. 존재하지 않는 binding·catalog event,
잘못된 stable ID·0..600000 밖 offset은 기존 draft를 보존한다. 편집은 디스크에 쓰지 않는다.
Workbench Object hit Detail의 기존 Valtan event dropdown과 source offset을 이 owner에 연결했고,
Save는 BalanceTool의 async/sync canonical owner transaction에 baseline/candidate를 함께 보낸다.
정확한 generation과 저장 bytes를 확인한 receipt만 local dirty를 해제한다. 미저장 reload를
막고 명시 discard 시 검증한 source를 다시 읽는다. Server Play는 미저장 Object Sound도 차단한다.
Python/PowerShell owner pair 게시와 UI 호출은 root가 통합했다. retired 직접 writer는 유지했다.

실제 document의 Parse/Load/Update/Prepare/Accept 함수 CPU probe는 16 checks, failures0이다.
`out/ValtanHud20260930/object-sound-probe.log`에 baseline 보존·잘못된 값·catalog asset 교체·
무파일 편집·candidate round-trip·미저장/다른 generation 거절·정확한 receipt accept를 기록했다.

## G05. 미저장 Sound의 Play Preview

Workbench는 Play Preview마다 Object Sound의 값 snapshot을 Animation Tool에 전달한다.
CValtan의 기존 local object instance가 산출한 owner-hit-chain 폭발 시각과 TIMED 반복 hit,
presentation event 시각을 read-only로 수집해 기존 managed Sound preview transport에 합류한다.
발생 시각은 collider/Effect와 같은 stage+spawn+hit/chain clock이며 source offset은 WAV 앞부분을
생략한다. 별도 지연 타이머가 아니다. pause·seek·역방향 재구성·재생속도·stop은 기존 Sound
channel 관리가 수행한다. Play staging 실패 시 이전 Object Sound snapshot을 복원한다.

실제 collector, Append, Sample 함수 CPU probe는 준비 event 분리까지 포함해 20 checks, failures0이다.
`out/ValtanHud20260930/preview-sound-probe.log`에 stable 반복 event와 별도 occurrence,
authoritative/non-preview 차단, 실제 chain 시각, 미저장 asset/offset, 조기 재생 방지,
정지 clock 중복 방지, pause·seek·속도·rewind stop·재생·saved snapshot 복귀를 기록했다.
이 probe는 audio API 호출을 기록하는 협력 객체를 사용하므로 실제 스피커 소리 판정은 아니다.
기존 HUD·Sound 3개 probe 합계 117 checks, failures0이며 Product 빌드 결과는 root 통합 RESULT에 기록한다.

## G06. 원본 바위 준비 연출 보존과 폭발 시각 정렬

사용자 의도에 따라 off.full의 준비 구간을 건너뛰는 playback offset은 제거했다(root).
`Data/Actors/BossCatalog.json`의 six-pizza·terrain3·terrain9·struggling 바위 네 visual은
기존 armedEffectAssetId/stopActiveOnArmed/armedEffectOwnsTerminal로 전체 원본 off를
한 번만 재생한다. 원본 Effect 문서는 수정하지 않았다.

`Data/Valtan/Valtan.combatobjects.json`의 세 owner-hit-chain은 cone 분류 delay1500ms를
유지하고 hit.trigger.atMs1820ms를 원본 preparation 길이로 사용한다. Server의 기존
CombatObjectRuntime은 분류0/1500ms 시각에 각각 armed marker를 발행하고 그 뒤
1820ms에 hit·피해를 발행한다. 해당 final hit의 기존 Sound binding이 동시에 재생된다.
발악 fixed 바위는 explicit `event.valtan.struggling.rock-pillar.prepare`를4133ms에
발행하고 hit5953ms, lifetime7153ms로 옮겨1200ms의 기존 tail을 보존했다.
GameplayCatalog는 chain delay+hit.atMs가 수명 안인지 검증한다.

local Sync도 같은 준비/폭발 시각을 사용하며 fixed preparation event의 armed visual이
terminal까지 소유하므로 final hit 때 다시 시작하지 않는다. Collector는 preparation
시각과 final hit 시각을 분리해 내보내고 Sound owner는 연결된 final hit만 재생한다.
관련 source와 schema admission·typed 편집 저장은 root, 실제 Server native direct/delayed
regression 추가는 gameplay agent가 통합했다. Shared cone 분류 계약은 변경하지 않았다.

- exact CValtan Stop/Update_LocalPreviewCombatObjectHitChains/Sync 함수를 추출한 CPU
  probe19 checks, failures0. 실제 Shared cone geometry를 링크했고 Effect runtime은
  spawn/seek/stop 호출을 기록하는 협력 객체다. direct prep1000→burst2820, delayed
  prep2500→burst4320, fixed spawn600+prep4133→hit5953, source0 시작·중복 방지·
  rewind idle 복원·모든 handle 해제를 확인했다. 실제 GPU 재생 증거는 아니다.
- 갱신한 Sound probe20 checks, failures0. preparation event에 Sound를 묶지 않으면
  조기 재생하지 않고 final hit의 실제 clock에서만 재생하는 계약을 확인했다.
- 둘 모두 MSVC `/W4 /WX`, 별도 out 경로 컴파일·실행. 로그는
  `out/ValtanHud20260930/preparation-clock-probe.log`, `preview-sound-probe.log`다.
  변경 source hash는 `preparation-source-manifest.json`에 기록했다.
- 두 canonical 데이터는 shared writer admission 아래 최신 bytes/CAS·ReplaceFileW
  backup으로 교체했고 무관한 row·field 보존을 검증했다. 설치 receipt는
  `out/ValtanHud20260930/preparation-clock-install-receipt.json`이다.
- Source JSON validation, 변경 source/data `git diff --check` 통과. 전체 HUD·Sound·
  준비 시각 CPU probe는136 checks, failures0. Product 게시·Debug/Release 빌드 및
  native Server 실제 실행 결과는 root 통합 RESULT에 별도로 기록한다.

## G07. 실제 마력구에서 숨겨지던 무력화바 교정

Show debug에서는 위치가 보이지만 실제 패턴에서 바가 나오지 않는 원인을
CombatHUDViewModel::Apply_Boss에서 확인했다. authored VALTAN_STAGGER_SLOT/channel은
G13 이후 SET_STAGGER_GAUGE로 iCurrentStagger/iMaximumStagger를 사용하고 response는0이다.
Server GameRoom_Replication은 이 stagger 값을 이미 snapshot으로 보내지만, Client의
옛 response projection이 authored channel의 최대치를0/NONE으로 덮어써 표시를 막았다.

Client/Private/CombatHUDViewModel.cpp만 고쳤다. authored CHANNEL은 Server stagger
current/max를 사용하고, legacy VALTAN_MAGIC_ORB_STAGGER_76/window만 response threshold가
있을 때 기존 progress를 소비한다. legacy response가 없으면 기존 stagger fallback을
유지한다. 정확한 pattern/action/archetype 조건과 clamp,0최대치 숨김, 다른 기믹의 직접
mechanic gauge, Show debug와 저장 위치·크기를 유지했다. Server·Shared·MainApp·Data 변경은 없다.

같은 실제 Apply_Boss 함수와 WORLD_ENTITY_SNAPSHOT/HUD_BOSS_STATE를 사용한 native
검사에서 구함수는16개 중9 PASS/7 FAIL, exit1로 authored CHANNEL 숨김을 재현했다.
수정 함수는16 PASS/실패0/exit0이다. 최대50000/현재0, 진행12500, 완료/상한초과,
0최대치, 임의 최대42000, 옛 response 무시, 망령, FINAL_ATTACK·Groggy·idle 숨김,
legacy response/fallback와 다른 보스·다른 패턴의 직접 mechanic snapshot 보존을 확인했다.
근거는 out/FinalRaidGuardian20260930의 stagger-hud-before.log, stagger-hud-after.log,
build-stagger-hud-before.log, build-stagger-hud-after.log와 두 source hash JSON이다.
profile container는 통제된 fixture이며 GPU 화면 판정이나 실제 새 Server 실행을 대신하지 않는다.

기존 UTF-8 BOM 없음·CRLF 유지, 해당 diff --check PASS. 저장 HUD SHA256은
EA5721200A0B4EBC4BA9FF094594955ED364675502F6DC824CD31BA16B8AE65F,
공중→속박 Server 수정본 SHA256은 BF50B5D925C68018B4849C4F77616035EAFD5883C7A9F9DA720CA281188362A6로
유지됐다. 제품 소스 완료·동결을 통합 담당에게 전달했으며 최신 Product 빌드와 ZIP은
통합 결과에서 별도로 확인한다. Client/UI 실행은 없었다.
