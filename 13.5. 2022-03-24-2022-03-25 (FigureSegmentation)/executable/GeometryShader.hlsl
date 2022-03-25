#ifndef NUM_DIR_LIGHTS
#define NUM_DIR_LIGHTS 3
#endif

#ifndef NUM_POINT_LIGHTS
#define NUM_POINT_LIGHTS 0
#endif

#ifndef NUM_SPOT_LIGHTS
#define NUM_SPOT_LIGHTS 0
#endif

#define MaxLights 16

struct Light
{
    float3 Strength;
    float FalloffStart; // ����/������
    float3 Direction;   // ���౤/������
    float FalloffEnd;   // ����/������
    float3 Position;    // ����
    float SpotPower;    // ������
};

struct Material
{
    float4 DiffuseAlbedo; // �л�ݻ���: ���� ������ ����Ҷ� ��� ���� ����ǰ�, ��� ���� �л�Ǿ� ����������, �̶� ���� �л�ݻ�Ǵ� ũ��
    float3 FresnelR0; // �����ڹ����Ŀ��� ����ϴ� �����ٻ翡���� R0: ���������� ������
    float Shininess; // (1 - Roughness)�̸� ��ĥ���� �ݴ�: [0, 1]�̸� 0�� �������� ��ĥ��, 1�� �������� �Ų��ϴ�
};

float CalcAttenuation(float d, float falloffStart, float falloffEnd)
{
    // ���� �����Լ� (falloff End - d / falloffEnd - falloffStart)
    return saturate((falloffEnd - d) / (falloffEnd - falloffStart));
}

// �����ڹ����Ŀ��� ����ϴ� �����ٻ�(Schlick approximation)
float3 SchlickFresnel(float3 R0, float3 normal, float3 lightVec)
{
    // �����ٻ�: R(r) = R(0) x {(1 - R(0)) * (cos(O)^5)}
    // ������Ʈ �ڻ��ι�Ģ: cos(O) = max(L (dot) n, 0)
    // L: lightVec
    // n: normal


    float cosIncidentAngle = saturate(dot(normal, lightVec));

    float f0 = 1.0f - cosIncidentAngle;
    float3 reflectPercent = R0 + (1.0f - R0) * (f0 * f0 * f0 * f0 * f0);

    return reflectPercent;
}

float3 BlinnPhong(float3 lightStrength, float3 lightVec, float3 normal, float3 toEye, Material mat)
{
    const float m = mat.Shininess * 256.0f;
    float3 halfVec = normalize(toEye + lightVec);

    float roughnessFactor = (m + 8.0f) * pow(max(dot(halfVec, normal), 0.0f), m) / 8.0f;
    float3 fresnelFactor = SchlickFresnel(mat.FresnelR0, halfVec, lightVec);

    float3 specAlbedo = fresnelFactor * roughnessFactor;

    // LDR�������� ����Ұ��̱� ������ �ݿ��ݻ����� [0, 1]���̷� ����
    specAlbedo = specAlbedo / (specAlbedo + 1.0f);

    return (mat.DiffuseAlbedo.rgb + specAlbedo) * lightStrength;
}

// ���౤ ���
float3 ComputeDirectionalLight(Light L, Material mat, float3 normal, float3 toEye)
{
    // ���� ���⿡ -1�� ���ϸ� �����Ͱ� �ȴ�
    float3 lightVec = -L.Direction;

    // ������Ʈ �ڻ��ι�Ģ
    float ndotl = max(dot(lightVec, normal), 0.0f);
    float3 lightStrength = L.Strength * ndotl;

    return BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
}

// ���� ���
float3 ComputePointLight(Light L, Material mat, float3 pos, float3 normal, float3 toEye)
{
    // ��x��������� ��ġ�������� ���� ��ġ�������� ���� ����
    float3 lightVec = L.Position - pos;

    // �������� ����
    float d = length(lightVec);

    // �´��� Ȯ��
    if (d > L.FalloffEnd)
        return 0.0f;

    // ������ �Ϲ�ȭ
    lightVec /= d;

    // ������Ʈ �ڻ��ι�Ģ
    float ndotl = max(dot(lightVec, normal), 0.0f);
    float3 lightStrength = L.Strength * ndotl;

    // ���Ǽ��⿡ ���������Լ��� ������� ���Ѵ�.
    float att = CalcAttenuation(d, L.FalloffStart, L.FalloffEnd);
    lightStrength *= att;

    return BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
}

// ������ ���
float3 ComputeSpotLight(Light L, Material mat, float3 pos, float3 normal, float3 toEye)
{
    // ��x��������� ��ġ�������� ���� ��ġ�������� ���� ����
    float3 lightVec = L.Position - pos;

    // �������� ����
    float d = length(lightVec);

    // �´��� Ȯ��
    if (d > L.FalloffEnd)
        return 0.0f;

    // ������ �Ϲ�ȭ
    lightVec /= d;

    // ������Ʈ �ڻ��ι�Ģ
    float ndotl = max(dot(lightVec, normal), 0.0f);
    float3 lightStrength = L.Strength * ndotl;

    // ���Ǽ��⿡ ���������Լ��� ������� ���Ѵ�.
    float att = CalcAttenuation(d, L.FalloffStart, L.FalloffEnd);
    lightStrength *= att;

    // ���Ǽ��⿡ max(-L (dot) d, 0)^s: s(L.SpotPower)�� ���������ν� ������������ ũ�⸦ ������ �� �ִ�.
    float spotFactor = pow(max(dot(-lightVec, L.Direction), 0.0f), L.SpotPower);
    lightStrength *= spotFactor;

    return BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
}

