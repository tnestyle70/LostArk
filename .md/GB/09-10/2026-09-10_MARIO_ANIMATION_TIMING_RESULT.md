# 마리오 구간 몬스터·광대 애니메이션 타이밍 정정

작성자: GB · 2026-09-10

사용자 관찰: 마리오 1~4 구간의 몬스터가 발을 움직이지 않고 뻣뻣하게 미끄러지듯 걷고
공격 모션이 보이지 않으며, 플레이어 공격 모션도 이상하게 보인다.

세 갈래로 조사했고 두 개는 실제 결함, 하나는 **결함이 아니었다**. 아래에 조사 근거와
적용 내용, 그리고 확인하지 않은 것을 구분해 남긴다.

## 1. 몬스터 — `.wmodel` 클립이 1000 t/s로 구워져 33배 느리게 재생

`Engine/Private/Animation.cpp:46`의 cooked 경로는 `.wmodel`에 적힌 `ticksPerSecond`를
버리고 항상 `COOKED_TICK_RATE = 30`으로 재생한다. `m_fDuration`은 파일에 적힌 tick 수를
그대로 쓴다. 이 고정은 `.md/JS/08-06/`에서 플레이어 클래스가 24 t/s로 잘못 구워진 것을
바로잡으려고 의도적으로 넣은 것이고, 그 클래스들에는 옳다.

문제는 30이 아닌 **진짜** rate로 구워진 파일이다. 아래 네 개가 duration을 밀리초로 담은
1000 t/s 패키지였다.

| 파일 | 클립 | 저작 의도 | 수정 전 런타임 |
|---|---|---|---|
| `Character/Monster/MarioOriginal/REUP/REUP.wmodel` | 30 | `walk_normal_1` 1.27초 | 42.2초 |
| `Character/Monster/MarioOriginal/CDMD/CDMD.wmodel` | 30 | `walk_normal_1` 3.07초 | 102.2초 |
| `Character/Monster/MarioOriginal/RHKP/RHKP.wmodel` | 47 | `att_battle_1_01` 1.87초 | 62.2초 |
| `Character/KoukuSaton/MN_CDMD_00/MN_CDMD_00.wmodel` | 30 | `att_battle_2_01` 11.33초 | 377.8초 |

걷기 한 사이클이 42~102초로 늘어나면 1초에 3% 남짓만 진행하므로 발이 멈춘 것처럼 보인다.

`SpawnGroups.world.json` 대조 결과 해당 archetype 배치는 마리오1 3/3, 마리오2 2/7,
마리오3 14/14, 마리오4 3/16이다. 나머지(`MONSTER_MARIO_CLUB/HEART/DIAMOND`)는 CardMiro
모델이라 30 t/s로 정상이었다. `MN_CDMD_00`은 월드 시퀀스의 `world.object.kouku.odd_doll`
(`animated: true`, 기본 모션 `att_battle_2_01`)이 쓴다.

### 적용

기존 `Tools/ActorXAssetCooker/retime_wmodel_ticks.py`로 네 파일을 30 t/s로 다시 표현했다.
클립 이름·채널 수·키 개수·파일 크기는 그대로이고 duration/rate/키 시간만 비율 변환된다.

```
python Tools/ActorXAssetCooker/retime_wmodel_ticks.py --wmodel <경로> \
  --ticks-per-second 30 --expect-ticks-per-second 1000
```

`--expect-ticks-per-second 1000`은 이미 고친 파일에 두 번 도는 사고를 막는다. 137개 클립
전부 event가 0이라 도구가 거부하지 않았다.

### 검증(실행함)

- 전후 스냅샷을 떠서 137개 클립 전부 **실제 초가 보존**되고, rate가 30이 되었으며,
  채널/pos/rot/scale 키 개수와 바이트 크기가 동일하고, 마지막 키 시간이 새 duration을
  넘지 않음을 확인했다. 결과 `RETIME_VERIFY_OK`.
- 원본 4개는 `out/WModelRetime/backup-2026-09-10/`에 보관했다. 전후 스냅샷은
  `out/WModelRetime/{before,after}.json`, 도구 리포트는 `out/WModelRetime/*.retime.json`.

### 배포

이 파일들은 `Client/Bin/Resources` 아래라 publish도 리빌드도 필요 없다. 다만 Git 비추적
영역이므로 팀원에게 Drive로 다시 전달해야 한다. **크기가 같아서 겉으로는 구분되지 않으니
같은 경로에 덮어써야 한다.** 오늘자 `Resource_Distribution_2026-09-10.txt`에는 Mario 세
개만 있고 `MN_CDMD_00` 행이 없다. 네 파일 목록은 `out/WModelRetime/redistribute.txt`.

## 2. 광대 상호작용 — 고정 3초 잠금이 1.0~3.0초 클립을 가둬 마지막 프레임이 굳었다

마리오 구간의 플레이어는 광대(`MN_RPCZ_00-1`) 몸체이고 공격은
`Data/Animation/Authored/KoukuSaydon/Clown.interactionbindings.json`의 슬롯 클립이다.
`Character.cpp`의 INTERACTION 경로는 클립을 서버 액션 경과 시간으로 스크럽하고
`min(age, duration - 0.0001)`로 **마지막 프레임에 clamp**한다. 반면 서버는
`KOUKU_INTERACTION_ACTION_MS = 3000` 하나로 모든 슬롯을 잠갔다.

