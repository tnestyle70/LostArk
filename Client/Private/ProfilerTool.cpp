#include "imgui.h"
#include "ProfilerTool.h"
#include "GameInstance.h"
#include "Engine_RenderTypes.h"
#include "ClientWindowDisplay.h"
#include "UserSettingsDocument.h"
#include "RenderingReferenceGuide.h"
#include <cstring>
#include <dxgi.h>

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <set>
#include <tuple>

namespace
{
    // SCOPE_CATALOG: kept in sync with actual CProfilerScope/GpuScope call sites.
    constexpr const char* CPU_SCOPE_CATALOG[] = {
        "Render.SSGI",
        "Render.SSGI.GatherHalf",
        "Render.SSGI.ResolveHalf",
        "Render.SSR",
        "Render.ScreenSpaceLighting",
        "Render.ScreenSpaceLighting.Copy",
        "Profiler.Memory.Sample",
        "Animation.Blend",
        "Animation.Bones.Combine",
        "Animation.Channels.Sample",
        "Animation.Channels.Update",
        "Animation.History.DebugVerify",
        "Animation.History.Sample",
        "Animation.Play",
        "Animation.SkinPalette.Bind",
        "Animation.SkinPalette.Build",
        "Animation.Transition.Build",
        "Catalog.PlayerSkills.Initialize",
        "Character.EffectCues.Admit",
        "Character.FaceMorph.Admit",
        "Character.FaceSliders.Admit",
        "Character.Initialize",
        "Character.SkillBindings.Admit",
        "CharacterAssets.Authoring.Capture",
        "CharacterAssets.Authoring.Prepare",
        "CharacterAssets.Commit",
        "CharacterAssets.FaceMorph.Prepare",
        "CharacterAssets.LevelTransition.Drain",
        "CharacterAssets.Prepare.Worker",
        "CharacterAssets.Retire.Worker",
        "Client.Render",
        "Client.Update",
        "Effect.Decal.Render",
        "Effect.FollowAnchors.Update",
        "Effect.LevelPresentation.VisibleUpdate",
        "Effect.Material.Bind",
        "Effect.Mesh.BindAndDraw",
        "Effect.Mesh.DrawSubmission",
        "Effect.Mesh.InstanceBuild",
        "Effect.Mesh.InstanceUpload",
        "Effect.Mesh.Render",
        "Effect.Occurrence.LateUpdate",
        "Effect.Occurrence.Render",
        "Effect.Occurrence.Update",
        "Effect.Particle.Render",
        "Effect.Particle.Spawn",
        "Effect.Particle.Update",
        "Effect.Playback.FixedStep",
        "Effect.Playback.FrameRebuild",
        "Effect.Playback.HistoryUpdate",
        "Effect.Playback.Update",
        "Effect.Prepare.Commit",
        "Effect.Prepare.Document",
        "Effect.Prepare.Metadata",
        "Effect.Prepare.Renderer",
        "Effect.Prepare.WorkerTarget",
        "Effect.Prewarm.Advance",
        "Effect.ProductGroups.Update",
        "Effect.Rect.Render",
        "Effect.Service.Update",
        "Effect.Spawn.Commit",
        "Effect.Spawn.CommitWorldRoots",
        "Effect.Sprite.DepthSort",
        "Effect.Sprite.DrawSubmission",
        "Effect.Sprite.InstanceBuild",
        "Effect.Sprite.InstanceUpload",
        "Effect.Trail.Render",
        "Effect.TrailAfterimage.Simulate",
        "EffectSequencer.LaneLayout",
        "EffectSequencer.Render",
        "EffectTool.AllEffectsWindow",
        "EffectTool.ArtistF.MaterialPreparation",
        "EffectTool.ArtistF.SourcePreparation",
        "EffectTool.AuthoringWindow",
        "EffectTool.DataFilesWindow",
        "EffectTool.DetailWindow",
        "EffectTool.DocumentLoad",
        "EffectTool.DocumentLoad.CanonicalBaseline",
        "EffectTool.DocumentLoad.Parse",
        "EffectTool.DocumentLoad.ValidateDrawable",
        "EffectTool.InitialIndexStep",
        "EffectTool.ModelViewWindow",
        "EffectTool.Render",
        "EffectTool.ResourceGrid",
        "EffectTool.ThumbnailTrim",
        "EffectTool.UnifiedEffectTree",
        "EffectTool.UnifiedFamilyRows",
        "Engine.Camera.Update",
        "Engine.Input.Update",
        "Engine.LateUpdate",
        "Engine.LevelUpdate",
        "Engine.ObjectUpdate",
        "Engine.Physics",
        "Engine.PostPhysicsUpdate",
        "Engine.PriorityUpdate",
        "Engine.Sound.Update",
        "ImGui.Composition.Details",
        "ImGui.Composition.PatternTree",
        "ImGui.Composition.PatternTree.Rebuild",
        "ImGui.Composition.Patterns",
        "ImGui.Composition.PresentationIndex.Rebuild",
        "ImGui.Composition.PresentationResources",
        "ImGui.Composition.Resources",
        "ImGui.Composition.Resources.Filter",
        "ImGui.Composition.Resources.Tree.Draw",
        "ImGui.Composition.Resources.Tree.Rebuild",
        "ImGui.Composition.Timeline",
        "ImGui.Composition.Timeline.Draw",
        "ImGui.Composition.Timeline.Layout",
        "ImGui.Composition.Toolbar",
        "Kouku.Presentation.BundleSample",
        "Kouku.Presentation.EncounterVisuals",
        "Kouku.Presentation.FrameLights",
        "Kouku.Presentation.PreviewPreparation",
        "Kouku.Presentation.SharedPresentation",
        "Kouku.Presentation.Update",
        "ImGui.BackendSubmit",
        "ImGui.BuildAndSubmit",
        "ImGui.DX11.BackupState",
        "ImGui.DX11.DeviceObjects",
        "ImGui.DX11.DrawSubmission",
        "ImGui.DX11.GrowBuffers",
        "ImGui.DX11.MapBuffers",
        "ImGui.DX11.RestoreState",
        "ImGui.DX11.SetupState",
        "ImGui.DX11.TextureUpdate",
        "ImGui.DX11.UploadBuffers",
        "ImGui.DeveloperTools",
        "ImGui.FinalizeDrawData",
        "ImGui.Hub.Build",
        "ImGui.Hub.CameraAndPlayer",
        "ImGui.Hub.FollowCamera",
        "ImGui.Hub.KoukuArena",
        "ImGui.Hub.KoukuCompletePlay",
        "ImGui.Hub.KoukuUIPreview",
        "ImGui.Hub.LevelNavigation",
        "ImGui.Hub.SequenceViewer.Build",
        "ImGui.Hub.SequenceViewer.Refresh",
        "ImGui.Hub.SequenceViewer.Update",
        "ImGui.Hub.ServerArena",
        "ImGui.Hub.ValtanCompletePlay",
        "ImGui.NewFrame",
        "ImGui.NewFrame.Core",
        "ImGui.NewFrame.DX11",
        "ImGui.NewFrame.Win32",
        "ImGui.PlatformRender",
        "ImGui.PlatformUpdate",
        "ImGui.PlatformViewport.Present",
        "ImGui.PlatformViewport.Render",
        "ImGui.ProfilerDetails",
        "ImGui.ProfilerOverlay",
        "ImGui.RenderDrawData",
        "ImGui.Tool.Animation.Build",
        "ImGui.Tool.Animation.Update",
        "ImGui.Tool.Balance.Build",
        "ImGui.Tool.Balance.Update_ServerRuntimeSetPublishJob",
        "ImGui.Tool.Balance.Update_ValtanSaveJob",
        "ImGui.Tool.Camera.Build",
        "ImGui.Tool.Camera.Update",
        "UI.Runtime.Chat.Update",
        "ImGui.Tool.Composition.Build",
        "ImGui.Tool.CompositionProfiler.Build",
        "ImGui.Tool.EffectV1.Build",
        "ImGui.Tool.EffectV1.Update",
        "ImGui.Tool.EffectV1.Update_AuthoringWorkspace",
        "ImGui.Tool.EffectV2.Build",
        "ImGui.Tool.EffectV2.Update_AuthoringWorkspace",
        "ImGui.Tool.Equipment.Build",
        "ImGui.Tool.HUDLayout.Build",
        "ImGui.Tool.KoukuBoss.Build",
        "ImGui.Tool.Map.Build",
        "ImGui.Tool.Map.Update",
        "ImGui.Tool.Open",
        "UI.Runtime.Party.Update",
        "ImGui.Tool.Rendering.Build",
        "ImGui.Tool.SequenceBenchmark.Build",
        "ImGui.Tool.ValtanBoss.Build",
        "ImGui.Tool.ValtanBoss.Render_LogicPatternWindow",
        "ImGui.Tool.ValtanBoss.Update",
        "ImGui.Tool.ValtanComposition.Update_SaveState",
        "ImGui.Tool.WorldObjects.Build",
        "ImGui.Tool.WorldObjects.Update",
        "Level.Kouku.Camera.Create",
        "Level.Kouku.CameraShots.Load",
        "Level.Kouku.Controller.Prepare",
        "Level.Kouku.DebugTriggers.Load",
        "Level.Kouku.DeployCommit",
        "Level.Kouku.InitialVisibility",
        "Level.Kouku.Initialize",
        "Level.Kouku.MapLights.Load",
        "Level.Kouku.MapPlacementCommit",
        "Level.Kouku.Replication.Initialize",
        "Level.Kouku.SelfMotion.Load",
        "Level.Kouku.StageMarkers.Load",
        "Level.Kouku.UI.Create",
        "Level.Kouku.WorldSequence.Load",
        "Loader.EffectPreparation",
        "Loader.LevelLoad",
        "MainApp.DebugTools.Update",
        "MainApp.Engine.Update",
        "MainApp.InputAndUI.Update",
        "MainApp.LevelAndEnvironment.Update",
        "MainApp.Presentation.Prepare",
        "Map.Batch.BindAndDraw",
        "Map.Batch.CullAndPack",
        "Map.Batch.InstanceUpload",
        "Map.Batch.ShadowPrepare",
        "Map.Batch.ShadowUpload",
        "Map.Batch.Visibility",
        "Model.Load.Animations",
        "Model.Load.Binary",
        "Model.Load.Bones",
        "Model.Load.Decode",
        "Model.Load.Materials",
        "Model.Load.MaterialWorker",
        "Model.Load.MaterialJoin",
        "Model.Load.Meshes",
        "Texture.Cache.Lookup",
        "Texture.Cache.SameKeyWait",
        "Texture.Load.FileAndUpload",
        "Navigation.AStar",
        "Navigation.FindPath",
        "Navigation.Follow",
        "Navigation.Request",
        "Navigation.RoundCorners",
        "Navigation.Simplify",
        "Network.DrainAndDispatch",
        "Network.PlayerAssets.Advance",
        "Network.PlayerPresentation.CommitSpawn",
        "Network.PlayerPresentation.Replace",
        "Picking.CopyPixel",
        "Picking.MapWait",
        "Picking.ReadPixel",
        "Picking.Readback",
        "Profiler.Capture.Snapshot",
        "Profiler.Panel.Refresh",
        "Map.Batch.Material.Bind",
        "Map.Batch.Pass.Apply",
        "Map.Batch.Mesh.Submit",
        "Map.Shadow.Material.Bind",
        "Map.Shadow.Pass.Apply",
        "Map.Shadow.Mesh.Submit",
        "Effect.Particle.Update.Worker",
        "Effect.Particle.Update.Join",
        "Render.BeginFrame",
        "Render.Blend",
        "Render.Bloom",
        "Render.BossShowcase",
        "Render.Combined",
        "Render.Debug",
        "Render.DisplayOverlays",
        "Render.Draw",
        "Render.Final",
        "Render.Lights",
        "Render.Lights.WorldReceivers",
        "Render.Lights.CharacterReceivers",
        "Render.Lights.StageAndSubmit",
        "Render.Lights.UploadAndDraw",
        "Render.NonBlend",
        "Render.NonLight",
        "Render.Portraits",
        "Render.Present",
        "Render.Priority",
        "Render.FinalCameraSubmission",
        "Render.Picking",
        "Render.SSAO",
        "Render.SceneColorSnapshot",
        "Render.SceneHDR",
        "Render.ScreenPosts",
        "Render.Shadow",
        "Render.Shadow.CacheAdmission",
        "Render.Shadow.CacheCopy",
        "Render.Shadow.StaticBuild",
        "Render.Shadow.Dynamic",
        "Render.SubmitFrameProviders",
        "Render.UI",
        "Render.UIText",
        "Render.World",
        "Replication.Update",
        "Shader.BuildBindings",
        "Shader.CreateEffect",
        "Shader.CreateInputLayouts",
        "Shader.Load",
        "Shader.ReadBytecode",
        "Tool.Composition.ReloadCanonical",
        "Tool.Composition.SaveReload",
        "Tool.ValtanBossTool.ReloadCanonicalGraph",
        "UI.Runtime.BossHealthBar.Update",
        "UI.Runtime.BossImmuneGauge.Update",
        "UI.Runtime.CharacterSelectWindow.Update",
        "UI.Runtime.ChargeGauge.Update",
        "UI.Runtime.CombatHUD.Update",
        "UI.Runtime.EstherGauge.Update",
        "UI.Runtime.ItemQuickSlots.Update",
        "UI.Runtime.ItemUpgrade.Update",
        "UI.Runtime.LanceMasterIdentityGauge.Update",
        "UI.Runtime.LobbyButtons.Update",
        "UI.Runtime.Minimap.Update",
        "UI.Runtime.PlayerHealthManaBar.Update",
        "UI.Runtime.QuickSlotFlash.Update",
        "UI.Runtime.SkillCooldowns.Update",
        "UI.Runtime.SkillIcons.Update",
        "WorldSequence.CollectLoadTargets",
        "WorldSequence.CommitPrepared",
        "WorldSequence.Document.Load",
        "WorldSequence.Document.Parse",
        "WorldSequence.Document.Validate",
        "WorldSequence.PrepareArea",
    };
    constexpr const char* GPU_SCOPE_CATALOG[] = {
        "Render.SSGI",
        "Render.SSGI.GatherHalf",
        "Render.SSGI.ResolveHalf",
        "Render.SSR",
        "Render.ScreenSpaceLighting.Copy",
        "ImGui.BackendSubmit",
        "ImGui.PlatformViewport.Present",
        "ImGui.PlatformViewport.Render",
        "ImGui.RenderDrawData",
        "Picking.CopyPixel",
        "Render.BeginFrame",
        "Render.Blend",
        "Render.Bloom",
        "Render.BossShowcase",
        "Render.Combined",
        "Render.Debug",
        "Render.DisplayOverlays",
        "Render.Draw",
        "Render.Final",
        "Render.Lights",
        "Render.Lights.WorldReceivers",
        "Render.Lights.CharacterReceivers",
        "Render.NonBlend",
        "Render.NonLight",
        "Render.Portraits",
        "Render.Priority",
        "Render.FinalCameraSubmission",
        "Render.Picking",
        "Render.SSAO",
        "Render.SceneColorCopy",
        "Render.SceneHDR",
        "Render.ScreenPosts",
        "Render.Shadow",
        "Render.Shadow.CacheCopy",
        "Render.Shadow.StaticBuild",
        "Render.Shadow.Dynamic",
        "Render.UI",
        "Render.UIText",
    };
    constexpr ImGuiTableFlags TABLE_FLAGS = ImGuiTableFlags_RowBg |
        ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_ScrollY |
        ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp;

    constexpr const char* COUNTER_LABELS[] = {
        "계측된 Engine draw 호출",
        "인스턴싱 draw 호출",
        "인스턴싱 제출 인스턴스",
        "제출 인덱스 (인스턴스 반영)",
        "렌더 큐: 우선",
        "렌더 큐: 그림자",
        "렌더 큐: 불투명",
        "렌더 큐: 투명",
        "맵 배치 레코드",
        "맵 가시 인스턴스",
        "맵 배치 묶음",
        "맵 개별 오브젝트",
        "텍스처 요청",
        "텍스처 경로 캐시 적중",
        "텍스처 내용 캐시 적중",
        "새 텍스처 SRV 생성 성공",
        "텍스처 GPU 추정 바이트",
        "클라이언트 경로 탐색 요청",
        "클라이언트 탐색 노드",
        "클라이언트 경로 탐색 시간 (us)",
        "클라이언트 경로 셀",
        "장면 색 복사",
        "장면 색 복사 바이트 (원본 크기)",
        "ImGui draw 목록",
        "ImGui 정점",
        "ImGui 인덱스",
        "ImGui draw 명령",
        "ImGui 실제 draw 호출",
        "ImGui 콜백",
        "ImGui 렌더 창",
        "ImGui 활성 창",
        "ImGui 외부 뷰포트",
        "ImGui 정점 업로드 바이트",
        "ImGui 인덱스 업로드 바이트",
        "ImGui 상수 업로드 바이트",
        "ImGui 텍스처 업로드 바이트",
        "ImGui 버퍼 확장",
        "ImGui 텍스처 생성",
        "ImGui 텍스처 갱신",
        "ImGui 장치 객체 생성",
        "ImGui 버퍼 Map 실패",
        "피킹 GPU 읽기",
        "피킹 읽기 바이트",
        "간접 draw 호출",
        "간접 인덱스 (LOD0 상한)",
        "그림자 캐시 적중",
        "그림자 캐시 실패",
        "정적 그림자 제출 후보",
        "동적 그림자 제출 후보",
        "맵 컬링 후보",
        "맵 컬링 통과",
        "맵 LOD0 draw",
        "맵 LOD1 draw",
        "맵 LOD2 draw",
        "맵 원본 LOD0 인덱스",
        "맵 선택 LOD 제출 인덱스",
        "광원 제출 레코드 (수광 패스별 중복)",
        "광원 draw 호출",
        "광원 레코드 업로드 바이트",
        "생성 LOD가 있는 맵 draw",
        "로컬 광원 컬링 후보",
        "로컬 광원 컬링 제외",
        "이펙트 bounds 후보",
        "이펙트 bounds 컬링 제외",
        "트리거 마커 표본 시도",
        "트리거 진입 이력 요청",
        "주변 이펙트 정지",
        "주변 이펙트 진행",
        "ImGui Present 시도",
        "ImGui Present 지연 (busy)",
        "ImGui Present 실패",
        "ImGui Present 가림",
        "맵 가시성 캐시 적중",
        "맵 가시성 재계산",
        "빈 맵 배치 렌더",
        "가시 맵 배치 렌더",
        "맵 배치 bounds 제외",
        "맵 인스턴스 업로드 바이트",
        "저작 숨김 NPC 갱신",
        "bounds 무효화 주변 이펙트 갱신",
        "NPC 컬링 후보",
        "NPC 컬링 제외",
        "NPC 지연 pose 평가",
        "메시 제출 호출",
        "메시 인스턴스 제출",
        "메시 인덱스 제출",
        "고유 CMesh 객체",
        "고유 메시 계측 누락",
        "주변 이펙트 시간 제한 적용",
        "주변 이펙트 제외 시간 합계 (effect-us, CPU 시간 아님)",
        "주변 이펙트 fixed-step 합계",
        "주변 이펙트 단일 갱신 최대 fixed-step",
        "맵 공간 묶음 수",
        "맵 공간 묶음 근거리 제출",
        "맵 공간 묶음 원거리 제출",
        "맵 공간 묶음 원본 제출 수",
        "맵 공간 묶음 제출 인덱스",
        "맵 공간 묶음 원본 인덱스",
        "맵 공간 묶음 GPU 버퍼 바이트",
        "맵 공간 묶음 무효화",
        "맵 공간 묶음 활성 수",
        "맵 공간 묶음 HLOD 활성 수",
        "동일 모델 결합 전 source draw",
        "동일 모델 결합 draw",
        "조명 bank 결합 전 source draw",
        "조명 bank 결합 draw",
        "가림 검사 후보 배치",
        "가림 검사 실행 배치",
        "가림 제외 배치",
        "가림 제외 mesh 제출 (인스턴싱 결합 전)",
        "가림 제외 인덱스",
        "가림 깊이 생성 배치",
        "가림 깊이 래스터 삼각형",
        "거리 검사 인스턴스",
        "거리 제외 인스턴스",
        "거리 제외 원본 인덱스",
        "동일 카메라·정적 배치 가림 결과 재사용",
        "맵 CPU 준비 batch",
        "맵 CPU 큰 작업 묶음",
        "맵 CPU caller 완료 작업",
        "맵 CPU worker 완료 작업",
        "맵 CPU 보조 callback 제출",
        "공유 대상 애니메이션 샘플 요청",
        "애니메이션 샘플 캐시 재사용",
        "파티클 root 역행렬 요청",
        "파티클 root 역행렬 캐시 재사용",
        "파티클 CPU caller 완료 작업",
        "파티클 CPU worker 완료 작업",
        "파티클 CPU 보조 callback 제출",
    };
    static_assert(std::size(COUNTER_LABELS) == static_cast<size_t>(Engine::EProfilerCounter::Count));

