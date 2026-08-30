# Character Select 중앙 보정·모서리 z-fighting 교정 결과

## 2026-08-29 bridge K4 전체 유일 Y 교정 결과

### 실제 반영값과 파일

사용자가 화면에서 해결됐다고 확인한 458의 `-142.733918m`를 anchor로 유지하고, 같은 bridge K4의 444와 474를 추가로 내렸다. 네 조각의 최종 Y는 모두 유일하다.

| source export / stable placement ID | 변경 전 Y(m) | 최종 Y(m) | Imported 대비 |
|---|---:|---:|---:|
| 458 / `17033911184007117021` | -142.733918 | -142.733918 | +0.002 |
| 442 / `13004387245150734382` | -142.735918 | -142.735918 | 0 |
| 444 / `9728074828520549347` | -142.735918 | -142.737918 | -0.002 |
| 474 / `9567686591551344922` | -142.735918 | -142.739918 | -0.004 |

실제 변경 단위는 다음 여섯 파일이다.

- [Authoring mapplacements](C:/Users/user/Desktop/LostArk/Data/Maps/Authoring/LV_LOBBY_CLASSSELECT_SL00/LV_LOBBY_CLASSSELECT_SL00.mapplacements): 444와 474의 Y를 수정한 정본.
- [runtime mapplacements](C:/Users/user/Desktop/LostArk/Client/Bin/DataFiles/Map/LV_LOBBY_CLASSSELECT_SL00.mapplacements): publisher가 Authoring에서 생성한 제품 입력.
- [surface depth contract](C:/Users/user/Desktop/LostArk/Tools/MapPipeline/test_map_surface_depth_contract.py): K4 여섯 조합, signed crossing 별도 진단, 조합 완전성 test.
- [Build README](C:/Users/user/Desktop/LostArk/Tools/Build/README.md): 새 444/474 expected correction과 열 후보 실행 계약.
- [구현 계획서](C:/Users/user/Desktop/LostArk/.md/GB/08-09/2026-08-09_CHARACTER_SELECT_CENTER_DEPTH_SEPARATION_IMPLEMENTATION_PLAN.md)와 이 RESULT: 최신 결정, 증거, 미검증 경계.

Imported, mapasset catalog, WModel, DDS, shader, material, navigation, camera, C++ runtime은 수정하지 않았다. Authoring/runtime은 각각 203,139 byte로 byte-identical이고 SHA-256은 `677e9af7cf01f23ba2fa571b5ffb988ef3d5ce4ff9c98dd2c2395d9d36055ff5`다. 2026-08-28의 다섯 edge correction과 이번 두 개를 합친 edge correction은 일곱 placement이며, 기존 중앙 490/495까지 합치면 Imported와 다른 placement는 아홉 개다.

### 여섯 조합과 인접 geometry 결과

458의 성공을 방향 anchor로 삼되, 네 origin Y가 다르다는 사실만으로 종료하지 않았다. 실제 WModel vertex에 placement S x R x T를 적용하고, K4 네 조각의 여섯 조합에서 양의 XZ 교차 polygon과 보간 Y를 다시 계산했다.

| K4 쌍 | 명목 Y 간격 | 교정 전 near-plane 쌍 | 교정 후 | 교정 후 mesh0 signed crossing |
|---|---:|---:|---:|---:|
| 442 / 444 | 2mm | 307 | 0 | 57 |
| 442 / 458 | 2mm | 0 | 0 | 57 |
| 442 / 474 | 4mm | 1,023 | 0 | 12 |
| 444 / 458 | 4mm | 0 | 0 | 12 |
| 444 / 474 | 2mm | 307 | 0 | 57 |
| 458 / 474 | 6mm | 0 | 0 | 0 |

K4 near-plane 합계는 `1,637 -> 0`이다. 기준은 교차 polygon 전체의 `maximumAbsoluteYGapMeters <= 0.2mm`다. 평탄한 top-face만 분리하면 여섯 쌍의 signed crossing도 모두 0이다. 표의 남은 crossing은 약 10~12도 경사·bevel이고, 이미 사용자 확인을 통과한 442/458에도 57쌍이 있으므로 “raw crossing 전체 0”은 성공 조건이 아니다.

