# 차원술사 Alt V 시작 장면과 큐브 연결

## G00. 현재 입력과 사용자 편집 보존

현재 gameplay는 `2050540 -> pc_sp_m_00_sk_sk_super_timewave ->
effect.dimensionmaster.skill.2050540.full.restore`를 사용한다. full은 현재309행이고,
tuning은26행이다. 이전 RESULT의318행을 현재 정본으로 되돌리지 않는다. full에서 초기
capture emitter18/31이 빠져 있고, 후반 `altv.source.notify036.cube`는 native material 없이
기본 OPAQUE 표면을 그린다. 사용자 요청의 시작 화면·중앙 큐브를 이 두 실제 소비자에 연결한다.

모든 기존 position/rotation/scale, source distribution, visibility와 명시 modelPreScale을
보존한다. `fm_h_box_01_1` 실제 WModel은 각 축[-25,25]cm이고 mesh StartSize는 무차원이므로,
생략되어1이 된 geometry preScale만 .01로 명시한다. animated cube는 이미 미터 단위와
unit bone basis이므로 같은 .01을 적용하지 않는다.

## G01. 기존 렌더러의 장면 입력과 제출 순서

`Effect_DimensionMasterALTVMaterial.h`는 원본 ALT178 descriptor를 animated cube의
선택 material로 허용한다. Codec과 Renderer의 기존 ModelCue stage에 동일 texture·parameter
검사를 연결한다. `Shader_VtxAnimMeshBinary.hlsl`은 기존 skinned VS와 native material pass에서
ALT178 RT0식을 재사용하고, Renderer는 해당 occurrence의 frozen SceneHDR를 texture2에 바인딩한다.
카메라의 별도 원본 capture-view 행렬은 미해석이며 기존 cube-face UV adapter 범위로 기록한다.

시작 캡처는 기존 occurrence별 stage/Save-preserve 수명을 사용한다. model cue만 capture를
요구해도 같은 snapshot을 준비한다. 캡처를 사용하는 문서에서는 기존 Mesh/Model을 먼저,
Sprite를 뒤에 제출해 mesh의 배경 읽기에 같은 문서의 sprite가 먼저 섞이지 않게 한다.
일반 문서의 기존 순서와 Engine 전역 render group은 변경하지 않는다.

## G02. 큐브 모션과 단위

설치된 animated cube는3333.333tick/1000Hz로 저장됐지만 CModel은30Hz로 소비한다.
기존 `retime_wmodel_ticks.py`로 out 후보를30Hz로 변환하고 각 key 값과 실제 pose를 대조한다.
사용자 Visual Studio build가 진행 중이므로 Resources 교체·제품 빌드·DLL/CSO 배포는
root와 빌드 종료 시점을 조율한 뒤 수행한다. 런타임에 임의33.333배 clock 분기를 추가하지 않는다.

## G03. 검증과 남은 경계

기존 WModel reader, focused Codec/Playback 검사와 최소 격리 컴파일로 source values,
parse/Save와 shader 지원 경계를 확인한다. 새 광역 validator나 admission framework는 만들지
않는다. Client/UI 실행·캡처는 하지 않는다. 제품 빌드 여부, Resources 실제 교체 여부와
사용자 화면 확인을 RESULT에 구분한다. 새 C++ 파일은 추가하지 않는다.

## G04. 시작 화면 전체의 카메라 프레이밍

camera source emitter18/31과 ALT178의 교집합만 CModel의 preScale 적용 bounds를 현재
view/projection으로 화면에 맞춘다. 두 occurrence는 root elapsed의 공통 .03→2초 구간을
smoothstep으로 소비하며 각 particle 생성 시 확대가 재시작되지 않는다. 시작 affine만
카메라에서 계산하고 보간의 끝은 기존 evaluated World를 정확히 반환한다. near plane,
영/비정상 bounds, 특이 view나 비유한 결과에서는 기존 World를 유지한다. native mesh
instance 경로와 단일 mesh fallback이 같은 함수로 연결되며 저장 필드는 추가하지 않는다.


## G05. 실행 중인 기존 Client와 데이터 배포

새 ALT178 ModelCue material은 기존 Codec에서 거부되므로 두 Authored 변경은 out 후보로
보관한다. 작업 전 사용자 원본의 bytes를 유지하고, 새 실행 파일·shader·retimed cube 설치
시점에 두 데이터를 함께 적용한다. 설치 직전 현재 source와 기록 baseline을 비교하고,
사용자 추가 편집이 있으면 먼저 합친다. 후보·SHA·보류 상태는 RESULT G04에 기록한다.
