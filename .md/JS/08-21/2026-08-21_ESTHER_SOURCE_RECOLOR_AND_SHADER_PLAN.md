# 2026-08-21 에스더 원본 재도색·왕관 제거·전용 셰이더 PLAN

요청: 실리안·웨이·바훈투르를 원본 색으로 재도색. 실리안은 왕관 제거(원작 59030은
왕관 없음)와 홍채 복구. 재쿠킹 후 에스더 전용 셰이더를 사용(공용
`Shader_VtxAnimMeshBinary`가 여러 사람 변경으로 불안정).

## 1. 실측 근거 (완료)

- LookInfo 슬롯 규칙 확정: 오버라이드는 같은 패키지 소속 메시 슬롯에 **순서 대응**.
  세 NPC 모두 검증(실리안 A~D→slot0~3, `-3_MI`→slot7; 웨이·바훈투르는 기본과 동일).
- 실리안 slot 7 = 왕관 서브메시(`np_lrsa_01e_mi`, `-2` 디퓨즈가 왕관 텍스처).
  `-3_MI`는 mask 32×32 전면 검정 + `mask_variation_visible=(1,1,1,0)` → dot=0 →
  슬롯 전체 숨김. 컨버터 `--exclude-material`로 동일 결과.
- 실리안 눈 `np_lrsa_00_eye_mi`(pc_eye_msk): base `pc_gn_00_eyebase_d` + 홍채
  `pc_gn_00_eyelash_d`(이름과 달리 홍채, 알파 마스크) × iriscolor(0.818,0.603,0.533).
  irissize 0.57은 v1에서 미적용(1:1 합성) — 육안 확인 후 필요 시 스케일 추가.
- 실리안 hair `haircolor_base`(0.342,0.257,0.198) 단색 — 웨이 방식 베이크.
- 실리안 검 `wp_np_lrsa_00_mi`: basecolor_color(0.453,0.446,0.452) 곱,
  emissive `wp_np_lrsa_00_e` × (0.845,0.040,0.033). cm(diffusecolor_a/b/c) 재도색은
  마스터 셰이더 의미 미확보라 v1 제외 — 육안 확인 후 후속.
- 웨이·바훈투르: 오버라이드=기본 MI, 색 파라미터 없음(웨이 머리 08-20 완료).
  재쿠킹 불필요. 색 인상 차이는 셰이더 쪽에서 해결.

## 2. 작업 항목

### A. 실리안 재쿠킹 (buildScript, 저장소 밖)

1. `cook_npc.py`: LookInfo materialOverrides를 확정 규칙(같은 패키지 슬롯 순서
   대응)으로 적용 — 오버라이드 MI의 텍스처로 remap을 해석. "not applied" 경고 제거.
2. 눈: `Bake-EyeTexture.ps1` base=np_lrsa 없음 → `pc_gn_00_eyebase_d` +
   iris=`pc_gn_00_eyelash_d`, tint=(0.818,0.603,0.533) → baked TGA를 eye 슬롯 remap.
3. 헤어: hair_d에 haircolor_base 베이크(단색, R게인 규칙은 웨이와 동일 검토).
4. 검: diffuse에 basecolor_color 베이크 + `--emissive-remap`으로 `wp_np_lrsa_00_e`
   (emissive_color 베이크) 추가.
5. 왕관: `--exclude-material np_lrsa_01e_mi`.
6. validate_wmodel + Compare-Skeletons/InverseBind + Resources 물리 폴더 배치.

### B. 에스더 전용 셰이더 (Client)

1. `Client/Bin/ShaderFiles/Shader_VtxEstherNpc.hlsl` 신설 — VtxAnimMeshBinary의
   에스더 필요 최소 경로(디퓨즈·노멀·스펙큘러·emissive, discard a<0.3, 디퍼드
   출력)만 고정. 다른 담당 변경에서 격리.
2. Loader에 `Prototype_Component_Shader_VtxEstherNpc` 등록(발탄 로더 + Development
   프리뷰 경로).
3. `NpcCatalog.json`에 optional `shaderProfile`("esther") 추가 — 파서 검증 포함,
   CNpc/컷인/프리뷰가 이 프로파일이면 전용 셰이더 태그 사용. 필드 없으면 기존
   셰이더 유지(다른 NPC 무영향).
4. 컷인 서비스(`EstherCutinPresentationService`)도 같은 셰이더 사용.

### C. 검증

- 쿠킹: validate/Compare 스크립트 전부 OK.
- Client 빌드 + NpcCatalog parse + `Publish-WorldGameplay.ps1 -Mode Validate`.
- ClientFrontendHarness 기준선(45) 동수 확인.
- 화면 판정(왕관 제거, 홍채, 머리·검 색, 셰이더)은 사용자 육안 — 발탄 진입 후
  Ctrl+Z/X/C 소환 + 컷인.

## 3. 경계

- 원본 마스터 셰이더(HLSL) 미확보 항목은 베이크 근사이며 근사 사실을 RESULT에 남긴다.
- `Client/Bin/Resources` 배치는 로컬 실행용이고 팀 배포는 팀장 물리 폴더 계약.
- 이펙트/사운드는 계속 이펙트 담당 인계 범위.
