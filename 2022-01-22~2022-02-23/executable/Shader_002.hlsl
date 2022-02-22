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

	// 동차 절단 공간으로 변환한다.
	vOut.position = mul(float4(vIn.position, 1.0f), gWorldViewProj);

	// 픽셀 색상을 픽셀 셰이더에 그대로 전달한다.
	vOut.color = vIn.color;

	return vOut;
}

float4 PS(VertexOut vOut) : SV_TARGET{
	return vOut.color;
}