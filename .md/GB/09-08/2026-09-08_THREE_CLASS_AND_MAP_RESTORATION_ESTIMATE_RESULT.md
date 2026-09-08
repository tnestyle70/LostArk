# 도화가·창술사·워로드와 맵 복원 전수조사 결과

## 현재 상태

2026-09-08 21시 전후 KST에 현재 데이터와 소비 코드를 읽고 기존 `렌더링 색감 및 맵 점검`
작업의 대화·결과 기록을 대조했다. 작업 조회 도구가 최근 turn의 본문을 비워 반환하여,
해당 작업 ID의 로컬 session 기록에서 사용자 메시지·완료 답변·최신 commentary를 읽었다.
과거 세션의 지시는 이번 제품 수정 권한으로 사용하지 않았다.

이 작업은 조사와 견적 문서만 작성했다. 세 직업·맵의 제품 구현, 빌드, publisher,
Client/UI 실행·캡처·새 visual PASS는 수행하지 않았다. 견적은
[범위·견적 PLAN](2026-09-08_THREE_CLASS_AND_MAP_RESTORATION_ESTIMATE_PLAN.md)에 있다.

브랜치 `codex/kouku-ball-motion-effects`, HEAD `f92178f054a759c128a61cebd8e2ff4b8712eb12`.
fetch 후 HEAD...origin/main은0/0이다. 다른 작업의 dirty 변경을 포함한 조사이며 commit 상태와 다르다.
LAN sync는 server-host, TCP7777 LocalSubnet 방화벽 준비,192.168.0.14:7777 not-listening을 반환했다.

## 기존 세션에서 이어받을 핵심

| 범위 | 확인된 진척 | 현재에도 구분할 경계 |
|---|---|---|
| 색감·바닥 | 실제 placement의 MIC override가 모델 기본 재질과 달랐음. 선택 표면에 원본 normal/specular/PBR/RNM/환경 식 연결 | 조명 slider만으로 빠진 표면식을 복원하지 못함. 전체 표면 복원 완료는 아님 |
| CS 중앙 링 | 사용자가 금색 변화 확인 | 전체 중앙 원판·십자 bridge 표시 조합의 원작 동일성은 아직 미확정 |
| 차원술사 Q | Q2 비교본 보존, Q전체10요소, CubeSample/검격/Glow 프로그램 연결, 사용자가 유리 확인 | 전체 Play All·위치·분포의 최근 교정과 전체 원작 유사도는 별도 확인 |
| Q 반복 결함 | cm→m, source basis, Dynamic 기본값, CDO/상속 분포와 시계/중복 준비 교정 | 공통 importer 상속 병합은 아직 자동화되지 않음 |
| 캐릭터 축소 품질 | 실제45모델의 TGA247개 CPU mip, 차원술사 DDS12개 하위 mip, Aniso16 | 피부/눈/머리/무기 원본식 전체 완료와 다름 |
| 캐릭터 source 재질 | 검토 도중 실제 CharacterCatalog에 창술사5/차원술사17 override 추가됨 | 누적 노트의 override0/보류 문장은 이 관찰보다 이전. 새 EXE·GPU·화면 결과는 이번 조사로 확인하지 않음 |
| 차원술사 다음 작업 | 기존 작업에서 전체 스킬·Alt+V Camera row·BA0~3 분리 요청 진행 중 | BA4단 방향은 최신 요청이며 구현 완료로 기록하지 않음. 이 세션과 중복 구현하지 않음 |

공통 원리와 연결 범위는 [복원 노트](../렌더링이펙트복원V2.md), 실제 Q 후속 결과는
[기존 RESULT의 후속 절](2026-09-08_CHARACTER_MATERIAL_AND_EFFECT_ROUND2_ESTIMATE_RESULT.md:260)을 따른다.

## 현재 세 직업 전체 집계

| 직업 | 스킬 정의 | catalog 등록 문서 | element | visible | sourceRecipe enabled | 명시 execution enabled | modelCue | nonempty parent 종류 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| 도화가 | 16 | 19 | 283 | 270 | 272 | 32 | 1 | 47 |
| 창술사 | 24 | 43 | 384 | 384 | 384 | 10 | 0 | 53 |
| 워로드 | 17 | 21 | 354 | 354 | 341 | 0 | 0 | 61 |
| 합계 | 57 | 83 | 1,021 | 1,008 | 997 | 42 | 1 | 직업 간 중복 있음 |

