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
    float FalloffStart; // 점광/점적광
    float3 Direction;   // 평행광/점적광
    float FalloffEnd;   // 점광/점적광
    float3 Position;    // 점광
    float SpotPower;    // 점적광
};

struct Material
{
    float4 DiffuseAlbedo; // 분산반사율: 빛이 매질을 통과할때 어떠한 빛은 흡수되고, 어떠한 빛은 분산되어 방출하지만, 이때 빛이 분산반사되는 크기
    float3 FresnelR0; // 프레넬방정식에서 사용하는 슐릭근사에서의 R0: 매질마다의 굴절률
    float Shininess; // (1 - Roughness)이며 거칠기의 반대: [0, 1]이며 0에 가까울수록 거칠고, 1에 가까울수록 매끈하다
};

float CalcAttenuation(float d, float falloffStart, float falloffEnd)
{
    // 선형 감쇠함수 (falloff End - d / falloffEnd - falloffStart)
    return saturate((falloffEnd - d) / (falloffEnd - falloffStart));
}

// 프레넬방정식에서 사용하는 슐릭근사(Schlick approximation)
float3 SchlickFresnel(float3 R0, float3 normal, float3 lightVec)
{
    // 슐릭근사: R(r) = R(0) x {(1 - R(0)) * (cos(O)^5)}
    // 람베르트 코사인법칙: cos(O) = max(L (dot) n, 0)
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

    // LDR렌더링을 사용할것이기 때문에 반영반사율을 [0, 1]사이로 매핑
    specAlbedo = specAlbedo / (specAlbedo + 1.0f);

    return (mat.DiffuseAlbedo.rgb + specAlbedo) * lightStrength;
}

// 평행광 계산
float3 ComputeDirectionalLight(Light L, Material mat, float3 normal, float3 toEye)
{
    // 빛의 방향에 -1을 곱하면 빛벡터가 된다
    float3 lightVec = -L.Direction;

    // 람베르트 코사인법칙
    float ndotl = max(dot(lightVec, normal), 0.0f);
    float3 lightStrength = L.Strength * ndotl;

    return BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
}

// 점광 계산
float3 ComputePointLight(Light L, Material mat, float3 pos, float3 normal, float3 toEye)
{
    // 뷰x투영행렬의 위치에서에서 빛의 위치방향으로 가는 벡터
    float3 lightVec = L.Position - pos;

    // 빛벡터의 길이
    float d = length(lightVec);

    // 맞는지 확인
    if (d > L.FalloffEnd)
        return 0.0f;

    // 빛벡터 일반화
    lightVec /= d;

    // 람베르트 코사인법칙
    float ndotl = max(dot(lightVec, normal), 0.0f);
    float3 lightStrength = L.Strength * ndotl;

    // 빛의세기에 선형감쇠함수의 결과값을 곱한다.
    float att = CalcAttenuation(d, L.FalloffStart, L.FalloffEnd);
    lightStrength *= att;

    return BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
}

// 점적광 계산
float3 ComputeSpotLight(Light L, Material mat, float3 pos, float3 normal, float3 toEye)
{
    // 뷰x투영행렬의 위치에서에서 빛의 위치방향으로 가는 벡터
    float3 lightVec = L.Position - pos;

    // 빛벡터의 길이
    float d = length(lightVec);

    // 맞는지 확인
    if (d > L.FalloffEnd)
        return 0.0f;

    // 빛벡터 일반화
    lightVec /= d;

    // 람베르트 코사인법칙
    float ndotl = max(dot(lightVec, normal), 0.0f);
    float3 lightStrength = L.Strength * ndotl;

    // 빛의세기에 선형감쇠함수의 결과값을 곱한다.
    float att = CalcAttenuation(d, L.FalloffStart, L.FalloffEnd);
    lightStrength *= att;

    // 빛의세기에 max(-L (dot) d, 0)^s: s(L.SpotPower)를 조율함으로써 점적광원뿔의 크기를 변경할 수 있다.
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