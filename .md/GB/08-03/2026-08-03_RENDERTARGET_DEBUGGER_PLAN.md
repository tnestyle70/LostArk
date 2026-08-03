# 2026-08-03 G1 Character Select Render Target 진단 툴 계획

참고 이미지: `C:\Users\user\Desktop\툴\03_RenderTarget_DebugView.png`

참고 마스터:
`.md/GB/08-02/2026-08-02_LOSTARK_ADVANCED_RENDERING_WORLDTOOL_INTEGRATION_SCOPE_PLAN.md`

작성 모드: **CODE_EXPLICIT**. 사용자가 계획서를 보며 직접 구현할 수 있도록 파일의 존재 이유와
계약을 먼저 설명한 다음, 사용자가 옮겨 반영할 전체 함수와 정확한 추가 코드를 함께 기록한다.
Codex는 C++를 선제 반영하지 않고 사용자 구현 뒤 검토와 빌드 검증을 맡는다.

---

## 1. 현재 체크포인트와 G1 완료 조건

현재 실측은 다음과 같다.

- `CRenderer::Initialize()`가 13개 Render Target과 7개 MRT를 만든다.
- `CTarget_Manager`가 `wstring` tag를 key로 Render Target을 소유하지만 Client가 읽을 수 있는
  snapshot API는 없다.
- `CRenderTarget`은 RTV만 공개하고 SRV와 실제 texture descriptor는 private이다.
- `CMainApp::Render()`는 `CGameInstance::Render()`로 모든 MRT를 종료한 뒤 ImGui를 그리므로,
  이 위치에서 read-only SRV를 표시하는 것은 D3D11 read/write hazard를 만들지 않는다.
- F1 Diagnostics와 Profiler의 visible 상태, UI 호출 순서, 종료 수명은 이미 `CMainApp`이 소유한다.
- Character Select는 실제 `CCharacter -> CModel -> CMaterial`을 쓰지만 현재 Effect/Animation/Map/UI
  Tool 버튼이 모두 Development에서만 활성화된다.

G1의 완료 조건은 다음 여섯 가지다.

1. Character Select에서 F1을 열면 Effect Tool과 Animation Tool은 활성화되고 Map Tool과 HUD
   Layout Tool은 비활성화된다.
2. Diagnostics의 `Render Targets`를 켜면 13개 target이 tag, width, height, format, thumbnail로
   보인다.
3. thumbnail을 누르면 선택 tag를 기준으로 확대하고 `Back to grid`로 복귀한다.
4. Engine의 target map, `CRenderTarget` owner, RTV는 Client에 공개하지 않는다.
5. Release에는 snapshot API와 진단 상태가 포함되지 않고 기존 제품 렌더 경로가 유지된다.
6. Engine Debug/Release -> UpdateLib Debug/Release -> Client Debug/Release와 실제 Character Select
   수동 검증을 통과한다.

G1은 channel 분리, depth linearize, HDR false color, pixel probe, PNG capture, GPU pass timing을
구현하지 않는다. 이 기능은 snapshot 구조를 확장하는 G2가 소유한다.

---

## 2. 전체 수직 흐름 한 줄

```text
CRenderer target 생성
-> CTarget_Manager owner map
-> DEBUG_RENDER_TARGET_SNAPSHOT 복사본
-> CGameInstance read-only relay
-> CMainApp F1 Diagnostics
-> ImGui grid/fullscreen
```

Character Select tool admission은 별도 데이터 흐름이다.

```text
현재 LEVEL ID + DEBUG_TOOL
-> CMainApp::IsDebugToolAvailable
-> Effect/Animation만 Character Select에서 허용
-> 기존 EnsureDebugTool lazy 생성
```

---

## 3. 전체 변경 지도

### 유지할 파일

- `Engine/Private/RenderTarget.cpp`: texture/RTV/SRV 생성 책임은 이미 맞으므로 수정하지 않는다.
- `Engine/Private/Renderer.cpp`: 13개 target 등록과 렌더 순서는 수정하지 않는다.
- `Engine/Default/Engine.vcxproj`, `.filters`: 신규 Engine 파일이 없으므로 수정하지 않는다.
- `Client/Default/Client.vcxproj`, `.filters`: 신규 Client 파일이 없으므로 수정하지 않는다.
- `Client/Public|Private/Level_CharacterSelect.*`: Tool 구현이나 RT 진단 구현을 Level 안에 넣지 않는다.
- `Client/Public|Private/Effect_Tool.*`: G1은 기존 Tool을 Character Select에서 열 수 있게만 한다.

### 수정할 파일