    const char* Scope_Description(std::string_view name)
    {
        struct FLabel { std::string_view Name; const char* Description; };
        static constexpr FLabel labels[] = {
            {"Client.Update", "클라이언트 전체 갱신"}, {"Client.Render", "클라이언트 렌더 제출"},
            {"Render.World", "월드 렌더 제출"}, {"Render.Draw", "렌더 패스 전체"},
            {"Render.FinalCameraSubmission", "최종 카메라 가시성·제출"},
            {"Map.Visibility.Prepare", "맵 CPU 준비·분배 전체"},
            {"Map.Visibility.Dispatch", "맵 CPU 작업 실행·동기화"},
            {"Map.Visibility.Join", "맵 CPU worker 잔여 대기"},
            {"Map.Visibility.Worker", "맵 CPU 보조 스레드 계산"},
            {"Render.SubmitFrameProviders", "프레임 제공자 제출"},
            {"Render.NonBlend", "불투명 메시·G-buffer"}, {"Render.Shadow", "그림자 전체"},
            {"Render.Shadow.CacheAdmission", "그림자 캐시 조건 확인"},
            {"Render.Shadow.CacheCopy", "그림자 캐시 복사"},
            {"Render.Shadow.StaticBuild", "정적 그림자 생성"},
            {"Render.Shadow.Dynamic", "동적 그림자 생성"},
            {"Render.SSAO", "화면 공간 주변 차폐"}, {"Render.Lights", "월드 직접광 합산"},
            {"Render.SSGI", "실험 화면 공간 간접광"},
            {"Render.SSGI.GatherHalf", "GI 절반 해상도 계산"}, {"Render.SSGI.ResolveHalf", "GI 경계 보존 합성"}, {"Render.SSR", "실험 화면 공간 반사"},
            {"Render.ScreenSpaceLighting", "실험 화면 공간 조명 전체"},
            {"Render.ScreenSpaceLighting.Copy", "화면 공간 조명 입력 복사"},
            {"Render.Lights.WorldReceivers", "일반 수광체 직접광"},
            {"Render.Lights.CharacterReceivers", "캐릭터 수광체 직접광"},
            {"Render.Lights.StageAndSubmit", "광원 컬링·레코드 구성·제출"},
            {"Render.Lights.UploadAndDraw", "광원 상수 바인딩·draw 제출"},
            {"Render.SceneHDR", "HDR 장면 합성 전체"},
            {"Render.Combined", "직접광·베이크·환경광 합성"},
            {"Render.NonLight", "비조명 재질"}, {"Render.Blend", "투명 메시·이펙트"},
            {"Render.ScreenPosts", "화면 후처리"}, {"Render.Bloom", "빛 번짐"},
            {"Render.Final", "톤 매핑·색보정·최종 출력"},
            {"Render.DisplayOverlays", "출력 오버레이"}, {"Render.UI", "제품 UI"},
            {"Render.UIText", "제품 글자"}, {"Render.Debug", "디버그 도형"},
            {"Render.Picking", "선택 피킹"}, {"Render.Priority", "우선 렌더"},
            {"Render.Present", "화면 제출·드라이버 대기"}, {"Render.BeginFrame", "프레임 준비"},
            {"Render.Portraits", "캐릭터 초상 렌더"}, {"Render.BossShowcase", "보스 연출"},
            {"Render.SceneColorSnapshot", "장면 색 스냅샷 준비"},
            {"Render.SceneColorCopy", "장면 색 GPU 복사"},
            {"MainApp.InputAndUI.Update", "입력·UI 갱신"},
            {"MainApp.Engine.Update", "엔진 갱신"},
            {"MainApp.Presentation.Prepare", "표현 리소스 준비"},
            {"MainApp.DebugTools.Update", "저작 도구 갱신"},
            {"MainApp.LevelAndEnvironment.Update", "레벨·환경 갱신"},
            {"Engine.Physics", "물리 시뮬레이션"}, {"Engine.Sound.Update", "음향 갱신"},
            {"Engine.Camera.Update", "카메라 갱신"}, {"Engine.Input.Update", "입력 갱신"},
            {"Map.Batch.Visibility", "맵 배치 가시성 검사"},
            {"Map.Batch.Material.Bind", "맵 재질 바인딩"},
            {"Map.Batch.Pass.Apply", "맵 셰이더 패스 적용"},
            {"Map.Batch.Mesh.Submit", "맵 메시 draw 제출"},
            {"Map.Batch.InstanceUpload", "맵 인스턴스 업로드"},
            {"Map.Batch.CullAndPack", "맵 컬링·인스턴스 구성"},
            {"Map.Batch.BindAndDraw", "맵 바인딩·draw 전체"},
            {"ImGui.BackendSubmit", "도구 GPU 제출 준비"},
            {"ImGui.BuildAndSubmit", "도구 UI 구성·제출"},
            {"ImGui.RenderDrawData", "도구 UI draw 제출"},
            {"ImGui.PlatformViewport.Present", "외부 창 제출·대기"},
            {"Profiler.Panel.Refresh", "프로파일러 표 집계"},
            {"Profiler.Timeline.Snapshot", "타임라인 프레임 복사"},
            {"Profiler.Memory.Sample", "OS 메모리 표본 조회"},
            {"Profiler.Capture.Snapshot", "캡처 스냅샷 복사"},
            {"Network.DrainAndDispatch", "수신 패킷 배분 (클라이언트)"},
        };
        for (const auto& label : labels) if (label.Name == name) return label.Description;
        if (name.starts_with("ClassMovie.")) return "캐릭터 선택 무비 갱신·표본";
        if (name.starts_with("Animation.")) return "애니메이션·스키닝 CPU 준비";
        if (name.starts_with("Effect.")) return "이펙트 갱신·제출";
        if (name.starts_with("Map.")) return "맵 가시성·재질·제출";
        if (name.starts_with("Npc.")) return "NPC 갱신·제출";
        if (name.starts_with("Ambient.")) return "주변 이펙트";
        if (name.starts_with("ImGui.") || name.starts_with("EffectTool.") || name.starts_with("EffectSequencer.")) return "저작 도구 UI";
        if (name.starts_with("Navigation.")) return "클라이언트 경로 탐색";
        if (name.starts_with("Network.") || name.starts_with("Replication.")) return "네트워크·복제 표현";
        if (name.starts_with("UI.")) return "제품 UI 갱신";
        if (name.starts_with("Loader.") || name.starts_with("Model.Load") || name.starts_with("Texture.")) return "리소스 로드·준비";
        if (name.starts_with("Character")) return "캐릭터 준비·교체";
        if (name.starts_with("WorldSequence.") || name.starts_with("Kouku.")) return "장면·패턴 연출";
        if (name.starts_with("Shader.")) return "셰이더 준비";
        if (name.starts_with("Picking.")) return "선택·GPU 읽기";
        if (name.starts_with("Engine.") || name.starts_with("Level.")) return "엔진·레벨 처리";
        return "추가 계측 구간";
    }

    void Scope_Label(const char* name)
    {
        ImGui::Text("%s | %s", Scope_Description(name), name);
    }

    bool Contains_CaseInsensitive(std::string_view text, const char* query)
    {
        if (!query || !query[0]) return true;
        const std::string_view needle(query);
        return std::search(text.begin(), text.end(), needle.begin(), needle.end(),
            [](char a, char b) { return std::tolower(static_cast<unsigned char>(a)) ==
                std::tolower(static_cast<unsigned char>(b)); }) != text.end();
    }

    std::string Capture_PathLabel(const std::filesystem::path& path)
    {
        const auto text = path.u8string();
        return std::string(text.begin(), text.end());
    }

    const char* Gpu_Status(Engine::EProfilerGpuFrameStatus status)
    {
        switch (status)
        {
        case Engine::EProfilerGpuFrameStatus::Unsupported: return "미지원";
        case Engine::EProfilerGpuFrameStatus::Pending: return "결과 대기";
        case Engine::EProfilerGpuFrameStatus::Valid: return "유효";
        case Engine::EProfilerGpuFrameStatus::Disjoint: return "타임스탬프 무효";
        case Engine::EProfilerGpuFrameStatus::Dropped: return "누락";
        case Engine::EProfilerGpuFrameStatus::Error: return "쿼리 오류";
        }
        return "알 수 없음";
    }

    void Unobserved_Row(const char* name, int columns)
    {
        ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextDisabled("%s", name);
        for (int i = 1; i < columns; ++i)
        {
            ImGui::TableNextColumn(); ImGui::TextDisabled(i == 1 ? "미관측" : "--");
        }
    }
}

Client::CProfilerTool::CProfilerTool(ID3D11Device* device)
{
    if (!device) return;
    m_CaptureContext.DeviceCreationFlags = device->GetCreationFlags();
    Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
    Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
    DXGI_ADAPTER_DESC desc{};
    if (FAILED(device->QueryInterface(IID_PPV_ARGS(dxgiDevice.GetAddressOf()))) ||
        FAILED(dxgiDevice->GetAdapter(adapter.GetAddressOf())) || FAILED(adapter->GetDesc(&desc))) return;
    const int bytes = WideCharToMultiByte(CP_UTF8, 0, desc.Description, -1, nullptr, 0, nullptr, nullptr);
    if (bytes > 1)
    {
        std::string text(static_cast<size_t>(bytes), '\0');
        WideCharToMultiByte(CP_UTF8, 0, desc.Description, -1, text.data(), bytes, nullptr, nullptr);
        text.pop_back();
        m_CaptureContext.Adapter = std::move(text);
    }
}

void Client::CProfilerTool::Begin_Capture(Engine::CProfiler& profiler)
{
    profiler.Reset_History();
    CProfilerCaptureIO::Reset_MovementSamples();
    profiler.Set_Enabled(true);
    m_fLastRefreshTime = -1.0;
}

void Client::CProfilerTool::Refresh(Engine::CProfiler& profiler)
{
    if (!m_bCatalogRegistered)
    {
        for (const char* name : CPU_SCOPE_CATALOG) profiler.Register_ScopeName(name);
        for (const char* name : GPU_SCOPE_CATALOG) profiler.Register_ScopeName(name);
        m_bCatalogRegistered = true;
    }
    Engine::CProfilerScope scope(&profiler, "Profiler.Panel.Refresh");
    m_iMainThreadId = profiler.Get_MainThreadId();
    m_iHistoryFrames = profiler.Get_HistoryFrameCount();
    profiler.Get_ScopeNames(m_ScopeNames);
    const size_t window = static_cast<size_t>((std::max)(m_iWindowFrameInput, 1));
    m_SaveWindowCoverage = profiler.Get_CaptureWindow(Save_FrameWindow());
    profiler.Get_ScopeAggregates(window, m_Aggregates);
    profiler.Get_GpuScopeAggregates(window, m_GpuAggregates, m_iGpuValidFrames, m_iGpuPartialFrames);
    profiler.Get_WindowFrameStats(window, m_fWindowCpuAvgMs, m_fWindowCpuMaxMs,
        m_fWindowGpuAvgMs, m_fWindowGpuMaxMs, m_iWindowFrames, &m_iGpuFrameValidFrames);
    profiler.Get_LongOperations(m_LongOperations);
    m_bLiveValid = profiler.Get_LiveStats(m_Live);
    auto latest = profiler.Snapshot(1);
    m_LatestFrame = latest.Frames.empty() ? Engine::FProfilerFrame{} : std::move(latest.Frames.back());
    m_fUnattributedCpuMs = m_LatestFrame.CpuFrameMs;
    std::vector<std::pair<uint64_t, uint64_t>> intervals;
    for (const auto& sample : m_LatestFrame.CpuScopes)
    {
        if (sample.ThreadId != m_iMainThreadId) continue;
        const auto begin = (std::max)(sample.BeginTick, m_LatestFrame.FrameBeginTick);
        const auto end = (std::min)(sample.EndTick, m_LatestFrame.FrameEndTick);
        if (end > begin) intervals.emplace_back(begin, end);
    }
    std::sort(intervals.begin(), intervals.end());
    uint64_t covered = 0, end = m_LatestFrame.FrameBeginTick;
    for (const auto& range : intervals)
    {
        if (range.second > end) covered += range.second - (std::max)(end, range.first);
        end = (std::max)(end, range.second);
    }
    m_fUnattributedCpuMs = (std::max)(0.0, m_LatestFrame.CpuFrameMs - profiler.Ticks_ToMs(covered));
    m_bCpuRowsDirty = true;
    m_bComparisonRefresh = true;
    m_bTimelineRefresh = true;
}

const char_t* Client::CProfilerTool::Scope_Name(uint32_t id) const
{
    return id < m_ScopeNames.size() ? m_ScopeNames[id].c_str() : "<unknown>";
}

std::string Client::CProfilerTool::Thread_Label(uint32_t id) const
{
    return id == m_iMainThreadId ? "메인" : "작업 스레드 " + std::to_string(id);
}

size_t Client::CProfilerTool::Save_FrameWindow() const noexcept
{
    return m_bSaveWindowOnly ? static_cast<size_t>(std::clamp(m_iSaveFrameInput,
        1, static_cast<int32_t>(Engine::CProfiler::MAX_HISTORY_FRAMES))) : Engine::CProfiler::MAX_HISTORY_FRAMES;
}

void Client::CProfilerTool::Request_Save(Engine::CProfiler& profiler)
{
    if (m_Exporter.IsSaving()) return;
    std::string error;
    if (!CProfilerCaptureIO::Validate_Name(m_CaptureName.data(), &error))
    { m_strCaptureStatus = error; return; }
    Engine::FProfilerCaptureSnapshot snapshot;
    {
        Engine::CProfilerScope scope(&profiler, "Profiler.Capture.Snapshot");
        snapshot = profiler.Snapshot(Save_FrameWindow());
    }
    if (snapshot.Frames.empty())
    { m_strCaptureStatus = "완료 프레임이 없습니다. 수집을 켜고 기다린 뒤 저장하세요."; return; }
    auto context = Sample_Context();
    CProfilerCaptureIO::Copy_MovementSamples(snapshot, context);
    const uint64_t frame = snapshot.Frames.back().FrameNumber;
    std::filesystem::path output;
    if (!CProfilerCaptureIO::Make_NamedPath(m_CaptureName.data(), frame, output, &error))
    { m_strCaptureStatus = error; return; }
    // Describe this immutable export, not the live history that keeps changing while it is saved.
    const auto& coverage = snapshot.CaptureWindow;
    size_t gpuPending = 0, gpuDropped = 0;
    uint64_t cpuScopesDropped = 0, gpuScopesDropped = 0;
    for (const auto& savedFrame : snapshot.Frames)
    {
        gpuPending += savedFrame.GpuStatus == Engine::EProfilerGpuFrameStatus::Pending;
        gpuDropped += savedFrame.GpuStatus == Engine::EProfilerGpuFrameStatus::Dropped;
        cpuScopesDropped += savedFrame.DroppedCpuScopes;
        gpuScopesDropped += savedFrame.DroppedGpuScopes;
    }
    const std::string coverageText = std::to_string(snapshot.Frames.size()) + "프레임 (" +
        std::to_string(coverage.FirstSavedFrameNumber) + " - " + std::to_string(coverage.LastSavedFrameNumber) +
        ") | 보관 중 제외 " + std::to_string(coverage.ExcludedRetainedFrames) +
        " | 초기화 이후 퇴출 " + std::to_string(coverage.EvictedFramesSinceReset) +
        " | 저장 GPU 결과 대기 " + std::to_string(gpuPending) + " / 누락 " + std::to_string(gpuDropped) +
        " | 저장 CPU/GPU 구간 누락 " + std::to_string(cpuScopesDropped) + "/" + std::to_string(gpuScopesDropped);
    if (m_Exporter.BeginSave(std::move(snapshot), output, &error, std::move(context)))
    {
        m_strSavingCoverage = coverageText;
        m_strCaptureStatus = "백그라운드에서 JSON 저장 중... " + m_strSavingCoverage;
    }
    else m_strCaptureStatus = error;
}

