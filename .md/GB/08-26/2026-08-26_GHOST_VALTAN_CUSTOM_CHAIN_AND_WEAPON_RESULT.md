# 고스트 발탄 커스텀 체인 저작과 소켓 무기 결과

## 1. 완료 범위

Animation Tool의 `boss.valtan.ghost` 타깃이 두 가지를 얻었다.

1. 제품 발탄과 같은 방식의 `Custom Chain` 창.
2. 제품 발탄이 드는 `ValtanWeapon.wmodel`을 자기 `b_wp_r_01` 소켓에 붙인 프리뷰.

**읽는 순서 주의.** 아래 4절은 저장 문서를 body별로 분리한 최초 설계이며, 11~14절에서 donor를
구운 뒤 18절에서 **하나로 합치는 것으로 뒤집혔다.** 최종 상태는 18절이다. 4절은 왜 한때
분리가 필요했는지와 무엇이 그 전제를 없앴는지를 남기기 위해 보존한다.

`boss.valtan`의 화면과 promotion 입력 계약은 바뀌지 않는다.

## 2. 두 프리뷰가 다른 경로인 이유

| 타깃 | `pBossArchetypeId` | 경로 | 무기 |
|---|---|---|---|
| `boss.valtan` | `BOSS_VALTAN` | `CValtanPresentationAssetService` -> `CValtan` | `CValtan::Ready_Parts`가 이미 붙임 |
| `boss.valtan.ghost` | 없음 | 일반 프리뷰 -> `CPart_Body` 단독 | 소켓 파츠 코드가 없었음 |

고스트에 `pBossArchetypeId`를 주면 `CValtan` 경로를 타지만 몸통이 `MN_RPBF_01`로 바뀌어
고스트가 아니게 된다. 그래서 일반 프리뷰 경로에 선택적 소켓 파츠 지원을 넣었다.

## 3. 무기 스케일은 실측으로 확정

두 몸통의 `b_wp_r_01` bind translation을 WSKL 섹션에서 직접 읽었다.

```text
MN_RPBF_01 (제품)  b_wp_r_01  ( 34.43719, 10.53491, 2.40201 )
MN_RPBF_02 (고스트) b_wp_r_01  (  0.34437,  0.10535, 0.02402 )
```

정확히 100배다. 본 87개와 회전 기저는 동일하다. 고스트 몸통이 100배 작게 저작됐고,
그래서 프리뷰 pre-scale이 `0.0001`(제품) 대 `0.01`(고스트)로 나뉜다. 두 몸통은 결과적으로
같은 크기로 그려진다.

`CPart_Equipment::Update`에서 socket 본 행렬이 몸통 pre-transform을 이미 담고 있으므로
무기 pre-scale은 두 저작 단위의 비율만 담당한다.

```text
제품:   weapon x 100 x (bone 0.0001)
고스트: weapon x S   x (bone 0.01)
100 x 0.0001 = S x 0.01  ->  S = 1.0
```

고스트 무기 pre-scale은 `1.0`이다. 프리뷰 yaw도 본 행렬이 담으므로 무기에 다시 넣지 않는다.

## 4. 체인 문서를 분리한 이유

`Tools/ValtanPipeline/promote_valtan_animation_chains.py`는 `presentationProfile`로
`MN_RPBF_01_AnimSet.wmodel` 하나를 고정하고 debug 문서의 clip 이름을 그 모델의 clip table과
join한다. 고스트 clip은 `ao_` 접두사를 쓰는 `MN_RPBF_02`의 것이라 제품 clip table에 없다.
같은 파일에 쓰면 promotion이 거부한다.

그래서 저작 body가 자기 문서를 소유한다.

| 타깃 | 문서 | occurrence 접두사 |
|---|---|---|
| `Valtan` | `Data/Valtan/Valtan.presentation.debug.json` | `valtan.debug` |
| `Valtan_Ghost_MN_RPBF_02` | `Data/Valtan/Valtan.ghost.presentation.debug.json` | `valtan.ghost.debug` |

schema, formatVersion, `bossArchetypeId`, `encounterId`, `chains` key 집합은 두 문서가 같다.
promotion script의 `_exact` key 검사를 깨지 않기 위해 필드를 추가하지 않았다. 고스트 문서를
실제로 승격할 때는 자기 `presentationProfile`을 가진 별도 manifest가 필요하며, 이는 3페이즈
모델 교체와 함께 결정할 별도 수직 슬라이스다.

