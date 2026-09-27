# 2026-09-26 마하라카 G06 — action 4225601 particle 원본 복구 RESULT

범위는 설계서 `2026-09-26_MAHARAKA_SOURCE_RESTORATION_HANDOFF.md`의 **G06만**이다.
입력은 `2026-09-26_MAHARAKA_G01_ACTION4225601_RESULT.md`의 연결표다.

실행 저장소 `C:\Users\USER\source\졸업팀폴\LostArk`, branch `codex/main-ship-maharaka-0926`,
HEAD `a84bbcd45ff995c70bf847c683f1d159cf1821a6`. Visual Studio가 열려 있어(pid 30616) 빌드하지 않았다.

**G06은 완료되지 않았다.** 원본 분석은 끝냈고 재질 추출에서 도구 경계에 막혔다.
particle 문서를 작성하지 않은 것은 실패가 아니라 의도적 판단이며 이유는 7절에 있다.

## 1. 이번 G에서 실제 바꾼 파일

정본 코드·데이터·Resources 변경 **없음**. `Data/Effects/` 아래 새 파일 0개,
`EffectCatalog.json` 수정 없음, `Client/Bin/Resources` 추가 0개.

산출물은 전부 gitignore 대상 `out/MaharakaContinuation_20260926_193455/G06/`에만 있다.

| 파일 | 역할 |
|---|---|
| `MokoParticles/FX_MN_ISMP_00.particle-graph.json` | 원본 particle graph 28 system |
| `targets_inventory.json` / `.txt` | 4종의 emitter→LOD→module 전개 (987줄) |
| `lifetime_analysis.json` | emitter loop·duration·lifetime·kill flag |
| `dependencies.json` | 재질 15종·메시 2종·외부 패키지 7개 |
| `package_resolution.json` | 논리→물리 UPK 대조 |
| `material_extraction_receipt.json` | 재질 추출 15건 전부 exit 1 기록 |
| `material_export_probe.json` | 추출 실패 원인 조사 |
| `offset_vector_scan.json` | Ground offset 1,500건 전수 대조 |
| `ground_axis_scan.json` | CEFActionParticleData 배치 블록 |
| 스크립트 8개 | 위 산출물 생성기(읽기 전용) |

## 2. 원본 근거

- 원본 particle 패키지 `FX_MN_ISMP_00` → 물리 `YGI3SBI3SJHBW3I11GHJMHQC6.upk`.
  추출 결과 `graphObjectCount 3392`, `particleSystemCount 28`, `propertyErrorCount 0`,
  `particleEmitterReferenceCount 383`, `missingParticleEmitterTargetCount 0`, exit 0.
- 참조 규칙을 graph 전체 9,671건으로 검증했다.
  `packageIndex > 0` → 이 패키지 export이고 `exportIndex == packageIndex - 1`(이름 일치 8,833건),
  `packageIndex < 0` → **다른 패키지 import**(838건). 이 규칙 없이는 모듈 트리를 펼칠 수 없다.
- 그래프의 시스템 이름은 전부 소문자다(`par_g_ismp_face_01`). notify는 대문자 혼용이다.

## 3. 4종 module inventory

| 원본 시스템 | notify | emitter | LOD 행 | 주요 module |
|---|---|---|---|---|
| `Par_G_ISMP_Face_01` | 005 | 4 | 8 | subUV 4, orbit 4, sphere location 4, rotation/rate 4 |
| `Par_G_ISMP_Attk01_Water_01` | 007 | 14 | 26 | TypeDataMesh 6, cylinder 6, velocity 18, orientationAxisLock 6, acceleration 4 |
| `Par_G_ISMP_Attk01_WaterGround_01` | 014 | 10 | 20 | TypeDataMesh 4, cylinder 14, velocity 18, `efparticlemodulevelocityoverlifetime` 2 |
| `Par_G_ISMP_WaterFinish_01` | 015 | 7 | 14 | cylinder 12, subUV 8, rotation 10, velocity 8 |

`efparticlemodulevelocityoverlifetime`은 UE3 표준이 아닌 **원작 자체 module**이다(WaterGround 전용 2행).
기존 소비자가 이 module을 지원하는지는 확인하지 못했고 미해결로 남긴다.

