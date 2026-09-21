# 쿠크 컷신 자막 원본 데이터 조사 결과

- 날짜: 2026-09-20, 기준: 작업 폴더 = `main` `8bce1318` (Action revision 1918, Sequence revision 120)
- 성격: **조사만 했다.** `Data/`, `Client/`, `Server/`, Resources는 수정하지 않았다(이 문서와 `out/` 산출물만 추가). 자막을 붙이는 작업은 별도 지시를 기다린다.
- 표기: **[실측]** 이번 세션에서 파일·DB·패키지를 직접 읽은 값, **[추론]** 실측 값들에서 이끌어낸 판단, **[미확인]** 확인하지 못한 것.
- 기계가 읽는 같은 내용: `out/KoukuSubtitleInventory20260920/subtitle_inventory.json` (git 제외 폴더).

## 0. 한 화면 결론

1. 아까 앵콜 조사에서 자막을 데이터로 찾은 것이 맞다. **[실측]** 앵콜(SCENE07A) 자막은 `EFTable_GameMsg`의 `cin.37081_12_01/02/03`이고, 시각은 컷신 패키지의 `EFInterpTrackSubtitle` 트랙에서 읽었다.
2. 이번에는 해시 접두사가 같은 9개만 본 게 아니라 **설치된 패키지 33,910개 전부**를 스캔했다. 자막 트랙이 있는 패키지는 게임 전체에서 1,147개이고, 그중 쿠크 존 자막(`cin.37081_*`)을 쓰는 패키지는 **7개**(SCENE01B, 01C, 02A, 02B, 03A, 04A, 07A)뿐이다. **[실측]**
3. 원본 GameMsg `cin.37081_*`는 정확히 **25행**이다. 이 중 20행이 컷신 자막 트랙에 들어 있고, 나머지 5행(`cin.37081_49_01~05`)은 어떤 자막 트랙에도 쓰이지 않는다. **[실측]** `SOUND` 열은 25행 모두 비어 있다.
4. 현재 main에는 팀원이 자막 리소스 19종(GameMsg id)을 Action과 Sequence Composition에 넣어 두었고, 그중 16종이 패턴에 배치돼 있다(Action 14행 + Sequence 17행 = 31행). 나머지 3종(앵콜 `12_01~03`)은 리소스만 있고 배치가 없다. **내가 원본에서 독립적으로 뽑은 값과 대조하니 배치된 31행 전부 시작·길이·위치·문구가 일치한다.** 어긋난 행은 없다. **[실측]**
5. **아직 안 붙은 것: 앵콜컷신(Action P97, Sequence P10)의 자막 3행**이다. 자막 리소스 3개는 이미 두 문서에 있고 값도 원본과 같지만, 두 패턴에는 자막 배치(occurrence)가 하나도 없다(카메라 1개, 이펙트 1개뿐). 위치는 **UPPER**여야 한다(원본 트랙이 upper). **[실측]**
6. 그 밖에 안 붙은 것: `cin.37081_29_01`(뿅망치 살인마, SCENE02A 안의 별도 9.4초 컷신)은 프로젝트에 대응하는 컷신·카메라가 없어서 붙일 곳이 없고, `cin.37081_49_01~05`는 원본 어디에서도 안 쓰인다. **[실측]**

## 1. 조사 범위와 방법

