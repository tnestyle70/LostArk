// 이전 깊이 단계의 2×2 영역에서 가장 먼 깊이를 골라 다음 Hi-Z UAV에 기록한다.
Texture2D<float> Previous : register(t0);
RWTexture2D<float> Output : register(u0);
[numthreads(8,8,1)]
void Main(uint3 tid : SV_DispatchThreadID)
{
    uint width, height, sourceWidth, sourceHeight;
    Output.GetDimensions(width, height);
    Previous.GetDimensions(sourceWidth, sourceHeight);
    if (tid.x >= width || tid.y >= height) return;
    uint2 p = tid.xy * 2;
    uint2 edge = uint2(sourceWidth-1, sourceHeight-1);
    float z = Previous.Load(int3(min(p,edge),0));
    z = max(z, Previous.Load(int3(min(p+uint2(1,0),edge),0)));
    z = max(z, Previous.Load(int3(min(p+uint2(0,1),edge),0)));
    z = max(z, Previous.Load(int3(min(p+uint2(1,1),edge),0)));
    Output[tid.xy] = z;
}
