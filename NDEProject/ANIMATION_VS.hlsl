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
	float4x4 instancesMatricies[2];
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

OUTPUT_VERTEX main(INPUT_VERTEX fromVertexBuffer, unsigned int _id : SV_InstanceID)
{
	OUTPUT_VERTEX sendToRasterizer = (OUTPUT_VERTEX)0;

	// Local coordinate with smooth skinning  
	float4 localCoordinate = float4(fromVertexBuffer.coordinate.xyz, 1);
	//if (_id == 1)
	//	localCoordinate = mul(localCoordinate, instancesMatricies[_id]);

	float4 animatedWorld = mul(localCoordinate, boneOffset[fromVertexBuffer.indices.x]) * fromVertexBuffer.weights.x; // The world position is animated at world origin
	animatedWorld += mul(localCoordinate, boneOffset[fromVertexBuffer.indices.y]) * fromVertexBuffer.weights.y;
	animatedWorld += mul(localCoordinate, boneOffset[fromVertexBuffer.indices.z]) * fromVertexBuffer.weights.z;
	animatedWorld += mul(localCoordinate, boneOffset[fromVertexBuffer.indices.w]) * fromVertexBuffer.weights.w;

	// Moving the object
	animatedWorld = mul(animatedWorld, positionOffset);
	//if (_id == 1)
	//	animatedWorld = mul(animatedWorld, instancesMatricies[_id]);

	sendToRasterizer.worldPosition = animatedWorld;

	// Smooth skinning normals
	float4 normal = float4(fromVertexBuffer.normals.xyz, 0);
	float4 animatedNormal = mul(normal, boneOffset[fromVertexBuffer.indices.x]) * fromVertexBuffer.weights.x;
	animatedNormal += mul(normal, boneOffset[fromVertexBuffer.indices.y]) * fromVertexBuffer.weights.y;
	animatedNormal += mul(normal, boneOffset[fromVertexBuffer.indices.z]) * fromVertexBuffer.weights.z;
	animatedNormal += mul(normal, boneOffset[fromVertexBuffer.indices.w]) * fromVertexBuffer.weights.w;

	// Smooth skinning tangents
	float4 tangent = float4(fromVertexBuffer.tangents.xyz, 0);
	float4 animatedTangent = mul(tangent, boneOffset[fromVertexBuffer.indices.x]) * fromVertexBuffer.weights.x;
	animatedTangent += mul(tangent, boneOffset[fromVertexBuffer.indices.y]) * fromVertexBuffer.weights.y;
	animatedTangent += mul(tangent, boneOffset[fromVertexBuffer.indices.z]) * fromVertexBuffer.weights.z;
	animatedTangent += mul(tangent, boneOffset[fromVertexBuffer.indices.w]) * fromVertexBuffer.weights.w;

	// Smooth skinning tangents
	float4 animatedBitangent = float4(cross(animatedNormal.xyz, animatedTangent.xyz).xyz,0);

	// Coordinate in world space
	//sendToRasterizer.worldPosition = mul(localCoordinate, instancesMatricies[_id]);

	// Coodrinate in projection space
	sendToRasterizer.projectedCoordinate = mul(animatedWorld, viewMatrix);
	sendToRasterizer.projectedCoordinate = mul(sendToRasterizer.projectedCoordinate, projectionMatrix);

	//animatedNormal = mul(animatedNormal, positionOffset);
	// Normals in world space
	sendToRasterizer.normals = animatedNormal = mul(animatedNormal, instancesMatricies[_id]);
	sendToRasterizer.uv.xyz = fromVertexBuffer.uv.xyz;
	
	// Tangends and bitangents in worldspace
	sendToRasterizer.tangents = mul(animatedTangent, instancesMatricies[_id]).xyzw;
	sendToRasterizer.bitangents = mul(animatedBitangent, instancesMatricies[_id]).xyzw;

	// Sending data to Rasterizer
	return sendToRasterizer;
}
