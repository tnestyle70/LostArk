# 쿠크·발탄 연출 복구 사용자 검증 가이드

기준일: 2026-09-20. 브랜치: `GB/koukubugfix-bingo`.

이 문서는 이전 요청과 후속 정정을 현재 구현·게시 데이터에 대조한 **화면 검토 순서**다. 아래의 반영 완료는 코드·데이터·자동 검사 상태를 뜻한다. 사용자가 앞서 실행하며 보고한 현상을 기준으로 수정했으며, 에이전트는 Client/UI를 실행하지 않았다. 복구 후 게임 화면, 다인 접속 화면, 실제 조작감에 대한 최종 사용자 판정은 아직 받지 않았다. 개별 수치 하네스와 WARP shader 검사를 게임 전체 화면 PASS로 간주하지 않는다.

실행 전제는 최신 게시 데이터를 읽는 정상 Server에 Client가 접속하고, Lobby에서 해당 아레나 입장을 승인받은 상태다. 이 문서는 Server를 다시 실행하거나 Client의 미저장 draft를 Reload하라고 요구하지 않는다.

후속 회귀 수정 상태: 1관문 캐릭터 전용 간접광, Mario1~4 fog 억제, G3 실효 노출2→1, 주사위 카드 전용 bloom0, ALT V 공통 capture/cube rig, 휠윈드 Motion offset 및 F1 Pattern Box 편집, Complete Play 리소스 준비 대기를 코드에 반영했다. 관련 데이터는 최신 저장본의 해당 필드만 병합했다. `Map/Lighting/Kouku/` 참조는 사용자가 옮긴 기존 `Map/Lighting/KoukuSaydon/` DDS로 통일했다. 리소스 복사·재생성은 하지 않았다.

이번 최종 전체 Product 빌드와 EXE 실행은 사용자가 직접 진행한다. 이전 Product 빌드 PASS를 이번 변경의 빌드 PASS로 재사용하지 않는다. 현재 변경에는 Engine 조명 ABI·Client·Server 코드가 포함되므로 **Debug x64 전체 Solution Build 후 Server와 Client를 새로 실행**한다. 실행 중인 예전 Server가 있다면 먼저 종료해야 새 EXE와 게시 데이터를 사용한다. F5/Ctrl+F5는 VS 설정에 따라 Build를 수행할 수 있다.

### 이번 수정 우선 검증 순서

1. **1관문 Complete Play**: 준비 진행 중에는 패턴 시간이 시작되지 않음 → 모든 필수 리소스 준비 성공 → 시퀀스 시작. 누락/실패는 부분 재생 대신 사유를 표시한다. 파티 Debug 참가자도 준비 완료 후 READY를 보낸다.
2. **1관문 책 카메라와 전투**: 원본 책 카메라37800ms가 끝나면1200ms blend-out으로 현재 플레이어 follow를 따른다. WORLD 행의 긴 꼬리 때문에 마지막 카메라 포즈를 붙잡지 않는다. 자연 종료·늦은 Seek·명시 Stop을 구분한다.
3. **밝기**: G1 세이튼 몸체·장비 → Mario1~4 근거리/원거리 fog → G3 불과 무지개댄스 순서로 비교한다. G1 간접광은 원본 계수를 이용한 uniform 근사이며 원본 SH 복원 완료가 아니다. Mario의 재질·LUT·bloom까지 제거한 것은 아니다.
4. **주사위 카드 P78**: 시작 후6.114/10.114/14.114/18.114초의4장(각4초 간격), 카드의 색·문양과 bloom 억제 확인. Stage 합계17.165초는 그대로여서 마지막 카드는 이전 Logic 행의 독립 수명으로 나온다.
5. **휠윈드 망치 P24**: Pattern Box 위치(1.25,0,0), 회전(0,180,0), 크기(2,2,2). 손 Motion은 기존 위치(.3,0,0), quaternion(-.5,-.5,.5,.5). F1에서도 같은 Pattern Box를 편집하며 플레이어 카드미로 망치 설정과 구분한다.
6. **차원술사 ALT V**: 이미지·액자·cube가 같은 중심에서 연결되는지 확인. Effect Detail의 `Capture / Cube Position`, `Capture / Cube Rotation (Degrees)`, `Capture / Cube Scale`과 기존 수축 방향·속도로 튜닝한다. 위치의 X/Y/Z는 카메라 기준 오른쪽/위/깊이다.
7. **빙고 복원**: 아래 G06의 `Bingo_Play1/Play2/Reset` → `Bingo_Bomb` → `Bingo_Hammer` → P94 `메두사공포` 순서. 생성 flip, 영구 경계, 폭탄 본체·심지·폭발, 망치 선행, 얼굴을 각각 확인한다.
8. **발탄 Complete Play**: 선택 Pattern 준비 진행률 → 준비 성공 후 자동 재생. 취소/선택 변경/데이터 revision 변경 시 시작하지 않음 확인. 별도 Saved Flow 전체의 새 준비 장벽까지 추가한 것은 아니다.

**거미 카운터 방향은 사용자가 직접 수정하기로 하여 이번 변경에서 보정하지 않았다.** 피격 시 공포 얼굴 복원은 유지한다. 개별 스킬 누락이 Release에서 자동 해결된다고 판정하지 않는다.

## G00. 도구 진입과 저장 기준

F1은 `LostArk Developer Tools`, F6는 Follow/Free Camera 전환이다. 실제 LMB/Q와 이동 판정을 확인할 때는 Follow 상태로 돌아오고 ImGui 입력이 마우스·키보드를 잡고 있지 않은지 확인한다. Free Camera에서는 gameplay command가 제출되지 않는다.

