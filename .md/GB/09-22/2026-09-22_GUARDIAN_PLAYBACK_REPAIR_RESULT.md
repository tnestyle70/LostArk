# Guardian 재생·화신화·ALT V 연결 결과

## G00. 실제 반영 범위

Z 화신화의 최대 identity 진입 조건을 제거했다. 빈 게이지에서도 기존 Server command로 요청하며, 실제 stance commit에서 identity와 Ember orb를 채운다. 기존 3초 공통 cooldown, action/stance 검사와 15초 변신 지속·해제는 유지한다. Client에서 게이지나 stance를 임의 변경하지 않는다.

Full Restore Play All의 COMBO stage는 선택한 실제 모델의 skillbindings와 animevents를 직접 대조한다. 전체 Effect tree의 다른 class enrichment 실패 때문에 ProductCues가 비어 있어도 선택한 LMB의 exact clip/stage를 조회한다. 모델 선택 중 catalog reload가 발생하므로 이전 skill 포인터를 재사용하지 않는다. saved Effect load 실패, disabled 버튼 이유, 재생 시작 이후의 실패는 현재 Effect 목록에 표시한다. 모델 시간·root·factory가 실패하면서 이유를 남기지 않던 경로도 상태 문구를 남긴다.

ALT V의 opaque background mesh 7개에 기존 `compositionLayer=sceneBackdrop`를 연결했다. 실제 해당 carrier가 활성인 구간에 기존 공통 배경 숨김을 사용한다. 최신 JSON bytes를 백업하고 해당 7필드만 바꾸어 원자 교체했다. camera source basis와 Close-up/Zoom out row는 [별도 결과](2026-09-22_GUARDIAN_ALTV_CAMERA_RESULT.md)를 따른다. gameplay에서만 용이 90도 달라 보이는 원인은 아직 재현·확정하지 못했으며 전체 용/particle에 회전을 강제하지 않았다.

## G01. 연결한 다른 작업

- [Monster 결과](2026-09-22_MONSTER_COMPOSITION_DAMAGE_ONLY_RESULT.md): Character/Clown/Monster 접기, 13종 실제 모델·462 clips·공격 collider, 공격 피해 1회 및 push/down 제거.
- [Gate3 계획](2026-09-22_GATE3_WORLD_AURA_IMPLEMENTATION_PLAN.md): 원본 진입/활성/리스폰 오라와 기존 typed 입장 경로.
- [Guardian shader/carrier 계획](2026-09-22_GUARDIAN_CARRIER_RENDER_REPAIR_IMPLEMENTATION_PLAN.md): native Trail/Decal 처리 범위·UV/접선·masked mesh 원본 입력 복구.

## G02. 검증 상태

World publisher를 live `Publish`로 실행했고 종료코드 0이다. runtime diff는 Valtan/Kouku/Character Select의 spawngroupsbootstrap 3개이며 대상 몬스터의 공격 push/down 4필드가 반영됐다. boss profile과 몬스터 자신의 피격 반응은 보존했다. Server 재시작 이전의 실행 메모리는 갱신되지 않는다.

빈 identity 변신 승인·commit 전 게이지 보존·commit 후 full gauge·15초 해제는 기존 PlayerActions fixture를 갱신했다. Gate3 aura가 entry 중심만 허용하고 respawn 중심·아래층·NaN을 제외하는 기존 Server fixture 단언도 추가했다. 단언 추가와 실행 통과는 구분한다.

첫 Product Debug 실행에서 Engine/Shared/Server 컴파일은 통과했다. Client는 신규 오라 Data 등록이 ProjectReference의 GUID 안에도 삽입된 MSB3107로 실패했다. 해당 두 삽입만 제거하고 등록기는 마지막 root closing tag만 사용하도록 수정했다. 프로젝트 XML parse와 두 reference의 GUID·childcount=0 검사는 통과했다. 수정 후 Client 재빌드는 진행 전이다.

Server 계약 검사는 진행 중이며 기존 KoukuProduct fixture의 7개 연쇄 실패를 확인했다. HEAD와 현재의 Gameplay.world.json 및 게시 worldbootstrap은 모두 g1kouku/g1saydon을 disabled로 둔다. 변경하지 않은 Is_ArenaBossPlacement는 두 배치를 모두 허용하지만 기존 fixture는 Kouku만 허용하지 않는다고 가정한다. 이번 World publish에서 해당 worldbootstrap의 diff는 없다. 이 실패를 이번 기능의 통과로 숨기거나 제품 데이터를 과거 fixture에 맞추지 않는다.

Client 실행·UI 조작·화면 판정은 하지 않았다. 최종 빌드 결과와 남은 경계는 검증 후 갱신한다.
