# 2026-09-20 팀장 검토 체크리스트 — 쿠크·발탄 버그 배치

이 문서는 Codex 세션이 진행 중인 16개 요청을 **팀장이 검토·확인하는 순서**로 재구성한 것이다.
작성 시점의 실측 기준이며, Codex가 계속 파일을 쓰고 있었으므로 revision 숫자는 검토 직전에 다시 확인한다.
구현 상태, 게시 상태, 사용자 화면 판정은 각각 분리해 기록한다.

---

## 0. 지금 상태 한 줄 요약

**코드와 데이터는 대부분 들어갔지만, 게시(publish)와 빌드가 안 돼 있어 지금 실행하면 거의 아무것도 안 보인다.**

| 항목 | 저작/소스 | 게시/런타임 | 판정 |
|---|---|---|---|
| 쿠크 Composition | revision 1906 | Encounter 1902, Gameplay.bootstrap 1902 | **미게시** |
| 쿠크 맵 worldsequences | revision 2136 | 런타임 2129 | **미게시** |
| 발탄 맵 worldsequences | revision 6 | 런타임 6 | 게시 완료 |
| RenderingProfiles | revision 67 | 런타임 67 | 게시 완료 |
| Gameplay.bootstrap | — | 09-20 09:06 생성 | **오늘 데이터(11:41~12:00)보다 낡음** |
| Client.exe / Server.exe | 소스 오늘 12:00까지 수정 | 09-19 18:53 / 16:07 | **낡음** |
| 오늘 Product 빌드 | — | 마지막 시도 = 실행 중 프로세스로 컴파일 전 FAIL | **오늘 성공 0회** |

`NETWORK_PROTOCOL_VERSION`은 94에서 **바뀌지 않았다.** 그러나 `PacketMessages.h`의 Shared inline 상수가
바뀌었으므로(카드미로 LMB 1000ms) **Server/Client를 같은 소스로 함께 빌드·재시작해야 한다.**
버전 핸드셰이크가 이 불일치를 잡아주지 못한다.

---

## 1. 검토 0단계 — 이걸 먼저 안 하면 아래 전부가 헛돈다

### 1-1. Codex가 정말 끝났는지 확인

```powershell
git status --short | Measure-Object -Line
Get-ChildItem Client\Private,Server\Private,Data,Tools -Recurse -File | Sort-Object LastWriteTime -Desc | Select-Object -First 5 LastWriteTime,FullName
```

30초 간격으로 두 번 실행해 파일 수와 최신 수정 시각이 멈춰 있는지 본다.
조사 중에도 `Level_Loading.cpp`(11:50), `KoukuSaydonPresentationPlayer.cpp`(11:51),
`bake_valtan_original_cinematic_actors.py`(12:00)가 계속 쓰이고 있었다.
**dirty 파일 수는 132 → 146 → 152 → 153으로 계속 늘었다.** 이 숫자를 기준으로 삼지 말 것.

### 1-2. 커밋 누락 확인 — 가장 위험한 항목

`.md/GB/09-20/` 폴더 **전체**가 untracked(`??`)다. 그리고 다음이 모두 untracked다.

```
Data/Effects/Authored/effect.valtan.action.15.stage002.full.restore.effect.json
Data/Effects/Authored/effect.kouku.bingo.bomb.full.restore.effect.json
Data/Effects/Authored/effect.kouku.bingo.hammer.aura.effect.json
Data/Effects/Authored/effect.kouku.bingo.medusa.face.full.restore.effect.json
Data/Effects/Authored/effect.kouku.mario.clown.hammer.end.effect.json
Data/Effects/Authored/effect.kouku.cardmaze.q.effect.json
Data/Effects/Authored/effect.kouku.cardmaze.lmb.effect.json
Data/Animation/Authored/<6 class>/<Class>.interactionbindings.json   (6개)
Tools/ActorXAssetCooker/build_card_maze_player_animations.py
Tools/EffectPipeline/restore_valtan_cinematic_static_effects.py
Tools/ValtanPipeline/restore_valtan_original_cameras.py
Tools/ValtanPipeline/bake_valtan_original_cinematic_actors.py
Tools/GameplayPipeline/Test-GameplayWorldTrackNumbers.ps1
```

**`Data/Effects/EffectCatalog.json`은 tracked이고 위 7개 effect 문서를 이미 참조한다.**
`Effect_Catalog.cpp:479-492`가 DIRECT_AUTHORED_DOCUMENT 행마다 실파일 존재를 검사하고,
하나라도 없으면 **Client의 Effect 카탈로그 스테이징 전체가 false**가 된다.
즉 catalog만 커밋하고 문서 7개를 빠뜨리면 다른 PC에서 쿠크·발탄뿐 아니라
**플레이어 스킬 이펙트까지 전부 죽는다.** 반드시 같은 커밋에 넣을 것.

`git clean -fd`를 돌리면 오늘 PLAN/RESULT 11개 문서도 통째로 사라진다.

### 1-3. Drive로 별도 전달해야 하는 물리 리소스 (Git 비추적)

```
Client/Bin/Resources/Character/{LanceMaster,GunSlinger,Slayer,Artist,DimensionMaster,Warlord}/AnimSets/<Class>_MazeHammerAnimSet.wmodel   (6개, 각 1.26~1.47MB)
Client/Bin/Resources/Character/KoukuSaton/WP_MN_RPCT_05/WP_MN_RPCT_05.wmodel  (앵콜 지팡이, 이미 존재)
Effect/KoukuSaydon/FullRestore/Meshes/fm_h_swing_03.wmodel  (마리오 광대 망치 끝 FX)
```

`CharacterCatalog.json`은 커밋되지만 wmodel은 안 된다.
**MazeHammerAnimSet이 없는 PC는 `Attach_AnimationSet` 실패 → 그 class 전체 admission 실패 →
카드미로뿐 아니라 Bern·Character Select·Valtan 어디에서도 그 직업을 못 쓴다.**

### 1-4. 빌드·게시 실행 순서

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Network/Sync-TeamLanEndpoint.ps1
```

실행 중인 Client/Server를 모두 종료한다. `Get-StandardProductOutputPaths`가 Debug/Release ×
Client/Server **네 EXE를 모두** 검사하므로, Debug Client 하나만 떠 있어도 Release 빌드가
컴파일 전에 `product:output-lock-preflight FAIL`로 멈춘다.

```bash
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug
```

그다음 게시를 **순차로** 실행한다(owner lock으로 직렬화되므로 동시 실행 금지).

```bash
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildDomainOwner.ps1 -Owner Client
```

```bash
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildDomainOwner.ps1 -Owner Server
```

- `Owner Client` = koukusaydon.product, map.kakulsaydon, composition.presentation, world.gameplay, navigation
- `Owner Server` = koukusaydon.product, world.gameplay, navigation, world.destruction, **gameplay.balance**, items, vehicles, honortitles, valtan.rewards
- **`gameplay.balance`는 Server/KoukuSaydon owner에만 있고 Client owner에는 없다.**
  `map.kakulsaydon`과 `composition.presentation`은 Server owner에 없다. **둘 다 돌려야 한다.**
- 예상 시간 3~6분 (쿠크 projection 66.7초, gameplay balance 104.5초 — 성능 최적화 후 실측)
- Owner Client/Server에는 **롤백 트랜잭션이 없다.** 중간 domain이 실패하면 앞쪽 출력만 새 것으로
  남는 혼합 상태가 된다(Owner KoukuSaydon에만 백업/복원이 있다).

### 1-5. 게시가 실제로 됐는지 숫자로 확인

```powershell
Select-String -Path Server\Bin\DataFiles\Gameplay\Gameplay.bootstrap -Pattern 'KOUKUSAYDONPRODUCTREVISION' | Select-Object -First 1
(Get-Content Data\KoukuSaydon\Gate1\KoukuSaydonComposition.json -Raw | ConvertFrom-Json).revision
(Get-Content Client\Bin\DataFiles\Map\LV_LUT_MIDNIGHTC_ED.worldsequences.json -Raw | ConvertFrom-Json).revision
(Get-Content Data\Maps\Authoring\LV_LUT_MIDNIGHTC_ED\LV_LUT_MIDNIGHTC_ED.worldsequences.json -Raw | ConvertFrom-Json).revision
```

bootstrap의 revision == Composition revision, 맵 런타임 revision == 저작 revision 이어야 한다.
`Select-String Gameplay.bootstrap -Pattern 'PATTERNSTAGEVOLLEY'`로 발탄 삼각형이 9 → 13.5로
바뀌었는지도 같이 본다.

### 1-6. Server를 반드시 새로 띄운다

게임 내 `Publish All Patterns`만으로는 안 된다. `GameRoom_KoukuAudition.cpp:180~196`의
`Has_SameNonKoukuGameplay`가 **발탄 행과 `PATTERNPRESENTATIONGENERATION ENCOUNTER_VALTAN <sha>` 행**의
변경을 감지해 `REJECTED_REVISION_MISMATCH`로 거부한다.

그 generation 해시의 입력(`valtan_presentation_generation.py`의 `FIXED_ARTIFACTS`)에는
`EffectCatalog.json`, `BossCatalog.json`, `ValtanEncounter.json`, `ValtanCombatObjects.json`,
`ValtanCinematicCamera.json`, `Valtan.patternsoundcues.json`과 `Data/Effects/V2/Authored/*.effectv2.json` 전체가 들어간다.
**쿠크 빙고 이펙트를 EffectCatalog에 한 줄 추가한 것만으로도 발탄 generation이 무효가 된다.**
이번 작업은 이 조건이 확실히 성립하므로 Server 재시작은 선택이 아니다.

Server → Client 순서로 띄운다. 직접 실행 시 Client 작업 디렉터리는 `Client/Default`.

---

## 2. 항목별 핵심 원리 + 검토 체크리스트

### A. 쿠크 1관문 과광 (사용자 요청 1)

**핵심 원리 — 노출과 블룸의 상속 비대칭**

`RenderingProfileService.cpp`의 `Resolve_EffectiveQuality()`는
`실효값 = base.값 × Profile.Multiplier`로 계산하는데, `base`를 공급하는 `Get_ProfileQuality()`가
**Level base profile의 `qualityOverride` 원본만 돌려주고 그 profile 자신의 multiplier는 적용하지 않는다.**

- `scene.kakulsaydon.g1.base.v1`은 09-19 source 복원에서 `qualityOverride.exposure 1.2 → 2`,
  `bloomIntensity 0.2 → 0.8`, `bloomThreshold 1.4 → 1.0`으로 올리고, **자기 자신에게만**
  `exposureMultiplier 0.5`, `bloomIntensityMultiplier 0`을 넣어 보정했다.
- gate alias(`g1.book-open.v1`, `g1.popup.v1`, `g3.dark.v1`)는 `qualityOverride`가 없어
  base의 **원본 2 / 0.8**을 상속한 뒤 자기 multiplier 1을 곱했다. → **정확히 2배.**

**Codex가 고친 것:** book-open / popup의 `exposureMultiplier`를 1 → 0.5. 이건 증상 보정이 아니라
현 계약에서 그 alias가 가져야 할 정확한 값이다. 이미 소스·런타임 양쪽 revision 67로 반영돼 있어
rendering publisher 재실행이 필요 없다.

**아직 남은 것 (더 큰 축):** `bloomIntensityMultiplier`가 base는 **0**, gate alias는 **1** 그대로다.
1관문 region `kouku.ps.environment.47`의 `bloomIntensity 0.9 / threshold 0.7`이
alias에서만 살아난다. 3관문 `g3.dark.v1`은 exposureMultiplier도 1이라 실효 노출이 여전히 2다.

**카드가 하얗게 번지는 것은 별개 경로:** `Shader_EffectV2_Common.hlsli`의
`output.vBloomContribution = Write_SceneBloom(output.vSceneColor)`가 **emissive가 아니라 base color 전체**를
bright-pass에 쓴다. 카드 9개 leaf는 emissive 슬롯이 비어 있어서 기존 `bloomIntensity`가 아무 효과가 없었다.
Codex가 `sceneBloomScale`(0~1, bloom RGB만 곱하고 alpha/본체색은 보존)을 추가하고 카드 9개에 0을 저장했다.

**검토 절차**

1. **V2 CSO 7개가 반드시 새로 만들어져야 한다.** `Shader_EffectV2_Common.hlsli`를 include하는 FX는
   Rect/Mesh/AnimMesh/Particle/MeshParticle/**Trail**/Decal V2 **7개**다.
   → CSO가 옛것이면 `Bind_RawValue("g_SceneBloomScale")`가 E_FAIL을 내고 **모든 V2 이펙트가 통째로 사라진다.**
   증상 확인은 화면이 아니라 `Client/Default/EffectFailure.user.log`의 `"Shader bind failed"`.
