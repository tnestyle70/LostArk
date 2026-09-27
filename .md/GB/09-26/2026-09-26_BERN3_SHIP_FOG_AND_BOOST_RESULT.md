# 2026-09-26 베른 배 — 탑승 중 안개 제거와 쾌속 항해(Space) RESULT

> **2026-09-26 추가 조정:** 배 카메라 거리가 40 m → **20 m**로 줄었고 쾌속 배수는
> 게시 평속의 5배 → **3배**가 됐다. 안개는 `shipFog.disable=true`로 탑승 중 끄는 방식이므로
> 거리 변경과 무관하게 그대로 동작한다. 최신 수치는
> `2026-09-26_BERN3_SHIP_SPEED_TUNING_RESULT.md`를 본다.

요청 두 건이다.

1. 배를 타면 화면이 뿌옇게 보이는 안개를 **배 탈 때만** 안 보이게 한다.
2. 탑승 중 **스페이스 바**로 더 빠르게 가는 쾌속 항해를 넣는다.

두 건 모두 구현하고 컴파일·파서 런타임까지 검증했다. 화면 판정과 Server 재빌드는 사용자 단계로 남는다.

---

## 1. 안개 — 원인과 구현

### 원인 (실측)

Bern의 세 scene profile(`scene.bern.before-restoration.v1`, `neutral-day.v1`, `source-rendering.v1`)은
모두 `sourceExponential` 블록을 가지므로 `Engine/Bin/ShaderFiles/Shader_SceneHeightFog.hlsli`의
`EvaluateSourceExponentialFog` 분기를 탄다. 이 분기의 투과율은

```hlsl
transmittance = exp2(-integral * max(distance - g_fHeightFogStartDistance * 100.f, 0.f) * cameraDensity)
```

즉 **카메라~픽셀 거리가 `startDistance`를 넘는 만큼** 안개가 쌓인다. Bern의 저장값은
`density 0.3 / heightFalloff 0.8 / topHeight 0 / startDistance 16 / maximumOpacity 1`이다.

| 카메라 | 눈 높이 | 배까지 거리 | 배에 걸리는 안개 |
|---|---|---|---|
| 기존 맵 카메라 16 m | 약 22.3 m | 16 m = `startDistance` | `max(1600-1600,0)=0` → **0 %** |
| 배 카메라 40 m | 약 39.2 m | 40 m | 약 **9 %** |

배 자체보다 큰 영향은 화면 구성이다. 40 m로 물러나면 프레임이 먼 바다로 채워지고, 그 거리의
바다는 같은 식에서 200 m에 약 52 %, 400 m에 약 78 % 안개가 걸린다. 안개 색이
`(3.55, 4.42, 6)` HDR이라 톤매핑 뒤 흰 뿌연 막으로 보인다.

**결론: 팀장 안개 설정이 바뀐 게 아니라 카메라 거리를 40 m로 옮긴 것의 부작용이다.**
(참고: 다른 분기인 height-ceiling 경로는 `topHeight 0`이라 해수면 y≈10.95에서 안개가 0이 된다.
실제로 화면이 뿌연 것은 source-exponential 분기가 활성이라는 증거이기도 하다.)

### 구현 — 기존 transient 경로 재사용

팀장 정본 `Data/Rendering/Authored/RenderingProfiles.json`과 게시본
`Client/Bin/DataFiles/Rendering/RenderingProfiles.runtime.json`은 **읽기만 하고 수정하지 않았다.**
(`git status -- Data/Rendering Client/Bin/DataFiles/Rendering` 비어 있음 — 아래 검증 참조)

이미 존재하는 `CRenderingProfileService::Apply_CameraEnvironment(..., suppressFog, ...)` 경로를 쓴다.
이 함수는 매 프레임 순서가

```
Restore_PresentationEnvironment()  // 원래 scene 안개 복원
-> Apply_CameraRegionEnvironment() // 카메라 구역 안개 적용
-> (suppressFog면) 그 위에 일시 적용
```

