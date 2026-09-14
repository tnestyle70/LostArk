# 쿠크 2관문 책·도박판·카드 분출 재조사 결과

2026-09-14. 사용자 요청: 관문 사이 책 에셋 공유 여부, 2관문 펼침 애니메이션과 이전 카드 분출의 느린 속도를 다시 확인한다.

## G00. 조사 결론과 범위

**1관문과 2관문은 원본 책 모델과 AnimSet을 공유한다. 2관문의 도박판 테이블은 별도 모델이다.** 같은 책 모델을 사용한다는 사실이 관문 전체 공간이나 실행 객체 하나를 공유한다는 뜻은 아니다. 2관문은 Book, HandBook, Table을 각각 배치하고 별도 애니메이션을 평가한다.

현재 2관문 책·테이블에는 애니메이션이 이미 설치·연결돼 있다. 따라서 새로 펼침 clip 하나를 붙이는 것만으로 남은 연출 차이가 해결되는 상태는 아니다. 카드의 과거 시간 단위 오류도 현재 설치본에는 교정돼 있다. 남은 확인 대상은 주변 세트의 부모 변환, 카드의 원본 FX 재질·CameraOffset 및 원본 시뮬레이션과의 차이다.

이 문서는 조사 결과다. 제품 C++·저작 JSON·Resources 수정, 생성기 실행, publish, 빌드, Client/UI 실행·조작·캡처는 하지 않았다. 기존 사용자 편집과 소품 원복 상태를 보존했다. 변경 파일은 이 RESULT 하나다.

## G01. 책은 어떤 것을 공유하는가

원본 import identity를 직접 대조했다.

| 대상 | 원본 연결 | 실제 원본 mesh |
|---|---|---|
| 1관문 팝업북 | SCENE03A group1042 → actor372 → component2290 → import -203 | `cine_prob_09_s2_2012.mesh.bg_rad_koukusaton_book` |
| 2관문 Book | SCENE04A group499 → actor1463 → component1479 → import -191 | 같은 책 |
| 2관문 HandBook | SCENE04A group517 → actor1471 → component1487 → import -191 | 같은 책 |
| 2관문 Table | SCENE04A group500 → actor1470 | `cine_prob_09_s2_2012.mesh.bg_rad_koukusaton_table` |

책의 AnimSet도 `cine_prob_09_s2_2012.ani.bg_rad_koukusaton_book_evt2_ani`로 같다. 원본 책은 13본이며 `evt2_book01`, `evt2_book02`가 각각 71프레임/30fps다. 테이블은 9본이며 `evt2_table_open01`은 47프레임/30fps다. 마지막 표본까지의 시간은 각각 70/30초, 46/30초이며 전체 컷신 길이와 구별한다.

설치 파일은 관문별로 다르게 가공됐다.

- 1관문: `Map/LV_LUT_MIDNIGHTC_ED/AnimatedProps/DEPLOY_CINE_KOUKU_BOOK/DEPLOY_CINE_KOUKU_BOOK.wmodel`.
- 2관문: `Map/KakulSaydon/Gate2Intro/Book/Book.wmodel`, `HandBook/HandBook.wmodel`, `Table/Table.wmodel`.
- 1관문 설치 책은 skeleton node16, 2관문 설치 책은 node14다. 원본 PSK 본 수와 변환 과정의 추가 노드를 구별한다. 두 관문 WModel의 mesh/skeleton section이 byte-identical인 것은 아니다.
- 1관문 설치 책과 `SourceSequences/kouku.gate1.full/Book/Book.wmodel`은 mesh/material/skeleton section이 동일하다. 2관문 Book과 HandBook도 이 세 section은 동일하지만 animation은 각각 별도다.

따라서 한 관문의 clip을 다른 관문의 설치 WModel에 이름만 맞춰 복사하는 방식은 적절하지 않다. 현재 변환 경로처럼 해당 skeleton에 맞는 포즈를 생성해야 한다.

