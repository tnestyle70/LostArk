# 천 시뮬레이션 재작성 — 복리 프레임 솔버 완료

작성자: JS · 2026-08-10 오후 · 브랜치 `feature/warlord-cloth-bone-chain`

오전 문서(`2026-08-10_LOSTARK_WARLORD_BONE_CHAIN_RESULT.md`)의 솔버를 폐기하고
SpringBone 계열 구조로 재작성했다. 워로드 치마 8갈래 + 망토 3갈래가 켜진 상태로
실행 확인을 통과했다(`BONE_CHAINS_ENABLED = true`). 이동 중 펄럭임, 정지 시 부드러운
정착, 스킬 중 고스팅 없음까지 사용자 체감 승인.

## 1. 이 세션에서 고친 것들 — 시간순

같은 증상("떨림", "꼿꼿함", "고스팅")이 서로 다른 다섯 원인에서 나왔다. 다음에 비슷한
증상을 보면 이 순서대로 의심할 것.

### 1.1 로코모션 IDLE 왕복 — 표현 지연 적용

오전 7절에서 확정한 데드존 왕복에 수정안 A를 적용했다. `Set_Locomotion`은 IDLE
전환을 즉시 커밋하지 않고 150ms 타이머만 걸고, RUN이 그 안에 돌아오면 취소한다.
실제 커밋은 `Commit_Locomotion`이 하고 `CCharacter::Update`가 타이머를 굴린다.

체감 효과는 작았다. 관측된 IDLE 구멍 중 156ms·594ms짜리는 못 삼키기 때문이다.
남겨는 뒀지만, 근본 대안은 서버 action 상태 대신 **스냅샷 위치의 실제 이동 속도로
표현용 locomotion을 결정**하는 것이다. 미구현.

### 1.2 구 솔버의 회전 좌표계 버그

엔진 규약은 `combined = local × parentCombined`(행벡터)다. 구 솔버는 모델 공간에서
구한 조준 축을 `ParentLocal × R`로 local에 직접 곱했는데, 그 위치의 R은 위 프레임
좌표계로 해석된다. 올바른 식은 `L' = L·A·R·A⁻¹`(A = 위 프레임 combined)이다. 이
오류가 매 스텝 틀린 축으로 교정하는 세차운동성 떨림을 만들었다.

### 1.3 행렬식 임계값 함정 — 이 리그의 영구 규칙

**이 리그는 preTransform 스케일 0.0001이 모든 combined 행렬에 들어가 있어 정상
행렬식이 ~1e-12다.** `|det| ≥ 0.0001` 같은 절대 임계값 가드는 모든 본을 특이 행렬로
오판해 쓰기를 전부 스킵시킨다. 증상은 "솔버는 돌지만 화면에 아무 변화 없음"이고,
세 번의 튜닝이 전부 무의미했던 원인이다. combined 역행렬 앞에는 `isfinite && != 0`
만 검사하고, 진짜 특이 행렬은 역행렬이 무한대가 되므로 결과 행렬의 finiteness
게이트로 거른다.

### 1.4 "꼿꼿함"의 구조 원인 — 스프링이 중력을 상쇄한다

구 구조는 각 마디를 "애니메이션이 뒀을 위치"로 되끄는 스프링이라, 중력의 평형
오프셋이 `g·dt²/k` — g=4, k=0.25면 **4mm**다. 중력을 아무리 올려도 스프링이 즉시
상쇄하고, 바인드 포즈로 뻗친 망토는 그대로 꼿꼿하다. 휴식 방향을 아래로 기울이는
hang bias를 중간에 시도했지만, 처짐이 마디별로 독립이라 체인 전체가 드레이프되지
않는 한계는 그대로였다. 이 두 시도가 재작성 결정의 근거다.

### 1.5 스킬 중 고스팅 — 프레임 순서

엔진 프레임은 `Object Update → 물리 → Level Update → Object Late_Update` 순이고,
스냅샷 적용(replication)은 **Level Update**에서 돈다. 스킬 중에는 스냅샷마다
`Seek_ActiveStageForward → Play_Animation(0)`이 스켈레톤 전체를 다시 굽기 때문에,
Update에서 푼 체인 포즈가 지워진다. 스냅샷은 30Hz라 격프레임으로 "시뮬 포즈 ↔ 순수
애니 포즈"가 번갈아 렌더돼 잔상처럼 보였다. **솔브를 `CCharacter::Late_Update`로
옮겨서 해결했다.** 스켈레톤을 늦게 다시 굽는 주체가 생기면 같은 증상이 재발한다.

## 2. 새 솔버 구조 (`Client/Private/BoneChainSimulation.cpp`)

핵심 원리 두 가지.

**복리 프레임.** 각 마디의 휴식 방향을 부모의 *이미 시뮬레이션된* 프레임 안에서
계산한다(`target = ChildLocal × ParentFrame`, ParentFrame은 걷기 중 누적 갱신).
마디 1이 처지면 마디 2는 그 처진 프레임 위에서 또 처지므로 끝단에 합이 실린다.
드레이프는 이 누적에서 나오지 개별 스프링에서 나오지 않는다.

**힘이 모양을, 스프링은 추종만.** 강성은 몸을 따라오는 최소한만 주고(망토 0.06),
형태는 중력 + 바람 가속이 만든다. 감쇠 0.55라 정지 시 튕길 운동량이 없다.

Update 한 번의 흐름:

