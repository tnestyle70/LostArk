# LostArk 창술사(LanceMaster) 스킬 결선 조사 결과

작성자: JS · 2026-07-30

애니메이션에 이벤트가 없다는 [이전 조사](2026-07-30_LOSTARK_LANCEMASTER_ANIM_EVENTS_INVESTIGATION_RESULT.md)의
후속으로, "그럼 이벤트/타이밍이 사는 데이터 테이블은 어디냐"를 추적했다.
**창술사 스킬 연출 결선 패키지를 특정하고 그 구조·참조를 전부 복구**했다.

## 1. 먼저 내릴 결론

1. 창술사 스킬 연출 결선의 정본은 논리 패키지 **`STANDARD_SKILLCAM_LANCEMASTER`**
   (물리 `NU5OQ5GQ9JN3PAAJ5H9UA5OJXH5NUXG.upk`)다. 클래스마다 한 개씩 있는
   `STANDARD_SKILLCAM_<CLASS>` 계열이며, LostArk의 **각성/궁극기 시네마틱을
   Kismet + Matinee 타임라인으로 결선한 패키지**다.
2. 이 패키지는 UModel이 export/view는 못 하지만("no supported objects"),
   **export table은 정상 파싱**한다(214 exports). Matinee/Kismet 클래스로
   구성되어 있어 애니·이벤트·카메라·카메라셰이크·씬이펙트가 **하나의 타임라인에
   시간축으로 얹혀 있다.** 즉 여기엔 실제 타이밍 트랙이 있다.
3. 결선 대상은 창술사 **궁극기/각성 3종**(super 계열)이다. `skillcam_lancemaster_01
   /02/03` 세 매티니가 각각의 스킬 시네마틱이다. 애니 클립·이펙트·부착 본까지
   이름 단위로 전부 확인했다(§4).
4. **정확한 float 시간 값(몇 초에 무슨 이벤트/셰이크)까지는 이 단계에서 안 뽑았다.**
   UModel `-save`로 받은 복호화 UPK는 헤더(name/import/export)뿐 아니라 export
   본문(프로퍼티 스트림)까지 난독화돼 있어(readable string은 `None` 하나뿐, 나머지
   전부 스크램블) 파일 파싱이 불가능하다. 복호화본은 UModel 메모리에만 존재하는데
   이 Kismet 클래스는 UModel이 deserialize하지 않아 구조체로도 안 남는다.
   시간 값 추출은 **복호화 export 본문의 UE3 프로퍼티 파서 제작**이 필요한 별도
   작업이다(§6).
5. **일반 전투 스킬(궁극기 아닌 ~8종)의 이펙트/사운드/타이밍 결선은 UModel로 읽는
   UPK 어디에도 없다.** `EFGame/*.lpk`(data*/leveldata*) 독자 암호화 아카이브에
   있으며 UModel은 이걸 디코드하지 못한다. 발탄 조사가 참조한 `EFTable_*` DB도
   이 `.lpk`를 별도 도구로 푼 산출물이었고, 그 도구는 이 PC에 없다.

## 2. 역매핑과 특정 방법

`*Skill*` 와일드카드로 44개 패키지가 잡힌다. 전부 클래스 기반 generic 이름이라
`-list`만으로는 어느 게 창술사인지 알 수 없다(오브젝트 이름이 `interpdata_0`,
`persistentlevel` 식). `-nameresolve`는 **물리 파일명 입력을 거부**하므로 논리
이름을 먼저 알아야 한다.

특정 절차(재현 가능):

```powershell
# 1) Skill 패키지 44개를 한 프로세스에 모두 로드 (뷰 가능한 게 없으면 즉시 종료되므로
#    -view 단독 대신 아래처럼 로드가 유지되는 형태를 쓴다)
umodel_lostark_v7.exe -view -game=lostark -kr -nameresolve `
  -path="...EFGame" *Skill*