## 5. 변경 파일

| 파일 | 변경 |
|---|---|
| `Client/Public/AnimationPreviewAssets.h` | `ANIMATION_PREVIEW_ASSET`에 선택적 무기 5필드 추가, 고스트 항목에 값 지정 |
| `Client/Public/CharacterPreviewPanel.h` | `m_pPreviewWeaponObject`, `Declares_Weapon` 추가 |
| `Client/Private/CharacterPreviewPanel.cpp` | 일반 경로의 무기 프로토타입 준비와 `CPart_Equipment` 소켓 staging, Release 정리 |
| `Client/Private/Loader.cpp` | `Ready_AnimationPreviewModels`가 Development에서도 같은 무기 프로토타입 등록 |
| `Client/Private/Animation_Tool.cpp` | `CUSTOM_CHAIN_PROFILE` 도입, 고스트 Custom Chain 진입점, 타깃 전환 시 체인 상태 초기화, idle 복귀 clip 분리 |

새 C++ 파일이 없으므로 `.vcxproj` / `.filters` 등록 변경은 없다.

## 6. 실패 경계

- 무기 4필드는 함께 선언하거나 전부 비운다. 부분 선언은 프리뷰하지 않고 상태 문자열로 거부한다.
- 무기 모델 준비 실패, socket 본 부재는 몸통까지 layer에서 되돌리고 이전 선택을 유지한다.
  `CPart_Equipment::Initialize`가 `Has_Bone` 실패를 `E_INVALIDARG`로 반환하므로 원점에 뜬
  무기를 만들지 않는다.
- 프로필이 없는 타깃에서 `Get_CustomChainFilePath`는 빈 경로를 반환하고 Load/Save가 모두
  fail-closed한다.
- 타깃을 바꾸면 `Adopt_AssetName`이 체인 step/library/입력 버퍼를 비운다. 이전에는 초기화하지
  않았고, 문서가 하나뿐이라 드러나지 않던 상태였다. 두 body가 저작 가능해진 지금은 이 초기화가
  없으면 한 body의 체인이 다른 body의 파일로 저장된다.
- 체인 정지 시 복귀 clip을 프로필이 소유한다. 굽기 전에는 고스트에 `mesh_*` clip이 하나도 없어
  이전 하드코딩(`mesh_idle_battle_1`)으로는 정지 시 마지막 포즈에서 얼어붙었다. 아래 14절의 donor를
  붙인 뒤로는 두 body 모두 `mesh_idle_battle_1`로 복귀한다.

## 7. 자동 검증

- `Client\Default\Client.vcxproj` x64 Debug `ClCompile`: PASS (exit 0)
- `Client\Default\Client.vcxproj` x64 Debug 전체 Build/Link: PASS (exit 0)
- `git diff --check`: 공백 오류 없음
- 변경 파일 5개 외 코드 변경 없음

`Client/Bin/DataFiles/World/LV_LUT_HEARTRB_ED.worlddestruction*.json` 두 개가 `git status`에
M으로 보이지만 이번 변경과 무관하다. Server Release 빌드의 pre-build publisher가 다시 쓴
생성물이고 `git diff --stat`상 내용 차이는 없다. 손대지 않았다.

## 8. 미검증 — 사용자 확인 필요

에이전트는 Client 화면을 판정하지 않는다. 다음은 사용자가 직접 봐야 한다.

- Animation Tool 타깃에서 `[Boss] Ghost Valtan - MN_RPBF_02`를 선택했을 때 도끼가 오른손에
  붙어 있는지, 크기와 각도가 제품 발탄과 같은 관계인지
- `Custom Chain` 창에서 clip 추가 -> `Play Chain` -> transport(Pause/Replay/Stop) 동작
- `Save Chain` 후 `Data/Valtan/Valtan.presentation.debug.json` 갱신과 `Reload File` 왕복 (18절에서 문서를 합쳤다)
- 발탄 타깃으로 되돌렸을 때 기존 체인이 그대로 보이는지
- 체인 정지 시 고스트가 `mesh_idle_battle_1`로 복귀하는지 (16절에서 재확인)

