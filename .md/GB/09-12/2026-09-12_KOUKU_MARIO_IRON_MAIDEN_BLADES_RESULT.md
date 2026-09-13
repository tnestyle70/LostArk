# 마리오 아이언메이든·일반/붉은 칼날 등록 결과

## G00. 실제 반영

아이언메이든 skeletal 모델과 4개 원본 모션을 추출·변환해 기존 World Object 경로에 등록하고 `WorldSequences` scope로 게시했다. 정적 원본 아이언메이든 모델 Effect/파괴 Effect 및 일반·붉은 칼날 Effect 네 문서를 V1 catalog/tree와 Kouku Composition `presentationResources`, Client 프로젝트/filters의 None 목록에 연결했다. 원본 payload가 이미 있던 세 문서는 표시명만 바꾸고 미등록 목록을 연결했다. 일반 칼날 문서 하나는 원본 first-LOD/module/default에서 새로 투영했다.

사용자 Pattern의 박스·시점·감금·칼날 경로를 자동 배치하지 않았다. 자산을 선택해 직접 조립하는 요청 범위다. 새 C++/shader/runtime 경로는 만들지 않았다. Resources는 Git에 추가하지 않았다.

## G01. 선택할 목록과 실제 파일

Effect Tool V1의 `All Effects → KoukuSaydon → 3관문 → 패턴 → 세이튼 → 마리오` 아래다.

| 하위 폴더 | 표시명 | 실제 원본 문서 suffix | 요소/문서 재생 길이 |
|---|---|---|---|
| 아이언메이든 | 마리오_아이언메이든_고정모델 | `fx_mn_rpct_07_d.par_d_boim_buff01_02l` | 1 / 20초 |
| 아이언메이든 | 마리오_아이언메이든_파괴이펙트 | `fx_mn_rpct_07_d.par_d_boim_buff01_03e_loc_int` | 8 / 2.2초 |
| 칼날 | 마리오_일반칼날 | `fx_mn_rpct_07_v.par_v_rpct_cutting_pjt_01` | 9 / 4초 |
| 칼날 | 마리오_붉은칼날 | `fx_mn_rpct_07_v.par_v_rpct_cutting_pjt_02_loc_int` | 10 / 4초 |

공통 stable ID 접두사는 `effect.kouku.source.`이며 파일은 `Data/Effects/Authored/<ID>.effect.json`이다. 전체 문서 길이는 모든 요소가 내내 보인다는 뜻이 아니다. 기존 원본의 각 particle lifetime·상대 시간·velocity를 유지하며 칼날의 게임 월드 진행 경로와 damage/감금 로직을 추가한 것은 아니다.

일반/붉은 구분은 표시명 추측이 아니다. 원본 Projectile `421991301/303`, `422204001`은 `_01`을, `421991302`는 `_02_loc_int`를 참조한다. 일반 color12 RGB는 `(2,2,2)`, 붉은 color12는 `(10,1,1)`, color8은 `(5,1,1)`이다. source evidence는 `out/KoukuMarioAssets20260912/iron_model/blade_source_identity.json`이다.

일반 칼날의 material/VF/VS/PS 11개 조합은 이미 설치된 native와 동일하다. `normal/installed_native_reuse.json`의 2440/2490/2472/2314/2475/2474 및 기존 2817/2715/2818/3161/2430을 재사용했다. out에 생성한 추가 native 후보는 제품에 설치하지 않았다. geometry/DDS는 붉은 칼날과 같은 설치 입력을 재사용했다.

## G02. 아이언메이든 World Object

설치 경로는 `Client/Bin/Resources/Effect/KoukuSaydon/WorldObjects/IronMaiden/IronMaiden.wmodel`과 같은 폴더의 `textures/` DDS 4개다. WModel 772,240 bytes, 전체 5파일 3,132,048 bytes를 new-or-equal로 설치했다. modelPreScale은0.01이며 원본14본에 converter wrapper1본,6,769정점/9,416삼각형이다.

원본은 `MN_BOIM_00`의 `mn_boim_00.mesh.mn_boim_00_sk`, AnimSet은 `mn_boim_00.ani.mn_boim_00_ani`다. 다음 네 clip을30Hz로 변환했다.

| 모션 | 원본 clip | 시간 |
|---|---|---|
| 기본 자세 | `idle_normal_1` | 약0.0333초 |
| 공격 | `att_battle_01` | 약1.1667초 |
| 파괴 | `dead_1` | 7초 |
| 파괴 뒤 자세 | `dead_1_loop` | 약0.0333초 |