Client::FProfilerCaptureContext Client::CProfilerTool::Sample_Context() const
{
    auto& game = Engine::CGameInstance::Get();
    auto context = m_CaptureContext;
    context.Valid = true;
    context.LevelId = game.Get_CurrentLevelID();
    const HWND foregroundWindow = GetForegroundWindow();
    DWORD foregroundProcessId = 0;
    context.ClientWindowForeground = foregroundWindow == g_hWnd;
    context.ProcessForeground = nullptr != foregroundWindow &&
        0 != GetWindowThreadProcessId(foregroundWindow, &foregroundProcessId) &&
        GetCurrentProcessId() == foregroundProcessId;
    context.WindowMinimized = CClientWindowDisplay::Is_Minimized();
    const auto& userSettings = CUserSettings::Get();
    context.ForegroundFpsLimit = userSettings.Get_FrameLimit(true);
    context.BackgroundFpsLimit = userSettings.Get_FrameLimit(false);
    context.EffectiveFpsLimit = userSettings.Get_FrameLimit(context.ProcessForeground);
    const auto viewport = game.Get_ViewportSize();
    context.Viewport = {viewport.x, viewport.y};
    if (const auto* camera = game.Get_CamPosition())
        context.CameraPosition = {camera->x, camera->y, camera->z, camera->w};
    if (const auto* view = game.Get_Transform(Engine::D3DTS::VIEW))
        std::memcpy(context.ViewMatrix.data(), view, sizeof(*view));
    if (const auto* projection = game.Get_Transform(Engine::D3DTS::PROJ))
        std::memcpy(context.ProjectionMatrix.data(), projection, sizeof(*projection));
    const auto& shadow = game.Get_ShadowLightDesc().Settings;
    context.ShadowEnabled = shadow.bEnabled;
    context.ShadowWidth = shadow.fOrthographicWidth;
    context.ShadowHeight = shadow.fOrthographicHeight;
    context.ShadowStrength = shadow.fStrength;
    const auto quality = game.Get_RenderQualitySettings();
    context.SSAOEnabled = quality.bSSAOEnabled;
    context.BloomEnabled = quality.bBloomEnabled;
    context.FXAAEnabled = quality.bFXAAEnabled;
    auto& options = context.RenderingOptions;
    options.clear(); context.RenderingAssets.clear();
    options["Texture.minimumMip"] = quality.iTextureMinMip;
    const auto visibility = game.Get_MapVisibilitySettings();
    options["MapVisibility.parallelPreparation"] = visibility.ParallelPreparationEnabled;
    options["MapVisibility.occlusion"] = visibility.OcclusionEnabled;
    options["MapVisibility.distance"] = visibility.DistanceEnabled;
    options["MapVisibility.distanceScale"] = visibility.DistanceScale;
    options["MapVisibility.distanceMaxPixels"] = visibility.DistanceMaxPixels;
    const auto vector3 = [&](const std::string& key, const auto& value)
    { options[key + ".x"] = value.x; options[key + ".y"] = value.y; options[key + ".z"] = value.z; };
    const auto vector4 = [&](const std::string& key, const auto& value)
    { vector3(key, value); options[key + ".w"] = value.w; };
    options["SSAO.radius"] = quality.fSSAORadius; options["SSAO.bias"] = quality.fSSAOBias;
    options["SSAO.intensity"] = quality.fSSAOIntensity; options["SSAO.power"] = quality.fSSAOPower;
    options["SSAO.distanceFade"] = quality.fSSAODistanceFade; options["SSAO.samples"] = quality.iSSAOSampleCount;
    options["SSGI.enabled"] = quality.bSSGIEnabled; options["SSGI.strength"] = quality.fSSGIStrength;
    options["SSGI.radius"] = quality.fSSGIRadius; options["SSGI.samples"] = quality.iSSGISampleCount;
    options["SSGI.halfResolution"] = quality.bSSGIHalfResolution;
    options["SSR.enabled"] = quality.bSSREnabled; options["SSR.strength"] = quality.fSSRStrength;
    options["SSR.maxDistance"] = quality.fSSRMaxDistance; options["SSR.thickness"] = quality.fSSRThickness;
    options["SSR.steps"] = quality.iSSRStepCount;
    options["Bloom.threshold"] = quality.fBloomThreshold; options["Bloom.softKnee"] = quality.fBloomSoftKnee;
    options["Bloom.intensity"] = quality.fBloomIntensity; options["Bloom.scatter"] = quality.fBloomScatter;
    vector4("Bloom.tint", quality.vBloomTint);
    options["Tone.exposure"] = quality.fExposure; options["Tone.whitePoint"] = quality.fWhitePoint;
    options["Tone.gamma"] = quality.fGamma; options["Tone.sceneDesaturation"] = quality.fSceneDesaturation;
    options["ColorFilter.type"] = quality.iColorFilterType; options["ColorFilter.strength"] = quality.fColorFilterStrength;
    options["FXAA.subpixel"] = quality.fFXAASubpixel; options["FXAA.edgeThreshold"] = quality.fFXAAEdgeThreshold;
    options["FXAA.edgeThresholdMin"] = quality.fFXAAEdgeThresholdMin;
    options["Shadow.near"] = shadow.fNear; options["Shadow.far"] = shadow.fFar;
    options["Shadow.depthBias"] = shadow.fDepthBias; options["Shadow.normalBias"] = shadow.fNormalBias;
    options["Shadow.dynamicBakedStrength"] = shadow.fDynamicBakedStrength;
    options["Shadow.PCFRadius"] = shadow.iPCFFilterRadius;
    const auto& source = quality.SourcePostProcess;
    options["SourcePost.enabled"] = source.bEnabled; options["SourcePost.scale"] = source.fToneScale;
    options["SourcePost.range"] = source.fToneRange; options["SourcePost.toe"] = source.fToneToe;
    options["SourcePost.desaturation"] = source.fDesaturation;
    vector3("SourcePost.highlights", source.vHighlights); vector3("SourcePost.midtones", source.vMidtones);
    vector3("SourcePost.shadows", source.vShadows); vector3("SourcePost.colorize", source.vColorize);
    options["SourcePost.LUTCount"] = double(source.LutLayers.size());
    for (size_t i = 0; i < (std::min)(source.LutLayers.size(), size_t(32)); ++i)
    {
        const auto name = "LUT." + std::to_string(i);
        options[name + ".weight"] = source.LutLayers[i].fWeight;
        context.RenderingAssets[name] = source.LutLayers[i].pLut ? source.LutLayers[i].pLut->strAssetId : "neutral";
    }
    const auto material = game.Get_MaterialRenderSettings();
    options["Material.source"] = material.bUseSourceMaterials;
    options["Material.debugView"] = static_cast<uint32_t>(material.eDebugView);
    options["MapPBR.enabled"] = material.MapPBR.bEnabled; options["MapPBR.level"] = material.MapPBR.iLevel;
    vector4("MapPBR.contribution", material.MapPBR.vContributionScale);
    vector4("MapPBR.surface", material.MapPBR.vSurfaceParameters);
    options["MapPBR.cubeDiffuseScale"] = material.MapPBR.fCubeDiffuseScale;
    const auto environment = game.Get_RenderEnvironment();
    vector4("Environment.color", environment.vColor); vector4("Environment.rotationIntensity", environment.vRotationIntensity);
    options["Environment.diffuseIntensity"] = environment.fDiffuseIntensity;
    options["Environment.sourcePBRIndirect"] = environment.bUseSourcePBRIndirect;
    context.RenderingAssets["Environment.cube"] = Capture_PathLabel(std::filesystem::path(environment.strCubePath));
    const auto fog = game.Get_HeightFogSettings();
    options["Fog.enabled"] = fog.bEnabled; options["Fog.sourceExponential"] = fog.bSourceExponential;
    vector4("Fog.color", fog.vColor); vector4("Fog.inscattering", fog.vInscatteringColor); vector4("Fog.lightDirection", fog.vFogLightDirection);
    options["Fog.density"] = fog.fDensity; options["Fog.heightFalloff"] = fog.fHeightFalloff;
    options["Fog.topHeight"] = fog.fTopHeight; options["Fog.startDistance"] = fog.fStartDistance;
    options["Fog.maximumOpacity"] = fog.fMaximumOpacity; options["Fog.driftSpeed"] = fog.fDriftSpeed;
    options["Fog.driftHeightAmplitude"] = fog.fDriftHeightAmplitude; options["Fog.driftDensityAmplitude"] = fog.fDriftDensityAmplitude;
    options["Fog.coveragePercent"] = fog.fCoveragePercent; options["Fog.windDirectionX"] = fog.fWindDirectionX;
    options["Fog.windDirectionZ"] = fog.fWindDirectionZ; options["Fog.windSpeed"] = fog.fWindSpeed;
    options["Fog.patchScale"] = fog.fPatchScale; options["Fog.patchSoftness"] = fog.fPatchSoftness;
    return context;
}

void Client::CProfilerTool::Update_SaveState()
{
    FProfilerCaptureSaveResult saveResult;
    if (!m_Exporter.Poll(saveResult)) return;
    m_strCaptureStatus = saveResult.Succeeded ?
        "Saved " + Capture_PathLabel(saveResult.OutputPath) + "\n" + m_strSavingCoverage : saveResult.Error;
    if (saveResult.Succeeded && Refresh_CaptureFiles())
        for (const auto& file : m_CaptureFiles)
            if (file.FileName == saveResult.OutputPath.filename())
            { m_strSelectedCaptureId = file.StableId; break; }
}

void Client::CProfilerTool::Render(Engine::CProfiler* profiler)
{
    if (!m_bOpen) return;
    ImGui::SetNextWindowSize(ImVec2(1060.f, 720.f), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("프레임 Profiler###LostArkProfilerToolV1", &m_bOpen))
    {
        ImGui::End(); return;
    }
    if (!profiler)
    {
        ImGui::TextUnformatted("Engine Profiler를 사용할 수 없습니다."); ImGui::End(); return;
    }

    if (!m_bCaptureFilesLoaded) Refresh_CaptureFiles();
    ImGui::SetNextItemWidth(300.f);
    ImGui::InputTextWithHint("저장 이름", "이름 입력 (한글 지원)", m_CaptureName.data(), m_CaptureName.size());
    ImGui::SameLine(); ImGui::TextDisabled("저장할 때마다 새 JSON 파일을 만듭니다.");
#ifdef _DEBUG
    ImGui::TextDisabled("Debug | F7: 창 표시 / 수집(Capture): 계측 시작·정지");
#else
    ImGui::TextDisabled("Release 빌드에서는 Profiler 창을 제공하지 않습니다.");
#endif
    ImGui::TextWrapped("비교 순서: 장면 준비 → 초기화 → F7로 창 숨김 → 같은 카메라·동작 재현 → 다시 열어 저장. 창을 닫아도 수집은 계속됩니다. JSON의 카메라·설정 정보는 저장 시점 값입니다.");
    bool enabled = profiler->Is_Enabled();
    if (ImGui::Checkbox("수집 (Capture)", &enabled))
    {
        profiler->Set_Enabled(enabled); m_fLastRefreshTime = -1.0;
    }
    ImGui::SameLine(); ImGui::SetNextItemWidth(100.f);
    if (ImGui::DragInt("분석·표시 프레임 범위", &m_iWindowFrameInput, 1.f, 1,
        static_cast<int>(Engine::CProfiler::MAX_HISTORY_FRAMES), "%d", ImGuiSliderFlags_AlwaysClamp))
        m_fLastRefreshTime = -1.0;
    ImGui::SameLine();
    if (ImGui::Button("초기화"))
    {
        profiler->Reset_History();
        CProfilerCaptureIO::Reset_MovementSamples();
        m_fLastRefreshTime = -1.0;
    }
    ImGui::SameLine();
    ImGui::BeginDisabled(m_Exporter.IsSaving());
    if (ImGui::Button("JSON 저장")) Request_Save(*profiler);
    ImGui::EndDisabled(); ImGui::SameLine();
    ImGui::TextDisabled("보관 프레임 %zu / %zu", m_iHistoryFrames, Engine::CProfiler::MAX_HISTORY_FRAMES);

    bool detailed = profiler->Is_DetailedScopesEnabled();
    if (ImGui::Checkbox("draw별 상세 CPU 계측 (추가 비용 발생)", &detailed))
    {
        profiler->Set_DetailedScopesEnabled(detailed);
        profiler->Reset_History();
        CProfilerCaptureIO::Reset_MovementSamples();
        m_fLastRefreshTime = -1.0;
    }
    if (ImGui::Checkbox("JSON 저장 범위 제한", &m_bSaveWindowOnly))
        m_fLastRefreshTime = -1.0;
    ImGui::SameLine();
    ImGui::BeginDisabled(!m_bSaveWindowOnly);
    ImGui::SetNextItemWidth(100.f);
    if (ImGui::DragInt("최근 프레임만 저장", &m_iSaveFrameInput, 1.f, 1,
        static_cast<int>(Engine::CProfiler::MAX_HISTORY_FRAMES), "%d", ImGuiSliderFlags_AlwaysClamp))
        m_fLastRefreshTime = -1.0;
    ImGui::EndDisabled();
    ImGui::TextDisabled("기본은 보관 전체 저장 (최대 %zu). 분석·표시 범위와 별도입니다.", Engine::CProfiler::MAX_HISTORY_FRAMES);
    ImGui::TextDisabled("GPU 결과 대기는 JSON에 pending으로 남으며, 완료된 0ms 측정값이 아닙니다.");

    const double now = ImGui::GetTime();
    if (m_fLastRefreshTime < 0.0 ||
        ((enabled || (m_bLiveValid && m_Live.LatestFrameGpuStatus == Engine::EProfilerGpuFrameStatus::Pending)) &&
            now - m_fLastRefreshTime >= m_fRefreshIntervalSeconds))
    {
        Refresh(*profiler); m_fLastRefreshTime = now;
    }
    ImGui::TextDisabled("저장 범위 %llu프레임 (%llu - %llu) | 보관 %llu | 초기화 이후 퇴출 %llu",
        static_cast<unsigned long long>(m_SaveWindowCoverage.SavedFrames),
        static_cast<unsigned long long>(m_SaveWindowCoverage.FirstSavedFrameNumber),
        static_cast<unsigned long long>(m_SaveWindowCoverage.LastSavedFrameNumber),
        static_cast<unsigned long long>(m_SaveWindowCoverage.RetainedFrames),
        static_cast<unsigned long long>(m_SaveWindowCoverage.EvictedFramesSinceReset));
    if (m_SaveWindowCoverage.ExcludedRetainedFrames != 0)
        ImGui::TextWrapped("선택 범위에서 보관 프레임 %llu개 제외 (최대 간격 %.2f ms). 전체 보관분을 저장하려면 JSON 저장 범위 제한을 끄세요.",
            static_cast<unsigned long long>(m_SaveWindowCoverage.ExcludedRetainedFrames),
            m_SaveWindowCoverage.ExcludedMaxFrameIntervalMs);
    if (m_SaveWindowCoverage.EvictedFramesSinceReset != 0)
        ImGui::TextWrapped("오래된 프레임은 1200개 보관 범위를 벗어나 복구할 수 없습니다. 보관 한도에 도달하기 전에 짧은 구간을 저장하세요.");
    if (ImGui::BeginTable("##FrameSummary", 3, ImGuiTableFlags_SizingStretchSame))
    {
        ImGui::TableNextColumn(); ImGui::TextDisabled("실제 프레임 간격 / FPS (최근)");
        if (m_bLiveValid && m_Live.FrameIntervalMs > 0.0)
            ImGui::Text("%.2f ms / %.1f FPS", m_Live.FrameIntervalMs, 1000.0 / m_Live.FrameIntervalMs);
        else ImGui::TextDisabled("--");
        ImGui::TableNextColumn(); ImGui::TextDisabled("CPU 처리 (구간 평균 / 최대)");
        if (m_iWindowFrames) ImGui::Text("%.2f / %.2f ms", m_fWindowCpuAvgMs, m_fWindowCpuMaxMs);
        else ImGui::TextDisabled("--");
        ImGui::TableNextColumn(); ImGui::TextDisabled("GPU 경과 시간 (유효 평균 / 최대)");
        if (m_iGpuFrameValidFrames > 0)
            ImGui::Text("%.2f / %.2f ms", m_fWindowGpuAvgMs, m_fWindowGpuMaxMs);
        else ImGui::TextDisabled("-- (유효 GPU 결과 없음)");
        ImGui::EndTable();
    }
    if (!enabled) ImGui::TextDisabled("수집 정지 상태입니다. 기존 결과는 유지됩니다.");
    if (m_bLiveValid && (m_Live.TotalDroppedCpuScopes || m_Live.TotalDroppedGpuFrames ||
        m_Live.TotalDroppedGpuScopes || m_Live.TotalDroppedModelAnimationSamples))
        ImGui::TextWrapped("초기화 이후 누락: CPU 구간 %llu / GPU 프레임 %llu / GPU 구간 %llu / 애니메이션 %llu",
            static_cast<unsigned long long>(m_Live.TotalDroppedCpuScopes), static_cast<unsigned long long>(m_Live.TotalDroppedGpuFrames),
            static_cast<unsigned long long>(m_Live.TotalDroppedGpuScopes), static_cast<unsigned long long>(m_Live.TotalDroppedModelAnimationSamples));
    if (m_bLiveValid && m_Live.DroppedCpuScopes)
        ImGui::TextColored(ImVec4(1.f, .65f, .25f, 1.f), "최근 프레임 CPU 구간 %llu개 누락: 전체·자체 비용 분석이 불완전합니다.",
            static_cast<unsigned long long>(m_Live.DroppedCpuScopes));
    if (m_bLiveValid && m_Live.TotalDroppedCpuScopes)
        ImGui::TextWrapped("CPU 구간 누락이 있습니다. 상세 계측을 끄고 초기화한 뒤 병목을 비교하세요. 누락이 있는 선택 구간의 자체 비용은 --로 표시합니다.");
    if (!m_strCaptureStatus.empty()) ImGui::TextWrapped("%s", m_strCaptureStatus.c_str());
    ImGui::Separator();
    ImGui::SetNextItemWidth(280.f);
    ImGui::InputTextWithHint("##ProfilerFilter", "이름·한국어 설명 검색", m_Filter.data(), m_Filter.size());
    ImGui::SameLine(); ImGui::Checkbox("미관측 구간 표시", &m_bShowUnobserved);
    if (ImGui::BeginTabBar("##ProfilerTabs"))
    {
        if (ImGui::BeginTabItem("한 프레임 해석")) { Render_FrameOverview(); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("타임라인###ScopeTimeline")) { Render_Timeline(*profiler); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("병목 후보·다음 실험###BottleneckCandidates")) { Render_Candidates(*profiler); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("RAM·GPU 메모리###MemoryTimeline")) { Render_Memory(*profiler); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("상용 엔진과 비교###CommercialProfilerReference")) { RenderingReferenceGuide::Render(); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("프레임 변화###FrameChanges")) { Render_FrameChanges(*profiler); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("기준 A/B###BaselineComparison")) { Render_Comparison(*profiler); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("CPU 병목")) { Render_Bottlenecks(); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("GPU 패스·draw")) { Render_Gpu(); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("ImGui 도구")) { Render_ImGui(); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("전체 작업량")) { Render_Counters(); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("긴 작업")) { Render_LongOperations(); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("저장 JSON")) { Render_CaptureFiles(); ImGui::EndTabItem(); }
        ImGui::EndTabBar();
    }
    ImGui::End();
}

void Client::CProfilerTool::Render_Bottlenecks(bool_t bImGuiOnly)
{
    if (ImGui::Checkbox("자체 비용 내림차순 (끄면 전체 비용)", &m_bCpuSortSelf)) m_bCpuRowsDirty = true;
    ImGui::SameLine();
    if (ImGui::Checkbox("메인 스레드만", &m_bMainThreadOnly)) m_bCpuRowsDirty = true;
    Rebuild_CpuRows(bImGuiOnly);
    if (m_bCpuSortSelf && std::any_of(m_Aggregates.begin(), m_Aggregates.end(), [](const auto& row) { return !row.SelfComplete; }))
        ImGui::TextWrapped("CPU 표본 누락: 자체 시간은 --로 표시하며 전체 비용 순서로 나열합니다.");
    ImGui::TextWrapped("전체는 자식 구간 포함, 자체는 같은 스레드의 계측된 자식 제외입니다. 부모와 자식 시간을 더하지 마세요. 자체 시간에는 미계측 작업·대기도 포함됩니다. 작업 스레드 시간은 CPU 프레임에 더하지 않습니다. 미관측은 0ms가 아닙니다.");
    const double frames = static_cast<double>((std::max)(m_iWindowFrames, size_t{1}));
    if (!ImGui::BeginTable("##CpuSections", 6, TABLE_FLAGS)) return;
    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableSetupColumn("구간", ImGuiTableColumnFlags_WidthStretch, 3.f);
    for (const char* label : { "스레드", "전체 ms/프레임", "자체 ms/프레임", "단일 호출 최대 ms", "호출/프레임" })
        ImGui::TableSetupColumn(label);
    ImGui::TableHeadersRow();
    ImGuiListClipper clipper;
    clipper.Begin(static_cast<int>(m_VisibleCpuRows.size()));
    while (clipper.Step())
        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
        {
            const auto& visible = m_VisibleCpuRows[static_cast<size_t>(i)];
            if (!visible.Aggregate) { Unobserved_Row(visible.Name, 6); continue; }
            const auto& row = *visible.Aggregate;
            ImGui::TableNextRow(); ImGui::TableNextColumn(); Scope_Label(visible.Name);
            ImGui::TableNextColumn();
            if (row.ThreadId == m_iMainThreadId) ImGui::TextUnformatted("메인");
            else ImGui::Text("작업 %u", row.ThreadId);
            ImGui::TableNextColumn(); ImGui::Text("%.3f", row.InclusiveMs / frames);
            ImGui::TableNextColumn();
            if (row.SelfComplete) ImGui::Text("%.3f", row.SelfMs / frames);
            else
            {
                ImGui::TextDisabled("--");
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("선택 범위에 CPU 구간 누락이 있어 자체 비용을 표시하지 않습니다. 빠진 자식을 부모 비용으로 오인할 수 있습니다. 상세 계측을 끄고 초기화해 다시 수집하세요.");
            }
            ImGui::TableNextColumn(); ImGui::Text("%.3f", row.MaxMs);
            ImGui::TableNextColumn(); ImGui::Text("%.2f", static_cast<double>(row.Calls) / frames);
        }
    ImGui::EndTable();
}

