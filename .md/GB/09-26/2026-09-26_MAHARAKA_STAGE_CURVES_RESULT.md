# 2026-09-26 마하라카 워터팡 무대 회전·붕괴·복구 곡선 판독 결과

> 2026-09-27 후속 정정: 아래 복구의 “회전 0, wrap artifact” 결론은 확정 근거가 부족했다.
> 끝점 +180/-180이 같은 방향인 것과 그 사이 Euler 보간이 회전하는 것은 별개다.
> 원본 Euler 키와 설치본 Default__InterpTrackMove를 확인해 키를 임의 unwrap하지 않고
> 실제 중간 자세를 보존했다. 사용자 시각 검증은 아직 없으며 정상 복구 동작이라고 확정하지 않는다.
> 붕괴에서 생략된 InterpLength는 설치본 Engine.u Default__InterpData의 5.0초를 상속한다.
> 09-27 후속 결과의 Camera/WorldSequence 및 효과음 게시 상태가 이 문서의 과거 미설치 상태를 대체한다.

G02 2차가 무대를 찾았지만 `interptrackmove` 87개의 키를 읽지 못했다고 남겼다. 이번 작업의 목표는
그 native 구조를 파싱하는 것이었다. **결과부터 말하면 native 파서는 필요 없었다.**

## 0. 선행 결과 정정

G02 2차의 "`PosTrack`/`EulerTrack`이 tagged property로 직렬화되지 않아 87/87 모두 못 읽음"은
**틀렸다.** 공용 파서 `Tools/LevelPlacementExtractor/extract_ue3_placements.py`는 `:925~926`에서
`interpcurvefloat`/`interpcurvevector`를 tagged struct로 이미 지원하고, `:960` 이후가 `points`
배열을 점 단위로 풀어낸다. 87개 전부 이미 읽히고 있었다.

오해의 원인은 G02 2차 자신의 덤프 스크립트다. `dump_scene03b.py`가
`rec[k + "KeyCount"] = len(v) if isinstance(v, list) else None`로 기록하는데, 파서가 돌려주는
값은 list가 아니라 `{"size":…, "properties":{"points":{…}}}` dict다. 그래서 `postrackKeyCount`가
`null`로 찍혔고, 실제 키는 `value["properties"]["points"]["value"]` 안에 온전히 들어 있었다.

이번 작업은 **두 번째 파서를 만들지 않았다.** 같은 공용 파서를 그대로 호출했다.

```
InterpTrackMove 87  curves decoded 87
```

## 1. 이번 G에서 실제 바꾼 것

제품 코드·저작 데이터·게시본·Resources 변경 **0건**. `out/MaharakaContinuation_20260926_193455/StageCurves/`
아래에만 기록했다. committed 도구도 수정하지 않았다.

| 파일 | 내용 |
|---|---|
| `parse_stage_curves.py` | 원본 재파싱, 87 곡선·그룹·트랙·바닥 조각 추출 |
| `resolve_stage_wiring.py` | Kismet 결선(신호→Matinee→InterpData→그룹→배우) 추적 |
| `analyse_stage_motion.py` | Matinee별 타일 모션 요약 |
| `rebuild_tile_binding.py` | 연결표를 손으로 적은 값 대신 파싱 결과로 재생성 |
| `stage-structure.json` / `move-tracks.json` / `stage-wiring.json` / `stage-motion-summary.json` | 판독 결과 |
| `conversion-mapping-draft.json` | WorldSequence 변환 매핑 초안 |

## 2. 원본 근거

- package `A89UVJO9YX8XOUPN9JHI29ONJXOX7SC.upk`, 논리명 `lv_ocn_eventis_mhp_scene03b`,
  packageVersion 868, export 740개.
- 좌표 매핑은 G02 2차가 `pool01_sm`으로 검산한 `project = (src.x/100, src.z/100, -src.y/100)`을 그대로 썼다.

### 2.1 신호 → Matinee 결선

`SeqEvent_RemoteEvent` 5개(수신)와 `EFSeqAct_EndRemoteEvent` 5개(발신)의 `eventname`이
**archetype 기본값이 아닌 실제 값으로 읽혔다.** G02 2차가 미확정으로 남긴 항목이다.