float4 ComputeLighting(Light gLights[MaxLights], Material mat,
    float3 pos, float3 normal, float3 toEye,
    float3 shadowFactor)
{
    float3 result = 0.0f;

    int i = 0;

#if (NUM_DIR_LIGHTS > 0)
    for (i = 0; i < NUM_DIR_LIGHTS; ++i)
    {
        result += shadowFactor[i] * ComputeDirectionalLight(gLights[i], mat, normal, toEye);
    }
#endif

#if (NUM_POINT_LIGHTS > 0)
    for (i = NUM_DIR_LIGHTS; i < NUM_DIR_LIGHTS + NUM_POINT_LIGHTS; ++i)
    {
        result += ComputePointLight(gLights[i], mat, pos, normal, toEye);
    }
#endif

#if (NUM_SPOT_LIGHTS > 0)
    for (i = NUM_DIR_LIGHTS + NUM_POINT_LIGHTS; i < NUM_DIR_LIGHTS + NUM_POINT_LIGHTS + NUM_SPOT_LIGHTS; ++i)
    {
        result += ComputeSpotLight(gLights[i], mat, pos, normal, toEye);
    }
#endif 

    return float4(result, 0.0f);
}

void Shader_Cartoon(inout float4 dLight) {
    dLight = saturate(dLight);
    dLight = ceil(dLight * 5.0f) / 5.0f;
}


Texture2D gTextures : register(t0);

SamplerState gsamPointWrap        : register(s0);
SamplerState gsamPointClamp       : register(s1);
SamplerState gsamLinearWrap       : register(s2);
SamplerState gsamLinearClamp      : register(s3);
SamplerState gsamAnisotropicWrap  : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);

cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
    float4x4 gTexTransform;
    float4x4 gWorldInvTranspose;
};

