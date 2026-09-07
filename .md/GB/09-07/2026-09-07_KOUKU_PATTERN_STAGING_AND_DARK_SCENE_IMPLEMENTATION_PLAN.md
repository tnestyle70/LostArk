# 2026-09-07 쿠크 패턴 방향·암전·관문 플레이어 위치

## G00. 이번 변경

사용자가 옮긴 카메라와 현재 저장 데이터를 기준으로 무력화 방패 및 두 반사 영역을 함께 90도 회전한다. 진짜 세이튼 찾기의 본체와 복제 세 명은 동일 중심을 기준으로 -60도 옮기고 반지름과 중심을 바라보는 정책을 보존한다. 댄스타임·룰렛은 반복 재생이나 앞 패턴의 yaw를 상속하지 않도록 패턴 시작의 고정 방향을 정본으로 저장한다.

현재 Server의 resetBossToSpawn은 XYZ만 복원한다. 필요한 yaw는 기존 reset 계약에 optional resetBossYawDegrees로 연결하며 Composition → projector → 기존 gameplay bootstrap → Server reset → snapshot 경로를 사용한다. 누락된 값은 기존 동작을 유지하고 잘못된 값은 publish/load에서 거절한다.

## G01. 암전 배율

새 enabled 맵 Spot은 Scene Profile의 directional RGB와 별개로 합산된다. 기존 exposure 배율은 화면 전체에 적용되므로 PLAYER/BOSS 패턴 Spot까지 함께 줄인다. RenderingProfiles에 optional mapLightIntensityMultiplier(0~4, 기본 1)를 추가하고 Scene Profile Light Detail에서 편집·Save·Publish한다. 암전 profile은 0.05로 시작해 맵 배치 광량을 5%로 낮춘다.

CRenderingProfileService가 active scene commit의 일부로 CMapLightPresentationRuntime의 scene 배율을 적용한다. 실패하면 이전 배율과 기존 render 상태를 함께 보존한다. Map provider는 실제 제출 시점의 배율을 읽어 같은 frame의 scene 전환을 소비한다. authored map brightness를 누적 곱하거나 저장 원본을 수정하지 않는다. PLAYER/BOSS LIGHT와 Effect 조명은 이 맵 전용 배율의 대상이 아니다.

현재 쿠크 scene profile 재생은 Activate_Profile을 즉시 호출하며 Blend ms 보간을 구현하지 않았다. 잘못된 UI 안내를 바로잡고 Blend ms와 최종 광량을 구분한다. 이번 변경에서 별도 보간 runtime은 만들지 않는다.

## G02. 플레이어 이동

CLevel_KakulSaydonArena::Get_DebugGates의 1·3관문 destination만 (-2.45, 1.32, 945.17)m로 바꾼다. 기존 typed Server teleport와 navigation 높이 보정을 유지한다. 다른 관문 좌표와 사용자가 수정한 boss placement yaw는 보존한다.

## G03. 검증·배포

기존 publisher의 schema/수치/실패 보존 검증, 패턴 회전 수치 확인, 변경 JSON parse 및 diff check를 수행한다. 필요한 Client/Server 컴파일은 기존 Product runner를 사용한다. 새 C++ 파일은 없으며 project/filter 등록 추가는 불필요하다. 패턴 Product/Server gameplay와 RenderingProfiles를 명시 publish하고 source revision 일치를 확인한다. Client/UI 실행과 화면 판정은 사용자에게 남긴다.

작업 중 사용자가 추가 저장한 `월드오브젝트_카드`의 최신 worldsequences 정본도 보존한다. 15:30 저장본의 template에는 `idle_normal_1`, `att_battle_1_start`, `att_battle_2_01` 세 animation clip이 들어 있다. 실행용 문서는 아직 두 clip이므로 최신 원본을 기준으로 기존 Area publisher를 실행해 반영하고 source/runtime의 JSON 의미 일치를 확인한다.

## G04. 마지막 저장과 공 Timeline 후속 요청

사용자가 종료 직전 저장한 worldsequences revision 366에는 카드·조커카드가 각각 세 clip이고, 실제 clipName을 한글 별명으로 변경한 상태다. clipName은 WModel lookup ID이므로 원래 세 ID로 복구하고 작성한 한글은 optional displayName에 보존한다. parser/serializer/동등성/기존 Map publisher와 Object Tool timeline에 연결한다. UI는 표시 이름만 편집하며 실제 ID는 읽기 전용으로 보여준다. 이전 무필드 문서의 동작은 유지한다.

요청한 MN_RHCN_00 공의 원본에는 bone animation clip이 없고 particle의 위치 곡선이 있다. 설치된 fm_d_rhcn_00 WModel은 원본 skeletal mesh와 동일한 전체 geometry이므로 기존 잘못 연결된 서커스 공 대신 이 모델과 MN_RHCN_00 diffuse를 사용한다. 새 모델 런타임이나 cook 경로는 만들지 않는다.

Object Tool의 기존 velocity/acceleration과 seek 평가를 재사용한다. Lifetime과 높이로 수직 포물선을 만드는 편의 입력 및 physics 높이 Timeline을 제공하고, v=4H/T, a=-8H/T²를 기존 Motion 필드로 저장한다. 원본 particle의 시간 곡선을 정확히 복원한 clip이라고 표시하지 않는다.

## G05. Effect·Scene Profile 동시 누락과 방향 재조정

사용자가 확인한 현상은 무력화 시작·진짜 세이튼 찾기 v2 Effect와 Scene Profile의 동시 누락이다.
현재 source 85의 Scene Profile duration 24127ms와 blend 600000ms는 저작 계약상 유효하지만,
Presentation Product reader가 blend를 일반 Effect fade로 검사해 전체 Product 로드를 취소한다.
Scene Profile만 저작 계약과 같은 0..600000 범위를 허용하고, 잘못된 Scene Profile 행은 진단과 함께
격리해 정상 Effect가 사라지지 않도록 수정한다. 일반 Effect fade 검사는 그대로 유지한다.
실제 Scene Profile은 현재 즉시 활성화되며 Blend 보간 구현은 이번 범위에 추가하지 않는다.
확인된 원인은 `.md/GB/gotchas.md`에 기록한다.

댄스타임·룰렛은 사용자가 현재 3시에서 6시를 향하도록 명확히 지정했다. 실제 Server yaw 전달과
카메라 투영을 확인한 결과 시계 방향은 +90도이므로 현재 절대 yaw 147도에서 237도로 저장한다.
진짜 세이튼 및 세 분신의 수정된 위치와 G1/G3 플레이어 생성 좌표는 유지한다. Composition을
86으로 올려 Product·Server bootstrap까지 같은 publisher로 반영하고 Client·Server를 함께 빌드한다.
