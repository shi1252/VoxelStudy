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
	float4 worldPos : TEXCOORD3;
};

VertexOut VS(VertexIn input)
{
	VertexOut output;

	input.position.w = 1.0f;

	output.position = mul(input.position, worldMat);
	output.worldPos = output.position;

	//output.lightDir = normalize(mul(lightDir, worldMat));

	output.viewDir = normalize(output.position.xyz - camPos);

	output.position = mul(output.position, viewMat);
	output.position = mul(output.position, projMat);

	output.uv = input.uv;
	
	output.normal = normalize(mul(input.normal, worldMat));

	output.color = normalize(input.color);

	return output;
}

Texture2D shaderTexture[4];
SamplerState SampleType;

float4 PS(VertexOut input) : SV_TARGET
{
	float3 triW = abs(input.normal.xyz);
	triW = saturate(triW - 0.2f);
	triW = pow(triW, 1.5f);
	triW = triW / (triW.x + triW.y + triW.z);

	float2 coordXY = input.worldPos.xy * 0.1f;
	if (input.normal.z < 0)
		coordXY.x = -coordXY.x;
	coordXY.x += 0.5f;
	float2 coordYZ = input.worldPos.zy * 0.1f;
	if (input.normal.x < 0)
		coordYZ.x = -coordYZ.x;
	coordYZ.y += 0.5f;
	float2 coordZX = input.worldPos.xz * 0.1f;
	if (input.normal.y < 0)
		coordZX.x = -coordZX.x;
	float4 colors[3];
	float2 fetchs[3];
	float3 bumps[3];

	colors[0] = shaderTexture[2].Sample(SampleType, coordXY);
	colors[1] = (input.normal.y > 0) ? shaderTexture[0].Sample(SampleType, coordZX) : shaderTexture[2].Sample(SampleType, coordZX);
	colors[2] = shaderTexture[2].Sample(SampleType, coordYZ);

	fetchs[0] = shaderTexture[3].Sample(SampleType, coordXY).xy - 0.5f;
	fetchs[1] = (input.normal.y > 0) ? shaderTexture[1].Sample(SampleType, coordZX).xy :  shaderTexture[3].Sample(SampleType, coordZX).xy;
	fetchs[2] = shaderTexture[3].Sample(SampleType, coordYZ).xy - 0.5f;

	bumps[0] = float3(0, fetchs[0].x, fetchs[0].y);
	bumps[1] = float3(fetchs[1].y, 0, fetchs[1].x);
	bumps[2] = float3(fetchs[2].x, fetchs[2].y, 0);

	float4 triColor = colors[0] * triW.zzzz + colors[1] * triW.yyyy + colors[2] * triW.xxxx;
	float3 bump = bumps[0] * triW.zzz + bumps[1] * triW.yyy + bumps[2] * triW.xxx;

	float4 color = ambient;

	float3 ld = normalize(lightDir);
	float3 normal = input.normal + bump;
	float4 diff = saturate(dot(-ld, normal));
	if (diff.x > 0)
	{
		color += diff * diffuse;
	}

	color = triColor * color;

	return color;
}
