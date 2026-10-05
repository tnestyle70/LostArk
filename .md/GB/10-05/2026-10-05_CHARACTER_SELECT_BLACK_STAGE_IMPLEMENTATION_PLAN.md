# 클래스 선택의 원본 검은 바닥 무대 구현 계획

## G00. 핵심 목표와 현재 기준

사용자가 요청한 핵심은 선택 화면의 현재 기본 캐릭터 발밑에 원본 검은 팔각 바닥이 실제로 깔리는 것이다. 원본 Fighter floor1053/star1054는 중앙336과 수평947.885m 떨어져 있다. 현재 사용자 저작본은 이 쌍을 중앙102m 옆으로 옮겼으므로 그 배치를 덮어쓰지 않는다. 원본 위치의 별도 표시용 무대와 기본 캐릭터를 기존 Prototype/Clone/Layer/CModel 경로로 만들고 체험에서는 중앙의 기존 Server 플레이어를 그대로 사용한다. 원본 무비의 배우·의상 복구, 기본 캐릭터 외형 변경, Server 순간이동은 범위에 없다.

현재 PREVIEW는 같은 Server 캐릭터를 정면4.6m/FOV30으로 보고 SL00 전체를 숨긴다. TRIAL은 카메라 override와 조작만 복구한다. 맵 숨김·선택 조명·생성 화면과 소개 무비의 각 소유권을 유지하면서 별도 바닥 무대가 PREVIEW에서만 보이도록 연결한다. 현재 브랜치는 codex/world-map-inspection-save-fix이며 다른 작업의 dirty 변경을 보존한다.

## G01. Level 소유의 표시 무대

새 Client/Public/CharacterSelectShowcase.h와 Client/Private/CharacterSelectShowcase.cpp는 원본 바닥·별의 임시 배치 두 개와 표시용 기본 CCharacter 하나의 수명을 소유한다. Data/Rendering/Authored/CharacterSelectShowcase.json은 원본 stable source placement, 원본 TRS, 실측한 발 기준점과 yaw, 조명을 이동하기 전 중앙 기준점 lightingOrigin을 소유한다. 기존 로드된 SL00의 catalog/placement에서 geometry·재질·RNM을 재사용하고 CMapPlacementRuntime::Create_Placement로 두 임시 배치를 만든다. Imported나 out 자료를 런타임에서 읽지 않는다. 저작 placement는 변경하지 않는다.

parse→validate→숨긴 후보 생성→commit 순서로 준비하며 실패한 후보는 전부 제거한다. 승인된 현재 class의 표준 prototype/spec으로 isLocallyControlled=false, navigation 없는 표시 모델을 생성한다. 기본 body·head·equipment·weapon을 유지하고 customization/원본 movie assembly를 대신 적용하지 않는다. 명령 sink·replication registry·전투 entity에 등록하지 않는다. 같은 class는 재사용하고 class 교체 실패 시 기존 객체를 파괴하지 않는다. 숨긴 실제 플레이어의 이전 suppression을 보관했다가 체험·생성·무비·disconnect·Level 종료 때 복원한다. Engine의 기존 객체 Update/LateUpdate/Render를 사용한다.

Level_CharacterSelect의 전신 카메라는 준비된 표시 모델을 대상으로 한다. 준비 실패는 기존 아레나 표시와 오류 상태를 유지한다. 무대는 main MapRuntime의 전체 숨김 대상 밖에서 PREVIEW일 때만 표시한다. 클래스 변경·빠른 버튼 전환·생성 modal에 이전 무대나 실제 플레이어의 숨김이 남지 않게 한다. MainApp은 기존 선택 조명의 방향·강도·품질 값을 유지하면서 그림자의 기준점만 표시 무대로 임시 이동하고 종료 시 자기 offset만 해제한다. CMapLightPresentationRuntime의 프레임별 위치 offset으로 POINT/SPOT의 frustum 검사·실제 제출 위치를 함께 옮기고 다음 기본 호출에서는0으로 복귀한다. 색·강도·방향·range·receiver·RNM·환경·후처리 저작 값은 변경하지 않는다.

## G02. 등록과 검증

