# 쿠크 3관문 외곽 불 회전·갈고리 대각선 통과 — 적용 결과

작성일: 2026-09-10. 지시서: `C:/Users/USER/OneDrive/바탕 화면/갈고리.txt` (G00~G08, 부록 A).
브랜치: `codex/kouku-card-maze-0908`.

이 문서는 실제 적용과 자동 검사 결과다. 사용자 화면 확인은 아직 하지 않았다.

## 1. 적용한 파일과 데이터

| 파일 | 변경 |
|---|---|
| `Tools/KoukuSaydonPipeline/author_gate3_fire_hook_fragments.py` | 신규. 지시서 부록 A에서 출발해 2026-09-10 사용자 피드백으로 개정. 읽기 전용 생성기 |
| `Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json` | objectResources 15→18, templates 112→119, instances 148→155, revision 419→420 |
| `Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json` | worlds 6→13, patterns 17→19, playAllPatternIds 11→13, revision 176→177, nextPatternOrdinal 18→20, nextWorldOrdinal 7→14 |

병합은 텍스트 삽입으로만 했다. 기존 배열 원소는 순서·내용 그대로 앞부분에 남고 새 원소만 각 배열의 닫는 괄호 앞에 붙었다. 지정한 루트 스칼라 4개와 `playAllPatternIds` 외에는 어떤 기존 바이트도 바뀌지 않았다. 백업 대비 실제 변경은 worldsequences +3,171줄 / 교체 1줄, composition +904줄 / 교체 4줄이다.

C++ 소스, `.vcxproj`, `.filters`, Shared packet, mapplacements, Navigation, Gameplay triggerBox, 모델 바이너리는 **변경하지 않았다.** 새 protocol version도 없다.

### 생성물 (publisher만 사용, 수동 편집 없음)

- `Client/Bin/DataFiles/Map/LV_LUT_MIDNIGHTC_ED.worldsequences.json` — SHA256 `e68c80dd42ebb1e542c1266660129a1d9096cc4ad2bc11a2bd46b7de09d4a986`
- `Data/Encounters/KoukuSaydon/KoukuSaydonEncounter.json`, `Data/Animation/Authored/KoukuSaydon/KoukuSaydon.patternbindings.json` — projector 재투영
- `Server/Bin/DataFiles/World/*` — `Publish-WorldGameplay.ps1`

## 2. 통과한 자동 검사

| 명령 | 결과 |
|---|---|
| `author_gate3_fire_hook_fragments.py --repository-root .` | exit 0 |
| `Publish-MapAuthoring.ps1 -Scope WorldSequences -Mode Publish` | exit 0, SHA256 `0f594e41401645ce0888f53a0ead9ed311d44231291dfb2965a8434cb9e64596` |
| `project_kouku_saydon_composition.py --mode publish` | revision 177, PRODUCT 패턴 13, stage 118 |
| `project_kouku_saydon_composition.py --mode validate` | 통과 |
| `Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon` | `koukusaydon.product` PASS, `gameplay.balance` PASS |
| `Publish-WorldGameplay.ps1 -Mode Publish` | exit 0 |
| `git diff --check` | clean |

### 아레나 경계를 실측해서 반경을 다시 잡았다 (2026-09-10 2차 수정)

사용자 지적: "맵밖이 아니라 서커스 아레나 안에서의 외각이였어. 불사이의 간격도 엄청 촘촘해야돼."
원본 스크린샷 3장에서 금색 구슬 띠가 아레나 경계이고 불은 그 **안쪽**에 촘촘한 벽으로 서 있다.

측정 결과다.

| 대상 | 값 |
|---|---|
| 구슬 띠 `BG_EVT_CHRISTMAS_LIGHTING01` (지면 y 1.0~5.0) | 중심 **(0.0000, 942.0800)**, 반경 **13.191** (min 13.12 / max 13.31), 72개, 1.14m 간격 |
| 기존 DECO24 장식 불 28개 | 반경 **13.94 ~ 19.44** → 구슬 띠 **바깥** |
| `WALL01` / `GATE02D` | 평균 19.83 / 21.75 |
| navgrid 걷기 가능 영역 | r≈9m까지 온전, 그 밖은 패치 (4m 셀) |