- `Engine/Public/Engine_Struct.h`: `_DEBUG` snapshot value type을 추가한다.
- `Engine/Public/RenderTarget.h`: `_DEBUG`에서 SRV와 실제 texture descriptor를 읽는 getter를 추가한다.
- `Engine/Public/Target_Manager.h`: snapshot public 함수 계약을 추가한다.
- `Engine/Private/Target_Manager.cpp`: private map을 value snapshot으로 복사한다.
- `Engine/Public/GameInstance.h`: Client가 사용할 Engine facade relay를 추가한다.
- `Engine/Private/GameInstance.cpp`: Target Manager snapshot을 그대로 전달한다.
- `Client/Public/MainApp.h`: Tool admission 함수, RT 창 함수, visible/registry/selected tag 상태를 추가한다.
- `Client/Private/MainApp.cpp`: Character Select admission, snapshot 초기화, grid/fullscreen ImGui를 연결한다.

### 새로 추가할 파일

- 없음.

### 삭제할 파일

- 없음.

---

## 4. 새 파일 필요성 판정

`RenderTargetInspector.h/.cpp`를 이번 G1에 새로 만들지 않는다.

```text
이 책임을 이미 소유한 파일이 있는가
-> 있다. CMainApp이 F1 Diagnostics, Profiler 표시와 종료를 이미 소유한다.

기존 class의 private 함수와 상태로 닫을 수 있는가
-> 가능하다. visible bool, snapshot vector, selected tag 세 상태면 G1이 끝난다.

새 파일만의 독립된 수명과 owner가 있는가
-> 아직 없다. CMainApp과 생성/파괴 시점이 완전히 같다.

새 파일을 만들지 않으면 기존 파일이 두 제품 책임을 갖는가
-> 아니다. 두 기능 모두 Debug Diagnostics 조율 책임이다.

새 파일의 실제 호출자가 현재 존재하는가
-> CMainApp 하나뿐이다.
```

G2에서 visualization shader, staging readback, capture job, frame revision처럼 독립 상태와 수명이
생기면 `RenderDebugTool` 분리를 다시 판정한다. 미래 기능만을 이유로 G1에 placeholder class를
만들지 않는다.

---

## 5. `Engine/Public/Engine_Struct.h`

### 수정 위치

```text
물리 경로: Engine/Public/Engine_Struct.h
수정 위치: namespace Engine 안, 기존 공용 struct 영역
직접 연결: Target_Manager.h, GameInstance.h, MainApp.h
삭제할 기존 책임: 없음
수정 후 남는 책임: Engine과 Client가 공유하는 Debug snapshot value type
```

### 파일이 존재하는 이유와 이번 한 문장 역할

이 파일은 Engine public value type의 정본이다. 이번 변경은 Render Target owner나 manager class를
노출하지 않고, 특정 시점의 읽기 전용 표시 값만 DLL 경계로 넘기기 위해 존재한다.

### 추가 구조체와 변수 한 줄 설명

- `DEBUG_RENDER_TARGET_SNAPSHOT`: live registry 참조가 아니라 snapshot 호출 시점의 표시 값 묶음이다.
- `strTargetTag`: 확대 선택과 표시가 사용하는 stable target ID다.
- `pSRV`: Engine이 소유하고 ImGui가 빌려 읽는 SRV raw pointer이며 AddRef나 소유권 이전은 없다.
- `iWidth`: 실제 texture width이며 thumbnail 종횡비와 라벨에 사용한다.
- `iHeight`: 실제 texture height이며 thumbnail 종횡비와 라벨에 사용한다.
- `eFormat`: 실제 texture `DXGI_FORMAT`이며 포맷 라벨과 후속 G2 visualization 판정의 입력이다.

### 실제 추가 코드

`namespace Engine` 안의 공용 struct 영역에 추가한다.

```cpp
#ifdef _DEBUG
	typedef struct tagDebugRenderTargetSnapshot
	{
		wstring_t strTargetTag;
		ID3D11ShaderResourceView* pSRV = { nullptr };
		uint32_t iWidth = {};
		uint32_t iHeight = {};
		DXGI_FORMAT eFormat = { DXGI_FORMAT_UNKNOWN };
	} DEBUG_RENDER_TARGET_SNAPSHOT;
#endif
```

owner 불변식은 `CTarget_Manager > CRenderTarget > SRV`다. `CMainApp`은 Engine이 살아 있는 동안만
snapshot pointer를 표시하고 `Free()`에서 registry를 먼저 비운다.

---

## 6. `Engine/Public/RenderTarget.h`

### 수정 위치