`manual first pixel`, `eye smoke`, `visual PASS`는 사용자의 서면 관찰 전까지 기록하지 않는다.

## 9. 남은 경계

- 고스트 체인의 Product 승격 경로는 열지 않았다. promotion manifest, `presentationProfile`,
  phase, stable pattern ID 부여는 3페이즈 모델 교체 결정과 함께 별도 변경 단위로 남긴다.
- 고스트 body에는 admitted pattern master가 없어 `Valtan Pattern Master`와 source reference
  1-67 목록은 노출하지 않는다. Custom Chain만 연다.
- 고스트 `.clipseq`가 없으므로 transport 창의 sequence 목록은 비어 있다. 정상이다.

---

# 추가: 고스트 발탄 AnimSet 굽기 (같은 세션 후속)

## 10. 왜 구웠나

사용자 결정: 리맵 계층을 문서로 남기는 대신 없앤다. 두 발탄 body가 같은 clip 이름을 쓰면
다른 팀원이 체인을 저작할 때 rename 규칙을 기억할 필요가 없다.

굽기 전 상태는 런타임 attach가 불가능했다. 두 관문 모두 실측으로 확인했다.

1. `CModel::Attach_AnimationSet`은 skeleton hash가 다르면 `E_FAIL`이다.
   제품/AnimSet `0xadc0ab8374d8398c`, 고스트 `0xaeb6e340137b70a3`.
   87개 본 중 85개는 이름과 hash가 완전히 같고, exporter의 mesh container 노드 2개만 다르다.
   `mesh`/`mesh.001` 대 `rpbf_02.ao`/`rpbf_02.mo`.
2. 트랜슬레이션이 정확히 100배다. `CBone::Update_CombinedTransformationMatrix`는 pre-transform을
   루트에만 곱하므로 pre-transform으로 상쇄되지 않는다. 그대로 붙이면 본 원점이 100배로 벌어지고
   스킨 메시는 고스트 크기로 남아 찢어진다.

## 11. 굽는 도구

`Tools/ModelAssetConverter/bake_ghost_valtan_animset.py`

제품 AnimSet을 고스트 rig가 받아들이는 donor package로 재작성한다. 고치는 것은 위 두 가지뿐이다.

| 단계 | 내용 |
|---|---|
| WSKL bone[1], bone[86] | nameHash와 name을 고스트 값으로 교체 |
| WMSH bone entry 2개 | 같은 교체. decoder가 WMSH/WSKL nameHash를 교차 검증한다 |
| 애니메이션 채널 146개 | 해당 본을 참조하는 채널의 `boneNameHash` 재태깅 (애니메이션당 1개) |
| position key 779,420개 | x, y, z에 0.01 곱함. `timeTicks`는 보존 |
| `ANIMATION_TRAILER` 146개 | `skeletonHash`를 고스트 값으로 갱신 |

package의 skeleton/mesh bind 행렬은 제품 스케일 그대로 둔다. `Attach_AnimationSet`은 animation만
복사하므로 이 블록들은 컨테이너 자체 검증을 통과시키는 역할만 하고 스키닝에 도달하지 않는다.
건드리면 화면 변화 없이 위험만 늘어난다.

`--check`로 쓰기 없이 검증만 실행한다.

## 12. 산출물과 검증

```
Client/Bin/Resources/Character/Valtan/Ghost/MN_RPBF_02_AnimSet.wmodel
  46,540,308 bytes
  sha256 cb21ca2cb57a439966789bfee4a0ee9d91ee9ce521f7f9c006fd4cc2824a3998
  46,540,308 중 8,997,014 byte 변경
```

- 구운 skeleton hash `0xaeb6e340137b70a3` = 고스트 body hash: 일치
- 저장소 정본 리더(`build_valtan_rootmotion.read_root_curves`)로 root motion 재비교:
  공통 140 clip, 성분 25,971개, 구운 값과 고스트 자체 clip의 **최대 절대 차 0.0000019**
- clip 이름 literal 충돌 0건 (구운 것 146 `mesh_*`, 고스트 body 140 `rpbf_02.ao_*`)
  → `Attach_AnimationSet`의 중복 이름 관문 통과
- attach 후 고스트가 소유하는 clip: 140 + 146 = **286개**
- 어제 저작한 체인 20개 / occurrence 99개가 **전부 exact 이름으로 해석**된다