MN_RPCZ_00-1에서 실측한 저작 길이는 다음과 같다.

| 모드 | 슬롯 | 클립 | 길이 | 기존 정지 구간 |
|---|---|---|---|---|
| POLYMORPH | 0 | `rpcz00p_att_battle_1_01` | 1500ms | 1500ms |
| POLYMORPH | 1 | `rpcz00p_att_battle_2_01` | 1500ms | 1500ms |
| POLYMORPH | 2 | `rpcz00p_att_battle_4_01` | 1000ms | 2000ms |
| MARIO | 0 (Q) | `rpcz00p_project_tuned_hammer` | 2500ms | 500ms |
| MARIO | 1 (W) | `rpcz00p_att_battle_3_01` | 1500ms | 1500ms |
| DANCE | 0~3 | `rpcz00p_project_tuned_pose_*` | 3000ms | 없음 |
| MAZE | 0 | `rpcz00p_project_tuned_hammer` | 2500ms | 500ms |

마리오 W는 액션 3초 중 1.5초를 마지막 프레임으로 굳은 채 보낸다. DANCE만 우연히 3000ms로
맞아서 드러나지 않았다.

### 적용

- `Shared/Public/Network/PacketMessages.h` — `Kouku_InteractionActionMs(mode, slotSkillIndex)`
  `constexpr` 추가. 위 실측값을 돌려주고, 모드가 저작하지 않은 슬롯은 기존 3000ms 천장을
  유지한다. `KOUKU_INTERACTION_ACTION_MS`의 주석을 "천장"으로 정정했다.
- `Server/Private/GameRoom.cpp` `Update_Players` — `interactionElapsed`가 슬롯별 틱을 쓰도록
  변경. 카드미로 탈출 텔레포트는 같은 INTERACTION 잠금을 슬롯 없이 빌려 쓰고
  **`INVALID_SKILL_ID`가 0이라 슬롯 0과 값이 같으므로**, skill id가 아니라
  `CardMaze.transferStartTick`으로 분리해 천장을 유지한다.

쿨다운(`KOUKU_INTERACTION_COOLDOWN_MS` 3000, `KOUKU_MAZE_HAMMER_COOLDOWN_MS` 400)은 건드리지
않았다. 따라서 밸런스는 그대로이고, 바뀌는 것은 "애니메이션이 끝나는 시점에 조작이 돌아온다"
뿐이다. Shared의 wire 구조를 바꾸지 않았으므로 protocol 버전은 그대로다.

### 검증(실행함)

- `Kouku_InteractionActionMs` 본문을 헤더에서 그대로 추출해 격리 TU로
  `cl.exe /std:c++20 /permissive- /W4`로 컴파일. 경고 0, `static_assert` 17개 통과.
  여기에는 잠금이 망치 판정 틱(`HAMMER_HIT_TICK_OFFSET` 12틱=400ms)과 카드미로 전송
  완료 틱(36틱=1200ms)보다 항상 길다는 보증이 포함된다.
- `Ticks_FromMs(2500/1500/1000/3000)` = 75/45/30/90틱으로 클립 길이와 정확히 일치함을 확인.
- `ServerGameplayContractTests.cpp`의 마리오 Q/W 테스트(2360~2400)를 검토했다. Q는 시작
  +14틱, W는 시작 +11틱까지만 진행하므로 새 잠금(75/45틱)보다 훨씬 앞이고 영향 없다.
  DANCE 테스트는 3000ms 그대로라 무관하다.

### 검증(미실행)

- **Server/Client 빌드를 하지 않았다.** Visual Studio(devenv.exe)가 열려 있어 같은 워킹
  트리에서 자동 빌드를 겹쳐 돌리지 않았다. 사용자가 Product 빌드 후 Server/Client를 재시작해야
  적용된다.
- `Server.exe --contract-test` 미실행.
- 인게임 화면 확인 미실행.

## 3. 클래스 공격 클립 전환 — 결함이 아니었다

처음에 "다중 클립 스킬 43개 중 39개에 `playMs` 전환 지점이 없어 앞 동작만 나온다"고
보고했다. **틀렸다.** 실측으로 뒤집혔고 아무것도 바꾸지 않았다.

- 여섯 클래스의 모든 다중 클립 바인딩에 대해 `.animnotify`의 `len=`과 `.wmodel`의 실제 클립
  길이(duration/30)를 비교했다. 마지막이 아닌 클립 **전원**이 `len ≈ 클립 전체 길이`였다.
  즉 원본이 그 클립들을 통째로 재생하도록 저작했으므로 `playMs`는 무의미하다.
  `Character.cpp:1033 Update_Chain`은 `Is_ClipFinished()`에서 다음 클립으로 넘어가고,
  `playMs`가 없으면 클립 끝에서 넘어간다 — 이미 올바른 동작이다.
- `actionDurationMs`와 클립 체인 총 길이를 전 클래스에서 대조했다. 비-콤보 스킬 전부
  **오차 0ms**로 일치한다. 서버가 애니메이션을 중간에 끊고 있지 않다.