K4 각각과 483/490/495를 WModel의 모든 mesh 조합으로 검사한 결과 12개 조합 모두 near-plane 0이었다. 인접 signed crossing은 대부분 그대로였고 474/483의 bridge mesh1 x MagicFloor mesh0 국소 bevel만 `39 -> 40`으로 한 쌍 증가했다. 넓은 평행면 재발은 아니지만 사용자가 아래 귀퉁이를 볼 때 함께 확인해야 한다. 내부 모델 고저차 때문에 일부 수평 triangle의 실제 최소 간격은 약 0.35346mm까지 내려간다. 기존 0.2mm gate는 통과하고 시작 카메라 1280x720 근사에서는 최소 약 6.12 D24 step이지만, 실제 camera capture가 아니므로 visual PASS 증거로 쓰지 않는다.

### publisher·focused 검증

실제 적용 순서는 `Validate -> publish 전 Check(expected exit 1, 무변경) -> Publish -> Check`였다. Publish 뒤 Authoring/runtime hash가 일치했다.

| 실행 항목 | 결과 |
|---|---|
| `Publish-MapAuthoring.ps1 -Mode Validate` | PASS, 803 placements / 2 files |
| publish 전 `-Mode Check` | 예상대로 exit 1, runtime mutation 없음 |
| `-Mode Publish` | PASS, runtime 생성 |
| publish 후 `-Mode Check` | PASS |
| `python -B -m unittest Tools.MapPipeline.test_map_surface_depth_contract` | 23 tests PASS, 0.041초 |
| Map Effect + surface 결합 unittest | 37 tests PASS, 49.905초 |
| 실제 Resources surface CLI | 열 후보 near-plane 0, K4 여섯 조합 포함, `cameraDepthStatus=not_requested` |
| Authoring/runtime byte identity | PASS, 위 SHA-256 일치 |

실제 Resources CLI에는 기존 405/427/436/458/471과 새 444/474의 Imported 대비 correction을 모두 명시했다. 444는 `-0.002`, 474는 `-0.004`, 458은 `+0.002`다. 진단기는 near-plane과 signed crossing을 서로 다른 field로 출력하며, unit test가 K4 후보 집합이 정확히 여섯 조합인지 고정한다.

### Debug/Release build와 병목 측정

모든 수치는 같은 PC에서 x64, `/m`, `/nodeReuse:false`, `BuildProjectReferences=false`로 한 번씩 측정한 wall time이다. Server 수치는 pre-build publisher를 포함한 `Server.vcxproj /t:Rebuild`다. FullDiagnostic compile 수치는 12개 프로젝트를 완전 Clean한 뒤 Engine -> UpdateLib -> Shared -> Core 하네스 2개 -> Server -> Client -> 진단 하네스 6개 순서로 빌드한 시간이며 실행형 회귀는 제외했다.

| 측정 | Debug | Release | 상태 |
|---|---:|---:|---|
| Server 단독 full rebuild | 45.819초 | 45.672초 | 둘 다 PASS |
| FullDiagnostic 12-project Clean | 2.589초 | 2.931초 | 둘 다 PASS, compile 시간에서 제외 |
| FullDiagnostic 12-project clean compile | 294.846초 | 666.883초 | 둘 다 PASS |

Release clean compile은 Debug의 약 2.26배다. 로그상 주 병목은 Server가 아니라 Client Release 셰이더 full compile과 LTCG다. Release Client는 50,366 functions를, Server는 21,044 functions를 usable IPDB/IOBJ 없이 전체 코드 생성했다. ActionPresentationTimeline 등 Release 하네스도 각각 LTCG를 수행했다. Server만 보면 Debug/Release가 모두 약 45.7~45.8초라 구성 차이가 거의 없다. 과거 동일 조건의 기준 시간이 없으므로 이번 한 번의 수치만으로 최근 최적화의 개선률을 주장하지 않는다.

