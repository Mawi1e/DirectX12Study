cbuffer cbPerObject : register(b0) {
	float4x4 gWorldViewProj;
}

struct VertexIn {
	float3 position : POSITION;
	float4 color : COLOR;
};

struct VertexOut {
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

VertexOut VS(VertexIn vIn) {
	VertexOut vOut;

	// ���� ���� �������� ��ȯ�Ѵ�.
	vOut.position = mul(float4(vIn.position, 1.0f), gWorldViewProj);

	// �ȼ� ������ �ȼ� ���̴��� �״�� �����Ѵ�.
	vOut.color = vIn.color;

	return vOut;
}

float4 PS(VertexOut vOut) : SV_TARGET{
	return vOut.color;
}