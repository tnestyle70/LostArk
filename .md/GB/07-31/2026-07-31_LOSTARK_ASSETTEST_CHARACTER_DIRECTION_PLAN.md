# LostArk AssetTest 캐릭터 연결 방향 계획서

- 작성일: 2026-07-31
- 문서 유형: 구현 전 방향 문서
- 우선순위: NavGrid 수동 편집 완료 뒤 진행
- 이번 범위: 현재 코드의 재사용 경로와 구현 순서만 고정한다. 코드는 NavGrid 검증 후 작성한다.

## 1. C1~C8 관점

| 관점 | 확인한 사실 | 중요도 |
|---|---|---:|
| C1 기준계 | 캐릭터와 발탄은 같은 AssetTest 월드 좌표와 NavGrid X/Z를 사용해야 한다. | ★★★ |
| C2 이동>계산 | skeleton/equipment/weapon cook는 Loader 전에 끝나고 런타임은 Prototype clone과 bone matrix 결합만 수행한다. | ★★★ |
| C3 공유는 비싸다 | body skeleton palette는 equipment가 공유하되 animation clock과 이동 경로는 캐릭터 인스턴스가 소유한다. | ★★★ |
| C4 수명은 선언된다 | Loader는 Prototype, Level은 실제 Character clone과 spawn 위치, Character는 part와 follower를 소유한다. | ★★★ |
| C5 이산화와 오차 | 피킹 좌표는 NavGrid cell로 변환되고 최종 Y는 cell height를 사용한다. | ★★★ |
| C6 가지치기 | 예전 `CPlayer/CBody_Player/CWeapon` 경로를 AssetTest에 다시 연결하지 않는다. | ★★★ |
| C7 권위와 정합성 | 클래스 차이는 `CHARACTER_SPEC`, 실제 배치는 `CHARACTER_DESC`, 이동 가능 여부는 최종 `.navgrid`가 정본이다. | ★★★ |
| C8 검증이 병목 | body·방어구·무기 socket, idle/run, 클릭 A*, 정면 방향을 각각 눈으로 확인한다. | ★★★ |

## 2. 문제 해결 ①~⑤

① 문제·제약: 새 `CCharacter + CHARACTER_SPEC + CPart_Body + CPart_Equipment`는 Test2에만 연결돼 있고 AssetTest 클릭은 현재 발탄을 움직인다.

② 단순 해법의 문제: 예전 `CPlayer`를 복사하면 두 캐릭터 조립 경로가 생기고, Character 안에서 마우스를 읽으면 Level 입력 책임과 섞인다.

③ 해결 방식: Loader는 AssetTest에 LanceMaster 모델·파트·Character Prototype을 등록하고, Level이 기존 `CHARACTER_DESC::vPosition`으로 발탄 앞에 spawn한다. Level의 클릭 피킹 결과를 Character의 NavPathFollower 진입점에 전달한다.

④ 비교: 기존 Test2는 조립과 렌더만 검증한다. AssetTest는 동일 조립 경로에 NavGrid와 click-to-move만 추가한다.

⑤ 대가: Character에 Navigation component와 follower 상태가 추가된다. 보스 target/chase는 이 단계와 섞지 않는다.

## 3. 자료구조·알고리즘 핵심

- `CHARACTER_SPEC`: body/equipment/weapon model tag, socket bone, animation clip 이름. 클래스별 불변 정의다.
- `CHARACTER_DESC`: Level이 clone할 때 전달하는 prototype level, spec, spawn position, navigation tag다.
- `CPart_Body`: skeleton과 animation clock의 owner다.
- `CPart_Equipment`: skinned 장비는 body bone palette를 읽고, weapon은 `b_weapon_rhand` bone matrix를 읽는다.
- `CNavPathFollower`: Character 인스턴스의 waypoint와 next index를 소유한다.

```text
Loader Prototype 등록
 -> Level_AssetTest가 CHARACTER_DESC로 spawn
 -> CCharacter::Ready_PartObjects
 -> body 생성과 CModel 확보
 -> armour는 skeleton palette 공유
 -> weapon은 right-hand socket 추적
 -> Level 좌클릭 Picking
 -> Character Request_Move
 -> Navigation A*
 -> follower가 Transform 이동
 -> idle/run clip 선택
```

## 4. 현재 코드 기준 결론

- Loader에 asset/skeleton/equipment/weapon을 등록하는 방향이 맞다. Loader는 spawn하지 않는다.
- spawn은 `CLevel_AssetTest`가 담당한다.
- 새 spawn 위치 필드는 필요 없다. `CCharacter::CHARACTER_DESC::vPosition`이 이미 존재한다.
- 피킹 입력은 `CLevel_AssetTest::Update_ClickMove`가 담당하고, `CCharacter`는 world goal만 받는다.
- weapon transform은 임의 offset부터 넣지 않는다. 현재 `body bone matrix × character parent world` 경로를 먼저 실측한다.
- 현재 `CValtan`은 `pTargetTransform`을 받으면 0.35초 간격으로 재탐색해 추적한다. 캐릭터 이동 검증 단계에서는 target을 연결하지 않는다.

## 5. 후속 수정 파일

| 파일 | 후속 역할 |
|---|---|
| `Client/Public|Private/Loader.*` | Test2의 LanceMaster Prototype 등록을 AssetTest에서도 재사용 |
| `Client/Public|Private/Character.*` | Navigation clone, Request_Move, follower, 이동 상태 |
| `Client/Public|Private/Level_AssetTest.*` | Character spawn, 클릭 대상 전환, 검증용 위치 |
| `Client/Private/Logic_LanceMaster.cpp` | 이동 상태에 따른 idle/run 선택이 필요할 때만 수정 |

새 파일은 현재 예상하지 않는다. 따라서 `.vcxproj/.filters` 변경도 현재 예상하지 않는다.

## 6. 적용 순서와 완료 게이트

1. NavGrid F1 paint/save와 최종 `.navgrid`를 먼저 검증한다.
2. AssetTest Loader에 기존 Character Prototype 세트를 등록한다.
3. 발탄보다 카메라 쪽의 검증된 walkable cell에 Character를 spawn한다.
4. body→equipment→weapon 조립과 socket을 정지 화면에서 확인한다.
5. 좌클릭을 발탄이 아닌 Character 이동으로 연결한다.
6. 직선·우회 이동, 정면, idle/run을 확인한다.
7. 그 다음 별도 단계에서만 Valtan target transform을 Character에 연결한다.

## 7. 이번에 하지 않는 것

- 발탄 AI와 공격 상태.
- skill/effect/hit window.
- 장비 교체 UI와 inventory.
- socket 보정용 magic transform.
- 예전 `CPlayer` 경로 정리.

