# 발탄 이펙트 트리와 게시 잠금 경합 수정 계획

## G00. 실측과 목표

사용자가 보고한 `canonical Product read admission failed; no unpinned fallback read was
attempted`는 Effect Tool의 공유 읽기 잠금 획득 실패 분기다. 같은 시각 Kouku owner의
Gameplay publisher가 발탄 writer 잠금을 잡고 있었다. 게시 종료 후 같은 Win32 공유 잠금은
성공했고 active-generation journal은 없다. 원자성 검사를 제거하지 않고 일시적인 게시
경합의 비용과 오류 표현을 줄인다.

## G01. Gameplay publisher의 독립 검증 순서

`Tools/GameplayPipeline/Publish-GameplayBalance.ps1`은 Kouku 읽기 전용 선검증을 발탄
writer 획득 전에 수행한다. 발탄 source 투영·snapshot·generation과 출력 생성·교체는 계속
동일 잠금을 사용한다. 공식 owner의 입력 hash와 revision 검사·실패 rollback을 유지한다.
외부 caller가 이미 소유한 writer는 여기서 풀지 않는다. 기존 admission 모듈·실제 publisher
앞부분을 사용해 선검증 중 공유 읽기 가능, 보호 구간 writer 소유, 실패 시 미획득을 검사한다.

## G02. Effect Tool의 게시 대기

`Client/Private/Effect_Tool_Valtan.cpp::Refresh_ValtanPatternTree`는 먼저 비차단 canonical
읽기를 시도한다. writer가 바쁜 동안 반복되는 catalog·원본 animation index 읽기를 건너뛰고
기존 표시 상태를 유지한다. typed WRITER_BUSY에는 게시 대기와 자동 재시도를 표시하고 실제
손상·stale 진단은 보존한다. 기존 exact-save revision 갱신은 다른 revision으로 재시도하지 않는다.

기존 `Render_ValtanPatternTreeSection`은 resource 목록·exact authored 문서를 pattern admission
분기 전에 표시한다. 이 경로와 Open 시 선택 문서만 decode하는 구조를 유지한다. 독립 이펙트
목록을 Server 패턴의 admission으로 다시 막거나 검증 없는 Product fallback을 만들지 않는다.

새 C++ 파일이나 프로젝트 등록은 없다. 변경 TU 격리 컴파일, focused read gate/목록 계약 검사,
인코딩 유지와 diff check를 확인한다. Client 실행·조작·화면 판정은 사용자가 한다.
