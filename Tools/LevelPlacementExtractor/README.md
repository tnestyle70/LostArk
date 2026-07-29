# Lost Ark UE3 Level Placement Extractor

Lost Ark 레벨 패키지에서 `StaticMeshActor / InterpActor /
StaticMeshCollectionActor -> StaticMeshComponent -> StaticMesh` 연결과 원본
Transform을 JSON으로 복구하는 읽기 전용 도구다.

## 왜 필요한가

UModel의 일반 메시 export는 메시 파일 자체만 내보낸다. 레벨이 어떤 메시를
몇 개 만들었고 어디에 배치했는지는 레벨 UPK의 ExportTable과 각 export의
UE3 tagged property 안에 별도로 저장된다. 이 도구는 다음 순서로 그 배치표를
복구한다.

1. Lost Ark 전용 UModel의 `-nameresolve`로 논리 패키지명을 실제 난독화 UPK에 연결한다.
2. UPK summary의 Lost Ark 20-byte chunk table을 읽는다.
3. 각 chunk의 앞 4096바이트를 AES-256 ECB로 해제하고 LZ4 block을 복원한다.
4. NameTable, ImportTable, ExportTable을 UE3 규칙으로 파싱한다.
5. Actor와 Component의 tagged property를 읽고 StaticMesh import를 연결한다.
6. MapTool이 소비할 수 있는 안정적인 `placementId`, asset path, UE3-native Transform을 기록한다.

## 실행

```powershell
python Tools\LevelPlacementExtractor\extract_ue3_placements.py `
  --umodel C:\path\to\umodel_lostark_v7.exe `
  --package-root C:\ProgramData\Smilegate\Games\LOSTARK\EFGame\ReleasePC\Packages `
  --output C:\path\to\placements `
  LV_LUT_HEARTRB_ED_PS `
  LV_LUT_HEARTRB_ED_SL00 `
  LV_LUT_HEARTRB_ED_SL01 `
  LV_LUT_HEARTRB_ED_SL02 `
  LV_LUT_HEARTRB_ED_SL03 `
  LV_LUT_HEARTRB_ED_SL04 `
  LV_LUT_HEARTRB_ED_SL05
```

출력은 레벨별 `*.placements.json`과 전체 집계
`placement_manifest.json`이다. 입력 UPK를 수정하거나 중간 복호화 패키지를
디스크에 만들지 않는다.

## Transform 계약

- `actor`와 `component`에는 추출한 원시 속성을 모두 보존한다.
- `transform`은 바로 배치에 사용할 정규화 결과다.
- 일반 `StaticMeshActor / InterpActor`는 Actor의
  `Location / Rotation / DrawScale / DrawScale3D`가 정본이다.
- `StaticMeshCollectionActor`는 Actor가 identity이고 Component의
  `Translation / Rotation / Scale / Scale3D`가 인스턴스별 정본이다.
- 좌표는 아직 `UE3-native`다. Client 좌표계 전환은 MapTool importer의 한
  지점에서만 적용해야 하며 추출 JSON 자체를 파괴적으로 변환하지 않는다.
- Rotator 원시값과 degree 값을 함께 저장한다. 한 바퀴는 65536 units다.

## Lost Ark 전용 주의점

- compressed chunk descriptor는 표준 UE3의 16바이트가 아니라 마지막
  `encrypted_lz4` 필드가 붙은 20바이트다.
- 현재 KR level package의 compression flags는 `0x44`이며 실제 payload는
  AES 해제 뒤 LZ4로 푼다.
- `IntProperty`는 선언된 4바이트 payload 앞에 Lost Ark 전용 8바이트
  descriptor가 추가된다.
- Actor export에는 UnrealScript stack frame이 있을 수 있어 tagged property의
  시작 위치가 고정 4바이트가 아니다. 파서는 유효한 Property FName 쌍과
  `None` terminator를 함께 검증한다.
- 에셋 import 목록만으로는 배치 복구가 되지 않는다. 반드시 ExportTable의
  Actor와 Component 직렬화 데이터를 읽어야 한다.

## HeartRB 실증과 이중 검토

2026-07-30 `PS + SL00~SL05`를 원본 package에서 두 번 독립 실행했다.

```text
placement                         13,091
unique StaticMesh object path        260
duplicate placement ID                 0
asset manifest join missing             0
owner Actor missing                      0
property errors                          0
non-finite Transform                     0
zero scale                               0
negative scale                       5,042
```

기존 결과와 재실행 결과의 placement JSON 7개 및 manifest 1개는 SHA-256이 전부
일치했다. 전수 감사 receipt는 다음 파일이다.

```text
C:/Users/user/Desktop/Resource_LostArk/05_Reports/MapExtraction/
  LV_LUT_HEARTRB_ED/placements/placement_audit.json
```

`12,949`는 streaming level `SL00~SL05`만의 Component 수이고, `13,091`은
persistent level `PS`의 142개를 더한 값이다.

## 아직 MapTool에 바로 commit하지 않는 이유

이 JSON의 판결은 `UE3-native placement PASS`다. Client map reconstruction 완료가
아니다.

- Client의 axis/handedness mapping은 중앙 Floor01과 두 번째 비대칭 anchor를 동시에
  맞춰 확정한다.
- 원본 placement 5,042개에는 음수 scale이 있다. 현재 MapTool의 양수-only validator로
  버리지 말고 reflection/winding/culling을 검증한다.
- property 오류 0은 tagged property를 읽었다는 뜻이다. hidden, collision-only,
  per-actor material override, Terrain, Particle, DeployData, navigation까지 끝났다는 뜻이 아니다.

전체 공통 순서와 모든 함정은 다음 두 정본에 기록한다.

```text
C:/Users/user/Desktop/Final_LostArk/Tools/AssetPipeline/맵추출파이프라인.md
C:/Users/user/Desktop/Final_LostArk/Tools/AssetPipeline/gotchas.md
```
