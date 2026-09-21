# 발탄 컷신 자막·대사 원본 조사 결과

> 읽기 전용 조사다. `Data/`, `Client/`, `Server/`, Resources는 수정하지 않았다. 프로젝트 쪽은 `origin/main`(`7d5e27ea`)을 `git show`로 읽었다. 표기: **[실측]** 이번에 원본·파일에서 직접 읽은 값, **[추론]** 실측 값에서 이끌어낸 판단, **[미확인]** 확인하지 못한 것.

## 1. 결론

- 발탄 존의 컷신 자막 원본은 **정확히 8행**이다: 일반 4행 + 상단 2행(자막 트랙 6행) + 말풍선 2행. GameMsg `cin.37053_*`도 정확히 8행이고 **8행 모두 쓰인다**(자막 트랙에서 참조되지 않는 행 0) **[실측]**.
- 프로젝트 WorldSequence(`revision 20`)에 **8행 전부 붙어 있고** 시작·길이·위치·문구·말풍선 슬롯이 원본과 모두 일치한다(**원본에는 있는데 안 붙은 행 0, 어긋난 행 0, 원본에 없는데 붙은 행 0**) **[실측]**.
- 다만 붙어 있는 것과 제품 흐름에서 재생되는 것은 다르다. 팀원 문서(`2026-09-20_VALTAN_SCENE_SUBTITLE_RESULT.md` G4)에 따르면 자동 연출인 입장·버러지들·사망 6행은 제품 흐름에서 재생되고, 흰늑대·검늑대 말풍선 2행은 MapTool 미리보기 인스턴스(`gate1-entrance`)에만 붙어 있으며 현재 제품 흐름에 자동 1관문 입장 컷신이 없다. 이 경계는 문서 인용이고 이번에 제품 흐름을 직접 따라가 확인하지는 않았다 **[미확인]**.
- 그래서 "발탄 자막을 더 달 곳"은 데이터상 없다. 남은 것은 (a) 말풍선 2행을 제품 흐름에서 재생시키는 1관문 입장 연출 연결과 (b) 화면 확인이다.

## 2. 조사 범위와 방법

| 항목 | 값 |
|---|---|
| 설치 패키지(.upk) | 33,910개 전부 스캔 **[실측]** |
| 읽기 실패 | 16개, 전부 129바이트 스텁 파일(`implausible Lost Ark compressed chunk count 0`) — 내용이 없어 자료 없음 **[실측]** |
| 자막 트랙 클래스 `EFInterpTrackSubtitle` 보유 | 게임 전체 1,147개 중 발탄 문구(`cin.37053`) 사용은 2개(SCENE04A, SCENE06A) **[실측]** (쿠크 조사의 기존 스캔 결과 재사용) |
| 말풍선 트랙 클래스 `EFInterpTrackSubtitleBalloon` 보유 | 이번에 새로 전체 스캔: 게임 전체 512개(행 4,561), 그중 발탄 문구 사용은 1개(SCENE07A, 2행) **[실측]** |
| 이름표에 `heartrb`(발탄 존)/`37053`이 있는 패키지 | 401개 **[실측]**. 그중 발탄 장면 이름 `lv_lut_heartrb_ed_scene*`을 가진 것은 8개(매틴 있는 컷신 패키지 6 + 레벨 패키지 2) **[실측]** |
| 본문에 `cin.37053_*` 문자열이 있는 패키지 | 위 401개 전부를 압축 해제해 확인: 3개(SCENE04A, SCENE06A, SCENE07A) **[실측]** |
| 다른 텍스트 방식 | 6개 컷신 패키지와 2개 레벨 패키지의 이름표에서 메시지/공지/화면 문구 계열 클래스와 GameMsg형 키(`tip.`/`cin.`/`sys.` 등)를 찾았으나 없음. `EFInterpTrack*` 클래스는 자막·말풍선 외에 `autoblendfloatprop`, `vectorparticleparam`뿐이다 **[실측]** |

발탄 존 GameMsg 접두는 **`cin.37053_*`**(쿠크는 `cin.37081_*`)다. 근거: 발탄 장면 패키지의 자막·말풍선 행이 이 접두만 쓰고, 다른 접두를 쓰는 발탄 장면 패키지가 없다 **[실측]**. 인접 접두 `cin.37041`(3행), `cin.37061`(21행)은 다른 존으로 보이며 발탄 장면 패키지에서 쓰이지 않는다 **[추론]**.

