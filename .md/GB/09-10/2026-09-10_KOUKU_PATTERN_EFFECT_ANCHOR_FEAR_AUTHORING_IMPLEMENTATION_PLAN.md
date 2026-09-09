# 쿠크 패턴 Effect 앵커·공포·저작 편집 구현 계획

작성일: 2026-09-10. 사용자 요청 범위의 구현 전 계약이다. 현재 브랜치는
`codex/kouku-pattern-authoring-effects-fear`이며 시작 시 존재한 대규모 미커밋 변경은 보존한다.

## G00. 현재 호출 경로와 목표

Composition은 `KoukuSaydonComposition.json`의 stable ID로 패턴, Logic, Resource와 occurrence를 저장한다.
Workbench Save는 이 원본을 저장하고 기존 projector와 domain publisher가 Encounter,
patternbindings 및 Server Gameplay.bootstrap을 생성한다. Server 판정은 기존 GameRoom/Logic runtime,
Client 표현은 `KoukuSaydonPresentationPlayer`와 Character/Npc를 계속 사용한다.

Effect occurrence에는 `bone`, `boneTarget`, `anchorKind`, `followBoss`가 있지만 WEAPON은 현재
Collider만 허용한다. V1 Effect는 Composition resource/runtime에 연결되지 않았고 V2 GROUP/LEAF만 소비한다.
조커 P13의 STAGE_5 → STAGE_13 → STAGE_6 전환은 절대 pose seek와 bone CONTACT 정합 때문에
일반 Character blend 상수로 변경할 수 없다. 기존 09-09 자연스러운 전환 PLAN의 opt-in 계약을 두 경계에 적용한다.

이 작업은 이전 턴에서 검토한 서버 전체 Hot Reload 구현을 포함하지 않는다. 필요한 publisher를 완료한 뒤
새 Shared/Server/Client 빌드와 사용자 재시작으로 검증한다. 실행 중 Client/UI를 조작하거나 캡처하지 않는다.

## G01. Effect 앵커와 V1·V2 Resource 소비

Workbench의 Collider BODY/WEAPON/Bone 선택을 Effect에도 제공한다. Composition parser/serializer,
projector, Product reader와 `KoukuSaydonPresentationPlayer::Make_Pivot`가 같은 필드를 소비한다.
머리 레이저는 실제 Large Saydon head bone, 조커 망치는 기존 타격 Collider의 named weapon bone과
offset을 기준으로 연결한다. 없는 bone을 root로 조용히 대체하지 않는다.

V2 기본 목록에는 GROUP만 표시하고 기존 저장 LEAF의 읽기 호환은 유지한다. V1 탭은 기존
EffectCatalog의 authored 문서와 stable element ID를 표시한다. `resourceKind=V1_EFFECT/V1_ELEMENT`,
optional `elementId`로 원본 문서를 참조하며 원본 Effect를 복제하거나 새 renderer를 만들지 않는다.
기존 CEffectObject/PresentationService 실행과 외부 시계·pivot·종료 소유권을 Composition occurrence에 연결한다.

수정 대상은 Workbench H/CPP, MainApp resource refresh, CompositionDocument H/CPP,
Kouku projector, PresentationPlayer H/CPP와 필요한 기존 Effect presentation API다.
한 Effect 또는 선택 element의 준비 실패는 해당 occurrence에 표시하고 기존 정상 항목을 보존한다.

## G02. 이름 변경과 Resource 창 배치

Pattern, Parent, Bundle, Logic과 Composition Resource의 Rename은 현재 표시 이름을 buffer에 복사한 뒤
사용자 Apply에서 displayName만 바꾼다. stable ID와 모든 참조는 유지하며 기존 Save에서 영속화한다.
Large Saydon의 `쿠크_레이저` 표시 이름은 `대형세이튼_레이저`로 변경한다.
Effect 목록의 표시 공간을 약 두 배로 늘리고 하단 Create Resources는 작은 scroll child로 구성한다.

## G03. 서버 권위 공포와 거미카운터 후속 패턴

기존 GAZE_REAL_BOSS의 플레이어 시야 collider 판정을 파1빨2의 세 Duration에 연결한다.
대형 세이튼이 검사 영역에 없을 때 FEAR 결과를 적용한다. 기존 진짜 세이튼 찾기의 FAIL 전멸은 유지한다.
거미카운터 공포는 플레이어 접촉을 판정하는 ENTER_AREA Trigger와 보스 BODY-follow Collider를 사용한다.
OBJECT_CONTACT는 카드 World occurrence 접촉 계약으로 유지하며 선택 오류를 정확히 설명한다.

