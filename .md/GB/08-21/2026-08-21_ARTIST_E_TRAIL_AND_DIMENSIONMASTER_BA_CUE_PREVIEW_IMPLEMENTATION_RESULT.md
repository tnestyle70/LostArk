# 2026-08-21 도화가 E 붓 Trail 및 차원술사 BA Cue 미리보기 구현 결과

## 0. 결과 상태

- 문서 종류: `RESULT`
- 작업 브랜치: `codex/artist-e-trail-ba-cue-0821`
- 작업 시작 기준 HEAD: `02e0f266613caffd88d2390eee4514d43335017f`
- 자동 검증: 완료
- Client/UI 수동 시각 검증: 미실행, 사용자 판정 대기
- Product publish: 실행하지 않음. `Publish-Effects.ps1 -Mode Validate`만 통과함

## 1. 실제 반영 내용

### 1.1 차원술사 BA Effect Tool 미리보기

- Product 매핑 `[ba1, ba1, ba3, ba1]`은 변경하지 않았다.
- Open Editor가 같은 Effect ID의 Product cue context를 보존하도록 연결했다.
- Saved direct-authored 문서는 skillbinding과 animevents에서 exact clip 후보만 resolve한다.
- `ba1.unified`는 BA1/BA2/BA4 중 선택한 clip 하나만 재생하고, `ba3.unified`는 BA3만 재생한다.
- 매핑 후보가 없는 player direct-authored 문서는 전체 BA chain으로 fallback하지 않는다.
- cue `startMs`는 read-only로 표시한다. animevents를 Effect Tool에서 직접 수정하는 writer는 추가하지 않았다.

### 1.2 Effect Tool `Duplicate Selected`

- `Delete Selected` 옆에 선택 element 복제 명령을 추가했다.
- timing, material, resource, attachment, source recipe를 deep-copy한다.
- 새 stable ID와 `authored-copy:<source-id>` source node를 부여하고 source presentation은 비운다.
- 전체 document validate/stage 성공 뒤에만 draft를 commit하므로 실패 시 원본과 선택 상태를 보존한다.

### 1.3 도화가 E 31480 흰백색 붓 ribbon

- Tool preview가 enabled+follow ActionCueAttachment의 실제 model bone/socket anchor를 매 update에 공급한다.
- direct-authored exact FlowRibbon element도 VisualProgram 전역 여부가 아니라 element admission 단위로 profile 35 CPU payload를 만든다.
- renderer와 playback이 같은 exact FlowRibbon material/resource predicate를 공유한다.
- malformed FlowRibbon tuple은 generic profile로 내려가지 않고 fail-closed한다.
- stable element `authored.source-particle.c6d4247396f641316d28767c`에만 `fx_m_trail_010.dds` project-tuned override와 `minDistance=0.01`을 적용했다.
- 기존 profile 35 shader의 RGB luminance coverage, 흰백색 emissive, dynamic dissolve와 기존 dynamic trail vertex/index buffer를 재사용했다. 별도 HLSL 또는 두 번째 trail renderer는 만들지 않았다.

### 1.4 차원술사 R helix 3타

- helix를 `0.50 / 0.70 / 1.05s`의 세 독립 occurrence로 분리했다.
- 각 occurrence는 burst 1이며 고유 stable ID를 사용한다.
- 원본 source lifetime `0.6..1.0s`가 다음 타격까지 누적되는 문제를 막기 위해 이 세 occurrence에만 `sourceScale.lifeTime=0.1`을 적용했다.
- generic particle lifetime 의미는 변경하지 않았다.
- layout-preserving materializer를 추가해 현재 손튜닝 문서의 다른 element를 재직렬화하지 않는다.

### 1.5 차원술사 A/Q voronoi

