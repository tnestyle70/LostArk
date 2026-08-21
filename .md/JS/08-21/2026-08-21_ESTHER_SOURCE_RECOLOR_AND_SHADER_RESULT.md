# 2026-08-21 에스더 원본 재도색·왕관 제거·전용 셰이더 RESULT

PLAN: `2026-08-21_ESTHER_SOURCE_RECOLOR_AND_SHADER_PLAN.md`

## 0. 결론

실리안(59030)을 원본 LookInfo 계약대로 재쿠킹해 왕관 제거·홍채 복구·머리색·검
발광을 반영했고, 에스더 3종과 스크린 컷인을 전용 셰이더
`Shader_VtxEstherNpc.hlsl`로 분리했다. **컷인이 이상해진 실측 원인 하나를 수리**:
공용 `Shader_VtxAnimMeshBinary`에 `EffectModelCueMaskedExact` 패스가 중간 삽입되며
ScreenCutin 패스가 4→5로 밀렸는데 컷인 서비스는 계속 4(현재는
EffectModelCueTranslucent)를 바인드하고 있었다. 전용 셰이더는 패스 순서를
계약으로 고정(0 Default / 1 Shadow / 2 ScreenCutin)해 재발을 구조적으로 막는다.

웨이·바훈투르는 실측 결과 **재도색 델타가 없다** — LookInfo 오버라이드가 기본
MI와 동일하고 색 파라미터가 없으며(웨이 머리 twotone은 08-20 완료), 색이 이상해
보였다면 위 셰이더 문제였을 가능성이 크다. 재쿠킹하지 않았다.

## 1. 실측 (PLAN §1 + 이후 확정)

- LookInfo 슬롯 규칙: 오버라이드는 같은 패키지 소속 슬롯에 순서 대응 — 세 NPC 검증.
- 실리안 slot7=왕관 서브메시, `-3_MI`가 mask(전면 0)·visible.A=0으로 통째 숨김.
- 홍채 텍스처는 이름만 eyelash인 `pc_gn_00_eyelash_d`(알파 마스크 실물 확인).
- 검: basecolor_color 0.453 곱 + 붉은 emissive(보석 부위만 밝은 e 텍스처 실물 확인).

## 2. 변경

### 파이프라인 (Desktop\buildScript, Git 밖 — .bak-0821 백업 있음)

- `cook_npc.py`: LookInfo materialOverrides 실적용(위 규칙), per-target
  `--exclude name:mat`, `--diffuse/--emissive name:mat=path` 지원.
- `bake_esther_silian_tex.py` 신규: hair_d×haircolor_base(알파 보존, RLE 지원),
  wp_d×basecolor_color, wp_e×emissive_color → `_fixed_tex_esther\`.
- 눈은 기존 `Bake-EyeTexture.ps1`(base+iris×0.818/0.603/0.533, 1:1 — irissize
  0.57 스케일은 미적용, 육안 후 필요 시).

### 재쿠킹·배치

- `Npc_59030.wmodel` 재쿠킹: 서브메시 9→8(왕관 제거), eye/hair/wp_d 베이크 교체,
  wp emissive 신규. validate OK, Compare-Skeletons ALIGNED 104/104,
  Compare-InverseBind maxDelta 1.58e-3(08-20과 동일한 무해 편차).
  `Client/Bin/Resources/Character/NPC/Npc_59030/`에 배치(구 미참조 tga 일부 잔존).

### Client

- `Shader_VtxEstherNpc.hlsl` 신규(vcxproj/filters 등록):
  `Bind_DeferredMaterialInputs` uniform 계약 유지, dye/effect-cue 패스 제거,
  패스 인덱스 고정 계약을 헤더에 명시.
- `Loader::Ready_AnimatedMeshShader`: 전용 셰이더 프로토타입 추가 등록.
- `NpcCatalog.json` + `ActorCatalog`: optional `shaderProfile`("esther"만 허용,
  그 외 parse 거부) — 에스더 3 entry에 지정.
- `ClientReplication`: NPC 스폰 시 shaderProfile로 셰이더 태그 선택.
- `EstherCutinPresentationService`: 전용 셰이더 + `CUTIN_SHADER_PASS 4→2`.

## 3. 실행한 검증

```text
bake 4종 + cook exit 0 + validate_wmodel OK
Compare-Skeletons ALIGNED / Compare-InverseBind 1.58e-3(기존과 동일)
Client x64 Debug build            PASS
ClientFrontendHarness             failures 45 — 기준선 동수(신규 회귀 0)
Publish-WorldGameplay Validate    전 Area 통과
```

## 3.1 사용자 확인 반영 (2026-08-21)

- 컷인 라이팅 3회 반복: 이전의 "정상"처럼 보이던 밝기는 잘못된 패스의 무라이팅
  풀브라이트였음. 최종 계약 = 고정광(ambient 0.85 + directional 0.3) + Blinn-Phong
  스페큘러 + **최종 combine과 동일한 감마 lift(1/2.2)** — 씬이 어두워도 컷인
  밝기는 불변. **사용자 확인: "지금 좋음"**.
- 실리안 머리색: 메시 기본 MI는 진갈색이나 사용자 원작 판정은 금발 —
  같은 패키지의 `hairdt` 틴트(0.713, 0.537, 0.416)를 스트랜드 텍스처에 단순
  곱(결·명암 보존)으로 채택. 음영 공식(`tint×min(1,R×1.8)`)은 단색으로 뭉개져
  폐기. **사용자 확인: "얼추 비슷"**.

## 4. 남은 것 / 경계

- **사용자 육안 확인 필요**: 발탄 진입 → Ctrl+Z/X/C — 실리안 왕관 없음·홍채·
  갈색 머리·검 붉은 발광, 컷인 라이팅 정상(전용 셰이더 ScreenCutin), 웨이·
  바훈투르 셰이더 교체 후 인상.
- 근사 항목: 눈 irissize 미적용(1:1), 검 cm(diffusecolor_a/b/c) 영역 재도색
  미적용, a~d 갑주 occlusion_color 미적용 — 육안 후 필요 시 후속.
- 팀 배포는 팀장 물리 폴더 계약(로컬 Resources만 갱신). 이펙트/사운드는 인계 유지.