## 13. 배포 경계 — 중요

`Client/Bin/Resources/`는 `.gitignore:79`로 제외된다. 따라서 구운 46MB는 **커밋되지 않는다.**
Git에 들어가는 것은 굽는 도구뿐이고, 팀원은 clone 후 다음 한 줄을 실행한다.

```
py Tools/ModelAssetConverter/bake_ghost_valtan_animset.py
```

원본 두 에셋에서 결정적으로 재생성되므로 46MB 중복을 저장소에 올리지 않는다.

## 14. 연결한 코드

| 파일 | 변경 |
|---|---|
| `Client/Public/AnimationPreviewAssets.h` | `pAnimationSetAssetId` 추가, 고스트 항목에 donor 지정 |
| `Client/Private/Loader.cpp` | Development 경로에서 donor를 attach, 실패 시 fail-closed |
| `Client/Private/CharacterPreviewPanel.cpp` | Character Select lazy 준비 경로에서 동일하게 attach |
| `Client/Private/Animation_Tool.cpp` | 고스트 idle 복귀 clip을 `mesh_idle_battle_1`로 통일, Import가 exact 이름 우선 |

Import는 남겨 뒀다. 제품 문서를 고스트 문서로 복사하는 역할은 계속 필요하고, donor가 붙은 뒤에는
exact 이름이 그대로 해석되므로 사실상 순수 복사가 된다. `clipAliases` 6건처럼 접두사 없는 body clip은
정규화 폴백이 잡되 제품 어휘(`mesh_*`)로 해석하도록 우선순위를 뒀다.

## 15. 자동 검증

- `bake_ghost_valtan_animset.py --check`: PASS
- 굽기 실행: PASS, 위 sha256
- 구운 자산 대 고스트 clip root motion 차이 1.9e-6: PASS
- Client x64 Debug `ClCompile`: PASS
- Client x64 Debug 전체 Build/Link: PASS
- `git diff --check`: 공백 오류 없음

## 16. 미검증 — 사용자 확인 필요

- 고스트 타깃 선택 시 donor attach가 성공해 clip 목록에 `mesh_*`가 보이는지
- 그 clip으로 재생한 모션이 제품 발탄과 같은지
- 정지 시 `mesh_idle_battle_1`로 복귀하는지

사용자는 굽기 전 `ao_` clip 기준으로 이미 한 번 재생해 "모션이 발탄과 다르지 않다"를 구두 확인했다.
donor attach 이후의 재확인은 아직 없다.

## 17. 남은 작업

`Data/Valtan/Valtan.ghost.presentation.debug.json`은 굽기 **이전에** Import된 상태라 clip 이름이
`rpbf_02.ao_*`이고 step이 98개다(`mesh_idle_battle_1` 1건 드롭). 재생에는 문제가 없으나 팀 통일이
목적이므로 `Import from Valtan Chains`를 한 번 더 눌러 `mesh_*` 99 step으로 갱신해야 한다.
같은 chainId는 제자리 교체된다.

---

# 최종 상태 (18절 — 앞 절의 분리 설계를 대체함)

## 18. 문서를 하나로 합쳤다

donor를 구워 두 body가 같은 clip 이름을 쓰게 된 순간, 4절이 문서를 나눈 근거가 사라졌다.
`promote_valtan_animation_chains.py`가 고정하는 제품 모델의 clip table에 고스트 저작 결과도
그대로 join되기 때문이다. 그래서 다음을 되돌렸다.

| 항목 | 결정 |
|---|---|
| 저장 문서 | 두 body 모두 `Data/Valtan/Valtan.presentation.debug.json` |
| occurrence 접두사 | 두 body 모두 `valtan.debug` |
| idle 복귀 clip | 두 body 모두 `mesh_idle_battle_1` |
| `Valtan.ghost.presentation.debug.json` | 폐기. 사용자가 이미 삭제 |
| `Import from Valtan Chains` | 제거. 같은 문서를 자기 자신에게 불러오는 셈이 됐다 |
| `Import_CustomChainLibrary`, `Normalize_ClipKey` | 소비자가 없어져 함께 제거 |

`Parse_CustomChainDocument` / `Read_CustomChainDocument` 분리는 남긴다. Load 경로가 계속 쓰고,
문서 admission 규칙이 한 곳에 모여 있는 편이 낫다.