| 신호 | 수신 export | 시작하는 Matinee | InterpData | 길이(초) | Completed 시 발신 |
|---|---|---|---|---|---|
| `ground_destroy_shake` | 480 | `efseqact_matinee_13` (40) | `interpdata_13` | 3.0031752586 | `ground_destroy_shake` |
| `ground_destroy` | 479 | `efseqact_matinee_14` (41) | `interpdata_14` | (미직렬화) | `ground_destroy` |
| `ground_destroy_repair` | 478 | `efseqact_matinee_4` (45) + `efseqact_matinee_18` (43) | `interpdata_4` / `_18` | 2.0 / 1.0 | `ground_destroy_repair` |
| `ta_grounddestroy` | 477 | `efseqact_matinee_18` (43) + `endremoteevent_4` | `interpdata_18` | 1.0, `bLooping=true` | `ta_grounddestroy` (자기 재진입) |
| `mode_start` | 476 | `setcameratarget_0`, `togglecinematicmode_0`, `togglehidden_0` | — | — | — |

`ta_grounddestroy`는 자기 자신을 다시 발신해 루프를 만든다. `mode_start`는 카메라·시네마틱 모드
전환이고 무대 모션이 아니다.

### 2.2 그룹 → 배우 바인딩

네 무대 Matinee 전부 `T01`~`T18`을 **같은 18개 바닥 InterpActor**에 묶는다. 사슬이 끊기지 않는다.

`bg_ocn_etc_floor01b`와 `bg_ocn_etc_floor01a`가 9개씩 정확히 교대하고, 재질은 두 메시 모두
`lv_ocn_eventis_mhp.mat.bg_ocn_etc_floor01a_mi_lnh` 하나다.

| 그룹 | export | 배우 | 메시 | yaw(도) | 링 방위각(도) | 반경(m) |
|---|---|---|---|---|---|---|
| T01 | 150 | interpactor_59 | floor01b | 180.000 | 269.908 | 6.215 |
| T02 | 105 | interpactor_18 | floor01a | 199.688 | 289.598 | 6.172 |
| T03 | 133 | interpactor_43 | floor01b | 219.375 | 309.310 | 6.169 |
| T04 | 134 | interpactor_44 | floor01a | 239.436 | 329.388 | 6.167 |
| T05 | 135 | interpactor_45 | floor01b | 259.651 | 349.638 | 6.165 |
| T06 | 136 | interpactor_46 | floor01a | 280.129 | 10.145 | 6.165 |
| T07 | 137 | interpactor_47 | floor01b | 300.476 | 30.524 | 6.167 |
| T08 | 138 | interpactor_48 | floor01a | 320.625 | 50.690 | 6.169 |
| T09 | 139 | interpactor_49 | floor01b | 340.312 | 70.402 | 6.172 |
| T10 | 141 | interpactor_50 | floor01a | 0.000 | 90.093 | 6.175 |
| T11 | 142 | interpactor_51 | floor01b | 19.688 | 109.835 | 6.160 |
| T12 | 143 | interpactor_52 | floor01a | 39.375 | 129.564 | 6.166 |
| T13 | 144 | interpactor_53 | floor01b | 59.722 | 149.934 | 6.173 |
| T14 | 145 | interpactor_54 | floor01a | 79.849 | 170.050 | 6.181 |
| T15 | 146 | interpactor_55 | floor01b | 99.954 | 190.126 | 6.188 |
| T16 | 147 | interpactor_56 | floor01a | 119.685 | 209.798 | 6.194 |
| T17 | 148 | interpactor_57 | floor01b | 140.032 | 230.087 | 6.197 |
| T18 | 149 | interpactor_58 | floor01a | 160.312 | 250.290 | 6.197 |

T10은 `rotation` property를 직렬화하지 않는다. UE3가 기본값을 생략한 것이고, 링 중심 기준
방위각이 90.093°로 20° 진행이 예측하는 자리와 정확히 맞는다. 모든 타일에서 yaw와 방위각의 차이가
일정하게 약 90°인 것도 정합성을 뒷받침한다.

이 표는 처음에 손으로 적었다가 9행의 `actorName`이 틀렸다. `rebuild_tile_binding.py`로 파싱
결과에서 재생성했고, 18개·9+9 교대·인접 중복 없음을 스크립트가 assert한다.

### 2.3 각 Matinee의 실제 모션

**`interpdata_13` — 흔들림 (`ground_destroy_shake`, 3.0032초, 타일당 61키 = 약 20fps)**