root가83문서를 JSON parse하고 ID별 catalog membership, resource path의 실물 존재를 다시 확인했다.
문서 내 모든 명시 Resources-relative 문자열을 재귀 집계하면 도화가157/창술사177/워로드237경로이며
누락0이다. 이 수는 `elements[].resources`만 센 수와 다르다. sourceProfile/실행 parameter/modelCue
등의 명시 경로도 포함한다. DDS 내용·선택 shader의 필수 texture 전체 충족·실제 draw를 증명하지 않는다.

`visible=true`도 실행 보장이 아니다. `execution.failClosed && !authoringApproximate`이면 renderer가
실제 occurrence를 억제한다. 저장 profile이 grouped여도 strict identity selector가 구체 식을 선택할 수 있다.
따라서 위 열에서 복원 완료율을 계산하지 않는다. parent 종류 수도 그대로 신규 shader 수가 아니다.

현재 집계 원본: [current_inventory.json](C:/Users/user/Desktop/LostArk/out/RestorationEstimate20260908/current_inventory.json).

## 어려운 스킬의 실제 경계

| 대상 | 확인된 코드·데이터 | 복원에 중요한 의미 |
|---|---|---|
| 도화가D | 68행 중 BLACK_TIGER_STROKE12행 명시 실행 | 호랑이 전체 재질·배치 완료로 확대하지 않음 |
| 도화가T | 23행 중 hidden12, visible helix3을 포함해15행이 실행 억제 조건 | 보이게 체크만 해도 실행되지 않는 명확한 복원 공백 |
| 도화가V | 46행, PROJECT_TUNED targetAttractor | 원본 운동과 손보정 집결을 구분해 비교 |
| 도화가Alt+V | 2문서97행, particle만 있음 | 별도 모델·화면 합성이 원본에 필요한지 구성 대조 |
| 창술사F/V | Flamesurface 용 mesh, 현재 Effect classifier에 명시 family 소비 미확인 | 원본 용 표면과 입력 식 우선 조사 |
| 창술사Alt+V | 210행/41parent, dragon opcode19는10행 | 기존 용식 재사용 뒤 나머지 전체 구성 |
| 워로드Z | 자세 동작은 있으나 원본 FilmNoise/ApShield cue에 effectref=asset 없음 | 시작·지속·해제 표현의 제품 연결 필요 |
| 워로드F | 현재4행; opcode22 코드는 남았지만 현재 F가 선택하지 않음 | 과거 canary 완료 기록으로 현행 F복원 완료라 할 수 없음 |
| 워로드Alt+V | 262행/50parent, RealPBR weapon/rock/worldoffset, 원본 FilmNoise/ZoomBlur | geometry·정점변형·화면 입력을 함께 다룰 큰 구성 |

도화가D/T/V/Alt+V에는 각각1/2/2/1개의 SHAKE cue가 있고 현재
`CCharacter::Update_CameraShakeCues`가 로컬 action timeline으로 소비한다.
Effect 문서에 screenPost가 없다는 것과 카메라가 전혀 없다는 것은 다르다.

근거:

- [실제 suppression](C:/Users/user/Desktop/LostArk/Client/Private/Effect_DocumentRenderer.cpp:5776)
- [strict family 선택](C:/Users/user/Desktop/LostArk/Client/Public/Effect_MaterialTemplate.h:345)
- [현재 미르 새김](C:/Users/user/Desktop/LostArk/Data/Effects/Authored/effect.artist.skill.31950.unified.effect.json:9050)
- [카메라 shake 소비](C:/Users/user/Desktop/LostArk/Client/Private/Character.cpp:692)
- [원본 EFFECT와 제품 cue 구분](C:/Users/user/Desktop/LostArk/Client/Private/AnimationEffectCueDocument.cpp:825)
- [워로드 Z 원본 cue](C:/Users/user/Desktop/LostArk/Data/Animation/Authored/Warlord/Warlord.animevents:410)
- [워로드 자세 표현](C:/Users/user/Desktop/LostArk/Client/Private/Logic_Warlord.cpp:61)
- [현재 importer SourceIndex](C:/Users/user/Desktop/LostArk/Tools/LevelPlacementExtractor/build_imported_effect_documents.py:176)
- [분포 op/default 처리](C:/Users/user/Desktop/LostArk/Tools/LevelPlacementExtractor/build_imported_effect_documents.py:1238)