```text
물리 경로: Engine/Public/RenderTarget.h
수정 class: Engine::CRenderTarget
수정 위치: 기존 #ifdef _DEBUG public 블록
직접 연결: Target_Manager.cpp
삭제할 기존 책임: 없음
수정 후 남는 책임: 자기 texture의 Debug 읽기 경계
```

### 필요한 헤더

추가 include는 없다. 기존 `Engine_Defines.h`가 D3D11 texture, SRV, descriptor 타입을 제공한다.

### 함수 계약과 한 줄 설명

`Get_SRV() const`

- 존재 이유: ImGui thumbnail이 shader binding을 거치지 않고 동일 SRV를 읽기 위해 필요하다.
- 호출자: `CTarget_Manager::Snapshot_RenderTargets()`.
- 입력: 없음.
- 출력: Engine 소유 `ID3D11ShaderResourceView*` borrowed pointer.
- 변경 멤버: 없음.
- 실패: SRV가 없으면 `nullptr`; owner 수명은 연장하지 않는다.

`Get_TextureDesc() const`

- 존재 이유: 별도 width/height/format 중복 멤버를 만들지 않고 실제 texture를 정본으로 읽는다.
- 호출자: `CTarget_Manager::Snapshot_RenderTargets()`.
- 입력: 없음.
- 출력: `D3D11_TEXTURE2D_DESC` 값 복사본.
- 변경 멤버: 없음.
- 실패: texture가 없으면 zero descriptor를 반환한다.

### 실제 추가 코드

```cpp
#ifdef _DEBUG
public:
	ID3D11ShaderResourceView* Get_SRV() const
	{
		return m_pSRV.Get();
	}

	D3D11_TEXTURE2D_DESC Get_TextureDesc() const
	{
		D3D11_TEXTURE2D_DESC textureDesc{};
		if (nullptr != m_pTexture2D)
			m_pTexture2D->GetDesc(&textureDesc);
		return textureDesc;
	}

	HRESULT Ready_DebugDesc(
		f32_t fX,
		f32_t fY,
		f32_t fSizeX,
		f32_t fSizeY);
	HRESULT Render(
		shared_ptr<class CShader> pShader,
		shared_ptr<class CVIBuffer_Rect> pVIBuffer);
#endif
```

기존 `Ready_DebugDesc`, `Render` 선언은 같은 block에 그대로 남긴다. `RenderTarget.cpp`에는 새
구현이 없다.

---

## 7. `Engine/Public/Target_Manager.h`와 `Engine/Private/Target_Manager.cpp`

### 수정 위치

```text
물리 경로 H: Engine/Public/Target_Manager.h
물리 경로 CPP: Engine/Private/Target_Manager.cpp
수정 class: Engine::CTarget_Manager
수정 위치: 기존 #ifdef _DEBUG public/API 구현 블록
직접 연결: RenderTarget.h, GameInstance.cpp
삭제할 기존 책임: 없음
수정 후 남는 책임: private owner map을 read-only value snapshot으로 변환
```

### 함수 계약과 한 줄 설명

`Snapshot_RenderTargets() const`

- 존재 이유: Client가 private map이나 `CRenderTarget` owner를 받지 않고 표시 값만 읽게 한다.
- 호출자: `CGameInstance::Snapshot_RenderTargets()`.
- 호출 시점: Engine 초기화 뒤 Debug main thread. G1에서는 ReadyDebugTools와 명시적 Refresh뿐이다.
- 입력: 없음.
- 출력: tag 오름차순 map 순서의 `vector<DEBUG_RENDER_TARGET_SNAPSHOT>`.
- 읽는 멤버: `m_RenderTargets`.
- 변경 멤버: 없음.
- 실패: null target은 건너뛰고 나머지는 보존한다. 빈 vector는 등록 target 없음이다.

### CPP 내부 흐름

```text
vector 생성
-> owner map size만큼 reserve
-> tag/target pair 순회
-> null target 건너뜀
-> 실제 texture desc와 SRV 읽기
-> snapshot 값 push
-> vector 반환
```

### H 실제 추가 코드

기존 `_DEBUG public` block에 첫 함수로 추가한다.

```cpp
	vector<DEBUG_RENDER_TARGET_SNAPSHOT>
		Snapshot_RenderTargets() const;
```

### CPP 실제 전체 함수

기존 `#ifdef _DEBUG` block의 `Ready_DebugDesc()` 앞에 추가한다.

