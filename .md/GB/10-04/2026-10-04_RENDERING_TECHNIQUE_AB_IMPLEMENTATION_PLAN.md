# Rendering Workbench 기법 선택과 실제 A/B 연결 구현 계획

## G00. 현재 실측과 범위

기준은 main `4cf12e1259`이다. 현존 SSAO·SSGI·SSR·PCF·RNM·IBL·후처리는 renderer와 shader에 연결돼 있다. 기법 사전은 읽기 전용이고 41개 세션 필드와 31개 recipe를 직접 열지 못한다. 옛 `Selected Reference A/B Start`는 품질 draft를 초기화하고 FXAA를 켜므로 이름과 동작이 다르다.

Character Select의 굴곡 음영은 SSAO, 표면 요철은 PBR normal, 구워진 명암은 RNM이다. 원본 SH·cube·BRDF 간접광 복원과 source postprocess는 실제 구현돼 있지만 독립 세션 비교가 없다. SSGI·SSR은 MapPBR marker 3만 지원하며 marker 5 캐릭터로 무리하게 확장하지 않는다. 미구현 기법에 작동하지 않는 스위치를 만들지 않는다.

## G01. RenderingProfileService.h / .cpp

기존 whitelist 끝에 `SOURCE_PBR_INDIRECT`, `SOURCE_POST_PROCESS`를 추가한다. 기존 필드 순서와 저장 ID는 유지한다. 전자는 RenderEnvironment의 원본 간접광 선택, 후자는 SourcePostProcess의 tone·grading 묶음 선택이다. 순수 tone만 분리하는 기능으로 표시하지 않는다.

Read → validate → session stage → 현재 scene 입력 위 적용 → 소유 필드만 복원한다. 환경 cube·SH·색·리소스는 재생성하거나 저장하지 않는다. 다른 renderer 적용이 모두 성공한 뒤 환경 bool을 commit하며 실패 시 기존 transaction rollback을 사용한다. profile/region/level/video owner 변경은 기존 세션 종료 경계를 따른다.

## G02. RenderingBenchmark와 기법 사전

새 필드를 수치 UI·recipe·측정값에 연결한다. common fingerprint는 비교하는 enabled bit만 제외하고 나머지 source postprocess/environment 입력은 유지한다. 실제 기법의 선택은 stable recipe ID로 기존 Prepare_Recipe에 연결하고 현재 A 보관·B 준비·A/B 적용·종료를 명시적으로 조작한다. 실험 중 B 교체는 명시적 버튼으로만 수행한다.

SSAO/GTAO, height fog/volumetric, SSR/planar의 지원 표시를 분리한다. 원본 간접광 ON에서 억제되는 cube diffuse 근사와 LUT 없는 장면, MapPBR 수신면 범위를 설명한다. 복원 전 profile은 여러 옵션을 함께 바꾸는 비교임을 밝힌다.

## G03. MainApp 진입과 문서

기본값을 덮어쓰던 옛 A/B 버튼을 현재 상태를 보관하는 세션 진입으로 교체한다. SSAO 표시 이름을 명확하게 한다. RenderingProfiles, 사용자 저장 camera/composition/mapplacements 및 rendering tuning은 수정·publish하지 않는다. 새 C++ 파일이 없으므로 project/filter 추가는 없다.

## G04. 종료 검증

변경 TU와 Debug Product를 빌드하고 새로운 bool의 유효성·on/off·종료 복원·실패 rollback·무관한 환경 필드 보존을 실제 함수 기반으로 검증한다. recipe 및 fingerprint 연결을 독립 검토하고 git diff --check와 사용자 데이터 hash를 확인한다. Client/UI는 실행하지 않으며 최종 Character Select의 시각적 비교는 사용자 확인으로 남긴다. 검증 후 RESULT와 관련 사용서/반복 결함 문서를 갱신하고 별도 PR로 merge한다.
