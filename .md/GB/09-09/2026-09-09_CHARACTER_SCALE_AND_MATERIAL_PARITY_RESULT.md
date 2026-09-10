# 캐릭터 크기·재질 연결 실측 결과

## G11. TJ 리소스와 캐릭터 재질 통합 — 2026-09-10

PR 352의 커스터마이징과 현재 재질 복구를 통합했다. 후속 사용자 요청으로 PR 353의
충돌 해결·Debug 빌드·머지와 기존 `GB_Resources` 갱신까지 진행한다. 아래 G10과
09-09 기록은 당시 상태이며, 전체 원본 재질의 시각 복구 완료를 뜻하지 않는다.

### 실제 반영

- Artist 기본 본체·장비·머리·붓 8모델의 21개 재질을 원본 MIC에 연결했다.
  기존 face/eye/eyeAO/base body의 PR 352 값은 보존하고, 의상·붓·헤어에 정확한
  source program 15~20을 추가했다. 기본 의상 3행의 generic family는 원본 family로 바꿨다.
  `CActorCatalog -> CModel -> CMaterial` 경로와 parameter·texture mask 검증을 유지한다.
- Artist 본체는 TJ의 눈 본 가중치·눈 UV1/UV2를 유지하며 hair submesh에 UV1 3,724개를
  추가했다. 별도 기본 머리는 두 submesh의 UV1 5,694+144개를 복구했다. 원본 native half
  대조로 확인된 UModel의 11개 zero UV 표현만 회복하고 기존 정점·가중치·UV0는 유지했다.
  source program별 추가 UV 검사를 확장하고, masked hair 19도 원본 TwoSided를 소비한다.
- Warlord legacy head program12의 Light varying을 원본 VS/PS의
  `v2=UV, v3=tangentLight, v5=tangentView, v6=sourcePosition`에 맞췄다.
  기존 공통 배치의 0 light 벡터가 만들던 NaN을 제거했으며 PR 352의 실제 얼굴 N/D/S를 유지한다.
- DimensionMaster는 현재 236개 본·헤어 UV1·본체 154개 clip·Esther clip을 유지한다.
  TJ Customizing 5개 clip은 원래 채널·key·time bytes를 보존하고 skeleton hash만
  현재 236본에 맞췄다. Artist/Warlord/LanceMaster의 TJ 본체·Customizing 변경도 보존했다.
- runtime 후보 102개 중 신규 83개·갱신 7개를 설치했고 동일 12개는 유지했다.
  추가로 PR 352 LanceMaster 눈의 누락된 공용 DDS 2개를 원본 추출본에서 설치했다.
  교체 전 파일은 `out/CharacterPR352Audit20260910/before-runtime-install`에 보존했다.

### 검증과 보류

| 항목 | 실제 결과 |
|---|---|
| 두 Character 폴더의 변경 9개 WModel | material chunk 동일. 정점 위치/normal/UV0/index 동일. Artist 눈 가중치·추가 UV, DimensionMaster rig, Customizing clip 차이를 별도로 확인 |
| 기존 face morph 연결 | 네 클래스의 morph/map 동일. 주소·rest position·UV0 대응 일치 |
| 통합 모델 후보 | 8개 기본 후보의 462 animation, 103,081 channel, 15,445,536 key 구조 검사 통과. Artist 추가 UV는 별도 원본/후보 byte 검사 통과 |
| Warlord 실제 원본 입력 WARP | 27조건·110,592픽셀. 기존 varying은 모두 NaN, 수정 후 nonfinite 0과 양수 RGB 110,592개. Client 화면 검증 아님 |
| 최종 재질 JSON | 63개 override의 parameter 이름 집합·texture mask·물리 texture 참조 검사 통과 |
| 프로젝트 | Client/Engine vcxproj와 filters 4개 XML parse 통과 |
| Debug Product 빌드 | PASS. Engine·Shared·Server·Client 컴파일/링크 오류 0. `out/BuildPipeline/runs/20260910T071032099Z-debug-product.json`의 모든 step PASS, missingRuntimeInputs 없음. 기존 인코딩·shader·PDB 경고는 남음 |
| Git whitespace 검사 | `git diff --check`와 `git diff --cached --check` 통과 |
| PR 충돌 | Part_Equipment의 본/재질 패스와 CharacterCatalog의 TJ/원본 재질을 함께 보존하여 해결 |
| 사용자 화면 | 새 통합본 확인 전. visual PASS를 기록하지 않음 |

