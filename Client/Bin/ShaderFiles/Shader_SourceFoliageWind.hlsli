// Native LostArk LocalVF foliage displacement arithmetic, with source-owned draw inputs.
// Original coordinates are centimetres [x,y,z]; WModel/runtime coordinates are metres [x,z,-y].
#ifndef SOURCE_FOLIAGE_WIND_INCLUDED
#define SOURCE_FOLIAGE_WIND_INCLUDED
uint g_SourceFoliageWindEnabled = 0u;
uint g_SourceFoliageWindProgram = 1u;
float4 g_SourceFoliageWindLocalCenter = float4(0,0,0,1);
float4 g_SourceFoliageWindLocalBounds = float4(1,1,1,1);
float4 g_SourceFoliageWindActorPosition = 0;
float4 g_SourceFoliageWindDirectionSpeed = float4(0,0,1,0);
float4 g_SourceFoliageWindPlayerPosition = 0;
float4 g_SourceFoliageWindScalars[4];
float g_SourceFoliageWindTime = 0;
// W=1 marks a verified per-placement original ActorWorldPos; dimensions/radius stay in source cm.
float4 g_SourceFoliageWindDrawOwnerPosition = 0;
float4 g_SourceFoliageWindDrawDimensionsAndRadius = 0;
float3 SourceFoliageToUE(float3 value) { return float3(value.x,-value.z,value.y)*100.f; }
// Native shader e4fe43228f19284f9ca096e27b675db6, displacement instructions 28..160.
float3 SourceFoliageOffsetProgram1(float3 worldPosition, float4 vertexColor, float4x4 world,
    float4 sourceOwner, float4 sourceDimensionsAndRadius)
{
    float4 cb0[18]; float4 cb1[6];
    [unroll] for(uint i=0;i<18;++i) cb0[i]=0;
    [unroll] for(uint j=0;j<6;++j) cb1[j]=0;
    cb0[0]=float4(1,0,0,0);cb0[1]=float4(0,1,0,0);
    cb0[2]=float4(0,0,1,0);cb0[3]=float4(0,0,0,1);
    float3 worldExtent = abs(world[0].xyz)*g_SourceFoliageWindLocalBounds.x+
        abs(world[1].xyz)*g_SourceFoliageWindLocalBounds.y+abs(world[2].xyz)*g_SourceFoliageWindLocalBounds.z;
    float3 columnSq=world[0].xyz*world[0].xyz+world[1].xyz*world[1].xyz+world[2].xyz*world[2].xyz;
    float maxScale=sqrt(max(columnSq.x,max(columnSq.y,columnSq.z)));
    float4 dimensionsAndRadius=sourceOwner.w==1.f ? sourceDimensionsAndRadius :
        float4(worldExtent.x,worldExtent.z,worldExtent.y,g_SourceFoliageWindLocalBounds.w*maxScale)*100.f+1.f;
    float4 actorPosition=sourceOwner.w==1.f ? float4(sourceOwner.xyz,0) : g_SourceFoliageWindActorPosition;
    float4 center=float4(SourceFoliageToUE(mul(g_SourceFoliageWindLocalCenter,world).xyz),1);
    cb0[4]=actorPosition; cb0[5].w=dimensionsAndRadius.w;
    cb0[6].xyz=dimensionsAndRadius.xyz; cb0[7]=g_SourceFoliageWindDirectionSpeed;
    cb0[8]=center; cb0[9]=g_SourceFoliageWindPlayerPosition;
    [unroll] for(uint k=0;k<4;++k) cb0[10+k]=g_SourceFoliageWindScalars[k];
    cb0[12].z=g_SourceFoliageWindTime;
    float4 v0=float4(SourceFoliageToUE(worldPosition),1);
    // Native packed VF color is BGRA; the installed WModel carrier is RGBA.
    float4 v3=vertexColor.bgra;
    float4 r0=0,r1=0,r2=0,r3=0,r4=0,r5=0,r6=0,r7=0,r8=0,r9=0,r10=0,r11=0,r12=0,r13=0;
    // mov r3.w, l(0)
    r3.w = asfloat(0u);
    // frc r4.xy, cb0[4].xyxx
    float stage1_x = frac(cb0[4].x);
    float stage1_y = frac(cb0[4].y);
    r4.x = stage1_x;
    r4.y = stage1_y;
    // mad r4.xy, r4.xyxx, r4.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    float stage2_x = r4.x * r4.x + -0.5f;
    float stage2_y = r4.y * r4.y + -0.5f;
    r4.x = stage2_x;
    r4.y = stage2_y;
    // add r4.xy, r4.xyxx, r4.xyxx
    float stage3_x = r4.x + r4.x;
    float stage3_y = r4.y + r4.y;
    r4.x = stage3_x;
    r4.y = stage3_y;
    // add r0.w, r4.y, r4.x
    r0.w = r4.y + r4.x;
    // mul r0.w, r0.w, cb0[12].w
    r0.w = r0.w * cb0[12].w;
    // lt r1.w, l(0.000000), cb0[5].w
    r1.w = asfloat((0.0f < cb0[5].w) ? 0xffffffffu : 0u);
    // movc r1.w, r1.w, cb0[5].w, l(0.000100)
    r1.w = asuint(r1.w) != 0u ? cb0[5].w : 0.0001f;
    // ge r2.w, cb0[5].w, l(0.000000)
    r2.w = asfloat((cb0[5].w >= 0.0f) ? 0xffffffffu : 0u);
    // movc r1.w, r2.w, r1.w, cb0[5].w
    r1.w = asuint(r2.w) != 0u ? r1.w : cb0[5].w;
    // div r1.w, l(100.000000), r1.w
    r1.w = 100.0f / r1.w;
    // mul r2.w, cb0[7].w, cb0[12].z
    r2.w = cb0[7].w * cb0[12].z;
    // mul r4.x, r1.w, r2.w
    r4.x = r1.w * r2.w;
    // mul r1.w, r1.w, cb0[13].x
    r1.w = r1.w * cb0[13].x;
    // mad r4.x, cb0[12].y, r4.x, r0.w
    r4.x = cb0[12].y * r4.x + r0.w;
    // mul r4.xyz, r4.xxxx, l(1.086065, 0.989267, 3.141593, 0.000000)
    float stage4_x = r4.x * 1.086065f;
    float stage4_y = r4.x * 0.989267f;
    float stage4_z = r4.x * 3.141593f;
    r4.x = stage4_x;
    r4.y = stage4_y;
    r4.z = stage4_z;
    // sincos r4.xyz, null, r4.xyzx
    float stage5_x = r4.x;
    float stage5_y = r4.y;
    float stage5_z = r4.z;
    float stage6_x = sin(stage5_x);
    float stage6_y = sin(stage5_y);
    float stage6_z = sin(stage5_z);
    r4.x = stage6_x;
    r4.y = stage6_y;
    r4.z = stage6_z;
    // add r4.xyz, r4.xyzx, l(1.000000, 1.666667, 1.000000, 0.000000)
    float stage7_x = r4.x + 1.0f;
    float stage7_y = r4.y + 1.666667f;
    float stage7_z = r4.z + 1.0f;
    r4.x = stage7_x;
    r4.y = stage7_y;
    r4.z = stage7_z;
    // mul r4.xy, r4.xyxx, l(0.500000, 0.375000, 0.000000, 0.000000)
    float stage8_x = r4.x * 0.5f;
    float stage8_y = r4.y * 0.375f;
    r4.x = stage8_x;
    r4.y = stage8_y;
    // mul r4.y, r4.y, r4.z
    r4.y = r4.y * r4.z;
    // mul r4.y, r4.y, l(-0.500000)
    r4.y = r4.y * -0.5f;
    // dp3 r4.z, cb0[7].xyzx, cb0[7].xyzx
    r4.z = dot(cb0[7].xyz, cb0[7].xyz);
    // sqrt r4.z, r4.z
    r4.z = sqrt(r4.z);
    // div r5.xyz, cb0[7].xyzx, r4.zzzz
    float stage9_x = cb0[7].x / r4.z;
    float stage9_y = cb0[7].y / r4.z;
    float stage9_z = cb0[7].z / r4.z;
    r5.x = stage9_x;
    r5.y = stage9_y;
    r5.z = stage9_z;
    // mul r6.xyz, r5.yzxy, l(0.000000, 0.000000, 1.000000, 0.000000)
    float stage10_x = r5.y * 0.0f;
    float stage10_y = r5.z * 0.0f;
    float stage10_z = r5.x * 1.0f;
    r6.x = stage10_x;
    r6.y = stage10_y;
    r6.z = stage10_z;
    // mad r6.xyz, r5.xyzx, l(0.000000, 1.000000, 0.000000, 0.000000), -r6.xyzx
    float stage11_x = r5.x * 0.0f + -r6.x;
    float stage11_y = r5.y * 1.0f + -r6.y;
    float stage11_z = r5.z * 0.0f + -r6.z;
    r6.x = stage11_x;
    r6.y = stage11_y;
    r6.z = stage11_z;
    // add r6.xyz, -r5.zxyz, r6.xyzx
    float stage12_x = -r5.z + r6.x;
    float stage12_y = -r5.x + r6.y;
    float stage12_z = -r5.y + r6.z;
    r6.x = stage12_x;
    r6.y = stage12_y;
    r6.z = stage12_z;
    // mad r4.xzw, r4.xxxx, r6.xxyz, r5.zzxy
    float stage13_x = r4.x * r6.x + r5.z;
    float stage13_z = r4.x * r6.y + r5.x;
    float stage13_w = r4.x * r6.z + r5.y;
    r4.x = stage13_x;
    r4.z = stage13_z;
    r4.w = stage13_w;
    // add r6.xyz, r4.xzwx, l(-1.000000, -0.000000, -0.000000, 0.000000)
    float stage14_x = r4.x + -1.0f;
    float stage14_y = r4.z + -0.0f;
    float stage14_z = r4.w + -0.0f;
    r6.x = stage14_x;
    r6.y = stage14_y;
    r6.z = stage14_z;
    // mul r7.xyz, cb0[1].xyzx, cb0[8].yyyy
    float stage15_x = cb0[1].x * cb0[8].y;
    float stage15_y = cb0[1].y * cb0[8].y;
    float stage15_z = cb0[1].z * cb0[8].y;
    r7.x = stage15_x;
    r7.y = stage15_y;
    r7.z = stage15_z;
    // mad r7.xyz, cb0[0].xyzx, cb0[8].xxxx, r7.xyzx
    float stage16_x = cb0[0].x * cb0[8].x + r7.x;
    float stage16_y = cb0[0].y * cb0[8].x + r7.y;
    float stage16_z = cb0[0].z * cb0[8].x + r7.z;
    r7.x = stage16_x;
    r7.y = stage16_y;
    r7.z = stage16_z;
    // mad r7.xyz, cb0[2].xyzx, cb0[8].zzzz, r7.xyzx
    float stage17_x = cb0[2].x * cb0[8].z + r7.x;
    float stage17_y = cb0[2].y * cb0[8].z + r7.y;
    float stage17_z = cb0[2].z * cb0[8].z + r7.z;
    r7.x = stage17_x;
    r7.y = stage17_y;
    r7.z = stage17_z;
    // mad r7.xyz, cb0[3].xyzx, cb0[8].wwww, r7.xyzx
    float stage18_x = cb0[3].x * cb0[8].w + r7.x;
    float stage18_y = cb0[3].y * cb0[8].w + r7.y;
    float stage18_z = cb0[3].z * cb0[8].w + r7.z;
    r7.x = stage18_x;
    r7.y = stage18_y;
    r7.z = stage18_z;
    // add r3.xyz, r7.xyzx, -cb1[5].xyzx
    float stage19_x = r7.x + -cb1[5].x;
    float stage19_y = r7.y + -cb1[5].y;
    float stage19_z = r7.z + -cb1[5].z;
    r3.x = stage19_x;
    r3.y = stage19_y;
    r3.z = stage19_z;
    // mul r7.xyzw, v0.yyyy, cb0[1].xyzw
    float stage20_x = v0.y * cb0[1].x;
    float stage20_y = v0.y * cb0[1].y;
    float stage20_z = v0.y * cb0[1].z;
    float stage20_w = v0.y * cb0[1].w;
    r7.x = stage20_x;
    r7.y = stage20_y;
    r7.z = stage20_z;
    r7.w = stage20_w;
    // mad r7.xyzw, cb0[0].xyzw, v0.xxxx, r7.xyzw
    float stage21_x = cb0[0].x * v0.x + r7.x;
    float stage21_y = cb0[0].y * v0.x + r7.y;
    float stage21_z = cb0[0].z * v0.x + r7.z;
    float stage21_w = cb0[0].w * v0.x + r7.w;
    r7.x = stage21_x;
    r7.y = stage21_y;
    r7.z = stage21_z;
    r7.w = stage21_w;
    // mad r7.xyzw, cb0[2].xyzw, v0.zzzz, r7.xyzw
    float stage22_x = cb0[2].x * v0.z + r7.x;
    float stage22_y = cb0[2].y * v0.z + r7.y;
    float stage22_z = cb0[2].z * v0.z + r7.z;
    float stage22_w = cb0[2].w * v0.z + r7.w;
    r7.x = stage22_x;
    r7.y = stage22_y;
    r7.z = stage22_z;
    r7.w = stage22_w;
    // mad r7.xyzw, cb0[3].xyzw, v0.wwww, r7.xyzw
    float stage23_x = cb0[3].x * v0.w + r7.x;
    float stage23_y = cb0[3].y * v0.w + r7.y;
    float stage23_z = cb0[3].z * v0.w + r7.z;
    float stage23_w = cb0[3].w * v0.w + r7.w;
    r7.x = stage23_x;
    r7.y = stage23_y;
    r7.z = stage23_z;
    r7.w = stage23_w;
    // add r8.xyz, r7.xyzx, -cb1[5].xyzx
    float stage24_x = r7.x + -cb1[5].x;
    float stage24_y = r7.y + -cb1[5].y;
    float stage24_z = r7.z + -cb1[5].z;
    r8.x = stage24_x;
    r8.y = stage24_y;
    r8.z = stage24_z;
    // add r9.xyz, -r3.xyzx, r8.xyzx
    float stage25_x = -r3.x + r8.x;
    float stage25_y = -r3.y + r8.y;
    float stage25_z = -r3.z + r8.z;
    r9.x = stage25_x;
    r9.y = stage25_y;
    r9.z = stage25_z;
    // dp3 r5.w, r9.xyzx, r9.xyzx
    r5.w = dot(r9.xyz, r9.xyz);
    // sqrt r5.w, r5.w
    r5.w = sqrt(r5.w);
    // div r10.xyz, r9.xyzx, r5.wwww
    float stage26_x = r9.x / r5.w;
    float stage26_y = r9.y / r5.w;
    float stage26_z = r9.z / r5.w;
    r10.x = stage26_x;
    r10.y = stage26_y;
    r10.z = stage26_z;
    // dp3 r5.x, r10.xyzx, r5.xyzx
    r5.x = dot(r10.xyz, r5.xyz);
    // mad r5.xyz, |r5.xxxx|, r6.xyzx, l(1.000000, 0.000000, 0.000000, 0.000000)
    float stage27_x = abs(r5.x) * r6.x + 1.0f;
    float stage27_y = abs(r5.x) * r6.y + 0.0f;
    float stage27_z = abs(r5.x) * r6.z + 0.0f;
    r5.x = stage27_x;
    r5.y = stage27_y;
    r5.z = stage27_z;
    // add r6.xy, -r3.xyxx, r8.xyxx
    float stage28_x = -r3.x + r8.x;
    float stage28_y = -r3.y + r8.y;
    r6.x = stage28_x;
    r6.y = stage28_y;
    // dp2 r5.w, r5.yzyy, r6.xyxx
    r5.w = dot(r5.yz, r6.xy);
    // mad r6.xyz, r5.yzxy, r5.wwww, r3.xywx
    float stage29_x = r5.y * r5.w + r3.x;
    float stage29_y = r5.z * r5.w + r3.y;
    float stage29_z = r5.x * r5.w + r3.w;
    r6.x = stage29_x;
    r6.y = stage29_y;
    r6.z = stage29_z;
    // mov r8.w, l(0)
    r8.w = asfloat(0u);
    // add r11.xyz, -r6.xyzx, r8.xywx
    float stage30_x = -r6.x + r8.x;
    float stage30_y = -r6.y + r8.y;
    float stage30_z = -r6.z + r8.w;
    r11.x = stage30_x;
    r11.y = stage30_y;
    r11.z = stage30_z;
    // mul r12.xyz, r5.xyzx, r11.yzxy
    float stage31_x = r5.x * r11.y;
    float stage31_y = r5.y * r11.z;
    float stage31_z = r5.z * r11.x;
    r12.x = stage31_x;
    r12.y = stage31_y;
    r12.z = stage31_z;
    // mad r5.xyz, r5.zxyz, r11.zxyz, -r12.xyzx
    float stage32_x = r5.z * r11.z + -r12.x;
    float stage32_y = r5.x * r11.x + -r12.y;
    float stage32_z = r5.y * r11.y + -r12.z;
    r5.x = stage32_x;
    r5.y = stage32_y;
    r5.z = stage32_z;
    // add r3.w, r8.z, r8.y
    r3.w = r8.z + r8.y;
    // add r3.w, r3.w, r8.x
    r3.w = r3.w + r8.x;
    // mul r3.w, r3.w, cb0[13].w
    r3.w = r3.w * cb0[13].w;
    // mad r0.w, r3.w, l(0.010000), r0.w
    r0.w = r3.w * 0.01f + r0.w;
    // mad r0.w, cb0[13].z, r2.w, r0.w
    r0.w = cb0[13].z * r2.w + r0.w;
    // mul r0.w, r0.w, l(6.283185)
    r0.w = r0.w * 6.283185f;
    // sincos r0.w, null, r0.w
    float stage33_w = r0.w;
    r0.w = sin(stage33_w);
    // lt r2.w, l(0.000000), cb0[6].z
    r2.w = asfloat((0.0f < cb0[6].z) ? 0xffffffffu : 0u);
    // movc r2.w, r2.w, cb0[6].z, l(0.000100)
    r2.w = asuint(r2.w) != 0u ? cb0[6].z : 0.0001f;
    // ge r3.w, cb0[6].z, l(0.000000)
    r3.w = asfloat((cb0[6].z >= 0.0f) ? 0xffffffffu : 0u);
    // movc r2.w, r3.w, r2.w, cb0[6].z
    r2.w = asuint(r3.w) != 0u ? r2.w : cb0[6].z;
    // div r2.w, r9.z, r2.w
    r2.w = r9.z / r2.w;
    // mul r2.w, r2.w, r2.w
    r2.w = r2.w * r2.w;
    // mul r3.w, r2.w, cb0[13].y
    r3.w = r2.w * cb0[13].y;
    // mul r1.w, r1.w, r2.w
    r1.w = r1.w * r2.w;
    // mul r0.w, r0.w, r3.w
    r0.w = r0.w * r3.w;
    // sincos r12.x, r13.x, r0.w
    float stage34_x = r0.w;
    r12.x = sin(stage34_x);
    r13.x = cos(stage34_x);
    // mul r5.xyz, r5.xyzx, r12.xxxx
    float stage35_x = r5.x * r12.x;
    float stage35_y = r5.y * r12.x;
    float stage35_z = r5.z * r12.x;
    r5.x = stage35_x;
    r5.y = stage35_y;
    r5.z = stage35_z;
    // mad r5.xyz, r11.xyzx, r13.xxxx, r5.xyzx
    float stage36_x = r11.x * r13.x + r5.x;
    float stage36_y = r11.y * r13.x + r5.y;
    float stage36_z = r11.z * r13.x + r5.z;
    r5.x = stage36_x;
    r5.y = stage36_y;
    r5.z = stage36_z;
    // add r5.xyz, r5.xyzx, r6.xyzx
    float stage37_x = r5.x + r6.x;
    float stage37_y = r5.y + r6.y;
    float stage37_z = r5.z + r6.z;
    r5.x = stage37_x;
    r5.y = stage37_y;
    r5.z = stage37_z;
    // add r5.xyz, -r8.xywx, r5.xyzx
    float stage38_x = -r8.x + r5.x;
    float stage38_y = -r8.y + r5.y;
    float stage38_z = -r8.w + r5.z;
    r5.x = stage38_x;
    r5.y = stage38_y;
    r5.z = stage38_z;
    // dp3 r0.w, r4.zwxz, r9.xyzx
    r0.w = dot(r4.zwx, r9.xyz);
    // mad r6.xyz, r4.zwxz, r0.wwww, r3.xyzx
    float stage39_x = r4.z * r0.w + r3.x;
    float stage39_y = r4.w * r0.w + r3.y;
    float stage39_z = r4.x * r0.w + r3.z;
    r6.x = stage39_x;
    r6.y = stage39_y;
    r6.z = stage39_z;
    // add r11.xyz, -r6.xyzx, r8.xyzx
    float stage40_x = -r6.x + r8.x;
    float stage40_y = -r6.y + r8.y;
    float stage40_z = -r6.z + r8.z;
    r11.x = stage40_x;
    r11.y = stage40_y;
    r11.z = stage40_z;
    // mul r12.xyz, r4.xzwx, r11.yzxy
    float stage41_x = r4.x * r11.y;
    float stage41_y = r4.z * r11.z;
    float stage41_z = r4.w * r11.x;
    r12.x = stage41_x;
    r12.y = stage41_y;
    r12.z = stage41_z;
    // mad r4.xzw, r4.wwxz, r11.zzxy, -r12.xxyz
    float stage42_x = r4.w * r11.z + -r12.x;
    float stage42_z = r4.x * r11.x + -r12.y;
    float stage42_w = r4.z * r11.y + -r12.z;
    r4.x = stage42_x;
    r4.z = stage42_z;
    r4.w = stage42_w;
    // dp3 r0.w, -cb0[7].xyzx, -cb0[7].xyzx
    r0.w = dot(-cb0[7].xyz, -cb0[7].xyz);
    // sqrt r0.w, r0.w
    r0.w = sqrt(r0.w);
    // mul r0.w, r1.w, r0.w
    r0.w = r1.w * r0.w;
    // mul r0.w, r0.w, r4.y
    r0.w = r0.w * r4.y;
    // sincos r12.x, r13.x, r0.w
    float stage43_x = r0.w;
    r12.x = sin(stage43_x);
    r13.x = cos(stage43_x);
    // mul r4.xyz, r4.xzwx, r12.xxxx
    float stage44_x = r4.x * r12.x;
    float stage44_y = r4.z * r12.x;
    float stage44_z = r4.w * r12.x;
    r4.x = stage44_x;
    r4.y = stage44_y;
    r4.z = stage44_z;
    // mad r4.xyz, r11.xyzx, r13.xxxx, r4.xyzx
    float stage45_x = r11.x * r13.x + r4.x;
    float stage45_y = r11.y * r13.x + r4.y;
    float stage45_z = r11.z * r13.x + r4.z;
    r4.x = stage45_x;
    r4.y = stage45_y;
    r4.z = stage45_z;
    // add r4.xyz, r4.xyzx, r6.xyzx
    float stage46_x = r4.x + r6.x;
    float stage46_y = r4.y + r6.y;
    float stage46_z = r4.z + r6.z;
    r4.x = stage46_x;
    r4.y = stage46_y;
    r4.z = stage46_z;
    // add r4.xyz, -r8.xyzx, r4.xyzx
    float stage47_x = -r8.x + r4.x;
    float stage47_y = -r8.y + r4.y;
    float stage47_z = -r8.z + r4.z;
    r4.x = stage47_x;
    r4.y = stage47_y;
    r4.z = stage47_z;
    // add r4.xyz, r5.xyzx, r4.xyzx
    float stage48_x = r5.x + r4.x;
    float stage48_y = r5.y + r4.y;
    float stage48_z = r5.z + r4.z;
    r4.x = stage48_x;
    r4.y = stage48_y;
    r4.z = stage48_z;
    // mul r4.xyz, r4.xyzx, v3.zzzz
    float stage49_x = r4.x * v3.z;
    float stage49_y = r4.y * v3.z;
    float stage49_z = r4.z * v3.z;
    r4.x = stage49_x;
    r4.y = stage49_y;
    r4.z = stage49_z;
    // add r5.xy, r3.xyxx, -cb0[9].xyxx
    float stage50_x = r3.x + -cb0[9].x;
    float stage50_y = r3.y + -cb0[9].y;
    r5.x = stage50_x;
    r5.y = stage50_y;
    // add r5.zw, -r3.xxxy, cb0[9].xxxy
    float stage51_z = -r3.x + cb0[9].x;
    float stage51_w = -r3.y + cb0[9].y;
    r5.z = stage51_z;
    r5.w = stage51_w;
    // dp2 r0.w, r5.zwzz, r5.zwzz
    r0.w = dot(r5.zw, r5.zw);
    // sqrt r0.w, r0.w
    r0.w = sqrt(r0.w);
    // div r5.yz, r5.xxyx, r0.wwww
    float stage52_y = r5.x / r0.w;
    float stage52_z = r5.y / r0.w;
    r5.y = stage52_y;
    r5.z = stage52_z;
    // mov r6.y, l(0)
    r6.y = asfloat(0u);
    // mov r5.x, l(0)
    r5.x = asfloat(0u);
    // mul r6.xz, r10.zzyz, r5.zzyz
    float stage53_x = r10.z * r5.z;
    float stage53_z = r10.y * r5.y;
    r6.x = stage53_x;
    r6.z = stage53_z;
    // mad r5.xyz, -r5.xyzx, r10.yzxy, r6.xyzx
    float stage54_x = -r5.x * r10.y + r6.x;
    float stage54_y = -r5.y * r10.z + r6.y;
    float stage54_z = -r5.z * r10.x + r6.z;
    r5.x = stage54_x;
    r5.y = stage54_y;
    r5.z = stage54_z;
    // dp3 r1.w, r5.xyzx, r9.xyzx
    r1.w = dot(r5.xyz, r9.xyz);
    // mad r6.xyz, r5.xyzx, r1.wwww, r3.xyzx
    float stage55_x = r5.x * r1.w + r3.x;
    float stage55_y = r5.y * r1.w + r3.y;
    float stage55_z = r5.z * r1.w + r3.z;
    r6.x = stage55_x;
    r6.y = stage55_y;
    r6.z = stage55_z;
    // add r10.xyz, -r6.xyzx, r8.xyzx
    float stage56_x = -r6.x + r8.x;
    float stage56_y = -r6.y + r8.y;
    float stage56_z = -r6.z + r8.z;
    r10.x = stage56_x;
    r10.y = stage56_y;
    r10.z = stage56_z;
    // mul r11.xyz, r5.zxyz, r10.yzxy
    float stage57_x = r5.z * r10.y;
    float stage57_y = r5.x * r10.z;
    float stage57_z = r5.y * r10.x;
    r11.x = stage57_x;
    r11.y = stage57_y;
    r11.z = stage57_z;
    // mad r11.xyz, r5.yzxy, r10.zxyz, -r11.xyzx
    float stage58_x = r5.y * r10.z + -r11.x;
    float stage58_y = r5.z * r10.x + -r11.y;
    float stage58_z = r5.x * r10.y + -r11.z;
    r11.x = stage58_x;
    r11.y = stage58_y;
    r11.z = stage58_z;
    // div r1.w, r9.z, cb0[6].z
    r1.w = r9.z / cb0[6].z;
    // mul r1.w, r1.w, r1.w
    r1.w = r1.w * r1.w;
    // mul r5.w, r1.w, cb0[10].w
    r5.w = r1.w * cb0[10].w;
    // sincos r12.x, r13.x, r5.w
    float stage59_x = r5.w;
    r12.x = sin(stage59_x);
    r13.x = cos(stage59_x);
    // mul r5.xyzw, r5.xyzw, cb0[11].xxxx
    float stage60_x = r5.x * cb0[11].x;
    float stage60_y = r5.y * cb0[11].x;
    float stage60_z = r5.z * cb0[11].x;
    float stage60_w = r5.w * cb0[11].x;
    r5.x = stage60_x;
    r5.y = stage60_y;
    r5.z = stage60_z;
    r5.w = stage60_w;
    // mul r11.xyz, r11.xyzx, r12.xxxx
    float stage61_x = r11.x * r12.x;
    float stage61_y = r11.y * r12.x;
    float stage61_z = r11.z * r12.x;
    r11.x = stage61_x;
    r11.y = stage61_y;
    r11.z = stage61_z;
    // mad r10.xyz, r10.xyzx, r13.xxxx, r11.xyzx
    float stage62_x = r10.x * r13.x + r11.x;
    float stage62_y = r10.y * r13.x + r11.y;
    float stage62_z = r10.z * r13.x + r11.z;
    r10.x = stage62_x;
    r10.y = stage62_y;
    r10.z = stage62_z;
    // add r6.xyz, r6.xyzx, r10.xyzx
    float stage63_x = r6.x + r10.x;
    float stage63_y = r6.y + r10.y;
    float stage63_z = r6.z + r10.z;
    r6.x = stage63_x;
    r6.y = stage63_y;
    r6.z = stage63_z;
    // add r6.xyz, -r8.xyzx, r6.xyzx
    float stage64_x = -r8.x + r6.x;
    float stage64_y = -r8.y + r6.y;
    float stage64_z = -r8.z + r6.z;
    r6.x = stage64_x;
    r6.y = stage64_y;
    r6.z = stage64_z;
    // min r1.w, r0.w, cb0[10].z
    r1.w = min(r0.w, cb0[10].z);
    // mul_sat r0.w, r0.w, cb0[11].y
    r0.w = saturate(r0.w * cb0[11].y);
    // div r1.w, r1.w, cb0[10].z
    r1.w = r1.w / cb0[10].z;
    // add r1.w, -r1.w, l(1.000000)
    r1.w = -r1.w + 1.0f;
    // mul r6.xyz, r6.xyzx, r1.wwww
    float stage65_x = r6.x * r1.w;
    float stage65_y = r6.y * r1.w;
    float stage65_z = r6.z * r1.w;
    r6.x = stage65_x;
    r6.y = stage65_y;
    r6.z = stage65_z;
    // mul r6.xyz, r0.wwww, r6.xyzx
    float stage66_x = r0.w * r6.x;
    float stage66_y = r0.w * r6.y;
    float stage66_z = r0.w * r6.z;
    r6.x = stage66_x;
    r6.y = stage66_y;
    r6.z = stage66_z;
    // mul r6.xyz, r6.xyzx, cb0[12].xxxx
    float stage67_x = r6.x * cb0[12].x;
    float stage67_y = r6.y * cb0[12].x;
    float stage67_z = r6.z * cb0[12].x;
    r6.x = stage67_x;
    r6.y = stage67_y;
    r6.z = stage67_z;
    // mad r4.xyz, r6.xyzx, r4.xyzx, r4.xyzx
    float stage68_x = r6.x * r4.x + r4.x;
    float stage68_y = r6.y * r4.y + r4.y;
    float stage68_z = r6.z * r4.z + r4.z;
    r4.x = stage68_x;
    r4.y = stage68_y;
    r4.z = stage68_z;
    // dp3 r2.w, r5.xyzx, r9.xyzx
    r2.w = dot(r5.xyz, r9.xyz);
    // mad r3.xyz, r5.xyzx, r2.wwww, r3.xyzx
    float stage69_x = r5.x * r2.w + r3.x;
    float stage69_y = r5.y * r2.w + r3.y;
    float stage69_z = r5.z * r2.w + r3.z;
    r3.x = stage69_x;
    r3.y = stage69_y;
    r3.z = stage69_z;
    // add r6.xyz, -r3.xyzx, r8.xyzx
    float stage70_x = -r3.x + r8.x;
    float stage70_y = -r3.y + r8.y;
    float stage70_z = -r3.z + r8.z;
    r6.x = stage70_x;
    r6.y = stage70_y;
    r6.z = stage70_z;
    // mul r9.xyz, r5.zxyz, r6.yzxy
    float stage71_x = r5.z * r6.y;
    float stage71_y = r5.x * r6.z;
    float stage71_z = r5.y * r6.x;
    r9.x = stage71_x;
    r9.y = stage71_y;
    r9.z = stage71_z;
    // mad r5.xyz, r5.yzxy, r6.zxyz, -r9.xyzx
    float stage72_x = r5.y * r6.z + -r9.x;
    float stage72_y = r5.z * r6.x + -r9.y;
    float stage72_z = r5.x * r6.y + -r9.z;
    r5.x = stage72_x;
    r5.y = stage72_y;
    r5.z = stage72_z;
    // sincos r9.x, r10.x, r5.w
    float stage73_x = r5.w;
    r9.x = sin(stage73_x);
    r10.x = cos(stage73_x);
    // mul r5.xyz, r5.xyzx, r9.xxxx
    float stage74_x = r5.x * r9.x;
    float stage74_y = r5.y * r9.x;
    float stage74_z = r5.z * r9.x;
    r5.x = stage74_x;
    r5.y = stage74_y;
    r5.z = stage74_z;
    // mad r5.xyz, r6.xyzx, r10.xxxx, r5.xyzx
    float stage75_x = r6.x * r10.x + r5.x;
    float stage75_y = r6.y * r10.x + r5.y;
    float stage75_z = r6.z * r10.x + r5.z;
    r5.x = stage75_x;
    r5.y = stage75_y;
    r5.z = stage75_z;
    // add r3.xyz, r3.xyzx, r5.xyzx
    float stage76_x = r3.x + r5.x;
    float stage76_y = r3.y + r5.y;
    float stage76_z = r3.z + r5.z;
    r3.x = stage76_x;
    r3.y = stage76_y;
    r3.z = stage76_z;
    // add r3.xyz, -r8.xyzx, r3.xyzx
    float stage77_x = -r8.x + r3.x;
    float stage77_y = -r8.y + r3.y;
    float stage77_z = -r8.z + r3.z;
    r3.x = stage77_x;
    r3.y = stage77_y;
    r3.z = stage77_z;
    // mul r3.xyz, r1.wwww, r3.xyzx
    float stage78_x = r1.w * r3.x;
    float stage78_y = r1.w * r3.y;
    float stage78_z = r1.w * r3.z;
    r3.x = stage78_x;
    r3.y = stage78_y;
    r3.z = stage78_z;
    // mad r3.xyz, r0.wwww, r3.xyzx, r4.xyzx
    float stage79_x = r0.w * r3.x + r4.x;
    float stage79_y = r0.w * r3.y + r4.y;
    float stage79_z = r0.w * r3.z + r4.z;
    r3.x = stage79_x;
    r3.y = stage79_y;
    r3.z = stage79_z;
    // add r3.xyz, r3.xyzx, r7.xyzx
    float stage80_x = r3.x + r7.x;
    float stage80_y = r3.y + r7.y;
    float stage80_z = r3.z + r7.z;
    r3.x = stage80_x;
    r3.y = stage80_y;
    r3.z = stage80_z;
    float3 delta=r3.xyz-r7.xyz;
    if(any(!isfinite(delta))) return 0;
    return float3(delta.x,delta.z,-delta.y)*.01f;
}

