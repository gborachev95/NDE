#pragma pack_matrix( row_major )
struct INPUT_VERTEX
{
	float4 coordinate : POSITION;
	float4 normals    : NORMALS;
	float4 uv         : UV;
	float4 tangents   : TANGENTS;
	float4 bitangents : BITANGENTS;
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
	float4x4 instancesMatricies[20];
}
cbuffer SCENE : register(b1)
{
	float4x4 viewMatrix;
	float4x4 projectionMatrix;
	float4 cameraPosition;
}

OUTPUT_VERTEX main(INPUT_VERTEX fromVertexBuffer, unsigned int _id : SV_InstanceID)
{
	OUTPUT_VERTEX sendToRasterizer = (OUTPUT_VERTEX)0;
	float4 localCoordinate = float4(fromVertexBuffer.coordinate.xyz, 1);

	// Shading
	localCoordinate = mul(localCoordinate, instancesMatricies[_id]);
	sendToRasterizer.worldPosition = localCoordinate;
	float4 worldNormals = mul(fromVertexBuffer.normals, instancesMatricies[_id]);
	localCoordinate = mul(localCoordinate, viewMatrix);
	localCoordinate = mul(localCoordinate, projectionMatrix);

	// Sending data
	sendToRasterizer.projectedCoordinate = localCoordinate;
	sendToRasterizer.normals = worldNormals;
	sendToRasterizer.uv = fromVertexBuffer.uv;
	sendToRasterizer.tangents = mul(fromVertexBuffer.tangents, instancesMatricies[_id]);
	sendToRasterizer.bitangents = mul(fromVertexBuffer.bitangents, instancesMatricies[_id]);

	return sendToRasterizer;
}