# 2026-08-21 4캐릭터 Effect Cue와 BA 회귀 수정 결과

## 1. 완료 결과

- 차원술사 LMB hold의 180ms/100ms 자동 재전송을 제거하고 물리 up→down edge당 command 한 번으로 고정했다.
- 차원술사 nonterminal BA 입력창을 stage duration `1400/1500/1067ms`까지 열어 네 번의 명시적 클릭 연계를 보존했다.
- 애니메이션 Effect cue 문서는 구조·duplicate 검증 뒤 unavailable exact target만 격리하며, 유효 sibling cue는 계속 commit한다.
- 도화가 runtime 누락 4개를 publish하고 obsolete legacy ba4를 제거해 Effect runtime catalog를 192행에서 195행으로 갱신했다.
- 차원술사 BA `ba1/ba1/ba3/ba1`, 도화가 BA1~4, 워로드 17240 Hold Start/Charge/Release `ba1/ba2/ba3`을 실행형 회귀로 고정했다.
- 기존 도화가 E, 차원술사 R/A/T, 워로드 HOLD family와 사용자 Q/S/F authored 튜닝을 같은 publish snapshot에 보존했다.

## 2. 실행 불일치 원인

이전 Debug Client는 09:40 빌드였지만 인접 EffectCatalog는 07:26의 192행 구본이었다. 차원술사 A는 source 34 elements 대신 runtime 10 elements를 읽었고, 도화가 cue 4개는 runtime에 없어 클래스 cue 문서 전체가 rollback됐다. 새 integration build는 195행 DataFiles와 같은 source snapshot에서 생성했다.

## 3. 데이터 결과

- EffectCatalog: 195행 / unique ID 195 / unique path 195
- Valtan 99, LanceMaster 41, Artist 18, DimensionMaster 15, Warlord 22
- 신규 Artist unified 4개 포함
- obsolete `effect.artist.skill.31210.ba4` 제거
- catalog SHA-256: `4B933297F17445465313CD71F550DD89AF434ED21DD20C4CFE338508DE0134BE`
- 신규 content-addressed sealed JSON 13개: filename hash, byte SHA, source semantic parity 전부 PASS

## 4. 자동 검증

- `Publish-Effects.ps1 -Mode Publish`: PASS, 195 Effects
- `Test-EffectPipeline.ps1`: PASS
- Gameplay Balance Validate/Publish: PASS, 136 skills
- BalanceRuntimeSet Validate: PASS
- ClientFrontendHarness Debug build: PASS
- `--player-command-buffer-fast`: 8/8 PASS
- `--effect-tool-preview-fast`: 17/17 PASS
- `--four-class-effect-cue-parity-fast`: 4/4 PASS
- NetworkProtocolHarness Debug: failures 0
- Server Debug build: PASS
- Server BA 계약: exact timings/full-stage windows와 command/advance 항목 PASS
- Server 전체 contract의 기존 Valtan `48 hit stages / 71 pulses` 기대치 1건은 FAIL이며 이번 변경과 무관한 발탄 작업 경계로 남겼다.
- Client Debug full compile/link: PASS
- Client EXE: 23,530,496 bytes, 2026-08-21 11:18:40 KST
- Client EXE SHA-256: `4C19119067DDEB6AA00284BF2A1A5A63251CA8E97963D5E2B67F70A525747E9D`
- `git diff --check`, PlayerSkills JSON, catalog path/hash/identity: PASS

## 5. 리소스와 실행 경계

새 물리 DDS/WModel 파일은 추가하지 않았다.

- 도화가 E는 기존 `Effect/Artist/Textures/fx_m_trail_010.dds`를 참조한다.
- 차원술사 T는 기존 `Character/DimensionMaster/DimensionMaster_DimensionSummon.wmodel`을 참조한다.
- Git에 추가된 실행 데이터는 publisher가 만든 sealed Effect JSON 13개다.

Client 화면과 Effect 모양의 최종 판정은 사용자가 Character Select와 All Effects에서 수행한다. Esther 구현은 이 변경에 포함하지 않았으며 main 병합 뒤 새 브랜치에서 계속한다.

## 6. All Effects 편집 진입 후속 복구

- Valtan occurrence 트리를 합친 뒤 캐릭터 Product cue 행이 `Play Full Effect`와 VisualProgram만 표시하고, direct-authored 편집 진입은 별도 `Saved Unified Effects` 하위로 밀린 UI 회귀를 수정했다.
- 각 `Skill -> Stage/BA` Product cue가 exact `DIRECT_AUTHORED_DOCUMENT_V13` 문서와 공용 Unified Effect family 트리를 직접 표시한다.
- 공용 편집 버튼 이름을 `Open Editor`로 통일했으며, 해당 버튼은 기존 atomic Save와 same-revision Product hot reload 경로를 그대로 사용한다.
- 비정상·누락 authored identity는 임의 문서를 열지 않고 Product cue 행에서 unavailable 사유를 표시한다.
- `Effect_Tool.cpp`는 커진 translation unit만 `/bigobj`로 컴파일하며 다른 Client 파일의 컴파일 옵션은 바꾸지 않았다.
- Debug/Release `ClCompile`: 오류 0, C1128 재발 0.
- 최신 ClientFrontendHarness Debug build: 오류 0.
- `--effect-tool-preview-fast`: failures 0.
- `--effect-incremental-prewarm-fast`: failures 0.
- 새 Debug Client 링크와 화면 확인은 실행 중이던 11:18 구버전 Client를 사용자가 종료한 뒤 수행하는 수동 경계다.
