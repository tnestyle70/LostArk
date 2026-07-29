# Lost Ark HeartRB 원본 레벨 Placement 복구 계획

## 1. C1~C8 관점

### C1. 목표

`LV_LUT_HEARTRB_ED_PS`, `SL00~SL05`가 실제로 배치한 StaticMesh의
Actor/Component 연결과 원본 Transform을 재현 가능한 JSON으로 복구한다.

### C2. 현재 문제

ImportTable은 “이 레벨이 어떤 외부 에셋을 참조하는가”만 알려 준다. 에셋의
개수, 위치, 회전, 스케일은 ExportTable의 Actor와 Component 직렬화 데이터에
있으므로 기존 254개 StaticMesh import 집합만으로는 맵을 자동 조립할 수 없다.

### C3. 입력

- Lost Ark KR 원본 UPK
- `umodel_lostark_v7.exe`
- 논리 레벨명 `LV_LUT_HEARTRB_ED_PS`, `SL00~SL05`

### C4. 출력

- 레벨별 `*.placements.json`
- 전체 수량을 검증하는 `placement_manifest.json`
- 각 placement의 안정적인 ID, 원본 asset path, Actor/Component raw transform,
  MapTool용 정규화 transform

### C5. 불변식

- 원본 UPK와 베이킹 결과물을 수정하지 않는다.
- 좌표는 추출 단계에서 `UE3-native`로 보존한다.
- 이름 추측이 아니라 Import/Export object reference로 StaticMesh를 연결한다.
- 파싱 실패를 누락으로 숨기지 않고 `propertyErrors`에 기록한다.

### C6. 팀 작업 경계

다른 작업이 생성 중인 `.wmodel`, `.wmat`, texture, `.mapassets`는 수정하지
않는다. 이 작업은 별도 `placements` report와 독립 추출기만 소유한다.

### C7. 완료 조건

- 7개 레벨 전부 파싱 성공
- 모든 StaticMeshComponent placement에 asset path와 transform 존재
- tagged property 오류 0개
- 중앙 링 `bg_pvp_retown_floor01_sm`의 실제 placement가 결과에 존재

### C8. 후속 적용

다음 작업에서 MapTool importer가 placement JSON을 `parse -> validate -> stage
-> commit`으로 읽고, 한 지점에서만 UE3 좌표계를 Client 좌표계로 바꾼다.

## 2. 문제 해결 ①~⑤

### ① 논리 패키지명 해석

UModel `-list -nameresolve` 출력에서 실제 난독화 UPK 파일명을 얻는다.

### ② Lost Ark UPK 복원

UE3 package summary 뒤의 Lost Ark 20-byte chunk descriptor를 읽는다. 암호화된
chunk는 각 LZ4 block 앞 4096바이트를 AES-256 ECB로 해제한 뒤 복원한다.

### ③ UE3 테이블 파싱

NameTable, 28-byte ImportTable entry, 가변 길이 ExportTable entry를 파싱하고
package index의 음수(import)/양수(export) 규칙으로 전체 object path를 만든다.

### ④ Actor/Component 속성 복구

UnrealScript stack frame 뒤에서 유효한 tagged-property 시작을 찾고 `None`
terminator까지 읽는다. Lost Ark 전용 `IntProperty` 8-byte descriptor도 반영한다.

### ⑤ Placement 생성 및 검증

일반 StaticMeshActor는 Actor transform을, StaticMeshCollectionActor는 인스턴스별
Component transform을 정규화 결과로 선택한다. 원시값도 함께 보존한다.

## 3. 자료구조·알고리즘 핵심

```text
Logical level name
  -> UModel name resolver
  -> obfuscated physical UPK
  -> PackageSummary + 20-byte chunk descriptors
  -> AES-256 ECB(first 4096 bytes/block) + LZ4
  -> NameTable / ImportTable / ExportTable
  -> Actor export -> Component export -> StaticMesh import
  -> placementId + asset path + UE3-native transform
```

`placementId`는 `<level>:export:<component-export-index>` 형식이다. vector index나
런타임 포인터를 저장 ID로 쓰지 않는다.

## 4. 추가·수정·삭제 파일 목록

### 추가

- `Tools/LevelPlacementExtractor/extract_ue3_placements.py`
- `Tools/LevelPlacementExtractor/README.md`
- `.md/GB/07-30/2026-07-30_LOSTARK_LEVEL_PLACEMENT_RECOVERY_PLAN.md`
- `.md/GB/07-30/2026-07-30_LOSTARK_LEVEL_PLACEMENT_RECOVERY_RESULT.md`

### 수정

- 없음

### 삭제

- 없음

## 5. 파일별 전체 구현 코드

신규 실행 코드의 전체 정본은
`Tools/LevelPlacementExtractor/extract_ue3_placements.py`다. 이 파일에 다음을
모두 포함하며 외부 비공개 모듈이나 별도 런타임 구현은 없다.

- Windows CNG AES-256 ECB wrapper
- Lost Ark LZ4 block decoder
- PackageSummary/NameTable/ImportTable/ExportTable parser
- UE3 tagged-property parser
- Actor/Component/StaticMesh join
- JSON/manifest writer

명령과 저장 계약의 전체 내용은 `Tools/LevelPlacementExtractor/README.md`를
정본으로 사용한다. 두 파일 모두 이번 계획 범위에서 새로 작성되어 일부 코드나
의사 코드로 대체되지 않는다.

## 6. 프로젝트 등록과 검증

독립 Python 도구이므로 `.vcxproj` 등록은 없다.

```powershell
python -m py_compile Tools\LevelPlacementExtractor\extract_ue3_placements.py
python Tools\LevelPlacementExtractor\extract_ue3_placements.py `
  --umodel C:\path\to\umodel_lostark_v7.exe `
  --package-root C:\ProgramData\Smilegate\Games\LOSTARK\EFGame\ReleasePC\Packages `
  --output C:\path\to\placements `
  LV_LUT_HEARTRB_ED_PS LV_LUT_HEARTRB_ED_SL00 LV_LUT_HEARTRB_ED_SL01 `
  LV_LUT_HEARTRB_ED_SL02 LV_LUT_HEARTRB_ED_SL03 LV_LUT_HEARTRB_ED_SL04 `
  LV_LUT_HEARTRB_ED_SL05
```

검증은 manifest의 7개 package, 모든 package의 `propertyErrorCount == 0`, 중앙
링 asset path 존재 여부를 함께 확인한다.
