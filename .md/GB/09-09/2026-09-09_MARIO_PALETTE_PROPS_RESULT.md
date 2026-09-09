# 마리오 공·폭탄 Palette 등록 결과

작성일: 2026-09-09. 정적 오브젝트 배치용 데이터/리소스 등록. 사용자 화면 확인 전.

## 적용한 기능

KoukuSaydon Area의 기존 Map Assets Palette에 `Mario Props` 그룹을 추가했다.

| Palette 표시 이름 | stable asset ID | Resources 상대 모델 경로 |
|---|---|---|
| Mario Yellow Ball (Triangle) | MAP_MARIO_YELLOW_BALL | Map/LV_LUT_MIDNIGHTC_ED/MarioProps/YellowBall/YellowBall.wmodel |
| Mario Blue Ball (Diamond) | MAP_MARIO_BLUE_BALL | Map/LV_LUT_MIDNIGHTC_ED/MarioProps/BlueBall/BlueBall.wmodel |
| Mario Skull Bomb | MAP_MARIO_SKULL_BOMB | Map/LV_LUT_MIDNIGHTC_ED/MarioProps/Bomb/Bomb.wmodel |

영문 표시 이름과 한국어 근거/설명을 함께 검색할 수 있다. 검색어 `Mario`, `노란`, `파란`, `폭탄`을 지원하는 기존 label/group/evidence 검색 경로를 사용한다. BottomCenter anchor로 클릭한 바닥에 모델 최하단이 놓인다. 배치 후 Inspector의 Transform을 기존대로 조정한다. 기본 공 지름은 약 0.942m, 폭탄 크기는 약 0.671 × 0.872 × 0.671m다. 게임 원본 actor DrawScale까지 복원한 크기라는 뜻은 아니며 사용자가 scale을 조정할 수 있다.

기존 catalog 323개 행은 유지하고 3개만 추가했다. MapCatalog.json assetCount도 326으로 맞췄다. 사용자의 기존 3,231개 배치를 변경하거나 임의의 위치에 새 인스턴스를 생성하지 않았다. 새 오브젝트 배치·Save·Reload는 기존 MapTool/CMapAssetObject 경로를 사용한다.

## 원본과 리소스 연결

- 공: `MN_PPCC_00_SK` 원본 glTF를 정적 쿠킹했다. 노란 공은 `MN_PPCC_00-2A_MI`의 d2/n1/s1, 파란 공은 `MN_PPCC_00-3A_MI`의 d3/n2/s2를 연결했다. glTF 실제 material key는 두 색 모두 `mn_ppcc_00_mi`다.
- 폭탄: 원본 NPC 480725/LookInfo `EFDLChar_MN_RHCN_01.MN_RHCN_01`의 `MN_RHCN_01.Mesh.MN_RHCN_01_SK` 및 `MN_RHCN_01.Mat.MN_RHCN_01_MI`를 확인했다. UModel이 logical `MN_RHCN_01`을 물리 `9G1M8ABG1BZ6E7JX4364S5P.upk`로 해석했다. `.props.txt`의 `texture_diffuse`, `texture_normal`, `texture_specular`가 `mn_rhcn_01_d_loc_int/n/s`를 지정한다.
- 기존 ModelAssetConverter의 `--pretransform --no-auto-textures --scale 100` 및 명시적인 `--material-remap`, `--normal-remap`, `--specular-remap`을 사용했다. 각 결과는 skeleton 없음/animation 0의 NONANIM WModel이다.
- WModel은 모델 옆 `textures/<이름>.dds`를 참조한다. 절대 out 경로나 원본 게임 설치 경로를 저장하지 않았다. specular를 ORM으로 바꾸지 않았다.
- 기존 Effect 경로의 공, Character 경로의 폭탄은 수정하지 않았다. UModel/glTF/props/조사 로그는 `out/MarioBallSource`, `out/MarioBombSource`, `out/MarioPaletteCook`에만 보존한다.

## 검증 결과

