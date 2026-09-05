# 2026-09-06 KoukuSaydon 관문 1·3 거부 수정, 뿅망치 배율·회전, 광기 수치 확장, 광대 아바타 교체 결과

> 계획서: [2026-09-06 계획서](2026-09-06_KOUKU_SAYDON_GATE_FIX_HAMMER_MADNESS_CLOWN_AVATAR_IMPLEMENTATION_PLAN.md)
>
> 상태: 후속 코드 검토·수정 완료 / Debug Product 컴파일·링크·배포 성공 / Protocol 646·Server 계약 1159·Python 106·저장 probe 5건 성공 / 사용자 화면 확인 전

## 1. 구현 완료

| G | 내용 | 파일 |
|---|---|---|
| G01 | 보스 spawn이 저작 XZ가 보행 가능 셀이면 그대로 두고 높이만 샘플. 비보행 셀만 셀 중심 투영 | `Server/Private/GameRoom.cpp` `Build_WorldEntity` |
| G01 | 계약 테스트: 관문 1 세이튼 정확 좌표 + `Is_PlayerPositionClear(-2.84, 1.32, 941.02)` | `ServerGameplayContractTests.cpp` |
| G02 | 빙고 `weaponModelPreScale` 0.0001, 대형 세이튼 `WP_MN_RPCT_06` 0.001 | `Data/Actors/BossCatalog.json` |
| G02 | BossCatalog **v8** `weaponModelPreRotationDegrees` (무기 row `[pitch, yaw, roll]`, 무기 없는 row null) | 데이터 8 row, `ActorCatalog.h/.cpp`(16속성), `Publish-GameplayBalance.ps1`, `valtan_presentation_generation.py`, `valtan_native_animation_inventory.py`, `validate_effect_v2.py`, python 테스트 4개 |
| G02 | 무기 admission이 회전을 굽는다 | `KoukuSaydonPresentationAssetService.cpp`, `ValtanPresentationAssetService.cpp`(Valtan은 0) |
| G02 | `CNpc` Debug yaw offset·catalog 회전에서 저장 목표 회전으로의 보정 | `Npc.h/.cpp` |
| G02 | 튜닝 슬라이스 재작성: 대형 세이튼 scale/offset + 아레나 boss 5개(`KOUKU_TUNE_BOSSES`) yaw 행 + 뿅망치 row(대형·빙고) scale·3축 회전. Save가 `bodyModelPreScale`, 각 placement `yawDegrees`, 무기 row `weaponModelPreScale`/`weaponModelPreRotationDegrees`, 대형 position을 patch | `MainApp.h/.cpp` |
| G02 | Save 버그 수정: row 경계를 다음 row 키로 잡아 `presentationClips` 뒤 키도 patch | `MainApp.cpp` `Patch_JsonNumberAfter` |
| G02 | Workbench "대형" preview 배율 = catalog `bodyModelPreScale / MN_RPCT_06 preview 0.017`(현재 5.88), preview 뿅망치 admission = catalog 배율·회전 | `Animation_Tool.cpp`, `CharacterPreviewPanel.cpp` |
| G03 | protocol **59**: `PLAYER_SNAPSHOT` `iCurrentMadness/iMaximumMadness/eMadnessForm`, 검증(`current <= maximum`, form < END) | `PacketType.h`, `PacketMessages.h/.cpp`, 하네스 |
| G03 | Server `SERVER_PLAYER` 필드, `MADNESS_GAUGE_MAXIMUM = 10000` 상수, spawn·class 변경·부활 리셋(0/10000/NORMAL), snapshot 복제 | `ServerPlayer.h`, `GameRoom.cpp` |
| G03 | HUD ViewModel 복사 | `CombatHUDViewModel.h/.cpp` |
| G04 | `C2S_DEBUG_SET_MADNESS_FORM` / `S2C_DEBUG_SET_MADNESS_FORM_RESULT`(7 verdict), Debug 전용 `Apply_DebugMadnessForm`(같은 world·newer sequence·생존·미포박·다른 form), 계약 테스트 5개 | Shared, `RoomCommand.h`, `ServerApp.cpp`, `GameRoom.h/.cpp`, 테스트 |
| G04 | Client 송수신·sink·`CPlayerController::Request_DebugMadnessForm` + verdict 소비 | `NetworkManager.*`, `PlayerCommandSink.h`, `NetworkPlayerCommandSink.*`, `PlayerController.*` |
| G04 | 교체 edge: `record->eMadnessForm != player.eMadnessForm`도 `Replace_CharacterClass`. `Create_Character(class, form, …)`가 CLOWN이면 `Ensure_ClownBodyPrototype` + `Find_ClownSpec()`; `CHARACTER_DESC::eCharacterClass` override로 퀵슬롯은 replicated class | `ClientReplication.*`, `NetObjectRegistry.h`, `Character.h/.cpp`, `PlayerController.cpp` |
| G04 | `Spec_KoukuSaydonClown`(MN_RPCT_03, 무기·장비 0, `rpct00_idle_battle_1`/`rpct00_run_battle_1`, 나머지 clip null, logic null); admission `Scaling(0.017) × RotationY(-90°)`(class 몸체와 같은 convention) | `CharacterCatalog.h/.cpp`, `KoukuSaydonPresentationAssetService.h/.cpp` |
| G04 | F1 `KoukuSaydon Arena` > `Madness Avatar`: `Change to Clown` / `Return to Player`(현재 form에 따라 하나만 활성, verdict 대기 중 비활성) + `Madness cur/max | form` | `MainApp.cpp` |
| 문서 | CLAUDE.md: protocol v59, BossCatalog v8, 관문 spawn 정확 좌표, 광기·광대 계약, preview 배율 | `CLAUDE.md` |

