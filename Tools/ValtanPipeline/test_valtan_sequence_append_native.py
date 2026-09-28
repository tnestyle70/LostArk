#!/usr/bin/env python3
"""Execute the real Valtan Resource Sequence append function without Client/UI.

The production function and source-window resolver are compiled unchanged.
Owner endpoints are narrow test doubles; source cuts and cooked ticks come from
this checkout's actual reference documents and installed Valtan WModels.
"""
from pathlib import Path
import sys, re, json, math, os, subprocess, tempfile, shutil, unittest

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT))
from Tools.ValtanPipeline.valtan_native_animation_inventory import (
    NativeAnimationInventoryError, load_valtan_composite_animation_inventory,
)


def toolchain(out):
    env={key.upper(): value for key, value in os.environ.items()}
    cl=shutil.which("cl.exe")
    if cl:
        return Path(cl),env
    vswhere=Path(os.environ.get("ProgramFiles(x86)", "C:/Program Files (x86)"))/"Microsoft Visual Studio/Installer/vswhere.exe"
    if not vswhere.is_file():
        raise unittest.SkipTest("MSVC toolchain is not installed")
    found=subprocess.run([str(vswhere),"-latest","-prerelease","-products","*","-requires","Microsoft.VisualStudio.Component.VC.Tools.x86.x64","-property","installationPath"],capture_output=True,check=True,text=True).stdout.strip()
    if not found:
        raise unittest.SkipTest("MSVC C++ component is not installed")
    script=out/"environment.cmd"
    script.write_text('@echo off\ncall "'+str(Path(found)/"Common7/Tools/VsDevCmd.bat")+'" -arch=x64 -host_arch=x64 >nul\nset\n',encoding="utf-8")
    setup=subprocess.run(["cmd.exe","/d","/c",str(script)],capture_output=True,check=True)
    for line in setup.stdout.decode(errors="replace").splitlines():
        if "=" in line and not line.startswith("="):
            key,value=line.split("=",1);env[key.upper()]=value
    path=env.get("PATH", "")
    cl=shutil.which("cl.exe",path=path)
    if not cl:
        raise unittest.SkipTest("MSVC environment did not expose cl.exe")
    return Path(cl),env