이라서 **하차·Level 이탈·카메라 구역 전환에서 원래 값이 그대로 복원되고 프레임마다 누적되지 않는다.**
새 안개 소유자를 만들지 않은 이유가 이것이다.

변경 파일:

| 파일 | 내용 |
|---|---|
| `Client/Public/RenderingProfileService.h` | `PRESENTATION_FOG_TUNING`(densityScale/startDistanceMeters/maximumOpacity) 추가, `Apply_CameraEnvironment`에 optional 마지막 인자 |
| `Client/Private/RenderingProfileService.cpp` | tuning이 없으면 기존처럼 안개 OFF, 있으면 scene 안개를 옅게만 조정(유한값 검사 포함) |
| `Client/Public/ArenaCameraProfile.h` | `ARENA_SHIP_FOG` + `shipFog`/`hasShipFog` |
| `Client/Private/ArenaCameraProfile.cpp` | `shipFog` 블록 parse(Bern 전용, 정확히 4필드)·Save·Validate 범위 검사 |
| `Client/Public/Level_Bern.h` | `Get_ActiveShipFog()` — 배 follow pose가 활성일 때만 non-null |
| `Client/Private/MainApp.cpp` | 기존 호출 지점에서 조건에 OR, tuning 전달 (추가 14줄 + 호출 1줄) |
| `Data/Camera/Bern.camera.json` | `shipFog` 블록 |

### 사용자 조절 (리빌드 불필요, Level 재진입으로 적용)

```json
"shipFog": {
  "disable": true,
  "densityScale": 0.25,
  "startDistanceMeters": 60,
  "maximumOpacity": 0.35
}
```

- `disable: true` — 배 탄 동안 안개 완전 OFF. **현재 기본값이고 요청한 동작이다.**
- `disable: false` — 안개를 끄지 않고 아래 세 값으로 옅게만 남긴다.
  `densityScale`은 scene density에 곱하는 배수(0~8), `startDistanceMeters`와 `maximumOpacity`는
  `-1`이면 scene 값을 그대로 쓴다. 위 기본값은 200 m 바다에서 약 13 %로 계산된다.
- 블록을 **생략**하면 OFF가 기본이다.

배 이외에는 아무 영향이 없다. `Get_ActiveShipFog()`는 `m_bShipCameraActive`가 참일 때만 값을
주고, 그 플래그는 `Update_ShipCamera()`가 `VEHICLE_ACTOR_ENTRY::isShip`으로만 세운다.

---

## 2. 쾌속 항해 — 원작 수치와 구현

### 원작 데이터 (실측, `EFTable_VoyageShip` 89열 / 94행)

컬럼 수 89는 확인됐다. 부스트 관련 열은 다음이다.

| 열 | 의미 | 검증 |
|---|---|---|
| `MoveSpeed`(6) | 통상 속도 | 기존 저장본이 이미 ÷100로 소비 |
| `BoostSpeed`(32) | **가산** 보너스 | `SkillBuff[BoostSkillId].PassiveOptionValue0`와 94행 전부 일치, `PassiveOptionType0=2 / KeyStat0=85`, UI 문구 `+{0} 노트 … 추가 이동 속도` |
| `BoostDuration`(33) | 초 | `BoostDuration*1000 == SkillBuff.Duration` 94행 전부 일치 |
| `BoostSkillId`(27) | 부스트 스킬 | `EFTable_Skill` 이름 `쾌속 운항`, `InstanceSkillEffectString=Voyage_Boost_Start` |
| 쿨타임 | 이 표에 없음 | `EFTable_Skill[BoostSkillId].Cooltime` |
| `BoostGauge`(29) / `GaugePerUU`(31) | 게이지 용량 / 거리 회복 | 1회 소모는 `Skill.CostBoostGauge` = 500 |
| `Camera_Booster`(14) | 1003 (FOV 70 / Pitch −42 / ZoomDist 1700, `해상 고속운항 뷰(부스터)`) | 전 행 동일 |

