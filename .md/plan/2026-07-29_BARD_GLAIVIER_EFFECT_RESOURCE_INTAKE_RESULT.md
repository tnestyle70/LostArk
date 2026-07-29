# 바드·창술사 Effect Resource 1차 인입 결과

작성일: 2026-07-29  
대상 저장소: `LostArk`  
상태: 원본 패키지 보존, ParticleSystem 카탈로그 생성, 커서 텍스처 추출 완료

## 1. 이번 작업의 범위

다음 기능을 만들기 위한 원본 후보를 설치된 한국 Lost Ark 클라이언트에서
찾고, 현재 팀 프레임워크의 로컬 리소스 영역으로 가져왔다.

- 바드 스킬 이펙트
- 창술사 스킬 이펙트
- 걷기와 달리기의 지면별 발걸음 이펙트
- 클래스 대시 이펙트
- 마우스 커서 및 바닥 클릭·이동 표시 후보

원본 게임 전체 리소스를 복사하지 않았다. 확인된 전용 FX와 공용 기능
패키지만 Manifest 기반으로 선별했다.

## 2. 생성 위치

실제 리소스는 다음 경로에 생성됐다.

```text
Client/Bin/Resources/LostArk/Effect/
├─ SourceRaw/          원본 UPK 스냅샷
├─ SourceCatalog/      Manifest, 전체 ParticleSystem 목록, 후보 CSV
└─ SourceExtracted/    UModel로 실제 추출한 PNG와 DDS
```

`Client/Bin/Resources`는 저장소 정책상 Git에서 제외된다. 원본 게임
리소스를 Git에 올리지 않고 기존 팀 리소스 공유 드라이브로 전달해야 한다.

다시 생성하는 도구는 다음 위치에 있다.

```text
Tools/EffectResourceIntake/Build-EffectSourceCatalog.ps1
```

## 3. 확인 및 인입 결과

| 구분 | 원본 패키지 | ParticleSystem |
|---|---:|---:|
| 바드 `FX_PC_MBD_00~08` | 9개 | 689개 |
| 창술사 `FX_PC_FLM_00~10` | 11개 | 754개 |
| 공용 `FX_BS_03`, `04`, `06` | 3개 | 790개 |
| 커서 `EFUI_CURSOR`, `EFUI_CURSOREFFECT` | 2개 | 커스텀 CursorData/SWF |
| 창술사 Trail | 1개 | 커스텀 TrailData |

총 26개 원본 패키지, 약 15.33 MiB를 선별 복사했다. 복사한 26개
모두 Manifest의 SHA-256과 다시 비교했으며 불일치가 없었다.

바드와 창술사의 애니메이션, 무기, 사운드, Skill Camera 패키지는
물리 파일 위치와 해시만 Manifest에 기록했다. 크기가 큰 공용 캐릭터
패키지까지 무조건 복사하지 않도록 원본 복사는 보류했다.

## 4. 기능별 1차 후보

### 4-1. 바드 대시

```text
FX_PC_MBD_05.par_s_mbd_exmove_01
FX_PC_MBD_05.par_s_mbd_exmove_02
```

동기화 참고 대상:

```text
PC_MBD_00.sk_moving_normal_1
PC_BARD_F.pc_bard_f_avata_dash1_1
```

### 4-2. 창술사 대시

```text
FX_PC_FLM_02.par_d_flm_dash_01
FX_PC_FLM_03.par_n_flm_dash_002
FX_PC_FLM_03.par_n_flm_dash_01
FX_PC_FLM_03.par_n_flm_dash_02
FX_PC_FLM_03.par_n_flm_dash_03
FX_PC_FLM_03.par_n_flm_dashup_01
FX_PC_FLM_03.par_n_flm_shockdash_01
FX_PC_FLM_03.par_n_flm_shockdash_02
FX_PC_FLM_03.par_n_flm_shockdash_04
FX_PC_FLM_05.par_d_flm_moving_01
```

이 목록에는 일반 이동, 스킬 돌진, 충격 대시 변형이 함께 들어 있다.
프리뷰 비교 후 실제 기본 대시 하나와 스킬용 돌진을 분리해야 한다.

### 4-3. 공용 발걸음

```text
FX_BS_06.par_d_footstepdefault_001
FX_BS_06.par_g_footstep_light_01
FX_BS_06.par_g_footstep_light_grass_01
FX_BS_06.par_g_footstep_light_mud_001
FX_BS_06.par_g_footstep_light_snow_001
FX_BS_06.par_s_footstepwater_001
```

바드와 창술사마다 발걸음 시스템을 복제하지 않는다. 엔진 공용
`Footstep Emitter`가 좌·우 발 AnimNotify와 지면 재질을 입력받아
Default, Grass, Mud, Snow, Water 프리셋을 고르는 구조가 적합하다.

### 4-4. 마우스 포인터

커서 자체와 월드 바닥 피드백을 분리했다.

커서 자체:

```text
EFUI_CURSOR
EFUI_CURSOREFFECT.cursoreffect_i5
```

`cursoreffect_i5`는 256×128 PNG와 DDS로 실제 추출했다.

월드 바닥 클릭·이동 표시 후보:

```text
FX_BS_03.par_b_select_01
FX_BS_03.par_b_select_02_loc_int
FX_BS_03.par_g_movetrack_01
FX_BS_03.par_g_movetrack_02
FX_BS_03.par_i_movetrack_01
FX_BS_03.par_i_movetrack_02
FX_BS_03.par_l_movetrack_01
```

## 5. 현재 자동으로 가져올 수 없는 것

현재 Lost Ark용 UModel은 `ParticleSystem` 이름과 패키지 내부 객체를
열거할 수 있지만 Lost Ark 커스텀 Cascade 그래프를 역직렬화하지 못한다.

따라서 이번 인입으로 다음 항목이 자동 복원된 것은 아니다.

- 스킬 한 번에 실행되는 ParticleSystem의 정확한 순서
- Emitter와 Module의 원본 연결
- Spawn, Burst, Lifetime, Curve의 원본 값
- ParticleSystem별 Mesh, Material, Texture의 정확한 의존 관계
- 원본 셰이더 그래프
- 창술사 Trail의 폭, 소켓, 시간, 머티리얼 값

카탈로그의 689개와 754개는 완성 스킬의 개수가 아니라 파트와 변형을
포함한 원본 객체 이름 수다.

## 6. 다음 작업 순서

1. 공용 발걸음 프리셋 한 개를 먼저 재구성해 현재 Effect Runtime을 검증한다.
2. 바드 기본 대시와 창술사 기본 대시를 각각 한 개씩 확정한다.
3. 마우스는 UI 커서와 월드 클릭 피드백을 별도 Effect Asset으로 만든다.
4. 바드 대표 스킬 한 개와 창술사 대표 스킬 한 개를 정한다.
5. 선택한 ParticleSystem만 Material → Mesh → Texture 순으로 의존성을 좁힌다.
6. 추출 가능한 원본은 Asset Converter 입력으로 보내고, 복원할 수 없는
   Cascade 값은 새 Effect Tool의 `reconstructed` 데이터로 작성한다.
7. 검증이 끝난 형식만 나머지 스킬에 반복한다.

전량을 한 번에 변환하지 않는 이유는 이름 후보와 실제 런타임 호출을
구분해야 하고, 현재 Converter와 Effect Runtime이 Cascade의 모든
Vertex/Material/Module 형식을 아직 지원하지 않기 때문이다.
