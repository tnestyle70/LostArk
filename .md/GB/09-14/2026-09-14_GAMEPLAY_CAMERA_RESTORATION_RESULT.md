# 원본 카메라 복원·FOV 비교 결과

## G00. 실제 완료 상태

Character Select, Bern, Valtan, KoukuSaydon의 시작 카메라와 F1 Player Follow Camera를
같은 ArenaCameraProfile 경로로 연결했다. 원본 패키지에서 확인된 값을 시작 JSON에 적용했고
이전 카메라는 `Before restoration`, 이번 기준은 `Source baseline`으로 즉시 비교할 수 있다.
카메라 적용은 캐릭터·이펙트·맵 Transform, Server 이동·스킬 범위를 수정하지 않는다.
제품 Debug 컴파일·링크·배포는 성공했다. Client 실행·사용자 화면 확인은 수행하지 않았다.

현재 branch는 `pattern-3`이며 다른 작업의 미커밋 변경을 보존했다. 자동 stage/commit/push하지 않았다.
LAN 설정은 server-host, 192.168.0.14:7777, TCP LocalSubnet 규칙 준비 완료다.
설정 당시 endpoint는 not-listening이었다. Server/Client를 시작하지 않았다.

## G01. 원본 출처와 적용값

설치 원본 `ReleasePC/NU1V7NCQ4YAE9ZPJVNOQS.u`의 export8239
`Default__EFIsometricCamera`는 FOV50, Pitch-45, Yaw45, Current/Desired/MaxZoomDist1600cm,
RelativeCenterPos Z-10cm를 가진다. SHA256은
`cd1a52f16bfc7402113da129a66b5b9a550da4a4ed8ff4c66ccdffcc764160d3`이다.
EFPlayerCamera wrapper의 FOV45는 이 isometric 소비자의 FOV50을 대신하지 않는다.

| 맵 | 수평각, 16:9 기준 | 저장 수직각 | 거리 | 근거와 경계 |
|---|---:|---:|---:|---|
| Character Select | 50° | 29.3949576° | 16m | 공통 isometric CDO, 별도 실제 focus bone 높이 미확정 |
| Bern | 50° | 29.3949576° | 16m | 공통 CDO, 조사한 Bern 패키지에는 gameplay camera override 없음 |
| Valtan | 55° | 32.6420632° | 18m | LV_LUT_HEARTRB_ED_PS export27의 efchangeplayercameravolume_4 |
| KoukuSaydon | 50° | 29.3949576° | 19m | MidnightC_PS export43의 efchangeplayercameravolume_1, 1관문 영역 |

Valtan volume의 runtime AABB는 X[105.361956,199.297964], Y[-9.192456,55.052458],
Z[-163.998047,-70.391953]m다. Kouku volume은 X[-31.462043,38.275830],
Y[-7.741910,16.491906], Z[-86.428345,1.409209]m다. Kouku 2·3관문의 19m는 사용자 요청에 따른
시험 기준 확장이며 해당 관문의 원본값을 찾았다는 뜻은 아니다. Mario·미로·시네마틱 카메라는 기존 연출을 유지한다.

현재 엔진은 `XMMatrixPerspectiveFovLH`의 수직 radians를 사용한다.
`vFOV = 2 atan(tan(hFOV / 2) / (16 / 9))`로 한 번 변환하고 JSON에는 수직 degrees를 저장한다.
기존 cinematic importer의 UE cm→runtime m basis `(x,z,-y)`를 적용해 runtime pitch45/yaw135,
eye offset `(-D/2, D/sqrt(2)-0.1, D/2)`를 사용한다. 원본 B_CameraTarget 높이는 미확정이므로
플레이어 origin을 기준으로 한 부분은 프로젝트 연결값이다. native aspect 보정 함수명은 발견했지만
21:9 보정 함수 본문은 복구되지 않았다. 현재 창 비율에 따른 수평각은 툴에서 별도로 표시한다.

