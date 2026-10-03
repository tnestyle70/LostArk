# 워로드 Movie 머리카락 primitive 입력 수정 결과

## G00. 실제 결함과 적용 범위

native700의 `world.object.classselect.warlord.a12206.p4`와 native701의
`world.object.classselect.warlord.a12252.p0`가 각각 사용하는 재질은
`pc_wbk_07.mat_high.cc_wr_prop_01_mi_class`, `pc_wbk_07.mat_high.pc_wr_12_hair_mi_class`다.
Intro64키·Loop2키는 모두 visible이며 instance enabled도 true다. 각1024×1024 diffuse의
alpha>0 pixel은321039개,623853개다. 사라짐은 전체 투명 texture나 authored hide의 결과가 아니다.

현재 CPU Configure는 material row2부터 채우고 row0/1을0으로 남긴다. 두 native의 Base/Light
명령은 최종 alpha에 engine primitive opacity row1.w를 곱한다. Base 환경 출력에도 row0.x를
곱한다. 기존 native600과 같은 engine prefix 누락이며 material 색이나 mask 기본값의 오류가 아니다.

## G01. 소스 변경

Engine/Client의 `Shader_SourceCharacterBaseGroup640.hlsli`,
`Shader_SourceCharacterLightGroup640.hlsli`에서700/701 함수에만 prefix를 추가했다.
Base source0.x와 source1.w, Light source1.w를1로 연결한다. 그 외 native 명령, 시간식,
MIC uniform·texture·alpha mask, 기존702와 다른 program은 유지했다.

`Tools/VehiclePipeline/build_vehicle_source_material.py`의 emit_function에는 exact
program/stage, leading unowned row0/1, 원본 명령 전체 SHA256 검사를 추가했다.
701은 보존된 Base/Light shader ID도 함께 검사한다. 생성기와 설치본의 prefix 삽입 순서가 같다.
다른 번호에는 이 규칙을 적용하지 않으며 근거가 달라지면 실패한다.

Data·Resources·현재 render tuning·C++·프로젝트/filter는 수정하지 않았다.

## G02. 실제 수치 검증

현재 저작 material parameter를 읽고 실제 Configure의 helper와700/701 분기를 추출해 CPU
packing했다. 두 stage의 engine prefix가0인 상태를 확인했다. 실제 설치 DDS의 불투명·중간 알파
pixel을 같은 UV에서 읽고 색 공간에 따라 선형 입력으로 바꿨다. texture alpha0은 별도 음성 대조다.

실제 native 함수 전체와 현재 Base/ForwardLight varying adapter를 사용한 창 없는 D3D11 WARP
PS5 readback 결과는220 checks, failures0이다. 기존 native에 명시적인 primitive 입력을 준
reference와 수정본 RGBA가1e-6 이내로 같고, 모든 검사 출력은 finite다.

| native / stage | 불투명 DDS alpha | 중간 DDS alpha | alpha0 음성 대조 |
|---|---:|---:|---:|
|700 Base / Light|0 → 1|0 → 0.501961|0 → 0|
|701 Base / Light|0 → 1|0 → 0.501961|0 → 0|

실제 `Shader_VtxAnimMeshBinary_SourceGroup640.hlsl` fx_5_0 consumer 컴파일도 성공했다.
기존 Evaluate_Material X4000 경고는 남아 있다. 생성기 실제 guard 본문16개 case,
4개 shader의 예상 삽입 외 전체 HEAD 본문 보존, Engine/Client mirror, 기존 UTF-8 no BOM/LF와
scoped diff check를 통과했다. 새 제품 테스트 프로젝트나 프레임워크는 추가하지 않았다.
독립 읽기 리뷰에서도 native 명령 개수·SHA, 기존600 계약, stage 호출자, mirror와 변경 범위를
재확인했으며 필수 수정 결함은 없었다.

산출물은 `out/MovieWarlordHairPrefix20261004/`의 `source_receipt.json`,
`final_receipt.json`, `packer_compile.log`, `packer_run.log`, `numeric_compile.log`,
`numeric_run.log`, `anim640.compile.log`다. 전체 제품 Debug/Release 빌드와 설치 결과는
상위 Rendering Workbench 작업이 기록한다.

## G03. 원본 증거와 남은 판정

701의 Base/Light ID는 `dea8fd54f818c441b66600ac13b9ee61`,
`3b0966d408041a43b77c4810b621ee51`이다. 700은 group 첫 함수에 shader ID 주석이 남아 있지
않아 Base223명령 SHA `8286cf4f2f97129f2a7499ad9d4018fe128fe9ec469241533592d7cbe3cbc5de`,
Light254명령 SHA `1d7f9c0e5e8d32099b0422e5dda2f15fa6448af91055d5eea765243984dd3d6d`를
핀했다. 임의의 shader ID를 만들지 않았다.

현재 raw extraction과 retail ReleasePC가 없으므로 원본 bytecode 재추출 비교를 실행했다고
기록하지 않는다. 보존된 native 명령·material row 소유와 기존600의 prefix 계약을 근거로 한
좁은 입력 복구다. 아직 미확정인 camera/world prefix, SH·전체 원본 조명과 관계없는 입력을
채우지 않았다. Client/UI를 실행하지 않았으며 최종 머리카락 외형은 사용자 확인 항목이다.
