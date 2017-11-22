#pragma pack_matrix( row_major )
struct INPUT_VERTEX
{
	float4 side : SIDE;
	float4 up : UP;
	float4 forward : FORWARD;
	float4 position : POSITION;
	float4 color : COLOR;
	float4 uv : UV;

};

struct OUTPUT_VERTEX
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float4 uv : UV;
};

cbuffer OBJECT : register(b0)
{
	float4x4 worldMatrix;
}
cbuffer SCENE : register(b1)
{
	float4x4 viewMatrix;
	float4x4 projectionMatrix;
	float4 cameraPosition;
}

OUTPUT_VERTEX main(INPUT_VERTEX input)
{
	OUTPUT_VERTEX output;

	//float4x4 partWorld;
	//partWorld._11 = input.side.x;	   //1;// 
	//partWorld._21 = input.side.y;	   //0;// 
	//partWorld._31 = input.side.z;	   //0;// 
	//partWorld._41 = input.side.w;	   //1;// 
	//								   //
	//partWorld._12 = input.up.x;		   //0;// 
	//partWorld._22 = input.up.y;		   //1;// 
	//partWorld._32 = input.up.z;		   //0;// 
	//partWorld._42 = input.up.w;		   //1;// 
	//								   //
	//partWorld._13 = input.forward.x;   //0;// 
	//partWorld._23 = input.forward.y;   //0;// 
	//partWorld._33 = input.forward.z;   //1;// 
	//partWorld._43 = input.forward.w;   //1;// 
	//								   //
	//partWorld._14 = input.position.x;  //0;// 
	//partWorld._24 = input.position.y;  //0;// 
	//partWorld._34 = input.position.z;  //0;// 
	//partWorld._44 = input.position.w;  //1;// 
	//
	//partWorld = mul(partWorld, worldMatrix);

	// Change the position vector to be 4 units for proper matrix calculations.
	input.position.w = 1.0f;

	// Calculate the position of the vertex against the world, view, and projection matrices.
	output.position = mul(input.position, worldMatrix);
	//output.position = mul(input.position, partWorld);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);

	// Store the texture coordinates for the pixel shader.
	output.uv = input.uv;
	// Store the particle color for the pixel shader. 
	output.color = input.color;

	return output;
}

