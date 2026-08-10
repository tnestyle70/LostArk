// Artist 31470 F RECONSTRUCTED_APPROVED_V1 row oracle.
// The input ABI is exactly 128 bytes.  This shader verifies the typed policy
// projection only; it does not promote any selected value to SOURCE_EXACT.

struct PolicyInput
{
    uint4 Header;  // policy kind, field code, decision code, reserved
    float4 Value0;
    float4 Padding0;
    float4 Padding1;
    float4 Padding2;
    float4 Padding3;
    float4 Padding4;
    float4 Padding5;
};

StructuredBuffer<PolicyInput> PolicyInputs : register(t0);
RWStructuredBuffer<float4> PolicyOutputs : register(u0);

[numthreads(64, 1, 1)]
void main(uint3 dispatchThreadId : SV_DispatchThreadID)
{
    uint index = dispatchThreadId.x;
    uint count = 0;
    uint stride = 0;
    PolicyInputs.GetDimensions(count, stride);
    if (index >= count)
        return;

    PolicyInput input = PolicyInputs[index];
    uint kind = input.Header.x;
    uint fieldCode = input.Header.y;
    uint decisionCode = input.Header.z;
    float selected = input.Value0.x;
    float4 outputValue = float4(0.0, 0.0, 0.0, 0.0);

    if (kind == 1) // render state
    {
        if (fieldCode == 1) // bDisableDepthTest -> DepthEnable
            outputValue = float4(selected, 1.0 - selected, (float)fieldCode, 1.0);
        else if (fieldCode == 2) // bUseOneLayerDistortion
            outputValue = float4(selected, selected, (float)fieldCode, 1.0);
        else if (fieldCode == 3) // OpacityMaskClipValue
            outputValue = float4(selected, selected >= 0.0 ? 1.0 : 0.0, (float)fieldCode, 1.0);
        else if (fieldCode == 4) // TwoSided -> CullMode
            outputValue = float4(selected, selected != 0.0 ? 1.0 : 3.0, (float)fieldCode, 1.0);
        else if (fieldCode == 5) // LightingModel enum
            outputValue = float4(selected, 0.0, (float)fieldCode, 1.0);
    }
    else if (kind == 2) // static permutation
    {
        outputValue = float4(selected, 1.0 - selected, (float)decisionCode, 1.0);
    }
    else if (kind == 3) // sampler descriptor projection
    {
        outputValue = float4(
            (float)input.Header.y,
            (float)input.Header.z,
            (float)input.Header.w,
            selected);
    }

    PolicyOutputs[index] = outputValue;
}
