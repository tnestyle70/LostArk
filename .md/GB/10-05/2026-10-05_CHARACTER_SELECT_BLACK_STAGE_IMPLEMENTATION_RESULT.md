# 클래스 선택의 원본 검은 바닥 무대 구현 결과

## G00. 반영 범위

선택 화면의 현재 기본 캐릭터 발밑에 원본 검은 팔각 바닥을 표시하도록 연결했다. SL00 Fighter floor1053/star1054를 원본 위치에 임시 Clone하고, 승인된 class의 기본 CCharacter를 그 위에 둔다. 원본 바닥은 중앙336과 수평947.885133m 떨어져 있다. 현재 사용자가 중앙102m 옆으로 옮긴 저작·게시 배치는 변경하지 않았다.

표시용 캐릭터는 Server entity·command sink·replication registry에 등록하지 않는다. 체험하기는 무대·표시 모델을 숨기고 기존 아레나의 서버 캐릭터와 follow camera를 복구한다. 최초 체험에서는 기존 중앙 spawn, 체험 후 재진입에서는 Server가 유지한 실제 위치를 사용한다. 서버 순간이동이나 새 전투 entity는 추가하지 않았다. 소개 무비의 배우/의상 대신 기본 class spec의 body·head·equipment·weapon과 idle을 사용한다.

## G01. 구현된 연결

- `CharacterSelectShowcase.h/.cpp`: 16KiB bounded JSON parse, schema/version/Area·source ID·runtime ID 중복·finite·unit quaternion·positive scale 검증, 바닥 두 개와 캐릭터 숨김 후보 생성, 실패 시 RAII 정리. 현재 class는 재사용하며 교체 실패 전에 이전 객체를 파기하지 않는다. 같은 실패는 approved 캐릭터 weak identity/class별로 기록한다.
- `CharacterSelectShowcase.json`: 원본 바닥·별 TRS, 발 기준점 `[74.8114209,0.075660566,-228.334375]`, yaw180°, 중앙 조명 기준점 `[-772,-142.55,197]`. 실제 별 윗면 최대Y0.074660566m보다1mm 높게 둔다. 현재 floor diffuseBrightness0/star1과 loaded record의 RNM/sourceWind, catalog 재질을 보존한다.
- `Level_CharacterSelect`: PREVIEW의 전신 카메라가 표시 모델을 보며 실제 replica의 이전 suppression은 weak pointer와 함께 보관·복원한다. 새 clone은 다음 정상 ObjectUpdate까지 숨겨 part world/pose가 원점에서 보이지 않게 한다. 캐릭터 world yaw와 hair/cloth chain의 presentation yaw도 함께 맞춘다. TRIAL·생성·무비·modal·disconnect·Level 종료에서 무대를 회수한다. 아레나 숨김은 runtime visibility를 덮지 않는 stage suppression을 사용한다.
- `MapLightPresentationRuntime`: 선택 화면의 POINT/SPOT 위치에만 프레임별 offset을 적용한다. frustum 검사와 실제 제출이 같은 위치를 쓰며 기본 호출은0으로 복귀한다. 방향·색·강도·range·receiver·static shadow channel·scene multiplier는 유지한다.
- `MainApp`: 기존 dark profile을 사용하면서 shadow eye/at만 평행이동한다. 마지막 적용 focus가 여전히 자기 값일 때만 이전 offset을 제거한다. profile/tool의 새 focus를 이전 값으로 덮지 않고 shadow OFF를 켜지 않는다. rendering option·조명 JSON을 수정하지 않았다.
- Client project/filter에 새 H/CPP 및96.DataFiles의 JSON을 각각 한 번 등록했다. 기존 파일의 UTF-8/CRLF를 보존했다.

## G02. 실행한 검증