정본 `Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile FullDiagnostic -AllowLocalEffectResources`는 완전 compile과 회귀를 진행한 뒤 645.276초에 **FAIL**했다. 그 직전 가장 긴 `test_valtan_pattern_master_v2.py` 73 tests는 365.121초에 PASS했지만, 이어진 `test_animation_tool_valtan_pattern_master.py` 12 tests 중 두 문자열 source-contract가 실패했다.

- `test_animation_tool_loads_the_shared_typed_projection`: 기존 `"Valtan Pattern Master (Authoritative)"` 문자열을 기대하지만 현재 코드에 없음.
- `test_complete_timeline_uses_server_wall_and_source_clocks`: 기존 `"Play Arena Presentation Locally"` 문자열을 기대하지만 현재 코드에 없음.

이는 이번 mapplacement·surface 변경과 무관한 현재 dirty tree의 Animation Tool UI/test 계약 drift다. 컴파일 성공이나 K4 focused PASS로 광역 회귀 PASS를 대신하지 않았고, 사용자 요청 범위를 벗어나므로 관련 C++/test는 수정하지 않았다. Release는 12-project clean compile까지 성공했으며, 같은 configuration-independent Python source gate를 다시 365초 실행해 PASS처럼 기록하지 않았다. 빌드에는 현재 로컬 Effect resource 3개를 명시적으로 허용했지만 source/resource validation 자체는 통과했다. C4819, C4828, X3577, X4000, LNK4099 등 기존 warning은 있었고 컴파일 오류는 없었다.

### 남은 사용자 판정

자동 검증으로 확정한 범위는 정본/runtime 동기화, K4 여섯 조합의 넓은 near-plane 제거, 인접 배치에서의 near-plane 비재발과 Debug/Release 컴파일이다. Client/UI는 실행하지 않았으므로 아래 왼쪽·아래 오른쪽의 실제 visual PASS는 아직 사용자 판정 대기다.

사용자는 팀 Server가 실행 중인 상태에서 Client를 직접 시작해 Lobby -> Character Select로 진입하고, 처음 카메라에서 아래 두 귀퉁이와 474/483 인접 bevel을 확인한다. 한 표면만 남긴 상태에서도 무늬가 반짝이면 geometry가 아니라 기존 mip 부재 후보를 다음 범위로 다룬다. 현재 LAN endpoint `10.207.18.103:7777`은 이 작업 시작 시 `not-listening`이었고 로컬 role은 `client`였으므로, Server가 꺼진 상태에서는 시각 확인을 시작할 수 없다.

---

## 2026-08-28 모서리 z-fighting 배치 교정 결과

### 실제 반영 상태

사용자가 승인한 여섯 겹침 쌍을 다섯 placement의 Y만 조정해 Authoring과 제품 runtime에 반영했다. Imported 보존본, asset, model/texture, material/shader, depth state, camera near/far와 navigation은 변경하지 않았다.

| source export / stable placement ID | 변경 전 Y(m) | 변경 후 Y(m) | 방향 |
|---|---:|---:|---|
| 405 / `18313157716296743356` | -142.826436 | -142.828436 | 2mm 아래 |
| 427 / `11064700097567933927` | -142.775361 | -142.777361 | 2mm 아래 |
| 436 / `11123722243846476314` | -142.775361 | -142.777361 | 2mm 아래 |
| 458 / `17033911184007117021` | -142.735918 | -142.733918 | 2mm 위 |
| 471 / `12265842867223881631` | -142.765332 | -142.767332 | 2mm 아래 |

458은 442/458과 458/474를 한 번에 분리한다. 처음 검토한 2mm 하향안은 기존 중앙 495와 여섯 개의 새 국소 근접 교차를 만들었으므로, 458만 위로 보내 그 교차를 0으로 유지했다. 이 방향은 관련 bevel 삼각형의 기존 위아래 순서 변경이 47쌍에서 48쌍으로 한 쌍 늘어나는 절충이다. 평탄한 바닥 전체가 다시 가까워진 쌍은 아니다.

