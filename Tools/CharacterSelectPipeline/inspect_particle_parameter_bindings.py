"""Join original typed PSC instance parameters to exact Cascade distributions."""
from pathlib import Path
import argparse
import json
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT/'Tools/EffectPipeline'))
from build_kouku_action_effect_groups import restored_index
import build_imported_effect_documents as imported


def untag(value):
    if isinstance(value, dict) and 'type' in value and 'value' in value:
        return untag(value['value'])
    if isinstance(value, dict):
        return {k: untag(v) for k, v in value.items()}
    if isinstance(value, list):
        return [untag(v) for v in value]
    return value


def bindings(index, system, parameter_name):
    result = []
    for key, obj in index.objects.items():
        if not key.startswith(system+'.') or 'particleparameter' not in obj.class_name.lower():
            continue
        props = untag(obj.properties)
        if str(props.get('parametername', '')).casefold() != parameter_name.casefold():
            continue
        expected = 'PSPT_SCALAR' if 'float' in obj.class_name.lower() else 'PSPT_VECTOR' if 'vector' in obj.class_name.lower() else 'UNKNOWN'
        owners = []
        for owner_key, owner in index.objects.items():
            if not owner_key.startswith(system+'.') or 'particlemodule' not in owner.class_name.lower():
                continue
            for field, target in owner.reference_paths:
                if target == key:
                    users = [dict(lod=lod_key, emitter=lod_key.rsplit('.', 1)[0], property=slot)
                             for lod_key, lod in index.objects.items()
                             if lod_key.startswith(system+'.') and lod.class_name.lower() == 'particlelodlevel'
                             for slot, module in lod.reference_paths if module == owner_key]
                    owners.append(dict(module=owner_key, moduleClass=owner.class_name, propertyPath=field,
                                       moduleProperties=untag(owner.properties), moduleUsers=users))
        result.append(dict(distribution=key, distributionClass=obj.class_name,
                           effectiveDistribution=props, expectedParameterType=expected, modules=owners))
    return result


def inspect(evidence, instances):
    index = restored_index(evidence)
    output = []
    for instance in instances:
        for raw in instance.get('instanceParameters', []):
            parameter = untag(raw)
            name = str(parameter.get('name', ''))
            kind = str(parameter.get('paramtype', 'PSPT_NONE')).upper()
            field = {'PSPT_SCALAR': 'scalar', 'PSPT_SCALAR_RAND': 'scalar',
                     'PSPT_VECTOR': 'vector', 'PSPT_VECTOR_RAND': 'vector', 'PSPT_COLOR': 'color'}.get(kind)
            found = bindings(index, instance['sourceSystem'], name)
            for binding in found:
                binding['declaredTypeMatchesDistribution'] = kind == binding['expectedParameterType']
                binding['fallbackConstant'] = binding['effectiveDistribution'].get('constant', 0 if binding['expectedParameterType']=='PSPT_SCALAR' else dict(x=0,y=0,z=0))
            output.append(dict(**{k:v for k,v in instance.items() if k!='instanceParameters'},
                parameterName=name, instanceParameterType=kind, instanceParameter=parameter,
                activeTypedField=field, activeTypedValue=parameter.get(field) if field else None,
                bindings=found, matchingRule='Exact source system and parameter FName; serialized ParamType selects the active union field. A populated inactive vector/color field is not an override.'))
    return output


if __name__ == '__main__':
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--source-evidence', required=True, type=Path)
    p.add_argument('--instances', required=True, type=Path)
    p.add_argument('--output', required=True, type=Path)
    a = p.parse_args()
    if not a.output.resolve().is_relative_to((ROOT/'out').resolve()):
        raise ValueError('Inspection output must remain beneath workspace/out')
    rows = inspect(a.source_evidence, json.loads(a.instances.read_text(encoding='utf-8-sig')))
    a.output.parent.mkdir(parents=True, exist_ok=True)
    a.output.write_text(json.dumps(rows, indent=2, ensure_ascii=False)+'\n', encoding='utf-8')
    print(json.dumps(dict(instanceParameters=len(rows), bound=sum(bool(r['bindings']) for r in rows),
        typeMismatches=sum(not b['declaredTypeMatchesDistribution'] for r in rows for b in r['bindings']))))
