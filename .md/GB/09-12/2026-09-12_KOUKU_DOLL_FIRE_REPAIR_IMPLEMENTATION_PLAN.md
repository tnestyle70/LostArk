# 괴기스러운 인형 골격과 외곽불 발광 입력 교정 계획

## G00. 실측

월드 오브젝트 world.object.kouku.odd_doll은 Character/KoukuSaton/MN_CDMD_00/MN_CDMD_00.wmodel을 쓴다. 설치본은 9,301정점·58노드·30clip이며 이미30Hz다. 원본57본 PSA30clip을 다시 추출해 att_battle_2_01, walk_normal_1, idle_normal_1의 첫 자세를 대조했으며 자식 본 회전 최대 성분 오차가 약1.414다. 09-11 말·호랑이의 child quaternion conjugate 누락과 같은 유형이다. 기존 weight 교정과 시간 단위 교정은 보존해야 한다.

외곽불 D/E/F 세 WModel은 WINT1.2 정적 모델이며 WMA2 slot0 emissivePath가 반사광용 textures/b2378a8f80d6_t_tds_specular04.dds다. 기존 repair_gate3_fire_materials.py는 WINT1.0만 받아 현재1.2 입력에서 거부한다. World Object는 해당 내장 재질을 사용하므로 잘못된 발광 입력이 실제 draw에 전달된다.

## G01. 인형

기존 build_umodel_gltf_psa.py의 root 유지·child conjugate·FLOAT weights 경로로 원본 glTF와 PSA를 재cook한다. 기존 모델의 geometry/skeleton/material과 원본 key position/scale/time이 동일한지 확인한 뒤 WANM의 회전 교정 결과만 후보 모델에 반영한다. 기존 body preScale .01과30Hz, 모든 clip 이름을 유지한다. 원본57본/30clip 전체의 회전 표본을 대조하고 각 기본 동작의 골격·정점 유한값을 확인한다.

기존 Resources를 사용자가 실행 중이므로 후보와 백업은 out/KoukuDollFire20260912에 먼저 둔다. 정지 확인 뒤 전체 입력 SHA 재확인과 원자 교체를 수행한다. binary를 Git에 넣지 않는다. 기존 Object Tool/Box Detail의 위치·회전·scale와 새 정확한 Object/Motion 열기를 그대로 사용한다.

## G02. 외곽불

기존 repair 도구에 구조가 확인된 WINT1.1/1.2의 mesh 추가 속성 버전을 허용하되 WMOD section·WMA2 layout·slot/material/diffuse/잘못된 emissive의 exact identity 검사를 유지한다. 세 후보에서 지정된520byte 필드 외 변화가 없는지, 재실행이 변경을 만들지 않는지 확인한다. diffuse/normal/specular·geometry·UV·배치와 사용자가 조절한 scale은 보존한다. apply는 사전 검증·backup·CAS·원자 교체와 실패 rollback을 사용한다.

## G03. 검증 경계

actual WModel reader와 기존 원본 변환 도구의 CPU 검증을 사용한다. 입력·후보·키 비교 결과와 설치 여부를 RESULT에 분리한다. Client/UI 실행과 화면 캡처는 수행하지 않으며 머리·다리와 불의 최종 시각 결과는 사용자 확인 대상이다. 새 C++ 파일과 project/filter 등록은 없다.
