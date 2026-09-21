# Valtan 편집 재생·진입 연출·유령 표시 결과

대응 계획: [캐릭터·탈것 복원 계획 G06](2026-09-22_CHARACTER_BONE_DRAGON_GUARDIAN_RESTORATION_PLAN.md). 반영 대상은 #437 병합 뒤 통합 worktree다. 최종 소스는 Desktop의 기존 사용자 변경을 보존해 3-way 병합했으며, 양쪽 Product 빌드 결과는 통합 RESULT를 따른다.

## G06-01. F1 위치와 발탄 소유 객체 수명

`MainApp::RenderValtanArenaControls`에 Start Position, Before Entrance, Arena Start 및 Despawn Valtan Boss를 추가했다. 위치는 기존 `CPlayerController -> IPlayerCommandSink`의 typed Server 이동 경로를 사용한다. 위치는 각각 (8.8,9.77,-20.22), (125.9,23.0176,-93.1), (147.75,23.0176,-117.25)다. 입장 전 위치는 Stage_Boss trigger 밖이다.

`CLevel_ValtanArena`가 spawn token, despawn 요청 및 10초 timeout을 소유한다. Server의 기존 world entity 명령을 확장하여 Debug Valtan Arena에서는 `ENCOUNTER_VALTAN`의 primary BOSS_VALTAN과 owner 연결로 도달하는 종속 객체만 제거한다. 유령의 자식이 vector 앞에 있어도 전체 소유 관계를 수집한다. 일반 NPC/웨이브 몬스터, 파괴 상태와 player 소유 combat object는 유지한다. Pattern/Flow/Timeline 수명과 boss 소유 combat object는 기존 취소 경로로 정리한다.

`ValtanBossTool`의 Play Pattern은 발탄이 없으면 canonical disabled placement `boss.valtan.center`를 Server에 요청하고 reliable spawn과 primary presentation을 기다린다. 준비 중 source revision/sound receipt를 고정하고, 최종 제출 전에 기존 exact presentation admission을 다시 수행한다. Server가 만든 발탄은 idle hold이며 패턴 실행 전 자동 전투를 시작하지 않는다. UI가 local boss를 만드는 경로는 추가하지 않았다.

## G06-02. 일리아칸 모델과 source cinematic 준비

기존 설치에는 `Map/Valtan/Cinematics/Actor64/Body/MN_TSLC_00-1.wmodel`(19,651,234 bytes), Weapon `WP_MN_TSLC_00-1.wmodel`(98,280 bytes), source DDS 10개가 존재한다. Body native92/weapon native93 재질과 entrance.actor64.body.0, weapon.0, weapon.1이 revision20 worldsequences의 실제 소비자다. Body clip은 valtan.cinematic.actor64.entrance, 길이는 24,708ms이며 body visibility keys 6개는 모두 visible이다. 이 세션은 기존 원본 모델을 임의 대체하거나 재추출하지 않았다.

기존 `Ready_SourceCinematics`는 normal async V1 effect 준비 중에도 false를 terminal failure로 취급하여 player.Clear 후 재시도하지 않았다. 이제 전체 effect closure의 pending/failed를 구분하고, pending이면 admitted document와 pool을 보존한 채 다음 입력 준비에서 재시도한다. 준비가 끝난 프레임에는 primary owner를 먼저 바인딩한 뒤 cinematic을 시작한다. 실제 모델/clip/clone 실패는 정확한 resource 상태를 유지한다. F1 Presentation Status에서 확인할 수 있다.

## G06-03. 유령 발탄: 확인된 결함과 남은 화면 경계

로컬 Workbench의 `Start_ValtanPatternMasterPreview`는 일반 Valtan clone을 선택한 뒤 저장된 authoringPhase를 모델 선택에 사용하지 않았다. 명시 phase3인 GHOST_FINALE/PORTAL_ONCE/RESPAWN_AUDITION/DEATH_AUDITION도 일반 part가 남았다. 이제 local pattern preview는 authoringPhase(없으면 minimumPhase)를 소비한다. 기존 body/weapon group stage에서 실제 clip/source clock와 effect bone anchor를 검증한 뒤 한 번에 교체한다. 실패는 기존 part group을 유지한다. 교체 뒤 AnimationTargetService generation과 master clock의 CModel 참조를 새 모델로 다시 바인딩한다. Effect Tool의 별도 standalone IDLE 경로에는 이 자동 교체를 강제하지 않는다.

현재 전투 consumer는 primary phase3 snapshot -> BOSS_VALTAN_GHOST part group -> CBody_Valtan BLEND enqueue -> native84 pass10 -> material/bone/scene light binding이다. Ghost body는 머리를 포함한 3개 mesh이며 도끼는 기존 별도 equipment와 b_weapon socket을 사용한다. 각 HRESULT를 보존하고 실패 stage를 F1에 표시한다. dormant replication, Server GHOST_HIDDEN, authored body window, source cinematic suppression도 구분한다. 진단 노출 자체를 표시 복구로 계산하지 않는다.

5회 반복 보고에 대해 현재 자료로 확인한 검증 공백은 다음과 같다. 이전 별도 probe는 조명을 바인딩하지 않고 RGB 또는 alpha가 clear와 달라진 픽셀을 성공으로 셌다. 검은 반투명 mesh도 통과하므로 실제 색 표시 증거가 아니었다. 현재 installed ghost 모델, 현재 150-clip cinematic donor를 합친 290 clip, preScale .01, body -90도/1.4 scale, 현재 Valtan directional 및 ambient로 직접 GPU 수치를 다시 측정했다. 조명이 없으면 covered192/colored0/RGBmax0이지만 실제 profile에서는 아래와 같이 양수다. 따라서 현재 native84 재질이 모든 상황에서 0으로 곱해진다는 주장은 재현되지 않았다. 새로운 임의 밝기/색 보정을 넣지 않았다.

