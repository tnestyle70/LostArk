# 2026-08-20 발탄 패턴 클립 체인 RESULT

작성자: JS · 2026-08-21 · branch `feature/valtan-pattern-sequences`

PLAN: `2026-08-20_VALTAN_PATTERN_CLIP_CHAINS_PLAN.md`

## 1. 구현 완료

### A. 제품 런타임: 패턴 스테이지 클립 체인 + 저작 타격 시각

- `Valtan.patternbindings.json` v1의 `clip`이 문자열 또는 배열(체인, 최대 16)을
  받는다. `CValtanPatternAnimationBindingDocument` parse/validate가 체인 전 클립을
  모델 클립 목록과 대조하고, 한 클립 체인은 기존 동작과 동일하다.
- `CValtan::Apply_NetworkState` 패턴 분기가 스테이지 age를
  `CActionPresentationTimeline::Resolve_Sample`로 체인에 seek한다(마지막 클립
  loop). Effect_Tool의 두 소비처(ActionsByClip, drift guard)를 체인 대응으로 수정.
- Server `PATTERNSTAGE`에 `hitDelayMs` 추가(bootstrap v12, 22필드).
  `CValtanBrain` 타격 시각 = 스테이지 시작 + hitDelayMs + k×hitIntervalMs.
  검증: NONE 셰이프는 delay=0 강제, delay ≥ durationMs 거부.
- `hitDelayMs` 스키마 소비자 연결: `Publish-GameplayBalance.ps1`(v12 emit),
  `Publish-ValtanWorldDestruction.ps1`, `validate_boss_pattern_effects.py`,
  `CEncounterPatternReference`, F1 Balance Tool(파싱·UI 편집·draft 검증·직렬화
  round-trip).
- 13개 패턴 저작(원작 clipseq 체인 순서 + animnotify len 길이 + HIT notify 접촉
  시각): 휘두르기, 내려찍기, 감금 사자후, 지진 찍기, 큰 베기 반격, 마력기운
  양자택일, 4연속 베기, 고공 점프 찍기, 지진파 내려찍기, 휠윈드, 붉은 검기,
  앞뒤앞 내려찍기, 두손 안밖 폭발. 인카운터 42스테이지 duration/delay/interval,
  바인딩 21건, 이펙트 큐 38창(start=delay, end=duration 동기화). hitCount는
  밸런스 보존을 위해 전부 유지.
- **대쉬 돌진은 보류**: ACTIVE 연장 시 서버 forced motion으로 돌진 거리가 함께
  늘어나 별도 설계 필요.

### B. Animation Tool: Valtan Pattern Reference 창

- 스킬 레퍼런스식 플로팅 창. `Valtan.clipseq` 265 시퀀스가 한글 이름 버튼으로
  나열되고 클릭 시 해당 체인이 씬 발탄에서 이어 재생된다(기존 preview 재생 경로
  재사용, 팀장 1-67 인라인 프리뷰는 무수정 유지). 필터(스킬 ID/이름),
  Raid-only(420xxx) 체크박스, Pause/Replay/Stop/배속, 재생 중 버튼 하이라이트.
- 스텝 길이는 원작 우선: 신규 `Valtan.clipcuts`(스테이지의 무조건
  `MonsterMoveNextStage` 전환 시각) > `.animnotify len=` > 클립 원 길이.
  0.02초 미만 컷 스텝은 원작처럼 스킵. `extract_action_loa.py`(개인
  buildScript)에 clipcuts 출력을 추가하고 재추출 — 기존 4개 Reference 산출물은
  byte 동일 검증 후 clipcuts만 추가 커밋.

## 2. 검증

- `Publish-GameplayBalance.ps1 -Mode Validate` PASS (33 field PROJECT_TUNED
  동기화, 124 스테이지), `Publish-ValtanWorldDestruction.ps1 -Mode Validate`
  PASS, `validate_boss_pattern_effects.py` PASS.
- `Server.exe --contract-test` **failures: 0** (hitDelay 수용/거부 신규 3건 포함,
  ordered audition 예산 9000→27000틱 — 패턴 길이 증가분).
- `ClientFrontendHarness` failures 62 = 기존 Effect 계열 기준선(팀장 몫)과 동일,
  Valtan 관련 전부 PASS(체인 parse/거부/보존, 실제 큐 문서 99건).
- Server/Client/Harness Debug 빌드 성공.
- 사용자 육안: 발탄 아레나 1-67 audition에서 체인 재생 확인(리듬 개선),
  Animation Tool 시퀀스 재생이 clipcuts 적용 후 원작 리듬에 근접함을 확인.

## 3. 남은 경계 / 다음 단계

- 대쉬 돌진 체인화(forced motion 연동 설계 필요).
- 조건부 `MonsterMoveNextStageCondition*` 스텝은 툴에서 전체 길이로 재생됨(런타임
  조건이라 데이터만으로 재현 불가) — 필요 시 "조건 발동 가정" 토글.
- 타격 시각 추정치 3곳(HIT notify 부재): 고공점프 착지 900ms, 앞뒤앞
  900/1500ms, 두손 안 1000ms — 육안 튜닝 대상.
- 미저작 패턴(STOMP, BACKSTEP, LEDGE_ROAR, 기믹류)은 기존 단일 클립 유지.
- 아레나 스테이지 경계 1프레임 스냅 가능성(스테이지 길이==체인 길이 랩) 진단만
  기록 — 현재 재현 판정 아님.
