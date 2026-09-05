# 2026-09-05 KoukuSaydon 명명 통일과 Valtan owner 분리 결과

> 문서 종류: 구현 결과서
>
> 상태: P0-2 자동 검증 완료 / 사용자 화면 검증 비대상
>
> 기준 브랜치: `GB/KoukuSaydon-DataFormat`

## 1. 완료된 계약

- authored data, composition, animation document, pipeline 이름을 `KoukuSaydon`으로 통일했다.
- 기존 public/runtime 계약인 `KAKULSAYDON_ARENA`, `kakulsaydon.*`과 물리 Resource alias `Character/KoukuSaton/...`은 호환 경계로 유지했다.
- 이름만 일반적이던 발탄 owner를 `CValtanBossTool`, `CValtanActionWorkbench`로 명시했다.
- 발탄 Workbench에서 Kouku/Kakul/Saydon 분기와 역참조를 제거했다. 발탄은 `Get_ValtanCompositionSequences`만 소비한다.
- Kouku animation action/pattern document는 별도 `CKoukuSaydonAnimationActionDocument`, `CKoukuSaydonAnimationPatternDocument`가 소유한다.
- Kouku pipeline이 발탄 navigation builder를 import하던 결합을 제거하고 중립 transform helper로 이동했다.
- generated Composition Product는 새 canonical 파일을 먼저 stage/promote한 뒤에만 폐기된 Kakul 파일을 정리한다.
- Valtan-owned production source가 Kouku domain을 다시 참조하지 않는 naming/separation validator를 추가했다.

## 2. 의도적으로 유지한 이름

다음은 오타나 미완료 rename이 아니라 기존 public 또는 물리 asset 계약이다.

- `LEVEL::KAKULSAYDON_ARENA`, `WORLD_ID::KAKULSAYDON_ARENA`
- `CLevel_KakulSaydonArena`
- `raid.kakul-saydon.arena`, `stage.kakul.*`, `player.spawn.kakul.*`
- `Character/KoukuSaton/...`
- `KakulArenaHiddenPlacements.h`

신규 authored 파일명, Tool domain, C++ document class에는 위 호환 표기를 확장하지 않는다.

## 3. 검증 결과

| 검증 | 결과 |
|---|---:|
| Composition source validation | PASS — Valtan 42 patterns, KoukuSaydon 4 profiles / 349 actions, sequencer 2개 |
| Valtan renamed owner/action presentation/resource regression | PASS — 118/118 |
| KoukuSaydon pipeline | PASS — 46/46 |
| Composition/native facade/neutral transform/build-domain receipt | PASS — 53/53 |
| Debug x64 Product build | PASS |
| Product compiled-shader closure | PASS — V1/V2 WARP pixels 1352/1352 |
| `git diff --check` | PASS — line-ending warning만 존재 |

광역 `Tools/ValtanPipeline` discovery 1회는 동시에 실행된 writer test끼리 canonical transaction lock을 경쟁해
`CANONICAL_TRANSACTION_BUSY`가 발생했으므로 검증 증거에서 제외했다. 격리 재실행한 P0-2 직접 영향 테스트는 전부 통과했고,
Valtan gameplay data, damage 수치, exact oracle 값은 변경하지 않았다.

## 4. 생성물과 선행 변경 경계

`Client/Bin/DataFiles/Compositions/Bosses/Valtan.bosscomposition.json`과
`Client/Bin/DataFiles/Compositions/Composition.publish.receipt.json`에는 작업 시작 전에 존재하던 Effect/Sound source 변경의 publish 결과도
포함되어 있다. 이번 변경은 그 source를 되돌리지 않고 현재 canonical input 전체를 publisher로 다시 생성했다.

## 5. 다음 구현 경계

P0-3은 발탄 Workbench에 Kouku 분기를 추가하지 않는다. 별도 `CKoukuSaydonBossTool`과
`CKoukuSaydonActionWorkbench`가 독립 document, command, selection, dirty state, save transaction, preview session을 소유한다.
첫 실제 Product consumer는 `KAKULSAYDON_G1_PIZZA`이며, Server play는 world/encounter/placement/archetype/gameplay revision을
모두 고정한 scoped request로만 허용한다.
