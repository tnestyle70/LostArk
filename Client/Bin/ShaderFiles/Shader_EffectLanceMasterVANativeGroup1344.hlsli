// Recovered LanceMaster source programs 1344..1407.
#ifndef LANCE_VA_NATIVE_MODEL_ONLY
float4 LanceVANative1344(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[6]; [unroll] for (uint i=0u; i<6u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_LanceVASourceMaterialParameters[4u];
    source[2] = g_LanceVASourceMaterialParameters[3u];
    source[3].x = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[3].y = (g_LanceVASourceMaterialTime.xxxx).x;
    source[3].z = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[3].w = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[4].x = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[4].y = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[4].z = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[4].w = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[5].x = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[5].y = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[5].z = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: div r0.xy, v7.xyxx, v7.wwww
    r0.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 2: mad r0.xy, r0.xyxx, cb2[0].xyxx, cb2[0].wzww
    r0.xy = ((r0.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // Native 3: source device depth mapped to centimetre view depth; reconstruction at 5.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 5-8: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 9: add r0.x, r0.x, -v7.w
    r0.x = ((r0.xxxx)+(-(v7.wwww))).x;
    // 10: add r0.y, -cb0[5].z, l(1.000000)
    r0.y = ((-(source[5].zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: max r0.y, -r0.y, l(0.001000)
    r0.y = (max(-(r0.yyyy),float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 12: div_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)/(r0.yyyy))).x;
    // 13: add r0.y, -v4.x, l(1.000000)
    r0.y = ((-(v4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 14: add r0.y, -r0.y, l(1.000000)
    r0.y = ((-(r0.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 15: mul_sat r0.y, r0.y, cb0[5].x
    r0.y = (saturate((r0.yyyy)*(source[5].xxxx))).y;
    // 16: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 17: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 18: mul r0.z, r0.z, cb0[5].y
    r0.z = ((r0.zzzz)*(source[5].yyyy)).z;
    // 19: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 20: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 21: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 22: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 23: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 24: mul r0.y, v2.x, cb0[3].w
    r0.y = ((v2.xxxx)*(source[3].wwww)).y;
    // 25: mul r0.z, cb0[3].x, cb0[3].y
    r0.z = ((source[3].xxxx)*(source[3].yyyy)).z;
    // 26: mad r1.x, r0.z, cb0[3].z, r0.y
    r1.x = ((r0.zzzz)*(source[3].zzzz)+(r0.yyyy)).x;
    // 27: mul r0.y, v2.y, cb0[4].x
    r0.y = ((v2.yyyy)*(source[4].xxxx)).y;
    // 28: mad r1.y, r0.z, cb0[4].y, r0.y
    r1.y = ((r0.zzzz)*(source[4].yyyy)+(r0.yyyy)).y;
    // 29: sample_b_indexable(texture2d)(float,float,float,float) r0.yzw, r1.xyxx, t1.wxyz, s1, l(0.000000)
    r0.yzw = (LanceVANativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).wxyz).yzw;
    // 30: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 31: add r1.xyz, -r0.yzwy, r1.xxxx
    r1.xyz = ((-(r0.yzwy))+(r1.xxxx)).xyz;
    // 32: mad r0.yzw, cb0[4].zzzz, r1.xxyz, r0.yyzw
    r0.yzw = ((source[4].zzzz)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 33: max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 34: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 35: mul r0.yzw, r0.yyzw, cb0[4].wwww
    r0.yzw = ((r0.yyzw)*(source[4].wwww)).yzw;
    // 36: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 37: mul r1.xyz, cb0[2].xyzx, cb0[2].wwww
    r1.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 38: mul r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)*(r1.xxyz)).yzw;
    // 39: mad r0.yzw, r0.yyzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 40: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 41: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 42: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

float4 LanceVANative1345(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[3]; [unroll] for (uint i=0u; i<3u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_LanceVASourceMaterialParameters[2u];
    source[2].x = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[2].y = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[2].z = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[2].w = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: mad r0.x, cb0[2].x, v4.x, l(-1.000000)
    r0.x = ((source[2].xxxx)*(v4.xxxx)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 2: mul r0.x, r0.x, l(0.500000)
    r0.x = ((r0.xxxx)*(float4(0.500000,0.500000,0.500000,0.500000))).x;
    // 3: mul r0.y, v4.x, cb0[2].x
    r0.y = ((v4.xxxx)*(source[2].xxxx)).y;
    // 4: mad r0.xy, r0.yyyy, v2.xyxx, -r0.xxxx
    r0.xy = ((r0.yyyy)*(v2.xyxx)+(-(r0.xxxx))).xy;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, r0.xyxx, t0.xyzw, s0, l(0.000000)
    r0.xyzw = (LanceVANativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 6: log r1.x, |r0.w|
    r1.x = (log2(abs(r0.wwww))).x;
    // 7: mul r1.x, r1.x, cb0[2].w
    r1.x = ((r1.xxxx)*(source[2].wwww)).x;
    // 8: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 9: mul r1.x, r1.x, v3.w
    r1.x = ((r1.xxxx)*(v3.wwww)).x;
    // 10: lt r0.w, |r0.w|, l(0.000001)
    r0.w = (asfloat((uint4)((abs(r0.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 11: movc r0.w, r0.w, l(0), |r1.x|
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (abs(r1.xxxx))).w;
    // 12: log r1.x, r0.w
    r1.x = (log2(r0.wwww)).x;
    // 13: lt r0.w, r0.w, l(0.000001)
    r0.w = (asfloat((uint4)((r0.wwww)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 14: mul r1.x, r1.x, v4.y
    r1.x = ((r1.xxxx)*(v4.yyyy)).x;
    // 15: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 16: mul r1.x, r1.x, cb0[0].x
    r1.x = ((r1.xxxx)*(source[0].xxxx)).x;
    // 17: movc r0.w, r0.w, l(0), r1.x
    r0.w = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.xxxx)).w;
    // 18: mul r1.xyz, r0.xyzx, cb0[2].yyyy
    r1.xyz = ((r0.xyzx)*(source[2].yyyy)).xyz;
    // 19: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 20: mad r0.xyz, -cb0[2].yyyy, r0.xyzx, r1.wwww
    r0.xyz = ((-(source[2].yyyy))*(r0.xyzx)+(r1.wwww)).xyz;
    // 21: mad r0.xyz, cb0[2].zzzz, r0.xyzx, r1.xyzx
    r0.xyz = ((source[2].zzzz)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 22: mad r0.xyz, r0.xyzx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xyzx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 23: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 24: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 25: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

float4 LanceVANative1346(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[13]; [unroll] for (uint i=0u; i<13u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_LanceVASourceMaterialParameters[7u];
    source[3] = input.dynamicParameter;
    source[4] = g_LanceVASourceMaterialParameters[5u];
    source[5] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[2u].yyyy,g_LanceVASourceMaterialParameters[2u].zzzz,1u);
    source[6].x = (g_LanceVASourceMaterialParameters[4u].zzzz).x;
    source[6].y = (g_LanceVASourceMaterialTime.xxxx).x;
    source[6].z = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[6].w = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[7].x = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[7].y = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[7].z = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[7].w = (g_LanceVASourceMaterialParameters[3u].zzzz).x;
    source[8].x = (g_LanceVASourceMaterialParameters[3u].wwww).x;
    source[8].y = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[8].z = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_LanceVASourceMaterialParameters[0u].yyyy)).x;
    source[8].w = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_LanceVASourceMaterialParameters[0u].yyyy))).x;
    source[9].x = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[9].y = ((float4(1.0, 0.0, 0.0, 0.0)-g_LanceVASourceMaterialParameters[0u].zzzz)).x;
    source[9].z = (max((float4(1.0, 0.0, 0.0, 0.0)-g_LanceVASourceMaterialParameters[0u].zzzz),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[9].w = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-g_LanceVASourceMaterialParameters[0u].zzzz),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    source[10].x = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[10].y = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[10].z = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[10].w = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[11].x = (g_LanceVASourceMaterialParameters[4u].xxxx).x;
    source[11].y = (g_LanceVASourceMaterialParameters[4u].yyyy).x;
    source[11].z = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[11].w = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[12].x = (g_LanceVASourceMaterialParameters[3u].yyyy).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f;
    // 1: div r0.xy, v7.xyxx, v7.wwww
    r0.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 2: mad r0.xy, r0.xyxx, cb2[0].xyxx, cb2[0].wzww
    r0.xy = ((r0.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // Native 3: source device depth mapped to centimetre view depth; reconstruction at 5.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 5-8: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 9: add r0.x, r0.x, -v7.w
    r0.x = ((r0.xxxx)+(-(v7.wwww))).x;
    // 10: add r0.y, -cb0[11].z, l(1.000000)
    r0.y = ((-(source[11].zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: max r0.y, -r0.y, l(0.001000)
    r0.y = (max(-(r0.yyyy),float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 12: div_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)/(r0.yyyy))).x;
    // 13: dp3 r0.y, v6.xyzx, v6.xyzx
    r0.y = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).y;
    // 14: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 15: mul r0.y, r0.y, v6.z
    r0.y = ((r0.yyyy)*(v6.zzzz)).y;
    // 16: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 17: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 18: mul r0.z, r0.z, cb0[11].w
    r0.z = ((r0.zzzz)*(source[11].wwww)).z;
    // 19: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 20: mul_sat r0.z, r0.z, cb0[12].x
    r0.z = (saturate((r0.zzzz)*(source[12].xxxx))).z;
    // 21: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 22: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 23: add r0.yz, v4.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v4.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 24: max r0.w, |r0.z|, |r0.y|
    r0.w = (max(abs(r0.zzzz),abs(r0.yyyy))).w;
    // 25: div r0.w, l(1.000000, 1.000000, 1.000000, 1.000000), r0.w
    r0.w = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.wwww)).w;
    // 26: min r1.x, |r0.z|, |r0.y|
    r1.x = (min(abs(r0.zzzz),abs(r0.yyyy))).x;
    // 27: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 28: mul r1.x, r0.w, r0.w
    r1.x = ((r0.wwww)*(r0.wwww)).x;
    // 29: mad r1.y, r1.x, l(0.020835), l(-0.085133)
    r1.y = ((r1.xxxx)*(float4(0.020835,0.020835,0.020835,0.020835))+(float4(-0.085133,-0.085133,-0.085133,-0.085133))).y;
    // 30: mad r1.y, r1.x, r1.y, l(0.180141)
    r1.y = ((r1.xxxx)*(r1.yyyy)+(float4(0.180141,0.180141,0.180141,0.180141))).y;
    // 31: mad r1.y, r1.x, r1.y, l(-0.330299)
    r1.y = ((r1.xxxx)*(r1.yyyy)+(float4(-0.330299,-0.330299,-0.330299,-0.330299))).y;
    // 32: mad r1.x, r1.x, r1.y, l(0.999866)
    r1.x = ((r1.xxxx)*(r1.yyyy)+(float4(0.999866,0.999866,0.999866,0.999866))).x;
    // 33: mul r1.y, r0.w, r1.x
    r1.y = ((r0.wwww)*(r1.xxxx)).y;
    // 34: mad r1.y, r1.y, l(-2.000000), l(1.570796)
    r1.y = ((r1.yyyy)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(1.570796,1.570796,1.570796,1.570796))).y;
    // 35: lt r1.z, |r0.z|, |r0.y|
    r1.z = (asfloat((uint4)((abs(r0.zzzz))<(abs(r0.yyyy))) * 0xffffffffu)).z;
    // 36: and r1.y, r1.z, r1.y
    r1.y = (asfloat(asuint(r1.zzzz) & asuint(r1.yyyy))).y;
    // 37: mad r0.w, r0.w, r1.x, r1.y
    r0.w = ((r0.wwww)*(r1.xxxx)+(r1.yyyy)).w;
    // 38: lt r1.x, r0.z, -r0.z
    r1.x = (asfloat((uint4)((r0.zzzz)<(-(r0.zzzz))) * 0xffffffffu)).x;
    // 39: and r1.x, r1.x, l(0xc0490fdb)
    r1.x = (asfloat(asuint(r1.xxxx) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).x;
    // 40: add r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)+(r1.xxxx)).w;
    // 41: min r1.x, r0.z, r0.y
    r1.x = (min(r0.zzzz,r0.yyyy)).x;
    // 42: lt r1.x, r1.x, -r1.x
    r1.x = (asfloat((uint4)((r1.xxxx)<(-(r1.xxxx))) * 0xffffffffu)).x;
    // 43: max r1.y, r0.z, r0.y
    r1.y = (max(r0.zzzz,r0.yyyy)).y;
    // 44: dp2 r0.y, r0.yzyy, r0.yzyy
    r0.y = (dot((r0.yzyy).xy,(r0.yzyy).xy).xxxx).y;
    // 45: sqrt r0.y, r0.y
    r0.y = (sqrt(r0.yyyy)).y;
    // 46: ge r0.z, r1.y, -r1.y
    r0.z = (asfloat((uint4)((r1.yyyy)>=(-(r1.yyyy))) * 0xffffffffu)).z;
    // 47: and r0.z, r0.z, r1.x
    r0.z = (asfloat(asuint(r0.zzzz) & asuint(r1.xxxx))).z;
    // 48: movc r0.z, r0.z, -r0.w, r0.w
    r0.z = ((asuint(r0.zzzz) != 0u) ? (-(r0.wwww)) : (r0.wwww)).z;
    // 49: mad r0.z, r0.z, l(0.159155), l(0.500000)
    r0.z = ((r0.zzzz)*(float4(0.159155,0.159155,0.159155,0.159155))+(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 50: mul r1.x, r0.z, cb0[5].x
    r1.x = ((r0.zzzz)*(source[5].xxxx)).x;
    // 51: mul r0.z, r0.z, cb0[6].w
    r0.z = ((r0.zzzz)*(source[6].wwww)).z;
    // 52: mul r2.x, cb0[6].y, cb0[10].w
    r2.x = ((source[6].yyyy)*(source[10].wwww)).x;
    // 53: mov r1.z, l(-1.000000)
    r1.z = (float4(-1.000000,-1.000000,-1.000000,-1.000000)).z;
    // 54: add r0.w, r0.y, r0.y
    r0.w = ((r0.yyyy)+(r0.yyyy)).w;
    // 55: log r0.w, r0.w
    r0.w = (log2(r0.wwww)).w;
    // 56: mul r1.y, cb0[3].z, cb0[7].y
    r1.y = ((source[3].zzzz)*(source[7].yyyy)).y;
    // 57: mul r0.w, r0.w, r1.y
    r0.w = ((r0.wwww)*(r1.yyyy)).w;
    // 58: exp r0.w, r0.w
    r0.w = (exp2(r0.wwww)).w;
    // 59: lt r1.y, r0.y, l(0.000000)
    r1.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000000,0.000000,0.000000,0.000000))) * 0xffffffffu)).y;
    // 60: mad r0.y, -r0.y, cb0[8].w, l(1.000000)
    r0.y = ((-(r0.yyyy))*(source[8].wwww)+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 61: mul_sat r0.y, r0.y, cb0[9].w
    r0.y = (saturate((r0.yyyy)*(source[9].wwww))).y;
    // 62: movc r0.w, r1.y, l(0), r0.w
    r0.w = ((asuint(r1.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.wwww)).w;
    // 63: mad r2.y, r0.w, cb0[5].y, cb0[3].x
    r2.y = ((r0.wwww)*(source[5].yyyy)+(source[3].xxxx)).y;
    // 64: mul r0.w, r0.w, cb0[7].x
    r0.w = ((r0.wwww)*(source[7].xxxx)).w;
    // 65: add r1.xy, r1.xzxx, r2.xyxx
    r1.xy = ((r1.xzxx)+(r2.xyxx)).xy;
    // 66: mul r1.xy, r1.xyxx, cb0[6].xxxx
    r1.xy = ((r1.xyxx)*(source[6].xxxx)).xy;
    // 67: sample_l_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t0.xyzw, s2, l(-1.000000)
    r1.x = (LanceVANativeSample1((r1.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).x;
    // 68: log r1.y, r0.y
    r1.y = (log2(r0.yyyy)).y;
    // 69: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 70: mul r1.y, r1.y, cb0[10].x
    r1.y = ((r1.yyyy)*(source[10].xxxx)).y;
    // 71: exp r1.y, r1.y
    r1.y = (exp2(r1.yyyy)).y;
    // 72: movc r0.y, r0.y, l(0), r1.y
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r1.yyyy)).y;
    // 73: mul r0.y, r1.x, r0.y
    r0.y = ((r1.xxxx)*(r0.yyyy)).y;
    // 74: mul_sat r0.y, r0.y, cb0[11].x
    r0.y = (saturate((r0.yyyy)*(source[11].xxxx))).y;
    // 75: log r1.x, r0.y
    r1.x = (log2(r0.yyyy)).x;
    // 76: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 77: mul r1.x, r1.x, cb0[11].y
    r1.x = ((r1.xxxx)*(source[11].yyyy)).x;
    // 78: exp r1.x, r1.x
    r1.x = (exp2(r1.xxxx)).x;
    // 79: mul r0.x, r0.x, r1.x
    r0.x = ((r0.xxxx)*(r1.xxxx)).x;
    // 80: mul_sat r0.x, r0.x, cb0[1].w
    r0.x = (saturate((r0.xxxx)*(source[1].wwww))).x;
    // 81: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 82: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 83: mul r0.y, cb0[6].x, cb0[6].y
    r0.y = ((source[6].xxxx)*(source[6].yyyy)).y;
    // 84: mad r1.x, r0.y, cb0[6].z, r0.z
    r1.x = ((r0.yyyy)*(source[6].zzzz)+(r0.zzzz)).x;
    // 85: mad r1.y, r0.y, cb0[7].z, r0.w
    r1.y = ((r0.yyyy)*(source[7].zzzz)+(r0.wwww)).y;
    // 86: sample_l_indexable(texture2d)(float,float,float,float) r0.yzw, r1.xyxx, t2.wxyz, s1, l(-1.000000)
    r0.yzw = (LanceVANativeSample0((r1.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).wxyz).yzw;
    // 87: dp3 r1.x, r0.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.x = (dot((r0.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 88: add r1.xyz, -r0.yzwy, r1.xxxx
    r1.xyz = ((-(r0.yzwy))+(r1.xxxx)).xyz;
    // 89: mad r0.yzw, cb0[7].wwww, r1.xxyz, r0.yyzw
    r0.yzw = ((source[7].wwww)*(r1.xxyz)+(r0.yyzw)).yzw;
    // 90: max r0.yzw, |r0.yyzw|, l(0.000000, 0.000001, 0.000001, 0.000001)
    r0.yzw = (max(abs(r0.yyzw),float4(0.000000,0.000001,0.000001,0.000001))).yzw;
    // 91: log r0.yzw, r0.yyzw
    r0.yzw = (log2(r0.yyzw)).yzw;
    // 92: mul r0.yzw, r0.yyzw, cb0[8].xxxx
    r0.yzw = ((r0.yyzw)*(source[8].xxxx)).yzw;
    // 93: exp r0.yzw, r0.yyzw
    r0.yzw = (exp2(r0.yyzw)).yzw;
    // 94: mul r1.xyz, cb0[4].xyzx, cb0[4].wwww
    r1.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 95: mul r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)*(r1.xxyz)).yzw;
    // 96: mad r0.yzw, r0.yyzw, cb0[1].xxyz, cb0[2].xxyz
    r0.yzw = ((r0.yyzw)*(source[1].xxyz)+(source[2].xxyz)).yzw;
    // 97: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 98: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 99: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

float4 LanceVANative1347(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[9]; [unroll] for (uint i=0u; i<9u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_LanceVASourceMaterialParameters[4u];
    source[3] = g_LanceVASourceMaterialParameters[2u];
    source[4] = input.dynamicParameter;
    source[5] = LanceVANativeAppend(cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[1u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[1u].xxxx)*float4(1.0, 0.0, 0.0, 0.0)))),1u);
    source[6] = LanceVANativeAppend(sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[1u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[1u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))),1u);
    source[7].x = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[1u].xxxx)*float4(1.0, 0.0, 0.0, 0.0))))).x;
    source[7].y = (cos(((float4(-0.523599029, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[1u].xxxx)*float4(1.0, 0.0, 0.0, 0.0)))).x;
    source[7].z = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[7].w = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[8].x = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[8].y = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[8].z = ((float4(0.0, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[0u].xxxx)).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: add r0.x, cb0[4].y, l(-1.000000)
    r0.x = ((source[4].yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).x;
    // 2: add r0.yz, v4.xxyx, l(0.000000, -0.500000, -0.500000, 0.000000)
    r0.yz = ((v4.xxyx)+(float4(0.000000,-0.500000,-0.500000,0.000000))).yz;
    // 3: dp2 r1.x, cb0[5].xyxx, r0.yzyy
    r1.x = (dot((source[5].xyxx).xy,(r0.yzyy).xy).xxxx).x;
    // 4: dp2 r1.y, cb0[6].xyxx, r0.yzyy
    r1.y = (dot((source[6].xyxx).xy,(r0.yzyy).xy).xxxx).y;
    // 5: add r0.x, r0.x, r1.x
    r0.x = ((r0.xxxx)+(r1.xxxx)).x;
    // 6: dp2 r0.z, r1.xyxx, r1.xyxx
    r0.z = (dot((r1.xyxx).xy,(r1.xyxx).xy).xxxx).z;
    // 7: sqrt r0.z, r0.z
    r0.z = (sqrt(r0.zzzz)).z;
    // 8: add r0.z, r0.z, r0.z
    r0.z = ((r0.zzzz)+(r0.zzzz)).z;
    // 9: min r0.z, r0.z, l(1.000000)
    r0.z = (min(r0.zzzz,float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 10: add r0.z, -r0.z, l(1.000000)
    r0.z = ((-(r0.zzzz))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 11: mul r0.w, cb0[4].z, cb0[7].z
    r0.w = ((source[4].zzzz)*(source[7].zzzz)).w;
    // 12: mad_sat r0.y, r1.y, r0.w, l(0.500000)
    r0.y = (saturate((r1.yyyy)*(r0.wwww)+(float4(0.500000,0.500000,0.500000,0.500000)))).y;
    // 13: add r0.xy, r0.xyxx, l(0.500000, 0.000000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(0.500000,0.000000,0.000000,0.000000))).xy;
    // 14: sample_l_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t0.xyzw, s0, l(-1.000000)
    r0.x = (LanceVANativeSample0((r0.xyxx).xy, (float4(-1.000000,-1.000000,-1.000000,-1.000000)).x, true).xyzw).x;
    // 15: mul r0.x, r0.z, r0.x
    r0.x = ((r0.zzzz)*(r0.xxxx)).x;
    // 16: mul_sat r0.x, r0.x, cb0[7].w
    r0.x = (saturate((r0.xxxx)*(source[7].wwww))).x;
    // 17: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 18: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 19: mul r0.y, r0.y, cb0[8].x
    r0.y = ((r0.yyyy)*(source[8].xxxx)).y;
    // 20: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 21: mul_sat r0.y, r0.y, cb0[1].w
    r0.y = (saturate((r0.yyyy)*(source[1].wwww))).y;
    // 22: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 23: add r0.x, r0.x, cb0[8].z
    r0.x = ((r0.xxxx)+(source[8].zzzz)).x;
    // 24: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 25: mul r0.yzw, cb0[3].xxyz, cb0[3].wwww
    r0.yzw = ((source[3].xxyz)*(source[3].wwww)).yzw;
    // 26: mad r0.yzw, r0.yyzw, cb0[1].xxyz, cb0[2].xxyz
    r0.yzw = ((r0.yyzw)*(source[1].xxyz)+(source[2].xxyz)).yzw;
    // 27: mul r0.yzw, r0.yyzw, v5.wwww
    r0.yzw = ((r0.yyzw)*(v5.wwww)).yzw;
    // 28: mul o0.xyz, r0.xxxx, r0.yzwy
    output.xyz = ((r0.xxxx)*(r0.yzwy)).xyz;
    // 29: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}

float4 LanceVANative1348(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[12]; [unroll] for (uint i=0u; i<12u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_LanceVASourceMaterialParameters[8u];
    source[3] = g_LanceVASourceMaterialParameters[6u];
    source[4] = input.dynamicParameter;
    source[5] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[2u].zzzz,g_LanceVASourceMaterialParameters[2u].wwww,1u);
    source[6].x = (g_LanceVASourceMaterialParameters[5u].xxxx).x;
    source[6].y = (g_LanceVASourceMaterialTime.xxxx).x;
    source[6].z = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[6].w = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[7].x = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[7].y = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[7].z = (g_LanceVASourceMaterialParameters[4u].zzzz).x;
    source[7].w = (g_LanceVASourceMaterialParameters[4u].wwww).x;
    source[8].x = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[8].y = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[8].z = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[8].w = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[9].x = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[9].y = (g_LanceVASourceMaterialParameters[3u].yyyy).x;
    source[9].z = ((float4(-0.523599029, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[3u].xxxx)).x;
    source[9].w = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[10].x = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[10].y = (g_LanceVASourceMaterialParameters[4u].xxxx).x;
    source[10].z = (g_LanceVASourceMaterialParameters[4u].yyyy).x;
    source[10].w = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[11].x = (g_LanceVASourceMaterialParameters[3u].zzzz).x;
    source[11].y = (g_LanceVASourceMaterialParameters[3u].wwww).x;
    source[11].z = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
    // 1: div r0.xy, v7.xyxx, v7.wwww
    r0.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 2: mad r0.xy, r0.xyxx, cb2[0].xyxx, cb2[0].wzww
    r0.xy = ((r0.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // Native 3: source device depth mapped to centimetre view depth; reconstruction at 5.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 5-8: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 9: add r0.x, r0.x, -v7.w
    r0.x = ((r0.xxxx)+(-(v7.wwww))).x;
    // 10: add r0.y, -cb0[10].w, l(1.000000)
    r0.y = ((-(source[10].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 11: max r0.y, -r0.y, l(0.001000)
    r0.y = (max(-(r0.yyyy),float4(0.001000,0.001000,0.001000,0.001000))).y;
    // 12: div_sat r0.x, r0.x, r0.y
    r0.x = (saturate((r0.xxxx)/(r0.yyyy))).x;
    // 13: dp3 r0.y, v6.xyzx, v6.xyzx
    r0.y = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).y;
    // 14: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 15: mul r0.y, r0.y, v6.z
    r0.y = ((r0.yyyy)*(v6.zzzz)).y;
    // 16: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 17: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 18: mul r0.z, r0.z, cb0[11].x
    r0.z = ((r0.zzzz)*(source[11].xxxx)).z;
    // 19: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 20: mul_sat r0.z, r0.z, cb0[11].y
    r0.z = (saturate((r0.zzzz)*(source[11].yyyy))).z;
    // 21: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 22: movc r0.x, r0.y, l(0), r0.x
    r0.x = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).x;
    // 23: add r0.y, cb0[4].y, l(-1.000000)
    r0.y = ((source[4].yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 24: mad r0.z, cb0[6].y, cb0[9].y, cb0[9].z
    r0.z = ((source[6].yyyy)*(source[9].yyyy)+(source[9].zzzz)).z;
    // 25: sincos r1.x, r2.x, r0.z
    r1.x = (sin(r0.zzzz)).x; r2.x = (cos(r0.zzzz)).x;
    // 26: mov r3.x, -r1.x
    r3.x = (-(r1.xxxx)).x;
    // 27: add r0.zw, v4.xxxy, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((v4.xxxy)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 28: mov r3.y, r2.x
    r3.y = (r2.xxxx).y;
    // 29: mov r3.z, r1.x
    r3.z = (r1.xxxx).z;
    // 30: dp2 r1.x, r3.zyzz, r0.zwzz
    r1.x = (dot((r3.zyzz).xy,(r0.zwzz).xy).xxxx).x;
    // 31: dp2 r0.z, r3.yxyy, r0.zwzz
    r0.z = (dot((r3.yxyy).xy,(r0.zwzz).xy).xxxx).z;
    // 32: mad r2.x, r0.z, cb0[5].x, r0.y
    r2.x = ((r0.zzzz)*(source[5].xxxx)+(r0.yyyy)).x;
    // 33: mul r2.z, r1.x, cb0[5].y
    r2.z = ((r1.xxxx)*(source[5].yyyy)).z;
    // 34: add r0.yz, r2.xxzx, l(0.000000, 0.500000, 0.500000, 0.000000)
    r0.yz = ((r2.xxzx)+(float4(0.000000,0.500000,0.500000,0.000000))).yz;
    // 35: sample_b_indexable(texture2d)(float,float,float,float) r0.y, r0.yzyy, t1.yxzw, s3, l(0.000000)
    r0.y = (LanceVANativeSample2((r0.yzyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 36: add r0.z, -cb0[4].x, l(1.000000)
    r0.z = ((-(source[4].xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 37: mul r1.xy, v4.xyxx, cb0[8].yzyy
    r1.xy = ((v4.xyxx)*(source[8].yzyy)).xy;
    // 38: mul r0.w, cb0[6].x, cb0[6].y
    r0.w = ((source[6].xxxx)*(source[6].yyyy)).w;
    // 39: mad r1.xy, r0.wwww, cb0[8].xwxx, r1.xyxx
    r1.xy = ((r0.wwww)*(source[8].xwxx)+(r1.xyxx)).xy;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r1.x, r1.xyxx, t0.xyzw, s2, l(0.000000)
    r1.x = (LanceVANativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 41: mad r0.y, r1.x, r0.y, -r0.z
    r0.y = ((r1.xxxx)*(r0.yyyy)+(-(r0.zzzz))).y;
    // 42: mul_sat r0.y, r0.y, cb0[10].y
    r0.y = (saturate((r0.yyyy)*(source[10].yyyy))).y;
    // 43: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 44: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 45: mul r0.z, r0.z, cb0[10].z
    r0.z = ((r0.zzzz)*(source[10].zzzz)).z;
    // 46: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 47: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 48: mul_sat r0.x, r0.x, cb0[1].w
    r0.x = (saturate((r0.xxxx)*(source[1].wwww))).x;
    // 49: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 50: movc o0.w, r0.y, l(0), r0.x
    output.w = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    // 51: mul r0.x, v4.x, cb0[6].w
    r0.x = ((v4.xxxx)*(source[6].wwww)).x;
    // 52: mad r0.x, r0.w, cb0[6].z, r0.x
    r0.x = ((r0.wwww)*(source[6].zzzz)+(r0.xxxx)).x;
    // 53: mul r0.z, v4.y, cb0[7].x
    r0.z = ((v4.yyyy)*(source[7].xxxx)).z;
    // 54: mad r0.y, r0.w, cb0[7].y, r0.z
    r0.y = ((r0.wwww)*(source[7].yyyy)+(r0.zzzz)).y;
    // 55: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t3.xyzw, s1, l(0.000000)
    r0.xyz = (LanceVANativeSample0((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 56: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 57: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 58: mad r0.xyz, cb0[7].zzzz, r1.xyzx, r0.xyzx
    r0.xyz = ((source[7].zzzz)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 59: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 60: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 61: mul r0.xyz, r0.xyzx, cb0[7].wwww
    r0.xyz = ((r0.xyzx)*(source[7].wwww)).xyz;
    // 62: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 63: mul r1.xyz, cb0[3].xyzx, cb0[3].wwww
    r1.xyz = ((source[3].xyzx)*(source[3].wwww)).xyz;
    // 64: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 65: mad r0.xyz, r0.xyzx, cb0[1].xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)*(source[1].xyzx)+(source[2].xyzx)).xyz;
    // 66: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

float4 LanceVANative1349(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[20]; [unroll] for (uint i=0u; i<20u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_LanceVASourceMaterialParameters[9u];
    source[3] = g_LanceVASourceMaterialParameters[6u];
    source[4] = g_LanceVASourceMaterialParameters[7u];
    source[5] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[1u].zzzz,g_LanceVASourceMaterialParameters[1u].wwww,1u);
    source[6] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[2u].wwww,g_LanceVASourceMaterialParameters[3u].yyyy,1u);
    source[7] = LanceVANativeAppend((g_LanceVASourceMaterialParameters[2u].zzzz*g_LanceVASourceMaterialTime.xxxx),(g_LanceVASourceMaterialParameters[3u].xxxx*g_LanceVASourceMaterialTime.xxxx),1u);
    source[8] = input.dynamicParameter;
    source[9] = LanceVANativeAppend(float4(0.0, 0.0, 0.0, 0.0),(g_LanceVASourceMaterialParameters[5u].xxxx*g_LanceVASourceMaterialTime.xxxx),1u);
    source[10] = LanceVANativeAppend(cos((g_LanceVASourceMaterialParameters[0u].wwww*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_LanceVASourceMaterialParameters[0u].wwww*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[11] = LanceVANativeAppend(sin((g_LanceVASourceMaterialParameters[0u].wwww*float4(0.25, 0.0, 0.0, 0.0))),cos((g_LanceVASourceMaterialParameters[0u].wwww*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[12] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[4u].yyyy,g_LanceVASourceMaterialParameters[4u].zzzz,1u);
    source[13] = LanceVANativeAppend(cos((g_LanceVASourceMaterialParameters[3u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_LanceVASourceMaterialParameters[3u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[14] = LanceVANativeAppend(sin((g_LanceVASourceMaterialParameters[3u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),cos((g_LanceVASourceMaterialParameters[3u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[15].x = ((g_LanceVASourceMaterialParameters[3u].xxxx*g_LanceVASourceMaterialTime.xxxx)).x;
    source[15].y = ((g_LanceVASourceMaterialParameters[2u].zzzz*g_LanceVASourceMaterialTime.xxxx)).x;
    source[15].z = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[15].w = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[16].x = (g_LanceVASourceMaterialParameters[5u].xxxx).x;
    source[16].y = ((g_LanceVASourceMaterialParameters[5u].xxxx*g_LanceVASourceMaterialTime.xxxx)).x;
    source[16].z = (g_LanceVASourceMaterialParameters[5u].zzzz).x;
    source[16].w = (g_LanceVASourceMaterialParameters[5u].yyyy).x;
    source[17].x = (g_LanceVASourceMaterialParameters[4u].wwww).x;
    source[17].y = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[17].z = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[17].w = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[18].x = (cos((g_LanceVASourceMaterialParameters[3u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[18].y = (g_LanceVASourceMaterialParameters[4u].zzzz).x;
    source[18].z = (g_LanceVASourceMaterialParameters[4u].yyyy).x;
    source[18].w = (g_LanceVASourceMaterialParameters[4u].xxxx).x;
    source[19].x = (g_LanceVASourceMaterialParameters[3u].wwww).x;
    source[19].y = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[19].z = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: add r0.x, v4.y, cb0[9].y
    r0.x = ((v4.yyyy)+(source[9].yyyy)).x;
    // 2: mul r0.x, r0.x, cb0[16].z
    r0.x = ((r0.xxxx)*(source[16].zzzz)).x;
    // 3: mul r0.x, r0.x, l(6.283185)
    r0.x = ((r0.xxxx)*(float4(6.283185,6.283185,6.283185,6.283185))).x;
    // 4: sincos r0.x, null, r0.x
    r0.x = (sin(r0.xxxx)).x;
    // 5: mul r0.y, cb0[8].z, cb0[16].w
    r0.y = ((source[8].zzzz)*(source[16].wwww)).y;
    // 6: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 7: add r0.y, -v4.y, l(1.000000)
    r0.y = ((-(v4.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 8: mul r0.y, r0.y, v4.y
    r0.y = ((r0.yyyy)*(v4.yyyy)).y;
    // 9: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 10: add r0.y, v4.x, l(-0.500000)
    r0.y = ((v4.xxxx)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).y;
    // 11: mad r0.x, r0.x, l(4.000000), r0.y
    r0.x = ((r0.xxxx)*(float4(4.000000,4.000000,4.000000,4.000000))+(r0.yyyy)).x;
    // 12: mad r0.zw, v4.xxxy, cb0[6].xxxy, cb0[7].xxxy
    r0.zw = ((v4.xxxy)*(source[6].xxxy)+(source[7].xxxy)).zw;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.zwzz, t2.zwxy, s0, l(0.000000)
    r0.zw = (LanceVANativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 14: mad r0.zw, r0.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 15: mad r1.xy, v4.xyxx, cb0[5].xyxx, l(0.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((v4.xyxx)*(source[5].xyxx)+(float4(0.000000,-1.000000,0.000000,0.000000))).xy;
    // 16: mad r0.zw, cb0[15].zzzz, r0.zzzw, r1.xxxy
    r0.zw = ((source[15].zzzz)*(r0.zzzw)+(r1.xxxy)).zw;
    // 17: add r1.x, r0.z, cb0[15].w
    r1.x = ((r0.zzzz)+(source[15].wwww)).x;
    // 18: add r1.y, r0.w, cb0[8].x
    r1.y = ((r0.wwww)+(source[8].xxxx)).y;
    // 19: mov r0.y, v4.y
    r0.y = (v4.yyyy).y;
    // 20: mad r0.xy, cb0[17].xxxx, r0.xyxx, r1.xyxx
    r0.xy = ((source[17].xxxx)*(r0.xyxx)+(r1.xyxx)).xy;
    // 21: add r0.xy, r0.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 22: dp2 r1.x, cb0[10].xyxx, r0.xyxx
    r1.x = (dot((source[10].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 23: dp2 r1.y, cb0[11].xyxx, r0.xyxx
    r1.y = (dot((source[11].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 24: add r0.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 25: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t3.xyzw, s1, l(0.000000)
    r0.xyz = (LanceVANativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 26: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 27: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 28: mad r0.xyz, cb0[17].yyyy, r1.xyzx, r0.xyzx
    r0.xyz = ((source[17].yyyy)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 29: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 30: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 31: mul r0.xyz, r0.xyzx, cb0[17].zzzz
    r0.xyz = ((r0.xyzx)*(source[17].zzzz)).xyz;
    // 32: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 33: mul r0.xyz, r0.xyzx, cb0[17].wwww
    r0.xyz = ((r0.xyzx)*(source[17].wwww)).xyz;
    // 34: add r1.xyz, -cb0[3].xyzx, cb0[4].xyzx
    r1.xyz = ((-(source[3].xyzx))+(source[4].xyzx)).xyz;
    // 35: mad r1.xyz, v4.yyyy, r1.xyzx, cb0[3].xyzx
    r1.xyz = ((v4.yyyy)*(r1.xyzx)+(source[3].xyzx)).xyz;
    // 36: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 37: mad r0.xyz, r0.xyzx, cb0[1].xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)*(source[1].xyzx)+(source[2].xyzx)).xyz;
    // 38: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    // 39: mul r0.x, v4.x, cb0[12].x
    r0.x = ((v4.xxxx)*(source[12].xxxx)).x;
    // 40: add r0.x, r0.x, cb0[18].w
    r0.x = ((r0.xxxx)+(source[18].wwww)).x;
    // 41: mad r0.z, v4.y, cb0[12].y, cb0[8].y
    r0.z = ((v4.yyyy)*(source[12].yyyy)+(source[8].yyyy)).z;
    // 42: add r0.xy, r0.xzxx, l(-0.500000, -1.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xzxx)+(float4(-0.500000,-1.500000,0.000000,0.000000))).xy;
    // 43: dp2 r1.x, cb0[13].xyxx, r0.xyxx
    r1.x = (dot((source[13].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 44: dp2 r1.y, cb0[14].xyxx, r0.xyxx
    r1.y = (dot((source[14].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 45: add r0.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 46: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t1.xyzw, s3, l(0.000000)
    r0.x = (LanceVANativeSample3((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 47: sample_b_indexable(texture2d)(float,float,float,float) r0.y, v4.xyxx, t0.yxzw, s2, l(0.000000)
    r0.y = (LanceVANativeSample2((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 48: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 49: mul_sat r0.x, r0.x, cb0[19].x
    r0.x = (saturate((r0.xxxx)*(source[19].xxxx))).x;
    // 50: mul r0.x, r0.x, cb0[1].w
    r0.x = ((r0.xxxx)*(source[1].wwww)).x;
    // 51: dp3 r0.y, v6.xyzx, v6.xyzx
    r0.y = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).y;
    // 52: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 53: mul r0.y, r0.y, v6.z
    r0.y = ((r0.yyyy)*(v6.zzzz)).y;
    // 54: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 55: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 56: mul r0.z, r0.z, cb0[19].y
    r0.z = ((r0.zzzz)*(source[19].yyyy)).z;
    // 57: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 58: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 59: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 60: movc o0.w, r0.y, l(0), r0.x
    output.w = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    return output;
}

float4 LanceVANative1350(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[8]; [unroll] for (uint i=0u; i<8u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_LanceVASourceMaterialParameters[5u];
    source[3] = input.dynamicParameter;
    source[4] = g_LanceVASourceMaterialParameters[3u];
    source[5].x = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[5].y = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[5].z = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[5].w = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[6].x = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[6].y = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[6].z = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[6].w = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[7].x = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.x, v4.xyxx, t1.xyzw, s0, l(0.000000)
    r0.x = (LanceVANativeSample0((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 2: mul r0.x, r0.x, cb0[5].x
    r0.x = ((r0.xxxx)*(source[5].xxxx)).x;
    // 3: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 4: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 5: mul r0.y, r0.y, cb0[5].y
    r0.y = ((r0.yyyy)*(source[5].yyyy)).y;
    // 6: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 7: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 8: mad r0.y, cb0[3].x, l(2.000000), l(-1.000000)
    r0.y = ((source[3].xxxx)*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 9: add r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)+(r0.yyyy)).x;
    // 10: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 11: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 12: mul r0.y, r0.y, cb0[5].z
    r0.y = ((r0.yyyy)*(source[5].zzzz)).y;
    // 13: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 14: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 15: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 16: ge r0.y, l(0.250000), r0.x
    r0.y = (asfloat((uint4)((float4(0.250000,0.250000,0.250000,0.250000))>=(r0.xxxx)) * 0xffffffffu)).y;
    // 17: mad r0.z, cb0[5].w, cb0[3].z, l(-1.000000)
    r0.z = ((source[5].wwww)*(source[3].zzzz)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).z;
    // 18: mul r0.z, r0.z, l(0.500000)
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 19: mul r0.w, cb0[3].z, cb0[5].w
    r0.w = ((source[3].zzzz)*(source[5].wwww)).w;
    // 20: mad r0.zw, r0.wwww, v4.xxxy, -r0.zzzz
    r0.zw = ((r0.wwww)*(v4.xxxy)+(-(r0.zzzz))).zw;
    // 21: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, r0.zwzz, t0.xyzw, s1, l(0.000000)
    r1.xyzw = (LanceVANativeSample1((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 22: mul r2.xyz, r1.xyzx, cb0[6].xxxx
    r2.xyz = ((r1.xyzx)*(source[6].xxxx)).xyz;
    // 23: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 24: mad r1.xyz, -cb0[6].xxxx, r1.xyzx, r0.zzzz
    r1.xyz = ((-(source[6].xxxx))*(r1.xyzx)+(r0.zzzz)).xyz;
    // 25: mad r1.xyz, cb0[6].yyyy, r1.xyzx, r2.xyzx
    r1.xyz = ((source[6].yyyy)*(r1.xyzx)+(r2.xyzx)).xyz;
    // 26: mul r2.xyz, r1.xyzx, cb0[6].zzzz
    r2.xyz = ((r1.xyzx)*(source[6].zzzz)).xyz;
    // 27: max r2.xyz, |r2.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r2.xyz = (max(abs(r2.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 28: log r2.xyz, r2.xyzx
    r2.xyz = (log2(r2.xyzx)).xyz;
    // 29: mul r2.xyz, r2.xyzx, cb0[6].wwww
    r2.xyz = ((r2.xyzx)*(source[6].wwww)).xyz;
    // 30: exp r2.xyz, r2.xyzx
    r2.xyz = (exp2(r2.xyzx)).xyz;
    // 31: dp3 r0.z, r2.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.z = (dot((r2.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).z;
    // 32: add r1.xyz, r1.xyzx, r0.zzzz
    r1.xyz = ((r1.xyzx)+(r0.zzzz)).xyz;
    // 33: mul r1.xyz, r1.xyzx, cb0[1].xyzx
    r1.xyz = ((r1.xyzx)*(source[1].xyzx)).xyz;
    // 34: mul r2.xyz, cb0[4].xyzx, cb0[4].wwww
    r2.xyz = ((source[4].xyzx)*(source[4].wwww)).xyz;
    // 35: movc r0.yzw, r0.yyyy, r2.xxyz, r1.xxyz
    r0.yzw = ((asuint(r0.yyyy) != 0u) ? (r2.xxyz) : (r1.xxyz)).yzw;
    // 36: add r0.yzw, r0.yyzw, cb0[2].xxyz
    r0.yzw = ((r0.yyzw)+(source[2].xxyz)).yzw;
    // 37: mad o0.xyz, r0.yzwy, v5.wwww, v5.xyzx
    output.xyz = ((r0.yzwy)*(v5.wwww)+(v5.xyzx)).xyz;
    // 38: log r0.y, |r1.w|
    r0.y = (log2(abs(r1.wwww))).y;
    // 39: lt r0.z, |r1.w|, l(0.000001)
    r0.z = (asfloat((uint4)((abs(r1.wwww))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).z;
    // 40: mul r0.y, r0.y, cb0[7].x
    r0.y = ((r0.yyyy)*(source[7].xxxx)).y;
    // 41: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 42: mul r0.y, r0.y, cb0[1].w
    r0.y = ((r0.yyyy)*(source[1].wwww)).y;
    // 43: movc r0.y, r0.z, l(0), |r0.y|
    r0.y = ((asuint(r0.zzzz) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (abs(r0.yyyy))).y;
    // 44: log r0.z, r0.y
    r0.z = (log2(r0.yyyy)).z;
    // 45: lt r0.y, r0.y, l(0.000001)
    r0.y = (asfloat((uint4)((r0.yyyy)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 46: mul r0.z, r0.z, cb0[3].y
    r0.z = ((r0.zzzz)*(source[3].yyyy)).z;
    // 47: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 48: mul r0.x, r0.x, r0.z
    r0.x = ((r0.xxxx)*(r0.zzzz)).x;
    // 49: min r0.x, r0.x, l(1.000000)
    r0.x = (min(r0.xxxx,float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 50: mul r0.x, r0.x, cb0[0].x
    r0.x = ((r0.xxxx)*(source[0].xxxx)).x;
    // 51: movc o0.w, r0.y, l(0), r0.x
    output.w = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.xxxx)).w;
    return output;
}

float4 LanceVANative1351(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[16]; [unroll] for (uint i=0u; i<16u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_LanceVASourceMaterialParameters[9u];
    source[3] = input.dynamicParameter;
    source[4] = LanceVANativeAppend(cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[5u].yyyy)*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[5u].yyyy)*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[5] = LanceVANativeAppend(sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[5u].yyyy)*float4(0.25, 0.0, 0.0, 0.0))),cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[5u].yyyy)*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[6] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[4u].yyyy,g_LanceVASourceMaterialParameters[4u].zzzz,1u);
    source[7].x = (g_LanceVASourceMaterialParameters[4u].wwww).x;
    source[7].y = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[7].z = (g_LanceVASourceMaterialTime.xxxx).x;
    source[7].w = (g_LanceVASourceMaterialParameters[5u].zzzz).x;
    source[8].x = (g_LanceVASourceMaterialParameters[5u].wwww).x;
    source[8].y = (g_LanceVASourceMaterialParameters[5u].yyyy).x;
    source[8].z = ((float4(3.1400001, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[5u].yyyy)).x;
    source[8].w = (((float4(3.1400001, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[5u].yyyy)*float4(0.25, 0.0, 0.0, 0.0))).x;
    source[9].x = (sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[5u].yyyy)*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[9].y = ((float4(-1.0, 0.0, 0.0, 0.0)*sin(((float4(3.1400001, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[5u].yyyy)*float4(0.25, 0.0, 0.0, 0.0))))).x;
    source[9].z = (cos(((float4(3.1400001, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[5u].yyyy)*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[9].w = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[10].x = (g_LanceVASourceMaterialParameters[4u].zzzz).x;
    source[10].y = (g_LanceVASourceMaterialParameters[4u].yyyy).x;
    source[10].z = (g_LanceVASourceMaterialParameters[6u].xxxx).x;
    source[10].w = (g_LanceVASourceMaterialParameters[6u].zzzz).x;
    source[11].x = (g_LanceVASourceMaterialParameters[7u].xxxx).x;
    source[11].y = (g_LanceVASourceMaterialParameters[7u].yyyy).x;
    source[11].z = (g_LanceVASourceMaterialParameters[6u].wwww).x;
    source[11].w = (g_LanceVASourceMaterialParameters[6u].yyyy).x;
    source[12].x = (g_LanceVASourceMaterialParameters[7u].zzzz).x;
    source[12].y = (g_LanceVASourceMaterialParameters[5u].xxxx).x;
    source[12].z = (g_LanceVASourceMaterialParameters[4u].xxxx).x;
    source[12].w = (g_LanceVASourceMaterialParameters[3u].zzzz).x;
    source[13].x = (g_LanceVASourceMaterialParameters[3u].wwww).x;
    source[13].y = (g_LanceVASourceMaterialParameters[3u].yyyy).x;
    source[13].z = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[13].w = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[14].x = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[14].y = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[14].z = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[14].w = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[15].x = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[15].y = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[15].z = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[15].w = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f, r4=0.f;
    // 1: div r0.xy, v7.xyxx, v7.wwww
    r0.xy = ((v7.xyxx)/(v7.wwww)).xy;
    // 2: mad r0.xy, r0.xyxx, cb2[0].xyxx, cb2[0].wzww
    r0.xy = ((r0.xyxx)*(passValues[0].xyxx)+(passValues[0].wzww)).xy;
    // Native 3: source device depth mapped to centimetre view depth; reconstruction at 5.
    r0.x = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.xyxx).xy, 0.f).y * 100000.f;
    // Native 5-8: reconstructed view depth is supplied by the runtime adapter.
    r0.x = r0.x;
    // 9: add r0.x, r0.x, -v7.w
    r0.x = ((r0.xxxx)+(-(v7.wwww))).x;
    // 10: mul_sat r0.x, r0.x, l(0.007143)
    r0.x = (saturate((r0.xxxx)*(float4(0.007143,0.007143,0.007143,0.007143)))).x;
    // 11: mul_sat r0.x, r0.x, cb0[15].w
    r0.x = (saturate((r0.xxxx)*(source[15].wwww))).x;
    // 12: dp3 r0.y, v6.xyzx, v6.xyzx
    r0.y = (dot((v6.xyzx).xyz,(v6.xyzx).xyz).xxxx).y;
    // 13: rsq r0.y, r0.y
    r0.y = (rsqrt(r0.yyyy)).y;
    // 14: mul r0.y, r0.y, v6.z
    r0.y = ((r0.yyyy)*(v6.zzzz)).y;
    // 15: log r0.z, |r0.y|
    r0.z = (log2(abs(r0.yyyy))).z;
    // 16: lt r0.y, |r0.y|, l(0.000001)
    r0.y = (asfloat((uint4)((abs(r0.yyyy))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).y;
    // 17: mul r0.z, r0.z, cb0[15].y
    r0.z = ((r0.zzzz)*(source[15].yyyy)).z;
    // 18: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 19: movc r0.y, r0.y, l(0), r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).y;
    // 20: add r0.zw, v4.xxxy, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((v4.xxxy)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 21: dp2 r1.x, cb0[4].xyxx, r0.zwzz
    r1.x = (dot((source[4].xyxx).xy,(r0.zwzz).xy).xxxx).x;
    // 22: dp2 r1.y, cb0[5].xyxx, r0.zwzz
    r1.y = (dot((source[5].xyxx).xy,(r0.zwzz).xy).xxxx).y;
    // 23: add r0.zw, r1.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r1.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 24: mul r1.x, r0.z, cb0[7].w
    r1.x = ((r0.zzzz)*(source[7].wwww)).x;
    // 25: mad r1.x, cb0[7].z, cb0[7].y, r1.x
    r1.x = ((source[7].zzzz)*(source[7].yyyy)+(r1.xxxx)).x;
    // 26: mul r1.z, r0.w, cb0[8].x
    r1.z = ((r0.wwww)*(source[8].xxxx)).z;
    // 27: mad r1.y, cb0[7].z, cb0[9].w, r1.z
    r1.y = ((source[7].zzzz)*(source[9].wwww)+(r1.zzzz)).y;
    // 28: add r1.xy, r1.xyxx, cb0[6].xyxx
    r1.xy = ((r1.xyxx)+(source[6].xyxx)).xy;
    // 29: add r1.z, cb0[3].w, cb0[12].x
    r1.z = ((source[3].wwww)+(source[12].xxxx)).z;
    // 30: add r1.z, r1.z, l(-1.000000)
    r1.z = ((r1.zzzz)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).z;
    // 31: mul r2.xy, r0.zwzz, cb0[11].xyxx
    r2.xy = ((r0.zwzz)*(source[11].xyxx)).xy;
    // 32: mul r0.zw, r0.zzzw, cb0[14].xxxy
    r0.zw = ((r0.zzzw)*(source[14].xxxy)).zw;
    // 33: mad r1.w, cb0[7].z, cb0[10].w, r2.x
    r1.w = ((source[7].zzzz)*(source[10].wwww)+(r2.xxxx)).w;
    // 34: mad r2.x, cb0[7].z, cb0[11].z, r2.y
    r2.x = ((source[7].zzzz)*(source[11].zzzz)+(r2.yyyy)).x;
    // 35: mad r2.y, cb0[3].y, cb0[11].w, r2.x
    r2.y = ((source[3].yyyy)*(source[11].wwww)+(r2.xxxx)).y;
    // 36: mad r2.x, cb0[3].y, cb0[10].z, r1.w
    r2.x = ((source[3].yyyy)*(source[10].zzzz)+(r1.wwww)).x;
    // 37: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, r2.xyxx, t0.xyzw, s1, l(0.000000)
    r2.xy = (LanceVANativeSample0((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 38: mad r1.xy, r1.zzzz, r2.xyxx, r1.xyxx
    r1.xy = ((r1.zzzz)*(r2.xyxx)+(r1.xyxx)).xy;
    // 39: add r1.zw, cb0[3].xxxz, l(0.000000, 0.000000, -1.000000, -1.000000)
    r1.zw = ((source[3].xxxz)+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 40: mad r2.x, r1.z, cb0[7].x, r1.x
    r2.x = ((r1.zzzz)*(source[7].xxxx)+(r1.xxxx)).x;
    // 41: mad r2.y, r1.z, cb0[12].y, r1.y
    r2.y = ((r1.zzzz)*(source[12].yyyy)+(r1.yyyy)).y;
    // 42: sample_b_indexable(texture2d)(float,float,float,float) r1.xyz, r2.xyxx, t2.xyzw, s3, l(0.000000)
    r1.xyz = (LanceVANativeSample2((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 43: sample_b_indexable(texture2d)(float,float,float,float) r2.xy, r2.xyxx, t1.xyzw, s2, l(0.000000)
    r2.xy = (LanceVANativeSample1((r2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 44: mad r2.xy, r2.xyxx, l(2.000000, 2.000000, 0.000000, 0.000000), l(-1.000000, -1.000000, 0.000000, 0.000000)
    r2.xy = ((r2.xyxx)*(float4(2.000000,2.000000,0.000000,0.000000))+(float4(-1.000000,-1.000000,0.000000,0.000000))).xy;
    // 45: mul r3.xyzw, r1.xxyz, cb0[1].wxyz
    r3.xyzw = ((r1.xxyz)*(source[1].wxyz)).xyzw;
    // 46: mad r0.z, cb0[7].z, cb0[13].w, r0.z
    r0.z = ((source[7].zzzz)*(source[13].wwww)+(r0.zzzz)).z;
    // 47: mad r0.w, cb0[7].z, cb0[14].z, r0.w
    r0.w = ((source[7].zzzz)*(source[14].zzzz)+(r0.wwww)).w;
    // 48: mad r4.y, cb0[3].y, cb0[14].w, r0.w
    r4.y = ((source[3].yyyy)*(source[14].wwww)+(r0.wwww)).y;
    // 49: mad r4.x, cb0[3].y, cb0[13].z, r0.z
    r4.x = ((source[3].yyyy)*(source[13].zzzz)+(r0.zzzz)).x;
    // 50: add r0.zw, r4.xxxy, l(0.000000, 0.000000, -0.500000, -0.500000)
    r0.zw = ((r4.xxxy)+(float4(0.000000,0.000000,-0.500000,-0.500000))).zw;
    // 51: dp2 r4.x, l(0.000796, -1.000000, 0.000000, 0.000000), r0.zwzz
    r4.x = (dot((float4(0.000796,-1.000000,0.000000,0.000000)).xy,(r0.zwzz).xy).xxxx).x;
    // 52: dp2 r4.y, l(1.000000, 0.000796, 0.000000, 0.000000), r0.zwzz
    r4.y = (dot((float4(1.000000,0.000796,0.000000,0.000000)).xy,(r0.zwzz).xy).xxxx).y;
    // 53: add r0.zw, r4.xxxy, l(0.000000, 0.000000, 0.500000, 0.500000)
    r0.zw = ((r4.xxxy)+(float4(0.000000,0.000000,0.500000,0.500000))).zw;
    // 54: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t0.yzxw, s1, l(0.000000)
    r0.z = (LanceVANativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 55: add r0.z, -r1.w, r0.z
    r0.z = ((-(r1.wwww))+(r0.zzzz)).z;
    // 56: mul_sat r0.z, r0.z, cb0[15].x
    r0.z = (saturate((r0.zzzz)*(source[15].xxxx))).z;
    // 57: mul r0.z, r0.z, r3.x
    r0.z = ((r0.zzzz)*(r3.xxxx)).z;
    // 58: mul r0.y, r0.y, r0.z
    r0.y = ((r0.yyyy)*(r0.zzzz)).y;
    // 59: mul_sat r0.y, r0.y, cb0[15].z
    r0.y = (saturate((r0.yyyy)*(source[15].zzzz))).y;
    // 60: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 61: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 62: dp3 r0.x, r3.yzwy, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.x = (dot((r3.yzwy).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).x;
    // 63: mad r0.xyz, -cb0[1].xyzx, r1.xyzx, r0.xxxx
    r0.xyz = ((-(source[1].xyzx))*(r1.xyzx)+(r0.xxxx)).xyz;
    // 64: mad r0.xyz, cb0[12].zzzz, r0.xyzx, r3.yzwy
    r0.xyz = ((source[12].zzzz)*(r0.xyzx)+(r3.yzwy)).xyz;
    // 65: mul r1.xyz, r0.xyzx, cb0[13].yyyy
    r1.xyz = ((r0.xyzx)*(source[13].yyyy)).xyz;
    // 66: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 67: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 68: mul r0.xyz, r0.xyzx, cb0[12].wwww
    r0.xyz = ((r0.xyzx)*(source[12].wwww)).xyz;
    // 69: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 70: mad r0.xyz, cb0[13].xxxx, r0.xyzx, r1.xyzx
    r0.xyz = ((source[13].xxxx)*(r0.xyzx)+(r1.xyzx)).xyz;
    // 71: add r0.xyz, r0.xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)+(source[2].xyzx)).xyz;
    // 72: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

float4 LanceVANative1352(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[10]; [unroll] for (uint i=0u; i<10u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_LanceVASourceMaterialParameters[4u];
    source[2] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[2u].xxxx,g_LanceVASourceMaterialParameters[2u].yyyy,1u);
    source[3] = LanceVANativeAppend(cos((g_LanceVASourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_LanceVASourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[4] = LanceVANativeAppend(sin((g_LanceVASourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),cos((g_LanceVASourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[5].x = (cos((g_LanceVASourceMaterialParameters[1u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[5].y = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[5].z = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[5].w = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[6].x = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[6].y = (g_LanceVASourceMaterialParameters[3u].yyyy).x;
    source[6].z = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[6].w = (g_LanceVASourceMaterialParameters[3u].zzzz).x;
    source[7].x = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[7].y = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[7].z = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[7].w = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[8].x = (max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_LanceVASourceMaterialParameters[0u].zzzz)).x;
    source[8].y = ((float4(1.0, 0.0, 0.0, 0.0)/max(float4(9.99999975e-06, 0.0, 0.0, 0.0),g_LanceVASourceMaterialParameters[0u].zzzz))).x;
    source[8].z = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[8].w = ((float4(1.0, 0.0, 0.0, 0.0)-g_LanceVASourceMaterialParameters[0u].xxxx)).x;
    source[9].x = (max((float4(1.0, 0.0, 0.0, 0.0)-g_LanceVASourceMaterialParameters[0u].xxxx),float4(9.99999975e-06, 0.0, 0.0, 0.0))).x;
    source[9].y = ((float4(1.0, 0.0, 0.0, 0.0)/max((float4(1.0, 0.0, 0.0, 0.0)-g_LanceVASourceMaterialParameters[0u].xxxx),float4(9.99999975e-06, 0.0, 0.0, 0.0)))).x;
    source[9].z = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[9].w = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f;
    // 1: add r0.xy, v2.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((v2.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 2: max r0.z, |r0.y|, |r0.x|
    r0.z = (max(abs(r0.yyyy),abs(r0.xxxx))).z;
    // 3: div r0.z, l(1.000000, 1.000000, 1.000000, 1.000000), r0.z
    r0.z = ((float4(1.000000,1.000000,1.000000,1.000000))/(r0.zzzz)).z;
    // 4: min r0.w, |r0.y|, |r0.x|
    r0.w = (min(abs(r0.yyyy),abs(r0.xxxx))).w;
    // 5: mul r0.z, r0.z, r0.w
    r0.z = ((r0.zzzz)*(r0.wwww)).z;
    // 6: mul r0.w, r0.z, r0.z
    r0.w = ((r0.zzzz)*(r0.zzzz)).w;
    // 7: mad r1.x, r0.w, l(0.020835), l(-0.085133)
    r1.x = ((r0.wwww)*(float4(0.020835,0.020835,0.020835,0.020835))+(float4(-0.085133,-0.085133,-0.085133,-0.085133))).x;
    // 8: mad r1.x, r0.w, r1.x, l(0.180141)
    r1.x = ((r0.wwww)*(r1.xxxx)+(float4(0.180141,0.180141,0.180141,0.180141))).x;
    // 9: mad r1.x, r0.w, r1.x, l(-0.330299)
    r1.x = ((r0.wwww)*(r1.xxxx)+(float4(-0.330299,-0.330299,-0.330299,-0.330299))).x;
    // 10: mad r0.w, r0.w, r1.x, l(0.999866)
    r0.w = ((r0.wwww)*(r1.xxxx)+(float4(0.999866,0.999866,0.999866,0.999866))).w;
    // 11: mul r1.x, r0.w, r0.z
    r1.x = ((r0.wwww)*(r0.zzzz)).x;
    // 12: mad r1.x, r1.x, l(-2.000000), l(1.570796)
    r1.x = ((r1.xxxx)*(float4(-2.000000,-2.000000,-2.000000,-2.000000))+(float4(1.570796,1.570796,1.570796,1.570796))).x;
    // 13: lt r1.y, |r0.y|, |r0.x|
    r1.y = (asfloat((uint4)((abs(r0.yyyy))<(abs(r0.xxxx))) * 0xffffffffu)).y;
    // 14: and r1.x, r1.y, r1.x
    r1.x = (asfloat(asuint(r1.yyyy) & asuint(r1.xxxx))).x;
    // 15: mad r0.z, r0.z, r0.w, r1.x
    r0.z = ((r0.zzzz)*(r0.wwww)+(r1.xxxx)).z;
    // 16: lt r0.w, r0.y, -r0.y
    r0.w = (asfloat((uint4)((r0.yyyy)<(-(r0.yyyy))) * 0xffffffffu)).w;
    // 17: and r0.w, r0.w, l(0xc0490fdb)
    r0.w = (asfloat(asuint(r0.wwww) & uint4(0xc0490fdbu,0xc0490fdbu,0xc0490fdbu,0xc0490fdbu))).w;
    // 18: add r0.z, r0.w, r0.z
    r0.z = ((r0.wwww)+(r0.zzzz)).z;
    // 19: min r0.w, r0.y, r0.x
    r0.w = (min(r0.yyyy,r0.xxxx)).w;
    // 20: lt r0.w, r0.w, -r0.w
    r0.w = (asfloat((uint4)((r0.wwww)<(-(r0.wwww))) * 0xffffffffu)).w;
    // 21: max r1.x, r0.y, r0.x
    r1.x = (max(r0.yyyy,r0.xxxx)).x;
    // 22: dp2 r0.x, r0.xyxx, r0.xyxx
    r0.x = (dot((r0.xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 23: ge r0.y, r1.x, -r1.x
    r0.y = (asfloat((uint4)((r1.xxxx)>=(-(r1.xxxx))) * 0xffffffffu)).y;
    // 24: and r0.y, r0.y, r0.w
    r0.y = (asfloat(asuint(r0.yyyy) & asuint(r0.wwww))).y;
    // 25: movc r0.y, r0.y, -r0.z, r0.z
    r0.y = ((asuint(r0.yyyy) != 0u) ? (-(r0.zzzz)) : (r0.zzzz)).y;
    // 26: mad r0.z, r0.y, l(0.318310), l(1.000000)
    r0.z = ((r0.yyyy)*(float4(0.318310,0.318310,0.318310,0.318310))+(float4(1.000000,1.000000,1.000000,1.000000))).z;
    // 27: mad r0.y, -r0.y, l(0.318310), l(1.000000)
    r0.y = ((-(r0.yyyy))*(float4(0.318310,0.318310,0.318310,0.318310))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 28: mul r0.z, r0.z, l(0.500000)
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))).z;
    // 29: mul r1.y, r0.z, cb0[6].y
    r1.y = ((r0.zzzz)*(source[6].yyyy)).y;
    // 30: mul r1.x, r0.z, cb0[5].w
    r1.x = ((r0.zzzz)*(source[5].wwww)).x;
    // 31: log r0.z, r0.x
    r0.z = (log2(r0.xxxx)).z;
    // 32: mul r0.z, r0.z, l(0.800000)
    r0.z = ((r0.zzzz)*(float4(0.800000,0.800000,0.800000,0.800000))).z;
    // 33: exp r0.z, r0.z
    r0.z = (exp2(r0.zzzz)).z;
    // 34: lt r0.w, r0.x, l(0.000001)
    r0.w = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).w;
    // 35: sqrt r0.x, r0.x
    r0.x = (sqrt(r0.xxxx)).x;
    // 36: movc r0.z, r0.w, l(0), r0.z
    r0.z = ((asuint(r0.wwww) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.zzzz)).z;
    // 37: mul r1.zw, r0.zzzz, cb0[6].xxxz
    r1.zw = ((r0.zzzz)*(source[6].xxxz)).zw;
    // 38: mad r0.zw, v4.zzzz, l(0.000000, 0.000000, 0.800000, -0.400000), r1.yyyw
    r0.zw = ((v4.zzzz)*(float4(0.000000,0.000000,0.800000,-0.400000))+(r1.yyyw)).zw;
    // 39: mad r1.yw, v4.zzzz, l(0.000000, 0.500000, 0.000000, -0.200000), r1.yyyw
    r1.yw = ((v4.zzzz)*(float4(0.000000,0.500000,0.000000,-0.200000))+(r1.yyyw)).yw;
    // 40: sample_b_indexable(texture2d)(float,float,float,float) r1.y, r1.ywyy, t0.yxzw, s0, l(0.000000)
    r1.y = (LanceVANativeSample0((r1.ywyy).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yxzw).y;
    // 41: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r0.zwzz, t0.yzxw, s0, l(0.000000)
    r0.z = (LanceVANativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 42: add r0.z, r1.y, r0.z
    r0.z = ((r1.yyyy)+(r0.zzzz)).z;
    // 43: mul r0.w, r1.y, cb0[6].w
    r0.w = ((r1.yyyy)*(source[6].wwww)).w;
    // 44: mad r1.yz, r0.wwww, v4.wwww, r1.xxzx
    r1.yz = ((r0.wwww)*(v4.wwww)+(r1.xxzx)).yz;
    // 45: mul r0.y, r0.y, r1.x
    r0.y = ((r0.yyyy)*(r1.xxxx)).y;
    // 46: mad r1.xw, v4.zzzz, l(0.400000, 0.000000, 0.000000, -0.600000), r1.yyyz
    r1.xw = ((v4.zzzz)*(float4(0.400000,0.000000,0.000000,-0.600000))+(r1.yyyz)).xw;
    // 47: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xwxx, t2.yzwx, s2, l(0.000000)
    r0.w = (LanceVANativeSample2((r1.xwxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 48: add r0.z, -r0.w, r0.z
    r0.z = ((-(r0.wwww))+(r0.zzzz)).z;
    // 49: mad r0.z, r0.z, l(0.500000), r0.w
    r0.z = ((r0.zzzz)*(float4(0.500000,0.500000,0.500000,0.500000))+(r0.wwww)).z;
    // 50: mad r1.x, -r0.x, l(2.000000), l(1.000000)
    r1.x = ((-(r0.xxxx))*(float4(2.000000,2.000000,2.000000,2.000000))+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 51: mad r0.x, -r0.x, cb0[8].y, l(1.000000)
    r0.x = ((-(r0.xxxx))*(source[8].yyyy)+(float4(1.000000,1.000000,1.000000,1.000000))).x;
    // 52: mul_sat r0.x, r0.x, cb0[9].y
    r0.x = (saturate((r0.xxxx)*(source[9].yyyy))).x;
    // 53: mul r0.x, r0.x, cb0[9].z
    r0.x = ((r0.xxxx)*(source[9].zzzz)).x;
    // 54: mul r1.x, r1.x, l(0.666667)
    r1.x = ((r1.xxxx)*(float4(0.666667,0.666667,0.666667,0.666667))).x;
    // 55: max r1.x, r1.x, l(0.000000)
    r1.x = (max(r1.xxxx,float4(0.000000,0.000000,0.000000,0.000000))).x;
    // 56: mul r0.y, r0.y, r1.x
    r0.y = ((r0.yyyy)*(r1.xxxx)).y;
    // 57: mul_sat r0.y, r0.y, l(10.000000)
    r0.y = (saturate((r0.yyyy)*(float4(10.000000,10.000000,10.000000,10.000000)))).y;
    // 58: add r1.xw, v4.xxxy, l(-1.000000, 0.000000, 0.000000, -1.000000)
    r1.xw = ((v4.xxxy)+(float4(-1.000000,0.000000,0.000000,-1.000000))).xw;
    // 59: mad r0.y, r0.z, r0.y, -r1.w
    r0.y = ((r0.zzzz)*(r0.yyyy)+(-(r1.wwww))).y;
    // 60: mad r1.xy, r1.xxxx, l(0.500000, -0.400000, 0.000000, 0.000000), r1.yzyy
    r1.xy = ((r1.xxxx)*(float4(0.500000,-0.400000,0.000000,0.000000))+(r1.yzyy)).xy;
    // 61: add r1.xy, r1.xyxx, cb0[2].xyxx
    r1.xy = ((r1.xyxx)+(source[2].xyxx)).xy;
    // 62: add r1.xy, r1.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 63: mul_sat r0.y, r0.y, l(10.000000)
    r0.y = (saturate((r0.yyyy)*(float4(10.000000,10.000000,10.000000,10.000000)))).y;
    // 64: dp2 r2.x, cb0[3].xyxx, r1.xyxx
    r2.x = (dot((source[3].xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 65: dp2 r2.y, cb0[4].xyxx, r1.xyxx
    r2.y = (dot((source[4].xyxx).xy,(r1.xyxx).xy).xxxx).y;
    // 66: add r1.xy, r2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r2.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 67: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.xyxx, t1.yzxw, s1, l(0.000000)
    r0.z = (LanceVANativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 68: mul_sat r1.x, r0.z, l(20.000000)
    r1.x = (saturate((r0.zzzz)*(float4(20.000000,20.000000,20.000000,20.000000)))).x;
    // 69: mad_sat r0.x, r1.x, r0.y, -r0.x
    r0.x = (saturate((r1.xxxx)*(r0.yyyy)+(-(r0.xxxx)))).x;
    // 70: mul_sat r0.x, r0.x, v3.w
    r0.x = (saturate((r0.xxxx)*(v3.wwww))).x;
    // 71: mul o0.w, r0.x, cb0[0].x
    output.w = ((r0.xxxx)*(source[0].xxxx)).w;
    // 72: mul r0.x, r0.w, r0.z
    r0.x = ((r0.wwww)*(r0.zzzz)).x;
    // 73: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 74: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 75: mul r0.y, r0.y, cb0[7].x
    r0.y = ((r0.yyyy)*(source[7].xxxx)).y;
    // 76: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 77: mul r0.y, r0.y, cb0[7].y
    r0.y = ((r0.yyyy)*(source[7].yyyy)).y;
    // 78: movc r0.x, r0.x, l(0), r0.y
    r0.x = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).x;
    // 79: mad r0.x, r0.z, cb0[7].z, r0.x
    r0.x = ((r0.zzzz)*(source[7].zzzz)+(r0.xxxx)).x;
    // 80: mad r0.xyz, r0.xxxx, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((r0.xxxx)*(v3.xyzx)+(source[1].xyzx)).xyz;
    // 81: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

float4 LanceVANative1353(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[8]; [unroll] for (uint i=0u; i<8u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_LanceVASourceMaterialParameters[5u];
    source[2] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[2u].zzzz,g_LanceVASourceMaterialParameters[2u].wwww,1u);
    source[3].x = (g_LanceVASourceMaterialParameters[4u].xxxx).x;
    source[3].y = (g_LanceVASourceMaterialTime.xxxx).x;
    source[3].z = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[3].w = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[4].x = (g_LanceVASourceMaterialParameters[0u].zzzz).x;
    source[4].y = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[4].z = (g_LanceVASourceMaterialParameters[1u].zzzz).x;
    source[4].w = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    source[5].x = (g_LanceVASourceMaterialParameters[2u].yyyy).x;
    source[5].y = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[5].z = ((float4(1.0, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[1u].xxxx)).x;
    source[5].w = (g_LanceVASourceMaterialParameters[1u].yyyy).x;
    source[6].x = (g_LanceVASourceMaterialParameters[3u].xxxx).x;
    source[6].y = (g_LanceVASourceMaterialParameters[3u].yyyy).x;
    source[6].z = ((float4(-0.523599029, 0.0, 0.0, 0.0)*g_LanceVASourceMaterialParameters[3u].xxxx)).x;
    source[6].w = (g_LanceVASourceMaterialParameters[2u].wwww).x;
    source[7].x = (g_LanceVASourceMaterialParameters[2u].zzzz).x;
    source[7].y = (g_LanceVASourceMaterialParameters[3u].zzzz).x;
    source[7].z = (g_LanceVASourceMaterialParameters[3u].wwww).x;
    source[7].w = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,float2(0.f,0.f)); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f, r3=0.f;
    // 1: mul r0.xy, v2.xyxx, cb0[4].zwzz
    r0.xy = ((v2.xyxx)*(source[4].zwzz)).xy;
    // 2: mul r0.z, cb0[3].x, cb0[3].y
    r0.z = ((source[3].xxxx)*(source[3].yyyy)).z;
    // 3: mad r1.x, r0.z, cb0[4].y, r0.x
    r1.x = ((r0.zzzz)*(source[4].yyyy)+(r0.xxxx)).x;
    // 4: mad r1.y, r0.z, cb0[5].x, r0.y
    r1.y = ((r0.zzzz)*(source[5].xxxx)+(r0.yyyy)).y;
    // 5: sample_b_indexable(texture2d)(float,float,float,float) r0.xy, r1.xyxx, t0.xyzw, s1, l(0.000000)
    r0.xy = (LanceVANativeSample0((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xy;
    // 6: mad r0.xy, cb0[5].zzzz, r0.xyxx, v2.xyxx
    r0.xy = ((source[5].zzzz)*(r0.xyxx)+(v2.xyxx)).xy;
    // 7: mul r0.w, r0.x, cb0[3].w
    r0.w = ((r0.xxxx)*(source[3].wwww)).w;
    // 8: mad r1.x, r0.z, cb0[3].z, r0.w
    r1.x = ((r0.zzzz)*(source[3].zzzz)+(r0.wwww)).x;
    // 9: mul r0.z, r0.z, cb0[5].w
    r0.z = ((r0.zzzz)*(source[5].wwww)).z;
    // 10: mad r1.y, cb0[4].x, r0.y, r0.z
    r1.y = ((source[4].xxxx)*(r0.yyyy)+(r0.zzzz)).y;
    // 11: add r0.xy, r0.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 12: sample_b_indexable(texture2d)(float,float,float,float) r0.z, r1.xyxx, t1.yzxw, s2, l(0.000000)
    r0.z = (LanceVANativeSample1((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzxw).z;
    // 13: mad r0.w, cb0[3].y, cb0[6].y, cb0[6].z
    r0.w = ((source[3].yyyy)*(source[6].yyyy)+(source[6].zzzz)).w;
    // 14: sincos r1.x, r2.x, r0.w
    r1.x = (sin(r0.wwww)).x; r2.x = (cos(r0.wwww)).x;
    // 15: mov r3.x, -r1.x
    r3.x = (-(r1.xxxx)).x;
    // 16: mov r3.y, r2.x
    r3.y = (r2.xxxx).y;
    // 17: mov r3.z, r1.x
    r3.z = (r1.xxxx).z;
    // 18: dp2 r0.w, r3.zyzz, r0.xyxx
    r0.w = (dot((r3.zyzz).xy,(r0.xyxx).xy).xxxx).w;
    // 19: dp2 r0.x, r3.yxyy, r0.xyxx
    r0.x = (dot((r3.yxyy).xy,(r0.xyxx).xy).xxxx).x;
    // 20: mul r1.z, r0.w, cb0[2].y
    r1.z = ((r0.wwww)*(source[2].yyyy)).z;
    // 21: add r0.y, v4.y, l(-1.000000)
    r0.y = ((v4.yyyy)+(float4(-1.000000,-1.000000,-1.000000,-1.000000))).y;
    // 22: mad r1.x, r0.x, cb0[2].x, r0.y
    r1.x = ((r0.xxxx)*(source[2].xxxx)+(r0.yyyy)).x;
    // 23: add r0.xy, r1.xzxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xzxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 24: sample_b_indexable(texture2d)(float,float,float,float) r0.x, r0.xyxx, t2.xyzw, s3, l(0.000000)
    r0.x = (LanceVANativeSample2((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 25: add r0.y, -v4.x, l(1.000000)
    r0.y = ((-(v4.xxxx))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 26: mad r0.x, r0.z, r0.x, -r0.y
    r0.x = ((r0.zzzz)*(r0.xxxx)+(-(r0.yyyy))).x;
    // 27: mul_sat r0.x, r0.x, cb0[7].y
    r0.x = (saturate((r0.xxxx)*(source[7].yyyy))).x;
    // 28: log r0.y, r0.x
    r0.y = (log2(r0.xxxx)).y;
    // 29: lt r0.x, r0.x, l(0.000001)
    r0.x = (asfloat((uint4)((r0.xxxx)<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 30: mul r0.y, r0.y, cb0[7].z
    r0.y = ((r0.yyyy)*(source[7].zzzz)).y;
    // 31: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 32: div r0.zw, v7.xxxy, v7.wwww
    r0.zw = ((v7.xxxy)/(v7.wwww)).zw;
    // 33: mad r0.zw, r0.zzzw, cb2[0].xxxy, cb2[0].wwwz
    r0.zw = ((r0.zzzw)*(passValues[0].xxxy)+(passValues[0].wwwz)).zw;
    // Native 34: source device depth mapped to centimetre view depth; reconstruction at 36.
    r0.z = g_EffectSceneDepthTexture.SampleLevel(EffectSliceDepthSampler, (r0.zwzz).xy, 0.f).y * 100000.f;
    // Native 36-39: reconstructed view depth is supplied by the runtime adapter.
    r0.z = r0.z;
    // 40: add r0.z, r0.z, -v7.w
    r0.z = ((r0.zzzz)+(-(v7.wwww))).z;
    // 41: add r0.w, -cb0[7].w, l(1.000000)
    r0.w = ((-(source[7].wwww))+(float4(1.000000,1.000000,1.000000,1.000000))).w;
    // 42: max r0.w, -r0.w, l(0.001000)
    r0.w = (max(-(r0.wwww),float4(0.001000,0.001000,0.001000,0.001000))).w;
    // 43: div_sat r0.z, r0.z, r0.w
    r0.z = (saturate((r0.zzzz)/(r0.wwww))).z;
    // 44: mul r0.y, r0.z, r0.y
    r0.y = ((r0.zzzz)*(r0.yyyy)).y;
    // 45: mul_sat r0.y, r0.y, v3.w
    r0.y = (saturate((r0.yyyy)*(v3.wwww))).y;
    // 46: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 47: movc o0.w, r0.x, l(0), r0.y
    output.w = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).w;
    // 48: add r0.xyz, v3.xyzx, cb0[1].xyzx
    r0.xyz = ((v3.xyzx)+(source[1].xyzx)).xyz;
    // 49: mad o0.xyz, r0.xyzx, v5.wwww, v5.xyzx
    output.xyz = ((r0.xyzx)*(v5.wwww)+(v5.xyzx)).xyz;
    return output;
}

float4 LanceVANative1354(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[4]; [unroll] for (uint i=0u; i<4u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1] = g_LanceVASourceMaterialParameters[2u];
    source[2] = g_LanceVASourceMaterialParameters[1u];
    source[3].x = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[3].y = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
    float4 v0 = float4(input.sourceBasisX,input.subUVBlend); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = float4(input.uv,input.uvNext); // native texcoord0
    float4 v3 = input.color; // native texcoord1
    float4 v4 = input.dynamicParameter; // native texcoord2
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f;
    // 1: sample_b_indexable(texture2d)(float,float,float,float) r0.xyzw, v2.zwzz, t0.xyzw, s0, l(0.000000)
    r0.xyzw = (LanceVANativeSample0((v2.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 2: sample_b_indexable(texture2d)(float,float,float,float) r1.xyzw, v2.xyxx, t0.xyzw, s0, l(0.000000)
    r1.xyzw = (LanceVANativeSample0((v2.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyzw;
    // 3: add r0.xyzw, r0.wxyz, -r1.wxyz
    r0.xyzw = ((r0.wxyz)+(-(r1.wxyz))).xyzw;
    // 4: mad r0.xyzw, v0.wwww, r0.xyzw, r1.wxyz
    r0.xyzw = ((v0.wwww)*(r0.xyzw)+(r1.wxyz)).xyzw;
    // 5: mul r1.xyz, r0.yzwy, cb0[3].xxxx
    r1.xyz = ((r0.yzwy)*(source[3].xxxx)).xyz;
    // 6: dp3 r1.w, r1.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r1.w = (dot((r1.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 7: mad r0.yzw, -cb0[3].xxxx, r0.yyzw, r1.wwww
    r0.yzw = ((-(source[3].xxxx))*(r0.yyzw)+(r1.wwww)).yzw;
    // 8: mul r0.x, r0.x, v3.w
    r0.x = ((r0.xxxx)*(v3.wwww)).x;
    // 9: mad r0.yzw, cb0[3].yyyy, r0.yyzw, r1.xxyz
    r0.yzw = ((source[3].yyyy)*(r0.yyzw)+(r1.xxyz)).yzw;
    // 10: mul r1.xyz, cb0[2].xyzx, cb0[2].wwww
    r1.xyz = ((source[2].xyzx)*(source[2].wwww)).xyz;
    // 11: mul r0.yzw, r0.yyzw, r1.xxyz
    r0.yzw = ((r0.yyzw)*(r1.xxyz)).yzw;
    // 12: mad r0.yzw, r0.yyzw, v3.xxyz, cb0[1].xxyz
    r0.yzw = ((r0.yyzw)*(v3.xxyz)+(source[1].xxyz)).yzw;
    // 13: mad o0.xyz, r0.yzwy, v5.wwww, v5.xyzx
    output.xyz = ((r0.yzwy)*(v5.wwww)+(v5.xyzx)).xyz;
    // 14: log r0.y, |r0.x|
    r0.y = (log2(abs(r0.xxxx))).y;
    // 15: lt r0.x, |r0.x|, l(0.000001)
    r0.x = (asfloat((uint4)((abs(r0.xxxx))<(float4(0.000001,0.000001,0.000001,0.000001))) * 0xffffffffu)).x;
    // 16: mul r0.y, r0.y, v4.y
    r0.y = ((r0.yyyy)*(v4.yyyy)).y;
    // 17: exp r0.y, r0.y
    r0.y = (exp2(r0.yyyy)).y;
    // 18: min r0.y, r0.y, l(1.000000)
    r0.y = (min(r0.yyyy,float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 19: mul r0.y, r0.y, cb0[0].x
    r0.y = ((r0.yyyy)*(source[0].xxxx)).y;
    // 20: movc o0.w, r0.x, l(0), r0.y
    output.w = ((asuint(r0.xxxx) != 0u) ? (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))) : (r0.yyyy)).w;
    return output;
}

float4 LanceVANative1355(LANCE_VA_NATIVE_INPUT input)
{
    float4 source[20]; [unroll] for (uint i=0u; i<20u; ++i) source[i]=0.f;
    source[0].x=1.f; // Project engine opacity multiplier.
    float4 output=0.f;
    source[1]=input.color; // Native mesh particle color prefix.
    source[2] = g_LanceVASourceMaterialParameters[9u];
    source[3] = g_LanceVASourceMaterialParameters[6u];
    source[4] = g_LanceVASourceMaterialParameters[7u];
    source[5] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[1u].yyyy,g_LanceVASourceMaterialParameters[1u].zzzz,1u);
    source[6] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[2u].zzzz,g_LanceVASourceMaterialParameters[3u].xxxx,1u);
    source[7] = LanceVANativeAppend((g_LanceVASourceMaterialParameters[2u].yyyy*g_LanceVASourceMaterialTime.xxxx),(g_LanceVASourceMaterialParameters[2u].wwww*g_LanceVASourceMaterialTime.xxxx),1u);
    source[8] = input.dynamicParameter;
    source[9] = LanceVANativeAppend(float4(0.0, 0.0, 0.0, 0.0),(g_LanceVASourceMaterialParameters[4u].wwww*g_LanceVASourceMaterialTime.xxxx),1u);
    source[10] = LanceVANativeAppend(cos((g_LanceVASourceMaterialParameters[0u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_LanceVASourceMaterialParameters[0u].zzzz*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[11] = LanceVANativeAppend(sin((g_LanceVASourceMaterialParameters[0u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),cos((g_LanceVASourceMaterialParameters[0u].zzzz*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[12] = LanceVANativeAppend(g_LanceVASourceMaterialParameters[4u].xxxx,g_LanceVASourceMaterialParameters[4u].yyyy,1u);
    source[13] = LanceVANativeAppend(cos((g_LanceVASourceMaterialParameters[3u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),(float4(-1.0, 0.0, 0.0, 0.0)*sin((g_LanceVASourceMaterialParameters[3u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))),1u);
    source[14] = LanceVANativeAppend(sin((g_LanceVASourceMaterialParameters[3u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),cos((g_LanceVASourceMaterialParameters[3u].yyyy*float4(0.25, 0.0, 0.0, 0.0))),1u);
    source[15].x = ((g_LanceVASourceMaterialParameters[2u].wwww*g_LanceVASourceMaterialTime.xxxx)).x;
    source[15].y = ((g_LanceVASourceMaterialParameters[2u].yyyy*g_LanceVASourceMaterialTime.xxxx)).x;
    source[15].z = (g_LanceVASourceMaterialParameters[2u].xxxx).x;
    source[15].w = (g_LanceVASourceMaterialParameters[1u].xxxx).x;
    source[16].x = (g_LanceVASourceMaterialParameters[4u].wwww).x;
    source[16].y = ((g_LanceVASourceMaterialParameters[4u].wwww*g_LanceVASourceMaterialTime.xxxx)).x;
    source[16].z = (g_LanceVASourceMaterialParameters[5u].yyyy).x;
    source[16].w = (g_LanceVASourceMaterialParameters[5u].xxxx).x;
    source[17].x = (g_LanceVASourceMaterialParameters[4u].zzzz).x;
    source[17].y = (g_LanceVASourceMaterialParameters[0u].xxxx).x;
    source[17].z = (g_LanceVASourceMaterialParameters[0u].yyyy).x;
    source[17].w = (g_LanceVASourceMaterialParameters[0u].wwww).x;
    source[18].x = (cos((g_LanceVASourceMaterialParameters[3u].yyyy*float4(0.25, 0.0, 0.0, 0.0)))).x;
    source[18].y = (g_LanceVASourceMaterialParameters[4u].yyyy).x;
    source[18].z = (g_LanceVASourceMaterialParameters[4u].xxxx).x;
    source[18].w = (g_LanceVASourceMaterialParameters[3u].wwww).x;
    source[19].x = (g_LanceVASourceMaterialParameters[3u].zzzz).x;
    source[19].y = (g_LanceVASourceMaterialParameters[1u].wwww).x;
    float4 passValues[5]={float4(.5f,-.5f,.5f,.5f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,0.f),float4(0.f,0.f,0.f,1.f),float4(1.f,1.f,1.f,1.f)}; // Existing SourceCharacter pass diffuse/specular override identities.
    float4 v0 = float4(input.sourceBasisX,0.f); // native texcoord10
    float4 v1 = float4(input.sourceBasisZ,input.handedness); // native texcoord11
    float4 v2 = input.vertexColor; // native color0
    float4 v3 = float4(0.f,0.f,0.f,0.f); // native color1
    float4 v4 = float4(input.uv,0.f,0.f); // native texcoord0
    float4 v5 = float4(0.f,0.f,0.f,1.f); // native texcoord4
    float4 v6 = float4(input.tangentView,1.f); // native texcoord6
    float4 v7 = float4((input.screenUV*float2(2.f,-2.f)+float2(-1.f,1.f))*input.projectionW,input.projectionZ,input.projectionW); // native texcoord5
    float4 v8 = asfloat(uint4(input.frontFace ? 0xffffffffu : 0u,0u,0u,0u)); // native sv_isfrontface0
    float4 r0=0.f, r1=0.f, r2=0.f;
    // 1: add r0.x, v4.y, cb0[9].y
    r0.x = ((v4.yyyy)+(source[9].yyyy)).x;
    // 2: mul r0.x, r0.x, cb0[16].z
    r0.x = ((r0.xxxx)*(source[16].zzzz)).x;
    // 3: mul r0.x, r0.x, l(6.283185)
    r0.x = ((r0.xxxx)*(float4(6.283185,6.283185,6.283185,6.283185))).x;
    // 4: sincos r0.x, null, r0.x
    r0.x = (sin(r0.xxxx)).x;
    // 5: mul r0.y, cb0[8].z, cb0[16].w
    r0.y = ((source[8].zzzz)*(source[16].wwww)).y;
    // 6: mul r0.x, r0.y, r0.x
    r0.x = ((r0.yyyy)*(r0.xxxx)).x;
    // 7: add r0.y, -v4.y, l(1.000000)
    r0.y = ((-(v4.yyyy))+(float4(1.000000,1.000000,1.000000,1.000000))).y;
    // 8: mul r0.y, r0.y, v4.y
    r0.y = ((r0.yyyy)*(v4.yyyy)).y;
    // 9: mul r0.x, r0.x, r0.y
    r0.x = ((r0.xxxx)*(r0.yyyy)).x;
    // 10: add r0.y, v4.x, l(-0.500000)
    r0.y = ((v4.xxxx)+(float4(-0.500000,-0.500000,-0.500000,-0.500000))).y;
    // 11: mad r0.x, r0.x, l(4.000000), r0.y
    r0.x = ((r0.xxxx)*(float4(4.000000,4.000000,4.000000,4.000000))+(r0.yyyy)).x;
    // 12: mad r0.zw, v4.xxxy, cb0[6].xxxy, cb0[7].xxxy
    r0.zw = ((v4.xxxy)*(source[6].xxxy)+(source[7].xxxy)).zw;
    // 13: sample_b_indexable(texture2d)(float,float,float,float) r0.zw, r0.zwzz, t2.zwxy, s0, l(0.000000)
    r0.zw = (LanceVANativeSample0((r0.zwzz).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).zwxy).zw;
    // 14: mad r0.zw, r0.zzzw, l(0.000000, 0.000000, 2.000000, 2.000000), l(0.000000, 0.000000, -1.000000, -1.000000)
    r0.zw = ((r0.zzzw)*(float4(0.000000,0.000000,2.000000,2.000000))+(float4(0.000000,0.000000,-1.000000,-1.000000))).zw;
    // 15: mad r1.xy, v4.xyxx, cb0[5].xyxx, l(0.000000, -1.000000, 0.000000, 0.000000)
    r1.xy = ((v4.xyxx)*(source[5].xyxx)+(float4(0.000000,-1.000000,0.000000,0.000000))).xy;
    // 16: mad r0.zw, cb0[15].zzzz, r0.zzzw, r1.xxxy
    r0.zw = ((source[15].zzzz)*(r0.zzzw)+(r1.xxxy)).zw;
    // 17: add r1.x, r0.z, cb0[15].w
    r1.x = ((r0.zzzz)+(source[15].wwww)).x;
    // 18: add r1.y, r0.w, cb0[8].x
    r1.y = ((r0.wwww)+(source[8].xxxx)).y;
    // 19: mov r0.y, v4.y
    r0.y = (v4.yyyy).y;
    // 20: mad r0.xy, cb0[17].xxxx, r0.xyxx, r1.xyxx
    r0.xy = ((source[17].xxxx)*(r0.xyxx)+(r1.xyxx)).xy;
    // 21: add r0.xy, r0.xyxx, l(-0.500000, -0.500000, 0.000000, 0.000000)
    r0.xy = ((r0.xyxx)+(float4(-0.500000,-0.500000,0.000000,0.000000))).xy;
    // 22: dp2 r1.x, cb0[10].xyxx, r0.xyxx
    r1.x = (dot((source[10].xyxx).xy,(r0.xyxx).xy).xxxx).x;
    // 23: dp2 r1.y, cb0[11].xyxx, r0.xyxx
    r1.y = (dot((source[11].xyxx).xy,(r0.xyxx).xy).xxxx).y;
    // 24: add r0.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r0.xy = ((r1.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 25: sample_b_indexable(texture2d)(float,float,float,float) r0.xyz, r0.xyxx, t3.xyzw, s1, l(0.000000)
    r0.xyz = (LanceVANativeSample1((r0.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).xyz;
    // 26: dp3 r0.w, r0.xyzx, l(0.300000, 0.590000, 0.110000, 0.000000)
    r0.w = (dot((r0.xyzx).xyz,(float4(0.300000,0.590000,0.110000,0.000000)).xyz).xxxx).w;
    // 27: add r1.xyz, -r0.xyzx, r0.wwww
    r1.xyz = ((-(r0.xyzx))+(r0.wwww)).xyz;
    // 28: mad r0.xyz, cb0[17].yyyy, r1.xyzx, r0.xyzx
    r0.xyz = ((source[17].yyyy)*(r1.xyzx)+(r0.xyzx)).xyz;
    // 29: max r0.xyz, |r0.xyzx|, l(0.000001, 0.000001, 0.000001, 0.000000)
    r0.xyz = (max(abs(r0.xyzx),float4(0.000001,0.000001,0.000001,0.000000))).xyz;
    // 30: log r0.xyz, r0.xyzx
    r0.xyz = (log2(r0.xyzx)).xyz;
    // 31: mul r0.xyz, r0.xyzx, cb0[17].zzzz
    r0.xyz = ((r0.xyzx)*(source[17].zzzz)).xyz;
    // 32: exp r0.xyz, r0.xyzx
    r0.xyz = (exp2(r0.xyzx)).xyz;
    // 33: mul r0.xyz, r0.xyzx, cb0[17].wwww
    r0.xyz = ((r0.xyzx)*(source[17].wwww)).xyz;
    // 34: add r1.xyz, -cb0[3].xyzx, cb0[4].xyzx
    r1.xyz = ((-(source[3].xyzx))+(source[4].xyzx)).xyz;
    // 35: mad r1.xyz, v4.yyyy, r1.xyzx, cb0[3].xyzx
    r1.xyz = ((v4.yyyy)*(r1.xyzx)+(source[3].xyzx)).xyz;
    // 36: mul r0.xyz, r0.xyzx, r1.xyzx
    r0.xyz = ((r0.xyzx)*(r1.xyzx)).xyz;
    // 37: mad r0.xyz, r0.xyzx, cb0[1].xyzx, cb0[2].xyzx
    r0.xyz = ((r0.xyzx)*(source[1].xyzx)+(source[2].xyzx)).xyz;
    // 38: mul r0.xyz, r0.xyzx, v5.wwww
    r0.xyz = ((r0.xyzx)*(v5.wwww)).xyz;
    // 39: mul r0.w, v4.x, cb0[12].x
    r0.w = ((v4.xxxx)*(source[12].xxxx)).w;
    // 40: add r1.x, r0.w, cb0[18].w
    r1.x = ((r0.wwww)+(source[18].wwww)).x;
    // 41: mad r1.z, v4.y, cb0[12].y, cb0[8].y
    r1.z = ((v4.yyyy)*(source[12].yyyy)+(source[8].yyyy)).z;
    // 42: add r1.xy, r1.xzxx, l(-0.500000, -1.500000, 0.000000, 0.000000)
    r1.xy = ((r1.xzxx)+(float4(-0.500000,-1.500000,0.000000,0.000000))).xy;
    // 43: dp2 r2.x, cb0[13].xyxx, r1.xyxx
    r2.x = (dot((source[13].xyxx).xy,(r1.xyxx).xy).xxxx).x;
    // 44: dp2 r2.y, cb0[14].xyxx, r1.xyxx
    r2.y = (dot((source[14].xyxx).xy,(r1.xyxx).xy).xxxx).y;
    // 45: add r1.xy, r2.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
    r1.xy = ((r2.xyxx)+(float4(0.500000,0.500000,0.000000,0.000000))).xy;
    // 46: sample_b_indexable(texture2d)(float,float,float,float) r0.w, r1.xyxx, t1.yzwx, s3, l(0.000000)
    r0.w = (LanceVANativeSample3((r1.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).yzwx).w;
    // 47: sample_b_indexable(texture2d)(float,float,float,float) r1.x, v4.xyxx, t0.xyzw, s2, l(0.000000)
    r1.x = (LanceVANativeSample2((v4.xyxx).xy, (float4(0.000000,0.000000,0.000000,0.000000)).x, false).xyzw).x;
    // 48: mul r0.w, r0.w, r1.x
    r0.w = ((r0.wwww)*(r1.xxxx)).w;
    // 49: mul_sat r0.w, r0.w, cb0[19].x
    r0.w = (saturate((r0.wwww)*(source[19].xxxx))).w;
    // 50: mul r0.w, r0.w, cb0[1].w
    r0.w = ((r0.wwww)*(source[1].wwww)).w;
    // 51: mul r0.w, r0.w, cb0[0].x
    r0.w = ((r0.wwww)*(source[0].xxxx)).w;
    // 52: mul o0.xyz, r0.wwww, r0.xyzx
    output.xyz = ((r0.wwww)*(r0.xyzx)).xyz;
    // 53: mov o0.w, l(0)
    output.w = (float4(asfloat(0u),asfloat(0u),asfloat(0u),asfloat(0u))).w;
    return output;
}
#endif