차원술사 무기는 사용자 요청에 따라 **추가 복원 보류**다. 사용 재질 19개에 누락 texture는
없고 빈 material record 5개는 사용 정점이 없다. 실제 전체 Base/Direct WARP 30조건의
114,060픽셀은 모두 유한하므로 중간 `1/0`을 최종 화면의 NaN 원인으로 단정하지 않는다.
다만 cube ON/OFF 6쌍의 전체 Base 출력 bytes가 동일하여 환경반사 기여가 사라지는 현상은
확인했다. 미연결 native BRDF/SH의 정확한 원본 입력은 확보되지 않았다.
`character_select_project_brdf.dds`는 프로젝트 근사 생성물임을 확인했으며 이를 원본으로
위장해 연결하거나 무기 값을 추정해서 바꾸지 않았다.

도화가 추가 선택 장비 71모델의 기존 리소스는 보존한다. 전체 79모델·142재질 중
소스 추적 134행은 runtime 복구 수가 아니다. 해당 선택 장비의 새 source override 연결은
이번 기본 8모델 범위에 포함하지 않았다. 미해결 source 8행과 기존 owner 제한은
`artist-all-part-source-lineage.json`과 `artist-final-handoff.json`에 기록했다.

증거 위치는 `out/CharacterPR352Audit20260910/`이며 `compare-wmodels.md`,
`artist-hair-uv-candidate.json`, `dimension-warlord-material-result.md`,
`dm-weapon-full-warp/handoff.md`, `runtime-install.json`을 사용한다.
리소스 물리 전달본은 기존 `out/ResourceDelivery20260910/GB_Resources.zip`과 추출 폴더를 갱신한다.
Git PR의 JSON·shader·코드와 함께 사용해야 하며 WModel만으로 재질 식이 전달되지 않는다.

## G10. 차원술사 재질 복원 재개 — 2026-09-10

사용자는 환경반사가 부족하게 느껴진 장소를 **Character Select**로 확인했다.
아래의 09-09 조사·배율 기록은 당시 상태다. 이번 재개에서 실제 별도 헤어와 Character Select
환경 cube를 추가 연결했으며, 원작 SH·다른 씬의 cube까지 복구한 것으로 확대하지 않는다.

### 실제 수정

- 본체·장비·무기의 기존 source material은 유지했다. 별도 `pc_sp_m_55_hair/head.wmodel`에
  실제 source program 7 override를 추가하고 장비의 source hair/eyelash main pass를 6으로 연결했다.
  기존 숨긴 body hair mask 512는 유지한다. 그림자는 기존 pass 1 diffuse-alpha 경로를 사용하며
  원본 native hair shadow와의 동일성은 이번 검증 범위가 아니다.
- 본체의 기존 225 bone 순서를 유지하고 원본 hair55 bone 11개를 뒤에 연결했다. body/head와
  Esther/Customizing AnimSet의 4 WModel을 236 bone 기준으로 정합했다. 154+1+1 animation의
  key·event·clip bytes와 원래 geometry·weights·material을 보존했다. WANM trailer의 골격 참조
  hash156개는 확장한236bone WSKL과 맞췄다. 헤어 16,956정점의 UV1을 원본
  triangle corner와 native half 자료로 복구했다. asset service는 차원술사 body/equipment의
  bone hash/name/parent/rest 호환성을 prototype commit 전에 검사한다. 원본 pack 계약이 다른
  워로드 등 다른 클래스에 이 exact 검사를 강제하지 않는다.
