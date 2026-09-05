# 2026-09-05 Effect V2 Rect 공유 + Particle 버퍼 free-list RESULT

브랜치: `feature/effect-v2-buffer-pool`
커밋: `d4eaba09 perf(effect-v2): share sprite/decal rect and pool particle instance buffers`
PLAN: `2026-09-05_EFFECT_V2_BUFFER_POOL_PLAN.md`

## 구현 완료

- `Client/Public/EffectV2_Object.h`: `Acquire_SharedRect`, `Acquire_ParticleBuffer`, `Release_ParticleBuffer` static 선언 추가.
- `Client/Private/EffectV2_Object.cpp`
  - `g_SharedRect`: Sprite/Decal이 process-global `CVIBuffer_Rect` 하나를 공유.
  - `g_ParticleBufferPool` + `Round_ParticleCapacity`: sprite particle 인스턴스 버퍼를 64/128/256/… bucket free-list에서 대여, 소멸자에서 `use_count()==1`일 때 반납, bucket당 64개 상한.
  - `Initialize`, `Build_ParticleInstances`, `Clear_ResourceCache`, 소멸자가 새 경로를 사용.
- Engine, `EffectV2_Runtime`, Valtan/Npc/ClientReplication, Effect Tool V2는 변경 없음. public API 시그니처 유지.

## 자동 검증

- `Invoke-BuildAndRegression.ps1 -Configuration Debug` (Core) 전 단계 PASS.
  Engine/Shared/Server/Client 빌드, CSO closure, NetworkProtocol, Character Select 격리 live harness, Valtan presentation/encounter 계약.
- `EffectV2_Object.cpp` 신규 컴파일 경고 0건.
- `git diff --check` 이상 없음.

## 수동 검증 (사용자)

- 로컬 Server(`127.0.0.1:7777`) + Debug Client에서 Effect Tool V2로 `boss.valtan.six.sonic` 다회 재생. 반복 재생 정상, 파티클 겹침·누락 없음.
- 체감 차이는 없음. 예상대로 평균 FPS가 아니라 스폰 프레임 spike만 줄어드는 변경이며 발탄 패턴 빈도에서는 눈으로 구분되지 않는다.

## 미검증

- 스폰 프레임 시간 실측(1회차 vs 재사용 회차) 없음. 사용자 결정으로 생략.
  플레이어 스킬 이펙트를 V2로 옮기는 시점에 스폰 빈도가 높아지면 그때 측정한다.

## 남는 경계

- Trail 버퍼(`CVIBuffer_DynamicTrail`)와 mesh particle `m_pMeshInstanceBuffer`는 같은 bucket 패턴으로 미적용.
- `CEffectV2Object` 객체 pool(layer 유지 + `Reset(Desc)`)은 미적용. 실측 후 필요 시 진행.
- `Clear_ResourceCache` 호출자 없음(기존과 동일).