도화가 grouped186행의 identity selector 이름·parent/profile 일치 후보는148행/18종이며
미일치38행은 D8/T3/V3/Alt+V24에 모인다. 이는 실제 texture packet/GPU 승인 수가 아니다.
sourceObjectPath는 있으나 assetId가 빈 texture lane204개도 비활성/절차식일 수 있으므로
그대로 파일 누락204개라고 세지 않았다. SourceRecipe의38종 module class에는 대응 코드 문자열이
있지만 Orbit/InheritParent/default/options까지 원본 동일하게 실행한다는 증거는 아니다.

창술사와 워로드의 parent family 합집합은78종, 교집합36종이다. 차원술사와 겹치는 parent도
각23/26종이다. 이 수는 재사용 후보를 찾는 출발점이며 exact shader/입력 확인을 대체하지 않는다.

## 네 맵의 현재 입력

| 맵 | catalog행 | 고유asset | 배치 | 선택 mapmaterials | placementLighting | 제품 scope |
|---|---:|---:|---:|---:|---:|---|
| Character Select | 63 | 63 | 803 | 9 | 29 | X[-792,-750],Z[158,218]+배경 |
| Valtan | 272 | 272 | 13,184 | 미선언 | 없음 | full |
| Kouku | 323 | 323 | 3,231 | 5 | 없음 | full |
| Bern | 3,021/13shard | 1,003 | 50,017 | 미선언 | 없음 | full |

root가 현재 catalog header/asset ID와 authoring placement header를 다시 집계했다.
CS source mesh55개와 catalog63개는 variant 때문에 분모가 다르다.
`mapmaterials` 미선언은 화면에 재질 자체가 없다는 뜻이 아니라 이번 source override 레이어가
연결되지 않았다는 뜻이다. 기존 WModel의 기본 재질·level 조명은 별도로 존재한다.

Kouku5행은 source 표면2행+diffuse Mirror sampler3행이다. maplight는 CS2/Kouku4/Valtan22,
Bern은 미선언이다. Valtan의 PointLight는 눈에 보이는 불꽃/sprite를 대신하지 않는다.
CS와 Kouku material의 authoring/runtime JSON 의미 일치는 확인했다.

근거: [maps_inventory.json](C:/Users/user/Desktop/LostArk/out/RestorationEstimate20260908/maps_inventory.json),
[LevelRegistry](C:/Users/user/Desktop/LostArk/Client/Private/LevelRegistry.cpp:121),
[CS material](C:/Users/user/Desktop/LostArk/Data/Maps/Authoring/LV_LOBBY_CLASSSELECT_SL00/LV_LOBBY_CLASSSELECT_SL00.mapmaterials.json),
[Bern mapset](C:/Users/user/Desktop/LostArk/Client/Bin/DataFiles/Map/LV_BER_BERNCASTLE.mapset:1).

CS 전체 원본 조사에는75재질/14family/179입력 조합/146program 후보가 있다.
현재 원본 재질6종이9개 material row로 연결됐고, 같은 baked/base/vertex/light program 조합의
추가4종27배치가 재사용 유력 후보다. 75개를 전부 실제 GPU에 연결했다는 결과는 아니다.
folio/grass/masked/trans/snowice/sky 등 나머지 family와 중앙 표시 조합을 확인해야 한다.

기존 큰 맵 binary census의 실제 used material row는 Bern1,453/Valtan347/Kouku399다.
이 값은 원본 shader 전수 대응 수가 아니다. 큰 세 맵에는 CS 수준의 native family 대응표가
아직 없다는 점을 견적에 반영했다.

## 전체맵·조작감·성능의 구분

