# 2026-08-10 Artist 31470 F ShaderCache Oracle Phase 2 Plan

## 목표

Phase 1의 opaque native tail을 UE868/licensee16의 실제 직렬화 경계까지 해석한다. 설치 ShaderCache가 도화가 31470 F의 23개 base Material ID와 24개 MIC `FStaticParameterSet`을 포함하는지 exact identity로 판정한다. 조인이 없으면 추측 evaluator를 만들지 않고 필요한 다음 취득물을 구체화한다.

## 구현 범위

1. primary cache의 271개 descriptor ID를 native shader-object table과 exact FName/ID로 결합한다.
2. 뒤따르는 `FMaterialShaderMap` table을 count, range, parameter-array cardinality, shader-reference count로 bounded decode한다.
3. exact source Material 23개의 native state ID와 MIC 24개의 `FStaticParameterSet`을 source UPK에서 다시 decode한다.
4. source base ID/static set과 decoded cache map을 exact 비교하고 각 map의 shader reference를 DXBC/disassembly signature에 결합한다.
5. Phase 1 receipt를 format v2로 올리고 external raw identity, code range, native key, 4,816 MIC windows, topology, decision/blocker를 validator가 독립 재도출한다.
6. duplicate JSON key와 coordinated re-seal 공격을 fail-closed test로 고정한다.

## 판정 경계

- proximity, GUID 유사성, register 이름만으로 Material join을 만들지 않는다.
- exact source shader map이 없으면 `RECONSTRUCTED_NUMERICALLY_VERIFIED`는 0/23으로 유지한다.
- graph provenance는 계속 `RECONSTRUCTED_GRAPH`; execution/Product admission은 false다.
- 이미지·육안 비교, runtime/HLSL/Source/Geometry 수정은 범위 밖이다.

## 완료 조건

- shader objects/maps/references denominator가 structural decoder와 독립 fixture에서 일치한다.
- source base Material 23개와 MIC static set 24개의 join denominator를 명시한다.
- raw SHA, code offset/size, native key, direct match, descriptor ID, window digest, topology export를 재봉인한 receipt가 모두 거부된다.
- focused unit, shallow/deep audit, JSON parse, `git diff --check`를 통과한다.
