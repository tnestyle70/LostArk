# 2026-09-10 World Object 저작 Emission 계획서

대응 결과: [World Object 저작 Emission 결과](2026-09-10_WORLD_OBJECT_AUTHORED_EMISSIONS_RESULT.md).
브랜치: `codex/kouku-arena-fire-0910`.

## G00. 현재 실제 상태와 이번 경계

팀장님이 09-07~09-09에 올린 World Object Tool(`b5634787` → `bf43ea52` → `6eff7787` →
`6b0d9ba2` → `c1e7b50c`)의 저장 계층은 세 개다.

```text
부모 Object Resource  모델 / diffuse / modelPreScale / animated / 기본 scale / anchor / Default Motion
자식 Motion           template + instance 한 쌍. Lifetime, Transform 키, Append Clip, Effect rows,
                      Physics / Motion / Emission, On Complete(STOP/HOLD/LOOP/NEXT)
Composition Object box  Workbench에서 부모를 Append한 배치 인스턴스. 절대 TRS와 시각을 소유
```

모션 하나는 오브젝트 하나다. `Is_SingleObjectMotion`(`WorldSequencePlayer.cpp:38`),
`Build_State`의 고정 슬롯 `"object"`(`WorldObjectTool.cpp:401-406`), Workbench 목록의 단일 binding
검사(`MainApp.cpp:8882-8883`)가 모두 이 전제를 쓴다. 다중 슬롯 template은 커튼·arena_rise·마리오 공처럼
`MAP_PLACEMENT` 별칭 전용이고 OBJECT_RESOURCE에는 하나도 없다.

복수 생성은 이미 `objectMotion`의 `count(1..128) / intervalMs / spreadDegrees / seed`가 소유한다.
런타임은 `for (emitter = 0; emitter < motion.count; ++emitter)`로 emitter마다 clone을 만들고
`emissionKey = motionId:start:emitter`로 식별한다(`WorldSequencePlayer_Objects.cpp:332-343`).
다만 이 emitter는 **같은 원점에서 seed 난수 방향으로** 뿜는 공 튀기기용이라 위치·방향을 지정할 수 없다.

그래서 3관문 외곽불 60개와 갈고리 18개는 Composition의 World 박스 78개로 배치돼 있었다. 배치가
Composition에 있으므로 World Object Tool에서는 한 개만 보이고, 개수·위치·시각을 그 도구에서 편집할 수 없다.

이번 G의 경계는 다음과 같다.

- 팀장님의 `Emission`을 **seed 난수**에서 **저작 목록**으로 확장한다. 새 개념·새 창·새 파일을 만들지 않는다.
- `formatVersion`은 3을 유지하고 새 필드는 optional로 넣는다. 목록이 없으면 기존 동작과 바이트 단위로 같다.
- binding은 계속 1개다. `Is_SingleObjectMotion`, Workbench 목록, Append 경로는 건드리지 않는다.
- Server C++과 protocol은 바꾸지 않는다. 갈고리 잡기 판정은 projector가 행 값을 Server가 이미 읽는
  `baselinePosition / baselineYawDegrees / startDelayMs`에 구워 넣는다.

## G01. 저장 계약

정본은 `Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json` v3다.
`objectMotion`에 optional `emissions` 배열을 추가한다.

```json
"objectMotion": {
  "velocity": [0, 0, 0],
  "acceleration": [0, 0, 0],
  "angularVelocityDegrees": [0, 24.0, 0],
  "revolutionDegreesPerSecond": [0, 24.0, 0],
  "revolutionOffset": [12.6, 0, 0],
  "count": 10, "intervalMs": 0, "spreadDegrees": 0, "seed": 1,
  "emissions": [
    {"positionOffset": [11.983, 0.0, 3.893], "yawDegrees": -324.0, "startDelayMs": 0}
  ]
}
```

| 필드 | 의미와 불변식 |
|---|---|
| `positionOffset` | 모션 로컬 프레임의 행 위치. 유한하고 ±100000 이내 |
| `yawDegrees` | 그 행의 방향. −36000..36000. 로컬 이동과 공전을 함께 돌린다 |
| `startDelayMs` | 그 행의 생성 지연. 0..600000이고 최대값이 Lifetime보다 작아야 한다 |

행이 있으면 `count`는 행 수와 같아야 하고 `intervalMs`와 `spreadDegrees`는 0이어야 한다. 한 문서가
같은 질문에 두 답을 갖지 않게 하는 규칙이며 기존 seed emitter 계약(`(count-1)*intervalMs < durationMs`)의
일반화다. 행이 비면 지금과 완전히 같다.

합성 순서는 다음과 같다. 행 yaw가 로컬 이동 전체를 돌리므로 한 모션이 여러 레인으로 갈라지거나
한 중심을 도는 링이 된다.

