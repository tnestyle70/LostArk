# LostArk 창술사(LanceMaster) 애니메이션 이벤트 조사 결과

작성자: JS · 2026-07-30

창술사 애니메이션마다 붙어 있을 것으로 기대한 이벤트
(공격 타이밍 / 사운드 / 이펙트 스폰 타이밍)를 찾기 위해,
난독화된 물리 UPK를 논리 패키지 이름으로 역매핑하고 애니메이션 패키지의
**NameTable / ImportTable / ExportTable을 전부 복구**해서 확인했다.

전제가 된 추출 작업은
[2026-07-29_LOSTARK_LANCEMASTER_ASSET_EXTRACTION_RESULT.md](2026-07-29_LOSTARK_LANCEMASTER_ASSET_EXTRACTION_RESULT.md)에 있다.

## 1. 먼저 내릴 결론

**이 클라이언트 빌드의 애니메이션 에셋(`UAnimSequence`)에는 UE3 표준
AnimNotify(공격/사운드/이펙트 스폰 타이밍) 데이터가 들어 있지 않다.**

- 429개 `AnimSequence`를 전수 덤프했지만 `Notifies` 배열이 **0개**다.
  `AnimNotify_PlaySound`, `AnimNotify_Script`, `AnimNotify_Effect` 같은 노티파이
  클래스는 export/import 어디에도 없다.
- 완전 복구한 652개 NameTable에도 `notify`, `sound`, `soundcue`, `akevent`,
  `wwise`, `time`, `particle`, `par_`, `camerashake`, `hitbox`, `damage` 토큰이
  **하나도 없다.** 노티파이가 있었다면 이 이름들이 반드시 잡힌다.
- 모든 `AnimSequence`가 `m_pBioAnimSetData = None`이다. 이 커스텀 슬롯이
  타이밍 blob을 담을 자리로 보이지만, 캐릭터 애니 패키지에는 비어 있다.

즉 **"애니메이션에 이벤트가 박혀 있다"는 전제가 이 데이터에서는 성립하지 않는다.**
공격 타이밍·사운드·이펙트 스폰은 애니메이션이 아니라 **외부 스킬/게임 데이터가
소유**하고, 애니메이션은 이름(`SequenceName`)과 부착 본으로만 참여한다.

## 2. 도구와 역매핑

```text
추출기  C:\Users\95jus\Downloads\umodel_win32\umodel_lostark_v7.exe
게임    C:\ProgramData\Smilegate\Games\LOSTARK\EFGame
```

`ReleasePC/Packages/`의 파일명은 전부 난독화되어 있다. `-game=lostark -nameresolve`
두 플래그가 같이 있어야 논리 이름으로 등록되고 `Game: 80004E`로 잡힌다
(자세한 이유는 추출 문서 §2).

창술사 애니메이션의 논리↔물리 매핑(실측):

| 논리 패키지 | 물리 UPK | Names | Exports | Imports |
|---|---|---:|---:|---:|
| `PC_FLM_00` (기본 복장 + 애니 429) | `UB1MW291QZZEZ7DQ61SSPSC.upk` | 652 | 517 | 24 |

`*flm*` 와일드카드로는 141개 패키지가 잡힌다. 애니메이션(`animsequence`/`animset`)을
소유한 것은 `PC_FLM_00`이며, `PC_FLM_01~08` 등 나머지 복장/아바타 패키지는
mesh·material·texture만 가진다. 나머지 대량 패키지는 전부 이펙트
(`particlesystem` 1,010개 등) 라이브러리다.

## 3. 테이블 복구 방법 — 재현 절차

`umodel -list`/`-pkginfo`는 export 목록과 개수만 주고 ImportTable의 모든 row나
NameTable 전체 문자열은 안정적으로 뱉지 않는다. 그래서 발탄 맵 조사와 동일하게
(맵 문서 §6.3~6.5) **umodel이 이미 해독해서 메모리에 올려둔 테이블을 읽었다.**

```powershell
# 1) 뷰어로 대상 패키지를 메모리에 로드
umodel_lostark_v7.exe -view -game=lostark -kr -nameresolve `
  -path="C:\ProgramData\Smilegate\Games\LOSTARK\EFGame" PC_FLM_00

# 2) 그 프로세스 메모리에서 NameTable / ImportTable 스캔 (읽기 전용)
python memscan_names.py <PID> 652 24 "sk_riseup_02,standup_normal_1" out.json
```

- 접근은 `OpenProcess(QUERY_INFORMATION|VM_READ)` + `VirtualQueryEx` +
  `ReadProcessMemory`뿐이다. write/patch/inject 없음.
- NameTable 원리: readable region에서 알려진 name 문자열 anchor를 찾고,
  그 주소를 가리키는 8-byte 포인터 배열을 확장해, 길이가 헤더의 `Names(652)`와
  **정확히 일치**하는 배열을 채택한다. → `652/652` 복구.
- ImportTable 원리: 32-byte stride row(`classPtr / int32 pkgIndex / objPtr / status`)로
  반복되는 세그먼트를 찾고, 헤더 `Imports(24)`만큼 전부 문자열로 resolve되는
  후보를 채택한다. → `24/24` 복구.

스캐너: `scratchpad/memscan_names.py`, 표준 파이썬 없어 **Blender 5.0 번들
파이썬**(`C:\Program Files\Blender Foundation\Blender 5.0\5.0\python\bin\python.exe`)으로
실행했다.

주소는 실행마다 다르므로 결과의 address는 재사용 계약이 아니다.

## 4. 복구한 것에서 확인한 사실

### 4.1 ExportTable 클래스 분포 (`PC_FLM_00`, 517 exports)

```text
429 animsequence          ← 실제 사용 223 + 백업 206
  2 animset               pc_flm_00_ani(223) / pc_flm_00_ani_bk(206)
 35 materialinstanceconstant
 27 texture2d
 12 locassetredirector
  5 skeletalmesh          upper/lower/arm/shoulder/helmet
  5 package
  2 objectredirector
