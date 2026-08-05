# 2026-08-05 UI Display Colorspace Fix RESULT

## G00. UI display-space texture load correction

### 1. 완료 요약

- UI PNG/DDS가 가진 sRGB metadata를 DirectXTK가 선형 색으로 decode한 뒤, scene gamma 이후의 UNORM back buffer에 그대로 기록하던 이중 색공간 불일치를 수정했다.
- loading UI의 `CTexture`, 제품 HUD의 `CHUDRuntimeView`, HUD 저작 도구의 `CHUDLayoutTool`이 모두 `*_LOADER_IGNORE_SRGB`를 사용해 저작된 display-space byte를 보존한다.
- shader, blend state, swap-chain format, world `CMaterial`, UI JSON 및 Resources payload는 변경하지 않았다.

### 2. 실제 변경

| 파일 | 실제 변경 |
|---|---|
| `Engine/Private/Texture.cpp` | DDS/WIC load를 `Create*TextureFromFileEx`와 `DDS_LOADER_IGNORE_SRGB` / `WIC_LOADER_IGNORE_SRGB`로 교체했다. 현재 이 Prototype을 소비하는 loading UI가 display-space SRV를 사용한다. |
| `Client/Private/HUDRuntimeView.cpp` | 제품 HUD texture cache가 동일한 display-space load 계약을 사용한다. |
| `Client/Private/HUDLayoutTool.cpp` | authoring canvas와 Debug runtime preview가 제품 HUD와 동일한 display-space load 계약을 사용한다. |

새 C++ 파일이나 public header 변경은 없으며 `.vcxproj`와 `.vcxproj.filters` 변경도 필요하지 않았다.

### 3. 자동 검증

#### PASS

- UI loader contract 정적 검사: 대상 3개 파일 모두 DDS/WIC ignore-sRGB flag를 포함한다.
- `Data/UI` JSON parse: 3개 문서 모두 성공했다.
- UI source metadata 실측: `Client/Bin/Resources/UI` PNG 148개 중 148개가 sRGB 또는 gAMA chunk를 가진다.
- 대상 diff `git diff --check`: 통과했다. 기존 C++ 파일의 다음 Git 처리 시 LF→CRLF 변환 경고만 있고 whitespace error는 없다.
- Engine x64 Debug 직접 build: 성공.
- Client x64 Debug 직접 build: 성공.
- Engine x64 Release 직접 build: 성공.
- Client x64 Release 직접 build: 성공.
- `ClientFrontendHarness` Debug: `failures : 0`.
- `ClientFrontendHarness` Release: `failures : 0`.
- 정본 `Invoke-BuildAndRegression.ps1 -Configuration Debug`: Engine/Shared/Server/Client build와 선행 harness는 성공했고 마지막 ProjectAudit까지 도달했다.
- 정본 `Invoke-BuildAndRegression.ps1 -Configuration Release`: Engine/Shared/Server/Client build와 선행 harness는 성공했고 마지막 ProjectAudit까지 도달했다.

#### 기존 작업 상태로 인한 FAIL

Debug와 Release의 마지막 ProjectAudit는 동일한 기존 Effect Tool 감사 항목으로 종료 코드 1을 반환했다.

```text
Project audit failed (2):
asset-lock.inventory: files=10256 bytes=5507131395
effect.g4-typed-resource-boundary: paths=0 authoredUnexpected=0 intake=0 shaders=0 symbols=0 project=0 entry=False typedResource=False
```

현재 worktree에는 이번 G와 무관한 Effect Tool 미커밋 변경과 신규 파일, ProjectAudit 변경이 이미 존재한다. UI 색공간 변경 파일에서는 위 실패를 재현시키는 변경이 없으므로 이를 되돌리거나 보정하지 않았다.

### 4. 수동 검증 상태

- 자동 build와 contract 검증은 완료했다.
- 실제 게임 창의 loading chrome, combat HUD, HUD Layout Tool canvas를 육안 비교하는 interactive smoke는 이번 실행 환경에서 수행하지 않았다.
- 수동 확인 시 원본 PNG와 UI 중간톤 밝기를 비교한다. 전체 UI가 정상인데 투명 경계만 어두우면 이번 색공간 문제와 분리해 premultiplied-alpha 계약을 진단한다.

### 5. 남은 경계

- `CTexture`의 현재 실사용자는 loading UI뿐이다. 이후 world/lighting texture consumer가 추가되면 loader flag를 암묵적으로 공유하지 말고 명시적 color-space 인자를 public 계약으로 분리해야 한다.
- 대규모 dirty worktree의 다른 담당 변경을 보존하기 위해 stage, commit, push는 수행하지 않았다.
