# 발탄·쿠크 기본 Bloom 일치 결과

## G00. 실제 반영

사용자의 현재 디스크 저장본 기준 반영 승인을 받아 revision 86을 87로 올렸다.
`scene.valtan.cool-low-key.v1`의 Bloom을 쿠크 기본
`scene.kakulsaydon.g1.base.v1`과 동일하게 변경했다.

| 필드 | 최종값 |
|---|---:|
| bloomEnabled | false |
| bloomThreshold | 1 |
| bloomSoftKnee | 0.5 |
| bloomIntensity | 0.800000012 |
| bloomScatter | 1 |
| bloomTint | [1, 1, 1, 1] |
| bloomIntensityMultiplier | 0 |

발탄의 `valtan.ps.environment.31.convex.0`과 `.1` 두 region postProcess에도
threshold 1과 intensity 0.800000012를 적용했다. 두 region에는 qualityOverride가
없으므로 기본 Bloom OFF를 유지한다. 실제 scene intensity는 multiplier 0에 따라 0이다.

원본과 runtime 각각 revision 외 Bloom scalar 9개만 바뀌었다. 비Bloom 필드와
다른 모든 profile, 기존 마하라카 추가 변경을 구조적으로 보존했다. 발탄의
before-restoration/source-rendering 비교 프로필과 개별 스킬 문서는 수정하지 않았다.
public schema, C++/shader, project/filter 변경은 없다.

## G01. 저장·게시와 검증

최신 원본과 runtime bytes를 보관한 뒤 stable profile/region ID로 범위를 제한했다.
교체 직전 원본 bytes/hash를 재확인했고, 동일 디렉터리의 임시 파일을 원자 교체했다.
실패 시 설치 결과 hash가 자기 후보와 같을 때만 백업으로 복귀하는 절차를 사용했다.
동시 저장 충돌과 rollback은 발생하지 않았다.

- 기존 Publish-RenderingProfiles.ps1의 후보 Validate: PASS.
- 같은 publisher의 후보 Publish 및 roundtrip 검증: PASS.
- 설치된 정본과 runtime 각각 기존 publisher Validate: PASS.
- 정본은 예상 10개 scalar 차이(revision 포함) 외 전체 구조 동일: PASS.
- 게시본도 예상 10개 scalar 차이 외 전체 구조 동일: PASS.
- 작업 트리 전체 `git diff --check`: exit 0. 기존 LF/CRLF 안내는 오류가 아니다.

백업, 후보, publisher 로그, 변경 delta와 SHA-256은 Git 제외
`out/ValtanBloomAlignment20260930/`에 있다. 결과 요약은 `result.json`이다.
정본 SHA-256: `002c9d7813852ee22044ec31d3708f29aea702a218f59441a725496ef5de4c92`.
게시본 SHA-256: `b8e5764cc2dffafa943552d23a8c910d842cb9f42a012ccee96039b342fefcea`.

## G02. 실행 중 반영과 남은 확인

데이터 변경이므로 컴파일은 필요하지 않아 실행하지 않았다. Client/UI 실행·조작·캡처와
실행 중 메모리 Reload는 수행하지 않았다. 서버 gameplay 데이터는 변경하지 않았다.

사용자는 `F1 → Tools → Rendering Workbench → Reload Runtime`을 실행한다.
발탄 기본 profile에서 `Benchmark → Rendering restoration`의 `Bloom off`와
실제 스킬 번짐을 직접 확인한다. Live comparison이나 source 비교 profile을 별도로
선택한 경우의 look은 기본 profile과 다르다. 사용자 화면 판정은 미실행 상태다.

대규모 기존 dirty 작업 트리를 보존했으며 자동 stage/commit/push하지 않았다.
