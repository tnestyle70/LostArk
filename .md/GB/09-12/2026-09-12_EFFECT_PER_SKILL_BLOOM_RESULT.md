# 모든 Full Restore의 개별 Bloom Intensity 결과

## G00. 데이터 반영

V1 Authored 919개를 전수 조사하여 Full Restore 124개와 동일 전체 element 구성을 가진 festival 파생본 1개, 총 125개에 각각 root `bloomIntensity` 행을 추가했다. 최종 사용자 지정 기본값은 **1.3**이다. 처음 추가했던 1.0을 125개 모두 1.3으로 교정했고 그 밖의 JSON 필드는 보존했다. 단계 1·2·3·4와 clip별·통합 버전을 같은 skill ID로 합치지 않았다.

| 분류 | 독립 문서 수 |
|---|---:|
| Artist | 18 |
| DimensionMaster | 14 |
| LanceMaster | 47 |
| Warlord | 27 |
| Kouku | 19 |
| 합계 | 125 |

Kouku 19개에는 명시 전체형 18개와 festival 파생본 1개가 포함된다. 미등록 DimensionMaster Q도 Authored 탐색에서 선택 가능하므로 catalog/tree 유무와 관계없이 포함했다. Artist LMB 1/2/3/4, Artist Z 1/2, 현재 존재하는 R 1/4, LanceMaster 34630 clip 1/2/3/4와 통합본, Warlord 17170 clip 1/2/3과 통합본은 모두 독립 행이다. 없는 단계는 새로 생성하지 않았다.

전체 ID·경로·분류 근거는 `out/KoukuMarioAssets20260912/bloom_inventory/`의 `inventory.json`, `full_restore_documents.json`, `REPORT.md`에 있다. 실제 변경 증거 `migration_result.json`은 125개 원본/결과 SHA와 기타 byte 보존, 최종 1.3을 기록한다. 초기 조사에서 기존 root 필드는 0개였다.

## G01. 저작·저장 계약

V1 `EFFECT_DOCUMENT_DESC::fBloomIntensity`와 JSON의 optional root 필드는 생략 기본값1.3, 유한한0~16 계약이다. invalid 입력은 기존 문서를 보존한 채 거부하고 Save는 필드를 직렬화한다. Effect Detail의 `Skill Bloom Intensity`는 현재 문서 전체 값이다. 같은 asset ID의 실행 preview scalar를 즉시 변경하며 clock/seek/리소스 재준비를 하지 않는다. 다른 문서는 자기 값을 유지한다.

사용자 경로는 `F1 → Effect Tool V1 → 원하는 Full Restore 선택 → Effect Detail → Skill Bloom Intensity → Save Changes`다. 단계별 문서는 각각 선택하여 저장한다. Rendering Benchmark의 전역 데이터는 개별 문서 편집으로 바뀌지 않는다. 개별1.3은 해당 스킬의 intensity이며 전역 intensity가 다시 곱해지지 않는 계약으로 연결한다. 구 Client로 Save하면 새 필드가 빠질 수 있으므로 새 바이너리를 적용한 뒤 저작을 계속해야 한다.

## G02. 렌더링 계약과 검증 경계

원본 HDR RT0와 distortion RT1을 유지하고 RT2에 문서별 가중치가 적용된 bloom 기여를 모은다. 기존 blur/final 합성을 재사용한다. threshold는 각 draw의 색에서 적용하며, 합성된 HDR를 다시 threshold하여 문서별 가중치를 잃지 않게 한다. 전역 enable/threshold/soft knee/scatter는 기존 품질 값을 사용한다. 비Effect는 전역 intensity, V1은 문서 intensity를 합성 전에 적용하여 두 값을 중복 곱하지 않는다.

배율1도 반투명·additive가 겹치는 장면의 기존 bloom과 완전히 같다는 뜻은 아니다. 예를 들어 threshold1/knee0에서0.75 두 출력이 겹치면 예전 합성 후 추출은0.5지만 개별 추출 합은0이다. RGB10/alpha0.01은 예전 합성 후0, 개별 추출은0.09가 된다. 원본 HDR 자체는 유지되며 사용자 기본 배율은1.3이다.

화면 SceneColor를 사용하는 native는 그 색을 자체 발광으로 다시 추출하면 안 된다. 원본 HDR 평가를 유지하고 `F(bloom)-F(black)`으로 기존 bloom을 운반하며 `F(black)`의 자체 색에만 문서 intensity를 적용한다. live SceneColor/SceneBloom snapshot과 ALTV178의 시작 시점 캡처를 쌍으로 유지한다. SceneColor를 읽지 않는 일반 재질은 추가 평가를 생략한다. motion1619의 RGB saturate는 선형 분해가 아니므로 bloom에 동일 연산자를 적용하는 전달 규칙으로 기록한다. 기존 native1228의 alpha 0으로 인해 현재 screen-post 합성이 보이지 않는 경계는 이 변경에서 원본 색 동작과 함께 유지했다.

