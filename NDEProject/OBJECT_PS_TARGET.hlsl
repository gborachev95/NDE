struct INPUT_PIXEL
{
	float4 projectedCoordinate : SV_POSITION;
	float4 worldPosition       : POSITION;
	float4 normals             : NORMALS;
	float4 uv                  : UV;
	float4 tangents            : TANGENTS;
	float4 bitangents          : BITANGENTS;
};

texture2D baseTexture : register(t0);
texture2D normalTexture : register(t1);
texture2D specularTexture : register(t2);
texture2D emissiveTexture : register(t3);

SamplerState filter : register(s0);
#define SPOTLIGHT 0
#define POINTLIGHT 1
#define COLORTEST 0

struct LIGHT_DATA
{
	float4 transform;
	float3 direction;
	float  radius;
	float4 color;
};
struct STRUCT_BUFFER
{
	int lightIndex;
	float4 lightColor;
};

cbuffer DIRECTION_LIGHT : register(b2)
{
	LIGHT_DATA dir_light;
};
cbuffer POINT_LIGHT : register(b3)
{
	LIGHT_DATA point_light[125];
};
cbuffer SPOT_LIGHT : register(b4)
{
	LIGHT_DATA spot_light;
};
cbuffer SCENE : register(b5)
{
	float4x4 viewMatrix;
	float4x4 projectionMatrix;
	float4 cameraPosition;
};

StructuredBuffer<STRUCT_BUFFER> structBufferTexture : register(t4);

#define CELL_WIDTH 5
#define CELL_HEIGHT 5
#define CELL_DEPTH 5
#define CELLS CELL_WIDTH * CELL_HEIGHT * CELL_DEPTH

