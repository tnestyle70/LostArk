# ITR_02326 ActorX 애니메이션 Cook 계획

- 작성일: 2026-08-01
- 작업 브랜치: `CY`
- 대상: `DEPLOY_ITR_02326`
- 목표: 원본 PSK/PSA를 검증된 Blender 경로로 결합하고 기존 `ModelAssetConverter`를 통해 애니메이션 포함 `.wmodel`을 생성한다.

## 1. 현재 상태

- 현재 런타임 WModel은 `skeleton=yes`, `animations=0`, embedded bone 22개다.
- 원본 AnimSet PSA에는 21개 track bone과 `off`, `on`, `spawn`, `hit1_1` 네 clip이 있다.
- 현재 `ModelAssetConverter.exe`는 `pack`, `material`, `info`를 지원하지만 PSK/PSA를 직접 읽지는 않는다.
- 백룸 Hound와 LostArk 창술사에서 `PSK + PSA -> Blender -> FBX -> ModelAssetConverter -> WModel` 경로가 성공했다.
- 작업 트리에는 베른성·Effect 관련 사용자 변경이 존재한다. 이 작업은 해당 파일을 수정하거나 정리하지 않는다.

## 2. 작업 경계

### 수정·추가 대상

- `Tools/ActorXAssetCooker/`의 범용 Blender/Powershell 도구와 README
- 검증 후 필요한 최소 `Client/Private/DeployPropObject.cpp` 변경
- 본 계획과 대응 RESULT 문서

### 저장소 밖 staging

- `C:/LostArkExtract/ITR_02326_ActorX_20260801/`
- PSK/PSA/FBX/중간 WModel/리포트는 staging에서만 생성한다.

### 수정하지 않는 대상

- 기존 베른성·Effect Tool 작업 파일
- 검증 전의 `Client/Bin/Resources/LostArk/Deploy/.../DEPLOY_ITR_02326_INTACT.wmodel`
- `Engine` 공용 API. 현재 `CModel`의 공개 조회·선택 API만으로 해결 가능하면 확장하지 않는다.

## 3. 단계와 완료 조건

### G0. 기준점과 진단

- 기존 WModel의 크기·해시·section·bone·material·bounds를 기록한다.
- animation 0과 CModel 생성 실패를 같은 원인으로 단정하지 않는다.
- 실패 시 `MODEL_DECODE_REPORT.error`를 확인할 수 있는 최소 진단 경로를 마련한다.

### G1. 원본 재추출

- UModel LostArk v7에서 exact `ITR_02326` SkeletalMesh를 `-psk`로 추출한다.
- exact `ITR_02326_Ani` PSA를 함께 확보한다.
- PSK/PSA bone 이름·순서, clip 수, FPS, frame 범위와 `SCALEKEYS` 수를 검증한다.

### G2. ActorX 조립

- Blender 3.0 headless와 기존 PSA/PSK importer를 사용한다.
- 모든 PSA sequence를 Action으로 만들고 Armature에 하나를 연결해 FBX take 누락을 방지한다.
- 구형 importer가 누락하는 PSA `SCALEKEYS`를 Bone Scale FCurve로 직접 주입한다.
- `--pretransform`은 사용하지 않는다.
- `off`, `on`, `spawn`, `hit1_1`의 이름과 frame 범위를 보존한다.

### G3. 시각 판정

- 네 Action을 각각 preview 또는 대표 frame으로 확인한다.
- 이름만으로 파괴 clip을 추측하지 않는다.
- 파괴/등장/활성/비활성 의미를 시각 근거와 원본 Trigger 근거로 구분한다.

### G4. WModel Cook

- Blender FBX를 기존 `ModelAssetConverter.exe`로 staging WModel에 변환한다.
- 예상 결과는 `sections=7`, `skeleton=yes`, `animations=4`다.
- 기존 bind-pose WModel과 bone, material, texture, bounds, scale을 비교한다.
- WSKL Bone과 WANM 채널을 연결해 PSA Bone별 scale key 수와 최소·최댓값을
  원본 PSA와 비교한다. 단순 key 수나 상수 bake key를 완료로 인정하지 않는다.
- 모든 검증을 통과하기 전 런타임 파일을 교체하지 않는다.

### G5. 런타임 연결

- `CDeployPropObject`는 animation index 0을 저장 계약으로 사용하지 않는다.
- 확인된 clip 이름을 검색해 선택하고 해당 index의 track position을 0으로 되돌린다.
- Trigger/Matinee 자동 타이밍이 확인되지 않은 동안 MapTool 상태 전환은 debug selector로 유지한다.

### G6. 빌드·실행

```text
1. 변경 Python/Powershell syntax 및 dry-run
2. ModelAssetConverter info 검증
3. Engine x64 Debug/Release (Engine 변경 시에만)
4. UpdateLib.bat Debug/Release (Engine public 변경 시에만)
5. Client x64 Debug/Release
6. F2 -> loader 완료 -> Enter -> F1
7. DeployProps 85개와 네 clip preview/state 전환 확인
8. 재진입과 실패 시 기존 상태 보존 확인
```

## 4. 협업·배포 규칙

- 도구·문서·최소 C++ 변경만 Git/PR에 포함한다.
- 원본 게임 리소스, PSK/PSA/FBX/WModel, Blender `.blend`는 Git에 올리지 않고 팀 Drive 팩으로 배포한다.
- runtime 교체본은 기존 상대경로를 유지한다.
- 실패한 중간 산출물은 runtime root로 복사하지 않는다.

## 5. 실행 상태

- G0 기준점 기록: 완료
- G1 exact PSK/PSA 재추출 및 Bone/clip/FPS/Scale Key 검증: 완료
- G2 Blender ActorX 조립, Scale FCurve 주입과 30fps 보존: 완료
- G3 네 Action 대표 frame PNG 12개 시각 판정: 완료
- G4 WModel 7 sections/4 animations/전체 재질/Bone별 scale 값 검증: 완료
- G5 런타임 리소스 반영 및 이름 기반 `on`/`off` 연결: 완료
- G6 Client x64 Debug/Release 빌드: 완료
- G6 실제 MapTool Intact/Fractured 화면 확인: 최종 리소스 반영 후 사용자 수동
  확인 항목으로 남김. 절차는 RESULT에 기록했다.
