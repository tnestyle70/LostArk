# Guardian Knight preview identity admission 구현 계획

기준: 2026-09-22, `GB/collider-pattern-bug-fix`, `0ebd23cd1f04a6a65a09125e00ef2ac319ce0d97`.
기존 dirty 변경을 보존하고 사용자가 승인한 가디언 선택/의상 preview 실패만 수정한다.

## G01. CharacterSpec.h의 identity presentation 계약

`EQUIPMENT_PART_SPEC` 바로 뒤에 공용 constexpr 검사를 둔다. 일반 장비는 알려진 kind,
HEAD~HANDS, NONE stance만 허용한다. 현재 설치된 identity 계약은 GuardianKnight의
DRAGON 전용 날개이며 IDENTITY + END로 선언된다. 이 조합은 의상 replacement mask를
소유하지 않으므로 허용하고 잘못된 kind, slot, class, stance는 거부한다.

## G02. 두 preview 소비자 연결

`PlayableCharacterPreviewContract::Stage`의 장비 검증과
`CCharacter::Apply_EquipmentPreview`의 default equipment 검증에서 같은 helper를 호출한다.
Stage 실패 시 OutComposition 보존과 장비 preview 실패 시 기존 part 보존은 유지한다.
날개 정의, 의상 mask, Server class authority, visibility/stance 소비는 변경하지 않는다.

## G03. 검증과 전달

기존 equipment authoring contract test에 GuardianKnight의 다섯 의상 슬롯과 END 날개,
두 소비자의 공용 검사 연결을 포함한다. 실제 C++ Stage를 사용하는 작은 headless 검사에서
정상 Guardian composition과 잘못된 slot/kind/stance 및 실패 시 기존 composition 보존을 확인한다.
변경 translation unit은 분리된 out 경로로 최소 컴파일한다.

새 제품 C++ 파일이 없으므로 기존 vcxproj/filters 등록은 그대로 사용한다.
수정 전 bytes와 SHA-256은 `out/GuardianPreviewFix20260922`에 보존한다.
제품 전체 빌드/Client 실행과 화면 확인은 통합 담당 및 사용자 경계다.
결과는 대응 RESULT에 실제 실행한 검증만 기록한다.
