# 쿠크 탄도 밀치기 Client 문서·편집 연결

## G01. 변경 범위와 계약

`KoukuSaydonCompositionDocument.h/.cpp`의 damage Result에 optional `pushBallistic`을 추가한다. 기본 false는 기존 일반 밀치기를 유지한다. 일반 거리는0..20m, 시간은 기존 MAX_TIME_MS 이내이며 거리와 시간은 함께0이거나 함께양수다. 탄도는 양수거리100m 이하,100..5000ms,`pushCanLeaveArena=true`를 요구한다. `pushDirection`은 `AWAY_FROM_BOSS`, `BOSS_FORWARD`, `AWAY_FROM_CONTACT`를 받으며 마지막 값은 Server의 실제 접촉중심을 소비한다. yaw offset은 양수 `BOSS_FORWARD`에만 허용한다. 기존 damage Result 소유권과 실패 시 기존 문서 보존을 유지한다.

`KoukuSaydonActionWorkbench.h/.cpp`의 Collider 빠른 편집 설정에 forcePush, pushCanLeaveArena, pushBallistic, pushYawOffsetDegrees를 모두 보존한다. 기존 Result에서 읽기, 동일 Result 재사용 판정, 신규 Result 생성에 같은 필드를 연결한다. 다른 결과의 정책을 일부 필드만 비교해 재사용하지 않는다. 일반 Result 편집과 Collider 편집 모두 탄도 여부에 맞는 거리·시간 한계 및 세 번째 방향을 표시한다. 탄도 선택은 아레나 이탈을 켜고, 밀치기 해제는 모든 종속 옵션을 해제한다.

## G02. 반영과 검증

기존 네 파일의 UTF-8 BOM 없음·CRLF와 무관한 미커밋 변경을 보존하며 out에 기준본을 백업한다. 새 제품 파일과 프로젝트 등록은 없다. Data/Server/Python/PowerShell 계약은 부모 작업자가 소유한다.

실제 native codec의 parse/validate/serialize/roundtrip에서 생략 기본값, 정상 탄도 경계, 잘못된 범위·시간·방향·필드소유권, 실패 후 기존 문서 보존을 확인한다. Collider 편집의 정책 유지·재사용은 실제 함수 코드와 대응 native 검증으로 확인하며 변경 TU를 scratch 컴파일한다. Product 빌드·publisher·Client UI·Server 실행은 이 하위 작업에서 수행하지 않는다.
