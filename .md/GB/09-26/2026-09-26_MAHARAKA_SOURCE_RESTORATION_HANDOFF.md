# 마하라카 원본 복원: Claude 순차 실행 설계·인계서

작성 기준: 2026-09-26, 실제 실행 저장소 `C:/Users/USER/source/졸업팀폴/LostArk`.

이 문서는 **다음 구현자를 위한 작업 설계와 실행 지침**이다. 게임 코드 적용 완료 보고서도, 그대로 붙여 넣는 C++ 전문 PLAN도 아니다. 아직 미확정인 무대 모델·소켓·트리거 연결을 가짜 값으로 채우지 않는다. 각 G에서 원본 연결을 확정한 뒤, 실제 수정 코드는 저장소 PLAN 규칙에 따라 작성한다.

목표는 마하라카의 지형·물·조명·모코모코·캐논·워터팡 무대가 **원본 입력과 원본 사건 순서로** 보이게 만드는 것이다. 베른의 구현 기능은 재사용하지만 베른의 좌표·조명·색·환경 데이터를 복사하지 않는다. 다른 연도 맵, 미니게임 보상·퀘스트 구현은 자동으로 범위에 넣지 않는다.

함께 사용할 명령문: `2026-09-26_MAHARAKA_CLAUDE_EXECUTION_PROMPT.md`.

## G00. 실제 작업본과 이미 끝난 작업을 먼저 고정한다

### G00-01. 읽을 문서와 보호 대상

1. 현재 저장소의 `AGENTS.md`, `CLAUDE.md`, `.md/GB/gotchas.md`, 존재하는 local 규칙.
2. `.md/TEAM/README.md`가 지정하는 현행 팀 정본, Area·렌더링·NPC 계약.
3. `.md/GB/09-26/2026-09-26_MAHARAKA_VIDEO_ISSUES_FIX_RESULT.md`의 **G05**. 그 앞의 중간 결과보다 G05가 최신이다.
4. `.md/GB/09-26/2026-09-26_MAHARAKA_SOURCE_FUNCTIONS_REPORT.md`의 정정 내용.
5. `.md/GB/렌더링이펙트복원V2.md`, `Tools/LevelPlacementExtractor/README.md`.
6. `.md/TEAM/NPC_OWNER_HANDOFF.md`의 optional material override 계약.

이 작업본에는 선박·베른 카메라·UI·NPC·렌더링 서비스 등 다른 작업의 미커밋 변경이 많다. `reset`, `checkout --`, 전체 파일 덮어쓰기, 자동 stash, `git add .`, 자동 merge를 하지 않는다. 같은 함수가 이미 수정되어 있으면 현재 diff를 읽어 보존한다. 충돌하는 의도가 확인될 때만 사용자에게 좁혀서 묻는다.

`C:/Users/USER/.codex/worktrees/7395/LostArk`는 이번에 조사한 실행 작업본이 아니다. 그곳에서 오래된 소스를 빌드하지 않는다. 엔드포인트도 이 복원 작업을 이유로 변경하지 않는다.

```powershell
Set-Location -LiteralPath 'C:\Users\USER\source\졸업팀폴\LostArk'
git status --short
git branch --show-current
git rev-parse HEAD
git rev-parse origin/main
git diff --stat
Get-CimInstance Win32_Process -Filter "Name='Client.exe' OR Name='Server.exe'" |
    Select-Object Name, ProcessId, ExecutablePath, CommandLine
```

문서 작성 시 branch는 `codex/main-ship-maharaka-0926`, HEAD는 `a84bbcd45ff995c70bf847c683f1d159cf1821a6`이었다. 이번 문서 조사에서 fetch만 했으며 origin/main은 `e234827fa72519cc4b9f9434012551b17363e524`였다. 다음 작업자는 자기 시작 시 다시 확인한다. 값이 바뀌었다고 이전 작업본으로 강제 되돌리지 않는다.

### G00-02. 현재 상태를 과장하지 않는다

| 대상 | 현재 설치·구현된 부분 | 아직 끝나지 않은 부분 |
|---|---|---|
| Landscape | 16개 컴포넌트의 source-layer diffuse/normal 32 PNG, 실제 WModel 입력에 설치 | 전체 GPU 재질·HDR·RNM/lightmap·원본 mip/derivative·별도 절벽 기하/UV 동일성 |
| 물 | mapmaterials의 water-42 10행, river-rock 13배치 포함 연결 | 각 MIC의 실제 permutation, VS 파형·반사·굴절·입력 carrier 전체 동일성 |
| 자체 회전 | mapmotions 65행/59배치의 축 합성 수정 | 미니게임 NPC 행동에 의한 회전·원본 상태 전이 |
| 모코모코·캐논 | 원본 2모델, 각 9 joints/10 clips, 원본 재질 3종, 2 NPC 배치·idle | 물 발사·얼굴·지면·종료 효과, material parameter 곡선, sound, 원본 AI 신호 |
| 워터팡 무대 | 원본 트리거·배치·일부 Prop 정의 확보 | 실제 무대 메시/재질/조각 연결, 회전·붕괴·복구 제품 소비자 |
| 실행 검증 | 이전 Debug Product 빌드/게시 PASS | 사용자 최종 화면 동일성 승인 |

NPC 제품 위치와 정체성은 다음을 유지한다. 새 연출 때문에 같은 NPC를 다시 WorldObject로 중복 생성하지 않는다.

| placement ID | archetype | 위치(m) / yaw |
|---|---|---|
| `npc.maharaka.source57009.actor100` | `NPC_MAHARAKA_MOKOMOKO` | `(75.179,19.904,-1007.262)` / 0° |
| `npc.maharaka.source57009.actor188` | `NPC_MAHARAKA_WATERCANNON` | `(75.05,22.36,-984.32)` / -46.1° |

두 모델은 기존 NPC `.01` prescale과 원본 ModelSize 170%/50%를 이미 반영했다. scale을 또 곱하지 않는다. yaw에는 기존 NPC preY -90° 보상이 포함되어 있다. 화면 방향은 사용자 확인 대상이다.

원본 3종 재질 중 기본은 program 26, 추가분은 1526/1527이다. 검증된 shader를 이름이 비슷한 PBR 재질로 교체하지 않는다.

### G00-03. 후보 출력은 새 폴더에만 만든다

다음 명령은 한 PowerShell 세션에서 실행한다. `$MhpRun`은 이 작업의 후보 폴더다. 이후 블록은 같은 변수와 작업 디렉터리를 사용한다.

```powershell
$MhpRepo = (Get-Location).Path
$MhpRun = Join-Path $MhpRepo ('out\MaharakaContinuation_' + (Get-Date -Format 'yyyyMMdd_HHmmss'))
if (Test-Path -LiteralPath $MhpRun) { throw '후보 폴더가 이미 존재합니다. 기존 결과를 덮어쓰지 않습니다.' }
New-Item -ItemType Directory -Path $MhpRun | Out-Null
$MhpPackageRoot = 'C:\ProgramData\Smilegate\Games\LOSTARK\EFGame\ReleasePC'
$MhpUmodel = 'C:\LostArkExtract\umodel\umodel_lostark_v7.exe'
$MhpEvidence = Join-Path $MhpRepo 'out\MaharakaReaudit20260926'
```

`out/MaharakaReaudit20260926/install_terrain.py`는 이전 파일 identity를 전제로 한 **이미 실행된 일회성 설치기**다. 다시 실행하거나 prehash 검사를 지우지 않는다. `check_moko.py`도 추출 출력을 쓰므로 읽기 전용 검사라고 부르지 않는다. 아래 `verify_installed.py`는 **09-26 설치 직후 snapshot용 읽기 검사**다. NPC 마지막 두 행·world 총 6배치·과거 NPC 행 불변을 assert하므로, 이후의 정당한 변경에도 실패할 수 있다. 실행 결과를 현재 diff와 대조하고 후속 검증은 stable ID와 해당 작업의 변경 전후 값을 기준으로 작성한다. 이 옛 검사를 통과시키려고 새 NPC나 배치를 삭제하지 않는다.

```powershell
python -B out/MaharakaReaudit20260926/verify_installed.py
if ($LASTEXITCODE -ne 0) { throw '09-26 snapshot과 다릅니다. 정상 후속 변경인지 설치 손상인지 먼저 구분하십시오.' }
```