새 H/CPP는 Client.vcxproj와 물리 폴더에 맞는 filters에 등록하고 JSON은96.DataFiles의 None 항목으로 등록한다. 기존 파일은 UTF-8/CRLF 등 감지한 인코딩을 보존한다. 새 C++는 UTF-8 BOM 없음으로 쓴다. 바닥 실측 상면·캐릭터 발 원점·원본 거리·재질/RNM 보존과 문서 parse/실패 rollback을 확인한다. 독립 검토 후 Debug/Release Product Build로 실제 소비자를 컴파일·링크한다. 이미 실행 중인 EXE/DLL이 잠긴 경우만 종료 안내를 한다. Client/UI는 자율 실행하지 않으며 사용자 최종 확인은 선택→체험→복귀·직업 변경·생성 화면의 발 접지와 바닥 표시다. 실행한 검증과 남은 화면 확인은 대응 RESULT에 분리한다.

구현과 Debug/Release Product 검증을 완료했다. 실제 검증 증거와 사용자 화면 확인 경계는 같은 이름의 RESULT를 따른다.

## G03. 워로드 PREVIEW 카메라 구도 보정

후속 사용자 승인에 따라 선택 화면에서 워로드 몸체가 작게 보이는 부분을 반영한다.
기본 몸체 자체는 가장 크지만 normal battle idle의 높이가 기본 대비80~85%로 낮아지는 것이
실측 원인이다. 기존 기본 캐릭터·전투 자세와 모든 월드의 presentation scale은 보존하고
워로드의 PREVIEW 카메라만 기존 공통구도보다 가까이 둔다.

`Level_CharacterSelect.cpp`의 공통4.6m를 다른 직업에는 유지하고 워로드는4.4m를 사용한다.
eye1.05m와look0.95m도4.4/4.6 비율로 함께 줄여 발 원점의 투영 위치와 카메라 기울기를
보존한다. FOVY30도는 유지한다. 선택 class의 실제 presentation character에서 class ID를
읽으며 PREVIEW를 종료하면 기존 owner override 종료 경로로 TRIAL/follow camera를 복구한다.
바닥 TRS·조명·shadow focus·카메라 JSON·무비/생성 카메라는 변경하지 않는다.

한 기존 CPP만 수정하므로 신규 파일/project/filter 등록은 없다. 파일의 UTF-8 무BOM/CRLF를
보존한다. 실제 기본 몸체의 기존 idle 네표본으로 보정 전후 화면 비율과 발 원점 위치를 비교하고,
가능한 실제 장비 attachment 범위에서 무기·방패의 화면 잘림을 검토한다. CPU 계산과 실제
화면은 구분한다. 독립 코드 검토 후 Debug/Release Product 빌드와diff check를 수행한다.

## G06. 사용자 화면 확인 후 팔각별과 바닥 반사 보정

후속 스크린샷에서 별이 밝고 바닥 뒤에 밝은 반사 무늬가 남는 것을 확인했다.
현재 별의 diffuseBrightness는1, 바닥은0이며 두 재질 모두 별도 reflectionIntensity와
specularPBRIntensity가 남아 있다. PBR 셰이더는 diffuseBrightness 적용 뒤 2D 반사를
기본색에 더하고, 직접광 및 환경광의 반사도 별도로 계산한다. 밝기0만으로 검정 무대가 되지 않는다.

CharacterSelectShowcase.cpp에서 두 임시 배치의 loaded asset materialOverrides를 복사하고
각 surface의 diffuseBrightness, reflectionIntensity, specularPBRIntensity 세 값만0으로
설정한다. 기존 Create_Placement의 materialOverrides 인자로 전달하여 CModel의 material
variant를 사용한다. source catalog와 prototype 재질, 원래11쌍의 저작·게시 입력은 보존한다.
RNM·환경 texture·geometry·normal·opacity와 캐릭터 및 조명 설정도 보존한다. 재질이 없으면
기존 stage transaction 실패 경로로 처리하여 일부 무대만 표시하지 않는다.

기존 CPP 한 파일만 변경하며 새 파일/project 등록은 없다. UTF-8 무BOM/LF를 보존한다.
실제 두 재질 입력과 shader의 반사·specular 소비를 대조하고 독립 diff 검토 및 Debug/Release
최소 컴파일을 수행한다. 실행 파일이 잠겨 있으면 제품 출력을 교체하지 않으며, 종료를
확인한 뒤 Debug/Release 제품 빌드까지 수행한다. Client/UI는 실행하지 않고 사용자 화면
확인과 제품 빌드를 결과에서 분리한다.
