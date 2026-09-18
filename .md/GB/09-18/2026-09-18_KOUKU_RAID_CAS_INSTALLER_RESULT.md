# 쿠크 통합 후보 CAS 설치 도구 결과

## G01. 입력과 허용 범위

`Tools/KoukuSaydonPipeline/install_raid_candidate.py`는 준비된 manifest를 검토하거나 설치한다. `--candidate-root`는 `manifest.json`, `candidate/`, `baseline-current/`가 들어 있는 묶음 디렉터리다. 기본 동작은 dry-run이고 실제 교체에는 `--install`이 필요하다. 이 도구는 projection, 자동 merge, domain publish, Client Reload 또는 프로세스 제어를 실행하지 않는다.

필수 Data 경로는 Action Composition, Sequence Composition, `Data/Worlds/LV_LUT_MIDNIGHTC_ED/Gameplay.world.json`, CharacterSoundCatalog 네 개다. 추가로 architecture 담당이 확인한 `Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json`만 optional로 허용한다. 그 외 Data나 runtime generated 파일은 거절한다. 미디어는 manifest에 적힌 `Client/Bin/Resources/Sound/` 아래 WAV만 허용하고, 동일 경로의 동일 SHA는 한 번만 설치한다. 같은 경로의 다른 SHA는 거절한다. 기존 음원이 같은 SHA이면 유지하고 다른 SHA이면 덮어쓰지 않는다.

모든 설치 경로는 상대 경로 형식과 `Path.resolve()` 후 repository 내부 여부를 함께 검증한다. candidate/baseline도 각 묶음 안에 있어야 하며, 미디어 source는 candidate 또는 repository `out`에 있어야 한다. 최종 파일 symlink와 외부 junction 탈출을 거절한다.

## G02. 저장 경쟁과 실패 처리

baseline bytes의 SHA와 revision, candidate revision을 manifest와 대조한다. 현재 저장본이 baseline 또는 이미 설치된 동일 candidate와 다르면 설치하지 않는다. stale 저장본은 stable ID 기준 변경 필드와 같은 필드 충돌 목록을 보고한다. 무관한 사용자 변경도 설치기가 자동으로 합치지 않으며 최신 저장본에서 후보를 다시 만드는 경계다.

설치 전 `out/transactions/kouku-raid-<uuid>/backup`에 원본을 보존한다. 각 대상의 같은 디렉터리에 transaction 전용 stage를 만들고 fsync/해시 검사한다. 미디어, SoundCatalog, 나머지 Data 순서로 반영하며 각 교체 직전에 manifest, source, stage, 현재 대상 해시를 다시 확인한다.

기존 파일은 Windows `ReplaceFileW`로 원자 교체하면서 실제 교체된 원본도 함께 capture한다. 마지막 해시 검사 직후 사용자 저장이 끼어들면 capture SHA로 이를 발견하여 해당 저장본을 복원한다. 새 음원은 같은 볼륨 hardlink의 create-if-absent로 생성하므로 동시 생성 파일을 덮어쓰지 않는다.

실패하면 역순으로 rollback하되 현재 bytes가 자기 설치 SHA일 때만 복원한다. 이미 다른 bytes로 저장되었다면 그대로 보존한다. rollback 자체와 저장이 경쟁하면 가장 최근 capture가 우선하며, 반복 경쟁이나 복구 실패는 receipt와 capture backup에 남긴다. 신규 파일 정리도 먼저 transaction 임시 이름으로 capture하고 자기 bytes인지 확인하므로 검사 직후의 저장을 무조건 지우지 않는다. transaction 소유 temporary만 삭제하며 원본 backup/receipt는 보존한다.

이는 파일별 원자 교체와 실패 시 보상 rollback이다. 여러 파일을 한 번의 filesystem operation으로 동시에 바꾸는 구조는 아니다. 도구가 강제 종료되거나 저장장치가 실패한 경우 transaction backup/receipt를 사용하는 수동 복구가 필요할 수 있다.

## G03. 검토 보고서와 사용 명령

보고서에는 Data SHA/revision, stable ID 변경 필드, 추가 Composition resource/occurrence, SoundCatalog event/asset, 중복 제거한 미디어 목록과 크기를 기록한다. `--report`는 repository `out` 아래 JSON만 허용하고 manifest/candidate/baseline/설치 대상 자체를 덮어쓰지 못한다.

검토 명령:

```powershell
python Tools/KoukuSaydonPipeline/install_raid_candidate.py --candidate-root out/KoukuRaidIntegration20260918 --repository-root . --dry-run --report out/KoukuRaidIntegration20260918/install-review.json
```

최종 후보 검증 이후 통합 담당이 실행할 설치 명령:

```powershell
python Tools/KoukuSaydonPipeline/install_raid_candidate.py --candidate-root out/KoukuRaidIntegration20260918 --repository-root . --install --report out/KoukuRaidIntegration20260918/install-result.json
```

이 작업에서는 위 실제 후보 명령 두 개를 실행하지 않았다. 특히 Live 설치는 하지 않았다.

## G04. 실행한 검증

`test_install_raid_candidate.py`는 실제 저장소 `out/KoukuRaidInstallerTests` 안에 격리 repository를 만든다. Windows `ReplaceFileW`를 실제 호출하는 15개 검사가 통과했다. 정상 설치/백업/idempotent 검토, optional WorldSequence, 기본 dry-run 무변경, 같은 필드 stale 충돌, 음원 중복 SHA, 경로 탈출, junction 탈출, 중간 실패 rollback, 설치 후 사용자 저장 보존, 교체 직전 저장 보존, 해시 검사와 교체 사이의 저장, rollback 도중 저장, 신규 음원 동시 생성, 신규 파일 정리 중 저장, 보고서의 manifest 덮어쓰기 거절을 확인했다. 일부 검사는 하나의 test 함수 안에서 여러 경계를 검사한다.

Python compile와 변경 파일 `git diff --check`가 통과했다. 증거는 `out/KoukuRaidInstallerTests/verification.json`과 보존한 test source다. 새 C++ 파일과 project/filter 등록은 없다. 최종 manifest가 계속 갱신 중이므로 실제 후보의 fresh dry-run/설치/publish와 Client Reload는 통합 담당의 후속 단계다.