G00 종료: 실제 checkout, 진행 중 편집, 원본 위치, 기존 diff, 현재 설치 검증 결과를 기록한다. 수정 전 파일 백업은 이번 작업에 해당하는 대상만 새 후보 폴더에 보존한다.

## G01. 원본 참조 사슬을 끊김 없이 복구한다

### G01-01. 원본 저장 위치

| 위치 | 내용·주의 |
|---|---|
| `C:/ProgramData/Smilegate/Games/LOSTARK/EFGame` | 설치 게임의 LPK, ReleasePC 패키지, 사운드 패키지 |
| `C:/LostArkExtract/LV_OCN_EVENTIS_MHP_20260919` | 기존 맵 배치·재질·지형 staging. 유일한 원본 범위가 아님 |
| `C:/LostArkExtract/MaharakaFunctions20260926/db/EFGame_Extra/ClientData/TableData` | 복호화된 797 DB. SQLite는 read-only로 연다 |
| `out/MaharakaFunctions20260926/deploy_57009.json` | 부모 섬 배치 |
| 같은 폴더의 `deploy_57010.json`, `deploy_57011.json` | 자식 존 배치. 부모 섬에 무조건 합치지 않음 |
| `out/MaharakaReaudit20260926/archive_index.json` | 원본 컨테이너 전체 인덱스. 해당 테이블에서 없다는 판단을 패키지 전체 부재로 확대하지 않음 |
| `out/MaharakaReaudit20260926/MN_ISMP_00.Action.loa` | 실제 남아 있는 모코모코/캐논 Action |
| `out/MaharakaReaudit20260926/MokoAction/MN_ISMP_00.action-effects.json` | 36 actions, 182 stages, 20종 particle, 111 sound notify. 111개의 서로 다른 소리라는 뜻 아님 |
| `out/MaharakaReaudit20260926/IstmAction/MN_ISTM_00.action-effects.json` | 무대 controller 관련 조사 입력. 전체 265 actions를 마하라카 전용이라고 단정하지 않음 |

새 조사 결과에는 `원본 파일 + 원본 hash + object/export/byte offset + 원본 ID + 참조 상대 + 해석 방식`을 남긴다. 이 hash는 조사·교체 안전장치이지 새 리소스 pack/lock 배포 시스템을 만들라는 뜻이 아니다.

### G01-02. 하나의 행동부터 자른다

첫 구현 대상은 원본 action **4225601 `모코코 물벼락_물벼락 쏟아내기`**다. 한 행동에서 모델·표정·물·지면·소리·정지가 맞은 후 나머지 행동으로 확대한다.

```powershell
python -B Tools/LevelPlacementExtractor/extract_action_effect_notifies.py `
    --source 'MN_ISMP_00=out/MaharakaReaudit20260926/MN_ISMP_00.Action.loa' `
    --action-id 4225601 --output (Join-Path $MhpRun 'MokoAction4225601')
if ($LASTEXITCODE -ne 0) { throw '원본 Action 추출 실패' }
```

원본 LOA SHA-256은 `251b1c56a9c4f7b4865b66780ef95bd9b96f052bb94307e66c59f22178e4a6a3`였다. 다르면 새 파일의 출처를 확인하고 이 문서 수치를 무조건 덮어씌우지 않는다.

| 원본 시각(초) | 원본 내용 | 구현 시 주의 |
|---|---|---|
| 0 → 6 | `Att_Battle_1_01` | 설치 clip `att_battle_1_01`과 실제 registry를 대조. pose와 notify가 같은 clock을 사용 |
| 0 → 6 | `PawnMaterialParam` 2개: `PetEmotion`, `opacity_intensity` | serialized payload의 조건·값·곡선·대상 slot을 추가 해석. 이름만 보고 상수 1로 만들지 않음 |
| 0 → 2.2999999523 | `PlayDecalEffect` | 현 추출에는 명시 asset ref가 없음. unresolved로 남겨 원본 payload를 조사 |
| 0 | `MococoWater1_Attack01_Cast1` 또는 Water2 변형 | LookInfo 조건에 따라 선택. 두 개를 동시에 재생하지 않음 |
| 0.5915589929 | `Par_G_ISMP_Face_01`, duration 5.4084410667 | `FX_01` attachment를 원본 socket에서 resolve |
| 2.2000000477 | `MococoWater1_Attack01_Shot1` 또는 Water2 변형 | cast/shot 선택 조건과 source gain/stop 의미 유지 |
| 2.2925870419 | `Par_G_ISMP_Attk01_Water_01`, duration 2.7232859135 | 발사 기원·방향·local/world simulation을 각각 추출 |
| 2.2999999523 | `Effect` 6개, duration 약 .1 | gameplay reference. 6개를 particle로 만들거나 Client에서 피해 판정하지 않음 |
| 2.3902139664 | `Par_G_ISMP_Attk01_WaterGround_01`, duration 2.9024178982 | 현재 labels에 FX_01 없음. 무조건 얼굴 socket에 붙이지 않음 |
| 5.0718688965 | `Par_G_ISMP_WaterFinish_01`, duration 0 | 0을 삭제·즉시 종료·임의 1초로 해석하지 않음. source particle lifetime/stop 의미 조사 |

위 particle의 full package는 `FX_MN_ISMP_00`이다. 정확한 float 값과 offset은 원본 JSON을 정본으로 삼는다. 표의 표시 자릿수로 원본을 다시 저장하지 않는다. ms 변환은 기존 clock codec 규칙으로 한 번만 수행한다.

G01 종료: 위 행 모두를 `source-bound / unresolved / gameplay-reference`로 분류하고, 실제 사용할 clip·particle·sound·socket·material parameter의 연결표를 만든다. `unresolved`가 있으면 무엇이 미완료인지 이름으로 남긴다.

## G02. 워터팡 무대의 정체성을 먼저 찾는다

### G02-01. 이미 틀린 것으로 확인된 가정

- `MN_ISTM_00` LookInfo는 **`MN_Empty_00.Mesh.MN_Empty_00_SK`**를 가리킨다. 링 위 20 NPC(570942)는 이것만으로 꽃 모양 무대 메시가 아니다.
- 57011의 Prop ID 필드는 `ints_0x30_0x68[13]`이다. [12]를 ID로 읽은 이전 비교를 반복하지 않는다.
- Prop는 `EFTable_Prop.db`에서 조회한다. Npc DB 조회 결과로 Prop 삭제를 판단하지 않는다.
- 118 Prop 배치 중 현재 Prop 정의와 연결된 것은 39개, 이 테이블에서 찾지 못한 것은 79개였다. 이는 패키지 삭제 증명이 아니다.
- 300004는 모델 없는 충돌 Prop다. 이름 비슷한 몬스터 메시로 대체하지 않는다.
- 57011 actor22/NPC570941 같은 자식 존 배우를 부모 57009에 복제하면 같은 연출이 겹칠 수 있다.

### G02-02. 조사 순서

1. `out/MaharakaReaudit20260926/prop-table-57009.json`, `prop-table-57011.json`, `audit_props.py`를 읽어 현재 조인 열과 정의를 확인한다. 스크립트는 결과 JSON을 다시 쓰므로 먼저 읽고 새 출력 폴더를 받도록 안전하게 정리한다.
2. 57011의 Prop ID·actor ordinal·좌표·rotation·scale·source visibility를 행 단위로 묶는다. 570947/570992/571055/571056/571057 등 기존 조사에 나온 ID도 **테이블 종류를 확인한 후** 추적한다.
3. Prop/PropReplace/LookInfo/NpcAiStateAction/NpcSignal/TriggerMapData의 실제 schema를 `sqlite_master`, `PRAGMA table_info`로 확인한다. 다른 DB의 같은 숫자를 같은 ID라고 취급하지 않는다.
4. `ground_destroy_shake`, `ground_destroy`, `ground_destroy_Repair`, `TA_GroundDestroy`의 발생자→수신자→target group→모델/상태 대체를 따라간다. Trigger 문자열이 있다는 사실만으로 모델을 찾았다고 하지 않는다.
5. Action·LookInfo·레벨 export에서 full object path를 모은다. StaticMesh/InterpActor/SkeletalMesh/FracturedStaticMesh/Matinee 트랙/동적 Spawn·replace를 각각 조사한다. visibility=hidden은 이벤트 전 상태일 수 있다.
6. full object path가 가리키는 패키지를 archive index·ReleasePC·기존 staging에서 찾는다. 공용 패키지와 다른 시즌 패키지도 후보로 조사하되 2021 source reference와 일치 여부를 분리한다.
7. 후보마다 vertex/index·material slots·bounds·pivot·UV·source transform·조각 수·트랙 지속시간을 추출해 원본 연결표를 완성한다. 홍보 이미지와 비슷하다는 이유만으로 정본 후보를 확정하지 않는다.
8. 정의가 비어 있으면 해당 파일/테이블/전체 인덱스에서 어디까지 검색했는지 기록한다. 다른 패키지를 조사할 수 있으면 계속한다. 소스가 여전히 불명인 조각은 적용하지 않고, 독립적인 G03~G08은 진행한다.