**하위 에이전트 보고 정정:** "쿨타임 == 지속시간이 모든 행에서 성립"은 **틀렸다.** level 1에서
8202는 지속 7000 ms / 쿨타임 5000 ms, 8204는 지속 5000 ms / 쿨타임 7000 ms다. 직접 재확인했다.
따라서 원작의 실제 제한은 쿨타임이 아니라 게이지(용량 500, 1회 500 소모 = level 1에서 1회분)다.

우리 저장본의 `moveSpeed`는 9척 모두 **upgrade level 1**과 정확히 일치한다
(`source.value == lv1 MoveSpeed`, 9/9 확인). 그래서 부스트도 level 1 행을 썼다.

| 배 | 통상 | 부스트 | 지속 | 쿨타임 | skillId |
|---|---|---|---|---|---|
| 8200 에스토크 | 2.0 | **2.75** | 5000 | 5000 | 8200080 |
| 8201 풍백 | 1.9 | **2.8** | 5000 | 5000 | 8201085 |
| 8202 아스트레이 | 2.2 | **3.2** | 7000 | 5000 | 8202100 |
| 8203 바크스툼 | 1.8 | **2.6** | 5000 | 5000 | 8203070 |
| 8204 에이번의 상처 | 1.83 | **2.73** | 5000 | 7000 | 8204075 |
| 8205 브람스 | 1.85 | **2.6** | 5000 | 5000 | 8205070 |
| 8206 트라곤 | 1.8 | **2.65** | 5000 | 5000 | 8206085 |
| 8207 프뉴마 | 2.1 | **2.95** | 3000 | 3000 | 8207085 |
| 8208 루미나스 | 2.0 | **2.75** | 5000 | 5000 | 8208000 |

배 통상 속도가 도보 2.8 m/s보다 느렸다는 점을 감안하면 체감 차이가 크다.

### Client는 코드 변경이 없다 — Space는 이미 배선되어 있었다

`CPlayerController::Poll_VehicleSkillSlots`가 이미 `SPACE`를 탈것의 SPACE 슬롯 스킬로 조회해
기존 `C2S_USE_SKILL`로 보낸다. 배에 SPACE 스킬이 없어서 아무 일도 없었을 뿐이다.
Controller에 skillId를 하드코딩하지 않는 기존 경계를 그대로 지켰다.

### Server 권위 — 프로토콜 변경 없음

`CGameRoom::Resolve_PlayerMoveSpeed` 한 곳이 실제 이동(`GameRoom_PlayerSimulation.cpp:1024`)과
복제(`GameRoom_Replication.cpp:516`의 `snapshot.fMoveSpeed`)를 **둘 다** 먹인다. 그래서 여기서
부스트 속도를 돌려주면 Client는 기존 필드로 그 값을 받는다. **새 패킷도, protocol 번호 상향도 없다.**

주의해서 피한 함정: 일반 탈것 스킬 경로는 `eAction = VEHICLE_SKILL`로 바꾸면서
`hasMoveGoal = false`, `MovePath.clear()`를 한다. 그대로 쓰면 스페이스를 누를 때마다 배가 멈춘다.
그래서 기존 `flightToggle` 특수 분기와 같은 방식으로 **배 전용 분기**를 만들어 이동 상태를
건드리지 않고 부스트 만료 tick과 쿨타임만 세운다. 다시 눌러도 창이 갱신될 뿐 배수가 중첩되지 않는다.

| 파일 | 내용 |
|---|---|
| `Server/Public/ServerPlayer.h` | `iShipBoostEndTick` (server-only, 복제 없음) |
| `Server/Public/VehicleCatalog.h` | `fBoostMoveSpeed` + `Has_Boost()` |
| `Server/Private/VehicleCatalog.cpp` | bootstrap version 4 수용, `VEHICLE` 11필드, 부스트 검증 |
| `Server/Private/GameRoom_VehicleRiding.cpp` | `Resolve_PlayerMoveSpeed` 부스트, SPACE 배 분기, `End_ShipVoyage`에서 해제 |
| `Tools/GameplayPipeline/Publish-VehicleProfiles.ps1` | optional `boost` 블록 검증·11번째 필드·header 4, `BoostSkillId` 허용 |
| `Data/Vehicles/VehicleProfiles.json` | 9척 `boost` 블록 + SPACE `VEHICLESKILL` |
| `Data/Actors/VehicleCatalog.json` | 9척 SPACE 스킬 항목 |
| `Server/Bin/DataFiles/Vehicles/Vehicles.bootstrap` | 게시본 (v4, 50행) |