void Client::CProfilerTool::Rebuild_CpuRows(bool_t bImGuiOnly)
{
    if (!m_bCpuRowsDirty && m_strCpuRowFilter == m_Filter.data() &&
        m_bCpuRowsImGuiOnly == bImGuiOnly && m_bCpuRowsShowUnobserved == m_bShowUnobserved)
        return;
    m_VisibleCpuRows.clear();
    const auto matches = [&](const char* name)
    {
        const std::string_view text(name);
        const bool isImGui = text.starts_with("ImGui.") || text.starts_with("Profiler.Panel.") ||
            text.starts_with("Profiler.Capture.") || text.starts_with("EffectTool.") || text.starts_with("EffectSequencer.");
        return (!bImGuiOnly || isImGui) && (Contains_CaseInsensitive(text, m_Filter.data()) ||
            Contains_CaseInsensitive(Scope_Description(text), m_Filter.data()));
    };
    for (const auto& row : m_Aggregates)
        if ((!m_bMainThreadOnly || row.ThreadId == m_iMainThreadId) && matches(Scope_Name(row.NameId)))
            m_VisibleCpuRows.push_back({Scope_Name(row.NameId), &row});
    std::stable_sort(m_VisibleCpuRows.begin(), m_VisibleCpuRows.end(), [&](const auto& left, const auto& right)
    {
        const auto cost = [&](const auto& row) { return m_bCpuSortSelf && row.Aggregate->SelfComplete ? row.Aggregate->SelfMs : row.Aggregate->InclusiveMs; };
        return cost(left) > cost(right);
    });
    if (m_bShowUnobserved)
        for (const char* name : CPU_SCOPE_CATALOG)
            if (matches(name) && std::none_of(m_Aggregates.begin(), m_Aggregates.end(),
                [&](const auto& row) { return (!m_bMainThreadOnly || row.ThreadId == m_iMainThreadId) && std::string_view(name) == Scope_Name(row.NameId); }))
                m_VisibleCpuRows.push_back({name, nullptr});
    m_strCpuRowFilter = m_Filter.data();
    m_bCpuRowsImGuiOnly = bImGuiOnly;
    m_bCpuRowsShowUnobserved = m_bShowUnobserved;
    m_bCpuRowsDirty = false;
}

void Client::CProfilerTool::Render_ImGui()
{
    ImGui::TextWrapped("Build는 도구 코드·위젯 구성, DX11은 업로드·상태 변경·draw 제출입니다. 외부 창 Present에는 OS·드라이버 대기가 포함될 수 있습니다. 같은 장면에서 도구 닫힘·열림·외부 창 상태를 비교하세요.");
    if (m_bLiveValid)
    {
        const auto count = [&](Engine::EProfilerCounter counter)
        { return static_cast<unsigned long long>(m_Live.Counters[static_cast<size_t>(counter)]); };
        ImGui::Text("CPU 프레임 %llu | 창 %llu / 활성 %llu | 뷰포트 %llu | draw %llu / 명령 %llu",
            static_cast<unsigned long long>(m_Live.FrameNumber), count(Engine::EProfilerCounter::ImGuiRenderWindows),
            count(Engine::EProfilerCounter::ImGuiActiveWindows), count(Engine::EProfilerCounter::ImGuiPlatformViewports),
            count(Engine::EProfilerCounter::ImGuiDrawCalls), count(Engine::EProfilerCounter::ImGuiDrawCommands));
        ImGui::Text("외부 창 제출 %llu | 지연 %llu | 오류 %llu | 가림 %llu",
            count(Engine::EProfilerCounter::ImGuiPresentAttempts), count(Engine::EProfilerCounter::ImGuiPresentBusy),
            count(Engine::EProfilerCounter::ImGuiPresentFailures), count(Engine::EProfilerCounter::ImGuiPresentOccluded));
        ImGui::Text("정점 %llu / 인덱스 %llu | 정점+인덱스 업로드 %.1f KiB | 버퍼 확장 %llu | Map 실패 %llu",
            count(Engine::EProfilerCounter::ImGuiVertices), count(Engine::EProfilerCounter::ImGuiIndices),
            (count(Engine::EProfilerCounter::ImGuiVertexUploadBytes) + count(Engine::EProfilerCounter::ImGuiIndexUploadBytes)) / 1024.0,
            count(Engine::EProfilerCounter::ImGuiBufferGrowths), count(Engine::EProfilerCounter::ImGuiBufferMapFailures));
    }
    for (const auto& row : m_GpuAggregates)
        if (std::string_view(Scope_Name(row.NameId)) == "ImGui.RenderDrawData")
            ImGui::Text("GPU draw 구간: 평균 %.3f ms / P95 %.3f ms (완전 프레임 %zu개, 모든 뷰포트)",
                row.InclusiveMs / static_cast<double>((std::max)(m_iGpuValidFrames, size_t{1})), row.P95FrameMs, m_iGpuValidFrames);
    if (ImGui::TreeNode("도구 백엔드 작업량"))
    {
        if (ImGui::BeginTable("##ImGuiWorkload", 2, TABLE_FLAGS, ImVec2(0.f, 160.f)))
        {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("작업 항목"); ImGui::TableSetupColumn("최근 CPU 프레임"); ImGui::TableHeadersRow();
            for (size_t i = static_cast<size_t>(Engine::EProfilerCounter::ImGuiDrawLists); i <= static_cast<size_t>(Engine::EProfilerCounter::ImGuiBufferMapFailures); ++i)
            {
                ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextUnformatted(COUNTER_LABELS[i]); ImGui::TableNextColumn();
                if (m_bLiveValid) ImGui::Text("%llu", static_cast<unsigned long long>(m_Live.Counters[i]));
                else ImGui::TextDisabled("--");
            }
            ImGui::EndTable();
        }
        ImGui::TreePop();
    }
    Render_Bottlenecks(true);
}

void Client::CProfilerTool::Render_FrameOverview()
{
    if (!m_bLiveValid) { ImGui::TextDisabled("수집을 켜면 프레임 비용을 표시합니다."); return; }
    if (!ImGui::BeginChild("##FrameOverviewScroll")) { ImGui::EndChild(); return; }
    ImGui::TextWrapped("60 FPS 예산은 16.667 ms, 30 FPS는 33.333 ms입니다. CPU와 GPU는 병렬로 진행하므로 시간을 더하지 않습니다. 프레임 간격에는 메시지 처리·프레임 제한·대기·계측 정리도 포함됩니다.");
    ImGui::Text("최근 CPU 프레임 #%llu", static_cast<unsigned long long>(m_Live.FrameNumber));
    if (ImGui::BeginTable("##FrameAccounting", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH))
    {
        ImGui::TableSetupColumn("비용 경계"); ImGui::TableSetupColumn("ms"); ImGui::TableHeadersRow();
        const auto timing = [](const char* label, double ms)
        {
            ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextUnformatted(label);
            ImGui::TableNextColumn(); ImGui::Text("%.3f", ms);
        };
        timing("현재 CPU 처리 전체", m_Live.CpuFrameMs);
        timing("현재 CPU 계측 구간 합집합 (중복 제외)", (std::max)(0.0, m_Live.CpuFrameMs - m_fUnattributedCpuMs));
        timing("현재 CPU 구간 밖·누락 비용", m_fUnattributedCpuMs);
        if (m_Live.FrameIntervalMs > 0.0)
        {
            timing("이전 CPU 처리 (아래 프레임 간격의 일부)", m_Live.PreviousCpuFrameMs);
            timing("이전 처리 종료 → 현재 시작 (대기·메시지·정리)", m_Live.FrameGapMs);
            timing("이전 시작 → 현재 시작 (실제 프레임 간격)", m_Live.FrameIntervalMs);
        }
        ImGui::EndTable();
    }
    ImGui::TextWrapped("구간 밖 비용은 특정 병목으로 단정하지 않습니다. CPU 구간 안에서도 자체 시간에는 미계측 자식과 스케줄링 대기가 포함됩니다. 상세 계측이 꺼진 작업과 누락 표본은 0ms가 아닙니다.");
    const auto count = [&](Engine::EProfilerCounter c) { return static_cast<unsigned long long>(m_Live.Counters[static_cast<size_t>(c)]); };
    ImGui::Separator();
    ImGui::Text("계측된 Engine draw %llu + ImGui draw %llu", count(Engine::EProfilerCounter::DrawCalls), count(Engine::EProfilerCounter::ImGuiDrawCalls));
    ImGui::Text("인스턴싱 draw %llu | 인스턴싱 제출 수 %llu | 인덱스 %llu", count(Engine::EProfilerCounter::InstancedDrawCalls), count(Engine::EProfilerCounter::Instances), count(Engine::EProfilerCounter::Indices));
    ImGui::Text("메시 draw %llu | 메시 인스턴스 %llu | 메시 인덱스 %llu | 고유 CMesh %llu",
        count(Engine::EProfilerCounter::MeshDrawCalls), count(Engine::EProfilerCounter::MeshInstances),
        count(Engine::EProfilerCounter::MeshIndices), count(Engine::EProfilerCounter::UniqueMeshes));
    ImGui::Text("동일 모델 결합 %llu -> %llu draw | 조명 bank %llu -> %llu draw",
        count(Engine::EProfilerCounter::MapIdenticalInstanceSourceDraws), count(Engine::EProfilerCounter::MapIdenticalInstanceDraws),
        count(Engine::EProfilerCounter::MapLightingBankSourceDraws), count(Engine::EProfilerCounter::MapLightingBankDraws));
    ImGui::TextWrapped("결합 전 수는 이번 프레임에 개별 제출했을 경우의 source draw입니다. 이전 제품 대비 감소율이나 전체 장면의 절감량을 의미하지 않습니다.");
    auto visibility = Engine::CGameInstance::Get().Get_MapVisibilitySettings();
    bool changedVisibility = ImGui::Checkbox("Bern parallel visibility preparation", &visibility.ParallelPreparationEnabled);
    changedVisibility |= ImGui::Checkbox("Bern occlusion culling", &visibility.OcclusionEnabled);
    changedVisibility |= ImGui::Checkbox("Bern distance + screen-size culling", &visibility.DistanceEnabled);
    changedVisibility |= ImGui::SliderFloat("Distance range scale", &visibility.DistanceScale, .25f, 4.f, "%.2f");
    changedVisibility |= ImGui::SliderFloat("Distance maximum diameter (px)", &visibility.DistanceMaxPixels, 4.f, 128.f, "%.0f");
    if (changedVisibility) (void)Engine::CGameInstance::Get().Apply_MapVisibilitySettings(visibility);
    ImGui::Text("가림 제외 %llu batch | 이번 재검사 %llu | 제외 인덱스 %llu | 깊이 geometry %llu triangles",
        count(Engine::EProfilerCounter::MapOcclusionRejectedBatches), count(Engine::EProfilerCounter::MapOcclusionTested),
        count(Engine::EProfilerCounter::MapOcclusionRejectedIndices), count(Engine::EProfilerCounter::MapOcclusionRasterizedTriangles));
    ImGui::Text("거리 제외 %llu instances | 이번 재검사 %llu | 제외 원본 인덱스 %llu",
        count(Engine::EProfilerCounter::MapDistanceRejectedInstances), count(Engine::EProfilerCounter::MapDistanceTestedInstances),
        count(Engine::EProfilerCounter::MapDistanceRejectedIndices));
    ImGui::TextWrapped("컬링은 세션 설정입니다. 범위 scale이 작을수록 가까이서 제외하며, 픽셀 제한을 높이면 더 큰 소품도 제외합니다. 가림/거리 인덱스와 draw 합계를 구분하고 Render.MapOcclusion CPU 비용도 비교하세요.");

    ImGui::Text("맵 CPU 준비 %llu batch / %llu 작업 | caller %llu / worker %llu | 보조 callback %llu",
        count(Engine::EProfilerCounter::MapVisibilityPreparedBatches), count(Engine::EProfilerCounter::MapVisibilityCpuJobs),
        count(Engine::EProfilerCounter::MapVisibilityCallerJobs), count(Engine::EProfilerCounter::MapVisibilityWorkerJobs),
        count(Engine::EProfilerCounter::MapVisibilityAssistants));
    ImGui::TextWrapped("CPU 준비는 큰 작업만 worker로 나눕니다. 정지 카메라 재사용은 준비 0이며, Dispatch는 caller 계산과 Join 대기를 포함합니다. Worker 시간 합계를 메인 스레드 시간에 더하지 마세요.");

    if (count(Engine::EProfilerCounter::DroppedMeshSamples))
        ImGui::TextColored(ImVec4(1.f, .65f, .25f, 1.f), "고유 메시 표본 %llu개 누락: 고유 수는 하한입니다.", count(Engine::EProfilerCounter::DroppedMeshSamples));
    ImGui::TextWrapped("draw는 API 제출 횟수, 인스턴싱 제출 수는 instanced draw만의 반복 개수, 고유 CMesh는 서로 다른 CPU 메시 객체 수입니다. 인덱스는 인스턴스 수를 반영하며 고유 정점·오브젝트·화면 삼각형 수와 다릅니다. 그림자·초상 재제출도 포함합니다. DirectXTK 글자·디버그 도형 내부 draw는 위 합계에 미포함입니다. 간접 draw와 그 인덱스 상한은 생산자가 없는 미계측 예약 항목입니다.");
    ImGui::Separator();
    ImGui::Text("광원 레코드 제출 %llu | draw %llu | 업로드 %.1f KiB | 컬링 제외 %llu / %llu",
        count(Engine::EProfilerCounter::LightRecords), count(Engine::EProfilerCounter::LightDrawCalls),
        count(Engine::EProfilerCounter::LightUploadBytes) / 1024.0,
        count(Engine::EProfilerCounter::LightCullingRejected), count(Engine::EProfilerCounter::LightCullingCandidates));
    ImGui::TextWrapped("빛의 비용: CPU의 광원 컬링·구성·업로드, GPU의 그림자 생성·직접광·SSAO·간접광 합성을 각각 읽습니다. 수광 패스·초상마다 같은 광원을 재제출하므로 레코드는 유일 광원 수가 아닙니다. 픽셀 셰이더 호출 수는 수학 연산 수가 아닙니다. 투명 재질의 빛 계산은 Render.Blend에 포함됩니다.");
    if (ImGui::BeginTable("##LightingCost", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH))
    {
        ImGui::TableSetupColumn("빛 관련 GPU 구간"); ImGui::TableSetupColumn("전체 ms/프레임");
        ImGui::TableSetupColumn("자체 ms/프레임"); ImGui::TableHeadersRow();
        const double frames = static_cast<double>((std::max)(size_t{1}, m_iGpuValidFrames));
        for (const auto& row : m_GpuAggregates)
        {
            const std::string_view name(Scope_Name(row.NameId));
            if (!name.starts_with("Render.Lights") && !name.starts_with("Render.Shadow") &&
                name != "Render.SSAO" && name != "Render.Combined") continue;
            ImGui::TableNextRow(); ImGui::TableNextColumn(); Scope_Label(Scope_Name(row.NameId));
            ImGui::TableNextColumn(); ImGui::Text("%.3f", row.InclusiveMs / frames);
            ImGui::TableNextColumn(); ImGui::Text("%.3f", row.SelfMs / frames);
        }
        ImGui::EndTable();
    }
    if (!m_iGpuValidFrames) ImGui::TextDisabled("빛 GPU 구간은 아직 유효한 완료 결과가 없습니다.");
    ImGui::TextWrapped("이 표의 부모·자식 행은 중첩됩니다. 전체 값을 더하지 마세요. GPU timestamp는 명령 공급 대기를 포함한 경과 시간이며 GPU 점유율이 아닙니다. 개별 광원의 ALU·메모리·캐시 비용과 자원별 VRAM 귀속, Server 연산은 미계측입니다. 프로세스 GPU segment 사용량은 RAM·GPU 메모리 탭에서 확인합니다.");
    ImGui::EndChild();
}