| 검증 | 실제 결과 |
|---|---|
| 설치 WModel 기하·원본/현재 배치 대조 | PASS. 바닥 폭6.75763m, 별 윗면 최대Y0.074660566m. `out/ClassSelectFighterStage20261005/geometry-receipt.json` 및 `audit_geometry.py` |
| JSON 파싱, 원본 TRS 일치, 임시 ID 미사용, 기존 배치/재질 hash 보존 | PASS |
| 프로젝트/filters XML 파싱, 신규3항목 중복 없음 | PASS |
| POINT/SPOT4개 상대위치·shadow 방향·복귀 float32 수치 | PASS. 최대오차0.000022888m, shadow 복귀0m. `out/CharacterSelectBlackStage20261005/data-contract-verification.json` |
| 독립 코드 검토 | 첫 정상 part Update·화면 전환·실패 정리·숨김 복원·빛 위치 복귀 확인. 발견한 cloth/hair yaw 불일치를 수정한 뒤 추가 차단 finding 없음 |
| 첫 Debug Product 전체 빌드 | PASS,155.833초. `out/BuildPipeline/runs/20261004T221139133Z-debug-product.json` |
| yaw 보완 후 첫 재빌드 | 외부 동시 엔진 작업의 미생성 `OcclusionCuller.h` 때문에 Engine C1083 실패. `out/BuildPipeline/runs/20261004T221254290Z-debug-product.json`. 해당 작업을 되돌리지 않았음 |
| 최종 Debug Product | PASS,164.139초. `out/BuildPipeline/runs/20261004T221853589Z-debug-product.json`. Engine/Shared/Server/Client 컴파일·링크·배포 및 runtime presence/catalog 검사 |
| 최종 Release Product | PASS,161.429초. `out/BuildPipeline/runs/20261004T222156739Z-release-product.json`. Engine/Shared/Server/Client 컴파일·링크·배포 및 runtime presence/catalog 검사 |
| git diff --check | 현재 변경 전체 오류 없음. 기존 LF→CRLF 안내만 있음 |

yaw 보완 뒤 최종 Debug/Release4개 제품 프로젝트가 모두 통과했다. 두 빌드 완료 뒤 이번 구현 파일11개의 hash가 동일하고 기존 placement/catalog/material/조명/profile 문서가 보존됐음을 확인했다. 제품 빌드와 CPU 수치 검증은 실제 GPU 화면 판정이 아니다. 빌드 로그의 기존 인코딩·외부 PDB·shader 경고는 warning0으로 설명하지 않는다.

## G03. 남은 화면 확인

저장소 규칙에 따라 Client/UI를 실행하거나 조작하지 않았다. 사용자가 선택 화면→직업 변경→체험→선택 복귀→캐릭터 생성/소개 무비 순서로 바닥 표시·발 접지·조명·숨김 잔존 여부를 확인해야 한다. 기본 class별 idle의 실제 신발 접지와 첫 준비1프레임의 시각적 전환은 아직 화면 검증하지 않았다. 원작이 동일 actor를 이동했는지 복제 actor를 썼는지는 확보한 원본 stage/camera 근거만으로 확정하지 않는다.

## G04. 워로드 선택창 크기 읽기 검토

2026-10-05 사용자 요청에 따라 수정 없이 비교했다. 무대 구현 입력11개 hash는 위 최종
Debug/Release 성공 때와 모두 동일하다. 무대 배치는 캐릭터 위치·yaw만 바꾸며 추가 scale을
주지 않는다. 실제 Client/UI 화면은 실행하지 않았다.

PREVIEW 카메라는 여섯 직업 모두 수평거리4.6m, eye 높이1.05m, look 높이0.95m,
수직FOV30도다. body/weapon bounds를 읽는 자동 맞춤이나 워로드 전용 카메라 축소는 없다.
워로드는 기본 WARLORD_NORMAL과 `wgl_idle_battle_1`을 사용한다. 방어 자세나 원본 무비의
classselect_loop가 아니다. 별도 워로드 축소 배율도 없다.

설치 기본 몸체 WModel에 실제 rig common basis·preScale·현재 presentation scale을 적용한
기본 geometry 높이는 워로드1.312375, 슬레이어1.295664, 창술사1.281268, 건슬링어1.276494,
차원술사1.164888, 도화가1.078114다. 엔진 공간의 몸체 범위이며 원작 설정상의 신장이 아니다.
워로드의 원래 몸체는 비교한 여섯 직업 중 가장 높다. stored WSKL rest pose는 이미 굽은
자세일 수 있으므로 canonical geometry와 별도로 계산했다.

| 직업 | 일반 전투 idle 몸체 높이 | 공통 카메라에서 몸체의 viewport 높이 |
|---|---:|---:|
| 워로드 |1.055~1.120|46.9~49.6%|
| 창술사 |1.184~1.205|51.1~51.8%|
| 건슬링어 |1.249~1.252|52.7~52.9%|
| 슬레이어 |1.158~1.175|50.4~51.1%|
| 차원술사 |1.154~1.155|48.6~48.7%|
| 도화가 |1.025~1.044|42.3~43.1%|