[Authoring 배치](C:/Users/user/Desktop/LostArk/Data/Maps/Authoring/LV_LOBBY_CLASSSELECT_SL00/LV_LOBBY_CLASSSELECT_SL00.mapplacements)를 정본으로 수정하고 [visual publisher](C:/Users/user/Desktop/LostArk/Tools/MapPipeline/Publish-MapAuthoring.ps1)의 `Validate → Publish → Check`로 [runtime 배치](C:/Users/user/Desktop/LostArk/Client/Bin/DataFiles/Map/LV_LOBBY_CLASSSELECT_SL00.mapplacements)를 생성했다. 최종 두 파일은 byte-identical이고 SHA-256은 `375c86c6e12cad769b8e1093acbc499d34e25b7a57816eef7dc2866e6e253700`이다.

### 교정 후 geometry 결과

`nearPlaneTrianglePairCount`는 양의 XZ 교차 면적을 가지면서 교차 polygon 전체의 최대 절대 Y 차이가 0.2mm 이하인 삼각형 쌍의 수다.

| source export 쌍 | 교정 전 근접 평면 쌍 | 교정 후 | 교정 후 교차 영역의 Y 간격 범위(mm) |
|---|---:|---:|---:|
| 402 / 405 | 14 | 0 | +2.000040 |
| 458 / 474 | 307 | 0 | +1.808462 ~ +2.191662 |
| 442 / 458 | 307 | 0 | -2.191539 ~ -1.808338 |
| 425 / 427 | 71 | 0 | +1.999682 ~ +2.048825 |
| 435 / 436 | 71 | 0 | +1.999682 ~ +2.048825 |
| 469 / 471 | 22 | 0 | +1.999873 ~ +2.000152 |

관련 4개 asset·59개 placement의 AABB 후보 121쌍으로 제한해 2/3/4mm와 458 방향을 비교했다. 최종안에서 새 **whole-polygon near-coplanar** 쌍은 0이다. 427/483·436/483의 약 32.46° 경사면과 bridge bevel에는 국소 교차·순서 변경이 남으므로 “모든 교차가 0”이라고 기록하지 않는다. 같은 Y와 가까운 origin만으로 추린 다른 후보 전체에 ID 순서 offset을 일괄 적용하지 않았다.

기존 중앙 410/411/490/495의 placement record는 변경하지 않았다. 특히 458/495의 확인한 두 bevel 구간은 교정 전 최소 간격 약 1.682mm/0.475mm에서 교정 후 3.682mm/2.475mm로 늘었다. 기존 490/495의 Imported 대비 -2mm 보정도 그대로다.

### publisher와 자동 검증

[visual publisher](C:/Users/user/Desktop/LostArk/Tools/MapPipeline/Publish-MapAuthoring.ps1)에 `Validate / Check / Publish` mode를 추가했다. 세 mode는 같은 expected bytes를 사용한다. Validate와 Check는 runtime 파일·디렉터리를 쓰지 않고, Publish만 기존 file-set transaction을 실행한다. single/shard 공통으로 placement/source ID, float transform, catalog asset 참조와 Resources-relative model path를 검증한다.

| 검증 | 실제 결과 |
|---|---|
| publisher temp fixture | 기존 Map Effect 6개 포함 14 tests PASS, 61.130초 |
| publisher 실패 계약 | single/shard 입력 거부, unsafe path, Validate/Check 무변경, 첫/중간 promote rollback PASS |
| 실제 Character Select publish 전 Check | 예상 불일치 exit 1, 기존 runtime bytes 유지 |
| 실제 `Validate → Publish → Check` | 각 exit 0, 803 placements / 2 files |
| surface 합성 계약 | 21 tests PASS |
| 교정 후 실제 resource geometry CLI | exit 0, 여섯 쌍 근접 평면 0 |
| placement 보존 | Imported 803 유지, Authoring/runtime 803 및 byte identity, 정확히 위 다섯 Y만 추가 변경 |
| 제품 origin scope | 779 placements / 54 assets 유지 |
| Character Select Client/UI 및 실제 카메라 JSON | 미실행/미확보 |
| 정본 전체 Debug/Release build/regression | 이번 배치 교정 단계에서는 미실행 |

