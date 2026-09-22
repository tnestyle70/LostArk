# Monster Composition 및 피해 전용 공격 결과

## G00. 반영한 소스와 데이터

- Composition Actions → Character의 Character Actions와 Clown/interaction/mount를 각각 접고 펼친다. 같은 위치에 Monster를 추가했다.
- 발탄 일반 4종·루가루, 쿠크 시작 구역 4종, 카드미로 병정 4종의 13 archetype을 등록했다. 설치 WModel 전체 462 clip을 선택할 수 있고 제품 attack 29개는 catalog와 연결했다.
- 기존 CharacterPreviewPanel → CModel → EffectAuthoringSequencer를 사용한다. 배율과 yaw는 MonsterCatalog 값으로 맞췄고, SJFC elite의 0.0115 배율도 독립 descriptor로 등록했다.
- clip 선택 시 MonsterProfiles를 다시 읽어 body 원과 일반 공격의 `collisionRadius + attackRange` 원을 함께 stage한다. 일반 공격은 windup 뒤 첫 active tick 한 번만 표시한다. 카드미로 참고 공격 clip에는 일반 MonsterBrain 판정을 붙이지 않는다. 미니게임 접촉과 대상 처치는 기존 카드미로 controller 소유다.
- 모델/clip 실패는 상태 문구로 표시하며 다른 몬스터의 마지막 목록을 유지한다. Monster preview의 Save Effect Sequence는 제품 combat 수치를 저장하지 않는다.
- 대상 13 profile의 `attackPushRangeM`, `attackPushMs`, `attackKnockdown`, `attackDownMs`를 `0/0/false/0`으로 맞췄다. 대상 중 이미 0인 카드미로 값은 그대로다. 몬스터가 플레이어에게 맞는 `hitKnockbackScale`, Mario profile, Valtan/Kouku boss는 변경하지 않았다.
- MonsterBrain 일반 공격에서 HP 직접 차감 후 공용 피해 함수를 다시 호출하던 중복을 제거했다. 이제 `CServerCombatHitRuntime::Apply_WorldToPlayer`가 피해·이벤트·반응을 한 번 처리한다.

## G01. 실행한 검사

| 검사 | 결과 |
|---|---|
| 13 대상 WModel 바이트 구조·필수 presentation/attack clip 확인 | 462 clip, 29 catalog attack 존재 |
| clip 시간 유한값·상한 | 통과. 실제 엔진의 COOKED_TICK_RATE=30 기준 사용 |
| profile 필드 보존 대조 | 대상 공격 반응 4필드 외 값 보존 |
| JSON 교체 | `out/MonsterComposition20260922/MonsterProfiles.before.json` 백업, 최신 원본 bytes 재검사 뒤 atomic replace |
| 기존 World publisher | scratch Publish 종료 0, 대상 13 profile의 생성 반응 필드 일치 |
| 수정 파일 git diff --check | 통과 |

scratch 게시 위치는 `out/MonsterComposition20260922/ServerWorld`와 `ClientWorld`다.
상위 통합 작업에서 공식 World publisher live Publish도 종료0으로 완료했다. 실제 Valtan/Kouku/
Character Select spawngroupsbootstrap3개의 대상 공격 반응 필드에 반영됐고 boss는 보존했다.

일부 카드미로 clip의 저장 tick rate가 30과 다르지만 런타임 CAnimation은 30을 사용한다.
이번 preview도 동일한 엔진 clock을 사용하며 리소스 바이너리는 변경하지 않았다.

## G02. 실행하지 않은 검사와 사용자 경로

제품 컴파일·링크는 상위 통합 Product Debug에서 통과했다.
기존 CharacterAdmission 검사에 한 번의 일반 공격이 HP를 한 번만 깎고 damage event 한 개를 만들며
push/down을 발생시키지 않는 단언을 추가했다. SpawnGroups의 게시 profile 기대값도 피해 전용으로 바꿨다.
actual Server object·게시 bootstrap focused probe에서 피해1회·event1개·push/down없음을 실행해
확인했다. 통합20/20 결과와 광역검사의 별도 기존 Kouku fixture 실패는 상위 RESULT에 구분했다.

Client/UI 실행·화면 캡처·육안 판정은 하지 않았다. 통합 빌드와 World publish 후 사용자가
F1 → Action Workbench → Composition Actions → Character → Monster에서 모델과 clip을 선택하고
Preview Play, timeline seek, body/attack collider 표시를 확인한다. 제품 damage-only 적용은 새 Server 실행이 필요하다.
