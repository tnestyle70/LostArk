# World Movie 머리카락·가디언 눈 재질 구현 계획

## G00. 현재 경계와 정본

사용자 정정에 따라 창술사 무비의 머리카락을 복구한다. 허리·몸체는 변경하지 않는다.
All Effects의 World 목록은 기존 다섯 Movie의 class ID를 사용하며 Open Editor와 Play All은
09-26 World Movie Effect Editor의 Level-owned 재생·편집 경로를 재사용한다.

## G01. 창술사 native600의 primitive 상수

`Shader_SourceCharacterBaseGroup576.hlsli`의 SourceCharacterBase600은 최종 opacity에
cb0[1].w를 곱하지만 Configure는 원본 material 소유 범위2..24만 채운다. 추출한
원본 PS040a63ec2e3e5e42a8f2194c6622723a는0/1을 engine 소유 prefix로 선언한다.
같은 opacity와 environment 배율의 기존 hair99 처리처럼 native600의 필요한 lane만
identity로 연결한다. Light의 대응 opacity lane도 같은 원본 계약으로 확인한다.
`Tools/VehiclePipeline/build_vehicle_source_material.py`가 같은 source ID와 상수 범위를
검증한 뒤 같은 보정을 생성하도록 하고 Client/Engine shader mirror를 함께 갱신한다.

## G02. Guardian eye의 실제 입력 검증

현재 일반 모델과 Movie의 eye는 모두 source.character.eye.v1이다. 현재 설치 WModel의
눈210/380정점, UV1/UV2와 원본 iris texture alpha가 존재한다. 원본 shader의 UV·parameter·
texture 입력, 실제 model bind와 draw 소비를 확인해 누락 지점을 수정한다. 기존 골격과
애니메이션·부착·조명 튜닝을 보존한다. 리소스 변경이 필요하면 원본 연결 증거, 최신 bytes
대조, 백업·원자 교체 후 Runtime Resources와 Desktop/GBResources에 같은 상대 ID로 전달한다.

## G03. 검증과 인계

원본 shader의 engine/material 상수 경계를 대조하고 수치 alpha/iris 검사를 수행한다.
변경 shader를 격리 컴파일하고 Product Debug/Release 빌드는 상위 통합 작업에서 조율한다.
새 C++ 파일·프로젝트 등록은 없다. Client/UI 실행·스크린샷·시각 PASS는 수행하지 않는다.
Movie5의 Open Editor/Play All은 실제 기존 호출 경로와 문서/resource 연결을 재검사하고,
RESULT에 자동 검사와 사용자 최종 화면 확인을 분리한다.
## G02. 가디언 눈 최종 프로젝트 조정

실제 Movie20s camera/pose와7개 Movie light·현재 scene directional을 사용하는 native5
수치 대조에서 원본 shadowfactor0.6은 각 직접광마다 최소0.4 밝기를 남겨 눈의 높은
휘도가 누적된다. 사용자가 반사도 낮춰 눈동자를 보이게 마무리하도록 요청했으므로
가디언 일반 body의 pc_dl_eye_mi와 Movie object a12265.p2의 pc_dk_eye_00_mi에만
shadowfactor1.0, tdspecular_intensity0.25를 적용한다. 이는 원본0.6/2를 프로젝트에 맞춘
조정이며 shader ABI 복원으로 설명하지 않는다. iris RGB·alpha·size·texture와 전역
렌더링 프로필은 보존한다. 최신 JSON의 두 stable row·두 field만 hash 확인·백업·원자
교체로 병합하고 WorldSequences 범위 publisher로 런타임 정본을 갱신한다. Guardian
Movie 재설치기는 현재 저장된 두 lighting field를 유지해 source donor refresh가 튜닝을
되돌리지 않게 한다. 새 schema·C++ 파일·Resources 추가는 없다.

## G04. 창술사 헤어 외형 개선과 가디언 Movie 근거 검사

사용자는 창술사 헤어를 원인 확정과 관계없이 개선하도록 요청했다. 일반·커스터마이징의
FT00은 원본 재질 override가 없고, Movie의 FT06-high는 실제 CModel pose에서 형상과 본
스케일이 유지된다. 따라서 이번 교체는 원본 오류 복원으로 설명하지 않고 요청한 외형
개선으로 기록한다. 같은 창술사 FT 계열의 더 풍성한 헤어를 우선 사용하며 다른 class의
두상과 골격을 추측해 이식하지 않는다.

일반 기본 장착과 커스터마이징의 초기 선택은 같은 asset ID를 사용하고 기존 사용자가
선택한 preset을 보존한다. 기본 장착의 material override는 CharacterCatalog의 해당
소유 경계로 옮기며 중복 소유를 만들지 않는다. Movie는 actor a12230.p0의 stable ID,
Intro·Loop clip과 clock을 보존한다. 일반 234본과 Movie 208본의 차이를 직접 검증하고,
양의 weight가 있는 본을 버리지 않는 호환 파생 모델을 기존 WModel 경로로 만든다.
원본 asset과 shader·전역 rendering option은 보존한다.

후보는 실제 골격·bind·clip과 정점 pose, material 소비를 검사한 후 최신 저장본의 해당
필드에만 병합한다. 설치와 Resources 전달은 같은 상대 ID 및 SHA로 확인하고, JSON parse,
변경 C++ 최소 컴파일, 필요한 Product Debug 빌드와 diff 검사를 수행한다. 사용자 화면
평가와 수치 검증은 분리한다. 새 런타임 C++ 타입이나 프로젝트 등록은 계획하지 않는다.

가디언의 회전·위치는 source root와 TypeData 값에 이어 실제 최종 particle draw matrix를
대조한다. 미연결 crack WORLD는 원본 actor·material·visibility 근거를 조사하며 사용자
표현인 유리와 동일하다고 단정하지 않는다. 근거가 없는 회전·위치·알파 보정은 적용하지 않는다.