- `RenderingProfileService`의 optional `environment`가 cube Resources ID, RGBM6 color/offset,
  rotation/intensity를 저장한다. Renderer는 cube SRV를 먼저 stage하고 quality/shadow/fog/light가
  모두 성공한 뒤 commit한다. scene environment를 생략하면 이전 cube를 비우며, 실패하면 이전
  profile 상태를 유지한다. 같은 cube를 사용하는 품질 slider·mood 변경은 기존 SRV를 재사용한다.
  명시적인 catalog Reload는 cube를 다시 stage하므로 파일 변경과 로드 실패가 숨겨지지 않는다.
- `CMaterial`의 기존 source base 바인딩에서 scene 환경을 전달하며 원본 program 3/8/9의
  0으로 대체됐던 cube sample 3곳을 실제 `SampleLevel`로 연결했다. 기존 방향·LOD·RGBM6
  decode 계산은 유지한다. 모코코·워로드의 material-owned 2D IBL을 복사하지 않았다.
- authored RenderingProfiles revision 25의 `scene.character-select.warm-high-key.v1`과
  같은 맵의 `scene.character-select.customizing-dark.v1`에 기존 map cube를 연결했다.
  `Map/Lighting/CharacterSelect/lv_lut_valhatrond_04_hdr01.rgbm.cube.dds`는 128×128×6,
  8 mip, DXGI B8G8R8A8_UNORM, RGBM6 입력이다. color `[1,1,1,0]`와 rotation/intensity
  `[0,1,1,0]`는 해당 map의 기존 프로젝트 선택을 공유하며 원작 전역 probe 선택값으로 표기하지 않는다.

실제 C++ decoder 재검사에서 body가 거절되고 animation 없는 head는 통과했다. WAnimationReader는
WANM 끝의 skeletonHash와 WSKL의 일치를 요구하는데, 초기 조립본이 이전225bone 해시를
보존한 것이 원인이었다. body154개와 외부2개 clip의 끝8byte만
`0x43588e0806b2ee7f`에서 `0xb8ac2e42de1c269e`로 교정했다. 이외 byte는 모두 동일하며
재조립 도구도 같은 참조 동기화를 수행한다. 근거는
`out/CharacterMaterialRestore20260908/geometry/hair_resume/animation_trailer_hash_repair.json`이다.

사용자가 올린 Visual Studio 오류 목록의 `Part_Equipment.cpp:124` C3083은 실제 컴파일 오류였다.
헤어 pass 분기가 참조하는 `MODEL_SURFACE_FAMILY`와 `MODEL_SURFACE_PARAMETERS`의
완전한 정의 헤더가 빠져 있었다. 같은 타입을 사용하는 Part_Body와 동일하게
`BinaryAsset/ModelAssetData.h`를 직접 include했다. 기존 UTF-8/CRLF를 보존했다.
아래 Product 빌드는 이 실제 소비자 CPP와 새 환경반사 셰이더까지 포함한다. 환경반사 경로 변환의
C4996 두 건도 C++20에서 폐기된 `filesystem::u8path`를 프로젝트의 기존
`filesystem::path(u8string(...))` 방식으로 교체해 닫았다. UTF-8 path 의미와 경로 검증은 유지했다.

### 현재 검증 증거와 남은 경계

