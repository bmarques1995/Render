#pragma pack_matrix(column_major)

#define rs_controller \
RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | DENY_GEOMETRY_SHADER_ROOT_ACCESS), \
RootConstants(num32BitConstants=48, b0), \
CBV(b1), \
DescriptorTable(SRV(t1, numDescriptors = 1)), \
DescriptorTable(Sampler(s1, numDescriptors = 1)), \
//StaticSampler( s2,\
//                filter = FILTER_ANISOTROPIC,\
//                addressU = TEXTURE_ADDRESS_WRAP,\
//                addressV = TEXTURE_ADDRESS_WRAP,\
//                addressW = TEXTURE_ADDRESS_WRAP,\
//                mipLODBias = 0.f,\
//                maxAnisotropy = 16,\
//                comparisonFunc = COMPARISON_LESS_EQUAL,\
//                borderColor = STATIC_BORDER_COLOR_OPAQUE_WHITE,\
//                minLOD = 0.f,\
//                maxLOD = 3.402823466e+38f,\
//                space = 0,\
//                visibility = SHADER_VISIBILITY_ALL), \
//DescriptorTable(CBV(b1))

struct SmallMVP
{
    float4x4 M;
    float4x4 V;
    float4x4 P;
};

struct CompleteMVP
{
    float4x4 M;
    float4x4 V;
    float4x4 P;
    float4x4 F;
    
};

#ifdef VK_HLSL

[[vk::push_constant]] SmallMVP m_SmallMVP;

#else

cbuffer u_SmallMVP : register(b0)
{
    SmallMVP m_SmallMVP;
};

#endif

[[vk::binding(1, 0)]] cbuffer u_CompleteMVP : register(b1)
{
    CompleteMVP m_CompleteMVP;
};

[[vk::binding(2, 0)]] Texture2D textureChecker : register(t1);
[[vk::binding(2, 0)]] SamplerState dynamicSampler : register(s1);

struct VSInput
{
    [[vk::location(0)]]float3 pos : POSITION;
    [[vk::location(1)]]float4 col : COLOR;
    [[vk::location(2)]]float2 txc : TEXCOORD;
};

struct PSInput
{
    float4 pos : SV_POSITION;
    float4 col : COLOR;
    float2 txc : TEXCOORD;
};

PSInput vs_main(VSInput vsInput)
{
    bool useComplete = true;
    PSInput vsoutput;
    if (useComplete)
    {
        vsoutput.pos = mul(float4(vsInput.pos, 1.0f), m_CompleteMVP.M);
        vsoutput.pos = mul(vsoutput.pos, m_CompleteMVP.V);
        vsoutput.pos = mul(vsoutput.pos, m_CompleteMVP.P);
    }
    else
    {
        vsoutput.pos = mul(float4(vsInput.pos, 1.0f), m_SmallMVP.M);
        vsoutput.pos = mul(vsoutput.pos, m_SmallMVP.V);
        vsoutput.pos = mul(vsoutput.pos, m_SmallMVP.P);
    }
    vsoutput.col = vsInput.col;
    vsoutput.txc = vsInput.txc;
    return vsoutput;
}

float4 ps_main(PSInput psInput) : SV_TARGET0
{
    float4 pixel = textureChecker.SampleLevel(dynamicSampler, psInput.txc, 0.0f);
    //float4 pixel = float4(1.0f);
    return float4(psInput.col.xyzw * pixel);
}