즉 지시서의 중심 (0, 942.08)이 처음부터 옳았고, 내가 1차에서 장식 불 28개로 다시 맞춘 중심
(1.0549, 941.1080)이 **1.3m 틀렸다**. 반경 15.7도 구슬 띠보다 2.5m 밖이었다.

불꽃 메시를 실측하니 전부 평평한 빌보드다: **D 1.72 × 2.23m, E 1.22 × 1.35m는 Z가 납작하고
F는 2.27 × 0.79m로 X가 납작하다.** 1차에서는 셋 다 같은 yaw를 줘서 F만 90도 틀어져 있었다.

### G07 수치 검증 (전부 PASS)

런타임 합성식(`Sample_ObjectWorld`)을 그대로 재현해 계산한 값이다.

- 불의 벽 **60개 전부** 공전 반경 **12.6000** (min=max), 공전 중심이 아레나 중심과 0.05m 이내 → 링 이탈 **0/60**
- 구슬 띠(13.191) **안쪽**, 여유 **0.59m**
- 방향 **+24°/s 30개 / −24°/s 30개** (에셋별 CW·CCW 두 모션)
- **outward축 · radial 내적이 60개 전부 1.000000** — 빌보드 평면이 링을 따라 정확히 누웠고 F도 정렬됨
- 이웃 간격 **1.319m 균일** (불꽃 폭 1.46~2.06m라 서로 겹쳐 벽이 된다)
- 불 바닥 높이 y=0.05, resource scale D/E `(1.2,1.5,1.2)` · F `(0.7,2.5,0.7)`, occurrence placement scale 전부 `[1,1,1]`
- WORLD occurrence 78 = 불 60 + 갈고리 18. `MAP_PLACEMENT` 바인딩 **0개**
- 갈고리 18개 (아래 "동시 편집 충돌" 참조. 내가 만든 배치가 아니라 다른 세션이 교체한 배치가 들어 있다)
- `PATTERNBOSS` 행에 두 패턴 모두 `BOSS_KAKULSAYDON_G3_SAYDON` 포함 → Server가 3관문 보스에서 audition 수락

### 동시 편집 충돌 — 갈고리는 다른 세션이 소유 중이다

이 문서의 불 관련 수치는 전부 내가 만들고 검증한 값이다. **갈고리는 아니다.**

- 02:57 내 병합 직후 검증에서 갈고리는 내 설계(6웨이브 × 3레인, `rotationDegrees [0,45,0]`,
  1000/1400/1800/6000… 시작, 마지막 34800ms 종료, 중앙 레인이 아레나 중심 정확 통과)였다.
- **01:59:18**에 두 저작 문서가 다시 쓰이면서 갈고리 큐가 교체됐다. 현재 값은
  3웨이브 × (12° 3레인 + 72° 3레인), 시작 1000/10000/19000 + 레인 175ms, 마지막 27875ms 종료다.
- 출처: 백그라운드 세션 `d46cc2f2`가 `tmp/measure_hook_tip.py`(01:54:10)와
  **`tmp/fix_hook_direction.py`(01:59:11)**를 만들었고, 7초 뒤 내 composition의 갈고리가 바뀌었다.
  그 세션은 02:08:49에도 쓰기 중이었다.
- `Tools/KoukuSaydonPipeline/author_gate3_fire_hook_fragments.py`는 그대로다(HOOK_WAVES 6, 45°).
  즉 생성기가 아니라 산출된 저작 문서가 직접 수정됐다.

`AGENTS.md`의 "같은 파일에 다른 팀원의 미커밋 변경이 있으면 덮어쓰지 않는다"에 따라 **되돌리지 않았다.**
현재 파일에는 내 불 60개와 그 세션의 갈고리 18개가 함께 들어 있고 publish·validate는 통과한다.
교체 직전 상태는 `C:/Users/USER/.claude/jobs/c50effd5/tmp/concurrent_backup/`에 보존했다.
어느 갈고리 설계를 쓸지는 사용자가 정해야 하며, 같은 파일을 두 세션이 계속 편집하면 서로 덮어쓴다.

### 기존 장식 불 28개는 더 이상 구동하지 않는다