# 2) 그 프로세스 메모리에서 문자열을 스캔 → 논리 패키지 이름이 드러남
python memscan_strings.py <PID> ".upk" all_upk.txt
#   → STANDARD_SKILLCAM_LANCEMASTER.upk 발견 (클래스별 SKILLCAM 계열 확인)
```

`STANDARD_SKILLCAM_*`는 클래스별로 전부 존재한다(BERSERKER, GUNLANCER, LANCEMASTER,
REAPER, SOULEATER, ... 확인). 이름 규칙 자체가 "표준 스킬 카메라"다.

특정 후, 이름 테이블(338개)은 발탄과 같은 방식으로 메모리에서 완전 복구했다.
뷰가 안 되는 패키지라 **뷰 가능한 메시를 메인으로, 스킬캠을 `-pkg=`로 함께 로드**해
프로세스를 살려둔 뒤 스캔했다.

```powershell
umodel_lostark_v7.exe -view -game=lostark -kr -nameresolve `
  -path="...EFGame" -pkg=STANDARD_SKILLCAM_LANCEMASTER PC_FLM_00
python find_nametable.py <PID> 338 skillcam_names.json   # 338/338 복구
```

## 3. 패키지 구조 (`STANDARD_SKILLCAM_LANCEMASTER`, 214 exports)

```text
  3 interpdata                                  ← 스킬 3종의 타임라인 컨테이너
  3 efseqact_matinee                            ← 각 타임라인을 재생하는 Kismet 액션
 16 interpgroup (+3 interpgroupdirector)        ← 트랙 그룹(플레이어/카메라/디렉터)
  3 interptrackanimcontrol                      ← 애니 재생 트랙 (어떤 클립을 언제)
  4 interptrackevent                            ← ★ 이벤트 트랙 (시간축 이벤트 발사)
 16 interptrackmove                             ← 카메라/액터 이동 키프레임
  7 interptrackfloatprop                        ← FOV 등 float 파라미터 커브
  3 interptrackdirector                         ← 카메라 컷 전환
  2 seqact_camerashake (+2 camerashake)         ← 카메라 흔들림 발사
  3 efinterptrackultimateskillcinematiccontrol  ← ★ LOA 전용 궁극기 시네마틱 제어
 24 cameraactor / 7 efskeletalmeshactor         ← 배치된 카메라·캐릭터 액터
  3 seqevent_input / 3 seqevent_remoteevent     ← 스킬 입력/원격 트리거 진입점
```

`interptrackevent` + `efinterptrackultimateskillcinematiccontrol`가 "공격/이펙트/
연출 스폰 타이밍"을 담는 트랙이고, `interptrackanimcontrol`이 그 위에서 애니 클립을
재생한다. 이름 테이블에 `starttime`, `currenttime`, `time`, `eventname`,
`eventtrack`, `linkedevents`, `sequenceevent`, `floattrack`, `interpcurvefloat`,
`animseqname`, `animseqs`, `slotname`, `matineeindex`가 모두 존재해 구조가 확정된다.

## 4. 복구한 결선 내용 (이름 단위, 실측)

### 4.1 스킬 타임라인 3개

```text
skillcam_lancemaster_01 / _02 / _03   (matineeindex 0/1/2)
```

### 4.2 애니 클립 (InterpTrackAnimControl이 재생, animset = pc_flm_00_ani)

```text
sk_super_gabolg_01 / _01_re / _02 / _02_re    게이볼그(가시타래) 계열 각성기
sk_super_squalllance_01 / _02 / _03 / _04     스퀄 랜스 계열
sk_superflashblink_01                          플래시 블링크
sk_flm_gdr_01, sk_flm_hor_00(+_sk), sk_flm_pmshb_00_sk
idle_normal, moveframe                         전이/기본
```

`_re`는 같은 동작의 리커버리/후속 클립으로 보인다. 이 이름들은 [애니 조사
문서](2026-07-30_LOSTARK_LANCEMASTER_ANIM_EVENTS_INVESTIGATION_RESULT.md)의
`PC_FLM_00` AnimSet `SequenceName`과 같은 계열이다(스킬캠이 그 클립을 참조).

### 4.3 이펙트·무기·부착점 참조

```text
fx_mi, fx_m_mi_x_00, fx_x_me_master_38_01_sk_dt_tr, pc_ft_fx_sk   시네마틱 이펙트
wp_flm_av_002, wp_flm_av_002s_sk                                  연출용 무기(오라)
b_effectroot, b_effectworldzero                                  이펙트 부착 본
```

### 4.4 카메라

