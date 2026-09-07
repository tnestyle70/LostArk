#include "LightResourceCatalog.h"
#include "DataJson.h"
#include "MapAssetCatalog.h"
#include "MapLightDocument.h"
#include "ProjectDataRoot.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <unordered_set>

namespace
{
using namespace Client;
std::string Read(const std::filesystem::path& path)
{
    std::ifstream in(path, std::ios::binary);
    if (!in) throw std::runtime_error("Cannot read light catalog: " + path.string());
    std::ostringstream out; out << in.rdbuf();
    if (in.bad()) throw std::runtime_error("Light catalog read failed: " + path.string());
    return out.str();
}
const DATA_JSON_VALUE& Field(const DATA_JSON_VALUE& row, const char* name)
{
    const auto* value = row.Find(name);
    if (!value) throw std::runtime_error(std::string("Missing light field: ") + name);
    return *value;
}
void Exact(const DATA_JSON_VALUE& row, std::initializer_list<const char*> names)
{
    if (!row.Is_Object() || row.Get_Object().size() != names.size())
        throw std::runtime_error("Light object has unexpected fields.");
    for (const auto* name : names) (void)Field(row, name);
}
std::string Text(const DATA_JSON_VALUE& row, const char* name)
{
    const auto& v = Field(row,name);
    if (!v.Is_String()) throw std::runtime_error(std::string("Light text is invalid: ") + name);
    return v.Get_String();
}
double Number(const DATA_JSON_VALUE& row, const char* name, double lo, double hi)
{
    const auto& v = Field(row,name);
    if (!v.Is_Number() || !std::isfinite(v.Get_Number()) ||
        (hi>100000.0 ? (v.Get_Number()<lo || v.Get_Number()>hi) :
         (static_cast<float>(v.Get_Number())<static_cast<float>(lo) || static_cast<float>(v.Get_Number())>static_cast<float>(hi))))
        throw std::runtime_error(std::string("Light number is invalid: ") + name);
    return v.Get_Number();
}
uint32_t UInt(const DATA_JSON_VALUE& row, const char* name)
{
    const double n = Number(row,name,1,4294967294.0);
    if (std::floor(n) != n) throw std::runtime_error(std::string("Light integer is invalid: ")+name);
    return static_cast<uint32_t>(n);
}
void Vector(const DATA_JSON_VALUE& row, const char* name, float* out, size_t count)
{
    const auto& v = Field(row,name);
    if (!v.Is_Array() || v.Get_Array().size()!=count) throw std::runtime_error(std::string("Light vector is invalid: ")+name);
    for (size_t i=0;i<count;++i)
    {
        const auto& n=v.Get_Array()[i];
        if (!n.Is_Number() || !std::isfinite(n.Get_Number()) || std::abs(n.Get_Number())>100000.)
            throw std::runtime_error(std::string("Light vector component is invalid: ")+name);
        out[i]=static_cast<float>(n.Get_Number());
    }
}
bool Stable(std::string_view s)
{
    return !s.empty() && s.size()<=128 && std::all_of(s.begin(),s.end(),[](unsigned char c)
    {return (c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||c=='.'||c=='_'||c=='-';});
}
void Parse(const std::string& bytes, std::vector<LIGHT_RESOURCE>& records,uint32_t& revision,uint32_t& ordinal)
{
    DATA_JSON_VALUE root; std::string error;
    if(!CDataJson::Parse(bytes,root,error)) throw std::runtime_error(error);
    Exact(root,{"schema","formatVersion","revision","nextLightResourceOrdinal","lights"});
    if(Text(root,"schema")!="lostark.light-resources" || Number(root,"formatVersion",1,1)!=1)
        throw std::runtime_error("Unsupported light resource catalog.");
    revision=UInt(root,"revision"); ordinal=UInt(root,"nextLightResourceOrdinal");
    const auto& rows=Field(root,"lights");
    if(!rows.Is_Array() || rows.Get_Array().size()>4096) throw std::runtime_error("Invalid light resource count.");
    std::unordered_set<std::string> ids;
    std::vector<LIGHT_RESOURCE> staged;
    for(const auto& row:rows.Get_Array())
    {
        Exact(row,{"lightResourceId","displayName","kind","defaultAnchorKind","localOffset","localRotationDegrees",
            "rangeMeters","falloffExponent","innerConeDegrees","outerConeDegrees","color","brightness"});
        LIGHT_RESOURCE r; r.strLightResourceId=Text(row,"lightResourceId"); r.strDisplayName=Text(row,"displayName");
        r.strDefaultAnchorKind=Text(row,"defaultAnchorKind"); const auto kind=Text(row,"kind");
        if(kind=="POINT") r.eType=LIGHT::POINT;
        else if(kind=="SPOT") r.eType=LIGHT::SPOT;
        else if(kind=="DIRECTIONAL") r.eType=LIGHT::DIRECTIONAL;
        else throw std::runtime_error("Unsupported light kind: "+kind);
        Vector(row,"localOffset",&r.vLocalOffset.x,3); Vector(row,"localRotationDegrees",&r.vLocalRotationDegrees.x,3);
        Vector(row,"color",&r.vColor.x,4);
        r.fRangeMeters=static_cast<float>(Number(row,"rangeMeters",0,1000));
        r.fFalloffExponent=static_cast<float>(Number(row,"falloffExponent",0.01,64));
        r.fInnerConeDegrees=static_cast<float>(Number(row,"innerConeDegrees",0,89.9));
        r.fOuterConeDegrees=static_cast<float>(Number(row,"outerConeDegrees",0,89.9));
        r.fBrightness=static_cast<float>(Number(row,"brightness",0,64));
        if(!CLightResourceCatalog::Validate_Resource(r,error)) throw std::runtime_error(error);
        if(!ids.insert(r.strLightResourceId).second) throw std::runtime_error("Duplicate light resource: "+r.strLightResourceId);
        const std::string prefix="light.runtime.";
        if(r.strLightResourceId.starts_with(prefix))
        {
            const auto suffix=r.strLightResourceId.substr(prefix.size());
            if(suffix.empty() || suffix[0]=='0' || !std::all_of(suffix.begin(),suffix.end(),[](char c){return c>='0'&&c<='9';}) || std::stoull(suffix)>=ordinal)
                throw std::runtime_error("Light resource ordinal exceeds nextLightResourceOrdinal.");
        }
        staged.push_back(std::move(r));
    }
    records=std::move(staged);
}
std::string Serialize(const std::vector<LIGHT_RESOURCE>& rows,uint32_t revision,uint32_t ordinal)
{
    std::ostringstream o; o << std::setprecision(9);
    o << "{\n  \"schema\": \"lostark.light-resources\",\n  \"formatVersion\": 1,\n  \"revision\": " <<revision
      <<",\n  \"nextLightResourceOrdinal\": "<<ordinal<<",\n  \"lights\": [";
    for(size_t i=0;i<rows.size();++i)
    {
        const auto& r=rows[i]; o<<(i?",\n":"\n")<<"    {\"lightResourceId\": \""<<CDataJson::Escape(r.strLightResourceId)
        <<"\", \"displayName\": \""<<CDataJson::Escape(r.strDisplayName)<<"\", \"kind\": \""<<CLightResourceCatalog::Kind_Name(r.eType)
        <<"\", \"defaultAnchorKind\": \""<<r.strDefaultAnchorKind<<"\", \"localOffset\": ["<<r.vLocalOffset.x<<","<<r.vLocalOffset.y<<","<<r.vLocalOffset.z
        <<"], \"localRotationDegrees\": ["<<r.vLocalRotationDegrees.x<<","<<r.vLocalRotationDegrees.y<<","<<r.vLocalRotationDegrees.z
        <<"], \"rangeMeters\": "<<r.fRangeMeters<<", \"falloffExponent\": "<<r.fFalloffExponent
        <<", \"innerConeDegrees\": "<<r.fInnerConeDegrees<<", \"outerConeDegrees\": "<<r.fOuterConeDegrees
        <<", \"color\": ["<<r.vColor.x<<","<<r.vColor.y<<","<<r.vColor.z<<","<<r.vColor.w<<"], \"brightness\": "<<r.fBrightness<<"}";
    }
    o<<"\n  ]\n}\n"; return o.str();
}
}

NS_BEGIN(Client)
const char* CLightResourceCatalog::Kind_Name(LIGHT type)
{
    switch(type){case LIGHT::DIRECTIONAL:return "DIRECTIONAL";case LIGHT::POINT:return "POINT";case LIGHT::SPOT:return "SPOT";default:return "INVALID";}
}
bool_t CLightResourceCatalog::Validate_Resource(const LIGHT_RESOURCE& r,std::string& status)
{
    const auto finite=[](float n,float a,float b){return std::isfinite(n)&&n>=a&&n<=b;};
    if(!Stable(r.strLightResourceId)||r.strDisplayName.empty()||r.strDisplayName.size()>256 ||
       MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,r.strDisplayName.data(),static_cast<int>(r.strDisplayName.size()),nullptr,0)==0 ||
       (r.strDefaultAnchorKind!="MAP"&&r.strDefaultAnchorKind!="PLAYER"&&r.strDefaultAnchorKind!="BOSS") ||
       !finite(r.fBrightness,0,64)||!finite(r.fFalloffExponent,0.01f,64))
    {status="Invalid light identity, anchor, brightness or falloff: "+r.strLightResourceId;return false;}
    for(float v:{r.vLocalOffset.x,r.vLocalOffset.y,r.vLocalOffset.z,r.vLocalRotationDegrees.x,r.vLocalRotationDegrees.y,r.vLocalRotationDegrees.z})
        if(!finite(v,-100000,100000)){status="Invalid light transform: "+r.strLightResourceId;return false;}
    for(float v:{r.vColor.x,r.vColor.y,r.vColor.z,r.vColor.w})
        if(!finite(v,0,1)){status="Invalid light color: "+r.strLightResourceId;return false;}
    if((r.eType!=LIGHT::POINT&&r.eType!=LIGHT::SPOT&&r.eType!=LIGHT::DIRECTIONAL)||
        !finite(r.fRangeMeters,r.eType==LIGHT::DIRECTIONAL?0.f:0.01f,1000)||
        !finite(r.fInnerConeDegrees,0,89.9f)||!finite(r.fOuterConeDegrees,0,89.9f)||
        (r.eType==LIGHT::SPOT&&(r.fInnerConeDegrees<=0||r.fInnerConeDegrees>r.fOuterConeDegrees))||
        (r.eType!=LIGHT::SPOT&&(r.fInnerConeDegrees!=0||r.fOuterConeDegrees!=0))||
        (r.eType==LIGHT::DIRECTIONAL&&(r.fRangeMeters!=0||r.vLocalOffset.x!=0||r.vLocalOffset.y!=0||r.vLocalOffset.z!=0)))
    {status="Invalid light type, range or cone: "+r.strLightResourceId;return false;}
    return true;
}
bool_t CLightResourceCatalog::Try_BuildLightDesc(const LIGHT_RESOURCE& r,const float4x4_t& pivot,
    f32_t multiplier,LIGHT_DESC& out,std::string& status)
{
    if(!Validate_Resource(r,status)||!std::isfinite(multiplier)||multiplier<0||multiplier>64)return false;
    const float* values=&pivot._11;
    for(size_t i=0;i<16;++i)if(!std::isfinite(values[i])){status="Light anchor contains a non-finite value.";return false;}
    matrix_t basis=XMLoadFloat4x4(&pivot);
    for(size_t i=0;i<3;++i)
    {
        const float lengthSquared=XMVectorGetX(XMVector3LengthSq(basis.r[i]));
        if(!std::isfinite(lengthSquared)||lengthSquared<0.000001f){status="Light anchor has an invalid axis.";return false;}
        basis.r[i]=XMVector3Normalize(basis.r[i]);
    }
    const matrix_t local=XMMatrixRotationRollPitchYaw(XMConvertToRadians(r.vLocalRotationDegrees.x),
        XMConvertToRadians(r.vLocalRotationDegrees.y),XMConvertToRadians(r.vLocalRotationDegrees.z))*
        XMMatrixTranslation(r.vLocalOffset.x,r.vLocalOffset.y,r.vLocalOffset.z)*basis;
    LIGHT_DESC light{};light.eType=r.eType;
    XMStoreFloat4(&light.vPosition,XMVectorSetW(local.r[3],1.f));
    XMStoreFloat4(&light.vDirection,XMVectorSetW(XMVector3Normalize(local.r[2]),0.f));
    light.fRange=r.fRangeMeters;light.fFalloffExponent=r.fFalloffExponent;
    light.vDiffuse={r.vColor.x*r.fBrightness*multiplier,r.vColor.y*r.fBrightness*multiplier,r.vColor.z*r.fBrightness*multiplier,1.f};
    light.vAmbient={0,0,0,0};light.vSpecular={0,0,0,0};
    light.fSpotInnerCos=std::cos(XMConvertToRadians(r.fInnerConeDegrees));
    light.fSpotOuterCos=std::cos(XMConvertToRadians(r.fOuterConeDegrees));
    for(float value:{light.vPosition.x,light.vPosition.y,light.vPosition.z,
        light.vDirection.x,light.vDirection.y,light.vDirection.z,
        light.vDiffuse.x,light.vDiffuse.y,light.vDiffuse.z})
        if(!std::isfinite(value)){status="Resolved light transform/color is not finite.";return false;}
    if(XMVectorGetX(XMVector3LengthSq(XMLoadFloat4(&light.vDirection)))<0.000001f)
    {status="Resolved light direction is zero.";return false;}
    out=light;return true;
}
bool_t CLightResourceCatalog::Load_Runtime(std::string& status)
{
    if(!Reload_Runtime(status))return false;
    std::string authorStatus;
    if(!m_AuthoredReady && !Load_Authored(authorStatus))
    {m_Authored=m_Runtime;Rebuild_ResourceView();status+=" Authoring unavailable: "+authorStatus;}
    return true;
}
bool_t CLightResourceCatalog::Reload_Runtime(std::string& status)
{
    try{std::vector<LIGHT_RESOURCE> rows;uint32_t rev=1,next=1;
        Parse(Read(CMapAssetCatalog::Get_MapDataRoot().parent_path()/L"Rendering/LightResources.runtime.json"),rows,rev,next);
        m_Runtime=std::move(rows);status="Light runtime reloaded: "+std::to_string(m_Runtime.size());
        if(!m_MapAreaId.empty()){std::string mapStatus;if(!Refresh_MapResources(m_MapAreaId,mapStatus))status+=" "+mapStatus;}return true;}
    catch(const std::exception& e){status=e.what();return false;}
}
bool_t CLightResourceCatalog::Load_Authored(std::string& status)
{
    try{const auto bytes=Read(CProjectDataRoot::Resolve(L"Rendering/Authored/LightResources.json"));
        std::vector<LIGHT_RESOURCE> rows;uint32_t rev=1,next=1;Parse(bytes,rows,rev,next);
        m_Authored=std::move(rows);m_Revision=rev;m_NextOrdinal=next;m_SourceBytes=bytes;m_AuthoredReady=true;
        Rebuild_ResourceView();status="Light authoring loaded.";return true;}
    catch(const std::exception& e){status=e.what();return false;}
}
const LIGHT_RESOURCE* CLightResourceCatalog::Find_Resource(std::string_view id) const
{const auto it=std::find_if(m_ResourceView.begin(),m_ResourceView.end(),[id](const auto& r){return r.strLightResourceId==id;});return it==m_ResourceView.end()?nullptr:&*it;}
const LIGHT_RESOURCE* CLightResourceCatalog::Find_RuntimeResource(std::string_view id) const
{
    for(const auto& row:m_Runtime)if(row.strLightResourceId==id)return &row;
    for(const auto& row:m_MapRuntime)if(row.strLightResourceId==id)return &row;
    return nullptr;
}
void CLightResourceCatalog::Rebuild_ResourceView()
{m_ResourceView=m_Authored;m_ResourceView.insert(m_ResourceView.end(),m_MapAuthored.begin(),m_MapAuthored.end());}
bool_t CLightResourceCatalog::Is_MapResource(std::string_view id) const
{return std::any_of(m_MapAuthored.begin(),m_MapAuthored.end(),[id](const auto& r){return r.strLightResourceId==id;});}
bool_t CLightResourceCatalog::Refresh_MapResources(const std::string& areaId,std::string& status)
{
    if(!Stable(areaId)){status="Map resource Area ID is invalid.";return false;}
    const auto convert=[](const CMapLightDocument& doc)
    {
        std::vector<LIGHT_RESOURCE> rows;
        for(const auto& map:doc.Get_Lights())
        {
            LIGHT_RESOURCE r;r.strLightResourceId=map.lightId;r.strDisplayName=map.displayName;
            r.strDefaultAnchorKind="MAP";r.eType=map.kind;r.vLocalOffset=map.position;r.vLocalRotationDegrees=map.rotationDegrees;
            r.fRangeMeters=map.radiusMeters;r.fFalloffExponent=map.falloffExponent;r.fInnerConeDegrees=map.innerConeDegrees;
            r.fOuterConeDegrees=map.outerConeDegrees;r.vColor=map.color;r.fBrightness=map.brightness;rows.push_back(std::move(r));
        }
        return rows;
    };
    CMapLightDocument runtime;
    if(!runtime.Load(CMapAssetCatalog::Get_MapDataRoot()/std::filesystem::path(areaId+".maplights.json"),areaId,status))return false;
    if(runtime.Get_FormatVersion()!=2){status="Imported map lights are not reusable authored resources.";return false;}
    const auto path=CProjectDataRoot::Get()/L"Maps/Authoring"/std::filesystem::path(areaId)/std::filesystem::path(areaId+".maplights.json");
    CMapLightDocument source;std::string sourceStatus;
    const bool sourceReady=source.Load(path,areaId,sourceStatus)&&source.Get_FormatVersion()==2;
    m_MapRuntime=convert(runtime);
    if(sourceReady)m_MapAuthored=convert(source);
    else if(m_MapAreaId!=areaId)m_MapAuthored.clear();
    m_MapAreaId=areaId;Rebuild_ResourceView();
    status=sourceReady?"Map light resources refreshed.":"Product map lights refreshed; source unavailable: "+sourceStatus;
    return true;
}
bool_t CLightResourceCatalog::Create(const std::string& name,const std::string& anchor,LIGHT type,std::string& id,std::string& status)
{
    if(!m_AuthoredReady||m_Authored.size()>=4096||m_NextOrdinal>=UINT32_MAX-1){status="Light authoring is unavailable or full.";return false;}
    LIGHT_RESOURCE r;r.strLightResourceId="light.runtime."+std::to_string(m_NextOrdinal);r.strDisplayName=name;r.strDefaultAnchorKind=anchor;r.eType=type;
    if(type!=LIGHT::SPOT){r.fInnerConeDegrees=0;r.fOuterConeDegrees=0;}
    if(type==LIGHT::DIRECTIONAL){r.vLocalOffset={0,0,0};r.fRangeMeters=0;}
    if(!Validate_Resource(r,status)||Find_Resource(r.strLightResourceId))return false;
    id=r.strLightResourceId;m_Authored.push_back(std::move(r));++m_NextOrdinal;Rebuild_ResourceView();status="Light created.";return true;
}
bool_t CLightResourceCatalog::Update(const LIGHT_RESOURCE& r,std::string& status)
{
    if(!m_AuthoredReady||!Validate_Resource(r,status))return false;
    for(auto& existing:m_Authored)if(existing.strLightResourceId==r.strLightResourceId){existing=r;Rebuild_ResourceView();status="Light draft updated.";return true;}
    status="Light resource is missing: "+r.strLightResourceId;return false;
}
bool_t CLightResourceCatalog::Delete(std::string_view id,std::string& status)
{
    if(!m_AuthoredReady){status="Light authoring is not loaded.";return false;}
    const auto it=std::find_if(m_Authored.begin(),m_Authored.end(),[id](const auto& r){return r.strLightResourceId==id;});
    if(it==m_Authored.end()){status="Light resource is missing.";return false;}
    try
    {
        const auto path=CProjectDataRoot::Resolve(L"KoukuSaydon/Gate1/KoukuSaydonComposition.json");
        DATA_JSON_VALUE root;std::string error;
        if(!CDataJson::Parse(Read(path),root,error)){status="Cannot verify composition light references: "+error;return false;}
        const auto* rows=root.Find("presentationResources");
        if(rows && !rows->Is_Array()){status="Composition resources are invalid; light preserved.";return false;}
        if(rows)for(const auto& row:rows->Get_Array())
        {const auto* asset=row.Find("assetId");if(asset&&asset->Is_String()&&asset->Get_String()==id){status="Light is referenced by Composition; remove its reference first.";return false;}}
    }catch(const std::exception& e){status=e.what();return false;}
    m_Authored.erase(it);Rebuild_ResourceView();status="Light deleted from draft.";return true;
}
bool_t CLightResourceCatalog::Save_Authored(std::string& status)
{
    if(!m_AuthoredReady||m_Revision>=UINT32_MAX-1){status="Light authoring cannot be saved.";return false;}
    const auto path=CProjectDataRoot::Resolve(L"Rendering/Authored/LightResources.json");
    auto temp=path;temp+=L".lighting-"+std::to_wstring(GetCurrentProcessId())+L".tmp";
    try
    {
        const auto bytes=Serialize(m_Authored,m_Revision+1,m_NextOrdinal);
        std::vector<LIGHT_RESOURCE> verified;uint32_t rev=1,next=1;Parse(bytes,verified,rev,next);
        if(Read(path)!=m_SourceBytes){status="Light source changed on disk. Reload before saving; draft preserved.";return false;}
        {std::ofstream out(temp,std::ios::binary|std::ios::trunc);out<<bytes;if(!out)throw std::runtime_error("Cannot stage light source.");}
        Parse(Read(temp),verified,rev,next);
        if(Read(path)!=m_SourceBytes)throw std::runtime_error("Light source changed during save; draft preserved.");
        if(!MoveFileExW(temp.c_str(),path.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))throw std::runtime_error("Cannot replace light source.");
        m_SourceBytes=bytes;m_Revision=rev;status="Light resources saved at revision "+std::to_string(rev)+". Publish to update Product.";return true;
    }catch(const std::exception& e){std::error_code ec;std::filesystem::remove(temp,ec);status=e.what();return false;}
}
bool_t CLightResourceCatalog::Run_Publisher(const std::filesystem::path& relativeScript,const std::wstring& arguments,std::string& status)
{
    const auto root=CProjectDataRoot::Get().parent_path();const auto script=root/relativeScript;
    if(!std::filesystem::is_regular_file(script)){status="Light publisher is missing.";return false;}
    std::wstring command=L"powershell.exe -NoProfile -ExecutionPolicy Bypass -File \""+script.wstring()+L"\" "+arguments;
    std::vector<wchar_t> buffer(command.begin(),command.end());buffer.push_back(0);
    STARTUPINFOW startup{};startup.cb=sizeof(startup);PROCESS_INFORMATION process{};
    if(!CreateProcessW(nullptr,buffer.data(),nullptr,nullptr,FALSE,CREATE_NO_WINDOW,nullptr,root.c_str(),&startup,&process))
    {status="Cannot start light publisher.";return false;}
    CloseHandle(process.hThread);const DWORD wait=WaitForSingleObject(process.hProcess,60000);DWORD code=1;
    const bool ok=wait==WAIT_OBJECT_0&&GetExitCodeProcess(process.hProcess,&code)&&code==0;CloseHandle(process.hProcess);
    status=ok?"Light publish completed. Reload Runtime to apply.":wait==WAIT_TIMEOUT?
        "Publisher is still running; runtime was not reloaded. Wait for its completion before publishing again.":"Light publish failed: "+std::to_string(code);
    return ok;
}
bool_t CLightResourceCatalog::Publish_Runtime(std::string& status) const
{return Run_Publisher(L"Tools/RenderingPipeline/Publish-LightResources.ps1",L"-Mode Publish",status);}
NS_END