위치는 사실상 움직이지 않는다. 18타일 전부 위치 진폭이 **1 mm 미만**(x 0.01~0.31 mm, y 0.00~0.39 mm,
z 0.00~0.01 mm)이다. 실제 움직임은 **yaw 진동**이며 진폭이 floor01b 타일 9개는 **2.0278°**,
floor01a 타일 9개는 **2.0174°**다. pitch는 전부 0, roll은 최대 0.0008°다.
즉 이것은 "위치 흔들림"이 아니라 **제자리 요(yaw) 떨림**이다.

**`interpdata_14` — 붕괴 (`ground_destroy`, 타일당 6키)**

타일마다 **3.083 m** 이동한다. 내역은 project Y로 **3.01 m 하강**, 바깥으로 최대 0.92 m이며
링 반경이 6.17 m에서 **6.97 m**로 벌어진다. 타일별 yaw sweep은 −16.699 / −0.813 / 1.978 / 4.482 /
17.468° 다섯 값 중 하나이고, pitch span 최대 0.571°, roll span 최대 1.626°다.
종료 시각은 타일마다 3.437~3.643초로 조금씩 다르다.
키 6개 중 3개가 `cim_curveautoclamped`, 3개가 `cim_curveuser`다.

이 Matinee만 타일 외 그룹을 갖는다 — `water`/`w02`/`w03`(각 toggle 트랙 1개)와 `sound`(AkEvent).
변수 링크는 interpactor 21개(중복 포함, 고유 18개 — `Sound` 그룹명이 3회 중복)와 **emitter 45개**이며
emitter 그룹명은 전부 `Water`다.

**`interpdata_18` — 붕괴 상태 유지 (`ta_grounddestroy`, 1.0초, `bLooping=true`)**

`interpdata_14`와 **같은 저작 모션을 −4.192초 시프트로 다시 구운 것**이다. 키 단위로 대조한 결과
위치 OutVal이 18/18 타일에서 **비트 단위로 동일**(최대 차이 0.000 cm), euler OutVal 최대 차이
**4.941e-07도**, 자동 탄젠트 최대 차이 **5.760e-02**다. 앞의 둘은 부동소수 잡음이고 탄젠트 차이는
재bake에서 온 것으로 본다(이 해석은 추론이다).
키 시각이 −4.192초에서 −0.755~−0.549초 사이에 있고 `interpLength`가 1.0이므로, 재생 창이 마지막 키
이후에 놓여 모든 타일이 **무너진 자세를 유지**한다.

**`interpdata_4` — 복구 (`ground_destroy_repair`, 2.0초, 타일당 3~4키)**

이동 0, 회전 0이다. 타일을 저작 자세로 2초간 유지·복귀시킨다.

> 정정: 중간 집계에서 t01의 roll span이 360°로 나와 "한 타일이 360° 회전"으로 읽힐 뻔했다.
> 실제 키는 t=0에서 roll **180.0**, t=1과 t=2에서 roll **−180.0**이다. 180과 −180은 같은 방향을
> wrap 양쪽에서 표기한 것이고 회전이 아니다. `max−min` 방식이 만든 측정 artifact였다.

### 2.4 그 외 트랙 55개

`interptrackakevent` 6(그룹 `sound`, 제목 `AkEvent_BGM`·`AkEvent_BGM_02` 및 무제 2), `interptrackdirector` 5,
`interptrackevent` 5, `interptracktoggle` 5(`water`/`w02`/`w03`/`splash`/`blur`),
`interptrackfloatmaterialparam` 19, `interptrackfloatprop` 9, `interptrackfade` 2,
`interptrackvectormaterialparam` 2, `interptrackanimcontrol` 1, `interptrackcolorscale` 1.

`interptrackfloatmaterialparam` 19개는 무대 바닥이 아니라 **컷신 후처리**에 붙는다. 파라미터가
`opacity`, `opacity_intensity`, `circle radius`, `noise speed`, `channel spread range`, `radius`,
`vignetting radius`, `vignetting hardness`이고 소속 그룹이 `post`/`blur`/`splash` 계열이다.
무대 4개 Matinee의 타일 그룹은 `interptrackmove` **하나씩만** 갖는다.

## 3. 상태 구분

| 항목 | 상태 |
|---|---|
| 원본 추출·판독 | 완료 (87/87 곡선, 결선, 바인딩) |
| 후보 생성 | 변환 매핑 초안만 |
| live 설치 | **없음** |
| 게시 | **없음** |
| 빌드 | **없음** (다른 작업이 빌드 잠금 사용 중, 지시대로 미실행) |
| 제품 소비자 연결 | **없음** |
| 사용자 수동확인 | 해당 없음 (화면에 나타나는 변경 없음) |

