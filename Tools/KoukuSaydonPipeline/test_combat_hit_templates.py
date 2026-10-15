# Python 공격 정의 검증기와 projector 함수에 입력을 주고 반환값 또는 예외를 확인한다.
# C++ Server 실행, 실제 전투 판정, 네트워크 복제나 Client 화면까지 증명하는 검사는 아니다.
import copy, math, unittest
from pathlib import Path
from Tools.KoukuSaydonPipeline.combat_hit_templates import validate_hits, validate_logic_hits
from Tools.KoukuSaydonPipeline import project_kouku_saydon_composition as p


class AttackTemplateTests(unittest.TestCase):
    # 예전 정의에는 비행 필드를 새로 만들지 않는다. 높이·시간을 지정할 때는 둘이 유효한 쌍이어야 한다.
    def test_optional_flight_preserves_legacy_and_bounds_pair(self):
        self.assertNotIn('riseHeightM', validate_hits([{'hitId': 'legacy'}])[0])
        row = {'hitId': 'rise', 'riseHeightM': 3, 'pushMs': 1200}
        self.assertEqual(validate_hits([row])[0]['pushMs'], 1200)

        # 한쪽 값 누락, 높이·시간 범위 초과, 숫자 대신 bool을 준 입력은 거부한다.
        for fields in (
            {'riseHeightM': 3},
            {'pushMs': 1200},
            {'riseHeightM': 101, 'pushMs': 1200},
            {'riseHeightM': 3, 'pushMs': 99},
            {'riseHeightM': True, 'pushMs': 1200},
        ):
            with self.subTest(fields=fields), self.assertRaises(ValueError):
                validate_hits([{'hitId': 'bad', **fields}])

    # 생략한 밀침 정책과 명시적인 false를 구분하며, 수평 이동 값과 방향을 그대로 보존한다.
    def test_explicit_push_policy_preserves_horizontal_and_legacy_defaults(self):
        legacy = validate_hits([dict(hitId='legacy', riseHeightM=2, pushMs=250)])[0]
        self.assertNotIn('forcePush', legacy)
        self.assertNotIn('pushDirection', legacy)
        for rise in (0, 2):
            hit = validate_hits([
                dict(
                    hitId='original.result', riseHeightM=rise, pushRangeM=1,
                    pushMs=250, forcePush=False, pushDirection='AWAY_FROM_BOSS',
                ),
            ])[0]
            self.assertIs(hit['forcePush'], False)
            self.assertEqual(hit['pushDirection'], 'AWAY_FROM_BOSS')
            self.assertEqual(hit['pushRangeM'], 1)

        for fields in (
            {'pushRangeM': 1},
            {'pushRangeM': 1, 'pushMs': 99},
            {'forcePush': 0},
            {'forcePush': 'false'},
            {'pushDirection': 'BOSS_FORWARD'},
        ):
            with self.subTest(fields=fields), self.assertRaises(ValueError):
                validate_hits([dict(hitId='bad', **fields)])

    # 최소 입력에는 반환값에서 피해 기본값 10을 채우지만, 호출자가 넘긴 원본 dict는 바꾸지 않는다.
    def test_defaults_and_no_input_mutation(self):
        row = {'hitId': 'attack.1'}
        before = copy.deepcopy(row)
        self.assertEqual(validate_hits([row])[0]['damagePercent'], 10)
        self.assertEqual(row, before)

    # 허용된 도형과 피해 정책 조합은 예외 없이 통과해야 한다.
    def test_primitive_and_damage_policies(self):
        for fields in (
            {},
            {'shape': 'RING', 'radiusM': 4, 'innerRadiusM': 2},
            {'shape': 'BOX', 'lengthM': 3, 'halfWidthM': 1},
            {'shape': 'CONE', 'lengthM': 3, 'angleDegrees': 90},
            {'damageKind': 'INSTANT_DEATH', 'damagePercent': 0},
            {'damageKind': 'PROFILE', 'damagePercent': 0, 'damageProfileId': 'source.damage'},
        ):
            validate_hits([{'hitId': 'attack.1', **fields}])

    # 유한하지 않은 값, 잘못된 범위·반복·도형·피해 정책과 알 수 없는 키를 각각 거부한다.
    def test_bad_values_are_rejected(self):
        for fields in (
            {'radiusM': math.nan},
            {'radiusM': 0},
            {'atMs': True},
            {'atMs': 601000},
            {'repeatCount': 0},
            {'repeatCount': 2, 'repeatIntervalMs': 1},
            {'shape': 'RING', 'innerRadiusM': 2},
            {'trigger': 'CONTACT', 'endMs': 0},
            {'damageKind': 'PROFILE'},
            {'damageKind': 'INSTANT_DEATH'},
            {'unknown': 1},
        ):
            with self.subTest(fields=fields), self.assertRaises(ValueError):
                validate_hits([{'hitId': 'a', **fields}])

    # 100ms 시작 + 100ms 간격으로 3회면 마지막은 300ms이므로 299ms 수명을 넘는다.
    def test_owner_lifetime_rejects_repeat_tail(self):
        with self.assertRaises(ValueError):
            validate_hits([
                {'hitId': 'a', 'atMs': 100, 'repeatCount': 3, 'repeatIntervalMs': 100},
            ], 299)

    # 같은 목록 안의 중복 ID와 상한 32개를 넘긴 33개 입력을 별도로 거부한다.
    def test_duplicate_and_count(self):
        for rows in ([{'hitId': 'a'}] * 2, [{'hitId': str(i)} for i in range(33)]):
            with self.assertRaises(ValueError):
                validate_hits(rows)

    # Logic 종류에 없는 공격 필드 또는 시각 set 수와 맞지 않는 무작위 공격 목록은 거부한다.
    def test_wrong_logic_owner_and_random_join(self):
        for logic in (
            {'logicType': 'TRIGGER', 'projectileHits': []},
            {'logicType': 'DURATION', 'judgementKind': 'PURSUIT_PROJECTILES', 'fixedHits': []},
            {'logicType': 'DURATION', 'judgementKind': 'SHOWTIME_PLAYER_TARGETS', 'randomVolleyHits': [[]]},
        ):
            with self.assertRaises(ValueError):
                validate_logic_hits(logic)

    # 같은 2000ms 공격이 원의 수명 5680ms 안에서는 통과하고, 수명을 1999ms로 줄이면 실패한다.
    def test_albion_hits_use_existing_circle_lifetime(self):
        logic = dict(
            logicType='TRIGGER', triggerKind='ALBION_BLUE_CIRCLE', effectLifetimeMs=5680,
            fixedHits=[dict(hitId='blue.impact', atMs=2000, radiusM=1.6)],
        )
        validate_logic_hits(logic)
        logic['effectLifetimeMs'] = 1999
        with self.assertRaises(ValueError):
            validate_logic_hits(logic)

    # projector에 시각 자원·Logic·occurrence를 함께 주어 연결된 추적 공격의 수명 경계를 검사한다.
    def test_showtime_projection_joins_same_instance_lifetime(self):
        resources = [
            dict(
                resourceId='effect.1', kind='EFFECT', assetId='effect.test',
                resourceKind='V1_EFFECT', durationMs=1000,
            ),
        ]
        logic = dict(
            logicId='logic.1', logicType='DURATION', judgementKind='SHOWTIME_PLAYER_TARGETS',
            trackingPresentationOccurrenceId='p.effect.1',
            trackingHits=[
                dict(
                    hitId='death', trigger='CONTACT', endMs=2000,
                    damageKind='INSTANT_DEATH', damagePercent=0,
                ),
            ],
        )
        pattern = dict(
            patternId='p', actorProfileId='MN_RPCT_07',
            targetBossPlacementId='boss.koukusaydon.g3.saydon',
            presentationOccurrences=[
                dict(
                    occurrenceId='p.effect.1', resourceId='effect.1', anchorKind='MAP',
                    followBoss=False, startMs=0, durationMs=1000,
                ),
            ],
            logicOccurrences=[
                dict(occurrenceId='p.logic.1', logicId='logic.1', startMs=0, durationMs=2000),
            ],
        )
        doc = dict(logics=[logic], presentationResources=resources)
        rows, templates, _ = p._project_showtime_targets(doc, pattern)
        self.assertEqual(rows[0]['trackingHits'][0]['endMs'], 2000)
        self.assertEqual(len(templates), 1)

        # 경계값 2000ms는 보존하지만 같은 정의에서 1ms 더 긴 접촉 구간은 거부한다.
        logic['trackingHits'][0]['endMs'] = 2001
        with self.assertRaises(p.CompositionError):
            p._project_showtime_targets(doc, pattern)


