struct SourceInstance
{
    float4 WorldSphere;
    uint LodBins[5];
    float WorldErrors[5];
    float DistanceLimit;
    float MaximumPixels;
    uint ActiveSource;
    uint Flags;
    uint2 Reserved;
};
StructuredBuffer<SourceInstance> Sources : register(t0);
ByteAddressBuffer Payload : register(t1);
StructuredBuffer<uint> Active : register(t2);
StructuredBuffer<uint4> Bins : register(t3);
Texture2D<float> HiZ : register(t4);
RWByteAddressBuffer Output : register(u0);
RWByteAddressBuffer Arguments : register(u1);
cbuffer Frame : register(b0)
{
    row_major float4x4 View;
    row_major float4x4 Projection;
    float4 Planes[6];
    float4 CameraNear;
    float4 ViewportPolicy;
    uint4 Counts;
    uint4 Flags;
};

[numthreads(64, 1, 1)]
void Main(uint3 tid : SV_DispatchThreadID)
{
    uint id = tid.x;
    if (id >= Counts.x) return;
    SourceInstance source = Sources[id];
    if (Active[source.ActiveSource] == 0) return;
    float3 center = source.WorldSphere.xyz;
    // CPU가 허용한 view 직교 오차(각 내적 0.001)까지 포함하는 확대 상계.
    float radius = source.WorldSphere.w * 1.002;
    [unroll] for (uint p = 0; p < 6; ++p)
        if (Flags.w != 0 && dot(float4(center, 1), Planes[p]) < -radius) return;
    float3 vc = mul(float4(center, 1), View).xyz;
    float nearest = vc.z - radius;
    float2 lo = float2(1e30, 1e30), hi = -lo;
    bool projectable = nearest > CameraNear.w;
    if (projectable)
    {
        // 구의 view-space AABB 8점을 투영해 off-axis에서도 실제 투영 구를 감싼다.
        [unroll] for (uint corner = 0; corner < 8; ++corner)
        {
            float3 signs = float3((corner & 1) ? 1 : -1, (corner & 2) ? 1 : -1, (corner & 4) ? 1 : -1);
            float4 clip = mul(float4(vc + signs * radius, 1), Projection);
            float2 projected = clip.xy / clip.w;
            lo = min(lo, projected);
            hi = max(hi, projected);
        }
        float pixels = max((hi.x-lo.x)*ViewportPolicy.x, (hi.y-lo.y)*ViewportPolicy.y) * .5;
        float distance = max(0, length(center - CameraNear.xyz) - radius);
        if (Flags.x != 0 && source.DistanceLimit > 0 && source.MaximumPixels > 0 &&
            distance > source.DistanceLimit * ViewportPolicy.w && pixels < source.MaximumPixels) return;
    }
    if (projectable && all(lo <= 1) && all(hi >= -1) && (source.Flags & 1) == 0 && Flags.z != 0 && Counts.w != 0)
    {
        uint width, height, levels;
        HiZ.GetDimensions(0, width, height, levels);
        float2 lowPixel = float2(lo.x * .5 + .5, .5 - hi.y * .5) * ViewportPolicy.xy;
        float2 highPixel = float2(hi.x * .5 + .5, .5 - lo.y * .5) * ViewportPolicy.xy;
        lowPixel = max(lowPixel - 1, 0);
        highPixel = min(highPixel + 1, ViewportPolicy.xy - 1);
        float extent = max(highPixel.x-lowPixel.x+1, highPixel.y-lowPixel.y+1);
        uint mip = min((uint)ceil(log2(max(extent, 1))), Counts.w - 1);
        int2 first = (int2)floor(lowPixel / exp2((float)mip));
        int2 last = (int2)floor(highPixel / exp2((float)mip));
        float maximumDepth = 0;
        // 선택 mip의 전체 footprint를 읽는다. max reduction은 빈 깊이 1도 보존한다.
        [loop] for (int y = first.y; y <= last.y; ++y)
            [loop] for (int x = first.x; x <= last.x; ++x)
                maximumDepth = max(maximumDepth, HiZ.Load(int3(x, y, mip)));
        float4 closestClip = mul(float4(0, 0, nearest, 1), Projection);
        if (closestClip.z / closestClip.w > maximumDepth + 0.0001) return;
    }
    uint bin = source.LodBins[0];
    if (Flags.y != 0 && projectable)
    {
        // CPU mesh LOD와 같은 off-axis / 깊이 변화 상계다. z 오차를 무시하면
        // 화면 가장자리와 근평면에서 같은 1px 계약을 위반할 수 있다.
        float2 projectionPixels = .5 * ViewportPolicy.xy * float2(Projection._11, Projection._22);
        float2 largest = abs(vc.xy) + radius;
        [unroll] for (uint lod = 1; lod < 5; ++lod)
        {
            float error = source.WorldErrors[lod] * 1.002;
            float safeZ = nearest - error;
            float2 pixels = projectionPixels * error * (1 + largest / nearest) / max(safeZ, 1e-20);
            if (source.LodBins[lod] != 0xffffffff && safeZ > CameraNear.w &&
                all(isfinite(pixels)) && max(pixels.x, pixels.y) <= ViewportPolicy.z)
                bin = source.LodBins[lod];
        }
    }
    uint4 target = Bins[bin];
    uint slot;
    Arguments.InterlockedAdd(bin * 20 + 4, 1, slot);
    // Stage는 모든 후보의 최대 개수를 검증한다. 방어 검사도 IA 범위 밖 쓰기를 막는다.
    if (slot >= target.y) return;
    uint sourceByte = id * Counts.y;
    uint targetByte = (target.x + slot) * Counts.y;
    [loop] for (uint offset = 0; offset < Counts.y; offset += 4)
        Output.Store(targetByte + offset, Payload.Load(sourceByte + offset));
}
