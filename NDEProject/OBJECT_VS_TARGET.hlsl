#pragma pack_matrix( row_major )
struct INPUT_VERTEX
{
	float4 coordinate : POSITION;
	float4 normals    : NORMALS;
	float4 uv         : UV;
	float4 tangents   : TANGENTS;
	float4 bitangents : BITANGENTS;
	float4 indices    : SKIN_INDICES;
	float4 weights    : SKIN_WEIGHT;
};
struct OUTPUT_VERTEX
{
	float4 projectedCoordinate : SV_POSITION;
	float4 worldPosition       : POSITION;
	float4 normals             : NORMALS;
	float4 uv                  : UV;
	float4 tangents            : TANGENTS;
	float4 bitangents          : BITANGENTS;
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

cbuffer BONES : register(b2)
{
	float4x4 boneOffset[59];
	float4x4 positionOffset;
}

OUTPUT_VERTEX main(INPUT_VERTEX fromVertexBuffer)
{
	OUTPUT_VERTEX sendToRasterizer = (OUTPUT_VERTEX)0;
	float4 localCoordinate = float4(fromVertexBuffer.coordinate.xyz, 1);

	// Shading
	localCoordinate = mul(localCoordinate, worldMatrix);
	sendToRasterizer.worldPosition = localCoordinate;
	float4 worldNormals = mul(fromVertexBuffer.normals, worldMatrix);
	localCoordinate = mul(localCoordinate, viewMatrix);
	localCoordinate = mul(localCoordinate, projectionMatrix);
	//float3 bitangent = cross(fromVertexBuffer.normals, fromVertexBuffer.tangents);

	// Sending data
	sendToRasterizer.projectedCoordinate = localCoordinate;
	sendToRasterizer.normals = worldNormals;
	sendToRasterizer.uv = fromVertexBuffer.uv;
	sendToRasterizer.tangents = mul(fromVertexBuffer.tangents, worldMatrix);
	sendToRasterizer.bitangents = mul(fromVertexBuffer.bitangents, worldMatrix);

	return sendToRasterizer;
}