| 항목 | 내용 |
|---|---|
| 대상 | 설치 게임 `C:\ProgramData\Smilegate\Games\LOSTARK\EFGame\ReleasePC\Packages` 의 `.upk` **33,910개 전부**(합계 83.5 GB) **[실측]** |
| 1차 필터 | 각 패키지에서 이름 표 구간만 부분 복호화해 클래스명 `EFInterpTrackSubtitle` 유무를 확인. 큰 패키지(130MB급)도 0.02초 안에 판별 **[실측]** |
| 2차 추출 | 필터에 걸린 패키지만 전체 복호화해 자막 트랙의 모든 행(GameMsg id, 시각, 길이, 위치, 덮어쓰기 필드)과 소속(트랙 → InterpGroup → InterpData)을 기록 **[실측]** |
| 결과 | 자막 트랙 패키지 1,147개(게임 전체), 그중 `cin.37081_` 참조 7개. 읽기 실패 16개는 모두 **129바이트 스텁 파일**(헤더 동일, 내용 없음)이라 자료가 없다 **[실측]** |
| 쿠크 존 패키지 확정 | 전체 이름 표에서 `midnightc` 이름을 가진 패키지 56개를 모아 분류. 쿠크 컷신 SCENE 패키지는 해시 접두사 `B9AVB2VAZIQRPQCJVKAVYRAVOKYPY` 9개(806=SCENE04A, 8E6=SCENE06A, 8F6=SCENE01A, 8FD=SCENE01B, 8FK=SCENE01C, 8L6=SCENE07A, 8M6=SCENE02A, 8MD=SCENE02B, 8T6=SCENE03A). 나머지는 맵 레벨·사운드·이펙트 패키지이고 자막 클래스가 없다 **[실측]** |
| 문구 | 복호화한 `EFTable_GameMsg.db`(706,743행)에서 `cin.37081_*` 25행을 읽음. 원문과 plain 변환본(`<br>`→줄바꿈, `<FONT>` 태그 제거)을 함께 기록 **[실측]** |
| 프로젝트 쪽 | main의 Action/Sequence Composition JSON을 직접 읽음. 카메라 접두사→원본 SCENE 대응과 시각 변환 규칙은 프로젝트 도구 `Tools/KoukuSaydonPipeline/build_scene_subtitle_candidates.py`의 표(`SCENES`, `LANDMARKS`)를 그대로 썼고, 원본 값은 내 추출 결과를 넣어 "붙어야 하는 값"을 다시 계산해 실제 값과 비교했다 **[실측]** |

스캔 검증: 33,910개 전부 처리됐다. 기록 파일에서 파일명에 **공백이 들어 있는 7개**가 잘못 쪼개져 빠진 것처럼 보였으나 직접 다시 스캔해 처리됐음을 확인했고 새로 나온 자막 패키지는 0개다 **[실측]**.

## 2. 원본 자막 25행 전체와 프로젝트 상태

표기: `#` 번호는 InterpData의 export 번호(1부터 셈)다. SCENE01B와 SCENE01C는 같은 자막을 가진 변형이라 둘 다 적었다.

시각 변환: 초 × 1000을 Python `round()`로 정수 ms로 만든다(1관문 P4만 프로젝트의 구간 시계표 `LANDMARKS`를 거치고, 3관문 진입 P7은 16,710ms를 뺀다). 길이는 (끝 시각 ms) − (시작 시각 ms)다. 위치는 "행이 덮어쓰면 행 값, 아니면 **트랙 값**, 없으면 normal" 규칙이다.

