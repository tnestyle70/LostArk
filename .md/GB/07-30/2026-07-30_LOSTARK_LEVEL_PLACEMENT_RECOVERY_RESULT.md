# Lost Ark HeartRB 원본 레벨 Placement 복구 결과

## 결론

`LV_LUT_HEARTRB_ED_PS + SL00~SL05`의 StaticMesh placement를 원본 UPK에서
복구했다. 수동 추정 배치가 아니라 UE3 ExportTable의 Actor/Component 속성과
ImportTable의 StaticMesh object reference를 직접 연결한 결과다.

## 최종 수치

| 레벨 | placement | 고유 StaticMesh path | property 오류 |
|---|---:|---:|---:|
| PS | 142 | 6 | 0 |
| SL00 | 682 | 86 | 0 |
| SL01 | 3,603 | 142 | 0 |
| SL02 | 2,094 | 122 | 0 |
| SL03 | 2,543 | 130 | 0 |
| SL04 | 2,324 | 81 | 0 |
| SL05 | 1,703 | 142 | 0 |
| 전체 | 13,091 | 260(전체 object path 합집합) | 0 |

기존 254는 “고유 StaticMesh 이름” 기준이고, 이번 260은 패키지까지 포함한
고유 object path 기준이므로 집계 기준이 다르다.

## 중앙 링 실증

`PVP_RETOWN_A.mesh.bg_pvp_retown_floor01_sm`은 SL00에서 실제 placement 3개가
복구됐다. 이름 후보가 아니라 Component의 `StaticMesh` object reference가
해당 import를 직접 가리킨다.

발탄 중심부 좌표군에는 다음 원본 placement가 함께 존재한다.

```text
bg_pvp_retown_floor01_sm
  position = (849.491821, 2140.945313, 975.819458)
  rotation = (pitch 0, yaw -90°, roll 0)
  scale    = (0.490689, 0.513935, 0.225717)

bg_pvp_retown_floor02_sm
  position = (848.054810, 2138.892822, 975.727417)
  rotation = (pitch 0, yaw 405°, roll 0)
  scale    = (-0.497844, 0.507700, 0.160000)

bg_pvp_retown_floormetal01_sm
  position = (849.491821, 2146.780029, 974.910522)
  rotation = (pitch 0, yaw -90°, roll 0)
  scale    = (0.719677, 0.753772, 0.331052)
```

또한 같은 중심 좌표군에 `StaticMeshCollectionActor` 버전의 Floor01 instance가
있으며, 이 경우 실제 인스턴스 transform은 Actor가 아니라 Component에 저장돼
있다. 추출기는 두 저장 방식을 구분해 `transform.source`에 기록한다.

## 구현 중 확인된 Lost Ark 전용 규칙

- package summary의 compression flags는 `0x44`다.
- compressed chunk entry는 표준 UE3보다 4바이트 긴 20바이트다.
- 암호화 chunk는 LZ4 block별 앞 4096바이트 AES 해제 후 복원된다.
- `IntProperty`에는 선언 크기 외 8바이트 descriptor가 추가된다.
- Actor에는 UnrealScript stack frame이 있을 수 있어 property 시작 offset이
  export마다 다르다.
- 일반 StaticMeshActor의 정본 transform은 Actor에, StaticMeshCollectionActor의
  인스턴스 transform은 Component에 있다.

## 산출물

- 추출기: `Tools/LevelPlacementExtractor/extract_ue3_placements.py`
- 사용법/계약: `Tools/LevelPlacementExtractor/README.md`
- 결과 폴더:
  `C:\Users\user\Desktop\Resource_LostArk\05_Reports\MapExtraction\LV_LUT_HEARTRB_ED\placements`
- 전체 manifest: `placement_manifest.json`

## 다음 단계

MapTool에는 원본 JSON을 직접 덮어쓰지 않는 importer를 추가한다. importer는
catalog의 안정적인 asset ID와 object path를 매칭한 뒤 `parse -> validate ->
stage -> commit`으로 13,091개 중 선택한 레벨/그룹만 생성해야 한다. 좌표계
변환은 importer 한 곳에서 검증하고, 먼저 중앙 Floor01/Floor02/FloorMetal01
세 개의 상대 배치가 원작 화면과 일치하는지 확인한 후 전체 맵으로 확장한다.

## 독립 이중 검토

같은 원본 7개 package를 별도 출력 폴더에 다시 추출했다. 기존 7개 placement JSON과
manifest, 총 8개 파일의 SHA-256이 모두 일치했다.

추가 전수 불변식:

| 검사 | 결과 |
|---|---:|
| placement ID 중복 | 0 |
| 260 asset manifest에 없는 object path | 0 |
| owner Actor 누락 | 0 |
| non-finite Transform | 0 |
| zero scale | 0 |
| 일반 Actor의 non-identity Component transform | 0 |
| CollectionActor의 non-identity Actor transform | 0 |
| 음수 scale placement | 5,042 |

Actor Transform 정본 2,282개와 Component Transform 정본 10,809개의 합은 13,091개다.
UModel `-list`로 독립 복구한 SL00~SL05 StaticMeshComponent 합계 12,949개와 placement
추출기의 streaming-level 합계도 정확히 일치한다.

감사 receipt는
`C:/Users/user/Desktop/Resource_LostArk/05_Reports/MapExtraction/LV_LUT_HEARTRB_ED/placements/placement_audit.json`이다.

판결은 **UE3-native placement PASS**다. 다만 음수 scale이 5,042개이므로 현재 MapTool의
양수-only scale validator를 그대로 사용하면 원본 배치의 약 38.5%가 탈락한다. Client
좌표축/handedness와 reflection 처리, hidden/collision/material override 분류를 끝내기 전에는
**Client map reconstruction PASS**로 승격하지 않는다.