교정 후 geometry 출력은 [geometry-after.json](C:/Users/user/Desktop/LostArk/.codex_tmp/character_select_height_adjust_root_20260828/geometry-after.json), 배치 보존 receipt는 [published-preservation.json](C:/Users/user/Desktop/LostArk/.codex_tmp/character_select_height_adjust_root_20260828/published-preservation.json), 방향 절충 수치는 [bridge_direction_comparison.json](C:/Users/user/Desktop/LostArk/.codex_tmp/surface_adjustment_20260828/bridge_direction_comparison.json)에 보존했다. 모두 로컬 수치 증거이며 runtime 입력은 아니다.

### 남은 사용자 판정과 texture 경계

이번 자동 검증은 지정한 geometry의 deterministic depth 순서를 만든 사실까지 확인한다. 실제 카메라·GPU rasterization·가림·최종 화면은 실행하지 않았으므로 **모서리 visual PASS는 사용자 판정 대기**다. 사용자는 Visual Studio의 **Server + Client** profile을 `Ctrl+F5`로 시작하고, Server 승인 뒤 Character Select에서 기존 모서리와 중앙부를 follow/F6 free 이동·회전으로 확인한다.

한쪽 표면만 보이는 상태에서도 무늬가 반짝이면 이번 z-fighting과 별개인 mip 부재 후보가 남는다. Character Select DDS의 mip chain은 이번 변경에서 생성하지 않았다. 화면 확인 뒤 단일 면에서도 남는 경우에만 계획서 G03을 진행한다.

---

## 2026-08-28 모서리 geometry 진단 체크포인트 — 교정 전 기록

### 실제 구현 상태

[구현 계획서](C:/Users/user/Desktop/LostArk/.md/GB/08-09/2026-08-09_CHARACTER_SELECT_CENTER_DEPTH_SEPARATION_IMPLEMENTATION_PLAN.md)의 G02 중 수치 진단을 추가했다. 이 단계에서 추가한 코드는 [test_map_surface_depth_contract.py](C:/Users/user/Desktop/LostArk/Tools/MapPipeline/test_map_surface_depth_contract.py) 한 파일이다. 기존 `parse_legacy_wmodel` decoder를 재사용하며 원본·Authoring·runtime 배치, model/texture, shader, near/far를 수정하지 않았다.

진단은 지정된 여섯 placement 쌍에만 1m spatial bin과 XZ AABB 필터를 적용하고, 높이 범위 `[-143.3,-142.4]m`의 실제 삼각형 교차 polygon과 높이 차이를 계산한다. 전체 map의 모든 삼각형 쌍을 비교하거나 화면에 보이는 면을 판정하는 도구가 아니다. missing resource는 asset ID를 포함한 오류로 반환하며 성공으로 숨기지 않는다.

### 이번에 실행한 자동 검증

```powershell
Set-Location 'C:/Users/user/Desktop/LostArk'
python -B -m unittest Tools.MapPipeline.test_map_surface_depth_contract
python -B 'C:/Users/user/Desktop/LostArk/Tools/MapPipeline/test_map_surface_depth_contract.py' --resource-root 'C:/Users/user/Desktop/LostArk/Client/Bin/Resources' --area-id LV_LOBBY_CLASSSELECT_SL00
```

| 검증 | 실제 결과 |
|---|---|
| 합성 geometry·depth·입력 실패·배치 보존 unit tests | 21 tests PASS |
| 실제 resource geometry CLI | exit 0 |
| Imported / Authoring / runtime placement | 각 803개, stable placement/source ID 집합 유지 |
| Authoring / runtime bytes | 일치 |
| Imported 대비 허용 변경 | 기존 490/495의 Y -0.002m만 존재, 다른 placement 속성 유지 |
| catalog / 제품 origin scope | 55 asset / 779 placement·54 asset 유지 |
| 실제 카메라 JSON | 미확보, `cameraDepthStatus=not_requested` |
| 이번 Character Select 단계의 publisher·빌드·Client/UI 실행 | 미실행 |