## 4. 실행한 명령

| 명령 | 결과 |
|---|---|
| `python -B parse_stage_curves.py --output .` | exit 0, `InterpTrackMove 87 curves decoded 87` |
| `python -B resolve_stage_wiring.py --output .` | exit 0, 신호 5개·Matinee 6개 결선 |
| `python -B analyse_stage_motion.py` | exit 0, 무대 4개 Matinee × 18타일 요약 |
| `python -B rebuild_tile_binding.py` | exit 0, 18행 재생성 + 9/9 교대 assert 통과 |

빌드·publisher는 실행하지 않았다.

## 5. WorldSequence 변환 가능성

대상은 `Data/Maps/Authoring/LV_OCN_EVENTIS_MHP/LV_OCN_EVENTIS_MHP.worldsequences.json`
(`lostark.world-sequences` formatVersion 3)이다. 쿠크 문서를 실측해 판단했다.

**표현 가능한 것**

- `templates[].tracks[].keys[]`가 `timeMs`, `positionOffset`, `rotationQuaternion`,
  `scaleMultiplier`, `visible`를 갖는다. 타일 이동·회전과 `water`/`w02`/`w03` 가시성 토글이 들어간다.
- `instances[].bindings[].targetKind`의 `MAP_PLACEMENT`가 쿠크 문서에서 이미 826회 쓰인다.
  서브레벨을 import하면 타일 18개를 그 배치에 묶을 수 있다.
- `instances[].motionEnd`가 `STOP`/`HOLD`/`LOOP`/`NEXT`를 지원한다. 흔들림 STOP, 붕괴 HOLD,
  `ta_grounddestroy` 유지 LOOP로 대응된다.
- `templates`에 `effectTracks`와 `colliderTracks`가 있어 `Water` emitter 45개와 서버 충돌 변경이
  들어갈 자리가 있다.

**표현 못 하는 것 (해결해야 할 결정)**

1. **키별 탄젠트가 없다.** 스키마의 `interpolation`은 template당 하나(`LINEAR` 또는 `SMOOTH_STEP`)이고
   탄젠트 필드가 없다. 원본은 키마다 `ArriveTangent`/`LeaveTangent`와 `cim_curveautoclamped` /
   `cim_curveuser` 모드를 갖는다. 흔들림은 61키 20fps라 LINEAR로도 근사되지만, **붕괴는 6키 중
   3키가 `cim_curveuser`**라 LINEAR로 바꾸면 눈에 띄게 달라진다. 원본 곡선을 조밀하게 재샘플하든
   스키마에 탄젠트를 추가하든 소유자 결정이 필요하다. 이번에 임의로 고르지 않았다.
2. **음수 키 시각.** `interpdata_18`이 −4.192초에서 시작하는데 `timeMs`는 음수를 쓰지 않는다.
   시간 시프트를 명시하거나 "최종 자세 HOLD"로 표현할지 정해야 한다.
3. **Euler→quaternion 축 규약 미확정.** 원본 `EulerTrack`의 `OutVal`은 도 단위 벡터인데, 그 성분을
   pitch/yaw/roll에 어떻게 대응시키고 `project = (x/100, z/100, −y/100)` 매핑 아래 어떤 축 순서로
   합성하는지 이번에 검증하지 않았다. **붕괴는 pitch·roll이 0이 아닌 유일한 무대 모션이라
   여기가 정확성의 최대 위험이다.** 추측해서 적지 않았다.
4. **사운드와 emitter 정의는 이 move 트랙 밖에 있다.** `sound` 그룹의 AkEvent와 `Water` emitter
   45개는 각자 소유자가 필요하다.

## 6. 미완료와 다음 조사 위치

- Euler→quaternion 축 규약 → 기존 Matinee 변환 규칙 문서와 `CWorldSequencePlayer`의 실제 소비 코드 대조
- 탄젠트 손실 대응(재샘플 vs 스키마 확장) → 소유자 결정 사항
- `Water` emitter 45개의 파티클 정의 → `scene03b`의 `emitter` 48개 export
- `sound` 그룹 AkEvent 트랙의 실제 이벤트 ID → AkEvent 트랙 payload 미판독
- `interptrackevent` 5개·`interptrackdirector` 5개 → 컷신 계열, 이번 범위 밖
- 무대 붕괴 시 Server support/collision/navigation 변경 → 설계서 G09 범위

이 문서는 원본 판독까지다. 저작 문서를 쓰지 않았고 제품에 연결하지 않았다.
