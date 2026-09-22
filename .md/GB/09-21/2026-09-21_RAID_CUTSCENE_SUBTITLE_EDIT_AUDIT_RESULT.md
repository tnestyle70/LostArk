# 발탄·쿠크 컷신 자막 현황과 편집 안내

2026-09-21 초기에 디스크 저장본과 C++ 소비 경로를 조사했고, 뒤이어 사용자가 제공한 발탄 등장 녹화본을 기준으로 누락 대사 두 행을 정본과 게시본에 추가했다. Client 실행·화면 조작은 하지 않았으므로 실제 화면에서의 최종 싱크 판정은 남아 있다.

## G1. 지금 확인된 상태

| 대상 | 현재 연결 | 데이터 확인 결과 |
|---|---|---|
| 발탄 | World 자막 10행 | 기존 원본 자막 8행은 보존했고, 녹화본에만 있던 부활 대사 2행을 `entrance`에 추가·게시했다. |
| 쿠크 Boss | 자막 17행 | 원본 수입 규칙과 문구·시작·길이·위치 일치. Action revision 2035와 patternbindings sourceRevision 2035의 자막 내용도 일치 |
| 쿠크 Sequence | 자막 20행 | 원본 수입 규칙과 문구·시작·길이·위치 일치. 저장 revision 156과 Encounter의 네 관문 sequenceRevision 156 일치 |

총 45행은 중복 컷신의 Boss/Sequence 사본까지 센 배치 수다. 서로 다른 대사 45개라는 뜻이 아니다. 쿠크에서 같은 패턴 안의 자막 시간끼리 겹치는 행은 없었다. 원본 문구와 시간이 보존됐다는 사실은 **현재 재편집된 음성·영상과 실제로 맞는다는 보증은 아니다.** 자동으로 대사 밀림을 찾아 주는 검사나 자막 전용 경고색은 없다.

### 발탄 부활 음성 대사의 누락 범위 (사용자 확인 후 보정)

위 발탄 8행은 원본 Matinee의 `EFInterpTrackSubtitle`/말풍선 트랙만 수입한 결과다. 사용자가 실제 부활 장면 음성에서 확인한 `일어나라, 발탄...`, `그 찢긴 영혼을 적의 피와 살로 채워라`는 이 트랙·GameMsg `cin.37053_*`에 없었다. 그래서 `entrance`에 `subtitle.custom.1`/`subtitle.custom.2` 두 행을 추가했다. 앞선 8행 전수 일치 검사는 **원본 자막 트랙 안에서만** 완전하다는 뜻이다.

사용자가 최종 지정한 시간은 각각 `2270ms / 3265ms`와 `9100ms / 5758ms`다. 기존 `cin.37053_03_01`/`03_02`의 시간·문구는 움직이지 않았다. 새 두 행은 `NORMAL` 위치와 빈 `slotId`를 사용한다.

### 먼저 재생해서 볼 곳

1. **쿠크 Sequence P4 `1관문_통합_시퀀스`**: 자막은 기존 변환 시각 32.777초·49.692초에 남아 있는데, 현재 음향은 두 구간으로 다시 잘라 배치돼 있다. 따라서 가장 먼저 음성을 들으며 확인할 후보다.
2. **실제로 문제 제보가 있는 다른 컷신**: 아래 전체 시간표를 기준으로 문구별 시작·끝을 확인한다. 아직 특정 줄이 몇 ms 늦다는 판정은 없다.
3. **발탄 MapTool에서만 밀리는 경우**: 첫 재생과 두 번째 재생의 오차가 같은지, 버벅일 때 더 벌어지는지 확인한다. 한 프레임 100ms를 넘는 지연에서 미리보기 시계와 연속 음향의 진행량이 달라질 가능성이 코드에 있다. 이런 경우에는 자막 원본값을 바꿔 보상하기 전에 재생 시계 문제를 구분해야 한다.

P4의 수치 근거: 원본 SCENE03A의 음향 이벤트는 0.7초 시작이고 기존 수입 변환의 배치는 12,958ms였다. 현재 SOUND `.presentation.54`는 시작 20,863ms / 음원 시작점 6,947ms, `.presentation.56`은 시작 13,496ms / 음원 시작점 352ms다. 같은 WAV의 유효 시작 기준은 각각 13,916ms와 13,144ms로, 이전 배치 대비 958ms / 186ms 뒤다. 두 자막은 모두 `.54` 구간 안에 있다. **이것은 음향과 자막의 상대 배치가 바뀌었다는 근거이며, 자막에 일괄 +958ms가 정답이라는 뜻은 아니다.** 원본 시간 변환과 실제 발화 구간을 함께 들어 확인해야 한다.