```powershell
rg -n -i 'ground_destroy|TA_GroundDestroy|MN_ISTM_00|PropReplace' `
    out/MaharakaFunctions20260926 out/MaharakaReaudit20260926 `
    -g '*.txt' -g '*.py' -g '*summary*.json'
```

이 검색은 첫 진입점이다. base64 payload와 binary UPK는 문자열 검색만으로 다 조사할 수 없다. 전체 archive index가 한 줄짜리라면 `rg`로 통째 출력하지 말고 JSON을 parse하여 관련 항목만 출력한다.

### G02-03. 무대 적용 승인 조건

아래 연결이 **한 줄도 끊기지 않은 조각만** G09로 보낸다.

`zone → actor/Prop ID → 원본 object path → mesh/material → pivot/transform → signal 수신 대상 → intact/rotate/destroy/repair 상태`

연결되지 않은 조각에 임의 원통·분홍 재질·회전 각속도를 넣지 않는다. 원본이 없다고 단정해 검색을 끝내지도 않는다. 모델은 확보했지만 회전곡선이 없으면 모델 설치와 회전 미완료를 따로 보고한다.

## G03. 베른의 렌더링 기능과 마하라카의 부족한 입력을 구분한다

### G03-01. 현재 파일 실측

| 항목 | 베른 | 마하라카 |
|---|---|---|
| `LevelRegistry.cpp` scene profile | `scene.bern.neutral-day.v1` | `scene.development.neutral.v1` |
| runtime mapmaterials 행 | 23,153 | 329 |
| 그중 `bakedLighting` 보유 행 | 21,321 | 0 |
| `placementLighting` 행 | 49,047 | 0 |
| maplights 행 | 315 | 33 |
| 상시 mapeffects | 91 presentation 행 | 해당 문서·Level 연결 없음 |

이 수치는 조사 시점의 **문서 행 수**다. 화면에 보이는 메시 개수도, 추가해야 할 목표 개수도 아니다. 베른이 더 예쁜 원인을 수치 하나로 확정하지 않는다. 다만 마하라카는 동일 렌더러를 쓰면서도 baked/instance 조명과 전용 scene 입력 연결에 차이가 있는 것이 확인됐다.

베른 현재 프로필에 shadow 옵션이 꺼져 있는 항목도 있다. “베른처럼 하려면 그림자를 전부 켜야 한다”는 결론은 틀리다. 양쪽 맵 전체를 character-select PBR로 바꾸지도 않는다.

### G03-02. 재사용할 기존 소유자

| 파일·함수 | 책임 | 이번 작업 |
|---|---|---|
| `Client/Private/LevelRegistry.cpp` | Area의 scene profile 선택 | 검증된 마하라카 전용 profile ID를 MAHARAKA에만 선택 |
| `Client/Private/Level_Bern.cpp` | 맵+조명+상시 이펙트의 준비/commit/해제 예시 | 흐름 참고. 베른 객체·좌표를 복사하지 않음 |
| `Client/Private/Level_Development.cpp::Ready_Lights` | MAHARAKA가 사용하는 맵 조명 생성 | 기존 33 lights 경로를 유지하고 원본 입력 누락 보완 |
| 같은 파일의 Initialize/Update/종료 | 맵·SelfMotions·replication 수명 | 준비된 Maharaka effect/sequence만 연결, 전용 Level을 또 만들지 않음 |
| `CMapLightPresentationRuntime` | 맵 light 로드·Submit_Frame·Clear | 원본 component와 scene intensity 계약 유지 |
| `CMapEffectPresentationRuntime::Load_AmbientArea` | 상시 맵 particle | 원본 상시 효과만. NPC의 시한부 얼굴 효과를 여기 넣지 않음 |
| `CWorldSequencePlayer` | 기존 World 시퀀스 모델·변환·effect·sound | 무대/명시 preview의 동일 source clock 사용 |
| `CModel → CMaterial` | 실제 메시/재질 draw | 기존 source material family/carrier 확장 |
| `CRenderingProfileService` | scene/region/global quality 적용 | 공용 조율값을 보존, 마하라카 입력의 출처를 분리 |

### G03-03. 원본 입력부터 보완하는 순서

1. source visibility·기하·UV·normal/tangent·vertex color가 맞는지 확인한다.
2. 원본 MIC parent chain·static switches·실제 selected shader·texture/sampler/mip를 확인한다.
3. component RNM/lightmap pair·UV scale/bias·계수와 배치 identity를 연결한다.
4. 원본 맵 light·hemisphere/ambient·fog·sky/reflection 입력을 연결한다.
5. 물/particle의 blend·depth·distortion·sort·soft intersection을 확인한다.
6. 그 뒤에도 남는 노출·색·bloom·AA·SSAO 차이는 별도 후보로 비교하여 사용자에게 승인받는다.

원본 누락을 exposure/gamma로 덮으면 지형 한 곳이 좋아지는 대신 물·NPC가 망가질 수 있다. 먼저 출력 비교를 재질 단위로 분리한다. 최종 예쁨은 사용자가 판단한다.

## G04. 지형·일반 맵 재질·RNM을 끝까지 연결한다

### G04-01. Landscape의 이전 수정은 보존한다

`Tools/LandscapeExtractor/extract_ue3_landscape.py`의 명시적 `source_layer_contract` 경로는 다음을 이미 반영했다.

- source UV `(sectionBase + local) * 0.1`, half-centred 회전 뒤 tiling.
- rotation scalar 계수 약 `3.1400001049` rad. degrees로 재해석하지 않음.
- sRGB decode 후 linear 공간 bilinear wrap, alpha는 linear.
- HeightBlend `saturate(2*paintWeight - 1 + diffuseAlpha)` 후 정규화.
- diffuse layer02~07과 normal layer02~05/06~07의 서로 다른 blend 방식.
- component static normal enable, luma `.3,.59,.11`.

노랑·초록 tint는 원본에도 있다. 얼룩을 없애겠다며 최대 픽셀 정규화·채도 제거·4배 임의 축소로 다시 바꾸지 않는다. 지금 512px PNG bake만으로 native Landscape 동일성을 주장하지 않는다.

원본 선택 shader가 공용 cache에 없으면 다음 실제 레벨 cache를 사용한다.

- `ReleasePC/9XUFAXIP8BXBAP1NIEG66EF.upk` (**Packages 아래가 아님**).
- ShaderCache export 813 `sc_lv_ocn_eventis_mhp_land01`.
- packed cache는 descriptor와 code blob 개수가 다르다. 기존 `extract_selected_packed_dxbc` 경로 사용.
- 확인 자료: `out/MaharakaReaudit20260926/landscape-level-matches.json`, `landscape-level-map-*.json`, `level_landscape_cache.py`.
- source Landscape 패키지: `ReleasePC/Packages/867STHM7WV6VMSNL7HFG07M83MO5C.upk`.

다음 구현은 대표 컴포넌트 1개를 골라 VS/PS 입력, 원본 mip, RNM, native layer 결과를 대조한 뒤 기존 `CModel/CMaterial`의 source family로 확장한다. 모든 terrain을 한 번에 다시 굽지 않는다. full GPU 경로가 미지원이면 32 PNG는 유지하고 그 경계를 명시한다.

### G04-02. RNM/일반 메시 재질의 실제 작업

