# 베른성 원본 크기 전수 비교 결과

## G00. 결론과 범위

현재 베른성의 정적 메시와 배치에 전체 축소는 없다. 원본 UPK를 새로 읽어 정적 메시 950개, 정적 배치 32,324개, foliage 배치 17,651개를 설치본과 대조했고 모두 원본 축척과 일치했다. 따라서 원본 복구라는 이름으로 추가 배율을 적용하지 않았다. 이 결과는 실제 데이터·기하 비교이며 사용자 화면의 원작 유사도 판정은 아니다.

대응 범위는 [구현 계획 G02](2026-09-11_KOUKU_FORWARD_MATERIAL_AND_BERN_SCALE_IMPLEMENTATION_PLAN.md)다. 조사 스크립트와 수치는 `out/BernScaleAudit20260911/`에 있으며 제품 런타임이나 별도 배포 계약을 추가하지 않는다.

## G01. 원본부터 실행 입력까지

| 비교 대상 | 실제 검사 | 결과 |
|---|---|---|
| 원본 level package 16개 | 현재 설치 게임의 암호화 UPK에서 이름·import/export·actor/component 값을 재추출 | 정적 배치 32,324개, property parse error 0, unresolved placement 0 |
| 정적 배치 전체 | 원본 UE3 위치·rotation·DrawScale/DrawScale3D와 현재 authoring 대조 | 32,324/32,324 일치 |
| 정적 메시 전체 | 정확 package/object identity의 LOD0 참조 정점 범위와 현재 WModel 정점 범위 대조 | 950/950 일치, geometry decode error 0 |
| foliage 전체 | 원본 InstancedStaticMeshComponent 1,697개의 native instance matrix를 재해석 | 17,651/17,651 위치·회전·축척 일치 |
| 실제 실행 입력 | authoring과 Client/Bin/DataFiles/Map의 source ID·asset·transform·visible 행 비교 | 50,017/50,017 전체 행 동일 |
| 로드 범위 | LevelRegistry의 Bern descriptor와 Loader 호출 확인 | MakeFullMapScope, 전체 shard-set 13개 |

정적 위치의 최대 오차는 0.000005m, quaternion 4.998e-10, signed scale 4.955e-8이다. foliage는 각각 0.0000005m, 4.9998e-10, 4.9979e-9다. 텍스트의 소수 자리수 범위에 해당하며 실제 축척 차이는 아니다.

원본 정적 배치 pivot의 범위는 X -1337.60984375~1017.08140625m, Y -97.9708203125~222.757265625m, Z -946.45671875~476.2702734375m다. X span은 약 2,354.69m이다. 이는 배경·연출 배치를 포함하는 범위이며 걸을 수 있는 마을 면적과 같지 않다.

`build_bern_castle_assets.py`는 glTF를 `--scale 100`으로 cook하고 `Loader.cpp`는 map CModel에 0.01 변환을 한 번 적용한다. placement 변환은 원본 cm의 `(x,y,z)`를 m의 `(x,z,-y)`로 바꾸며 DrawScale의 축척값은 유지한다. 설치 WModel 범위를 원본 cm 범위와 직접 대조해 중복 단위 변환에 따른 100배 축소가 없음을 확인했다.

## G02. 화면의 크기 감각과 별도로 남은 항목

현재 Bern follow camera는 FOV 60도, 위치 offset `(0.4,7.5,4.5)m`, look offset `(0,1.2,0)m`의 프로젝트 저작값이다. 이것을 원본 게임 camera와 동일한 값으로 검증하지 않았다. 맵 기하의 원본 축척과 화면에서 차지하는 크기는 서로 다른 입력이다.

CharacterCatalog의 차원술사 `presentationScale=1.5`는 이전 사용자의 외형 저작값으로 유지돼 있다. 이 값 역시 맵 자체 축척의 증거가 아니다.

