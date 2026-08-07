# 2026-08-08 차원술사 A 2050210 Hit 1 Seed 결과서

## 1. 결론

다른 세션의 A 분석 방향은 유효하다. 특히 A는 단일 Bloom Mesh가 아니라 한 타격당
`Mesh 11 + Sprite 8`을 겹친 SwingHit 시스템을 네 번 반복하고, Screen Post와 잔상이
누적되는 구조다.

첫 인게임 검증을 위해 canonical A를 변경하지 않는 독립 candidate를 생성했다.

```text
effect.dimensionmaster.skill.2050210.a-restoration-candidate
```

candidate에는 첫 타격의 Body, Rim, Highlight와 비활성 Afterimage가 들어 있다. Effect Tool에서
직접 Load하고 Solo/튜닝할 수 있으며 A 키의 canonical runtime에는 아직 반영되지 않는다.

## 2. 분석 재검증 결과

### 2.1 일치한 내용

```text
전체 Elements                  117
Static Mesh-backed              48
Sprite                           52
Light                             4
Screen Post                      13
Screen RGB Noise                  9
Screen Zoom Blur                  4
SwingHit 1회        Mesh 11 + Sprite 8
SwingHit 시작       0.25 / 0.60 / 0.90 / 1.30초
마지막 Ribbon/Cube 꼬리          1.8535초
```

네 타격의 Position과 실제 Texture slot 조합도 기존 계획과 일치했다.

### 2.2 교정한 내용

#### Lifetime

`detail.timing.lifeTimeSeconds=0.10`은 emitter window다. 실제 화면상 Mesh 수명은
`detail.particle.lifeTimeSeconds`에 따로 있었다.

| 원본 layer | Particle Lifetime |
|---|---:|
| `fm_h_swing_02` Body | `0.50 s` |
| `fm_h_swing_02` Rim | `0.30 s` |
| `fm_m_trail_002` Flow | `0.30 s` |
| `fm_m_trail_01` Afterimage | `1.70 s` |
| `fm_a_broken_012` Crack | `0.90~1.00 s` |

따라서 수동 MESH를 최종 `0.10~0.18초`로 줄이는 기존 계획은 폐기했다. Highlight만
의도적으로 `0.20초`의 짧은 별도 layer로 사용한다.

#### Scale

Body의 `0.0341`은 다음 원본 수치에서 환산된다.

```text
Particle start size 0.031 × parent transform scale 1.1 = 0.0341
```

Rim은 약 `0.03135/0.03355`, Afterimage는 `0.077/0.066`이다. 수동 Tool 기본 `0.01`은
일반적인 안전 생성값이지 A 원본 크기가 아니다.

#### Rotation/Revolution

- Body start rotation `-0.05 turn`과 rotation rate `-1 turn/s` 단서가 있다.
- Rim에는 `0.5/-0.3 turn` 시작 회전과 `-0.27 turn/s` 단서가 있다.
- 그러나 RotationRateMultiplyLife 원본 곡선을 현재 선형 Lerp 하나로 정확히 옮길 수 없다.

따라서 candidate의 Body `-280 → -70°/s`, Rim `-140 → 0°/s`는 source exact가 아니라
GPU A/B용 `PROJECT_TUNED_SEED`다.

#### Rim Noise

제안된 `fx_j_normal_02.dds`는 현재 A canonical ResourceBindings에서 occurrence가 0이다.
부모 graph 증거가 추가로 확인되기 전까지 Rim Noise에 넣지 않았다.

## 3. 생성된 candidate

### 3.1 Body

```text
Layer       manual.a.hit01.body
Mesh        fm_h_swing_02
Scale       0.0341
Life        0.50 s
Delay       0.25 s
Rotation Z  -18°
Revolution  -280 → -70°/s
Render      Alpha / Two-Sided / Depth Read
Distortion  0.04
```

Resources는 원본 linearflow occurrence의 Base/Noise/Mask/Dissolve를 사용한다.

### 3.2 Rim

```text
Layer       manual.a.hit01.rim
Mesh        fm_h_swing_02
Scale       0.03135 / 0.03355 / 0.03355
Life        0.30 s
Rotation    X 180°, Z -108°
Revolution  -140 → 0°/s
Render      Alpha / Two-Sided / Depth Read
```