1. 원본 배치의 `sourcePlacementId`를 component export와 연결한다. 런타임 vector index를 저장 ID로 쓰지 않는다.
2. `extract_source_map_component_lighting.py`로 실제 logical map package별 RNM을 후보에 추출한다. 모델 UPK가 아니라 **배치 component가 들어 있는 map UPK**가 대상이다.
3. 원본 glTF/WModel의 UV1/UV2, tangent.w, COLOR0와 RNM이 쓰는 채널을 대조한다. 없으면 원본 추출기부터 수정한다. UV0 복제로 통과시키지 않는다.
4. RNM average/directional DDS pair와 원본 mip·colorSpace·scale/bias를 보존한다. 같은 mesh라도 atlas pair가 다르면 variant asset으로 분리한다. 같은 pair의 배치별 차이는 `placementLighting`에 둔다.
5. `build_source_map_materials.py`의 현행 입력 schema로 candidate를 만든다. diffuse/normal만 연결한 행을 RNM complete로 표시하지 않는다.
6. component 정보에서 `source-absent`가 확인된 경우만 baked 없음으로 기록한다. 미추출 상태는 `unresolved`다.
7. 지형/일반 배경/foliage/물/NPC native 재질을 구분한다. 범용 PBR로 일괄 교체하지 않는다.

source map material compiler의 일부 slot 출력으로 현재 329행의 mapmaterials 전체를 대체하지 않는다. compiler가 지원하는 BG/RNM 후보만 `(assetId, materialName)` 기준으로 기존 문서에 stage-merge하고, 기존 water/native 행·사용자 override·미변경 placementLighting은 보존한다. 같은 키가 충돌하면 근거와 diff를 확인하고, 중복 키를 조용히 마지막 값으로 덮지 않는다. 자원 atlas pair 때문에 새 variant가 필요할 때는 관련 catalog·배치 참조·조명 행을 하나의 검증 단위로 바꾼다.

기존 도구의 실제 CLI를 먼저 읽는다. 아래는 도움말 조회라 제품을 바꾸지 않는다.

```powershell
python -B Tools/LevelPlacementExtractor/extract_source_map_component_lighting.py --help
python -B Tools/LevelPlacementExtractor/build_source_map_materials.py --help
python -B Tools/LevelPlacementExtractor/build_source_map_material_inputs.py --help
```

component 추출기의 필수 옵션은 `--package`, `--logical-package`, `--output`, `--receipt`이며 `--area-id`를 지정할 수 있다. **G01에서 확정한 물리·논리 package 쌍을 넣어야 하므로 이 문서는 임의 UPK를 실행 인자로 대입하지 않는다.** exit 2 `PARTIAL_UNSUPPORTED`는 PASS가 아니다.

material compiler 필수 옵션은 `--input`, `--resources-root`, `--output`, `--receipt`다. `Tools/LevelPlacementExtractor/README.md`의 입력 계약을 따른다. `build_source_map_material_inputs.py`는 현재 미커밋 파일이므로 먼저 내용·지원 범위를 확인하고 다른 세션 수정과 조정한다.

새 candidate가 나온 뒤 `build_maptool_scene.py`의 `--source-materials-receipt`, `--materials-output`, `--runtime-asset-root` 연결을 사용해 asset/slot/MIC 일치를 검사한다. area 전체를 초기화해 기존 사용자 배치를 지우지 않는다.

```powershell
python -B -m unittest discover -s Tools/LandscapeExtractor -p test_extract_ue3_landscape.py
if ($LASTEXITCODE -ne 0) { throw 'Landscape 회귀 실패' }
python -B -m unittest discover -s Tools/LevelPlacementExtractor -p test_build_source_map_materials.py
if ($LASTEXITCODE -ne 0) { throw 'Map material 회귀 실패' }
```

G04 종료: 대표 patch의 원본→candidate→설치→실제 slot 소비 연결, 원본 mip·RNM·UV별 coverage, 실패 시 기존 파일 보존 검사를 남긴다. source compiler receipt의 `originalVisualFidelityVerified=false`를 자동으로 true로 바꾸지 않는다.

## G05. 마하라카 전용 scene·상시 효과·물을 보완한다

### G05-01. 전용 scene profile

수정 대상은 `Data/Rendering/Authored/RenderingProfiles.json`, 해당 publisher, `LevelRegistry.cpp`의 MAHARAKA 행이다. 제안 신규 ID는 **`scene.maharaka.source-day.v1`**이며, 아직 존재하는 ID가 아니다.

1. 현재 globalQuality 및 모든 기존 profile/region/quality 값을 값 단위로 백업·비교한다.
2. 현재 유효한 profile 구조를 재사용하여 별도 후보를 만들고 마하라카 원본 fog/light/sky/ambient를 넣는다.
3. Bern의 5개 region과 좌표를 복사하지 않는다. Maharaka의 원본 region이 확인된 경우만 자기 좌표로 구성한다.
4. 원본 UE3 값의 단위·색공간·식이 엔진 입력과 다르면 변환 근거를 남긴다. `extract_ue3_map_lights.py` 결과는 현재 `PROJECT_AUTHORED`로 출력되는 근사도 포함하므로 원본 정확도 PASS로 승격하지 않는다.
5. 팀장이 저장한 FXAA/SSAO/bloom/exposure/gamma/LUT/scene·region quality를 보존한다. 기존 shared development profile도 변경하지 않는다.
6. 원본 scene 입력 복원을 위한 새 profile 후보와 예술적 튜닝 후보를 구분한다. 기존 조율값을 바꾸는 적용은 diff를 보여 주고 승인받는다. “베른이 예쁘다”는 요청이 모든 전역 옵션 강제 활성화 권한은 아니다.
7. 승인된 전용 profile만 MAHARAKA descriptor에 연결하고 Bern/Kouku/Valtan/Development의 profile ID와 값이 불변인지 검사한다.

maplights 보완에서는 원본의 static/baked 기여와 실제 dynamic 기여를 구분한다. RNM을 붙인 뒤 같은 원본 광원을 동적 light로 다시 전부 켜면 빛을 두 번 더할 수 있다. 원본 채널·cast/receive flags·attenuation·scene multiplier가 어느 pass에서 한 번 적용되는지 확인한다. texture decode·exposure·gamma도 여러 pass에서 중복하지 않는다. material normal은 source encoding/채널에 맞게 linear로 읽고, diffuse용 sRGB 설정을 normal/mask/RNM 전체에 일괄 적용하지 않는다.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/RenderingPipeline/Publish-RenderingProfiles.ps1 -Mode Validate
if ($LASTEXITCODE -ne 0) { throw 'Rendering profile 검증 실패' }
```

이 명령은 현행 저장본을 검사한다. 새 후보 파일을 검사할 때는 도구가 실제 지원하는 `-SourcePath`로 후보를 지정한다. 검증 성공은 사용자 색·밝기 승인과 다르다.

### G05-02. 상시 맵 particle

`Level_Development`에는 현재 Bern처럼 `CMapEffectPresentationRuntime::Load_AmbientArea`를 호출하는 경로가 없다. 원본 상시 particle를 복구해 mapeffects JSON만 만드는 것으로 끝내지 않는다.

준비→유효성 검사→Level commit→Update_LevelPresentation→Clear를 Bern의 기존 runtime으로 연결한다. Area는 `LV_OCN_EVENTIS_MHP`만 선택한다. 원본이 level-active인 분수·환경 효과와 action-triggered인 얼굴/물대포를 분리한다. 없는 원본을 이미지에서 상상해 채우지 않는다.

### G05-03. 물 재질

1. 현재 water-42 10행을 출발점으로 original MIC·parent·static parameters·selected VS/PS key를 행별로 확인한다. **같은 ocean_trn parent라는 사실만으로 program42가 모두 정확하다고 단정하지 않는다.**
2. 원본 uniform expression 상수, texture expression index·sampler·colorSpace, camera vector/world normal, time, wave/WPO, reflection cube/sky·Fresnel·depthfade·distortion을 실제 소비 식과 대조한다.
3. water41→42는 상수 배열 길이도 다르다. 기존 41의 13개 상수를 42의 20개 슬롯에 순서대로 복사하지 않는다. 이름과 expression 근거로 pack한다.
4. 겹치는 수면, depth-write, translucent sort, scene color/depth 입력, 정상적인 물 속/위 clip을 점검한다. 물이 불투명 파랑이라고 alpha만 낮추지 않는다.
5. 한 수면에서 canary를 검증한 뒤 강/슬라이드/바다/river-rock에 각각 적용한다. 같은 겉모양이라도 원본 permutation이 다르면 나눈다.
6. 현재 59배치의 SelfMotion 합성 수정은 보존한다. 물 파형 시간과 맵 회전 시간을 혼용하지 않는다.

G05 종료: 전용 scene 후보와 승인 범위, 원본 상시 효과 수·소비자, 물 행별 정확/근사/미해결 목록. 사용자 화면 판정 전에는 “원작과 똑같음”이라고 하지 않는다.

## G06. 얼굴·물줄기·지면·종료 particle의 실제 원본을 만든다

### G06-01. 추출과 제품 생성은 별개다

다음은 원본 그래프를 새 후보 폴더에 추출하는 명령이다. 게임 UI를 열지 않는다.

```powershell
python -B Tools/LevelPlacementExtractor/extract_ue3_particle_graph.py `
    --umodel $MhpUmodel --package-root $MhpPackageRoot --region kr `
    --output (Join-Path $MhpRun 'MokoParticles') FX_MN_ISMP_00
