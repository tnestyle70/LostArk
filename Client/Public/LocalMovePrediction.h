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
		using GroundContinuityValidator = bool (*)(const Vec3& from, const Vec3& to, void* context);

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
		static constexpr double SERVER_TICK_SECONDS = 1.0 / 30.0;
		static constexpr double CLOCK_OBSERVATION_WINDOW_SECONDS = 0.5;
		static constexpr double CLOCK_OFFSET_SLEW_PER_SECOND = 0.05;
		static constexpr double CORRECTION_SECONDS = 0.08;
		static constexpr double MAX_SMOOTH_CORRECTION_DISTANCE = 10.0;
		// A small catch-up margin retires ordinary lag while bounding total XZ travel.
		static constexpr double MAX_PRESENTATION_SPEED_SCALE = 1.15;

		void Reset() { *this = CLocalMovePrediction{}; }
		bool HasPendingMove() const { return m_hasPendingMove; }
		bool UsesLocalPath() const { return m_usesLocalPath; }
		bool IsSameMoveGoal(const Vec3& goal) const
		{
			return m_hasMoveGoal && ValidVector(goal) && DistanceSquared(goal, m_moveGoal) < 0.000001;
		}
		float Get_MoveSpeed() const { return m_hasSnapshot ? m_snapshot.moveSpeed : 0.f; }
		float Get_LocalPathDeltaSeconds() const { return static_cast<float>(MotionSeconds()); }

		bool Can_SubmitMove(const double now) const
		{
			return m_hasSnapshot && ValidTime(now) && now >= m_receivedAt &&
				now - m_receivedAt < TIMEOUT_SECONDS &&
				m_snapshot.canPredictMove && m_snapshot.moveSpeed > 0.f;
		}

		// Call only after sending the intent and staging a valid local path. A
		// failed stage/submit must retain the previous path and prediction state.
		bool SubmitMove(const std::uint32_t sequence, const double now,
			const Pose& currentVisual, const Vec3* goal = nullptr)
		{
			if ((goal != nullptr && !ValidVector(*goal)) || !Can_SubmitMove(now) || !ValidPose(currentVisual) ||
				!NewerSequence(sequence, m_lastSubmittedSequence) ||
				!NewerSequence(sequence, m_snapshot.processedMoveSequence))
			{
				return false;
			}
			const bool sameGoal = goal != nullptr && IsSameMoveGoal(*goal);
			if (!sameGoal)
			{
				m_usesLocalPath = true;
				m_localPathSequence = sequence;
				m_localPathStartedAt = m_hasPendingMove ? m_submittedAt : now;
			}
			if (goal != nullptr) { m_moveGoal = *goal; m_hasMoveGoal = true; }
			m_lastSubmittedSequence = sequence;
			m_pendingSequence = sequence;
			if (!m_hasPendingMove) m_submittedAt = now;
			m_hasPendingMove = true;
			// A command changes intent, not the presentation clock or outstanding correction.
			return true;
		}

		SnapshotDisposition ApplySnapshot(const Snapshot& snapshot, const double now,
			const Pose& currentVisual, GroundContinuityValidator validateGround = nullptr,
			void* groundContext = nullptr)
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
			const bool pathAcknowledged = m_usesLocalPath &&
				(snapshot.processedMoveSequence == m_localPathSequence ||
					NewerSequence(snapshot.processedMoveSequence, m_localPathSequence));
			const bool largeCorrection = (!m_usesLocalPath || pathAcknowledged) &&
				DistanceSquared(currentVisual.position, snapshot.position) >
					MAX_SMOOTH_CORRECTION_DISTANCE * MAX_SMOOTH_CORRECTION_DISTANCE;
			const bool reset = !m_hasSnapshot || !snapshot.canPredictMove ||
				IsDiscontinuous(snapshot, validateGround, groundContext) || largeCorrection;
			m_previousSnapshotPosition = m_snapshot.position;
			m_previousSnapshotWaypoint = m_snapshot.nextWaypoint;
			m_previousMoveSpeed = m_snapshot.moveSpeed;
			m_previousHadGoal = m_snapshot.hasMoveGoal;
			m_previousSnapshotServerSeconds = m_snapshotServerSeconds;
			m_hasPreviousSnapshot = m_hasSnapshot && !reset;
			ObserveServerClock(snapshot.serverTick, now, reset);
			m_snapshot = snapshot;
			m_receivedAt = now;
			m_hasSnapshot = true;
			if (m_hasCornerWaypoint)
			{
				const Vec3 outgoing{ snapshot.position.x - m_cornerWaypoint.x, 0.f,
					snapshot.position.z - m_cornerWaypoint.z };
				const Vec3 previousSegment{ m_previousSnapshotWaypoint.x - m_previousSnapshotPosition.x, 0.f,
					m_previousSnapshotWaypoint.z - m_previousSnapshotPosition.z };
				if (!MovesAlongServerSegment(previousSegment) || !snapshot.hasMoveGoal ||
					(DistanceSquaredXZ(snapshot.position, m_cornerWaypoint) > 0.00000001 &&
					!MovesAlongServerSegment(outgoing))) m_hasCornerWaypoint = false;
			}
			if (acknowledged) m_hasPendingMove = false;
			if (reset)
			{
				m_hasPendingMove = false;
				m_usesLocalPath = false;
				m_hasMoveGoal = false;
				m_hasCornerWaypoint = false;
				m_correctionOffset = {};
				m_lastMotionDelta = {};
				m_frameCompleted = true;
				m_lastVisual = AuthoritativePose();
				m_lastFrameAt = now;
				return SnapshotDisposition::RESET;
			}
			if (m_usesLocalPath && !pathAcknowledged &&
				now - m_localPathStartedAt < TIMEOUT_SECONDS)
			{
				// An older command ACK must not undo a new turn. While the actual
				// path still advances along this Server segment, however, consume
				// the same authority correction as an already acknowledged move.
				if (MovesAlongServerSegment(m_lastMotionDelta))
					BeginCorrection(currentVisual, ProjectSnapshot());
				return SnapshotDisposition::PRESERVE_LOCAL_PATH;
			}
			m_usesLocalPath = false;
			if (!snapshot.hasMoveGoal) m_hasCornerWaypoint = false;
			else if (!m_hasCornerWaypoint && KnownServerCorner())
			{
				const double ax = m_previousSnapshotWaypoint.x - m_previousSnapshotPosition.x;
				const double az = m_previousSnapshotWaypoint.z - m_previousSnapshotPosition.z;
				const double vx = currentVisual.position.x - m_previousSnapshotWaypoint.x;
				const double vz = currentVisual.position.z - m_previousSnapshotWaypoint.z;
				if (vx * ax + vz * az < -0.000001)
				{
					m_cornerWaypoint = m_previousSnapshotWaypoint;
					m_hasCornerWaypoint = true;
				}
			}
			BeginCorrection(currentVisual, ProjectSnapshot());
			return SnapshotDisposition::RECONCILE;
		}

		FrameResult Update(const double now, const float deltaSeconds,
			const Pose& localPathPose)
		{
			FrameResult result{};
			if (!m_hasSnapshot) return result;
			result.active = true;
			if (!ValidTime(now) || now < m_receivedAt || now < m_lastFrameAt ||
				!std::isfinite(deltaSeconds) || deltaSeconds < 0.f)
			{
				result.pose = m_lastVisual;
				result.pose.isMoving = false;
				result.stopLocalPath = m_usesLocalPath;
				m_hasPendingMove = false;
				m_usesLocalPath = false;
				return result;
			}
			if (!m_usesLocalPath && now == m_receivedAt)
			{
				result.pose = ProjectSnapshot();
				result.pose.position = m_lastVisual.position;
				return result;
			}
			const double frameSeconds = now > m_lastFrameAt ? static_cast<double>(deltaSeconds) : 0.0;
			if (frameSeconds > 0.0)
			{
				if (!m_hasPresentationClock)
				{
					m_presentationOrigin = now - frameSeconds - m_presentationSeconds;
					m_hasPresentationClock = true;
				}
				m_frameStartVisual = m_lastVisual;
				m_frameSeconds = frameSeconds;
				m_frameCompleted = false;
				m_presentationSeconds += frameSeconds;
				const double difference = m_observedClockOffset - m_clockOffset;
				const double adjustment = CLOCK_OFFSET_SLEW_PER_SECOND * frameSeconds;
				m_clockOffset += (std::max)(-adjustment, (std::min)(adjustment, difference));
			}
			m_lastFrameAt = now;
			const bool snapshotExpired = now - m_receivedAt >= TIMEOUT_SECONDS;
			if (m_usesLocalPath && !snapshotExpired &&
				now - m_localPathStartedAt < TIMEOUT_SECONDS && ValidPose(localPathPose))
			{
				result.pose = localPathPose;
				result.useLocalPath = true;
				return result;
			}
			if (m_hasPendingMove && (snapshotExpired || now - m_submittedAt >= TIMEOUT_SECONDS))
				m_hasPendingMove = false;
			if (m_usesLocalPath)
			{
				result.stopLocalPath = true;
				m_usesLocalPath = false;
				if (!MovesAlongServerSegment(m_lastMotionDelta))
					BeginCorrection(m_lastVisual, snapshotExpired ? AuthoritativePose() : ProjectSnapshot(frameSeconds));
			}
			if (snapshotExpired)
			{
				result.pose = m_lastVisual;
				result.pose.isMoving = false;
				m_lastVisual = result.pose;
				m_frameCompleted = true;
				return result;
			}
			Pose motion = AuthoritativePose();
			motion.position = m_frameStartVisual.position;
			const bool followingCorner = m_hasCornerWaypoint;
			if (motion.isMoving)
			{
				double travel = m_snapshot.moveSpeed * MotionSeconds();
				if (m_hasCornerWaypoint)
				{
					AdvanceTowards(motion, m_cornerWaypoint, travel);
					if (DistanceSquaredXZ(motion.position, m_cornerWaypoint) < 0.00000001)
						m_hasCornerWaypoint = false;
				}
				if (!m_hasCornerWaypoint) AdvanceTowards(motion, m_snapshot.nextWaypoint, travel);
			}
			// While rounding an observed Server corner, consume its two segments
			// before resuming vector correction; a residual must not cut inside it.
			result.pose = CompleteFrame(motion, !followingCorner);
			return result;
		}

		// Character stages its existing navigation step, then commits the same
		// correction and travel budget used by acknowledged snapshot movement.
		FrameResult CompleteLocalPathFrame(const double now, const Pose& localPathPose)
		{
			FrameResult result{};
			result.active = m_hasSnapshot;
			result.useLocalPath = m_usesLocalPath;
			result.pose = m_lastVisual;
			if (m_usesLocalPath && now == m_lastFrameAt && ValidPose(localPathPose))
			{
				const double dx = localPathPose.position.x - m_frameStartVisual.position.x;
				const double dz = localPathPose.position.z - m_frameStartVisual.position.z;
				const double sx = m_snapshot.nextWaypoint.x - m_snapshot.position.x;
				const double sz = m_snapshot.nextWaypoint.z - m_snapshot.position.z;
				const double dot = dx * sx + dz * sz;
				const double lengths = (dx * dx + dz * dz) * (sx * sx + sz * sz);
				const bool turning = lengths > 0.00000001 && (dot <= 0.0 || dot * dot < lengths * 0.99);
				if (m_hasCornerWaypoint && dx * dx + dz * dz > 0.00000001)
				{
					const double cx = m_cornerWaypoint.x - m_frameStartVisual.position.x;
					const double cz = m_cornerWaypoint.z - m_frameStartVisual.position.z;
					const double cornerDot = dx * cx + dz * cz;
					if (cornerDot <= 0.0 || cornerDot * cornerDot <
						(dx * dx + dz * dz) * (cx * cx + cz * cz) * 0.99)
						m_hasCornerWaypoint = false;
				}
				// Only an unacknowledged actual turn may defer the old segment's
				// residual. Keep both residual and duration for the next authority sample.
				result.pose = CompleteFrame(localPathPose, !turning);
			}
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
		bool IsDiscontinuous(const Snapshot& snapshot, GroundContinuityValidator validateGround,
			void* groundContext) const
		{
			if (!m_hasSnapshot)
				return false;
			// ApplySnapshot admits only a forward tick distance below half the
			// uint32 range, including wrap. Server motion spans that whole interval
			// even when a stalled Client coalesces snapshots. The local freshness
			// timeout limits prediction; it must not turn legal Server travel into
			// an authoritative teleport.
			const double elapsed =
				static_cast<double>(snapshot.serverTick - m_snapshot.serverTick) / 30.0;
			const double allowed = 0.75 + elapsed *
				(std::max)(snapshot.moveSpeed, m_snapshot.moveSpeed);
			if (DistanceSquared(snapshot.position, m_snapshot.position) <= allowed * allowed)
				return false;
			const double x = static_cast<double>(snapshot.position.x) - m_snapshot.position.x;
			const double z = static_cast<double>(snapshot.position.z) - m_snapshot.position.z;
			if (x * x + z * z > allowed * allowed)
				return true;
			// Only navigation-proven ground changes may exempt a vertical step.
			return nullptr == validateGround ||
				!validateGround(m_snapshot.position, snapshot.position, groundContext);
		}
		Pose AuthoritativePose() const
		{
			return { m_snapshot.position, m_snapshot.yawDegrees,
				m_snapshot.canPredictMove && m_snapshot.hasMoveGoal };
		}
		static double DistanceSquaredXZ(const Vec3& a, const Vec3& b)
		{
			const double x = static_cast<double>(a.x) - b.x;
			const double z = static_cast<double>(a.z) - b.z;
			return x * x + z * z;
		}
		static void AdvanceTowards(Pose& pose, const Vec3& waypoint, double& travel)
		{
			const double x = static_cast<double>(waypoint.x) - pose.position.x;
			const double z = static_cast<double>(waypoint.z) - pose.position.z;
			const double distance = std::sqrt(x * x + z * z);
			if (distance <= 0.00001) return;
			const double step = (std::min)(distance, travel);
			pose.position.x += static_cast<float>(x * step / distance);
			pose.position.z += static_cast<float>(z * step / distance);
			pose.yawDegrees = static_cast<float>(std::atan2(x, z) * 180.0 / 3.14159265358979323846);
			travel -= step;
		}
		bool KnownServerCorner() const
		{
			if (!m_hasPreviousSnapshot || !m_previousHadGoal || !m_snapshot.hasMoveGoal) return false;
			const double ax = m_previousSnapshotWaypoint.x - m_previousSnapshotPosition.x;
			const double az = m_previousSnapshotWaypoint.z - m_previousSnapshotPosition.z;
			const double bx = m_snapshot.position.x - m_previousSnapshotWaypoint.x;
			const double bz = m_snapshot.position.z - m_previousSnapshotWaypoint.z;
			const double cx = m_snapshot.nextWaypoint.x - m_snapshot.position.x;
			const double cz = m_snapshot.nextWaypoint.z - m_snapshot.position.z;
			const double first = std::sqrt(ax * ax + az * az);
			const double second = std::sqrt(bx * bx + bz * bz);
			const double directionProduct = (bx * bx + bz * bz) * (cx * cx + cz * cz);
			const double dot = bx * cx + bz * cz;
			return first > 0.00001 && std::abs(ax * bz - az * bx) > 0.00001 &&
				dot > 0.0 && dot * dot >= directionProduct * 0.999999 &&
				first + second <= (std::max)(m_previousMoveSpeed, m_snapshot.moveSpeed) *
					(m_snapshotServerSeconds - m_previousSnapshotServerSeconds) + 0.001;
		}
		bool MovesAlongServerSegment(const Vec3& movement) const
		{
			const double x = m_snapshot.nextWaypoint.x - m_snapshot.position.x;
			const double z = m_snapshot.nextWaypoint.z - m_snapshot.position.z;
			const double dot = x * movement.x + z * movement.z;
			const double lengths = (x * x + z * z) * (movement.x * movement.x + movement.z * movement.z);
			return m_snapshot.hasMoveGoal && dot > 0.0 && dot * dot >= lengths * 0.999999;
		}
		Pose ProjectSnapshot(const double secondsBehind = 0.0) const
		{
			Pose result = AuthoritativePose();
			const double serverNow = m_presentationOrigin + m_presentationSeconds - secondsBehind - m_clockOffset;
			if (result.isMoving)
				result.yawDegrees = static_cast<float>(std::atan2(m_snapshot.nextWaypoint.x - result.position.x,
					m_snapshot.nextWaypoint.z - result.position.z) * 180.0 / 3.14159265358979323846);
			if (result.isMoving && m_hasPreviousSnapshot && serverNow < m_snapshotServerSeconds)
			{
				// A packet applied after Character::Update may describe a point just
				// ahead of the displayed frame. Sample the same Server time instead
				// of clamping that negative age to a newly arrived tick's position.
				const double span = m_snapshotServerSeconds - m_previousSnapshotServerSeconds;
				const double alpha = span > 0.0 ? (std::max)(0.0,
					(std::min)(1.0, (serverNow - m_previousSnapshotServerSeconds) / span)) : 1.0;
				const double ax = m_previousSnapshotWaypoint.x - m_previousSnapshotPosition.x;
				const double az = m_previousSnapshotWaypoint.z - m_previousSnapshotPosition.z;
				const double bx = m_snapshot.position.x - m_previousSnapshotWaypoint.x;
				const double bz = m_snapshot.position.z - m_previousSnapshotWaypoint.z;
				const double firstLength = std::sqrt(ax * ax + az * az);
				const double secondLength = std::sqrt(bx * bx + bz * bz);
				const bool reachedCorner = KnownServerCorner();
				if (reachedCorner)
				{
					// Preserve the known waypoint bend instead of cutting diagonally
					// across the inside of a navigation corner between two packets.
					const double distance = (firstLength + secondLength) * alpha;
					const double alongFirst = (std::min)(distance, firstLength) / firstLength;
					const double alongSecond = secondLength > 0.00001 ?
						(std::max)(0.0, distance - firstLength) / secondLength : 0.0;
					result.position.x = m_previousSnapshotPosition.x +
						static_cast<float>(ax * alongFirst + bx * alongSecond);
					result.position.z = m_previousSnapshotPosition.z +
						static_cast<float>(az * alongFirst + bz * alongSecond);
				}
				else
				{
					result.position.x = m_previousSnapshotPosition.x +
						static_cast<float>((m_snapshot.position.x - m_previousSnapshotPosition.x) * alpha);
					result.position.z = m_previousSnapshotPosition.z +
						static_cast<float>((m_snapshot.position.z - m_previousSnapshotPosition.z) * alpha);
				}
				return result;
			}
			if (!result.isMoving)
				return result;
			const double x = static_cast<double>(m_snapshot.nextWaypoint.x) - result.position.x;
			const double z = static_cast<double>(m_snapshot.nextWaypoint.z) - result.position.z;
			const double distance = std::sqrt(x * x + z * z);
			if (distance <= 0.00001)
				return result;
			const double seconds = (std::min)(MAX_EXTRAPOLATION_SECONDS,
				(std::max)(0.0, serverNow - m_snapshotServerSeconds));
			const double ratio = (std::min)(1.0, m_snapshot.moveSpeed * seconds / distance);
			result.position.x += static_cast<float>(x * ratio);
			// A distant waypoint's height is not the ground under this frame's XZ.
			// Character samples the current navigation cell after this projection.
			result.position.z += static_cast<float>(z * ratio);
			result.yawDegrees = static_cast<float>(std::atan2(x, z) * 180.0 /
				3.14159265358979323846);
			return result;
		}
		void ObserveServerClock(const std::uint32_t tick, const double now, const bool reset)
		{
			if (reset)
			{
				m_snapshotServerSeconds = 0.0;
				m_presentationOrigin = now - m_presentationSeconds;
				m_clockOffset = now;
				m_observedClockOffset = now;
				m_windowClockOffset = now;
				m_clockObservationStartedAt = now;
				return;
			}
			std::uint32_t ticks = tick - m_snapshot.serverTick;
			// GameRoom skips zero at wrap; accept zero too for existing protocol fixtures.
			if (tick < m_snapshot.serverTick && tick != 0u) --ticks;
			m_snapshotServerSeconds += static_cast<double>(ticks) * SERVER_TICK_SECONDS;
			const double observation = now - m_snapshotServerSeconds;
			if (!m_snapshot.hasMoveGoal)
			{
				// A stopped actor has no movement phase to preserve. Establish the
				// receive-clock baseline before its next continuous movement span.
				m_clockOffset = m_observedClockOffset = m_windowClockOffset = observation;
				m_clockObservationStartedAt = now;
				return;
			}
			// The earliest arrival in a short window rejects batching jitter. New
			// windows still admit a persistent latency change. Slew only once per
			// displayed frame, independently of command/ACK count in this batch.
			m_windowClockOffset = (std::min)(m_windowClockOffset, observation);
			if (observation < m_observedClockOffset) m_observedClockOffset = observation;
			if (now - m_clockObservationStartedAt >= CLOCK_OBSERVATION_WINDOW_SECONDS)
			{
				// Frame-quantized receive stamps do not identify sub-tick increases
				// in transport delay. Admit a persistent increase only beyond that
				// uncertainty, instead of retiming every 30/40 Hz arrival phase.
				if (m_windowClockOffset > m_observedClockOffset + SERVER_TICK_SECONDS)
					m_observedClockOffset = m_windowClockOffset;
				m_windowClockOffset = observation;
				m_clockObservationStartedAt = now;
			}
		}
		// The same Server-clock horizon bounds both local and acknowledged motion.
		double MotionSeconds() const
		{
			const double ageBeforeFrame = (std::max)(0.0, m_presentationOrigin +
				m_presentationSeconds - m_frameSeconds - (m_snapshotServerSeconds + m_clockOffset));
			const double horizon = (std::max)(MAX_EXTRAPOLATION_SECONDS,
				(std::min)(TIMEOUT_SECONDS, m_snapshotServerSeconds - m_previousSnapshotServerSeconds));
			return (std::min)(m_frameSeconds,
				(std::max)(0.0, horizon - ageBeforeFrame));
		}
		Pose CompleteFrame(Pose motion, const bool consumeCorrection = true)
		{
			if (m_frameCompleted) return m_lastVisual;
			m_frameCompleted = true;
			const double fraction = !consumeCorrection ? 0.0 : (m_correctionDurationSeconds > 0.0 ?
				(std::min)(1.0, m_frameSeconds / m_correctionDurationSeconds) : 1.0);
			const Vec3 base = motion.position;
			const Vec3 motionDelta{ base.x - m_frameStartVisual.position.x,
				base.y - m_frameStartVisual.position.y, base.z - m_frameStartVisual.position.z };
			if (motionDelta.x * motionDelta.x + motionDelta.z * motionDelta.z > 0.00000001f)
				m_lastMotionDelta = motionDelta;
			motion.position.x -= static_cast<float>(m_correctionOffset.x * fraction);
			motion.position.y -= static_cast<float>(m_correctionOffset.y * fraction);
			motion.position.z -= static_cast<float>(m_correctionOffset.z * fraction);
			const double x = static_cast<double>(motion.position.x) - m_frameStartVisual.position.x;
			const double z = static_cast<double>(motion.position.z) - m_frameStartVisual.position.z;
			const double distance = std::sqrt(x * x + z * z);
			const double speed = m_snapshot.moveSpeed > 0.f ? m_snapshot.moveSpeed : 1.0;
			const double budget = speed * MAX_PRESENTATION_SPEED_SCALE * m_frameSeconds;
			if (distance > budget)
			{
				motion.position.x = m_frameStartVisual.position.x + static_cast<float>(x * budget / distance);
				motion.position.z = m_frameStartVisual.position.z + static_cast<float>(z * budget / distance);
			}
			m_correctionOffset.x += motion.position.x - base.x;
			m_correctionOffset.y += motion.position.y - base.y;
			m_correctionOffset.z += motion.position.z - base.z;
			if (consumeCorrection)
				m_correctionDurationSeconds = (std::max)(0.0, m_correctionDurationSeconds - m_frameSeconds);
			m_lastVisual = motion;
			return motion;
		}
		void BeginCorrection(const Pose& visual, const Pose& target)
		{
			if (DistanceSquared(visual.position, target.position) >
				MAX_SMOOTH_CORRECTION_DISTANCE * MAX_SMOOTH_CORRECTION_DISTANCE)
			{
				m_correctionOffset = {};
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
			m_lastVisual = visual;
		}

		Snapshot m_snapshot{};
		Pose m_lastVisual{};
		Pose m_frameStartVisual{};
		Vec3 m_correctionOffset{};
		Vec3 m_lastMotionDelta{};
		Vec3 m_moveGoal{};
		Vec3 m_previousSnapshotPosition{};
		Vec3 m_previousSnapshotWaypoint{};
		Vec3 m_cornerWaypoint{};
		float m_previousMoveSpeed = 0.f;
		// Wall time owns freshness and clock observations; MOVE ACKs own only sequence retirement.
		double m_receivedAt = 0.0;
		double m_lastFrameAt = 0.0;
		double m_submittedAt = 0.0;
		double m_localPathStartedAt = 0.0;
		double m_snapshotServerSeconds = 0.0;
		double m_previousSnapshotServerSeconds = 0.0;
		double m_clockOffset = 0.0;
		double m_observedClockOffset = 0.0;
		double m_windowClockOffset = 0.0;
		double m_clockObservationStartedAt = 0.0;
		double m_presentationOrigin = 0.0;
		double m_presentationSeconds = 0.0;
		double m_frameSeconds = 0.0;
		double m_correctionDurationSeconds = CORRECTION_SECONDS;
		std::uint32_t m_lastSubmittedSequence = 0u;
		std::uint32_t m_pendingSequence = 0u;
		std::uint32_t m_localPathSequence = 0u;
		bool m_frameCompleted = true;
		bool m_hasPresentationClock = false;
		bool m_hasSnapshot = false;
		bool m_hasPendingMove = false;
		bool m_usesLocalPath = false;
		bool m_hasMoveGoal = false;
		bool m_hasPreviousSnapshot = false;
		bool m_previousHadGoal = false;
		bool m_hasCornerWaypoint = false;
	};
}
