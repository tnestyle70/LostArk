# 워로드 본 체인 시뮬레이션 — 미완 결과

작성자: JS · 2026-08-10 · 브랜치 `feature/warlord-skill-timing`

머리카락·천 시뮬레이션의 첫 수직 슬라이스다. **완료가 아니라 보류다.** 코드는 전부 들어가 있고
스위치만 꺼져 있다(`Character.cpp`의 `BONE_CHAINS_ENABLED`). 원인 미상 크래시와 남은 떨림 때문이며,
그 조사 내용을 다음 사람이 처음부터 다시 하지 않도록 남긴다.

## 1. PhysX로는 천을 못 만든다

팀장이 넣은 PhysX 5.6.1은 **강체 전용**이다. 천·머리카락을 여기서 못 쓰는 이유가 둘이다.

- `PhysXGpu_64.dll`이 배포에 없다. PhysX 5의 `PxDeformableSurface`와 PBD 파티클 천은 CUDA 전용이라
  이 DLL 없이는 생성 자체가 안 되고, NVIDIA GPU 없는 팀원 PC에서는 아예 못 돈다.
- `CPhysics_Manager` 파사드에 joint/articulation이 하나도 없다. 강체를 사슬로 엮을 수단이 없다.

강체 + D6 joint로 사슬을 만드는 길은 열려 있으나 네 클래스 합쳐 258개 마디라 액터 수가 감당되지
않는다. 그래서 **본 체인 시뮬레이션**을 택했고 PhysX는 쓰지 않는다.

## 2. 리그가 런타임 솔버를 전제로 만들어져 있다

네 클래스 전부 머리·천 체인을 갖고 있다.

| 클래스 | 전체 본 | 동적 본 | 주요 체인 |
|---|---|---|---|
| 창술사 | 224 | 43 | 머리 4갈래, `upper_cloth` 2×5, `upper_skirt` 6×2 |
| 도화가 | 239 | 81 | 머리 7갈래(34마디), `capatcloth` 3×4, `skirt` 8×4 |
| 워로드 | 218 | 63 | 머리 1갈래, `capatcloth` 3×5, `skirt` 8×5 |
| 차원술사 | 225 | 71 | 머리 5갈래, `capatcloth` 3×6, `skirt` 7×4 |

**결정적인 것은 클립이 각 체인의 `_01`만 실제로 회전시킨다는 점이다.** 모든 클립이 모든 본에
채널을 갖지만(쿠킹이 전부 굽는다) 회전량을 재보면 `_02` 이하는 전 클립에서 고정이다.

```text
워로드 193개 클립: 움직이는 동적 본 12개(전부 _01), 전 클립 고정 34개
```

원작이 뿌리만 애니메이터가 키하고 아래는 런타임에 푼다는 뜻이다. 그래서 작업 정의가 이렇게 된다.

> 애니메이션은 `_01`이 계속 소유하고, 시뮬레이션은 `_02`부터 끝 마디까지만 덮어쓴다.

조사 스크립트는 `list_bones.py`, `check_dynamic_channels.py`, `measure_dynamic_motion.py`로
scratchpad에 두고 커밋하지 않았다. `.wmodel` section kind 3이 skeleton, kind 4가 clip이며 bone record는
256바이트, 앞 8바이트가 name hash다.

## 3. 구현

**Engine — 본을 덮어쓸 진입점**

애니메이션 갱신과 스키닝 사이에 끼어들 자리가 없어 `CModel`에 최소한만 열었다.

```cpp
int32_t Find_BoneIndex(const char_t*) const;
int32_t Get_BoneParentIndex(uint32_t) const;
bool_t  Get_BoneLocalMatrix / Get_BoneCombinedMatrix(uint32_t, matrix_t&) const;
bool_t  Set_BoneLocalMatrix(uint32_t, fmatrix_t);
void    Refresh_BoneCombinedMatrices();
```

`combined = local * parentCombined`이고 부모 인덱스가 항상 자식보다 앞서므로 한 번의 전방 순회로
계층 전체가 다시 선다. `CBone`에는 `Get_ParentBoneIndex()`만 추가했다.

**Client — `CBoneChainSimulation`**

버렛 적분 + 길이 보존 + 이탈 제한이다. 쓰기는 회전만 한다. 본의 translation은 부모로부터의 고정
오프셋이라 건드리면 메시가 찢어지므로, 각 부모를 자식이 도착한 지점으로 조준시킨다.

본을 돌리면 그 아래가 전부 따라 돌기 때문에 **뿌리부터 한 마디씩 내려가며 combined를 다시 세운다.**
애니메이션 포즈 기준으로 전 마디를 한꺼번에 계산하면 깊은 마디가 이미 움직인 부모를 기준으로
조준하게 된다.

**연결** — `CHARACTER_SPEC`에 `pBoneChains`를 trailing optional로 추가했다(`STANCE_LOCOMOTION_SPEC`과
같은 방식이라 나머지 다섯 spec은 그대로). 구동은 `CCharacter::Update`의 `__super::Update` 직후다.
`Update_Presentation`이 아닌 이유는 로직이 `m_isLocallyControlled`일 때만 생성돼서 거기 두면 다른
플레이어의 천이 안 움직이기 때문이다.

