# 2026-08-15 에스더 게이지·실리안 소환 런타임 RESULT

작성자: JS · 2026-08-15 · branch `feature/esther-summon-runtime`

PLAN: `2026-08-15_ESTHER_GAUGE_AND_SILIAN_SUMMON_RUNTIME_PLAN.md`

## 1. 구현 완료

PLAN §3의 변경 파일 표 그대로 구현했다. 요점:

- Shared v20: `C2S_USE_ESTHER_SKILL{seq, slot 1..3, aim}` 신규,
  `S2C_WORLD_SNAPSHOT`에 `iEstherGauge`/`iEstherGaugeMaximum`(max 0 = 에스더 없는
  world, gauge ≤ max 불변식).
- Server `CEstherSkillSystem`(신규 2파일, vcxproj/filters 등록): VALTAN_ARENA 한정,
  0~1000, 초당 200 충전(플레이어 있을 때만), 가득 시 슬롯1만 소모 → `NPC_59030`.
  슬롯 2·3(웨이·바훈투르)은 에셋 추출 전까지 UNSUPPORTED 거부.
- GameRoom: `Handle_UseEstherSkill`(사망자·미충전 거부, 소모 전 entity id 선검증으로
  부분 커밋 차단), `Spawn_EstherSummon`(시전자 위치·마우스 yaw, kind NPC,
  fCollisionRadius 0), 타임라인 `esther.appear 0.8s → esther.strike 3.2s →
  esther.leave 1.5s(+3.5/s 상승) → despawn`, 빈 방 리셋 시 게이지 0.
- Client: `Ctrl+Z/X/C` → `Request_EstherSkill`(Ctrl 프레임엔 일반 슬롯 차단 —
  Warlord Z/X 태세 보호), HUD 하단 게이지 바(가득 시 "ESTHER READY Ctrl+Z"),
  `NpcCatalog` optional `actionClips`로 `esther.strike → npc_att_battle_7_01` 재생
  (미등록 액션은 idleClip). Server는 클립 이름을 모른다.
- 소환 모델 admission은 기존 `CNpcPresentationAssetService::Ensure_Prototypes`
  lazy 경로 재사용 — 로더 변경 없음. 최초 소환 프레임에 모델 로드 히치 가능.

## 2. 자동 검증 (실행함)

- Shared/NetworkProtocolHarness/Server/Client x64 Debug 빌드 전부 exit 0.
- NetworkProtocolHarness: **failures 0** — 에스더 왕복 8건 + snapshot 게이지
  왕복·불변식 거부 2건 + 기존 전체. 버전 고정 검사 v19→v20 갱신 포함.
- `Server.exe --contract-test`: **failures 0** — 에스더 계약 9건(월드 게이팅,
  점유 시에만 충전, 슬롯 2·3/비레이드/미충전/재사용 거부, 소환 위치·yaw·소모,
  appear→strike→leave 전이, 상승, despawn, 빈 방 리셋) 포함 전체 통과.
- `git diff --check` clean.

## 3. 수동 검증 — 사용자 확인 완료 (2026-08-15)

로컬 Server(127.0.0.1:7777)+Client에서 발탄 진입 → 게이지 충전 → 가득 전
Ctrl+Z 무반응 → 가득 후 Ctrl+Z 소환(등장→대검 일격→상승 소멸, 게이지 0 리셋)
전 과정 정상 동작 확인. 1차 확인 피드백 3건을 같은 날 반영하고 재확인 완료:

1. 게이지 HUD를 하단 중앙 → 좌측 상단(기준 좌표 20~280, 56~68)으로 이동.
2. 최초 소환 프레임의 모델 로드 히치 제거 — Valtan 로더가
   `CNpcPresentationAssetService::Ensure_Prototypes("NPC_59030")`를 로딩 단계에서
   선실행(서버 roster 미러, 데이터 계약 승격 시 함께 이동).
3. 퇴장 상승 속도 `ESTHER_LEAVE_RISE_PER_SECOND` 3.5 → 12.0.

2클라이언트 동시 게이지 표시는 미확인(단일 클라 검증).

## 4. 남은 경계 (후속)

- 게이지 수치·충전 메커니즘은 테스트 상수(서버 코드 소유)다. 원작 충전 규칙 확정
  시 `Data/Balance` 정본 + publisher 계약으로 승격한다.
- 원작의 파티장 전용 사용 제한 없음(사용자 결정).
- 웨이(슬롯2)·바훈투르(슬롯3): 에셋 추출 후 `CEstherSkillSystem` roster에 추가.
- 소환 이펙트/사운드 없음 — 이펙트 담당 인계표는 추출 PLAN §2.
- leave 단계의 알파 페이드 없음(상승+despawn만). 컷인 UI 별도.
- 소환체는 데미지·판정 없음(연출 전용).
