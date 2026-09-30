# 가디언나이트 기본 헤어 소실 조사·수정 결과

## G00. 확인한 원인

기준 commit은 `a1329e0c2ba0274f21bba74e9120eca0de26bac6`이다.
9월30일 `63f2e54cf`에서 별도 헤어50종을 추가하며 Guardian의 `BAKED_HAIR`를
0에서 bit0으로 변경했다. `CCharacter`의 최초 생성과 장비 표시 갱신은 별도 헤어가
장착됐는지와 무관하게 이 mask를 적용했다. Guardian 기본 파츠에는 helmet만 있으므로
저장 외형이 없는 spawn·Character Select audition·일반 preview에서 본체 머리만 숨겨진다.

설치된 본체 WModel은6개 submesh이며0번은 `pc_ft_15_hair_mi`,1514정점·6192indices다.
diffuse와 normal 파일도 존재한다. 본체 SHA256은
`2126da1ef2097119407e85fa25638a5a3132ecc41c48ed91ba65c62be9af8ed9`이다.
머리 geometry가 삭제됐다는 증거는 없으며 실제 소비 mask와 최근 변경으로 소실이 설명된다.

기본 HEAD는 `Character/GuardianKnight/Equipment/class_select_hr00/helmet.wmodel`이다.
이 연결은9월22일 `829385641`부터 유지됐다. 현재3808정점·16248indices,
`pc_ddk_02-4_helmet_mi`를 사용한다. 사용자 보고의 뿔은 기존 머리 장식 노출과 일치하는
후보이며, 화면을 보지 않았으므로 정확한 형태를 육안 확정하지 않았다.

최근 통합한 `GuardianKnight_CustomizingAnimSet.wmodel`은 별도5clip 애니메이션 리소스로
설치본이 `TJ_Resources (5).zip`과 byte 동일하다. 신규 기본 선택 헤어
`Equipment/pc_dk_55_hair/head.wmodel`도 같은 ZIP과 동일하고 참조 texture가 존재한다.
`8653c320b`의 미지원 index1 차단은 기본 body mask를 복구하지 않는 별도 변경이다.

## G01. 반영한 수정

`CHARACTER_SPEC.isBodyHairFallback`을 기본false로 추가하고 Guardian만true로 지정했다.
`Character.cpp`의 공용 계산을 최초 생성과 장비 표시 갱신 두 곳에 연결했다.
Guardian은 HEAD 대체가 없으면 mask6으로 본체 헤어를 유지하고, HEAD 교체가 성공하면
mask7로 기존 헤어를 숨긴다. reset은6으로 복귀한다. clone·group commit 실패는 기존처럼
표시 상태를 바꾸기 전에 반환한다. 다른 클래스는 기존 body|hair 정책을 유지한다.

기존 C++3개는 UTF-8/BOM 없음·CRLF를 유지했다. 새 C++ 파일·project/filter 등록은 없고
Resources·JSON·Shared/Server·렌더링 옵션은 수정하지 않았다. 재발 원리와 class 선언 계약은
gotchas, 렌더링 복원 문서, 팀 사용서의 해당 항목에 반영했다.

## G02. 실행한 검증

- 실제 제품 source에서 추출한 동일 mask 함수의 C++ fixture: Guardian과 기존 정책의
  각64개 slot mask, HEAD→reset 확인, exit0. `out/GuardianHairFallback20261001/hair_policy.cpp`
  및 `hair_policy_source.json`에 범위와 입력 source hash를 기록했다. 전체 CCharacter/GPU
  실행을 대신하는 검사가 아니다.
- 기존 equipment catalog 검사4개 PASS.
- 갱신한 spawn/commit hair policy 연결 검사1개 PASS.
- 기존 equipment authoring contract 전체14개 중13개 PASS,1개 FAIL.
  실패는 LanceMaster 기본 파츠의 기대8개/실제6개다. 수정 전 HEAD의 원래 검사13개도
  같은 실패1개가 재현되므로 이번 수정의 회귀가 아니다. 무관한 기본 파츠는 바꾸지 않았다.
- 독립 정적 검토에서 최초 생성·HEAD commit·reset·실패 보존과 다른 class의 기본값을 확인했다.
- 변경 C++ 및 Markdown의 `git diff --check` PASS.

## G03. 빌드와 실행 반영

표준 Product runner의 최초 Debug 시도는 실행 중인 Release Client/Server까지 막는
`ProductOutputGuard`에서 컴파일 전에 중단됐다. 공유 Data 변경 없는 Debug 빌드를 직접
호출했으나, 이전 제품 빌드의 VS18 Insiders 대신 VS2022를 선택하여 불필요한 FX 갱신이
진행됐다. 해당 세션이 시작한 MSBuild 및 자식 compiler만 중단했다. 이어진 VS2022
ClCompile도 기존 PCH와 C1853 호환 오류가 났으며 Debug 빌드 성공으로 기록하지 않는다.
중간 산출물·PCH·추적 파일 삭제나 timestamp 보정은 하지 않았다.