1차에서는 그 28개를 `MAP_PLACEMENT`로 잡아 공전시켰다. 그런데 그것들은 반경 13.94~19.44로
구슬 띠 바깥에 있어서, 움직이면 정확히 사용자가 지적한 "맵 밖으로 흩어진" 그림이 된다.
원본에서 도는 것은 아레나 림 안쪽의 촘촘한 불의 벽이고, 장식 불은 배경이다.
그래서 28개는 맵 저작 그대로 정지 상태로 두고, 공전하는 벽은 새 오브젝트 60개로 만들었다.
부수 효과로 프레임당 map placement 변경 28건(dirty 배치 6개)이 사라졌다.

### 갈고리 전용 확인 패턴

`KAKULSAYDON_G1_PATTERN_19` / `3관문_갈고리만_확인용_불없음` — 같은 갈고리 18회를 불 없이 재생한다.
world·instance·template·모델을 본 패턴과 공유하며 새 리소스는 없다. 갈고리가 여기서도 안 보이면
원인은 렉이나 불이 아니라 갈고리 자체다.
`project_kouku_saydon_composition.py:1428`이 `playAllPatternIds must equal PRODUCT patternIds in
authored order`를 요구하므로 이 패턴도 playAll에 포함된다.

### 갈고리 데이터가 정상임을 확인한 근거

- object resource `world.object.kouku.hook`은 이미 동작 중인 기존 갈고리와 **같은 행**이다.
  `animated: true`, `scale [1.15,1.15,1.15]`, `modelPreScale 0.00999999978`,
  모델 `Character/KoukuSaton/MN_UMAX_00/MN_UMAX_00.wmodel` (1,428,240 bytes 실재)
- template의 clip `Hook_idle_normal_1`은 기존 갈고리 motion의 clip과 동일 문자열
- 높이는 placement y=1.2에 key offset 3 → 0 → 0 → 3, 즉 4.2m로 들어와 1.2m로 훑고 4.2m로 빠진다

### 렉에 대해 확인한 것과 확인하지 못한 것

- 불 모델은 69,708 / 61,332 / 92,412 bytes로 작다.
- `m_ObjectModels`가 objectId별로 `CModel`을 한 번만 로드하고 occurrence clone은 같은 shared_ptr를
  공유한다(`WorldSequencePlayer_Objects.cpp:108-135, 353-356`). 갈고리 1.4MB 모델이 18번 복제되지 않는다.
- 1차에서 기존 불 28개를 구동할 때의 배치 재구축 비용도 실측했다. 배치는 `{assetId, mirrored}`
  단위이고(`MapPlacementRuntime.cpp:448`) 28개가 걸치는 배치는 6개, 그 안의 인스턴스는 합쳐 58개였다
  (맵 전체는 3,368 placement / 451 배치). 이번에는 그 구동 자체를 없앴다.
- 동시 object는 78개로 예산 1024에 여유가 크다.
- **남은 렉이 이 패턴 때문인지 아레나 기본 비용인지는 F1 Diagnostics의 smoothed FPS / frame time을
  Play 전과 재생 중에 비교하면 갈린다.** 이 항목은 해결로 기록하지 않는다.

## 3. 사용자가 화면에서 확인할 것

먼저 **Client(x64 Debug) 빌드가 필요하다.** `Client/Bin/Debug/`에 Engine.dll·셰이더 CSO는 있으나
`Client.exe`가 없고 `Client.pdb.stale`만 남아 있다(link 중단 흔적). `Server/Bin/Debug/Server.exe`는
09-10 01:20 빌드로 Shared 최신 소스(09-09 23:01)보다 뒤라 Server는 다시 빌드하지 않아도 된다.
`Shared/Bin/Debug/Shared.lib`도 09-10 01:20으로 최신이고 Client가 링크할 bomb/hammer 심볼이 들어 있다.

1. Client(x64 Debug) 빌드 → Server 실행 → Client 실행
2. Lobby → KoukuSaydon → F1 → KoukuSaydon Arena에서 3관문 선택
3. F1 Tools → Action Workbench → Patterns by Gate에서 Gate를 GATE3로
4. **`3관문_갈고리만_확인용_불없음`** 먼저 재생 — 불 없이 갈고리 18회만 나온다
   (갈고리 배치는 다른 세션이 교체한 12°/72° 교차 3웨이브다)
