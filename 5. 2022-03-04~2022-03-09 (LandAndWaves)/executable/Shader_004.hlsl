#ifndef NUM_DIR_LIGHTS
    #define NUM_DIR_LIGHTS 1
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


cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
};

cbuffer cbMaterial : register(b1)
{
	float4 gDiffuseAlbedo;
    float3 gFresnelR0;
    float  gRoughness;
	float4x4 gMatTransform;
};

cbuffer cbPass : register(b2)
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

    Light gLights[MaxLights];
};
 
struct VertexIn
{
	float3 PosL    : POSITION;
    float3 NormalL : NORMAL;
};

struct VertexOut
{
	float4 PosH    : SV_POSITION;
    float3 PosW    : POSITION;
    float3 NormalW : NORMAL;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout = (VertexOut)0.0f;
	
    // ���� ��ķ� ��ȯ
    float4 posW = mul(float4(vin.PosL, 1.0f), gWorld);
    vout.PosW = posW.xyz;

    // if1. ������Ŀ� ��յ��ʰ� �ִٸ�, ����ġ ��������� ����ؾ��Ѵ�.
    // if2. ������Ŀ� ��յ��ʰ� ���ٸ�, ��������� �̿��� ������ ��ȯ�Ѵ�.
    vout.NormalW = mul(vin.NormalL, (float3x3)gWorld);

    // �������ܰ������� ��ȯ
    vout.PosH = mul(posW, gViewProj);

    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    // ������ �����Ͽ�, �������̰� �ƴ� �� �ֱ⶧���� ����ȭ�Ѵ�.
    pin.NormalW = normalize(pin.NormalW);

    // ����Ǵ� ������ �������� ����
    float3 toEyeW = normalize(gEyePosW - pin.PosW);

	// �ֺ������
    float4 ambient = gAmbientLight*gDiffuseAlbedo;

    // ���౤���
    const float shininess = 1.0f - gRoughness;
    Material mat = { gDiffuseAlbedo, gFresnelR0, shininess };
    float3 shadowFactor = 1.0f;
    float4 directLight = ComputeLighting(gLights, mat, pin.PosW,
        pin.NormalW, toEyeW, shadowFactor);

    float4 litColor = ambient + directLight;

    // �л��������� ���ĸ� �����´�.
    litColor.a = gDiffuseAlbedo.a;

    return litColor;
}


