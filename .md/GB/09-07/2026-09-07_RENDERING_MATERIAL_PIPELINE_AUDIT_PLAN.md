# Rendering Material Pipeline 전수 조사 계획

작성일: 2026-09-07

사용자 요청: Character Select와 KoukuSaydon에서 느끼는 원작 대비 탈색감을 출발점으로,
맵·캐릭터·이펙트 전체의 리소스, 재질 입력, 셰이더, 조명, 후처리를 조사하고 튜닝 방향을 정리한다.

이 문서는 조사 계획이다. 제품 구현이나 현재 미술 값 변경을 제안하는 전체 코드 계획서가 아니다.
실측 결과와 실제 검증은 대응 `2026-09-07_RENDERING_MATERIAL_PIPELINE_AUDIT_RESULT.md`가 소유한다.

## G00. 현재 조사 기준과 파일 소유권

- 시작 branch: `codex/kouku-gate-pattern-bundles`
- 시작 HEAD: `13fa34d7df9f37d1c697ddefe1e5e7aebfdafef9`
- 다른 작업의 C++·JSON·문서 미커밋 변경이 있는 공유 작업 폴더다.
- 현재 working copy를 조사하고, HEAD만의 상태 또는 원작 전체의 동일성으로 보고하지 않는다.
- 제품 C++/HLSL, Data 정본, runtime 생성물, Resources를 수정하지 않는다.
- 새 문서는 이 PLAN과 대응 RESULT이며, 일회성 조사 코드·전수 행별 JSON은
  `out/RenderingAudit20260907/`에만 둔다. 새 C++ 파일과 project/filter 등록은 없다.
- 원본 package 전체 재추출이나 새로운 영구 validator 체계는 만들지 않는다.

## G01. CModel과 CMaterial 입력 조사

기존 map catalog와 설치된 Map/Character/Deploy WModel을 구분하여 조사한다.
Effect WModel은 Effect 조사에 포함한다. 모델 metadata를 읽고 다음을 기록한다.

- catalog가 참조하는 모델과 물리 폴더에만 있는 모델.
- 실제 submesh가 사용하는 material과 보관만 된 material table row.
- Diffuse/Normal/Specular/Emissive/Opacity/ORM/Metallic/Roughness/AO와 염색 입력.
- 빈 슬롯, 선언된 파일 부재, decode 실패를 각각 분리.
- DDS 포맷, mip 개수, sRGB 표기와 실제 로더 정책.
- 현재 material profile과 shader가 소비하는 입력·채널·상수.

## G02. Effect 입력과 shader 프로그램 조사

V1과 V2의 catalog, authored 문서, 실제 binding/consumer를 따라간다.
저작 문서 수와 활성 참조 수를 구분하고, generic/typed/원본 프로그램 자료와 실제 실행 경로를
서로 다른 상태로 기록한다. Effect 버전 번호만으로 원본 재현 정도를 판단하지 않는다.
시간·geometry·texture·sampler·constant·blend·scene input이 픽셀 계산과 만나는 위치를 확인한다.

## G03. Renderer, 광원과 색 출력 조사

- Engine/Client의 shader source·include·project producer 전수 목록.
- 일반 표면, instancing, animation, water/sky/translucency, Effect, preview/cutin/UI 경로.
- G-buffer 채널과 포맷, shadow/SSAO, 광원 합산, HDR/왜곡/화면 효과/Bloom/Final 순서.
- 모든 scene profile과 map/anchor light의 저장값, source/runtime 일치, 실제 선택 경로.
- Workbench 조절·Save·Publish·Reload 범위와 Benchmark가 측정하는 값.
- 원본 조명·재질·후처리 자료의 보유 여부와 제품에 운반되지 않은 입력.
- 기존 Debug/Release CSO 존재 및 복사본 일치. 이 검사를 재컴파일·실행 성공으로 부르지 않는다.

## G04. 튜닝 방향과 확인 순서

조사 결과를 다음 실행 순서에 연결한다.

1. 같은 카메라·해상도·장면/패턴 시각·프로필을 고정한다.
2. 공통 색 공간과 HDR/Final clipping을 분리한다.
3. 원본 material/channel/parameter 차이가 확인된 대표 표면을 복원한다.
4. 광원·반사·그림자·간접광과 Effect의 발광/coverage/합성을 맞춘다.
5. tone mapping, 색보정, Bloom, anti-aliasing을 정리한다.
6. 기존 Benchmark로 성능을 별도로 확인한다.

RESULT에는 각 단계에서 기존 Tool로 가능한 조작, 제품 구현이 필요한 소비 경계,
완료를 판정할 수치와 사용자의 시각 관찰을 나눠 적는다. 시험용 수치를 원작 정답으로 제시하지 않는다.

## G05. 조사 종료 검증

- 기록한 분모의 모든 파일이 성공 또는 명시된 오류 행으로 집계됐는지 확인한다.
- 생성한 JSON을 다시 parse하고 합계·고유 집합·오류 개수를 대조한다.
- 조사 핵심 입력의 hash 변화가 있으면 변경된 범위를 다시 읽어 보고서를 갱신한다.
- 새 Markdown 링크·전수 JSON과 `git diff --check`를 확인한다.
- 제품 소스 변경이 없으므로 컴파일은 하지 않는다. Client/UI 실행·조작·캡처도 하지 않는다.
- 정적 소비 경로 확인, GPU 실행 확인, 원본 동일성, 사용자 visual 판정을 분리한다.

LAN 시작 설정은 `server-host`, TCP 7777 LocalSubnet ready, endpoint `not-listening`이었다.
실제 화면 확인의 시작 대상은 사용자가 `Ctrl+F5`로 실행하는 `Server + Client` profile이다.
