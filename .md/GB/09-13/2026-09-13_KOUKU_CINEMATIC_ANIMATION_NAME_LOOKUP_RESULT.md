# 쿠크 1관문 연출 애니메이션 한글 이름 조사

2026-09-13. 사용자 요청은 Animation Resources에서 1관문 연출이 한글 이름으로 제공되는지 확인하고, 기존 패턴과 같은 원본 데이터 대조 방식으로 연출 자료를 찾는 것이다. 조사 기준은 `codex/kouku-donut-ball-motion`, HEAD `e6f19ec806c7f41eacc37553a4d919cb17163d18`와 현재 미커밋 저장본이다. 제품 코드·Data·Resources를 수정하지 않았다.

## 확인한 결론

1관문 연출 전체의 한글 Action 항목은 현재 Animation Resources에 없다. 애니메이션 실물이나 원본 Action을 가져오는 과정에서 누락된 것은 아니다. 일반 패턴 목록은 원본 Action의 한글 `displayName`과 연결된 클립을 읽지만, 이 연출은 별도 맵 Matinee의 A/B 애니메이션 트랙을 사용한다. 원본 Matinee와 원본 Action은 같은 클립을 재사용할 수 있고, 그 경우 전투용 Action 이름이 연출 전체의 이름을 뜻하지는 않는다.

현재 `Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json`의 `KAKULSAYDON_G1_PATTERN_36`, `세이튼_1관문연출`은 프로젝트에서 저작한 패턴이다. MN_RPCT_05의 클립 10종을 13구간으로 사용하며, 각 구간은 `sourceActionId=0`, `sourceStageId=RAW`다. 이 한글 표시명을 원본 Action 이름으로 분류하지 않는다.

## 일반 패턴과 같은 원본 대조

원본 입력 위치는 `C:/LostArkExtract/LV_LUT_MIDNIGHTC_ED_20260829/RemainingCharacterExtraction-20260829/ActionNameSources/`의 `<Profile>.action-effects.json`이다. 현재 목록은 `Data/Animation/Reference/KoukuSaydon/<Profile>.actionreference.json`이다. 원본 `actionId`와 현재 `sourceActionId`의 집합을 직접 비교했다.

| Profile | 원본 Action | 현재 목록 | 누락 ID | 추가 ID |
|---|---:|---:|---:|---:|
| MN_RPCT_05 | 115 | 115 | 0 | 0 |
| MN_RPCT_06 | 47 | 47 | 0 | 0 |
| MN_RPCT_07 | 102 | 102 | 0 | 0 |
| MN_RPCZ_00 | 85 | 85 | 0 | 0 |
| 합계 | 349 | 349 | 0 | 0 |

네 원본 보고서의 `actionFilter.minimum`, `maximum`, `actionIds`는 모두 null이다. 생성기 `Tools/KoukuSaydonPipeline/build_kouku_saydon_animation_reference.py`는 해당 Action 보고서와 설치 WModel을 대조하며, 기본 STAND/MOVE 등의 명칭만 공통 한글 이름으로 변환한다. Matinee의 장면·애니메이션 트랙을 이 Action 목록으로 수집하지 않는다.

다음 세 클립은 네 actionreference의 모든 Action/Stage/Slot을 대조해도 연결되는 한글 Action이 0개였다. 원본 Action 보고서에도 해당 클립이 없다.

| 설치 클립 | SCENE03A 원본 확인 위치 | 원본 local 시작 시각 |
|---|---|---|
| `rpct00_idle_phase1_start_1` | group 1059/1062/998의 A/B 트랙 | 19.548820초, 33.643795초 |
| `rpct00_evt2_blk6` | group 1062 `kuku02`, track 1245, slot A | 29.947241초 |
| `rpct00_evt2_rpct_talk_01` | group 1062 `kuku02`, track 1246, slot B | 30.480902초 |

시각은 원본 Matinee local time이다. 현재 편집한 P36이나 통합 시퀀스 박스의 시각으로 대신 사용하지 않는다. talk의 원본 Source In은 0.800000초다. `blk6`의 의미를 이름만으로 특정 동작으로 번역하지 않았다.

## 이미 한글 패턴에 연결된 공유 클립

