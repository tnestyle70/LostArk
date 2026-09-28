# Release 레이드 통합·실시간 수치 저장·실행 ZIP 결과

## 구현 완료

PR #475 상점/재화, #476 마하라카, #477 창술사/가디언나이트, #478 캐릭터 선택과
#479 발탄 저장본을 `codex/release-integration-20260929`에 ancestry를 보존하여 통합했다.
독립 packet 변경을 protocol 120으로 합치고 Artist effect catalog/carrier와 Bern 진입 충돌을 조정했다.

Release F1 Balance Test는 Server 활성 수치 목록을 읽고 typed Save + Apply를 제출한다.
PLAYER/SKILL/DAMAGE/BOSS/MADNESS/STAGGER/PATTERN_DAMAGE 7종을 지원한다.
일반 스킬과 ALT_V를 분리하며 계수/고정 가산 피해, ALT_V 총 boss 체력줄 피해,
쿠크 최대 HP 비율·고정 피해, 실제 패턴 무력화 문턱을 수정한다. native Server가 기존
catalog validator로 검증하고 canonical source, Retail/provenance, bootstrap과 receipt를
원자 저장한 뒤 모든 shared/private room에 동일 numeric revision을 적용한다.
신규 입장은 저장 경계 뒤 처리하며 경쟁 저장은 BUSY, 오래된 draft는 STALE_REVISION으로 거부한다.
저장 실패는 기존 파일과 실행 메모리를 보존하고 Client draft를 덮어쓰지 않는다.

기존 gameplay/presentation revision과 실행 중 패턴 순서는 유지한다. 최대 HP/자원은
현재 비율과 사망 상태를 보존하며 이후 판정은 새 수치를 사용한다. 사용자는
**플레이 → F1 Save + Apply 성공 → 게임 안에서 관문/레이드 재시작 → 새 수치로 플레이**한다.
저장 자체가 레이드를 재시작하지 않으며 Server/Client EXE 재시작은 필요 없다.
나중에 EXE를 다시 실행해도 receipt 검증으로 저장된 수치를 복원한다.

쿠크 P17의 현재 콜라이더와 연결되지 않는 이전 피해 logic 6/7/8을 비활성화하고,
P109 빙고 망치의 기존 collider 9에 누락된 logic 1 연결을 복원했다. 원래 10%와
콜라이더 크기를 보존하고 타이밍을 collider 구간 1122/112 ms에 맞췄다.
정식 projection/publisher로 GATE1/GATE2/GATE3/BINGO 네 관문을 게시했다.
발탄의 비어 있는 보조 loop 공격들은 의도한 비피해 항목이라 임의 피해를 추가하지 않았다.
렌더링 옵션과 Mario FXAA OFF는 변경하지 않았다.

## 실제 실행 검증

- Release Engine/Shared/Server/Client 정상 Build 완료. 최종 Product Build PASS:
  `out/BuildPipeline/runs/20260928T201651456Z-release-product.json`.
  clean/rebuild 없이 기존 중간 산출물을 사용했다. 기존 shader/PDB warning은 존재한다.
- NetworkProtocolHarness Release 전체 PASS: numeric serialization/validation과
  통합 상점·Waterpang packet 포함. protocol size 검사의 누락된 Waterpang byte를 수정했다.
- Valtan lifecycle/presentation PASS. 네 CClientSession의 실제 Join, typed 이동/G 입장,
  3개 컷씬 stage와 등장 패턴, 8개 HP 구간의 순서·한 바퀴 반복, 7개 HP 기믹,
  Death→Respawn 40줄 복원, 실제 승인된 플레이어 스킬로 유령 처치,
  전원 동일 snapshot과 각 1회 clear/MVP 및 보상 수신까지 연속 확인했다.
  `VALTAN_4P complete=1 ticks=23344 window=7 mechanics=7 parity=1 ready=1`.
  로그: `out/ReleaseIntegration20260929/valtan-lifecycle-final.log` (failures 0).
  생존용 무적과 HP 문턱 입력은 fixture에서만 사용하며 제품 stage/timer/분기는 조작하지 않았다.
  처음의 무진행은 fixture가 G 전 실제 이동 명령을 생략하여 combat-ready가 되지 않은 원인이었다.
- Kouku raid, Bingo, NPC raid return Release PASS. 관문 입장/준비/재시도, 1~3관문과
  Bingo 종료/MVP 경계는 실제 Server simulation으로 확인했다.
  로그: `out/ReleaseIntegration20260929/raids/`.
- Numeric 실제 네 세션 PASS: 비공대장 저장 → 네 명 APPLIED, 경쟁 BUSY/stale,
  잘못된 수치 거절, Windows 파일 잠금에 의한 저장 실패 rollback,
  새 방 동일 수치와 EXE 재시작 catalog 복원을 확인했다.
  저장 후 Valtan Play/Restart가 새 boss HP를 사용하며 쿠크 10→8%와 ALT_V
  35→34 체력줄 피해를 독립 저장/복원한다.
  로그: `out/ReleaseIntegration20260929/numeric-balance-final.log` (failures 0).
- Skill stages Release PASS: notify 피해 창, 창술사 chain 및 Guardian Knight 포함.
  World playback Release PASS: Valtan의 새 게시 sequence 목록과 Kouku raid entry 예약을
  반영하여 오래된 fixture 가정을 교정했다. 로그는 같은 폴더의 `*-final.log`.
- 패키지 도구 nonlaunch 19 tests PASS. 외부 Server data 환경변수도 bundle 내부 경로로
  고정한다. 실제 Client/UI를 실행하지 않고 launcher manifest/hash/격리를 검사했다.
- 통합 변경 JSON 117개와 XML 6개 parse PASS, numeric source 576 fields/6 files 검증 PASS,
  git diff --check PASS. 리소스 참조 10,518개에서 누락 0건을 확인했다.

## 배포

`Tools/ReleasePackaging`에 기존 portable 실행 흐름을 재현 가능한 도구로 포함했다.
최상위 LostArk.exe, ServerHost.cmd, Release DLL/compiled shader, 필요한 Data/DataFiles와
app-local VC runtime을 포함하며 Resources는 Drive 공유본을 선택한다.
사용자 profile/CharacterRoster.json과 Resources는 ZIP에 포함하지 않는다.
새 PC도 #478의 고정 4개 클래스 카드/기본 닉네임을 보며 개인 JSON 닉네임은 별도 로컬 저장이다.
공유 endpoint는 `192.168.0.22:7777`이다.

대상: `C:/Users/user/Desktop/LostArk-Release-20260929.zip`.
기존 20260923 ZIP은 보존한다. 최종 ZIP 및 GitHub 병합 영수증은 아래에 후속 기록한다.

## 남은 사용자 화면 확인

AGENTS의 Client/UI 자율 실행 금지에 따라 실제 Client 네 창/네 PC를 실행하지 않았다.
서버 simulation/전송 frame 검증은 실제 LAN 전송 품질, GPU 표시, 실제 음향 청취 성공의
증거가 아니다. 네 명이 동일 ZIP과 최신 Drive Resources로 발탄/쿠크 컷씬·이펙트·음향을
확인해야 한다. 제품 실행 자체를 네 PC에서 완료했다고 기록하지 않는다.