CameraSetting244행, IsometricCamera141행, ContentsSetting14행을 대조했고, 30개 map package의
133,807 export를 분류했다. 단일 CDO나 자료 행의 존재를 모든 게임 모드의 실행값으로 확대하지 않았다.
원본 export·brush bounds·CDO·table 증거는 `out/CameraRestoration/`에 저장했다.

공식 웹 자료의 [2021-12-22 PVP 변경 공지](https://lostark.game.onstove.com/News/Notice/Views/1771)는
PVP FOV50→55만 뒷받침한다. 이를 PVE 레이드 공통값으로 사용하지 않았다.
축 계약은 [UE3 카메라 문서](https://docs.unrealengine.com/udk/Three/CameraTechnicalGuide.html)와
[DirectXMath 문서](https://learn.microsoft.com/en-us/windows/win32/api/directxmath/nf-directxmath-xmmatrixperspectivefovlh)를 대조했다.

## G02. 실제 수정과 수습 경계

- ArenaCameraProfile: 네 맵 ID/경로/default, 이전 preset, 실제 look offset, 원본 수평각 변환.
- Bern/Valtan Level: 최초 spawn, follow bind, 재진입과 Valtan 연출 복귀가 같은 profile을 소비한다.
- MainApp: 네 맵 draft/load/save, FOV 중심 편집, 접힌 고급 pose, source/before 비교, 연출 중 적용 차단.
- Engine Camera: projection 갱신 때 유효한 현재 viewport aspect를 사용한다.
- Camera JSON 두 개 추가 및 Client 프로젝트/필터 None 등록. 새 C++ 파일은 없다.

기존 pose는 CS eye(0,6,4), pitch55/yaw-180, focus7, vFOV70;
Kouku eye(-3.1500001,7.75,3.1500001), pitch53.1100006/yaw132/roll-1.75, focus7.75241899, vFOV60;
Bern/Valtan eye(.4,7.5,4.5), look(0,1.2,0), vFOV60이다.
이전 follow response는 CS/Kouku12, Bern0, Valtan18이며 이번에도 유지했다.

Save는 로드한 raw baseline과 현재 디스크를 대조하고 외부 변경이면 파일·draft를 보존하며 거절한다.
최종 파일 대조와 MoveFileEx 사이의 매우 짧은 경쟁 구간은 완전한 filesystem CAS가 아니다.
비교 preset은 live 변경이고, `Save`가 JSON에 영구 저장한다. Reload는 유효한 파일을 다시 읽어 적용한다.

## G03. 크기 보정의 수학적 한계

고정 pose에서 작은 billboard의 화면 크기를 유지하는 배율은
`s = z_new * tan(vFOV_new/2) / (z_old * tan(vFOV_old/2))`다.
여기서 z는 카메라 전방으로 투영한 깊이 `dot(P-eye, forward)`이며 단순 거리나 focusDistance가 아니다.
이번 변경은 FOV·거리·pitch/yaw가 함께 달라져 단일 asset scale로 모든 위치·면 방향을 유지할 수 없다.

실제 네 profile의 view/projection으로 플레이어 원점의 1m 수직선과 1m 지면 사각형을 비교했다.

| 맵 | 수직선 화면 길이, 새/이전 | 지면 사각형 화면 면적, 새/이전 |
|---|---:|---:|
| Character Select | 1.43449 | 1.23194 |
| Bern | 1.57035 | 1.18996 |
| Valtan | 1.24270 | 0.75390 |
| KoukuSaydon | 1.37792 | 0.85602 |

이것은 reference primitive의 수치이며 실제 모델·스킬·완성 화면을 측정한 값이 아니다.
특히 캐릭터 세로 길이와 지면 범위가 서로 다르게 변하므로 일괄 역배율을 적용하지 않았다.
Server 판정과 일치해야 하는 지면 범위는 world size를 유지하고, 사용자 화면 관찰 뒤 필요한
개별 presentation occurrence만 원본 단위·socket·notify scale과 함께 재측정해야 한다.
수치 원본은 `out/CameraRestoration/camera_projection_comparison.json`이다.

## G04. 참고 이미지 여섯 장의 개별 관찰

실제 폴더는 `C:/Users/user/Desktop/로스트아크_베른성`이다. PNG6개 총88,217,215byte를 모두 열람했다.
아래 내용은 제공된 원작 참고 이미지의 관찰이며 현재 Client와의 pixel 비교나 사용자 승인 결과가 아니다.

| 파일 | 관찰과 필요한 복원 층 |
|---|---|
| 베른성_도서관.png,3796×2119 | 따뜻한 갈색 목재·금속 장식, 계단의 음영, 오른쪽 높은 창의 흰빛과 공기 중 광선. 재질 외 방향광·구운 명암·안개·광선/먼지 표현·최종 색 응답을 분리해야 한다. |
| 베른성1.png,3758×2062 | 청색/보랏빛 수로, 금색 문양의 위치별 반사, 식생 그림자. 물의 반사·깊이·굴절/파문과 단단한 석재를 다른 입력으로 봐야 한다. |
| 베른성2.png,3758×2062 | 정원 석재 광장, 대각선 장식과 선명한 방향 그림자·접촉 그림자. 식생 투과광과 인물/소품의 스케일 관계가 보이며 이미지 외곽 때문에 정확한 FOV pixel 교정 기준으로 단정하지 않는다. |
| 베른성3.png,3780×2060 | 금색 원형 이동 장치, 보라색 꽃과 녹색 관목, 난간. 금속 highlight·바닥 접촉·꽃의 색 밀도와 배치 계층을 함께 봐야 한다. |
| 베른성4.png,3780×2060 | 어두운 기둥 안쪽 디테일과 외부 개구부의 흰색/라벤더 빛이 공존한다. 노출·공기 산란·발광/광선·tone response의 관계이며 bloomIntensity 하나로 원인을 확정할 수 없다. |
| 베른성5.png,3708×2036 | 나무·수로·금색 발코니가 겹친다. 식생 층, 물의 깊이/반사, 장식과 그림자의 조합이 중요하며 이미지 자체에는 정확한 카메라 메타데이터가 없다. |

렌더링 입력의 후속 조사·연결은 같은 날짜의 BERN_RENDERING_RESTORATION PLAN/RESULT에 기록한다.

## G05. 실행한 검증과 사용자 경로

- Debug Product build 성공: `out/BuildPipeline/runs/20260914T141902455Z-debug-product.json`,
  약42.956초. Engine/Shared/Server/Client 컴파일·링크·배포 완료. 기존 C4819/C4018/LNK4099 경고 존재.
- 실제 제품 ArenaCameraProfile/DataJson/ProjectDataRoot/PCH OBJ를 링크한 비UI 검사 68개 통과.
  네 기본값, 수평/수직 변환, 이전값, Save/Load roundtrip, baseline 갱신, 외부 변경 거절,
  NaN/손상 JSON/잘못된 area의 실패 보존을 검사했다. `out/CameraRestoration/profile-check.log`.
- 전체 `git diff --check` 성공. Client/UI는 실행·조작·캡처하지 않았다.
- 후속 렌더링 통합에서도 Debug Product build가 성공했고 Camera JSON/XML 및 Bern/Valtan None
  각1개 등록을 다시 확인했다. 최신 실행파일 근거는 같은 날짜 BERN_RENDERING_RESTORATION_RESULT에 있다.

사용자는 Server를 실행하고 `Client/Bin/Debug/Client.exe` 또는 경로가 같은 Client(no build)를 직접 실행한다.
Lobby에서 Character Select/Bern/Valtan/KoukuSaydon에 입장한 뒤 F1 → Player Follow Camera에서
`Before restoration`과 `Source baseline`을 비교한다. 원하는 FOV를 정하고 `Save`한다.
F6 복귀, class 변경, Valtan 연출 복귀, 재진입을 실제 화면에서 확인해야 최종 시각 판정을 내릴 수 있다.
