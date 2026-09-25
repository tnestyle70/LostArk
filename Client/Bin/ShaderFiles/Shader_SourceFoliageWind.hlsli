// Source VS e4fe43228f19284f9ca096e27b675db6, displacement instructions 28..160.
// Literal DXBC arithmetic emitted by translate_ue3_dxbc_to_hlsl.py.
// Upstream UE3 no-wind fallback (0,0,1,0); LostArk process value is unmeasured.
#ifndef SOURCE_FOLIAGE_WIND_INCLUDED
#define SOURCE_FOLIAGE_WIND_INCLUDED
uint g_SourceFoliageWindEnabled = 0u;
float4 g_SourceFoliageWindLocalCenter = float4(0,0,0,1);
float4 g_SourceFoliageWindLocalBounds = float4(1,1,1,1);
float4 g_SourceFoliageWindActorPosition = 0;
float4 g_SourceFoliageWindDirectionSpeed = float4(0,0,1,0);
float4 g_SourceFoliageWindPlayerPosition = 0;
float4 g_SourceFoliageWindScalars[4];
float g_SourceFoliageWindTime = 0;
float3 SourceFoliageToUE(float3 value) { return float3(value.x,-value.z,value.y)*100.f; }
float3 SourceFoliageWorldOffset(float3 worldPosition, float4 vertexColor, float4x4 world)
{
    if (g_SurfaceProgram != 9u || g_SourceFoliageWindEnabled == 0u) return 0;
    float4 cb0[18]; float4 cb1[6];
    [unroll] for(uint i=0;i<18;++i) cb0[i]=0;
    [unroll] for(uint j=0;j<6;++j) cb1[j]=0;
    cb0[0]=float4(1,0,0,0);cb0[1]=float4(0,1,0,0);
    cb0[2]=float4(0,0,1,0);cb0[3]=float4(0,0,0,1);
    cb0[4]=g_SourceFoliageWindActorPosition;
    float3 worldExtent = abs(world[0].xyz)*g_SourceFoliageWindLocalBounds.x+
        abs(world[1].xyz)*g_SourceFoliageWindLocalBounds.y+abs(world[2].xyz)*g_SourceFoliageWindLocalBounds.z;
    float maxScale=max(length(world[0].xyz),max(length(world[1].xyz),length(world[2].xyz)));
    cb0[5].w=g_SourceFoliageWindLocalBounds.w*maxScale*100.f;
    cb0[6].xyz=float3(worldExtent.x,worldExtent.z,worldExtent.y)*100.f;
    cb0[7]=g_SourceFoliageWindDirectionSpeed;
    cb0[8]=float4(SourceFoliageToUE(mul(g_SourceFoliageWindLocalCenter,world).xyz),1);
    cb0[9]=g_SourceFoliageWindPlayerPosition;
    [unroll] for(uint k=0;k<4;++k) cb0[10+k]=g_SourceFoliageWindScalars[k];
    cb0[12].z=g_SourceFoliageWindTime;
    float4 v0=float4(SourceFoliageToUE(worldPosition),1);
    // Original VF v3 is BGRA; the existing WModel carrier is decoded RGBA.
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
    // Undefined native zero-length inputs cannot poison unrelated draw vertices.
    if (!all(isfinite(delta))) return 0;
    return float3(delta.x,delta.z,-delta.y)*.01f;
}
#endif
