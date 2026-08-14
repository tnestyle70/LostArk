# 2026-08-13 도화가 31470 F V5 family transform/material 결과

## 현재 결론

V5는 사용자의 family isolation 화면을 근거로 V4의 material 복원 위에 source attachment basis,
SubUV, Decal, Ribbon 교정을 추가했다. 코드·candidate·registry 수직 슬라이스와 immutable
approval/authority publication은 최종 runtime catalog까지 연결됐고, Debug/Release material 및 WARP
자동 검증도 통과했다. 다만 자동 검증은 원작과의 육안 동일성을 대신하지 않는다. 사용자의 재촬영·서면
승인 전에는 Artist F 전체 복원 완료나 visual PASS를 선언하지 않는다.

현재 registry 분모는 exact replay `7`, bounded explicit `20`, unresolved fail-closed `6`, non-consumer
forbidden `2`로 총 `35`다. Core33은 draw-admitted `27`과 fail-closed `6`으로 나뉜다. 따라서 V5의
자동검증 완료는 모든 Core occurrence가 원본 ShaderMap/VF/pass까지 닫혔다는 뜻이 아니다.

## 사용자 캡처 보존

- 저장 위치: [V5](V5/README.md)
- 사용자 지정 사본:
  `C:\Users\user\Desktop\로스트아크이펙트이미지\도화가 F Restore Mesh particle, sprite particle, decal particle cascade ribbon`
- 최초 family 이미지 6개는 원본과 길이·SHA-256 6/6 일치한다.
- 추가 Sprite 위치 이미지 `08_sprite_particle_smoke_position.png`도 원본과 길이
  `1,238,124` bytes, SHA-256
  `4785F6C62F7E0E9479148CC0D052A029D139748C740C9517B6ADEAF007B2F5C5`가 일치한다.

## 원인 분석

### 공통 90° 위치·방향

Artist playable CModel은 source import에서 Client Y축 `-90°` basis를 소비한다. bone-follow
`#0..#4`는 model bone world를 통해 이 basis를 이미 상속한다. 나머지 30개 root-snapshot 행은 actor
root에 직접 결합해 source `+X`가 Client `+X`에 남았고, 캐릭터 정면 `+Z`보다 정확히 오른쪽 90°에
생성됐다.

교정은 global particle yaw나 Mesh geometry 회전이 아니라 root-snapshot attachment의 typed
`snapshotRootSourceBasisYawDegrees=-90`으로 구현한다. bone-follow, WModel preScale `0.01`, Mesh
dimensionless StartSize는 바꾸지 않는다.

### Sprite 흰 사각형과 연무

- `#5/#6` smoke-square: source StartSize `100 cm`, cue scale `3`, SizeMultiplyLife 최대
  `1.4163327`, burst `5/6`, lifetime `0.8..1.0 s`, cylinder radius `110/130 cm`다. 따라서 카드 크기는
  기본 `3 m`, 최대 약 `4.249 m`이며 100배 오류가 아니다. 추가 화면 오른쪽의 회색 연무 두 덩이는
  이 수치와 `fx_m_smokesq_01` silhouette에 부합한다.
- exact cache PS `3e38be239225c7498a67fd9d3d400d24`는 t0을 한 번 샘플한다. 현재 opcode10의
  current/next 이중 샘플·임의 lerp는 exact code와 다르므로 current atlas UV 단일 샘플로 교정한다.
- 흰 얼음/기포 사각형의 owner는 `#16`이다. 이 행은 burst `50`, source square alignment,
  StartSize `50..100 cm`와 cue scale `3`, SizeMultiplyLife `0..1`로 개별 폭 `0..3 m`인 카드를
  50장 만든다. 화면 pattern도 실제 bound texture `fx_a_ice_002`와 `fx_d_atypical_098`에 대응한다.
  exact static-set SHA `733e602d…`, PS `13e34fcd…` / DXBC `4f8d149d…`, VS `fece8af6…` /
  DXBC `072befb4…`까지 회수했지만 exact alpha는 아직 SceneDepth·projected screen carrier·fog·aux MRT를
  요구한다. 현재 ABI로 식을 꾸며 열지 않고 `#16` 하나만 `UNRESOLVED_FAIL_CLOSED`, draw/output none으로
  격리했다.
- `#19` 보라/먹물 sibling은 `60..80 cm × cue 3 × life 0.288..1.382`, 약 `0.52..3.32 m`,
  burst `23`이다. 개별 크기는 #16과 동급이며 화면 차이는 100배 scale이 아니라 `50 vs 23`의 수량과
  서로 다른 material coverage에서 발생했다.

### Decal

- 현재 Decal isolation에서 draw되는 것은 `#22` 하나다. exact source DDS
  `fx_a_decal_014`의 crack/radial pattern과 현재 화면이 일치하므로 잘못된 texture 대체가 아니다.
- `#20/#21`은 source StartSize `350×500 cm`, `200×500 cm`, cue scale `1.3`인 넓은 custom-lit
  LocalDecal이다. 원작의 검은 먹물 바닥 후보이며, official v974에서 engine-equality static SHA
  `1f5c2484…`, LocalDecal VS `5d79421d…` / DXBC `94072a22…`, PS `ef68ae7a…` /
  DXBC `d5a1d550…`를 회수했다. native six-SRV wire는 height `t0`, diffuse `t1`, fixed decal `t2`,
  normal `t3`, spec `t4`, emissive `t5`다. 하지만 exact VS projector/fog carrier와 RT0·Target2~5,
  두 parent-default DDS 및 actual pass/state admission이 현재 Rect RT0/RT1 ABI에 없다. 이 증거는 stable
  registry에 보존하되 두 occurrence는 계속 draw 전 fail-closed한다.