- `movementDistance`는 전 스킬 0이라 루트 모션 불일치도 해당 없다.

`.md/JS/08-06/` §5가 창술사 16개에 넣은 값은 세 가지 서로 다른 출처(animnotify `len`,
레이블 없는 `win=NONE` CANCEL 윈도우 시작값, `.loa` 스테이지 레코드 추정)와 사용자의 도구
내 확인을 섞은 것이다. 기계적으로 유도되는 단일 규칙이 아니므로 나머지 클래스에 자동으로
채워 넣는 것은 근거 없는 변경이 된다. 하지 않았다.

### 다만 하나 남은 실제 결함 (미수정)

`Slayer 45070`(S, 회전베기)의 `wbk_sk_whirlwind_loop`는 클립이 0.333초인데 `.animnotify`는
4.0초를 재생하라고 적혀 있다. 원본은 이 클립을 **반복**시킨다. 현재 런타임은 한 번만
재생하고 다음 클립으로 넘어가, 체인 3.333초가 액션 7.0초 안에서 끝나고 **3.667초 동안
마지막 포즈로 굳는다.**

고치려면 ACTIVE 체인 중간 클립에 "지정 시간까지 반복" 능력이 필요하다. 현재 `CLIP_STEP.loop`는
HOLD 스킬의 중간 스테이지 마지막 클립(`isHoldLoop`)에만 쓰인다. 새 저작 능력이라 이번 범위에
넣지 않았다. 같은 검사에서 나온 나머지 정지 구간은 67~233ms로 무시할 수준이다.

## 4. 다른 작업과의 경계

작업 시작 시 `Server/Private/GameRoom.cpp`가 다른 세션에 의해 이미 수정되어 있었다
(빙고 망치 World Sequence, `Handle_DebugBingoHammer` 3492행 / `Update_KoukuBingo` 3559행).
이번 변경은 `Update_Players` 15463행 한 곳이라 겹치지 않는다. 편집은 전부 바이트 앵커
치환으로 수행했고, 치환 후 CRLF 개수·비ASCII 바이트 수·UTF-8 유효성·BOM 부재를 매번
확인했다.

## 5. 사용자가 할 일

1. Visual Studio에서 Product 빌드(Engine → Shared → Server → Client). Shared public 헤더가
   바뀌었으므로 Server와 Client 둘 다 필요하다.
2. Server와 Client 재시작.
3. 마리오 구간에서 몬스터 걷기·공격, 광대 Q/W 공격 끝의 정지 유무를 화면으로 확인.
4. 팀원에게 `out/WModelRetime/redistribute.txt`의 네 파일을 Drive로 재배포.

## 6. 추가 — 마리오에서 Q/W가 아무 반응도 없던 원인

사용자 관찰: 마리오 2에서 Q/W/E를 눌러도 스킬이 안 나간다.

전 구간 문제였다. `KOUKU_HUD_MODE::MARIO`는 두 가지를 결정한다. 서버가 광대의 상호작용
슬롯을 몇 개 배분할지(`KoukuSaydonLogicRuntime.cpp:1246`, MARIO=2), 그리고 클라이언트가
마리오 구간에서 슬롯 입력을 **제출할지 말지**다.

```cpp
// Client/Private/PlayerController.cpp:208
if (marioControlsActive &&
    KOUKU_HUD_MODE::MARIO == CCombatHUDViewModel::Get().Get_Player().eKoukuHudMode)
{
    Poll_SkillSlots(...);   // 여기서만 Q/W가 전송된다
}
```

`marioControlsActive`가 참이면 그 아래 일반 클래스 스킬 경로는 `return`으로 건너뛴다.
따라서 모드가 MARIO가 아니면 Q/W는 전송조차 되지 않는다.

그런데 이 모드를 설정하는 경로는 `ServerTriggerSystem.cpp:107` 하나뿐이고, 그 값은
movePlayer 트리거의 저작 필드 `koukuHudMode`에서 온다. 월드 전체를 조사한 결과 이 필드를
가진 배치는 정확히 둘이다.

| 배치 | enabled | koukuHudMode |
|---|---|---|
| `Mario1_go` | **false** | MARIO |
| `Mario1_Trigger_5` | true | NONE |

`MarioN_go` 네 개는 전부 `enabled=false`인 설정 행이라 그 movePlayer는 발화하지 않는다.
즉 **어떤 구간에서도 MARIO 모드에 들어간 적이 없다.** 모드는 `NONE`으로 남고,
`KoukuSaydonLogicRuntime.cpp:1227`의 `NONE && CLOWN -> POLYMORPH` 규칙에 걸려 POLYMORPH가
된다. 실제 구간 진입은 `MarioN_Intro`(enabled)를 밟을 때 `Update_MarioControlState`가
`iMarioStage`와 CLOWN을 확정하는 경로이고, 여기에는 HUD 모드 설정이 없었다.

이 결함이 계약 테스트를 빠져나간 이유도 같은 자리에 있다.
`ServerGameplayContractTests.cpp:2349`가 `eKoukuHudMode = MARIO`와
`ModeSkillIndexBySlot[0..1]`을 **손으로 채운 뒤** `Handle_InteractionSlot`을 부른다.
망치 판정만 격리 검증하고 모드 배관은 통과시킨 적이 없다.

