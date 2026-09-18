# 2026-09-18 발탄 Map Tool 컷신 배우를 레이드 발탄 조립으로 교체 RESULT

## 사용자 결정

1. 컷신 발탄은 프레임워크가 가진 레이드 발탄(`BossCatalog.json` `BOSS_VALTAN`: 몸통 + 갑옷 2 + 무기 + AnimSet)으로 쓴다. 발탄이 나오는 5개 배우 전부(등장 무채색 0~13s, 등장 본체, 최후, 버러지, 포효). 1관문 늑대는 범위 밖.
2. 원본 컷신 애니메이션(fork B 저작본 rev 2 의 클립 체인·시각·위치 키)을 레이드 발탄 모델로 재생한다.
3. 발탄 템플릿 키 yaw 에 접었던 −90° 를 되돌리고, 최종 화면 방향에 −90° 가 몇 번 적용되는지 확정한다.

## 1. 레이드 조립 경로 (확인)

| 단계 | 위치 |
|---|---|
| 모델 프로토타입 등록(몸통+AnimSet 부착, 무기 preScale 100·preRotation, 갑옷 plate) | `Client/Private/ValtanPresentationAssetService.cpp:75-234` `Ensure_Prototypes` |
| 발탄 레벨 로드 때 등록 | `Client/Private/Loader.cpp:1528-1545` `Ready_ValtanPresentation` (`BOSS_VALTAN`, `BOSS_VALTAN_GHOST`) |
| 몸통 파츠 −90° | `Client/Private/Body_Valtan.cpp:52` `m_pTransformCom->Rotation(0.f, -90.f, 0.f)` |
| 무기: 소켓 `b_wp_r_01`, 몸통 뼈 행렬 × 몸통 루트 | `Client/Private/Valtan.cpp:3948-3972`, `Client/Private/Part_Equipment.cpp:84-98` |
| 갑옷: 몸통 스켈레톤 palette 를 빌려 쓰는 skinned plate | `Client/Private/Valtan.cpp:3974-4008`, `Part_Equipment.cpp:135-141` |
| `Prototype_GameObject_Part_Equipment` 발탄 레벨 등록 | `Loader.cpp` `Ready_Character_Shared_Prototypes` (VALTAN_ARENA 의 `Ready_Character_Rendering` 경로) |

선택한 재사용 방식: 선택지 (a)+(b) 혼합.
- 오브젝트 리소스에 `presentationBossArchetypeId` 를 두고, 런타임은 `CValtanPresentationAssetService::Ensure_Prototypes` 로 레이드가 이미 등록한 몸통 프로토타입을 **복제**해 쓴다(몸통을 다시 디코드하지 않음).
- 갑옷·무기는 레이드와 같은 `CPart_Equipment` 를 `CWorldSequenceObject` 가 소유해 그린다. 부모 행렬은 배우의 `m_World`, 소켓 루트는 없음(레이드 몸체 파츠의 −90° 를 거치지 않음). 재질 프로필은 레이드와 같은 `material.valtan.monster-base.v1`.
- 두 번째 런타임 경로나 새 모델 로더는 만들지 않았다.

## 2. 바뀐 파일

| 파일 | 줄 | 변경 |
|---|---|---|
| `Client/Public/WorldSequenceDocument.h` | 93-97 | `WORLD_SEQUENCE_OBJECT_RESOURCE::presentationBossArchetypeId` |
| `Client/Private/WorldSequenceDocument.cpp` | 437, 514-519, 1039-1040, 1297, 1352-1355, 1863 | 선택 필드 허용·파싱·저장·그룹 금지·검증(비별칭 animated + 안정 ID)·동등성 |
| `Client/Public/WorldSequencePlayer.h` | 277-278, 337-338 | `OBJECT_MODEL::presentationBossArchetypeId`, `Admit_PresentationBossModel` 선언 |
| `Client/Private/WorldSequencePlayer_Objects.cpp` | 8-9, 30-53, 320, 324-362, 491-497, 694, 1024 | include, `Fill_PresentationParts`/`Narrow_PrototypeTag`, 모델 동일성 비교, `Admit_PresentationBossModel`(카탈로그 대조·레이드 프로토타입 복제·파츠/소켓 존재 확인, 실패 사유를 `m_Status` 로), 준비 분기, 두 생성 지점에서 파츠 채움 |
| `Client/Public/WorldSequenceObject.h` | 12, 18-26, 30-31, 62-63 | `CPart_Equipment` 전방 선언, `PRESENTATION_PART`, DESC 파츠·재질 프로필, 멤버 |
| `Client/Private/WorldSequenceObject.cpp` | 6, 58-77, 114-115, 170-172, 203-205, 236-238 | 파츠 생성(`&m_World` 부모, 소켓 루트 없음), `Sample` 직후 파츠 갱신, NONBLEND 등록, 몸통 재질 프로필 적용(불투명·반투명) |
| `Tools/MapPipeline/Publish-MapAuthoring.ps1` | 1436, 1510, 1541-1547 | 선택 필드 허용, 그룹 금지, 비별칭 animated + ID 형식 검증. 다른 fork 의 `Placements` Scope 변경은 해시 확인 후 보존 |
| `Data/Maps/Authoring/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.worldsequences.json` | — | rev 2 → 3. body 리소스에 `presentationBossArchetypeId: "BOSS_VALTAN"`. `entrance.colorless`·`finale` 인스턴스 바인딩 ghost → body. 발탄 템플릿 5개 키 41개의 yaw +90°. 파일은 생성기 출력이라 `json.dumps(indent=2, ensure_ascii=False)` 가 원본을 바이트 그대로 재현함을 확인한 뒤 같은 방식으로 썼다(LF 유지, 87+/86−줄) |
| `Client/Bin/DataFiles/Map/LV_LUT_HEARTRB_ED.worldsequences.json` | — | 게시 출력(저작본과 SHA-256 동일 `467469cc…`) |