if ($LASTEXITCODE -ne 0) { throw '원본 particle graph 추출 실패' }
```

이 명령은 `.particle-graph.json`과 manifest를 만들 뿐, runtime EffectCatalog와 실행 코드를 자동으로 끝내지 않는다. 논리 package resolve 실패 시 전체 index에서 찾은 정확한 쌍만 `--physical-package LOGICAL=ABSOLUTE_PATH`로 제공한다.

### G06-02. 순차 구현

1. G01의 4종 particle 각각의 emitter, LOD, required/spawn/burst/lifetime/velocity/size/color/rotation/subimage/local-space/beam/ribbon/mesh/event modules를 전부 inventory로 만든다.
2. `extract_ue3_particle_module_closure.py`와 재질 closure 도구의 실제 CLI/지원 범위를 읽는다. 이름이 Artist/Valtan 전용인 materialize 스크립트를 Maharaka에 그대로 실행하지 않는다.
3. 기존 EffectCatalog·source particle compiler·V1 carrier·CEffectPresentationService가 지원하는 module은 동일 실행 경로로 변환한다. 미지원 module은 현재 소비자에 최소 확장하고 단위검사를 붙인다. 두 번째 particle renderer를 만들지 않는다.
4. 원본 material의 selected program·dynamic parameter·texture·blend/depth/cull·distortion 입력을 복구한다. 물 particle를 흰 sprite로 대체한 뒤 완료라고 하지 않는다.
5. 원본 `FX_01`의 SkeletalMeshSocket export/LookInfo attachment를 찾아 **socket → parent bone → local transform**을 복구한다. 현재 glTF node에는 `FX_01`이라는 bone이 없다. 이름 그대로 bone에 넣거나 못 찾았다고 root를 선택하면 안 된다.
6. socket 위치·방향을 `CModel` pose와 결합한다. engine 행/열 convention, UE→engine 축, centimetre→metre, NPC preScale/ModelSize가 각각 정확히 한 번만 적용되는지 검사한다.
7. born-at-world와 계속-follow, translation-only와 rotation-follow, local simulation과 world simulation을 나눈다. 캐논이 회전할 때 이미 발사된 물이 같이 돌아야 하는지는 source module이 정한다.
8. ground effect는 원본 위치/방향/지면 projection 정책을 따로 복구한다. 화면 피킹 위치나 플레이어 위치에 붙이지 않는다.
9. `PetEmotion`, `opacity_intensity`, finish `Color_1/Alpha_1`은 source serialized 값·곡선·대상 material을 해석해 **해당 occurrence clone**에만 적용한다. shared prototype 상수를 바꾸지 않는다. stop/reset 시 원래 값으로 되돌린다.
10. 원본 duration=0·tail·emitter loop·stop spawning와 kill-all의 차이를 보존한다. 화면 밖에서 초기화되지 않아 정지 후 물이 남는 실패도 검사한다.

제품 NPC 재질은 기존 `CNpc::Get_Model()` → `CModel::Override_SourceCharacterConstants()` 경로를 재사용한다. `Engine/Private/Model.cpp`는 material이 공유 중이면 `Clone_ForOverrides()`한다. 다만 현재 setter는 material name **fragment**로 매칭하므로, 호출 전에 예상 slot 목록을 대조하고 반환 matched count가 정확한지 확인한다. 서로 비슷한 이름의 slot을 잘못 함께 바꾸지 않는다. `CWorldSequencePlayer::Set_ObjectMaterialConstants()`는 WorldObject용이며 제품 CNpc에 그대로 적용하는 API가 아니다.

G06 종료: 4225601의 4종 particle 및 decal/material parameter의 명시 coverage, socket 근거, deterministic seed 정책, 잘못된 resource/slot/socket 거부와 rollback 검사. 이 단계만으로 자동 행동 제품 연결 완료는 아니다.

## G07. 사운드를 조건과 시간까지 함께 복구한다

### G07-01. 실제 원본 이벤트

full source event는 `S_Mob_MococoWater1.MococoWater1_Attack01_Cast1`, `...MococoWater1_Attack01_Shot1` 및 **같은 package의** Water2 변형이다. `EFDLChar_MN_ISMP_00-1.MN_ISMP_00-1` 조건을 해석한 뒤 알맞은 한 쪽을 고른다.

```powershell
python -B Tools/SoundPipeline/wwise_audio_package.py `
    --package-root 'C:\ProgramData\Smilegate\Games\LOSTARK\EFGame' `
    --filter MococoWater --list
if ($LASTEXITCODE -ne 0) { throw '검색 범위/이름을 확인하십시오. 원본 삭제로 결론 내리지 마십시오.' }
```

목록이 실제 어느 package를 가리키는지 확인한 뒤 event를 resolve한다. Wwise event object의 short name/hash를 확인하고 leaf name을 사용한다. 이 도구의 hash는 FNV1-32이며 FNV1a로 바꾸지 않는다.

```powershell
python -B Tools/SoundPipeline/wwise_audio_package.py `
    --package-root 'C:\ProgramData\Smilegate\Games\LOSTARK\EFGame' `
    --filter MococoWater `
    --event MococoWater1_Attack01_Cast1 --event MococoWater1_Attack01_Shot1 `
    --event MococoWater2_Attack01_Cast1 --event MococoWater2_Attack01_Shot1 `
    --extract (Join-Path $MhpRun 'MokoSoundWem')
if ($LASTEXITCODE -ne 0) { throw 'Wwise 추출 실패' }
```

**중요:** 현재 도구는 event가 `EVENT NOT FOUND`여도 최종 exit 0일 수 있다. 위 exit 검사만으로 PASS하지 않는다. 각 event의 sources/unresolved/실제 stream 파일을 별도로 assert하는 검사를 작성한다. Stop-only event가 소리 파일 0개인 것은 정상일 수 있으며 stop action을 따로 해석한다.

### G07-02. 제품 사운드 계약

1. HIRC Event→Action→Random/Sequence/Switch container→Sound→WEM을 따라간다. `resolve_event`는 Play 중심이므로 Stop/RTPC/attenuation/loop/gain을 별도 확인한다.
2. WEM을 기존 `wwise_vorbis_to_ogg.py` 및 저장소의 해당 codec 경로로 변환한다. 모든 WEM이 Vorbis라고 가정하지 않는다. source format·길이·채널·sample rate를 확인한다.
3. `Data/Sound/CharacterSoundCatalog.json` 및 `CSoundCueCatalog`의 기존 Resources-relative Sound 경로 계약을 사용한다. 실제 현재 Sound root와 패키징 규칙을 확인하고 임의 새 root를 만들지 않는다.
4. equal weight/avoid-repeat/switch를 source 근거가 있는 만큼 보존한다. variant 목록에 적힌 모든 파일을 동시에 틀지 않는다.
5. G08의 같은 occurrence clock에서 cast=0, shot=2.2000000477초를 소비한다. 매 프레임 재생하지 않는다. 늦은 입장 때 지난 one-shot을 몰아서 재생하지 않는다.
6. loop는 owner/occurrence handle로 보유해 cancel/actor despawn/area leave에서 끝낸다. 원본 범위에 맞는 2D/3D 및 거리 감쇠를 사용하고 임의 전역 음량 증폭을 하지 않는다.
7. `CNpc::Arm_HitReactionSound`의 기존 타 몬스터 5개 하드코딩 배열에 이 행동 소리를 덧붙이지 않는다. 데이터 기반 source event 경계로 연결한다.

G07 종료: event별 variant 조건·시각·음량/loop/stop 의미·resource resolve 검사. 청취와 화면 동기화의 최종 판단은 사용자가 한다.

## G08. CNpc에 정확한 occurrence·clock·attachment를 연결한다

