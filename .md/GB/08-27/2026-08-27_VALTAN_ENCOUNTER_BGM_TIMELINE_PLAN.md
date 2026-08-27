# 2026-08-27 발탄 전투 BGM 타임라인 구현 계획

## 현재 상태와 구현 경계

발탄 BGM 원본 WAV와 `SoundCatalog.json`의 stable asset ID는 존재하지만 제품 런타임 소비자가 없다.
이번 변경은 Server snapshot이 이미 확정한 발탄 pattern/action 경계를 Client presentation이 읽어
다음 한 개의 음악 상태만 재생하도록 연결한다. 전투 판정과 phase 결정은 바꾸지 않는다.

```text
Valtan Level activation                 -> M01
spawn.valtan.stage02.miniboss 종료       -> M04
VALTAN_ENTRANCE_WHIRLWIND 시작          -> M05
entrance pattern 종료 후 첫 일반 전투      -> M06
VALTAN_GHOST_TRANSITION_15 시작           -> M07
ghost-transition-15.ghost stage 진입       -> M08
WORLD_ENTITY_ACTION::DEAD 진입             -> M09
```

## 수정 파일과 책임

| 파일 | 변경 책임 |
|---|---|
| `Engine/Public/Sound/Sound_Manager.h` | 추적 가능한 music channel의 loop/one-shot 선택 계약 |
| `Engine/Private/Sound/Sound_Manager.cpp` | loop mode별 FMOD Sound cache와 단일 music channel 교체 |
| `Engine/Public/GameInstance.h` | music loop 선택을 Client에 전달 |
| `Engine/Private/GameInstance.cpp` | Sound Manager forwarding |
| `Client/Public/Valtan.h` | raid BGM state와 entrance 관찰 상태 소유 |
| `Client/Private/Valtan.cpp` | Server snapshot edge를 M05~M09로 변환하고 resource asset 재생 |
| `Client/Public/ClientReplication.h` | reliable world-entity despawn presentation callback |
| `Client/Private/ClientReplication.cpp` | placement/archetype identity를 보존해 Level callback 전달 |
| `Client/Public/Level_ValtanArena.h` | M01/M04 선행 구간 음악 상태 소유 |
| `Client/Private/Level_ValtanArena.cpp` | Level activation과 stage02 Lugaru despawn을 M01/M04로 변환 |
| `Client/Bin/Resources/Sound/BGM/Valtan/**` | Sound catalog가 이미 선언한 exact WAV dependency closure |

## 불변식

- Level은 M01/M04 선행 구간을 소유하고 Server-authoritative `CValtan`은 M05~M09를 소유한다.
- Animation Tool/Character Select preview는 음악을 시작하거나 정지하지 않는다.
- 같은 BGM 상태의 반복 snapshot은 재생을 다시 시작하지 않는다.
- M07 이후 일반 pattern snapshot이 와도 M06으로 되돌아가지 않는다.
- M09는 one-shot이며 boss 객체가 제거되거나 level이 끝나면 추적 channel을 정지한다.
- 음악 파일 누락은 해당 presentation만 격리하고 Server snapshot 적용은 계속한다.

## 검증

1. exact WAV 5개의 runtime 경로와 catalog asset ID를 대조한다.
2. Engine Debug 빌드 후 `UpdateLib.bat Debug`를 실행한다.
3. Client Debug 빌드와 `git diff --check`를 실행한다.
4. 사용자가 Lobby -> Valtan에서 등장, 첫 일반 전투, 14줄 시작, GHOST stage, 사망 순서로 직접 듣는다.
