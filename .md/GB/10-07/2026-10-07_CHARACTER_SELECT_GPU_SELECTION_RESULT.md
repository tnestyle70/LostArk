# Character Select GPU 선택 수정 결과

## 완료한 수정

`CGraphic_Device`의 HARDWARE 생성은 DXGI의 HIGH_PERFORMANCE 순서로 실제 adapter를
선택한다. software adapter를 제외하고 명시 adapter에는 UNKNOWN driver type을 사용한다.
성공한 device/context만 반영한다. 우선 후보를 만들 수 없거나 Factory6가 없으면 기존
hardware 경로를 사용한다. 명시 WARP, Debug layer, feature level과 swapchain 정책은 유지했다.

`CMainApp`은 실제 생성된 device의 GPU 이름·vendor/device ID·LUID·feature level·메모리
용량을 기존 `Client/Default/ClientStartup.user.log`의 `Graphics.Adapter`에 기록한다.
용량은 사용량이 아니며 로그 실패는 초기화를 실패시키지 않는다.

Character Select의 체험 전환 때 숨겨진 표시 캐릭터를 Layer에서 해제하는 수정도 함께
빌드했다. 이 변경의 상세와 전체 코드는 10-05 BLACK_STAGE PLAN/RESULT G07에 있다.
숨김 캐릭터의 불필요한 animation update를 제거하지만 전체 FPS 회복을 이 수정만으로
입증한 것은 아니다.

## 원인 판정 근거와 Git 이력

- 문제 실행 PID3504는 실제 Debug Client였다. Windows의 두 Client EXE GPU preference는
  이미 2였으므로 설정값만으로 실제 RTX 사용을 확정할 수 없었다.
- 2026-10-07 02:19:23 UTC의 raw counter 표본 간격은 2.1010824초였다. AMD 840M의
  LUID `0x17055` 3D RunningTime은 5,247,407,174 → 5,266,729,001로 증가하여91.9613%였다.
  RTX4050 LUID `0x196D1`의3D는318,786 →318,786, 양쪽 Copy는0%였다.
- 같은 PID GPU committed allocation은 AMD6,785,732,608바이트, RTX20,819,968바이트였다.
  실제 Client 내부 device를 직접 query한 과거 로그는 없지만 render work가 AMD에 집중된
  강한 실측 증거다. 일부 formatted counter 이상치는 판정에서 제외했다.
- Release6696의 세션 로그에는 private14.2GiB, available RAM0.31GiB, main pump gap1.079초가
  있었다. Debug3504에도 RAM 부족과 page-in이 관측됐다. private commit·physical working set·
  GPU allocation은 서로 다른 지표이며 이를 VRAM 부족 하나로 치환하지 않았다.
- `Graphic_Device.cpp`는10월2일 `a0ff0185`와 현재 HEAD 사이 변경이 없다. 기존 코드는
  `D3D11CreateDevice(nullptr, HARDWARE, ...)`였다. 마이그레이션이 GPU를 바꿨다고 단정할
  이력은 없고, 과거 실행의 GPU 로그가 없어 전환 시점도 확정하지 않았다.

## 검증 완료

| 검증 | 결과 |
|---|---|
| Release Product 증분 빌드·배포 | PASS,68.915초, `20261007T022407502Z-release-product.json` |
| Debug Product 증분 빌드·배포 | PASS,45.175초, `20261007T022504858Z-debug-product.json` |
| 동일 helper 무창 device 생성, Release/Debug | 모두 RTX4050 `0x196D1`, feature level11_0 |
| 명시 WARP, Release/Debug | 모두 Microsoft Basic Render Driver, 기존 선택 유지 |
| 독립 diff 검토 | adapter/driver 조합·fallback·swapchain·로그 실패 격리 PASS |
| 소스 인코딩 | Graphic_Device CP949, MainApp UTF-8, CRLF와 기존 비ASCII 바이트 보존 |
| `git diff --check` | PASS |

빌드 receipt는 `out/BuildPipeline/runs`, 장치 생성 검증은
`out/GpuAdapterProbe20261007/probe.log`와 `source.sha256.txt`에 있다. 기존 C4819/외부 PDB
경고는 남았고 신규 컴파일/링크 오류는 없었다. Client/UI 실행·종료는 에이전트가 하지 않았다.

## 조사한 별도 경계

10월2일 기준 Character Select 저장 렌더링 profile은 조사 당시 현재와 동일했다.
새 SSGI/SSR/HorizonAO는 기본 OFF이며 OFF일 때 GI/SSR draw는 실행하지 않는다.
SSAO 실험 통합으로 기본 entry의 컴파일 레지스터가6→18로 증가한 별도 후보는
`out/CharacterSelectRenderAudit20261007/shader_history`에 보존했다. 사용자의 즉시 GPU 연결
요청을 우선해 이 후보를 제품 셰이더에 적용하지 않았다. 실제 FPS 영향은 미측정이다.

별도 Unreal 프로젝트는 `C:/Users/tnest/Desktop/LostArkUnrealComparison`이다.
원래 LostArk에서 uasset/umap은 발견되지 않았고 Framework/Client/Engine 제품 프로젝트에
Unreal 빌드 연결도 없다. `out/UnrealComparisonCandidate/ViewerBuild`에는 임시 uproject만
있으며 외부 프로젝트의28330 uasset/34 umap과 구분한다.

동시 작업에서 발생한 UserSettings·RenderingProfiles·texture harness 변경은 본 GPU 수정의
소유 범위가 아니다. 해당 변경을 덮어쓰거나 본 작업의 옵션 수정으로 기록하지 않았다.
사용자가11:25:36에 재실행한 Release PID22260의11:25:38 `Graphics.Adapter` 로그에서
실제 `NVIDIA GeForce RTX 4050 Laptop GPU`, LUID `0x196d1`, feature level `0xb000`을
확인했다. 따라서 수정된 제품 Client의 RTX 연결까지 확인 완료다. Character Select의
실제 FPS·최종 화면 판정은 사용자 확인 대상이며 수치상 개선율은 아직 측정하지 않았다.

공식 API 계약: [GPU preference 열거](https://learn.microsoft.com/en-us/windows/win32/api/dxgi1_6/nf-dxgi1_6-idxgifactory6-enumadapterbygpupreference),
[명시 adapter의 driver type](https://learn.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-d3d11createdevice).

Git commit은 작성자 이름·메일 미설정으로 실패했다. 다른 사람의 identity를 임의 설정하지
않았다. 변경과 빌드 결과는 작업 폴더에 보존했고 본 작업이 추가한 staging만 해제했다.