### 적용

구간이 형태(CLOWN)를 소유하듯 HUD 모드도 소유하게 했다.

- `Server/Private/GameRoom.cpp` `Update_MarioControlState` — 진입 블록과 유지 블록 두 곳에서
  `eMadnessForm = CLOWN` 옆에 `eKoukuAreaHudMode = KOUKU_HUD_MODE::MARIO`를 함께 설정
- `Server/Public/ServerPlayer.h` `Clear_MarioControl()` — 이탈 시 `eMadnessForm = ePreMarioForm`
  옆에서 `eKoukuAreaHudMode`를 NONE으로 복귀. `Clear_KoukuInteractionState()`를 쓰지 않은
  이유는 그 함수가 Debug override까지 지우기 때문이다.

저작 데이터는 건드리지 않았다. `Mario1_go`의 MARIO와 `Mario1_Trigger_5`의 NONE은 그대로
두었다. 전자는 그 상자가 언젠가 enabled가 되면 여전히 옳고, 후자는 이제 코드가 하는 일과
같은 값이라 무해하다.

### 남은 것 — E 슬롯

E는 이 수정으로도 안 먹는다. 설계상 비어 있다. MARIO 모드는 슬롯 2개만 배분하고
(`KoukuSaydonLogicRuntime.cpp:1248`), 저작 문서
`Data/Animation/Authored/KoukuSaydon/Clown.interactionbindings.json`의 MARIO 항목도 클립이
둘(`rpcz00p_project_tuned_hammer`, `rpcz00p_att_battle_3_01`)뿐이다. E까지 쓰려면 저작에
세 번째 클립을 추가하고 배분 수를 3으로 올려야 하는데, 무엇을 넣을지는 저작 결정이라
하지 않았다.

### 검증(실행함)

- `GameRoom.cpp`와 `ServerGameplayContractTests.cpp` 두 TU를 프로젝트 실제 정의로 컴파일,
  경고 0 · exit 0.
- 계약 테스트는 모드를 손으로 채우므로 이 변경의 영향을 받지 않는다.

### 검증(미실행)

- Server 빌드와 인게임 확인. VS가 열려 있어 자동 빌드를 겹치지 않았다.

## 7. 광대 뿅망치 — 1차 실패 원인과 2차(정적 사본) 적용

### 1차 시도가 광대 전체를 깨뜨린 이유

`Spec_KoukuSaydonClown`의 `pWeapons`가 비어 있어 `WP_MN_RPCT_06`을 `MODEL::NONANIM`으로 붙였다.
그러나 `Engine/Private/Model.cpp:1434`는 `(MODEL::ANIM == m_eType) != asset.hasSkeleton`이면
`E_FAIL`이고, `hasSkeleton`은 메시가 스킨드(`VF_BONE_WEIGHT`)일 때 켜진다. `WP_MN_RPCT_06`은
`vtxFlags=0x1f, 본 8개, 클립 16개`인 스킨드 무기라 즉시 실패했고, 제가 그 실패를 하드 리젝트로
묶어둔 탓에 `Ensure_ClownBodyPrototype` 전체가 실패 → `ClientReplication.cpp:2032`가 `false` →
**광대 몸체 자체가 생성되지 않았다.** 사용자가 본 "컷신 이후 원래 캐릭터로 돌아옴"이 이것이다.
14:56 Client 빌드에 들어갔고, 15:06에 세 파일을 `git checkout`으로 되돌렸다.

실측으로 확인한 사실:

- 플레이 가능 클래스 무기 11개 전부 `vtxFlags=0x0f, bones=0, 스켈레톤 섹션 없음` — 순수 정적 메시.
  그래서 `PlayableCharacterAssetService.cpp:326`의 `MODEL::NONANIM`이 성립한다.
- `CPart_Equipment`는 소켓 모드에서 `iSocketedPassIndex`로 그리고 본 팔레트를 바인딩하지 않는다
  (`Part_Equipment.cpp:105,111,142`). 즉 **소켓 무기는 정적 메시여야 한다.**
- 스킨드 정점(76B)의 앞 44B는 정적 정점과 같은 순서(pos/normal/uv/tangent)이고
  (`WMeshReader.cpp:391 MakeSkinnedVertex`), 정적 정점은 그 뒤에 handedness float 1개가 붙는다
  (`MakeStaticVertex`). 출하된 클래스 무기는 전 정점 handedness=1.0이다.
- 디코더의 `ConvertToStaticBindPose`(분리형 `.wskel` 경로)도 같은 필드 복사만 한다 —
  스킨드 정점은 바인드 포즈 모델 공간에 저장돼 있다.

### 2차 적용 — 정적 사본을 굽고 클래스 무기와 동일한 경로로 소켓