### 자막이 없어도 입력 오류가 아닌 경우

- 발탄 늑대의 `머리는 내 것이다, 루가루!`와 `새로운 사냥감이군.`은 MapTool의 `1관문 입장(늑대)`에 연결돼 있다. 제품 자동 레이드 흐름에 늑대 컷신을 시작하는 연결은 없다.
- 발탄 포효·2페이즈 전환과 쿠크 카드미로는 조사한 원본에 해당 자막 트랙이 없다.
- 쿠크 원본 SCENE02A의 망치 말풍선은 대응 컷신 연결이 없다. 기존에 붙인 다른 자막의 Start를 고쳐서 나타나게 할 수 있는 항목이 아니다.
- 앵콜의 `누구 맘대로 끝을 내?!` 등 3행은 **현재는 Boss P97/Sequence P10에 모두 붙어 있다.** 09-20 초반 문서의 ‘미연결’ 설명은 현재 상태가 아니다.
- 쿠크 `자, 이제 클라이막스를 시작해볼까?`가 두 줄 반복되는 것은 저장 중 중복 추가가 아니라 원본 GameMsg의 두 줄을 보존한 결과다. `장르를 바꾸는 거야. / 로맨스?`도 한 자막 항목 안의 두 줄이다.
- `31_04`, `31_06`, `33_03`의 원본 두 번째 줄에는 회색 FONT 태그가 있지만 현재 plain text 변환은 태그를 제거해 두 줄 모두 흰색으로 표시한다. 타이밍 문제와는 별개의 표현 차이다.

## G2. 쿠크는 어느 컷신을 고치는가

**실제 레이드 입장·전환·엔딩은 Sequence 문서를 먼저 편집한다.** 같은 이름이 Boss 쪽에도 있지만 별도 데이터다. 한쪽 변경이 다른 쪽으로 자동 복사되지 않는다.

| 게임에서 확인할 장면 | 우선 편집할 Sequence 항목 | Boss 쪽 별도 사본 |
|---|---|---|
| 1관문 입장 | P4 `1관문_통합_시퀀스` | 같은 통합 자막 사본 없음 |
| 2관문 입장 | P3 `2관문_진입컷씬` | P73 `2관문_진입컷씬` |
| 2관문 클리어→3관문 진입 | P5 `2관문_클리어` | P74 `2관문클리어_3관문진입` |
| 앵콜·빙고 진입 | P10 `앵콜컷신` | P97 `앵콜컷신` |
| 빙고 마지막 엔딩 | P9 `빙고_최종엔딩씬` | P75 `빙고_최종엔딩씬` |
| 쇼타임 스킬 연출 | 위 자동 입장 시퀀스와 별개 | P76 `쇼타임_연출` |

여기서 P번호는 `KAKULSAYDON_G1_PATTERN_번호`의 축약이다. Sequence P7 `3관문_진입`에도 자막 2행이 있지만 실제 raid plan이 GATE3 진입에 사용하는 항목은 P5다. P7만 편집하면 P5의 자동 전환에는 반영되지 않는다.

자막 보유 패턴의 `authoringStatus`가 DRAFT여도 자막 미게시·재생 불가라고 단정하지 않는다. 현재 Action의 17행은 실제 생성물에 있고 자동 Sequence는 위 raid plan이 선택한다.

쿠크 정본:

- Sequence: `Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json`
- Boss: `Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json`
- 표시 문구·상하 위치: `presentationResources[]`의 `subtitleText`, `subtitlePosition`
- 각 장면의 시작·표시 길이: `patterns[].presentationOccurrences[]`의 `startMs`, `durationMs`

같은 resource의 문구를 바꾸면 그 resource를 쓰는 같은 문서의 모든 자막 배치에 적용된다. 각 줄의 시간은 배치별로 따로 바뀐다. 표시 이름을 Rename하는 것은 실제 자막 본문 수정이 아니다.

## G3. 쿠크 시간 편집 순서

1. **KoukuSaydon Arena → F1 → Action Workbench → Sequence**로 들어간다. 다른 Level에서도 편집 문서를 열 수 있지만 실제 쿠크 연출 Preview는 쿠크 Arena가 필요하다.
2. 위 표의 컷신을 선택한다. 타임라인의 **Subtitle 줄에 놓인 박스**를 선택한다. Resources 목록은 원문 확인·추가용이고 이미 배치된 줄의 시간은 타임라인 박스에서 고친다.
3. Box Detail에서 **Start ms**, **Lifetime ms**를 수정한다.
4. **Apply**를 누른다. 숫자를 입력한 뒤 Save만 누르면 아직 Apply하지 않은 숫자는 저장되지 않는다.
5. **Play / Play Sequence**로 확인한다. 현재 커서부터 재생하므로 전체 검토는 Reset 후 처음부터 재생한다. 정확한 입력에는 숫자 칸을 쓰는 편이 편하다. 박스 가운데를 드래그하면 시각을 이동하고 양 끝 드래그는 길이를 바꾼다.
6. 확정한 뒤 **Save**한다.