| 맵 | 현재 navigation 기본 grid | 해석 |
|---|---|---|
| CS | 62×62,0.5m | 선택 아레나의 보행 범위 |
| Bern | 50×347,0.5m | 50,017개 visual 배치 전역 보행을 뜻하지 않음 |
| Valtan | 392×312,0.5m | 시각 전체 로드와 전투 보행 조건을 별도 유지 |
| Kouku | 524×800,4m + Mario 세부 region | 관문별 보행/세부 영역과 입력 모드 확인 필요 |

네 `Data/Navigation/<Area>.navsource` header를 직접 확인했다.
전체 탐색 가능 맵이 목표라면 visual 확장뿐 아니라 Server navgrid/collision/trigger를 확대해야 한다.
클라이언트 Transform을 직접 움직여 보행 완성으로 처리하지 않는다.

입력→서버 action→애니메이션/이펙트/소리→hit→종료/재입력의 시점을 같은 조건으로 측정해야
조작감 차이의 주원인을 정할 수 있다. 이번 조사에서 입력 지연, GPU frame time, 4인 동시
Alt+V 부하, 원작과의 FPS 비교는 측정하지 않았다. Renderer/시계/입력 경로의 존재와 성능 완성은 다르다.

## 오래된 기록을 현재 상태로 읽으면 안 되는 부분

- 8월22일 창술사D/F의88/186행, V134행 candidate와 워로드F56행은 현재2/3/9/4행과 다르다.
  손편집·직접저작 정본화 이후의 현재 문서가 우선이다. 과거 원본 참조는 조사 자료로만 사용한다.
- 현재 Effect 입력은 DIRECT_AUTHORED_DOCUMENT다. 옛 `Client/Bin/DataFiles/Effect`와
  sealed publish 기록을 지금의 설치/재생 완료 근거로 사용하지 않는다.
- AGENTS/CLAUDE의 일부 ACTIVE 키 나열보다 현재 PlayerSkills/Controller가 더 넓다.
  D/F/X/Space를 옛 목록만으로 비활성이라고 판단하지 않았다.
- TEAM Area 표와 MapCatalog 설명의 Valtan275/13,186, Kouku318/2,971,
  CS catalog55/nav42×60 등은 현재 header와 다르다. 이번 보고는 실제 수량을 사용했다.
- 복원 노트의 캐릭터 override0은 이 조사 중 추가된22행보다 이전이다. 현재 source activation
  변경과 실행 검증을 분리했다. 공유 노트와 다른 작업 파일은 이 읽기 전용 검토에서 덮어쓰지 않았다.

## 이번 검증 결과

| 항목 | 실제 결과 |
|---|---|
| 시작 상태·fetch·branch/HEAD | 확인 |
| 세 직업83문서 JSON parse·catalog ID join | 성공, catalog 부재0 |
| 문서의 명시 Resources-relative 경로 실재 | 세 직업 모두 누락0, binary/GPU 검증 아님 |
| 전체57개 definition과 저장 문서 수 대조 | 아래 전체 표 및 inventory JSON에 기록 |
| map catalog/header/asset 집계 | 네 맵 확인 |
| CS/Kouku material authoring/runtime JSON | parse 및 의미 일치 |
| 실제 소비자 검토 | strict selector, suppression, original/asset cue, shake, importer, Level scope 확인 |
| 제품 변경·compile·publish·UI 실행·화면 | 이번 조사에서 미실행 |
| 문서 링크·집계 확인 | 2문서의 local link21개 실재·줄 위치 확인, 전체57행의 직업별 문서/element 합계 일치 |
| 이번 두 문서 공백 검사 | `git diff --no-index --check`로 확인, 끝의 빈 줄1개 제거 후 재검사 |
| 전체 worktree `git diff --check` | 다른 작업의 `ArenaCameraProfile.cpp:53`, `Effect_DocumentRenderer.cpp:20233` trailing whitespace2건으로 실패. 해당 파일은 수정하지 않음 |

자동 검사 기록과 사용자가 기존 작업에서 확인한 링 금색/Q유리는 서로 다른 증거다.
이번 조사로 새로운 visual PASS나 전체 복원 완료를 부여하지 않았다.

