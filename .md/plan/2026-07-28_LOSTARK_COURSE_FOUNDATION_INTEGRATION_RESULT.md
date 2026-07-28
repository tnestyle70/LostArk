# Session - 21일차 수업 프레임워크 기반 저장소·필터·외부 도구 통합 결과
좌표: LA-F00 · 축: C2 이동>계산, C4 수명은 선언된다
관련: `2026-07-28_LOSTARK_COURSE_FOUNDATION_INTEGRATION_PLAN.md`

## 1. 의도 vs 실전

- 일치: 기존 수업 `Framework.sln`과 Engine/Client project는 건드리지 않고 저장소 ignore 기준만 추가했다.
- 일치: 실제 LostArk, Winters, Dungeons의 project/filter/third-party 배치를 읽고 CMake 전환을 보류했다.
- 발견: 초기 상태에는 Git LFS 정책이 없고 candidate tracked content가 544개/약 325.88 MiB였다. 첫 commit 준비에서 `.gitattributes`를 추가해 asset/vendor binary를 LFS pointer로 stage했다. 재배포 allowlist 전에는 push하지 않는다.
- 빗나감: Debug x64 build는 성공했지만 기존 빌드가 `pwsh.exe` 미발견 메시지를 성공 뒤에 남긴다. MSBuild 전처리 결과 전역 vcpkg applocal target의 PowerShell 7 탐색이며 Windows PowerShell fallback이 이어짐을 확인했다. 실행 smoke는 아직 하지 않았다.

## 2. 판정

수정 반영. F00은 부분 완료이며 commit 가능 상태는 아니다.

- 완료: `.gitignore` 작성.
- 완료: `.vs`, `x64`, `EngineSDK`, exe/dll/lib/pdb/cso ignore 확인.
- 완료: 향후 local extraction/cook output인 `_work`, `out` ignore 추가.
- 완료: FBX/DDS/TGA/BMP/image/font/data/W-format/audio와 required vendor lib/DLL LFS pattern 적용.
- 완료: source, `.sln/.vcxproj/.filters`, runtime Resources/DataFiles/ShaderFiles 노출 확인.
- 완료: `Framework.sln / Debug / x64` build exit code 0.
- 완료: 전체 roadmap 독립 비평 3회, 최종 P0=0/P1=0 통과.
- 완료: initial baseline용 `.gitattributes`와 LFS pattern 확정.
- 미완료: 외부 공개 push를 위한 asset/vendor provenance·redistribution allowlist.
- 미완료: 새 작업 폴더 clone 기준 build/run.
- 미완료: LostArk 전용 project/filter 생성기.
- 미완료: ImGui, FMOD, AssetConverter.

## 3. 역갱신

- 새 정본 결정: 현재 build owner는 legacy MSBuild다. CMake를 병행하지 않는다.
- 새 팀 규칙: `.vcxproj`의 설정·고정 item은 수동 정본, 관리 source block은 물리 tree 정본이다. `.filters`는 Git 추적하되 사람이 편집하지 않는다.
- 새 실행 순서: clean-clone build → filter 자동화 → 기존 Winters W-format reader → `Resource_LostArk` 모델/animation render → Converter → ImGui → FMOD.
- 다음 시작점: F00 별도 PLAN에서 LFS/재배포 allowlist와 `BuildDebug.bat`의 전체 내용을 먼저 제시한다.

## 4. 검증 기록

```text
명령:
MSBuild.exe Framework.sln /m /p:Configuration=Debug /p:Platform=x64 /verbosity:minimal /nologo

결과:
exit code 0
Engine/Client output 생성 확인
FXC deprecation, 수치 변환, third-party PDB 경고 존재
pwsh.exe 미발견 메시지 존재
```

```text
명령:
git status --short --ignored
git lfs version

결과:
.gitignore가 의도한 생성물을 ignored로 분류
git-lfs/3.5.1 설치 확인
stage/commit/push 없음
```