void Client::CProfilerTool::Render_Gpu()
{
    ImGui::TextWrapped("GPU 결과는 제출했던 원래 프레임에 연결됩니다. 전체는 자식 포함, 자체는 계측된 자식 구간 제외입니다. CPU와 GPU·부모와 자식을 더하지 마세요. 불완전 GPU 프레임은 아래 평균·P95 분모에서 제외합니다.");
    ImGui::Text("완전 GPU 프레임 %zu / 구간 %zu | 제외된 부분 프레임 %zu", m_iGpuValidFrames, m_iWindowFrames, m_iGpuPartialFrames);
    if (m_bLiveValid)
    {
        ImGui::Text("최근 CPU #%llu 의 GPU 상태: %s", static_cast<unsigned long long>(m_Live.FrameNumber), Gpu_Status(m_Live.LatestFrameGpuStatus));
        if (m_Live.GpuValid) ImGui::Text("마지막 유효 GPU #%llu | 읽기 지연 %u프레임 | 누락 구간 %u",
            static_cast<unsigned long long>(m_Live.GpuFrameNumber), m_Live.GpuLatencyFrames, m_Live.DroppedGpuScopes);
        if (!m_Live.GpuScopesSupported) ImGui::TextDisabled("패스 query 미지원: 전체 프레임 계측만 가능할 수 있습니다.");
    }
    ImGui::Checkbox("GPU 자체 비용 내림차순 (끄면 전체 비용)", &m_bGpuSortSelf);
    std::stable_sort(m_GpuAggregates.begin(), m_GpuAggregates.end(), [&](const auto& left, const auto& right)
    { return (m_bGpuSortSelf ? left.SelfMs : left.InclusiveMs) > (m_bGpuSortSelf ? right.SelfMs : right.InclusiveMs); });
    const auto matches = [&](const char* name) { return Contains_CaseInsensitive(name, m_Filter.data()) || Contains_CaseInsensitive(Scope_Description(name), m_Filter.data()); };
    if (m_bLiveValid && m_Live.GpuValid && ImGui::TreeNode("마지막 유효 GPU 프레임 시간 순서"))
    {
        if (ImGui::BeginTable("##GpuIntervals", 5, TABLE_FLAGS, ImVec2(0, 180.f)))
        {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("패스", ImGuiTableColumnFlags_WidthStretch, 3.f);
            for (const char* label : {"시작 ms", "종료 ms", "전체 ms", "자체 ms"}) ImGui::TableSetupColumn(label);
            ImGui::TableHeadersRow();
            for (const auto& row : m_Live.GpuScopes)
            {
                if (!matches(Scope_Name(row.NameId))) continue;
                ImGui::TableNextRow(); ImGui::TableNextColumn();
                ImGui::Text("%*s%s | %s", static_cast<int>(row.Depth * 2), "", Scope_Description(Scope_Name(row.NameId)), Scope_Name(row.NameId));
                for (double value : {row.BeginMs, row.EndMs, row.DurationMs, row.SelfMs})
                { ImGui::TableNextColumn(); ImGui::Text("%.3f", value); }
            }
            ImGui::EndTable();
        }
        ImGui::TreePop();
    }
    const double frames = static_cast<double>((std::max)(m_iGpuValidFrames, size_t{1}));
    if (!ImGui::BeginTabBar("##GpuDetailTabs")) return;
    if (ImGui::BeginTabItem("시간·셰이더 처리량"))
    {
        ImGui::TextWrapped("PS/VS는 셰이더 호출 수, IA는 GPU 입력 정점·primitive 수입니다. 고유 메시 정점이나 정확한 광원 연산 수가 아닙니다. 선택한 패스만 pipeline 계측하며 --는 미지원·미계측·불완전입니다.");
        if (ImGui::BeginTable("##GpuPasses", 10, TABLE_FLAGS | ImGuiTableFlags_ScrollX, ImVec2(0, 0), 1650.f))
        {
            ImGui::TableSetupScrollFreeze(1, 1);
            ImGui::TableSetupColumn("패스", ImGuiTableColumnFlags_WidthFixed, 380.f);
            for (const char* label : {"자체 ms/프레임", "전체 ms/프레임", "자체 P95 ms", "전체 P95 ms", "전체 최대 ms", "PS/프레임", "VS/프레임", "IA 정점/프레임", "IA 도형/프레임"}) ImGui::TableSetupColumn(label);
            ImGui::TableHeadersRow();
            for (const auto& row : m_GpuAggregates)
            {
                if (!matches(Scope_Name(row.NameId))) continue;
                ImGui::TableNextRow(); ImGui::TableNextColumn(); Scope_Label(Scope_Name(row.NameId));
                for (double value : {row.SelfMs / frames, row.InclusiveMs / frames, row.SelfP95FrameMs, row.P95FrameMs, row.MaxFrameMs})
                { ImGui::TableNextColumn(); ImGui::Text("%.3f", value); }
                for (uint64_t value : {row.PSInvocations, row.VSInvocations, row.IAVertices, row.IAPrimitives})
                {
                    ImGui::TableNextColumn();
                    if (row.PipelineSamples && row.PipelineSamples == row.Calls) ImGui::Text("%.0f", value / frames);
                    else ImGui::TextDisabled("--");
                }
            }
            if (m_bShowUnobserved)
                for (const char* name : GPU_SCOPE_CATALOG)
                    if (matches(name) && std::none_of(m_GpuAggregates.begin(), m_GpuAggregates.end(), [&](const auto& row) { return name == std::string_view(Scope_Name(row.NameId)); }))
                        Unobserved_Row(name, 10);
            ImGui::EndTable();
        }
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("draw·메시·인덱스"))
    {
        ImGui::TextWrapped("아래는 같은 유효 GPU 프레임 구간에서 CPU가 제출한 Engine draw 누계입니다. GPU가 최종 표시한 수가 아닙니다. 부모 값은 자식을 포함합니다. ImGui·DirectXTK는 이 패스 draw 표에 미포함이며 ImGui는 전체 작업량에서 별도 확인합니다. 간접 draw·인덱스 상한은 생산자가 없는 미계측 예약 항목입니다.");
        if (ImGui::BeginTable("##GpuDrawWork", 9, TABLE_FLAGS | ImGuiTableFlags_ScrollX, ImVec2(0, 0), 1540.f))
        {
            ImGui::TableSetupScrollFreeze(1, 1);
            ImGui::TableSetupColumn("패스", ImGuiTableColumnFlags_WidthFixed, 380.f);
            for (const char* label : {"전체 ms/프레임", "draw/프레임", "인스턴싱 draw", "인스턴싱 제출 수", "직접 제출 인덱스", "메시 draw", "메시 인스턴스", "메시 인덱스"}) ImGui::TableSetupColumn(label);
            ImGui::TableHeadersRow();
            for (const auto& row : m_GpuAggregates)
            {
                if (!matches(Scope_Name(row.NameId))) continue;
                ImGui::TableNextRow(); ImGui::TableNextColumn(); Scope_Label(Scope_Name(row.NameId));
                ImGui::TableNextColumn(); ImGui::Text("%.3f", row.InclusiveMs / frames);
                for (uint64_t value : {row.Draw.DrawCalls, row.Draw.InstancedDrawCalls, row.Draw.Instances, row.Draw.Indices, row.Draw.MeshDrawCalls, row.Draw.MeshInstances, row.Draw.MeshIndices})
                { ImGui::TableNextColumn(); ImGui::Text("%.1f", value / frames); }
            }
            ImGui::EndTable();
        }
        ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
}

void Client::CProfilerTool::Render_Counters() const
{
    if (!m_bLiveValid) { ImGui::TextDisabled("수집된 프레임이 없습니다."); return; }
    if (!ImGui::BeginChild("##WorkloadScroll")) { ImGui::EndChild(); return; }
    const auto& animation = m_Live.Animation;
    ImGui::Text("CPU 프레임 %llu", static_cast<unsigned long long>(m_Live.FrameNumber));
    ImGui::TextWrapped("아래 고정 CPU 작업은 상세 모드와 관계없이 메인 스레드 호출을 누적합니다. 시간은 자식을 포함하므로 행끼리 더하지 마세요. 호출 수는 시도 횟수이며 화면 표시 수가 아닙니다.");
    if (ImGui::BeginTable("##CpuWorkCategories", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH))
    {
        ImGui::TableSetupColumn("CPU 작업");
        ImGui::TableSetupColumn("최근 프레임 호출");
        ImGui::TableSetupColumn("전체 ms");
        ImGui::TableHeadersRow();
        for (size_t i = 0; i < m_Live.CpuWork.size(); ++i)
        {
            const auto& work = m_Live.CpuWork[i];
            ImGui::TableNextRow(); ImGui::TableNextColumn();
            Scope_Label(Engine::CProfiler::Get_WorkName(static_cast<Engine::EProfilerWork>(i)));
            ImGui::TableNextColumn(); ImGui::Text("%llu", static_cast<unsigned long long>(work.Calls));
            ImGui::TableNextColumn(); ImGui::Text("%.3f", work.CpuMs);
        }
        ImGui::EndTable();
    }
    ImGui::Text("애니메이션 평가 %.3f ms | 호출 %llu / 모델 %llu", animation.CpuMs,
        static_cast<unsigned long long>(animation.UpdateCalls), static_cast<unsigned long long>(animation.UpdatedModels));
    ImGui::Text("갱신 후 미제출: 모델 %llu / %.3f ms", static_cast<unsigned long long>(animation.NotSubmittedUpdatedModels), animation.NotSubmittedCpuMs);
    if (animation.DroppedSamples) ImGui::Text("애니메이션 표본 상한: 이 프레임 %llu개 누락", static_cast<unsigned long long>(animation.DroppedSamples));
    ImGui::TextWrapped("미제출은 해당 프레임에 성공한 모델 draw가 없다는 뜻입니다. 숨김·도구 preview·화면 밖 모델 등을 포함하므로 프러스텀 컬링 수와 다릅니다. 이력 표본은 CPU 표에서 별도로 확인합니다.");
    if (m_Live.GpuValid)
        ImGui::Text("GPU 프레임 %llu: IA 입력 정점 %llu | VS 호출 %llu | PS 호출 %llu | 입력 primitive %llu",
            static_cast<unsigned long long>(m_Live.GpuFrameNumber),
            static_cast<unsigned long long>(m_Live.Pipeline.IAVertices), static_cast<unsigned long long>(m_Live.Pipeline.VSInvocations),
            static_cast<unsigned long long>(m_Live.Pipeline.PSInvocations), static_cast<unsigned long long>(m_Live.Pipeline.IAPrimitives));
    ImGui::TextWrapped("경로 탐색은 클라이언트 측정값입니다. Server 연산은 별도입니다. 텍스처 요청은 LoadSharedTexture 시도(실패 포함), 경로 적중은 weak cache 재사용, 새 SRV는 생성 성공 건수이며 상주 개수가 아닙니다. worker 요청과 생성 완료는 다른 프레임일 수 있어 당프레임 hit/request를 효율로 단정하지 않습니다. content cache·추정 GPU bytes·간접 draw는 미계측 예약 항목입니다.");
    ImGui::TextWrapped("맵 가시성 캐시 적중은 이전 카메라 결과 재사용입니다. 재계산 후보 0은 컬링 비활성화를 뜻하지 않습니다. 저작 숨김 NPC는 프러스텀과 별개이며, bounds 무효화 이펙트는 root 변경으로 기존 bounds가 무효화된 항목입니다.");
    if (ImGui::BeginTable("##WorkCounters", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH))
    {
        ImGui::TableSetupColumn("작업 항목"); ImGui::TableSetupColumn("최근 CPU 프레임"); ImGui::TableHeadersRow();
        for (size_t i = 0; i < std::size(COUNTER_LABELS); ++i)
        {
            ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextUnformatted(COUNTER_LABELS[i]);
            ImGui::TableNextColumn();
            if (i == static_cast<size_t>(Engine::EProfilerCounter::TextureContentHits) ||
                i == static_cast<size_t>(Engine::EProfilerCounter::TextureEstimatedGpuBytes) ||
                i == static_cast<size_t>(Engine::EProfilerCounter::IndirectDrawCalls) ||
                i == static_cast<size_t>(Engine::EProfilerCounter::IndirectIndexUpperBound))
                ImGui::TextDisabled("미계측");
            else ImGui::Text("%llu", static_cast<unsigned long long>(m_Live.Counters[i]));
        }
        ImGui::EndTable();
    }
    ImGui::EndChild();
}

void Client::CProfilerTool::Render_LongOperations()
{
    ImGui::TextWrapped("%.0f ms 이상 완료된 호출을 최신순으로 표시합니다. 여러 프레임에 걸친 worker 작업은 종료 프레임에 기록하며 메인 스레드 시간에 합산하지 않습니다.", Engine::CProfiler::LONG_OPERATION_THRESHOLD_MS);
    if (!ImGui::BeginTable("##LongOperations", 4, TABLE_FLAGS)) return;
    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableSetupColumn("구간", ImGuiTableColumnFlags_WidthStretch, 3.f);
    for (const char* label : {"스레드", "소요 ms", "완료 프레임"}) ImGui::TableSetupColumn(label);
    ImGui::TableHeadersRow();
    for (auto row = m_LongOperations.rbegin(); row != m_LongOperations.rend(); ++row)
    {
        if (!Contains_CaseInsensitive(Scope_Name(row->NameId), m_Filter.data()) &&
            !Contains_CaseInsensitive(Scope_Description(Scope_Name(row->NameId)), m_Filter.data())) continue;
        ImGui::TableNextRow(); ImGui::TableNextColumn(); Scope_Label(Scope_Name(row->NameId));
        ImGui::TableNextColumn(); ImGui::TextUnformatted(Thread_Label(row->ThreadId).c_str());
        ImGui::TableNextColumn(); ImGui::Text("%.3f", row->DurationMs);
        ImGui::TableNextColumn(); ImGui::Text("%llu", static_cast<unsigned long long>(row->FrameNumber));
    }
    ImGui::EndTable();
}


bool_t Client::CProfilerTool::Refresh_CaptureFiles()
{
    m_bCaptureFilesLoaded = true;
    if (!CProfilerCaptureIO::List_JsonFiles(CProfilerCaptureIO::Get_CaptureDirectory(),
        m_CaptureFiles, &m_strCaptureFilesStatus)) return false;
    if (std::none_of(m_CaptureFiles.begin(), m_CaptureFiles.end(),
        [&](const auto& file) { return file.StableId == m_strSelectedCaptureId; }))
        m_strSelectedCaptureId.clear();
    return true;
}

void Client::CProfilerTool::Render_CaptureFiles()
{
    if (ImGui::Button("목록 새로고침")) Refresh_CaptureFiles();
    ImGui::SameLine();
    const auto selected = std::find_if(m_CaptureFiles.begin(), m_CaptureFiles.end(),
        [&](const auto& file) { return file.StableId == m_strSelectedCaptureId; });
    ImGui::BeginDisabled(selected == m_CaptureFiles.end() || m_Exporter.IsSaving());
    if (ImGui::Button("선택 JSON 삭제") && selected != m_CaptureFiles.end())
    {
        const FProfilerCaptureFile file = *selected;
        std::string error;
        if (CProfilerCaptureIO::Delete_JsonFile(CProfilerCaptureIO::Get_CaptureDirectory(), file, &error))
        {
            m_strSelectedCaptureId.clear();
            std::erase_if(m_CaptureFiles, [&](const auto& row) { return row.StableId == file.StableId; });
            const bool refreshed = Refresh_CaptureFiles();
            m_strCaptureFilesStatus = "Deleted " + file.DisplayName +
                (refreshed ? std::string{} : ". Refresh failed: " + m_strCaptureFilesStatus);
        }
        else m_strCaptureFilesStatus = error;
    }
    ImGui::EndDisabled();
    ImGui::SameLine(); ImGui::TextDisabled("파일 %zu개", m_CaptureFiles.size());
    ImGui::TextWrapped("%s", Capture_PathLabel(CProfilerCaptureIO::Get_CaptureDirectory()).c_str());
    if (!m_strCaptureFilesStatus.empty()) ImGui::TextWrapped("%s", m_strCaptureFilesStatus.c_str());
    if (!ImGui::BeginTable("##SavedCaptures", 3, TABLE_FLAGS)) return;
    ImGui::TableSetupScrollFreeze(0, 1);
    ImGui::TableSetupColumn("파일", ImGuiTableColumnFlags_WidthStretch, 5.f);
    ImGui::TableSetupColumn("크기 (KiB)"); ImGui::TableSetupColumn("수정 시각"); ImGui::TableHeadersRow();
    ImGuiListClipper clipper;
    clipper.Begin(static_cast<int>(m_CaptureFiles.size()));
    while (clipper.Step())
        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
        {
            const auto& file = m_CaptureFiles[static_cast<size_t>(i)];
            ImGui::TableNextRow(); ImGui::TableNextColumn();
            ImGui::PushID(file.StableId.c_str());
            const ImVec2 labelPosition = ImGui::GetCursorScreenPos();
            if (ImGui::Selectable("##CaptureFile", file.StableId == m_strSelectedCaptureId,
                ImGuiSelectableFlags_SpanAllColumns, ImVec2(0.f, ImGui::GetTextLineHeight())))
                m_strSelectedCaptureId = file.StableId;
            ImGui::GetWindowDrawList()->AddText(labelPosition, ImGui::GetColorU32(ImGuiCol_Text), file.DisplayName.c_str());
            ImGui::PopID();
            ImGui::TableNextColumn(); ImGui::Text("%.1f", file.SizeBytes / 1024.0);
            ImGui::TableNextColumn();
            const FILETIME utc{static_cast<DWORD>(file.LastWriteTicks), static_cast<DWORD>(file.LastWriteTicks >> 32u)};
            FILETIME local{}; SYSTEMTIME time{};
            if (FileTimeToLocalFileTime(&utc, &local) && FileTimeToSystemTime(&local, &time))
                ImGui::Text("%04u-%02u-%02u %02u:%02u:%02u", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);
            else ImGui::TextDisabled("--");
        }
    ImGui::EndTable();
}

namespace
{
    using FComparisonMeans = std::map<std::string, Client::FProfilerComparisonMean>;

    std::string ComparisonMetricLabel(const std::string& key)
    {
        if (key == "frame::cpu") return "CPU 프레임 전체";
        if (key == "frame::interval") return "프레임 시작 간격 (이전 시작 → 현재 시작)";
        if (key == "frame::gap") return "이전 CPU 종료 → 현재 시작 대기·간격";
        if (key == "frame::previousCpu") return "간격을 구성한 이전 CPU 프레임";
        if (key == "frame::gpu") return "같은 프레임 번호의 GPU 전체";
        const auto split = key.find("::");
        const auto name = split == std::string::npos ? key : key.substr(split + 2);
        if (key.starts_with("counter::"))
            for (size_t i = 0; i < std::size(COUNTER_LABELS); ++i)
                if (name == Client::CProfilerCaptureIO::Counter_Name(i)) return COUNTER_LABELS[i];
        if (key.starts_with("animation."))
        {
            static const std::map<std::string, std::string> labels = {
                {"cpuMs", "애니메이션 평가 CPU 전체"}, {"notSubmittedCpuMs", "갱신 후 미제출 애니메이션 CPU"},
                {"updateCalls", "애니메이션 평가 호출"}, {"updatedModels", "애니메이션 갱신 모델"},
                {"submittedUpdatedModels", "갱신 후 제출 모델"}, {"notSubmittedUpdatedModels", "갱신 후 미제출 모델"},
                {"droppedSamples", "애니메이션 계측 표본 누락"}
            };
            const auto found = labels.find(name); return found == labels.end() ? name : found->second;
        }
        if (key.starts_with("memory::"))
        {
            static const std::map<std::string, std::string> labels = {
                {"privateCommitBytes", "프로세스 전용 commit"}, {"workingSetBytes", "프로세스 Working Set"},
                {"peakWorkingSetBytes", "프로세스 lifetime 최대 Working Set"}, {"peakPrivateCommitBytes", "프로세스 lifetime 최대 전용 commit"},
                {"systemCommitBytes", "시스템 commit"}, {"systemCommitLimitBytes", "시스템 commit 한도"}, {"systemAvailableBytes", "시스템 가용 RAM"},
                {"localUsageBytes", "DXGI Local 프로세스 사용량"}, {"localBudgetBytes", "DXGI Local 예산"},
                {"nonLocalUsageBytes", "DXGI Non-local 프로세스 사용량"}, {"nonLocalBudgetBytes", "DXGI Non-local 예산"}
            };
            const auto found = labels.find(name);
            return (found == labels.end() ? name : found->second) + " (MiB / OS 표본)";
        }
        const char* prefix = "";
        if (key.starts_with("cpu.main.self::")) prefix = "CPU 메인 자체 | ";
        else if (key.starts_with("cpu.main.total::")) prefix = "CPU 메인 전체 | ";
        else if (key.starts_with("cpu.worker.self::")) prefix = "CPU worker 합계 자체 | ";
        else if (key.starts_with("cpu.worker.total::")) prefix = "CPU worker 합계 전체 | ";
        else if (key.starts_with("gpu.self::")) prefix = "GPU 자체 | ";
        else if (key.starts_with("gpu.total::")) prefix = "GPU 전체 | ";
        else if (key.starts_with("work.time::")) prefix = "고정 CPU 작업 전체 | ";
        else if (key.starts_with("work.calls::")) prefix = "고정 CPU 작업 호출 | ";
        else if (key.starts_with("pipeline::"))
        {
            if (name == "iaVertices") return "GPU 프레임 IA 입력 정점";
            if (name == "iaPrimitives") return "GPU 프레임 IA 입력 primitive";
            if (name == "psInvocations") return "GPU 프레임 PS 호출";
            if (name == "vsInvocations") return "GPU 프레임 VS 호출";
            return "GPU 프레임 " + name;
        }
        else return key;
        const auto description = Scope_Description(name);
        return std::string(prefix) + (description && *description ? std::string(description) + " | " : "") + name;
    }

    bool ComparisonTimeKey(const std::string& key)
    { return key.starts_with("frame::") || key.starts_with("cpu.") || key.starts_with("work.time::") ||
        key.starts_with("animation.time::") || key.starts_with("gpu.self::") || key.starts_with("gpu.total::"); }

    const Client::FProfilerComparisonMean* ComparisonFind(const FComparisonMeans& means, const std::string& key)
    { const auto it = means.find(key); return it == means.end() ? nullptr : &it->second; }

    void ComparisonValue(const Client::FProfilerComparisonMean* value, bool time, double divisor = 1.0)
    {
        if (!value || !value->Available) ImGui::TextDisabled("미계측");
        else if (time) ImGui::Text("%.3f", value->Value);
        else ImGui::Text("%.2f", value->Value / divisor);
        if (value && ImGui::IsItemHovered())
            ImGui::SetTooltip("유효 관측 %zu / 대상 관측 %zu", value->Samples, value->Expected);
    }

    void ComparisonDelta(const FComparisonMeans& a, const FComparisonMeans& b, const std::string& key, bool time, double divisor = 1.0)
    {
        const auto* left = ComparisonFind(a, key); const auto* right = ComparisonFind(b, key);
        if (!left || !right || !left->Available || !right->Available) { ImGui::TextDisabled("--"); return; }
        const double delta = (right->Value - left->Value) / divisor;
        ImGui::TextColored(delta > 0.0 ? ImVec4(1.f, .65f, .3f, 1.f) : ImVec4(.55f, .85f, .65f, 1.f),
            time ? "%+.3f" : "%+.2f", delta);
    }

    // Pointers only live for this call; captures/mean maps cannot be replaced mid-table.
    void RenderComparisonRows(const FComparisonMeans& a, const FComparisonMeans& b, bool time, const char* filter)
    {
        struct FRow { std::string Key, Label; const Client::FProfilerComparisonMean* A = nullptr; const Client::FProfilerComparisonMean* B = nullptr; };
        std::map<std::string, FRow> joined;
        const auto add = [&](const FComparisonMeans& values, bool right)
        {
            for (const auto& [key, value] : values)
            {
                if (ComparisonTimeKey(key) != time || key.starts_with("gpu.draw.") || key.starts_with("gpu.pipeline.")) continue;
                auto& row = joined[key]; row.Key = key; row.Label = ComparisonMetricLabel(key);
                if (right) row.B = &value; else row.A = &value;
            }
        };
        add(a, false); add(b, true);
        std::vector<FRow> rows;
        for (auto& [key, row] : joined)
            if (Contains_CaseInsensitive(row.Label, filter) || Contains_CaseInsensitive(key, filter)) rows.push_back(std::move(row));
        const auto comparable = [](const FRow& row) { return row.A && row.B && row.A->Available && row.B->Available; };
        const auto delta = [](const FRow& row)
        { return (row.B->Value - row.A->Value) / (row.Key.starts_with("memory::") ? 1024.0 * 1024.0 : 1.0); };
        std::stable_sort(rows.begin(), rows.end(), [&](const auto& x, const auto& y)
        {
            if (comparable(x) != comparable(y)) return comparable(x);
            if (comparable(x) && delta(x) != delta(y)) return delta(x) > delta(y);
            return x.Key < y.Key;
        });
        ImGui::TextWrapped(time ?
            "B − A 증가 순위 (ms). 부모·자식과 CPU·GPU·worker·고정 작업은 중첩되므로 행을 합산하지 마세요. GPU 옆 제출량은 같은 패스의 전체 범위입니다. 고정 CPU 작업 옆에는 호출 차이를 표시합니다." :
            "B − A 수치 증가 순위. 단위가 다른 항목은 시간 병목 순위가 아닙니다. draw는 제출 횟수, 인덱스는 인스턴스 포함 제출량, IA는 GPU 입력량입니다. 인스턴싱 제출 수는 instanced draw만 셉니다. 광원 PS 호출은 정확한 빛 연산 횟수가 아닙니다.");
        if (!ImGui::BeginTable(time ? "##ComparisonTimes" : "##ComparisonWork", time ? 9 : 6, TABLE_FLAGS)) return;
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("항목", ImGuiTableColumnFlags_WidthStretch, 3.5f);
        for (const char* label : {"A", "B", "증감 B−A", "증감 %", "분모 A / B"}) ImGui::TableSetupColumn(label);
        if (time) for (const char* label : {"Δ draw/호출", "Δ 포함 index", "Δ PS 호출"}) ImGui::TableSetupColumn(label);
        ImGui::TableHeadersRow();
        ImGuiListClipper clipper; clipper.Begin(static_cast<int>(rows.size()));
        while (clipper.Step()) for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
        {
            const auto& row = rows[static_cast<size_t>(i)];
            const double divisor = row.Key.starts_with("memory::") ? 1024.0 * 1024.0 : 1.0;
            ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextUnformatted(row.Label.c_str());
            ImGui::TableNextColumn(); ComparisonValue(row.A, time, divisor);
            ImGui::TableNextColumn(); ComparisonValue(row.B, time, divisor);
            ImGui::TableNextColumn(); ComparisonDelta(a, b, row.Key, time, divisor);
            ImGui::TableNextColumn();
            if (comparable(row) && row.A->Value > 0.0) ImGui::Text("%+.1f%%", (row.B->Value / row.A->Value - 1.0) * 100.0);
            else ImGui::TextDisabled("--");
            ImGui::TableNextColumn();
            ImGui::Text("%zu/%zu | %zu/%zu", row.A ? row.A->Samples : 0, row.A ? row.A->Expected : 0,
                row.B ? row.B->Samples : 0, row.B ? row.B->Expected : 0);
            if (time)
            {
                const bool pass = row.Key.starts_with("gpu.");
                const auto name = pass ? row.Key.substr(row.Key.find("::") + 2) : std::string{};
                for (const auto* metric : {"drawCalls", "indices", "psInvocations"})
                {
                    ImGui::TableNextColumn();
                    if (pass) ComparisonDelta(a, b, std::string(metric == std::string_view("psInvocations") ? "gpu.pipeline." : "gpu.draw.") + metric + "::" + name, false);
                    else if (row.Key.starts_with("work.time::") && std::string_view(metric) == "drawCalls")
                        ComparisonDelta(a, b, "work.calls::" + row.Key.substr(row.Key.find("::") + 2), false);
                    else if (row.Key == "animation.time::cpuMs" && std::string_view(metric) == "drawCalls")
                        ComparisonDelta(a, b, "animation.count::updateCalls", false);
                    else if (row.Key == "frame::cpu" || row.Key == "frame::gpu")
                        ComparisonDelta(a, b, std::string(metric == std::string_view("psInvocations") ? "pipeline::" : "counter::") + metric, false);
                    else ImGui::TextDisabled("--");
                }
            }
        }
        ImGui::EndTable();
    }

    void RenderMeshDraws(const Client::FProfilerComparisonFrame& frame, const FComparisonMeans& values, const char* filter)
    {
        ImGui::TextWrapped("선택 프레임 #%llu의 CMesh 실제 제출 순서입니다. 이름은 표시용이며 asset·placement 고유 ID가 아닙니다. 개별 draw GPU 시간은 미계측입니다. 마지막 열은 같은 이름의 패스 전체 GPU 시간이며 draw마다 나누거나 합산하지 마세요.",
            static_cast<unsigned long long>(frame.Number));
        if (!frame.MeshDrawsKnown) { ImGui::TextUnformatted("상세 메시 draw 목록 미계측: 상세 수집을 켜고 새 프레임을 수집하세요. 과거 JSON의 필드 부재도 미계측입니다."); return; }
        ImGui::Text("보관 %zu / 누락 %llu (프레임당 상한 %zu)", frame.MeshDraws.size(),
            static_cast<unsigned long long>(frame.DroppedMeshDraws), Engine::CProfiler::MAX_MESH_DRAWS_PER_FRAME);
        ImGui::TextWrapped("정점은 CMesh geometry의 정점 수, index/draw는 실제 선택된 제출 인덱스 수 (LOD 반영), 제출 index는 index × instances입니다. 실제 참조된 고유 정점 수나 IA/VS 호출 수가 아닙니다. 패스 미계측·미지원·누락은 --로 표시합니다.");
        if (!ImGui::BeginTable("##MeshDrawTrace", 9, TABLE_FLAGS)) return;
        ImGui::TableSetupScrollFreeze(0, 1);
        for (const char* label : {"순서", "패스", "메시 이름", "재질 slot", "정점", "index/draw", "instances", "제출 index", "패스 전체 ms"}) ImGui::TableSetupColumn(label);
        ImGui::TableHeadersRow();
        for (size_t i = 0; i < frame.MeshDraws.size(); ++i)
        {
            const auto& draw = frame.MeshDraws[i];
            if (!Contains_CaseInsensitive(draw.Mesh, filter) && !Contains_CaseInsensitive(draw.Pass, filter) &&
                !Contains_CaseInsensitive(Scope_Description(draw.Pass), filter)) continue;
            ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::Text("%zu", i + 1);
            ImGui::TableNextColumn(); ImGui::TextUnformatted(draw.Pass.empty() ? "--" : draw.Pass.c_str());
            ImGui::TableNextColumn(); ImGui::TextUnformatted(draw.Mesh.empty() ? "이름 없음" : draw.Mesh.c_str());
            ImGui::TableNextColumn(); ImGui::Text("%u", draw.MaterialSlot);
            ImGui::TableNextColumn(); ImGui::Text("%u", draw.VertexCount);
            ImGui::TableNextColumn(); ImGui::Text("%u", draw.IndexCount);
            ImGui::TableNextColumn(); ImGui::Text("%u", draw.Instances);
            ImGui::TableNextColumn(); ImGui::Text("%llu", static_cast<unsigned long long>(uint64_t(draw.IndexCount) * draw.Instances));
            ImGui::TableNextColumn(); ComparisonValue(ComparisonFind(values, "gpu.total::" + draw.Pass), true);
        }
        ImGui::EndTable();
    }

    void RenderComparisonTables(const FComparisonMeans& a, const FComparisonMeans& b, const char* filter,
        const Client::FProfilerComparisonFrame* frame = nullptr)
    {
        if (!ImGui::BeginTabBar("##ComparisonMetricTabs")) return;
        if (ImGui::BeginTabItem("CPU·GPU 비용")) { RenderComparisonRows(a, b, true, filter); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("draw·메시·광원·컬링 작업량")) { RenderComparisonRows(a, b, false, filter); ImGui::EndTabItem(); }
        if (frame && ImGui::BeginTabItem("선택 프레임 메시 draw")) { RenderMeshDraws(*frame, b, filter); ImGui::EndTabItem(); }
        ImGui::EndTabBar();
    }

    const char* ComparisonGpuStatus(Engine::EProfilerGpuFrameStatus status)
    {
        switch (status)
        {
        case Engine::EProfilerGpuFrameStatus::Valid: return "유효";
        case Engine::EProfilerGpuFrameStatus::Pending: return "GPU 응답 대기";
        case Engine::EProfilerGpuFrameStatus::Disjoint: return "GPU clock 불연속";
        case Engine::EProfilerGpuFrameStatus::Dropped: return "GPU 수집 누락";
        case Engine::EProfilerGpuFrameStatus::Error: return "GPU query 오류";
        default: return "GPU 미지원·미계측";
        }
    }

    std::string ComparisonConditionLabel(const std::string& key)
    {
        static const std::map<std::string, std::string> labels = {
            {"levelId", "장면 Level ID"}, {"viewport", "렌더 viewport 해상도"}, {"cameraPosition", "카메라 위치"},
            {"viewMatrix", "카메라 view 행렬"}, {"projectionMatrix", "카메라 projection 행렬"},
            {"configuredForegroundFpsLimit", "전경 FPS 제한"}, {"configuredBackgroundFpsLimit", "배경 FPS 제한"},
            {"effectiveFpsLimit", "적용 FPS 제한 (0=해제)"}, {"foregroundWindowOwnedByProcess", "프로세스 전경"},
            {"clientWindowForeground", "메인 창 전경"}, {"windowMinimized", "최소화"},
            {"shadowEnabled", "그림자"}, {"shadowWidthHeightStrength", "그림자 폭·높이·강도"},
            {"ssaoEnabled", "SSAO"}, {"bloomEnabled", "Bloom"}, {"fxaaEnabled", "FXAA"},
            {"sampledAtExport", "보관 시점 context"}, {"runtimeContextValid", "장면 context 유효"},
            {"buildConfiguration", "빌드 구성"}, {"adapter", "GPU adapter"}, {"deviceCreationFlags", "D3D 생성 옵션"},
            {"d3dDebugLayer", "D3D debug layer"}, {"debuggerAttached", "디버거 연결"}, {"logicalProcessors", "논리 CPU 수"}
        };
        const auto found = labels.find(key); return found == labels.end() ? key : found->second;
    }

    void RenderComparisonConditions(const Client::FProfilerComparisonCapture& a, const Client::FProfilerComparisonCapture& b)
    {
        if (!ImGui::CollapsingHeader("비교 조건·수집 범위 (차이와 미계측 확인)", ImGuiTreeNodeFlags_DefaultOpen)) return;
        const auto summarize = [](const auto& capture)
        {
            size_t gpu = 0, passes = 0, detail = 0, unknown = 0, self = 0;
            for (const auto& frame : capture.Frames)
            { gpu += frame.GpuValid; passes += frame.GpuComplete; detail += frame.DetailKnown && frame.Detailed; unknown += !frame.DetailKnown; self += frame.CpuSelfKnown; }
            ImGui::TextWrapped("%s | %zu 프레임 | GPU 전체 %zu / 패스 %zu | CPU self 완전 %zu | 상세 ON %zu, 상태 미계측 %zu",
                capture.Label.c_str(), capture.Frames.size(), gpu, passes, self, detail, unknown);
            ImGui::TextDisabled("출처: %s", capture.Source.c_str());
        };
        summarize(a); summarize(b);
        if (!ImGui::BeginTable("##ComparisonConditions", 4, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY, ImVec2(0.f, 145.f))) return;
        ImGui::TableSetupScrollFreeze(0, 1);
        for (const char* label : {"조건", "A", "B", "판정"}) ImGui::TableSetupColumn(label);
        ImGui::TableHeadersRow();
        std::map<std::string, bool> keys;
        for (const auto& [key, value] : a.Conditions) keys[key] = true;
        for (const auto& [key, value] : b.Conditions) keys[key] = true;
        // These must remain visible even for legacy captures with no metadata.
        for (const auto* key : {"levelId", "viewport", "cameraPosition", "viewMatrix", "projectionMatrix", "effectiveFpsLimit",
            "shadowEnabled", "ssaoEnabled", "bloomEnabled", "fxaaEnabled"}) keys[key] = true;
        for (const auto& [key, ignored] : keys)
        {
            const auto left = a.Conditions.find(key), right = b.Conditions.find(key);
            const bool known = left != a.Conditions.end() && right != b.Conditions.end();
            ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextUnformatted(ComparisonConditionLabel(key).c_str());
            ImGui::TableNextColumn(); ImGui::TextWrapped("%s", left == a.Conditions.end() ? "미계측" : left->second.c_str());
            ImGui::TableNextColumn(); ImGui::TextWrapped("%s", right == b.Conditions.end() ? "미계측" : right->second.c_str());
            ImGui::TableNextColumn(); ImGui::TextUnformatted(!known ? "미계측" : left->second == right->second ? "같음" : "다름");
        }
        ImGui::EndTable();
    }
}

void Client::CProfilerTool::Render_FrameChanges(Engine::CProfiler& profiler)
{
    if (ImGui::Checkbox("최신 완료 프레임 따라가기", &m_bFollowComparisonFrames)) m_bComparisonRefresh = true;
    ImGui::SameLine();
    const bool refresh = ImGui::Button("현재 창 다시 가져오기");
    if (refresh || (m_bComparisonRefresh && m_bFollowComparisonFrames) || m_ComparisonFrames.Frames.empty())
    {
        auto snapshot = profiler.Snapshot(static_cast<size_t>((std::max)(2, m_iWindowFrameInput)));
        m_ComparisonFrames = CProfilerCaptureIO::Build_Comparison(snapshot, {}, "완료 프레임");
        m_iComparisonFrame = static_cast<int>(m_ComparisonFrames.Frames.size()) - 1;
        m_iComparedFrame = -1;
        m_bComparisonRefresh = false;
    }
    if (m_ComparisonFrames.Frames.empty()) { ImGui::TextUnformatted("수집된 완료 프레임이 없습니다."); return; }
    ImGui::TextWrapped("실제 완료 프레임을 선택합니다. 이 표는 선택 프레임 B와 바로 이전 보관 프레임 A의 차이이며, A/B 기준창 평균과 다릅니다. GPU는 각 프레임 번호에 귀속된 결과만 표시합니다. 대기 중 결과를 다른 프레임 GPU 값으로 대신하지 않습니다.");
    ImGui::SetNextItemWidth(360.f);
    if (ImGui::SliderInt("보관 프레임 위치", &m_iComparisonFrame, 0, static_cast<int>(m_ComparisonFrames.Frames.size()) - 1))
        m_bFollowComparisonFrames = false;
    const auto& current = m_ComparisonFrames.Frames[static_cast<size_t>(m_iComparisonFrame)];
    ImGui::Text("선택 B #%llu | %s | 상세 %s", static_cast<unsigned long long>(current.Number), ComparisonGpuStatus(current.GpuStatus),
        !current.DetailKnown ? "미계측" : current.Detailed ? "ON" : "OFF");
    if (m_iComparisonFrame == 0) ImGui::TextUnformatted("이전 보관 프레임 없음: A와 차이는 미계측입니다.");
    else
    {
        const auto& previous = m_ComparisonFrames.Frames[static_cast<size_t>(m_iComparisonFrame - 1)];
        ImGui::Text("이전 A #%llu | %s", static_cast<unsigned long long>(previous.Number), ComparisonGpuStatus(previous.GpuStatus));
        if (previous.Number + 1 != current.Number)
            ImGui::TextColored(ImVec4(1.f, .65f, .3f, 1.f), "보관 프레임 번호 사이에 공백이 있습니다. 연속 실행 1프레임 차이가 아닙니다.");
    }
    if (m_iComparedFrame != m_iComparisonFrame)
    {
        FProfilerComparisonCapture a, b; b.Frames.push_back(current);
        if (m_iComparisonFrame > 0) a.Frames.push_back(m_ComparisonFrames.Frames[static_cast<size_t>(m_iComparisonFrame - 1)]);
        m_FrameComparisonMeans[0] = CProfilerCaptureIO::Compare_Means(a);
        m_FrameComparisonMeans[1] = CProfilerCaptureIO::Compare_Means(b);
        m_iComparedFrame = m_iComparisonFrame;
    }
    RenderComparisonTables(m_FrameComparisonMeans[0], m_FrameComparisonMeans[1], m_Filter.data(), &current);
}

void Client::CProfilerTool::Render_Comparison(Engine::CProfiler& profiler)
{
    ImGui::TextWrapped("쿠크 2관문·빙고·현재 장면처럼 이름을 붙여 완료 프레임 창을 보관합니다. 각 기준은 독립 스냅샷이며 새 수집·Reset 뒤에도 유지됩니다. 저장 JSON은 '저장 JSON' 탭에서 선택한 파일을 사용합니다.");
    for (size_t side = 0; side < m_Baselines.size(); ++side)
    {
        ImGui::PushID(static_cast<int>(side));
        ImGui::SetNextItemWidth(260.f);
        ImGui::InputTextWithHint(side ? "B 이름" : "A 이름", side ? "빙고 / 현재 장면" : "쿠크 2관문 / 기준 장면",
            m_BaselineLabels[side].data(), m_BaselineLabels[side].size());
        ImGui::SameLine();
        if (ImGui::Button(side ? "현재 수집창 → B" : "현재 수집창 → A"))
        {
            Engine::CProfilerScope scope(&profiler, "Profiler.Comparison.Snapshot");
            auto snapshot = profiler.Snapshot(static_cast<size_t>((std::max)(1, m_iWindowFrameInput)));
            if (snapshot.Frames.empty()) m_ComparisonStatus = "완료 프레임이 없어 이전 기준을 유지했습니다.";
            else
            {
                auto label = std::string(m_BaselineLabels[side].data());
                if (label.empty()) label = side ? "B 현재 장면" : "A 기준 장면";
                auto staged = CProfilerCaptureIO::Build_Comparison(snapshot, Sample_Context(), std::move(label));
                auto means = CProfilerCaptureIO::Compare_Means(staged);
                m_Baselines[side] = std::move(staged); m_BaselineMeans[side] = std::move(means);
                m_ComparisonStatus = "완료 프레임 창을 이 실행의 기준으로 보관했습니다. 상단 JSON 저장은 저장 버튼을 누른 시점의 수집창을 저장합니다.";
            }
        }
        ImGui::SameLine();
        const auto selected = std::find_if(m_CaptureFiles.begin(), m_CaptureFiles.end(),
            [&](const auto& file) { return file.StableId == m_strSelectedCaptureId; });
        ImGui::BeginDisabled(selected == m_CaptureFiles.end() || m_Exporter.IsSaving());
        if (ImGui::Button(side ? "선택 JSON → B" : "선택 JSON → A") && selected != m_CaptureFiles.end())
        {
            FProfilerComparisonCapture staged; std::string error;
            if (CProfilerCaptureIO::Load_Comparison(CProfilerCaptureIO::Get_CaptureDirectory(), *selected, staged, &error))
            {
                if (m_BaselineLabels[side][0]) staged.Label = m_BaselineLabels[side].data();
                auto means = CProfilerCaptureIO::Compare_Means(staged);
                m_Baselines[side] = std::move(staged); m_BaselineMeans[side] = std::move(means);
                m_ComparisonStatus = "선택 JSON을 검증한 뒤 기준을 교체했습니다. 파일 읽기·분석을 실행한 현재 프레임은 측정 실험에서 제외하세요.";
            }
            else m_ComparisonStatus = "불러오기 실패: 이전 기준 유지. " + error;
        }
        ImGui::EndDisabled();
        if (selected != m_CaptureFiles.end()) ImGui::TextDisabled("선택 JSON: %s", selected->DisplayName.c_str());
        ImGui::PopID();
    }
    if (!m_ComparisonStatus.empty()) ImGui::TextWrapped("%s", m_ComparisonStatus.c_str());
    if (m_Baselines[0].Frames.empty() || m_Baselines[1].Frames.empty())
    { ImGui::TextUnformatted("A와 B를 각각 보관하거나 불러오면 창 평균·작업량 차이를 표시합니다."); return; }
    ImGui::TextWrapped("구조 비교: 장면·카메라·설정·FPS 제한·상세 계측·유효 GPU 분모 차이가 섞일 수 있습니다. 다른 장면의 차이를 한 옵션의 성능 효과로 단정하지 않습니다. context는 보관/저장 시점 표본이며 과거 창 전체가 같은 조건이었다는 보장은 없습니다. 개별 오브젝트 재질·광원별 설정은 전부 저장하지 않습니다.");
    RenderComparisonConditions(m_Baselines[0], m_Baselines[1]);
    ImGui::Separator();
    ImGui::TextWrapped("아래는 캡처창의 프레임당 평균입니다. CPU는 같은 이름과 main/worker 역할로 병합합니다. 고정 CPU 작업·애니메이션은 상세 OFF에서도 수집한 전체 비용이며 다른 scope와 합산하지 않습니다. GPU 패스는 유효·완료·누락 없음 프레임만 평균에 포함하며 분모를 표시합니다. 누락 필드·부분 self·생산자 없는 항목은 미계측입니다.");
    ImGui::TextWrapped("메모리 항목만은 예외로 같은 OS 조회를 재사용한 프레임을 중복 가중하지 않습니다. memory:: 평균·분모는 서로 다른 ProcessId·SampleTick·SampleFrameNumber의 실제 OS 조회 표본 기준입니다.");
    RenderComparisonTables(m_BaselineMeans[0], m_BaselineMeans[1], m_Filter.data());
}

bool Client::CProfilerTool::Refresh_RecentFrames(Engine::CProfiler& profiler, bool force)
{
    if (!force && !m_bTimelineRefresh && !m_RecentFrameSnapshot.Frames.empty()) return false;
    Engine::CProfilerScope scope(&profiler, "Profiler.Timeline.Snapshot");
    m_RecentFrameSnapshot = profiler.Snapshot(120);
    m_bTimelineRefresh = false;
    if (m_bTimelineFollow) m_iTimelineBuiltFrame = -1;
    return true;
}

const Engine::FProfilerCaptureSnapshot& Client::CProfilerTool::Timeline_Snapshot() const
{
    return m_bTimelineFollow ? m_RecentFrameSnapshot : m_TimelineSnapshot;
}

void Client::CProfilerTool::Freeze_Timeline()
{
    if (m_bTimelineFollow) m_TimelineSnapshot = m_RecentFrameSnapshot;
    m_bTimelineFollow = false;
}

void Client::CProfilerTool::Render_Timeline(Engine::CProfiler& profiler)
{
    bool follow = m_bTimelineFollow;
    const bool followChanged = ImGui::Checkbox("완료 프레임 따라가기###TimelineFollow", &follow);
    if (followChanged)
    {
        if (!follow) Freeze_Timeline();
        else { m_bTimelineFollow = true; m_bTimelineRefresh = true; }
    }
    ImGui::SameLine();
    const bool refresh = ImGui::Button("완료 프레임 다시 가져오기###TimelineRefresh");
    ImGui::SameLine();
    const bool modeChanged = ImGui::Checkbox("최신 GPU 완료 우선###TimelinePreferGpu", &m_bTimelinePreferGpu);
    if (m_bTimelineFollow || refresh || Timeline_Snapshot().Frames.empty())
    {
        const bool updated = Refresh_RecentFrames(profiler, refresh);
        if (!m_bTimelineFollow) m_TimelineSnapshot = m_RecentFrameSnapshot;
        if (updated || refresh || followChanged || modeChanged || m_iTimelineFrame < 0 || m_iTimelineBuiltFrame < 0)
        {
            m_iTimelineFrame = ProfilerLatestObservableFrame(Timeline_Snapshot(), m_bTimelinePreferGpu);
            m_iTimelineBuiltFrame = -1; m_iTimelineSelectedEvent = -1;
        }
    }
    if (Timeline_Snapshot().Frames.empty()) { ImGui::TextUnformatted("완료 프레임 없음: 수집을 켜고 프레임이 끝날 때까지 기다리세요."); return; }
    ImGui::TextWrapped("최근 최대 120개 완료 프레임의 실제 scope입니다. CPU는 프레임 CPU 시작 QPC 기준, GPU는 같은 번호 GPU query 시작 기준의 독립 상대축입니다. 두 축의 x 위치를 동기화된 시각이나 CPU→GPU 지연으로 해석하지 마세요. GPU 응답 대기를 다른 프레임 결과로 대체하지 않습니다.");
    ImGui::SetNextItemWidth(340.f);
    m_iTimelineFrame = (std::clamp)(m_iTimelineFrame, 0, static_cast<int>(Timeline_Snapshot().Frames.size()) - 1);
    if (ImGui::SliderInt("완료 프레임 위치###TimelineFrame", &m_iTimelineFrame, 0, static_cast<int>(Timeline_Snapshot().Frames.size()) - 1)) Freeze_Timeline();
    const auto& snapshot = Timeline_Snapshot();
    const auto& frame = snapshot.Frames[static_cast<size_t>(m_iTimelineFrame)];
    if (m_iTimelineBuiltFrame != m_iTimelineFrame)
    {
        m_TimelineEvents = ProfilerBuildTimelineEvents(frame, snapshot.TicksPerSecond);
        m_CpuTimelineView = {0.0, (std::max)(0.01, frame.CpuFrameMs)};
        m_GpuTimelineView = {0.0, (std::max)(0.01, frame.GpuFrameMs)};
        m_iTimelineBuiltFrame = m_iTimelineFrame; m_iTimelineSelectedEvent = -1;
    }
    const double covered = ProfilerMainCoveredMs(frame, snapshot.MainThreadId, snapshot.TicksPerSecond);
    ImGui::Text("선택 #%llu / 최근 CPU 완료 #%llu | %llu 프레임 지연 | %s", static_cast<unsigned long long>(frame.FrameNumber),
        static_cast<unsigned long long>(snapshot.Frames.back().FrameNumber),
        static_cast<unsigned long long>(snapshot.Frames.back().FrameNumber - frame.FrameNumber),
        m_bTimelineFollow ? (m_bTimelinePreferGpu ? "GPU 유효 완료 우선" : "최신 CPU 완료") : "수동 고정");
    ImGui::Text("프레임 #%llu | CPU %.3f ms | GPU %s | 메인 scope 합집합 %.3f ms / 구간 밖 %.3f ms",
        static_cast<unsigned long long>(frame.FrameNumber), frame.CpuFrameMs, ComparisonGpuStatus(frame.GpuStatus), covered,
        (std::max)(0.0, frame.CpuFrameMs - covered));
    ImGui::Text("상세 %s | CPU 표본 누락 %llu / GPU 패스 누락 %u / 메시 draw 누락 %llu",
        frame.DetailedCpuScopes ? "ON" : "OFF", static_cast<unsigned long long>(frame.DroppedCpuScopes),
        frame.DroppedGpuScopes, static_cast<unsigned long long>(frame.DroppedMeshDraws));
    if (ImGui::CollapsingHeader("타임라인 해석: 부모·자식 / self / 대기 / 프레임 경계"))
        ImGui::TextWrapped("행은 스레드와 호출 깊이(depth)입니다. 부모 전체 시간에는 자식이 포함되며 self는 같은 스레드의 관측된 직접 자식만 뺀 시간입니다. 미계측 자식·스케줄링 대기는 self에 남습니다. 빈 곳은 idle 증거가 아니며 OS context switch·task dependency·GPU queue/fence는 미계측입니다. worker는 완료 프레임에 귀속되어 이전 프레임에서 시작한 구간도 있습니다. 주황 테두리는 축에서 잘린 구간이며 원본 시간은 선택 정보에 남습니다. 창 밖에 있는 부모·누락·잘못된 중첩은 self를 미계측으로 만듭니다. 고정한 스냅샷의 pending GPU는 자동 갱신되지 않습니다.");
    if (m_iTimelineSelectedEvent >= 0 && static_cast<size_t>(m_iTimelineSelectedEvent) < m_TimelineEvents.size())
    {
        const auto& event = m_TimelineEvents[static_cast<size_t>(m_iTimelineSelectedEvent)];
        const char* name = event.NameId < snapshot.ScopeNames.size() ? snapshot.ScopeNames[event.NameId].c_str() : "<unknown>";
        ImGui::Separator(); Scope_Label(name);
        ImGui::Text("%s | thread %u / depth %u | [%.3f, %.3f] ms | 전체 %.3f ms%s", event.Gpu ? "GPU 상대축" : "CPU QPC 상대축",
            event.ThreadId, event.Depth, event.BeginMs, event.EndMs, event.EndMs - event.BeginMs, event.CrossFrame ? " | CPU 프레임 경계 밖 포함" : "");
        if (event.SelfKnown) ImGui::Text("관측 self %.3f ms (순수 연산 시간 아님)", event.SelfMs);
        else ImGui::TextDisabled("self 미계측: 누락·부모 부재·계층 불완전·CPU 프레임 경계 밖");
        if (event.Gpu)
        {
            ImGui::Text("패스 포함 draw %llu | index %llu | mesh draw %llu | instanced 제출 %llu",
                static_cast<unsigned long long>(event.Draw.DrawCalls), static_cast<unsigned long long>(event.Draw.Indices),
                static_cast<unsigned long long>(event.Draw.MeshDrawCalls), static_cast<unsigned long long>(event.Draw.Instances));
            if (event.PipelineValid) ImGui::Text("PS %llu / VS %llu / IA 입력 정점 %llu", static_cast<unsigned long long>(event.PSInvocations),
                static_cast<unsigned long long>(event.VSInvocations), static_cast<unsigned long long>(event.IAVertices));
            else ImGui::TextDisabled("해당 패스 pipeline 통계 미계측");
        }
        else ImGui::TextDisabled("CPU event별 draw 귀속은 미계측입니다. GPU 패스 또는 '선택 프레임 메시 draw'에서 확인하세요.");
        if (ImGui::SmallButton("선택 구간 확대###FitTimelineEvent"))
        {
            auto& view = event.Gpu ? m_GpuTimelineView : m_CpuTimelineView;
            view = {event.BeginMs, (std::max)(0.001, event.EndMs - event.BeginMs)};
        }
    }
    Render_TimelineAxis(false, frame);
    Render_TimelineAxis(true, frame);
}

void Client::CProfilerTool::Render_TimelineAxis(bool gpu, const Engine::FProfilerFrame& frame)
{
    const auto& snapshot = Timeline_Snapshot();
    ImGui::PushID(gpu ? "GpuTimeline" : "CpuTimeline");
    ImGui::SeparatorText(gpu ? "GPU 패스 — GPU frame 시작 기준 ms" : "CPU 스레드·호출 깊이 — CPU frame 시작 QPC 기준 ms");
    if (gpu && !frame.GpuValid)
    { ImGui::TextDisabled("%s: 해당 완료 프레임의 유효 GPU 시간 없음", ComparisonGpuStatus(frame.GpuStatus)); ImGui::PopID(); return; }
    if (!gpu && !snapshot.TicksPerSecond)
    { ImGui::TextDisabled("QPC 주파수 미계측"); ImGui::PopID(); return; }
    auto& view = gpu ? m_GpuTimelineView : m_CpuTimelineView;
    if (ImGui::SmallButton("확대")) view.Zoom(0.5);
    ImGui::SameLine(); if (ImGui::SmallButton("축소")) view.Zoom(2.0);
    ImGui::SameLine(); if (ImGui::SmallButton("왼쪽")) view.Pan(-0.25);
    ImGui::SameLine(); if (ImGui::SmallButton("오른쪽")) view.Pan(0.25);
    ImGui::SameLine(); if (ImGui::SmallButton("프레임에 맞춤")) view = {0.0, (std::max)(0.01, gpu ? frame.GpuFrameMs : frame.CpuFrameMs)};
    ImGui::SameLine(); ImGui::SetNextItemWidth(110.f); ImGui::InputDouble("시작 ms", &view.BeginMs, 0.0, 0.0, "%.3f");
    ImGui::SameLine(); ImGui::SetNextItemWidth(100.f); ImGui::InputDouble("폭 ms", &view.SpanMs, 0.0, 0.0, "%.3f");
    if (!std::isfinite(view.BeginMs)) view.BeginMs = 0.0;
    if (!std::isfinite(view.SpanMs)) view.SpanMs = 16.6667;
    view.BeginMs = (std::clamp)(view.BeginMs, -3600000.0, 3600000.0);
    view.SpanMs = (std::clamp)(view.SpanMs, 0.001, 3600000.0);
    using FLane = std::pair<uint32_t, uint32_t>;
    std::vector<FLane> lanes;
    for (const auto& event : m_TimelineEvents)
        if (event.Gpu == gpu && std::find(lanes.begin(), lanes.end(), FLane{event.ThreadId, event.Depth}) == lanes.end())
            lanes.emplace_back(event.ThreadId, event.Depth);
    std::sort(lanes.begin(), lanes.end(), [&](const auto& a, const auto& b)
    {
        if ((a.first == snapshot.MainThreadId) != (b.first == snapshot.MainThreadId)) return a.first == snapshot.MainThreadId;
        return a < b;
    });
    if (lanes.empty()) { ImGui::TextDisabled("이 축에서 관측된 구간이 없습니다. 미지원·미계측·미실행은 시간 0과 다릅니다."); ImGui::PopID(); return; }
    const float laneHeight = ImGui::GetTextLineHeightWithSpacing() + 5.f;
    const float labelWidth = 155.f, headerHeight = 28.f;
    ImGui::BeginChild("Canvas", ImVec2(0.f, gpu ? 230.f : 290.f), true);
    const auto origin = ImGui::GetCursorScreenPos();
    const float width = (std::max)(220.f, ImGui::GetContentRegionAvail().x);
    const float axisWidth = (std::max)(50.f, width - labelWidth - 5.f);
    const float height = headerHeight + laneHeight * static_cast<float>(lanes.size());
    ImGui::InvisibleButton("##TimelineCanvas", ImVec2(width, height));
    const bool canvasHovered = ImGui::IsItemHovered();
    const auto mouse = ImGui::GetIO().MousePos;
    auto* draw = ImGui::GetWindowDrawList();
    const auto visibleOrigin = ImGui::GetWindowPos(); const auto visibleSize = ImGui::GetWindowSize();
    draw->AddRectFilled(origin, ImVec2(origin.x + width, origin.y + height), IM_COL32(25, 29, 36, 255));
    for (int tick = 0; tick <= 5; ++tick)
    {
        const float x = origin.x + labelWidth + axisWidth * (float(tick) / 5.f);
        char text[48]; std::snprintf(text, sizeof(text), "%.3f", view.BeginMs + view.SpanMs * double(tick) / 5.0);
        draw->AddText(ImVec2(x, origin.y + 3.f), IM_COL32(190, 195, 205, 255), text);
        draw->AddLine(ImVec2(x, origin.y + headerHeight), ImVec2(x, origin.y + height), IM_COL32(55, 60, 72, 255));
    }
    for (size_t lane = 0; lane < lanes.size(); ++lane)
    {
        const float y = origin.y + headerHeight + laneHeight * float(lane);
        if (y + laneHeight < visibleOrigin.y || y > visibleOrigin.y + visibleSize.y) continue;
        char label[96];
        if (gpu) std::snprintf(label, sizeof(label), "GPU / depth %u", lanes[lane].second);
        else std::snprintf(label, sizeof(label), "%s %u / d%u", lanes[lane].first == snapshot.MainThreadId ? "Main" : "Worker", lanes[lane].first, lanes[lane].second);
        draw->AddText(ImVec2(origin.x + 3.f, y + 2.f), IM_COL32(205, 211, 220, 255), label);
        draw->AddLine(ImVec2(origin.x, y + laneHeight), ImVec2(origin.x + width, y + laneHeight), IM_COL32(49, 54, 65, 255));
    }
    int hovered = -1; size_t clipped = 0, crossFrame = 0, filtered = 0;
    draw->PushClipRect(ImVec2(origin.x + labelWidth, visibleOrigin.y), ImVec2(origin.x + width, visibleOrigin.y + visibleSize.y), true);
    for (size_t index = 0; index < m_TimelineEvents.size(); ++index)
    {
        const auto& event = m_TimelineEvents[index]; if (event.Gpu != gpu) continue;
        const char* name = event.NameId < snapshot.ScopeNames.size() ? snapshot.ScopeNames[event.NameId].c_str() : "<unknown>";
        if (!Contains_CaseInsensitive(name, m_Filter.data()) && !Contains_CaseInsensitive(Scope_Description(name), m_Filter.data())) { ++filtered; continue; }
        crossFrame += event.CrossFrame;
        const auto clip = ProfilerClipTimeline(event.BeginMs, event.EndMs, view); if (!clip.Visible) continue;
        clipped += clip.LeftClipped || clip.RightClipped;
        const auto lane = std::find(lanes.begin(), lanes.end(), FLane{event.ThreadId, event.Depth});
        const float y = origin.y + headerHeight + float(lane - lanes.begin()) * laneHeight;
        if (y + laneHeight < visibleOrigin.y || y > visibleOrigin.y + visibleSize.y) continue;
        const float x1 = origin.x + labelWidth + float(clip.BeginFraction) * axisWidth;
        const float x2 = (std::max)(x1 + 1.0f, origin.x + labelWidth + float(clip.EndFraction) * axisWidth);
        const ImVec2 lo(x1, y + 2.f), hi(x2, y + laneHeight - 2.f);
        const auto color = ImGui::ColorConvertFloat4ToU32(ImColor::HSV(float((event.NameId * 47u) % 360u) / 360.f, .55f, .68f).Value);
        draw->AddRectFilled(lo, hi, color, 2.f);
        if (clip.LeftClipped || clip.RightClipped) draw->AddRect(lo, hi, IM_COL32(255, 177, 60, 255), 2.f, 0, 2.f);
        if (static_cast<int>(index) == m_iTimelineSelectedEvent) draw->AddRect(lo, hi, IM_COL32(255, 255, 255, 255), 2.f, 0, 2.f);
        if (x2 - x1 > 35.f)
        {
            draw->PushClipRect(lo, hi, true); draw->AddText(ImVec2(x1 + 3.f, y + 3.f), IM_COL32(255, 255, 255, 255), name); draw->PopClipRect();
        }
        if (canvasHovered && mouse.x >= lo.x && mouse.x <= hi.x && mouse.y >= lo.y && mouse.y <= hi.y) hovered = static_cast<int>(index);
    }
    draw->PopClipRect();
    if (hovered >= 0)
    {
        const auto& event = m_TimelineEvents[static_cast<size_t>(hovered)];
        const char* name = event.NameId < snapshot.ScopeNames.size() ? snapshot.ScopeNames[event.NameId].c_str() : "<unknown>";
        ImGui::SetTooltip("%s\n%s\n전체 %.3f ms | 시작 %.3f / 종료 %.3f\n클릭하여 선택·스냅샷 고정", Scope_Description(name), name, event.EndMs - event.BeginMs, event.BeginMs, event.EndMs);
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) { m_iTimelineSelectedEvent = hovered; Freeze_Timeline(); }
    }
    ImGui::EndChild();
    ImGui::TextDisabled("현재 축에서 잘림 %zu | CPU 프레임 경계 밖 포함 %zu | 검색으로 숨김 %zu", clipped, crossFrame, filtered);
    ImGui::PopID();
}

