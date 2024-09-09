#pragma pack_matrix(column_major)


#define rs_controller \
RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), \
RootConstants(num32BitConstants=48, b0, space=0), \
CBV(b1)
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

cbuffer u_CompleteMVP : register(b1)
{
    CompleteMVP m_CompleteMVP;
};

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
    return float4(psInput.txc, 0.0f, 1.0f);
}
