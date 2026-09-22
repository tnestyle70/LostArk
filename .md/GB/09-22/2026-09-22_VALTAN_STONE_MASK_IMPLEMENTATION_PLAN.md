# 발탄 복원 돌의 유지 구간 마스크와 크기 교정

## G00. 현재 재질과 실제 소비자

`effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_stonewave_01_01/02`와
`effect.valtan.source.fx_mn_rpbf_00_s.par_s_rpbf_stone_01_1`의 돌은 native2391과
설치된 `Effect/KoukuSaydon/FullRestore/Meshes/fm_d_stoneparts_003.wmodel`을 사용한다.
같은 재질은 Product의 땅구르기·피자·발악 active 돌에도 연결되어 있다. 십자 Product의
기존 네 방향 생성기에 이 재질을 연결하는 범위는 `2026-09-22_VALTAN_STONE_PRODUCT_IMPLEMENTATION_PLAN.md`가 소유한다.

현재 native2391은 Dynamic.X를 최대 1로 제한하고, UV의 radial mask와 실제 noise,
0.1 및 `09.gap_offset=0.175`를 합쳐 0.3333 미만이면 픽셀을 버린다. 유지 구간에도
낮은 noise 영역의 최솟값이 0.275가 되어 구멍이 남을 수 있다. 원본 shader 전체의
discard를 없애지 않고 해당 돌 occurrence의 저작 scalar로 보정한다.

## G01. 데이터 후보

실제 native 함수와 설치 DDS의 검증을 거쳐 해당 돌의 `09.gap_offset`을 0.25로 올린다.
`material.sourceProfile.scalars`에 유효값을 기록하고 `authoringOverrides.scalars`에
원본 compilerValue와 교정값을 함께 보존한다. 복원 재질 전체의 기본값 변경으로 기록하지 않는다.
유지 구간의 최저 마스크는 0.35가 된다. 수명 말기 Dynamic.X와 alpha의 실제 결과를
검사하고 원래 소멸 기능은 유지한다.

세 source 문서의 native2391 기둥 돌 6개는 기존 detail transform scale에 정확히 1.2를 곱한다.
같은 shader를 사용하지만 다른 mesh인 파편 2개는 변경하지 않는다.
Product의 네 십자 돌과 세 stationary 돌에도 기존 크기 기준 1.2를 적용한다. particle
시작 위치·속도·발생 간격·충돌·Server cover 반경에는 배율을 곱하지 않는다.

기존 데이터의 SHA256와 원문은 out 아래에 백업하고 후보만 준비한다. 최종 저장본
기준 승인을 받은 뒤 stable element ID와 scalar name으로 최신 디스크에 병합한다.
서로 다른 필드의 편집을 보존하며 같은 필드 충돌은 임의로 덮지 않는다. 교체 직전 hash를
재확인하고 원자적 교체·실패 시 자기 변경 rollback을 적용한다.

## G02. 검증과 완료 경계

실제 설치 WModel의 UV와 DDS를 이용해 native2391의 유지/소멸 구간을 headless WARP로
비교한다. 상수 텍스처 중심점만의 검증으로 돌 전체를 판정하지 않는다. 후보 JSON의 실제
codec load/validate와 재생, mesh transform의 1.2배, 기존 생성 위치/수명을 검사한다.
관련 TU와 제품 컴파일 및 git diff --check를 확인한다.

Client/UI를 자동 실행하지 않는다. 실행 중 도구의 미저장 draft와 디스크 반영을 구분하고,
제품 링크 및 사용자의 재입장 화면 확인은 별도 완료 상태로 기록한다. 새 C++ 파일이나
Resources 파일은 추가하지 않으므로 vcxproj/filters 등록 변경은 없다.