Resources는 blackline occurrence의 Base/Mask/Dissolve를 사용하며 Noise는 비워 두었다.

### 3.3 Highlight

```text
Layer       manual.a.hit01.highlight
Mesh        fm_h_swing_02
Scale       0.032
Life        0.20 s
Render      Additive / Two-Sided / Depth Read
Emissive    fx_j_auraline_19_ycl
Intensity   1.5
```

Mask texture를 Emissive carrier로 재사용한 부분은 source exact가 아니라 백색 core 확인용
수동 근사다. 카드나 끊긴 선이 나오면 첫 교체 대상이다.

### 3.4 Afterimage

```text
Layer       manual.a.hit01.afterimage
Mesh        fm_m_trail_01
Scale       0.077 / 0.066 / 0.077
Life        0.70 s audition seed
Visible     false
```

원본 수명은 `1.70초`지만 네 타격에서 과도하게 누적될 수 있으므로 첫 실루엣 검증에서는
끄고, Body/Rim/Highlight 합격 후에만 켠다.

## 4. 변경 파일

| 파일 | 상태 |
|---|---|
| `Tools/EffectPipeline/seed_dimensionmaster_a_candidate.py` | deterministic seed 생성기 추가 |
| `Tools/EffectPipeline/test_seed_dimensionmaster_a_candidate.py` | source identity, resource, lifetime/scale, overwrite 보호 테스트 추가 |
| `Data/Effects/Authored/effect.dimensionmaster.skill.2050210.a-restoration-candidate.effect.json` | Hit 1 candidate 생성 |
| `2026-08-08_DIMENSIONMASTER_2050210_A_FOUR_SLASH_RESTORATION_IMPLEMENTATION_PLAN.md` | lifetime/scale 근거 교정 |

canonical `effect.dimensionmaster.skill.2050210.effect.json`, Assembly, WFX, Runtime Catalog,
PlayerSkills, Animation/Anchor는 변경하지 않았다.

## 5. 자동 검증

| 검증 | 결과 |
|---|---|
| Seed Python tests | PASS, 5 tests |
| Canonical A C++ v12 codec parse | PASS |
| Candidate C++ v12 codec parse | PASS |
| Effect Tool final audit | PASS, documents 16 |
| `git diff --check` | PASS; 기존 shared-tree line-ending warning만 출력 |

생성 직후 candidate SHA-256은 다음과 같다.

```text
47EC805E61DE9B10AD18653248CBFFFD31D93044A5AC80498B792CC8B1D7AAD6
```

C++ codec 검증에는 다음 process-local runtime root를 사용했다.

```text
LOSTARK_RESOURCE_ROOT=C:/Users/user/Desktop/LostArk/Client/Bin/Resources
```

root를 지정하지 않은 첫 실행에서는 canonical과 candidate가 모두 resource file 검증에
실패했다. 이는 candidate 오류가 아니라 harness 실행 환경 오류였으며 root 지정 후 둘 다
PASS했다.

C++/HLSL은 이번 슬라이스에서 변경하지 않았으므로 Client 재빌드는 수행하지 않았다.
현재 Effect Tool은 Authored JSON을 `Refresh Files`로 직접 다시 읽을 수 있다.

## 6. 수동 GPU 검증

아직 수행하지 않았다. 다음 순서로 사용자가 검증한다.

1. Effect Tool의 Data Files에서 `Refresh Files`를 누른다.
2. `effect.dimensionmaster.skill.2050210.a-restoration-candidate`를 Load한다.
3. `manual.a.hit01.body`만 Solo한다.
4. Sample Time `0.30초`, Screen Post OFF 상태로 캡처한다.
5. Body가 원호로 보이면 Rim, Highlight 순으로 추가한다.
6. 세 layer 합성 캡처가 합격한 뒤 Afterimage를 켠다.

첫 캡처에서 볼 것은 색의 완성도가 아니라 다음 네 항목이다.

```text
Mesh가 초승달로 보이는가
Rotation 축이 맞는가
Scale 0.0341이 원작 범위와 가까운가
Mask가 카드 외곽을 제거하는가
```

