struct VS_INPUT
{
	float3 position : POSITION;
	float4 color : COLOR;
	float2 texcoord : TEXCOORD;
};

float4x4 g_ProjectionMatrix : register( c0 );

float4 VS( VS_INPUT input ) : POSITION
{
	float4 position = mul( float4( input.position.x, input.position.y, 0.0f, 1.0f ), g_ProjectionMatrix );
	return position;
}