## 3. 원본 자막 8행 (원문 그대로)

시간 변환: 원본은 초(float)이며 밀리초로 **반올림**(예: 20.644535s -> 20645ms, 3.275742s -> 3276ms). 프로젝트 저장 시각이 원본 시각과 정수 ms까지 일치하고, 발탄 인스턴스 11개는 모두 `startDelayMs` 0, `playbackSpeed` 1(phase2만 `1.0`)이며 인스턴스 필드에 시계 변환 항목이 없다 **[실측]**. 실행 시 WorldSequencePlayer가 template 시계를 어떻게 샘플하는지는 코드로 따라가지 않았다 **[미확인]**. 위치는 행이 재정의하지 않으면(`boverride_positiontype` false) 트랙 수준 값을 따른다: SCENE06A 버러지 트랙만 `upper`, 나머지 자막 트랙은 `normal`.

| GameMsg id | 원문(raw) | 원본 SCENE / 매틴 | 시작 ms | 길이 ms | 위치 | 말풍선 대상 | SOUND |
|---|---|---|---:|---:|---|---|---|
| `cin.37053_04_01` | 안돼, 안돼! | SCENE04A / interpdata_14 (23.000s) | 5460 | 2300 | NORMAL | - | 없음 |
| `cin.37053_04_02` | 크으으아아아악! 카마... 인...! | SCENE04A / interpdata_14 (23.000s) | 7860 | 6500 | NORMAL | - | 없음 |
| `cin.37053_09_01` | 이 버러지들... | SCENE06A / interpdata_1 (6.374s) | 623 | 1800 | UPPER | - | 없음 |
| `cin.37053_09_02` | 다... 짓뭉개주마! | SCENE06A / interpdata_1 (6.374s) | 3276 | 2500 | UPPER | - | 없음 |
| `cin.37053_03_01` | 나, 는... | SCENE06A / interpdata_0 (24.708s) | 19289 | 1300 | NORMAL | - | 없음 |
| `cin.37053_03_02` | 마수군단장... 발탄...! | SCENE06A / interpdata_0 (24.708s) | 20645 | 3500 | NORMAL | - | 없음 |
| `cin.37053_01_02` | 머리는 내 것이다, 루가루! | SCENE07A / interpdata_37 (13.000s) | 3733 | 1800 | BALLOON | 흰늑대 (원 balloon circle) | 없음 |
| `cin.37053_01_01` | 새로운 사냥감이군. *(끝 공백 있음)* | SCENE07A / interpdata_37 (13.000s) | 7500 | 2200 | BALLOON | 검늑대 (원 balloon circle) | 없음 |

원문에 `<br>`나 `<FONT ...>` 태그가 있는 행은 없다(8행 모두 plain과 raw 동일). `01_01`의 끝 공백은 원본에 있는 그대로다 **[실측]**.

## 4. 컷신별 표: 원본 SCENE <-> 프로젝트 템플릿 <-> 자막 행