| 원본 한글 Action | Action ID | 연출과 공유하는 설치 클립 |
|---|---:|---|
| 세이튼_서커스 룰렛 춤A | 4219813 | `rpct00_att_battle_13_01`, `rpct00_att_battle_13_02` |
| 세이튼_서커스 룰렛 춤B | 4219814 | `rpct00_att_battle_13_04`, `rpct00_att_battle_13_02` |
| 댄스타임 준비 | 4219879 | `rpct00_att_battle_25_01` |

이 항목은 `Animation Resources → Saydon → MN_RPCT_05`에서 찾는다. 이동·대기 클립도 여러 Action에서 재사용한다. 별도로 발견한 `MN_RPCT_07 / 4219905 / 빙고맵 오프닝`은 1관문 연출과 다른 Action이며, 그 한글 이름을 1관문 연출에 붙이지 않는다.

## 찾은 원본 연출 자료

원본 장면은 `LV_LUT_MIDNIGHTC_ED_SCENE03A → EFSeqAct_Matinee_0 → InterpData_0`이다. 추출 export는 Matinee 365 / InterpData 857, 길이는 41.487556초다. 기존 추출본 `out/KoukuSourceSequenceRestore20260912/LV_LUT_MIDNIGHTC_ED_SCENE03A.json`을 다시 읽어 다음 세 배우 그룹과 원본 키를 확인했다.

| 원본 그룹 | Group export | Animation track export | 키 수 |
|---|---:|---|---:|
| `kuku` | 1059 | 1243 A, 1244 B | 4 |
| `kuku02` | 1062 | 1245 A, 1246 B | 11 |
| `kuku_동기화` | 998 | 1240 A, 1241 B | 5 |

세 배우의 20키에서 중복을 제거하면 클립 10종, 시간·클립 기준 13회 발생이다. A/B가 동시에 동작하므로 기존 13구간 편집 패턴과 원본 혼합 결과의 동일성을 이 조사로 판정하지 않는다.

SCENE03A의 원문 주석에는 `커다란 서커스 천막 팝업 연출`, `커다란 서커스 천막 내부이동 연출`, `천막 진입 매크로 카메라`, `서커스 천막 환호하는 관중 연출` 등이 있다. 이들은 장면 프레임/카메라 설명이고 위 세 클립 각각의 한글 이름은 아니다.

두 evt2 클립의 원본 PSA는 다음 파일이며 현재 존재를 확인했다.

`C:/LostArkExtract/LV_LUT_MIDNIGHTC_ED_20260829/CanonicalSource/Character/UModelExports/MN_RPCT_00/Export/MN_RPCT_00/ani/mn_rpct_00_evt2_ani.psa`

설치 모델은 `Client/Bin/Resources/Character/KoukuSaton/MN_RPCT_05/MN_RPCT_05.wmodel`이다. 현재 원본 이름으로 찾는 경로는 `Resources → Physical Animation → MN_RPCT_05`이며, 단독 Workbench에서는 `Animation Resources → Physical Clips → MN_RPCT_05`다.

## 검증과 남은 경계

- 현재 네 reference와 원본 네 Action 보고서의 JSON을 파싱하고 Action ID 전체 집합 일치를 확인했다.
- 현재 P36의 10종 클립을 네 reference의 모든 slot에 대조했다.
- SCENE03A의 원본 3개 그룹, 6개 animation track, 20개 키와 한글 주석을 다시 읽었다.
- 분석용 기존 PLAN/RESULT의 설명과 현재 P36을 구분했다. 과거 문서의 P35를 현재 ID로 사용하지 않았다.
- 이번 변경 파일은 이 결과 문서뿐이다. 한글 연출 목록 등록, 클립 변경, publisher, 빌드와 Client/UI 재생은 수행하지 않았다.

관련 기존 근거는 `.md/GB/09-11/2026-09-11_KOUKU_CUTSCENE_SOURCE_VISUAL_AUDIT_RESULT.md`, `.md/GB/09-12/2026-09-12_KOUKU_ANIMATION_CLIP_RANGE_RESULT.md`, `out/KoukuActualBossSequenceReview20260912/g1_native_key_order.md`다.
