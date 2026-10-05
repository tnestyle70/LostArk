#include <algorithm>
#include <array>
#include <atomic>
#include <barrier>
#include <bit>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include "DataJson.h"
#include <Psapi.h>

using Clock = std::chrono::steady_clock;
using Client::CDataJson;
using Client::DATA_JSON_VALUE;
using Client::DATA_JSON_TYPE;
using Client::DATA_JSON_PARSE_LIMITS;
static double Ms(Clock::time_point a, Clock::time_point b) { return std::chrono::duration<double, std::milli>(b-a).count(); }
static std::atomic<bool> trackAlloc{false};
static std::atomic<uint64_t> allocations{0}, allocationBytes{0};
#ifdef _DEBUG
static int AllocHook(int kind, void*, size_t size, int, long, const unsigned char*, int) {
    if(kind == _HOOK_ALLOC && trackAlloc.load(std::memory_order_relaxed)) {
        allocations.fetch_add(1, std::memory_order_relaxed);
        allocationBytes.fetch_add(size, std::memory_order_relaxed);
    }
    return TRUE;
}
#endif
struct Digest {
    uint64_t hash=14695981039346656037ull, nodes=0;
    void Byte(uint8_t b){hash=(hash^b)*1099511628211ull;}
    void U64(uint64_t n){for(int i=0;i<8;i++) Byte(static_cast<uint8_t>(n>>(i*8)));}
    void Text(std::string_view s){U64(s.size());for(unsigned char c:s)Byte(c);}
    void Value(const DATA_JSON_VALUE& v){
        ++nodes; Byte(static_cast<uint8_t>(v.Get_Type()));
        switch(v.Get_Type()){
        case DATA_JSON_TYPE::BOOLEAN: Byte(v.Get_Boolean()); break;
        case DATA_JSON_TYPE::NUMBER: U64(std::bit_cast<uint64_t>(v.Get_Number())); Byte(v.Was_FloatingPointToken()); break;
        case DATA_JSON_TYPE::STRING: Text(v.Get_String());break;
        case DATA_JSON_TYPE::ARRAY: U64(v.Get_Array().size());for(const auto& x:v.Get_Array())Value(x);break;
        case DATA_JSON_TYPE::OBJECT:
            U64(v.Get_Object().size());for(const auto& [k,x]:v.Get_Object()){Text(k);Value(x);}
            U64(v.Get_ObjectInsertionOrder().size());for(const auto& k:v.Get_ObjectInsertionOrder())Text(k);break;
        default:break;
        }
    }
};
static std::string Hex(uint64_t v){std::ostringstream s;s<<std::hex<<std::setw(16)<<std::setfill('0')<<v;return s.str();}
static std::string Quote(std::string_view v){return "\""+CDataJson::Escape(v)+"\"";}
struct Input {std::string id, path, text;};
struct Row {bool ok=false;std::string error;uint64_t hash=0,nodes=0;double parseMs=0,digestMs=0,destroyMs=0;};
static uint64_t FT(FILETIME v){return (uint64_t(v.dwHighDateTime)<<32)|v.dwLowDateTime;}
struct Usage {uint64_t idle=0,kernel=0,user=0,process=0;};
static Usage UsageNow(){FILETIME i{},k{},u{},c{},e{},pk{},pu{};GetSystemTimes(&i,&k,&u);GetProcessTimes(GetCurrentProcess(),&c,&e,&pk,&pu);return {FT(i),FT(k),FT(u),FT(pk)+FT(pu)};}
static void Require(bool yes, const char* message){if(!yes)throw std::runtime_error(message);}
static unsigned Contracts(){
    unsigned n=0;std::string e;DATA_JSON_VALUE v;
    const std::string good=R"({"z":[null,true,false,1,1.0,-0,2e1,"a\n\uD83D\uDE00"],"a":{"q":2}})";
    Require(CDataJson::Parse(good,v,e),"valid mixed input");++n;Digest d;d.Value(v);
    Require(v.Get_ObjectInsertionOrder()==std::vector<std::string>({"z","a"}),"insertion order");++n;
    Require(v.Get_Object().begin()->first=="a","sorted object keys");++n;
    Require(!v.Find("z")->Get_Array()[3].Was_FloatingPointToken() && v.Find("z")->Get_Array()[4].Was_FloatingPointToken(),"numeric token flags");++n;
    auto copy=v;Digest dc;dc.Value(copy);Require(dc.hash==d.hash,"copy value");++n;
    Require(&copy.Get_Object()!=&v.Get_Object() && &copy.Find("z")->Get_Array()!=&v.Find("z")->Get_Array(),"deep copy owns subtree");++n;
    auto moved=std::move(copy);Digest dm;dm.Value(moved);Require(dm.hash==d.hash,"move destination");++n;
    moved=moved;Digest ds;ds.Value(moved);Require(ds.hash==d.hash,"self-copy assignment");++n;
    for(const std::string bad:{"", "{", "[1,]", "{\"a\":1,\"a\":2}", "[01]", "[1e]", "[NaN]", "true false", "\"\\uD800\"", "\"\\uDC00\""}) {
        auto keep=v;Require(!CDataJson::Parse(bad,keep,e),"malformed input accepted");Digest k;k.Value(keep);Require(k.hash==d.hash,"failure changed output");++n;
    }
    DATA_JSON_PARSE_LIMITS l;l.iMaximumBytes=2;Require(CDataJson::Parse("[]",v,e,l),"byte boundary");++n;
    Require(!CDataJson::Parse("[0]",v,e,l),"byte limit");++n;
    l={};l.iMaximumValues=2;Require(CDataJson::Parse("[0]",v,e,l),"value boundary");++n;
    Require(!CDataJson::Parse("[0,1]",v,e,l),"value limit");++n;
    l={};l.iMaximumDepth=1;Require(CDataJson::Parse("[0]",v,e,l),"depth boundary");++n;
    Require(!CDataJson::Parse("[[0]]",v,e,l),"depth limit");++n;
    l={};l.iMaximumValues=0;Require(!CDataJson::Parse("null",v,e,l),"zero limits");++n;
    return n;
}
int main(int argc,char** argv){try{
    if(argc<4)throw std::runtime_error("usage: exe corpus.tsv workers output.json [alloc]");
    const size_t workers=std::stoull(argv[2]);Require(workers==1||workers==3,"workers must be 1 or 3");
    const bool allocationRun=argc>4 && std::string(argv[4])=="alloc";
    Require(!allocationRun || workers==1,"allocation pass must have one worker");
    const auto contracts=Contracts();
    std::vector<Input> inputs;uint64_t bytes=0;std::ifstream list(argv[1]);std::string line;
    while(std::getline(list,line)){if(!line.empty()&&line.back()=='\r')line.pop_back();if(line.empty())continue;const auto tab=line.find('\t');Require(tab!=std::string::npos,"invalid corpus row");Input in{line.substr(0,tab),line.substr(tab+1),{}};std::ifstream f(in.path,std::ios::binary);Require(bool(f),"missing corpus file");in.text.assign(std::istreambuf_iterator<char>(f),{});Require(!f.bad(),"read failed");bytes+=in.text.size();inputs.push_back(std::move(in));}
    Require(!inputs.empty(),"empty corpus");
    std::vector<Row> rows(inputs.size());std::vector<std::unique_ptr<DATA_JSON_VALUE>> roots(workers);
    std::barrier gate(static_cast<std::ptrdiff_t>(workers+1));std::atomic<int> phase{0};size_t wave=0;
    DATA_JSON_PARSE_LIMITS limits;limits.iMaximumBytes=64u*1024u*1024u;limits.iMaximumDepth=64;limits.iMaximumValues=3'000'000;
    std::vector<std::thread> threads;
    for(size_t slot=0;slot<workers;slot++)threads.emplace_back([&,slot]{for(;;){gate.arrive_and_wait();const int p=phase.load();if(p==4){gate.arrive_and_wait();break;}const size_t index=wave+slot;if(index<inputs.size()){
        Row& r=rows[index];auto a=Clock::now();
        if(p==1){roots[slot]=std::make_unique<DATA_JSON_VALUE>();r.ok=CDataJson::Parse(inputs[index].text,*roots[slot],r.error,limits);r.parseMs=Ms(a,Clock::now());}
        if(p==2){Digest d;if(r.ok)d.Value(*roots[slot]);r.hash=d.hash;r.nodes=d.nodes;r.digestMs=Ms(a,Clock::now());}
        if(p==3){roots[slot].reset();r.destroyMs=Ms(a,Clock::now());}
    }gate.arrive_and_wait();}});
#ifdef _DEBUG
    const auto previousHook=allocationRun?_CrtSetAllocHook(AllocHook):nullptr;
#endif
    const Usage u0=UsageNow();const auto totalStart=Clock::now();double parseWall=0,digestWall=0,destroyWall=0;
    for(wave=0;wave<inputs.size();wave+=workers)for(int p=1;p<=3;p++){
        phase.store(p);trackAlloc.store(allocationRun && p==1);const auto a=Clock::now();gate.arrive_and_wait();gate.arrive_and_wait();const auto elapsed=Ms(a,Clock::now());trackAlloc.store(false);if(p==1)parseWall+=elapsed;if(p==2)digestWall+=elapsed;if(p==3)destroyWall+=elapsed;
    }
    const auto totalEnd=Clock::now();const Usage u1=UsageNow();phase.store(4);gate.arrive_and_wait();gate.arrive_and_wait();for(auto& t:threads)t.join();
#ifdef _DEBUG
    if(allocationRun)_CrtSetAllocHook(previousHook);
#endif
    PROCESS_MEMORY_COUNTERS_EX memory{};GetProcessMemoryInfo(GetCurrentProcess(),reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&memory),sizeof(memory));
    uint64_t nodes=0,failed=0;Digest aggregate;double parseSum=0,destroySum=0;for(size_t i=0;i<rows.size();i++){nodes+=rows[i].nodes;failed+=!rows[i].ok;aggregate.Text(inputs[i].id);aggregate.U64(rows[i].hash);parseSum+=rows[i].parseMs;destroySum+=rows[i].destroyMs;}
    const double busy=double((u1.kernel-u0.kernel)+(u1.user-u0.user)-(u1.idle-u0.idle));
    const double all=double((u1.kernel-u0.kernel)+(u1.user-u0.user));
    std::ofstream out(argv[3]);out<<std::setprecision(12)<<"{\n\"workers\":"<<workers<<",\n\"allocationInstrumentation\":"<<(allocationRun?"true":"false")<<",\n\"debug\":"
#ifdef _DEBUG
    <<"true"
#else
    <<"false"
#endif
    <<",\n\"iteratorDebugLevel\":"<<_ITERATOR_DEBUG_LEVEL<<",\n\"sizeofValue\":"<<sizeof(DATA_JSON_VALUE)<<",\n\"contractCases\":"<<contracts<<",\n\"fileCount\":"<<inputs.size()<<",\n\"inputBytes\":"<<bytes<<",\n\"nodes\":"<<nodes<<",\n\"failed\":"<<failed<<",\n\"semanticDigestFnv1a64\":"<<Quote(Hex(aggregate.hash))<<",\n\"parseWallMs\":"<<parseWall<<",\n\"digestWallMs\":"<<digestWall<<",\n\"destructionWallMs\":"<<destroyWall<<",\n\"totalWallMs\":"<<Ms(totalStart,totalEnd)<<",\n\"parseSummedWorkerMs\":"<<parseSum<<",\n\"destructionSummedWorkerMs\":"<<destroySum<<",\n\"processCpuMs\":"<<double(u1.process-u0.process)/10000.<<",\n\"systemCpuBusyPercent\":"<<(all?busy/all*100.:0.)<<",\n\"peakWorkingSetBytes\":"<<memory.PeakWorkingSetSize<<",\n\"privateBytesAtEnd\":"<<memory.PrivateUsage<<",\n\"parseAllocations\":"<<allocations.load()<<",\n\"parseAllocatedBytes\":"<<allocationBytes.load()<<",\n\"boundary\":\"Preloaded memory; actual unmodified CDataJson parser and Engine headers; no codec, GPU, Client or UI. Phases measured in waves; worker sums are not wall time. FNV semantic traversal is outside parse timing. Allocation passes are excluded from speed comparisons.\",\n\"files\":[\n";
    for(size_t i=0;i<rows.size();i++){const auto&r=rows[i];if(i)out<<",\n";out<<"{\"id\":"<<Quote(inputs[i].id)<<",\"bytes\":"<<inputs[i].text.size()<<",\"ok\":"<<(r.ok?"true":"false")<<",\"error\":"<<Quote(r.error)<<",\"nodes\":"<<r.nodes<<",\"digest\":"<<Quote(Hex(r.hash))<<",\"parseMs\":"<<r.parseMs<<",\"digestMs\":"<<r.digestMs<<",\"destroyMs\":"<<r.destroyMs<<"}";}
    out<<"\n]}\n";out.close();std::cout<<"files="<<inputs.size()<<" failed="<<failed<<" parse_ms="<<parseWall<<" destroy_ms="<<destroyWall<<" total_ms="<<Ms(totalStart,totalEnd)<<" digest="<<Hex(aggregate.hash)<<"\n";return failed?2:0;
}catch(const std::exception&e){std::cerr<<e.what()<<"\n";return 1;}}
