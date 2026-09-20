# 쿠크 보스 원본 노멀·탄젠트 복구 구현 계획

## G00. WINT 1.5 skinned tangent handedness

기존 설치 보스의 정점·삼각형·UV와 원본 glTF의 대응을 유지하고 원본 노멀·탄젠트 basis를 복구한다. 기존 skinned 정점 76바이트에는 tangent handedness가 없으므로, 새 복구 산출물만 WINT minor 5와 80바이트 정점으로 명시적으로 구분한다.

- `Engine/Private/BinaryAsset/Winters/WFormatTypes.h`: minor 5 및 stride 80 상수를 추가한다. 기존 76바이트 뒤 offset 76의 float 하나가 handedness이며 flags는 `VF_STATIC_BASE | VF_BONE_WEIGHT | VF_TANGENT_HANDEDNESS`만 허용한다. 추가 UV·Color는 이 버전에 포함하지 않는다.
- `WMeshReader.cpp`: 새 버전에만 finite·nonzero N/T/cross와 ±1 handedness를 검사하고 정규화한 cross(N,T)에 sign을 곱해 binormal을 복구한다. 1.0~1.4의 stride·flag·basis 처리와 fallback은 유지한다.
- `Engine/Private/BinaryAsset/WModelDecoder.cpp`: package/animation-catalog header가 minor 5를 받아들이게 하며 기존 outer WMOD/inner WMSH version 일치와 전체 decode rollback을 유지한다.
- 기존 `WModelGeometryContractHarness.cpp`에 새 형식의 positive/negative 검사만 추가한다. 신규 C++ 파일과 project/filter 등록은 없다. root가 converter·실제 원본 대응 검증 및 통합 빌드를 소유한다. Client 실행과 화면 판정은 사용자 경계로 남긴다.
- Converter는 기존 정점 슬롯과 index를 유지하고, 같은 기존 정점에 실제로 다른 원본 basis가 연결될 때만 새 슬롯을 추가한다. 새 형식의 Python parser도 N/T의 개별 길이 및 정규화한 cross의 퇴화 여부를 검사해 native decoder와 같은 경계를 소비한다. 기존 후보를 수정할 때는 새 경로에서 재생성·검증한 뒤 기존 후보 hash를 재확인하고 원자적으로 교체한다.
- Native corrupt fixture는 기존 frozen golden bytes를 baseline으로 사용한다. 현재 cooker의 자기 파일 hash가 담긴 새 cook 결과를 고정 hash baseline으로 잘못 사용하는 문제를 제거하며, C++의 baseline/31개 corruption identity와 오류 판정은 완화하지 않는다. 현재 writer의 회귀는 기존 Python round-trip 검증으로 별도 확인한다.