```cpp
vector<DEBUG_RENDER_TARGET_SNAPSHOT>
CTarget_Manager::Snapshot_RenderTargets() const
{
	vector<DEBUG_RENDER_TARGET_SNAPSHOT> snapshots;
	snapshots.reserve(m_RenderTargets.size());

	for (const auto& renderTargetPair : m_RenderTargets)
	{
		const wstring_t& strTargetTag = renderTargetPair.first;
		const shared_ptr<CRenderTarget>& pRenderTarget =
			renderTargetPair.second;
		if (nullptr == pRenderTarget)
			continue;

		const D3D11_TEXTURE2D_DESC textureDesc =
			pRenderTarget->Get_TextureDesc();

		snapshots.emplace_back();
		DEBUG_RENDER_TARGET_SNAPSHOT& snapshot = snapshots.back();
		snapshot.strTargetTag = strTargetTag;
		snapshot.pSRV = pRenderTarget->Get_SRV();
		snapshot.iWidth = textureDesc.Width;
		snapshot.iHeight = textureDesc.Height;
		snapshot.eFormat = textureDesc.Format;
	}

	return snapshots;
}
```

`snapshot`은 owner가 아니라 값 복사다. stable selection은 vector index나 pointer가 아니라
`strTargetTag`를 사용한다.

---

## 8. `Engine/Public/GameInstance.h`와 `Engine/Private/GameInstance.cpp`

### 수정 위치

```text
물리 경로 H: Engine/Public/GameInstance.h
물리 경로 CPP: Engine/Private/GameInstance.cpp
수정 class: Engine::CGameInstance
수정 위치: For.Target_Manager의 기존 #ifdef _DEBUG relay block
직접 연결: Target_Manager.h, MainApp.cpp
삭제할 기존 책임: 없음
수정 후 남는 책임: Engine singleton facade의 Debug snapshot relay
```

### 함수 계약과 한 줄 설명

`Snapshot_RenderTargets() const`

- 존재 이유: Client가 `CTarget_Manager` 구현 header를 include하지 않고 Engine facade만 사용한다.
- 호출자: `CMainApp::ReadyDebugTools()`, `CMainApp::RenderRenderTargetInspector()`의 Refresh.
- 출력: Target Manager가 만든 snapshot vector 값.
- 실패: Engine 초기화 전 manager가 없으면 빈 vector를 반환한다.
- 외부 부작용: 없음.

### H 실제 추가 코드

```cpp
	vector<DEBUG_RENDER_TARGET_SNAPSHOT>
		Snapshot_RenderTargets() const;
```

### CPP 실제 전체 함수

```cpp
vector<DEBUG_RENDER_TARGET_SNAPSHOT>
CGameInstance::Snapshot_RenderTargets() const
{
	if (nullptr == m_pTarget_Manager)
		return {};
	return m_pTarget_Manager->Snapshot_RenderTargets();
}
```

이 함수는 기존 `Ready_RT_DebugDesc()` 앞에 둔다.

---

## 9. `Client/Public/MainApp.h`

### 수정 위치

```text
물리 경로: Client/Public/MainApp.h
수정 class: Client::CMainApp
수정 위치: #ifdef _DEBUG private 함수와 멤버
직접 연결: MainApp.cpp, GameInstance.h
삭제할 기존 책임: Development 하나로 묶인 일괄 Tool admission
수정 후 남는 책임: tool별 context admission과 F1 diagnostics 상태 수명
```

### public 함수

추가 없음. Render Target 진단은 제품 public 계약이 아니다.

### private 함수와 한 줄 설명

- `IsDebugToolAvailable(DEBUG_TOOL, uint32_t) const`: 현재 Level에서 tool별 사용 가능 여부만 판정한다.
- `RenderRenderTargetInspector()`: snapshot registry를 grid 또는 선택 target 확대 화면으로 그린다.

### 멤버 변수와 한 줄 설명

- `m_bRenderTargetInspectorVisible`: F1 checkbox와 ImGui close 버튼이 공유하는 창 visible 상태다.
- `m_RenderTargetSnapshotRegistry`: 매 프레임 allocation 없이 표시하는 Engine snapshot 복사본이다.
- `m_strSelectedRenderTargetTag`: 확대 target을 stable tag로 기억하고 vector index/SRV pointer를 ID로 쓰지 않는다.

### 실제 헤더 추가 코드

기존 `_DEBUG` private 함수 block에 추가한다.

```cpp
	bool_t IsDebugToolAvailable(
		DEBUG_TOOL eTool,
		uint32_t iCurrentLevelId) const;
	void RenderRenderTargetInspector();
```

기존 `_DEBUG` 멤버 block에 추가한다.

