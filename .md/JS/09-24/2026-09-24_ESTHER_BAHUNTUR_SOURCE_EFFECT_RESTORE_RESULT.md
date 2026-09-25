# 2026-09-24 에스더 바훈투르 이펙트 원작 복원 RESULT

작성자: JS · 선례 `.md/JS/09-22/2026-09-22_ESTHER_NINAVE_SOURCE_EFFECT_RESTORE_RESULT.md`

목표: 손저작 V2 바인딩 8개(`esther.balthorr.*`)를 원본 소환 시퀀스 기반 full-restore 문서로 교체한다.

**현재 상태: 데이터·셰이더·C++ 반영, 코덱 하네스와 Debug 빌드 통과. 화면 확인은 사용자.**

## 1. 원본 근거

| 항목 | 값 |
|---|---|
| 사슬 | `EFTable_EpicSkill 1010` slot 3 → `CommonAction 53202`(바훈투르) → `CommonActionEffect 108` ValueA NPC 53300, ValueB skill 533000 → `SkillEffect Key 12` → `XMLData/Projectile/*.loa` |
| 파일 | 번호가 어긋난다. `533000.loa`는 실리안 시퀀스다. `Esther_Bahunturr1_Attack1_*` 음성을 가진 파일은 `531200/532200/532210/533600`이고 SkillEffect 행 수만 다르다. `532200` 사용 |
| 카메오 | `MN_YOBR_00_SK` `SK_BreathOfArcturus`, 시퀀스 1.6s, 원점 기준 X −420cm, scale 1.05, 무기 `WP_MN_YOBR_00` |

### 시퀀스 해독

Timer 본문 첫 int가 자식 수이고 지연 float은 마지막 자식 끝 −16 byte다(니나브 규약과 같음). 다만 이 파일은
한 Timer에 자식이 2~4개라 공용 문자열 스캐너가 자식 수를 문자열 길이로 읽고 첫 자식을 삼킨다. 드라이버는
`CEFSequenceSummonsAction*` FString을 길이 접두로 직접 찾는다.

| 시퀀스(s) | 문서(s) | 시스템 | 위치(NPC 기준, m) |
|---|---|---|---|
| 0.0 | 0.0 | `Atk_05_01`, `Atk_07_01`(×1.5) | 전방 4.2 |
| 2.0 | 0.4 | `Atk_01_01`, Light(y 1.5), `ZoomBlur_03`(×0.4), `FilmNoise_01` | 전방 1.4 |
| 1.6+1.2 | 1.2 | `Atk_02_01` | 본 `b_wp_1` follow (+0.65) |
| 1.6+1.4 | 1.4 | `Trail_01` | 본 `b_wp_1` follow |
| 4.0 | 2.4 | `Atk_04_01`, `Atk_06_01`, Light(y 2), `ZoomBlur_01`(×1.5), `Atk_03_01`(y 0.5) | 전방 4.2 |
| 6.0 | 4.4 | `Atk_08_01`(×1.5) | 전방 4.2 |

Server는 에스더를 착지 지점에 스폰하자마자 `npc_sk_breathofarcturus`(4100ms)를 재생한다. 그래서 문서 시각 =
시퀀스 − 1.6s, 위치 = 원본 + 4.2m 전방이다. 손저작본의 망치 1200ms·타격 2400ms가 이 환산과 정확히 일치했다.
카메오 등장 전 행(시퀀스 0.0의 마법진 두 개)은 클립 시작으로 당겼다. 이 부분은 원본보다 1.6초 늦다(추론).

## 2. 파이프라인 — `Tools/EffectPipeline/build_esther_bahuntur_source_effects.py`

웨이 복원 때 에스더 공용 드라이버로 일반화했다(모듈 상수로 에스더를 바꾼다). 카메오 오프셋은 SkeletalMeshFX payload의
마지막 균등 scale 3개 앞 24 byte 위치에서 읽으며, 일반화 뒤 바훈투르 문서는 바이트 단위로 같았다.

니나브 driver와 같은 구성(이난나 driver의 acquire/native/project 재사용, 니나브의 `MN_PPNN_00 4189601` 템플릿).
`MN_YOBR_00.loa`에는 PlayParticleEffect가 하나도 없어 템플릿은 payload 레이아웃만 빌린다.
시퀀스 CreateFX의 scale(+136)은 `transform.scale`에 곱한다(니나브는 버렸다).

| 단계 | 결과 |
|---|---|
| acquire | notify 14(월드 12, `b_wp_1` 2), emitter 80, 재질 43 |
| native | fresh 36, 재사용 6, 보류 0, 실패 0 |
| 상한 확장 | 4671 → **4735**, 그룹 4672 신설(이후 웨이 복원에서 4799·그룹 4736까지 확장, `..._WEI_..._RESULT.md`). `install_kouku_gate1_native_shaders.py`(assert·range 2곳·Decal/Trail 정규식), `install_kouku_gate1_native_materials.py` LAST, C++ `Effect_DocumentRenderer_Geometry/MaterialBinding/Particles.cpp`·`Effect_NativeScreenPostMaterial.cpp`, `Shader_VtxEffectDecal/Trail.hlsl`. 그룹 4672 hlsli·Vtx wrapper·vcxproj/filters·`Effect_ShaderFamily.h`는 설치 스크립트가 생성 |
| project | `effect.esther.bahuntur.cameo.npc_sk_breathofarcturus.full.restore` **77 element**(root 63, `b_wp_1` follow 14), 보류 3(Lifetime 모듈 없는 emitter), 9000ms, native program 41 |
| 리소스 | 참조 65개 전부 이 PC Resources에 존재. `Effect/Esther/Balthorr` FX_SM 메시는 Ninave/Wei 설치본과 바이트 동일(x100 cook). 모루 `BG_RAD_VALTAN_A` 2종은 같은 폴더 기존 설치본(배율 미대조) |

## 3. 런타임 연결

- `Data/Effects/V2/Bindings/NPC_59060.effectv2bindings.json` bindings 8 → 0. 원본은 `out/BahunturFX20260924/NPC_59060.effectv2bindings.backup.json`. V2 문서 `esther.balthorr.*`는 파일로 남는다.
- `Data/Effects/NpcActionCues/NPC_59060.npcactioncues.json` 1 cue(0ms), `EffectCatalog.json` +1.
- `Effect_PresentationService.cpp` NPC 본 0.01 basis 목록에 `effect.esther.bahuntur.` 추가.
- `Level_Loading.cpp` Esther roster 선준비에는 `NPC_59060`이 이미 있다.

## 4. 검증

- 실행함: driver 전 단계 exit 0, 변경 JSON parse, `git diff --check`, 코덱 하네스(새 `.inl`로 재빌드) Load/Validate/Drawable OK.
- Debug Product 빌드 PASS(바훈투르·웨이 함께) — `out/BuildPipeline/runs/20260924T100613376Z-debug-product.json`, Client OBJ 70·CSO 10, `Shader_VtxEffect{Mesh,Particle}Kouku4672/4736.cso` 생성.
- 하지 않음: 화면 확인(사용자).

## 5. 남은 경계

- `FX_Post` ZoomBlur/FilmNoise는 screenPost로 투영됐다. 화면 강도는 사용자 관찰로 판단.
- 카메오 등장 전 마법진 1.6초 선행은 런타임 구조상 재현하지 않았다.
