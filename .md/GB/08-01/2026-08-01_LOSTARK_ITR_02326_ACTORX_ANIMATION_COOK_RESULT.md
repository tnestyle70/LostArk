# ITR_02326 ActorX 애니메이션 Cook 결과

- 작업일: 2026-08-01
- 브랜치: `CY`
- 대상: `DEPLOY_ITR_02326`
- 결론: 백룸 Hound·LostArk 창술사에서 검증한
  `PSK + PSA -> Blender -> FBX -> ModelAssetConverter -> WModel` 경로로
  애니메이션 포함 02326 런타임 모델을 만들고 반영했다.

## 1. 최종 결과

```text
PSK/PSA Bone       21 / 21, 이름과 순서 일치
Blender Actions    4: hit1_1, off, on, spawn
PSA/Blender FPS    30 / 30
WModel sections    7
WModel animations  4
WModel skeleton    yes
WModel FPS         전 clip 30 ticks/sec
Scale FCurve       Action마다 63개, 전체 FCurve 210개
off scale minimum  약 0.0001, WModel까지 원본과 일치
Runtime Intact     on
Runtime Fractured  off
```

기존 `animations=0` bind-pose WModel은 검증을 통과한 새 WModel로 교체했다.
숫자 animation index `0`은 더 이상 상태 계약으로 사용하지 않는다.

## 2. 원본 재추출

원본 패키지:

```text
logical  ITR_02326
physical C:\ProgramData\Smilegate\Games\LOSTARK\EFGame\ReleasePC\Packages\GL70PYCJC4DRYTKANAQH7K.upk
SHA-256  5EEC6D70E3EF73F1789D51521A29F4392D93911F75C504CCCAC5926FF3C69F57
```

재추출 위치:

```text
C:\LostArkExtract\ITR_02326_ActorX_20260801
```

| 파일 | 크기 | SHA-256 |
|---|---:|---|
| `itr_02326_sk.psk` | 176,748 | `3D1293EE7FFD992723BD6F28BEB60F94271C337C9B7BA96455A95AB81788D131` |
| `itr_02326_ani.psa` | 101,128 | `F8ACE00F6793761A1BA151FB63ADA6FA03829DCBD3A3301B3E4759E478949106` |

PSK는 2,806 vertices, 3,339 faces, 21 bones다. PSA에는 다음 네
sequence가 있고 전부 30fps다.

| Action | 원본 frame 수 | WModel duration ticks | WModel ticks/sec |
|---|---:|---:|---:|
| `off` | 62 | 61 | 30 |
| `on` | 2 | 1 | 30 |
| `spawn` | 31 | 30 | 30 |
| `hit1_1` | 2 | 1 | 30 |

## 3. 추가한 범용 도구

`Tools/ActorXAssetCooker/`를 추가했다.

- `build_actorx_fbx.py`
  - PSK와 하나 이상의 PSA를 Blender 3.0 headless에서 조립한다.
  - PSA `BONENAMES`, `ANIMINFO`, `ANIMKEYS`, `SCALEKEYS`를 직접 읽는다.
  - PSK와 PSA의 Bone 21개 이름·순서를 완전 일치 검증한다.
  - 구형 importer가 누락하는 Bone Scale FCurve 63개를 각 Action에 주입한다.
  - 원본 `AnimRate`를 보존한다.
  - 모든 Action과 원본 sequence를 1:1 검증한다.
  - Action 하나를 Armature에 할당하고 모든 Action을 FBX에 bake한다.
- `Cook-ActorXWModel.ps1`
  - Blender, 기존 ModelAssetConverter, `info`를 한 번에 실행한다.
  - 저장소와 Runtime Resources를 staging 대상으로 사용할 수 없다.
  - 관리되는 ActorX marker가 없는 기존 폴더는 덮어쓸 수 없다.
  - junction/symbolic link를 거부하고 최종 교체 직전에 marker를 다시 검사한다.
  - WMA2 9개 texture slot의 명시적 remap과 SHA-256 검증을 지원한다.
  - WModel의 WSKL/WANM/WMA2를 직접 읽어 Bone 순서, FPS, duration,
    Bone별 scale key 수·최소·최댓값, 전체 재질과 텍스처 SHA-256을 검증한다.
- `render_action_preview.py`
  - Action과 frame 목록을 입력받아 같은 카메라의 비교 PNG와 JSON을 만든다.
- `README.md`
  - 실행법, remap, 안전 규칙, preview 절차를 기록했다.

