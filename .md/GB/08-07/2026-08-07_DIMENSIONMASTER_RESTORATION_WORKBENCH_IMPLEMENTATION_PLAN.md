# 2026-08-07 차원술사 복원 워크벤치 구현 계획서

## 1. 결정

차원술사 복원은 `자동 추출만으로 완성`과 `581 Particle 전면 수작업` 중 하나를 고르는 문제가 아니다. 다음 반자동 계약으로 전환한다.

```text
UPK/Cascade 파싱
→ Imported 자동 baseline
→ 부모 Material finite profile / fail-closed
→ Authored 복원 워크벤치에서 emitter 단위 튜닝
→ Save/Reload
→ Assembly/WFX/Runtime Catalog publish
→ 고정 레퍼런스 A/B
```

자동 파이프라인은 원본 emitter, timeline, transform, mesh/texture 후보와 SourceRecipe를 채우는 importer다. 최종 형상은 차원술사 레퍼런스 PNG와 원본 파싱 증거를 함께 보며 Authored에서 완성한다.

## 2. 현재 존재하는 기능

코드 실측 결과 다음 기능은 이미 존재한다.

- `Effect Detail`의 Transform/Color/UV/Timing/Particle/Lerp 편집
- SourceRecipe module/literal/distribution 편집
- drag 중 live world preview
- Apply Detail / Apply Module / Apply Particle System
- Mesh/Base/Noise/Mask/Emissive/Dissolve 동적 slot
- DimensionMaster 전용 Mesh/Texture resource browser
- Complete/Particle/Solo/Mute audition과 sample time
- atomic Save/Save As/Reload와 dirty guard
- All Effects의 Skill/Assembly/Component/Emitter/Module 계층
- Runtime Sample과 source material/profile/resource diagnostic

## 3. 현재 끊긴 authoring loop

### G01. Apply와 Save의 공간 분리

Apply는 `Effect Detail`, Save는 화면 아래 `Data Files`에 있다. 사용자가 live preview와 메모리 commit, disk save를 서로 다른 기능으로 오해하기 쉽다.

수정:

- Effect Detail 상단에 `Restoration Session` bar를 둔다.
- `Apply + Save Authored`, `Save Authored`, `Reload Saved`, `Restart Preview`를 한 곳에 둔다.
- Detail/Module/Particle draft와 Document dirty 상태를 항상 표시한다.

### G02. Active Authored가 Published Assembly에 가려짐

현재 All Effects는 Assembly가 있으면 runtime component tree를 표시하고 active Authored element tree를 생략한다. 저장 전/후 수동 수정이 tree에 즉시 반영돼도 사용자가 볼 수 없다.

수정:

- active skill은 current Authored document layer tree를 항상 표시한다.
- Published Assembly는 `Published Runtime Hierarchy (diagnostic)` 아래 별도로 표시한다.
- 비활성 skill은 기존 published hierarchy를 유지한다.

### G03. Saved와 Published 상태 분리

`Try_SaveDocument`는 Authored JSON을 원자 저장하지만 Assembly/WFX와 Runtime Catalog를 재빌드하지 않는다.

수정:

- 저장 성공 메시지에 preview 반영과 publish pending을 명시한다.
- runtime equivalence가 참일 때만 Published와 동일하다고 표시한다.
- 이번 슬라이스에서 Client가 PowerShell publisher를 직접 실행하지 않는다. publish는 기존 검증 파이프라인을 사용한다.

### G04. 저장 성공, drawable, live preview 상태 분리

구조적으로 유효하지만 Base/Mask 같은 필수 리소스가 아직 없는 partial draft도
Authored로 저장할 수 있다. 이때 `Try_CommitDocument`는 preview를 숨긴다. 저장 성공을
곧바로 live preview 성공으로 표시하면 안 된다.

수정:

- `Saved`, `Drawable`, `Live Preview`, `Loaded Runtime Snapshot`을 별도 상태로 표시한다.
- non-drawable partial draft는 저장을 허용하되 preview hidden과 publish blocked 이유를 보존한다.
- runtime equivalence는 현재 프로세스에 로드된 catalog snapshot과의 비교임을 명시한다.

### G05. Save/Reload 동일성과 동시 writer 보호

기존 `Save_Atomic`은 임시 파일을 parse했지만 입력 문서 전체와 canonical serialize 결과를
비교하지 않았다. 또한 Tool이 문서를 연 뒤 외부 promotion이나 다른 세션이 파일을 바꾸면
그 변경을 덮을 수 있었다.