## 4. duration 0의 의미 — 원본으로 확정

설계서와 G01이 미해결로 남긴 항목이다. **원본 4종에서 예외 없는 상관을 찾았다.**

| 시스템 | notify duration | `emitterloops` | 자체 종료 |
|---|---|---|---|
| Face | 5.408441066741943 | 전 8행 **미직렬화 → UE3 기본 0 = 무한** | 없음 |
| Water | 2.7232859134674072 | 전 26행 **기본 0 = 무한** | 없음 |
| WaterGround | 2.9024178981781006 | 전 20행 **기본 0 = 무한** | 없음 |
| **WaterFinish** | **0.0** | 전 14행 **명시적 1 = 1회** | **최대 1.800초** |

원본 계약은 이렇게 읽힌다.

- **양수 duration** = 무한 loop 시스템이므로 소유자가 그 시각에 **정지시켜야 한다.**
- **duration 0** = 시스템이 스스로 끝나므로 소유자가 **정지시키면 안 된다.** 실제 수명은 1.800초다.

1.800초는 `particlespriteemitter_2`의 `delay 0 + loops 1 × duration 1.0 + lifetime 최대 0.80`이다.
**임의 1초로 바꾸거나 이벤트를 삭제하면 원본과 달라진다.**

kill flag도 원본이 나뉘어 있어 tail 처리가 시스템마다 다르다.

| 시스템 | `bKillOnDeactivate` | `bKillOnCompleted` | 정지 시 거동 |
|---|---|---|---|
| Face | 전 8행 미설정 | 전 8행 미설정 | 남은 입자가 수명까지 살아남음(tail) |
| Water | **22/26 True** | **22/26 True** | 즉시 소멸(tail 없음) |
| WaterGround | 10/20 True | 8/20 True | emitter별로 갈림 |
| WaterFinish | 4/14 True | 4/14 True | emitter별로 갈림 |

## 5. 의존 closure — 재질 15종·메시 2종이 외부 7개 패키지에 있다

`FX_MN_ISMP_00` 안에는 재질도 메시도 없다. 전부 import다.

| 논리 패키지 | 물리 UPK | 객체 |
|---|---|---|
| `fx_m_mi_00` | `YGI3SB3OBJ3O11GUMP6QMP885.upk` | 재질 2 |
| `fx_m_mi_01` | `YGI3SB3OBJ3O18GUMP6QMP8L5.upk` | 재질 4 |
| `fx_m_mi_03` | `YGI3SB3OBJ3O1MGUMP6QMP8B5.upk` | 재질 1 |
| `fx_m_mi_k_00` | `ZHJ4TC4PCK4PY4J22HIXEYUXOU.upk` | 재질 1 |
| `fx_m_mi_m_00` | `ZHJ4TC4PCK4PC4J22HIXEYUXEU.upk` | 재질 3 |
| `bfx_m_mi_00` | `6YGC3DB3SBJ3S11G16MH6QMH8.upk` | 재질 4 |
| `fx_sm_00` | `XFH2RGA2R00F04YE900X0SMQ.upk` | 메시 2 |

