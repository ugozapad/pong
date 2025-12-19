struct VS_INPUT
{
	float3 position : POSITION;
	float4 color : COLOR;
	float2 texcoord : TEXCOORD;
};

struct VS_OUTPUT
{
	float4 position : POSITION;
	float2 texcoord : TEXCOORD;
};

float4x4 g_ModelViewProjection : register( c0 );

VS_OUTPUT VS( VS_INPUT input ) 
{
	VS_OUTPUT output;
	output.position = mul( g_ModelViewProjection, float4( input.position.x, input.position.y, input.position.z, 1.0f ) );
	output.texcoord = input.texcoord;
	return output;
}