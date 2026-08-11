# 2026-08-10 Artist 31470 F ShaderCache Oracle Phase 2 Result

## 결론

primary `sc_lv_eflobby_sl_class` native tail을 `271 shader objects → 25 FMaterialShaderMap rows → 534 shader references`로 bounded decode했다. 모든 shader reference는 descriptor ID, shader type FName, DXBC SHA와 D3D disassembly binding signature에 다시 결합된다.

25개 map의 `BaseMaterialId`는 모두 `2cf1c13888745046a6a2742eff3f6b86`이며 landscape static parameter set이다. exact source UPK에서 재해석한 도화가 23개 base Material ID는 이 ID와 하나도 일치하지 않고, 24개 MIC `FStaticParameterSet` semantic digest도 하나도 일치하지 않는다.

- base Material ID structural join: 0/23
- MIC exact static parameter set join: 0/24
- reconstructed numerically verified evaluator: 0/23
- execution/Product admission: false/false

따라서 설치 cache의 DXBC 271개는 유효하지만 도화가 F Material evaluator의 근거로 사용할 수 없다.

## 구조 증거

- shader objects: 271, descriptor ID coverage 271/271
- material shader maps: 25
- shader references: 534, unique descriptor IDs 271
- compressed LZ4/DXBC/D3DDisassemble: 271/271/271
- unique DXBC: 240
- map BaseMaterialId distinct count: 1
- source Material native state ID: 23/23 exact source/current equality
- source MIC static permutation resource: 24/24 structurally decoded
- MIC aligned 16-byte windows: 4,816, descriptor direct intersection 0

## 남은 blocker와 다음 취득물

- `INSTALLED_SHADER_CACHE_BASE_MATERIAL_ID_JOIN_0_OF_23`
- `INSTALLED_SHADER_CACHE_STATIC_PARAMETER_SET_JOIN_0_OF_24`
- `SOURCE_REVISION_SHADER_CACHE_PACKAGE_NOT_ACQUIRED`
- `PERMUTATION_CONSTANT_TEXTURE_SAMPLER_SEMANTICS_UNAVAILABLE_FOR_ARTIST`
- `DETERMINISTIC_NUMERIC_SAMPLE_ORACLE_UNAVAILABLE`

다음으로 필요한 최소 증거는 pinned source UPK와 같은 revision에서 생성된 ShaderCache export의 raw package SHA, 또는 pinned source UPK를 실제로 로드하면서 `FMaterialShaderMapId/FStaticParameterSet/shader object ID`를 함께 기록한 controlled runtime capture다. 이 identity join 뒤에만 G09가 bytecode evaluator와 versioned fixed numeric vectors를 작성할 수 있다.

## Portable receipt 보강

receipt format을 v2로 올렸다. validator는 다음을 독립적으로 재계산하거나 externally replay된 section digest에 묶는다.

- ShaderCache/DKV/D3D compiler raw identity
- 32 group/271 code row의 descriptor 및 compressed range exact coverage
- 271 shader object/25 map/534 reference의 range와 identity
- 23 Material/27 recipe/24 static set identity
- 4,816 MIC window projection과 271 descriptor intersection
- topology raw export identity와 denominator
- structural join, summary, blocker, admission

strict duplicate-key loader와 validate-only subprocess test를 추가했다. raw SHA, code range, Material key, direct match, descriptor ID, MIC window digest, topology export, structural join을 재봉인하는 공격은 모두 거부된다.

## 검증

- Python focused unit: 16/16 PASS
- shallow receipt validation: PASS
- deep external raw replay: PASS (16 source UPK, DKV, 1,596-export ShaderCache UPK, 30 inventory reports, pinned D3D compiler)
- 이미지 검증: 수행하지 않음
