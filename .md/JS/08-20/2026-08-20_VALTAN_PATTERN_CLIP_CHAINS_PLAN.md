# 2026-08-20 발탄 패턴 클립 체인 제품 런타임 PLAN

작성자: JS · branch `feature/valtan-pattern-sequences` (`41236c12` 위)
선행: 팀장 `Valtan.patternpreview.json` v2(65패턴 한글 라벨·원본 시퀀스 매핑),
Animation Tool 발탄 패턴 프리뷰, `2026-08-20_ESTHER_WAYE_BAHUNTUR_SLOTS_RESULT.md`
(actionClips 체인 선례)

## 목표

제품 발탄 전투에서 패턴 스테이지가 스테이지당 1클립 + 임의 duration으로 끊기는
현재 상태를, **원작 시퀀스의 클립 체인 + 원본 클립 길이 기반 duration**으로
교정한다(사용자 허용: encounter durationMs 변경 포함). 툴 프리뷰가 아니라
서버 권위 전투 경로(`ValtanEncounter → snapshot → CValtan`)가 대상이다.

## 실측 (2026-08-20)

- 제품 계약: `ValtanEncounter.json` 33패턴 × 3~N 스테이지(`actionId`,
  `durationMs`, hit/push/knockdown). 클라 `Valtan.patternbindings.json` v1이
  `actionId → 클립 1개`, `CValtan::Apply_NetworkState`가 스테이지 age를
  단일 클립 Timing으로 `CActionPresentationTimeline::Resolve_Sample` 시크.
- `Resolve_Sample`은 이미 `span<ACTION_PRESENTATION_CLIP_TIMING>` 체인을 받는다
  (플레이어 스킬 다중 클립과 같은 인프라) — 클라 확장은 span만 바꾸면 된다.
- 원본 정본: `Data/Animation/Reference/Valtan/Valtan.clipseq`(sourceActionId·
  seq별 ordered clips, 예: 휠윈드 420633 seq4 =
  `mesh_idle_battle_1, 20_02, 20_03, 20_04`), `Valtan.animnotify`(클립별 len=
  스테이지 재생 길이). 패턴→원본 액션 매핑 근거는 `patternpreview.json`의
  라벨·confidence와 encounter `displayName`.
- 이펙트 cue(`patterneffectcues`)는 pattern/stage/age 기준이라 체인화와 직교.

## 계약

| 계층 | 계약 |
|---|---|
| Data(클라) | `Valtan.patternbindings.json` v1 유지하되 `clip`이 문자열 또는 순서 배열(NpcCatalog actionClips와 같은 형식). 체인은 스테이지 안에서 순차 1회씩 재생, 마지막 클립은 스테이지 잔여 시간 hold(비루프) |
| Data(서버) | `ValtanEncounter.json` 스테이지 `durationMs`를 해당 스테이지 체인의 원본 클립 길이 합(animnotify len, 30fps)으로 교정. hit/push 수치는 유지(스테이지 상대 타이밍) |
| Client | 바인딩 파서 v2(문자열/배열), `CValtan` 패턴 시크를 체인 span으로 확장(클립 존재 검증 fail-closed 유지). 스테이지 전환·역행 가드, effect cue 경로는 기존 그대로 |
| Server | 코드 변경 없음(듀레이션은 데이터) — publisher/contract 재검증만 |
| 도구 | 매핑 작업용 스크래치 스크립트: clipseq+animnotify를 조인해 sourceActionId·seq별 "클립: len" 표 출력(저장소 밖 scratchpad) |
| 검증 | 바인딩 로더의 잘못된 체인(빈 배열/미존재 클립) 거부, `Publish-GameplayBalance`(또는 encounter publisher) Validate, `Server --contract-test`, Client 빌드, 발탄 육안(휠윈드 등 핵심 패턴이 원작처럼 이어지는지) |

## 진행 순서

1. **G1 인프라**: 바인딩 v2 파서 + CValtan 체인 재생. 기존 단일 클립 데이터
   그대로 동작(배열 미사용 시 무변화) 확인.
2. **G2 데이터**: 조인 표를 뽑아 패턴별로 체인·duration 교정. patternpreview
   confidence가 USER_CONFIRMED/SOURCE_FAMILY_DIRECT인 계열부터(휠윈드,
   4연속 베기, 지진 찍기, 대쉬 돌진, 고공 점프, 앞뒤앞 등), CANDIDATE/
   UNRESOLVED는 현행 유지하고 RESULT에 목록화.
3. **G3 검증**: publisher Validate, contract test, 육안.

## 결정

1. 스테이지 구조(WINDUP/ACTIVE/RECOVERY 수)는 유지 — 서버 판정·이펙트 cue가
   스테이지 식별에 묶여 있으므로 스테이지 안에서 체인만 확장한다.
2. 원본 시퀀스 선두의 `mesh_idle_battle_1`류 대기 클립은 제품 체인에서 제외
   (서버 select 단계가 이미 그 간격을 소유).
3. duration 교정으로 패턴 총 길이가 변하는 것은 사용자 허용(2026-08-20).
