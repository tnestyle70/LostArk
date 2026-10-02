"""Compile production Animation/Channel/Bone code with real DirectXMath.

Only application/profiler plumbing and unrelated asset types are stubbed. The
test exposes private fields to inspect cache ownership and copies production
method bodies unchanged; the optional collision run changes only the registry
bucket selection. This is CPU pose evidence, not a rendered Client/FPS test.
"""
from pathlib import Path
import argparse
import hashlib
import json
import struct
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]
SOURCES = ['Engine/Public/Animation.h', 'Engine/Private/Animation.cpp',
           'Engine/Public/Channel.h', 'Engine/Private/Channel.cpp',
           'Engine/Public/Bone.h', 'Engine/Private/Bone.cpp']

DEFINES = r'''
#pragma once
#include <Windows.h>
#undef min
#undef max
#include <DirectXMath.h>
#include <algorithm>
#include <array>
#include <atomic>
#include <bit>
#include <cmath>
#include <cstring>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>
#define NS_BEGIN(x) namespace x {
#define NS_END }
#define MSG_BOX(x) throw std::runtime_error(x)
using namespace DirectX;
using namespace std;
namespace Engine {
using bool_t=bool; using char_t=char; using f32_t=float;
using float3_t=XMFLOAT3; using float4_t=XMFLOAT4; using float4x4_t=XMFLOAT4X4;
using matrix_t=XMMATRIX; using fmatrix_t=FXMMATRIX; using vector_t=XMVECTOR;
inline std::atomic<unsigned> reuseHits{0};
}
using namespace Engine;
'''