- Q `2050100` 문서는 수정하지 않았다. A와 Q는 DDS만 공유하며 별도 effect/element/transform이다.
- A Product가 unified document 전체를 네 번 재생하던 outer cue를 제거하고 `startms=0`, root `snapshot` 단일 cue로 바꿨다.
- A 문서의 8개 SwingHit family를 원본 시각 `0.25 / 0.60 / 0.90 / 1.30s`의 네 explicit occurrence로 확장했다.
- voronoi와 대응 slash가 같은 root를 한 번만 합성하고 일정한 local 상대 위치를 유지하도록 고정했다.
- A materializer는 target/source effect ID, canonical sourceNode, source family와 원본 네 timing을 fail-closed로 검증한다.

## 2. 자동 검증 결과

### 2.1 실행형 focused harness

- `ClientFrontendHarness.exe --effect-tool-preview-fast`: 5 PASS, failures 0
- `ClientFrontendHarness.exe --effect-artist-e-trail-fast`: PASS, failures 0
- `ClientFrontendHarness.exe --effect-dm-r-boundary-fast`: PASS, failures 0
  - 실제 활성 particle frame count `0 / 1 / 1 / 1 / 0`
- `ClientFrontendHarness.exe --effect-dm-a-voronoi-fast`: 5 PASS, failures 0
- ClientFrontendHarness x64 Debug build/link: PASS

### 2.2 Effect 문서와 publisher

- Artist E, DimensionMaster R, DimensionMaster A `--effect-document`: PASS
- `Tools/EffectPipeline/Test-EffectPipeline.ps1`: PASS
- `Publish-Effects.ps1 -Mode Validate -ResourceRoot Client/Bin/Resources`: PASS
  - Effect catalog entry 195개
  - visual-program sidecar 14 programs / 135 rows
  - `productMutation=false`
  - 동시 편집 pin 충돌 없음

### 2.3 Client 빌드

- Engine, Shared, Client x64 Debug 전체 build/link: PASS
- 격리 출력: `%TEMP%\LostArkArtistETrailClientDebug_6a8553f4180442e0afabc3ace66d5517\out\Client.exe`
- 실행 중인 기존 Client/Server 프로세스와 canonical 출력은 종료하거나 교체하지 않았다.
- 기존 FXC, C4819, third-party PDB 경고만 있었고 컴파일/링크 오류는 없었다.

### 2.4 정적 검증

- 수정 JSON parse: PASS
- 신규 Python AST/compile 및 idempotent check: PASS
- `git diff --check`: PASS. 기존 LF/CRLF 변환 경고만 존재
- untracked build artifact와 project/filter drift: 없음

## 3. 보존한 사용자 변경

다음 문서는 사용자가 동시에 손튜닝하던 기존 dirty 파일이므로 이번 commit/stage에서 제외한다.

```text
Data/Effects/Authored/effect.dimensionmaster.skill.2050100.unified.effect.json
Data/Effects/Authored/effect.dimensionmaster.skill.2050220.unified.effect.json
Data/Effects/Authored/effect.dimensionmaster.skill.2050230.unified.effect.json
```

R `2050180`과 A `2050210`은 이번 요청의 명시 대상이므로 현재 손튜닝을 보존하는 layout-preserving 변경만 포함한다.

## 4. 남은 수동 검증 경계

- Artist E: All Effects에서 E를 열고 Play All로 붓 끝을 따라 흰백색 ribbon이 생성·dissolve되는지 확인한다.
- 차원술사 BA: BA3 Open Editor가 BA3 하나만, ba1 selector가 BA1/BA2/BA4 중 하나만 재생하는지 확인한다.
- 차원술사 R: 세 타격에 helix가 하나씩 나오고 이전 타격과 누적되지 않는지 확인한다.
- 차원술사 A: 여러 시전 방향에서 voronoi와 slash의 상대 위치가 유지되는지 확인한다.

Tool Play All의 정상 순차 재생과 loop seek 직후 anchor는 지원한다. 임의 scrub이나 저 FPS catch-up 중 과거의
서로 다른 bone pose 전체를 fixed-step별로 재구축하는 offline history provider는 이번 범위에 포함하지 않았다.
최종 visual fidelity는 사용자가 직접 실행한 Client 화면으로 판정한다.