| 확인 대상 | 현재 코드의 패널·버튼 이름 | 저장·실행의 의미 |
|---|---|---|
| 크기·플레이어 망치 | F1 → `Player Follow Camera` → `Camera map` → `Character Size` / `Card Maze Player Hammer` | `Save camera settings`로 선택 맵 profile 저장, `Reload saved`로 저장값 재독. 연출 카메라가 활성인 동안 live preview는 제한된다. |
| 보스 휠윈드 망치 | F1 → `KoukuSaydon Arena` → `Kouku Whirlwind Hammer Transform` | `Local position (m)`, `Rotation (degrees)`, `Scale`, `Save Pattern changes`. P24의 World Box를 저장하며 같은 Workbench의 다른 미저장 Pattern 변경도 포함된다. 손 부착 Motion은 별도 Object 데이터다. |
| 앵콜 회전 | F1 → `KoukuSaydon Arena` → `Bingo Encore Rotation` | `Encore yaw (degrees)`, `Save Encore Rotation`, `Reload Saved Rotation`, `Reset Rotation Preview`. 전투 회전은 World Gameplay 게시와 Server world 재로드 이후 적용된다. |
| Object | F1 → `Open Action Workbench`의 Object 작업 영역 | `Object Resources`, `Object Sequencer`, `Object Detail`. `Visual Play`는 저작 미리보기, `Server Collision Playback`의 `Play (with Collisions)`는 저장된 패턴의 Server 판정 재생이다. |
| 쿠크 패턴 | F1 → `Open KoukuSaydon Boss Tool` → `All Patterns` | `Reload Published Patterns`, `Play Isolated` / `Play Bundle`. 전체 흐름은 `KoukuSaydon Complete Play (Server Boss Replay)`의 `Complete Play - Sequences + Pattern Flow`. |
| 원본·스킬 이펙트 | F1 → `Open Effect Tool V1` → `All Effects` / `Effect Detail` | Effect 미리보기와 저장된 Server `Complete Play`를 구분한다. 미저장 Effect 편집이 Server Product에 자동 포함되지 않는다. |
| 발탄 연출 | 기존 Valtan Action Workbench의 Pattern → Stage → Animation, 또는 All Effects의 발탄 Pattern | `Animation Play`는 선택 clip 구간, `Complete Play (Server/Arena)`는 승인된 패턴과 원본 cinematic assembly를 재생한다. |

`composition changed`나 stale revision 거절은 최신 저장본과 메모리 draft가 달라졌다는 뜻이다. 거절을 무시하고 이전 파일 전체를 덮어쓰지 않는다. 변경값과 stable ID를 기록하고 현재 draft를 보존한 다음 최신 저장본과 조정한다. `Reload Source`의 `Discard and Reload`는 미저장 Object 편집을 버리는 별도 선택이다. 이번 설치가 이미 열려 있던 메모리 draft까지 바꾼 것은 아니다.

쿠크 source의 `DRAFT`는 게시 불가와 같은 뜻이 아니다. 이번 수정 직전 source revision 1909의 게시 inventory와 실제 생성 encounter를 대조했으며 P15·P19·P24·P31·P77·P94·P95 모두 게시 pattern이 존재하고 `unavailableReason`은 비어 있다. 최종 재생은 도구가 보여 주는 현재 admission 상태를 따른다. source의 DRAFT를 테스트 때문에 임의 PRODUCT로 바꾸지 않는다.

## G01. 요구사항과 현재 구현 대조

아래 순서는 실제 확인 순서와 연결된다. 별도 표시가 없어도 에이전트의 게임 화면 검사는 미실행이며, 복구 후 최종 사용자 판정은 미수신 상태다.