## 7. 남은 단계

- Hit 1 GPU A/B에 따라 Rotation, Color, Emissive, Distortion seed 교정
- 합격한 그룹을 `0.60/0.90/1.30초`에 복제
- 기존 Sprite/Crack/Ring/SwingDeco와 결합
- candidate SHA guard를 사용하는 canonical merge
- isolated Assembly/WFX/Runtime publish
- Client 재시작 후 실제 A 키 검증

현재 상태는 `Hit 1 인게임 검증 준비 완료`이며 A 4검격 복원 완료가 아니다.

## 8. Data Files 카테고리 목록 교정

사용자 검증에서 `Refresh Files` 뒤 DimensionMaster 목록이 비어 있는 문제가 확인됐다.
기존 `Refresh_DataFiles`는 Authored/Imported 문서 하나가 invalid이거나 Effect ID가 중복되면
정상 문서를 포함한 staged 목록 전체를 폐기했다.

처음에는 category 선택에 즉시 전체 재스캔을 연결했으나, 큰 Authored 문서를 매 선택마다
다시 파싱하여 UI가 오래 멈추는 회귀가 발생했다. 최종 계약은 다음과 같이 교정했다.

- Data Files 창을 처음 열 때 전체 파일 인덱스를 한 번 만든다.
- Authoring Category 변경은 메모리의 cached index만 필터링한다.
- 디스크 전체 재스캔은 사용자가 `Refresh Files`를 누를 때만 수행한다.
- 정상 문서는 모두 목록에 표시한다.
- invalid/duplicate 문서는 해당 행만 격리한다.
- 상태에는 격리 건수와 첫 실패 이유를 표시한다.
- 카테고리의 모든 Effect를 월드에 동시에 재생하지는 않는다. 목록에서 한 문서를
  `Load Selected`하여 active preview로 전환하는 계약은 유지한다.

검증:

| 검증 | 결과 |
|---|---|
| Effect Tool final audit | PASS |
| Client x64 Debug incremental build | PASS, exit 0 |
| 새 Client.exe | `Client/Bin/Debug/Client.exe`, 2026-08-08 01:50:20 |
| 실제 카테고리 선택 UI | 사용자 재검증 대기 |

## 9. Hit 그룹과 개별 Element 튜닝 교정

사용자 인게임 검증에서 한 검격의 Body, Rim, Highlight가 각각 존재하지만 All Effects가
`groupId`를 계층으로 표시하지 않아 개별 Element와 합성된 한 타격을 오가기 어려운 문제가
확인됐다. 저장 문서를 Body 파일, Hit 파일, Skill 파일로 중복 분리하지 않고 기존 v12의
`groupId`를 다음 저작 계층으로 사용하도록 교정했다.

```text
Effect Document
└─ Element Groups
   └─ Hit 01 | manual.a.hit01
      ├─ body
      ├─ rim
      ├─ highlight
      └─ afterimage
```

- Element 행을 클릭하면 해당 Element의 Effect Detail draft를 연다.
- `Solo Element`는 단일 Mesh/Material/Transform을 진단한다.
- `Solo Group`은 같은 `groupId`의 Element만 합성해 한 타격을 진단한다.
- `Mute Group`은 선택 그룹을 제외하고 나머지 Effect를 진단한다.
- `Complete Effect`는 향후 Hit 01~04와 보조 layer 전체를 재생한다.
- Data Files는 저장된 Effect Document 목록이며 runtime layer outliner로 중복 사용하지 않는다.

`Visible`은 기존 Effect Detail draft에서 Element별로 편집 가능하다. 숫자 변경은 live preview,
Apply는 Active Document 메모리, Save는 Authored 파일에 각각 반영되는 기존 경계를 유지한다.

## 10. A Hit 01 회전축과 시드 교정

현재 캡처에서 세 갈래 발톱처럼 벌어진 형상은 원작의 세부 형상이 아니라 Body, Rim,
Highlight가 서로 다른 회전 평면을 사용한 결과였다. WModel의 실제 정점을 읽어 다음 범위를
확인했다.

```text
fm_h_swing_02.wmodel
vertex count 166
X span 140
Y span 약 0
Z span 140
```

