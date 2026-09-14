# 원본_갈고리 비교용 모션 추가 결과

## 구현 상태

- World Object Tool 정본 `Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json` revision 672→673.
- `world.object.kouku.hook` 아래 기존 `hook_diagonal` 바로 다음에 template/instance 한 쌍을 추가했다. 표시 이름 `원본_갈고리`, Count 1, Lifetime 11334ms.
- 기존 resource/Default Motion/178개 template/233개 instance 모두 semantic equality 확인. Composition 및 실제 3관문 패턴, C++/Shared/Server/Resources는 변경하지 않았다.
- 실제 설치 WModel에서 원본 4클립의 이름과 길이를 확인했다. respawn/attack1/attack2/attack3를 0/1334/8334/9834ms에 재생한다.
- 전진 clip의 부모 골격까지 적용한 root displacement는 scale 적용 후 24.43749684m. 이동 자체는 원본 animation이며, 다음 clip의 root 초기화 시점에만 저작 위치를 24.4375m 넘겨준다. 8333~8334ms는 선형 보간 중 이중 위치가 보이지 않도록 숨긴다.
- ±12.21875m 프리뷰 위치, 네 클립의 순차 연결 및 1ms 전환 경계는 프로젝트 저작값이다. 원작 서버 패턴/소환 배치 복원을 주장하지 않는다. 사운드, 14단계 연속기, 플레이어 잡기와 피해 판정은 이번 비교 모션에 없다.

## 검증과 배포 경계

- 새 ID 유일성, JSON parse, native clip/길이/root displacement, 기존 모든 row 불변 검사 통과.
- 변경 authoring의 `git diff --check` exit 0. 정본 데이터 diff는 새 모션/instance 207줄 추가와 revision 한 줄 변경뿐이다.
- 공식 `Publish-MapAuthoring.ps1 -AreaId LV_LUT_MIDNIGHTC_ED -Scope WorldSequences -Mode Validate` 통과.
- 기존 배포본의 Check는 수정 전부터 실패했다. 저작 revision 672와 실행용 revision 675가 다르고, 실행용에는 다른 컷신 리소스/모션과 별도 변경이 있다. 이를 삭제하거나 정본으로 역수입하지 않았다. Publish는 보류했고 기존 배포본은 수정하지 않았다.
- C++ 변경이 없어 재빌드는 하지 않았다. Client/UI 실행·조작·캡처 및 visual PASS는 수행하지 않았다.

## 사용자 확인

1. 현재 툴에 미저장 편집이 있다면 Reload 전에 먼저 별도로 보존한다. 이번에는 Save/Publish를 누르지 않는다.
2. 쿠크 아레나 → F1 → World Object Tool → Reload Source.
3. 월드오브젝트_갈고리 → 원본_갈고리 → Preview at Character 켬 → Play.
4. 미리보기는 `CWorldObjectTool::Begin_Preview`가 저작 `m_Document`를 직접 넘기는 기존 경로여서 제품 배포 없이 가능하다.
5. 등장·전진·내려치기·회수와 연결부를 사용자가 판정한다. 기존 18개 모션과 전투 패턴은 그대로다.

저장/게시 재개 전에는 기존 실행용과 정본 차이의 소유 작업을 확인해야 한다. 이번 추가를 이유로 전체 실행용 문서를 덮어쓰지 않는다.

## 2026-09-12 사용자 확인 및 클립 표시 이름 정정

- 사용자가 첨부한 175056 화면과 함께 “잘 나오는거 확인했어 진짜똑같아”라고 서면 확인했다. 이는 원본_갈고리 단일 미리보기에 대한 사용자 관찰이다. 실제 전투 패턴, 소환 개수·배치, 잡기·피해·사운드의 원작 일치 증거로 확대하지 않는다.
- revision 673→674: animationTracks 네 displayName을 각각 Hook_respawn_1, Hook_att_battle_1_01, Hook_att_battle_2_01, Hook_att_battle_3_01로 교정했다. 전체 모션 이름과 실제 clipName, 타이밍, Transform, 개수, 배치는 유지했다. 변경 전후 JSON semantic 비교로 revision과 네 라벨 외 변경이 없음을 확인했다.
- 위치·개수 편집 안내는 기존 Object Detail의 Map Position과 Authored Emissions를 사용한다. Preview at Character는 저장된 Map Position을 대신하므로 월드 배치 확인 시 해제한다. 현재 Lifetime 변경은 clip/key 시간을 함께 재조정하므로 승인된 동작을 유지하는 동시 생성 예시에서는 11334ms와 emission delay 0을 유지한다.
- 기존 authoring/runtime 불일치의 소유권 정리가 안 된 상태이므로 전체 Save/Publish는 계속 보류한다. 이번 변경에 C++ 재빌드 및 Client 자율 실행은 없다.