void Client::CProfilerTool::Render_Candidates(Engine::CProfiler& profiler)
{
    Refresh_RecentFrames(profiler);
    const int selected = ProfilerLatestObservableFrame(m_RecentFrameSnapshot);
    if (selected < 0) { ImGui::TextUnformatted("완료 프레임이 없어 후보를 판단하지 않습니다."); return; }
    ImGui::TextWrapped("최근 완료 CPU 프레임과 그 번호에 귀속된 GPU 결과만 사용합니다. 아래는 원인 확정이 아닌 다음 검사를 고르는 후보입니다. 프레임 경과·대기·GPU timestamp와 순수 연산 비용은 다르며 서로 더하지 않습니다.");
    ImGui::SetNextItemWidth(120.f); ImGui::InputFloat("검토 목표 FPS", &m_fCandidateTargetFps, 1.f, 10.f, "%.1f");
    ImGui::SameLine(); ImGui::SetNextItemWidth(120.f); ImGui::InputInt("draw 검토 기준", &m_iCandidateDrawThreshold, 100, 1000);
    if (!std::isfinite(m_fCandidateTargetFps)) m_fCandidateTargetFps = 60.f;
    m_fCandidateTargetFps = (std::clamp)(m_fCandidateTargetFps, 1.f, 1000.f);
    m_iCandidateDrawThreshold = (std::clamp)(m_iCandidateDrawThreshold, 1, 10000000);
    const auto& frame = m_RecentFrameSnapshot.Frames[static_cast<size_t>(selected)];
    const auto evidence = ProfilerAssessCandidates(frame, 1000.0 / m_fCandidateTargetFps, static_cast<uint64_t>(m_iCandidateDrawThreshold));
    ImGui::Text("프레임 #%llu | 검토 예산 %.3f ms | CPU %.3f ms | GPU %s", static_cast<unsigned long long>(frame.FrameNumber),
        evidence.TargetMs, frame.CpuFrameMs, ComparisonGpuStatus(frame.GpuStatus));
    ImGui::Text("최근 CPU 완료 #%llu보다 %llu 프레임 지연 | %s",
        static_cast<unsigned long long>(m_RecentFrameSnapshot.Frames.back().FrameNumber),
        static_cast<unsigned long long>(m_RecentFrameSnapshot.Frames.back().FrameNumber - frame.FrameNumber),
        frame.GpuValid ? "최근 120개 중 최신 GPU 유효 완료: CPU·GPU 같은 원본 프레임" : "유효 GPU 완료 없음: 최신 CPU 원본의 pending / 미계측 상태 유지");
    ImGui::TextDisabled("기준값은 이 패널의 후보 표시만 바꿉니다. 실제 FPS 제한이나 렌더 옵션을 변경하지 않습니다.");
    if (ImGui::BeginTable("##CandidateExperiments", 4, TABLE_FLAGS, ImVec2(0.f, 330.f)))
    {
        ImGui::TableSetupColumn("후보 / 상태", ImGuiTableColumnFlags_WidthFixed, 180.f);
        ImGui::TableSetupColumn("현재 표본 근거", ImGuiTableColumnFlags_WidthStretch, 1.f);
        ImGui::TableSetupColumn("다음 A/B·확인", ImGuiTableColumnFlags_WidthStretch, 1.3f);
        ImGui::TableSetupColumn("아직 모르는 원인", ImGuiTableColumnFlags_WidthStretch, 1.f); ImGui::TableHeadersRow();
        const auto row = [&](const char* name, bool candidate, const std::string& observed, const char* next, const char* missing)
        {
            ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextWrapped("%s\n%s", name, candidate ? "검토 후보" : "기준 미충족 / 보류");
            ImGui::TableNextColumn(); ImGui::TextWrapped("%s", observed.c_str());
            ImGui::TableNextColumn(); ImGui::TextWrapped("%s", next);
            ImGui::TableNextColumn(); ImGui::TextWrapped("%s", missing);
        };
        const auto ms = [](double value) { char text[40]; std::snprintf(text, sizeof(text), "%.3f ms", value); return std::string(text); };
        row("CPU 경과", evidence.CpuOverBudget, "전체 " + ms(frame.CpuFrameMs) + " / 예산 " + ms(evidence.TargetMs),
            "CPU 타임라인에서 큰 self·고정 작업·Present 구간을 확인하고 같은 장면에서 한 변수만 바꿔 A/B 비교합니다.",
            "OS 스케줄링·동기화 wait·디스크·page fault는 미계측입니다. CPU 우세만으로 계산 병목을 확정하지 않습니다.");
        row("GPU 경과", evidence.GpuOverBudget, frame.GpuValid ? "원래 프레임 GPU " + ms(frame.GpuFrameMs) : ComparisonGpuStatus(frame.GpuStatus),
            "GPU 패스와 포함 draw/PS·IA를 확인합니다. 같은 카메라에서 해상도·SSAO 표본·그림자 PCF 중 하나를 바꿔 비교합니다.",
            "queue stall·캐시·대역폭·wave 점유율·개별 draw ms는 미계측입니다. PS 호출은 광원 ALU 수가 아닙니다.");
        row("프레임 사이 간격", evidence.GapLarge, "이전 CPU " + ms(frame.PreviousCpuFrameMs) + " + gap " + ms(frame.FrameGapMs),
            "FPS cap·전경/배경·최소화 조건을 맞추고 A/B합니다. CPU/GPU 시간이 낮아도 cap 대기로 FPS가 제한될 수 있습니다.",
            "gap에는 loop 대기·메시지·계측 관리가 섞입니다. 현재 CPU 시간을 이전 CPU 대신 더하지 않습니다.");
        row("갱신 후 미제출 애니메이션", evidence.AnimationNotSubmitted,
            "모델 " + std::to_string(frame.Animation.NotSubmittedUpdatedModels) + " / CPU " + ms(frame.Animation.NotSubmittedCpuMs),
            "동일 장면의 카메라·저작 숨김·NPC culling·지연 pose 조건을 확인하고 미제출 CPU와 표시 결과를 함께 비교합니다.",
            "미제출은 화면 밖 확정이 아닙니다. 숨김·preview·draw 실패도 포함하며 animation 표본 누락이면 후보를 보류합니다.");
        row("많은 제출", evidence.ManySubmissions, "Engine + ImGui draw " + std::to_string(evidence.DrawCalls) + " / 검토 기준 " + std::to_string(m_iCandidateDrawThreshold),
            "패스·상세 메시 목록에서 반복 제출을 보고 instancing·LOD·culling을 한 항목씩 비교합니다. UI 창 수도 동일하게 맞춥니다.",
            "호출 수만으로 비용을 확정하지 않습니다. 재질 전환·shader 복잡도·overdraw·DirectXTK 내부 draw는 전부 귀속되지 않습니다.");
        row("계측 범위", evidence.MissingCoverage, "CPU 누락 " + std::to_string(frame.DroppedCpuScopes) + " / GPU 누락 " + std::to_string(frame.DroppedGpuScopes) + " / 상세 " + (frame.DetailedCpuScopes ? "ON" : "OFF") + " / GPU 패스 " + (frame.GpuScopesSupported ? "지원" : "미지원"),
            "동일 상세 수집 조건에서 다시 수집하고 GPU 응답이 완료된 원래 프레임을 확인합니다. 부모·자식과 메인·worker를 합산하지 않습니다.",
            "미계측은 0이 아닙니다. frame 밖 scope 공백·표본 상한·unsupported GPU는 병목이 없다는 증거가 아닙니다.");
        ImGui::EndTable();
    }
    ImGui::TextWrapped("기준을 넘지 않았어도 병목이 없다는 뜻은 아닙니다. 빠른 장면(쿠크 2관문·빙고 등)과 느린 장면의 차이는 '기준 A/B'에서 구조 비교로 조사하며 동일 조건 인과성으로 단정하지 않습니다.");
}