새 포맷이나 두 번째 런타임 로더는 만들지 않았다. 결과는 기존
`CModel -> CMaterial/CMesh/CBone/CAnimation` 경로로 로드된다.

## 4. 작업 중 발견하고 수정한 문제

### PSA SCALEKEYS 유실

검증에 사용한 구형 Blender ActorX importer는 PSA의 `ANIMKEYS` 위치·회전만
Action으로 만들고, 뒤의 `SCALEKEYS` 2,037개를 읽지 않았다. 이전 중간 FBX는
Action마다 147개 FCurve만 있었고, Converter가 만든 WModel scale key도 전부
약 `1.0`인 상수 bake 값이었다.

원본 PSA를 직접 해석해 다음을 확인하고 수정했다.

```text
BONENAMES  21
ANIMKEYS   2,037
SCALEKEYS  2,037
off        b_piece_00~14가 frame 57부터 1.0 -> 약 0.0001
on/spawn/hit1_1 scale 1.0 유지
```

PSA Scale Key의 네 번째 float는 이 파일에서 전부 `1.0`이므로 frame으로 쓰지
않고 `(FirstRawFrame + localFrame) * boneCount + boneIndex`로 원본 key를 찾는다.
scale은 무차원 값이므로 `bScaleDown`도 적용하지 않는다. 새 Action은 각각
`21 bones * (location 3 + rotation 4 + scale 3) = 210 FCurve`를 가진다.

### Blender 기본 24fps

처음 FBX는 Blender 기본 24fps를 사용해 `off`가 2.542초,
`spawn`이 1.25초로 원본보다 25% 느렸다. PSA의 `AnimRate=30`을 직접 읽어
Blender effective FPS와 WModel `ticksPerSecond`가 모두 30인지 실패 차단
검사를 추가한 뒤 다시 Cook했다.

### 자동 texture-root 오매칭

처음 자동 탐색은 첫 재질의 normal을 forest normal로 잘못 골랐고 두 번째
재질의 일부 슬롯을 비웠다. 원본 Material Instance `.props.txt`를 근거로
재질별 exact remap을 사용했다.

| 재질 | Base | Normal | Emissive |
|---|---|---|---|
| `itr_02326_01_mi` | `diffuse.tga` | `bg_rad_end_crystal01_n_pcs.tga` | `bg_rad_end_crystal01_em_pcs_01.tga` |
| `itr_02326_02_mi` | `diffuse.tga` | `bg_pap_sforest_floor01c_n_ksr.tga` | `bg_rad_end_crystal01_em_pcs_01.tga` |

각 remap은 Cook 후 WMA2 entry와 실제 복사 파일의 SHA-256까지 대조한다.

## 5. 최종 staging 산출물

```text
C:\LostArkExtract\ITR_02326_ActorX_20260801\CookedScale30\DEPLOY_ITR_02326_INTACT
```

| 파일 | 크기 | SHA-256 |
|---|---:|---|
| `DEPLOY_ITR_02326_INTACT.fbx` | 773,244 | `029AA7748DE498810503C93FC7C64D8651AB8F11D17A4A1F7D1F53654ED96079` |
| `DEPLOY_ITR_02326_INTACT.wmodel` | 786,946 | `07ABEF6D23A684FD0CE015CD6028E0437BFA0F620B50FC095D2AB30EBF408747` |

WModel animation 이름:

```text
itr_02326_sk.ao_hit1_1
itr_02326_sk.ao_off
itr_02326_sk.ao_on
itr_02326_sk.ao_spawn
```

## 6. 시각 판정

최종 30fps `.blend`에서 네 Action의 대표 frame을 모두 고정 카메라로
렌더했다. `off`는 0, 55, 56, 57, 61, `on`은 0, 1, `spawn`은 0, 15,
30, `hit1_1`은 0, 1 frame이다.

```text
C:\LostArkExtract\ITR_02326_ActorX_20260801\CookedScale30\DEPLOY_ITR_02326_INTACT\Preview
```

- frame 0: 원형
- frame 55: 원형 유지
- frame 56: 원형 유지, scale 약 1.0
- frame 57: 15개 조각 Bone이 약 0.0001로 축소되어 사라짐
- frame 61: 축소 상태 유지

따라서 실제 파괴 Action은 `hit1_1`이 아니라 `off`다. `on`은 정상 상태에
사용한다. `spawn`은 별도 SPAWNING 상태/이벤트가 아직 없으므로 런타임 상태에
임의 연결하지 않았다.

## 7. 런타임 반영

반영 경로:

```text
Client\Bin\Resources\LostArk\Deploy\LV_LUT_HEARTRB_ED\DEPLOY_ITR_02326\intact
```