## 4. 떨림 — 네 번의 원인이 전부 달랐다

| 증상 | 실제 원인 |
|---|---|
| 항상 최대로 펄럭임 | 강성이 낮아 평형 지연(`v·dt/k ≈ 0.29m`)이 이탈 한계 0.09m를 3배 초과 → 걷는 내내 한계에 붙어 있었다 |
| 팽팽하게 끊김 | 이탈 한계가 위치를 딱 잘라냄 → 45% knee부터 점근 압축으로 교체 |
| 여러 번 왕복 | 속도 유지율 0.78 + **60fps 초과 시 매 프레임 통스텝** → 실시간보다 빠르게 적분. 누적기로 교체 |
| 정지는 멀쩡, 이동 시 떨림 | **서브스텝이 전부 같은 목표를 겨냥** → 한 프레임에 2번 걷어차임. 이전 프레임 포즈에서 lerp하도록 교체 |

수렴 자체를 막던 것도 둘 있었다. `fRestLength`를 최초 1회만 재서 클립마다 다른 관절 거리와 영원히
싸우고 있었고(쿠킹이 translation도 키한다), 마지막 수 마이크로미터가 계속 진동했다(1mm 미만이면
애니메이션 포즈로 스냅하도록 추가).

마지막으로 **월드 공간 풀이를 캐릭터 공간으로 옮겼다.** 캐릭터의 월드 변환은 replicated라 위치가
30Hz 스냅샷을 향해 ease하고 yaw가 초당 720도로 돌아 둘 다 등속이 아니다. 월드에서 풀면 그 불균일함이
전부 체인에 가속도로 주입되며, 감쇠로는 절대 제거되지 않는다.

**그럼에도 떨림이 남는다.** 다음 의심 지점은 천이 아니라 **로코모션 상태 전환**이다. 셀 경계에서
서버 이동 상태가 깜빡이면 `Set_Locomotion`이 IDLE↔RUN을 왕복하고 클립 자체가 바뀐다. 체인은 바뀐
포즈를 새 목표로 쫓아간다. 이건 캐릭터 공간으로 옮겨도 안 없어진다.

## 5. 미해결 크래시

워로드가 Server gameplay로 스폰하는 프레임에 `Client.exe`가 `c0000005`로 두 번 죽었다.

단계 트레이스를 넣어 확인한 결과 **솔버는 매번 완주했다**(`enter → read-combined → rest-length →
settled → integrated → wrote-back → refreshed`). 크래시는 솔버가 리턴한 뒤, 써넣은 본 행렬을 소비하는
쪽에서 난다.

값 손상을 의심해 세 가지 가드를 넣었다.

- 체인 뿌리 local matrix가 특이하면(스케일 0 본) 역행렬이 무한대가 되므로 행렬식 검사 후 건너뛴다
- 시뮬레이션 위치가 NaN/Inf면 애니메이션 포즈로 되돌린다
- 본에 쓰기 직전 16개 성분의 유한성을 검사한다

**세 가드 모두 한 번도 걸리지 않았고, 그 뒤로 크래시도 재현되지 않았다**(1,800 사이클 무사고).
따라서 가드는 원인이 아니며 크래시는 여전히 미해결이다. 값 손상이 아니라 타이밍이나 상태에
의존하는 종류로 보인다.

이 PC에 디버거(`cdb`/WinDbg)도 WER 로컬 덤프 설정도 없어 호출 스택을 못 얻었다. **다음 시도는 VS에서
F5로 붙여 스택을 확보하는 것부터 시작해야 한다.**

## 6. 상태

```text
Engine / Client 빌드            성공
BONE_CHAINS_ENABLED             false (기본 꺼짐)
디버그 스캐폴드                  제거 완료
```

꺼진 상태에서는 캐릭터가 애니메이션 그대로 포즈를 잡는다. 워로드 치마 8갈래 + 망토 3갈래 튜닝값도
`Logic_Warlord.cpp`에 남아 있다(철판 `0.45/0.40/1.5/0.012`, 철망토 `0.38/0.50/2.0/0.018`).

## 7. 다음에 할 것

1. **크래시 스택 확보.** VS F5로 워로드 Server Play 재현. 이게 선행 조건이다.
2. 로코모션 IDLE↔RUN 깜빡임 확인. 클립 전환 로그 한 줄이면 판정된다.
3. 충돌 캡슐 미구현. 지금은 천이 다리를 뚫을 수 있다.
4. 체인 파라미터가 `Logic_Warlord.cpp` 상수다. 클래스가 늘면 `Data/Animation/Authored`의 저작 문서로
   옮기는 게 맞다. 지금 옮기면 형식을 성급히 고정하게 되므로 보류했다.
5. 나머지 세 클래스는 본 조사까지만 끝났다. 워로드가 닫히면 데이터만 추가하면 된다.
