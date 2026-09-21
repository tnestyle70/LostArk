# 쿠크 연출 애니메이션 Character 본체 통합 구현 계획

## G00. 현재 실측과 범위

현재 Character 본체는 쿠크 `MN_RPCZ_00` 91개, 세이튼 `MN_RPCT_05` 249개, 큰 세이튼 `MN_RPCT_06` 34개 클립이다. 쇼타임 `rpct00_evt2_rpct_showtime_01`은 세이튼 본체에 이미 있다. Map에 분리된 보스 연출 모델 11개의 skeleton과 material section은 대응 Character 본체와 바이트 단위로 같다. 1관문 통합 3개, 2관문 진입 2개, 2관문 클리어 3개, 카드미로 1개, 3관문 진입 1개, 빙고 앵콜 1개를 모두 본체에 추가한다.

## G01. 영속 변환 도구

`Tools/KoukuSaydonPipeline/bake_character_cinematic_clips.py`는 donor와 본체의 안정 asset ID를 명시하고 기존 Valtan section append 구현을 재사용한다. 소스 도구가 이미 완성한 WANM을 그대로 옮기므로 좌표·단위·채널·시간을 다시 변환하지 않는다. 본체의 최신 geometry, material, skeleton, 기존 animation section은 모두 바이트 단위로 보존한다. 같은 이름·같은 내용은 건너뛰고 같은 이름·다른 내용은 거부한다. 후보는 `out/KoukuCharacterCinematicBake20260921/candidates/Character/...`에 만들며 실행 중 Resources는 직접 교체하지 않는다.

도구는 현재 WorldSequence 문서가 이미 Character 참조로 바뀌어도 같은 donor manifest로 다시 실행할 수 있어야 한다. source bake 도구가 저작 참조를 다시 Map 보스 donor로 연결하지 않도록 연동을 확인한다. 책·테이블·카드 등 환경 소품은 대상이 아니다.

## G02. 검증과 통합 경계

후보의 원본 section 보존, 11개 WANM 내용 동일, skeleton 동일, 모든 키의 유한성·시간·quaternion을 확인한다. 두 번째 실행 결과의 동일성과 충돌 거부를 검사한다. 실제 CModel WARP 무창구 실행으로 11개 클립 전체 프레임의 bone combined matrix를 donor와 대조한다. Client·아레나 실행과 화면 판정은 수행하지 않는다.

Resources 설치는 원본 hash를 재확인하는 최종 통합 담당자가 수행한다. Data 참조와 BossCatalog, publisher, C++ 로딩 실패 수정은 같은 작업의 다른 담당 범위다. 새 C++ 제품 파일은 추가하지 않으므로 `.vcxproj/.filters` 등록이 필요 없다. 결과 문서는 실제 후보·검증과 설치·게시 상태를 분리한다.