출력 원본은 [surface_depth_geometry.json](C:/Users/user/Desktop/LostArk/.codex_tmp/bern_frustum_sweep_20260828/surface_depth_geometry.json)이다. 현재 로컬 실행 증거이며 Git으로 배포하는 runtime 입력은 아니다. JSON은 `status=diagnostic_only`이고 여섯 후보 모두 camera-depth 항목이 없다. uint64 placement ID는 정밀도를 잃지 않도록 문자열로 기록한다.

### 실제 geometry 결과와 중앙 보정 보존

아래 source export는 모두 `LV_LOBBY_CLASSSELECT_SL00:export:<번호>`다. 각 쌍의 전체 stable placement ID·asset ID·mesh/triangle 번호는 [계획서의 후보 표](C:/Users/user/Desktop/LostArk/.md/GB/08-09/2026-08-09_CHARACTER_SELECT_CENTER_DEPTH_SEPARATION_IMPLEMENTATION_PLAN.md)에 유지한다.

| source export 쌍 | AABB 후보 비교 / 교차 삼각형 쌍 | 근접 평면 쌍 | 기록된 검사 XZ(m) | 검사점의 절대 Y 차이(m) |
|---|---|---|---|---|
| 402 / 405 | 592 / 316 | 14 | -762.85, 197.60 | `4.0324636e-8` |
| 458 / 474 | 1539 / 643 | 307 | -773.95, 197.30 | `2.6297670e-8` |
| 442 / 458 | 1539 / 643 | 307 | -772.00, 195.20 | `5.8495464e-5` |
| 425 / 427 | 2105 / 816 | 71 | -770.20, 187.70 | `4.8609133e-5` |
| 435 / 436 | 2105 / 816 | 71 | -762.10, 199.25 | `4.8771437e-5` |
| 469 / 471 | 74 / 44 | 22 | -778.60, 198.20 | `1.4819787e-7` |

근접 평면 쌍은 양의 XZ 교차 면적이 있고 **교차 polygon 전체의 최대 절대 Y 차이가 0.0002m 이하**인 삼각형 쌍이다. 개수는 placement 수나 실제 화면 결함 수가 아니다. 검사점 Y 차이는 world-space geometry 수치이며 동일 화면 ray의 post-projection depth 차이를 대신하지 않는다.

| 기존 중앙 source / stable placement ID | Imported Y | Authoring / runtime Y | Imported 대비 |
|---|---|---|---|
| export 490 / `11968900681581939590` | `-142.711572` | `-142.713572` | `-0.002m` 유지 |
| export 495 / `10547857777741800178` | `-142.71916` | `-142.72116` | `-0.002m` 유지 |

중앙부의 이번 검증 범위는 기존 placement 보정과 나머지 속성 보존이다. 새 geometry scan은 위 여섯 모서리 후보 쌍에 한정하므로 중앙부의 새 camera-depth 검증으로 기록하지 않는다.

### 미검증 경계와 다음 순서

실제 카메라 JSON과 사용자의 수동 화면 관찰을 확보하지 않았다. **이번 결과는 camera-depth PASS, visual PASS 또는 모서리 수정 완료가 아니다.** 합성 camera/depth test가 통과했다는 사실도 제품 카메라 검증을 대신하지 않는다.

베른 core 수학 수정을 먼저 검증하고, Character Select에서 현상이 남으면 사용자가 발생 위치를 기존 MapTool 또는 제품 화면에서 대응시킨다. 필요한 경우에만 공통 capture JSON을 확보하는 별도 단계를 진행해 `--camera-log`로 검사한다. 새 ImGui 진단 패널은 제품에 반영하지 않으며 새 F1 메뉴가 있다고 안내하지 않는다. 배치 교정·mip 생성·shader나 near/far 변경은 이 체크포인트에서 수행하지 않았다.

---

## 2026-08-09 중앙 보정 완료 이력 — 당시 기록 보존