```text
world = Scale(resource x key) x Rot(key x 자전) x T(로컬)
        x RotY(행 yaw) x T(행 offset)
        x T(instance.position)          # placement / emission anchor가 없을 때만
        x anchor basis
        x [box placement: Scale x RotYPR x T]
로컬 = key.positionOffset + v*t + 0.5*a*t^2 + (R(w*t)*orbit - orbit)
```

행이 `R(yaw)*orbit`을 offset으로 가지면 그 행의 궤적은 `R(yaw + w*t)*orbit + boxPos`가 되어
**모든 행이 box 위치를 중심으로 하는 한 원**을 돈다. `Distribute on Ring` preset이 이 식을 쓴다.

Composition 쪽에는 `presentationOccurrences`에 optional `worldEmissionIndex`(0..127)를 추가한다.
WORLD anchor Collider에만 허용하며 그 박스가 따라갈 행 번호다. 0이면 저장하지 않는다.

## G02. 파일 목록

| 구분 | 경로 | 역할 |
|---|---|---|
| 수정 | `Client/Public/WorldSequenceDocument.h` | `WORLD_SEQUENCE_OBJECT_EMISSION`, `EmissionCount/EmissionDelayMs/LastEmissionDelayMs` |
| 수정 | `Client/Private/WorldSequenceDocument.cpp` | parse, `Is_ValidEmissionList`, Validate, writer, `Is_Equivalent` |
| 수정 | `Client/Public/WorldSequencePlayer.h` | pivot 조회에 `emissionIndex` |
| 수정 | `Client/Private/WorldSequencePlayer.cpp` | span 계산의 emitter 지연 |
| 수정 | `Client/Private/WorldSequencePlayer_Objects.cpp` | emitter 순회, 합성식, pivot 선택 |
| 수정 | `Client/Public/WorldObjectTool.h` | Ring preset 입력 두 개 |
| 수정 | `Client/Private/WorldObjectTool.cpp` | `Authored Emissions` 표와 `Distribute on Ring` |
| 수정 | `Client/Public/KoukuSaydonCompositionDocument.h` | `iWorldEmissionIndex` |
| 수정 | `Client/Private/KoukuSaydonCompositionDocument.cpp` | `worldEmissionIndex` parse와 두 writer |
| 수정 | `Client/Public/KoukuSaydonActionWorkbench.h` | 리소스 행의 `iEmissionCount` |
| 수정 | `Client/Private/KoukuSaydonActionWorkbench.cpp` | Box Detail의 `Emission index` |
| 수정 | `Client/Private/MainApp.cpp` | 리소스 행에 행 수 공급 |
| 수정 | `Client/Private/KoukuSaydonPresentationPlayer.cpp` | 제품 문서 읽기와 pivot 전달 |
| 수정 | `Client/Public/Level_KakulSaydonArena.h`, `Client/Private/Level_KakulSaydonArena.cpp` | pivot 두 함수에 행 번호 |
| 수정 | `Tools/MapPipeline/Publish-MapAuthoring.ps1` | `emissions` 구조 검증과 생성 창 |
| 수정 | `Tools/MapPipeline/test_world_sequence_authoring_contract.py` | 유효 1 + 거부 8 케이스 |
| 수정 | `Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py` | 행 값을 Server baseline에 fold |
| 추가 | `Tools/KoukuSaydonPipeline/migrate_gate3_fire_hook_emissions.py` | 박스 78/18 → 행 이전과 재현 검증 |

새 C++ 파일이 없으므로 `.vcxproj`와 `.filters` 등록은 필요하지 않다.

## G03. 런타임 합성 교체 블록

`Client/Private/WorldSequencePlayer_Objects.cpp`의 `Sample_ObjectWorld`에서 `instance.position`을
덧셈에서 행렬로 옮기고 행을 그 사이에 끼운다. 목록이 비면 `T(pos) x T(ip) == T(pos + ip)`이므로
기존 결과와 동일하다.

```cpp
    const vector_t position = XMLoadFloat3(&key.positionOffset) +
        velocity * seconds + XMLoadFloat3(&motion.acceleration) * (.5f * seconds * seconds) +
        XMVector3TransformNormal(orbit, revolution) - orbit;
...
    matrix_t world = XMMatrixScalingFromVector(scale) * rotation * XMMatrixTranslationFromVector(position);
    /* An authored row turns the whole local motion, orbit included, so one row
       set fans a path into lanes or lays a ring of copies orbiting one centre. */
    if (!motion.emissions.empty())
    {
        const auto& emission = motion.emissions[(std::min)(static_cast<size_t>(emitter), motion.emissions.size() - 1u)];
        world *= XMMatrixRotationY(XMConvertToRadians(emission.yawDegrees)) *
            XMMatrixTranslationFromVector(XMLoadFloat3(&emission.positionOffset));
    }
    if (!(active.placement || anchor.emissionOverride))
        world *= XMMatrixTranslationFromVector(XMLoadFloat3(&instance.position));
    if (!anchor.emissionOverride) world *= basis;
```

emitter 순회는 지연을 `EmissionDelayMs`에서 읽는다.