현재 42개 landscape placement는 authoring/runtime에서 visible=false다. Imported와 Authoring의 차이도 이 42개 visible 값뿐이다. 이 숨김은 이번 세션 전에 이미 Git에 저장된 상태이며 이번에는 임의로 되돌리지 않았다. 42개 heightfield의 원본 높이·지형 재질은 이번 정적 메시 950개 비교 분모에 포함하지 않았다. 앞선 지형 복구 기록에는 절벽면 투영 문제와 표시 격리 이력이 있다. 전체 지형의 화면 충실도를 이번 축척 검사만으로 완료 처리하지 않는다.

## G03. 검증 경계

원본 재추출과 위 분모의 수치 비교를 실행했으며 생성 JSON을 다시 읽어 검사했다. 이 축척 조사 단계에서는 Bern 코드·authoring·Resources를 수정하지 않았다. 이후 사용자가 전체 재질·환경 복구를 승인하여 별도 구현이 진행 중이다. 따라서 이 문서의 축척 일치 결과를 이후 재질 설치와 전체 복구 완료 증거로 사용하지 않는다. Client와 Server는 자율 실행하지 않았고 화면 캡처도 하지 않았다. 원본 camera 또는 사용자가 비교한 동일 지점의 화면에 근거한 framing 조정과 landscape 표시 복구는 위의 원본 크기 일치 결과와 별도로 판단해야 한다.

## G04. 사용자 이미지 이후 추가 확인

사용자가 첨부한 원작/현재 화면 3개와 `C:/Users/user/Desktop/로스트아크_베른성`의 원작 이미지 5개를 열람했다. 현재 화면은 원작과 수면 반사, 석재·금속 표면, 식생 색, 그림자, 안개, framing에 차이가 있다. 이미지 하나에서 전체 스케일 오류를 확정하지 않았다.

현재 Valtan과 Bern follow camera는 같은 위치 offset `(0.4,7.5,4.5)`, look offset `(0,1.2,0)`, FOV 60을 사용한다. playable character도 같은 `CPlayableCharacterAssetService`의 admission transform을 소비한다. 베른에 진입했다고 별도 축소되는 분기는 발견하지 않았다. 일부 캐릭터는 armature 배율과 축 회전이 있으므로 raw mesh Y 범위만으로 실제 키를 계산할 수 없다. 실제 animation pose 측정은 현재 공통 Engine 수정 이후 동일 바이너리로 재검증해야 하며, 이전 중간 CSV를 최종 키로 사용하지 않는다.

정적 메시 950개는 StaticMeshActor의 고유 기하 분모다. 이후 전체 instance 조사에서 별도의 foliage 전용 기하 11개를 추가 확인했으며, 이 11개와 숨겨진 landscape 42개는 기존 950개 정적 메시 경계 비교에 포함되지 않는다. foliage의 17,651개 instance transform 비교와 그 기하/재질/조명 채널의 복구는 서로 구분한다.

## G05. 최신 Engine의 실제 pose 수치 확인

최신 Engine으로 playable 6개 class의 본체와 장비 37개를 실제 CModel로 생성하고 원본 `idle_battle_1` 시작 pose를 계산했다. 장비는 본체 pose와 자기 skeleton inverseBind를 사용했다. 생성·pose·정점 유한값 검사 오류는 0이다. 근거는 `out/BernScaleAudit20260911/character_pose_bounds.csv`와 `character_pose_latest.log`다.

이 값은 현재 admission transform과 CharacterCatalog presentationScale을 적용한 각 mesh의 Y 범위다. 실제 선택된 장비·avatar visibility를 합친 최종 외형 키, 원작 actor의 추가 배율, 화면 projection 일치로 해석하지 않는다. 기존 미확정 원본 카메라를 확인하지 않은 상태에서 이 검사만으로 Bern 맵이나 character 전체 배율을 변경하지 않았다. 후속 RNM catalog 분할은 full scope를 유지하며 shard 수만 늘고 배치 transform은 보존한다.
