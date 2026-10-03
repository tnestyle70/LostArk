# 워로드 Movie 머리카락 primitive 입력 구현 계획

## G00. 원본 계산과 실제 사용 범위

Character Select 원본 근거 감사에서 native700/701의 engine prefix 누락을 확인했다.
700은 `world.object.classselect.warlord.a12206.p4`의 `cc_wr_prop_01_mi_class`,
701은 `world.object.classselect.warlord.a12252.p0`의 `pc_wr_12_hair_mi_class`다.
각각 Intro64키·Loop2키가 전부 visible이며, 두 diffuse DDS의 alpha>0 pixel은
321039/1048576과623853/1048576이다. 원본 asset의 전체 투명 설정이 아니다.

actual Configure는 두 program의 material uniform을 row2부터 채우고 engine row0/1을
0으로 남긴다. Base와 Light의 보존된 원본 명령은 최종 alpha에 row1.w를 곱한다.
Base는 환경 출력에 row0.x도 곱한다. 이미 복구한 native600과 같은 engine prefix 계약이다.
row0.yzw/row1.xyz의 다른 scene 의미와 material opacity/mask는 이번 변경 대상이 아니다.

701의 Base/Light ID는 `dea8fd54f818c441b66600ac13b9ee61`,
`3b0966d408041a43b77c4810b621ee51`이다. 700은 group 첫 함수에 원본 ID 주석이
남아 있지 않아 보존된 원본 명령 전체의 SHA256을 guard로 사용한다. 현재 PC의 raw extraction과
retail ReleasePC 부재를 숨기거나 원본 bytecode를 새로 대조했다고 기록하지 않는다.

## G01. 파일과 생성 계약

Engine/Client의 BaseGroup640·LightGroup640 HLSLI에서700/701 함수에만 engine identity를
추가한다. Base는 source[0].x와 source[1].w, Light는 source[1].w를1로 연결한다.
MIC 색·texture·cutting mask·저장된 render tuning·다른 native는 그대로 보존한다.

`Tools/VehiclePipeline/build_vehicle_source_material.py::emit_function`은 exact program/stage,
원본 명령 fingerprint, leading unowned row0/1 및701 shader ID를 확인한 뒤 같은 prefix를
생성한다. 근거가 달라지면 실패한다. 신규 C++·project/filter·Data·Resources 변경은 없다.

## G02. 검증

실제 현재 material parameter, actual Configure의 두 분기와 native shader 함수 전체를 추출해
headless CPU packing·PS5 WARP 수치 검증을 한다. 현재 설치 DDS에서 읽은 샘플로 이전 alpha0과
복구 후 finite/nonzero alpha를 대조하고 texture alpha0/cutting mask의 의도적 투명도도 유지한다.
원본 instruction·무관한 함수 보존, Engine/Client mirror, generator fail-closed guard,
Group640 실제 consumer 컴파일, 인코딩과 diff check를 확인한다.

전체 품질 복원 완료나 역사적 초기 WModel 장면을 재현했다고 표현하지 않는다.
Client/UI 및 최종 사용자 화면 확인은 자동 수치 검증과 분리한다.