2. **가장 깔끔한 A/B: GATE1 → GATE2 → GATE1 왕복.** Gate 2는 전용 alias가 없어 Level base로 되돌아간다.
   이제 base와 book-open의 차이는 **블룸 하나뿐**이다(exposure는 둘 다 1, region postProcess 내용 동일).
   GATE1을 눌렀을 때 밝아진다면 그것은 100% `bloomIntensityMultiplier` 0↔1 차이다.
3. F1 → Tools → Rendering Workbench 패널 하단의 `Active profile:`과 현재 environment region ID를
   **읽어서** 확인한다. region 밖으로 나가면 alias는 region이 1개뿐이라 블룸이 0.9↔0.8로 바뀐다.
4. 시스템 옵션 **블룸 체크를 껐다 켜서** A/B 한다. 끄면 원하는 밝기에 가까워지면 원인은 블룸이다.
5. `Map light comparison` → `Map lights off`로 맵 라이트 기여를 분리한다.
   단, **크게 어두워져도 그것만으로는 중복 가산의 증거가 아니다** — 맵 라이트가 원래 1관문 주광원일 수 있다.
6. 카드: 카드 자체 색·알파는 그대로이고 **주변 하얀 번짐만** 사라져야 한다.

**주의 / 회귀 위험**

- 맵 라이트 109개(enabled·brightness≠0) 중 **25개가 `LIGHT_RECEIVER::ALL`**이다(JSON에 `receiver` 키가
  없는 행 = 기본값 ALL. `"ALL"` 문자열을 grep하면 0건이다). 1관문 SL01의 `light.kouku.source.sl01.42/46/47/48/267` 포함.
  `Reject_LightReceiver`는 ALL 광원의 baked 비트를 **검사조차 하지 않는데 이건 버그가 아니라 설계**다
  (RNM이 없는 이동 오브젝트도 비춰야 한다). **25개를 일괄 UNBAKED로 바꾸는 시도는 절대 하지 말 것.**
- 카드의 `sceneBloomScale=0`과 alias의 `bloomIntensityMultiplier` 수정은 **둘 중 하나만 남겨야 한다.**
  나중에 alias를 0으로 맞추면 카드가 이중으로 죽어 원작보다 밋밋해진다.
- Effect Tool V2에서 카드의 blend를 Multiply로 바꿨다 되돌리면 `sceneBloomScale`이 **조용히 1로 복원**된다(경고 없음).
- source tone mapping 분기는 `Shader_Deferred.hlsl:1546`에서 조기 return하므로 Hable/gamma/채도/색약필터를
  건너뛴다. 다만 **밝기 슬라이더는 동작한다** — `fGamma`가 source grading LUT을 굽는 입력으로 들어간다.
  진짜로 무효화되는 것은 **색약 보정 필터**뿐이다.

---

### B. 2관문 컷씬 directional light + 연출 중 Fog (사용자 요청 12, 13)

**핵심 원리**

15048ms는 `KAKULSAYDON_G1_PATTERN_73`("2관문_진입컷씬") 안의
`presentation.kouku.gate2.intro.camera.3`(startMs **13950**, durationMs **5540** → 13950~19490ms) 구간에 **포함된다.**

쿠크가 검게 나오는 이유: Level base `scene.kakulsaydon.g1.base.v1`의 light가 `diffuse [0,0,0]`이고,
GATE2는 `Commit_GatePresentation`에서 gate profile을 **빈 문자열**로 두어 base를 그대로 쓴다.
컷씬 세트의 unbaked native 몸체가 받을 diffuse가 없다.

Codex의 해법: `before-restoration.v1`의 Light를 복사해 `eReceiver = LIGHT_RECEIVER::SOURCE_CHARACTER`로
바꿔 override로 올린다. `Light_Manager.cpp:135`가 이 광원을 일반 ALL 패스에서 제외하고,
`Renderer.cpp:1236~`의 `SOURCE_LIGHT_MASK` 스텐실 전용 패스에서만 그린다. → **baked lightmap 맵 지오메트리를 이중 조명하지 않는다.**

Fog 정본은 scene profile **하나뿐**이다(`RenderingProfiles.json`의 `profiles[].fog`와
`environmentRegions[].fog`). maplights 문서는 fog를 소유하지 않는다.
**region JSON에는 `enabled` 키 자체가 없고 파서가 무조건 `true`로 강제한다.**
그리고 region 블렌드가 `bEnabled`를 블렌드하지도 재설정하지도 않으므로,
**데이터만으로는 region 안에서 fog를 끌 방법이 없다.** 진짜 off는 스키마에 enabled 추가가 필요하고,
지금 가능한 우회는 opacity/density를 0에 가깝게 저작하는 것뿐이다.

**검토 절차**

1. **관문 버튼만 눌러서는 컷씬이 재생되지 않는다.** 이 shot은 `activation: "PATTERN_ONLY"`다.
   Action Workbench에서 `KAKULSAYDON_G1_PATTERN_73`을 `Play Isolated` / `Start Full Pattern`,
   또는 F1 Complete Play로 GATE2 Flow를 돌려야 한다. **관문 버튼만 눌러 놓고 "수정이 안 먹었다"고 판단하지 말 것.**
2. **F6 자유 카메라 상태면 안 된다.** `Sample_CompositionCamera`가 `Is_FollowEnabled()`를 먼저 검사하므로
   자유 카메라에서는 composition 카메라 자체가 비활성이고, **fog 억제도 directional 복구도 전혀 동작하지 않는다.**
3. 13.95~19.49초 구간에서 쿠크/세이튼이 회색 ambient 덩어리가 아니라 위-왼쪽 방향
   (`direction [0.48,-0.77,-0.43]`) directional 음영을 받는지 본다.
4. 실패 시 증거는 파일 로그가 아니라 **Visual Studio 출력창 / DebugView**의
   `Presentation directional override requires one scene light.` /
   `Presentation directional override rejected; current scene preserved.` 문자열이다.
   `Client/Bin/<cfg>/Diagnostics/client-session-<pid>.jsonl`의 `kouku.cinematic.transition` 이벤트에서
   `cameraShot=kouku.gate2.intro.camera.3`, `cameraReturning=0`도 확인한다.
5. Fog: **camerashots 114개 중 `cameraTrack` 보유 92개만 커버된다.** 미커버 22개는
   `cardmaze.telescope`, `cardmaze.follow`, `*Mario.follow.*` 18개, `1Stage.finale`, `kouku.gate1.authored.finale`.
   → **카드미로 전 구간과 마리오 플레이 구간은 fog가 꺼지지 않는다. 이건 현재 정상 동작이며 미구현 항목이다.**
   (마리오 *인트로* shot `1Mario`~`4Mario`는 cameraTrack이 있어 커버된다.)
6. 발탄 컷씬 fog도 확인하되, **발탄 쪽은 `Is_CinematicCameraActive()` getter 하나가 아니라
   원본 컷씬 재생 서브시스템 전체가 새로 들어왔다.** 안개와 컷씬 재생 자체를 나눠서 판정할 것.

**주의**

- `Needs_Gate2IntroCharacterLight()`의 두 번째(area-shot) 갈래는 이 shot의 `sequenceInstanceId`가
  빈 문자열이라 **사실상 죽은 코드**다. 나중에 camerashots에 sequenceInstanceId를 채우면 갑자기 살아난다.
- shot ID `"kouku.gate2.intro.camera.3"`이 C++ 문자열 리터럴로 하드코딩돼 있다.
  Workbench에서 이 assetId를 바꾸면 **조용히 light 복구가 꺼진다.**
- `Is_CinematicPresentationActive()`의 첫 줄은 `if (m_bSequenceCombatPending) return true;`다.
  카메라 shot과 무관하게 sequence 전투 대기 구간 전체에서도 fog가 꺼진다.
  "컷씬이 아닌데 안개가 사라졌다"는 버그가 아니라 이 갈래일 수 있다.
- 컷씬 5.54초 동안 scene light 전체가 override 하나로 교체된다. 카메라가 region 49/50/55/sl03.59
  안에 있으면 diffuse 최대 0.8짜리 ALL directional이 통째로 사라져 **맵·프롭이 부분적으로 어두워질 수 있다**
  (base의 [0,0,0]만 보고 "차이 없음"이라고 판단하면 안 된다 — region이 base를 덮어쓴다).

---

### C. 1관문 머리 위 카드 문양 배분 (사용자 요청 2)

**핵심 원리 — Server 권위, Client는 enum → GROUP ID 변환만**

정본은 `SERVER_PLAYER::eMechanicCardSymbol` / `eMechanicCardColor` 두 필드다.

- `Apply_KoukuGateEntryCard`(유일한 배정 호출자)가 `m_Players`를 돌며 **자기 외 플레이어의 문양을 4비트 mask**로
  모으고, 자기 문양이 남과 겹칠 때만 폐기한 뒤 `Assign_EncounterCard(..., used)`를 호출한다.
- `Assign_EncounterCard`는 `used`에 없는 문양만 `available[]`에 담아 `available[roll % count]`로 고른다.
  색은 같은 seed를 **한 번 더 해시한** `Mix(roll) & 1u`라 문양 deck과 독립이다.
- 배정 시점: `Start_KoukuRaidCombat`이 보스 entity를 `m_WorldEntities`에 push한 **직후**,
  다음 `S2C_WORLD_SNAPSHOT`이 나가기 **전**. → 사용자 요구("전투 진입 시 바로 표시")와 일치한다.
- 제거: `Begin_KoukuRaidCinematic`이 **모든 관문 연출 시작에서 전원 Clear**한다. Client에 별도 despawn
  트리거 코드가 없고, Server가 NONE을 보내는 것만으로 사라진다.
- 관문 게이팅: `BOSS_KAKULSAYDON_G1_KOUKU` / `G1_SAYDON`일 때만 배정. 2관문은 `G2_*`라 못 받는다.

**검토 절차 — 이 부분이 가장 함정이 많다**

