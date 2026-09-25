"""Evaluate the original, bounded class-selection AnimTree into local poses.

This is an offline source evaluator. It creates no alternate runtime animation
path; its output is consumed by the existing WModel/WANM writer.
"""
from __future__ import annotations

import math
import numpy as np

from project_guardian_selection import base
from build_bingo_ending_actors import blend, strength_at


def control_names(tracks):
    names = set()
    for track in tracks:
        p = track['p']
        if track['cls'] == 'interptrackskelcontrolstrength':
            names.add(p['skelcontrolname'])
        elif track['cls'] in ('efinterptrackskelcontrolvector', 'efinterptrackskelcontrolmulti'):
            for key, value in p.items():
                if key.startswith('skelcontrolnamelist'):
                    assert isinstance(value, list), ('Undecoded source controls', track['index'])
                    names.update(value)
    return names


def source_strengths(tracks, seconds):
    result = strength_at(tracks, seconds)
    for track in tracks:
        if track['cls'] == 'efinterptrackskelcontrolmulti':
            p = track['p']
            value = float(base.curve(p.get('floattrack', {}).get('points', []), seconds, 0.))
            for name in p['skelcontrolnamelist']:
                result[name] = value
    return result


def source_controls(model, rows, root, tracks):
    """Resolve driven source chains, retaining palette omissions in the receipt."""
    names, found, result, omitted = control_names(tracks), set(), [], []
    palette = {bone.name.casefold(): i for i, bone in enumerate(model.skeleton_bones)}
    assert len(palette) == len(model.skeleton_bones), 'Duplicate case-insensitive bone names'
    for chain in rows[root]['p'].get('skelcontrollists', []):
        index = palette.get(chain['bonename'].casefold())
        ref, seen = chain['controlhead'], set()
        while ref:
            assert ref not in seen and len(seen) < 4096, 'Cyclic or oversized control chain'
            seen.add(ref)
            row = rows[ref]
            assert 'parseError' not in row, ('Unparsed control', ref)
            p = row['p']; ref = p.get('nextcontrol', 0)
            name = p.get('controlname')
            if name not in names:
                # Engine's serialized SkelControlBase CDO has strength 1.
                # Omission is safe only for zero strength or an absent source
                # actor bone (the Fighter tree's unused cloth Trail chains).
                if index is None:
                    if float(p.get('controlstrength', 1.)) != 0:
                        omitted.append(dict(bone=chain['bonename'], control=name,
                            sourceNode=row['name'], reason='Undriven control bone absent from exact source palette'))
                else:
                    assert float(p.get('controlstrength', 1.)) == 0., (
                        'Observable undriven source control requires evaluation', row['name'], chain['bonename'])
                continue
            found.add(name)
            if index is None:
                omitted.append(dict(bone=chain['bonename'], control=name,
                                    reason='Bone absent from this exact source actor palette'))
                continue
            assert row['cls'] == 'skelcontrolsinglebone', ('Unsupported driven control', row['name'])
            assert p.get('bonerotationspace', 'bcs_bonespace') in ('bcs_parentbonespace', 'bcs_bonespace')
            assert p.get('bonetranslationspace', 'bcs_bonespace') in (
                'bcs_parentbonespace', 'bcs_bonespace', 'bcs_otherbonespace')
            if p.get('bonetranslationspace') == 'bcs_otherbonespace':
                assert p.get('translationspacebonename', 'none') == 'none'
            if p.get('bapplytranslation'):
                assert p.get('baddtranslation'), ('Unsupported replacement translation', row['name'])
            if p.get('bapplyrotation'):
                assert p.get('baddrotation'), ('Unsupported replacement rotation', row['name'])
            result.append((index, p))
    # A Matinee group can drive several actors with different original trees.
    # A name lookup cannot alter a controller absent from this actor's exact
    # tree (notably the inherited DM LookInfo tree's four asymmetric controls).
    # Preserve that source no-op explicitly instead of inventing a controller.
    omitted.extend(dict(control=name, reason='Named control absent from exact source AnimTree')
                   for name in sorted(names - found))
    return sorted(result, key=lambda pair: pair[0]), omitted


def _masked_blend(lower, upper, weights):
    if not np.any(weights):
        return lower
    if np.all(weights == 1):
        return upper
    output = []
    for left, right, weight in zip(lower, upper, weights):
        if weight == 0:
            output.append(left)
        elif weight == 1:
            output.append(right)
        else:
            p, q, s = left; ap, aq, asc = right
            output.append((p*(1-weight)+ap*weight, base.slerp(q, aq, weight), s*(1-weight)+asc*weight))
    return output


