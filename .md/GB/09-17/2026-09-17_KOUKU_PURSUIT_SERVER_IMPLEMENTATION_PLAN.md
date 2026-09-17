# 쿠크 카드 추적과 접촉 재생 Server 연결

## G00. 기존 경계와 목표

기존 `CCombatObjectRuntime`의 transaction, snapshot, reliable presentation event와 despawn을 사용한다. 새 Shared packet이나 별도 projectile runtime은 만들지 않는다. `PURSUIT_PROJECTILES` duration Logic은 Server가 선정한 살아 있는 target과 1~4개 카드 visual template을 입력으로 받아 지정한 cadence에 생성한다. contact는 Shared XZ swept circle로 판정하며 피해량은 새로 만들지 않는다.

## G01. 코드와 데이터 흐름

`GameplayCatalog.h`의 mechanic trigger에 visual pool, 접촉 visual ID, 속도(m/s), 반경(m), 생성 반경(m), 수명(ms), cadence(ms), homing과 wave당 수를 추가한다. `CombatObjectRuntime`의 live object는 무한 수명, homing target 및 접촉 event ID를 소유한다. target 사망·퇴장 또는 owner 사망은 소멸하며 명시 Stop은 기존 source cancel을 사용한다. 자연 animation 종료는 발사 완료 후 남은 카드의 수명을 종료시키지 않는다.

`project_kouku_saydon_composition.py`는 Composition의 resource ID를 같은 revision의 targetedCombatVisuals로 변환한다. `Publish-GameplayBalance.ps1`는 이 Client template과 Server 행을 exact join한다. Server bootstrap `PATTERNPURSUITPROJECTILES`는 asset 경로 없이 stable visual ID만 전달한다. Client는 정확한 pursuit archetype에 한정해 contact ID를 pinned Effect asset으로 resolve한다.

## G02. 검증

기존 Server support-surface 계약 검사에 생성 시간, 유한·무한 수명, target 변경 후 이동, swept contact 1회와 cleanup을 추가한다. projector의 유효 입력과 잘못된 reference/range 검사를 수행하고 변경 TU를 컴파일한다. 새 제품 C++ 파일이나 vcxproj/filter 등록은 없다. Client 실행·조작·캡처 및 시각 판정은 수행하지 않는다.

## G03. 원작 회전 카드의 속도와 최대 이동 거리

원패키지의 EFSequenceSummonsProjectile property 순서와 형식으로 카드 Missile의 Speed8m/s, MaxSpeed15m/s, Lifetime5s, MaxDistance15m를 확인한다. 초기 속도와 최대 속도를 최소/최대 거리로 해석한 기존 builder 표기를 교정한다. 가속식은 확인되지 않았으므로 추가하지 않는다.

PURSUIT_PROJECTILES의 optional `maxDistanceM`은0~1000m이며0은 기존 lifetime/contact 정책이다. Client parser·serializer·편집기, Python projection, PowerShell bootstrap, Server admission과 실제 movement cap에 연결한다. lifetime0과 양수 거리 제한의 조합은 거절한다. 기존18-field bootstrap은 유지하고 양수 거리를19번째 field로 추가하며 Server는 두 형태를 읽는다. 현재 P48의 연속 spin 창2833~7633ms는700ms 뒤부터300ms 간격3개씩14wave를 후보로 만든다. 원작4wave 뒤의 연장은 사용자가 요청한 현재 clip 길이에 따른 저작 정책으로 기록한다. 살아 있는 Composition을 저장·publish하지 않고 SHA와 기존 row를 포함한 후보 patch만 만든다.

원작 필드,18/19-field admission·거절 rollback, 초기8m/s 이동과15m 거리 종료·폭발1회,14wave42개, Client의 GAZE_REAL_BOSS 기존 maxDistanceM 보존과 pursuit roundtrip을 격리 검증한다. 제품 빌드·Client/UI 실행은 하지 않는다.