float4 main(INPUT_PIXEL _inputPixel) : SV_TARGET
{
	unsigned int i = 0;

float4 finalColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
float4 dirLightColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
float4 pointLightColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
float4 spotLightColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
float4 specular = float4(0.0f, 0.0f, 0.0f, 1.0f);

// Normalize stuff
float3 normals = normalize(_inputPixel.normals).xyz;
float3 tangents = normalize(_inputPixel.tangents).xyz;
float3 bitangents = normalize(_inputPixel.bitangents).xyz;
// Normal mapping
float4 normalMap = normalTexture.Sample(filter, _inputPixel.uv.xy);
normalMap = (normalMap * 2.0f) - 1.0f;
float3 newTangent = tangents;// Mirror mapping
float3 newBitangent = bitangents;// Mirror mapping
float4 bumpNormal = float4(((normalMap.x * newTangent.xyz) + (normalMap.y * newBitangent.xyz) + (normalMap.z * normals.xyz)).xyz,1);
bumpNormal = normalize(bumpNormal);
// Specular mapping
float4 specularMap = specularTexture.Sample(filter, _inputPixel.uv.xy);
// Emissive mapping
float4 emissiveColor = emissiveTexture.Sample(filter, _inputPixel.uv.xy);

// Highlighted effect
if (normals.x < 0.6f && normals.x > -0.6f)
finalColor = float4(0.8f, 0.1f, 0.1f, 1.0f);
else
finalColor = baseTexture.Sample(filter, _inputPixel.uv.xy);


// ***Directional light*** //
if (dir_light.color.a > 0)
{
	float dirLightRatio = saturate(dot(-dir_light.direction.xyz, bumpNormal.xyz));
	dirLightColor = dirLightRatio * finalColor * dir_light.color;
	// Specular effect
	float3 dirPixelToCamera = normalize(cameraPosition.xyz - _inputPixel.worldPosition.xyz);
	float3 dirReflectionVec = normalize(reflect(dir_light.direction.xyz, bumpNormal.xyz));
	float dirSpecularFactor = dot(dirPixelToCamera.xyz, dirReflectionVec.xyz);
	if (dirSpecularFactor > 0.0f)
		specular += pow(abs(dirSpecularFactor), 1000) *  specularMap * _inputPixel.uv.z;
	dirLightColor += saturate(finalColor);
	// Turn on and off the light
	dirLightColor.xyz *= dir_light.color.a;
}

// ***Point light*** //
#if POINTLIGHT
float3 max = float3(157.937744f, 129.208206f, 101.357941f);
float3 min = float3(-111.172050f, -14.9827614f, -178.151398f);
// Find index of light
float3 offsetRatio = (_inputPixel.worldPosition.xyz - min.xyz) / (max.xyz - min.xyz);
unsigned int index = int(offsetRatio.z * CELL_DEPTH) * CELL_WIDTH * CELL_HEIGHT + int(offsetRatio.y * CELL_HEIGHT) * CELL_WIDTH + int(offsetRatio.x * CELL_WIDTH);
unsigned int lightIndex = structBufferTexture[index].lightIndex;
//269 * 144 * 279
if (point_light[lightIndex].color.a > 0)
{
	// Variables
	float  pointLightRatio = 0.0f;
	float  pointSpecularFactor = 0.0f;
	float  pointAttenuation = 0.0f;
	float3 pointLightDir = float3(0.0f, 0.0f, 0.0f);
	float3 cameraDir = float3(viewMatrix._31, viewMatrix._32, viewMatrix._33);
	float3 camLightDir = float3(0.0f, 0.0f, 0.0f);
	float3 pointPixelToCamera = float3(0.0f, 0.0f, 0.0f);
	float3 pointReflectionVec = float3(0.0f, 0.0f, 0.0f);

	// Lighting calculations 
	pointLightDir = point_light[lightIndex].transform.xyz - _inputPixel.worldPosition.xyz;
	pointAttenuation = saturate(1.0f - (dot(pointLightDir, pointLightDir) / (point_light[lightIndex].radius * 2.0f)));
	pointLightDir = normalize(pointLightDir);
	camLightDir = normalize(pointLightDir + cameraDir);
	pointLightRatio = saturate(dot(pointLightDir.xyz, bumpNormal.xyz));
	pointLightRatio *= 1 / dot(point_light[lightIndex].transform.xyz - _inputPixel.worldPosition.xyz,
		point_light[lightIndex].transform.xyz - _inputPixel.worldPosition.xyz) * point_light[lightIndex].radius;
	// Specular effect 
	pointPixelToCamera = normalize(cameraPosition.xyz - _inputPixel.worldPosition.xyz);
	pointReflectionVec = normalize(reflect(dir_light.direction.xyz, bumpNormal.xyz));
	pointSpecularFactor = dot(camLightDir, pointReflectionVec);
	if (pointSpecularFactor > 0.0f)
		specular += (pow(abs(pointSpecularFactor),128 /*32*/) *  specularMap * pointAttenuation) * _inputPixel.uv.z;

	// Light color 
	pointLightColor = saturate(finalColor * point_light[lightIndex].color * pointAttenuation * pointLightRatio);
	// Turn on and off the light 
	pointLightColor *= point_light[lightIndex].color.a;
	// Turn off specular 
	if (dir_light.color.a == 0 && point_light[lightIndex].color.a == 0)
		specular *= 0;
}
#endif

// ***Spot light*** //
#if SPOTLIGHT
float spotFactor = 0;
float3 lightDir = spot_light.transform.xyz - _inputPixel.worldPosition.xyz;
float len = length(lightDir);
lightDir = normalize(lightDir);
float surRatio = saturate(dot(-lightDir.xyz, spot_light.direction));
// Calculating attenuation
float aten = 1 - saturate((len / 20.0f));
if (surRatio > spot_light.radius)
spotFactor = 2.0f* aten;
else
spotFactor = aten * 0.7f;
float lightRatio = saturate(dot(lightDir.xyz, bumpNormal.xyz));
// Specular effect
float3 pixelToCamera = normalize(cameraPosition.xyz - _inputPixel.worldPosition.xyz);
float3 reflectionVec = normalize(reflect(lightDir.xyz,  bumpNormal.xyz));
float specularFactor = dot(pixelToCamera, reflectionVec);
if (specularFactor > 0.0f)
specular += pow(specularFactor, 32) * specularMap * aten * _inputPixel.uv.z;
spotLightColor = saturate(spotFactor * lightRatio * aten * spot_light.color * currColor);
// Turn on and off the light
spotLightColor *= spot_light.color.a;
#endif

#if COLORTEST
float3 max = float3(157.937744f, 129.208206f, 101.357941f);
float3 min = float3(-111.172050f, -14.9827614f, -178.151398f);
// Find index of light
float3 offsetRatio = (_inputPixel.worldPosition.xyz - min.xyz) / (max.xyz - min.xyz);
unsigned int index = int(offsetRatio.z * 10) * 100 + int(offsetRatio.y * 10) * 10 + int(offsetRatio.x * 10);
// Access the structure
return structBufferTexture[index].lightColor;
#else
// Return final color
float4 returnColor = saturate(dirLightColor + pointLightColor + saturate(specular));
clip(returnColor.w < 0.1f ? -1 : 1);
return returnColor;
#endif 
}