이후 사용자 Client 종료를 확인했다. 이전 Release 성공 receipt의 정확한
`Visual Studio/18/Insiders/MSBuild/Current/Bin/amd64/MSBuild.exe`로 Client Release의
정상 증분 Build를 수행했다. Release Client 컴파일·링크·배포 완료, 오류0,
기존 경고1065개,1분27.35초다. `out/GuardianHairFallback20261001-release-client.log`와
동일 이름 binlog가 증거다. 제품 runtime은 `Client/Bin/Release/Client.exe`에 반영됐다.
Debug의 실패를 Release 성공으로 대체하지 않는다. Client/UI 실행·조작·화면 캡처는 하지 않았다.

## G04. 화면 확인과 범위

사용자가 수정 실행본에서 기본 Guardian 모습, 새 헤어 선택, 기본 외형 복귀를 확인한다.
기본 helmet과 원본 Movie의 별도 뿔은 삭제하지 않았다. full 의상을 명시적으로 선택하면
그 세트가 HEAD까지 점유하여 기존 헤어를 교체하는 현재 의상 계약도 유지한다.
생성창 기본 진입은 costume=-1이며 의상을 자동 장착하지 않는다.

## G05. 사용자 후속 확인과 생성창의 남은 결함

사용자는 첫 선택의 기본 머리가 정상으로 돌아왔음을 확인했다. 이어서 생성 버튼을 누르면
검은 머리로 바뀌고, 아바타를 바꾸면 머리가 다시 사라진다고 보고했다. 후속 요청은 원인
분석이며 아래 확인 동안 제품 코드·데이터를 추가 수정하지 않았다. 앞선 fallback 수정은
최초 기본 생성 경로를 복구했고 이 두 생성창 경로는 해결하지 못했다.

생성창의 `CustomizingView::Open`은 hairChanged=true를 설정한다. Guardian의
CustomizingHairstyles에는 defaultVisualSetId가 없어서 reader가 index0인
`character.guardian_knight.pc_dk_55_hair.head`를 선택한다. Level의
Apply_CustomizingHair→Wear_CustomizingSet이 본체의 pc_ft_15_hair를 별도55헤어로 교체한다.

설치55헤어의 두 WMA3 재질은 diffusecolor RGB 약(0.005525,0.003984,0.003693),
두 번째 색 약(0.027623,0.018450,0.020346)을 내장한다. 두 dye mask DDS도 존재한다.
DeferredMaterialRenderUtils가 이 값을 바인딩하고 일반 hair shader가 색을 보간한 뒤
mask.g를 곱한다. 따라서 화면 진입 때 검은 기본 헤어가 나오는 것은 새 헤어의 선택과
어두운 내장 염색값으로 설명되며, 누락 texture나 본체 WModel 손상으로 판정하지 않는다.

생성창의 Guardian 의상5개는 모두 UPPER/LOWER/HANDS/SHOULDER와 HEAD를 함께 점유하고
helmet part를 포함한다. Wear_CustomizingSet은 HEAD가 겹친 기존 hairstyle set을 목록에서
제거한다. 이후 HEAD 점유 mask가 본체 헤어도 숨기므로 선택 헤어와 본체 헤어가 모두 빠진다.
모코코 torso outfit은 HEAD를 점유하지 않으므로 이5개 try-on 의상과 구분한다.

수정 방향은 생성창의 기본 헤어·색상 정본을 첫 선택 외형과 맞추고, 생성창에서 의상과
헤어를 함께 유지할 수 있도록 슬롯 조합을 분리하는 것이다. 단순히 의상 뒤에 헤어를
다시 적용하면 HEAD 충돌 해소가 의상 세트 전체를 제거하므로 충분하지 않다. 저장 외형
복원도 같은 의상→헤어 순서이므로 같은 계약으로 연결해야 한다.

## G06. 후속 생성창·아바타 수정 반영

사용자 `전부 해결해줘` 지시에 따라 G05 두 경로도 구현했다.

- Guardian hairstyle에 defaultBodyHair=true를 추가했다. parser는 hairstyle 전용 bool과
  중복 default를 검증하고 stage 완료 뒤 commit한다. View는 본체 기본 -1을 최초 진입,
  reset, 저장, 복원에 유지한다. 다른 헤어 51행의 기존 index는 변경하지 않았다.
- 기존 full outfit 5개를 보존하고 같은 torso parts에서 HEAD만 제외한 customizing_outfit
  5개를 추가했다. 생성창 costume 0~4가 이를 참조한다. 헤어/의상 어느 순서로 선택해도
  HEAD 충돌로 서로 제거되지 않는다. 기본 머리 선택은 HEAD만 비운 후보를 commit한다.
- CharacterOutfitApplier는 의상과 머리를 후보 배열에 구성한 뒤 Apply_Preview를 한 번 호출한다.
  실패하면 이전 장비가 유지된다. 별도 hairstyle 문서가 없는 class의 기존 기본 장비는 보존한다.
