#include "MapAssetRenderUtils.h"

#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

using namespace Client;
using namespace Engine;

namespace
{
    // Frozen numeric inputs observed in the Bern placement/spawn documents on
    // 2026-08-28. These are arithmetic fixtures, not a runtime map/mesh smoke.
    constexpr float BernFar = 13084.80859375f;
    const float3_t BernSpawn{ 137.586334f, 42.2498169f, -22.4640217f };
    const float3_t BernEye{ 137.986328125f, 49.74981689453125f, -17.964021682739258f };
    const float3_t CancellationCenter{
        138.1667938232422f, 41.78025817871094f, -17.2905330657959f };

    struct ContractAssertions final
    {
        size_t count = {};
        size_t failures = {};

        void Expect(const bool condition, const std::string& label)
        {
            ++count;
            if (!condition)
            {
                ++failures;
                std::cerr << "FAIL " << label << '\n';
            }
        }
    };

    struct CameraPose final
    {
        float3_t eye;
        float3_t direction;
    };

    template <typename T>
    std::array<unsigned char, sizeof(T)> ObjectBytes(const T& value)
    {
        static_assert(std::is_trivially_copyable_v<T>);
        std::array<unsigned char, sizeof(T)> bytes{};
        std::memcpy(bytes.data(), &value, sizeof(value));
        return bytes;
    }

    float4x4_t IdentityMatrix()
    {
        float4x4_t result{};
        XMStoreFloat4x4(&result, XMMatrixIdentity());
        return result;
    }

    float4x4_t Projection(const float farPlane = BernFar)
    {
        float4x4_t result{};
        XMStoreFloat4x4(&result, XMMatrixPerspectiveFovLH(
            XMConvertToRadians(60.f), 1280.f / 720.f, 0.1f, farPlane));
        return result;
    }

