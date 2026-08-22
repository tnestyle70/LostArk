# 2026-08-22 Client frontend 광역 계약 하네스 삭제 결과

## 결론

`ClientFrontendHarness`는 현재 정본의 기능 승인 경계에서 완전히 제거했다. 이 프로젝트는 Client의
effect/resource/frontend seam을 약 5만 줄의 두 번째 실행 파일로 복제하고 서로 독립적인 계약을 한 번에
판정했다. 그 결과 PR #141에서 실제 이펙트 회귀와 오래된 기대값의 불일치가 46건 이상 뒤섞였으며,
작은 effect family 변경을 main에 승격할 수 없는 병목이 됐다.

삭제 뒤에는 같은 역할의 광역 하네스를 새로 만들지 않는다. effect/resource 변경은 해당 domain
publisher/check, focused Python/WARP 계약, 그리고 정본 Client Debug/Release 빌드로 검증한다.
화면 fidelity와 occurrence 승인은 계속 사용자의 Effect Tool 육안 A/B 경계다.

## 삭제 범위

- `Tools/ClientFrontendHarness` 프로젝트, filters, 광역 실행 소스와 authored materializer를 삭제했다.
- `Framework.sln`의 프로젝트와 Debug/Release 구성 매핑을 삭제했다.
- `Invoke-BuildAndRegression.ps1`의 빌드, 일반 실행, reconstructed GPU material 실행을 삭제했다.
- 과거 FinalCode 폴더에 복제돼 있던 실제 프로젝트/소스 snapshot도 삭제했다.
- 활성 build/팀 인계/프로젝트 문서가 삭제된 프로젝트를 정본 검증으로 지칭하지 않도록 정리했다.
- Valtan proof builder의 설명은 독립된 tracked-input proof라는 실제 역할로 바로잡았다. proof 수식과
  Valtan authored/candidate/world 데이터는 변경하지 않았다.

## 보존 범위

- `MaterialEvaluatorHarness`
- `PointLightFalloffContractHarness`
- `NetworkProtocolHarness`, `ValtanFourPlayerHarness`, `CharacterSelectIsolationHarness`
- `PhysicsContractHarness`, `WModelGeometryContractHarness` 등 effect/resource 광역 frontend 복제와 무관한
  focused/core 계약 실행 파일

## Effect visual program 계약

`build_effect_visual_program_runtime.py`가 삭제된 실행 파일의 Debug/Release 경로를 암묵적으로 찾던 동작도
제거했다. tracked runtime artifact의 정상 승인은 `--artifact-check`이며 Product 입력을 변경하지 않는다.
재생성이 꼭 필요할 때만 호출자가 `LOSTARK_EFFECT_DOCUMENT_CODEC_TOOL`로 별도의 typed codec 실행 파일을
명시해야 한다. 환경 변수가 없거나 파일이 아니면 이유를 보존한 채 실패한다.

## 자동 검증

| 검증 | 결과 |
|---|---|
| active source/solution/regression/team 문서의 `ClientFrontendHarness` 참조 | 0건 |
| `Framework.sln` 프로젝트 이름/GUID 잔존 | 0건 |
| `Invoke-BuildAndRegression.ps1` PowerShell parser | PASS |
| 변경한 effect Python script `py_compile` | PASS |
| Valtan focused drawable proof | 24/24 PASS |
| `Publish-Effects.ps1 -Mode Validate -ResourceRoot Client/Bin/Resources` | PASS |
| effect visual program artifact check | 17 programs, 135 rows, Product mutation `false` |
| `Sync-EffectDataProject.ps1 -Check` | PASS |
| `Test-EffectPipeline.ps1` | 110/110 PASS |
| `Framework.sln` x64 Debug | PASS |
| `Framework.sln` x64 Release | PASS (`Client/Bin/Release/Client.exe`) |
| `git diff --check` | PASS |

Debug/Release 전체 solution build는 삭제된 프로젝트 없이 Client와 보존 대상 하네스를 모두 빌드했다.
기존 FXC `X4717/X4000`, MSVC
`C4819/C4244/C4267`, linker `LNK4099` 경고는 이번 삭제가 만든 오류가 아니다.

## 직접 재생성 테스트의 기존 입력 불일치

`test_build_effect_visual_program_runtime.py`의 직접 regeneration 경로는 변경한 함수에 도달하기 전 corpus
raw-hash setUp에서 기존 불일치를 보고했다. 전체 effect pipeline과 tracked artifact check는 통과했으므로
삭제 변경의 회귀는 아니다.

확인된 입력은 다음과 같다.

- checkout EOL 차이: Artist `31950` authored 문서, Artist ribbon parent receipt
- HEAD 내용과 receipt 기대값 불일치: Valtan safe-reviewed-gap application receipt,
  Valtan trail-adapter packets, effect visual program corpus schema

이 cleanup에서 hash를 다시 봉인하면 Artist/Valtan 수입 증거와 schema의 의미를 함께 변경하게 되므로
수정하지 않았다. 특히 Valtan authored/candidate/world 및 imported evidence를 되돌리거나 덮어쓰지 않았다.
재생성이 필요하면 해당 corpus receipt 소유 변경에서 입력 의미를 검토한 뒤 별도로 재봉인해야 한다.

## 수동 경계

이 삭제에는 Client/UI 자동 실행이나 visual PASS가 없다. PR #143의 Glasshole02 Tool canary는 계속 기본
OFF이고 Product admission은 false다. 사용자가 Effect Tool에서 차원술사 W `2050120.clip3`, occurrence
`authored.source-particle.40e1b48e2f0f88dcfeff1549`를 열어 canary를 켠 뒤 첫 픽셀, 카드 경계, 굴절,
내부 방사, depth intersection, UV 움직임, 방향과 크기를 원본과 A/B해야 한다.
