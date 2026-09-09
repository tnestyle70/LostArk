# 쿠크 네 다리 화염 인형 모델 식별 결과

작성일: 2026-09-09. 현재 상태: **후속 작업에서 모델·30개 clip·texture 4개 설치 및 CModel 검증 완료 / 사용자 외형 확인 전**.
후속 정본은 [실행 RESULT](C:/Users/user/Desktop/LostArk/.md/GB/09-09/2026-09-09_MAP_MATERIAL_COMPARISON_AND_REFERENCE_MODEL_RESULT.md)다.
아래 미설치·UModel 실패 서술은 최초 식별 당시의 조사 이력이며 현재 설치 상태가 아니다.

## 식별 결과

가장 강한 후보는 **`MN_CDMD_00`, 원본 NPC 설명명 `괴기스러운 인형`**이다.
설치 게임의 LookInfo가 정확한 SkeletalMesh·Material·AnimSet을 연결하고,
기존 추출 Action이 `괴기스러운 인형_회전 화염 뿜기`와 실제 clip 이름을 명시한다.
이 두 source 계약은 확인했다. 네 다리 skeleton과 첨부 이미지의 줄무늬·체크무늬 texture를
원본 geometry로 대조하는 단계는 완료하지 못했으므로 이미지 속 모델과의 동일성을 최종 확정하지 않는다.

| 항목 | 확인한 원본 식별값 |
|---|---|
| NPC | `480641`, `480742`, `480746`, `480747`: `괴기스러운 인형` |
| LookInfo | `EFDLChar_MN_CDMD_00.MN_CDMD_00` |
| SkeletalMesh | `mn_cdmd_00.Mesh.MN_CDMD_00_SK_LOC_INT` |
| Material | `MN_CDMD_00.Mat.MN_CDMD_00_MI` |
| AnimSet | `MN_CDMD_00.Ani.MN_CDMD_00_Ani` |
| PhysicsAsset | `MN_CDMD_00.Mesh.MN_CDMD_00_Physics` |
| Action group | `MN_CDMD_00` |
| 설치 원본 UPK | `C:/ProgramData/Smilegate/Games/LOSTARK/EFGame/ReleasePC/Packages/9G1MBI9I1BZZE74HQH64SSP.upk` |

이는 Unreal 원본 object 경로다. 현재 제품의 Resources-relative asset ID로 등록된 경로는 아니다.
`Client/Bin/Resources/Character`의 경로와 WModel 내부 문자열, 현재 actor/map/animation/effect
저작 catalog에서 `MN_CDMD` 또는 `OddDoll` 모델 연결을 찾지 못했다.
**최초 식별 당시에는 PC의 runtime Resources에 이 모델이 설치되어 있지 않았다.**
아직 생성하지 않은 `.wmodel` 경로를 이미 존재하는 asset ID처럼 제시하지 않는다.

## 이미지 관찰과 확인 범위

사용자 첨부 이미지를 직접 열람했다. 작은 보라 모자, 밝은 둥근 얼굴, 검은 턱·머리 부분,
금장 무늬의 둥근 몸통, 흑백 세로줄과 체크무늬가 서로 다른 다리 부분, 좌우 주변의 불꽃이 보인다.
사용자는 네 다리와 양방향 불 뿜기 동작을 설명했다. 정지 이미지에는 다리 일부가 겹치거나
가려져 있어 이미지 하나만으로 독립적인 네 다리 bone chain 또는 화염 방출 방향을 확정하지 않았다.

원본에서 확인한 명칭은 `회전 화염 뿜기`다. 추가 Action payload 조사로 서로 다른
`FX_Prj_01`, `FX_Prj_02` 두 발사점에서 같은 화염을 발생시키는 계약도 확인했다.
이는 사용자 설명과 맞는 추가 근거다. 두 socket이 정확히 반대 방향을 향하는지는
원본 skeleton/socket transform을 읽지 못했으므로 미확정이다.
기존 `MN_PPCT_00`은 카드미로 세토이고, `MN_PPCC_00`은 서커스 공,
`MN_RPRL_00`은 룰렛판, `MN_RPPB_01`은 삐에로 상자이므로 별도 대상으로 구분했다.