| 구분 | 실제 결과 |
|---|---|
| 헤어 리소스 수치 | 4 WModel 설치, 범위를 넘던 positive-weight 정점 13,057개→0. rest/idle0 16,956정점 finite, 자기 palette와 body palette 위치 차이 0 |
| Rendering publisher | Validate 통과. 기존 test 파일에서 environment 저장·재로드 및 잘못된 9개 입력의 거절·이전 runtime bytes 보존 통과 |
| 실제 cube 숫자 readback | 기존 `s_native_ps_probe.exe`로 원본 program 3 sample/decode 부분만 검증. 6면×8 mip×켜짐/꺼짐×3회=288 case, nonfinite 0. 꺼짐 144 case RGB 0, 켜짐 144 case 모두 RGB 양수, 최대 1.50920427 |
| shader 범위 | 위 수치는 실제 cube SampleLevel/RGBM6 decode 부분이며 full material·Client 화면 또는 원작 visual fidelity 판정이 아님 |
| 실제 헤어 CModel | 현재 Engine에서236bone/154clip body + 외부2clip Attach/Clone,156clip finite, program7/pass6 바인딩, 이전UV/225bone AnimSet 거절과 live clone 보존 PASS·failures0 |
| C++·최종 Product build | Debug Engine/Shared/Server/Client compile·link·DLL/CSO 배포 PASS. `out/BuildPipeline/runs/20260910T063843956Z-debug-product.json` |
| 새 제품 바이너리의 재질 바인딩 | 실제 Product DLL/CSO를 probe에 복사해 hash 일치 확인. 창술사+차원술사10모델/23override/24slot, 워로드9모델/14slot의 실제 CModel/CMaterial 바인딩 모두 failures0 |
| runtime profile publish | 공식 publisher Publish PASS, authored/runtime 모두 revision25와 두 Character Select environment 입력 일치 |
| source 공백·인코딩 | scoped `git diff --check` 통과. 기존 C++ UTF-8/CP949와 CRLF 유지 |
| 사용자 화면 | 미실시. 사용자가 Server + Client profile을 Ctrl+F5로 시작해 Lobby → Character Select → 차원술사에서 확인 |

세부 근거는 [헤어 설치와 수치](C:/Users/user/Desktop/LostArk/out/CharacterMaterialRestore20260908/geometry/hair_resume/hair_restore_receipt.json),
[publisher 검사](C:/Users/user/Desktop/LostArk/out/DimensionMasterMaterialResume20260910/publisher-environment-test.log),
[cube 수치](C:/Users/user/Desktop/LostArk/out/DimensionMasterMaterialResume20260910/cube-numeric-result.json),
[cube 검사 범위](C:/Users/user/Desktop/LostArk/out/DimensionMasterMaterialResume20260910/cube-seam-source.json),
[실제 헤어 CModel](C:/Users/user/Desktop/LostArk/out/DimensionMasterMaterialResume20260910/hair-activation-before-product.json)에 있다.
기존 headless 소비자와 out fixture를 사용했으며 새 runtime·전용 프레임워크·Client 실행은 추가하지 않았다.
최종 근거는 `out/DimensionMasterMaterialResume20260910/final-product-build.log`,
`final-rendering-publish.log`, `final-probe-inputs.json`, `final-character-activation.json`과
`final-warlord-activation.json`이다. 입력 기록의 DLL/CSO는 실제 `Client/Bin/Debug`와 byte exact다.
기존 코드페이지·FXC·외부 PDB 경고는 남아 있으며 경고0이나 화면 PASS를 뜻하지 않는다.

현재 Bern/쿠크의 runtime map material에는 environment 연결이 없고 설치된 대응 cube도 확인되지
않았다. 원본 package에 해당 cube가 존재하지 않는다는 판정은 아니다. 이번 사용자 확인 장소가
Character Select이므로 다른 씬 추출을 추가하지 않았다. 원작 SH packing/전역 environment 선택,
헤어 먼 거리 mip·sorted transparency·native shadow 및 실제 화면 인상은 별도 남은 경계다.

작성일: 2026-09-09. 상태: **조사·전체 복구 계획 작성 / 차원술사 1.5배 외형 코드 반영**.

대응 [구현 계획서](C:/Users/user/Desktop/LostArk/.md/GB/09-09/2026-09-09_CHARACTER_SCALE_AND_MATERIAL_PARITY_IMPLEMENTATION_PLAN.md)의
G00 실측 근거, 사용자의 후속 요청에 따라 먼저 적용한 차원술사 1.5배와 남은 실행 경계를 기록한다.
G01·G05~G08의 헤어/재질/환경광 복구와 다른 클래스 확대는 구현하지 않았다.

