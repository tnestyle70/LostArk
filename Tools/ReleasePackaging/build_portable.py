from __future__ import annotations

import argparse
import datetime as dt
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import zipfile

SCRIPT_ROOT = Path(__file__).resolve().parent
ROOT = SCRIPT_ROOT.parents[1]
OUTPUT_ROOT = ROOT / 'out/ReleasePackaging'
HOST = '192.168.0.22'
MODULES = ('Client.exe', 'Engine.dll', 'assimp-vc143-mt.dll', 'fmod.dll',
           'PhysX_64.dll', 'PhysXCommon_64.dll', 'PhysXFoundation_64.dll')
# These current consumers enumerate whole domains at runtime, including class/event
# selection, UI layouts and effect group/catalog admission. Reference/Navigation/
# Maps authoring is NOT copied wholesale: source literals and JSON references below
# include the exact direct read dependencies.
DIRECT_DOMAINS = ('Actors', 'Animation/Authored', 'Animation/HitShapes', 'Balance',
                  'Camera', 'Compositions', 'Customizing', 'Effects', 'Encounters',
                  'Guide', 'Items', 'KoukuSaydon', 'Rendering/Authored', 'Sound',
                  'Titles', 'UI', 'Valtan', 'Vehicles', 'Worlds')
DATA_SUFFIXES = {'.json', '.animevents'}
TRANSIENT = re.compile(r'(^|\.)(staging|rollback|backup|before|tmp|temp)(\.|$)', re.I)


def read(path):
    return json.loads(Path(path).read_text(encoding='utf-8-sig'))