cbuffer cbPass : register(b1)
{
    float4x4 gView;
    float4x4 gInvView;
    float4x4 gProj;
    float4x4 gInvProj;
    float4x4 gViewProj;
    float4x4 gInvViewProj;
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

cbuffer cbMaterial : register(b2)
{
    float4 gDiffuseAlbedo;
    float3 gFresnelR0;
    float  gRoughness;
    float4x4 gMatTransform;
};

struct VertexIn
{
    float3 PosW     : POSITION;
    float3 NormalW  : NORMAL;
    float2 TexC     : TEXCOORD;
};

struct VertexOut
{
    float3 PosL     : POSITION;
    float3 NormalW  : NORMAL;
    float2 TexC     : TEXCOORD;
};

struct GeoOut
{
    float4 PosH     : SV_POSITION;
    float3 PosW     : POSITION;
    float3 NormalW  : NORMAL;
    float2 TexC     : TEXCOORD;
};

VertexOut VS(VertexIn vin) {
    VertexOut vout;

    vout.PosL = vin.PosW;
    vout.NormalW = vin.NormalW;
    vout.TexC = vin.TexC;

    return vout;
}


void Subdivision1_1(VertexOut vin[3], out VertexOut vout[6]) {
    VertexOut m[3];

    m[0].PosL = 0.5f * (vin[0].PosL + vin[1].PosL);
    m[1].PosL = 0.5f * (vin[1].PosL + vin[2].PosL);
    m[2].PosL = 0.5f * (vin[2].PosL + vin[0].PosL);

    m[0].NormalW = normalize(0.5f * (m[0].PosL + m[1].PosL));
    m[1].NormalW = normalize(0.5f * (m[1].PosL + m[2].PosL));
    m[2].NormalW = normalize(0.5f * (m[2].PosL + m[0].PosL));

    m[0].TexC = 0.5f * (vin[0].TexC + vin[1].TexC);
    m[1].TexC = 0.5f * (vin[1].TexC + vin[2].TexC);
    m[2].TexC = 0.5f * (vin[2].TexC + vin[0].TexC);

    vout[0] = vin[0];
    vout[1] = m[0];
    vout[2] = m[2];
    vout[3] = m[1];
    vout[4] = vin[2];
    vout[5] = vin[1];
}

void Subdivision1_2(VertexOut vin[6], inout TriangleStream<GeoOut> geoStream) {
    GeoOut geoOut[6];

    [unroll]
    for (int i = 0; i < 6; ++i) {
        float4 posW = mul(float4(vin[i].PosL, 1.0f), gWorld);
        geoOut[i].PosW = posW.xyz;
        geoOut[i].NormalW = mul(vin[i].NormalW, (float3x3)gWorldInvTranspose);
        geoOut[i].PosH = mul(posW, gViewProj);
        geoOut[i].TexC = vin[i].TexC;
    }

    [unroll]
    for (int i = 0; i < 5; ++i) {
        geoStream.Append(geoOut[i]);
    }

    geoStream.RestartStrip();

    geoStream.Append(geoOut[1]);
    geoStream.Append(geoOut[5]);
    geoStream.Append(geoOut[3]);
}

void Subdivision2_1(VertexOut vin[3], inout TriangleStream<GeoOut> geoStream) {
    GeoOut geoOut[3];

    [unroll]
    for (int i = 0; i < 3; ++i) {
        float4 posW = mul(float4(vin[i].PosL, 1.0f), gWorld);
        geoOut[i].PosW = posW.xyz;
        geoOut[i].NormalW = mul(vin[i].NormalW, (float3x3)gWorldInvTranspose);
        geoOut[i].PosH = mul(posW, gViewProj);
        geoOut[i].TexC = vin[i].TexC;
    }

    [unroll]
    for (int i = 0; i < 3; ++i) {
        geoStream.Append(geoOut[i]);
    }

    geoStream.RestartStrip();
}

/*
void Subdivision1(VertexOut vin[3], out VertexOut vout[6]) {
    VertexOut m[3];

    m[0].PosL = 0.5f * (vin[0].PosL + vin[1].PosL);
    m[1].PosL = 0.5f * (vin[1].PosL + vin[2].PosL);
    m[2].PosL = 0.5f * (vin[2].PosL + vin[0].PosL);

    m[0].PosL = normalize(m[0].PosL);
    m[1].PosL = normalize(m[1].PosL);
    m[2].PosL = normalize(m[2].PosL);

    m[0].NormalW = m[0].PosL;
    m[1].NormalW = m[1].PosL;
    m[2].NormalW = m[2].PosL;

    m[0].TexC = 0.5f * (vin[0].TexC + vin[1].TexC);
    m[1].TexC = 0.5f * (vin[1].TexC + vin[2].TexC);
    m[2].TexC = 0.5f * (vin[2].TexC + vin[0].TexC);

    vout[0] = vin[0];
    vout[1] = m[0];
    vout[2] = m[2];
    vout[3] = m[1];
    vout[4] = vin[2];
    vout[5] = vin[1];
}

void Subdivision2(VertexOut vin[6], inout TriangleStream<GeoOut> geoStream) {
    GeoOut geoOut[6];

    [unroll]
    for (int i = 0; i < 6; ++i) {
        float4 posW = mul(float4(vin[i].PosL, 1.0f), gWorld);
        geoOut[i].PosW = posW.xyz;
        geoOut[i].NormalW = mul(vin[i].NormalW, (float3x3)gWorldInvTranspose);
        geoOut[i].PosH = mul(posW, gViewProj);
        geoOut[i].TexC = vin[i].TexC;
    }

    [unroll]
    for (int i = 0; i < 5; ++i) {
        geoStream.Append(geoOut[i]);
    }

    geoStream.RestartStrip();

    geoStream.Append(geoOut[1]);
    geoStream.Append(geoOut[5]);
    geoStream.Append(geoOut[3]);
}
*/

[maxvertexcount(8)]
void GS(triangle VertexOut vin[3], inout TriangleStream<GeoOut> geoStream) {
    VertexOut v[6];

    float3 tmp_posL = 0.5f * (vin[0].PosL + vin[2].PosL);
    float3 posL = 0.5f * (tmp_posL + vin[1].PosL);
    float3 posW = mul(float4(posL, 1.0f), gWorld).xyz;
    float d = length(posW - gEyePosW);

    if (d < 45.0f) {
        Subdivision1_1(vin, v);
        Subdivision1_2(v, geoStream);
    }
    else {
        Subdivision2_1(vin, geoStream);
    }
}

float4 PS(GeoOut vin) : SV_Target {
    float4 diffuseAlbedo = gTextures.Sample(gsamAnisotropicWrap, vin.TexC) * gDiffuseAlbedo;
    float4 ambient = gAmbientLight * diffuseAlbedo;

#ifdef ALPHA_TEST
    clip(diffuseAlbedo.a - 0.1f);
#endif

    vin.NormalW = normalize(vin.NormalW);

    float3 toEyePosW = gEyePosW - vin.PosW;
    float distToEye = length(toEyePosW);
    toEyePosW /= distToEye;
    
    float3 shadowFactor = 1.0f;
    const float shiness = 1.0f - gRoughness;
    Material mat = { diffuseAlbedo, gFresnelR0, shiness };
    float4 directionalLight = ComputeLighting(gLights, mat, vin.PosW, vin.NormalW, toEyePosW, shadowFactor);

    //Shader_Cartoon(directionalLight);

    float4 litColor = ambient + directionalLight;

#ifdef FOG
    float fogAmount = saturate((distToEye - gFogStart) / gFogRange);
    litColor = lerp(litColor, gFogColor, fogAmount);
#endif

    litColor.a = gDiffuseAlbedo.a;

    return litColor;
}