FEAR 결과는 durationMs=3000, optional sceneProfileId, lightResourceId, effectResourceId, effectDelayMs를 저장한다.
Server는 Result logicId를 presentationId로, action 시작 tick과 fear 종료 tick을 snapshot에 복제한다.
공포 중 이동/스킬을 Server에서 차단하고 종료·사망·퇴장 시 정상 상태를 복원한다.
Character는 실제 모델에 있는 공포 clip을 재생하며 로컬 입력으로 공포 상태를 만들지 않는다.

Client patternbindings의 fearPresentations에는 presentationId, resolved rendering profile ID,
effectResource/lightResource 전체 참조 또는 null, effectDelayMs, durationMs를 투영한다.
로컬 플레이어가 공포에 걸린 동안에만 암전 profile을 적용하고 캐릭터 조명을 유지한다.
얼굴은 기존 V2 SCREEN Effect로 관리하며 `Effect/KoukuSaydon/Textures/FX_TEX_06/fx_g_rpcz_01.dds`를
사용하는 재사용 GROUP을 만들고 거미카운터 Result에 1000ms 지연으로 연결한다.
fullscreen 이미지를 월드 CModel로 위장하지 않는다. Scene Profile과 Effect는 FEAR Result가 함께 참조한다.

거미카운터의 성공 Result는 기존 FOLLOWUP으로 쿠크_그로기를 시작하고 그 종료에서 원 패턴 실행도 끝낸다.
이름만 존재하는 카운터 Duration에는 `COUNTER_WINDOW` 판정을 연결한다. 기존 boss counterable flag와
`CBossCombatRuntime::Try_TriggerCounter`의 COUNTER_HIT 이벤트를 소비해 성공을 확정하며,
HP 감소를 검사하는 STAGGER를 카운터 성공으로 대신하지 않는다.
Shared packet reader/writer, Server catalog/publisher/logic, Client snapshot/Character 표현,
저작 UI/문서 parser와 Product projection을 같은 변경으로 연결한다.

거미 ENTER_AREA에는 optional bossChargeDistanceM=7을 지정한다. Trigger occurrence의 시작과
duration을 최종 한 개 돌진 animation clip에 맞추고, Server는 window 시작 때 살아 있는 target의
방향을 한 번 확정한다. 기존 BossMotion sampler에서 그 방향으로 7m 이동을 시간에 따라 평가하고
navigation/collision을 거친 같은 Server pose로 플레이어 접촉을 검사한다. 이동 중 target을 다시
추적하지 않는다. 새 client local 돌진이나 별도 portal 구현은 만들지 않는다.

중복 돌진 clip 삭제/복제가 막힌 원인은 패턴 전체를 덮는 정지 BossMotion의 시간 구간이었다.
Stage 삽입·삭제 시 BossMotion 시간도 같은 splice로 이동·압축하고 위치/yaw를 유지한다.
최종 사용자가 Save했다고 알려주기 전에는 Composition 원본을 수정하지 않는다.

## G04. 조커의 두 전환과 공 재질

P13의 animation.6 → animation.16 → animation.7 경계만 100ms blendInMs를 명시한다.
Engine의 기존 local bone sampler를 확장해 두 clip/time과 절대 alpha를 평가하며 body와 hammer가
같은 값을 사용한다. Preview/Pause/Seek와 Server action-age Product 모두 동일 계약을 소비한다.
CONTACT와 겹치는 전환은 projector의 기존 bone bake도 같은 보간 pose로 갱신한다.
사용자 카드 위치, Collider 시간, 나머지 전환과 패턴 순서는 보존한다.

공 World Object의 저장 model/texture, 실제 WModel material과 DDS 및 runtime binding을 대조한다.
누락이 확인된 필드만 기존 model/material 경로에 연결하며 임의 단색 텍스처나 별도 shader로 덮지 않는다.
Resources 물리 변경이 필요하면 상대 asset ID와 실제 변경 내용을 RESULT에 남기고 Git에 포함하지 않는다.

## G05. 검증과 완료 경계