Kouku Product는 다른 세션이 올린 composition revision 50 기준으로 다시 projection했다(`KoukuSaydonEncounter.json`, `KoukuSaydon.patternbindings.json`). balance publisher 선행 조건이며 내용은 결정적이다.

### G02·G04 후속 검토에서 수정한 오류

- `Level_KakulSaydonArena::Update`가 광대 몸체 교체 후 `Set_LocalCharacter`를 호출해 이동·행동 sequence를 1로 되돌렸다. 같은 Server player를 다시 연결하는 `Rebind_LocalCharacter`로 바꿔 오래된 명령 거부를 방지했다.
- quick-slot은 원래 class를 읽었지만 LMB는 class가 END인 광대 spec을 읽었다. `Poll_BasicAttack`도 `Get_CharacterClass()`를 받아 원래 클래스의 평타를 조회한다. command sink 변경 시에는 미응답 광대 명령 상태를 정리한다.
- `Apply_CharacterClassChange`의 `staged = player`가 광기 상태를 그대로 복사했다. 기존 class-change transaction 안에서 `0/10000/NORMAL`로 초기화하고, 기존 계약 테스트에 `250/500/CLOWN` 입력을 추가했다.
- 망치 live의 `R(catalog) * R(offset)`과 Save의 `R(baseline Euler + offset Euler)`가 달랐다. 실제 catalog 회전과 저장 baseline을 분리해 `inverse(R(catalog)) * R(target)`으로 보정한다. 보스 yaw도 저장 목표와 실제 unadjusted yaw의 차이를 적용해 Save → Reload 후 목표를 유지한다.
- 두 JSON 저장 직전 외부 writer가 끼어드는 경우 외부 값을 덮어쓰고 성공을 반환했다. `ReplaceFileW`가 만든 backup을 expected bytes와 즉시 비교하고, 충돌이면 기존 rollback으로 외부 변경까지 보존한다. 정상 저장·stale baseline·두 번째 교체 실패·첫째/둘째 교체 직전 외부 저장을 실제 WinAPI 파일 연산으로 검증했다.
- catalog 부재 시 옛 100배·망치 원본 배율을 쓰던 fallback을 제거했다. 유효하지 않은 catalog는 오류를 표시하고 이전 preview를 보존한다. Animation Tool과 팀 인계서의 남은 100배 안내도 현재 계약으로 교정했다.

기존 미커밋 변경은 보존했다. 이번 수정 전 파일은 `out/KoukuArena/review-baseline/`에 보관하며, 여러 기능의 dirty worktree이므로 자동 stage/commit/push하지 않았다. 새 제품 C++ 파일이나 project/filter 항목은 추가하지 않았다.

## 2. 검증

