struct INPUT_PIXEL
{
	float4 color : COLOR;
	float4 projectedCoordinate : SV_POSITION;
	float4 uv : UV;
};

texture2D baseTexture : register (t0);

texture2D depthTexture : register (t1);

SamplerState filter : register(s0);

cbuffer TIME : register(b0)
{
	float deltaTime;
	float effectType;
};

float4 main(INPUT_PIXEL _inputPixel) : SV_TARGET
{
float newU = _inputPixel.uv.x;
float newV = _inputPixel.uv.y;
//if (effectType == 5)
//newV += cos(_inputPixel.uv.y * 8 * 3.14159 + deltaTime) / 100;
float4 color = baseTexture.Sample(filter, float2(newU, newV));


// Multipliers
float densityMultiplier = 0.07f;
float distanceMultiplier = 300.0f;

// True value of Z
float zLinear = (2.0f * densityMultiplier) / (distanceMultiplier + densityMultiplier - depthTexture.Sample(filter, _inputPixel.uv.xy ).r * (distanceMultiplier - densityMultiplier));


//
// Fog
//if (effectType == 1)
color.rgba = lerp(float4(0.75f, 0.58f, 0.25f, 1.0f),  color.rgba, ( 1 - saturate(zLinear + effectType)) ); 
// TANGENT GIVES GOOD EFFECT // float4(0.75f, 0.58f, 0.25f, 1) 0.7f, 0.7f, 0.7f, 1.0f
// SILENT HILL color.rgba = lerp(float4(0.75f /* saturate(zLinear + effectType)*/, 0.58f, 0.25f, 1.0f), float4(saturate(zLinear + effectType), 0.0f, 0.0f, 1.0) - color.rgba, (1 - saturate(zLinear + effectType))); // TANGENT GIVES GOOD EFFECT //float4(0.75f, 0.58f, 0.25f, 1) 0.7f, 0.7f, 0.7f, 1.0f

// Mute colors
//if (effectType == 2)
//color.xyz = dot(float3(0.15f, 0.4f, 0.4f), color.xyz);
//
// Weird side effect
//if (effectType == 3)
//color.r = lerp(float4(1, 1, 1, 1), color.rgba, sin(deltaTime)).r;
//
// Blinking
//if (effectType == 4)
//color *= saturate(lerp(float4(0, 0, 0, 1), float4(0.5f, 0.5f, 0.5f, 1), (cos(deltaTime * 2) + _inputPixel.uv.y)));

//clip(color.w < 0.1f ? -1 : 1);
return color; //float4(1 - (color.x ), 1 - (color.x ) , 1 - (color.x), 1);

}