1. **혼자 테스트로는 이번 수정을 전혀 검증할 수 없다.**
   `used == 0`이면 새 코드 `available[roll % 4]`는 삭제된 옛 코드 `1u + (roll & 3u)`와 **비트 단위로 동일**하다.
   실제로 바뀐 것은 (a) 2인 이상일 때의 mask 배제, (b) 색 재혼합 두 가지뿐이다.
   → **반드시 2인 이상으로 확인할 것.**
2. **F1 `1관문 - 세이튼` 버튼으로 다인 테스트하면 안 된다.**
   그 Debug 경로(`GameRoom_PartyWorld.cpp:1313`)는 **버튼을 누른 session의 플레이어 1명에게만** 배분한다.
   제품 경로 검증은 `Complete Play`로 GATE1 Flow를 돌려 전투 진입 순간을 봐야 한다.
3. 4인이면 하트/스페이드/클로버/다이아 **전부 다른 문양**. **색은 독립이라 4명 전원 빨강도 정상이다.**
4. 2관문 진입 연출 시작 순간 전원 동시 제거. **F1 버튼으로 2관문에 가면 누른 사람 카드만 지워진다**(경로 차이, 버그 아님).
5. 카드미로 진입 시 **머리 위 관문 카드와 발밑 미로 문양 Decal은 서로 다른 필드**다
   (`eMechanicCardSymbol` vs `eCardMazeSuit`). 값이 달라도 정상.
   단, raid가 돌고 있지 않을 때 F1 카드미로 버튼은 despawn-all로 머리 위 카드를 지운다.

**주의 / Codex에게 확인할 것**

- **카드는 표현 전용이 아니다.** `Judge_Roulette`이 `eMechanicCardSymbol`로 룰렛 sector 정답을 판정한다.
  문양 유일화는 **4인 룰렛에서 네 명이 반드시 다른 섹터로 흩어지게 만드는 난이도 변경**이다.
  (현재 저작에 `cardRegions`가 없어 색은 판정에 안 쓰이지만, 코드는 색까지 AND 비교한다.)
- **전투 진입 tick에 사망 중이던 플레이어는 카드를 못 받고, 부활해도 재배분 경로가 없다.**
  그 관문 내내 룰렛을 전부 실패한다. 의도인지 확인 필요.
- `PacketMessages.h:869-870`의 주석("룰렛 창이 열릴 때 배정하고 판정될 때 회수")이 **현재 구현과 정반대**다.
  이 주석을 믿고 Judge 경로에 Clear를 넣으면 1회차 판정 직후 카드가 사라져 2·3회차가 전원 실패한다. 주석 정정 필요.
- 새 계약 테스트는 `Assign_EncounterCard`를 직접 호출하며 mask를 테스트 코드가 손으로 누적한다.
  **제품 로직인 `Apply_KoukuGateEntryCard`의 mask 수집·archetype 게이팅·충돌 분기는 한 줄도 실행되지 않는다.**
  그 셋이 전부 망가져도 새 테스트는 통과한다. 다인 유일성은 사실상 사용자 실행으로만 확인된다.
- 기존 계약 테스트 `"Deal all eight suit/color cards..."`가 색 선택식 변경으로 깨질 수 있다.
  `Server.exe --contract-test`를 돌린다면 **새로 추가된 줄이 아니라 이 기존 줄**을 함께 봐야 한다
  (이 스위트에는 baseline 실패 23건이 있다).
- **Client는 실패한 카드를 절대 재시도하지 않는다.** 리소스 로드 실패 시 로그 메시지조차 남지 않고,
  Server가 문양/색을 바꿔주기 전까지 그 사람만 카드가 없다.
  "한 명만 카드가 없다"를 볼 때 Server 배분 실패와 Client 1회성 리소스 실패를 구분해야 한다.

---

### D. 카드미로 쿠크 손 뿅망치 + Transform 수치 (사용자 요청 5, 16)

**핵심 원리 — 연출 World actor와 전투 NPC는 다른 골격이다**

뿅망치는 `anchorKind: "BOSS"`, `anchorBone: "b_wp_1"`, `anchorBossArchetypeId: "BOSS_KAKULSAYDON_G2_KOUKU"`인
World Object다. `Make_WorldSequenceTargets().bossAnchor`가 예전에는 **복제된 전투 NPC**만 찾았다.
그런데 카드미로 컷씬에서 화면에 보이는 쿠크는 전투 NPC가 아니라 **연출용 World actor**
`world.object.kouku.gate2.maze.kouku`(다른 wmodel, 다른 pose)다.
→ resolver 실패 → `Apply_Objects`가 **모든 object를 Hide한 채 return true** → **BOSS anchor에는 pivot fallback이 없어 조용히 사라진다.**

Codex의 해법: `Try_GetPresentationBossAnchor`가 연출 World actor를 먼저 찾는다.
매칭은 `presentationBossArchetypeId == archetype` **또는** derivedActor 휴리스틱
(`animated && materialSourceModelAssetId == bodyModel && |modelPreScale - bodyModelPreScale| < 1e-7`).
**실제 저작 데이터에 `presentationBossArchetypeId`를 가진 resource가 459개 중 0개**이므로
현재 이 resolver는 100% derivedActor 휴리스틱에만 의존한다. BossCatalog의 `bodyModelPreScale`을 바꾸면 조용히 끊긴다.

**사용자 수치 반영 상태:** `sequence.LV_LUT_MIDNIGHTC_ED.world_object.whirlwind_hammer`(휠윈드 전용)의
키가 `[1.25,0,0]` / quaternion `[0,1,0,0]`(= Y 180°)로 **이미 들어가 있다.**
`scale [2,2,2]`는 resource에 원래 있던 값이다.
카드미로용으로 별도 template `sequence.kouku.maze_cinematic_hammer`(옛 `[0.3,0,0]` 값 보존)와
신규 world box `kakulsaydon.g1.world.40`이 추가되어 **휠윈드와 분리됐다.**

**"composition changed" Save 거부의 정체**

`Save_Atomic`이 3-way merge를 시도하고 (a) 현재 파일 parse 실패, (b) `current.iRevision <= m_LastGood.iRevision`,
(c) 같은 필드 충돌 중 하나면 거부한다. Codex가 revision을 1902→1906으로 올렸으므로
사용자 draft가 같은 행(PATTERN_77.worldOccurrences / worlds)을 건드렸다면 (c)다.

**해결 순서:** ① 화면의 미저장 편집 내용을 메모 → ② Action Workbench에서 **Reload**(저장본 다시 읽기)
→ ③ 메모한 편집만 다시 입력 → ④ Save.
이유 문자열에 `external revision did not advance`가 있으면 외부에서 revision을 안 올리고 덮어쓴 것이므로
Codex에게 revision 증가와 writer lock 준수를 요구해야 한다.

**검토 절차**

1. F1 → KoukuSaydon Arena → `Kouku Whirlwind Hammer Transform` 트리에서
   Pos `1.250/0/0`, Rotation `0/180/0`, Size `2.000/2.000/2.000`이 읽히는지 확인.
   → "The saved hammer Motion is unavailable."가 뜨면 저장본에서 resource/instance를 못 찾은 것.
2. **F1 Complete Play로 GATE2 Flow를 돌리는 것으로는 카드미로 망치를 검증할 수 없다.**
   `world.40`은 `PATTERN_77`("카드미로연출")에만 붙어 있고 **PATTERN_77은 어떤 Flow/Bundle에도 없다.**
   Gate2 Flow가 도는 카드미로는 `bundle.4 → PATTERN_28`이고 worldOccurrences가 **빈 배열**이다.
   → Action Workbench에서 **PATTERN_77을 명시적으로 Play/Play Isolated** 해야 한다.
3. 휠윈드 패턴(`PATTERN_24`)과 카드미로 컷씬을 연달아 재생해 두 망치가 **서로 다른 위치/회전**인지 대조.
4. 전투 중(연출이 아닌 실제 2관문 쿠크 전투) 휠윈드 망치가 정상인지도 확인.
   → 전투 중에만 사라지면 새 cinematic resolver가 status를 채운 채 실패해 **기존 NPC fallback까지 막은 것**이다
   (`if (!status.empty()) return false;`). 이 증상이 나오면 Codex에게 수정 요청.
5. 실패 로그는 **두 문자열을 모두** 검색: `"Cinematic World boss anchor is ambiguous: "` (같은 player 안 중복)와
   `"Multiple cinematic World actors own this Boss anchor: "` (서로 다른 player).

**게시 순서 함정 — 가장 위험**

- worldsequences만 게시하고 Composition 투영(`Publish All Patterns`)을 빼먹으면,
  카드미로 컷씬이 `whirlwind_hammer` 인스턴스를 계속 가리켜 **새 1.25/180 값을 그대로 상속한다.**
  (지금 `KoukuSaydonEncounter.json`의 PATTERN_77.world.2가 정확히 그 상태다.)
- 반대로 Composition만 게시하고 worldsequences를 빼먹으면 `maze_cinematic_hammer` 인스턴스가
  런타임에 없어 **조용히 사라진다.**
- **Composition Validate는 `sequenceInstanceId`가 stable ID 문자열인지만 보고 실재를 교차검증하지 않는다.**
- → **worldsequences를 먼저(또는 동시에), Composition을 그다음.**

**주의**

- F1 Quick Transform 패널은 `whirlwind_hammer` **하나만** 편집한다. 카드미로 망치를 이 패널로 고치려 하면
  엉뚱하게 휠윈드가 바뀐다. 카드미로는 Object Tool에서 `sequence.kouku.maze_cinematic_hammer`를 직접 골라야 한다.
- 이 패널의 편집은 **모든 track·모든 key에 델타를 일괄 적용**한다. 이 motion에 중간 키프레임이 생기면
  한 번의 드래그가 애니메이션 전체를 오프셋한다.
- `KoukuSaydonComposition.json`이 pretty(20,759줄) → compact(2,738줄)로 **통째로 재직렬화됐다.**
  내용 손실은 없다(logics 98→98, patterns 89→90, worlds 70→71 확인). 그러나 **이 파일의 PR diff는
  사람이 리뷰할 수 없다.** 요소 수 대조로만 검증하고 눈 diff로 승인하지 말 것.
  다른 세션이 pretty로 저장하면 전체가 또 뒤집힌다 — 팀에 공유 필요.

---

### E. 카드미로 플레이어 뿅망치 + LMB/Q (사용자 요청 4)

**핵심 원리 — 무기 교체는 전적으로 Server snapshot 기준**

`SNAPSHOT_PLAYER::eKoukuHudMode`가 `Apply_NetworkAction`의 `interactionMode` 인자로 들어오고,
그 첫머리가 `Apply_MazePresentation(KOUKU_HUD_MODE::MAZE == interactionMode)`를 호출한다.
Client가 스스로 "지금 카드미로다"라고 판단하는 제품 경로는 없다.

`Apply_MazePresentation`은 `bip001-r-hand` 본을 확인하고 `WhirlwindHammer.wmodel`을 ×2 스케일
`MODEL::NONANIM` 프로토타입으로 커밋한 뒤 `Part_95_MazeHammer`로 부착한다.
기본 무기 숨김은 `Set_PartVisible`의 새 가드가 담당한다.

입력: **새 packet 없음.** LMB는 기존 `C2S_INTERACTION_SLOT`을 `INTERACTION_SLOT::W`(index 1)로 재사용한다.
`Update_PlayerModes`의 MAZE count를 1 → 2로 늘려 W 슬롯을 열었다.
액션 길이 Q=2500ms / LMB=1000ms, 타격 tick Q=30 / **LMB=12**.

