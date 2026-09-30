# 가디언나이트 기본 머리카락 표시 복구

## G00. 원인과 변경 범위

사용자가 최근 변경 후 머리카락 소실과 머리 장식 노출을 보고했고 원인 조사 후 수정을 요청했다.
기준 HEAD는 `a1329e0c2ba0274f21bba74e9120eca0de26bac6`이다. 같은 작업 트리의
콜로세움 계획서와 다른 미추적 파일은 보존한다.

`63f2e54cf`에서 Guardian의 `BAKED_HAIR`가0에서 bit0으로 바뀌었다. 현재
`CCharacter::Ready_PartObjects`와 `Apply_DefaultEquipmentVisibility`는 별도 HEAD
착용 여부와 무관하게 이 bit를 숨긴다. 기본 파츠 목록에는 별도 헤어가 없으므로 저장 외형을
적용하지 않는 preview·audition·원격 player에서 본체 헤어만 사라진다.

설치 `GuardianKnight.wmodel`의 submesh0은 `pc_ft_15_hair_mi`,1514정점·6192indices이며
diffuse/normal 파일도 존재한다. 기본 HEAD의 `class_select_hr00/helmet.wmodel` 경로는
9월22일부터 유지됐다. 최근 추가된 CustomizingAnimSet은 별도5clip 리소스다.
기본 helmet이나 원본 Movie의 뿔을 임의 삭제하지 않고 이전 기본 외형을 복구한다.

## G01. CharacterSpec과 Guardian 선언

`Client/Public/CharacterSpec.h`의 마지막에 `isBodyHairFallback=false`를 추가한다.
기본값은 기존 클래스의 영구 숨김 정책을 보존한다. `Logic_GuardianKnight.cpp`만 true로
선언하여, 대체 HEAD가 commit되지 않은 동안 본체 헤어를 사용한다. 기존 hair bit0은 유지해
새 헤어와 겹쳐 그려지지 않게 한다. 변수 설명도 실제 소비 조건으로 맞춘다.

## G02. Character 표시 갱신

`Client/Private/Character.cpp`의 공용 mask 계산에서 fallback인 class는 성공적으로
commit된 HEAD 점유가 있을 때만 본체 hair bit를 숨긴다. 최초 생성은 점유0으로 계산한다.
헤어·의상 적용 성공은 기존 `Apply_DefaultEquipmentVisibility`를, reset은 기존0점유 경로를
소비한다. stage/clone 실패는 현재 파츠·점유·mask를 변경하지 않는다.

새 C++ 파일과 project/filter 등록은 없다. Resources·JSON·렌더링 옵션·Shared/Server
계약은 변경하지 않는다. 공통 재발 방지는 gotchas와 렌더링 복원 문서에 기록한다.

## G03. 검증

설치 body의 실제 submesh와 재질, 새 헤어와 장식의 분리 상태를 읽기 검증한다.
기본 생성·HEAD 성공·HEAD 이외 장비·reset·실패 보존 및 기존 클래스의 mask 정책을 확인한다.
필요한 제품 증분 컴파일·링크와 `git diff --check`를 수행하고 실행 증거를 RESULT에 기록한다.
현재 사용 중인 Release EXE는 종료하지 않는다. 잠금이 없는 구성부터 검증하며,
Release 반영에 실제 링크 잠금 해제가 필요하면 완성된 수정본 기준으로 사용자에게 안내한다.
Client/UI는 실행·조작하지 않으며 최종 화면은 사용자 확인 대상이다.

## G04. 생성창 기본과 저장 색상

사용자가 초기 선택의 기본 머리 복구를 확인했으나 생성창 진입 시 검은 별도 헤어로 바뀌고,
아바타 변경 시 다시 사라짐을 보고했다. 전부 수정을 승인했다.
`CustomizingHairstyles.json` Guardian에 `defaultBodyHair: true`를 선언한다.
`CCustomizingCostumeDocument`는 hairstyle에서만 이 설정을 허용하고 기존 defaultVisualSetId와
중복을 거절한다. 기본 index -1은 본체 fallback이다. `CustomizingView`는 해당 class에서만
-1을 유효하게 resolve하고 최초·reset·저장·복원에 유지한다. 미선택 surface는 alpha -1이며
기존 hair의 [0,0,0,0] 저장값만 unset으로 해석한다. 실제 선택한 검정 alpha1은 보존한다.
`Level_CharacterSelect::Apply_CustomizingHair`는 fallback 선택 시 HEAD 세트만 제거한 후보를
기존 equipment service로 commit한다. 실패하면 이전 장비를 유지한다.

## G05. 아바타와 머리의 독립 착용

Guardian 생성창의 5개 outfit은 HEAD를 점유하며 helmet을 포함한다. 기존 full outfit은
유지하고 `.customizing_outfit` stable ID로 HEAD를 뺀 torso 전용 정의를 추가한다.
`CustomizingCostumes.json`의 기존 0~4 순서는 이 정의를 참조한다. 머리 index 및 기존 저장
의상 index는 바뀌지 않는다. `CharacterOutfitApplier`도 같은 문서를 소비하여 의상+머리를
후보 배열로 구성한 뒤 한 번만 Apply_Preview한다. 어느 부분이 실패해도 기존 외형을 보존한다.
Resources/WModel 재생성 및 렌더링 옵션 변경은 없다. 별도 C++ 파일과 project 등록은 없다.

## G06. 추가 검증

5개 torso 정의와 원래 full outfit의 body parts 일치, HEAD 미점유, 사용 가능한 전체 헤어와의
양방향 조합을 검사한다. parser의 -1 기본·잘못된 설정 거절·이전 상태 보존 및 저장 외형
미선택 색상 처리를 확인한다. 같은 VS 설치/amd64 tool host의 Release 증분 빌드로 연결한다.
제품 EXE 잠금이 확인되면 후보 완성 후 해당 Client 종료만 요청한다. UI 실행은 사용자 전용이다.

## G07. 기본 머리 색 편집과 교체 후 유지

본체 헤어는 native source 상수를 소비하므로 기존 WMA3 dye API만으로는 색 편집이 반영되지
않는다. Character::Set_DyeColor의 HAIR은 기존 source parameter API로 두 hair 색을 연결한다.
unset은 actor의 해당 재질 원래 두 색만 복원해 얼굴·눈·화장 상태를 보존한다. 투톤 강도는
실제 source 혼합 계수를 사용하고 native 범위 A/B는 원본 값을 보존한다.
CustomizingView::Reapply_HairControls는 class가 일치하고 실제 선택된 색/투톤만 장비 commit
뒤 다시 적용한다. 투톤 미변경 여부는 optional hairTwoToneOverride로 저장하며, reset과 class
변경은 unset으로 돌아간다. 별도 shader·Resources 변경은 없다.