기존 폴더 전체 백업:

```text
C:\LostArkExtract\ITR_02326_ActorX_20260801\RuntimeBackup_PreAnimated30\intact
```

새 WModel과 TGA 4개를 반영하고 staging과 runtime SHA-256이 모두 같은지
재검증했다. 최종 runtime WModel SHA-256은
`07ABEF6D23A684FD0CE015CD6028E0437BFA0F620B50FC095D2AB30EBF408747`이다.
기존 DDS 파일은 삭제하지 않았다. Scale Key 수정 직전 런타임도 아래에 추가로
백업했다.

```text
C:\LostArkExtract\ITR_02326_ActorX_20260801\RuntimeBackup_PreScaleKeyFix\intact
```

`Client/Bin/Resources`는 `.gitignore` 대상이므로 팀원에게는 위
CookedScale30 패키지를 Drive로 전달해야 한다.

## 8. 런타임 코드 변경

`Client/Private/DeployPropObject.cpp`만 변경했다.

- `Set_Animation(0u)` 제거
- WModel의 접두어가 붙은 이름에서도 논리 이름 `on`, `off`를 찾는 helper 추가
- 같은 논리 suffix가 둘 이상이면 모호한 선택을 하지 않고 실패
- 초기 Intact 상태는 `on` 선택
- Fractured 전환은 `off` 선택
- 전환할 때 해당 animation의 track을 0으로 되감음
- `Play_Animation(0.f)`로 같은 frame에 Bone 행렬을 즉시 갱신
- 상태 전환용 Action 선택이 실패하면 객체 상태도 바꾸지 않음
- `spawn`과 `hit1_1`은 임의 연결하지 않음

Engine 공개 API는 변경하지 않았다.

## 9. 검증 결과

- Python 구문 검사: 통과
- PowerShell parser 검사: 통과
- Blender 3.0.1 PSK/PSA 실제 조립: 통과
- Action 4개, Bone 21개, weighted vertex 2,806: 통과
- PSA와 PSK Bone 이름·순서 21개 완전 일치: 통과
- Action마다 Scale FCurve 63개, 전체 FCurve 210개: 통과
- ModelAssetConverter Cook/info: 통과
- WModel WANM 4개 모두 `ticksPerSecond=30`: 통과
- WModel duration과 Action frame span: 통과
- WModel 21개 PSA Bone의 scale key 수·최소·최댓값 원본 대조: 통과
- `off` WModel scale minimum 약 `0.0001`, non-unit component 225개: 통과
- `on`, `spawn`, `hit1_1` WModel scale 약 `1.0`: 통과
- 최종 안전 검사 보강 후 별도 staging 재Cook SHA-256 재현: 통과
- WMA2 재질 2개와 exact remap SHA-256: 통과
- Preview PNG 12개와 JSON 4개 생성 및 육안 비교: 통과
- Client x64 Debug 빌드: 성공
- Client x64 Release 빌드: 성공

Debug는 경고 0개·오류 0개였다. Release는 기존 C4819 5개와 LNK4099
9개가 있었고 오류는 0개였다. `DeployPropObject.cpp`에서 생긴 새 경고는 없다.

## 10. 남은 수동 실행 확인

기존 `--effect-open` Debug `Client.exe`는 사용자가 종료할 때까지 임의 종료하지
않았다. 종료된 것을 확인한 뒤 최종 WModel을 반영했으며, 아래 MapTool 화면
상태 전환 검증은 수동 확인 항목으로 남긴다.

```text
1. 실행 중인 Client/Visual Studio 디버깅을 정상 종료
2. Client\Bin\Debug\Client.exe 실행
3. F2 -> Loader 완료 -> Enter -> F1
4. MapTool의 Reload
5. Gameplay DeployProps가 85개인지 확인
6. bind-pose-only 수가 0인지 확인
7. Intact 선택: 02326이 정상 형태인지 확인
8. Fractured 선택: 약 1.87초 유지 후 파괴가 시작되는지 확인
9. Intact 재선택: `on` 0 frame으로 정상 복귀하는지 확인
10. Reload와 레벨 재진입 후 동일한지 확인
```

## 11. 복구 방법

문제가 있으면 게임을 종료한 뒤 백업의 `intact` 내용을 런타임 `intact`
폴더로 다시 복사한다. 소스 코드는 `DeployPropObject.cpp`의 논리 이름 연결
변경만 되돌리면 된다. `git reset --hard`나 다른 팀원의 작업 파일 정리는 하지
않는다.
