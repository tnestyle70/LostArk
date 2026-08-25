# Rendering Workbench Save/Publish/Reload 복구 결과

## 1. 결론

- Bloom `Scatter`는 `Data/Rendering/Authored/RenderingProfiles.json`에 정상 저장되고 있었다.
- 실제 실패는 같은 문서의 `fxaaEdgeThreshold=0.0311999992`를 publisher가 decimal double 최소값 `0.0312`와 비교해 거부하면서 runtime publish 전체가 중단된 것이었다.
- publisher의 renderer 소유 수치 검증을 C++과 같은 float32 계약으로 통일했고, authored 저장부터 runtime publish와 live reload까지 한 번에 수행하는 `Save + Publish + Reload` 경로를 추가했다.
- 정본 runtime은 authored와 같은 `revision=10`, `bloomScatter=1`, `fxaaEdgeThreshold=0.0311999992`로 승격됐다.

## 2. 구현

### 2.1 float32 publisher 계약

- `Tools/RenderingPipeline/Publish-RenderingProfiles.ps1`
  - renderer가 소비하는 f32 field를 float32 최소/최대 경계로 검증한다.
  - 정상적인 float32 JSON round-trip 값은 허용하고 실제 경계 밖 값은 계속 거부한다.
  - 실패 publish는 기존 runtime과 backup을 보존한다.

### 2.2 저장·적용 transaction

- `Client/Private/RenderingProfileService.cpp`
- `Client/Public/RenderingProfileService.h`
- `Client/Private/MainApp.cpp`
  - stale authored revision을 덮어쓰지 않도록 저장 전 disk revision을 확인한다.
  - unique staged path와 process output 보존을 사용한다.
  - `Save + Publish + Reload` 버튼은 앞 단계 성공 때만 다음 단계를 수행하고 partial failure를 구분해 보고한다.
  - 기존 개별 Save/Publish/Reload 버튼은 복구·진단용으로 유지한다.

### 2.3 회귀 테스트

- `Tools/RenderingPipeline/Test-RenderingProfiles.ps1`
  - canonical authored validate와 임시 publish
  - float32 경계값 수용과 실제 범위 밖 거부
  - 실패 transaction의 기존 runtime 보존
  - numeric string과 duplicate JSON key 거부를 검증한다.

## 3. 검증 결과

- `Tools/RenderingPipeline/Test-RenderingProfiles.ps1`: PASS
- `Publish-RenderingProfiles.ps1 -Mode Validate`: PASS
- `Publish-RenderingProfiles.ps1 -Mode Publish`: PASS
- 정본 parity:
  - authored/runtime revision: `10 / 10`
  - authored/runtime Bloom Scatter: `1 / 1`
  - authored/runtime FXAA edge threshold: `0.0311999992 / 0.0311999992`
- JSON parse: PASS
- Client x64 Release compile/link: PASS, 기존 shader/encoding/PDB warning만 유지

## 4. 수동 확인 경계

- 현재 실행 중이던 Debug Client는 에이전트가 종료하지 않았다.
- 사용자는 새 Debug Client에서 Rendering Workbench의 `Save + Publish + Reload`를 사용하거나 Client를 재시작한 뒤 Scatter 변화를 확인한다.
- 사용자 관찰 전에는 visual PASS로 기록하지 않는다.