**검토 절차**

1. 여섯 직업 중 최소 2개로 카드미로 진입 → 기본 무기가 사라지고 오른손에 뿅망치만.
   MAZE를 벗어나면 원래 무기 복귀. **진입/이탈을 반복**해 복구를 확인한다.
2. LMB = 오른쪽→왼쪽 가로 휘두르기 ~1.0초, Q = 점프 내려찍기 ~2.5초. **자기 직업 골격**이어야 한다.
3. LMB 연타 간격 0.4초, Q와 쿨다운 독립. Q 스윙 중 LMB로 회수 동작이 끊기는 것은 의도된 순서다.
4. **이펙트는 아직 안 나온다. 이건 정상이다.** 두 저작 문서의 `elements`가 빈 배열이고,
   `Validate_Drawable`이 `"Effect has no visible Element or Model / Summon to preview."`로 거부해
   준비 큐에 등록되지 않는다. **EffectCatalog.json 등록은 이미 돼 있다** — 카탈로그를 뒤질 필요 없다.
   F1 → Effect Tool V1 → All Effects → `CARD MAZE SKILLS` → `Open Effect`로 열어
   **visible element를 최소 1개 넣고 Save**하면 그때 계약이 성립한다.
5. **Save 후에는 레벨을 나갔다 다시 들어가야 한다.** `Load_InteractionAnimationBindings`의 호출자는
   `CCharacter::Initialize` 단 한 곳이다.
6. 이펙트는 **6직업 공용 문서 1쌍**이다(`"Shared by all six classes."`). 직업별로 다른 FX를 원하면
   새 assetId·새 문서·카탈로그 등록이 추가로 필요하다.

**주의 / 회귀 위험**

- **`Apply_MazePresentation` 실패가 snapshot action 적용 전체 실패로 번진다.**
  `Apply_NetworkAction` 최상단에 `if (!Apply_MazePresentation(...)) return false;`가 있고,
  로컬 캐릭터 경로에서는 `FATAL_FAILURE`로 **Lobby 복귀**까지 간다.
  망치 wmodel 누락이나 `bip001-r-hand` 부재가 "이펙트 하나 안 나옴"이 아니라 "세션 이탈"이 된다.
- **12tick 재도입.** `HAMMER_HIT_TICK_OFFSET`을 30으로 올린 기존 주석이 그 이유를 명시한다:
  "12는 와인드업 안이라 아무것도 맞지 않았는데 스윙이 성립했다."
  이번에 그 폐기된 12를 LMB에 다시 썼고, Codex RESULT도 "LMB contact 0.4초는 원본 notify 확정값이 아닌
  **프로젝트 튜닝**"이라고 인정했다. → **LMB 클립에서 12틱(0.4초) 시점에 망치 머리가 실제로 대상 위치를 지나가는지
  눈으로 대조할 것.** 지나가기 전이면 과거 광대 버그가 직업 LMB로 재현된 것이다.
- **LMB도 Q와 완전히 같은 판정을 한다.** `Resolve_CardMazeHammerHit`이 index를 구분하지 않아
  삐에로 상자 파괴·망원경 claim·병사 처치가 LMB로도 된다. 그런데 HUD 안내 문구와 CLAUDE.md는 여전히 "Q"로만 쓰여 있다.
  '망원경은 Q 전용'이 설계 의도였다면 이것이 회귀다.
- `Poll_SkillSlots`의 MAZE 차단은 `i != 0` 슬롯을 영구히 막는다. MAZE 슬롯을 3개 이상으로 늘리면 조용히 죽는다.
- **`Save camera settings`를 한 번만 눌러도 그 맵 JSON에 `classSizeMultipliers`(Artist 1.6 / DM 0.7 / clown 0.7)가
  영구히 굳는다.** 뿅망치 오프셋을 잡기 전에 캐릭터 크기부터 판정할 것 (아래 G 항목 참조).
- Mario 뿅망치 경로는 손대지 않았다(고정 30tick, 슬롯 0만). 마리오 공 터뜨리기 타이밍이 달라 보이면 이번 변경 때문이 아니다.

---

### F. 마리오 광대 뿅망치 끝 이펙트 + 광기 광대 무기 숨김 (사용자 요청 3, 7 일부)

**핵심 원리 — 두 개의 독립 경로**

1. **저작/미리보기(World Object):** 신규 objectResource `world.object.kouku.mario_clown`("마리오_광대", REUP.wmodel)와
   자식 motion `world.object.kouku.mario_clown.attack`("뿅망치_휘두르기_마무리", clip `att_battle_1_01`, **1367ms**).
   REUP.wmodel 헤더를 직접 파싱한 실측 클립 길이는 **41 tick / 30 tps = 1366.67ms** — 저작값과 일치한다.
   effectTrack의 **`timing: "MOTION_END"`**가 핵심이다. `EffectStartMs()`가 `effect.startMs`가 아니라
   **템플릿 `durationMs`를 trigger로 반환**한다. 이것이 "클립 끝"의 코드상 정의다.
2. **실제 게임 NPC:** `MonsterCatalog.json`의 `MONSTER_MARIO_REUP.attackPresentations[0]`에
   optional `endEffectAssetId` 추가. `CNpc::Schedule_ClipEndEffect`가 남은 시간을 계산해 0이 되면 1회 Spawn한다.

**광대 무기 게이트:** `eMadnessForm`(CLOWN)은 **몸체 교체만**, `iMarioStage`(1..4)는 **망치 표시만** 결정한다.
`Apply_MarioPresentation`의 신규 3줄 + `Set_PartVisible`의 강제 차단 게이트가
`m_bMazePresentation || (광대 spec && !Mario)`일 때 무기를 숨긴다.
→ 광기 게이지 100%로 CLOWN이 돼도 `iMarioStage == 0`이면 망치가 숨고, Mario 1~4에서만 보인다. **요구와 일치.**

**검토 절차**

1. Object Tool 미리보기: F1 → Action Workbench → **Object** → `마리오_광대` → `뿅망치_휘두르기_마무리` → `Visual Play`.
   1.37초 스윙 끝에 `fm_h_swing_03` 메시 FX가 한 번 나타나 ~1.4초 뒤 사라진다. **클립 끝 pose에 고정**되고 따라다니지 않는다.
2. FX 방향: 템플릿 회전 -90°와 cue 회전 +90°가 **한 번만** 상쇄돼야 한다. 90°/180° 틀어지면 이중 적용이다.
3. 실제 NPC: Mario 1 또는 3에서 REUP NPC 공격 끝에 같은 FX 1회.
4. 광기 광대: F1 → KoukuSaydon Arena → `Change to Clown` → **손에 망치 없음.** HUD mode를 바꾸거나 이동·스킬을 써도 계속 없어야 한다.
5. Mario 1~4 진입: 키가 1.5m로 줄고 **망치가 나타난다.** Mario에서 나오면 다시 사라진다.
6. 2인 이상으로 **원격 시점**도 확인(local/remote 두 호출 지점).

**주의 / 미구현**