```cpp
            for (uint32_t emitter = 0; emitter < motion.EmissionCount(); ++emitter)
            {
                const f32_t delayMs = static_cast<f32_t>(motion.EmissionDelayMs(emitter));
                const f32_t ageMs = localMs - delayMs;
                if (ageMs < 0.f || (!holdFinalPose && !sequence.effectTracks.empty() && ageMs >= sequence.durationMs)) continue;
                const f32_t birthMs = emissionStartMs + delayMs / emissionRate;
```

`Try_GetObjectPivot`은 행이 있으면 `emissionIndex`가 가리키는 clone 하나를 고르고, 없으면 기존
단일 오브젝트 계약을 그대로 지킨다. `Try_GetSequencePivot`은 배치 별칭에 행 번호가 오면 거부한다.

## G04. projector가 Server에 굽는 값

`_load_region_world`가 행을 읽어 baseline에 접는다. Client가 행을 로컬 모션 뒤·박스 TRS 앞에
적용하므로 offset에는 placement scale만 닿고 행 yaw는 baseline yaw에 더해진다.

```python
        if emission is not None:
            placement_scale = placement["scale"] if cue is not None and "placement" in cue else [1, 1, 1]
            offset = _rotate_y([a*b for a,b in zip(emission["positionOffset"], placement_scale)], yaw)
            position = [a+b for a,b in zip(position, offset)]
            yaw += float(emission["yawDegrees"])
```

지연은 Client가 재생률 **뒤에** 빼므로 Server의 단일 `startDelayMs`에는 `delay / speed`를 싣고
나누어떨어지지 않으면 거부한다. `_sample_region_world`도 같은 순서로 맞춘다.

```python
        emission_delay = int(emission["startDelayMs"]) / speed
        if abs(emission_delay - round(emission_delay)) > 1e-9:
            raise CompositionError("WORLD Trigger emission delay must divide evenly by its playback speed")
```

`_require_single_static_object_motion`은 "count 1"에서 "count 1 또는 저작 행 하나"로 넓히고
행 번호가 범위를 벗어나면 거부한다.

## G05. Tool UI

`Object Detail`의 기존 `Physics / Motion / Emission` 섹션 **안에** `Authored Emissions`를 둔다.
행이 있으면 `Count / Creation Interval / Spread`를 비활성화하고 행 수로 동기화한다.

```text
Authored Emissions
  표: #  Offset X/Y/Z (m)  Yaw (deg)  Start Delay (ms)  [Dup] [Del]
  [Add Emission] [Clear Emissions]
  Ring Count [ ]  Ring Start (deg) [ ]  [Distribute on Ring]
```

`Distribute on Ring`은 행 i에 `yaw = start + 360*i/N`, `offset = R(yaw) * revolutionOffset`을 넣는다.
그 결과 모든 행이 저장 위치를 중심으로 `|revolutionOffset|` 반경의 한 원을 돈다.
`Lifetime (ms)` 편집은 기존 interval clamp와 같은 자리에서 모든 행 지연도 clamp한다.

Workbench Box Detail의 WORLD anchor에는 행이 2개 이상일 때만 `Emission index`가 보인다.

## G06. 3관문 데이터 이전

`migrate_gate3_fire_hook_emissions.py`가 수행한다. 한 박스 = 한 사본일 때와 한 박스 + 행일 때의
합성식을 비교하면 `ey = 기존 box yaw`, `eo = 기존 box position − base`가 정확한 해다.

- 불 D/E/F × CW/CCW 6모션: base = 공유 공전 중심(아레나 중심), 행 10개씩. 행 반경은 각 모션의
  `revolutionOffset`과 같다(12.6 / 11.7 / 10.8).
- 갈고리 1모션: base = 아레나 중심의 갈고리 높이, 행 18개(3웨이브 × 2방향 × 3레인, 지연 175ms 간격).
  행 지연이 모션 Lifetime을 넘으므로 마지막 키 뒤에 `visible=false` 꼬리 키를 붙이고 Lifetime을
  8000 → 26875로 늘린다. 각 행의 첫 8초 동작은 그대로다.
- Composition: `PATTERN_18` World 박스 78 → 7, `PATTERN_19` 18 → 1. Collider 18개는 살아남은
  박스를 가리키고 `worldEmissionIndex` 0..17을 갖는다.

## G07. 검증

```powershell
python Tools/KoukuSaydonPipeline/migrate_gate3_fire_hook_emissions.py --check
python -m unittest discover -s Tools/MapPipeline -p "test_world_sequence_authoring_contract.py"
python -m unittest discover -s Tools/KoukuSaydonPipeline -p "test_project_kouku_saydon_composition.py"
python Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py --mode validate
git diff --check
```

Visual Studio를 닫은 뒤 아래를 실행해야 런타임에 반영된다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_LUT_MIDNIGHTC_ED -Scope WorldSequences -Mode Publish
python Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py --mode publish
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon
```

그다음 Client(x64 Debug)를 빌드하고 Server를 재시작한다. 화면 확인은 사용자가 직접 한다.