publisher는 provenance를 실제로 대조한다. `boost.source`의
`baseValue + value` ÷ `divisor` == `boost.moveSpeed`이고 `baseValue`가 `source.value`와 같아야 하며,
`table`/`column`이 `EFTable_VoyageShip`/`BoostSpeed`여야 하고, 8200~8208만 허용하며,
부스트 속도가 통상보다 빨라야 하고, SPACE 스킬이 정확히 하나여야 한다.

---

## 실행한 검증

| 검사 | 결과 |
|---|---|
| Client `/Zs` 문법 검사 4 TU (ArenaCameraProfile, RenderingProfileService, Level_Bern, MainApp) | **PASS**, error 0 |
| Server `/Zs` 문법 검사 6 TU (VehicleCatalog, GameRoom_VehicleRiding, _Replication, _PlayerSimulation, _PlayerCommands, ContractTests_VehicleRiding) | **PASS**, error 0 |
| 경고 | Client 65개·Server 72개 전부 기존 `C4819` 코드페이지 경고. 다른 종류 0개, 추가한 줄에서 난 경고 0개 |
| `Bern.camera.json` parse | OK, 값 노드 37개(파서 한도 64), 1417 byte(한도 8192), root 12키 = parser `expectedFields` 12 |
| `Publish-VehicleProfiles.ps1 -Mode Validate` | `16 vehicles, 34 skills` 성공 |
| `-Mode Publish` | 성공, `Vehicles.bootstrap` header `4 50` |
| 게시본 구조 | 배 `VEHICLE` 11필드(11번째 = 부스트 속도), 배 `VEHICLESKILL` 8필드 sampleCount 0 `-`, 탈것 6705는 11번째 0 |
| **실제 Server 파서 런타임 probe** (`out/ShipFogBoost20260926/parser_probe.cpp`, 실제 `VehicleCatalog.cpp`를 링크해 실제 게시본 로드) | `LOAD_OK`, 9척 `hasBoost=yes`·속도·skillId·지속시간 모두 저작값과 일치, 6705/9523은 `hasBoost=no`로 기존 4스킬 유지 |
| 음성·호환 검사 11건 (부스트 ≤ 통상 / 30 초과 / 음수 / 비숫자 / v4에 10필드 / v3 헤더에 11필드 / 비행 탈것에 부스트 / 미래 version 5 → 전부 거부, 통제군과 **기존 v3 게시본**은 로드) | **11/11 PASS** |
| 구형 `Server.exe`(11:54 빌드) + 신규 v4 게시본 | `Vehicle bootstrap header is invalid`로 **fail-closed 거부**. 버전 게이트가 동작하고 Server 재빌드가 필요함을 확인 |
| `git diff --check` | 내가 추가한 줄에는 공백 오류 없음. 보고된 `MainApp.cpp` 7962/7972/7982/8026/8043 후행 공백은 **이전 세션이 CP949 줄 삼킴을 막기 위해 의도적으로 넣은 것**이므로 건드리지 않았다 |
| 인코딩 보존 | 수정한 모든 파일의 CRLF/LF·BOM 없음·비ASCII 바이트 수를 패치 전후 대조해 동일 확인. 추가한 C++ 주석은 전부 영문 ASCII |
| 보호 경로 | `Data/Rendering`, `Client/Bin/DataFiles/Rendering`, `Engine` **변경 0건**. `.vcxproj`/`.filters` 변경 0건 |

`out/ShipFogBoost20260926/`에 컬럼 덤프, 행 CSV, 조인 CSV, 컴파일 로그, probe와 백업이 있다.

---

## 하지 않은 것 · 사용자 단계

- **화면 판정은 하지 않았다.** 에이전트는 Client를 실행·조작하지 않고 캡처도 만들지 않는다.
  안개가 실제로 사라졌는지, 쾌속이 체감되는지는 사용자가 직접 확인한다.
