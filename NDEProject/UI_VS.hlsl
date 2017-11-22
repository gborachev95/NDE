#pragma pack_matrix( row_major )
struct INPUT_VERTEX
{
	float4 coordinate : POSITION;
	float4 color : COLOR;
	float4 uv : UV;
};

struct OUTPUT_VERTEX
{
	float4 colorOut : COLOR;
	float4 projectedCoordinate : SV_POSITION;
	float4 uv : UV;
};

cbuffer OBJECT : register(b0)
{
	float4x4 worldMatrix;
}

OUTPUT_VERTEX main(INPUT_VERTEX fromVertexBuffer)
{
	OUTPUT_VERTEX sendToRasterizer = (OUTPUT_VERTEX)0;
	float4 localCoordinate = float4(fromVertexBuffer.coordinate.xyz, 1);
	// Sending data
	sendToRasterizer.projectedCoordinate = mul(localCoordinate, worldMatrix); //localCoordinate;
	sendToRasterizer.uv = fromVertexBuffer.uv;
	return sendToRasterizer;
}