`Play Row`는 애니메이션 행용이므로 자막·음성 싱크 확인에는 컷신 전체 Play를 사용한다. 자막 resource/박스의 Preview만 눌러서는 원래 카메라·음향과의 싱크를 판단할 수 없다. 타임라인 눈금 이동은 특정 순간의 문구·위치를 확인하는 용도이며, 음성과 맞는지는 해당 대사보다 1~2초 앞에서 연속 재생해 들어 본다.

일반 Sequence Play는 편집 중 draft의 로컬 미리보기다. Complete Play는 관문의 Server 진행을 포함한 검토다. Boss의 일부 연속 패턴은 Play도 Server 경로로 넘기므로 저장·게시를 요구할 수 있다.

### 문구·위치는 현재 어디서 바꾸는가

현재 쿠크 UI는 `Resources → Subtitle`에서 **문구를 보여 주기만 한다.** `subtitleText` 입력 칸이나 `NORMAL / UPPER` 선택 UI는 없다. 문구나 상하 위치 변경은 위 정본 JSON의 해당 resource를 수정해야 한다.

타이밍만 고칠 때는 JSON을 직접 편집할 필요가 없다. 문구도 고쳐야 한다면 미저장 편집을 먼저 저장하고 현재 파일의 정확한 resource ID를 기준으로 변경한 뒤 Reload해야 한다. Reload는 미저장 draft를 버리는 동작이므로 남은 편집이 있을 때 누르지 않는다. 직접 JSON을 바꾸는 경우 revision 및 저장·게시 freshness 계약까지 맞춰야 하므로 두 문서 전체를 이전 사본으로 덮어쓰지 않는다.

09-21 조사 당시에는 범용 Subtitle Box의 3D Position / Rotation / Scale이 최종 자막
렌더러에 전달되지 않았다. 이 누락은 [09-22 자막 배치 수정](../09-22/2026-09-22_KOUKU_SUBTITLE_LAYOUT_RESULT.md)에서
연결했다. 현재 Kouku Subtitle Box Detail의 `Screen X / Y`는 1080 높이 기준 pixel offset
(양수 Y 아래), `Text scale`은 균일 글자 배율이며 Preview/Apply/Save가 보존한다.
NORMAL/UPPER 기준점과 색상은 공통 렌더러가 유지한다. 이 교정은 해당 쿠크 자막 경로의
계약이며 World/Valtan 자막에 새 배치 필드를 추가했다는 뜻은 아니다.

## G4. 발탄 시간·문구 편집 순서

1. MapTool에서 발탄 Area **`LV_LUT_HEARTRB_ED`**를 선택하고 **Camera**를 연다. 독립 맵 편집 진입은 `Lobby → Test → Map Editor`다.
2. **`통합 컷신 편집` 체크를 끈다.** 체크돼 있으면 Composition 화면으로 전환되어 아래의 기존 자막 편집 UI가 숨겨진다.
3. **컷신**에서 장면을 고르고 **World 배우**에서 자막 소유 인스턴스를 선택한다.

| 컷신 표시명 | 선택할 World 배우 ID의 끝부분 |
|---|---|
| 발탄 등장(부활) | `entrance` |
| 발탄 스킬 연출(버러지) | `trash` |
| 발탄 최후 | `finale` |
| 1관문 입장(늑대), 흰늑대 | `gate1-entrance` |
| 1관문 입장(늑대), 검늑대 | `gate1-entrance.black-wolf` |

전체 ID 앞부분은 `world.sequence.instance.valtan.source-preview.`다. `entrance.colorless` 같은 보조 배우에는 자막이 없으므로 본체 `entrance`를 고른다.

4. **Subtitles** 목록에서 줄을 고른다.
5. **Text**, **Subtitle Start (ms)**, **Subtitle Duration (ms)**를 수정한다. **Position**은 Normal / Upper / Balloon이다. Balloon의 **Actor Slot**은 현재 늑대에서 `actor`다.
6. **Apply Subtitle → Play/Pause/Time (ms)로 확인 → `컷신 저장 (카메라 + World 저작본)`** 순서로 반영한다.

