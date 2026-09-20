# 2026-09-20 쿠크 거미 피격 공포 얼굴 원본 복구 결과

## G1. 원본과 실제 소비자

- 거미 action 4219776의 피격 결과는 buff 422181026 `KoukuSaton_Drakness_Fear`를 경유해 `fx_mn_rpct_06_x.par_x_rpct_screenpp_01_1_loc_int`를 사용한다. 기존 `boss.kouku.fear.face` V2의 `fx_g_rpcz_01`과 다른 얼굴이다.
- 원본 `fx_d_symbol_103_loc_int` 얼굴/검은 마스크 5개와 링·광원·연기 7개의 총 12개 source emitter를 별도 `effect.kouku.fear.screen.full.restore`에 보존했다. 원본 공유 leaf, texture, native 재질, burst, per-particle 분포/곡선은 변경하지 않았다.
- 첫 얼굴의 수명은 .8초, 뒤의 큰 얼굴은 .3초 지연과 .7초 수명 및 1.5→1.8 SizeOverLife를 갖는다. 원본 전체가 반복되는 구조는 아니므로 파생 alias의 emitter duration 1초/loop 0은 사용자의 반복 요청에 따른 **PROJECT_TUNED** 정책이다. .3/.4초 지연과 source particle 꼬리를 유지하며 Server FEAR occurrence가 방출/정리를 제한한다.
- 원본 Buff CEFParticleData 위치 `[0,0,-10]` cm와 scale 1을 읽었다. Client 단위 위치는 `[0,-.1,0]`이다. EPAL_Z·.25 turn·음의 UE Z 깊이를 camera-facing plane으로 소비하기 위한 attachment adapter는 `[0,90,-90]`도이다. 이는 런타임 좌표계 변환이며 원본 Euler 값이라고 기록하지 않는다. Buff의 미해독 generic camera flag를 복원했다고 주장하지 않는다.

## G2. 코드와 연결

- `KoukuSaydonPresentationPlayer::Make_SourceAnchorSampler`는 이 정확한 alias이며 모든 visible emitter가 camera-local/follow이고 model cue·source transform·bone 연결이 없을 때만 현재 camera source anchor를 허용한다. world-space 원본에 필요한 camera history 검사는 그대로 유지한다.
- FEAR Product loader가 해당 V1 asset을 loop-to-occurrence로 구성한다. native infinite emitter의 기존 `SourceLoopEndSeconds`와 FEAR 종료 cleanup을 사용한다. 엔진에 별도 화면 이펙트 경로나 신규 ABI를 만들지 않았다.
- Composition logic `kakulsaydon.g1.logic.35`의 effect resource만 새 `kakulsaydon.effect.94502eeace8ae6ae07bf`로 연결했다. 기존 scene profile, 3초 FEAR 수명, 1초 effect delay, 다른 logic, 기존 V2 리소스는 보존했다.
- Catalog, Tree, 프로젝트 None 항목을 등록했다. 최신 디스크 stable ID field 병합과 SHA 재확인/backup/원자 교체를 사용했다. 설치 시 Composition revision 1908이며 이후 다른 저작 변경으로 증가할 수 있다.

## G3. 검증과 남은 확인

- 최신 실제 C++ codec/Playback의 Load, Validate_Drawable, 안정 roundtrip, 두 번 rewind: 12/12 emitter, 1,212 particle samples, 20,612 checks PASS.
- 실제 `Make_ParticleSpriteWorld`까지 사용한 moving-camera 수치 검사: 4회 반복 동안 5개 얼굴/마스크 레이어 모두 camera 앞 2.6/3.1m, 정면과 수평·수직 방향 정상. 폭 2→3.58571로 원본 크기 변화 확인. 긴 꼬리의 다음 pulse overlap 302 frames, 방출 종료 후 잔존 0. 총 4,704 particle samples/120,017 checks PASS.
- 최신 `KoukuSaydonPresentationPlayer.cpp` TU `/Zs` PASS. 새 JSON parse, codec validator 및 `git diff --check` PASS.
- 근거: `out/FearRepair20260920/source.receipt.json`, `install.receipt.json`, `fear-probe.log`, `codec.log`, `compile.log`. 직접 Client/UI 실행이나 화면 확인은 하지 않았다. 카메라 FOV별 화면 점유율/최종 모양과 Server 피격 통합은 사용자 화면 검증 및 root 통합 빌드 이후 확인이 남는다.
- 후속 통합: root의 Composition revision 1909 명시 publish가 완료되었다. `out/RaidRepair20260920/kouku-publish.log`의 Product 85 patterns / 473 stages / 8 bundles 결과를 확인했다. 실제 피격과 화면 재생을 실행한 검증은 아니다.