- converter 3개 쿠킹 성공 및 WModel 구조 해석 성공. 공은 각각 290 vertices / 512 triangles, 폭탄은 622 vertices / 996 triangles다.
- 원본 glTF와 정적 결과의 위치/UV/삼각형 대조 성공. cm 좌표 위치+UV 오차는 최대 약 3.89e-6 이하. 모든 삼각형의 winding/normal 부호 일치. 이는 사용자 화면 PASS가 아니다.
- 실제 사용하는 material 슬롯의 diffuse/normal/specular 9개 참조가 정확히 해당 DDS를 가리키며, 설치본도 원본 bytes와 일치한다. DDS 9개 디코딩 성공. diffuse alpha는 모두 255로 기존 Opaque 경로를 사용한다.
- catalog header 326, 중복 asset/prototype ID 없음, 신규 행 26-token 형식 및 3개 모델 실물 존재 확인. MapCatalog JSON parse 및 기존 Client vcxproj/filters XML parse 성공. 두 기존 Data 파일은 이미 프로젝트에 등록되어 있다.
- 최소 컴파일: C++/HLSL 변경이 없어 제품 재컴파일 대상 없음. 기존 converter를 실행해 런타임 WModel을 만들었다. 새 Client 빌드 성공을 주장하지 않는다.
- 기존 `Publish-MapAuthoring.ps1 -AreaId LV_LUT_MIDNIGHTC_ED -Mode Validate`, `Publish`, `Check` 모두 exit 0. 각 단계는 Area/single, 3,231 placements, 8개 출력 파일을 확인했다. 실행 전 기존 source/runtime 쌍은 의미상 동일했고 byte 차이는 형식 차이였다. 실행 shell의 PATH에 bundled Python을 추가해 light v2 검증기를 사용했다.
- runtime v5 catalog의 326개 행이 source v4 catalog의 행과 완전히 일치한다. 기존 배치 문서는 그대로이며 SHA-256 `71f6fec72ee2270354504ca5d40e1a0c02be235fcaa7347ab91735ccc48c2540`이다. Camera/Light/Material/WorldSequence JSON은 publisher 줄바꿈 정규화가 있었지만 데이터 내용은 변경하지 않았다.
- Git LFS clean filter는 기본 sandbox에서 .git/lfs 쓰기가 거부되어 승인된 git diff 검사로 재실행했다. `git diff --check` 성공. 기존 사용자 변경은 stage/commit/push하지 않았다.
- 독립 읽기 전용 검토로 NONANIM 타입, BottomCenter, 0.01 pretransform, model-relative material path 및 culling 경로를 확인했다.

## 사용자 확인 경로

1. 실행 중 MapTool에 미저장 변경이 있다면 먼저 저장한다. 새 리소스/카탈로그 확인은 Client를 다시 실행한 세션에서 한다.
2. Lobby `Test` -> F1 `Open Map Tool` -> Area `KoukuSaydon / MidnightC ED` -> `Map Assets`.
3. Palette 검색창에 `Mario` 입력. `Mario Props` 그룹의 세 항목 중 하나를 클릭해 미리보기를 확인한다.
4. 항목 더블클릭 또는 `Arm placement` -> 원하는 바닥 클릭 -> Hierarchy에서 선택 -> Transform 조정 -> `Save`.
5. `Reload` 및 새 세션에서 배치가 유지되는지 사용자가 확인한다. 제품 아레나에 새 배치를 적용하려면 저장 뒤 해당 Area map publisher를 실행하고 재진입한다.

Client/UI 실행·화면 캡처·배치 입력·Save/Reload 실조작은 에이전트가 수행하지 않았다. 실제 색감, 크기, 문양 방향, 미리보기와 배치 화면 확인은 사용자가 해야 한다.

## Drive 전달

물리 폴더: `Client/Bin/Resources/Map/LV_LUT_MIDNIGHTC_ED/MarioProps/`.
총 12파일, 3,770,540 bytes. WModel 3개와 DDS 9개다. 이 폴더 전체를 다른 PC의 동일 Resources 상대 위치에 전달하면 된다. Git 제외 실물 준비 완료, Drive 업로드는 하지 않았다. Git에 Resources binary를 force-add하지 않는다.

## 범위 밖

공을 때려 파괴하기, 폭탄 폭발/데미지, 심지 불꽃 이펙트, 리그 애니메이션, 몬스터 AI, 서버 스폰 및 상호작용은 이번 등록에 포함하지 않았다. 현재는 맵에 계속 표시되는 정적 소품이다. 원본 부모 shader 전체 일치나 실제 게임플레이 완성을 의미하지 않는다.

