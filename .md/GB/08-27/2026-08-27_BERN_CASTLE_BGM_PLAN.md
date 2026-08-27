# 2026-08-27 베른성 BGM 구현 계획

## 실측 결과

`Client/Bin/Resources/Sound1/SoundCatalog.json`에는 사용자가 지정한 음악이 다음 하나의
stable asset으로 등록되어 있다.

```text
sound:bgm.berncastle:bgm_berntown_mscene01_thecapital
Sound/BGM/BernCastle/bgm_berntown_mscene01_thecapital.wav
event: bgm_berntown_mscene01_thecapital
```

현재 `CLevel_Bern`에는 BGM 소비자가 없고 Lobby만 자체 BGM을 시작한 뒤 실제 레벨 이탈 시
정지한다. 베른 로딩 실패가 Lobby 음악까지 끄지 않도록, 베른의 모든 필수 초기화가 성공한 뒤에만
지정 WAV를 시작하고 실제 재생 성공 여부를 레벨 상태로 보존한다.

## 반영 코드

`Client/Private/Level_Bern.cpp`에 `CRuntimeAssetRoot`를 사용한 정확한 asset resolve와 반복 재생을
추가한다. 파일 누락이나 FMOD 재생 실패는 베른 진입을 막지 않고 해당 음향 presentation만
격리한다.

```cpp
constexpr const wchar_t* BERN_CASTLE_BGM_ASSET_ID =
	L"Sound/BGM/BernCastle/bgm_berntown_mscene01_thecapital.wav";

const std::filesystem::path musicPath =
	CRuntimeAssetRoot::Resolve(BERN_CASTLE_BGM_ASSET_ID);
if (!musicPath.empty() && std::filesystem::is_regular_file(musicPath) &&
	SUCCEEDED(CGameInstance::Get().Play_Music(
		musicPath.wstring(), 1.f, true)))
{
	m_bBernBgmStarted = true;
}
```

`Client/Public/Level_Bern.h`에는 실패한 Bern 임시 객체가 Lobby BGM을 정지하지 않도록 실제 재생
성공 여부만 기록하는 다음 멤버를 추가한다.

```cpp
bool_t m_bBernBgmStarted = false;
```

소멸자는 이 값이 참일 때만 단일 music channel을 정지한다.

## 런타임 리소스와 검증

1. Sound1 원본 WAV를 catalog asset ID와 동일한 `Sound/BGM/BernCastle` 런타임 경로로 복사한다.
2. 원본과 런타임 WAV의 SHA-256이 같은지 확인한다.
3. SoundCatalog의 ID와 asset 경로가 정확히 한 건인지 확인한다.
4. Client x64 Debug를 빌드하고 `git diff --check`를 실행한다.
5. 사용자가 Lobby에서 Bern으로 진입해 음악의 반복 재생과 레벨 이탈 시 정지를 직접 확인한다.
