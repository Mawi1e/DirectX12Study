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


struct MaterialBuffer
{
    float4 DiffuseAlbedo;
    float3 FresnelR0;
    float  Roughness;
    float4x4 MatTransform;

    uint DiffuseMapIndex;
    uint pad0;
    uint pad1;
    uint pad2;
};

Texture2D gTextures[7] : register(t0);
StructuredBuffer<MaterialBuffer> gMaterialBuffer : register(t0, space1);

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

    uint gMaterialIndex;
    uint MatPad0;
    uint MatPad1;
    uint MatPad2;
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
 
struct VertexIn
{
	float3 PosL    : POSITION;
    float3 NormalL : NORMAL;
    float2 TexC : TEXCOORD;
};

struct VertexOut
{
	float4 PosH    : SV_POSITION;
    float3 PosW    : POSITION;
    float3 NormalW : NORMAL;
    float2 TexC : TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout = (VertexOut)0.0f;

    MaterialBuffer matBuffer = gMaterialBuffer[gMaterialIndex];
	
    // 월드 행렬로 변환
    float4 posW = mul(float4(vin.PosL, 1.0f), gWorld);
    vout.PosW = posW.xyz;

    // if1. 세계행렬에 비균등비례가 있다면, 역전치 월드행렬을 사용해야한다.
    // if2. 세계행렬에 비균등비례가 없다면, 월드행렬을 이용해 법선을 변환한다.
    vout.NormalW = mul(vin.NormalL, (float3x3)gWorld);

    // 동차절단공간으로 변환
    vout.PosH = mul(posW, gViewProj);

    float4 texTrans = mul(float4(vin.TexC, 0.0f, 1.0f), gTexTransform);
    vout.TexC = mul(texTrans, matBuffer.MatTransform).xy;

    return vout;
}

float4 toonShading(float4 diffuse) {
    float4 color = saturate(diffuse);
    color = ceil(color * 5) / 5.0f;
    color.a = diffuse.a;

    return color;
}

float4 toonShading_kd(float4 kd) {
    float4 color = saturate(kd);

    if (color.x <= 0.0f) {
        color.x = 0.4f;
    }
    else if (0.0f < color.x && color.x <= 0.5f) {
        color.x = 0.6f;
    }
    else if (0.5f < color.x && color.x <= 1.0f) {
        color.x = 1.0f;
    }

    if (color.y <= 0.0f) {
        color.y = 0.4f;
    }
    else if (0.0f < color.y && color.y <= 0.5f) {
        color.g = 0.6f;
    }
    else if (0.5f < color.y && color.y <= 1.0f) {
        color.y = 1.0f;
    }

    if (color.z <= 0.0f) {
        color.z = 0.4f;
    }
    else if (0.0f < color.z && color.z <= 0.5f) {
        color.z = 0.6f;
    }
    else if (0.5f < color.z && color.z <= 1.0f) {
        color.z = 1.0f;
    }

    color.a = saturate(kd).a;

    return color;
}

float4 toonShading_ks(float4 ks) {
    float4 color = saturate(ks);

    if (0.0f < color.x && color.x <= 0.1f) {
        color.x = 0.0f;
    }
    else if (0.1f < color.x && color.x <= 0.8f) {
        color.x = 0.5f;
    }
    else if (0.8f < color.x && color.x <= 1.0f) {
        color.x = 0.8f;
    }

    if (0.0f < color.y && color.y <= 0.1f) {
        color.y = 0.0f;
    }
    else if (0.1f < color.y && color.y <= 0.8f) {
        color.y = 0.5f;
    }
    else if (0.8f < color.y && color.y <= 1.0f) {
        color.y = 0.8f;
    }

    if (0.0f < color.z && color.z <= 0.1f) {
        color.z = 0.0f;
    }
    else if (0.1f < color.z && color.z <= 0.8f) {
        color.z = 0.5f;
    }
    else if (0.8f < color.z && color.z <= 1.0f) {
        color.z = 0.8f;
    }

    color.a = saturate(ks).a;

    return color;
}

float4 PS(VertexOut pin) : SV_Target
{
    MaterialBuffer matBuffer = gMaterialBuffer[gMaterialIndex];
    float4 diffuseAlbedo = matBuffer.DiffuseAlbedo;
    float3 FresnelR0 = matBuffer.FresnelR0;
    float Roughness = matBuffer.Roughness;
    uint diffuseMapIndex = matBuffer.DiffuseMapIndex;

    diffuseAlbedo = gTextures[diffuseMapIndex].Sample(gsamLinearWrap, pin.TexC) * diffuseAlbedo;

#ifdef ALPHA_TEST
    clip(diffuseAlbedo.a - 0.1f);
#endif

    // 법선을 보간하여, 단위길이가 아닐 수 있기때문에 정규화한다.
    pin.NormalW = normalize(pin.NormalW);

    // 조명되는 점에서 눈으로의 벡터
    float3 toEyeW = gEyePosW - pin.PosW;
    float distToEye = length(toEyeW);
    toEyeW /= distToEye;
    
	// 주변광계산
    float4 ambient = gAmbientLight * diffuseAlbedo;

    // 평행광계산
    const float shininess = 1.0f - Roughness;
    Material mat = { diffuseAlbedo, FresnelR0, shininess };
    float3 shadowFactor = 1.0f;
    float4 directLight = ComputeLighting(gLights, mat, pin.PosW,
        pin.NormalW, toEyeW, shadowFactor);

    // directLight = toonShading(directLight);
    // directLight = toonShading_ks(directLight);
    // directLight = toonShading_kd(directLight);

    float4 litColor = ambient + directLight;


#ifdef FOG
    float fogAmount = saturate((distToEye - gFogStart) / gFogRange);
    litColor = lerp(litColor, gFogColor, fogAmount);
#endif


    // 분산재질에서 알파를 가져온다.
    litColor.a = diffuseAlbedo.a;

    return litColor;
}