| 사용자 요구 | 실제 구현·설치 상태 | 남은 제한 또는 구분 | 사용자 확인 |
|---|---|---|---|
| 1관문 빛이 약 2배 밝은 원인 분석·수정 | book-open/popup 두 scene의 실효 exposure 2→1. source LUT의 중간톤 증폭과 카드 bloom 소비를 분리했다. | prebaked light·gamma·spotlight의 중복 적용이 직접 원인이라는 근거는 확인되지 않았다. 전역 bloom·다른 맵을 일괄 낮추지 않았다. | G02: 동일 카메라에서 바닥·보스·발광 FX의 명암을 비교한다. |
| 머리 위 카드와 사진의 번지는 효과 조절 | white 및 4문양×2색 카드 9 V2 leaf의 `Scene Bloom`을 0으로 저장했다. 본체 색·alpha는 유지한다. | 모든 이펙트의 bloom을 제거한 변경이 아니다. 카드가 충분히 선명한지 실제 화면으로 판정한다. | G02: 카드 윤곽과 RED/BLACK 식별, 다른 좋은 발광 표현의 유지 확인. |
| 1~4명에게 문양 중복 없이 랜덤 배정, 색 독립 랜덤 | 다른 참가자의 문양을 제외하고 Server 배정. 색은 RED/BLACK 독립 선택, 이미 유효한 배정은 유지한다. 전투 진입 snapshot 전에 확정한다. | 색별 문양 제한을 두지 않는다. 혼자 보는 UI preview로 4인 배정을 검증할 수 없다. | G02: 1·2·3·4명 각각 진입하여 문양 중복 0건, 진입 직후 표시, 반복 tick의 불필요한 재배정 없음 확인. |
| 1관문 종료·2관문 연출 시작에 머리 카드 제거 | G2 cinematic 진입 시 이전 관문 카드 상태를 정리한다. | 실제 관문 전환 화면은 미검증이다. | G02: G2 연출 첫 구간부터 4명 모두 카드가 사라지는지 확인. |
| 마리오 NPC 광대의 마지막 망치 휘두르기에 FX | Object `마리오_광대` Parent의 `뿅망치_휘두르기_마무리`, 1367ms clip 끝에 원본 FX 1회, 약 1401ms tail. 실제 Server-spawn REUP NPC도 같은 cue 사용. | 원본 notify 시각을 clip 끝으로 옮긴 것은 요청 기반 튜닝. 플레이어 Mario Q에 임의로 FX를 추가한 것이 아니다. | G05: 마지막 타격 직후 1회 발생, 공격 취소 시 예약 FX 미발생 확인. |
| 카드미로 진입에 기본 무기→뿅망치 | 여섯 정상 class가 MAZE snapshot을 받으면 기본 무기를 숨기고 기존 WhirlwindHammer 모델을 오른손 본에 장착, 이탈 시 stance 무기 복구. | 보스 컷씬 망치와 별개 consumer다. 직업마다 손 부착 화면 확인이 남는다. | G04: 여섯 직업에서 진입·공격·이탈을 순서대로 확인. |
| 카드미로 LMB 수평 공격/Q 점프 내려치기와 All Effects 슬롯 | 직업별 원본 clip 12개, LMB 1초/Q 2.5초, 기존 typed command와 Server 타격 연결. All Effects에 두 저작 슬롯 추가. | FX 두 문서는 사용자가 붙이도록 비워 두었다. LMB .4초 타격은 프로젝트 튜닝이다. | G04: 오른쪽→왼쪽 LMB, 점프 후 Q 내려치기, 각 타격 1회, W 중복 실행 없음. |
| 카드미로 쿠크 손에 망치가 안 생김 | P77의 실제 World cinematic Kouku 모델·pose를 BOSS bone anchor로 사용. 전투 NPC 골격을 잘못 참조하던 경로 교정. | 플레이어 망치 또는 휠윈드 값과 혼동하지 않는다. | G04: P77 연출에서 쿠크 손에 생성되고 손을 따라 움직이는지 확인. |
| 스크린샷의 망치 TRS 저장 | **쿠크 휠윈드**에 Pos `(1.25,0,0)`m / Rotation `(0,180,0)`도 / Size `(2,2,2)` 설치. P77은 별도 instance로 기존 offset 보존. | 이 숫자는 카드미로 플레이어 손 튜닝값이 아니다. 저장 거절 검사를 제거하지 않았다. | G04: F1 휠윈드 패널 값, P24 휠윈드 부착, 저장 후 재열기 확인. |
| 플레이어 카드미로 망치 별도 F1 Pos/Rotation/Size·Save | `Card Maze Player Hammer`에서 cm 단위 손 offset, degree 회전, 축별 size를 맵 profile에 저장한다. | 기본 Pos/Rotation 0, Size 1. 직업별 원본 손 기준 위에 추가된다. | G04: 한 축씩 소폭 변경→Save→Reload, `Reset player hammer`로 원복 확인. |
| 칼날 Y·일반/즉사 수치·FX 분리 | Parent 중앙 `Parent Map Position`으로 연결된 WORLD Motion들을 동일 XYZ만큼 이동. 일반·즉사 모션/FX/collider는 별도 유지. | 부모 이동은 preview 좌표다. 전투는 저장 패턴의 World box placement를 사용한다. | G05: 부모 Y 변경 때 전체가 함께 이동하고 emission 간격은 유지되는지 확인. |
| 칼날·갈고리 hot reload 즉시 표시와 Play 충돌 | 좌표 편집 시 이전 Preview-at-Character override 해제 후 같은 clock 재샘플. `Saved Pattern`→`Play (with Collisions)`로 Server 판정 연결. | dirty Object 또는 게시 중에는 Server Play가 비활성화된다. `Visual Play`에는 피해·끌림 권위가 없다. | G05: 저장 후 P19 갈고리/P31 일반/P95 즉사 재생, 잡힘·끌림·종료 확인. |
| 칼날 FX가 끊기지 않도록 | finite emission 1초 주기를 tail 수명과 분리해 이전 tail과 다음 emission을 겹쳐 유지한다. | 모션/행의 명시적 끝은 유지한다. 영구 무제한 생성으로 바꾼 것은 아니다. | G05: 여러 주기 관찰, seek/재생 시 빈 구간과 종료 후 잔존 여부 확인. |
| 도화가 1.6배·차원술사 .7배·광기 광대 .7배, 전체 크기 패널 | 현재 catalog 기준 상대배율 적용. Artist 유효 scale 2.4, DimensionMaster .735. 전체/6직업/Madness/Mario를 맵별로 튜닝 가능. | 화면용 root scale이며 Server collider·사거리·피해 범위는 변경하지 않는다. Mario 기본은 1배 유지. | G03: 기본값과 로컬·원격 크기, Save/Reload 후 유지 확인. |
| 광기 100% 광대는 망치 숨김, Mario 광대만 들기 | 광기 변신과 후속 stance snapshot 모두 무기 숨김 유지. Mario는 기존 뿅망치 표시. | 일반 플레이어 MAZE의 뿅망치는 별도 요구로 유지한다. | G03: 일반 광기→복귀→Mario를 비교하고 다음 snapshot에도 정책이 유지되는지 확인. |
| 앵콜 세이튼 회전 저장, G3 모델·지팡이·패턴 재사용 | F1 yaw preview/save 복구, G3 Saydon과 동일 model/staff. `Reuse Gate 3 attacks on Encore`로 기존 일반 공격 ID 재생. | G3 전용 layout/World/mechanic은 빙고에서 재사용 거절. 모든 G3 패턴의 무조건 호환을 의미하지 않는다. | G06: 모델·지팡이, yaw 저장, 일반 공격 1개 재생, 전용 기믹의 거절 확인. |
| 빙고 원본 폭탄 본체·심지·폭발 | 빙고 본체 1배 분리, 설치 후 2초 심지→폭발 및 3301ms FX tail. Server 보드 판정 기존 경로 유지. | 기존 Object 폭탄 2배·쇼타임 3배는 보존한다. | G06: `Bingo_Bomb`으로 실제 설치/심지/폭발/십자 변경 시각 확인. |
| 빙고 망치 선행 FX·메두사 얼굴 | 망치 네 방향 template에 선행 aura. P94 원본 32_01→32_02→32_03 및 1초 얼굴 cue 연결. | 원본 disabled 얼굴 notify를 빙고 파생 alias에만 활성화. P94 새 시선·공포 피해 로직은 추가하지 않았다. | G06: `Bingo_Hammer`, P94 `메두사공포` 재생으로 원본 얼굴 방향·크기 확인. |
| 일반 해골 경계 영구 유지·생성 flip | `월드오브젝트_빙고일반해골` Parent의 `일반해골_유지`·`일반해골_생성_플립`·`빙고해골_유지`. Server 칸 제거까지 반복한다. | 조커 애니메이션을 직접 이식하지 않고 원본 보드 flip(2 turns/s, .5초 회전+.5초 burst)을 사용했다. | G06: Play1 흰칸 flip/유지→Play2 붉은빙고→Reset 제거, 최소 18초 경계 연속 관찰. |
| 거미 카운터 collider 피격 때 공포 얼굴 반복·크기 | 실제 hit→Darkness/Fear 결과가 원본 screen PS 12 emitters를 재생. 원본 크기 곡선을 유지하고 1초 반복 cadence만 튜닝. | 카운터 성공 자체에 얼굴을 무조건 띄우는 기능이 아니다. FEAR 3초/FX delay 1초를 보존했다. | G04: P15에서 피격/회피/카운터 성공을 구분해 hit 대상 화면의 반복과 크기를 확인. |
| ALT V 액자·capture 45도, 큐브 안 블렌딩 | 같은 frozen Color/Bloom pair의 crop·UV 중심을 native178 cube에 연결. 원본 액자 5개에 같은 회전/방향별 수축, 누락된 2행 추가. | capture 생성 누락 수정이 아니다. PROJECT_TUNED UV adapter이며 원본 capture camera CB 완전 복원이나 모든 3D 면의 픽셀 일치까지 검증하지 않았다. | G07: 액자와 영상이 함께 45도 회전·수축, 큐브 내부 영상/외곽 FX가 이어지는지 확인. |
| ALT V 압축 방향·속도·rotation Detail 패널 | Shrink Duration, 네 Edge Speed, Destination Offset, Capture Rotation, Square Capture, Use Model Center, 45도 preset 제공. | source capture mesh 18/31은 숨김 유지하여 이중 capture를 만들지 않는다. | G07: 방향 비대칭과 속도를 하나씩 조절하고 Apply/Save 후 재열기 확인. |
| G2 약15048ms 검은 쿠크·세이튼 | 13950~19490ms camera3 shot에서 SOURCE_CHARACTER directional만 임시 복구. | 전역 directional 상시 복원이 아니다. | G04: 14~19.5초의 두 몸체/장비와 shot 종료 후 환경 복귀 확인. |
| 연출 안개 끄기 | 쿠크/발탄 활성 cinematic camera 동안 fog 해제, 끝나면 기존 region 값 복원. | 전투 fog를 영구 제거하거나 시작 위치만으로 재설계한 것은 아니다. | G02/G08: 연출 중 뿌연 막 없음, 종료 후 전투 환경 복귀 확인. |
| 발탄 입장·벽 파괴·2페이즈·버러지들·사망 카메라/FX | 원본 actor/clip/camera/WorldSequence를 기존 pattern consumer에 연결. 입장 일리아칸 body/weapon/손 FX도 포함. | FX 원본 7구간 보류, roar16구간은 저장 WorldSequence까지만 연결. 아래 G09에 정확히 열거한다. | G08의 6개 pattern을 개별 재생하고 마지막에 자연 전투 전환을 확인. |
| 발탄 All Effects Stage/Animation 편집 | 정본 Workbench Pattern→Stage→Animation, 기존 Effect/Sound 편집·Complete Play 연결. | 보조 actor·bone·원본 World FX 배치는 World/Object 문서 소유다. Matinee 전체를 새 Workbench row로 전환한 것은 아니다. | G08: 각 pattern의 stage/clip 존재와 Animation Play/Complete Play 역할 확인. |
| 유령 발탄 생성·부활 Play/Full Restore 누락 | normal/ghost assembly 교체와 Respawn1 cue, 원본 action/clip join 복구. 실패 시 기존 presentation 유지. | 단순 랜덤 nav 스폰 성공만으로 부활 연출을 통과 처리하지 않는다. | G08: 부활 pattern의 ghost 몸체·장비 생성, FX/Full Restore 연결, 실제 전투 사용 확인. |
| 3연속 발탄 삼각형 지름 1.5배 | 반경 9→13.5m, 지름 18→27m. 1.3초 이동 시간 유지를 위해 속도도 조정했다. | 실제 nav/화면 경계와 충돌 범위는 사용자 확인이 남는다. | G08: 세 꼭짓점 거리 확대, 이동 시간, 이탈·조기 종료 없음 확인. |
| Stage와 Effect/Sound/Logic/Summon/World 수명 독립 | 쿠크 Stage 합계로 다음 패턴 진행, 이전 행은 원래 clock·source revision·owner/ledger로 수명 유지. 자동 다음 Flow Entry도 보존한다. | 명시 Stop/수동 Restart/new run/사망/관문 변경은 취소. 발탄은 이미 시작한 자연 종료 FX·one-shot sound 유지까지이며 미래 cue 시작까지 확장하지 않았다. | G10: 짧은 Stage/긴 행, 긴 Stage/짧은 행, 자동 다음 Entry, 명시 Stop을 따로 확인. |