PROBE = r'''
#include "Animation.cpp"
#include "Channel.cpp"
#include "Bone.cpp"
#include <fstream>
#include <iostream>
#include <thread>
#include <chrono>
#include <stdexcept>

struct CMesh {
    uint32_t m_iNumBones=0;
    vector<float4x4_t> m_OffsetMatrices;
    vector<uint32_t> m_BoneIndices;
    void Build_SkinPalette(const vector<shared_ptr<CBone>>&,float4x4_t*)const;
};
/*PRODUCTION_SKIN_PALETTE*/

unsigned checks=0;
void require(bool value,const char* label) { ++checks; if(!value) throw std::runtime_error(label); }
void requireMatrix(FXMMATRIX a,FXMMATRIX b,const char* label) {
    XMFLOAT4X4 x,y;XMStoreFloat4x4(&x,a);XMStoreFloat4x4(&y,b);
    require(std::memcmp(&x,&y,sizeof(x))==0,label);
}
using Bones=vector<shared_ptr<CBone>>;
Bones makeBones(unsigned count=4) {
    Bones bones;
    for(unsigned i=0;i<count;++i) { MODEL_BONE_DATA b;b.name="bone"+to_string(i);
        b.parentIndex=i ? int(i)-1 : -1;XMStoreFloat4x4(&b.restLocal,XMMatrixTranslation(float(i),.5f,0));
        bones.push_back(CBone::Create(b)); }
    return bones;
}
Bones cloneBones(const Bones& src) { Bones out;for(const auto& b:src)out.push_back(b->Clone());return out; }
void compare(const Bones& a,const Bones& b) {
    require(a.size()==b.size(),"bone count");
    for(size_t i=0;i<a.size();++i) {
        requireMatrix(a[i]->Get_TransformationMatrix(),b[i]->Get_TransformationMatrix(),"local matrix differs");
        requireMatrix(a[i]->Get_CombinedTransformationMatrix(),b[i]->Get_CombinedTransformationMatrix(),"combined matrix differs");
    }
}
void combine(Bones& bones,float scale) {
    for(const auto& bone:bones)bone->Update_CombinedTransformationMatrix(bones,XMMatrixScaling(scale,scale,scale));
}
MODEL_ANIMATION_DATA sampleClip() {
    MODEL_ANIMATION_DATA c;c.name="fixture";c.durationTicks=30;c.ticksPerSecond=30;
    for(int i=0;i<3;++i) {
        MODEL_ANIMATION_CHANNEL_DATA ch;ch.resolvedBoneIndex=i;
        ch.positionKeys={{0,{0,0,0}},{5,{1,2,3}},{10,{2,3,4}},{30,{3,5,7}}};
        ch.rotationKeys={{0,{0,0,0,1}},{10,{0,0,.70710677f,.70710677f}},{30,{0,0,1,0}}};
        ch.scaleKeys={{0,{1,1,1}},{30,{2,3,4}}};c.channels.push_back(ch);
    }
    return c;
}
void runSequence(const MODEL_ANIMATION_DATA& clip,const Bones& rest,unsigned copies=10,const CMesh* mesh=nullptr) {
    auto expected=cloneBones(rest);
    auto baseline=CAnimation::Create(clip,expected);
    vector<Bones> bones;vector<shared_ptr<CAnimation>> animations;
    for(unsigned i=0;i<copies;++i) {
        bones.push_back(cloneBones(rest));animations.push_back(CAnimation::Create(clip,bones.back()));
        animations.back()->Enable_SampleReuse();
        require(bool(animations.back()->m_pSampleReuse),"cooked clip opted in");
        require(animations.front()->m_pSampleReuse==animations.back()->m_pSampleReuse,"identical tracks share owner");
    }
    const auto capacity=animations[0]->m_pSampleReuse->localTransforms.capacity();
    // Nonmonotonic times, repeated sample, end clamp, random seek and loop wrap.
    vector<float> fractions={0,.2f,.4f,.2f,.2f,1,0,.9999f,.5f,1,0};
    uint32_t random=731u;
    for(unsigned i=0;i<32;++i) { random=1664525u*random+1013904223u;fractions.push_back(float(random%10000u)/10000.f); }
    for(const float scale:{.01f,1.f,2.5f}) for(const float fraction:fractions) {
        const float t=fraction*clip.durationTicks;
        baseline->Set_TrackPosition(t);const bool finish=baseline->Update_TransformationMatrix(0,expected,false);combine(expected,scale);
        const unsigned before=reuseHits.load();
        for(unsigned n=0;n<copies;++n) {
            animations[n]->Set_TrackPosition(t);
            require(animations[n]->Update_TransformationMatrix(0,bones[n],false)==finish,"finished flag preserved");
            require(animations[n]->Get_CurrentTrackPosition()==baseline->Get_CurrentTrackPosition(),"clock preserved");
            combine(bones[n],scale);compare(expected,bones[n]);
            if(mesh) {
                vector<float4x4_t> expectedPalette(mesh->m_iNumBones),actualPalette(mesh->m_iNumBones);
                mesh->Build_SkinPalette(expected,expectedPalette.data());mesh->Build_SkinPalette(bones[n],actualPalette.data());
                require(std::memcmp(expectedPalette.data(),actualPalette.data(),expectedPalette.size()*sizeof(float4x4_t))==0,"installed skin palette differs");
            }
        }
        require(reuseHits.load()-before>=copies-1,"duplicate interpolation skipped");
    }
    for(const float dt:{.02f,.2f,-.4f,5.f,-9.f,0.f}) {
        const bool finish=baseline->Update_TransformationMatrix(dt,expected,true);combine(expected,.01f);
        for(unsigned n=0;n<copies;++n) {
            require(animations[n]->Update_TransformationMatrix(dt,bones[n],true)==finish,"loop/reverse result");
            combine(bones[n],.01f);compare(expected,bones[n]);
        }
    }
    // Reset/reuse restores local state but cannot mutate the shared memo.
    for(auto& instance:bones)for(size_t i=0;i<instance.size();++i)
        instance[i]->Update_TransformationMatrix(rest[i]->Get_TransformationMatrix());
    for(size_t i=0;i<expected.size();++i)expected[i]->Update_TransformationMatrix(rest[i]->Get_TransformationMatrix());
    baseline->Set_TrackPosition(clip.durationTicks*.25f);baseline->Update_TransformationMatrix(0,expected,false);combine(expected,.01f);
    for(unsigned n=0;n<copies;++n) {
        animations[n]->Set_TrackPosition(clip.durationTicks*.25f);animations[n]->Update_TransformationMatrix(0,bones[n],false);
        combine(bones[n],.01f);compare(expected,bones[n]);
    }
    require(animations[0]->m_pSampleReuse->localTransforms.capacity()==capacity,"sample storage stays bounded");
    // Each clone can diverge in time; returning to another clone's sample is exact.
    animations[0]->Set_TrackPosition(1);animations[1]->Set_TrackPosition(2);
    animations[0]->Update_TransformationMatrix(0,bones[0],false);
    animations[1]->Update_TransformationMatrix(0,bones[1],false);
    require(animations[0]->Get_CurrentTrackPosition()==1 && animations[1]->Get_CurrentTrackPosition()==2,"independent clocks");
}
double measureGroup(const MODEL_ANIMATION_DATA& clip,const Bones& rest,bool cached) {
    vector<Bones> bones;vector<shared_ptr<CAnimation>> animations;
    for(unsigned i=0;i<10;++i) {bones.push_back(cloneBones(rest));animations.push_back(CAnimation::Create(clip,bones.back()));if(cached)animations.back()->Enable_SampleReuse();}
    const auto started=std::chrono::steady_clock::now();
    for(unsigned frame=0;frame<180;++frame)for(unsigned i=0;i<10;++i) {
        animations[i]->Set_TrackPosition(float(frame)/180.f*clip.durationTicks);
        animations[i]->Update_TransformationMatrix(0,bones[i],false);combine(bones[i],.01f);
    }
    const auto ended=std::chrono::steady_clock::now();
    return std::chrono::duration<double,std::milli>(ended-started).count()/180.;
}
vector<pair<double,double>> benchmarks;
template<class T> T read(std::ifstream& in) { T value{};in.read(reinterpret_cast<char*>(&value),sizeof(value));if(!in)throw std::runtime_error("fixture truncated");return value; }
string readString(std::ifstream& in) {const auto count=read<uint32_t>(in);string out(count,'\0');in.read(out.data(),count);if(!in)throw std::runtime_error("fixture name truncated");return out;}
unsigned realClips=0,realBones=0;
void realFixture(const char* path) {
    std::ifstream in(path,std::ios::binary);if(!in)throw std::runtime_error("fixture open failed");
    const auto modelCount=read<uint32_t>(in);
    for(unsigned model=0;model<modelCount;++model) {
        Bones bones;const auto count=read<uint32_t>(in);realBones+=count;
        for(unsigned i=0;i<count;++i) {MODEL_BONE_DATA b;b.name=readString(in);b.parentIndex=read<int32_t>(in);b.restLocal=read<float4x4_t>(in);bones.push_back(CBone::Create(b));}
        CMesh mesh;mesh.m_iNumBones=read<uint32_t>(in);
        for(unsigned i=0;i<mesh.m_iNumBones;++i) {mesh.m_BoneIndices.push_back(i);mesh.m_OffsetMatrices.push_back(read<float4x4_t>(in));}
        const auto clips=read<uint32_t>(in);realClips+=clips;
        for(unsigned i=0;i<clips;++i) {
            MODEL_ANIMATION_DATA clip;clip.name=readString(in);clip.durationTicks=read<float>(in);clip.ticksPerSecond=read<float>(in);
            const auto channels=read<uint32_t>(in);
            for(unsigned j=0;j<channels;++j) {MODEL_ANIMATION_CHANNEL_DATA ch;ch.resolvedBoneIndex=read<int32_t>(in);
                auto n=read<uint32_t>(in);for(unsigned k=0;k<n;++k)ch.positionKeys.push_back(read<MODEL_VECTOR_KEY_DATA>(in));
                n=read<uint32_t>(in);for(unsigned k=0;k<n;++k)ch.rotationKeys.push_back(read<MODEL_QUAT_KEY_DATA>(in));
                n=read<uint32_t>(in);for(unsigned k=0;k<n;++k)ch.scaleKeys.push_back(read<MODEL_VECTOR_KEY_DATA>(in));
                clip.channels.push_back(std::move(ch));}
            runSequence(clip,bones,10,&mesh);
            vector<double> baseline,reused;
            for(unsigned trial=0;trial<3;++trial) {
                baseline.push_back(measureGroup(clip,bones,false));reused.push_back(measureGroup(clip,bones,true));
            }
            std::sort(baseline.begin(),baseline.end());std::sort(reused.begin(),reused.end());
            benchmarks.push_back({baseline[1],reused[1]});
        }
    }
}
int main(int argc,char** argv) try {
    auto rest=makeBones();auto clip=sampleClip();runSequence(clip,rest);
    auto a=CAnimation::Create(clip,rest);a->Enable_SampleReuse();
    auto same=CAnimation::Create(clip,rest);same->Enable_SampleReuse();
    require(a->m_pSampleReuse==same->m_pSampleReuse,"equal input shared");
    for(unsigned kind=0;kind<8;++kind) {
        auto changed=clip;
        switch(kind) {
        case 0:changed.channels[0].positionKeys[1].value.x+=.001f;break;
        case 1:changed.channels[0].rotationKeys[1].value.w+=.001f;break;
        case 2:changed.channels[0].scaleKeys[1].value.z+=.001f;break;
        case 3:changed.channels[0].positionKeys[1].timeTicks+=.001f;break;
        case 4:changed.channels[0].resolvedBoneIndex=3;break;
        case 5:std::swap(changed.channels[0],changed.channels[1]);changed.channels[0].positionKeys[1].value.x+=1;break;
        case 6:changed.channels[0].positionKeys.pop_back();break;
        case 7:changed.channels.pop_back();break;
        }
        auto other=CAnimation::Create(changed,rest);other->Enable_SampleReuse();
        require(a->m_pSampleReuse!=other->m_pSampleReuse,"different tracks/index/count must not alias");
        runSequence(changed,rest,3);
    }
    auto b=cloneBones(rest);b[3]->Update_TransformationMatrix(XMMatrixTranslation(100,200,300));
    a->Set_TrackPosition(10);a->Update_TransformationMatrix(0,rest,false);
    same->Set_TrackPosition(10);same->Update_TransformationMatrix(0,b,false);
    requireMatrix(b[3]->Get_TransformationMatrix(),XMMatrixTranslation(100,200,300),"unkeyed bone remains clone-local");
    // Caller edits keyed bones after sampling (root suppression/blend/attachments)
    // must not contaminate the immutable sample used by another actor.
    const auto before=rest[0]->Get_TransformationMatrix();
    rest[0]->Update_TransformationMatrix(XMMatrixTranslation(-900,0,0));
    same->Update_TransformationMatrix(0,b,false);requireMatrix(b[0]->Get_TransformationMatrix(),before,"post-sample edit cannot contaminate memo");
    std::weak_ptr<CAnimation::SAMPLE_REUSE> dead;
    { auto unique=clip;unique.channels[0].positionKeys[0].value.x=734;auto owner=CAnimation::Create(unique,b);owner->Enable_SampleReuse();dead=owner->m_pSampleReuse; }
    require(dead.expired(),"registry retains no dead sample owner");
    // Parallel clones have separate clocks and bones, while the memo is shared.
    std::atomic<bool> parallelOK{true};vector<std::thread> workers;
    for(unsigned n=0;n<4;++n)workers.emplace_back([&,n] {
        auto own=cloneBones(b),expected=cloneBones(b);auto actual=CAnimation::Create(clip,own),baseline=CAnimation::Create(clip,expected);actual->Enable_SampleReuse();
        for(unsigned frame=0;frame<200;++frame) { const float t=float((frame*7+n*3)%31);actual->Set_TrackPosition(t);baseline->Set_TrackPosition(t);
            actual->Update_TransformationMatrix(0,own,false);baseline->Update_TransformationMatrix(0,expected,false);
            for(unsigned i=0;i<4;++i) {XMFLOAT4X4 x,y;XMStoreFloat4x4(&x,own[i]->Get_TransformationMatrix());XMStoreFloat4x4(&y,expected[i]->Get_TransformationMatrix());if(std::memcmp(&x,&y,sizeof(x)))parallelOK=false;}
        }
    });
    for(auto& worker:workers)worker.join();require(parallelOK,"parallel sampling stays exact");
    if(argc>1)realFixture(argv[1]);
    std::cout<<"{\"checks\":"<<checks<<",\"compiler\":"<<_MSC_FULL_VER<<",\"reuseHits\":"<<reuseHits.load()<<",\"realClips\":"<<realClips<<",\"realBones\":"<<realBones<<",\"tenActorSampleAndCombineMs\":[";
    for(size_t i=0;i<benchmarks.size();++i) {if(i)std::cout<<',';std::cout<<"{\"uncached\":"<<benchmarks[i].first<<",\"cached\":"<<benchmarks[i].second<<'}';}
    std::cout<<"]}\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
'''