따라서 이 Mesh는 처음부터 XZ 바닥 평면에 놓인 carrier이며 표면 법선 축은 Y다. 기존 시드는
Rotation/Revolution을 Z축에 넣어 바닥에서 도는 대신 Mesh 평면 자체를 뒤집었다. 다음처럼
교정했다.

```text
Position              (0.5, 0.15, -0.9)
Rotation              (0, -18, 0)
Revolution Start      (0, -280, 0)
Revolution End        (0, -70, 0)
```

Body, Rim, Highlight는 같은 Position/Rotation/Revolution으로 먼저 겹치고 Scale, Mask,
Color, Emissive만 분리한다. `Y=0.15`는 바닥과의 z-fighting만 피하는 시작값이다. 평면은
맞고 진행 방향만 반대라면 축을 다시 바꾸지 않고 Revolution Y의 부호만 반전한다.

교정 candidate SHA-256:

```text
4F3487A9249D50F7B14686DD721FB7BFF471773F535C2FE3585D48E7A979C8FE
```

검증:

| 검증 | 결과 |
|---|---|
| A candidate Python tests | PASS, 7 tests |
| checked-in candidate와 deterministic seed 동일성 | PASS |
| Effect Tool final audit | PASS |
| Client x64 Debug compile | 사용자 보고: 컴파일 오류 없음 |
| 교정된 Hit 01 GPU A/B | 사용자 재검증 대기 |

## 11. TCP listener 10049 진단

`10049`는 `WSAEADDRNOTAVAIL`이며 실행 시점의 로컬 인터페이스에 bind 요청 주소를 사용할 수
없다는 뜻이다. 포트 중복 오류 `10048`이나 Effect 문서 파싱 실패와는 다른 경계다.

당시 로컬 실측(개인 주소는 Git 계약에 따라 익명화):

```text
Wi-Fi   <LAN_IPV4>/24 Preferred
Server debugger argument  --bind-address <LAN_IPV4>
Client debugger env       LOSTARK_SERVER_HOST=<LAN_IPV4>
```

동일한 Debug Server를 다음 인자로 직접 실행한 smoke는 성공했다.

```text
Server.exe --bind-address <LAN_IPV4> --smoke-timeout-ms 1000
Listening on <LAN_IPV4>:7777 ...
exit 0
```

따라서 이번 실패는 Wi-Fi 이동 직후 지정한 주소가 아직 어댑터의 bind 가능한 Preferred 주소가
되기 전에 Server를 시작한 일시적 상태다. LAN 주소가 자주 바뀌는 환경에서는 Server를
`0.0.0.0`에 명시적으로 bind하고 Client host만 현재 사설 IPv4로 두거나, 같은 PC에서만
검증할 때는 양쪽 모두 `127.0.0.1`을 사용하는 편이 안정적이다. 개인 IP는 현재처럼
Git 제외 `.vcxproj.user`에만 둔다.

## 12. Cascade와 복원 과정 gotchas

### Cascade `Particle`은 화면 점의 동의어가 아니다

현재 문서의 Particle은 Cascade emitter/module 컨테이너다. 실제 화면의 큰 형상은 그 안의
Mesh renderer 48개와 Sprite renderer 52개가 만든다. `Particle 100`이라는 수치만 보고 큰
검격 Mesh가 없다고 판단하면 안 된다.

### Cascade 데이터 복원과 최종 픽셀 복원은 별도다

시간, Burst, Transform, Velocity, Lifetime, Mesh/Texture 연결은 상당 부분 복구됐다. 그러나
Parent Material graph, 중간 Material Instance 상속, 채널 조합, Blend/Depth/Cull/Two-sided,
Dynamic Parameter가 완전하지 않아 최종 픽셀 자동 복원은 실패했다. 회귀해 데이터를 잃은 것이
아니라 geometry/timeline과 GPU Material 의미의 복구율이 서로 다른 상태다.

### 흰 카드와 보라 판은 먼저 Scale 문제가 아니다

Mask, Opacity, Dissolve가 실행되지 않으면 원래 잘려야 할 carrier Quad/Mesh 전체 면이 보인다.
이때 전역 Scale을 먼저 줄이면 Material이 복구된 뒤 실제 형상이 작아지는 2차 회귀가 생긴다.
Base/Mask/Alpha와 render profile을 먼저 닫고 마지막에 Scale을 조정한다.