## G02. 1관문 밝기·카드·관문 전환

1. Lobby 승인으로 쿠크에 입장하고 `Complete Play - Sequences + Pattern Flow`에서 1관문 진입 흐름을 재생한다. 단순 UI preview는 실제 카드 배정 검사가 아니다.
2. 1관문 연출 중 fog, 전투 첫 화면의 바닥·보스·광원 밝기와 머리 카드를 함께 확인한다. LUT의 색감은 남아 있으므로 전체가 무채색이 되는 것을 기대하지 않는다.
3. 1~4명 구성마다 전투 진입 직후의 카드 문양을 기록한다. 4명이면 하트·클로버·스페이드·다이아가 각각 하나씩이어야 한다. 빨간 스페이드나 검은 하트도 허용된다. 색 균등 분배는 요구하지 않는다.
4. 수 초 기다리며 유효한 카드가 이유 없이 재추첨되지 않는지 확인한다. 관문 종료 후 G2 연출 시작 시 모두 제거되는지 확인한다.

## G03. 캐릭터 크기와 광대 무기

1. F1 `Player Follow Camera`에서 현재 `Camera map`을 선택한다. `Character Size`의 `All characters`, 각 class, `Madness clown`, `Mario clown`을 확인한다. `Requested size defaults`는 전체1/Artist1.6/DimensionMaster.7/Madness.7/Mario1 기준이다.
2. Artist와 DimensionMaster의 같은 카메라·위치 화면을 비교한다. 카메라 FOV/거리 변경을 캐릭터 크기 변경과 혼동하지 않는다. 이 Client에 표시되는 로컬·원격 캐릭터 모델은 이 PC의 같은 맵 profile을 적용받는다. 설정을 다른 참가자 PC로 네트워크 전파하는 기능은 아니다.
3. `Save camera settings`→`Reload saved` 후 숫자와 실제 크기가 유지되는지 확인한다. 값을 시험했다면 사용자가 원하는 최종값을 다시 저장한다.
4. `KoukuSaydon Arena`→`Clown`의 `Change to Clown` / `Return to Player`로 외형과 무기 정책을 먼저 확인하고, 실제 광기 100% 변신에서도 반복한다. `Kouku UI Preview (Debug)`의 gauge 조절은 화면 전용이므로 실제 변신 검사로 세지 않는다.
5. Mario 진입에서는 망치가 보이고 일반 광기 광대에서는 보이지 않아야 한다. Mario 복귀 뒤 정상 몸체와 원래 무기까지 확인한다.