유령 리소스(`world.object.valtan.source-preview.ghost`)는 참조가 없어졌지만 문서에서 지우지 않았다(Validate 통과).
새 C++ 파일·vcxproj/filters 변경 없음. `MapTool*.cpp`, `MainApp.cpp`, 전투 데이터, `BossCatalog.json`, 조명·재질 파일 무변경.

## 3. 클립 대조

유령·무채색에서 본체로 바꾼 두 배우를 포함해 5개 배우의 클립 22종(중복 포함 체인 기준) 전부가 `MN_RPBF_01.wmodel` + `MN_RPBF_01_AnimSet.wmodel` 에 있다(없는 것 0). 유령 AnimSet 에만 있는 이름은 없다. fork B 의 길이표(`out/Handoff2_G01G03G04/clip_durations.json`)로 유령/본체 같은 이름 클립의 길이가 같고, 두 배우의 sourceStart 가 본체 클립 길이를 넘지 않음을 확인했다. 런타임도 준비 단계에서 `World Object clip is absent` / source range 검사를 그대로 한다.

## 4. −90° 적용 횟수 — 최종 판정

- 레이드: 화면 몸통 = `R(−90)`(몸체 파츠) × 발탄 월드(서버 yaw). **1번.**
- 컷신 배우(이번 구현): 몸통 = 키 행렬(`scale × R(key yaw) × T`), 무기 = `b_wp_r_01` 뼈 행렬 × 같은 키 행렬, 갑옷 = 같은 키 행렬 + 몸통 palette. 레이드 몸체 파츠를 쓰지 않으므로 추가 회전 없음. 키 yaw = 원본 액터 yaw. **0번.**
- 근거:
  - [확인] 원본 컷신 배우는 폰이 아니라 `EFSkeletalMeshActorLookInfoMAT` 이고, 메시 컴포넌트에 회전 속성이 직렬화돼 있지 않다(액터 `rotation` 만 있음). 최후 액터 yaw −135, 등장 무채색 45, 등장 본체 56.25, 버러지·포효 −135(`out/ValtanCutsceneRaidAssembly20260918/dump_actor_rotation.py`). 되돌린 키의 첫 yaw 가 이 값과 정확히 같다(최후 −135, 무채색 45).
  - [추론] 레이드의 −90 은 폰 기준(서버 facing) 방향을 모델 전방축에 맞추는 보정이라 매티니 스켈레탈 메시 액터에는 해당하지 않는다. 클래스 기본값(CDO)의 컴포넌트 회전은 직렬화되지 않아 확인하지 못했다.
  - [측정] 원본 카메라가 머리를 화면 어디에 두는지(`orientation_check_rev2.py`, 저작본 기준, 몸통 뼈만 사용):

| 배우 | 표본 | 머리 화면거리 전 → 후 | 화면 안 뼈(87) 평균 전 → 후 | 머리 화면 밖 전 → 후 |
|---|---|---|---|---|
| 등장 본체 | 5 | 0.46 → 0.26 | 64.2 → 68.4 | 0 → 0 |
| 등장 무채색 | 3 | 0.25 → 0.18 | 78.0 → 79.3 | 0 → 0 |
| 최후 | 12 | 0.66 → 0.38 | 70.2 → 79.4 | 4 → 1 |
| 포효 | 7 | 0.47 → 0.36 | 76.4 → 75.1 | 0 → 0 |
| 버러지 | 5 | 1.29 → 1.44 | 28.8 → 32.8 | 3 → 3 |

  최후 cut03(쓰러짐) 11595/15360ms 는 머리 화면 밖(+1.16, +1.17) → 화면 안(−0.23, +0.03), 화면 안 뼈 40·34 → 68·70. 버러지 cut01 은 원래 발탄을 거의 비추지 않는 컷이라 방향과 무관하다. 측정 원문: `out/ValtanCutsceneRaidAssembly20260918/orientation_before.txt`, `orientation_after.txt`.