## 조사 기준과 산출물

기준 브랜치는 `codex/dimensionmaster-tool-round3`, 조회 HEAD는
`591012dbebf7eeab0b660baec42852b9396e77d4`였다. origin/main과 같은 HEAD였으나
다른 세션의 대규모 미커밋 코드·데이터·문서 변경이 진행 중인 working copy를 조사했다.
기존 변경을 되돌리거나 stage/commit/push하지 않았다.

- [크기 측정값](C:/Users/user/Desktop/LostArk/out/CharacterSizeAudit20260909/measurements.json)
- [CPU 측정 방법](C:/Users/user/Desktop/LostArk/out/CharacterSizeAudit20260909/measure_character_sizes.py)
- [모델별 실제 material 소비 조사](C:/Users/user/Desktop/LostArk/out/CharacterMaterialConsumerAudit20260909/character_inventory.json)
- [새 헤어 DDS](C:/Users/user/Desktop/LostArk/out/CharacterMaterialConsumerAudit20260909/added_hair_dds.json)
- [현재 추가 UV](C:/Users/user/Desktop/LostArk/out/CharacterMaterialConsumerAudit20260909/current_uv_flags.json)
- [조사 시점 소스 식별](C:/Users/user/Desktop/LostArk/out/CharacterMaterialConsumerAudit20260909/source_snapshot.json)

out의 입력 식별값은 이번 진단의 재현 자료이며 runtime admission, Resource 배포 manifest나
신규 전수 검증 절차가 아니다. 현재 파일이 후속 구현으로 바뀌면 당시 조사값과 최신 상태를 구분한다.

## 크기 실측

CPU에서 실제 WModel 정점/weight/inverse bind와 WSKL hierarchy를 읽고, 클래스의 기본 battle idle
첫 pose를 적용했다. body palette를 사용하는 equipment와 bone socket을 사용하는 weapon의 실제 흐름,
기본 장착·body hidden mesh mask를 반영했다. 사용자 추가 Transform과 camera projection은 적용하지 않았다.

| 대상 | asset preScale | idle0 무기 제외 높이 | rest 무기 제외 높이 | 모코코 idle0 높이까지 배율 |
|---|---:|---:|---:|---:|
| 모코코 창술사 | 0.0001 | 1.5001m | 1.4026m | 1.0000 |
| 도화가 | 0.0001 | 1.0385m | 1.0139m | 1.4444 |
| 워로드 | 0.0001 | 1.3155m | 1.3174m | 1.1403 |
| 차원술사, 헤어 제외 | 0.01 | 1.0682m | 1.0844m | 1.4043, 참고값 |
| 건슬링어 | 0.0001 | 1.2481m | 1.0533m | 1.2019 |
| 슬레이어 | 0.0001 | 1.2392m | 1.1013m | 1.2106 |

차원술사 별도 헤어는 body palette 225개 범위를 넘는 bone index가 있었으며 최대 235였다.
양수 weight가 이 범위를 사용하는 정점은 13,057개였다. 임의 remap/clamp를 하지 않고 측정에서 제외했다.
따라서 헤어 포함 전체 아바타의 실제 높이나 최종 배율은 이번 자료로 확정하지 않는다.
도화가·워로드 등도 이 숫자는 pose AABB이며 화면 크기나 항상 일정한 신체 키가 아니다.

확대 반영 전 code는 asset service의 0.01/0.0001 단위 변환과 gameplay Transform parent를 사용했고
별도 CharacterCatalog 외형 배율은 없었다. body/장비/무기와 동적 교체 parent,
AnimationTargetService root 및 Effect owner presentation root를 확인했다.
확대는 새 presentation root에서 한 번 적용하고 gameplay Transform과 Server collision은 보존하도록 계획했다.