def fixture(out):
    sys.path.insert(0, str(ROOT / 'Tools/ModelAssetConverter'))
    import verify_dimensionmaster_summon_bind_pose as wm
    document = json.loads((ROOT / 'Data/Maps/Authoring/LV_LOBBY_CLASSSELECT_SL00/LV_LOBBY_CLASSSELECT_SL00.worldsequences.json').read_bytes())
    objects = document['objectResources']
    chosen = []
    for token in ('artist.a12246.p0', 'dimensionmaster.a12266.p0'):
        match = next(row for row in objects if token in row['objectId'] and row['animated'])
        chosen.append(ROOT / 'Client/Bin/Resources' / match['modelAssetId'])
    data=bytearray(struct.pack('<I',len(chosen))); metadata=[]
    def string(value):
        raw=value.encode('utf8');data.extend(struct.pack('<I',len(raw)));data.extend(raw)
    for path in chosen:
        model=wm.read_wmodel(path,include_geometry=False)
        metadata.append(dict(path=str(path),sha256=hashlib.sha256(path.read_bytes()).hexdigest(),bones=len(model.skeleton_bones),clips=len(model.animations)))
        data.extend(struct.pack('<I',len(model.skeleton_bones)))
        for bone in model.skeleton_bones:
            string(bone.name);data.extend(struct.pack('<i16f',bone.parent,*bone.transform))
        data.extend(struct.pack('<I',len(model.mesh_bones)))
        for bone in model.mesh_bones:data.extend(struct.pack('<16f',*bone.transform))
        data.extend(struct.pack('<I',len(model.animations)))
        for clip in model.animations:
            string(clip.name);data.extend(struct.pack('<ffI',clip.duration_ticks,clip.ticks_per_second,len(clip.channels)))
            for channel in clip.channels:
                data.extend(struct.pack('<i',channel.bone_index))
                for keys,width in ((channel.position_keys,4),(channel.rotation_keys,5),(channel.scale_keys,4)):
                    data.extend(struct.pack('<I',len(keys)))
                    for key in keys:data.extend(struct.pack('<'+'f'*width,*key))
    (out/'installed-fixture.bin').write_bytes(data)
    return metadata