- **가장 큰 회귀 위험:** `Level_Loading.cpp`가 모든 `endEffectAssetId`를 필수 준비 대상에 넣고,
  준비 실패가 하나라도 있으면 쿠크 아레나에서 **`"Required raid Effect preparation failed: ..."`로
  activation이 영원히 안 되고 로딩 화면에 머문다**(Debug/Release 공통).
  effect 문서가 untracked이고 의존 메시는 Drive Resources이므로,
  **둘 중 하나라도 없는 PC는 "FX만 안 나오는" 게 아니라 쿠크 아레나에 못 들어간다.**
  진입이 멈추면 `EffectFailure.user.log`의 `Kouku.Loading.V1Settled`의 targets/prepared/**failed** 카운트를 먼저 볼 것.
- `Schedule_ClipEndEffect`의 **거부 조건 6개 이상이 아무 로그도 남기지 않는다.**
  `[Npc] Clip-end Effect isolated:` 로그는 예약이 **성공한 뒤** Spawn이 실패했을 때만 찍힌다.
  → 카탈로그 미등록으로 예약 자체가 안 된 경우 사용자는 아무 단서도 못 본다.
- 같은 이펙트인데 **두 경로의 수명이 다르다.** Object Tool 미리보기는 1401ms에서 잘리고,
  NPC 런타임은 자연 종료(~2.0초)까지 간다. 미리보기 길이를 실제 게임 길이로 기록하지 말 것.
- **Object Tool 미리보기는 디스크 파일이 아니라 Tool의 메모리 초안을 읽는다.**
  "미리보기가 보인다 = authoring 파일에 저장됐다"조차 성립하지 않는다. Save → publish 두 단계를 모두 확인.
- `world.object.kouku.mario_clown`은 **Composition에서 참조 0회**다. 어떤 Pattern에도 Append되지 않아
  실제 전투 재생 중에는 뜨지 않는다. 현재는 저작·미리보기 전용 + NPC 경로만 제품이다.
- 이 Object의 `defaultMotionInstanceId`가 idle이 아니라 **attack**이다. Append하면 초기 상태가 공격 모션이 된다.
- **플레이어가 조종하는 변신 광대의 Q 뿅망치에는 이 이펙트가 안 붙는다.**
  `Clown.interactionbindings.json`의 MARIO 모드 skills[0]에 `effectAssetId`가 없다.
  붙이더라도 Character 경로는 **action 시작 시점**에 spawn하므로 '클립 끝' 의미가 아니다(별도 스케줄러 필요).
- **Codex에게 확인:** 사용자가 원한 '끝'이 **클립 종료 시점(1367ms)**인지 **타격 시점**인지.
  원본 notify는 0.724025초였고 Codex가 클립 끝으로 옮긴 것은 "사용자 요청 기반 프로젝트 튜닝"이라고 기록했다.
- **MvpResultView 회귀:** 결과 화면의 `Set_WeaponPartsVisible(true)`가 광기 광대/MAZE 상태에서 **no-op**가 된다.
  쿠크 클리어·MVP 연출에서 무기가 안 보이는지 같이 확인할 것.
- **CLAUDE.md와 코드 불일치:** CLAUDE.md는 광대 아바타 몸체를 `MN_RPCT_03`이라 적었지만
  실제 `Ensure_ClownBodyPrototype`이 admission하는 것은 `MN_RPCZ_00-1`(preScale 0.012053, `rpcz00p_*` 클립)이다.
  어느 쪽이 정본인지 정리 필요.

---

### G. 캐릭터 크기 ImGui 패널 (사용자 요청 7)

**핵심 원리 — catalog scale과 곱해지는 맵별 표현 배율**

`ARENA_CAMERA_PROFILE`에 `classSizeMultipliers`(7칸, 인덱스는 `CHARACTER_CLASS_ID` 순서, 4=DESTROYER 예약),
`clownSizeMultiplier`, `marioSizeMultiplier`가 추가됐다. 기본값이 **구조체에** 있다:
Artist 1.6, DimensionMaster 0.7, 광대 0.7, Mario 1.

```
Get_PresentationScale() = m_fPresentationScale(catalog) × m_fPresentationSizeMultiplier × characterSizeMultiplier × modelMultiplier
```

catalog의 Artist `presentationScale = 1.5`, DimensionMaster `1.05`는 **안 바꿨다.**
→ 유효 배율 **Artist 2.4 / DimensionMaster 0.735.** 광대는 spec의 `eCharacterClass == END`라
catalog scale이 1로 남아 유효 **0.7**이다.

Server/Shared 전체에 크기 참조 **0건**이고, Debug collider도 네트워크 transform을 쓴다. → **표현 전용 경계 지켜짐.**

**검토 절차**

1. F1 → `Player Follow Camera` → **`Character Size` 트리**(별도 패널이 아니라 tree node다).
   슬라이더 9개: `All characters` / `Lance Master` / `Gunslinger` / `Slayer` / `Artist` /
   `DimensionMaster` / `Warlord` / `Madness clown` / `Mario clown`, 버튼 `Requested size defaults`.
   기존 `Character size` 단일 슬라이더와 `Reset size` 버튼은 **제거됐다.**
2. **눈으로 재지 말고 숫자로 읽는다.** 트리 바로 아래
   `Catalog scale: %.3f | Current visual scale: %.3f`가 있다.
   도화가 = `1.500 / 2.400`, 차원술사 = `1.050 / 0.735`여야 한다.
   `1.600 / 0.700`이 나오면 catalog 곱이 빠진 것이므로 즉시 보고.
3. **Bern에서는 `Arena Camera / Player` 헤더 자체가 그려지지 않는다.** Bern은 `Player Follow Camera`만 렌더된다.
   두 헤더는 부모-자식이 아니라 형제다.
4. 맵별 분리 확인: 한 맵에서 조절한 값은 다른 맵에 전파되지 않는다(`Data/Camera/<맵>.camera.json` 4개).
   **네 맵 모두 같은 값을 원하면 네 맵에 각각 들어가서 각각 Save해야 한다.**
5. 전투 검증: `Player Skill Hit Geometry`(F1 → Live Combat Geometry)로 스킬 판정 도형이 캐릭터 크기와
   무관하게 유지되는지 본다. — **`Show Combat Colliders`라는 UI는 존재하지 않는다**(CLAUDE.md에만 남은 표현).
6. 이펙트가 캐릭터와 **같은 비율로** 줄어드는지 확인(presentation root가 이펙트 본 앵커도 스케일한다).
   차원술사 이펙트가 작아진 것을 별개 버그로 오인하지 말 것.

**주의**

- **`Data/Camera/*.camera.json` 4개는 아직 미변경이다.** 새 배율은 코드 기본값으로만 들어간다.
  즉 **이 빌드를 켜는 순간 Bern/Valtan/Kouku/Character Select 네 맵 전부에서 도화가가 2.4배, 차원술사가 0.735배가 된다.**
  ALT V나 뿅망치만 볼 게 아니라 "다른 맵 캐릭터 크기가 왜 바뀌었나"를 먼저 예상해야 한다.
- `Save camera settings` 한 번에 8필드 문서가 **15필드**로 재작성된다(`mazeHammer*` 3개 포함).
  "모르는 필드가 섞였다"고 오판하지 말 것.
- **`Set_MapPresentationSizeProfile`의 반환값을 8개 호출부 전부가 버린다.**
  프로필이 거부되면 크기가 조용히 "직전 맵 값"으로 남고 화면에도 로그에도 아무 표시가 없다.
  크기가 안 바뀌면 그 맵 JSON의 **카메라 필드**(pitch ±89, fov 등)가 범위 안인지 먼저 확인.
- **Character Select는 매 프레임 갱신되지 않는다.** `Bind_CameraTarget`이 캐릭터 객체가 바뀔 때만 실행된다.
  크기가 이상하면 class 썸네일로 한 번 바꿔(=캐릭터 교체) 재적용되는지 확인하는 게 정확한 재현 절차다.
- `Mario clown` 슬라이더만으로 1.5m가 보호되지 않는다. `All characters`도 그대로 곱해진다.
- `LEVEL::DEVELOPMENT`(Map Editor)와 `LEVEL::MAHARAKA`에는 적용 지점이 없어 **마지막 맵 값이 그대로 남고
  F1 패널도 뜨지 않아 되돌릴 방법이 없다**(로비 → 해당 맵 재진입 필요).
- `Set_PresentationSizeMultiplier()`가 이번 변경으로 **호출자 0개인 고아 함수**가 됐다.
  AGENTS.md의 "내 변경 때문에 안 쓰이게 된 함수는 제거한다" 대상이다.
- **CLAUDE.md 761~766행이 stale**이다(구 단일 슬라이더·`Reset size` 버튼 서술). 커밋 전 갱신 필요.
- **Codex에게 확인:** '도화가 1.6배'가 (a) 현재 화면 대비 1.6배(→최종 2.4)인지 (b) 원본 모델 대비 최종 1.6배인지.
  RESULT는 (a)로 해석했다. 차원술사 0.7도 동일.

---

### H. 칼날과 갈고리 (사용자 요청 6)

**핵심 원리 — Y 권위가 두 군데로 갈린다**

```cpp
// WorldSequencePlayer_Objects.cpp::Sample_ObjectWorld
const bool hasInstanceOffset = !((active.placement && instance.anchorKind == "WORLD") || anchor.emissionOverride);
```

**occurrence가 `placement`를 가지면 instance.position(= Object Tool의 Map Position)이 통째로 무시된다.**
같은 규칙이 Server 베이커 `world_object_collider.py::sample_object`에도 그대로 있다.

실측: **P31("3관문 칼날 바닥")의 5개 occurrence는 전부 `placement.position[1] == 1.3`**을 갖고,
P95("즉사 칼날 Object 판정 테스트")의 occurrence는 placement가 없다.
→ **Object Tool의 Parent Map Position(Y)은 P95에만 먹히고, P31/P33에는 Action Workbench Box Detail의 placement Y가 정본이다.**

**이펙트 끊김 원리:** 기존 `sourceCycle = floor(ageMs / preparedDuration)`인데 preparedDuration에는
particle/after-image **tail이 포함**되어 있다. 그래서 emission이 끝난 뒤 tail만 남는 공백이 매 주기 생겼다.
새 코드는 element별 emission 종료 시각을 계산해 `sourceCycleMs = min(emissionSeconds, preparedDuration)`을
재발생 주기로 쓰고, **이전 cycle tail과 다음 emission을 한 프레임에 동시에 유지**한다.

**검토 절차**

1. **선행: `Publish All Patterns` + Object Tool `Save`(Map publisher 자동 기동).**
   지금 상태(저장 1906 / 게시 1902, 맵 2136 / 런타임 2129)에서 `Play (with Collisions)`를 누르면
   `"Saved authoring and published KoukuSaydon revisions differ; use Publish All Patterns before Server Play."`로
   **무조건 거부된다.**
2. **Boss Tool의 `Reuse Gate 3 attacks on Encore` 체크박스를 반드시 끈다.**
   켜져 있으면 GATE3 + world occurrence를 가진 패턴(P31/P33/P95 전부)이
   `"This pattern owns a Gate 3 layout or mechanic. Disable Encore reuse to play it in Gate 3."`로 막힌다.
   원인 불명으로 헤맬 수 있는 지점이다.
3. 중앙 앵커: Object Detail의 `Parent Map Position` **Y만 드래그** → 칼날 8개가 간격을 유지한 채 통째로 이동.
   상태줄에 `"Moved all connected Map Motions. Preview now uses the authored Map Position."`
   **절대 좌표를 믿지 말고 delta로만 쓸 것** — 칼날 instance 4개 중 하나가 아레나 밖 레거시 좌표
   `[3.29, 8.64, -8.69]`라 표시 중심이 엉뚱하다(갈고리는 7개 중 5개가 그렇다).
4. **드래그하는 동안 아래 위젯(Transform/재질/Collider)이 사라지는 것은 정상**이다
   (`Render_MapAnchor`가 commit 성공 프레임에 true를 반환하고 호출부가 즉시 return).
5. Server 판정: 즉사 칼날 Motion 선택 → Sequencer 아래 `Server Collision Playback` →
   `Saved Pattern`에서 P95 선택 → `Play (with Collisions)`. 칼날 경로에 캐릭터를 세워 둔다.
   → INSTANT_DEATH는 방어/카운터 무시로 즉사.
   버튼이 회색이면 원인 5가지: 저장 안 됨 / Publish 중 / 패턴 미선택 / 이미 준비 중 / audition 진행 중.
   **Release 빌드에서는 무조건 거부된다**(`"KoukuSaydon Server Play is available only in Debug builds."`).
6. 갈고리: P33으로 `Play (with Collisions)`. 잡히면 GRABBED로 바닥에 붙은 채 grip 좌표를 따라 끌려가고
   방향이 +90도로 고정된다.
   **"즉시 풀렸다"를 곧바로 버그로 보지 말 것** — 유예 1.5초는 **보스가 살아 있고 `iAttachmentPatternSequence`가
   일치할 때만** 성립한다. 패턴이 끝나거나 보스가 죽으면 유예 0ms로 즉시 해제가 정상이다.
7. 이펙트 연속성: 11초 전체를 한 지점만 응시. 1초마다 새 emission이 시작되고 잔상이 겹쳐
   **밀도가 0에 가까워지는 구간 없이** 유지되어야 한다.
   **"4겹"이 아니라 "2겹"이 보여도 정상이다** — RESULT의 "tail 4초 / 4개 cycle"은 JSON 필드로 재현되지 않는다
   (필드 산식상 tail-inclusive end는 2.0초, 겹침 2개). 실제 값은 런타임 prepared duration이 결정한다.
8. 일반 vs 즉사: 즉사가 붉은 계열로 더 화려해야 한다(문서가 별도다 — `pjt_01` 8 element vs `pjt_02_loc_int` 9 element).

**미구현 / 사용자 결정 필요**

- **칼날 Y 실제 수치는 그대로다**(일반 0.33 / 즉사 0.22 / 마리오2 1.3). 도구만 생겼고 값 조정은 사용자 몫.
- **"즉사 칼날의 수치를 다르게"** — 현재 collider `halfExtents`는 양쪽 **동일**하다
  `[0.0805, 0.9799, 0.9799]`. 다른 건 damage 성격(DAMAGE 20% vs INSTANT_DEATH)과 effect 문서뿐이다.
  크기·개수·속도 중 무엇을 다르게 할지 결정이 필요하고, Object Tool의 즉사 Motion Collider 행에서 직접 저작해야 한다.

**주의**

- `placement` override 규칙을 "버그"로 보고 `hasInstanceOffset` 조건을 풀면
  **placement를 가진 모든 기존 World occurrence(카드·세토·커튼·룰렛·마리오 공)가 instance 좌표만큼 밀린다.**
  절대 건드리면 안 되는 경계다.
- `Server Collision Playback` 섹션은 **개별 Motion을 선택해야** 보인다. 부모 Object만 선택한 상태에서는
  안 보이는 게 정상이다.
- finite-cycle 수정은 V1 `loopEffectToDuration` 경로에만 들어갔다. native infinite emitter는
  자동으로 기존 경로로 되돌아간다(가드 확인됨). 다른 World Object 이펙트에서 끊김이 남으면
  그 문서에 infinite emitter가 섞여 있는지 먼저 볼 것.

---

### I. 빙고 (사용자 요청 8, 9, 10)

**핵심 원리**

**(a) 앵콜 회전:** `Render_KoukuEncoreRotation`이 `Gameplay.world.json`의 `boss.kakulsaydon.bingo.saydon`
`yawDegrees`(현재 151)를 편집한다. 저장은 전체 재직렬화가 아니라 **텍스트에서 두 숫자만 치환**
(placement 뒤 첫 `yawDegrees` + 파일 첫 `revision`)하고 `ReplaceFileW` 원자 교체한다.
라이브 프리뷰는 `Set_DebugPresentationYawOffset` — **표현 전용, Server 회전·판정 불변.**

**(b) 3관문 패턴 재사용:** BossCatalog의 `BOSS_KAKULSAYDON_BINGO_SAYDON` 무기가
`WP_MN_RPCT_06 / 0.00711685 / [23,8,8]` → `WP_MN_RPCT_05 / 0.01 / [-90,0,0]`으로 바뀌어
**G3_SAYDON과 완전히 동일**해졌다(bodyModel·animationSetId·bodyModelPreScale은 원래도 동일).
`bossArchetypeIds`는 bodyModel 조인이라 MN_RPCT_05 패턴은 **이미** 앵콜 archetype을 admit하고 있었다.
실제로 막던 것은 placement/gate 일치 검사였고, Server에 `encoreReplay` 예외가 추가됐다:
bundle이 아니고 + target이 bingo.saydon + 패턴이 GATE3 + g3.saydon +
**WorldSequences/SceneProfiles/MechanicTriggers/LogicWindows/BossMotion이 전부 비어 있을 때만** 통과.
→ 조건을 통과하는 GATE3 패턴은 **25개, 그중 23개가 이미 게시본에 있다.**

**(c) 빙고 이펙트:** 신규 V1 문서 3개(폭탄/망치 aura/메두사)가 EffectCatalog와 EffectResourceTree에 등록됐다.
폭탄의 실제 재생 연결은 코드가 아니라 worldsequences의 `sequence.kouku.bingo.bomb.planted`
effectTrack 2개(fuse TIME 0~2000 + explosion MOTION_END 3301ms)이고,
신규 objectResource `world.object.kouku.bingo_bomb.original`(scale `[1,1,1]`)이
기존 scale `[2,2,2]` 오브젝트를 대체했다 — **이것이 "원본 사이즈" 복구다.**
메두사는 신규 패턴 `KAKULSAYDON_G1_PATTERN_94`("메두사공포")에 배치됐다.

**(d) 월드오브젝트_빙고일반해골 — 전혀 저작되지 않았다.**
authoring worldsequences에 `"일반해골"` 0회, `"플립"` 0회다.
현재 빙고 해골 표현은 World Object가 아니라 **V2 decal**(`bingo.skull.red/white`)이고,
Server mask가 지울 때까지 유지되는 state다. "영구 재생되는 경계 이펙트"가 아니다.
재사용할 flip 원본은 `월드오브젝트_빈카드뒤집힘`/`조커카드뒤집힘`(667ms, clip `mn_rhoc_00_sk.ao_att_battle_1_start`)인데
**이 clip은 카드 모델 MN_RHOC_00 전용이라 해골 모델에 그대로 못 쓴다.**

**검토 절차**

1. 게시 후 F1 → KoukuSaydon Arena → `빙고 - 앵콜을 외친 쿠크세이튼` 버튼.
   세이튼이 스폰되고 **지팡이가 3관문과 동일한 크기·각도**여야 한다.
2. `Bingo Encore Rotation` 섹션에서 `Encore yaw (degrees)` 드래그 → 실시간 회전 →
   `Save Encore Rotation` → `Reload Saved Rotation`.
   **저장 성공 후에도 preview 플래그가 안 꺼져서 화면은 계속 새 방향으로 보이지만 Server는 옛 yaw를 갖고 있다.**
   `Reset Rotation Preview`를 눌렀을 때도 새 방향이면 진짜 적용된 것이다.
   실제 전투 반영은 World Gameplay 게시 + Server 재시작 이후다.
3. `Reuse Gate 3 attacks on Encore` 체크 → `빙고 | 앵콜세이튼 | 블랙홀빔`(P62) 또는
   `쿠크세이튼_십자화염폭발`(P40) → `Play Isolated`. 관문 이동·텔레포트 없이 그 자리에서 재생.
4. `Bingo_Bomb` 버튼: 폭탄이 **원본 크기(이전의 절반)**이고 심지 2초 + 파란 광역폭발.
   `Bingo_Hammer`: 내려오는 동안 aura가 5초간 따라붙는다.
   → 폭탄이 여전히 2배거나 aura가 없으면 **런타임 worldsequences가 2129에 머물러 있는 것**이다.
5. `메두사공포`(P94) Play Isolated → `rpct00_att_battle_32_*` 재생 중 1.0초부터 4.2초간 쿠크 얼굴 FX.
   (저작 문서 길이는 11100ms인데 배치가 4201ms라 도중에 끊기는 건 현재 저작값 그대로다 — 버그 아님.)
6. Object Resources에 `월드오브젝트_빙고일반해골`이 **없는 것이 현재 정상 상태**다.

**⚠️ 가장 중요한 회귀 위험 — projector의 stage 패딩 삭제**

`project_kouku_saydon_composition.py`의 diff는 "성능 memoization"만이 아니다.
**leaf Pattern의 마지막 Stage를 timeline 길이까지 늘려 주던 패딩 한 줄이 삭제됐다.**

```
-  stages[-1]["durationMs"] += parent["durationMs"] - sum(stage["durationMs"] for stage in stages)
```

현재 저작본 기준 `durationMs > stage 합`인 leaf Pattern이 **20개**다
(예: `PATTERN_25 쿠크_피자` stage합 18,794ms vs timeline 70,793ms; `PATTERN_43` 6,434 vs 12,554; 마리오 1/2페이즈 0 vs 15,000).
→ **다음 `Publish All Patterns`에서 이 20개 패턴의 보스 Stage 시계가 짧아지고 마지막 포즈 유지가 사라진다.**
새로 나가는 `timelineDurationMs` / `stageDurationMs` 필드를 읽는 C++ 소비자는 **어디에도 없다.**
**GATE1/2/3 전 게이트 회귀 위험이 게시 한 번에 실린다. Codex에게 의도를 반드시 확인할 것.**

**기타 주의**

- `Data/Encounters/KoukuSaydon/KoukuSaydonEncounter.json`은 clean이 아니라 **이미 수정된 상태**다
  (HEAD 1901 → 작업 트리 1902, `PATTERN_33`의 stages 17 → 19).
  `Publish All Patterns`를 누르면 이 미커밋 산출물이 통째로 교체되므로 게시 전후 git diff를 볼 것.
- `Patch_EncoreNumber`는 JSON 파서가 아니라 **텍스트 치환**이다. `"revision"` 앵커는 파일 최초 등장을 찾으므로
  World Gameplay 문서의 키 순서가 바뀌면 엉뚱한 값을 올린다.
- `Commit_EncoreFiles`의 안전 검사는 **파일 전체 바이트 동일성**이다. 다른 세션이 한 글자라도 바꿨으면
  yaw가 그대로여도 거부된다(데이터 손상 아님 — `Reload Saved Rotation` 후 재시도).
  저장 거부 문구 중 `"Encore source changed; Reload Baseline before saving."`의
  **`Reload Baseline` 버튼은 UI에 없다.** `Reload Saved Rotation`을 누르면 된다.
- `encoreReplay` 예외는 placement/gate 검사를 **통째로 건너뛴다.** 다섯 개 empty 검사와 bundle 제외를 반드시 유지해야 한다.
- Client는 **저장본**, Server는 **게시본**을 보고 encoreReplay를 판정한다. revision이 어긋나면 한쪽만 통과한다.
- `KAKULSAYDON_G1_PATTERN_94`가 DRAFT인 것은 게시 차단 사유가 **아니다**(`_publication_candidate`가 PRODUCT로 승격).
  목록에 안 뜨면 상태가 아니라 validate 실패이고 사유는 inventory의 `unavailableReason`에 있다.
- 신규 테스트 패턴 `KAKULSAYDON_G1_PATTERN_95`("빙고 칼날 | Object 연결 테스트")도
  `Publish All Patterns`가 선택·필터와 무관하게 **전체 트리를 게시**하므로 제품 게시물에 실린다. 의도인지 확인.
- **저작 worldsequences는 2136인데 런타임이 2129다 = Object Tool Save를 거치지 않고 파일을 직접 편집했다는 뜻이다.**
  Action Workbench → Object를 열 때 Tool이 외부 변경(2136)을 다시 읽었는지 먼저 확인할 것.
  이전 draft를 들고 있으면 Save가 폭탄 effectTrack·망치 aura·`bingo_bomb.original` 바인딩을 **되돌릴 수 있다.**

---

### J. 차원술사 ALT+V (사용자 요청 11)

**핵심 원리**

제품 target은 `effect.dimensionmaster.skill.2050540.**full**.restore.effect.json`(312 elements) **하나**다.
`...tuning.restore...`(27 elements)는 **EffectCatalog.json에 없어서** 런타임에 안 올라온다 —
거기를 고쳐도 인게임 ALT+V는 안 바뀐다.

방향별 수축 속도·회전·정사각형·model center의 **기능 본체는 09-19에 이미 커밋돼 있었고**,
오늘 Codex가 한 일은 **그 기능에 실제 값 3개를 저장한 것 + 프리셋 버튼 추가**다:
`captureRotationDegrees: 45`, `captureSquare: true`, `captureUseModelCenter: true`.

가장 큰 구도 변화는 **`captureUseModelCenter`**다.
```cpp
if (!post.bCaptureUseModelCenter) snapshot.vCaptureDestinationUV = {.5f, .5f};
```
→ 축소 종점이 **화면 중앙이 아니라 큐브의 투영 위치**가 됐다.

셰이더(`PS_SCENE_COLLAPSE`)에서 `angle = radians(rot) * progress`이므로
**45도는 시작부터 고정이 아니라 진행과 함께 0 → 45로 커진다.**
그리고 `captureUV` **한 개**를 `g_CapturedSceneColor`와 `g_CapturedSceneBloom` **둘 다** 샘플하므로
Color와 Bloom에 같은 회전·crop이 적용된다.

**검토 절차**

1. ALT+V (전체 5.1초, 캡처 0~2.0초, 큐브 2.0~3.95초): 0초에 스냅샷 → 2초에 걸쳐 기울어지며 정사각형으로
   줄어들고 **화면 중앙이 아니라 큐브가 나타날 자리로 수렴** → 2.0초에 큐브가 같은 이미지를 이어받는다.
2. F1 → Effect Tool V1 → All Effects → `이펙트_차원술사AltV_전체` → 맨 아래
   `ALT V Starting Scene Capture to Cube` → `Presentation Screen Post` 패널:
   Rotation **45.00**, Square Capture 체크, Use Model Center 체크.
3. 압축 방향 테스트: `Left Edge Speed` 3.0 / `Right` 0.3 → Apply → 재생.
   Left가 먼저 들어오고 Right가 늦게 따라오되 둘 다 같은 시각에 종점 도달.
4. Bloom 동시 회전: 밝은 곳에서 ALT+V → 번짐이 회전한 액자 **안쪽에만** 따라 돈다.
5. **`tuning` 문서는 건드리지 말 것.** 현재 dirty이고 매니페스트가 해시를 고정해 뒀다.
   제품 정본이 full.restore임은 `EffectCatalog.json`에 `2050540`이 line 250/1500 두 군데만 있고
   tuning은 0건이라는 grep으로 증명된다.

**주의 / 미완료**

- **큐브 색·투명도는 cue tint가 아니라 ALT178 native 재질이 결정한다.**
  제품 문서의 cue는 `opacity 1, colorMultiply [1,1,1,1]`이다.
  (`opacity 0.22 / [0.3,0.65,1,1]`은 런타임에 안 올라오는 tuning 문서 값이다.)
  → **"파란 반투명이 아니다"를 회귀로 오판하지 말 것.**
- `ScreenPost 종료(0+2.0) == 큐브 cue 첫 pose(2.0)`를 1e-4로 강제한다. **한쪽만 바꾸면 문서 전체 Load가 거부되어
  ALT V가 통째로 사라진다.** 이 검사는 Load뿐 아니라 매 프레임 런타임에서도 다시 강제된다
  (큐브 cue의 visible을 끄거나 시네마틱 카메라가 큐브를 near plane 뒤로 보내면 액자가 사라진다).
- `captureSquare=true`는 16:9에서 좌우를 UV.x `[0.21875, 0.78125]`로 center crop한다.
  → **"원작 액자 전체를 보여 달라"는 요구와 직접 충돌한다.** 전체가 목표면 square를 끄고 다른 수단이 필요하다.
- `captureUseModelCenter`는 매 프레임 재투영하므로 ALT V 중 카메라 전환 구간에서 **종점이 미끄러지거나 떨릴 수 있다.**
  다만 끄더라도 **중심만 고정되고 크기 떨림은 남는다**(크기는 원래부터 매 프레임 재계산).
- **미완료 3항목(Codex RESULT 원문):** ① 숨겨진 source camera mesh 18/31 복구(여전히 `visible: false`),
  ② 원작 capture camera CB 해석(`source[2]=1.f; // native capture-view constants were not exported.`),
  ③ 액자 전체와 2D→3D UV/구도의 완전한 연속성.
  큐브 면은 캡처 텍스처를 자기 카메라 UV로 **1.15배 타일링**해 읽으므로(`capture_centeruvtile = 1.15`,
  `00_alpha01_usecamuv = true`) 2D 액자와 3D 면의 구도는 설계상 자동 일치하지 않는다.
- **45도는 원작 값이 아니라 사용자 첨부 이미지를 보고 맞춘 저작 튜닝**으로 읽는 것이 맞다.
- ALT V 회귀가 나더라도 `Effect_ShaderFamily.h`와 `Effect_ArtistMaterial_Tables.inl`의 diff는
  **전부 쿠크**(PARTICLE row 이동, native program 2616~2619 추가)이므로 원인 후보로 지목하지 말 것.

---

### K. 발탄 연출 복구 (사용자 요청 14)

**핵심 원리 — 두 정본을 Server stage clock 위에 얹었다**

- 카메라 정본 = `Data/Encounters/Valtan/ValtanCinematicCamera.json`
- 배우·FX 정본 = `Data/Maps/Authoring/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.worldsequences.json`

두 정본을 잇는 것이 `Level_ValtanArena`의 신규 4함수다. **world sequence는 자기 시계를 돌리지 않고
카메라 컨트롤러의 action clock으로 seek된다.** → **카메라 cue가 선택되지 않으면 그 연출의 원본 FX도 재생되지 않는다.**

`Ready_SourceCinematics`가 준비하는 instance는 `{ entrance, entrance.colorless, trash, finale }` **4개뿐**이다.

파서 변경이 데이터의 **하드 선행조건**이다: `MAX_KEYFRAME_COUNT` 64 → **512**,
keyframe optional `up`/`cutBefore` 허용, `Read_Fov` [10,120] → (1,179).
→ **새 JSON을 구 Client에 주면 발탄 카메라가 전멸한다**(문서 전체 파싱 거부).

**실제 변경 내역**

| 항목 | 이전 | 현재 |
|---|---|---|
| cues | 11 | 15 (wall-break 2, trash 2 추가) |
| `entrance.establish` | 8600ms CATMULL_ROM 7키 | LINEAR 67키 |
| `entrance.arena-reveal` | 5800ms 5키 | 46키 (cut 1) |
| `entrance.hero-handoff` | 4467ms 5키 | **10308ms 198키** (cut 1) |
| `deathCue clear.wide` | 2090ms 3키 | **23000ms 161키** (cut 3) |
| worldsequences effectTracks | 0 | **17개** (entrance 3 / trash 3 / roar 6 / finale 5) |
| finale 배우 | `...source-preview.body` | `...source-preview.ghost` |

**2페이즈(`VALTAN_ARENA_BREAK_109`) 6개 cue는 손대지 않았다.**

**검토 절차**

1. **Client와 Server를 같은 소스로 함께 빌드**한 뒤 Valtan 입장 → 진입 컷씬
   (설계상 8600 + 5800 + 10308 = **24.708초**). 카메라가 컷 전환하며 **기울기(roll)까지 보존**되는지.
2. 6방향 피자(`VALTAN_SIX_PIZZA_106`) STEP_04(2.8초) → STEP_05(3.924초):
   **STEP_05 안에서 2회 컷 지점에 보간 없이 즉시 점프**해야 한다. 부드럽게 훑으면 구 Client 바이너리다.
3. 버러지(`VALTAN_TRASH`) STEP_05(0.667초) → STEP_06(5.707초):
   보스 **왼손(`bip001-l-hand`)에 붙은 FX 2종**이 손을 따라 움직인다.
   STEP_06이 **4.1초에 끊기면 Server가 아직 구 duration(4100)으로 도는 것 = Gameplay 재게시 누락.**
4. 사망: **23초 finale 후에야** 클리어/MVP UI가 뜬다. 배우는 **유령 모델**(MN_RPBF_02)이다.
5. 실패 증거는 파일 로그가 아니라 **출력창의 `[ValtanSourceCinema]` / `[Loader][ValtanSourceCinema]`** 문자열이다.
6. **2페이즈 진입 '포효' 연출을 기대했다면 안 나온다.**
   `world.sequence.instance.valtan.source-preview.roar`(7003ms, 오늘 FX 6개 설치됨)와
   `gate1-entrance` 계열이 `Ready_SourceCinematics`의 4개 목록에 **없어 미연결**이다.
   사용자가 말한 "2페이즈 진입 컷씬"이 이 roar인지 확인이 필요하다.

**⚠️ 중요 정정 — Server 게시 누락의 실제 증상**

Client는 `ValtanEncounter.json`(11:45, 이미 새 값)을 읽고, Server는 `Gameplay.bootstrap`(09:06, 옛 값)을 읽는다.
→ **Client는 hero-handoff 10308 / trash step-06 5707로 정상 파싱되는데 Server는 5467 / 4100에서 stage를 넘긴다.**
사용자가 볼 증상은 **"컷씬이 중간에 툭 끊기고 follow로 복귀"**이고,
**이때 Client 로그에는 아무 실패도 남지 않는다.**
(생성 receipt는 Server manifest가 아니라 Client가 디스크 파일을 직접 해시하므로,
"Data JSON을 고쳤다는 이유만으로 발탄 표현이 거부되지는 않는다.")

**주의**

- **카메라 문서 파싱은 all-or-nothing이다.** keyframe 512 초과, `up`이 forward와 평행,
  fov ≤1 또는 ≥179, cue duration > stage duration, cue 32개 초과, sceneId 중복,
  **stage tuple(`patternId + stageIndex + actionId`) 중복** 중 하나라도 걸리면 문서 전체가 무효가 되어
  **손대지 않은 109 2페이즈 카메라까지 전부 사라진다.**
- **발탄 입장 로딩이 재생되지 않는 연출의 FX까지 전부 미리 읽는다.**
  `Try_CollectPreparedAreaV1EffectTargets`가 Area의 **모든** 템플릿을 순회하므로
  쓰이지 않는 roar 6개와 gate1 계열도 준비 대상이다. 하나라도 실패하면 `Prepare_AreaLoad`가 실패하고
  **입장/버러지/사망 컷씬 4종이 전부 조용히 꺼진다.** 로딩이 느려졌거나 컷씬이 하나도 안 나오면 이 경로를 먼저 의심.
- `ValtanCinematicCamera.json`은 순수 추가가 아니라 **17,471줄 추가 + 646줄 삭제**다.
  기존 카메라 값이 교체됐을 수 있으므로 "신규라 기존 연출에 영향 없음"으로 읽으면 안 된다.
- entrance 3개 cue의 trackingMode가 `BOSS_FACING` → `WORLD`로 바뀌었다.
  **보스 spawn 위치가 원본과 다르면 프레이밍이 어긋난다.** 의도인지 확인 필요.
- 투영기는 `cameraCueId`가 실제 카메라 문서에 있는지 **검사하지 않는다.**
  없는 ID를 넣어도 검증을 통과하고 런타임에서 조용히 아무 일도 안 일어난다.
- `camera.valtan.source.wall-break.*` cue를 만든 **생성기가 Tools에 없다.** 손수정 데이터일 가능성이 있어
  나중에 원본을 다시 굽는 작업에서 덮어써 잃기 쉽다.
- `restore_valtan_original_cameras.py`가 `entrance offset == 24708`, `trash offset == 6374`를 assert한다.
  gameplay의 HERO_HANDOFF(10308)나 TRASH STEP_06(5707)을 다시 조정하면 스크립트가 즉시 실패한다.
  **두 값은 한 쌍으로 다뤄야 한다.**
- `ValtanEncounter.json`이 바뀌었으므로 `VALTAN_ARENA.worlddestructionbootstrap`(09-19 01:07)이 stale이다.
  `Publish-ValtanWorldDestruction.ps1` 재실행 여부를 Codex에게 확인할 것.
- **컷씬 배우의 '몸동작'은 아직 원본이 아니다.** 신규 PLAN `VALTAN_CINEMATIC_ANIMATION_BAKE_PLAN.md`(11:57)와
  `bake_valtan_original_cinematic_actors.py`(12:00)가 방금 시작됐고, 현재 entrance/finale 배우는
  기존 146클립 AnimSet의 근사 조합(`mesh_idle_normal_1`, `mesh_abn_groggy_1_*`, `mesh_walk_normal_1` 등)이다.
  **"동작이 원본과 다르다"는 현시점에선 정상이며 버그로 보고하지 말 것.**

---

### L. 유령 발탄 + 삼각형 1.5배 (사용자 요청 15)

**핵심 원리 — 두 에러의 정확한 원인**

**① "no saved Effect cue or combat-object visual"**
`Effect_Tool_Valtan.cpp`의 `erase_if`가 `ProductSources.empty() && CombatObjectStages.empty()`인 행을 전부 지운다.
`VALTAN_GHOST_RESPAWN_AUDITION`의 유일한 stage STEP_01은 `"effectCues": []`였다 → 행 0개 → 에러.
→ Codex가 `cue.valtan.ghost-respawn.original`을 추가했다.

**② "No saved Full Restore matching this pattern's animation clips"**
`Is_ValtanPatternFullRestoreSource()`가 세 조건을 AND로 요구하는데, 두 번째가 **영구히 실패**했다:
이 pattern의 `sourceActionIds`는 `[420624]`인데 원본 Full Restore는 **Action 15 / stage 2** 소유다.
→ Codex가 `ActionMatches || **CueMatches**`로 완화하고
`ValtanFullRestoreAnimations.json`에 `mesh_respawn_1 / 3000ms` 항목을 추가했다.

**③ 이펙트 문서가 없었던 이유**
Action 15 / stage 2 / notify-002는 base `CEFParticleData`의 ParticleSystem이 **null**이고
뒤따르는 `CEFParticleDataModifier`가 유령 몸체와 `Par_N_RPBF_Spawn_Cast_01`을 소유하는 특수 레이아웃이다.
공용 디코더가 modifier 파라미터 테이블을 transform으로 **오독**했다.
→ `decode_valtan_particle_notify()`가 바이트 오프셋과 sha256을 assert로 못박은 1건 한정 예외 경로로 추가됐다.
결과 문서는 915,861 bytes, particle element 16개다. **이건 "유령을 만들어 내는 스폰 캐스트 파티클"이지 유령 모델이 아니다.**

**⚠️ "유령 발탄 생성 실패"는 세 경로가 전혀 다르다**

| 경로 | 소유자 | 이번 수정 |
|---|---|---|
| (a) Effect Tool preview 몸체 | `Set_LocalPreviewGhostPresentation`(신규) | **고침** |
| (b) 아레나 3페이즈 본체 | `phase >= 3`에서 Client part-group 교체 | **원래 설계 그대로, 미변경** |
| (c) finale 포탈 주자 3체 | Server `Update_ValtanGhostPortalScheduler`(진짜 entity) | 상수만 갱신 |

**Codex의 diff는 (a)만 직접 고쳤다.** 사용자가 말한 "생성 실패"가 어느 것인지 확인이 필요하다.
(b)는 Server spawn이 아니라 Client part-group 교체이고, 실패해도
`OutputDebugStringA("[Client][Valtan] phase presentation swap isolated: ...")`로만 격리된다(화면·로그에 안 뜸).

**삼각형 9m → 13.5m:** 정본은 `Valtan.gameplay.json`의 `radiusM`.
변 `23.3826859022m`, 속도 `17.9866814632 m/s`(정확히 1.5배)로 **이동 시간 1.3초가 보존**된다.
소비자 4곳 동기 확인: Server 상수 / Client oracle(`PORTAL_TRIANGLE_RADIUS_M = 13.5`) /
`ValtanEncounter.json` / `ValtanCombatObjects.json`.

**검토 절차**

1. F1 → Effect Tool V1 → All Effects → `3페이즈 망령화 발탄 부활` →
   `Saved Pattern Effects`에 `effect.valtan.action.15.stage002.full.restore` 행 1개.
2. `Full Restore` 섹션에 `[FULL RESTORE]` 노드 → `Source animation: mesh_respawn_1`.
3. `Play Effect + Animation` → preview 몸체가 유령(MN_RPBF_02)으로 교체되고 3초 클립 위에 파티클 16개.
   **preview 보스가 바인딩돼 있지 않으면 아무 메시지 없이 통째로 건너뛴다.**
   "몸체가 안 바뀌는데 오류 문구도 없다"는 실패가 아니라 미바인딩일 수 있다.
4. **회귀 확인 필수:** 유령 재생 직후 **곧바로 다른 발탄 패턴**(예: `VALTAN_SIX_PIZZA_106`)을 재생한다.
   유령 교체는 양방향 강제라 비-`mesh_respawn_1` 재생마다 `BOSS_VALTAN`으로 되돌리고,
   유령 직후 첫 일반 재생은 part 전체를 다시 clone한다.
   `"Valtan presentation part resources are unavailable: BOSS_VALTAN."`가 뜨면 발탄 도구 전체가 막힌 것이다.
5. **기존 패턴 회귀:** worldScale 1.5 cue를 쓰던 기존 항목을 하나 열어 Play까지.
   `Reload_PatternEffectCues` / `Load_ForProductPrewarm`은 **all-or-nothing**이라
   신규 cue 하나가 거부되면 **발탄의 모든 pattern effect가 죽는다.**
6. 삼각형: **publish를 건너뛰면 `PATTERNSTAGEVOLLEY ... RADIAL 9`가 그대로라
   데미지 미사일은 9m, 유령 주자 3체는 13.5m에 서는 "이중 삼각형"이 보인다.**
   이게 가장 알아보기 쉬운 게시 누락 징후다.

**주의**

- **`worldScale` 가드 완화가 발탄 split presentation cue 전체에 적용된다.**
  이전에는 정확히 1.5가 아니면 거부돼 오저작이 즉시 드러났지만, 이제 0.000001~1000이면 통과한다.
  0.01이나 100을 실수로 저장해도 막히지 않고 런타임에서 거대/미세 이펙트로 나타난다.
- **삼각형 외에 Server가 소비하는 stage duration도 함께 바뀌었다:**
  HERO_HANDOFF 5467→10308, center-trash-rush-if STEP_06 4100→5707, endPolicy EXACT→HOLD_LAST_POSE.
  → **게시 후 검증에 입장 컷신과 버러지 패턴 재생을 반드시 포함할 것.**
- `PATTERNSTAGEVOLLEY` 파서에 radius 상한이 없다(양수·유한만 검사).
  아레나 벽이나 navigation 밖으로 나가도 게시는 통과한다. 주자는 navigation을 걷지 않는다.
- Client의 `EncounterPatternReference.cpp:596`이 radius를 **1e-6 오차로 13.5에 못박았다.**
  `ValtanEncounter.json`을 옛 소스로 재생성하면 Client가 encounter 문서 전체 Load에 실패한다.
- **BossCatalog의 유령 scale(`bodyModelPreScale 0.01`, `presentationScale 1.4`)은 이번에 안 바꿨다.**
  유령 크기 불만이 남아 있다면 별도 요청이 필요하다.

---

## 3. 검증 절차를 틀리면 버그로 오인하는 것들 (요약)

| 증상 | 오판 | 실제 |
|---|---|---|
| 혼자서 카드 받았는데 문양이 그대로 | "수정 안 됨" | `used==0`이면 옛 코드와 비트 동일. 2인 이상 필수 |
| F1 관문 버튼 눌렀는데 2관문 컷씬 안 나옴 | "light 수정 실패" | shot이 `PATTERN_ONLY`. PATTERN_73을 재생해야 함 |
| F6 자유 카메라에서 fog/light 안 바뀜 | "override 실패" | composition camera 자체가 비활성 |
| Complete Play 돌렸는데 카드미로 망치 없음 | "anchor 버그" | world.40은 PATTERN_77에만 있고 Flow에 없음 |
| Object Tool에서 칼날 Y 올렸는데 P31이 그대로 | "앵커 버그" | placement override 규칙. Box Detail Y가 정본 |
| 칼날 이펙트 겹침이 4겹이 아니라 2겹 | "수정 미흡" | JSON 필드 산식상 2겹이 맞음 |
| 빙고에서 칼날 패턴이 막힘 | "게시 실패" | `Reuse Gate 3 attacks on Encore` 체크박스 |
| ALT V 큐브가 파란 반투명이 아님 | "회귀" | 제품 cue는 opacity 1. 색은 ALT178 재질 소관 |
| Bern에서 `Arena Camera / Player` 없음 | "패널 누락" | Bern은 `Player Follow Camera`만 렌더 |
| `Show Combat Colliders` 체크박스 없음 | "제거됨" | 원래 없음. CLAUDE.md에만 남은 표현 |
| 광대 동작이 원본과 다름(발탄 컷씬) | "복구 실패" | 애니메이션 베이크가 아직 시작 단계 |
| 발탄 컷씬이 중간에 툭 끊김 | "카메라 버그" | Server가 옛 stage 길이. Client 로그에 흔적 없음 |
| 카드 그룹 10초 뒤 사라짐(저작본) | "수명 버그" | Client가 런타임에 durationMs 0으로 덮어씀 |
| 앵콜 회전 Save 후 화면이 바뀜 | "적용 완료" | preview일 뿐. `Reset Rotation Preview` 후에도 새 방향이어야 진짜 |

---

## 4. Codex에게 반드시 물어야 할 것

1. **`RAID_PRESENTATION_REPAIR_IMPLEMENTATION_PLAN.md`의 RESULT가 아직 없다.**
   (다른 6건은 RESULT가 있다.) 어디까지 끝났는지 확정 필요.
2. **projector의 leaf Pattern stage 패딩 삭제가 의도인가?** 20개 패턴의 보스 Stage 시계가 짧아지고
   마지막 포즈 유지가 사라진다. 새로 나가는 `timelineDurationMs`/`stageDurationMs`를 읽는 C++ 소비자가 없다.
3. `KAKULSAYDON_G1_PATTERN_95`("즉사 칼날 Object 판정 테스트")가 제품 게시물에 실리는 게 의도인가?
4. 마리오 광대 망치 FX의 '끝'이 **클립 종료(1367ms)**인가 **타격 시점(원본 notify 0.724초)**인가?
5. 카드: 전투 진입 시 사망 중이던 플레이어를 부활 후 재배분할 것인가?
   룰렛 회차마다 재배분할 것인가(PacketMessages.h 주석과 구현이 정반대)?
6. 카드미로 **LMB가 망원경 claim·상자 파괴까지 하는 게 의도인가?** HUD 문구와 CLAUDE.md는 Q 전용으로 쓰여 있다.
7. LMB 12tick이 실측 기반인가? (RESULT가 "프로젝트 튜닝"이라고 인정. 30tick으로 올린 기존 주석과 충돌)
8. `ValtanPatternAuditionServiceHarness.exe`(11:47 빌드됨) **실행 결과가 무엇이었나?**
9. `Publish-ValtanWorldDestruction.ps1` 재실행 계획이 있는가? (산출물 09-19 01:07로 stale)
10. 발탄 `roar`(7003ms, FX 6개 설치됨)와 `gate1-entrance`를 연결할 계획인가?
    사용자가 말한 "2페이즈 진입 컷씬"이 이것인가?
11. `world.object.kouku.mario_clown`을 Pattern에 Append할 계획인가? (현재 Composition 참조 0회)
12. 문서 갱신: CLAUDE.md의 Character size 절(stale), 광대 몸체 MN_RPCT_03 vs MN_RPCZ_00-1,
    뿅망치 12/30tick 서술, `PacketMessages.h:869` 주석.

---

## 5. 사용자 요구 중 아직 구현되지 않은 것

| 요청 | 상태 |
|---|---|
| 빙고 `월드오브젝트_빙고일반해골` Parent / `일반해골_유지` / `일반해골_생성_플립` | **전혀 저작 안 됨** (문자열 0건). 해골용 flip clip 선정 필요 |
| 카드미로·마리오 **플레이 구간** fog off | 미구현 (cameraTrack 없는 shot 20개). 컷씬만 커버됨 |
| "전투 중에만 fog" / "시작 위치에서만 fog" 계약 | 미구현. region 스키마에 `enabled` 키가 없어 데이터만으로는 불가 |
| 1관문 블룸 축(`bloomIntensityMultiplier` 0↔1) | 미수정. exposure만 고침 |
| 3관문 `g3.dark.v1` 실효 노출 2 | 미수정 (범위 밖) |
| 칼날 Y 실제 수치 조정 | 도구만 생김. 값은 그대로 |
| 즉사 칼날 collider "수치를 다르게" | `halfExtents` 양쪽 동일. 저작 필요 |
| 카드미로 LMB/Q 이펙트 | 빈 저작 슬롯 2개 제공됨. **사용자가 채울 차례** |
| 유령 발탄 (b) 아레나 3페이즈 본체 / (c) finale 주자 | 원래 설계 그대로. (a) preview만 수정됨 |
| 발탄 컷씬 배우 원본 애니메이션 | 베이크 도구가 12:00에 막 시작됨 |
| ALT V 액자 전체 / 원작 capture CB / camera mesh 18·31 | 미완 (RESULT가 명시) |