- surfaceColors의 미선택은 alpha -1이다. 예전 hair [0,0,0,0]만 unset으로 읽고 사용자가 직접
  선택한 검정 alpha1은 보존한다. hairTwoToneOverride는 조작 여부를 저장하여 기본 투톤을
  저장 복원 때 임의 0으로 바꾸지 않는다. 새로운 class의 색상 선택창 상태도 초기화한다.
- 장비 transaction 성공 직후 Reapply_HairControls가 실제 선택한 hair 색/투톤만 적용한다.
  class 일치 guard로 이전 class 선택값 유입을 막는다. 본체 hair는 native parameter 두 색을
  사용하고 unset은 해당 두 색만 원본 복원한다. 얼굴·눈·화장을 함께 초기화하지 않는다.
  native 투톤은 원본 강도 계수만 연결하며 별도 native 범위 A/B는 기존 원본을 유지한다.

Resources/WModel·shader·렌더링 옵션·Server 계약은 변경하지 않았다. 새로운 C++ 파일이나
project 등록은 없으며 기존 C++의 UTF-8/BOM 없음·CRLF를 보존했다. JSON은 교체 직전 bytes를
재확인하고 out 백업 뒤 원자 교체했다. 이 JSON은 ProjectDataRoot로 직접 읽는 정본이므로
별도 DataFiles publisher는 없다. 이미 실행 중인 화면을 Reload하거나 게임을 실행하지 않았다.

## G07. 후속 자동 검증

- 신규 Guardian 데이터 검사 5개 PASS: 기존 헤어 index51개 보존, torso variants 5개와
  원본 body part 동등성, 지원 헤어50종×의상5종의 양방향 slot 독립, 원본 native hair 색과
  설치 resource 존재. 기존 catalog 4개 및 spawn/commit mask 검사1개 포함 총10개 PASS.
- 실제 CustomizingCostumeDocument.cpp와 DataJson.cpp의 격리 C++20 compile/run은
  14 assertions PASS. 기본 -1/기존0/명시 default, wrong type·conflict·late invalid row·
  costume 오용 거절, reload 실패 시 이전 document 보존, 실제51행 문서 로드를 확인했다.
  증거: out/GuardianHairFallback20261001/parser-regression/.
- 첫 Release 증분 Client Build는 C++ 컴파일 성공 후 실행 중 Client.exe 점유로 링크에서만
  LNK1104가 발생했다. 로그 out/GuardianHairFallback20261001-customizing-release.log,
  14.29초, 기존 경고700개. Client 종료 후 최종 링크와 마지막 코드 검증 결과는 아래에 기록한다.
- 독립 리뷰에서 class 변경 중 열린 picker의 옛 색 유입과 색 변경 시 legacy 투톤 alpha 덮임을
  확인하여 해당 View 경로를 보완했다. 전체 게임 화면/GPU 표시 성공으로 확대하지 않는다.

## G08. 최종 상태와 실행 파일 반영 경계

최종 View 보완을 포함한 Release C++ 컴파일은 성공했다. shader FxCompile은 기존 유효한
출력을 정상 Build가 재사용했다. 최종 Build의 링크 직전 사용자가 Client를 다시 실행하여
LNK1104가 재발했다. 최종 로그는
`out/GuardianHairFallback20261001-customizing-final-release.log`와 동일 이름 binlog이며,
6.93초, 기존 경고14개, 링크 오류1개다. 실행 중인 파일은
`C:/Users/user/Desktop/LostArk/Client/Bin/Release/Client.exe`, 확인 PID11668이었다.

Client를 종료해 달라고 안내한 뒤 해당 절대 경로의 process 종료를120초 기다렸으나 계속
실행 중이어서 최종 링크를 추가 실행하지 않았다. Client·Server를 임의 종료하지 않았다.
따라서 소스·데이터 수정과 컴파일·기능 자동 검사·최종 정적 리뷰는 완료했고, 새 Release EXE
반영은 아직 미완료다. Client 종료 후 같은 VS18 Insiders amd64의 정상 증분 Client Build를
마무리해야 한다. 현재 실행 게임에 새 코드가 반영됐다고 설명하지 않는다.

최종 scoped git diff --check, C++ UTF-8/CRLF, 변경 JSON3개 parse는 PASS다.
입력 source hash는 out/GuardianHairFallback20261001/customizing-final-source.json에 기록했다.
사용자 화면 확인은 최초 선택의 기본 머리까지만 확인됐고, 이번 생성창 기본 머리·아바타 변경·
선택색 유지·캐릭터 생성 후 복원은 새 실행 파일 반영 뒤 사용자 확인 대상이다.

## G09. 최종 기본 머리 제품 빌드

이동 보완과 모든 최신 Client/Server 소스를 포함한 정상 Product Debug/Release가 모두 성공했다.
앞선 EXE 잠금·최종 링크/제품 빌드 대기는 해소됐으며 실행 파일과 실제 로그는
`../09-27/2026-09-27_GUIDE_AI_TOOL_IMPLEMENTATION_RESULT.md`의 G09에 기록했다.
이 결과는 기존 기능별 검증을 대체하거나 실제 Client 화면·다인 플레이·성능 확인으로 확대하지 않는다.