```text
바람 갱신    월드 속도 → 저역 필터(5/s) → yaw 역회전으로 모델 공간 변환
             15m/s 초과 프레임은 teleport로 보고 무시
프레임 입력  전 링크의 animated local + 체인 루트의 animated combined
프라이밍     최초 1회, 링크를 애니메이션 체인 위에 배치하고 반환
적분(서브스텝) 60Hz 고정 스텝, 프레임당 최대 4스텝, 잔여 시간은 누적
  마디마다   target = Locals[i] × ParentFrame (부모의 굽은 프레임에 애니 local을 얹음)
             Verlet: 속도유지(damping) + 스프링(stiffness→target) + (중력+바람)·dt²
             길이 보존 → soft 이탈 한계(45% knee) → 1mm settle 스냅 → NaN 가드
             부모 프레임을 결과 방향으로 굽히고(Aim_Frame) 다음 마디로 하강
쓰기(1회)    이전/현재 스텝 위치를 누적기 잔여분으로 보간(렌더 보간)
             같은 walk로 부모 local을 위 프레임 역행렬로 복원해 Set_BoneLocalMatrix
             마지막에 Refresh_BoneCombinedMatrices
```

구동 위치는 `CCharacter::Late_Update`(1.5절), 바람 입력은 transform 위치와
`m_fPresentationYawDegrees`다. 솔버는 월드 변환을 직접 보지 않는다.

## 3. 파라미터 계약 (`BONE_CHAIN_SPEC`)

| 필드 | 의미 | 워로드 치마 | 워로드 망토 |
|---|---|---|---|
| `fStiffness` | 스텝당 애니 방향으로 닫는 비율. 추종 속도 | 0.12 | 0.06 |
| `fDamping` | 스텝당 속도 유지율. **0.5 초과부터 링잉** | 0.45 | 0.55 |
| `fGravity` | 모델 공간 아래 가속. 복리라 실제 중력보다 큰 값이 정상 | 12 | 20 |
| `fMaxDisplacement` | 휴식 포즈 이탈 상한(m). teleport 방어 | 0.15 | 0.35 |
| `fWindResponse` | 속도 1m/s당 이동 반대 방향 가속 | 2 | 5 |

튜닝 감각: 더 무겁게 = gravity ↑, 더 흐느적 = stiffness ↓, 더 날림 = wind ↑,
통통거림 = damping ↓. 치마는 다리 충돌이 없어서 일부러 망토보다 뻣뻣하다.

## 4. 검증

```text
Debug 정본 회귀(오전, 로코모션 A 포함)   빌드·NetworkProtocolHarness·
                                        ClientFrontendHarness·contract-test 전부 PASS
ProjectAudit                            9건 실패 — 전부 팀원 Artist 이펙트(skill.31470)
                                        워크스트림의 기존 드리프트. cdb052f가 Data 5개를
                                        vcxproj 미등록으로 커밋. 이 브랜치 변경과 무관
솔버 재작성 이후                        Client Debug 빌드 + Server/Client 로컬 실행으로
                                        수동 검증만 수행. 정본 회귀 재실행은 미수행
수동 확인 항목                          이동 펄럭임 / 정지 정착(링잉 없음) /
                                        스킬 중 고스팅 없음 / 콤보 단계 전환
```

환경 함정 둘(재현 시 참고): 에이전트 셸에 `NoDefaultCurrentDirectoryInExePath=1`이
있으면 `UpdateLib.bat` 호출이 깨진다 — 변수 제거 후 실행. audit의 python 검사는
시스템 python이 없으므로 Blender 5.0 번들(`C:\Program Files\Blender Foundation\
Blender 5.0\5.0\python\bin`)을 PATH에 얹어 돌린다.

로컬 실행: 서버는 별도 콘솔로 `Server\Bin\Debug\Server.exe --bind-address 0.0.0.0`
(백그라운드 셸은 stdin EOF로 즉시 종료된다), Client는 작업 디렉터리
`Client/Default` + `LOSTARK_SERVER_HOST=127.0.0.1`.

## 5. 남은 작업

1. **충돌 캡슐.** 치마가 다리를, 망토가 등을 뚫을 수 있다. 다리·몸통 캡슐 몇 개와
   push-out이면 치마 강성을 낮출 여지도 생긴다.
2. **나머지 세 클래스.** 본 조사는 오전 문서 2절에 있다. 솔버는 공용이므로 각
   `Logic_*.cpp`에 `BONE_CHAIN_SPEC` 배열과 `CHARACTER_SPEC` 연결만 추가하면 된다.
   머리카락 체인도 같은 스펙으로 붙는다(창술사 4갈래, 도화가 7, 차원술사 5).
3. **파라미터 저작 데이터 이동.** 클래스가 늘면 `Logic_*.cpp` 상수를
   `Data/Animation/Authored/<Asset>` 문서로 옮긴다. 형식 확정은 두 클래스째 붙일 때.
4. **스폰 크래시 관찰.** 오전 5절의 c0000005는 솔브가 스냅샷 재포즈와 같은 프레임
   단계에 있던 시절의 것이다. Late_Update 이동으로 얽힘 자체가 사라졌으므로 재현
   여부만 지켜본다. WER LocalDumps는 `C:\Users\95jus\CrashDumps`에 걸려 있고, 덤프가
   생기면 리빌드 전에 열어야 PDB가 맞는다.
5. **로코모션 속도 기반 결정.** 1.1절의 잔여 항목. 지금 150ms 지연은 임시 완화다.
