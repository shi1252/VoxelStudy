cbuffer MatrixBuffer
{
	matrix worldMat;
	matrix viewMat;
	matrix projMat;
};

cbuffer LightBuffer
{
	float4 ambient;
	float4 diffuse;
	float3 lightDir;
	float padding;
	// float specularPower;
	// float4 specular;
};

cbuffer CameraBuffer
{
	float4 camPos;
};

struct VertexIn
{
	float4 position : POSITION;
	float2 uv : TEXCOORD0;
	float3 normal : NORMAL;
	float4 color : COLOR;
};

struct VertexOut
{
	float4 position : SV_POSITION;
	float3 normal : NORMAL;
	float4 color : COLOR;
	float2 uv : TEXCOORD0;
	float3 lightDir : TEXCOORD1;
	float3 viewDir : TEXCOORD2;
};

VertexOut VS(VertexIn input)
{
	VertexOut output;

	input.position.w = 1.0f;

	output.position = mul(input.position, worldMat);

	//output.lightDir = normalize(mul(lightDir, worldMat));

	output.viewDir = normalize(output.position.xyz - camPos);

	output.position = mul(output.position, viewMat);
	output.position = mul(output.position, projMat);

	output.uv = input.uv;
	
	output.normal = normalize(mul(input.normal, worldMat));

	output.color = normalize(input.color);

	return output;
}

Texture2D shaderTexture;
SamplerState SampleType;

float4 PS(VertexOut input) : SV_TARGET
{
	float4 color = ambient;

	float3 ld = normalize(lightDir);
	float4 diff = saturate(dot(-ld, input.normal));
	if (diff.x > 0)
	{
		color += diff * diffuse;
	}

	float4 textureColor = shaderTexture.Sample(SampleType, input.uv);
	color = textureColor * color;

	return color;
}