실제 idle의0/25/50/75% 네 시점을 CPU skin으로 계산했다. 워로드는 기본 몸체 대비 높이가
80.4~85.3%로 낮아지는 반면 창술사는92.4~94.0%다. 현재 작게 보이는 주요 원인은 기본 체형
축소가 아니라 전투 대기 자세의 높이 감소와 직업별 차이를 보정하지 않는 공통 구도다.
큰 무기로 인해 자동으로 카메라가 멀어지는 경로는 없다.

몸체에는 얼굴과 발까지의 기본 인체가 있지만 실제 화면에서 갑옷 아래 숨기는 submesh도
포함한다. 별도 갑옷·투구·머리·무기·GPU cloth를 합친 최종 착장 실루엣이나 실제 픽셀 측정은
아니다. 첨부 원작 동영상의 배우·전용 자세·카메라를 현재 기본 PREVIEW로 간주하지 않는다.
현재 ClassSelection.cinematics의 여러 카메라도 PROJECT_TUNED이므로 원작 고유 수치로
설명하지 않는다.

근거는 `out/WarlordShowcaseReview20261005/comparison-summary.json`,
`out/CharacterSelectSizeAudit20261005/model-size-receipt.json`,
`out/CharacterSelectFramingReview20261005/body-framing-receipt.json`이다. 독립 두 계산의
동일 idle 높이는1e-6 이내로 일치하고 실제 body 입력 hash도 재확인했다.
`Level_CharacterSelect.cpp:91~94,408~411`, `CharacterSelectShowcase.cpp:267~278`,
`Logic_Warlord.cpp:48~59,197`, `Character.cpp:2281~2289`가 연결 근거다.

향후 선택창에서 크기를 맞춘다면 전투 월드의 모델 배율을 변경하기 전에 PREVIEW 전용
자세와 직업별 카메라 거리·시선 높이를 조정하는 것이 범위에 맞다. 몸체 기준 구도와 긴 무기·
방패의 잘림을 함께 확인해야 한다. 이번 검토에서는 제품 코드·저작 설정·모델을 변경하지 않았고
빌드를 반복하지 않았다.

## G05. 승인 후 워로드 PREVIEW 구도 반영

사용자 승인 후 `Level_CharacterSelect.cpp`의 실제 표시 캐릭터가 WARLORD일 때만
카메라 거리를4.6m에서4.4m로 줄였다. eye 높이는1.004347826m, look 높이는0.908695652m로
같은 비율을 적용한다. FOVY30도와 캐릭터 원점 anchor·pitch를 유지한다. 다른 직업은 원래
수치를 그대로 사용하고 TRIAL·무비·커스터마이징에는 기존 PREVIEW guard가 적용을 막는다.
기본 모델·전투 idle·게임 내 크기·팔각 바닥·조명·shadow focus·저작 JSON은 변경하지 않았다.

실제 body idle 네표본에서 화면 높이는46.87~49.59%에서49.14~51.96%로 약4.8% 커진다.
창술사의 기존51.05~51.85%에 가까워진다. 원점의 화면 위치는 같지만 실제 발 정점은 깊이
차이로 아래0.21%p 이동하므로 모든 신발 픽셀이 고정됐다고 설명하지 않는다.

실제 표시되는 body face/eye, 갑옷·투구, 건랜스·방패를 합친33개 idle 표본도 계산했다.
16:9에서 화면 밖 정점0, 왼쪽 최소여유2.75%, 아래 최소여유6.98%다. 앞선4.3m 후보는
건랜스 왼쪽 여유가0.98%라 최종4.4m를 선택했다. 자기 골격 장비는 이름으로 body pose를
연결한 own inverse-bind palette를 사용하고 무기는 socket combined를 한 번만 적용했다.
4:3에서는 기존 카메라에서도 긴 건랜스가 잘린다. 이번 검증을 모든 화면비의 자동 맞춤으로
확대하지 않는다. UI 겹침·cloth/plume·표본 사이 극점과 실제 사용자 화면은 미검증이다.

| 최종 검증 | 결과 |
|---|---|
| 독립 CPP diff 검토 | PASS. 워로드 상수와카메라 계산 두 hunk만 변경, 다른 class 수치 동일 |
| 기존 파일 인코딩 | UTF-8 무BOM·CRLF 보존 |
| 실제 body·장비 CPU 투영 | 위 수치 확인. GPU 화면 검증 아님 |
| Debug Product | PASS, 14.995초. `out\BuildPipeline\runs\20261004T230726789Z-debug-product.json` |
| Release Product | PASS, 178.915초. `out\BuildPipeline\runs\20261004T231049631Z-release-product.json` |
| 최종 입력 hash | 수정 CPP와보호한 기존 무대·카메라·profile·캐릭터 입력 모두 빌드 중 변경 없음 |

