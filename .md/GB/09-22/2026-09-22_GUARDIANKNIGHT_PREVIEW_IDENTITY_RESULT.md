# Guardian Knight preview identity admission 결과

기준: 2026-09-22, `GB/collider-pattern-bug-fix`, `0ebd23cd1f04a6a65a09125e00ef2ac319ce0d97`.
계획: [구현 계획](2026-09-22_GUARDIANKNIGHT_PREVIEW_IDENTITY_IMPLEMENTATION_PLAN.md).

## G01. 실제 반영

`CharacterSpec.h`의 `Is_ValidEquipmentPresentationPart`를 Model View의
`PlayableCharacterPreviewContract::Stage`와 의상 교체의
`CCharacter::Apply_EquipmentPreview`가 함께 사용한다.

GuardianKnight의 IDENTITY + END + GUARDIANKNIGHT_DRAGON 파츠는 통과한다.
일반 장비는 알려진 kind, HEAD~HANDS, NONE stance를 요구한다. 잘못된 kind/slot/stance,
다른 클래스의 identity 조합은 거부한다. END mask는 0으로 유지하므로 의상 교체가 날개를
가리지 않는다. 실패는 기존 composition/장비 staging 이전에 반환한다.

`Character.cpp`는 수정 전부터 dirty였다. bytes와 SHA-256을
`out/GuardianPreviewFix20260922/before.json` 및 `before/`에 백업하고 위 검증 블록 밖의
bytes가 수정 전과 동일함을 역치환 비교했다. C++의 UTF-8 인코딩과 BOM/CRLF를 유지했다.

## G02. 실행한 검증

- MSVC x64 Debug 분리 컴파일: `Character.cpp`, `PlayableCharacterPreviewContract.cpp`,
  실제 7개 `Logic_*.cpp`와 out 전용 headless 검사 성공. 제품 EXE/DLL은 링크하지 않았다.
- 실제 7개 CHARACTER_SPEC 장비에 공용 admission을 적용해 모두 성공했다.
- 실제 selector 및 CharacterCatalog JSON에서 읽은 입력으로 GuardianKnight와 기존 5클래스의
  `Stage` 성공. Guardian은 장비 6, 무기 1, HUMAN fallback을 보존했다.
- 17개 잘못된 slot/kind/stance 입력에서 실제 `Stage`가 거부하고 기존 OutComposition을 보존했다.
  다른 클래스 identity도 거부하고 날개 END mask 0을 확인했다.
- 기존 equipment authoring suite: 13개 중 12개 성공. 새 Guardian 공용 검사 연결 테스트 성공.
  나머지 1개는 기존 LanceMaster 슬롯 기대 합계 8과 실제 6 불일치다. 수정 전 test 원문을
  현재 동일 소스에 적용해 같은 실패를 확인했으며 무관한 fixture를 바꾸지 않았다.
- 변경 파일 `git diff --check` 성공. 기존 vcxproj/filters 항목과 XML parse 테스트 성공.

headless 실행 결과: `out/GuardianPreviewFix20260922/verify.log`.
재실행: `powershell -ExecutionPolicy Bypass -File out/GuardianPreviewFix20260922/compile.ps1`.
기존 dirty 보존 검사: `python out/GuardianPreviewFix20260922/check_preservation.py`.

## G03. 분리해서 발견한 기존 문제와 미검증

GunSlinger는 장비 admission 자체는 성공하지만 실제 Stage의 기존 무기 수 검증에서 거부된다.
`Logic_GunSlinger`는 같은 모델을 양손에 붙이는 무기 part 2개이고 CharacterCatalog JSON은
무기 resource 1개이므로 inventory 수와 part 수의 동등 비교가 맞지 않는다. 이번 identity
수정으로 발생한 문제가 아니며 통합 담당에게 별도 전달했다. 실제 seven-class Stage 전체 성공으로
기록하지 않는다.

제품 전체 Build/Client 실행, GPU 표시, 클래스 선택과 의상 교체 화면 확인은 이 작업에서
실행하지 않았다. 통합 담당의 빌드와 사용자의 화면 확인이 남아 있다. ghost/stone 수정은
다른 작업 범위다. 공통 gotchas/렌더링 가이드 갱신 문구는 통합 담당에게 전달한다.
