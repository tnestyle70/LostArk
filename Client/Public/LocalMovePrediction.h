#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace Client
{
	// Owns only the local presentation's command acknowledgement and correction.
	// Character's existing navigation/path follower supplies unacknowledged poses;
	// Server snapshots remain the position and movement authority.
	class CLocalMovePrediction final
	{
	public:
		struct Vec3 final
		{
			float x = 0.f, y = 0.f, z = 0.f;
		};
		struct Pose final
		{
			Vec3 position{};
			float yawDegrees = 0.f;
			bool isMoving = false;
		};
		struct Snapshot final
		{
			std::uint32_t serverTick = 0u;
			std::uint32_t processedMoveSequence = 0u;
			Vec3 position{};
			float yawDegrees = 0.f;
			float moveSpeed = 0.f;
			bool canPredictMove = false;
			bool hasMoveGoal = false;
			Vec3 nextWaypoint{};
		};
		enum class SnapshotDisposition
		{
			IGNORED,
			PRESERVE_LOCAL_PATH,
			RECONCILE,
			RESET
		};
		struct FrameResult final
		{
			Pose pose{};
			bool useLocalPath = false;
			bool stopLocalPath = false;
			// True means pose is valid, including an authoritative locked pose.
			bool active = false;
		};

		static constexpr double TIMEOUT_SECONDS = 0.35;
		static constexpr double MAX_EXTRAPOLATION_SECONDS = 0.15;
		static constexpr double MAX_HALF_ROUND_TRIP_SECONDS = 0.1;
		static constexpr double CORRECTION_SECONDS = 0.08;
		static constexpr double MAX_SMOOTH_CORRECTION_DISTANCE = 10.0;

		void Reset() { *this = CLocalMovePrediction{}; }
		bool HasPendingMove() const { return m_hasPendingMove; }
		float Get_MoveSpeed() const { return m_hasSnapshot ? m_snapshot.moveSpeed : 0.f; }

		bool Can_SubmitMove(const double now) const
		{
			return m_hasSnapshot && ValidTime(now) && now >= m_receivedAt &&
				now - m_receivedAt < TIMEOUT_SECONDS &&
				m_snapshot.canPredictMove && m_snapshot.moveSpeed > 0.f;
		}

		// Call only after sending the intent and staging a valid local path. A
		// failed stage/submit must retain the previous path and prediction state.
		bool SubmitMove(const std::uint32_t sequence, const double now,
			const Pose& currentVisual)
		{
			if (!Can_SubmitMove(now) || !ValidPose(currentVisual) ||
				!NewerSequence(sequence, m_lastSubmittedSequence) ||
				!NewerSequence(sequence, m_snapshot.processedMoveSequence))
			{
				return false;
			}
			m_lastSubmittedSequence = sequence;
			m_pendingSequence = sequence;
			m_submittedAt = now;
			m_hasPendingMove = true;
			m_lastVisual = currentVisual;
			m_correctionOffset = {};
			m_correctionStartedAt = now;
			return true;
		}

		SnapshotDisposition ApplySnapshot(const Snapshot& snapshot, const double now,
			const Pose& currentVisual)
		{
			if (!ValidSnapshot(snapshot) || !ValidTime(now) || !ValidPose(currentVisual) ||
				(m_hasSnapshot && (now < m_receivedAt ||
					!NewerCounter(snapshot.serverTick, m_snapshot.serverTick) ||
					(snapshot.processedMoveSequence != m_snapshot.processedMoveSequence &&
						!NewerSequence(snapshot.processedMoveSequence,
							m_snapshot.processedMoveSequence)))))
			{
				return SnapshotDisposition::IGNORED;
			}

			const bool acknowledged = m_hasPendingMove &&
				(snapshot.processedMoveSequence == m_pendingSequence ||
					NewerSequence(snapshot.processedMoveSequence, m_pendingSequence));
			const bool largeCorrection = (!m_hasPendingMove || acknowledged) &&
				DistanceSquared(currentVisual.position, snapshot.position) >
					MAX_SMOOTH_CORRECTION_DISTANCE * MAX_SMOOTH_CORRECTION_DISTANCE;
			const bool reset = !m_hasSnapshot || !snapshot.canPredictMove ||
				IsDiscontinuous(snapshot) || largeCorrection;
			if (acknowledged)
			{
				m_halfRoundTripSeconds = (std::min)(MAX_HALF_ROUND_TRIP_SECONDS,
					(std::max)(0.0, now - m_submittedAt) * 0.5);
			}
			m_snapshot = snapshot;
			m_receivedAt = now;
			m_hasSnapshot = true;
			if (reset)
			{
				m_hasPendingMove = false;
				m_halfRoundTripSeconds = 0.0;
				m_correctionOffset = {};
				m_correctionStartedAt = now;
				m_lastVisual = AuthoritativePose();
				return SnapshotDisposition::RESET;
			}
			if (m_hasPendingMove && !acknowledged &&
				now - m_submittedAt < TIMEOUT_SECONDS)
			{
				m_lastVisual = currentVisual;
				return SnapshotDisposition::PRESERVE_LOCAL_PATH;
			}
			m_hasPendingMove = false;
			BeginCorrection(currentVisual, ProjectSnapshot(now), now);
			return SnapshotDisposition::RECONCILE;
		}

		FrameResult Update(const double now, const float deltaSeconds,
			const Pose& localPathPose)
		{
			FrameResult result{};
			if (!m_hasSnapshot)
				return result;
			result.active = true;
			if (!ValidTime(now) || now < m_receivedAt ||
				!std::isfinite(deltaSeconds) || deltaSeconds < 0.f)
			{
				result.pose = m_lastVisual;
				result.pose.isMoving = false;
				result.stopLocalPath = m_hasPendingMove;
				m_hasPendingMove = false;
				return result;
			}

			const bool snapshotExpired = now - m_receivedAt >= TIMEOUT_SECONDS;
			if (m_hasPendingMove && !snapshotExpired &&
				now - m_submittedAt < TIMEOUT_SECONDS && ValidPose(localPathPose))
			{
				result.pose = localPathPose;
				result.useLocalPath = true;
				m_lastVisual = result.pose;
				return result;
			}
			if (m_hasPendingMove)
			{
				result.stopLocalPath = true;
				m_hasPendingMove = false;
				BeginCorrection(m_lastVisual,
					snapshotExpired ? AuthoritativePose() : ProjectSnapshot(now), now);
			}

			// A dead connection must not run the path indefinitely or keep the RUN
			// clip alive. Hold the last presented pose until a fresh snapshot arrives.
			if (snapshotExpired)
			{
				result.pose = m_lastVisual;
				result.pose.isMoving = false;
				m_lastVisual = result.pose;
				return result;
			}
			result.pose = ProjectSnapshot(now);
			const float correctionWeight = static_cast<float>((std::max)(0.0,
				1.0 - (now - m_correctionStartedAt) / m_correctionDurationSeconds));
			result.pose.position.x += m_correctionOffset.x * correctionWeight;
			result.pose.position.y += m_correctionOffset.y * correctionWeight;
			result.pose.position.z += m_correctionOffset.z * correctionWeight;
			// Character owns shortest-arc yaw smoothing; ACK correction is position-only.
			m_lastVisual = result.pose;
			return result;
		}

	private:
		static bool ValidTime(const double value) { return std::isfinite(value) && value >= 0.0; }
		static bool ValidVector(const Vec3& value)
		{
			return std::isfinite(value.x) && std::isfinite(value.y) && std::isfinite(value.z);
		}
		static bool ValidPose(const Pose& value)
		{
			return ValidVector(value.position) && std::isfinite(value.yawDegrees);
		}
		static bool ValidSnapshot(const Snapshot& value)
		{
			return ValidVector(value.position) && std::isfinite(value.yawDegrees) &&
				std::isfinite(value.moveSpeed) && value.moveSpeed >= 0.f &&
				(!value.hasMoveGoal || (value.canPredictMove &&
					ValidVector(value.nextWaypoint) && value.moveSpeed > 0.f));
		}
		static bool NewerCounter(const std::uint32_t candidate, const std::uint32_t previous)
		{
			const std::uint32_t distance = candidate - previous;
			return distance != 0u && distance < 0x80000000u;
		}
		static bool NewerSequence(const std::uint32_t candidate, const std::uint32_t previous)
		{
			return candidate != 0u && (previous == 0u || NewerCounter(candidate, previous));
		}
		static double DistanceSquared(const Vec3& left, const Vec3& right)
		{
			const double x = static_cast<double>(left.x) - right.x;
			const double y = static_cast<double>(left.y) - right.y;
			const double z = static_cast<double>(left.z) - right.z;
			return x * x + y * y + z * z;
		}
		bool IsDiscontinuous(const Snapshot& snapshot) const
		{
			if (!m_hasSnapshot)
				return false;
			const double elapsed = (std::min)(TIMEOUT_SECONDS,
				static_cast<double>(snapshot.serverTick - m_snapshot.serverTick) / 30.0);
			const double allowed = 0.75 + elapsed *
				(std::max)(snapshot.moveSpeed, m_snapshot.moveSpeed);
			return DistanceSquared(snapshot.position, m_snapshot.position) > allowed * allowed;
		}
		Pose AuthoritativePose() const
		{
			return { m_snapshot.position, m_snapshot.yawDegrees,
				m_snapshot.canPredictMove && m_snapshot.hasMoveGoal };
		}
		Pose ProjectSnapshot(const double now) const
		{
			Pose result = AuthoritativePose();
			if (!result.isMoving)
				return result;
			const double x = static_cast<double>(m_snapshot.nextWaypoint.x) - result.position.x;
			const double z = static_cast<double>(m_snapshot.nextWaypoint.z) - result.position.z;
			const double distance = std::sqrt(x * x + z * z);
			if (distance <= 0.00001)
				return result;
			const double seconds = (std::min)(MAX_EXTRAPOLATION_SECONDS,
				(std::max)(0.0, now - m_receivedAt) + m_halfRoundTripSeconds);
			const double ratio = (std::min)(1.0, m_snapshot.moveSpeed * seconds / distance);
			result.position.x += static_cast<float>(x * ratio);
			// A distant waypoint's height is not the ground under this frame's XZ.
			// Character samples the current navigation cell after this projection.
			result.position.z += static_cast<float>(z * ratio);
			result.yawDegrees = static_cast<float>(std::atan2(x, z) * 180.0 /
				3.14159265358979323846);
			return result;
		}
		void BeginCorrection(const Pose& visual, const Pose& target, const double now)
		{
			if (DistanceSquared(visual.position, target.position) >
				MAX_SMOOTH_CORRECTION_DISTANCE * MAX_SMOOTH_CORRECTION_DISTANCE)
			{
				m_correctionOffset = {};
				m_correctionStartedAt = now;
				m_lastVisual = target;
				return;
			}
			// Keep the old 80 ms floor for small errors, but never correct a large
			// ordinary disagreement faster than the replicated locomotion speed.
			m_correctionDurationSeconds = (std::max)(CORRECTION_SECONDS,
				std::sqrt(DistanceSquared(visual.position, target.position)) /
					(std::max)(1.0, static_cast<double>(m_snapshot.moveSpeed)));
			m_correctionOffset = { visual.position.x - target.position.x,
				visual.position.y - target.position.y, visual.position.z - target.position.z };
			m_correctionStartedAt = now;
			m_lastVisual = visual;
		}

		Snapshot m_snapshot{};
		Pose m_lastVisual{};
		Vec3 m_correctionOffset{};
		double m_receivedAt = 0.0;
		double m_submittedAt = 0.0;
		double m_halfRoundTripSeconds = 0.0;
		double m_correctionStartedAt = 0.0;
		double m_correctionDurationSeconds = CORRECTION_SECONDS;
		std::uint32_t m_lastSubmittedSequence = 0u;
		std::uint32_t m_pendingSequence = 0u;
		bool m_hasSnapshot = false;
		bool m_hasPendingMove = false;
	};
}