    float4x4_t View(const CameraPose& pose)
    {
        const vector_t look = XMLoadFloat3(&pose.direction);
        const vector_t right = XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), look);
        const vector_t up = XMVector3Cross(look, right);
        matrix_t world;
        world.r[0] = XMVector3Normalize(right);
        world.r[1] = XMVector3Normalize(up);
        world.r[2] = XMVector3Normalize(look);
        world.r[3] = XMVectorSet(pose.eye.x, pose.eye.y, pose.eye.z, 1.f);
        float4x4_t result{};
        XMStoreFloat4x4(&result, XMMatrixInverse(nullptr, world));
        return result;
    }

    float3_t AxisCenter(const CameraPose& pose)
    {
        float3_t result{};
        XMStoreFloat3(&result, XMVectorMultiplyAdd(
            XMVector3Normalize(XMLoadFloat3(&pose.direction)),
            XMVectorReplicate(8.f), XMLoadFloat3(&pose.eye)));
        return result;
    }

    float3_t Direction(const double pitchDegrees, const double yawDegrees)
    {
        constexpr double Pi = 3.1415926535897932384626433832795;
        const double pitch = pitchDegrees * Pi / 180.0;
        const double yaw = yawDegrees * Pi / 180.0;
        return float3_t(
            static_cast<float>(std::cos(pitch) * std::sin(yaw)),
            static_cast<float>(std::sin(pitch)),
            static_cast<float>(std::cos(pitch) * std::cos(yaw)));
    }

    CameraPose FollowPose(const float3_t& player)
    {
        const float3_t eye(player.x + 0.4f, player.y + 7.5f, player.z + 4.5f);
        const float3_t at(player.x, player.y + 1.2f, player.z);
        // Retain the actual float(at-eye) quantization used by LookAt.
        return { eye, float3_t(at.x - eye.x, at.y - eye.y, at.z - eye.z) };
    }

    MAP_FRUSTUM_CULLING_POLICY BernPolicy()
    {
        MAP_FRUSTUM_CULLING_POLICY policy{};
        policy.baseMargin = 0.25f;
        policy.largeObjectRadiusThreshold = 4.f;
        policy.largeObjectAbsoluteMargin = 2.f;
        policy.largeObjectRelativeMargin = 0.12f;
        policy.rejectHysteresisFrames = 3u;
        return policy;
    }

    MAP_CAMERA_CULL_SNAPSHOT Build(
        const float4x4_t& view, const float4x4_t& projection,
        const uint64_t revision = 1u)
    {
        MAP_CAMERA_CULL_SNAPSHOT result{};
        std::string reason;
        if (!CMapAssetRenderUtils::Build_CameraCullSnapshot(
            view, projection, revision, result, &reason))
        {
            throw std::runtime_error("Valid camera fixture rejected: " + reason);
        }
        return result;
    }

    MAP_FRUSTUM_CULL_DECISION Evaluate(
        const MAP_CAMERA_CULL_SNAPSHOT& snapshot, const float3_t& center,
        const float radius, const MAP_FRUSTUM_CULLING_POLICY& policy,
        MAP_FRUSTUM_RUNTIME_STATE& state, const std::string& group = "staticmesh")
    {
        MAP_FRUSTUM_CULL_DECISION result{};
        std::string reason;
        if (!CMapAssetRenderUtils::Evaluate_FrustumVisibility(
            policy, snapshot, "fixture.numeric-sphere", group, 1u,
            center, radius, state, result, &reason))
        {
            throw std::runtime_error("Valid sphere fixture rejected: " + reason);
        }
        return result;
    }

    bool Visible(
        const MAP_CAMERA_CULL_SNAPSHOT& snapshot, const float3_t& center,
        const float radius, const MAP_FRUSTUM_CULLING_POLICY& policy = {})
    {
        MAP_FRUSTUM_RUNTIME_STATE fresh{};
        return Evaluate(snapshot, center, radius, policy, fresh).wouldBeVisible;
    }

    // Independent oracle: transform a world point by View and then Projection
    // in double, and apply the six homogeneous clip inequalities directly.
    // It does not read production planes or copy the builder's plane extraction.
    // This is an ideal reference for the same float inputs, not a GPU execution.
    bool CenterInsideClip(
        const float4x4_t& view, const float4x4_t& projection, const float3_t& center)
    {
        const std::array<double, 4> world{ center.x, center.y, center.z, 1.0 };
        std::array<double, 4> camera{};
        std::array<double, 4> clip{};
        for (size_t column = 0u; column < 4u; ++column)
        {
            for (size_t row = 0u; row < 4u; ++row)
                camera[column] += world[row] * static_cast<double>(view.m[row][column]);
        }
        for (size_t column = 0u; column < 4u; ++column)
        {
            for (size_t row = 0u; row < 4u; ++row)
                clip[column] += camera[row] * static_cast<double>(projection.m[row][column]);
        }
        return std::isfinite(clip[0]) && std::isfinite(clip[1]) &&
            std::isfinite(clip[2]) && std::isfinite(clip[3]) && clip[3] > 0.0 &&
            clip[0] > -clip[3] && clip[0] < clip[3] &&
            clip[1] > -clip[3] && clip[1] < clip[3] &&
            clip[2] > 0.0 && clip[2] < clip[3];
    }

    // Negative control only: reproduce the removed inverse-corner expression.
    // No production call uses these planes, and passing this comparison alone
    // would not establish that a real map placement has correct model bounds.
    std::array<float4_t, 6> LegacyCornerPlanes(
        const float4x4_t& view, const float4x4_t& projection)
    {
        const matrix_t inverseProjection = XMMatrixInverse(nullptr, XMLoadFloat4x4(&projection));
        const matrix_t inverseView = XMMatrixInverse(nullptr, XMLoadFloat4x4(&view));
        const float3_t ndc[8] = {
            { -1.f, 1.f, 0.f }, { 1.f, 1.f, 0.f }, { 1.f, -1.f, 0.f }, { -1.f, -1.f, 0.f },
            { -1.f, 1.f, 1.f }, { 1.f, 1.f, 1.f }, { 1.f, -1.f, 1.f }, { -1.f, -1.f, 1.f }
        };
        float3_t points[8]{};
        for (size_t index = 0u; index < 8u; ++index)
        {
            const vector_t camera = XMVector3TransformCoord(XMLoadFloat3(&ndc[index]), inverseProjection);
            XMStoreFloat3(&points[index], XMVector3TransformCoord(camera, inverseView));
        }
        const size_t triples[6][3] = {
            { 1u, 5u, 6u }, { 4u, 0u, 3u }, { 4u, 5u, 1u },
            { 3u, 2u, 6u }, { 5u, 4u, 7u }, { 0u, 1u, 2u }
        };
        std::array<float4_t, 6> planes{};
        for (size_t index = 0u; index < planes.size(); ++index)
        {
            XMStoreFloat4(&planes[index], XMPlaneFromPoints(
                XMLoadFloat3(&points[triples[index][0]]),
                XMLoadFloat3(&points[triples[index][1]]),
                XMLoadFloat3(&points[triples[index][2]])));
        }
        return planes;
    }

    float PlaneDistance(const float4_t& plane, const float3_t& center)
    {
        return XMVectorGetX(XMPlaneDotCoord(XMLoadFloat4(&plane), XMLoadFloat3(&center)));
    }

    bool LegacyRejects(
        const std::array<float4_t, 6>& planes, const float3_t& center,
        const float effectiveRadius)
    {
        for (const float4_t& plane : planes)
        {
            if (PlaneDistance(plane, center) >= effectiveRadius)
                return true;
        }
        return false;
    }

    void TestSixPlanesAndTangency(ContractAssertions& assertions)
    {
        const auto snapshot = Build(IdentityMatrix(), IdentityMatrix());
        const std::array<float4_t, 6> expected{ {
            { 1.f, 0.f, 0.f, -1.f }, { -1.f, 0.f, 0.f, -1.f },
            { 0.f, 1.f, 0.f, -1.f }, { 0.f, -1.f, 0.f, -1.f },
            { 0.f, 0.f, 1.f, -1.f }, { 0.f, 0.f, -1.f, 0.f }
        } };
        const std::array<float3_t, 6> boundary{ {
            { 1.f, 0.f, 0.5f }, { -1.f, 0.f, 0.5f },
            { 0.f, 1.f, 0.5f }, { 0.f, -1.f, 0.5f },
            { 0.f, 0.f, 1.f }, { 0.f, 0.f, 0.f }
        } };
        assertions.Expect(Visible(snapshot, float3_t(0.f, 0.f, 0.5f), 0.1f), "identity clip interior");
        for (size_t index = 0u; index < expected.size(); ++index)
        {
            const auto& n = expected[index];
            const auto& p = snapshot.worldPlanes[index];
            assertions.Expect(p.x == n.x && p.y == n.y && p.z == n.z && p.w == n.w,
                "outward plane orientation " + std::to_string(index));
            const auto& b = boundary[index];
            const float3_t tangent(b.x + n.x, b.y + n.y, b.z + n.z);
            MAP_FRUSTUM_RUNTIME_STATE state{};
            const auto decision = Evaluate(snapshot, tangent, 1.f, {}, state);
            assertions.Expect(decision.wouldBeVisible && decision.shouldRender &&
                decision.rejectingPlane == -1, "tangent is retained " + std::to_string(index));
            assertions.Expect(std::isfinite(decision.planeTolerances[index]) &&
                decision.planeTolerances[index] >= 0.f && decision.planeTolerances[index] < 0.001f,
                "tangent tolerance is bounded " + std::to_string(index));
            const float3_t outside(b.x + n.x * 1.01f, b.y + n.y * 1.01f, b.z + n.z * 1.01f);
            state = {};
            const auto rejected = Evaluate(snapshot, outside, 1.f, {}, state);
            assertions.Expect(!rejected.wouldBeVisible && !rejected.shouldRender &&
                rejected.rejectingPlane == static_cast<int32_t>(index),
                "outside beyond tolerance " + std::to_string(index));
        }
    }

    void TestDepthClipping(ContractAssertions& assertions)
    {
        for (const float farPlane : { 2000.f, BernFar, 40000.f })
        {
            const auto projection = Projection(farPlane);
            const auto snapshot = Build(IdentityMatrix(), projection);
            const double clipNear = -static_cast<double>(projection._43) / projection._33;
            const double clipFar = -static_cast<double>(projection._43) /
                (static_cast<double>(projection._33) - 1.0);
            assertions.Expect(Visible(snapshot, float3_t(0.f, 0.f, 8.f), 0.5f), "perspective interior");
            assertions.Expect(!Visible(snapshot, float3_t(0.f, 0.f, -2.f), 0.1f), "behind camera");
            assertions.Expect(!Visible(snapshot, float3_t(0.f, 0.f, 0.f), 0.01f),
                "camera-containing sphere wholly before near is clipped");
            assertions.Expect(Visible(snapshot, float3_t(0.f, 0.f, 0.f), 0.2f),
                "camera-containing sphere crossing near is retained");
            assertions.Expect(Visible(snapshot, float3_t(0.f, 0.f, 0.f), 500.f),
                "large camera-containing sphere retained");
            assertions.Expect(Visible(snapshot, float3_t(0.f, 0.f, static_cast<float>(clipNear - 0.5)), 0.5f),
                "perspective near tangent");
            assertions.Expect(Visible(snapshot, float3_t(0.f, 0.f, static_cast<float>(clipFar + 0.5)), 0.5f),
                "perspective far tangent uses quantized projection");
            assertions.Expect(!Visible(snapshot, float3_t(0.f, 0.f, static_cast<float>(clipFar + 10.0)), 0.5f),
                "beyond actual far plane");
        }
    }

    void TestSnapshotFailureRollback(ContractAssertions& assertions)
    {
        const auto identity = IdentityMatrix();
        auto output = Build(identity, identity, 77u);
        auto expectFailure = [&](const float4x4_t& view, const float4x4_t& projection,
                                 const uint64_t revision, const std::string& label)
        {
            const auto before = ObjectBytes(output);
            std::string reason;
            const bool ok = CMapAssetRenderUtils::Build_CameraCullSnapshot(
                view, projection, revision, output, &reason);
            assertions.Expect(!ok && !reason.empty(), "snapshot rejection reason: " + label);
            assertions.Expect(before == ObjectBytes(output), "snapshot output rollback: " + label);
        };
        expectFailure(identity, identity, 0u, "zero revision");
        expectFailure(float4x4_t{}, identity, 1u, "singular view");
        expectFailure(identity, float4x4_t{}, 1u, "singular projection");
        auto duplicateRow = identity;
        for (size_t column = 0u; column < 4u; ++column)
            duplicateRow.m[1][column] = duplicateRow.m[0][column];
        expectFailure(duplicateRow, identity, 1u, "dependent view rows");
        auto degenerateTop = identity;
        degenerateTop._24 = 1.f;
        expectFailure(identity, degenerateTop, 1u, "top plane degenerates after two staged planes");
        auto bad = identity;
        bad._21 = std::numeric_limits<float>::quiet_NaN();
        expectFailure(bad, identity, 1u, "NaN view");
        bad = identity;
        bad._43 = std::numeric_limits<float>::infinity();
        expectFailure(identity, bad, 1u, "infinite projection");
    }

    void TestDecisionFailureRollback(ContractAssertions& assertions)
    {
        const auto snapshot = Build(IdentityMatrix(), IdentityMatrix());
        const auto policy = BernPolicy();
        const float3_t center(0.f, 0.f, 0.5f);
        MAP_FRUSTUM_RUNTIME_STATE state{};
        auto decision = Evaluate(snapshot, center, 0.5f, policy, state);
        auto expectFailure = [&](const MAP_CAMERA_CULL_SNAPSHOT& camera,
            const MAP_FRUSTUM_CULLING_POLICY& candidatePolicy, const float3_t& sphereCenter,
            const float radius, const std::string& label)
        {
            const auto stateBefore = ObjectBytes(state);
            const auto outputBefore = ObjectBytes(decision);
            std::string reason;
            const bool ok = CMapAssetRenderUtils::Evaluate_FrustumVisibility(
                candidatePolicy, camera, "fixture.invalid", "staticmesh", 1u,
                sphereCenter, radius, state, decision, &reason);
            assertions.Expect(!ok && !reason.empty(), "decision rejection reason: " + label);
            assertions.Expect(stateBefore == ObjectBytes(state) && outputBefore == ObjectBytes(decision),
                "decision/state rollback: " + label);
        };
        expectFailure(snapshot, policy, center, 0.f, "zero radius");
        expectFailure(snapshot, policy, center, -1.f, "negative radius");
        expectFailure(snapshot, policy, center, std::numeric_limits<float>::quiet_NaN(), "NaN radius");
        expectFailure(snapshot, policy, center, std::numeric_limits<float>::infinity(), "infinite radius");
        expectFailure(snapshot, policy, float3_t(std::numeric_limits<float>::quiet_NaN(), 0.f, 0.f),
            1.f, "NaN center");
        expectFailure(snapshot, policy, float3_t(0.f, std::numeric_limits<float>::infinity(), 0.f),
            1.f, "infinite center");
        auto camera = snapshot;
        camera.revision = 0u;
        expectFailure(camera, policy, center, 1.f, "zero snapshot revision");
        camera = snapshot;
        camera.worldPlanes[1] = float4_t(0.f, 0.f, 0.f, 1.f);
        expectFailure(camera, policy, center, 1.f, "zero plane normal");
        camera = snapshot;
        camera.worldPlanes[2].y = 2.f;
        expectFailure(camera, policy, center, 1.f, "non-unit plane normal");
        camera = snapshot;
        camera.worldPlanes[0] = float4_t(1.0004f, 0.f, 0.f, -1.0004f);
        expectFailure(camera, {}, float3_t(2.f, 0.f, 0.5f), 1.f,
            "slightly scaled plane cannot reject a tangent sphere");
        camera = snapshot;
        camera.worldPlanes[3].w = std::numeric_limits<float>::quiet_NaN();
        expectFailure(camera, policy, center, 1.f, "NaN plane");
        camera = snapshot;
        camera.worldPlanes[4].w = std::numeric_limits<float>::infinity();
        expectFailure(camera, policy, center, 1.f, "infinite plane");
        auto invalidPolicy = policy;
        invalidPolicy.baseMargin = -1.f;
        expectFailure(snapshot, invalidPolicy, center, 1.f, "negative base margin");
        invalidPolicy = policy;
        invalidPolicy.baseMargin = std::numeric_limits<float>::quiet_NaN();
        expectFailure(snapshot, invalidPolicy, center, 1.f, "NaN base margin");
        invalidPolicy = policy;
        invalidPolicy.largeObjectRadiusThreshold = -1.f;
        expectFailure(snapshot, invalidPolicy, center, 1.f, "negative large threshold");
        invalidPolicy = policy;
        invalidPolicy.largeObjectAbsoluteMargin = -1.f;
        expectFailure(snapshot, invalidPolicy, center, 1.f, "negative large margin");
        invalidPolicy = policy;
        invalidPolicy.largeObjectRelativeMargin = std::numeric_limits<float>::infinity();
        expectFailure(snapshot, invalidPolicy, center, 1.f, "infinite relative margin");
        invalidPolicy = policy;
        invalidPolicy.baseMargin = (std::numeric_limits<float>::max)();
        expectFailure(snapshot, invalidPolicy, center, (std::numeric_limits<float>::max)(), "radius margin overflow");
    }

    void TestPolicyAndGrace(ContractAssertions& assertions)
    {
        const auto snapshot = Build(IdentityMatrix(), IdentityMatrix());
        const auto policy = BernPolicy();
        const float3_t interior(0.f, 0.f, 0.5f);
        const float3_t exterior(4.f, 0.f, 0.5f);
        MAP_FRUSTUM_RUNTIME_STATE fresh{};
        const auto first = Evaluate(snapshot, exterior, 0.5f, policy, fresh);
        assertions.Expect(!first.wouldBeVisible && !first.shouldRender && fresh.rejectGraceFrames == 0u,
            "fresh state rejects immediately");
        MAP_FRUSTUM_RUNTIME_STATE warm{};
        const auto visible = Evaluate(snapshot, interior, 0.5f, policy, warm);
        assertions.Expect(visible.wouldBeVisible && warm.rejectGraceFrames == 3u, "visible state primes grace");
        for (uint32_t rejection = 1u; rejection <= 4u; ++rejection)
        {
            const auto result = Evaluate(snapshot, exterior, 0.5f, policy, warm);
            const uint32_t expectedGrace = rejection < 3u ? 3u - rejection : 0u;
            assertions.Expect(!result.wouldBeVisible && result.shouldRender == (rejection <= 3u) &&
                warm.rejectGraceFrames == expectedGrace,
                "same raw reject, warm grace call " + std::to_string(rejection));
        }
        const auto returned = Evaluate(snapshot, interior, 0.5f, policy, warm);
        assertions.Expect(returned.shouldRender && warm.rejectGraceFrames == 3u, "visible return replenishes grace");
        auto bypass = policy;
        bypass.bypass = true;
        fresh = {};
        const auto bypassed = Evaluate(snapshot, exterior, 0.5f, bypass, fresh);
        assertions.Expect(!bypassed.wouldBeVisible && bypassed.shouldRender, "bypass retains raw decision");
        fresh = {};
        const auto landscape = Evaluate(snapshot, interior, 0.5f, policy, fresh, "landscape");
        assertions.Expect(landscape.largeGeometry && landscape.margin == 2.f &&
            landscape.effectiveRadius == 2.5f, "landscape group margin");
        fresh = {};
        const auto large = Evaluate(snapshot, interior, 50.f, policy, fresh);
        assertions.Expect(large.largeGeometry && std::abs(large.margin - 6.f) < 0.00001f,
            "large radius relative margin");
        std::cout << "grace=fresh_reject_immediate warm_render_calls_1_2_3_reject_4\n";
    }

    void TestFrozenCancellationAndDeterminism(ContractAssertions& assertions)
    {
        const float4x4_t view(
            0.9659257531166077f, 0.25783416628837585f, 0.022557567805051804f, -0.0f,
            0.f, 0.08715573698282242f, -0.9961946606636047f, 0.f,
            -0.258819043636322f, 0.9622501134872437f, 0.08418598026037216f, 0.f,
            -137.93399047851562f, -22.627695083618164f, 47.960182189941406f, 1.f);
        const float4x4_t projection(
            0.9742785692214966f, 0.f, 0.f, 0.f,
            0.f, 1.7320507764816284f, 0.f, 0.f,
            0.f, 0.f, 1.0000076293945312f, 1.f,
            0.f, 0.f, -0.100000761449337f, 0.f);
        const auto oldPlanes = LegacyCornerPlanes(view, projection);
        const auto snapshot = Build(view, projection, 99u);
        assertions.Expect(CenterInsideClip(view, projection, CancellationCenter), "frozen center strictly inside clip");
        assertions.Expect(LegacyRejects(oldPlanes, CancellationCenter, 0.75f), "legacy negative control reproduces cancellation");
        assertions.Expect(Visible(snapshot, CancellationCenter, 0.5f, BernPolicy()), "production keeps frozen cancellation fixture");
        assertions.Expect(ObjectBytes(snapshot.view) == ObjectBytes(view) &&
            ObjectBytes(snapshot.projection) == ObjectBytes(projection), "shader matrix inputs preserved exactly");
        const auto expectedPlanes = ObjectBytes(snapshot.worldPlanes);
        bool deterministic = true;
        for (uint32_t repetition = 0u; repetition < 100u; ++repetition)
        {
            const auto repeated = Build(view, projection, 99u);
            deterministic = deterministic && expectedPlanes == ObjectBytes(repeated.worldPlanes) &&
                ObjectBytes(oldPlanes) == ObjectBytes(LegacyCornerPlanes(view, projection));
            assertions.Expect(Visible(repeated, CancellationCenter, 0.5f, BernPolicy()), "identical input remains visible");
        }
        assertions.Expect(deterministic, "100 identical matrix calls are deterministic");
        std::cout << "frozen_legacy_left_distance=" << PlaneDistance(oldPlanes[1], CancellationCenter)
            << " production_left_distance=" << PlaneDistance(snapshot.worldPlanes[1], CancellationCenter)
            << " repeated_identical_calls=100 deterministic=" << deterministic << '\n';
    }

    void TestBernPoseSweep(ContractAssertions& assertions)
    {
        std::vector<CameraPose> poses;
        poses.reserve(773u);
        for (const float3_t offset : {
            float3_t(0.f, 0.f, 0.f), float3_t(0.001f, 0.f, 0.f),
            float3_t(10.f, 0.f, 10.f), float3_t(-100.f, 0.f, 0.f), float3_t(-1000.f, 0.f, 0.f) })
        {
            poses.push_back(FollowPose(float3_t(
                BernSpawn.x + offset.x, BernSpawn.y + offset.y, BernSpawn.z + offset.z)));
        }
        const std::array<float3_t, 4> eyes{ {
            BernEye, { -1337.60984f, 80.f, -278.808809f },
            { 297.991191f, 100.f, 476.270273f }, { 0.f, 5.f, 0.f }
        } };
        for (const auto& eye : eyes)
        {
            for (const double pitch : { -85.0, -70.0, -45.0, -15.0, 0.0, 15.0, 45.0, 80.0 })
            {
                for (int yaw = 0; yaw < 360; yaw += 15)
                    poses.push_back({ eye, Direction(pitch, static_cast<double>(yaw)) });
            }
        }
        const auto projection = Projection();
        size_t referenceInside = {};
        size_t oldRejects = {};
        size_t productionRejects = {};
        for (size_t index = 0u; index < poses.size(); ++index)
        {
            const auto view = View(poses[index]);
            const auto center = AxisCenter(poses[index]);
            const auto snapshot = Build(view, projection, index + 1u);
            const bool inside = CenterInsideClip(view, projection, center);
            referenceInside += inside ? 1u : 0u;
            oldRejects += inside && LegacyRejects(LegacyCornerPlanes(view, projection), center, 0.75f) ? 1u : 0u;
            const bool visible = Visible(snapshot, center, 0.5f, BernPolicy());
            productionRejects += inside && !visible ? 1u : 0u;
            assertions.Expect(inside && visible, "Bern optical axis pose " + std::to_string(index));
        }
        assertions.Expect(poses.size() == 773u && referenceInside == poses.size(), "773 valid Bern poses");
        assertions.Expect(oldRejects > 0u && productionRejects == 0u, "pose sweep separates legacy and production");
        std::cout << "bern_pose_count=" << poses.size() << " reference_centers_inside=" << referenceInside
            << " legacy_false_rejects=" << oldRejects << " production_false_rejects=" << productionRejects << '\n';
    }

    void TestSmallPerturbations(ContractAssertions& assertions)
    {
        const auto projection = Projection();
        const std::array<double, 9> deltas{
            -0.01, -0.001, -0.0001, -0.00001, 0.0, 0.00001, 0.0001, 0.001, 0.01 };
        size_t tested = {};
        size_t legacyFlipGroups = {};
        size_t legacyRejectCount = {};
        size_t referenceInsideCount = {};
        size_t productionRejectCount = {};
        auto runGroup = [&](const std::vector<CameraPose>& poses, const float3_t& fixedCenter)
        {
            bool oldKept = false;
            bool oldRejected = false;
            for (const auto& pose : poses)
            {
                ++tested;
                const auto view = View(pose);
                const auto snapshot = Build(view, projection, tested);
                const bool inside = CenterInsideClip(view, projection, fixedCenter);
                const bool visible = Visible(snapshot, fixedCenter, 0.5f, BernPolicy());
                referenceInsideCount += inside ? 1u : 0u;
                productionRejectCount += inside && !visible ? 1u : 0u;
                const bool oldReject = LegacyRejects(LegacyCornerPlanes(view, projection), fixedCenter, 0.75f);
                legacyRejectCount += oldReject ? 1u : 0u;
                oldKept = oldKept || !oldReject;
                oldRejected = oldRejected || oldReject;
                assertions.Expect(inside && visible,
                    "small perturbation keeps fixed interior sphere " + std::to_string(tested));
            }
            legacyFlipGroups += oldKept && oldRejected ? 1u : 0u;
        };
        for (const bool changePitch : { false, true })
        {
            std::vector<CameraPose> poses;
            for (const double delta : deltas)
                poses.push_back({ BernEye, Direction(-85.0 + (changePitch ? delta : 0.0),
                    15.0 + (changePitch ? 0.0 : delta)) });
            runGroup(poses, CancellationCenter);
        }
        const auto freeDirection = Direction(-85.0, 15.0);
        const auto followCenter = AxisCenter(FollowPose(BernSpawn));
        for (size_t axis = 0u; axis < 3u; ++axis)
        {
            std::vector<CameraPose> translated;
            std::vector<CameraPose> followed;
            for (const double delta : deltas)
            {
                auto eye = BernEye;
                auto player = BernSpawn;
                float* eyeCoordinates = &eye.x;
                float* playerCoordinates = &player.x;
                eyeCoordinates[axis] = static_cast<float>(static_cast<double>(eyeCoordinates[axis]) + delta);
                playerCoordinates[axis] = static_cast<float>(static_cast<double>(playerCoordinates[axis]) + delta);
                translated.push_back({ eye, freeDirection });
                followed.push_back(FollowPose(player));
            }
            runGroup(translated, CancellationCenter);
            runGroup(followed, followCenter);
        }
        assertions.Expect(legacyFlipGroups > 0u, "tiny angle changes reproduce legacy visibility flips");
        std::cout << "fixed_near=0.1 fixed_far=" << BernFar << " perturbation_cases=" << tested
            << " reference_centers_inside=" << referenceInsideCount
            << " legacy_rejects=" << legacyRejectCount << " legacy_flip_groups=" << legacyFlipGroups
            << " production_false_rejects=" << productionRejectCount << '\n';
    }

    void TestBernFollowGrid(ContractAssertions& assertions)
    {
        const auto projection = Projection();
        const CameraPose frozenFollow{
            { 37.98633575439453f, 49.74981689453125f, -117.96401977539062f },
            { -0.40000152587890625f, -6.299999237060547f, -4.5f }
        };
        const float3_t frozenCenter{
            37.57355880737305f, 43.248619079589844f, -122.60773468017578f };
        const auto frozenView = View(frozenFollow);
        const auto frozen = Build(frozenView, projection);
        assertions.Expect(CenterInsideClip(frozenView, projection, frozenCenter) &&
            LegacyRejects(LegacyCornerPlanes(frozenView, projection), frozenCenter, 0.75f) &&
            Visible(frozen, frozenCenter, 0.5f, BernPolicy()), "quantized at-eye follow cancellation fixture");

        // A camera arithmetic grid, not a claim that these player coordinates
        // are navigable or have been visited by a running Server/Client.
        std::vector<CameraPose> poses;
        poses.reserve(1689u);
        for (int x = -100; x <= 100; x += 5)
        {
            for (int z = -100; z <= 100; z += 5)
            {
                poses.push_back(FollowPose(float3_t(BernSpawn.x + static_cast<float>(x),
                    BernSpawn.y, BernSpawn.z + static_cast<float>(z))));
            }
        }
        for (const float y : { -10.f, -1.f, -0.1f, -0.01f, 0.01f, 0.1f, 1.f, 10.f })
            poses.push_back(FollowPose(float3_t(BernSpawn.x, BernSpawn.y + y, BernSpawn.z)));
        size_t referenceInside = {};
        size_t oldRejects = {};
        size_t productionRejects = {};
        for (size_t index = 0u; index < poses.size(); ++index)
        {
            const auto view = View(poses[index]);
            const auto center = AxisCenter(poses[index]);
            const auto snapshot = Build(view, projection, index + 1u);
            const bool inside = CenterInsideClip(view, projection, center);
            const bool visible = Visible(snapshot, center, 0.5f, BernPolicy());
            referenceInside += inside ? 1u : 0u;
            oldRejects += inside && LegacyRejects(LegacyCornerPlanes(view, projection), center, 0.75f) ? 1u : 0u;
            productionRejects += inside && !visible ? 1u : 0u;
            assertions.Expect(inside && visible, "quantized Bern follow grid " + std::to_string(index));
        }
        assertions.Expect(poses.size() == 1689u && referenceInside == poses.size() &&
            oldRejects > 0u && productionRejects == 0u, "follow grid separates legacy and production");
        std::cout << "bern_follow_grid_count=" << poses.size() << " reference_centers_inside=" << referenceInside
            << " legacy_false_rejects=" << oldRejects << " production_false_rejects=" << productionRejects << '\n';
    }
}

int main(const int argumentCount, char*[])
{
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);
    Engine::Set_NonInteractiveErrorMode(true);
#ifdef _DEBUG
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
#endif
    if (argumentCount != 1)
    {
        std::cerr << "MapFrustumContractHarness takes no arguments.\n";
        return 2;
    }
    ContractAssertions assertions;
    std::cout << std::setprecision(9);
    try
    {
        TestSixPlanesAndTangency(assertions);
        TestDepthClipping(assertions);
        TestSnapshotFailureRollback(assertions);
        TestDecisionFailureRollback(assertions);
        TestPolicyAndGrace(assertions);
        TestFrozenCancellationAndDeterminism(assertions);
        TestBernPoseSweep(assertions);
        TestSmallPerturbations(assertions);
        TestBernFollowGrid(assertions);
    }
    catch (const std::exception& error)
    {
        assertions.Expect(false, std::string("exception: ") + error.what());
    }
    std::cout << "MapFrustumContractHarness assertions=" << assertions.count
        << " failures=" << assertions.failures
        << " production_scope=Build_CameraCullSnapshot,Evaluate_FrustumVisibility"
        << " client_ui_or_mesh_buffer_smoke=not_run\n";
    return assertions.failures == 0u ? 0 : 1;
}