| 원본 SCENE / 매틴 | 원본 길이 | 텍스트 트랙 | 프로젝트 템플릿(길이) | 자막 행 | 대응 근거 |
|---|---:|---:|---|---|---|
| SCENE04A<br>efseqact_matinee_14 (matineeindex 1) | 23.000s | 1 | valtan.source-preview.finale (23000ms) | `cin.37053_04_01`, `cin.37053_04_02` (2행) | 길이 23.0s = 23000ms 일치 [실측], 자막 시각 정확 일치 [실측], 템플릿 이름 "발탄 최후" [실측] |
| SCENE06A<br>efseqact_matinee_0 (matineeindex 1) | 24.708s | 1 | valtan.source-preview.entrance (24708ms)<br>entrance.colorless / actor64.body.0 / actor64.weapon.0 / actor64.weapon.1 (같은 24708ms 보조 배우) | `cin.37053_03_01`, `cin.37053_03_02` (2행) | 길이 일치 [실측], 자막 03_01/03_02 시각 정확 일치 [실측] |
| SCENE06A<br>efseqact_matinee_1 (matineeindex 9) | 6.374s | 1 | valtan.source-preview.trash (6374ms) | `cin.37053_09_01`, `cin.37053_09_02` (2행) | 길이 일치 [실측], 트랙 그룹 이름 "발탄", 자막 09_01/09_02 정확 일치 [실측] |
| SCENE06A<br>efseqact_matinee_3 (matineeindex 4) | 7.003s | 0 | valtan.source-preview.roar (7003ms) | 자막 없음 | 길이 일치 [실측]. 원본에 텍스트 트랙 없음 [실측] -> 자막 없음 |
| SCENE06A<br>efseqact_matinee_4/5/6 (matineeindex 7/6/8) | 6.724s | 0 | (대응 템플릿 없음) | 자막 없음 | 길이 6.724s 3개. 프로젝트 템플릿 중 대응 후보를 확인하지 못함 [미확인]. 원본에 텍스트 트랙 없음 [실측] |
| SCENE07A<br>efseqact_matinee_37 (matineeindex 1) | 13.000s | 2 | valtan.source-preview.gate1-entrance (13000ms, 흰늑대)<br>valtan.source-preview.gate1-entrance.black-wolf (13000ms, 검늑대) | `cin.37053_01_01`, `cin.37053_01_02` (2행) | 길이 13.0s 일치 [실측], 말풍선 그룹 "흰늑대"/"검늑대"와 템플릿 이름 일치 [실측], 시각 정확 일치 [실측] |
| SCENE02A<br>efseqact_matinee_20 (matineeindex 8) | 5.500s | 0 | valtan.source-preview.phase2 (5500ms) | 자막 없음 | 길이 5.5s = 5500ms 일치만 확인 [추론]. 원본에 텍스트 트랙 없음 [실측] -> 자막 없음 |
| SCENE02A<br>efseqact_matinee_14/4/5 (matineeindex 2/4/9) | 0.1 / 2.97 / 0.1 | 0 | (대응 템플릿 없음) | 자막 없음 | 프로젝트 템플릿 없음. 원본에 텍스트 트랙 없음 [실측] -> 자막 없음 |
| SCENE02A01 / SCENE02A02<br>각각 3개(붕괴 collapse 계열) | 0.1 ~ 2.5 | 0 | (대응 템플릿 없음) | 자막 없음 | 원본에 EFInterpTrack* 클래스 자체가 없음 [실측] -> 자막 없음 |

프로젝트의 발탄 WorldSequence 템플릿 11개 중 자막이 붙은 것은 5개(흰늑대, 검늑대, entrance, finale, trash)이고 나머지 6개(entrance.colorless, roar, phase2, actor64.body.0, actor64.weapon.0, actor64.weapon.1)는 원본에 텍스트 트랙이 없거나 같은 시계의 보조 배우라서 자막이 없는 것이 맞다 **[실측]**. 인스턴스 11개는 템플릿과 1:1이다.

## 5. 프로젝트 저장 위치와 현재 붙음 상태

- 저장 위치는 쿠크(Composition의 SUBTITLE 리소스)와 달리 WorldSequence 문서의 템플릿별 `subtitleTracks`다: `Data/Maps/Authoring/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.worldsequences.json`. 게시 출력 `Client/Bin/DataFiles/Map/LV_LUT_HEARTRB_ED.worldsequences.json`과 내용이 같고(`origin/main` 기준) `revision`은 20이다 **[실측]**.
- 필드: `subtitleTrackId`, `stringId`, `text`(GameMsg 한국어 plain), `position`(NORMAL/UPPER/BALLOON), `slotId`(말풍선은 `actor`), `startMs`, `durationMs`.

| 프로젝트 템플릿 | subtitleTrackId | 시작 | 길이 | 위치 | slot | 원본과 일치 |
|---|---|---:|---:|---|---|---|
| `finale` | `subtitle.valtan.cin.37053_04_01` | 5460 | 2300 | NORMAL | - | 예 |
| `finale` | `subtitle.valtan.cin.37053_04_02` | 7860 | 6500 | NORMAL | - | 예 |
| `trash` | `subtitle.valtan.cin.37053_09_01` | 623 | 1800 | UPPER | - | 예 |
| `trash` | `subtitle.valtan.cin.37053_09_02` | 3276 | 2500 | UPPER | - | 예 |
| `entrance` | `subtitle.valtan.cin.37053_03_01` | 19289 | 1300 | NORMAL | - | 예 |
| `entrance` | `subtitle.valtan.cin.37053_03_02` | 20645 | 3500 | NORMAL | - | 예 |
| `gate1-entrance` | `subtitle.valtan.cin.37053_01_02` | 3733 | 1800 | BALLOON | actor | 예 |
| `gate1-entrance.black-wolf` | `subtitle.valtan.cin.37053_01_01` | 7500 | 2200 | BALLOON | actor | 예 |

