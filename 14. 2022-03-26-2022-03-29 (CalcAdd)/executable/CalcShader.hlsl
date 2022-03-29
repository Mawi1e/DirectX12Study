struct InputA {
	float3 a;
	float2 b;
};

StructuredBuffer<InputA> gInputA : register(t0);
StructuredBuffer<InputA> gInputB : register(t1);
RWStructuredBuffer<InputA> gOutput : register(u0);

[numthreads(32, 1, 1)]
void CS(int3 dtid : SV_DispatchThreadID) {
	gOutput[dtid.x].a = gInputA[dtid.x].a + gInputB[dtid.x].a;
	gOutput[dtid.x].b = gInputA[dtid.x].b + gInputB[dtid.x].b;
}