- **Server를 빌드하지 않았다.** 작업 중 Visual Studio(devenv PID 40636)가 열려 있어 같은 working
  tree에서 MSBuild를 겹쳐 돌리지 않았다. `/Zs`는 산출물을 쓰지 않으므로 안전하게 실행했다.
  bootstrap 세대가 3 → 4로 올라갔으므로 **Server를 반드시 다시 빌드해야 한다.** 구형 EXE는 위
  표처럼 게시본을 거부한다. protocol 번호는 그대로이므로 Server/Client 동시 재시작 요구는 이
  변경 때문에 새로 생기지 않지만, Server EXE와 게시본은 세대를 맞춰야 한다.
- `Server.exe --vehicle-riding-contract-test`를 **초록으로 통과시키지 못했다.** 빌드를 못 해서
  구형 EXE로는 의미가 없다. 재빌드 뒤 사용자가 실행해야 한다. 대신 실제 파서를 격리 probe로
  링크해 런타임 검증했고, 그 결과가 위 표에 있다.
- **원작 쾌속 게이지는 구현하지 않았다.** `BoostGauge` 500 용량, 1회 500 소모, `GaugePerUU`
  거리 회복이 원작의 실제 제한이다. 지금은 쿨타임만 제한이므로 쿨타임이 끝날 때마다 다시 누르면
  사실상 계속 빠르게 갈 수 있다(8202는 쿨타임 5000 < 지속 7000이라 끊김이 없다). 게이지를 원하면
  플레이어 게이지 값·거리 누적·HUD 복제가 필요하고 그때는 snapshot 필드가 늘어난다.
- **부스터 카메라(CameraSetting 1003: FOV 70 / Pitch −42 / ZoomDist 1700)는 적용하지 않았다.**
  요청에 없었고 기본 OFF가 맞다고 판단했다. 원하면 `shipCamera` 옆에 같은 방식의 블록으로 넣을 수 있다.
- **미니맵 회색은 원인만 규명하고 고치지 않았다.** 안개와 무관하다. 미니맵은 렌더타깃 캡처가 아니라
  정적 PNG 스프라이트(`UI/Minimap/Maps/BernCastle.png`, 1280×1280, 파일 존재)이므로 scene 안개가
  물들일 수 없다. 배가 가는 `BernSea` 영역(X 192~340, Z −262~−156)이 이미지가 덮는 범위
  (X 14.72~270.72, Z −220.48~35.52) 밖이라, 영역의 약 98.6 %가 UV 밖(셰이더 `discard`)이거나
  알파 0으로 떨어진다. 그래서 지도 스프라이트가 사실상 아무것도 그리지 않고 아래
  `frame_bg.png`(검정 30 % 불투명)만 남아 회색 판으로 보인다. 육지 spawn에서는 약 55 %가 실제
  지도라 정상이다. 해결은 코드가 아니라 데이터다 — `MinimapAreas.json`의 BERN 월드 범위를 넓히고
  지도 이미지를 바다까지 다시 그리거나, 바다용 영역 행을 추가한다(후자는 `Parse_Level`이 `LEVEL`로만
  키를 잡으므로 코드 변경이 함께 필요하다). 별도 작업으로 두었다.

## 사용자 확인 순서

1. Visual Studio에서 **Server와 Client를 같은 구성으로 Build**한다(bootstrap 세대 4 반영).
2. Server 실행 → Client → Lobby → Bern 입장.
3. 선착장 NPC에서 배에 탑승 → **안개가 사라져 바다가 맑게 보이는지** 확인.
4. 탑승 중 우클릭으로 항해하며 **Space**를 누르고 더 빨라지는지 확인. 배마다 3~7초다.
5. 하차 → 안개와 카메라가 원래대로 돌아오는지 확인.
6. 안개를 완전히 끄는 대신 옅게 남기고 싶으면 `Data/Camera/Bern.camera.json`의
   `shipFog.disable`을 `false`로 바꾸고 Bern에 다시 들어온다(리빌드 불필요).

SHIP_FOG_BOOST_DONE