class HollowColliderTests(unittest.TestCase):
    # 각 테스트가 독립적으로 수정할 최소 입력이다. 바깥 반경 3m, 빈 안쪽 반경 1m이며 XZ 배율은 2다.
    def fixture(self):
        resource = dict(
            resourceId='shape.1', displayName='Hollow', kind='COLLIDER', assetId='',
            radiusM=3, innerRadiusM=1, shape='CIRCLE', durationMs=1000,
        )
        box = dict(
            occurrenceId='p.collider.1', resourceId='shape.1', logicOccurrenceId='p.logic.1',
            startMs=0, durationMs=1000, scale=[2, 1, 2],
        )
        return (
            dict(presentationResources=[resource]),
            dict(presentationOccurrences=[box]),
            dict(occurrenceId='p.logic.1', startMs=0, durationMs=1000),
            dict(logicType='TRIGGER', triggerKind='ENTER_AREA'),
        )

    # 원·부채꼴의 빈 중심도 XZ 배율을 적용하여 안쪽 2m, 바깥 6m로 투영되어야 한다.
    def test_ring_and_hollow_sector_preserve_scaled_hole(self):
        for shape in ('CIRCLE', 'SECTOR'):
            doc, pattern, box, logic = self.fixture()
            doc['presentationResources'][0]['shape'] = shape
            p._validate_presentation_resources(doc)
            result = p._project_collider_regions(doc, pattern, box, logic, {}, Path('.'))[0]
            self.assertEqual((result['innerRadiusM'], result['radiusM']), (2, 6))

    # 음수·동일 반경·NaN과 빈 중심을 지원하지 않는 자원/도형 조합을 거부한다.
    def test_invalid_inner_shape_and_radius_rejected(self):
        for fields in (
            {'innerRadiusM': -1},
            {'innerRadiusM': 3},
            {'innerRadiusM': math.nan},
            {'shape': 'BOX'},
            {'shape': 'REVERSE_SECTOR'},
            {'kind': 'EFFECT', 'assetId': 'effect.test'},
        ):
            doc, *_ = self.fixture()
            doc['presentationResources'][0].update(fields)
            with self.subTest(fields=fields), self.assertRaises(p.CompositionError):
                p._validate_presentation_resources(doc)

    # 원형 구멍에 X와 Z의 서로 다른 배율을 적용하는 입력은 지원 범위를 벗어나므로 거부한다.
    def test_nonuniform_hole_rejected(self):
        doc, pattern, box, logic = self.fixture()
        pattern['presentationOccurrences'][0]['scale'] = [1, 1, 2]
        with self.assertRaises(p.CompositionError):
            p._project_collider_regions(doc, pattern, box, logic, {}, Path('.'))

    # 원본에 innerRadiusM이 없는 기존 채운 도형에는 projector도 그 필드를 새로 만들지 않는다.
    def test_legacy_filled_resource_output_unchanged(self):
        doc, *_ = self.fixture()
        r = doc['presentationResources'][0]
        r.pop('innerRadiusM')
        self.assertNotIn('innerRadiusM', p._project_presentation_resource(r))


if __name__ == '__main__':
    unittest.main()
