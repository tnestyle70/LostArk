# 2026-09-24 에스더 웨이 이펙트 원작 복원 RESULT

작성자: JS · 선례 `2026-09-24_ESTHER_BAHUNTUR_SOURCE_EFFECT_RESTORE_RESULT.md`(같은 드라이버)

목표: 손저작 V2 바인딩 17개(`esther.wei.*`)를 원본 소환 시퀀스 기반 full-restore 문서로 교체한다.

**현재 상태: 데이터·셰이더·C++ 반영, 코덱 하네스와 Debug 빌드 통과. 화면 확인은 사용자.**

## 1. 원본 근거

| 항목 | 값 |
|---|---|
| 사슬 | `CommonAction 53201`(발탄 레벨 1~3, 쿠크 11~13) → `CommonActionEffect 108` skill 532000 → 소환 시퀀스 |
| 파일 | `Esther_Waye1_*` 음성 파일 `531100/531110/532100/533100/533110/533500/532410000`은 FX 구성이 같다. `533510`만 카멘 `Darkness` 연출이 섞인 변형. `532100` 사용 |
| 카메오 | `NP_DPWI_00_dead_SK` `SK_Dochul`, 시퀀스 0.0s, 원점, scale 1.25 → 시각·위치 보정 없음 |
| 분신 | `MN_CUDC_00` `FX_CUDC_00_SK`(scale 3.5) 1.75s 원점 기준 (1.5, 우 2.0)m / 3.0s (1.5, 좌 2.0)m, `FX_CUDC_00_PRL_SK` 4.7s 뒤 7m |

손저작본의 분신 1733/2966/4700ms가 원본 시퀀스 시각과 일치했다.

### 시퀀스 해독 보강

- 자식 0개 Timer(78 byte, 지연만 가짐)가 있어 건너뛴다.
- 분신 스켈레탈 메시는 런타임이 재생하지 못한다. 거기 붙은 CEFAN 파티클(`Swing_01_01` ×2, `SpawnLightning_01`)은
  메시 스폰 위치의 월드 snapshot으로 두고 메시 scale 3.5를 곱했다. 본 로컬 오프셋·회전과 분신 애니메이션 추종은
  재현하지 않는다(추론).
- `FX_CM_02.Light.Par_MP_Light_01`(점광원만 있는 시스템)은 연속 생성 라이트라 투영기 계약(단일 burst)에 맞지 않아
  제외했다(`SKIPPED_SYSTEMS`).

## 2. 파이프라인 — `Tools/EffectPipeline/build_esther_wei_source_effects.py`

바훈투르 드라이버(공용화)의 상수만 바꾼다.

| 단계 | 결과 |
|---|---|
| acquire | notify 33, emitter 179, 재질 83. 손 창 파티클 `FX_R_Hand/FX_L_Hand` → `bip001-r/l-hand` |
| native | fresh 67(4684..4751), 재사용 13, 실패 0, 보류 3(검토 안 된 decal prefix 2, pass constant 1 — pass constant 1개는 설치본 재질 재사용으로 복귀) |
| 상한 확장 | 4735 → **4799**, 그룹 4736 신설(바훈투르와 같은 7곳 + 설치 스크립트 2개) |
| project | `effect.esther.wei.cameo.npc_sk_dochul.full.restore` **167 element**(root 151, 양손 follow 각 8), 보류 9(Lifetime 없는 emitter 7, decal 2), 10400ms, native program 79 |
| 리소스 | 참조 132개 존재. 새 파일 1개: `Effect/Esther/Wei/FullRestore/Textures/fx_tex_05/fx_m_mark_001.dds` — **Drive 전달 필요** |

## 3. 런타임 연결

- `Data/Effects/V2/Bindings/NPC_58700.effectv2bindings.json` bindings 17 → 3. 원본은 `out/WeiFX20260924/NPC_58700.effectv2bindings.backup.json`.
  남긴 3개는 도철 분신 스켈레탈 메시 `esther.wei.dochul_1/2/3`(V2 Mesh, `FX_CUDC_00_SK` + `npc_sk_dochul_01/02/03`)이다.
  V1 문서는 스켈레탈 FX 메시를 싣지 못해 사용자 첫 확인에서 "사자 같은 게 안 나온다"(09-25). 손저작 위치·시각이
  원본 시퀀스(우 2m·전방 1.5m / 좌 2m / 뒤 7m, 1.75/3.0/4.7s)와 거의 같아 그대로 둔다. `CNpc`는 V2 `Notify_Clip`과 V1 cue를 함께 재생한다.
- `NPC_58700.npcactioncues.json` 1 cue, `EffectCatalog.json` +1, `Effect_PresentationService.cpp` basis 목록에 `effect.esther.wei.`.

## 4. 검증

- 실행함: driver 전 단계 exit 0, JSON parse, `git diff --check`, 코덱 하네스(4736 그룹 포함 재빌드) Load/Validate/Drawable OK(웨이·바훈투르).
- Debug Product 빌드 PASS(바훈투르·웨이 함께) — `out/BuildPipeline/runs/20260924T100613376Z-debug-product.json`, Client OBJ 70·CSO 10, `Shader_VtxEffect{Mesh,Particle}Kouku4672/4736.cso` 생성.
- 하지 않음: 화면 확인(사용자).

## 5. 남은 경계

- 좌우 배치(±0.8m, ±2m)는 UE y→엔진 z 규약(니나브와 같음)을 따른다. 좌우가 뒤집혀 보이면 여기부터.
- 분신 파티클 위치·크기와 decal 2개 보류.
