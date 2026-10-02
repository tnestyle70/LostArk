# World Sequence Debug·Release 공통 샘플 재사용 결과

2026-10-03 PR 통합 확인: 아래 기록 이후 정식 Debug Product가 2026-10-02 17:40 KST에 PASS했다.
`out/BuildPipeline/runs/20261002T084006819Z-debug-product.json`과 외부
`Build-Debug-GuideNavMovie-Exit.json`에서 제품 링크·배포와 source 검증 성공을 확인했다.
현재 입력47개도 해당 검토본 SHA와 모두 일치한다. 아래의 Debug 전체 링크 대기는 해소됐으며
Release Product는 아직 실행 중이다. 사용자 parts/attachments·화면·FPS 확인은 별도로 남아 있다.

## G01. 구현과 적용 범위

Bone.cpp의 Debug /O2는 Debug 컴파일 설정 보완이었다. Release는 이미 최적화하여 그 설정을
복사할 이유가 없고, 같은 골격 채널을 반복 보간하는 구조적 비용은 그대로였다.
이번5파일 변경은 `_DEBUG` guard 없이 Debug/Release 공통 경로에 적용했다.

WorldSequenceObject가 준비하는 CModel clone만 Enable_AnimationSampleReuse를 호출한다.
immutable cooked channel 입력의 bone index·순서·모든 key scalar bit가 정확히 같은 경우에만
마지막 같은 시각의 local matrix를 재사용한다. hash 충돌은 exact 비교로 분리한다.
legacy Assimp merged-key와 일반 gameplay Character는 opt-in하지 않는다.

각 clone의 clock/loop/finished, unkeyed bones, blend/root motion, preScale, combined matrix,
skin palette invalidation, material과 parts는 기존 소유자에서 그대로 실행한다.
재질·모델 전체·mutable pose를 서로 공유하지 않는다. 재사용 시 channel interpolation만 생략한다.
registry는 weak owner, memo는 live channel 집합당 한 시각·고정 matrix vector이며 mutex로 보호한다.
실행 frame에서 새 matrix 저장 공간을 할당하지 않는다. 최초 cursor 준비는 기존 lazy 할당을 유지한다.

## G02. 실제 수치 검증

production Animation.cpp/Channel.cpp/Bone.cpp와 CMesh::Build_SkinPalette를 실제 DirectXMath로
컴파일했다. app/profiler plumbing만 대역이며 header private은 검사 접근을 위해 public으로 열었다.
도화가 `pc_sp_00_sk_12246.material-00.wmodel`237본과 차원술사
`pc_swp_m_hr_00_helmet_sk_12266.material-00.wmodel`175본, 두 모델의 intro/loop4개를 사용했다.
원본 모델 파일과 테스트 입력 SHA는 각 result.json에 보존한다.

검사: 임의/역방향·반복·끝·loop 시각, pool reset, clone별 clock, 다른 key/순서/index/count,
unkeyed bone 보존, local 후처리의 cache 오염 차단, owner 파괴,4thread 동시 sampling,
실제 WModel inverse-bind를 사용한 skin palette의 bit-exact 일치.

| 구성 | 결과 | 근거 |
|---|---|---|
| v143-Debug | 2,314,896 checks PASS | compiler 194435228, cache hits 27,797 |
| v143-Release | 2,314,896 checks PASS | compiler 194435228, cache hits 27,812 |
| v143-Release-Collision | 2,314,896 checks PASS | compiler 194435228, cache hits 27,810 |

최종 근거는 `out/MovieSampleReuseRegression/v143-*`이다. 앞선 기본 vcvars probe는14.51(v145)이었으므로
보존하되 제품 toolchain 검증 근거로 쓰지 않는다. 최종 v143는14.44, SDK10.0.26100.0,
Debug /O2 /MDd /Zi /D_DEBUG, Release /O2 /MD /DNDEBUG를 명시했다.
Debug /O2는 실제 Animation/Channel과 이번 Bone item 설정에 맞춘 것이다.
기존 Assimp 경로의 double→float 경고 및 throw형 MSG_BOX 대역 뒤 unreachable 경고는 기능 실패가 아니다.

