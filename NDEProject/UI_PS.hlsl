struct INPUT_PIXEL
{
	float4 color : COLOR;
	float4 projectedCoordinate : SV_POSITION;
	float4 uv : UV;
};

texture2D baseTexture : register (t0);

SamplerState filter : register(s0);

cbuffer UI_DATA : register(b1)
{
	float4 color;
	float4 textureTranslation;
};

float4 main(INPUT_PIXEL _inputPixel) : SV_TARGET
{
	float finalU = _inputPixel.uv.x ;//+ textureTranslation.x;
    float finalV = _inputPixel.uv.y ;//+ textureTranslation.y;

	float4 finalPixel = baseTexture.Sample(filter, float2(finalU, finalV));
	finalPixel = float4(finalPixel.x * color.x, finalPixel.y * color.y, finalPixel.z * color.z, finalPixel.w * color.w);
	clip(finalPixel.w < 0.1f ? -1 : 1);
	return finalPixel;
}