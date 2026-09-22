# 유령 발탄 Character Select 미리보기 재질 경로 복구 결과

대응 [구현 계획](2026-09-22_GHOST_VALTAN_CHARACTER_SELECT_PREVIEW_IMPLEMENTATION_PLAN.md). 기준은 `GB/collider-pattern-bug-fix`, HEAD `0ebd23cd1f04a6a65a09125e00ef2ac319ce0d97`의 기존 dirty 작업 폴더다. 사용자 첨부 이미지는 직접 열람했으며 Client 실행·조작·캡처는 하지 않았다.

## G01. 확인한 원인과 코드 반영

첨부 화면에서 머리와 상체는 매우 옅고 도끼와 손·발 일부는 진하게 남아 있다. `boss.valtan.ghost`의 `pBossArchetypeId`는 null이므로 Character Select는 `CBody_Valtan` 대신 `CharacterPreviewPanel::Select_Asset -> CPart_Body`로 ghost를 만든다. 설치된 모델·catalog는 동일해도 실제 render consumer가 다르다.

`CPart_Body::Resolve_TranslucentSourcePass`에 native84가 빠져 있었다. 이 때문에 모델의 3재질이 NONBLEND/pass0으로 그려지고 `Shader_SourceCharacterMaterial.hlsli`의 4×4 ordered coverage가 원본 alpha를 잘라냈다. 전투 `CBody_Valtan`에만 있던 BLEND/pass10 검증으로는 이 누락을 검출할 수 없었다.

`Client/Private/Part_Body.cpp`에 one-sided pass 상수와 native84 선택을 추가했다. 이 기존 resolver의 소비자들이 함께 바뀌므로 Initialize에서 translucent mesh를 인식하고 Late_Update에서 BLEND에 등록하며, 기본 opaque draw에서는 제외하고 Render_Translucent에서 scene lights/base/light/bone을 바인딩한 뒤 기존 pass10에 제출한다. native6/7/18/99의 pass9와 다른 프로그램의 기존 경로를 유지한다. 재질 opacity·색·조명·셰이더·모델·donor·데이터는 수정하지 않았다.

## G02. 실제 검증

- `Part_Body.cpp`를 현 Desktop include와 Debug `/c` 설정으로 컴파일했다. 성공, 제품 EXE 링크 없음. 기존 vcxproj와 filters 등록을 확인했다.
- 수정 전 파일의 resolver와 수정 후 실제 resolver를 추출해 컴파일한 routing probe는 native84 `0 -> 10`을 확인했다. 나머지 native0~108, null, legacy84의 기존 선택 보존까지 220 checks / failures0이다. 제품 객체 실행을 대신하는 테스트라고 기록하지 않는다.
- 실제 설치 ghost WModel, catalog의 3개 native84 재질/texture/parameter, 실제 cinematic donor(합산 290 clip)를 기존 `CModel`, material binding, 설치된 Engine DLL/CSO로 읽어 D3D WARP에서 비교했다. 모든 mesh의 material/light/bone/pass binding HRESULT가 성공했다.
- 모델 preTransform은 preview와 같은 `.01 * Y(-90°)`, world 위치는 party01 spawn 기준 preview offset `(2.5,0,0)`이다. saved `CharacterSelect.camera.json`의 offset/FOV와 현재 `scene.character-select.warm-high-key.v1`, 게시 maplights 4개(합계 5 lights)를 사용했다. reference viewport는 960×540, clip은 `mesh_idle_battle_1`, time0이다. 사용자 실행 메모리의 실제 카메라·편집 pose를 읽었다는 뜻은 아니다.

| mesh / material | 기존 pass0 covered | 수정 pass10 covered | 수정 pass10 colored | 수정 pass10 RGB max | nonfinite |
|---|---:|---:|---:|---:|---:|
| 0 / mn_rpbf_01-1_mi | 288 | 1346 | 1346 | 0.636512 | 0 |
| 1 / mn_rpbf_01_2-1_mi | 329 | 1821 | 1820 | 1.363281 | 0 |
| 2 / mn_rpbf_01_1-1_mi | 200 | 858 | 857 | 1.542374 | 0 |

covered는 alpha>1e-5, colored는 RGB max>1e-5다. 비교는 각 mesh를 분리한 수치이므로 3개 count를 합산해 화면 픽셀 수라고 부르지 않는다. pass0의 출력은 GBuffer diffuse, pass10은 alpha blend된 forward color이므로 두 RGB를 밝기 개선 배율로 비교하지 않는다. 기존 coverage가 원본의 낮은 alpha 영역을 버리는 경로와 수정된 연속 blend 경로를 구분한 증거다. background/depth/occlusion을 포함한 전체 Character Select 화면 검증은 아니다.

`git diff --check`는 변경 CPP와 전용 PLAN/RESULT 범위에서 통과했다. 빌드·routing probe의 기존 혼합 인코딩 헤더 warning은 해당 헤더를 재저장하지 않고 보존했다.

## G03. 백업과 남은 확인

백업 및 로컬 증거는 Git 제외 `out/GhostCharacterSelect20260922/`에 있다. `Part_Body.before.cpp`의 SHA-256은 `8E2478E8923EAB4457AA43A41A733255BCC7CC5A59477251869379CC1AAA57E9`, 수정 후는 `3AE4B97E88AE8F20D1826FB0E1592489F7F3F66E074DE1681A82B90CA62232F0`이다. 교체 직전 baseline hash를 재확인했고 UTF-8 no BOM/CRLF를 유지했다. `prepare_probe.py`, `preview_probe.cpp`, `routing-probe.log`, `mesh1-pass*.log`, `mesh2-pass*.log`와 tool stdout이 검증 증거다.

전체 제품 빌드·설치·commit/push는 이 하위 작업에서 하지 않았다. 배포 후 사용자가 Character Select의 Ghost Valtan을 다시 선택해 머리·상체의 이어지는 반투명 표시와 도끼 부착을 확인해야 한다. 원작 fidelity나 실행 중 Server의 phase3 표시까지 이번 probe로 승인하지 않는다.
