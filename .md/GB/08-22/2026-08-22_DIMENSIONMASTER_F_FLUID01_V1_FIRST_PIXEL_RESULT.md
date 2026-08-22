# 차원술사 F Fluid01 V1 first-pixel 결과

## 완료선

도화가 F와 같은 현재 V1 기준인 `typed RT0 Base + 올바른 Sprite carrier`까지만 닫는다.
native ShaderMap/VF/MRT parity와 사용자 육안 승인은 이번 완료 조건이 아니다.

## 선택 복구 판정

| 원본 행 | Product stable ID | carrier 판정 | cohort 역할 |
|---|---|---|---|
| `Par_S_SWP_Chrono_Atk_01.ParticleSpriteEmitter_24` | `authored.source-particle.1ae3416ac205fee634b746a9` | `KEEP` | `CANARY` |
| `Par_S_SWP_Chrono_Rewind_02.ParticleSpriteEmitter_37` | `authored.source-particle.ed33fb10661afb8854e76957` | `KEEP` | `DATA_ONLY_EXPANSION` |

raw 69행 중 child `fx_w_pa_fd_01_3_tr`, parent `fx_mm_fluid_01_tr`인 Sprite는 위 두 행뿐이다.
따라서 신규 source 전량 복구 없이 현재 8행과 사용자의 transform/size/timing을 그대로 보존했다.

## 자동 검증

`verify_dimensionmaster_2050230_fluid01_first_pixel.py`는 다음을 한 번에 검사한다.

- authored → direct EffectCatalog → sealed runtime document Product join
- 두 행의 particle/Sprite carrier, opcode 17, 네 DDS lane
- opcode 17 HLSL include/dispatch와 `g_SourceTextureMask == 0x0f`
- DXT1/ATI2 원본 DDS를 직접 해제하고 source-positive particle color/dynamic carrier로 계산한 RT0 nonzero grid pixel

이는 화면 품질 승인이 아니라 first-pixel 구조 증거다. Effect Tool에서 사용자가 F를 재생해 두 원형 Fluid Sprite의 위치·크기·색을 확인해야 `V1_COMPLETE`로 승격할 수 있다.

## 실행 결과

- focused materializer + first-pixel unittest: `8/8 PASS`
- first-pixel RT0 nonzero grid: canary `773/1024`, data-only expansion `774/1024`
- materializer check: `stable`, artifact SHA-256 `015ba5f31fa918175a0cdbe71f5f2e23b88a6c94786821a31890e9d2663639d7`
- Effect data project registration: `PASS`, files `1862`, filters `210`
- Effect visual-program artifact check: `PASS`, programs `17`, rows `135`, `productMutation=false`

이번 변경은 authored Product 문서, sealed runtime 문서, EffectCatalog, C++ 또는 HLSL을 수정하지 않는다.
기존 opcode 17 packet/HLSL이 이미 nonzero이고 direct authored catalog와 sealed runtime join도 유효하므로
새 publish 산출물은 없다. 따라서 catalog publish는 이 검증 단위의 필수 변경이 아니며, 통합 브랜치의
Debug/Release 빌드와 사용자의 Effect Tool 육안 검증만 후속 경계로 남긴다.