```text
targetcamgroup, defaultviewx/y, defaultviewzoom      카메라 그룹/기본뷰
camoverridepostprocess(+alpha)                       포스트프로세스 오버라이드
shake0 / shake01 / shake02 / shake03 / shake04       카메라셰이크 프리셋
shakescale, oscillationblendintime                   셰이크 강도/블렌드
boverride_enablesceneeffect                          씬 이펙트 토글(궁극기 연출)
```

## 5. 결론적 아키텍처

```text
[스킬 입력]
  → SeqEvent_Input → EFSeqAct_Matinee(skillcam_lancemaster_0N)
  → InterpData 타임라인:
      · InterpTrackAnimControl  : pc_flm_00에 sk_super_* 애니 재생
      · InterpTrackEvent        : 시간 T에 named event 발사(이펙트/사운드 훅)
      · EFInterpTrackUltimateSkillCinematicControl : 궁극기 씬 연출 on/off
      · SeqAct_CameraShake      : 시간 T에 shake0N 발사
      · InterpTrackMove/FloatProp/Director : 카메라 이동·FOV·컷
```

즉 **궁극기/각성 스킬의 "공격·이펙트·연출 타이밍"은 애니가 아니라 이 SkillCam
매티니 타임라인이 소유**한다. 애니 클립은 타임라인이 불러 쓰는 재료다.

일반 전투 스킬은 이 시네마틱 매티니를 쓰지 않는다. 그쪽 결선(스킬→이펙트→사운드
→프레임 오프셋)은 `.lpk` 데이터 테이블에 있으며 현재 툴체인으로는 못 읽는다.

## 6. 남은 것 — 정확한 시간 값을 뽑으려면

두 갈래다.

1. **SkillCam 시간 값 파서(궁극기 3종 정밀 복원).**
   복호화된 export 본문(UModel 메모리)에서 `InterpTrackEvent.EventTrack`
   (`TArray<{float Time; FName EventName}>`), `InterpTrackAnimControl`의
   `AnimSeqName`/`StartTime`, `SeqAct_CameraShake` 발사 시각, `InterpCurveFloat`
   키를 파싱한다. export table(SerialOffset/Size)까지 메모리에서 복구해야 하고,
   난독화 때문에 UModel이 이미 복호화해 둔 메모리를 읽는 것이 유일한 경로다.
   발탄 ImportTable 복구와 같은 난도의 별도 작업이다.
2. **`.lpk` 데이터 테이블 디코더(일반 스킬 전체).**
   `data*.lpk`/`leveldata*.lpk`를 여는 LOA 아카이브 디코더가 필요하다. 발탄
   조사의 `EFTable_*` DB가 이 경로로 나온 산출물이며, 그 소스 카탈로그 도구는
   `Final_LostArk`(다른 작업공간)에 있고 이 PC에는 없다. 일반 스킬의 이펙트/사운드
   /타이밍 전량은 여기 있다.

**현실적 권고:** 우리 엔진에서 궁극기 연출을 붙일 거라면, 원본 float 시간을
1번으로 정밀 복원하기보다 위에서 확정한 **트랙 구성(애니 클립 + 이벤트/셰이크/
이펙트 슬롯)** 을 골격으로 우리 쪽 타임라인을 저작하는 편이 빠르다. 원본은
난독화 + 미지원 클래스라 완전 자동 복원 비용이 크다.

## 7. 산출물

```text
scratchpad/memscan_strings.py            프로세스 메모리 문자열 스캐너(논리이름 발견)
scratchpad/find_nametable.py             구조 기반 NameTable 파인더(앵커 불필요)
scratchpad/skillcam_names.json           복구된 338 names
scratchpad/skillcam_lancemaster_names.txt  같은 338 names(텍스트)
umodel_win32/_skill_list.log             *Skill* 44개 패키지 -list
umodel_win32/_skillcam_flm_list.log      SKILLCAM_LANCEMASTER -list(214 exports)
umodel_win32/_skillcam_flm_dump.log      -dump 시도(클래스 통계, 값은 미노출)
```

물리↔논리: `STANDARD_SKILLCAM_LANCEMASTER` ↔ `NU5OQ5GQ9JN3PAAJ5H9UA5OJXH5NUXG.upk`.
원본 리소스·데이터는 Smilegate 저작물이다. 로그·복호화 UPK·스캔 산출물은 Git에
올리지 않는다.