### WModel의 실제 평면과 회전축을 먼저 측정한다

파일명이나 UE 회전 벡터만으로 Engine 축을 추측하지 않는다. `fm_h_swing_02`처럼 XZ 평면인
Mesh는 Y축으로 회전해야 한다. Rotation은 초기 자세, Revolution은 초당 회전속도이며 둘 다
같은 Euler 회전에 합쳐진다. 축이 틀린 상태에서 부호나 속도만 반복 튜닝하면 답이 나오지 않는다.

### Emitter window와 화면상 Element lifetime을 구분한다

원본 `0.10초` emitter duration은 방출 가능한 시간이며 Body Mesh 수명 `0.50초`, Rim
`0.30초`와 다르다. 이를 수동 MESH Life Time으로 잘못 복사하면 검격이 순간적으로 사라진다.

### 한 검격은 단일 Mesh가 아니라 Element 그룹이다

A의 한 타격은 Body, 흰 Highlight/Core, 보라 Rim, 필요 시 Flow/Afterimage로 분리한다.
한 Mesh의 Bloom만 올리면 흰 중심과 짙은 외곽을 독립 조절할 수 없고 과노출 카드가 된다.
Hit 01 그룹을 먼저 완성한 뒤 같은 조립식을 `0.60/0.90/1.30초`로 복제한다.

### Screen Post와 Material Noise를 분리한다

A는 RGB Noise 9개와 Zoom Blur 4개의 Screen Post를 별도로 사용한다. 이를 Mesh Distortion
하나로 흉내 내면 형상이 깨진다. Shape/Material A/B는 Post OFF, 최종 합성만 Post ON으로
검증한다.

### Save 성공은 runtime 적용 성공이 아니다

Effect Detail live preview, Apply, Authored Save/Reload, Assembly/WFX build, Runtime Catalog
publish, 실제 A 키 재생은 각각 다른 단계다. 후보 문서 저장만으로 canonical A나 제품 runtime이
바뀌었다고 기록하지 않는다.

### Data Files와 All Effects의 책임을 섞지 않는다

Data Files는 저장 문서 catalog이고 All Effects는 현재 문서의 Hit 그룹과 Element를 고르는
outliner다. 카테고리 변경 때 모든 대형 JSON을 재파싱하거나 모든 Effect를 월드에 동시에
로드하지 않는다. 디스크 재스캔은 명시적 Refresh에서만 수행한다.

### 완성형 Skeletal Effect와 carrier Mesh를 구분한다

Dimension Summon은 Material을 추측해 만든 Cascade 성공 사례가 아니라 완성형 Skeletal
WModel+Animation을 재생한 사례다. 반면 A의 `fm_h_swing_02`는 Mask/Noise/Dissolve가 필요한
static carrier다. 두 자산을 같은 복원 전략으로 처리하면 안 된다.

### 후보를 canonical에 조기에 병합하지 않는다

Hit 01이 Solo GPU A/B를 통과하기 전에는 canonical A, Assembly/WFX, Runtime Catalog를
교체하지 않는다. candidate SHA와 stale-writer 보호를 유지하고 합격한 manual layer만 이후
결정적으로 병합한다.

## 13. 4연검격 승격 및 편집 UI 정리

Hit 01 GPU 확인에서 `body + rim + highlight`가 하나의 검격 실루엣으로 겹치는 것을 확인한 뒤,
동일한 조립층을 원본 Cascade event source의 네 시점으로 확장했다.

| Group | Elements | Start Delay | Position |
|---|---:|---:|---|
| `manual.a.hit01` | 4 | `0.25 s` | `[0.5, 0.15, -0.9]` |
| `manual.a.hit02` | 4 | `0.60 s` | `[0.5, 0.15, 0.8]` |
| `manual.a.hit03` | 4 | `0.90 s` | `[0.5, 0.30, -0.9]` |
| `manual.a.hit04` | 4 | `1.30 s` | `[0.5, 0.60, -0.8]` |

