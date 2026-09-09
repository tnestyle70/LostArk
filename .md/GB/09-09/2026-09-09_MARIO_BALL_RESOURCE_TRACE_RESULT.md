# 마리오 노란·파란 공 원본 메시/재질 추적 결과

작성일: 2026-09-09. 조사·설명 요청 결과이며 제품 설치나 툴 구현은 하지 않았다.

## 결론

노란·파란 공의 본체는 원본 skeletal mesh와 전용 material/texture가 존재한다. `전부 이펙트이므로 본체 텍스처가 없다`는 설명은 이 대상에는 맞지 않는다. 현재 Resources에 전용 파일이 없었던 것과 원본에 없는 것을 구분해야 한다.

기존 `Client/Bin/Resources/Effect/KoukuSaydon/Meshes/fx_sm_01/fm_k_ppct_ball_01.wmodel`은 좌표계·단위·중심 차이를 정규화하면 원본 공의 위치/UV/삼각형 구성이 일치하는 정적 메시다. 모델 자체의 texture 슬롯은 비어 있다. 원본은 7본 skeletal mesh이고 기존 WModel은 skeleton/animation이 없으므로 애니메이션까지 같은 자산은 아니다.

## 원본 연결

NPC 증거는 `.codex_tmp/monster_source_20260806/EFTable_Npc.db`를 read-only로 조회했다. LookInfo는 현재 설치된 `C:/ProgramData/Smilegate/Games/LOSTARK/EFGame/data4.lpk`에서 MN_PPCC_00 계열 7개만 추출했다.

| 항목 | 노란 공 | 파란 공 |
|---|---|---|
| NPC ID | 480716 / 480733 | 480717 / 480734 |
| 원본 설명 | 횡구간 서커스 노란공 | 횡구간 서커스 파란공 |
| LookInfo | EFDLChar_MN_PPCC_00-5.MN_PPCC_00-5 | EFDLChar_MN_PPCC_00-6.MN_PPCC_00-6 |
| 공통 메시 | MN_PPCC_00.Mesh.MN_PPCC_00_SK | 동일 |
| Material | MN_PPCC_00.Mat.MN_PPCC_00-2A_MI | MN_PPCC_00.Mat.MN_PPCC_00-3A_MI |
| Diffuse | mn_ppcc_00a_d2.dds | mn_ppcc_00a_d3.dds |
| Normal | mn_ppcc_00a_n1.dds | mn_ppcc_00a_n2.dds |
| Specular | mn_ppcc_00a_s1.dds | mn_ppcc_00a_s2.dds |

LookInfo의 공통 AnimSet은 `MN_PPCC_00.Ani.MN_PPCC_00_Ani`다. 노란 공은 `MN_PPCC_00-2A_MI_Dead`, 파란 공은 `MN_PPCC_00-3A_MI_Dead` 문자열도 갖는다. 이번에는 사망 재질/AnimSet payload를 추출·검증하지 않았다.

UModel v7의 logical package `MN_PPCC_00`는 물리 `9G1MUUBB1BZZE7TT4464SSP.upk`로 resolve됐다. 정확한 `mn_ppcc_00-2a_mi`, `mn_ppcc_00-3a_mi`, `mn_ppcc_00_sk` export 세 번으로 해당 재질과 메시를 조사했다. Material props의 명시적인 texture_diffuse/texture_normal/texture_specular 연결이 위 표와 일치한다. 두 Diffuse는 각각 512×1024이고 직접 열람했을 때 노란 삼각형·파란 다이아·별무늬 원형 테두리를 포함한다.

UModel은 부모 material의 일부 property/expression에 unknown 경고를 냈다. 따라서 추출 성공을 원본 shader 전체 복원 또는 게임 화면 검증 성공으로 주장하지 않는다. 이번 결론은 명시 material 참조, DDS 내용, geometry/UV 비교에 한정한다.

## 기존 메시 대조

- 원본 glTF: 290 vertices, 512 triangles, 7 joints.
- glTF 저장 위치 bounds: 약 0.9416095 × 0.9416095 × 0.9416095. skin을 실제 구동한 월드 크기 측정값은 아님.
- 기존 `fm_k_ppct_ball_01.wmodel`: 290 vertices, 512 triangles, skeleton/animation 없음. Material v2 first name `wp_mn_ppct_00_mi`, base/normal/specular/emissive 경로 모두 비어 있음.
- 비교: WModel position을 100으로 나누고 두 메시의 AABB 중심을 제거, glTF Z 부호 반전. UV는 그대로 사용.
- 위치+UV nearest 오차 최대 약 5.08e-8. 소수 5자리의 위치+UV vertex key로 비교한 triangle multiset 일치. 이 검사는 face winding·bone weight·animation 동등성을 의미하지 않는다.
- 원본 glTF 중심 Y는 약 0.4693361, 기존 정적 메시 중심은 약 0이다. 기존 메시를 배치할 때 중심/바닥 anchor 차이를 반영해야 한다.