- 세 Decal의 source 회전과 size는 유지하고, 공통 root-snapshot basis로 위치·방향만 정면화한다.
  `#20/#21`은 공식 cache/UPK/DDS/projector 정보를 추가 추출해 안전하게 열 수 있을 때만 복원한다.

### CascadeRibbon과 붓

- Ribbon `#3`은 붓 WModel이 아니라 `WP_SDM_R_Battle/b_wp_1` history다. 수치는 `40 Hz`, point
  life `2 s`, width `0.15 m`, tile `6 m`, tessellation `0.05 m`, max `500`이다.
- 검은 굵은 고리의 원인은 불투명 DDS container alpha를 coverage로 사용한 것이다. source static
  `use_gra_r_channel=true`, scalar `gra_pow=1.5`에 따라 main texture red channel의 거듭제곱과 trail
  alpha가 coverage를 소유하도록 교정한다. source 근거 없는 시간/UV RGB 보정과 RT1은 제거한다.
- 붓은 별도 Mesh occurrence `#4`다. Ribbon family isolation에서 보이지 않는 것은 정상이며, 사용자
  캡처에서 식별되지 않았으므로 V5 핵심 네 결함 이후 별도로 추적한다. 화면의 초록 캐릭터를 carrier로
  오인했던 초기 관찰은 철회했다.

## 확장성 계약

- character import basis는 attachment data가 소유하며 Artist order 하드코딩으로 다른 캐릭터에
  전파하지 않는다.
- stable occurrence + recipe + family + static-set + renderer/VF가 visual program 선택의 단일 key다.
- family adapter는 공용으로 유지하고 source 수치와 resource/channel은 occurrence packet으로 전달한다.
- ShaderCache recovery와 `PROJECT_TUNED` 수동 값은 별도 fidelity로 기록한다. 공식 data를 더 추출할
  수 있으면 수동 튜닝보다 먼저 닫는다.

## 자동 검증

실제로 실행해 통과한 항목만 기록한다.

- [x] runtime-program builder focused unit 12/12 및 deterministic regeneration
- [x] Debug x64 ClientFrontendHarness build/link
- [x] Release x64 ClientFrontendHarness build/link
- [x] Debug/Release reconstructed material gate 각 21/21, failures `0`
- [x] Debug/Release WARP family·occurrence·60 Hz transform sweep, failures `0`
- [x] 관련 Sprite/Ribbon HLSL `fxc`
- [x] scoped `git diff --check`

최종 published catalog는 `27,147,692` bytes, SHA-256
`1029365468DFB9FB4E17C166A2F2FD5D4870EC87121267D1BC74CF8B17F64460`이다. WARP는 WARP device,
77개 DDS SRV/sampler staging, prepared-cache denominator, first production EffectObject render,
family/occurrence isolation, position canary, 60 Hz 전체 sweep과 Lance BA1 회귀까지 검사했다. Debug/Release
모두 1.5초 표본에서 `active=28`, `candidate=27`, `submitted=23`, `suppressed=5`, `failed=0`이었다.
이는 시간 표본에서 활성화된 행의 수치이며 registry 전체 분모 `27 draw + 6 fail-closed`와 모순되지 않는다.

ProjectAudit는 사용자가 퇴역·삭제한 상태라 실행하거나 완료 근거로 기록하지 않는다. Client/UI는
에이전트가 실행하지 않는다.

## 남은 근거 경계

- `#16`은 흰 대형 Sprite card의 owner를 확정했지만 SceneDepth·projected screen·fog·aux MRT가 없어
  occurrence 하나만 draw 전 억제한다.
- `#20/#21`은 검은 먹물 LocalDecal의 exact static-set, VS/PS DXBC와 6-SRV wire를 회수했지만 현재
  Rect carrier에는 native projector/fog/MRT/pass가 없어 억제한다. `#22` crack만 source DDS로 그린다.
- `#33`은 RT1 distortion shader가 아니라 SceneColor를 읽어 RT0에 합성하는 pass다. engine-owned
  constant, fog, actual VF/pass/state가 없어 계속 억제한다.
- 붓은 Ribbon이 아니라 Mesh `#4`다. 사용자가 나머지를 확인한 뒤 필요할 때 별도 occurrence로 추적한다.
- bounded 행은 recovered equation 또는 source 수치의 명시적 부분 복원이며 raw DXBC/native pass admission과
  동일하지 않다. 추가 official package/cache/VF 자료가 확보되면 수동 튜닝보다 먼저 이 경계를 좁힌다.

## 사용자 수동 검증

사용자가 Effect Tool의 동일 family isolation과 stable occurrence isolation으로
재촬영한다. 위치·방향, #16 흰 카드, #5/#6 연무, #22 crack, #20/#21 먹물 Decal, #3 Ribbon을 원작과
직접 비교한다. 사용자의 서면 승인 전에는 V5 visual PASS로 기록하지 않는다.