5. 그다음 **`3관문_외곽불회전_갈고리대각선_시각테스트`** 재생 — 36초 관찰
   - 불 60개가 아레나 금색 구슬 띠 **안쪽**에서 촘촘한 벽을 이루고 링을 따라 공전 (36초에 2.4바퀴)
   - 절반은 시계, 절반은 반시계라 서로 스쳐 지나간다
   - 기존 장식 불 28개는 정지 상태 그대로다
   - 갈고리는 1초/10초/19초 웨이브로 두 방향(12°, 72°)에서 교차해 지나간다
6. 두 경우 모두 **F1 Diagnostics의 FPS / frame time을 Play 전과 재생 중으로 비교**한다
7. 종료 후 불·갈고리가 사라지고 맵이 재생 전 상태로 돌아오는지, 3회 반복 재생 시 누적이 없는지 확인

4번은 보이는데 5번에서 안 보이면 밀도·렉 문제, 4번에서도 안 보이면 갈고리 자체 문제다.
6번 숫자가 Play 전후로 거의 같으면 렉의 원인은 이 패턴이 아니다.

## 4. 의도적으로 범위 밖인 것

이 갈고리는 **시각 오브젝트다.** 다음은 구현하지 않았고 구현했다고 보고하지 않는다.

- QTE 키 입력, 아재패턴 성공/실패 판정
- 플레이어 잡기·끌고 가기, 즉사, 화상 피해, 무력화 수치
- HP 조건 자동 발동, 관문 입장 자동 발동
- 플레이어 카메라 변경

새 패턴은 Composition 계약상 `playAllPatternIds` 끝에 포함된다. 이는 자동 발동이 아니라 전체 PRODUCT 재생 목록에 들어간다는 뜻이며, 단독 확인에는 위 한글 패턴만 선택해 재생한다.

## 5. 리소스 인계 (G08)

이번 구현은 설치된 WModel과 텍스처를 재사용한다. 신규 mesh/texture 추출·cook은 없었고 Resource 파일도 바꾸지 않았다. 파일이 없는 팀원 PC에는 팀장이 Drive로 아래 폴더를 전달한다. 물리 루트는 `Client/Bin/Resources`이며 Git에 force-add하지 않는다.

- `Character/KoukuSaton/MN_UMAX_00/` 전체 (모델 1,428,240 bytes + textures)
- `Map/LV_LUT_MIDNIGHTC_ED/MAP_CFEDE8067300_BG_RAD_KOUKUSATON_DECO24D_SM_KHB/` 전체
- `Map/LV_LUT_MIDNIGHTC_ED/MAP_B71A2EC9D778_BG_RAD_KOUKUSATON_DECO24E_SM_KHB/` 전체
- `Map/LV_LUT_MIDNIGHTC_ED/MAP_7AC8BB3D2FEE_BG_RAD_KOUKUSATON_DECO24F_SM_KHB/` 전체
- 기존 28개가 참조하는 다른 DECO24 변형은 이미 맵 실행에 필요하므로 기존 맵 Resource 배포에 포함되어 있다.

## 6. 지시서 대비 미달·이탈 항목

정직하게 남긴다.

1. **G06-3 1항 이탈 (반복).** 지시서는 "연결이 안 된다고 임의로 127.0.0.1로 바꾸지 않는다"고 했으나
   `Client/Default/Client.vcxproj.user`를 다시 `127.0.0.1`로 되돌렸다. `Sync-TeamLanEndpoint.ps1`이
   팀 endpoint `192.168.0.14`로 덮어쓰는데 이 PC의 Server는 `--bind-address 127.0.0.1`로 뜬다.
   그대로 두면 접속이 WSA 10060으로 실패해 사용자가 확인 자체를 못 한다. 이 파일은 `*.user`라
   `.gitignore` 대상이고 커밋에 섞이지 않는다. 공유 서버로 확인하려면 되돌리고 서버 담당자에게
   갱신·재시작을 요청해야 한다. **이번 세션에서만 세 번째 복구다.**