def write(path, obj):
    Path(path).write_text(json.dumps(obj, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')


def digest(path):
    with Path(path).open('rb') as stream:
        return hashlib.file_digest(stream, 'sha256').hexdigest()


def eligible(path):
    return not any(TRANSIENT.search(p) or p.startswith('.') for p in path.parts) and path.suffix.lower() not in {
        '.log', '.tlog', '.pdb', '.lastbuildstate', '.lock', '.previous', '.bak', '.old'}


def protocol(root=ROOT):
    text = (root / 'Shared/Public/Network/PacketType.h').read_text(encoding='utf-8-sig')
    return int(re.search(r'NETWORK_PROTOCOL_VERSION\s*=\s*(\d+)', text).group(1))


def data_revisions(root=ROOT):
    action = read(root / 'Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json')['revision']
    encounter = read(root / 'Data/Encounters/KoukuSaydon/KoukuSaydonEncounter.json')['sourceRevision']
    presentation = read(root / 'Data/Animation/Authored/KoukuSaydon/KoukuSaydon.patternbindings.json')['sourceRevision']
    sequence = read(root / 'Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json')['revision']
    rows = [line.split('\t') for line in (root / 'Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap').read_text(encoding='utf-8-sig').splitlines()]
    products = [int(row[3]) for row in rows if len(row) == 4 and row[0] == 'KOUKUSAYDONPRODUCTREVISION' and row[1] == 'ENCOUNTER_KAKULSAYDON_G1']
    gates = [int(row[5]) for row in rows if len(row) > 5 and row[0] == 'RAIDGATE' and row[1] == 'ENCOUNTER_KAKULSAYDON_G1']
    assert products and action > 0 and action == encounter == presentation and all(p == action for p in products), 'Kouku source/Product mismatch'
    assert sequence > 0 and len(gates) == 4 and all(g == sequence for g in gates), 'Kouku Sequence/Server mismatch'
    return dict(sourceRevision=action, sequenceRevision=sequence)


def strings(value):
    if isinstance(value, str):
        yield value
    elif isinstance(value, dict):
        for v in value.values():
            yield from strings(v)
    elif isinstance(value, list):
        for v in value:
            yield from strings(v)


def collect(root=ROOT):
    root = root.resolve()
    files = {}
    reasons = {}

    def add(path, reason):
        path = path.resolve()
        assert path.is_relative_to(root), str(path)
        relative = path.relative_to(root).as_posix()
        assert '/Resources/' not in relative and path.suffix.lower() != '.png', relative
        assert not path.is_symlink(), relative
        if path.is_file() and eligible(path.relative_to(root)):
            files[relative] = path
            reasons.setdefault(relative, reason)

    for name in MODULES:
        path = root / 'Client/Bin/Release' / name
        assert path.is_file(), str(path)
        add(path, 'Product module')
    add(root / 'Server/Bin/Release/Server.exe', 'Product module')
    assert 'Server/Bin/Release/Server.exe' in files
    shaders = list((root / 'Client/Bin/Release').glob('*.cso'))
    assert shaders, 'No compiled product shaders'
    for path in shaders:
        add(path, 'Compiled product shader')
    for folder in ('Client/Bin/DataFiles', 'Server/Bin/DataFiles'):
        for path in (root / folder).rglob('*'):
            if path.is_file() and eligible(path.relative_to(root)):
                add(path, 'Published runtime tree')
    for folder in DIRECT_DOMAINS:
        for path in (root / 'Data' / folder).rglob('*'):
            if path.is_file() and path.suffix.lower() in DATA_SUFFIXES:
                add(path, 'Current runtime domain ' + folder)
    # Area-based consumers compose these names dynamically. Include only their
    # direct source families, not authoring geometry, nav paint or raw references.
    for path in (root / 'Data/Maps/Authoring').rglob('*.json'):
        if path.name.endswith(('.worldsequences.json', '.camerashots.json', '.maplights.json', '.mapmotions.json')):
            add(path, 'Dynamic Area runtime source family')

    # Read the actual current source, so newly added UI/catalog direct consumers
    # cannot be omitted merely because the previous ZIP did not contain them.
    literals = []
    for folder in ('Client', 'Server', 'Engine', 'Shared'):
        for path in (root / folder / 'Private').rglob('*.cpp'):
            content = path.read_bytes().decode('utf-8-sig', errors='replace')
            literals.extend(re.findall(r'"([^"\r\n]{3,260}\.(?:json|animevents))"', content))
        for path in (root / folder / 'Public').rglob('*.h'):
            content = path.read_bytes().decode('utf-8-sig', errors='replace')
            literals.extend(re.findall(r'"([^"\r\n]{3,260}\.(?:json|animevents))"', content))

    def resolve(value, parent=None):
        value = value.replace('\\\\', '/').replace('\\', '/')
        if ':' in value or '*' in value or '%' in value or '\n' in value:
            return []
        options = [root / value, root / 'Data' / value]
        if parent:
            options.append(parent / value)
        found = []
        for candidate in options:
            candidate = candidate.resolve()
            if candidate.is_relative_to(root / 'Data') and candidate.is_file() and candidate.suffix.lower() in DATA_SUFFIXES:
                found.append(candidate)
        return found

    for literal in literals:
        for path in resolve(literal):
            add(path, 'Current C++ direct path literal')

    inspected = set()
    while True:
        pending = [(relative, path) for relative, path in files.items() if path.suffix.lower() == '.json' and relative not in inspected]
        if not pending:
            break
        for relative, path in pending:
            inspected.add(relative)
            document = read(path)
            for value in strings(document):
                if not value.lower().endswith(('.json', '.animevents')):
                    continue
                for linked in resolve(value, path.parent):
                    add(linked, 'JSON reference from ' + relative)
    # Catalog admission is fail-close: every direct authoring input must exist.
    catalog = read(root / 'Data/Effects/EffectCatalog.json')
    for value in strings(catalog):
        if value.startswith(('Data/', 'Effects/')) and value.endswith('.json'):
            paths = resolve(value)
            assert paths and all(p.relative_to(root).as_posix() in files for p in paths), 'Missing Effect catalog document: ' + value
    return files, reasons


def validate_numeric_sources(files, root=ROOT):
    mapping_path = root / 'Data/Balance/NumericSourceBindings.json'
    assert mapping_path.is_file(), 'Generate NumericSourceBindings after the final publish'
    mapping = read(mapping_path)
    assert mapping['schema'] == 'lostark.numeric-source-bindings' and mapping['formatVersion'] == 1
    for source in mapping['files']:
        relative = source['path']
        assert relative in files and digest(root / relative) == source['sha256'], 'Numeric source binding is stale/missing: ' + relative
    return len(mapping['entries'])


def compile_launcher(destination, revision):
    destination = Path(destination)
    destination.mkdir(parents=True, exist_ok=True)
    contract = destination / 'BundleContract.cs'
    contract.write_text('internal static class BundleContract { public const int Protocol = %d; public const string Host = "%s"; public const string Endpoint = "%s:7777"; }\n' % (revision, HOST, HOST), encoding='utf-8')
    compiler = Path(os.environ.get('WINDIR', r'C:\Windows')) / 'Microsoft.NET/Framework64/v4.0.30319/csc.exe'
    output = destination / 'LostArk.exe'
    subprocess.run([str(compiler), '/nologo', '/target:winexe', '/platform:x64', '/optimize+',
                    '/r:System.Windows.Forms.dll', '/r:System.Web.Extensions.dll', '/out:' + str(output),
                    str(SCRIPT_ROOT / 'PortableLauncher.cs'), str(contract)], check=True)
    return output


def crt_files(explicit=None):
    if explicit:
        directory = Path(explicit)
    else:
        vswhere = Path(os.environ.get('ProgramFiles(x86)', r'C:\Program Files (x86)')) / 'Microsoft Visual Studio/Installer/vswhere.exe'
        install = subprocess.check_output([str(vswhere), '-latest', '-products', '*', '-prerelease', '-requires', 'Microsoft.VisualStudio.Component.VC.Tools.x86.x64', '-property', 'installationPath'], text=True).strip()
        candidates = list((Path(install) / 'VC/Redist/MSVC').glob('*/x64/Microsoft.VC*.CRT'))
        assert candidates, 'VC redistributable directory missing; specify --crt-root'
        directory = max(candidates, key=lambda p: tuple(int(n) for n in re.findall(r'\d+', p.parts[-3])))
    result = list(directory.glob('*.dll'))
    assert {'msvcp140.dll', 'vcruntime140.dll', 'vcruntime140_1.dll'} <= {p.name.lower() for p in result}
    return result


def validate_build(path):
    build = read(path)
    assert build.get('schema') == 'lostark.compile-result' and build.get('configuration') == 'Release'
    assert build.get('result') == 'PASS' and not build.get('skippedBuild')
    assert not build.get('missingRuntimeInputs') and not build.get('invalidRuntimeInputs')
    return build


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--stage', type=Path)
    parser.add_argument('--output-zip', type=Path)
    parser.add_argument('--build-receipt', type=Path)
    parser.add_argument('--crt-root', type=Path)
    parser.add_argument('--plan-only', action='store_true')
    args = parser.parse_args()
    OUTPUT_ROOT.mkdir(parents=True, exist_ok=True)
    files, reasons = collect()
    numeric_fields = validate_numeric_sources(files)
    revisions = data_revisions()
    plan = dict(numericSourceFields=numeric_fields, files=len(files), directDataFiles=sum(p.startswith('Data/') for p in files),
                compiledShaders=sum(p.endswith('.cso') for p in files), resourcesIncluded=0,
                uncompressedPayloadBytes=sum(p.stat().st_size for p in files.values()), protocol=protocol(), dataRevisions=revisions)
    if args.plan_only:
        write(OUTPUT_ROOT / 'payload-plan.json', dict(**plan, paths=reasons))
        print(json.dumps(plan)); return
    assert args.stage and args.output_zip and args.build_receipt, 'Provide --stage --output-zip --build-receipt'
    validate_build(args.build_receipt)
    stage = args.stage.resolve(); output = args.output_zip.resolve()
    assert stage.is_relative_to(ROOT / 'out'), 'Stage must remain in task output folder'
    assert not stage.exists(), 'Use a fresh stage directory'
    assert output.suffix.lower() == '.zip' and output.parent == ROOT.parent, 'Only the requested Desktop ZIP area is supported'
    partial = output.with_suffix('.partial.zip')
    assert not partial.exists(), 'Preserve previous partial archives'
    previous = digest(output) if output.exists() else None
    stage.mkdir(parents=True)
    (stage / 'Client/Default').mkdir(parents=True)
    (stage / 'Server/Default').mkdir(parents=True)
    rows = []
    def copy(source, relative, origin):
        target = stage / relative; target.parent.mkdir(parents=True, exist_ok=True)
        before = digest(source)
        shutil.copy2(source, target)
        assert digest(source) == digest(target) == before, 'File changed while staging: ' + str(source)
        rows.append(dict(path=relative, bytes=target.stat().st_size, sha256=before, sourcePath=origin))
    for relative, source in sorted(files.items()):
        copy(source, relative, relative)
    for source in crt_files(args.crt_root):
        for module in ('Client', 'Server'):
            copy(source, module + '/Bin/Release/' + source.name, str(source))
    launcher = compile_launcher(OUTPUT_ROOT / ('compiled-' + str(protocol())), protocol())
    copy(launcher, 'LostArk.exe', str(launcher))
    copy(ROOT / 'Tools/Network/Collect-RuntimeDiagnostics.ps1', 'Collect-RuntimeDiagnostics.ps1', 'Tools/Network/Collect-RuntimeDiagnostics.ps1')
    copy(SCRIPT_ROOT / 'ServerHost.cmd', 'ServerHost.cmd', str(SCRIPT_ROOT / 'ServerHost.cmd'))
    copy(SCRIPT_ROOT / 'README_실행방법.md', 'README_실행방법.md', str(SCRIPT_ROOT / 'README_실행방법.md'))
    copy(args.build_receipt, 'build-evidence.json', str(args.build_receipt))
    pins = {name: next(r for r in rows if r['path'] == path) for name, path in {
        'clientBinary': 'Client/Bin/Release/Client.exe', 'serverBinary': 'Server/Bin/Release/Server.exe', 'engineBinary': 'Client/Bin/Release/Engine.dll'}.items()}
    write(stage / 'release-ready.receipt.json', dict(status='PASS', protocol=protocol(), binaryPins=pins,
        gitHead=subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip(),
        buildReceiptSha256=digest(args.build_receipt), dataRevisions=revisions))
    ready = stage / 'release-ready.receipt.json'
    rows.append(dict(path=ready.name, bytes=ready.stat().st_size, sha256=digest(ready)))
    manifest = dict(schema='lostark.portable-runtime-bundle', formatVersion=1, bundleVersion=9,
        configuration='Release', protocol=protocol(), serverEndpoint=HOST + ':7777',
        resourcePolicy='external-only-no-install', createdAtUtc=dt.datetime.now(dt.timezone.utc).isoformat(),
        dataRevisions=revisions, binaryPins=pins, buildReceipt='build-evidence.json', counts=plan, files=rows)
    write(stage / 'bundle-manifest.json', manifest)
    assert revisions == data_revisions(stage) == data_revisions()
    # Recheck every current source before packaging. No stale source is silently frozen.
    for row in rows:
        if row.get('sourcePath'):
            source = Path(row['sourcePath']); source = source if source.is_absolute() else ROOT / source
            assert digest(source) == row['sha256'], 'Source changed before archive: ' + str(source)
    preflight = OUTPUT_ROOT / ('preflight-' + stage.name + '.json')
    subprocess.run([str(stage / 'LostArk.exe'), '--check', str(ROOT), str(preflight)], check=True, timeout=180)
    assert read(preflight)['status'] == 'PASS' and not read(preflight)['clientStarted']
    with zipfile.ZipFile(partial, 'w', compression=zipfile.ZIP_DEFLATED, compresslevel=6, allowZip64=True) as archive:
        for empty in ('Client/Default/', 'Server/Default/'):
            archive.writestr(empty, b'')
        for path in sorted(stage.rglob('*')):
            if path.is_file(): archive.write(path, path.relative_to(stage).as_posix())
    with zipfile.ZipFile(partial) as archive:
        assert archive.testzip() is None
        names = archive.namelist()
        assert len(names) == len(set(n.lower() for n in names))
        for row in rows:
            payload = archive.read(row['path'])
            assert len(payload) == row['bytes'] and hashlib.sha256(payload).hexdigest() == row['sha256']
        assert not any('/resources/' in n.lower() or n.lower().endswith(('.png', '.pdb')) for n in names)
    assert data_revisions() == revisions
    for relative, source in files.items():
        assert digest(source) == next(r['sha256'] for r in rows if r['path'] == relative), 'Source changed during archive: ' + relative
    backup = None
    if previous:
        assert digest(output) == previous, 'Previous ZIP changed during staging'
        backup = output.with_name(output.stem + '.backup-' + dt.datetime.now().strftime('%Y%m%d-%H%M%S-%f') + '.zip')
        shutil.copy2(output, backup)
        assert digest(backup) == digest(output) == previous
    else:
        assert not output.exists(), 'Destination appeared during staging'
    os.replace(partial, output)
    result = dict(status='PASS', output=str(output), bytes=output.stat().st_size, sha256=digest(output),
        previousBackup=str(backup) if backup else None, stage=str(stage), counts=plan,
        clientStarted=False, serverStarted=False, preflight=str(preflight))
    write(OUTPUT_ROOT / 'portable-delivery.receipt.json', result)
    print(json.dumps(result, ensure_ascii=False))


if __name__ == '__main__':
    main()