물리 UPK는 전부 `ReleasePC\Packages\` 아래다. `ReleasePC` 직하가 아니다.
논리→물리 대조는 추측이 아니라 `extract_ue3_particle_graph.py`의 UModel 이름 해석 출력이다.

설치 현황을 실제 파일로 확인했다.

| 의존 | 설치 여부 |
|---|---|
| `fx_sm_00.fm_d_bendplane_003` (메시) | **설치됨** — `Effect/KoukuSaydon/Meshes/fx_sm_00/`, `Effect/LV_LUT_MIDNIGHTC_ED/Meshes/fx_sm_00/` 등 4곳 |
| `fx_sm_00.fm_h_tornado_01_2` (메시) | **없음** — 설치본은 `fm_h_tornado_02_1`뿐이고 다른 변형이다 |
| 재질 15종 | **0종 설치** |

## 6. Ground offset — 필드는 실재하고 축은 미확정

G01은 notify-014에 `-200.0` cm가 있다고 했고 축을 미해결로 남겼다. 검증했다.

**필드는 실재한다.** notify-005(Face, 588 byte)와 notify-014(Ground, 594 byte)를 **끝에서 정렬해**
byte diff하면 마지막 200 byte 중 다른 곳이 정확히 8 byte다.

| 끝에서 | Face | Ground | 의미 |
|---|---|---|---|
| 194~198 | `46 58 5F 30 31` = `FX_01` | `00 00 00 00 00` | socket 이름 필드. Ground는 socket 없음 |
| 165~166 | `00 00` | `48 C3` | 이 2 byte가 float를 `0.0` → `-200.0`으로 바꾼다 |
| 4 | `18` | `23` | 말미 counter |

offset 426은 4 byte 정렬이 아니지만 **이 payload 스트림 자체가 byte-packed**라서
비정렬 offset이 정상이다. 정렬을 근거로 무효라고 판단하면 틀린다.

**전수 대조로 용도를 확정했다.** MN_ISMP_00의 `PlayParticleEffect` notify **1,500건** 중
이 위치에 0이 아닌 값을 가진 것은 **6건뿐이고 6건 전부 이름이 `Ground`다.**

| action | notify | 이름 | 값 |
|---|---|---|---|
| 4225601 | notify-014 | Ground | −200.0 |
| 4225611 | notify-012, notify-013 | Ground | −200.0 |
| 4225627 | notify-007 | Ground | −200.0 |
| 4225612 | notify-012 | Ground | −40.0 |
| 4225617 | notify-008 | Ground | −40.0 |

Face·water·finish는 전부 0.0이다. 즉 **지면 effect 전용 offset**이며 항상 음수다.

**축 index는 확정하지 못했다.** 1,500건 어디에도 같은 벡터에서 0이 아닌 성분이 2개 이상인
사례가 없어 X/Y/Z 중 어느 성분인지 byte 배치로 증명할 수 없다. 지면으로 내리는 값이라는
해석은 의미론적 추론이다. 임의 축으로 저장하지 않았다.

별도로 `CEFActionParticleData` 배치 블록은 4종 모두 translation `(0,0,0)`·scale `1.0`이다.
이 offset은 그 블록이 아니라 socket 이름 뒤의 별도 필드에 있다.

## 7. particle 문서를 작성하지 않은 이유

**현재 소비자로는 3종이 영원히 멈추지 않는다.** 코드에서 확인했다.

`Client/Private/Npc.cpp::Update_ActionEffectCues`(521~566행)는 cue마다
`EFFECT_LEVEL_PLACEMENT_SPAWN_DESC`를 만들어 `Spawn_LevelPlacement`를 호출하는데,

- `desc.RootWorld = *m_pTransformCom->Get_WorldMatrixPtr()` — NPC world만 넣는다.
- `cue.strBone`, `cue.bFollowBone`, `cue.iDurationMs`는 **문서에서 읽히기만 하고 이 경로에서 쓰이지 않는다.**
  `grep`으로 확인했고 사용처는 `NpcActionEffectCueDocument.cpp`의 파싱 3줄과 헤더 선언뿐이다.
- 반환 `EFFECT_WORLD_ROOT_HANDLE handle`은 **지역변수로 받고 버린다.** Seek·Stop 경로가 없다.

4절에서 Face·Water·WaterGround의 emitter가 전부 무한 loop임을 확인했으므로,
지금 JSON만 써 넣으면 얼굴·물줄기·지면 물이 **정지하지 않는다.** 이것은 복원이 아니라 회귀다.
그래서 문서 작성을 보류하고 원인을 기록했다.

**다만 필요한 API는 이미 존재한다.** 새 runtime을 만들 필요가 없다.

| 기존 필드/API | 위치 | 이번 용도 |
|---|---|---|
| `bOwnerSustainedSourceLoops` | `Effect_PresentationService.h:125` | 무한 emitter를 소유자 수명에 묶음 |
| `fSourceLoopEndSeconds` | 같은 헤더 :129, 주석 "zero keeps authored timing" | 양수 duration 3종의 종료 시각 |
| `pAnchorOwner` | 같은 헤더 :133, 주석 "Source socket follow anchors read this NPC's body model each frame" | socket follow |
| `EFFECT_SOURCE_BONE_ANCHOR_BUILD_DESC{RawBone, OwnerWorld}` | 같은 헤더 :136 | bone→world 합성 |
| `Seek_WorldRoot` / `Stop_WorldRoot` | 같은 헤더 | handle 보유 후 정리 |

`Effect_Playback.cpp:4721`의 `bBoundedSourceLoop = m_fSourceLoopEndSeconds > 0.f && ...`가
양수면 loop를 끊고, 0이면 `bOwnerSustainedSourceLoops`로 유지하는 것을 확인했다.
**이 의미가 4절의 원본 계약과 정확히 일치한다.** duration 0 → `fSourceLoopEndSeconds = 0` →
저작 타이밍 유지 → WaterFinish가 스스로 1.800초에 끝난다.

이미 `Character.cpp:1276`, `ClassSelectionPresentation.cpp:973`, `ClickMoveEffect.cpp:119`가
같은 필드를 쓰고 있다. 즉 G08에서 `Update_ActionEffectCues`가 이 값을 채우고 handle을
occurrence에 보유하면 된다. 스키마는 socket local transform 필드가 없어 확장이 필요하다(G01 6절).

## 8. 재질 파라미터 적용 경로 — 확인한 위험

`Engine/Private/Model.cpp:1402 Override_SourceCharacterConstants`를 읽었다.

- `LoweredFragment` + `MaterialNameContains`로 **소문자 부분문자열 매칭**이다.
- `Has_SourceCharacterProgram()`인 재질만 대상이다.
- `pMaterial.use_count() > 1`이면 `Clone_ForOverrides()`한다 — 공유 prototype 오염은 막힌다.
- `matched` 개수를 돌려준다.

설치 WModel 두 개에서 실제 재질 slot 이름을 읽었다. **양쪽 모두 3종이다.**

```
mn_ismp_00_mi
mn_ismp_00-1_mi
mn_ismp_00-2_mi
```

**fragment 충돌이 실재한다.** `mn_ismp_00`은 **세 개 전부의 부분문자열**이므로
`Override_SourceCharacterConstants("mn_ismp_00", ...)`는 3 slot을 함께 바꾼다.
`_mi`까지 포함한 전체 이름은 서로의 부분문자열이 아니므로 단독 지정이 가능하다.

`opacity_intensity`가 세 slot 중 어디를 대상으로 하는지는 G01이 미해결로 남긴
`CEFActionNotify_PawnMaterialParam` payload 의미에 달려 있다. 따라서 **적용하지 않았다.**

## 9. 추출 완료 / 후보 / 설치 / 게시 / 빌드 / 소비자 연결

| 단계 | 상태 |
|---|---|
| particle graph 추출 | **예** — 28 system, property error 0, exit 0 |
| 4종 module inventory | **예** — emitter/LOD/module 전개 987줄 |
| lifetime·loop·kill 해석 | **예** — duration 0 의미 확정 |
| 의존 closure 식별 | **예** — 재질 15·메시 2·패키지 7, 논리→물리 확정 |
| 재질 추출 | **아니오** — 15건 전부 exit 1, 도구 경계(10절) |
| 후보 effect 문서 생성 | **아니오** — 의도적 보류(7절) |
| live 설치 | **아니오** — `Data/**`·Resources 변경 0 |
| 게시 | **아니오** |
| 빌드 / 자동검사 | **아니오** — VS 실행 중, 새 단위검사 없음 |
| 제품 소비자 연결 | **아니오** — G08 범위이며 7절 수정 선행 |
| 사용자 수동확인 | **아니오** — 화면에 나타난 변화가 없다 |

## 10. 실행한 명령과 exit code

| 명령 | exit | 결과 |
|---|---|---|
| `extract_ue3_particle_graph.py --help` | 0 | CLI 확인 |
| `extract_ue3_particle_graph.py ... FX_MN_ISMP_00` | 0 | 28 system 추출 |
| `extract_ue3_particle_graph.py ... fx_m_mi_01 bfx_m_mi_00 fx_sm_00` | 0 | 물리 UPK 이름 해석 |
| `extract_ue3_particle_graph.py ... fx_m_mi_00 fx_m_mi_03 fx_m_mi_k_00 fx_m_mi_m_00` | 0 | 나머지 4개 해석 |
| `extract_ue3_particle_module_closure.py --normalized-graph <particle-graph>` | **0** | **`requestCount 0` 무동작** |
| `extract_ue3_material_graph.py` × 15 | **1** × 15 | `Material export is missing` |
| `inventory_targets.py` / `build_inventory.py` / `analyze_lifetime.py` / `collect_deps.py` | 0 | 산출물 생성 |
| `resolve_ground_axis.py` / `find_offset_vector.py` | 0 | 1,500건 전수 대조 |

**exit 0을 성공으로 쓰지 않았다.** `extract_ue3_particle_module_closure.py`는 exit 0이면서
`packageCount 0 / requestCount 0 / closureObjectCount 0`이다. 이 도구는 *normalized skill graph*를
입력으로 받으며 raw particle graph를 조용히 무시한다. 성공이 아니라 무동작으로 기록한다.

## 11. 재질 추출이 막힌 정확한 이유

`extract_ue3_material_graph.py`는 7개 UPK를 모두 정상적으로 열었다(경로를 `Packages\`로
고친 뒤 오류가 `FileNotFoundError` → `Material export is missing`으로 바뀌었다).
즉 **패키지 파싱은 성공했고 이름만 못 찾는다.**

원인은 도구 자체의 상수다.

```
EFFECT_MATERIAL_CLASSES = frozenset({'material', 'decalmaterial'})
```

대상 15종은 `*_mi`(material instance) 패키지의 `*_tr` / `*_ad` 객체, 즉
**MaterialInstanceConstant 계열**이다. 이 도구는 base `Material`과 `DecalMaterial`만 인정하므로
구조적으로 찾을 수 없다.

MIC를 다루는 도구는 `extract_ue3_effect_material_closure.py`이고 docstring이
"records the exact Material Instance parameters and surviving parent Material graph"라고 명시한다.
그런데 이 도구는 `--source-receipt`, `--conversion-receipt`, `--action-cue-recipe`,
`--package-inventory` 네 개를 요구한다. 이 Area에는 그 상위 receipt가 없다. G03/G04가 확인한
`LV_OCN_EVENTIS_MHP.build.receipt.json`의 **`sourceMaterialBuild: null`**이 같은 원인이다.

**이것은 "원본이 없다"가 아니다.** 원본 15종의 논리·물리 패키지와 정확한 objectPath를 모두
확정했고, 막힌 것은 MIC 추출 경로의 상위 입력이다.

## 12. 미해결과 다음 조사 위치

| 미해결 | 확인된 범위 | 다음 위치 |
|---|---|---|
| 재질 15종 추출 | 논리·물리 패키지·objectPath 확정, MIC라서 base Material 추출기 불가 | `extract_ue3_effect_material_closure.py`의 4개 상위 receipt를 이 Area에 만드는 경로. 기존 `build_kouku_all_source_effects.py`가 같은 receipt를 어떻게 만드는지 대조 |
| `fm_h_tornado_01_2` 메시 | 설치본은 `_02_1`뿐(다른 변형) | `fx_sm_00` = `XFH2RGA2R00F04YE900X0SMQ.upk`에서 `_01_2` 추출 |
| Ground offset 축 | 필드 실재·Ground 전용·항상 음수·1,500건 전수 확인 | 다른 몬스터 LOA에서 같은 필드가 2성분 이상인 사례. 또는 `CEFActionNotify_PlayParticleEffect` 클래스 레이아웃 |
| `efparticlemodulevelocityoverlifetime` | WaterGround 전용 2행 존재 확인 | 기존 소비자의 지원 범위. 미지원이면 최소 확장 대상 |
| `PawnMaterialParam` 대상 slot | 재질 slot 3종과 fragment 충돌까지 확정 | G01 9절과 동일. payload 의미 확정 전 적용 금지 |
| `PlayDecalEffect` 패키지 | G01이 ID까지 확정 | G01 9절과 동일 |
| offset 47 플래그 | G01이 "attachment 아님"까지 확정 | 클래스 레이아웃 |

## 13. 범위 밖 관찰 한 줄

같은 graph에 `par_g_ismp_arena_attk01_water_01`(emitter 12)과
`par_g_ismp_arena_attk01_waterground_01`(emitter 17)이 있다. 이름의 `arena`가 워터팡 아레나
변형일 수 있으나 4225601의 대상이 아니므로 조사하지 않았다. G02/G09 담당이 확인할 값이다.

## 최종 재확인

주장한 것을 실제 출력으로 다시 확인했다.

- particle graph 추출기의 summary JSON을 그대로 인용했다. `particleSystemCount 28`,
  `propertyErrorCount 0`은 추측이 아니다.
- 참조 규칙(`packageIndex-1 == exportIndex`)을 graph 전체 9,671건에 적용해 8,833 일치 /
  838 import로 집계했다. 838건이 전부 음수 index임도 같은 스크립트로 확인했다.
- 4절의 `emitterloops`는 **미직렬화와 명시값을 구분해 표시**했다. Face/Water/WaterGround는
  원본에 값이 없어 UE3 기본 0을 적용했고 표에 `*`로 구분했다. WaterFinish 14행만 명시적 1이다.
  이 구분 없이 "무한"이라고 단정하지 않았다.
- duration 0 해석은 4종 상관(양수↔무한 loop, 0↔명시적 1회)에 근거한다. 예외 0건임을 확인했다.
- 1.800초는 표에서 눈으로 고른 값이 아니라 스크립트가 `delay + loops×duration + lifetime최대`로
  계산한 유한 행의 최대값이다.
- Ground offset은 끝에서 정렬한 byte diff로 8 byte 차이를 직접 출력해 확인했고,
  1,500건 전수 대조에서 6건 전부 이름이 `Ground`임을 확인했다. **축은 확정하지 못했다고 적었고
  임의 축으로 저장하지 않았다.**
- 재질 15종 실패는 `material_extraction_receipt.json`에 exit code 15개를 그대로 남겼다.
  성공 0건이다. `EFFECT_MATERIAL_CLASSES` 값은 도구 모듈을 import해 직접 출력했다.
- `extract_ue3_particle_module_closure.py`의 exit 0을 **성공으로 기록하지 않았다.**
  `requestCount 0`을 근거로 무동작이라고 적었다.
- 설치 여부는 `find`로 실제 파일을 확인했다. `fm_d_bendplane_003`은 4곳에 있고
  `fm_h_tornado_01_2`는 0건이며 재질 15종은 0건이다.
- 재질 slot 3종은 설치 WModel 두 개에서 바이트로 읽었다. fragment 충돌은 문자열 포함 관계를
  직접 대조한 결과다.
- 소비자 공백은 `Npc.cpp` 521~566행 원문과 `grep` 결과로 확인했다.
  `strBone`/`bFollowBone`/`iDurationMs`의 사용처가 파싱과 선언뿐임을 출력으로 확인했다.
- 기존 API 필드는 `Effect_PresentationService.h`와 `Effect_Playback.cpp:4721`을 직접 읽었다.
  "zero keeps authored timing" 주석은 원문 인용이다.
- **정본 변경이 없음을 `git status --short -- Data Client Engine Server Shared Tools`와
  `git status --short -- Data/Effects`로 확인했다.** `Data/Effects` 아래 내 추가는 0건이고,
  나열된 dirty 항목은 전부 이전부터 있던 다른 작업의 변경이다. 내 산출물은 gitignore 대상
  `out/.../G06/` 아래에만 있다.
- 빌드를 돌리지 않았다. Client/UI를 실행·조작·캡처하지 않았고 visual 판정을 하지 않았다.
- **G06을 완료로 적지 않았다.** 재질 추출·문서 작성·소비자 연결이 남았음을 9절 표에 명시했고,
  막힌 항목을 임의 값이나 흰 sprite로 메우지 않았다.
