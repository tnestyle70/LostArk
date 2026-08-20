# 발탄 109줄 붉은 소용돌이 하늘 구현 결과

## 결과

- `VALTAN_ARENA_BREAK_109`의 Server 권위 6단계에 붉은 구름과 검은 aperture 하늘을 연결했다.
- 기존 ValtanPhase 맵 레이어 6개를 cache하되 원형 합성에 적합한 4개만 표시한다. 새 이미지, 새 모델 런타임, Client 로컬 패턴 진행은 추가하지 않았다.
- 사용자 재확인 요청 뒤 카메라와 레이어를 화면 투영 기준으로 다시 감사했다. 1차 카메라는 상승 시작에 하늘을 보지 않고 DROP 후반에 두 레이어가 갈라졌으며, Additive SpaceHole만으로는 검은 중심을 만들 수 없었다. 이 결함을 후속 교정했다.
- TAKEOFF 시작부터 DROP 종료까지 카메라가 typed sky의 공통 중심과 focal ring을 화면 중앙에 유지하고 IMPACT에서 arena landing anchor로 복귀한다.
- `SPACEHOLE_CORE`는 검은 Alpha aperture, `CHAOS_RING`은 저광도 붉은 Additive ring, 두 cloud는 원형 burgundy Alpha disc로 presentation override한다. 사각 번개와 streak는 숨기며 나머지 맵/캐릭터에는 이 분기가 적용되지 않는다.
- RECOVERY 종료에는 opacity와 권위 aperture 상태가 0으로 돌아가며, 패턴 중단·disconnect·level exit에도 원본 transform/profile/visibility를 복구한 뒤 presentation을 숨긴다.
- 자동 검증과 빌드는 통과했으며 최종 화면 fidelity 판정은 사용자가 수행해야 한다.

## 구현 내용

### 하늘 데이터와 시간축

- `ValtanCinematicCamera.json`을 formatVersion 3으로 갱신했다.
- 109 단계 순서는 `TAKEOFF / DROP / IMPACT / IMPACT_HOLD / WIDE_REVEAL / RECOVERY`, 총 6,270ms다.
- 모든 sky cue는 다음 stable seed ID를 사용한다.
  - 붉은 구름: `VALTAN_PHASE_CHAOS_CLOUD`
  - 검은 aperture: `VALTAN_PHASE_SPACEHOLE_CORE`
- 새 Server snapshot은 권위 action age로 seek하고 같은 snapshot 사이에는 최대 0.1초의 local delta만 보간한다.
- 이전 stage들의 저작 duration과 rotation speed를 합산해 stage 경계와 late join에서도 회전 위상이 끊기지 않는다.
- 빈 ID, 경로 형태 ID, 128바이트 초과 ID, 잘못된 format version은 기존 문서를 보존한 채 거부한다.

### 실제 렌더 소비자

- Level 초기화 때 seed placement의 source level을 찾아 다음 6개 non-batched map object를 한 번만 cache한다.
  - `VALTAN_PHASE_CHAOS_CLOUD/ELECTRIC/RING`
  - `VALTAN_PHASE_SPACEHOLE_CLOUD/CORE/STREAK`
- 한 레이어라도 없거나 seed가 중복·동일 그룹·overlap이면 level 준비를 실패시키며 일부만 표시하지 않는다.
- 여섯 stable asset ID의 exact set을 검증해 임의의 같은 source-level 3개로 조용히 대체되지 않게 했다.
- 여섯 레이어의 typed policy를 하나만 두고, `CHAOS_ELECTRIC`과 `SPACEHOLE_STREAK`는 `NONE + hidden`으로 고정한다. 화면에 보이는 레이어는 `CHAOS_CLOUD`, `CHAOS_RING`, `SPACEHOLE_CLOUD`, `SPACEHOLE_CORE` 정확히 네 개다.
- 네 레이어는 기존의 서로 다른 높이와 큰 placement scale을 사용하지 않는다. 모두 `[156.03, 74, -122.06]` 공통 중심·공통 면에 놓고, Loader가 이미 적용하는 0.01 model pre-transform 위에 절대 scale `2.75 / 2.45 / 2.25 / 1.90`을 적용한다.
- 검은 중심은 `SPACEHOLE_CORE`의 raw UV 중심에 어두운 radial Alpha mask를 만들고, 붉은 외곽은 `CHAOS_RING`에 raw UV annulus를 만든다. 두 cloud도 raw UV 원형 edge mask와 burgundy procedural disc를 사용해 사각 텍스처 변·모서리의 alpha를 0으로 만든다. texture pan은 내부 breakup으로 계속 움직인다.
- `Shader_VtxMeshBinary`는 캐릭터 무기와 props도 공유하므로 모든 map draw 성공·실패 뒤 presentation profile/strength를 `NONE/0`으로 복구한다.
- `CMapAssetObject::Render`와 `Render_Shadow`는 draw 직전에 visibility를 다시 확인해 level 전환 직전 queue에 들어간 한 프레임 잔상을 차단한다.
- camera override 종료와 sky teardown을 분리해 카메라 cue 부재가 활성 sky를 지우지 않게 했다.

