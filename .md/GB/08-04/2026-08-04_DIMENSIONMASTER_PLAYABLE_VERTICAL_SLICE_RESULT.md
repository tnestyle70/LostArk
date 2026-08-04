# 2026-08-04 DimensionMaster playable vertical slice RESULT

## 0. 결론

차원술사(`CHARACTER_CLASS_ID::DIMENSIONMASTER = 5`)를 기존 Character Select → Server 입장 승인 → remote spawn → `CCharacter` 표현 경로에 연결했다. 별도 NetworkClient나 class 전용 packet 구조체를 만들지 않고, 기존 `C2S_ENTER_WORLD`, `SERVER_PLAYER::eCharacterClass`, `S2C_PLAYER_SPAWNED`, `CClientReplication` 계약을 확장했다.

로컬 실행에 필요한 Character/Animation/Effect payload는 요청한 Resources 경로에 있다. 다만 사용자가 immutable resource pack `2026.08.03.4` 제작을 폐기했으므로 `.3`만 Hydrate한 다른 PC에는 DimensionMaster payload가 없다. 따라서 이번 결과는 로컬 수직 슬라이스이며 팀 배포 가능한 immutable pack 완료로 기록하지 않는다.

## 1. 구현 상태

### 1.1 Shared와 네트워크

- Shared protocol을 v6으로 올렸다.
- 기존 `DESTROYER = 4` wire 값을 보존하고 `DIMENSIONMASTER = 5`를 추가했다.
- 지원 predicate, enter-world와 player-spawned round trip, raw value `0x05`, reserved class 거부를 하네스에서 검증한다.

### 1.2 Character catalog와 런타임

- `Data/Actors/CharacterCatalog.json`에 `PLAYER_DIMENSIONMASTER`를 추가했다.
- body는 `Character/DimensionMaster/DimensionMaster_Character.wmodel`이다.
- DimensionMaster 추출물은 장비/무기 분리형이 아니라 combined body이므로 actor admission이 다음 두 완전한 표현만 허용하도록 확장했다.
  - 기존 네 class: body + equipment + weapon
  - DimensionMaster: combined body only
- `CPlayableCharacterAssetService`가 class별 equipment 개수와 optional weapon을 검증하고 같은 `CModel -> CMaterial` 경로로 prototype을 admission한다.
- `Spec_DimensionMaster`의 기본 표현 clip은 다음 실물 이름을 사용한다.
  - `pc_sp_m_00_sk_idle_battle_1`
  - `pc_sp_m_00_sk_run_battle_1`
  - `pc_sp_m_00_sk_dmg_idle_1`
  - `pc_sp_m_00_sk_dead_1`
- 현재 제품 snapshot action에는 HIT 의미가 없으므로 HIT 자동 재생 완료로 주장하지 않는다.

### 1.3 Character Select와 Server

- Character Select roster와 Loader를 다섯 class로 확장했다.
- 선택 이름, Lobby handoff, MainApp 표시와 player-skill catalog class parser에 DimensionMaster를 추가했다.
- `PlayerProfiles.json`에는 HP 5000, resource 100, move speed 6.0의 명시적 training baseline을 추가했다.
- Server gameplay catalog/publisher가 DimensionMaster profile을 읽는다.
- Server에는 에셋 경로나 별도 class 전용 구조체를 추가하지 않았다. 기존 typed class field가 정본이다.

## 2. 로컬 에셋 상태

### 2.1 Character와 Animation Tool

경로: `Client/Bin/Resources/Character/DimensionMaster`

| 종류 | 개수 |
|---|---:|
| `.wmodel` | 3 |
| `.tga` | 51 |
| `.dds` | 20 |
| 합계 | 74 files / 146,400,423 bytes |

- `DimensionMaster_Character.wmodel`: skeleton 있음, 10 meshes, 154 animations
- `DimensionMaster_DimensionCore.wmodel`: skeleton 있음, 1 animation
- `DimensionMaster_DimensionSummon.wmodel`: skeleton 있음, 2 animations
- Animation Tool target은 Character/Core/Summon 세 개이며 WModel 자체 clip table을 그대로 열거한다.
- authored 시작점은 `Data/Animation/Authored/DimensionMaster/DimensionMaster.animevents`이고 현재 event 수는 0이다.
- `Data/Animation/Reference/DimensionMaster`의 `.animnotify`, `.clipmap`, `.clipseq`, `.skilltiming`은 기존 Tool과 같은 포맷의 0-row 정식 컨테이너다. 원본 Action `.loa`를 확보하지 못했으므로 row는 추가하지 않았으며, 파일 존재를 추출된 notify/skill timing 증거로 취급하지 않는다.

