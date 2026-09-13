# 마리오 아이언메이든·일반/붉은 칼날 리소스 연결 계획

## G00. 요청 범위와 현재 입력

쇼타임 Animation Box의 위치·회전 및 이펙트별 bloom 질문에는 현재 저장/실행 경로와 권장 구조를 설명한다. 이번 구현은 요청한 마리오 모델·칼날 리소스의 추출과 저작 목록 연결이다. 보스 이동·회전 또는 bloom 저장 계약을 이번 변경에서 새로 구현하지 않는다.

기존 `codex/kouku-authored-finale-popup` checkout에 다른 작업의 대규모 미커밋 변경이 있다. 관련 catalog/tree/Composition/project 파일의 시작 상태를 `out/KoukuMarioAssets20260912/baseline`에 보존했다. 기존 Pattern의 박스·시간·위치·사용자 표시명과 shader 변경을 유지하고 자동 stage/commit하지 않는다.

원본 아이언메이든은 `mn_boim_00.mesh.fx_d_boim_00`, 재질은 `mn_boim_00.mat.mn_boim_00_mi_dead`다. 기존 V1 원본 `fx_mn_rpct_07_d.par_d_boim_buff01_02l`의 mesh 요소와 종료 `_03e_loc_int`를 조사한다. `Effect/KoukuSaydon/FullRestore/Meshes/fx_d_boim_00.wmodel`과 native3107 입력이 이미 설치돼 있으므로 같은 리소스를 중복 추출하지 않는다.

추가 원본 조사에서 `mn_boim_00.mesh.mn_boim_00_sk`와 14 tracks/5 clip의 AnimSet도 확인했다. `respawn_1`, `idle_normal_1`, `att_battle_01`, `dead_1`, `dead_1_loop`를 기존 UModel glTF/PSA 결합·축/weight 보정·30Hz 정규화 경로로 별도 애니메이션 WModel에 보존한다. 정적 particle mesh와 구분해 필요한 World Object 모션으로 연결하고 target slot과 원본 재질의 호환성을 먼저 검증한다.

칼날은 `fx_mn_rpct_07_v.par_v_rpct_cutting_pjt_01`과 `_02_loc_int` 및 대응 `cutting_exp` 원본이다. 일반 `_01`은 원본 그래프에 있지만 현재 설치 목록에서 빠졌고, `_02_loc_int`는 기존 문서와 native2818 mesh 재질이 있다. source projectile·색상 입력으로 둘의 대응을 확인한 뒤 표시명을 정한다.

## G01. 원본 추가 투영과 목록 등록

기존 EffectPipeline의 first-LOD/module/default 추출과 native descriptor 투영을 재사용해 빠진 일반 칼날만 추가한다. 설치된 source material/VF/VS/PS와 texture·parameter가 일치하면 기존 native 프로그램을 재사용한다. 새로운 임의 shader나 근사 색칠로 일반/붉은 variant를 만들지 않는다.

기존 아이언메이든·붉은 칼날 문서는 payload를 보존하고 EffectCatalog/EffectResourceTree와 Kouku Composition의 `presentationResources`에 연결한다. 사용자는 마리오 폴더에서 선택해 Pattern에 직접 Append한다. 저장된 Pattern에 발생 시점이나 플레이어 감금 로직을 자동 배치하지 않는다. 아이언메이든 원본이 static particle mesh인 경우 독립 skeletal 애니메이션 복원으로 표현하지 않는다.

`install_kouku_effect_library.py --library-only`의 기존 등록 경로로 목록·stable ID와 Client `.vcxproj`/`.filters`의 `96.DataFiles` None 항목을 추가한다. Resources binary는 기존 일곱 최상위 폴더의 Effect 하위에 두고 Git에 추가하지 않는다. 새 C++ 파일은 제안하지 않는다.

## G02. 검증과 전달

새 문서의 실제 Client codec Load/Validate/Save roundtrip, 시간별 CPU playback·seek, 필요한 WModel/DDS·native resource staging을 기존 비UI 검사로 확인한다. catalog/tree/Composition의 실제 참조와 기존 Pattern 보존, JSON/XML parse 및 변경 범위 diff를 검사한다. 신규 C++/shader가 없으면 불필요한 제품 재컴파일을 하지 않는다.

Client/UI를 실행·조작하거나 화면을 캡처하지 않는다. 사용자에게 Effect Tool V1과 Action Workbench의 선택 경로 및 실제 설치 모델 경로를 전달한다. 리소스 연결·수치 검증과 사용자 화면 판정, Server gameplay 감금/칼날 이동 구현 상태를 RESULT에서 구분한다.