기존 focused 검사로 source 저장/재로드/잘못된 참조 거부·기존 상태 보존, projector의 FEAR/앵커/V1 계약,
Shared packet 왕복, Server 공포 종료·입력 차단·후속 패턴과 bone bake 정합을 확인한다.
변경 JSON/PowerShell/XML parse와 scoped git diff --check를 수행한다. Engine public 변경은
Product Engine → Shared → Server → Client 빌드와 SDK 반영을 확인한다. 실행 중 EXE 잠금이면
최소 컴파일 결과와 최종 링크 준비를 구분하고 사용자의 종료 후 최종 링크한다.

기존 파일을 확장하되 실제 소비자로 새 C++ 파일이 필요하면 해당 .vcxproj와 .filters에 함께 등록한다.
원본 Composition은 root가 단독으로 변경하며 쓰기 직전 bytes를 대조해 외부 Save를 덮지 않는다.
Source 저장, Product publish, 새 바이너리 빌드, 사용자 화면 확인은 RESULT에서 별도 상태로 기록한다.
최종 화면은 사용자가 F1 Workbench와 쿠크 아레나에서 직접 확인한다.

## G06. 대형 세이튼 잡기 — 사용자 승인 후 구현 범위

판정 구간과 붙잡힌 상태 유지 구간을 분리한다. 짧은 ENTER_AREA Trigger occurrence에 부채꼴 Collider를
동일 start/duration으로 연결하고 Success에서 CAPTURE_PLAYER Result를 한 번 소비하는 안이다.
별도 DURATION ATTACHMENT_HOLD는 판정과 Success/Fail 없이 상태 종료 시각만 소유한다.
Trigger occurrence의 holdLogicOccurrenceId가 같은 패턴의 Hold occurrence stable ID를 참조하도록 한다.
재사용 Result 정의는 BOSS_LEFT_HAND와 grip offset을 소유하고 특정 패턴의 시간 참조를 저장하지 않는다.

잡기 성공 때 이미 열린 Hold 구간인지 검증하고 그 구간의 원래 끝 tick을 부착 종료로 확정한다.
늦게 잡힌 대상도 같은 끝에 해제한다. Collider 종료는 잡힌 대상을 해제하지 않는다. 대상 중복 획득을
막고 Hold 종료·패턴 Stop/Restart·보스/대상 사망·퇴장 시 기존 Release_PlayerAttachment를 호출한다.
해제와 던지기는 서로 다른 명시 결과로 둔다. 기본 해제에 투사/피해를 숨겨 넣지 않는다.

현재 Server GameRoom의 Capture/Update/Release_PlayerAttachment와 Shared GRABBED/BOSS_LEFT_HAND,
Character의 IPlayerHandGripSocketSource를 재사용할 수 있다. ClientReplication은 현재 CValtan만
이 인터페이스에 연결하므로 CNpc의 실제 BODY bip001-l-hand socket을 연결해야 한다.
대형 세이튼 grip offset은 발탄의 스케일 보정값을 복사하지 않고 해당 모델 기준으로 저작해야 한다.
사용자가 최종 Save와 EXE 종료 후 전체 적용을 요청했으므로 잡기 결과·Hold kind·CNpc 부착까지 구현한다.
최종 원본 revision228을 별도로 보존했다. P17의 짧은 판정은 기존 Collider가 지정한 2029~2817ms에
맞추고, 유지 Duration의 2029~7484ms는 그대로 둔다. 기본 gripLocalOffset은 각 축0으로 실제
대형 세이튼 왼손 socket을 사용하며 Workbench에서 보정할 수 있게 한다.

CompositionDocument와 Workbench는 CAPTURE_PLAYER/ATTACHMENT_HOLD, holdLogicOccurrenceId의
parse/validate/save 및 Duplicate 참조 재발급을 소유한다. projector와 Gameplay publisher는 같은
hold 참조와 slot/offset을 Server bootstrap에 투영한다. patternbindings의 attachmentGrips는
패턴별 한 손/offset을 소유하며 같은 패턴 안의 서로 다른 grip은 거부한다.
Server LogicRuntime은 결과를 capture 요청으로 전달하고 GameRoom이 기존 attachment 함수를 호출한다.
Client Npc는 IPlayerHandGripSocketSource를 구현하고 ClientReplication이 같은 Character API에 연결한다.
새 C++ 파일이나 protocol 추가 없이 기존 파일과 protocol73을 사용한다.