### 2.2 Effect Tool

경로: `Client/Bin/Resources/Effect/DimensionMaster`

| 종류 | 개수 |
|---|---:|
| `.dds` | 693 |
| `.wmodel` | 139 |
| 합계 | 832 files / 90,608,380 bytes |

`Data/Effects/Authored/DimensionMaster/Candidates`에는 schema 5 `.effect` 459개가 있다. admission 상태는 의도적으로 `candidate_only`다. 외부 Cascade module import, 일부 material reference, disabled emitter를 해결하지 않았으므로 skill/animation notify에 자동 연결하지 않았다.

현재 툴 저장 형식은 다음과 같다.

- `.effect`: line-oriented text authoring 형식
- `.weffect`: cook된 binary runtime 형식
- JSON이나 `.wfx`를 이 Effect Tool의 저장 형식으로 사용하지 않는다.

## 3. 검증 결과

### 자동 검증

- Debug/Release Engine, Shared, NetworkProtocolHarness, ClientFrontendHarness, Server, Client: PASS
- Debug/Release NetworkProtocolHarness: `failures : 0`
- Debug/Release ClientFrontendHarness: `failures : 0`
- Debug/Release `Server.exe --contract-test`: `failures : 0`
- gameplay balance validate/publish: 5 player profiles를 포함해 PASS
- ModelAssetConverter info: body 154, core 1, summon 2 animations와 skeleton 확인
- 대표 `par-j-swp-dimensionthrust-impact-01-01.effect`: text Load/Save/Reload와 `.weffect` Cook/BinaryLoad round trip PASS
- Character/Effect JSON parse와 Client project/filter XML parse: PASS
- `git diff --check`: PASS
- Debug local runtime: Server `127.0.0.1:7777` listener와 Client가 20초 동안 생존·응답함을 확인하고 시작한 PID만 종료

### ProjectAudit 경계

Debug/Release ProjectAudit는 각각 64개 중 63개를 통과했고 최종 결과는 동일한 1건 실패다.

```text
asset-lock.inventory: files=10158 bytes=5393658360
```

원인은 `.3` immutable manifest 이후 로컬 Character/Effect/Map payload가 추가됐기 때문이다. 폐기한 `.4`를 다시 만들거나 audit를 약화시키지 않았다. 이 실패는 로컬 에셋이 immutable 배포 정본에 없다는 현재 상태를 정확히 표시한다.

### 수동 검증 미완료

다음 GUI smoke는 자동 하네스로 대체할 수 없어 아직 수동 확인 증거가 없다.

- `Framework.slnLaunch` Server + Client → Character Select → DimensionMaster 실제 렌더/선택 → Lobby → Test 입장
- 두 Client에서 DimensionMaster remote spawn 표현
- Animation Tool에서 Character/Core/Summon target과 clip 재생
- 대표 candidate를 Effect Tool UI에서 열고 texture/model resolve 확인

## 4. 비평 반영과 남은 경계

비평 에이전트 지적을 실물 코드/데이터로 다시 확인해 다음을 반영했다.

- `.3` pack에 DimensionMaster가 없음을 public 문서와 이 RESULT에 명시했다.
- 존재하지 않는 Local Preview 검증 문구를 제거하고 실제 Server + Client 경로로 고쳤다.
- body-only DimensionMaster와 기존 modular class의 admission 차이를 명시했다.
- Action reference 4종을 0-row 정식 컨테이너로 명시하고 추출 데이터로 오인하지 않도록 문서와 audit 의미를 제한했다.
- candidate-only Effect를 playable class 완료의 hard dependency나 완료된 runtime skill binding으로 주장하지 않았다.

남은 범위는 DimensionMaster 고유 skill/action/damage 매핑, animation notify, effect binding, 초월기 camera/cutscene, immutable resource 배포다. 또한 기존 modular 네 class의 여러 prototype을 순차 commit하는 범용 rollback은 별도 Engine batch admission 과제로 남는다. DimensionMaster는 body 한 개만 stage하므로 이번 경로에는 partial prefix가 생기지 않는다.