### 재검토한 카메라 구도

- 기존 TAKEOFF 0ms 카메라는 붉은 Chaos 중심을 사실상 뒤쪽에 두었고, DROP 700ms에는 SpaceHole 중심이 세로 반시야각 밖으로 벗어났다.
- TAKEOFF 첫 프레임부터 위를 보며, TAKEOFF terminal pose를 DROP 전체에서 유지한다.
- DROP 마지막과 IMPACT 첫 프레임의 eye/lookAt/FOV를 같게 해 전투장 복귀 시작에 화면 점프가 없다.
- typed policy의 공통 중심과 focal ring의 실제 proxy half-span 5.12, 절대 scale 2.45를 TAKEOFF/DROP의 30Hz 전 tick에 투영한다. 중심은 NDC x/y 0.25 안, 16:9 equivalent-area 반경은 0.45~0.75에 있어야 자동 검사를 통과한다.

### 109 카메라 활성 상태의 Client 종료

- 사용자 스크린샷의 `DirectXMathConvert.inl:2104 / pDestination`
  Assertion을 종료 순서 문제로 확정했다.
- 기존에는 `CGameInstance::Release_Engine`이 `CPipeLine`을 먼저 파기한 뒤
  `CLevel_ValtanArena` 소멸자를 호출했다. 소멸자의
  `End_CinematicCamera -> CCamera::End_PresentationOverride -> Update_PipeLine`
  경로가 이미 파기된 pipeline에 view/projection 행렬을 쓰려고 했다.
- 현재 Level을 pipeline, frustum, renderer보다 먼저 파기하도록 순서를
  바꿔 카메라 복구와 Loading worker 종료가 살아 있는 엔진
  서비스에서 완료된다.
- `ClientFrontendHarness --valtan-camera-fast`에 reset 순서 검사와 WARP
  Engine의 실제 `Release_Engine()` 실행 회귀를 추가했다. synthetic Level
  소멸자에서 활성 camera override가 정상 복구되는 것까지 확인한다.

## 자동 검증

- 실제 실행 checkout `C:/Users/USER/source/졸업팀폴/LostArk`에 구현과 로더 호환 수정 적용: PASS
- 실제 hydrated map data의 6 placement, 6 catalog row, 6 WModel과 6 texture identity: PASS
- `Publish-GameplayBalance.ps1 -Mode Validate`: PASS
  - 6 player profiles, 136 skills, 108 damage profiles, 1 boss, 2 armour plates, 33 patterns, 129 stages, 67 audition occurrences
- x64 Debug Client 전체 rebuild/link: PASS
- x64 Debug Server build: PASS
- Debug `ClientFrontendHarness --valtan-camera-fast`: 36 PASS, failures 0
- 종료 순서 추가 후 Codex worktree Debug Engine / UpdateLib / Client link: PASS
- 종료 순서 WARP 회귀 2건을 포함한 Debug `--valtan-camera-fast`: failures 0
- 첨부 화면 차이 교정 뒤 Codex worktree Debug Client 전체 rebuild/link 및 `Shader_VtxMeshBinary.cso` compile: PASS
- 첨부 화면 차이 교정 뒤 Codex worktree Debug `--valtan-camera-fast`: 31 PASS, failures 0
  - typed 6-layer policy / visible 4-layer / hidden electric·streak: PASS
  - profile 1/2/3 raw-UV 원형 edge mask: PASS
  - TAKEOFF/DROP 공통 중심과 focal ring 화면 점유 범위: PASS