현재 P1/P4의 팝업북은 같은 `world.object.kouku.popup.book`을 참조한다. P3의 Book/HandBook는 서로 다른 objectId·instance·template을 사용한다. `CWorldSequenceObject::Initialize`는 prototype에서 CModel을 Clone하고 각 객체가 본·애니메이션 상태를 소유한다. 원본 모델 재사용과 실행 포즈 공유를 구별해야 한다.

현재 P7 `3관문_진입`에는 책 WORLD 참조가 없다. 이것만으로 3관문 정적 맵 전체의 기하를 동일 책의 마지막 포즈라고 판정할 수 없다. `bg_rad_koukusaton_paperstage`도 별도 종이무대/다리 모델이다. 확인된 공유 범위는 위의 원본 책 mesh/AnimSet이다.

근거: `out/KoukuSourceSequenceRestore20260912/LV_LUT_MIDNIGHTC_ED_SCENE03A.json:162807,172868`, `out/KoukuGate2Restore20260911/source.json:73715,73911,77791,77792`의 원본 actor/component/import, `Tools/KoukuSaydonPipeline/build_gate2_intro_composition.py`의 actor 정의와 bake, 설치 WModel section·animation 직접 열람, 현재 World/Sequence JSON. 런타임 소비는 `Client/Private/WorldSequenceObject.cpp:22`, `WorldSequencePlayer_Objects.cpp:263`.

## G02. 2관문 펼침 애니메이션의 현재 연결

조사 시 Sequence Composition revision58의 P3 `KAKULSAYDON_G1_PATTERN_3`은 27,000ms다. WORLD 17개는 배우2, Book/HandBook/Table3, 의자4, 촛대2, 카드 발생점6이다. 17개 바인딩의 instance/template/resource를 모두 resolve했고 참조 모델 파일도 존재했다.

Book, HandBook, Table은 각각 `gate2_intro_27s`를 사용한다. 세 설치 모델은 810ticks/30Hz=27초다. template/occurrence는 27초, track 시작0, sourceStart 생략(기본0), animation playbackRate1, instance/occurrence playbackSpeed1이다. 비루프이며 holdLastFrame=true이고 instance motionEnd=STOP이다. 1관문 팝업북의 instance 0.7배속을 2관문에 적용하지 않는다.

원본 SCENE04A의 주요 시각은 다음과 같다.

| 시각 | 원본 동작 |
|---:|---|
| 4.11초 | HandBook의 `evt2_book02` 정재생 |
| 9.04초 | HandBook의 `evt2_book01` 역재생 |
| 11.00초 | Table의 `evt2_table_open01` 역재생 |
| 15.19초 | 큰 Book의 `evt2_book01` 정재생 |
| 15.66초 | Table의 `evt2_table_open01` 정재생 |
| 16.29초 | 테이블 구간의 검정 fade가 걷힘. 이미 펼침이 0.63초 진행됨 |
| 21.04~26.96초 | 카드 분출 occurrence 표시 |

설치 clip의 811개 표본도 직접 읽었다. Table은 회전 변화4본/최대90도(역재생·펼침 전체11.233~16.967초), 큰 Book은4본/최대90.598도(15.200~17.167초), HandBook은6본/최대90도(4.133~11.367초)다. 빈 clip이나 전체 정지 clip이 아니다. 원본 키 시각과 30Hz 표본에서 처음 관찰되는 변화 시각은 구별한다.

생성기는 원본 A/B 애니메이션 weight, 역재생, preroll, 일부 SkelControl을 30Hz 포즈로 구워 27초 clip에 넣는다. 이 clip은 단순한 원본 `table_open01` 하나와 다르다. 런타임은 World 시각을 `Try_SampleAnimationTicks`로 ticks로 변환한 뒤 `Set_AnimTrackPosition`과 `Play_Animation(0)`으로 포즈를 평가한다. 현재 코드에는 연결된 animation을 무시하고 항상 정지 포즈로 두는 경로가 없다. 실제 화면에서의 표시 성공을 이 정적 검토만으로 판정하지는 않았다.

