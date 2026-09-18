# 쇼타임·베른 인수 수정 결과

## G01. 작업 및 검증 경계

실제 설치 저장소: `C:/Users/USER/source/졸업팀폴/LostArk`.
기존 미커밋 변경은 보존했다. 커밋·푸시는 하지 않았다. Client/UI 실행과 화면 캡처는 하지 않았다.
쇼타임 화면 문제에 대한 코드·데이터·Debug 실행 파일 반영까지 완료했다. 최종 화면 판정은 사용자 확인 대기다.
베른 통로는 조사 중이며 바닥 복구 또는 보행 정상화를 완료한 상태가 아니다.

## G02. 앞선 카메라 거리 보정

네 쇼타임 카메라의 190키를 원본 배우 기준점에서 1.4166666103733934배 거리로 보정했다. 원본 drawscale 1.2000000476837158과 현재 bodyModelPreScale 0.017을 대조한 값이다. 시간·FOV·up·ID는 보존했다. 원본 백업은 `out/ShowtimeBernContinuation20260918/camerashots.before-scale.json`이다.
이 보정에는 바닥 여유 높이 검사가 빠져 있었다. 다음 G03이 그 결함을 수정한다. 데이터 비례만으로 원작과의 최종 구도가 같다고 판정하지 않는다.

## G03. 사용자 20:29~20:30 화면의 두 결함

### 바닥 관통

사용자 Sequencer 커서는 1548ms다. 해당 샷은 `kouku.gate3.showtime.camera.2`, 샷 로컬 시각 268ms다.
설치 모델의 submesh별 vertex/index offset을 반영해 `LV_LUT_MIDNIGHTC_ED_SL05:export:342`의 `bg_rad_koukusaton_floor08a_sm` 삼각형을 월드 변환하고 교차 높이를 계산했다.

- 바닥 상면 Y: 1.317626m.
- 1548ms 카메라는 occurrence 위치 오프셋 적용 후 상면보다 0.088936m 낮았다.
- 두 번째 샷 최저점은 로컬 296ms이며 상면보다 0.122626m 낮았다.
- 두 번째 샷의 eye/lookAt/선택 box center Y만 +0.4m 올렸다. 시각, X/Z, FOV, up, 카메라 방향은 그대로다. 다른 샷은 이전 보정 상태에서 바뀌지 않았다.
- authoring revision 89 → 90. 첫 원본 백업에서 재계산하므로 중복 실행해도 크기·높이를 거듭 적용하지 않는다. 추가 수동 수정이 있으면 자동 덮어쓰기 대신 거절한다.

`test_showtime_camera_clearance.py`: 3개 검사 통과. 4개 샷을 매 1ms 보간하고 near=0.1m, 화면비 4:3·16:9·21:9에서 near plane 네 모서리의 상면 여유를 검사했다. 최저 여유 0.289473m. 이는 해당 평평한 바닥에 대한 수치 검사이며 장면 전체의 시각 PASS가 아니다.

### 정지 보스 중복

MainApp의 Animation 포함 Pattern 미리보기는 `Begin_BundlePreview(..., externalWorldPreview=true)`로 별도 CNpc 배우를 만든다. 기존 replicated boss는 그대로 그려지고 있었다.

- CNpc에 미리보기 전용 표시 억제 참조 카운트를 추가했다. 기존 base visibility, network Update, transform, action, 충돌과 서버 상태는 수정하지 않는다.
- 살아 있는 미리보기 멤버만 해당 source archetype의 서버 본체를 억제한다. source archetype은 검증된 Gameplay boss placement에서 얻는다. G3 세이튼은 현재 문서에서 해당 archetype 배치가 하나임을 확인했다. 동일 archetype 본체가 여러 placement에 생기는 미래 계약은 별도 대상 식별 보강이 필요하다.
- synthetic split/summon clone은 별도 본체 숨김을 획득하지 않는다.
- 모든 staging 성공 후 commit에서 숨긴다. 실패한 새 미리보기는 기존 표시 상태를 바꾸지 않는다.
- commit 직후, Update 준비 대기 전, Sample에서 동기화해 첫 프레임과 서버 객체 교체도 처리한다.
- Stop, 실패, 교체, Reset/destructor에서 자신이 획득한 억제만 해제한다. 여러 소유자가 겹쳐도 마지막 억제 해제 전에는 보이지 않는다. 원래 숨김 상태도 유지한다.
- 같은 bundle actor 경로를 쓰는 source-model 미리보기에도 이 규칙이 적용된다. Product Play 자체에는 preview 멤버가 없으므로 이 숨김이 걸리지 않는다.

`test_preview_source_visibility.py`: 실제 NPC inline 메서드를 추출해 native 비GUI 테스트로 컴파일·실행했다. 중첩 획득·해제, base visibility 보존, 반복 10,000회, 추가 release를 검사했다. 별도 source wiring 검사까지 2개 통과. 초기 테스트 실행은 Windows shell 인용 문제로 실패했고 명령을 수정해 재실행했다.

## G04. 설치 및 빌드

- VS2022 Community의 기존 v143 14.44.35207, SDK 10.0.26100.0 경로로 `Client.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64 /p:BuildProjectReferences=false` 성공(exit 0).
- Clean/Rebuild 하지 않았다. Server/Engine 프로젝트 빌드는 하지 않았다. 기존 설치 Engine/Shared 산출물을 사용했다. 기존 C4819/C4828 인코딩 경고는 있었으며 광역 인코딩 변경은 하지 않았다.
- 설치: `Client/Bin/Debug/Client.exe`. Client 작업 디렉터리 `Client/Default`.
- `Publish-MapAuthoring.ps1 -AreaId LV_LUT_MIDNIGHTC_ED -Scope CameraShots -Mode Validate/Publish/Check` 각각 성공. 카메라 파일 하나만 게시했다.
- 동일 Area 비카메라 runtime 파일 7개는 게시 전후 SHA256 불변. 조명 동반 게시 없음.
- 카메라 runtime SHA256 `d3ee95ce5a77f7b68364bf78bee04642e84319cf05313ba6b7dd5211547da157`.
- C++ 네 파일은 UTF-8 BOM 없음과 CRLF 보존. 신규 C++ 파일이 없어 project/filter 등록 없음.

## G05. 사용자 확인

기존 방식으로 Client를 다시 실행하고 F1의 같은 Action Workbench/Composition Sequencer에서 `쇼타임_연출`을 선택한다.

1. Play: 미리보기 배우만 보이고 정지한 서버 보스가 겹치지 않는지.
2. 1548ms에 스크럽·Pause: 바닥으로 화면이 가려지지 않는지. 1280~2496ms 전체도 확인한다.
3. Stop: 전투 보스가 다시 보이는지.
4. Play → Stop → Play 및 다른 Pattern 전환: 숨김이 남지 않는지.

Play Pattern의 미완료 PRODUCT 애니메이션 stage 게시 문제는 이번 미리보기 표시/카메라 수정과 별개다. 기존 Claude animationOccurrences 두 개를 임의로 삭제·재게시하지 않았다. 베른은 후속 조사 항목으로 남긴다.
