# 마리오 작은 오망성 바닥 연결 구현 계획

## G00. 현재 입력과 변경 범위

사용자가 확인한 작은 별 그리기 V1의 기존 8개 mesh 요소를 보존하고, 바닥 별을 함께 한 번 재생한다. 표시 이름은 `작은 오망성`으로 정리한다.

`effect.kouku.gate3.mario.center.pentagram.full.restore`는 실제로 바닥 해골·연기 sprite 10개이며, `center.full.restore`도 이 요소와 진입 포탈 7개를 조합한 문서다. 여기에는 별 바닥 decal이 없다. 바닥 재사용 입력은 기존 `boss.kouku.disarm` 그룹의 `boss.kouku.disarm.star.decal_1` 5개 배치다. 이 원본 그룹은 수정하지 않는다.

작은 별은 설치된 `fm_d_rpct_07/08.wmodel`의 geometryPreScale과 source StartSize를 한 번씩 적용한 중심선 반경을 실측한다. source MeshRotation lookupTable의 bounds 뒤 roll/pitch/yaw도 적용해 바닥 방향을 맞춘다. 큰 바닥 별은 기존 5개 배치의 중심 반경과 정오망성의 변 중심 관계로 기준 반경을 구한다. 두 반경의 비율로 위치와 decal 폭·길이를 함께 축소한다.

## G01. 기존 V1 문서와 생성기

`Tools/EffectPipeline/build_kouku_gate3_slam_mario_restore.py`에 작은 별 완성 함수를 연결한다. 같은 함수를 새 투영과 기존 작은 별에 대한 명시적 적용에서 사용하며 기존 8개 요소, asset ID, 다른 마리오 문서는 보존한다.

바닥은 기존 V1의 일반 LocalDecal 다섯 요소로 연결한다. base는 기존 `fx_a_decal_020.dds`이며 shader와 Resources payload는 추가하지 않는다. 별 그리기의 마지막 실제 burst와 particle lifetime으로 종료 시점을 계산하고, 같은 시간 안에 나타나고 사라지도록 기존 alpha 곡선 경로를 사용한다. 원본의 20m 크기와 9초 수명을 그대로 가져오지 않는다.

`Data/Effects/EffectResourceTree.json`과 `install_kouku_effect_library.py`의 작은 별 표시 이름을 함께 정리한다. 공용 Composition의 resource 이름은 Parent 구현자가 같은 작업에서 반영한다. 새 C++ 및 project/filter 등록은 없다.

## G02. 검증과 수동 확인

JSON parse, 기존 codec/CPU playback probe, 실제 Resource 파일 존재, 5개 바닥의 크기·기간·중복 ID 검사와 변경 범위 diff 검사를 수행한다. probe는 out 산출물만 사용한다. 원본 후보의 8개 요소가 보존됐는지 비교한다.

Client/UI 실행과 캡처는 하지 않는다. 사용자가 Effect Tool V1의 마리오 분류에서 `작은 오망성`을 재생해 별의 바닥 크기와 사라지는 시간을 최종 확인한다.