## 전체 스킬 정의별 현재 문서 수

동일 슬롯이라도 자세·기상/이동 의미가 다르면 서로 다른 정의다. 문서0개가 항상 결손은 아니며,
애니메이션 원본 cue와 현재 gameplay 소비를 함께 봐야 한다. element는 현재 저장본 수다.
마지막 열은 `failClosed=true && authoringApproximate=false`인 실행 억제 조건의 행 수다.
`failClosed=true`여도 근사 실행이 허용된 행은 포함하지 않는다.

### 도화가

| 슬롯 | skill ID | 이름 | 자세 | 문서 | element | visible | 명시 execution | 실행 억제 조건 |
|---|---:|---|---|---:|---:|---:|---:|---:|
| LMB | 31000 | 평타 | NONE | 4 | 11 | 10 | 0 | 0 |
| Q | 31200 | 필법 : 먹물세례 | NONE | 1 | 4 | 4 | 2 | 0 |
| W | 31430 | 필법 : 흩뿌리기 | NONE | 1 | 1 | 1 | 0 | 0 |
| E | 31480 | 묵법 : 두루미나래 | NONE | 1 | 7 | 7 | 0 | 0 |
| R | 31210 | 필법 : 콩콩이 | NONE | 2 | 2 | 2 | 0 | 0 |
| A | 31460 | 묵법 : 호접몽 | NONE | 1 | 2 | 2 | 0 | 0 |
| S | 31420 | 묵법 : 난치기 | NONE | 1 | 2 | 2 | 0 | 0 |
| D | 31490 | 묵법 : 범가르기 | NONE | 1 | 68 | 68 | 12 | 0 |
| F | 31470 | 필법 : 한획긋기 | NONE | 1 | 17 | 17 | 17 | 0 |
| T | 31950 | 묵법 : 미르 새김 | NONE | 1 | 23 | 11 | 1 | 15 |
| V | 31910 | 절기 : 몽유도원 | NONE | 1 | 46 | 46 | 0 | 0 |
| ALT_V | 31930 | 몽중백화원 | NONE | 2 | 97 | 97 | 0 | 0 |
| X | 31110 | 떠오르는 해 | NONE | 0 | 0 | 0 | 0 | 0 |
| Z | 31050 | 저무는 달 | NONE | 2 | 3 | 3 | 0 | 0 |
| SPACE | 31020 | 필법 : 흘리기 | NONE | 0 | 0 | 0 | 0 | 0 |
| SPACE | 31030 | 기상기 | NONE | 0 | 0 | 0 | 0 | 0 |

### 창술사