아래 내용은 08-09 당시 변경과 검증 기록이다. 옛 `Tools/ProjectAudit/Test-CharacterSelectCenterDepthSeparation.ps1`과 ProjectAudit 명령은 현재 존재하지 않으며, 아래 build·startup probe·audit PASS를 08-28에 다시 실행한 결과로 해석하지 않는다. 현재 실행 가능한 수치 진단과 미검증 경계는 위 체크포인트를 따른다.

### 완료 상태

`LV_LOBBY_CLASSSELECT_SL00` 중앙의 교차 bridge 조각 두 쌍에 안정적인 2mm 깊이 간격을 적용했다. placement 개수, stable placement ID, asset ID와 Imported 원본 증거는 유지했다.

이미지·스크린샷 확인은 수행하지 않았다. 원인과 결과는 원본 package actor/material, glTF triangle, placement transform, depth 수치와 audit로만 판정했다.

### 확인한 원인

- 중앙 mesh는 `MAP_AC527A4AF171_BG_ELG_ARYANORB_BRIDGE01E_SM`이다.
- export 410/490과 411/495는 같은 bridge mesh를 각각 회전해 교차 배치한다.
- 원본 UE3 actor의 material override는 서로 다른 layer 의도를 보존하지만 모두 opaque parent다.
- glTF triangle과 runtime scale `0.01`을 적용한 교차면 계산 결과:
  - 410/490 최대 높이 차이: 약 `7.913e-10m`
  - 411/495 최대 높이 차이: 약 `4.187e-5m`
- 두 값 모두 중앙 교차부에서 안정적인 depth 우선순위를 만들지 못하므로 카메라 이동에 따라 z-fighting이 발생할 수 있다.

교차 조각은 각자 넓은 고유 영역을 가지므로 중복 actor 삭제는 올바른 수정이 아니다. material을 임의의 alpha 경로로 바꾸는 것도 원본 opaque 계약을 훼손한다.

### 실제 변경

- export 490 Y: `-142.711572 -> -142.713572`
- export 495 Y: `-142.71916 -> -142.72116`
- 다음 authoring/runtime 문서를 같은 내용으로 갱신했다.
  - `Data/Maps/Authoring/LV_LOBBY_CLASSSELECT_SL00/LV_LOBBY_CLASSSELECT_SL00.mapplacements`
  - `Client/Bin/DataFiles/Map/LV_LOBBY_CLASSSELECT_SL00.mapplacements`
- `Data/Maps/Imported/...`는 원본 증거로 유지했다.
- `Tools/ProjectAudit/Test-CharacterSelectCenterDepthSeparation.ps1`을 추가했다.

2mm는 원본의 서로 다른 layer 간격 약 7.59mm보다 작고, 해당 교차면의 기존 오차보다 충분히 크다. 다른 transform이나 placement는 변경하지 않았다.

### 자동 검증

| 검증 | 결과 |
|---|---|
| `Test-CharacterSelectCenterDepthSeparation.ps1` | PASS |
| Imported 원본 좌표 유지 | PASS |
| Authoring/runtime byte identity | PASS |
| placement count 803 및 대상 ID 유지 | PASS |
| ProjectAudit `maps.character-select-area-contract` | PASS, assets 55 / placements 803 / manifest 55 |
| Release Client build | PASS |
| Release Client 10초 startup probe | PASS |

전체 ProjectAudit의 남은 실패 2개는 `maps.product-editor-visual-scope`와 다른 세션의 rendering profile FXAA 범위이며 Character Select area 계약과 depth separation audit는 통과했다.

### 수동 검증과 남은 경계

- 이미지 확인 금지 규칙에 따라 중앙 바닥의 육안 비교는 미실행이다.
- 이번 변경은 현재 runtime이 실제로 소비하는 placement 좌표의 deterministic depth 순서를 고정한다.
- 원본 actor별 material override를 converter가 완전히 보존하는 작업은 별도 asset-import 수직 슬라이스이며 이번 z-fighting 수정 범위에는 포함하지 않았다.