```cpp
	bool_t m_bRenderTargetInspectorVisible = false;
	vector<DEBUG_RENDER_TARGET_SNAPSHOT>
		m_RenderTargetSnapshotRegistry;
	wstring_t m_strSelectedRenderTargetTag;
```

기존 `m_strToolStatus` 기본 문구는 다음으로 교체한다.

```cpp
	string m_strToolStatus =
		"Effect/Animation: Character Select or Development. "
		"Map/UI: Development only.";
```

---

## 10. `Client/Private/MainApp.cpp`: helper 함수

### 필요한 헤더

추가 include는 없다. `imgui.h`, `MainApp.h`, `GameInstance.h`, `algorithm` 계열은 현재 include
경로에서 이미 제공된다. helper는 anonymous namespace의 기존 `_DEBUG` 구간에 둔다.

### helper 함수와 한 줄 설명

- `GetRenderTargetFormatName`: 현재 실제 사용 포맷을 사람이 읽는 label로 바꾼다.
- `MakeRenderTargetTextureRef`: borrowed D3D11 SRV를 현재 ImGui 1.92 `ImTextureRef`로 감싼다.
- `FitRenderTargetImageSize`: 원본 종횡비를 유지하며 주어진 최대 영역 안에 이미지를 맞춘다.

### 실제 helper 코드

```cpp
#ifdef _DEBUG
	const char_t* GetRenderTargetFormatName(const DXGI_FORMAT eFormat)
	{
		switch (eFormat)
		{
		case DXGI_FORMAT_R8G8B8A8_UNORM:
			return "R8G8B8A8_UNORM";
		case DXGI_FORMAT_R16G16B16A16_UNORM:
			return "R16G16B16A16_UNORM";
		case DXGI_FORMAT_R16G16B16A16_FLOAT:
			return "R16G16B16A16_FLOAT";
		case DXGI_FORMAT_R32G32B32A32_FLOAT:
			return "R32G32B32A32_FLOAT";
		case DXGI_FORMAT_R11G11B10_FLOAT:
			return "R11G11B10_FLOAT";
		default:
			return "DXGI_FORMAT_OTHER";
		}
	}

	ImTextureRef MakeRenderTargetTextureRef(
		ID3D11ShaderResourceView* pSRV)
	{
		return ImTextureRef(pSRV);
	}

	ImVec2 FitRenderTargetImageSize(
		const uint32_t iWidth,
		const uint32_t iHeight,
		const f32_t fMaximumWidth,
		const f32_t fMaximumHeight)
	{
		if (0u == iWidth || 0u == iHeight)
			return ImVec2(1.f, 1.f);

		const f32_t fSafeMaximumWidth =
			(std::max)(1.f, fMaximumWidth);
		const f32_t fSafeMaximumHeight =
			(std::max)(1.f, fMaximumHeight);
		const f32_t fScale = (std::min)(
			fSafeMaximumWidth / static_cast<f32_t>(iWidth),
			fSafeMaximumHeight / static_cast<f32_t>(iHeight));

		return ImVec2(
			(std::max)(1.f, static_cast<f32_t>(iWidth) * fScale),
			(std::max)(1.f, static_cast<f32_t>(iHeight) * fScale));
	}
#endif
```

---

## 11. `CMainApp::ReadyDebugTools()` 전체 코드

### 함수 한 줄 설명

Profiler와 RT snapshot의 초기 Debug 상태를 Engine 초기화 직후 한 번 고정한다.

### 실제 전체 함수

```cpp
HRESULT CMainApp::ReadyDebugTools()
{
	if (Engine::CProfiler* pProfiler =
		CGameInstance::Get().Get_Profiler())
	{
		pProfiler->Reset_History();
		pProfiler->Set_Enabled(false);
	}

	m_bProfilerVisible = false;
	m_bRenderTargetInspectorVisible = false;
	m_strSelectedRenderTargetTag.clear();
	m_RenderTargetSnapshotRegistry =
		CGameInstance::Get().Snapshot_RenderTargets();

	return S_OK;
}
```

Renderer target 등록은 `Initialize_Engine()` 안에서 이미 끝났으므로 이 시점의 registry는 현재
13개를 가진다.

---

## 12. `CMainApp::IsDebugToolAvailable()` 전체 코드

### 함수 한 줄 설명

Debug tool을 제품 Level 전역에 풀지 않고 Character Select와 Development의 허용 범위를 tool별로
판정한다.

### 실제 전체 함수

