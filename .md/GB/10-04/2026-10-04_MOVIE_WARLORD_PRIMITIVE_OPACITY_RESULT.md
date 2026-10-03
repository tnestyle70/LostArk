# World Movie 워로드 primitive opacity 수정 결과

## G00. 사라진 모델의 실제 원인

사용자가 선택한 actor12207 part0의 stable ID는
`world.sequence.instance.classselect.warlord.intro.a12207.p0`다. 실제 mesh 이름은
`wp_wwbk_12_mi_high`지만 원본 연출 override는 `scene_a.mat.transparent_inst`와 native702다.
이는 보이는 actor726의 `warrior_sword.slot0.wmodel` 및 native703 일반 무기 재질과 다르다.

native702의 실제 Base/Light 번역 명령은 alpha에 texture R × material op × engine
primitive opacity(row0.x)를 사용한다. material Configure가 채우는 것은 row1~3이며
row0은 zero-init으로 남아 있었다. Intro와 Loop에서 op가0.7로 보간되어도 alpha0이 되어
forward shader의 opacity discard에 걸린다. 검정 material color 자체를 하얗게 바꾸거나
authored op 기본값0을 수정해야 하는 문제가 아니다.

`CWorldSequenceObject::Try_PickInspection`은 현재 pose의 CPU 삼각형을 검사하며 재질
opacity를 샘플하지 않는다. 선택 강조는 별도 `Render_CombatHoverMesh` outline pass다.
따라서 재질이 보이지 않아도 선택과 빨간 윤곽은 나타날 수 있다.

## G01. 반영한 변경

Engine/Client의 `Shader_SourceCharacterBaseGroup640.hlsli`와
`Shader_SourceCharacterLightGroup640.hlsli`에서 SourceCharacterBase702/Light702에만
`source[0].x = 1.f`를 추가했다. 702의 원본 RGB 명령과 material op·색·texture,
actor726의 native703 및 다른 모든 함수 본문은 보존했다.

`build_vehicle_source_material.py::emit_function`은 program702에만 같은 engine 입력을
생성한다. Base ID `3b3abe5b3d623749aeec90310df73939`, Light ID
`d3542163f308a34e94adac8baf7fd59d`, leading unowned row0과 정확한 opacity 소비 명령이
달라지면 실패한다. Data·Resources·렌더링 옵션·C++·project/filter 변경은 없다.

## G02. 실제 검증

- 변경 전 HEAD와 변경 후의 실제 Base/Light 함수 전체를 추출해 PS5로 컴파일하고
  창 없는 D3D11 WARP target에서 수치 readback했다. 252 checks, failures0이다.
- texture R0/0.25/1 × authored op0/0.7/1의18조합에서 이전 alpha는0,
  수정 alpha는 정확한 texture R × op다. 양 stage의 R1/op0.7은0→0.7,
  R0.25/op0.7은0→0.175다. 명시적 op0은 계속0이며 RGB는 전후 동일하다.
- 실제 설치된 `Character/ClassSelect/SourceTextures/50c22a2692ed/flat_white.dds`는
  2×2이며 RGBA 모든 pixel이255다. 원본 texture 선택을 유지한다.
- 실제 `Shader_VtxAnimMeshBinary_SourceGroup640.hlsl`의 fx_5_0 컴파일 exit0.
  기존 Evaluate_Material의 X4000 경고는 남아 있다. 이 consumer는 forward light도 포함한다.
- 실제 생성기 guard 본문으로 양 stage의 허용/잘못된 shader/row 소유/명령/다른 program
  10개 case를 검사했다. 4개 shader 전체에서 의도한2줄 외 HEAD 본문이 동일하며,
  Engine/Client mirror, 기존 UTF-8 no BOM/LF, scoped diff check가 통과했다.

증거는 `out/MovieWarlordPrimitiveOpacity20261004/`의 `source_receipt.json`,
`final_receipt.json`, `numeric_compile.log`, `numeric_run.log`, `anim640.compile.log`다.
제품 Debug/Release 통합 빌드 결과는 상위 반영 작업이 소유한다.

## G03. 근거와 화면 판정의 범위

현재 PC에는 이전 ClassMovies20260925 raw extraction과 retail ReleasePC가 없다.
exact shader ID, 보존된 원본 DXBC 명령 주석, material packer와 실제 CPU/GPU 소비자를
대조했으며 원본 package 또는 bytecode를 새로 추출했다고 주장하지 않는다. 생성기 guard의
closure fixture도 현재 material row 소유에서 확인한 [0]이며 새 raw dump가 아니다.

Client/UI는 실행하지 않았다. 이 수정은 잘못된 alpha0 discard를 제거한다. 검정·반투명
연출 재질의 색을 일반 갑옷 색으로 바꾸지 않으며, 최종 장면에서의 외형은 사용자 확인 항목이다.
