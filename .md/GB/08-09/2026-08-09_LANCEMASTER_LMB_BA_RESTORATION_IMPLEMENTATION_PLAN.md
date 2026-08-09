# 2026-08-09 창술사 LMB BA 수동 복원 후보 계획

## 1. 목표와 완료 경계

창술사 긴창 LMB `34010` BA1~4와 짧은창 LMB `34510` BA1~3의 Source 리소스와 발생 근거를 고정하고, 사용자가 Effect Tool에서 직접 조립할 수 있는 별도 후보를 만든다.

- Cascade/Imported는 WModel, DDS, Material, emitter 종류, timing, anchor의 근거로만 사용한다.
- 자동 평탄화 위치를 최종 시각 정답으로 간주하지 않는다.
- Product Effect ID, Assembly, Component, runtime catalog에는 후보를 게시하지 않는다.
- 상태는 `SOURCE_EXTRACTED -> MANUAL_ASSEMBLY_PENDING -> VISUAL_APPROVED`로만 승격한다. 실행 가능한 후보가 준비된 것만으로 승격하지 않으며, 현재 수동 gate는 `MANUAL_VISUAL_PENDING`이다.
- 실제 화면 검증 전에는 어떤 단계도 `VISUAL_APPROVED`로 기록하지 않는다.

현재 worktree에는 검증된 authoring v13 codec/playback 코어를 재베이스했다. 후보 7개는 `transformInheritance` 계약으로 정본 Resource root를 사용해 Load/Validate/Drawable 검증할 수 있지만, 수동 화면 검증 전에는 계속 `SOURCE_EXTRACTED / MANUAL_VISUAL_PENDING`에 머문다. 이 worktree에는 `Client/Bin/Resources`가 없으므로 무환경 standalone 실행은 완료 조건으로 기록하지 않는다.

## 2. 전수 감사가 정한 문제 분류

4직업 101 Product의 2168개 요소는 현재 Mesh 888, Sprite 1234, Decal 46이다.

- Mesh 888 = Imported meshParticle을 standalone Mesh로 평탄화한 872 + 차원술사 A 수동 16
- Sprite 1234 = spriteParticle proxy 1231 + Ribbon을 Sprite로 잘못 평탄화한 3
- Product Particle, Trail, Light, Screen Post = 모두 0

Imported canonical/event 근거는 meshParticle 1187, spriteParticle 3564, Ribbon 95, Decal 79, Light 218, Screen Post 89이다. 별도 animation notify에도 TrailGhost 72 + Trails 8 = 80건이 있다.

Product와 Imported의 물리 에셋 누락 및 visible Base 누락은 0이다. 따라서 본 작업은 missing asset 복구가 아니라 renderer semantics와 관계 구조의 손실 복구로 분류한다. `fx_a_blankwhite_01.dds`도 실제 source carrier일 수 있으므로 이름만 보고 fallback으로 판정하지 않는다.

## 3. 34010 조립 계약

각 BA는 다음 세 축을 분리한다.

1. `fm_m_ring_001` Mesh
   - 큰 남청색 초승달의 visual master 후보
   - Player root snapshot 공간
   - source Particle color `[0.2, 0.375, 0.5]`, alpha `0.5`를 preview multiply로 사용
   - 최종 위치, 회전, 크기, pivot, 원본 Material graph는 수동 대기
2. AnimTrail
   - `EFData_AnimNotify_Trails`와 `ParticleModuleTypeDataAnimTrail` 근거
   - 별도 weapon-follow 공간
   - 현재 generic Trail은 audition용 축약이며 source-exact AnimTrail 완료가 아님
3. Ribbon
   - `ParticleModuleTypeDataRibbon`과 `WP_FLM_1_Battle` event 근거
   - source Material `fx_m_pa_ribbonmaster_01_9_tr`이 두 물리 패키지로 모호함
   - candidate에는 invisible `fallback-blocked` Trail queue 요소로 남기고 exact package/DDS/shader/runtime가 닫히기 전에는 켜지 않음

Ring과 weapon Trail은 attachment와 시작 시간이 다르므로 v13 transform inheritance로 연결하지 않는다. BA4 impact의 같은 root/phase 레이어만 impact Mesh를 terminal master로 삼아 Sprite companion이 최종 행렬을 상속한다.

## 4. 34510 조립 계약

각 BA는 emitter 9의 `fm_d_plane_003`을 후보 master로 두고, Mesh 2개와 Sprite 3개를 같은 group, root attachment, stage-local `0.08 s`에 둔다.

- v13 companion은 master의 최종 행렬을 상속한다.
- Cascade에서 decode한 상대 위치와 DirectX row-vector 회전 `R_relative * R_master`는 receipt의 Source 근거로만 보존한다.
- element별 lifetime으로 위치나 회전을 다시 정규화하지 않는다.
- emitter 2의 `+0.1 s` 및 random distribution은 실행값으로 쓰지 않는다.
- textureless emitter 0/1은 다른 emitter DDS를 빌려 쓰지 않고 blocked carrier로 둔다.
- 같은 `0.08 s`에 발생하는 Light carrier는 현재 slice 밖의 미복원 항목으로 기록한다.

## 5. 구현 항목

1. Source/Imported/animnotify/animevents/material report를 SHA와 함께 receipt에 고정한다.
2. 7개 Product와 분리된 `.restoration-candidate` 문서를 생성한다.
3. visible Mesh/Sprite/Trail의 Base 및 Mesh WModel 누락을 fail-close한다.
4. Ribbon처럼 Base가 결정되지 않은 hidden 요소는 `effect.ue3.fallback-blocked.v1`일 때만 허용한다.
5. 기존 후보의 raw SHA가 모두 알려진 seed와 일치할 때만 v12→v13 또는 Ribbon placeholder migration을 허용한다.
6. LanceMaster/Artist Decal source/Converted inventory를 분리해 기록한다.
7. Product, Assembly, Component, runtime catalog는 수정하지 않는다.
8. 다른 세션 소유 전역 worklist에 생성 의존성을 두지 않고, 현재 main의 7개 Product 문서를 SHA로 고정해 carrier inventory 근거를 만든다.

## 6. 검증 순서

자동 검증:

1. generator unit test 및 `--check`
2. JSON parse와 candidate/receipt SHA closure
3. fallback, cross-emitter proxy, migration overwrite guard
4. Product runtime catalog에 candidate ID가 0개인지 확인
5. v13 codec으로 기존 Product와 candidate를 함께 load/validate하는 회귀 검증
6. `git diff --check`

수동 검증은 v13 codec/workbench 자동 회귀가 통과한 후보에서만 수행한다.

1. Solo Element
2. Solo Group / generic Trail audition
3. weapon tip/width/tail 수동 조정
4. 실제 Character Select 입력
5. 사용자 승인 후에만 Product 게시