def main():
    parser=argparse.ArgumentParser()
    parser.add_argument('--out',type=Path,default=ROOT/'out/MovieSampleReuseRegression')
    parser.add_argument('--configuration',choices=['Debug','Release'],default='Debug')
    parser.add_argument('--force-hash-collision',action='store_true')
    parser.add_argument('--prepare-only',action='store_true')
    parser.add_argument('--toolset',default='14.44')
    parser.add_argument('--sdk',default='10.0.26100.0')
    args=parser.parse_args();out=args.out.resolve();out.mkdir(parents=True,exist_ok=True)
    (out/'BinaryAsset').mkdir(exist_ok=True)
    (out/'Engine_Defines.h').write_text(DEFINES,encoding='utf8')
    (out/'Profiler.h').write_text('#pragma once\nnamespace Engine { struct CProfilerScope { CProfilerScope(void*,const char* label) { if(std::strcmp(label,"Animation.Channels.Reuse")==0) ++reuseHits; } }; }\n',encoding='utf8')
    (out/'GameInstance.h').write_text('#pragma once\nnamespace Engine { struct CGameInstance { static CGameInstance& Get(){static CGameInstance instance;return instance;} void* Get_Profiler(){return nullptr;} }; }\n',encoding='utf8')
    (out/'Engine_AnimationTypes.h').write_text('#pragma once\n#include "Engine_Defines.h"\nnamespace Engine {struct KEYFRAME {float3_t vScale;float4_t vRotation;float3_t vTranslation;float fTrackPosition;};}\n',encoding='utf8')
    types=(ROOT/'Engine/Public/BinaryAsset/ModelAssetData.h').read_text(encoding='utf8')
    types=types[types.index('struct MODEL_BONE_DATA\n'):types.index('struct MODEL_ASSET_DATA\n')]
    (out/'BinaryAsset/ModelAssetData.h').write_text('#pragma once\n#include "Engine_Defines.h"\nnamespace Engine {\n'+types+'}\n',encoding='utf8')
    for name in SOURCES:
        source=(ROOT/name).read_text(encoding='utf8')
        if name.endswith('.h'):source=source.replace('private:','public:')
        if name.endswith('Animation.cpp') and args.force_hash_collision:
            assert source.count('registry[hash]')==1
            source=source.replace('registry[hash]','registry[0u]')
        (out/Path(name).name).write_text(source,encoding='utf8')
    mesh=(ROOT/'Engine/Private/Mesh.cpp').read_text(encoding='utf8')
    begin=mesh.index('void CMesh::Build_SkinPalette(')
    end=mesh.index('\nHRESULT CMesh::Render_Instanced(',begin)
    palette=mesh[begin:end].strip()
    (out/'probe.cpp').write_text(PROBE.replace('/*PRODUCTION_SKIN_PALETTE*/',palette),encoding='utf8')
    assets=fixture(out)
    if args.prepare_only:
        print(json.dumps(dict(prepared=str(out),assets=assets)));return
    vcvars=next(Path('C:/Program Files/Microsoft Visual Studio').glob('*/*/VC/Auxiliary/Build/vcvars64.bat'),None)
    if vcvars is None:vcvars=next(Path('C:/Program Files/Microsoft Visual Studio').glob('*/VC/Auxiliary/Build/vcvars64.bat'))
    # Animation/Channel and the updated Bone item use /O2 in Product Debug.
    flags='/O2 /MDd /Zi /D_DEBUG' if args.configuration=='Debug' else '/O2 /MD /DNDEBUG'
    (out/'compile.cmd').write_text('@echo off\ncall "'+str(vcvars)+f'" {args.sdk} -vcvars_ver={args.toolset} >nul\nif errorlevel 1 exit /b 1\n'
        +f'cl /nologo /std:c++20 /EHsc /W4 /utf-8 {flags} /I"{out}" /I"{ROOT}/Engine/Public" probe.cpp /Fe:probe.exe /Fo:probe.obj\n',encoding='utf8')
    compiled=subprocess.run(['cmd.exe','/d','/c','compile.cmd'],cwd=out,capture_output=True,text=True,encoding='utf8',errors='replace')
    (out/'compile.log').write_text(compiled.stdout+compiled.stderr,encoding='utf8')
    if compiled.returncode:raise RuntimeError(f'Compile failed: {out / "compile.log"}')
    ran=subprocess.run([str(out/'probe.exe'),str(out/'installed-fixture.bin')],cwd=out,capture_output=True,text=True)
    (out/'run.log').write_text(ran.stdout+ran.stderr,encoding='utf8')
    if ran.returncode:raise RuntimeError(f'Probe failed: {out / "run.log"}')
    result=json.loads(ran.stdout)
    result.update(configuration=args.configuration,forcedHashCollision=args.force_hash_collision,assets=assets,
                  toolset=args.toolset,sdk=args.sdk,flags=flags,
                  sourceSha256={p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in SOURCES},
                  skinPaletteMethodSha256=hashlib.sha256(palette.encode()).hexdigest(),
                  scope='Production Animation/Channel/Bone/CMesh skin palette and real DirectXMath; no GPU, Client, material, live CModel/part attachment or FPS verification')
    (out/'result.json').write_text(json.dumps(result,indent=2),encoding='utf8')
    print(json.dumps(result))


if __name__=='__main__':main()
