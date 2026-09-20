# 쿠크 보스 원본 노멀·탄젠트 복구 결과

## 확인된 원인과 적용 범위

쿠크 보스·무기 5개 설치 WModel은 원본 LOD0과 삼각형 수가 같다. 저해상도 LOD를 선택한 것이 아니라, 설치본의 모든 유효 삼각형에서 정점 노멀이 면 법선 방향으로 바뀐 것이 확인됐다. 원본 glTF의 smooth normal·tangent를 실제 indexed corner에 대응해 복구했다. 이 변경은 보스 몸체·무기 표면의 조명 basis 복구이며 맵 밝기·바닥 specular·LUT 복구는 별도 결과를 따른다.

| 모델 | 원본 LOD0 / 설치 / 후보 삼각형 | 설치 / 후보 정점 | 골격 본 | 후보 tangent sign + / - |
|---|---:|---:|---:|---:|
| MN_RPCZ_00 | 10,210 / 10,210 / 10,210 | 30,106 / 30,106 | 103 | 13,208 / 16,898 |
| MN_RPCT_05 | 57,012 / 57,012 / 57,012 | 169,810 / 169,810 | 168 | 81,604 / 88,206 |
| MN_RPCT_06 | 27,772 / 27,772 / 27,772 | 82,833 / 82,833 | 84 | 39,397 / 43,436 |
| WP_MN_RPCT_05 | 5,130 / 5,130 / 5,130 | 15,172 / 15,172 | 5 | 5,544 / 9,628 |
| WP_MN_RPCT_06 | 4,386 / 4,386 / 4,386 | 9,622 / 9,622 | 8 | 4,361 / 5,261 |

합계 104,510삼각형 / 307,543정점이다. `-lods`로 원본을 추출했으며 5개 모두 추가 LOD 접미 파일 없이 LOD0 glTF 하나씩 생성됐다. 다른 숨겨진 원본이나 별도 모델까지 조사했다는 뜻은 아니다. 면 법선 감사는 `out/KoukuRenderingQuality20260919/boss-geometry-audit.json`에 있다.

## 원본 추출과 식별 근거

실제 성공한 명령은 다음과 같다. encrypted package basename을 직접 지정한 초기 시도는 실패했고, 논리 package 및 object 이름으로 추출했다.

```powershell
& 'C:/LostArkUModelP1C/runs/20260728T035441252Z-025d51a550a74a2b9e9729a1ad6313a6/input/tool/umodel_lostark_v7.exe' -export -gltf -lods -noanim -notex -nomorph -kr -game=lostark -path=C:/ProgramData/Smilegate/Games/LOSTARK/EFGame/ReleasePC -out=C:/Users/user/Desktop/LostArk/out/KoukuRenderingQuality20260919/OriginalMeshes MN_RPCZ_00 mn_rpcz_00_sk
```

나머지 4회는 마지막 두 인자만 각각 `MN_RPCT_05 mn_rpct_05_sk`, `MN_RPCT_06 mn_rpct_06_sk`, `WP_MN_RPCT_05 wp_mn_rpct_05_sk`, `WP_MN_RPCT_06 wp_mn_rpct_06_sk`로 바꿨다. 도구 SHA256은 `b9573cdcbb7e9d26dbf60a0e3af47fb5af8543140873da8483c26d58cf40b249`다. 추출물은 `out/KoukuRenderingQuality20260919/OriginalMeshes/<MODEL>/SkeletalMesh3/<lowercase_model>_sk.gltf` 및 같은 이름 `.bin`이다.

원본 package 디렉터리는 `C:/ProgramData/Smilegate/Games/LOSTARK/EFGame/ReleasePC/Packages`다.

| 모델 | package 파일 | package SHA256 |
|---|---|---|
| MN_RPCZ_00 | 9G1M8UBS1BZZE7JT4F64SSP.upk | fb87460ad33011c96ed24d13363e9341c31a3b47d2f83ca5d09975bb635be627 |
| MN_RPCT_05 | 9G1M8UBM1BZYE7JT4964SLP.upk | b988094f7fc3517913cc522db8e96a685f9b950499fdca14e25b8d7c4e3dd970 |
| MN_RPCT_06 | 9G1M8UBM1BZ5E7JT4964SYP.upk | 24b8266a08cea415a6ad679261fafd6553edf372079f4cf5218b6fdf117db979 |
| WP_MN_RPCT_05 | 8V2NAH2N9VCN2C0ZF8BEHR1C.upk | f74e8fc3825e56d133ef1de46da87274968a0561e527203ed03e93fc9254286e |
| WP_MN_RPCT_06 | 8V2NAH2N9VCN2C06F8BEHR1C.upk | 041be34374786b25dcc6c5d09a96d9f9646de4b2a4bbceeb50baaef904401e1f |