| 실제 모델 clip/시간 | RGB > 1e-5 픽셀 | RGB 최대 | 비정상 수치 |
|---|---:|---:|---:|
| mesh_idle_battle_1 / 0s | 467 | 0.651508 | 0 |
| mesh_respawn_1 / 0s | 193 | 0.284872 | 0 |
| mesh_respawn_1 / 2s | 501 | 0.923802 | 0 |
| valtan.cinematic.finale / 2s | 500 | 0.947511 | 0 |

이 결과는 actual CModel/material/native shader의 headless D3D WARP 수치다. 사용자 실행 중의 camera/depth/owner suppression과 최종 화면까지 성공했다는 뜻은 아니다. 실제 전투의 마지막 비표시 발생 원인은 아직 한 branch로 확정하지 못했다. 이전 5번의 작업 전체가 실패한 원인을 이 한 가지로 단정하지 않는다. 현재 실행 화면은 사용자 확인 대상이다.

## G06-04. 실행한 검증

- 통합 worktree의 변경 Client 6개 TU(Body_Valtan, Valtan, Animation_Tool_ValtanPlayback, Level_ValtanArena, MainApp, ValtanBossTool) MSVC C++20 syntax PASS. Server Debug 실제 증분 compile/link PASS.
- 실제 GameRoom/typed command 소비자를 링크한 집중 contract 5개 PASS, failures0: room 밖 session 거부, 역순 owner closure 제거, NPC/monster 보존, canonical idle-held respawn, 중복 spawn 방지. fixture는 기존 ServerGameplayContractTests_ValtanPinnedGeneration에 등록했다.
- 실제 Server navigation으로 F1 3좌표 모두 direct validation PASS. 바닥 Y는 9.771/23.047/23.018m.
- `git diff --check` PASS. 전체 Server --contract-test는 불필요한 Kouku 선행 suite가 길어 집중 검증 성공 후 해당 임시 process만 중단했다. 전체 suite PASS로 기록하지 않는다.
- Client/UI 실행과 화면 조작은 수행하지 않았다. 최종 Product 통합 빌드·설치 여부는 세션 최종 RESULT가 소유한다.

로컬 증거: `out/GhostValtan20260922/valtan_editor_contract.log`, `server-build.log`, `*.integrated.log`, `navigation-probe.log`, `current-consumer-*.log`. 빌드 산출물과 probe DLL/CSO는 소스 전달 대상이 아니다. 이 G는 Resources 파일을 추가하지 않았다.


## G06-05. 전투 재질 0 곱 경계 재감사

현재 BossCatalog의 ghost 3재질은 opacity=1, dead=0, object_color=(0.6,1,1,1), diffuse_color=(0.1,0.6,0.5,1)/(0.2,1.2,1,1)이며 action/buff intensity는 (0,0,0,1)이다. RGB 0인 intensity 벡터는 shader의 `rgb*w+1`으로 소비되어 기본색을 0으로 만드는 승수가 아니다. native84 base/light의 primitive source[0].x는 매 평가 1이며 light source[16]은 실제 lightColor로 교체한다. base의 원본 engine sky rows[20/21]가 0인 사실만으로 전체 출력이 0이라고 판단할 수 없다. 실제 profile WARP RGB 검증이 이를 구분한다.

CValtan 및 CBody_Valtan 전투 경로에는 `Override_SourceCharacterConstants/Texture` 호출이 없다. 현재 source override 소비자는 Character 및 Effect model presentation이다. Sea CModel과 ghost CModel은 별도 prototype이므로 Sea의 값이 ghost 재질로 직접 넘어간다는 근거도 없다. 다만 같은 모델의 여러 Clone이 CMaterial을 공유하던 실제 결함은 material write의 copy-on-write로 수정했고, 실물 Sea 2Clone/5재질에서 owner만 변경·peer/prototype 보존·원복을 검증했다. 이것을 사용자 ghost 비표시 원인의 확정으로 계산하지 않는다.

BLEND consumer는 scene forward light/fog → mesh별 base material → light material → bone → program84/pass10 → mesh submission을 순서대로 검사한다. F1에는 최초 실패 stage와 mesh index/HRESULT, 프로그램과 제출 mesh 수, parent의 dormant/GHOST_HIDDEN/authored window/cinematic suppression이 나온다. 바인딩 성공이 곧 RGB 양수를 뜻하지는 않는다. `BindForwardSceneLights`는 diffuse RGB가 모두0인 조명을 제외한다. scene/transient light가 전부 제외되면 count=0으로 정상 바인딩되고, 현재 native84는 조명이 없을 때 RGB0이다. F1 Live rendering comparison에서 Directional light를 끄는 명시 옵션도 diffuse를0으로 만들어 이 상태를 재현할 수 있다. 이 옵션의 기본은 켜짐이며 현재 사용자의 실행 중 옵션/조명 상태를 읽은 것은 아니다.

실제 재발 시 첫 분류는 (1) parent hidden 사유, (2) parts=BOSS_VALTAN_GHOST 및 program84/3mesh, (3) 제출3/첫 실패 slot, (4) scene light count와 diffuse/ambient 및 Live comparison 상태다. render-stage 실패는 기록하지만 successful-draw RGB0를 CPU HRESULT만으로 판별하지 못한다. 최종 화면과 그 프레임의 light/suppression 상태가 없으므로 실제 전투의 다섯 번째 비표시 원인은 여전히 확정하지 못했다. profile/light를 임의로 밝게 만들거나 원본 색에 상수를 더하는 수정은 하지 않았다.
