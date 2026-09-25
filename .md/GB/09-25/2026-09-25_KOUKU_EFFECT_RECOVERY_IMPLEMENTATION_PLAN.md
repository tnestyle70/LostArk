# 쿠크 이펙트·본 부착 복구 구현 계획

## G00. 현재 기준

`codex/kouku-timeline-local-preview`의 1ddbcffc와 사용자가 저장한 현재 Composition을 기준으로 한다. 기존 미커밋 변경과 렌더링 옵션은 보존한다. Data 문서의 적용은 통합 담당자가 최신 저장본을 다시 읽고 stable ID/필드 병합으로 수행한다. 이 담당자는 `out/KoukuEffectRecovery20260925`에 후보와 검증을 준비한다.

## G01. 실제 소비 이펙트

회전카드는 `logic.74`의 네 suit 문서가 소비자다. 원본 보라 SpawnPerUnit 요소는 현재 네 문서에 존재하므로 존재 여부만으로 복구를 선언하지 않고 실제 moving root와 lifetime/alpha/axis를 확인한다. 주사위 카드출력의 원본 본 부착에 occurrence 회전이 적용되는지 확인하고 해당 요소에만 기존 followEmitterAxisRotation을 연결한다. 메두사의 원본 action42198102 발생 요소와 현재 합본19요소를 비교하고 누락된 붉은 외곽의 원본 system을 찾는다.

## G02. 모델과 본의 소비

노란시선 sourceModelPreview는 MN_RPCT_06/GATE2를 지정한다. 현재 preview model 준비 실패 분기를 확인해 기존 Prototype/CNpc 경로로 복구한다. 바주카는 LaserCannon.wmodel의 원본 두 재질 슬롯과 BossCatalog native override를 비교하여 같은 CModel/CMaterial 입력을 연결한다. 백스텝 Collider는 실제 animation bone basis를 기존 publisher/Client 소비자에 연결하며 Server 판정을 Client Transform으로 대신하지 않는다.

## G03. 검증

새 C++ 파일 없이 기존 경로를 좁게 수정한다. 후보 JSON과 Resources 참조를 검사하고 기존 native Codec/Playback 및 실제 WModel 본 샘플로 회전·위치·입자 수를 측정한다. 통합 Product 컴파일은 root가 수행한다. Client 실행·화면 캡처는 하지 않으며 사용자가 최종 색·형태를 확인한다.
