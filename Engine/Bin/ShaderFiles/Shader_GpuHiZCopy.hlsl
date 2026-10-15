// 깊이 texture를 Hi-Z 첫 단계 UAV로 복사한다. 원본 밖 여백은 먼 깊이로 채워 보수적으로 가림을 판단한다.
Texture2D<float> Depth : register(t0);
RWTexture2D<float> Output : register(u0);
[numthreads(8,8,1)]
void Main(uint3 tid : SV_DispatchThreadID)
{
    uint width, height, sourceWidth, sourceHeight;
    Output.GetDimensions(width, height);
    Depth.GetDimensions(sourceWidth, sourceHeight);
    if (tid.x >= width || tid.y >= height) return;
    // 2의 거듭제곱 바깥 여백은 먼 깊이로 유지해 부분 타일을 가림자로 오인하지 않는다.
    Output[tid.xy] = tid.x < sourceWidth && tid.y < sourceHeight ? Depth.Load(int3(tid.xy,0)) : 1;
}