```cpp
bool_t CMainApp::IsDebugToolAvailable(
	const DEBUG_TOOL eTool,
	const uint32_t iCurrentLevelId) const
{
	const bool_t isDevelopment =
		ETOUI(LEVEL::DEVELOPMENT) == iCurrentLevelId;
	const bool_t isCharacterSelect =
		ETOUI(LEVEL::CHARACTER_SELECT) == iCurrentLevelId;

	switch (eTool)
	{
	case DEBUG_TOOL::ANIMATION:
	case DEBUG_TOOL::EFFECT:
		return isDevelopment || isCharacterSelect;
	case DEBUG_TOOL::MAP:
	case DEBUG_TOOL::UI:
		return isDevelopment;
	default:
		return false;
	}
}
```

이 함수는 socket을 열거나 Character를 찾지 않는다. 실제 target binding은 G4가 기존
`CAnimationTargetService`를 통해 연결한다.

---

## 13. `CMainApp::RenderDeveloperTools()` 수정 코드

기존 `isTestLevel` 변수와 이를 capture하는 `toolButton` lambda를 다음 코드로 교체한다.

```cpp
	const uint32_t currentLevelId =
		CGameInstance::Get().Get_CurrentLevelID();
	ImGui::Text("Current level id: %u", currentLevelId);
	ImGui::TextDisabled(
		"Effect/Animation: Character Select or Development. "
		"Map/UI: Development only.");
	ImGui::SeparatorText("Tools");

	const auto toolButton = [this, currentLevelId](
		const char_t* pLabel,
		const DEBUG_TOOL eTool)
	{
		const bool_t isAvailable =
			IsDebugToolAvailable(eTool, currentLevelId);
		ImGui::BeginDisabled(!isAvailable);
		if (ImGui::Button(pLabel))
		{
			m_strToolStatus = SUCCEEDED(EnsureDebugTool(eTool)) ?
				"Tool opened." : "Tool initialization failed.";
		}
		ImGui::EndDisabled();
	};
```

기존 Profiler checkbox 처리 뒤, F1 안내 문구 앞에 다음을 추가한다.

```cpp
	ImGui::SameLine();
	ImGui::Checkbox(
		"Render Targets",
		&m_bRenderTargetInspectorVisible);
```

RT Diagnostics는 read-only이므로 Level admission으로 막지 않는다. Character Select, Development,
Bern, Valtan 모두 동일 snapshot을 볼 수 있다.

---

## 14. `CMainApp::RenderRenderTargetInspector()` 전체 코드

### 함수 한 줄 설명

Engine snapshot 복사본을 stable tag 기반 grid 또는 확대 화면으로 표시한다.

### 내부 흐름

```text
창 크기/Begin
-> Refresh 요청이면 snapshot을 다시 복사
-> selected tag가 있으면 registry에서 검색
-> 찾으면 metadata + aspect-fit Image
-> 없으면 selected tag 정리
-> grid table에서 모든 snapshot 표시
-> ImageButton 클릭 시 tag 저장
-> End
```

### 실제 전체 함수

