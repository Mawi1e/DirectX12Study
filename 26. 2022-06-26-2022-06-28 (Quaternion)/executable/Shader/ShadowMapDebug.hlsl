#define MaxLights 16

struct Light
{
    float3 Strength;
    float FalloffStart; // Á¡±¤/Á¡Àû±¤
    float3 Direction;   // ÆòÇà±¤/Á¡Àû±¤
    float FalloffEnd;   // Á¡±¤/Á¡Àû±¤
    float3 Position;    // Á¡±¤
    float SpotPower;    // Á¡Àû±¤
};

struct MaterialBuffer
{
    float4 DiffuseAlbedo;
    float3 FresnelR0;
    float  Roughness;
    float4x4 MatTransform;

    int DiffuseMapIndex;
    int NormalSrvHeapIndex;
    uint pad0;
    uint pad1;
};

TextureCube gCubeMap : register(t0);
Texture2D gShadowMap : register(t1);

StructuredBuffer<MaterialBuffer> gMaterialBuffer : register(t0, space1);

Texture2D gTextures[8] : register(t2);

SamplerState gsamPointWrap        : register(s0);
SamplerState gsamPointClamp       : register(s1);
SamplerState gsamLinearWrap       : register(s2);
SamplerState gsamLinearClamp      : register(s3);
SamplerState gsamAnisotropicWrap  : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);
SamplerComparisonState gsamShadow : register(s6);


cbuffer cbObject : register(b0)
{
    float4x4 gWorld;
    float4x4 gTexTransform;

    uint gMaterials;
    uint matPad1;
    uint matPad2;
    uint matPad3;
};

cbuffer cbPass : register(b1)
{
    float4x4 gView;
    float4x4 gInvView;
    float4x4 gProj;
    float4x4 gInvProj;
    float4x4 gViewProj;
    float4x4 gInvViewProj;
    float4x4 gShadowTransform;
    float3 gEyePosW;
    float cbPerObjectPad1;
    float2 gRenderTargetSize;
    float2 gInvRenderTargetSize;
    float gNearZ;
    float gFarZ;
    float gTotalTime;
    float gDeltaTime;
    float4 gAmbientLight;
    float4 gFogColor;
    float gFogStart;
    float gFogRange;
    float2 cbPerObjectPad2;

    Light gLights[MaxLights];
};

struct VertexIn
{
    float3 PosL    : POSITION;
    float2 TexC : TEXCOORD;
};

struct VertexOut
{
    float4 PosH    : SV_POSITION;
    float2 TexC : TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout = (VertexOut)0.0f;

    vout.PosH = float4(vin.PosL, 1.0f);
    vout.TexC = vin.TexC;

    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    return float4(gShadowMap.Sample(gsamLinearWrap, pin.TexC).rrr, 1.0f);
}