class SourceAnimTree:
    def __init__(self, model, rows, root, tracks, clips):
        assert root in rows and rows[root]['cls'] == 'animtree', ('Expected exact AnimTree root', root)
        self.model, self.rows, self.root, self.clips = model, rows, root, clips
        self.animation = {}
        for track in tracks:
            if track['cls'] != 'interptrackanimcontrol':
                continue
            slot = track['p']['slotname'].casefold()
            assert slot not in self.animation, ('Duplicate active slot', slot)
            self.animation[slot] = track['p']
            for key in track['p'].get('animseqs', []):
                assert key['animseqname'] in clips, ('Missing exact source clip', key['animseqname'])
        self.palette = {bone.name.casefold(): i for i, bone in enumerate(model.skeleton_bones)}
        assert len(self.palette) == len(model.skeleton_bones)
        self.masks, self.mask_receipt, self.reachable, slots = {}, [], set(), set()
        visiting = set()

        def visit(ref):
            assert ref > 0 and ref in rows, ('Unresolved AnimTree reference', ref)
            assert ref not in visiting, ('Cyclic AnimTree', ref)
            if ref in self.reachable:
                return
            assert len(self.reachable) < 4096, 'Oversized AnimTree'
            visiting.add(ref); self.reachable.add(ref)
            row = rows[ref]; p = row['p']
            assert 'parseError' not in row, ('Unparsed AnimTree node', row['name'])
            children = p.get('children', [])
            for child in children:
                assert not child.get('bmirrorskeleton') and not child.get('bisadditive'), (
                    'Unsupported mirrored/additive child', row['name'])
                if child.get('anim'):
                    visit(child['anim'])
            if row['cls'] == 'animnodeslot':
                slot = p['nodename'].casefold()
                assert slot not in slots, ('Duplicate source slot node', slot)
                slots.add(slot)
                assert len(children) == 2 and children[0].get('anim') and not children[1].get('anim'), (
                    'Expected original source plus an unbound Matinee channel', row['name'])
            elif row['cls'] == 'animnode_multiblendperbone':
                masks = p.get('masklist', [])
                assert len(children) == len(masks)+1
                self.masks[ref] = [self._mask(row, mask) for mask in masks]
            elif row['cls'] not in ('animtree', 'animnodesequence', 'animnodeblend', 'animnoderandom'):
                raise ValueError('Unsupported source animation node: '+row['name']+' / '+row['cls'])
            visiting.remove(ref)

        visit(root)
        assert set(self.animation) <= slots, ('Matinee slot not present in exact source tree', set(self.animation)-slots)
        self.evaluated, self.default_leaves = set(), set()
        self.last_signature, self.last_pose = None, None

    def _mask(self, row, mask):
        assert not mask.get('weightrulelist') and not mask.get('bweightbasedonnoderules')
        assert not mask.get('bdisablefornonlocalhumanplayers')
        assert float(mask.get('blendtimetogo', 0.)) == 0.
        weight = float(mask.get('desiredweight', 1.))
        assert math.isfinite(weight) and 0 <= weight <= 1
        output, missing = np.zeros(len(self.model.skeleton_bones)), []
        for branch in mask.get('branchlist', []):
            assert float(branch['perboneweightincrease']) == 1., ('Unsupported graduated bone mask', row['name'])
            root = self.palette.get(branch['bonename'].casefold())
            if root is None:
                missing.append(branch['bonename'])
                continue
            for i in range(len(output)):
                index, seen = i, set()
                while index >= 0:
                    assert index not in seen, 'Cyclic skeleton hierarchy'
                    seen.add(index)
                    if index == root:
                        output[i] = weight
                        break
                    index = self.model.skeleton_bones[index].parent
        self.mask_receipt.append(dict(nodeExportIndex0=row['index']-1, boneCount=int(np.count_nonzero(output)),
                                      sourceMask=mask, absentSourceBranches=missing))
        return output

    def sample(self, seconds):
        # Source loops frequently hold a trimmed endpoint for tens of seconds.
        # Reuse only an exactly identical set of slot inputs; this introduces
        # no epsilon reduction and never freezes an observed playing default.
        signature = []
        for slot_name, track in sorted(self.animation.items()):
            if not track.get('animseqs'):
                signature.append((slot_name, None))
                continue
            clip, age = base.anim_at(track, self.clips, seconds)
            alpha = float(np.clip(base.curve(track.get('floattrack', {}).get('points', []), seconds, 1.), 0., 1.))
            signature.append((slot_name, clip.name, age, alpha))
        signature = tuple(signature)
        if not self.default_leaves and signature == self.last_signature:
            return self.last_pose
        cache, poses = {}, {}

        def sample_clip(clip, age):
            key = clip.name, float(age)
            if key not in poses:
                poses[key] = base.pose_sample(self.model, clip, age)
            return poses[key]

        def evaluate(ref):
            if ref in cache:
                return cache[ref]
            self.evaluated.add(ref)
            row = self.rows[ref]; p = row['p']; children = p.get('children', [])
            kind = row['cls']
            if kind == 'animtree':
                assert len(children) == 1
                pose = evaluate(children[0]['anim'])
            elif kind == 'animnodeslot':
                track = self.animation.get(p['nodename'].casefold())
                if not track or not track.get('animseqs'):
                    pose = evaluate(children[0]['anim'])
                else:
                    alpha = float(np.clip(base.curve(track.get('floattrack', {}).get('points', []), seconds, 1.), 0., 1.))
                    if alpha == 0:
                        pose = evaluate(children[0]['anim'])
                    else:
                        clip, age = base.anim_at(track, self.clips, seconds)
                        pose = sample_clip(clip, age)
                        if alpha != 1:
                            pose = blend(evaluate(children[0]['anim']), pose, alpha)
            elif kind == 'animnode_multiblendperbone':
                pose = evaluate(children[0]['anim'])
                for child, weights in zip(children[1:], self.masks[ref]):
                    if np.any(weights):
                        pose = _masked_blend(pose, evaluate(child['anim']), weights)
            elif kind == 'animnodeblend':
                assert len(children) == 2
                weights = [float(c.get('weight', 0.)) for c in children]
                assert all(math.isfinite(w) and w >= 0 for w in weights) and abs(sum(weights)-1) < 1e-6
                if weights[1] == 0:
                    pose = evaluate(children[0]['anim'])
                elif weights[0] == 0:
                    pose = evaluate(children[1]['anim'])
                else:
                    pose = blend(evaluate(children[0]['anim']), evaluate(children[1]['anim']), weights[1])
            elif kind == 'animnoderandom':
                # A fully weighted Matinee slot normally masks these source
                # defaults. Do not replace an observable random clip with rest.
                assert len(children) == 1 and not p.get('randominfo'), ('Observable random source node', row['name'], seconds)
                pose = evaluate(children[0]['anim'])
            elif kind == 'animnodesequence':
                name = p['animseqname']
                assert name in self.clips, ('Missing observable source default clip', name, seconds)
                clip = self.clips[name]
                age = float(p.get('currenttime', 0.))
                if p.get('bplaying', False):
                    age += seconds*float(p.get('rate', 1.))
                duration = clip.duration_ticks/clip.ticks_per_second
                age = age % duration if p.get('blooping', False) else min(duration, max(0., age))
                self.default_leaves.add(name)
                pose = sample_clip(clip, age)
            else:
                raise ValueError('Unsupported source node '+kind)
            cache[ref] = pose
            return pose

        result = evaluate(self.root)
        self.last_signature, self.last_pose = signature, result
        return result

    def receipt(self):
        return dict(treeExportIndex0=self.root-1, reachableNodes=len(self.reachable),
                    evaluatedNodes=len(self.evaluated), slots=sorted(self.animation),
                    emptySourceSlots=sorted(k for k, v in self.animation.items() if not v.get('animseqs')),
                    masks=self.mask_receipt, observedDefaultClips=sorted(self.default_leaves))


def sample_times(tracks, duration_ms):
    end = duration_ms/1000.
    result = {i/60. for i in range(math.ceil(end*60))} | {end}
    for track in tracks:
        if track['cls'] not in ('interptrackanimcontrol', 'interptrackskelcontrolstrength',
                                'efinterptrackskelcontrolvector', 'efinterptrackskelcontrolmulti'):
            continue
        for value in track['p'].values():
            points = value.get('points', []) if isinstance(value, dict) else value if isinstance(value, list) else []
            for point in points:
                if not isinstance(point, dict):
                    continue
                seconds = point.get('inval', point.get('starttime'))
                if isinstance(seconds, (int, float)) and 0 <= seconds <= end:
                    result.add(float(seconds))
                    if seconds and (point.get('interpmode') == 'cim_constant' or 'starttime' in point):
                        result.add(max(0., float(seconds)-.0001))
    return sorted(result)