수정:

- 임시 파일 parse 뒤 `Serialize(roundTrip) == Serialize(input)`을 요구한다.
- transaction마다 고유한 temporary/backup 경로를 사용한다.
- Authored load 시 canonical baseline을 기억하고 save promote 직전에 현재 디스크와 비교한다.
- baseline이 다르면 저장을 거부하고 `Reload Saved`를 요구한다.
- Save As/New은 대상이 새로 생겼으면 저장을 거부한다.

### G06. 수동 튜닝 소유권

현재 base11 promotion은 Authored 15개를 교체한다. 수동 복원이 시작된 뒤 같은 명령을 무조건 실행하면 작업을 덮을 수 있다.

이번 경계:

- promotion receipt의 직전 `promotedSha256`와 현재 Authored SHA를 비교한다.
- 지난 자동 승격 뒤 사람이 바꾼 Authored가 하나라도 있으면 base11 promotion을 시작 전에 거부한다.
- 자동 재생성은 staging/Imported까지만 자유롭게 수행하고 Authored promotion은 명시적으로 실행한다.

후속 정밀 병합 슬라이스:

- stable element ID 기반 `Effect override sidecar`
- baseline SHA와 field mask
- parse → validate → stage → merge → commit
- 누락 element/충돌/구 baseline rollback

### G07. bounded Material 복원 편집

이번 슬라이스에서는 임의 HLSL 문자열이나 무제한 graph를 받지 않는다. 현재 Renderer가
실행할 수 있는 finite shader profile만 선택할 수 있게 하고, Imported에서 복사된 named
scalar/vector/static switch와 Dynamic semantic/SubUV 값을 Authored 문서에서 조정한다.

- Parent/원본 Material path와 parameter name/group은 provenance이므로 읽기 전용이다.
- 값이나 runtime profile을 바꾸면 상태는 `RECONSTRUCTED_PROFILE`로 내려간다.
- Base/Noise/Mask/Emissive/Dissolve/Mesh는 기존 typed resource binding 경로를 사용한다.
- 원본 Imported 파일은 수정하지 않으며 Data Files에서 언제든 다시 대조할 수 있다.

### G08. 고정 조건 A/B 캡처

- Preview에서 Screen Post를 문서 변경 없이 ON/OFF한다.
- 화면에 Active Effect ID, Sample Time, selected emitter, Screen Post 상태를 함께 표시한다.
- `Copy A/B Metadata`는 class와 pivot까지 복사한다.
- 카메라 transform/FOV/resolution은 같은 조건으로 고정한다.
- 기존 `차원술사_S00~S06`은 2050550 자료이므로 S 2050220 PASS 근거로 사용하지 않는다.

## 4. 복원 작업 순서

1. Q 2050100: Blackline Aura / Local Crack, 기본 save loop
2. S 2050220: Shine/SpriteWave, billboard/card 제거
3. W 2050120: RGBSplit/Helix/Plane mask와 alpha
4. T 2050500: dome/ring 유지, master parent와 post
5. E 2050160: `fm_h_box_01_1.wmodel` model material Solo
6. D/BA/R/F/A
7. Light/Post/V

각 emitter는 원본 Mesh/Texture/MI 후보를 초기값으로 사용하고, Reference A/B에서 큰 silhouette layer부터 고친다.

## 5. 완료 기준

- Effect Detail 한 곳에서 draft Apply + atomic Save 가능
- Save 후 같은 Authored 문서를 Reload하여 전체 canonical 값 동일
- 로드 이후 외부에서 바뀐 Authored를 stale Tool session이 덮지 못함
- 지난 자동 promotion 뒤 수동 수정된 Authored를 base11 promotion이 덮지 못함
- active Authored element/resource/module tree가 즉시 현재값 표시
- 저장 상태와 Runtime publish 상태가 혼동되지 않음
- finite runtime shader와 named Material 값이 live preview에 반영됨
- 고정 카메라 A/B에 필요한 Effect/Sample/Emitter/Post 상태가 캡처 화면에 노출됨
- Client Debug build와 Effect Tool harness PASS
- 실제 Q emitter 하나에서 edit → live preview → apply/save → reload 회귀 확인

픽셀 복원 완료는 이 워크벤치 완료와 별도다.
