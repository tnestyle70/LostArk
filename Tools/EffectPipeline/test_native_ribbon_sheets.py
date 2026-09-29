"""Exercise the actual renderer sheet loop with DirectXMath, without a Client."""
from pathlib import Path
import json
import os
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]


def block_at(text, marker):
    start = text.index(marker)
    brace = text.index('{', start)
    depth = 1
    end = brace + 1
    while depth:
        depth += (text[end] == '{') - (text[end] == '}')
        end += 1
    return text[start:end]


class NativeRibbonSheets(unittest.TestCase):
    @unittest.skipUnless(os.name == 'nt', 'DirectXMath/MSVC CPU probe requires Windows')
    def test_actual_sheet_geometry_and_original_flight_count(self):
        document = json.loads((ROOT / 'Data/Effects/Authored/effect.world.item.whirlwind_grenade.flight.effect.json').read_bytes())
        ribbon = next(row for row in document['elements'] if row['kind'] == 'trail')
        typed = next(row for row in ribbon['sourceRecipe']['modules'] if row['className'] == 'particlemoduletypedataribbon')
        self.assertEqual(next(row['value'] for row in typed['literals'] if row['propertyPath'] == 'sheetspertrail'), 5)
        renderer = (ROOT / 'Client/Private/Effect_DocumentRenderer_Particles.cpp').read_text(encoding='utf8')
        loop = block_at(renderer, '// A strip repeats after pi radians; each sheet keeps its own indices.')
        settings = renderer[renderer.index('struct NATIVE_RIBBON_CURVE_SETTINGS final'):renderer.index('// Project interpolation of admitted source tangent flags')]
        code = r'''
#include <DirectXMath.h>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
using namespace DirectX;
using vector_t=XMVECTOR; using float2_t=XMFLOAT2;
XMFLOAT3 To_Float3(FXMVECTOR value) { XMFLOAT3 out; XMStoreFloat3(&out,value);return out; }
namespace Client {
enum class EFFECT_SOURCE_LITERAL_KIND {BOOLEAN,NUMBER};
struct Literal {std::string strPropertyPath; EFFECT_SOURCE_LITERAL_KIND eKind; bool bBoolean=false; double fNumber=0;};
struct Module {std::string strStableId,strClassName;std::vector<Literal>Literals;};
struct EFFECT_ELEMENT_DESC {struct {bool bEnabled=true;std::vector<Module>Modules;}SourceRecipe;struct {std::string strTypeDataModuleStableId="ribbon";}RuntimeCarrier;};
}
''' + settings + r'''
struct Vertex {XMFLOAT3 position;XMFLOAT2 uv;XMFLOAT4 color,dynamic;};
int main() {
  unsigned checks=0;
  for(unsigned count: {1u,5u,8u}) for(unsigned axis=0;axis<3;++axis) {
    Client::EFFECT_ELEMENT_DESC element;
    element.SourceRecipe.Modules.push_back({"ribbon","particlemoduletypedataribbon",{
      {"sheetspertrail",Client::EFFECT_SOURCE_LITERAL_KIND::NUMBER,false,double(count)}}});
    auto Curve=Native_RibbonCurveSettings(element);assert(Curve.sheetCount==count);++checks;
    auto SheetAxis=axis==0?XMVectorSet(1,0,0,0):axis==1?XMVectorSet(0,1,0,0):XMVectorSet(0,0,1,0);
    auto Side=axis==0?XMVectorSet(0,1,0,0):XMVectorSet(1,0,0,0);
    for(unsigned iSheet=0;iSheet<count;++iSheet) {
    std::vector<Vertex>Vertices;std::vector<uint32_t>Indices;
    bool bPreviousCenterlinePair=false;float Width=2.f;
    XMFLOAT4 Color{.2f,.3f,.4f,.5f};struct {XMFLOAT4 vDynamicParameter{1,2,3,4};}Point;
    for(unsigned point=0;point<4;++point) {
      if(point==2) bPreviousCenterlinePair=false; // one skipped/degenerate interval
      const auto Position=SheetAxis*float(point);float U=float(point);
''' + loop + r'''
      bPreviousCenterlinePair=true;
    }
    assert(Vertices.size()==8 && Indices.size()==12);checks+=2;
    for(unsigned i=0;i<Vertices.size();++i) {
      const auto point=i/2;const auto center=SheetAxis*float(point);
      const auto offset=XMLoadFloat3(&Vertices[i].position)-center;
      assert(std::abs(XMVectorGetX(XMVector3Length(offset))-1)<1e-5);
      assert(std::abs(XMVectorGetX(XMVector3Dot(offset,SheetAxis)))<1e-5);
      const float expected=(i%2 ? 1.f : -1.f)*std::cos(XM_PI*float(iSheet)/float(count));
      assert(std::abs(XMVectorGetX(XMVector3Dot(offset,Side))-expected)<1e-5);++checks;
      assert(Vertices[i].uv.x==float(point) && Vertices[i].uv.y==float(i%2));checks+=3;
    }
    for(unsigned i=0;i<Indices.size();i+=3) {

      for(unsigned lane=0;lane<3;++lane) {assert(Indices[i+lane]<Vertices.size());++checks;}
      auto a=Indices[i]/2,b=Indices[i+1]/2,c=Indices[i+2]/2;
      assert(!((a<2 || b<2 || c<2) && (a>=2 || b>=2 || c>=2)));++checks;
    }
    if(count==1) {assert(Indices[0]==0 && Indices[1]==1 && Indices[2]==2 && Indices[5]==2);++checks;}
    }
  }
  std::cout<<checks<<" actual C++ sheet geometry checks passed\n";
}
'''
        vcvars = next(Path('C:/Program Files/Microsoft Visual Studio').glob('*/VC/Auxiliary/Build/vcvars64.bat'), None)
        if vcvars is None:
            vcvars = next(Path('C:/Program Files/Microsoft Visual Studio').glob('*/*/VC/Auxiliary/Build/vcvars64.bat'))
        with tempfile.TemporaryDirectory(prefix='battle-ribbon-') as temporary:
            path = Path(temporary)
            (path / 'probe.cpp').write_text(code, encoding='utf8')
            (path / 'build.cmd').write_text('@echo off\ncall "' + str(vcvars) + '" >nul\nif errorlevel 1 exit /b 1\ncl /nologo /std:c++20 /EHsc /W4 probe.cpp /Fe:probe.exe\n', encoding='utf8')
            build = subprocess.run(['cmd.exe', '/d', '/c', 'build.cmd'], cwd=path, capture_output=True, text=True)
            self.assertEqual(build.returncode, 0, build.stdout + build.stderr)
            result = subprocess.run([str(path / 'probe.exe')], capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            print(result.stdout.strip())


if __name__ == '__main__':
    unittest.main()
