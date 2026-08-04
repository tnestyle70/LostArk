# Animation Tool과 Effect Tool 연결 경계 결과

## 완료된 범위

- 기존 Animation Tool의 Target, Playback, Chain, Event, Import, Save, Reload UI를 유지했다.
- Character Select의 다섯 클래스와 Dimensionist Character/Core/Summon preview target을 Animation Tool에서 선택할 수 있다.
- `AnimationTargetService`가 target 교체 generation을 제공해 오래된 preview 참조를 구분할 수 있게 했다.
- `AnimationAuthoringBridge`가 현재 animation asset, clip, playhead 시간, duration, pause 상태를 읽기 전용 snapshot으로 제공한다.
- `.animevents` v3를 기존 상태 그대로 읽고, 실제 편집 후에는 EFFECT source와 admitted asset ID를 구분하는 v4로 저장할 수 있게 했다.
- Load는 전체 검증 후 교체하며, Save는 임시 파일 작성·flush·재검증·원자 교체 순서로 처리한다.
- 저장하지 않은 상태에서 Reload하거나 target을 바꿀 때 명시적으로 저장 또는 폐기하도록 보호한다.
- 기존 EFFECT event 버튼과 source payload 편집 흐름을 복구했다.

## 이번 변경에서 만들지 않은 것

- 공용 Character Preview Panel
- 새 Effect Tool UI
- admitted Effect catalog와 실제 `Use Selected Effect` 연결
- root/weapon/bone/mouse-ground attachment 편집
- Trail start/stop runtime
- Animation HIT marker에서 Server damage 판정으로 이어지는 publisher

위 항목은 현재 Animation 기능을 다시 깨뜨리지 않도록 별도 수직 슬라이스에서 연결한다.

## 실제 검증

- Client x64 Debug 빌드 성공
- Client x64 Release 빌드 성공
- Debug Client 시작 smoke 성공
- 사용자가 실행 화면에서 기존 Animation 선택, clip과 event UI 복구를 수동 확인
- Dimensionist Character/Core/Summon target 등록 확인
- `git diff --check` 오류 없음

`ProjectAudit`은 코드 항목이 아니라 로컬 `Client/Bin/Resources`와 `Data/AssetPacks.lock.json`의 inventory 불일치에서 실패했다. ZIP과 asset pack 버전은 사용자 관리 범위로 분리했으며 이번 코드 PR에는 lock rollback이나 기존 immutable manifest 삭제를 포함하지 않는다.

## 담당자 인계

Animation 담당자는 `.animevents`와 event timing을 계속 소유한다. Effect 담당자는 `.effect`의 emitter/module/attachment를 소유하고 Animation 파일을 직접 쓰지 않는다. Preview 담당자는 Character/Body/Weapon 생성과 anchor transform을 소유한다. 실제 hit와 damage는 Server가 판정한다.