- Codex worktree Release ClientFrontendHarness / Client 전체 rebuild/link 및 shader compile: PASS
- Codex worktree Release `--valtan-camera-fast`: 31 PASS, failures 0
- 실제 실행 checkout의 최신 visual delta 적용 뒤 Debug ClientFrontendHarness / Client link 및 shader compile: PASS
- 실제 실행 checkout Debug `--valtan-camera-fast`: 45 PASS, failures 0
  - `Engine Shutdown Destroys The Active Level Before Its Dependencies`: PASS
  - `Engine Shutdown Restores An Active Cinematic Camera Before Releasing Services`: PASS
- Debug `Server.exe --contract-test`: failures 0
  - 109 단독 audition은 109 패턴만 queue하며 159/115/100 패턴을 섞지 않는 계약 포함
  - 109 외벽 30개 붕괴, 109에서 바닥 6 sector 유지, 109 leap/landing anchor 계약 포함
- `git diff --check` 및 conflict marker 검사: PASS

실제 실행 checkout의 최종 Debug `Client.exe`는
`2026-08-21 00:29:57.619 KST`, `23,124,480 bytes`이며 Client/UI는
에이전트가 실행하지 않았다. HLSL source-contract가 Windows CRLF를 LF로만
가정해 처음 1건 실패한 문제는 입력에서 `\r`을 제거하도록 교정했고,
Debug/Release Codex harness와 실제 실행 checkout harness를 모두 다시 빌드해
각각 `31/31/45 PASS`, `failures 0`을 확인했다.

실행 checkout의 별도 방어구 작업이 `ValtanEncounter.json`에 추가한
`armorRequirement`를 Client read-only encounter loader도 exact enum으로 검증해
받아들이도록 연결했다. 이 누락이 있으면 Server는 빌드돼도 Client camera/sky 문서가
연쇄적으로 로드되지 않아 109 하늘이 보이지 않는다.

## 실행 배치와 수동 판정 경계

- 사용자가 평소 실행하는 `C:/Users/USER/source/졸업팀폴/LostArk` checkout에 구현을 적용했다.
- 그 checkout의 `Client/Bin/Resources/Map/ValtanPhase`에 필요한 6개 WModel과 texture가 모두 존재한다.
- 새 코드가 들어간 `Client/Bin/Debug/Client.exe`와 `Server/Bin/Debug/Server.exe`를 다시 빌드했다.
- 에이전트는 Client/UI를 실행하거나 화면을 대신 판정하지 않았다.
- 자동 투영 검사는 typed 공통 중심, focal ring의 audited proxy half-span·절대 scale, 카메라 구도를 보장하지만 실제 최종 pixel의 Alpha/Additive 합성, 색·구름 밀도, 발탄이 원본 화면의 작은 중심 실루엣과 같은지는 보장하지 않는다. 이 항목은 사용자 육안 판정이 필요하다.

## 사용자 직접 확인 절차

1. `C:/Users/USER/source/졸업팀폴/LostArk`에서 x64 Debug Server를 먼저 실행한다.
2. x64 Debug Client를 실행하고 Lobby에서 `Valtan`을 눌러 발탄 맵에 들어간다.
3. 캐릭터가 나타날 때까지 기다린다. `Valtan Pattern Audition` 창은 자동으로 표시되므로 F1을 누를 필요가 없다.
4. `Reset + Play 109 Only (Outer Wall 30 x 12)`를 누른다. 이 버튼은 라디오 선택과 무관하게 Server에 109줄만 직접 요청한다.
5. TAKEOFF가 시작되는 즉시 카메라가 상공을 보는지, DROP이 끝날 때까지 붉은 다층 구름과 중앙의 어두운 소용돌이가 화면 중심에 함께 유지되는지 확인한다.
6. IMPACT에서만 카메라가 전투장으로 자연스럽게 복귀하는지 확인한다.
7. RECOVERY 뒤 레이어가 완전히 사라지는지, 같은 버튼을 반복해도 이전 구름 잔상이 남지 않는지 확인한다.

첨부 화면과의 색, 화면 점유 크기, 회전 속도, 카메라 구도 최종 일치 여부는 사용자 육안 판정 전까지 visual PASS로 기록하지 않는다.
