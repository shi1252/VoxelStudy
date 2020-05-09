cbuffer MatrixBuffer
{
	matrix worldMat;
	matrix viewMat;
	matrix projMat;
};

struct VertexIn
{
	float4 position : POSITION;
	float2 tex : TEXCOORD0;
};

struct VertexOut
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
};

VertexOut VS(VertexIn input)
{
	VertexOut output;

	input.position.w = 1.0f;

	output.position = mul(input.position, worldMat);
	output.position = mul(output.position, viewMat);
	output.position = mul(output.position, projMat);

	output.tex = input.tex;

	return output;
}

Texture2D shaderTexture;
SamplerState SampleType;

float4 PS(VertexOut input) : SV_TARGET
{
	float4 color = shaderTexture.Sample(SampleType, input.tex);
	return color;
}