```

노티파이/사운드/이펙트 관련 export 클래스는 없다.

### 4.2 AnimSequence가 실제로 들고 있는 프로퍼티 (429개 전부 동일 셋)

```text
SequenceName, SequenceLength, NumFrames, RateScale,
bNoLoopingInterpolation, bIsAdditive, AdditiveRefName,
RawAnimData, KeyFrameData, Tracks, TrackOffsets,
TranslationCompressionFormat, RotationCompressionFormat,
KeyEncodingFormat(=AKF_ACLDefault), CompressedTrackOffsets,
CompressedTrackTimeOffsets, m_pBioAnimSetData(=None)
```

압축 포즈 키만 있고 **타임라인 이벤트 트랙이 없다.** 표준 UE3라면 여기에
`Notifies[]`(각 원소가 `Time` + `AnimNotify` 오브젝트)가 있어야 한다.

### 4.3 이벤트가 실제로 사는 곳 (아키텍처 추론, 이름 근거)

애니 이름과 본 이름이 이벤트가 **외부에서** 결선됨을 보여준다.

- **이벤트성 애니 이름**: `SequenceName`이 곧 게임 이벤트 슬롯이다.
  `evt1_att_battle_1_01`, `evt1_sk_counterattack_01`, `evt1_sk_downwardhit`,
  `evt1_sk_penetrationlunge_custom_7_02`, `evt_defence_loop_1`.
  스킬 시스템이 이 이름으로 애니를 재생시키는 구조.
- **데미지 판정 슬롯**: `att_battle_compete_1_dmg`, `dmg_idle`, `sk_downwardhit`
  같은 이름이 공격 타이밍을 이름으로 구분한다(타임스탬프가 아니라 별도 클립).
- **이펙트 부착 본**: `b_effectroot`, `b_effectname`, `b_effectworldzero`,
  `b_cameratarget`. 이펙트/카메라가 이 본에 런타임 부착된다.
- **이펙트 라이브러리**: `par_*_flm_*` ParticleSystem 1,010개가 별도 패키지에
  있다(`par_s_flm_dragon_atk_*`, `par_d_flm_combo_01` 등). 스킬 데이터가 어떤
  애니에 어떤 par를 언제 스폰할지 소유하고, 애니 자체는 그것을 모른다.

즉 결선은 `스킬/게임 데이터 테이블 → (애니 SequenceName + par_ 이펙트 + 부착 본
+ 사운드)`이며, 애니 에셋은 종점이 아니라 참조 대상이다.

## 5. 그래서 이벤트를 실제로 얻으려면

애니 패키지를 아무리 풀어도 타이밍은 안 나온다. 다음 두 갈래 중 하나다.

1. **게임 데이터 테이블 경로 (정답 방향).** 공격/사운드/이펙트 타이밍은
   Smilegate의 스킬·연출 데이터에 있다. 발탄 조사에서 `EFTable_Npc` 등
   DB/테이블을 교차 조회했던 것과 같은 계열의 데이터를 열어
   `스킬 → 애니 SequenceName → par_ 이펙트 / 사운드 / 프레임 오프셋` 결선을
   찾아야 한다. 이건 애니 추출과 별개의 작업(데이터 패키지 디코드)이다.
2. **런타임 관찰 경로.** 우리 엔진에서 애니를 재생하고, 필요한 타이밍
   (예: 특정 프레임에 par 스폰, 사운드 재생)을 **우리가 직접 저작**한다.
   원본에 노티파이가 없으므로 원본을 그대로 복원할 수는 없고, 우리 쪽에서
   애니 클립별 이벤트 테이블을 새로 정의하는 편이 현실적이다.

## 6. 산출물

```text
scratchpad/memscan_names.py            NameTable/ImportTable 메모리 스캐너
scratchpad/flm00_tables.json           복구된 652 names + 24 imports + 후보 로그
umodel_win32/_flm_all_list.log         *flm* 141개 패키지 -list (물리↔논리 매핑)
umodel_win32/_flm_dumpall.log          PC_FLM_00 전체 export 프로퍼티 덤프(-dump -all)
```

`umodel -save`로 뽑은 복호화 UPK(`UmodelSaved/.../UB1MW291QZZEZ7DQ61SSPSC.upk`)는
직렬화 payload가 여전히 암호화되어 있어 오프라인 헤더 파싱은 실패했다. **umodel이
메모리에서 해독한 테이블을 읽는 경로가 유일하게 통했다**(발탄 조사와 동일 결론).

## 7. 주의

- 이 스캐너는 조사 도구이지 파이프라인 정본이 아니다. 팀 공용으로 쓰려면 헤더
  카운트 자동 수집, umodel 프로세스 lifecycle, receipt를 갖춘 승격이 필요하다.
- 원본 리소스·데이터는 Smilegate 저작물이다. 스테이징·로그·복호화 UPK는 Git에
  올리지 않는다.