```cpp
void CMainApp::RenderRenderTargetInspector()
{
	ImGui::SetNextWindowSize(
		ImVec2(1000.f, 700.f),
		ImGuiCond_FirstUseEver);

	if (!ImGui::Begin(
		"Render Targets",
		&m_bRenderTargetInspectorVisible))
	{
		ImGui::End();
		return;
	}

	if (ImGui::Button("Refresh Snapshot"))
	{
		m_RenderTargetSnapshotRegistry =
			CGameInstance::Get().Snapshot_RenderTargets();
	}

	const DEBUG_RENDER_TARGET_SNAPSHOT* pSelectedTarget = nullptr;
	if (!m_strSelectedRenderTargetTag.empty())
	{
		for (const DEBUG_RENDER_TARGET_SNAPSHOT& snapshot :
			m_RenderTargetSnapshotRegistry)
		{
			if (snapshot.strTargetTag ==
				m_strSelectedRenderTargetTag)
			{
				pSelectedTarget = &snapshot;
				break;
			}
		}

		if (nullptr == pSelectedTarget)
			m_strSelectedRenderTargetTag.clear();
	}

	if (nullptr != pSelectedTarget)
	{
		if (ImGui::Button("< Back to grid"))
		{
			m_strSelectedRenderTargetTag.clear();
			ImGui::End();
			return;
		}

		ImGui::SameLine();
		ImGui::Text("%ls", pSelectedTarget->strTargetTag.c_str());
		ImGui::TextDisabled(
			"%u x %u | %s (%u)",
			pSelectedTarget->iWidth,
			pSelectedTarget->iHeight,
			GetRenderTargetFormatName(pSelectedTarget->eFormat),
			static_cast<uint32_t>(pSelectedTarget->eFormat));

		if (nullptr == pSelectedTarget->pSRV ||
			0u == pSelectedTarget->iWidth ||
			0u == pSelectedTarget->iHeight)
		{
			ImGui::TextDisabled(
				"The selected render target has no valid SRV.");
			ImGui::End();
			return;
		}

		const ImVec2 availableSize =
			ImGui::GetContentRegionAvail();
		const ImVec2 imageSize = FitRenderTargetImageSize(
			pSelectedTarget->iWidth,
			pSelectedTarget->iHeight,
			availableSize.x,
			availableSize.y);

		ImGui::Image(
			MakeRenderTargetTextureRef(pSelectedTarget->pSRV),
			imageSize);
		ImGui::End();
		return;
	}

	if (m_RenderTargetSnapshotRegistry.empty())
	{
		ImGui::TextDisabled("No render targets were registered.");
		ImGui::End();
		return;
	}

	constexpr f32_t fThumbnailMaximumWidth = 220.f;
	constexpr f32_t fThumbnailMaximumHeight = 140.f;
	const f32_t fCellWidth = fThumbnailMaximumWidth +
		ImGui::GetStyle().ItemSpacing.x;
	const f32_t fAvailableWidth =
		(std::max)(1.f, ImGui::GetContentRegionAvail().x);
	const int32_t iColumnCount = (std::max)(
		1,
		static_cast<int32_t>(fAvailableWidth / fCellWidth));

	if (ImGui::BeginTable(
		"##RenderTargetGrid",
		iColumnCount,
		ImGuiTableFlags_SizingFixedFit))
	{
		for (const DEBUG_RENDER_TARGET_SNAPSHOT& snapshot :
			m_RenderTargetSnapshotRegistry)
		{
			ImGui::TableNextColumn();
			ImGui::PushID(&snapshot);

			ImGui::Text("%ls", snapshot.strTargetTag.c_str());
			ImGui::TextDisabled(
				"%u x %u | %s (%u)",
				snapshot.iWidth,
				snapshot.iHeight,
				GetRenderTargetFormatName(snapshot.eFormat),
				static_cast<uint32_t>(snapshot.eFormat));

			const ImVec2 imageSize = FitRenderTargetImageSize(
				snapshot.iWidth,
				snapshot.iHeight,
				fThumbnailMaximumWidth,
				fThumbnailMaximumHeight);

			if (nullptr == snapshot.pSRV)
			{
				ImGui::Dummy(imageSize);
				ImGui::TextDisabled("SRV unavailable");
			}
			else if (ImGui::ImageButton(
				"##RenderTargetThumbnail",
				MakeRenderTargetTextureRef(snapshot.pSRV),
				imageSize))
			{
				m_strSelectedRenderTargetTag =
					snapshot.strTargetTag;
			}

			ImGui::PopID();
		}

		ImGui::EndTable();
	}

	ImGui::End();
}
```

현재 target 이름은 ASCII지만 저장과 비교는 원래 `wstring_t` tag를 그대로 사용한다. 표시만 `%ls`
로 한다.

---

## 15. `CMainApp::Render()`와 `Free()` 연결

### Render 호출 코드

`CGameInstance::Render()`가 성공한 뒤의 기존 `m_bDeveloperToolsVisible` block에서 active tool을
그린 다음, Profiler보다 앞에 다음을 추가한다.

```cpp
			if (m_bRenderTargetInspectorVisible)
				RenderRenderTargetInspector();
```

이 위치에서는 모든 MRT write가 끝나 backbuffer가 복원되어 있다. Begin_MRT와 End_MRT 사이에는
Inspector를 호출하지 않는다.

### Free 정리 코드

`#ifdef _DEBUG`의 Tool reset 앞에 다음을 추가한다.

```cpp
	m_bRenderTargetInspectorVisible = false;
	m_strSelectedRenderTargetTag.clear();
	m_RenderTargetSnapshotRegistry.clear();
```

borrowed SRV pointer를 가진 vector를 `CGameInstance::Release_Engine()`보다 먼저 비워 수명 순서를
명시한다.

---

## 16. 프로젝트 설정과 등록

새 H/CPP가 없으므로 `.vcxproj`, `.vcxproj.filters`, include directory, project reference, link
library 변경은 모두 **추가 없음**이다.

현재 실제 filter는 그대로 유지한다.

- `Engine_Struct.h`: `99.Defines`
- `RenderTarget.h`: `02.Utility\08.RenderTargets`
- `Target_Manager.h/.cpp`: `02.Utility\08.RenderTargets\Manager`
- `GameInstance.h/.cpp`: `00.GameInstance`
- `MainApp.h/.cpp`: `00.MainApp`