# G02. 추가 네 모델과 트리거용 표시 모션

아래는 후속 요청 반영 상태다. 앞의 G01 수치는 그 시점의 기록이며 현재 최종 catalog는 330개, MarioProps는 7모델이다.

## 등록 항목

| Palette 이름 | 한국어 검색어 | ID suffix / 물리 폴더 |
|---|---|---|
| Mario Red Ball (Star) | 빨간 별 공 | red_star_ball / RedStarBall |
| Mario Striped Circus Ball | 줄무늬 서커스 공 | striped_ball / StripedBall |
| Mario Clown Face Ball | 광대 얼굴 공 | clown_face_ball / ClownFaceBall |
| Mario Horn Clown (Static) | 나팔 광대 | horn_clown / HornClown |

위 네 폴더는 모두 `Client/Bin/Resources/Map/LV_LUT_MIDNIGHTC_ED/MarioProps/` 아래이며 폴더와 같은 이름의 WModel을 사용한다. Palette stable asset ID는 각각 `MAP_MARIO_RED_STAR_BALL`, `MAP_MARIO_STRIPED_BALL`, `MAP_MARIO_CLOWN_FACE_BALL`, `MAP_MARIO_HORN_CLOWN`이다. 기존 노란 공/파란 공/폭탄도 그대로 남아 있다.

## 원본 연결

- 빨간 공은 MN_PPCC_00-4 LookInfo -> MN_PPCC_00_SK + MN_PPCC_00-1A_MI, 줄무늬 공은 MN_PPCC_00 -> 같은 메시 + MN_PPCC_00_MI다. d1/n/s 및 c/n/s를 각각 연결했다. 두 diffuse에서 별/줄무늬 원본 문양을 확인했다.
- 얼굴 공은 WP_MN_RHCN_00의 StaticMesh fm_d_rhcn_00 + MN_RHCN_00_MI다. d/n/s를 연결했고 별도 diffuse에서 광대 얼굴 문양을 확인했다. 기존 Effect 경로의 모델은 변경하지 않았다.
- 광대는 NPC 480701 나팔부는 서커스 광대의 MN_REUP_04_SK 몸체와 LookInfo 부품 WP_MN_REUP_01_SK를 사용한다. 몸체 d/n/s, 나팔 variant WP_MN_REUP_01-1_MI의 d 및 공통 n을 연결했다. UModel -dump의 wp_01 socket은 bip001-prop1/위치 0/Pitch 16384/Scale 1이며 glTF bind bone과 socket transform으로 나팔을 결합했다. 임의 손 위치를 찍어서 붙이지 않았다.
- 광대는 뼈대의 기본 자세를 정적으로 굳힌 모델이다. 사진의 나팔 부는 애니메이션 프레임이나 불꽃을 재현한 것은 아니다. 몸체와 부품의 최종 육안 연결은 사용자 확인이 남아 있다.
- 새 모델 네 개 모두 NONANIM이며 CModel/CMaterial 기존 경로다. 불꽃·emissive 연출·공격/접촉/AI는 추가하지 않았다.

## 배치와 트리거 사용

1. Client를 새로 실행해 Lobby Test -> F1 -> Open Map Tool -> KoukuSaydon -> Map Assets -> Palette에서 `Mario`를 검색한다. 네 항목을 더블클릭하거나 Arm placement로 바닥에 배치한다. Save 후 제품맵에 반영하려면 Map publish가 필요하다.
2. 트리거로 나타내려면 F1 `World Object Tool` -> Map 그룹에서 같은 이름의 소품 -> `Show ... / 표시 유지` 모션을 선택한다.
3. `Map Position`을 원하는 소환 위치로 수정하고 미리보기/Save한다. 기본 좌표는 [0,0,0]이며 마리오 특정 위치에 자동 배치하지 않았다.
4. MapTool World Gameplay에서 원하는 TriggerBox를 만들거나 선택하고 `Play Sequence Action`으로 해당 `world.object.instance.mario.<suffix>.show`를 지정한다.
5. Trigger 저장 뒤 Map publisher와 WorldGameplay publisher로 해당 Area를 내보내고 Server 재시작/Client 재진입한다. 이번에는 실제 TriggerBox를 임의 생성하거나 기존 트리거를 바꾸지 않았다.