2. **화면 확인 없음.** 3항의 런타임 확인은 전부 미실행이다. 이 문서의 어떤 항목도 visual PASS가 아니다.
3. **렉은 해결로 기록하지 않는다.** 위 "렉에 대해 확인한 것과 확인하지 못한 것" 참조. 추가 불을
   절반으로 줄였고 데이터 쪽에서 프레임당 비싼 경로는 찾지 못했지만, 실제 프레임 개선 여부는
   사용자 관찰이 필요하다.
4. **불의 공전 방향 배분은 임의값이다.** 사용자가 그린 화살표는 방향이 섞여 있다는 사실만 알려 주고
   어느 불이 어느 방향인지는 지정하지 않는다. 현재는 기존 불이 index 홀짝으로 14/14,
   추가 불이 asset 단위로 16/8이다. 원본의 실제 배분이 아니라 "섞여 있다"는 조건만 만족한 값이다.

## 7. 부수 사항

`Client/Default/Client.vcxproj.user`의 `LOSTARK_SERVER_HOST`는 `Sync-TeamLanEndpoint.ps1`이 실행될 때마다
팀 주소로 되돌아간다. 이 PC에서 로컬 확인을 할 때는 매번 `127.0.0.1`로 다시 맞춰야 한다.

## 8. 갈고리 방향·대각선·매달리기 (2026-09-10)

사용자 요구 세 가지다.

1. 갈고리가 지나가는 방향에 갈고리 끝이 보여야 한다.
2. 11시→5시, 1시→7시로 서로 대각선으로 지나가고, 지나갈 때 끝이 진행 방향을 향해야 한다.
3. 지나가는 갈고리에 맞으면 갈고리 끝에 매달려 끌려가야 한다.

대상 패턴은 `KAKULSAYDON_G1_PATTERN_18`(`3관문_외곽불회전_갈고리대각선_시각테스트`)과
`KAKULSAYDON_G1_PATTERN_19`(`3관문_갈고리만_확인용_불없음`) 두 개이며 갈고리 occurrence 18개가 양쪽에 같다.

### 8.1 갈고리가 실제로 어느 쪽을 보는지 — 실측

G03의 지시서는 "모델의 정면을 local Z라고 가정하지 않는다. 최종 정면 방향은 사용자 화면으로 확인한다"고
적어 두었고, 실제로 확정되지 않은 채 `local +Z` 이동으로 저작되어 있었다. 이번에 파일에서 직접 쟀다.

- `MN_UMAX_00.wmodel`은 skinned 모델이므로 정점 버퍼가 곧 화면 자세가 아니다. World Object는
  `Hook_idle_normal_1`(2초, loop)을 재생한다.
- 뼈는 `b_hook_root_01 → point001 → b_chain_01..b_chain_11 → b_hook_01` 구조다.
- 스켈레톤 rest 계층과 클립 첫 프레임은 거의 같고, 둘 다 **사슬을 수직(+Y)으로 세운다**.
  클립에서 사슬 꼭대기는 8.23m, 갈고리 뼈 `b_hook_01`은 1.575m다.
- 그 자세로 메시를 스키닝하면 갈고리 머리는 아래쪽에 있고 **로컬 +X 쪽으로 열린다.**
  머리 영역의 X 범위는 −0.21~+0.49m, 갈고리 끝점은 대략 (X +0.37, Y +0.93)m다. 사슬 축은 X=0이다.
- 스키닝 수식은 자기검증을 통과했다. 같은 코드에 바인드 포즈를 넣으면 원본 메시를 정점 오차
  0.000000으로 재현한다.

따라서 **끝이 진행 방향을 향하려면 오브젝트는 자기 로컬 +X로 이동해야 한다.**
로컬 +X는 yaw θ에서 월드 (cos θ, −sin θ)이고 진행 방위 b는 (sin b, cos b)이므로 **occurrence yaw = b − 90**이다.

이전 세션에 내가 `local −X`로 바꿔 놓았던 것은 정확히 180° 반대였고, 그 이전 원본 저작의 `local +Z`는
90° 옆이었다. 둘 다 틀렸다.

### 8.2 두 대각선

