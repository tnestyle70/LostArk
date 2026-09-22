# Effect Tool 개별·그룹 회전과 고정축 면 연결 구현 계획

## G00. 현재 실측과 목표

사용자는 쿠크 팡파레의 음표·도넛·경고·양방향 요소와 대형 세이튼 파1빨2 fog에서 회전이 내부 무늬만 움직이는 것처럼 보이며, 개별 요소도 같은 앵커 중심으로 회전하기를 요청했다. 삭제한 바람 메시 요소는 복원하지 않는다.

Composition2195 P11 presentation32는 `effect.kouku.source.fx_mn_rpcz_00_u.par_u_rpcz_safezone_fog_01_loc_int`를 사용한다. 9개 source sprite 중 6개는 EPAL 고정축, 3개는 camera facing이다. 저장 Element 회전은 약[-19.5,42.5,48.5]이나 모두 기존 `followEmitterAxisRotation`이 꺼져 있다. fixed-axis 6개만 이번 저장 후보에 포함한다. Playback은 emitter matrix를 만들지만 GeometryHelpers가 fixed-axis quad 방향을 재구성하면서 그 회전을 제외한다. native 재질은 emitter frame을 별도로 소비할 수 있어 내부 무늬와 실제 면이 달라진다.

`Effect_Tool_Detail.cpp::Render_TransformDetail`은 현재 회전 값만 바꾼다. `Effect_Tool_Helpers.cpp::Rotate_AttachmentElementGroup`은 별도로 pivot 주위 위치·속도·방향을 quaternion delta로 바꾼다. 일반 Detail와 그룹의 편집 의미가 다르다. 이 두 입력을 동일 helper로 연결한다. 원본 runtime 기본값은 바꾸지 않고 명시적으로 편집한 요소에만 기존 fixed-axis opt-in을 저장한다.

P23의 최신 저장 자산은 trumpet_c(피자), trumpet_d_music(음표), trumpet_d(도넛), outerdonut.warning이다. 미저장 도구 draft와 삭제된 바람 요소는 디스크 감사로 확정하지 않는다. Mesh particle은 TypeData pre-rotation, StartRotation, emitter world, renderer의 locked-axis camera facing을 별도로 검증하며 sprite 원인으로 일반화하지 않는다.

## G01. Tool 내부 helper와 상태

`Client/Private/Effect_Tool_Internal.h`와 `Effect_Tool_Helpers.cpp`는 그룹·개별 편집에 공통으로 쓰이는 pivot 수학과 source locked-axis 판정을 소유한다. 기존 MaterialDetail의 `Has_SourceLockedAxisSprite`를 공통 내부 helper로 이동하여 UI checkbox와 회전 명령의 admission을 일치시킨다. 새 runtime schema·프로그램·저장 pivot owner는 만들지 않는다.

회전 명령은 전체 대상의 finite/range/소유자 검증 후 위치, 방향, 선형 속도와 끝점을 함께 stage한다. 대상 source fixed/rotate-axis sprite에는 기존 `detail.sprite.followEmitterAxisRotation=true`를 함께 stage한다. 다른 sprite, mesh, decal, trail의 billboard 정책은 변경하지 않는다. 선택하지 않은 요소와 실패 전 문서는 그대로 유지한다.

그룹은 기존 anchor/manual group ID를 사용한다. pivot 선택은 도구 세션의 center/anchor-origin/custom/Element-origin 네 가지를 사용하며, 결과 Element TRS만 저장한다. 기본 pivot은 anchor origin으로 두고 center/custom은 명시 선택으로 유지한다. animated rotation/revolution, source transform track, runtime carrier, master transform 등 별도 소유자가 있는 항목은 기존 제한을 유지한다. 단일 선택은 선택 요소의 eligibility만 검사한다. 서로 다른 parent를 섞은 그룹은 공통 중심이 정의되지 않으므로 단일 선택에서도 Group center만 거부하며 Anchor/Custom/Element origin은 해당 요소의 parent 공간에서 허용한다.

## G02. 개별 Transform Detail 연결

`Client/Public/Effect_Tool.h`의 `Render_TransformDetail` 인자를 Detail에서 Element로 바꾸어 stable ID와 source carrier를 함께 판단한다. `Client/Private/Effect_Tool_Detail.cpp`의 호출자와 구현을 같이 바꾼다.

현재 draft를 포함하는 staged document에서 해당 anchor group을 찾고 같은 pivot selector를 표시한다. 회전 입력은 단일 element ID를 지정한 기존 group helper로 처리한다. 성공하면 해당 draft의 Transform/LinearLerp와 제한된 sprite flag를 갱신하고, 기존 live preview 및 Apply/Save 경로로 전달한다. 실패하면 이전 draft와 preview를 유지하고 이유를 표시한다. source-owned 회전은 거절 이유와 기존 local transform 의미를 구분해 표시한다.

`Effect_Tool_MaterialDetail.cpp`는 중복 판정을 제거하고 공통 내부 helper를 사용한다. 원본 별도 axis-follow checkbox와 저장 동작은 유지한다. 기존 다섯 H/CPP만 변경하므로 프로젝트/filter 등록 추가는 없다. 인코딩과 CRLF를 보존한다. root의 진행 중 Product 빌드와 충돌하지 않도록 우선 `out/EffectRotation20260922/candidate`에서 준비한다.

## G03. fog 저장 후보

현재 설치 fog 중 fixed-axis 6개 stable element ID에 `detail.sprite.followEmitterAxisRotation`만 추가한 후보와 before/hash/field manifest를 만든다. 사용자 각도·위치·scale·sourceRecipe·재질·삭제 상태는 보존한다. 실제 설치는 root가 최신 저장본에 stable field/hash/backup/atomic 절차로 병합한다. 데이터 후보 작성과 실제 설치·도구 Reload·화면 확인은 분리한다.

## G04. 검증과 종료 조건

- 실제 후보 helper 본문을 추출한 native probe에서 group/individual, anchor/custom pivot, 복합 Euler·비균일 scale, velocity/end-position, invalid-input 보존을 검사한다.
- 실제 Codec save/reload와 Playback/final sprite quad에서 fog의 원점·면·source emitter basis가 같은 delta를 소비하는지 검사한다. opt-off 원본 재생과 camera/velocity sprite는 보존한다.
- 현재 남은 source mesh particle의 TypeData/StartRotation과 실제 Playback world를 검사하고, renderer locked-axis override 대상 여부를 기록한다. 일반 비대칭 mesh/decal를 포함해 pivot 수학이 carrier 종류에 따라 분기하지 않는지 검사한다. 삭제된 요소 자체의 재현 성공을 주장하지 않는다.
- 변경 Tool TU를 out에서 최소 컴파일하고 JSON/XML parse 및 해당 diff를 확인한다. Product build/link는 root가 담당한다. Client/UI 실행은 하지 않는다.
- RESULT에는 구현, 실제 설치, 자동 수치/컴파일, GPU와 사용자 최종 화면 미확인을 구분한다. 재사용 원리는 gotchas/V2에 중복 없이 root와 조율해 반영한다.
