# 2026-08-13 도화가 31470 F V5 family transform/material 구현 계획

## 목표

사용자가 직접 격리한 MeshParticle, SpriteParticle, DecalParticle, CascadeRibbon 결과를 원작 이미지와
비교하고, 화면을 줄이거나 숨기는 보정이 아니라 UE3 원본 수치와 실제 런타임 소비 계약의 차이를
교정한다. 위치·회전·크기·SubUV·coverage·Ribbon history를 서로 분리해 검증하며, 근거가 부족한
occurrence는 V4의 fail-closed 경계를 유지한다.

사용자 입력 이미지는 [V5](V5/README.md)에 원본 바이트 그대로 보존한다. 여섯 장의 family 진단
이미지와 한 장의 원작 비교 이미지는 수동 진단 입력이며 자동 visual PASS 증거로 승격하지 않는다.

## 확정된 원인과 구현 경계

1. playable Artist CModel은 source import에서 Client Y축 `-90°` basis를 갖는다. `b_wp_1` bone-follow는
   이 basis를 이미 상속하지만 `SNAPSHOT_ROOT` occurrence는 actor root에 직접 결합해 source `+X`가
   캐릭터 정면 `+Z`가 아니라 오른쪽 `+X`로 남는다. attachment에 source-root basis를 명시하고
   root snapshot 행에만 `-90°`를 적용한다.
2. Mesh geometry preScale `0.01`과 dimensionless StartSize는 그대로 유지한다. 전역 Mesh scale, WModel
   pitch 또는 bone-follow occurrence를 변경하지 않는다.
3. 초기에는 `#5/#6` smoke-square를 흰 사각형의 1순위 후보로 두되, family capture의 pattern과 exact
   cache code를 occurrence별로 대조해 owner를 확정한다. 조사 결과 `#5/#6`은 오른쪽의 회색 연무이며
   source StartSize `100 cm`, cue scale `3`, SizeMultiplyLife 최대 `1.4163327`로 기본 `3 m`, 최대 약
   `4.249 m`인 정상 수치다. exact PS는 t0을 한 번만 샘플하므로 임의 current/next lerp를 제거한다.
   흰 얼음/기포 card의 실제 owner `#16`은 최대 `3 m` card `50`장이며, SceneDepth·projected-screen·
   fog·aux MRT가 없는 현재 ABI에서 occurrence 단위 fail-closed한다.
4. Decal isolation에서 보이는 crack은 `#22`의 exact source DDS `fx_a_decal_014`다. 잘못된 대체
   texture가 아니다. 원작의 넓은 검은 먹물 Decal 후보인 `#20/#21`은 LocalDecal projector/VF/pass와
   full texture graph가 닫히지 않아 계속 fail-closed한다. 위치는 공통 source-root basis로 교정하되
   각 Decal의 source rotation과 size는 바꾸지 않는다.
5. Ribbon `#3`은 붓 mesh가 아니라 `WP_SDM_R_Battle/b_wp_1`의 무기 history다. source 계약은
   `40 points/s`, point life `2 s`, width `15 cm`, tile `6 m`, tessellation step `5 cm`, max `500`이다.
   붓 WModel은 별도 Mesh occurrence `#4`다.
6. Ribbon DDS container alpha는 불투명하지만 source static은 `use_gra_r_channel=true`이고
   `gra_pow=1.5`다. Ribbon coverage는 main texture red channel의 거듭제곱과 trail alpha가 소유하게
   하고, source graph 근거가 없는 시간/UV 색 보정과 RT1 출력을 금지한다.

## 구현 순서

1. attachment 문서/codec/builder에 root-snapshot source basis를 typed field로 추가한다.
2. runtime program 35행을 재생성해 root snapshot 30행은 `-90°`, bone-follow 5행은 `0°`임을 검증한다.
3. playback에서 cue·emitter·particle local transform 전체에 source basis를 한 번만 합성하고 60 Hz
   covariance harness로 위치와 회전을 검증한다.
4. `#5/#6`의 source SubUV index·flip과 recovered one-sample PS를 대조하고, `#16`의 exact PS/VS가
   요구하는 depth/fog/MRT 입력을 현재 carrier가 제공하지 못하면 그 occurrence만 억제한다.
5. `#22` DDS identity와 `#20/#21` suppression을 유지한 채 Decal transform covariance를 검증한다.
6. Ribbon `#3` material static/channel 계약을 strict pin하고 red-channel coverage로 교정한다.
7. builder focused unit, Debug/Release harness build, reconstructed runtime/material fast gate, WARP family/
   occurrence sweep, 관련 HLSL `fxc`, `git diff --check`를 실행한다.
8. Client/UI는 실행하지 않고 사용자가 같은 family isolation으로 V5 화면을 직접 비교한다.

## 확장 가능한 복원 규칙

- 캐릭터 방향 보정은 Artist order 번호나 화면 좌표에 하드코딩하지 않는다. source character import
  basis와 attachment mode가 소유하는 typed transform으로 저장하고 다른 캐릭터는 자신의 basis를
  명시한다.
- material 선택은 stable occurrence, recipe, family, engine-equality static set, renderer/VF의 유일 join을
  사용한다. 같은 profile 번호나 texture 이름이 비슷하다는 이유로 다른 캐릭터 프로그램을 공유하지
  않는다.
- SubUV, Decal projector, Ribbon history는 family adapter의 공용 데이터 계약으로 구현하되 실제 수치와
  source capability row는 occurrence packet으로 전달한다.
- recovered ShaderCache 식, typed MIC 값, 수동 튜닝값을 같은 fidelity로 표시하지 않는다. 원본 정보가
  더 필요하면 공식 cohort package/ShaderMap/VF/CB/SRV를 우선 추출하고, 마지막 수단의 손 튜닝은
  별도 `PROJECT_TUNED` 근거와 사용자 수동 승인으로만 연다.

## 완료 기준

- root snapshot과 bone-follow가 서로 다른 source-basis 계약을 소비하고 전역 회전 보정이 없다.
- source `+X 60 cm` cue가 캐릭터 정면으로 투영되며 cue scale과 particle size가 그대로 보존된다.
- `#5/#6`은 recovered one-sample PS 계약을 소비하고, 흰 card owner `#16`은 근거 없는 대체식으로
  그려지지 않는다.
- `#22`는 exact crack DDS임을 유지하고 `#20/#21`은 근거 없이 열리지 않는다.
- Ribbon은 폭·history 수치가 source와 일치하고 불투명 container alpha를 coverage로 사용하지 않는다.
- V4 registry의 fidelity 분류, `#32/#34` non-consumer, gameplay/Effect Tool shared cache 계약이 회귀하지 않는다.
- 자동 검증과 사용자의 수동 visual 판정을 별도로 기록한다.
