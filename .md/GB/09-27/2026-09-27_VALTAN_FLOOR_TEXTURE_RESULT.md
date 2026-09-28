# 발탄 진입로 바닥 겹침 수정 결과

## G00. 실제 원인

사용자가 첨부한 화면을 열람했다. 바닥 중앙의 삼각형 단위 교차 무늬와 지정 좌표
`(47, 10, -63.77)`를 실제 Map placement 및 설치 WModel에 대조했다.
이곳은 Y 약 10 m인 발탄 진입로다. Y 약 23 m의 파괴 아레나 균열과 다른 표면이다.

문제 위치를 덮는 표면은 editor 배치 43과 44다. 둘 다
`MAP_0A63D07B8405_BG_FAT_OCASTLE_TILE_FLOOR02_SM_KSR`의 평면 mesh 0을 사용하고,
같은 Y `10.0600004`에 배치되어 있었다. 44의 비균일 크기 때문에 실제 정점 변환 후 아주
작은 차이는 생기지만 안정적인 깊이 분리가 아니다. texture 파일 손상이 아니라 서로 다른
UV 위치의 바닥 두 장이 거의 같은 깊이에 그려지는 Z-fighting이다.

| 실측 | 수정 전 | 수정 후 |
|---|---:|---:|
| 배치 43의 지정 XZ 표면 Y | 10.06000078146969 | 동일 |
| 배치 44의 지정 XZ 표면 Y | 10.060000655584698 | 10.050000655584698 |
| 전 중첩 영역의 최소 표면 간격 | 0.0000001258849913 m | 0.0100001258849911 m |
| XZ 중첩 면적 | 4.4062670635 m² | 동일 |

설치 WModel의 실제 정점·인덱스, runtime preScale 0.01 및 저장 quaternion/scale/position을
사용했다. 원본 복원 WModel 1.1/1.2/1.4와 legacy 1.0 reader를 각각 적용했으며, 지정 XZ와
Y 9~11 m에서 확인된 표면은 이 두 장뿐이다. 근거 파일은 로컬
`out/ValtanFloor20260927/geometry-diagnostic.json`에 있다.

## G01. 변경한 데이터

저작 placement의 stable ID `44`, source ID `editor:LV_LUT_HEARTRB_ED:44`의
`position.y`만 `10.0600004 -> 10.0500004`로 바꿨다. 넓게 늘린 연결 바닥인 44를
1 cm 낮추고 사용자가 지적한 지점의 기존 위쪽 표면 43은 유지했다.

- 바닥을 삭제하거나 XZ 크기를 줄이지 않아 기존 덮개와 연결 구역을 보존했다.
- 13,184행 중 해당 필드 하나만 바뀌었다. ID, asset, XZ, 회전, 크기, 가시성은 동일하다.
- texture, WModel, material, rendering option, navigation, gameplay 데이터는 변경하지 않았다.
- 최신 원본을 `out/ValtanFloor20260927/LV_LUT_HEARTRB_ED.mapplacements.before`에
  백업하고 교체 직전 원본 bytes 일치 확인 후 임시 파일을 원자 교체했다.

배치 44 주위 15 m의 설치 geometry에서 높이 10.04~10.07 m를 통과하는 XZ 중첩
삼각형 750쌍을 추가 대조했다. 교차 다각형 전체가 1 mm 이내인 공면 쌍은
43과의 2쌍에서 0쌍으로 감소했다. 주변 바위와 풀의 경사진 면은 원래 바닥과 교차하고
있으며, 높이 조정으로 그 교차선도 이동한다. 신규 근접/교차 후보 25쌍의 가장 작은
높이 변화 폭도 0.0142 m로, 두 평면 전체가 겹친 원인과 구분된다. 이 수치 검사는
인접 지형의 최종 화면 검증을 대신하지 않는다.

## G02. 게시와 검증

| 실행한 검증 | 결과 |
|---|---|
| `Publish-MapAuthoring.ps1 -AreaId LV_LUT_HEARTRB_ED -Scope Placements -Mode Validate` | PASS, 13,184 placements / 8 shards |
| 같은 명령의 `-Mode Publish` | PASS |
| 최신 저작 행과 BASE runtime shard의 배치 44 비교 | 동일 |
| 실제 수정 뒤 WModel 교차 다각형 높이 재측정 | 전 구간 약 1 cm 분리 |
| 전체 저작 행 비교 | 44의 Y 한 필드만 변경 |
| `git diff --check` 대상 경로 | PASS |

publisher는 17개 출력 후보를 처리했지만 실제 변경된 runtime 파일은
`Client/Bin/DataFiles/Map/LV_LUT_HEARTRB_ED_BASE.mapplacements` 하나다.
저작 파일과 이 게시 파일은 Git LFS 전달 대상이다. Resources 교체나 Drive 추가 전달은 없다.
C++/shader 변경이 없으므로 이 수정 자체에는 재컴파일이 필요하지 않다.

## G03. 남은 사용자 확인

Client/UI를 실행·조작하거나 화면을 캡처하지 않았다. 새로 Client를 실행하고 발탄 맵의
`(47, 10, -63.77)`와 이어지는 바닥 경계에서 삼각형 깜빡임이 없어졌는지 사용자가 확인한다.
수치상 원인 제거와 게시 완료까지 확인했으며, 사용자 최종 화면 판정은 아직 받지 않았다.
