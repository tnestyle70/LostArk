# 쿠크 포커판 텍스처 AddressU 복구 결과

## G01. 구현 상태

원본 package `542N9YJ2RWOUWUGYNOH2R4FT.upk`의 포커판 Texture2D 세 개는
`AddressX=TA_Mirror`다. 기존 diffuse Wrap sampler가 이 설정을 버렸고, U가 1을 넘는
반쪽 바닥과 테두리에서 다른 픽셀을 읽었다. 원본 glTF와 WModel의 위치·UV·index를 비교했으며
해당 거대 삼각형에 별도 geometry 손상이나 diffuse 누락은 없었다.

`LV_LUT_MIDNIGHTC_ED.mapmaterials.json`에 포커판 asset의 SLOT_000/001/002 세 행을
`family=diffuse-sampler`, `addressU=MIRROR`로 추가했다. 원본 material/texture object 경로를
근거로 보존한다. 기존 두 surface 행과 Resources texture·WModel·UV는 바꾸지 않았다.

MapAssetCatalog와 Map publisher가 행의 정확한 필드·enum·stable asset/material identity를 검사한다.
CModel은 기존 named override join에서 diffuse 주소 방식만 변경하고 CMaterial이 매 diffuse bind에
flag를 바인딩한다. static/mapinstance diffuse와 shadow alpha는 공용 MirrorU sampler를 사용한다.
non-opted 재질은 기존 sampler를 유지하며 diffuse override draw도 shared flag를 초기화한다.
sampler-only 행은 asset 그림자 정책에 영향을 주지 않는다. 별도 모델/runtime이나 신규 C++ 파일은 없다.
공유 파일의 Character Material mip·PBR emissive 변경은 담당자 완료 인계 뒤 국소 수정하여 보존했다.

## G02. 수행한 확인

- 원본 package 직접 파싱: Texture2D 세 행 모두 `addressx=ta_mirror`, native material 경로 확인.
- 기존 WModel material reader를 사용한 source 문서 검증: 기존 두 행과 새 세 행의 unique named join 통과.
- Map publisher PowerShell AST parse 통과. publisher 본체/배포는 실행하지 않고 기존 reader 함수만 호출했다.
- 지원하지 않는 `CLAMP`, 중복 key, 실제 WModel에 없는 이름, 허용하지 않은 추가 field가 모두 거절됐다.
- 변경 범위 `git diff --check` 통과. 기존 C++·shader·publisher의 UTF-8/CRLF를 유지했다.
- 면적 가중 CPU UV 50,000표본에서 반쪽 바닥의 여백 녹색 비중은 Wrap 28.748%, Mirror 0.15%였다.
  이는 texture/UV 진단이며 사용자 화면의 visual PASS는 아니다.
- 진단 근거는 Git 제외 `out/KoukuPokerFloor20260908/sampler-evidence.json`과
  `source-address-evidence.json`에 기록했다. runtime 생성물은 직접 편집하지 않았다.

## G03. 통합 담당자와 사용자 확인 경계

변경 C++와 shader의 최소 컴파일, 해당 Area Map publish, Client 재시작은 통합 담당자에게 인계한다.
이 하위 작업은 빌드·publish·Client/UI 실행·GPU 화면 캡처를 수행하지 않았다.
최종 포커판 무늬와 오른쪽 테두리가 정상인지 사용자가 아레나에서 확인해야 한다.
리소스 binary는 새로 만들지 않았으므로 추가 Drive payload 전달은 없다.

## G04. 통합 빌드·배포와 사용자 입장 오류 진단 (2026-09-08)

쿠크 Map publish3231 placement/8파일과 Character Select Map publish803 placement/4파일이
통과했다. static/instance/skinned shader3개 fxc 컴파일과 최종 Debug Product의 Engine/Shared/
Server/Client compile·link·deploy가 PASS/exit0다. 최종 receipt는
`out/BuildPipeline/runs/20260908T062920624Z-debug-product.json`이다.

사용자 중간 실행의 map catalog 실패는13:46 구Client가14:57 새 Mirror/emissive 재질 입력을
읽은 불일치였다. 현재 CMapAssetCatalog의 runtime/source 직접 로드는 쿠크323개·CS63개 모두
PASS였고, 최종EXE에는 새 parser 문자열이 포함됐다. 이 검사에서 GPU/Device/GameInstance/UI는
실행하지 않았다. 근거는 `out/KoukuJoker20260908/catalog-only-receipt.json`과
`old-client-map-parser-evidence.json`이다. 새EXE의 실제 재입장과 포커판 무늬·중앙 링 화면은
사용자가 확인해야 하며 visual PASS는 기록하지 않았다.