// Native shader 1c39a832e3e5f745a865324bed15057e, displacement instructions 28..143.
float3 SourceFoliageOffsetProgram3(float3 worldPosition, float4 vertexColor, float4x4 world,
    float4 sourceOwner, float4 sourceDimensionsAndRadius)
{
    float4 cb0[14]; float4 cb1[6];
    [unroll] for(uint i=0;i<14;++i) cb0[i]=0;
    [unroll] for(uint j=0;j<6;++j) cb1[j]=0;
    cb0[0]=float4(1,0,0,0);cb0[1]=float4(0,1,0,0);
    cb0[2]=float4(0,0,1,0);cb0[3]=float4(0,0,0,1);
    float3 worldExtent = abs(world[0].xyz)*g_SourceFoliageWindLocalBounds.x+
        abs(world[1].xyz)*g_SourceFoliageWindLocalBounds.y+abs(world[2].xyz)*g_SourceFoliageWindLocalBounds.z;
    float3 columnSq=world[0].xyz*world[0].xyz+world[1].xyz*world[1].xyz+world[2].xyz*world[2].xyz;
    float maxScale=sqrt(max(columnSq.x,max(columnSq.y,columnSq.z)));
    float4 dimensionsAndRadius=sourceOwner.w==1.f ? sourceDimensionsAndRadius :
        float4(worldExtent.x,worldExtent.z,worldExtent.y,g_SourceFoliageWindLocalBounds.w*maxScale)*100.f+1.f;
    float4 actorPosition=sourceOwner.w==1.f ? float4(sourceOwner.xyz,0) : g_SourceFoliageWindActorPosition;
    float4 center=float4(SourceFoliageToUE(mul(g_SourceFoliageWindLocalCenter,world).xyz),1);
    cb0[4]=actorPosition; cb0[5].xyz=dimensionsAndRadius.xyz;
    cb0[6]=g_SourceFoliageWindDirectionSpeed; cb0[7]=g_SourceFoliageWindPlayerPosition;
    cb0[8]=center;
    [unroll] for(uint k=0;k<3;++k) cb0[9+k]=g_SourceFoliageWindScalars[k];
    cb0[10].x=g_SourceFoliageWindTime;
    float4 v0=float4(SourceFoliageToUE(worldPosition),1);
    // Native packed VF color is BGRA; the installed WModel carrier is RGBA.
    float4 v3=vertexColor.bgra;
    float4 r0=0,r1=0,r2=0,r3=0,r4=0,r5=0,r6=0,r7=0,r8=0,r9=0,r10=0,r11=0,r12=0,r13=0,r14=0,r15=0,r16=0;
    // mov r3.w, l(0)
    r3.w = asfloat(0u);
    // frc r4.xy, cb0[4].xyxx
    float stage81_x = frac(cb0[4].x);
    float stage81_y = frac(cb0[4].y);
    r4.x = stage81_x;
    r4.y = stage81_y;
    // mad r4.xy, r4.xyxx, r4.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    float stage82_x = r4.x * r4.x + -0.5f;
    float stage82_y = r4.y * r4.y + -0.5f;
    r4.x = stage82_x;
    r4.y = stage82_y;
    // add r4.xy, r4.xyxx, r4.xyxx
    float stage83_x = r4.x + r4.x;
    float stage83_y = r4.y + r4.y;
    r4.x = stage83_x;
    r4.y = stage83_y;
    // add r0.w, r4.y, r4.x
    r0.w = r4.y + r4.x;
    // mul r0.w, r0.w, cb0[10].y
    r0.w = r0.w * cb0[10].y;
    // mul r1.w, cb0[6].w, cb0[10].x
    r1.w = cb0[6].w * cb0[10].x;
    // mad r2.w, cb0[9].w, r1.w, r0.w
    r2.w = cb0[9].w * r1.w + r0.w;
    // mul r4.xyz, r2.wwww, l(1.086065, 0.989267, 3.141593, 0.000000)
    float stage84_x = r2.w * 1.086065f;
    float stage84_y = r2.w * 0.989267f;
    float stage84_z = r2.w * 3.141593f;
    r4.x = stage84_x;
    r4.y = stage84_y;
    r4.z = stage84_z;
    // sincos r4.xyz, null, r4.xyzx
    float stage85_x = r4.x;
    float stage85_y = r4.y;
    float stage85_z = r4.z;
    float stage86_x = sin(stage85_x);
    float stage86_y = sin(stage85_y);
    float stage86_z = sin(stage85_z);
    r4.x = stage86_x;
    r4.y = stage86_y;
    r4.z = stage86_z;
    // add r4.xyz, r4.xyzx, l(1.000000, 1.666667, 1.000000, 0.000000)
    float stage87_x = r4.x + 1.0f;
    float stage87_y = r4.y + 1.666667f;
    float stage87_z = r4.z + 1.0f;
    r4.x = stage87_x;
    r4.y = stage87_y;
    r4.z = stage87_z;
    // mul r4.xy, r4.xyxx, l(0.500000, 0.375000, 0.000000, 0.000000)
    float stage88_x = r4.x * 0.5f;
    float stage88_y = r4.y * 0.375f;
    r4.x = stage88_x;
    r4.y = stage88_y;
    // mul r2.w, r4.y, r4.z
    r2.w = r4.y * r4.z;
    // mul r2.w, r2.w, l(-0.500000)
    r2.w = r2.w * -0.5f;
    // dp3 r4.y, cb0[6].xyzx, cb0[6].xyzx
    r4.y = dot(cb0[6].xyz, cb0[6].xyz);
    // sqrt r4.y, r4.y
    r4.y = sqrt(r4.y);
    // div r4.yzw, cb0[6].xxyz, r4.yyyy
    float stage89_y = cb0[6].x / r4.y;
    float stage89_z = cb0[6].y / r4.y;
    float stage89_w = cb0[6].z / r4.y;
    r4.y = stage89_y;
    r4.z = stage89_z;
    r4.w = stage89_w;
    // mul r5.xyz, r4.zwyz, l(0.000000, 0.000000, 1.000000, 0.000000)
    float stage90_x = r4.z * 0.0f;
    float stage90_y = r4.w * 0.0f;
    float stage90_z = r4.y * 1.0f;
    r5.x = stage90_x;
    r5.y = stage90_y;
    r5.z = stage90_z;
    // mad r5.xyz, r4.yzwy, l(0.000000, 1.000000, 0.000000, 0.000000), -r5.xyzx
    float stage91_x = r4.y * 0.0f + -r5.x;
    float stage91_y = r4.z * 1.0f + -r5.y;
    float stage91_z = r4.w * 0.0f + -r5.z;
    r5.x = stage91_x;
    r5.y = stage91_y;
    r5.z = stage91_z;
    // add r5.xyz, -r4.wyzw, r5.xyzx
    float stage92_x = -r4.w + r5.x;
    float stage92_y = -r4.y + r5.y;
    float stage92_z = -r4.z + r5.z;
    r5.x = stage92_x;
    r5.y = stage92_y;
    r5.z = stage92_z;
    // mad r5.xyz, r4.xxxx, r5.xyzx, r4.wyzw
    float stage93_x = r4.x * r5.x + r4.w;
    float stage93_y = r4.x * r5.y + r4.y;
    float stage93_z = r4.x * r5.z + r4.z;
    r5.x = stage93_x;
    r5.y = stage93_y;
    r5.z = stage93_z;
    // add r6.xyz, r5.xyzx, l(-1.000000, -0.000000, -0.000000, 0.000000)
    float stage94_x = r5.x + -1.0f;
    float stage94_y = r5.y + -0.0f;
    float stage94_z = r5.z + -0.0f;
    r6.x = stage94_x;
    r6.y = stage94_y;
    r6.z = stage94_z;
    // mul r7.xyz, cb0[1].xyzx, cb0[8].yyyy
    float stage95_x = cb0[1].x * cb0[8].y;
    float stage95_y = cb0[1].y * cb0[8].y;
    float stage95_z = cb0[1].z * cb0[8].y;
    r7.x = stage95_x;
    r7.y = stage95_y;
    r7.z = stage95_z;
    // mad r7.xyz, cb0[0].xyzx, cb0[8].xxxx, r7.xyzx
    float stage96_x = cb0[0].x * cb0[8].x + r7.x;
    float stage96_y = cb0[0].y * cb0[8].x + r7.y;
    float stage96_z = cb0[0].z * cb0[8].x + r7.z;
    r7.x = stage96_x;
    r7.y = stage96_y;
    r7.z = stage96_z;
    // mad r7.xyz, cb0[2].xyzx, cb0[8].zzzz, r7.xyzx
    float stage97_x = cb0[2].x * cb0[8].z + r7.x;
    float stage97_y = cb0[2].y * cb0[8].z + r7.y;
    float stage97_z = cb0[2].z * cb0[8].z + r7.z;
    r7.x = stage97_x;
    r7.y = stage97_y;
    r7.z = stage97_z;
    // mad r7.xyz, cb0[3].xyzx, cb0[8].wwww, r7.xyzx
    float stage98_x = cb0[3].x * cb0[8].w + r7.x;
    float stage98_y = cb0[3].y * cb0[8].w + r7.y;
    float stage98_z = cb0[3].z * cb0[8].w + r7.z;
    r7.x = stage98_x;
    r7.y = stage98_y;
    r7.z = stage98_z;
    // add r3.xyz, r7.xyzx, -cb1[5].xyzx
    float stage99_x = r7.x + -cb1[5].x;
    float stage99_y = r7.y + -cb1[5].y;
    float stage99_z = r7.z + -cb1[5].z;
    r3.x = stage99_x;
    r3.y = stage99_y;
    r3.z = stage99_z;
    // mul r7.xyzw, v0.yyyy, cb0[1].xyzw
    float stage100_x = v0.y * cb0[1].x;
    float stage100_y = v0.y * cb0[1].y;
    float stage100_z = v0.y * cb0[1].z;
    float stage100_w = v0.y * cb0[1].w;
    r7.x = stage100_x;
    r7.y = stage100_y;
    r7.z = stage100_z;
    r7.w = stage100_w;
    // mad r7.xyzw, cb0[0].xyzw, v0.xxxx, r7.xyzw
    float stage101_x = cb0[0].x * v0.x + r7.x;
    float stage101_y = cb0[0].y * v0.x + r7.y;
    float stage101_z = cb0[0].z * v0.x + r7.z;
    float stage101_w = cb0[0].w * v0.x + r7.w;
    r7.x = stage101_x;
    r7.y = stage101_y;
    r7.z = stage101_z;
    r7.w = stage101_w;
    // mad r7.xyzw, cb0[2].xyzw, v0.zzzz, r7.xyzw
    float stage102_x = cb0[2].x * v0.z + r7.x;
    float stage102_y = cb0[2].y * v0.z + r7.y;
    float stage102_z = cb0[2].z * v0.z + r7.z;
    float stage102_w = cb0[2].w * v0.z + r7.w;
    r7.x = stage102_x;
    r7.y = stage102_y;
    r7.z = stage102_z;
    r7.w = stage102_w;
    // mad r7.xyzw, cb0[3].xyzw, v0.wwww, r7.xyzw
    float stage103_x = cb0[3].x * v0.w + r7.x;
    float stage103_y = cb0[3].y * v0.w + r7.y;
    float stage103_z = cb0[3].z * v0.w + r7.z;
    float stage103_w = cb0[3].w * v0.w + r7.w;
    r7.x = stage103_x;
    r7.y = stage103_y;
    r7.z = stage103_z;
    r7.w = stage103_w;
    // add r8.xyz, r7.xyzx, -cb1[5].xyzx
    float stage104_x = r7.x + -cb1[5].x;
    float stage104_y = r7.y + -cb1[5].y;
    float stage104_z = r7.z + -cb1[5].z;
    r8.x = stage104_x;
    r8.y = stage104_y;
    r8.z = stage104_z;
    // add r9.xyz, -r3.xyzx, r8.xyzx
    float stage105_x = -r3.x + r8.x;
    float stage105_y = -r3.y + r8.y;
    float stage105_z = -r3.z + r8.z;
    r9.x = stage105_x;
    r9.y = stage105_y;
    r9.z = stage105_z;
    // dp3 r4.x, r9.xyzx, r9.xyzx
    r4.x = dot(r9.xyz, r9.xyz);
    // sqrt r4.x, r4.x
    r4.x = sqrt(r4.x);
    // div r10.xyz, r9.xyzx, r4.xxxx
    float stage106_x = r9.x / r4.x;
    float stage106_y = r9.y / r4.x;
    float stage106_z = r9.z / r4.x;
    r10.x = stage106_x;
    r10.y = stage106_y;
    r10.z = stage106_z;
    // dp3 r4.x, r10.xyzx, r4.yzwy
    r4.x = dot(r10.xyz, r4.yzw);
    // mad r4.xyz, |r4.xxxx|, r6.xyzx, l(1.000000, 0.000000, 0.000000, 0.000000)
    float stage107_x = abs(r4.x) * r6.x + 1.0f;
    float stage107_y = abs(r4.x) * r6.y + 0.0f;
    float stage107_z = abs(r4.x) * r6.z + 0.0f;
    r4.x = stage107_x;
    r4.y = stage107_y;
    r4.z = stage107_z;
    // add r6.xy, -r3.xyxx, r8.xyxx
    float stage108_x = -r3.x + r8.x;
    float stage108_y = -r3.y + r8.y;
    r6.x = stage108_x;
    r6.y = stage108_y;
    // dp2 r4.w, r4.yzyy, r6.xyxx
    r4.w = dot(r4.yz, r6.xy);
    // mad r6.xyz, r4.yzxy, r4.wwww, r3.xywx
    float stage109_x = r4.y * r4.w + r3.x;
    float stage109_y = r4.z * r4.w + r3.y;
    float stage109_z = r4.x * r4.w + r3.w;
    r6.x = stage109_x;
    r6.y = stage109_y;
    r6.z = stage109_z;
    // mov r8.w, l(0)
    r8.w = asfloat(0u);
    // add r11.xyz, -r6.xyzx, r8.xywx
    float stage110_x = -r6.x + r8.x;
    float stage110_y = -r6.y + r8.y;
    float stage110_z = -r6.z + r8.w;
    r11.x = stage110_x;
    r11.y = stage110_y;
    r11.z = stage110_z;
    // mul r12.xyz, r4.xyzx, r11.yzxy
    float stage111_x = r4.x * r11.y;
    float stage111_y = r4.y * r11.z;
    float stage111_z = r4.z * r11.x;
    r12.x = stage111_x;
    r12.y = stage111_y;
    r12.z = stage111_z;
    // mad r4.xyz, r4.zxyz, r11.zxyz, -r12.xyzx
    float stage112_x = r4.z * r11.z + -r12.x;
    float stage112_y = r4.x * r11.x + -r12.y;
    float stage112_z = r4.y * r11.y + -r12.z;
    r4.x = stage112_x;
    r4.y = stage112_y;
    r4.z = stage112_z;
    // add r3.w, r8.z, r8.y
    r3.w = r8.z + r8.y;
    // add r3.w, r3.w, r8.x
    r3.w = r3.w + r8.x;
    // mul r3.w, r3.w, cb0[11].y
    r3.w = r3.w * cb0[11].y;
    // mad r0.w, r3.w, l(0.010000), r0.w
    r0.w = r3.w * 0.01f + r0.w;
    // mad r0.w, cb0[11].x, r1.w, r0.w
    r0.w = cb0[11].x * r1.w + r0.w;
    // mul r0.w, r0.w, l(6.283185)
    r0.w = r0.w * 6.283185f;
    // sincos r0.w, null, r0.w
    float stage113_w = r0.w;
    r0.w = sin(stage113_w);
    // lt r1.w, l(0.000000), cb0[5].z
    r1.w = asfloat((0.0f < cb0[5].z) ? 0xffffffffu : 0u);
    // movc r1.w, r1.w, cb0[5].z, l(0.000100)
    r1.w = asuint(r1.w) != 0u ? cb0[5].z : 0.0001f;
    // ge r3.w, cb0[5].z, l(0.000000)
    r3.w = asfloat((cb0[5].z >= 0.0f) ? 0xffffffffu : 0u);
    // movc r1.w, r3.w, r1.w, cb0[5].z
    r1.w = asuint(r3.w) != 0u ? r1.w : cb0[5].z;
    // div r1.w, r9.z, r1.w
    r1.w = r9.z / r1.w;
    // mul r1.w, r1.w, r1.w
    r1.w = r1.w * r1.w;
    // mul r12.xy, r1.wwww, cb0[10].zwzz
    float stage114_x = r1.w * cb0[10].z;
    float stage114_y = r1.w * cb0[10].w;
    r12.x = stage114_x;
    r12.y = stage114_y;
    // mul r1.w, r1.w, cb0[9].y
    r1.w = r1.w * cb0[9].y;
    // sincos r13.x, r14.x, r1.w
    float stage115_x = r1.w;
    r13.x = sin(stage115_x);
    r14.x = cos(stage115_x);
    // mul r0.w, r0.w, r12.y
    r0.w = r0.w * r12.y;
    // sincos r15.x, r16.x, r0.w
    float stage116_x = r0.w;
    r15.x = sin(stage116_x);
    r16.x = cos(stage116_x);
    // mul r4.xyz, r4.xyzx, r15.xxxx
    float stage117_x = r4.x * r15.x;
    float stage117_y = r4.y * r15.x;
    float stage117_z = r4.z * r15.x;
    r4.x = stage117_x;
    r4.y = stage117_y;
    r4.z = stage117_z;
    // mad r4.xyz, r11.xyzx, r16.xxxx, r4.xyzx
    float stage118_x = r11.x * r16.x + r4.x;
    float stage118_y = r11.y * r16.x + r4.y;
    float stage118_z = r11.z * r16.x + r4.z;
    r4.x = stage118_x;
    r4.y = stage118_y;
    r4.z = stage118_z;
    // add r4.xyz, r4.xyzx, r6.xyzx
    float stage119_x = r4.x + r6.x;
    float stage119_y = r4.y + r6.y;
    float stage119_z = r4.z + r6.z;
    r4.x = stage119_x;
    r4.y = stage119_y;
    r4.z = stage119_z;
    // add r4.xyz, -r8.xywx, r4.xyzx
    float stage120_x = -r8.x + r4.x;
    float stage120_y = -r8.y + r4.y;
    float stage120_z = -r8.w + r4.z;
    r4.x = stage120_x;
    r4.y = stage120_y;
    r4.z = stage120_z;
    // dp3 r0.w, r5.yzxy, r9.xyzx
    r0.w = dot(r5.yzx, r9.xyz);
    // mad r6.xyz, r5.yzxy, r0.wwww, r3.xyzx
    float stage121_x = r5.y * r0.w + r3.x;
    float stage121_y = r5.z * r0.w + r3.y;
    float stage121_z = r5.x * r0.w + r3.z;
    r6.x = stage121_x;
    r6.y = stage121_y;
    r6.z = stage121_z;
    // add r11.xyz, -r6.xyzx, r8.xyzx
    float stage122_x = -r6.x + r8.x;
    float stage122_y = -r6.y + r8.y;
    float stage122_z = -r6.z + r8.z;
    r11.x = stage122_x;
    r11.y = stage122_y;
    r11.z = stage122_z;
    // mul r12.yzw, r5.xxyz, r11.yyzx
    float stage123_y = r5.x * r11.y;
    float stage123_z = r5.y * r11.z;
    float stage123_w = r5.z * r11.x;
    r12.y = stage123_y;
    r12.z = stage123_z;
    r12.w = stage123_w;
    // mad r5.xyz, r5.zxyz, r11.zxyz, -r12.yzwy
    float stage124_x = r5.z * r11.z + -r12.y;
    float stage124_y = r5.x * r11.x + -r12.z;
    float stage124_z = r5.y * r11.y + -r12.w;
    r5.x = stage124_x;
    r5.y = stage124_y;
    r5.z = stage124_z;
    // dp3 r0.w, -cb0[6].xyzx, -cb0[6].xyzx
    r0.w = dot(-cb0[6].xyz, -cb0[6].xyz);
    // sqrt r0.w, r0.w
    r0.w = sqrt(r0.w);
    // mul r0.w, r12.x, r0.w
    r0.w = r12.x * r0.w;
    // mul r0.w, r0.w, r2.w
    r0.w = r0.w * r2.w;
    // sincos r12.x, r15.x, r0.w
    float stage125_x = r0.w;
    r12.x = sin(stage125_x);
    r15.x = cos(stage125_x);
    // mul r5.xyz, r5.xyzx, r12.xxxx
    float stage126_x = r5.x * r12.x;
    float stage126_y = r5.y * r12.x;
    float stage126_z = r5.z * r12.x;
    r5.x = stage126_x;
    r5.y = stage126_y;
    r5.z = stage126_z;
    // mad r5.xyz, r11.xyzx, r15.xxxx, r5.xyzx
    float stage127_x = r11.x * r15.x + r5.x;
    float stage127_y = r11.y * r15.x + r5.y;
    float stage127_z = r11.z * r15.x + r5.z;
    r5.x = stage127_x;
    r5.y = stage127_y;
    r5.z = stage127_z;
    // add r5.xyz, r5.xyzx, r6.xyzx
    float stage128_x = r5.x + r6.x;
    float stage128_y = r5.y + r6.y;
    float stage128_z = r5.z + r6.z;
    r5.x = stage128_x;
    r5.y = stage128_y;
    r5.z = stage128_z;
    // add r5.xyz, -r8.xyzx, r5.xyzx
    float stage129_x = -r8.x + r5.x;
    float stage129_y = -r8.y + r5.y;
    float stage129_z = -r8.z + r5.z;
    r5.x = stage129_x;
    r5.y = stage129_y;
    r5.z = stage129_z;
    // add r4.xyz, r4.xyzx, r5.xyzx
    float stage130_x = r4.x + r5.x;
    float stage130_y = r4.y + r5.y;
    float stage130_z = r4.z + r5.z;
    r4.x = stage130_x;
    r4.y = stage130_y;
    r4.z = stage130_z;
    // mul r4.xyz, r4.xyzx, v3.zzzz
    float stage131_x = r4.x * v3.z;
    float stage131_y = r4.y * v3.z;
    float stage131_z = r4.z * v3.z;
    r4.x = stage131_x;
    r4.y = stage131_y;
    r4.z = stage131_z;
    // add r5.xy, r3.yxyy, -cb0[7].yxyy
    float stage132_x = r3.y + -cb0[7].y;
    float stage132_y = r3.x + -cb0[7].x;
    r5.x = stage132_x;
    r5.y = stage132_y;
    // add r5.zw, -r3.xxxy, cb0[7].xxxy
    float stage133_z = -r3.x + cb0[7].x;
    float stage133_w = -r3.y + cb0[7].y;
    r5.z = stage133_z;
    r5.w = stage133_w;
    // dp2 r0.w, r5.zwzz, r5.zwzz
    r0.w = dot(r5.zw, r5.zw);
    // sqrt r0.w, r0.w
    r0.w = sqrt(r0.w);
    // lt r1.w, l(0.000000), r0.w
    r1.w = asfloat((0.0f < r0.w) ? 0xffffffffu : 0u);
    // movc r1.w, r1.w, r0.w, l(0.000100)
    r1.w = asuint(r1.w) != 0u ? r0.w : 0.0001f;
    // ge r2.w, r0.w, l(0.000000)
    r2.w = asfloat((r0.w >= 0.0f) ? 0xffffffffu : 0u);
    // movc r1.w, r2.w, r1.w, r0.w
    r1.w = asuint(r2.w) != 0u ? r1.w : r0.w;
    // div r5.xz, r5.xxyx, r1.wwww
    float stage134_x = r5.x / r1.w;
    float stage134_z = r5.y / r1.w;
    r5.x = stage134_x;
    r5.z = stage134_z;
    // mul r6.xy, r10.yzyy, r5.zxzz
    float stage135_x = r10.y * r5.z;
    float stage135_y = r10.z * r5.x;
    r6.x = stage135_x;
    r6.y = stage135_y;
    // mov r6.z, l(0)
    r6.z = asfloat(0u);
    // mov r5.y, l(0)
    r5.y = asfloat(0u);
    // mad r5.xyz, -r5.xyzx, r10.xyzx, r6.xyzx
    float stage136_x = -r5.x * r10.x + r6.x;
    float stage136_y = -r5.y * r10.y + r6.y;
    float stage136_z = -r5.z * r10.z + r6.z;
    r5.x = stage136_x;
    r5.y = stage136_y;
    r5.z = stage136_z;
    // dp3 r1.w, r5.yzxy, r9.xyzx
    r1.w = dot(r5.yzx, r9.xyz);
    // mad r3.xyz, r5.yzxy, r1.wwww, r3.xyzx
    float stage137_x = r5.y * r1.w + r3.x;
    float stage137_y = r5.z * r1.w + r3.y;
    float stage137_z = r5.x * r1.w + r3.z;
    r3.x = stage137_x;
    r3.y = stage137_y;
    r3.z = stage137_z;
    // add r6.xyz, -r3.xyzx, r8.xyzx
    float stage138_x = -r3.x + r8.x;
    float stage138_y = -r3.y + r8.y;
    float stage138_z = -r3.z + r8.z;
    r6.x = stage138_x;
    r6.y = stage138_y;
    r6.z = stage138_z;
    // mul r9.xyz, r5.xyzx, r6.yzxy
    float stage139_x = r5.x * r6.y;
    float stage139_y = r5.y * r6.z;
    float stage139_z = r5.z * r6.x;
    r9.x = stage139_x;
    r9.y = stage139_y;
    r9.z = stage139_z;
    // mad r5.xyz, r5.zxyz, r6.zxyz, -r9.xyzx
    float stage140_x = r5.z * r6.z + -r9.x;
    float stage140_y = r5.x * r6.x + -r9.y;
    float stage140_z = r5.y * r6.y + -r9.z;
    r5.x = stage140_x;
    r5.y = stage140_y;
    r5.z = stage140_z;
    // mul r5.xyz, r13.xxxx, r5.xyzx
    float stage141_x = r13.x * r5.x;
    float stage141_y = r13.x * r5.y;
    float stage141_z = r13.x * r5.z;
    r5.x = stage141_x;
    r5.y = stage141_y;
    r5.z = stage141_z;
    // mad r5.xyz, r6.xyzx, r14.xxxx, r5.xyzx
    float stage142_x = r6.x * r14.x + r5.x;
    float stage142_y = r6.y * r14.x + r5.y;
    float stage142_z = r6.z * r14.x + r5.z;
    r5.x = stage142_x;
    r5.y = stage142_y;
    r5.z = stage142_z;
    // add r3.xyz, r3.xyzx, r5.xyzx
    float stage143_x = r3.x + r5.x;
    float stage143_y = r3.y + r5.y;
    float stage143_z = r3.z + r5.z;
    r3.x = stage143_x;
    r3.y = stage143_y;
    r3.z = stage143_z;
    // add r3.xyz, -r8.xyzx, r3.xyzx
    float stage144_x = -r8.x + r3.x;
    float stage144_y = -r8.y + r3.y;
    float stage144_z = -r8.z + r3.z;
    r3.x = stage144_x;
    r3.y = stage144_y;
    r3.z = stage144_z;
    // min r1.w, r0.w, cb0[9].x
    r1.w = min(r0.w, cb0[9].x);
    // mul_sat r0.w, r0.w, cb0[9].z
    r0.w = saturate(r0.w * cb0[9].z);
    // lt r2.w, l(0.000000), cb0[9].x
    r2.w = asfloat((0.0f < cb0[9].x) ? 0xffffffffu : 0u);
    // movc r2.w, r2.w, cb0[9].x, l(0.000100)
    r2.w = asuint(r2.w) != 0u ? cb0[9].x : 0.0001f;
    // ge r3.w, cb0[9].x, l(0.000000)
    r3.w = asfloat((cb0[9].x >= 0.0f) ? 0xffffffffu : 0u);
    // movc r2.w, r3.w, r2.w, cb0[9].x
    r2.w = asuint(r3.w) != 0u ? r2.w : cb0[9].x;
    // div r1.w, r1.w, r2.w
    r1.w = r1.w / r2.w;
    // add r1.w, -r1.w, l(1.000000)
    r1.w = -r1.w + 1.0f;
    // mul r3.xyz, r3.xyzx, r1.wwww
    float stage145_x = r3.x * r1.w;
    float stage145_y = r3.y * r1.w;
    float stage145_z = r3.z * r1.w;
    r3.x = stage145_x;
    r3.y = stage145_y;
    r3.z = stage145_z;
    // mad r3.xyz, r0.wwww, r3.xyzx, r4.xyzx
    float stage146_x = r0.w * r3.x + r4.x;
    float stage146_y = r0.w * r3.y + r4.y;
    float stage146_z = r0.w * r3.z + r4.z;
    r3.x = stage146_x;
    r3.y = stage146_y;
    r3.z = stage146_z;
    // add r3.xyz, r3.xyzx, r7.xyzx
    float stage147_x = r3.x + r7.x;
    float stage147_y = r3.y + r7.y;
    float stage147_z = r3.z + r7.z;
    r3.x = stage147_x;
    r3.y = stage147_y;
    r3.z = stage147_z;
    float3 delta=r3.xyz-r7.xyz;
    if(any(!isfinite(delta))) return 0;
    return float3(delta.x,delta.z,-delta.y)*.01f;
}