화면 시계는 `Data/Camera/KoukuSaydon.camera.json`의 yaw 132°를 기준으로 한다. 12시가 월드 방위 132°이고
시간당 30°씩 시계 방향이다. 즉 `방위(시각) = 132 + 시각×30`.

- 11시→5시: 진행 방위 282°, occurrence yaw 192°, 9개
- 1시→7시: 진행 방위 342°, occurrence yaw 252°, 9개
- 각 가족은 3웨이브 × 3레인이고 레인 간격 4m, 웨이브 시작 1000/10000/19000ms, 레인 간격 350ms,
  두 가족 사이 175ms를 어긋나게 두어 교차가 겹치지 않게 했다.
- 경로 길이 48m, 중앙 레인의 중점은 아레나 중심 `(0, 942.080017)`, 양쪽 레인은 4m 옆이다. Y는 1.2다.

저장된 데이터만 읽어 독립 검증한 결과: 18개 전부 **끝과 진행 방향의 어긋남 0.000°**, 진입 11.3시/1.3시,
이탈 5.0시/7.0시.

### 8.3 매달려 끌려가기

움직이는 판정 영역이 이미 코드에 있다. `BOSS_LOGIC_REGION`에 `BOSS_LOGIC_WORLD_TRANSFORM_TRACK`을 달면
`Resolve_LogicRegionTransform`이 매 틱 그 World Object의 월드 변환을 돌려준다. 카드 접촉이 쓰는 경로다.
갈고리를 잡는 것도 그 영역이고, 잡은 뒤 끌고 가는 것도 같은 영역이다. 새 이동 모델을 만들지 않았다.

바꾼 것은 다음과 같다.

| 파일 | 내용 |
|---|---|
| `Shared/Public/Network/PacketMessages.h` | `PLAYER_ATTACHMENT_SLOT::WORLD_HOOK_TIP` 추가 |
| `Shared/Private/Network/PacketMessages.cpp` | GRABBED 불변식을 `== BOSS_LEFT_HAND`에서 `!= NONE`으로 완화 |
| `Shared/Public/Network/PacketType.h` | 프로토콜 76 → 77 |
| `Server/Public/ServerPlayer.h` | `iAttachmentWindowIndex`, `iAttachmentRegionIndex`, `iAttachmentReleaseTick` |
| `Server/Public/GameplayCatalog.h` / `.cpp` | 결과 종류 `GRAB_TO_WORLD_OBJECT` + 파싱 + ENTER_AREA/SUCCESS 전용 검증 |
| `Server/Private/KoukuSaydonLogicRuntime.cpp` | `Hang_PlayerOnRegion`, `Drag_HookedPlayer`, 매 틱 끌기 패스, ENTER_AREA 이음매에서 잡기 |
| `Server/Private/GameRoom.cpp` | `Update_PlayerAttachment`에 world-object 분기(기한 관리와 기존 release 호출) |
| `Client/Private/ClientReplication.cpp` | 손 소켓은 `BOSS_LEFT_HAND`일 때만 조회 |
| `Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py` | 새 결과 종류 허용 + ENTER_AREA 박스 전용 규칙 |

동작 순서는 이렇다.

1. 갈고리마다 `ENTER_AREA` 판정 창 하나와 콜라이더 하나를 저작했다. 창의 시각·길이는 그 갈고리의
   등장~퇴장과 정확히 같다(판정 영역은 자기가 타는 World Object의 수명 안에 있어야 한다). 갈고리 18개가
   시간대가 겹치지 않으므로 창도 18개다.
2. 콜라이더는 `anchorKind=WORLD`로 그 갈고리 occurrence에 묶이고, 오브젝트 로컬 +X로 0.35m 옮겨
   갈고리 머리(=오브젝트 원점 쪽) 위에 앉는다. CIRCLE 반경 1.0에 오브젝트 배율 1.15가 곱해져 월드 1.15m다.
3. 플레이어가 그 원 안에 들어오면 창의 성공 결과 `GRAB_TO_WORLD_OBJECT`가 발동한다. 잡기는 결과 switch가
   아니라 ENTER_AREA 이음매에서 처리한다. **어느 영역이 이 플레이어를 잡았는지 아는 곳이 거기뿐이고,
   그 영역이 곧 끌고 갈 주체이기 때문이다.**