### G08-01. 현재 코드에서 확인된 보완점

| 파일·함수 | 현재 동작 | 이번 구현에서 필요한 조치 |
|---|---|---|
| `NpcActionEffectCueDocument.cpp::Load` | parse 전 빈 cache 삽입, 없는 EffectCatalog cue는 continue | 없는 선택 문서와 잘못된 필수 문서를 분리. 실패가 다음 호출에서 빈 성공으로 바뀌지 않게 stage/commit. missing cue 수·이유 보존 |
| 같은 파일 `ReadUInt` | fractional number를 uint로 cast | ms는 유한한 정수인지 검사. 중복 ID·범위·sort·source 연결도 검사 |
| `Npc.cpp::Arm_ActionEffectCues` | clip 이름으로 arm, 첫 arm 때 effect 준비 queue | 재사용 clip에 여러 action이 붙는 경우 stable action/occurrence로 구분. 첫 frame cue 전에 loader/preparation에서 필수 리소스 준비 |
| `Npc.cpp::Update_ActionEffectCues` | `fTimeDelta`를 누적하여 spawn | pose/source clock과 동기화. seek/속도/cancel/late join/긴 frame을 처리 |
| 같은 함수의 spawn descriptor | NPC RootWorld·owner·initial sample time만 설정 | 읽어 둔 `bone`, `followBone`, `durationMs`가 여기서 직접 소비되지 않음. 실제 effect document 경로까지 추적하고 end-to-end 적용 검사 |
| 같은 함수의 반환 `EFFECT_WORLD_ROOT_HANDLE` | 지역변수로 받은 뒤 보관하지 않음 | actor/occurrence별로 보유하여 seek·cancel·leave에 명시적으로 정리 |
| `EffectV2_Runtime.cpp::Resolve_Archetype` | 이미 CActorCatalog NPC를 조회 | 새 NPC 이름별 하드코딩 resolver는 필요 없음. 기존 stable identity를 소비 |

현재 JSON 형식 `lostark.npc-action-effect-cues` version1은 `cueId/effectAssetId/clip/startMs/durationMs/bone/followBone`을 읽는다. **여기에 sound·action ID·socket transform을 임의 필드로 써 놓는 것만으로 실행되지 않는다.** version 확장이 필요하면 reader, validator/publisher, 소비자, 기존 v1 호환, 잘못된 version 거부 검사를 같은 변경으로 작성한다.

### G08-02. 소유권과 데이터 흐름

```text
원본 Trigger/AI/Action 해석 결과
  → 저작 데이터 검증·게시
  → Server: world/actor/action 발생 ID·시작 tick·상태·회전 권위
  → Shared typed state/event
  → ClientReplication: 기존 CNpc를 stable entity ID로 찾아 연출 전달
  → CModel pose + EffectPresentation + SoundCue + clone material parameters
     모두 같은 occurrence의 원본 시각을 샘플
```

이것은 **연결할 목표 구조**다. 현재 Maharaka idle에 위 서버 이벤트가 전부 구현되어 있다는 뜻이 아니다. 기존 world entity/NPC 행동·회전 message에 필요한 값이 있는지 먼저 확인해 재사용한다. 부족한 부분만 typed 계약으로 확장한다. 서버에는 asset path·clip name·Client pointer를 보내지 않는다.

각 occurrence가 필요한 의미는 다음과 같다. 실제 새 struct 이름은 호출자를 확인한 뒤 PLAN에서 정한다.

| 의미 | owner·불변식 |
|---|---|
| World/actor stable identity | Server 발급 entity와 world. ordinal/vector index를 저장 ID로 사용하지 않음 |
| action ID와 source revision | 게시 데이터에 있는 행동과 버전. 미등록 ID는 정상 idle로 위장하지 않음 |
| occurrence sequence | Server가 새 행동마다 증가. 같은 snapshot 재수신으로 재시작하지 않음 |
| start tick·playback phase | pose/effect/sound의 공통 기준. 렌더 frame 수와 별개 |
| presentation handles | Client가 소유. 해당 occurrence 종료/취소/leave에서 해제 |
| socket transform | 원본 socket local → animated bone → NPC world. scale/axis 변환 한 번 |

### G08-03. 구현 순서

1. 첫 동작 4225601을 **원본 스케줄과 분리된 명시적 검증 명령**으로 재생 가능하게 한다. 기존 F1/Workbench의 typed command 경계를 확장한다. 없는 메뉴를 있다고 사용자에게 안내하지 않는다.
2. 먼저 준비된 source clip+4 particles+2 sound notifies+material params를 한 시계로 재생한다. 미해결 decal은 기능 미완료로 남긴다.
3. cue crossing은 이전 시각과 현재 시각의 구간으로 계산한다. 시작 0초도 빠뜨리지 않고, frame overshoot는 source sample age로 시작한다. 이미 수명이 지난 burst를 새로 살리지 않는다.
4. 동기화로 시간이 뒤로 가는 경우 one-shot 중복, 정상 루프의 다음 occurrence, 명시 preview seek를 분리한다. paused clock에서 sound만 진행하지 않게 한다.
5. source attachment/lifetime을 기존 effect runtime의 검증된 binding·handle API로 전달한다. 해당 API가 없으면 기존 consumer를 확장한다. 별도 “MaharakaEffectManager”를 만들어 이중 재생하지 않는다.
6. 실패 시 기존 모델·idle·다른 NPC를 보존한다. 필수 연출 묶음은 부분 spawn 후 성공으로 처리하지 않고 후보를 정리한다. 게임 접속 자체를 막을지 해당 presentation만 격리할지는 기존 domain 정책을 따른다.
7. 첫 행동이 닫히면 4225603/4 등의 정·역회전, 빠른 회전, 물 분사 변형으로 확대한다. action 이름의 `_800/_500`만으로 속도·범위를 정하지 않는다.
8. `NpcAiStateAction/NpcSignal/TriggerMapData`의 실제 조건·stage 전이를 복구해 자동 스케줄을 연결한다. AS020/030/040/050 및 카메라 참조 AS010/070/100을 이름만 보고 일대일 매핑하지 않는다.
9. scene actor의 회전은 base transform에 source key/각속도를 평가한다. 프레임마다 누적 회전하여 drift를 만들지 않는다. 자기축과 공전축, pivot, parent transform을 분리한다.

재사용할 실제 effect API는 `Client/Public/Effect_PresentationService.h`의 `Spawn_LevelPlacement`, `Update_WorldRoot`, `Seek_WorldRoot`, `Stop_WorldRoot`다. RootWorld/transform provider/source sample/end/parameter가 전달되는 기존 WorldSequence 소비 예제를 함께 읽는다. spawn에서 받은 handle을 occurrence 상태가 소유하고, 정상 tail과 명시 kill을 구분해 해제한다. 별도의 spawn 경로를 만들거나 actor 포인터 주소를 persistent ID로 저장하지 않는다.

G08 종료: 동일 occurrence 중복 수신, 30/60/144Hz 및 긴 frame, 늦은 입장, cancel, respawn, area leave, resource 미준비, 잘못된 socket/revision에 대한 실행형 검사. preview 성공과 서버 제품 자동 실행 성공을 따로 기록한다.

## G09. 워터팡 무대를 기존 WorldSequence·파괴 경로에 연결한다

G02 모델/조각/상태 연결이 확정된 부분만 시작한다. unknown 무대 모델을 `MN_ISTM_00`으로 채우지 않는다.

### G09-01. 모델과 회전

1. 현재 맵 catalog에 이미 존재하는 mesh인지 먼저 확인한다. 기존 배치를 재사용할 수 있으면 새 중복 모델을 생성하지 않는다.
2. 원본 별도 SkeletalMesh라면 기존 `CModel` animation, InterpActor라면 source transform track, 여러 Prop면 source parent/pivot·조각별 track을 사용한다. 이 셋을 이름만 보고 혼용하지 않는다.
3. 각 조각의 original local pose→source parent→world 변환과 visibility를 정본으로 저장한다. 맵 중앙이나 사용자 위치를 임시 원점으로 두지 않는다.
4. `CWorldSequencePlayer`를 사용하여 정확한 시작 위치·clock·회전·effect·sound를 한 instance로 샘플한다. 기존 `WorldSequencePlayer_Objects.cpp`의 Apply_ObjectEffects/Apply_Sounds와 tail/stop 기능을 재사용한다.
5. 제품에서 NPC가 소유하는 모코모코를 WorldObject가 또 그리지 않는다. 명시 preview와 제품 actor의 소유권을 분리하고 동일 source 데이터만 공유한다.

