# 원본 시퀀스 정적 소품의 반사 변형 계획

## G00. 입력과 소유 경계

1관문 통합 시퀀스의 원본 정적 소품 94개는 24개 WModel과 26개 `(modelAssetId, reflectionSigns)` 조합을 사용한다. 설치본은 모두 WINT 1.2, WMOD의 mesh/material 두 section, bone/animation 0인 정적 모델이다. 기존 `cook_wmodel_geometry_contract.parse_geometry_wmodel`로 24개가 모두 통과하며 sourceToWModelScale 100, geometryPreScale 0.01, evidenceFlags 0x47fd다. 부호는 X 반사, Z 반사, X/Z 두 축 반사다. 음수 scaleMultiplier를 양수로만 바꾸면 원본 모양이 달라진다.

`build_source_sequences.py` 담당자가 요청 목록과 WorldSequence 후보를 소유한다. 이 작업은 새 `Tools/KoukuSaydonPipeline/bake_reflected_static_props.py`에 WModel 반사와 검증을 모은다. 기존 Resources 입력, 다른 저작 JSON, C++ parser 및 runtime scale 검증은 변경하지 않는다. 새 C++ 파일과 project/filter 등록은 없다.

## G01. 모델의 반사와 재질 보존

정점 P와 normal N, tangent XYZ에 대각 반사 F를 적용한다. tangent W에는 det(F)를 곱하고 det(F)가 음수일 때만 각 삼각형의 두 번째/세 번째 index를 교환한다. UV0/UV1/선택 UV2, COLOR0, material index/hash/name, vertex/index 수와 순서는 보존한다. bounds의 min/max/center를 반사하고 radius는 보존한다. 기존 geometry metadata의 입력 근거는 유지하되 geometry payload digest, 이번 geometry tool digest, metadata identity를 다시 계산한다. 원본 UPK fidelity 승인으로 승격하지 않는다.

새 파일명은 원본 디렉터리 안의 `<stem>__reflect_npp.wmodel`, `__reflect_ppn.wmodel`, `__reflect_npn.wmodel`이다. 부모 디렉터리를 유지하므로 내장 `textures/...` 상대 경로와 material section의 모든 byte가 유지된다. 먼저 out 아래 Resources-relative 후보를 만든다. 이미 존재하는 출력은 byte가 같을 때만 재사용하고 다르면 실패한다. 설치도 같은 규칙으로 신규 경로만 허용한다.

## G02. 시퀀스 담당자가 소비하는 계약

helper는 원본 asset ID, 세 축 부호, Resources root와 후보 root를 받아 검증한 새 asset ID와 수치 검증 정보를 반환한다. 요청 집계 CLI는 26개 mapping과 94개 occurrence 연결을 out JSON으로 기록한다. 각 요청의 scale 부호와 reflectionSigns 일치를 요구한다. 호출자는 원본 object resource를 `.reflect.npp/ppn/npn` ID로 복제해 원래 mapMaterialBindings와 preScale을 유지하고 해당 instance/slot만 새 resource로 바꾼다. 해당 track key의 scale은 절댓값으로 저장한다. 위치/회전과 원래 object resource는 유지한다.

## G03. 종료 증거

24개 원본과 26개 후보를 기존 Python strict geometry parser로 읽는다. 모든 vertex의 반사 위치/normal/tangent, tangent handedness, UV/color byte, parity별 winding, bounds, material section byte와 상대 texture 경로를 비교한다. 양수 TRS와 반사 geometry의 곱이 원래 signed TRS와 일치함을 94개 occurrence의 모든 정점에서 수치로 확인한다. 기존 WModelGeometryContractHarness의 actual Engine decoder로 후보 입력을 검사할 수 있으면 그 결과를 별도로 기록한다. 제품 빌드, Client/UI 실행·캡처와 화면 판정은 하지 않는다. SOURCE와 후보 hash 및 실제 실행한 검사만 RESULT에 기록한다.
