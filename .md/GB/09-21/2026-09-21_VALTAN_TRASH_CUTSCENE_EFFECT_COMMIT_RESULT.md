# 발탄 버러지 컷신 V1 이펙트 복구 결과

## 원인

`발탄 스킬 연출(버러지)`의 `effect.valtan.cinematic.trash.actor106.at667`에는 두 문제가 순서대로 있었다.

1. MapTool은 MainApp의 일반 pending Effect commit 뒤에 실행되므로, 새 World-root V1 handle을 만든 같은 프레임의 seek가 pending handle을 샘플했다.
2. 이를 commit한 뒤 683ms에서 bounded source-loop 검증이 실패했다. actor106 원본은 무한 `EmitterLoops=0` sprite/mesh/ribbon carrier 7개와 같은 source의 light carrier 4개를 함께 가진다. 기존 검사가 light도 particle/ribbon이어야 한다고 요구해 정상 source를 거절했다.

## 반영

- `Client/Public/WorldSequencePlayer.h`: MapTool 전용 `bCommitWorldRootEffectsAfterSpawn`를 추가했다.
- `Client/Private/MapTool_Cutscenes.cpp`: MapTool cutscene target에서만 이 플래그를 켰다.
- `Client/Private/WorldSequencePlayer_Objects.cpp`: 새 V1 Level-placement effect를 만든 직후 MapTool에서만 해당 root를 commit하고 seek한다.
- `Client/Private/Effect_Playback.cpp`: bounded source loop의 필수 주체를 admitted sprite/mesh/ribbon particle로 확인한다. 같은 admitted source visual program의 `LIGHT`/`light` element는 보조 표현으로 허용하되, light만으로 loop0 조건을 통과하지 못하게 했다.

원본 effect document와 `loopEffectToDuration=true`는 유지했다. 따라서 2.175초 effect 창 동안 원본 loop0 방출과 빛 표현을 함께 사용한다.

## 검증

- `Client.vcxproj /t:ClCompile`에서 `Effect_Playback.cpp` 컴파일 성공, 오류 0건.
- Debug Product Build 성공. `Client/Bin/Debug/Client.exe`가 2026-09-21 17:52:02에 새로 링크됐고 실패 marker가 없다.
- actor106 JSON에서 loop0 sprite/mesh/ribbon primary 7개, `light` supplemental 4개를 확인했다.
- 변경 파일 `git diff --check` 통과. 기존 EngineSDK 인코딩 경고만 남았다.

MapTool 화면을 자동 조작하지는 않았다. 새 Debug Client로 `발탄 스킬 연출(버러지)`를 재생했을 때 667~683ms에서 `World actor failed`와 `Every actor was released`가 더 이상 표시되지 않아야 한다.