## 확인한 애니메이션과 화염 연결

기존 추출 `MN_CDMD_00.loa`를 저장소의
`Tools/LevelPlacementExtractor/extract_action_effect_notifies.py`로 읽었다.
아래 시간은 Action의 Anim notify에 직렬화된 clip 길이다. cook된 WModel의 재생 검증값은 아니다.

| Action ID | 원본 표시명 | clip | 길이 |
|---|---|---|---:|
| `4222305` | 괴기스러운 인형_회전 화염 뿜기 | `Att_Battle_2_01` | 11.333333초 |
| `4222308` | 괴기스러운 인형_회전 화염 뿜기_횡스크롤 | `Att_Battle_2_01` | 11.333333초 |
| `4222310` | 괴기스러운 인형_회전화염 뿜기_밈 섬 | `Att_Battle_2_01` | 11.333333초 |
| `4222311` | 괴기스러운 인형_회전화염 뿜기_노말 난이도 | `Att_Battle_2_01` | 11.333333초 |
| `4222301`~`4222304` | 용수철 튕기기 전방·후방·측면 | `Att_Battle_1_F/B/R/L` | 각각 2초 |
| `4222306`, `4222309` | 괴기스러운 형상 / 횡스크롤 | `Att_Battle_3_01` | 5.3초 |
| `4222307` | 괴기스러운 인형_자폭 | `Att_Battle_4_01` | 2.933333초 |

`4222305`의 각 stage는 다음 particle source를 사용한다.

| 시점 | duration | source particle |
|---:|---:|---|
| 0초 | 1.646356초 | `FX_MN_CDMD_00.Par_L_CDMD_00_Sk_02_1` |
| 1.539113초 | 9.019805초 | `FX_MN_CDMD_00.Par_L_CDMD_00_Sk_02` |
| 10.629546초 | 0.703753초 | `FX_MN_CDMD_00.Par_L_CDMD_00_Sk_02_2` |

연결 음원은 `S_Mob_OddDoll1.OddDoll1_Attack02_CastVox1`,
`OddDoll1_Attack02_Cast1`, `OddDoll1_Attack02_Shot1`이다.
따라서 모델 animation만으로 화염까지 표시된다고 볼 수 없으며, 후속 반영은 모델/clip과
이 particle·sound 발생 시점을 함께 연결해야 한다.

### 두 발사점의 Action payload 확인

기존 `build_action_cue_recipe.py::decode_typed_payload`로 `CEFParticleData`의
attachment selector와 anchor 문자열 배열을 읽었다. `4222305`의 준비·본체·종료
particle notify 모두 `FOLLOW_NAMED_ANCHORS`이며 `FX_Prj_01`, `FX_Prj_02`를 함께 지정한다.
첫 stage의 원본 notify byte offset은 각각 `79753`, `81273`, `87061`이다.
단순 문자열 검색 결과가 아니라 selector count와 뒤따르는 transform/parameter 구조까지
정상 decode한 결과다.

| Action | source anchor 배열 |
|---|---|
| `4222305` 일반 회전 화염 | `FX_Prj_01`, `FX_Prj_02` |
| `4222310` 밈 섬 / `4222311` 노말 난이도 | `FX_Prj_01`, `FX_Prj_02` |
| `4222308` 횡스크롤 | `FX_Prj_01` |

같은 clip을 쓰는 횡스크롤 변형은 발사점 하나만 지정한다. 따라서 **일반 회전 화염의
두 발사점 발생과 횡스크롤의 한 발사점 발생은 원본에서 구분되어 있다.**
actor socket 자료를 제공하지 않은 decoder의 두 anchor resolution은 모두
`MISSING_SOURCE_SOCKET`이다. 결과 JSON에 들어간 socket transform 기본값은 실제 원본
socket pose의 측정값이 아니므로 반대 방향 판단에 사용하지 않는다.