`world.object.kouku.iron_maiden`의 표시명은 `아이언메이든`이며 기본 모션은 idle다. 원본 clip 이름을 가진 template/instance 네 개는 `HOLD`로 마지막 자세를 유지한다. `Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json`에 object1/template4/instance4만 추가했다. revision676→677이며 기존 배열 행은 모두 보존했다. 실제 runtime 게시물과 authoring의 JSON은 동일하다. publisher는 개행을LF로 정규화하므로 byte 전체 동일성으로 판단하지 않았다.

사용자 경로는 F1 `World Object Tool`에서 해당 Area의 `아이언메이든`을 선택해 네 모션을 확인하거나, Action Workbench `Resources → World → World Objects → 아이언메이든 → Append Object`다. Box Detail에서 위치·회전·수명을 설정한다. 원점은 저작 기본값이며 원작 감금 위치나 플레이어를 자동 선택하지 않는다. 실행 중 Tool은 새 목록을 재로드하거나 Client를 다시 열어야 한다.

## G03. 완료하지 않은 원본 경계

원본 등장 clip `respawn_1`0.5초는 UModel이 `animation is not supported`로 제외했다. 가짜 대체 모션은 없다.

skeletal 원본 재질 `mn_boim_00.mat.mn_boim_00_mi`의 MODEL Base/Light pair는 현재 미등록이다. 설치 모델은 원본 D/N/S/E embedded 입력을 유지한 변환본이며 native 표면/환경반사까지 복원한 것으로 기록하지 않는다. static Effect의 `_mi_dead`/native3107은 다른 MIC이므로 skeletal에 복사하지 않았다. 정적 모델 Effect는 기존 native3107을 사용하는 별도 선택 항목이다.

추가 조사한 칼날 충돌 Effect의 원본 `epcc_freezemovement`는 현재 codec가 승인하지 않는다. 요청한 일반/붉은 칼날 비행 Effect와 구분하고 충돌 문서는 원본을 변경하거나 새 목록에 등록하지 않았다. 이 경계를 `kill`로 바꿔 원작 동작처럼 처리하지 않았다.

## G04. 실행한 검사

- 기존 실제 Client codec/CPU playback probe로 등록 대상4문서의 Load/Validate_Drawable/Save roundtrip,60Hz 시간 진행·반복 Seek 통과. WARP device와 실제 CEffectObject Stage_Document도4문서 모두 통과했다. 창/swapchain/draw/UI는 실행하지 않았다.
- 검증 binary는17:51/17:52산출물로20시 이후 다른 작업의 Playback/Renderer 변경을 재검증한 것은 아니다. 현재 codec의 source/obj시점을 함께 확인했다. `validation/*existing_probe*.json`에 실행 범위를 남겼다.
- skeletal 원본 quaternion3,514샘플 최대오차6.29e-7,bind matrix 최대오차1.53e-5,36pose sample finite. 실제 모델 parser로 bone/animation/재질경로/수치와 설치 SHA를 확인했다. `iron_model/validation.json`, `installation.json`을 따른다.
- `Publish-MapAuthoring.ps1 -AreaId LV_LUT_MIDNIGHTC_ED -Scope WorldSequences -Mode Publish` 성공. source/runtime revision677 JSON동일,기존 행 보존·9개 신규 행 확인.
- catalog/tree/Composition resource의 unique ID와 재생 길이, project/filter각1회 등록 및 JSON/XML parse 통과. 변경 범위 `git diff --check`는오류0이다. `final_assets_check.json`, `world-publish.log`에 근거가 있다.

등록 직전/직후 Composition의 원본 필드와 기존 resource를 비교해 해당 순간 다른 값의 변경이 없음을 확인했다. 작업 시작 baseline과는 사용자의 동시 Pattern/folder 저작 변경이 있어서 전체 파일이 같다고 기록하지 않는다. World수정도 직전bytes를 보존하고 compare-before-write로 적용했다. 대규모 dirty checkout이므로 자동 stage/commit/push하지 않았다.

Client/UI·스크린샷·사용자 visual판정은 실행하지 않았다. 이 자산 등록에는 새 제품 C++/shader가 없어 별도 제품 컴파일이 필요하지 않았다. 이후 요청한 모든 Full Restore의 스킬별 bloom구현은 별도 `2026-09-12_EFFECT_PER_SKILL_BLOOM_IMPLEMENTATION_PLAN.md`와 대응RESULT가 소유한다.