## G04. 2관문 조명·세 종류의 망치·거미 피격

1. G2 진입 연출의 약15048ms에서 쿠크/세이튼이 검은 실루엣으로 사라지지 않는지 본다. 해당 shot 밖에서도 directional이 계속 남아 다른 구간을 밝히지 않는지 확인한다.
2. Boss Tool의 P24 `쿠크_휠윈드`를 선택한다. F1 `Kouku Whirlwind Hammer Transform`의 m/degree/Size 값과 실제 손 부착을 확인한다. 초기 설치값은 `(1.25,0,0)/(0,180,0)/(2,2,2)`다.
3. P77 `카드미로연출`을 재생한다. 이 컷씬의 쿠크 망치가 생성되고 실제 손 pose를 따라가며 휠윈드 조절의 영향으로 옮겨지지 않는지 확인한다.
4. 실제 카드미로 진입 트리거를 발동한다. 화면에 G 상호작용 안내가 나오면 G로 실행한다. 정상 class 몸체의 기본 무기가 숨고 손에 망치가 나타나야 한다. LMB 수평 공격/Q 점프 내려치기와 Server 결과를 확인하고, 이탈하면 원래 stance 무기가 복원돼야 한다. 여섯 class를 각각 확인한다. 필요 시 `HUD mode`의 `Card Maze`는 보조 presentation 확인으로만 사용하고 실제 진입 트리거 검사를 대신하지 않는다.
5. 플레이어 망치는 `Player Follow Camera`→`Card Maze Player Hammer`의 `Hammer position (cm)`, `Hammer rotation (deg)`, `Hammer size`에서 시험한다. 보스의 1.25m 값을 여기에 옮기지 않는다. `Reset player hammer`는 0/0/1로 되돌린다.
6. All Effects에서 class를 선택하고 `CARD MAZE SKILLS`의 `Skill | LMB | 카드미로 휘두르기`, `Skill | Q | 카드미로 내려치기`를 연다. `Play animation`으로 clip, `Open Effect`로 빈 저작 문서를 확인한다. FX가 없는 것은 사용자가 직접 추가하기로 한 현재 상태다.
7. P15 `쿠크_거미카운터`를 Server 재생하고 collider에 실제로 맞은 경우를 관찰한다. hit→FEAR 상태 동안 1초 delay 뒤 얼굴/마스크가 크기 곡선을 유지하며 반복되고 FEAR 끝에 정리돼야 한다. 별도로 회피와 카운터 성공을 확인하여 비피격자에게 얼굴이 임의 발생하지 않는지 비교한다. 이 검사는 `Fire fear status word` 같은 UI 문자 preview로 대체하지 않는다.

## G05. Mario NPC·칼날·갈고리