각 Group은 `body`, `rim`, `highlight`, `afterimage`로 구성한다. 앞의 세 Element는 기본 표시,
`afterimage`는 과노출과 형상 판정 방해를 피하기 위해 기본 숨김이다. Hit 01에서 검증된 XZ 평면과 Y축
Rotation/Revolution 계약은 네 Group에 동일하게 보존했다.

Data Files와 All Effects의 책임도 다음처럼 정리했다.

- `Load Document`: 선택한 저장 문서를 열고 완성 Effect를 처음부터 재생한다.
- `Unload Document`: 메모리의 작업 문서와 화면 Preview만 내린다. 저장 파일은 삭제하지 않는다.
- `Hide Preview`: 문서와 Detail 값을 유지하고 월드 표시만 숨긴다.
- `Play Complete Effect`: 현재 문서의 모든 표시 Element를 재생한다.
- `Play Group`: Hit 하나의 네 Element만 재생한다.
- `Solo`: 선택한 가장 작은 Element 하나만 재생하며 같은 행 선택은 Effect Detail 편집 대상으로 삼는다.
- Data Files에는 Effect Asset ID 필터를 추가했고, 항목 더블클릭도 Load로 처리한다.
- 생성/승격/Save As 같은 드문 명령은 `Advanced Document Commands`로 접었다.
- All Effects의 skill 행에는 `Load / Play Complete Skill`을 명시했다.

현재 candidate SHA-256은 다음과 같다.

```text
22925981482F1DF413F39A722BFA54A1F2E4514714D240D5F14A0BF2455CBBA6
```

검증 결과:

| 검증 | 결과 |
|---|---|
| A 4-hit deterministic seed | PASS, 7 tests |
| Effect Tool final audit | PASS |
| Client x64 Debug build | PASS, 오류 0 |
| ClientFrontendHarness build | PASS |
| ClientFrontendHarness run | PASS, failures 0 |

하네스는 정본 자동화와 동일하게 `LOSTARK_RESOURCE_ROOT=Client/Bin/Resources`를 설정해 실행했다.
이 환경 변수가 없으면 하네스 실행 파일 옆의 존재하지 않는 Resources를 기준으로 모든 Effect 문서를
resource-invalid로 오진하므로 실행 계약을 생략하면 안 된다.

아직 자동 PASS가 아닌 것은 실제 네 타격의 화면 진행, 각 타격의 방향 차이, Rim 폭/alpha,
흰 Core의 두께와 Afterimage 활성화 여부다. 이는 최신 Debug Client에서 Group Solo와 Complete Effect를
번갈아 눈으로 확인한 뒤 조정한다.

## 14. A 애니메이션 연결과 전방축 tilt 교정

Animation Authored 정본은 유실되지 않았다. `DimensionMaster.skillbindings.json`은 A `2050210`을
`pc_sp_m_00_sk_sk_willowrend`에 연결하고, `PlayerSkills.json`은 같은 ID를 `A | 분광`으로 소유한다.
Model View가 종전에는 모델의 raw clip 이름만 출력했기 때문에 정본이 사라진 것처럼 보였다.

Model View Animation Clip 목록은 이제 다음 표시 계약을 사용한다.

```text
[A] 분광 | pc_sp_m_00_sk_sk_willowrend
```

복수 clip 스킬은 `1/N`, `2/N` 순번을 함께 표시하고, `Reload Skill Labels`가 Authored binding과
PlayerSkills를 다시 읽는다. 복원 candidate도 canonical `2050210`을 찾아 자동으로 A 애니메이션을 시작하며,
`Restart + Play`, complete/group 재생은 이펙트와 바인딩 애니메이션을 함께 0초로 되돌린다.

바닥에 눕던 원인은 앵커가 아니라 회전축이었다. `fm_h_swing_02`는 XZ 평면 carrier인데 이전 후보는
Y축으로만 회전해 평면 normal을 유지했다. 원본 Body emitter의 Cooked module은 다음 값을 가진다.

```text
ParticleModuleMeshRotation      X = -0.05 turn = -18 degrees
ParticleModuleMeshRotationRate  X = -1.00 turn/second
```

네 event-source clone이 같은 X축 계약을 사용하므로 manual 근사값도 다음처럼 교정했다.