### Custom Chain clip 목록을 `mesh_*`로 제한한다

donor를 attach해도 고스트는 자기 `rpbf_02.ao_*` 140개를 계속 들고 있다. 합계 286개다. 재생에는
문제가 없지만 제품 body에는 그 이름의 clip이 없으므로, 공유 문서에 저작되면 promotion이 제품
모델이 재생할 수 없는 clip을 거부한다. 그래서 Custom Chain의 clip 목록은 두 body가 공유하는
어휘만 노출한다. 이는 표시상의 편의가 아니라 공유 문서의 불변식을 지키는 장치다.

## 19. 리스폰·사망 chain과 promotion manifest

사용자가 `Valtan.presentation.debug.json`에 chain 2개를 직접 저작했다.

```
respawn  -> mesh_respawn_1   playMs=0
dead     -> mesh_dead_1      playMs=0
```

manifest는 debug chain 목록과 순서까지 정확히 일치해야 하므로(`promotion manifest order/closure
must exactly match debug chains`) chain 추가만으로 이미 Validate가 깨져 있었다. 대응 항목을
추가하고 `sourceDocument.sha256`을 `ca2a0c4e...`에서 `deea1e6e...`로 갱신했다.

```json
{ "sourceChainId": "respawn", "patternId": "VALTAN_RESPAWN",
  "displayName": "3페이즈 망령화 발탄 부활",
  "authoringPhase": 3, "admissionState": "MANUAL_SERVER_AUDITION" },
{ "sourceChainId": "dead",    "patternId": "VALTAN_DEAD",
  "displayName": "3페이즈 발탄 사망",
  "authoringPhase": 3, "admissionState": "MANUAL_SERVER_AUDITION" }
```

형식 검사는 스크립트 규칙 그대로 재현해 전부 통과했다. JSON parse, manifest 최상위 key 집합,
`sourceDocument.sha256` 일치, manifest 22 대 debug chain 22의 순서 포함 정확 일치,
22개 전부의 key 집합·stable ID·중복·`displayName` 길이·phase·`admissionState`.

## 20. 남은 결정 — 팀장 인계

`--mode Validate`는 아직 실패한다. **형식이 아니라 데이터 결정이 남았다.**

```
ERROR: clip has no Valtan.animnotify source action: mesh_respawn_1
```

promotion은 clip마다 원작 Action ID를 요구한다. `mesh_respawn_1`과 `mesh_dead_1`은 AnimSet에
실재하는 애니메이션이지만 원작 Action 테이블이 참조하지 않는다. `.animnotify` 108행, `.clipmap`,
`.clipseq`, `.skilltiming` 어디에도 없다.

조사한 선택지:

- **사망**은 원작에 있다. skill `420625` "레이드 발탄_망령화 사망", 5-clip HOLD 시퀀스
  `mesh_abn_groggy_1_start` -> `mesh_att_battle_5_01_loop` -> `mesh_att_battle_5_01_end` ->
  `mesh_abn_groggy_1_loop` -> `mesh_att_battle_5_03`. 5개 전부 notify 행이 있어 계약 변경 없이
  승격된다.
- **부활**은 원작에 대응 skill이 없다. 추출 소스 전체를 검색해도 없다. 계약을 확장해
  source-action 없는 clip을 명시 선언으로 허용하지 않으면 승격 경로가 없다.

어느 clip이 연출상 맞는지는 애니메이션 의도의 문제이므로 에이전트가 정하지 않았다.
계약 확장을 택할 경우 `sourceActionIds`가 비는 것이 하류 Effect cue join에 미치는 영향을
먼저 조사해야 한다.

## 21. 이번 변경의 자동 검증

- Client x64 Debug `ClCompile`: PASS
- Client x64 Debug 전체 Build/Link: PASS
- manifest/debug 두 JSON parse와 스크립트 형식 규칙 재현: PASS
- `promote_valtan_animation_chains.py --mode Validate`: **FAIL (20절의 미결 사항)**
- `git diff --check`: 공백 오류 없음

`Client/Bin/DataFiles/World/LV_LUT_HEARTRB_ED.worlddestruction*.json` 두 개는 Server Release
빌드의 pre-build publisher가 다시 쓴 생성물이고 내용 차이가 없어 커밋에서 제외했다.