소환용 stable object는 `world.object.mario.<suffix>`, template은 `sequence.mario.<suffix>.show`, instance는 `world.object.instance.mario.<suffix>.show`다. 1000ms 동안 동일 자세를 표시한 뒤 HOLD로 유지한다. 명시적 Play 전에는 나타나지 않는다. Stop/Level 종료로 정리된다. 정적 Palette 배치와 show 모션을 같은 자리에 둘 다 사용하면 중복 외형이므로 용도에 맞게 하나를 선택한다.

기본 scale=1/회전 없음에서 모델 bounds를 이용해 바닥 기준을 맞췄다. World Object Tool에서 크기나 회전을 바꾸면 Position Offset을 함께 조정해야 한다. 이 소환은 서버 trigger가 지시하는 Client presentation이지 체력/AI가 있는 서버 몬스터 스폰은 아니다. 늦은 입장 플레이어에게 영구 스폰 상태를 재전송하는 기능도 추가하지 않았다.

## 검증 및 남은 확인

- WModel 네 개 생성 성공. 빨간/줄무늬 공 각각 290정점/512삼각형, 얼굴 공 716정점/1320삼각형, 광대+나팔 7731정점/11055삼각형(2 submesh)이다.
- 각 원본/staging glTF 대비 위치·UV 오차 최대 0.00001028cm 이하, 삼각형 연결 일치. 광대 submesh 인덱스는 각 vertex base를 반영해 대조했다. 조립 전 무기의 원본 자세와 최종 화면 일치를 대신 증명하는 검사는 아니다.
- 실제 사용 재질 슬롯에 diffuse/normal/specular 경로가 정확히 연결되어 있고 참조 DDS 14개가 source/cooked/설치본과 바이트 일치하며 디코딩된다. 나팔은 source에 명시된 diffuse/normal만 사용한다.
- 기존 326 catalog 행과 기존 sequence 11 resources/95 templates/131 instances 모두 내용 불변. 신규 등록 후 330 assets, 15 resources/99 templates/135 instances, revision 413이다. 기존 placement 3231개와 SHA-256 71f6fec72ee2270354504ca5d40e1a0c02be235fcaa7347ab91735ccc48c2540 유지.
- Map publisher Validate/Publish/Check 모두 성공, 8개 runtime 출력 검사. source/runtime WorldSequence JSON 완전 동등, MapCatalog JSON 및 Client/Default 프로젝트 XML parse 성공. 새 C++/HLSL/프로젝트 파일 변경이 없어 컴파일 대상은 없으며 Client 빌드 성공으로 기록하지 않는다.
- publisher를 별도 PowerShell child에서 실행할 때 Python 탐지가 실패했다. bundled Python을 PATH에 추가한 동일 PowerShell에서 직접 기존 publisher를 호출해 성공했다. publisher 코드는 바꾸지 않았다.
- 큰 WorldSequence 파일을 tool apply_patch가 읽을 때 base64 오류가 있어 기존 apply_patch CLI로 같은 국소 패치를 적용했다. 문서 전체를 JSON 재직렬화하지 않았다.
- 독립 읽기 전용 검토로 기존 Play Sequence trigger -> Server broadcast -> Client WORLD/HOLD 경로를 확인했다. git diff --check 성공. Client/UI를 실행·조작하거나 화면 PASS를 판정하지 않았다.
- 사용자 남은 확인: 네 미리보기와 나팔 연결/자세, 실제 바닥 배치 및 Save/Reload, 원하는 위치/트리거 지정 후 아레나 재생. 기존 팀 LAN 접속 주소는 그대로이며 Server가 실행되어 있어야 제품 아레나에 들어갈 수 있다.

## Drive 전달 갱신

MarioProps 전체는 현재 30파일 / 9,884,450 bytes이다. 이번 네 소품 추가분은 18파일 / 6,113,910 bytes(WModel 4 + DDS 14). 위 네 폴더만 추가 배포하거나 MarioProps 전체를 같은 Resources 상대 위치로 전달한다. 물리 파일 준비 완료, Drive 업로드는 하지 않았다. 리소스는 Git 제외이며 다른 사람의 코드/미커밋 파일을 stage/commit/push하지 않았다.