- 주의: 등장 컷신 끝 자세(키 yaw −135 = 메시 225°)는 전투 시작 레이드 발탄(배치 225 + 몸체 −90 = 메시 135°)과 90° 다르게 보일 수 있다. 원본 데이터가 그렇게 되어 있다는 것까지만 확인했다.

## 5. 검증

- 격리 `/Zs`(`out/ValtanCutsceneRaidAssembly20260918/build.bat`, `_UNICODE`/`UNICODE`, 저장소 IntDir/OutDir 미사용, cl/link/MSBuild 부재 확인 후 실행): `WorldSequenceDocument`, `WorldSequenceObject`, `WorldSequencePlayer_Objects`, `WorldSequencePlayer`, `WorldSequenceToolPanel`, `MapTool_Cutscenes`, `WorldObjectTool`, `Level_KakulSaydonArena` 8개 모두 EXITCODE=0, error 0건, 경고는 기존 C4819 64건뿐. 링크·코드 생성은 하지 않았다.
- `git diff --check` EXIT 0. C++ 6파일 UTF-8 무BOM·CRLF·lone LF 0, 비ASCII 0. ps1 BOM·CRLF 유지. JSON LF 유지.
- `Publish-MapAuthoring.ps1 -AreaId LV_LUT_HEARTRB_ED -Scope WorldSequences -Mode Validate` EXIT 0 → `-Mode Publish` EXIT 0, FileCount 1. Server·Client 미실행 확인 후 게시. 발탄 maplights/mapmaterials/mapeffects(09-16), deploy(14:06) 무변경.
- Map Tool 은 World 편집 패널이 이 Area 문서를 들고 있으면 그 저작본을, 없으면 게시본을 쓴다(`MapTool_Cutscenes.cpp:190-221`). 두 쪽이 같은 rev 3 이다.

## 6. 사용자 확인 절차

1. `Client.exe` 종료 → VS `Client` `Debug|x64` **Build**(증분). 헤더 3개(`WorldSequenceDocument.h`, `WorldSequencePlayer.h`, `WorldSequenceObject.h`)가 바뀌어 관련 TU 가 여럿 재컴파일된다. 셰이더 변경 없음. Rebuild/Clean 금지.
2. 이전 빌드로 World 편집 패널에 rev 2 를 열어둔 세션에서 저장하지 않는다. 새로 실행하면 rev 3 을 읽는다.
3. Lobby → Valtan → F1 → Map Tool → 발탄 Area → Camera → 컷신 선택 → Play.
   - 5개 배우 모두 **갑옷(어깨·팔 plate)과 오른손 무기가 달린 레이드 발탄**이 나와야 정상이다.
   - 발탄 등장 0~13s: 유령/무채색이 아니라 레이드 발탄. 13s 이후도 레이드 발탄.
   - 발탄 최후: 유령이 아니라 레이드 발탄. 쓰러지는 7.8~15.4s 컷에서 머리가 화면 안에 들어와야 한다.
   - 방향: 원본 액터 yaw 그대로(이전보다 90° 돌아간 방향).
4. 안 보이면 Camera 패널 상태 문구를 전달한다. 새로 생긴 문구:
   - `World Object body fields must match presentation boss BOSS_VALTAN: …` (리소스와 카탈로그 불일치)
   - `World Object presentation boss admission failed / body is unavailable / part is unavailable: <tag> / socket bone is unavailable: b_wp_r_01`
   - 파츠 오브젝트 생성 실패는 `World Object clone/shader creation failed: …` 로 뜬다.

## 7. 확인하지 못한 것

- 실제 화면(갑옷·무기 위치, 재질, 방향). 사용자 VS 빌드와 실행 필요.
- `Prototype_GameObject_Part_Equipment` 와 `VtxMeshBinary` 셰이더가 발탄 레벨 외(예: Test 워크스페이스 DEVELOPMENT)에서도 등록되는지. 발탄 아레나 runtime attach 경로만 코드로 확인했다.
- 레이드는 몸통에 루트 모션 억제(`Enable_RootMotionSuppression`)를 켜지만 컷신 배우는 켜지 않는다(이전과 동일). 이전 조사에서 발탄 b_root 이동은 1.5m 이하였다.
- 컷신 배우는 그림자를 그리지 않는다(몸통·파츠 모두, 이전과 동일).
- 매티니 액터 클래스 CDO 의 컴포넌트 회전.
- fork B 의 생성기 `out/Handoff2_G01G03G04/g04_actor_refit.py`(`MODEL_YAW = -90.0`)와 `g04_actor_verify.py` 는 여전히 −90 규칙이다. 다시 돌리면 키가 다시 접힌다. 이번에 수정하지 않았다.

## 8. 이번 작업의 실수

1. 첫 C++ 패치 드라이런에서 검증 문구 앵커가 파서의 같은 문구와 겹쳐 2회 일치했다. 앞 줄까지 포함해 유일화한 뒤 적용했다(파일은 쓰기 전 멈춤).
2. 게시 스크립트 줄 확인에서 셸 변수 없이 `sed` 범위를 만들어 한 번 명령 오류를 냈다. 파일에는 영향 없음.