| GameMsg id | 문구(plain) | 원본 위치(패키지·InterpData) | 원본 시각/길이(초) | 위치 | 프로젝트에 붙은 곳 (패턴, 시작ms/길이ms) | 상태 |
|---|---|---|---|---|---|---|
| `cin.37081_12_01` | 누구 맘대로 끝을 내?! | SCENE07A(8L6) · interpdata_23 (#44, 23.333초) · 트랙 sub_0 | 10.2333 / 1.7000 | UPPER(트랙) | - | **미부착**(Action P97, Sequence P10) |
| `cin.37081_12_02` | 무효야, 전부 무효! | SCENE07A(8L6) · interpdata_23 (#44, 23.333초) · 트랙 sub_0 | 12.3667 / 3.6000 | UPPER(트랙) | - | **미부착**(Action P97, Sequence P10) |
| `cin.37081_12_03` | 진짜 시작은 지금부터라고! | SCENE07A(8L6) · interpdata_23 (#44, 23.333초) · 트랙 sub_0 | 16.7000 / 3.0500 | UPPER(트랙) | - | **미부착**(Action P97, Sequence P10) |
| `cin.37081_22_01` | 고맙습니다. 고맙습니다. 여러분! 고맙습니다~! | SCENE03A(8T6) · interpdata_0 (#857, 41.488초) · 트랙 sub_0 | 21.9106 / 3.9000 | NORMAL | Sequence P4 32777/4246 NORMAL | 일치 |
| `cin.37081_22_02` | 여러분, 혜성처럼 등장한 머저리들을 소개하겠습니다! | SCENE03A(8T6) · interpdata_0 (#857, 41.488초) · 트랙 sub_0 | 35.0194 / 5.3000 | NORMAL | Sequence P4 49692/7471 NORMAL | 일치 |
| `cin.37081_25_01` | 몸풀기는 여기까지 하고, 다음 장으로 넘어가 볼까? | SCENE04A(806) · interpdata_2 (#394, 27.000초) · 트랙 sub_0 | 3.0800 / 6.3000 | NORMAL | Action P73 3080/6300 NORMAL; Sequence P3 3080/6300 NORMAL | 일치 |
| `cin.37081_25_02` | 판이 깔렸으니, 신나게 놀아보자고! | SCENE04A(806) · interpdata_2 (#394, 27.000초) · 트랙 sub_0 | 20.0000 / 4.3000 | NORMAL | Action P73 20000/4300 NORMAL; Sequence P3 20000/4300 NORMAL | 일치 |
| `cin.37081_29_01` | 여러분들, 뿅망치 살인마가 등장합니다!↵과연 누가 살아남을까요? | SCENE02A(8M6) · interpdata_14 (#118, 9.436초) · 트랙 sub_0 | 0.8835 / 8.0000 | UPPER(트랙) | - | **붙일 컷신 없음** |
| `cin.37081_31_01` | 다들 준비됐어? | SCENE02A(8M6) · interpdata_10 (#117, 35.368초) · 트랙 sub_1 | 1.3431 / 1.7000 | NORMAL | Action P74 1343/1700 NORMAL; Sequence P5 1343/1700 NORMAL | 일치 |
| `cin.37081_31_02` | 오케이! 다음으로 넘어가자고! | SCENE02A(8M6) · interpdata_10 (#117, 35.368초) · 트랙 sub_1 | 3.5762 / 3.0000 | NORMAL | Action P74 3576/3000 NORMAL; Sequence P5 3576/3000 NORMAL | 일치 |
| `cin.37081_31_03` | 다음 이야긴 뭔데? | SCENE02A(8M6) · interpdata_10 (#117, 35.368초) · 트랙 sub_1 | 8.9415 / 1.5000 | NORMAL | Action P74 8942/1500 NORMAL; Sequence P5 8942/1500 NORMAL | 일치 |
| `cin.37081_31_04` | 장르를 바꾸는 거야.↵로맨스? | SCENE02A(8M6) · interpdata_10 (#117, 35.368초) · 트랙 sub_1 | 10.9911 / 3.0000 | NORMAL | Action P74 10991/3000 NORMAL; Sequence P5 10991/3000 NORMAL | 일치 |
| `cin.37081_31_05` | 아니. 잔혹한, 스릴러...! | SCENE02A(8M6) · interpdata_10 (#117, 35.368초) · 트랙 sub_1 | 14.5678 / 3.2000 | NORMAL | Action P74 14568/3200 NORMAL; Sequence P5 14568/3200 NORMAL; Sequence P7 0/1058 NORMAL | 일치 |
| `cin.37081_31_06` | 자, 이제 클라이막스를 시작해볼까?↵자, 이제 클라이막스를 시작해볼까? | SCENE02A(8M6) · interpdata_10 (#117, 35.368초) · 트랙 sub_1 | 26.4453 / 4.0000 | NORMAL | Action P74 26445/4000 NORMAL; Sequence P5 26445/4000 NORMAL; Sequence P7 9735/4000 NORMAL | 일치 |
| `cin.37081_32_01` | 렛츠, 쇼타임~! | SCENE02B(8MD) · interpdata_15 (#49, 길이 값 없음) · 트랙 sub_0 | 0.6890 / 2.3000 | UPPER(트랙) | Action P76 689/2300 UPPER | 일치 |
| `cin.37081_33_01` | 일어나, 발연기 그만하라고. | SCENE01B(8FD) · interpdata_0 (#45, 49.083초) / SCENE01C(8FK) · interpdata_0 (#44, 49.083초) · 트랙 sub_0 | 14.4667 / 3.1757 | NORMAL | Action P75 14467/3175 NORMAL; Sequence P9 14467/3175 NORMAL | 일치 |
| `cin.37081_33_02` | 좋아. 오늘은 여기까지! | SCENE01B(8FD) · interpdata_0 (#45, 49.083초) / SCENE01C(8FK) · interpdata_0 (#44, 49.083초) · 트랙 sub_0 | 19.6333 / 3.1824 | NORMAL | Action P75 19633/3183 NORMAL; Sequence P9 19633/3183 NORMAL | 일치 |
| `cin.37081_33_03` | 계속 분발해.↵자만하지 말고! | SCENE01B(8FD) · interpdata_0 (#45, 49.083초) / SCENE01C(8FK) · interpdata_0 (#44, 49.083초) · 트랙 sub_0 | 27.2667 / 2.9701 | NORMAL | Action P75 27267/2970 NORMAL; Sequence P9 27267/2970 NORMAL | 일치 |
| `cin.37081_33_04` | 우리가 널 응원하고 있으니까 말이야. | SCENE01B(8FD) · interpdata_0 (#45, 49.083초) / SCENE01C(8FK) · interpdata_0 (#44, 49.083초) · 트랙 sub_0 | 31.2083 / 5.2500 | NORMAL | Action P75 31208/5250 NORMAL; Sequence P9 31208/5250 NORMAL | 일치 |
| `cin.37081_33_05` | 그럼 혼돈이 가득 차오를 때, 다시 보자고! | SCENE01B(8FD) · interpdata_0 (#45, 49.083초) / SCENE01C(8FK) · interpdata_0 (#44, 49.083초) · 트랙 sub_0 | 38.3667 / 5.0563 | NORMAL | Action P75 38367/5056 NORMAL; Sequence P9 38367/5056 NORMAL | 일치 |
| `cin.37081_49_01` | 일어나, 발연기 그만하라고. | 원본 자막 트랙 없음 | - | - | - | 원본 미사용 |
| `cin.37081_49_02` | 좋아. 오늘은 여기까지! | 원본 자막 트랙 없음 | - | - | - | 원본 미사용 |
| `cin.37081_49_03` | 계속 분발해.↵자만하지 말고! | 원본 자막 트랙 없음 | - | - | - | 원본 미사용 |
| `cin.37081_49_04` | 우리가 널 응원하고 있으니까 말이야. | 원본 자막 트랙 없음 | - | - | - | 원본 미사용 |
| `cin.37081_49_05` | 그럼 혼돈이 가득 차오를 때, 다시 보자고! | 원본 자막 트랙 없음 | - | - | - | 원본 미사용 |

원문에 태그가 들어 있는 행(plain 변환 필요): `cin.37081_29_01`(`<br>`), `31_04`(`<br>` + `<FONT color='#6b6b6b'>`), `31_06`(`<br>` + `<FONT>`), `33_03`, `49_03`. 원문 그대로는 JSON 산출물의 `rawText`에 있다. 프로젝트의 붙은 문구는 이 변환을 이미 거친 값과 일치한다 **[실측]**.

## 3. 컷신별 대조 (원본 SCENE ↔ 프로젝트 패턴)

대응 근거: 프로젝트 도구의 카메라 접두사 표 + 길이 일치. 길이가 같다는 것은 근거를 보강하는 정황이다.

| 프로젝트 컷신 | Action | Sequence | 원본 SCENE·InterpData | 근거 | 자막 행 | 상태 |
|---|---|---|---|---|---|---|
| 2관문 진입 컷신 | P73 (27,000ms) | P3 (27,000ms) | SCENE04A(806) `interpdata_2` 27.0초 | 카메라 `kouku.gate2.intro.camera`, 길이 27.0초 일치 **[실측]** | `25_01`, `25_02` | 붙음·일치 |
| 2관문 클리어 (+3관문 진입) | P74 (35,368ms) | P5 (35,368ms), P7 3관문 진입 (18,658ms) | SCENE02A(8M6) `interpdata_10` 35.368초 | 카메라 `gate2.clear` / `gate3.intro`, P7 = 35.368 − 16.710 = 18.658 **[실측]** | `31_01`~`31_06` (P7은 `31_05`,`31_06`만 범위 안) | 붙음·일치 |
| 1관문 통합 시퀀스 | (Action에 없음) | P4 (62,919ms) | SCENE03A(8T6) `interpdata_0` 41.488초 | 카메라 `kouku.gate1.authored.*`, 구간 시계표 `LANDMARKS` **[실측]** | `22_01`, `22_02` | 붙음·일치 |
| 빙고 최종 엔딩 | P75 (51,285ms) | P9 (51,285ms) | SCENE01B(8FD) `interpdata_0` 49.083초 (SCENE01C(8FK)는 자막이 같은 변형) | 카메라 `kouku.bingo.ending.camera` **[실측]** | `33_01`~`33_05` | 붙음·일치 |
| 쇼타임 연출 | P76 (7,474ms) | (Sequence에 없음) | SCENE02B(8MD) `interpdata_15` | 카메라 `kouku.gate3.showtime.camera` **[실측]** | `32_01` (UPPER) | 붙음·일치 |
| **앵콜 컷신** | **P97 (23,333ms)** | **P10 (23,333ms)** | SCENE07A(8L6) `interpdata_23` 23.333초 | 카메라 `kouku.bingo.encore.camera`, 길이 23.333초 일치 **[실측]** | `12_01`~`12_03` | **미부착** |

### 3-1. 프로젝트 패턴별 결과 (자동 대조)

| 문서 | 패턴 | 이름 | 붙어야 하는 행 | 일치 | 미부착 | 어긋남 | 범위 밖(설계) |
|---|---|---|---|---|---|---|---|
| Action | PATTERN_73 | 2관문_진입컷씬 | 2 | 2 | 0 | 0 | 0 |
| Action | PATTERN_74 | 2관문클리어_3관문진입 | 6 | 6 | 0 | 0 | 0 |
| Action | PATTERN_75 | 빙고_최종엔딩씬 | 5 | 5 | 0 | 0 | 0 |
| Action | PATTERN_76 | 쇼타임_연출 | 1 | 1 | 0 | 0 | 0 |
| Action | PATTERN_97 | 앵콜컷신 | 3 | 0 | 3 | 0 | 0 |
| Sequence | PATTERN_3 | 2관문_진입컷씬 | 2 | 2 | 0 | 0 | 0 |
| Sequence | PATTERN_4 | 1관문_통합_시퀀스 | 2 | 2 | 0 | 0 | 0 |
| Sequence | PATTERN_5 | 2관문_클리어 | 6 | 6 | 0 | 0 | 0 |
| Sequence | PATTERN_7 | 3관문_진입 | 2 | 2 | 0 | 0 | 4 |
| Sequence | PATTERN_9 | 빙고_최종엔딩씬 | 5 | 5 | 0 | 0 | 0 |
| Sequence | PATTERN_10 | 앵콜컷신 | 3 | 0 | 3 | 0 | 0 |

Sequence P7의 "범위 밖" 4행(`31_01~31_04`)은 3관문 진입이 원본 16.710초부터 시작하기 때문에 그 앞의 자막이 잘려 나가는 정상 동작이다. `31_05`는 원본 14.568초 시작·3.2초 길이라 P7에서는 0ms 시작·1,058ms 길이로 잘려 붙어 있고 이것도 규칙과 일치한다 **[실측]**.

## 4. 원본에는 있는데 프로젝트에 안 붙은 자막 전체 목록

### 4-1. 앵콜컷신 3행 — 붙일 수 있는 상태

자막 리소스는 이미 있다(`subtitle.kouku.cin.37081_12_01~03`, UPPER, 길이 1,700 / 3,600 / 3,050ms, 문구 일치). 필요한 것은 **각 패턴에 배치(occurrence) 3개를 추가**하는 것뿐이다. 원본 값에서 계산한 붙일 값(앵콜 패턴 길이 23,333ms 안에 모두 들어온다):

| GameMsg id | 문구 | 시작 ms | 길이 ms | 위치 | 붙일 곳 |
|---|---|---|---|---|---|
| `cin.37081_12_01` | 누구 맘대로 끝을 내?! | 10233 | 1700 | UPPER | Action P97, Sequence P10 (둘 다 같은 값) |
| `cin.37081_12_02` | 무효야, 전부 무효! | 12367 | 3600 | UPPER | Action P97, Sequence P10 (둘 다 같은 값) |
| `cin.37081_12_03` | 진짜 시작은 지금부터라고! | 16700 | 3050 | UPPER | Action P97, Sequence P10 (둘 다 같은 값) |

- 원본 근거: SCENE07A 패키지(`B9AVB2VAZIQRPQCJVKAVYRAVOKYPY8L6.upk`) `efinterptracksubtitle_0`(export 15), 소속 `interpdata_23`. 트랙 속성 `positiontype = cinematic_subtitle_position_type_upper`이고 세 행 모두 `boverride_positiontype=false`라서 **UPPER**다. **[실측]**
- 팀원 도구가 앵콜에 못 붙인 이유: 그 도구는 카메라 접두사 `kouku.bingo.intro.camera.`가 있는 패턴에만 붙이는데, 우리 앵콜 카메라 접두사는 `kouku.bingo.encore.camera.`다. 도구의 `SCENES` 표에 `("kouku.bingo.encore.camera.", "SCENE07A", 15, "full")` 한 줄을 넣으면 같은 규칙으로 계산된다(내 대조에서 그렇게 넣어 위 값을 얻었다). **[실측]**
- 오디오와의 연결: `SOUND` 열이 25행 모두 비어 있어 GameMsg에서 음성 id를 따라갈 수 없다. 앵콜 첫 진입 대사 WAV(`Sound/KoukuSaton/Events/scene_midnightc_ed_koukustopclearingdungeon.source.wav`)가 팀원 문서 `2026-09-20_KOUKU_RESOURCE_AND_SUBTITLE_INVENTORY_RESULT.md`의 설치 목록에 적혀 있다 **[문서 기준, 이번에 재확인하지 않음]**.

### 4-2. 붙일 컷신이 프로젝트에 없는 행

| GameMsg id | 문구 | 원본 위치 | 이유 |
|---|---|---|---|
| `cin.37081_29_01` | %s | SCENE02A(8M6) `interpdata_14` 9.436초, 0.8835초부터 8.0초, **UPPER**(트랙) | 이 InterpData는 뿅망치 살인마 등장 컷신이다. 프로젝트에 대응 카메라 샷·패턴·월드 시퀀스가 없다(카메라 샷 접두사 목록과 두 Composition의 패턴 이름·길이로 확인). 새 컷신이 만들어지면 그 카메라 접두사를 `SCENES` 표에 넣고 붙이면 된다 **[실측]** |

### 4-3. 원본 어디에도 안 쓰이는 행

| GameMsg id | 문구 |
|---|---|
| `cin.37081_49_01` | 일어나, 발연기 그만하라고. |
| `cin.37081_49_02` | 좋아. 오늘은 여기까지! |
| `cin.37081_49_03` | 계속 분발해.↵자만하지 말고! |
| `cin.37081_49_04` | 우리가 널 응원하고 있으니까 말이야. |
| `cin.37081_49_05` | 그럼 혼돈이 가득 차오를 때, 다시 보자고! |

`49_01~05`는 `33_01~05`(빙고 엔딩)와 문구가 같다. 게임 전체 자막 트랙 스캔에서 참조하는 트랙이 없고, 쿠크 SCENE 패키지 11개(9개 + 레벨 2개)의 복호화 본문 바이트 검색과 복호화한 원본 표(.db) 전체 검색에서도 `EFTable_GameMsg.db` 자신 외에는 나오지 않았다 **[실측]**. 다른 종류의 원본(레벨 스크립트 등 대형 패키지)에서 쓰일 가능성은 배제하지 못했다 **[미확인]**.

### 4-4. 문서마다 컷신 자체가 없는 경우 (정상)

- Action에는 `22_01`, `22_02`가 없다: Action 문서에는 1관문 통합 시퀀스에 해당하는 패턴이 없다.
- Sequence에는 `32_01`이 없다: Sequence 문서에는 쇼타임 연출 패턴이 없다.

## 5. 시각·문구가 어긋난 행

**없다.** Action 14행, Sequence 17행(합 31행)을 원본에서 다시 계산한 값과 비교했고 시작 ms, 길이 ms, 위치, 문구가 모두 같다 **[실측]**. 1관문 P4의 값(`22_01`이 원본 21.911초인데 프로젝트 32,777ms 등)이 원본과 크게 달라 보이는 것은 오류가 아니라 프로젝트의 구간 시계표를 거친 정상 값이다(원본 21.9106초 → 32,777ms, 35.0194초 → 49,692ms를 손으로 계산해 일치 확인).

## 6. 원본에 자막 트랙이 없는 컷신 ("자막 없음"이 정답)

| 프로젝트 컷신 | 원본 | 근거 |
|---|---|---|
| 2관문 카드미로 (Action P77, Sequence P6) | SCENE04A(806) `interpdata_1` 11.95초 | 이 패키지의 자막 트랙은 `interpdata_2`(27초, 2관문 진입) 하나뿐이다 **[실측]** |
| (프로젝트 컷신 없음) | SCENE01A(8F6, InterpData 54개), SCENE06A(8E6, 5개) | 패키지 이름 표에 자막 클래스가 아예 없다 **[실측]** |
| SCENE03A의 다른 InterpData 5개(interpdata_2,3,4,6,7) | 8T6 | 자막 트랙은 `interpdata_0`에만 있다 **[실측]** |
| 연출_팝업북(Sequence P1), 연출_1관문 피날레(P2) | (SCENE03A 일부로 추정) | 둘 다 DRAFT 조각이고 실제 1관문 입장 연출은 통합본 P4이다(`2026-09-20_KOUKU_GATE_ARCHITECTURE_MAP_RESULT.md`). 자막은 P4에 붙어 있다. P1/P2 카메라(`2Stage.book`, `1Stage.finale`)가 원본 SCENE03A 어느 구간인지는 확인하지 않았다 **[미확인]** |

## 7. 프로젝트에 붙이는 방법(저장 경로)

- 쿠크 컷신 자막은 Composition의 `presentationResources`(`kind=SUBTITLE`, `assetId`=GameMsg id, `subtitleText`, `subtitlePosition`)와 패턴의 `presentationOccurrences`(`resourceId`, `startMs`, `durationMs`, `anchorKind=MAP`)로 저장한다. 앵콜은 리소스가 이미 있어 occurrence 3개만 더하면 된다 **[실측]**.
- 쿠크 월드 시퀀스(`LV_LUT_MIDNIGHTC_ED.worldsequences.json`)에는 자막 저장 경로(`subtitleTracks`)가 없다(0회). 발탄 월드 시퀀스에는 그 문자열이 5회 있으나 이번 범위 밖이다 **[실측]**. 쿠크 컷신을 월드 시퀀스로 재생하더라도 자막은 위 Composition 경로에 둔다 **[추론]**.
- 계약: `subtitleText`는 1~4096바이트 유효 UTF-8, LF만 허용, markup 금지(팀원 문서 `2026-09-20_KOUKU_SCENE_AUDIO_SUBTITLE_RESULT.md`).

## 8. 팀원 문서의 주장 검증

| 주장(`KOUKU_SCENE_AUDIO_SUBTITLE_RESULT`) | 판정 |
|---|---|
| 카드미로 SCENE04A InterpData393에는 원본 자막 트랙이 없다 | **참**. 다만 export 번호가 문서는 1부터, 내 추출은 0부터 세서 393이 내 번호 392(`interpdata_1`, 11.95초)다. 같은 패키지의 `interpdata_2`(내 번호 393)는 27초 2관문 진입이고 자막이 있다 **[실측]** |
| SCENE01A와 SCENE06A에도 없다 | **참** **[실측]** |
| SCENE01C는 ending의 별도 variant | **참**: SCENE01B와 자막 5행의 시각·길이가 같다 **[실측]** |
| SCENE07A Bingo 3행, SCENE02A hammer 1행은 미연결 | 당시에는 참. 지금 main에는 앵콜 패턴이 있으므로 **07A 3행은 붙일 곳이 생겼고**, 02A hammer 1행은 여전히 대응 컷신이 없다 **[실측]** |

## 9. 확인하지 못한 것과 한계

- **[미확인]** `cin.37081_49_01~05`가 자막 트랙 밖의 다른 방식(레벨 스크립트 등 대형 패키지)에서 쓰이는지. 자막 트랙 스캔(전체 패키지)과 SCENE 패키지 11개 본문 검색, 원본 표 검색에서는 안 나왔다.
- **[미확인]** 연출_팝업북(P1)/1관문 피날레(P2) 카메라가 원본 SCENE03A 어느 구간인지.
- **[미확인]** 화면에서 자막이 어떻게 보이는지(위/아래 위치, 겹침, 폰트). Client는 실행하지 않았다.
- **[추론]** 이름에 `midnightc_ed`가 들어 있지만 자막 문구가 다른 존 id(`cin.11101_15_01/02` "녀석이 무섭게 성장하고 있잖아." / "아직 시간이 좀 더 필요한데...")인 패키지 1개(`756R9UD6VD2LM2R6VR6RKGULU439.upk`)는 쿠크 레이드 맵 컷신이 아니라고 판단해 제외했다(사운드 이벤트 이름에만 쿠크 이름이 들어 있음).
- 다른 존에도 쿠크세이튼이 언급되는 GameMsg가 있으나(`cin.10001_*`, `cin.37541_*` 등) 쿠크 레이드 존(37081)이 아니라서 범위 밖으로 뒀다.

## 10. 재실행과 산출물

- 원본 추출 스크립트: `C:\LostArkExtract\KoukuSubtitle_20260920\scripts\` (`scan_all_packages.py`, `scan_all_packages_v2.py`, `scan_midnightc.py`, `tracks_full.py`, `gamemsg_cin.py`, `build_inventory.py`, `write_report.py` 등). 추출 자료: 같은 위치의 `data\`.
- 재실행 순서: `scan_all_packages_v2.py 22`(약 3분) → `gamemsg_cin.py` → `tracks_full.py` → `build_inventory.py` → `write_report.py`.
- 이 조사는 UPK 파서(`Tools/LevelPlacementExtractor/extract_ue3_placements.py`)와 게임 설치본을 읽기만 했다.