## 재질 실측과 확인된 누락

모코코 2모델/3재질은 source program 1, material-owned 2D IBL과 source texture/constant가
실제 CModel→CMaterial→part→Renderer 경로에 연결돼 있다.
차원술사 6모델의 19사용 material record 중 18개는 source program이 연결됐다.
다만 그 수에는 숨겨진 body hair가 있고 실제 새 헤어는 일반 재질 경로다.
도화가는 8모델/21record, 워로드는 9모델/14record에 source program 연결이 0개였다.

도화가와 워로드에도 diffuse/normal/specular, 직접광과 그림자 경로가 존재한다.
현재 참조 texture의 물리 누락은 0이다. “재질이나 반사가 아예 적용되지 않았다”가 아니라,
각 원본 shader family와 추가 입력이 연결되지 않았다는 판정이다.
원본 재질과 최종 화면이 같은지에 대한 사용자 판정은 받지 않았다.

| 확인 사항 | 실제 근거 | 남은 작업 |
|---|---|---|
| 차원술사 새 헤어 skinning | equipment는 body palette 바인딩, 정점 index는 그 범위 초과 | rig 정합 후 전체 크기 재측정 |
| 차원술사 새 헤어 native material | 숨긴 body hair는 program 7, 별도 hair override 0 | 실제 모델의 native UV·정확 material override 연결 |
| 헤어의 추가 UV | 새 hair minor 0/flags 31, body minor 3/flags 415 | program 7이 요구하는 UV1 복구. override만 추가하면 로드 거부 |
| 장비 source hair pass | body는 program 6/7 양면 pass, equipment는 고정 pass | main/shadow의 실제 material별 pass·coverage 연결 |
| 차원술사 환경반사 | source program 3/8/9의 texturecube 샘플이 0으로 대체 | scene-owned cube/SH 입력과 Renderer 소비 연결 |
| 도화가·워로드 원본 재질 | source program 소비 0/21, 0/14 | 정확 MIC/PS/VF·texture/UV·typed 입력을 전 모델 소비자에 연결 |
| 새 헤어 축소 sampling | Artist/DimensionMaster/LanceMaster D/N DDS 6개가 1 mip | mip0·채널 의미 보존한 full mip |
| 원본 투명·장면 표현 | ordered coverage 및 ambient/shadow 근사 경계 존재 | 실제 입력/장면 계약 복구와 사용자 비교 |

기존 TGA mip과 차원술사 weapon DDS mip 복구는 현재 반영돼 있었다.
그 과거 누락을 이번 조사에서 다시 미구현으로 기록하지 않았다.
스킬 Effect 복구 상태와 캐릭터 표면 material 상태도 구분했다.

최종 인계 전 독립 재확인에서도 23개 모델의 material section과 submesh material 연결은
조사 snapshot과 동일했다. 차원술사 18/19, 도화가 0/21, 워로드 0/14의 source program 수와
새 헤어 UV 부재·차원술사 body palette 초과 수치도 유지됐다. 조사 이후의 이펙트 셰이더 변경을
이 캐릭터 표면 재질 누락의 복구로 간주하지 않는다.

## 실제 반영: 차원술사만 1.5배

사용자는 조사 중 “차원술사만 일단 1.5배 정도 사이즈 키워서 적용”을 명시했다.
이 값은 헤어 제외 실측비 1.4043을 채택한 결과가 아니라 사용자가 직접 정한 외형 배율이다.
헤어 rig 복구를 이 변경의 선행 조건으로 붙이지 않았다.

- `CharacterCatalog.json`은 기존 format 4를 유지하고 차원술사 row에만 `presentationScale: 1.5`를 추가했다.
  JSON을 비교해 이 값 이외 데이터 변경이 없음을 확인했다. 다른 다섯 class는 기본값 1이다.
- `ActorCatalog.h/.cpp`는 optional 배율의 기본값 1과 finite·양수·최대 100 검사를 추가했다.
  기존 catalog의 stage→commit을 유지한다.