World authoring/runtime 문서는 revision1845이며 SHA256이 같았다. Sequence workspace는 `Resolve_SequencePath()`로 ProjectDataRoot의 `Compositions/Sequences/KoukuSaydonSequenceComposition.json`을 직접 읽는다. `Publish_AllPatterns`는 이 workspace에서 local preview/Save만 지원한다고 거절한다. 별도의 `KoukuSaydonArena.sequencer.json` SHADOW projection을 P3의 게시 사본으로 비교하면 안 된다.

근거: `build_gate2_intro_composition.py:324,377,452,719`, `WorldSequenceDocument.cpp:1816`, `WorldSequenceObject.cpp:45`, `KoukuSaydonCompositionDocument.cpp:2152`, `KoukuSaydonActionWorkbench.cpp:1422`. 원본 시각은 09-10 관문 시작 PLAN과 실제 SCENE04A 추출값을 대조했다.

## G03. 카드 분출이 느렸던 원인과 현재 수치

과거 파일은 14,000ticks/1,000Hz였지만 Engine의 cooked animation은 30Hz를 사용한다. 그 결과 14초 연출이 466.667초, 약33.33배 느리게 재생됐다. 이 원인은 09-12에 교정됐다.

현재 설치 `Map/KakulSaydon/Gate2Intro/CardEruption/CardEruption.wmodel`을 직접 파싱한 결과:

- animation `card_eruption`, channel108, duration420ticks, ticksPerSecond30: **14초**.
- channel별 position/rotation/scale 421키, 키 시각0~420. header만 바꾼 상태가 아니다.
- SHA256 `3865b1110911be80fd7de96b97ddb69095923ec4905194f65b06a8af69ee0bae`. 09-12 교정 설치 기록과 동일하다.
- 독립 재생과 P3의 6개 발생점 모두 animation/instance/occurrence 배율1이다.
- builder는 현재도 cook 뒤 `retime_wmodel_ticks.py --ticks-per-second 30`으로 전체 key time을 변환한다.

원본 시스템은 `fx_q_w_01.fx_par_02.par_q_cardfly_01`이다. 6개 발생점 각각 3개 mesh emitter를 사용하며 각 emitter가 초당6장, 카드 수명8초다. 현재 bake는 방출6초+tail8초로 14초이며 발생점당108장을 담는다. 현재 6개 occurrence는 원본 Matinee Toggle의 21.039999초 trigger/26.959999초 OFF와 같은 **21,040~26,960ms**를 사용한다. 카드 발생은 이 Toggle 경로이며 보스 animnotify에 새 발사를 붙일 필요가 없다.

컷신은 14초 bake의 첫5.92초를 표시한다. 원본 방출 속도를 유지하려면 전체14초를5.92초로 압축하지 않는다.

원본 초기 속도는 UE Z축으로 1,500~2,500cm/s, 즉 상향15~25m/s다. 설치 모델의 첫 카드 position은 0→1초에 `(202.53259,1887.12415,87.80016)cm` 이동한다. 현재 modelPreScale0.01 반영 후 이동거리 **18.999910m**다. 3개 emitter의 첫 카드 1초 이동거리는 각각18.99991/20.71077/14.67552m다. 이는 설치 바이너리 키와 배율 계산이며 실제 Client/GPU 재생 검증은 아니다.

현재 디스크 입력에는 과거33배 슬로우가 남아 있지 않다. 실제로 다시 느리게 보인다면 실행본 Resources 경로, 메모리에 남은 이전 모델, 전체 preview 배율을 실제 재생 상태에서 먼저 대조해야 한다. 현재 실행 프로세스의 메모리나 UI 상태는 이번 조사에서 확인하지 않았다.

근거: `Engine/Private/Animation.cpp:52`, `Engine/Public/Animation.h:24`, `build_gate2_card_eruption.py:54,61,155`, `out/KoukuGate2Restore20260911/cardfly.json`, `source.json:60314`, 09-12 `KOUKU_SOURCE_SEQUENCE_RESTORE_RESULT.md:155,273`.