`WP_MN_RPCT_06_Static.wmodel`을 원본 옆에 새로 만들었다(원본은 건드리지 않음).
메시 섹션만 재작성: 정점 76B→48B(앞 44B + handedness 1.0), 본 항목 제거, 바운드 재계산,
`vtxFlags=0x0f, stride=48, bones=0, hasBounds=1`. 재질 섹션은 바이트 동일하게 복사하고
스켈레톤·애니메이션 섹션은 제외. `MODEL_HEADER`는 sections=2, animations=0, flags=0
(클래스 무기와 동일). 같은 폴더에 두므로 `WMaterialReader.cpp:90`의
`materialPath.parent_path() / storedPath` 규약으로 textures를 공유한다.

코드는 1차와 같은 세 파일이되 `CLOWN_HAMMER_ASSET`이 정적 사본을 가리키고 `MODEL::NONANIM`을
쓴다. 크기·회전은 빙고 세이튼 값(0.00711685, 23/8/8)을 출발점으로 두었고 광대 리그에서 화면 조정이
필요하다.

### 검증(실행함)

- 굽기 자체 검증: 정점 9,622·인덱스 13,158·서브메시 1 유지, 모든 정점의 44B prefix와 인덱스
  blob이 원본과 바이트 동일, 재질 섹션 바이트 동일, 바운드 span 1.367×0.406×0.823 / radius 0.685가
  원본과 소수점까지 동일. 파일 1,040,388B → 493,268B.
- `Tools/ModelAssetConverter/cook_wmodel_geometry_contract.parse_legacy_wmodel`(출하 정적 자산을
  만드는 debris 도구가 쓰는 파서)로 라운드트립: header (2,0,0), 섹션 (mesh 488,312B / material
  4,780B), 첫 정점 handedness 1.0, max index 9,621 < 9,622.
- 리더 게이트 대조: legacy WINT 1.0, `flags == VF_STATIC_BASE`, stride 48, boneCount 0,
  idxStride 2, 서브메시 오프셋 연속, 바운드 행 수 = 서브메시 수.
- 바뀐 Client TU 두 개(`CharacterCatalog.cpp`, `KoukuSaydonPresentationAssetService.cpp`)를 프로젝트
  정의(`/DUNICODE /D_UNICODE`)로 컴파일: 경고 0, exit 0. CRLF/UTF-8/BOM 무결성 재확인.

### 검증(미실행 / 불가)

- `WModelGeometryContractHarness --candidate`: 하네스 exe가 8/30 빌드라 오늘 15:19 `Engine.dll`과
  ABI가 달라 **기준 클래스 무기에서도** `0xC0000005`로 죽는다. VS가 열려 있어 재빌드하지 않았다.
  이 하네스 결과는 이번 판정에 쓰지 않았다.
- Client 빌드와 인게임 확인.

### 배포

`WP_MN_RPCT_06_Static.wmodel`은 신규 파일이며 어떤 배포 목록에도 없다. 원본과 textures 4장을
포함해 `out/WModelRetime/redistribute.txt`에 실었다. 팀원에게 없으면 광대 admission이 실패한다
(보스 무기와 같은 fail-closed).

### 3차 — 망치가 1.2cm였다: 소켓 본이 곱하는 배율을 실측하고 identity로 정정

2차 빌드(16:09) 뒤 광대는 뜨는데 망치가 보이지 않았다. 무기 파츠 생성이 실패했다면
`CCharacter::Ready_Parts`가 `E_FAIL`을 내서 몸체까지 안 떴을 것이므로 파츠는 있었고, 크기가 문제였다.

정점 좌표와 스켈레톤을 직접 계산한 결과(`bind_pose_probe.py`, `CBone`의 `child × parent`와
`CMesh`의 `Offset × Combined` 규약을 그대로 재현):

| 모델 | 소켓 본 전역 바인드 스케일 | 정점 raw span | 바인드 스키닝 후 |
|---|---|---|---|
| 광대 `MN_RPCZ_00-1` (`b_wp_1`) | **100.0** | 1.99 (m) | 199.08 → ×0.012053 = **2.399m** (엔진 실측 2.373m와 일치) |
| 뿅망치 `WP_MN_RPCT_06` | (자체 체인 ×100) | 1.367 (m) | 136.7 |
| 차원술사 (`b_wp_swm_m_1`) | 1.0 | 110.9 (cm) | 110.9 |

즉 Kouku 쿡은 **메시가 m, 스켈레톤 체인이 ×100**이고, 클래스 몸체는 메시가 cm에 체인 1.0이다.
클래스 무기가 cm 단위 + `XMMatrixIdentity()`로 맞는 이유가 그것이다. 광대 `b_wp_1`에 소켓된
정적 메시는 raw 1단위당 **100 × 0.012053 = 1.2053**을 받는다. m 단위인 정적 뿅망치(1.367)는
identity에서 1.65m인데, 제가 곱한 0.00711685(리깅된 보스 무기의 값)가 이를 **1.17cm**로 만들었다.

굽기는 그대로 맞다(m 단위 정점이 이 소켓에 정확히 맞는 단위다). 정정한 것:

- `KoukuSaydonPresentationAssetService.cpp` — `CLOWN_HAMMER_PRE_SCALE` 1.0, 회전 0/0/0
  (클래스 무기와 같은 identity 기준. 이 리그에서 손잡이가 어긋나면 이 넷이 조정 손잡이다).