namespace
{
    void MemoryValueCell(const Engine::FProfilerMemoryStats& sample, Client::EProfilerMemoryMetric metric)
    {
        const auto value = Client::ProfilerMemoryValue(sample, metric);
        if (value.Available) ImGui::Text("%.1f", value.MiB); else ImGui::TextDisabled("미계측");
    }

    void MemorySamplePlot(const char* id, const char* title, const char* firstName, const char* secondName,
        const std::vector<Engine::FProfilerMemoryStats>& samples, Engine::CProfiler& profiler,
        Client::EProfilerMemoryMetric firstMetric, Client::EProfilerMemoryMetric secondMetric)
    {
        ImGui::PushID(id); ImGui::TextUnformatted(title);
        if (samples.empty()) { ImGui::TextDisabled("수집된 OS 조회 표본 없음"); ImGui::PopID(); return; }
        const ImU32 firstColor = IM_COL32(91, 177, 244, 255), secondColor = IM_COL32(136, 212, 139, 255);
        ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(firstColor), "%s", firstName); ImGui::SameLine();
        ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(secondColor), "%s", secondName); ImGui::SameLine(); ImGui::TextDisabled("단위 MiB / x 실제 표본 시각 (s)");
        const auto origin = ImGui::GetCursorScreenPos();
        const float width = (std::max)(200.f, ImGui::GetContentRegionAvail().x), height = 100.f;
        const float left = origin.x + 64.f, right = origin.x + width - 8.f, top = origin.y + 7.f, bottom = origin.y + height - 22.f;
        ImGui::InvisibleButton("##MemoryChart", ImVec2(width, height)); const bool hovered = ImGui::IsItemHovered();
        auto* draw = ImGui::GetWindowDrawList();
        draw->AddRectFilled(origin, ImVec2(origin.x + width, origin.y + height), IM_COL32(25, 29, 36, 255));
        const auto firstTick = samples.front().SampleTick, lastTick = samples.back().SampleTick;
        const double seconds = (std::max)(0.001, profiler.Ticks_ToMs(lastTick - firstTick) / 1000.0);
        double maximum = 1.0; bool anyValid = false;
        for (const auto& sample : samples) for (const auto metric : {firstMetric, secondMetric})
        {
            const auto value = Client::ProfilerMemoryValue(sample, metric);
            if (value.Available) { maximum = (std::max)(maximum, value.MiB); anyValid = true; }
        }
        maximum *= 1.05;
        char number[64]; std::snprintf(number, sizeof(number), "%.0f MiB", maximum);
        draw->AddText(ImVec2(origin.x + 2.f, top), IM_COL32(175, 182, 193, 255), number);
        draw->AddText(ImVec2(origin.x + 2.f, bottom - 12.f), IM_COL32(175, 182, 193, 255), "0");
        for (int tick = 0; tick <= 4; ++tick)
        {
            const float x = left + (right - left) * float(tick) / 4.f;
            draw->AddLine(ImVec2(x, top), ImVec2(x, bottom), IM_COL32(55, 60, 72, 255));
            std::snprintf(number, sizeof(number), "%.2f", seconds * double(tick) / 4.0);
            draw->AddText(ImVec2(x, bottom + 3.f), IM_COL32(175, 182, 193, 255), number);
        }
        int nearest = -1; float nearestDistance = 9.f;
        for (int series = 0; series < 2; ++series)
        {
            bool previousValid = false; ImVec2 previous;
            for (size_t i = 0; i < samples.size(); ++i)
            {
                const auto& sample = samples[i]; const auto value = Client::ProfilerMemoryValue(sample, series ? secondMetric : firstMetric);
                const float x = left + float(profiler.Ticks_ToMs(sample.SampleTick - firstTick) / 1000.0 / seconds) * (right - left);
                if (!series && hovered && std::abs(ImGui::GetIO().MousePos.x - x) < nearestDistance)
                { nearest = static_cast<int>(i); nearestDistance = std::abs(ImGui::GetIO().MousePos.x - x); }
                if (!value.Available) { previousValid = false; continue; }
                const ImVec2 point(x, bottom - float(value.MiB / maximum) * (bottom - top));
                if (previousValid) draw->AddLine(previous, point, series ? secondColor : firstColor, 1.5f);
                draw->AddCircleFilled(point, 2.5f, series ? secondColor : firstColor);
                previous = point; previousValid = true;
            }
        }
        if (!anyValid) draw->AddText(ImVec2(left + 8.f, top + 15.f), IM_COL32(200, 180, 150, 255), "N/A: no valid samples");
        if (nearest >= 0)
        {
            const auto& sample = samples[static_cast<size_t>(nearest)];
            ImGui::BeginTooltip();
            ImGui::Text("조회 frame #%llu | PID %u | 현재 기준 %.1f ms 전", static_cast<unsigned long long>(sample.SampleFrameNumber), sample.ProcessId, sample.AgeMs);
            for (int series = 0; series < 2; ++series)
            {
                const auto value = Client::ProfilerMemoryValue(sample, series ? secondMetric : firstMetric);
                if (value.Available) ImGui::Text("%s %.1f MiB", series ? secondName : firstName, value.MiB);
                else ImGui::Text("%s 미계측", series ? secondName : firstName);
            }
            ImGui::EndTooltip();
        }
        ImGui::PopID();
    }
}