Apply가 실패하면 기존 draft를 보존하고 이유를 표시한다. Add Subtitle / Delete Subtitle도 같은 draft에서 처리한다. Reset Subtitle Inputs는 해당 줄의 입력을 현재 draft 값으로 되돌리는 기능이며, 이미 Apply한 모든 변경을 최초 원본으로 복구하는 기능은 아니다. 시작은 0 이상, 길이는 0보다 커야 하며 시작+길이가 해당 World 템플릿 길이를 넘어갈 수 없다.

정본은 `Data/Maps/Authoring/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.worldsequences.json`의 `templates[].subtitleTracks[]`다. `stringId`는 출처 식별자이고 출력 본문은 `text`다. 문구를 조정할 때 stringId를 바꿀 필요가 없다.

### 발탄 시간값의 기준

일반적으로 `템플릿 시간 = (컷신 Time - 배우 시작 지연) × 배우 재생 속도`다. **현재 자막 소유 배우 5개는 모두 지연 0, 속도 1**이므로 Start는 컷신 시작 후 ms이며 위쪽 Time 표시와 같다. 카메라 컷의 시작값을 더하거나 빼지 않는다.

`발탄 등장 본체(13.0s 부터)`라는 이름은 배우가 보이기 시작하는 시점 설명이다. 자막 19.289초에서 13초를 빼면 안 된다. 자막만 맞출 때 배우의 시작 지연·재생 속도를 바꾸면 애니메이션·사운드까지 움직이므로 Subtitle Start만 조정한다.

## G5. 실제로 싱크를 맞추는 방법

| 화면에서 관찰한 현상 | 먼저 조정할 것 |
|---|---|
| 목소리가 나온 뒤 자막이 늦게 등장 | Start를 줄임 |
| 말하기 전에 너무 일찍 등장 | Start를 늘림 |
| 등장 시각은 맞는데 너무 빨리 사라짐 | Duration / Lifetime을 늘림 |
| 등장 시각은 맞는데 다음 대사까지 남음 | Duration / Lifetime을 줄임 |
| 장면 전체의 자막이 매번 같은 만큼 어긋남 | 여러 대사에서 같은 오차인지 확인 후 해당 자막들의 Start를 같은 양만큼 이동 |
| 뒤로 갈수록 오차가 커짐, 재생마다 오차가 다름 | 음원 편집·배속·재생 시계/프레임 지연 확인. 전체 자막을 일정량 이동해서 해결하지 않음 |
| 미리보기에서는 맞는데 실제 진행에서는 다름 | Sequence/Boss 선택, 저장·게시 revision, 현재 재생 세션이 이전 데이터를 쓰는지 확인 |

단위는 1초=1,000ms다. 예를 들어 Start 10,233 / Lifetime 1,700인 자막이 실제로 0.5초 늦다면 Start를 9,733으로 바꾸고 Lifetime 1,700은 유지한다. 사라지는 시각도 함께 0.5초 빨라진다. 시작은 그대로 두고 종료만 0.5초 늦추려면 Lifetime을 2,200으로 바꾼다. 이 예시는 계산 설명이며 현재 앵콜에 필요한 수정값을 판정한 것이 아니다.

권장 반복 순서:

1. 쿠크는 Reset 후 Play, 발탄은 Stop 후 Play로 처음부터 정상 속도로 한 번 전체 재생한다.
2. 문제 문구, 현재 Start/Lifetime, 실제로 원하는 표시 시작·종료, 오차가 고정인지 변하는지를 적는다.
3. 한 줄의 Start부터 조정하고 Apply한다. 등장 시점이 맞으면 표시 길이를 조정한다.
4. 그 줄보다 1~2초 앞에서 다시 재생해 앞뒤 자막과 함께 확인한다. Pause/스크럽만으로 음성 싱크를 확정하지 않는다.
5. 같은 컷신을 처음부터 다시 재생한다. 두 번의 결과가 다르면 데이터 고정 오차보다 재생 상태를 조사한다.
6. Save 및 아래 게시 절차 뒤 실제 관문 흐름에서 다시 확인한다.

1관문 P4는 이미 원본 시계를 프로젝트 구간 시계로 변환했다. 원본의 21.9106초·35.0194초를 JSON에 그대로 넣어 되돌리지 않는다. 현재 전체 시간표의 32.777초·49.692초에서 실제 음성을 기준으로 조정한다.

## G6. 저장한 내용을 실제 게임에 반영하기

### 쿠크

**Sequence의 Apply → Save → Boss의 Actions 창 `Publish All Patterns` → 완료 메시지 확인 → 새 Complete Play** 순서다. Boss 쪽 수정이 있으면 그것도 Save한 뒤 게시한다. Sequence 편집창에는 Publish 버튼이 없으며 게시 작업은 Boss의 Publish All Patterns를 사용한다.