def run_probe(OUT,cl,env):
    wb = (ROOT / 'Client/Private/ValtanActionWorkbench.cpp').read_text(encoding='utf-8')
    timeline = (ROOT / 'Client/Private/ActionPresentationTimeline.cpp').read_text(encoding='utf-8')
    def function(text, needle):
        at=text.index(needle); line=text.rfind('\n',0,at)+1
        indent=text[line:at]
        end=text.index('\n'+indent+'}',text.index('{',at))+len('\n'+indent+'}')
        return text[line:end]
    
    inventory = load_valtan_composite_animation_inventory(ROOT)
    cuts={}
    for line in (ROOT/'Data/Animation/Reference/Valtan/Valtan.clipcuts').read_text(encoding='utf-8').splitlines()[1:]:
        m=re.fullmatch(r'(\d+)\s+seq=(\d+)\s+cuts="([^"]*)"',line)
        cuts[(int(m[1]),int(m[2]))]=[math.floor(float(x)*1000+0.5) for x in m[3].split(',')]
    sequences={}
    diagnostics=[]
    for line in (ROOT/'Data/Animation/Reference/Valtan/Valtan.clipseq').read_text(encoding='utf-8').splitlines()[1:]:
        m=re.fullmatch(r'(\d+)\s+"[^"]*"\s+seq=(\d+)\s+mode=(\S+)\s+clips="([^"]*)"',line)
        key=(int(m[1]),int(m[2]))
        rows=[]
        for name,cut in zip(m[4].split(','),cuts[key]):
            if not cut: continue
            native=inventory.clips[name]
            cooked=math.floor(native.duration_ticks/30*1000+0.5)
            rows.append((name,cut,native.duration_ticks))
            if cut>cooked:
                diagnostics.append(dict(action=key[0],sequence=key[1],clip=name,cut=cut,cooked=cooked,importRate=native.ticks_per_second,explicitLoop='_loop' in name.lower()))
        sequences[key]=(m[3],rows)
    (OUT/'corpus.json').write_text(json.dumps(dict(sequenceCount=len(sequences),overrunCount=len(diagnostics),nonLoopOverrunCount=sum(not x['explicitLoop'] for x in diagnostics),overruns=diagnostics),indent=2),encoding='utf-8')
    
    code=r'''
    #include <string>
    #include <vector>
    #include <map>
    #include <set>
    #include <algorithm>
    #include <numeric>
    #include <cmath>
    #include <cstdint>
    #include <limits>
    #include <sstream>
    #include <iomanip>
    #include <iostream>
    #include <stdexcept>
    using bool_t=bool;
    namespace Client {
    inline constexpr std::size_t MAX_BOSS_PATTERN_ANIMATION_CLIPS=256u;
    struct CActionPresentationTimeline { static bool Validate_AuthoredSourceWindow(float,float,uint32_t,uint32_t,float,uint32_t&); };
    struct VALTAN_PATTERN_VIEW {std::string strPatternId="P";};
    struct VALTAN_STAGE_VIEW {std::string strStageId="S",strSequenceRole="ACTIVE";};
    enum class VALTAN_VIEW_ADMISSION {ADMITTED,STALE};
    struct CBalanceTool {
     struct ANIMATION_SLOT_EDIT {std::string clipOccurrenceId,clip,mappingBasis;uint32_t sourceStartMs=0,playMs=0;double playRate=1;bool repeatUntilStageEnd=false;};
     struct PATTERN_STAGE_EDIT {bool animationEditable=true;uint32_t durationMs=1200,animationRepeatCount=1;std::string animationEndPolicy="EXACT";std::vector<ANIMATION_SLOT_EDIT> animationSlots;};
     PATTERN_STAGE_EDIT saved; bool rejectCommit=false;
     bool Get_ValtanStageDraft(const std::string&,const std::string&,PATTERN_STAGE_EDIT& d,std::string&){d=saved;return true;}
    };
    struct CAnimation_Tool {
     struct COMPOSITION_SEQUENCE_CLIP_VIEW {std::string strClipName;uint32_t iDurationMs=0;};
     struct COMPOSITION_SEQUENCE_VIEW {std::string strStableId="selected",strMode="SEQUENCE";int iSkillId=420623,iSequenceIndex=1;std::vector<COMPOSITION_SEQUENCE_CLIP_VIEW> Clips;};
     std::map<std::string,float> native;
     bool Resolve_ValtanCompositionNativeClipDurationMs(const std::string& name,uint32_t& duration,std::string& status){auto n=native.find(name);if(n==native.end()){status="missing native";return false;}return CActionPresentationTimeline::Validate_AuthoredSourceWindow(n->second,30.f,0,0,1,duration)&&duration;}
    };
    struct CValtanActionWorkbench {
     enum class DETAIL_OWNER {ANIMATION};
     VALTAN_VIEW_ADMISSION m_eAdmission=VALTAN_VIEW_ADMISSION::ADMITTED;
     CBalanceTool* m_pBalanceTool; CAnimation_Tool* m_pAnimationTool;
     std::vector<CAnimation_Tool::COMPOSITION_SEQUENCE_VIEW> m_AnimationSequences;
     std::string m_strSelectedSequenceStableId="selected",m_strStatus,m_strSelectedStableId;
     bool m_bPatternShakesReady=false;int m_PatternShakes=0;DETAIL_OWNER m_eDetailOwner=DETAIL_OWNER::ANIMATION;
     unsigned invalidations=0;
     void Invalidate_TimelineCache(){++invalidations;}
     bool Apply_SelectedSequenceToStage(const VALTAN_PATTERN_VIEW&,const VALTAN_STAGE_VIEW&,bool);
    };
    }
    using namespace Client;
    std::string Lower(std::string text){for(char&c:text)c=char(std::tolower((unsigned char)c));return text;}
    bool SetValtanStageDraftWithSoundDependencyAdmission(CAnimation_Tool* a,CBalanceTool* b,int*,const VALTAN_PATTERN_VIEW&,const VALTAN_STAGE_VIEW&,CBalanceTool::PATTERN_STAGE_EDIT& d,std::string& status,uint32_t,uint32_t){
     if(b->rejectCommit){status="dependency rejection";return false;}
     std::set<std::string> ids;uint64_t duration=0;
     for(const auto&s:d.animationSlots){uint32_t native=0;if(!a->Resolve_ValtanCompositionNativeClipDurationMs(s.clip,native,status)||!s.playMs||s.playMs>native||s.repeatUntilStageEnd||!ids.insert(s.clipOccurrenceId).second){status="invalid finite occurrence";return false;}duration+=uint64_t(std::llround(s.playMs/s.playRate));}
     if(duration>d.durationMs||d.animationRepeatCount!=1){status="invalid wall budget";return false;}
     b->saved=d;return true;
    }
    '''
    code+=function(timeline,'bool Client::CActionPresentationTimeline::Validate_AuthoredSourceWindow(')+'\n'
    for needle in ['std::string BuildCompositionSlotId(', 'std::string BuildNextCompositionSlotId(', 'std::string ClipReplacementRole(', 'bool_t IsRoleAwareHoldReplacementChain(', 'bool_t FitCompositionSequenceCutsToStage(', 'bool_t Client::CValtanActionWorkbench::Apply_SelectedSequenceToStage(']:
        code+=function(wb,needle)+'\n'
    
    code+='\nvoid fill(CValtanActionWorkbench& w,int action,int index){\n w.m_AnimationSequences.clear(); CAnimation_Tool::COMPOSITION_SEQUENCE_VIEW s;\n'
    for (action,index),(mode,rows) in sequences.items():
        code+=f' if(action=={action}&&index=={index}){{s.iSkillId={action};s.iSequenceIndex={index};s.strMode="{mode}";\n'
        for name,cut,ticks in rows:
            code+=f' s.Clips.push_back({{{json.dumps(name)},{cut}}});w.m_pAnimationTool->native[{json.dumps(name)}]={ticks:.9f}f;\n'
        code+=' w.m_AnimationSequences.push_back(s);return;}\n'
    code+='throw std::runtime_error("unknown source fixture");}\n'
    code+=r'''
    int checks=0;void check(bool pass,const std::string& msg){++checks;if(!pass)throw std::runtime_error(msg);}
    int main(){try{CBalanceTool b;CAnimation_Tool a;CValtanActionWorkbench w;w.m_pBalanceTool=&b;w.m_pAnimationTool=&a;
     VALTAN_PATTERN_VIEW p;VALTAN_STAGE_VIEW s;
     fill(w,420623,1);b.saved.animationSlots={{"original","mesh_att_battle_21_01","BASE",0,1200,1,false},{"original2","mesh_att_battle_21_01","BASE",0,1300,1,false}};b.saved.durationMs=2500;
     check(w.Apply_SelectedSequenceToStage(p,s,true),w.m_strStatus);
     check(b.saved.animationSlots.size()==17,"existing2 plus15 source entries append exactly once");
     check(b.saved.animationSlots[0].clipOccurrenceId=="original"&&b.saved.animationSlots[0].playMs==1200,"existing slot preserved");
     check(b.saved.animationSlots[2].clip=="mesh_idle_battle_1"&&b.saved.animationSlots[2].playMs==2233,"67 cooked ticks is2233ms, package cut2333ms normalized");
     uint64_t expected=2500;for(const auto&c:w.m_AnimationSequences[0].Clips){uint32_t n=0;std::string st;a.Resolve_ValtanCompositionNativeClipDurationMs(c.strClipName,n,st);expected+=std::min(n,c.iDurationMs);}
     check(b.saved.durationMs==expected,"appended Stage clock equals actual native-bounded windows");
     check(b.saved.animationSlots[10].clip==b.saved.animationSlots[14].clip&&b.saved.animationSlots[10].clipOccurrenceId!=b.saved.animationSlots[14].clipOccurrenceId,"intentional duplicate source clips retain distinct IDs");
     check(w.m_strStatus.find("2333 -> 2233 ms")!=std::string::npos,"success explains exact native cut adjustment");
     check(w.m_strSelectedStableId==b.saved.animationSlots[2].clipOccurrenceId&&w.invalidations==1,"new box selected and timeline rebuilt");
     check(w.Apply_SelectedSequenceToStage(p,s,true),w.m_strStatus);check(b.saved.animationSlots.size()==32,"second append retains source multiplicity");
     std::set<std::string> ids;for(const auto&c:b.saved.animationSlots)ids.insert(c.clipOccurrenceId);check(ids.size()==32,"second append generates fresh stable IDs");
     auto before=b.saved;b.rejectCommit=true;check(!w.Apply_SelectedSequenceToStage(p,s,true),"failed downstream dependency rejected");check(b.saved.durationMs==before.durationMs&&b.saved.animationSlots.size()==before.animationSlots.size(),"failed downstream dependency preserves draft");b.rejectCommit=false;
     s.strSequenceRole="WAIT";check(!w.Apply_SelectedSequenceToStage(p,s,true),"WAIT still rejected");s.strSequenceRole="ACTIVE";
     b.saved=CBalanceTool::PATTERN_STAGE_EDIT{};b.saved.durationMs=1;
     fill(w,420610,2);check(w.Apply_SelectedSequenceToStage(p,s,true),w.m_strStatus);
     unsigned loops=0;uint64_t loopMs=0;for(const auto&c:b.saved.animationSlots)if(c.clip=="mesh_att_battle_8_01_loop"){++loops;loopMs+=c.playMs;}
     check(loops>1&&loopMs==5900,"explicit _loop retains exact5900ms finite materialization");
     b.saved=CBalanceTool::PATTERN_STAGE_EDIT{};b.saved.durationMs=18000;fill(w,420623,1);
     check(w.Apply_SelectedSequenceToStage(p,s,false),w.m_strStatus);check(b.saved.animationSlots.size()==15&&b.saved.durationMs==18000,"Replace imports same15 native-bounded slots and preserves Stage budget");
     std::cout<<"PASS "<<checks<<" checks: actual Apply_SelectedSequenceToStage/native source-window functions; actual source fixture, stable identity, repeated source clips, native clocks, explicit loops, failure preservation\n";
    }catch(const std::exception&e){std::cerr<<"FAIL "<<checks<<": "<<e.what()<<'\n';return 1;}}
    '''
    (OUT/'probe.cpp').write_text(code,encoding='utf-8')

    args=[str(cl),'/nologo','/std:c++20','/MDd','/EHsc','/utf-8','/Fo'+str(OUT)+'/', '/Fe'+str(OUT/'probe.exe'),str(OUT/'probe.cpp')]
    result=subprocess.run(args,env=env,cwd=OUT,capture_output=True)
    if result.returncode:
        raise AssertionError((result.stdout+result.stderr).decode(errors='replace'))
    result=subprocess.run([str(OUT/'probe.exe')],env=env,cwd=OUT,capture_output=True)
    if result.returncode:
        raise AssertionError((result.stdout+result.stderr).decode(errors='replace'))
    return (result.stdout+result.stderr).decode(errors='replace')


class ValtanSequenceAppendNative(unittest.TestCase):
    @unittest.skipUnless(os.name=="nt", "Native MSVC regression runs on Windows")
    def test_actual_resource_append_with_installed_cooked_animation(self):
        try:
            load_valtan_composite_animation_inventory(ROOT)
        except (NativeAnimationInventoryError, OSError) as error:
            self.skipTest("Installed Valtan native resources unavailable: "+str(error))
        with tempfile.TemporaryDirectory(prefix="LostArkSequenceAppend-") as temp:
            out=Path(temp)
            cl,env=toolchain(out)
            output=run_probe(out,cl,env)
            self.assertIn("PASS 18 checks",output)


if __name__=="__main__":
    unittest.main()