void Client::CProfilerTool::Render_Memory(Engine::CProfiler& profiler)
{
    const double now = ImGui::GetTime();
    const bool refresh = ImGui::Button("메모리 표본 다시 읽기###MemoryRefresh");
    if (refresh || m_fLastMemoryRefresh < 0.0 || now - m_fLastMemoryRefresh >= 0.5)
    {
        profiler.Get_MemorySamples(Engine::CProfiler::MAX_HISTORY_FRAMES, m_MemorySamples);
        m_fLastMemoryRefresh = now;
    }
    ImGui::SameLine(); ImGui::TextDisabled("실제 OS 조회 %zu개 / 보관 최대 %zu 프레임", m_MemorySamples.size(), Engine::CProfiler::MAX_HISTORY_FRAMES);
    ImGui::TextWrapped("Capture ON에서 main thread가 약 1Hz로 관측한 실제 조회 표본만 표시합니다. 같은 SampleFrameNumber·SampleTick·PID를 여러 프레임이 재사용해도 점은 하나입니다. 조회 실패는 0으로 연결하지 않고 차트를 끊습니다. 재조회는 보관 표본을 읽을 뿐 OS 샘플 주기를 높이지 않습니다.");
    ImGui::TextWrapped("Private commit은 프로세스 전용 커밋, Working Set은 현재 RAM 상주량(공유 가능 페이지 포함)입니다. DXGI Local/Non-local은 이 프로세스의 adapter segment 사용량과 OS 동적 예산이며 UMA의 Local을 항상 전용 VRAM으로 볼 수 없습니다. RAM·commit·GPU 값을 합산하지 마세요.");
    if (ImGui::CollapsingHeader("메모리 표본의 한계와 다음 실험"))
        ImGui::TextWrapped("main thread stall 동안 표본이 없고 순간 peak를 놓칠 수 있습니다. 선 사이를 실제 연속 사용량으로 단정하지 마세요. 프로세스 lifetime peak는 이 capture의 peak가 아닙니다. 100~250ms 로딩 phase tracker·allocation callstack·resource별 bytes/owner·residency/page fault·해제 수명은 미지원입니다. 같은 프로세스에서 Lobby 안정 → 입장 → 안정 → 퇴장/재진입의 잔존량을 비교하고, 증가만으로 누수/OOM을 확정하지 마세요. 미제출/culling은 자원 메모리 해제를 뜻하지 않습니다.");
    if (m_MemorySamples.empty()) { ImGui::TextDisabled("완료 프레임에 귀속된 OS 조회 표본이 없습니다."); return; }
    const auto& latest = m_MemorySamples.back();
    ImGui::Text("최신 조회 frame #%llu | PID %u | 현재 기준 %.1f ms 전", static_cast<unsigned long long>(latest.SampleFrameNumber), latest.ProcessId, latest.AgeMs);
    if (latest.AdapterIdentityValid) ImGui::Text("Adapter LUID %08x:%08x / node %u", static_cast<unsigned>(latest.AdapterLuidHigh), latest.AdapterLuidLow, latest.AdapterNodeIndex);
    else ImGui::TextDisabled("Adapter identity 미계측");
    if (ImGui::BeginTable("##MemoryLatest", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH))
    {
        const auto row = [&](const char* label, EProfilerMemoryMetric metric)
        { ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextUnformatted(label); ImGui::TableNextColumn(); MemoryValueCell(latest, metric); };
        ImGui::TableSetupColumn("최신 OS 조회 항목"); ImGui::TableSetupColumn("MiB (실패는 미계측)"); ImGui::TableHeadersRow();
        row("프로세스 전용 commit", EProfilerMemoryMetric::PrivateCommit); row("프로세스 현재 Working Set", EProfilerMemoryMetric::WorkingSet);
        row("프로세스 lifetime 최대 전용 commit", EProfilerMemoryMetric::PeakPrivateCommit); row("프로세스 lifetime 최대 Working Set", EProfilerMemoryMetric::PeakWorkingSet);
        row("시스템 전체 commit", EProfilerMemoryMetric::SystemCommit); row("시스템 commit 한도", EProfilerMemoryMetric::SystemLimit); row("시스템 가용 RAM", EProfilerMemoryMetric::SystemAvailable);
        row("DXGI Local 프로세스 사용량", EProfilerMemoryMetric::LocalUsage); row("DXGI Local 동적 예산", EProfilerMemoryMetric::LocalBudget);
        row("DXGI Non-local 프로세스 사용량", EProfilerMemoryMetric::NonLocalUsage); row("DXGI Non-local 동적 예산", EProfilerMemoryMetric::NonLocalBudget);
        ImGui::EndTable();
    }
    if (ImGui::CollapsingHeader("실제 조회 표본의 시간 변화", ImGuiTreeNodeFlags_DefaultOpen))
    {
        MemorySamplePlot("Process", "프로세스 RAM·commit", "전용 commit", "Working Set", m_MemorySamples, profiler, EProfilerMemoryMetric::PrivateCommit, EProfilerMemoryMetric::WorkingSet);
        MemorySamplePlot("Local", "DXGI Local segment", "프로세스 사용량", "동적 예산", m_MemorySamples, profiler, EProfilerMemoryMetric::LocalUsage, EProfilerMemoryMetric::LocalBudget);
        MemorySamplePlot("NonLocal", "DXGI Non-local segment", "프로세스 사용량", "동적 예산", m_MemorySamples, profiler, EProfilerMemoryMetric::NonLocalUsage, EProfilerMemoryMetric::NonLocalBudget);
        MemorySamplePlot("System", "시스템 commit", "전체 commit", "commit 한도", m_MemorySamples, profiler, EProfilerMemoryMetric::SystemCommit, EProfilerMemoryMetric::SystemLimit);
    }
    if (ImGui::CollapsingHeader("표본별 값·실패 확인"))
        if (ImGui::BeginTable("##MemorySampleRows", 8, TABLE_FLAGS | ImGuiTableFlags_ScrollX, ImVec2(0.f, 220.f), 1250.f))
        {
            for (const char* label : {"OS 조회 frame", "현재 나이 ms", "private MiB", "WS MiB", "Local 사용 MiB", "Local 예산 MiB", "시스템 commit MiB", "시스템 가용 MiB"}) ImGui::TableSetupColumn(label);
            ImGui::TableSetupScrollFreeze(1, 1); ImGui::TableHeadersRow();
            ImGuiListClipper clipper; clipper.Begin(static_cast<int>(m_MemorySamples.size()));
            while (clipper.Step()) for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
            {
                const auto& sample = m_MemorySamples[static_cast<size_t>(i)];
                ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::Text("%llu", static_cast<unsigned long long>(sample.SampleFrameNumber));
                ImGui::TableNextColumn(); ImGui::Text("%.1f", sample.AgeMs);
                for (const auto metric : {EProfilerMemoryMetric::PrivateCommit, EProfilerMemoryMetric::WorkingSet, EProfilerMemoryMetric::LocalUsage,
                    EProfilerMemoryMetric::LocalBudget, EProfilerMemoryMetric::SystemCommit, EProfilerMemoryMetric::SystemAvailable})
                { ImGui::TableNextColumn(); MemoryValueCell(sample, metric); }
            }
            ImGui::EndTable();
        }
}
