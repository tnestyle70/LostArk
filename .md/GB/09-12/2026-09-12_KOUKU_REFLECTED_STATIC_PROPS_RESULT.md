# 원본 시퀀스 정적 소품 반사 변형 결과

## G00. 반영한 파일과 데이터

`Tools/KoukuSaydonPipeline/bake_reflected_static_props.py`를 추가했다. 기존 strict geometry parser와 WMA2 parser를 재사용해 기존 WINT 1.2 정적 모델 24개로부터 26개 반사 변형을 만들었다. 변형은 25개 홀수 반사와 1개 두 축 반사이며 94개 원본 배치가 소비한다. 후보 전체는 1,976정점·5,262index다.

새 모델은 원본과 같은 Resources 디렉터리의 `<stem>__reflect_npp.wmodel`, `__reflect_ppn.wmodel`, `__reflect_npn.wmodel`이다. 정확한 source/variant asset ID와 94개 instance/slot 대응은 `out/Reflect/mapping.json`에 있다. 시퀀스 담당자가 `write_new_or_equal`로 신규 Resources 26개를 설치했고 모든 설치 SHA가 후보와 같음을 재검사했다. 기존 원본 24개의 SHA는 유지됐다. binary는 Git에 추가하지 않았다.

시퀀스 generator 담당자는 반사 모델 resource의 mapMaterialBindings와 preScale을 원본에서 유지하고 94개 해당 track scale을 양수로 바꿨다. MapTool target 중복 제약 때문에 최종 배경 266개 actor resource에는 `.actor.<sourceExport>` 고유 ID가 추가된다. 이 helper의 `reflectedObjectId`는 그 actor suffix 이전의 제안 ID다. 최종 WorldSequence/Composition 후보와 Map publish 결과는 별도 `KOUKU_SOURCE_SEQUENCE_RESTORE` 결과가 소유한다.

## G01. geometry와 재질의 보존

정점·normal·tangent XYZ에 축별 부호를 적용하고 tangent W에는 determinant를 곱했다. determinant가 음수인 모델만 triangle index 1/2를 교환한다. AABB min/max/center는 반사하고 radius는 유지한다. UV0/UV1, COLOR0, submesh 이름·material index/hash, material section 전체 byte는 유지한다. WGEO payload SHA, geometry tool SHA, metadata identity만 해당 역할에 맞게 갱신한다. source GLTF/package/receipt 근거와 sourceToWModelScale 100, geometryPreScale 0.01, evidenceFlags 0x47fd는 유지하며 원본 UPK visual fidelity 승인으로 승격하지 않았다.

모든 비어 있지 않은 WMA2 입력을 기존 `WMaterialReader::ResolveTexturePath/ResolveBelowAssetRoot` 순서와 대조했다. 일반 `textures/...`와 일부 기존 `Resource/LostArk/...` relocation 문자열을 byte 그대로 보존하며 103개 고유 물리 입력이 존재한다. 새 model 부모 디렉터리가 같으므로 새 texture 복사나 material 경로 재작성은 없다.

## G02. 실제 실행한 검사

- `python -B Tools/KoukuSaydonPipeline/bake_reflected_static_props.py --requests out/KoukuSourceSequenceRestore20260912/reflected_prop_requests.json --output-root out/Reflect/Resources --mapping out/Reflect/mapping.json`: 26 variant/94 occurrence PASS. 재실행도 동일 byte를 재사용했다.
- 기존 Python strict parser로 원본과 후보의 구조·geometry metadata SHA·index 범위·finite/basis·bounds 검사 PASS. 94개 배치 모든 정점의 `원본 P × signedScale == 반사 P × abs(scale)` 최대 오차 0.
- 실제 `Engine/Private/BinaryAsset` 7개 CPP와 기존 `WModelGeometryContractHarness.cpp`를 `/MP2`로 out에만 컴파일·링크했다. `out/Reflect/wmodel_decoder.exe --dump-candidate` 26개 PASS. position, UV0/UV1, tangent W, index, bounds, channel/evidence flags와 metadata digest를 Python 해석 결과와 비교했다.
- Engine reader는 normal/tangent를 정규화하므로 raw input SHA 대신 별도 out CPU probe에서 실제 source/설치본을 함께 decode했다. 모든 정점의 position/normal/tangent/binormal 반사 최대 오차 0, tangent W·UV·color·winding 보존 PASS. 실제 diffuse/normal/specular/emissive/opacity 해석 경로 120개가 source/variant에서 동일하고 존재한다.
- invalid reflection 입력 4종 거부, 같은 출력 byte 재사용, 다른 출력 byte 교체 거부 및 기존 byte 보존 PASS. 원본 24개 SHA와 설치 26개 SHA를 재검사했다.
- helper AST, mapping/verification/basis JSON parse 및 `git diff --check` PASS. 컴파일의 기존 `Engine_Function.h` CP949 주석 경고 C4828은 있었고 새 compile error는 없다.

증거는 `out/Reflect/verification.json`, `basis_verification.json`, `verify.py`, `basis_probe.cpp`, `decoder.rsp`, `basis.rsp`, `compile.log`, `basis_compile.log`에 있다. 사전 생성되어 있던 Tools harness exe가 원본도 거부해 현재 Engine source로 out 실행 파일을 새로 만들었다. 첫 후보 폴더는 긴 경로의 Windows hardlink 제한에 걸려 짧은 `out/Reflect`로 바꿨다. 중간 후보를 runtime 입력으로 쓰지 않았다.

## G03. 보존 경계와 남은 확인

helper와 기존 geometry tool은 tool 파일 SHA를 metadata에 기록한다. helper가 바뀐 뒤 이미 존재하는 variant byte와 달라지면 덮어쓰지 않고 거부한다. 새 의미의 변환은 별도 variant asset ID/version으로 진행해야 한다. Source Sequence 담당자가 추가한 actor별 resource ID를 기존 배경이나 양수 scale actor의 공유 resource에 덮어씌우면 안 된다.

새 C++ 제품 파일·런타임 cache·negative scale 허용 예외는 추가하지 않았다. 제품 빌드, EngineSDK/DLL/CSO 배포, Client/UI 실행·조작·캡처, GPU와 화면 fidelity 판정은 하지 않았다. 사용자의 실행 화면 확인은 별도다.