### G09-02. 붕괴와 복구

1. source `shake → destroy → repair`의 조건·delay·조각 선택을 실제 trigger/event로 복구한다. 이것이 문서상의 이름 순서와 같다고 가정하지 않는다.
2. 원본이 intact/broken mesh 전환인지, bone animation인지, fractured chunks인지 확인한다. 확인된 방식의 geometry·재질·속도/회전·visibility를 사용한다.
3. 현재 제품 예시는 `Server/Private/GameRoom_WorldDestruction.cpp`, `WorldDestructionRuntime.cpp`, Client `WorldDestructionProjectionRuntime`/debris 경로다. 오래된 문서의 “파괴는 전부 debug-only” 설명보다 현재 제품 코드를 확인한다.
4. `Tools/WorldPipeline/Publish-ValtanWorldDestruction.ps1`은 **Valtan 전용**이다. 존재하지 않는 `-WorldId MAHARAKA` 옵션을 붙여 호출하지 않는다. 공용화가 필요하면 현재 검증된 Valtan 입력·출력을 그대로 보존하는 scoped 확장과 회귀검사를 만든다.
5. 원본 물리 초기값이 없는데 프로젝트 임의 debris impulse를 쓰면 `PROJECT_AUTHORED approximation`으로 분리한다. Valtan의 격자 조각을 복사한 것을 원본 무대 붕괴라고 하지 않는다.
6. 무대가 실제 발판이라면 렌더 hide만으로 끝내지 않는다. 같은 Server event에서 support/collision/navigation 상태가 바뀌어야 한다. 플레이어가 투명 바닥에 서거나 클라이언트별로 낙사 판정이 갈리지 않게 한다.
7. repair는 initial pose/visibility/support를 한 세대의 상태로 복구한다. 파괴·복구 중 늦게 접속해도 전체 state snapshot으로 현재 상태를 복원한다. 과거 one-shot 연출은 다시 틀지 않는다.
8. 테스트용 붕괴 명령은 명확히 debug audition으로 표시한다. 실제 original signal 연결을 끝내기 전 “게임처럼 자동 작동”이라고 보고하지 않는다.

G09 종료: 조각별 source ID와 모델, 회전축·clock, destroy/repair 상태, Server support와 Client visibility 동시성, 중복 event/late join/재입장 회귀. 원본 모델 미확정 조각은 남은 목록으로 유지한다.

## G10. Loader·제품 Area·정리 경로를 닫는다

대상: `Client/Private/Level_Development.cpp`, 해당 header, 현재 Loader의 MAHARAKA 분기, 기존 replication/event consumer, `CWorldSequencePlayer`.

1. 현재 MAHARAKA의 Map/SelfMotion/MapLight/replication 생성 순서를 읽는다. 다른 Development 시나리오의 동작을 바꾸지 않는다.
2. 새 WorldSequence가 있으면 Loader에서 `CWorldSequencePlayer::Prepare_AreaLoad`로 검증·리소스를 stage한다. 제품 activation은 `Load_PreparedArea`를 소비한다. 실패했다고 frame 중 `Load_Area` 동기 I/O로 우회하지 않는다.
3. NPC source effects도 첫 cast 전에 product targets를 준비한다. 준비 완료 전에 server action이 도착하면 원본 clock을 보존한 pending/격리 정책으로 처리하고 이유를 기록한다. 임의 1초 sleep 후 성공 처리하지 않는다.
4. MapLight·AmbientEffect·WorldSequence·NPC cue owner를 명확히 하나씩 둔다. 어느 파일이 Prepare/Commit/Update/Clear하는지 RESULT에 적는다.
5. Area leave, disconnect, loader cancel, failed activation, actor despawn에서 sound/particle/clone/material override를 정리한다. 다른 Area의 global state를 초기화하지 않는다.
6. 신규 authoring이 있어도 publisher/runtime consumer가 없다면 설치 완료가 아니다. Data/Map/World/Effect/Sound/Rendering 각 domain의 로드 경로를 실제 호출자로 증명한다.

## G11. 게시·빌드·사용자 검증·배포

### G11-01. 검증부터 한다

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_OCN_EVENTIS_MHP -Mode Validate -Scope Area
if ($LASTEXITCODE -ne 0) { throw 'Maharaka Map Validate 실패' }
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/WorldPipeline/Publish-WorldGameplay.ps1 -WorldId MAHARAKA -Mode Validate
if ($LASTEXITCODE -ne 0) { throw 'Maharaka World Validate 실패' }
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/RenderingPipeline/Publish-RenderingProfiles.ps1 -Mode Validate
if ($LASTEXITCODE -ne 0) { throw 'Rendering Validate 실패' }
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/EffectPipeline/Validate-EffectSources.ps1 -RepositoryRoot $MhpRepo -ResourceRoot (Join-Path $MhpRepo 'Client\Bin\Resources')
if ($LASTEXITCODE -ne 0) { throw 'Effect source 검증 실패: 누락과 이번 변경 회귀를 분리하십시오.' }
```

위 도구 외에 새 action/sound/socket/state 계약을 검사하는 focused harness가 필요하다. EffectSources 전체 검사에서 무관한 기존 resource 누락이 나오면 이를 PASS로 바꾸거나 validator를 완화하지 말고 기존 실패와 이번 변경을 분리한다. preview 허용 스위치를 제품 admission 대용으로 쓰지 않는다.

새 테스트의 필수 사례:

- 정상 source action/variant/socket, version/ID/path 오류, 중복 cue, NaN·fractional ms.
- 누락 texture/clip/sound/material slot/socket, catalog cache 실패 후 재시도 정책.
- 0초 notify, overshoot, 수명이 지난 burst, pause/seek/speed, 동일 occurrence 중복.
- 두 NPC 동시 재생 시 prototype material·sound handle 공유 오염 없음.
- 준비 중 취소·중간 파일 교체 실패·일부 모델 생성 실패 rollback.
- stage 파괴/repair와 Server support, late join snapshot, 재입장 clean state.
- 기존 Bern 렌더링/선박·Kouku/Valtan cue와 world destruction 회귀.

### G11-02. 승인된 저작 변경만 게시한다

아래는 실제 제품 생성물을 바꾼다. source와 Resources 준비가 끝났고 편집 중인 값이 저장되어 있는지 확인한 뒤, **변경한 domain만** 실행한다. 생성 bootstrap/RuntimeData를 손으로 수정하지 않는다.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_OCN_EVENTIS_MHP -Mode Publish -Scope Area
if ($LASTEXITCODE -ne 0) { throw 'Maharaka Map Publish 실패' }
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_OCN_EVENTIS_MHP -Mode Check -Scope Area
if ($LASTEXITCODE -ne 0) { throw 'Maharaka runtime/source 불일치' }
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/WorldPipeline/Publish-WorldGameplay.ps1 -WorldId MAHARAKA -Mode Publish
if ($LASTEXITCODE -ne 0) { throw 'Maharaka World Publish 실패' }
```

Rendering을 변경한 경우에만 승인 후 다음을 실행한다.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/RenderingPipeline/Publish-RenderingProfiles.ps1 -Mode Publish
if ($LASTEXITCODE -ne 0) { throw 'Rendering Publish 실패' }
```

현재 `Publish-MapAuthoring`의 scope는 `Area/WorldSequences/Lights/Deploy/Placements/CameraShots`다. 존재하지 않는 `Materials` scope를 만들었다고 가정하지 않는다. effects/sound/cue의 추가 출력은 실제 현행 publisher 경로를 확인해 함께 닫는다. 위 세 명령이 모든 새 domain을 자동 게시한다고 가정하지 않는다.

### G11-03. 빌드와 실행 경계

실제 이번 빌드 대상 EXE/DLL의 링크·교체가 실행 프로세스의 파일 점유로 막히는 경우에만 사용자에게 저장·종료를 요청한다. 존재하는 모든 Client/Server, 다른 checkout의 실행본, 팀 공유 Server를 종료 대상으로 삼지 않는다. 데이터 검사·게시에도 항상 종료가 필요한 것처럼 확대하지 않는다. 임의 강제 종료하지 않고 VS와 동시 빌드하지 않는다. 정본 Product 증분 경로를 사용한다.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug
if ($LASTEXITCODE -ne 0) { throw 'Debug Product 빌드/회귀 실패' }
git diff --check
```