## G04. 여전히 원본과 다른 부분

카드의 시간 교정과 원본 표현의 완전한 복원은 구별한다.

| 항목 | 현재 상태 |
|---|---|
| 카드 재질 | BG map material3slot을 사용. 원본 FX MIC `fx_q_me_card_01_tr`, `fx_q_me_card_01_01_tr`, `fx_q_me_card_02_tr`의 계산은 이 bake에 미반영 |
| CameraOffset | 원본50cm 모듈이 mesh bake에 미반영 |
| 난수·적분 | 저작 seed4210401, 30Hz 오프라인 적분/회전. 원본 실행 난수와 약1/60초 update의 완전한 동일성을 증명한 상태는 아님 |
| 중복 방출 | native sequence effect에서는 기존 CModel이 소유하는 6×3=18 mesh 요소를 의도적으로 제외. 추후 carrier를 복원할 때 중복 표시 방지 필요 |
| 주변 촬영 세트 | 추가 소품165개/16WORLD는 현재 P3에 없음. 09-13 사용자 화면에서 붉은 수직 띠·공중 카드 오배치를 확인해 되돌린 상태 |

소품165개를 다시 붙이기 전에 원본 부모/상대 변환, base rotation 처리와 실제 받침 구조를 재검증해야 한다. 위치 계산이 finite라는 사실은 원본 배치가 맞다는 증거가 아니다. 카드 흔들림의 zero-phase sine도 원본 위상을 확인한 값이 아니다. 기존 RESULT에서 의심한 변환 원인을 이번 조사에서 새로 확정하지는 않았다.

현재 `build_gate2_intro_composition.py:754`는 `build_backdrops`를 무조건 호출하고, 이후 P3의 occurrence 목록을 다시 구성한다. `--install`을 단순 재실행하면 사용자가 되돌린165개 소품이 다시 들어올 수 있다. 또한 install=false여도 actor bake는 Resources 파일을 생성하므로 읽기 전용 검사 명령으로 쓰지 않았다. 후속 수정은 최신 사용자 정본에서 해당 범위만 교정해야 한다.

근거: `build_gate2_card_eruption.py:165`, `build_source_sequence_effects.py:137`, 09-13 `KOUKU_G12_CUTSCENES_CAMERA_MAP_RESULT.md`의 G13-R6, `build_gate2_intro_backdrops.py:140`.

## G05. 검증과 남은 경계

- LAN sync 성공: server-host, TCP7777 LocalSubnet ready, endpoint192.168.0.14:7777 not-listening. 설정 완료이며 Server는 시작하지 않았다.
- `git status --short`, 현재 branch `codex/sequence-capture-focus`, `git fetch` 확인. 기존 대규모 미커밋 변경을 보존했다.
- 현재 Sequence/World JSON parse 및 참조 연결, World authoring/runtime SHA 동일성, 설치 WModel animation/section 직접 열람, 카드 key 수·시간·변위 계산을 수행했다.
- 기존 PLAN/RESULT와 현재 코드·원본 추출값을 대조했다. 오래된 문서의 미설치 table 주장, 소품165개 설치 완료 주장, 예전 카드 속도를 현재 상태로 사용하지 않았다.
- 제품 입력 변경이 없으므로 컴파일·publish는 실행하지 않았다. 이 RESULT의 UTF-8·줄 끝 공백 및 한정 `git diff --check`를 확인했다.
- 실제 UI 재생, 카드 화면 속도, 책/무대의 최종 시각 일치, 원본 게임 전체의 동일성은 판정하지 않았다. 3관문 정적 맵 전체를 책의 최종 포즈와 정점 단위로 대조한 조사도 아니다.

사용자 확인 위치는 기존 F1의 시퀀서에서 P3 `2관문_진입컷씬`이다. 전체 재생 중15.66~17.3초의 도박판 펼침과21.04~26.96초의 카드 분출을 구분해 관찰한다. 이번 조사로 실행 파일이나 리소스를 새로 교체하지 않았다.