각 `<MODEL>-basis.json`은 package·glTF·buffer·입력 WModel·출력 WModel 경로와 SHA256을 모두 기록한다. 원본 glTF 노드 이름만으로 같은 모델이라고 간주하지 않고 모든 index corner의 위치와 UV를 확인했다.

## 구현과 보존 계약

`Tools/ModelAssetConverter/restore_skinned_source_basis.py`는 기존 WINT1.0 모델만 입력으로 받고 새 후보 경로에 기록한다. 원본 basis를 정규화하고 `(x,-z,-y)`로 변환하며, 역행렬식 좌표계에 맞춰 `handedness=-glTF.tangent.w`를 기록한다. glTF 삼각형의 corner 순서 `[2,1,0]`이 설치본과 전부 대응한다. source/installed 위치 대응 최대오차는 2.3842e-7, UV는 정확히 일치했다.

최초 후보가 first-encounter 순으로 정점을 재배열하는 것을 독립 감사에서 발견했다. 변환기를 기존 `old_index` 슬롯 유지로 보강했다. 실제 다른 basis가 한 기존 정점에 연결될 때만 별도 슬롯을 추가하도록 했으며, 이번 5개는 추가 슬롯이 0개다. 최종 후보는 index bytes, 정점 슬롯별 position 12bytes·UV0 8bytes·bone indices/weights 32bytes를 모두 원본 그대로 유지한다. material·skeleton·animation section 전체 bytes도 동일하다. mesh의 bone table·bounds tail은 그대로 복사한다.

기존 skinned76 형식에는 tangent handedness 저장 공간이 없어 음수인 정점도 `cross(N,T)`만 쓰고 있었다. `WFormatTypes.h`, `WMeshReader.cpp`, `WModelDecoder.cpp`에 opt-in WINT1.5를 추가했다. 기존 76bytes 뒤 offset76의 float sign을 붙인 stride80이며, flags는 정확히 `VF_STATIC_BASE | VF_BONE_WEIGHT | VF_TANGENT_HANDEDNESS`만 허용한다. 추가 UV·Color는 이 버전에 포함하지 않는다. outer WMOD와 inner WMSH minor가 다르면 전체 decode를 거부하고 staged asset을 rollback한다.

새 형식만 finite·nonzero normal/tangent/cross와 ±1 sign을 검사하고 `binormal=normalize(cross(normalize(N),normalize(T)))*sign`을 만든다. 1.0~1.4의 데이터 형식·legacy fallback·추가 UV 처리는 유지했다. GPU vertex layout 자체는 기존 `VTXANIMMESH`를 사용하며 파일 정점만 확장한다. 새 C++ 파일과 프로젝트 등록은 없다. 변경 C++는 기존 UTF-8/no-BOM 및 CRLF를 보존했다.

`cook_wmodel_geometry_contract.py::parse_skinned_uv_wmodel`과 `verify_dimensionmaster_summon_bind_pose.py::read_wmodel`도 stride80을 읽는다. 변환 parser는 normal/tangent 개별 길이와 정규화 후 cross를 검사한다. 이전 Python 검사에 `N=(1e-10,0,0), T=(0,1e10,0)`를 넣으면 raw cross가 1이라 통과하지만 native는 너무 작은 N을 거부하는 반례를 실제 재현해 수정했다. `G11-parser-negative-audit.json`에 반례 전/후와 9개 거부 결과를 기록했다.

## 후보와 검증 결과

최종 후보는 `out/KoukuRenderingQuality20260919/candidate/Client/Bin/Resources/Character/KoukuSaton/<MODEL>/<MODEL>.wmodel`이다. 새 `regenerated/<MODEL>.wmodel`에서 먼저 재생성·검증한 후 기존 후보 hash와 원본 input hash를 재확인하고 원자 교체했다. 이전 후보는 `regenerated/previous`에 보존했다. 후보 단계와 실제 Resources 배포는 아래처럼 구분해 수행했다.

