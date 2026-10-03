# World Movie 워로드 투명 배우 primitive opacity 구현 계획

## G00. 현재 입력과 실패 소비자

선택된 `world.sequence.instance.classselect.warlord.intro.a12207.p0`는
`warrior_armor.slot0.wmodel`의 `wp_wwbk_12_mi_high` 메시다. 이름과 달리 실제 배우
override는 `scene_a.mat.transparent_inst`, `source.character.selection-native-702.v1`이다.
MIC color와 selectioncolor는 검정이며 op 기본값은0, Intro/Loop 곡선은0.7이다.
이 값은 정본으로 보존한다.

현재 Configure는 native702의 material 소유 row1~3만 채운다. Base PS
`3b3abe5b3d623749aeec90310df73939`의 명령3과 Light PS
`d3542163f308a34e94adac8baf7fd59d`의 명령25는 texture R × row3.x(op)에
row0.x를 곱한다. zero-init된 미소유 row0 때문에 opacity가0이며 forward shader가 discard한다.
선택은 CPU 삼각형 검사이고 강조는 별도 outline pass라 재질 표시와 독립적이다.

원본 의미의 근거는 저장소에 보존된 exact shader ID·번역된 DXBC 명령과 같은 MIC packer다.
현재 PC에는 이전 ClassMovies20260925 extraction 및 retail ReleasePC가 없어 원본 package
재추출이나 bytecode 재대조를 실행했다고 기록하지 않는다. 기존 native600의 동일한
engine-owned prefix 복구 절차를 exact702에 적용한다.

## G01. 좁은 shader adapter와 생성기

Engine/Bin/ShaderFiles와 Client/Bin/ShaderFiles의
`Shader_SourceCharacterBaseGroup640.hlsli`, `Shader_SourceCharacterLightGroup640.hlsli`에서
702 함수가 constant 배열을 읽은 직후 engine primitive opacity인 `source[0].x = 1.f`만
설정한다. 다른 native 함수, material row, texture와 forward blending은 유지한다.

`Tools/VehiclePipeline/build_vehicle_source_material.py`의 emit_function은 program702를
생성할 때 Base/Light별 위 shader ID, leading unowned row0 및 정확한 opacity 명령을
확인한 뒤 같은 한 줄을 생성한다. 다른 번호를 자동 보정하지 않으며 근거가 달라지면 실패한다.
기존 파일만 수정하므로 새 프로젝트/filter 등록은 없다.

## G02. 검증과 반영 경계

실제 변경 함수의 Base/Light HLSL을 headless WARP에서 컴파일·수치 readback한다.
기존 row0=0이면 alpha0, 수정 후 flat-white와 op0.7이면 alpha0.7, texture R0/0.25/1 및
명시적 op0/0.7/1의 곱을 확인한다. material color와 다른 함수 본문은 보존 대조한다.
실제 Group640 shader 컴파일, Engine/Client mirror, generator guard, 인코딩과 diff check를
확인하고 제품 Debug/Release 통합 빌드는 상위 작업에서 수행한다.

Data·Resources·렌더링 옵션 변경과 Client/UI 실행은 없다. 수치 검증은 원본 장면과의
시각 동등성이나 사용자의 최종 화면 확인을 대신하지 않는다.