## G03. 측정 범위와 아직 확인하지 않은 것

10actor 동일clip의 interpolation+combined CPU microbenchmark는 runner result의
tenActorSampleAndCombineMs에 저장한다. 캐시 준비와 파일 로드는 timer 밖이며3회 median이다.
렌더·GPU·driver·lighting·material·effect·CModel blend/attachment/skin palette bind는 timer에 없다.
이 결과를 실제 FPS 또는 전체 Movie 비용 감소율로 바꾸지 않는다. PC의 동시 실행 앱/클럭 영향도 있다.


| v143 Release, 동일10actor pose sample+combine | 기존 ms | 재사용 ms |
|---|---:|---:|
| 도화가 intro | 0.370710 | 0.057861 |
| 도화가 loop | 0.237861 | 0.048368 |
| 차원술사 intro | 0.243868 | 0.041032 |
| 차원술사 loop | 0.168707 | 0.035801 |

정상 프로젝트 MSBuild `/t:ClCompile`로 Animation.cpp, Model.cpp, Bone.cpp,
WorldSequenceObject.cpp, ClassSelectionPresentation.cpp를 Debug/Release 각각 실제 컴파일했다.
총 10개 TU가 오류 0으로 통과했다. v143 14.44, x64 host, 정상 프로젝트 PCH와 item 설정을 사용했다.
Client의 기존 PrepareEngineSdk target이 header를 export했고 Animation.h/Model.h의 SDK SHA가
source와 일치한다. 기존 C4819 문자 코드 페이지 경고, C4244/C4267 축소 변환 경고는 남아 있다.

각 호출은 단일 SelectedFiles와 BuildProjectReferences=false를 사용했다. 첫 다중 escaped
SelectedFiles 시도는 SDK/PCH 준비만 수행했으므로 TU 컴파일 성공에 포함하지 않는다.
최종 근거는 Recovery 폴더의 Movie-Integrated-ClCompile-Selected-Receipt.json과 10개 log다.
검사 전후 Engine/Client/Server Debug·Release의 EXE/DLL/PDB/LIB/CSO 298개는 SHA256·크기·시각이
모두 같았다. 런타임 링크/배포, shader compile, 사용자 앱 재실행은 하지 않았다.

전체 dependency Product compile/link와 실제 Client parts/attachments·그림·FPS는 아직 확인하지
않았다. root가 Guide/Nav와 통합한 정상 Product build를 별도 수행한다.
사용자 실제 재생과 새 캡처 전에는 Movie 문제 전체가 해결됐다고 기록하지 않는다.
GPU Lights9.5ms 등 이전 측정의 다른 병목은 이번 CPU 샘플 재사용 범위 밖이다.

## G04. 변경·검증 위치

- [전체 코드 PLAN](2026-10-02_MOVIE_ANIMATION_SAMPLE_REUSE_PLAN.md)
- [이전 실측 분석](2026-10-02_CLASS_MOVIE_PERFORMANCE_RESULT.md)
- 실행 도구: `Tools/CharacterSelectPipeline/test_movie_animation_sample_reuse.py`
- 외부 원본5파일: `Desktop/LostArkTransfer/Sync-20261002/Recovery-20261002-1315/MovieSampleReuse-Candidate/before`
- 외부 최종 영수증: 같은 Recovery 폴더의 `Movie-Animation-Sample-Reuse-Verification.json`

기존5파일의 UTF-8 BOM 없음·CRLF를 보존했고 PLAN fenced code와 실제 byte를 대조했다.
새 production C++ 파일이 없어 project/filter 등록 변경은 없다. Data/Resources·rendering option,
기존 ClassMovie 계측3개와 Bone 프로젝트 설정, 사용자 실행 프로세스를 수정하지 않았다.
commit/push·Client/UI 실행·전체 publish는 하지 않았다.