- `Character.cpp` `Apply_MarioPresentation` — 마리오에서 몸체를 1.5m로 줄일 때 무기 파츠의
  Transform도 같은 배율로 `Scale`. 소켓 본 행렬은 모델 pre-transform만 싣고 파츠 Transform은
  싣지 않아, 그대로 두면 1.5m 광대가 1.65m 망치를 들게 된다. 파츠 자체 Transform이
  `CPart_Equipment` 소켓 행렬의 첫 인자라 소켓 기준으로 균일 축소된다.

### 검증(실행함)

- 위 표의 수치 계산(광대 몸체 키가 엔진 실측과 1% 이내로 일치 — 단위 모델의 자기 검증).
- 바뀐 TU 컴파일과 CRLF/UTF-8 무결성은 아래 최종 항목에 기록.

- 최종 컴파일: `Character.cpp`, `KoukuSaydonPresentationAssetService.cpp`, `CharacterCatalog.cpp`를
  프로젝트 정의(`/DUNICODE /D_UNICODE`)로 컴파일, 셋 다 경고 0 · exit 0.
- 바뀐 Client 파일 4개 CRLF/LF 일치(3042/701/106/84), UTF-8 유효, BOM 없음, `git diff --check` 통과.
  총 +67/-2줄.

### 검증(미실행)

- Client 빌드(현재 exe 16:09, 소스 16:19)와 인게임 확인. 화면 판정은 사용자 몫이다.

### 4차 — 망치가 손에서 수 미터 벗어나 흔들림: 파츠 합성 불일치와 raw 정점 굽기

사용자 스크린샷 4장(16:25): 망치(노란 원·흰 별 문양으로 `wp_mn_rpct_06_d.dds`와 일치, 빙고 망치
`mn_umac_01_d`의 팔각 금장 문양과 다름)가 광대 머리 위 수 미터에 떠서 플레이어 방향이 뒤집힐 때
큰 반경으로 돈다.

원인 둘:

1. **파츠 합성 불일치.** 몸체는 `스키닝 정점 × 본 × 몸체파츠Transform × 루트`로 그려지고
   (`CPart_Body::Update`), 소켓 무기는 `own × RotY × 본 × 루트`다(`CPart_Equipment::Update`).
   `Apply_MarioPresentation`이 몸체 파츠 Transform을 0.632배로 줄이므로 손은 0.62m에 그려지는데
   무기 피벗은 본 이동 그대로 0.98m에 놓인다. 제가 넣었던 "무기 Transform도 Scale" 루프는 정점만
   줄이고 본 이동은 못 줄인다.
   → `Ready_Parts`의 weaponDesc에 `pSocketRootMatrix = 몸체 파츠 Transform 월드 행렬`을 연결
   (`CharacterPreviewPanel.cpp:784`, `Valtan.cpp:3743`과 같은 방식). 이러면 무기 합성이
   `본 × 몸체파츠 × 루트`가 되어 몸체의 손과 수학적으로 동일해진다. 클래스는 몸체 파츠가
   identity라 no-op. 무기 Scale 루프는 제거.
2. **raw 정점 굽기.** v1 정적 사본은 스킨드 정점을 그대로 복사해 리그의 바인드 회전(Y↔Z 90°)이
   빠져 있었다. 보스는 바인드 스키닝된 형상을 소켓에 붙이므로 광대도 같아야 한다.
   → `bind_pose_probe`의 규약(`Offset × Global`, ×100은 균일 스케일이라 /100)으로 정점·법선·탄젠트를
   바인드 포즈로 변환해 다시 구움. span 1.367×0.823×0.406(원본 Y/Z 대비 리그 자세), 정점·인덱스 수
   동일, `parse_legacy_wmodel` 통과, 파일 크기 동일(493,268B — 팀원 배포 시 같은 경로 덮어쓰기).

소켓 방향 실측: 빙고 세이튼 `b_wp_1`은 모델 축과 일치(+X→(1,0,0))하지만 광대 `b_wp_1`은
+X→캐릭터 (−0.07,−0.03,0.997), +Y→(−0.71,−0.71,−0.07). 보스용 각도(23/8/8)를 광대에 쓸 수 없는
근거이며, 남는 조정은 손 안에서의 3축 각도뿐이다(identity에서 시작).

### 5차 — 망치 자산 정정: 광대는 1마리오 몬스터(REUP)의 망치를 든다

사용자 지적: 원본에서 플레이어는 1마리오 몬스터가 든 것과 같은 망치를 든다. 제가 쓴
`WP_MN_RPCT_06`(세이튼 보스 망치)은 다른 자산이었다.

실측: `REUP.wmodel`은 서브메시 2개 — `mn_reup_05_sk-0`(몸체, 8,935 정점)와
`mn_reup_05_sk-1`(**망치**, 2,718 정점, 재질 `wp_mn_rhkp_07_mi`, 텍스처
`wp_mn_rhkp_07_{d,n,s}.dds`). 망치는 Biped 무기 본 `bip001-prop1`에 **100% 가중치**로 붙어 있고
REUP은 cm 단위·체인 스케일 1.0(몸체 raw 130 → 1.3m)이라 바인드 스키닝은 항등이다. 망치 길이 1.05m.