// Native shader 098c9d03d56cd64497e15be7ff9e16a3, displacement instructions 28..111.
float3 SourceFoliageOffsetProgram4(float3 worldPosition, float4 vertexColor, float4x4 world,
    float4 sourceOwner, float4 sourceDimensionsAndRadius)
{
    float4 cb0[11]; float4 cb1[6];
    [unroll] for(uint i=0;i<11;++i) cb0[i]=0;
    [unroll] for(uint j=0;j<6;++j) cb1[j]=0;
    cb0[0]=float4(1,0,0,0);cb0[1]=float4(0,1,0,0);
    cb0[2]=float4(0,0,1,0);cb0[3]=float4(0,0,0,1);
    float3 worldExtent = abs(world[0].xyz)*g_SourceFoliageWindLocalBounds.x+
        abs(world[1].xyz)*g_SourceFoliageWindLocalBounds.y+abs(world[2].xyz)*g_SourceFoliageWindLocalBounds.z;
    float3 columnSq=world[0].xyz*world[0].xyz+world[1].xyz*world[1].xyz+world[2].xyz*world[2].xyz;
    float maxScale=sqrt(max(columnSq.x,max(columnSq.y,columnSq.z)));
    float4 dimensionsAndRadius=sourceOwner.w==1.f ? sourceDimensionsAndRadius :
        float4(worldExtent.x,worldExtent.z,worldExtent.y,g_SourceFoliageWindLocalBounds.w*maxScale)*100.f+1.f;
    float4 actorPosition=sourceOwner.w==1.f ? float4(sourceOwner.xyz,0) : g_SourceFoliageWindActorPosition;
    float4 center=float4(SourceFoliageToUE(mul(g_SourceFoliageWindLocalCenter,world).xyz),1);
    // This native closure pivots at LocalToWorld translation, rather than the material center.
    cb0[3]=float4(SourceFoliageToUE(world[3].xyz),1);
    cb0[4].w=dimensionsAndRadius.w; cb0[5].xyz=dimensionsAndRadius.xyz;
    cb0[6]=g_SourceFoliageWindDirectionSpeed;
    cb0[7]=g_SourceFoliageWindScalars[0];cb0[8]=g_SourceFoliageWindScalars[1];
    cb0[7].y=g_SourceFoliageWindTime;
    float4 v0=float4(SourceFoliageToUE(worldPosition)-cb0[3].xyz,1);
    // Native packed VF color is BGRA; the installed WModel carrier is RGBA.
    float4 v3=vertexColor.bgra;
    float4 r0=0,r1=0,r2=0,r3=0,r4=0,r5=0,r6=0,r7=0,r8=0,r9=0,r10=0,r11=0,r12=0;
    // mov r3.w, l(0)
    r3.w = asfloat(0u);
    // dp3 r0.w, cb0[6].xyzx, cb0[6].xyzx
    r0.w = dot(cb0[6].xyz, cb0[6].xyz);
    // sqrt r0.w, r0.w
    r0.w = sqrt(r0.w);
    // div r4.xyz, cb0[6].xyzx, r0.wwww
    float stage148_x = cb0[6].x / r0.w;
    float stage148_y = cb0[6].y / r0.w;
    float stage148_z = cb0[6].z / r0.w;
    r4.x = stage148_x;
    r4.y = stage148_y;
    r4.z = stage148_z;
    // mul r5.xyz, r4.yzxy, l(0.000000, 0.000000, 1.000000, 0.000000)
    float stage149_x = r4.y * 0.0f;
    float stage149_y = r4.z * 0.0f;
    float stage149_z = r4.x * 1.0f;
    r5.x = stage149_x;
    r5.y = stage149_y;
    r5.z = stage149_z;
    // mad r5.xyz, r4.xyzx, l(0.000000, 1.000000, 0.000000, 0.000000), -r5.xyzx
    float stage150_x = r4.x * 0.0f + -r5.x;
    float stage150_y = r4.y * 1.0f + -r5.y;
    float stage150_z = r4.z * 0.0f + -r5.z;
    r5.x = stage150_x;
    r5.y = stage150_y;
    r5.z = stage150_z;
    // add r5.xyz, -r4.zxyz, r5.xyzx
    float stage151_x = -r4.z + r5.x;
    float stage151_y = -r4.x + r5.y;
    float stage151_z = -r4.y + r5.z;
    r5.x = stage151_x;
    r5.y = stage151_y;
    r5.z = stage151_z;
    // lt r0.w, l(0.000000), cb0[4].w
    r0.w = asfloat((0.0f < cb0[4].w) ? 0xffffffffu : 0u);
    // movc r0.w, r0.w, cb0[4].w, l(0.000100)
    r0.w = asuint(r0.w) != 0u ? cb0[4].w : 0.0001f;
    // ge r1.w, cb0[4].w, l(0.000000)
    r1.w = asfloat((cb0[4].w >= 0.0f) ? 0xffffffffu : 0u);
    // movc r0.w, r1.w, r0.w, cb0[4].w
    r0.w = asuint(r1.w) != 0u ? r0.w : cb0[4].w;
    // div r0.w, l(100.000000), r0.w
    r0.w = 100.0f / r0.w;
    // mul r1.w, cb0[6].w, cb0[7].y
    r1.w = cb0[6].w * cb0[7].y;
    // mul r2.w, r0.w, r1.w
    r2.w = r0.w * r1.w;
    // mul r0.w, r0.w, cb0[7].w
    r0.w = r0.w * cb0[7].w;
    // add r3.xyz, cb0[3].xyzx, -cb1[5].xyzx
    float stage152_x = cb0[3].x + -cb1[5].x;
    float stage152_y = cb0[3].y + -cb1[5].y;
    float stage152_z = cb0[3].z + -cb1[5].z;
    r3.x = stage152_x;
    r3.y = stage152_y;
    r3.z = stage152_z;
    // frc r6.xy, r3.xyxx
    float stage153_x = frac(r3.x);
    float stage153_y = frac(r3.y);
    r6.x = stage153_x;
    r6.y = stage153_y;
    // mad r6.xy, r6.xyxx, r6.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    float stage154_x = r6.x * r6.x + -0.5f;
    float stage154_y = r6.y * r6.y + -0.5f;
    r6.x = stage154_x;
    r6.y = stage154_y;
    // add r6.xy, r6.xyxx, r6.xyxx
    float stage155_x = r6.x + r6.x;
    float stage155_y = r6.y + r6.y;
    r6.x = stage155_x;
    r6.y = stage155_y;
    // add r4.w, r6.y, r6.x
    r4.w = r6.y + r6.x;
    // mul r4.w, r4.w, cb0[7].z
    r4.w = r4.w * cb0[7].z;
    // mad r2.w, cb0[7].x, r2.w, r4.w
    r2.w = cb0[7].x * r2.w + r4.w;
    // mul r6.xyz, r2.wwww, l(1.086065, 0.989267, 3.141593, 0.000000)
    float stage156_x = r2.w * 1.086065f;
    float stage156_y = r2.w * 0.989267f;
    float stage156_z = r2.w * 3.141593f;
    r6.x = stage156_x;
    r6.y = stage156_y;
    r6.z = stage156_z;
    // sincos r6.xyz, null, r6.xyzx
    float stage157_x = r6.x;
    float stage157_y = r6.y;
    float stage157_z = r6.z;
    float stage158_x = sin(stage157_x);
    float stage158_y = sin(stage157_y);
    float stage158_z = sin(stage157_z);
    r6.x = stage158_x;
    r6.y = stage158_y;
    r6.z = stage158_z;
    // add r6.xyz, r6.xyzx, l(1.000000, 1.666667, 1.000000, 0.000000)
    float stage159_x = r6.x + 1.0f;
    float stage159_y = r6.y + 1.666667f;
    float stage159_z = r6.z + 1.0f;
    r6.x = stage159_x;
    r6.y = stage159_y;
    r6.z = stage159_z;
    // mul r6.xy, r6.xyxx, l(0.500000, 0.375000, 0.000000, 0.000000)
    float stage160_x = r6.x * 0.5f;
    float stage160_y = r6.y * 0.375f;
    r6.x = stage160_x;
    r6.y = stage160_y;
    // mul r2.w, r6.y, r6.z
    r2.w = r6.y * r6.z;
    // mad r5.xyz, r6.xxxx, r5.xyzx, r4.zxyz
    float stage161_x = r6.x * r5.x + r4.z;
    float stage161_y = r6.x * r5.y + r4.x;
    float stage161_z = r6.x * r5.z + r4.y;
    r5.x = stage161_x;
    r5.y = stage161_y;
    r5.z = stage161_z;
    // mul r2.w, r2.w, l(-0.500000)
    r2.w = r2.w * -0.5f;
    // add r6.xyz, r5.xyzx, l(-1.000000, -0.000000, -0.000000, 0.000000)
    float stage162_x = r5.x + -1.0f;
    float stage162_y = r5.y + -0.0f;
    float stage162_z = r5.z + -0.0f;
    r6.x = stage162_x;
    r6.y = stage162_y;
    r6.z = stage162_z;
    // mul r7.xyzw, v0.yyyy, cb0[1].xyzw
    float stage163_x = v0.y * cb0[1].x;
    float stage163_y = v0.y * cb0[1].y;
    float stage163_z = v0.y * cb0[1].z;
    float stage163_w = v0.y * cb0[1].w;
    r7.x = stage163_x;
    r7.y = stage163_y;
    r7.z = stage163_z;
    r7.w = stage163_w;
    // mad r7.xyzw, cb0[0].xyzw, v0.xxxx, r7.xyzw
    float stage164_x = cb0[0].x * v0.x + r7.x;
    float stage164_y = cb0[0].y * v0.x + r7.y;
    float stage164_z = cb0[0].z * v0.x + r7.z;
    float stage164_w = cb0[0].w * v0.x + r7.w;
    r7.x = stage164_x;
    r7.y = stage164_y;
    r7.z = stage164_z;
    r7.w = stage164_w;
    // mad r7.xyzw, cb0[2].xyzw, v0.zzzz, r7.xyzw
    float stage165_x = cb0[2].x * v0.z + r7.x;
    float stage165_y = cb0[2].y * v0.z + r7.y;
    float stage165_z = cb0[2].z * v0.z + r7.z;
    float stage165_w = cb0[2].w * v0.z + r7.w;
    r7.x = stage165_x;
    r7.y = stage165_y;
    r7.z = stage165_z;
    r7.w = stage165_w;
    // mad r7.xyzw, cb0[3].xyzw, v0.wwww, r7.xyzw
    float stage166_x = cb0[3].x * v0.w + r7.x;
    float stage166_y = cb0[3].y * v0.w + r7.y;
    float stage166_z = cb0[3].z * v0.w + r7.z;
    float stage166_w = cb0[3].w * v0.w + r7.w;
    r7.x = stage166_x;
    r7.y = stage166_y;
    r7.z = stage166_z;
    r7.w = stage166_w;
    // add r8.xyz, r7.xyzx, -cb1[5].xyzx
    float stage167_x = r7.x + -cb1[5].x;
    float stage167_y = r7.y + -cb1[5].y;
    float stage167_z = r7.z + -cb1[5].z;
    r8.x = stage167_x;
    r8.y = stage167_y;
    r8.z = stage167_z;
    // add r9.xyz, -r3.xyzx, r8.xyzx
    float stage168_x = -r3.x + r8.x;
    float stage168_y = -r3.y + r8.y;
    float stage168_z = -r3.z + r8.z;
    r9.x = stage168_x;
    r9.y = stage168_y;
    r9.z = stage168_z;
    // dp3 r5.w, r9.xyzx, r9.xyzx
    r5.w = dot(r9.xyz, r9.xyz);
    // sqrt r5.w, r5.w
    r5.w = sqrt(r5.w);
    // div r10.xyz, r9.xyzx, r5.wwww
    float stage169_x = r9.x / r5.w;
    float stage169_y = r9.y / r5.w;
    float stage169_z = r9.z / r5.w;
    r10.x = stage169_x;
    r10.y = stage169_y;
    r10.z = stage169_z;
    // dp3 r4.x, r10.xyzx, r4.xyzx
    r4.x = dot(r10.xyz, r4.xyz);
    // mad r4.xyz, |r4.xxxx|, r6.xyzx, l(1.000000, 0.000000, 0.000000, 0.000000)
    float stage170_x = abs(r4.x) * r6.x + 1.0f;
    float stage170_y = abs(r4.x) * r6.y + 0.0f;
    float stage170_z = abs(r4.x) * r6.z + 0.0f;
    r4.x = stage170_x;
    r4.y = stage170_y;
    r4.z = stage170_z;
    // dp2 r5.w, r4.yzyy, r9.xyxx
    r5.w = dot(r4.yz, r9.xy);
    // mad r6.xyz, r4.yzxy, r5.wwww, r3.xywx
    float stage171_x = r4.y * r5.w + r3.x;
    float stage171_y = r4.z * r5.w + r3.y;
    float stage171_z = r4.x * r5.w + r3.w;
    r6.x = stage171_x;
    r6.y = stage171_y;
    r6.z = stage171_z;
    // mov r8.w, l(0)
    r8.w = asfloat(0u);
    // add r10.xyz, -r6.xyzx, r8.xywx
    float stage172_x = -r6.x + r8.x;
    float stage172_y = -r6.y + r8.y;
    float stage172_z = -r6.z + r8.w;
    r10.x = stage172_x;
    r10.y = stage172_y;
    r10.z = stage172_z;
    // mul r11.xyz, r4.xyzx, r10.yzxy
    float stage173_x = r4.x * r10.y;
    float stage173_y = r4.y * r10.z;
    float stage173_z = r4.z * r10.x;
    r11.x = stage173_x;
    r11.y = stage173_y;
    r11.z = stage173_z;
    // mad r4.xyz, r4.zxyz, r10.zxyz, -r11.xyzx
    float stage174_x = r4.z * r10.z + -r11.x;
    float stage174_y = r4.x * r10.x + -r11.y;
    float stage174_z = r4.y * r10.y + -r11.z;
    r4.x = stage174_x;
    r4.y = stage174_y;
    r4.z = stage174_z;
    // add r3.w, r8.z, r8.y
    r3.w = r8.z + r8.y;
    // add r3.w, r3.w, r8.x
    r3.w = r3.w + r8.x;
    // mul r3.w, r3.w, cb0[8].z
    r3.w = r3.w * cb0[8].z;
    // mad r3.w, r3.w, l(0.010000), r4.w
    r3.w = r3.w * 0.01f + r4.w;
    // mad r1.w, cb0[8].y, r1.w, r3.w
    r1.w = cb0[8].y * r1.w + r3.w;
    // mul r1.w, r1.w, l(6.283185)
    r1.w = r1.w * 6.283185f;
    // sincos r1.w, null, r1.w
    float stage175_w = r1.w;
    r1.w = sin(stage175_w);
    // lt r3.w, l(0.000000), cb0[5].z
    r3.w = asfloat((0.0f < cb0[5].z) ? 0xffffffffu : 0u);
    // movc r3.w, r3.w, cb0[5].z, l(0.000100)
    r3.w = asuint(r3.w) != 0u ? cb0[5].z : 0.0001f;
    // ge r4.w, cb0[5].z, l(0.000000)
    r4.w = asfloat((cb0[5].z >= 0.0f) ? 0xffffffffu : 0u);
    // movc r3.w, r4.w, r3.w, cb0[5].z
    r3.w = asuint(r4.w) != 0u ? r3.w : cb0[5].z;
    // div r3.w, r9.z, r3.w
    r3.w = r9.z / r3.w;
    // dp3 r4.w, r5.yzxy, r9.xyzx
    r4.w = dot(r5.yzx, r9.xyz);
    // mad r3.xyz, r5.yzxy, r4.wwww, r3.xyzx
    float stage176_x = r5.y * r4.w + r3.x;
    float stage176_y = r5.z * r4.w + r3.y;
    float stage176_z = r5.x * r4.w + r3.z;
    r3.x = stage176_x;
    r3.y = stage176_y;
    r3.z = stage176_z;
    // mul r3.w, r3.w, r3.w
    r3.w = r3.w * r3.w;
    // mul r4.w, r3.w, cb0[8].x
    r4.w = r3.w * cb0[8].x;
    // mul r0.w, r0.w, r3.w
    r0.w = r0.w * r3.w;
    // mul r1.w, r1.w, r4.w
    r1.w = r1.w * r4.w;
    // sincos r9.x, r11.x, r1.w
    float stage177_x = r1.w;
    r9.x = sin(stage177_x);
    r11.x = cos(stage177_x);
    // mul r4.xyz, r4.xyzx, r9.xxxx
    float stage178_x = r4.x * r9.x;
    float stage178_y = r4.y * r9.x;
    float stage178_z = r4.z * r9.x;
    r4.x = stage178_x;
    r4.y = stage178_y;
    r4.z = stage178_z;
    // mad r4.xyz, r10.xyzx, r11.xxxx, r4.xyzx
    float stage179_x = r10.x * r11.x + r4.x;
    float stage179_y = r10.y * r11.x + r4.y;
    float stage179_z = r10.z * r11.x + r4.z;
    r4.x = stage179_x;
    r4.y = stage179_y;
    r4.z = stage179_z;
    // add r4.xyz, r4.xyzx, r6.xyzx
    float stage180_x = r4.x + r6.x;
    float stage180_y = r4.y + r6.y;
    float stage180_z = r4.z + r6.z;
    r4.x = stage180_x;
    r4.y = stage180_y;
    r4.z = stage180_z;
    // add r4.xyz, -r8.xywx, r4.xyzx
    float stage181_x = -r8.x + r4.x;
    float stage181_y = -r8.y + r4.y;
    float stage181_z = -r8.w + r4.z;
    r4.x = stage181_x;
    r4.y = stage181_y;
    r4.z = stage181_z;
    // add r6.xyz, -r3.xyzx, r8.xyzx
    float stage182_x = -r3.x + r8.x;
    float stage182_y = -r3.y + r8.y;
    float stage182_z = -r3.z + r8.z;
    r6.x = stage182_x;
    r6.y = stage182_y;
    r6.z = stage182_z;
    // mul r9.xyz, r5.xyzx, r6.yzxy
    float stage183_x = r5.x * r6.y;
    float stage183_y = r5.y * r6.z;
    float stage183_z = r5.z * r6.x;
    r9.x = stage183_x;
    r9.y = stage183_y;
    r9.z = stage183_z;
    // mad r5.xyz, r5.zxyz, r6.zxyz, -r9.xyzx
    float stage184_x = r5.z * r6.z + -r9.x;
    float stage184_y = r5.x * r6.x + -r9.y;
    float stage184_z = r5.y * r6.y + -r9.z;
    r5.x = stage184_x;
    r5.y = stage184_y;
    r5.z = stage184_z;
    // dp3 r1.w, -cb0[6].xyzx, -cb0[6].xyzx
    r1.w = dot(-cb0[6].xyz, -cb0[6].xyz);
    // sqrt r1.w, r1.w
    r1.w = sqrt(r1.w);
    // mul r0.w, r0.w, r1.w
    r0.w = r0.w * r1.w;
    // mul r0.w, r0.w, r2.w
    r0.w = r0.w * r2.w;
    // sincos r9.x, r10.x, r0.w
    float stage185_x = r0.w;
    r9.x = sin(stage185_x);
    r10.x = cos(stage185_x);
    // mul r5.xyz, r5.xyzx, r9.xxxx
    float stage186_x = r5.x * r9.x;
    float stage186_y = r5.y * r9.x;
    float stage186_z = r5.z * r9.x;
    r5.x = stage186_x;
    r5.y = stage186_y;
    r5.z = stage186_z;
    // mad r5.xyz, r6.xyzx, r10.xxxx, r5.xyzx
    float stage187_x = r6.x * r10.x + r5.x;
    float stage187_y = r6.y * r10.x + r5.y;
    float stage187_z = r6.z * r10.x + r5.z;
    r5.x = stage187_x;
    r5.y = stage187_y;
    r5.z = stage187_z;
    // add r3.xyz, r3.xyzx, r5.xyzx
    float stage188_x = r3.x + r5.x;
    float stage188_y = r3.y + r5.y;
    float stage188_z = r3.z + r5.z;
    r3.x = stage188_x;
    r3.y = stage188_y;
    r3.z = stage188_z;
    // add r3.xyz, -r8.xyzx, r3.xyzx
    float stage189_x = -r8.x + r3.x;
    float stage189_y = -r8.y + r3.y;
    float stage189_z = -r8.z + r3.z;
    r3.x = stage189_x;
    r3.y = stage189_y;
    r3.z = stage189_z;
    // add r3.xyz, r4.xyzx, r3.xyzx
    float stage190_x = r4.x + r3.x;
    float stage190_y = r4.y + r3.y;
    float stage190_z = r4.z + r3.z;
    r3.x = stage190_x;
    r3.y = stage190_y;
    r3.z = stage190_z;
    // mad r3.xyz, v3.zzzz, r3.xyzx, r7.xyzx
    float stage191_x = v3.z * r3.x + r7.x;
    float stage191_y = v3.z * r3.y + r7.y;
    float stage191_z = v3.z * r3.z + r7.z;
    r3.x = stage191_x;
    r3.y = stage191_y;
    r3.z = stage191_z;
    float3 delta=r3.xyz-r7.xyz;
    if(any(!isfinite(delta))) return 0;
    return float3(delta.x,delta.z,-delta.y)*.01f;
}
float3 SourceFoliageWorldOffset(float3 worldPosition, float4 vertexColor, float4x4 world,
    float4 sourceOwner, float4 sourceDimensionsAndRadius)
{
    if(g_SourceFoliageWindEnabled==0u) return 0;
    if(g_SourceFoliageWindProgram==1u) return SourceFoliageOffsetProgram1(worldPosition,vertexColor,world,sourceOwner,sourceDimensionsAndRadius);
    if(g_SourceFoliageWindProgram==3u) return SourceFoliageOffsetProgram3(worldPosition,vertexColor,world,sourceOwner,sourceDimensionsAndRadius);
    if(g_SourceFoliageWindProgram==4u) return SourceFoliageOffsetProgram4(worldPosition,vertexColor,world,sourceOwner,sourceDimensionsAndRadius);
    return 0;
}
float3 SourceFoliageWorldOffset(float3 worldPosition, float4 vertexColor, float4x4 world)
{
    return SourceFoliageWorldOffset(worldPosition,vertexColor,world,
        g_SourceFoliageWindDrawOwnerPosition,g_SourceFoliageWindDrawDimensionsAndRadius);
}
#endif