shared model cue shader는 draw 성공/실패 뒤 전역 sentinel로 복귀한다. deferred 소환 모델은 source-character marker 5에서 기존에 사용하지 않던 Normal.a에 `(intensity+1)/17`을 저장한다. 실제 Normal RT가 UNORM16이라 양수 표식을 사용하며, 0은 정확히 억제되고 1.3은 약 1.29987로 양자화된다. 기존 typed lighting/SSAO/decal의 RGB·AO·roughness·specular 소비를 보존한다. OneLayer의 custom RT2 write와 Debug fixed-function 검증도 새 출력에 맞췄다.

검토에서 125문서 중 SceneColor를 읽는 world native 17프로필/66요소/12문서와 별도 frozen 캡처를 확인했다. `bloom_inventory/render_review.md`와 `scene_native_usage.json`에 정확한 ID와 수정 근거를 남겼다. light module이 비추는 월드 표면은 기존 장면 조명이며 별도 문서별 조명 권한으로 바꾸지 않았다. display-space overlay는 기존 계약대로 bloom 이후 출력한다.

## G03. 현재 수행한 검증과 제품 적용 상태

- 125문서의 root 값과 JSON parse, 다른 필드의 보존, 원본/결과 SHA를 확인했다. 누락·변경·불일치는 0이며 `bloom_inventory/final_data_check.json`에 기록했다.
- 기본값 1.3을 적용한 V1 codec/instance 검사 69조건이 통과했다. 실제 125문서 전부 Load/Validate_Drawable/직렬화/재Parse와 1.3 보존을 통과했다. 실제 v15 문서 2개는 0·0.25·2의 저장·재로드와 document-owned runtime projection 생성 6조건도 통과했다. `bloom_compile/test_result.json`, `inventory_result.json`이 근거다.
- 최종 generic mesh CSO의 창 없는 WARP 12조건이 통과했다. additive/alpha에서 문서 A/B의 0·0.25·1·1.3, 불투명 가림, 전역 2.61/0과의 독립 강도, scene fallback, threshold 조합을 확인했다. 원본 HDR는 그대로이며 RT2 기여만 변한다.
- 최종 native post CSO WARP 5조건이 통과했다. 다른 문서의 bloom 3은 post 문서값 0/1.3에서도 유지되고, 자체 방출은 intensity 1.3으로 3.9만 추가했다. 억제된 HDR 색에서 다른 문서 bloom이 재생성되지 않았다.
- 최종 Deferred CSO WARP 4조건이 통과했다. UNORM 표식의 문서값 0은 전역 2.61에서도 bloom 0, 문서값 1.3은 전역 2.61/0 모두 약 3.89961이었다. 표식 없는 scene fallback은 7.83, 원본 HDR는 모두 4였다. 총 21 GPU 수치 검사는 `out/EffectSkillBloom/warp-bloom-result.log`, `native-warp-result.log`, `gbuffer-warp-result.log`에 기록했다. 전체 native 조건이나 사용자 화면 판정을 대신하지 않는다.
- Client codec/render/object 의존 20개와 Tool/Sequencer/Workspace의 최소 컴파일·링크가 통과했다. 마지막 Object/DocumentRenderer 변경 뒤 재컴파일·재링크와 69검사도 통과했다. 기존 Engine DLL을 쓰는 CPU 검사이므로 새 Engine 렌더링의 실행 증거로 대체하지 않는다.
- Engine Renderer/Shader/Render_OutputContract/Presentation_Manager의 최종 최소 컴파일 4개가 통과했다. HLSL `fx_5_0` 115개 전부 최종 컴파일에 통과했고, UNORM 표식 교정 뒤 Deferred/AnimMeshBinary도 다시 컴파일했다. `out/EffectSkillBloom/shader-compilation.json`과 `bloom-validation-summary.json`에 기록했다. CSO·OBJ는 out에만 생성했으며 제품/SDK 배포 완료로 기록하지 않는다.
- `generate_artist_native_runtime_shader.py`는 SceneColor 샘플을 공용 hook으로 생성하고, `install_kouku_gate1_native_shaders.py`는 구형 추출 산출물을 같은 hook으로 정규화한다. 두 파일의 Python AST와 Sample/Bias/Level 변환·멱등성 검사는 통과했다. 실제 재추출·설치 실행은 하지 않았으며 기존 소유자의 생성기 변경도 보존했다.
- Rendering의 Base Bloom Intensity와 Scene Profile의 Bloom Intensity Multiplier에는 Full Restore의 개별값을 Effect Detail에서 조절한다는 tooltip을 추가했다. 두 CPP의 추가 최소 컴파일이 통과했고 설정 데이터는 변경하지 않았다.
- 현재 workingtree 전체 `git diff --check`는 오류 0이다. 기존 파일의 LF/CRLF 안내는 오류와 구분하며 `bloom_compile/final_diff_check.log`에 기록했다.

`powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Profile Product -Configuration Debug`를 실행했지만 `ProductOutputGuard.psm1:85`에서 실행 중인 Client PID 15004와 Server PID 80532를 확인해 exit 1로 빌드 시작 전에 차단했다. 로그는 `bloom_compile/product_build.log`다. 정본 Product 컴파일·제품 CSO 배포는 미완료이며 out 검증 산출물과 구분한다.

사용자에게 저장 후 종료를 요청했으며 에이전트가 프로세스를 종료·실행하거나 Client/UI를 조작하지 않았다. 사용자 visual 판정은 미실행이다. 대규모 기존 dirty checkout과 사용자 저작 변경을 보존하며 자동 stage/commit/push하지 않았다. 현재 PC는 LAN 설정에서 server-host였으므로 제품 빌드 후 사용자가 VS의 `Server + Client` profile을 Ctrl+F5로 시작한다.

마리오 자산 설치와 원본 누락 경계는 [아이언메이든·칼날 결과](2026-09-12_KOUKU_MARIO_IRON_MAIDEN_BLADES_RESULT.md)에 별도로 기록했다.

## G04. 쿠크 1관문 연출 축포의 Play All 미표시 조사

사용자가 보고한 대상은 `effect.kouku.gate1.intro.festival.full.restore`의 Play All 미표시다. 리소스 파일 소실과 구분한다. 해당 v13 문서는 80개 요소이며 bloom 추가 전 보관본과 비교하여 `bloomIntensity: 1.3` 이외의 위치·시간·재질 입력은 동일했다. 물리 리소스 참조 13개도 존재한다. 동일 표시명의 authored 파생본은 별개 문서이며 사용자가 그것을 선택했다고 추정하지 않는다.

문제 보고 당시 Client는 20:51:43에 시작된 기존 실행 파일이었다. EXE/DLL 및 축포 cohort 2304의 제품 CSO는 이번 bloom 변경 이전 산출물이었으며, Shader는 실행 파일 옆 CSO를 읽는다. 따라서 새 bloom 구현이 당시 실행에 적용되지 않았다는 사실을 확인했다. 이것만으로 구 버전이 미표시 원인이라거나 최신 빌드로 해결된다고 판정하지 않는다.

실제 문서의 구 ABI 리소스 stage 검사가 통과했고, 현재 codec/playback의 24.2초 CPU 재생은 첫 입자 0.0167초, 최대 1,356개, drawable 70개와 hidden provider 10개로 통과했다. 임의 플레이어 root 위치·회전을 사용한 좌표는 유한하며 맵의 고정 연출 좌표를 요구하지 않았다. native 2365의 제어된 창 없는 WARP 비교 3개에서는 구·신 shader 모두 1,352 pixel과 동일 HDR `[63,24,6,1]`을 출력했고 신 shader의 intensity 0은 bloom만 제거했다. 이는 전체 실제 화면이나 모든 재질의 표시 성공 증거는 아니다.

Play All은 현재 scene player의 위치와 yaw를 매번 새로 가져오고 clock을 0으로 초기화한다. Composition의 MAP/WORLD 배치 앵커를 소비하지 않는다. 패턴에 Append한 Effect box는 별도 occurrence의 BOSS/WORLD/MAP 앵커, 위치, 회전을 저장하며 BOSS/WORLD는 기준 상대 offset, MAP은 고정 맵 좌표로 런타임에 연결된다. Box Detail의 Apply/Save가 이 배치 값을 유지한다.

현재 stage·CPU spawn·shader 수치 검사와 소스 호출 경로에서는 사용자 미표시를 재현하지 못했으므로 추측으로 shader나 앵커를 변경하지 않았다. 실제 Effect render 실패는 당시 Debug output과 Tool 상태에만 남아 종료 후 파일 로그로 원인을 복원하지 못했다. 세부 증거는 `out/KoukuMarioAssets20260912/validation/festival_play_all_investigation.md`, 같은 폴더의 `festival_old_resource_stage_result.json`, `festival_cpu/result.json`, `out/EffectSkillBloom/festival-probe-result.log`다. 사용자 화면 확인은 미완료다.

사용자가 Client와 Server를 종료한 뒤 같은 정본 Product Debug 명령을 다시 실행했다. 23:03 이후 사용자의 마무리 요청 시점에는 Engine·Shared·Server 빌드가 통과했고 Client 컴파일이 진행 중이었다. `out/KoukuMarioAssets20260912/bloom_compile/product_build_current.log`의 컴파일 오류는 0개였으나 최종 `Product compile/deploy completed` 표식은 아직 없었다. 사용자의 마무리 요청에 따라 추가 조사는 종료하며, 이미 실행한 빌드는 계속 진행 중인 상태로 보고했다. 최종 링크·배포 성공이나 실제 Play All 표시 성공으로 기록하지 않는다.