## 6. 원본에는 있는데 안 붙은 행 / 어긋난 행 / 원본에 없는 행

- **원본에는 있는데 프로젝트에 안 붙은 행: 0개** **[실측]**.
- **붙어 있지만 시각·길이·위치·문구·슬롯이 어긋난 행: 0개** **[실측]** (8행 모두 반올림 후 정수 ms까지 일치).
- **원본에 없는데 프로젝트에 붙은 행: 0개** **[실측]**.
- 팀원 주장 검증: "일반/상단 6개와 배우 말풍선 2개, 총 8개, source revision 20" -> 일반 4 + 상단 2 = 6, 말풍선 2, 합계 8, revision 20 모두 **사실** **[실측]**.

## 7. 참고: 쿠크 조사에 대한 보충

- 쿠크 자막 조사의 전체 스캔은 `EFInterpTrackSubtitle`만 찾았고 말풍선 클래스는 찾지 않았다. 이번에 말풍선 클래스를 게임 전체(33,910개)에서 새로 스캔했고 **`cin.37081_*`(쿠크) 말풍선 행은 0개**다 **[실측]**. 따라서 쿠크 자막 결론(붙지 않은 것은 앵콜 3행뿐, `29_01`과 `49_01~05`는 트랙에서 미참조)은 말풍선 트랙을 포함해도 바뀌지 않는다. 다만 `49_*`가 다른 방식으로 쓰이는지는 여전히 **[미확인]**이다.

## 8. 확인하지 못한 것 [미확인]

- 제품 흐름에서 실제로 어떤 발탄 연출이 재생되는지(입장·버러지들·사망의 재생 경로, 1관문 입장 컷신 부재)는 팀원 문서 인용이다. 코드로 따라가 확인하지 않았다.
- SCENE06A의 매틴 3개(`efseqact_matinee_4/5/6`, 각 6.724s)와 SCENE02A의 매틴 3개(0.1s, 2.97s, 0.1s)에 대응하는 프로젝트 템플릿을 찾지 못했다. 텍스트 트랙이 없으므로 자막에는 영향이 없다.
- SCENE02A `efseqact_matinee_20`(5.5s)이 프로젝트 `phase2`(5500ms)에 대응하는지는 길이 일치만으로 판단했다 **[추론]**.
- 전투 중 대사(보스 패턴 외침, 몬스터 말풍선)는 컷신 자막이 아니라서 조사 범위 밖이다.
- 화면에서 자막·말풍선이 어떻게 보이는지는 확인하지 않았다(Client 미실행).
- 읽기 실패 16개 패키지는 129바이트 스텁이라 자료가 없다.

## 9. 산출물과 재실행

- 기계용 자료: `out/ValtanSubtitleInventory20260920/valtan_subtitle_inventory.json`
- 스크립트·중간 자료: `C:\LostArkExtract\ValtanSubtitle_20260920\scripts\`, `...\data\`
  - `scan_heartrb.py [workers]`: 전체 패키지 이름표 스캔(발탄 존 패키지 401개 추림, 약 9분)
  - `scan_balloon_all.py [workers]`: 말풍선 클래스 전체 스캔(약 2분, 이어서 실행 가능)
  - `dump_valtan_scenes.py`: 발탄 컷신 패키지 6개의 트랙·행 덤프
  - `project_valtan_side.py`, `map_and_compare.py`: `origin/main`의 WorldSequence를 읽어 원본과 대조
  - `build_outputs.py`: 이 문서와 JSON 생성
- 전제 데이터: 쿠크 조사의 스캔 결과 `C:\LostArkExtract\KoukuSubtitle_20260920\data\hits.jsonl`(자막 트랙 클래스 전체 스캔), 복호화한 `EFTable_GameMsg.db` 사본.