1. Object의 `마리오_광대` Parent에서 `대기`와 `뿅망치_휘두르기_마무리`를 재생한다. 약1.367초 clip 끝에 FX가 1회 발생하고 잔여 FX가 자연스럽게 사라지는지 확인한다. 실제 Mario 구간의 NPC 공격도 같은 방식으로 확인한다.
2. 칼날 Parent의 `Parent Map Position` Y를 소폭 바꾼다. 여러 emission이 같은 delta로 움직이고, 선택한 개별 오브젝트만 따로 이동하지 않아야 한다. 화면이 이전 character anchor에 고정돼 편집을 숨기지 않아야 한다.
3. 일반/즉사 모션을 따로 선택하고 수치·FX가 섞이지 않는지 확인한다. 여러 emission 주기 동안 FX가 켜졌다 비는 틈이 없는지 본다.
4. 원하는 Object와 Composition placement를 저장·게시한 후 `Server Collision Playback`의 `Saved Pattern`을 고른다. P19 `3관문_갈고리만_확인용_불없음`, P31 `3관문 칼날 바닥`, P95 `즉사 칼날 | Object 판정 테스트`가 현재 확인 대상이다. 갈고리 Parent 기본 모션은 게시된 `hook_diagonal`이다.
5. `Play (with Collisions)`에서 실제 잡힘·끌림·일반 피해·즉사를 확인한다. `Stop Server Play`와 정상 종료에서 끌림/FX가 정리되는지 확인한다. P95는 수동 audition 패턴이며 자동 RaidFlow에 새로 넣지 않았다.

## G06. 빙고와 앵콜 세이튼

1. 빙고에 입장한 뒤 `Bingo Encore Rotation`을 조절하고 저장·재독한다. 저장 즉시 보이는 것은 authoring yaw preview이며 Server 회전 변경까지 완료된 것으로 세지 않는다.
2. Boss Tool에서 `Reuse Gate 3 attacks on Encore`를 켜고 저장된 G3 일반 공격을 `Play Isolated`한다. 같은 몸체/지팡이와 clip을 확인한다. 전용 layout/mechanic은 원래 G3에서 확인하며 빙고 재생 거절 사유를 기록한다.
3. F1 `KoukuSaydon Arena`→`Bingo Board`에서 `Bingo_Play1`을 누른다. 흰칸 0~2가 생성 flip 후 경계 유지로 이어져야 한다. 최소18초 관찰한다. `Bingo_Play2`는 3~4를 채워 첫 줄을 빨강으로 바꾼다. `Bingo_Reset`에서 잔존 FX가 제거되는지 확인한다.
4. `Bingo_Bomb`으로 MARKED→설치→2초 심지→폭발/십자 변경을 확인한다. 원본 1배 본체, 심지 위치와 폭발 tail을 함께 본다. `Bingo_Hammer`로 선행 aura가 낙하/쓸기와 맞는지 확인한다.
5. P94 `메두사공포`를 재생해 뒤돌기와 쿠크 얼굴 표시를 확인한다. source DRAFT 표시는 유지되지만 최신 게시 패턴이 있으므로 도구의 실제 admission으로 재생한다. 새 시선 판정·공포 피해를 추가한 변경으로 평가하지 않는다.
6. Object에서 `월드오브젝트_빙고일반해골`의 세 모션을 각각 열어 생성 flip/흰 유지/붉은 유지가 분리돼 있는지 확인한다. 기존 floor와 해골 효과가 이중으로 겹치지 않는지 본다.

## G07. 차원술사 ALT V

1. 차원술사를 선택해 실제 ALT V를 사용한다. 시작 장면이 고정되고 액자와 capture가 함께45도 회전·압축된 뒤 큐브 내부의 같은 장면으로 이어지는지 확인한다. 액자 누락, 과도한 texture clamp, 한쪽 Color/Bloom만 다른 crop이 보이는지를 기록한다.
2. All Effects에서 해당 ALT V 저장 문서의 `altv.authored.starting-scene-capture`를 선택하고 `Effect Detail`의 `Screen Post Profile` 아래 capture 설정을 연다.
3. `45 degree capture into cube`를 기준으로 `Capture Rotation (deg)`, `Square Capture`, `Use Model Center`를 확인한다. 압축 방향은 `Left/Right/Top/Bottom Edge Speed`, 전체 속도는 `Shrink Duration (s)`, 종착 위치는 `Destination Offset (UV)`다.
4. 한 번에 한 설정만 바꿔 액자와 영상이 같이 따라가는지 본다. 적용 후 `Save`하고 문서를 다시 열어 저장값과 재생을 확인한다. 원래 사용자 tint/visibility/cube opacity를 불필요하게 초기화하지 않는다.
5. 16:9 외 화면비를 사용하는 경우 그 비율도 확인한다. 자동 검사는 UV/crop과 실제 원본 액자 geometry의 수치 연속성에 한정되므로, 2D→3D 전체 silhouette와 FX blending의 최종 자연스러움은 여기서 판정한다.

## G08. 발탄 원본 연출·유령·삼각형

Valtan Arena에서 아래 순서로 개별 `Complete Play (Server/Arena)`를 실행한 뒤 실제 전투 흐름에서도 확인한다. 각 재생 사이에는 이전 재생의 정상 종료 또는 명시 Stop을 확인한다.