게시 버튼은 `Tools/Build/Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon`을 실행한다. 현행 완료 메시지·소비 계약은 다음 Complete Play에서 새 revision을 Server가 승인하고, 이미 실행 중인 재생은 원래 revision을 유지하는 방식이다. Publish 클릭만으로 진행 중인 컷신이나 모든 실행 프로세스의 메모리가 즉시 바뀌는 것은 아니다. 이 자막 시간 조정을 위해 매번 전체 Client/Server 빌드를 할 필요는 없다. 월드 배치 변경의 Server 재시작 조건은 자막 편집과 별개다.

게시 실패·revision 불일치는 상태 메시지와 로그를 확인한다. 자료를 다시 덮어쓰거나 revision 검사를 제거해 넘어가지 않는다. 다른 PC Server를 쓴다면 로컬 게시 결과가 그 PC에 자동 전송되지는 않는다. 그 Server가 승인하는 게시 데이터 세대와 Client 저장본도 일치해야 한다.

### 발탄

`컷신 저장`은 저작본만 저장한다. 자막만 바꿨다면 아래 명령으로 WorldSequences 범위만 게시한다. 저장소 루트에서 실행한다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_LUT_HEARTRB_ED -Scope WorldSequences -Mode Publish
```

실행용 출력은 `Client/Bin/DataFiles/Map/LV_LUT_HEARTRB_ED.worldsequences.json`이며 직접 편집하지 않는다. MapTool의 draft 미리보기와 제품 Level의 로드된 WorldSequence는 별도이므로, 제품 확인은 게시된 데이터를 새로 읽은 진입에서 한다. 새 진입의 캐시 여부가 불명확하면 저장을 마친 뒤 Client를 재실행해 확인한다. 이는 **게시 전에 Client 종료가 필요하다는 뜻이 아니다.**

문구·시각 같은 JSON만 편집하면 C++ 재컴파일은 필요 없다. 공통 글꼴·글자 크기·세부 좌표의 C++ 렌더러를 수정한다면 별도 코드 변경 및 Product Build가 필요하다.

## G7. 현재 전체 자막 시간표

아래 표는 조사 시점의 저장값이다. 초 단위의 시작·끝은 편집 전 기준점이며 ‘음성과 일치함’ 판정표가 아니다. 줄바꿈은 `<br>`로 표시했다. 파일의 자막 배열 순서와 무관하게 표는 시작 시간순으로 정렬했다.

### 발탄 World — 10행

| 배우 ID 끝부분 | GameMsg | 시작 초 | 끝 초 | 길이 ms | 위치 | 문구 |
|---|---|---:|---:|---:|---|---|
| `gate1-entrance` | `cin.37053_01_02` | 3.733 | 5.533 | 1800 | BALLOON | 머리는 내 것이다, 루가루! |
| `gate1-entrance.black-wolf` | `cin.37053_01_01` | 7.500 | 9.700 | 2200 | BALLOON | 새로운 사냥감이군.  |
| `entrance` | `subtitle.custom.1` | 2.270 | 5.535 | 3265 | NORMAL | 일어나라, 발탄... |
| `entrance` | `subtitle.custom.2` | 9.100 | 14.858 | 5758 | NORMAL | 그 찢긴 영혼을 적의 피와 살로 채워라. |
| `entrance` | `cin.37053_03_01` | 19.289 | 20.589 | 1300 | NORMAL | 나, 는... |
| `entrance` | `cin.37053_03_02` | 20.645 | 24.145 | 3500 | NORMAL | 마수군단장... 발탄...! |
| `finale` | `cin.37053_04_01` | 5.460 | 7.760 | 2300 | NORMAL | 안돼, 안돼! |
| `finale` | `cin.37053_04_02` | 7.860 | 14.360 | 6500 | NORMAL | 크으으아아아악! 카마... 인...! |
| `trash` | `cin.37053_09_01` | 0.623 | 2.423 | 1800 | NORMAL | 이 버러지들... |
| `trash` | `cin.37053_09_02` | 3.276 | 5.776 | 2500 | NORMAL | 다... 짓뭉개주마! |

### 쿠크 Sequence — 20행

| 패턴 | 문구 | 시작 초 | 끝 초 | 길이 ms | 위치 | 배치 ID 끝부분 / GameMsg |
|---|---|---:|---:|---:|---|---|
| P3 2관문_진입컷씬 | 몸풀기는 여기까지 하고, 다음 장으로 넘어가 볼까? | 3.080 | 9.380 | 6300 | NORMAL | `.presentation.14` / `cin.37081_25_01` |
| P3 2관문_진입컷씬 | 판이 깔렸으니, 신나게 놀아보자고! | 20.000 | 24.300 | 4300 | NORMAL | `.presentation.15` / `cin.37081_25_02` |
| P4 1관문_통합_시퀀스 | 고맙습니다. 고맙습니다. 여러분! 고맙습니다~! | 32.777 | 37.023 | 4246 | NORMAL | `.presentation.45` / `cin.37081_22_01` |
| P4 1관문_통합_시퀀스 | 여러분, 혜성처럼 등장한 머저리들을 소개하겠습니다! | 49.692 | 57.163 | 7471 | NORMAL | `.presentation.46` / `cin.37081_22_02` |
| P5 2관문_클리어 | 다들 준비됐어? | 1.343 | 3.043 | 1700 | NORMAL | `.presentation.28` / `cin.37081_31_01` |
| P5 2관문_클리어 | 오케이! 다음으로 넘어가자고! | 3.576 | 6.576 | 3000 | NORMAL | `.presentation.32` / `cin.37081_31_02` |
| P5 2관문_클리어 | 다음 이야긴 뭔데? | 8.942 | 10.442 | 1500 | NORMAL | `.presentation.29` / `cin.37081_31_03` |
| P5 2관문_클리어 | 장르를 바꾸는 거야.<br>로맨스? | 10.991 | 13.991 | 3000 | NORMAL | `.presentation.30` / `cin.37081_31_04` |
| P5 2관문_클리어 | 아니. 잔혹한, 스릴러...! | 14.568 | 17.768 | 3200 | NORMAL | `.presentation.31` / `cin.37081_31_05` |
| P5 2관문_클리어 | 자, 이제 클라이막스를 시작해볼까?<br>자, 이제 클라이막스를 시작해볼까? | 26.445 | 30.445 | 4000 | NORMAL | `.presentation.33` / `cin.37081_31_06` |
| P7 3관문_진입 | 아니. 잔혹한, 스릴러...! | 0.000 | 1.058 | 1058 | NORMAL | `.presentation.15` / `cin.37081_31_05` |
| P7 3관문_진입 | 자, 이제 클라이막스를 시작해볼까?<br>자, 이제 클라이막스를 시작해볼까? | 9.735 | 13.735 | 4000 | NORMAL | `.presentation.16` / `cin.37081_31_06` |
| P9 빙고_최종엔딩씬 | 일어나, 발연기 그만하라고. | 14.467 | 17.642 | 3175 | NORMAL | `.presentation.15` / `cin.37081_33_01` |
| P9 빙고_최종엔딩씬 | 좋아. 오늘은 여기까지! | 19.633 | 22.816 | 3183 | NORMAL | `.presentation.16` / `cin.37081_33_02` |
| P9 빙고_최종엔딩씬 | 계속 분발해.<br>자만하지 말고! | 27.267 | 30.237 | 2970 | NORMAL | `.presentation.17` / `cin.37081_33_03` |
| P9 빙고_최종엔딩씬 | 우리가 널 응원하고 있으니까 말이야. | 31.208 | 36.458 | 5250 | NORMAL | `.presentation.18` / `cin.37081_33_04` |
| P9 빙고_최종엔딩씬 | 그럼 혼돈이 가득 차오를 때, 다시 보자고! | 38.367 | 43.423 | 5056 | NORMAL | `.presentation.19` / `cin.37081_33_05` |
| P10 앵콜컷신 | 누구 맘대로 끝을 내?! | 10.233 | 11.933 | 1700 | NORMAL | `.presentation.6` / `cin.37081_12_01` |
| P10 앵콜컷신 | 무효야, 전부 무효! | 12.367 | 15.967 | 3600 | NORMAL | `.presentation.7` / `cin.37081_12_02` |
| P10 앵콜컷신 | 진짜 시작은 지금부터라고! | 16.700 | 19.750 | 3050 | NORMAL | `.presentation.8` / `cin.37081_12_03` |

### 쿠크 Boss — 17행

| 패턴 | 문구 | 시작 초 | 끝 초 | 길이 ms | 위치 | 배치 ID 끝부분 / GameMsg |
|---|---|---:|---:|---:|---|---|
| P73 2관문_진입컷씬 | 몸풀기는 여기까지 하고, 다음 장으로 넘어가 볼까? | 3.080 | 9.380 | 6300 | NORMAL | `.presentation.14` / `cin.37081_25_01` |
| P73 2관문_진입컷씬 | 판이 깔렸으니, 신나게 놀아보자고! | 20.000 | 24.300 | 4300 | NORMAL | `.presentation.15` / `cin.37081_25_02` |
| P74 2관문클리어_3관문진입 | 다들 준비됐어? | 1.343 | 3.043 | 1700 | NORMAL | `.presentation.28` / `cin.37081_31_01` |
| P74 2관문클리어_3관문진입 | 오케이! 다음으로 넘어가자고! | 3.576 | 6.576 | 3000 | NORMAL | `.presentation.32` / `cin.37081_31_02` |
| P74 2관문클리어_3관문진입 | 다음 이야긴 뭔데? | 8.942 | 10.442 | 1500 | NORMAL | `.presentation.29` / `cin.37081_31_03` |
| P74 2관문클리어_3관문진입 | 장르를 바꾸는 거야.<br>로맨스? | 10.991 | 13.991 | 3000 | NORMAL | `.presentation.30` / `cin.37081_31_04` |
| P74 2관문클리어_3관문진입 | 아니. 잔혹한, 스릴러...! | 14.568 | 17.768 | 3200 | NORMAL | `.presentation.31` / `cin.37081_31_05` |
| P74 2관문클리어_3관문진입 | 자, 이제 클라이막스를 시작해볼까?<br>자, 이제 클라이막스를 시작해볼까? | 26.445 | 30.445 | 4000 | NORMAL | `.presentation.33` / `cin.37081_31_06` |
| P75 빙고_최종엔딩씬 | 일어나, 발연기 그만하라고. | 14.467 | 17.642 | 3175 | NORMAL | `.presentation.15` / `cin.37081_33_01` |
| P75 빙고_최종엔딩씬 | 좋아. 오늘은 여기까지! | 19.633 | 22.816 | 3183 | NORMAL | `.presentation.16` / `cin.37081_33_02` |
| P75 빙고_최종엔딩씬 | 계속 분발해.<br>자만하지 말고! | 27.267 | 30.237 | 2970 | NORMAL | `.presentation.17` / `cin.37081_33_03` |
| P75 빙고_최종엔딩씬 | 우리가 널 응원하고 있으니까 말이야. | 31.208 | 36.458 | 5250 | NORMAL | `.presentation.18` / `cin.37081_33_04` |
| P75 빙고_최종엔딩씬 | 그럼 혼돈이 가득 차오를 때, 다시 보자고! | 38.367 | 43.423 | 5056 | NORMAL | `.presentation.19` / `cin.37081_33_05` |
| P76 쇼타임_연출 | 렛츠, 쇼타임~! | 0.689 | 2.989 | 2300 | UPPER | `.presentation.6` / `cin.37081_32_01` |
| P97 앵콜컷신 | 누구 맘대로 끝을 내?! | 10.233 | 11.933 | 1700 | NORMAL | `.presentation.6` / `cin.37081_12_01` |
| P97 앵콜컷신 | 무효야, 전부 무효! | 12.367 | 15.967 | 3600 | NORMAL | `.presentation.7` / `cin.37081_12_02` |
| P97 앵콜컷신 | 진짜 시작은 지금부터라고! | 16.700 | 19.750 | 3050 | NORMAL | `.presentation.8` / `cin.37081_12_03` |

## G8. 조사 근거와 검증 범위

- 원본 및 기존 검증: `09-20/2026-09-20_VALTAN_CUTSCENE_SUBTITLE_SOURCE_INVENTORY_RESULT.md`, `09-20/2026-09-20_VALTAN_SCENE_SUBTITLE_RESULT.md`, `09-20/2026-09-20_KOUKU_CUTSCENE_SUBTITLE_SOURCE_INVENTORY_RESULT.md`, `09-20/2026-09-20_KOUKU_ENCORE_SUBTITLE_ATTACH_RESULT.md`와 해당 out 원본 추출 자료를 현재 데이터와 대조했다.
- 쿠크 자동 관문 선택: `Tools/KoukuSaydonPipeline/raid_flow_projection.py:16`, 실제 Sequence 로드·revision 고정·Server clock: `Client/Private/MainApp.cpp:1532`.
- 쿠크 시간 UI: `Client/Private/KoukuSaydonActionWorkbench.cpp:12619`, Apply `:13067`, 문구 표시만 `:12334`, 게시 실행 `:1843`, 완료 뒤 새 재생 적용 계약 `:1915`.
- 발탄 편집: `Client/Private/MapTool_CameraShots.cpp:473`, `:1519`, `:1684`, `:2097`. 제품 자동 컷신 선택: `Client/Private/Level_ValtanArena.cpp:2862`.
- 공통 자막 렌더러: `Client/Private/MainApp.cpp:8423`. 현재 1080 높이 기준 글자 높이 26px, 행 간격 34px, 흰색과 검정 그림자. NORMAL/UPPER는 화면 높이 비율을 사용한다. 원본의 색 태그를 그대로 그리는 방식이 아니다.
- 말풍선: `Client/Private/WorldSequencePlayer.cpp:1378`의 visible 모델/bounds 조건과 `MainApp.cpp:8492`의 화면 투영 조건에 따라 표시를 생략할 수 있다.
- MapTool 지연 후보: `Client/Private/MapTool_Cutscenes.cpp:449`의 최대 0.1초 시계 진행과 `Client/Private/WorldSequencePlayer_Objects.cpp:1563`의 연속 음향 재생. 실제 프레임 지연으로 증상을 재현한 것은 아니다.

실행한 검증은 원본과 게시본 JSON 파싱, `entrance`의 네 자막 행 필드 비교, `Publish-MapAuthoring.ps1 -Mode Validate`, 같은 scope의 Publish다. Client 화면을 조작하거나 재생하지 않았으므로 실제 글자 표시와 음성 싱크의 최종 화면 판정은 남아 있다.


### 조사 시점 파일 식별

| 정본 파일 | SHA-256 |
|---|---|
| `Data/Maps/Authoring/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.worldsequences.json` | `19ec52d8f2a1226c7de8f3c43c3db9208a8d5ed6279a15afe02a8961a4ca82e1` |
| `Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json` | `aa2844e5855d1fb267d8b53b7902bb15c24ea8a6b232956ac6e16bb081338034` |
| `Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json` | `29cf5073ed65e3bad01c41e5f2ab119addd3e448558e27622b11224d98ff57e2` |

## G9. 사용자 녹화본 기준 발탄 부활 대사 적용

사용자가 제공한 `C:/Users/USER/OneDrive/바탕 화면/발탄 등장 컷신 .mp4`(25.13초, 30fps)를 재생 시간의 기준으로 사용했다. 기존 `나, 는...`가 19.289초에 시작하는 값은 그대로 보존했다. 이후 사용자가 실제 확인을 바탕으로 두 새 대사의 최종 시작·종료를 지정했으며, 이전에 추정했던 음향 오프셋은 적용하지 않는다.

| 새 stable ID | 시작 ms | 길이 ms | 끝 ms | 문구 |
|---|---:|---:|---:|---|
| `subtitle.custom.1` | 2270 | 3265 | 5535 | 일어나라, 발탄... |
| `subtitle.custom.2` | 9100 | 5758 | 14858 | 그 찢긴 영혼을 적의 피와 살로 채워라. |

두 행은 `NORMAL` 위치와 빈 `slotId`를 사용하며, 기존 `cin.37053_03_01`/`03_02`는 수정하지 않았다. 편집 전 정본은 `out/RaidSubtitleTiming20260921/valtan-worldsequences-before-illyakan-subtitles.json`으로 보관했다. 현재 값은 사용자가 확정한 시작·종료값을 duration으로 환산한 정본이다.

`Publish-MapAuthoring.ps1 -AreaId LV_LUT_HEARTRB_ED -Scope WorldSequences -Mode Validate`와 Publish를 실행했다. 게시본 `Client/Bin/DataFiles/Map/LV_LUT_HEARTRB_ED.worldsequences.json`에도 동일한 네 행이 파싱되어 들어간 것을 확인했다. 2026-09-21 최신 게시에서는 버러지 두 줄의 위치도 `UPPER`에서 `NORMAL`로 바뀌었고, 게시본 SHA-256은 `527f32f97995017be87c81323f76122b5a8dbcec4cd373c35b0a19a51e329d91`이다. 실행 중인 MapTool이나 Client 메모리에는 자동 반영되지 않으므로, 다음 확인은 컷신을 다시 열거나 재실행한 뒤 `발탄 등장 본체(13.0s 부터)`를 처음부터 재생해 수행한다.

## G10. 앵콜컷신 자막 위치 수정

앵콜컷신의 `cin.37081_12_01`~`03`은 `UPPER`로 저장돼 있어 화면 위에 표시됐다.
실행 정본 `Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json`의 P97과 Sequence
정본 `Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json`의 P10에서 세
resource의 `subtitlePosition`을 모두 `NORMAL`로 변경했다. `NORMAL`은 런타임의 화면 아래
자막 위치다.

쿠크 projection publisher와 Composition publisher를 실행했다. 생성된
`Data/Animation/Authored/KoukuSaydon/KoukuSaydon.patternbindings.json`은 sourceRevision
2036이며, 실제 P97 `.presentation.6`~`.presentation.8` occurrence 세 개 모두 `NORMAL`로
확인했다. Composition publish도 통과했다. 실행 중인 Client는 이미 이전 데이터를 메모리에
읽었으므로, 재시작 뒤 앵콜컷신을 다시 재생해야 변경이 표시된다.