Release 제품 계약을 바꿨으면 Debug 검증 후 같은 명령의 `-Configuration Release`도 검증한다. 기본으로 Clean/Rebuild/FullDiagnostic/SkipBuild를 사용하지 않는다. **기본 Product는 protocol·Server contract harness를 실행하지 않는다.** Shared/Server를 수정하면 관련 focused 검사를 별도로 빌드·실행해야 한다. 새 C++ 파일이 필요하면 `.vcxproj/.filters` 등록을 같이 검사한다.

Shared packet을 바꾼 경우 다음을 현재 Product와 **같은 toolchain**의 Developer PowerShell에서 실행한다. MSBuild가 PATH에 없으면 이번 Product receipt의 `toolchain.msbuildPath`를 확인해 정확한 경로를 사용한다. 임의 다른 VS 버전을 선택하지 않는다.

```powershell
$MhpMSBuild = (Get-Command MSBuild.exe -ErrorAction Stop).Source
& $MhpMSBuild 'Tools\NetworkProtocolHarness\Default\NetworkProtocolHarness.vcxproj' /t:Build /p:Configuration=Debug /p:Platform=x64 /m /v:minimal
if ($LASTEXITCODE -ne 0) { throw 'Protocol harness 빌드 실패' }
& '.\Tools\NetworkProtocolHarness\Bin\Debug\NetworkProtocolHarness.exe'
if ($LASTEXITCODE -ne 0) { throw 'Protocol contract 실패' }
```

Server World playback/support를 변경한 경우, 방금 빌드한 Server로 관련 기존 회귀를 실행한다. 다음은 게임 서버 listener를 여는 일반 실행이 아니라 `Main.cpp`가 분기하는 headless contract 명령이다.

```powershell
& '.\Server\Bin\Debug\Server.exe' --world-playback-contract-test
if ($LASTEXITCODE -ne 0) { throw 'World playback contract 실패' }
& '.\Server\Bin\Debug\Server.exe' --valtan-arena-support-contract-test
if ($LASTEXITCODE -ne 0) { throw '기존 Valtan support 회귀 실패' }
```

이 기존 검사가 새 Maharaka action/파괴 상태를 자동 검사하지는 않는다. G08/G09 테스트를 실제 Server contract runner에 등록하고, 새 focused 진입점을 구현했다면 그 **실제 옵션**과 실행 결과를 RESULT에 추가한다. 필요 시 기존 `--contract-test` 전체 진입점으로 새 등록 테스트의 실행을 확인할 수 있으나, 광역 FullDiagnostic 빌드를 기본 요구로 바꾸지는 않는다.

빌드 후 `out/BuildPipeline/runs/`의 이번 receipt, 실제 exe 경로·수정 시각, 게시한 domain·source revision을 기록한다. 이전 성공 로그를 이번 변경의 PASS로 재사용하지 않는다. 기존 무관한 whitespace 오류가 있으면 위치를 분리해 보고하고 정리하지 않는다.

에이전트는 Client/UI를 실행·조작·캡처하지 않는다. 실행 준비 뒤 사용자에게 현재 Server/Client 상태와 아래 확인 경로를 안내한다. F5/Ctrl+F5는 VS 설정에 따라 빌드를 수행할 수 있으므로 “빌드 없이 실행”이라고 단정하지 않는다.

### G11-04. 사용자 확인표

실행 전에 `Tools/Network/TeamLanEndpoint.json`, 현재 debugger override와 실행 상태를 읽고 **실제 접속 endpoint와 이 PC의 역할**을 확인한다. 문서 조사 시 팀 endpoint는 `192.168.0.22:7777`였다. 팀 서버 호스트라면 사용자가 `Server + Client`, 팀 클라이언트라면 Client만 실행한다. 사용자가 명시한 로컬 override로 테스트 중이라면 그 설정을 보존하고 필요한 로컬 Server 실행 상태를 안내한다. 이 복원 작업 때문에 endpoint를 자동 변경하지 않는다. 그다음 Lobby → Maharaka로 진입한다. 기능별 정확한 메뉴 이름은 구현한 코드에서 확인해 보고한다. 현재 없는 “Maharaka 재생 메뉴”가 있다고 안내하지 않는다.

| 순서 | 사용자가 볼 것 | 코드·로그에서 먼저 확인할 것 |
|---|---|---|
| 1 | 이전 얼룩 구간, 평지/길/잔디/절벽 | 실제 source slot·RNM·mip가 설치 파일로 resolve되는지 |
| 2 | 바다·슬라이드·river-rock의 색·반사·흰 덩어리·수면 중첩 | 행별 shader/permutation·texture·scene-depth 입력 |
| 3 | 모코모코/캐논 크기·방향·idle | actor100/188만 한 번 생성, model/scale identity |
| 4 | action4225601: 얼굴→물→지면→종료, cast/shot 소리 | 위 시간표와 공통 occurrence clock, 선택된 variant |
| 5 | 정·역회전 중 물의 시작점과 이미 발사된 물의 경로 | socket와 source local/world simulation |
| 6 | 워터팡 무대의 대기→회전→흔들림→붕괴→복구 | 원본 signal 순서, 조각별 state, Server support |
| 7 | 재입장·행동 중 나가기·다른 클라이언트 늦게 입장 | tail/loop/clone 누수 없음, 같은 현재 상태 |
| 8 | 기존 Bern의 화면·선박/Valtan/Kouku | 이번 작업이 기존 profile와 공용 cue를 훼손하지 않았는지 |

한 번에 “예쁜가요?”만 묻지 않는다. 위치·시각·대상·기대 동작을 지정해서 사용자가 비교할 수 있게 한다. 원본 비교 영상과 현재 패키지 연도가 다른 부분은 버전 차이로 분리한다.

### G11-05. Git과 Drive의 구분

- Git: 변경 C++/shader/Tools, 정본 JSON/schema, 검증된 runtime DataFiles, 대응 PLAN/RESULT 및 필요한 팀 계약.
- 별도 Resources 배포: 실제 새/변경 WModel, texture/mip/RNM, particle mesh/texture, sound와 이들이 참조하는 공용 입력. **Resources 상대 경로를 그대로 유지**한다.
- 기존 추가 대상: `Map/LV_OCN_EVENTIS_MHP_LAND/Landscape/<asset>/textures/`의 32 PNG, `Character/NPC/Maharaka/`의 모델·텍스처와 사용 중 공용 참조.
- 새 무대·sound·effects는 실제 참조 closure로 목록을 늘린다. 확장자만 보고 전체 폴더를 복사하거나 반대로 “C++만 바꿨다”며 resource를 누락하지 않는다.
- exe/obj/pdb/EngineSDK/원본 UPK·LPK·복호화 DB/작업 staging 전체는 소스 PR에 넣지 않는다.
- push/PR/Drive 업로드는 사용자가 요청한 경우에만 수행한다. 이 설계서 자체는 그 작업을 지시하지 않는다.

## G12. 단계별 결과를 넘기는 형식

각 G의 대응 RESULT에 다음 순서로 남긴다.

1. 이번 G에서 실제 바꾼 파일·함수·데이터 행과 기존 사용자 변경 보존 여부.
2. 원본 근거: package/object/export/offset/ID와 source 연결표.
3. 추출 완료 / 후보 생성 / live 설치 / 게시 / 빌드 / 제품 소비자 연결 상태를 각각 표시.
4. 실행한 명령, exit code, focused test 개수와 실패 목록, build/publish receipt.
5. 사용자에게 확인받은 항목과 아직 확인받지 않은 항목.
6. 미해결 입력의 정확한 이름·다음 검색 위치. “없음/삭제” 대신 확인된 검색 범위를 적음.
7. 다음 G를 시작해도 되는 근거. 의존 G가 막힌 항목과 독립적으로 진행할 항목 분리.

최종 완료 조건은 `원본 연결 + 실제 consumer + 검증 + 사용자 관찰`이다. CLI exit0, PNG 정상, WModel 생성, material EXACT, 빌드 성공 중 하나만으로 전체 복원을 완료 처리하지 않는다.

이 인계서를 작성한 턴에서는 **게임 코드·리소스·저작 데이터·실행 상태를 변경하지 않았다.** 현재 호출자와 원본 증거를 조사해 다음 작업의 설계를 구체화했다.