기존 `FX_MN_CDMD_00` particle graph의 본체 `Par_L_CDMD_00_Sk_02`도 확인했다.
이 graph에는 actor의 `FX_Prj_01/02` bone/socket 정의가 없으며, 발사점 선택은 위 Action이
소유한다. particle graph만으로 두 socket의 상대 회전이나 네 다리 geometry를 복원할 수 없다.

## 실제 확인과 남은 경계

- LAN sync를 먼저 실행했다. `server-host`, 방화벽 준비 완료, 공유 endpoint는 `192.168.0.14:7777`, probe는 `not-listening`이었다.
- 기존 NPC SQLite를 읽기 전용으로 조회하고, 설치 `data4.lpk`의 정확한 LookInfo 항목을 읽어 위 object 연결을 확인했다. 설치 원본은 변경하지 않았다.
- 기존 `C:/LostArkExtract`, Desktop `Resources`/모델 폴더와 보유 UModel 조사 폴더에서 같은 이름의 기존 skeletal export, texture, raw LOD 또는 runtime WModel을 찾지 못했다. 문서의 옛 `C:/Users/user/Desktop/Resource_LostArk` 경로는 현재 PC에 없다.
- UModel CLI `-list MN_CDMD_00`은 위 physical package를 찾았지만 NameTable 읽기에서 `LZ4_decompress_safe returned -18`로 실패했다. 모델 export 결과, skeleton bone 목록, texture 대조 결과는 생성되지 않았다. 같은 오류를 반복 재시도하거나 Client/UI를 실행하지 않았다.
- 소스 코드·Resources·publisher를 수정하지 않았고, compile 또는 제품 실행 검증은 이 읽기 전용 모델 식별 작업의 범위에 포함하지 않았다. 사용자 화면 확인을 PASS로 기록하지 않는다.

후속 단계는 이 exact package의 기존 정상 export 확보 또는 해당 package decoder 문제 해결,
`MN_CDMD_00_SK_LOC_INT`의 네 다리 rig·texture 대조, 기존 `CModel -> CMaterial` 경로의
WModel cook과 Resources 설치다. 그 다음 World Object/Composition Resources의 stable asset 등록,
`Att_Battle_2_01`과 화염·sound 타이밍 연결, 사용자 아레나 재생 확인을 진행할 수 있다.

## 근거 위치

- [이번 조사 식별값과 source offset](C:/Users/user/Desktop/LostArk/out/KoukuOddDollAudit20260909/identification.json)
- [화염 Action의 두 anchor decode 근거](C:/Users/user/Desktop/LostArk/out/KoukuOddDollAudit20260909/flame-anchor-evidence.json)
- [live LookInfo 읽기 결과](C:/Users/user/Desktop/LostArk/out/KoukuOddDollAudit20260909/LookInfo-MN_CDMD_00.loa)
- [기존 원본 Action](C:/LostArkExtract/LV_LUT_MIDNIGHTC_ED_20260829/RemainingCharacterExtraction-20260906/loa/MN_CDMD_00.loa)
- [NPC 원본 조회 DB](C:/LostArkExtract/LV_LUT_MIDNIGHTC_ED_20260829/WorldObjectExtraction-20260907/EFTable_Npc.db)
- [기존 화염 particle source graph](C:/LostArkExtract/LV_LUT_MIDNIGHTC_ED_20260829/CanonicalSource/Effect/Graphs/LV_LUT_MIDNIGHTC_ED.particle-graph.json/FX_MN_CDMD_00.particle-graph.json)

out의 파일은 이번 식별 근거이며 제품 runtime 입력이나 배포 manifest가 아니다.