광대 리그(`MN_RPCZ_00-1`)에는 prop 본이 없지만 둘 다 Biped라 `bip001-r-hand`가 있다. REUP의
`prop1`은 `r-hand`의 자식으로 같은 방향에 ~10cm 떨어져 있을 뿐이므로, 망치를 **REUP `r-hand` 로컬
프레임**으로 옮겨 굽고 광대의 `r-hand`에 소켓하면 몬스터의 손 기준 그립이 그대로 옮겨진다.

적용:

- `cook_reup_hammer.py bip001-prop1 bip001-r-hand` → `Character/Monster/MarioOriginal/REUP/WP_MN_RHKP_07_Static.wmodel`
  (170,860B). 서브메시 1만 추출, `v × inv(G_rhand)` 후 cm→m, 법선·탄젠트 회전, 서브메시 상대
  인덱스 확인(max 2717 < 2718), 재질 섹션 바이트 동일(재질 2개 유지, materialIndex 1 —
  `WMeshReader.cpp:832`·`WMaterialReader:180`이 섹션 수로 크기를 잡으므로 유효). 로컬 span
  0.60×1.12×0.29m. `parse_legacy_wmodel` 통과.
- `KoukuSaydonPresentationAssetService.h` — `KOUKU_CLOWN_HAMMER_SOCKET_BONE = "bip001-r-hand"`
  (스펙과 admission이 같은 상수를 사용).
- `KoukuSaydonPresentationAssetService.cpp` — 자산 경로 교체, admission의 본 검사를 이 상수로,
  `CLOWN_HAMMER_PRE_SCALE = 1.313` = 1/(100 × 0.012053 × 0.632): 마리오 무대에서 광대 망치가
  몬스터 망치와 같은 1.05m가 되도록(광대 축소에 딸려 줄지 않게). 회전 identity(쿡이 이미 손
  프레임).
- `CharacterCatalog.cpp` — `Weapons[]` 소켓 본을 상수로.
- 이전 `WP_MN_RPCT_06_Static.wmodel`은 Resources에서 제거(`out/WModelRetime/`에 보관).

검증(실행함): 두 Client TU 컴파일 경고 0·exit 0, 네 파일 CRLF/UTF-8/BOM 무결성, `git diff --check`.
검증(미실행): Client 빌드·화면 확인.

### 6차 — 망치 방향: 머리가 등 뒤로 향해 pitch 180°

사용자 스크린샷(17:13): 망치가 손에 붙었으나 머리가 캐릭터 뒤로 수평. 원본 참고(17:14)는 머리가
앞·위. 쿡의 로컬 축은 +Y가 머리, X가 머리 원통 축(별 문양 면)이므로 X 기준 180°(pitch)로 뒤집으면
별 면을 유지한 채 머리만 앞으로 온다. `CLOWN_HAMMER_PITCH_DEGREES = 180`. 같은 축이 화면 평면에서
머리를 올리고 내리는 손잡이이기도 하다(참고 이미지처럼 더 들어 올리려면 이 값에 ±30 정도).

검증(실행함): 해당 TU 컴파일 exit 0. 검증(미실행): Client 빌드·화면.

### 7차 — 머리 고도 +30°: idle 클립을 샘플링해 pitch 220으로 확정

`hand_pose_probe.py`가 `rpcz00p_idle_battle_1`의 첫 키로 전체 체인을 포즈해 오른손 프레임을 캐릭터
공간에서 계산했다(엔진 `CChannel`의 `XMMatrixAffineTransformation` S·R·T, 쿼터니언 x,y,z,w와 동일 규약).

| pitch | 머리 방향 (x우, y위, z앞) | 고도 |
|---|---|---|
| 0 | (−0.37, −0.02, −0.93) | 뒤·수평 — 첫 스크린샷과 일치 |
| 180 | (0.37, 0.02, 0.93) | 앞·수평 — 둘째 상태와 일치 |
| **220** | (−0.11, 0.50, 0.86) | **앞 +30°** |

손 +Z가 (0.61, −0.76, −0.23)로 대부분 아래를 향해 pitch 증가가 머리를 올리며, Z가 수직이 아니라
40°의 pitch가 30°의 고도가 된다. run 자세(`rpcz00p_run_battle_1`)에서는 손이 다르게 놓여 같은 값이
+33° 옆쪽으로 가리키지만, 망치는 손에 강체로 붙어 팔 스윙을 따라가므로 정지 자세 기준 220으로
확정했다. 검증(실행함): TU 컴파일. 검증(미실행): Client 빌드·화면.

## 8. 마리오 원본 공 — Q로 터뜨리고 색이 다 터지면 저주 해제

### 요구와 구현 경계

1마리오~4마리오의 빨강·파랑·노랑 공은 Client 배치일 뿐 서버에 엔티티가 없었다. 그래서
`Resolve_MarioHammerHit`이 MONSTER 엔티티만 훑던 기존 코드로는 아무리 때려도 반응이 없었다.
서버가 공의 좌표를 알아야 판정할 수 있으므로 배치를 부트스트랩으로 실어 보낸다.

