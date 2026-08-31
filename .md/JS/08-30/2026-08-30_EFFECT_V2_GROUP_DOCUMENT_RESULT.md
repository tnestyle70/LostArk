# 2026-08-30 Effect Tool v2 Group 문서 + Multiply 블렌드 RESULT

작성자: JS · branch `feature/effect-v2-multiply-blend` (main `9e24aaee` 기준)
계획서: `2026-08-30_EFFECT_V2_GROUP_DOCUMENT_PLAN.md`

## 구현 완료

### Multiply 블렌드 (Effect V2)

- `Shader_EffectV2_Common.hlsli`: `BS_EffectV2Multiply`(RT0 `Dest_Color × Inv_Src_Alpha`), `PS_EFFECT_V2_MULTIPLY`(alpha premultiply),
  `EFFECT_V2_PASSES`에 pass 5 `MultiplyDepth` / 6 `MultiplyNoDepth`
- `Shader_EffectDecalV2.hlsl`: pass 2 `Multiply`. 본체를 `Decal_Shade(input, uniform bool bPremultiply)`로 공유
  (엔트리 함수 중첩 호출은 fxc internal error `argument pulled into unrelated predicate`를 내서 이 형태로 고정)
- `EffectV2_Object.h/.cpp`: `BLEND_MODE::MULTIPLY`(뒤에 추가), pass 매핑, Outline pass 5 → 7
- `EffectV2_Document.cpp`: `"blend": "Multiply"`, `Effect_Tool_V2.cpp`: Blend 콤보
- 결과 = 화면색 × lerp(1, 텍스처색, alpha)

### Group 문서 (G1~G4, 계획서 그대로)

- 문서: `Data/Effects/V2/Groups/<id>.effectv2group.json`, schema `lostark.effect-v2-group` v1,
  `durationMs` + `children[]{effectId, startMs, durationMs, stop: Kill|Deactivate, offset, yawDegrees}`
- 바인딩 row: `effectId` 또는 `group` 정확히 하나. 기존 바인딩 파일 무변경
- 런타임: `Expand_Binding`이 그룹을 lane 시계 위 자식 pending으로 펼침, `Apply_ChildStops`가 Kill(`Finish`)/Deactivate(`Stop_Emission`)를
  lane당 1회 적용, `Set_FollowLocal`로 follow 중에도 자식 offset/yaw 유지, free-group lane(`Play_Group/Stop_Group/Group_Seconds/Advance_FreeGroups`)
- 툴: `Group...` 창(New/Load/Save/Rescan, 자식 편집, 타임라인 바 + playhead, Play/Stop Preview), Attach 창 `Bind As` 콤보와 group row
- 검증기: 그룹 문서 검사, `group` 바인딩 참조, 자식을 bound로 인정, 미바인딩 그룹 거부, `rotation: World` 허용(C++ 키와 일치)

새 C++ 파일 없음, vcxproj/filters 변경 없음. Server/Shared/Effect V1 무변경.

## 자동 검증 (실행함)

- fxc `fx_5_0` 6개 V2 셰이더 컴파일 통과
- Client x64 Debug 전체 빌드·링크 통과 (V2 CSO 6개 재생성)
- `Tools/EffectToolV2/test_validate_effect_v2.py`: 11 tests OK
- `Tools/EffectToolV2/validate_effect_v2.py`: `75 authored, 75 bindings, 0 groups, 38 textures` 성공
- `git diff --check` 클린. `*.py`는 LF, C++는 CRLF로 저장소 규칙에 맞춤
- 세션 시작 시 `Invoke-BuildAndRegression.ps1 -Configuration Debug -AllowLocalEffectResources` (Core) 통과 — main `9e24aaee` 기준, 이 변경 전

## 수동 검증 (사용자, 2026-08-30 로컬 Server 127.0.0.1 + Client)

사용자가 다음을 직접 확인하고 통과로 판정했다.

1. 웨이 `NPC_58700` `Runtime spawns on target` — 도철 3종이 이전과 같이 스폰 (기존 문서 바인딩 회귀 없음)
2. Blend `Multiply` — 어두운 텍스처가 바닥을 검게 누르고 alpha 가장자리가 부드럽게 빠짐
3. `test.group`(dochul_1/2/3, Start 0/500/1000, Duration 0/800/0, Kill/Deactivate/Kill) Save → 파일 생성, 재로드 시 값 유지
4. `Play Preview` — 자식이 Start 순서로 스폰, Deactivate 자식이 스폰만 멈추고 잔여 소멸, playhead 동작, 종료 후 버튼 복귀; Offset (2,0,0) 자식만 2 m 옆
5. `Bind As` group 바인딩 + `Target Bone (follow)` — row `group:test.group`, 파일에 `"group"` row, 타깃 이동 시 offset 유지

검증용 `test.group` 문서와 바인딩 row는 삭제했다(`Groups/` 폴더 없음, `NPC_58700` 바인딩 원복).

## 남은 것 (이 변경 범위 밖)

- 발탄 포탈 자식 문서 7개 + 그룹 + `BOSS_VALTAN` `ghost-finale.step-02~09` 바인딩 8줄
- 메시 파티클(돌 파편 ×21)은 V2 `Particle`에 메시 인스턴싱이 없음
- HDR `colorMul > 1` 허용 여부
- v2 런타임 B안 경로 자체는 계속 팀장 합의 항목