```text
Rotation Start       (-18, 0, 0)
Revolution Start     (-280, 0, 0)
Revolution End       (-70, 0, 0)
```

플레이어 root pivot은 Tool `Update`와 제품 `EffectPresentationService::Synchronize_FollowAnchors`에서
매 프레임 다시 계산된다. 따라서 캐릭터 이동과 facing 방향을 따라가며, 네 Hit의 위치는 원본 event-source
상대 위치와 시작 시각을 유지한다. 정확한 화면상 tilt와 pivot은 UE 축·WModel pivot·카메라 차이 때문에
최신 GPU 화면에서 Group Solo로 마지막 조정한다.

교정 candidate SHA-256:

```text
37F03D4C92676BE58E0EB7BA3FC61E80748AE9CB2CD4866E84029C61F9950740
```

추가 검증은 A seed 7 tests PASS, Effect Tool final audit PASS, Client x64 Debug build 오류 0이다.

## 15. Animation source-time 동기화

사용자 GPU 검증에서 Effect와 `pc_sp_m_00_sk_sk_willowrend`가 약간 어긋나는 현상을 확인했다.
원인은 두 경로가 같은 frame delta를 받아도 Animation에는 binding `playRate`가 적용되고,
Effect Tool의 preview clock은 보정되지 않은 wall-clock seconds로 증가하던 데 있다. 기존에는
Effect Timeline의 Pause와 Sample Time도 Animation model에는 전달되지 않았다.

Tool preview의 master clock을 현재 Animation track source time으로 교정했다.

```text
effectTime = currentAnimationTrackPosition / animationTicksPerSecond
```

- `playRate`는 Animation track의 진행 속도에 적용되므로 Effect `0.250초`도 같은 pose에서
  자동으로 빨라지거나 느려진다.
- Pause, Play, Restart, Loop, Sample Time이 Animation과 Effect에 함께 적용된다.
- 여러 binding clip은 앞 clip의 source duration 또는 `playMs`를 누적한다.
- Model View의 독립 Animation Play/Restart/Frame 컨트롤은 binding timeline이 소유하는 동안
  disabled되어 두 clock을 다시 분리하지 못하게 했다.
- A의 네 Group은 원본 animevents의 `0.250 / 0.600 / 0.900 / 1.300초`를 그대로 유지한다.

자동 검증은 `Test-EffectToolFinal.ps1` PASS, Client `ClCompile` PASS이며 실행 중인 기존
`Client.exe`를 건드리지 않는 별도 target name으로 full Client link도 PASS했다. 다만 실제
`Client.exe`는 수정 전 프로세스가 점유 중이므로 정본 이름으로 재링크하고 최신 GPU를 확인하는
작업은 Client 종료 후 다시 수행해야 한다.

## 16. TCP 10049 조사와 로컬 실행 계약

`10049`는 포트 사용 중 오류가 아니라 요청한 local bind address가 해당 시점의 NIC에 없다는
`WSAEADDRNOTAVAIL`이다. Wi-Fi 전환 뒤 특정 주소를 Server debugger argument에 고정한 것이
원인이었다.

- 당시 실측 주소: 사설 Wi-Fi `<LAN_IPV4>`, loopback `127.0.0.1`
- 포트 `7777` 점유 없음
- Debug Server를 기본 `127.0.0.1`과 명시적 `0.0.0.0`에서 각각 실행: 둘 다 exit 0
- 로컬 `.vcxproj.user`: Server `--bind-address 0.0.0.0`, Client
  `LOSTARK_SERVER_HOST=127.0.0.1`
- 두 `.user` 파일은 Git ignore 대상이며 shared source/data에는 개인 IP를 기록하지 않는다.

Server의 listener 실패 메시지에는 이후 실제 `Address=<address>:7777`도 출력한다. 이미 떠 있던
Client/Visual Studio debug child는 process start 때 읽은 이전 환경값을 유지하므로 새 설정을
검증하려면 Server와 Client를 모두 종료한 뒤 `Server + Client` launch profile을 다시 시작해야
한다. 다른 PC의 LAN Client를 붙일 때만 그 Client의 로컬 `LOSTARK_SERVER_HOST`를 현재 Server
LAN IP로 설정한다.