| 검사 | 결과 | 증거 |
|---|---|---|
| Debug Product (Engine → Shared → Server → Client) | 최종 컴파일·링크·SDK/shader/DLL 배포 성공, missingRuntimeInputs 0 | `out/KoukuArena/review-product-final.log`, `out/BuildPipeline/runs/20260905T160807679Z-debug-product.json` |
| 새로 빌드한 `NetworkProtocolHarness.exe` | 646 PASS, failures 0 | `out/KoukuArena/review-protocol-build.log`, `review-protocol-run.log` |
| 새로 빌드한 `Server.exe --contract-test` | 1159 PASS, failures 0. 관문 정확 XZ, 광대 명령, class-change 초기화 포함 | `out/KoukuArena/review-server-contract.log` |
| python (`test_kouku_saydon_runtime_inputs`, `test_project_kouku_saydon_composition`, `test_valtan_model_view_composition`, `test_valtan_native_animation_inventory`, `test_validate_effect_v2`) | 100 tests OK | `out/KoukuArena/review-python.log` |
| `test_ground_target_preview_prototype_scope` | 6 tests OK | 터미널 |
| `Publish-GameplayBalance.ps1 -Mode Publish`, `Publish-WorldGameplay.ps1 -Mode Publish` | 성공. composition revision 50, BossCatalog v8 소비 | `out/KoukuArena/review-publish-gameplay.log`, `review-publish-world.log` |
| 실제 저장 helper를 추출한 임시 CPU/WinAPI probe | 수정 전 lost update 재현 → 수정 후 5 tests, failures 0, 잔여 sidecar 0 | `out/KoukuArena/tuning-save-probe-before.log`, `tuning-save-probe-after.log` |
| 회전 수치 검사 | 비영 3축 baseline+offset 오차 0.2474568 → 1.11e-16, Save/Reload와 5 placement yaw 일치 | `out/KoukuArena/tuning-rotation-numeric-review.json` |
| 변경 JSON parse / project XML parse | JSON 8개, 기존 Client/Server project·filters 4개 성공. 변경 XML 없음 | `out/KoukuArena/review-document-parse.json` |
| `git diff --check` | 성공 | 터미널 |
| Client 실행·UI 조작·화면 확인 | 미실행, 사용자 전용 | 화면 PASS로 기록하지 않음 |

빌드에는 기존 C4819 코드 페이지 경고와 DirectXTK PDB 누락 LNK4099 경고가 남아 있다. 오류 0과 경고 0은 구분한다. publisher의 일부 클래스 hit-shape coverage 경고는 로그에 보존했다. 광대 실제 이동·평타 입력과 시각적 크기·방향, F1 Save/Reload 조작은 위 자동 검사만으로 완료 판정하지 않는다.

## 3. 사용자 확인 절차

1. 빌드·배포·계약 테스트는 완료했다. 이 PC는 LAN `server-host`이며 Visual Studio `Server + Client` profile을 선택하고 사용자가 `Ctrl+F5`로 시작한다. Client 작업 디렉터리는 `Client/Default`, endpoint는 `192.168.0.4:7777`이다.
2. 같은 v59 Server+Client에서 Lobby → `KoukuSaydon` → F1 `KoukuSaydon Arena`:
   - `1관문 - 세이튼` / `3관문 - 세이튼`: 이동 성공, 세이튼이 (-0.07, 942.33)에 선다.
   - `빙고`: 뿅망치 크기. `2관문`: 대형 세이튼 뿅망치가 보인다.
   - `Arena Boss Tuning`: 각 보스 `yaw offset`, 뿅망치 `scale`/`rotation offset` → `Save Tuning` → `Reload Baseline`. 조정한 크기·방향이 유지되는지 확인한다. catalog 값은 다음 Client 실행, yaw/position은 `Publish-WorldGameplay.ps1` + Server 재시작 뒤 적용.
   - 먼저 우클릭 이동·스킬을 여러 번 사용한 다음 `Madness Avatar` > `Change to Clown` → 우클릭 이동·퀵슬롯·LMB → `Return to Player`. 변신/복귀 후 입력이 계속 반영되는지와 idle/run을 확인한다. 광대에는 별도 skill animation이 없으며 Server skill command만 원래 class를 유지한다. F1의 `Madness 0 / 10000`도 확인한다.
   - Workbench에서 대형 세이튼 action preview 크기가 스폰된 대형 세이튼과 같은지.
3. 광대 몸체가 90° 틀어져 보이면 `Ensure_ClownBodyPrototype`의 `-90°`를 조정한다. 보스들이 "왼쪽을 본다"는 현상은 Kouku rig가 class rig와 같은 forward를 가진 채 CNpc admission에 회전이 없어서일 가능성이 높다. 5개 yaw 행으로 보정한 뒤 값이 모두 ±90°로 일관되면 admission 회전으로 옮긴다.

## 4. 남은 경계

- 광기 게이지 증가·가득 참 → 자동 광대 변신·지속 시간·HUD 바: 없음(상수 0/10000, Debug 버튼만).
- 진짜 쿠크 찾기 45° 시야각, 방패 반사 45° cone, 댄스타임 최대 HP% 피해: Logic 런타임 슬라이스(계획서 G05)로 후속. 저작 Logic 박스는 이름만 있다.
- 튜닝 슬라이스는 값 확정 뒤 제거한다.