일반 후보도 실측했다. `fm_b_sphere_001`, `fm_d_sphere_001`, `fm_h_candysphere_01_3`, `fm_m_sphere_004`는 구형 후보지만 정확한 공 메시가 확인됐으므로 이것들로 대체할 이유가 없다. `fm_m_sphere_002/003`은 한 축 두께 약 2.89라 파일 이름만 보고 완전한 구라고 판단하면 안 된다.

## 적용 방향

정지한 공을 배치하는 목표에는 기존 정적 메시와 이번에 확인한 **노란/파란 재질 변형 두 개**를 연결하는 것이 우선이다. World Object의 diffuse override만 사용하면 기본 외형 확인은 가능하지만 원본 normal/specular까지 연결한 완료 상태는 아니므로, 최종은 기존 CModel/CMaterial에 명시적인 재질 슬롯을 넣은 두 variant로 준비한다. `_s`를 표준 ORM으로 추측하지 않는다.

원본 변형/clip이 필요하면 새로 찾은 skeletal 원본과 AnimSet을 기존 ActorX→WModel 경로로 조리해 Deploy ANIM/기존 animation track으로 연결한다. 정적 mesh에 리그가 있다고 취급하지 않는다.

MapTool에서는 Yellow Target/Blue Target 두 정의를 선택해 배치한다. 두 정의는 모양을 재사용하고 색·문양 재질만 구분한다. 상시 표시·서버 공격 판정·파괴·재입장은 앞선 배치 설계의 기능 연결이 별도로 필요하다. 이번에 파일을 찾았다는 이유로 이미 맵툴 palette에 설치된 것은 아니다.

추가 빛/반짝임/피격/폭발이 필요하면 기존 V2 Group을 객체에 붙인다. 본체 문양과 테두리를 이펙트로 다시 그릴 필요는 없다. 해당 추가 효과가 원본에서 무엇인지 이번 조사에서 확정하지 않았다.

만약 다른 자산에 정말 texture가 없다면 현재 `Shader_EffectV2_Common.hlsli:260`은 Base 미지정 시 white를 사용하고 ColorMul/ColorOffset 및 mask/rim을 적용할 수 있다. 기존 V2 MESH leaf에 geometry를 지정해 색을 주고 별도의 TEXTURE leaf로 문양, ring/particle child로 테두리·빛을 조합하는 프로젝트 제작 방식은 가능하다. 그러나 이번 두 공에는 전용 원본 texture가 발견됐으므로 이 우회가 기본안은 아니다.

## 조사 산출물과 미실행 항목

저장소 루트 기준 `out/MarioBallSource/`에 다음을 보존했다. 모두 Git 제외 조사 파일이며 Resources에 설치하지 않았다.

- `source_evidence.json`: NPC 행, LookInfo 참조, geometry/UV 대조 수치.
- `yellow_blue_textures.png`: DDS 두 장의 오프라인 열람 이미지. Client 캡처 아님.
- `mesh/MN_PPCC_00/SkeletalMesh3/mn_ppcc_00_sk.gltf`와 `.bin`.
- `yellow/MN_PPCC_00/MaterialInstanceConstant/` 및 `Texture2D/`.
- `blue/MN_PPCC_00/MaterialInstanceConstant/` 및 `Texture2D/`.
- `EFGame_Extra/ClientData/XmlData/LookInfo/Monster/`: 추적한 LookInfo 7개.

기존 Python LPK reader의 Crypto 패키지가 bundled Python에 없어, 조사 전용 `out/mario_lpk_probe.py`에서 설치된 cryptography backend를 연결해 원래 reader를 실행했다. 원래 tool 코드는 바꾸지 않았다.

게임 설치 파일은 읽기 전용이었다. 기존 사용자 변경 보존, 제품 코드/JSON/Resources 변경 없음. 신규 쿠킹·MapTool 등록·publisher·빌드·Client 실행·화면 PASS·Drive 업로드는 하지 않았다.