두 구성에서 Engine/Shared/Server/Client 제품 빌드·배포와 runtime presence/catalog 검사를
통과했다. 기존 인코딩·shader/외부 PDB 경고는 남아 있으며 무경고 빌드로 표시하지 않는다.
실행 증거는 `out/WarlordShowcaseReview20261005/final-verification.json`,
`out/CharacterSelectFramingReview20261005/warlord-camera-final-4.4-receipt.json`,
`out/CharacterSelectSizeAudit20261005/warlord-visible-framing-receipt.json`에 보관했다.
Client/UI는 자율 실행하지 않았다. 선택창의 워로드 크기와 체험 복귀 화면은 사용자가 확인한다.

## G06. 팔각별 검정 처리와 바닥 반사 보정

사용자 후속 화면에서 밝게 보인 별은 export1054/runtime9000102이며, 설치 재질의
밝기1·반사0.5·PBR specular0.5가 그대로 복제되고 있었다. 큰 바닥1053/9000101은
밝기0이지만 lightbox_cube3.dds 반사1.5와 PBR specular0.5가 남았다. 두 단일 mesh의
실제 재질 family는 bg_base_pbr_opa이고 발광은 없다.

Shader_MapMaterialSurface.hlsli의 PBR 식은 diffuseBrightness 적용 뒤 반사 texture를
기본색에 더한다. 따라서 밝기0·흰 반사 texel·normal alpha1·반사1.5에서는 최종 albedo가
흰색1까지 올라갈 수 있다. 이는 SSR와 별개인 재질 내2D 반사이며 현재 화면 뒤쪽의 밝은
무늬를 만들 수 있는 확인된 경로다. 해당 screenshot 픽셀의 기여를 GPU로 분리하지 않았으므로
반사 외 추가 기여까지 모두 없었다고 단정하지 않는다.

CharacterSelectShowcase.cpp에서 임시 두 배치에 사용할 loaded materialOverrides를
복사하고 diffuseBrightness/reflectionIntensity/specularPBRIntensity만0으로 변경했다.
기존 Create_Placement→CMapAssetObject→CModel::Create_MaterialVariant로 전달한다.
geometry를 재사용하고 clone의 CMaterial만 분리한다. 캐릭터·광원·그림자 설정·원래 배치와
저작 재질은 변경하지 않았다. 이는 G01의 초기 floor0/star1 복제 이후 사용자 요청에 따른
표시 무대 전용 보정이다. RNM 및 native indirect/direct PBR는 albedo/F0가0이므로
해당 재질 기여도0이다. Source Materials OFF 진단의 legacy fallback이나 별도 실험 SSR를
이 보정의 검증 범위로 확대하지 않는다.

| 검증 | 결과 |
|---|---|
| 실제 설치 placement/catalog/material 연결 | PASS. stable ID·path·source hash·family·세 입력과 발광 부재 기록 |
| 수정 CPP Debug/Release 독립 OBJ 컴파일 | 두 구성 PASS. 기존 header C4828 경고는 각각266건 남음 |
| 소스 인코딩과 보호 입력 | UTF-8 무BOM/LF 유지. 저작·게시·Rendering 입력21개 hash 동일 |
| JSON parse 및 diff check | PASS |
| 독립 코드 검토 | PASS. 공유 재질 보존·variant 수명·생성 실패 rollback 확인 |
| Debug Product | PASS,16.963초. 20261005T010337903Z-debug-product.json |
| Release Product | PASS,43.408초. 20261005T010449761Z-release-product.json |
| GPU 화면 | Client/UI를 실행하지 않아 사용자 확인이 남음 |

실행 증거는 out/CharacterSelectBlackStage20261005/black-material-followup의
material-evidence.json, verification.json, source-change.diff와 구성별 컴파일 로그다.
조사 당시 Release Client(PID38392)와 Server(PID8368)가 실행 중이어서 먼저 독립 컴파일했다.
이후 두 프로세스의 종료를 확인해 Debug/Release 제품 빌드·링크·배포까지 완료했다. 에이전트가
프로세스를 종료하지 않았다. 두 빌드 동안 수정 CPP hash와 보호 입력21개가 유지됐다.
제품 receipt는 out/BuildPipeline/runs에 있으며 기존 인코딩·외부PDB 경고를 무경고로
설명하지 않는다. 다음 Client 실행에서 변경을 소비한다. Client/UI를 자율 실행하지 않았고
최종 검정 별·바닥 화면은 사용자 확인이 남는다.