| 계층 | 변경 |
|---|---|
| Publisher | `Publish-WorldGameplay.ps1`이 `world.sequence.instance.marioN.source.layoutM`의 바인딩 순서 그대로 `MARIOBALL` 행을 만든다. 부트스트랩 헤더 v10 → v11(끝에 ball count) |
| Server | `CWorldBootstrap`이 v10·v11을 모두 읽고 `MARIO_SOURCE_BALL`을 보관. `CGameRoom`이 스테이지별 `m_MarioPoppedBalls` 비트마스크를 소유 |
| Shared | `PLAYER_SNAPSHOT`에 `iMarioPoppedBallMask`(u16)·`iMarioCurseReleasedMask`(u8). 프로토콜 78 → **79** |
| Client | `CWorldSequencePlayer`에 억제 배치 집합, 터진 공은 연기 이펙트 1회 + 중앙 문구 3초 |

핵심은 **슬롯 번호가 곧 Client의 바인딩 인덱스**라는 점이다. 서버는 공의 ID를 모르고 슬롯 비트만
보내며, Client는 같은 순서의 바인딩을 숨긴다. publisher가 그 순서를 보증하고, 부트스트랩 파서는
슬롯이 (stage, layout)마다 0,1,2…로 연속인지 검사해 어긋나면 로드를 거부한다.

### 판정

`Resolve_MarioHammerHit`이 몬스터 루프 뒤에 공 루프를 돈다. 같은 전방 120°·2.4m 원뿔(공 반지름
0.47m 가산)에 높이 창 1.2m다. 스테이지의 층 간격이 2.56m이므로 1.2m는 위아래 층의 공을 확실히
배제한다. 이미 터진 비트는 다시 판정하지 않고, 색깔별로 그 레이아웃의 공이 전부 터지면
`Mario_CurseReleasedMask`가 해당 비트를 세운다. 공은 엔티티가 아니므로 데미지 이벤트를 만들지 않는다.

### 이 슬라이스에서 함께 고친 가격 타이밍

같은 뿅망치가 카드미로와 마리오에서 쓰이는데 `HAMMER_HIT_TICK_OFFSET`이 12틱(400ms)이었다.
`rpcz00p_project_tuned_hammer`(75틱)의 머리 궤적을 실측하니 27틱까지는 몸 뒤에서 감아올리는
중이고 30틱에 앞쪽 지면에 닿아 42틱까지 머문다. 12는 감아올리는 도중이라 "아직 때리지도 않았는데
맞았다"가 됐다. **30틱(1.0초)** 으로 고쳤고 카드미로 판정도 같은 값을 쓴다.

### 도달성 실측 — 데이터 변경은 필요 없었다

"맵의 빨간 공을 다 터뜨리면"이 성립하려면 모든 공이 실제로 닿아야 한다. 처음 두 모델(트리거 박스
중심을 잇는 선분, 착지점에서 뻗는 레일 마칭)은 24~51개가 사거리 밖이라고 나왔지만, 둘 다
`CServerNavigation::Select_Region`과 달리 **4m 격자인 base navgrid를 Mario2/3/4 세부 격자보다 먼저**
샘플한 탓이었다. 서버와 같은 순서(region 우선)로 다시 재면 결과가 뒤집힌다.

| 항목 | 값 |
|---|---|
| 사거리 밖 공 | **117개 중 0개** |
| 가장 가까운 보행 셀까지 거리 | 최소 0.03m / 중앙값 0.23m / 최대 **1.96m** (사거리 2.87m) |
| 그 셀과의 높이차 | 최대 **0.07m** (창 1.2m) |
| 사거리 마지막 0.4m가 필요한 공 | 0개 |

12개 레이아웃 전부에서 세 색 모두 완주 가능하다. 공 배치도, 사거리도 손대지 않았다.

### 검증(실행함)

- 변경한 TU 9개 전부 컴파일 exit 0: `PacketMessages.cpp`, `WorldBootstrap.cpp`, `GameRoom.cpp`,
  `ServerGameplayContractTests.cpp`, `WorldSequencePlayer.cpp`, `Level_KakulSaydonArena.cpp`,
  `Level_KakulSaydonArena_WorldObjects.cpp`, `CombatHUDViewModel.cpp`, `NetworkProtocolHarness.cpp`
- `Publish-WorldGameplay.ps1 -Mode Publish -WorldId KAKULSAYDON_ARENA` exit 0 →
  헤더 `…	11	…	112	209	36	117`, `MARIOBALL` 117행(9×9 + 12×3), 후행 행 0
- 부트스트랩을 C++ 파서 규칙 그대로 Python으로 재현: 슬롯 연속성·색·ID 전부 통과
- 공 117개의 placement ID가 Client 런타임 `.mapplacements`에 전부 존재하고 색상 불일치 0,
  Client 레이아웃 바인딩 순서와 게시된 슬롯 순서 불일치 0
- `git diff --check` clean, CRLF/UTF-8/BOM 없음 유지

### 검증(미실행)

- Server·Client 재빌드와 실제 아레나 화면 확인은 사용자가 한다. 프로토콜이 79로 올라갔으므로
  **Server와 Client를 함께 다시 빌드해야** 한다. 78 실행 파일과 섞으면 접속이 거부된다.
- 계약 테스트(`Server.exe --contract-test`)는 컴파일만 했고 실행하지 않았다(VS 열려 있어 빌드 금지).