| 검증 | 결과와 증거 |
|---|---|
| 원본 glTF와 후보 독립 대응 | 313,530 indexed corners 전체, N/T/sign 최대오차 2.98e-8. index 및 슬롯별 P/UV/skin bytes·nonmesh bytes 동일. `G11-basis-independent-audit.json` |
| Python parser negative | tiny N/large T, zero N, parallel basis, zero sign, Inf N, sign flag 누락, UV1 혼입, stride76, outer/inner mismatch 총9개 거부. `G11-parser-negative-audit.json` |
| 기존 Python 회귀 | `WModelGeometryContractTests` 12개 PASS, `WModelSelectiveReadTest` 11개 PASS. 수정 Python3파일 py_compile PASS |
| Release native 실제 후보 | root가 5개 후보 전체 307,543정점의 기록 sign과 decoded binormal 방향을 대조해 모두 PASS. 양수·음수 개수는 위 표와 같다 |
| Release native suite | 기존 validColor/noColor/tangent boundary/legacy static·skinned 및 새1.5 ±basis positive, 9개 negative·rollback PASS. frozen31corruption identity/rejection/rollback/error category 전부 PASS. wrapper exit0 |
| 컴파일 및 구조 | Release harness TU만 `BuildProjectReferences=false`로 재빌드 exit0. `wmodel-harness-build-release-final.log`, `wmodel-native-release-final.log`. 기존 Engine_Enum.h C4819 경고는 남음. 담당 diff --check PASS |

Native suite 초기 실패는 신규 모델 거부가 아니었다. fixture generator가 현재 cooker를 다시 실행하면서 자기 source SHA256과 metadata identity만 변경됐고, C++은 기존 frozen850bytes/hash를 요구했다. 현재 writer 출력과 golden의 차이는 offset698..729 및794..825의 두 digest였다. 테스트 generator가 기존 immutable golden의 길이·SHA256을 검증한 뒤 그 bytes에서 corruption을 만들게 수정했다. C++의 baseline 및31개 corruption hash를 바꾸거나 거부조건을 완화하지 않았다.

그 뒤 남은 2개 오류 category는 이미 지원하는 minor2를 여전히 "WINT header invalid"로 기대한 오래된 항목이었다. 같은 frozen corruption bytes가 inner2에서는 flags/stride 불일치, outer2에서는 outer/inner version 불일치로 거부되는 실제 경로로 expected string만 갱신했다. 최종 wrapper는 missing/rename/case-only rename/empty 및 golden hash/semantic mutation negative까지 모두 예상 exit를 만족했다.

## Product 빌드와 실제 Resources 배포

사용자가 저장·종료를 확인한 뒤 root가 정상 Product Release와 Debug를 모두 빌드해 PASS했다. 각각 `out/BuildPipeline/runs/20260919T084237404Z-release-product.json`, `20260919T084643387Z-debug-product.json`의 result가 PASS이며 skippedBuild=false다. Engine·Client의 새 decoder/preload 소스가 표준 Product 빌드에 포함됐다. 팀 배포 archive 생성은 수행하지 않았다.

그 뒤 root가 실제 `Client/Bin/Resources/Character/KoukuSaton/<MODEL>/<MODEL>.wmodel` 5개를 hash 재확인·백업·원자 교체했다. 설치 파일은 검증된 후보 hash와 같다. 원본 백업은 `out/KoukuRenderingQuality20260919/backup-boss/<MODEL>.wmodel`이며 `boss-install.receipt.json`이 before/after SHA256·설치 byte 수·backup 경로를 보유한다.

| 설치 모델 | 최종 SHA256 |
|---|---|
| MN_RPCZ_00 | a5a49dd5f8bde10c51ba24e50853702757276b90a90d44f846f901fae5aef034 |
| MN_RPCT_05 | 7d8a4ff9444e99dcd6e865c61afc6ace6ba6c91c3b6a50ca7b73389f7345f6bd |
| MN_RPCT_06 | c0bef6d9027b32beaa8fa7fc4f328a4a0da6fdb8dc2592cdf3bb91a40fbd4a0b |
| WP_MN_RPCT_05 | 4076c017fd737227dd8163f5a78e3a934641bc46e3b30bcc7f7dcb617c6af489 |
| WP_MN_RPCT_06 | 6ae91e9e161e5b2810613df8f2b06eeb256c180bef4359612da29d60bf2642fe |

Client 실행·도구 조작·GPU 화면 판정은 수행하지 않았다. 삼각형 수·basis·native decode·파일 설치 성공을 사용자의 최종 시각 품질 판정으로 대신하지 않는다. CSO closure의 별도 전체 검증은 관련 rendering/effect 후속 결과를 따른다.