| 순서 | Pattern ID / 표시 이름 | 핵심 관찰 지점 |
|---|---|---|
| 1 | `VALTAN_ENTRANCE_CINEMATIC` / 발탄 등장 컷신 | 24.708초 원본 카메라, 일반/colorless 발탄, 일리아칸 원본 몸체·무기·두 손 FX. actor가 원점으로 튀거나 장비가 손에서 분리되지 않는지. |
| 2 | `VALTAN_SIX_PIZZA_106` / 중앙이동 후 6방향 공격 후 피자 패턴 | STEP_04/05의 벽 파괴 camera 전환과 기존 gameplay 시점. 별도 roar WorldSequence의16 FX가 자동으로 모두 따라온다고 기대하지 않는다. |
| 3 | `VALTAN_ARENA_BREAK_109` / 중앙 이동 후 2페이즈 컷씬 | phase/wall authority 1600ms 유지, pattern2600ms부터 원본5.5초 actor/camera, RECOVERY2700ms. 연출 끝이 잘리지 않는지. |
| 4 | `VALTAN_TRASH` / 버러지 패턴 | STEP_05/06의6.374초 원본 actor/camera/FX, 원본 reverse/blend와 이동. |
| 5 | `VALTAN_GHOST_RESPAWN_AUDITION` / 3페이즈 망령화 발탄 부활 | ghost body/weapon/armor가 실제 생성되고 부활 clip과 FX가 재생되는지. All Effects `Full Restore`에서 매칭된 원본을 열 수 있는지. |
| 6 | `VALTAN_GHOST_DEATH_AUDITION` / 3페이즈 발탄 사망 | ghost 원본23초 finale, 알파 곡선과 원본 이동, 끝나기 전 clear UI가 덮지 않는지. 실제 DEAD 전이도 별도 확인. |
| 7 | `VALTAN_GHOST_PORTAL_ONCE` / 망령 포탈 돌진 1회 | 해당 패턴의 3개 소환물이 만드는 지름27m 삼각형, 기존1.3초 이동 시간, nav 경계에서 중도 소멸·조기 종료 없는지. |

Workbench의 현재 정본은 `Valtan.gameplay.json`/`Valtan.presentation.json`이다. retired `Valtan.pattern.json`의 구형 RECOVERY870을 현재값으로 읽지 않는다. Pattern의 `Animation Play`만으로 동반 actor·카메라·World FX가 포함된 전체 연출을 확인했다고 기록하지 않는다.

## G09. 발탄에서 보류된 원본 구간

최종 연결은 52 asset/89 FX 시간 구간이며 실제 Codec Load/Drawable/Stage가 전부 통과했다. 구성은 입장16/버러지들8/roar16/사망39/2페이즈10이다. **roar16구간은 저장된 WorldSequence 미리보기 범위이며 별도 자동 전투 연출 소비까지 완료한 항목이 아니다.**

아래 5개 원본 시스템의7구간은 prewarm 전체 실패를 막기 위해 연결만 보류했다. 원본 recipe/material은 보존했다. 이 누락은 새 화면 회귀로 잘못 기록하지 말고 남은 복구 범위로 관리한다.

| 원본 SCENE06A 구간 | 보류 FX | 현재 거절 사유 |
|---|---|---|
| Matinee53 actor75, 2260~8309ms | `par_m_ghostmeteor_01` | locationemitter family/cardinality 미지원 |
| Matinee53 actor76, 4439~10593ms | `par_m_ghostmeteor_loop_02` | 원본 lifetime 하한0이 현재 Detail 양수 계약과 불일치 |
| Matinee53 actor77, 0~678ms와678~15846ms | `par_m_gravityarea_01_1` | meshmaterial module 미지원 |
| Matinee55 actor92, 3515~7003ms | `par_j_arktrail_02_cine` | 원본 lifetime `[0,0]`이 현재 Detail 양수 계약과 불일치 |
| Matinee55 actor98, 3810~4889ms와4889~7003ms | `par_c_lightning_001` | subuvdirect module 미지원 |

사망 마지막 emitter의 lifetime `[0,0]`→`[6,8]` 수정은 이 보류항목과 다르다. 해당 사망 emitter는 원본 seeded lookup에서 정확한6~8초 범위가 확인돼 교정했으며, 원본 의미를 추측해 양수로 바꾼 것이 아니다.

## G10. Stage와 행 수명의 독립 확인

1. 쿠크 Action Workbench에서 검토할 패턴의 Stage `Duration ms`와 Effect/Sound/Logic/World/Summon 행의 시작·끝을 먼저 기록한다. 실험은 보존할 원본 수치를 기록한 패턴에서 진행한다.
2. Stage보다 긴 Effect/Sound 행을 두고 Stage를 줄인다. 행의 시작·길이가 자동으로 줄거나 마지막 Stage가 다시 늘어나지 않아야 한다. `Full lifetime ms`와 옆 `Apply`는 패턴 전체 타임라인 수명만 바꾸며, 개별 행의 시작·길이와 Stage/Animation 시각을 유지한다. 기존 행의 끝보다 작은 전체 수명은 거절한다.
3. Save→재열기→게시 후 자동 Pattern Flow로 재생한다. 이 조건에서는 Stage가 끝나면 다음 패턴이 진행하고 이전 행은 원래 시계로 끝까지 이어져야 한다. `Play Isolated`는 단일 패턴 검사이며 후속 패턴 자동 전환 검사로 세지 않는다. 반대로 Stage를 늘려도 짧은 Effect/Sound가 보스 진행을 조기에 끝내면 안 된다.
4. 지연 World/Logic/Summon이 있는 승인된 테스트에서 다음 보스 패턴의 상태를 덮어쓰지 않으면서 원래 owner 기준으로 한 번만 발생하는지 확인한다. 현재 boss를 따라가도록 저작된 행은 현재 위치를 읽지만, 시작 원점과 판정 ledger는 이전 occurrence의 것이다.
5. 자동 Pattern Flow의 다음 Entry에서도 잔여 행이 이어지는지 확인한다. 수동 재시작/새 실행/Stop/사망/관문 전환에서는 잔여 행이 취소돼야 한다. 두 동작을 같은 조건으로 평가하지 않는다.
6. 발탄은 이미 시작한 `stopWithClip=false` V2, V1 NATURAL, one-shot sound의 자연 종료를 확인한다. 아직 시작하지 않은 이전 Stage의 미래 cue를 새 Stage에서 시작하는 동작까지 구현된 것으로 평가하지 않는다.

