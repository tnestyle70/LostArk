param(
    [string]$RepositoryRoot = (Resolve-Path (Join-Path $PSScriptRoot '../..')).Path,
    [string]$SourcePath = ''
)
$ErrorActionPreference = 'Stop'
$root = $RepositoryRoot
$out = Join-Path $root 'out/WorldObjectProvenanceContracts'
[IO.Directory]::CreateDirectory($out) | Out-Null
if (!$SourcePath) { $SourcePath = Join-Path $root 'Client/Private/WorldObjectTool.cpp' }
$path = $SourcePath
$source = [IO.File]::ReadAllText($path)
$start = $source.IndexOf('bool SynchronizeEmissionReferences(')
$end = $source.IndexOf('bool ResolveTransferredEffectDuration(', $start)
$methods = $source.Substring($start,$end-$start)
$doc = [IO.File]::ReadAllText((Join-Path $root 'Client/Private/WorldSequenceDocument.cpp')).Replace("`r`n","`n")
$start = $doc.IndexOf("Client::WORLD_SEQUENCE_TEMPLATE*`nClient::CWorldSequenceDocument::Find_Template")
$end = $doc.IndexOf('Client::WORLD_SEQUENCE_OBJECT_FOLDER* Client::CWorldSequenceDocument::Find_ObjectFolder', $start)
$findMethods = $doc.Substring($start,$end-$start)
$fixture = @'
#include "WorldSequenceDocument.h"
#include "KoukuSaydonCompositionDocument.h"
#include <algorithm>
#include <cmath>
#include <set>
#include <map>
#include <iostream>
// FIND_METHODS
// SYNC_METHOD
int main() {
    using namespace Client;
    int checks=0,failures=0;
    const auto check=[&](bool good,const char* name){++checks;if(!good){++failures;std::cerr<<"FAIL "<<name<<'\n';}};
    CWorldSequenceDocument before;
    WORLD_SEQUENCE_TEMPLATE motion; motion.sequenceId="motion"; motion.displayName="Motion"; motion.objectMotion.count=1;
    before.Get_Templates().push_back(motion);
    WORLD_SEQUENCE_INSTANCE instance; instance.instanceId="instance"; instance.templateId="motion";
    before.Get_Instances().push_back(instance);
    auto after=before; after.Get_Templates().front().objectMotion.emissions.resize(2);
    KOUKU_SAYDON_COMPOSITION_DOCUMENT composition;
    KOUKU_SAYDON_COMPOSITION_WORLD_DEFINITION world; world.strWorldId="world"; world.strSequenceInstanceId="instance";
    composition.Worlds.push_back(world);
    KOUKU_SAYDON_COMPOSITION_PATTERN pattern; pattern.strPatternId="pattern";
    KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE box; box.strOccurrenceId="world-box"; box.strWorldId="world"; box.iDurationMs=10000;
    pattern.WorldOccurrences.push_back(box); composition.Patterns.push_back(pattern);
    std::map<std::string,std::vector<uint32_t>> provenance{{"motion",{0,UINT32_MAX}}};
    const std::set<std::string> edited{"motion"}; bool references=false;std::string status;
    const auto plain=composition;
    check(SynchronizeEmissionReferences(composition,before,after,provenance,edited,references,status),"no-linked-emission restoration permitted");
    check(composition==plain,"no-linked composition retained");
    KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE resource; resource.strResourceId="collider"; resource.eKind=KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER;
    composition.PresentationResources.push_back(resource);
    KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE collider; collider.strOccurrenceId="collider-box"; collider.strResourceId="collider";
    collider.strWorldId="world"; collider.strWorldOccurrenceId="world-box"; collider.strAnchorKind="WORLD"; collider.iDurationMs=100;
    composition.Patterns.front().PresentationOccurrences.push_back(collider);
    const auto linked=composition;
    check(!SynchronizeEmissionReferences(composition,before,after,provenance,edited,references,status),"missing saved linked provenance refused");
    check(composition==linked,"refusal preserves linked rows");
    provenance["motion"]={0,0};
    check(SynchronizeEmissionReferences(composition,before,after,provenance,edited,references,status),"valid clone provenance allowed");
    check(composition.Patterns.front().PresentationOccurrences.size()==2,"valid clone keeps separate linked colliders");
    composition=linked; composition.Patterns.front().PresentationOccurrences.front().strWorldOccurrenceId.clear();
    box.strOccurrenceId="second-box";composition.Patterns.front().WorldOccurrences.push_back(box);
    check(!SynchronizeEmissionReferences(composition,before,after,provenance,edited,references,status),"ambiguous colliders still refused");
    composition=linked; composition.Patterns.front().PresentationOccurrences.front().strWorldId="unrelated";
    provenance["motion"]={0,UINT32_MAX};
    check(SynchronizeEmissionReferences(composition,before,after,provenance,edited,references,status),"unrelated colliders do not require provenance");
    std::cout<<"checks="<<checks<<" failures="<<failures<<'\n';return failures?1:0;
}
'@
$fixture=$fixture.Replace('// FIND_METHODS',$findMethods).Replace('// SYNC_METHOD',$methods)
$utf8=[Text.UTF8Encoding]::new($false)
$cpp=Join-Path $out 'WorldProvenance.cpp'
[IO.File]::WriteAllText($cpp,$fixture,$utf8)
$args=@('/nologo','/EHsc','/std:c++20','/utf-8','/W3','/permissive-','/MDd','/D_DEBUG','/D_UNICODE','/DUNICODE','/D_WINDOWS',('/I"'+$root+'/Client/Public"'),('/I"'+$root+'/EngineSDK/Inc"'),('/I"'+$root+'/Shared/Public"'),('/Fo"'+$out+'/"'),('/Fe"'+$out+'/WorldProvenance.exe"'),('"'+$cpp+'"'),'/link','/INCREMENTAL:NO')
$rsp=Join-Path $out 'compile.rsp';[IO.File]::WriteAllText($rsp,($args -join "`r`n"),$utf8)
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
$vs = & $vswhere -latest -prerelease -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (!$vs) { throw 'Visual C++ Build Tools are required.' }
$vcvars = Join-Path $vs 'VC/Auxiliary/Build/vcvars64.bat'
$cmd='call "'+$vcvars+'" >nul && cl.exe @"'+$rsp+'"'
& $env:COMSPEC /d /s /c $cmd *> (Join-Path $out 'compile.log')
if($LASTEXITCODE){Get-Content (Join-Path $out 'compile.log') -Tail 30;throw 'provenance probe compile failed'}
& (Join-Path $out 'WorldProvenance.exe') | Tee-Object -FilePath (Join-Path $out 'run.log')
if($LASTEXITCODE){throw 'provenance probe failed'}