Engine public header가 바뀌므로 구성별 `UpdateLib.bat`은 필수다.

---

## 17. 실패·rollback과 금지 경계

- snapshot이 비면 `No render targets were registered`를 표시하고 제품 렌더 상태는 바꾸지 않는다.
- 한 target의 SRV가 null이면 그 cell만 unavailable로 표시하고 나머지 target을 유지한다.
- Refresh 뒤 선택 tag가 없어졌으면 확대 선택만 해제하고 registry 자체는 유지한다.
- Client는 `CTarget_Manager`, `CRenderTarget`, RTV, owner map을 include하거나 저장하지 않는다.
- snapshot pointer를 stable ID로 저장하지 않는다. 선택 ID는 `strTargetTag`다.
- G1은 readback, file write, report writer, scenario parser, 자동 UI 입력, 자동 종료를 추가하지 않는다.
- Character Select에서 Tool을 열어도 socket 연결, packet 작성, gameplay command는 만들지 않는다.
- Map/UI Tool은 Character Select에서 활성화하지 않는다.
- 다른 세션이 수정 중인 `MainApp`을 되돌리거나 전체 교체하지 않고 최신 체크포인트 위에 좁은
  hunk만 병합한다.

---

## 18. 사용자 구현과 Codex 검토 순서

1. 다른 세션의 Character Select/MainApp 변경이 빌드 가능한 체크포인트인지 확인한다.
2. 사용자가 clean Engine 파일에 snapshot value/getter/manager/facade를 직접 반영한다.
3. Codex가 실제 diff와 함수·변수·owner 불변식을 검토한 뒤 Engine Debug/Release와 UpdateLib
   Debug/Release를 검증한다.
4. 사용자가 최신 MainApp을 다시 읽고 tool admission과 diagnostics 상태/함수만 좁게 반영한다.
5. Codex가 Client diff를 검토하고 Debug/Release를 빌드한다.
6. 사용자와 Codex가 실제 Lobby -> Character Select에서 F1 Tool admission과 13-target
   grid/fullscreen을 검증한다.
7. Codex가 ProjectAudit와 `git diff --check`를 실행하고 G1 RESULT에 실행한 증거만 기록한다.

---

## 19. Breakpoint와 관찰값

```text
CRenderer::Initialize 직후
-> CTarget_Manager::m_RenderTargets.size() == 13

CMainApp::ReadyDebugTools
-> Snapshot_RenderTargets 반환 size == 13
-> 모든 strTargetTag non-empty
-> 모든 pSRV non-null

Character Select F1
-> IsDebugToolAvailable(EFFECT, CHARACTER_SELECT) == true
-> IsDebugToolAvailable(ANIMATION, CHARACTER_SELECT) == true
-> IsDebugToolAvailable(MAP, CHARACTER_SELECT) == false
-> IsDebugToolAvailable(UI, CHARACTER_SELECT) == false

RenderRenderTargetInspector grid
-> BloomExtract/Ping/Result는 half resolution
-> SceneHDR/Distortion은 full resolution
-> 선택 후 m_strSelectedRenderTargetTag가 tag 문자열과 일치
```

`Target_LightDepth`가 clear color만 보일 수 있다. 현재 `Add_Shadow_Light` producer 호출이 없다는
실측과 일치하며, G1은 값을 숨기거나 정상 shadow처럼 꾸미지 않는다. G2 metadata에서 producer
없음을 명시한다.

---

## 20. 빌드·실행 검증과 다음 단계

정본 순서:

```text
Engine x64 Debug
-> UpdateLib.bat Debug
-> Client x64 Debug
-> Engine x64 Release
-> UpdateLib.bat Release
-> Client x64 Release
```

수동 실행:

```text
Client 작업 디렉터리: Client/Default
-> Lobby
-> Character Select
-> F1
-> Effect/Animation enabled, Map/UI disabled
-> Render Targets ON
-> 13개 grid
-> Diffuse/Normal/Depth/SceneHDR/Distortion/BloomResult 확대
-> Back to grid
-> Character class 교체 뒤 같은 진단 창 유지
-> Lobby 복귀와 종료
```

종료 검사:

```text
Tools/ProjectAudit/Invoke-ProjectAudit.ps1
git diff --check
수정한 C++ 인코딩 유지 확인
```

G1 다음은 카메라 툴이 아니다. G2에서 snapshot metadata, channel/normal/depth/HDR/NaN/pixel probe,
one-shot capture와 pass GPU ms를 추가한 뒤 G3 Effect 문서/material 계약과 G4 Character attachment로
이어 간다.