- `Character.h/.cpp`는 `Scale * gameplayWorld`를 소유하는 안정된 instance 행렬을 추가했다.
  생성과 Set_Position, network/attachment 반영 뒤 parts Update 전에 갱신한다.
  body/기본 equipment/weapon/교체 equipment 부모 4곳이 이 행렬을 사용한다.
- `AnimationTargetService.cpp`의 scene/preview/historical pose는 visual root를 소비한다.
  `Effect_PresentationService.cpp`의 bone anchor는 visual root를 사용하지만 Character `root` cue는
  gameplay world를 유지한다. `skill_target`, 명시 world root, Boss root의 기존 계약도 유지했다.
- 차원술사 asset unit 0.01, gameplay Transform/collider/Server 수치는 수정하지 않았다.
  class가 없는 쿠크 변신 avatar는 기존 크기를 유지한다.

독립 리뷰에서 clone/초기화, 이미 등록된 prototype의 catalog 조회, root 갱신 순서, 동적 장비 교체,
현재 Transform에서 계산하는 getter와 root Effect 분기를 확인했다. 새 오류는 발견하지 못했다.
WORLD_FOOTPRINT source-bone의 기존 policy 순서는 유지했다. 그 정책의 가상 player bone cue는
확대 bone offset을 상속하지 않는 경계가 남지만, 현재 player cue 제출은 OWNER_RELATIVE를 사용한다.

실제 검증 근거는 [verification.json](C:/Users/user/Desktop/LostArk/out/DimensionMasterScale20260909/verification.json)이다.
JSON parse, 차원술사 모델 6개 존재, 18개 회전·위치 조합의 독립 CPU 행렬 계산,
기존 UTF-8/CRLF 보존과 scoped diff check를 수행했다.
CPU 행렬 검사는 world translation 불변과 visual offset 1.5배 단일 적용 등을 확인한 것이며 Client 실행이 아니다.

변경한 `ActorCatalog.cpp`, `Character.cpp`, `AnimationTargetService.cpp`, `Effect_PresentationService.cpp`는
설치된 VS2022 x64 compiler로 각각 컴파일해 **4/4 exit 0**을 확인했다.
공유 Client 빌드와 산출물/PDB가 경합하지 않게 out의 별도 object에 동일 Debug C++20/include/define으로
컴파일했으며 debug 정보는 `/Z7`을 사용했다. 제품 object를 교체하거나 별도 runtime을 만들지 않았다.
[focused_compile.json](C:/Users/user/Desktop/LostArk/out/DimensionMasterScale20260909/focused_compile.json)에
source 식별, object/log 경로, exit code를 기록했다. 이 최소 컴파일은 최종 제품 링크와 구분한다.

## 실제 수행과 미수행

| 구분 | 이번 수행 상태 |
|---|---|
| LAN 설정 | Sync-TeamLanEndpoint 실행. server-host, 192.168.0.14:7777, firewall ready, probe not-listening |
| source/Resources 읽기 | 수행. 카탈로그·실제 WModel·DDS·shader·소비자 연결 확인 |
| CPU 크기/rig 진단 | 수행. 6클래스, 차원술사 헤어는 명시 제외 |
| 계획 문서 | 작성. 실제 변경 파일·소유자·호출 흐름·실패 보존·검증 범위 기록 |
| 제품 C++/H/JSON 수정 | 차원술사 외형 배율 7파일 반영. HLSL/Server 변경 없음 |
| Resources 설치·변환·Drive 배포 | 미수행 |
| 최소 C++ compile | 변경 CPP 4개 모두 exit 0. HLSL 변경/별도 compile 없음 |
| Client C++ 증분/link/배포 | 17:39 KST 명시 compile/link target exit 0, warning 0/error 0. 현재 output은 최신으로 판정 |
| 전체 최신 셰이더 반영 | 다른 세션 빌드 중·후 변경된 HLSL이 있어 별도 미완료. 아래 인계 상태 참조 |
| Client/Server UI 실행·조작·캡처 | 미수행 |
| 입력/Save/Play/아레나 visual fidelity | 이번 캐릭터 조사에서 실행하지 않음. 사용자 확인 전 |
| commit/push | 미수행. 공유 dirty worktree 보존 |

