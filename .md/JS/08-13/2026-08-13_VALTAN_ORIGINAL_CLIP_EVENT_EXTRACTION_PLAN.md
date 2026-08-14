# 2026-08-13 발탄 원작 클립·이벤트 전량 추출 PLAN

작성자: JS · 2026-08-13

발탄 패턴 콜라이더 인계 작업(팀장 → JS)의 1단계. 타이밍 동기화·클립 바인딩의 재료인
원작 Action 데이터(클립 시퀀스, hit/notify 이벤트, 스테이지 길이)를 전량 확보해
Animation Tool에서 볼 수 있게 한다.

## 배경 (실측 완료)

- 발탄 = `EFTable_Npc` PK 480007 "마수군단장 발탄", 모델 `MN_RPBF_01`.
- `ValtanEncounter.json` 패턴들이 `sourceActionIds`(420601~420663, 63개)를 provenance로 보유.
- 원본 Action은 `XmlData/Action/MN_RPBF_00.loa`(7.3MB) 등 3파일. notify 이름에
  `MN_RPBF_00_420601_0_0_0`처럼 액션ID_스테이지가 박혀 있고 `CEFActionNotify_*` 클래스
  스트림이 클래스 `.loa`와 동일 계열 → `extract_action_loa.py` 파서 확장으로 해석 가능.
- 현재 `Character/Valtan/MN_RPBF_01.wmodel`은 27클립(공격 5계열)뿐. 클라 `CValtan`은
  패턴 무관 공용 3클립 재생(별도 후속 작업).

## 단계

1. `buildScript/extract_valtan_action_loa.py` 신설 — 몬스터 loa에서 액션ID → 클립 시퀀스·
   notify(종류/시각/파라미터)·스테이지 길이 추출 → `Data/Animation/Reference/Valtan/`에
   기존 `.clipseq/.animnotify` 계열 read-only 문서로 저장.
2. 추출 클립 전체 vs wmodel 27클립 대조 → 부족 클립 psa 추출 목록.
3. 부족 클립 포함 `MN_RPBF_01` 재쿠킹(39자 클립명 충돌 검증), 로컬 반영.
   리소스 정본 채택은 팀장 전달 후 결정.
4. Animation Tool(Monster/Boss 프리뷰 경로)에서 전 클립·notify 확인.

## 완료 기준

- 63개 sourceActionId 전부에 대해 클립·이벤트가 추출되거나, 실측 근거와 함께
  "원본에 없음"으로 분류된다. hit 이벤트가 없는 액션 목록이 드러난다.
- 재쿠킹 wmodel이 `validate_wmodel` 통과, 기존 presentation 6클립 이름 보존.