## G11. 자동 검사 결과와 사용자 결과 기록

최종 Debug Product 빌드는 Engine/Shared/Server/Client compile·link·배포 PASS다. 새 binary에서 bundle/card-maze/bingo/object-overlap 계약은 모두0 failures. 독립 row 저장·재열기와 발탄 presentation 계약도 PASS다. 삼각형 기대반경을13.5로 교정한 실제 RevisionProtocol 전체 focused 검사는0 failures다. 이것은 아래 남은 회귀를 포함한 모든 검사가 통과했다는 뜻이 아니다.

| 구분 | 현재 상태 |
|---|---|
| 쿠크 Support 새 독립 수명 검사 | 지연 Logic/World/Summon, 다음 primary, source/current 원점, GC pin, counter/shield, 자동 Entry, Stop/expiry 모두 PASS |
| Support 전체의 기존4실패 | finite-card lifetime burst / swept-card contact explosion / tracker half-speed / repeated-tick tracker. 현재 해결 완료로 표시하지 않는다. |
| 기타 광역 검사 | 기존 mixed-model editor fixture, catalog/world revision 기대 차이와 이전 광역 실패가 남아 있다. 마지막 focused PASS로 전체 회귀 PASS를 대체하지 않는다. |
| 발탄 FX admission | 최신52개 모두 PASS. 보류7구간은 G09 참조. |
| ALT V 수치 검사 | frozen Color/Bloom WARP21,678검사, 실제 액자 CModel1,514검사, codec164,942검사 PASS. 전체 게임 화면은 미검증. |
| 실제 게임 화면·다인 플레이 | 에이전트 미실행 / 복구 후 최종 사용자 판정 미수신. 사용자가 앞서 실행하며 오류를 보고한 이력과 구분하고, 본 문서의 순서로 복구 후 결과를 기록한다. |

화면 결과는 `확인 항목 / 맵·직업·인원 / Pattern·Object·Effect ID / 재생 시각 / 기대와 실제 / 저장값 / PASS 또는 미확인·실패`로 남긴다. 이펙트 단독 재생은 통과했지만 Server 입력 재생이 실패했다면 두 결과를 분리한다. 밝기·방향·손 부착 실패는 해당 시각의 화면과 수치를 함께 기록하면 동일 occurrence를 재현할 수 있다.

## G12. 세부 근거 문서

- [통합 PLAN](C:/Users/user/Desktop/LostArk/.md/GB/09-20/2026-09-20_RAID_PRESENTATION_REPAIR_IMPLEMENTATION_PLAN.md), [통합 RESULT](C:/Users/user/Desktop/LostArk/.md/GB/09-20/2026-09-20_RAID_PRESENTATION_REPAIR_IMPLEMENTATION_RESULT.md)
- [렌더링·Object·충돌](C:/Users/user/Desktop/LostArk/.md/GB/09-20/2026-09-20_KOUKU_RENDERING_OBJECT_REPAIR_RESULT.md), [빙고](C:/Users/user/Desktop/LostArk/.md/GB/09-20/2026-09-20_KOUKU_BINGO_EFFECT_REPAIR_RESULT.md), [거미 피격 얼굴](C:/Users/user/Desktop/LostArk/.md/GB/09-20/2026-09-20_KOUKU_FEAR_SOURCE_REPAIR_RESULT.md)
- [카드미로 플레이어 망치](C:/Users/user/Desktop/LostArk/.md/GB/09-20/2026-09-20_CARD_MAZE_PLAYER_HAMMER_IMPLEMENTATION_RESULT.md), [Mario NPC](C:/Users/user/Desktop/LostArk/.md/GB/09-20/2026-09-20_MARIO_NPC_HAMMER_END_IMPLEMENTATION_RESULT.md), [캐릭터 크기](C:/Users/user/Desktop/LostArk/.md/GB/09-20/2026-09-20_CHARACTER_SIZE_AND_ALTV_RESULT.md)
- [ALT V 최종 cube handoff](C:/Users/user/Desktop/LostArk/.md/GB/09-20/2026-09-20_DIMENSIONMASTER_ALTV_CUBE_HANDOFF_RESULT.md): 초기 크기/ALT V RESULT 이후 완료된 frame·UV 범위는 이 문서가 우선한다.
- [발탄 최종 복구](C:/Users/user/Desktop/LostArk/.md/GB/09-20/2026-09-20_VALTAN_PRESENTATION_REPAIR_RESULT.md), [쿠크 독립 행 수명](C:/Users/user/Desktop/LostArk/.md/GB/09-20/2026-09-20_KOUKU_ROW_LIFETIME_RESULT.md)

패널 이름은 현재 `MainApp.cpp`, `WorldObjectTool.cpp`, `Effect_Tool_ResourceBrowser.cpp`, `Effect_Tool_MaterialDetail.cpp`, `Effect_Tool_Valtan.cpp`, `KoukuSaydonActionWorkbench.cpp`, `KoukuSaydonBossTool.cpp`를 읽어 확인했다. 이 가이드 작성에서는 코드·데이터를 수정하거나 Client/UI를 실행하지 않았다.