| 슬롯 | skill ID | 이름 | 자세 | 문서 | element | visible | 명시 execution | 실행 억제 조건 |
|---|---:|---|---|---:|---:|---:|---:|---:|
| Q | 34040 | 이연격 | LANCE_MASTER_LONG_SPEAR | 2 | 6 | 6 | 0 | 0 |
| W | 34090 | 철량추 | LANCE_MASTER_LONG_SPEAR | 1 | 4 | 4 | 0 | 0 |
| E | 34100 | 청룡출수 | LANCE_MASTER_LONG_SPEAR | 3 | 17 | 17 | 0 | 0 |
| R | 34160 | 공의연무 | LANCE_MASTER_LONG_SPEAR | 2 | 14 | 14 | 0 | 0 |
| A | 34140 | 선풍참혼 | LANCE_MASTER_LONG_SPEAR | 3 | 16 | 16 | 0 | 0 |
| S | 34120 | 연환섬 | LANCE_MASTER_LONG_SPEAR | 3 | 6 | 6 | 0 | 0 |
| D | 34110 | 반월섬 | LANCE_MASTER_LONG_SPEAR | 1 | 2 | 2 | 0 | 0 |
| F | 34150 | 맹룡열파 | LANCE_MASTER_LONG_SPEAR | 1 | 3 | 3 | 0 | 0 |
| T | 34650 | 적룡필살 | NONE | 2 | 10 | 10 | 0 | 0 |
| V | 34610 | 적룡질풍격 | NONE | 3 | 9 | 9 | 0 | 0 |
| ALT_V | 34630 | 마룡합일섬 | NONE | 4 | 210 | 210 | 10 | 0 |
| SPACE | 34020 | 탄영 | LANCE_MASTER_LONG_SPEAR | 0 | 0 | 0 | 0 | 0 |
| SPACE | 34030 | 기상기 | NONE | 0 | 0 | 0 | 0 | 0 |
| LMB | 34010 | 긴 창_평타 | LANCE_MASTER_LONG_SPEAR | 4 | 9 | 9 | 0 | 0 |
| Z | 34000 | 무기 변경(짧은 창) | LANCE_MASTER_LONG_SPEAR | 0 | 0 | 0 | 0 | 0 |
| Z | 34500 | 무기 변경(긴 창) | LANCE_MASTER_SHORT_SPEAR | 0 | 0 | 0 | 0 | 0 |
| LMB | 34510 | 평타 | LANCE_MASTER_SHORT_SPEAR | 3 | 10 | 10 | 0 | 0 |
| SPACE | 34520 | 돌파 | LANCE_MASTER_SHORT_SPEAR | 0 | 0 | 0 | 0 | 0 |
| Q | 34540 | 나선창 | LANCE_MASTER_SHORT_SPEAR | 1 | 4 | 4 | 0 | 0 |
| W | 34550 | 사두룡격 | LANCE_MASTER_SHORT_SPEAR | 1 | 8 | 8 | 0 | 0 |
| E | 34560 | 굉열파 | LANCE_MASTER_SHORT_SPEAR | 2 | 17 | 17 | 0 | 0 |
| R | 34570 | 유성강천 | LANCE_MASTER_SHORT_SPEAR | 2 | 22 | 22 | 0 | 0 |
| A | 34580 | 절룡세 | LANCE_MASTER_SHORT_SPEAR | 2 | 4 | 4 | 0 | 0 |
| S | 34590 | 적룡포 | LANCE_MASTER_SHORT_SPEAR | 3 | 13 | 13 | 0 | 0 |

### 워로드

| 슬롯 | skill ID | 이름 | 자세 | 문서 | element | visible | 명시 execution | 실행 억제 조건 |
|---|---:|---|---|---:|---:|---:|---:|---:|
| LMB | 17000 | 기본 평타 | NONE | 1 | 3 | 3 | 0 | 0 |
| SPACE | 17020 | 돌격 | NONE | 0 | 0 | 0 | 0 | 0 |
| SPACE | 17025 | 기상기 | NONE | 0 | 0 | 0 | 0 | 0 |
| Q | 17030 | 날카로운 창 | NONE | 1 | 7 | 7 | 0 | 0 |
| S | 17040 | 배쉬 | NONE | 1 | 11 | 11 | 0 | 0 |
| W | 17060 | 파이어 불릿 | NONE | 1 | 2 | 2 | 0 | 0 |
| E | 17080 | 대쉬 어퍼 파이어 | NONE | 1 | 7 | 7 | 0 | 0 |
| A | 17090 | 갈고리 사슬 | NONE | 1 | 14 | 14 | 0 | 0 |
| D | 17100 | 방패 격동 | NONE | 1 | 7 | 7 | 0 | 0 |
| R | 17110 | 리프 어택 | NONE | 2 | 4 | 4 | 0 | 0 |
| F | 17140 | 가디언의 낙뢰 | NONE | 1 | 4 | 4 | 0 | 0 |
| V | 17170 | 가디언의 수호 | NONE | 3 | 9 | 9 | 0 | 0 |
| T | 17240 | 풀배럴 캐넌 | NONE | 3 | 17 | 17 | 0 | 0 |
| ALT_V | 17250 | 수호의 맹세 | NONE | 2 | 262 | 262 | 0 | 0 |
| Z | 17800 | 방어 태세 | WARLORD_NORMAL | 0 | 0 | 0 | 0 | 0 |
| Z | 17810 | 방어 태세 해제 | WARLORD_DEFENSE | 0 | 0 | 0 | 0 | 0 |
| X | 17820 | 전장의 방패 | NONE | 3 | 7 | 7 | 0 | 0 |
