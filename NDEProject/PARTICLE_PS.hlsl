struct INPUT_PIXEL
{
	float4 projectedCoordinate : SV_POSITION;
	float4 color : COLOR;
	float4 uv : UV;
};

texture2D baseTexture : register(t0);
SamplerState filter : register(s0);

cbuffer MOD_DATA : register(b1)
{
	float4 color;
	float4 textureTranslation;
};

float4 main(INPUT_PIXEL _inputPixel) : SV_TARGET
{
	float finalU = _inputPixel.uv.x; //+ textureTranslation.x;
    float finalV = _inputPixel.uv.y; //+ textureTranslation.y;

	float4 textureColor;
	float4 finalColor;
	// Sample the pixel color from the texture using the sampler at this texture coordinate location.
	textureColor = baseTexture.Sample(filter, float2(finalU, finalV));

 	//textureColor.w -= textureTranslation.y * 2.0f;
	// Combine the texture color and the particle color to get the final color result.
	finalColor = textureColor * _inputPixel.color;
	clip(finalColor.w < 0.1f ? -1 : 1);

	return finalColor /** saturate(float4(0.75f, 0.58f, 0.25f, 1.0f) * (1 - textureTranslation.y))*/;
}