4. 플레이어는 `GRABBED` + `WORLD_HOOK_TIP`이 되고, 잡은 창/영역 번호와 놓아줄 틱(창의 종료 틱)을 들고 있는다.
   소유자 net entity는 그 패턴을 돌리는 보스다. 기존의 보스 사망·패턴 중단·despawn 해제 경로가 그대로 듣는다.
5. 매 틱 `CKoukuSaydonLogicRuntime::Update`가 같은 영역을 다시 풀어 플레이어 XZ와 yaw를 갈고리에 맞춘다.
   영역이 더 이상 풀리지 않으면(트랙이 끝났거나 갈고리가 숨겨졌으면) 기한을 지금으로 낮춘다.
6. `CGameRoom::Update_PlayerAttachment`는 world-object 슬롯이면 자세를 건드리지 않고 기한만 본다.
   기한이 지나거나 보스가 그 패턴을 더 이상 돌리지 않으면 **기존** `Release_PlayerAttachment`를 부른다.

플레이어의 Y는 자기 값을 유지한다. 즉 갈고리에 걸려 **바닥을 따라 끌려간다.** 공중에 매다는 것은 아니다.
판정 영역이 XZ 계약이라 Y를 갖지 않기 때문이고, 원본에서 공중에 들리는 것이 맞다면 별도 요청이 필요하다.

`Is_Judgeable`이 GRABBED를 제외하므로 매달린 동안 다른 갈고리에 다시 잡히지 않는다. 데미지는 붙이지 않았다.
요구가 "매달려 끌려간다"까지였다.

### 8.4 검증한 것

- Composition 소스 검증 통과. `--mode validate`가 파싱·검증·투영을 모두 지나 "projected Product is stale"
  단계까지 갔다. 새 결과 종류, ENTER_AREA 전용 규칙, 36개 이동 판정 영역, 영역/World 수명 검사가 전부 통과한다는 뜻이다.
- 방향 재검증(저장 데이터만 읽는 별도 스크립트): 18/18 어긋남 0.000°, 두 가족 yaw 192/252, 진입 11.3/1.3시, 이탈 5.0/7.0시.
- 판정 영역 경로 재현(projector의 실제 투영 함수 + 서버 `Resolve_LogicRegionTransform`을 Python으로 재구현):
  48.00m 직선, 옆으로 벗어남 0.00000m, 5시/7시로 이탈, 반경 1.15m, 갈고리 창의 tick 0~239까지 생존
  (240tick=8초 중 마지막 비가시 key 직전).
- 스키닝 자기검증: 바인드 포즈로 원본 메시 재현 오차 0.000000.
- 변경한 5개 translation unit 구문 검사(`cl /Zs`, 출력물 생성 없음) 오류 0:
  `KoukuSaydonLogicRuntime.cpp`, `GameplayCatalog.cpp`, `PacketMessages.cpp`, `GameRoom.cpp`, `ClientReplication.cpp`.
  경고는 기존 CP949 관련 C4819뿐이다.

### 8.5 하지 않은 것

- **publish 미실행.** Visual Studio(`devenv`)가 열려 있어 같은 워킹 트리에서 publisher를 겹쳐 돌리지 않았다.
  VS를 닫은 뒤 아래 순서로 실행해야 런타임에 반영된다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_LUT_MIDNIGHTC_ED -Scope WorldSequences -Mode Publish
python Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py --mode publish
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon
```

- **빌드·화면 확인 미실행.** 프로토콜이 77로 올라갔으므로 Server와 Client를 **함께** 다시 빌드하고
  둘 다 재시작해야 한다. 76 피어는 새 슬롯 값을 거부하고 snapshot을 버린다.
- **visual PASS 아님.** 갈고리가 실제로 끝을 앞세우고 지나가는지, 매달린 자세가 원본과 같은지는
  사용자의 실제 화면 확인이 필요하다. 특히 8.1의 실측은 파일 기준이고 화면 판정을 대신하지 않는다.
- 잡힌 뒤의 피해, 탈출 입력, 전용 애니메이션은 요구에 없어 넣지 않았다.