작성한 문서 7개의 UTF-8·링크 대상 존재·신규 파일 공백 검사와 조사 JSON 12개의 parse를 확인했다.
제품 변경 7파일과 팀 인터페이스 문서의 scoped `git diff --check`도 통과했다.
[documents_verified.json](C:/Users/user/Desktop/LostArk/out/DimensionMasterScale20260909/documents_verified.json)에 기록했다.
최소 컴파일은 기존 헤더의 C4819 경고가 남았고 error는 0이었다. 기존 파일을 일괄 재인코딩하지 않았다.
다른 작업의 빌드 성공이나 사용자 화면 관찰을 이 계획의 구현·검증 성공으로 승격하지 않는다.

## 최종 빌드와 실행 인계

2026-09-09 17:39 KST에 실제 Client 프로젝트의 `PrepareForBuild`, `ResolveReferences`,
`_ClCompile`, `_ResourceCompile`, `BuildLink`, shader/runtime 배포 target을 실행했다.
MSBuild는 C++·resource·link output을 모두 최신으로 판정했고 warning 0/error 0, exit 0이었다.
이 확인은 변경 CPP 네 개만 검사한 앞의 독립 컴파일과 달리 프로젝트의 header 의존 관계와
정상 link/deployment target을 사용한다. 이번 호출에서 새 컴파일이나 재링크가 발생한 것은 아니다.
`Character.cpp`와 `Character.h` 수정 시각은 16:57:30, 제품 `Character.obj`는 17:30:07,
`Client.exe`는 17:30:46이며, 검사 전후 이번 변경 7파일의 SHA-256이 동일했다.
[client_cpp_build.json](C:/Users/user/Desktop/LostArk/out/DimensionMasterScale20260909/client_cpp_build.json)에
정확한 targets, source 식별, EXE/DLL 식별과 exit code를 기록했다.

다른 세션의 전체 Product build는 17:30:46 PASS였지만, MeshPreview CSO가 생성된 16:58:53 뒤
그 셰이더가 직접 include하는 `Shader_EffectDimensionMasterQNative.hlsli`가 17:23:34에 변경됐다.
Particle source에도 17:35:30 변경이 있었다. 따라서 이후 일반 Build가 FXC를 다시 실행한 것은
오류만으로 볼 수 없으며, Product PASS receipt만으로 현재 모든 HLSL이 반영됐다고 판단하지 않는다.
이번 작업이 시작한 반복 FXC의 프로세스 소유 관계를 확인해 그 작업만 중단한 뒤 C++ target을
명시했다. 기존 두 CSO의 hash는 중단 전후 동일했다.
[중단 기록](C:/Users/user/Desktop/LostArk/out/DimensionMasterScale20260909/repeated_shader_build_cancelled.json)을 남겼다.
최신 이펙트 셰이더 반영은 해당 변경의 후속 빌드에서 확인할 별도 경계이며, 이를 이번 배율 변경의
C++ 오류나 이미 완료된 전체 셰이더 검증으로 기록하지 않는다.

최종 확인 시 Server와 Client process는 없었다. 사용자는 Visual Studio의 `Server + Client`
profile을 `Ctrl+F5`로 실행한 뒤 Lobby → Character Select에서 차원술사를 선택해 외형을 확인한다.
ActorCatalog는 process 첫 초기화에 읽으므로 이미 실행 중인 Client가 있으면 재시작해야 한다.
다른 다섯 class 크기, gameplay body radius, 차원술사 asset unit은 그대로다.
실제 크기 인상·조명·재질의 화면 판정과 조커찾기 Play는 수행하지 않았다.
